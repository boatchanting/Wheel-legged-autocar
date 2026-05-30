#ifndef _PLAN3_POINT_SPEED_PLANNING_H_
#define _PLAN3_POINT_SPEED_PLANNING_H_

// 科目三方案B：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按点对点逐个寻点，速度由剩余距离在线规划，不依赖离线路表 target_speed。
// 2. 特殊点必须先“到点”，再按该点 target_yaw_deg 原地对准角度；只有位置和角度都满足后才触发状态机。
// 3. 特殊点在接近过程中沿用在线减速 + 提前刹停思路，先把速度压下来，再在点附近做最终对角。
// 4. 转圈点仍然沿用雷区旋转状态机；旋转总角度统一至少 721 度。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE                 1

// 普通路径点的通过半径（mm）；进入后直接推进索引，不停车。
#define NAV_PLAN3_POINT_PATH_ARRIVE_RADIUS                70.0f
// 特殊点的位置到点半径（mm）；只有进入该半径后，才开始按 target_yaw_deg 对角。
#define NAV_PLAN3_POINT_SPECIAL_POS_RADIUS                85.0f
// 特殊点提前刹停准备圆最小半径（mm）。
#define NAV_PLAN3_POINT_SPECIAL_BRAKE_PREP_MIN_RADIUS     650.0f
// 特殊点提前刹停准备圆最大半径（mm）。
#define NAV_PLAN3_POINT_SPECIAL_BRAKE_PREP_MAX_RADIUS     1600.0f
// 特殊点提前刹停准备圆随速度平方放大的系数。
#define NAV_PLAN3_POINT_SPECIAL_BRAKE_SPEED2_RADIUS_GAIN  0.0035f
// 刹车前馈尚未明显建立时，额外放大的准备距离（mm）。
#define NAV_PLAN3_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN      140.0f
// 判断刹车前馈已经明显建立的 PWM 阈值。
#define NAV_PLAN3_POINT_SPECIAL_BRAKE_READY_PWM           1200.0f
// 特殊点位置环进入“超低速补点”前，要求车速先降到该阈值以下（mm/s）。
#define NAV_PLAN3_POINT_SPECIAL_CRAWL_ENTRY_SPEED_MM_S    80.0f
// 特殊点在位置环内补点时使用的低速速度指令。
#define NAV_PLAN3_POINT_SPECIAL_CRAWL_SPEED               (-55.0f)
// 特殊点位置环之外的近距离补点半径（mm）；只有进入该范围且速度足够低时才允许爬行补点。
#define NAV_PLAN3_POINT_SPECIAL_CRAWL_NEAR_RADIUS         260.0f
// 特殊点位置到点后，认为已经停稳的速度阈值（mm/s）。
#define NAV_PLAN3_POINT_STOP_SPEED_MM_S                   80.0f
// 特殊点位置到点后的停稳计数阈值（控制周期数）。
#define NAV_PLAN3_POINT_STOP_STABLE_TICKS                 8U
// 特殊点对准角度时允许触发的角度误差（deg）。
#define NAV_PLAN3_POINT_SPECIAL_YAW_TOLERANCE             1.0f
// 特殊点对准角度时的最大原地转向误差输出（deg）。
#define NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_MAX_ERR         2.0f
// 特殊点位置到点后，对角完成前认为角度稳定的计数阈值（控制周期数）。
#define NAV_PLAN3_POINT_SPECIAL_YAW_STABLE_TICKS          4U
// 位置已到但角度还没对准时，只要速度高于该阈值仍继续要求停住（mm/s）。
#define NAV_PLAN3_POINT_SPECIAL_YAW_ALIGN_SPEED_MM_S      70.0f
// 最终终点的停车半径（mm）。
#define NAV_PLAN3_POINT_FINAL_STOP_RADIUS                 80.0f
// 最终终点的朝向对准容差（deg）。
#define NAV_PLAN3_POINT_FINAL_YAW_TOLERANCE               1.0f
// 起跑前航向对齐容差（deg）。
#define NAV_PLAN3_POINT_START_HEADING_TOLERANCE           0.3f

// 在线速度规划的快速巡航速度指令。
#define NAV_PLAN3_POINT_SPEED_FAST                        (-220.0f)
// 在线速度规划的慢速靠近速度指令。
#define NAV_PLAN3_POINT_SPEED_SLOW                        (-55.0f)
// 停车速度指令。
#define NAV_PLAN3_POINT_SPEED_STOP                        (0.0f)
// v^2 = 2ad 中的“指令域减速度”。
#define NAV_PLAN3_POINT_SPEED_DECEL_CMD2_PER_MM           48.0f
// 速度指令上升斜率。
#define NAV_PLAN3_POINT_SPEED_ACCEL_STEP                  15.0f
// 速度指令正常减速斜率。
#define NAV_PLAN3_POINT_SPEED_DECEL_STEP                  35.0f
// 跨零或瞬时停车时的最大速度变化量。
#define NAV_PLAN3_POINT_SPEED_CROSS_ZERO_STEP             85.0f
// 中等角度偏差阈值（deg）；超过后只允许低速靠近。
#define NAV_PLAN3_POINT_YAW_STOP_TOLERANCE                12.0f
// 大角度偏差阈值（deg）；超过后直接停车原地修方向。
#define NAV_PLAN3_POINT_YAW_SLOW_TOLERANCE                24.0f

// 允许优先倒车/反向朝向的偏置量（deg）。
#define NAV_PLAN3_POINT_REVERSE_SELECT_BIAS_DEG           10.0f
// 雷区旋转最小总角度（deg）；统一按至少 721 度处理。
#define NAV_PLAN3_POINT_SPIN_MIN_TOTAL_ANGLE              721.0f

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
// 科目三方案B主循环；建议放在 10ms/20ms 周期任务中调用。
void NavReplay_Process(void);
// 将静态路表加载到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
