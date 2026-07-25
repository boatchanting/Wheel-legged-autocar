#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../calculate/pid-new.h"

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_POINT_SPEED_PLANNING)

extern volatile float target_speed_set;
extern volatile float err_degree;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;
volatile uint16 g_nav_point_spin_debug_idx = 0U;
volatile float g_nav_point_spin_debug_current_yaw = 0.0f;
volatile float g_nav_point_spin_debug_exit_yaw = 0.0f;
volatile float g_nav_point_spin_debug_total_angle = 0.0f;
volatile float g_nav_point_spin_debug_direction = 0.0f;
volatile float g_nav_point_spin_debug_cw_total_angle = 0.0f;
volatile float g_nav_point_spin_debug_ccw_total_angle = 0.0f;
volatile uint16 g_nav_point_special_debug_target_idx = 0U;
volatile float g_nav_point_special_debug_target_x = 0.0f;
volatile float g_nav_point_special_debug_target_y = 0.0f;
volatile float g_nav_point_special_debug_dist_mm = 0.0f;
volatile float g_nav_point_special_debug_brake_radius_mm = 0.0f;
volatile float g_nav_point_special_debug_speed_ref_mm_s = 0.0f;
volatile uint8 g_nav_point_special_debug_zero_brake_issued = 0U;
volatile uint8 g_nav_point_special_debug_zero_brake_active = 0U;

static uint16 g_target_idx = 0U;
static uint8 g_start_heading_aligned = 1U;
static uint8 s_special_execute_circle_entered = 0U;
static uint8 s_special_zero_brake_issued = 0U;
static uint8 s_special_zero_brake_active = 0U;
static uint8 s_special_crawl_active = 0U;
static uint8 s_special_prep_zero_brake_latched = 0U;
static uint8 s_special_reverse_recover_active = 0U;
static float s_special_capture_speed_ref_mag = 0.0f;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_spin_exit_pending = 0U;
static uint8 s_spin_exit_align_ticks = 0U;
static uint8 s_stop_predict_has_prev = 0U;
static uint8 s_stop_predict_decel_observed = 0U;
static float s_stop_predict_prev_speed_mag = 0.0f;
static float s_stop_predict_prev_speed_sign = 0.0f;
static float s_stop_predict_decel_mm_s2 = NAV_POINT_SPEED_DECEL_CMD2_PER_MM;

#define NAV_POINT_SPIN_DIR_CW_SIGN             (1.0f)
#define NAV_POINT_SPIN_DIR_CCW_SIGN            (-1.0f)
#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static float NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float CalcDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

static float CalcBearingDeg(float x1, float y1, float x2, float y2)
{
    return -atan2f(y2 - y1, -(x2 - x1)) * 57.29578f;
}

static void ResetStopState(void)
{
    s_special_execute_circle_entered = 0U;
    s_special_zero_brake_issued = 0U;
    s_special_zero_brake_active = 0U;
    s_special_crawl_active = 0U;
    s_special_prep_zero_brake_latched = 0U;
    s_special_reverse_recover_active = 0U;
}

uint8 NavReplay_SpecialPointZeroBrakeActive(void)
{
    return s_special_zero_brake_active;
}

uint8 NavReplay_SpecialPointCrawlActive(void)
{
    return s_special_crawl_active;
}

uint8 NavReplay_SpecialPointPrepZeroBrakeLatched(void)
{
    return s_special_prep_zero_brake_latched;
}

static void ResetSpinExitState(void)
{
    s_spin_exit_pending = 0U;
    s_spin_exit_align_ticks = 0U;
}

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) || (point_type == NAV_POINT_JUMP));
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

static float PositiveAngle360(float angle);
static float CalcSpinTotalAngle(float current_yaw, float exit_yaw, float spin_sign);
static void SelectSpecialForwardHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign);
static void SelectSpecialPointHeading(float point_yaw_deg, float dist_to_point, float abs_vehicle_speed, float *selected_err_deg, float *speed_sign);
static float PlanDistanceSpeedAbs(float dist_mm, float stop_radius_mm);
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg);
static float PlanSpeedAbsAfterSpinExit(float dist_mm, float stop_radius_mm, float yaw_err_deg);
static float GetApproachSpeedMag(float speed_sign);
static float UpdateSpecialCaptureSpeedRef(float approach_speed_mag, float speed_sign);
static void ResetEncoderStopPrediction(void);
static void UpdateEncoderStopPrediction(float approach_speed_mag, float speed_sign);
static float ApplyEncoderStopPrediction(float speed_mag, float dist_mm, float stop_radius_mm, float speed_sign);
static float CalcSpecialBrakeDecel(void);
static float CalcSpecialBrakeRadius(float approach_speed_mag);
static uint8 ShouldStartSpecialPointCapture(float dist_to_point, float brake_radius_mm);
static float CalcSpecialCrawlReleaseDistance(float abs_vehicle_speed);
static float PlanSpecialApproachSpeed(float dist_to_point, float speed_sign, float abs_vehicle_speed);
static uint8 ShouldTriggerSpecialAction(float dist_to_point, float speed_mag);
static uint8 ShouldFinishAtLastPoint(uint16 point_idx, float dist_to_point);
static float PlanFinalPassSpeedAbs(float yaw_err_deg);

