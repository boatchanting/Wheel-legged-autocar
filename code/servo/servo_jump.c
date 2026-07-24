#include "servo_jump.h"
#include "servo_executor.h"
#include "../config/sys_options.h"
#include "../config/car_select.h" // 根据小车选择配置不同的跳跃参数
// 状态变量
uint8_t jump_flag = 0;
uint32_t jump_start_time = 0;
volatile JumpPhase g_current_jump_phase = JUMP_PHASE_NONE; // 初始化为 NONE
JumpType_e g_current_jump_type = JUMP_TYPE_NORMAL;
JumpProfile_t g_jump_profile; // 当前正在执行的跳跃参数
bool vision_detected_jump_point = false;//跳跃测试用
bool vision_detected_three_jump_point = false; // 三连跳测试用
volatile int32 g_jump_target_pwm_lf = 0;
volatile int32 g_jump_target_pwm_rf = 0;
volatile int32 g_jump_target_pwm_rr = 0;
volatile int32 g_jump_target_pwm_lr = 0;
volatile uint32_t g_jump_launch_cmd_time_ms = 0;
volatile uint32_t g_jump_flight_cmd_time_ms = 0;
static uint8_t g_jump_launch_cmd_time_recorded = 0U;
static uint8_t g_jump_flight_cmd_time_recorded = 0U;
// 引用外部变量 (来自servo.c)
extern volatile int32 PWM_CH1_LAST, PWM_CH2_LAST, PWM_CH3_LAST, PWM_CH4_LAST;
extern float servo_height; 
extern int16 pwm_high; // 查表后的高度duty基准
// 动量轮私有变量
float g_air_kp;
float g_air_kd;
float g_air_target_pitch;
extern InertialNav_t inertial_nav;
extern volatile float target_speed_set;

typedef enum {
    STEP_UP_TEST_IDLE = 0,
    STEP_UP_TEST_RUN_1,
    STEP_UP_TEST_WAIT_1,
    STEP_UP_TEST_RUN_2,
    STEP_UP_TEST_WAIT_2,
    STEP_UP_TEST_RUN_3,
    STEP_UP_TEST_WAIT_3,
    STEP_UP_TEST_FINISH
} StepUpTestState_e;

typedef struct {
    float left_module_length_mm;   // 起点到第1级台阶“目标落脚区”的等效路程
    float middle_module_length_mm; // 第1级到第2级之间的等效路程
    float right_module_length_mm;  // 第2级到第3级之间的等效路程
    float step_height_mm;          // 单级台阶高度（当前三连跳触发逻辑未直接使用，主要用于记录赛道）
    float ramp_angle_deg;          // 斜面角度（当前三连跳触发逻辑未直接使用）
    float ramp_face_length_mm;     // 斜面长度（当前三连跳触发逻辑未直接使用）
    float ramp_top_flat_mm;        // 台阶平台平直长度（当前三连跳触发逻辑未直接使用）
} ThreeStepGeom_t;

// 三连跳赛道几何参数（单位统一为 mm/deg）
// 当前触发距离只使用前三个“等效路程”参数：
//   第1跳门限 = left_module_length_mm - STEP_UP_TRIGGER_LEAD_MM
//   第2跳门限 = left + middle - STEP_UP_TRIGGER_LEAD_MM
//   第3跳门限 = left + middle + right - STEP_UP_TRIGGER_LEAD_MM
//
// 下面这组值对应的门限大约是：
//   第1跳 505mm, 第2跳 1005mm, 第3跳 1505mm
static const ThreeStepGeom_t g_three_step_geom = {
    625.0f, // left_module_length_mm
    500.0f, // middle_module_length_mm
    500.0f, // right_module_length_mm
    50.0f,  // step_height_mm
    22.0f,  // ramp_angle_deg
    404.0f, // ramp_face_length_mm
    250.0f  // ramp_top_flat_mm
};

