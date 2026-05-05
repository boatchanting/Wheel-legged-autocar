#include "nav_replay.h"
#include "../common.h"
#include "nav_replay_route_table.h"
#include "../config/sys_options.h"
#include "vision/vision_bridge_control.h"

// ========================= 内部变量 =========================
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;                    // 当前正在前往的点索引
uint8 g_current_point_type = NAV_POINT_PATH;// 当前点的类型
uint8 g_special_action_trigger = 0;         // 触发标志

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

#if CURRENT_NAV_PLAN == 1 //如果是科目一，仅仅需要直线行驶即可，这步暂时不做特调的情况下，不需要状态机切换

// 外部/静态变量声明
static float prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
static float prev_curve_f = 0.0f;

#if IMU_CATEGORY == 3
#define NAV_START_ALIGN_MAX_ERR      25.0f
#define NAV_START_ALIGN_STABLE_COUNT 6U
#endif

// 高效平方距离计算
static inline float CalcDistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief 严格单向索引追踪
 * 强制要求索引只能在当前位置往后 [0, search_range] 范围内寻找。
 * 彻底解决在原路折返轨迹中，索引跳到回程路径的问题。
 * 严格单向索引追踪 (带防穿模锁 + 支持大范围重定位)
 * 强制要求索引只能在当前位置往后寻找，且绝不允许跳过特殊点！
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 只往后搜，不回头
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, 
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
        
        // 【核心修复】：只要扫描遇到特殊点，必须立刻终止！// 这个科目一不需要
        // 哪怕 current_idx 自己就是特殊点，也绝不允许再往后搜！死死冻结索引！
        // if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
        //     if (closest_idx > i) closest_idx = i;
        //     break; 
        // }
    }
    
    // 丢位保护：如果是重定位状态，豁免 800mm 限制！允许车子从远处强行切回主路
    if (!is_recovering && min_dist_sq > 800.0f * 800.0f) {
        return current_idx; 
    }
    return closest_idx;
}

/**
 * @brief 预判前方曲率因子
 */
