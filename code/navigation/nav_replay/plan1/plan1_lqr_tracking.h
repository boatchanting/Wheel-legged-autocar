#ifndef _PLAN1_LQR_TRACKING_H_
#define _PLAN1_LQR_TRACKING_H_

// 1: use compile-time route table generated from CSV (no flash dependency).
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

// Basic replay thresholds.
#define NAV_DIST_ARRIVE                    20.0f
#define NAV_START_HEADING_TOLERANCE         0.3f
#define NAV_SPEED_STOP                      0.0f

// Plan1 simplified LQR reference selection.
#define LQR_PREVIEW_POINTS                  3U
#define LQR_SEARCH_RANGE_NORMAL            80U
#define LQR_SEARCH_RANGE_RECOVER          300U
#define LQR_MAX_TRACK_DIST_MM             800.0f
#define LQR_PROJECTION_MIN_SEG_LEN_MM       1.0f

// Control gains. Curvature is about 1/mm, so LQR_K_CURV maps 1/mm to deg.
#define LQR_SIGN                            1.0f
#define LQR_K_CURV                       9000.0f
#define LQR_K_LATERAL                       0.012f
#define LQR_K_HEADING                       0.85f

// Curvature feedforward direction handling. target_speed < 0 is forward.
#define LQR_CURVATURE_SPEED_SIGN_ENABLE     1
#define LQR_FORWARD_SPEED_IS_NEGATIVE       1

// Output smoothing.
#define LQR_ERR_MAX_DEG                    35.0f
#define LQR_ERR_SLEW_DEG                   18.0f
#define LQR_FILTER_ALPHA                    0.35f
#define LQR_LATERAL_ERR_LIMIT_MM          500.0f

// Speed command slew-rate. NavReplay_Process() is expected to run near 10-20 ms.
#define NAV_SPEED_SLEW_EPS                  1.0f
#define NAV_SPEED_SLEW_LOW_SPEED_TH        80.0f
#define NAV_SPEED_SLEW_FAST_DECEL_TH      220.0f
#define NAV_SPEED_SLEW_UP_LOW              30.0f
#define NAV_SPEED_SLEW_UP_NORMAL           45.0f
#define NAV_SPEED_SLEW_DOWN_NORMAL         65.0f
#define NAV_SPEED_SLEW_DOWN_FAST           95.0f
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO    120.0f
#define NAV_STOP_LOCK_SPEED_EPS             1.0f

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

extern NavReplayState_e g_replay_state;
extern uint16 g_target_idx;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
