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

/* —— lateral 中线滤波（2026-08-18：左右边线直接合成中线 → 3 帧满窗中值 + 可信度）——
   结构：raw_L/raw_R（原始单帧边线，不单独滤波）→ lateral_raw（左右直接合成中线/单侧逆推）
         → 本滤波器。
   ① 每来一个有效中线数据（lateral_raw≠0 且非野值 |Δx|≤JERK）→ 推进 3 帧中值窗；
   ② 3 帧满窗后：中值直接输出（LOCK），可信度线性累加（封顶）；
   ③ 无输入/野值 → 中值窗不推进、lat_stable 保持（HOLD），可信度按递增步长扣减；
      可信度耗尽（=0）→ freeze=1 剥离 meas_valid（值仍保持，不污染）。
   0 值帧（无线/间距自检失败）不入窗，从源头消除 0 值污染偏置。 */
#define BUMPY_LAT_FILTER_N         (3U)    /* 中值窗长：满 3 个有效帧才输出（滑动窗，每帧推进） */
#define BUMPY_LAT_JERK_MM          (80.0f) /* 单帧野值门限：|Δx| > J 视为野值，不入窗 */
#define BUMPY_LAT_CONF_MAX         (255U)  /* 横向可信度上限（0~255） */
#define BUMPY_LAT_CONF_STEP        (10U)   /* 有效稳定帧线性累加步长（26 帧满置信） */
#define BUMPY_LAT_CONF_GRACE       (5U)    /* 丢失免扣帧数（5 帧内不扣可信度） */
#define BUMPY_LAT_CONF_PENALTY_INC (1U)    /* 免扣期后每帧递增扣分步长：第 1 帧扣 1、第 2 帧扣 2… */
static float   s_lat_win[BUMPY_LAT_FILTER_N];
static uint8_t s_lat_win_cnt;             /* 窗口内有效样本数（≤N） */
static uint8_t s_lat_miss;                /* 连续无观测/野值帧计数 */
static uint8_t s_lat_conf;                /* 横向可信度（0~255）：有效稳定帧线性累加，丢失非线性扣减 */
static uint8_t s_lat_penalty;             /* 非线性扣分步长：超过免扣期后每帧 +1 */
static float   s_lat_stable;              /* 稳定输出 lat_stable */

/**
 * @brief lateral 中线滤波（3 帧满窗中值 + 可信度）
 * @param lateral_raw 本帧由左右边线直接合成的中线偏差（0=无横向观测）
 * @param freeze      出参：1=可信度耗尽（剥离 meas_valid，值保持）
 * @return 稳定输出 lat_stable
 */
static float bumpy_vision_lateral_filter(float lateral_raw, uint8_t *freeze)
{
    int i;

    *freeze = 0U;

    /* 观测存在且非野值（|Δx| 相对当前稳定值）→ 有效稳定帧：推进中值窗 + 累加可信度 */
    if ((lateral_raw != 0.0f) &&
        (fabsf(lateral_raw - s_lat_stable) <= BUMPY_LAT_JERK_MM))
    {
        /* 可信度线性累加（封顶），重置丢失计数与扣分步长 */
        s_lat_conf += BUMPY_LAT_CONF_STEP;
        if (s_lat_conf > BUMPY_LAT_CONF_MAX)
        {
            s_lat_conf = BUMPY_LAT_CONF_MAX;
        }
        s_lat_miss = 0U;
        s_lat_penalty = 0U;

        /* 每来一个中线数据推进一次中值窗 */
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
            /* 满 3 窗：三值排序求中值 → 直接输出（不再 EMA） */
            float a = s_lat_win[0], b = s_lat_win[1], c = s_lat_win[2];
            float x_mid, t;
            if (a > b) { t = a; a = b; b = t; }
            if (b > c) { t = b; b = c; c = t; }
            if (a > b) { t = a; a = b; b = t; }
            x_mid = b;
            s_lat_stable = x_mid;
        }
    }
    else
    {
        /* 无观测/野值：丢失计数；免扣期（GRACE=5 帧）内不扣可信度；
           超过后按递增步长扣减；可信度耗尽（=0）才 FREEZE 剥离 meas_valid */
        if (s_lat_miss < 0xFFU)
        {
            s_lat_miss++;
        }
        if (s_lat_miss > BUMPY_LAT_CONF_GRACE)
        {
            s_lat_penalty += BUMPY_LAT_CONF_PENALTY_INC;
            if (s_lat_conf >= s_lat_penalty)
            {
                s_lat_conf -= s_lat_penalty;
            }
            else
            {
                s_lat_conf = 0U;
            }
        }
        if (s_lat_conf == 0U)
        {
            *freeze = 1U;   /* 可信度耗尽：冻结并剥离 meas_valid */
        }
    }

    return s_lat_stable;
}