static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 5) return 0.0f;

    float max_curve = 0.0f;
    // 分三段扫描前方 (近、中、远)，寻找最急的弯点
    float check_dists[3] = {preview_dist * 0.4f, preview_dist * 0.7f, preview_dist};
    
    for(int step = 0; step < 3; step++) {
        float p_dist_sq = check_dists[step] * check_dists[step];
        int far_idx = start_idx;
        
        // 同样限制扫描深度，防止扫过头
        for (int i = start_idx; i < nav_ram_data.point_count; i++) {
            if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break;
            if (CalcDistanceSq(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y) >= p_dist_sq) {
                far_idx = i; break;
            }
            if (i > start_idx + 150) break; // 扫描深度限制
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

uint8 is_arrived = 0;  // 到达判定状态锁

// 局部静态变量：用于滤波历史保持与下降沿检测
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 用于检测状态机结束的瞬间（下降沿）

/*这里注释了，保存的是Pure Pursuit 联合 特殊点直走 状态机*/

static void NavReplay_ResetProcessState(void)
{
    prev_err_degree = 0.0f;
    prev_speed_set = 0.0f;
    prev_curve_f = 0.0f;
    is_arrived = 0;
    s_is_aligning = 0;
    s_prev_trigger = 0;
#if IMU_CATEGORY == 3
    s_start_heading_stable_count = 0;
#endif
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

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING) return;
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

    // 如果状态机正在干预，记录状态并退出
    // if (g_special_action_trigger == 1) {
    //     s_prev_trigger = 1;
    //     return; 
    // }

    // ==========================================
    // 🎯 灾后重建机制 (Recovery)：检测状态机刚刚结束的瞬间
    // ==========================================
    uint8 is_recovering = 0; // 【注】为保证下方函数调用不报错，将其声明放出
    // if (s_prev_trigger == 1 && g_special_action_trigger == 0) {
    //     is_recovering = 1;
    //     s_prev_trigger = 0;
    //     is_arrived = 0;
        
    //     // 【关键】：清空历史包袱！
    //     // 防止车子把进入特殊点前的旧角度和速度带入到现在，导致突然猛打方向盘
    //     prev_err_degree = 0.0f;
    //     prev_speed_set = 0.0f;
    //     s_is_aligning = 0; 
        
    //     #if DEBUG_LOG_ENABLE
    //     printf("[Nav] Special Action Finished. Recovering back to route...\r\n");
    //     #endif
    // }

    // 1. 获取当前车辆在路径上的基准索引
    // 如果是刚刚结束状态机(is_recovering=1)，搜寻范围扩大到 300点(6米)，并豁免距离限制
    int scan_range = 80;
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, scan_range, is_recovering);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; 
        s_is_aligning = 0;
        return;
    }

    // ====================================================================
    // 👇 以下为寻找特殊点、去特殊点（模式A）和状态机的全部逻辑，已按要求整体注释
    // ====================================================================

    // // 2. 往前扫描，寻找即将到来的特殊点以及计算其真实距离
    // int special_idx = -1;
    // float dist_to_special = 99999.0f;
    // // 扫描范围 100个点(2000mm)
    // for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 100; i++) {
    //     if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
    //         special_idx = i;
    //         dist_to_special = CalcDistance(inertial_nav.x, inertial_nav.y, 
    //                                        nav_ram_data.points[i].x, nav_ram_data.points[i].y);
    //         break;
    //     }
    // }

    // // ====================================================================
    // // 双模式自动切换：1000mm 内进入"先转再走"模式；否则执行"高速 Pure Pursuit"
    // // ====================================================================
    // if (special_idx != -1 && dist_to_special <= 1000.0f)
    // {
    //     // -------------------------------------------------------------
    //     // 【模式A】精准逼近模式 (1000mm以内)：先转再走，绝对位置精准触发
    //     // -------------------------------------------------------------
    //     
    //     // 航向瞄准点计算：为了不抄近道，距离大于300mm时依然看路径前方，极近时直接看特殊点
    //     int aim_idx = base_idx + 15; // 往前看约300mm
    //     if (aim_idx > special_idx) aim_idx = special_idx;
    //     
    //     float tx = nav_ram_data.points[aim_idx].x;
    //     float ty = nav_ram_data.points[aim_idx].y;
    //
    //     float dx = tx - inertial_nav.x;
    //     float dy = ty - inertial_nav.y;
    //     float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    //     
    //     // 精准模式下，角度不做滤波，要求直接打到目标角度
    //     err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);
    //
    //     if (!is_arrived) {//根据状态锁判断
    //         // ==========================================
    //         // 【核心修复】：引入宽容到达判定，防止高速穿透
    //         // ==========================================
    //         if (dist_to_special <= NAV_DIST_ARRIVE) {
    //             is_arrived = 1; // 精确实达
    //         } 
    //         // 宽容判定：如果底层追踪索引已经被卡死在这个特殊点上了，
    //         // 且物理距离在稍大范围内(如 60mm 内)，说明车子因为惯性稍微冲过了一点，强制判作到达！
    //         // else if (base_idx == special_idx && dist_to_special <= NAV_DIST_ARRIVE + 40.0f) {
    //         //     is_arrived = 1;
    //         // }
    //     }
    //
    //     if (is_arrived)
    //     {
    //         // --- 1. 到达特殊点：触发动作 ---
    //         target_speed_set = NAV_SPEED_STOP;
    //         g_current_point_type = nav_ram_data.points[special_idx].point_type;
    //
    //         #if DEBUG_LOG_ENABLE
    //         printf("[Nav] Arrived Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
    //         #endif
    //
    //         // 提取该特殊点记录的目标偏航角
    //         float special_target_yaw = nav_ram_data.points[special_idx].target_yaw_deg;
    //         
    //         // 计算车身当前角度与目标角度的偏差
    //         float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);
    //
    //         // 判断角度是否对齐
    //         if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
    //         {
    //             // 角度还没对齐！将特殊点的角度误差喂给底层，触发原地自转对齐
    //             err_degree = special_yaw_err;
    //             
    //             #if DEBUG_LOG_ENABLE
    //             // printf("[Nav] Aligning Yaw at Special Point... err: %.2f\r\n", special_yaw_err);
    //             #endif
    //         }
    //         else
    //         {
    //             // 位置到了，角度也对齐了！正式触发状态机！
    //             g_current_point_type = nav_ram_data.points[special_idx].point_type;
    //
    //             #if DEBUG_LOG_ENABLE
    //             printf("[Nav] Arrived & Aligned Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
    //             #endif
    //
    //             if (g_current_point_type != NAV_POINT_PATH)
    //             {
    //                 if (g_current_point_type == NAV_POINT_CIRCLE) {
    //                     minefield_flag = 1;
    //                 }
    //                 if (g_current_point_type == NAV_POINT_JUMP) {
    //                     vision_detected_three_jump_point = 1;//触发三级跳状态机
    //                 }
    //                 if (g_current_point_type == NAV_POINT_BRIDGE) {
    //                     vision_detected_bridge_point = 1;//触发三桥桥状态机
    //                 }
    //                 if (g_current_point_type == NAV_POINT_BUMP) {
    //                     BumpyRoad_Trigger();  // 触发颠簸路段状态机
    //                 }
    //                 g_special_action_trigger = 1;
    //             }
    //             
    //             // 防死锁：动作触发后，强行跨过这个特殊点
    //             g_target_idx = special_idx + 1;
    //         }
    //     }
    //     else
    //     {
    //         // --- 2. 未到达特殊点：先转再走 ---
    //         if (fabsf(err_degree) > NAV_YAW_TOLERANCE)
    //         {
    //             // 角度偏差较大，先原地/极低速旋转
    //             target_speed_set = NAV_SPEED_STOP;
    //             #if DEBUG_LOG_ENABLE
    //             // printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
    //             #endif
    //         }
    //         else
    //         {
    //             // 角度基本对准，根据到特殊点的物理距离来规划速度
    //             if (dist_to_special > NAV_DIST_FAR)
    //             {
    //                 target_speed_set = NAV_SPEED_FAST/5.0f;
    //             }
    //             else if (dist_to_special > NAV_DIST_NEAR)
    //             {
    //                 float ratio = (dist_to_special - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
    //                 target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST/5.0f - NAV_SPEED_SLOW) * ratio;
    //             }
    //             else
    //             {
    //                 target_speed_set = NAV_SPEED_SLOW;
    //             }
    //         }
    //     }
    //     
    //     // 同步滤波历史，防止切回高速模式时车辆猛抖
    //     prev_err_degree = err_degree;
    //     prev_speed_set = target_speed_set;
    // }
    // else
    // {

    // ====================================================================
    // 👇 全程保持执行以下【模式B】高速 Pure Pursuit 寻迹模式代码
    // ====================================================================

    // -------------------------------------------------------------
    // 【模式B】高速 Pure Pursuit 寻迹模式 (距离特殊点 > 800mm 或无特殊点)
    // -------------------------------------------------------------
    
        // 动态极限前瞻计算
        float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
        float lookahead_dist_sq = lookahead_dist * lookahead_dist;

        // 极限选点 (寻找远方前瞻点)
        float tx = nav_ram_data.points[base_idx].x;
        float ty = nav_ram_data.points[base_idx].y;
        int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
        if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

        for (int i = base_idx; i < ld_scan_limit; i++) {
            float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
            if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
                break;
            }
        }

        // 计算航向
        float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
        float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        // 曲率计算与暴力速度规划
        float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
        
        if (curve_f < prev_curve_f) {
            curve_f *= 0.4f; // 出弯弹射
        }
        prev_curve_f = curve_f;

        if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
        else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
        curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

        float raw_spd = 0;
        float current_max_spd = (curve_f <= 0.0f) ? (NAV_SPEED_FAST * 1.3f) : NAV_SPEED_FAST;
        raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;
        
        // 3. 角度纠偏减速：极速行驶时，小偏差不减速，大偏差才微调
        float ang_p = fabsf(raw_err_degree) / SPD_ANGLE_TOLERANCE;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - SPD_ANGLE_PENALTY * ang_p);

        // 极速滤波输出
        float diff = raw_err_degree - prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree - SLEW_RATE_ANGLE;

        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * prev_err_degree;
        target_speed_set = FILTER_ALPHA_SPEED * raw_spd + (1.0f - FILTER_ALPHA_SPEED) * prev_speed_set;

        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    // }
}
#endif

