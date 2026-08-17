#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../vision/vision_three_stage_control.h"
#include "../../../vision/vision_slope_control.h"
#include "../../../plan/bumpy_road.h"
#include "../../../plan/minefield.h"

#if (CURRENT_NAV_PLAN == 4) && (NAV_PLAN4_METHOD == PLAN4_METHOD_LQR_SPEED_PLANNING)

typedef enum
{
    PLAN4_SPECIAL_NONE = 0,
    PLAN4_SPECIAL_SLOPE,
    PLAN4_SPECIAL_JUMP,
    PLAN4_SPECIAL_BRIDGE,
    PLAN4_SPECIAL_BUMP,
    PLAN4_SPECIAL_MINEFIELD
} Plan4Special_e;

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
} Plan4LqrReference_t;

NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0U;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0U;
volatile uint8 entry_beep_request = 0U;
volatile uint8 exit_beep_request = 0U;

static float s_prev_err_degree = 0.0f;
static float s_prev_speed_cmd = 0.0f;
static uint8 s_start_heading_aligned = 1U;
static Plan4Special_e s_active_special = PLAN4_SPECIAL_NONE;
static uint16 s_active_exit_idx = 0xFFFFU;
static uint16 s_active_entry_idx = 0xFFFFU;
static uint8 s_handoff_ticks = 0U;
static uint8 s_exit_rejoin_active = 0U;
static uint16 s_exit_rejoin_end_idx = 0xFFFFU;
static uint8 s_minefield_zero_brake_issued = 0U;
static float s_minefield_exit_speed_cmd = 0.0f;
static uint16 s_minefield_exit_speed_end_idx = 0xFFFFU;
static uint8 s_finish_decel_active = 0U;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif
#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static float Plan4_Clamp(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float Plan4_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

static float Plan4_PositiveAngle360(float angle)
{
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

static float Plan4_CalcBearingDeg(float x1, float y1, float x2, float y2)
{
    return -atan2f(y2 - y1, -(x2 - x1)) * 57.29578f;
}

static uint8 Plan4_FinalPointCrossed(uint16 last_segment)
{
    const NavRamPoint_t *start = &nav_ram_data.points[last_segment];
    const NavRamPoint_t *end = &nav_ram_data.points[last_segment + 1U];
    float dx = end->x - start->x;
    float dy = end->y - start->y;
    float len_sq = dx * dx + dy * dy;
    float progress;

    if (len_sq <= 1.0e-6f) return 0U;
    progress = ((nav_vision_fusion_x - start->x) * dx +
                (nav_vision_fusion_y - start->y) * dy) / len_sq;
    return (uint8)(progress >= 1.0f);
}

static float Plan4_LerpBySpeed(float low_value, float high_value, float speed_mm_s)
{
    float ratio = (speed_mm_s - PLAN4_LQR_LOW_SPEED_MM_S) /
                  (PLAN4_LQR_HIGH_SPEED_MM_S - PLAN4_LQR_LOW_SPEED_MM_S);
    return low_value + (high_value - low_value) * Plan4_Clamp(ratio, 0.0f, 1.0f);
}

static float Plan4_Ramp(float current, float target, float step)
{
    if (current < target) return (current + step > target) ? target : current + step;
    if (current > target) return (current - step < target) ? target : current - step;
    return current;
}

static uint8 Plan4_IsEntryType(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_CIRCLE) ||
                   (point_type == NAV_POINT_SLOPE) ||
                   (point_type == NAV_POINT_JUMP) ||
                   (point_type == NAV_POINT_BRIDGE) ||
                   (point_type == NAV_POINT_BUMP));
}

static uint8 Plan4_ExitTypeForEntry(uint8 point_type)
{
    if (point_type == NAV_POINT_SLOPE) return NAV_POINT_SLOPE_EXIT;
    if (point_type == NAV_POINT_JUMP) return NAV_POINT_JUMP_EXIT;
    if (point_type == NAV_POINT_BRIDGE) return NAV_POINT_BRIDGE_EXIT;
    if (point_type == NAV_POINT_BUMP) return NAV_POINT_BUMP_EXIT;
    return NAV_POINT_PATH;
}

static Plan4Special_e Plan4_SpecialForEntry(uint8 point_type)
{
    if (point_type == NAV_POINT_CIRCLE) return PLAN4_SPECIAL_MINEFIELD;
    if (point_type == NAV_POINT_SLOPE) return PLAN4_SPECIAL_SLOPE;
    if (point_type == NAV_POINT_JUMP) return PLAN4_SPECIAL_JUMP;
    if (point_type == NAV_POINT_BRIDGE) return PLAN4_SPECIAL_BRIDGE;
    if (point_type == NAV_POINT_BUMP) return PLAN4_SPECIAL_BUMP;
    return PLAN4_SPECIAL_NONE;
}

