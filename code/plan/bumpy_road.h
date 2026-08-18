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
    BUMPY_ROAD_STATE_FINISH       // 收尾态
} BumpyRoadState_e;

/* 中线修正（一次性位置修正）的执行时刻 */
typedef enum
{
    BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF = 0,  // 起飞时刻（默认）：垂直加速度超阈值瞬间修正
    BUMPY_ROAD_CORRECTION_MOMENT_VISUAL_EXIT,  // 视觉脱出时刻：视觉确认出口瞬间修正
} BumpyRoadCorrectionMoment_e;

typedef enum
{
    BUMPY_ROAD_EXIT_NONE = 0,
    BUMPY_ROAD_EXIT_POST_CORRECTION_COMPLETE,
    BUMPY_ROAD_EXIT_AUTO_DISTANCE
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

/**
 * @brief 外部显式解除“起飞”锁存（当前锁存不自动释放）
 *
 * @note 垂直加速度超过阈值（BUMPY_ROAD_VERT_ACC_TAKEOFF_TH_G，默认 5g）后锁存
 *       “起飞”标志并锁住转向控制量（err_degree 归零，视觉不再接入转向）；
 *       视觉数据管线（左右偏差/出入口检测）照常更新，出口确认仍依赖视觉。
 *       需要恢复视觉转向时由外部调用本函数显式解除锁存。
 */
void BumpyRoad_ClearTakeoffLatch(void);

/**
 * @brief 查询是否处于“起飞”锁存状态（调试/遥测用）
 *
 * @return 1: 起飞锁存中, 0: 未锁存
 */
uint8_t BumpyRoad_IsTakeoff(void);

/**
 * @brief 设置中线修正执行时刻
 *
 * @param moment BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF（起飞时刻，默认）或
 *               BUMPY_ROAD_CORRECTION_MOMENT_VISUAL_EXIT（视觉脱出时刻）
 * @note 未调用本函数时默认为起飞时刻；起飞时刻修正后 correction_applied=1，
 *       视觉脱出路径自然跳过，无需专门处理。
 */
void BumpyRoad_SetCorrectionMoment(BumpyRoadCorrectionMoment_e moment);

/**
 * @brief 查询当前中线修正执行时刻（调试用）
 */
BumpyRoadCorrectionMoment_e BumpyRoad_GetCorrectionMoment(void);

#endif // __BUMPY_ROAD_H__
