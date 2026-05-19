#include "../../nav_replay.h"
#include "../../nav_ram.h"
#include "../../gps_nav_replay_route_table.h"
#include "../../gnss_transform.h"
#include "../../../common.h"

#if (CURRENT_NAV_PLAN == 1) && (NAV_PLAN1_METHOD == PLAN1_METHOD_GNSS)
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;
static uint16 g_target_idx = 0;
static float NormalizeAngle(float a){while(a>180)a-=360;while(a<-180)a+=360;return a;}
static float CalcDistance(float x1,float y1,float x2,float y2){float dx=x2-x1,dy=y2-y1;return sqrtf(dx*dx+dy*dy);} 
static float Bearing(float fx,float fy,float tx,float ty){return atan2f(ty-fy,tx-fx)*57.2957795f;}
void NavReplay_Start(void){g_target_idx=0;g_replay_state=REPLAY_RUNNING;g_special_action_trigger=0;}
void NavReplay_Stop(void){target_speed_set=0;err_degree=0;g_replay_state=REPLAY_IDLE;}
void NavReplay_Process(void){if(g_replay_state!=REPLAY_RUNNING)return; float cx=gnss_trans.x*1000.0f,cy=gnss_trans.y*1000.0f; if(g_target_idx>=nav_ram_data.point_count){g_replay_state=REPLAY_FINISHED;target_speed_set=0;err_degree=0;return;} float tx=nav_ram_data.points[g_target_idx].x,ty=nav_ram_data.points[g_target_idx].y; float d=CalcDistance(cx,cy,tx,ty); err_degree=NormalizeAngle(Bearing(cx,cy,tx,ty)-euler_angle.yaw); target_speed_set=(d>2200.0f)?-600.0f:-80.0f; if(d<500.0f)g_target_idx++;}
#endif
