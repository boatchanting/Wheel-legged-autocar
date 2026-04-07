#include "zf_common_headfile.h"


// ============================================================================
//  全局变量初始化
//  将宏定义的参数填入结构体
// ============================================================================
PID_Param_t pid_servo_speed = {SERVO_SPEED_KP, SERVO_SPEED_KI, SERVO_SPEED_KD, SERVO_SPEED_MAX_O, SERVO_SPEED_MAX_I, SERVO_SPEED_COMP, 0,0,0,0,0};//舵机速度环初始化参数
PID_Param_t pid_speed = {SPD_KP, SPD_KI, SPD_KD, SPD_MAX_O, SPD_MAX_I, SPD_COMP,      0,0,0,0,0};//速度环初始化参数
PID_Param_t pid_angle = {ANG_KP, ANG_KI, ANG_KD, ANG_MAX_O, ANG_MAX_I, ANG_MECH_ZERO, 0,0,0,0,0};//角度环初始化参数
PID_Param_t pid_gyro  = {GYR_KP, GYR_KI, GYR_KD, GYR_MAX_O, GYR_MAX_I, GYR_DEAD_ZONE, 0,0,0,0,0};//角速度环初始化参数
PID_Param_t pid_turn_angle = {TURN_ANG_KP, TURN_ANG_KI, TURN_ANG_KD, TURN_ANG_MAX_O, TURN_ANG_MAX_I, TURN_ANG_DEAD_ZONE, 0,0,0,0,0};//转向角度环初始化参数
PID_Param_t pid_turn_gyro = {TURN_GYR_KP, TURN_GYR_KI, TURN_GYR_KD, TURN_GYR_MAX_O, TURN_GYR_MAX_I, TURN_GYR_DEAD_ZONE, 0,0,0,0,0};//转向角速度环初始化参数
PID_Param_t pid_roll = {ROLL_KP, ROLL_KI, ROLL_KD, ROLL_MAX_O, ROLL_MAX_I, ROLL_MECH_ZERO, 0,0,0,0,0};//横滚环初始化参数


volatile float target_speed_set = 0.0f;

//状态与调试变量
volatile float now_speed       = 0.0f;
volatile float now_angle       = 0.0f;
volatile float now_gyro        = 0.0f;
float current_actual_speed = 0.0f; // 当前实际速度变量（单位：r/min）
float speed_loop_out    = 0.0f;// 速度环的输出 (目标角度)
float angle_loop_out    = 0.0f;// 角度环的输出 (目标角速度)
float gyro_loop_out     = 0.0f;// 角速度环的输出 (目标角加速度)
volatile float turn_angle_loop_out = 0.0f;// 转向角度环输出（期望角速度）
volatile float turn_gyro_loop_out = 0.0f;// 转向角速度环输出（PWM）
volatile float final_motor_pwm = 0.0f;
uint8_t roll_balance_enable = 0; // 横滚平衡环使能开关
volatile int16 g_target_pwm_roll_adj = 0; // 目标横滚调整分量

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
    // 初始化舵机速度环PID参数
    pid_servo_speed.kp = SERVO_SPEED_KP;
    pid_servo_speed.ki = SERVO_SPEED_KI;
    pid_servo_speed.kd = SERVO_SPEED_KD;
    pid_servo_speed.max_output = SERVO_SPEED_MAX_O;
    pid_servo_speed.max_integral = SERVO_SPEED_MAX_I;
    pid_servo_speed.compensation = SERVO_SPEED_COMP;
    
    // 重置舵机速度环状态变量
    pid_servo_speed.error = 0;
    pid_servo_speed.last_error = 0;
    pid_servo_speed.prev_error = 0;
    pid_servo_speed.error_integral = 0;
    pid_servo_speed.output = 0;

    // 初始化速度环PID参数
    pid_speed.kp = SPD_KP;
    pid_speed.ki = SPD_KI;
    pid_speed.kd = SPD_KD;
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
    pid_angle.kp = ANG_KP;
    pid_angle.ki = ANG_KI;
    pid_angle.kd = ANG_KD;
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
    pid_gyro.kp = GYR_KP;
    pid_gyro.ki = GYR_KI;
    pid_gyro.kd = GYR_KD;
    pid_gyro.max_output = GYR_MAX_O;
    pid_gyro.max_integral = GYR_MAX_I;
    pid_gyro.compensation = GYR_DEAD_ZONE;
    
    // 重置角速度环状态变量
    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;

    //初始化转向角度环PID参数
    pid_turn_angle.kp = TURN_ANG_KP;
    pid_turn_angle.ki = TURN_ANG_KI;
    pid_turn_angle.kd = TURN_ANG_KD;
    pid_turn_angle.max_output = TURN_ANG_MAX_O;
    pid_turn_angle.max_integral = TURN_ANG_MAX_I;
    pid_turn_angle.compensation = TURN_ANG_DEAD_ZONE;
    
    // 重置转向角度环状态变量
    pid_turn_angle.error = 0;
    pid_turn_angle.last_error = 0;
    pid_turn_angle.prev_error = 0;
    pid_turn_angle.error_integral = 0;
    pid_turn_angle.output = 0;

    // 初始化转向角速度环PID参数
    pid_turn_gyro.kp = TURN_GYR_KP;
    pid_turn_gyro.ki = TURN_GYR_KI;
    pid_turn_gyro.kd = TURN_GYR_KD;
    pid_turn_gyro.max_output = TURN_GYR_MAX_O;
    pid_turn_gyro.max_integral = TURN_GYR_MAX_I;
    pid_turn_gyro.compensation = TURN_GYR_DEAD_ZONE;
    
    // 重置转向角速度环状态变量
    pid_turn_gyro.error = 0;
    pid_turn_gyro.last_error = 0;
    pid_turn_gyro.prev_error = 0;
    pid_turn_gyro.error_integral = 0;
    pid_turn_gyro.output = 0;

    // 初始化横滚环PID参数
    pid_roll.kp = ROLL_KP;
    pid_roll.ki = ROLL_KI;
    pid_roll.kd = ROLL_KD;
    pid_roll.max_output = ROLL_MAX_O;
    pid_roll.max_integral = ROLL_MAX_I;
    pid_roll.compensation = ROLL_MECH_ZERO;

    // 重置横滚环状态变量
    pid_roll.error = 0;
    pid_roll.last_error = 0;
    pid_roll.prev_error = 0;
    pid_roll.error_integral = 0;
    pid_roll.output = 0;

     // 重置横滚环使能位
    roll_balance_enable = 0;
    g_target_pwm_roll_adj = 0;

    // 重置目标速度
    target_speed_set = 0.0f;
}

