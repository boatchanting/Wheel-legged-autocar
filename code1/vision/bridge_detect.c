/**
 * ============================================================================
 * bridge_detect.c  ——  单边桥三线透视结构提取 (C 端, 与 pc_tools/bridge_v4.py 一致)
 * ============================================================================
 * Copyright (C) 2026  Ji Zixiang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ============================================================================
 * 流水线:
 *   94x60 → 汇编 4x4 可分离卷积 (Gx/Gy 57x91)
 *   → lock 抑制 + p99 动态阈值 + 每行/列 top-2 候选
 *   → 序贯 RANSAC 提全部竖线 → 间距先验分类 (红/绿/蓝)
 *   → VP 共点精化 → 门控粉色退出线 (五重校验)
 * ============================================================================
 */

#include "bridge_detect.h"
#include "bridge_asm_ops.h"
#include "tcm.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/* BRIDGE_PROF_ON 不定义: 精准分段测时开关, 需要时在工程选项里打开 (2026-08-14 移植时关闭) */

/* ===== 精准分段测时 (2026-08-13) =====
   问题: 115200 波特 printf ~1.7ms/行 — 任何 printf 在测时窗内都会污染该段
   (旧版 c_thr/base 被内部打印虚高, 帧耗时需手工相加且含错漏)。
   方案: 帧内只记时间戳 PROF_MARK()(零输出), 相邻两戳 = 该段纯时间差;
   帧末由 main 在测时区外调 bridge_prof_report() 一次性打印全部分段 + 总计,
   无需相加, 无 printf 污染。
   槽位: 0=帧始 1=conv后 2=step3_4后 3=ransac后 4=rest后(base末)
         5=blur始 6=blur后 7=feats后 8=fit后 9=topedge后
   step3_4 内部独立槽位: gh / bg / enq。
   DWT 使能由 main 完成; 直接地址访问 CYCCNT(不 include core_cm7.h 避 FPU 冲突)。 */
#if defined(__ICCARM__) && defined(BRIDGE_PROF_ON)
#define BRIDGE_PROF 1
#define BRIDGE_DWT_CYCCNT (*(volatile uint32_t *)0xE0001004UL)
#define PROF_US(x)   ((uint32_t)((x) / 250))
#define PROF_PRINT(...) printf(__VA_ARGS__)
#define DWT_CYCCNT_RAW BRIDGE_DWT_CYCCNT

#define BRIDGE_PROF_MARKS 10
#define BRIDGE_PROF_S34   4
#define BRIDGE_PROF_FIT   4
#define BRIDGE_PROF_FEAT  4
static uint32_t s_pmk[BRIDGE_PROF_MARKS];
static int      s_pn;
static uint32_t s_s34mk[BRIDGE_PROF_S34];
static int      s_s34n;
static uint32_t s_fitmk[BRIDGE_PROF_FIT];
static int      s_fitn;
static uint32_t s_femk[BRIDGE_PROF_FEAT];
static int      s_fen;
#define PROF_BEGIN()     (s_pn = 0, s_s34n = 0, s_fitn = 0, s_fen = 0, s_pmk[s_pn++] = DWT_CYCCNT_RAW)
#define PROF_MARK()      do { if (s_pn < BRIDGE_PROF_MARKS) s_pmk[s_pn++] = DWT_CYCCNT_RAW; } while (0)
#define PROF_S34_BEGIN() (s_s34n = 0, s_s34mk[s_s34n++] = DWT_CYCCNT_RAW)
#define PROF_S34_MARK()  do { if (s_s34n < BRIDGE_PROF_S34) s_s34mk[s_s34n++] = DWT_CYCCNT_RAW; } while (0)
#define PROF_FIT_BEGIN() (s_fitn = 0, s_fitmk[s_fitn++] = DWT_CYCCNT_RAW)
#define PROF_FIT_MARK()  do { if (s_fitn < BRIDGE_PROF_FIT) s_fitmk[s_fitn++] = DWT_CYCCNT_RAW; } while (0)
#define PROF_FE_BEGIN()  (s_fen = 0, s_femk[s_fen++] = DWT_CYCCNT_RAW)
#define PROF_FE_MARK()   do { if (s_fen < BRIDGE_PROF_FEAT) s_femk[s_fen++] = DWT_CYCCNT_RAW; } while (0)
void bridge_prof_report(void);
#else
#define PROF_PRINT(...)
#define DWT_CYCCNT_RAW 0
#define PROF_BEGIN()     ((void)0)
#define PROF_MARK()      ((void)0)
#define PROF_S34_BEGIN() ((void)0)
#define PROF_S34_MARK()  ((void)0)
#define PROF_FIT_BEGIN() ((void)0)
#define PROF_FIT_MARK()  ((void)0)
#define PROF_FE_BEGIN()  ((void)0)
#define PROF_FE_MARK()   ((void)0)
void bridge_prof_report(void);
#endif

#ifdef BRIDGE_PROF
/* 帧末报告 (main 在测时区外调用): 各段纯时间差 + step3_4 内部 + 总计 */
void bridge_prof_report(void)
{
    static const char *nm[BRIDGE_PROF_MARKS - 1] = {
        "conv", "step34", "ransac", "rest",
        "mlp_prep", "blur", "feats", "fit", "topedge"
    };
    static const char *sn[BRIDGE_PROF_S34 - 1] = { "gh", "bg", "enq" };
    static const char *fn[BRIDGE_PROF_FIT - 1] = { "fprep", "fcross", "fransac" };
    static const char *en[BRIDGE_PROF_FEAT - 1] = { "fgypeak", "fgyagg", "frow" };
    int i;
    for (i = 0; i + 1 < s_pn && i < BRIDGE_PROF_MARKS - 1; i++)
        printf("p%s=%lu ", nm[i], PROF_US(s_pmk[i + 1] - s_pmk[i]));
    for (i = 0; i + 1 < s_s34n && i < BRIDGE_PROF_S34 - 1; i++)
        printf("s%s=%lu ", sn[i], PROF_US(s_s34mk[i + 1] - s_s34mk[i]));
    for (i = 0; i + 1 < s_fitn && i < BRIDGE_PROF_FIT - 1; i++)
        printf("f%s=%lu ", fn[i], PROF_US(s_fitmk[i + 1] - s_fitmk[i]));
    for (i = 0; i + 1 < s_fen && i < BRIDGE_PROF_FEAT - 1; i++)
        printf("e%s=%lu ", en[i], PROF_US(s_femk[i + 1] - s_femk[i]));
    printf("total=%lu\r\n", PROF_US(s_pmk[s_pn - 1] - s_pmk[0]));
}
#else
void bridge_prof_report(void) { }
#endif

/* 调试: BRIDGE_EDGE_DBG=1 时打印每候选线的边线度量 (仅 host 分析用, MCU 不受影响) */
static int s_edge_dbg = 0;

/* ================================ 常量 (与 PC 版一致) ================================ */
#define W           BRIDGE_W            /* 94  */
#define H           BRIDGE_H            /* 60  */
#define GW          (W - 3)             /* 91  */
#define GH          (H - 3)             /* 57  */

#define LOCK_K      2.0f                /* lock 抑制比 |g| > K·|var|      */
#define T_FLOOR     300.0f              /* 动态阈值下限                   */
#define Q_P99       0.3f                /* 阈值 = Q_P99 · p99             */
#define TOPK        2                   /* 每行/列候选数                  */

#define MIN_INLIERS 4                   /* RANSAC 最少内点                */
#define INLIER_TOL  1.5f                /* 内点容差 (px)                  */
#define RANSAC_ITER 40                  /* RANSAC 迭代数 (实测: 40~150 零质量损失, 20 出现假线) */
#define SLOPE_MAX_V 2.5f                /* 竖线斜率上限                   */
#define SLOPE_MAX_H 0.9f                /* 顶线斜率上限                   */

#define VLINE_MAX   4                   /* 每种符号最多提取线数           */
#define MIN_LINE_INL 10                 /* 成线最少内点                   */
#define DEDUP_DX    2.5f                /* 双线去重距离 @Y_REF            */

#define Y_REF       55.0f               /* 间距参考行                     */
#define MIN_SPACING 14.0f               /* 无先验引导的最小红蓝间距       */

/* ---- 左右边线先验间距 w(y)=A*y+B (随 y 变化, 透视线性) ----
   距离合规(间距≥LO*w) 才是边线; 过近(间距<LO*w) → 提取的是中线 (用户)。
   可直接调参; 运行中从 RB/RMB 帧的 (y,w) 最小二乘自校准 A/B。 */
#define W_PRIOR_INIT_A  1.75f   /* 初始斜率 (px/行, 66GT 中位 1.75)    */
#define W_PRIOR_INIT_B  3.0f    /* 初始截距 (px @y=0, 66GT 中位 3.0)   */
#define W_PRIOR_LO      0.70f   /* 过近判据: 间距 < LO*w(y) → 中线      */
#define W_PRIOR_HI      1.40f   /* 过远判据: 间距 > HI*w(y) → 异常      */
#define W_CALIB_MIN_N   20.0f   /* 自校准最小样本数(生效门槛)           */
#define W_CALIB_MAX_N   400.0f  /* 自校准样本上限(超限整体减半滑动)     */
#define W_CALIB_W_MIN   8.0f    /* 有效间距样本下限(px)                 */

/* ---- 边线差分校验 (2026-08-07): 边线外侧(地面)必须明显暗于内侧(桥面),
       否则该"边线"实为中线被误配。两个互补的局部判据 (2026-08-07 晚 用户设计:
       一小段"两边同亮"就足以判中线, 无需全局平均):
       · 段二次矩差比和 |内²-外²|/(内²+外²) —— 两侧对比度 (曝光不变)
       · 段"两边同亮"否决 —— 某段 i2>MID_BRIGHT2 且 o2>MID_BRIGHT2 (两侧都是
         桥面亮区) → 该线在桥面内部 → 直接判中线 (真边线外侧是地面, 永不触发)
       任一段差比和 > RATIO_EDGE2 → 有边线证据; 任一段两边同亮 → 中线优先;
       外侧全段出画 → 贴边。 ---- */
#define EDGE_SEG     4                   /* 沿全行分段数 (每段15行)       */
#define MIN_FLANK_N 40                  /* 外侧带最少样本数(全行采样, 半段≈45) 否则贴边 */
#define RATIO_EDGE2 0.45f               /* 段两侧二次矩差比和阈值 (用户: 用二次矩) */
#define MID_BRIGHT2 0.85f               /* 段两侧二次矩都>此 → 该段两边近白同亮(桥面)
                                             → 中线 (用户: 局部证据, 不要全局)。
                                             0.85 = 亮度≈226/255 (近纯白桥面)。
                                             66GT 亮地面外侧仅 0.51~0.83, 不触发。 */
#define EDGE_MARGIN 10.0f               /* 边框伪线判定: 线@Y_REF 距画面边缘<此 视为贴边框
                                             (2026-08-07 晚 用户: 太靠边用先验/配对一致性滤掉) */

/* ---- 有效检测 valid 判定 (2026-08-09 用户定案, 取消帧级白像素层, 线级恒真) ----
   线级级联全通才有桥: 边线 maxr>VALID_MXR → 夹角<VALID_ANGLE → 靠近点y<VALID_YC
     → 间距 w_min>=VALID_WMIN → 边线包裹区条带白>VALID_STRIP_W。
   全部帧统一走此级联 (无 wh/botwhite 快速通道)。 ---- */
#define VALID_MXR     0.35f              /* 边线原图亮度差(4段二次矩差比和)下限 */
#define VALID_ANGLE   90.0f              /* 线对夹角上限 (真桥 50-76°)      */
#define VALID_YC      30.0f              /* 线对交点(靠近点)行坐标上限: 交点须在画面上方(远处);
                                             交点在下方=严重夹角错误 (v02_00240/241 交点y>200) */
#define VALID_WMIN    15.0f              /* 线对最小间距下限 (过近 10-14 无效) */
#define VALID_STRIP_W 0.5f               /* 边线包裹区最大条带近白比例下限 */
#define VALID_NSTRIP  12                 /* 条带白分带数 */
#define VALID_WHITE   200                /* 近白像素灰度阈值 */

/* ---- 三线平行约束 (2026-08-07 用户): 三线近似平行(共消失点), 否则否决绿线 ---- */
#define PAR_A_TOL    0.15f               /* G 斜率与 R/B 中位斜率最大偏差 */

#define GATE_ROWS   52                  /* 底部变白门控起始行             */

/* ---- 行背景判断 (与 pc_tools/bridge_v5.py row_bg_mask 一致) ---- */
#define ROW_BG_FILTER 1                 /* 0=关闭行过滤 (A/B 回退)        */
#define CLU_MAX     4                   /* 行内边缘簇数上限 (红绿蓝+1)    */
#define MID_LO      0.3f                /* 中间带下界 (x 动态阈值 t)      */
#define MID_DIST    2                   /* 强簇拖尾半径                   */
#define MID_OUT_MAX 8                   /* 簇外中间带像素数上限           */
#define ROW_OK_MIN  12                  /* 降级回退的最少有效行数         */

/* ---- 粉色脱出线: 连通亮区顶边界 (与 pc_tools/bridge_v6.py 一致) ---- */
#define TOP_GRAD    0                   /* 1=旧梯度法 (A/B), 0=亮区法     */
#define TOP_SLOPE   0.6f                /* 脱出线斜率上限                 */
#define TOP_MIN_PTS 6                   /* 平台列数/内点下限              */
#define TOP_MIN_SPAN 4.0f               /* 平台 x 跨度下限                */
#define TOP_ABOVE_MAX 0.35f             /* 线上方允许最大亮比例           */
#define TOP_BELOW_MIN 0.5f              /* 线下方最小亮比例               */

/* ---- 上方回桥面否决 (2026-08-07 用户: 结束线增加前瞻) ----
   结束线上方 12~32 行 近白(I>200) 比例过高 → 上方仍是桥面 → 该结束线是假。
   场景: 边线取代中线后, 两线间区域完美符合结束线, MLP 在桥面内部误检;
   真结束线上方是远处地面(非近白)。 */
#define TOP_FAR_WHITE 0.12f             /* 上方近白比例阈值 (与 PC bridge_v8
                                           TOP_FAR_WHITE=0.12 同步 2026-08-13;
                                           66GT 真T max=0.03, 6081 亮带延伸被拒) */
#define TOP_FAR_LO    12
#define TOP_FAR_HI    32
#define TOP_FAR_THR   200

/* ---- v8 weak_top_bright 否决 (2026-08-12 金标准重标定) ----
   弱包络(仅一条边线 R 或 B, 无对侧/中线) 且 结束线在画面顶部(T_y<28)
   且 结束线上方 0~12 行整行亮度均值>80 → 画面顶部白色带被误检为 T。
   金标准依据: 66GT 真T 单线帧 upg 全<80 或 T_y>=28 或有中线;
   6081 顶部白色带虚假帧 (如 02 00802, 07 00512) 被拒。 */
#define V8_WEAK_TL   28.0f              /* 结束线 T_y 上限 */
#define V8_WEAK_UM   80.0f              /* 上方 0~12 行亮度均值上限 */
#define V8_WEAK_BAND 12                 /* 上方带行数 */

/* ---- v8 边线交点(消失点)否决 (2026-08-12 金标准推算) ----
   R/B 交点 y_i>=30 → 交点跑到画面下方 → 检测几何异常。
   66GT 真桥交点 y_i p90=1.8; 与 valid_detect VALID_YC=30 一致。 */
#define V8_INT_Y     30.0f

/* ---- v8 无桥面滤除: 全图双峰(Otsu 类间方差) (2026-08-12 用户定案) ----
   用户: 区分背景应是基于全图的东西 (PC bimodal_stats.otsu_bcv 全图版)。
   全图 Otsu 类间方差 = 整图"暗背景 <-> 亮桥面"双峰可分强度;
   total=94*60=5640 固定 -> 阈值可标定(非魔法数), 区分度 5.5x 无重叠:
   66GT 真T bcv min=5.47e10 vs 无桥面(02_00405/03_00773) max=1.27e10;
   6081 误报 07_00448-451 (暗场景) bcv~7e9 被拒。阈值下限(hard)优于门控。 */
#define V8_BCV_GLOBAL_MIN 2.0e10f      /* 全图类间方差下限 (真T min=5.47e10) */

/* ---- v11 结束线: gy>0 行游程连通域贯通 (2026-08-13 用户定案, 对齐 bridge_v11.py) ----------------
   取消 MLP, 极简逻辑: 包络内逐行 gy>THR run → 跨行 x 重叠 union 成连通域
   → 连通域整体贯通 (最左到左边界 / 最右到右边界) → 逐列 gy 峰值拟合斜顶边。 */
#define V11_GY_THR       200          /* gy 阈值 (2026-08-13 标定) */
#define V11_LEFT_TOL     1            /* 贯通左端容差 (左严) */
#define V11_SPAN_TOL     6            /* 贯通右端容差 (右宽, 倾斜容许) */
#define V11_EDGE_MARGIN  5            /* 缺侧边线: run 触到 x=5 / W-6 即算接边 */
#define V11_SLOPE_MAX    0.9f         /* 结束线斜率上限 */
#define V11_MAX_RUNS     512          /* run 上限 */
#define V11_MAX_ROW_RUNS 32           /* 每行 run 上限 */

#define MID_R_LO    0.35f               /* 中线间距比带                   */
#define MID_R_HI    0.65f
#define MID_SUP_MRG 8.0f                /* 支撑范围余量                   */

#define MAX_CAND    (GH * TOPK)         /* 竖线候选上限 (单符号)          */
#define MAX_TOPC    (GW * TOPK)         /* 顶线候选上限                   */
#define MAX_LINES   (2 * VLINE_MAX)     /* 全部竖线上限                   */

/* ================================ 数据类型 ================================ */
typedef struct { float u, v, w; } bpt_t;    /* 候选点 (自变量, 因变量, 权) */

typedef struct {
    bridge_line_t f;                    /* 拟合直线                       */
    float   inl_u[MAX_CAND];            /* 内点自变量 (竖线: y)           */
    int16_t inl_n;                      /* 内点数                         */
} iline_t;

/* ================================ 静态缓冲 ================================ */
/* SRAM: 降采样图 + 梯度全帧 */
DTCM_BSS static uint8_t  s_img[H][W];   /* R5b(2026-08-13): 移 DTCM 减压 L1
                                           (gvar/hvar 每像素 16 次读; 5.6KB < 14KB 空闲) */
