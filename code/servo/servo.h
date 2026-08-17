#ifndef _SERVO_H_
#define _SERVO_H_

#include "zf_common_headfile.h"
#include "../config/car_select.h"

#if CAR_SELECT == 0 // 0代表学习板小车 板子 学习板 v1.2
// *************************** 【学习板小车】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH09_P05_0)     // 左前 LF
#define SERVO_MOTOR_PWM2            (TCPWM_CH10_P05_1)      // 右前 RF
#define SERVO_MOTOR_PWM3            (TCPWM_CH12_P05_3)       // 右后 RR
#define SERVO_MOTOR_PWM4            (TCPWM_CH11_P05_2)      // 左后 LR
// *************************** 【学习板小车】硬件引脚定义结束***************************

// ===================== 舵机平腿(90度)占空比定义 (Duty级)【学习板小车】 =====================
#define SERVO_MOTOR_PWM1_90            (4500)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_90            (4500)      // 右前 RF --
#define SERVO_MOTOR_PWM3_90            (4600)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_90            (4300)      // 左后 LR --

// ***************++的意思是向下伸腿要加duty*********
// ===================== 舵机极性定义 (向下伸腿为正) 【学习板小车】 =====================
#define SERVO_MOTOR_PWM1_DIR            (1)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_DIR            (-1)      // 右前 RF --
#define SERVO_MOTOR_PWM3_DIR            (1)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_DIR            (-1)      // 左后 LR --
#endif

#if CAR_SELECT == 1 //1代表学习板小车 对应板子 2026/01 队名还未定 【此板子没有wifi，暂时弃用】 
// *************************** 【我们板小车1】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH13_P00_3)      // 右前 RF
#define SERVO_MOTOR_PWM2            (TCPWM_CH12_P01_0)      // 右后 RR 
#define SERVO_MOTOR_PWM3            (TCPWM_CH28_P19_3)      // 左前 LF 
#define SERVO_MOTOR_PWM4            (TCPWM_CH27_P19_2)      // 左后 LR 
// *************************** 【我们板小车1】硬件引脚定义结束***************************
#endif

#if CAR_SELECT == 2 // 2代表 【2026/1/31新车】 对应板子 【2026/01/16 锦鲤跃龙门】
// *************************** 【2026/1/31新车】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH29_P22_5)     // 左前 LF
#define SERVO_MOTOR_PWM2            (TCPWM_CH28_P22_6)      // 右前 RF
#define SERVO_MOTOR_PWM3            (TCPWM_CH30_P22_4)       // 右后 RR
#define SERVO_MOTOR_PWM4            (TCPWM_CH31_P22_3)      // 左后 LR
// *************************** 【2026/1/31新车】硬件引脚定义结束***************************
// ===================== 舵机平腿(90度)占空比定义 (Duty级)【2026/1/31新车】 =====================
#define SERVO_MOTOR_PWM1_90            (4550)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_90            (4700)      // 右前 RF --
#define SERVO_MOTOR_PWM3_90            (4600)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_90            (4600)      // 左后 LR --
// ===================== 舵机极性定义 (向下伸腿为正) 【2026/1/31新车】=====================
#define SERVO_MOTOR_PWM1_DIR            (1)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_DIR            (-1)      // 右前 RF --
#define SERVO_MOTOR_PWM3_DIR            (1)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_DIR            (-1)      // 左后 LR --
#endif


