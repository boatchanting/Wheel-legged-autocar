#include "zf_common_headfile.h"

// 防止头文件重复包含
#ifndef CODE_EKF_H_
#define CODE_EKF_H_

// 角度转弧度转换系数 (180/π的倒数)
#define DEG_TO_RAD      (57.295779513082320876798154814105f)
// EKF更新时间间隔(秒)，200Hz采样率对应0.005秒
#define dt              (0.005f)
// 一阶低通滤波系数，用于加速度计数据滤波
#define K               (0.9f)

// IMU数据结构体定义
typedef struct
{
    // 陀螺仪X轴角速度(rad/s)
    float gyro_x;
    // 陀螺仪Y轴角速度(rad/s)
    float gyro_y;
    // 陀螺仪Z轴角速度(rad/s)
    float gyro_z;
    // 加速度计X轴加速度(g)
    float acc_x;
    // 加速度计Y轴加速度(g)
    float acc_y;
    // 加速度计Z轴加速度(g)
    float acc_z;
}imu_t;

// EKF初始化函数声明
void EKF_Init(void);

// EKF更新函数声明
void EKF_UpData(void);

#endif /* CODE_EKF_H_ */
