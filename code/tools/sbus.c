#include "sbus.h"
#include "config/config.h"//【提醒】配置请在这里修改
#include "../common.h"
// ==========================================
// 1. 宏定义 (参数配置区)
// ==========================================

// --- 摇杆中值 ---
#define RC_CH1_MID      1088    // 转向中值
#define RC_CH2_MID      976     // 油门中值
#define RC_DEADZONE     50      // 摇杆死区 (防止抖动漂移)

// --- 开关阈值 ---
#define RC_SW_THRESHOLD 1000    // 二态开关判定阈值

// CH4 三态开关判定区间 (低=192, 中=992, 高=1792)
#define RC_SW_MID_LOW   600     // <600 判为 LOW
#define RC_SW_MID_HIGH  1400    // >1400 判为 HIGH

// --- 增量系数 (灵敏度) ---
// 说明: 每次调用 Process 函数增加的数值 = (摇杆偏差值) * 系数
// 假设 Process 每 10ms 调用一次
#define K_STEER_INC     0.00225f  // 转向灵敏度
#define K_SPEED_INC     0.005f  // 速度灵敏度

// 积分限幅
#define MAX_STEER_ANGLE 45.0f   // 最大转向角度 (例如 +/- 45度)
#define MAX_SPEED_VAL   500.0f  // 最大速度目标值 (对应 target_speed_set)

// ==========================================
// 2. 全局变量定义
// ==========================================
robot_ctrl_t robot_ctrl;

// ==========================================
// 3. 函数实现
// ==========================================
uint8 Remote_control_connected =0;
// 初始化
void Remote_Control_Init(void)
{
    robot_ctrl.target_angle = 0.0f;
    robot_ctrl.target_speed = 0.0f;
    robot_ctrl.mark_trigger = 0;
    robot_ctrl.motor_enable = 1;  //1=使能,0=急停
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
        robot_ctrl.motor_enable = 0;//如果遥控器断联，直接停机
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
    if (ch6_off > RC_SW_THRESHOLD) 
    {
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
        // 积分计算
        robot_ctrl.target_angle += (float)diff_steer * K_STEER_INC;
        
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
    if (ch5_brake > RC_SW_THRESHOLD)//可能需要改为跳变
    {
        robot_ctrl.target_speed = 0.0f; // 刹车清零
    }
    else
    {
        int16 diff_speed = ch2_thro - RC_CH2_MID;
        
        if (abs(diff_speed) > RC_DEADZONE) 
        {
            float current_k_spd = K_SPEED_INC;
            
            // 积分计算
            robot_ctrl.target_speed += (float)diff_speed * current_k_spd;
            
            // 速度限幅逻辑
            if (robot_ctrl.target_speed > MAX_SPEED_VAL) 
                robot_ctrl.target_speed = MAX_SPEED_VAL;
            
            if (robot_ctrl.target_speed < -MAX_SPEED_VAL) 
                robot_ctrl.target_speed = -MAX_SPEED_VAL;
        }
    }

    // --------------------------------------------------------
    // Step 6: 处理打点 (CH3 状态跳变检测)利用CH4和CH5的状态判断属于与哪一个type的打点，ch4为三态开关，ch5为两态开关
    // --------------------------------------------------------
    static uint8 last_ch3_state = 0; 
    uint8 curr_ch3_state = (ch3_mark > RC_SW_THRESHOLD) ? 1 : 0;

    // 检测状态跳变进行打点 (当前状态 != 上一次状态)
    // 这意味着无论是从0变1(上升沿)还是从1变0(下降沿)，都会触发
    if (curr_ch3_state != last_ch3_state) 
    {
    robot_ctrl.mark_trigger = 1; // 置位，Main函数处理完需手动清零
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