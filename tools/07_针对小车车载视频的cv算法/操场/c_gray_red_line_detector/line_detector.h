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
#define LINE_TEMPORAL_MODE_LOST      0
#define LINE_TEMPORAL_MODE_DETECTED  1
#define LINE_TEMPORAL_MODE_PREDICTED 2

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
    uint8_t active;
    int lost_count;
    int max_lost;
    float smooth_alpha;
    float min_temporal_score;
    float bottom_x;
    float lookahead_x;
    float yaw_deg;
    float confidence;
} LineTemporalState;

typedef struct {
    uint8_t mode;       /* LINE_TEMPORAL_MODE_* */
    uint8_t accepted;   /* 1 when raw frame accepted by temporal gating */
    float temporal_score;
} LineTemporalDecision;

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

void line_temporal_state_init(LineTemporalState *state, int max_lost, float smooth_alpha, float min_temporal_score);

LineTemporalDecision line_temporal_update(
    LineTemporalState *state,
    int image_width,
    const LineDetectResult *raw,
    LineDetectResult *out);

#ifdef __cplusplus
}
#endif

#endif
