/**
 * ============================================================================
 * bumpy_pipeline.h  ——  颠簸路三段式边线提取 (C 落地)
 * ============================================================================
 * 管线 (与 heatmap/pipeline_extract_v3.py 完全同语义, 阈值宏复用):
 *   ① 梯度+阈值: 平面区域 |Gy|≥p85·VERT_RELAX 且 |Gx|≤p85·VERT_RELAX·HORIZ_CAP
 *                 (p85 按 |G| 分位), 横向条纹 |θs|<DIR_TOL
 *   ② 横向连通域+方向角: 8邻域 CCL → 每域 PCA 主轴角 + 线性度 rms
 *   ③ 外点: 每域 x 极值 3 点 → 倾角外扩跨域剔除 (ALONG_GAP/CROSS_TOL) → RANSAC
 *   ④ 帧航向角 (线性域加权圆均值) + 夹角门控 MAX_HDG_DIFF
 *   ⑤ 时间验证: 连续 MIN_STABLE 帧稳定 + 跳变滤除 (MAX_ANG_JMP/MAX_POS_JMP)
 *
 * 状态 bumpy_pipeline_t 按"视频"隔离 (时间历史只在本视频内延续).
 * ============================================================================
 */
#ifndef _BUMPY_PIPELINE_H_
#define _BUMPY_PIPELINE_H_

#include <stdint.h>
#include "bumpy_conv.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 阶段调试: 1=板端逐帧打印中间统计 (定位正确性用, 默认关) */
#ifndef BP_DEBUG_FRAME
#define BP_DEBUG_FRAME 0
#endif

/* 时间验证开关: 1=连续 MIN_STABLE 帧稳定才显示 (真实部署);
   0=每帧独立输出 (batch test 图集帧序被打乱, 无法凑连续帧) */
#ifndef BP_TEMPORAL
#define BP_TEMPORAL 1
#endif

/* 分段计时: 1=打印 frame 内各阶段周期 (性能定位用, 默认关).
   启用时调用方需实现 unsigned int bp_stage_cyc(void) 返回周期计数 (MCU=DWT). */
#ifndef BP_STAGE_TIMER
#define BP_STAGE_TIMER 0
#endif
#if BP_STAGE_TIMER
unsigned int bp_stage_cyc(void);
#endif

/* ---- 阈值宏 (与 Python 参考一致) ---- */
#define BP_MAG_PCT        85.0f   /* 强梯度分位 */
#define BP_VERT_RELAX     0.88f   /* 垂直方向阈值放松系数 (平面区域) */
#define BP_HORIZ_CAP      0.60f   /* 水平(切向)分量上限系数 */
#define BP_DIR_TOL        20.0f   /* 横向条纹角度容差 (°) */
#define BP_MIN_CC_PIX     30      /* 连通域最小像素 */
#define BP_MIN_CC_W       30      /* 连通域最小宽度 */
#define BP_EDGE_M         3       /* 外点硬边界剔除 */
#define BP_LINEAR_SIGMA   1.5f    /* 域线性度门槛 (rms) */
#define BP_ALONG_GAP      40.0f   /* 跨域剔除: 沿条带方向断裂间隙容差 */
#define BP_CROSS_TOL      2.0f    /* 跨域剔除: 垂直条带方向同带容差 */
#define BP_RANSAC_TOL     3.0f    /* RANSAC 绝对容差 (px) */
#define BP_RANSAC_ITERS   300     /* RANSAC 迭代数 */
#define BP_MIN_OUT_N      5       /* 显著性: 内点至少 5 */
#define BP_MIN_OUT_SPAN   8.0f    /* 显著性: 内点 x 或 y 跨度至少 8px */
#define BP_MAX_HDG_DIFF   70.0f   /* 夹角门控: 边线与航向角差上限 */
#define BP_MIN_STABLE     3       /* 时间验证: 连续帧数 */
#define BP_MAX_ANG_JMP    8.0f    /* 时间验证: 角度跳变上限 */
#define BP_MAX_POS_JMP    5.0f    /* 时间验证: 中心位移上限 */
#define BP_MIN_HDG_LINES  3       /* 帧航向角(hdg_valid)有效最少横向条纹数：检出少于 N 条即视为"无颠簸条纹"（2026-08-18 新增，可调） */

/* ---- 单线结果 ---- */
typedef struct {
    int    valid;
    float  ang;      /* [0,180) 条纹方向角 */
    float  cx, cy;   /* 拟合线中心 */
    int    n;        /* 内点数 */
} bumpy_line_t;

/* ---- 单帧结果 ---- */
typedef struct {
    bumpy_line_t L, R;
    bumpy_line_t raw_L, raw_R;  /* 单帧 RANSAC 拟合原始边线（未时间验证；渲染/横向观测用，2026-08-18） */
    int    hdg_valid;  /* 帧航向角(条纹倾斜角)有效：检出 ≥BP_MIN_HDG_LINES 条横向线性连通域即 1，与边线成败无关 */
    float  hdg;        /* 帧航向角 [deg]，frame_heading 加权圆均值直出（2026-08-17 引出） */
} bumpy_frame_result_t;

/* ---- 跨帧状态 (按视频隔离; SRAM 精简版) ---- */
typedef struct {
    /* 时间验证历史 */
    int    hL_n;  float hL_a[BP_MIN_STABLE]; float hL_x[BP_MIN_STABLE]; float hL_y[BP_MIN_STABLE];
    int    hR_n;  float hR_a[BP_MIN_STABLE]; float hR_x[BP_MIN_STABLE]; float hR_y[BP_MIN_STABLE];
    /* 工作缓冲 (单帧, 阶段间时分复用, 互不重迭, 详见 bumpy_pipeline.c 顶部):
       gx   : ①② Gx            → ③④ CCL 标号 labels (int32 别名)
       gy   : ①② Gy            → ③ 并查集 uf (int32 别名, 用量 ≤ PIX/2+1 << PIX)
       mag2 : ① 卷积水平中间结果 gxh/gyh (bumpy_conv7 scratch, 2×PIX int32)
              → ② |G|² (就地 quickselect 分位, 之后不再需要)
              → ③ CCL relab 根→新域映射 (int32 别名, 用前清零) */
    int32_t gx[BUMPY_PIX];      /* Gx (卷积输出, 行优先) */
    int32_t gy[BUMPY_PIX];      /* Gy */
    uint64_t mag2[BUMPY_PIX];   /* |G|² + 复用区 (见上) */
    uint8_t horiz[BUMPY_PIX];   /* 横向条纹掩膜 (CCL 输入; strong 为同循环局部量, 不落 RAM) */
} bumpy_pipeline_t;

void bumpy_pipeline_init(bumpy_pipeline_t *s);
void bumpy_pipeline_frame(bumpy_pipeline_t *s, const uint8_t *img, bumpy_frame_result_t *out);

#ifdef __cplusplus
}
#endif

#endif /* _BUMPY_PIPELINE_H_ */
