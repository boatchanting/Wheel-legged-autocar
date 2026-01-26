#include "zf_common_headfile.h"

// ============================================================================
//  *** TUNING AREA / 核心调参区 ***
//  这里定义了初始化结构体的具体数值。
//  修改这里的宏定义，即可改变平衡车的特性。
// ============================================================================

// ----------------------------------------------------------------------------
// 1. 速度环参数 (最外环 - 周期约 20ms~50ms)
//    作用：通过改变车身倾角，让车“跑”起来去追重心，从而保持位置或达到目标速度。
// ----------------------------------------------------------------------------
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
#define GYR_KD      0.0f    // [消除抖动] 抑制高频噪声和电机抖动。

#define GYR_MAX_I   0.0f    
#define GYR_MAX_O   6000.0f // [PWM满幅] 满是10000，这里留点余量设3000。

// [关键补偿] 电机死区 (Dead Zone Voltage)
// 直流电机存在静摩擦，PWM太小(如200)时不转。
// 如果PID算出输出100，加上死区300，实际给400，车轮正好能动，消除了低速时的非线性迟滞。
#define GYR_DEAD_ZONE  0.0f  

// [传感器误差] 陀螺仪静态零偏 (需静止测量)
#define GYRO_SENSOR_OFFSET  0.0f 


// ============================================================================
//  全局变量初始化
//  将宏定义的参数填入结构体
// ============================================================================
PID_Param_t pid_speed = {SPD_KP, SPD_KI, SPD_KD, SPD_MAX_O, SPD_MAX_I, SPD_COMP,      0,0,0,0,0};
PID_Param_t pid_angle = {ANG_KP, ANG_KI, ANG_KD, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO, 0,0,0,0,0};
PID_Param_t pid_gyro  = {GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE, 0,0,0,0,0};

float target_speed_set = 0.0f;

//状态与调试变量
volatile float now_speed       = 0.0f;
volatile float now_angle       = 0.0f;
volatile float now_gyro        = 0.0f;

float speed_loop_out    = 0.0f;
float angle_loop_out    = 0.0f;
float gyro_loop_out     = 0.0f; 

volatile float final_motor_pwm = 0.0f;



// ============================================================================
//  辅助函数实现
// ============================================================================

/**
 * @brief 限幅函数
 */
