#include "nav_replay.h"
#include "../common.h"
#include "nav_replay_route_table.h"
#include "../config/sys_options.h"
#include "vision/vision_bridge_control.h"

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static uint8 g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
static void NavReplay_ResetProcessState(void);
#endif

#if CURRENT_NAV_PLAN == 3
static void NavReplay_ResetPlan3State(void);
#endif

static float NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static float CalcDistanceSq(float x1, float y1, float x2, float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
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

    nav_ram_data.plan_type = (uint8)CURRENT_NAV_PLAN;
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
    g_current_point_type = NAV_POINT_PATH;
    g_replay_state = REPLAY_RUNNING;
    g_special_action_trigger = 0;

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0 : 1;
    s_start_heading_stable_count = 0;
#else
    g_start_heading_aligned = 1;
#endif

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
    NavReplay_ResetProcessState();
#endif

#if CURRENT_NAV_PLAN == 3
    NavReplay_ResetPlan3State();
#endif

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay START. Plan: %d, Total Points: %d\r\n",
           nav_ram_data.plan_type, nav_ram_data.point_count);
#endif
}

void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_special_action_trigger = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
    NavReplay_ResetProcessState();
#endif

#if CURRENT_NAV_PLAN == 3
    NavReplay_ResetPlan3State();
#endif

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
#endif
}

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2

static float s_prev_err_degree = 0.0f;
static float s_prev_speed_set = 0.0f;
static uint8 s_prev_trigger = 0;
static uint8 s_stop_lock_active = 0;
static float s_stop_lock_yaw_deg = 0.0f;

#if IMU_CATEGORY == 3
#define NAV_START_ALIGN_MAX_ERR      25.0f
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

static void NavReplay_ClearStopLock(void)
{
    s_stop_lock_active = 0;
    s_stop_lock_yaw_deg = 0.0f;
}

static void NavReplay_ResetProcessState(void)
{
    s_prev_err_degree = 0.0f;
    s_prev_speed_set = 0.0f;
    s_prev_trigger = 0;
    NavReplay_ClearStopLock();
}

#if IMU_CATEGORY == 3
static uint8 NavReplay_HandleStartHeadingAlignment(void)
{
    float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
    float heading_cmd = heading_err;

    if (heading_cmd > NAV_START_ALIGN_MAX_ERR) heading_cmd = NAV_START_ALIGN_MAX_ERR;
    if (heading_cmd < -NAV_START_ALIGN_MAX_ERR) heading_cmd = -NAV_START_ALIGN_MAX_ERR;

    err_degree = heading_cmd;
    target_speed_set = NAV_SPEED_STOP;

    if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
    {
        if (s_start_heading_stable_count < NAV_START_ALIGN_STABLE_COUNT)
        {
            s_start_heading_stable_count++;
        }
    }
    else
    {
        s_start_heading_stable_count = 0;
    }

    if (s_start_heading_stable_count < NAV_START_ALIGN_STABLE_COUNT)
    {
        return 0;
    }

    g_start_heading_aligned = 1;
    s_start_heading_stable_count = 0;
    err_degree = 0.0f;
    target_speed_set = NAV_SPEED_STOP;
    return 1;
}

static void NavReplay_ResetLaunchPose(void)
{
    inertial_nav.x = 0.0f;
    inertial_nav.y = 0.0f;
    inertial_nav.vx_body = 0.0f;
    inertial_nav.vy_body = 0.0f;
    inertial_nav.slip_flag = 0;
    inertial_nav.relative_yaw = 0.0f;
    inertial_nav.init_yaw = euler_angle.yaw;

    g_target_idx = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0;

    NavReplay_ResetProcessState();
    err_degree = 0.0f;
    target_speed_set = NAV_SPEED_STOP;
}
#endif

