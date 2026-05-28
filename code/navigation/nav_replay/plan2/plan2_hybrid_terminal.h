#ifndef _PLAN2_HYBRID_TERMINAL_H_
#define _PLAN2_HYBRID_TERMINAL_H_

// 科目二方案5：远距离路径跟踪 + 近距离点对点终端制导
// 远距离引导方式由 NAV_PLAN2_HYBRID_GUIDE_MODE 选择：
// 1. PLAN2_HYBRID_GUIDE_PURE_PURSUIT：纯追踪前瞻点
// 2. PLAN2_HYBRID_GUIDE_LOS：LOS 线段视线制导
// 近距离统一切换到雷区中心点对点停车，保证中心精度和旋转安全。

// 1 表示直接使用静态路表；方案5同样走 RAM 数据结构，便于和其他方案统一。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 进入终端点对点接管的距离（mm）；大于该值走远距离引导，小于该值强制切点对点停车。
#define NAV_HYBRID_TERMINAL_DIST                900.0f
// 雷区点/特殊点的停车半径（mm）；进入后开始执行停稳判定。
#define NAV_HYBRID_SPECIAL_STOP_RADIUS          110.0f
// 普通路径屏障点的通过半径（mm）；防止前瞻点跨越终端停车区。
#define NAV_HYBRID_PATH_PASS_RADIUS             80.0f
// 最终终点停车半径（mm）。
#define NAV_HYBRID_FINAL_STOP_RADIUS            80.0f
// 低速稳定计数阈值（周期数）；连续满足后才允许触发特殊动作。
#define NAV_HYBRID_STOP_STABLE_TICKS            12U
// 认为“已经停稳”的车体纵向速度阈值（mm/s）。
#define NAV_HYBRID_STOP_SPEED_MM_S              80.0f
// 起跑前航向对齐容差（deg）。
#define NAV_HYBRID_START_HEADING_TOLERANCE      0.3f

// 远距离巡航的快速速度指令。
#define NAV_HYBRID_SPEED_FAST                   (-330.0f)
// 接近停车屏障或姿态不优时的慢速速度指令。
#define NAV_HYBRID_SPEED_SLOW                   (-90.0f)
// 停车速度指令。
#define NAV_HYBRID_SPEED_STOP                   (0.0f)
// 远距离到终端段的在线刹车参数；值越大，允许更晚刹车。
#define NAV_HYBRID_SPEED_DECEL_CMD2_PER_MM      85.0f
// 速度上升斜率限制。
#define NAV_HYBRID_SPEED_ACCEL_STEP             22.0f
// 速度下降斜率限制。
#define NAV_HYBRID_SPEED_DECEL_STEP             55.0f
// 跨零或瞬时停车时允许的最大速度变化量。
#define NAV_HYBRID_SPEED_CROSS_ZERO_STEP        100.0f

// 反向朝向选择偏置（deg）；仅当反向误差更明显更优时才采用反向行驶。
#define NAV_HYBRID_REVERSE_SELECT_BIAS_DEG      10.0f
// 中等角度偏差阈值（deg）；超过后只允许慢速逼近。
#define NAV_HYBRID_TERMINAL_YAW_STOP_TOL        18.0f
// 大角度偏差阈值（deg）；超过后直接停车调姿。
#define NAV_HYBRID_TERMINAL_YAW_SLOW_TOL        35.0f

// 纯追踪最小前瞻距离（mm）；过小容易抖，过大容易转向发钝。
#define NAV_HYBRID_PP_LD_MIN                    450.0f
// 纯追踪前瞻距离随速度增长的增益。
#define NAV_HYBRID_PP_LD_SPEED_GAIN             0.9f
// LOS 前视距离（mm）；决定 LOS 在当前线段上往前看多远。
#define NAV_HYBRID_LOS_LOOKAHEAD                600.0f
// 正常运行时最近点搜索窗口（点数）。
#define NAV_HYBRID_SCAN_RANGE                   90U
// 特殊动作恢复后放宽的搜索窗口（点数）；便于从接管状态重新吸附到路径。
#define NAV_HYBRID_RECOVER_SCAN_RANGE           300U

// 雷区旋转最小总角度（deg）。
#define NAV_HYBRID_SPIN_MIN_TOTAL_ANGLE         730.0f

// 输出到底盘控制层的目标速度指令。
extern volatile float target_speed_set;
// 输出到底盘控制层的转向误差（deg）。
extern volatile float err_degree;

// 导航状态机：运行/停止/完成。
extern NavReplayState_e g_replay_state;
// 当前路径点类型，供外部动作调度使用。
extern uint8 g_current_point_type;
// 特殊动作触发标志；置位后由外部状态机接管。
extern uint8 g_special_action_trigger;

// 启动科目二方案5导航。
void NavReplay_Start(void);
// 停止科目二方案5导航。
void NavReplay_Stop(void);
// 方案5主循环：远距离按 PP/LOS 跟踪，近距离切点对点终端制导。
void NavReplay_Process(void);
// 加载静态路表到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