/* 这些任务的入口朝向决定状态机能否稳定接管。坡道保留原有交接行为。 */
static uint8 Plan4_SpecialNeedsAlignment(uint8 point_type)
{
    return (uint8)((point_type == NAV_POINT_JUMP) ||
                   (point_type == NAV_POINT_BRIDGE) ||
                   (point_type == NAV_POINT_BUMP));
}

static uint16 Plan4_FindNextSpecialEntry(uint16 start_idx)
{
    uint16 i;
    for (i = start_idx; i < nav_ram_data.point_count; i++)
    {
        if (Plan4_IsEntryType(nav_ram_data.points[i].point_type)) return i;
    }
    return 0xFFFFU;
}

static uint16 Plan4_FindMatchingExit(uint16 entry_idx)
{
    uint16 i;
    uint8 exit_type;
    if (entry_idx >= nav_ram_data.point_count) return 0xFFFFU;
    if (nav_ram_data.points[entry_idx].point_type == NAV_POINT_CIRCLE) return 0xFFFFU;
    exit_type = Plan4_ExitTypeForEntry(nav_ram_data.points[entry_idx].point_type);
    for (i = (uint16)(entry_idx + 1U); i < nav_ram_data.point_count; i++)
    {
        if (nav_ram_data.points[i].point_type == exit_type) return i;
    }
    return 0xFFFFU;
}

/* Return along-path distance, rather than Euclidean distance, so the
 * generated special-task corridor is respected on curved approaches. */
static float Plan4_PathDistance(uint16 first_idx, uint16 last_idx, float stop_distance_mm)
{
    uint16 i;
    float distance = 0.0f;
    if ((first_idx >= last_idx) || (last_idx >= nav_ram_data.point_count)) return 0.0f;
    for (i = first_idx; i < last_idx; i++)
    {
        float dx = nav_ram_data.points[i + 1U].x - nav_ram_data.points[i].x;
        float dy = nav_ram_data.points[i + 1U].y - nav_ram_data.points[i].y;
        distance += sqrtf(dx * dx + dy * dy);
        if (distance > stop_distance_mm) return distance;
    }
    return distance;
}

static uint16 Plan4_FindForwardIndex(uint16 first_idx, float distance_mm)
{
    uint16 i;
    float distance = 0.0f;

    if (first_idx >= nav_ram_data.point_count - 1U) return first_idx;
    for (i = first_idx; i < nav_ram_data.point_count - 1U; i++)
    {
        float dx = nav_ram_data.points[i + 1U].x - nav_ram_data.points[i].x;
        float dy = nav_ram_data.points[i + 1U].y - nav_ram_data.points[i].y;
        distance += sqrtf(dx * dx + dy * dy);
        if (distance >= distance_mm) return (uint16)(i + 1U);
    }
    return (uint16)(nav_ram_data.point_count - 1U);
}

/* 沿路径向前查找指定距离处的速度，用于雷区转圈后的起步恢复。 */
static float Plan4_FindMinefieldExitSpeed(uint16 entry_idx, uint16 *speed_end_idx)
{
    uint16 i;
    float distance = 0.0f;
    float speed = PLAN4_TRACK_MIN_SPEED_CMD;

    *speed_end_idx = (uint16)(entry_idx + 1U);

    for (i = (uint16)(entry_idx + 1U); i < nav_ram_data.point_count; i++)
    {
        float dx = nav_ram_data.points[i].x - nav_ram_data.points[i - 1U].x;
        float dy = nav_ram_data.points[i].y - nav_ram_data.points[i - 1U].y;
        distance += sqrtf(dx * dx + dy * dy);
        if (distance >= PLAN4_MINEFIELD_EXIT_SPEED_LOOKAHEAD_MM)
        {
            speed = nav_ram_data.points[i].target_speed;
            *speed_end_idx = i;
            break;
        }
    }
    if (speed >= 0.0f) speed = PLAN4_TRACK_MIN_SPEED_CMD;
    return speed;
}

