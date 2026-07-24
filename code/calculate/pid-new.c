#include "zf_common_headfile.h"
#include "../servo/servo.h"
#include "../servo/servo_executor.h"


// ============================================================================
//  全局变量初始化
//  将宏定义的参数填入结构体
// ============================================================================
PID_Param_t pid_servo_speed = {SERVO_SPEED_KP, SERVO_SPEED_KI, SERVO_SPEED_KD, SERVO_SPEED_MAX_O, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP, 0,0,0,0,0};//舵机速度环初始化参数
PID_Param_t pid_angle = {ANG_KP, ANG_KI, ANG_KD, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO, 0,0,0,0,0};//角度环初始化参数
PID_Param_t pid_gyro  = {GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE, 0,0,0,0,0};//角速度环初始化参数
PID_Param_t pid_turn_angle = {TURN_ANG_KP, TURN_ANG_KI, TURN_ANG_KD, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE, 0,0,0,0,0};//转向角度环初始化参数
PID_Param_t pid_turn_gyro = {TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, TURN_GYR_MAX_O, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE, 0,0,0,0,0};//转向角速度环初始化参数
PID_Param_t pid_roll = {ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO, 0,0,0,0,0};//横滚环初始化参数


volatile ControlMode_e g_control_mode_requested = CONTROL_MODE_NORMAL; //外部调用(只在导航给就行，其他地方别写，放置冲突)给的目标pid场景
volatile ControlMode_e g_control_mode_applied = CONTROL_MODE_NORMAL;//内部状态，实际应用的pid场景，从外部请求过来后，经过平滑切换，再赋值给这个变量
ControlProfile_t g_control_profile_active;
static ControlProfile_t g_control_profile_target;

static const ControlProfile_t g_control_profile_normal = {
    SERVO_SPEED_KP, SERVO_SPEED_KI, SERVO_SPEED_KD, SERVO_SPEED_MAX_O, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP,
    ANG_KP, ANG_KI, ANG_KD, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO,
    GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE,
    TURN_ANG_KP, TURN_ANG_KI, TURN_ANG_KD, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE,
    TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, TURN_GYR_MAX_O, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE,
    ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO,
    BRAKE_GAIN_LIGHT, BRAKE_GAIN_MED, BRAKE_GAIN_HEAVY,
    BRAKE_MAX_LIGHT, BRAKE_MAX_MED, BRAKE_MAX_HEAVY,
    BRAKE_RAMP_UP_LIGHT, BRAKE_RAMP_UP_MED, BRAKE_RAMP_UP_HEAVY, BRAKE_RAMP_DOWN,
    ACCEL_FF_GAIN, ACCEL_FF_MAX, ACCEL_FF_RAMP_UP, ACCEL_FF_RAMP_DOWN,
    10.0f, 10.0f, 0.020f, 0.010f, 60.0f
};

static const ControlProfile_t g_control_profile_accel = {
    -5.4f, SERVO_SPEED_KI, -0.14f, 2600.0f, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP,
    -13.2f, ANG_KI, -11.8f, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO,
    GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE,
    TURN_ANG_KP, TURN_ANG_KI, TURN_ANG_KD, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE,
    TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, TURN_GYR_MAX_O, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE,
    ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO,
    3.2f, 8.5f, 18.0f,
    700.0f, 1450.0f, 3000.0f,
    100.0f, 260.0f, 620.0f, 850.0f,
    13.0f, 3600.0f, 1100.0f, 650.0f,
    22.0f, 16.0f, 0.028f, 0.013f, 95.0f
};// 【优化点】这些参数是ai随便写的，需要调整，以及可以扩展更多的场景

static const ControlProfile_t g_control_profile_brake = {
    -4.8f, SERVO_SPEED_KI, -0.22f, 2600.0f, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP,
    -10.8f, ANG_KI, -15.0f, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO,
    GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE,
    TURN_ANG_KP, TURN_ANG_KI, TURN_ANG_KD, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE,
    TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, 7200.0f, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE,
    ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO,
    4.8f, 11.5f, 25.0f,
    900.0f, 1800.0f, 3900.0f,
    150.0f, 360.0f, 900.0f, 900.0f,
    6.0f, 1800.0f, 450.0f, 900.0f,
    14.0f, 28.0f, 0.018f, 0.015f, 80.0f
};// 【优化点】这些参数是ai随便写的，需要调整，以及可以扩展更多的场景

volatile uint8 profile_switch_beep_request = 0U; // 复刻模式下PID切换蜂鸣请求

volatile float target_speed_set = 0.0f;

//状态与调试变量
volatile float now_speed       = 0.0f;
volatile float now_angle       = 0.0f;
volatile float now_gyro        = 0.0f;
float current_actual_speed = 0.0f; // 当前实际速度变量（单位：r/min）
float speed_loop_out    = 0.0f;// 速度环的输出 (目标角度) 这个值恒定不变，基本相当于没有使用，其更改会改变车身倾角，影响比较小，并且导致pid复杂度上升，先不动(date0707)
float angle_loop_out    = 0.0f;// 角度环的输出 (目标角速度)
float gyro_loop_out     = 0.0f;// 角速度环的输出 (目标角加速度)
volatile float turn_angle_loop_out = 0.0f;// 转向角度环输出（期望角速度）
volatile float turn_gyro_loop_out = 0.0f;// 转向角速度环输出（PWM）
volatile float final_motor_pwm = 0.0f;
uint8_t roll_balance_enable = ROLL_BALANCE_ENABLE_INIT; // Rolling平衡环统一使能开关
volatile int16 g_target_pwm_roll_adj = 0; // 目标横滚调整分量
volatile int16 g_target_pwm_turn_roll_lf = 0; // 转向主动侧倾左前查表差动
volatile int16 g_target_pwm_turn_roll_rf = 0; // 转向主动侧倾右前查表差动
volatile int16 g_target_pwm_turn_roll_rr = 0; // 转向主动侧倾右后查表差动
volatile int16 g_target_pwm_turn_roll_lr = 0; // 转向主动侧倾左后查表差动
volatile float g_turn_active_roll_height_delta_cm = 0.0f; // 转向主动侧倾单侧目标高度差，便于实车观测
volatile float g_turn_active_roll_request_degree = 0.0f; // 未斜率限制前的主动侧倾目标角
volatile float g_turn_active_roll_forward_speed_mps = 0.0f; // 向心加速度计算用纵向速度
volatile float g_turn_active_roll_yaw_rate_radps = 0.0f; // 向心加速度计算用实际 yaw 角速度
volatile float g_turn_active_roll_lateral_accel_mps2 = 0.0f; // v*w 计算得到的向心加速度
static uint8 turn_active_roll_extend_only_side = 0U; // 主动侧倾伸腿锁存：0未锁，1左侧不收只伸右侧，2右侧不收只伸左侧
static float brake_ff_pwm = 0.0f;
static float brake_ff_target = 0.0f;
static float brake_last_target_speed = 0.0f;
static uint8 brake_nav_hard_stop_active = 0U;      // 导航强停刹锁存，主要由科目二雷区准备圆请求
static uint8 brake_nav_hard_stop_life_ticks = 0U;  // 强停刹保持计数，用于桥接导航周期和刹车前馈周期
static float brake_nav_hard_stop_strength = 0.0f;  // 导航强停刹强度，0.0 释放，1.0 等价旧强停刹
static uint8 brake_lockout = 0;     // 重置屏蔽锁
static uint8 brake_zero_hold = 0;   // 刹停零速迟滞锁，避免停车附近反复建压/释放
static uint8 brake_overspeed_ticks = 0U;  // 持续超速计数，达到 BRAKE_OVERSPEED_HOLD_TICKS 后才允许纠偏刹车
static float accel_ff_pwm = 0.0f;              // 当前实际输出的加速前馈 PWM，经过斜率限制后用于最终融合
static float accel_ff_target = 0.0f;           // 本周期期望加速前馈 PWM，先限幅再由 accel_ff_pwm 追踪
static float accel_kp_boost = 1.0f;            // Kp 增强模式下的舵机速度环 Kp 倍率，未触发前馈时保持 1.0
static float accel_last_target_speed = 0.0f;   // 上一次 9ms 更新时的目标速度，用于判断目标速度是否明显抬升
static uint16 accel_start_window_ticks = 0U;   // 复刻启动/目标跃升后的加速窗口剩余 tick 数，每 tick 约 9ms
static uint8 accel_last_replay_running = 0U;   // 上一次更新时复刻是否运行，用于检测 REPLAY_RUNNING 上升沿

/**
 * @brief 将毫秒窗口换算成加速前馈调用 tick 数
 * @param time_ms 需要保持加速补偿的时间，单位 ms
 * @return 对应的 9ms 控制周期数量；调大窗口宏会让起步/出弯补偿持续更久
 */
static uint16 Accel_Feedforward_MsToTicks(uint16 time_ms)
{
    return (uint16)((time_ms + ACCEL_FF_UPDATE_PERIOD_MS - 1U) / ACCEL_FF_UPDATE_PERIOD_MS);
}

static void Accel_Feedforward_UpdateKpBoost(uint8 accel_request)
{
    // 加速请求有效时平滑拉高 Kp 倍率；请求消失后按较慢斜率回落，避免速度环突变。
    float target_boost = (accel_request != 0U) ? ACCEL_KP_BOOST_MAX : 1.0f;
    float ramp_limit = (target_boost > accel_kp_boost) ? ACCEL_KP_BOOST_RAMP_UP : ACCEL_KP_BOOST_RAMP_DOWN;

    accel_kp_boost += Float_Constrain(target_boost - accel_kp_boost, -ramp_limit, ramp_limit);
    if (fabsf(accel_kp_boost - 1.0f) < 0.001f)
    {
        accel_kp_boost = 1.0f;
    }
}

/**
 * @brief 清空加速前馈内部输出和补偿窗口
 * @note 电机关闭、跳跃、强制刹车、特殊任务接管时调用，避免残留前馈继续推车
 */
static void Accel_Feedforward_ClearOutput(void)
{
    accel_ff_pwm = 0.0f;
    accel_ff_target = 0.0f;
    accel_kp_boost = 1.0f;
    accel_start_window_ticks = 0U;
}

// 清空导航强停刹请求；退出雷区准备圆、停车完成或刹车前馈复位时调用。
void Brake_NavHardStop_Reset(void)
{
    brake_nav_hard_stop_active = 0U;
    brake_nav_hard_stop_life_ticks = 0U;
    brake_nav_hard_stop_strength = 0.0f;
}

