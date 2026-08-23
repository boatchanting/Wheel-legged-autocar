#include "minefield.h"
#include "../config/sys_options.h"

extern uint8 g_special_action_trigger;

// 默认旋转总角度（deg）；当前统一要求至少 725 度。
#define SPIN_TARGET_ANGLE_DEFAULT MINEFIELD_SPIN_MIN_TOTAL_ANGLE
// 旋转总角度下限（deg）；外部即使给得更小，也会被钳到这个值。
#define SPIN_TARGET_ANGLE_MIN     MINEFIELD_SPIN_MIN_TOTAL_ANGLE
// 旋转阶段的最大角速度指令（deg/s）。
#define SPIN_MAX_SPEED            1800.0f   //华东给的1700.0f//角速度指令上限
// 减速区角度（deg）；进入最后这段角度后开始线性收速。
#define SPIN_DECEL_ANGLE          1500.0f
// 旋转末段的最小角速度指令（deg/s）；避免末段因速度过低卡住。
#define SPIN_MIN_SPEED            500.0f
// 旋转输出符号；用于统一适配底层角速度方向定义。
#define SPIN_OUTPUT_SIGN          1.0f
#define MINEFIELD_SPIN_COAST_ANGLE_DEG            150.0f  // 距计划总角度剩余此角度时关闭转向环，靠惯性继续出圈
#define MINEFIELD_SPIN_CAPTURE_MAX_SPEED_DPS      500.0f  // 惯性未命中出口时的低速捕获角速度上限，调小可减轻末端打滑
#define MINEFIELD_SPIN_CAPTURE_YAW_KP             8.0f    // 低速捕获航向比例增益：出口航向误差每度换算为的角速度指令
#define MINEFIELD_SPIN_MAX_DURATION_S       7.5f
#define MINEFIELD_SPIN_STALL_CMD_MIN_DPS    120.0f
#define MINEFIELD_SPIN_STALL_GYRO_MIN_DPS   25.0f
#define MINEFIELD_SPIN_STALL_MAX_DURATION_S 2.0f
volatile uint8_t minefield_flag = 0;
static uint8_t  s_is_spinning = 0;
static MinefieldSpinPhase_e s_spin_phase = MINEFIELD_SPIN_PHASE_IDLE;
static float    s_accumulated_angle = 0.0f;
static float    s_current_speed_cmd = 0.0f;
static float    s_planned_total_angle = SPIN_TARGET_ANGLE_DEFAULT;
static float    s_spin_speed_sign = 1.0f;
static float    s_exit_yaw_deg = 0.0f;
static uint8_t  s_exit_release_enabled = 0;
static float    s_spin_elapsed_s = 0.0f;
static float    s_spin_stall_elapsed_s = 0.0f;
static float    s_feedforward_speed = 0.0f;  // 前馈速度跟踪，用于加速爬升
uint8_t vision_detected_marker = 0;
volatile uint8_t g_minefield_spin_abort_reason = MINEFIELD_SPIN_ABORT_NONE;
volatile uint8_t g_minefield_beep_request = 0; // 自转结束蜂鸣器请求标志

static float Minefield_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float Minefield_ExitYawError(float current_yaw_deg)
{
    return Minefield_NormalizeAngle(s_exit_yaw_deg - current_yaw_deg);
}

static void Minefield_FinishSpin(uint8_t abort_reason)
{
    s_is_spinning = 0;
    s_spin_phase = MINEFIELD_SPIN_PHASE_IDLE;
    s_current_speed_cmd = 0.0f;
    s_exit_release_enabled = 0;
    s_spin_elapsed_s = 0.0f;
    s_spin_stall_elapsed_s = 0.0f;
    g_minefield_spin_abort_reason = abort_reason;
    g_special_action_trigger = 0;
    g_minefield_beep_request = 1; // 请求蜂鸣器响一下
}

static uint8_t Minefield_ShouldReleaseByExitYaw(float current_yaw_deg)
{
    float exit_yaw_err;

    if ((s_exit_release_enabled == 0) ||
        (s_accumulated_angle < MINEFIELD_SPIN_MIN_TOTAL_ANGLE))
    {
        return 0;
    }

    exit_yaw_err = Minefield_ExitYawError(current_yaw_deg);
    return (uint8_t)(fabsf(exit_yaw_err) <= MINEFIELD_SPIN_EXIT_RELEASE_YAW_TOLERANCE);
}

