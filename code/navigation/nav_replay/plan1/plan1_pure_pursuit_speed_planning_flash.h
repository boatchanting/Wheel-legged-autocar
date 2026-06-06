#ifndef _PLAN1_PURE_PURSUIT_SPEED_PLANNING_FLASH_H_
#define _PLAN1_PURE_PURSUIT_SPEED_PLANNING_FLASH_H_

// Flash-backed route mode: record points locally, save to Flash, rebuild route on car.
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   0

// Corner Fillet interpolation, matching chazhi.py method 4.
#define NAV_FLASH_INTERPOLATE_DIST_MM       50.0f
#define NAV_FLASH_FILLET_RADIUS_SCALE       0.5f
#define NAV_FLASH_BEZIER_SAMPLES            20U
#define NAV_FLASH_MIN_RAW_POINTS            2U

// Offline speed planning parameters copied from chazhi.py.
#define NAV_FLASH_PATH_SPEED_MAX_MM_S       5000.0f
#define NAV_FLASH_MAX_ACCEL_MM_S2           2500.0f
#define NAV_FLASH_MAX_DECEL_MM_S2           1500.0f
#define NAV_FLASH_MAX_LATERAL_ACCEL_MM_S2   800.0f
#define NAV_FLASH_SPEED_TO_MM_S             4.936f
#define NAV_FLASH_CURVATURE_EPS             1.0e-6f

#define NAV_DIST_ARRIVE                 20.0f
#define NAV_START_HEADING_TOLERANCE      0.3f
#define NAV_SPEED_STOP                   0.0f
#define PP_LD_MIN_CURVE                500.0f
#define PP_LD_MIN_STRAIGHT              1.2f
#define PP_LD_SPEED_GAIN                0.7f
#define FILTER_ALPHA_ANGLE              0.45f
#define FILTER_ALPHA_SPEED              0.9f
#define SLEW_RATE_ANGLE                35.0f
#define NAV_SPEED_SLEW_EPS             1.0f
#define NAV_SPEED_SLEW_LOW_SPEED_TH    80.0f
#define NAV_SPEED_SLEW_FAST_DECEL_TH   220.0f
#define NAV_SPEED_SLEW_UP_LOW          30.0f
#define NAV_SPEED_SLEW_UP_NORMAL       45.0f
#define NAV_SPEED_SLEW_DOWN_NORMAL     65.0f
#define NAV_SPEED_SLEW_DOWN_FAST       95.0f
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO 120.0f
#define NAV_STOP_LOCK_DIST_MM          50.0f
#define NAV_STOP_LOCK_SPEED_EPS         1.0f

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);

// Loads raw points from Flash and rebuilds the interpolated speed-planned RAM route.
uint16 NavReplay_LoadStaticRouteToRam(void);

// Rebuilds the route currently stored in nav_ram_data using method 4.
uint16 NavReplay_BuildMethod4RouteFromRam(void);

#endif
