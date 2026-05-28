#include "../nav_replay.h"
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

static uint16 g_target_idx = 0;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static uint8 g_start_heading_aligned = 1;
static uint16 g_special_eval_idx = 0xFFFFU;
static float g_special_min_center_dist = 1000000.0f;
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

// 清空当前特殊点的逼近统计状态；切换目标点或退出特殊点判断时调用。
static void ResetSpecialApproachState(void)
{
    g_special_eval_idx = 0xFFFFU;
    g_special_min_center_dist = 1000000.0f;
}

// 将雷区逼近信息同步给底层雷霆重刹逻辑；只有圆环点会触发该请求。
static void UpdateMinefieldThunderBrakeRequest(uint8 point_type, float dist_to_center)
{
    if (point_type == NAV_POINT_CIRCLE)
    {
        Brake_MinefieldThunderBrake_Update(1U, dist_to_center);
    }
    else
    {
        Brake_MinefieldThunderBrake_Reset();
    }
}

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) || (point_type == NAV_POINT_JUMP));
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

// 估算车辆沿“指向目标点方向”的逼近速度。
// 宽松触发模式下会用这个量预测下一小段时间是否会进入中心触发区。
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

// 判断当前特殊点是否应该触发。
// 严格模式只看中心半径；宽松模式会额外参考逼近速度和最近距离历史。
static uint8 ShouldTriggerSpecialPoint(float dist_to_center, float approach_speed)
{
    if (dist_to_center <= NAV_SPECIAL_TRIGGER_RADIUS)
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

        if (predicted_dist <= NAV_SPECIAL_TRIGGER_RADIUS)
        {
            return 1U;
        }

        if ((g_special_min_center_dist <= NAV_SPECIAL_RELAX_APPROACH_WINDOW) &&
            (dist_to_center >= (g_special_min_center_dist + NAV_SPECIAL_PASS_AWAY_MARGIN)) &&
            (approach_speed <= 0.0f))
        {
            return 1U;
        }
    }

    return 0U;
#else
    (void)approach_speed;
    return 0U;
#endif
}

static uint16 FindNextSpecialPointIndex(uint16 start_idx)
{
    uint16 idx;

    for (idx = start_idx; idx < nav_ram_data.point_count; idx++)
    {
        if (IsSpecialPointType(nav_ram_data.points[idx].point_type))
        {
            return idx;
        }
    }

    return nav_ram_data.point_count;
}

static float ComputeDirectionalSpinDelta(float from_yaw_deg, float to_yaw_deg, float spin_dir_sign)
{
    float delta;

    if (spin_dir_sign >= 0.0f)
    {
        delta = NormalizeAngle(to_yaw_deg - from_yaw_deg);
    }
    else
    {
        delta = NormalizeAngle(from_yaw_deg - to_yaw_deg);
    }

    if (delta < 0.0f)
    {
        delta += 360.0f;
    }

    return delta;
}

