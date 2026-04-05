#include "nav_replay.h"
#include "../common.h"
#include "nav_replay_route_table.h"

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
    target_speed_set = 0.0f;
    g_replay_state = REPLAY_IDLE;
    err_degree = 0.0f;
    g_start_heading_aligned = 1;
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
    #endif
}

// =================================================================
// 【性能调优宏定义区】 - 修改此处参数即可改变行驶风格
// =================================================================
//这里注释了，保存的是pure pursuit算法的控制逻辑，逻辑是完备的，测试了科目一的逻辑,需要将上面的辅助函数开启，需要将参数修改到.h文件中，并且针对不同车辆配置进行调参【优化点】

// --- 1. 纯追踪 (Pure Pursuit) 导航参数 ---
#define PP_LD_MIN_CURVE        500.0f   // 弯道最小前瞻 (mm)。越小越贴线，但容易抖动。要求精度25mm建议不低于300。
#define PP_LD_MIN_STRAIGHT     1.2f     // 直道前瞻倍率。针对3m大点距，建议设为当前点距的1.1-1.5倍。
#define PP_LD_SPEED_GAIN       0.7f     // 速度增益系数。Ld = Ld_min + Speed * Gain。高速时看的更远。
#define CURVE_PREVIEW_DIST     1200.0f   // 曲率预判距离 (mm)。探测多远处的弯道，决定提早减速的时机。

// --- 2. 速度规划 (Speed Planning) 参数 ---
#define SPD_CURVE_DEADZONE     0.35f     // 曲率感应死区 (0-1)。低于此值的弯道视为直道，不减速，释放速度。
#define SPD_CURVE_EXPONENT     3.5f     // 曲率减速指数。1.0为线性，2.0为平方律。越大则轻微弯道速度越快。
#define SPD_ANGLE_PENALTY      0.08f     // 转向角度惩罚权重 (0-1)。值越小，纠偏时减速越少，动力更足。
#define SPD_ANGLE_TOLERANCE    60.0f    // 转向角度容忍门槛 (度)。角度偏差在此范围内不触发剧烈减速。

// --- 3. 丝滑滤波 (Smoothness) 参数 ---
#define FILTER_ALPHA_ANGLE     0.45f    // 角度滤波系数 (0-1)。值越大越跟手，值越小越丝滑。
#define FILTER_ALPHA_SPEED     1.0f    // 速度滤波系数 (0-1)。值越大提速越猛，值越小加速越柔和。
#define SLEW_RATE_ANGLE        35.0f    // 单次周期最大转角变化 (度)。防止电机/舵机瞬间猛打。

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
 * @brief 【关键修改】严格单向索引追踪
 * 强制要求索引只能在当前位置往后 [0, search_range] 范围内寻找。
 * 彻底解决在原路折返轨迹中，索引跳到回程路径的问题。
 */