void Minefield_Init(void)
{
    minefield_flag = 0;
    s_is_spinning = 0;
    s_spin_phase = MINEFIELD_SPIN_PHASE_IDLE;
    s_accumulated_angle = 0.0f;
    s_current_speed_cmd = 0.0f;
    s_planned_total_angle = SPIN_TARGET_ANGLE_DEFAULT;
    s_spin_speed_sign = 1.0f;
    s_exit_yaw_deg = 0.0f;
    s_exit_release_enabled = 0;
    s_spin_elapsed_s = 0.0f;
    s_spin_stall_elapsed_s = 0.0f;
    s_feedforward_speed = 0.0f;
    g_minefield_spin_abort_reason = MINEFIELD_SPIN_ABORT_NONE;
}

// 设置本次旋转动作：total_spin_deg 是兜底总角，exit_yaw_deg 用于 725 度后的提前释放。
void Minefield_SetSpinPlan(float total_spin_deg, float exit_yaw_deg, float spin_speed_sign)
{
    s_planned_total_angle = total_spin_deg;
    if (s_planned_total_angle < SPIN_TARGET_ANGLE_MIN)
    {
        s_planned_total_angle = SPIN_TARGET_ANGLE_MIN;
    }

    s_exit_yaw_deg = exit_yaw_deg;
    s_exit_release_enabled = 1;
    s_spin_speed_sign = (spin_speed_sign >= 0.0f) ? 1.0f : -1.0f;
    g_minefield_spin_abort_reason = MINEFIELD_SPIN_ABORT_NONE;
}

// 设置精确旋转角度（用于调试PD控制器），可选择是否启用航向提前释放。
void Minefield_SetSpinPlanExact(float total_spin_deg, float spin_speed_sign, uint8_t enable_exit_release)
{
    s_planned_total_angle = total_spin_deg;
    s_exit_yaw_deg = 0.0f;
    s_exit_release_enabled = enable_exit_release;  // 由调用方决定是否启用
    s_spin_speed_sign = (spin_speed_sign >= 0.0f) ? 1.0f : -1.0f;
    g_minefield_spin_abort_reason = MINEFIELD_SPIN_ABORT_NONE;
}

uint8_t Minefield_Is_Active(void)
{
    return s_is_spinning;
}

uint8_t Minefield_IsCoasting(void)
{
    return (uint8_t)(s_spin_phase == MINEFIELD_SPIN_PHASE_COAST);
}

MinefieldSpinPhase_e Minefield_GetSpinPhase(void)
{
    return s_spin_phase;
}

volatile float g_minefield_debug_accumulated_angle = 0.0f;
volatile float g_minefield_debug_angle_cmd = 0.0f;
volatile float g_minefield_debug_feedforward_speed = 0.0f;
volatile float g_minefield_debug_current_speed_cmd = 0.0f;
volatile float g_minefield_debug_stall_elapsed_s = 0.0f;
volatile uint8_t g_minefield_debug_phase = MINEFIELD_SPIN_PHASE_IDLE;
volatile float g_minefield_debug_exit_yaw_error = 0.0f;

float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg, volatile float* target_yaw_ptr)
{
    float remaining;
    float target_speed = 0.0f;
    float gyro_abs;
    float exit_yaw_error;

    (void)target_yaw_ptr;

    if (dt_s <= 0.0f)
    {
        dt_s = 0.001f;
    }

    // 外部将 minefield_flag 置 1 后，这里正式进入旋转状态机。
    if (minefield_flag == 1)
    {
        minefield_flag = 0;
        s_is_spinning = 1;
        s_spin_phase = MINEFIELD_SPIN_PHASE_DRIVE;
        s_accumulated_angle = 0.0f;
        s_current_speed_cmd = 0.0f;
        s_spin_elapsed_s = 0.0f;
        s_spin_stall_elapsed_s = 0.0f;
        s_feedforward_speed = 0.0f;
        g_minefield_spin_abort_reason = MINEFIELD_SPIN_ABORT_NONE;
    }

    if (s_is_spinning == 0)
    {
        return 0.0f;
    }

    gyro_abs = fabsf(gyro_z_deg);
    s_spin_elapsed_s += dt_s;
    s_accumulated_angle += gyro_abs * dt_s;
    remaining = s_planned_total_angle - s_accumulated_angle;
    exit_yaw_error = Minefield_ExitYawError(current_yaw_deg);

    if (Minefield_ShouldReleaseByExitYaw(current_yaw_deg) != 0)
    {
        Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_NONE);
        return 0.0f;
    }

    if (s_spin_elapsed_s >= MINEFIELD_SPIN_MAX_DURATION_S)
    {
        Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_TIMEOUT);
        return 0.0f;
    }

    // 梯形速度规划（前馈）：加速爬升 + 匀速 + 末段线性减速到 0。
