#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../vision/vision_three_stage_control.h"
#include "../../../calculate/pid-new.h"
#if (CURRENT_NAV_PLAN == 3) && (NAV_PLAN3_METHOD == PLAN3_METHOD_PRECISE)
// ========================= 内部变量 =========================
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;                    // 当前正在前往的点索引
uint8 g_current_point_type = NAV_POINT_PATH;// 当前点的类型
uint8 g_special_action_trigger = 0;         // 触发标志
volatile uint8 entry_beep_request = 0U;
volatile uint8 exit_beep_request = 0U;

#ifndef NAV_REPLAY_START_HEADING_VALID
#define NAV_REPLAY_START_HEADING_VALID 0
#endif

#ifndef NAV_REPLAY_START_HEADING_DEG
#define NAV_REPLAY_START_HEADING_DEG 0.0f
#endif

static uint8 g_start_heading_aligned = 1;
// 特殊任务入口已交给视觉状态机后，等待其结束并消费紧邻的退出锚点。
static uint8 s_jump_exit_pending = 0U;
static uint8 s_bridge_exit_pending = 0U;
static uint8 s_bumpy_exit_pending = 0U;
static uint8 s_slope_exit_pending = 0U;
#define NAV_BRIDGE_HANDOFF_TICKS            (10U)  // 100ms：视觉退出控制平滑切换至 Plan3
#define NAV_BRIDGE_HANDOFF_SPEED_STEP        (3.0f) // 每 10ms 最大速度目标变化
#define NAV_BRIDGE_HANDOFF_ERR_STEP_DEG      (0.5f) // 每 10ms 最大转向误差变化
static uint8 s_special_handoff_ticks = 0U;
#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#endif

static float s_prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
static float prev_curve_f = 0.0f;

static void NavReplay_ResetProcessState(void)
{
    s_prev_err_degree = 0.0f;
    prev_speed_set = 0.0f;
    prev_curve_f = 0.0f;
#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif
}

// ========================= 辅助函数 =========================

/**
 * @brief  角度归一化 (-180 ~ 180)
 * @param  angle 原始角度
 * @return 归一化后的角度
 */
static float NormalizeAngle(float angle)
{
    while (angle > 180.0f)  angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

/**
 * @brief  计算两点间距离
 */
static float CalcDistance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static inline float CalcDistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

static int Find_Closest_Point_Index_Strict(int current_idx, int search_range)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 只往前搜，并且距离是相对于当前视觉惯导融合坐标
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(nav_vision_fusion_x, nav_vision_fusion_y, 
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
    }
    return closest_idx;
}

static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 5) return 0.0f;

    float max_curve = 0.0f;
    float check_dists[3] = {preview_dist * 0.4f, preview_dist * 0.7f, preview_dist};
    
    for(int step = 0; step < 3; step++) {
        float p_dist_sq = check_dists[step] * check_dists[step];
        int far_idx = start_idx;
        
        for (int i = start_idx; i < nav_ram_data.point_count; i++) {
            if (CalcDistanceSq(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y) >= p_dist_sq) {
                far_idx = i; break;
            }
            if (i > start_idx + 150) break;
        }
        
        if (far_idx > start_idx) {
            float dx = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
            float dy = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
            float path_angle = -atan2f(dy, -dx) * 57.29578f;
            float angle_diff = fabsf(NormalizeAngle(path_angle - inertial_nav.relative_yaw));
            
            float factor = (angle_diff / 60.0f) * (1.2f - 0.2f * step);
            if (factor > max_curve) max_curve = factor;
        }
    }
    return (max_curve > 1.0f) ? 1.0f : max_curve;
}

static float NavReplay_SpeedSlew_Update(float raw_speed)
{
    float abs_raw = fabsf(raw_speed);
    float abs_prev = fabsf(prev_speed_set);
    float diff = raw_speed - prev_speed_set;
    float step_limit;

    if (abs_raw > (abs_prev + NAV_SPEED_SLEW_EPS))
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

    return prev_speed_set + Float_Constrain(diff, -step_limit, step_limit);
}

static uint8 NavReplay_IsBridgeExitPoint(uint16 point_idx)
{
    return (uint8)((point_idx < nav_ram_data.point_count) &&
                   (nav_ram_data.points[point_idx].point_type == NAV_POINT_BRIDGE_EXIT));
}

static uint8 NavReplay_IsJumpExitPoint(uint16 point_idx)
{
    return (uint8)((point_idx < nav_ram_data.point_count) &&
                   (nav_ram_data.points[point_idx].point_type == NAV_POINT_JUMP_EXIT));
}

