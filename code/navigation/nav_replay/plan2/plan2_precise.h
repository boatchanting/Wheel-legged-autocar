#ifndef _PLAN2_PRECISE_H_
#define _PLAN2_PRECISE_H_

// 科目二方案3：点对点 + 离线路表速度规划
// 核心思路：
// 1. 转向仍按“当前位置 -> 当前目标点”的点对点方式计算，保证雷区中心定位直接。
// 2. 目标速度来自 chazhi.py 写入路表的 target_speed，和纯追踪速度规划版共用离线速度曲线思想。
// 3. 普通路径点只推进索引，不再每点停车；雷区点/终点仍按距离触发停车和特殊动作。

// 1 表示直接装载静态路表运行。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

// 原有普通路径点到点半径（mm）；方案3仍保留这个到点推进阈值。
#define NAV_DIST_FAR                        300.0f
// 旧方案中的近距离量级（mm）；当前主要作为特殊点慢速保护的尺度保留。
#define NAV_DIST_NEAR                       150.0f
// 认为“已经到点”的距离阈值（mm）。
#define NAV_DIST_ARRIVE                     20.0f
// 严格中心触发模式下，进入该半径才允许触发特殊点（mm）。
#define NAV_SPECIAL_TRIGGER_RADIUS          300.0f
// 特殊点慢速保护距离（mm）；进入该范围后即使离线曲线给得更快，也压到慢速。
#define NAV_SPECIAL_APPROACH_DIST           800.0f
// 宽松触发模式的观测窗口（mm）；在此范围内允许用预测/越过最近点方式触发。
#define NAV_SPECIAL_RELAX_APPROACH_WINDOW   420.0f
// 宽松触发模式的预测停车时间（s）；用当前逼近速度估计是否会进入中心触发区。
#define NAV_SPECIAL_STOP_PREDICT_TIME       0.35f
// 宽松触发模式的“越过最近点”回退边界（mm）。
#define NAV_SPECIAL_PASS_AWAY_MARGIN        35.0f
// 点对点行驶时允许的最大朝向误差（deg）；超出后先停车修方向。
#define NAV_YAW_TOLERANCE                   1.0f
// 允许切换到反向朝向的偏置量（deg）。
#define NAV_REVERSE_SELECT_BIAS_DEG         8.0f
// 雷区旋转最小总角度（deg）。
#define NAV_SPIN_MIN_TOTAL_ANGLE            730.0f
// 起跑航向对齐容差（deg）。
#define NAV_START_HEADING_TOLERANCE         0.3f

// 兼容旧逻辑保留的快/慢/停速度定义；当前方案3的主速度来自离线路表 target_speed。
#define NAV_SPEED_FAST                      (-300.0f)
#define NAV_SPEED_SLOW                      (-100.0f)
#define NAV_SPEED_STOP                      (0.0f)

// 当离线路表速度接近 0 时，认为是停车点或无效速度。
#define NAV_OFFLINE_SPEED_EPS               1.0f
// 速度斜率切换死区；小于该值视作速度基本不变。
#define NAV_OFFLINE_SPEED_SLEW_EPS          1.0f
// 低速加速门槛；低于该值时用更柔和的起步斜率。
#define NAV_OFFLINE_SPEED_SLEW_LOW_TH       80.0f
// 高速减速门槛；高于该值时允许更大的收速步长。
#define NAV_OFFLINE_SPEED_SLEW_FAST_TH      220.0f
// 低速起步斜率。
#define NAV_OFFLINE_SPEED_SLEW_UP_LOW       30.0f
// 正常加速斜率。
#define NAV_OFFLINE_SPEED_SLEW_UP_NORMAL    45.0f
// 正常减速斜率。
#define NAV_OFFLINE_SPEED_SLEW_DOWN_NORMAL  65.0f
// 高速减速斜率。
#define NAV_OFFLINE_SPEED_SLEW_DOWN_FAST    95.0f
// 跨零或紧急停车时的最大速度变化量。
#define NAV_OFFLINE_SPEED_SLEW_CROSS_ZERO   120.0f

// 输出到底盘控制层的目标速度。
extern volatile float target_speed_set;
// 输出到底盘控制层的转向误差（deg）。
extern volatile float err_degree;

// 状态机总状态。
extern NavReplayState_e g_replay_state;
// 当前目标点类型。
extern uint8 g_current_point_type;
// 特殊动作触发标志。
extern uint8 g_special_action_trigger;

// 启动科目二方案3。
void NavReplay_Start(void);
// 停止科目二方案3。
void NavReplay_Stop(void);
// 方案3主循环：点对点转向 + 离线路表速度规划。
void NavReplay_Process(void);
// 装载静态路表到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