static int16_t  s_gx[GH][GW];
static int16_t  s_gy[GH][GW];

/* DTCM: 卷积行缓冲 (s_raw 必须 4 字节对齐: 汇编 LDR 字加载) */
#if defined(__ICCARM__)
#define DATA_ALIGN4 _Pragma("data_alignment=4")
#else
#define DATA_ALIGN4
#endif
DATA_ALIGN4 DTCM_BSS int16_t s_raw[W + 2];
DTCM_BSS int16_t s_ringx[4][GW + 1];    /* GW+1=92: 行起始保持 4 字节对齐 */
DTCM_BSS int16_t s_ringy[4][GW + 1];

/* 候选点 / RANSAC 工作区 */
static bpt_t    s_pos[MAX_CAND], s_neg[MAX_CAND], s_topc[MAX_TOPC];
static bpt_t    s_rem[MAX_TOPC];        /* 序贯 RANSAC 剩余点 (取大者)    */
static uint8_t  s_mask[MAX_TOPC];
static iline_t  s_lines[MAX_LINES];
static float    s_sort[MAX_CAND];       /* 分位数排序工作区               */

/* 行背景判断: 行有效性掩码 + 行内 strong 标志 + 每行 top-2 暂存 */
static uint8_t  s_row_ok[GH];
static uint8_t  s_strong[GW];
static int8_t   s_tail_nd[GW];       /* R6c: 右邻 MID_DIST 内有强像素标志 (拖尾剔除用) */
static int16_t  s_bpx[GH][2], s_bpm[GH][2];
static int16_t  s_bnx[GH][2], s_bnm[GH][2];
/* lock box-diff 行缓存: 每行一次计算, step3/4 复用 (消除重复 gvar/hvar)
   R5b(2026-08-13): 移 DTCM — s_gh/s_bg 每像素访问, DTCM 1 周期 */
DTCM_BSS static int16_t s_gvar_r[GW];
DTCM_BSS static int16_t s_hvar_r[GW];

/* 脱出线 (亮区法): 区域位图 + BFS 队列 + 逐行包络 */
#define REG_WORDS   ((W + 31) / 32)
static uint32_t s_region[H][REG_WORDS];
static uint16_t s_bfs_q[W * H];
static int16_t  s_env_lo[H], s_env_hi[H];
static int8_t   s_col_top[W];

/* v11 结束线: gy 行游程连通域 */
typedef struct { int16_t yg, x0, x1; } v11_run_t;
static v11_run_t s_v11_runs[V11_MAX_RUNS];
static int16_t  s_v11_row_runs[GH][V11_MAX_ROW_RUNS];
static int16_t  s_v11_row_n[GH];
static int16_t  s_v11_bounds[GH][2];       /* 每行包络 c0/c1 (gy 坐标) */
static int16_t  s_v11_parent[V11_MAX_RUNS];
static int16_t  s_v11_cxmin[V11_MAX_RUNS], s_v11_cxmax[V11_MAX_RUNS];
static int16_t  s_v11_cymin[V11_MAX_RUNS], s_v11_cymax[V11_MAX_RUNS];
static bpt_t    s_v11_pts[GW];             /* 逐列拟合点 (最多 91 列) */
static float    s_v11_y[GW];               /* 拟合点 y 值 (求中位数) */

/* ================================ 小工具 ================================ */
static int cmp_f32(const void *a, const void *b)
{
    float d = *(const float *)a - *(const float *)b;
    return (d > 0) - (d < 0);
}

static int cmp_u8(const void *a, const void *b)
{
    return (int)*(const uint8_t *)a - (int)*(const uint8_t *)b;
}

/* gvar: 中间两列垂直 box-diff (抑制 gx 的水平边缘响应), 输出点 (r,j)
   C2(2026-08-13): static inline, step3 热路径每像素调用 ~10K 次 */
static inline int gvar_at(int r, int j)
{
    const uint8_t *r0 = s_img[r], *r1 = s_img[r + 1];
    const uint8_t *r2 = s_img[r + 2], *r3 = s_img[r + 3];
    int c1 = j + 1, c2 = j + 2;
    return (r2[c1] + r3[c1] - r0[c1] - r1[c1])
         + (r2[c2] + r3[c2] - r0[c2] - r1[c2]);
}

/* hvar: 中间两行水平 box-diff (抑制 gy), 输出点 (r,j) */
static inline int hvar_at(int r, int j)
{
    const uint8_t *r1 = s_img[r + 1], *r2 = s_img[r + 2];
    return (r1[j + 2] + r1[j + 3] - r1[j] - r1[j + 1])
         + (r2[j + 2] + r2[j + 3] - r2[j] - r2[j + 1]);
}

/* 256 bin 直方图 (|g|>>4) 的 p99 估计 → 动态阈值 */
ITCM_FUNC static float thr_from_hist(const uint16_t *hist)
{
    uint32_t total = 0, cum = 0;
    int i;
    for (i = 0; i < 256; i++)
        total += hist[i];
    if (!total)
        return T_FLOOR;
    for (i = 0; i < 256; i++) {
        cum += hist[i];
        if (cum * 100 >= total * 99)
            break;
    }
    {
        float p99 = (float)(i << 4);
        float t = Q_P99 * p99;
        return t > T_FLOOR ? t : T_FLOOR;
    }
}

/* ================================ RANSAC ================================ */
static uint32_t s_rng;
static uint32_t xr32(void)
{
    s_rng ^= s_rng << 13;
    s_rng ^= s_rng >> 17;
    s_rng ^= s_rng << 5;
    return s_rng;
}

/* 最大内点数直线 v = a·u + b; 定数种子, 返回内点数 (0=失败) */
ITCM_FUNC static int ransac_best(const bpt_t *p, int n, float smax,
                       float *oa, float *ob, uint8_t *mask)
{
    int best_n = 0, it, i;
    float best_a = 0, best_b = 0;
    if (n < MIN_INLIERS)
        return 0;
    s_rng = 12345;
    for (it = 0; it < RANSAC_ITER; it++) {
        int i1 = (int)(xr32() % (uint32_t)n);
        int i2 = (int)(xr32() % (uint32_t)n);
        float a, b, du;
        int cnt = 0;
        if (i1 == i2)
            continue;
        du = p[i2].u - p[i1].u;
        if (du > -1e-6f && du < 1e-6f)
            continue;
        a = (p[i2].v - p[i1].v) / du;
        if (a > smax || a < -smax)
            continue;
        b = p[i1].v - a * p[i1].u;
        for (i = 0; i < n; i++) {
            float r = p[i].v - (a * p[i].u + b);
            if (r < 0)
                r = -r;
            if (r <= INLIER_TOL)
                cnt++;
        }
        if (cnt > best_n) {
            best_n = cnt;
            best_a = a;
            best_b = b;
            if (best_n * 3 >= n * 2)     /* 内点≥2/3 提前终止 */
                break;
        }
    }
    if (best_n < MIN_INLIERS)
        return 0;
    for (i = 0; i < n; i++) {
        float r = p[i].v - (best_a * p[i].u + best_b);
        if (r < 0)
            r = -r;
        mask[i] = (r <= INLIER_TOL);
    }
    *oa = best_a;
    *ob = best_b;
    return best_n;
}

/* 内点加权最小二乘重拟合, 返回 rms */
ITCM_FUNC static float refit(const bpt_t *p, const uint8_t *mask, int n,
                   float *oa, float *ob, int *on)
{
    float sw = 0, su = 0, sv = 0, suu = 0, suv = 0, a, b, se = 0;
    int i, m = 0;
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            float w = p[i].w;
            sw += w;
            su += w * p[i].u;
            sv += w * p[i].v;
            suu += w * p[i].u * p[i].u;
            suv += w * p[i].u * p[i].v;
            m++;
        }
    }
    {
        float den = sw * suu - su * su;
        if (den < 1e-9f && den > -1e-9f)
            den = 1e-9f;
        a = (sw * suv - su * sv) / den;
        b = (sv - a * su) / sw;
    }
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            float r = p[i].v - (a * p[i].u + b);
            se += r * r;
        }
    }
    *oa = a;
    *ob = b;
    *on = m;
    return sqrtf(se / m);
}

/* ================================ 竖线提取 ================================ */
/* 单符号序贯 RANSAC, 结果从 s_lines[base] 起追加, 返回条数 */
ITCM_FUNC static int extract_sign_lines(int base, const bpt_t *pts, int n, int max_out)
{
    int cnt = 0;
    iline_t *L;
    memcpy(s_rem, pts, (size_t)n * sizeof(bpt_t));
    while (cnt < max_out) {
        float a, b, rms;
        int nin, nn, i, m;
        nin = ransac_best(s_rem, n, SLOPE_MAX_V, &a, &b, s_mask);
        if (!nin)
            break;
        rms = refit(s_rem, s_mask, n, &a, &b, &nn);
        if (nn < MIN_LINE_INL)
            break;
        L = &s_lines[base + cnt];
        L->f.a = a;
        L->f.b = b;
        L->f.rms = rms;
        L->f.n = (int16_t)nn;
        /* 记录内点自变量 + 支撑范围 (p10/p90 ± MID_SUP_MRG) */
        m = 0;
        for (i = 0; i < n; i++) {
            if (s_mask[i]) {
                L->inl_u[m] = s_rem[i].u;
                s_sort[m] = s_rem[i].u;
                m++;
            }
        }
        L->inl_n = (int16_t)m;
        qsort(s_sort, (size_t)m, sizeof(float), cmp_f32);
        L->f.u_lo = s_sort[m / 10] - MID_SUP_MRG;
        L->f.u_hi = s_sort[m - 1 - m / 10] + MID_SUP_MRG;
        cnt++;
        /* 移除内点 */
        m = 0;
        for (i = 0; i < n; i++) {
            if (!s_mask[i])
                s_rem[m++] = s_rem[i];
        }
        n = m;
    }
    return cnt;
}

/* 合并排序 (x@Y_REF 升序) + 双线去重, 返回最终线数 */
static int merge_lines(int n)
{
    int i, j, m = 0;
    /* 插入排序 */
    for (i = 1; i < n; i++) {
        iline_t t = s_lines[i];
        float x = t.f.a * Y_REF + t.f.b;
        for (j = i - 1; j >= 0; j--) {
            float xj = s_lines[j].f.a * Y_REF + s_lines[j].f.b;
            if (xj <= x)
                break;
            s_lines[j + 1] = s_lines[j];
        }
        s_lines[j + 1] = t;
    }
    /* 去重: 相邻 < DEDUP_DX 保留内点更多者 */
    for (i = 0; i < n; i++) {
        if (m > 0) {
            float x0 = s_lines[m - 1].f.a * Y_REF + s_lines[m - 1].f.b;
            float x1 = s_lines[i].f.a * Y_REF + s_lines[i].f.b;
            if (x1 - x0 < DEDUP_DX) {
                if (s_lines[i].f.n > s_lines[m - 1].f.n)
                    s_lines[m - 1] = s_lines[i];
                continue;
            }
        }
        s_lines[m++] = s_lines[i];
    }
    return m;
}

/* ================================ 线身份分类 ================================ */
/* 线外侧 4~9px 带亮比例 > 0.5 ?  (side=-1 左 / +1 右) */
static int outer_bright(const iline_t *L, int side, int tb)
{
    int step = L->inl_n / 20, i, br = 0, tot = 0;
    if (step < 1)
        step = 1;
    for (i = 0; i < L->inl_n; i += step) {
        int y = (int)L->inl_u[i];
        int x = (int)(L->f.a * L->inl_u[i] + L->f.b);
        int k;
        if (y < 0 || y >= H)
            continue;
        for (k = 0; k < 6; k++) {
            int xx = (side < 0) ? (x - 9 + k) : (x + 4 + k);
            if (xx >= 0 && xx < W) {
                br += s_img[y][xx] > tb;
                tot++;
            }
        }
    }
    return tot > 0 && br * 2 > tot;
}

/* 沿线全行分 EDGE_SEG 段, 两侧二次矩差比和 |内2-外2|/(内2+外2) > RATIO_EDGE2
   → 边线证据。二次矩 E[(I/255)^2] 强调高亮像素, 比一次矩(均值)对曝光更稳定
   (用户 2026-08-07: 差比和应使用二次矩输入; 只保留小分段判据)。
   side<0: 左边界候选(内侧=右带); side>0: 右边界候选(内侧=左带)。
   返回: 1=边线, 0=中线, -1=贴边 */
static int line_edge_ratio(const iline_t *L, int side);
static void edge_dbg_print(const iline_t *L, int side);

static int line_edge_ratio(const iline_t *L, int side)
{
    int k, s, has_edge = 0, outer_n = 0;
    for (s = 0; s < EDGE_SEG; s++) {
        int y0 = s * H / EDGE_SEG, y1 = (s + 1) * H / EDGE_SEG, y;
        float ls2 = 0, rs2 = 0;          /* 左/右带 二次矩和 Σ(I/255)^2 */
        int lc = 0, rc = 0;
        for (y = y0; y < y1; y++) {
            int x = (int)(L->f.a * (float)y + L->f.b);
            for (k = 0; k < 6; k++) {
                int xl = x - 11 + k, xr = x + 6 + k;   /* 间隔2px */
                if (xl >= 0 && xl < W) {
                    float v = s_img[y][xl] / 255.0f;
                    ls2 += v * v; lc++;
                }
                if (xr >= 0 && xr < W) {
                    float v = s_img[y][xr] / 255.0f;
                    rs2 += v * v; rc++;
                }
            }
        }
        outer_n += (side < 0) ? lc : rc;
        if (lc >= 6 && rc >= 6) {
            float i2 = (side < 0) ? rs2 / rc : ls2 / lc;   /* 内侧二次矩 */
            float o2 = (side < 0) ? ls2 / lc : rs2 / rc;   /* 外侧二次矩 */
            float ratio = fabsf(i2 - o2) / (i2 + o2 + 1e-3f);
            /* 局部中线证据 (用户): 该段两侧都亮(桥面) → 线在桥面内部 → 中线。
               真边线外侧是地面(暗), 此段永不出现。 */
            if (i2 > MID_BRIGHT2 && o2 > MID_BRIGHT2)
                { edge_dbg_print(L, side); return 0; }
            if (ratio > RATIO_EDGE2)
                has_edge = 1;
        }
    }
    if (has_edge)
        { edge_dbg_print(L, side); return 1; }
    if (outer_n < MIN_FLANK_N)
        { edge_dbg_print(L, side); return -1; }  /* 外侧全段出画 → 贴边 */
    edge_dbg_print(L, side);
    return 0;                            /* 无强对比段 → 中线 */
}

/* 调试: 打印单线边线度量 (host 分析用) */
static void edge_dbg_print(const iline_t *L, int side)
{
    int s, k, outer_n = 0;
    float outer_s2 = 0.0f, maxr = 0.0f;
    float i2s[EDGE_SEG], o2s[EDGE_SEG];
    if (!s_edge_dbg)
        return;
    for (s = 0; s < EDGE_SEG; s++) {
        int y0 = s * H / EDGE_SEG, y1 = (s + 1) * H / EDGE_SEG, y;
        float ls2 = 0, rs2 = 0;
        int lc = 0, rc = 0;
        for (y = y0; y < y1; y++) {
            int x = (int)(L->f.a * (float)y + L->f.b);
            for (k = 0; k < 6; k++) {
                int xl = x - 11 + k, xr = x + 6 + k;
                if (xl >= 0 && xl < W) {
                    float v = s_img[y][xl] / 255.0f;
                    ls2 += v * v; lc++;
                }
                if (xr >= 0 && xr < W) {
                    float v = s_img[y][xr] / 255.0f;
                    rs2 += v * v; rc++;
                }
            }
        }
        outer_n += (side < 0) ? lc : rc;
        outer_s2 += (side < 0) ? ls2 : rs2;
        i2s[s] = (side < 0) ? rs2 / rc : ls2 / lc;
        o2s[s] = (side < 0) ? ls2 / lc : rs2 / rc;
    }
    printf("dbg-edge side=%+d a=%.3f b=%.1f outer_s2=%.3f outer_n=%d ",
           side, L->f.a, L->f.b, (outer_n > 0) ? outer_s2 / outer_n : 0.0f, outer_n);
    for (s = 0; s < EDGE_SEG; s++) {
        float r = fabsf(i2s[s] - o2s[s]) / (i2s[s] + o2s[s] + 1e-3f);
        if (r > maxr) maxr = r;
        printf("s%d(i=%.2f,o=%.2f,r=%.3f) ", s, i2s[s], o2s[s], r);
    }
    {
        int dec, s2, any_bright = 0, any_edge = 0;
        for (s2 = 0; s2 < EDGE_SEG; s2++) {
            if (i2s[s2] > MID_BRIGHT2 && o2s[s2] > MID_BRIGHT2)
                any_bright = 1;          /* 两边同亮 → 中线 (优先) */
            else if (fabsf(i2s[s2] - o2s[s2]) /
                     (i2s[s2] + o2s[s2] + 1e-3f) > RATIO_EDGE2)
                any_edge = 1;
        }
        if (any_bright) dec = 0;
        else if (any_edge) dec = 1;
        else if (outer_n < MIN_FLANK_N) dec = -1;
        else dec = 0;
        printf("maxr=%.3f dec=%d\n", maxr, dec);
    }
}

/* 边线差分校验 (差比和): 正常分支用。有边线证据或贴边 → 边线; 否则中线 */
static int line_is_edge(const iline_t *L, int side)
{
    return line_edge_ratio(L, side) != 0;
}

/* 边线状态: 1=有效边线, 0=非边线(中线), -1=贴边无法校验 (外侧全段出画) */
static int line_edge_status(const iline_t *L, int side)
{
    return line_edge_ratio(L, side);
}

/* ==================== 有效检测 valid (线级级联, 2026-08-09) ====================
   与 PC 端 review_bridge_gui.line_maxr 一致: 沿线分 4 段, 每段两侧二次矩差比
   r=|内²-外²|/(内²+外²), 取 4 段最大值。真边线>0.5, 幻觉线<0.1。
   采样 x 用 round (与 PC 一致; 原 line_edge_ratio 用 truncate 会差 1px)。 */
