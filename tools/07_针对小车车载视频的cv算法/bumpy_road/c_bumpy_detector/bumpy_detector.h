#ifndef BUMPY_DETECTOR_H
#define BUMPY_DETECTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BUMPY_MAX_WIDTH             160
#define BUMPY_MAX_HEIGHT            120
#define BUMPY_MAX_PIXELS            (BUMPY_MAX_WIDTH * BUMPY_MAX_HEIGHT)
#define BUMPY_MAX_COMPONENTS        256
#define BUMPY_MAX_RUNS              BUMPY_MAX_HEIGHT
#define BUMPY_MAX_RIB_BANDS         32

#define BUMPY_ROI_X0                0
#define BUMPY_ROI_X1                93
#define BUMPY_ROI_Y0                6
#define BUMPY_ROI_Y1                59

#define BUMPY_MIN_COMPONENT_AREA    150
#define BUMPY_MIN_COMPONENT_WIDTH   10
#define BUMPY_MIN_COMPONENT_HEIGHT  4

#define BUMPY_MIN_ROW_RUN_WIDTH     12
#define BUMPY_MAX_ROW_GAP           2
#define BUMPY_BOTTOM_TARGET_ROWS    14

#define BUMPY_MIN_RIB_ROW_PIXELS    18
#define BUMPY_MIN_RIB_WIDTH         24
#define BUMPY_MIN_RIB_HEIGHT        2

#define BUMPY_IMAGE_CENTER_X        46.5f

typedef enum {
    BUMPY_PHASE_UNCERTAIN = 0,
    BUMPY_PHASE_APPROACH = 1,
    BUMPY_PHASE_INSIDE = 2,
    BUMPY_PHASE_EXIT = 3,
    BUMPY_PHASE_WHITE_SURFACE_ONLY = 4
} BumpyPhase;

typedef enum {
    BUMPY_MODE_FALLBACK_SEARCH = 0,
    BUMPY_MODE_SEEK_ENTRANCE = 1,
    BUMPY_MODE_FOLLOW_CENTERLINE = 2,
    BUMPY_MODE_HOLD_EXIT_LINE = 3,
    BUMPY_MODE_HOLD_WHITE_SURFACE = 4
} BumpyControllerMode;

typedef struct {
    int area;
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    float centroid_x;
    float centroid_y;
    float fill_ratio;
    uint8_t touches_border;
    float mean_gray;
    float score;
} BumpyComponent;

typedef struct {
    int y;
    int xmin;
    int xmax;
    int threshold;
} BumpyWhiteRun;

typedef struct {
    int ymin;
    int ymax;
    int xmin;
    int xmax;
    int area;
    int max_row_pixels;
    float mean_gray;
} BumpyRibBand;

typedef struct {
    float target_x;
    float steer_error_px;
    int row_count;
    int bottom_row_count;
    int top_y;
    int bottom_y;
    float mean_width;
} BumpyCenterlineSummary;

typedef struct {
    uint8_t detected;
    BumpyPhase phase;
    BumpyControllerMode mode;

    float white_threshold;
    float white_threshold_candidate;
    int white_threshold_int;
    float dark_threshold;

    int component_count;
    int candidate_count;
    int best_component_found;
    BumpyComponent best_component;

    int run_count;
    int rib_count;
    BumpyCenterlineSummary centerline;
} BumpyDetectResult;

typedef struct {
    uint8_t visited[BUMPY_MAX_PIXELS];
    int stack[BUMPY_MAX_PIXELS];
    BumpyComponent components[BUMPY_MAX_COMPONENTS];
    BumpyComponent candidates[BUMPY_MAX_COMPONENTS];
    uint8_t global_white_mask[BUMPY_MAX_PIXELS];
    uint8_t scan_white_mask[BUMPY_MAX_PIXELS];
    uint8_t white_mask[BUMPY_MAX_PIXELS];
    uint8_t rib_mask[BUMPY_MAX_PIXELS];
    BumpyWhiteRun runs[BUMPY_MAX_RUNS];
    BumpyRibBand rib_bands[BUMPY_MAX_RIB_BANDS];
} BumpyDetectScratch;

void bumpy_detect_result_clear(BumpyDetectResult *result);

int bumpy_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    float prev_white_threshold,
    int has_prev_white_threshold,
    BumpyDetectScratch *scratch,
    BumpyDetectResult *result);

const char *bumpy_phase_name(BumpyPhase phase);
const char *bumpy_mode_name(BumpyControllerMode mode);

#ifdef __cplusplus
}
#endif

#endif