static int Find_Closest_Point_Index_Strict(int current_idx, int search_range)
{
    int closest_idx = current_idx;
    float min_dist_sq = 1e9f; 

    int end_idx = current_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    // 只往后搜，不回头，且搜寻范围限制在 search_range（建议 50-80 点）
    for (int i = current_idx; i <= end_idx; i++) {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, 
                                    nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d_sq < min_dist_sq) {
            min_dist_sq = d_sq;
            closest_idx = i;
        }
    }
    
    // 如果最近点离车依然非常远（如>0.8m），说明可能丢位了，
    // 此时保持原索引不跳跃，等待车纠偏回来
    if (min_dist_sq > 800.0f * 800.0f) {
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
            float path_angle = atan2f(dy, dx) * 57.29578f;
            float angle_diff = fabsf(NormalizeAngle(path_angle - inertial_nav.relative_yaw));
            
            float factor = (angle_diff / 60.0f) * (1.2f - 0.2f * step);
            if (factor > max_curve) max_curve = factor;
        }
    }
    return (max_curve > 1.0f) ? 1.0f : max_curve;
}

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

    // 1. 定位当前基准 (保持严格单向搜索，范围扩大到80点以适应极速)
    int base_idx = Find_Closest_Point_Index_Strict(g_target_idx, 80);
    g_target_idx = base_idx;

    if (g_target_idx >= nav_ram_data.point_count - 1) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0; err_degree = 0; return;
    }

    // 2. 动态极限前瞻计算
    // 速度越快，LD越大，选取的点越靠前，转向越平滑
    float lookahead_dist = PP_LD_MIN_CURVE + fabsf(prev_speed_set) * PP_LD_SPEED_GAIN;
    float lookahead_dist_sq = lookahead_dist * lookahead_dist;

    // 3. 极限选点 (寻找远方前瞻点)
    float tx = nav_ram_data.points[base_idx].x;
    float ty = nav_ram_data.points[base_idx].y;
    // 搜索深度配合 LD：如果 LD 是 1000mm，则搜索 1000/20 + 容余 = 70个点
    int ld_scan_limit = base_idx + (int)(lookahead_dist / 15.0f) + 40;
    if (ld_scan_limit > nav_ram_data.point_count) ld_scan_limit = nav_ram_data.point_count;

    for (int i = base_idx; i < ld_scan_limit; i++) {
        float d_sq = CalcDistanceSq(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        tx = nav_ram_data.points[i].x; ty = nav_ram_data.points[i].y;
        // 只有当点离车距离真正达标，或者遇到动作点才停
        if (d_sq >= lookahead_dist_sq || nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
            break;
        }
    }

    // 4. 计算航向
    float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
    float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 5. 暴力速度规划
    float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, CURVE_PREVIEW_DIST);
    
    // 【提速关键】出弯瞬间“弹射”逻辑：如果曲率正在快速变小，直接无视剩余曲率，强制给油
    if (curve_f < prev_curve_f) {
        curve_f *= 0.4f; // 更加激进的出弯策略
    }
    prev_curve_f = curve_f;

    // 非线性曲率映射
    if (curve_f < SPD_CURVE_DEADZONE) curve_f = 0.0f;
    else curve_f = (curve_f - SPD_CURVE_DEADZONE) / (1.0f - SPD_CURVE_DEADZONE);
    curve_f = powf(curve_f, SPD_CURVE_EXPONENT);

    // 剩余距离 (索引差值判定)
    int stop_idx = -1;
    float dist_stop = 99999.0f;
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 300; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            stop_idx = i;
            dist_stop = (stop_idx - base_idx) * 20.0f;
            break;
        }
    }

    float raw_spd = 0;
    if (dist_stop < NAV_DIST_FAR) { 
        // 刹车区保持原样保证精度
        if (dist_stop <= NAV_DIST_ARRIVE) raw_spd = 0;
        else if (dist_stop <= NAV_DIST_NEAR) raw_spd = NAV_SPEED_SLOW * (dist_stop / NAV_DIST_NEAR);
        else raw_spd = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ((dist_stop - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR));
    } else {
        // 【极限区域】
        // 1. 直道不仅跑 FAST，还要允许超频加速 (1.3倍)
        float current_max_spd = (curve_f <= 0.0f) ? (NAV_SPEED_FAST * 1.3f) : NAV_SPEED_FAST;
        
        // 2. 弯道速度分配
        raw_spd = current_max_spd - (current_max_spd - NAV_SPEED_SLOW) * curve_f;
        
        // 3. 角度纠偏减速：极速行驶时，小偏差不减速，大偏差才微调
        float ang_p = fabsf(raw_err_degree) / SPD_ANGLE_TOLERANCE;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - SPD_ANGLE_PENALTY * ang_p);
    }

    // 6. 极速滤波输出
    float diff = raw_err_degree - prev_err_degree;
    if (diff > SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree + SLEW_RATE_ANGLE;
    else if (diff < -SLEW_RATE_ANGLE) raw_err_degree = prev_err_degree - SLEW_RATE_ANGLE;

    err_degree = FILTER_ALPHA_ANGLE * raw_err_degree + (1.0f - FILTER_ALPHA_ANGLE) * prev_err_degree;
    target_speed_set = FILTER_ALPHA_SPEED * raw_spd + (1.0f - FILTER_ALPHA_SPEED) * prev_speed_set;

    prev_err_degree = err_degree;
    prev_speed_set = target_speed_set;

    // 7. 停车判定
    if (stop_idx != -1 && dist_stop <= NAV_DIST_ARRIVE + 20.0f) { 
        if (nav_ram_data.points[stop_idx].point_type != NAV_POINT_PATH) {
            target_speed_set = 0; g_special_action_trigger = 1;
        } else if (stop_idx >= nav_ram_data.point_count - 5) {
            g_replay_state = REPLAY_FINISHED;
        }
    }
}


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