static float line_maxr_val(const bridge_line_t *L, int side)
{
    int k, s;
    float maxr = 0.0f;
    for (s = 0; s < EDGE_SEG; s++) {
        int y0 = s * H / EDGE_SEG, y1 = (s + 1) * H / EDGE_SEG, y;
        float ls2 = 0, rs2 = 0;
        int lc = 0, rc = 0;
        for (y = y0; y < y1; y++) {
            int x = (int)(L->a * (float)y + L->b + 0.5f);
            for (k = 0; k < 6; k++) {
                int xl = x - 11 + k, xr = x + 6 + k;   /* 间隔2px */
                if (xl >= 0 && xl < W) {
                    float v = s_img[y][xl] / 255.0f;
                    ls2 += v * v; lc++;
                }
                if (xr >= 0 && xr < W) {
                    float v = s_img[y][xr] / 255.0f;
                    rs2 += v * v; rc++;
                }
            }
        }
        if (lc >= 6 && rc >= 6) {
            float i2 = (side < 0) ? rs2 / rc : ls2 / lc;   /* 内侧二次矩 */
            float o2 = (side < 0) ? ls2 / lc : rs2 / rc;   /* 外侧二次矩 */
            float r = fabsf(i2 - o2) / (i2 + o2 + 1e-3f);
            if (r > maxr) maxr = r;
        }
    }
    return maxr;
}

/* 边线包裹区条带白 (与 PC interline_maxwhite 一致): 两线之间区域按 VALID_NSTRIP
   条带分, 每条带统计近白(I>VALID_WHITE)比例, 返回最大带比例。
   用户: 全局平均 wh 会被区域外稀释, 条带法凸显局部桥面白带。 */
static float interline_maxwhite_c(const bridge_line_t *l1,
                                  const bridge_line_t *l2)
{
    int s;
    float mx = 0.0f;
    for (s = 0; s < VALID_NSTRIP; s++) {
        int y0 = s * H / VALID_NSTRIP, y1 = (s + 1) * H / VALID_NSTRIP;
        int br = 0, tot = 0, y;
        for (y = y0; y < y1; y++) {
            float xl = l1->a * (float)y + l1->b;
            float xr = l2->a * (float)y + l2->b;
            int x0 = (int)(xl < xr ? xl : xr) + 2;
            int x1 = (int)(xl > xr ? xl : xr) - 2;
            int x;
            if (x0 < 0) x0 = 0;
            if (x1 > W - 1) x1 = W - 1;
            for (x = x0; x <= x1; x++) {
                if (s_img[y][x] > VALID_WHITE)
                    br++;
                tot++;
            }
        }
        if (tot && (float)br / tot > mx)
            mx = (float)br / tot;
    }
    return mx;
}

/* 有效检测判定: 纯线级级联全通 (2026-08-09 用户定案, 取消 wh 层)。
   入参: has_red/green/blue + 三条线 (未检出线可 NULL)。
   规则: 纯绿线→无效; 无边线→无效; 边线maxr<=VALID_MXR→无效;
   线对: R&B→(R,B); 仅单边线+有G→(边线,G); 仅单边线→无效;
   级联: 夹角<VALID_ANGLE → 靠近点y<VALID_YC → w_min>=VALID_WMIN → 条带白>VALID_STRIP_W。 */
static int valid_detect(const bridge_line_t *red, const bridge_line_t *green,
                        const bridge_line_t *blue)
{
    const bridge_line_t *l1, *l2;
    int has_r = red != NULL, has_g = green != NULL, has_b = blue != NULL;
    float a1, b1, a2, b2, cc, ang, yc, wmin, mxw;
    int da, y;

    if (!has_r && !has_b)
        return 0;                        /* 无边线 (含纯绿线) */
    if (has_r && has_b) {                /* 线对 = R-B */
        l1 = red;
        l2 = blue;
    } else if (has_g) {                  /* 单边线+中线 */
        l1 = has_r ? red : blue;
        l2 = green;
    } else {
        return 0;                        /* 仅单边线, 无中线可配对 */
    }
    a1 = l1->a; b1 = l1->b;
    a2 = l2->a; b2 = l2->b;
    /* 边线亮度差: 所有边线 maxr>VALID_MXR (原图亮度差, 4段二次矩差比和) */
    if (has_r && line_maxr_val(red, -1) <= VALID_MXR)
        return 0;
    if (has_b && line_maxr_val(blue, +1) <= VALID_MXR)
        return 0;
    /* 级联: 夹角 → 靠近点 → 间距 → 白带 */
    cc = (a1 * a2 + 1.0f) /
         (sqrtf(a1 * a1 + 1.0f) * sqrtf(a2 * a2 + 1.0f));
    ang = acosf(cc < -1.0f ? -1.0f : (cc > 1.0f ? 1.0f : cc)) * 57.29578f;
    if (ang > VALID_ANGLE)
        return 0;
    da = (int)((a2 - a1) * 1000.0f);     /* 交点 y = -(b2-b1)/(a2-a1) */
    if (da != 0) {
        yc = -(b2 - b1) / (a2 - a1);
        if (yc > VALID_YC)
            return 0;                    /* 靠近点在画面下方 → 严重夹角错误 */
    }
    wmin = 1e9f;
    for (y = 0; y < H; y++) {
        float w = fabsf((a2 * y + b2) - (a1 * y + b1));
        if (w < wmin) wmin = w;
    }
    if (wmin < VALID_WMIN)
        return 0;
    mxw = interline_maxwhite_c(l1, l2);
    if (mxw <= VALID_STRIP_W)
        return 0;
    return 1;
}

/* 由 中线+有效边线 推断缺失的另一侧边线 (中线≈红蓝平分线):
   out = 2*mid - side。仅用于一条边线严重贴边(外侧出画, 无法阈值校验)的情形
   (用户规则)。梯度佐证: 沿线 |gx| 强点比例≥40% 才放入; 推断线出画则跳过。 */
static int infer_side_line(const iline_t *mid, const iline_t *side,
                           float *out_a, float *out_b)
{
    float ai = 2.0f * mid->f.a - side->f.a;
    float bi = 2.0f * mid->f.b - side->f.b;
    float xr = ai * Y_REF + bi;
    int i, hit = 0, tot = 0;
    if (xr < 4.0f || xr > W - 5.0f)
        return 0;                          /* 推断线出画 */
    for (i = 0; i < GH; i += 2) {
        float y = (float)i + 1.5f;
        float xf = ai * y + bi;
        int gy = (int)(y + 0.5f), gx = (int)(xf + 0.5f);
        int g;
        if (gy < 0 || gy >= GH || gx < 0 || gx >= GW)
            continue;
        g = s_gx[gy][gx];
        if (g < 0) g = -g;
        if (g > 700) hit++;
        tot++;
    }
    if (tot < 8 || hit * 5 < tot * 2)
        return 0;
    *out_a = ai;
    *out_b = bi;
    return 1;
}

/* 中线几何硬条件: 三线平行(共消失点) + 支撑范围内参考行间距比 ∈ [0.35,0.65]
   平行约束 (用户 2026-08-07): G 斜率必须 ≈ R/B 斜率中位 (透视收敛下 R,B 反向
   倾斜 ~1.5, 但 G 恒为二者中位, p90 偏差仅 0.065)。不平行 → 否决绿线。 */
static int mid_geo_ok(const iline_t *red, const iline_t *mid,
                      const iline_t *blue)
{
    static const float rows[3] = { 15.0f, 38.0f, Y_REF };
    static const float minw[3] = { 12.0f, 6.0f, 3.0f };
    int checked = 0, t;
    if (fabsf(mid->f.a - (red->f.a + blue->f.a) * 0.5f) > PAR_A_TOL)
        return 0;                        /* G 斜率偏离 R/B 中位 → 否决绿线 */
    for (t = 0; t < 3; t++) {
        float y = rows[t];
        float xl, xr, w, r;
        if (y < mid->f.u_lo || y > mid->f.u_hi)
            continue;                       /* 支撑外纯外推, 豁免 */
        xl = red->f.a * y + red->f.b;
        xr = blue->f.a * y + blue->f.b;
        w = xr - xl;
        if (w < minw[t])
            continue;
        r = (mid->f.a * y + mid->f.b - xl) / w;
        if (r < MID_R_LO || r > MID_R_HI)
            return 0;
        checked++;
    }
    return checked > 0;
}

/* ---- 随 y 变化先验间距自校准 w(y) = wp_a*y + wp_b (最小二乘) ----
   RB/RMB 帧喂 (y, w) 样本; 样本数≥W_CALIB_MIN_N 后生效; 超上限整体减半滑动。 */
static void wp_add_sample(bridge_state_t *st, float y, float w)
{
    if (w < W_CALIB_W_MIN || w > W - W_CALIB_W_MIN)
        return;
    st->wp_n += 1.0f;
    st->wp_sy  += y;
    st->wp_sw  += w;
    st->wp_syy += y * y;
    st->wp_syw += y * w;
    if (st->wp_n >= W_CALIB_MAX_N) {     /* 滑动: 历史减半, 保持响应性 */
        st->wp_n *= 0.5f;
        st->wp_sy *= 0.5f;  st->wp_sw *= 0.5f;
        st->wp_syy *= 0.5f; st->wp_syw *= 0.5f;
    }
    if (st->wp_n >= W_CALIB_MIN_N) {
        float n = st->wp_n;
        float den = n * st->wp_syy - st->wp_sy * st->wp_sy;
        if (den > 1e-6f) {
            st->wp_a = (n * st->wp_syw - st->wp_sy * st->wp_sw) / den;
            st->wp_b = (st->wp_sw - st->wp_a * st->wp_sy) / n;
        }
    }
}

/* 用红蓝边线在多个参考行的间距喂自校准 */
static void wp_calibrate_frame(bridge_state_t *st,
                               const iline_t *red, const iline_t *blue)
{
    static const float rows[4] = { 15.0f, 30.0f, 45.0f, Y_REF };
    int i;
    for (i = 0; i < 4; i++) {
        float y = rows[i];
        float w = (blue->f.a * y + blue->f.b) - (red->f.a * y + red->f.b);
        wp_add_sample(st, y, w);
    }
    if (s_edge_dbg)
        printf("dbg-wp n=%.0f A=%.3f B=%.1f w55=%.1f\n",
               st->wp_n, st->wp_a, st->wp_b, st->wp_a * Y_REF + st->wp_b);
}

/* 候选红蓝对 距离合规检查: 在参考行 Y_REF 上, 实测间距须 ≥ LO*prior 才是边线;
   过近(< LO*prior) → 该对实为 中线+边线 (用户: 近的就是提取出来的中线)。
   prior 与配对选择用同一值 (随 y 变化自校准模型在 Y_REF 的值), 两线须在画面内。 */
static int pair_too_close(const iline_t *l, const iline_t *r, float prior)
{
    float y = Y_REF;
    float xl = l->f.a * y + l->f.b;
    float xr = r->f.a * y + r->f.b;
    float wm;
    if (xl < 2.0f || xl > W - 3.0f || xr < 2.0f || xr > W - 3.0f)
        return 0;                        /* 出画外推不可靠, 不判过近 */
    if (prior < W_CALIB_W_MIN)
        return 0;
    wm = xr - xl;
    return wm < W_PRIOR_LO * prior;
}

/* 分类: 填 ir/ig/ib (索引, -1=无), 返回 mode, *sp_out=红蓝间距(无则0) */
static bridge_mode_t classify(int n, float prior, int tb,
                              int *ir, int *ig, int *ib, float *sp_out)
{
    float xs[MAX_LINES];
    int i, j;
    *ir = *ig = *ib = -1;
    *sp_out = 0;
    if (prior < MIN_SPACING)
        prior = MIN_SPACING;
    for (i = 0; i < n; i++)
        xs[i] = s_lines[i].f.a * Y_REF + s_lines[i].f.b;

    if (n == 0)
        return BRIDGE_MODE_NONE;
    if (n == 1) {
        int lb = outer_bright(&s_lines[0], -1, tb);
        int rb = outer_bright(&s_lines[0], +1, tb);
        if (!lb && rb) { *ir = 0; return BRIDGE_MODE_R; }
        if (lb && !rb) { *ib = 0; return BRIDGE_MODE_B; }
        *ig = 0;
        return BRIDGE_MODE_M;
    }
    /* 选红蓝候选对: 有先验取间距最接近先验的; 无先验取最宽对 */
    {
        int bi = -1, bj = -1;
        float best_sc = 0, bs = 0;
        for (i = 0; i < n; i++) {
            for (j = i + 1; j < n; j++) {
                float s = xs[j] - xs[i], sc;
                if (s < 4)
                    continue;
                sc = (prior > 0) ? fabsf(s - prior) : -s;
                if (bi < 0 || sc < best_sc) {
                    best_sc = sc;
                    bi = i;
                    bj = j;
                    bs = s;
                }
            }
        }
        if (bi < 0)
            return BRIDGE_MODE_NONE;
        if (s_edge_dbg)
            printf("dbg-cls n=%d prior=%.1f pair=(%d,%d) bs=%.1f wm=%.1f ",
                   n, prior, bi, bj, bs,
                   (s_lines[bj].f.a - s_lines[bi].f.a) * Y_REF +
                   (s_lines[bj].f.b - s_lines[bi].f.b));
        if (pair_too_close(&s_lines[bi], &s_lines[bj], prior)) {
            if (s_edge_dbg)
                printf("->too_close\n");
            /* 间距过近 → 侧线+中线。贴边线(外侧出画, 无法阈值校验)视为中线;
               有 中线+有效边线 时推断缺失的第3条边线 (梯度佐证, 用户规则) */
            int le = line_edge_status(&s_lines[bi], -1);
            int re = line_edge_status(&s_lines[bj], +1);
            /* 贴边一致性 (用户 2026-08-07 晚: 太靠边用配对/先验滤掉):
               R 有效边线但贴左边框(外侧出画/在地面) + B 在画面内 → R 是边框伪线
               (真宽桥 B 会出画; 伪 R 落在桥外地面/边框)。只出 B。
               注意: 不做镜像的"B 贴右边框→只出 R", 用户数据里 B 全部正确。 */
            if (le == 1 && s_lines[bi].f.a * Y_REF + s_lines[bi].f.b < EDGE_MARGIN &&
                s_lines[bj].f.a * Y_REF + s_lines[bj].f.b < W - EDGE_MARGIN) {
                *ib = bj;              /* R 是边框伪线 → 只出 B */
                return BRIDGE_MODE_B;
            }
            if (le != 1) {              /* bi 非有效左边界 → 中线 */
                *ig = bi;
                *ib = bj;
                if (le == -1 && re == 1) {   /* bi 贴边 + bj 有效右界 → 推断红 */
                    float a, b;
                    if (n < MAX_LINES &&
                        infer_side_line(&s_lines[bi], &s_lines[bj], &a, &b)) {
                        memset(&s_lines[n], 0, sizeof(s_lines[n]));
                        s_lines[n].f.a = a;
                        s_lines[n].f.b = b;
                        s_lines[n].f.n = 1;
                        *ir = n;
                    }
                }
                return BRIDGE_MODE_MB;
            }
            if (re != 1) {              /* bj 非有效右边界 → 中线 */
                *ir = bi;
                *ig = bj;
                if (re == -1 && le == 1) {   /* bj 贴边 + bi 有效左界 → 推断蓝 */
                    float a, b;
                    if (n < MAX_LINES &&
                        infer_side_line(&s_lines[bj], &s_lines[bi], &a, &b)) {
                        memset(&s_lines[n], 0, sizeof(s_lines[n]));
                        s_lines[n].f.a = a;
                        s_lines[n].f.b = b;
                        s_lines[n].f.n = 1;
                        *ib = n;
                    }
                }
                return BRIDGE_MODE_RM;
            }
            *ir = bi;
            *ib = bj;
            return BRIDGE_MODE_RB_Q;    /* 判不出, 保守当红蓝 */
        }
        if (prior <= 0 && bs < MIN_SPACING)
            return BRIDGE_MODE_NONE;
        /* 边线差分校验: 把误配成边线的中线剔除 (外侧亮/不暗者即中线) */
        {
            int le = line_edge_status(&s_lines[bi], -1);   /* -1=贴边 0=中线 1=边线 */
            int re = line_edge_status(&s_lines[bj], +1);
            /* 贴边一致性 (用户 2026-08-07 晚: 太靠边用配对/先验滤掉):
               一边贴边(外侧出画无法校验)但另一边在画面内 → 该贴边线是边框伪线
               (真宽桥两边都应贴/出画; 伪线多落在桥外地面/边框)。只输出有效边。 */
            if (le == -1 && s_lines[bi].f.a * Y_REF + s_lines[bi].f.b < EDGE_MARGIN &&
                s_lines[bj].f.a * Y_REF + s_lines[bj].f.b < W - EDGE_MARGIN) {
                *ib = bj;              /* R 是边框伪线 → 只出 B */
                return BRIDGE_MODE_B;
            }
            if (!le && re) {            /* bi 不是左边界 → 是中线 */
                *ig = bi;
                *ib = bj;
                return BRIDGE_MODE_MB;
            }
            if (le && !re) {            /* bj 不是右边界 → 是中线 */
                *ir = bi;
                *ig = bj;
                return BRIDGE_MODE_RM;
            }
            if (!le && !re)             /* 都不是边线: 不输出错配的"边线" */
                return BRIDGE_MODE_NONE;
        }
        if (s_edge_dbg)
            printf("->normal RB\n");
        *ir = bi;
        *ib = bj;
        *sp_out = bs;
        for (j = bi + 1; j < bj; j++) {
            if (mid_geo_ok(&s_lines[bi], &s_lines[j], &s_lines[bj])) {
                *ig = j;
                break;
            }
        }
        return (*ig >= 0) ? BRIDGE_MODE_RMB : BRIDGE_MODE_RB;
    }
}

/* ================================ 亮度阈值 ================================ */
/* 全图 Otsu */
static int otsu_hist(const uint16_t *hist)
{
    uint32_t total = 0, sum = 0, wb = 0, sb = 0;
    float best = -1;
    int i, thr = 0;
    for (i = 0; i < 256; i++) {
        total += hist[i];
        sum += (uint32_t)i * hist[i];
    }
    for (i = 0; i < 256; i++) {
        float num, den, s2;
        wb += hist[i];
        if (!wb || wb == total)
            continue;
        sb += (uint32_t)i * hist[i];
        num = (float)total * sb - (float)sum * wb;
        den = (float)wb * (float)(total - wb);
        s2 = num * num / (den + 1e-12f);
        if (s2 > best) {
            best = s2;
            thr = i;
        }
    }
    return thr;
}

