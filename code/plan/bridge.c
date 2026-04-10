#include "bridge.h"
// 必须包含你的全局变量声明头文件，确保能拿到下方的 extern 变量
// #include "zf_common_headfile.h"
// #include "pid.h"  

// ============================================================================
// 外部全局变量引用 (请确保这些在你的主工程中存在且名称一致)
// ============================================================================
extern InertialNav_t inertial_nav;          // 惯导定位数据
extern volatile float target_speed_set;     // 电机闭环目标速度
extern float servo_height;                  // 规划给执行器的高度指令

// 舵机执行器的斜率限制 (过桥时需放开，让腿瞬间弹射)
extern int32 acc_limit;
extern int32 dec_limit;

// Roll 平衡环使能开关 (在 pid.c 中定义)
extern uint8_t roll_balance_enable;

// 主平衡 PID 结构体 (用于动态插值)
extern PID_Param_t pid_angle;
extern PID_Param_t pid_gyro;
extern PID_Param_t pid_turn_angle;
extern PID_Param_t pid_turn_gyro;
extern volatile float roll_degree;
extern volatile float err_degree;
extern uint8 g_special_action_trigger;

// ============================================================================
// 内部状态与变量
// ============================================================================
BridgeState_e current_bridge_state = BRIDGE_STATE_IDLE;
BridgeParams_t bridge_params;

// 记录世界坐标起点
static float start_x = 0.0f;
static float start_y = 0.0f;
static float initial_distance_to_bridge = 0.0f;

// 保存正常的舵机限制，以便下桥后恢复
static int32 saved_acc_limit = 10;     
static int32 saved_dec_limit = 10;

typedef enum {
    BRIDGE_TEST_STATE_IDLE = 0,
    BRIDGE_TEST_STATE_PREPARE,
    BRIDGE_TEST_STATE_ON_BRIDGE,
    BRIDGE_TEST_STATE_EXIT
} BridgeTestState_e;

uint8_t debug_triple_bridge_test_enable = 0;
static BridgeTestState_e s_bridge_test_state = BRIDGE_TEST_STATE_IDLE;
static float s_test_roll_target = 0.0f;
static float s_locked_heading_deg = 0.0f;

// ============================================================================
// 【状态机新增】视觉触发标志位
// ============================================================================
bool vision_detected_bridge_point = false;

#define BRIDGE_TEST_PREPARE_LEN_MM      300.0f
#define BRIDGE_TEST_TOTAL_LEN_MM       3000.0f
#define BRIDGE_TEST_EXIT_BUFFER_MM      200.0f

#define BRIDGE_TEST_EDGE_MARGIN_MM     1000.0f
#define BRIDGE_TEST_OBS_LENGTH_MM       200.0f
#define BRIDGE_TEST_OBS_GAP_MM          200.0f
#define BRIDGE_TEST_ROLL_LEAD_MM         80.0f
#define BRIDGE_TEST_ROLL_HOLD_MM          40.0f

#define BRIDGE_TEST_ROLL_BIAS_DEG         3.0f
#define BRIDGE_TEST_ROLL_RAMP_STEP_DEG    0.3f
static const float k_bridge_test_side_sign[3] = {1.0f, -1.0f, 1.0f};

// 绝对值辅助函数
#define MY_ABS_F(x) ((x) > 0.0f ? (x) : -(x))

static float Bridge_Normalize_Angle(float angle_deg) {
    while (angle_deg > 180.0f) angle_deg -= 360.0f;
    while (angle_deg < -180.0f) angle_deg += 360.0f;
    return angle_deg;
}

// ============================================================================
// 辅助函数定义
// ============================================================================

/**
 * @brief 获取二维平面直线行驶距离
 */
static float Get_Traveled_Distance(void) {
    float dx = inertial_nav.x - start_x;
    float dy = inertial_nav.y - start_y;
    return sqrtf(dx * dx + dy * dy); 
}

