#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"

#if (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_LQR_TRACKING)

extern volatile float target_speed_set;
extern volatile float err_degree;

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

#define LQR_DEG_TO_RAD 0.0174532925f

typedef struct
{
    float x;
    float y;
    float yaw_deg;
    float curvature;
    float target_speed;
    uint8 point_type;
    uint16 idx;
} LqrReference_t;

static uint8 g_start_heading_aligned = 1;
static float s_prev_err_degree = 0.0f;
static float s_prev_speed_set = 0.0f;
static uint8 s_prev_trigger = 0;

#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#define NAV_START_ALIGN_MAX_ERR      25.0f
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

static void NavReplay_ResetProcessState(void);

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
#if GNSS_NAV == 1
    GpsNavReplay_Stop();
#endif

#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif

    if (nav_ram_data.point_count == 0)
    {
#if DEBUG_LOG_ENABLE
        printf("[Nav-LQR] RAM is empty, cannot start replay.\r\n");
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

    NavReplay_ResetProcessState();

#if DEBUG_LOG_ENABLE
    printf("[Nav-LQR] Replay START. Plan: %d, Total Points: %d\r\n",
           nav_ram_data.plan_type, nav_ram_data.point_count);
#endif
}

void NavReplay_Stop(void)
{
    target_speed_set = NAV_SPEED_STOP;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_special_action_trigger = 0;
    g_current_point_type = NAV_POINT_PATH;
    g_start_heading_aligned = 1;

#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

    NavReplay_ResetProcessState();

#if DEBUG_LOG_ENABLE
    printf("[Nav-LQR] Replay STOPPED.\r\n");
#endif
}

static void NavReplay_ResetProcessState(void)
{
    s_prev_err_degree = 0.0f;
    s_prev_speed_set = 0.0f;
    s_prev_trigger = 0;
}

static float NavReplay_SpeedSlew_Update(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(s_prev_speed_set);
    float diff = raw_speed - s_prev_speed_set;
    float step_limit;

    if (((raw_speed * s_prev_speed_set) >= 0.0f) &&
        (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS)))
    {
        return raw_speed;
    }

    if ((raw_speed * s_prev_speed_set) < 0.0f)
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

    return s_prev_speed_set + Float_Constrain(diff, -step_limit, step_limit);
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
    if (start_idx > last_idx)
    {
        start_idx = last_idx;
    }

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
    int closest_idx;
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

    closest_idx = current_idx;
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

    if (!is_recovering && min_dist_sq > (LQR_MAX_TRACK_DIST_MM * LQR_MAX_TRACK_DIST_MM))
    {
        return current_idx;
    }

    return closest_idx;
}

static uint16 NavReplay_GetPreviewIndex(uint16 base_idx, uint16 stop_idx)
{
    uint16 last_idx;
    uint16 ref_idx;

    if (nav_ram_data.point_count == 0)
    {
        return 0;
    }

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    if (base_idx > last_idx)
    {
        base_idx = last_idx;
    }
    if (stop_idx < base_idx)
    {
        stop_idx = base_idx;
    }
    if (stop_idx > last_idx)
    {
        stop_idx = last_idx;
    }

    ref_idx = (uint16)(base_idx + LQR_PREVIEW_POINTS);
    if (ref_idx > stop_idx)
    {
        ref_idx = stop_idx;
    }
    if (ref_idx > last_idx)
    {
        ref_idx = last_idx;
    }

    return ref_idx;
}

static void NavReplay_BuildReference(uint16 base_idx, uint16 stop_idx, LqrReference_t *ref)
{
    uint16 last_idx = (uint16)(nav_ram_data.point_count - 1U);
    uint16 seg_end_idx;
    uint16 ref_idx;
    const NavRamPoint_t *base;
    const NavRamPoint_t *seg_end;
    const NavRamPoint_t *preview;
    float seg_dx;
    float seg_dy;
    float seg_len_sq;
    float proj_t = 0.0f;

    if (base_idx > last_idx)
    {
        base_idx = last_idx;
    }
    if (stop_idx < base_idx)
    {
        stop_idx = base_idx;
    }

    seg_end_idx = base_idx;
    if ((base_idx < stop_idx) && ((uint16)(base_idx + 1U) <= last_idx))
    {
        seg_end_idx = (uint16)(base_idx + 1U);
    }

    ref_idx = NavReplay_GetPreviewIndex(base_idx, stop_idx);
    base = &nav_ram_data.points[base_idx];
    seg_end = &nav_ram_data.points[seg_end_idx];
    preview = &nav_ram_data.points[ref_idx];

    seg_dx = seg_end->x - base->x;
    seg_dy = seg_end->y - base->y;
    seg_len_sq = seg_dx * seg_dx + seg_dy * seg_dy;

    if (seg_len_sq > (LQR_PROJECTION_MIN_SEG_LEN_MM * LQR_PROJECTION_MIN_SEG_LEN_MM))
    {
        float car_dx = inertial_nav.x - base->x;
        float car_dy = inertial_nav.y - base->y;
        proj_t = (car_dx * seg_dx + car_dy * seg_dy) / seg_len_sq;
        proj_t = Float_Constrain(proj_t, 0.0f, 1.0f);
    }

    ref->x = base->x + proj_t * seg_dx;
    ref->y = base->y + proj_t * seg_dy;
    ref->yaw_deg = preview->target_yaw_deg;
    ref->curvature = preview->curvature;
    ref->target_speed = base->target_speed;
    ref->point_type = base->point_type;
    ref->idx = ref_idx;
}

