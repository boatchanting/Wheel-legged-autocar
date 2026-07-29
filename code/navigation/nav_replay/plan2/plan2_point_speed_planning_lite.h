#ifndef _PLAN2_POINT_SPEED_PLANNING_LITE_H_
#define _PLAN2_POINT_SPEED_PLANNING_LITE_H_

// 科目二方案4：点对点 + 在线速度规划
// 核心思路：
// 1. 普通路径点按点对点导航，速度由剩余距离在线规划。
// 2. 雷区点不再等进中心后才慢慢收速，而是进入准备区就直接给 0 速度，
//    让底层普通刹车前馈尽快介入；速度压下来后再用超低速补进中心。
// 3. 雷区旋转从触发瞬间的车头角开始规划，至少 725 度后若出口航向可用则提前释放。

// 1 表示直接使用编译期静态路表，不再依赖 Flash 读表。
// [调参效果]：设为 1 时，每次烧录代码会直接读取 csv_to_nav_table.py 生成的静态数组；设为 0 时需配合特定上位机写入 Flash。通常保持为 1。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE       1

// 普通路径点的到达判定半径（mm）。进入该半径后，直接切换到下一个目标点，不停车。
// [调参效果]：调大该值，车子会在距离路径点更远的地方提前“切角”转向，轨迹更圆滑但偏离原始打点；调小该值，车子会更精确地压点，但可能导致车身频繁左右摇摆或降速。
#define NAV_POINT_PATH_ARRIVE_RADIUS            70.0f

// 特殊动作（如雷区旋转）的最终执行中心圈半径（mm）。
// [调参效果]：小车到达此半径内，且速度降至阈值下，才会触发特殊动作。调大该值，会在离雷区中心较远（边缘区）提前开始旋转；调小该值，会强制车子更深入圆心才开始旋转，更严谨但也更容易冲过头。
#define NAV_POINT_SPECIAL_EXECUTE_RADIUS        200.0f

// （当前 Lite 版本未使用）特殊动作的准备刹车半径。
// [调参效果]：原完整版中用于更早触发减速的参数，Lite 版已改为按速度动态计算。
#define NAV_POINT_SPECIAL_PREP_STOP_RADIUS      200.0f

// （当前 Lite 版本未使用）雷区释放蠕动距离的边距。
#define NAV_POINT_SPECIAL_CRAWL_RELEASE_MARGIN_MM 80.0f

// （当前 Lite 版本未使用）动态刹车预测的减速度 (mm/s^2)。
// [调参效果]：原完整版参数。当前 Lite 版已改为使用二阶多项式方程独立计算刹车距离。
#define NAV_POINT_SPECIAL_BRAKE_DECEL_MM_S2     170.0f

// 刹车安全余量（mm），加在雷区执行半径之上，用于提前触发刹车。
// [调参效果]：调大该值，小车会更早识别到前方是雷区并提前下发刹车指令，防止冲过头；调小该值，刹车时机会更晚更极限，如果在光滑路面极易冲过雷区中心。
#define NAV_POINT_SPECIAL_BRAKE_MARGIN_MM       100.0f

// （当前 Lite 版本未使用）刹车半径的下限兜底值（mm）。
#define NAV_POINT_SPECIAL_BRAKE_RADIUS_MIN      300.0f

// （当前 Lite 版本未使用）刹车半径的上限安全钳位值（mm）。
#define NAV_POINT_SPECIAL_BRAKE_RADIUS_MAX      4000.0f

// （当前 Lite 版本未使用）稳定刹停的前馈预设值。
#define NAV_POINT_SPECIAL_BRAKE_READY_PWM       1200.0f

// （当前 Lite 版本未使用）刹车前馈补充边距。
#define NAV_POINT_SPECIAL_BRAKE_WEAK_FF_MARGIN  180.0f

// （当前 Lite 版本未使用）刹车后蠕动补进中心的速度。
#define NAV_POINT_SPECIAL_STEP_IN_SPEED         (-240.0f)

// （当前 Lite 版本未使用）蠕动补进开始的速度阈值。
#define NAV_POINT_SPECIAL_STEP_IN_START_SPEED_MM_S 250.0f

// 执行特殊动作（雷区旋转）允许的最大实际速度绝对值（mm/s）。
// [调参效果]：进入执行圈后，只有车速低于此值才会真正开始旋转。调大该值，车还没完全停稳就急着旋转，离心力过大极易打滑翻车；调小该值（比如降到 500），要求必须彻底刹停才转，稳定性好但耗时更长。
#define NAV_POINT_SPECIAL_TRIGGER_SPEED_MM_S    1500.0f

// （当前 Lite 版本未使用）允许倒车补进执行圆的角度阈值。
#define NAV_POINT_SPECIAL_REVERSE_RECOVER_YAW_MIN 110.0f

// （当前 Lite 版本未使用）最后一个点的判定通过半径。
#define NAV_POINT_FINAL_PASS_RADIUS             350.0f