/* 从单调递增索引的前方选择最近线段，并在下一任务入口前截断搜索，避免普通跟踪跳过任务。 */
static uint16 Plan4_FindClosestSegment(uint16 start_idx, uint16 end_idx, uint8 recovering)
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

        if (len_sq > PLAN4_LQR_PROJECTION_MIN_SEG_MM * PLAN4_LQR_PROJECTION_MIN_SEG_MM)
        {
            t = Plan4_Clamp(((car_x - ax) * dx + (car_y - ay) * dy) / len_sq, 0.0f, 1.0f);
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

    if ((!recovering) && (best_dist_sq > PLAN4_LQR_MAX_TRACK_DIST_MM * PLAN4_LQR_MAX_TRACK_DIST_MM))
    {
        return start_idx;
    }
    return best_idx;
}

static void Plan4_BuildReference(uint16 segment_idx, uint16 preview_limit, Plan4LqrReference_t *reference)
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

    if (len_sq > PLAN4_LQR_PROJECTION_MIN_SEG_MM * PLAN4_LQR_PROJECTION_MIN_SEG_MM)
    {
        t = Plan4_Clamp(((nav_vision_fusion_x - start->x) * dx +
                         (nav_vision_fusion_y - start->y) * dy) / len_sq, 0.0f, 1.0f);
    }

    preview_idx = (uint16)(segment_idx + PLAN4_LQR_PREVIEW_POINTS);
    if (fabsf(start->curvature) >= PLAN4_LQR_SHARP_CURVATURE_TH)
    {
        preview_idx = (uint16)(segment_idx + PLAN4_LQR_SHARP_PREVIEW_POINTS);
    }
    if (preview_idx > preview_limit) preview_idx = preview_limit;
    if (preview_idx >= nav_ram_data.point_count) preview_idx = (uint16)(nav_ram_data.point_count - 1U);

    reference->x = start->x + t * dx;
    reference->y = start->y + t * dy;
    if (len > PLAN4_LQR_PROJECTION_MIN_SEG_MM)
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
    reference->heading_err = Plan4_NormalizeAngle(reference->yaw_deg - inertial_nav.relative_yaw);
    reference->curvature = nav_ram_data.points[preview_idx].curvature;
    reference->target_speed = start->target_speed;
    reference->preview_idx = preview_idx;
}

static float Plan4_CalcSteer(const Plan4LqrReference_t *reference)
{
    float speed_mm_s = fabsf(reference->target_speed) * PLAN4_LQR_SPEED_TO_MM_S;
    float err_limit = Plan4_LerpBySpeed(PLAN4_LQR_ERR_LIMIT_LOW_DEG,
                                        PLAN4_LQR_ERR_LIMIT_HIGH_DEG, speed_mm_s);
    float slew_limit = Plan4_LerpBySpeed(PLAN4_LQR_ERR_SLEW_LOW_DEG,
                                         PLAN4_LQR_ERR_SLEW_HIGH_DEG, speed_mm_s);
    float alpha = Plan4_LerpBySpeed(PLAN4_LQR_FILTER_ALPHA_LOW,
                                    PLAN4_LQR_FILTER_ALPHA_HIGH, speed_mm_s);
    float lateral_err = Plan4_Clamp(reference->lateral_err,
                                    -PLAN4_LQR_LATERAL_ERR_LIMIT_MM,
                                    PLAN4_LQR_LATERAL_ERR_LIMIT_MM);
    float yaw_rate_ref = speed_mm_s * reference->curvature;
    float raw = PLAN4_LQR_SIGN * (PLAN4_LQR_K_YAW_RATE_FF * yaw_rate_ref +
                                  PLAN4_LQR_K_LATERAL * lateral_err +
                                  PLAN4_LQR_K_HEADING * reference->heading_err);
    float limited;

    raw = Plan4_Clamp(raw, -err_limit, err_limit);
    limited = s_prev_err_degree + Plan4_Clamp(raw - s_prev_err_degree, -slew_limit, slew_limit);
    return alpha * limited + (1.0f - alpha) * s_prev_err_degree;
}

