#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../vision/vision_three_stage_control.h"
#include "../../../vision/vision_slope_control.h"
#include "../../../plan/bumpy_road.h"

#if (CURRENT_NAV_PLAN == 3) && (NAV_PLAN3_METHOD == PLAN3_METHOD_LQR_SPEED_PLANNING)

typedef enum
{
    PLAN3_SPECIAL_NONE = 0,
    PLAN3_SPECIAL_SLOPE,
    PLAN3_SPECIAL_JUMP,
    PLAN3_SPECIAL_BRIDGE,
    PLAN3_SPECIAL_BUMP
} Plan3Special_e;

typedef struct
{
    float x;
    float y;
    float tangent_x;
    float tangent_y;
    float yaw_deg;
    float curvature;
    float target_speed;
    float lateral_err;
    float heading_err;
    uint16 preview_idx;
} Plan3LqrReference_t;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0U;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0U;
volatile uint8 entry_beep_request = 0U;
volatile uint8 exit_beep_request = 0U;

static float s_prev_err_degree = 0.0f;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_start_heading_aligned = 1U;
static Plan3Special_e s_active_special = PLAN3_SPECIAL_NONE;
static uint16 s_active_exit_idx = 0xFFFFU;
static uint8 s_handoff_ticks = 0U;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif
#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static float Plan3_Clamp(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float Plan3_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

static float Plan3_LerpBySpeed(float low_value, float high_value, float speed_mm_s)
{
    float ratio = (speed_mm_s - PLAN3_LQR_LOW_SPEED_MM_S) /
                  (PLAN3_LQR_HIGH_SPEED_MM_S - PLAN3_LQR_LOW_SPEED_MM_S);
    return low_value + (high_value - low_value) * Plan3_Clamp(ratio, 0.0f, 1.0f);
}

static float Plan3_Ramp(float current, float target, float step)
{
    if (current < target) return (current + step > target) ? target : current + step;
    if (current > target) return (current - step < target) ? target : current - step;
    return current;
}

static uint8 Plan3_IsEntryType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_SLOPE) ||
                   (point_type == NAV_POINT_JUMP) ||
                   (point_type == NAV_POINT_BRIDGE) ||
                   (point_type == NAV_POINT_BUMP));
}

static uint8 Plan3_ExitTypeForEntry(uint8 point_type)
{
    if (point_type == NAV_POINT_SLOPE) return NAV_POINT_SLOPE_EXIT;
    if (point_type == NAV_POINT_JUMP) return NAV_POINT_JUMP_EXIT;
    if (point_type == NAV_POINT_BRIDGE) return NAV_POINT_BRIDGE_EXIT;
    if (point_type == NAV_POINT_BUMP) return NAV_POINT_BUMP_EXIT;
    return NAV_POINT_PATH;
}

static Plan3Special_e Plan3_SpecialForEntry(uint8 point_type)
{
    if (point_type == NAV_POINT_SLOPE) return PLAN3_SPECIAL_SLOPE;
    if (point_type == NAV_POINT_JUMP) return PLAN3_SPECIAL_JUMP;
    if (point_type == NAV_POINT_BRIDGE) return PLAN3_SPECIAL_BRIDGE;
    if (point_type == NAV_POINT_BUMP) return PLAN3_SPECIAL_BUMP;
    return PLAN3_SPECIAL_NONE;
}

static uint16 Plan3_FindNextSpecialEntry(uint16 start_idx)
{
    uint16 i;
    for (i = start_idx; i < nav_ram_data.point_count; i++)
    {
        if (Plan3_IsEntryType(nav_ram_data.points[i].point_type)) return i;
    }
    return 0xFFFFU;
}

static uint16 Plan3_FindMatchingExit(uint16 entry_idx)
{
    uint16 i;
    uint8 exit_type;
    if (entry_idx >= nav_ram_data.point_count) return 0xFFFFU;
    exit_type = Plan3_ExitTypeForEntry(nav_ram_data.points[entry_idx].point_type);
    for (i = (uint16)(entry_idx + 1U); i < nav_ram_data.point_count; i++)
    {
        if (nav_ram_data.points[i].point_type == exit_type) return i;
    }
    return 0xFFFFU;
}

