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

/* —— lateral 稳定滤波（2026-08-18 方案 v5 §3，docs/任务规划/颠簸视觉角度响应整形与横向偏差稳定滤波方案.md）——
   3 帧满窗中值（替代旧"连续3帧"时序门控）+ 门控（观测存在/野值剔除） + LOCK/HOLD/FREEZE 状态机：
     LOCK  : 满 3 个可检测帧 → 中值 → EMA 更新（miss 清零）
     HOLD  : 无观测/野值且 miss≤K → 保持 s_lat_stable 不更新（丢失 5 帧内仍可信）
     FREEZE: miss>K → 冻结并剥离 meas_valid（唯一"横向完全不可信"出口）
   0 值帧（无线/间距自检失败）不入窗，从源头消除 0 值污染偏置（见方案 §3.3.2e）。 */
#define BUMPY_LAT_FILTER_N         (3U)    /* 中值窗长：必须满 3 个可检测帧（lateral_raw≠0 且非野值）才输出 */
#define BUMPY_LAT_JERK_MM          (80.0f) /* 单帧野值门限：|Δx| > J 视为野值，不入窗直接进 HOLD */
#define BUMPY_LAT_EMA_BETA         (0.30f) /* LOCK 态 EMA 系数（0~1，越小越平滑） */
#define BUMPY_LAT_HOLD_MAX         (5U)    /* HOLD 保持上限帧（50ms@100fps） */
static float   s_lat_win[BUMPY_LAT_FILTER_N];
static uint8_t s_lat_win_cnt;             /* 窗口内有效样本数（≤N） */
static uint8_t s_lat_miss;                /* 连续无观测/野值帧计数 */
static float   s_lat_stable;              /* 稳定输出 lat_stable */

/**
 * @brief lateral 稳定滤波（方案 v5 §3.2）
 * @param lateral_raw 本帧原始横向偏差（0=无横向观测）
 * @param freeze      出参：1=FREEZE（长期丢失，需剥离 meas_valid）
 * @return 稳定滤波值 lat_stable（LOCK 更新 / HOLD 保持 / FREEZE 冻结）
 */
static float bumpy_vision_lateral_filter(float lateral_raw, uint8_t *freeze)
{
    int i;

    *freeze = 0U;

    /* C1 观测存在且非野值（|Δx| 相对当前稳定值）→ 入窗 */
    if ((lateral_raw != 0.0f) &&
        (fabsf(lateral_raw - s_lat_stable) <= BUMPY_LAT_JERK_MM))
    {
        if (s_lat_win_cnt < BUMPY_LAT_FILTER_N)
        {
            s_lat_win_cnt++;
        }
        for (i = (int)BUMPY_LAT_FILTER_N - 1; i > 0; i--)
        {
            s_lat_win[i] = s_lat_win[i - 1];
        }
        s_lat_win[0] = lateral_raw;

        if (s_lat_win_cnt >= BUMPY_LAT_FILTER_N)
        {
            /* 满 3 窗：三值排序求中值 → EMA 更新（LOCK） */
            float a = s_lat_win[0], b = s_lat_win[1], c = s_lat_win[2];
            float x_mid, t;
            if (a > b) { t = a; a = b; b = t; }
            if (b > c) { t = b; b = c; c = t; }
            if (a > b) { t = a; a = b; b = t; }
            x_mid = b;
            s_lat_stable += (x_mid - s_lat_stable) * BUMPY_LAT_EMA_BETA;
            s_lat_miss = 0U;
        }
    }
    else
    {
        /* 无观测/野值：HOLD 计数；超上限 FREEZE */
        if (s_lat_miss < 0xFFU)
        {
            s_lat_miss++;
        }
        if (s_lat_miss > BUMPY_LAT_HOLD_MAX)
        {
            *freeze = 1U;
        }
    }

    return s_lat_stable;
}

/**
 * @brief 边线与图像第 BUMPY_IPM_BASE_ROW 行交点的物理 x（IPM 查表）
 * @note  边线 IPM 后基本竖直，固定行求 x 即可代表整条边线位置，
 *        避免"解算点随可见边线长度漂移"（审批方案 §3.2）
 * @return 1=成功（*x_mm_out 有效），0=失败（IPM 无效/交点出图/线近水平不可交）
 */
