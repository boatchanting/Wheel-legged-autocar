#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../vision/vision_three_stage_control.h"
#include "../../../vision/vision_slope_control.h"
#include "../../../vision/vision_bumpy_control.h"
#include <math.h>
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

#ifndef NAV_SPEED_SLEW_DOWN_FAST
#define NAV_SPEED_SLEW_DOWN_FAST 95.0f
#endif

static uint8 g_start_heading_aligned = 1;
// 特殊任务入口已交给视觉状态机后，等待其结束并消费紧邻的退出锚点。
static uint8 s_jump_exit_pending = 0U;
static uint8 s_bridge_exit_pending = 0U;
static uint8 s_bumpy_exit_pending = 0U;
static uint8 s_slope_exit_pending = 0U;
static float s_bumpy_vision_err_filtered = 0.0f;
#define NAV_BRIDGE_HANDOFF_TICKS            (10U)  // 100ms：视觉退出控制平滑切换至 Plan3
#define NAV_BRIDGE_HANDOFF_SPEED_STEP        (3.0f) // 每 10ms 最大速度目标变化
#define NAV_BRIDGE_HANDOFF_ERR_STEP_DEG      (0.5f) // 每 10ms 最大转向误差变化
static uint8 s_special_handoff_ticks = 0U;
// 当前目标点的最近距离，用于判定“已经穿过目标点”，避免错过小半径后回头。
static uint16 s_arrival_target_idx = 0xFFFFU;
static float s_arrival_min_dist = 0.0f;

#if IMU_CATEGORY == 3
static uint8 s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1
static void NavReplay_ResetProcessState(void);
#endif

static float s_prev_speed_cmd = 0.0f;
static float s_special_brake_dist_ratio = 1.0f;
static uint8 s_special_zero_brake_issued = 0U;

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