#if CAR_SELECT ==  3 // 3代表 【2026/3/30新车】 对应板子 【2026/03/24 最后的舵机v腿】
// *************************** 【2026/03/24 最后的舵机v腿】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH29_P22_5)     // 左前 LF
#define SERVO_MOTOR_PWM2            (TCPWM_CH28_P22_6)      // 右前 RF
#define SERVO_MOTOR_PWM3            (TCPWM_CH30_P22_4)       // 右后 RR
#define SERVO_MOTOR_PWM4            (TCPWM_CH31_P22_3)      // 左后 LR
// *************************** 【2026/03/24 最后的舵机v腿】硬件引脚定义结束***************************
// ===================== 舵机平腿(90度)占空比定义 (Duty级)【2026/03/24 最后的舵机v腿】 =====================
#define SERVO_MOTOR_PWM1_90            (4550)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_90            (4700)      // 右前 RF --
#define SERVO_MOTOR_PWM3_90            (4700)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_90            (4800)      // 左后 LR --
// ===================== 舵机极性定义 (向下伸腿为正) 【2026/03/24 最后的舵机v腿】=====================
#define SERVO_MOTOR_PWM1_DIR            (1)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_DIR            (-1)      // 右前 RF --
#define SERVO_MOTOR_PWM3_DIR            (1)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_DIR            (-1)      // 左后 LR --
#endif

#if CAR_SELECT == 4 // 4代表 【小车4】 初版参数与小车3相同，待实车标定
// *************************** 【小车4】硬件引脚定义开始 ***************************
#define SERVO_MOTOR_PWM1            (TCPWM_CH29_P22_5)     // 左前 LF
#define SERVO_MOTOR_PWM2            (TCPWM_CH28_P22_6)      // 右前 RF
#define SERVO_MOTOR_PWM3            (TCPWM_CH30_P22_4)       // 右后 RR
#define SERVO_MOTOR_PWM4            (TCPWM_CH31_P22_3)      // 左后 LR
// *************************** 【小车4】硬件引脚定义结束 ***************************
// ===================== 舵机平腿(90度)占空比定义 (Duty级)【小车4】=====================
#define SERVO_MOTOR_PWM1_90            (4450)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_90            (4500)      // 右前 RF --
#define SERVO_MOTOR_PWM3_90            (4500)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_90            (4700)      // 左后 LR --
// ===================== 舵机极性定义 (向下伸腿为正) 【小车4】====================
#define SERVO_MOTOR_PWM1_DIR            (1)      // 左前 LF ++
#define SERVO_MOTOR_PWM2_DIR            (-1)      // 右前 RF --
#define SERVO_MOTOR_PWM3_DIR            (1)      // 右后 RR ++
#define SERVO_MOTOR_PWM4_DIR            (-1)      // 左后 LR --
#endif

// ===================== 舵机全局配置 =====================
#define SERVO_FREQ          (300)      // 频率300Hz

// ===================== 8个独立硬件限幅变量 (Duty级) =====================
//伸腿极限一般正负2500，收腿极限一般正负1500
// 右前 (RF) 限幅
#define RF_LIMIT_DUTY_MIN   (SERVO_MOTOR_PWM2_90-2000+SERVO_MOTOR_PWM2_DIR*500) 
#define RF_LIMIT_DUTY_MAX   (SERVO_MOTOR_PWM2_90+2000+SERVO_MOTOR_PWM2_DIR*500) 
// 右后 (RR) 限幅
#define RR_LIMIT_DUTY_MIN   (SERVO_MOTOR_PWM3_90-2000+SERVO_MOTOR_PWM3_DIR*500) 
#define RR_LIMIT_DUTY_MAX   (SERVO_MOTOR_PWM3_90+2000+SERVO_MOTOR_PWM3_DIR*500) 
// 左前 (LF) 限幅
#define LF_LIMIT_DUTY_MIN   (SERVO_MOTOR_PWM1_90-2000+SERVO_MOTOR_PWM1_DIR*500) 
#define LF_LIMIT_DUTY_MAX   (SERVO_MOTOR_PWM1_90+2000+SERVO_MOTOR_PWM1_DIR*500) 
// 左后 (LR) 限幅
#define LR_LIMIT_DUTY_MIN   (SERVO_MOTOR_PWM4_90-2000+SERVO_MOTOR_PWM4_DIR*500) 
#define LR_LIMIT_DUTY_MAX   (SERVO_MOTOR_PWM4_90+2000+SERVO_MOTOR_PWM4_DIR*500) 
// 舵机初始化高度
extern float servo_height;
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
// 180°角度范围对应的duty变化量：2.0ms * 300Hz * 10000 / 1000 = 6000
#define DUTY_RANGE_180      (2.0f * SERVO_FREQ * PWM_DUTY_MAX / 1000.0f)  // 6000.0f