// 更新导航强停刹强度；strength 为 0 时释放，大于 0 时刷新保持计数。
void Brake_NavHardStop_UpdateStrength(float strength)
{
    strength = Float_Constrain(strength, 0.0f, 1.0f);

    if (strength > 0.0f)
    {
        brake_nav_hard_stop_active = 1U;
        brake_nav_hard_stop_life_ticks = NAV_HARD_BRAKE_LIFE_TICKS;
        brake_nav_hard_stop_strength = strength;
    }
    else
    {
        Brake_NavHardStop_Reset();
    }
}

// 更新导航强停刹请求；active 为 1 时等价于满强度，兼容旧调用点。
void Brake_NavHardStop_Update(uint8 active)
{
    Brake_NavHardStop_UpdateStrength((active != 0U) ? 1.0f : 0.0f);
}

float Brake_Feedforward_Update(float target_speed, float actual_speed, uint8 motor_enable, uint8 jump_flag)
{
    float abs_speed = fabsf(actual_speed);
    float abs_target = fabsf(target_speed);
    float abs_err = fabsf(target_speed - actual_speed);
    float abs_last_target = fabsf(brake_last_target_speed);
    float err_ratio = 0.0f;
    float brake_gain = 0.0f;
    float brake_max = 0.0f;
    float brake_ramp_up = g_control_profile_active.brake_ramp_up_light;
    uint8 brake_level = 0;
    uint8 target_decel_cmd = 0U;
    uint8 overspeed_request = 0U;
    uint8 decel_request = 0U;
    uint8 nav_hard_stop_request = 0U;
    uint8 zero_stop_request = 0U;
    float nav_hard_stop_strength = 0.0f;

    if ((motor_enable == 0U) || (jump_flag != 0U) || (abs_speed <= NAV_HARD_BRAKE_RELEASE_SPEED))
    {
        Brake_NavHardStop_Reset();
    }
    else if ((brake_nav_hard_stop_active != 0U) && (brake_nav_hard_stop_life_ticks > 0U))
    {
        nav_hard_stop_request = 1U;
        nav_hard_stop_strength = brake_nav_hard_stop_strength;
        brake_nav_hard_stop_life_ticks--;
    }
    else if (brake_nav_hard_stop_active != 0U)
    {
        Brake_NavHardStop_Reset();
    }

    if (brake_zero_hold)
    {
        if ((abs_target <= BRAKE_ZERO_TARGET_MAX) && (abs_speed <= BRAKE_ZERO_HOLD_EXIT))
        {
            brake_ff_target = 0.0f;
            brake_ff_pwm = 0.0f;
            brake_last_target_speed = target_speed;
            return 0.0f;
        }
        brake_zero_hold = 0U;
        brake_overspeed_ticks = 0U;
    }

    if ((target_speed * brake_last_target_speed < 0.0f) ||
        (abs_target + BRAKE_TARGET_DECEL_MIN < abs_last_target))
    {
        target_decel_cmd = 1U;
    }

    // 目标速度持续为 0 时也要保持刹车请求，避免只在目标速度刚下降的一瞬间刹一下，后续靠惯性滑过目标点。
    zero_stop_request = (uint8)(abs_target <= BRAKE_ZERO_TARGET_MAX);

    // 持续超速计数：只有真实速度连续高于目标速度一段时间，才允许走“纠偏刹车”路径。
    // 这样可以避免目标速度轻微下调或编码器瞬时噪声，直接触发急刹。
    overspeed_request = (uint8)(abs_speed > (abs_target + BRAKE_OVERSPEED_ERR_MIN));
    if (overspeed_request != 0U)
    {
        if (brake_overspeed_ticks < 255U)
        {
            brake_overspeed_ticks++;
        }
    }
    else
    {
        brake_overspeed_ticks = 0U;
    }

    // 1. 正常的刹车条件判断
    if (motor_enable && (jump_flag == 0U) && (abs_speed > BRAKE_SPEED_DEADBAND))
    {
        // 刹车请求分三类：
        // 1) 目标速度跨零/反向：必须刹；
        // 2) 目标速度主动大幅下降：计划性收速，只允许轻/中刹逐级建立；
        // 3) 持续超速：真实超速纠偏，满足更高门槛后才允许重刹。
        decel_request = (uint8)((nav_hard_stop_request != 0U) ||
                                (zero_stop_request != 0U) ||
                                (actual_speed * target_speed <= 0.0f) ||
                                ((target_decel_cmd != 0U) && (abs_err >= BRAKE_ERR_MIN)) ||
                                ((brake_overspeed_ticks >= BRAKE_OVERSPEED_HOLD_TICKS) &&
                                 (abs_err >= BRAKE_OVERSPEED_ERR_MIN)));
        if (decel_request)
        {
            if (nav_hard_stop_request != 0U)
            {
                brake_level = 4U;
            }
            else if (zero_stop_request != 0U)
            {
                // 停车指令持续期间按当前速度保持刹车档位，正车/倒车同等处理。
                if (abs_speed < BRAKE_CH5_LIGHT_SPEED) brake_level = 1U;
                else if (abs_speed < BRAKE_CH5_MED_SPEED) brake_level = 2U;
                else brake_level = 3U;
            }
            else if (g_brake_active || g_reverse_brake_active)
            {
                if (abs_speed < BRAKE_CH5_LIGHT_SPEED) brake_level = 1U;
                else if (abs_speed < BRAKE_CH5_MED_SPEED) brake_level = 2U;
                else brake_level = 3U;
            }
            else
            {
                err_ratio = abs_err / abs_speed;
                if (target_decel_cmd && (abs_err >= BRAKE_ERR_MIN) && (err_ratio >= BRAKE_RATIO_LIGHT))
                {
                    brake_level = 1U;
                }
                if (target_decel_cmd &&
                    (abs_speed >= BRAKE_MED_SPEED_TH) &&
                    (abs_err >= BRAKE_ERR_MED_MIN) &&
                    (err_ratio >= BRAKE_RATIO_MED))
                {
                    brake_level = 2U;
                }
                if ((brake_overspeed_ticks >= BRAKE_OVERSPEED_HOLD_TICKS) &&
                    (abs_speed >= BRAKE_HEAVY_SPEED_TH) &&
                    (abs_err >= BRAKE_ERR_HEAVY_MIN) &&
                    (err_ratio >= BRAKE_RATIO_HEAVY))
                {
                    brake_level = 3U;
                }
            }

            if ((nav_hard_stop_request == 0U) && (g_brake_active == 0U) && (g_reverse_brake_active == 0U) && (abs_speed < BRAKE_LOW_SPEED_TH) && (brake_level > 1U)) brake_level = 1U;
        }
    }

    if (decel_request && (abs_target <= BRAKE_ZERO_TARGET_MAX) && (abs_speed <= BRAKE_ZERO_HOLD_ENTER))
    {
        brake_zero_hold = 1U;
        brake_ff_target = 0.0f;
        brake_ff_pwm = 0.0f;
        brake_last_target_speed = target_speed;
        return 0.0f;
    }

    if (brake_level == 4U)
    {
        brake_gain = g_control_profile_active.brake_gain_heavy + (NAV_HARD_BRAKE_GAIN - g_control_profile_active.brake_gain_heavy) * nav_hard_stop_strength;
        brake_max = g_control_profile_active.brake_max_heavy + (NAV_HARD_BRAKE_MAX_PWM - g_control_profile_active.brake_max_heavy) * nav_hard_stop_strength;
        brake_ramp_up = g_control_profile_active.brake_ramp_up_heavy + (NAV_HARD_BRAKE_RAMP_UP - g_control_profile_active.brake_ramp_up_heavy) * nav_hard_stop_strength;
    }
    else if (brake_level == 3U) { brake_gain = g_control_profile_active.brake_gain_heavy; brake_max = g_control_profile_active.brake_max_heavy; brake_ramp_up = g_control_profile_active.brake_ramp_up_heavy; }
    else if (brake_level == 2U) { brake_gain = g_control_profile_active.brake_gain_med; brake_max = g_control_profile_active.brake_max_med; brake_ramp_up = g_control_profile_active.brake_ramp_up_med; }
    else if (brake_level == 1U) { brake_gain = g_control_profile_active.brake_gain_light; brake_max = g_control_profile_active.brake_max_light; brake_ramp_up = g_control_profile_active.brake_ramp_up_light; }

    if (nav_hard_stop_request != 0U)
    {
        brake_lockout = 0U;
    }

    // 2. 【核心新增】处理屏蔽锁
    if (brake_lockout)
    {
        // 外部还在强制要求刹车，但因为处于 Reset 后的锁定状态，我们强行压制输出为 0
        brake_ff_target = 0.0f;
        brake_ff_pwm = 0.0f;
        if (brake_level != 0U)
        {
            brake_last_target_speed = target_speed;
            return 0.0f;
        }
        brake_lockout = 0;
    }

    // 3. 正常的输出计算
    brake_ff_target = (brake_gain > 0.0f) ? Float_Constrain(-brake_gain * actual_speed, -brake_max, brake_max) : 0.0f;
    brake_ff_pwm += Float_Constrain(brake_ff_target - brake_ff_pwm, -g_control_profile_active.brake_ramp_down, brake_ramp_up);
    brake_last_target_speed = target_speed;
    return brake_ff_pwm;
}

void Brake_Feedforward_Reset(void)
{
    // 清空内部变量
    brake_ff_pwm = 0.0f;
    brake_ff_target = 0.0f;
    brake_last_target_speed = 0.0f;
    brake_zero_hold = 0U;
    brake_overspeed_ticks = 0U;
    Brake_NavHardStop_Reset();
    brake_lockout = 1; // 【核心】上锁！无视接下来外部的强制刹车条件，直到外部条件自然释放为止
}

float Brake_Feedforward_GetPwm(void)
{
    return brake_ff_pwm;
}

/**
 * @brief 加速前馈更新
 * @param target_speed 导航/复刻给出的目标速度
 * @param actual_speed 编码器/速度估计得到的当前速度
 * @param motor_enable 电机使能，0 时清空前馈
 * @param jump_flag 跳跃保护标志，非 0 时清空前馈
 * @param replay_running 复刻运行标志，仅复刻运行时允许起步和出弯加速补偿
 * @param inhibit_accel 外部仲裁屏蔽标志，刹车较强或视觉/特殊任务接管时置 1
 * @return 本周期加速前馈 PWM；调大 ACCEL_FF_GAIN/RAMP_UP/MAX 会增强起步和出弯推力
 */
