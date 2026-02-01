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

#endif // _NAV_REPLAY_H_