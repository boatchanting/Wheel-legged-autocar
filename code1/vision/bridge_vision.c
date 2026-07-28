#include "bridge_vision.h"

#include <string.h>

volatile runtime_profiler_t g_bridge_vision_cost_profiler = {0};
volatile runtime_profiler_t g_bridge_vision_frame_profiler = {0};
volatile bridge_vision_output_t g_bridge_vision_output = {0};
volatile uint8 g_bridge_vision_output_write_busy = 0U;

/* 新检测器的跨帧状态与单帧结果。
   bridge_state_t ≈ 50 字节 (间距先验滑动窗 + 门控锁存),
   bridge_result_t ≈ 200 字节 (直线方程 + 标志位). */
static bridge_state_t  g_bridge_state;
static bridge_result_t g_bridge_result;
static bridge_vision_output_t g_bridge_output_shadow;
static uint32 g_bridge_last_frame_time_us = 0U;

/* ================================ 线段工具 ================================ */

static void bridge_vision_set_segment_invalid(int16 *x0, int16 *y0, int16 *x1, int16 *y1)
{
    *x0 = BRIDGE_VISION_COORD_INVALID;
    *y0 = BRIDGE_VISION_COORD_INVALID;
    *x1 = BRIDGE_VISION_COORD_INVALID;
    *y1 = BRIDGE_VISION_COORD_INVALID;
}

static void bridge_vision_clamp_segment(int16 *x0, int16 *y0, int16 *x1, int16 *y1)
{
    if (*x0 < 0) *x0 = 0;
    if (*x0 >= (int16)BRIDGE_VISION_IMAGE_W) *x0 = (int16)(BRIDGE_VISION_IMAGE_W - 1U);
    if (*y0 < 0) *y0 = 0;
    if (*y0 >= (int16)BRIDGE_VISION_IMAGE_H) *y0 = (int16)(BRIDGE_VISION_IMAGE_H - 1U);
    if (*x1 < 0) *x1 = 0;
    if (*x1 >= (int16)BRIDGE_VISION_IMAGE_W) *x1 = (int16)(BRIDGE_VISION_IMAGE_W - 1U);
    if (*y1 < 0) *y1 = 0;
    if (*y1 >= (int16)BRIDGE_VISION_IMAGE_H) *y1 = (int16)(BRIDGE_VISION_IMAGE_H - 1U);
}

static void bridge_vision_clear_frame(bridge_vision_frame_result_t *frame)
{
    memset(frame, 0, sizeof(*frame));

    bridge_vision_set_segment_invalid(&frame->left_line_x0, &frame->left_line_y0,
                                      &frame->left_line_x1, &frame->left_line_y1);
    bridge_vision_set_segment_invalid(&frame->right_line_x0, &frame->right_line_y0,
                                      &frame->right_line_x1, &frame->right_line_y1);
    bridge_vision_set_segment_invalid(&frame->down_line_x0, &frame->down_line_y0,
                                      &frame->down_line_x1, &frame->down_line_y1);
    bridge_vision_set_segment_invalid(&frame->up_line_x0, &frame->up_line_y0,
                                      &frame->up_line_x1, &frame->up_line_y1);
    bridge_vision_set_segment_invalid(&frame->center_line_x0, &frame->center_line_y0,
                                      &frame->center_line_x1, &frame->center_line_y1);
}

/* ---- 竖线 x = a*y + b → 线段 (y 取自支撑范围 u_lo..u_hi) ---- */
static void bridge_vision_line_v_to_segment(const bridge_line_t *line,
                                            int16 *x0, int16 *y0,
                                            int16 *x1, int16 *y1)
{
    if ((line == NULL) || (line->n < 4))
    {
        bridge_vision_set_segment_invalid(x0, y0, x1, y1);
        return;
    }

    *y0 = (int16)(line->u_lo);
    *y1 = (int16)(line->u_hi);
    *x0 = (int16)(line->a * (*y0) + line->b);
    *x1 = (int16)(line->a * (*y1) + line->b);
    bridge_vision_clamp_segment(x0, y0, x1, y1);
}

/* ---- 横线 y = a*x + b → 线段 (x 取自支撑范围 u_lo..u_hi) ---- */
static void bridge_vision_line_h_to_segment(const bridge_line_t *line,
                                            int16 *x0, int16 *y0,
                                            int16 *x1, int16 *y1)
{
    if ((line == NULL) || (line->n < 4))
    {
        bridge_vision_set_segment_invalid(x0, y0, x1, y1);
        return;
    }

    *x0 = (int16)(line->u_lo);
    *x1 = (int16)(line->u_hi);
    *y0 = (int16)(line->a * (*x0) + line->b);
    *y1 = (int16)(line->a * (*x1) + line->b);
    bridge_vision_clamp_segment(x0, y0, x1, y1);
}

