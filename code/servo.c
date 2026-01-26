#include "servo.h"

// 内部静态变量：存储当前四个舵机的实际角度（经过限幅处理后的真实值）
// 索引：[0]:RF, [1]:RR, [2]:LF, [3]:LR
static float current_angles[4] = {90.0f, 90.0f, 90.0f, 90.0f};

/**
 * @brief 内部工具函数：通用逻辑角度转占空比 (已经优化掉了，这个后面不再使用，改为使用duty调用舵机)
 */
// static uint32 _angle_to_duty(float angle)
// {
//     return (uint32)(3000.0f * (0.5f + (angle / 90.0f)));
// }

/**
 * @brief 内部工具函数：通用占空比转逻辑角度 (仅供初始化函数使用，保持原样)
 */
static float _duty_to_angle(uint32 duty)
{
    return ((float)duty / 3000.0f - 0.5f) * 90.0f;
}

/**
 * @brief 初始化所有舵机 (完全保持原样，不改变初始化逻辑)
 */
void servo_init_all(void)
{
    // 右前 RF 和 左后 LR
    pwm_init(SERVO_MOTOR_PWM1, SERVO_FREQ, SERVO_MOTOR_PWM1_MID); 
    pwm_init(SERVO_MOTOR_PWM4, SERVO_FREQ, SERVO_MOTOR_PWM4_MID); 
    
    // 右后 RR 和 左前 LF
    pwm_init(SERVO_MOTOR_PWM2, SERVO_FREQ, SERVO_MOTOR_PWM2_MID); 
    pwm_init(SERVO_MOTOR_PWM3, SERVO_FREQ, SERVO_MOTOR_PWM3_MID); 
    
    // 更新内部记录数组 (使用通用估算值，初始化后第一次运动会自动修正)
    current_angles[0] = _duty_to_angle(SERVO_MOTOR_PWM1_MID);
    current_angles[1] = _duty_to_angle(SERVO_MOTOR_PWM2_MID);
    current_angles[2] = _duty_to_angle(SERVO_MOTOR_PWM3_MID);
    current_angles[3] = _duty_to_angle(SERVO_MOTOR_PWM4_MID);
}

/**
 * @brief 执行收腿动作
 */
void action_contract_legs(void)
{
    servo_write_angle(SERVO_MOTOR_PWM1, 105.0f); // 右前
    servo_write_angle(SERVO_MOTOR_PWM4, 105.0f); // 左后
    servo_write_angle(SERVO_MOTOR_PWM2, 75.0f);  // 右后
    servo_write_angle(SERVO_MOTOR_PWM3, 75.0f);  // 左前
}

/**
 * @brief 写入舵机角度 (已修改：针对每一条腿单独撰写，利用90度中位值计算)
 * @param ch    通道宏定义
 * @param angle 目标角度
 */
void servo_write_angle(pwm_channel_enum ch, float angle)
{
    int32 duty_calc = 0; // 使用有符号整型中间变量，防止计算过程溢出
    float slope = 33.3333f; // 斜率 k = 3000 / 90

    // 公式说明: Target_Duty = Mid_90_Duty + (Target_Angle - 90) * Slope

    if (ch == SERVO_MOTOR_PWM1)
    {
        // === 腿 1 (PWM1) 独立逻辑 ===
        // 1. 计算占空比 (基于 SERVO_MOTOR_PWM1_90)
        duty_calc = SERVO_MOTOR_PWM1_90 + (int32)((angle - 90.0f) * slope);

        // 2. 独立限幅 (使用 RF 限幅参数，保持原逻辑映射)
        if (duty_calc < RF_LIMIT_DUTY_MIN) duty_calc = RF_LIMIT_DUTY_MIN;
        if (duty_calc > RF_LIMIT_DUTY_MAX) duty_calc = RF_LIMIT_DUTY_MAX;

        // 3. 反向更新实际角度 (Angle = 90 + (Duty - Mid) / Slope)
        current_angles[0] = 90.0f + ((float)duty_calc - SERVO_MOTOR_PWM1_90) / slope;
        
        // 4. 输出
        pwm_set_duty(SERVO_MOTOR_PWM1, (uint32)duty_calc);
    }
    else if (ch == SERVO_MOTOR_PWM2)
    {
        // === 腿 2 (PWM2) 独立逻辑 ===
        duty_calc = SERVO_MOTOR_PWM2_90 + (int32)((angle - 90.0f) * slope);

        if (duty_calc < RR_LIMIT_DUTY_MIN) duty_calc = RR_LIMIT_DUTY_MIN;
        if (duty_calc > RR_LIMIT_DUTY_MAX) duty_calc = RR_LIMIT_DUTY_MAX;

        current_angles[1] = 90.0f + ((float)duty_calc - SERVO_MOTOR_PWM2_90) / slope;
        
        pwm_set_duty(SERVO_MOTOR_PWM2, (uint32)duty_calc);
    }
    else if (ch == SERVO_MOTOR_PWM3)
    {
        // === 腿 3 (PWM3) 独立逻辑 ===
        duty_calc = SERVO_MOTOR_PWM3_90 + (int32)((angle - 90.0f) * slope);

        if (duty_calc < LF_LIMIT_DUTY_MIN) duty_calc = LF_LIMIT_DUTY_MIN;
        if (duty_calc > LF_LIMIT_DUTY_MAX) duty_calc = LF_LIMIT_DUTY_MAX;

        current_angles[2] = 90.0f + ((float)duty_calc - SERVO_MOTOR_PWM3_90) / slope;
        
        pwm_set_duty(SERVO_MOTOR_PWM3, (uint32)duty_calc);
    }
    else if (ch == SERVO_MOTOR_PWM4)
    {
        // === 腿 4 (PWM4) 独立逻辑 ===
        duty_calc = SERVO_MOTOR_PWM4_90 + (int32)((angle - 90.0f) * slope);

        if (duty_calc < LR_LIMIT_DUTY_MIN) duty_calc = LR_LIMIT_DUTY_MIN;
        if (duty_calc > LR_LIMIT_DUTY_MAX) duty_calc = LR_LIMIT_DUTY_MAX;

        current_angles[3] = 90.0f + ((float)duty_calc - SERVO_MOTOR_PWM4_90) / slope;
        
        pwm_set_duty(SERVO_MOTOR_PWM4, (uint32)duty_calc);
    }
}

/**
 * @brief 获取当前四个舵机的物理角度
 */
void servo_get_current_angles(float *angles_array)
{
    if (angles_array == (void*)0) return;
    
    angles_array[0] = current_angles[0]; 
    angles_array[1] = current_angles[1]; 
    angles_array[2] = current_angles[2]; 
    angles_array[3] = current_angles[3]; 
}