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

// ========================= 曲率预判辅助函数 =========================
// 往前扫描路径，计算未来一段距离内的弯曲程度 (返回 0.0~1.0 的系数)
// 0.0 代表前方是大直道，1.0 代表前方有急弯/发卡弯
static float Calculate_Upcoming_Curve_Factor(int start_idx, int total_points, float current_x, float current_y)
{
    float accumulated_dist = 0.0f;
    float preview_distance = 600.0f; // 往前看 60 厘米 (约涵盖 3 个弯道点或 1 个直道点)
    int far_idx = start_idx;

    // 1. 寻找前方 60cm 处的那个点
    for (int i = start_idx; i < total_points; i++)
    {
        // 如果遇到特殊点，必须把它当作“急需处理的情况”
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) {
            far_idx = i;
            break; 
        }
        float d = CalcDistance(current_x, current_y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
        if (d >= preview_distance) {
            far_idx = i;
            break;
        }
    }

    // 如果终点就在眼前，直接返回 0 让底层停车逻辑接管
    if (far_idx <= start_idx + 1) return 0.0f;

    // 2. 计算航向角的差值 (当前走向 vs 远方走向)
    // 向量1：当前位置 -> 目标点 (近处走向)
    float dx_near = nav_ram_data.points[start_idx].x - current_x;
    float dy_near = nav_ram_data.points[start_idx].y - current_y;
    float angle_near = -atan2f(dy_near, -dx_near) * 57.29578f;

    // 向量2：目标点 -> 远方预览点 (远处走向)
    float dx_far = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
    float dy_far = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
    float angle_far = -atan2f(dy_far, -dx_far) * 57.29578f;

    // 3. 计算前方路径的偏角
    float angle_diff = fabsf(NormalizeAngle(angle_far - angle_near));

    // 4. 将偏角映射为 0.0 ~ 1.0 的曲率系数 (假设 >60度 就算满格急弯)
    float curve_factor = angle_diff / 60.0f;
    if (curve_factor > 1.0f) curve_factor = 1.0f;

    return curve_factor;
}

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

    // === 核心杀手锏：静态变量记录上一次的状态，用于低通滤波平滑输出 ===
    static float prev_err_degree = 0.0f;
    static float prev_speed_set = 0.0f;

    // 如果刚起步，初始化滤波器
    if (g_target_idx == 0 && target_speed_set == 0.0f) {
        prev_err_degree = 0.0f;
        prev_speed_set = 0.0f;
    }

    // 1. 检查是否跑完全部点位
    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        prev_err_degree = 0.0f;
        #if DEBUG_LOG_ENABLE
        printf("[Nav] Replay Finished.\r\n");
        #endif
        return;
    }

    float curr_x = inertial_nav.x;
    float curr_y = inertial_nav.y;
    float curr_yaw = inertial_nav.relative_yaw;
    g_current_point_type = nav_ram_data.points[g_target_idx].point_type;

    float dist_to_target = CalcDistance(curr_x, curr_y, 
                                        nav_ram_data.points[g_target_idx].x, 
                                        nav_ram_data.points[g_target_idx].y);

    // =================================================================
    // 步骤 A：航点更新逻辑 (调整切点时机，防止死磕某一个点)
    // =================================================================
    if (g_current_point_type != NAV_POINT_PATH || g_target_idx == nav_ram_data.point_count - 1)
    {
        if (dist_to_target <= NAV_DIST_ARRIVE)
        {
            target_speed_set = NAV_SPEED_STOP;
            if (g_current_point_type != NAV_POINT_PATH)
            {
                if (g_current_point_type == NAV_POINT_CIRCLE) minefield_flag = 1;
                g_special_action_trigger = 1;
            }
            g_target_idx++;
            return; 
        }
    }
    else
    {
        if (g_target_idx < nav_ram_data.point_count - 1)
        {
            float dist_to_next = CalcDistance(curr_x, curr_y, 
                                              nav_ram_data.points[g_target_idx+1].x, 
                                              nav_ram_data.points[g_target_idx+1].y);
            
            // 【调参1】：增大航点切换半径 (从150改到了250)。
            // 弯道点相距200mm，当离当前点250mm时(说明快到了)或者离下个点更近时，果断抛弃当前点看下一个。
            if (dist_to_target <= 250.0f || dist_to_next < dist_to_target)
            {
                g_target_idx++;
                return; 
            }
        }
    }

    // =================================================================
    // 步骤 B：动态前瞻 Pure Pursuit 核心算法
    // =================================================================
    float current_abs_speed = fabsf(prev_speed_set); // 用平滑后的上一周期速度来算前瞻
    
    // 【调参2】：拉长前瞻距离 Ld (这是解决画龙的核心)
    // 基础前瞻从 150 提升到 250(弯道)，最大前瞻提升到 550(直道)。
    // 这样在弯道时，车头永远看向前方 1~2 个点，而不是脚底下的点。
    float Ld = 250.0f + (current_abs_speed / fabsf(NAV_SPEED_FAST)) * 300.0f;

    int lookahead_idx = g_target_idx;
    float lookahead_x = nav_ram_data.points[lookahead_idx].x;
    float lookahead_y = nav_ram_data.points[lookahead_idx].y;

    for (int i = g_target_idx; i < nav_ram_data.point_count; i++)
    {
        lookahead_x = nav_ram_data.points[i].x;
        lookahead_y = nav_ram_data.points[i].y;
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break; 

        float d = CalcDistance(curr_x, curr_y, lookahead_x, lookahead_y);
        if (d >= Ld) break; 
    }

    // 计算原始期望角度误差
    float dx = lookahead_x - curr_x;
    float dy = lookahead_y - curr_y;
    float target_yaw = -atan2f(dy, -dx) * 57.29578f;
    float raw_err_degree = NormalizeAngle(target_yaw - curr_yaw);

    // =================================================================
    // 步骤 C：根据前瞻曲率动态控速
    // =================================================================
    float abs_err = fabsf(raw_err_degree);
    float raw_target_speed = NAV_SPEED_FAST; 

    if (g_current_point_type != NAV_POINT_PATH || g_target_idx == nav_ram_data.point_count - 1)
    {
        if (dist_to_target > NAV_DIST_FAR) {
            raw_target_speed = NAV_SPEED_FAST;
        } else if (dist_to_target > NAV_DIST_ARRIVE) {
            float ratio = (dist_to_target - NAV_DIST_ARRIVE) / (NAV_DIST_FAR - NAV_DIST_ARRIVE);
            raw_target_speed = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ratio;
        } else {
            raw_target_speed = NAV_SPEED_STOP;
        }
    }

    // 弯道柔和减速 (放宽角度限制，防止刹车感)
    if (abs_err > 45.0f) 
    {
        raw_target_speed = NAV_SPEED_SLOW;
    } 
    else if (abs_err > NAV_YAW_TOLERANCE) 
    {
        float turn_ratio = abs_err / 45.0f;
        raw_target_speed = NAV_SPEED_FAST - (NAV_SPEED_FAST - NAV_SPEED_SLOW) * turn_ratio;
    } 

    // =================================================================
    // 步骤 D：一阶低通滤波 (消除机械震荡的最终防线)
    // =================================================================
    // 【调参3】：滤波系数 Alpha (0.0 ~ 1.0)
    // Alpha = 0.4 意味着：本次输出 = 40%的新计算值 + 60%的上一周期老值。
    // 这就像给电机的指令加了“弹簧”，点位切变引起的瞬间角度跳变会被圆滑过渡掉。
    float alpha = 0.4f; 
    
    err_degree = (alpha * raw_err_degree) + ((1.0f - alpha) * prev_err_degree);
    target_speed_set = (alpha * raw_target_speed) + ((1.0f - alpha) * prev_speed_set);

    // 更新历史值
    prev_err_degree = err_degree;
    prev_speed_set = target_speed_set;
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