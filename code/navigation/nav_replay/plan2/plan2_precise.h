#ifndef _PLAN2_PRECISE_H_
#define _PLAN2_PRECISE_H_

// 科目二方案3：点对点 + 离线路表速度规划
// 核心思路：
// 1. 横向仍按“当前位置 -> 当前目标点”做点对点指向，保证雷区中心能被直接追向。
// 2. 纵向速度来自路表 target_speed；普通路径点不停车，只推进索引。
// 3. 雷区点进入刹车准备区后直接给 0 速度，让底层普通刹车前馈尽快建压；
//    速度降下来后再用超低速补进中心，停稳后触发旋转。
// 4. 雷区旋转从触发瞬间的车头角开始规划，在车头/车尾朝向下一个目标点之间选更快的一组，总角度至少 721 度。

// 1 表示直接装载编译期静态路表运行。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

// 普通路径点认为“已经到点”的距离阈值（mm）；到点后只推进索引，不停车。
#define NAV_DIST_ARRIVE                     20.0f
// 雷区中心停车触发半径（mm）；只有进入该半径并停稳后才允许触发旋转。
#define NAV_SPECIAL_TRIGGER_RADIUS          300.0f
// 雷区刹车准备距离（mm）；进入该范围后直接给 0 速度，让刹车前馈先把速度压下来。
#define NAV_SPECIAL_BRAKE_PREP_DIST         650.0f
// 雷区补进中心时使用的超低速指令；只在已经基本停住、但中心还差一点时短距离爬行。
#define NAV_SPECIAL_CRAWL_SPEED             (-60.0f)
// 从“直接刹停”切到“超低速爬行”前，要求当前车速先低于该阈值（mm/s）。
#define NAV_SPECIAL_CRAWL_ENTRY_SPEED_MM_S  90.0f
// 认为“已经停稳”的车速阈值（mm/s）；低于它才开始累计停稳计数。
#define NAV_SPECIAL_STOP_SPEED_MM_S         80.0f
// 雷区中心停稳判定计数（控制周期数）；满足后才触发旋转。
#define NAV_SPECIAL_STOP_STABLE_TICKS       8U
// 宽松模式共享的逼近观察窗口（mm）；复用 nav_options.h 的全局配置。
#define NAV_SPECIAL_RELAX_APPROACH_WINDOW   NAV_PLAN2_SPECIAL_RELAX_APPROACH_WINDOW_MM
// 宽松模式预测时间窗口（s）；复用 nav_options.h 的全局配置。
#define NAV_SPECIAL_STOP_PREDICT_TIME       NAV_PLAN2_SPECIAL_STOP_PREDICT_TIME_S
// 点对点行驶时允许的最大朝向误差（deg）；超出后先停车修方向。
#define NAV_YAW_TOLERANCE                   1.0f
// 允许切到反向朝向的偏置量（deg）；只有反向误差明显更小时才切换到倒车逼近。
#define NAV_REVERSE_SELECT_BIAS_DEG         8.0f
// 雷区旋转最小总角度（deg）；统一按至少 721 度处理。
#define NAV_SPIN_MIN_TOTAL_ANGLE            721.0f
// 起跑前航向对齐容差（deg）。
#define NAV_START_HEADING_TOLERANCE         0.3f

// 兼容旧逻辑保留的快/慢/停速度定义；方案3的主速度来自离线路表 target_speed。
#define NAV_SPEED_FAST                      (-300.0f)
#define NAV_SPEED_SLOW                      (-100.0f)
#define NAV_SPEED_STOP                      (0.0f)

// 当路表 target_speed 非常接近 0 时，认为它是停车点或旧表残留 0 速度。
#define NAV_OFFLINE_SPEED_EPS               1.0f
// 速度斜率切换死区；小于该值视作目标速度基本不变。
#define NAV_OFFLINE_SPEED_SLEW_EPS          1.0f
// 低速起步门槛；低于该值时使用更柔和的起步斜率。
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

// 导航状态机总状态。
extern NavReplayState_e g_replay_state;
// 当前目标点类型。
extern uint8 g_current_point_type;
// 特殊动作触发标志。
extern uint8 g_special_action_trigger;

// 启动科目二方案3。
void NavReplay_Start(void);
// 停止科目二方案3。
void NavReplay_Stop(void);
// 方案3主循环：点对点转向 + 离线路表速度 + 雷区停车旋转。
void NavReplay_Process(void);
// 装载静态路表到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
