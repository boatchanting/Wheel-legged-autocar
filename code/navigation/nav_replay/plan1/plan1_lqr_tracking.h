#ifndef _PLAN1_LQR_TRACKING_H_
#define _PLAN1_LQR_TRACKING_H_

/*
 * Plan1 local-projection LQR-style tracking.
 *
 * target_speed_set: chassis speed command from route target_speed, with slew.
 * err_degree: steering/yaw command = local feedback + curvature/yaw-rate feedforward.
 *
 * Control structure:
 *   u = u_ff(kappa_preview, speed)
 *     + K_y * e_y
 *     + K_psi * e_psi
 *     + K_r * (r_ref - r_actual)
 *
 * e_y/e_psi are calculated from the closest local path segment projection.
 * Preview is used only for curvature feedforward. Negative target_speed means forward.
 */

#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

#define NAV_DIST_ARRIVE                    20.0f
#define NAV_START_HEADING_TOLERANCE         0.3f
#define NAV_SPEED_STOP                      0.0f

/* Curvature preview only. Feedback never uses preview point yaw. */
#define LQR_PREVIEW_POINTS                  3U
#define LQR_SHARP_CURVATURE_TH              0.0010f
#define LQR_SHARP_PREVIEW_POINTS            4U

#define LQR_SEARCH_RANGE_NORMAL            80U
#define LQR_SEARCH_RANGE_RECOVER          300U
#define LQR_MAX_TRACK_DIST_MM             800.0f
#define LQR_PROJECTION_MIN_SEG_LEN_MM       1.0f

/* Global steering sign. Flip this first if real-car left/right is reversed. */
#define LQR_SIGN                            1.0f

/* Feedback and feedforward gains. */
#define LQR_K_YAW_RATE_FF                   8.0f
#define LQR_K_LATERAL                       0.025f
#define LQR_K_HEADING                       0.75f

/*
 * Optional yaw-rate feedback. Default is off: inertial_nav.actual_yaw_rate exists,
 * but enable this only after sensor sign/noise are verified on the real car.
 */
#define LQR_USE_ACTUAL_YAW_RATE_FB          0
#define LQR_K_YAW_RATE                      0.0f

#define LQR_CURVATURE_SPEED_SIGN_ENABLE     1
#define LQR_FORWARD_SPEED_IS_NEGATIVE       1
#define LQR_SPEED_TO_MM_S                   4.936f

/* Low speed keeps authority; high speed tightens stability envelopes. */
#define LQR_LOW_SPEED_MM_S                500.0f
#define LQR_HIGH_SPEED_MM_S              2500.0f
#define LQR_LOW_SPEED_ERR_MAX_DEG          45.0f
#define LQR_HIGH_SPEED_ERR_MAX_DEG         28.0f
#define LQR_LOW_SPEED_ERR_SLEW_DEG         24.0f
#define LQR_HIGH_SPEED_ERR_SLEW_DEG        10.0f
#define LQR_LOW_SPEED_FILTER_ALPHA          0.80f
#define LQR_HIGH_SPEED_FILTER_ALPHA         0.45f
#define LQR_LOW_SPEED_YAW_RATE_LIMIT_RAD_S  3.20f
#define LQR_HIGH_SPEED_YAW_RATE_LIMIT_RAD_S 2.80f
#define LQR_LOW_SPEED_YAW_ACCEL_LIMIT_RAD_S2 80.0f
#define LQR_HIGH_SPEED_YAW_ACCEL_LIMIT_RAD_S2 35.0f
#define LQR_LOW_SPEED_LATERAL_ACCEL_MM_S2  4200.0f
#define LQR_HIGH_SPEED_LATERAL_ACCEL_MM_S2 3800.0f
#define LQR_CONTROL_PERIOD_S                0.01f
#define LQR_OUTPUT_DEG_PER_RADPS           30.0f
#define LQR_LATERAL_ERR_LIMIT_MM          600.0f

/* Route speed command slew, kept compatible with the existing replay style. */
#define NAV_SPEED_SLEW_EPS                  50.0f
#define NAV_SPEED_SLEW_LOW_SPEED_TH        80.0f
#define NAV_SPEED_SLEW_FAST_DECEL_TH      220.0f
#define NAV_SPEED_SLEW_UP_LOW              30.0f
#define NAV_SPEED_SLEW_UP_NORMAL           70.0f
#define NAV_SPEED_SLEW_DOWN_NORMAL        400.0f
#define NAV_SPEED_SLEW_DOWN_FAST          800.0f
#define NAV_SPEED_SLEW_DOWN_CROSS_ZERO    120.0f
#define NAV_STOP_LOCK_SPEED_EPS             1.0f

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

extern NavReplayState_e g_replay_state;
extern uint16 g_target_idx;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
extern uint8 g_plan1_fast_uturn_state;
extern uint8 g_plan1_fast_uturn_lead;
extern NavReplayState_e g_gps_replay_state;
extern uint8 g_gps_current_point_type;
extern uint8 g_gps_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