static float Plan4_SafeSpeed(const Plan4LqrReference_t *reference)
{
    float raw = reference->target_speed;
    float magnitude;
    float factor = 1.0f;
    float lateral = fabsf(reference->lateral_err);
    float heading = fabsf(reference->heading_err);

    /* A missing / malformed route speed must still move, never turn a normal
     * sample into a zero-speed point. */
    if (raw >= 0.0f) raw = PLAN4_TRACK_MIN_SPEED_CMD;
    magnitude = fabsf(raw);
    if ((s_minefield_exit_speed_end_idx != 0xFFFFU) &&
        (g_target_idx < s_minefield_exit_speed_end_idx) &&
        (fabsf(s_minefield_exit_speed_cmd) > magnitude))
    {
        magnitude = fabsf(s_minefield_exit_speed_cmd);
    }
    if (s_exit_rejoin_active != 0U)
    {
        /* The exit anchor is an intentional fusion-coordinate rebase.  Ignore
         * the resulting transient error, but keep a slow bounded rejoin and
         * retain the normal protection for a genuinely implausible deviation. */
        if (magnitude > PLAN4_EXIT_REJOIN_MAX_SPEED_CMD)
        {
            magnitude = PLAN4_EXIT_REJOIN_MAX_SPEED_CMD;
        }
        if ((lateral < PLAN4_EXIT_REJOIN_EMERGENCY_CROSS_MM) &&
            (heading < PLAN4_EXIT_REJOIN_EMERGENCY_YAW_DEG))
        {
            raw = -magnitude;
            return Plan4_Ramp(s_prev_speed_cmd, raw,
                              (fabsf(raw) > fabsf(s_prev_speed_cmd)) ?
                              PLAN4_SPEED_ACCEL_STEP : PLAN4_SPEED_DECEL_STEP);
        }
    }
    if (lateral > PLAN4_TRACK_CROSS_TRACK_SOFT_MM)
    {
        factor = (PLAN4_TRACK_CROSS_TRACK_HARD_MM - lateral) /
                 (PLAN4_TRACK_CROSS_TRACK_HARD_MM - PLAN4_TRACK_CROSS_TRACK_SOFT_MM);
        factor = Plan4_Clamp(factor, 0.20f, 1.0f);
    }
    if (heading > PLAN4_TRACK_YAW_SOFT_DEG)
    {
        float heading_factor = (PLAN4_TRACK_YAW_HARD_DEG - heading) /
                               (PLAN4_TRACK_YAW_HARD_DEG - PLAN4_TRACK_YAW_SOFT_DEG);
        heading_factor = Plan4_Clamp(heading_factor, 0.20f, 1.0f);
        if (heading_factor < factor) factor = heading_factor;
    }
    raw = -magnitude * factor;
    if (raw > PLAN4_TRACK_MIN_SPEED_CMD) raw = PLAN4_TRACK_MIN_SPEED_CMD;
    return Plan4_Ramp(s_prev_speed_cmd, raw,
                      (fabsf(raw) > fabsf(s_prev_speed_cmd)) ?
                      PLAN4_SPEED_ACCEL_STEP : PLAN4_SPEED_DECEL_STEP);
}

static uint8 Plan4_SpecialEntryAligned(uint16 entry_idx,
                                        const Plan4LqrReference_t *reference)
{
    uint8 point_type = nav_ram_data.points[entry_idx].point_type;

    if (Plan4_SpecialNeedsAlignment(point_type) == 0U) return 1U;
    return (uint8)((fabsf(reference->heading_err) <= PLAN4_SPECIAL_ENTRY_YAW_TOLERANCE_DEG) &&
                   (fabsf(reference->lateral_err) <= PLAN4_SPECIAL_ENTRY_CROSS_TOLERANCE_MM));
}

static float Plan4_LimitSpecialApproachSpeed(uint16 entry_idx,
                                              float distance_mm,
                                              const Plan4LqrReference_t *reference,
                                              float speed_cmd)
{
    float yaw_factor = 1.0f;
    float cross_factor = 1.0f;
    float factor;
    float magnitude;
    float desired;

    if ((Plan4_SpecialNeedsAlignment(nav_ram_data.points[entry_idx].point_type) == 0U) ||
        (distance_mm > PLAN4_SPECIAL_ALIGN_DISTANCE_MM)) return speed_cmd;

    if (fabsf(reference->heading_err) > PLAN4_SPECIAL_ALIGN_YAW_FULL_SPEED_DEG)
    {
        yaw_factor = (PLAN4_SPECIAL_ALIGN_YAW_BLOCK_DEG - fabsf(reference->heading_err)) /
                     (PLAN4_SPECIAL_ALIGN_YAW_BLOCK_DEG - PLAN4_SPECIAL_ALIGN_YAW_FULL_SPEED_DEG);
        yaw_factor = Plan4_Clamp(yaw_factor, PLAN4_SPECIAL_ALIGN_MIN_SPEED_FACTOR, 1.0f);
    }
    if (fabsf(reference->lateral_err) > PLAN4_SPECIAL_ALIGN_CROSS_FULL_MM)
    {
        cross_factor = (PLAN4_SPECIAL_ALIGN_CROSS_BLOCK_MM - fabsf(reference->lateral_err)) /
                       (PLAN4_SPECIAL_ALIGN_CROSS_BLOCK_MM - PLAN4_SPECIAL_ALIGN_CROSS_FULL_MM);
        cross_factor = Plan4_Clamp(cross_factor, PLAN4_SPECIAL_ALIGN_MIN_SPEED_FACTOR, 1.0f);
    }
    factor = (yaw_factor < cross_factor) ? yaw_factor : cross_factor;
    magnitude = fabsf(speed_cmd);
    desired = -magnitude * factor;
    return Plan4_Ramp(speed_cmd, desired, PLAN4_SPEED_DECEL_STEP);
}