/* 全图 Otsu 最大类间方差 (双峰可分强度, 用户定案无桥面滤除用) */
static float otsu_bcv_hist(const uint16_t *hist)
{
    uint32_t total = 0, sum = 0, wb = 0, sb = 0;
    float best = -1;
    int i;
    for (i = 0; i < 256; i++) {
        total += hist[i];
        sum += (uint32_t)i * hist[i];
    }
    for (i = 0; i < 256; i++) {
        float num, den, s2;
        wb += hist[i];
        if (!wb || wb == total)
            continue;
        sb += (uint32_t)i * hist[i];
        num = (float)total * sb - (float)sum * wb;
        den = (float)wb * (float)(total - wb);
        s2 = num * num / (den + 1e-12f);
        if (s2 > best)
            best = s2;
    }
    return best < 0 ? 0.0f : best;
}

/* 全图直方图缓存: Otsu / bimodal_midref / bcv_global 共用一次扫描
   (2026-08-13, 8ms 达标: 省 2 次 H*W 像素扫描) */
static uint16_t s_hist[256];
static int      s_hist_ready;

ITCM_FUNC static void build_hist(void)
{
    int y, x;
    if (s_hist_ready)
        return;
    memset(s_hist, 0, sizeof(s_hist));
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
            s_hist[s_img[y][x]]++;
    s_hist_ready = 1;
}

static float bcv_global(void)
{
    build_hist();
    return otsu_bcv_hist(s_hist);
}

/* ========================================================================
 * v11 结束线 (2026-08-13 用户定案, 对齐 pc_tools/bridge_v11.py)
 *   取消 MLP: 包络内逐行 gy>V11_GY_THR run → 跨行 x 重叠 union 成连通域
 *   → 连通域整体贯通(最左到左边界/最右到右边界) → 逐列 gy 峰值拟合斜顶边
 * ======================================================================== */

/* 错误线条驳回 (提取自 v8 结束线 line_angle 内联逻辑):
   ① 红蓝交点 y_i = (bB-bR)/(aR-aB) >= V8_INT_Y → 结构错误
   ② 有向夹角 θ_s = atan2(aR-aB, 1+aR·aB) > 0 → 向下汇聚 → 结构错误 */
static int line_angle_ok(const bridge_line_t *lf, const bridge_line_t *rf)
{
    float a1 = lf->a, b1 = lf->b, a2 = rf->a, b2 = rf->b;
    float da = a1 - a2;
    if (da < -1e-6f || da > 1e-6f) {
        float yi = (b2 - b1) / da;
        if (yi >= V8_INT_Y)
            return 0;
    }
    if (atan2f(a1 - a2, 1.0f + a1 * a2) > 0.0f)
        return 0;
    return 1;
}

/* banker's rounding (round half to even), 对齐 Python round */
static int rnd_he(float x)
{
    int i = (int)x;
    float f = x - i;
    if (f == 0.5f)
        return (i & 1) ? i + 1 : i;
    if (f == -0.5f)
        return (i & 1) ? i - 1 : i;
    return (int)(x + (x >= 0.0f ? 0.5f : -0.5f));
}

/* 包络边界 (缺侧边线 → x=V11_EDGE_MARGIN / W-1-MARGIN), 对齐 bridge_v11.edge_bounds */
static void v11_edge_bounds(float y, const bridge_line_t *lf,
                            const bridge_line_t *rf, int *bx0, int *bx1)
{
    float xl = lf ? lf->a * y + lf->b : 0.0f;
    float xr = rf ? rf->a * y + rf->b : 0.0f;
    int m = V11_EDGE_MARGIN, x0, x1;
    if (!lf && !rf) { x0 = m; x1 = W - 1 - m; }
    else if (!lf)   { x0 = m; x1 = rnd_he(xr); }
    else if (!rf)   { x0 = rnd_he(xl); x1 = W - 1 - m; }
    else {
        float lo = xl < xr ? xl : xr, hi = xl > xr ? xl : xr;
        x0 = rnd_he(lo); x1 = rnd_he(hi);
    }
    if (x0 < 0) x0 = 0;
    if (x0 > W - 1) x0 = W - 1;
    if (x1 < 0) x1 = 0;
    if (x1 > W - 1) x1 = W - 1;
    if (x1 < x0) { x0 = m; x1 = W - 1 - m; }
    *bx0 = x0; *bx1 = x1;
}

/* 并查集 find (路径压缩) */
static int16_t v11_find(int16_t *p, int16_t x)
{
    while (p[x] != x) {
        p[x] = p[p[x]];
        x = p[x];
    }
    return x;
}

static void v11_union(int16_t *p, int16_t a, int16_t b)
{
    int16_t ra = v11_find(p, a), rb = v11_find(p, b);
    if (ra != rb)
        p[rb] = ra;
}

/* v11 结束线主函数: 成功返回 1 并填 tf (横线 y=a·x+b) */
static int v11_top_gy(const bridge_line_t *lf, const bridge_line_t *rf,
                      bridge_line_t *tf)
{
    int nruns = 0, yg, i, j;
    /* ---- 阶段A: 逐行 run 提取 ---- */
    for (yg = 0; yg < GH; yg++) {
        int bx0, bx1, c0, c1, c;
        v11_edge_bounds(yg + 1.5f, lf, rf, &bx0, &bx1);
        c0 = rnd_he(bx0 - 1.5f);
        c1 = rnd_he(bx1 - 1.5f);
        if (c0 < 0) c0 = 0;
        if (c0 > GW - 1) c0 = GW - 1;
        if (c1 < 0) c1 = 0;
        if (c1 > GW - 1) c1 = GW - 1;
        s_v11_bounds[yg][0] = (int16_t)c0;
        s_v11_bounds[yg][1] = (int16_t)c1;
        s_v11_row_n[yg] = 0;
        if (c1 >= c0) {
            for (c = c0; c <= c1; c++) {
                if (s_gy[yg][c] > V11_GY_THR) {
                    int c2 = c;
                    while (c2 + 1 <= c1 && s_gy[yg][c2 + 1] > V11_GY_THR)
                        c2++;
                    if (nruns < V11_MAX_RUNS &&
                        s_v11_row_n[yg] < V11_MAX_ROW_RUNS) {
                        s_v11_row_runs[yg][s_v11_row_n[yg]++] = (int16_t)nruns;
                        s_v11_runs[nruns].yg = (int16_t)yg;
                        s_v11_runs[nruns].x0 = (int16_t)c;
                        s_v11_runs[nruns].x1 = (int16_t)c2;
                        nruns++;
                    }
                    c = c2;
                }
            }
        }
    }
    if (!nruns)
        return 0;
    /* ---- 阶段C+D: 跨行 x 重叠 → union ---- */
    for (i = 0; i < nruns; i++)
        s_v11_parent[i] = (int16_t)i;
    for (yg = 1; yg < GH; yg++) {
        for (i = 0; i < s_v11_row_n[yg]; i++) {
            int ia = s_v11_row_runs[yg][i];
            int a0 = s_v11_runs[ia].x0, a1 = s_v11_runs[ia].x1;
            for (j = 0; j < s_v11_row_n[yg - 1]; j++) {
                int ib = s_v11_row_runs[yg - 1][j];
                int b0 = s_v11_runs[ib].x0, b1 = s_v11_runs[ib].x1;
                if (a0 <= b1 && b0 <= a1)
                    v11_union(s_v11_parent, (int16_t)ia, (int16_t)ib);
            }
        }
    }
    /* ---- 阶段F: 连通域贯通判定 (跨行累计 bbox) ---- */
    {
        int best_ymx = -1, best_ymn = 0, best_xmn = 0, best_xmx = 0;
        int best_ok = 0;
        for (i = 0; i < nruns; i++) {
            s_v11_cxmin[i] = s_v11_cxmax[i] = s_v11_cymin[i] =
                s_v11_cymax[i] = -1;
        }
        for (i = 0; i < nruns; i++) {
            int16_t r = v11_find(s_v11_parent, (int16_t)i);
            if (s_v11_cxmin[r] < 0) {
                s_v11_cxmin[r] = s_v11_runs[i].x0;
                s_v11_cxmax[r] = s_v11_runs[i].x1;
                s_v11_cymin[r] = s_v11_runs[i].yg;
                s_v11_cymax[r] = s_v11_runs[i].yg;
            } else {
                if (s_v11_runs[i].x0 < s_v11_cxmin[r]) s_v11_cxmin[r] = s_v11_runs[i].x0;
                if (s_v11_runs[i].x1 > s_v11_cxmax[r]) s_v11_cxmax[r] = s_v11_runs[i].x1;
                if (s_v11_runs[i].yg < s_v11_cymin[r]) s_v11_cymin[r] = s_v11_runs[i].yg;
                if (s_v11_runs[i].yg > s_v11_cymax[r]) s_v11_cymax[r] = s_v11_runs[i].yg;
            }
        }
        for (i = 0; i < nruns; i++) {
            if (s_v11_cxmin[i] < 0)
                continue;
            {
                int L = GW - 1, R = 0, y2;
                for (y2 = s_v11_cymin[i]; y2 <= s_v11_cymax[i]; y2++) {
                    if (s_v11_bounds[y2][0] < L) L = s_v11_bounds[y2][0];
                    if (s_v11_bounds[y2][1] > R) R = s_v11_bounds[y2][1];
                }
                if (s_v11_cxmin[i] <= L + V11_LEFT_TOL &&
                    s_v11_cxmax[i] >= R - V11_SPAN_TOL) {
                    if (!best_ok || s_v11_cymax[i] > best_ymx) {
                        best_ok = 1;
                        best_ymx = s_v11_cymax[i];
                        best_ymn = s_v11_cymin[i];
                        best_xmn = s_v11_cxmin[i];
                        best_xmx = s_v11_cxmax[i];
                    }
                }
            }
        }
        if (!best_ok)
            return 0;
        /* ---- 阶段G: 逐列 gy 峰值拟合 ---- */
        {
            int m = 0;
            for (j = best_xmn; j <= best_xmx && j < GW; j++) {
                int k = best_ymn, k2;
                for (k2 = best_ymn + 1; k2 <= best_ymx; k2++)
                    if (s_gy[k2][j] > s_gy[k][j])
                        k = k2;
                if (s_gy[k][j] > V11_GY_THR && m < GW) {
                    s_v11_pts[m].u = j + 1.5f;
                    s_v11_pts[m].v = (float)k + 1.5f;   /* k 已是绝对行号 */
                    s_v11_pts[m].w = 1.0f;
                    m++;
                }
            }
            if (m <= 0)
                return 0;
            {
                float sw = 0, su = 0, sv = 0, suu = 0, suv = 0, a, b;
                for (i = 0; i < m; i++) {
                    float u = s_v11_pts[i].u, v = s_v11_pts[i].v;
                    sw += 1.0f; su += u; sv += v; suu += u * u; suv += u * v;
                }
                if (m >= 3) {
                    float den = sw * suu - su * su;
                    if (den < 1e-9f && den > -1e-9f) den = 1e-9f;
                    a = (sw * suv - su * sv) / den;
                    b = (sv - a * su) / sw;
                } else {
                    a = 0.0f;
                    b = sv / sw;
                }
                if (a > V11_SLOPE_MAX || a < -V11_SLOPE_MAX || m < 3) {
                    a = 0.0f;
                    for (i = 0; i < m; i++)
                        s_v11_y[i] = s_v11_pts[i].v;
                    for (i = 0; i < m; i++)
                        for (j = i + 1; j < m; j++)
                            if (s_v11_y[j] < s_v11_y[i]) {
                                float t = s_v11_y[i];
                                s_v11_y[i] = s_v11_y[j];
                                s_v11_y[j] = t;
                            }
                    b = (m & 1) ? s_v11_y[m / 2] :
                        (s_v11_y[m / 2 - 1] + s_v11_y[m / 2]) * 0.5f;
                }
                /* TOP_FAR 上方稳定白门控 (对齐 v11) */
                {
                    int near = 0, tot = 0, xx;
                    for (xx = 0; xx < W; xx++) {
                        float yt = a * xx + b;
                        int y0 = (int)(yt - TOP_FAR_HI);
                        int y1 = (int)(yt - TOP_FAR_LO);
                        int yy;
                        if (y0 < 0) y0 = 0;
                        if (y1 <= y0 || y1 > H) continue;
                        for (yy = y0; yy < y1; yy++)
                            if (s_img[yy][xx] > TOP_FAR_THR) near++;
                        tot += y1 - y0;
                    }
                    if (tot > 40 && (float)near / (float)tot > TOP_FAR_WHITE)
                        return 0;
                }
                tf->a = a;
                tf->b = b;
                tf->n = (int16_t)m;
                tf->rms = 0.0f;
                tf->u_lo = tf->u_hi = 0;
                return 1;
            }
        }
    }
}

static int otsu_img(void)
{
    build_hist();
    return otsu_hist(s_hist);
}

/* 红蓝包裹区内像素的 Otsu (暗块 vs 亮桥面 区内双峰); 样本不足退回全局 */
static int inner_threshold(const iline_t *lf, const iline_t *rf, int tb)
{
    uint16_t hist[256];
    uint32_t n = 0;
    int y, i;
    memset(hist, 0, sizeof(hist));
    for (y = 2; y < H; y += 2) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0 = (int)(xl < xr ? xl : xr) + 2;
        int x1 = (int)(xl > xr ? xl : xr) - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (i = x0; i <= x1; i++) {
            hist[s_img[y][i]]++;
            n++;
        }
    }
    if (n < 200)
        return tb;
    return otsu_hist(hist);
}

/* 亮度双峰分界 mid_ref (与 PC bridge_v7.bimodal_ref + mid_ref 一致):
   全图直方图平滑后取 top2 峰, 分离>=40 且次峰>=5%主峰 → (lo+hi)/2;
   无双峰 → 全图中位数 (PC: np.percentile(gray,50))。
   2026-08-12: MLP 重训特征 tb 用 mid_ref, 与训练/PC v7 一致。 */
#define BIMODAL_SEP_C  40
static int bimodal_midref(void)
{
    float h[256];
    int i, x;
    float mx;
    build_hist();
    for (i = 0; i < 256; i++)
        h[i] = (float)s_hist[i];
    /* 高斯近似平滑 (sigma~3: [1,2,1]/4 两次) */
    for (i = 0; i < 2; i++) {
        float g[256];
        for (x = 0; x < 256; x++)
            g[x] = h[x];
        for (x = 0; x < 256; x++) {
            float s = g[x] * 2.0f;
            if (x > 0) s += g[x - 1];
            if (x < 255) s += g[x + 1];
            h[x] = s * 0.25f;
        }
    }
    mx = h[0];
    for (i = 1; i < 256; i++)
        if (h[i] > mx) mx = h[i];
    {
        int p0 = -1, p1 = -1;           /* 峰亮度 (top2) */
        float v0 = 0, v1 = 0;
        for (i = 2; i < 254; i++) {
            if (h[i] >= h[i - 1] && h[i] > h[i + 1] && h[i] > 0.02f * mx) {
                if (h[i] > v0) {
                    v1 = v0; p1 = p0; v0 = h[i]; p0 = i;
                } else if (h[i] > v1) {
                    v1 = h[i]; p1 = i;
                }
            }
        }
        if (p0 >= 0 && p1 >= 0) {
            int lo = p0 < p1 ? p0 : p1;
            int hi = p0 < p1 ? p1 : p0;
            if (hi - lo >= BIMODAL_SEP_C && v1 >= 0.05f * mx)
                return (lo + hi) / 2;
        }
    }
    /* 中位数 (近似 percentile 50) */
    {
        uint32_t total = 0, half, acc = 0;
        for (i = 0; i < 256; i++)
            total += s_hist[i];
        half = (total + 1) / 2;
        for (i = 0; i < 256; i++) {
            acc += s_hist[i];
            if (acc >= half)
                return i;
        }
        return 128;
    }
}

#if TOP_GRAD
/* ================================ 粉线校验 (旧梯度法, TOP_GRAD=1 时编译) ================================ */
/* 区域亮度一致性 (top 带: 亮 1..3, 暗 -4..-1): 亮带p25 - 暗带p75 > delta/2 */
static int region_ok_top(float a, float b, float delta)
{
    static uint8_t bv[256], dv[256];
    int nb = 0, nd = 0, oob = 0, tot = 0, x, off;
    for (x = 1; x < W - 1; x += 2) {
        float yf = a * x + b;
        int yr = (int)(yf + 0.5f);
        for (off = 1; off <= 3; off++) {
            int yi = yr + off;
            tot++;
            if (yi >= 0 && yi < H)
                bv[nb++] = s_img[yi][x];
            else
                oob++;
        }
        for (off = -4; off <= -1; off++) {
            int yi = yr + off;
            if (yi >= 0 && yi < H)
                dv[nd++] = s_img[yi][x];
        }
    }
    if (tot && oob * 10 > tot * 3)
        return 1;
    if (!nb || !nd)
        return 1;
    qsort(bv, (size_t)nb, 1, cmp_u8);
    qsort(dv, (size_t)nd, 1, cmp_u8);
    return (float)bv[nb / 4] - (float)dv[(nd * 3) / 4] > delta * 0.5f;
}

/* 线上方带 (左右线之间) 亮比例 < 0.5 (上方不应是桥面) */
static int bright_ok_top(float a, float b,
                         const iline_t *lf, const iline_t *rf, int tb)
{
    int br = 0, tot = 0, x, off;
    for (x = 2; x < W - 2; x += 2) {
        float yf = a * x + b;
        int yr = (int)(yf + 0.5f);
        for (off = -10; off <= -3; off++) {
            int yi = yr + off;
            float xl, xr;
            if (yi < 0 || yi >= H)
                continue;
            xl = lf->f.a * yi + lf->f.b;
            xr = rf->f.a * yi + rf->f.b;
            if (x >= xl - 2 && x <= xr + 2) {
                br += s_img[yi][x] > tb;
                tot++;
            }
        }
    }
    return tot == 0 || br * 2 < tot;
}

/* 线间灰度带: 均值 + 亮比例 */
static void band_gray(int y0, int y1,
                      const iline_t *lf, const iline_t *rf, int tb,
                      float *mean, float *frac, int *cnt)
{
    int y, x, n = 0, br = 0;
    float sum = 0;
    if (y0 < 0)
        y0 = 0;
    if (y1 > H)
        y1 = H;
    for (y = y0; y < y1; y++) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0 = (int)ceilf(xl < xr ? xl : xr) + 2;
        int x1 = (int)floorf(xl > xr ? xl : xr) - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (x = x0; x <= x1; x++) {
            sum += s_img[y][x];
            br += s_img[y][x] > tb;
            n++;
        }
    }
    *mean = n ? sum / n : 0;
    *frac = n ? (float)br / n : 0;
    *cnt = n;
}

