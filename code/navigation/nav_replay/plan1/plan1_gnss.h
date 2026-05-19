#ifndef _PLAN1_GNSS_H_
#define _PLAN1_GNSS_H_
extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
#endif
