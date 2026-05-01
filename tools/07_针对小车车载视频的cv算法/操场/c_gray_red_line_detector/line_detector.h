#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_MAX_WIDTH       320
#define LINE_MAX_HEIGHT      240
#define LINE_MAX_PIXELS      (LINE_MAX_WIDTH * LINE_MAX_HEIGHT)
#define LINE_MAX_COMPONENTS  512
#define LINE_MIN_DECISION_SCORE 0.35f

typedef struct {
    int area;
    int xmin;
    int ymin;
    int xmax;
    int ymax;
    float centroid_x;
    float centroid_y;
    float score;
} LineComponent;

typedef struct {
    uint8_t detected;
    float confidence;
    int component_count;
    int candidate_count;

    int best_label;
    int bbox_xmin;
    int bbox_ymin;
    int bbox_xmax;
    int bbox_ymax;
    float centroid_x;
    float centroid_y;

    float line_x_bottom;
    float line_x_lookahead;
    float line_yaw_deg;
    float lateral_error_px;
    int line_point_rows;
} LineDetectResult;

typedef struct {
    uint8_t gray_blur[LINE_MAX_PIXELS];
    uint8_t mask[LINE_MAX_PIXELS];
    uint8_t visited[LINE_MAX_PIXELS];
    uint16_t labels[LINE_MAX_PIXELS];
    int integral[(LINE_MAX_HEIGHT + 1) * (LINE_MAX_WIDTH + 1)];
    int stack[LINE_MAX_PIXELS];
    LineComponent components[LINE_MAX_COMPONENTS];
    LineComponent candidates[LINE_MAX_COMPONENTS];
} LineDetectScratch;

void line_detect_result_clear(LineDetectResult *result);

int line_detect_frame_gray(
    const uint8_t *gray,
    int width,
    int height,
    LineDetectScratch *scratch,
    LineDetectResult *result);

#ifdef __cplusplus
}
#endif

#endif
