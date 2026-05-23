#include "sbus.h"
#include "config/config.h"//【提醒】配置请在这里修改
#include "../common.h"
#include "../vision/vision_bridge_control.h"

// ==========================================
// 1. 宏定义 (参数配置区)
// ==========================================

// --- 摇杆中值 ---
#define RC_CH1_MID      1088    // 转向中值
#define RC_CH2_MID      976     // 油门中值
#define RC_DEADZONE     30      // 摇杆死区 (防止抖动漂移)

// --- 开关阈值 ---
#define RC_SW_THRESHOLD 1000    // 二态开关判定阈值

// CH4 三态开关判定区间 (低=192, 中=992, 高=1792)
#define RC_SW_MID_LOW   600     // <600 判为 LOW
#define RC_SW_MID_HIGH  1400    // >1400 判为 HIGH

// --- 增量系数 (灵敏度) ---

// --- 线控转向倍率参数 (随车速动态调整) ---
// 说明：低速时转向倍率高，高速时转向倍率低
#define STEER_GAIN_MAX         1.00f    // 最大发电倍率（低速）
#define STEER_GAIN_MIN         0.30f    // 最小转向倍率（高速）
#define STEER_SPEED_BREAK_LOW  350.0f   // 低速分界点（单位：与轮速反馈一致）
#define STEER_SPEED_BREAK_HIGH 1300.0f  // 高速分界点（单位：与轮速反馈一致）
// 说明: 每次调用 Process 函数增加的数值 = (摇杆偏差值) * 系数
// 假设 Process 每 10ms 调用一次
#define K_STEER_INC     0.00225f  // 转向灵敏度
#define K_SPEED_INC     0.005f  // 速度灵敏度

// 积分限幅
#define MAX_STEER_ANGLE 45.0f   // 最大转向角度 (例如 +/- 45度)
#define MAX_SPEED_VAL   3000.0f  // 最大速度目标值 (对应 target_speed_set)

#define SPEED_DECEL_RATIO    0.99f  // 百分比减速，实际没那么快
#define SPEED_STOP_THRESHOLD  1.0f// 速度归零阈值：当目标速度削弱到这个绝对值以下时，直接归 0 且锁定
#define REVERSE_BRAKE_TRIGGER_COUNT 10U // 反向速度杆连续触发次数，达到后锁存重刹直到摇杆回中
static uint8 throttle_locked = 0; // 油门锁标记：0=正常接受指令, 1=锁定(需等摇杆回中)

// ==========================================
// 2. 全局变量定义
// ==========================================
robot_ctrl_t robot_ctrl;
volatile uint8 g_brake_active = 0;
volatile uint8 g_reverse_brake_active = 0;

// ==========================================
// 3. 函数实现
// ==========================================
// 基于左右轮速反馈计算当前车速对应的转向倍率
// 设计目标：
// 1) 低速放大转向灵敏度；2) 高速降低转向灵敏度；3) 过渡无突变
static float Remote_Calc_Steer_Gain_BySpeed(float left_speed, float right_speed)
{
    // 采用平均绝对轮速作为“车速估计”，避免正反转方向影响分段逻辑
    float vehicle_speed_abs = (ABS(left_speed) + ABS(right_speed)) * 0.5f;

    // 低速区：固定最大倍率
    if (vehicle_speed_abs <= STEER_SPEED_BREAK_LOW)
    {
        return STEER_GAIN_MAX;
    }

    // 高速区：固定最小倍率
    if (vehicle_speed_abs >= STEER_SPEED_BREAK_HIGH)
    {
        return STEER_GAIN_MIN;
    }

    // 中速过渡区：使用 smoothstep 平滑插值，保证一阶连续，避免响应突变
    float t = (vehicle_speed_abs - STEER_SPEED_BREAK_LOW) /
              (STEER_SPEED_BREAK_HIGH - STEER_SPEED_BREAK_LOW);

    // smoothstep(t) = 3t^2 - 2t^3
    float smooth_t = t * t * (3.0f - 2.0f * t);

    // 从最大倍率平滑过渡到最小倍率
    return STEER_GAIN_MAX + (STEER_GAIN_MIN - STEER_GAIN_MAX) * smooth_t;
}