static uint8 NavReplay_IsBumpyExitPoint(uint16 point_idx)
{
    return (uint8)((point_idx < nav_ram_data.point_count) &&
                   (nav_ram_data.points[point_idx].point_type == NAV_POINT_BUMP_EXIT));
}

static uint8 NavReplay_IsSlopeExitPoint(uint16 point_idx)
{
    return (uint8)((point_idx < nav_ram_data.point_count) &&
                   (nav_ram_data.points[point_idx].point_type == NAV_POINT_SLOPE_EXIT));
}

static void NavReplay_CompleteVisionBridgeExit(void)
{
    // 40 只是视觉桥任务的退出锚点，不能再次作为普通特殊点触发状态机。
    if (NavReplay_IsBridgeExitPoint(g_target_idx))
    {
        if (g_bridge_vision_task_exit_reason == VISION_BRIDGE_EXIT_VISUAL_CONFIRMED)
        {
            nav_vision_fusion_x = nav_ram_data.points[g_target_idx].x;
            nav_vision_fusion_y = nav_ram_data.points[g_target_idx].y;
            exit_beep_request = 1U;
        }
        g_target_idx++;
    }

    s_bridge_exit_pending = 0U;
    s_special_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
    g_special_action_trigger = 0U;
}

static void NavReplay_CompleteVisionJumpExit(void)
{
    /* 30 仅是三级跳的退出锚点。只有状态机视觉确认完成时才允许重定位，
     * 防止超时/急停后把融合坐标错误钉到赛道后方。 */
    if (NavReplay_IsJumpExitPoint(g_target_idx))
    {
        if (g_vision_three_stage_control_status.exit_reason ==
            VISION_THREE_STAGE_EXIT_SUCCESS)
        {
            nav_vision_fusion_x = nav_ram_data.points[g_target_idx].x;
            nav_vision_fusion_y = nav_ram_data.points[g_target_idx].y;
            exit_beep_request = 1U;
        }
        g_target_idx++;
    }

    s_jump_exit_pending = 0U;
    s_special_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
    g_special_action_trigger = 0U;
}

static void NavReplay_CompleteBumpyExit(void)
{
    if (NavReplay_IsBumpyExitPoint(g_target_idx))
    {
        g_target_idx++;
    }

    s_bumpy_exit_pending = 0U;
    s_special_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
    g_special_action_trigger = 0U;
}

static void NavReplay_CompleteVisionSlopeExit(void)
{
    if (NavReplay_IsSlopeExitPoint(g_target_idx))
    {
        g_target_idx++;
    }

    s_slope_exit_pending = 0U;
    s_special_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
    g_special_action_trigger = 0U;
}

// ========================= 接口实现 =========================

uint16 NavReplay_LoadStaticRouteToRam(void)
{
#if NAV_REPLAY_USE_STATIC_ROUTE_TABLE
    uint16 load_count = NAV_REPLAY_STATIC_ROUTE_COUNT;
    if (load_count > NAV_RAM_MAX_POINTS)
    {
        load_count = NAV_RAM_MAX_POINTS;
    }
    nav_ram_data.plan_type = NAV_PLAN_1;
    nav_ram_data.point_count = load_count;
    for (uint16 i = 0; i < load_count; i++)
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

    g_target_idx = 0; // 从第1个点开始（起始点没有储存，默认为(0,0)）
    g_replay_state = REPLAY_RUNNING;
    g_special_action_trigger = 0;
    s_jump_exit_pending = 0U;
    s_bridge_exit_pending = 0U;
    s_bumpy_exit_pending = 0U;
    s_slope_exit_pending = 0U;
    s_special_handoff_ticks = 0U;
    NavReplay_ResetProcessState();
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay START. Plan: %d, Total Points: %d\r\n", 
           nav_ram_data.plan_type, nav_ram_data.point_count);
    #endif
}

void NavReplay_Stop(void)
{
    target_speed_set = 0.0f;
    g_replay_state = REPLAY_IDLE;
    err_degree = 0.0f;
    g_special_action_trigger = 0;
    s_jump_exit_pending = 0U;
    s_bridge_exit_pending = 0U;
    s_bumpy_exit_pending = 0U;
    s_slope_exit_pending = 0U;
    s_special_handoff_ticks = 0U;
    NavReplay_ResetProcessState();
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
    #endif
}

