#ifndef _PLAN1_STANLEY_TRACKING_H_
#define _PLAN1_STANLEY_TRACKING_H_

// 1: use compile-time route table generated from csv (no flash dependency)
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

/** @brief 到点判定距离阈值（mm），用于终点/特殊点到达判定 */
#define NAV_DIST_ARRIVE                 20.0f
/** @brief 起跑航向对齐阈值（deg），越小起跑姿态越严格 */
#define NAV_START_HEADING_TOLERANCE      0.3f
/** @brief 停车速度指令，0 表示停止 */
#define NAV_SPEED_STOP                   0.0f
/** @brief Stanley 横向误差增益系数。越大越紧贴轨迹；越小越平滑但会轻微切弯 */
#define STANLEY_K 5.0f             //调参可以更改的值STANLEY_K   2.5和K_FF_CURVATURE  100 

/** @brief 转向误差一阶低通系数（0~1），越大越跟手 */
#define FILTER_ALPHA_ANGLE              0.45f
/** @brief 速度指令一阶低通系数（0~1），越大越接近离线规划原值 */
#define FILTER_ALPHA_SPEED              0.9f
/** @brief 单周期最大角度变化限幅（deg），抑制突变打角 */
#define SLEW_RATE_ANGLE                35.0f
/** @brief 弯道旁路滤波曲率阈值（1/mm），|kappa| 超过此值时旁路角度滤波与限幅 */
#define KAPPA_CURVE_BYPASS_THRESH      0.0005f
/** @brief 曲率前馈增益（无量纲），将 kappa*speed 换算为叠加转向角（deg）。调大入弯更主动，过大会振荡 */
#define K_FF_CURVATURE               80.0f   //调参可以更改的值STANLEY_K   和K_FF_CURVATURE  
/** @brief 近停点锁死触发距离（mm），小于该值才允许锁航向 */
// 速度目标限斜率参数：NavReplay_Process() 每 10ms 左右调用一次，数值表示每次允许 target_speed_set 改变的最大量
#define NAV_SPEED_SLEW_EPS             1.0f    // 速度变化死区，小于该值认为没有明显加/减速，避免浮点噪声反复切换斜率档位
#define NAV_SPEED_SLEW_LOW_SPEED_TH    80.0f   // 低速加速分界；低于该速度使用 NAV_SPEED_SLEW_UP_LOW，避免起步瞬间过猛
#define NAV_SPEED_SLEW_FAST_DECEL_TH   220.0f  // 高速减速分界；高于该速度允许更大的降速步长，弯前更快收敛
#define NAV_SPEED_SLEW_UP_LOW          30.0f   // 低速/起步加速步长；加大起步更冲，减小更柔
#define NAV_SPEED_SLEW_UP_NORMAL       45.0f   // 正常加速步长；加大出弯提速更快，过大会带来速度目标突跳
#define NAV_SPEED_SLEW_DOWN_NORMAL     65.0f   // 普通减速步长；加大弯前收速更积极，减小弯前更顺但可能慢半拍
#define NAV_SPEED_SLEW_DOWN_FAST       95.0f   // 高速减速步长；主要处理高速进弯，过大会增加“急刹”体感
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO 120.0f  // 目标速度跨零或停车时的步长；加大停车更干脆，减小停车更平滑
#define NAV_STOP_LOCK_DIST_MM          50.0f
/** @brief 近停点判定的零速阈值（速度指令单位），小于该值视作“停止点” */
#define NAV_STOP_LOCK_SPEED_EPS         1.0f

/** @brief 底盘目标速度输出（速度指令单位） */
extern volatile float target_speed_set;
/** @brief 转向误差输出（deg） */
extern volatile float err_degree;
/** @brief 横滚角（由其他模块维护） */
extern volatile float roll_degree;

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
#endif //_PLAN1_PURE_PURSUIT_SPEED PLANNING_H_