static uint8 Plan4_SpecialIsActive(void)
{
    if (s_active_special == PLAN4_SPECIAL_MINEFIELD)
    {
        /* minefield_flag 由高速陀螺仪任务消费。请求尚未被锁存时仍保持
         * Plan4 的动作交接状态，避免较慢的导航任务先判定结束并重复触发。 */
        return (uint8)((minefield_flag != 0U) || (Minefield_Is_Active() != 0U));
    }
    if (s_active_special == PLAN4_SPECIAL_SLOPE) return VisionSlopeTask_IsActive();
    if (s_active_special == PLAN4_SPECIAL_JUMP) return VisionThreeStageControl_IsActive();
    if (s_active_special == PLAN4_SPECIAL_BRIDGE) return VisionBridgeTask_IsActive();
    if (s_active_special == PLAN4_SPECIAL_BUMP) return BumpyRoad_Is_Active();
    return 0U;
}

static void Plan4_CompleteSpecial(void)
{
    uint8 rebase = 0U;
    if ((s_active_special == PLAN4_SPECIAL_BRIDGE) &&
        (g_bridge_vision_task_exit_reason == VISION_BRIDGE_EXIT_VISUAL_CONFIRMED)) rebase = 1U;

    if ((rebase != 0U) && (s_active_exit_idx < nav_ram_data.point_count))
    {
        nav_vision_fusion_x = nav_ram_data.points[s_active_exit_idx].x;
        nav_vision_fusion_y = nav_ram_data.points[s_active_exit_idx].y;
        exit_beep_request = 1U;
    }
    if (s_active_special == PLAN4_SPECIAL_MINEFIELD)
    {
        /* 雷区没有出口标记，不能重定位融合坐标；从入口后的首个路表点恢复。 */
        if ((s_active_entry_idx + 1U) < nav_ram_data.point_count)
        {
            g_target_idx = (uint16)(s_active_entry_idx + 1U);
            s_minefield_exit_speed_cmd = Plan4_FindMinefieldExitSpeed(
                s_active_entry_idx, &s_minefield_exit_speed_end_idx);
        }
    }
    else if (s_active_exit_idx < nav_ram_data.point_count)
    {
        /* Restart on the exit segment itself, never one sample after it.  This
         * keeps the generated post-exit straight corridor continuous even when
         * the state machine has just re-anchored fusion coordinates. */
        g_target_idx = s_active_exit_idx;
        s_exit_rejoin_end_idx = Plan4_FindForwardIndex(s_active_exit_idx,
                                                        PLAN4_EXIT_REJOIN_DISTANCE_MM);
        s_exit_rejoin_active = (uint8)(s_exit_rejoin_end_idx > s_active_exit_idx);
    }
    s_active_special = PLAN4_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    s_active_entry_idx = 0xFFFFU;
    s_minefield_zero_brake_issued = 0U;
    g_special_action_trigger = 0U;
    s_handoff_ticks = PLAN4_SPECIAL_HANDOFF_TICKS;
    s_prev_err_degree = err_degree;
    s_prev_speed_cmd = target_speed_set;
}

