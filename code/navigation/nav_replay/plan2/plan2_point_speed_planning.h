#ifndef _PLAN2_POINT_SPEED_PLANNING_H_
#define _PLAN2_POINT_SPEED_PLANNING_H_

// 科目二方案4：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按点对点导航，速度由剩余距离在线规划。
// 2. 雷区点不再等进中心后才慢慢收速，而是进入准备区就直接给 0 速度，
//    让底层普通刹车前馈尽快介入；速度压下来后再用超低速补进中心。
// 3. 雷区旋转从触发瞬间的车头角开始规划，在车头/车尾朝向下一个目标点之间选更快的一组，总角度至少 721 度。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的通过半径（mm）；进入后直接推进索引，不停车。
#define NAV_POINT_PATH_ARRIVE_RADIUS            40.0f
// 雷区点/跳跃点等特殊点的中心停车执行圆半径（mm）；直径 70cm，因此半径为 350mm。
#define NAV_POINT_SPECIAL_STOP_RADIUS           350.0f
// 雷区执行圆外的近距离补进半径（mm）；只在执行圆外沿附近且速度很低时爬进执行圆，避免提前很远就慢慢爬。
#define NAV_POINT_SPECIAL_CRAWL_NEAR_RADIUS     500.0f
// 雷区动态刹车准备圆最小半径（mm）；实测需要近 1m 刹停，因此默认从 950mm 起步。
#define NAV_POINT_SPECIAL_BRAKE_PREP_MIN_RADIUS 950.0f
// 雷区动态刹车准备圆最大半径（mm）；限制极端速度下过早停车。
#define NAV_POINT_SPECIAL_BRAKE_PREP_MAX_RADIUS 2000.0f
// 速度平方到准备圆半径的换算系数；当前速度越快，准备圆越大。
#define NAV_POINT_SPECIAL_BRAKE_SPEED2_RADIUS_GAIN 0.0040f
// 判断刹车前馈已经明显建压的 PWM 阈值；低于该值时准备圆会额外放大。
#define NAV_POINT_SPECIAL_BRAKE_READY_PWM       1200.0f
// 刹车前馈尚未建压时的额外准备距离（mm）。
#define NAV_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN  180.0f
// 雷区补进中心时使用的低速速度指令；执行圆已放大到 70cm，补进速度可略高，减少贴近后等待时间。
#define NAV_POINT_SPECIAL_CRAWL_SPEED           (-90.0f)
// 当前速度低于该阈值（mm/s）后，才从“直接刹停”切到“超低速爬行补中心”。
#define NAV_POINT_SPECIAL_CRAWL_ENTRY_SPEED_MM_S 60.0f
// 雷区强停刹释放速度（mm/s）；低于该速度后关闭重重刹，避免中心附近反复反抽。
#define NAV_POINT_SPECIAL_HARD_BRAKE_RELEASE_SPEED_MM_S 80.0f
// 雷区执行圆内允许直接触发旋转的速度阈值（mm/s）；不再要求连续停稳，避免进圈后还要慢走几步才触发。
#define NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S    180.0f
// 最终终点的停车半径（mm）。
#define NAV_POINT_FINAL_STOP_RADIUS             80.0f
// 中等角度偏差阈值（deg）；超过后只允许低速逼近。
#define NAV_POINT_YAW_STOP_TOLERANCE            18.0f
// 大角度偏差阈值（deg）；超过后直接停车原地修方向。
#define NAV_POINT_YAW_SLOW_TOLERANCE            35.0f
// 允许优先倒车/反向朝向的偏置量（deg）。
#define NAV_POINT_REVERSE_SELECT_BIAS_DEG       10.0f
// 停稳判定计数阈值（周期数）；满足后才允许触发旋转。
#define NAV_POINT_STOP_STABLE_TICKS             12U
// 认为“已经停稳”的车体速度阈值（mm/s）。
#define NAV_POINT_STOP_SPEED_MM_S               80.0f
// 起跑前航向对齐容差（deg）。
#define NAV_POINT_START_HEADING_TOLERANCE       0.3f

// 在线速度规划的快速巡航速度指令。
#define NAV_POINT_SPEED_FAST                    (-320.0f)
// 在线速度规划的慢速逼近速度指令。
#define NAV_POINT_SPEED_SLOW                    (-80.0f)
// 停车速度指令。
#define NAV_POINT_SPEED_STOP                    (0.0f)
// v^2 = 2ad 中的“指令域减速度”。
#define NAV_POINT_SPEED_DECEL_CMD2_PER_MM       80.0f
// 速度指令上升斜率。
#define NAV_POINT_SPEED_ACCEL_STEP              18.0f
// 速度指令正常减速斜率。
#define NAV_POINT_SPEED_DECEL_STEP              45.0f
// 跨零或瞬时停车时的最大速度变化量。
#define NAV_POINT_SPEED_CROSS_ZERO_STEP         90.0f

// 雷区旋转最小总角度（deg）；统一按至少 721 度处理。
#define NAV_POINT_SPIN_MIN_TOTAL_ANGLE          721.0f

// 输出到底盘控制层的目标速度指令。
extern volatile float target_speed_set;
// 输出到底盘控制层的目标转向误差（deg）。
extern volatile float err_degree;

// 导航状态机：由上层任务轮询，判断是否在运行、是否已完成。
extern NavReplayState_e g_replay_state;
// 当前正在逼近或刚触发的点类型。
extern uint8 g_current_point_type;
// 特殊动作触发标志；置 1 后导航暂停，由上层动作状态机接管。
extern uint8 g_special_action_trigger;

// 启动导航回放并初始化状态机。
void NavReplay_Start(void);
// 停止导航回放并清空控制输出。
void NavReplay_Stop(void);
// 科目二方案4主循环；建议放在 10ms/20ms 周期任务中调用。
void NavReplay_Process(void);
// 将静态路表加载到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
