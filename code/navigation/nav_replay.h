#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"
#include "../config/sys_options.h"

#if GNSS_NAV == 1
//---------------------------------------------
//--------------纯gnss逻辑------------------
//--------------------------------------------
#define GPS_NAV_REPLAY_USE_STATIC_ROUTE_TABLE 1

#define GPS_NAV_MIN_SAT_USED              4U

// === Pure Pursuit 极简丝滑版配置 ===
// 前瞻距离（决定过弯丝滑度，数值越大越喜欢切内道，2500.0f 表示看向前方 2.5 米）
#define GPS_NAV_LOOKAHEAD_DIST            2500.0f 
#define GPS_NAV_DIST_ARRIVE               500.0f   // 仅用于终点停车的判定距离 (0.5米)
#define GPS_NAV_DIST_NEAR                 2200.0f  // 速度控制的远近临界值

#define GPS_NAV_HEADING_OFFSET_DEG        0.0f
#define GPS_NAV_SPEED_FAST                -600.0f
#define GPS_NAV_SPEED_SLOW                -80.0f
#define GPS_NAV_SPEED_STOP                NAV_SPEED_STOP

#endif


// 1: use compile-time route table generated from CSV (no flash dependency)
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1


#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
/** @brief 到点判定距离阈值（mm），用于终点/特殊点到达判定 */
#define NAV_DIST_ARRIVE                 20.0f
/** @brief 起跑航向对齐阈值（deg），越小起跑姿态越严格 */
#define NAV_START_HEADING_TOLERANCE      0.3f
/** @brief 停车速度指令，0 表示停止 */
#define NAV_SPEED_STOP                   0.0f
/** @brief 纯追踪最小前瞻距离（mm），增大可抑制抖动但会变钝 */
#define PP_LD_MIN_CURVE                500.0f
/** @brief 直道前瞻倍率（无量纲），按点间距放大前瞻基准 */
#define PP_LD_MIN_STRAIGHT              1.2f
/** @brief 前瞻速度增益（mm/(cmd·cycle)），速度越大看得越远 */
#define PP_LD_SPEED_GAIN                0.7f
/** @brief 转向误差一阶低通系数（0~1），越大越跟手 */
#define FILTER_ALPHA_ANGLE              0.45f
/** @brief 速度指令一阶低通系数（0~1），越大越接近离线规划原值 */
#define FILTER_ALPHA_SPEED              0.9f
/** @brief 单周期最大角度变化限幅（deg），抑制突变打角 */
#define SLEW_RATE_ANGLE                35.0f
/** @brief 近停点锁死触发距离（mm），小于该值才允许锁航向 */
#define NAV_STOP_LOCK_DIST_MM          50.0f
/** @brief 近停点判定的零速阈值（速度指令单位），小于该值视作“停止点” */
#define NAV_STOP_LOCK_SPEED_EPS         1.0f
#endif

#if CURRENT_NAV_PLAN == 3
/** @brief plan3 远距离速度段阈值（mm） */
#define NAV_DIST_FAR                  500.0f
/** @brief plan3 近距离速度段阈值（mm） */
#define NAV_DIST_NEAR                 150.0f
/** @brief plan3 到点判定距离阈值（mm） */
#define NAV_DIST_ARRIVE                20.0f
/** @brief plan3 角度对齐阈值（deg） */
#define NAV_YAW_TOLERANCE               1.0f
/** @brief plan3 起跑航向对齐阈值（deg） */
#define NAV_START_HEADING_TOLERANCE     0.3f
/** @brief plan3 远距离速度指令 */
#define NAV_SPEED_FAST               (-140.0f)
/** @brief plan3 近距离速度指令 */
#define NAV_SPEED_SLOW                (-40.0f)
/** @brief plan3 停车速度指令 */
#define NAV_SPEED_STOP                  0.0f
/** @brief plan3 弯道最小前瞻距离（mm） */
#define PP_LD_MIN_CURVE              500.0f
/** @brief plan3 直道前瞻倍率 */
#define PP_LD_MIN_STRAIGHT             1.2f
/** @brief plan3 前瞻速度增益 */
#define PP_LD_SPEED_GAIN               0.7f
/** @brief plan3 曲率预瞄距离（mm） */
#define CURVE_PREVIEW_DIST          1200.0f
/** @brief plan3 曲率减速死区 */
#define SPD_CURVE_DEADZONE             0.35f
/** @brief plan3 曲率减速指数 */
#define SPD_CURVE_EXPONENT             3.5f
/** @brief plan3 角度惩罚权重 */
#define SPD_ANGLE_PENALTY              0.08f
/** @brief plan3 角度惩罚归一阈值（deg） */
#define SPD_ANGLE_TOLERANCE           60.0f
/** @brief plan3 角度低通系数 */
#define FILTER_ALPHA_ANGLE             0.45f
/** @brief plan3 速度低通系数 */
#define FILTER_ALPHA_SPEED             1.0f
/** @brief plan3 单周期角度变化限幅（deg） */
#define SLEW_RATE_ANGLE               35.0f
#endif

/** @brief 底盘目标速度输出（速度指令单位） */
extern volatile float target_speed_set;
/** @brief 转向误差输出（deg） */
extern volatile float err_degree;
/** @brief 横滚角（由其他模块维护） */
extern volatile float roll_degree;

/** @brief 导航回放状态机 */
typedef enum
{
    /** 空闲/停止状态 */
    REPLAY_IDLE,
    /** 正在回放轨迹 */
    REPLAY_RUNNING,
    /** 全部轨迹点处理完成 */
    REPLAY_FINISHED
} NavReplayState_e;

// 暴露给外部的状态
extern NavReplayState_e g_replay_state;         // 当前复现状态
extern uint8 g_current_point_type;              // 当前正在前往/到达的点的类型
extern uint8 g_special_action_trigger;          // 特殊动作触发标志 (1: 到达特殊点，请执行动作)
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

// ========================= 函数接口 =========================

/**
 * @brief 开始导航回放
 * @note 常由遥控/任务入口调用，内部会装载静态路表并复位状态机
 */
void NavReplay_Start(void);
/**
 * @brief 停止导航回放
 * @note 常由上层停止命令调用，内部清零速度与误差输出
 */
void NavReplay_Stop(void);
/**
 * @brief 导航回放主循环
 * @note 建议在 10ms/20ms 周期任务中调用，是速度与转向控制的核心入口
 */
void NavReplay_Process(void);
/**
 * @brief 将编译期静态路表装载到 RAM
 * @return 实际装载的点数
 * @note 由 NavReplay_Start() 调用，也可用于调试阶段手动触发
 */
uint16 NavReplay_LoadStaticRouteToRam(void);

#if GNSS_NAV == 1
    uint16 GpsNavReplay_LoadStaticRouteToRam(void);
    void GpsNavReplay_Start(void);
    void GpsNavReplay_Stop(void);
    void GpsNavReplay_Process(void);
#endif

#endif