uint8 Remote_control_connected =0;
// 初始化
void Remote_Control_Init(void)
{
    robot_ctrl.target_angle = 0.0f;
    robot_ctrl.target_speed = 0.0f;
    robot_ctrl.mark_trigger = 0;
    robot_ctrl.motor_enable = 1;  //1=使能,0=急停
    robot_ctrl.brake_active = 0;
    robot_ctrl.reverse_brake_active = 0;
    g_brake_active = 0;
    g_reverse_brake_active = 0;
    robot_ctrl.point_type = 0;
    // 模式枚举 (对应 CH4 三态开关和CH5开关的组合状态，使用ch3开关进行触发)
    // NAV_POINT_PATH = 0,     // 普通路径点
    // NAV_POINT_CIRCLE = 1,   // 转圈点
    // NAV_POINT_SLOPE = 2,    // 上坡点
    // NAV_POINT_JUMP = 3,     // 跳跃点
    // NAV_POINT_BRIDGE = 4,   // 单边桥点
    // NAV_POINT_BUMP = 5      // 颠簸路段点
}

// 核心处理逻辑
void Remote_Control_Process(void)
{
    // --------------------------------------------------------
    // Step 1: 读取 S.BUS 原始数据
    // --------------------------------------------------------
    // 依赖 zf_device_uart_receiver.h 中的 uart_receiver 全局变量
    if(1 == uart_receiver.state && 0 == Remote_control_connected)                             // 遥控器失控状态判断 == uart_receiver.state
    {
        // gpio_toggle_level(BUZZER_PIN); // 翻转电平
        // gpio_toggle_level(BUZZER_PIN); // 翻转电平,蜂鸣器
        // printf("Remote control is connected. ");
        // // Remote_control_connected=1;
        // for(int i = 0; i < 6; i++)
        // {
        //     printf("%d ", uart_receiver.channel[i]);         // 串口输出6个通道数据
        // }
        // printf("\r\n");
    }
    else
    {
        // printf("Remote control is disconnected. ");
        robot_ctrl.brake_active = 0;
        robot_ctrl.reverse_brake_active = 0;
        g_brake_active = 0;
        g_reverse_brake_active = 0;
        robot_ctrl.motor_enable = 0;//如果遥控器断联，直接停机【优化点】不能直接停机
        return; // 失控则不进行后续处理

    }
    int16 ch1_steer = uart_receiver.channel[0];
#if SUBS_CATEGORY == 1
    int16 ch2_thro  = uart_receiver.channel[1];
#else
    int16 ch2_thro  = 1952 - uart_receiver.channel[1];
#endif
    int16 ch3_mark  = uart_receiver.channel[2];
    int16 ch4_mode  = uart_receiver.channel[3];
    int16 ch5_brake = uart_receiver.channel[4];
    int16 ch6_off   = uart_receiver.channel[5];

    // --------------------------------------------------------
    // Step 2: 处理最高优先级逻辑 (CH6 总开关)
    // --------------------------------------------------------
    // 1792 (>1000) 为关电机状态
    if(ch1_steer == 0 && ch2_thro == 0 && ch3_mark == 0 && ch4_mode == 0 && ch5_brake == 0 && ch6_off == 0)
    {
        robot_ctrl.brake_active = 0;
        robot_ctrl.reverse_brake_active = 0;
        g_brake_active = 0;
        g_reverse_brake_active = 0;
        robot_ctrl.motor_enable = 0;//如果遥控器断联，直接停机【优化点】不能直接停机
        return; // 遥控器完全回中且总开关关闭，则不进行后续处理
    }
    
    // CH6 消抖逻辑：5次连续采样确认状态变化
    static uint8 ch6_debounce_count = 0;
    static uint8 ch6_last_valid_state = 0; // 0=电机使能, 1=电机关
    uint8 ch6_current_raw = (ch6_off > RC_SW_THRESHOLD) ? 1 : 0;
    
    if (ch6_current_raw != ch6_last_valid_state)
    {
        ch6_debounce_count++;
        if (ch6_debounce_count >= 5)
        {
            ch6_last_valid_state = ch6_current_raw;
            ch6_debounce_count = 0;
        }
    }
    else
    {
        ch6_debounce_count = 0;
    }
    
    if (ch6_last_valid_state == 1) 
    {
        robot_ctrl.brake_active = 0;
        robot_ctrl.reverse_brake_active = 0;
        g_brake_active = 0;
        g_reverse_brake_active = 0;
        robot_ctrl.motor_enable = 0;
        // printf("Motor disabled by CH6 switch\n");
        // 关机状态下，不进行增量计算，防止后台积分
        return; 
    }
    else 
    {
        robot_ctrl.motor_enable = 1;
    }

    // --------------------------------------------------------
    // Step 4: 处理转向 (CH1)
    // --------------------------------------------------------
    int16 diff_steer = ch1_steer - RC_CH1_MID;
    
    if (abs(diff_steer) > RC_DEADZONE)
    {
        // 根据当前车速动态计算转向倍率：低速更灵敏，高速更稳
        float steer_gain = Remote_Calc_Steer_Gain_BySpeed(motor_value.receive_left_speed_data,
                                                          motor_value.receive_right_speed_data);

        // 积分计算（带动态倍率）
        robot_ctrl.target_angle += (float)diff_steer * K_STEER_INC * steer_gain;
        
        // // 限幅逻辑
        // if (robot_ctrl.target_angle > MAX_STEER_ANGLE) 
        //     robot_ctrl.target_angle = MAX_STEER_ANGLE;
        
        // if (robot_ctrl.target_angle < -MAX_STEER_ANGLE) 
        //     robot_ctrl.target_angle = -MAX_STEER_ANGLE;
        // printf("Target Angle: %.2f\n", robot_ctrl.target_angle);
    }



    // --------------------------------------------------------
    // Step 5: 处理刹车/油门/急停 (CH5 & CH2)
    // --------------------------------------------------------

    robot_ctrl.brake_active = (ch5_brake > RC_SW_THRESHOLD) ? 1U : 0U;
    g_brake_active = robot_ctrl.brake_active;
    int16 diff_speed = ch2_thro - RC_CH2_MID;
    static uint8 reverse_brake_count = 0;
    uint8 reverse_speed_cmd = (uint8)((robot_ctrl.target_speed > 0.0f && diff_speed < 0) ||
                                      (robot_ctrl.target_speed < 0.0f && diff_speed > 0));

    if (abs(diff_speed) <= RC_DEADZONE)
    {
        reverse_brake_count = 0;
        robot_ctrl.reverse_brake_active = 0;
    }
    else if ((robot_ctrl.reverse_brake_active == 0U) && reverse_speed_cmd)
    {
        if (reverse_brake_count < 255U)
        {
            reverse_brake_count++;
        }
        if (reverse_brake_count >= REVERSE_BRAKE_TRIGGER_COUNT)
        {
            robot_ctrl.reverse_brake_active = 1U;
        }
    }
    else if (reverse_speed_cmd == 0U)
    {
        reverse_brake_count = 0;
    }
    g_reverse_brake_active = robot_ctrl.reverse_brake_active;

    if ((ch5_brake > RC_SW_THRESHOLD) || (robot_ctrl.reverse_brake_active != 0U))
    {
        robot_ctrl.target_speed = 0.0f; // 刹车清零
        throttle_locked = 1;            // 建议：拨下刹车后也锁定油门，防止松开刹车瞬间车子暴冲，必须重新回中
    }
    else
    {
        // 【情况 A】摇杆在死区内 (回中)
        if (abs(diff_speed) <= RC_DEADZONE) 
        {
            throttle_locked = 0; // 摇杆已回中，解除油门锁定
        }
        // 【情况 B】摇杆在死区外，且未被锁定
        else if (throttle_locked == 0) 
        {
            // 判断当前属于【反向减速削弱】还是【同向加速/起步】
            // 逻辑: (速度为正且摇杆往后拉) 或者 (速度为负且摇杆往前推)
            if ((robot_ctrl.target_speed > 0.0f && diff_speed < 0) || 
                (robot_ctrl.target_speed < 0.0f && diff_speed > 0))
            {
                if(diff_speed >650|| diff_speed < -650)//如果摇杆反向的幅度很大，说明用户是想急停或者急刹，这时候可以更激烈一点削弱
                {
                    robot_ctrl.target_speed *= SPEED_DECEL_RATIO-(ABS(diff_speed)-650) * 0.0002f-0.065; // 激烈削弱
                }
                else
                {
                    robot_ctrl.target_speed *= SPEED_DECEL_RATIO-ABS(diff_speed) * 0.0001f; // 正常削弱
                }
                
                // 2. 判断是否小于归零阈值
                if (robot_ctrl.target_speed > -SPEED_STOP_THRESHOLD && 
                    robot_ctrl.target_speed <  SPEED_STOP_THRESHOLD) 
                {
                    robot_ctrl.target_speed = 0.0f; // 直接归零
                    throttle_locked = 1;            // 触发锁定，不再接受加减速
                }
            }
            else
            {
                // 3. 正常加速逻辑 (同向，或是从 0 开始起步)
                robot_ctrl.target_speed += (float)diff_speed * K_SPEED_INC;
                
                // 速度限幅逻辑
                if (robot_ctrl.target_speed > MAX_SPEED_VAL) 
                    robot_ctrl.target_speed = MAX_SPEED_VAL;
                else if (robot_ctrl.target_speed < -MAX_SPEED_VAL) 
                    robot_ctrl.target_speed = -MAX_SPEED_VAL;
            }
        }
        // 【情况 C】摇杆在死区外，但 throttle_locked == 1 (已锁定)
        // 此时什么都不做，保持 target_speed = 0，强制要求用户松开摇杆
    }
    // --------------------------------------------------------
    // Step 6: 处理打点 (CH3 状态跳变检测)利用CH4和CH5的状态判断属于与哪一个type的打点，ch4为三态开关，ch5为两态开关
    // --------------------------------------------------------
    static uint8 last_ch3_state = 0; 
    uint8 curr_ch3_state = (ch3_mark > RC_SW_THRESHOLD) ? 1 : 0;

    // if (curr_ch3_state ==1){
    //     g_bridge_vision_task_enable = 1;//进入pcv控制调试使用
    // }
    // else{
    //     g_bridge_vision_task_enable = 0;
    // }

    // if (curr_ch3_state ==1){
    //     g_pvc_control_enable = 1;//进入pcv控制调试使用
    // }
    // else{
    //     g_pvc_control_enable = 0;
    // }
    

    // 检测状态跳变进行打点 (当前状态 != 上一次状态)
    // 这意味着无论是从0变1(上升沿)还是从1变0(下降沿)，都会触发
    if(ch3_mark ==0)
    {
        curr_ch3_state =last_ch3_state ;
    }
    if (curr_ch3_state != last_ch3_state )
    {
        //vision_detected_three_jump_point =1;//视觉控制的三级跳状态机，测试用
        //vision_detected_jump_point = 1;//跳跃点调用,测试用
        //vision_detected_bumpy_point = 1;//颠簸路段调用,测试用
        vision_detected_bridge_point = 1; // 单边桥调用,测试用
        //robot_ctrl.mark_trigger = 1; // 打点触发标记，Main函数处理完需手动清零
    // NAV_POINT_PATH = 0,     // 普通路径点
    // NAV_POINT_CIRCLE = 1,   // 转圈点
    // NAV_POINT_SLOPE = 2,    // 上坡点
    // NAV_POINT_JUMP = 3,     // 跳跃点
    // NAV_POINT_BRIDGE = 4,   // 单边桥点
    // NAV_POINT_BUMP = 5      // 颠簸路段点
        if (ch5_brake < RC_SW_THRESHOLD)
        {
            if (ch4_mode < RC_SW_MID_LOW)
            {
                robot_ctrl.point_type =0;//ch5 0 ch4 0
            #if DEBUG_LOG_ENABLE
                printf("Point Type: NAV_POINT_PATH\n");
            #endif
            }
            else if (ch4_mode > RC_SW_MID_HIGH)
            {
                robot_ctrl.point_type =2;//ch5 0 ch4 2
            #if DEBUG_LOG_ENABLE
                printf("Point Type: NAV_POINT_SLOPE\n");
            #endif
            }
            else
            {
                robot_ctrl.point_type =1;//ch5 0 ch4 1
            #if DEBUG_LOG_ENABLE
                printf("Point Type: NAV_POINT_CIRCLE\n");
            #endif
            }
        }
        else if(ch5_brake > RC_SW_THRESHOLD){
            if (ch4_mode < RC_SW_MID_LOW)
            {
                robot_ctrl.point_type = 3;//ch5 1 ch4 0
            #if DEBUG_LOG_ENABLE
                printf("Point Type: NAV_POINT_JUMP\n");
            #endif
            }
            else if (ch4_mode > RC_SW_MID_HIGH)
            {
                robot_ctrl.point_type = 5;//ch5 1 ch4 2
                #if DEBUG_LOG_ENABLE
                    printf("Point Type: NAV_POINT_BUMP\n");
                #endif
            }
            else
            {
                robot_ctrl.point_type = 4;//ch5 1 ch4 1
                #if DEBUG_LOG_ENABLE
                    printf("Point Type: NAV_POINT_BRIDGE\n");
                #endif
            }
        }
    }
    last_ch3_state = curr_ch3_state; 
}
