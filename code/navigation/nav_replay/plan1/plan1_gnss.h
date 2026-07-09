#ifndef _PLAN1_GNSS_H_
#define _PLAN1_GNSS_H_
#if GNSS_NAV == 1
//---------------------------------------------
//--------------GPS+惯导融合寻迹逻辑-------------
//--------------------------------------------
#define GPS_NAV_REPLAY_USE_STATIC_ROUTE_TABLE 1

#define GPS_NAV_MIN_SAT_USED              4U

// === Pure Pursuit 配置 ===
// 前瞻距离（决定过弯丝滑度，数值越大越喜欢切内道，2500.0f 表示看向前方 2.5 米）
#define GPS_NAV_LOOKAHEAD_DIST            2500.0f
#define GPS_NAV_DIST_ARRIVE               500.0f   // 仅用于终点停车的判定距离 (0.5米)
#define GPS_NAV_DIST_NEAR                 2200.0f  // 速度控制的远近临界值

#define GPS_NAV_HEADING_OFFSET_DEG        0.0f
#define GPS_NAV_SPEED_FAST                -600.0f
#define GPS_NAV_SPEED_SLOW                -80.0f
#define GPS_NAV_SPEED_STOP                0.0f

extern NavReplayState_e g_replay_state;         // 当前复现状态
extern uint8 g_current_point_type;              // 当前正在前往/到达的点的类型
extern uint8 g_special_action_trigger;          // 特殊动作触发标志 (1: 到达特殊点，请执行动作)
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

uint16 GpsNavReplay_LoadStaticRouteToRam(void);
void GpsNavReplay_Start(void);
void GpsNavReplay_Stop(void);
void GpsNavReplay_Process(void);

#if (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
#endif
#endif
#endif
