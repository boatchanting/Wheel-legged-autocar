#ifndef BRIDGE_DETECTION_H
#define BRIDGE_DETECTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BRIDGE_DETECTION_MAX_WIDTH   96
#define BRIDGE_DETECTION_MAX_HEIGHT  60
#define BRIDGE_DETECTION_MAX_PIXELS  (BRIDGE_DETECTION_MAX_WIDTH * BRIDGE_DETECTION_MAX_HEIGHT)
#define BRIDGE_DETECTION_WORDS_PER_ROW ((BRIDGE_DETECTION_MAX_WIDTH + 31) / 32)

typedef enum {
    BRIDGE_DETECTION_STATE_NONE = 0,
    BRIDGE_DETECTION_STATE_PREPARE_ENTER = 1,
    BRIDGE_DETECTION_STATE_ON_BRIDGE = 2,
    BRIDGE_DETECTION_STATE_PREPARE_EXIT = 3
} BridgeDetectionState;

typedef struct {
    float min_valid_score;
    float min_edge_contrast;
    int fixed_threshold;
} BridgeDetectionConfig;

typedef struct {
    uint8_t valid;
    float slope;
    float intercept;
    float support_min_y;
    float support_max_y;
    int inlier_count;
    float span;
    float residual;
    float border_touch_ratio;
    float mean_x;
} BridgeDetectionSideLine;

typedef struct {
    uint8_t valid;
    int x0;
    int y0;
    int x1;
    int y1;
} BridgeDetectionSegment;

typedef struct {
    uint8_t candidate_found;
    uint8_t bridge_found;
    BridgeDetectionState state;
    int threshold;
    float candidate_score;
    int area;
    float area_ratio;
    int top_row;
    int start_row;
    int bottom_row;
    int max_width;
    int bottom_width;
    float center_x;
    float edge_contrast;
    float left_clip_ratio;
    float right_clip_ratio;
    float dual_clip_ratio;
    float border_monotonic;
    uint8_t left_line_visible;
    uint8_t right_line_visible;
    uint8_t top_line_visible;
    uint8_t entry_line_visible;
    BridgeDetectionSideLine left_line;
    BridgeDetectionSideLine right_line;
    BridgeDetectionSegment left_segment;
    BridgeDetectionSegment right_segment;
    BridgeDetectionSegment center_segment;
    float control_center_x;
    float lateral_error_px;
    float heading_dx_per_dy;
} BridgeDetectionResult;

typedef struct {
    uint32_t row[BRIDGE_DETECTION_MAX_HEIGHT][BRIDGE_DETECTION_WORDS_PER_ROW];
} BridgeDetectionBitmap;

typedef struct {
    BridgeDetectionBitmap work0;
    BridgeDetectionBitmap work1;
    BridgeDetectionBitmap work2;
    BridgeDetectionBitmap work4;
    BridgeDetectionBitmap best_visible;
    BridgeDetectionBitmap best_outer;
    uint16_t queue[BRIDGE_DETECTION_MAX_PIXELS];
    int16_t column_top[BRIDGE_DETECTION_MAX_WIDTH];
    int16_t column_bottom[BRIDGE_DETECTION_MAX_WIDTH];
    /* Geometry of the selected candidate.  The final reporting stage used to
     * rescan best_visible/best_outer five times.  Keep compact row/column
     * extrema instead: -1 is the invalid sentinel and all valid coordinates
     * fit in int8_t for the <=96x60 camera image. */
    int8_t best_visible_left[BRIDGE_DETECTION_MAX_HEIGHT];
    int8_t best_visible_right[BRIDGE_DETECTION_MAX_HEIGHT];
    int8_t best_outer_left[BRIDGE_DETECTION_MAX_HEIGHT];
    int8_t best_outer_right[BRIDGE_DETECTION_MAX_HEIGHT];
    int8_t best_outer_top[BRIDGE_DETECTION_MAX_WIDTH];
    int8_t best_outer_bottom[BRIDGE_DETECTION_MAX_WIDTH];
    uint8_t previous_gray[BRIDGE_DETECTION_MAX_PIXELS];
    BridgeDetectionResult cached_result;
    uint32_t cache_magic;
    uint16_t cache_width;
    uint16_t cache_height;
    float cache_min_valid_score;
    float cache_min_edge_contrast;
    int cache_fixed_threshold;
    int cache_status;
    uint32_t exact_cache_hits;
    uint32_t temporal_fast_hits;
    uint32_t full_detection_calls;
    uint8_t temporal_streak;
} BridgeDetectionScratch;

void bridge_detection_default_config(BridgeDetectionConfig *config);
void bridge_detection_result_clear(BridgeDetectionResult *result);
const char *bridge_detection_state_name(BridgeDetectionState state);
int bridge_detection_detect_gray(const uint8_t *gray, int width, int height, int stride,
                                 const BridgeDetectionConfig *config,
                                 BridgeDetectionScratch *scratch,
                                 BridgeDetectionResult *result);

#ifdef __cplusplus
}
#endif

#endif
