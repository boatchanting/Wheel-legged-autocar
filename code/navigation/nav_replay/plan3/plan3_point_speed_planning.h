#ifndef _PLAN3_POINT_SPEED_PLANNING_H_
#define _PLAN3_POINT_SPEED_PLANNING_H_

// 科目三方案B：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按“当前位置 -> 当前目标点”做逐点逼近，速度由剩余距离在线规划。
// 2. 特殊点进入刹车准备区后先把速度压下来，再用超低速补进目标点中心。
// 3. 特殊点必须同时满足“到点 + 车速足够低 + 对准 target_yaw_deg”后，才触发对应状态机。
// 4. 终点也按“到点 + 对准角度”收尾，避免最后一个点只到位置、不收姿态。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的通过半径（mm）；进入后直接推进索引，不停车。
#define NAV_POINT_PATH_ARRIVE_RADIUS            70.0f
// 特殊点的中心停车半径（mm）；只有进入该范围后才允许开始姿态对准并触发动作。
#define NAV_POINT_SPECIAL_STOP_RADIUS           60.0f
// 特殊点执行前的近距离补进半径（mm）；只在已经很靠近且速度很低时才慢速补进。
#define NAV_POINT_SPECIAL_CRAWL_NEAR_RADIUS     180.0f
// 特殊点动态刹车准备圆最小半径（mm）。
#define NAV_POINT_SPECIAL_BRAKE_PREP_MIN_RADIUS 420.0f
// 特殊点动态刹车准备圆最大半径（mm）。
#define NAV_POINT_SPECIAL_BRAKE_PREP_MAX_RADIUS 1400.0f
// 速度平方到准备圆半径的换算系数；速度越快，准备圆越大。
#define NAV_POINT_SPECIAL_BRAKE_SPEED2_RADIUS_GAIN 0.0032f
// 判断刹车前馈已经明显建压的 PWM 阈值。
#define NAV_POINT_SPECIAL_BRAKE_READY_PWM       1200.0f
// 刹车前馈偏弱时，额外放大的准备距离（mm）。
#define NAV_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN  120.0f
// 特殊点补进中心时使用的低速速度指令。
#define NAV_POINT_SPECIAL_CRAWL_SPEED           (-55.0f)
// 当前速度低于该阈值（mm/s）后，才从“直接刹停”切到“低速补进”。
#define NAV_POINT_SPECIAL_CRAWL_ENTRY_SPEED_MM_S 70.0f
// 特殊点强停刹车释放速度（mm/s）；低于该速度后关闭重刹，避免在点边反复抽搐。
#define NAV_POINT_SPECIAL_HARD_BRAKE_RELEASE_SPEED_MM_S 90.0f
// 特殊点触发前允许认为“已经足够低速”的阈值（mm/s）。
#define NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S    70.0f
// 特殊点目标角对准容差（deg）；位置到点后必须对准该角度才允许触发状态机。
#define NAV_POINT_SPECIAL_TARGET_YAW_TOLERANCE  1.5f
// 特殊点原地对角时输出到底盘的最大误差幅值（deg）。
#define NAV_POINT_SPECIAL_ALIGN_MAX_ERR         2.0f
// 最终终点的停车半径（mm）。
#define NAV_POINT_FINAL_STOP_RADIUS             70.0f
// 终点目标角对准容差（deg）。
#define NAV_POINT_FINAL_YAW_TOLERANCE           1.5f
// 中等角度偏差阈值（deg）；超过后只允许低速逼近。
#define NAV_POINT_YAW_STOP_TOLERANCE            18.0f
// 大角度偏差阈值（deg）；超过后直接停车原地修方向。
#define NAV_POINT_YAW_SLOW_TOLERANCE            35.0f
// 停稳判定计数阈值（周期数）；满足后才允许触发特殊动作或结束回放。
#define NAV_POINT_STOP_STABLE_TICKS             10U
// 认为“已经停稳”的车体速度阈值（mm/s）。
#define NAV_POINT_STOP_SPEED_MM_S               70.0f
// 起跑前航向对齐容差（deg）。
#define NAV_POINT_START_HEADING_TOLERANCE       0.3f

// 在线速度规划的快速巡航速度指令。
#define NAV_POINT_SPEED_FAST                    (-220.0f)
// 在线速度规划的慢速逼近速度指令。
#define NAV_POINT_SPEED_SLOW                    (-70.0f)
// 停车速度指令。
#define NAV_POINT_SPEED_STOP                    (0.0f)
// v^2 = 2ad 中的“指令域减速度”。
#define NAV_POINT_SPEED_DECEL_CMD2_PER_MM       55.0f
// 速度指令上升斜率。
#define NAV_POINT_SPEED_ACCEL_STEP              16.0f
// 速度指令正常减速斜率。
#define NAV_POINT_SPEED_DECEL_STEP              40.0f
// 跨零或瞬时停车时的最大速度变化量。
#define NAV_POINT_SPEED_CROSS_ZERO_STEP         80.0f

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
