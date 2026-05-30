#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../calculate/pid-new.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../plan/bumpy_road.h"
#include "../../../vision/vision_three_stage_control.h"

#if (CURRENT_NAV_PLAN == 3) && (NAV_PLAN3_METHOD == PLAN3_POINT_SPEED_PLANNING)

extern volatile float target_speed_set;
extern volatile float err_degree;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;

static uint16 g_target_idx = 0U;
static uint8 g_start_heading_aligned = 1U;
static uint8 s_stop_stable_ticks = 0U;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_special_yaw_stable_ticks = 0U;

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
    s_special_yaw_stable_ticks = 0U;
}

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)(point_type == NAV_POINT_CIRCLE);
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

static float CalcSpecialBrakePrepareRadius(void);
static uint8 TriggerPlan3SpecialAction(uint8 point_type);

// 仅在速度还比较高时使用导航强停刹；低速阶段交给普通零速停车控制，避免原地抽搐。
static void UpdateSpecialHardBrakeBySpeed(float abs_vehicle_speed)
{
    if (abs_vehicle_speed > NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_SPEED_MM_S)
    {
        Brake_NavHardStop_Update(1U);
    }
    else
    {
        Brake_NavHardStop_Reset();
    }
}

// 单周期速度斜率限制；普通巡航仍然平滑，但特殊点停车阶段会直接绕过它给 0。
static float SpeedSlew(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_PLAN3_POINT_SPEED_ACCEL_STEP;

    // 加速段直接给目标速度，保留目标速度台阶，避免把加速前瞻的触发条件抹平。
    if (((raw_speed * s_prev_speed_cmd) >= 0.0f) &&
        (fabsf(raw_speed) > fabsf(s_prev_speed_cmd)))
    {
        s_prev_speed_cmd = raw_speed;
        return s_prev_speed_cmd;
    }

    if ((raw_speed * s_prev_speed_cmd) < 0.0f)
    {
        step_limit = NAV_PLAN3_POINT_SPEED_CROSS_ZERO_STEP;
    }
    else if (fabsf(raw_speed) < fabsf(s_prev_speed_cmd))
    {
        step_limit = NAV_PLAN3_POINT_SPEED_DECEL_STEP;
    }

    s_prev_speed_cmd += Float_Constrain(diff, -step_limit, step_limit);
    return s_prev_speed_cmd;
}

// 估算当前沿目标点方向的逼近速度；用于更早进入特殊点刹停准备。
static float ComputeApproachSpeedToPoint(float target_x, float target_y)
{
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, target_x, target_y);
    float yaw_rad;
    float cos_theta;
    float sin_theta;
    float vx_world;
    float vy_world;
    float ux;
    float uy;

    if (dist <= 1.0f)
    {
        return 0.0f;
    }

    yaw_rad = inertial_nav.relative_yaw * 0.0174532925f;
    cos_theta = cosf(yaw_rad);
    sin_theta = sinf(yaw_rad);
    vx_world = inertial_nav.vx_body * cos_theta - inertial_nav.vy_body * sin_theta;
    vy_world = inertial_nav.vx_body * sin_theta + inertial_nav.vy_body * cos_theta;
    ux = (target_x - inertial_nav.x) / dist;
    uy = (target_y - inertial_nav.y) / dist;

    return vx_world * ux + vy_world * uy;
}

