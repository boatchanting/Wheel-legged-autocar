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

void bumpy_vision_reset_filter(void)
{
    bumpy_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    empty.direction_y = 1.0f;
    g_bumpy_vision_output_shadow = empty;
    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = empty;
    g_bumpy_vision_output_write_busy = 0U;
}

void bumpy_vision_init(void)
{
    bumpy_edge_detect_init();
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

    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = next;
    g_bumpy_vision_output_shadow = next;
    g_bumpy_vision_output_write_busy = 0U;

#if BUMPY_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif
}
