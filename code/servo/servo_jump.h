#ifndef _SERVO_JUMP_H_
#define _SERVO_JUMP_H_

#include "zf_common_headfile.h"

// ===================== 跳跃参数调优 =====================
// 注意：这里的值都是相对于当前身高的“偏移量”
// 正数代表“想让腿更长（伸腿）”，负数代表“想让腿更短（收腿）”
// 代码内部会自动根据 DIR 宏来处理 PWM 到底是加还是减

#define JUMP_OFFSET_LAUNCH    2500   // 起跳爆发力 (数值越大跳得越高)
#define JUMP_OFFSET_FLIGHT    -1800  // 空中收腿幅度 (数值越小腿收得越紧，防止绊倒)
#define JUMP_OFFSET_LAND      800    // 落地前探幅度 (提前接触地面)

// ===================== 状态控制 =====================
extern uint8_t jump_flag;          // 0:空闲, 1:跳跃中
extern uint32_t jump_start_time;   // 记录起跳时的 loop_counter
extern uint32_t loop_counter;      // 引用外部的1ms计数器
extern bool vision_detected_jump_point; // 视觉检测到的跳跃点，测试用
typedef enum {
    JUMP_PHASE_NONE,        // 0: 不在跳跃状态
    JUMP_PHASE_LAUNCH,      // 1: 阶段 A, 爆发起跳
    JUMP_PHASE_FLIGHT,      // 2: 阶段 B, 空中飞行 (动量轮作用)
    JUMP_PHASE_LANDING,     // 3: 阶段 C, 落地准备
    JUMP_PHASE_RECOVERY     // 4: 阶段 D, 缓冲恢复
} JumpPhase;
extern volatile JumpPhase g_current_jump_phase;//跳跃阶段的变量
// 跳跃阶段枚举

// ===================== 函数声明 =====================
void jump_module_init(void);
void jump_trigger(void);           // 触发跳跃
void servo_jump_executor(void);    // 周期性调用执行函数
/**
 * @brief 初始化空中姿态控制参数
 */
void Momentum_Wheel_Control_Init(void);
/**
 * @brief 动量轮姿态控制核心算法 (在空中运行时调用)
 * @param current_pitch 当前俯仰角 (来自 IMU)
 * @param current_gyro  当前俯仰角速度 (来自 IMU)
 * @return int16_t       计算出的电机PWM值
 */
int16_t Momentum_Wheel_Control_Run(float current_pitch, float current_gyro);

#endif