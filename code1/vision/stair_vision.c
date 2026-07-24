/**
 * @file    stair_vision.c
 * @brief   台阶检测视觉模块 — Core 1 算法封装层实现 (V10)
 * @details 完整 V10 台阶检测流水线，SRAM 全图 Gx/Gy 存储。
 * @date    2026-07-24
 */

#include "stair_vision.h"
#include "tcm.h"
#include <string.h>


/* ==========================================================================
 * SRAM 全图 Gx/Gy 缓冲 (后处理需要全图随机访问)
 *   Gx: 119×185 int16 ≈ 44KB
 *   Gy: 117×185 int16 ≈ 43KB
 * ========================================================================== */
static int16_t gx_buf[V10_GX_ROWS][V10_GX_COLS];
static int16_t gy_buf[V10_GY_ROWS][V10_GY_COLS];

/* 图像行展开环形缓冲 (4 行, Gx 需 2 行, Gy 需 4 行) */
#define ROW_RING_DEPTH 4U
DTCM_BSS static int16_t row_ring[ROW_RING_DEPTH][STAIR_IMAGE_W];


/* ==========================================================================
 * 全局状态
 * ========================================================================== */
volatile runtime_profiler_t g_stair_vision_cost_profiler = {0};
volatile runtime_profiler_t g_stair_vision_frame_profiler = {0};
volatile stair_vision_output_t g_stair_vision_output = {0};
volatile uint8 g_stair_vision_output_write_busy = 0U;

static stair_vision_output_t g_shadow;
#if STAIR_VISION_PROFILE_ENABLE
static uint32 g_last_frame_time_us = 0U;
#endif


/* ==========================================================================
 * 对外 API
 * ========================================================================== */

void stair_vision_init(void)
{
    memset((void *)row_ring, 0, sizeof(row_ring));
    stair_vision_reset_filter();

#if STAIR_VISION_PROFILE_ENABLE
    timer_init(STAIR_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(STAIR_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_stair_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_stair_vision_frame_profiler);
    g_last_frame_time_us = timer_get(STAIR_VISION_PROFILE_TIMER);
#endif
}

void stair_vision_reset_filter(void)
{
    stair_vision_output_t empty;
    memset(&empty, 0, sizeof(empty));
    empty.result.crease_y  = -1;
    empty.result.peak_y    = -1;
    empty.result.peak2_y   = -1;
    g_shadow = empty;
    g_stair_vision_output_write_busy = 1U;
    g_stair_vision_output = empty;
    g_stair_vision_output_write_busy = 0U;
}

const volatile stair_vision_output_t *stair_vision_get_output(void)
{
    return &g_stair_vision_output;
}

void stair_vision_process_camera_frame(const uint8 *gray)
{
    v10_stair_result_t result;
    stair_vision_output_t next;
    int row, x;
    uint32_t ring_idx = 0;
    int gx_out = 0, gy_out = 0;

    if (gray == NULL) return;

#if STAIR_VISION_PROFILE_ENABLE
    {
        const uint32 now_us = timer_get(STAIR_VISION_PROFILE_TIMER);
        runtime_profiler_update(&g_stair_vision_frame_profiler,
                                (uint32)(now_us - g_last_frame_time_us));
        g_last_frame_time_us = now_us;
    }
    RUNTIME_PROFILE_BEGIN(g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);
#endif

    /* ---- 逐行卷积流水线 ---- */
    for (row = 0; row < STAIR_IMAGE_H; row++)
    {
        const uint8 *src = &gray[row * STAIR_IMAGE_W];
        int16_t *rb = row_ring[ring_idx];

        for (x = 0; x < STAIR_IMAGE_W; x++) {
            rb[x] = (int16_t)src[x];
        }

        /* Gx: 2行→1行 */
        if (row >= 1) {
            v10_conv_gx_row(
                row_ring[(ring_idx + ROW_RING_DEPTH - 1) & 3], rb,
                gx_buf[gx_out], V10_GX_COLS);
            gx_out++;
        }

        /* Gy: 4行→1行 */
        if (row >= 3) {
            v10_conv_gy_row(
                row_ring[(ring_idx + ROW_RING_DEPTH - 3) & 3],
                row_ring[(ring_idx + ROW_RING_DEPTH - 2) & 3],
                row_ring[(ring_idx + ROW_RING_DEPTH - 1) & 3],
                rb,
                gy_buf[gy_out], V10_GY_COLS);
            gy_out++;
        }

        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 后处理 ---- */
    v10_stair_process_full(&gx_buf[0][0], &gy_buf[0][0],
                           V10_GY_ROWS, V10_GY_COLS, &result);

    /* ---- 多帧滤波 ---- */
    next = g_shadow;
    next.frame_id++;
    next.raw_detected = result.has_stairs;

    if (result.has_stairs) {
        if (next.detected_streak < 255U) next.detected_streak++;
        next.lost_streak = 0;
        if (next.detected_streak >= STAIR_VISION_CONFIRM_FRAMES) {
            next.detected = 1;
            next.result = result;
        }
    } else {
        next.detected_streak = 0;
        if (next.lost_streak < 255U) next.lost_streak++;
        if (next.lost_streak >= STAIR_VISION_LOST_HOLD_FRAMES) {
            next.detected = 0;
            memset(&next.result, 0, sizeof(next.result));
            next.result.crease_y  = -1;
            next.result.peak_y    = -1;
            next.result.peak2_y   = -1;
        }
    }

#if STAIR_VISION_PROFILE_ENABLE
    RUNTIME_PROFILE_END(&g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);
#endif

    /* 发布 */
    g_stair_vision_output_write_busy = 1U;
    g_stair_vision_output = next;
    g_shadow = next;
    g_stair_vision_output_write_busy = 0U;
}
