#include "../nav_replay.h"
#include "../../../config/generated/sys_options_debug.h"
#include "../../../config/generated/sys_options_imu.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../calculate/pid-new.h"

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_METHOD_PRECISE)

extern volatile float target_speed_set;
extern volatile float err_degree;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;

static uint16 g_target_idx = 0U;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static uint8 g_start_heading_aligned = 1U;
static uint8 s_special_stop_stable_ticks = 0U;
static uint8 s_special_stop_yaw_locked = 0U;
static float s_special_stop_yaw_deg = 0.0f;
static float s_prev_speed_cmd = 0.0f;

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

// 重置雷区停车稳定判定状态；切换目标点或退出雷区捕获逻辑时调用。
static void ResetSpecialStopState(void)
{
    s_special_stop_stable_ticks = 0U;
    s_special_stop_yaw_locked = 0U;
    s_special_stop_yaw_deg = 0.0f;
}

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) || (point_type == NAV_POINT_JUMP));
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

// 估算车体当前沿“车->目标点”方向的逼近速度；大于 0 表示正在朝目标点逼近。
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

// 判断是否要进入“雷区刹停捕获”阶段。
// 严格模式按固定准备距离进入；宽松模式允许按预测距离更早进入减速准备。
static uint8 ShouldStartSpecialBrakeCapture(float dist_to_center, float approach_speed)
{
    if (dist_to_center <= NAV_SPECIAL_BRAKE_PREP_DIST)
    {
        return 1U;
    }

#if NAV_PLAN2_SPECIAL_APPROACH_MODE == PLAN2_SPECIAL_APPROACH_CENTER_RELAXED
    if (dist_to_center <= NAV_SPECIAL_RELAX_APPROACH_WINDOW)
    {
        float predicted_dist = dist_to_center;

        if (approach_speed > 0.0f)
        {
            predicted_dist -= approach_speed * NAV_SPECIAL_STOP_PREDICT_TIME;
        }

        if (predicted_dist <= NAV_SPECIAL_BRAKE_PREP_DIST)
        {
            return 1U;
        }
    }
#else
    (void)approach_speed;
#endif

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

            while (total_angle < NAV_SPIN_MIN_TOTAL_ANGLE)
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

#if DEBUG_LOG_ENABLE
    printf("[Nav] Spin Plan idx=%d exit=%.2f total=%.2f sign=%.0f\r\n",
           point_idx, best_exit_yaw, best_total_angle, best_spin_sign);
#endif
}

// 在“正向朝向目标点”和“反向朝向目标点”之间选择转向误差更小的一种。
// speed_sign = -1 表示按当前前进符号前进，speed_sign = 1 表示倒车逼近。
static void SelectDriveHeading(float point_yaw_deg,
                               float *selected_yaw_deg,
                               float *selected_err_deg,
                               float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

#if NAV_PLAN2_ALLOW_REVERSE_TO_NEXT_POINT
    // 允许倒车时，自动比较车头/车尾朝向目标点所需的转角，选更快的一侧。
    if ((fabsf(err_reverse) + NAV_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
    {
        *selected_yaw_deg = reverse_yaw;
        *selected_err_deg = err_reverse;
        *speed_sign = 1.0f;
    }
    else
#endif
    {
        *selected_yaw_deg = point_yaw_deg;
        *selected_err_deg = err_forward;
        *speed_sign = -1.0f;
    }
}

// 对离线路表目标速度做斜率限制，避免路表速度台阶直接传到底盘。
static float OfflineSpeedSlew(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_cmd);
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit;

    // 加速段直接给路表目标速度，保留目标速度台阶，避免把加速前馈的触发条件抹平。
    if (((raw_speed * s_prev_speed_cmd) >= 0.0f) && (abs_raw > abs_prev))
    {
        s_prev_speed_cmd = raw_speed;
        return s_prev_speed_cmd;
    }

    if ((raw_speed * s_prev_speed_cmd) < 0.0f)
    {
        step_limit = NAV_OFFLINE_SPEED_SLEW_CROSS_ZERO;
    }
    else if (abs_raw > (abs_prev + NAV_OFFLINE_SPEED_SLEW_EPS))
    {
        step_limit = (abs_prev < NAV_OFFLINE_SPEED_SLEW_LOW_TH) ? NAV_OFFLINE_SPEED_SLEW_UP_LOW : NAV_OFFLINE_SPEED_SLEW_UP_NORMAL;
    }
    else if ((abs_raw + NAV_OFFLINE_SPEED_SLEW_EPS) < abs_prev)
    {
        step_limit = (abs_prev > NAV_OFFLINE_SPEED_SLEW_FAST_TH) ? NAV_OFFLINE_SPEED_SLEW_DOWN_FAST : NAV_OFFLINE_SPEED_SLEW_DOWN_NORMAL;
    }
    else
    {
        step_limit = NAV_OFFLINE_SPEED_SLEW_UP_NORMAL;
    }

    s_prev_speed_cmd += Float_Constrain(diff, -step_limit, step_limit);
    return s_prev_speed_cmd;
}

// 读取离线路表目标速度。普通点按路表速度跑；若路表给 0 但尚未到点，给一个低速兜底防止爬死。
static float GetOfflineSpeedAbs(const NavRamPoint_t *point, float nav_dist)
{
    float speed_abs = fabsf(point->target_speed);

    if ((speed_abs <= NAV_OFFLINE_SPEED_EPS) && (nav_dist > NAV_DIST_ARRIVE))
    {
        speed_abs = fabsf(NAV_SPEED_SLOW);
    }

    return speed_abs;
}

// 统一处理雷区点“提前刹停 -> 中心停车 -> 触发旋转/特殊动作”流程。
// 返回 0 表示未接管；返回 1 表示本周期已接管导航输出；返回 2 表示本周期已触发特殊动作。
static uint8 HandleSpecialPointStopAndTrigger(uint16 point_idx,
                                              uint8 point_type,
                                              float tx,
                                              float ty,
                                              float dist_to_center,
                                              float selected_err_deg,
                                              float speed_sign)
{
    float approach_speed = ComputeApproachSpeedToPoint(tx, ty);
    float abs_vehicle_speed = fabsf(current_actual_speed);

    if (ShouldStartSpecialBrakeCapture(dist_to_center, approach_speed) == 0U)
    {
        ResetSpecialStopState();
        return 0U;
    }

    // 还未进入中心触发半径时，先刹停；速度已经很低后，切换到超低速爬行补进中心。
    if (dist_to_center > NAV_SPECIAL_TRIGGER_RADIUS)
    {
        ResetSpecialStopState();
        if (abs_vehicle_speed > NAV_SPECIAL_CRAWL_ENTRY_SPEED_MM_S)
        {
            target_speed_set = NAV_SPEED_STOP;
            s_prev_speed_cmd = 0.0f;
        }
        else
        {
            target_speed_set = speed_sign * fabsf(NAV_SPECIAL_CRAWL_SPEED);
        }
        return 1U;
    }

    if (s_special_stop_yaw_locked == 0U)
    {
        s_special_stop_yaw_locked = 1U;
        s_special_stop_yaw_deg = inertial_nav.relative_yaw;
    }

    target_speed_set = NAV_SPEED_STOP;
    s_prev_speed_cmd = 0.0f;
    err_degree = NormalizeAngle(s_special_stop_yaw_deg - inertial_nav.relative_yaw);

    if (abs_vehicle_speed <= NAV_SPECIAL_STOP_SPEED_MM_S)
    {
        if (s_special_stop_stable_ticks < 255U)
        {
            s_special_stop_stable_ticks++;
        }
    }
    else
    {
        s_special_stop_stable_ticks = 0U;
    }

    if (s_special_stop_stable_ticks < NAV_SPECIAL_STOP_STABLE_TICKS)
    {
        return 1U;
    }

    if (IsSpinPointType(point_type))
    {
        ConfigureSpinPlanForPoint(point_idx);
        minefield_flag = 1U;
    }

    g_special_action_trigger = 1U;
    ResetSpecialStopState();
    return 2U;
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
#if DEBUG_LOG_ENABLE
        printf("[Nav] RAM is empty, cannot start replay.\r\n");
#endif
        return;
    }

    g_target_idx = 0U;
    g_replay_state = REPLAY_RUNNING;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    ResetSpecialStopState();
    Minefield_Init();

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0U : 1U;
#else
    g_start_heading_aligned = 1U;
#endif

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay START. Plan: %d, Total Points: %d\r\n",
           nav_ram_data.plan_type, nav_ram_data.point_count);