#if CURRENT_NAV_PLAN == 2 //如果是科目二，这步暂时不做特调的情况下，仅仅需要雷区状态机，不需要进去的时候对准科目角度

// 外部/静态变量声明
static float prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
static float prev_curve_f = 0.0f;

// 高效平方距离计算
static inline float CalcDistanceSq(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return dx * dx + dy * dy;
}

/**
 * @brief 严格单向索引追踪
 * 强制要求索引只能在当前位置往后 [0, search_range] 范围内寻找。
 * 彻底解决在原路折返轨迹中，索引跳到回程路径的问题。
 * 严格单向索引追踪 (带防穿模锁 + 支持大范围重定位)
 * 强制要求索引只能在当前位置往后寻找，且绝不允许跳过特殊点！
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range, uint8 is_recovering)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 只往后搜，不回头
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, 
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
        
        // 【核心修复】：只要扫描遇到特殊点，必须立刻终止！
        // 哪怕 current_idx 自己就是特殊点，也绝不允许再往后搜！死死冻结索引！
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
            if (closest_idx > i) closest_idx = i;
            break; 
        }
    }
    
    // 丢位保护：如果是重定位状态，豁免 800mm 限制！允许车子从远处强行切回主路
    if (!is_recovering && min_dist_sq > 800.0f * 800.0f) {
        return current_idx; 
    }
    return closest_idx;
}

/**
 * @brief 预判前方曲率因子
 */
