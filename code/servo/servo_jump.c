#include "servo_jump.h"
#include "servo_executor.h"
#include "../config/car_select.h"//根据小车选择配置不同的跳跃参数
// 状态变量
uint8_t jump_flag = 0;
uint32_t jump_start_time = 0;
volatile JumpPhase g_current_jump_phase = JUMP_PHASE_NONE; // 初始化为 NONE
JumpType_e g_current_jump_type = JUMP_TYPE_NORMAL;
JumpProfile_t g_jump_profile; // 当前正在执行的跳跃参数
bool vision_detected_jump_point = false;//跳跃测试用
// 引用外部变量 (来自servo.c)
extern volatile int32 PWM_CH1_LAST, PWM_CH2_LAST, PWM_CH3_LAST, PWM_CH4_LAST;
extern float servo_height; 
extern int16 pwm_high; // 查表后的高度duty基准
// 动量轮私有变量
float g_air_kp;
float g_air_kd;
float g_air_target_pitch;
// ===================== 参数加载器 =====================
uint32_t time_elapsed1, time_elapsed2, time_elapsed3, time_elapsed4=0; // 距离起跳的时间 (ms)
/**
 * @brief 根据跳跃类型加载对应的动作参数
 * @param type 跳跃类型
 * @param current_height 起跳时的当前身高，用于平地跳恢复身高
 */
static void load_jump_profile(JumpType_e type, float current_height)
{
    switch(type) {
        //以下为0车的参数，其他车需要调参
        #if CAR_SELECT == 0 // 0代表学习板小车 板子 学习板 v1.2
        case JUMP_TYPE_HURDLE: // 【跨杆模式】
            g_jump_profile.t_launch = 100;
            g_jump_profile.t_flight = 320; // 跨杆需要更长的滞空收腿时间
            g_jump_profile.t_landing = 350;
            g_jump_profile.t_recovery = 420;
            
            g_jump_profile.offset_launch = 3000;  // 极限发力
            g_jump_profile.offset_flight = -3000; // 【防挂杆】极限收腿
            g_jump_profile.offset_land = 1000;           
            g_jump_profile.air_target_pitch = -1.0f; // 空中保持绝对水平
            g_jump_profile.post_jump_height = current_height; // 落地高度不变
            break;
            
        case JUMP_TYPE_STEP_UP: // 【上台阶模式】
            g_jump_profile.t_launch = 110; 
            g_jump_profile.t_flight = 220; // 【高度截断】台阶高，提前触地，腾空时间缩短
            g_jump_profile.t_landing = 250;
            g_jump_profile.t_recovery = 330;
            g_jump_profile.offset_launch = 3000;  // 更强发力获取高度
            g_jump_profile.offset_flight = -1500; // 适度收腿即可
            g_jump_profile.offset_land = 1000;
            g_jump_profile.air_target_pitch = -1.0f;
            g_jump_profile.post_jump_height = current_height;  // 【重心控制】跳上台阶后，调低基准身高防止摔倒 (需调参)
            break;
            
        case JUMP_TYPE_NORMAL: // 【普通平地跳】
        default:
            g_jump_profile.t_launch = 100;
            g_jump_profile.t_flight = 200;
            g_jump_profile.t_landing = 220;
            g_jump_profile.t_recovery = 280;
            g_jump_profile.offset_launch = 2700; 
            g_jump_profile.offset_flight = -1500;
            g_jump_profile.offset_land = 1700;
            g_jump_profile.air_target_pitch = -1.0f; // 默认轻微低头
            g_jump_profile.post_jump_height = current_height; // 落地高度不变
            break;
        #endif
          //以下为0车的参数，其他车需要调参
        #if CAR_SELECT == 3 // // 3代表 【2026/3/16带壳新车】 对应板子 【2026/02/15 锦鲤队】
        case JUMP_TYPE_HURDLE: // 【跨杆模式】
            g_jump_profile.t_launch = 100;
            g_jump_profile.t_flight = 320; // 跨杆需要更长的滞空收腿时间
            g_jump_profile.t_landing = 350;
            g_jump_profile.t_recovery = 420;
            
            g_jump_profile.offset_launch = 3000;  // 极限发力
            g_jump_profile.offset_flight = -3000; // 【防挂杆】极限收腿
            g_jump_profile.offset_land = 1000;           
            g_jump_profile.air_target_pitch = -1.0f; // 空中保持绝对水平
            g_jump_profile.post_jump_height = current_height; // 落地高度不变
            break;
            
        case JUMP_TYPE_STEP_UP: // 【上台阶模式】
            g_jump_profile.t_launch = 110; 
            g_jump_profile.t_flight = 220; // 【高度截断】台阶高，提前触地，腾空时间缩短
            g_jump_profile.t_landing = 250;
            g_jump_profile.t_recovery = 330;
            g_jump_profile.offset_launch = 3000;  // 更强发力获取高度
            g_jump_profile.offset_flight = -1500; // 适度收腿即可
            g_jump_profile.offset_land = 1000;
            g_jump_profile.air_target_pitch = -1.0f;
            g_jump_profile.post_jump_height = current_height;  // 【重心控制】跳上台阶后，调低基准身高防止摔倒 (需调参)
            break;
            
        case JUMP_TYPE_NORMAL: // 【普通平地跳】
        default:
            g_jump_profile.t_launch = 100;
            g_jump_profile.t_flight = 200;
            g_jump_profile.t_landing = 220;
            g_jump_profile.t_recovery = 280;
            g_jump_profile.offset_launch = 2700; 
            g_jump_profile.offset_flight = -1500;
            g_jump_profile.offset_land = 1700;
            g_jump_profile.air_target_pitch = -1.0f; // 默认轻微低头
            g_jump_profile.post_jump_height = current_height; // 落地高度不变
            break;
        #endif
    }
    
    // 同步更新动量轮的控制目标
    g_air_target_pitch = g_jump_profile.air_target_pitch;
}