// 触发提前量（mm）：用于“提前起跳”而不是到台阶边缘再起跳。
// 规则：值越大 -> 触发越早；值越小 -> 触发越晚。
// 若出现“还没到就跳”，应减小此值；若出现“到边缘才跳/跳不上”，应增大此值。
#define STEP_UP_TRIGGER_LEAD_MM         (120.0f)
#define STEP_UP_JUMP_GAP_MS             (150U)
#define STEP_UP_START_DELAY_MS          (200U)
#define STEP_UP_SPEED_APPROACH_1        (-60.0f)
#define STEP_UP_SPEED_APPROACH_2        (-52.0f)
#define STEP_UP_SPEED_APPROACH_3        (-45.0f)
#define STEP_UP_SPEED_FINISH            (0.0f)
#define STEP_UP_POST_RUN_MM             (1000.0f)
#define STEP_UP_FINISH_HOLD_MS          (800U)

static StepUpTestState_e g_step_up_test_state = STEP_UP_TEST_IDLE;
static uint32_t g_step_up_state_tick = 0;
static float g_step_up_start_x = 0.0f;
static float g_step_up_start_y = 0.0f;
static float g_step_up_finish_start_distance_mm = 0.0f;
static uint8_t g_step_up_finish_stopped = 0;

static float jump_stepup_test_get_distance_mm(void)
{
    // 以“三连跳开始时”的惯导坐标为零点，计算累计平面位移（mm）。
    // 该距离直接用于 trigger1/2/3 的门限判断。
    float dx = inertial_nav.x - g_step_up_start_x;
    float dy = inertial_nav.y - g_step_up_start_y;
    return sqrtf(dx * dx + dy * dy);
}
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
             // === 时间轴参数 (ms) ===
            g_jump_profile.t_launch = 80;        // 起跳发力：时间缩短至80ms
            g_jump_profile.t_flight = 150;       // 腾空时间：缩短至150ms开始准备落地
            g_jump_profile.t_landing = 170;      // 落地准备：保持20ms的空中伸腿前摇
            g_jump_profile.t_recovery = 230;     // 缓冲恢复：冲击小，恢复时间可缩短至60ms完成
            
            // === 动作幅度参数 (Duty Offset) ===
            // 设基准值1500，具体需根据这台车的实际物理反馈微调（见下方指南）
            g_jump_profile.offset_launch = 1500; // 起跳推力：原推力2700的约55%
            g_jump_profile.offset_flight = -800; // 空中收腿：无需大幅度收紧
            g_jump_profile.offset_land = 1000;   // 落地缓冲：伸腿幅度减小
            
            // === 姿态与其他 ===
            // 如果这台车起跳容易“栽头”或者“后翻”，可以微调这个Pitch值
            g_jump_profile.air_target_pitch = -1.0f; 
            g_jump_profile.post_jump_height = current_height; 
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
             // === 时间轴参数 (ms) ===
            // 滞空时间与高度的平方根成正比，高度减半，总时间约缩短 25%~30%
            g_jump_profile.t_launch = 80;        // 行程变短，起跳发力时间相应减少 (原100)
            g_jump_profile.t_flight = 150;       // 腾空时间变短，提前结束空中姿态 (原200)
            g_jump_profile.t_landing = 170;      // 落地伸腿准备时间保持20ms左右 (原220)
            g_jump_profile.t_recovery = 230;     // 冲击力小了，恢复时间可以缩短 (原280)
            
            // === 动作幅度参数 (Duty Offset) ===
            // 由于机器人自重和机械损耗，占空比直接减半可能导致离不开地，所以取一半偏上
            g_jump_profile.offset_launch = 1500; // 起跳推力：约为原推力(2700)的一半多一点
            g_jump_profile.offset_flight = -800; // 空中收腿：跳得低，不需要大幅度收腿 (原-1500)
            g_jump_profile.offset_land = 1000;   // 落地缓冲：下落冲击力小，伸腿幅度减小 (原1700)
            
            // === 姿态与其他 ===
            g_jump_profile.air_target_pitch = -0.5f; // 小跳姿态变化小，稍微低头即可
            g_jump_profile.post_jump_height = current_height; 
            break;
            
        case JUMP_TYPE_STEP_UP: // 【上台阶模式】
            g_jump_profile.t_launch = 90;
            g_jump_profile.t_flight = 100;
            g_jump_profile.t_landing = 130;
            g_jump_profile.t_recovery = 1500;
            g_jump_profile.offset_launch = 3000; 
            g_jump_profile.offset_flight = -1000;
            g_jump_profile.offset_land = 1700;
            g_jump_profile.air_target_pitch = -1.0f; // 默认轻微低头
            g_jump_profile.post_jump_height = current_height;// 【重心控制】跳上台阶后，调低基准身高防止摔倒 (需调参)
            break;
            
        case JUMP_TYPE_NORMAL: // 【普通平地跳】
        default:
            g_jump_profile.t_launch = 110;
            g_jump_profile.t_flight = 112;
            g_jump_profile.t_landing = 140;
            g_jump_profile.t_recovery = 160;
            g_jump_profile.offset_launch = 3100; 
            g_jump_profile.offset_flight = -500;
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
        g_current_jump_type = type;
        //load_jump_profile(g_current_jump_type, servo_height);
        time_elapsed1 = 0;
        time_elapsed2 = 0;
        time_elapsed3 = 0;
        time_elapsed4 = 0;
        g_jump_target_pwm_lf = 0;
        g_jump_target_pwm_rf = 0;
        g_jump_target_pwm_rr = 0;
        g_jump_target_pwm_lr = 0;
        g_jump_launch_cmd_time_ms = 0;
        g_jump_flight_cmd_time_ms = 0;
        g_jump_launch_cmd_time_recorded = 0U;
        g_jump_flight_cmd_time_recorded = 0U;
        jump_flag = 1;
        jump_start_time = loop_counter; // 锚定当前毫秒时间戳
        g_current_jump_phase = JUMP_PHASE_LAUNCH; // 初始阶段设为 A
    }
}

