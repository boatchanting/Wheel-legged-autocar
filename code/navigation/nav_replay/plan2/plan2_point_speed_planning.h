#ifndef _PLAN2_POINT_SPEED_PLANNING_H_
#define _PLAN2_POINT_SPEED_PLANNING_H_

// 科目二方案4：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按点对点导航，速度由剩余距离在线规划。
// 2. 雷区点不再等进中心后才慢慢收速，而是进入准备区就直接给 0 速度，
//    让底层普通刹车前馈尽快介入；速度压下来后再用超低速补进中心。
// 3. 雷区旋转从触发瞬间的车头角开始规划，至少 725 度后若出口航向可用则提前释放。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的通过半径（mm）；进入后直接推进索引，不停车。
#define NAV_POINT_PATH_ARRIVE_RADIUS            70.0f
// 特殊动作最终执行中心圈半径（mm）；小于雷区物理半径，避免边缘或外侧提前旋转。
#define NAV_POINT_SPECIAL_EXECUTE_RADIUS        150.0f
#define NAV_POINT_SPECIAL_PREP_STOP_RADIUS      300.0f
#define NAV_POINT_SPECIAL_CRAWL_RELEASE_MARGIN_MM 80.0f
// 特殊点提前刹车按当前接近速度动态计算：执行圆 + 余量 + v^2/(2a)。
#define NAV_POINT_SPECIAL_BRAKE_DECEL_MM_S2     170.0f
#define NAV_POINT_SPECIAL_BRAKE_MARGIN_MM       100.0f
#define NAV_POINT_SPECIAL_BRAKE_RADIUS_MIN      3300.0f
#define NAV_POINT_SPECIAL_BRAKE_RADIUS_MAX      3300.0f
// 普通刹车前馈尚未明显建压时，额外放大准备区，复用 stable tag 的稳定刹停节奏。
#define NAV_POINT_SPECIAL_BRAKE_READY_PWM       1200.0f
#define NAV_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN  180.0f
// 刹到可执行低速后、但尚未进入执行圈时，用低于执行阈值的速度正常 PID 补进中心。
// 补进速度必须低于执行准许速度，避免刚进执行圆时速度超限。
#define NAV_POINT_SPECIAL_STEP_IN_SPEED         (-90.0f)
#define NAV_POINT_SPECIAL_STEP_IN_START_SPEED_MM_S 100.0f
// 执行动作允许的最大实际速度绝对值（mm/s）；进执行圈且 |实际速度| 不超过该值才开转。
#define NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S    100.0f
// 高速冲过雷区中心后，目标点已明显落在车后方时，允许倒车低速补回执行圆。
#define NAV_POINT_SPECIAL_REVERSE_RECOVER_YAW_MIN 110.0f
// 最后点通过结束半径（mm）：只判定完成，不强制精确停车。
#define NAV_POINT_FINAL_PASS_RADIUS             350.0f
// High-speed finish fallback: if the car crosses the last segment end line
// within this lateral width, finish even when one 10ms tick skips the radius.
#define NAV_POINT_FINAL_PASS_LATERAL_RADIUS     500.0f
// 中等角度偏差阈值（deg）；超过后只允许低速逼近。
#define NAV_POINT_YAW_STOP_TOLERANCE            18.0f
// 大角度偏差阈值（deg）；超过后直接停车原地修方向。
#define NAV_POINT_YAW_SLOW_TOLERANCE            35.0f
// 雷区旋转结束后的移动对准窗口周期数；窗口内允许边低速出发边修正航向。
#define NAV_POINT_SPIN_EXIT_ALIGN_TICKS         20U
// 移动对准允许的最大残余航向误差（deg）；超过后仍然原地修正，避免方向明显错误时硬冲。
#define NAV_POINT_SPIN_EXIT_MOVE_YAW_MAX        90.0f
// 移动对准低速上限占正常速度的比例；0.5 表示最多按正常速度的一半出发。
#define NAV_POINT_SPIN_EXIT_SPEED_RATIO         0.5f
// 允许优先倒车/反向朝向的偏置量（deg）。
#define NAV_POINT_REVERSE_SELECT_BIAS_DEG       10.0f
// 起跑前航向对齐容差（deg）。
#define NAV_POINT_START_HEADING_TOLERANCE       0.3f

// 在线速度规划的快速巡航速度指令。
#define NAV_POINT_SPEED_FAST                    (-800.0f)
// 在线速度规划的慢速逼近速度指令。
#define NAV_POINT_SPEED_SLOW                    (-120.0f)
// 停车速度指令。
#define NAV_POINT_SPEED_STOP                    (0.0f)
// v^2 = 2ad 中的“指令域减速度”。
#define NAV_POINT_SPEED_DECEL_CMD2_PER_MM       110.0f
// 编码器刹停预测的导航调用周期（s）；NavReplay_Process 当前约 10ms 调用一次。
#define NAV_POINT_STOP_PREDICT_DT_S             0.010f
// 预测刹停点相对目标边界的死区（mm）；小误差不调目标速度，避免来回抖。
#define NAV_POINT_STOP_PREDICT_DEADBAND_MM      35.0f
// 编码器减速度估计滤波系数；越大越跟手，越小越平稳。
#define NAV_POINT_STOP_PREDICT_DECEL_ALPHA      0.25f
// 编码器减速度估计下限，避免接近 0 时停车距离发散。
#define NAV_POINT_STOP_PREDICT_DECEL_MIN        20.0f
// 编码器减速度估计上限，避免瞬时编码器噪声让目标速度突然放大。
#define NAV_POINT_STOP_PREDICT_DECEL_MAX        300.0f
// 速度指令上升斜率。
#define NAV_POINT_SPEED_ACCEL_STEP              18.0f
// 速度指令正常减速斜率。
#define NAV_POINT_SPEED_DECEL_STEP              45.0f
// 跨零或瞬时停车时的最大速度变化量。
#define NAV_POINT_SPEED_CROSS_ZERO_STEP         90.0f

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
// 最近一次雷区旋转规划调试量：direction 1=CW，-1=CCW。
extern volatile uint16 g_nav_point_spin_debug_idx;
extern volatile float g_nav_point_spin_debug_current_yaw;
extern volatile float g_nav_point_spin_debug_exit_yaw;
extern volatile float g_nav_point_spin_debug_total_angle;
extern volatile float g_nav_point_spin_debug_direction;
extern volatile float g_nav_point_spin_debug_cw_total_angle;
extern volatile float g_nav_point_spin_debug_ccw_total_angle;
extern volatile uint16 g_nav_point_special_debug_target_idx;
extern volatile float g_nav_point_special_debug_target_x;
extern volatile float g_nav_point_special_debug_target_y;
extern volatile float g_nav_point_special_debug_dist_mm;
extern volatile float g_nav_point_special_debug_brake_radius_mm;
extern volatile float g_nav_point_special_debug_speed_ref_mm_s;
extern volatile uint8 g_nav_point_special_debug_zero_brake_issued;
extern volatile uint8 g_nav_point_special_debug_zero_brake_active;

// 启动导航回放并初始化状态机。
void NavReplay_Start(void);
// 停止导航回放并清空控制输出。
void NavReplay_Stop(void);
// 科目二方案4主循环；建议放在 10ms/20ms 周期任务中调用。
void NavReplay_Process(void);
uint8 NavReplay_SpecialPointZeroBrakeActive(void);
uint8 NavReplay_SpecialPointCrawlActive(void);
uint8 NavReplay_SpecialPointPrepZeroBrakeLatched(void);
// 将静态路表加载到 RAM。
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
