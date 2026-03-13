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

// ========================= 新增/调整的控制参数 =========================
#define PP_K_V               1.2f    // 前瞻系数：速度越大看越远 (单位: s)
#define PP_L_MIN             150.0f  // 最小前瞻距离 (mm)，保证弯道精度
#define PP_L_MAX             800.0f  // 最大前瞻距离 (mm)，保证直道平滑
#define CURVE_REDUCE_GAIN    15.0f   // 弯道减速增益
#define ANGLE_STOP_LIMIT     60.0f   // 航向偏差超过此值才原地旋转
#define ARRIVE_LIMIT_NORMAL  40.0f   // 普通点判定范围
#define ARRIVE_LIMIT_SPECIAL 25.0f   // 特殊点/弯道判定范围 (25mm)

// ========================= 辅助计算函数 =========================

// 获取两点间距离
static float GetDist(float x1, float y1, float x2, float y2) {
    return sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/**
 * @brief 计算前方路径的平均曲率（预计算）
 * @param start_idx 当前目标点索引
 * @return 预测的弯道激烈程度 (0-1, 0为直道)
 */
static float PredictCurvature(uint16 start_idx) {
    if (start_idx + 2 >= nav_ram_data.point_count) return 0.0f;
    
    // 取未来三个点计算夹角
    float x1 = nav_ram_data.points[start_idx].x;
    float y1 = nav_ram_data.points[start_idx].y;
    float x2 = nav_ram_data.points[start_idx + 1].x;
    float y2 = nav_ram_data.points[start_idx + 1].y;
    float x3 = nav_ram_data.points[start_idx + 2].x;
    float y3 = nav_ram_data.points[start_idx + 2].y;

    float angle1 = atan2f(y2 - y1, x2 - x1);
    float angle2 = atan2f(y3 - y2, x3 - x2);
    float diff = fabsf(NormalizeAngle((angle2 - angle1) * 57.29578f));
    
    return fminf(diff / 45.0f, 1.0f); // 45度以上视为急弯
}

// ========================= 核心逻辑修改 =========================

void NavReplay_Process(void)
{
    if (g_replay_state != REPLAY_RUNNING || g_special_action_trigger == 1) return;

    // 1. 终点检查
    if (g_target_idx >= nav_ram_data.point_count) {
        g_replay_state = REPLAY_FINISHED;
        target_speed_set = 0;
        err_degree = 0;
        return;
    }

    // 2. 基准定位
    int base_idx = Find_Closest_Point_Index(g_target_idx, 20);
    g_target_idx = base_idx;

    // 3. 动态前瞻距离计算 (Ld)
    // 关键：针对3m点距，前瞻必须能跨越当前线段
    float seg_dist = 0;
    if (base_idx < nav_ram_data.point_count - 1) {
        seg_dist = CalcDistance(nav_ram_data.points[base_idx].x, nav_ram_data.points[base_idx].y, 
                                nav_ram_data.points[base_idx+1].x, nav_ram_data.points[base_idx+1].y);
    }
    float Ld_min = (seg_dist < 400.0f) ? 400.0f : seg_dist * 1.1f; // 弯道保底0.4m，直道保底1.1倍点距
    float Ld_speed_gain = 0.4f; // 随速度增加的前瞻增益
    float lookahead_dist = Ld_min + fabsf(prev_speed_set) * Ld_speed_gain;

    // 4. 寻找插值前瞻点 (解决打点不均匀带来的跳变)
    float tx = nav_ram_data.points[base_idx].x;
    float ty = nav_ram_data.points[base_idx].y;
    for (int i = base_idx; i < nav_ram_data.point_count - 1; i++) {
        float d_node = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i+1].x, nav_ram_data.points[i+1].y);
        if (d_node >= lookahead_dist) {
            float d_curr = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            float step = CalcDistance(nav_ram_data.points[i].x, nav_ram_data.points[i].y, nav_ram_data.points[i+1].x, nav_ram_data.points[i+1].y);
            float ratio = (lookahead_dist - d_curr) / step;
            if (ratio < 0) ratio = 0; if (ratio > 1) ratio = 1;
            tx = nav_ram_data.points[i].x + ratio * (nav_ram_data.points[i+1].x - nav_ram_data.points[i].x);
            ty = nav_ram_data.points[i].y + ratio * (nav_ram_data.points[i+1].y - nav_ram_data.points[i].y);
            break;
        }
        tx = nav_ram_data.points[i+1].x; ty = nav_ram_data.points[i+1].y;
        if (nav_ram_data.points[i+1].point_type != NAV_POINT_PATH) break;
    }

    // 5. 计算方向偏差
    float target_yaw = -atan2f(ty - inertial_nav.y, -(tx - inertial_nav.x)) * 57.29578f;
    float raw_err_degree = NormalizeAngle(target_yaw - inertial_nav.relative_yaw);

    // 6. 速度规划
    float curve_f = Calculate_Upcoming_Curve_Factor(base_idx, 600.0f);
    float dist_stop = 9999.0f;
    int stop_idx = -1;
    for (int i = base_idx; i < nav_ram_data.point_count && i < base_idx + 10; i++) {
        if (nav_ram_data.points[i].point_type != NAV_POINT_PATH || i == nav_ram_data.point_count - 1) {
            dist_stop = CalcDistance(inertial_nav.x, inertial_nav.y, nav_ram_data.points[i].x, nav_ram_data.points[i].y);
            stop_idx = i; break;
        }
    }

    float raw_spd = 0;
    if (dist_stop < NAV_DIST_FAR) { // 终端减速模式
        if (dist_stop <= NAV_DIST_ARRIVE) raw_spd = 0;
        else if (dist_stop <= NAV_DIST_NEAR) raw_spd = NAV_SPEED_SLOW * (dist_stop / NAV_DIST_NEAR);
        else raw_spd = NAV_SPEED_SLOW + (NAV_SPEED_FAST - NAV_SPEED_SLOW) * ((dist_stop - NAV_DIST_NEAR) / (NAV_DIST_FAR - NAV_DIST_NEAR));
    } else { // 巡航模式
        raw_spd = NAV_SPEED_FAST - (NAV_SPEED_FAST - NAV_SPEED_SLOW) * curve_f;
        // 角度过大惩罚减速，防止震荡
        float ang_p = fabsf(raw_err_degree) / 40.0f;
        if (ang_p > 1.0f) ang_p = 1.0f;
        raw_spd *= (1.0f - 0.4f * ang_p); 
    }

    // 7. 丝滑滤波输出
    // 限幅滤波
    float max_step = 15.0f;
    float diff = raw_err_degree - prev_err_degree;
    if (diff > max_step) raw_err_degree = prev_err_degree + max_step;
    else if (diff < -max_step) raw_err_degree = prev_err_degree - max_step;
    // 低通滤波
    float a_ang = 0.45f, a_spd = 0.35f;
    err_degree = a_ang * raw_err_degree + (1.0f - a_ang) * prev_err_degree;
    target_speed_set = a_spd * raw_spd + (1.0f - a_spd) * prev_speed_set;

    prev_err_degree = err_degree;
    prev_speed_set = target_speed_set;

    // 8. 到达判定
    if (stop_idx != -1 && dist_stop < NAV_DIST_ARRIVE) {
        if (nav_ram_data.points[stop_idx].point_type != NAV_POINT_PATH) {
            target_speed_set = 0; g_special_action_trigger = 1;
        } else if (stop_idx == nav_ram_data.point_count - 1) {
            g_replay_state = REPLAY_FINISHED;
        }
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