static void Plan4_StartSpecial(uint16 entry_idx)
{
    uint8 point_type = nav_ram_data.points[entry_idx].point_type;
    s_active_special = Plan4_SpecialForEntry(point_type);
    s_active_exit_idx = Plan4_FindMatchingExit(entry_idx);
    s_active_entry_idx = entry_idx;
    g_current_point_type = point_type;
    g_special_action_trigger = 1U;
    s_minefield_exit_speed_cmd = 0.0f;
    s_minefield_exit_speed_end_idx = 0xFFFFU;
    s_exit_rejoin_active = 0U;
    s_exit_rejoin_end_idx = 0xFFFFU;

    if (point_type == NAV_POINT_CIRCLE)
    {
        uint16 next_idx = (uint16)(entry_idx + 1U);
        float exit_yaw;
        float current_yaw = inertial_nav.relative_yaw;
        float delta_cw;
        float delta_ccw;
        float total_cw;
        float total_ccw;
        float spin_sign = 1.0f;

        if (next_idx < nav_ram_data.point_count)
        {
            exit_yaw = Plan4_CalcBearingDeg(nav_ram_data.points[entry_idx].x,
                                            nav_ram_data.points[entry_idx].y,
                                            nav_ram_data.points[next_idx].x,
                                            nav_ram_data.points[next_idx].y);
        }
        else
        {
            exit_yaw = nav_ram_data.points[entry_idx].target_yaw_deg;
        }
        delta_cw = Plan4_PositiveAngle360(current_yaw - exit_yaw);
        delta_ccw = Plan4_PositiveAngle360(exit_yaw - current_yaw);
        total_cw = MINEFIELD_SPIN_MIN_TOTAL_ANGLE + delta_cw;
        total_ccw = MINEFIELD_SPIN_MIN_TOTAL_ANGLE + delta_ccw;
        if (total_ccw < total_cw) spin_sign = -1.0f;
        Minefield_SetSpinPlan((total_ccw < total_cw) ? total_ccw : total_cw,
                              exit_yaw,
                              spin_sign);
        minefield_flag = 1U;
        target_speed_set = 0.0f;
        s_prev_speed_cmd = 0.0f;
    }
    else if (point_type == NAV_POINT_SLOPE)
    {
        VisionSlopeTask_Start();
    }
    else if (point_type == NAV_POINT_JUMP)
    {
        entry_beep_request = 1U;
        if (s_active_exit_idx < nav_ram_data.point_count)
        {
            VisionThreeStageControl_SetExitAnchor(nav_ram_data.points[s_active_exit_idx].x,
                                                  nav_ram_data.points[s_active_exit_idx].y);
        }
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
            BumpyRoad_SetExitAnchor(nav_ram_data.points[s_active_exit_idx].x,
                                    nav_ram_data.points[s_active_exit_idx].y);
        }
        BumpyRoad_Trigger();
    }
}

/* 仅有入口标记的雷区采用 Plan2 风格在线接近。不同于普通 Plan4 特殊任务，
 * 此处直接点对点指向 type=1 标记，按实测车速刹车，并蠕行进入执行圆后才启动转圈。 */
static void Plan4_ProcessMinefieldApproach(uint16 entry_idx)
{
    const NavRamPoint_t *entry = &nav_ram_data.points[entry_idx];
    float dx = entry->x - nav_vision_fusion_x;
    float dy = entry->y - nav_vision_fusion_y;
    float dist_mm = sqrtf(dx * dx + dy * dy);
    float point_yaw_deg = Plan4_CalcBearingDeg(nav_vision_fusion_x,
                                               nav_vision_fusion_y,
                                               entry->x,
                                               entry->y);
    float yaw_err_deg = Plan4_NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    float actual_speed_mm_s = fabsf(inertial_nav.vx_body);
    float speed_mag;

    g_current_point_type = NAV_POINT_CIRCLE;
    err_degree = yaw_err_deg;

    if (s_minefield_zero_brake_issued == 0U)
    {
        float brake_dist_mm = (PLAN4_MINEFIELD_BRAKE_POLY_A * actual_speed_mm_s * actual_speed_mm_s +
                               PLAN4_MINEFIELD_BRAKE_POLY_B * actual_speed_mm_s +
                               PLAN4_MINEFIELD_BRAKE_POLY_C) *
                              PLAN4_MINEFIELD_BRAKE_DIST_RATIO;
        if (brake_dist_mm < 0.0f) brake_dist_mm = 0.0f;
        brake_dist_mm += PLAN4_MINEFIELD_BRAKE_MARGIN_MM;
        if (dist_mm <= brake_dist_mm) s_minefield_zero_brake_issued = 1U;
    }

    if (s_minefield_zero_brake_issued != 0U)
    {
        if (dist_mm <= PLAN4_MINEFIELD_EXECUTE_RADIUS_MM)
        {
            if (actual_speed_mm_s <= PLAN4_MINEFIELD_TRIGGER_SPEED_MM_S)
            {
                Plan4_StartSpecial(entry_idx);
                return;
            }
            target_speed_set = 0.0f;
            s_prev_speed_cmd = 0.0f;
            return;
        }

        if (actual_speed_mm_s <= PLAN4_MINEFIELD_TRIGGER_SPEED_MM_S)
        {
            speed_mag = PLAN4_MINEFIELD_TRIGGER_SPEED_MM_S *
                        PLAN4_MINEFIELD_CRAWL_SPEED_RATIO / SPEED_TO_MM_S;
            target_speed_set = Plan4_Ramp(s_prev_speed_cmd, -speed_mag, PLAN4_SPEED_ACCEL_STEP);
            s_prev_speed_cmd = target_speed_set;
            return;
        }

        target_speed_set = 0.0f;
        s_prev_speed_cmd = 0.0f;
        return;
    }

    if (dist_mm <= PLAN4_MINEFIELD_EXECUTE_RADIUS_MM)
    {
        speed_mag = 0.0f;
    }
    else
    {
        speed_mag = sqrtf(2.0f * PLAN4_MINEFIELD_SPEED_DECEL_CMD2_PER_MM *
                          (dist_mm - PLAN4_MINEFIELD_EXECUTE_RADIUS_MM));
        speed_mag = Plan4_Clamp(speed_mag, 0.0f, fabsf(PLAN4_MINEFIELD_SPEED_FAST));
    }

    if (fabsf(yaw_err_deg) > PLAN4_MINEFIELD_YAW_SLOW_TOLERANCE_DEG)
    {
        speed_mag = 0.0f;
    }
    else if (fabsf(yaw_err_deg) > PLAN4_MINEFIELD_YAW_STOP_TOLERANCE_DEG)
    {
        speed_mag *= 0.35f;
    }
    target_speed_set = Plan4_Ramp(s_prev_speed_cmd, -speed_mag,
                                  (speed_mag > fabsf(s_prev_speed_cmd)) ?
                                  PLAN4_SPEED_ACCEL_STEP : PLAN4_SPEED_DECEL_STEP);
    s_prev_speed_cmd = target_speed_set;
}