/**
 * @brief 重置坐标积分起点
 */
static void Reset_Start_Point(void) {
    start_x = inertial_nav.x;
    start_y = inertial_nav.y;
}

/**
 * @brief 【核心】根据当前车身高度，动态平滑插值 PID 参数
 * 解决抬高车身时的原地震荡问题
 */
static void PID_Dynamic_Update_By_Height(float current_height) {
    float h_min = bridge_params.height_normal;
    float h_max = bridge_params.height_bridge;
    
    // 计算当前高度在区间内的比例 (0.0 ~ 1.0)
    float ratio = 0.0f;
    if ((h_max - h_min) > 0.1f) {
        ratio = (current_height - h_min) / (h_max - h_min);
    }
    
    // 严格限幅
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    // 线性插值计算当前应该使用的 PID 参数
    // 角度环
    pid_angle.kp = ANG_KP + ratio * (ANG_KP_HIGH - ANG_KP);
    pid_angle.kd = ANG_KD + ratio * (ANG_KD_HIGH - ANG_KD);
    pid_angle.compensation = ANG_MECH_ZERO + ratio * (ANG_MECH_ZERO_HIGH - ANG_MECH_ZERO);
    
    // 角速度环
    pid_gyro.kp = GYR_KP + ratio * (GYR_KP_HIGH - GYR_KP);

    // 舵机速度环
    pid_servo_speed.kp = SERVO_SPEED_KP + ratio * (SERVO_SPEED_KP_HIGH - SERVO_SPEED_KP);
    pid_servo_speed.ki = SERVO_SPEED_KI + ratio * (SERVO_SPEED_KI_HIGH - SERVO_SPEED_KI);

    // 转向环
    pid_turn_angle.kp = TURN_ANG_KP + ratio * (TURN_ANG_KP_HIGH - TURN_ANG_KP);
    pid_turn_gyro.kp  = TURN_GYR_KP + ratio * (TURN_GYR_KP_HIGH - TURN_GYR_KP);
}

/**
 * @brief 平滑高度控制，并联动触发 PID 动态更新
 */
static void Smooth_Height_Control(float target_height, float step_size) {
    if (MY_ABS_F(servo_height - target_height) >= step_size) {
        if (servo_height < target_height) {
            servo_height += step_size;
            if (servo_height > target_height) servo_height = target_height;
        } 
        else if (servo_height > target_height) {
            servo_height -= step_size;
            if (servo_height < target_height) servo_height = target_height;
        }
    } else {
        servo_height = target_height;
    }

    // 只要高度在控制，就必须实时更新 PID，确保不震荡
    PID_Dynamic_Update_By_Height(servo_height);
}

static float Ramp_Float(float current, float target, float step) {
    if (current < target) {
        current += step;
        if (current > target) current = target;
    } else if (current > target) {
        current -= step;
        if (current < target) current = target;
    }
    return current;
}

static void Bridge_Test_Reset_All(uint8 keep_speed) {
    roll_balance_enable = 0;
    roll_degree = 0.0f;
    err_degree = 0.0f;
    s_test_roll_target = 0.0f;
    s_locked_heading_deg = inertial_nav.relative_yaw;

    acc_limit = saved_acc_limit;
    dec_limit = saved_dec_limit;

    if (keep_speed == 0) {
        target_speed_set = 0.0f;
    } else {
        target_speed_set = bridge_params.speed_normal;
    }

    s_bridge_test_state = BRIDGE_TEST_STATE_IDLE;
    g_special_action_trigger = 0;
}

