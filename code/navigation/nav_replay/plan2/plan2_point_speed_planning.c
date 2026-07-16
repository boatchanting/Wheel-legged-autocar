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

static uint16 g_target_idx = 0U;
static uint8 g_start_heading_aligned = 1U;
static uint8 s_stop_stable_ticks = 0U;
static uint8 s_stop_yaw_locked = 0U;
static float s_stop_yaw_deg = 0.0f;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_spin_exit_pending = 0U;
static uint8 s_spin_exit_align_ticks = 0U;

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
    s_stop_stable_ticks = 0U;
    s_stop_yaw_locked = 0U;
    s_stop_yaw_deg = 0.0f;
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

static float CalcSpecialEstimatedStopDist(float speed_abs);
static float CalcSpecialHardBrakeStrength(float dist_to_point, float speed_abs);
static float CalcSpecialBrakePrepareRadius(float speed_abs);
static float PositiveAngle360(float angle);
static float CalcSpinTotalAngle(float current_yaw, float exit_yaw, float spin_sign);
static float PlanDistanceSpeedAbs(float dist_mm, float stop_radius_mm);
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg);
static float PlanSpeedAbsAfterSpinExit(float dist_mm, float stop_radius_mm, float yaw_err_deg);
static uint8 ShouldStartSpecialBrakeCapture(float dist_to_point, float speed_abs);
static void UpdateSpecialHardBrakeBySpeed(float hard_brake_strength);
static float PlanSpecialApproachSpeed(float dist_to_point, float speed_sign, float hard_brake_strength);
static uint8 ShouldTriggerSpecialAction(float dist_to_point, float speed_abs, float yaw_err);
static uint8 ShouldFinishAtLastPoint(uint16 point_idx, float dist_to_point);
static float PlanFinalPassSpeedAbs(float yaw_err_deg);

static float CalcSpecialEstimatedStopDist(float speed_abs)
{
    if (NAV_POINT_SPECIAL_STOP_DECEL_MM_S2 <= 0.0f)
    {
        return 0.0f;
    }

    return (speed_abs * speed_abs) / (2.0f * NAV_POINT_SPECIAL_STOP_DECEL_MM_S2);
}

static float CalcSpecialHardBrakeStrength(float dist_to_point, float speed_abs)
{
    float estimated_stop_dist = CalcSpecialEstimatedStopDist(speed_abs);
    float brake_budget = estimated_stop_dist + NAV_POINT_SPECIAL_BRAKE_SAFETY_MARGIN;
    float brake_error = brake_budget - dist_to_point;

    return Float_Constrain(0.5f + (brake_error / NAV_POINT_SPECIAL_BRAKE_BLEND_DIST), 0.0f, 1.0f);
}

static float CalcSpecialBrakePrepareRadius(float speed_abs)
{
    float brake_budget = CalcSpecialEstimatedStopDist(speed_abs) +
                         NAV_POINT_SPECIAL_BRAKE_SAFETY_MARGIN;
    float prepare_radius = brake_budget + (0.5f * NAV_POINT_SPECIAL_BRAKE_BLEND_DIST);

    if (prepare_radius < NAV_POINT_SPECIAL_CRAWL_RADIUS)
    {
        prepare_radius = NAV_POINT_SPECIAL_CRAWL_RADIUS;
    }

    return prepare_radius;
}

static uint8 ShouldStartSpecialBrakeCapture(float dist_to_point, float speed_abs)
{
    return (uint8)(dist_to_point <= CalcSpecialBrakePrepareRadius(speed_abs));
}

static void UpdateSpecialHardBrakeBySpeed(float hard_brake_strength)
{
    Brake_NavHardStop_UpdateStrength(hard_brake_strength);
}

static float PlanSpecialApproachSpeed(float dist_to_point, float speed_sign, float hard_brake_strength)
{
    float slow_abs = fabsf(NAV_POINT_SPEED_SLOW);
    float crawl_abs = fabsf(NAV_POINT_SPECIAL_CRAWL_SPEED);
    float min_approach_abs = (slow_abs < crawl_abs) ? slow_abs : crawl_abs;
    float max_approach_abs = (slow_abs > crawl_abs) ? slow_abs : crawl_abs;
    float approach_abs;
    uint8 in_crawl_band;

    if (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS)
    {
        return NAV_POINT_SPEED_STOP;
    }

    in_crawl_band = (uint8)(dist_to_point <= NAV_POINT_SPECIAL_CRAWL_RADIUS);
    if (in_crawl_band != 0U)
    {
        approach_abs = crawl_abs;
    }
    else
    {
        approach_abs = PlanSpeedAbsByDistance(dist_to_point,
                                              NAV_POINT_SPECIAL_EXECUTE_RADIUS,
                                              0.0f);
        approach_abs = Float_Constrain(approach_abs,
                                       min_approach_abs,
                                       max_approach_abs);
    }

    if ((hard_brake_strength > 0.0f) &&
        (dist_to_point > NAV_POINT_SPECIAL_CRAWL_RADIUS))
    {
        approach_abs *= (1.0f - hard_brake_strength);
        if (approach_abs < min_approach_abs)
        {
            approach_abs = min_approach_abs;
        }
    }

    return speed_sign * approach_abs;
}