static float CalcSpecialBrakeDecel(void)
{
    float decel = NAV_POINT_SPECIAL_BRAKE_DECEL_MM_S2;

    if ((s_stop_predict_decel_observed != 0U) &&
        (s_stop_predict_decel_mm_s2 < decel))
    {
        decel = s_stop_predict_decel_mm_s2;
    }

    return Float_Constrain(decel,
                           NAV_POINT_STOP_PREDICT_DECEL_MIN,
                           NAV_POINT_STOP_PREDICT_DECEL_MAX);
}

static float CalcSpecialBrakeRadius(float approach_speed_mag)
{
    float speed_mag = fabsf(approach_speed_mag);
    float brake_pwm_abs = fabsf(Brake_Feedforward_GetPwm());
    float decel = CalcSpecialBrakeDecel();
    float stop_dist = (speed_mag * speed_mag) / (2.0f * decel);
    float brake_radius = NAV_POINT_SPECIAL_EXECUTE_RADIUS +
                         NAV_POINT_SPECIAL_BRAKE_MARGIN_MM +
                         stop_dist;

    brake_radius = Float_Constrain(brake_radius,
                                   NAV_POINT_SPECIAL_BRAKE_RADIUS_MIN,
                                   NAV_POINT_SPECIAL_BRAKE_RADIUS_MAX);

    if (brake_pwm_abs < NAV_POINT_SPECIAL_BRAKE_READY_PWM)
    {
        brake_radius += NAV_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN;
    }

    return Float_Constrain(brake_radius,
                           NAV_POINT_SPECIAL_BRAKE_RADIUS_MIN,
                           NAV_POINT_SPECIAL_BRAKE_RADIUS_MAX);
}

static uint8 ShouldStartSpecialPointCapture(float dist_to_point, float brake_radius_mm)
{
    return (uint8)(dist_to_point <= brake_radius_mm);
}

static float CalcSpecialCrawlReleaseDistance(float abs_vehicle_speed)
{
    float decel = CalcSpecialBrakeDecel();
    float stop_distance = (abs_vehicle_speed * abs_vehicle_speed) / (2.0f * decel);

    return stop_distance + NAV_POINT_SPECIAL_CRAWL_RELEASE_MARGIN_MM;
}

static float PlanSpecialApproachSpeed(float dist_to_point, float speed_sign, float abs_vehicle_speed)
{
    float step_mag = fabsf(NAV_POINT_SPECIAL_STEP_IN_SPEED);
    float remain_to_execute_circle = dist_to_point - NAV_POINT_SPECIAL_EXECUTE_RADIUS;

    if (s_special_crawl_active != 0U)
    {
        if ((dist_to_point <= NAV_POINT_SPECIAL_PREP_STOP_RADIUS) &&
            (s_special_prep_zero_brake_latched == 0U))
        {
            s_special_prep_zero_brake_latched = 1U;
            s_special_crawl_active = 0U;
            s_special_zero_brake_active = 1U;
            return NAV_POINT_SPEED_STOP;
        }

        if (dist_to_point > NAV_POINT_SPECIAL_EXECUTE_RADIUS)
        {
            return speed_sign * step_mag;
        }
    }

    if ((dist_to_point <= NAV_POINT_SPECIAL_PREP_STOP_RADIUS) &&
        (s_special_prep_zero_brake_latched == 0U))
    {
        s_special_prep_zero_brake_latched = 1U;
        s_special_zero_brake_active = 1U;
    }

    if ((s_special_zero_brake_active != 0U) &&
        (abs_vehicle_speed <= NAV_POINT_SPECIAL_STEP_IN_START_SPEED_MM_S) &&
        (remain_to_execute_circle > CalcSpecialCrawlReleaseDistance(abs_vehicle_speed)))
    {
        s_special_zero_brake_active = 0U;
        s_special_crawl_active = 1U;
        return speed_sign * step_mag;
    }

    return NAV_POINT_SPEED_STOP;
}

static uint8 ShouldTriggerSpecialAction(float dist_to_point, float speed_mag)
{
    return (uint8)((s_special_execute_circle_entered != 0U) &&
                   (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS) &&
                   (speed_mag <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S));
}

/**
 * @brief 目标速度分段限斜率 + 多预设PID模式切换
 * @param raw_speed 本周期导航层计算出的原始目标速度，符号方向保持不变
 * @return 经过单周期步长限制后的目标速度
 * @note 根据实际车速与目标速度的关系，自动切换 NORMAL/ACCEL/BRAKE 三档PID预设，
 *       同时对速度目标施加分段斜率限制，避免阶跃导致速度环超调。
 *       0724修复：加速段也走斜率限制，不再直接放行。
 */
