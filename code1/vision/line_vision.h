#ifndef LINE_VISION_H
#define LINE_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "pvc_vision.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_IMAGE_W                         (PVC_IMAGE_W)
#define LINE_IMAGE_H                         (PVC_IMAGE_H)
#define LINE_IMAGE_SIZE                      (LINE_IMAGE_W * LINE_IMAGE_H)

#define LINE_VISION_ENABLE                   (1)
#define LINE_VISION_PROFILE_ENABLE           (1)
#define LINE_VISION_PROFILE_TIMER            (PVC_VISION_PROFILE_TIMER)
#define LINE_VISION_DEBUG_PRINT_EVERY        (0U)

#define LINE_VISION_ROI_TOP_RATIO_X100       (25U)
#define LINE_VISION_MIN_ROWS                 (15U)
#define LINE_VISION_MIN_WIDTH                (8U)
#define LINE_VISION_MIN_Y_SPAN               (22U)
#define LINE_VISION_MAX_ABS_YAW_DEG          (35.0f)
#define LINE_VISION_MIN_CONFIDENCE           (0.72f)

#define LINE_VISION_BRIDGE_DARK_THRESHOLD    (180U)
#define LINE_VISION_BRIDGE_MIN_AREA          (35U)
#define LINE_VISION_BRIDGE_MIN_WIDTH         (18U)
#define LINE_VISION_BRIDGE_MIN_HEIGHT        (4U)
#define LINE_VISION_BRIDGE_MIN_FILL_RATIO    (0.22f)
#define LINE_VISION_BRIDGE_MIN_CONFIDENCE    (0.56f)
#define LINE_VISION_BRIDGE_SPEED_HINT        (-90.0f)

#define LINE_VISION_CONFIRM_FRAMES           (2U)
#define LINE_VISION_LOST_HOLD_FRAMES         (3U)
#define LINE_VISION_BRIDGE_CONFIRM_FRAMES    (1U)
#define LINE_VISION_BRIDGE_LOST_HOLD_FRAMES  (5U)

typedef struct
{
    uint8 detected;
    uint8 bridge_detected;
    uint8 bridge_component_count;
    uint8 points_used;
    uint8 y_span;
    uint8 bridge_bbox_xmin;
    uint8 bridge_bbox_ymin;
    uint8 bridge_bbox_xmax;
    uint8 bridge_bbox_ymax;
    float confidence;
    float bridge_confidence;
    float lateral_error_px;
    float yaw_error_deg;
    float line_x_bottom;
    float line_x_lookahead;
    float fit_rmse;
    float mean_track_width;
    float roi_white_ratio;
    float target_speed_hint;
} line_vision_frame_result_t;

typedef struct
{
    uint32 frame_id;
    uint8 raw_detected;
    uint8 stable_detected;
    uint8 bridge_raw_detected;
    uint8 bridge_stable_detected;
    uint8 detected_streak;
    uint8 lost_streak;
    uint8 bridge_detected_streak;
    uint8 bridge_lost_streak;
    line_vision_frame_result_t raw;
    line_vision_frame_result_t stable;
} line_vision_output_t;

extern volatile runtime_profiler_t g_line_vision_cost_profiler;
extern volatile runtime_profiler_t g_line_vision_frame_profiler;
extern volatile line_vision_output_t g_line_vision_output;
extern volatile uint8 g_line_vision_output_write_busy;

void line_vision_init(void);
void line_vision_reset_filter(void);
const volatile line_vision_output_t *line_vision_get_output(void);
void line_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
