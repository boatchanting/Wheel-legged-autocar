#ifndef CODE__PID_NEW_H__
#define CODE__PID_NEW_H__
#define SPD_KP      0.0f   // [响应力度] 值越大，车对速度误差越敏感，加速越猛，但容易超调晃动
#define SPD_KI      0.0f  // [消除静差] 值越大，车越能克服阻力达到目标速度，但回正越慢
#define SPD_KD      0.0f    // [抑制震荡] 速度环一般不加D，因为编码器噪声大，且不需要极快响应

#define SPD_MAX_I   2000.0f // [积分防饱和] 限制积分项的最大贡献
#define SPD_MAX_O   1500.0f    // [安全角度] 速度环输出的是“期望角度”。限制为8度，意味着车最快加速时也不能倾斜超过8度，防止扑街。
#define SPD_COMP    0.0f    // 速度环暂不需要额外补偿

// ----------------------------------------------------------------------------
// 2. 角度环参数 (中间环 - 周期约 5ms)
//    作用：根据期望角度(来自机械零点+速度环)，计算出需要的角速度。
//    这是维持直立最关键的一环。
// ----------------------------------------------------------------------------
#define ANG_KP      0.0f   // [直立刚度] 类似于弹簧的硬度。值太小车软绵绵扶不正；值太大车会剧烈低频抖动。
#define ANG_KI      0.0f    // [一般不用] 平衡车本身是不稳定系统，加积分容易导致无法直立，除非是完全静态的高精度控制。
#define ANG_KD      0.0f    // [直立阻尼] 极重要！类似于减震器。值太小车会有余震；值太大车反应迟钝且有高频噪音。

#define ANG_MAX_I   0.0f    // 积分限幅
#define ANG_MAX_O   8000.0f // [最大角速度] 限制期望的旋转速度，防止电机指令过大。

// [关键补偿] 机械零点 (Mechanical Zero)
// 理想情况下0度是平衡点。但因电池安装、传感器贴歪等原因，实际平衡点可能是 -1.5度。
// 调试方法：如果车总是往“前”跑，说明它觉得自己后仰了，需要减小这个值；反之增大。
#define ANG_MECH_ZERO  0.0f   

// ----------------------------------------------------------------------------
// 3. 角速度环参数 (最内环 - 周期约 1ms)
//    作用：直接控制电机PWM，让车身角速度迅速跟随角度环的指令。
//    这一环必须响应最快。
// ----------------------------------------------------------------------------
#define GYR_KP      0.0f    // [响应速度] 决定了电机对旋转的抵抗力。
#define GYR_KI      0.0f    // [一般不用] 响应太快，积分来不及反应，反而造成滞后。
#define GYR_KD      0.1f    // [消除抖动] 抑制高频噪声和电机抖动。

#define GYR_MAX_I   0.0f    
#define GYR_MAX_O   6000.0f // [PWM满幅] 满是10000，这里留点余量设3000。

// [关键补偿] 电机死区 (Dead Zone Voltage)
// 直流电机存在静摩擦，PWM太小(如200)时不转。
// 如果PID算出输出100，加上死区300，实际给400，车轮正好能动，消除了低速时的非线性迟滞。
#define GYR_DEAD_ZONE  0.0f  

// [传感器误差] 陀螺仪静态零偏 (需静止测量)
#define GYRO_SENSOR_OFFSET  0.0f 
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