#include <math.h>

#include "../../code/navigation/nav_replay/plan1/plan1_speed_target_limit.h"

static int is_near(float actual, float expected)
{
    return fabsf(actual - expected) <= 0.001f;
}

int main(void)
{
    if (!is_near(Plan1_LimitAcceleratingTarget(-10.0f, -1.0f, 4.0f), -5.0f)) return 1;
    if (!is_near(Plan1_LimitAcceleratingTarget(-3.0f, -1.0f, 4.0f), -3.0f)) return 2;
    if (!is_near(Plan1_LimitAcceleratingTarget(-6.0f, -8.0f, 4.0f), -6.0f)) return 3;
    if (!is_near(Plan1_LimitAcceleratingTarget(10.0f, -8.0f, 4.0f), 10.0f)) return 4;
    if (!is_near(Plan1_LimitAcceleratingTarget(-10.0f, 0.0f, 4.0f), -4.0f)) return 5;
    if (!is_near(Plan1_ApplyActualSpeedLeadLimit(-10.0f, -10.0f, -5.9f, 4.0f), -9.9f)) return 6;
    if (!is_near(Plan1_ApplyActualSpeedLeadLimit(-6.0f, -9.2f, -8.0f, 4.0f), -9.2f)) return 7;
    return 0;
}
