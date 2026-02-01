#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"

//-------------------------------------------------------------------------
// 参数配置 (可根据实际车辆性能调整)
//-------------------------------------------------------------------------

// 速度设置 (负数代表前进)
#define REPLAY_SPEED_STRAIGHT   -100.0f  // 直线行驶速度 (可以快一些)
#define REPLAY_SPEED_CURVE      -45.0f  // 弯道行驶速度 (需要慢一些保证循迹精度)

// 导航参数
#define REPLAY_TARGET_RADIUS_MM 50.0f   // 到达目标点的判定半径(mm)
#define REPLAY_CURVE_YAW_DIFF   1.5f    // 判断为曲线的偏航角变化阈值(度)

#define REPLAY_LOOK_AHEAD_MM    400.0f   // 前瞻距离(mm)，追逐目标点与小车的距离。
//-------------------------------------------------------------------------
// 状态定义
//-------------------------------------------------------------------------

// 复现状态枚举
typedef enum {
    REPLAY_IDLE = 0,      // 空闲，未开始
    REPLAY_RUNNING,       // 复现中
    REPLAY_FINISHED,      // 复现完成
    REPLAY_NO_DATA        // Flash中无有效数据
} NavReplayStatus_t;


//-------------------------------------------------------------------------
// 函数声明
//-------------------------------------------------------------------------

void NAV_Replay_Init(void);
void NAV_Replay_Start(void);
void NAV_Replay_Stop(void);
void NAV_Replay_Task(void); // 核心函数，需要被周期性调用 (如 10-20ms 一次)
NavReplayStatus_t NAV_Replay_GetStatus(void);
uint8 NAV_Replay_IsReady(void); // 检查数据是否已成功加载到RAM
void NAV_Replay_ReloadData(void);//重新从Flash加载数据并更新状态
/**
 * @brief 寻找前瞻点 (Look-Ahead Point)
 * @param current_x, current_y: 小车当前位置
 * @param look_ahead_x, look_ahead_y: 返回找到的前瞻点坐标
 * @return 1=找到有效前瞻点, 0=未找到 (已到终点)
 */
static uint8 Find_LookAhead_Point(float current_x, float current_y, float *look_ahead_x, float *look_ahead_y);

#endif // _NAV_REPLAY_H_