static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 5) return 0.0f;

    float max_curve = 0.0f;
    // 分三段扫描前方 (近、中、远)，寻找最急的弯点
    float check_dists[3] = {preview_dist * 0.4f, preview_dist * 0.7f, preview_dist};
    
    for(int step = 0; step < 3; step++) {
        float p_dist_sq = check_dists[step] * check_dists[step];
        int far_idx = start_idx;
        
        // 同样限制扫描深度，防止扫过头
        for (int i = start_idx; i < nav_ram_data.point_count; i++) {
            if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break;
            if (CalcDistanceSq(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y) >= p_dist_sq) {
                far_idx = i; break;
            }
            if (i > start_idx + 150) break; // 扫描深度限制
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
uint8 is_arrived = 0;  // 到达判定状态锁

// 局部静态变量：用于滤波历史保持与下降沿检测
static uint8 s_is_aligning = 0;
static uint8 s_prev_trigger = 0;  // 用于检测状态机结束的瞬间（下降沿）

/*这里注释了，保存的是Pure Pursuit 联合 特殊点直走 状态机*/

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING) return;

    // 如果状态机正在干预，记录状态并退出
    if (g_special_action_trigger == 1) {
        s_prev_trigger = 1;
        return; 
    }

    // ==========================================
    // 🎯 灾后重建机制 (Recovery)：检测状态机刚刚结束的瞬间
    // ==========================================
    uint8 is_recovering = 0;
    if (s_prev_trigger == 1 && g_special_action_trigger == 0) {
        is_recovering = 1;
        s_prev_trigger = 0;
        is_arrived = 0;
        
        // 【关键】：清空历史包袱！
        // 防止车子把进入特殊点前的旧角度和速度带入到现在，导致突然猛打方向盘
        prev_err_degree = 0.0f;
        prev_speed_set = 0.0f;
        s_is_aligning = 0; 
        
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Special Action Finished. Recovering back to route...\r\n");
        #endif
    }

    // 1. 获取当前车辆在路径上的基准索引
    // 如果是刚刚结束状态机(is_recovering=1)，搜寻范围扩大到 300点(6米)，并豁免距离限制
    int scan_range = is_recovering ? 300 : 80;
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, scan_range, is_recovering);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; 
        s_is_aligning = 0;
        return;
    }

    // 2. 往前扫描，寻找即将到来的特殊点以及计算其真实距离
    int special_idx = -1;
    float dist_to_special = 99999.0f;
    // 扫描范围 100个点(2000mm)
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 100; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            special_idx = i;
            dist_to_special = CalcDistance(inertial_nav.x, inertial_nav.y, 
                                           nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            break;
        }
    }

    // ====================================================================
    // 双模式自动切换：1000mm 内进入"先转再走"模式；否则执行"高速 Pure Pursuit"
    // ====================================================================
    if (special_idx != -1 && dist_to_special <= 1000.0f)
    {
        // -------------------------------------------------------------
        // 【模式A】精准逼近模式 (1000mm以内)：先转再走，绝对位置精准触发
        // -------------------------------------------------------------
        
        // 航向瞄准点计算：为了不抄近道，距离大于300mm时依然看路径前方，极近时直接看特殊点
        int aim_idx = base_idx + 15; // 往前看约300mm
        if (aim_idx > special_idx) aim_idx = special_idx;
        
        float tx = nav_ram_data.points[aim_idx].x;
        float ty = nav_ram_data.points[aim_idx].y;

        float dx = tx - inertial_nav.x;
        float dy = ty - inertial_nav.y;
        float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
        
        // 精准模式下，角度不做滤波，要求直接打到目标角度
        err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        if (!is_arrived) {//根据状态锁判断
            // ==========================================
            // 【核心修复】：引入宽容到达判定，防止高速穿透
            // ==========================================
            if (dist_to_special <= NAV_DIST_ARRIVE) {
                is_arrived = 1; // 精确到达
            } 
            // 宽容判定：如果底层追踪索引已经被卡死在这个特殊点上了，
            // 且物理距离在稍大范围内(如 60mm 内)，说明车子因为惯性稍微冲过了一点，强制判作到达！
            // else if (base_idx == special_idx && dist_to_special <= NAV_DIST_ARRIVE + 40.0f) {
            //     is_arrived = 1;
            // }
        }

        if (is_arrived)
        {
            // --- 1. 到达特殊点：触发动作 ---
            target_speed_set = NAV_SPEED_STOP;
            g_current_point_type = nav_ram_data.points[special_idx].point_type;

            #if DEBUG_LOG_ENABLE
            printf("[Nav] Arrived Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
            #endif

            // 提取该特殊点记录的目标偏航角
            // float special_target_yaw = nav_ram_data.points[special_idx].target_yaw_deg;
            
            // 计算车身当前角度与目标角度的偏差
            // float special_yaw_err = NormalizeAngle(special_target_yaw - inertial_nav.relative_yaw);

            // // 判断角度是否对齐
            // if (fabsf(special_yaw_err) > NAV_YAW_TOLERANCE)
            // {
            //     // 角度还没对齐！将特殊点的角度误差喂给底层，触发原地自转对齐
            //     err_degree = special_yaw_err;
                
            //     #if DEBUG_LOG_ENABLE
            //     // printf("[Nav] Aligning Yaw at Special Point... err: %.2f\r\n", special_yaw_err);
            //     #endif
            // }
            // else
            // {
                // 位置到了，角度也对齐了！正式触发状态机！【科目二优化】不要角度对齐
                g_current_point_type = nav_ram_data.points[special_idx].point_type;

                #if DEBUG_LOG_ENABLE
                printf("[Nav] Arrived & Aligned Special Point[%d] Type[%d]\r\n", special_idx, g_current_point_type);
                #endif

                if (g_current_point_type != NAV_POINT_PATH)
                {
                    if (g_current_point_type == NAV_POINT_CIRCLE) {
                        minefield_flag = 1;
                    }
                    // if (g_current_point_type == NAV_POINT_JUMP) {
                    //     vision_detected_three_jump_point = 1;//触发三级跳状态机
                    // }
                    // if (g_current_point_type == NAV_POINT_BRIDGE) {
                    //     vision_detected_bridge_point = 1;//触发三桥桥状态机
                    // }
                    // if (g_current_point_type == NAV_POINT_BUMP) {
                    //     BumpyRoad_Trigger();  // 触发颠簸路段状态机
                    // }
                    g_special_action_trigger = 1;
                }
                
                // 防死锁：动作触发后，强行跨过这个特殊点
                g_target_idx = special_idx + 1;
            // }
        }
        else
        {
            // --- 2. 未到达特殊点：先转再走 ---
            if (fabsf(err_degree) > NAV_YAW_TOLERANCE)
            {
                // 角度偏差较大，先原地/极低速旋转
                target_speed_set = NAV_SPEED_STOP;
                #if DEBUG_LOG_ENABLE
                // printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
                #endif
            }
            else
            {
                // 角度基本对准，根据到特殊点的物理距离来规划速度
                if (dist_to_special > NAV_DIST_FAR)
                {
                    target_speed_set = NAV_SPEED_FAST/5.0f;
                }
                else if (dist_to_special > NAV_DIST_NEAR)
                {
                    float ratio = (dist_to_special - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                    target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST/5.0f - NAV_SPEED_SLOW) * ratio;
                }
                else
                {
                    target_speed_set = NAV_SPEED_SLOW;
                }
            }
        }
        
        // 同步滤波历史，防止切回高速模式时车辆猛抖
        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    }
    else
    {
        // -------------------------------------------------------------
        // 【模式B】高速 Pure Pursuit 寻迹模式 (距离特殊点 > 800mm 或无特殊点)
        // -------------------------------------------------------------
        
        // 动态极限前瞻计算
        float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
        float lookahead_dist_sq = lookahead_dist * lookahead_dist;

        // 极限选点 (寻找远方前瞻点)
        float tx = nav_ram_data.points[base_idx].x;
        float ty = nav_ram_data.points[base_idx].y;
        int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
        if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

        for (int i = base_idx; i < ld_scan_limit; i++) {
            float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
            if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
                break;
            }
        }

        // 计算航向
        float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
        float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

        // 曲率计算与暴力速度规划
        float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
        
        if (curve_f < prev_curve_f) {
            curve_f *= 0.4f; // 出弯弹射
        }
        prev_curve_f = curve_f;

        if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
        else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
        curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

        float raw_spd = 0;
        float current_max_spd = (curve_f <= 0.0f) ? (NAV_SPEED_FAST * 1.3f) : NAV_SPEED_FAST;
        raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;
        
        // 3. 角度纠偏减速：极速行驶时，小偏差不减速，大偏差才微调
        float ang_p = fabsf(raw_err_degree) / SPD_ANGLE_TOLERANCE;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - SPD_ANGLE_PENALTY * ang_p);

        // 极速滤波输出
        float diff = raw_err_degree - prev_err_degree;
        if (diff > SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree + SLEW_RATE_ANGLE;
        else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree - SLEW_RATE_ANGLE;

        err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * prev_err_degree;
        target_speed_set = FILTER_ALPHA_SPEED * raw_spd + (1.0f - FILTER_ALPHA_SPEED) * prev_speed_set;

        prev_err_degree = err_degree;
        prev_speed_set = target_speed_set;
    }
}
#endif