// （当前 Lite 版本未使用）最后冲线的横向宽度判定半径。
#define NAV_POINT_FINAL_PASS_LATERAL_RADIUS     500.0f

// 中等航向误差限速阈值（deg）。
// [调参效果]：当车头偏离目标点的角度大于此值时，巡航速度会被强制乘以 0.35 降速。调大该值，车子容忍更大的偏航不减速，速度快但容易偏离轨迹；调小该值，车子稍有偏航就会频繁刹车减速，行驶顿挫感强。
#define NAV_POINT_YAW_STOP_TOLERANCE            18.0f

// 极大航向误差停车阈值（deg）。
// [调参效果]：偏航角大于此值时，目标速度直接设为 0，小车原地打转修正方向。调大该值，车子允许“斜着身子”继续往前开，对横向滑动容忍度高；调小该值，一旦方向不对就死死停住原地修方向，安全性高但可能在打滑路面频繁卡顿。
#define NAV_POINT_YAW_SLOW_TOLERANCE            35.0f

// （当前 Lite 版本未使用）雷区旋转结束后的移动对准窗口期。
#define NAV_POINT_SPIN_EXIT_ALIGN_TICKS         20U

// （当前 Lite 版本未使用）雷区退出允许的最大残余误差。
#define NAV_POINT_SPIN_EXIT_MOVE_YAW_MAX        90.0f

// （当前 Lite 版本未使用）雷区退出时的初速度比例限制。
#define NAV_POINT_SPIN_EXIT_SPEED_RATIO         0.9f

// （当前 Lite 版本未使用）允许优先倒车/反向朝向的偏置量。
#define NAV_POINT_REVERSE_SELECT_BIAS_DEG       10.0f

// （当前 Lite 版本未使用）起跑前航向对齐容差。
#define NAV_POINT_START_HEADING_TOLERANCE       0.3f

// 绕桩/飞越点（Flyby）的提前切角判定半径（mm）。
// [调参效果]：专门用于科目二高速绕桩。调大该值，车子离桩桶很远就会认定该点完成并切向下一个点，走的是大平缓弧线（容易压线或漏桩）；调小该值，车子会贴着桩桶开，转弯极锐利，但速度过快时离心力大极易打滑摔倒。
#define NAV_POINT_FLYBY_ARRIVE_RADIUS           350.0f

// 正常巡航的最大速度绝对限制 (负号表示前进)。
// [调参效果]：决定了普通直道/弯道能达到的最高速度。绝对值调大（如 -900），整体巡航变快，更容易突破轮胎极限；绝对值调小，跑得更慢更稳妥。
#define NAV_POINT_SPEED_FAST                    (-800.0f)

// 绕桩点（Flyby）的专属固定目标速度。
// [调参效果]：绕桩时的专属限速。绝对值调大，绕桩更激进，极度考验底盘向心力；绝对值调小，绕桩保守，防滑效果好。
#define NAV_POINT_SPEED_FLYBY                   (-1000.0f)

// 冲线阶段（最后冲刺）的固定全速目标速度。
// [调参效果]：跑完所有路表点后，沿垂线冲向终点线（Y轴）的速度。绝对值调大，冲刺极速更高；绝对值调小，冲刺求稳。
#define NAV_POINT_SPEED_DASH                    (-1000.0f)

// 冲线后多跑的缓冲距离（mm），即越过初始 Y 轴（X=0）多远才认定冲线完成触发硬刹车。
// [调参效果]：调大该值，车子会冲出起点界限很远才开始停下；调小该值（如 100），车子刚压线就踩死刹车。注意结合场地的实际缓冲空间调节，防止撞墙。
#define NAV_POINT_DASH_OVERRUN_MM               1000.0f

// （当前 Lite 版本未使用）降速逼近的目标速度。
#define NAV_POINT_SPEED_SLOW                    (-220.0f)

// （当前 Lite 版本未使用）停车目标速度宏，直接填了 0。
#define NAV_POINT_SPEED_STOP                    (0.0f)

// 常规路径点减速曲线的虚拟加速度系数（公式 v^2 = 2as 中的 a）。
// [调参效果]：决定了接近普通点时的降速平滑度。调大该值，车子会“晚刹车、猛刹车”，减速曲线极陡；调小该值，车子离点很远就开始平缓溜车降速。
#define NAV_POINT_SPEED_DECEL_CMD2_PER_MM       110.0f

// （以下参数在当前 Lite 版本中均未使用，属于完整版或旧版留存参数）
#define NAV_POINT_STOP_PREDICT_DT_S             0.010f
#define NAV_POINT_STOP_PREDICT_DEADBAND_MM      35.0f
#define NAV_POINT_STOP_PREDICT_DECEL_ALPHA      0.25f
#define NAV_POINT_STOP_PREDICT_DECEL_MIN        20.0f
#define NAV_POINT_STOP_PREDICT_DECEL_MAX        300.0f
#define NAV_POINT_SPEED_ACCEL_STEP              18.0f
#define NAV_POINT_SPEED_DECEL_STEP              45.0f
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
