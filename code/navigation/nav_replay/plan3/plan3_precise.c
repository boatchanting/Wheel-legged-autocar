#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"
#include "../../../plan/bumpy_road.h"
#include "../../../servo/servo_jump.h"
#include "../../../vision/vision_bridge_control.h"

#if (CURRENT_NAV_PLAN == 3) && (NAV_PLAN3_METHOD == PLAN3_METHOD_PRECISE)

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0U;

static uint16 g_target_idx = 0U;
static uint8 g_start_heading_aligned = 1U;
static uint8 s_is_aligning_special_point = 0U;
static float s_prev_err_degree = 0.0f;

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

static void ResetPlan3ProcessState(void)
{
    s_is_aligning_special_point = 0U;
    s_prev_err_degree = 0.0f;
}

static void TriggerSpecialAction(void)
{
    if (g_current_point_type == NAV_POINT_CIRCLE)
        minefield_flag = 1U;
    else if (g_current_point_type == NAV_POINT_JUMP)
        vision_detected_three_jump_point = true;
    else if (g_current_point_type == NAV_POINT_BRIDGE)
        VisionBridgeTask_Start();
    else if (g_current_point_type == NAV_POINT_BUMP)
        BumpyRoad_Trigger();
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

    for (i = 0U; i < load_count; i++)
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

    if (nav_ram_data.point_count == 0U)
    {
        return;
    }

    g_target_idx = 0U;
    g_replay_state = REPLAY_RUNNING;
    g_current_point_type = NAV_POINT_PATH;
    g_special_action_trigger = 0U;
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    ResetPlan3ProcessState();

#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0U : 1U;
#else
    g_start_heading_aligned = 1U;
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
    ResetPlan3ProcessState();
}

void NavReplay_Process(void)
{
    float tx;
    float ty;
    float dist_to_target;
    float target_yaw;
    float raw_err_degree;

    if ((g_replay_state != REPLAY_RUNNING) || (g_special_action_trigger != 0U))
    {
        ResetPlan3ProcessState();
        return;
    }

#if IMU_CATEGORY == 3
    if (g_start_heading_aligned == 0U)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);

        if (heading_err > PLAN3_MAX_SPIN_ERR_DEG) heading_err = PLAN3_MAX_SPIN_ERR_DEG;
        if (heading_err < -PLAN3_MAX_SPIN_ERR_DEG) heading_err = -PLAN3_MAX_SPIN_ERR_DEG;

        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading)) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1U;
            err_degree = 0.0f;
            s_prev_err_degree = 0.0f;
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
        ResetPlan3ProcessState();
        return;
    }

    tx = nav_ram_data.points[g_target_idx].x;
    ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;
    dist_to_target = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);
    target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
    raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    if ((dist_to_target <= NAV_DIST_ARRIVE) || (s_is_aligning_special_point != 0U))
    {
        target_speed_set = NAV_SPEED_STOP;

        if (g_current_point_type == NAV_POINT_PATH)
        {
            g_target_idx++;
            s_is_aligning_special_point = 0U;
            return;
        }

        s_is_aligning_special_point = 1U;
        raw_err_degree = NormalizeAngle(nav_ram_data.points[g_target_idx].target_yaw_deg -
                                        inertial_nav.relative_yaw);

        if (fabsf(raw_err_degree) > NAV_YAW_TOLERANCE)
        {
            if (raw_err_degree > PLAN3_MAX_SPIN_ERR_DEG) raw_err_degree = PLAN3_MAX_SPIN_ERR_DEG;
            if (raw_err_degree < -PLAN3_MAX_SPIN_ERR_DEG) raw_err_degree = -PLAN3_MAX_SPIN_ERR_DEG;

            err_degree = raw_err_degree;
            s_prev_err_degree = err_degree;
            return;
        }

        TriggerSpecialAction();
        g_special_action_trigger = 1U;
        g_target_idx++;
        ResetPlan3ProcessState();
        return;
    }

    // 靠近目标点时限制方向误差，避免小车越过点后大幅反向修正。
    if (dist_to_target < (NAV_DIST_ARRIVE + PLAN3_NEAR_POINT_ERROR_WINDOW))
    {
        if (raw_err_degree > PLAN3_NEAR_POINT_MAX_ERR_DEG) raw_err_degree = PLAN3_NEAR_POINT_MAX_ERR_DEG;
        if (raw_err_degree < -PLAN3_NEAR_POINT_MAX_ERR_DEG) raw_err_degree = -PLAN3_NEAR_POINT_MAX_ERR_DEG;
    }
    else
    {
        if (raw_err_degree > PLAN3_MAX_APPROACH_ERR_DEG) raw_err_degree = PLAN3_MAX_APPROACH_ERR_DEG;
        if (raw_err_degree < -PLAN3_MAX_APPROACH_ERR_DEG) raw_err_degree = -PLAN3_MAX_APPROACH_ERR_DEG;
    }

    err_degree = PLAN3_ANGLE_FILTER_ALPHA * raw_err_degree +
                 (1.0f - PLAN3_ANGLE_FILTER_ALPHA) * s_prev_err_degree;
    s_prev_err_degree = err_degree;

    if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
    {
        target_speed_set = NAV_SPEED_STOP;
    }
    else if (dist_to_target > NAV_DIST_FAR)
    {
        target_speed_set = NAV_SPEED_FAST;
    }
    else if (dist_to_target > NAV_DIST_NEAR)
    {
        float ratio = (dist_to_target - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
        target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
    }
    else
    {
        target_speed_set = NAV_SPEED_SLOW;
    }
}

#endif
