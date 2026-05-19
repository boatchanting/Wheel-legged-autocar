#ifndef _NAV_PLAN_TEMPLATE_H_
#define _NAV_PLAN_TEMPLATE_H_
/* 模板头文件：仅保留统一 API 契约与状态外部暴露 */
extern NavReplayState_e g_replay_state;
extern uint8 g_current_point_type;
extern uint8 g_special_action_trigger;
void NavReplay_Start(void);
void NavReplay_Stop(void);
void NavReplay_Process(void);
#endif