#endif
}

void NavReplay_Stop(void)
{
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    g_start_heading_aligned = 1U;
    s_prev_speed_cmd = 0.0f;
    ResetSpecialStopState();
    Minefield_Init();

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
#endif
}

void NavReplay_Process(void)
{
    float tx;
    float ty;
    float dist_to_center;
    float point_yaw_deg;
    float selected_yaw_deg;
    float selected_err_deg;
    float speed_sign;
    float speed_abs;
    uint8 point_type;
    uint8 is_special_point;

    if ((g_replay_state != REPLAY_RUNNING) || (g_special_action_trigger != 0U))
    {
        return;
    }

#if IMU_CATEGORY == 3
    if (g_start_heading_aligned == 0U)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1U;
            err_degree = 0.0f;
#if DEBUG_LOG_ENABLE
            printf("[Nav] Start heading aligned: %.2f deg\r\n", heading);
#endif
        }
        else
        {
            return;
        }
    }
#endif

    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        s_prev_speed_cmd = 0.0f;
        err_degree = 0.0f;
#if DEBUG_LOG_ENABLE
        printf("[Nav] Replay Finished.\r\n");
#endif
        return;
    }

    tx = nav_ram_data.points[g_target_idx].x;
    ty = nav_ram_data.points[g_target_idx].y;
    point_type = nav_ram_data.points[g_target_idx].point_type;
    is_special_point = IsSpecialPointType(point_type);
    g_current_point_type = point_type;

    dist_to_center = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    if ((!is_special_point) && (dist_to_center <= NAV_DIST_ARRIVE))
    {
        g_target_idx++;
        ResetSpecialStopState();

#if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Path Point[%d]\r\n", (g_target_idx - 1U));
#endif
        return;
    }

    point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    if (is_special_point)
    {
        uint8 special_result = HandleSpecialPointStopAndTrigger(g_target_idx,
                                                                point_type,
                                                                tx,
                                                                ty,
                                                                dist_to_center,
                                                                selected_err_deg,
                                                                speed_sign);
        if (special_result != 0U)
        {
            if (special_result == 2U)
            {
                g_target_idx++;
            }
            return;
        }
    }
    else
    {
        ResetSpecialStopState();
    }

    if (fabsf(selected_err_deg) > NAV_YAW_TOLERANCE)
    {
        target_speed_set = NAV_SPEED_STOP;
        s_prev_speed_cmd = 0.0f;
        return;
    }

    speed_abs = GetOfflineSpeedAbs(&nav_ram_data.points[g_target_idx], dist_to_center);
    target_speed_set = OfflineSpeedSlew(speed_sign * speed_abs);
    (void)selected_yaw_deg;
}

#endif
