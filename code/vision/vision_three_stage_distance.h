#ifndef VISION_THREE_STAGE_DISTANCE_H
#define VISION_THREE_STAGE_DISTANCE_H

#include <stdint.h>

static inline uint8_t VisionThreeStageJump1DistanceReached(float start_x_mm,
                                                            float start_y_mm,
                                                            float current_x_mm,
                                                            float current_y_mm,
                                                            float required_distance_mm)
{
    float dx = current_x_mm - start_x_mm;
    float dy = current_y_mm - start_y_mm;

    return (uint8_t)((dx * dx + dy * dy) >=
                     (required_distance_mm * required_distance_mm));
}

#endif