/* Return along-path distance, rather than Euclidean distance, so the
 * generated special-task corridor is respected on curved approaches. */
static float Plan3_PathDistance(uint16 first_idx, uint16 last_idx)
{
    uint16 i;
    float distance = 0.0f;
    if ((first_idx >= last_idx) || (last_idx >= nav_ram_data.point_count)) return 0.0f;
    for (i = first_idx; i < last_idx; i++)
    {
        float dx = nav_ram_data.points[i + 1U].x - nav_ram_data.points[i].x;
        float dy = nav_ram_data.points[i + 1U].y - nav_ram_data.points[i].y;
        distance += sqrtf(dx * dx + dy * dy);
        if (distance > PLAN3_SPECIAL_HANDOFF_LEAD_MM) return distance;
    }
    return distance;
}

/* Select the closest *segment* ahead of the monotonic index.  The search is
 * clamped before the next task entry so normal tracking cannot skip a task. */
static uint16 Plan3_FindClosestSegment(uint16 start_idx, uint16 end_idx, uint8 recovering)
{
    uint16 i;
    uint16 best_idx = start_idx;
    float best_dist_sq = 1.0e30f;
    float car_x = nav_vision_fusion_x;
    float car_y = nav_vision_fusion_y;

    if (nav_ram_data.point_count < 2U) return 0U;
    if (start_idx >= nav_ram_data.point_count - 1U) start_idx = (uint16)(nav_ram_data.point_count - 2U);
    if (end_idx >= nav_ram_data.point_count - 1U) end_idx = (uint16)(nav_ram_data.point_count - 2U);
    if (end_idx < start_idx) end_idx = start_idx;

    for (i = start_idx; i <= end_idx; i++)
    {
        float ax = nav_ram_data.points[i].x;
        float ay = nav_ram_data.points[i].y;
        float dx = nav_ram_data.points[i + 1U].x - ax;
        float dy = nav_ram_data.points[i + 1U].y - ay;
        float len_sq = dx * dx + dy * dy;
        float t = 0.0f;
        float px;
        float py;
        float ex;
        float ey;
        float dist_sq;

        if (len_sq > PLAN3_LQR_PROJECTION_MIN_SEG_MM * PLAN3_LQR_PROJECTION_MIN_SEG_MM)
        {
            t = Plan3_Clamp(((car_x - ax) * dx + (car_y - ay) * dy) / len_sq, 0.0f, 1.0f);
        }
        px = ax + t * dx;
        py = ay + t * dy;
        ex = car_x - px;
        ey = car_y - py;
        dist_sq = ex * ex + ey * ey;
        if (dist_sq < best_dist_sq)
        {
            best_dist_sq = dist_sq;
            best_idx = i;
        }
    }

    if ((!recovering) && (best_dist_sq > PLAN3_LQR_MAX_TRACK_DIST_MM * PLAN3_LQR_MAX_TRACK_DIST_MM))
    {
        return start_idx;
    }
    return best_idx;
}