/* 剖面否决: 上下带同亮且灰度接近 → 同一桥面, 伪顶 */
static int profile_ok_top(float a, float b,
                          const iline_t *lf, const iline_t *rf, int tb)
{
    int y_l = (int)(a * (W * 0.5f) + b + 0.5f);
    float ma, fa, mb, fb;
    int ca, cb;
    band_gray(y_l - 6, y_l - 1, lf, rf, tb, &ma, &fa, &ca);
    band_gray(y_l + 1, y_l + 6, lf, rf, tb, &mb, &fb, &cb);
    if (!ca || !cb)
        return 1;
    return !(fa > 0.6f && fb > 0.6f && fabsf(ma - mb) < 25.0f);
}

/* 禁止横穿亮区: 红蓝跨度内沿线采样, 上下 4px 皆亮比例 > 0.4 → 否决 */
static int crosses_bright(float a, float b,
                          const iline_t *lf, const iline_t *rf, int tb_in)
{
    int both = 0, tot = 0, x;
    for (x = 0; x < W; x += 2) {
        float yf = a * x + b;
        int yi = (int)yf;
        float xl, xr;
        if (yi < 4 || yi >= H - 4)
            continue;
        xl = lf->f.a * yf + lf->f.b;
        xr = rf->f.a * yf + rf->f.b;
        if (x <= xl || x >= xr)
            continue;
        both += (s_img[yi - 4][x] > tb_in) && (s_img[yi + 4][x] > tb_in);
        tot++;
    }
    return tot >= 6 && both * 10 > tot * 4;
}

/* 角点结构: 与两侧线交点须在画面内; 尖端在画面附近时角点须低于尖端 */
static int top_corners_ok(float a, float b,
                          const iline_t *lf, const iline_t *rf)
{
    const iline_t *side[2] = { lf, rf };
    float ymin = 1e9f, dl, vy;
    int t;
    for (t = 0; t < 2; t++) {
        float den = 1.0f - side[t]->f.a * a;
        float x, y;
        if (den < 1e-3f && den > -1e-3f)
            return 0;                       /* 与侧线平行/共线 */
        x = (side[t]->f.a * b + side[t]->f.b) / den;
        y = a * x + b;
        if (x < -3 || x > W + 3 || y < -3 || y > H + 3)
            return 0;
        if (y < ymin)
            ymin = y;
    }
    dl = lf->f.a - rf->f.a;
    if (dl > 1e-6f || dl < -1e-6f) {
        vy = (rf->f.b - lf->f.b) / dl;      /* 红蓝尖端 y */
        if (vy > -20.0f && ymin < vy + 3.0f)
            return 0;                       /* 线穿了尖端 */
    }
    return 1;
}

/* 逐行亮比例 (低通) 最长亮段顶行, 无可靠亮段返回 -1 */
static int bright_run_top(const iline_t *lf, const iline_t *rf, int tb_in)
{
    float prof[H], sm[H];
    int y, x, best_len = 0, best_top = -1, run = -1;
    for (y = 0; y < H; y++) {
        float xl = lf->f.a * y + lf->f.b;
        float xr = rf->f.a * y + rf->f.b;
        int x0, x1, br = 0, n = 0;
        prof[y] = 0;
        if (xr - xl < 6)
            continue;                       /* 消失点上方: 不在桥面 */
        x0 = (int)xl + 2;
        x1 = (int)xr - 2;
        if (x0 < 0)
            x0 = 0;
        if (x1 > W - 1)
            x1 = W - 1;
        for (x = x0; x <= x1; x++) {
            br += s_img[y][x] > tb_in;
            n++;
        }
        if (n >= 2)
            prof[y] = (float)br / n;
    }
    for (y = 0; y < H; y++) {               /* 5 点滑动平均低通 */
        float s = 0;
        int k;
        for (k = -2; k <= 2; k++) {
            if (y + k >= 0 && y + k < H)
                s += prof[y + k];
        }
        sm[y] = s * 0.2f;
    }
    for (y = 0; y <= H; y++) {
        int on = (y < H) && sm[y] > 0.5f;
        if (on && run < 0)
            run = y;
        else if (!on && run >= 0) {
            if (y - run > best_len) {
                best_len = y - run;
                best_top = run;
            }
            run = -1;
        }
    }
    return best_len >= 8 ? best_top : -1;
}

/* ================================ 顶线提取 ================================ */
/* 支撑端点: 内点自变量 pct 分位 → (x_end, y_end) */
static void support_end(const iline_t *L, int pct, float *xe, float *ye)
{
    int m = L->inl_n, i = (m * pct) / 100;
    memcpy(s_sort, L->inl_u, (size_t)m * sizeof(float));
    qsort(s_sort, (size_t)m, sizeof(float), cmp_f32);
    *ye = s_sort[i];
    *xe = L->f.a * s_sort[i] + L->f.b;
}

/* 期望 y(x): 由左右线支撑端点 (10 分位) 插值; 返回锚点数 (0=不可用) */
static int make_yexp(const iline_t *lf, const iline_t *rf,
                     float *x1, float *y1, float *x2, float *y2)
{
    int n = 0;
    if (lf->f.n >= 8) {
        support_end(lf, 10, x1, y1);
        n++;
    }
    if (rf->f.n >= 8) {
        if (n == 0)
            support_end(rf, 10, x1, y1);
        else
            support_end(rf, 10, x2, y2);
        n++;
    }
    return n;
}

static float yexp_at(float x, int nanc,
                     float x1, float y1, float x2, float y2)
{
    if (nanc == 1)
        return y1;
    if (x2 - x1 < 2 && x1 - x2 < 2)
        return (y1 + y2) * 0.5f;
    return y1 + (y2 - y1) * (x - x1) / (x2 - x1);
}
#endif /* TOP_GRAD */

#if !TOP_GRAD
/* ================================ 脱出线 (亮区顶边界法) ================================ */
/* 与 pc_tools/bridge_v6.py top_from_bright 一致:
   门控行亮像素为种子做 4-连通 BFS (限红蓝包络内) 得桥面亮区,
   逐列顶边 -> 平台 (top<=min_top+tol 最长连续列段) -> 中点精化 -> RANSAC。
   先验: 线上方包络内应全暗; 亮区贴画面顶或上方亮比例高则否决。   */

#define REG_SET(y, x)   (s_region[y][(x) >> 5] |=  (1u << ((x) & 31)))
#define REG_GET(y, x)   (s_region[y][(x) >> 5] &   (1u << ((x) & 31)))

static int extract_top_region(const iline_t *lf, const iline_t *rf,
                              int tb_in, bridge_line_t *tf)
{
    int y, x, i, k, head, tail, ncol, min_top, tol;
    int m = 0, nin, nn;
    float a, b, rms, span;

    /* 逐行包络 (扩张限界 ±3) */
    for (y = 0; y < H; y++) {
        float xl = lf->f.a * y + lf->f.b - 3.0f;
        float xr = rf->f.a * y + rf->f.b + 3.0f;
        s_env_lo[y] = (int16_t)(xl > 0 ? (int)xl : 0);
        s_env_hi[y] = (int16_t)(xr < W - 1 ? (int)xr : W - 1);
    }

    /* 1) BFS: 门控行包络内亮像素为种子 */
    memset(s_region, 0, sizeof(s_region));
    head = tail = 0;
    for (y = GATE_ROWS; y < H; y++) {
        for (x = s_env_lo[y]; x <= s_env_hi[y]; x++) {
            if (s_img[y][x] > tb_in && !REG_GET(y, x)) {
                REG_SET(y, x);
                s_bfs_q[tail++] = (uint16_t)(y * W + x);
            }
        }
    }
    while (head < tail) {
        int p = s_bfs_q[head++];
        static const int8_t d4[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
        int cy = p / W, cx = p % W;
        for (k = 0; k < 4; k++) {
            int yy = cy + d4[k][0], xx = cx + d4[k][1];
            if (yy < 0 || yy >= H || xx < s_env_lo[yy] || xx > s_env_hi[yy])
                continue;
            if (s_img[yy][xx] > tb_in && !REG_GET(yy, xx)) {
                REG_SET(yy, xx);
                s_bfs_q[tail++] = (uint16_t)(yy * W + xx);
            }
        }
    }

    /* 2) 逐列顶边 + 平台 (top <= min_top+tol 的最长连续列段, tol 0..3) */
    ncol = 0;
    min_top = H;
    for (x = 0; x < W; x++) {
        s_col_top[x] = -1;
        for (y = 0; y < H; y++) {
            if (REG_GET(y, x)) {
                s_col_top[x] = (int8_t)y;
                if (y < min_top)
                    min_top = y;
                break;
            }
        }
        ncol += (s_col_top[x] >= 0);
    }
    if (ncol < TOP_MIN_PTS || min_top <= 1)
        return 0;                           /* 列不足 / 亮区贴画面顶 */

    for (tol = 0; tol <= 3 && m == 0; tol++) {
        int run_start = -1, best_len = 0, best_s = -1, best_e = -1;
        for (i = 0; i <= W; i++) {
            int ok = (i < W) && s_col_top[i] >= 0 &&
                     s_col_top[i] <= min_top + tol;
            if (ok && run_start < 0)
                run_start = i;
            if (!ok && run_start >= 0) {
                if (i - run_start > best_len) {
                    best_len = i - run_start;
                    best_s = run_start;
                    best_e = i - 1;
                }
                run_start = -1;
            }
        }
        if (best_len >= TOP_MIN_PTS) {
            /* 2.5) 中点精化: 平台列顶边按局部暗/亮均值中点回扫 */
            for (x = best_s; x <= best_e; x++) {
                int t = s_col_top[x], sd = 0, sb = 0, nd = 0, nb = 0, mid, yy;
                for (yy = t - 6; yy <= t - 2; yy++) {
                    if (yy >= 0) {
                        sd += s_img[yy][x];
                        nd++;
                    }
                }
                for (yy = t + 1; yy <= t + 5; yy++) {
                    if (yy < H) {
                        sb += s_img[yy][x];
                        nb++;
                    }
                }
                if (nd < 2 || nb < 3) {
                    yy = t;
                } else {
                    mid = (sd / nd + sb / nb) / 2;
                    yy = t;
                    while (yy - 1 >= 0 && s_img[yy - 1][x] > mid)
                        yy--;
                }
                s_rem[m].u = x + 0.5f;
                s_rem[m].v = (float)yy;
                s_rem[m].w = 1.0f;
                m++;
            }
        }
    }
    if (m == 0)
        return 0;

    /* 3) 抗噪拟合 */
    nin = ransac_best(s_rem, m, TOP_SLOPE, &a, &b, s_mask);
    if (!nin)
        return 0;
    rms = refit(s_rem, s_mask, m, &a, &b, &nn);
    if (nn < TOP_MIN_PTS)
        return 0;
    {
        float xlo = 1e9f, xhi = -1e9f;
        for (i = 0; i < m; i++) {
            if (s_mask[i]) {
                if (s_rem[i].u < xlo)
                    xlo = s_rem[i].u;
                if (s_rem[i].u > xhi)
                    xhi = s_rem[i].u;
            }
        }
        span = xhi - xlo;
    }
    if (span < TOP_MIN_SPAN)
        return 0;

    /* 4) 先验校验: 线上方 (包络内, 跳过 ±2px 过渡带) 须暗, 线下方须亮 */
    {
        int above = 0, nab = 0, below = 0, nbl = 0;
        for (x = 2; x < W - 2; x += 2) {
            float yf = a * x + b;
            int yi = (int)yf, yy;
            if (!(lf->f.a * yf + lf->f.b < x && x < rf->f.a * yf + rf->f.b))
                continue;
            for (yy = yi - 8; yy <= yi - 3; yy++) {
                if (yy >= 0 && yy < H &&
                    lf->f.a * yy + lf->f.b < x && x < rf->f.a * yy + rf->f.b) {
                    above += s_img[yy][x] > tb_in;
                    nab++;
                }
            }
            for (yy = yi + 2; yy <= yi + 6; yy++) {
                if (yy >= 0 && yy < H &&
                    lf->f.a * yy + lf->f.b < x && x < rf->f.a * yy + rf->f.b) {
                    below += s_img[yy][x] > tb_in;
                    nbl++;
                }
            }
        }
        if (nab < 10) {                 /* 上方包络过窄: 贴顶伪线否决 */
            if (a * (W * 0.5f) + b < 5.0f)
                return 0;
        } else if ((float)above / nab > TOP_ABOVE_MAX) {
            return 0;                   /* 线上方还有大量白色 -> 提取必有错 */
        }
        if (nbl >= 10 && (float)below / nbl < TOP_BELOW_MIN)
            return 0;
    }

    tf->a = a;
    tf->b = b;
    tf->n = (int16_t)nn;
    tf->rms = rms;
    tf->u_lo = tf->u_hi = 0;
    return 1;
}
#endif /* !TOP_GRAD */

/* ========================================================================
 * MLP 结束线 (行级 int8 推理) —— 替换亮区法 (与 pc_tools/bridge_mlp_end.py 一致)
 *
 * 管线: 23维行特征(int8) → MLP[23,10,5,1] per-channel int8 → 逐行 logit
 *      → 平滑+argmax+亚像素质心 → 存在性门控(logit>=-3 ≈ 概率0.30)
 *      → 包络内逐列亮暗穿越(下方持续亮/上方持续暗/gy极性) → 锚定RANSAC
 * ======================================================================== */
#if 0  /* ====== 原 MLP 结束线已删除 (2026-08-13 v11 替换) ====== */
#define MLP_LOGIT_THR   4         /* 存在性: 平滑logit (v9 thr=0.80,
                                     export_mlp_int8 标定 scale_last=0.367) */
#define MLP_WIN         6         /* 穿越点窗口 (图像行) */
#define MLP_MAX_DEV     2.0f      /* 穿越点距质心行最大偏差 */
#define MLP_SLOPE       0.15f     /* 结束线斜率上限 (近似水平) */
#define MLP_MIN_PTS     2         /* 含锚点后 <2 (即穿越点 0) 失败 */
#define MLP_PTS_MAX     128
#define MLP_D1          4       /* 无 blur 直接差分间隔1 (替代 B1 σ1.5 blur+diff) */
#define MLP_D2          8       /* 无 blur 直接差分间隔2 (替代 B2 σ3   blur+diff) */

/* ---- v9 16维行特征 (2026-08-13 特征银行挖掘定案, pc row_feats_v9) ----
   特征序 (必须与 mlp_end_weights_v9.json feature_names 一致):
   0 gyB1_mean  1 gyB1_pos  2 gyB2_mean  3 gyB2_pos  (多尺度模糊纵向差分)
   4 gyp_mean   5 gy_abs_mean  6 gy_pos_frac  7 gy_coh
   8 bmean_up10  9 brmid_up10  10 bmean_dn6  11 cc_edge
   12 y_norm  13 width_norm  14 trans_gy  15 dbr_up */
static float    s_mlp_br[H], s_mlp_bmean[H], s_mlp_wnorm[H];
static float    s_gb1m[H], s_gb1p[H], s_gb2m[H], s_gb2p[H];
static float    s_gyp[H], s_gya[H], s_gyf[H], s_gyc[H], s_cce[H];
static int      s_colmax_row[GW];
static float    s_colmax_val[GW];
/* ============ 无 blur 直接差分 (2026-08-13 用户: 不要模糊换一种方式) ============
   旧: 盒式模糊 x3 近似高斯 σ1.5+σ3 后相邻差分 (每像素 ~4 ops + 中间 buffer)。
   新: 直接差分 s_img[y]-s_img[y-D1/D2] (D1=4/D2=8, Python noblur_v9 标定),
       只读 s_img 两次, 无中间 buffer, blur 段耗时归零。
   行为由 host 复验锁 (66GT/6081 逐项对比)。 */
static uint8_t  s_imgT[W][H];           /* s_img 转置 (列访问→行连续,
                                            trans_gy/穿越点/top_edge 带区) */
/* 特征行 16 字节 (4 对齐) 供 SMLAD 汇编读 */
static int8_t   s_mlp_feat[GH][MLP_END_NF] __attribute__((aligned(4)));
static int16_t  s_mlp_logit[GH], s_mlp_logit_s[GH];
static bpt_t    s_mlp_pts[MLP_PTS_MAX];

ITCM_FUNC static void mlp_env(int y, const iline_t *lf, const iline_t *rf,
                    int *x0, int *x1)
{
    float xl, xr;
    if (lf) xl = lf->f.a * y + lf->f.b; else xl = 1;
    if (rf) xr = rf->f.a * y + rf->f.b; else xr = W - 2;
    if (!lf) { *x0 = 1; *x1 = (int)xr - 1; }
    else if (!rf) { *x0 = (int)xl + 1; *x1 = W - 2; }
    else { *x0 = (int)(xl < xr ? xl : xr) + 2;
           *x1 = (int)(xl < xr ? xr : xl) - 2; }
    if (*x0 < 0) *x0 = 0;
    if (*x1 > W - 1) *x1 = W - 1;
    if (*x1 <= *x0) { *x0 = W / 2 - 4; *x1 = W / 2 + 3; }
}

/* 特征 int8 量化: q = clip(round((f-center)*QSCALE/scale)) (同 v7)
   B3(2026-08-13): d/iscale 均 int32 值域, 显式 int32*int32→int64 (SMULL),
   避免 Debug 下 int64*int64 软乘 __aeabi_lmul; 逐位等价 (PC verify)。 */
static int8_t mlp_feat_quant(float f, int j)
{
    int32_t d  = (int32_t)(f * 32768.0f) - mlp_feat_center[j];
    int64_t q  = ((int64_t)d * mlp_feat_iscale[j] + (1LL << 29)) >> 30;
    if (q > 127) q = 127;
    if (q < -128) q = -128;
    return (int8_t)q;
}

static inline int ref_idx(int i, int n) /* scipy reflect (保留给 trans_gy 等) */
{
    if (i < 0) {
        i = -i;
        if (i >= n)
            i = 2 * (n - 1) - i;
    } else if (i >= n) {
        i = 2 * (n - 1) - i;
    }
    return i;
}