/**
 * @brief 将所有PID结构体成员变量设置为0
 */
void PID_Data_Reset(void) {
    // 初始化舵机速度环PID参数
    pid_servo_speed.kp = 0;
    pid_servo_speed.ki = 0;
    pid_servo_speed.kd = 0;
    pid_servo_speed.max_output = SERVO_SPEED_MAX_O;
    pid_servo_speed.max_integral = SERVO_SPEED_MAX_I;
    pid_servo_speed.compensation = SERVO_SPEED_COMP;
    
    // 重置舵机速度环状态变量
    pid_servo_speed.error = 0;
    pid_servo_speed.last_error = 0;
    pid_servo_speed.prev_error = 0;
    pid_servo_speed.error_integral = 0;
    pid_servo_speed.output = 0;

    // 初始化速度环PID参数
    pid_speed.kp = 0;
    pid_speed.ki = 0;
    pid_speed.kd = 0;
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
    pid_angle.kp = 0;
    pid_angle.ki = 0;
    pid_angle.kd = 0;
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
    pid_gyro.kp = 0;
    pid_gyro.ki = 0;
    pid_gyro.kd = 0;
    pid_gyro.max_output = GYR_MAX_O;
    pid_gyro.max_integral = GYR_MAX_I;
    pid_gyro.compensation = GYR_DEAD_ZONE;
    
    // 重置角速度环状态变量
    pid_gyro.error = 0;
    pid_gyro.last_error = 0;
    pid_gyro.prev_error = 0;
    pid_gyro.error_integral = 0;
    pid_gyro.output = 0;

    //初始化转向角度环PID参数
    pid_turn_angle.kp = 0;
    pid_turn_angle.ki = 0;
    pid_turn_angle.kd = 0;
    pid_turn_angle.max_output = TURN_ANG_MAX_O;
    pid_turn_angle.max_integral = TURN_ANG_MAX_I;
    pid_turn_angle.compensation = TURN_ANG_DEAD_ZONE;
    
    // 重置转向角度环状态变量
    pid_turn_angle.error = 0;
    pid_turn_angle.last_error = 0;
    pid_turn_angle.prev_error = 0;
    pid_turn_angle.error_integral = 0;
    pid_turn_angle.output = 0;

    // 初始化转向角速度环PID参数
    pid_turn_gyro.kp = 0;
    pid_turn_gyro.ki = 0;
    pid_turn_gyro.kd = 0;
    pid_turn_gyro.max_output = TURN_GYR_MAX_O;
    pid_turn_gyro.max_integral = TURN_GYR_MAX_I;
    pid_turn_gyro.compensation = TURN_GYR_DEAD_ZONE;
    
    // 重置转向角速度环状态变量
    pid_turn_gyro.error = 0;
    pid_turn_gyro.last_error = 0;
    pid_turn_gyro.prev_error = 0;
    pid_turn_gyro.error_integral = 0;
    pid_turn_gyro.output = 0;

    //初始化横滚环PID参数
    pid_roll.kp = 0;
    pid_roll.ki = 0;
    pid_roll.kd = 0;
    pid_roll.max_output = ROLL_MAX_O;
    pid_roll.max_integral = ROLL_MAX_I;
    pid_roll.compensation = ROLL_MECH_ZERO;

    // 重置横滚环状态变量
    pid_roll.error = 0;
    pid_roll.last_error = 0;
    pid_roll.prev_error = 0;
    pid_roll.error_integral = 0;
    pid_roll.output = 0;

    // 重置目标速度
    target_speed_set = 0.0f;
}


