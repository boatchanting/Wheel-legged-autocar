#ifndef CODE__PID_NEW_H__
#define CODE__PID_NEW_H__

#include "zf_common_headfile.h" // 引用你的工程公共头文件

// ==========================================
// 1. 宏定义 (参数限幅与常量)
// ==========================================

// 输出限幅
#define SPEED_OUT_LIMIT  8.0f     // 速度环输出限幅（最大倾斜角度，单位：度）
#define ANGLE_OUT_LIMIT  3000.0f  // 角度环输出限幅（最大期望角速度，单位：度/秒 或 LSB）
#define PWM_MAX_LIMIT    9000     // 电机PWM最大值 (根据你的定时器ARR决定，通常是10000或7200)

// 积分限幅
#define SPEED_INT_MAX    2000.0f  // 速度环积分限幅

// 采样分频 (用于中断逻辑)
#define SPEED_LOOP_DIV   20       // 速度环分频系数 (如 1ms中断，20代表20ms执行一次)
#define ANGLE_LOOP_DIV   5        // 角度环分频系数 (如 1ms中断，5代表5ms执行一次)

// ==========================================
// 2. 数据类型定义
// ==========================================

// PID参数结构体
typedef struct {
    float kp;               // 比例系数
    float ki;               // 积分系数
    float kd;               // 微分系数
    
    float error;            // 当前误差
    float last_error;       // 上次误差
    float prev_error;       // 上上次误差 (用于增量式)
    
    float error_integral;   // 误差积分累加值
    float output;           // PID计算输出结果
} PID_Param_t;

// ==========================================
// 3. 全局变量声明 (extern)
// ==========================================
// 使用 extern 关键字，以便在 main.c 中可以调节这些参数

extern PID_Param_t pid_speed;   // 速度环参数对象
extern PID_Param_t pid_angle;   // 角度环参数对象
extern PID_Param_t pid_gyro;    // 角速度环参数对象

extern float mechanical_zero_angle; // 机械零点 (平衡角度)
extern float target_speed_set;      // 期望速度
extern float final_motor_pwm;       // 最终计算出的PWM值

// ==========================================
// 4. 函数声明
// ==========================================

/**
 * @brief 初始化PID参数和变量 (建议在main函数开始时调用)
 */
void PID_Param_Init(void);

/**
 * @brief 辅助函数：浮点数限幅
 */
float Float_Constrain(float val, float min, float max);

/**
 * @brief 速度环控制 (外环)
 * @param target_speed 期望速度
 * @param actual_speed 实际编码器速度
 * @return 期望的角度调整量 (度)
 */
float Speed_Loop_Control(float target_speed, float actual_speed);

/**
 * @brief 角度环控制 (中环)
 * @param speed_loop_output 速度环输出的角度调整量
 * @param actual_angle 当前IMU角度
 * @param mech_zero 机械零点
 * @return 期望的角速度
 */
float Angle_Loop_Control(float speed_loop_output, float actual_angle, float mech_zero);

/**
 * @brief 角速度环控制 (内环)
 * @param angle_loop_output 角度环输出的期望角速度
 * @param actual_gyro 当前IMU角速度
 * @return 电机PWM占空比
 */
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro);

/**
 * @brief 总中断控制函数 (建议在定时器中断中调用)
 */
//void Balance_Control_Interrupt(void);

#endif /* __PID_CONTROL_H__ */