#ifndef _NAVIGATION_EKF_H_
#define _NAVIGATION_EKF_H_
#include "zf_common_headfile.h"
#define WHEEL_CIRCUMFERENCE 1.00943f // 车轮周长，单位米，根据实际车轮调整   Πd  10圈显示1.06大概214cm  待调整
#define pitch_initialization 0.83176f  //倒地时俯仰角为37.82，机械零点目前为ANG_MECH_ZERO4.1f    cos（37.82°-4.1°）  
// ==========================================
// 1. 参数配置区
// ==========================================
// 控制周期 (秒)，务必与你的调用频率一致 (例如 5ms = 0.005)
#define NAV_DT          0.005f  

// 状态量维度 (X, Y, Vx, Vy)
#define NAV_STATE_DIM   4       
// 观测量维度 (Venc_x, Venc_y)
#define NAV_MEAS_DIM    2       
// 输入量维度 (Acc_x, Acc_y)
#define NAV_INPUT_DIM   2       

// ==========================================
// 2. 数据结构定义
// ==========================================
typedef struct {
    float pos_x;    // [输出] 世界坐标系位置 X (米)
    float pos_y;    // [输出] 世界坐标系位置 Y (米)
    float vel_x;    // [输出] 世界坐标系速度 Vx (m/s)
    float vel_y;    // [输出] 世界坐标系速度 Vy (m/s)
    
    // 用于调试的内部状态
    float acc_world_x; // 转换后的世界系加速度
    float acc_world_y;
} nav_state_t;


extern nav_state_t nav_result; // 全局结果变量

// ==========================================
// 3. 函数声明
// ==========================================

// 初始化惯导卡尔曼滤波器
void Navigation_EKF_Init(void);

// 惯导更新函数 (建议每 NAV_DT 调用一次)
// 输入: 
//   imu_ax_mpss, imu_ay_mpss: IMU去除重力和零偏后的加速度 (单位: m/s^2)这里传入imu数据即可，单位转换在函数内执行
//   yaw_rad: 当前的偏航角 (单位: 角度)
//   enc_vel_mps: 编码器测得的车体线速度 (单位: m/s)传入编码器计算所得速度即可
void Navigation_EKF_Update(float imu_ax_mpss, float imu_ay_mpss, float yaw_rad, float enc_vel_mps);

// 重置位置 (比如回到起跑线时调用)
void Navigation_Reset(void);//写在外部中断里

#endif