// ============================================================================
//  控制函数实现
// ============================================================================

/**
 * @brief 转向角度环控制器（外环）- 完整PID参数实现
 * @param angle_error 角度误差（期望转向角 - 实际转向角，单位：度）
 *                    由视觉系统或编码器差分计算得出
 * @return 期望角速度指令（单位：°/s），作为转向角速度环的输入
 * 
 * 【完整参数应用】
 * - Kp：比例增益，将角度误差映射为角速度指令的基础刚度
 * - Ki：积分增益（默认0），保留接口但禁用（避免转向累积误差）
 * - Kd：微分增益，抑制转向过程中的超调和振荡
 * - max_integral：积分限幅（因Ki=0，实际无效）
 * - compensation：补偿值（角度环通常为0，保留结构统一性）
 * - max_output：输出限幅，防止角度环输出过大导致内环饱和
 * 
 * 【场景自适应】
 * 根据赛道元素动态调整控制增益（单边桥降低灵敏度防跌落）
 */
float Turn_Angle_Loop_Control(float angle_error)
{
    // 1. 误差赋值（注意：angle_error 已是 (期望-实际) 的差值）
    pid_turn_angle.error = angle_error;

    // 2. 积分项计算（保留完整结构，但因Ki=0实际无效）
    if (pid_turn_angle.ki != 0.0f) {
        pid_turn_angle.error_integral += pid_turn_angle.error;
        // 积分限幅保护
        pid_turn_angle.error_integral = Float_Constrain(
            pid_turn_angle.error_integral, 
            -pid_turn_angle.max_integral, 
            pid_turn_angle.max_integral
        );
    } else {
        pid_turn_angle.error_integral = 0.0f; // 显式清零确保无累积
    }

    // 3. 场景自适应增益调度（单边桥特殊处理）
    float kp_adj = pid_turn_angle.kp;
    float kd_adj = pid_turn_angle.kd;
    
    // if (danbianqiao_flag && danbianqiao_flag != 99) {
    //     kp_adj *= 0.7f;  // 单边桥降低Kp 30% 防跌落
    //     kd_adj *= 0.7f;  // 同比例缩放保持阻尼比
    // }
    // 可扩展：三级跳台阶，草地等场景的增益调整

    // 4. 完整PID计算（实际为PD，因Ki=0）
    float output_raw = (kp_adj * pid_turn_angle.error) + 
                       (pid_turn_angle.ki * pid_turn_angle.error_integral) + 
                       (kd_adj * (pid_turn_angle.error - pid_turn_angle.last_error));

    // 5. 输出限幅（防止角度环输出过大导致内环饱和）
    pid_turn_angle.output = Float_Constrain(
        output_raw, 
        -pid_turn_angle.max_output, 
        pid_turn_angle.max_output
    );

    // 6. 更新历史误差（为下一次微分计算准备）
    pid_turn_angle.prev_error = pid_turn_angle.last_error;
    pid_turn_angle.last_error = pid_turn_angle.error;

    return pid_turn_angle.output;  // 作为转向角速度环的目标值
}

