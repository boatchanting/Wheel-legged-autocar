#include "nav_replay.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

//-------------------------------------------------------------------------
// 外部全局变量 (由本模块进行写操作)
//-------------------------------------------------------------------------
extern volatile float target_speed_set;
extern volatile float err_degree;

//-------------------------------------------------------------------------
// 模块内部静态变量
//-------------------------------------------------------------------------
static NavReplayStatus_t replay_status = REPLAY_IDLE; // 当前复现状态
static uint8 is_data_loaded_ok = 0;                   // 标志位：数据是否加载成功
static uint16 total_points_in_ram = 0;               // RAM中总点数
static uint16 target_point_index = 0;                // 当前目标点的索引

/**
 * @brief 初始化导航复现模块
 *        会尝试从Flash加载路径到RAM
 */
void NAV_Replay_Init(void) {
    // 1. 初始化依赖模块和自身状态
    Ram2Flash_Init();
    NAV_Replay_ReloadData(); // 直接调用重载函数完成初始化加载
}

/**
 * @brief 开始或重新开始轨迹复现
 */
void NAV_Replay_Start(void) {
    if (is_data_loaded_ok) {
        // 从第一个点开始，目标是第二个点 (索引为1)
        target_point_index = 1; 
        replay_status = REPLAY_RUNNING;
        printf("Replay: Started.\r\n");
    }
}

/**
 * @brief 停止轨迹复现
 */
void NAV_Replay_Stop(void) {
    replay_status = REPLAY_IDLE;
    target_speed_set = 0; // 立即停车
    err_degree = 0;
    printf("Replay: Stopped.\r\n");
}

/**
 * @brief 导航复现核心任务函数 (需要被周期性调用)
 */
/**
 * @brief 导航复现核心任务函数 (需要被周期性调用)
 */
void NAV_Replay_Task(void) {
    // 1. 检查状态
    if (replay_status != REPLAY_RUNNING) {
        return;
    }
    
    // 2. 获取当前车辆位姿
    float current_x = inertial_nav.x;
    float current_y = inertial_nav.y;
    float current_yaw = inertial_nav.relative_yaw;
    
    // 3. 寻找前瞻点
    float look_ahead_x, look_ahead_y;
    if (!Find_LookAhead_Point(current_x, current_y, &look_ahead_x, &look_ahead_y)) {
        // Find_LookAhead_Point 中已经处理了 target_point_index 的推进
        // 如果返回 0，说明已经到达终点
        replay_status = REPLAY_FINISHED;
        target_speed_set = 0; // 到达终点，停车
        err_degree = 0;
        printf("Replay: Finished.\r\n");
        return;
    }

    // 4. 计算前瞻航向角 (从当前点指向前瞻点)
    float dx = look_ahead_x - current_x;
    float dy = look_ahead_y - current_y;
    
    float target_angle_rad = atan2f(dy, dx);
    float target_angle_deg = target_angle_rad * 180.0f / M_PI;

    // 5. 计算航向误差
    float error = target_angle_deg - current_yaw;

    // 6. 归一化误差到 [-180, 180] 区间
    while (error > 180.0f)  error -= 360.0f;
    while (error < -180.0f) error += 360.0f;
    
    // 7. 输出到全局变量，驱动底层控制器
    err_degree = error;

    // 8. 根据**前瞻点**的路径曲率设定目标速度
    // 速度判断仍然使用 RAM 轨迹点之间的曲率 (target_point_index 及其前一个点)
    // 这样能确保我们提前减速，而不是到了弯道才开始减速
    if (target_point_index >= total_points_in_ram) {
         // 已经到达终点前最后一个 RAM 点，保持当前速度或减速
         target_speed_set = REPLAY_SPEED_CURVE; // 减速确保平稳到达终点
    } else {
        NavPoint_t target_point, prev_point;
        // 使用当前正在追逐的 RAM 轨迹点和它的前一个 RAM 轨迹点来判断曲率
        // 只有 target_point_index > 0 才安全
        uint16 p1_idx = (target_point_index > 0) ? target_point_index : 1;
        uint16 p0_idx = p1_idx - 1;

        NAV_RAM_GetRecord(p1_idx, &target_point);
        NAV_RAM_GetRecord(p0_idx, &prev_point);
        
        float path_yaw_diff = fabsf(target_point.yaw - prev_point.yaw);
        if (path_yaw_diff > 180.0f) path_yaw_diff = 360.0f - path_yaw_diff; 
        
        if (path_yaw_diff > REPLAY_CURVE_YAW_DIFF) {
            // 角度变化大，是弯道，减速
            target_speed_set = REPLAY_SPEED_CURVE;
        } else {
            // 角度变化小，是直线，加速
            target_speed_set = REPLAY_SPEED_STRAIGHT;
        }
    }
}

/**
 * @brief 获取当前复现状态
 */
NavReplayStatus_t NAV_Replay_GetStatus(void) {
    return replay_status;
}

