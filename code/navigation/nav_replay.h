#ifndef _NAV_REPLAY_H_
#define _NAV_REPLAY_H_

#include "zf_common_headfile.h"
#include "nav_ram.h"
#include "../config/sys_options.h"

// 1: use compile-time route table generated from CSV (no flash dependency)
#define NAV_REPLAY_USE_STATIC_ROUTE_TABLE   1

#if CURRENT_NAV_PLAN == 1 || CURRENT_NAV_PLAN == 2
#define NAV_DIST_ARRIVE                 20.0f
#define NAV_START_HEADING_TOLERANCE      0.3f
#define NAV_SPEED_STOP                   0.0f
#define PP_LD_MIN_CURVE                500.0f
#define PP_LD_MIN_STRAIGHT              1.2f
#define PP_LD_SPEED_GAIN                0.7f
#define FILTER_ALPHA_ANGLE              0.45f
#define FILTER_ALPHA_SPEED              0.9f
#define SLEW_RATE_ANGLE                35.0f
#define NAV_STOP_LOCK_DIST_MM          50.0f
#define NAV_STOP_LOCK_SPEED_EPS         1.0f
#endif

#if CURRENT_NAV_PLAN == 3
#define NAV_DIST_FAR                  500.0f
#define NAV_DIST_NEAR                 150.0f
#define NAV_DIST_ARRIVE                20.0f
#define NAV_YAW_TOLERANCE               1.0f
#define NAV_START_HEADING_TOLERANCE     0.3f
#define NAV_SPEED_FAST               (-140.0f)
#define NAV_SPEED_SLOW                (-40.0f)
#define NAV_SPEED_STOP                  0.0f
#define PP_LD_MIN_CURVE              500.0f
#define PP_LD_MIN_STRAIGHT             1.2f
#define PP_LD_SPEED_GAIN               0.7f
#define CURVE_PREVIEW_DIST          1200.0f
#define SPD_CURVE_DEADZONE             0.35f
#define SPD_CURVE_EXPONENT             3.5f
#define SPD_ANGLE_PENALTY              0.08f
#define SPD_ANGLE_TOLERANCE           60.0f
#define FILTER_ALPHA_ANGLE             0.45f
#define FILTER_ALPHA_SPEED             1.0f
#define SLEW_RATE_ANGLE               35.0f
#endif

extern volatile float target_speed_set;
extern volatile float err_degree;
extern volatile float roll_degree;

typedef enum
{
    REPLAY_IDLE,
    REPLAY_RUNNING,
    REPLAY_FINISHED
} NavReplayState_e;

extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
uint16 NavReplay_LoadStaticRouteToRam(void);

#endif