static float NavReplay_SpeedSlew_Update(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_cmd);
    float abs_actual = fabsf(current_actual_speed);
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit;

    ControlMode_e target_mode = CONTROL_MODE_NORMAL;
    static ControlMode_e s_current_req_mode = CONTROL_MODE_NORMAL;
    static uint16 s_mode_cooldown = 0;

    // --- 1. 基于实际车速决定目标 PID 模式 ---
    float actual_speed_clamped = (abs_actual < 50.0f) ? 0.0f : current_actual_speed;
    if ((raw_speed * actual_speed_clamped) < 0.0f)
    {
        target_mode = CONTROL_MODE_BRAKE;
    }
    else if (abs_raw > (abs_actual + NAV_SPEED_SLEW_EPS))
    {
        target_mode = CONTROL_MODE_ACCEL;
    }
    else if ((abs_raw + NAV_SPEED_SLEW_EPS) < abs_actual)
    {
        target_mode = CONTROL_MODE_BRAKE;
    }
    else
    {
        target_mode = CONTROL_MODE_NORMAL;
    }

    // --- 2. 带有紧急豁免的 PID 切换冷却机制 ---
    if (target_mode == CONTROL_MODE_BRAKE && s_current_req_mode != CONTROL_MODE_BRAKE)
    {
        // 紧急情况：需要刹车，无视冷却立即切换
        s_current_req_mode = CONTROL_MODE_BRAKE;
        Control_Profile_RequestMode(CONTROL_MODE_BRAKE);
        s_mode_cooldown = 30; // 切换后进入 300ms 冷却
    }
    else if (target_mode != s_current_req_mode && s_mode_cooldown == 0)
    {
        // 正常切换：冷却完毕允许切换
        s_current_req_mode = target_mode;
        Control_Profile_RequestMode(target_mode);
        s_mode_cooldown = 30; // 重置 300ms 冷却
    }
    else if (s_mode_cooldown > 0)
    {
        s_mode_cooldown--;
        Control_Profile_RequestMode(s_current_req_mode); // 维持冷却中的状态
    }
    else
    {
        Control_Profile_RequestMode(target_mode); // 平稳保持
    }

    // --- 3. 速度曲线斜率生成 (0724修复：加速段也走斜率限制) ---
    if ((raw_speed * s_prev_speed_cmd) < 0.0f)
    {
        step_limit = NAV_SPEED_SLEW_DOWN_CROSS_ZERO;
    }
    else if (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS))
    {
        step_limit = (abs_prev < NAV_SPEED_SLEW_LOW_SPEED_TH) ? NAV_SPEED_SLEW_UP_LOW : NAV_SPEED_SLEW_UP_NORMAL;
    }
    else if ((abs_raw + NAV_SPEED_SLEW_EPS) < abs_prev)
    {
        step_limit = (abs_prev > NAV_SPEED_SLEW_FAST_DECEL_TH) ? NAV_SPEED_SLEW_DOWN_FAST : NAV_SPEED_SLEW_DOWN_NORMAL;
    }
    else
    {
        step_limit = NAV_SPEED_SLEW_UP_NORMAL;
    }

    s_prev_speed_cmd += Float_Constrain(diff, -step_limit, step_limit);
    return s_prev_speed_cmd;
}

static float PositiveAngle360(float angle)
{
    angle = NormalizeAngle(angle);
    if (angle < 0.0f)
    {
        angle += 360.0f;
    }
    return angle;
}

static float CalcSpinTotalAngle(float current_yaw, float exit_yaw, float spin_sign)
{
    float delta;
    float total_angle;

    if (spin_sign == NAV_POINT_SPIN_DIR_CW_SIGN)
    {
        delta = PositiveAngle360(current_yaw - exit_yaw);
    }
    else
    {
        delta = PositiveAngle360(exit_yaw - current_yaw);
    }

    total_angle = MINEFIELD_SPIN_BASE_CIRCLE_ANGLE + delta;
    if (total_angle < MINEFIELD_SPIN_MIN_TOTAL_ANGLE)
    {
        total_angle = MINEFIELD_SPIN_MIN_TOTAL_ANGLE;
    }

    return total_angle;
}