float Accel_Feedforward_Update(float target_speed, float actual_speed, uint8 motor_enable, uint8 jump_flag, uint8 replay_running, uint8 inhibit_accel)
{
#if ACCEL_FF_ENABLE && (ACCEL_FF_MODE != ACCEL_FF_MODE_DISABLE)
    float abs_target = fabsf(target_speed);
    float abs_speed = fabsf(actual_speed);
    float abs_last_target = fabsf(accel_last_target_speed);
    uint8 start_window_active = 0U;
    uint8 target_step_up = 0U;
    uint8 speed_lag = 0U;
    uint8 same_direction_or_start = 0U;
    uint8 accel_request = 0U;

    if ((replay_running != 0U) && (accel_last_replay_running == 0U))
    {
        accel_start_window_ticks = Accel_Feedforward_MsToTicks(ACCEL_FF_START_WINDOW_MS);
    }
    accel_last_replay_running = replay_running ? 1U : 0U;

    if ((motor_enable == 0U) ||
        (jump_flag != 0U) ||
        (replay_running == 0U) ||
        (inhibit_accel != 0U) ||
        (g_brake_active != 0U) ||
        (g_reverse_brake_active != 0U))
    {
        Accel_Feedforward_ClearOutput();
        accel_last_target_speed = target_speed;
        accel_last_replay_running = replay_running ? 1U : 0U;
        return 0.0f;
    }

    if (abs_target > ACCEL_FF_SPEED_DEADBAND)
    {
        if (((target_speed * accel_last_target_speed) > 0.0f) &&
            ((abs_target - abs_last_target) >= ACCEL_FF_TARGET_STEP_MIN))
        {
            target_step_up = 1U;
        }
        else if ((abs_last_target <= ACCEL_FF_SPEED_DEADBAND) &&
                 (abs_target >= ACCEL_FF_TARGET_STEP_MIN))
        {
            target_step_up = 1U;
        }

        if (target_step_up != 0U)
        {
            uint16 boost_ticks = Accel_Feedforward_MsToTicks(ACCEL_FF_BOOST_WINDOW_MS);
            if (accel_start_window_ticks < boost_ticks)
            {
                accel_start_window_ticks = boost_ticks;
            }
        }
    }

    start_window_active = (uint8)(accel_start_window_ticks > 0U);
    if (accel_start_window_ticks > 0U)
    {
        accel_start_window_ticks--;
    }

    if (abs_target <= ACCEL_FF_SPEED_DEADBAND)
    {
#if ACCEL_FF_MODE == ACCEL_FF_MODE_KP
        Accel_Feedforward_UpdateKpBoost(0U);
        accel_ff_target = 0.0f;
        accel_ff_pwm = 0.0f;
        accel_last_target_speed = target_speed;
        return 0.0f;
#else
        accel_ff_target = 0.0f;
        accel_ff_pwm += Float_Constrain(accel_ff_target - accel_ff_pwm,
                                        -g_control_profile_active.accel_ff_ramp_down,
                                        g_control_profile_active.accel_ff_ramp_down);
        if (fabsf(accel_ff_pwm) < 1.0f)
        {
            accel_ff_pwm = 0.0f;
        }
        accel_last_target_speed = target_speed;
        return accel_ff_pwm;
#endif
    }

    speed_lag = (uint8)(abs_target > (abs_speed + ACCEL_FF_ERR_MIN));
    same_direction_or_start = (uint8)(((target_speed * actual_speed) > 0.0f) ||
                                      (abs_speed <= ACCEL_FF_SPEED_DEADBAND));
    accel_request = (uint8)((abs_target > ACCEL_FF_SPEED_DEADBAND) &&
                            (same_direction_or_start != 0U) &&
                            (speed_lag != 0U) &&
                            ((target_step_up != 0U) || (start_window_active != 0U)));

#if ACCEL_FF_MODE == ACCEL_FF_MODE_KP
    Accel_Feedforward_UpdateKpBoost(accel_request);
    accel_ff_target = 0.0f;
    accel_ff_pwm = 0.0f;
    accel_last_target_speed = target_speed;
    return 0.0f;
#else

    if (accel_request != 0U)
    {
        float speed_deficit = abs_target - abs_speed;
        float target_dir = (target_speed >= 0.0f) ? 1.0f : -1.0f;
        accel_ff_target = Float_Constrain(ACCEL_FF_SIGN * g_control_profile_active.accel_ff_gain * speed_deficit * target_dir,
                                          -g_control_profile_active.accel_ff_max,
                                          g_control_profile_active.accel_ff_max);
    }
    else
    {
        accel_ff_target = 0.0f;
    }

    {
        float ramp_limit = g_control_profile_active.accel_ff_ramp_up;
        if ((accel_ff_target == 0.0f) ||
            ((accel_ff_pwm * accel_ff_target) < 0.0f) ||
            (fabsf(accel_ff_target) < fabsf(accel_ff_pwm)))
        {
            ramp_limit = g_control_profile_active.accel_ff_ramp_down;
        }
        accel_ff_pwm += Float_Constrain(accel_ff_target - accel_ff_pwm, -ramp_limit, ramp_limit);
    }

    if ((accel_ff_target == 0.0f) && (fabsf(accel_ff_pwm) < 1.0f))
    {
        accel_ff_pwm = 0.0f;
    }

    accel_last_target_speed = target_speed;
    return accel_ff_pwm;
#endif
#else
    (void)target_speed;
    (void)actual_speed;
    (void)motor_enable;
    (void)jump_flag;
    (void)replay_running;
    (void)inhibit_accel;
    Accel_Feedforward_ClearOutput();
    accel_last_target_speed = 0.0f;
    accel_last_replay_running = 0U;
    return 0.0f;
#endif
}

/**
 * @brief 外部强制复位加速前馈
 * @note 复刻停止、锁定刹车或任务切换时调用，防止下一次发车继承上次前馈状态
 */
void Accel_Feedforward_Reset(void)
{
    Accel_Feedforward_ClearOutput();
    accel_last_target_speed = 0.0f;
    accel_last_replay_running = 0U;
}

/**
 * @brief 读取当前加速前馈 PWM
 * @return 已经经过斜率限制后的加速前馈输出，用于 ISR 中和刹车前馈仲裁
 */
float Accel_Feedforward_GetPwm(void)
{
    return accel_ff_pwm;
}

float Accel_Feedforward_GetKpBoost(void)
{
    return accel_kp_boost;
}

// ============================================================================
//  辅助函数实现
// ============================================================================

/**
 * @brief 由最大高度差换算主动侧倾能实际执行的最大横滚角
 * @note roll_degree 不允许超过这个角度，避免腿部已经饱和时 Rolling 环继续硬追目标。
 */
static float Turn_Active_Roll_Executable_Max_Deg(void)
{
    float max_roll = atan2f(TURN_ACTIVE_ROLL_HEIGHT_MAX_CM,
                            TURN_ACTIVE_ROLL_HALF_TRACK_CM) *
                     TURN_ACTIVE_ROLL_RAD_TO_DEG;

    return Float_Constrain(max_roll, 0.0f, TURN_ACTIVE_ROLL_MAX);
}

/**
 * @brief 根据纵向速度和转向角速度计算普通转向主动侧倾目标
 * @note 这里只返回目标横滚角，不写舵机执行量；斜率限制由调用处用 roll_degree 完成。
 */
float Turn_Active_Roll_Target_Update(float turn_cmd, uint8 hard_clear)
{
    float desired = 0.0f;
    float executable_max_deg = Turn_Active_Roll_Executable_Max_Deg();
    float speed_for_roll_raw = current_actual_speed +
                               TURN_ACTIVE_ROLL_SPEED_PREVIEW_RATIO *
                               (target_speed_set - current_actual_speed);
    float abs_speed_raw = fabsf(speed_for_roll_raw);
    float abs_yaw_rate_dps = fabsf(turn_cmd);

    g_turn_active_roll_request_degree = 0.0f;
    g_turn_active_roll_forward_speed_mps = 0.0f;
    g_turn_active_roll_yaw_rate_radps = 0.0f;
    g_turn_active_roll_lateral_accel_mps2 = 0.0f;

    if ((roll_balance_enable == 0U) || (hard_clear != 0U))
    {
        return 0.0f;
    }

    if ((abs_speed_raw > TURN_ACTIVE_ROLL_SPEED_DEADBAND) &&
        (abs_yaw_rate_dps > TURN_ACTIVE_ROLL_YAW_RATE_DEAD_DPS))
    {
        float forward_speed_mps = TURN_ACTIVE_ROLL_FORWARD_SPEED_SIGN *
                                  speed_for_roll_raw *
                                  TURN_ACTIVE_ROLL_SPEED_TO_MPS;
        float yaw_rate_radps = turn_cmd * TURN_ACTIVE_ROLL_DEG_TO_RAD;
        float lateral_accel_mps2 = forward_speed_mps * yaw_rate_radps;

        g_turn_active_roll_forward_speed_mps = forward_speed_mps;
        g_turn_active_roll_yaw_rate_radps = yaw_rate_radps;
        g_turn_active_roll_lateral_accel_mps2 = lateral_accel_mps2;

        desired = TURN_ACTIVE_ROLL_SIGN *
                  atan2f(lateral_accel_mps2, TURN_ACTIVE_ROLL_GRAVITY_MPS2) *
                  TURN_ACTIVE_ROLL_RAD_TO_DEG;
        desired = Float_Constrain(desired,
                                  -executable_max_deg,
                                  executable_max_deg);
    }

    g_turn_active_roll_request_degree = desired;
    return desired;
}

static void Turn_Active_Roll_Duty_Clear(void)
{
    g_target_pwm_turn_roll_lf = 0;
    g_target_pwm_turn_roll_rf = 0;
    g_target_pwm_turn_roll_rr = 0;
    g_target_pwm_turn_roll_lr = 0;
    g_turn_active_roll_height_delta_cm = 0.0f;
    g_turn_active_roll_request_degree = 0.0f;
    g_turn_active_roll_forward_speed_mps = 0.0f;
    g_turn_active_roll_yaw_rate_radps = 0.0f;
    g_turn_active_roll_lateral_accel_mps2 = 0.0f;
    turn_active_roll_extend_only_side = 0U;
}

static int16 Turn_Active_Roll_Duty_Deadband(int16 duty)
{
    if ((duty < TURN_ACTIVE_ROLL_DUTY_DEADBAND) &&
        (duty > -TURN_ACTIVE_ROLL_DUTY_DEADBAND))
    {
        return 0;
    }
    return duty;
}

