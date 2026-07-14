#ifndef _FUSION_NAV_H_
#define _FUSION_NAV_H_

#include "zf_common_headfile.h"
#include "inertial_nav.h"
#include "gnss_transform.h"

// 宏开关
#define ENABLE_SLIP_WEIGHT_SHIFT 0
#define GPS_DELAY_SEC 0.1f // 100ms GPS硬件延时假设

// 互补滤波状态结构体
typedef struct {
    float ins_x;      // 惯导内部独立积分 X (mm)
    float ins_y;      // 惯导内部独立积分 Y (mm)
    float ins_yaw;    // 惯导内部独立积分 Yaw (度)

    float offset_x;   // 零点偏移量 X (mm)，由 GPS 弹性修正
    float offset_y;   // 零点偏移量 Y (mm)

    float fuse_x;     // 输出给上层的平滑 X (mm)
    float fuse_y;     // 输出给上层的平滑 Y (mm)
    float fuse_yaw;   // 输出给上层的平滑 Yaw (度)
    
    float k_pos;      // 当前生效的互补滤波权重
} FusionState_t;

extern FusionState_t g_fuse_state;
extern float g_track_base_yaw;
extern float g_startup_avg_heading; // 上电时静置2秒采集的航向均值

void Fusion_Init(void);
void Fusion_Ins_Update(void);
void Fusion_Gps_Correct(void);
void Fusion_Set_Origin(void);

#endif // _FUSION_NAV_H_