// 从触发瞬间的当前车头角出发，分别计算车头/车尾朝向下一个目标点的总旋转角度，选更快的一组。
static void ConfigureSpinPlanForPoint(uint16 point_idx)
{
    uint16 next_idx = (uint16)(point_idx + 1U);
    float current_yaw = inertial_nav.relative_yaw;
    float exit_forward_yaw;
    float exit_reverse_yaw;
    float best_total_angle = 1000000.0f;
    float best_exit_yaw = current_yaw;
    float best_spin_sign = NAV_POINT_SPIN_DIR_CW_SIGN;
    float best_cw_total_angle = 0.0f;
    float best_ccw_total_angle = 0.0f;
    uint8 exit_candidate_count = (NAV_PLAN2_ALLOW_REVERSE_TO_NEXT_POINT != 0) ? 2U : 1U;
    uint8 exit_candidate_idx;

    if (next_idx < nav_ram_data.point_count)
    {
        exit_forward_yaw = CalcBearingDeg(nav_ram_data.points[point_idx].x,
                                          nav_ram_data.points[point_idx].y,
                                          nav_ram_data.points[next_idx].x,
                                          nav_ram_data.points[next_idx].y);
    }
    else
    {
        exit_forward_yaw = nav_ram_data.points[point_idx].target_yaw_deg;
    }

    exit_reverse_yaw = NormalizeAngle(exit_forward_yaw + 180.0f);

    for (exit_candidate_idx = 0U; exit_candidate_idx < exit_candidate_count; exit_candidate_idx++)
    {
        float exit_yaw = (exit_candidate_idx == 0U) ? exit_forward_yaw : exit_reverse_yaw;
        float cw_total_angle = CalcSpinTotalAngle(current_yaw,
                                                  exit_yaw,
                                                  NAV_POINT_SPIN_DIR_CW_SIGN);
        float ccw_total_angle = CalcSpinTotalAngle(current_yaw,
                                                   exit_yaw,
                                                   NAV_POINT_SPIN_DIR_CCW_SIGN);

        if (cw_total_angle < best_total_angle)
        {
            best_total_angle = cw_total_angle;
            best_exit_yaw = exit_yaw;
            best_spin_sign = NAV_POINT_SPIN_DIR_CW_SIGN;
            best_cw_total_angle = cw_total_angle;
            best_ccw_total_angle = ccw_total_angle;
        }

        if (ccw_total_angle < best_total_angle)
        {
            best_total_angle = ccw_total_angle;
            best_exit_yaw = exit_yaw;
            best_spin_sign = NAV_POINT_SPIN_DIR_CCW_SIGN;
            best_cw_total_angle = cw_total_angle;
            best_ccw_total_angle = ccw_total_angle;
        }
    }

    g_nav_point_spin_debug_idx = point_idx;
    g_nav_point_spin_debug_current_yaw = current_yaw;
    g_nav_point_spin_debug_exit_yaw = best_exit_yaw;
    g_nav_point_spin_debug_total_angle = best_total_angle;
    g_nav_point_spin_debug_direction = best_spin_sign;
    g_nav_point_spin_debug_cw_total_angle = best_cw_total_angle;
    g_nav_point_spin_debug_ccw_total_angle = best_ccw_total_angle;

    Minefield_SetSpinPlan(best_total_angle, best_exit_yaw, best_spin_sign);
}

// 在“正向朝向目标点”和“反向朝向目标点”之间自动选择转向误差更小的一侧。
static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

#if NAV_PLAN2_ALLOW_REVERSE_TO_NEXT_POINT
    // 允许倒车时，自动比较车头/车尾朝向目标点所需的转角，选更快的一侧。
    if ((fabsf(err_reverse) + NAV_POINT_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
    {
        *selected_err_deg = err_reverse;
        *speed_sign = 1.0f;
    }
    else
#endif
    {
        *selected_err_deg = err_forward;
        *speed_sign = -1.0f;
    }
}

static void SelectSpecialForwardHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    *selected_err_deg = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    *speed_sign = -1.0f;
}

static void SelectSpecialPointHeading(float point_yaw_deg,
                                      float dist_to_point,
                                      float abs_vehicle_speed,
                                      float *selected_err_deg,
                                      float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);
    uint8 reverse_recover_ready;

    *selected_err_deg = err_forward;
    *speed_sign = -1.0f;

    reverse_recover_ready =
        (uint8)((s_special_zero_brake_issued != 0U) &&
                (dist_to_point > NAV_POINT_SPECIAL_EXECUTE_RADIUS) &&
                (abs_vehicle_speed <= NAV_POINT_SPECIAL_STEP_IN_START_SPEED_MM_S) &&
                (fabsf(err_forward) >= NAV_POINT_SPECIAL_REVERSE_RECOVER_YAW_MIN) &&
                ((fabsf(err_reverse) + NAV_POINT_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward)));

    if ((s_special_reverse_recover_active != 0U) ||
        (reverse_recover_ready != 0U))
    {
        s_special_reverse_recover_active = 1U;
        *selected_err_deg = err_reverse;
        *speed_sign = 1.0f;
    }
}

// 按“离停车边界还剩多少距离”实时规划允许速度上限。
static float PlanDistanceSpeedAbs(float dist_mm, float stop_radius_mm)
{
    float remain = dist_mm - stop_radius_mm;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    speed_abs = sqrtf(2.0f * NAV_POINT_SPEED_DECEL_CMD2_PER_MM * remain);
    speed_abs = Float_Constrain(speed_abs, 0.0f, fabsf(NAV_POINT_SPEED_FAST));

    if ((speed_abs < fabsf(NAV_POINT_SPEED_SLOW)) && (remain > NAV_POINT_PATH_ARRIVE_RADIUS))
    {
        speed_abs = fabsf(NAV_POINT_SPEED_SLOW);
    }

    return speed_abs;
}