static uint16 NavReplay_FindStopBarrierIndex(uint16 start_idx, uint16 search_range)
{
    uint16 i;
    uint16 last_idx;
    uint16 end_idx;

    if (nav_ram_data.point_count == 0)
    {
        return 0;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    end_idx = start_idx + search_range;
    if (end_idx > last_idx)
    {
        end_idx = last_idx;
    }

    for (i = start_idx; i <= end_idx; i++)
    {
        const NavRamPoint_t *point = &nav_ram_data.points[i];
        if (point->point_type == NAV_POINT_CIRCLE)
        {
            return i;
        }
        if (fabsf(point->target_speed) <= NAV_STOP_LOCK_SPEED_EPS)
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

static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int i;
    int closest_idx = current_idx;
    int end_idx;
    int barrier_idx;
    float min_dist_sq = 1e12f;

    if (nav_ram_data.point_count == 0)
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

    barrier_idx = (int)NavReplay_FindStopBarrierIndex((uint16)current_idx, (uint16)search_range);
    if (barrier_idx < end_idx)
    {
        end_idx = barrier_idx;
    }

    for (i = current_idx; i <= end_idx; i++)
    {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq)
        {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
    }

    if (!is_recovering && min_dist_sq > 800.0f * 800.0f)
    {
        return current_idx;
    }

    return closest_idx;
}

static void NavReplay_FindLookaheadTarget(uint16 base_idx, uint16 stop_idx, float lookahead_dist, float *tx, float *ty)
{
    uint16 i;
    float lookahead_dist_sq = lookahead_dist * lookahead_dist;

    *tx = nav_ram_data.points[stop_idx].x;
    *ty = nav_ram_data.points[stop_idx].y;

    for (i = base_idx; i <= stop_idx; i++)
    {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y,
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        *tx = nav_ram_data.points[i].x;
        *ty = nav_ram_data.points[i].y;
        if (d_sq >= lookahead_dist_sq)
        {
            break;
        }
    }
}

static void NavReplay_UpdateStopLock(uint16 stop_idx, float dist_to_stop)
{
    if (stop_idx < nav_ram_data.point_count &&
        fabsf(nav_ram_data.points[stop_idx].target_speed) <= NAV_STOP_LOCK_SPEED_EPS &&
        dist_to_stop < NAV_STOP_LOCK_DIST_MM)
    {
        if (!s_stop_lock_active)
        {
            s_stop_lock_active = 1;
            s_stop_lock_yaw_deg = inertial_nav.relative_yaw;
        }
    }
    else
    {
        NavReplay_ClearStopLock();
    }
}

void NavReplay_Process(void)
{
    int scan_range;
    int base_idx;
    uint8 is_recovering = 0;
    uint16 stop_idx;
    uint16 last_idx;
    float dist_to_stop;
    float base_spacing = 0.0f;
    float lookahead_min = PP_LD_MIN_CURVE;
    float lookahead_dist;
    float tx;
    float ty;
    float raw_err_degree;
    float diff;
    float raw_speed;

    if (g_replay_state != REPLAY_RUNNING)
    {
        return;
    }

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        if (!NavReplay_HandleStartHeadingAlignment())
        {
            return;
        }

        NavReplay_ResetLaunchPose();

#if DEBUG_LOG_ENABLE
        printf("[Nav] Start heading aligned, launch pose reset.\r\n");
#endif
        return;
    }
#endif

    if (g_special_action_trigger == 1)
    {
        s_prev_trigger = 1;
        NavReplay_ClearStopLock();
        return;
    }

    if (s_prev_trigger == 1)
    {
        is_recovering = 1;
        s_prev_trigger = 0;
        s_prev_err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        NavReplay_ClearStopLock();
    }

    scan_range = is_recovering ? 300 : 80;
    base_idx = Find_Closest_Point_Index_Strict((int)g_target_idx, scan_range, is_recovering);
    g_target_idx = (uint16)base_idx;

    if (nav_ram_data.point_count == 0)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    stop_idx = NavReplay_FindStopBarrierIndex((uint16)base_idx, (uint16)(nav_ram_data.point_count - 1U - (uint16)base_idx));
    dist_to_stop = CalcDistance(inertial_nav.x, inertial_nav.y,
                                nav_ram_data.points[stop_idx].x, nav_ram_data.points[stop_idx].y);

    if (stop_idx == last_idx && dist_to_stop <= NAV_DIST_ARRIVE)
    {
        g_replay_state = REPLAY_FINISHED;
        g_target_idx = stop_idx;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        NavReplay_ClearStopLock();
        return;
    }

#if CURRENT_NAV_PLAN == 2
    if (nav_ram_data.points[stop_idx].point_type == NAV_POINT_CIRCLE &&
        dist_to_stop <= NAV_DIST_ARRIVE)
    {
        g_current_point_type = NAV_POINT_CIRCLE;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        NavReplay_ClearStopLock();
        minefield_flag = 1;
        g_special_action_trigger = 1;
        s_prev_trigger = 1;
        if (stop_idx < last_idx)
        {
            g_target_idx = (uint16)(stop_idx + 1U);
        }
        else
        {
            g_target_idx = stop_idx;
        }
        return;
    }
#endif

    NavReplay_UpdateStopLock(stop_idx, dist_to_stop);

    if (base_idx + 1 < nav_ram_data.point_count)
    {
        base_spacing = CalcDistance(nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y,
                                    nav_ram_data.points[base_idx + 1].x, nav_ram_data.points[base_idx + 1].y);
    }
    else if (base_idx > 0)
    {
        base_spacing = CalcDistance(nav_ram_data.points[base_idx - 1].x, nav_ram_data.points[base_idx - 1].y,
                                    nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y);
    }

    if (base_spacing * PP_LD_MIN_STRAIGHT > lookahead_min)
    {
        lookahead_min = base_spacing * PP_LD_MIN_STRAIGHT;
    }

    lookahead_dist = lookahead_min + fabsf(s_prev_speed_set) * PP_LD_SPEED_GAIN;
    NavReplay_FindLookaheadTarget((uint16)base_idx, stop_idx, lookahead_dist, &tx, &ty);

    if (s_stop_lock_active)
    {
        err_degree = NormalizeAngle(s_stop_lock_yaw_deg - inertial_nav.relative_yaw);
        s_prev_err_degree = err_degree;
    }
    else
    {
        float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
        raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);
        diff = raw_err_degree - s_prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree - SLEW_RATE_ANGLE;
        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * s_prev_err_degree;
        s_prev_err_degree = err_degree;
    }

    raw_speed = nav_ram_data.points[base_idx].target_speed;
    if (s_stop_lock_active && raw_speed > NAV_SPEED_STOP)
    {
        raw_speed = NAV_SPEED_STOP;
    }

    target_speed_set = FILTER_ALPHA_SPEED * raw_speed + (1.0f - FILTER_ALPHA_SPEED) * s_prev_speed_set;
    s_prev_speed_set = target_speed_set;
    g_current_point_type = nav_ram_data.points[base_idx].point_type;
}
#endif