static float NavReplay_GetCurvatureDirectionSign(float target_speed)
{
#if LQR_CURVATURE_SPEED_SIGN_ENABLE
    if (fabsf(target_speed) <= NAV_STOP_LOCK_SPEED_EPS)
    {
        return 1.0f;
    }
#if LQR_FORWARD_SPEED_IS_NEGATIVE
    return (target_speed < 0.0f) ? 1.0f : -1.0f;
#else
    return (target_speed >= 0.0f) ? 1.0f : -1.0f;
#endif
#else
    (void)target_speed;
    return 1.0f;
#endif
}

static float NavReplay_CalcLqrErr(const LqrReference_t *ref)
{
    float psi_ref_rad = ref->yaw_deg * LQR_DEG_TO_RAD;
    float dx = inertial_nav.x - ref->x;
    float dy = inertial_nav.y - ref->y;
    float e_y = -sinf(psi_ref_rad) * dx + cosf(psi_ref_rad) * dy;
    float e_psi = NormalizeAngle(ref->yaw_deg - inertial_nav.relative_yaw);
    float curv_sign = NavReplay_GetCurvatureDirectionSign(ref->target_speed);
    float curv_ff = LQR_K_CURV * ref->curvature * curv_sign;
    float err_raw;
    float diff;

    e_y = Float_Constrain(e_y, -LQR_LATERAL_ERR_LIMIT_MM, LQR_LATERAL_ERR_LIMIT_MM);

    err_raw = LQR_SIGN * (curv_ff + LQR_K_LATERAL * e_y + LQR_K_HEADING * e_psi);
    err_raw = Float_Constrain(err_raw, -LQR_ERR_MAX_DEG, LQR_ERR_MAX_DEG);

    diff = err_raw - s_prev_err_degree;
    err_raw = s_prev_err_degree + Float_Constrain(diff, -LQR_ERR_SLEW_DEG, LQR_ERR_SLEW_DEG);

    return LQR_FILTER_ALPHA * err_raw + (1.0f - LQR_FILTER_ALPHA) * s_prev_err_degree;
}

void NavReplay_Process(void)
{
    int scan_range;
    int base_idx;
    uint8 is_recovering = 0;
    uint16 stop_idx;
    uint16 last_idx;
    float dist_to_stop;
    float raw_speed;
    LqrReference_t ref;

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
        printf("[Nav-LQR] Start heading aligned, launch pose reset.\r\n");
#endif
        return;
    }
#endif

    if (g_special_action_trigger == 1)
    {
        s_prev_trigger = 1;
        return;
    }

    if (s_prev_trigger == 1)
    {
        is_recovering = 1;
        s_prev_trigger = 0;
        s_prev_err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
    }

    if (nav_ram_data.point_count == 0)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    scan_range = is_recovering ? (int)LQR_SEARCH_RANGE_RECOVER : (int)LQR_SEARCH_RANGE_NORMAL;
    base_idx = Find_Closest_Point_Index_Strict((int)g_target_idx, scan_range, is_recovering);
    g_target_idx = (uint16)base_idx;

    last_idx = (uint16)(nav_ram_data.point_count - 1U);
    stop_idx = NavReplay_FindStopBarrierIndex((uint16)base_idx, (uint16)(last_idx - (uint16)base_idx));
    dist_to_stop = CalcDistance(inertial_nav.x, inertial_nav.y,
                                nav_ram_data.points[stop_idx].x, nav_ram_data.points[stop_idx].y);

    if ((stop_idx == last_idx) && (dist_to_stop <= NAV_DIST_ARRIVE))
    {
        g_replay_state = REPLAY_FINISHED;
        g_target_idx = stop_idx;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        s_prev_speed_set = 0.0f;
        s_prev_err_degree = 0.0f;
        return;
    }

    NavReplay_BuildReference((uint16)base_idx, stop_idx, &ref);

    err_degree = NavReplay_CalcLqrErr(&ref);
    s_prev_err_degree = err_degree;

    raw_speed = ref.target_speed;
    target_speed_set = NavReplay_SpeedSlew_Update(raw_speed);
    s_prev_speed_set = target_speed_set;
    g_current_point_type = ref.point_type;
}

#endif
