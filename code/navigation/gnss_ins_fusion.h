#ifndef __GNSS_INS_FUSION_H
#define __GNSS_INS_FUSION_H

#include "zf_common_headfile.h"

// ==================== 赛道基准角 (发车角) ====================
// 由手动锁定原点时从 IMU 磁力计读取，记录赛道的绝对地理朝向
extern float g_track_base_yaw;

// ==================== 融合状态结构体 ====================
typedef struct {
    float ins_x;      // 惯导内部独立积分 X (mm)
    float ins_y;      // 惯导内部独立积分 Y (mm)
    float ins_yaw;    // 惯导内部独立积分 Yaw (度)

    float offset_x;   // 零点偏移量 X (mm)，由 GPS 极慢速拉动
    float offset_y;   // 零点偏移量 Y (mm)
    float offset_yaw; // 零点偏移量 Yaw (度)，单天线不更新

    float fuse_x;     // 最终输出给纯追踪的平滑 X (mm)
    float fuse_y;     // 最终输出给纯追踪的平滑 Y (mm)
    float fuse_yaw;   // 最终输出给纯追踪的平滑 Yaw (度)
    
    // --- 状态监测 (供上位机遥测使用) ---
    float k_pos;                   // 当前互补滤波权重
    uint8_t jump_reject_count;     // 跃变剔除计数器
    uint8_t zupt_flag;             // 零速挂起标志
    
    // --- 发车角初始化状态 ---
    uint8_t heading_calculating;   // 1: 发车角计算中 (Mode2屏蔽GPS，Mode1采样中)
    uint16_t heading_sample_count; // Mode1: 采样帧数; Mode2: 已收到有效GPS帧数
    float heading_sum_cos;         // Mode1: cos(yaw)累加
    float heading_sum_sin;         // Mode1: sin(yaw)累加
    
    uint8_t special_element_flag;  // 特殊元素屏蔽标志
} FusionState_t;

extern FusionState_t g_fuse_state;

// ==================== API 接口 ====================

/**
 * @brief 融合模块初始化
 * @note  清零所有状态
 */
void Fusion_Init(void);

/**
 * @brief 100Hz 惯导推算任务 (10ms 调用一次)
 * @note  在 cm7_0_isr.c 中与 InertialNav_Update 同周期调用
 */
void Fusion_Ins_Update(void);

/**
 * @brief 10Hz GPS 纠偏任务 (每次 GNSS 数据到来时调用)
 * @note  含坐标旋转、杆臂补偿、跃变剔除、零速挂起、打滑权重转移
 *        单天线模式下不修正 yaw
 */
void Fusion_Gps_Correct(void);

/**
 * @brief 手动锁定原点 (上位机点击"清空轨迹"时调用)
 * @note  锁定当前 GPS 位置为原点，记录发车角，彻底清零惯导状态
 *        要求卫星数 >= 8 且经纬度有效
 */
void Fusion_Manual_Lock_Origin(void);

#endif // __GNSS_INS_FUSION_H
