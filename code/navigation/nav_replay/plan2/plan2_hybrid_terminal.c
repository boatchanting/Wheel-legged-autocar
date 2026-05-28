#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../calculate/pid-new.h"

#if (CURRENT_NAV_PLAN == 2) && (NAV_PLAN2_METHOD == PLAN2_HYBRID_TERMINAL)

extern volatile float target_speed_set;
extern volatile float err_degree;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;

static uint16 g_target_idx = 0;
static uint8 g_start_heading_aligned = 1U;
static uint8 s_prev_special_takeover = 0U;
static uint8 s_stop_stable_ticks = 0U;
static uint8 s_stop_yaw_locked = 0U;
static float s_stop_yaw_deg = 0.0f;
static float s_prev_speed_cmd = 0.0f;
static float s_prev_err_degree = 0.0f;

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

static float CalcDistanceSq(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static float CalcBearingDeg(float x1, float y1, float x2, float y2)
{
    return -atan2f(y2 - y1, -(x2 - x1)) * 57.29578f;
}

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

static uint8 IsSpinPointType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) || (point_type == NAV_POINT_JUMP));
}

static void ResetStopState(void)
{
    s_stop_stable_ticks = 0U;
    s_stop_yaw_locked = 0U;
    s_stop_yaw_deg = 0.0f;
}

// 速度斜率限制：让远距离 PP/LOS 切终端点对点时，速度变化不要过于生硬。
static float SpeedSlew(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_HYBRID_SPEED_ACCEL_STEP;

    if ((raw_speed * s_prev_speed_cmd) < 0.0f)
    {
        step_limit = NAV_HYBRID_SPEED_CROSS_ZERO_STEP;
    }
    else if (fabsf(raw_speed) < fabsf(s_prev_speed_cmd))
    {
        step_limit = NAV_HYBRID_SPEED_DECEL_STEP;
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

// 搜索“停止屏障”点：圆环点或终点都视作当前不能看穿的停车边界。
// 纯追踪/LOS 的前瞻点会被限制在这个屏障之前，避免直接把目标看到雷区后面。
static uint16 FindStopBarrierIndex(uint16 start_idx, uint16 search_range)
{
    uint16 i;
    uint16 last_idx;
    uint16 end_idx;

    if (nav_ram_data.point_count == 0U)
    {
        return 0U;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    end_idx = start_idx + search_range;
    if (end_idx > last_idx)
    {
        end_idx = last_idx;
    }

    for (i = start_idx; i <= end_idx; i++)
    {
        if (nav_ram_data.points[i].point_type == NAV_POINT_CIRCLE)
        {
            return i;
        }

        if (i == last_idx)
        {
            return i;
        }
    }

    return end_idx;
}

// 最近点恢复：只允许索引向前搜索，避免特殊动作恢复后跳回已经跑过的历史点。
static int FindClosestPointIndexStrict(int current_idx, int search_range, uint8 is_recovering)
{
    int i;
    int closest_idx = current_idx;
    int end_idx;
    int barrier_idx;
    float min_dist_sq = 1e12f;

    if (nav_ram_data.point_count == 0U)
    {
        return 0;
    }

    if (current_idx < 0)
    {
        current_idx = 0;
    }
    if (current_idx >= nav_ram_data.point_count)
    {
        current_idx = nav_ram_data.point_count - 1;
    }

    end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count)
    {
        end_idx = nav_ram_data.point_count - 1;
    }

    barrier_idx = (int)FindStopBarrierIndex((uint16)current_idx, (uint16)search_range);
    if (barrier_idx < end_idx)
    {
        end_idx = barrier_idx;
    }

    for (i = current_idx; i <= end_idx; i++)
    {
        float dist_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                       nav_ram_data.points[i].x,
                                       nav_ram_data.points[i].y);
        if (dist_sq < min_dist_sq)
        {
            min_dist_sq = dist_sq;
            closest_idx = i;
        }
    }

    if ((is_recovering == 0U) && (min_dist_sq > 800.0f * 800.0f))
    {
        return current_idx;
    }

    return closest_idx;
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

// 为雷区点规划旋转总角度和退出朝向；和方案4相同，统一选四种组合中代价最小的一种。
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

            while (total_angle < NAV_HYBRID_SPIN_MIN_TOTAL_ANGLE)
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

static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    float err_forward = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float reverse_yaw = NormalizeAngle(point_yaw_deg + 180.0f);
    float err_reverse = NormalizeAngle(reverse_yaw - inertial_nav.relative_yaw);

    if ((fabsf(err_reverse) + NAV_HYBRID_REVERSE_SELECT_BIAS_DEG) < fabsf(err_forward))
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

// 按“到停止屏障的剩余距离”规划速度，保证远距离跟踪不会把停车区看穿。
static float PlanSpeedAbsToStop(float dist_to_stop, float stop_radius, float yaw_err_deg)
{
    float remain = dist_to_stop - stop_radius;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    // 按“距离停止屏障还有多少”规划速度，确保纯追踪/LOS 不把前瞻点看穿雷区中心。
    speed_abs = sqrtf(2.0f * NAV_HYBRID_SPEED_DECEL_CMD2_PER_MM * remain);
    speed_abs = Float_Constrain(speed_abs, 0.0f, fabsf(NAV_HYBRID_SPEED_FAST));

    if ((speed_abs < fabsf(NAV_HYBRID_SPEED_SLOW)) && (remain > NAV_HYBRID_PATH_PASS_RADIUS))
    {
        speed_abs = fabsf(NAV_HYBRID_SPEED_SLOW);
    }

    // 终端段要优先保证进框姿态：大偏差停车，中等偏差慢走，小偏差按距离规划速度。
    if (fabsf(yaw_err_deg) > NAV_HYBRID_TERMINAL_YAW_SLOW_TOL)
    {
        speed_abs = 0.0f;
    }
    else if (fabsf(yaw_err_deg) > NAV_HYBRID_TERMINAL_YAW_STOP_TOL)
    {
        speed_abs *= 0.35f;
    }

    return speed_abs;
}

// 方案5远距离纯追踪分支：在 [base_idx, stop_idx] 内找前瞻目标并输出转向误差。
static float PurePursuitErrDegree(uint16 base_idx, uint16 stop_idx)
{
    uint16 i;
    float lookahead_dist = NAV_HYBRID_PP_LD_MIN + fabsf(s_prev_speed_cmd) * NAV_HYBRID_PP_LD_SPEED_GAIN;
    float lookahead_sq = lookahead_dist * lookahead_dist;
    float tx = nav_ram_data.points[stop_idx].x;
    float ty = nav_ram_data.points[stop_idx].y;

    for (i = base_idx; i <= stop_idx; i++)
    {
        float dist_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                       nav_ram_data.points[i].x,
                                       nav_ram_data.points[i].y);
        tx = nav_ram_data.points[i].x;
        ty = nav_ram_data.points[i].y;
        if (dist_sq >= lookahead_sq)
        {
            break;
        }
    }

    return NormalizeAngle(CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty) -
                          inertial_nav.relative_yaw);
}

