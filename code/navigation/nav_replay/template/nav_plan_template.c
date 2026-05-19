#include "../../nav_replay.h"

#if 1
/*
 * 模板说明：
 * 1) 每个算法方案都必须维护自己的全局状态变量；
 * 2) 每个算法方案都必须实现统一 API：Start/Stop/Process；
 * 3) 每个算法方案都可以在本文件内定义私有数学工具函数（static）。
 */
NavReplayState_e g_replay_state = REPLAY_IDLE;
uint8 g_current_point_type = 0;
uint8 g_special_action_trigger = 0;

static float Template_NormalizeAngle(float angle)
{
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

void NavReplay_Start(void) { (void)Template_NormalizeAngle(0.0f); }
void NavReplay_Stop(void) {}
void NavReplay_Process(void) {}
#endif