static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float speed_abs = PlanDistanceSpeedAbs(dist_mm, stop_radius_mm);

    if (fabsf(yaw_err_deg) > NAV_POINT_YAW_SLOW_TOLERANCE)
    {
        speed_abs = 0.0f;
    }
    else if (fabsf(yaw_err_deg) > NAV_POINT_YAW_STOP_TOLERANCE)
    {
        speed_abs *= 0.35f;
    }

    return speed_abs;
}

// 统一处理雷区点“提前刹停 -> 中心停车 -> 触发旋转/特殊动作”流程。
// 返回 0 表示未接管；返回 1 表示本周期已接管导航输出；返回 2 表示本周期已触发特殊动作。
static float GetApproachSpeedMag(float speed_sign)
{
    float approach_speed_mag = current_actual_speed * speed_sign;

    if (approach_speed_mag < 0.0f)
    {
        approach_speed_mag = 0.0f;
    }

    return approach_speed_mag;
}

static float UpdateSpecialCaptureSpeedRef(float approach_speed_mag, float speed_sign)
{
    float approach_cmd_mag = s_prev_speed_cmd * speed_sign;

    if (approach_cmd_mag < 0.0f)
    {
        approach_cmd_mag = 0.0f;
    }

    approach_speed_mag = fmaxf(approach_speed_mag, approach_cmd_mag);
    if ((s_special_zero_brake_issued == 0U) &&
        (approach_speed_mag > s_special_capture_speed_ref_mag))
    {
        s_special_capture_speed_ref_mag = approach_speed_mag;
    }

    return fmaxf(s_special_capture_speed_ref_mag, approach_speed_mag);
}

static void ResetEncoderStopPrediction(void)
{
    s_stop_predict_has_prev = 0U;
    s_stop_predict_decel_observed = 0U;
    s_stop_predict_prev_speed_mag = 0.0f;
    s_stop_predict_prev_speed_sign = 0.0f;
    s_stop_predict_decel_mm_s2 = NAV_POINT_SPEED_DECEL_CMD2_PER_MM;
    s_special_capture_speed_ref_mag = 0.0f;
}

static void UpdateEncoderStopPrediction(float approach_speed_mag, float speed_sign)
{
    float approach_accel;
    float measured_decel;

    if ((s_stop_predict_has_prev == 0U) ||
        (fabsf(speed_sign - s_stop_predict_prev_speed_sign) > 0.5f))
    {
        s_stop_predict_has_prev = 1U;
        s_stop_predict_prev_speed_mag = approach_speed_mag;
        s_stop_predict_prev_speed_sign = speed_sign;
        return;
    }

    approach_accel = (approach_speed_mag - s_stop_predict_prev_speed_mag) / NAV_POINT_STOP_PREDICT_DT_S;
    if (approach_accel < -NAV_POINT_STOP_PREDICT_DECEL_MIN)
    {
        measured_decel = Float_Constrain(-approach_accel,
                                         NAV_POINT_STOP_PREDICT_DECEL_MIN,
                                         NAV_POINT_STOP_PREDICT_DECEL_MAX);
        s_stop_predict_decel_mm_s2 =
            (NAV_POINT_STOP_PREDICT_DECEL_ALPHA * measured_decel) +
            ((1.0f - NAV_POINT_STOP_PREDICT_DECEL_ALPHA) * s_stop_predict_decel_mm_s2);
        s_stop_predict_decel_observed = 1U;
    }

    s_stop_predict_prev_speed_mag = approach_speed_mag;
    s_stop_predict_prev_speed_sign = speed_sign;
}

static float ApplyEncoderStopPrediction(float speed_mag, float dist_mm, float stop_radius_mm, float speed_sign)
{
    float remain = dist_mm - stop_radius_mm;
    float approach_speed_mag = GetApproachSpeedMag(speed_sign);
    float decel;
    float predicted_stop_dist;
    float allowed_speed_mag;

    UpdateEncoderStopPrediction(approach_speed_mag, speed_sign);
    decel = Float_Constrain(s_stop_predict_decel_mm_s2,
                            NAV_POINT_STOP_PREDICT_DECEL_MIN,
                            NAV_POINT_STOP_PREDICT_DECEL_MAX);

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    predicted_stop_dist = (approach_speed_mag * approach_speed_mag) / (2.0f * decel);
    allowed_speed_mag = sqrtf(2.0f * decel * remain);
    allowed_speed_mag = Float_Constrain(allowed_speed_mag, 0.0f, fabsf(NAV_POINT_SPEED_FAST));

    if (predicted_stop_dist > (remain + NAV_POINT_STOP_PREDICT_DEADBAND_MM))
    {
        if (allowed_speed_mag < speed_mag)
        {
            speed_mag = allowed_speed_mag;
        }
    }
    else if (predicted_stop_dist < (remain - NAV_POINT_STOP_PREDICT_DEADBAND_MM))
    {
        if (allowed_speed_mag > speed_mag)
        {
            speed_mag = allowed_speed_mag;
        }
    }

    return Float_Constrain(speed_mag, 0.0f, fabsf(NAV_POINT_SPEED_FAST));
}

