#include "zf_common_headfile.h"

// ==========================================
// 1. 参数定义与结构体
// ==========================================


// 定义三个环的PID参数结构体
PID_Param_t pid_speed = {0.05, 0.002, 0, 0, 0, 0, 0, 0};    // 速度环 (需调试)
PID_Param_t pid_angle = {45.0, 0, 1.5, 0, 0, 0, 0, 0};      // 角度环 (需调试)
PID_Param_t pid_gyro  = {2.5,  0, 0.8, 0, 0, 0, 0, 0};      // 角速度环 (需调试)

// 全局变量
float mechanical_zero_angle = 0.0; // 机械零点（平衡时的理想角度，通常为0或微调值）
float target_speed_set = 0.0;      // 目标速度（由其他逻辑设置，如图像处理）
float final_motor_pwm = 0.0;       // 最终输出给电机的PWM

// 限幅宏定义
#define SPEED_OUT_LIMIT  8.0f     // 速度环输出限幅（最大倾斜角度，例如8度）
#define ANGLE_OUT_LIMIT  3000.0f  // 角度环输出限幅（最大期望角速度）
#define PWM_MAX_LIMIT    9000     // 电机PWM最大值

// 辅助函数：限幅
float Float_Constrain(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

// ==========================================
// 2. 蓝色框：速度环 (Velocity Loop)
// 作用：输入期望速度，输出期望的角度调整量（Leg Tilt）
// 频率：低频 (建议 20ms - 50ms 执行一次)
// ==========================================
float Speed_Loop_Control(float target_speed, float actual_speed)
{
    // 1. 计算误差
    pid_speed.error = target_speed - actual_speed;

    // 2. 积分项计算（位置式PID）
    pid_speed.error_integral += pid_speed.error;
    
    // 积分限幅 (防止积分饱和)
    pid_speed.error_integral = Float_Constrain(pid_speed.error_integral, -2000, 2000);

    // 3. 计算输出
    // 速度环通常只需要PI，D项很少用，因为编码器噪声大
    pid_speed.output = (pid_speed.kp * pid_speed.error) + 
                       (pid_speed.ki * pid_speed.error_integral);

    // 4. 输出限幅
    // 这一点非常重要：速度环的输出是"角度"，车不可能倾斜90度，所以要限制在安全范围内（如±10度）
    pid_speed.output = Float_Constrain(pid_speed.output, -SPEED_OUT_LIMIT, SPEED_OUT_LIMIT);
    
    // 更新历史误差 (若需要D项)
    pid_speed.last_error = pid_speed.error;

    return pid_speed.output; // 返回的是“腿部倾斜角度控制量”
}

// ==========================================
// 3. 红色框：角度环 (Angle Loop)
// 作用：输入期望角度，输出期望角速度
// 频率：中频 (建议 5ms 或与角速度环同频)
// ==========================================
float Angle_Loop_Control(float speed_loop_output, float actual_angle, float mech_zero)
{
    float target_angle;
    
    // 1. 确定期望角度
    // 期望角度 = 机械零点 - 速度环输出的倾角调整量
    // 符号说明：通常加速时需要前倾。假设前倾角度为负，若速度环输出正值（想加速），
    // 则需要让目标角度变负，所以这里可能是减号，具体取决于你的IMU安装方向。
    // 逻辑：Target = Zero - Adjustment
    target_angle = mech_zero - speed_loop_output; 

    // 2. 计算误差
    pid_angle.error = target_angle - actual_angle;

    // 3. 计算输出 (位置式PD控制)
    // 平衡车角度环主要靠Kp恢复，Kd抑制震荡，Ki通常不需要（或者很小）
    pid_angle.output = (pid_angle.kp * pid_angle.error) + 
                       (pid_angle.kd * (pid_angle.error - pid_angle.last_error));

    // 4. 输出限幅
    // 角度环的输出是"角速度"，物理上车子旋转速度有限
    pid_angle.output = Float_Constrain(pid_angle.output, -ANGLE_OUT_LIMIT, ANGLE_OUT_LIMIT);

    // 更新历史误差
    pid_angle.last_error = pid_angle.error;

    return pid_angle.output; // 返回的是“角速度干扰量/期望角速度”
}

// ==========================================
// 4. 绿色框：角速度环 (Gyro Loop)
// 作用：输入期望角速度，输出电机PWM
// 频率：高频 (建议 1ms - 2ms 执行一次)
// ==========================================
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro)
{
    float target_gyro;

    // 1. 确定期望角速度
    // 框图中：稳态角速度(0) - 输入干扰量。
    // 实际控制中：角度环的输出就是我们需要达到的角速度。
    target_gyro = angle_loop_output; 

    // 2. 计算误差
    pid_gyro.error = target_gyro - actual_gyro;

    // 3. 计算输出 (位置式PD控制)
    // 角速度环是响应最快的一环，Kd非常重要，用于消除抖动
    pid_gyro.output = (pid_gyro.kp * pid_gyro.error) + 
                      (pid_gyro.kd * (pid_gyro.error - pid_gyro.last_error));

    // 4. PWM限幅
    pid_gyro.output = Float_Constrain(pid_gyro.output, -PWM_MAX_LIMIT, PWM_MAX_LIMIT);

    // 更新历史误差
    pid_gyro.last_error = pid_gyro.error;

    return pid_gyro.output; // 返回的是“电机控制PWM占空比”
}

