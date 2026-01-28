#include "servo_executor.h"
// --- 目标值定义 ---
volatile int16 g_target_pwm_high = 0;
volatile int16 g_target_pwm_speed_adj = 0;
volatile int16 g_target_pwm_angle_adj = 0;

// --- 执行器的内部状态变量 (static封装) ---
static int32 PWM_CH1_LAST, PWM_CH2_LAST, PWM_CH3_LAST, PWM_CH4_LAST;

/**
 * @brief 初始化舵机执行器的内部状态
 */
void servo_executor_init(void)
{
    // 从查表或宏定义中获取初始的90度中位占空比
    // 这里我们直接使用 servo_init_all 里的逻辑来保证一致性
    int16 initial_pwm_high;
    high_control_table(servo_height);
    initial_pwm_high = (pwm_high == 10000) ? 0 : pwm_high;

    // 执行 PWM 硬件初始化
    pwm_init(SERVO_MOTOR_PWM1, SERVO_FREQ, SERVO_MOTOR_PWM1_90 + SERVO_MOTOR_PWM1_DIR * initial_pwm_high); 
    pwm_init(SERVO_MOTOR_PWM2, SERVO_FREQ, SERVO_MOTOR_PWM2_90 + SERVO_MOTOR_PWM2_DIR * initial_pwm_high); 
    pwm_init(SERVO_MOTOR_PWM3, SERVO_FREQ, SERVO_MOTOR_PWM3_90 + SERVO_MOTOR_PWM3_DIR * initial_pwm_high); 
    pwm_init(SERVO_MOTOR_PWM4, SERVO_FREQ, SERVO_MOTOR_PWM4_90 + SERVO_MOTOR_PWM4_DIR * initial_pwm_high); 

    PWM_CH1_LAST = SERVO_MOTOR_PWM1_90 + SERVO_MOTOR_PWM1_DIR * initial_pwm_high;
    PWM_CH2_LAST = SERVO_MOTOR_PWM2_90 + SERVO_MOTOR_PWM2_DIR * initial_pwm_high;
    PWM_CH3_LAST = SERVO_MOTOR_PWM3_90 + SERVO_MOTOR_PWM3_DIR * initial_pwm_high;
    PWM_CH4_LAST = SERVO_MOTOR_PWM4_90 + SERVO_MOTOR_PWM4_DIR * initial_pwm_high;

    uint16_t initial_duties[4] = {
        (uint16_t)PWM_CH1_LAST,
        (uint16_t)PWM_CH2_LAST,
        (uint16_t)PWM_CH3_LAST,
        (uint16_t)PWM_CH4_LAST
    };
    update_all_servo_angles(initial_duties);//初始化完成后，更新舵机角度数组，舵机debug使用，优化时可以删除
}

/**
 * @brief 舵机执行器更新函数 (移植自您的 servo_control)
 */
