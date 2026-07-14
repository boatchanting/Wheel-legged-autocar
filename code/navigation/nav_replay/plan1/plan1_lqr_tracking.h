#ifndef _PLAN1_LQR_TRACKING_H_
#define _PLAN1_LQR_TRACKING_H_

/*
 * Plan1 简化 LQR / 曲率前馈路径跟踪方案
 *
 * 输出仍然沿用全车已有接口：
 *   target_speed_set：底盘目标速度，来自路径表 target_speed，并做斜率限制
 *   err_degree      ：底盘转向误差角，来自曲率前馈 + 横向误差 + 航向误差
 *
 * 控制律：
 *   err_raw = LQR_SIGN * (LQR_K_CURV * curvature * speed_sign
 *                       + LQR_K_LATERAL * e_y
 *                       + LQR_K_HEADING * e_psi)
 *
 * 注意：
 *   1. 路径表 x/y 单位为 mm。
 *   2. target_yaw_deg 使用路径切线航向，不再用纯追踪前瞻点算目标航向。
 *   3. curvature 约为 1/mm，数值通常很小，所以 LQR_K_CURV 会比较大。
 *   4. 负速度代表前进，本文件默认按这个约定处理曲率前馈方向。
 */

// 是否使用编译期静态路径表。1：从 nav_replay_route_table.h 装载；0：使用 RAM 内已有打点数据。
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

// 终点到达距离，单位 mm。调大：更早判定完成；调小：终点更精确，但可能在终点附近多抖一会。
#define NAV_DIST_ARRIVE                    20.0f

// 起跑航向对齐阈值，单位 deg。调大：更容易起跑；调小：起跑姿态更严格，但可能等待更久。
#define NAV_START_HEADING_TOLERANCE         0.3f

// 停车速度指令。一般保持 0。
#define NAV_SPEED_STOP                      0.0f

// 参考点往前看几个路径点。调大：更平顺、提前看弯，但可能切弯；调小：更贴线，但绕桩更容易抖。
#define LQR_PREVIEW_POINTS                  4U

// 急弯判断阈值，单位约为 1/mm。调大：只有更急的弯才启用急弯软化；调小：普通弯也会更柔、更保守。
#define LQR_SHARP_CURVATURE_TH              0.0010f

// 急弯附近使用的预瞄点数。调大：提前看弯、入弯更主动；调小：入弯更稳，不容易突然崴脚，但可能稍微晚一点转。
#define LQR_SHARP_PREVIEW_POINTS            6U

// 正常跟踪时最近点向前搜索窗口，单位：点数。调大：更容易追上索引；调小：更不容易跳到相似回程段。
#define LQR_SEARCH_RANGE_NORMAL            80U

// 特殊动作恢复后的搜索窗口，单位：点数。调大：恢复时更容易重新吸回路径；调小：更保守，不容易跳索引。
#define LQR_SEARCH_RANGE_RECOVER          300U

// 正常跟踪允许的最大离线距离，单位 mm。超过后不更新最近点，防止索引乱跳。
// 调大：离线很远也会尝试找新点；调小：索引更稳，但严重偏离时恢复慢。
#define LQR_MAX_TRACK_DIST_MM             800.0f

// 投影线段最小长度，单位 mm。小于该长度认为两点太近，不做投影，防止除以很小的数。
#define LQR_PROJECTION_MIN_SEG_LEN_MM       1.0f

// 总输出方向符号。整车转向方向完全反了时，优先把 1.0f 改成 -1.0f，不要到处改公式。
#define LQR_SIGN                            1.0f

// 曲率前馈增益。调大：进弯更主动、弯道滞后少；过大：提前扎进弯里，容易切桩。
// 调小：弯道更依赖反馈，车更稳但进弯可能慢半拍。
#define LQR_K_CURV                       6000.0f

// 横向误差增益，单位约 deg/mm。调大：离线后回线更快；过大：绕桩左右摆、贴线过猛。
// 调小：更柔和，但可能带着横向偏差跑。
#define LQR_K_LATERAL                       0.010f