/* —— 角度响应整形（2026-08-18 由 0 核 vision_bumpy_control.c 上移，与 valid/横向完全无关）——
   按角度大小修改响应：小偏差迅速修正、大偏差慢速修正。
   ① 静态整形 S(e)=e/(1+B·|e|)：|e| 大 → 增益小 → 阻尼；|e|→0 → 增益→1 → 灵敏
   ② 自适应 EMA α=ALPHA_MAX·exp(−|e|/TAU)：小角度 α 大 → 跟手；大角度 α 小 → 压抖
   ③ 限幅 + 死区。
   SIGN 在本函数内应用，0 核仅直通，最终 err_degree 符号与原实现一致。
   调用方保证：仅 hdg 有效时调用；无条纹时向 0 核报 0（EMA 状态保持不更新）。 */
static float s_yaw_shaped_deg = 0.0f;   /* 角度整形+EMA 状态（按路段复位） */

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
    /* lateral 滤波状态按路段/视频隔离 */
    s_lat_win_cnt = 0U;
    s_lat_miss = 0U;
    s_lat_conf = 0U;
    s_lat_penalty = 0U;
    s_lat_stable = 0.0f;
    s_yaw_shaped_deg = 0.0f;  /* 角度整形 EMA 状态随路段复位 */
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
        const float arad = res.hdg * 0.01745329251f;
        next.direction_x = cosf(arad);
        next.direction_y = sinf(arad);
        /* 偏差角度：条纹法向相对车头(图像 +y/前)，正=需右转；
           经“按角度大小整形+自适应EMA”（1 核，小偏差快修/大偏差慢修）输出稳定提案；
           SIGN 在 1 核内应用，0 核仅直通，最终 err_degree 符号与原实现一致 */
        const float raw_deg = atan2f(-sinf(arad), cosf(arad)) * 57.2957795f;
        next.yaw_error_deg_x100 = (int16)(bumpy_vision_shape_yaw_error(raw_deg) * 100.0f);
    }
    /* hdg 无效（无条纹）：yaw_error_deg_x100 保持 0 上报，EMA 状态保持不更新 */

    /* 横向偏差：固定 y=BUMPY_IPM_BASE_ROW 行 IPM + 已知 1m 边线间距（审批方案 §3.2）
       双侧：中点 + 间距自检；单侧：±500mm 逆解算；两侧都没有：0
       → 稳定滤波（3帧满窗中值+门控+保持状态机，2026-08-18 方案 v5 §3）后输出 lat_stable */
    {
        float xl = 0.0f, xr = 0.0f;
        float lateral_raw = 0.0f;
        uint8_t lat_freeze = 0U;
        const int ok_l = (res.raw_L.valid && bumpy_vision_edge_x_at_base_row(&res.raw_L, &xl));
        const int ok_r = (res.raw_R.valid && bumpy_vision_edge_x_at_base_row(&res.raw_R, &xr));

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
        /* valid 只服务横向/边线（0 核出口修正门）：纯置信度驱动，与角度(hdg)解耦；
           lat_stable 值本身与置信度无关（HOLD 保持、不随 conf 清零）；conf 耗尽才 valid=0。 */
        next.meas_valid = (s_lat_conf > 0U) ? 1U : 0U;
    }

    /* 原始边线透出（仅渲染用，不进 IPC；raw 为单帧拟合，未时间验证） */
    next.line_l = res.raw_L;
    next.line_r = res.raw_R;

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
