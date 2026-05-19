#ifndef _NAV_PLAN_TEMPLATE_H_
#define _NAV_PLAN_TEMPLATE_H_

#include "zf_common_headfile.h"

extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;

void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);

#endif
