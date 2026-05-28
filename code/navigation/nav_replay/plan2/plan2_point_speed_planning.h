#ifndef _PLAN2_POINT_SPEED_PLANNING_H_
#define _PLAN2_POINT_SPEED_PLANNING_H_

// 科目二方案4：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点只负责把索引往前推，速度由距离和刹车距离统一规划。
// 2. 雷区点必须在中心小半径内停车稳定后才触发旋转，避免带速度开转。
// 3. 速度规划采用 v^2 = 2ad 的离散形式，单位仍为底盘速度指令，不直接假设真实 m/s。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的通过半径（mm）；进入该范围后直接推进索引，不做停车。
#define NAV_POINT_PATH_ARRIVE_RADIUS            70.0f
// 雷区点/跳跃点等特殊点的停车半径（mm）；进入后开始执行“停稳再触发”流程。
#define NAV_POINT_SPECIAL_STOP_RADIUS           110.0f
// 最终终点的停车半径（mm）；终点不需要继续出框，允许比普通点更保守。
#define NAV_POINT_FINAL_STOP_RADIUS             80.0f
// 中等角度偏差阈值（deg）；超过它后只允许低速逼近，避免带着横摆误差进白框。
#define NAV_POINT_YAW_STOP_TOLERANCE            18.0f
// 大角度偏差阈值（deg）；超过它后直接停车原地修正方向。
#define NAV_POINT_YAW_SLOW_TOLERANCE            35.0f
// 允许优先倒车/反向朝向的偏置量（deg）；反向误差明显更小才切换方向。
#define NAV_POINT_REVERSE_SELECT_BIAS_DEG       10.0f
// 低速稳定计数阈值（周期数）；只有连续满足低速条件才允许触发旋转。
#define NAV_POINT_STOP_STABLE_TICKS             12U
// 认为“已经停稳”的车体纵向速度阈值（mm/s）。
#define NAV_POINT_STOP_SPEED_MM_S               80.0f
// 起跑前航向对齐容差（deg）；IMU963RA 模式下先对齐再发车。
#define NAV_POINT_START_HEADING_TOLERANCE       0.3f

// 在线速度规划的快速巡航速度指令；值越大，远距离逼近越积极。
#define NAV_POINT_SPEED_FAST                    (-320.0f)
// 在线速度规划的慢速逼近速度指令；用于角度偏差较大或接近停车区时的保守速度。
#define NAV_POINT_SPEED_SLOW                    (-80.0f)
// 停车速度指令。
#define NAV_POINT_SPEED_STOP                    (0.0f)
// v^2 = 2ad 中的“指令域减速度”；值越大，系统允许更晚刹车。
#define NAV_POINT_SPEED_DECEL_CMD2_PER_MM       80.0f
// 速度指令上升斜率；限制每周期提速幅度，避免起步过猛。
#define NAV_POINT_SPEED_ACCEL_STEP              18.0f
// 速度指令正常减速斜率；限制每周期收速幅度，避免突兀急刹。
#define NAV_POINT_SPEED_DECEL_STEP              45.0f
// 跨零或瞬时停车时的最大速度变化量；用于让停车动作更干脆。
#define NAV_POINT_SPEED_CROSS_ZERO_STEP         90.0f

// 雷区旋转最小总角度（deg）；统一按至少 730 度处理，满足规则要求。
#define NAV_POINT_SPIN_MIN_TOTAL_ANGLE          730.0f

// 输出到底盘控制层的目标速度指令。
extern volatile float target_speed_set;
// 输出到底盘控制层的目标转向误差（deg）。
extern volatile float err_degree;

// 导航状态机：由上层任务轮询，判断是否在运行、是否已完成。
extern NavReplayState_e g_replay_state;
// 当前正在逼近或刚触发的点类型，供上层决定是否转圈/跳跃等。
extern uint8 g_current_point_type;
// 特殊动作触发标志；置 1 后导航暂停，由上层动作状态机接管。
extern uint8 g_special_action_trigger;

// 启动导航回放并初始化状态机。
void NavReplay_Start(void);
// 停止导航回放并清空控制输出。
void NavReplay_Stop(void);
// 科目二方案4主循环；建议放在 10ms/20ms 周期任务中调用。
void NavReplay_Process(void);
// 将静态路表加载到 RAM，便于运行时统一按 RAM 数据结构访问。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
