#ifndef _PLAN4_LQR_SPEED_PLANNING_H_
#define _PLAN4_LQR_SPEED_PLANNING_H_

/*
 * Plan4 连续路线跟踪。
 *
 * 路径表由 generate_plan4_smooth_path.py 生成。每个采样点包含切线方向
 * （target_yaw_deg）、曲率和离线规划的可行速度。普通采样点均可直接通过，
 * 不会形成零速度屏障。
 */

#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE           1       // 是否启动时将静态路径表装入 RAM：1=是，0=否

/* 局部线段投影与前瞻参数。生成路径默认每点间隔约 50 mm，因此 120 点约为 6 m。 */
#define PLAN4_LQR_SEARCH_RANGE_POINTS               120U    // 正常跟踪时向前搜索最近线段的最大点数
#define PLAN4_LQR_RECOVER_SEARCH_RANGE_POINTS       300U    // 预留：恢复状态下向前搜索的最大点数
#define PLAN4_LQR_MAX_TRACK_DIST_MM                 900.0f  // 允许采用新投影线段的最大距离（mm）
#define PLAN4_LQR_PROJECTION_MIN_SEG_MM             1.0f    // 可参与投影计算的最小线段长度（mm），避免除零
#define PLAN4_LQR_PREVIEW_POINTS                    5U      // 一般曲率下的前瞻点数
#define PLAN4_LQR_SHARP_CURVATURE_TH                0.0015f // 视为急弯的曲率绝对值阈值（1/mm）
#define PLAN4_LQR_SHARP_PREVIEW_POINTS              2U      // 急弯时缩短后的前瞻点数

/* 转向指令：u = Kff * v*kappa + Ky*ey + Kpsi*epsi。若实车修正方向相反，
 * 应优先检查 PLAN4_LQR_SIGN。 */
#define PLAN4_LQR_SIGN                              1.0f    // 转向指令方向系数：实车修正方向相反时改为 -1.0f
#define PLAN4_LQR_K_LATERAL                         0.030f  // 横向偏差 ey 的反馈增益
#define PLAN4_LQR_K_HEADING                         0.80f   // 航向偏差 epsi 的反馈增益
#define PLAN4_LQR_K_YAW_RATE_FF                     8.0f    // 曲率速度项 v*kappa 的前馈增益
#define PLAN4_LQR_SPEED_TO_MM_S                     4.79f   // 路径表速度指令换算为 mm/s 的系数
#define PLAN4_LQR_ERR_LIMIT_LOW_DEG                 34.0f   // 低速时转向指令的幅值上限（度）
#define PLAN4_LQR_ERR_LIMIT_HIGH_DEG                22.0f   // 高速时转向指令的幅值上限（度）
#define PLAN4_LQR_ERR_SLEW_LOW_DEG                  16.0f   // 低速时每周期转向指令的最大变化量（度）
#define PLAN4_LQR_ERR_SLEW_HIGH_DEG                  8.0f   // 高速时每周期转向指令的最大变化量（度）
#define PLAN4_LQR_FILTER_ALPHA_LOW                  0.75f   // 低速时转向滤波系数，越大越快跟随新指令
#define PLAN4_LQR_FILTER_ALPHA_HIGH                 0.48f   // 高速时转向滤波系数，越小越平滑
#define PLAN4_LQR_LOW_SPEED_MM_S                   500.0f  // 参数由低速档开始插值的速度阈值（mm/s）
#define PLAN4_LQR_HIGH_SPEED_MM_S                 2500.0f  // 参数插值到高速档的速度阈值（mm/s）
#define PLAN4_LQR_LATERAL_ERR_LIMIT_MM             650.0f  // 参与转向计算的横向偏差限幅值（mm）