// 方案5远距离 LOS 分支：在当前路径线段上做投影，再沿线段向前看一个 LOS 距离。
static float LosErrDegree(uint16 base_idx, uint16 stop_idx)
{
    uint16 next_idx = (uint16)(base_idx + 1U);
    float x0;
    float y0;
    float x1;
    float y1;
    float vx;
    float vy;
    float seg_len;
    float ux;
    float uy;
    float proj_len;
    float los_len;
    float tx;
    float ty;

    if (next_idx > stop_idx)
    {
        next_idx = stop_idx;
    }
    if (next_idx >= nav_ram_data.point_count)
    {
        next_idx = (uint16)(nav_ram_data.point_count - 1U);
    }

    x0 = nav_ram_data.points[base_idx].x;
    y0 = nav_ram_data.points[base_idx].y;
    x1 = nav_ram_data.points[next_idx].x;
    y1 = nav_ram_data.points[next_idx].y;
    vx = x1 - x0;
    vy = y1 - y0;
    seg_len = sqrtf(vx * vx + vy * vy);

    if (seg_len < 1.0f)
    {
        return NormalizeAngle(CalcBearingDeg(inertial_nav.x, inertial_nav.y,
                                             nav_ram_data.points[stop_idx].x,
                                             nav_ram_data.points[stop_idx].y) -
                              inertial_nav.relative_yaw);
    }

    ux = vx / seg_len;
    uy = vy / seg_len;
    proj_len = (inertial_nav.x - x0) * ux + (inertial_nav.y - y0) * uy;
    proj_len = Float_Constrain(proj_len, 0.0f, seg_len);
    los_len = proj_len + NAV_HYBRID_LOS_LOOKAHEAD;
    los_len = Float_Constrain(los_len, 0.0f, seg_len);
    tx = x0 + ux * los_len;
    ty = y0 + uy * los_len;

    return NormalizeAngle(CalcBearingDeg(inertial_nav.x, inertial_nav.y, tx, ty) -
                          inertial_nav.relative_yaw);
}

// 近距离终端停车与动作触发流程；返回 1 表示已经停稳并触发动作。
static uint8 HandleTerminalStop(uint16 point_idx, uint8 point_type, float dist_to_point)
{
    float stop_radius = IsSpecialPointType(point_type) ? NAV_HYBRID_SPECIAL_STOP_RADIUS : NAV_HYBRID_FINAL_STOP_RADIUS;

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

    target_speed_set = NAV_HYBRID_SPEED_STOP;
    s_prev_speed_cmd = 0.0f;
    err_degree = NormalizeAngle(s_stop_yaw_deg - inertial_nav.relative_yaw);

    if (fabsf(inertial_nav.vx_body) <= NAV_HYBRID_STOP_SPEED_MM_S)
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

    if (s_stop_stable_ticks < NAV_HYBRID_STOP_STABLE_TICKS)
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
    target_speed_set = NAV_HYBRID_SPEED_STOP;
    err_degree = 0.0f;
    s_prev_special_takeover = 0U;
    s_prev_speed_cmd = 0.0f;
    s_prev_err_degree = 0.0f;
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
    target_speed_set = NAV_HYBRID_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    g_start_heading_aligned = 1U;
    s_prev_special_takeover = 0U;
    s_prev_speed_cmd = 0.0f;
    s_prev_err_degree = 0.0f;
    ResetStopState();
    Minefield_Init();
    Brake_MinefieldThunderBrake_Reset();
}

