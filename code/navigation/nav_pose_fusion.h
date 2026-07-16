#ifndef __NAV_POSE_FUSION_H__
#define __NAV_POSE_FUSION_H__

#include "zf_common_headfile.h"
#include "../config/sys_options.h"

typedef struct {
    // 融合后的全局坐标 (mm)
    float fused_x_mm;
    float fused_y_mm;
    
    // 融合后的车身纵向速度 (mm/s)
    float fused_vx_body;
    
    // 锁定状态下的基准航向角 (degree)
    float heading0;
    uint8_t heading_lock;
    
    // 当前GPS动态权重
    float gps_weight;
    
    // GPS 数据是否有效
    uint8_t gps_valid;
    
    // 低频GPS速度残差（用于低通保持）
    float v_err_filtered;
} NavPoseFusion_t;

extern NavPoseFusion_t nav_pose_fusion;

// 发车时调用，用于初始化和锁定基准
void NavPoseFusion_StartLock(float current_x, float current_y);

// 在10ms中断等高频任务中调用，更新融合坐标
void NavPoseFusion_Update(float delta_t);

// 辅助函数：根据GPS更新权重（外部传入前瞻点的曲率等信息）
void NavPoseFusion_UpdateWeight(float curvature);

#endif // __NAV_POSE_FUSION_H__
