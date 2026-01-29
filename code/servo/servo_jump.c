#include "servo_jump.h"

// 状态变量
uint8_t jump_flag = 0;
uint32_t jump_start_time = 0;
bool vision_detected_jump_point = false;//跳跃测试用
// 引用外部变量 (来自 main.c 或 servo.c)
extern volatile int32 PWM_CH1_LAST, PWM_CH2_LAST, PWM_CH3_LAST, PWM_CH4_LAST;
extern float servo_height; 
extern int16 pwm_high; // 查表后的高度duty基准

// 初始化
void jump_module_init(void)
{
    jump_flag = 0;
}

// 触发逻辑：记录当前时间
void jump_trigger(void)
{
    if(jump_flag == 0)
    {
        jump_flag = 1;
        jump_start_time = loop_counter; // 锚定当前毫秒时间戳
    }
}

/**
 * @brief 核心解算器：根据极性计算目标占空比
 * @param base_90     90度中位 Duty
 * @param dir         方向极性 (1 或 -1)
 * @param height_duty 基础身高 Duty
 * @param offset_duty 动作偏移 Duty (+伸腿, -收腿)
 */
static int32 get_joint_target(int32 base_90, int8_t dir, int32 height_duty, int32 offset_duty)
{
    // 公式解析：
    // 1. (height_duty + offset_duty) 计算出总的“机械伸长量”
    // 2. 乘以 dir：
    //    - 如果 dir=1 (LF/LR): 向下为正，Duty 增加。
    //    - 如果 dir=-1(RF/RR): 向下为正，Duty 减小。
    // 3. 加上 base_90 基础值。
    return base_90 + (dir * (height_duty + offset_duty));
}

/**
 * @brief 跳跃执行器 (需在定时器或主循环中调用)
 */