// 每单位duty变化对应的角度变化量（度/duty）
#define DEGREE_PER_DUTY     (180.0f / DUTY_RANGE_180)  // 0.03f

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

/**
 * @brief 根据指定舵机当前的 PWM 占空比（duty）值，计算其对应的实际机械角度，
 *        并更新全局静态数组 `current_angles` 中对应元素。
 *
 * 该函数依据舵机控制标准：0° ~ 180° 对应高电平脉宽 0.5 ms ~ 2.5 ms，
 * 结合系统配置的 PWM 频率（SERVO_FREQ = 300 Hz）及各舵机独立的中位（90°）基准 duty 值
 * 和安装方向极性（DIR），进行线性反向映射计算。
 *
 * 计算公式为：
 *     angle = 90.0 + (current_duty - duty_90) × (180.0 / DUTY_RANGE_180) × direction
 * 其中 DUTY_RANGE_180 = 2.0 ms × SERVO_FREQ × PWM_DUTY_MAX / 1000.0，
 * 表示 180° 角度范围所对应的 duty 变化总量（理论值为 6000）。
 *
 * 计算结果将被限制在物理有效范围 [0.0, 180.0] 度内，防止因异常 duty 值导致角度越界。
 *
 * @param[in] servo_index 舵机索引号，取值范围为 0 ~ 3：
 *                        - 0: 左前 (LF)
 *                        - 1: 右前 (RF)
 *                        - 2: 右后 (RR)
 *                        - 3: 左后 (LR)
 * @param[in] current_duty 当前施加于该舵机通道的 PWM 占空比数值（单位：duty count），
 *                         应为非负整数，典型范围取决于具体限幅宏（如 2000~7200）。
 *
 * @note 此函数不执行硬件读取，需由调用者传入已知或刚设置的 duty 值。
 * @note 函数内部包含参数有效性检查，若 servo_index 超出 [0,3] 范围，函数直接返回而不修改状态。
 * @note 全局数组 `current_angles` 在函数内部被更新，供其他模块只读访问。
 */
void update_servo_angle(uint8_t servo_index, uint16_t current_duty);


/**
 * @brief 批量更新全部四个舵机的角度状态。
 *
 * 调用此函数可一次性根据传入的四个 duty 值数组，依次调用 `update_servo_angle`
 * 更新 `current_angles[0..3]` 的全部元素。
 *
 * @param[in] duty_values 指向包含 4 个元素的 uint16_t 数组，
 *                        顺序应为 {LF_duty, RF_duty, RR_duty, LR_duty}。
 *
 * @note 若传入指针为 NULL，函数将立即返回，不做任何操作。
 * @note 数组元素顺序必须严格匹配舵机物理布局，否则角度映射将出错。
 */
void update_all_servo_angles(const uint16_t* duty_values);


/**
 * @brief 获取指定舵机当前存储的角度值（只读接口）。
 *
 * 返回由 `update_servo_angle` 或 `update_all_servo_angles` 最近一次计算并缓存的角度。
 * 该值反映软件认为的舵机当前位置，可用于运动学解算、状态反馈等。
 *
 * @param[in] servo_index 舵机索引号（0 ~ 3，含义同上）。
 *
 * @return 当前角度值，单位为度（°），范围 [0.0, 180.0]。
 *         若索引无效，则返回默认中位值 90.0f。
 *
 * @warning 此函数不触发实际角度测量，仅返回缓存值。确保在舵机动作后及时调用更新函数。
 */
float get_servo_angle(uint8_t servo_index);

#endif