static uint8 HandleSpecialPointStopAndTrigger(uint16 point_idx,
                                              uint8 point_type,
                                              float dist_to_point,
                                              float point_yaw_deg,
                                              float selected_err_deg,
                                              float speed_sign,
                                              float *out_override_speed)
{
    float abs_vehicle_speed = fabsf(current_actual_speed);
    float approach_speed_mag;
    float brake_radius_mm;
    float target_speed_cmd;

    *out_override_speed = 1e30f; /* sentinel: 未覆盖 */

    SelectSpecialPointHeading(point_yaw_deg,
                              dist_to_point,
                              abs_vehicle_speed,
                              &selected_err_deg,
                              &speed_sign);
    approach_speed_mag = UpdateSpecialCaptureSpeedRef(GetApproachSpeedMag(speed_sign), speed_sign);
    brake_radius_mm = CalcSpecialBrakeRadius(approach_speed_mag);

    /* ---- 补丁：两个雷区距离过近时，放宽刹车距离、提高通过速度 ---- */
    {
        float dist_to_next_minefield = 1e9f;
        uint16 next_idx = (uint16)(point_idx + 1U);
        if ((next_idx < nav_ram_data.point_count) &&
            IsSpecialPointType(nav_ram_data.points[next_idx].point_type))
        {
            dist_to_next_minefield = CalcDistance(nav_ram_data.points[point_idx].x,
                                                  nav_ram_data.points[point_idx].y,
                                                  nav_ram_data.points[next_idx].x,
                                                  nav_ram_data.points[next_idx].y);
        }
        if (dist_to_next_minefield < brake_radius_mm)
        {
            brake_radius_mm = dist_to_next_minefield * 0.5f;
            brake_radius_mm = Float_Constrain(brake_radius_mm,
                                              NAV_POINT_SPECIAL_EXECUTE_RADIUS,
                                              NAV_POINT_SPECIAL_BRAKE_RADIUS_MAX);
        }
    }
    /* ---- 补丁结束 ---- */

    g_nav_point_special_debug_target_idx = point_idx;
    g_nav_point_special_debug_target_x = nav_ram_data.points[point_idx].x;
    g_nav_point_special_debug_target_y = nav_ram_data.points[point_idx].y;
    g_nav_point_special_debug_dist_mm = dist_to_point;
    g_nav_point_special_debug_brake_radius_mm = brake_radius_mm;
    g_nav_point_special_debug_speed_ref_mm_s = approach_speed_mag;
    g_nav_point_special_debug_zero_brake_issued = s_special_zero_brake_issued;
    g_nav_point_special_debug_zero_brake_active = NavReplay_SpecialPointZeroBrakeActive();

    if ((s_special_zero_brake_issued == 0U) &&
        (ShouldStartSpecialPointCapture(dist_to_point, brake_radius_mm) == 0U))
    {
        /* ---- 补丁：两个雷区距离过近时，以 NAV_POINT_SPEED_FAST/2 通过 ---- */
        {
            uint16 next_idx = (uint16)(point_idx + 1U);
            if ((next_idx < nav_ram_data.point_count) &&
                IsSpecialPointType(nav_ram_data.points[next_idx].point_type))
            {
                float dist_to_next = CalcDistance(nav_ram_data.points[point_idx].x,
                                                  nav_ram_data.points[point_idx].y,
                                                  nav_ram_data.points[next_idx].x,
                                                  nav_ram_data.points[next_idx].y);
                float orig_brake_radius = CalcSpecialBrakeRadius(approach_speed_mag);
                if (dist_to_next < orig_brake_radius)
                {
                    *out_override_speed = NAV_POINT_SPEED_FAST * 0.5f;
                }
            }
        }
        /* ---- 补丁结束 ---- */
        Brake_NavHardStop_Reset();
        ResetStopState();
        return 0U;
    }

    UpdateEncoderStopPrediction(approach_speed_mag, speed_sign);

    if (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS)
    {
        s_special_execute_circle_entered = 1U;
    }
    else
    {
        s_special_execute_circle_entered = 0U;
    }

    if (ShouldTriggerSpecialAction(dist_to_point, abs_vehicle_speed) != 0U)
    {
        Brake_NavHardStop_Reset();
        Brake_Feedforward_Reset();
        target_speed_set = NAV_POINT_SPEED_STOP;
        s_prev_speed_cmd = NAV_POINT_SPEED_STOP;

        if (IsSpinPointType(point_type))
        {
            ConfigureSpinPlanForPoint(point_idx);
            minefield_flag = 1U;
            s_spin_exit_pending = 1U;
        }
        else
        {
            ResetSpinExitState();
        }

        g_special_action_trigger = 1U;
        ResetStopState();
        return 2U;
    }

    if (s_special_zero_brake_issued == 0U)
    {
        s_special_zero_brake_issued = 1U;
        s_special_zero_brake_active = 1U;
        g_nav_point_special_debug_zero_brake_issued = s_special_zero_brake_issued;
        g_nav_point_special_debug_zero_brake_active = NavReplay_SpecialPointZeroBrakeActive();
    }

    target_speed_cmd = PlanSpecialApproachSpeed(dist_to_point, speed_sign, abs_vehicle_speed);
    g_nav_point_special_debug_zero_brake_active = NavReplay_SpecialPointZeroBrakeActive();
    Brake_NavHardStop_Reset();

    err_degree = (s_special_execute_circle_entered != 0U) ? 0.0f : selected_err_deg;
    target_speed_set = target_speed_cmd;
    s_prev_speed_cmd = target_speed_cmd;

    return 1U;
}