/* ---- 红蓝底部估算横线段 (用作 down_line) ---- */
static void bridge_vision_entry_from_rb(const bridge_line_t *red,
                                        const bridge_line_t *blue,
                                        int16 *x0, int16 *y0,
                                        int16 *x1, int16 *y1)
{
    float y_bot, x_left, x_right;

    if ((red == NULL) || (blue == NULL) || (red->n < 4) || (blue->n < 4))
    {
        bridge_vision_set_segment_invalid(x0, y0, x1, y1);
        return;
    }

    y_bot = (red->u_hi > blue->u_hi) ? red->u_hi : blue->u_hi;
    if (y_bot < 0.0f) y_bot = 0.0f;
    if (y_bot > (float)(BRIDGE_VISION_IMAGE_H - 1U)) y_bot = (float)(BRIDGE_VISION_IMAGE_H - 1U);

    x_left  = red->a  * y_bot + red->b;
    x_right = blue->a * y_bot + blue->b;

    *y0 = (int16)y_bot;
    *y1 = (int16)y_bot;
    *x0 = (int16)x_left;
    *x1 = (int16)x_right;
    bridge_vision_clamp_segment(x0, y0, x1, y1);
}

/* ---- 中线: 有 green 直接用, 否则取红蓝中点 ---- */
static void bridge_vision_center_from_rgb(const bridge_line_t *red,
                                          const bridge_line_t *green,
                                          const bridge_line_t *blue,
                                          int16 *x0, int16 *y0,
                                          int16 *x1, int16 *y1)
{
    if ((green != NULL) && (green->n >= 4))
    {
        bridge_vision_line_v_to_segment(green, x0, y0, x1, y1);
        return;
    }

    if ((red != NULL) && (blue != NULL) && (red->n >= 4) && (blue->n >= 4))
    {
        float y_lo = (red->u_lo + blue->u_lo) * 0.5f;
        float y_hi = (red->u_hi + blue->u_hi) * 0.5f;
        float x_mid_lo = (red->a * y_lo + red->b + blue->a * y_lo + blue->b) * 0.5f;
        float x_mid_hi = (red->a * y_hi + red->b + blue->a * y_hi + blue->b) * 0.5f;
        *y0 = (int16)y_lo;
        *y1 = (int16)y_hi;
        *x0 = (int16)x_mid_lo;
        *x1 = (int16)x_mid_hi;
        bridge_vision_clamp_segment(x0, y0, x1, y1);
        return;
    }

    bridge_vision_set_segment_invalid(x0, y0, x1, y1);
}

/* ================================ 结果导出 ================================ */

static void bridge_vision_export_result(const bridge_result_t *res,
                                        bridge_vision_frame_result_t *frame)
{
    int geometry_valid;

    bridge_vision_clear_frame(frame);

    /* 桥候选: 任何非 NONE 模式都算检测到桥 */
    frame->bridge_detected = (res->mode != BRIDGE_MODE_NONE) ? 1U : 0U;

    /* state 映射: bridge_mode_t → 兼容 VISION_BRIDGE_STATE_* (0=NONE, 2=ON_BRIDGE) */
    if (res->mode == BRIDGE_MODE_RB || res->mode == BRIDGE_MODE_RMB ||
        res->mode == BRIDGE_MODE_RB_Q)
    {
        frame->state = 2U; /* 红蓝双线可见 → ON_BRIDGE */
    }
    else if (res->mode == BRIDGE_MODE_NONE)
    {
        frame->state = 0U;
    }
    else
    {
        frame->state = (uint8)res->mode;
    }

    /* 左线 ← red */
    if (res->has_red)
    {
        bridge_vision_line_v_to_segment(&res->red,
                                        &frame->left_line_x0, &frame->left_line_y0,
                                        &frame->left_line_x1, &frame->left_line_y1);
    }
    /* 右线 ← blue */
    if (res->has_blue)
    {
        bridge_vision_line_v_to_segment(&res->blue,
                                        &frame->right_line_x0, &frame->right_line_y0,
                                        &frame->right_line_x1, &frame->right_line_y1);
    }
    /* 中线 ← green 或红蓝中点 */
    bridge_vision_center_from_rgb(res->has_red  ? &res->red  : NULL,
                                  res->has_green ? &res->green : NULL,
                                  res->has_blue  ? &res->blue  : NULL,
                                  &frame->center_line_x0, &frame->center_line_y0,
                                  &frame->center_line_x1, &frame->center_line_y1);
    /* 上线 ← top (横线) */
    if (res->has_top)
    {
        bridge_vision_line_h_to_segment(&res->top,
                                        &frame->up_line_x0, &frame->up_line_y0,
                                        &frame->up_line_x1, &frame->up_line_y1);
    }
    /* 下线 ← 红蓝底部估算 */
    if (res->has_red && res->has_blue)
    {
        bridge_vision_entry_from_rb(&res->red, &res->blue,
                                    &frame->down_line_x0, &frame->down_line_y0,
                                    &frame->down_line_x1, &frame->down_line_y1);
    }

    /* 几何有效 = 至少中线存在 (green 或红蓝双线) */
    geometry_valid = (res->has_green || (res->has_red && res->has_blue)) ? 1 : 0;
    frame->geometry_valid = (uint8)geometry_valid;
    if (geometry_valid)
    {
        frame->detected = 1U;
    }
}