// ============================================================================
// 纯追踪复刻处理逻辑 (Pure Pursuit + 曲率前瞻)
// 用于科三状态机之间的高速平滑过渡
// ============================================================================

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING)
    {
        s_prev_err_degree = 0.0f; 
        return;
    }

    // 三级跳结束后，消费紧邻的 30 退出锚点；只有视觉确认的正常脱出才重定位。
    if (s_jump_exit_pending)
    {
        if (!VisionThreeStageControl_IsActive())
        {
            NavReplay_CompleteVisionJumpExit();
            s_prev_err_degree = 0.0f;
            prev_speed_set = 0.0f; // 脱出重置
        }
        else
        {
            return;
        }
    }

    // 视觉桥任务结束后，将融合坐标钉到 40 退出锚点并直接进入后续普通点。
    if (s_bridge_exit_pending)
    {
        if (!VisionBridgeTask_IsActive())
        {
            NavReplay_CompleteVisionBridgeExit();
            s_prev_err_degree = 0.0f;
            prev_speed_set = 0.0f;
        }
        else
        {
            return;
        }
    }

    // 颠簸路段状态机结束后，将退出锚点消费掉
    if (s_bumpy_exit_pending)
    {
        if (BumpyRoad_Is_Active() == 0U)
        {
            NavReplay_CompleteBumpyExit();
            s_prev_err_degree = 0.0f;
            prev_speed_set = 0.0f;
        }
        else
        {
            return;
        }
    }

    // 坡道状态机结束后，将退出锚点消费掉
    if (s_slope_exit_pending)
    {
        if (VisionSlopeTask_IsActive() == 0U)
        {
            NavReplay_CompleteVisionSlopeExit();
            s_prev_err_degree = 0.0f;
            prev_speed_set = 0.0f;
        }
        else
        {
            return;
        }
    }

    if (g_special_action_trigger == 1)
    {
        s_prev_err_degree = 0.0f;
        prev_speed_set = 0.0f;
        return;
    }

    // 视觉状态机结束后，给一段强制直行的恢复时间，避免刚刚覆盖坐标后乱抖
    if (s_special_handoff_ticks > 0U)
    {
        err_degree = 0.0f;
        target_speed_set = NavReplay_SpeedSlew_Update(NAV_SPEED_FAST);
        s_prev_err_degree = 0.0f;
        s_special_handoff_ticks--;
        return;
    }

#if IMU_CATEGORY == 3
    // 开局起跑角度对齐
    if (!g_start_heading_aligned)
    {
        float heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        
        // 使用一个较小的常量 2.0f 作为起步对齐限幅
        if (heading_err > 2.0f) heading_err = 2.0f;
        if (heading_err < -2.0f) heading_err = -2.0f;
        
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading)) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
            err_degree = 0.0f;
            s_prev_err_degree = 0.0f;
            prev_speed_set = 0.0f;
        }
        else return;
    }