// ============================================================================
//  转向角速度环控制函数 (内环 - 2ms周期) - 完整PID+死区补偿
// ============================================================================
/**
 * @brief 转向角速度环控制器（内环）- 完整PID+死区补偿实现
 * @param target_gyro 期望角速度（来自转向角度环，单位：°/s）
 * @param actual_gyro 实际角速度（来自IMU陀螺仪Z轴，单位：°/s）
 * @return 转向专用PWM值（直接叠加到电机驱动）
 * 
 * 【完整参数应用】
 * - Kp：比例增益，决定角速度跟踪的响应速度
 * - Ki：积分增益（默认0），高频环路禁用积分
 * - Kd：微分增益，抑制高频抖动和电机噪声
 * - max_integral：积分限幅（因Ki=0，实际无效）
 * - compensation：死区补偿电压（关键！克服转向电机静摩擦）
 * - max_output：动态输出限幅（普通赛道/单边桥双阈值）
 * 
 * 【传感器说明】
 * - 陀螺仪Z轴（gyro_z）对应偏航角速度（yaw rate），即车体旋转速度
 * - 符号约定：需根据实际安装方向调整（示例中使用负号匹配物理方向）
 */
float Turn_Gyro_Loop_Control(float target_gyro, float actual_gyro)
{
    // 1. 计算角速度误差
    pid_turn_gyro.error = target_gyro - actual_gyro;

    // 2. 积分项计算（保留完整结构，但因Ki=0实际无效）
    if (pid_turn_gyro.ki != 0.0f) {
        pid_turn_gyro.error_integral += pid_turn_gyro.error;
        // 积分限幅保护
        pid_turn_gyro.error_integral = Float_Constrain(
            pid_turn_gyro.error_integral, 
            -pid_turn_gyro.max_integral, 
            pid_turn_gyro.max_integral
        );
    } else {
        pid_turn_gyro.error_integral = 0.0f; // 显式清零确保无累积
    }

    // 3. 完整PD计算（实际为PD，因Ki=0）
    float output_raw = (pid_turn_gyro.kp * pid_turn_gyro.error) + 
                       (pid_turn_gyro.ki * pid_turn_gyro.error_integral) + 
                       (pid_turn_gyro.kd * (pid_turn_gyro.error - pid_turn_gyro.last_error));

    // 4. 死区补偿（关键！克服转向电机静摩擦）
    // 原理：当输出意图非零时，叠加最小启动电压使电机立即响应
    if (output_raw > 0) {
        output_raw += pid_turn_gyro.compensation; // 正转加死区
    } else if (output_raw < 0) {
        output_raw -= pid_turn_gyro.compensation; // 反转减死区
    }
    // 注意：output_raw=0时不做补偿，避免零点漂移

    // 5. 动态输出限幅（根据赛道类型切换阈值）
    // float max_output = danbianqiao_flag ? TURN_GYR_MAX_O_BRIDGE : pid_turn_gyro.max_output;//单边桥情形下的示例
    pid_turn_gyro.output = Float_Constrain(output_raw, -pid_turn_gyro.max_output, pid_turn_gyro.max_output);

    // 6. 更新历史误差（为下一次微分计算准备）
    pid_turn_gyro.prev_error = pid_turn_gyro.last_error;
    pid_turn_gyro.last_error = pid_turn_gyro.error;

    return pid_turn_gyro.output;
}



//内部静态变量，用于舵机速度环的滤波
static float servo_speed_last = 0.0f;
static float servo_speed_prelast = 0.0f;
/**
 * @brief 舵机速度闭环控制器 (移植并使用 PID_Param_t 结构)
 * @param target_speed 目标速度
 * @param actual_speed 实际速度 (来自编码器)
 * @param actual_angle 当前姿态角度 (来自IMU)
 * @return 姿态调整量 (例如，需要前倾/后仰的角度)
 */