static float Bridge_Test_Get_Roll_Bias(float dist_mm) {
    float obstacle_start = BRIDGE_TEST_EDGE_MARGIN_MM;
    int i;

    for (i = 0; i < 3; i++) {
        float trigger_begin = obstacle_start - BRIDGE_TEST_ROLL_LEAD_MM;
        float trigger_end = obstacle_start + BRIDGE_TEST_OBS_LENGTH_MM + BRIDGE_TEST_ROLL_HOLD_MM;

        if ((dist_mm >= trigger_begin) && (dist_mm <= trigger_end)) {
            return k_bridge_test_side_sign[i] * BRIDGE_TEST_ROLL_BIAS_DEG;
        }

        obstacle_start += BRIDGE_TEST_OBS_LENGTH_MM + BRIDGE_TEST_OBS_GAP_MM;
    }

    return 0.0f;
}

// ============================================================================
// 【状态机新增】视觉触发相关函数实现
// ============================================================================

/**
 * @brief 检查单边桥测试是否处于活动状态
 * @return true: 测试进行中 / false: 空闲状态
 */
bool Bridge_Test_Triple_SingleSide_Is_Active(void) {
    return (s_bridge_test_state != BRIDGE_TEST_STATE_IDLE);
}

/**
 * @brief 启动单边桥测试
 * @note 仅当状态机处于 IDLE 时生效，防止测试中途重复触发
 */
void Bridge_Test_Triple_SingleSide_Start(void) {
    if (s_bridge_test_state == BRIDGE_TEST_STATE_IDLE) {
        debug_triple_bridge_test_enable = 1;
        s_locked_heading_deg = inertial_nav.relative_yaw;
        // 状态机会在下一次调用 Bridge_Test_Triple_SingleSide_Inertial 时自动从 IDLE 转到 PREPARE
    }
}

// ============================================================================
// 主流程控制
// ============================================================================

void Bridge_Init(void) {
    current_bridge_state = BRIDGE_STATE_IDLE;
    roll_balance_enable = 0;
    debug_triple_bridge_test_enable = 0;
    s_bridge_test_state = BRIDGE_TEST_STATE_IDLE;
    s_test_roll_target = 0.0f;
    vision_detected_bridge_point = false;  // 【状态机新增】初始化视觉标志位
    
    // 距离参数 (需实测微调)
    bridge_params.trigger_brake_dist = 800.0f; 
    bridge_params.trigger_ready_dist = 400.0f; // 提前40cm开始升腿
    bridge_params.enter_bridge_dist  = 150.0f; 
    bridge_params.on_bridge_length   = 1500.0f; 
    bridge_params.cooldown_distance  = 1000.0f; 

    // 速度参数 
    bridge_params.speed_brake   = -30.0f; 
    bridge_params.speed_ready   = -45.0f; 
    bridge_params.speed_climb   = -60.0f; 
    bridge_params.speed_normal  = -90.0f; 

    // 姿态参数
    bridge_params.height_normal = 3.0f;   // 平地正常高度
    bridge_params.height_bridge = 6.0f;  // 桥上高度 (留一点余量给一边缩短)
    
    // 高度步长：假设 20ms 调用一次此函数
    // 升高13cm需要：13 / 0.5 = 26次 = 0.52秒，时间足够
    bridge_params.height_step_rise = 0.5f;   
    bridge_params.height_step_drop = 0.8f;   

    // 舵机刚度：上桥时放开限制，允许 100 满斜率响应 PID
    bridge_params.servo_acc_bridge = 100;  
    bridge_params.servo_dec_bridge = 100;
}

void Bridge_Trigger(float distance_to_bridge) {
    if (current_bridge_state == BRIDGE_STATE_IDLE) {
        Reset_Start_Point();
        initial_distance_to_bridge = distance_to_bridge;
        
        saved_acc_limit = acc_limit;
        saved_dec_limit = dec_limit;
        
        current_bridge_state = BRIDGE_STATE_BRAKE;
    }
}

