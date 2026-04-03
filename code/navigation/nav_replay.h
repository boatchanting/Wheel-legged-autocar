#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"

// 1: use compile-time route table generated from CSV (no flash dependency)
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

// ========================= 控制参数宏定义 =========================
// 距离阈值 (单位: mm)
#define NAV_DIST_FAR            300.0f  // 远距离界限，全速
#define NAV_DIST_NEAR           100.0f  // 近距离界限，开始最低速
#define NAV_DIST_ARRIVE         20.0f   // 到达判定阈值
#define NAV_YAW_TOLERANCE        1.0f    //转向阈值，先转再走

// 速度设定 (负数为前进，数值对应 motor rpm 或 pwm 级)
#define NAV_SPEED_FAST          (-120.0f) // 高速行驶速度
#define NAV_SPEED_SLOW          (-60.0f)  // 低速逼近速度 (-60 约等于 20cm/s)
#define NAV_SPEED_STOP          (0.0f)

// ========================= 全局控制变量声明 =========================
// 这些变量由外部定义 (通常在 control.c 或 main.c)，此处引用
extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

// ========================= 模块状态变量 =========================
typedef enum
{
    REPLAY_IDLE,        // 停止/空闲
    REPLAY_RUNNING,     // 正在跑图
    REPLAY_FINISHED     // 完成所有点
} NavReplayState_e;

// 暴露给外部的状态
extern NavReplayState_e g_replay_state;         // 当前复现状态
extern uint8 g_current_point_type;              // 当前正在前往/到达的点的类型
extern uint8 g_special_action_trigger;          // 特殊动作触发标志 (1: 到达特殊点，请执行动作)

// ========================= 函数接口 =========================

/**
 * @brief  开始复现路径
 * @note   Loads route from compile-time table when NAV_REPLAY_USE_STATIC_ROUTE_TABLE = 1
 *         会将状态置为 REPLAY_RUNNING，索引置 0
 */
void NavReplay_Start(void);

/**
 * @brief  停止复现
 * @note   速度置 0，状态置 IDLE
 */
void NavReplay_Stop(void);

/**
 * @brief  惯性导航复现控制周期函数
 * @note   建议放在 10ms 或 20ms 定时器中断或主循环中调用
 *         它会根据当前 inertial_nav 坐标计算 target_speed_set 和 err_degree
 */
void NavReplay_Process(void);

/**
 * @brief  Load static route table (generated from CSV) into nav_ram_data
 * @return loaded point count
 */
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
