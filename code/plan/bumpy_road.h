#ifndef __BUMPY_ROAD_H__
#define __BUMPY_ROAD_H__

#include "zf_common_headfile.h"

extern volatile float err_degree;           /* 方向盘打多少度 */
extern volatile float target_speed_set;     /* 目标速度（负数代表前进） */
/**
 * @brief 颠簸路段状态机状态
 */
// 状态枚举
// 状态枚举
typedef enum
{
    BUMPY_ROAD_STATE_IDLE = 0,    // 空闲态
    BUMPY_ROAD_STATE_RUNNING,     // 运行态
    BUMPY_ROAD_STATE_FINISH,      // 收尾态
    BUMPY_ROAD_STATE_JUMPING      // 视觉确认后，等待颠簸专用跳跃结束
} BumpyRoadState_e;

typedef enum
{
    BUMPY_ROAD_EXIT_NONE = 0,
    BUMPY_ROAD_EXIT_POST_CORRECTION_COMPLETE,
    BUMPY_ROAD_EXIT_AUTO_DISTANCE,
    BUMPY_ROAD_EXIT_JUMP_COMPLETE
} BumpyRoadExitReason_e;

typedef enum
{
    BUMPY_ROAD_EVENT_NONE = 0,
    BUMPY_ROAD_EVENT_STARTED,
    BUMPY_ROAD_EVENT_ENDED
} BumpyRoadEvent_e;

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
 * @brief 设置视觉出口锚点；视觉确认出口时将融合坐标修正到该位置
 */
void BumpyRoad_SetExitAnchor(float x_mm, float y_mm);

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

/**
 * @brief 获取最近一次颠簸路段的脱出原因
 */
BumpyRoadExitReason_e BumpyRoad_GetExitReason(void);

/* 最近一次状态机边界事件与其单调递增序号，供遥测日志去重。 */
BumpyRoadEvent_e BumpyRoad_GetLastEvent(void);
uint32_t BumpyRoad_GetEventSequence(void);

#endif // __BUMPY_ROAD_H__
