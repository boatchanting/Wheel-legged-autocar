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

/* 中线修正（一次性位置修正）的执行时刻（2026-08-19 恢复供移植，惰性禁用：
   对正变量 lateral_mm 恒 0 → 修正整体不执行） */
typedef enum
{
    BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF = 0,  // 起飞时刻（默认）：垂直加速度超阈值瞬间修正
    BUMPY_ROAD_CORRECTION_MOMENT_VISUAL_EXIT,  // 视觉脱出时刻：视觉确认出口瞬间修正
} BumpyRoadCorrectionMoment_e;

typedef enum
{
    BUMPY_ROAD_EXIT_NONE = 0,
    BUMPY_ROAD_EXIT_POST_CORRECTION_COMPLETE,   /* 中线修正后行驶修正距离结束（惰性，当前不触发） */
    BUMPY_ROAD_EXIT_VISUAL_CONFIRMED,           /* 视觉确认出口后行驶缓冲距离结束（当前生效） */
    BUMPY_ROAD_EXIT_AUTO_DISTANCE               /* 视觉始终未确认出口，满目标距离兜底结束 */
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
 * @brief 设置视觉出口锚点（中线对正，2026-08-19 恢复供移植、惰性禁用：
 *        对正变量 lateral_mm 恒 0 → 修正不执行，锚点仅记录不影响任何内容）
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
 * @brief 外部显式清除起飞/落地检测（复位滞回比较器与冲击计数，检测重新武装）
 *
 * @note 起飞/落地由垂直冲击滞回比较器检测（g_vert_acc_world_g >5g 置位、<2g 复位，
 *       2~5g 保持），第 1 次冲击上升沿=起飞，第 2 次=落地。需要重新检测时调用。
 */
void BumpyRoad_ClearTakeoffLatch(void);

/**
 * @brief 查询是否已检测到起飞（第 1 次高冲击上升沿，调试/遥测用）
 *
 * @return 1: 已检测到起飞, 0: 未检测到
 */
uint8_t BumpyRoad_IsTakeoff(void);

/**
 * @brief 查询是否已检测到落地（第 2 次高冲击上升沿，调试/遥测用）
 *
 * @return 1: 已检测到落地, 0: 未检测到
 */
uint8_t BumpyRoad_IsLanding(void);

/**
 * @brief 设置中线修正执行时刻（2026-08-19 恢复供移植，惰性禁用：当前不影响任何内容）
 *
 * @param moment BUMPY_ROAD_CORRECTION_MOMENT_TAKEOFF（起飞时刻，默认）或
 *               BUMPY_ROAD_CORRECTION_MOMENT_VISUAL_EXIT（视觉脱出时刻）
 */
void BumpyRoad_SetCorrectionMoment(BumpyRoadCorrectionMoment_e moment);

/**
 * @brief 查询当前中线修正执行时刻（调试用）
 */
BumpyRoadCorrectionMoment_e BumpyRoad_GetCorrectionMoment(void);

#endif // __BUMPY_ROAD_H__
