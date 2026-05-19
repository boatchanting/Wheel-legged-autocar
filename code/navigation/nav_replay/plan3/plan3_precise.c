#include "../../nav_replay.h"
#include "../../nav_ram.h"
#include "../../../common.h"
#include "vision/vision_bridge_control.h"

#if (CURRENT_NAV_PLAN == 3)
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = NAV_POINT_PATH;
uint8 g_special_action_trigger = 0;
static uint16 g_target_idx = 0;
static float NormalizeAngle(float a){while(a>180)a-=360;while(a<-180)a+=360;return a;}
static float CalcDistance(float x1,float y1,float x2,float y2){float dx=x2-x1,dy=y2-y1;return sqrtf(dx*dx+dy*dy);} 
void NavReplay_Start(void){g_target_idx=0;g_replay_state=REPLAY_RUNNING;g_special_action_trigger=0;}
void NavReplay_Stop(void){target_speed_set=0;err_degree=0;g_replay_state=REPLAY_IDLE;}
void NavReplay_Process(void){if(g_replay_state!=REPLAY_RUNNING||g_special_action_trigger)return; if(g_target_idx>=nav_ram_data.point_count){g_replay_state=REPLAY_FINISHED;target_speed_set=0;err_degree=0;return;} float tx=nav_ram_data.points[g_target_idx].x,ty=nav_ram_data.points[g_target_idx].y; float d=CalcDistance(inertial_nav.x,inertial_nav.y,tx,ty); float yaw=-atan2f(ty-inertial_nav.y,-(tx-inertial_nav.x))*57.29578f; err_degree=NormalizeAngle(yaw-inertial_nav.relative_yaw); g_current_point_type=nav_ram_data.points[g_target_idx].point_type; target_speed_set=(d>500.0f)?-140.0f:-40.0f; if(d<=20.0f){target_speed_set=0;if(g_current_point_type==NAV_POINT_CIRCLE)minefield_flag=1;else if(g_current_point_type==NAV_POINT_JUMP)vision_detected_three_jump_point=1;else if(g_current_point_type==NAV_POINT_BRIDGE)VisionBridgeTask_Start();else if(g_current_point_type==NAV_POINT_BUMP)BumpyRoad_Trigger(); if(g_current_point_type!=NAV_POINT_PATH)g_special_action_trigger=1; g_target_idx++;}}
#endif