/* 行特征提取 (57 x 16 int8), 与 Python row_feats_v9 逐项对齐 */
ITCM_FUNC static void mlp_extract_feats(const iline_t *lf, const iline_t *rf,
                              int tb_in, int gate)
{
    int y, j, x0, x1, x, k, i;
    (void)gate;                     /* v9 无 gate 特征 */

    /* ---- 无 blur 多尺度直接差分 (2026-08-13 用户: 不要模糊换一种方式):
       gyB1 = s_img[y]-s_img[y-4], gyB2 = s_img[y]-s_img[y-8]
       (替代 B1=G1.5(img)/B2=G3(img) 模糊+差分; Python noblur_v9 验证
       66GT 等效 + 6081 对拍, 免去整段 blur 卷积) ---- */
    /* s_img 转置 (列访问循环换行连续读: trans_gy/穿越点/top_edge 带区) */
    for (y = 0; y < H; y++)
        for (x = 0; x < W; x++)
            s_imgT[x][y] = s_img[y][x];
    PROF_MARK();                            /* blur 起点 (slot5, 无 blur 恒零) */

    /* ---- 基础行统计 + gyB1 聚合 (直接差分 D1) ---- */
    for (y = 0; y < H; y++) {
        int n = 0; int64_t s = 0; int nb = 0;
        mlp_env(y, lf, rf, &x0, &x1);
        for (x = x0; x <= x1; x++) {
            int v = s_img[y][x];
            n++; s += v;
            if (v > tb_in) nb++;
        }
        if (n <= 0) n = 1;
        s_mlp_br[y] = (float)nb / (float)n;
        s_mlp_bmean[y] = (float)s / (float)n / 255.0f;
        s_mlp_wnorm[y] = (float)(x1 - x0 + 1) / (float)W;
        /* cc_edge: 最长亮段两端离包络边缘距离归一化 */
        {
            int best = 0, cur = 0, bst = -1, ben = -1, st = 0;
            int len = x1 - x0 + 1;
            for (i = 0; i <= len; i++) {
                int br = (i < len && s_img[y][x0 + i] > tb_in);
                if (br) { if (cur == 0) st = i; cur++; }
                else {
                    if (cur > best) { best = cur; bst = st; ben = i - 1; }
                    cur = 0;
                }
            }
            s_cce[y] = best > 0 ?
                1.0f - (float)(bst + (len - 1 - ben)) / (float)(len > 1 ? len : 1)
                : 0.0f;
        }
        /* gyB1 行聚合: 直接差分 s_img[y]-s_img[y-D1] (无 blur)
           B4(2026-08-13): 替代模糊后相邻差分; 免中间 buffer,
           只读 s_img 两次 (y 与 y-D1 行) */
        {
            int sd = 0, nd = 0, ng = 0;
            for (x = x0; x <= x1; x++) {
                int d = y >= MLP_D1 ? (int)s_img[y][x]
                                    - (int)s_img[y - MLP_D1][x] : 0;
                sd += d; nd++;
                if (d > 3) ng++;
            }
            if (nd <= 0) nd = 1;
            s_gb1m[y] = (float)sd / (float)nd / 30.0f;
            if (s_gb1m[y] < 0.0f) s_gb1m[y] = 0.0f;
            s_gb1p[y] = (float)ng / (float)nd;
        }
        /* gyB2 行聚合: 直接差分 s_img[y]-s_img[y-D2] (无 blur) */
        {
            int sd = 0, nd = 0, ng = 0;
            for (x = x0; x <= x1; x++) {
                int d = y >= MLP_D2 ? (int)s_img[y][x]
                                    - (int)s_img[y - MLP_D2][x] : 0;
                sd += d; nd++;
                if (d > 3) ng++;
            }
            if (nd <= 0) nd = 1;
            s_gb2m[y] = (float)sd / (float)nd / 30.0f;
            if (s_gb2m[y] < 0.0f) s_gb2m[y] = 0.0f;
            s_gb2p[y] = (float)ng / (float)nd;
        }
    }
    PROF_MARK();                                /* blur 终点 (slot6, 兼 feats 起点) */
    PROF_FE_BEGIN();                            /* feats 内部子计时起点 */

    /* ---- gy 列峰 (gy_coh 用): 每列 gy>0 最大行/值 ---- */
    for (j = 0; j < GW; j++) {
        int best = -1; float bv = 0.0f;
        for (y = 0; y < GH; y++) {
            int g = s_gy[y][j];
            if (g > 0 && (float)g > bv) { bv = (float)g; best = y; }
        }
        s_colmax_row[j] = best;
        s_colmax_val[j] = bv;
    }
    PROF_FE_MARK();                             /* fgypeak 终点 (gy 列峰) */
    /* ---- gy 行聚合 (包络裁到 GW 有效列)
           B2(2026-08-13): sp/sa int 累加 (sum<2^24 精确域, 逐位等价) ---- */
    for (y = 0; y < H; y++) {
        int n = 0, npos = 0;
        int sp = 0, sa = 0; int nc = 0;
        int yyg = (int)(y - 1.5f + 0.5f);
        if (yyg < 0) yyg = 0;
        if (yyg > GH - 1) yyg = GH - 1;
        mlp_env(y, lf, rf, &x0, &x1);
        {
            const int16_t *gyr = s_gy[yyg];
            for (x = x0; x <= x1; x++) {
                int xg = x < GW ? x : GW - 1;
                int g = gyr[xg], gg = g < 0 ? -g : g;
                if (g > 0) sp += g;
                sa += gg;
                if (g > 0) npos++;
                if (s_colmax_row[xg] >= 0 &&
                    abs(s_colmax_row[xg] - yyg) <= 2 &&
                    s_colmax_val[xg] > 200.0f)
                    nc++;
                n++;
            }
        }
        if (n <= 0) n = 1;
        s_gyp[y] = (float)sp / (float)n / 600.0f;
        s_gya[y] = (float)sa / (float)n / 600.0f;
        s_gyf[y] = (float)npos / (float)n;
        s_gyc[y] = (float)nc / (float)n;
    }
    PROF_FE_MARK();                             /* fgyagg 终点 (gy 行聚合) */

    for (y = 0; y < GH; y++) {
        int yi = (int)(y + 1.5f + 0.5f);
        float f[MLP_END_NF];
        if (yi > H - 1) yi = H - 1;
        mlp_env(yi, lf, rf, &x0, &x1);
        /* 上下文窗 */
        {
            float s1 = 0, s2 = 0, s3 = 0; int c1 = 0;
            for (k = yi - 10; k < yi; k++)
                if (k >= 0) { s1 += s_mlp_bmean[k]; s2 += s_mlp_br[k]; c1++; }
            f[8] = c1 ? s1 / c1 : 0.0f;     /* bmean_up10 */
            f[9] = c1 ? s2 / c1 : 0.0f;     /* brmid_up10 */
            s3 = 0.0f; c1 = 0;
            for (k = yi + 1; k <= yi + 6; k++)
                if (k < H) { s3 += s_mlp_bmean[k]; c1++; }
            f[10] = c1 ? s3 / c1 : 0.0f;    /* bmean_dn6 */
        }
        /* trans_gy: 逐列 ±3 窗分位中点穿越 (免排序)
           B1(2026-08-13): 只需 sorted[nseg/5](第a+1小) 与 sorted[4nseg/5]
           (第 nseg-b 大), 一遍维护 min1/min2/max1/max2 替代冒泡全排序;
           与原排序取值逐位一致 (PC verify_trans_gy 已验证)。 */
        {
            int nn = 0, ng = 0, yy;
            for (x = x0; x <= x1; x++) {
                int y_lo = yi - 3, y_hi = yi + 3, nseg = 0;
                int mn1 = 256, mn2 = 256, mx1 = -1, mx2 = -1, v;
                int mid2;
                if (y_lo < 1) y_lo = 1;
                if (y_hi > H - 3) y_hi = H - 3;
                for (yy = y_lo; yy <= y_hi; yy++) {
                    v = s_imgT[x][yy];          /* 转置: 行连续读 */
                    nseg++;
                    if (v < mn1) { mn2 = mn1; mn1 = v; }
                    else if (v < mn2) { mn2 = v; }
                    if (v > mx1) { mx2 = mx1; mx1 = v; }
                    else if (v > mx2) { mx2 = v; }
                }
                if (nseg < 4)
                    continue;
                {
                    int a = nseg / 5, b = nseg * 4 / 5;
                    int lo_v = (a == 0) ? mn1 : mn2;       /* 第 a+1 小 */
                    int hi_v = (nseg - b == 1) ? mx1 : mx2; /* 第 nseg-b 大 */
                    mid2 = lo_v + hi_v;                     /* 2*midv 整数 */
                }
                nn++;
                if (yi >= 1 && 2 * s_imgT[x][yi - 1] < mid2 &&
                    2 * s_imgT[x][yi] >= mid2) {
                    int gy_y = (int)(yi - 1.5f + 0.5f);
                    int gy_x = (int)(x - 1.5f + 0.5f);
                    if (gy_y >= 0 && gy_y < GH && gy_x >= 0 && gy_x < GW &&
                        s_gy[gy_y][gy_x] > 0)
                        ng++;
                }
            }
            f[14] = nn > 0 ? (float)ng / (float)nn : 0.0f;
        }
        f[0] = s_gb1m[yi];
        f[1] = s_gb1p[yi];
        f[2] = s_gb2m[yi];
        f[3] = s_gb2p[yi];
        f[4] = s_gyp[yi];
        f[5] = s_gya[yi];
        f[6] = s_gyf[yi];
        f[7] = s_gyc[yi];
        f[11] = s_cce[yi];
        f[12] = (float)y / (float)GH;
        f[13] = s_mlp_wnorm[yi];
        f[15] = s_mlp_br[yi] - (yi > 0 ? s_mlp_br[yi - 1] : 0.0f);
        for (j = 0; j < MLP_END_NF; j++)
            s_mlp_feat[y][j] = mlp_feat_quant(f[j], j);
    }
    PROF_FE_MARK();                             /* frow 终点 (行特征+量化) */
    PROF_MARK();                                /* feats 终点 (slot7, 兼 fit 起点) */
}

/* σ=8 双峰参考 (与 PC bridge_v8.bimodal_ref8 一致):
   直方图 σ8 高斯平滑 (r=32 exp 核, reflect 边界) → top2 峰 →
   峰距>=BIMODAL_SEP_C 且次峰>=5%主峰 → sep_ok=1, 回传 lo/hi。
   仅供 top_edge_validate 亮峰/分界参考 (隔离全局 bimodal_midref σ3)。 */
ITCM_FUNC static int bimodal_ref8(float *plo, float *phi)
{
    uint16_t hist[256];
    int32_t h[256];
    static int16_t gk[65];              /* σ8 Q8 核: 静态缓存只算一次 */
    static int gk_done = 0;
    int i, x, r = 32;
    int32_t mx;
    /* D1(2026-08-13): 复用 build_hist 的 s_hist (内容一致), 省全图重扫 */
    build_hist();
    memcpy(hist, s_hist, sizeof(hist));
    if (!gk_done) {                     /* expf 每帧 65 次太耗, 只算一次 */
        float k, gs = 0.0f;
        int sum = 0;
        for (i = -r; i <= r; i++) {
            k = expf(-(float)(i * i) / (2.0f * 8.0f * 8.0f));
            gk[r + i] = (int16_t)(k * 256.0f + 0.5f);   /* exp(0)=1 起步 */
            gs += k;
        }
        /* 按 float 和重归一化: q' = round(k/gs*256) */
        sum = 0;
        for (i = -r; i <= r; i++) {
            k = expf(-(float)(i * i) / (2.0f * 8.0f * 8.0f)) / gs;
            gk[r + i] = (int16_t)(k * 256.0f + 0.5f);
            sum += gk[r + i];
        }
        gk[r] = (int16_t)(gk[r] + (256 - sum)); /* 中心补差, 总和=256 */
        gk_done = 1;
    }
    /* int32 卷积: acc = Σ gk*hist, (acc+128)>>8 */
    for (x = 0; x < 256; x++) {
        int32_t s = 0;
        for (i = -r; i <= r; i++) {
            int xx = x + i;
            if (xx < 0) xx = -xx;
            if (xx > 255) xx = 2 * 255 - xx;
            s += (int32_t)gk[r + i] * hist[xx];
        }
        h[x] = (s + 128) >> 8;
    }
    mx = h[0];
    for (i = 1; i < 256; i++)
        if (h[i] > mx) mx = h[i];
    {
        /* 峰检测 + NMS(±6 bin): Q8 量化的阶梯平顶会产生相邻伪峰
           (如 35@81 旁冒出 37@80), float σ8 平滑曲线无此问题;
           NMS 后与 float 峰选一致 (000289/290/205 教训)。 */
        int pk_i[24], npk = 0;
        int32_t pk_v[24];
        int p0 = -1, p1 = -1;
        int32_t v0 = -1, v1 = -1;
        for (i = 2; i < 254 && npk < 24; i++) {
            if (h[i] >= h[i - 1] && h[i] > h[i + 1] && h[i] * 50 > mx) {
                int j, sup = 0;
                for (j = 0; j < npk; j++) {
                    int d = i - pk_i[j];
                    if (d < 0) d = -d;
                    if (d <= 6 && h[i] <= pk_v[j]) { sup = 1; break; }
                }
                if (!sup) {
                    pk_i[npk] = i;
                    pk_v[npk] = h[i];
                    npk++;
                }
            }
        }
        for (i = 0; i < npk; i++) {                 /* top2 by height */
            if (pk_v[i] > v0) { v1 = v0; p1 = p0; v0 = pk_v[i]; p0 = pk_i[i]; }
            else if (pk_v[i] > v1) { v1 = pk_v[i]; p1 = pk_i[i]; }
        }
        if (p0 >= 0 && p1 >= 0) {
            int lo = p0 < p1 ? p0 : p1;
            int hi = p0 < p1 ? p1 : p0;
            /* 次峰 >= 0.05*mx (×20 整数化) */
            if (hi - lo >= BIMODAL_SEP_C && v1 * 20 >= mx) {
                *plo = (float)lo;
                *phi = (float)hi;
                return 1;
            }
        }
    }
    *plo = *phi = 0.0f;
    return 0;
}

/* ================= v9 结束线边线式 y 方向差比和验证 =================
   (与 PC bridge_v8.top_edge_validate 一致, 2026-08-13 定稿):
   框选 = 红蓝边线之间; 缺侧边线以图象边缘为界 (无左→x=0, 无右→x=W-1,
   用户: 差比和验证是必要一步, 边线只是约束范围)。
   段内顺序: 取带 → 同亮驳回(不豁免) → 同暗中性段(不计入, 与边线
   line_edge_ratio 同暗逻辑对齐) → T_y<10 只跳差比和 → valid/ok 计数。
   >=75% 有效段差比和符合才通过; 全部段被跳过 → 无法验证 → 放行。 */
#define TOP_EDGE_SEG      6
#define TOP_EDGE_BAND_LO  6
#define TOP_EDGE_BAND_HI  11
#define TOP_EDGE_SKIP_Y   10.0f
#define TOP_EDGE_RATIO    30.0f          /* RATIO_LOOSE_PER */
#define TOP_EDGE_MEAN_D   18.0f          /* MEAN_DIFF_LOOSE */

ITCM_FUNC static int top_edge_validate(const bridge_line_t *tf,
                             const iline_t *lf, const iline_t *rf)
{
    float a = tf->a, b = tf->b;
    float lo, hi, bright, mid_ref = 0.0f;
    int sep_ok = bimodal_ref8(&lo, &hi);
    int xmin = W, xmax = -1, y, x, s, yy;
    int ok_seg = 0, valid_seg = 0, skip_top = 0;
    float ty_c;
    bright = sep_ok ? hi * 0.6f : (float)TOP_FAR_THR;
    if (sep_ok)
        mid_ref = (lo + hi) * 0.5f;
    /* 框选 x 范围 (缺侧以图象边缘为界) */
    for (y = 0; y < H; y++) {
        float xl = lf ? lf->f.a * y + lf->f.b : 0.0f;
        float xr = rf ? rf->f.a * y + rf->f.b : (float)(W - 1);
        int x0 = (int)ceilf(xl < xr ? xl : xr);
        int x1 = (int)floorf(xl > xr ? xl : xr);
        if (x1 > x0) {
            if (x0 < xmin) xmin = x0;
            if (x1 > xmax) xmax = x1;
        }
    }
    xmin = xmin + 2 > 0 ? xmin + 2 : 0;
    xmax = xmax - 2 < W - 1 ? xmax - 2 : W - 1;
    if (xmax <= xmin)
        return 0;
    ty_c = a * (W * 0.5f) + b;
    for (s = 0; s < TOP_EDGE_SEG; s++) {
        int xs0 = xmin + (xmax - xmin) * s / TOP_EDGE_SEG;
        int xs1 = xmin + (xmax - xmin) * (s + 1) / TOP_EDGE_SEG;
        float su = 0.0f, sd = 0.0f, up, dn, d;
        int nu = 0, nd = 0;
        if (xs1 <= xs0)
            continue;
        for (x = xs0; x < xs1; x++) {
            float yt = a * x + b;
            int u0 = (int)(yt - TOP_EDGE_BAND_HI);
            int u1 = (int)(yt - TOP_EDGE_BAND_LO);
            int d0, d1;
            if (u0 < 0) u0 = 0;
            if (u1 < 0) u1 = 0;
            if (u1 <= u0) {             /* 顶部自适应: 上带越界取全上方 */
                u0 = 0;
                u1 = (int)(yt - 2);
                if (u1 < 0) u1 = 0;
            }
            d0 = (int)(yt + TOP_EDGE_BAND_LO); if (d0 > H) d0 = H;
            d1 = (int)(yt + TOP_EDGE_BAND_HI); if (d1 > H) d1 = H;
            for (yy = u0; yy < u1; yy++) { su += (float)s_imgT[x][yy]; nu++; }
            for (yy = d0; yy < d1; yy++) { sd += (float)s_imgT[x][yy]; nd++; }
        }
        if (nu < 6 || nd < 6)
            continue;
        up = su / nu;
        dn = sd / nd;
        d = fabsf(up - dn);
        /* 同亮驳回: 上下都亮且无跳变 → 直接驳回 ("两侧均白", 不豁免) */
        if (up > bright && dn > bright && d <= TOP_EDGE_MEAN_D)
            return 0;
        /* 同暗中性段: 上下都暗(mid_ref下)且无跳变 → 桥外背景, 不计入 */
        if (sep_ok && up < mid_ref && dn < mid_ref && d <= TOP_EDGE_MEAN_D)
            continue;
        /* 帧级/段中心 T_y<SKIP_Y → 只跳过差比和统计 */
        if (ty_c < TOP_EDGE_SKIP_Y ||
            a * ((xs0 + xs1) * 0.5f) + b < TOP_EDGE_SKIP_Y) {
            skip_top++;
            continue;
        }
        valid_seg++;
        if (d * 100.0f > TOP_EDGE_RATIO * (up + dn) || d > TOP_EDGE_MEAN_D)
            ok_seg++;
    }
    if (valid_seg == 0)
        return skip_top > 0;            /* 全是顶部段 → 无法验证 → 放行 */
    return ok_seg * 100 >= valid_seg * 75;
}

