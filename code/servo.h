#ifndef _SERVO_H_
#define _SERVO_H_

#include "zf_common_headfile.h"

// *************************** 【学习板小车】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH09_P05_0)     // 左前 LF
#define SERVO_MOTOR_PWM2            (TCPWM_CH10_P05_1)      // 右前 RF
#define SERVO_MOTOR_PWM3            (TCPWM_CH11_P05_2)       // 右后 RR
#define SERVO_MOTOR_PWM4            (TCPWM_CH12_P05_3)      // 左后 LR
// *************************** 【学习板小车】硬件引脚定义结束***************************

// *************************** 【我们板小车1】硬件引脚定义开始 ***************************
// #define SERVO_MOTOR_PWM1            (TCPWM_CH13_P00_3)      // 右前 RF
// #define SERVO_MOTOR_PWM2            (TCPWM_CH12_P01_0)      // 右后 RR 
// #define SERVO_MOTOR_PWM3            (TCPWM_CH28_P19_3)      // 左前 LF 
// #define SERVO_MOTOR_PWM4            (TCPWM_CH27_P19_2)      // 左后 LR 
// *************************** 【我们板小车1】硬件引脚定义结束***************************


// ***************++的意思是向下伸腿要加duty*********
// ===================== 舵机角度-占空比转换宏定义 =====================
// ===================== 舵机收腿极限占空比定义 (Duty级) =====================
// #define SERVO_MOTOR_PWM1_MID            (4500-1500)      // 左前 LF ++
// #define SERVO_MOTOR_PWM2_MID            (4500+1500)      // 右前 RF --
// #define SERVO_MOTOR_PWM3_MID            (4500+1500)      // 右后 RR --
// #define SERVO_MOTOR_PWM4_MID            (4700-1500)      // 左后 LR ++

// ===================== 舵机平腿(90度)占空比定义 (Duty级) =====================
#define SERVO_MOTOR_PWM1_90            (4500)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_90            (4500)      // 右前 RF --
#define SERVO_MOTOR_PWM3_90            (4500)      // 右后 RR --
#define SERVO_MOTOR_PWM4_90            (4700)      // 左后 LR ++   
// ===================== 舵机伸腿极限占空比定义 (Duty级) =====================
// #define SERVO_MOTOR_PWM1_MID            (7000)      // 左前 LF ++
// #define SERVO_MOTOR_PWM2_MID            (2000)      // 右前 RF --
// #define SERVO_MOTOR_PWM3_MID            (2000)      // 右后 RR --
// #define SERVO_MOTOR_PWM4_MID            (7200)      // 左后 LR ++
// ===================== 舵机正常占空比定义 (Duty级) =====================
// #define SERVO_MOTOR_PWM1_MID            (3500)      // 左前 LF ++
// #define SERVO_MOTOR_PWM2_MID            (5500)      // 右前 RF --
// #define SERVO_MOTOR_PWM3_MID            (5500)      // 右后 RR --
// #define SERVO_MOTOR_PWM4_MID            (3700)      // 左后 LR ++
// ===================== 舵机极性定义 (向下伸腿为正) =====================
#define SERVO_MOTOR_PWM1_DIR            (1)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_DIR            (-1)      // 右前 RF --
#define SERVO_MOTOR_PWM3_DIR            (-1)      // 右后 RR --
#define SERVO_MOTOR_PWM4_DIR            (1)      // 左后 LR ++
// ===================== 舵机全局配置 =====================
#define SERVO_FREQ          (300)      // 频率300Hz

// ===================== 8个独立硬件限幅变量 (Duty级) =====================
// 右前 (RF) 限幅
#define RF_LIMIT_DUTY_MIN   (3000) 
#define RF_LIMIT_DUTY_MAX   (7000) 

// 右后 (RR) 限幅
#define RR_LIMIT_DUTY_MIN   (2000) 
#define RR_LIMIT_DUTY_MAX   (6000) 

// 左前 (LF) 限幅
#define LF_LIMIT_DUTY_MIN   (2000) 
#define LF_LIMIT_DUTY_MAX   (6000) 

// 左后 (LR) 限幅
#define LR_LIMIT_DUTY_MIN   (3200) 
#define LR_LIMIT_DUTY_MAX   (7200) 

// 舵机初始化高度
#define INIT_HEIGHT   3.0f

//五连杆参数
#define L1  6.0f    //左小腿长
#define L2  9.0f    //左大腿长
#define L3  9.0f    //右大腿长
#define L4  6.0f    //右小腿长
#define L5  3.7f    //舵机间距

#define P_max 14.50f
#define P_min 2.70f
#define A_max 20.00f
#define A_min -20.00f
#define P_step 0.03f
#define A_step 0.10f

extern int16 pwm_high;
extern float pwm_angle;

// ===================== 4. 函数声明 =====================

void servo_init_all(void);    // 初始化所有舵机到收腿状态
        
void action_contract_legs(void);//收腿到收腿极限
// 写入目标角度 (0.0 - 180.0)，内部自动执行8变量限幅
void servo_write_angle(pwm_channel_enum ch, float angle); 

// 一键联动控制 (输入90中位，输入70按逻辑收腿)
void robot_posture_control(float base_angle);             

// 获取当前四个舵机的物理角度 (结果存入长度为4的数组)
// 数组顺序: [0]=RF, [1]=RR, [2]=LF, [3]=LR
void servo_get_current_angles(float *angles_array);

void servo_write_duty(pwm_channel_enum ch, int32 duty);//单独控制指定舵机至目标占空比 (带独立限幅)

void high_control_table(float p);//高度查表函数
void servo_control_table(float p, float degree);//五连杆解算，舵机控制查表函数

#endif