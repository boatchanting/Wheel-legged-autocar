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
#define EDGE_R_SQ_BUMPY     (0.043f)  /* R² > 0.81 (对应 R > 0.9)     */ /*主要调这两个，滤除背景用*/
#define EDGE_MIN_STRONG_N   (300U)   /* 强边缘数 > 128 才判定        */ /*主要调这两个，判别面积用，体现在远近上*/

/* ==========================================================================
 * DTCM 环形缓冲 (模块私有, 0 等待数据访问)
 * ========================================================================== */

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
            energy += (uint64)((int64)gx * gx + (int64)gy * gy);
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

static void bumpy_edge_detect_process(const uint8_t *gray, bumpy_edge_detect_output_t *out)
{
    edge_dir_result_t edge_accum;
    int ring_idx;
    int row;
    uint64 strong_energy = 0U;
    uint16 max_gradient_mag = 0U;
    float r_sq, norm;

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
                                                                   &max_gradient_mag);
        }

        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 计算 R² 和方向向量 ---- */
    if ((edge_accum.strong_count > 0U) && (strong_energy > 0U))
    {
        r_sq = (float)(((double)((int64)edge_accum.sum_gx * edge_accum.sum_gx +
                                 (int64)edge_accum.sum_gy * edge_accum.sum_gy)) /
                       ((double)edge_accum.strong_count * (double)strong_energy));
        if (r_sq > 1.0f)
        {
            r_sq = 1.0f;
        }

        /* 方向向量归一化 */
        norm = edge_sqrtf((float)((int64_t)edge_accum.sum_gx * edge_accum.sum_gx
                                + (int64_t)edge_accum.sum_gy * edge_accum.sum_gy));
        if (norm > 0.0f)
        {
            out->dir_x = (float)edge_accum.sum_gx / norm;
            out->dir_y = (float)edge_accum.sum_gy / norm;
        }
        else
        {
            out->dir_x = 1.0f;
            out->dir_y = 0.0f;
        }
    }
    else
    {
        r_sq = 0.0f;
        out->dir_x = 1.0f;
        out->dir_y = 0.0f;
    }

    /* ---- 判定颠簸路段: N > 128 且 R² > 0.81 ---- */
    out->is_bumpy = (edge_accum.strong_count > EDGE_MIN_STRONG_N
                     && r_sq > EDGE_R_SQ_BUMPY) ? 1U : 0U;
    out->coherence_r = edge_sqrtf(r_sq);
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

static void bumpy_vision_clear_frame_result(bumpy_vision_frame_result_t *result)
{
    memset(result, 0, sizeof(*result));
    result->bbox_xmin = 0xFFU;
    result->bbox_ymin = 0xFFU;
    result->bbox_xmax = 0xFFU;
    result->bbox_ymax = 0xFFU;
    result->centerline_top_y = 0xFFU;
    result->centerline_bottom_y = 0xFFU;
}

static void bumpy_vision_make_frame_result(const bumpy_edge_detect_output_t *edge,
                                           bumpy_vision_frame_result_t *result)
{
    bumpy_vision_clear_frame_result(result);
    result->detected = edge->is_bumpy;
    result->phase = edge->is_bumpy ? BUMPY_PHASE_INSIDE : BUMPY_PHASE_UNCERTAIN;
    result->mode = edge->is_bumpy ? BUMPY_MODE_FOLLOW_CENTERLINE : BUMPY_MODE_FALLBACK_SEARCH;
    result->confidence_u16 = edge->is_bumpy ? 1000U : 0U;
    result->direction_x = edge->dir_x;
    result->direction_y = edge->dir_y;
    result->coherence_r = edge->coherence_r;
    result->strong_count = edge->strong_count;
    result->total_pixels = edge->total_pixels;
    result->max_gradient_mag = edge->max_gradient_mag;
}

void bumpy_vision_reset_filter(void)
{
    bumpy_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    bumpy_vision_clear_frame_result(&empty.raw);
    bumpy_vision_clear_frame_result(&empty.stable);
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

    bumpy_edge_detect_process(gray, &edge);

    next.frame_id++;
    bumpy_vision_make_frame_result(&edge, &next.raw);
    next.raw_detected = edge.is_bumpy;
    next.stable = next.raw;
    next.stable_detected = edge.is_bumpy;
    next.detected_streak = edge.is_bumpy ?
        (uint8)((next.detected_streak < 255U) ? (next.detected_streak + 1U) : 255U) : 0U;
    next.lost_streak = edge.is_bumpy ? 0U :
        (uint8)((next.lost_streak < 255U) ? (next.lost_streak + 1U) : 255U);
    next.start_seen = (uint8)(next.start_seen || edge.is_bumpy);
    next.end_seen = (uint8)((g_bumpy_vision_output_shadow.stable_detected != 0U) &&
                             (edge.is_bumpy == 0U));

    g_bumpy_vision_output_write_busy = 1U;
    g_bumpy_vision_output = next;
    g_bumpy_vision_output_shadow = next;
    g_bumpy_vision_output_write_busy = 0U;

#if BUMPY_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_bumpy_vision_cost_profiler, BUMPY_VISION_PROFILE_TIMER);
#endif
}