void NavReplay_Process(void)
{
    int scan_range;
    int base_idx;
    uint8 is_recovering = 0U;
    uint16 stop_idx;
    uint16 last_idx;
    uint8 stop_type;
    float dist_to_stop;
    float raw_err;
    float diff;
    float speed_abs;

    if (g_replay_state != REPLAY_RUNNING)
    {
        return;
    }

#if IMU_CATEGORY == 3
    if (g_start_heading_aligned == 0U)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_HYBRID_SPEED_STOP;
        if (fabsf(heading_err) <= NAV_HYBRID_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1U;
            err_degree = 0.0f;
        }
        return;
    }
#endif

    if (g_special_action_trigger != 0U)
    {
        s_prev_special_takeover = 1U;
        Brake_MinefieldThunderBrake_Reset();
        return;
    }

    if (s_prev_special_takeover != 0U)
    {
        is_recovering = 1U;
        s_prev_special_takeover = 0U;
        s_prev_speed_cmd = 0.0f;
        s_prev_err_degree = 0.0f;
        ResetStopState();
    }

    if (nav_ram_data.point_count == 0U)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_HYBRID_SPEED_STOP;
        err_degree = 0.0f;
        Brake_MinefieldThunderBrake_Reset();
        return;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    scan_range = (is_recovering != 0U) ? (int)NAV_HYBRID_RECOVER_SCAN_RANGE : (int)NAV_HYBRID_SCAN_RANGE;
    base_idx = FindClosestPointIndexStrict((int)g_target_idx, scan_range, is_recovering);
    g_target_idx = (uint16)base_idx;

    stop_idx = FindStopBarrierIndex((uint16)base_idx, (uint16)(last_idx - (uint16)base_idx));
    stop_type = nav_ram_data.points[stop_idx].point_type;
    dist_to_stop = CalcDistance(inertial_nav.x, inertial_nav.y,
                                nav_ram_data.points[stop_idx].x,
                                nav_ram_data.points[stop_idx].y);

    if (stop_type == NAV_POINT_CIRCLE)
    {
        Brake_MinefieldThunderBrake_Update(1U, dist_to_stop);
    }
    else
    {
        Brake_MinefieldThunderBrake_Reset();
    }

    if ((stop_type == NAV_POINT_CIRCLE) && (dist_to_stop <= NAV_HYBRID_TERMINAL_DIST))
    {
        float point_yaw = CalcBearingDeg(inertial_nav.x, inertial_nav.y,
                                         nav_ram_data.points[stop_idx].x,
                                         nav_ram_data.points[stop_idx].y);
        float selected_err;
        float speed_sign;
        SelectDriveHeading(point_yaw, &selected_err, &speed_sign);
        err_degree = selected_err;

        if (HandleTerminalStop(stop_idx, stop_type, dist_to_stop))
        {
            g_target_idx = (stop_idx < last_idx) ? (uint16)(stop_idx + 1U) : stop_idx;
            return;
        }

        speed_abs = PlanSpeedAbsToStop(dist_to_stop, NAV_HYBRID_SPECIAL_STOP_RADIUS, selected_err);
        target_speed_set = SpeedSlew(speed_sign * speed_abs);
        s_prev_err_degree = err_degree;
        return;
    }

    if ((stop_idx == last_idx) && (dist_to_stop <= NAV_HYBRID_FINAL_STOP_RADIUS))
    {
        if (HandleTerminalStop(stop_idx, stop_type, dist_to_stop))
        {
            g_replay_state = REPLAY_FINISHED;
        }
        return;
    }

#if NAV_PLAN2_HYBRID_GUIDE_MODE == PLAN2_HYBRID_GUIDE_LOS
    // 远距离可编译期切到 LOS；LOS 更贴线，适合点列比较直、希望转向更平顺的路表。
    raw_err = LosErrDegree((uint16)base_idx, stop_idx);
#else
    // 默认远距离使用纯追踪；容错更强，适合打点间距不完全均匀的实车路线。
    raw_err = PurePursuitErrDegree((uint16)base_idx, stop_idx);
#endif

    diff = raw_err - s_prev_err_degree;
    if (diff > 35.0f) raw_err = s_prev_err_degree + 35.0f;
    else if (diff < -35.0f) raw_err = s_prev_err_degree - 35.0f;
    err_degree = 0.5f * raw_err + 0.5f * s_prev_err_degree;
    s_prev_err_degree = err_degree;

    speed_abs = PlanSpeedAbsToStop(dist_to_stop,
                                   (stop_type == NAV_POINT_CIRCLE) ? NAV_HYBRID_SPECIAL_STOP_RADIUS : NAV_HYBRID_FINAL_STOP_RADIUS,
                                   0.0f);
    target_speed_set = SpeedSlew(-speed_abs);
    g_current_point_type = nav_ram_data.points[base_idx].point_type;
}

#endif