// ==========================================
// 5. 总中断调用示例 (系统调度)
// 假设此函数在定时器中断中被调用，周期为 1ms
// ==========================================
int time_count = 0;
float angle_loop_out = 0;
float speed_loop_out = 0;

// void Balance_Control_Interrupt(void)
// {
//     // 获取传感器数据 (伪代码，请替换为你实际的获取函数)
//     float current_angle = imu_data.pitch;  // 获取实际姿态角
//     float current_gyro  = imu_data.gyro_y; // 获取实际角速度
//     float current_speed = get_encoder_speed(); // 获取编码器速度

//     time_count++;

//     // --- 20ms 执行一次 速度环 ---
//     if(time_count % 20 == 0) 
//     {
//         // 速度环计算：输入(目标速度，当前速度)，输出(角度调整量)
//         speed_loop_out = Speed_Loop_Control(target_speed_set, current_speed);
//     }

//     // --- 5ms 执行一次 角度环 (也可以设为1ms) ---
//     if(time_count % 5 == 0)
//     {
//         // 角度环计算：输入(速度环输出，当前角度，机械零点)，输出(期望角速度)
//         angle_loop_out = Angle_Loop_Control(speed_loop_out, current_angle, mechanical_zero_angle);
//     }

//     // --- 1ms 执行一次 角速度环 ---
//     // 角速度环计算：输入(角度环输出，当前角速度)，输出(PWM)
//     final_motor_pwm = Gyro_Loop_Control(angle_loop_out, current_gyro);

//     // --- 将PWM输出给电机 ---
//     // 假设你有两个电机，根据安装方向，可能是一个正一个反，或者同向
//     // Check_Abnormal(); // 倒地检测，如果倒地了强制PWM为0
//     Motor_Set_PWM((int)final_motor_pwm);
    
//     // 计数器清零防止溢出
//     if(time_count >= 1000) time_count = 0;
// }
void PID_Param_Init(void)
{
    // 速度环初始化
    pid_speed.kp = 0.05f;
    pid_speed.ki = 0.002f;
    pid_speed.kd = 0.0f;
    pid_speed.error_integral = 0;

    // 角度环初始化
    pid_angle.kp = 45.0f;
    pid_angle.ki = 0.0f;
    pid_angle.kd = 1.5f;
    
    // 角速度环初始化
    pid_gyro.kp  = 2.5f;
    pid_gyro.ki  = 0.0f;
    pid_gyro.kd  = 0.8f;

    // 变量初始化
    mechanical_zero_angle = 0.0f; // 根据实际情况测量修改
    target_speed_set = 0.0f;
}