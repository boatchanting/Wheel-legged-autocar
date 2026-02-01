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
void NAV_Replay_Task(void) {
    // 1. 检查状态，不在运行中则直接返回
    if (replay_status != REPLAY_RUNNING) {
        return;
    }
    
    // 2. 检查是否已到达最后一个点
    if (target_point_index >= total_points_in_ram) {
        replay_status = REPLAY_FINISHED;
        target_speed_set = 0; // 到达终点，停车
        err_degree = 0;
        printf("Replay: Finished.\r\n");
        return;
    }

    // 3. 获取当前车辆位姿
    float current_x = inertial_nav.x;
    float current_y = inertial_nav.y;
    float current_yaw = inertial_nav.relative_yaw;

    // 4. 获取目标点和上一个点的坐标
    NavPoint_t target_point, prev_point;
    NAV_RAM_GetRecord(target_point_index, &target_point);
    NAV_RAM_GetRecord(target_point_index - 1, &prev_point);

    // 5. 计算到目标点的距离
    float dx = target_point.x - current_x;
    float dy = target_point.y - current_y;
    float distance_to_target = sqrtf(dx * dx + dy * dy);

    // 6. 判断是否到达目标点，如果到达则切换到下一个点
    if (distance_to_target < REPLAY_TARGET_RADIUS_MM) {
        target_point_index++;
        // 切换后立即返回，下一周期再计算新目标，避免数据错乱
        return; 
    }

    // 7. 计算期望航向角 (从当前点指向目标点)
    // atan2f返回弧度，范围是[-PI, PI]
    float target_angle_rad = atan2f(dy, dx);
    float target_angle_deg = target_angle_rad * 180.0f / M_PI;

    // 8. 计算航向误差
    float error = target_angle_deg - current_yaw;

    // 9. 归一化误差到 [-180, 180] 区间
    while (error > 180.0f)  error -= 360.0f;
    while (error < -180.0f) error += 360.0f;
    
    // 10. 输出到全局变量，驱动底层控制器
    err_degree = error;

    // 11. 根据路径曲率设定目标速度
    float path_yaw_diff = fabsf(target_point.yaw - prev_point.yaw);
    if (path_yaw_diff > 180.0f) path_yaw_diff = 360.0f - path_yaw_diff; // 处理角度环绕
    
    if (path_yaw_diff > REPLAY_CURVE_YAW_DIFF) {
        // 角度变化大，是弯道，减速
        target_speed_set = REPLAY_SPEED_CURVE;
    } else {
        // 角度变化小，是直线，加速
        target_speed_set = REPLAY_SPEED_STRAIGHT;
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