// 连续上三级台阶测试状态机
void jump_stepup_three_stairs_test_start(void)
{
    g_step_up_start_x = inertial_nav.x;
    g_step_up_start_y = inertial_nav.y;
    g_step_up_state_tick = loop_counter;
    g_step_up_test_state = STEP_UP_TEST_RUN_1;
}

bool jump_stepup_three_stairs_test_is_active(void)
{
    return (g_step_up_test_state != STEP_UP_TEST_IDLE);
}

void jump_stepup_three_stairs_test_update(void)
{
    float distance_mm;
    float trigger1_mm;
    float trigger2_mm;
    float trigger3_mm;

    // if (vision_detected_three_jump_point)
    // {
    //     if (g_step_up_test_state == STEP_UP_TEST_IDLE)
    //     {
    //         jump_stepup_three_stairs_test_start();
    //     }
    //     vision_detected_three_jump_point = false;
    // }

    if (g_step_up_test_state == STEP_UP_TEST_IDLE)
    {
        return;
    }

    // distance_mm: 从三连跳起点累计走过的平面距离（mm）
    distance_mm = jump_stepup_test_get_distance_mm();

    // 三个跳跃触发门限（mm）：
    // 1) trigger1_mm：接近第1级台阶前的起跳点
    // 2) trigger2_mm：接近第2级台阶前的起跳点
    // 3) trigger3_mm：接近第3级台阶前的起跳点
    //
    // 设计要点：三个门限都减去同一个 STEP_UP_TRIGGER_LEAD_MM，
    // 便于只改一个参数就整体“提前/延后”三次起跳时机。
    trigger1_mm = g_three_step_geom.left_module_length_mm - STEP_UP_TRIGGER_LEAD_MM;
    trigger2_mm = g_three_step_geom.left_module_length_mm +
                  g_three_step_geom.middle_module_length_mm - STEP_UP_TRIGGER_LEAD_MM;
    trigger3_mm = g_three_step_geom.left_module_length_mm +
                  g_three_step_geom.middle_module_length_mm +
                  g_three_step_geom.right_module_length_mm - STEP_UP_TRIGGER_LEAD_MM;

    switch (g_step_up_test_state)
    {
        case STEP_UP_TEST_RUN_1:
            target_speed_set = STEP_UP_SPEED_APPROACH_1;
            if ((loop_counter - g_step_up_state_tick) < STEP_UP_START_DELAY_MS)
            {
                break;
            }
            if ((distance_mm >= trigger1_mm) && (jump_flag == 0))
            {
                jump_trigger_with_type(JUMP_TYPE_STEP_UP);
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_WAIT_1;
            }
            break;

        case STEP_UP_TEST_WAIT_1:
            target_speed_set = STEP_UP_SPEED_APPROACH_2;
            if ((jump_flag == 0) && ((loop_counter - g_step_up_state_tick) > STEP_UP_JUMP_GAP_MS))
            {
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_RUN_2;
            }
            break;

        case STEP_UP_TEST_RUN_2:
            target_speed_set = STEP_UP_SPEED_APPROACH_2;
            if ((distance_mm >= trigger2_mm) && (jump_flag == 0))
            {
                jump_trigger_with_type(JUMP_TYPE_STEP_UP);
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_WAIT_2;
            }
            break;

        case STEP_UP_TEST_WAIT_2:
            target_speed_set = STEP_UP_SPEED_APPROACH_3;
            if ((jump_flag == 0) && ((loop_counter - g_step_up_state_tick) > STEP_UP_JUMP_GAP_MS))
            {
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_RUN_3;
            }
            break;

        case STEP_UP_TEST_RUN_3:
            target_speed_set = STEP_UP_SPEED_APPROACH_3;
            if ((distance_mm >= trigger3_mm) && (jump_flag == 0))
            {
                jump_trigger_with_type(JUMP_TYPE_STEP_UP);
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_WAIT_3;
            }
            break;

        case STEP_UP_TEST_WAIT_3:
            target_speed_set = STEP_UP_SPEED_APPROACH_3;
            if ((jump_flag == 0) && ((loop_counter - g_step_up_state_tick) > STEP_UP_JUMP_GAP_MS))
            {
                g_step_up_state_tick = loop_counter;
                g_step_up_test_state = STEP_UP_TEST_FINISH;
                g_step_up_finish_start_distance_mm = distance_mm;
                g_step_up_finish_stopped = 0;
            }
            break;

        case STEP_UP_TEST_FINISH:
            if (g_step_up_finish_stopped == 0)
            {
                // 第三跳完成后继续前进固定距离，再停车。
                if ((distance_mm - g_step_up_finish_start_distance_mm) >= STEP_UP_POST_RUN_MM)
                {
                    g_step_up_finish_stopped = 1;
                    g_step_up_state_tick = loop_counter;
                    target_speed_set = STEP_UP_SPEED_FINISH;
                }
                else
                {
                    target_speed_set = STEP_UP_SPEED_APPROACH_3;
                }
            }
            else
            {
                target_speed_set = STEP_UP_SPEED_FINISH;
                if ((loop_counter - g_step_up_state_tick) > STEP_UP_FINISH_HOLD_MS)
                {
                    g_step_up_test_state = STEP_UP_TEST_IDLE;
                    g_special_action_trigger = 0;  // 交还总状态机控制权限
                }
            }
            break;

        case STEP_UP_TEST_IDLE:
        default:
            g_step_up_test_state = STEP_UP_TEST_IDLE;
            break;
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
        if (g_jump_launch_cmd_time_recorded == 0U) {
            g_jump_launch_cmd_time_ms = time_elapsed;
            g_jump_launch_cmd_time_recorded = 1U;
        }
        
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
        if (g_jump_flight_cmd_time_recorded == 0U) {
            g_jump_flight_cmd_time_ms = time_elapsed;
            g_jump_flight_cmd_time_recorded = 1U;
        }
        
        target_lf = get_joint_target(current_duties_jump[0], SERVO_MOTOR_PWM1_DIR, h_duty, g_jump_profile.offset_flight);
        target_rf = get_joint_target(current_duties_jump[1], SERVO_MOTOR_PWM2_DIR, h_duty, g_jump_profile.offset_flight);
        target_rr = get_joint_target(current_duties_jump[2], SERVO_MOTOR_PWM3_DIR, h_duty, g_jump_profile.offset_flight);
        target_lr = get_joint_target(current_duties_jump[3], SERVO_MOTOR_PWM4_DIR, h_duty, g_jump_profile.offset_flight);
    }
#if JUMP_ENABLE_LANDING_BUFFER
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
#endif
    // --- 阶段 E: 结束 ---
    else
    {
        jump_flag = 0; // 动作完成，交还控制权
        g_current_jump_phase = JUMP_PHASE_NONE;
        return;
    }

    g_jump_target_pwm_lf = target_lf;
    g_jump_target_pwm_rf = target_rf;
    g_jump_target_pwm_rr = target_rr;
    g_jump_target_pwm_lr = target_lr;

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
