#include "../nav_replay.h"
#if 0
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;
static float NormalizeAngle(float a){ while(a>180.0f)a-=360.0f; while(a<-180.0f)a+=360.0f; return a; }
static float CalcDistance(float x1,float y1,float x2,float y2){ float dx=x2-x1,dy=y2-y1; return sqrtf(dx*dx+dy*dy);} 
void NavReplay_Start(void){ g_replay_state = REPLAY_RUNNING; g_special_action_trigger = 0; }
void NavReplay_Stop(void){ g_replay_state = REPLAY_IDLE; g_special_action_trigger = 0; }
void NavReplay_Process(void){ (void)NormalizeAngle(0.0f); (void)CalcDistance(0,0,0,0); }
#endif