static uint8 Turn_Active_Roll_Duty_Is_OverLimit(int32 duty, int32 min_duty, int32 max_duty)
{
    /* 收腿侧如果会撞到舵机 duty 边界，就取消这一侧收腿，避免低车身时继续压腿。 */
    if ((duty < min_duty) || (duty > max_duty))
    {
        return 1U;
    }
    return 0U;
}

static int32 Turn_Active_Roll_Duty_Abs(int32 value)
{
    return (value < 0) ? -value : value;
}

static int32 Turn_Active_Roll_Duty_Shrink_Room(int32 duty, int32 dir, int32 min_duty, int32 max_duty)
{
    int32 room;

    /* DIR>0 时收腿会让 duty 变小；DIR<0 时收腿会让 duty 变大。 */
    if (dir > 0)
    {
        room = duty - min_duty;
    }
    else
    {
        room = max_duty - duty;
    }

    return (room > 0) ? room : 0;
}

static int16 Turn_Active_Roll_Height_To_Pwm(float height_cm)
{
    high_control_table(height_cm);
    if (pwm_high == 10000)
    {
        return 0;
    }
    return pwm_high;
}

/**
 * @brief 根据目标横滚角查表生成普通转向主动侧倾的四腿差动量
 * @note 正横滚目标表示右侧抬高：左侧收腿、右侧伸腿；负横滚目标相反。
 */
void Turn_Active_Roll_Duty_Update(float target_roll, uint8 hard_clear)
{
    float roll_rad;
    float height_delta_cm;
    float plan_roll;
    float plan_height_delta_cm;
    float executable_max_deg;
    float shrink_capacity_cm;
    float left_height_cm;
    float right_height_cm;
    int16 base_pwm;
    int16 left_pwm;
    int16 right_pwm;
    int16 left_delta;
    int16 right_delta;
    int16 shrink_probe_pwm;
    int32 shrink_probe_need_duty;
    int32 pre_turn_duty_lf;
    int32 pre_turn_duty_rf;
    int32 pre_turn_duty_rr;
    int32 pre_turn_duty_lr;
    uint8 left_shrink_not_enough = 0U;
    uint8 right_shrink_not_enough = 0U;
    uint8 shrink_side = 0U;

    if ((roll_balance_enable == 0U) || (hard_clear != 0U) ||
        (servo_height < P_min) || (servo_height > P_max))
    {
        Turn_Active_Roll_Duty_Clear();
        return;
    }

    executable_max_deg = Turn_Active_Roll_Executable_Max_Deg();
    target_roll = Float_Constrain(target_roll,
                                  -executable_max_deg,
                                  executable_max_deg);
    if (fabsf(target_roll) < TURN_ACTIVE_ROLL_TARGET_DEAD_DEG)
    {
        Turn_Active_Roll_Duty_Clear();
        return;
    }

    base_pwm = Turn_Active_Roll_Height_To_Pwm(servo_height);
    if (pwm_high == 10000)
    {
        Turn_Active_Roll_Duty_Clear();
        return;
    }

    roll_rad = target_roll * TURN_ACTIVE_ROLL_DEG_TO_RAD;
    height_delta_cm = TURN_ACTIVE_ROLL_HALF_TRACK_CM * tanf(roll_rad);
    height_delta_cm = Float_Constrain(height_delta_cm,
                                      -TURN_ACTIVE_ROLL_HEIGHT_MAX_CM,
                                      TURN_ACTIVE_ROLL_HEIGHT_MAX_CM);

    plan_roll = g_turn_active_roll_request_degree;
    if (fabsf(plan_roll) < fabsf(target_roll))
    {
        plan_roll = target_roll;
    }
    plan_roll = Float_Constrain(plan_roll,
                                -executable_max_deg,
                                executable_max_deg);
    plan_height_delta_cm = TURN_ACTIVE_ROLL_HALF_TRACK_CM *
                           tanf(plan_roll * TURN_ACTIVE_ROLL_DEG_TO_RAD);
    plan_height_delta_cm = Float_Constrain(plan_height_delta_cm,
                                           -TURN_ACTIVE_ROLL_HEIGHT_MAX_CM,
                                           TURN_ACTIVE_ROLL_HEIGHT_MAX_CM);

    if (plan_height_delta_cm > 0.0f)
    {
        shrink_side = 1U;
    }
    else if (plan_height_delta_cm < 0.0f)
    {
        shrink_side = 2U;
    }

    if ((turn_active_roll_extend_only_side != 0U) &&
        (turn_active_roll_extend_only_side != shrink_side))
    {
        turn_active_roll_extend_only_side = 0U;
    }

    left_height_cm = Float_Constrain(servo_height - height_delta_cm, P_min, P_max);
    right_height_cm = Float_Constrain(servo_height + height_delta_cm, P_min, P_max);
    left_pwm = Turn_Active_Roll_Height_To_Pwm(left_height_cm);
    right_pwm = Turn_Active_Roll_Height_To_Pwm(right_height_cm);
    left_delta = left_pwm - base_pwm;
    right_delta = right_pwm - base_pwm;

    pre_turn_duty_lf = SERVO_MOTOR_PWM1_90 + SERVO_MOTOR_PWM1_DIR * base_pwm +
                       SERVO_MOTOR_PWM1_DIR * g_target_pwm_speed_adj -
                       g_target_pwm_angle_adj;
    pre_turn_duty_rf = SERVO_MOTOR_PWM2_90 + SERVO_MOTOR_PWM2_DIR * base_pwm +
                       SERVO_MOTOR_PWM2_DIR * g_target_pwm_speed_adj -
                       g_target_pwm_angle_adj;
    pre_turn_duty_rr = SERVO_MOTOR_PWM3_90 + SERVO_MOTOR_PWM3_DIR * base_pwm -
                       SERVO_MOTOR_PWM3_DIR * g_target_pwm_speed_adj +
                       g_target_pwm_angle_adj;
    pre_turn_duty_lr = SERVO_MOTOR_PWM4_90 + SERVO_MOTOR_PWM4_DIR * base_pwm -
                       SERVO_MOTOR_PWM4_DIR * g_target_pwm_speed_adj +
                       g_target_pwm_angle_adj;

    shrink_capacity_cm = servo_height - P_min;
    if (shrink_capacity_cm < 0.0f)
    {
        shrink_capacity_cm = 0.0f;
    }
    shrink_probe_pwm = Turn_Active_Roll_Height_To_Pwm(
        Float_Constrain(servo_height - fabsf(plan_height_delta_cm), P_min, P_max));
    shrink_probe_need_duty = Turn_Active_Roll_Duty_Abs((int32)shrink_probe_pwm - (int32)base_pwm) +
                             TURN_ACTIVE_ROLL_SHRINK_DUTY_MARGIN;

    if (fabsf(plan_height_delta_cm) >
        (shrink_capacity_cm - TURN_ACTIVE_ROLL_SHRINK_HEIGHT_MARGIN_CM))
    {
        if (plan_height_delta_cm > 0.0f)
        {
            left_shrink_not_enough = 1U;
        }
        else if (plan_height_delta_cm < 0.0f)
        {
            right_shrink_not_enough = 1U;
        }
    }

    if ((Turn_Active_Roll_Duty_Shrink_Room(pre_turn_duty_lf,
                                           SERVO_MOTOR_PWM1_DIR,
                                           LF_LIMIT_DUTY_MIN,
                                           LF_LIMIT_DUTY_MAX) < shrink_probe_need_duty) ||
        (Turn_Active_Roll_Duty_Shrink_Room(pre_turn_duty_lr,
                                           SERVO_MOTOR_PWM4_DIR,
                                           LR_LIMIT_DUTY_MIN,
                                           LR_LIMIT_DUTY_MAX) < shrink_probe_need_duty))
    {
        left_shrink_not_enough = 1U;
    }

    if ((Turn_Active_Roll_Duty_Shrink_Room(pre_turn_duty_rf,
                                           SERVO_MOTOR_PWM2_DIR,
                                           RF_LIMIT_DUTY_MIN,
                                           RF_LIMIT_DUTY_MAX) < shrink_probe_need_duty) ||
        (Turn_Active_Roll_Duty_Shrink_Room(pre_turn_duty_rr,
                                           SERVO_MOTOR_PWM3_DIR,
                                           RR_LIMIT_DUTY_MIN,
                                           RR_LIMIT_DUTY_MAX) < shrink_probe_need_duty))
    {
        right_shrink_not_enough = 1U;
    }

    if ((shrink_side == 1U) && (left_shrink_not_enough != 0U))
    {
        turn_active_roll_extend_only_side = 1U;
    }
    else if ((shrink_side == 2U) && (right_shrink_not_enough != 0U))
    {
        turn_active_roll_extend_only_side = 2U;
    }

    if (turn_active_roll_extend_only_side == 1U)
    {
        left_shrink_not_enough = 1U;
    }
    else if (turn_active_roll_extend_only_side == 2U)
    {
        right_shrink_not_enough = 1U;
    }

    if (height_delta_cm > 0.0f)
    {
        if ((left_shrink_not_enough != 0U) ||
            (Turn_Active_Roll_Duty_Is_OverLimit(pre_turn_duty_lf + SERVO_MOTOR_PWM1_DIR * left_delta,
                                                LF_LIMIT_DUTY_MIN,
                                                LF_LIMIT_DUTY_MAX) != 0U) ||
            (Turn_Active_Roll_Duty_Is_OverLimit(pre_turn_duty_lr + SERVO_MOTOR_PWM4_DIR * left_delta,
                                                LR_LIMIT_DUTY_MIN,
                                                LR_LIMIT_DUTY_MAX) != 0U))
        {
            /* 左侧收腿触底时，左侧不再收腿，右侧加倍伸腿补足目标左右高度差。 */
            left_height_cm = servo_height;
            right_height_cm = Float_Constrain(servo_height + 2.0f * height_delta_cm, P_min, P_max);
            left_pwm = base_pwm;
            right_pwm = Turn_Active_Roll_Height_To_Pwm(right_height_cm);
            left_delta = 0;
            right_delta = right_pwm - base_pwm;
        }
    }
    else if (height_delta_cm < 0.0f)
    {
        if ((right_shrink_not_enough != 0U) ||
            (Turn_Active_Roll_Duty_Is_OverLimit(pre_turn_duty_rf + SERVO_MOTOR_PWM2_DIR * right_delta,
                                                RF_LIMIT_DUTY_MIN,
                                                RF_LIMIT_DUTY_MAX) != 0U) ||
            (Turn_Active_Roll_Duty_Is_OverLimit(pre_turn_duty_rr + SERVO_MOTOR_PWM3_DIR * right_delta,
                                                RR_LIMIT_DUTY_MIN,
                                                RR_LIMIT_DUTY_MAX) != 0U))
        {
            /* 右侧收腿触底时，右侧不再收腿，左侧加倍伸腿补足目标左右高度差。 */
            right_height_cm = servo_height;
            left_height_cm = Float_Constrain(servo_height - 2.0f * height_delta_cm, P_min, P_max);
            right_pwm = base_pwm;
            left_pwm = Turn_Active_Roll_Height_To_Pwm(left_height_cm);
            right_delta = 0;
            left_delta = left_pwm - base_pwm;
        }
    }

    g_turn_active_roll_height_delta_cm = 0.5f * (right_height_cm - left_height_cm);

    g_target_pwm_turn_roll_lf = Turn_Active_Roll_Duty_Deadband(left_delta);
    g_target_pwm_turn_roll_lr = Turn_Active_Roll_Duty_Deadband(left_delta);
    g_target_pwm_turn_roll_rf = Turn_Active_Roll_Duty_Deadband(right_delta);
    g_target_pwm_turn_roll_rr = Turn_Active_Roll_Duty_Deadband(right_delta);

    if ((g_target_pwm_turn_roll_lf == 0) && (g_target_pwm_turn_roll_rf == 0) &&
        (g_target_pwm_turn_roll_rr == 0) && (g_target_pwm_turn_roll_lr == 0))
    {
        g_turn_active_roll_height_delta_cm = 0.0f;
    }

    high_control_table(servo_height);
}