float Float_Constrain(float val, float min, float max) {
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

/**
 * @brief PID 过程数据初始化
 * @note  调用此函数后，所有PID环的积分项和输出都会被重置为0。
 *        
 */
void PID_Param_Init(void) {
    // 初始化速度环PID参数
    // pid_speed.kp = SPD_KP;
    // pid_speed.ki = SPD_KI;
    // pid_speed.kd = SPD_KD;
    pid_speed.max_output = SPD_MAX_O;
    pid_speed.max_integral = SPD_MAX_I;
    pid_speed.compensation = SPD_COMP;
    
    // 重置速度环状态变量
    pid_speed.error = 0;
    pid_speed.last_error = 0;
    pid_speed.prev_error = 0;
    pid_speed.error_integral = 0;
    pid_speed.output = 0;

    // 初始化角度环PID参数
    // pid_angle.kp = ANG_KP;
    // pid_angle.ki = ANG_KI;
    // pid_angle.kd = ANG_KD;
    pid_angle.max_output = ANG_MAX_O;
    pid_angle.max_integral = ANG_MAX_I;
    pid_angle.compensation = ANG_MECH_ZERO;
    
    // 重置角度环状态变量
    pid_angle.error = 0;
    pid_angle.last_error = 0;
    pid_angle.prev_error = 0;
    pid_angle.error_integral = 0;
    pid_angle.output = 0;
    
    // 初始化角速度环PID参数
    // pid_gyro.kp = GYR_KP;
    // pid_gyro.ki = GYR_KI;
    // pid_gyro.kd = GYR_KD;
    pid_gyro.max_output = GYR_MAX_O;
    pid_gyro.max_integral = GYR_MAX_I;
    pid_gyro.compensation = GYR_DEAD_ZONE;
    
    // 重置角速度环状态变量
    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;
    
    // 重置目标速度
    target_speed_set = 0.0f;
}

/**
 * @brief 将所有PID结构体成员变量设置为0
 */
void PID_Data_Reset(void) {
    memset(&pid_speed, 0, sizeof(PID_Param_t));
    memset(&pid_angle, 0, sizeof(PID_Param_t));
    memset(&pid_gyro, 0, sizeof(PID_Param_t));
    target_speed_set = 0;
}


// ============================================================================
//  控制函数实现 (核心算法)
// ============================================================================

/**
 * @brief 速度环控制 (外环)
 * @param target_speed 期望速度 (通常遥控给定)
 * @param actual_speed 实际速度 (编码器测得)
 * @return 期望的角度调整量 (单位：度)
 * @note   原理：想让车加速，就得让车身先往前倾斜，利用重力分量加速。
 *         所以速度环的输出，实际上是角度环的目标输入。
 */
float Speed_Loop_Control(float target_speed, float actual_speed)
{
    // 1. 计算误差
    pid_speed.error = target_speed - actual_speed;
    
    // 2. 积分计算 (速度环核心)
    // 速度环主要靠积分作用来消除静差，达到恒定速度
    pid_speed.error_integral += pid_speed.error;
    
    // 积分限幅：防止积分项过大导致系统失控
    pid_speed.error_integral = Float_Constrain(pid_speed.error_integral, -pid_speed.max_integral, pid_speed.max_integral);

    // 3. PI计算 (速度环通常不需要D项)
    pid_speed.output = (pid_speed.kp * pid_speed.error) + 
                       (pid_speed.ki * pid_speed.error_integral);

    // 4. 输出限幅 (关键！)
    // 速度环输出的是“目标倾角”。我们不能让车为了加速而倾斜45度，那样就倒了。
    // 所以这里限制最大倾角为 pid_speed.max_output (例如8度)。
    pid_speed.output = Float_Constrain(pid_speed.output, -pid_speed.max_output, pid_speed.max_output);
    
    // 5. 更新历史误差链
    // 顺序很重要：先把 上次 存为 上上次，再把 这次 存为 上次
    pid_speed.prev_error = pid_speed.last_error; 
    pid_speed.last_error = pid_speed.error;
    
    return pid_speed.output; 
}

/**
 * @brief 角度环控制 (中环)
 * @param speed_loop_output 速度环计算出的角度调整量
 * @param actual_angle      当前IMU测量的实际角度
 * @return 期望的角速度 (单位：度/秒 或 LSB)
 * @note   这是维持平衡的核心。
 */
float Angle_Loop_Control(float speed_loop_output, float actual_angle)
{
    // 1. 确定目标角度
    // 目标角度 = 机械零点(平衡点) - 速度环调节量
    // 如果速度环输出正值(想加速)，通常需要车前倾。
    // 假设前倾是负角度，那么 Target = Zero - Positive，目标变小(变负)，车会前倾。
    // (注意：这里的正负号取决于你的IMU安装方向，可能需要改为 + )
    float target_angle = pid_angle.compensation - speed_loop_output; 

    // 2. 计算误差
    pid_angle.error = target_angle - actual_angle;

    // 3. 积分计算 (直立环一般 ki=0)
    if(pid_angle.ki != 0) {
        pid_angle.error_integral += pid_angle.error;
        pid_angle.error_integral = Float_Constrain(pid_angle.error_integral, -pid_angle.max_integral, pid_angle.max_integral);
    }

    // 4. PD计算 (直立环核心)
    // P项：回复力，偏差越大，回复力越大。
    // D项：阻尼力，偏差变化越快，反向阻力越大，防止超调震荡。
    pid_angle.output = (pid_angle.kp * pid_angle.error) + 
                       (pid_angle.ki * pid_angle.error_integral) + 
                       (pid_angle.kd * (pid_angle.error - pid_angle.last_error));

    // 5. 输出限幅
    // 限制期望的最大角速度
    pid_angle.output = Float_Constrain(pid_angle.output, -pid_angle.max_output, pid_angle.max_output);
    
    // 6. 更新历史误差链
    pid_angle.prev_error = pid_angle.last_error;
    pid_angle.last_error = pid_angle.error;

    // 返回负值通常是为了匹配电机控制方向，需根据实际情况调整
    return -pid_angle.output; 
}

/**
 * @brief 角速度环控制 (内环)
 * @param angle_loop_output 角度环计算出的期望角速度
 * @param actual_gyro       当前IMU测量的实际角速度
 * @return 最终电机 PWM 值
 * @note   这一环频率最高(1ms)，直接反应给电机电压。
 */
float Gyro_Loop_Control(float angle_loop_output, float actual_gyro)
{
    // 0. 传感器校准
    // 减去静态零偏，保证静止时数据为0
    float real_gyro = actual_gyro - GYRO_SENSOR_OFFSET; 

    // 1. 计算误差
    pid_gyro.error = angle_loop_output - real_gyro;

    // 2. PD计算
    // 角速度环 D项能极好地抑制高频抖动
    pid_gyro.output = (pid_gyro.kp * pid_gyro.error) + 
                      (pid_gyro.kd * (pid_gyro.error - pid_gyro.last_error));

    // 3. 死区补偿 (关键优化)
    // 电机有静摩擦力。如果计算出的 PWM 很小(如50)，电机不动，控制就失效了。
    // 所以只要有输出意图，就额外叠加一个起步电压(compensation)，让电机立即响应。
    if (pid_gyro.output > 0) {
        pid_gyro.output += pid_gyro.compensation; // 正转加死区
    } else if (pid_gyro.output < 0) {
        pid_gyro.output -= pid_gyro.compensation; // 反转减死区
    }

    // 4. 输出限幅
    // 限制在定时器允许的 PWM 范围内
    pid_gyro.output = Float_Constrain(pid_gyro.output, -pid_gyro.max_output, pid_gyro.max_output);

    // 5. 更新历史误差链
    pid_gyro.prev_error = pid_gyro.last_error;
    pid_gyro.last_error = pid_gyro.error;

    return pid_gyro.output;
}