/**
 * @brief 检查数据是否已加载，是否可以开始复现
 * @return 1=已就绪, 0=未就绪
 */
uint8 NAV_Replay_IsReady(void) {
    return is_data_loaded_ok;
}

/**
 * @brief 核心函数：从Flash加载轨迹到RAM，并更新模块状态
 *        这个函数现在是Init和手动重载的公共部分
 */
void NAV_Replay_ReloadData(void) {
    // 1. 重置状态
    is_data_loaded_ok = 0;
    replay_status = REPLAY_IDLE;
    target_point_index = 0;
    
    // 2. 检查Flash中是否有有效数据
    if (Ram2Flash_CheckValid()) {
        printf("Replay: Valid data found in Flash. Loading...\r\n");
        // 3. 如果有，加载到RAM
        if (Ram2Flash_Load()) {
            total_points_in_ram = NAV_RAM_GetRecordCount();
            if (total_points_in_ram > 1) {
                is_data_loaded_ok = 1; // 设置成功标志位
                replay_status = REPLAY_IDLE; // 状态变为空闲，等待启动指令
                printf("Replay: Loaded %d points successfully. Ready.\r\n", total_points_in_ram);
                return; // 成功加载，直接返回
            }
        }
    }
    
    // 如果加载失败或无数据，执行到这里
    replay_status = REPLAY_NO_DATA; // 设置状态为无数据
    printf("Replay Warning: No valid trajectory data loaded from Flash.\r\n");
}

/**
 * @brief 寻找前瞻点 (Look-Ahead Point)
 * @param current_x, current_y: 小车当前位置
 * @param look_ahead_x, look_ahead_y: 返回找到的前瞻点坐标
 * @return 1=找到有效前瞻点, 0=未找到 (已到终点)
 */
static uint8 Find_LookAhead_Point(float current_x, float current_y, float *look_ahead_x, float *look_ahead_y) {
    uint16 i;
    NavPoint_t p_current_ram_target;
    
    // 1. 检查当前 RAM 目标点是否已经可以丢弃
    // target_point_index 指向下一个要追踪的 RAM 轨迹点
    if (target_point_index < total_points_in_ram) {
        NAV_RAM_GetRecord(target_point_index, &p_current_ram_target);
        float dist_sq = (p_current_ram_target.x - current_x) * (p_current_ram_target.x - current_x) + 
                        (p_current_ram_target.y - current_y) * (p_current_ram_target.y - current_y);
        
        // 如果小车离当前 RAM 目标点很近，则推进 RAM 索引
        if (dist_sq < (REPLAY_TARGET_RADIUS_MM * REPLAY_TARGET_RADIUS_MM)) {
            target_point_index++;
        }
    }

    // 2. 检查是否到达终点
    if (target_point_index >= total_points_in_ram) {
        return 0; // 已到达终点
    }
    
    // 3. 寻找与小车距离为 REPLAY_LOOK_AHEAD_MM 的轨迹点作为前瞻点
    // 从当前 RAM 目标点开始向后搜索（这里的搜索是在 RAM 轨迹点之间插值）
    
    // 目标是找到轨迹线段 (Pi, Pi+1) 使得圆心在小车、半径为 L 的圆与该线段相交
    
    // 简化处理：直接找离小车距离最接近 L 的**下一个** RAM 轨迹点
    // 并在其与前一个RAM轨迹点之间插值 (这里只进行 RAM 点间的搜索，不进行线段插值)

    uint16 start_search_idx = target_point_index;
    float L = REPLAY_LOOK_AHEAD_MM;
    float L_sq = L * L;
    
    // 遍历 RAM 轨迹点，找到第一个离小车距离 >= L 的点作为前瞻目标
    for (i = start_search_idx; i < total_points_in_ram; i++) {
        NavPoint_t p_ram;
        NAV_RAM_GetRecord(i, &p_ram);
        
        float dx = p_ram.x - current_x;
        float dy = p_ram.y - current_y;
        float dist_sq = dx * dx + dy * dy;

        if (dist_sq >= L_sq) {
            // 找到了一个足够远的点，我们追逐它，并直接使用它的坐标
            // (更精确的纯追踪需要在线段 Pi-1 和 Pi 之间进行圆与线段的交点插值，这里简化)
            *look_ahead_x = p_ram.x;
            *look_ahead_y = p_ram.y;
            return 1;
        }
    }

    // 如果所有点都比 L_sq 近 (已接近终点)
    // 则将终点作为前瞻点，此时 L 会自动缩小到实际距离，平稳减速到终点。
    if (total_points_in_ram > 0) {
        NavPoint_t p_final;
        NAV_RAM_GetRecord(total_points_in_ram - 1, &p_final);
        *look_ahead_x = p_final.x;
        *look_ahead_y = p_final.y;
        return 1;
    }
    
    return 0; // 理论上不会执行到这里
}