#if CURRENT_NAV_PLAN == 3 //如果是科目二，这步暂时不做特调的情况下，不需要雷区状态机，进去的时候对准科目角度
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
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) 
    {
        s_prev_err_degree = 0.0f; 
        s_is_aligning = 0; // 状态机接管或停止时，确保解锁
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
    float dx = tx - inertial_nav.x;
    float dy = ty - inertial_nav.y;
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

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
                else if (g_current_point_type == NAV_POINT_BRIDGE) VisionBridgeTask_Start();
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

/*这里注释了，保存的是原有的到一个点停一次的控制逻辑，仅仅能实现最基本的到达，但它的控制距离是精准的，逻辑是完备的，后面所有的代码都在其基础上进行优化和尝试*/
/*
void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

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


    // 1. 检查是否跑完全部点位
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

    // 2. 获取当前目标点数据
    float tx = nav_ram_data.points[g_target_idx].x;
    float ty = nav_ram_data.points[g_target_idx].y;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    // 3. 计算距离和期望角度
    // 假设 inertial_nav 是全局结构体，x, y, relative_yaw 实时更新
    float dx = tx - inertial_nav.x;
    float dy = ty - inertial_nav.y;
    float dist = CalcDistance(inertial_nav.x, inertial_nav.y, tx, ty);

    // 计算期望方位角 (atan2 返回弧度值，转为角度)
    // 根据描述：X正方向向后，Y正方向向右，符合标准笛卡尔坐标旋转。
    float target_yaw = -atan2f(dy, -dx) * 57.29578f; 
    
    // err_degree = 期望 - 实际
    err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 5. 控制策略：先转再走
    if (dist <= NAV_DIST_ARRIVE)
    {
        // --- A. 到达目标点 ---
        target_speed_set = NAV_SPEED_STOP;
        
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Arrived Point[%d] Type[%d]\r\n", g_target_idx, g_current_point_type);
        #endif

        if (g_current_point_type != NAV_POINT_PATH)//处理特殊点
        {
             if (g_current_point_type == NAV_POINT_CIRCLE) {
                minefield_flag = 1;
            }
            g_special_action_trigger = 1;
        }
        
        g_target_idx++;
    }
    else
    {
        // --- B. 未到达目标点 ---
        // 先检查角度是否对准
        if (fabsf(NormalizeAngle(err_degree)) > NAV_YAW_TOLERANCE)
        {
            // 角度偏差较大，先原地旋转
            target_speed_set = NAV_SPEED_STOP;
            #if DEBUG_LOG_ENABLE
            printf("[Nav] Rotating to target, err: %.2f\r\n", err_degree);
            #endif
        }
        else
        {
            // 角度基本对准，开始移动
            if (dist > NAV_DIST_FAR)
            {
                // 远程段：满速行驶
                target_speed_set = NAV_SPEED_FAST;
            }
            else if (dist > NAV_DIST_NEAR)
            {
                // 减速段：线性插值减速
                float ratio = (dist - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR);
                target_speed_set = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
            }
            else
            {
                // 精准逼近段：极低速
                target_speed_set = NAV_SPEED_SLOW;
            }
        }
    }
}
*/
// 【使用说明】
//  // 惯导复现控制循环 (建议放在 20ms 定时器中)
//         // if (timer_20ms_flag) {
//             NavReplay_Process(); 
//         // }

//         // === 处理特殊点逻辑 ===
//         if (g_replay_state == REPLAY_RUNNING && g_special_action_trigger)
//         {
//             switch (g_current_point_type)
//             {
//                 case NAV_POINT_CIRCLE:
//                     // 暂停复现，执行转圈状态机
//                     // Run_Circle_Task();
//                     // 任务完成后清除标志
//                     break;
//                 case NAV_POINT_JUMP:
//                     // 只有在点类型为跳跃点时，可能需要加速冲过去
//                     // Override_Speed_For_Jump();
//                     break;
//                 // ... 其他类型
//             }
//         }
        
//         // 底层电机控制 (使用 target_speed_set 和 err_degree)
//         // Motor_Control(target_speed_set, err_degree);
