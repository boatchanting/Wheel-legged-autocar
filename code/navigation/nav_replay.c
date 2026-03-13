#include "nav_replay.h"
#include "../common.h"

// ========================= 内部变量 =========================
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint16 g_target_idx = 0;                    // 当前正在前往的点索引
uint8 g_current_point_type = NAV_POINT_PATH;// 当前点的类型
uint8 g_special_action_trigger = 0;         // 触发标志

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

void NavReplay_Start(void)
{
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
    
    #if DEBUG_LOG_ENABLE
    printf("[Nav] Replay STOPPED.\r\n");
    #endif
}

/**
 * @brief 在路径上寻找离小车当前位置最近的点 (根本上解决'追不上点')
 * @param start_idx 开始搜索的索引
 * @param search_range 向前搜索的点数范围
 * @return 找到的最近点的索引
 */
static int Find_Closest_Point_Index(int start_idx, int search_range)
{
    int closest_idx = start_idx;
    float min_dist = 99999.0f;

    int end_idx = start_idx + search_range;
    if (end_idx >= nav_ram_data.point_count) {
        end_idx = nav_ram_data.point_count - 1;
    }

    for (int i = start_idx; i <= end_idx; i++) {
        float d = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d < min_dist) {
            min_dist = d;
            closest_idx = i;
        }
    }
    return closest_idx;
}

/**
 * @brief 预判前方路径的弯曲程度 (核心预判函数)
 * @param start_idx 当前基准点索引
 * @param preview_dist 预判距离 (mm)
 * @return 曲率因子 (0.0 for straight, 1.0 for sharp curve)
 */