/* ================================ 滤波与发布 (保持不变) ================================ */

static void bridge_vision_publish(const bridge_vision_output_t *next)
{
    g_bridge_output_shadow = *next;
    g_bridge_vision_output_write_busy = 1U;
    g_bridge_vision_output = *next;
    g_bridge_vision_output_write_busy = 0U;
}

static void bridge_vision_update_filter(const bridge_vision_frame_result_t *raw)
{
    bridge_vision_output_t next = g_bridge_output_shadow;

    next.frame_id++;
    next.raw = *raw;
    next.raw_detected = raw->detected;
    next.bridge_raw_detected = raw->bridge_detected;

    if (raw->bridge_detected)
    {
        if (next.bridge_detected_streak < 255U) next.bridge_detected_streak++;
        next.bridge_lost_streak = 0U;
        if (next.bridge_detected_streak >= BRIDGE_VISION_CONFIRM_FRAMES)
        {
            next.bridge_stable_detected = 1U;
            next.stable = *raw;
        }
    }
    else
    {
        next.bridge_detected_streak = 0U;
        if (next.bridge_lost_streak < 255U) next.bridge_lost_streak++;
        if (next.bridge_lost_streak >= BRIDGE_VISION_LOST_HOLD_FRAMES)
        {
            next.bridge_stable_detected = 0U;
        }
    }

    if (raw->detected)
    {
        if (next.detected_streak < 255U) next.detected_streak++;
        next.lost_streak = 0U;
        next.stable_detected = 1U;
        next.stable = *raw;
    }
    else
    {
        next.detected_streak = 0U;
        if (next.lost_streak < 255U) next.lost_streak++;
        next.stable_detected = 0U;
    }

    if ((next.bridge_stable_detected == 0U) && (next.stable_detected == 0U))
    {
        bridge_vision_clear_frame(&next.stable);
    }
    bridge_vision_publish(&next);
}

/* ================================ 公开接口 ================================ */

void bridge_vision_init(void)
{
    bridge_detect_init(&g_bridge_state);
    bridge_vision_reset_filter();
    timer_init(BRIDGE_VISION_PROFILE_TIMER, TIMER_US);
    timer_start(BRIDGE_VISION_PROFILE_TIMER);
    RUNTIME_PROFILE_RESET(&g_bridge_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_bridge_vision_frame_profiler);
    g_bridge_last_frame_time_us = timer_get(BRIDGE_VISION_PROFILE_TIMER);
}

void bridge_vision_reset_filter(void)
{
    bridge_vision_output_t empty;

    bridge_detect_init(&g_bridge_state);
    memset(&empty, 0, sizeof(empty));
    bridge_vision_clear_frame(&empty.raw);
    bridge_vision_clear_frame(&empty.stable);
    bridge_vision_publish(&empty);
}

const volatile bridge_vision_output_t *bridge_vision_get_output(void)
{
    return &g_bridge_vision_output;
}

void bridge_vision_process_camera_frame(const uint8 *gray)
{
    bridge_vision_frame_result_t raw;
    const uint32 now_us = timer_get(BRIDGE_VISION_PROFILE_TIMER);

    if (gray == NULL)
    {
        return;
    }

    runtime_profiler_update(&g_bridge_vision_frame_profiler,
                            (uint32)(now_us - g_bridge_last_frame_time_us));
    g_bridge_last_frame_time_us = now_us;
    RUNTIME_PROFILE_BEGIN(g_bridge_vision_cost_profiler, BRIDGE_VISION_PROFILE_TIMER);

    bridge_detect_frame(gray, &g_bridge_state, &g_bridge_result);
    bridge_vision_export_result(&g_bridge_result, &raw);
    bridge_vision_update_filter(&raw);

    RUNTIME_PROFILE_END(&g_bridge_vision_cost_profiler, BRIDGE_VISION_PROFILE_TIMER);
}
