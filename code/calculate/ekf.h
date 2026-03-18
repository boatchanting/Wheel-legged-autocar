#ifndef CODE_EKF_H_
#define CODE_EKF_H_
#include "zf_common_headfile.h"
#include "../config/sys_options.h"
// 角度转弧度转换系数 (180/π)
#define DEG_TO_RAD      (57.295779513082320876798154814105f)
// 采样时间间隔 (秒)
#define dt              (0.005f)
// 低通滤波系数 (0-1之间，值越大滤波效果越弱)
#define K               (0.9f)
extern volatile float g_initial_yaw;         // 存储记录下来的初始偏航角
extern volatile bool  g_yaw_initialized;     // 偏航角是否已成功初始化的标志

// --- 函数原型声明 ---
/**
 * @brief  检查车模是否稳定，如果稳定则记录初始偏航角作为零点 (优化版)
 * @param  current_tick 当前的中断计数值 (来自 loop_counter)
 * @retval None
 * @note   此函数应在获取到最新欧拉角后被调用
 */
void record_initial_yaw_task(uint32_t current_tick);



// 加速度静态偏移量变量
extern float imu660ra_acc_x_AND;//【优化点】暂时未使用
extern float imu660ra_acc_y_AND;//暂时未使用
extern float imu660ra_acc_z_AND;//暂时未使用
extern  volatile float g_initial_yaw;         // 存储记录下来的初始偏航角
extern  volatile bool  g_yaw_initialized;     // 偏航角是否已成功初始化的标志

// --- 函数原型声明 ---
/**
 * @brief  检查车模是否稳定，如果稳定则记录初始偏航角作为零点 (优化版)
 * @param  current_tick 当前的中断计数值 (来自 loop_counter)
 * @retval None
 * @note   此函数应在获取到最新欧拉角后被调用
 */
void record_initial_yaw_task(uint32_t current_tick);
extern void IMU_Calibrate_All_Gyro(void); // 校准陀螺仪
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
        // --- 新增：估计的重力分量 (单位向量, 1g) ---
        float grav_x; 
        float grav_y; 
        float grav_z;
}imu_t;
extern imu_t imu_data;
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

#if IMU_CATEGORY == 3 // IMU963RA的磁力计模块
extern volatile float heading;
void EKF_Update_Heading(void);
void mag_calibrate(float raw_x, float raw_y, float raw_z, float *mag_x, float *mag_y, float *mag_z);
// 声明校准后的磁力计全局变量，允许其他文件访问
extern float mag_x;
extern float mag_y;
extern float mag_z;
#endif

#endif /* CODE_EKF_H_ */
