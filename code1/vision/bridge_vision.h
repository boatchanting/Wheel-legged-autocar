#ifndef BRIDGE_VISION_H
#define BRIDGE_VISION_H

#include "zf_common_headfile.h"
#include "tools/runtime_profiler.h"
#include "bridge_detection.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRIDGE_VISION_IMAGE_W                 (94U)
#define BRIDGE_VISION_IMAGE_H                 (60U)
#define BRIDGE_VISION_CONFIRM_FRAMES          (1U)
#define BRIDGE_VISION_LOST_HOLD_FRAMES        (5U)

/* Vehicle deployment setting: use exactly one threshold per frame.  Set to
 * -1 to return to the detector's adaptive multi-threshold mode. */
#define BRIDGE_VISION_FIXED_THRESHOLD          (225)
#define BRIDGE_VISION_PROFILE_TIMER           (TC_TIME2_CH1)

/* The detector score is calibrated around 350.  IPC confidence remains a
 * normalized 0.0 .. 1.0 value so Core 0 keeps its existing wire format. */
#define BRIDGE_VISION_SCORE_FULL_SCALE         (500.0f)

typedef struct
{
    uint8 detected;                 /* valid bridge centre-line geometry */
    uint8 bridge_detected;          /* bridge candidate passed detector gates */
    uint8 state;                    /* BridgeDetectionState */
    uint8 geometry_valid;
    uint8 left_line_visible;
    uint8 right_line_visible;
    uint8 top_line_visible;
    uint8 entry_line_visible;
    uint8 bbox_xmin;
    uint8 bbox_ymin;
    uint8 bbox_xmax;
    uint8 bbox_ymax;
    uint16 area;
    float confidence;
    float bridge_confidence;
    float lateral_error_px;
    float yaw_error_deg;
    float center_x;
    uint8 center_x0;
    uint8 center_y0;
    uint8 center_x1;
    uint8 center_y1;
    float line_x_bottom;
    float line_x_lookahead;
    float candidate_score;
    float edge_contrast;
} bridge_vision_frame_result_t;

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
    bridge_vision_frame_result_t raw;
    bridge_vision_frame_result_t stable;
} bridge_vision_output_t;

extern volatile runtime_profiler_t g_bridge_vision_cost_profiler;
extern volatile runtime_profiler_t g_bridge_vision_frame_profiler;
extern volatile bridge_vision_output_t g_bridge_vision_output;
extern volatile uint8 g_bridge_vision_output_write_busy;

void bridge_vision_init(void);
void bridge_vision_reset_filter(void);
const volatile bridge_vision_output_t *bridge_vision_get_output(void);
void bridge_vision_process_camera_frame(const uint8 *gray);

#ifdef __cplusplus
}
#endif

#endif