#if MINEFIELD_INERTIAL_BUFFER_ENABLE
    if (s_spin_phase == MINEFIELD_SPIN_PHASE_COAST)
    {
        s_current_speed_cmd = 0.0f;
        s_feedforward_speed = 0.0f;

        if (s_accumulated_angle < s_planned_total_angle)
        {
            g_minefield_debug_accumulated_angle = s_accumulated_angle;
            g_minefield_debug_angle_cmd = 0.0f;
            g_minefield_debug_feedforward_speed = 0.0f;
            g_minefield_debug_current_speed_cmd = 0.0f;
            g_minefield_debug_stall_elapsed_s = 0.0f;
            g_minefield_debug_phase = (uint8_t)s_spin_phase;
            g_minefield_debug_exit_yaw_error = exit_yaw_error;
            return 0.0f;
        }

        if (s_exit_release_enabled == 0U)
        {
            Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_NONE);
            return 0.0f;
        }
        s_spin_phase = MINEFIELD_SPIN_PHASE_CAPTURE;
    }

    if (s_spin_phase == MINEFIELD_SPIN_PHASE_CAPTURE)
    {
        target_speed = fabsf(exit_yaw_error) * MINEFIELD_SPIN_CAPTURE_YAW_KP;
        if (target_speed > MINEFIELD_SPIN_CAPTURE_MAX_SPEED_DPS)
        {
            target_speed = MINEFIELD_SPIN_CAPTURE_MAX_SPEED_DPS;
        }
        s_feedforward_speed = target_speed;
        s_current_speed_cmd = (exit_yaw_error >= 0.0f) ? target_speed : -target_speed;
        goto minefield_spin_stall_check;
    }

    if ((s_exit_release_enabled != 0U) &&
        (s_accumulated_angle >= (s_planned_total_angle - MINEFIELD_SPIN_COAST_ANGLE_DEG)))
    {
        s_spin_phase = MINEFIELD_SPIN_PHASE_COAST;
        s_current_speed_cmd = 0.0f;
        s_feedforward_speed = 0.0f;
        g_minefield_debug_accumulated_angle = s_accumulated_angle;
        g_minefield_debug_angle_cmd = 0.0f;
        g_minefield_debug_feedforward_speed = 0.0f;
        g_minefield_debug_current_speed_cmd = 0.0f;
        g_minefield_debug_stall_elapsed_s = 0.0f;
        g_minefield_debug_phase = (uint8_t)s_spin_phase;
        g_minefield_debug_exit_yaw_error = exit_yaw_error;
        return 0.0f;
    }

    if (s_accumulated_angle >= s_planned_total_angle)
    {
        Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_NONE);
        return 0.0f;
    }

#else
    if (s_accumulated_angle >= s_planned_total_angle)
    {
        Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_NONE);
        return 0.0f;
    }
#endif

    if (remaining < SPIN_DECEL_ANGLE)
    {
        // 减速阶段：线性收速
        target_speed = SPIN_MAX_SPEED * (remaining / SPIN_DECEL_ANGLE);
        
        // 保底速度，防止掉入死区引发单轮蠕动偏移和1秒延迟
        if (target_speed < SPIN_MIN_SPEED)
        {
            target_speed = SPIN_MIN_SPEED;
        }
        
        s_feedforward_speed = target_speed;
    }
    else
    {
        target_speed = SPIN_MAX_SPEED;
        s_feedforward_speed = target_speed;
    }

    s_current_speed_cmd = target_speed;

#if MINEFIELD_INERTIAL_BUFFER_ENABLE
minefield_spin_stall_check:
#endif
    if ((fabsf(s_current_speed_cmd) >= MINEFIELD_SPIN_STALL_CMD_MIN_DPS) &&
        (gyro_abs <= MINEFIELD_SPIN_STALL_GYRO_MIN_DPS))
    {
        s_spin_stall_elapsed_s += dt_s;
        if (s_spin_stall_elapsed_s >= MINEFIELD_SPIN_STALL_MAX_DURATION_S)
        {
            Minefield_FinishSpin(MINEFIELD_SPIN_ABORT_STALLED);
            return 0.0f;
        }
    }
    else
    {
        s_spin_stall_elapsed_s = 0.0f;
    }

    g_minefield_debug_accumulated_angle = s_accumulated_angle;
    g_minefield_debug_angle_cmd = 0.0f;
    g_minefield_debug_feedforward_speed = s_feedforward_speed;
    g_minefield_debug_current_speed_cmd = s_current_speed_cmd;
    g_minefield_debug_stall_elapsed_s = s_spin_stall_elapsed_s;
    g_minefield_debug_phase = (uint8_t)s_spin_phase;
    g_minefield_debug_exit_yaw_error = exit_yaw_error;

    if (s_spin_phase == MINEFIELD_SPIN_PHASE_CAPTURE)
    {
        return s_current_speed_cmd * SPIN_OUTPUT_SIGN;
    }
    return s_current_speed_cmd * s_spin_speed_sign * SPIN_OUTPUT_SIGN;
}