static uint8 ShouldStartSpecialBrakeCapture(float dist_to_point, float approach_speed)
{
    float brake_prepare_radius = CalcSpecialBrakePrepareRadius();

    if (dist_to_point <= brake_prepare_radius)
    {
        return 1U;
    }

    if (dist_to_point <= NAV_PLAN2_SPECIAL_RELAX_APPROACH_WINDOW_MM)
    {
        float predicted_dist = dist_to_point;

        if (approach_speed > 0.0f)
        {
            predicted_dist -= approach_speed * NAV_PLAN2_SPECIAL_STOP_PREDICT_TIME_S;
        }

        if (predicted_dist <= brake_prepare_radius)
        {
            return 1U;
        }
    }

    return 0U;
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
    float best_spin_sign = 1.0f;
    uint8 exit_candidate_count = (NAV_PLAN2_ALLOW_REVERSE_TO_NEXT_POINT != 0) ? 2U : 1U;
    uint8 exit_candidate_idx;
    uint8 dir_candidate_idx;

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

        for (dir_candidate_idx = 0U; dir_candidate_idx < 2U; dir_candidate_idx++)
        {
            float spin_sign = (dir_candidate_idx == 0U) ? 1.0f : -1.0f;
            float total_angle;

            if (spin_sign >= 0.0f)
            {
                total_angle = NormalizeAngle(exit_yaw - current_yaw);
            }
            else
            {
                total_angle = NormalizeAngle(current_yaw - exit_yaw);
            }

            if (total_angle < 0.0f)
            {
                total_angle += 360.0f;
            }

            while (total_angle < NAV_PLAN3_POINT_SPIN_MIN_TOTAL_ANGLE)
            {
                total_angle += 360.0f;
            }

            if (total_angle < best_total_angle)
            {
                best_total_angle = total_angle;
                best_exit_yaw = exit_yaw;
                best_spin_sign = spin_sign;
            }
        }
    }

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
    if ((fabsf(err_reverse) + NAV_PLAN3_POINT_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
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

// 按“离停车边界还剩多少距离”实时规划允许速度上限。
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float remain = dist_mm - stop_radius_mm;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    speed_abs = sqrtf(2.0f * NAV_PLAN3_POINT_SPEED_DECEL_CMD2_PER_MM * remain);
    speed_abs = Float_Constrain(speed_abs, 0.0f, fabsf(NAV_PLAN3_POINT_SPEED_FAST));

    if ((speed_abs < fabsf(NAV_PLAN3_POINT_SPEED_SLOW)) && (remain > NAV_PLAN3_POINT_PATH_ARRIVE_RADIUS))
    {
        speed_abs = fabsf(NAV_PLAN3_POINT_SPEED_SLOW);
    }

    if (fabsf(yaw_err_deg) > NAV_PLAN3_POINT_YAW_SLOW_TOLERANCE)
    {
        speed_abs = 0.0f;
    }
    else if (fabsf(yaw_err_deg) > NAV_PLAN3_POINT_YAW_STOP_TOLERANCE)
    {
        speed_abs *= 0.35f;
    }

    return speed_abs;
}

// 根据当前速度和已经建立的刹车前馈，动态计算特殊点刹停准备圆半径。
// 速度越快，准备圆越大；刹车前馈尚未明显建压时，再额外放大准备圆。
static float CalcSpecialBrakePrepareRadius(void)
{
    float speed_abs = fabsf(current_actual_speed);
    float brake_pwm_abs = fabsf(Brake_Feedforward_GetPwm());
    float prepare_radius = NAV_PLAN3_POINT_SPECIAL_BRAKE_PREP_MIN_RADIUS +
                           speed_abs * speed_abs * NAV_PLAN3_POINT_SPECIAL_BRAKE_SPEED2_RADIUS_GAIN;

    if (brake_pwm_abs < NAV_PLAN3_POINT_SPECIAL_BRAKE_READY_PWM)
    {
        prepare_radius += NAV_PLAN3_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN;
    }

    return Float_Constrain(prepare_radius,
                           NAV_PLAN3_POINT_SPECIAL_BRAKE_PREP_MIN_RADIUS,
                           NAV_PLAN3_POINT_SPECIAL_BRAKE_PREP_MAX_RADIUS);
}

static float ClampSpecialYawAlignErr(float yaw_err_deg)
{
    return Float_Constrain(yaw_err_deg,
                           -NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_MAX_ERR,
                           NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_MAX_ERR);
}

static uint8 TriggerPlan3SpecialAction(uint8 point_type)
{
    if (point_type == NAV_POINT_CIRCLE)
    {
        minefield_flag = 1U;
        g_special_action_trigger = 1U;
        return 1U;
    }
    else if (point_type == NAV_POINT_JUMP)
    {
        VisionThreeStageControl_Start();
        g_special_action_trigger = 1U;
        return 1U;
    }
    else if (point_type == NAV_POINT_BRIDGE)
    {
        VisionBridgeTask_Start();
        g_special_action_trigger = 1U;
        return 1U;
    }
    else if (point_type == NAV_POINT_BUMP)
    {
        BumpyRoad_Trigger();
        g_special_action_trigger = 1U;
        return 1U;
    }

    return 0U;
}

// 统一处理特殊点“提前刹停 -> 位置到点 -> 对准目标角 -> 触发特殊动作”流程。
// 返回 0 表示未接管；返回 1 表示本周期已接管导航输出；返回 2 表示本周期已完成该点。
static uint8 HandleSpecialPointStopAndTrigger(uint16 point_idx,
                                              uint8 point_type,
                                              float tx,
                                              float ty,
                                              float dist_to_point,
                                              float speed_sign)
{
    float approach_speed = ComputeApproachSpeedToPoint(tx, ty);
    float abs_vehicle_speed = fabsf(current_actual_speed);
    float target_yaw_err = NormalizeAngle(nav_ram_data.points[point_idx].target_yaw_deg - inertial_nav.relative_yaw);

    if (ShouldStartSpecialBrakeCapture(dist_to_point, approach_speed) == 0U)
    {
        Brake_NavHardStop_Reset();
        ResetStopState();
        return 0U;
    }

    if (dist_to_point > NAV_PLAN3_POINT_SPECIAL_POS_RADIUS)
    {
        ResetStopState();

        // 对角之前先保证位置真正到点；执行圆外优先刹停，只有在足够近且速度很低时才超低速补点。
        if ((dist_to_point > NAV_PLAN3_POINT_SPECIAL_CRAWL_NEAR_RADIUS) ||
            (abs_vehicle_speed > NAV_PLAN3_POINT_SPECIAL_CRAWL_ENTRY_SPEED_MM_S))
        {
            UpdateSpecialHardBrakeBySpeed(abs_vehicle_speed);
            target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
            s_prev_speed_cmd = 0.0f;
            err_degree = 0.0f;
            return 1U;
        }

        Brake_NavHardStop_Reset();
        err_degree = 0.0f;
        target_speed_set = speed_sign * fabsf(NAV_PLAN3_POINT_SPECIAL_CRAWL_SPEED);
        return 1U;
    }

    target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
    s_prev_speed_cmd = 0.0f;

    if (abs_vehicle_speed > NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_SPEED_MM_S)
    {
        err_degree = 0.0f;
        UpdateSpecialHardBrakeBySpeed(abs_vehicle_speed);
        s_stop_stable_ticks = 0U;
        s_special_yaw_stable_ticks = 0U;
        return 1U;
    }

    Brake_NavHardStop_Reset();

    if (abs_vehicle_speed <= NAV_PLAN3_POINT_STOP_SPEED_MM_S)
    {
        if (s_stop_stable_ticks < 255U)
        {
            s_stop_stable_ticks++;
        }
    }
    else
    {
        s_stop_stable_ticks = 0U;
    }

    if (s_stop_stable_ticks < NAV_PLAN3_POINT_STOP_STABLE_TICKS)
    {
        err_degree = 0.0f;
        s_special_yaw_stable_ticks = 0U;
        return 1U;
    }

    err_degree = ClampSpecialYawAlignErr(target_yaw_err);

    if (fabsf(target_yaw_err) <= NAV_PLAN3_POINT_SPECIAL_YAW_TOLERANCE)
    {
        if (s_special_yaw_stable_ticks < 255U)
        {
            s_special_yaw_stable_ticks++;
        }
    }
    else
    {
        s_special_yaw_stable_ticks = 0U;
    }

    if (s_special_yaw_stable_ticks < NAV_PLAN3_POINT_SPECIAL_YAW_STABLE_TICKS)
    {
        return 1U;
    }

    if (IsSpinPointType(point_type))
    {
        ConfigureSpinPlanForPoint(point_idx);
    }

    TriggerPlan3SpecialAction(point_type);
    Brake_NavHardStop_Reset();
    ResetStopState();
    return 2U;
}

// 统一处理最终终点停车并对准终点 target_yaw_deg。
static uint8 HandleFinalStopAndFinish(uint16 point_idx, float dist_to_point)
{
    float target_yaw_err = NormalizeAngle(nav_ram_data.points[point_idx].target_yaw_deg - inertial_nav.relative_yaw);

    if (dist_to_point > NAV_PLAN3_POINT_FINAL_STOP_RADIUS)
    {
        Brake_NavHardStop_Reset();
        ResetStopState();
        return 0U;
    }

    target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
    s_prev_speed_cmd = 0.0f;

    if (fabsf(current_actual_speed) <= NAV_PLAN3_POINT_STOP_SPEED_MM_S)
    {
        if (s_stop_stable_ticks < 255U)
        {
            s_stop_stable_ticks++;
        }
    }
    else
    {
        s_stop_stable_ticks = 0U;
    }

    if (s_stop_stable_ticks < NAV_PLAN3_POINT_STOP_STABLE_TICKS)
    {
        err_degree = 0.0f;
        s_special_yaw_stable_ticks = 0U;
        return 1U;
    }

    err_degree = ClampSpecialYawAlignErr(target_yaw_err);

    if (fabsf(target_yaw_err) <= NAV_PLAN3_POINT_FINAL_YAW_TOLERANCE)
    {
        if (s_special_yaw_stable_ticks < 255U)
        {
            s_special_yaw_stable_ticks++;
        }
    }
    else
    {
        s_special_yaw_stable_ticks = 0U;
    }

    if (s_special_yaw_stable_ticks < NAV_PLAN3_POINT_SPECIAL_YAW_STABLE_TICKS)
    {
        return 1U;
    }

    g_replay_state = REPLAY_FINISHED;
    target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
    err_degree = 0.0f;
    Brake_NavHardStop_Reset();
    ResetStopState();
    return 1U;
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

    nav_ram_data.plan_type = NAV_PLAN_3;
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
    target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
    err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    ResetStopState();
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
    target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    g_start_heading_aligned = 1U;
    s_prev_speed_cmd = 0.0f;
    ResetStopState();
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

    if ((g_replay_state != REPLAY_RUNNING) || (g_special_action_trigger != 0U))
    {
        Brake_NavHardStop_Reset();
        return;
    }

#if IMU_CATEGORY == 3
    if (g_start_heading_aligned == 0U)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
        if (fabsf(heading_err) <= NAV_PLAN3_POINT_START_HEADING_TOLERANCE)
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
        target_speed_set = NAV_PLAN3_POINT_SPEED_STOP;
        err_degree = 0.0f;
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
        (dist_to_point <= NAV_PLAN3_POINT_PATH_ARRIVE_RADIUS))
    {
        g_target_idx++;
        ResetStopState();
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
                                                                tx,
                                                                ty,
                                                                dist_to_point,
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
        if (HandleFinalStopAndFinish(g_target_idx, dist_to_point) != 0U)
        {
            return;
        }
    }
    else
    {
        ResetStopState();
        Brake_NavHardStop_Reset();
    }

    stop_radius = IsSpecialPointType(point_type) ? NAV_PLAN3_POINT_SPECIAL_POS_RADIUS : NAV_PLAN3_POINT_PATH_ARRIVE_RADIUS;
    if (is_last_point != 0U)
    {
        stop_radius = NAV_PLAN3_POINT_FINAL_STOP_RADIUS;
    }

    speed_abs = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);

    target_speed_set = SpeedSlew(speed_sign * speed_abs);
    Brake_NavHardStop_Reset();
}

#endif
