#ifndef __BUMPY_ROAD_H__
#define __BUMPY_ROAD_H__

#include "zf_common_headfile.h"

/**
 * @brief 颠簸路段状态机状态
 */
typedef enum
{
    BUMPY_ROAD_STATE_IDLE = 0,    // 空闲态：未触发
    BUMPY_ROAD_STATE_RUNNING,     // 执行态：锁速直行并累计里程
    BUMPY_ROAD_STATE_FINISH       // 收尾态：停止并退出
} BumpyRoadState_e;

extern volatile uint8_t vision_detected_bumpy_point;
/**
 * @brief 颠簸路段状态机初始化
 *
 * @note 建议在系统初始化阶段调用一次
 */
void BumpyRoad_Init(void);

/**
 * @brief 外部触发颠簸路段动作
 *
 * @note 仅当状态机处于空闲态时触发有效，触发后将记录当前惯导坐标作为起点
 */
void BumpyRoad_Trigger(void);

/**
 * @brief 颠簸路段状态机周期更新（1ms节拍）
 *
 * @note 建议放在 `pit0_ch0_isr` 中每 1ms 调用一次
 */
void BumpyRoad_Update_1ms(void);

/**
 * @brief 查询状态机是否正在运行
 *
 * @return 1: 正在执行颠簸路段动作, 0: 空闲
 */
uint8_t BumpyRoad_Is_Active(void);

/**
 * @brief 获取当前状态机状态（调试用）
 */
BumpyRoadState_e BumpyRoad_GetState(void);

/**
 * @brief 获取已累计行驶距离（单位：mm，调试用）
 */
float BumpyRoad_GetDistanceMm(void);

#endif // __BUMPY_ROAD_H__