static float NavReplay_RampFloat(float current, float target, float step)
{
    if (current < target)
    {
        current += step;
        return (current > target) ? target : current;
    }
    if (current > target)
    {
        current -= step;
        return (current < target) ? target : current;
    }
    return current;
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

static void NavReplay_CompleteVisionSlopeExit(void)
{
    if (NavReplay_IsSlopeExitPoint(g_target_idx))
    {
        nav_vision_fusion_x = nav_ram_data.points[g_target_idx].x;
        nav_vision_fusion_y = nav_ram_data.points[g_target_idx].y;
        exit_beep_request = 1U;
        g_target_idx++;
    }

    s_slope_exit_pending = 0U;
    s_special_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
    g_special_action_trigger = 0U;
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

static void NavReplay_CompleteVisionBumpyExit(void)
{
    if (NavReplay_IsBumpyExitPoint(g_target_idx))
    {
        g_target_idx++;
    }

    s_bumpy_exit_pending = 0U;
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
    s_bumpy_vision_err_filtered = 0.0f;
    s_special_handoff_ticks = 0U;
    s_arrival_target_idx = 0xFFFFU;
    s_arrival_min_dist = 0.0f;
    entry_beep_request = 0U;
    exit_beep_request = 0U;
    s_prev_speed_cmd = 0.0f;
    s_special_zero_brake_issued = 0U;
#if IMU_CATEGORY == 3
    g_start_heading_aligned = (NAV_REPLAY_START_HEADING_VALID == 1) ? 0 : 1;
    s_start_heading_stable_count = 0;
#else
    g_start_heading_aligned = 1;
#endif

#if CURRENT_NAV_PLAN == 1
    NavReplay_ResetProcessState();
#endif
    
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
    s_bumpy_vision_err_filtered = 0.0f;
    s_special_handoff_ticks = 0U;
    s_arrival_target_idx = 0xFFFFU;
    s_arrival_min_dist = 0.0f;
    s_prev_speed_cmd = 0.0f;
    s_special_zero_brake_issued = 0U;
    g_start_heading_aligned = 1;
#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif

#if CURRENT_NAV_PLAN == 1
    NavReplay_ResetProcessState();
#endif
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
    #endif
}

// ============================================================================
// 新版动态连续规划处理逻辑 (从 Plan2 Lite 移植)
// ============================================================================

static uint8 IsSpecialPointType(uint8 point_type)
{
    return (uint8)(point_type != NAV_POINT_PATH);
}

static float CalcBearingDeg(float x1, float y1, float x2, float y2)
{
    return -atan2f(y2 - y1, -(x2 - x1)) * 57.29578f;
}

// 速度斜率限制器
static float NavReplay_SpeedSlew_Update_Plan3(float raw_speed)
{
    float diff = raw_speed - s_prev_speed_cmd;
    float step_limit = NAV_SPEED_SLEW_DOWN_FAST;

    if (diff > step_limit)
    {
        raw_speed = s_prev_speed_cmd + step_limit;
    }
    else if (diff < -step_limit)
    {
        raw_speed = s_prev_speed_cmd - step_limit;
    }

    s_prev_speed_cmd = raw_speed;
    return raw_speed;
}

// 底层对正：计算指向目标点的偏航误差
static void SelectDriveHeading(float point_yaw_deg, float *selected_err_deg, float *speed_sign)
{
    *selected_err_deg = NormalizeAngle(point_yaw_deg - inertial_nav.relative_yaw);
    *speed_sign = -1.0f; // 始终向车尾方向行驶（假设原框架负速度代表前进）
}

// 基于距离和对正角的常规降速
static float PlanSpeedAbsByDistance(float dist_mm, float stop_radius_mm, float yaw_err_deg)
{
    float remain = dist_mm - stop_radius_mm;
    float speed_abs;

    if (remain <= 0.0f)
    {
        return 0.0f;
    }

    speed_abs = sqrtf(2.0f * NAV_POINT_SPEED_DECEL_CMD2_PER_MM * remain);
    
    if (speed_abs > fabsf((float)NAV_SPEED_FAST)) speed_abs = fabsf((float)NAV_SPEED_FAST);
    if (speed_abs < 0.0f) speed_abs = 0.0f;

    if (fabsf(yaw_err_deg) > NAV_POINT_YAW_SLOW_TOLERANCE)
    {
        speed_abs = 0.0f; // 角度误差太大，原地调整不发车
    }
    else if (fabsf(yaw_err_deg) > NAV_POINT_YAW_STOP_TOLERANCE)
    {
        speed_abs *= 0.35f; // 角度误差中等，限速行驶
    }

    return speed_abs;
}

// 计算刹车距离
static float CalcSpecialBrakeRadius(float v_actual)
{
    float v_mmps = fabsf(v_actual);
    float stop_dist = (0.00025f * v_mmps * v_mmps - 0.2877f * v_mmps + 887.0f) * s_special_brake_dist_ratio;
    if (stop_dist < 0.0f) stop_dist = 0.0f;
    return stop_dist;
}

// 混合刹车与带速度对正逻辑
static uint8 HandleSpecialPointStopAndTrigger_Plan3(float dist_to_point)
{
    float v_actual = fabsf(inertial_nav.vx_body); 
    float stop_dist_current = CalcSpecialBrakeRadius(v_actual);
    float stop_dist_target = CalcSpecialBrakeRadius(NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S);
    float required_brake_dist = stop_dist_current - stop_dist_target;
    float brake_radius_mm;
    float entry_yaw_err;
    float speed_cmd;

    if (required_brake_dist < 0.0f) required_brake_dist = 0.0f;
    brake_radius_mm = NAV_POINT_SPECIAL_EXECUTE_RADIUS + NAV_POINT_SPECIAL_BRAKE_MARGIN_MM + required_brake_dist;

    if (s_special_zero_brake_issued == 0U)
    {
        if (dist_to_point <= brake_radius_mm)
        {
            s_special_zero_brake_issued = 1U;
        }
    }

    if (s_special_zero_brake_issued != 0U)
    {
        if (dist_to_point <= NAV_POINT_SPECIAL_EXECUTE_RADIUS)
        {
            if (v_actual <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S + 50.0f)
            {
                if (g_current_point_type == NAV_POINT_BUMP)
                {
                    VisionBumpyControl_SetEnable(1U);
                    if (VisionBumpyControl_IsEnabled() && (g_vision_bumpy_control_status.state != VISION_BUMPY_CTRL_IDLE))
                    {
                        float raw_err = VisionBumpyControl_GetErrDegreeCmd();
                        s_bumpy_vision_err_filtered = 0.1f * raw_err + 0.9f * s_bumpy_vision_err_filtered;
                        entry_yaw_err = s_bumpy_vision_err_filtered;
                    }
                    else
                    {
                        entry_yaw_err = NormalizeAngle(nav_ram_data.points[g_target_idx].target_yaw_deg - inertial_nav.relative_yaw);
                        s_bumpy_vision_err_filtered = entry_yaw_err;
                    }
                }
                else
                {
                    entry_yaw_err = NormalizeAngle(nav_ram_data.points[g_target_idx].target_yaw_deg - inertial_nav.relative_yaw);
                }
                
                if (fabsf(entry_yaw_err) <= NAV_SPECIAL_ENTRY_YAW_TOLERANCE)
                {
                    g_special_action_trigger = 1U;
                    
                    if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1U;
                    else if (g_current_point_type == NAV_POINT_SLOPE)
                    {
                        entry_beep_request = 1U;
                        s_slope_exit_pending = NavReplay_IsSlopeExitPoint(g_target_idx + 1U);
                        VisionSlopeTask_Start();
                    }
                    else if (g_current_point_type == NAV_POINT_JUMP)
                    {
                        entry_beep_request = 1U;
                        s_jump_exit_pending = NavReplay_IsJumpExitPoint(g_target_idx + 1U);
                        VisionThreeStageControl_Start();
                    }
                    else if (g_current_point_type == NAV_POINT_BRIDGE)
                    {
                        entry_beep_request = 1U;
                        s_bridge_exit_pending = NavReplay_IsBridgeExitPoint(g_target_idx + 1U);
                        VisionBridgeTask_Start();
                    }
                    else if (g_current_point_type == NAV_POINT_BUMP)
                    {
                        s_bumpy_exit_pending = NavReplay_IsBumpyExitPoint(g_target_idx + 1U);
                        if (s_bumpy_exit_pending)
                        {
                            BumpyRoad_SetExitAnchor(nav_ram_data.points[g_target_idx + 1U].x,
                                                    nav_ram_data.points[g_target_idx + 1U].y);
                        }
                        BumpyRoad_Trigger();
                    }
                    return 2U;
                }
                else
                {
                    speed_cmd = - (NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S / SPEED_TO_MM_S);
                    target_speed_set = NavReplay_SpeedSlew_Update_Plan3(speed_cmd);
                    err_degree = entry_yaw_err;
                    return 0U;
                }
            }
            else
            {
                target_speed_set = 0.0f;
                s_prev_speed_cmd = 0.0f;
                return 0U;
            }
        }
        else
        {
            if (v_actual <= NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S + 50.0f)
            {
                speed_cmd = - (NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S / SPEED_TO_MM_S);
                target_speed_set = NavReplay_SpeedSlew_Update_Plan3(speed_cmd);
                return 0U;
            }
            else
            {
                target_speed_set = 0.0f;
                s_prev_speed_cmd = 0.0f;
                return 0U;
            }
        }
    }
    return 1U;
}

void NavReplay_Process(void)
{
    float tx, ty, nav_x, nav_y, dist_to_point;
    float point_yaw_deg, selected_err_deg, speed_sign, stop_radius, speed_mag;
    uint8 special_res;
#if IMU_CATEGORY == 3
    float heading_err;
#endif

    if (g_replay_state != REPLAY_RUNNING)
    {
        return;
    }

    if (s_slope_exit_pending)
    {
        if (!VisionSlopeTask_IsActive())
        {
            NavReplay_CompleteVisionSlopeExit();
        }
        else
        {
            return;
        }
    }
    if (s_jump_exit_pending)
    {
        if (!VisionThreeStageControl_IsActive())
        {
            NavReplay_CompleteVisionJumpExit();
        }
        else
        {
            return;
        }
    }
    if (s_bridge_exit_pending)
    {
        if (!VisionBridgeTask_IsActive())
        {
            NavReplay_CompleteVisionBridgeExit();
        }
        else
        {
            return;
        }
    }
    if (s_bumpy_exit_pending)
    {
        if (!BumpyRoad_Is_Active())
        {
            NavReplay_CompleteVisionBumpyExit();
        }
        else
        {
            return;
        }
    }

    if (g_special_action_trigger == 1)
    {
        return;
    }

    if (s_special_handoff_ticks > 0U)
    {
        float target_yaw, desired_err, desired_dist, desired_speed, ratio;
        
        nav_x = nav_vision_fusion_x;
        nav_y = nav_vision_fusion_y;
        
        if (g_target_idx >= nav_ram_data.point_count)
        {
            s_special_handoff_ticks = 0U;
            target_speed_set = NAV_SPEED_STOP;
            err_degree = 0.0f;
            return;
        }

        tx = nav_ram_data.points[g_target_idx].x;
        ty = nav_ram_data.points[g_target_idx].y;
        desired_dist = CalcDistance(nav_x, nav_y, tx, ty);
        target_yaw = CalcBearingDeg(nav_x, nav_y, tx, ty);
        desired_err = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE) 
        {
            desired_speed = NAV_SPEED_STOP;
        }
        else if (desired_dist > NAV_DIST_FAR) 
        {
            desired_speed = NAV_SPEED_FAST;
        }
        else if (desired_dist > NAV_DIST_NEAR)
        {
            ratio = (desired_dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
            desired_speed = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
        }
        else 
        {
            desired_speed = NAV_SPEED_SLOW;
        }

        err_degree = NavReplay_RampFloat(err_degree, desired_err, NAV_BRIDGE_HANDOFF_ERR_STEP_DEG);
        target_speed_set = NavReplay_RampFloat(target_speed_set, desired_speed, NAV_BRIDGE_HANDOFF_SPEED_STEP);
        s_special_handoff_ticks--;
        return;
    }

#if IMU_CATEGORY == 3
    if (!g_start_heading_aligned)
    {
        heading_err = NormalizeAngle(NAV_REPLAY_START_HEADING_DEG - heading);
        err_degree = heading_err;
        target_speed_set = NAV_SPEED_STOP;

        if (fabsf(heading_err) <= NAV_START_HEADING_TOLERANCE)
        {
            g_start_heading_aligned = 1;
            err_degree = 0.0f;
        }
        else return;
    }
#endif

    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    tx = nav_ram_data.points[g_target_idx].x;
    ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    nav_x = nav_vision_fusion_x;
    nav_y = nav_vision_fusion_y;
    dist_to_point = CalcDistance(nav_x, nav_y, tx, ty);

    if (g_current_point_type == NAV_POINT_PATH && dist_to_point <= NAV_DIST_ARRIVE)
    {
        g_target_idx++;
        s_special_zero_brake_issued = 0U;
        return;
    }

    point_yaw_deg = CalcBearingDeg(nav_x, nav_y, tx, ty);
    SelectDriveHeading(point_yaw_deg, &selected_err_deg, &speed_sign);

    if (IsSpecialPointType(g_current_point_type))
    {
        uint8 special_res = HandleSpecialPointStopAndTrigger_Plan3(dist_to_point);
        if (special_res == 2U)
        {
            if (g_target_idx < nav_ram_data.point_count - 1U)
            {
                g_target_idx++;
                s_special_zero_brake_issued = 0U;
            }
            else
            {
                g_replay_state = REPLAY_FINISHED;
            }
            return;
        }
        else if (special_res == 0U)
        {
            // 在刹车或对正阶段，HandleSpecialPointStopAndTrigger_Plan3 已经接管了 target_speed_set 和 err_degree
            if (s_special_zero_brake_issued != 0U && dist_to_point > NAV_POINT_SPECIAL_EXECUTE_RADIUS)
            {
                err_degree = selected_err_deg;
            }
            return;
        }
    }

    // 6. 普通路径规划降速
    err_degree = selected_err_deg;
    stop_radius = IsSpecialPointType(g_current_point_type) ? NAV_POINT_SPECIAL_EXECUTE_RADIUS : NAV_DIST_ARRIVE;
    speed_mag = PlanSpeedAbsByDistance(dist_to_point, stop_radius, selected_err_deg);
    
    target_speed_set = NavReplay_SpeedSlew_Update_Plan3(speed_sign * speed_mag);
}

#endif