static void Plan3_BuildReference(uint16 segment_idx, uint16 preview_limit, Plan3LqrReference_t *reference)
{
    uint16 preview_idx;
    const NavRamPoint_t *start = &nav_ram_data.points[segment_idx];
    const NavRamPoint_t *end = &nav_ram_data.points[segment_idx + 1U];
    float dx = end->x - start->x;
    float dy = end->y - start->y;
    float len_sq = dx * dx + dy * dy;
    float len = sqrtf(len_sq);
    float t = 0.0f;
    float car_dx;
    float car_dy;

    if (len_sq > PLAN3_LQR_PROJECTION_MIN_SEG_MM * PLAN3_LQR_PROJECTION_MIN_SEG_MM)
    {
        t = Plan3_Clamp(((nav_vision_fusion_x - start->x) * dx +
                         (nav_vision_fusion_y - start->y) * dy) / len_sq, 0.0f, 1.0f);
    }

    preview_idx = (uint16)(segment_idx + PLAN3_LQR_PREVIEW_POINTS);
    if (fabsf(start->curvature) >= PLAN3_LQR_SHARP_CURVATURE_TH)
    {
        preview_idx = (uint16)(segment_idx + PLAN3_LQR_SHARP_PREVIEW_POINTS);
    }
    if (preview_idx > preview_limit) preview_idx = preview_limit;
    if (preview_idx >= nav_ram_data.point_count) preview_idx = (uint16)(nav_ram_data.point_count - 1U);

    reference->x = start->x + t * dx;
    reference->y = start->y + t * dy;
    if (len > PLAN3_LQR_PROJECTION_MIN_SEG_MM)
    {
        reference->tangent_x = dx / len;
        reference->tangent_y = dy / len;
        reference->yaw_deg = -atan2f(dy, -dx) * 57.29578f;
    }
    else
    {
        reference->tangent_x = 1.0f;
        reference->tangent_y = 0.0f;
        reference->yaw_deg = start->target_yaw_deg;
    }
    car_dx = nav_vision_fusion_x - reference->x;
    car_dy = nav_vision_fusion_y - reference->y;
    reference->lateral_err = reference->tangent_y * car_dx - reference->tangent_x * car_dy;
    reference->heading_err = Plan3_NormalizeAngle(reference->yaw_deg - inertial_nav.relative_yaw);
    reference->curvature = nav_ram_data.points[preview_idx].curvature;
    reference->target_speed = start->target_speed;
    reference->preview_idx = preview_idx;
}

static float Plan3_CalcSteer(const Plan3LqrReference_t *reference)
{
    float speed_mm_s = fabsf(reference->target_speed) * PLAN3_LQR_SPEED_TO_MM_S;
    float err_limit = Plan3_LerpBySpeed(PLAN3_LQR_ERR_LIMIT_LOW_DEG,
                                        PLAN3_LQR_ERR_LIMIT_HIGH_DEG, speed_mm_s);
    float slew_limit = Plan3_LerpBySpeed(PLAN3_LQR_ERR_SLEW_LOW_DEG,
                                         PLAN3_LQR_ERR_SLEW_HIGH_DEG, speed_mm_s);
    float alpha = Plan3_LerpBySpeed(PLAN3_LQR_FILTER_ALPHA_LOW,
                                    PLAN3_LQR_FILTER_ALPHA_HIGH, speed_mm_s);
    float lateral_err = Plan3_Clamp(reference->lateral_err,
                                    -PLAN3_LQR_LATERAL_ERR_LIMIT_MM,
                                    PLAN3_LQR_LATERAL_ERR_LIMIT_MM);
    float yaw_rate_ref = speed_mm_s * reference->curvature;
    float raw = PLAN3_LQR_SIGN * (PLAN3_LQR_K_YAW_RATE_FF * yaw_rate_ref +
                                  PLAN3_LQR_K_LATERAL * lateral_err +
                                  PLAN3_LQR_K_HEADING * reference->heading_err);
    float limited;

    raw = Plan3_Clamp(raw, -err_limit, err_limit);
    limited = s_prev_err_degree + Plan3_Clamp(raw - s_prev_err_degree, -slew_limit, slew_limit);
    return alpha * limited + (1.0f - alpha) * s_prev_err_degree;
}

static float Plan3_SafeSpeed(const Plan3LqrReference_t *reference)
{
    float raw = reference->target_speed;
    float magnitude;
    float factor = 1.0f;
    float lateral = fabsf(reference->lateral_err);
    float heading = fabsf(reference->heading_err);

    /* A missing / malformed route speed must still move, never turn a normal
     * sample into a zero-speed point. */
    if (raw >= 0.0f) raw = PLAN3_TRACK_MIN_SPEED_CMD;
    magnitude = fabsf(raw);
    if (lateral > PLAN3_TRACK_CROSS_TRACK_SOFT_MM)
    {
        factor = (PLAN3_TRACK_CROSS_TRACK_HARD_MM - lateral) /
                 (PLAN3_TRACK_CROSS_TRACK_HARD_MM - PLAN3_TRACK_CROSS_TRACK_SOFT_MM);
        factor = Plan3_Clamp(factor, 0.20f, 1.0f);
    }
    if (heading > PLAN3_TRACK_YAW_SOFT_DEG)
    {
        float heading_factor = (PLAN3_TRACK_YAW_HARD_DEG - heading) /
                               (PLAN3_TRACK_YAW_HARD_DEG - PLAN3_TRACK_YAW_SOFT_DEG);
        heading_factor = Plan3_Clamp(heading_factor, 0.20f, 1.0f);
        if (heading_factor < factor) factor = heading_factor;
    }
    raw = -magnitude * factor;
    if (raw > PLAN3_TRACK_MIN_SPEED_CMD) raw = PLAN3_TRACK_MIN_SPEED_CMD;
    return Plan3_Ramp(s_prev_speed_cmd, raw,
                      (fabsf(raw) > fabsf(s_prev_speed_cmd)) ?
                      PLAN3_SPEED_ACCEL_STEP : PLAN3_SPEED_DECEL_STEP);
}