/* int8 MLP 前向: 完整 3 层融合汇编 (mlp_forward_s8, FC0 双发射)
   回退开关: 置 0 用逐层 mlp_fc_s8_layer (数学等价, 便于对比)。 */
#define MLP_USE_FULL_ASM 1
static int32_t s_w0d[80];           /* FC0 交错双通道权重 (5对x16 int32) */
static int s_w0d_done;

/* 由行优先 mlp_w0p 构建 FC0 交错布局 {chA_p0,chB_p0,chA_p1,chB_p1}
   (双发射内层单指针连续加载, 同旧版 fc_layer_dual 思路)。 */
static void mlp_dual_pack_init(void)
{
    int p, g;
    if (s_w0d_done)
        return;
    for (p = 0; p < 5; p++)
        for (g = 0; g < 4; g++) {
            s_w0d[p * 16 + g * 4 + 0] = mlp_w0p[(2 * p) * 8 + g * 2 + 0];
            s_w0d[p * 16 + g * 4 + 1] = mlp_w0p[(2 * p + 1) * 8 + g * 2 + 0];
            s_w0d[p * 16 + g * 4 + 2] = mlp_w0p[(2 * p) * 8 + g * 2 + 1];
            s_w0d[p * 16 + g * 4 + 3] = mlp_w0p[(2 * p + 1) * 8 + g * 2 + 1];
        }
    s_w0d_done = 1;
}

static const mlp_forward_s8_model_t s_mlp_model = {
    mlp_w0p, mlp_b0, mlp_mult0,
    mlp_w1p, mlp_b1, mlp_mult1,
    mlp_w2p, mlp_b2, mlp_mult2,
    s_w0d
};

/* R6b: 穿越点 midv 只需第 (k+1) 小 / 第 (k+1) 大 (k<=2, n<=13, 值域 0..255),
   免全量冒泡排序 — 位精确 (结果同 sorted[k])。 */
static inline int sel_small(const int *a, int n, int k)
{
    int b[3] = { 256, 256, 256 };      /* 值域 0..255, 256 作哨兵 */
    int i, j;
    for (i = 0; i < n; i++) {
        int v = a[i];
        if (v < b[k]) {
            for (j = k - 1; j >= 0 && b[j] > v; j--)
                b[j + 1] = b[j];
            b[j + 1] = v;
        }
    }
    return b[k];
}
static inline int sel_large(const int *a, int n, int k)
{
    int b[3] = { -1, -1, -1 };         /* 值域 0..255, -1 作哨兵 */
    int nb = 0, i, j;
    for (i = 0; i < n; i++) {
        int v = a[i];
        if (nb < k + 1) {              /* 未满: 直接插入保持升序 */
            for (j = nb - 1; j >= 0 && b[j] > v; j--)
                b[j + 1] = b[j];
            b[j + 1] = v;
            nb++;
        } else if (v > b[0]) {         /* 已满: 丢最小, 插入更大值 */
            for (j = 0; j < k; j++)
                b[j] = b[j + 1];       /* b[0..k-1] = 旧 b[1..k] (升序) */
            for (j = k - 1; j >= 0 && b[j] > v; j--)
                b[j + 1] = b[j];
            b[j + 1] = v;
        }
    }
    return b[0];                     /* 升序集合最小 = 第 (k+1) 大 */
}

static int16_t mlp_forward_row(const int8_t *x)
{
#if MLP_USE_FULL_ASM
    mlp_dual_pack_init();
    return (int16_t)mlp_forward_s8(x, &s_mlp_model);
#else
    static int8_t a0[12] __attribute__((aligned(4)));
    static int8_t a1[8] __attribute__((aligned(4)));
    /* 层0: 16->10, pad 16 (4组); 层1: 10->5, pad 12 (3组); 层2: 5->1, pad 8 (2组) */
    mlp_fc_s8_layer(x, mlp_w0p, mlp_b0, mlp_mult0, mlp_shift0,
                    a0, 4, MLP_END_H1, 1);
    mlp_fc_s8_layer(a0, mlp_w1p, mlp_b1, mlp_mult1, mlp_shift1,
                    a1, 3, MLP_END_H2, 1);
    mlp_fc_s8_layer(a1, mlp_w2p, mlp_b2, mlp_mult2, mlp_shift2,
                    a1, 2, 1, 0);
    return (int16_t)a1[0];
#endif
}

/* MLP 结束线检测入口: 成功返回 1 并填 tf */
static int mlp_end_detect(const iline_t *lf, const iline_t *rf,
                          int tb_in, int gate, bridge_line_t *tf)
{
    int y, x, y0, m = 0, i, n, nn;
    float a = 0, b = 0, rms = 0, yf, y_img, x_c, span;
    int x_lo, x_hi;

    mlp_extract_feats(lf, rf, tb_in, gate);
    PROF_FIT_BEGIN();                       /* fit 段子计时起点 (=feats 末) */
    for (y = 0; y < GH; y++)
        s_mlp_logit[y] = mlp_forward_row(s_mlp_feat[y]);
    /* 平滑 [1,2,1]/4 (四舍五入, 与 Python np.convolve 对齐) */
    for (y = 0; y < GH; y++) {
        int32_t s = s_mlp_logit[y] * 2;
        if (y > 0) s += s_mlp_logit[y - 1];
        if (y < GH - 1) s += s_mlp_logit[y + 1];
        s_mlp_logit_s[y] = (int16_t)((s + (s >= 0 ? 2 : -2)) / 4);
    }
    /* argmax + 存在性门控 */
    y0 = 0;
    for (y = 1; y < GH; y++)
        if (s_mlp_logit_s[y] > s_mlp_logit_s[y0])
            y0 = y;
    if (s_mlp_logit_s[y0] < MLP_LOGIT_THR)
        return 0;
    /* 亚像素质心 (y0±2) */
    {
        int32_t sw = 0;
        double sy = 0;
        for (y = y0 - 2; y <= y0 + 2; y++) {
            int32_t w;
            if (y < 0 || y >= GH)
                continue;
            w = s_mlp_logit_s[y] > 0 ? s_mlp_logit_s[y] : 0;
            sw += w; sy += (double)y * w;
        }
        yf = sw > 0 ? (float)(sy / sw) : (float)y0;
    }
    y_img = yf + 1.5f;
    mlp_env((int)y_img, lf, rf, &x_lo, &x_hi);
    if (x_lo < 3) x_lo = 3;
    if (x_hi > W - 4) x_hi = W - 4;
    PROF_FIT_MARK();                        /* fprep 终点 (前向+预处理) */

    /* 穿越点收集: 包络内逐列, 亮暗穿越 + 三重校验 */
    for (x = x_lo; x <= x_hi && m < MLP_PTS_MAX; x++) {
        int y_lo = (int)(y_img - MLP_WIN), y_hi = (int)(y_img + MLP_WIN);
        int seg[13], nseg = 0, yy, best_y = -1;
        float midv;
        if (y_lo < 1) y_lo = 1;
        if (y_hi > H - 3) y_hi = H - 3;
        for (yy = y_lo; yy <= y_hi; yy++)
            seg[nseg++] = s_imgT[x][yy];            /* 转置: 行连续读 */
        if (nseg < 4)
            continue;
        /* R6b: 部分选择 (位精确) 替代全量冒泡排序 — 只需第 nseg/5 小/大 */
        midv = ((float)(sel_small(seg, nseg, nseg / 5) +
                        sel_large(seg, nseg, nseg - 1 - nseg * 4 / 5)) * 0.5f);
        for (yy = y_lo + 1; yy <= y_hi; yy++) {
            if (s_imgT[x][yy - 1] < midv && s_imgT[x][yy] >= midv) {
                int ok = 1;
                int nnb = 0, na = 0, k;
                if (yy + 3 < H) {           /* 下方持续亮 */
                    for (k = yy; k <= yy + 3; k++)
                        if (s_imgT[x][k] > midv) nnb++;
                    if (nnb < 2) ok = 0;
                }
                if (ok && yy >= 9) {        /* 上方持续暗 (2026-08-07: 白结束后
                                                向上审查范围放大 4→8 行) */
                    for (k = yy - 9; k <= yy - 2; k++)
                        if (s_imgT[x][k] >= midv) na++;
                    if (na > 2) ok = 0;
                }
                if (ok) {                   /* gy 极性: 暗→亮 gy>0 */
                    int gy_y = (int)(yy - 1.5f + 0.5f);
                    int gy_x = (int)(x - 1.5f + 0.5f);
                    if (gy_y >= 0 && gy_y < GH && gy_x >= 0 && gy_x < GW) {
                        if (s_gy[gy_y][gy_x] <= 0)
                            ok = 0;
                    }
                }
                if (ok)
                    best_y = yy;
                break;
            }
        }
        if (best_y >= 0) {
            s_mlp_pts[m].u = x + 0.5f;
            s_mlp_pts[m].v = (float)best_y;
            s_mlp_pts[m].w = 1.0f;
            m++;
        }
    }
    /* 剔除偏离质心行过远 */
    {
        int k = 0;
        for (i = 0; i < m; i++)
            if (fabsf(s_mlp_pts[i].v - y_img) <= MLP_MAX_DEV)
                s_mlp_pts[k++] = s_mlp_pts[i];
        m = k;
    }
    PROF_FIT_MARK();                        /* fcross 终点 (穿越点收集) */
    /* 锚点 + RANSAC (2026-08-12 对齐 PC top_line_from_scores: 穿越点不足 /
       RANSAC 无解 / 内点少 时回退水平线 (a=0, b=y_img), 顶部帧可见列少       是正常情况, 不应判漏检) */
    x_c = (x_lo + x_hi) * 0.5f;
    if (m >= MLP_PTS_MAX)
        m = MLP_PTS_MAX - 1;
    s_mlp_pts[m].u = x_c;
    s_mlp_pts[m].v = y_img;
    s_mlp_pts[m].w = (float)(m > 3 ? m : 3);
    m++;
    /* 水平线回退 (对齐 PC top_line_from_scores): 穿越点不足 / RANSAC 无解 /
       内点少时回退 (0, y_img)。2026-08-12 仅限顶部帧 (y_img<12): 真结束线
       在顶部可见列少是正常; 中下部穿越点不足则无支撑, 拒绝 (控 6081 误检)。 */
#define MLP_BACK_FALLBACK() do { if (y_img < 12.0f) { \
    tf->a = 0.0f; tf->b = y_img; tf->n = (int16_t)m; tf->rms = 0.0f; \
    tf->u_lo = tf->u_hi = 0; return 1; } return 0; } while (0)
    if (m < MLP_MIN_PTS)
        MLP_BACK_FALLBACK();
    n = ransac_best(s_mlp_pts, m, MLP_SLOPE, &a, &b, s_mask);
    if (!n)
        MLP_BACK_FALLBACK();
    rms = refit(s_mlp_pts, s_mask, m, &a, &b, &nn);
    if (nn < 4)
        MLP_BACK_FALLBACK();
#undef MLP_BACK_FALLBACK
    (void)span;
    tf->a = a;
    tf->b = b;
    tf->n = (int16_t)nn;
    tf->rms = rms;
    tf->u_lo = tf->u_hi = 0;
    PROF_FIT_MARK();                        /* fransac 终点 (RANSAC+refit) */
    /* 上方回桥面否决 (TOP_FAR_WHITE): v8 2026-08-12 金标准重标定 ——
       PC end_line_reject 含 top_far, 66GT 50/50 通过 (真T 上方12~32行近白
       max=0.03 < 0.12), 6081 亮带延伸帧被拒。
       C 端用 y_img(MLP 质心行, 水平锚定) 代替拟合线 a,b —— int8 MLP 拟合线
       在个别帧 (000273/000253) 位置偏下致 top_far 误判; y_img 更鲁棒。 */
    PROF_MARK();                                /* fit 终点 (slot8) */
    {
        int xx, near = 0, tot = 0;
        for (xx = 0; xx < W; xx++) {
            float yt = a * (float)xx + b;
            int y0 = (int)(yt - TOP_FAR_HI), y1 = (int)(yt - TOP_FAR_LO), yy;
            if (y0 < 0)
                y0 = 0;
            if (y1 <= y0 || y1 > H)
                continue;
            for (yy = y0; yy < y1; yy++)
                if (s_imgT[xx][yy] > TOP_FAR_THR)
                    near++;
            tot += y1 - y0;
        }
        if (1 && tot > 40) {
            float r = (float)near / tot;
            if (r > TOP_FAR_WHITE)
                return 0;
        }
    }
    return 1;
}
#endif /* !TOP_GRAD */
void bridge_detect_init(bridge_state_t *st)
{
    memset(st, 0, sizeof(*st));
    st->wp_a = W_PRIOR_INIT_A;
    st->wp_b = W_PRIOR_INIT_B;
    if (getenv("BRIDGE_EDGE_DBG"))
        s_edge_dbg = atoi(getenv("BRIDGE_EDGE_DBG")) != 0;
}

/* ================= R5(2026-08-13): step3+step4 ITCM 热点 =================
   base 段最大固定头; 22KB .text 跑 Flash, I-cache/ART 取指停顿是隐藏成本。
   纯代码搬移 (逻辑与原 bridge_detect_frame 内完全一致, 阈值存 s_tp/s_tn/s_tt)。
   返回 n_rows_ok (降级后); *pnpos 与 *pnneg 填候选数。 */
static float s_tp, s_tn, s_tt;
ITCM_FUNC static int step3_4_dyn_threshold(int *pnpos, int *pnneg)
{
    static uint16_t hist_p[256], hist_n[256], hist_t[256];
    int r, j, i;
    int npos = 0, nneg = 0, nok = 0;
    float tp, tn, tt;
    memset(hist_p, 0, sizeof(hist_p));
    memset(hist_n, 0, sizeof(hist_n));
    memset(hist_t, 0, sizeof(hist_t));
    PROF_S34_BEGIN();                           /* step3_4 内部计时起点 */
    for (r = 0; r < GH; r++) {
        /* 缓存本行 gvar/hvar (一次计算, hist + step4 复用) */
        for (j = 0; j < GW; j++) {
            s_gvar_r[j] = (int16_t)gvar_at(r, j);
            s_hvar_r[j] = (int16_t)hvar_at(r, j);
        }
        for (j = 0; j < GW; j++) {
            int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
            int gv = s_gvar_r[j];
            int gy = s_gy[r][j], ay = gy < 0 ? -gy : gy;
            int hv = s_hvar_r[j], bin;
            if (gv < 0)
                gv = -gv;
            if (hv < 0)
                hv = -hv;
            bin = ax >> 4;
            if (bin > 255)
                bin = 255;
            if (ax > (gv * 2)) {
                if (gx > 0)
                    hist_p[bin]++;
                else
                    hist_n[bin]++;
            }
            bin = ay >> 4;
            if (bin > 255)
                bin = 255;
            if (gy > 0 && ay > (hv * 2))
                hist_t[bin]++;
        }
    }
    tp = thr_from_hist(hist_p);
    tn = thr_from_hist(hist_n);
    tt = thr_from_hist(hist_t);
    s_tp = tp; s_tn = tn; s_tt = tt;
    PROF_S34_MARK();                            /* s_gh 终点 (gvar/hvar+直方图) */

    /* ---- 行背景判断 + 候选点: 每行 top-2 ---- */
    /* 第一遍: 各行 strong 标志 + top-2 暂存 + 行有效性掩码
       (bridge_v5.py row_bg_mask: 过阈簇 1..CLU_MAX 且
        簇外中间带 MID_LO·t<|gx|<=t 像素 <= MID_OUT_MAX) */
    /* R5b: 中间带下界 midp/midn 每帧定值, 提到行循环外 (原每候选像素浮点转换) */
    {
        int midp = (int)(MID_LO * tp), midn = (int)(MID_LO * tn);
    for (r = 0; r < GH; r++) {
        int bp_x[2], bp_m[2] = { 0, 0 };
        int bn_x[2], bn_m[2] = { 0, 0 };
        int n_clu = 0, mid_out = 0, last_s = -100;
        for (j = 0; j < GW; j++) {
            int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
            int gv = s_gvar_r[j], strong = 0;
            if (gv < 0)
                gv = -gv;
            if (ax > (gv * 2)) {
                if (gx > 0) {
                    if (ax > tp) {
                        strong = 1;
                        if (ax > bp_m[0]) { bp_m[1] = bp_m[0]; bp_x[1] = bp_x[0]; bp_m[0] = ax; bp_x[0] = j; }
                        else if (ax > bp_m[1]) { bp_m[1] = ax; bp_x[1] = j; }
                    } else if (ax > midp) {
                        mid_out++;          /* 暂记, 后面剔除强簇邻域 */
                    }
                } else if (gx < 0) {
                    if (ax > tn) {
                        strong = 1;
                        if (ax > bn_m[0]) { bn_m[1] = bn_m[0]; bn_x[1] = bn_x[0]; bn_m[0] = ax; bn_x[0] = j; }
                        else if (ax > bn_m[1]) { bn_m[1] = ax; bn_x[1] = j; }
                    } else if (ax > midn) {
                        mid_out++;
                    }
                }
            }
            s_strong[j] = (uint8_t)strong;
            if (strong) {
                if (j - last_s > 2)
                    n_clu++;                /* 间隔 >=3 开新簇 (同 PC) */
                last_s = j;
            }
        }
        /* 剔除强簇 ±MID_DIST 邻域内的中间带像素 (边缘拖尾)
           R6c: O(5×GW)→O(2×GW) 双指针。位精确: 非强像素 j 在拖尾内
           ⇔ 最近强像素距离 ≤ MID_DIST (原内层 k∈[j-MID_DIST,j+MID_DIST] 扫描)。 */
        if (mid_out) {
            int in_tail = 0;
            int jj, nd = 100;
            for (jj = GW - 1; jj >= 0; jj--) {      /* 预计算右邻强标志 */
                if (s_strong[jj]) nd = 0; else nd++;
                s_tail_nd[jj] = (int8_t)(nd <= MID_DIST);
            }
            {
                int ld = 100;
                for (j = 0; j < GW; j++) {
                    if (s_strong[j]) { ld = 0; continue; }
                    ld++;
                    if (ld <= MID_DIST || s_tail_nd[j]) {   /* 左或右邻强 */
                        int gx = s_gx[r][j], ax = gx < 0 ? -gx : gx;
                        int gv = s_gvar_r[j];
                        int tmid;
                        if (gv < 0)
                            gv = -gv;
                        tmid = (gx > 0 ? midp : midn);
                        if (ax > (gv * 2) && ax > tmid &&
                            ax <= (gx > 0 ? tp : tn))
                            in_tail++;
                    }
                }
            }
            mid_out -= in_tail;
        }
        s_row_ok[r] = (uint8_t)(n_clu >= 1 && n_clu <= CLU_MAX &&
                                mid_out <= MID_OUT_MAX);
        s_bpx[r][0] = (int16_t)bp_x[0]; s_bpx[r][1] = (int16_t)bp_x[1];
        s_bpm[r][0] = (int16_t)bp_m[0]; s_bpm[r][1] = (int16_t)bp_m[1];
        s_bnx[r][0] = (int16_t)bn_x[0]; s_bnx[r][1] = (int16_t)bn_x[1];
        s_bnm[r][0] = (int16_t)bn_m[0]; s_bnm[r][1] = (int16_t)bn_m[1];
    }
    }   /* R5b: midp/midn 作用域块结束 */
    PROF_S34_MARK();                            /* s_bg 终点 (行背景判断) */
    /* 降级: 有效行过少 -> 全行有效 (整帧杂乱不至于完全无线) */
    for (r = 0; r < GH; r++)
        nok += s_row_ok[r];
    if (nok < ROW_OK_MIN) {
        for (r = 0; r < GH; r++)
            s_row_ok[r] = 1;
        nok = GH;
    }
    /* 第二遍: 有效行候选入队 */
    for (r = 0; r < GH; r++) {
        if (ROW_BG_FILTER && !s_row_ok[r])
            continue;
        for (i = 0; i < 2; i++) {
            if (s_bpm[r][i] && npos < MAX_CAND) {
                s_pos[npos].u = r + 1.5f;
                s_pos[npos].v = s_bpx[r][i] + 1.5f;
                s_pos[npos].w = (float)s_bpm[r][i];
                npos++;
            }
            if (s_bnm[r][i] && nneg < MAX_CAND) {
                s_neg[nneg].u = r + 1.5f;
                s_neg[nneg].v = s_bnx[r][i] + 1.5f;
                s_neg[nneg].w = (float)s_bnm[r][i];
                nneg++;
            }
        }
    }
    *pnpos = npos;
    *pnneg = nneg;
    PROF_S34_MARK();                            /* s_enq 终点 (候选入队) */
    return nok;
}

