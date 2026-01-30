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
// ===================== 函数声明 =====================
void jump_module_init(void);
void jump_trigger(void);           // 触发跳跃
void servo_jump_executor(void);    // 周期性调用执行函数

#endif