#include "nav_replay.h"

// 全局变量定义
ReplayManager_t replay_manager;

// 静态RAM缓冲区 (用于存储解压后的路径点)
static ReplayNode_t replay_nodes[REPLAY_MAX_NODES];

// 引用外部控制变量
// 注意：在主main.c中定义这些变量，这里声明引用
// volatile float target_speed_set;
// volatile float err_degree;

/**
 * @brief 初始化复现模块
 */
void NAV_REPLAY_Init(void) {
    replay_manager.status = REPLAY_STATE_IDLE;
    replay_manager.total_nodes = 0;
    replay_manager.current_index = 0;
    replay_manager.data_valid_flag = 0;
    
    // 初始化输出变量
    target_speed_set = 0;
    err_degree = 0;
}

/**
 * @brief 从Flash读取并解压所有轨迹数据到RAM
 * @return 1=成功, 0=失败
 */
uint8_t NAV_REPLAY_LoadFromFlash(void) {
    uint16_t seg_count = 0;
    
    // 1. 检查Flash数据
    if (!R2F_HasValidTrajectory()) {
        // printf("REPLAY: No valid data in Flash!\r\n");
        replay_manager.status = REPLAY_STATE_ERROR;
        return 0;
    }
    
    // 2. 获取段总数
    if (R2F_LoadTrajectoryInfo(&seg_count) != R2F_STATUS_SUCCESS) {
        return 0;
    }
    
    printf("REPLAY: Loading %d segments...\r\n", seg_count);
    replay_manager.status = REPLAY_STATE_LOADING;
    replay_manager.total_nodes = 0;
    
    // 3. 遍历读取每个段并转换格式
    // 临时缓冲区，用于读取Flash原始数据
    static float temp_coords[60]; 
    const uint16_t buffer_size_f = 60; // 缓冲区大小 (float 个数)
    
    uint8_t seg_type; 
    uint16_t segment_points_count;

    
    for (uint16_t i = 0; i < seg_count; i++) {
        // 防止RAM溢出
        if (R2F_GetSegment(i, &seg_type, &segment_points_count, temp_coords, buffer_size_f) != R2F_STATUS_SUCCESS) {
            printf("REPLAY: Read error at seg %d\r\n", i);
            replay_manager.status = REPLAY_STATE_ERROR;
            return 0;
        }
        
        // 根据类型转换为ReplayNode
        if (seg_type == SEGMENT_TYPE_LINE) {
            // 直线段：R2F 存了起点 (0, 1, 2) 和终点 (3, 4, 5)
            if (seg_count != 2) continue;
            
            ReplayNode_t* node = &replay_nodes[replay_manager.total_nodes];
            // 终点坐标在索引 3 (x), 4 (y)
            node->x = temp_coords[3]; 
            node->y = temp_coords[4];
            node->type = NODE_TYPE_LINE;
            replay_manager.total_nodes++;
            
        } else if (seg_type == SEGMENT_TYPE_CURVE) {
            // 曲线段：包含 seg_count 个点
            for (uint16_t k = 0; k < seg_count; k++) {
                if (replay_manager.total_nodes >= REPLAY_MAX_NODES) break;
                
                ReplayNode_t* node = &replay_nodes[replay_manager.total_nodes];
                
                // 坐标在 temp_coords 中的索引: k * 3, k * 3 + 1, k * 3 + 2
                node->x = temp_coords[k * 3];
                node->y = temp_coords[k * 3 + 1];
                // yaw (k*3 + 2) 不需要存入 ReplayNode，因为它只用于转向误差计算
                node->type = NODE_TYPE_CURVE;
                replay_manager.total_nodes++;
            }
        }
    }
    
    printf("REPLAY: Loaded %d nodes into RAM.\r\n", replay_manager.total_nodes);
    replay_manager.data_valid_flag = 1;
    replay_manager.status = REPLAY_STATE_READY;
    return 1;
}

/**
 * @brief 开始复现
 */
void NAV_REPLAY_Start(void) {
    if (replay_manager.data_valid_flag && replay_manager.total_nodes > 0) {
        replay_manager.current_index = 0;
        replay_manager.status = REPLAY_STATE_RUNNING;
        printf("REPLAY: Started.\r\n");
    } else {
        printf("REPLAY: Cannot start, no data.\r\n");
    }
}

/**
 * @brief 停止复现
 */
void NAV_REPLAY_Stop(void) {
    replay_manager.status = REPLAY_STATE_FINISHED;
    target_speed_set = REPLAY_SPEED_STOP;
    err_degree = 0;
    printf("REPLAY: Stopped.\r\n");
}

/**
 * @brief 计算角度差 (标准化到 -180 ~ 180 度)
 */
static float calc_angle_diff(float target, float current) {
    float diff = target - current;
    while (diff > 180.0f)  diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    return diff;
}

/**
 * @brief 复现控制任务 (建议10ms-20ms调用一次)
 */
void NAV_REPLAY_Task(void) {
    // 仅在运行状态下执行
    if (replay_manager.status != REPLAY_STATE_RUNNING) {
        return;
    }
    
    // 获取当前车辆状态
    float cur_x = inertial_nav.x;
    float cur_y = inertial_nav.y;
    float cur_yaw = inertial_nav.relative_yaw; // 假设单位是度
    
    // 获取当前目标节点
    ReplayNode_t* target_node = &replay_nodes[replay_manager.current_index];
    
    // 1. 计算距离误差
    float dx = target_node->x - cur_x;
    float dy = target_node->y - cur_y;
    float dist = sqrtf(dx*dx + dy*dy);
    
    // 2. 检查是否到达目标点 (切换逻辑)
    // 如果是最后一个点，判定范围要更小
    float threshold = (replay_manager.current_index == replay_manager.total_nodes - 1) ? 
                      REPLAY_FINISH_DIST_MM : REPLAY_WAYPOINT_DIST_MM;
    
    if (dist < threshold) {
        // 到达当前点，切换到下一个
        replay_manager.current_index++;
        
        // 检查是否结束
        if (replay_manager.current_index >= replay_manager.total_nodes) {
            NAV_REPLAY_Stop();
            return;
        }
        
        // 更新目标点指针
        target_node = &replay_nodes[replay_manager.current_index];
        // 重新计算新的差值
        dx = target_node->x - cur_x;
        dy = target_node->y - cur_y;
    }
    
    // 3. 计算目标角度 (atan2返回弧度，需转为度)
    // 57.29578 = 180 / PI
    float target_yaw = atan2f(dy, dx) * 57.29578f;
    
    // 4. 计算转向误差 (输出给转向环)
    err_degree = calc_angle_diff(target_yaw, cur_yaw);
    
    // 5. 设定速度 (输出给速度环)
    if (target_node->type == NODE_TYPE_LINE) {
        // 直线段：全速
        // 为了平滑，如果角度误差太大(说明偏离严重)，可以减速
        if (fabsf(err_degree) > 30.0f) {
            target_speed_set = REPLAY_SPEED_CURVE; // 偏离太远，减速修正
        } else {
            target_speed_set = REPLAY_SPEED_STRAIGHT;
        }
    } else {
        // 曲线段：常规速度
        target_speed_set = REPLAY_SPEED_CURVE;
    }
    
    // 可选：打印调试信息 (频率不要太高)
    // printf("IDX:%d Dist:%.0f Err:%.1f Spd:%.0f\r\n", 
    //        replay_manager.current_index, dist, err_degree, target_speed_set);
}