void Bridge_Update(void) {
    // 计算离桥头的剩余距离
    float remaining_dist = initial_distance_to_bridge - Get_Traveled_Distance();

    switch (current_bridge_state) {
        
        case BRIDGE_STATE_IDLE:
            break;

        case BRIDGE_STATE_BRAKE:
            target_speed_set = bridge_params.speed_brake; 
            if (remaining_dist < bridge_params.trigger_brake_dist) {
                current_bridge_state = BRIDGE_STATE_READY;
            }
            break;

        case BRIDGE_STATE_READY:
            // 开始平滑抬升车身，并动态更新 PID 防止发抖
            Smooth_Height_Control(bridge_params.height_bridge, bridge_params.height_step_rise);

            // 当距离逼近桥头，进入上桥爬坡状态
            if (remaining_dist < bridge_params.trigger_ready_dist) {
                target_speed_set = bridge_params.speed_ready;
                current_bridge_state = BRIDGE_STATE_CLIMB;
            }
            break;

        case BRIDGE_STATE_CLIMB:
            // 维持抬升状态直到完全抬起
            Smooth_Height_Control(bridge_params.height_bridge, bridge_params.height_step_rise);
            target_speed_set = bridge_params.speed_climb;
            
            // 越过桥头一段距离，说明车身已完全上了单边桥
            if (remaining_dist < -bridge_params.enter_bridge_dist) {
                
                // 【核心开启】：放开舵机速度，开启底层“一边不动一边缩短”的横滚平衡
                acc_limit = bridge_params.servo_acc_bridge;
                dec_limit = bridge_params.servo_dec_bridge;
                roll_balance_enable = 1; 

                Reset_Start_Point(); 
                current_bridge_state = BRIDGE_STATE_ON_BRIDGE;
            }
            break;

        case BRIDGE_STATE_ON_BRIDGE:
            // 在桥上保持高姿态，底层 Roll_Balance_Control 正在自动吸收左右高度差
            target_speed_set = bridge_params.speed_climb;
            
            // 行驶完桥长，准备离开
            if (Get_Traveled_Distance() > bridge_params.on_bridge_length) {
                // 【核心关闭】：下桥瞬间关闭自适应横滚，恢复舵机平滑度
                roll_balance_enable = 0; 
                acc_limit = saved_acc_limit;
                dec_limit = saved_dec_limit;
                target_speed_set = bridge_params.speed_normal;
                
                Reset_Start_Point(); 
                current_bridge_state = BRIDGE_STATE_LEAVING;
            }
            break;

        case BRIDGE_STATE_LEAVING:
            // 下桥后，平滑降低车身，并在下降过程中继续动态插值 PID
            Smooth_Height_Control(bridge_params.height_normal, bridge_params.height_step_drop);

            // 当高度完全恢复正常
            if (MY_ABS_F(servo_height - bridge_params.height_normal) < 0.1f) {
                current_bridge_state = BRIDGE_STATE_COOLDOWN;
            }
            break;

        case BRIDGE_STATE_COOLDOWN:
            // 冷却阶段，防止车身震动引起图像误判二次触发
            if (Get_Traveled_Distance() > bridge_params.cooldown_distance) {
                current_bridge_state = BRIDGE_STATE_IDLE;
            }
            break;

        default:
            current_bridge_state = BRIDGE_STATE_IDLE;
            break;
    }
}

// 定义测试控制变量
uint8_t debug_force_height_up = 0; 

/**
 * @brief 【测试专用】平地原地升降测试函数
 * @note 测试时，请注释掉主循环里的 Bridge_Update()，改为调用此函数
 */
void Bridge_Test_Smooth_PID(void) {
    // 确定当前的目标高度
    float target_h;
    if (debug_force_height_up == 1) {
        target_h = bridge_params.height_bridge; // 目标：高姿态
    } else {
        target_h = bridge_params.height_normal; // 目标：矮姿态
    }

    // 调用现成的平滑升降函数 (内部会自动插值更新 PID)
    // 为了测试安全，我们可以把步长设慢一点，比如 0.2f，让升降过程延长到1秒左右，方便观察
    Smooth_Height_Control(target_h, 0.2f); 
    
    // 测试时，强制关闭底层的 Roll 自适应平衡，避免干扰纯直立 PID 的整定
    //roll_balance_enable = 0;

}

