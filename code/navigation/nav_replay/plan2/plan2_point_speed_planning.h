#ifndef _PLAN2_POINT_SPEED_PLANNING_H_
#define _PLAN2_POINT_SPEED_PLANNING_H_

// 科目二方案4：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按点对点导航，速度由剩余距离在线规划。
// 2. 雷区点不再等进中心后才慢慢收速，而是进入准备区就直接给 0 速度，
//    让底层普通刹车前馈尽快介入；速度压下来后再用超低速补进中心。
// 3. 旋转动作提前算好总角度和出口朝向，保证转满至少 730 度后可直接出发。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的通过半径（mm）；进入后直接推进索引，不停车。
#define NAV_POINT_PATH_ARRIVE_RADIUS            70.0f
// 雷区点/跳跃点等特殊点的中心停车半径（mm）；进入后执行停稳再触发流程。
#define NAV_POINT_SPECIAL_STOP_RADIUS           110.0f
// 雷区刹车准备距离（mm）；进入该范围后先直接给 0 速度，让普通刹车前馈建压。
#define NAV_POINT_SPECIAL_BRAKE_PREP_DIST       650.0f
// 雷区补进中心时使用的超低速速度指令。
#define NAV_POINT_SPECIAL_CRAWL_SPEED           (-60.0f)
// 当前速度低于该阈值（mm/s）后，才从“直接刹停”切到“超低速爬行补中心”。
#define NAV_POINT_SPECIAL_CRAWL_ENTRY_SPEED_MM_S 90.0f
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

// 雷区旋转最小总角度（deg）；统一按至少 730 度处理。
#define NAV_POINT_SPIN_MIN_TOTAL_ANGLE          730.0f

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