// 触发逻辑：记录当前时间
void jump_trigger(void)
{
    jump_trigger_with_type(JUMP_TYPE_NORMAL);
}

void jump_trigger_with_type(JumpType_e type)
{
    if(jump_flag == 0)
    {
        jump_flag = 1;
        jump_start_time = loop_counter; // 锚定当前毫秒时间戳
        g_current_jump_phase = JUMP_PHASE_LAUNCH; // 初始阶段设为 A
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
    uint16_t current_duties_jump[4] = {
        (uint16_t)current_duty_lf,
        (uint16_t)current_duty_rf,
        (uint16_t)current_duty_rr,
        (uint16_t)current_duty_lr
    };
    // 1. 计算当前时刻 (ms)
    uint32_t time_elapsed = loop_counter - jump_start_time;

    // 2. 获取基础身高分量
    high_control_table(servo_height); 
    int32 h_duty = (pwm_high == 10000) ? 0 : pwm_high;

    // ===================== 时序状态机 =====================
    
    // --- 阶段 A: 爆发起跳 (0 - g_jump_profile.t_launch) ---
    if (time_elapsed <= g_jump_profile.t_launch)
    {
        // 动作：全力伸腿
        // 限幅：极大值 (10000)，相当于无视斜率限制，电机全速动作
        g_current_jump_phase = JUMP_PHASE_LAUNCH; // 更新阶段
        dynamic_slope_limit = 10000; 
        
        target_lf = get_joint_target(current_duties_jump[0], SERVO_MOTOR_PWM1_DIR, h_duty, g_jump_profile.offset_launch);
        target_rf = get_joint_target(current_duties_jump[1], SERVO_MOTOR_PWM2_DIR, h_duty, g_jump_profile.offset_launch);
        target_rr = get_joint_target(current_duties_jump[2], SERVO_MOTOR_PWM3_DIR, h_duty, g_jump_profile.offset_launch);
        target_lr = get_joint_target(current_duties_jump[3], SERVO_MOTOR_PWM4_DIR, h_duty, g_jump_profile.offset_launch);

    }
    // --- 阶段 B: 空中收腿 (g_jump_profile.t_launch - g_jump_profile.t_flight) ---
    else if (time_elapsed <= g_jump_profile.t_flight)
    {
        // 动作：快速收缩
        g_current_jump_phase = JUMP_PHASE_FLIGHT;
        dynamic_slope_limit = 10000;
        
        target_lf = get_joint_target(current_duties_jump[0], SERVO_MOTOR_PWM1_DIR, h_duty, g_jump_profile.offset_flight);
        target_rf = get_joint_target(current_duties_jump[1], SERVO_MOTOR_PWM2_DIR, h_duty, g_jump_profile.offset_flight);
        target_rr = get_joint_target(current_duties_jump[2], SERVO_MOTOR_PWM3_DIR, h_duty, g_jump_profile.offset_flight);
        target_lr = get_joint_target(current_duties_jump[3], SERVO_MOTOR_PWM4_DIR, h_duty, g_jump_profile.offset_flight);
    }
    // --- 阶段 C: 落地准备 (g_jump_profile.t_flight - g_jump_profile.t_landing) ---
    else if (time_elapsed <= g_jump_profile.t_landing)
    {
        // 动作：伸腿准备触地
        g_current_jump_phase = JUMP_PHASE_LANDING;
        dynamic_slope_limit = 10000;
        
        target_lf = get_joint_target(current_duties_jump[0], SERVO_MOTOR_PWM1_DIR, h_duty, g_jump_profile.offset_land);
        target_rf = get_joint_target(current_duties_jump[1], SERVO_MOTOR_PWM2_DIR, h_duty, g_jump_profile.offset_land);
        target_rr = get_joint_target(current_duties_jump[2], SERVO_MOTOR_PWM3_DIR, h_duty, g_jump_profile.offset_land);
        target_lr = get_joint_target(current_duties_jump[3], SERVO_MOTOR_PWM4_DIR, h_duty, g_jump_profile.offset_land);
    }
    // --- 阶段 D: 缓冲恢复 (g_jump_profile.t_landing - g_jump_profile.t_recovery) ---
    else if (time_elapsed <= g_jump_profile.t_recovery)
    {
        // 动作：恢复到正常身高 (Offset = 0)
        // 限幅：【关键】设为 20 左右，模拟弹簧阻尼
        // 这会让腿“慢慢”缩回到正常高度，消化地面的冲击力
        g_current_jump_phase = JUMP_PHASE_RECOVERY;
        dynamic_slope_limit = 20; 
        
        target_lf = get_joint_target(current_duties_jump[0], SERVO_MOTOR_PWM1_DIR, h_duty, 0);
        target_rf = get_joint_target(current_duties_jump[1], SERVO_MOTOR_PWM2_DIR, h_duty, 0);
        target_rr = get_joint_target(current_duties_jump[2], SERVO_MOTOR_PWM3_DIR, h_duty, 0);
        target_lr = get_joint_target(current_duties_jump[3], SERVO_MOTOR_PWM4_DIR, h_duty, 0);
    }
    // --- 阶段 E: 结束 ---
    else
    {
        jump_flag = 0; // 动作完成，交还控制权
        g_current_jump_phase = JUMP_PHASE_NONE;
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

    // 5.检查舵机是否已执行完成
    // 注意：这里比较的是“目标值”和“实际输出值（final_x）”
    int32 err_lf = ABS((int32)target_lf - (int32)final_lf);
    int32 err_rf = ABS((int32)target_rf - (int32)final_rf);
    int32 err_rr = ABS((int32)target_rr - (int32)final_rr);
    int32 err_lr = ABS((int32)target_lr - (int32)final_lr);

    uint8_t all_reached = (err_lf <= TARGET_TOLERANCE) &&
                          (err_rf <= TARGET_TOLERANCE) &&
                          (err_rr <= TARGET_TOLERANCE) &&
                          (err_lr <= TARGET_TOLERANCE);

    // 根据当前阶段，记录首次完成时间（相对时间）
    if (all_reached) {
        if (g_current_jump_phase == JUMP_PHASE_LAUNCH && time_elapsed1 == 0) {
            time_elapsed1 = time_elapsed; // 相对时间
        }
        else if (g_current_jump_phase == JUMP_PHASE_FLIGHT && time_elapsed2 == 0) {
            time_elapsed2 = time_elapsed; // 相对时间
        }
        else if (g_current_jump_phase == JUMP_PHASE_LANDING && time_elapsed3 == 0) {
            time_elapsed3 = time_elapsed; // 相对时间
        }
        else if (g_current_jump_phase == JUMP_PHASE_RECOVERY && time_elapsed4 == 0) {
            time_elapsed4 = time_elapsed; // 相对时间
        }
    }
}

/**
 * @brief 初始化空中姿态控制参数
 */
void Momentum_Wheel_Control_Init(void)
{
    g_air_kp = 60.0f;  // 空中姿态P，需要非常激进
    g_air_kd = 8.0f;   // 空中姿态D，抑制空中翻转速度
    g_air_target_pitch = -1.0f; // 空中目标角度(度)，轻微后仰
    // 加载当前跳跃类型的时序和参数
    g_current_jump_type = JUMP_TYPE_NORMAL;//这里面可以选择不同的跳跃类型，测试时先用普通跳
    load_jump_profile(g_current_jump_type, servo_height);//【优化点】加载初始化跳跃姿态控制参数
}

/**
 * @brief 动量轮姿态控制核心算法 (在空中运行时调用)
 * @param current_pitch 当前俯仰角 (来自 IMU)
 * @param current_gyro  当前俯仰角速度 (来自 IMU)
 * @return int16_t       计算出的电机PWM值
 */
int16_t Momentum_Wheel_Control_Run(float current_pitch, float current_gyro)
{
    // 1. 计算姿态误差
    float error = g_air_target_pitch - current_pitch;
    
    // 2. PD 控制器计算
    //    原理:
    //    - 车头过低 (current_pitch > target), error < 0. 需抬头.
    //    - 抬头需要向后的反作用力矩, 故轮子需向前加速.
    //    - 假设向前加速是正PWM, 则公式为 Kp * (-error), 即 Kp * (target - current).
    //    - D项同理, 抑制角速度.
    float pwm_out = (g_air_kp * error) - (g_air_kd * current_gyro);

    // 3. 输出限幅 (空中需要很大力矩, 限幅可以给高一些)
    //    现在的 GYR_MAX_O 是 6000，这里可以给到更高，测试为了安全，我先给到6000
    pwm_out = Float_Constrain(pwm_out, -6000.0f, 6000.0f);

    return (int16_t)pwm_out;
}