// 最后点是通过结束点，不做精确停车稳定判定。
static uint8 ShouldFinishAtLastPoint(uint16 point_idx, float dist_to_point)
{
    const NavRamPoint_t *finish_point = &nav_ram_data.points[point_idx];
    const NavRamPoint_t *prev_point;
    float vx;
    float vy;
    float wx;
    float wy;
    float len2;
    float progress;
    float lateral_err;

    if (dist_to_point <= NAV_POINT_FINAL_PASS_RADIUS)
    {
        return 1U;
    }

    if (point_idx == 0U)
    {
        return 0U;
    }

    prev_point = &nav_ram_data.points[(uint16)(point_idx - 1U)];
    vx = finish_point->x - prev_point->x;
    vy = finish_point->y - prev_point->y;
    len2 = vx * vx + vy * vy;
    if (len2 <= 1.0f)
    {
        return 0U;
    }

    wx = inertial_nav.x - prev_point->x;
    wy = inertial_nav.y - prev_point->y;
    progress = ((wx * vx) + (wy * vy)) / len2;
    lateral_err = fabsf((wx * vy) - (wy * vx)) / sqrtf(len2);

    return (uint8)((progress >= 1.0f) &&
                   (lateral_err <= NAV_POINT_FINAL_PASS_LATERAL_RADIUS));
}

static float PlanFinalPassSpeedAbs(float yaw_err_deg)
{
    float speed_abs = fabsf(NAV_POINT_SPEED_FAST);

    if (fabsf(yaw_err_deg) > NAV_POINT_YAW_SLOW_TOLERANCE)
    {
        speed_abs = 0.0f;
    }
    else if (fabsf(yaw_err_deg) > NAV_POINT_YAW_STOP_TOLERANCE)
    {
        speed_abs *= 0.35f;
    }

    return speed_abs;
}

static float PlanSpeedAbsAfterSpinExit(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float speed_abs = PlanDistanceSpeedAbs(dist_mm, stop_radius_mm);
    float yaw_abs = fabsf(yaw_err_deg);
    float spin_exit_speed_max = fabsf(NAV_POINT_SPEED_FAST) *
                                NAV_POINT_SPIN_EXIT_SPEED_RATIO;

    if (yaw_abs <= NAV_POINT_YAW_STOP_TOLERANCE)
    {
        s_spin_exit_align_ticks = 0U;
        return PlanSpeedAbsByDistance(dist_mm, stop_radius_mm, yaw_err_deg);
    }

    if (yaw_abs > NAV_POINT_SPIN_EXIT_MOVE_YAW_MAX)
    {
        return PlanSpeedAbsByDistance(dist_mm, stop_radius_mm, yaw_err_deg);
    }

    return Float_Constrain(speed_abs, 0.0f, spin_exit_speed_max);
}

uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 i;
    uint16 load_count = NAV_REPLAY_STATIC_ROUTE_COUNT;

    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }

    nav_ram_data.plan_type = NAV_PLAN_2;
    nav_ram_data.point_count = load_count;

    for (i = 0; i < load_count; i++)
    {
        nav_ram_data.points[i] = nav_replay_static_route_points[i];
    }

    return load_count;
#else
    return nav_ram_data.point_count;
#endif
}

void NavReplay_Start(void)
{
#if GNSS_NAV == 1
    GpsNavReplay_Stop();
#endif

#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0U)
    {
        return;
    }

    g_target_idx = 0U;
    g_replay_state = REPLAY_RUNNING;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    target_speed_set = NAV_POINT_SPEED_STOP;
    err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    ResetStopState();
    ResetSpinExitState();
    ResetEncoderStopPrediction();
    Minefield_Init();
    Brake_NavHardStop_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0U : 1U;
#else
    g_start_heading_aligned = 1U;
#endif
}

void NavReplay_Stop(void)
{
    target_speed_set = NAV_POINT_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    g_start_heading_aligned = 1U;
    s_prev_speed_cmd = 0.0f;
    ResetStopState();
    ResetSpinExitState();
    ResetEncoderStopPrediction();
    Minefield_Init();
    Brake_NavHardStop_Reset();
    Control_Profile_RequestMode(CONTROL_MODE_NORMAL);
}

