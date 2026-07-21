#ifndef _PLAN3_PRECISE_H_
#define _PLAN3_PRECISE_H_

// 1：每次启动时都从 nav_replay_route_table.h 装载编译期路线；0：使用 RAM 中已有路线。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE  1

// ========================= 科目三方法一关键调参区 =========================
// 距离单位：mm。
#define NAV_DIST_FAR                    500.0f  // 距目标大于此值时使用高速；减小会更早开始降速。
#define NAV_DIST_NEAR                   150.0f  // 进入低速逼近区的距离；增大可给停车和对角留出更多余量。
#define NAV_DIST_ARRIVE                  20.0f  // 判定到达目标点的半径；过小易因定位抖动无法到点，过大则会提前触发。

// 角度单位：deg。
#define NAV_YAW_TOLERANCE                 1.0f  // 行驶/特殊点对角允许误差；增大更容易起步，减小则对点更准但可能反复转向。
#define NAV_START_HEADING_TOLERANCE       0.3f  // 启动前绝对航向对齐阈值，仅 IMU_CATEGORY == 3 且路线记录了起跑航向时生效。

// 速度单位沿用底盘 target_speed_set；本车约定负值为前进。
#define NAV_SPEED_FAST                (-140.0f) // 远距离直行速度；提高前需确认定位、底盘和平衡余量。
#define NAV_SPEED_SLOW                 (-40.0f) // 近点和特殊点前的逼近速度；降低可减轻过冲。
#define NAV_SPEED_STOP                   0.0f   // 停车/原地对角指令。

// 方向控制保护参数。建议一次只改一个，并优先在单元素路线验证。
#define PLAN3_MAX_SPIN_ERR_DEG            2.0f  // 原地对角时单周期最大误差输出；增大转得更快，但更易过冲。
#define PLAN3_MAX_APPROACH_ERR_DEG         4.0f  // 正常逼近时最大方向误差输出；增大循迹更激进，过大易摆动。
#define PLAN3_ANGLE_FILTER_ALPHA           0.3f  // 方向误差一阶滤波系数(0~1)：越小越平顺但越迟钝，越大越跟手。
#define PLAN3_NEAR_POINT_ERROR_WINDOW    150.0f // 距离到点阈值外的近点保护窗口；进入后额外限制方向输出。
#define PLAN3_NEAR_POINT_MAX_ERR_DEG      15.0f // 近点保护窗口内允许的最大方向误差，避免在目标点附近猛打方向。

extern volatile float target_speed_set;
extern volatile float err_degree;

extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;

uint16 NavReplay_LoadStaticRouteToRam(void);
void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);

#endif