#if CURRENT_NAV_PLAN == 3
#define MAX_SPIN_ERR        2.0f
#define MAX_APPROACH_ERR    4.0f
#define ANGLE_FILTER_ALPHA  0.3f

static float s_plan3_prev_err_degree = 0.0f;
static uint8 s_plan3_is_aligning = 0;

static void NavReplay_ResetPlan3State(void)
{
    s_plan3_prev_err_degree = 0.0f;
    s_plan3_is_aligning = 0;
}

void NavReplay_Process(void)
{
    float tx;
    float ty;
    float dx;
    float dy;
    float dist;
    float target_yaw;
    float raw_err;

    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1)
    {
        NavReplay_ResetPlan3State();
        return;
    }

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);

        if (heading_err > MAX_SPIN_ERR) heading_err = MAX_SPIN_ERR;
        if (heading_err < -MAX_SPIN_ERR) heading_err = -MAX_SPIN_ERR;

        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading)) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
            err_degree = 0.0f;
            s_plan3_prev_err_degree = 0.0f;
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
        err_degree = 0.0f;
        s_plan3_is_aligning = 0;
        return;
    }

    tx = nav_ram_data.points[g_target_idx].x;
    ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    dx = tx - inertial_nav.x;
    dy = ty - inertial_nav.y;
    dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    target_yaw = -atan2f(dy, -dx) * 57.29578f;
    raw_err = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    if (dist <= NAV_DIST_ARRIVE || s_plan3_is_aligning)
    {
        target_speed_set = NAV_SPEED_STOP;

        if (g_current_point_type != NAV_POINT_PATH)
        {
            float special_target_yaw;
            float special_yaw_err;

            s_plan3_is_aligning = 1;
            special_target_yaw = nav_ram_data.points[g_target_idx].target_yaw_deg;
            special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);

            if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
            {
                if (special_yaw_err > MAX_SPIN_ERR) special_yaw_err = MAX_SPIN_ERR;
                if (special_yaw_err < -MAX_SPIN_ERR) special_yaw_err = -MAX_SPIN_ERR;

                err_degree = special_yaw_err;
                s_plan3_prev_err_degree = err_degree;
            }
            else
            {
                if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1;
                else if (g_current_point_type == NAV_POINT_JUMP) vision_detected_three_jump_point = 1;
                else if (g_current_point_type == NAV_POINT_BRIDGE) VisionBridgeTask_Start();
                else if (g_current_point_type == NAV_POINT_BUMP) BumpyRoad_Trigger();

                g_special_action_trigger = 1;
                g_target_idx++;
                NavReplay_ResetPlan3State();
            }
        }
        else
        {
            g_target_idx++;
            s_plan3_is_aligning = 0;
        }
    }
    else
    {
        if (dist < NAV_DIST_ARRIVE + 150.0f)
        {
            if (raw_err > 15.0f) raw_err = 15.0f;
            if (raw_err < -15.0f) raw_err = -15.0f;
        }
        else
        {
            if (raw_err > MAX_APPROACH_ERR) raw_err = MAX_APPROACH_ERR;
            if (raw_err < -MAX_APPROACH_ERR) raw_err = -MAX_APPROACH_ERR;
        }

        err_degree = ANGLE_FILTER_ALPHA * raw_err + (1.0f - ANGLE_FILTER_ALPHA) * s_plan3_prev_err_degree;
        s_plan3_prev_err_degree = err_degree;

        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
        {
            target_speed_set = NAV_SPEED_STOP;
        }
        else
        {
            if (dist > NAV_DIST_FAR)
            {
                target_speed_set = NAV_SPEED_FAST;
            }
            else if (dist > NAV_DIST_NEAR)
            {
                float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
            }
            else
            {
                target_speed_set = NAV_SPEED_SLOW;
            }
        }
    }
}
#endif
