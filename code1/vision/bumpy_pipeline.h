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

/* ---- 阈值宏 (v3, 2026-08-19) ---- */
/* 阶段② 阈值 (亮度归一双阈值带符号, 数据依据: lat_study/卷积核评估与阈值锚定证明报告.md):
   T = BP_NORM_K·mean(gray), k∈[2500,3300] 免调参窗口 (曝光严格消去);
   判定: gy ≥ +T → horiz=1 (正沿); gy ≤ −T → horiz=2 (负沿); 其余 0.
   取消绝对值: 两个阈值正好对应两种符号连通域, 天然拆符号 (ccl8 等值邻接). */
#ifndef BP_NORM_K
#define BP_NORM_K 2500
#endif
#ifndef BP_HDG_STD_MAX
#define BP_HDG_STD_MAX    2.0f    /* 帧内条纹倾角散布门限 (°): 超过则本帧 hdg 不可信 */
#endif
#define BP_MIN_CC_PIX     30      /* 连通域最小像素 */
#define BP_MIN_CC_W       30      /* 连通域最小宽度 */
#define BP_EDGE_M         3       /* 外点硬边界剔除 */
#define BP_LINEAR_SIGMA   1.5f    /* 域线性度门槛 (rms) */
#define BP_ALONG_GAP      40.0f   /* 跨域剔除: 沿条带方向断裂间隙容差 */
#define BP_CROSS_TOL      2.0f    /* 跨域剔除: 垂直条带方向同带容差 */
#define BP_MIN_HDG_LINES  2       /* 帧航向角(hdg_valid)有效最少横向条纹数：拒绝单线（方差+个数双门限，2026-08-19 v3） */

/* ---- 单线结果 (保留类型定义, v3 边线不再用: 边线=IPM 后物理 x 主带均值) ---- */
typedef struct {
    int    valid;
    float  ang;      /* [0,180) 条纹方向角 */
    float  cx, cy;   /* 拟合线中心 */
    int    n;        /* 内点数 */
} bumpy_line_t;

/* ---- 单帧结果 ---- */
#define BP_LIN_MAX 16   /* 导出的横向线性连通域数上限 (渲染用, 2026-08-19) */
#define BP_OUT_MAX 48   /* 每侧导出的外点(像素)数上限 (v3: 供 bumpy_vision 逐点 IPM → 物理 x 主带提取, 2026-08-19) */
typedef struct {
    int    hdg_valid;  /* 帧航向角(条纹倾斜角)有效：合规条纹 ≥BP_MIN_HDG_LINES(2) 且 方差≤BP_HDG_STD_MAX */
    float  hdg;        /* 帧航向角 [deg]，frame_heading 加权圆均值直出 */
    /* 左右外点像素集 (v3, 2026-08-19): 每合规线性 CC 的 x 极值 3 点 + 同符号跨域剔除;
       由 bumpy_vision 逐点 IPM → 物理 x 主带 → 主带内点均值 = 边线物理 x (替代 RANSAC 拟合+基准行) */
    uint8_t lp_n, rp_n;
    int16_t lp_x[BP_OUT_MAX], lp_y[BP_OUT_MAX];
    int16_t rp_x[BP_OUT_MAX], rp_y[BP_OUT_MAX];
    /* 横向线性连通域列表 (条纹, 渲染用, 2026-08-19): 质心 + PCA 方向角 + 像素数 */
    uint8_t lin_n;
    float   lin_cx[BP_LIN_MAX], lin_cy[BP_LIN_MAX], lin_ang[BP_LIN_MAX];
    int16_t lin_pix[BP_LIN_MAX];
} bumpy_frame_result_t;

/* ---- 跨帧状态 (按视频隔离; SRAM 精简版, v3 无时间验证历史) ---- */
typedef struct {
    /* 工作缓冲 (单帧, 阶段间时分复用, 互不重迭, 详见 bumpy_pipeline.c 顶部, v3 gy-only):
       gy   : ① Gy (垂直 D 核输出)  → ② 双阈值判定 → ③④ 并查集 uf (int32 别名, 用量 ≤ PIX/2+1)
       gyh  : ① 卷积水平中间结果 (bumpy_conv7_gy scratch, 1×PIX int32)
              → ③④ 拆为 labels(uint16[PIX]) + relab(uint16[PIX]) (两段别名, 用前清零)
       horiz: ② 双阈值带符号输出 (0/1/2) → ③ CCL 输入 (全程只读) */
    int32_t gy[BUMPY_PIX];      /* Gy (卷积输出, 行优先) */
    int32_t gyh[BUMPY_PIX];     /* 水平中间 gyh + ③④ labels/relab 复用区 (见上) */
    uint8_t horiz[BUMPY_PIX];   /* 横向条纹掩膜 0/1/2 (CCL 输入; 无 strong 落 RAM) */
} bumpy_pipeline_t;

void bumpy_pipeline_init(bumpy_pipeline_t *s);
void bumpy_pipeline_frame(bumpy_pipeline_t *s, const uint8_t *img, bumpy_frame_result_t *out);

#ifdef __cplusplus
}
#endif

#endif /* _BUMPY_PIPELINE_H_ */
