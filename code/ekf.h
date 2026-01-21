#include "zf_common_headfile.h"
#ifndef CODE_EKF_H_
#define CODE_EKF_H_

// 角度转弧度转换系数 (180/π)
#define DEG_TO_RAD      (57.295779513082320876798154814105f)
// 采样时间间隔 (秒)
#define dt              (0.005f)
// 低通滤波系数 (0-1之间，值越大滤波效果越弱)
#define K               (0.9f)


/**
 * @brief IMU数据结构体
 * @note 包含陀螺仪和加速度计的三轴数据
 */
typedef struct
{
        float gyro_x;   // 陀螺仪X轴角速度 (弧度/秒)
        float gyro_y;   // 陀螺仪Y轴角速度 (弧度/秒)
        float gyro_z;   // 陀螺仪Z轴角速度 (弧度/秒)
        float acc_x;    // 加速度计X轴加速度 (g)
        float acc_y;    // 加速度计Y轴加速度 (g)
        float acc_z;    // 加速度计Z轴加速度 (g)
}imu_t;

/**
 * @brief 扩展卡尔曼滤波器初始化函数
 * @note 初始化EKF的状态变量和协方差矩阵
 */
void EKF_Init(void);

/**
 * @brief 扩展卡尔曼滤波器更新函数
 * @note 执行EKF预测和更新步骤，更新姿态估计
 */
void EKF_UpData(void);


#endif /* CODE_EKF_H_ */
