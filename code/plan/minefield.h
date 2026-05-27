#ifndef _MINEFIELD_H_
#define _MINEFIELD_H_

#include "zf_common_headfile.h"

// 导出触发标志位，外部逻辑（如图像处理/gps信号）将此置1触发动作
extern volatile uint8_t minefield_flag;
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
 * @param current_yaw_deg 当前的总偏航角 (用于结束时重置目标)
 * @param target_yaw_ptr  指向全局目标偏航角变量的指针 (用于修改 g_initial_yaw)
 * 
 * @return float          计算出的目标旋转角速度 (单位: °/s)，若未激活则返回 0.0f
 * 
 * @note 此函数内部实现了梯形速度规划（加速-匀速-减速），并自动处理标志位复位。
 */
float Minefield_Spin_Controller(float gyro_z_deg, float dt_s, float current_yaw_deg,volatile float* target_yaw_ptr);

#endif
