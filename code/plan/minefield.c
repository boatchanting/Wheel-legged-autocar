#include "minefield.h"

extern uint8 g_special_action_trigger;

// 默认旋转总角度（deg）；当前统一要求至少 730 度。
#define SPIN_TARGET_ANGLE_DEFAULT 730.0f
// 旋转总角度下限（deg）；外部即使给得更小，也会被钳到这个值。
#define SPIN_TARGET_ANGLE_MIN     730.0f
// 旋转阶段的最大角速度指令（deg/s）。
#define SPIN_MAX_SPEED            360.0f
// 角速度爬升斜率；避免转圈动作起转过猛。
#define SPIN_ACCEL_STEP           0.6f
// 减速区角度（deg）；进入最后这段角度后开始线性收速。
#define SPIN_DECEL_ANGLE          180.0f
// 旋转末段的最小角速度指令（deg/s）；避免末段因速度过低卡住。
#define SPIN_MIN_SPEED            45.0f
// 达到总旋转角度后，退出朝向仍需满足的航向误差容差（deg）。
#define SPIN_EXIT_YAW_TOLERANCE   6.0f
// 旋转输出符号；用于统一适配底层角速度方向定义。
#define SPIN_OUTPUT_SIGN          1.0f

volatile uint8_t minefield_flag = 0;
static uint8_t  s_is_spinning = 0;
static float    s_accumulated_angle = 0.0f;
static float    s_current_speed_cmd = 0.0f;
static float    s_planned_total_angle = SPIN_TARGET_ANGLE_DEFAULT;
static float    s_planned_exit_yaw_deg = 0.0f;
static float    s_spin_speed_sign = 1.0f;
uint8_t vision_detected_marker = 0;

// 浮点爬坡函数：将当前速度逐步逼近目标速度，而不是一步跳变。
static float Minefield_RampFloat(float current, float target, float step)
{
    if (current < target)
    {
        current += step;
        if (current > target) current = target;
    }
    else if (current > target)
    {
        current -= step;
        if (current < target) current = target;
    }
    return current;
}

// 将角度归一化到 [-180, 180]，便于统一做航向误差判断。
static float Minefield_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

void Minefield_Init(void)
{
    minefield_flag = 0;
    s_is_spinning = 0;
    s_accumulated_angle = 0.0f;
    s_current_speed_cmd = 0.0f;
    s_planned_total_angle = SPIN_TARGET_ANGLE_DEFAULT;
    s_planned_exit_yaw_deg = 0.0f;
    s_spin_speed_sign = 1.0f;
}

// 设置本次旋转动作的目标：总角度、退出航向、顺/逆时针方向。
void Minefield_SetSpinPlan(float total_spin_deg, float exit_yaw_deg, float spin_speed_sign)
{
    s_planned_total_angle = total_spin_deg;
    if (s_planned_total_angle < SPIN_TARGET_ANGLE_MIN)
    {
        s_planned_total_angle = SPIN_TARGET_ANGLE_MIN;
    }

    s_planned_exit_yaw_deg = Minefield_NormalizeAngle(exit_yaw_deg);
    s_spin_speed_sign = (spin_speed_sign >= 0.0f) ? 1.0f : -1.0f;
}

uint8_t Minefield_Is_Active(void)
{
    return s_is_spinning;
}

float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg, volatile float* target_yaw_ptr)
{
    float remaining;
    float yaw_err;
    float target_speed = 0.0f;

    // 外部将 minefield_flag 置 1 后，这里正式进入旋转状态机。
    if (minefield_flag == 1)
    {
        minefield_flag = 0;
        s_is_spinning = 1;
        s_accumulated_angle = 0.0f;
        s_current_speed_cmd = 0.0f;
    }

    if (s_is_spinning == 0)
    {
        return 0.0f;
    }

    s_accumulated_angle += fabsf(gyro_z_deg * dt_s);
    remaining = s_planned_total_angle - s_accumulated_angle;
    yaw_err = Minefield_NormalizeAngle(s_planned_exit_yaw_deg - current_yaw_deg);

    if ((s_accumulated_angle >= s_planned_total_angle) &&
        (fabsf(yaw_err) <= SPIN_EXIT_YAW_TOLERANCE))
    {
        s_is_spinning = 0;
        s_current_speed_cmd = 0.0f;
        g_special_action_trigger = 0;

        if (target_yaw_ptr != 0)
        {
            *target_yaw_ptr = current_yaw_deg;
        }

        return 0.0f;
    }

    // 采用“匀速 + 末段线性减速”的简单梯形速度思想，兼顾快转和出框姿态稳定。
    if (remaining < SPIN_DECEL_ANGLE)
    {
        target_speed = SPIN_MAX_SPEED * (remaining / SPIN_DECEL_ANGLE);
        if (target_speed < SPIN_MIN_SPEED)
        {
            target_speed = SPIN_MIN_SPEED;
        }
    }
    else
    {
        target_speed = SPIN_MAX_SPEED;
    }

    s_current_speed_cmd = Minefield_RampFloat(s_current_speed_cmd, target_speed, SPIN_ACCEL_STEP);
    return s_current_speed_cmd * s_spin_speed_sign * SPIN_OUTPUT_SIGN;
}