// 航向误差增益，单位约 deg/deg。调大：车头更快对准路径切线；过大：掉头出口或绕桩容易抽。
// 调小：航向修正更慢，可能出现车身斜着贴线走。
#define LQR_K_HEADING                       0.65f

// 是否根据速度方向修正曲率前馈符号。1：启用；0：曲率直接使用路径表符号。
#define LQR_CURVATURE_SPEED_SIGN_ENABLE     1

// 速度方向约定。1：target_speed < 0 为前进；0：target_speed >= 0 为前进。
#define LQR_FORWARD_SPEED_IS_NEGATIVE       1

// err_degree 最大限幅，单位 deg。调大：极端偏差恢复能力强；过大：可能猛打方向。
// 调小：输出更稳，但偏差大时可能拉不回来。
#define LQR_ERR_MAX_DEG                    28.0f

// err_degree 单周期最大变化量，单位 deg/周期。调大：响应快；过大：绕桩抽搐。
// 调小：输出平顺；过小：进弯跟不上。
#define LQR_ERR_SLEW_DEG                   10.0f

// 急弯段 err_degree 单周期最大变化量，单位 deg/周期。调大：急弯响应更快；调小：急弯入口更柔，跳轮/颤动更少。
#define LQR_SHARP_ERR_SLEW_DEG             10.0f

// err_degree 一阶低通系数，范围 0~1。调大：更跟手；过大：噪声和曲率突变更明显。
// 调小：更平顺；过小：明显滞后，容易切弯。
#define LQR_FILTER_ALPHA                    0.25f

// 急弯段低通滤波系数。调大：急弯跟手但更容易颤；调小：急弯更顺，但太小会入弯滞后。
#define LQR_SHARP_FILTER_ALPHA              0.30f

// 横向误差限幅，单位 mm。调大：大偏差时纠偏更强；过大：离线后可能突然猛打。
// 调小：极端情况下更稳，但回线能力变弱。
#define LQR_LATERAL_ERR_LIMIT_MM          350.0f

// 速度变化死区，单位为速度指令。调大：小速度波动会被忽略；调小：更严格跟随路径表速度。
#define NAV_SPEED_SLEW_EPS                  50.0f

// 低速/正常加速分界。低于该值用 NAV_SPEED_SLEW_UP_LOW，高于该值用 NAV_SPEED_SLEW_UP_NORMAL。
#define NAV_SPEED_SLEW_LOW_SPEED_TH        80.0f

// 高速减速分界。高于该值允许使用更大的减速步长 NAV_SPEED_SLEW_DOWN_FAST。
#define NAV_SPEED_SLEW_FAST_DECEL_TH      220.0f

// 低速起步加速步长。调大：起步更冲；调小：起步更柔。
#define NAV_SPEED_SLEW_UP_LOW              30.0f

// 正常加速步长。调大：出弯提速更快；调小：速度变化更平顺。
#define NAV_SPEED_SLEW_UP_NORMAL           45.0f

// 正常减速步长。调大：弯前收速更快；调小：减速更柔但可能进弯偏快。
#define NAV_SPEED_SLEW_DOWN_NORMAL         400.0f

// 高速减速步长。调大：高速进弯收得更狠；过大：体感像急刹。
#define NAV_SPEED_SLEW_DOWN_FAST           800.0f

// 跨零或停车时的速度步长。调大：停车更干脆；调小：停车更缓。
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO    120.0f

// 判断路径点是否为停车屏障的速度阈值。小于该值认为是零速点。
#define NAV_STOP_LOCK_SPEED_EPS             1.0f

// 底盘控制输出，由其他控制模块实际消费。
extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

// 回放状态变量，沿用其他 Plan1 方案命名，供上层任务和特殊动作状态机查询。
extern NavReplayState_e g_replay_state;
extern uint16 g_target_idx;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
// 科目一极速掉头运行态：0空闲，1接近动作点，2跳轮/大转角，3急刹低速转向，4后段跟踪，5完成。
extern uint8 g_plan1_fast_uturn_state;
// 科目一极速掉头接入端：0未选择，1车头超前，2车尾超前。
extern uint8 g_plan1_fast_uturn_lead;
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

// Plan1 回放统一接口，和纯追踪方案保持一致。
void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
