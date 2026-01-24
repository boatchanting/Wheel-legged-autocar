#ifndef CODE__PID_NEW_H__
#define CODE__PID_NEW_H__

#include "zf_common_headfile.h"
// ============================================================================
// 1. PID 参数结构体
// ============================================================================
typedef struct {
    // --- 调节参数 ---
    float kp;               
    float ki;               
    float kd;               

    // --- 限幅参数 ---
    float max_output;       
    float max_integral;     

    // --- 补偿参数 (零点/死区) ---
    float compensation;     

    // --- 运行时变量 (Runtime Variables) ---
    float error;            // e(k)   : 当前误差
    float last_error;       // e(k-1) : 上一次误差
    float prev_error;       // e(k-2) : [新增] 上上次误差 (预留给增量计算)
    
    float error_integral;   // 积分累加
    float output;           // 最终输出
} PID_Param_t;

// ============================================================================
// 2. 全局声明
// ============================================================================
extern PID_Param_t pid_speed;
extern PID_Param_t pid_angle;
extern PID_Param_t pid_gyro;
extern volatile float now_speed;        // 当前速度 (来自编码器)
extern volatile float now_angle;        // 当前角度 (来自IMU)
extern volatile float now_gyro;         // 当前角速度 (来自IMU)

extern float speed_loop_out;    // 速度环的输出 (目标角度)
extern float angle_loop_out;    // 角度环的输出 (目标角速度)
extern float gyro_loop_out;     // 角速度环的输出 (目标角加速度)

extern volatile float final_motor_pwm;  // 最终输出到电机的PWM值

extern float target_speed_set;

void PID_Param_Init(void);//pid参数初始化
void PID_Data_Reset(void);//pid参数全清空，用于倒地保护
float Float_Constrain(float val, float min, float max);

float Speed_Loop_Control(float target_speed, float actual_speed);//速度环(外环)
float Angle_Loop_Control(float speed_loop_output, float actual_angle);//角度环(中环)
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro);//角速度环(内环)

#endif