static uint8 ShouldTriggerSpecialAction(float dist_to_point, float speed_abs, float yaw_err)
{
    uint8 yaw_ok = (uint8)((s_stop_yaw_locked != 0U) ||
                           (fabsf(yaw_err) <= NAV_POINT_YAW_STOP_TOLERANCE));

    return (uint8)((dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS) &&
                   (speed_abs <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S) &&
                   (yaw_ok != 0U));
}

// 单周期速度斜率限制；普通巡航仍然平滑，但雷区停车阶段会直接绕过它给 0�?
static float SpeedSlew(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_POINT_SPEED_ACCEL_STEP;

    // 加速段直接给目标速度，保留目标速度台阶，避免把加速前馈的触发条件抹平�?
    if (((raw_speed * s_prev_speed_cmd) >= 0.0f) &&
        (fabsf(raw_speed) > fabsf(s_prev_speed_cmd)))
    {
        s_prev_speed_cmd = raw_speed;
        return s_prev_speed_cmd;
    }

    if ((raw_speed * s_prev_speed_cmd) < 0.0f)
    {
        step_limit = NAV_POINT_SPEED_CROSS_ZERO_STEP;
    }
    else if (fabsf(raw_speed) < fabsf(s_prev_speed_cmd))
    {
        step_limit = NAV_POINT_SPEED_DECEL_STEP;
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

// 从触发瞬间的当前车头角出发，分别计算车头/车尾朝向下一个目标点的总旋转角度，选更快的一组�?
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

// 在“正向朝向目标点”和“反向朝向目标点”之间自动选择转向误差更小的一侧�?
static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

#if NAV_PLAN2_ALLOW_REVERSE_TO_NEXT_POINT
    // 允许倒车时，自动比较车头/车尾朝向目标点所需的转角，选更快的一侧�?
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

// 按“离停车边界还剩多少距离”实时规划允许速度上限�?
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

// 统一处理雷区点“提前刹�?-> 中心停车 -> 触发旋转/特殊动作”流程�?
// 返回 0 表示未接管；返回 1 表示本周期已接管导航输出；返�?2 表示本周期已触发特殊动作�?
static uint8 HandleSpecialPointStopAndTrigger(uint16 point_idx,
                                              uint8 point_type,
                                              float dist_to_point,
                                              float selected_err_deg,
                                              float speed_sign)
{
    float abs_vehicle_speed = fabsf(current_actual_speed);
    float hard_brake_strength;
    float target_speed_cmd;

    if (ShouldStartSpecialBrakeCapture(dist_to_point, abs_vehicle_speed) == 0U)
    {
        Brake_NavHardStop_Reset();
        ResetStopState();
        return 0U;
    }

    hard_brake_strength = CalcSpecialHardBrakeStrength(dist_to_point, abs_vehicle_speed);

    if ((dist_to_point > NAV_POINT_SPECIAL_EXECUTE_RADIUS) &&
        (dist_to_point <= NAV_POINT_SPECIAL_CRAWL_RADIUS) &&
        (abs_vehicle_speed <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S))
    {
        hard_brake_strength = 0.0f;
    }

    if (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS)
    {
        if (s_stop_yaw_locked == 0U)
        {
            s_stop_yaw_locked = 1U;
            s_stop_yaw_deg = inertial_nav.relative_yaw;
        }
    }
    else
    {
        ResetStopState();
    }

    if (s_stop_yaw_locked != 0U)
    {
        err_degree = NormalizeAngle(s_stop_yaw_deg - inertial_nav.relative_yaw);
    }
    else
    {
        err_degree = selected_err_deg;
    }

    UpdateSpecialHardBrakeBySpeed(hard_brake_strength);
    target_speed_cmd = PlanSpecialApproachSpeed(dist_to_point, speed_sign, hard_brake_strength);
    target_speed_set = target_speed_cmd;
    s_prev_speed_cmd = target_speed_cmd;

    if (ShouldTriggerSpecialAction(dist_to_point, abs_vehicle_speed, selected_err_deg) != 0U)
    {
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
        Brake_NavHardStop_Reset();
        ResetStopState();
        return 2U;
    }

    return 1U;
}

// 最后点是通过结束点，不做精确停车稳定判定�?
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
    Minefield_Init();
    Brake_NavHardStop_Reset();

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
    Minefield_Init();
    Brake_NavHardStop_Reset();
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
    float speed_abs;
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
        Brake_NavHardStop_Reset();
        return;
    }

    point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    if (IsSpecialPointType(point_type))
    {
        uint8 special_result = HandleSpecialPointStopAndTrigger(g_target_idx,
                                                                point_type,
                                                                dist_to_point,
                                                                selected_err_deg,
                                                                speed_sign);
        if (special_result != 0U)
        {
            if (special_result == 2U)
            {
                if (g_target_idx < (uint16)(nav_ram_data.point_count - 1U))
                {
                    g_target_idx++;
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
    use_spin_exit_align = (uint8)((s_spin_exit_align_ticks != 0U) &&
                                  (is_last_point == 0U));
    if (is_last_point != 0U)
    {
        speed_abs = PlanFinalPassSpeedAbs(selected_err_deg);
    }
    else if (use_spin_exit_align != 0U)
    {
        speed_abs = PlanSpeedAbsAfterSpinExit(dist_to_point, stop_radius, selected_err_deg);
    }
    else
    {
        speed_abs = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);
    }

    target_speed_set = SpeedSlew(speed_sign * speed_abs);
    if (s_spin_exit_align_ticks != 0U)
    {
        s_spin_exit_align_ticks--;
    }
    Brake_NavHardStop_Reset();
}

#endif