static int bumpy_vision_edge_x_at_base_row(const bumpy_line_t *line, float *x_mm_out)
{
    IPM_Point_t p;
    const float arad = line->ang * 0.01745329251f;
    const float s = sinf(arad);     /* 像素系 x 向右/y 向下，直线方向 (cos,sin) */

    if (fabsf(s) < 0.0872f)         /* <sin5°：近水平，与水平基准行不可交，降级用拟合中心 */
    {
        p = IPM_GetPhysicalCoord((uint8_t)(line->cx + 0.5f), (uint8_t)(line->cy + 0.5f));
    }
    else
    {
        const float x_pix = line->cx + ((float)BUMPY_IPM_BASE_ROW - line->cy) * cosf(arad) / s;
        if (x_pix < 0.0f || x_pix > (float)(IPM_IMG_WIDTH - 1))
        {
            return 0;
        }
        p = IPM_GetPhysicalCoord((uint8_t)(x_pix + 0.5f), (uint8_t)BUMPY_IPM_BASE_ROW);
    }

    if (!p.is_valid || (p.x_mm == IPM_INVALID_VAL))
    {
        return 0;
    }
    *x_mm_out = (float)p.x_mm;
    return 1;
}
#endif

void bumpy_vision_reset_filter(void)
{
    bumpy_vision_output_t empty;

#if BUMPY_USE_NEW_PIPELINE
    bumpy_pipeline_init(&s_bumpy_pipeline);   /* 时间验证历史按路段/视频隔离 */
    /* lateral 稳定滤波状态按路段/视频隔离（与管线历史同生命周期，2026-08-18 v5 §5.4） */
    s_lat_win_cnt = 0U;
    s_lat_miss = 0U;
    s_lat_stable = 0.0f;
    memset(s_lat_win, 0, sizeof(s_lat_win));
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
    next.bumpy_detected  = (uint8)((res.L.valid || res.R.valid) ? 1U : 0U);
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
       valid 位只标"条纹也没有"的严重丢失（审批方案 §3.1） */
    if (res.hdg_valid)
    {
        const float arad = res.hdg * 0.01745329251f;
        next.direction_x = cosf(arad);
        next.direction_y = sinf(arad);
        /* 偏差角度：条纹法向相对车头(图像 +y/前)，正=需右转（0核 VISION_BUMPY_YAW_SIGN 兜底验符号） */
        next.yaw_error_deg_x100 = (int16)(atan2f(-sinf(arad), cosf(arad)) * 57.2957795f * 100.0f);
        next.meas_valid = 1U;
    }

    /* 横向偏差：固定 y=BUMPY_IPM_BASE_ROW 行 IPM + 已知 1m 边线间距（审批方案 §3.2）
       双侧：中点 + 间距自检；单侧：±500mm 逆解算；两侧都没有：0
       → 稳定滤波（3帧满窗中值+门控+保持状态机，2026-08-18 方案 v5 §3）后输出 lat_stable */
    {
        float xl = 0.0f, xr = 0.0f;
        float lateral_raw = 0.0f;
        uint8_t lat_freeze = 0U;
        const int ok_l = (res.L.valid && bumpy_vision_edge_x_at_base_row(&res.L, &xl));
        const int ok_r = (res.R.valid && bumpy_vision_edge_x_at_base_row(&res.R, &xr));

        if (ok_l && ok_r)
        {
            /* 物理 x 向右为正，右边线 x 应比左边线大约 BUMPY_EDGE_SPACING_MM */
            if (fabsf((xr - xl) - (float)BUMPY_EDGE_SPACING_MM) <= (float)BUMPY_WIDTH_TOL_MM)
            {
                lateral_raw = -(xl + xr) * 0.5f;   /* 车身偏右为正 */
            }
            /* 间距自检失败：lateral_raw 保持 0（无横向观测），meas_valid 不受影响 */
        }
        else if (ok_l)
        {
            lateral_raw = -(xl + (float)BUMPY_HALF_SPACING_MM);
        }
        else if (ok_r)
        {
            lateral_raw = -(xr - (float)BUMPY_HALF_SPACING_MM);
        }

        next.lateral_mm = (int16)bumpy_vision_lateral_filter(lateral_raw, &lat_freeze);
        if (lat_freeze != 0U)
        {
            next.meas_valid = 0U;   /* FREEZE：横向长期丢失，角度一并剥离可信（条纹同样长期缺失） */
        }
    }

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