static uint8 Plan3_SpecialIsActive(void)
{
    if (s_active_special == PLAN3_SPECIAL_SLOPE) return VisionSlopeTask_IsActive();
    if (s_active_special == PLAN3_SPECIAL_JUMP) return VisionThreeStageControl_IsActive();
    if (s_active_special == PLAN3_SPECIAL_BRIDGE) return VisionBridgeTask_IsActive();
    if (s_active_special == PLAN3_SPECIAL_BUMP) return BumpyRoad_Is_Active();
    return 0U;
}

static void Plan3_CompleteSpecial(void)
{
    uint8 rebase = 0U;
    if ((s_active_special == PLAN3_SPECIAL_JUMP) &&
        (g_vision_three_stage_control_status.exit_reason == VISION_THREE_STAGE_EXIT_SUCCESS)) rebase = 1U;
    if ((s_active_special == PLAN3_SPECIAL_BRIDGE) &&
        (g_bridge_vision_task_exit_reason == VISION_BRIDGE_EXIT_VISUAL_CONFIRMED)) rebase = 1U;

    if ((rebase != 0U) && (s_active_exit_idx < nav_ram_data.point_count))
    {
        nav_vision_fusion_x = nav_ram_data.points[s_active_exit_idx].x;
        nav_vision_fusion_y = nav_ram_data.points[s_active_exit_idx].y;
        exit_beep_request = 1U;
    }
    if (s_active_exit_idx < nav_ram_data.point_count)
    {
        g_target_idx = (uint16)(s_active_exit_idx + 1U);
    }
    s_active_special = PLAN3_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    g_special_action_trigger = 0U;
    s_handoff_ticks = PLAN3_SPECIAL_HANDOFF_TICKS;
    s_prev_err_degree = err_degree;
    s_prev_speed_cmd = target_speed_set;
}

static void Plan3_StartSpecial(uint16 entry_idx)
{
    uint8 point_type = nav_ram_data.points[entry_idx].point_type;
    s_active_special = Plan3_SpecialForEntry(point_type);
    s_active_exit_idx = Plan3_FindMatchingExit(entry_idx);
    g_current_point_type = point_type;
    g_special_action_trigger = 1U;

    if (point_type == NAV_POINT_SLOPE)
    {
        VisionSlopeTask_Start();
    }
    else if (point_type == NAV_POINT_JUMP)
    {
        entry_beep_request = 1U;
        VisionThreeStageControl_Start();
    }
    else if (point_type == NAV_POINT_BRIDGE)
    {
        entry_beep_request = 1U;
        VisionBridgeTask_Start();
    }
    else if (point_type == NAV_POINT_BUMP)
    {
        if (s_active_exit_idx < nav_ram_data.point_count)
        {
            BumpyRoad_SetRouteAnchors(nav_ram_data.points[entry_idx].x,
                                      nav_ram_data.points[entry_idx].y,
                                      nav_ram_data.points[s_active_exit_idx].x,
                                      nav_ram_data.points[s_active_exit_idx].y);
        }
        BumpyRoad_Trigger();
    }
}

uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 i;
    uint16 count = NAV_REPLAY_STATIC_ROUTE_COUNT;
    if (count > NAV_RAM_MAX_POINTS) count = NAV_RAM_MAX_POINTS;
    nav_ram_data.plan_type = NAV_PLAN_3;
    nav_ram_data.point_count = count;
    for (i = 0U; i < count; i++) nav_ram_data.points[i] = nav_replay_static_route_points[i];
    return count;
