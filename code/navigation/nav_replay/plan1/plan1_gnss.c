#include "../nav_replay.h"
#include "../../common.h"
#include "../gps_nav_replay_route_table.h"
#include "../gnss_transform.h"
#if (CURRENT_NAV_PLAN == 1) && (GNSS_NAV == 1) && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
extern volatile float target_speed_set; extern volatile float err_degree;
NavReplayState_e g_replay_state = REPLAY_IDLE; uint8 g_current_point_type = NAV_POINT_PATH; uint8 g_special_action_trigger = 0;
static uint16 g_target_idx = 0;
static float NormalizeAngle(float a){while(a>180.0f)a-=360.0f;while(a<-180.0f)a+=360.0f;return a;} static float CalcDistance(float x1,float y1,float x2,float y2){float dx=x2-x1,dy=y2-y1;return sqrtf(dx*dx+dy*dy);} 
static uint16 NavReplay_LoadStaticRouteToRam(void){uint16 i,c=GPS_NAV_REPLAY_STATIC_ROUTE_COUNT; if(c>NAV_RAM_MAX_POINTS)c=NAV_RAM_MAX_POINTS; nav_ram_data.plan_type=1; nav_ram_data.point_count=c; for(i=0;i<c;i++) nav_ram_data.points[i]=gps_nav_replay_static_route_points[i]; return c;}
void NavReplay_Start(void){NavReplay_LoadStaticRouteToRam(); g_target_idx=0; g_replay_state=REPLAY_RUNNING; g_special_action_trigger=0;}
void NavReplay_Stop(void){target_speed_set=0.0f; err_degree=0.0f; g_replay_state=REPLAY_IDLE;}
void NavReplay_Process(void){ if(g_replay_state!=REPLAY_RUNNING||g_special_action_trigger) return; if(g_target_idx>=nav_ram_data.point_count){g_replay_state=REPLAY_FINISHED;target_speed_set=0;err_degree=0;return;} float cx=gnss_trans.x*1000.0f,cy=gnss_trans.y*1000.0f; float tx=nav_ram_data.points[g_target_idx].x,ty=nav_ram_data.points[g_target_idx].y; float d=CalcDistance(cx,cy,tx,ty); float tyaw=atan2f(ty-cy,tx-cx)*57.29578f; err_degree=NormalizeAngle(tyaw-euler_angle.yaw); target_speed_set=(d>2200.0f)?-600.0f:-80.0f; g_current_point_type=nav_ram_data.points[g_target_idx].point_type; if(d<500.0f) g_target_idx++; }
#endif