static float Calculate_Upcoming_Curve_Factor(int start_idx, float preview_dist)
{
    if (start_idx >= nav_ram_data.point_count - 2) return 0.0f;

    // 找到预判距离远端的点
    int far_idx = start_idx;
    for (int i = start_idx; i < nav_ram_data.point_count; i++) {
        float d = CalcDistance(nav_ram_data.points[start_idx].x, nav_ram_data.points[start_idx].y, 
                               nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        far_idx = i;
        if (d >= preview_dist) break;
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break;
    }
    
    if (far_idx <= start_idx) return 0.0f;

    // 计算从当前点到远端点的向量方向
    float dx = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
    float dy = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
    float path_angle = atan2f(dy, dx) * 57.29578f;
    
    // 计算当前车头朝向与路径方向的夹角，并取其锐角
    // 注意: inertial_nav.relative_yaw 是车头朝向, 我们需要的是车身运动方向与路径的夹角
    // 这里用 车头-路径夹角 做近似，效果已经很好
    float angle_diff = fabsf(NormalizeAngle(path_angle - inertial_nav.relative_yaw));

    // 将角度差映射到 0-1 的曲率因子
    // 超过60度的弯基本就是急弯了
    float curve_factor = angle_diff / 60.0f; 
    if (curve_factor > 1.0f) curve_factor = 1.0f;

    return curve_factor;
}
// 新增：用于滤波的历史状态变量 (限制在当前文件内可见)
static float prev_err_degree = 0.0f;
static float prev_speed_set = 0.0f;
void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

    if (g_target_idx >= nav_ram_data.point_count) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        return;
    }

    // ==========================================================
    // 1. 定位：找到路径上离小车最近的点作为“基准点”
    // 这是所有计算的起点，确保小车始终与路径关联，不会“跟丢”
    // ==========================================================
    int base_idx = Find_Closest_Point_Index(g_target_idx, 15);
    g_target_idx = base_idx; // 将全局索引更新到我们实际在的位置

    // ==========================================================
    // 2. 动态参数计算 (核心稳定策略)
    // ==========================================================
    
    // a. 动态前瞻距离 (Ld) - 【关键修复】
    //    核心思想：引入一个足够大的基础前瞻距离，确保在任何速度下，
    //    小车都不会因为看得太近而剧烈转向。
    float Ld_base = 250.0f;              // 【可调参数】基础前瞻距离 (mm)，这是低速稳定性的保证！
    float Ld_dynamic_gain = 300.0f;      // 【可调参数】速度对前瞻距离的增益
    
    float current_speed_k = fabsf(prev_speed_set) / fabsf(NAV_SPEED_FAST); // 当前速度百分比 (0~1)
    float lookahead_dist = Ld_base + Ld_dynamic_gain * current_speed_k;

    // b. 预判前方曲率，用于动态减速
    //    注意：这里的曲率判断仅用于降速，不再影响前瞻距离，避免引入不稳定性
    float curve_factor = Calculate_Upcoming_Curve_Factor(base_idx, 400.0f); // 预判前方400mm

    // c. 计算目标速度 (只受曲率和终点影响)
    float raw_target_speed;
    if (curve_factor > 0.15f) { // 仅在明显弯道才减速
        // 速度 = 低速 + (高速-低速) * (1-曲率)，弯道越急越慢
        raw_target_speed = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * (1.0f - curve_factor);
    } else {
        raw_target_speed = NAV_SPEED_FAST; // 直道全速
    }

    // d. 预判前方特殊点/终点，强制减速
    float dist_to_stop = 9999.0f;
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 10; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            // 计算到路径上该点的实际距离
            float dist_along_path = 0;
            for(int j = base_idx; j < i; ++j){
                dist_along_path += CalcDistance(nav_ram_data.points[j].x, nav_ram_data.points[j].y, 
                                                nav_ram_data.points[j+1].x, nav_ram_data.points[j+1].y);
            }
            dist_to_stop = dist_along_path;
            break;
        }
    }
    if (dist_to_stop <= NAV_DIST_FAR) {
        if (dist_to_stop <= NAV_DIST_NEAR) raw_target_speed = NAV_SPEED_SLOW;
        else raw_target_speed = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ((dist_to_stop - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR));
    }

    // ==========================================================
    // 3. Pure Pursuit 核心计算：寻找前瞻点并计算转向角
    // ==========================================================
    int lookahead_idx = base_idx;
    for (int i = base_idx; i < nav_ram_data.point_count; i++) {
        float d = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        lookahead_idx = i;
        if (d >= lookahead_dist) break;
    }

    float lx = nav_ram_data.points[lookahead_idx].x;
    float ly = nav_ram_data.points[lookahead_idx].y;
    // 使用你最初验证过的、符合你的坐标系的atan2计算方式
    float target_yaw = -atan2f(ly - inertial_nav.y, -(lx - inertial_nav.x)) * 57.29578f; 
    float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // ==========================================================
    // 4. 滤波与输出 (平滑指令，消除电机抖动)
    // ==========================================================
    float max_angle_change = 20.0f; 
    float delta_err = raw_err_degree - prev_err_degree;
    if (delta_err > max_angle_change) raw_err_degree = prev_err_degree + max_angle_change;
    else if (delta_err < -max_angle_change) raw_err_degree = prev_err_degree - max_angle_change;
    
    // 采用稍慢的滤波，给系统更多稳定时间
    float alpha_angle = 0.5f;
    float alpha_speed = 0.3f;
    err_degree = (alpha_angle * raw_err_degree) + ((1.0f - alpha_angle) * prev_err_degree);
    target_speed_set = (alpha_speed * raw_target_speed) + ((1.0f - alpha_speed) * prev_speed_set);

    prev_err_degree = err_degree;
    prev_speed_set = target_speed_set;

    // ==========================================================
    // 5. 特殊点触发逻辑
    // ==========================================================
    float dist_to_base_point = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y);
    g_current_point_type = nav_ram_data.points[base_idx].point_type;

    if (g_current_point_type != NAV_POINT_PATH && dist_to_base_point < NAV_DIST_ARRIVE) {
        target_speed_set = NAV_SPEED_STOP;
        prev_speed_set = NAV_SPEED_STOP;
        g_special_action_trigger = 1;
    }
}

/*这里注释了，保存的是原有的到一个点停一次的控制逻辑，仅仅能实现最基本的到达，但它的控制距离是精准的，逻辑是完备的，后面所有的代码都在其基础上进行优化和尝试
void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;


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