#else
    return nav_ram_data.point_count;
#endif
}

void NavReplay_Start(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    NavReplay_LoadStaticRouteToRam();
#endif
    if (nav_ram_data.point_count < 2U) return;
    g_target_idx = 0U;
    g_current_point_type = NAV_POINT_PATH;
    g_replay_state = REPLAY_RUNNING;
    g_special_action_trigger = 0U;
    s_active_special = PLAN3_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    s_handoff_ticks = 0U;
    s_prev_err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    entry_beep_request = 0U;
    exit_beep_request = 0U;
#if IMU_CATEGORY == 3
    s_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0U : 1U;
#else
    s_start_heading_aligned = 1U;
#endif
}

void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    err_degree = 0.0f;
    g_replay_state = REPLAY_IDLE;
    g_special_action_trigger = 0U;
    s_active_special = PLAN3_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    s_handoff_ticks = 0U;
    s_prev_err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    s_start_heading_aligned = 1U;
}

void NavReplay_Process(void)
{
    uint16 last_segment;
    uint16 next_special;
    uint16 search_end;
    uint16 base_idx;
    Plan3LqrReference_t reference;
    float steer_cmd;
    float speed_cmd;

    if (g_replay_state != REPLAY_RUNNING) return;

#if IMU_CATEGORY == 3
    if (s_start_heading_aligned == 0U)
    {
        float heading_err = Plan3_NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        target_speed_set = 0.0f;
        err_degree = Plan3_Clamp(heading_err, -PLAN3_START_HEADING_ERR_LIMIT_DEG,
                                 PLAN3_START_HEADING_ERR_LIMIT_DEG);
        if (fabsf(heading_err) <= PLAN3_START_HEADING_TOLERANCE_DEG)
        {
            s_start_heading_aligned = 1U;
            s_prev_err_degree = 0.0f;
        }
        return;
    }
#endif

    if (s_active_special != PLAN3_SPECIAL_NONE)
    {
        if (Plan3_SpecialIsActive() != 0U) return;
        Plan3_CompleteSpecial();
    }
    else if (g_special_action_trigger != 0U)
    {
        /* External emergency/special ownership: never overwrite its command. */
        return;
    }

    if (g_target_idx >= nav_ram_data.point_count - 1U)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0.0f;
        err_degree = 0.0f;
        return;
    }

    last_segment = (uint16)(nav_ram_data.point_count - 2U);
    next_special = Plan3_FindNextSpecialEntry(g_target_idx);
    search_end = (uint16)(g_target_idx + PLAN3_LQR_SEARCH_RANGE_POINTS);
    if (search_end > last_segment) search_end = last_segment;
    if ((next_special != 0xFFFFU) && (next_special > 0U) && (search_end >= next_special))
    {
        search_end = (uint16)(next_special - 1U);
    }
    base_idx = Plan3_FindClosestSegment(g_target_idx, search_end, (s_handoff_ticks != 0U));
    g_target_idx = base_idx;

    if ((next_special != 0xFFFFU) &&
        (Plan3_PathDistance(base_idx, next_special) <= PLAN3_SPECIAL_HANDOFF_LEAD_MM))
    {
        Plan3_StartSpecial(next_special);
        return;
    }

    Plan3_BuildReference(base_idx, (next_special == 0xFFFFU) ? last_segment : (uint16)(next_special - 1U), &reference);
    steer_cmd = Plan3_CalcSteer(&reference);
    speed_cmd = Plan3_SafeSpeed(&reference);

    if (s_handoff_ticks > 0U)
    {
        err_degree = Plan3_Ramp(err_degree, steer_cmd, PLAN3_SPECIAL_HANDOFF_ERR_STEP_DEG);
        target_speed_set = Plan3_Ramp(target_speed_set, speed_cmd, PLAN3_SPECIAL_HANDOFF_SPEED_STEP);
        s_handoff_ticks--;
    }
    else
    {
        err_degree = steer_cmd;
        target_speed_set = speed_cmd;
    }
    s_prev_err_degree = err_degree;
    s_prev_speed_cmd = target_speed_set;
    g_current_point_type = nav_ram_data.points[base_idx].point_type;
}

#endif