float Servo_Speed_Control(float target_speed, float actual_speed, float actual_angle)
{
    // 1. 输入滤波
    float speed_now = actual_speed * 0.6f + servo_speed_last * 0.3f + servo_speed_prelast * 0.1f;
    servo_speed_prelast = servo_speed_last;
    servo_speed_last = speed_now;

    // 2. 动态速度规划 (移植思想)
    float speed_qiwang_now = target_speed; // 默认使用传入的目标速度
    // --- 【核心智能化决策区】 ---
    // 在这里，您需要根据摄像头的赛道信息、陀螺仪姿态等来动态修改 speed_qiwang_now
    // 例如：
    // if (is_in_big_turn()) {
    //     speed_qiwang_now = 100.0f; // 弯道减速
    // } else if (is_in_long_straight()) {
    //     speed_qiwang_now = 300.0f; // 直道加速
    // }

    // 3. 计算误差
    pid_servo_speed.error = speed_qiwang_now - speed_now;

    // 4. 自适应 Kp
    float k, adaptive_kp;
    float e = expf(-fabsf(pid_servo_speed.error / 10.0f)); // 调整分母灵敏度
    k = ((1.0f - e) / (1.0f + e)) * 0.6f + 0.4f; // k 在 [0.4, 1.0] 之间
    adaptive_kp = pid_servo_speed.kp * k;

    // 5. 位置式 PID 计算
    // 积分项 & 积分限幅
    // 只有在接近机械零点的情况下，才使用积分项，修复起来的时候开始积分的问题
    // if (fabsf(actual_angle-ANG_MECH_ZERO) < 2.0f) {
        //pid_servo_speed.error_integral += pid_servo_speed.error;
    // }
    // else{
    //     pid_servo_speed.error_integral = 0.0f;
    // }
    if (fabsf(pid_servo_speed.error) < 100.0f) {
        pid_servo_speed.error_integral += pid_servo_speed.error;
    } else {
        pid_servo_speed.error_integral = 0.0f;
    }
    pid_servo_speed.error_integral = Float_Constrain(pid_servo_speed.error_integral, -pid_servo_speed.max_integral, pid_servo_speed.max_integral);

    // PID输出计算
    float output_raw = (adaptive_kp * pid_servo_speed.error) +
                       (pid_servo_speed.ki * pid_servo_speed.error_integral) +
                       (pid_servo_speed.kd * (pid_servo_speed.error - pid_servo_speed.last_error));

    // 6. 输出限幅与更新
    pid_servo_speed.output = Float_Constrain(output_raw, -pid_servo_speed.max_output, pid_servo_speed.max_output);
    
    // 更新历史误差 (prev_error 也更新，保持结构完整性)
    pid_servo_speed.prev_error = pid_servo_speed.last_error;
    pid_servo_speed.last_error = pid_servo_speed.error;

    return pid_servo_speed.output;
}


/**
 * @brief 速度环控制 (外环)无刷电机
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
    //pid_angle.prev_error = pid_angle.last_error;//预留给增量式pid，现在注释掉,想用的时候可以加上
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
    //pid_gyro.prev_error = pid_gyro.last_error;//预留给增量式pid，现在注释掉,想用的时候可以加上
    pid_gyro.last_error = pid_gyro.error;

    return pid_gyro.output;
}

/**
 * @brief Rolling 自适应平衡控制，主要用于单边桥
 * @param actual_roll 当前横滚角 (单位: 度, 右高左低为正)
 * @return float 计算出的单侧缩短量 (PWM值, 总是 >= 0)
 * @note 此函数应在 5ms 定时器中调用
 */
float Roll_Balance_Control(float actual_roll,float target_roll)
{
    // 0. 安全检查
    if (roll_balance_enable == 0) {
        g_target_pwm_roll_adj = 0; // 这里的含义稍后解释
        return 0.0f;
    }

    // 1. 计算误差 (目标 - 实际)
    // 目标是 0 度
    float error = target_roll - actual_roll; 

    // 2. 计算 PD 输出 (标准 PID 公式)
    // 注意：这里计算的是一个“总矫正力”，正负代表方向
    float p_out = pid_roll.kp * error;
    float d_out = pid_roll.kd * (error - pid_roll.last_error);
    
    pid_roll.last_error = error; // 更新历史误差
    
    float total_out = p_out + d_out;
    
    // 限幅
    total_out = Float_Constrain(total_out, -pid_roll.max_output, pid_roll.max_output);
    
    // 3. 将总输出转换为 "一边不动，一边缩短" 的逻辑
    // total_out 的物理含义：
    // 如果 roll > 0 (右高)，error < 0，total_out < 0。我们需要缩短右腿。
    // 如果 roll < 0 (左高)，error > 0，total_out > 0。我们需要缩短左腿。
    
    // 我们约定 g_target_pwm_roll_adj 的含义：
    // 这个变量不再直接加减，而是作为一个“带符号的缩短量”传递给 servo_executor。
    // > 0 : 表示左侧需要缩短 (值越大缩得越多)
    // < 0 : 表示右侧需要缩短 (绝对值越大缩得越多)
    // = 0 : 大家都不动
    
    g_target_pwm_roll_adj = (int16)total_out; 
    
    return total_out;
}