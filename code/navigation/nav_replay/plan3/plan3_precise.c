#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#include "../../../vision/vision_bridge_control.h"
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
// 单边桥入口已交给视觉任务后，等待视觉任务结束并消费紧邻的 40 退出锚点。
static uint8 s_bridge_exit_pending = 0U;
#define NAV_BRIDGE_HANDOFF_TICKS            (10U)  // 100ms：视觉退出控制平滑切换至 Plan3
#define NAV_BRIDGE_HANDOFF_SPEED_STEP        (3.0f) // 每 10ms 最大速度目标变化
#define NAV_BRIDGE_HANDOFF_ERR_STEP_DEG      (0.5f) // 每 10ms 最大转向误差变化
static uint8 s_bridge_handoff_ticks = 0U;

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
    s_bridge_handoff_ticks = NAV_BRIDGE_HANDOFF_TICKS;
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
    s_bridge_exit_pending = 0U;
    s_bridge_handoff_ticks = 0U;
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
    s_bridge_exit_pending = 0U;
    s_bridge_handoff_ticks = 0U;
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

// 局部静态变量：用于滤波历史保持与下降沿检测
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 用于检测状态机结束的瞬间（下降沿）
// 局部静态变量，用于记录历史角度和状态锁

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING)
    {
        s_prev_err_degree = 0.0f; 
        s_is_aligning = 0; // 状态机接管或停止时，确保解锁
        return;
    }

    // 视觉桥任务结束后，将融合坐标钉到 40 退出锚点并直接进入后续普通点。
    if (s_bridge_exit_pending)
    {
        if (!VisionBridgeTask_IsActive())
        {
            NavReplay_CompleteVisionBridgeExit();
            s_prev_err_degree = 0.0f;
            s_is_aligning = 0U;
        }
        else
        {
            return;
        }
    }

    if (g_special_action_trigger == 1)
    {
        s_prev_err_degree = 0.0f;
        s_is_aligning = 0U;
        return;
    }

    // 视觉状态机结束后，先以最后的视觉控制量向下一个点平滑过渡，避免速度/转向突变。
    if (s_bridge_handoff_ticks > 0U)
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
            s_bridge_handoff_ticks = 0U;
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
        s_bridge_handoff_ticks--;
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
        s_is_aligning = 0;
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

    // 4. 控制策略：先转再走
    // 🌟 核心修改：如果距离够近，或者【已经被锁在对齐状态中】，都强行进入到达逻辑！🌟
    if (dist <= NAV_DIST_ARRIVE || s_is_aligning)
    {
        // ==========================================
        // 【A. 已经到达目标点 (执行停车 / 对角)】
        // ==========================================
        target_speed_set = NAV_SPEED_STOP;

        if (g_current_point_type != NAV_POINT_PATH)
        {
            // 第一步：只要进来了，立刻锁死状态！即便下一帧 dist 变大了也不会退出去！
            s_is_aligning = 1; 

            // 第二步：开始专心对齐特殊点的角度
            float special_target_yaw = nav_ram_data.points[g_target_idx].target_yaw_deg;
            float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);

            if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
            {
                // 限幅保护，温柔转向
                if (special_yaw_err > MAX_SPIN_ERR) special_yaw_err = MAX_SPIN_ERR;
                if (special_yaw_err < -MAX_SPIN_ERR) special_yaw_err = -MAX_SPIN_ERR;
                
                err_degree = special_yaw_err;
                s_prev_err_degree = err_degree; 
            }
            else
            {
                // 位置到了，角度也转对了！正式触发状态机！
                if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1;
                else if (g_current_point_type == NAV_POINT_JUMP) vision_detected_three_jump_point = 1;
                else if (g_current_point_type == NAV_POINT_BRIDGE)
                {
                    // 桥入口、视觉桥任务真正结束时各鸣叫两声，便于实车确认交接时刻。
                    entry_beep_request = 1U;
                    s_bridge_exit_pending = NavReplay_IsBridgeExitPoint(g_target_idx + 1U);
                    VisionBridgeTask_Start();
                }
                else if (g_current_point_type == NAV_POINT_BUMP) BumpyRoad_Trigger();
                
                g_special_action_trigger = 1;
                g_target_idx++;     // 切向下一个点
                
                s_prev_err_degree = 0.0f;
                s_is_aligning = 0;  // 🌟 对齐完成，解除锁定！🌟
            }
        }
        else
        {
            // 普通路径点：到了直接切下一个点
            g_target_idx++;
            s_is_aligning = 0;      // 确保普通点不会被误锁
        }
    }
    else
    {
        // ==========================================
        // 【B. 未到达目标点 (还在路上)】
        // ==========================================
        
        // 距离点非常近时的抽搐保护
        if (dist < NAV_DIST_ARRIVE + 150.0f) {
            if (raw_err > 15.0f) raw_err = 15.0f;
            if (raw_err < -15.0f) raw_err = -15.0f;
        } 
        else {
            if (raw_err > MAX_APPROACH_ERR) raw_err = MAX_APPROACH_ERR;
            if (raw_err < -MAX_APPROACH_ERR) raw_err = -MAX_APPROACH_ERR;
        }

        err_degree = ANGLE_FILTER_ALPHA * raw_err + (1.0f - ANGLE_FILTER_ALPHA) * s_prev_err_degree;
        s_prev_err_degree = err_degree;

        // 检查车头是否对准目标点
        if (fabsf(NormalizeAngle(target_yaw - inertial_nav.relative_yaw)) > NAV_YAW_TOLERANCE)
        {
            target_speed_set = NAV_SPEED_STOP; // 角度偏大，原地转
        }
        else
        {
            // 角度基本对准，开始直线移动逼近
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
