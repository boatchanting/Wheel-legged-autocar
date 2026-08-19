/**
 * @file    bumpy_vision.c
 * @brief   边缘方向检测 — 颠簸路段视觉导航 (极简封装实现)
 * @details 完整边缘检测流水线，全部内部状态自包含。
 *          对外仅暴露 EdgeDetect_Init() 和 EdgeDetect_Process()。
 *
 * @date    2026-07-18
 */

#include "bumpy_vision.h"
#include "tcm.h"
#include "edge_conv_asm.h"
#include <math.h>
#include <string.h>

/* ==========================================================================
 * 旧管线 (188×120 输入) — 仅 BUMPY_USE_NEW_PIPELINE=0 时编译
 * 新管线开启时整段裁掉：常量 / DTCM 环形缓冲 / 旧检测函数全部不编译
 * (2026-08-18)
 * ========================================================================== */
#if !BUMPY_USE_NEW_PIPELINE

/* ==========================================================================
 * 内部常量 (模块自包含, 不暴露到头文件)
 * ========================================================================== */

#define EDGE_IMAGE_W        (BUMPY_IMAGE_W)
#define EDGE_IMAGE_H        (BUMPY_IMAGE_H)
#define EDGE_OUT_W          (EDGE_IMAGE_W - 3U) /* W-3 */

#define EDGE_FIXED_THR      (1500)   /* 固定阈值: |Gx|+|Gy| >= thr  一般不太会动 188*120 条件下是1500，1000这个测试不通过*/
#define EDGE_R              (0.945)
#define EDGE_R_SQ_BUMPY     (EDGE_R * EDGE_R)  /* 结构张量 R²；对应显示 R > 0.945 r**2>0.893 */
#define EDGE_MIN_STRONG_N   (300U)   /* 全图强边缘数 > 300 才判定    */

/* ==========================================================================
 * DTCM 环形缓冲 (模块私有, 0 等待数据访问)
 * ========================================================================== */

#define EDGE_RING_DEPTH     (4U)
DTCM_BSS static int16_t  edge_gx_ring[EDGE_RING_DEPTH][EDGE_OUT_W];
DTCM_BSS static int16_t  edge_gy_ring[EDGE_RING_DEPTH][EDGE_OUT_W];
DTCM_BSS static int16_t  edge_row_buf[EDGE_IMAGE_W];


/* ==========================================================================
 * 内部辅助
 * ========================================================================== */
static float edge_sqrtf(float x)
{
    return sqrtf(x);
}

static uint64 bumpy_edge_accumulate_strong_energy(const int16_t *gx_row,
                                                   const int16_t *gy_row,
                                                   uint32 width,
                                                   int32 threshold,
                                                   uint64 *strong_gx_sq,
                                                   uint64 *strong_gy_sq,
                                                   int64 *strong_gx_gy,
                                                   uint16 *max_gradient_mag)
{
    uint64 energy = 0U;

    for (uint32 x = 0U; x < width; x++)
    {
        const int32 gx = gx_row[x];
        const int32 gy = gy_row[x];
        const int32 abs_gx = (gx < 0) ? -gx : gx;
        const int32 abs_gy = (gy < 0) ? -gy : gy;
        const int32 gradient_mag = abs_gx + abs_gy;

        if (gradient_mag > (int32)*max_gradient_mag)
        {
            *max_gradient_mag = (gradient_mag > 65535) ? 65535U : (uint16)gradient_mag;
        }

        if ((abs_gx >= threshold) || (gradient_mag >= threshold))
        {
            const uint64 gx_sq = (uint64)((int64)gx * gx);
            const uint64 gy_sq = (uint64)((int64)gy * gy);

            *strong_gx_sq += gx_sq;
            *strong_gy_sq += gy_sq;
            *strong_gx_gy += (int64)gx * gy;
            energy += gx_sq + gy_sq;
        }
    }

    return energy;
}

/* ==========================================================================
 * API 实现
 * ========================================================================== */

static void bumpy_edge_detect_init(void)
{
    memset((void *)edge_gx_ring, 0, sizeof(edge_gx_ring));
    memset((void *)edge_gy_ring, 0, sizeof(edge_gy_ring));
    memset((void *)edge_row_buf,  0, sizeof(edge_row_buf));
}