/* 运行时速度包络仅限制离线速度计划；普通路径跟踪期间始终保留非零速度指令。 */
#define PLAN4_TRACK_MIN_SPEED_CMD                  (-100.0f) // 普通跟踪的最小前进速度指令，负值表示前进
#define PLAN4_TRACK_CROSS_TRACK_SOFT_MM             250.0f  // 开始按横向偏差降速的阈值（mm）
#define PLAN4_TRACK_CROSS_TRACK_HARD_MM             650.0f  // 横向偏差降速达到最大程度的阈值（mm）
#define PLAN4_TRACK_YAW_SOFT_DEG                     35.0f  // 开始按航向偏差降速的阈值（度）
#define PLAN4_TRACK_YAW_HARD_DEG                     80.0f  // 航向偏差降速达到最大程度的阈值（度）
#define PLAN4_SPEED_ACCEL_STEP                       110.0f  // 每周期加速时速度指令的最大变化量
#define PLAN4_SPEED_DECEL_STEP                      200.0f  // 每周期减速时速度指令的最大变化量
#define PLAN4_FINISH_SPEED_DECEL_STEP                20.0f  // 通过末点后每周期速度指令的减速量
#define PLAN4_FINISH_STOP_SPEED_MM_S                150.0f  // 末点减速完成后锁定转向的实测速度阈值（mm/s）

/* 成对视觉任务的路径表在入口前后各保留 600 mm 直线走廊。控制端先在入口
 * 走廊内持续跟踪并收敛航向，再在最后 100 mm 交给任务状态机。 */
#define PLAN4_SPECIAL_ALIGN_DISTANCE_MM             600.0f  // 开始检查入口航向/横向收敛的沿线距离
#define PLAN4_SPECIAL_HANDOFF_LEAD_MM               100.0f  // 满足对准条件后，距入口小于该值才交接（mm）
#define PLAN4_SPECIAL_ALIGN_YAW_FULL_SPEED_DEG        6.0f  // 小于该航向误差时不因入任务对准额外限速
#define PLAN4_SPECIAL_ALIGN_YAW_BLOCK_DEG            28.0f  // 大于该航向误差时仅保留爬行速度
#define PLAN4_SPECIAL_ALIGN_CROSS_FULL_MM            60.0f  // 小于该横向误差时不因入任务对准额外限速
#define PLAN4_SPECIAL_ALIGN_CROSS_BLOCK_MM          220.0f  // 大于该横向误差时仅保留爬行速度
#define PLAN4_SPECIAL_ALIGN_MIN_SPEED_FACTOR          0.25f // 未对准时的最低行驶速度比例，仍可边跑边收敛
#define PLAN4_SPECIAL_ENTRY_YAW_TOLERANCE_DEG         6.0f  // 交接给台阶/单边桥/颠簸状态机的最大航向误差
#define PLAN4_SPECIAL_ENTRY_CROSS_TOLERANCE_MM      100.0f  // 交接给台阶/单边桥/颠簸状态机的最大横向误差
#define PLAN4_SPECIAL_HANDOFF_TICKS                  20U     // 特殊任务结束后恢复 LQR 输出的渐变周期数
#define PLAN4_SPECIAL_HANDOFF_SPEED_STEP             40.0f   // 交接恢复期间每周期速度指令的最大变化量
#define PLAN4_SPECIAL_HANDOFF_ERR_STEP_DEG            1.0f   // 交接恢复期间每周期转向指令的最大变化量（度）

/* 状态机出口的融合坐标会被锚定到路径表出口点。该坐标切换不是实际横向偏差，
 * 因而在出口直线走廊内单独再并线，避免常规偏差保护误判为离轨并长期低速。 */
#define PLAN4_EXIT_REJOIN_DISTANCE_MM               600.0f  // 沿出口直线走廊再并线的长度
#define PLAN4_EXIT_REJOIN_MAX_SPEED_CMD              300.0f  // 再并线期间的速度指令绝对值上限
#define PLAN4_EXIT_REJOIN_EMERGENCY_CROSS_MM        900.0f  // 再并线仍保留保护的横向误差硬阈值
#define PLAN4_EXIT_REJOIN_EMERGENCY_YAW_DEG           95.0f  // 再并线仍保留保护的航向误差硬阈值

/* 雷区转圈结束后，路径表从零速点开始会按离线加速度缓慢恢复。此处设置恢复时
 * 读取前方安全速度的最短前瞻距离，避免出雷区后长期维持低速。 */
#define PLAN4_MINEFIELD_EXIT_SPEED_LOOKAHEAD_MM      800.0f

