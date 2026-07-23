#include "stair_vision.h"
#include "tcm.h"
#include <string.h>

/* ---- DTCM 环形缓冲 (与 V9 原始测试代码一致) ---- */
DTCM_BSS int16_t edge_ring[4][94];

/* ---- SRAM 大数组 ---- */
static int16_t gx_buf[59][91];
static int16_t gy_buf[57][91];

/* ---- 全局输出 ---- */
volatile runtime_profiler_t g_stair_vision_cost_profiler = {0};
volatile runtime_profiler_t g_stair_vision_frame_profiler = {0};
volatile stair_vision_output_t g_stair_vision_output = {0};
volatile uint8 g_stair_vision_output_write_busy = 0U;

/* ---- 内部影子变量 ---- */
static stair_vision_output_t g_stair_shadow;
static uint32 g_stair_last_frame_time_us = 0U;

static void stair_vision_clear_frame(stair_vision_frame_result_t *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->crease_y = -1;
}

static void stair_vision_publish(const stair_vision_output_t *next)
{
    g_stair_shadow = *next;
    g_stair_vision_output_write_busy = 1U;
    g_stair_vision_output = *next;
    g_stair_vision_output_write_busy = 0U;
}

static void stair_vision_update_filter(const stair_vision_frame_result_t *raw)
{
    stair_vision_output_t next = g_stair_shadow;

    next.frame_id++;
    next.raw = *raw;
    next.raw_detected = raw->detected;

    if (raw->detected)
    {
        if (next.detected_streak < 255U) next.detected_streak++;
        next.lost_streak = 0U;
        if (next.detected_streak >= STAIR_VISION_CONFIRM_FRAMES)
        {
            next.stable_detected = 1U;
            next.stable = *raw;
        }
    }
    else
    {
        next.detected_streak = 0U;
        if (next.lost_streak < 255U) next.lost_streak++;
        if (next.lost_streak >= STAIR_VISION_LOST_HOLD_FRAMES)
        {
            next.stable_detected = 0U;
        }
    }

    if (next.stable_detected == 0U)
    {
        stair_vision_clear_frame(&next.stable);
    }
    stair_vision_publish(&next);
}

void stair_vision_init(void)
{
    stair_vision_reset_filter();
    timer_init(STAIR_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(STAIR_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_stair_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_stair_vision_frame_profiler);
    g_stair_last_frame_time_us = timer_get(STAIR_VISION_PROFILE_TIMER);
}

void stair_vision_reset_filter(void)
{
    stair_vision_output_t empty;

    memset(&empty, 0, sizeof(empty));
    stair_vision_clear_frame(&empty.raw);
    stair_vision_clear_frame(&empty.stable);
    stair_vision_publish(&empty);
}

const volatile stair_vision_output_t *stair_vision_get_output(void)
{
    return &g_stair_vision_output;
}

void stair_vision_process_camera_frame(const uint8 *gray)
{
    v9_stair_result_t result;
    stair_vision_frame_result_t raw;
    const uint32 now_us = timer_get(STAIR_VISION_PROFILE_TIMER);
    int ring_idx = 0, gx_out = 0, gy_out = 0;
    int row, x;

    if (gray == NULL)
    {
        return;
    }

    runtime_profiler_update(&g_stair_vision_frame_profiler,
                            (uint32)(now_us - g_stair_last_frame_time_us));
    g_stair_last_frame_time_us = now_us;
    RUNTIME_PROFILE_BEGIN(g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);

    /* ---- 卷积：逐行流入, 调用汇编算子 ---- */
    for (row = 0; row < 60; row++)
    {
        int16_t *rb = edge_ring[ring_idx];
        for (x = 0; x < 94; x++)
        {
            rb[x] = (int16_t)gray[row * 94 + x];
        }

        if (row >= 1)
        {
            v9_conv_gx_row(edge_ring[(ring_idx - 1) & 3], rb, gx_buf[gx_out], 91);
            gx_out++;
        }
        if (row >= 3)
        {
            v9_conv_gy_row(edge_ring[(ring_idx - 3) & 3],
                           edge_ring[(ring_idx - 2) & 3],
                           edge_ring[(ring_idx - 1) & 3],
                           edge_ring[ring_idx],
                           gy_buf[gy_out], 91);
            gy_out++;
        }
        ring_idx = (ring_idx + 1) & 3;
    }

    /* ---- 后处理 ---- */
    memset(&result, 0, sizeof(result));
    result.crease_y = -1;
    v9_stair_process_full(&gx_buf[0][0], &gy_buf[0][0], 57, 91, &result);

    RUNTIME_PROFILE_END(&g_stair_vision_cost_profiler, STAIR_VISION_PROFILE_TIMER);

    /* ---- 填充 raw 结果 + 连续帧滤波 ---- */
    memset(&raw, 0, sizeof(raw));
    raw.crease_y     = -1;
    raw.detected     = result.has_stairs;
    raw.joint_score  = result.joint_score;
    raw.left_rho     = result.left_rho;
    raw.left_theta   = result.left_theta;
    raw.right_rho    = result.right_rho;
    raw.right_theta  = result.right_theta;
    raw.center_a     = result.center_a;
    raw.center_b     = result.center_b;
    raw.center_c     = result.center_c;
    raw.crease_y     = result.crease_y;
    raw.crease_span  = result.crease_span;

    stair_vision_update_filter(&raw);
}