/**
 * @brief 限幅函数
 */
float Float_Constrain(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

static const ControlProfile_t *Control_Profile_GetPreset(ControlMode_e mode)
{
    if (mode == CONTROL_MODE_ACCEL)
    {
        return &g_control_profile_accel;
    }
    if (mode == CONTROL_MODE_BRAKE)
    {
        return &g_control_profile_brake;
    }
    return &g_control_profile_normal;
}

static float Control_Profile_Follow(float current, float target, float alpha, float epsilon)
{
    float next = current + (target - current) * alpha;

    if (fabsf(target - next) <= epsilon)
    {
        return target;
    }
    return next;
}

static void Control_Profile_ApplyToControllers(const ControlProfile_t *profile)
{
    pid_servo_speed.kp = profile->servo_speed_kp;
    pid_servo_speed.ki = profile->servo_speed_ki;
    pid_servo_speed.kd = profile->servo_speed_kd;
    pid_servo_speed.max_output = profile->servo_speed_max_output;
    pid_servo_speed.max_integral = profile->servo_speed_max_integral;
    pid_servo_speed.compensation = profile->servo_speed_compensation;

    pid_angle.kp = profile->angle_kp;
    pid_angle.ki = profile->angle_ki;
    pid_angle.kd = profile->angle_kd;
    pid_angle.max_output = profile->angle_max_output;
    pid_angle.max_integral = profile->angle_max_integral;
    pid_angle.compensation = profile->angle_compensation;

    pid_gyro.kp = profile->gyro_kp;
    pid_gyro.ki = profile->gyro_ki;
    pid_gyro.kd = profile->gyro_kd;
    pid_gyro.max_output = profile->gyro_max_output;
    pid_gyro.max_integral = profile->gyro_max_integral;
    pid_gyro.compensation = profile->gyro_compensation;

    pid_turn_angle.kp = profile->turn_angle_kp;
    pid_turn_angle.ki = profile->turn_angle_ki;
    pid_turn_angle.kd = profile->turn_angle_kd;
    pid_turn_angle.max_output = profile->turn_angle_max_output;
    pid_turn_angle.max_integral = profile->turn_angle_max_integral;
    pid_turn_angle.compensation = profile->turn_angle_compensation;

    pid_turn_gyro.kp = profile->turn_gyro_kp;
    pid_turn_gyro.ki = profile->turn_gyro_ki;
    pid_turn_gyro.kd = profile->turn_gyro_kd;
    pid_turn_gyro.max_output = profile->turn_gyro_max_output;
    pid_turn_gyro.max_integral = profile->turn_gyro_max_integral;
    pid_turn_gyro.compensation = profile->turn_gyro_compensation;

    pid_roll.kp = profile->roll_kp;
    pid_roll.ki = profile->roll_ki;
    pid_roll.kd = profile->roll_kd;
    pid_roll.max_output = profile->roll_max_output;
    pid_roll.max_integral = profile->roll_max_integral;
    pid_roll.compensation = profile->roll_compensation;

    servo_executor_set_profile(profile->servo_exec_acc_limit,
                               profile->servo_exec_dec_limit,
                               profile->servo_exec_boost_from_speed,
                               profile->servo_exec_boost_from_error,
                               profile->servo_exec_boost_max);
}

void Control_Profile_RequestMode(ControlMode_e mode)
{
    if (g_control_mode_requested != mode)
    {
        profile_switch_beep_request = 1U;
    }
    g_control_mode_requested = mode;
    g_control_profile_target = *Control_Profile_GetPreset(mode);
}

void Control_Profile_ApplyNow(ControlMode_e mode)
{
    const ControlProfile_t *preset = Control_Profile_GetPreset(mode);

    g_control_mode_requested = mode;
    g_control_mode_applied = mode;
    g_control_profile_target = *preset;
    g_control_profile_active = *preset;
    Control_Profile_ApplyToControllers(&g_control_profile_active);
}

void Control_Profile_Init(void)
{
    Control_Profile_ApplyNow(CONTROL_MODE_NORMAL);
}

void Control_Profile_Update1ms(void)
{
    const float pid_alpha = 0.12f;
    const float ff_alpha = 0.10f;
    const float limit_alpha = 0.18f;
    const float eps = 0.0005f;

    g_control_profile_active.servo_speed_kp = Control_Profile_Follow(g_control_profile_active.servo_speed_kp, g_control_profile_target.servo_speed_kp, pid_alpha, eps);
    g_control_profile_active.servo_speed_ki = Control_Profile_Follow(g_control_profile_active.servo_speed_ki, g_control_profile_target.servo_speed_ki, pid_alpha, eps);
    g_control_profile_active.servo_speed_kd = Control_Profile_Follow(g_control_profile_active.servo_speed_kd, g_control_profile_target.servo_speed_kd, pid_alpha, eps);
    g_control_profile_active.servo_speed_max_output = Control_Profile_Follow(g_control_profile_active.servo_speed_max_output, g_control_profile_target.servo_speed_max_output, limit_alpha, 0.5f);
    g_control_profile_active.servo_speed_max_integral = Control_Profile_Follow(g_control_profile_active.servo_speed_max_integral, g_control_profile_target.servo_speed_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.servo_speed_compensation = Control_Profile_Follow(g_control_profile_active.servo_speed_compensation, g_control_profile_target.servo_speed_compensation, pid_alpha, eps);

    g_control_profile_active.angle_kp = Control_Profile_Follow(g_control_profile_active.angle_kp, g_control_profile_target.angle_kp, pid_alpha, eps);
    g_control_profile_active.angle_ki = Control_Profile_Follow(g_control_profile_active.angle_ki, g_control_profile_target.angle_ki, pid_alpha, eps);
    g_control_profile_active.angle_kd = Control_Profile_Follow(g_control_profile_active.angle_kd, g_control_profile_target.angle_kd, pid_alpha, eps);
    g_control_profile_active.angle_max_output = Control_Profile_Follow(g_control_profile_active.angle_max_output, g_control_profile_target.angle_max_output, limit_alpha, 0.5f);
    g_control_profile_active.angle_max_integral = Control_Profile_Follow(g_control_profile_active.angle_max_integral, g_control_profile_target.angle_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.angle_compensation = Control_Profile_Follow(g_control_profile_active.angle_compensation, g_control_profile_target.angle_compensation, pid_alpha, eps);

    g_control_profile_active.gyro_kp = Control_Profile_Follow(g_control_profile_active.gyro_kp, g_control_profile_target.gyro_kp, pid_alpha, eps);
    g_control_profile_active.gyro_ki = Control_Profile_Follow(g_control_profile_active.gyro_ki, g_control_profile_target.gyro_ki, pid_alpha, eps);
    g_control_profile_active.gyro_kd = Control_Profile_Follow(g_control_profile_active.gyro_kd, g_control_profile_target.gyro_kd, pid_alpha, eps);
    g_control_profile_active.gyro_max_output = Control_Profile_Follow(g_control_profile_active.gyro_max_output, g_control_profile_target.gyro_max_output, limit_alpha, 0.5f);
    g_control_profile_active.gyro_max_integral = Control_Profile_Follow(g_control_profile_active.gyro_max_integral, g_control_profile_target.gyro_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.gyro_compensation = Control_Profile_Follow(g_control_profile_active.gyro_compensation, g_control_profile_target.gyro_compensation, pid_alpha, eps);

    g_control_profile_active.turn_angle_kp = Control_Profile_Follow(g_control_profile_active.turn_angle_kp, g_control_profile_target.turn_angle_kp, pid_alpha, eps);
    g_control_profile_active.turn_angle_ki = Control_Profile_Follow(g_control_profile_active.turn_angle_ki, g_control_profile_target.turn_angle_ki, pid_alpha, eps);
    g_control_profile_active.turn_angle_kd = Control_Profile_Follow(g_control_profile_active.turn_angle_kd, g_control_profile_target.turn_angle_kd, pid_alpha, eps);
    g_control_profile_active.turn_angle_max_output = Control_Profile_Follow(g_control_profile_active.turn_angle_max_output, g_control_profile_target.turn_angle_max_output, limit_alpha, 0.5f);
    g_control_profile_active.turn_angle_max_integral = Control_Profile_Follow(g_control_profile_active.turn_angle_max_integral, g_control_profile_target.turn_angle_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.turn_angle_compensation = Control_Profile_Follow(g_control_profile_active.turn_angle_compensation, g_control_profile_target.turn_angle_compensation, pid_alpha, eps);

    g_control_profile_active.turn_gyro_kp = Control_Profile_Follow(g_control_profile_active.turn_gyro_kp, g_control_profile_target.turn_gyro_kp, pid_alpha, eps);
    g_control_profile_active.turn_gyro_ki = Control_Profile_Follow(g_control_profile_active.turn_gyro_ki, g_control_profile_target.turn_gyro_ki, pid_alpha, eps);
    g_control_profile_active.turn_gyro_kd = Control_Profile_Follow(g_control_profile_active.turn_gyro_kd, g_control_profile_target.turn_gyro_kd, pid_alpha, eps);
    g_control_profile_active.turn_gyro_max_output = Control_Profile_Follow(g_control_profile_active.turn_gyro_max_output, g_control_profile_target.turn_gyro_max_output, limit_alpha, 0.5f);
    g_control_profile_active.turn_gyro_max_integral = Control_Profile_Follow(g_control_profile_active.turn_gyro_max_integral, g_control_profile_target.turn_gyro_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.turn_gyro_compensation = Control_Profile_Follow(g_control_profile_active.turn_gyro_compensation, g_control_profile_target.turn_gyro_compensation, pid_alpha, eps);

    g_control_profile_active.roll_kp = Control_Profile_Follow(g_control_profile_active.roll_kp, g_control_profile_target.roll_kp, pid_alpha, eps);
    g_control_profile_active.roll_ki = Control_Profile_Follow(g_control_profile_active.roll_ki, g_control_profile_target.roll_ki, pid_alpha, eps);
    g_control_profile_active.roll_kd = Control_Profile_Follow(g_control_profile_active.roll_kd, g_control_profile_target.roll_kd, pid_alpha, eps);
    g_control_profile_active.roll_max_output = Control_Profile_Follow(g_control_profile_active.roll_max_output, g_control_profile_target.roll_max_output, limit_alpha, 0.5f);
    g_control_profile_active.roll_max_integral = Control_Profile_Follow(g_control_profile_active.roll_max_integral, g_control_profile_target.roll_max_integral, limit_alpha, 0.5f);
    g_control_profile_active.roll_compensation = Control_Profile_Follow(g_control_profile_active.roll_compensation, g_control_profile_target.roll_compensation, pid_alpha, eps);

    g_control_profile_active.brake_gain_light = Control_Profile_Follow(g_control_profile_active.brake_gain_light, g_control_profile_target.brake_gain_light, ff_alpha, eps);
    g_control_profile_active.brake_gain_med = Control_Profile_Follow(g_control_profile_active.brake_gain_med, g_control_profile_target.brake_gain_med, ff_alpha, eps);
    g_control_profile_active.brake_gain_heavy = Control_Profile_Follow(g_control_profile_active.brake_gain_heavy, g_control_profile_target.brake_gain_heavy, ff_alpha, eps);
    g_control_profile_active.brake_max_light = Control_Profile_Follow(g_control_profile_active.brake_max_light, g_control_profile_target.brake_max_light, limit_alpha, 0.5f);
    g_control_profile_active.brake_max_med = Control_Profile_Follow(g_control_profile_active.brake_max_med, g_control_profile_target.brake_max_med, limit_alpha, 0.5f);
    g_control_profile_active.brake_max_heavy = Control_Profile_Follow(g_control_profile_active.brake_max_heavy, g_control_profile_target.brake_max_heavy, limit_alpha, 0.5f);
    g_control_profile_active.brake_ramp_up_light = Control_Profile_Follow(g_control_profile_active.brake_ramp_up_light, g_control_profile_target.brake_ramp_up_light, limit_alpha, 0.5f);
    g_control_profile_active.brake_ramp_up_med = Control_Profile_Follow(g_control_profile_active.brake_ramp_up_med, g_control_profile_target.brake_ramp_up_med, limit_alpha, 0.5f);
    g_control_profile_active.brake_ramp_up_heavy = Control_Profile_Follow(g_control_profile_active.brake_ramp_up_heavy, g_control_profile_target.brake_ramp_up_heavy, limit_alpha, 0.5f);
    g_control_profile_active.brake_ramp_down = Control_Profile_Follow(g_control_profile_active.brake_ramp_down, g_control_profile_target.brake_ramp_down, limit_alpha, 0.5f);

    g_control_profile_active.accel_ff_gain = Control_Profile_Follow(g_control_profile_active.accel_ff_gain, g_control_profile_target.accel_ff_gain, ff_alpha, eps);
    g_control_profile_active.accel_ff_max = Control_Profile_Follow(g_control_profile_active.accel_ff_max, g_control_profile_target.accel_ff_max, limit_alpha, 0.5f);
    g_control_profile_active.accel_ff_ramp_up = Control_Profile_Follow(g_control_profile_active.accel_ff_ramp_up, g_control_profile_target.accel_ff_ramp_up, limit_alpha, 0.5f);
    g_control_profile_active.accel_ff_ramp_down = Control_Profile_Follow(g_control_profile_active.accel_ff_ramp_down, g_control_profile_target.accel_ff_ramp_down, limit_alpha, 0.5f);

    g_control_profile_active.servo_exec_acc_limit = Control_Profile_Follow(g_control_profile_active.servo_exec_acc_limit, g_control_profile_target.servo_exec_acc_limit, limit_alpha, 0.25f);
    g_control_profile_active.servo_exec_dec_limit = Control_Profile_Follow(g_control_profile_active.servo_exec_dec_limit, g_control_profile_target.servo_exec_dec_limit, limit_alpha, 0.25f);
    g_control_profile_active.servo_exec_boost_from_speed = Control_Profile_Follow(g_control_profile_active.servo_exec_boost_from_speed, g_control_profile_target.servo_exec_boost_from_speed, ff_alpha, eps);
    g_control_profile_active.servo_exec_boost_from_error = Control_Profile_Follow(g_control_profile_active.servo_exec_boost_from_error, g_control_profile_target.servo_exec_boost_from_error, ff_alpha, eps);
    g_control_profile_active.servo_exec_boost_max = Control_Profile_Follow(g_control_profile_active.servo_exec_boost_max, g_control_profile_target.servo_exec_boost_max, limit_alpha, 0.25f);

    Control_Profile_ApplyToControllers(&g_control_profile_active);
    g_control_mode_applied = g_control_mode_requested;
}

/**
 * @brief PID 过程数据初始化
 * @note  调用此函数后，所有PID环的积分项和输出都会被重置为0。
 *        
 */
void PID_Param_Init(void) {
    Control_Profile_Init();
    // 初始化舵机速度环PID参数
    pid_servo_speed.kp = SERVO_SPEED_KP;
    pid_servo_speed.ki = SERVO_SPEED_KI;
    pid_servo_speed.kd = SERVO_SPEED_KD;
    pid_servo_speed.max_output = SERVO_SPEED_MAX_O;
    pid_servo_speed.max_integral = SERVO_SPEED_MAX_I;
    pid_servo_speed.compensation = SERVO_SPEED_COMP;
    
    // 重置舵机速度环状态变量
    pid_servo_speed.error = 0;
    pid_servo_speed.last_error = 0;
    pid_servo_speed.prev_error = 0;
    pid_servo_speed.error_integral = 0;
    pid_servo_speed.output = 0;

    // 初始化角度环PID参数
    pid_angle.kp = ANG_KP;
    pid_angle.ki = ANG_KI;
    pid_angle.kd = ANG_KD;
    pid_angle.max_output = ANG_MAX_O;
    pid_angle.max_integral = ANG_MAX_I;
    pid_angle.compensation = ANG_MECH_ZERO;
    
    // 重置角度环状态变量
    pid_angle.error = 0;
    pid_angle.last_error = 0;
    pid_angle.prev_error = 0;
    pid_angle.error_integral = 0;
    pid_angle.output = 0;
    
    // 初始化角速度环PID参数
    pid_gyro.kp = GYR_KP;
    pid_gyro.ki = GYR_KI;
    pid_gyro.kd = GYR_KD;
    pid_gyro.max_output = GYR_MAX_O;
    pid_gyro.max_integral = GYR_MAX_I;
    pid_gyro.compensation = GYR_DEAD_ZONE;
    
    // 重置角速度环状态变量
    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;

    //初始化转向角度环PID参数
    pid_turn_angle.kp = TURN_ANG_KP;
    pid_turn_angle.ki = TURN_ANG_KI;
    pid_turn_angle.kd = TURN_ANG_KD;
    pid_turn_angle.max_output = TURN_ANG_MAX_O;
    pid_turn_angle.max_integral = TURN_ANG_MAX_I;
    pid_turn_angle.compensation = TURN_ANG_DEAD_ZONE;
    
    // 重置转向角度环状态变量
    pid_turn_angle.error = 0;
    pid_turn_angle.last_error = 0;
    pid_turn_angle.prev_error = 0;
    pid_turn_angle.error_integral = 0;
    pid_turn_angle.output = 0;

    // 初始化转向角速度环PID参数
    pid_turn_gyro.kp = TURN_GYR_KP;
    pid_turn_gyro.ki = TURN_GYR_KI;
    pid_turn_gyro.kd = TURN_GYR_KD;
    pid_turn_gyro.max_output = TURN_GYR_MAX_O;
    pid_turn_gyro.max_integral = TURN_GYR_MAX_I;
    pid_turn_gyro.compensation = TURN_GYR_DEAD_ZONE;
    
    // 重置转向角速度环状态变量
    pid_turn_gyro.error = 0;
    pid_turn_gyro.last_error = 0;
    pid_turn_gyro.prev_error = 0;
    pid_turn_gyro.error_integral = 0;
    pid_turn_gyro.output = 0;

    // 初始化横滚环PID参数
    pid_roll.kp = ROLL_KP;
    pid_roll.ki = ROLL_KI;
    pid_roll.kd = ROLL_KD;
    pid_roll.max_output = ROLL_MAX_O;
    pid_roll.max_integral = ROLL_MAX_I;
    pid_roll.compensation = ROLL_MECH_ZERO;

    // 重置横滚环状态变量
    pid_roll.error = 0;
    pid_roll.last_error = 0;
    pid_roll.prev_error = 0;
    pid_roll.error_integral = 0;
    pid_roll.output = 0;

    // 重置横滚环使能位
    roll_balance_enable = ROLL_BALANCE_ENABLE_INIT;
    g_target_pwm_roll_adj = 0;
    Turn_Active_Roll_Duty_Clear();

    // 重置目标速度
    target_speed_set = 0.0f;
    Accel_Feedforward_Reset();
    Control_Profile_ApplyToControllers(&g_control_profile_active);
}

/**
 * @brief 重置pid数据
 */
void PID_Data_Reset(void) {
    // 只清运行态，不破坏当前 profile 下的参数
    pid_servo_speed.error = 0;
    pid_servo_speed.last_error = 0;
    pid_servo_speed.prev_error = 0;
    pid_servo_speed.error_integral = 0;
    pid_servo_speed.output = 0;

    pid_angle.error = 0;
    pid_angle.last_error = 0;
    pid_angle.prev_error = 0;
    pid_angle.error_integral = 0;
    pid_angle.output = 0;

    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;

    pid_turn_angle.error = 0;
    pid_turn_angle.last_error = 0;
    pid_turn_angle.prev_error = 0;
    pid_turn_angle.error_integral = 0;
    pid_turn_angle.output = 0;

    pid_turn_gyro.error = 0;
    pid_turn_gyro.last_error = 0;
    pid_turn_gyro.prev_error = 0;
    pid_turn_gyro.error_integral = 0;
    pid_turn_gyro.output = 0;

    pid_roll.error = 0;
    pid_roll.last_error = 0;
    pid_roll.prev_error = 0;
    pid_roll.error_integral = 0;
    pid_roll.output = 0;
    g_target_pwm_roll_adj = 0;
    Turn_Active_Roll_Duty_Clear();

    // 重置目标速度
    target_speed_set = 0.0f;
    Accel_Feedforward_Reset();
    Control_Profile_ApplyToControllers(&g_control_profile_active);
}

/**
 * @brief 将所有PID结构体成员变量设置为0
 */
void PID_Data_Clean_All(void) {
    // 初始化舵机速度环PID参数
    pid_servo_speed.kp = 0;
    pid_servo_speed.ki = 0;
    pid_servo_speed.kd = 0;
    pid_servo_speed.max_output = SERVO_SPEED_MAX_O;
    pid_servo_speed.max_integral = SERVO_SPEED_MAX_I;
    pid_servo_speed.compensation = SERVO_SPEED_COMP;
    
    // 重置舵机速度环状态变量
    pid_servo_speed.error = 0;
    pid_servo_speed.last_error = 0;
    pid_servo_speed.prev_error = 0;
    pid_servo_speed.error_integral = 0;
    pid_servo_speed.output = 0;

    // 初始化角度环PID参数
    pid_angle.kp = 0;
    pid_angle.ki = 0;
    pid_angle.kd = 0;
    pid_angle.max_output = ANG_MAX_O;
    pid_angle.max_integral = ANG_MAX_I;
    pid_angle.compensation = ANG_MECH_ZERO;
    
    // 重置角度环状态变量
    pid_angle.error = 0;
    pid_angle.last_error = 0;
    pid_angle.prev_error = 0;
    pid_angle.error_integral = 0;
    pid_angle.output = 0;
    
    // 初始化角速度环PID参数
    pid_gyro.kp = 0;
    pid_gyro.ki = 0;
    pid_gyro.kd = 0;
    pid_gyro.max_output = GYR_MAX_O;
    pid_gyro.max_integral = GYR_MAX_I;
    pid_gyro.compensation = GYR_DEAD_ZONE;
    
    // 重置角速度环状态变量
    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;

    //初始化转向角度环PID参数
    pid_turn_angle.kp = 0;
    pid_turn_angle.ki = 0;
    pid_turn_angle.kd = 0;
    pid_turn_angle.max_output = TURN_ANG_MAX_O;
    pid_turn_angle.max_integral = TURN_ANG_MAX_I;
    pid_turn_angle.compensation = TURN_ANG_DEAD_ZONE;
    
    // 重置转向角度环状态变量
    pid_turn_angle.error = 0;
    pid_turn_angle.last_error = 0;
    pid_turn_angle.prev_error = 0;
    pid_turn_angle.error_integral = 0;
    pid_turn_angle.output = 0;

    // 初始化转向角速度环PID参数
    pid_turn_gyro.kp = 0;
    pid_turn_gyro.ki = 0;
    pid_turn_gyro.kd = 0;
    pid_turn_gyro.max_output = TURN_GYR_MAX_O;
    pid_turn_gyro.max_integral = TURN_GYR_MAX_I;
    pid_turn_gyro.compensation = TURN_GYR_DEAD_ZONE;
    
    // 重置转向角速度环状态变量
    pid_turn_gyro.error = 0;
    pid_turn_gyro.last_error = 0;
    pid_turn_gyro.prev_error = 0;
    pid_turn_gyro.error_integral = 0;
    pid_turn_gyro.output = 0;

    //初始化横滚环PID参数
    pid_roll.kp = 0;
    pid_roll.ki = 0;
    pid_roll.kd = 0;
    pid_roll.max_output = ROLL_MAX_O;
    pid_roll.max_integral = ROLL_MAX_I;
    pid_roll.compensation = ROLL_MECH_ZERO;

    // 重置横滚环状态变量
    pid_roll.error = 0;
    pid_roll.last_error = 0;
    pid_roll.prev_error = 0;
    pid_roll.error_integral = 0;
    pid_roll.output = 0;
    g_target_pwm_roll_adj = 0;
    Turn_Active_Roll_Duty_Clear();

    // 重置目标速度
    target_speed_set = 0.0f;
    Accel_Feedforward_Reset();
}

// ============================================================================
//  控制函数实现
// ============================================================================

/**
 * @brief 转向角度环控制器（外环）- 完整PID参数实现
 * @param angle_error 角度误差（期望转向角 - 实际转向角，单位：度）
 *                    由视觉系统或编码器差分计算得出
 * @return 期望角速度指令（单位：°/s），作为转向角速度环的输入
 * 
 * 【完整参数应用】
 * - Kp：比例增益，将角度误差映射为角速度指令的基础刚度
 * - Ki：积分增益（默认0），保留接口但禁用（避免转向累积误差）
 * - Kd：微分增益，抑制转向过程中的超调和振荡
 * - max_integral：积分限幅（因Ki=0，实际无效）
 * - compensation：补偿值（角度环通常为0，保留结构统一性）
 * - max_output：输出限幅，防止角度环输出过大导致内环饱和
 * 
 * 【场景自适应】
 * 根据赛道元素动态调整控制增益（单边桥降低灵敏度防跌落）
 */
float Turn_Angle_Loop_Control(float angle_error)
{
    // 1. 误差赋值（注意：angle_error 已是 (期望-实际) 的差值）
    pid_turn_angle.error = angle_error;

    // 2. 积分项计算（保留完整结构，但因Ki=0实际无效）
    if (pid_turn_angle.ki != 0.0f) {
        pid_turn_angle.error_integral += pid_turn_angle.error;
        // 积分限幅保护
        pid_turn_angle.error_integral = Float_Constrain(
            pid_turn_angle.error_integral, 
            -pid_turn_angle.max_integral, 
            pid_turn_angle.max_integral
        );
    } else {
        pid_turn_angle.error_integral = 0.0f; // 显式清零确保无累积
    }

    // 3. 场景自适应增益调度（单边桥特殊处理）
    float kp_adj = pid_turn_angle.kp;
    float kd_adj = pid_turn_angle.kd;
    
    // if (danbianqiao_flag && danbianqiao_flag != 99) {
    //     kp_adj *= 0.7f;  // 单边桥降低Kp 30% 防跌落
    //     kd_adj *= 0.7f;  // 同比例缩放保持阻尼比
    // }
    // 可扩展：三级跳台阶，草地等场景的增益调整

    // 4. 完整PID计算（实际为PD，因Ki=0）
    float output_raw = (kp_adj * pid_turn_angle.error) + 
                       (pid_turn_angle.ki * pid_turn_angle.error_integral) + 
                       (kd_adj * (pid_turn_angle.error - pid_turn_angle.last_error));

    // 5. 输出限幅（防止角度环输出过大导致内环饱和）
    pid_turn_angle.output = Float_Constrain(
        output_raw, 
        -pid_turn_angle.max_output, 
        pid_turn_angle.max_output
    );

    // 6. 更新历史误差（为下一次微分计算准备）
    pid_turn_angle.prev_error = pid_turn_angle.last_error;
    pid_turn_angle.last_error = pid_turn_angle.error;

    return pid_turn_angle.output;  // 作为转向角速度环的目标值
}

// ============================================================================
//  转向角速度环控制函数 (内环 - 2ms周期，当前Core0调度1ms) - 完整PID+死区补偿
// ============================================================================
/**
 * @brief 转向角速度环控制器（内环）- 完整PID+死区补偿实现
 * @param target_gyro 期望角速度（来自转向角度环，单位：°/s）
 * @param actual_gyro 实际角速度（来自IMU陀螺仪Z轴，单位：°/s）
 * @return 转向专用PWM值（直接叠加到电机驱动）
 * 
 * 【完整参数应用】
 * - Kp：比例增益，决定角速度跟踪的响应速度
 * - Ki：积分增益（默认0），高频环路禁用积分
 * - Kd：微分增益，抑制高频抖动和电机噪声
 * - max_integral：积分限幅（因Ki=0，实际无效）
 * - compensation：死区补偿电压（关键！克服转向电机静摩擦）
 * - max_output：动态输出限幅（普通赛道/单边桥双阈值）
 * 
 * 【传感器说明】
 * - 陀螺仪Z轴（gyro_z）对应偏航角速度（yaw rate），即车体旋转速度
 * - 符号约定：需根据实际安装方向调整（示例中使用负号匹配物理方向）
 */
float Turn_Gyro_Loop_Control(float target_gyro, float actual_gyro)
{
    // 1. 计算角速度误差
    pid_turn_gyro.error = target_gyro - actual_gyro;

    // 2. 积分项计算（保留完整结构，但因Ki=0实际无效）
    if (pid_turn_gyro.ki != 0.0f) {
        pid_turn_gyro.error_integral += pid_turn_gyro.error;
        // 积分限幅保护
        pid_turn_gyro.error_integral = Float_Constrain(
            pid_turn_gyro.error_integral, 
            -pid_turn_gyro.max_integral, 
            pid_turn_gyro.max_integral
        );
    } else {
        pid_turn_gyro.error_integral = 0.0f; // 显式清零确保无累积
    }

    // 3. 完整PD计算（实际为PD，因Ki=0）
    float output_raw = (pid_turn_gyro.kp * pid_turn_gyro.error) + 
                       (pid_turn_gyro.ki * pid_turn_gyro.error_integral) + 
                       (pid_turn_gyro.kd * (pid_turn_gyro.error - pid_turn_gyro.last_error));

    // 4. 死区补偿（关键！克服转向电机静摩擦）
    // 原理：当输出意图非零时，叠加最小启动电压使电机立即响应
    if (output_raw > 0) {
        output_raw += pid_turn_gyro.compensation; // 正转加死区
    } else if (output_raw < 0) {
        output_raw -= pid_turn_gyro.compensation; // 反转减死区
    }
    // 注意：output_raw=0时不做补偿，避免零点漂移

    // 5. 动态输出限幅（根据赛道类型切换阈值）
    // float max_output = danbianqiao_flag ? TURN_GYR_MAX_O_BRIDGE : pid_turn_gyro.max_output;//单边桥情形下的示例
    pid_turn_gyro.output = Float_Constrain(output_raw, -pid_turn_gyro.max_output, pid_turn_gyro.max_output);

    // 6. 更新历史误差（为下一次微分计算准备）
    pid_turn_gyro.prev_error = pid_turn_gyro.last_error;
    pid_turn_gyro.last_error = pid_turn_gyro.error;

    return pid_turn_gyro.output;
}



//内部静态变量，用于舵机速度环的滤波
static float servo_speed_last = 0.0f;
static float servo_speed_prelast = 0.0f;
/**
 * @brief 舵机速度闭环控制器 (移植并使用 PID_Param_t 结构)
 * @param target_speed 目标速度
 * @param actual_speed 实际速度 (来自编码器)
 * @param actual_angle 当前姿态角度 (来自IMU)
 * @return 姿态调整量 (例如，需要前倾/后仰的角度)
 */
float Servo_Speed_Control(float target_speed, float actual_speed, float actual_angle)
{
    // 1. 输入滤波
    float speed_now = actual_speed * 0.6f + servo_speed_last * 0.3f + servo_speed_prelast * 0.1f;
    servo_speed_prelast = servo_speed_last;
    servo_speed_last = speed_now;
    // 2.这部分预留的速度规划已经从pid中移除，其他模块直接使用target_speed即可

    // 3. 计算误差
    pid_servo_speed.error = target_speed - speed_now;

    // 4. 自适应 Kp
    float k, adaptive_kp, kp_boost;
    float e = expf(-fabsf(pid_servo_speed.error / 10.0f)); // 分母越小，速度误差变化对 Kp 权重越敏感
    k = ((1.0f - e) / (1.0f + e)) * 0.6f + 0.4f; // 基础自适应倍率限制在 [0.4, 1.0]
    kp_boost = Accel_Feedforward_GetKpBoost(); // 仅 Kp 增强模式触发加速请求时会大于 1.0
    adaptive_kp = pid_servo_speed.kp * k * kp_boost; // 最终 Kp = 基础 Kp * 自适应倍率 * 加速前馈增强倍率

    // 5. 位置式 PID 计算
    // 积分项 & 积分限幅
    // 只有在接近机械零点的情况下，才使用积分项，修复起来的时候开始积分的问题
    // if (fabsf(actual_angle-ANG_MECH_ZERO) < 2.0f) {
        //pid_servo_speed.error_integral += pid_servo_speed.error;
    // }
    // else{
    //     pid_servo_speed.error_integral = 0.0f;
    // }
    if (fabsf(pid_servo_speed.error) < 1000.0f) {
        pid_servo_speed.error_integral += pid_servo_speed.error;
    } else {
        pid_servo_speed.error_integral = 0.0f;
    }
    pid_servo_speed.error_integral = Float_Constrain(pid_servo_speed.error_integral, -pid_servo_speed.max_integral, pid_servo_speed.max_integral);

    // PID输出计算
    float output_raw = (adaptive_kp * pid_servo_speed.error) +
                       (pid_servo_speed.ki * pid_servo_speed.error_integral) +
                       (pid_servo_speed.kd * (pid_servo_speed.error - pid_servo_speed.last_error));

    // 6. 输出限幅与更新
    {
        float output_limit = pid_servo_speed.max_output;
#if ACCEL_FF_ENABLE && (ACCEL_FF_MODE == ACCEL_FF_MODE_KP)
        if (kp_boost > 1.0f)
        {
            output_limit *= kp_boost;
            if (output_limit > ACCEL_KP_OUTPUT_MAX)
            {
                output_limit = ACCEL_KP_OUTPUT_MAX;
            }
            if (output_limit < pid_servo_speed.max_output)
            {
                output_limit = pid_servo_speed.max_output;
            }
        }
#endif
        pid_servo_speed.output = Float_Constrain(output_raw, -output_limit, output_limit);
    }
    
    // 更新历史误差 (prev_error 也更新，保持结构完整性)
    pid_servo_speed.prev_error = pid_servo_speed.last_error;
    pid_servo_speed.last_error = pid_servo_speed.error;

    return pid_servo_speed.output;
}

/**
 * @brief 角度环控制 (中环)
 * @param speed_loop_output 速度环计算出的角度调整量
 * @param actual_angle      当前IMU测量的实际角度
 * @return 期望的角速度 (单位：度/秒 或 LSB)
 * @note   这是维持平衡的核心。
 */
float Angle_Loop_Control(float speed_loop_output, float actual_angle)
{
    // 1. 确定目标角度
    // 目标角度 = 机械零点(平衡点) - 速度环调节量
    // 如果速度环输出正值(想加速)，通常需要车前倾。
    // 假设前倾是负角度，那么 Target = Zero - Positive，目标变小(变负)，车会前倾。
    // (注意：这里的正负号取决于你的IMU安装方向，可能需要改为 + )
    float target_angle = pid_angle.compensation - speed_loop_output; 

    // 2. 计算误差
    pid_angle.error = target_angle - actual_angle;

    // 3. 积分计算 (直立环一般 ki=0)
    if(pid_angle.ki != 0) {
        pid_angle.error_integral += pid_angle.error;
        pid_angle.error_integral = Float_Constrain(pid_angle.error_integral, -pid_angle.max_integral, pid_angle.max_integral);
    }

    // 4. PD计算 (直立环核心)
    // P项：回复力，偏差越大，回复力越大。
    // D项：阻尼力，偏差变化越快，反向阻力越大，防止超调震荡。
    pid_angle.output = (pid_angle.kp * pid_angle.error) + 
                       (pid_angle.ki * pid_angle.error_integral) + 
                       (pid_angle.kd * (pid_angle.error - pid_angle.last_error));

    // 5. 输出限幅
    // 限制期望的最大角速度
    pid_angle.output = Float_Constrain(pid_angle.output, -pid_angle.max_output, pid_angle.max_output);
    
    // 6. 更新历史误差链
    //pid_angle.prev_error = pid_angle.last_error;//预留给增量式pid，现在注释掉,想用的时候可以加上
    pid_angle.last_error = pid_angle.error;

    // 返回负值通常是为了匹配电机控制方向，需根据实际情况调整
    return -pid_angle.output; 
}

/**
 * @brief 角速度环控制 (内环)
 * @param angle_loop_output 角度环计算出的期望角速度
 * @param actual_gyro       当前IMU测量的实际角速度
 * @return 最终电机 PWM 值
 * @note   这一环频率最高(1ms)，直接反应给电机电压。
 */
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro)
{
    // 0. 传感器校准
    // 减去静态零偏，保证静止时数据为0
    float real_gyro = actual_gyro - GYRO_SENSOR_OFFSET; 

    // 1. 计算误差
    pid_gyro.error = angle_loop_output - real_gyro;

    // 2. PD计算
    // 角速度环 D项能极好地抑制高频抖动
    pid_gyro.output = (pid_gyro.kp * pid_gyro.error) + 
                      (pid_gyro.kd * (pid_gyro.error - pid_gyro.last_error));

    // 3. 死区补偿 (关键优化)
    // 电机有静摩擦力。如果计算出的 PWM 很小(如50)，电机不动，控制就失效了。
    // 所以只要有输出意图，就额外叠加一个起步电压(compensation)，让电机立即响应。
    if (pid_gyro.output > 0) {
        pid_gyro.output += pid_gyro.compensation; // 正转加死区
    } else if (pid_gyro.output < 0) {
        pid_gyro.output -= pid_gyro.compensation; // 反转减死区
    }

    // 4. 输出限幅
    // 限制在定时器允许的 PWM 范围内
    pid_gyro.output = Float_Constrain(pid_gyro.output, -pid_gyro.max_output, pid_gyro.max_output);

    // 5. 更新历史误差链
    //pid_gyro.prev_error = pid_gyro.last_error;//预留给增量式pid，现在注释掉,想用的时候可以加上
    pid_gyro.last_error = pid_gyro.error;

    return pid_gyro.output;
}

/**
 * @brief Rolling 自适应平衡控制，主要用于单边桥
 * @param actual_roll 当前横滚角 (单位: 度, 右高左低为正)
 * @return float 计算出的单侧缩短量 (PWM值, 总是 >= 0)
 * @note 此函数应在 5ms 定时器中调用
 */
float Roll_Balance_Control(float actual_roll,float target_roll)
{
    // 0. 安全检查
    if (roll_balance_enable == 0U) {
        g_target_pwm_roll_adj = 0; // 这里的含义稍后解释
        Turn_Active_Roll_Duty_Clear();
        return 0.0f;
    }

    // 1. 计算误差 (目标 - 实际)
    // 目标是 0 度
    float error = target_roll - actual_roll;

    // 2. 计算 PD 输出 (标准 PID 公式)
    // 注意：这里计算的是一个“总矫正力”，正负代表方向
    float p_out = pid_roll.kp * error;
    float d_out = pid_roll.kd * (error - pid_roll.last_error);
    
    pid_roll.last_error = error; // 更新历史误差
    
    float total_out = p_out + d_out;
    
    // 限幅
    total_out = Float_Constrain(total_out, -pid_roll.max_output, pid_roll.max_output);

    // 普通转向主动侧倾已经通过查表差动给左右腿前馈高度差。
    // 此时 Rolling 环只保留小幅反馈修正，避免和前馈动作互相抢腿导致抖动。
    if ((g_target_pwm_turn_roll_lf != 0) || (g_target_pwm_turn_roll_rf != 0) ||
        (g_target_pwm_turn_roll_rr != 0) || (g_target_pwm_turn_roll_lr != 0))
    {
        total_out *= TURN_ACTIVE_ROLL_FB_KEEP_RATIO;
        total_out = Float_Constrain(total_out,
                                    -TURN_ACTIVE_ROLL_FB_MAX_PWM,
                                    TURN_ACTIVE_ROLL_FB_MAX_PWM);
    }
    
    // 3. 将总输出转换为 "一边不动，一边缩短" 的逻辑
    // total_out 的物理含义：
    // 如果 roll > 0 (右高)，error < 0，total_out < 0。我们需要缩短右腿。
    // 如果 roll < 0 (左高)，error > 0，total_out > 0。我们需要缩短左腿。
    
    // 我们约定 g_target_pwm_roll_adj 的含义：
    // 这个变量不再直接加减，而是作为一个“带符号的缩短量”传递给 servo_executor。
    // > 0 : 表示左侧需要缩短 (值越大缩得越多)
    // < 0 : 表示右侧需要缩短 (绝对值越大缩得越多)
    // = 0 : 大家都不动
    
    g_target_pwm_roll_adj = (int16)total_out;
    
    return total_out;
}