static void Plan4_StartFinishDecel(void)
{
    g_target_idx = (uint16)(nav_ram_data.point_count - 1U);
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;
    g_replay_state = REPLAY_FINISHED;
    s_finish_decel_active = 1U;
}

static void Plan4_ProcessFinishDecel(void)
{
    uint16 last_segment;
    Plan4LqrReference_t reference;
    float speed_cmd;

    if ((s_finish_decel_active == 0U) || (nav_ram_data.point_count < 2U))
    {
        target_speed_set = 0.0f;
        err_degree = 0.0f;
        return;
    }

    last_segment = (uint16)(nav_ram_data.point_count - 2U);
    speed_cmd = Plan4_Ramp(s_prev_speed_cmd, 0.0f, PLAN4_FINISH_SPEED_DECEL_STEP);
    Plan4_BuildReference(last_segment, last_segment, &reference);
    reference.target_speed = speed_cmd;
    err_degree = Plan4_CalcSteer(&reference);
    target_speed_set = speed_cmd;
    s_prev_err_degree = err_degree;
    s_prev_speed_cmd = target_speed_set;

    if ((target_speed_set == 0.0f) &&
        (fabsf(inertial_nav.vx_body) <= PLAN4_FINISH_STOP_SPEED_MM_S))
    {
        err_degree = 0.0f;
        s_prev_err_degree = 0.0f;
        s_finish_decel_active = 0U;
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
    s_active_special = PLAN4_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    s_active_entry_idx = 0xFFFFU;
    s_minefield_zero_brake_issued = 0U;
    s_minefield_exit_speed_cmd = 0.0f;
    s_minefield_exit_speed_end_idx = 0xFFFFU;
    s_exit_rejoin_active = 0U;
    s_exit_rejoin_end_idx = 0xFFFFU;
    s_finish_decel_active = 0U;
    s_handoff_ticks = 0U;
    s_prev_err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    entry_beep_request = 0U;
    exit_beep_request = 0U;
    Minefield_Init();
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
    s_active_special = PLAN4_SPECIAL_NONE;
    s_active_exit_idx = 0xFFFFU;
    s_active_entry_idx = 0xFFFFU;
    s_minefield_zero_brake_issued = 0U;
    s_minefield_exit_speed_cmd = 0.0f;
    s_minefield_exit_speed_end_idx = 0xFFFFU;
    s_exit_rejoin_active = 0U;
    s_exit_rejoin_end_idx = 0xFFFFU;
    s_finish_decel_active = 0U;
    s_handoff_ticks = 0U;
    s_prev_err_degree = 0.0f;
    s_prev_speed_cmd = 0.0f;
    s_start_heading_aligned = 1U;
    Minefield_Init();
}

void NavReplay_Process(void)
{
    uint16 last_segment;
    uint16 next_special;
    uint16 search_end;
    uint16 base_idx;
    Plan4LqrReference_t reference;
    float steer_cmd;
    float speed_cmd;
    float special_distance_mm = 0.0f;

    /* Keep controlling along the terminal path while decelerating. */
    if (g_replay_state == REPLAY_FINISHED)
    {
        g_target_idx = (nav_ram_data.point_count > 0U) ?
                       (uint16)(nav_ram_data.point_count - 1U) : 0U;
        Plan4_ProcessFinishDecel();
        return;
    }
    if (g_replay_state != REPLAY_RUNNING) return;

#if IMU_CATEGORY == 3
    if (s_start_heading_aligned == 0U)
    {
        float heading_err = Plan4_NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        target_speed_set = 0.0f;
        err_degree = Plan4_Clamp(heading_err, -PLAN4_START_HEADING_ERR_LIMIT_DEG,
                                 PLAN4_START_HEADING_ERR_LIMIT_DEG);
        if (fabsf(heading_err) <= PLAN4_START_HEADING_TOLERANCE_DEG)
        {
            s_start_heading_aligned = 1U;
            s_prev_err_degree = 0.0f;
        }
        return;
    }
#endif

    if (s_active_special != PLAN4_SPECIAL_NONE)
    {
        if (Plan4_SpecialIsActive() != 0U) return;
        Plan4_CompleteSpecial();
    }
    else if (g_special_action_trigger != 0U)
    {
        /* External emergency/special ownership: never overwrite its command. */
        return;
    }

    if (g_target_idx >= nav_ram_data.point_count - 1U)
    {
        Plan4_StartFinishDecel();
        Plan4_ProcessFinishDecel();
        return;
    }

    last_segment = (uint16)(nav_ram_data.point_count - 2U);
    next_special = Plan4_FindNextSpecialEntry(g_target_idx);
    search_end = (uint16)(g_target_idx + PLAN4_LQR_SEARCH_RANGE_POINTS);
    if (search_end > last_segment) search_end = last_segment;
    if ((next_special != 0xFFFFU) && (next_special > 0U) && (search_end >= next_special))
    {
        search_end = (uint16)(next_special - 1U);
    }
    base_idx = Plan4_FindClosestSegment(g_target_idx, search_end, (s_handoff_ticks != 0U));
    g_target_idx = base_idx;
    if ((s_exit_rejoin_active != 0U) && (base_idx >= s_exit_rejoin_end_idx))
    {
        s_exit_rejoin_active = 0U;
        s_exit_rejoin_end_idx = 0xFFFFU;
    }
    if ((base_idx >= last_segment) && Plan4_FinalPointCrossed(last_segment))
    {
        Plan4_StartFinishDecel();
        Plan4_ProcessFinishDecel();
        return;
    }
    if ((s_minefield_exit_speed_end_idx != 0xFFFFU) &&
        (g_target_idx >= s_minefield_exit_speed_end_idx))
    {
        s_minefield_exit_speed_cmd = 0.0f;
        s_minefield_exit_speed_end_idx = 0xFFFFU;
    }

    if ((next_special != 0xFFFFU) &&
        (nav_ram_data.points[next_special].point_type == NAV_POINT_CIRCLE))
    {
        /* 从前一特殊任务/普通路点经 type=0 接近 type=1 时，使用完整的
         * Plan2 风格在线点对点接近；转圈结束后从后续 type=0 恢复普通 LQR。 */
        Plan4_ProcessMinefieldApproach(next_special);
        return;
    }

    if (next_special != 0xFFFFU)
    {
        special_distance_mm = Plan4_PathDistance(base_idx, next_special,
                                                  PLAN4_SPECIAL_ALIGN_DISTANCE_MM);
    }

    Plan4_BuildReference(base_idx, (next_special == 0xFFFFU) ? last_segment : (uint16)(next_special - 1U), &reference);
    if ((next_special != 0xFFFFU) &&
        (special_distance_mm <= PLAN4_SPECIAL_HANDOFF_LEAD_MM) &&
        (Plan4_SpecialEntryAligned(next_special, &reference) != 0U))
    {
        Plan4_StartSpecial(next_special);
        return;
    }
    steer_cmd = Plan4_CalcSteer(&reference);
    speed_cmd = Plan4_SafeSpeed(&reference);
    if (next_special != 0xFFFFU)
    {
        speed_cmd = Plan4_LimitSpecialApproachSpeed(next_special, special_distance_mm,
                                                     &reference, speed_cmd);
    }

    if (s_handoff_ticks > 0U)
    {
        err_degree = Plan4_Ramp(err_degree, steer_cmd, PLAN4_SPECIAL_HANDOFF_ERR_STEP_DEG);
        target_speed_set = Plan4_Ramp(target_speed_set, speed_cmd, PLAN4_SPECIAL_HANDOFF_SPEED_STEP);
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
