#ifndef PVC_DETECTOR_H
#define PVC_DETECTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PVC_MAX_WIDTH              160
#define PVC_MAX_HEIGHT             120
#define PVC_MAX_PIXELS             (PVC_MAX_WIDTH * PVC_MAX_HEIGHT)
#define PVC_MAX_COMPONENTS         256

#define PVC_WHITE_THRESHOLD        245
#define PVC_MIN_AREA               120
#define PVC_MIN_WIDTH              12
#define PVC_MIN_HEIGHT             4
#define PVC_MIN_FILL_RATIO         0.25f
#define PVC_MIN_DECISION_SCORE     0.58f

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
} PvcComponent;

typedef struct {
    uint8_t detected;
    float confidence;
    int component_count;
    int candidate_count;

    int area;
    int bbox_xmin;
    int bbox_ymin;
    int bbox_xmax;
    int bbox_ymax;
    float centroid_x;
    float centroid_y;
    float fill_ratio;
    uint8_t touches_border;
    float mean_gray;
    int entry_bottom_y;
    int entry_top_y;

    /* Control-facing fields. Replace scale tables with calibrated values on car. */
    float forward_mm;
    float lateral_mm;
    float yaw_error_deg;
} PvcDetectResult;

typedef struct {
    uint8_t visited[PVC_MAX_PIXELS];
    int stack[PVC_MAX_PIXELS];
    PvcComponent components[PVC_MAX_COMPONENTS];
    PvcComponent candidates[PVC_MAX_COMPONENTS];
} PvcDetectScratch;

void pvc_detect_result_clear(PvcDetectResult *result);

int pvc_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    PvcDetectScratch *scratch,
    PvcDetectResult *result);

float pvc_estimate_forward_mm_from_row(int row, int image_height);
float pvc_estimate_lateral_mm_from_x(float x, int image_width);

#ifdef __cplusplus
}
#endif

#endif