void NavReplay_Process(void)
{
    const NavRamPoint_t *point;
    float tx;
    float ty;
    float dist_to_point;
    float point_yaw_deg;
    float selected_err_deg;
    float speed_sign;
    float stop_radius;
    float speed_mag;
    float override_speed = 1e30f;
    uint8 point_type;
    uint8 is_last_point;
    uint8 use_spin_exit_align;

    if (g_replay_state != REPLAY_RUNNING)
    {
        Brake_NavHardStop_Reset();
        return;
    }

    if (g_special_action_trigger != 0U)
    {
        Brake_NavHardStop_Reset();
        return;
    }

    if (s_spin_exit_pending != 0U)
    {
        s_spin_exit_pending = 0U;
        s_spin_exit_align_ticks = NAV_POINT_SPIN_EXIT_ALIGN_TICKS;
    }

#if IMU_CATEGORY == 3
    if (g_start_heading_aligned == 0U)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_POINT_SPEED_STOP;
        if (fabsf(heading_err) <= NAV_POINT_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1U;
            err_degree = 0.0f;
        }
        return;
    }
#endif

    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_POINT_SPEED_STOP;
        err_degree = 0.0f;
        ResetSpinExitState();
        Brake_NavHardStop_Reset();
        return;
    }

    point = &nav_ram_data.points[g_target_idx];
    tx = point->x;
    ty = point->y;
    point_type = point->point_type;
    is_last_point = (uint8)(g_target_idx >= (uint16)(nav_ram_data.point_count - 1U));
    g_current_point_type = point_type;

    dist_to_point = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    if ((point_type == NAV_POINT_PATH) && (is_last_point == 0U) &&
        (dist_to_point <= NAV_POINT_PATH_ARRIVE_RADIUS))
    {
        g_target_idx++;
        ResetStopState();
        ResetSpinExitState();
        ResetEncoderStopPrediction();
        Brake_NavHardStop_Reset();
        return;
    }

    point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    if (IsSpecialPointType(point_type))
    {
        SelectSpecialForwardHeading(point_yaw_deg, &selected_err_deg, &speed_sign);
        err_degree = selected_err_deg;

        uint8 special_result = HandleSpecialPointStopAndTrigger(g_target_idx,
                                                                point_type,
                                                                dist_to_point,
                                                                point_yaw_deg,
                                                                selected_err_deg,
                                                                speed_sign,
                                                                &override_speed);
        if (special_result != 0U)
        {
            if (special_result == 2U)
            {
                if (g_target_idx < (uint16)(nav_ram_data.point_count - 1U))
                {
                    g_target_idx++;
                    ResetEncoderStopPrediction();
                }
                else
                {
                    g_replay_state = REPLAY_FINISHED;
                }
            }
            return;
        }
    }
    else if (is_last_point != 0U)
    {
        if (ShouldFinishAtLastPoint(g_target_idx, dist_to_point) != 0U)
        {
            g_replay_state = REPLAY_FINISHED;
            Brake_NavHardStop_Reset();
            ResetStopState();
            ResetSpinExitState();
            return;
        }
        ResetStopState();
        Brake_NavHardStop_Reset();
    }
    else
    {
        ResetStopState();
        Brake_NavHardStop_Reset();
    }

    stop_radius = IsSpecialPointType(point_type) ? NAV_POINT_SPECIAL_EXECUTE_RADIUS : NAV_POINT_PATH_ARRIVE_RADIUS;

    /* ---- 补丁：两个雷区过近时，直接使用覆盖速度，跳过正常规划 ---- */
    if (override_speed < 1e29f)
    {
        target_speed_set = NavReplay_SpeedSlew_Update(override_speed);
        if (s_spin_exit_align_ticks != 0U)
        {
            s_spin_exit_align_ticks--;
        }
        Brake_NavHardStop_Reset();
        return;
    }
    /* ---- 补丁结束 ---- */

    use_spin_exit_align = (uint8)((s_spin_exit_align_ticks != 0U) &&
                                  (is_last_point == 0U));
    if (is_last_point != 0U)
    {
        speed_mag = PlanFinalPassSpeedAbs(selected_err_deg);
    }
    else if (use_spin_exit_align != 0U)
    {
        speed_mag = PlanSpeedAbsAfterSpinExit(dist_to_point, stop_radius, selected_err_deg);
    }
    else
    {
        speed_mag = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);
    }

    if (is_last_point == 0U)
    {
        speed_mag = ApplyEncoderStopPrediction(speed_mag, dist_to_point, stop_radius, speed_sign);
    }
    target_speed_set = NavReplay_SpeedSlew_Update(speed_sign * speed_mag);
    if (s_spin_exit_align_ticks != 0U)
    {
        s_spin_exit_align_ticks--;
    }
    Brake_NavHardStop_Reset();
}

#endif