void Bridge_Test_Triple_SingleSide_Inertial(void) {
    float traveled_mm;
    float target_roll_bias;

    if (debug_triple_bridge_test_enable == 0) {
        if (s_bridge_test_state != BRIDGE_TEST_STATE_IDLE) {
            Bridge_Test_Reset_All(1);
        }
        return;
    }

    switch (s_bridge_test_state) {
        // ====================================================================
        // 【状态机修改】IDLE 状态：确保 debug_triple_bridge_test_enable 为 1 时正确启动
        // ====================================================================
        case BRIDGE_TEST_STATE_IDLE:
            if (debug_triple_bridge_test_enable) {
                saved_acc_limit = acc_limit;
                saved_dec_limit = dec_limit;
                Reset_Start_Point();
                s_test_roll_target = 0.0f;
                roll_degree = 0.0f;
                s_locked_heading_deg = inertial_nav.relative_yaw;
                s_bridge_test_state = BRIDGE_TEST_STATE_PREPARE;
            }
            break;

        case BRIDGE_TEST_STATE_PREPARE:
            err_degree = Bridge_Normalize_Angle(s_locked_heading_deg - inertial_nav.relative_yaw);
            target_speed_set = bridge_params.speed_ready;
            Smooth_Height_Control(bridge_params.height_bridge, bridge_params.height_step_rise);
            acc_limit = bridge_params.servo_acc_bridge;
            dec_limit = bridge_params.servo_dec_bridge;
            roll_balance_enable = 1;

            if (Get_Traveled_Distance() >= BRIDGE_TEST_PREPARE_LEN_MM) {
                Reset_Start_Point();
                s_bridge_test_state = BRIDGE_TEST_STATE_ON_BRIDGE;
            }
            break;

        case BRIDGE_TEST_STATE_ON_BRIDGE:
            err_degree = Bridge_Normalize_Angle(s_locked_heading_deg - inertial_nav.relative_yaw);
            target_speed_set = bridge_params.speed_climb;
            Smooth_Height_Control(bridge_params.height_bridge, bridge_params.height_step_rise);

            traveled_mm = Get_Traveled_Distance();
            target_roll_bias = Bridge_Test_Get_Roll_Bias(traveled_mm);
            s_test_roll_target = Ramp_Float(s_test_roll_target, target_roll_bias, BRIDGE_TEST_ROLL_RAMP_STEP_DEG);
            roll_degree = s_test_roll_target;

            if (traveled_mm >= (BRIDGE_TEST_TOTAL_LEN_MM + BRIDGE_TEST_EXIT_BUFFER_MM)) {
                Reset_Start_Point();
                s_bridge_test_state = BRIDGE_TEST_STATE_EXIT;
            }
            break;

        case BRIDGE_TEST_STATE_EXIT:
            err_degree = Bridge_Normalize_Angle(s_locked_heading_deg - inertial_nav.relative_yaw);
            target_speed_set = bridge_params.speed_normal;
            Smooth_Height_Control(bridge_params.height_normal, bridge_params.height_step_drop);

            s_test_roll_target = Ramp_Float(s_test_roll_target, 0.0f, BRIDGE_TEST_ROLL_RAMP_STEP_DEG);
            roll_degree = s_test_roll_target;

            if ((MY_ABS_F(servo_height - bridge_params.height_normal) < 0.1f) &&
                (MY_ABS_F(s_test_roll_target) < 0.2f) &&
                (Get_Traveled_Distance() >= bridge_params.cooldown_distance)) {
                Bridge_Test_Reset_All(1);
                debug_triple_bridge_test_enable = 0;
            }
            break;

        default:
            Bridge_Test_Reset_All(0);
            debug_triple_bridge_test_enable = 0;
            break;
    }
}
