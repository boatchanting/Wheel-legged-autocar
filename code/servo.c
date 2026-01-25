#include "servo.h"

// 内部静态变量：存储当前四个舵机的实际角度（经过限幅处理后的真实值）
// 索引：[0]:RF, [1]:RR, [2]:LF, [3]:LR
static float current_angles[4] = {90.0f, 90.0f, 90.0f, 90.0f};

/**
 * @brief 内部工具函数：将逻辑角度(0-180)转换为硬件占空比(Duty)
 * @note  在 300Hz 频率下，PWM_DUTY_MAX 为 10000 时，计算公式如下：
 */
static uint32 _angle_to_duty(float angle)
{
    // 计算公式推导：
    // 300Hz 周期约为 3.33ms。
    // 标准舵机：0.5ms->0度, 1.5ms->90度, 2.5ms->180度
    // 换算公式：Duty = (10000 / 3.33ms) * (0.5ms + (angle/90)*1.0ms)
    // 简化后约为：3000 * (0.5 + angle/90)
    return (uint32)(3000.0f * (0.5f + (angle / 90.0f)));
}

/**
 * @brief 内部工具函数：将占空比 (Duty) 逆向换算回逻辑角度
 */
static float _duty_to_angle(uint32 duty)
{
    // 逆运算公式：angle = ((duty / 3000) - 0.5) * 90
    return ((float)duty / 3000.0f - 0.5f) * 90.0f;
}

/**
 * @brief 初始化所有舵机并执行“收腿”动作
 * @note  根据原始定义：PWM1/PWM4 为 5000，PWM2/PWM3 为 4000
 */
void servo_init_all(void)
{
    // 定义初始收腿状态的占空比
    uint32 duty_contract_high = 4500; // 对应 105 度
    uint32 duty_contract_low  = 4500; // 对应 75 度
    
    // 1. 初始化外设并直接设置初始占空比 (逐飞科技开源库接口)
    // 右前 RF 和 左后 LR
    pwm_init(SERVO_MOTOR_PWM1, SERVO_FREQ, SERVO_MOTOR_PWM1_MID); 
    pwm_init(SERVO_MOTOR_PWM4, SERVO_FREQ, SERVO_MOTOR_PWM4_MID); 
    
    // 右后 RR 和 左前 LF
    pwm_init(SERVO_MOTOR_PWM2, SERVO_FREQ, SERVO_MOTOR_PWM2_MID); 
    pwm_init(SERVO_MOTOR_PWM3, SERVO_FREQ, SERVO_MOTOR_PWM3_MID); 
    
    
    // 2. 更新内部记录数组 current_angles
    // 使用 _duty_to_angle 确保角度记录与实际输出的 Duty 严格同步
    current_angles[0] = _duty_to_angle(duty_contract_high); // RF (PWM1)
    current_angles[1] = _duty_to_angle(duty_contract_low);  // RR (PWM2)
    current_angles[2] = _duty_to_angle(duty_contract_low);  // LF (PWM3)
    current_angles[3] = _duty_to_angle(duty_contract_high); // LR (PWM4)
}

/**
 * @brief 执行收腿动作（封装为函数，方便后续调用）
 */
void action_contract_legs(void)
{
    // 直接调用写角度函数，会自动触发限幅保护
    // 5000 duty -> 105.0f, 4000 duty -> 75.0f
    servo_write_angle(SERVO_MOTOR_PWM1, 105.0f); // 右前
    servo_write_angle(SERVO_MOTOR_PWM4, 105.0f); // 左后
    servo_write_angle(SERVO_MOTOR_PWM2, 75.0f);  // 右后
    servo_write_angle(SERVO_MOTOR_PWM3, 75.0f);  // 左前
}

/**
 * @brief 写入舵机角度并执行8变量硬件级占空比限幅保护
 * @param ch    通道宏定义 (SERVO_MOTOR_PWM1 ~ PWM4)
 * @param angle 目标角度 (0.0 - 180.0)
 */
void servo_write_angle(pwm_channel_enum ch, float angle)
{
    uint32 duty = _angle_to_duty(angle);
    uint32 min_limit, max_limit;
    uint8 index;

    // 1. 匹配对应通道的限幅变量与存储索引
    if      (ch == SERVO_MOTOR_PWM1) { min_limit = RF_LIMIT_DUTY_MIN; max_limit = RF_LIMIT_DUTY_MAX; index = 0; }
    else if (ch == SERVO_MOTOR_PWM2) { min_limit = RR_LIMIT_DUTY_MIN; max_limit = RR_LIMIT_DUTY_MAX; index = 1; }
    else if (ch == SERVO_MOTOR_PWM3) { min_limit = LF_LIMIT_DUTY_MIN; max_limit = LF_LIMIT_DUTY_MAX; index = 2; }
    else if (ch == SERVO_MOTOR_PWM4) { min_limit = LR_LIMIT_DUTY_MIN; max_limit = LR_LIMIT_DUTY_MAX; index = 3; }
    else return;

    // 2. 硬件级安全拦截：强制截断超出安全边界的 Duty
    if (duty < min_limit) duty = min_limit;
    if (duty > max_limit) duty = max_limit;

    // 3. 更新当前角度记录 (存储限幅截断后的真实物理角度)
    current_angles[index] = _duty_to_angle(duty);

    // 4. 执行 PWM 输出
    pwm_set_duty(ch, duty);
}

/**
 * @brief 机器人联动控制逻辑
 * @param base_angle 基础参考角度 (输入90中位，输入70执行联动收腿)
 * @note  根据要求：90->70度时
 *        RF(PWM1) 往下转 | LF(PWM3) 往上转 | LR(PWM4) 往下转 | RR(PWM2) 往上转
 */
void robot_posture_control(float base_angle)
{
    float offset = base_angle - 90.0f; // 计算相对中位的偏移量

    // 联动逻辑映射 (已内部集成了8变量限幅保护)
    servo_write_angle(SERVO_MOTOR_PWM1, 90.0f + offset); // 右前 (90-20=70)
    servo_write_angle(SERVO_MOTOR_PWM4, 90.0f + offset); // 左后 (90-20=70)
    
    servo_write_angle(SERVO_MOTOR_PWM3, 90.0f - offset); // 左前 (90+20=110)
    servo_write_angle(SERVO_MOTOR_PWM2, 90.0f - offset); // 右后 (90+20=110)
}

/**
 * @brief 获取当前四个舵机的物理角度（供显示或PID计算使用）
 * @param angles_array 用于存放结果的数组指针 (长度须 >= 4)
 * @note 数组内容对应：[0]=RF, [1]=RR, [2]=LF, [3]=LR
 */
void servo_get_current_angles(float *angles_array)
{
    if (angles_array == (void*)0) return;
    
    angles_array[0] = current_angles[0]; // RF
    angles_array[1] = current_angles[1]; // RR
    angles_array[2] = current_angles[2]; // LF
    angles_array[3] = current_angles[3]; // LR
}