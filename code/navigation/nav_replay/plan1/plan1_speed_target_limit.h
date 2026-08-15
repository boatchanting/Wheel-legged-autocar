#ifndef PLAN1_SPEED_TARGET_LIMIT_H
#define PLAN1_SPEED_TARGET_LIMIT_H

#include <math.h>

/* Only constrain same-direction acceleration; all three inputs share one unit. */
static inline float Plan1_LimitAcceleratingTarget(float path_target,
                                                  float actual_speed,
                                                  float max_lead)
{
    float path_abs = fabsf(path_target);
    float actual_abs = fabsf(actual_speed);
    float capped_abs;

    if ((path_target * actual_speed < 0.0f) || (path_abs <= actual_abs))
    {
        return path_target;
    }

    capped_abs = actual_abs + max_lead;
    if (capped_abs >= path_abs)
    {
        return path_target;
    }

    return (path_target < 0.0f) ? -capped_abs : capped_abs;
}

/* Keep deceleration and direction reversal outside the actual-speed lead cap. */
static inline float Plan1_ApplyActualSpeedLeadLimit(float path_target,
                                                     float target_candidate,
                                                     float actual_speed,
                                                     float max_lead)
{
    if ((path_target * actual_speed < 0.0f) ||
        (fabsf(path_target) <= fabsf(actual_speed)))
    {
        return target_candidate;
    }

    return Plan1_LimitAcceleratingTarget(target_candidate, actual_speed, max_lead);
}

#endif /* PLAN1_SPEED_TARGET_LIMIT_H */
