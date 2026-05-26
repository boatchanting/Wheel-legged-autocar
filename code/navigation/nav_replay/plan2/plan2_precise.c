#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../plan/minefield.h"

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

#if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
#endif
}

void NavReplay_Process(void)
{
    float tx;
    float ty;
    float dx;
    float dy;
    float dist;
    float target_yaw;

    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1)
    {
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
        err_degree = 0.0f;
#if DEBUG_LOG_ENABLE
        printf("[Nav] Replay Finished.\r\n");
#endif
        return;
    }

    tx = nav_ram_data.points[g_target_idx].x;
    ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    dx = tx - inertial_nav.x;
    dy = ty - inertial_nav.y;
    dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    target_yaw = -atan2f(dy, -dx) * 57.29578f;
    err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    if (dist <= NAV_DIST_ARRIVE)
    {
        target_speed_set = NAV_SPEED_STOP;

#if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Point[%d] Type[%d]\r\n", g_target_idx, g_current_point_type);
#endif

        if (g_current_point_type != NAV_POINT_PATH)
        {
            if (g_current_point_type == NAV_POINT_CIRCLE)
            {
                minefield_flag = 1;
            }
            g_special_action_trigger = 1;
        }

        g_target_idx++;
        return;
    }

    if (fabsf(err_degree) > NAV_YAW_TOLERANCE)
    {
        target_speed_set = NAV_SPEED_STOP;
#if DEBUG_LOG_ENABLE
        printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
#endif
        return;
    }

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

#endif
