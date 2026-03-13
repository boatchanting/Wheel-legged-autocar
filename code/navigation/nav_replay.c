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
static float Calculate_Upcoming_Curve_Factor(int start_idx, int total_points, float current_x, float current_y)
{
    float accumulated_dist = 0.0f;
    float preview_distance = 600.0f; 
    int far_idx = start_idx;

    for (int i = start_idx; i < total_points; i++)
    {
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

    if (far_idx <= start_idx + 1) return 0.0f;

    float dx_near = nav_ram_data.points[start_idx].x - current_x;
    float dy_near = nav_ram_data.points[start_idx].y - current_y;
    float angle_near = -atan2f(dy_near, -dx_near) * 57.29578f;

    float dx_far = nav_ram_data.points[far_idx].x - nav_ram_data.points[start_idx].x;
    float dy_far = nav_ram_data.points[far_idx].y - nav_ram_data.points[start_idx].y;
    float angle_far = -atan2f(dy_far, -dx_far) * 57.29578f;

    float angle_diff = fabsf(NormalizeAngle(angle_far - angle_near));

    float curve_factor = angle_diff / 60.0f;
    if (curve_factor > 1.0f) curve_factor = 1.0f;

    return curve_factor;
}

// ========================= 核心控制主函数 =========================
void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

    static float prev_err_degree = 0.0f;
    static float prev_speed_set = 0.0f;

    if (g_target_idx == 0 && target_speed_set == 0.0f) {
        prev_err_degree = 0.0f;
        prev_speed_set = 0.0f;
    }

    if (g_target_idx >= nav_ram_data.point_count)
    {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = NAV_SPEED_STOP;
        err_degree = 0.0f;
        prev_err_degree = 0.0f;
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
    // 步骤 A：航点推进逻辑 —— 加入【绝不回头】防抽搐机制
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
        // 计算当前目标点相对车头的偏角
        float dx_tgt = nav_ram_data.points[g_target_idx].x - curr_x;
        float dy_tgt = nav_ram_data.points[g_target_idx].y - curr_y;
        float angle_tgt = -atan2f(dy_tgt, -dx_tgt) * 57.29578f;
        float err_tgt = fabsf(NormalizeAngle(angle_tgt - curr_yaw));

        // 【致命 Bug 修复】：如果距离小于 150mm，
        // 或者：该点已经被我们甩在身后 (偏角 > 90度) 且距离小于 500mm (说明是刚错过的近点)
        // 必须果断抛弃它！绝不能掉头回去找点！
        if (dist_to_target <= 150.0f || (err_tgt > 90.0f && dist_to_target < 500.0f)) 
        {
            g_target_idx++;
            return; 
        }
    }

    // =================================================================
    // 步骤 B：曲率预判 与 动态前瞻
    // =================================================================
    float curve_factor = Calculate_Upcoming_Curve_Factor(g_target_idx, nav_ram_data.point_count, curr_x, curr_y);

    // 稍微放大直道前瞻，缩小弯道前瞻，让小车出弯更果断
    float Ld_straight = 600.0f; 
    float Ld_curve    = 200.0f; 
    float Ld = Ld_straight - (Ld_straight - Ld_curve) * curve_factor;

    // =================================================================
    // 步骤 C：搜寻前瞻“胡萝卜”点
    // =================================================================
    float lookahead_x = nav_ram_data.points[g_target_idx].x;
    float lookahead_y = nav_ram_data.points[g_target_idx].y;

    for (int i = g_target_idx; i < nav_ram_data.point_count; i++)
    {
        lookahead_x = nav_ram_data.points[i].x;
        lookahead_y = nav_ram_data.points[i].y;
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH) break; 

        float d = CalcDistance(curr_x, curr_y, lookahead_x, lookahead_y);
        if (d >= Ld) break; 
    }

    float dx = lookahead_x - curr_x;
    float dy = lookahead_y - curr_y;
    float target_yaw = -atan2f(dy, -dx) * 57.29578f;
    float raw_err_degree = NormalizeAngle(target_yaw - curr_yaw);

    // =================================================================
    // 步骤 D：速度控制 (基于前瞻曲率)
    // =================================================================
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
    else 
    {
        raw_target_speed = NAV_SPEED_FAST - (NAV_SPEED_FAST - NAV_SPEED_SLOW) * curve_factor;
    }

    // =================================================================
    // 步骤 E：双重滤波（跳变限幅 + 低通平滑）解决抖动
    // =================================================================
    
    // 1. 角度跳变限幅 (Slew Rate Limit)：即使目标点瞬间从左变右，车轮也不准猛打
    // 防止因找点引起的原始角度瞬间跳变导致电机抽搐
    float max_angle_change_per_cycle = 15.0f; // 每次周期最多允许变化 15 度
    float delta_err = raw_err_degree - prev_err_degree;
    if (delta_err > max_angle_change_per_cycle) {
        raw_err_degree = prev_err_degree + max_angle_change_per_cycle;
    } else if (delta_err < -max_angle_change_per_cycle) {
        raw_err_degree = prev_err_degree - max_angle_change_per_cycle;
    }

    // 2. 一阶低通平滑
    float alpha = 0.3f; 
    
    err_degree = (alpha * raw_err_degree) + ((1.0f - alpha) * prev_err_degree);
    target_speed_set = (alpha * raw_target_speed) + ((1.0f - alpha) * prev_speed_set);

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