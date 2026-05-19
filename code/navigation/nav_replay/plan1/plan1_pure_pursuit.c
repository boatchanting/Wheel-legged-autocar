#include "../nav_replay.h"
#include "../../../common.h"
#include "../../nav_replay_route_table.h"
#if (CURRENT_NAV_PLAN == 1) && !(GNSS_NAV == 1 && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS))
extern volatile float target_speed_set; extern volatile float err_degree;
NavReplayState_e g_replay_state = REPLAY_IDLE; uint8 g_current_point_type = NAV_POINT_PATH; uint8 g_special_action_trigger = 0;
static uint16 g_target_idx = 0;
static float NormalizeAngle(float a){while(a>180.0f)a-=360.0f;while(a<-180.0f)a+=360.0f;return a;} static float CalcDistance(float x1,float y1,float x2,float y2){float dx=x2-x1,dy=y2-y1;return sqrtf(dx*dx+dy*dy);} 
static uint16 NavReplay_LoadStaticRouteToRam(void){uint16 i,c=NAV_REPLAY_STATIC_ROUTE_COUNT; if(c>NAV_RAM_MAX_POINTS)c=NAV_RAM_MAX_POINTS; nav_ram_data.plan_type=(uint8)CURRENT_NAV_PLAN; nav_ram_data.point_count=c; for(i=0;i<c;i++) nav_ram_data.points[i]=nav_replay_static_route_points[i]; return c;}
void NavReplay_Start(void){NavReplay_LoadStaticRouteToRam(); g_target_idx=0; g_replay_state=REPLAY_RUNNING; g_special_action_trigger=0;}
void NavReplay_Stop(void){target_speed_set=0.0f; err_degree=0.0f; g_replay_state=REPLAY_IDLE;}
void NavReplay_Process(void){ if(g_replay_state!=REPLAY_RUNNING||g_special_action_trigger) return; if(g_target_idx>=nav_ram_data.point_count){g_replay_state=REPLAY_FINISHED;target_speed_set=0;err_degree=0;return;} float tx=nav_ram_data.points[g_target_idx].x,ty=nav_ram_data.points[g_target_idx].y; float d=CalcDistance(inertial_nav.x,inertial_nav.y,tx,ty); float tyaw=-atan2f(ty-inertial_nav.y,-(tx-inertial_nav.x))*57.29578f; err_degree=NormalizeAngle(tyaw-inertial_nav.relative_yaw); target_speed_set=nav_ram_data.points[g_target_idx].target_speed; g_current_point_type=nav_ram_data.points[g_target_idx].point_type; if(d<20.0f) g_target_idx++; }
#endif