void bridge_detect_frame(const uint8_t *img94,
                         bridge_state_t *st,
                         bridge_result_t *out)
{
    int r, j, i, ring = 0;
    int tb, tb_in = 0;
    int npos = 0, nneg = 0, ntop = 0;
    int nlines, ir, ig, ib;
    float prior = 0, spacing = 0;
    bridge_mode_t mode;

    memset(out, 0, sizeof(*out));

    PROF_BEGIN();                               /* 帧计时起点 (slot0) */

    /* ---- 1) 输入快照 (防处理期间 DMA 改写源缓冲造成撕裂) ---- */
    memcpy(s_img, img94, H * W);
    /* 全图直方图一次扫描, Otsu/bimodal_midref/bcv_global 复用 */
    s_hist_ready = 0;
    build_hist();

    /* ---- 2) 汇编卷积: 水平环形 + 垂直 ---- */
    for (r = 0; r < H; r++) {
        for (j = 0; j < W; j++)
            s_raw[j] = s_img[r][j];
        b2_conv1d_horiz_gxgy(s_raw, s_ringx[ring], s_ringy[ring], GW);
        if (r >= 3) {
            int i0 = (ring + 1) & 3;    /* 最老行 r-3 */
            int i1 = (ring + 2) & 3;
            int i2 = (ring + 3) & 3;
            int i3 = ring;              /* 最新行 r   */
            b2_conv1d_vert_gxgy_row(s_ringx[i0], s_ringx[i1],
                                 s_ringx[i2], s_ringx[i3],
                                 s_ringy[i0], s_ringy[i1],
                                 s_ringy[i2], s_ringy[i3],
                                 s_gx[r - 3], s_gy[r - 3], GW);
        }
        ring = (ring + 1) & 3;
    }
    PROF_MARK();                                /* conv 终点 (slot1) */

    /* ---- 3)+4) 动态阈值 + 行背景/候选 (R5: 抽为 ITCM 热点函数) ---- */
    {
        int npos2, nneg2;
        out->n_rows_ok = (uint8_t)step3_4_dyn_threshold(&npos2, &nneg2);
        npos = npos2;
        nneg = nneg2;
#if TOP_GRAD
        {
            int r, j, i;
            float tt = s_tt;
            for (j = 0; j < GW; j++) {          /* 顶线: 逐列 top-2 */
                int bt_y[2], bt_m[2] = { 0, 0 };
                for (r = 0; r < GH; r++) {
                    int gy = s_gy[r][j], ay;
                    int hv = hvar_at(r, j);
                    if (gy <= 0)
                        continue;
                    ay = gy;
                    if (hv < 0)
                        hv = -hv;
                    if (ay <= (hv * 2) || ay <= tt)
                        continue;
                    if (ay > bt_m[0]) { bt_m[1] = bt_m[0]; bt_y[1] = bt_y[0]; bt_m[0] = ay; bt_y[0] = r; }
                    else if (ay > bt_m[1]) { bt_m[1] = ay; bt_y[1] = r; }
                }
                for (i = 0; i < 2; i++) {
                    if (bt_m[i] && ntop < MAX_TOPC) {
                        s_topc[ntop].u = j + 1.5f;   /* 顶线: u=x */
                        s_topc[ntop].v = bt_y[i] + 1.5f;
                        s_topc[ntop].w = (float)bt_m[i];
                        ntop++;
                    }
                }
            }
        }
#else
        (void)s_tt;
        (void)ntop;
#endif
    }
    PROF_MARK();                                /* step3_4 终点 (slot2) */

    /* ---- 5) 全图 Otsu ---- */
    tb = otsu_img();

    /* ---- 6) 竖线提取 (正/负分开序贯 RANSAC) ---- */
    nlines = extract_sign_lines(0, s_pos, npos, VLINE_MAX);
    nlines += extract_sign_lines(nlines, s_neg, nneg, VLINE_MAX);
    nlines = merge_lines(nlines);
    out->n_lines = (uint8_t)nlines;
    PROF_MARK();                                /* ransac 终点 (slot3) */

    /* ---- 7) 分类 (随 y 变化先验间距 w(y)=A*y+B, 自校准) ----
       先验 = wp_a*Y_REF + wp_b。wp_a(斜率) 由 RB/RMB 帧最小二乘自校准;
       wp_b(截距) 用滑动窗中位 w@Y_REF 锚定 (稳健, 防幸存者偏差使间距虚大)。
       未校准(sp_n=0)时用初始可调参数 W_PRIOR_INIT_A/B。
       距离合规(间距≥LO*先验)才是边线; 过近 → 提取的是中线 (用户)。 */
    if (st->sp_n > 0) {
        float tmp[10];
        memcpy(tmp, st->sp_buf, st->sp_n * sizeof(float));
        qsort(tmp, st->sp_n, sizeof(float), cmp_f32);
        st->wp_b = tmp[st->sp_n / 2] - st->wp_a * Y_REF;   /* 中位锚定 */
    }
    prior = st->wp_a * Y_REF + st->wp_b;
    if (prior < MIN_SPACING)
        prior = MIN_SPACING;
    mode = classify(nlines, prior, tb, &ir, &ig, &ib, &spacing);
    /* v11 非赛道驳回: 全图 bcv 不够 → 无边线 (mode=none), 边线不渲染
       (用户: 明显没桥面的画面不应渲染出识别到的边线) */
    if (bcv_global() < V8_BCV_GLOBAL_MIN) {
        ir = ig = ib = -1;
        mode = BRIDGE_MODE_NONE;
        spacing = 0.0f;
    }
    /* v11 错误线条驳回 → mode=RB_Q(结构错误), 边线不渲染(后续不处理)。
       ① 红蓝夹角/交点几何不合理  ② 边线先验距离: 左右边线 Y_REF 间距过近
          (覆盖 classify.pair_too_close 出画漏判, 如 05_00000 间距13.3) */
    if (mode == BRIDGE_MODE_RB || mode == BRIDGE_MODE_RMB) {
        float xl = s_lines[ir].f.a * Y_REF + s_lines[ir].f.b;
        float xr = s_lines[ib].f.a * Y_REF + s_lines[ib].f.b;
        if (!line_angle_ok(&s_lines[ir].f, &s_lines[ib].f) ||
            xr - xl < MIN_SPACING)
            mode = BRIDGE_MODE_RB_Q;
    }
    /* mode=8(结构错误, 含 classify.pair_too_close 产出) 统一边线置空:
       不渲染, 后续不处理 */
    if (mode == BRIDGE_MODE_RB_Q) {
        ir = ig = ib = -1;
        spacing = 0.0f;
    }
    out->mode = mode;
    out->spacing = spacing;
    if ((mode == BRIDGE_MODE_RB || mode == BRIDGE_MODE_RMB) && spacing > 0 &&
        ir >= 0 && ib >= 0) {
        float xr = s_lines[ir].f.a * Y_REF + s_lines[ir].f.b;
        float xb = s_lines[ib].f.a * Y_REF + s_lines[ib].f.b;
        if (xr >= 2.0f && xr <= W - 3.0f && xb >= 2.0f && xb <= W - 3.0f) {
            /* 两线在画面内才记账: 出画外推的大间距会污染间距中位(→先验虚大→全过近) */
            st->sp_buf[st->sp_head] = spacing;
            st->sp_head = (uint8_t)((st->sp_head + 1) % 10);
            if (st->sp_n < 10)
                st->sp_n++;
            /* 自校准先验间距 w(y): 更新斜率 A (B 由中位锚定) */
            wp_calibrate_frame(st, &s_lines[ir], &s_lines[ib]);
        }
    }

    /* ---- 8) 双峰分界 mid_ref (MLP 特征 tb, 与训练/PC v7 一致) + 门控 (锁存) ----
       2026-08-12: MLP 重训特征 tb 用 mid_ref, 替换 inner_threshold */
    tb_in = bimodal_midref();
    if (!st->gate) {
        int first = -1, last = -1;
        if (ir >= 0) { first = last = ir; }
        if (ig >= 0) { if (first < 0) first = ig; last = ig; }
        if (ib >= 0) { if (first < 0) first = ib; last = ib; }
        if (first >= 0 && last != first) {
            const iline_t *fl = &s_lines[first], *fr = &s_lines[last];
            int br = 0, tot = 0, y, x;
            for (y = GATE_ROWS; y < H; y++) {
                float xl = fl->f.a * y + fl->f.b;
                float xr = fr->f.a * y + fr->f.b;
                int x0 = (int)(xl < xr ? xl : xr) + 2;
                int x1 = (int)(xl > xr ? xl : xr) - 2;
                if (x0 < 0)
                    x0 = 0;
                if (x1 > W - 1)
                    x1 = W - 1;
                for (x = x0; x <= x1; x++) {
                    br += s_img[y][x] > tb_in;
                    tot++;
                }
            }
            if (tot > 0 && br * 2 > tot)
                st->gate = 1;
        }
    }
    out->gate = st->gate;

    /* ---- 9) 三线透视共点精化 (失败回退) ---- */
    if (mode == BRIDGE_MODE_RMB) {
        iline_t *lf = &s_lines[ir], *mf = &s_lines[ig], *rf = &s_lines[ib];
        float dl = lf->f.a - rf->f.a, vy, vx;
        float xlv = lf->f.a * Y_REF + lf->f.b;
        float xrv = rf->f.a * Y_REF + rf->f.b;
        out->mid_ratio = (mf->f.a * Y_REF + mf->f.b - xlv)
                       / (xrv - xlv > 1e-6f ? xrv - xlv : 1e-6f);
        if (dl > 1e-6f || dl < -1e-6f) {
            iline_t *trio[3] = { lf, mf, rf };
            float na[3], nb[3];
            int ok = 1, t;
            vy = (rf->f.b - lf->f.b) / dl;
            vx = lf->f.a * vy + lf->f.b;
            if (vy < 80.0f && vx > -400.0f && vx < 400.0f) {
                for (t = 0; t < 3; t++) {
                    float s1 = 0, s2 = 0;
                    for (i = 0; i < trio[t]->inl_n; i++) {
                        float u = trio[t]->inl_u[i];
                        float v = trio[t]->f.a * u + trio[t]->f.b;
                        float dy = u - vy;
                        s1 += dy * dy;
                        s2 += dy * (v - vx);
                    }
                    if (s1 < 1e-6f) {
                        na[t] = trio[t]->f.a;
                        nb[t] = trio[t]->f.b;
                    } else {
                        na[t] = s2 / s1;
                        nb[t] = vx - na[t] * vy;
                    }
                }
                /* 精化后保序: y=15/58 中线须在间距内缩 15% 带内 */
                for (t = 0; t < 2 && ok; t++) {
                    float y = t ? 58.0f : 15.0f;
                    float xl = na[0] * y + nb[0];
                    float xr = na[2] * y + nb[2];
                    float xm = na[1] * y + nb[1];
                    float w = xr - xl;
                    if (w < 3)
                        continue;
                    if (xm < xl + 0.15f * w || xm > xr - 0.15f * w)
                        ok = 0;
                }
                if (ok) {
                    for (t = 0; t < 3; t++) {
                        trio[t]->f.a = na[t];
                        trio[t]->f.b = nb[t];
                    }
                }
            }
        }
    }

    /* ---- 10) 粉色退出线 (门控 + 红蓝都在) ---- */
    if (ir >= 0)
        { out->has_red = 1; out->red = s_lines[ir].f; }
    if (ig >= 0)
        { out->has_green = 1; out->green = s_lines[ig].f; }
    if (ib >= 0)
        { out->has_blue = 1; out->blue = s_lines[ib].f; }

    /* ---- 10.5) 有效检测 valid (线级级联, 2026-08-09 用户定案) ----
       取消帧级白像素层, 全部帧统一走线级级联。 */
    out->valid = (uint8_t)valid_detect(
        out->has_red ? &out->red : NULL,
        out->has_green ? &out->green : NULL,
        out->has_blue ? &out->blue : NULL);

#if TOP_GRAD
    if (st->gate && ir >= 0 && ib >= 0) {
        const iline_t *lf = &s_lines[ir], *rf = &s_lines[ib];
        int nanc, m = 0, round, found = 0;
        float x1 = 0, y1 = 0, x2 = 0, y2 = 0, tlo = 20.0f, thi = 10.0f;
        bridge_line_t tf;
        /* 候选二次过滤: 左右夹逼 ±6 + 期望 y 带 */
        nanc = make_yexp(lf, rf, &x1, &y1, &x2, &y2);
        if (nanc == 1) {
            tlo = 22.0f;
            thi = 12.0f;
        }
        for (i = 0; i < ntop; i++) {
            float x = s_topc[i].u, y = s_topc[i].v;
            float xl = lf->f.a * y + lf->f.b;
            float xr = rf->f.a * y + rf->f.b;
            if (x < xl - 6 || x > xr + 6)
                continue;
            if (nanc > 0) {
                float ye = yexp_at(x, nanc, x1, y1, x2, y2);
                if (y < ye - tlo || y > ye + thi)
                    continue;
            }
            s_rem[m++] = s_topc[i];
        }
        /* 序贯 3 轮: RANSAC + 区域/亮度校验 */
        for (round = 0; round < 3 && !found; round++) {
            float a, b, rms;
            int nin, nn;
            nin = ransac_best(s_rem, m, SLOPE_MAX_H, &a, &b, s_mask);
            if (!nin)
                break;
            rms = refit(s_rem, s_mask, m, &a, &b, &nn);
            if ((a > SLOPE_MAX_H || a < -SLOPE_MAX_H) ||
                !region_ok_top(a, b, 25.0f) ||
                !bright_ok_top(a, b, lf, rf, tb)) {
                /* 剔除该线内点, 继续找下一条 */
                int k = 0;
                for (i = 0; i < m; i++) {
                    if (!s_mask[i])
                        s_rem[k++] = s_rem[i];
                }
                m = k;
                continue;
            }
            tf.a = a;
            tf.b = b;
            tf.n = (int16_t)nn;
            tf.rms = rms;
            tf.u_lo = tf.u_hi = 0;
            found = 1;
        }
        /* 四重否决 */
        if (found && tf.n < 10)
            found = 0;                          /* 内点过少: 背景/阴影伪边 */
        if (found && !profile_ok_top(tf.a, tf.b, lf, rf, tb))
            found = 0;
        if (found && crosses_bright(tf.a, tf.b, lf, rf, tb_in))
            found = 0;                          /* 禁止横穿亮区 */
        if (found && !top_corners_ok(tf.a, tf.b, lf, rf))
            found = 0;                          /* 角点结构 */
        if (found) {
            int ytv = bright_run_top(lf, rf, tb_in);
            if (ytv >= 0 && tf.a * (W * 0.5f) + tf.b > ytv + 5)
                found = 0;                      /* 线落在亮段顶行之下 */
        }
        if (found) {
            out->has_top = 1;
            out->top = tf;
        }
    }
#else
    /* 粉线: 行级 int8 MLP 推理 (bridge_mlp_end.py)
       2026-08-12: 无条件调用 (v13 原设计; gate 门控实验致 66GT 命中 90.2%→64.7%,
       C 端 classify 单线场景多, lf&&rf 条件误杀 → 回退)。 */
    PROF_MARK();                                /* rest 终点 = base 末 (slot4) */
    {
        const iline_t *lf = ir >= 0 ? &s_lines[ir] : NULL;
        const iline_t *rf = ib >= 0 ? &s_lines[ib] : NULL;
        bridge_line_t tf;
        /* v11 结束线: gy 行游程连通域贯通 (替换 MLP)
           gate 锁存 + 至少一条线 + 非赛道驳回(全图 bcv) + 非结构错误(mode!=RB_Q) */
        if (st->gate && (lf || rf) && mode != BRIDGE_MODE_RB_Q &&
            bcv_global() >= V8_BCV_GLOBAL_MIN) {
            if (v11_top_gy(lf ? &lf->f : NULL, rf ? &rf->f : NULL, &tf)) {
                out->has_top = 1;
                out->top = tf;
            }
        }
    }
#endif
}
