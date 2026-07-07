#ifndef _MINEFIELD_H_
#define _MINEFIELD_H_

#include "zf_common_headfile.h"

// 导出触发标志位，外部逻辑（如图像处理/gps信号）将此置1触发动作
extern volatile uint8_t minefield_flag;
// 雷区转圈规则：720 度是基础圈数，额外 5 度作为硬性安全余量。
#define MINEFIELD_SPIN_BASE_CIRCLE_ANGLE          720.0f
#define MINEFIELD_SPIN_RESERVE_ANGLE              5.0f
#define MINEFIELD_SPIN_MIN_TOTAL_ANGLE            (MINEFIELD_SPIN_BASE_CIRCLE_ANGLE + MINEFIELD_SPIN_RESERVE_ANGLE)
// 满足最小转圈角后，出口航向在该误差内即可释放，剩余偏差由导航边跑边修。
#define MINEFIELD_SPIN_EXIT_RELEASE_YAW_TOLERANCE 35.0f
extern uint8 vision_detected_marker;//雷区调用,测试用
/**
 * @brief 初始化/复位旋转控制的相关变量
 */
void Minefield_Init(void);
void Minefield_SetSpinPlan(float total_spin_deg, float exit_yaw_deg, float spin_speed_sign);

/**
 * @brief 判断当前是否处于旋转动作执行中
 * @return 1: 正在旋转, 0: 空闲/正常行驶
 */
uint8_t Minefield_Is_Active(void);

/**
 * @brief 旋转动作核心控制函数 (需在2ms Gyro环中调用，当前Core0调度1ms)
 * 
 * @param gyro_z_deg      当前Z轴角速度 (单位: °/s)
 * @param dt_s            调用周期时间 (单位: 秒，通常为 0.002f，当前Core0转向角速度环为0.001f)
 * @param current_yaw_deg 当前偏航角；保留参数用于兼容旧调用，新逻辑不再用它慢速补角
 * @param target_yaw_ptr  目标偏航角指针；保留参数用于兼容旧调用，新逻辑不再修改 g_initial_yaw
 * 
 * @return float          计算出的目标旋转角速度 (单位: °/s)，若未激活则返回 0.0f
 * 
 * @note 此函数内部实现梯形速度规划；满足 725 度后会用出口航向判断是否提前释放。
 */
float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg,volatile float* target_yaw_ptr);

#endif
