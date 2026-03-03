#ifndef _GNSS_REPLAY_H_
#define _GNSS_REPLAY_H_

#include "zf_common_headfile.h"
#include "gnss_ram.h"

// ========================= 控制参数宏定义 =========================
#define GNSS_DIST_FAR            0.30f   // 远距离界限 (单位: 米)
#define GNSS_DIST_NEAR           0.10f   // 近距离界限 (单位: 米)
#define GNSS_DIST_ARRIVE         0.1f   // 到达判定阈值 (单位: 米)
#define GNSS_YAW_TOLERANCE       2.0f    // 转向阈值，先转再走 (度)

// 速度设定 (负数为前进)
#define GNSS_SPEED_FAST          (-120.0f) 
#define GNSS_SPEED_SLOW          (-60.0f)  
#define GNSS_SPEED_STOP          (0.0f)

// ========================= 航向融合参数 =========================
// 融合低通滤波系数 (0.01 ~ 0.1 之间，越小越平滑但跟随越慢)
#define YAW_FUSION_KP            0.05f   

// ========================= 全局控制变量声明 =========================
extern volatile float target_speed_set;
extern volatile float err_degree;
extern uint8 minefield_flag; 

// ========================= 模块状态变量 =========================
typedef enum
{
    GNSS_REPLAY_IDLE,        // 停止/空闲
    GNSS_REPLAY_RUNNING,     // 正在跑图
    GNSS_REPLAY_FINISHED     // 完成所有点
} GnssReplayState_e;

extern GnssReplayState_e g_gnss_replay_state;   
extern uint8 g_gnss_current_point_type;         
extern uint8 g_gnss_special_action_trigger;     

// ========================= 函数接口 =========================
void GnssReplay_Start(void);
void GnssReplay_Stop(void);
void GnssReplay_Process(void);

// 获取当前融合后的绝对航向角 (用于调试观测)
float GnssReplay_GetFusedYaw(void);

#endif // _GNSS_REPLAY_H_