void servo_jump_executor(void)
{
    int32 target_lf, target_rf, target_rr, target_lr;
    int32 dynamic_slope_limit; // 动态斜率限制 (决定电机响应速度)
    
    // 1. 计算当前时刻 (ms)
    uint32_t time_elapsed = loop_counter - jump_start_time;

    // 2. 获取基础身高分量
    high_control_table(servo_height); 
    int32 h_duty = (pwm_high == 10000) ? 0 : pwm_high;

    // ===================== 时序状态机 =====================
    
    // --- 阶段 A: 爆发起跳 (0 - 100ms) ---
    if (time_elapsed <= 100)
    {
        // 动作：全力伸腿
        // 限幅：极大值 (10000)，相当于无视斜率限制，电机全速动作
        dynamic_slope_limit = 10000; 
        
        target_lf = get_joint_target(SERVO_MOTOR_PWM1_90, SERVO_MOTOR_PWM1_DIR, h_duty, JUMP_OFFSET_LAUNCH);
        target_rf = get_joint_target(SERVO_MOTOR_PWM2_90, SERVO_MOTOR_PWM2_DIR, h_duty, JUMP_OFFSET_LAUNCH);
        target_rr = get_joint_target(SERVO_MOTOR_PWM3_90, SERVO_MOTOR_PWM3_DIR, h_duty, JUMP_OFFSET_LAUNCH);
        target_lr = get_joint_target(SERVO_MOTOR_PWM4_90, SERVO_MOTOR_PWM4_DIR, h_duty, JUMP_OFFSET_LAUNCH);
    }
    // --- 阶段 B: 空中收腿 (100 - 280ms) ---
    else if (time_elapsed <= 280)
    {
        // 动作：快速收缩
        dynamic_slope_limit = 10000;
        
        target_lf = get_joint_target(SERVO_MOTOR_PWM1_90, SERVO_MOTOR_PWM1_DIR, h_duty, JUMP_OFFSET_FLIGHT);
        target_rf = get_joint_target(SERVO_MOTOR_PWM2_90, SERVO_MOTOR_PWM2_DIR, h_duty, JUMP_OFFSET_FLIGHT);
        target_rr = get_joint_target(SERVO_MOTOR_PWM3_90, SERVO_MOTOR_PWM3_DIR, h_duty, JUMP_OFFSET_FLIGHT);
        target_lr = get_joint_target(SERVO_MOTOR_PWM4_90, SERVO_MOTOR_PWM4_DIR, h_duty, JUMP_OFFSET_FLIGHT);
    }
    // --- 阶段 C: 落地准备 (280 - 300ms) ---
    else if (time_elapsed <= 300)
    {
        // 动作：伸腿准备触地
        dynamic_slope_limit = 10000;
        
        target_lf = get_joint_target(SERVO_MOTOR_PWM1_90, SERVO_MOTOR_PWM1_DIR, h_duty, JUMP_OFFSET_LAND);
        target_rf = get_joint_target(SERVO_MOTOR_PWM2_90, SERVO_MOTOR_PWM2_DIR, h_duty, JUMP_OFFSET_LAND);
        target_rr = get_joint_target(SERVO_MOTOR_PWM3_90, SERVO_MOTOR_PWM3_DIR, h_duty, JUMP_OFFSET_LAND);
        target_lr = get_joint_target(SERVO_MOTOR_PWM4_90, SERVO_MOTOR_PWM4_DIR, h_duty, JUMP_OFFSET_LAND);
    }
    // --- 阶段 D: 缓冲恢复 (300 - 360ms) ---
    else if (time_elapsed <= 360)
    {
        // 动作：恢复到正常身高 (Offset = 0)
        // 限幅：【关键】设为 20 左右，模拟弹簧阻尼
        // 这会让腿“慢慢”缩回到正常高度，消化地面的冲击力
        dynamic_slope_limit = 20; 
        
        target_lf = get_joint_target(SERVO_MOTOR_PWM1_90, SERVO_MOTOR_PWM1_DIR, h_duty, 0);
        target_rf = get_joint_target(SERVO_MOTOR_PWM2_90, SERVO_MOTOR_PWM2_DIR, h_duty, 0);
        target_rr = get_joint_target(SERVO_MOTOR_PWM3_90, SERVO_MOTOR_PWM3_DIR, h_duty, 0);
        target_lr = get_joint_target(SERVO_MOTOR_PWM4_90, SERVO_MOTOR_PWM4_DIR, h_duty, 0);
    }
    // --- 阶段 E: 结束 ---
    else
    {
        jump_flag = 0; // 动作完成，交还控制权
        return;
    }

    // ===================== 输出与安全限幅 =====================
    
    // 1. 应用斜率限制 (Slope Limit)
    // 这一步决定了电机是从“当前位置”瞬移到“目标位置”，还是平滑过渡
    PWM_CH1_LAST += Float_Constrain(target_lf - PWM_CH1_LAST, -dynamic_slope_limit, dynamic_slope_limit);
    PWM_CH2_LAST += Float_Constrain(target_rf - PWM_CH2_LAST, -dynamic_slope_limit, dynamic_slope_limit);
    PWM_CH3_LAST += Float_Constrain(target_rr - PWM_CH3_LAST, -dynamic_slope_limit, dynamic_slope_limit);
    PWM_CH4_LAST += Float_Constrain(target_lr - PWM_CH4_LAST, -dynamic_slope_limit, dynamic_slope_limit);

    // 2. 硬件绝对安全限幅 (Hardware Clamp)
    // 确保无论怎么算，都不会烧坏舵机
    uint32 final_lf = (uint32)Float_Constrain(PWM_CH1_LAST, LF_LIMIT_DUTY_MIN, LF_LIMIT_DUTY_MAX);
    uint32 final_rf = (uint32)Float_Constrain(PWM_CH2_LAST, RF_LIMIT_DUTY_MIN, RF_LIMIT_DUTY_MAX);
    uint32 final_rr = (uint32)Float_Constrain(PWM_CH3_LAST, RR_LIMIT_DUTY_MIN, RR_LIMIT_DUTY_MAX);
    uint32 final_lr = (uint32)Float_Constrain(PWM_CH4_LAST, LR_LIMIT_DUTY_MIN, LR_LIMIT_DUTY_MAX);

    // 3. 写入寄存器
    pwm_set_duty(SERVO_MOTOR_PWM1, final_lf);
    pwm_set_duty(SERVO_MOTOR_PWM2, final_rf);
    pwm_set_duty(SERVO_MOTOR_PWM3, final_rr);
    pwm_set_duty(SERVO_MOTOR_PWM4, final_lr);

    // 4. 更新角度数组 (用于Debug显示)
    uint16_t current_duties[4] = {(uint16_t)final_lf, (uint16_t)final_rf, (uint16_t)final_rr, (uint16_t)final_lr};
    update_all_servo_angles(current_duties);
}