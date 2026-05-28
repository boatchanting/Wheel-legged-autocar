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

static uint16 g_target_idx = 0;
static uint8 g_start_heading_aligned = 1;
static uint8 s_stop_stable_ticks = 0U;
static uint8 s_stop_yaw_locked = 0U;
static float s_stop_yaw_deg = 0.0f;
static float s_prev_speed_cmd = 0.0f;

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

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) || (point_type == NAV_POINT_JUMP));
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

static void ResetStopState(void)
{
    s_stop_stable_ticks = 0U;
    s_stop_yaw_locked = 0U;
    s_stop_yaw_deg = 0.0f;
}

// 对目标速度做单周期限斜率，避免在线规划输出突跳导致底盘顿挫。
static float SpeedSlew(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_POINT_SPEED_ACCEL_STEP;

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

// 为当前特殊点规划“旋转总角度 + 退出朝向 + 旋转方向”。
// 这里会同时比较顺时针/逆时针、正向出框/反向出框四种组合，选总角度最小的一种。
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

            while (total_angle < NAV_POINT_SPIN_MIN_TOTAL_ANGLE)
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

// 在“正向朝向目标点”和“反向朝向目标点”之间自动选择误差更小的一侧，
// 这样车可以前进或倒退接近中心点，减少原地大幅转向的时间。
static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

    if ((fabsf(err_reverse) + NAV_POINT_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
    {
        *selected_err_deg = err_reverse;
        *speed_sign = 1.0f;
    }
    else
    {
        *selected_err_deg = err_forward;
        *speed_sign = -1.0f;
    }
}

// 按“距离停车边界还剩多少”实时规划允许速度上限。
// 这个函数是方案4在线规划的核心：不查离线路表，只看当前剩余距离和姿态误差。
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float remain = dist_mm - stop_radius_mm;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    // v^2 = 2ad：这里的 v 是底盘目标速度指令绝对值，a 是“指令域减速度”。
    speed_abs = sqrtf(2.0f * NAV_POINT_SPEED_DECEL_CMD2_PER_MM * remain);
    speed_abs = Float_Constrain(speed_abs, 0.0f, fabsf(NAV_POINT_SPEED_FAST));

    if ((speed_abs < fabsf(NAV_POINT_SPEED_SLOW)) && (remain > NAV_POINT_PATH_ARRIVE_RADIUS))
    {
        speed_abs = fabsf(NAV_POINT_SPEED_SLOW);
    }

    // 角度偏差较大时先停车转向；中等偏差时降速逼近，防止车轮带着横向误差冲进白框。
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

// 统一处理雷区点/终点的“停稳判定 -> 触发动作”流程。
// 返回 1 表示本周期已经完成触发，上层应停止继续做跟踪控制。
static uint8 HandleStopAndTrigger(uint16 point_idx, uint8 point_type, float dist_to_point)
{
    float stop_radius = IsSpecialPointType(point_type) ? NAV_POINT_SPECIAL_STOP_RADIUS : NAV_POINT_FINAL_STOP_RADIUS;

    if (dist_to_point > stop_radius)
    {
        ResetStopState();
        return 0U;
    }

    if (s_stop_yaw_locked == 0U)
    {
        s_stop_yaw_locked = 1U;
        s_stop_yaw_deg = inertial_nav.relative_yaw;
    }

    target_speed_set = NAV_POINT_SPEED_STOP;
    s_prev_speed_cmd = 0.0f;
    err_degree = NormalizeAngle(s_stop_yaw_deg - inertial_nav.relative_yaw);

    if (fabsf(inertial_nav.vx_body) <= NAV_POINT_STOP_SPEED_MM_S)
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

    if (s_stop_stable_ticks < NAV_POINT_STOP_STABLE_TICKS)
    {
        return 0U;
    }

    if (IsSpinPointType(point_type))
    {
        ConfigureSpinPlanForPoint(point_idx);
        minefield_flag = 1;
    }

    g_special_action_trigger = IsSpecialPointType(point_type) ? 1U : 0U;
    ResetStopState();
    Brake_MinefieldThunderBrake_Reset();
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
    Minefield_Init();
    Brake_MinefieldThunderBrake_Reset();

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
    Minefield_Init();
    Brake_MinefieldThunderBrake_Reset();
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
        Brake_MinefieldThunderBrake_Reset();
        return;
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
        Brake_MinefieldThunderBrake_Reset();
        return;
    }

    point = &nav_ram_data.points[g_target_idx];
    tx = point->x;
    ty = point->y;
    point_type = point->point_type;
    is_last_point = (uint8)(g_target_idx >= (uint16)(nav_ram_data.point_count - 1U));
    g_current_point_type = point_type;

    dist_to_point = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    if (point_type == NAV_POINT_CIRCLE)
    {
        Brake_MinefieldThunderBrake_Update(1U, dist_to_point);
    }
    else
    {
        Brake_MinefieldThunderBrake_Reset();
    }

    if ((point_type == NAV_POINT_PATH) && (is_last_point == 0U) &&
        (dist_to_point <= NAV_POINT_PATH_ARRIVE_RADIUS))
    {
        // 普通过渡点只推进索引，不做停车动作；停车只留给雷区点和终点。
        g_target_idx++;
        ResetStopState();
        return;
    }

    if ((IsSpecialPointType(point_type) || is_last_point) &&
        HandleStopAndTrigger(g_target_idx, point_type, dist_to_point))
    {
        if (g_target_idx < (uint16)(nav_ram_data.point_count - 1U))
        {
            g_target_idx++;
        }
        else
        {
            g_replay_state = REPLAY_FINISHED;
        }
        return;
    }

    // 点对点终端制导：每周期重算“当前位置 -> 目标点”的朝向，并自动选择前进/倒车误差较小的一侧。
    point_yaw_deg = CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_err_deg, &speed_sign);
    err_degree = selected_err_deg;

    stop_radius = IsSpecialPointType(point_type) ? NAV_POINT_SPECIAL_STOP_RADIUS : NAV_POINT_PATH_ARRIVE_RADIUS;
    if (is_last_point != 0U)
    {
        stop_radius = NAV_POINT_FINAL_STOP_RADIUS;
    }

    speed_abs = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);

#if NAV_PLAN2_SPECIAL_APPROACH_MODE == PLAN2_SPECIAL_APPROACH_CENTER_RELAXED
    // 宽松模式下，特殊点进入共享宽松窗口后提前压到慢速，
    // 让方案4更容易在中心停车区内停稳，减少高速逼近时错过中心点的概率。
    if (IsSpecialPointType(point_type) &&
        (dist_to_point <= NAV_PLAN2_SPECIAL_RELAX_APPROACH_WINDOW_MM) &&
        (speed_abs > fabsf(NAV_POINT_SPEED_SLOW)))
    {
        speed_abs = fabsf(NAV_POINT_SPEED_SLOW);
    }
#endif

    target_speed_set = SpeedSlew(speed_sign * speed_abs);
}

#endif
