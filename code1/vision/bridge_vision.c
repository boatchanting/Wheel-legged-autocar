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

static float bridge_vision_clamp_unit(float value)
{
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

static uint8 bridge_vision_clamp_u8(int value)
{
    if (value < 0) return 0U;
    if (value > 255) return 255U;
    return (uint8)value;
}

static uint16 bridge_vision_clamp_u16(int value)
{
    if (value < 0) return 0U;
    if (value > 65535) return 65535U;
    return (uint16)value;
}

static void bridge_vision_clear_frame(bridge_vision_frame_result_t *frame)
{
    memset(frame, 0, sizeof(*frame));
    frame->bbox_xmin = 0xFFU;
    frame->bbox_ymin = 0xFFU;
    frame->bbox_xmax = 0xFFU;
    frame->bbox_ymax = 0xFFU;
}

static void bridge_vision_export_result(const BridgeDetectionResult *detected,
                                        bridge_vision_frame_result_t *frame)
{
    int xmin = BRIDGE_VISION_IMAGE_W - 1;
    int xmax = 0;
    int ymin = detected->top_row;
    int ymax = detected->bottom_row;
    int geometry_valid;

    bridge_vision_clear_frame(frame);
    if (detected->candidate_found == 0U)
    {
        return;
    }

    frame->bridge_detected = detected->bridge_found;
    frame->state = (uint8)detected->state;
    frame->area = bridge_vision_clamp_u16(detected->area);
    frame->candidate_score = detected->candidate_score;
    frame->edge_contrast = detected->edge_contrast;
    frame->center_x = detected->center_x;
    frame->left_line_visible = detected->left_line_visible;
    frame->right_line_visible = detected->right_line_visible;
    frame->top_line_visible = detected->top_line_visible;
    frame->entry_line_visible = detected->entry_line_visible;
    frame->bridge_confidence = bridge_vision_clamp_unit(
        detected->candidate_score / BRIDGE_VISION_SCORE_FULL_SCALE);

    if (detected->left_segment.valid)
    {
        if (detected->left_segment.x0 < xmin) xmin = detected->left_segment.x0;
        if (detected->left_segment.x1 < xmin) xmin = detected->left_segment.x1;
        if (detected->left_segment.x0 > xmax) xmax = detected->left_segment.x0;
        if (detected->left_segment.x1 > xmax) xmax = detected->left_segment.x1;
    }
    if (detected->right_segment.valid)
    {
        if (detected->right_segment.x0 < xmin) xmin = detected->right_segment.x0;
        if (detected->right_segment.x1 < xmin) xmin = detected->right_segment.x1;
        if (detected->right_segment.x0 > xmax) xmax = detected->right_segment.x0;
        if (detected->right_segment.x1 > xmax) xmax = detected->right_segment.x1;
    }
    if ((xmin <= xmax) && (ymin >= 0) && (ymax >= ymin))
    {
        frame->bbox_xmin = bridge_vision_clamp_u8(xmin);
        frame->bbox_xmax = bridge_vision_clamp_u8(xmax);
        frame->bbox_ymin = bridge_vision_clamp_u8(ymin);
        frame->bbox_ymax = bridge_vision_clamp_u8(ymax);
    }

    geometry_valid = (detected->bridge_found != 0U) &&
                     (detected->center_segment.valid != 0U);
    frame->geometry_valid = (uint8)geometry_valid;
    if (geometry_valid)
    {
        frame->detected = 1U;
        frame->confidence = frame->bridge_confidence;
        frame->lateral_error_px = detected->lateral_error_px;
        /* For the small slopes of a 94x60 image, atan(s) is accurately
         * approximated by s.  This avoids a costly libm call. */
        frame->yaw_error_deg = detected->heading_dx_per_dy * 57.29578f;
        frame->center_x0 = bridge_vision_clamp_u8(detected->center_segment.x0);
        frame->center_y0 = bridge_vision_clamp_u8(detected->center_segment.y0);
        frame->center_x1 = bridge_vision_clamp_u8(detected->center_segment.x1);
        frame->center_y1 = bridge_vision_clamp_u8(detected->center_segment.y1);
        frame->line_x_bottom = (float)detected->center_segment.x1;
        frame->line_x_lookahead = (float)detected->center_segment.x0;
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
    bridge_vision_reset_filter();
    RUNTIME_PROFILE_RESET(&g_bridge_vision_cost_profiler);
    RUNTIME_PROFILE_RESET(&g_bridge_vision_frame_profiler);
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

    if (gray == NULL)
    {
        return;
    }

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
}
