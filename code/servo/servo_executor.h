#ifndef __SERVO_EXECUTOR_H__
#define __SERVO_EXECUTOR_H__

#include "zf_common_headfile.h"

// --- 规划器(20ms)需要更新的目标值 ---
// 使用 'volatile' 关键字确保编译器不会优化掉对这些变量的读写，
// 因为它们在主程序和中断之间共享。
extern volatile int16 g_target_pwm_high;      // 目标高度分量
extern volatile int16 g_target_pwm_speed_adj; // 目标速度调整分量
extern volatile int16 g_target_pwm_angle_adj; // 目标转向/姿态调整分量
extern int32 current_duty_lf, current_duty_rf, current_duty_rr, current_duty_lr; // 当前实际占空比 (用于斜率限制计算)
/**
 * @brief 初始化舵机执行器的内部状态
 */
void servo_executor_init(void);

/**
 * @brief 舵机执行器更新函数 (应在1ms中断中调用)
 * @note  此函数负责平滑地将舵机驱动到目标位置
 */
void servo_executor_update(void);

#endif