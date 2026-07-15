#include "bridge_vision.h"

#include <string.h>

volatile runtime_profiler_t g_bridge_vision_cost_profiler = {0};
volatile runtime_profiler_t g_bridge_vision_frame_profiler = {0};
volatile bridge_vision_output_t g_bridge_vision_output = {0};
volatile uint8 g_bridge_vision_output_write_busy = 0U;

/* This workspace is intentionally static: BridgeDetectionScratch is about
 * 22 KiB and must never be placed on the Core 1 task/interrupt stack. */
static BridgeDetectionScratch g_bridge_detection_scratch;
static BridgeDetectionConfig g_bridge_detection_config;
static bridge_vision_output_t g_bridge_output_shadow;
static uint32 g_bridge_last_frame_time_us = 0U;

static void bridge_vision_set_segment_invalid(int16 *x0, int16 *y0, int16 *x1, int16 *y1)
{
    *x0 = BRIDGE_VISION_COORD_INVALID;
    *y0 = BRIDGE_VISION_COORD_INVALID;
    *x1 = BRIDGE_VISION_COORD_INVALID;
    *y1 = BRIDGE_VISION_COORD_INVALID;
}

static void bridge_vision_set_segment(int16 *dst_x0,
                                      int16 *dst_y0,
                                      int16 *dst_x1,
                                      int16 *dst_y1,
                                      const BridgeDetectionSegment *segment)
{
    if ((segment == NULL) || (segment->valid == 0U))
    {
        bridge_vision_set_segment_invalid(dst_x0, dst_y0, dst_x1, dst_y1);
        return;
    }

    *dst_x0 = (int16)segment->x0;
    *dst_y0 = (int16)segment->y0;
    *dst_x1 = (int16)segment->x1;
    *dst_y1 = (int16)segment->y1;
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

static void bridge_vision_export_result(const BridgeDetectionResult *detected,
                                        bridge_vision_frame_result_t *frame)
{
    int geometry_valid;

    bridge_vision_clear_frame(frame);
    if (detected->candidate_found == 0U)
    {
        return;
    }

    frame->bridge_detected = detected->bridge_found;
    frame->state = (uint8)detected->state;

    bridge_vision_set_segment(&frame->left_line_x0,
                              &frame->left_line_y0,
                              &frame->left_line_x1,
                              &frame->left_line_y1,
                              &detected->left_segment);
    bridge_vision_set_segment(&frame->right_line_x0,
                              &frame->right_line_y0,
                              &frame->right_line_x1,
                              &frame->right_line_y1,
                              &detected->right_segment);
    bridge_vision_set_segment(&frame->down_line_x0,
                              &frame->down_line_y0,
                              &frame->down_line_x1,
                              &frame->down_line_y1,
                              &detected->entry_segment);
    bridge_vision_set_segment(&frame->up_line_x0,
                              &frame->up_line_y0,
                              &frame->up_line_x1,
                              &frame->up_line_y1,
                              &detected->top_segment);
    bridge_vision_set_segment(&frame->center_line_x0,
                              &frame->center_line_y0,
                              &frame->center_line_x1,
                              &frame->center_line_y1,
                              &detected->center_segment);

    geometry_valid = (detected->bridge_found != 0U) &&
                     (detected->center_segment.valid != 0U);
    frame->geometry_valid = (uint8)geometry_valid;
    if (geometry_valid)
    {
        frame->detected = 1U;
    }
}

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

void bridge_vision_init(void)
{
    bridge_detection_default_config(&g_bridge_detection_config);
    g_bridge_detection_config.fixed_threshold = BRIDGE_VISION_FIXED_THRESHOLD;
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

    memset(&g_bridge_detection_scratch, 0, sizeof(g_bridge_detection_scratch));
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
    BridgeDetectionResult detected;
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

    bridge_detection_result_clear(&detected);
    (void)bridge_detection_detect_gray(gray,
                                       BRIDGE_VISION_IMAGE_W,
                                       BRIDGE_VISION_IMAGE_H,
                                       BRIDGE_VISION_IMAGE_W,
                                       &g_bridge_detection_config,
                                       &g_bridge_detection_scratch,
                                       &detected);
    bridge_vision_export_result(&detected, &raw);
    bridge_vision_update_filter(&raw);
    RUNTIME_PROFILE_END(&g_bridge_vision_cost_profiler, BRIDGE_VISION_PROFILE_TIMER);
}