// 定义斜率限制 (加速和减速限制)
int32 acc_limit = 10;  // <<-- 【需要您根据实际情况调整】
int32 dec_limit = 10;  // <<-- 【需要您根据实际情况调整】
void servo_executor_update(void)
{
    // 2.2 尝试计算目标高度分量
    high_control_table(servo_height);
    if (pwm_high != 10000)
    {
        // 只有查表成功时，才更新目标值
        g_target_pwm_high = pwm_high;

        // 2.4 (可选) 计算目标转向/姿态分量
        // g_target_pwm_angle_adj = calculate_steering_pid();
    }


    // 1. 计算基础姿态的目标值 (只与高度有关)
    int32 target_base_duty_lf = SERVO_MOTOR_PWM1_90 + SERVO_MOTOR_PWM1_DIR * g_target_pwm_high;
    int32 target_base_duty_rf = SERVO_MOTOR_PWM2_90 + SERVO_MOTOR_PWM2_DIR * g_target_pwm_high;
    int32 target_base_duty_rr = SERVO_MOTOR_PWM3_90 + SERVO_MOTOR_PWM3_DIR * g_target_pwm_high;
    int32 target_base_duty_lr = SERVO_MOTOR_PWM4_90 + SERVO_MOTOR_PWM4_DIR * g_target_pwm_high;

    // 2. 叠加速度调整量，得到最终的目标值
    // 模型: 前倾加速 = 前腿伸展 + 后腿收缩
    int32 target_final_duty_lf = target_base_duty_lf + SERVO_MOTOR_PWM1_DIR * g_target_pwm_speed_adj;
    int32 target_final_duty_rf = target_base_duty_rf + SERVO_MOTOR_PWM2_DIR * g_target_pwm_speed_adj;
    int32 target_final_duty_rr = target_base_duty_rr - SERVO_MOTOR_PWM3_DIR * g_target_pwm_speed_adj;
    int32 target_final_duty_lr = target_base_duty_lr - SERVO_MOTOR_PWM4_DIR * g_target_pwm_speed_adj;

    // 3. 叠加转向/姿态调整量 (例如单边桥逻辑)
    // 这里我们简化，假设 g_target_pwm_angle_adj 用于转向
    // 左转 (>0): 左侧收，右侧伸
    target_final_duty_lf -= g_target_pwm_angle_adj; // ++舵机, 收缩是减
    target_final_duty_rf -= g_target_pwm_angle_adj; // --舵机, 伸展是减
    target_final_duty_rr += g_target_pwm_angle_adj; // --舵机, 收缩是加
    target_final_duty_lr += g_target_pwm_angle_adj; // ++舵机, 伸展是加

    // 4. 【核心】使用斜率限制，平滑地趋近目标值
    // 公式: new = last + limit(target - last, -dec, +acc)
    int32 current_duty_lf = PWM_CH1_LAST + Float_Constrain(target_final_duty_lf - PWM_CH1_LAST, -dec_limit, acc_limit);
    int32 current_duty_rf = PWM_CH2_LAST + Float_Constrain(target_final_duty_rf - PWM_CH2_LAST, -dec_limit, acc_limit);
    int32 current_duty_rr = PWM_CH3_LAST + Float_Constrain(target_final_duty_rr - PWM_CH3_LAST, -dec_limit, acc_limit);
    int32 current_duty_lr = PWM_CH4_LAST + Float_Constrain(target_final_duty_lr - PWM_CH4_LAST, -dec_limit, acc_limit);

    // 5. 更新内部状态，为下一次计算做准备
    PWM_CH1_LAST = current_duty_lf;
    PWM_CH2_LAST = current_duty_rf;
    PWM_CH3_LAST = current_duty_rr;
    PWM_CH4_LAST = current_duty_lr;

    // 6. 调用底层驱动，直接写入硬件 (这里不再需要servo_write_duty, 因为限幅已在底层完成)
    // 注意：这里的最终限幅依然重要，作为最后一道防线
    pwm_set_duty(SERVO_MOTOR_PWM1, (uint32)Float_Constrain(current_duty_lf, LF_LIMIT_DUTY_MIN, LF_LIMIT_DUTY_MAX));
    pwm_set_duty(SERVO_MOTOR_PWM2, (uint32)Float_Constrain(current_duty_rf, RF_LIMIT_DUTY_MIN, RF_LIMIT_DUTY_MAX));
    pwm_set_duty(SERVO_MOTOR_PWM3, (uint32)Float_Constrain(current_duty_rr, RR_LIMIT_DUTY_MIN, RR_LIMIT_DUTY_MAX));
    pwm_set_duty(SERVO_MOTOR_PWM4, (uint32)Float_Constrain(current_duty_lr, LR_LIMIT_DUTY_MIN, LR_LIMIT_DUTY_MAX));

    uint16_t current_duties[4] = {
        (uint16_t)current_duty_lf,
        (uint16_t)current_duty_rf,
        (uint16_t)current_duty_rr,
        (uint16_t)current_duty_lr
    };
    update_all_servo_angles(current_duties);//更新舵机角度数组，舵机debug使用，优化时可以删除
}