static void bumpy_edge_detect_process(const uint8_t *gray,
                                      uint8 reference_valid,
                                      float reference_dir_x,
                                      float reference_dir_y,
                                      bumpy_edge_detect_output_t *out)
{
    edge_dir_result_t edge_accum;
    int ring_idx;
    int row;
    uint64 strong_energy = 0U;
    uint64 strong_gx_sq = 0U;
    uint64 strong_gy_sq = 0U;
    int64 strong_gx_gy = 0;
    uint16 max_gradient_mag = 0U;
    float r_sq, r, norm;

    /* ---- 初始化累加器 ---- */
    edge_accum.sum_gx       = 0;
    edge_accum.sum_gy       = 0;
    edge_accum.strong_count = 0;
    edge_accum.total_pixels = 0;
    ring_idx = 0;

    /* ---- 逐行流水线 ---- */
    for (row = 0; row < EDGE_IMAGE_H; row++)
    {
        const uint8_t *src_row = &gray[row * EDGE_IMAGE_W];
        int x;

        /* 1) uint8 → int16 展开 */
        for (x = 0; x < EDGE_IMAGE_W; x++)
        {
            edge_row_buf[x] = (int16_t)src_row[x];
        }

        /* 2) 水平 pass */
        conv1d_horiz_gxgy(
            edge_row_buf,
            edge_gx_ring[ring_idx],
            edge_gy_ring[ring_idx],
            EDGE_OUT_W);

        /* 3) 积累 4 行后触发垂直 pass + 方向累加 */
        if (row >= 3)
        {
            edge_dir_result_t edge_row;
            int i0 = (ring_idx + 1) & 3;
            int i1 = (ring_idx + 2) & 3;
            int i2 = (ring_idx + 3) & 3;
            int i3 = ring_idx;

            conv1d_vert_gxgy_row(
                edge_gx_ring[i0], edge_gx_ring[i1],
                edge_gx_ring[i2], edge_gx_ring[i3],
                edge_gy_ring[i0], edge_gy_ring[i1],
                edge_gy_ring[i2], edge_gy_ring[i3],
                edge_gx_ring[i0],
                edge_gy_ring[i0],
                EDGE_OUT_W);

            gradient_mag_dir_fixed(
                edge_gx_ring[i0],
                edge_gy_ring[i0],
                &edge_row,
                EDGE_FIXED_THR,
                EDGE_OUT_W);

            edge_accum.sum_gx += edge_row.sum_gx;
            edge_accum.sum_gy += edge_row.sum_gy;
            edge_accum.strong_count += edge_row.strong_count;
            edge_accum.total_pixels += EDGE_OUT_W;
            strong_energy += bumpy_edge_accumulate_strong_energy(edge_gx_ring[i0],
                                                                   edge_gy_ring[i0],
                                                                   EDGE_OUT_W,
                                                                    EDGE_FIXED_THR,
                                                                    &strong_gx_sq,
                                                                    &strong_gy_sq,
                                                                    &strong_gx_gy,
                                                                    &max_gradient_mag);
        }

        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 计算结构张量方向一致性 R²=L² 和主梯度方向 ---- */
    if ((edge_accum.strong_count > 0U) && (strong_energy > 0U))
    {
        const float inv_energy = 1.0f / (float)strong_energy;
        const float tensor_diff =
            (float)((int64)strong_gx_sq - (int64)strong_gy_sq) * inv_energy;
        const float tensor_cross = 2.0f * (float)strong_gx_gy * inv_energy;

        /* L² = ((A-B)/(A+B))² + (2C/(A+B))²；判定时无需开方。 */
        r_sq = tensor_diff * tensor_diff + tensor_cross * tensor_cross;
        if (r_sq > 1.0f)
        {
            r_sq = 1.0f;
        }

        r = edge_sqrtf(r_sq);

        /*
         * 由 cos(2θ)=(A-B)/D、sin(2θ)=2C/D 直接构造主特征向量。
         * 分支形式避开 atan2f/sinf/cosf，并避免 θ 接近 90° 时的消减误差。
         */
        if ((edge_accum.strong_count > EDGE_MIN_STRONG_N) &&
            (r_sq > EDGE_R_SQ_BUMPY))
        {
            if (tensor_diff >= 0.0f)
            {
                out->dir_x = r + tensor_diff;
                out->dir_y = tensor_cross;
            }
            else
            {
                out->dir_x = tensor_cross;
                out->dir_y = r - tensor_diff;
            }

            norm = edge_sqrtf(out->dir_x * out->dir_x + out->dir_y * out->dir_y);
            if (norm > 0.0f)
            {
                out->dir_x /= norm;
                out->dir_y /= norm;
            }
            else
            {
                out->dir_x = 1.0f;
                out->dir_y = 0.0f;
            }

            /* 结构张量只确定一条轴；选择与上一帧同向的符号，消除 180° 翻转。 */
            if (reference_valid != 0U)
            {
                if ((out->dir_x * reference_dir_x + out->dir_y * reference_dir_y) < 0.0f)
                {
                    out->dir_x = -out->dir_x;
                    out->dir_y = -out->dir_y;
                }
            }
            else if ((out->dir_y < 0.0f) ||
                     ((out->dir_y == 0.0f) && (out->dir_x < 0.0f)))
            {
                out->dir_x = -out->dir_x;
                out->dir_y = -out->dir_y;
            }
        }
        else
        {
            /* 当前帧方向不可信时保持上一次有效方向，防止噪声抖动。 */
            norm = edge_sqrtf(reference_dir_x * reference_dir_x +
                              reference_dir_y * reference_dir_y);
            if ((reference_valid != 0U) && (norm > 0.0f))
            {
                out->dir_x = reference_dir_x / norm;
                out->dir_y = reference_dir_y / norm;
            }
            else
            {
                out->dir_x = 1.0f;
                out->dir_y = 0.0f;
            }
        }
    }
    else
    {
        r_sq = 0.0f;
        r = 0.0f;
        norm = edge_sqrtf(reference_dir_x * reference_dir_x +
                          reference_dir_y * reference_dir_y);
        if ((reference_valid != 0U) && (norm > 0.0f))
        {
            out->dir_x = reference_dir_x / norm;
            out->dir_y = reference_dir_y / norm;
        }
        else
        {
            out->dir_x = 1.0f;
            out->dir_y = 0.0f;
        }
    }

    /* ---- 判定颠簸路段: 全图 N 达标且结构张量 L² 达标 ---- */
    out->is_bumpy = (edge_accum.strong_count > EDGE_MIN_STRONG_N
                     && r_sq > EDGE_R_SQ_BUMPY) ? 1U : 0U;
    out->coherence_r = r;
    out->strong_count = edge_accum.strong_count;
    out->total_pixels = edge_accum.total_pixels;
    out->max_gradient_mag = max_gradient_mag;
}

#endif /* !BUMPY_USE_NEW_PIPELINE: 旧管线 */

volatile runtime_profiler_t g_bumpy_vision_cost_profiler = {0};
volatile runtime_profiler_t g_bumpy_vision_frame_profiler = {0};
volatile bumpy_vision_output_t g_bumpy_vision_output = {0};
volatile uint8 g_bumpy_vision_output_write_busy = 0U;

static bumpy_vision_output_t g_bumpy_vision_output_shadow;
#if BUMPY_VISION_PROFILE_ENABLE
static uint32 g_bumpy_last_frame_time_us = 0U;
#endif

#if BUMPY_USE_NEW_PIPELINE
/* —— 新管线实例与辅助（2026-08-17 落地）—— */
static bumpy_pipeline_t s_bumpy_pipeline;   /* 新管线状态（gx/gy/mag2/labels 等，~121KB） */

/* —— lateral 中线 v4（2026-08-19：IPM 后点簇直方图，边线-中线链条极简；中线严禁任何时间滤波）——
   实测依据（12 视频 2365 帧人工标注 + 5m/s 等效隔帧仿真，见 trials/bumpy-road-new/pc_tools/lat_study）：
   检出高度离散且几乎永远单边；5m/s 等效全段仅 0~6 个有效观测（中位≈2）。
   设计（2026-08-19 用户指示）：中线不做任何滤波 —— 边线侧主带提取（离散性处理）已足够；
   中线配合 0 核锁存、只在起飞/脱出时刻一次性取用（BumpyRoad_ApplyExitCorrection 读 IPC 直通值），
   时间平滑（采信窗/EMA/限速）只会污染"起飞一刻"的读数，故彻底删除。
   链路：边线域瞬时质量门（IPM 后物理 x 主带提取，兼作位置解算）→ 本帧瞬时合成中线，直出。
   ① 边线提取（v3）：pipeline 输出的左右外点(像素)逐点 IPM → 物理 x 主带（中值 ±100mm、
      占比 ≥0.78 且 ≥7 点）→ 主带内点物理 x 均值 = 边线物理 x。
      质量门=主带提取本身，无直线拟合/无基准行外推（替代 RANSAC+基准行）。
   ② 合成（v3）：双侧间距自检(1000±150mm) → 中点；单侧 ±500mm 逆推；全无 → 0（无观测）。
   ③ 输出：lateral_mm = 本帧瞬时合成值（无滤波）；meas_valid = 本帧有横向观测。
   ④ 双侧间距不符(非 1000±150mm, 含同一物理边被拆成两条)一律扔掉, 无观测 (抢救已删除, 2026-08-19)。
   状态按路段经 bumpy_vision_reset_filter 隔离。 */
#define BUMPY_CLS_MIN_PTS      (7U)     /* 物理 x 主带：有效 IPM 点/主带点数下限 (2026-08-19 4→7):
                                           断裂点/幻影(条纹局部右端) 主带点数 4~6, GT 真边线 98% ≥7
                                           (全库 diag 统计: L 50/58 + R 92/97 ≥7), 提取时即过滤 */
#define BUMPY_CLS_TOL_X_MM     (100.0f) /* 物理 x 主带：中值容差(±) */
#define BUMPY_CLS_MIN_RATIO    (0.78f)  /* 物理 x 主带：主带点数占比下限 */

/**
 * @brief 边线提取 (v3, 2026-08-19): 外点逐点 IPM → 物理 x 主带 → 主带内点均值 = 边线物理 x
 * @note  真边线物理 X 恒定 → 外点 IPM 后 X 聚成一团；带中幻影/顺条纹错检 X 散布大。
 *        参数与 v2 点簇门一致（178 条人工标注：正确线一致率 1.0，错检 ≤0.75，零误杀）。
 *        同时输出主带内点的像素坐标均值（供图传渲染画竖直边线，wifi.c）。
 * @return 1=成功（*x_mm_out = 主带内点物理 x 均值；*pix_cx/*pix_cy = 像素均值），0=失败
 */
static int bumpy_vision_edge_x_mm(const int16_t *px, const int16_t *py, uint8_t n,
                                  float *x_mm_out, float *pix_cx, float *pix_cy)
{
    float xs[BP_OUT_MAX];
    uint8_t nv = 0, i, j;
    int cnt = 0;
    float med, sum = 0.0f, sx = 0.0f, sy = 0.0f;

    /* 遍1: IPM 全部有效点 → xs[] 求中值 */
    for (i = 0U; (i < n) && (i < BP_OUT_MAX); i++)
    {
        IPM_Point_t p = IPM_GetPhysicalCoord((uint8_t)px[i], (uint8_t)py[i]);
        if (p.is_valid && (p.x_mm != IPM_INVALID_VAL))
        {
            xs[nv++] = (float)p.x_mm;
        }
    }
    if (nv < BUMPY_CLS_MIN_PTS)
    {
        return 0;
    }
    /* 插入排序求中值（nv ≤ 48，代价可忽略） */
    for (i = 1U; i < nv; i++)
    {
        const float v = xs[i];
        for (j = i; (j > 0U) && (xs[j - 1U] > v); j--)
        {
            xs[j] = xs[j - 1U];
        }
        xs[j] = v;
    }
    med = xs[nv >> 1];
    /* 遍2: 主带内点累加物理x/像素x/像素y */
    for (i = 0U; (i < n) && (i < BP_OUT_MAX); i++)
    {
        IPM_Point_t p = IPM_GetPhysicalCoord((uint8_t)px[i], (uint8_t)py[i]);
        if (p.is_valid && (p.x_mm != IPM_INVALID_VAL) &&
            (fabsf((float)p.x_mm - med) <= BUMPY_CLS_TOL_X_MM))
        {
            cnt++;
            sum += (float)p.x_mm;
            sx += (float)px[i];
            sy += (float)py[i];
        }
    }
    if (((float)cnt >= (float)BUMPY_CLS_MIN_PTS) &&
        ((float)cnt >= BUMPY_CLS_MIN_RATIO * (float)nv))
    {
        *x_mm_out = sum / (float)cnt;   /* 主带内点物理 x 均值 = 边线物理 x */
        *pix_cx = sx / (float)cnt;
        *pix_cy = sy / (float)cnt;
        return 1;
    }
    return 0;
}

/* —— 角度响应整形（2026-08-18 由 0 核 vision_bumpy_control.c 上移，与 valid/横向完全无关）——
   按角度大小修改响应：小偏差迅速修正、大偏差慢速修正。
   ① 静态整形 S(e)=e/(1+B·|e|)：|e| 大 → 增益小 → 阻尼；|e|→0 → 增益→1 → 灵敏
   ② 自适应 EMA α=ALPHA_MAX·exp(−|e|/TAU)：小角度 α 大 → 跟手；大角度 α 小 → 压抖
   ③ 限幅 + 死区。
   SIGN 在本函数内应用，0 核仅直通，最终 err_degree 符号与原实现一致。
   调用方保证：仅 hdg 有效时调用；无条纹时向 0 核报 0（EMA 状态保持不更新）。 */
static float s_yaw_shaped_deg = 0.0f;   /* 角度整形+EMA 状态（按路段复位） */

/* 条纹倾角时间滤波 (2026-08-19): 圆域向量 EMA (mod 180°, 对 2θ 的 (cos,sin) 分量滤波),
   仅在 hdg_valid 帧更新; 无效帧保持并输出 0 (归零语义由上方 hdg_valid 分支保证) */
#ifndef VISION_BUMPY_HDG_EMA_ALPHA
#define VISION_BUMPY_HDG_EMA_ALPHA  (0.40f)
#endif
static float s_hdg_ex = 0.0f;           /* EMA 状态: Σcos2θ 分量 */
static float s_hdg_ey = 0.0f;           /* EMA 状态: Σsin2θ 分量 */

static float bumpy_vision_shape_yaw_error(float raw_deg)
{
    float err;
    const float raw = raw_deg * VISION_BUMPY_YAW_SIGN;

    /* ① 静态整形：|e|→0 增益→1（小偏差灵敏）；|e|→∞ 趋 1/B 饱和（大偏差阻尼） */
    err = raw / (1.0f + VISION_BUMPY_ANGLE_SHAPE_B * fabsf(raw));

    /* ② 自适应 EMA：α 随 |e| 指数衰减（小角度跟手、大角度压抖） */
    {
        const float alpha = VISION_BUMPY_ANGLE_ALPHA_MAX *
                            expf(-fabsf(raw) / VISION_BUMPY_ANGLE_TAU_DEG);
        err = alpha * err + (1.0f - alpha) * s_yaw_shaped_deg;
    }
    s_yaw_shaped_deg = err;

    /* ③ 限幅 + 死区 */
    if (err >  VISION_BUMPY_MAX_ERR_DEG) err =  VISION_BUMPY_MAX_ERR_DEG;
    if (err < -VISION_BUMPY_MAX_ERR_DEG) err = -VISION_BUMPY_MAX_ERR_DEG;
    if (fabsf(err) < VISION_BUMPY_DEADBAND_DEG)
    {
        err = 0.0f;
    }
    return err;
}

/* (v3, 2026-08-19) 基准行 IPM 交点解算 bumpy_vision_edge_x_at_base_row 已删除:
   边线物理 x 直接由主带内点均值得出 (bumpy_vision_edge_x_mm), 无需直线/角度/基准行. */
#endif

void bumpy_vision_reset_filter(void)
{
    bumpy_vision_output_t empty;

#if BUMPY_USE_NEW_PIPELINE
    bumpy_pipeline_init(&s_bumpy_pipeline);   /* 时间验证历史按路段/视频隔离 */
    /* 角度/倾角 EMA 状态随路段复位（中线无滤波状态，无复位项） */
    s_yaw_shaped_deg = 0.0f;  /* 角度整形 EMA 状态随路段复位 */
    s_hdg_ex = 0.0f;          /* 倾角圆域 EMA 状态随路段复位 */
    s_hdg_ey = 0.0f;
#endif

    memset(&empty, 0, sizeof(empty));
    empty.direction_y = 1.0f;
    g_bumpy_vision_output_shadow = empty;
    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = empty;
    g_bumpy_vision_output_write_busy = 0U;
}

void bumpy_vision_init(void)
{
#if BUMPY_USE_NEW_PIPELINE
    bumpy_pipeline_init(&s_bumpy_pipeline);
#else
    bumpy_edge_detect_init();
#endif
    bumpy_vision_reset_filter();

#if BUMPY_VISION_PROFILE_ENABLE
    timer_init(BUMPY_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(BUMPY_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_bumpy_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_bumpy_vision_frame_profiler);
    g_bumpy_last_frame_time_us = timer_get(BUMPY_VISION_PROFILE_TIMER);
#endif
}

const volatile bumpy_vision_output_t *bumpy_vision_get_output(void)
{
    return &g_bumpy_vision_output;
}

void bumpy_vision_process_camera_frame(const uint8 *gray)
{
#if BUMPY_USE_NEW_PIPELINE
    /* ============================================================
     * 新管线（2026-08-17 落地）：输入 94×60 压缩图
     *   bumpy_pipeline_frame → 左右边线 → 对准 IPC 契约字段
     *   （bumpy_detected / direction_x/y / yaw_error_deg_x100 /
     *     lateral_mm / meas_valid，详见 docs/任务规划/颠簸路段新视觉管线落地文档.md）
     * ============================================================ */
    bumpy_frame_result_t res;
    bumpy_vision_output_t next;

    if (gray == NULL)
    {
        return;
    }

    bumpy_pipeline_frame(&s_bumpy_pipeline, gray, &res);

    next.frame_id        = g_bumpy_vision_output_shadow.frame_id + 1U;
    /* bumpy_detected = 条纹存在性（hdg_valid：检出 ≥BP_MIN_HDG_LINES 条横向条纹即 1，
       与边线成败无关）。0 核入口/出口判定（连续 3 帧 1/0）依赖该位，语义应为
       "是否在颠簸路段"；若绑到 L/R 边线时间验证（hist_update 需连续 3 帧角度/位置稳定），
       颠簸振动会让边线频繁失效 → bumpy_detected 在段内反复变 0 → 0 核误判"脱出"
       提前结束任务（2026-08-18 修复：改用 hdg_valid，且要求最少 N 条横向线）。 */
    next.bumpy_detected  = (uint8)(res.hdg_valid ? 1U : 0U);
    next.coherence_r     = 0.0f;   /* 新管线无 R² 概念，保留字段置 0 */
    next.strong_count    = 0U;
    next.total_pixels    = 0U;
    next.max_gradient_mag = 0U;
    next.direction_x     = 0.0f;
    next.direction_y     = 1.0f;
    next.yaw_error_deg_x100 = 0;
    next.lateral_mm      = 0;
    next.meas_valid      = 0U;

    /* 条纹倾斜角：永远取自管线 frame_heading（与边线成败无关，常输出）；
       角度路径与 valid/横向完全解耦；无条纹时向 0 核报 0（0 核不做任何视觉可信度锁） */
    if (res.hdg_valid)
    {
        /* 圆域向量 EMA 滤波 (2026-08-19): 压帧间抖动, 大跳变时 EMA 天然慢跟 */
        const float a2 = 2.0f * res.hdg * 0.01745329251f;
        s_hdg_ex += VISION_BUMPY_HDG_EMA_ALPHA * (cosf(a2) - s_hdg_ex);
        s_hdg_ey += VISION_BUMPY_HDG_EMA_ALPHA * (sinf(a2) - s_hdg_ey);
        const float hdg_f = 0.5f * atan2f(s_hdg_ey, s_hdg_ex) * 57.2957795f;
        const float arad = hdg_f * 0.01745329251f;
        next.direction_x = cosf(arad);
        next.direction_y = sinf(arad);
        /* 偏差角度：条纹法向相对车头(图像 +y/前)，正=需右转；
           经“按角度大小整形+自适应EMA”（1 核，小偏差快修/大偏差慢修）输出稳定提案；
           SIGN 在 1 核内应用，0 核仅直通，最终 err_degree 符号与原实现一致 */
        const float raw_deg = atan2f(-sinf(arad), cosf(arad)) * 57.2957795f;
        next.yaw_error_deg_x100 = (int16)(bumpy_vision_shape_yaw_error(raw_deg) * 100.0f);
    }
    /* hdg 无效（无条纹）：yaw_error_deg_x100 保持 0 上报，EMA 状态保持不更新 */

    /* 横向偏差 (v4, 2026-08-19): 外点逐点 IPM → 物理 x 主带 → 主带内点均值 = 边线物理 x
       (链条极简; 无直线拟合/无基准行)。双侧: 间距自检(1m±150mm) → 中点;
       单侧: ±500mm 逆解算; 两侧都没有: 0 (无观测)。
       中线严禁任何时间滤波(用户指示): lateral_mm 直出本帧瞬时合成值 —— 边线主带提取
       (离散性处理)已足够; 中线配合 0 核锁存只在起飞/脱出时刻一次性取用, 平滑无必要. */
    {
        float xl = 0.0f, xr = 0.0f;
        float lcx = 0.0f, lcy = 0.0f, rcx = 0.0f, rcy = 0.0f;
        float lateral_raw = 0.0f;
        int ok_l = bumpy_vision_edge_x_mm(res.lp_x, res.lp_y, res.lp_n, &xl, &lcx, &lcy);
        int ok_r = bumpy_vision_edge_x_mm(res.rp_x, res.rp_y, res.rp_n, &xr, &rcx, &rcy);

        /* (2026-08-19 简化) 过门边线物理 x + 像素表示透出 (诊断/图传渲染用, 不进 IPC, v3) */
        next.edge_l_xmm = ok_l ? (int16)xl : 0;
        next.edge_r_xmm = ok_r ? (int16)xr : 0;
        next.line_l.valid = ok_l ? 1 : 0;
        next.line_l.ang = 90.0f;   /* 竖直近似 (主带内点像素均值, 供 wifi.c 画线) */
        next.line_l.cx = lcx;
        next.line_l.cy = lcy;
        next.line_l.n = 0;
        next.line_r.valid = ok_r ? 1 : 0;
        next.line_r.ang = 90.0f;
        next.line_r.cx = rcx;
        next.line_r.cy = rcy;
        next.line_r.n = 0;

        /* 双侧合成 (2026-08-19 简化): 仅"标准 1m 间距(1000±150mm)"认作有效双边 → 中点;
           间距不符(过近 = 同一物理边被拆成两条/断裂点幻影, 或过远)一律扔掉, 无观测 ——
           抢救分支已删除(用户指示): 不再"信当前估计选边", 避免锁死延续 */
        if ((ok_l != 0) && (ok_r != 0))
        {
            if (fabsf((xr - xl) - (float)BUMPY_EDGE_SPACING_MM) <= (float)BUMPY_WIDTH_TOL_MM)
            {
                lateral_raw = -(xl + xr) * 0.5f;   /* 车身偏右为正 */
            }
            /* 间距不符: 无观测 (lateral_raw 保持 0) */
        }
        else if (ok_l)
        {
            lateral_raw = -(xl + (float)BUMPY_HALF_SPACING_MM);
        }
        else if (ok_r)
        {
            lateral_raw = -(xr - (float)BUMPY_HALF_SPACING_MM);
        }

        next.lateral_mm = (int16)lateral_raw;   /* 瞬时合成值，无任何时间滤波 */
        /* valid 只服务横向/边线（0 核出口修正门）：= 本帧有横向观测（合成成功），
           与角度(hdg)解耦；无观测帧 lateral_mm=0 且 valid=0（观测存在性，非历史锁定）。 */
        next.meas_valid = (lateral_raw != 0.0f) ? 1U : 0U;
    }

    /* 横向条纹连通域透出（仅渲染用，不进 IPC） */
    next.lin_n = res.lin_n;
    memcpy((void *)next.lin_cx, res.lin_cx, sizeof(next.lin_cx));
    memcpy((void *)next.lin_cy, res.lin_cy, sizeof(next.lin_cy));
    memcpy((void *)next.lin_ang, res.lin_ang, sizeof(next.lin_ang));
    memcpy((void *)next.lin_pix, res.lin_pix, sizeof(next.lin_pix));

    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = next;
    g_bumpy_vision_output_shadow = next;
    g_bumpy_vision_output_write_busy = 0U;
#else
    /* ============================================================
     * 旧算法（BUMPY_USE_NEW_PIPELINE=0）：输入 188×120 原图
     * ============================================================ */
    bumpy_edge_detect_output_t edge = {0};
    bumpy_vision_output_t next = g_bumpy_vision_output_shadow;

    if (gray == NULL)
    {
        return;
    }

#if BUMPY_VISION_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(BUMPY_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_bumpy_vision_frame_profiler,
                                (uint32)(now_us - g_bumpy_last_frame_time_us));
        g_bumpy_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif

    bumpy_edge_detect_process(gray,
                               (uint8)(next.frame_id != 0U),
                               next.direction_x,
                               next.direction_y,
                               &edge);

    next.frame_id++;
    next.bumpy_detected = edge.is_bumpy;
    next.direction_x = edge.dir_x;
    next.direction_y = edge.dir_y;
    next.coherence_r = edge.coherence_r;
    next.strong_count = edge.strong_count;
    next.total_pixels = edge.total_pixels;
    next.max_gradient_mag = edge.max_gradient_mag;

    /* =====================================================================
     * 新视觉预留接口（2026-08-17 规划 §3，详见 docs/任务规划/颠簸路段视觉跨核接口规划.md）
     *   视觉核需直接输出两个物理量 + 一个可信位：
     *     1) yaw_error_deg_x100：偏差角度（条纹主方向相对车头，正值=需右转，×100）
     *     2) lateral_mm        ：水平方向偏差（车身偏右为正，单位 mm）
     *     3) meas_valid        ：本帧 yaw_error/lateral 是否可信
     *                             → 0 核经 IPC VISION_VALID_BUMPY_MEAS 消费
     * ===================================================================== */
    if (edge.is_bumpy != 0U)
    {
        /* 偏差角度：与 0 核 vision_bumpy_calc_err_degree() 同号（-atan2f(dir_x,dir_y)）。
         * 新视觉算法若直接输出角度，可跳过此换算直接填该字段。 */
        const float yaw_deg = -atan2f(edge.dir_x, edge.dir_y) * (180.0f / 3.14159265358979f);
        next.yaw_error_deg_x100 = (int16)(yaw_deg * 100.0f);

        /* TODO(新视觉)：计算水平方向偏差 lateral_mm
         *   - 用 IPM_GetPhysicalCoord() 求"条纹中心点"与"车辆中心点"物理 x 之差；
         *   - 约定：车身偏右为正（与单边桥 lateral_mm 口径一致）；
         *   - 未实现前保持 0；后续需同时校验 IPM 有效、|lateral| 未饱和，
         *     再置 meas_valid（当前暂由 is_bumpy 代表可信）。 */
        next.lateral_mm = 0;
        next.meas_valid = 1U;
    }
    else
    {
        next.yaw_error_deg_x100 = 0;
        next.lateral_mm = 0;
        next.meas_valid = 0U;
    }

    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = next;
    g_bumpy_vision_output_shadow = next;
    g_bumpy_vision_output_write_busy = 0U;

#if BUMPY_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif
#endif
}
