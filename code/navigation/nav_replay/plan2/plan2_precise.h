#ifndef _PLAN2_PRECISE_H_
#define _PLAN2_PRECISE_H_

#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

#define NAV_DIST_FAR                        300.0f
#define NAV_DIST_NEAR                       150.0f
#define NAV_DIST_ARRIVE                     20.0f
#define NAV_SPECIAL_TRIGGER_RADIUS          300.0f
#define NAV_SPECIAL_APPROACH_DIST           800.0f
#define NAV_SPECIAL_RELAX_APPROACH_WINDOW   420.0f
#define NAV_SPECIAL_STOP_PREDICT_TIME       0.35f
#define NAV_SPECIAL_PASS_AWAY_MARGIN        35.0f
#define NAV_YAW_TOLERANCE                   1.0f
#define NAV_REVERSE_SELECT_BIAS_DEG         8.0f
#define NAV_SPIN_MIN_TOTAL_ANGLE            370.0f
#define NAV_START_HEADING_TOLERANCE         0.3f

#define NAV_SPEED_FAST                      (-300.0f)
#define NAV_SPEED_SLOW                      (-100.0f)
#define NAV_SPEED_STOP                      (0.0f)

extern volatile float target_speed_set;
extern volatile float err_degree;

extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