#endif

    // 1. 获取当前车辆在路径上的基准索引
    int scan_range = 80;
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, scan_range);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; 
        s_prev_err_degree = 0;
        return;
    }

    // 2. 往前扫描，寻找即将到来的特殊点以及计算其真实距离
    int special_idx = -1;
    float dist_to_special = 99999.0f;
    // 扫描范围 100个点
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 100; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            special_idx = i;
            dist_to_special = CalcDistance(nav_vision_fusion_x, nav_vision_fusion_y, 
                                           nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            break;
        }
    }

    // 3. 动态触发状态机 (无需停车，带着速度冲入)
    // 条件：距离极近 (NAV_DIST_ARRIVE) 且索引已经到了该点附近
    if (special_idx != -1 && dist_to_special <= NAV_DIST_ARRIVE && base_idx >= special_idx - 2)
    {
        g_current_point_type = nav_ram_data.points[special_idx].point_type;

        #if DEBUG_LOG_ENABLE
        printf("[Nav] Passing Special Point[%d] Type[%d], triggering action!\r\n", special_idx, g_current_point_type);
        #endif

        if (g_current_point_type != NAV_POINT_PATH)
        {
            if (g_current_point_type == NAV_POINT_CIRCLE) {
                minefield_flag = 1;
            }
            else if (g_current_point_type == NAV_POINT_JUMP)
            {
                entry_beep_request = 1U;
                s_jump_exit_pending = NavReplay_IsJumpExitPoint(special_idx + 1U);
                VisionThreeStageControl_Start();
            }
            else if (g_current_point_type == NAV_POINT_BRIDGE)
            {
                entry_beep_request = 1U;
                s_bridge_exit_pending = NavReplay_IsBridgeExitPoint(special_idx + 1U);
                VisionBridgeTask_Start();
            }
            else if (g_current_point_type == NAV_POINT_BUMP)
            {
                s_bumpy_exit_pending = NavReplay_IsBumpyExitPoint(special_idx + 1U);
                if (s_bumpy_exit_pending)
                {
                    BumpyRoad_SetExitAnchor(nav_ram_data.points[special_idx + 1U].x,
                                             nav_ram_data.points[special_idx + 1U].y);
                }
                BumpyRoad_Trigger();
            }
            else if (g_current_point_type == NAV_POINT_SLOPE)
            {
                s_slope_exit_pending = NavReplay_IsSlopeExitPoint(special_idx + 1U);
                if (s_slope_exit_pending)
                {
                    VisionSlopeTask_SetExitAnchor(nav_ram_data.points[special_idx + 1U].x,
                                                  nav_ram_data.points[special_idx + 1U].y);
                }
                VisionSlopeTask_Start();
            }
            g_special_action_trigger = 1U;
            g_target_idx = special_idx + 1; // 强行跨过该特殊点，防止重复触发
        }
        return; // 直接 return，本周期由状态机接管
    }

    // 4. Pure Pursuit 动态极限前瞻计算
    float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
    float lookahead_dist_sq = lookahead_dist * lookahead_dist;

    // 寻找前瞻点
    float tx = nav_ram_data.points[base_idx].x;
    float ty = nav_ram_data.points[base_idx].y;
    int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
    if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

    for (int i = base_idx; i < ld_scan_limit; i++) {
        float d_sq = CalcDistanceSq(nav_vision_fusion_x, nav_vision_fusion_y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
        if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
            break;
        }
    }

    // 计算纯追踪的基准偏航角
    float pp_target_yaw = -atan2f(ty - nav_vision_fusion_y, -(tx - nav_vision_fusion_x)) * 57.29578f;
    float final_target_yaw = pp_target_yaw;
    float yaw_align_penalty_speed_factor = 1.0f;

    // 5. 逼近特殊任务点时的入站强行对齐逻辑
    if (special_idx != -1 && dist_to_special < 1000.0f)
    {
        float special_target_yaw = nav_ram_data.points[special_idx].target_yaw_deg;
        
        // 距离越近，越信任 special_target_yaw (在 1000mm 内线性过渡)
        float blend_ratio = (1000.0f - dist_to_special) / 1000.0f;
        if (blend_ratio > 1.0f) blend_ratio = 1.0f;
        if (blend_ratio < 0.0f) blend_ratio = 0.0f;

        float yaw_diff = NormalizeAngle(special_target_yaw - pp_target_yaw);
        final_target_yaw = NormalizeAngle(pp_target_yaw + yaw_diff * blend_ratio);
        
        // 入站前如果姿态偏差特别大，引入轻微减速惩罚机制以换取对齐时间
        float current_yaw_err = fabsf(NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw));
        if (current_yaw_err > NAV_YAW_TOLERANCE * 4.0f) {
            yaw_align_penalty_speed_factor = 0.4f; // 降至 40% 速度
        } else if (current_yaw_err > NAV_YAW_TOLERANCE * 2.0f) {
            yaw_align_penalty_speed_factor = 0.6f; // 降至 60% 速度
        } else if (current_yaw_err > NAV_YAW_TOLERANCE) {
            float p = (current_yaw_err - NAV_YAW_TOLERANCE) / NAV_YAW_TOLERANCE; // 0 ~ 1
            yaw_align_penalty_speed_factor = 1.0f - 0.4f * p; // 100% 降至 60%
        }
    }

    // 误差角度计算
    float raw_err_degree = NormalizeAngle(final_target_yaw - inertial_nav.relative_yaw);

    // 6. 曲率计算与在线速度规划
    float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
    
    if (curve_f < prev_curve_f) {
        curve_f *= 0.4f; // 出弯提早加速
    }
    prev_curve_f = curve_f;

    if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
    else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
    curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

    float current_max_spd = NAV_SPEED_FAST;
    float raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;

    // 叠加入站对齐降速惩罚
    raw_spd *= yaw_align_penalty_speed_factor;

    // 7. 最终平滑输出
    // 限制单次最大转向步长，防止舵机打手
    float diff = raw_err_degree - s_prev_err_degree;
    if (diff > SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree + SLEW_RATE_ANGLE;
    else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = s_prev_err_degree - SLEW_RATE_ANGLE;

    // 低通滤波平滑输出
    err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * s_prev_err_degree;
    target_speed_set = NavReplay_SpeedSlew_Update(raw_spd);

    s_prev_err_degree = err_degree;
    prev_speed_set = target_speed_set;
}

#endif
