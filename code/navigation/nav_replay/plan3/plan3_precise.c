#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
#include "../../../vision/vision_slope_control.h"
#include "../../../vision/vision_three_stage_control.h"
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
    s_special_handoff_ticks = 0U;
    s_arrival_target_idx = 0xFFFFU;
    s_arrival_min_dist = 0.0f;
    entry_beep_request = 0U;
    exit_beep_request = 0U;
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
    s_special_handoff_ticks = 0U;
    s_arrival_target_idx = 0xFFFFU;
    s_arrival_min_dist = 0.0f;
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
// 精准复刻处理逻辑 (点到点，先转再走，加入防震荡与平滑滤波)
// 慢慢的跑科三
// ============================================================================
// --- 角度平滑与防过冲参数 ---
#define MAX_SPIN_ERR        2.0f   // 原地对齐时的最大输出角度(度)。越小转得越柔和，建议 20-40，彻底解决原地打转过冲！
#define MAX_APPROACH_ERR    4.0f   // 直线逼近时的最大转角(度)。防止车子在行进中猛烈变道。
#define ANGLE_FILTER_ALPHA  0.3f    // 角度滤波系数(0~1)。越小越丝滑，越大越跟手。防止在点旁边抽搐。

static float s_prev_err_degree = 0.0f; // 用于角度滤波的静态变量
uint8 is_arrived = 0;  // 到达判定状态锁

static uint8 NavReplay_TargetArrived(uint16 target_idx, float dist)
{
    if (target_idx != s_arrival_target_idx)
    {
        s_arrival_target_idx = target_idx;
        s_arrival_min_dist = dist;
    }
    else if (dist < s_arrival_min_dist)
    {
        s_arrival_min_dist = dist;
    }

    if (dist <= NAV_DIST_ARRIVE)
    {
        return 1U;
    }

    return (uint8)((s_arrival_min_dist <= NAV_DIST_PASS_CAPTURE) &&
                   (dist >= (s_arrival_min_dist + NAV_DIST_PASS_HYSTERESIS)));
}

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
        }
        else
        {
            return;
        }
    }

    // 颠簸状态机结束后，将 50 退出锚点消费掉；视觉异常自动结束时不重定位。
    if (s_bumpy_exit_pending)
    {
        if (!BumpyRoad_Is_Active())
        {
            NavReplay_CompleteVisionBumpyExit();
            s_prev_err_degree = 0.0f;
        }
        else
        {
            return;
        }
    }

    if (g_special_action_trigger == 1)
    {
        s_prev_err_degree = 0.0f;
        return;
    }

    // 视觉状态机结束后，先以最后的视觉控制量向下一个点平滑过渡，避免速度/转向突变。
    if (s_special_handoff_ticks > 0U)
    {
        float nav_x = nav_vision_fusion_x;
        float nav_y = nav_vision_fusion_y;
        float tx;
        float ty;
        float dist;
        float target_yaw;
        float desired_err;
        float desired_speed;

        if (g_target_idx >= nav_ram_data.point_count)
        {
            s_special_handoff_ticks = 0U;
            target_speed_set = NAV_SPEED_STOP;
            err_degree = 0.0f;
            return;
        }

        tx = nav_ram_data.points[g_target_idx].x;
        ty = nav_ram_data.points[g_target_idx].y;
        dist = CalcDistance(nav_x, nav_y, tx, ty);
        target_yaw = -atan2f(ty - nav_y, -(tx - nav_x)) * 57.29578f;
        desired_err = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);
        if (desired_err > MAX_APPROACH_ERR) desired_err = MAX_APPROACH_ERR;
        if (desired_err < -MAX_APPROACH_ERR) desired_err = -MAX_APPROACH_ERR;

        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
        {
            desired_speed = NAV_SPEED_STOP;
        }
        else if (dist > NAV_DIST_FAR)
        {
            desired_speed = NAV_SPEED_FAST;
        }
        else if (dist > NAV_DIST_NEAR)
        {
            float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
            desired_speed = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
        }
        else
        {
            desired_speed = NAV_SPEED_SLOW;
        }

        err_degree = NavReplay_RampFloat(err_degree, desired_err, NAV_BRIDGE_HANDOFF_ERR_STEP_DEG);
        target_speed_set = NavReplay_RampFloat(target_speed_set, desired_speed, NAV_BRIDGE_HANDOFF_SPEED_STEP);
        s_prev_err_degree = err_degree;
        s_special_handoff_ticks--;
        return;
    }

#if IMU_CATEGORY == 3
    // 开局起跑角度对齐
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
            s_prev_err_degree = 0.0f;
        }
        else return;
    }
#endif

    // 1. 检查是否跑完全部点位
    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    // 2. 获取当前目标点数据
    float tx = nav_ram_data.points[g_target_idx].x;
    float ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    // 3. 计算距离和期望位置角度
    float nav_x = nav_vision_fusion_x;
    float nav_y = nav_vision_fusion_y;
    float dx = tx - nav_x;
    float dy = ty - nav_y;
    float dist = CalcDistance(nav_x, nav_y, tx, ty);

    float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    float raw_err = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 4. 控制策略：不在点前降速；进入捕获范围或已穿点时直接切换状态机。
    if (NavReplay_TargetArrived(g_target_idx, dist))
    {
        if (g_current_point_type != NAV_POINT_PATH)
        {
            float entry_yaw_err = NormalizeAngle(
                nav_ram_data.points[g_target_idx].target_yaw_deg - inertial_nav.relative_yaw);

            // Require the recorded heading as well as the target position before
            // handing control to a special-point state machine.
            if (fabsf(entry_yaw_err) > NAV_SPECIAL_ENTRY_YAW_TOLERANCE)
            {
                if (entry_yaw_err > MAX_SPIN_ERR) entry_yaw_err = MAX_SPIN_ERR;
                if (entry_yaw_err < -MAX_SPIN_ERR) entry_yaw_err = -MAX_SPIN_ERR;

                target_speed_set = NAV_SPEED_STOP;
                err_degree = entry_yaw_err;
                s_prev_err_degree = entry_yaw_err;
                return;
            }

            // 特殊点到达后直接把控制权交给对应状态机，不停车、不原地对角。
            //if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1;//科目三没有雷区逻辑
            if (g_current_point_type == NAV_POINT_SLOPE)
            {
                g_slope_vision_task_enable = 1U;
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
                    BumpyRoad_SetRouteAnchors(nav_ram_data.points[g_target_idx].x,
                                              nav_ram_data.points[g_target_idx].y,
                                              nav_ram_data.points[g_target_idx + 1U].x,
                                              nav_ram_data.points[g_target_idx + 1U].y);
                }
                BumpyRoad_Trigger();
            }

            g_special_action_trigger = 1U;
            g_target_idx++;
            s_prev_err_degree = 0.0f;
        }
        else
        {
            // 普通路径点：到达或穿过后直接切下一个点。
            g_target_idx++;
            s_prev_err_degree = 0.0f;
        }
    }
    else
    {
        if (raw_err > MAX_APPROACH_ERR) raw_err = MAX_APPROACH_ERR;
        if (raw_err < -MAX_APPROACH_ERR) raw_err = -MAX_APPROACH_ERR;

        err_degree = ANGLE_FILTER_ALPHA * raw_err + (1.0f - ANGLE_FILTER_ALPHA) * s_prev_err_degree;
        s_prev_err_degree = err_degree;

        // 如果角度偏差过大，则原地转向；否则快速逼近
        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
        {
            target_speed_set = NAV_SPEED_STOP;
        }
        else
        {
            target_speed_set = NAV_SPEED_FAST;
        }
    }
}

#endif