/* NAV_POINT_CIRCLE（type=1）是仅有入口的雷区标记。驶入雷区前使用
 * Plan2 风格的在线点对点速度规划，不使用普通视觉任务的提前交接距离，
 * 雷区结束后也绝不重定位 nav_vision_fusion_x/y。
 *
 * 以下数值从 plan2_point_speed_planning_lite 迁移而来。Plan4 唯一的适配是：
 * 距离和航向使用 nav_vision_fusion_x/y 计算，以继承前一视觉任务的位置校正；
 * 实际速度仍使用 inertial_nav.vx_body。 */

/* NAV_POINT_MINEFIELD_FLYBY（type=11）是雷区不停点：采用同一套点到点
 * 航向/距离速度曲线，但到达半径后只推进路表索引，不停车、不触发旋转状态机。 */
#define PLAN4_MINEFIELD_FLYBY_ARRIVE_RADIUS_MM       350.0f

/* type=1 标记周围允许触发旋转的最终执行圆半径。
 * 调大：更早触发、较不易冲过点，但转圈中心可能偏离标记；
 * 调小：中心更准确，但刹车误差和 IMU 漂移更容易导致冲过头。 */
#define PLAN4_MINEFIELD_EXECUTE_RADIUS_MM            150.0f

/* 允许触发旋转时的最大实测纵向速度（mm/s）。
 * 调大：更早进入转圈、节省时间，但可能带着滑移旋转；
 * 调小：要求更接近静止，稳定性更好，但耗时更长。 */
#define PLAN4_MINEFIELD_TRIGGER_SPEED_MM_S          1500.0f

/* 动态刹车距离模型，v 的单位为 mm/s：
 * brake_distance_mm = (A*v*v + B*v + C) * RATIO + MARGIN。
 * A/B/C 应根据不同接近速度下的实测刹停距离拟合；RATIO 是场地整体修正系数，
 * 调大则更早刹车，调小则更晚刹车；MARGIN 是额外安全距离。 */
#define PLAN4_MINEFIELD_BRAKE_POLY_A                  0.00025f
#define PLAN4_MINEFIELD_BRAKE_POLY_B                 (-0.2877f)
#define PLAN4_MINEFIELD_BRAKE_POLY_C                887.0f
#define PLAN4_MINEFIELD_BRAKE_DIST_RATIO               1.0f //华东用的0.7f，夜间跑的0.6f白天跑不了
#define PLAN4_MINEFIELD_BRAKE_MARGIN_MM                0.0f

/* 正常接近阶段的最大速度指令（负号表示前进），也是
 * v=sqrt(2*a*剩余距离) 的上限。若进入刹车区仍过快，应优先减小此值。 */
#define PLAN4_MINEFIELD_SPEED_FAST                  (-1000.0f)   //华东赛1300.0f

/* 在线速度曲线 v^2=2*a*s 中的 a（mm/s^2）。
 * 调大：更晚减速、减速更陡；调小：更早减速、接近过程更平缓。 */
#define PLAN4_MINEFIELD_SPEED_DECEL_CMD2_PER_MM       110.0f

/* 已开始刹车但仍在执行圆外时，速度降到阈值后以“触发速度 × 本比例”蠕行补进。
 * 调大：提前刹停后的补进更快；调小：更不易穿过标记点，但耗时更长。 */
#define PLAN4_MINEFIELD_CRAWL_SPEED_RATIO               0.5f

/* 仅用于 type=1 点对点接近的航向门限。
 * 偏差超过 SLOW 门限时停车原地对正；处于 STOP 与 SLOW 门限之间时，
 * 接近速度乘以 0.35。调大门限允许更大的斜向行驶；调小则对点更严格，
 * 但可能频繁停车再转向。 */
#define PLAN4_MINEFIELD_YAW_STOP_TOLERANCE_DEG         18.0f
#define PLAN4_MINEFIELD_YAW_SLOW_TOLERANCE_DEG         35.0f

#define PLAN4_START_HEADING_TOLERANCE_DEG             0.3f   // 起步前航向对正完成的允许误差（度）
#define PLAN4_START_HEADING_ERR_LIMIT_DEG             2.0f   // 起步前航向对正时转向指令的限幅值（度）

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

extern NavReplayState_e g_replay_state;
extern uint16 g_target_idx;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
extern volatile uint8 entry_beep_request;
extern volatile uint8 exit_beep_request;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