static void SelectDriveHeading(float point_yaw_deg, float *selected_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

    if ((fabsf(err_reverse) + NAV_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
    {
        *selected_yaw_deg = reverse_yaw;
        *selected_err_deg = err_reverse;
        *speed_sign = 1.0f;
    }
    else
    {
        *selected_yaw_deg = point_yaw_deg;
        *selected_err_deg = err_forward;
        *speed_sign = -1.0f;
    }
}

// 对离线路表目标速度做斜率限制。
// 这样即使 chazhi.py 规划出的相邻点速度变化较快，实车执行也更平顺。
static float OfflineSpeedSlew(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_cmd);
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit;

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

// 从路表读取离线规划速度，并在少数边界场景下做安全兜底：
// 1. 路表速度为 0 但车尚未真正到点时，给一个很小的爬行速度；
// 2. 特殊点附近即使离线速度偏快，也压到慢速保证进框姿态。
static float GetOfflineSpeedAbs(const NavRamPoint_t *point, uint8 is_special_point, float nav_dist)
{
    float speed_abs = fabsf(point->target_speed);

    // 离线速度规划的唯一速度来源是路表 target_speed；若旧路表/停车点速度为 0，
    // 仅在尚未到点时给一个低速爬行兜底，避免卡在终点或雷区中心前一个采样点。
    if ((speed_abs <= NAV_OFFLINE_SPEED_EPS) && (nav_dist > NAV_DIST_ARRIVE))
    {
        speed_abs = fabsf(NAV_SPEED_SLOW);
    }

    // 雷区附近即使离线曲线给得偏快，也压到慢速，优先保证进框停车姿态。
    if (is_special_point && (nav_dist <= NAV_SPECIAL_APPROACH_DIST) && (speed_abs > fabsf(NAV_SPEED_SLOW)))
    {
        speed_abs = fabsf(NAV_SPEED_SLOW);
    }

    return speed_abs;
}

// 为当前特殊点规划旋转总角度、退出航向和旋转方向。
// 方案3虽然速度来自离线路表，但转圈后的出框朝向仍然按当前路径关系在线决定。
static void ConfigureSpinPlanForPoint(uint16 point_idx)
{
    uint16 next_special_idx;
    float current_yaw = inertial_nav.relative_yaw;
    float exit_forward_yaw;
    float exit_reverse_yaw;
    float best_total_angle = 1000000.0f;
    float best_exit_yaw = current_yaw;
    float best_spin_sign = 1.0f;
    uint8 exit_candidate_idx;
    uint8 dir_candidate_idx;

    next_special_idx = FindNextSpecialPointIndex((uint16)(point_idx + 1U));
    if (next_special_idx < nav_ram_data.point_count)
    {
        exit_forward_yaw = CalcBearingDeg(nav_ram_data.points[point_idx].x,
                                          nav_ram_data.points[point_idx].y,
                                          nav_ram_data.points[next_special_idx].x,
                                          nav_ram_data.points[next_special_idx].y);
    }
    else
    {
        exit_forward_yaw = nav_ram_data.points[point_idx].target_yaw_deg;
    }

    exit_reverse_yaw = NormalizeAngle(exit_forward_yaw + 180.0f);

    for (exit_candidate_idx = 0U; exit_candidate_idx < 2U; exit_candidate_idx++)
    {
        float exit_yaw = (exit_candidate_idx == 0U) ? exit_forward_yaw : exit_reverse_yaw;

        for (dir_candidate_idx = 0U; dir_candidate_idx < 2U; dir_candidate_idx++)
        {
            float spin_sign = (dir_candidate_idx == 0U) ? 1.0f : -1.0f;
            float total_angle = ComputeDirectionalSpinDelta(current_yaw, exit_yaw, spin_sign);

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

    if (nav_ram_data.point_count == 0)
    {
#if DEBUG_LOG_ENABLE
        printf("[Nav] RAM is empty, cannot start replay.\r\n");
#endif
        return;
    }

    g_target_idx = 0;
    g_replay_state = REPLAY_RUNNING;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0;
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    Minefield_Init();
    ResetSpecialApproachState();
    Brake_MinefieldThunderBrake_Reset();

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0 : 1;
#else
    g_start_heading_aligned = 1;
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
    g_special_action_trigger = 0;
    g_start_heading_aligned = 1;
    s_prev_speed_cmd = 0.0f;
    Minefield_Init();
    ResetSpecialApproachState();
    Brake_MinefieldThunderBrake_Reset();

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
#endif
}

void NavReplay_Process(void)
{
    float tx;
    float ty;
    float dist_to_center;
    float nav_dist;
    float approach_speed;
    float point_yaw_deg;
    float selected_yaw_deg;
    float selected_err_deg;
    float speed_sign;
    float speed_abs;
    uint8 point_type;
    uint8 is_special_point;

    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1)
    {
        Brake_MinefieldThunderBrake_Reset();
        return;
    }

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
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
        Brake_MinefieldThunderBrake_Reset();
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
    nav_dist = dist_to_center;
    approach_speed = 0.0f;
    UpdateMinefieldThunderBrakeRequest(point_type, dist_to_center);

    if (is_special_point)
    {
        if (g_special_eval_idx != g_target_idx)
        {
            g_special_eval_idx = g_target_idx;
            g_special_min_center_dist = dist_to_center;
        }
        else if (dist_to_center < g_special_min_center_dist)
        {
            g_special_min_center_dist = dist_to_center;
        }

        approach_speed = ComputeApproachSpeedToPoint(tx, ty);
    }
    else
    {
        ResetSpecialApproachState();
    }

    if (is_special_point && ShouldTriggerSpecialPoint(dist_to_center, approach_speed))
    {
#if DEBUG_LOG_ENABLE
        float min_center_dist_snapshot = g_special_min_center_dist;
#endif

        target_speed_set = NAV_SPEED_STOP;
        s_prev_speed_cmd = 0.0f;
        err_degree = 0.0f;

        if (IsSpinPointType(point_type))
        {
            ConfigureSpinPlanForPoint(g_target_idx);
            minefield_flag = 1;
        }

        g_special_action_trigger = 1;
        g_target_idx++;

#if DEBUG_LOG_ENABLE
        printf("[Nav] Trigger Special Point[%d] Type[%d] CenterDist=%.1f MinDist=%.1f V=%.1f\r\n",
               (g_target_idx - 1U), point_type, dist_to_center, min_center_dist_snapshot, approach_speed);
#endif
        ResetSpecialApproachState();
        Brake_MinefieldThunderBrake_Reset();
        return;
    }

    if ((!is_special_point) && (dist_to_center <= NAV_DIST_ARRIVE))
    {
        // 普通路径点只推进索引，速度继续沿离线路表曲线走，不再每个点停顿。
        g_target_idx++;
        ResetSpecialApproachState();
        Brake_MinefieldThunderBrake_Reset();

#if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Path Point[%d]\r\n", (g_target_idx - 1U));
#endif
        return;
    }

    point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    if (fabsf(selected_err_deg) > NAV_YAW_TOLERANCE)
    {
        target_speed_set = NAV_SPEED_STOP;
        s_prev_speed_cmd = 0.0f;
        return;
    }

    speed_abs = GetOfflineSpeedAbs(&nav_ram_data.points[g_target_idx], is_special_point, nav_dist);
    target_speed_set = OfflineSpeedSlew(speed_sign * speed_abs);
    (void)selected_yaw_deg;
}

#endif
