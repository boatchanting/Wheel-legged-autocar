#include "menu.h"
#include "tools/sbus.h"
#include "calculate/ekf.h" // For g_initial_yaw
#include "calculate/matrix.h" // For euler_angle
#include <math.h>

// ==================== 全局变量定义 ====================
MenuState_t current_state = MENU_STATE_MAIN; 
uint8_t menu_index = MENU_STATE_SUBJECT;
// 新增：推车模式全局标志位（供底层PID控制环使用）
uint8_t g_is_push_mode = 0; // 0: 非推车模式，正常PID控制；1: 推车模式
uint8_t menu_values[MENU_STATE_COUNT] = {0};
const uint8_t menu_max_values[MENU_STATE_COUNT] = {
    1,  // MENU_STATE_MAIN
    4,  // MENU_STATE_SUBJECT: 1-3，推车模式4
    2,  // MENU_STATE_CALIBRATION: 1=自动, 2=手动
    2,  // MENU_STATE_ACTION_SELECT: 1=Record, 2=Start
    1,  // MENU_STATE_ACTION_CONFIRM
    1,  // MENU_STATE_ACTION_RUNNING
    1   // MENU_STATE_ACTION_COMPLETE
};

uint8_t need_redraw = 1;
float motor_speeds[2] = {0.0f, 0.0f};
float current_angles[4] = {0.0f, 0.0f, 0.0f, 0.0f};

#define MENU_KEY_UP                 KEY_1
#define MENU_KEY_LEFT               KEY_2
#define MENU_KEY_DOWN               KEY_3
#if (CAR_SELECT == 2) || (CAR_SELECT == 3)
#define MENU_KEY_RIGHT              KEY_5
#define MENU_KEY_ONE_CLICK_START    KEY_4
#define MENU_HAS_ONE_CLICK_START    1
#elif CAR_SELECT == 4
#define MENU_KEY_RIGHT              KEY_5
#define MENU_KEY_ONE_CLICK_START    KEY_4
#define MENU_HAS_ONE_CLICK_START    1
#else
#define MENU_KEY_RIGHT              KEY_4
#define MENU_HAS_ONE_CLICK_START    0
#endif

#define MENU_LOCAL_START_DELAY_MS   2000U
#define MENU_LOCAL_START_TICK_MS    10U
#define MENU_LOCAL_START_DELAY_TICKS (MENU_LOCAL_START_DELAY_MS / MENU_LOCAL_START_TICK_MS)

static volatile uint8_t menu_local_start_pending = 0;
static volatile uint16_t menu_local_start_delay_ticks = 0;

void Menu_TriggerRecordAction(void)
{
    gpio_toggle_level(P19_0);       // 指示灯切换
    if (g_motor_enable && g_yaw_initialized)
    {
        // 倒地打点逻辑：如果处于倒地状态，直接起立并给一个0.5m/s的初速度和直线方向
        if (g_fallen) {
            g_fallen = false;
            robot_ctrl.target_speed = 100.0f; // 0.5m/s 向前
            // err_degree = -target_angle + g_initial_yaw - euler_angle.yaw;
            // 为使误差为0(保持当前车头朝向)，需设置 target_angle = g_initial_yaw - euler_angle.yaw
            robot_ctrl.target_angle = g_initial_yaw - euler_angle.yaw; 
        }

        g_nav_start_recording = 1;  // 开始录制，会在 main 中调用惯导系统初始化和点记录初始化
        g_nav_recording = 1;
    }
}

void Menu_TriggerStartAction(void)
{
    gpio_toggle_level(P19_0);       // 指示灯切换
    if (g_motor_enable)
    {
        g_fallen = false;           // 【新增】倒地状态下允许直接起立发车
        g_load_flash_request = 1;   // 请求读取测试
        g_save_flash_request = 0;   // 清除保存请求
        g_nav_recording = 0;        // 确保停止录制
    }
}

#if (LAUNCH_STRATEGY_SELECT == 1)
// =================================================================================
// 【直立发车 / 航向校准】状态机实现
// =================================================================================
volatile UprightLaunchState_e g_upright_state = UPRIGHT_LAUNCH_IDLE;
static volatile uint16_t s_upright_timer_ticks = 0; // 10ms 递减计时器

// 2秒航向角采样累加器 (矢量三角均值)
static uint16_t s_sample_count = 0;
static float s_sample_sum_sin = 0.0f;
static float s_sample_sum_cos = 0.0f;

static void UprightLaunch_Update_10ms(void)
{
    // 倒地时自动重置回待机，确保安全
    if (g_fallen && g_upright_state != UPRIGHT_LAUNCH_IDLE && g_upright_state != UPRIGHT_LAUNCH_WAIT_STANDUP)
    {
        g_upright_state = UPRIGHT_LAUNCH_IDLE;
        g_turn_loop_disabled = false;
        s_upright_timer_ticks = 0;
    }

    if (s_upright_timer_ticks > 0)
    {
        s_upright_timer_ticks--;
    }

    switch (g_upright_state)
    {
        case UPRIGHT_LAUNCH_WAIT_STANDUP:
            if (s_upright_timer_ticks == 0)
            {
                // 1) 延时1秒后触发起立
                if (g_motor_enable)
                {
                    g_fallen = false; // 触发起立自平衡
                }
                g_upright_state = UPRIGHT_LAUNCH_WAIT_STABILIZE;
                s_upright_timer_ticks = 100U; // 2) 再延时1秒 (100 * 10ms = 1000ms) 等待自平衡稳定
            }
            break;

        case UPRIGHT_LAUNCH_WAIT_STABILIZE:
            if (s_upright_timer_ticks == 0)
            {
                // 3) 站稳后请求主循环播放长-短-长提示音
                g_upright_state = UPRIGHT_LAUNCH_PLAY_BEEP_PREP;
                g_upright_long_short_long_request = 1U;
            }
            break;

        case UPRIGHT_LAUNCH_PLAY_BEEP_PREP:
            if (g_upright_beep_done != 0U)
            {
                g_upright_beep_done = 0U;
                // 4) 响完之后，关闭转向环，供手推校准方向
                g_turn_loop_disabled = true;
                g_upright_state = UPRIGHT_LAUNCH_MANUAL_AIMING;
            }
            break;

        case UPRIGHT_LAUNCH_WAIT_SAMPLE_DELAY:
            if (s_upright_timer_ticks == 0)
            {
                // 5) 0.5s 撤手延时到，先响一声短鸣，然后开启 2 秒采样
                g_upright_single_beep_request = 1U;
                s_sample_count = 0U;
                s_sample_sum_sin = 0.0f;
                s_sample_sum_cos = 0.0f;
                g_upright_state = UPRIGHT_LAUNCH_SAMPLING_2S;
                s_upright_timer_ticks = 200U; // 200 * 10ms = 2000ms (2秒采样)
            }
            break;

        case UPRIGHT_LAUNCH_SAMPLING_2S:
            {
                // 在2秒期间持续累加偏航角矢量 (10ms采样一次)
                float rad = euler_angle.yaw * 0.0174532925f; // DEG2RAD
                s_sample_sum_sin += sinf(rad);
                s_sample_sum_cos += cosf(rad);
                s_sample_count++;

                if (s_upright_timer_ticks == 0)
                {
                    // 6) 2秒采样完成，计算矢量三角均值并锁定航向
                    float mean_yaw;
                    if ((s_sample_count > 0U) &&
                        ((fabsf(s_sample_sum_sin) > 1e-6f) || (fabsf(s_sample_sum_cos) > 1e-6f)))
                    {
                        mean_yaw = atan2f(s_sample_sum_sin, s_sample_sum_cos) * 57.2957795f; // RAD2DEG
                    }
                    else
                    {
                        mean_yaw = euler_angle.yaw;
                    }

                    g_initial_yaw = mean_yaw;
                    robot_ctrl.target_angle = 0.0f;
                    err_degree = 0.0f;
                    PID_Param_Init();             // 清零转向PID积分与历史误差
                    g_turn_loop_disabled = false; // 重新开启转向环，锁死当前航向
                    g_upright_single_beep_request = 1U; // 锁定成功，主循环响一声短鸣提示
                    g_upright_state = UPRIGHT_LAUNCH_HEADING_LOCKED;
                }
            }
            break;

        case UPRIGHT_LAUNCH_WAIT_START_2S:
            if (s_upright_timer_ticks == 0)
            {
                // 7) 延时2秒后发车
                g_upright_state = UPRIGHT_LAUNCH_IDLE;
                Menu_TriggerStartAction();
            }
            break;

        default:
            break;
    }
}
#endif

// ==================== 辅助函数 ====================
static void Menu_RequestLocalStartAction(void)
{
    if (menu_local_start_pending)
    {
        return;
    }

    menu_local_start_delay_ticks = MENU_LOCAL_START_DELAY_TICKS;
    menu_local_start_pending = 1;
}

static void Menu_UpdateLocalStartAction(void)
{
    if (!menu_local_start_pending)
    {
        return;
    }

    if (menu_local_start_delay_ticks > 0)
    {
        menu_local_start_delay_ticks--;
    }

    if (menu_local_start_delay_ticks != 0)
    {
        return;
    }

    menu_local_start_pending = 0;
    Menu_TriggerStartAction();
}

static void Menu_ClearScreen(void)
{
    ips200_clear();
}
static void State_Static_Screen(void)
{
     // 基本UI框架
    ips200_show_string(0,15*4, "Pitch:");
    ips200_show_string(0,15*5, "Roll :");
    ips200_show_string(0,15*6, "Yaw  :");
    ips200_show_string(0,15*7, "State:"); 
    ips200_show_string(0, 15*8, "RF:");
    ips200_show_string(0, 15*9, "RR:");
    ips200_show_string(0, 15*10, "LF:");
    ips200_show_string(0, 15*11, "LR:");
    ips200_show_string(0, 15*12, "Motor Speed:");
    ips200_show_string(0, 15*13, "L:");
    ips200_show_string(100, 15*13, "R:");
    ips200_show_string(0, 15*15, "g_motor_enable");
}
static void State_Dynamic_Screen(void)
{
     // 显示动态数据（和主界面一样）
    ips200_show_float(60,15*4, euler_angle.pitch, 3, 2);
    ips200_show_float(60,15*5, euler_angle.roll, 3, 2);
    ips200_show_float(60,15*6, euler_angle.yaw, 3, 2);
#if (LAUNCH_STRATEGY_SELECT == 1)
    if (g_upright_state == UPRIGHT_LAUNCH_WAIT_STANDUP)
    {
        ips200_show_string(60, 15*7, "Stand 1s  ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_WAIT_STABILIZE)
    {
        ips200_show_string(60, 15*7, "Stab 1s   ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_PLAY_BEEP_PREP)
    {
        ips200_show_string(60, 15*7, "Beeping   ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_MANUAL_AIMING)
    {
        ips200_show_string(60, 15*7, "AIM YAW   ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_WAIT_SAMPLE_DELAY)
    {
        ips200_show_string(60, 15*7, "Wait 0.5s ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_SAMPLING_2S)
    {
        ips200_show_string(60, 15*7, "SAMP 2.0s ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_HEADING_LOCKED)
    {
        ips200_show_string(60, 15*7, "YAW LCK   ");
    }
    else if (g_upright_state == UPRIGHT_LAUNCH_WAIT_START_2S)
    {
        ips200_show_string(60, 15*7, "GO IN 2s  ");
    }
    else
    {
        ips200_show_float(60,15*7, gnss.state, 3, 2);ips200_show_uint(80,15*7,gnss.satellite_used,5);
    }
#else
    ips200_show_float(60,15*7, gnss.state, 3, 2);ips200_show_uint(80,15*7,gnss.satellite_used,5);
#endif
    ips200_show_float(25, 15*8, current_angles[0], 3, 1);
    ips200_show_float(25, 15*9, current_angles[1], 3, 1);
    ips200_show_float(25, 15*10, current_angles[2], 3, 1);
    ips200_show_float(25, 15*11, current_angles[3], 3, 1);
    ips200_show_float(25, 15*13, motor_speeds[0], 5, 1);
    ips200_show_float(125, 15*13, motor_speeds[1], 5, 1);
    ips200_show_string(155, 15*15, g_motor_enable ? "Yes" : "No");
    
}
static void Menu_ShowMainScreen(void)
{
    Menu_ClearScreen();
    if(current_state == MENU_STATE_MAIN)
    {
    State_Static_Screen();
    // 提示信息
    ips200_show_string(0, 15*16, "Press any key to menu");
    }
    else
    {
    State_Static_Screen();
    }
}

static void Menu_ShowSubjectScreen(void)
{
    Menu_ClearScreen();
    ips200_show_string(80, 0, "Select Subject");
    ips200_draw_line(10, 20, 230, 20, RGB565_RED);
    
    uint8_t subject = menu_values[MENU_STATE_SUBJECT];
    
    ips200_set_color((subject == 1) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 40, "Subject 1");
    
    ips200_set_color((subject == 2) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0,60, "Subject 2");
    
    ips200_set_color((subject == 3) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 80, "Subject 3");
      // 新增：第4个选项 推车模式
    ips200_set_color((subject == 4) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 100, "Push Mode"); 
    
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_draw_line(10, 120, 230, 120, RGB565_RED);
}

static void Menu_ShowCalibrationScreen(void)
{
    Menu_ClearScreen();
    ips200_show_string(80, 0, "Calibration Mode");
    ips200_draw_line(10, 20, 230, 20, RGB565_RED);
    
    uint8_t calib = menu_values[MENU_STATE_CALIBRATION];
    
    ips200_set_color((calib == 1) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 40, "Auto Calibration");
    
    ips200_set_color((calib == 2) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0,60, "Manual Calibration");
    
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_draw_line(10, 80, 230, 80, RGB565_RED);
}

static void Menu_ShowActionSelectScreen(void)
{
    Menu_ClearScreen();
    ips200_show_string(80, 0, "Select Action");
    ips200_draw_line(10, 20, 230, 20, RGB565_RED);
    
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    
    ips200_set_color((action == 1) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 40, "Record");
    
    ips200_set_color((action == 2) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0,60, "Start");
    
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_draw_line(10, 80, 230, 80, RGB565_RED);
}

static void Menu_ShowActionConfirmScreen(void)
{
    // 显示选择的动作
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    uint8_t subject = menu_values[MENU_STATE_SUBJECT]; // 获取当前科目
    
    if (subject == 4) {
        ips200_show_string(80, 0, "Push Mode...");
    } else {
        ips200_show_string(80, 0, (action == 1) ? "Recording..." : "Starting...");
    }
    ips200_draw_line(10, 20, 230, 20, RGB565_RED);
    
    // 显示动态数据（和主界面一样）
    ips200_show_float(60,15*4, euler_angle.pitch, 3, 2);
    ips200_show_float(60,15*5, euler_angle.roll, 3, 2);
    ips200_show_float(60,15*6, euler_angle.yaw, 3, 2);

    ips200_show_float(60, 15*7, gnss.state, 3, 2);
    ips200_show_float(25, 135, current_angles[0], 3, 1);
    ips200_show_float(25, 150, current_angles[1], 3, 1);
    ips200_show_float(25, 165, current_angles[2], 3, 1);
    ips200_show_float(25, 180, current_angles[3], 3, 1);
    
    ips200_show_float(25, 215, motor_speeds[0], 5, 1);
    ips200_show_float(125, 215, motor_speeds[1], 5, 1);
    ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    
    // 下面打印"未发车"
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    if (subject == 4) {
        ips200_show_string(0, 15*16, "Pushing Phase Active"); // 显示为推车阶段
    } else {
        ips200_show_string(0, 15*16, "Not Started          ");
    }
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
}

static void Menu_ShowActionRunningScreen(void)
{
    // 显示动态数据（和ACTION_CONFIRM一样）
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    uint8_t subject = menu_values[MENU_STATE_SUBJECT]; // 获取当前科目

    if (subject == 4) {
        ips200_show_string(80, 0, "Push Mode...");
    } else {
        ips200_show_string(80, 0, (action == 1) ? "Recording..." : "Starting...");
    }
    ips200_draw_line(10, 20, 230, 20, RGB565_BLUE);
    
    State_Dynamic_Screen();// 显示动态数据（和主界面一样）
    // 下面打印"发车成功"
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    if (subject == 4) {
        ips200_show_string(0, 15*16, "Pushing Phase Active"); // 显示为推车阶段
    } else {
        ips200_show_string(0, 15*16, "Started Successfully");
    }
}

static void Menu_ShowActionCompleteScreen(void)
{
    Menu_ClearScreen();
    
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    uint8_t subject = menu_values[MENU_STATE_SUBJECT]; // 获取当前科目
    
    if (subject == 4)
    {
        ips200_show_string(0, 15*1, "Push Mode Ended");
        ips200_show_string(0,15*2, "System returned");
    }
    else if (action == 1)  // Record
    {
        ips200_show_string(0, 15*1, "Record Complete");
        ips200_show_string(0,15*2, "Data saved successfully");
    }
    else if (action == 2)  // Start
    {
        ips200_show_string(0, 15*1, "Start Complete");
        ips200_show_string(0,15*2, "System paused");
    }
     ips200_show_string(0,15*3, "Press any key to return");

}

// ==================== 主显示函数 ====================
void Menu_ShowStatic(void)
{
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    if (!need_redraw) return;
    
    switch (current_state)
    {
        case MENU_STATE_MAIN:
            Menu_ShowMainScreen();
            break;
        case MENU_STATE_SUBJECT:
            Menu_ShowSubjectScreen();
            break;
        case MENU_STATE_CALIBRATION:
            Menu_ShowCalibrationScreen();
            break;
        case MENU_STATE_ACTION_SELECT:
            Menu_ShowActionSelectScreen();
            break;
        case MENU_STATE_ACTION_CONFIRM:
           Menu_ShowMainScreen(); // 先显示主界面框架
            break;
        case MENU_STATE_ACTION_RUNNING:
            if (menu_values[MENU_STATE_SUBJECT] == 4)
            {
                gpio_toggle_level(P19_0);
                g_is_push_mode = 1; // 打开底层推车控制逻辑！！！
            }
            else if(action==1)
            {
                Menu_TriggerRecordAction();
            }
            else if(action==2)
            {
                Menu_RequestLocalStartAction();
            }
            Menu_ShowMainScreen(); // 先显示主界面框架
            break;
        case MENU_STATE_ACTION_COMPLETE:
            // 不管什么模式，到了完成界面都关闭推车模式
            g_is_push_mode = 0; 
            if(action==1)
            {
                gpio_toggle_level(P19_0);       // 指示灯切换
                if (g_nav_recording)
                {
                    g_nav_recording = 0; // 停止录制
                    // 只有电机仍开启才保存（防止倒地保存）
                    if (g_motor_enable) 
                    {
                        g_save_flash_request = 1; // 通知 main 循环执行 Flash 写操作。不能在中断中进行写入，以免阻塞中断
                    }
                    else
                    {
                        #if DEBUG_LOG_ENABLE
                        printf("Button2: Recording stopped (motor disabled), data discarded.\r\n");
                        #endif
                    }
                }
            }
            if(action==2)
            {
                gpio_toggle_level(P19_0);       // 指示灯切换
            }
            Menu_ShowActionCompleteScreen();
            break;
        default:
            break;
    }
    
    need_redraw = 0;
}

void Menu_ShowDynamic(void)
{
                
    servo_get_current_angles(current_angles);
    // 获取电机速度数据
    //small_driver_get_speed();
    motor_speeds[0] = motor_value.receive_left_speed_data;
    motor_speeds[1] = motor_value.receive_right_speed_data;
    // 在ACTION_RUNNING状态下显示动态数据
    if (current_state == MENU_STATE_ACTION_RUNNING)
    {
        Menu_ShowActionRunningScreen();
    }
    else if(current_state == MENU_STATE_ACTION_CONFIRM)
    {
        Menu_ShowActionConfirmScreen();
    }
    // 在主界面显示动态数据
    else if (current_state == MENU_STATE_MAIN)
    {
        State_Dynamic_Screen();
    }
}

// ==================== 按键处理（完全按照您的设计） ====================
// ==================== 按键处理（状态机重写版） ====================
void Menu_HandleKey(void)
{
#if (LAUNCH_STRATEGY_SELECT == 1)
    // 0. 更新直立发车状态机
    UprightLaunch_Update_10ms();

    // 方案 B：最高优先级拦截并响应 P10_0 (KEY_1) 与 P10_4 (KEY_4)
    // -------------------------------------------------------------
    // 按键 1: P10_0 (KEY_1) - 延时起立 / 航向锁定
    // -------------------------------------------------------------
    if (key_get_state(KEY_1) == KEY_SHORT_PRESS)
    {
        key_clear_state(KEY_1);
        if (g_upright_state == UPRIGHT_LAUNCH_IDLE)
        {
            g_upright_state = UPRIGHT_LAUNCH_WAIT_STANDUP;
            s_upright_timer_ticks = 100U; // 1000ms (100 * 10ms) 延时准备起立
            return;
        }
        else if (g_upright_state == UPRIGHT_LAUNCH_MANUAL_AIMING)
        {
            // 第二次按 10_0：先进入 0.5s 撤手延时 (50 * 10ms = 500ms)
            g_upright_state = UPRIGHT_LAUNCH_WAIT_SAMPLE_DELAY;
            s_upright_timer_ticks = 50U;
            return;
        }
    }

    // -------------------------------------------------------------
    // 按键 4: P10_4 (KEY_4) - 发车
    // -------------------------------------------------------------
    #if MENU_HAS_ONE_CLICK_START
    if (key_get_state(MENU_KEY_ONE_CLICK_START) == KEY_SHORT_PRESS)
    {
        key_clear_state(MENU_KEY_ONE_CLICK_START);
        if (g_upright_state == UPRIGHT_LAUNCH_HEADING_LOCKED)
        {
            // 已锁定航向，按下 10_4 进入 2 秒延时发车
            g_upright_state = UPRIGHT_LAUNCH_WAIT_START_2S;
            s_upright_timer_ticks = 200U; // 2000ms (200 * 10ms)
            return;
        }
        else if (g_upright_state == UPRIGHT_LAUNCH_IDLE)
        {
            // 原有一键倒地发车
            Menu_RequestLocalStartAction();
            return;
        }
    }
    #endif

    // 如果处于直立发车流程中，拦截其他按键对菜单的干扰
    if (g_upright_state != UPRIGHT_LAUNCH_IDLE)
    {
        return;
    }
#else
    #if MENU_HAS_ONE_CLICK_START
    if (key_get_state(MENU_KEY_ONE_CLICK_START) == KEY_SHORT_PRESS)
    {
        Menu_RequestLocalStartAction();
        key_clear_state(MENU_KEY_ONE_CLICK_START);
        return;
    }
    #endif
#endif

    Menu_UpdateLocalStartAction();
    if (menu_local_start_pending)
    {
        return;
    }

    // 1. 在主界面：任意键进入菜单
    if (current_state == MENU_STATE_MAIN)
    {
        if (key_get_state(MENU_KEY_UP) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_LEFT) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_DOWN) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_RIGHT) == KEY_SHORT_PRESS)
        {
            current_state = MENU_STATE_SUBJECT;
            menu_index = MENU_STATE_SUBJECT;
            menu_values[menu_index] = 1;
            need_redraw = 1;
            key_clear_all_state();
        }
        return;
    }
    
    // 2. 在完成界面：任意键返回主界面
    if (current_state == MENU_STATE_ACTION_COMPLETE)
    {
        if (key_get_state(MENU_KEY_UP) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_LEFT) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_DOWN) == KEY_SHORT_PRESS ||
            key_get_state(MENU_KEY_RIGHT) == KEY_SHORT_PRESS)
        {
            current_state = MENU_STATE_MAIN;
            menu_index = MENU_STATE_SUBJECT;
            need_redraw = 1;
            key_clear_all_state();
        }
        return;
    }
    
    // ==========================================================
    // 3. 左键：返回 / 锁住
    // ==========================================================
    if (key_get_state(MENU_KEY_LEFT) == KEY_SHORT_PRESS)
    {
        switch (current_state)
        {
            case MENU_STATE_SUBJECT:
                current_state = MENU_STATE_MAIN; // 退回主界面
                break;
                
            case MENU_STATE_CALIBRATION:
                current_state = MENU_STATE_SUBJECT; // 退回科目选择
                break;
                
            case MENU_STATE_ACTION_SELECT:
                current_state = MENU_STATE_CALIBRATION; // 退回标定选择
                break;
                
            case MENU_STATE_ACTION_CONFIRM:
                current_state = MENU_STATE_ACTION_SELECT; // 退回动作选择
                break;
                
            case MENU_STATE_ACTION_RUNNING:
                // 【核心退出逻辑：如果是推车模式，直接退回科目选择并锁死！】
                if (menu_values[MENU_STATE_SUBJECT] == 4) {
                    current_state = MENU_STATE_SUBJECT; 
                    g_is_push_mode = 0; // 强行清0，车轮瞬间变硬锁死！
                } else {
                    current_state = MENU_STATE_ACTION_CONFIRM; // 普通模式退回确认界面
                }
                break;
                
            default:
                break;
        }
        menu_index = current_state;
        need_redraw = 1;
        key_clear_state(MENU_KEY_LEFT);
    }
    
    // ==========================================================
    // 4. 右键：前进 / 开启推车模式
    // ==========================================================
    else if (key_get_state(MENU_KEY_RIGHT) == KEY_SHORT_PRESS)
    {
        switch (current_state)
        {
            case MENU_STATE_SUBJECT:
                // 【核心跳转逻辑：如果选了4，直接飞越到运行界面！】
                if (menu_values[MENU_STATE_SUBJECT] == 4) {
                    current_state = MENU_STATE_ACTION_RUNNING; 
                } else {
                    current_state = MENU_STATE_CALIBRATION; // 选1,2,3则正常进入标定选择
                }
                
                menu_index = current_state;
                if (menu_values[menu_index] == 0) menu_values[menu_index] = 1;
                break;
                
            case MENU_STATE_CALIBRATION:
                current_state = MENU_STATE_ACTION_SELECT;
                menu_index = current_state;
                if (menu_values[menu_index] == 0) menu_values[menu_index] = 1;
                break;
                
            case MENU_STATE_ACTION_SELECT:
                current_state = MENU_STATE_ACTION_CONFIRM;
                menu_index = current_state;
                break;
                
            case MENU_STATE_ACTION_CONFIRM:
                current_state = MENU_STATE_ACTION_RUNNING;
                menu_index = current_state;
                break;
                
            case MENU_STATE_ACTION_RUNNING:
                current_state = MENU_STATE_ACTION_COMPLETE; // 运行中按右键，进入完成界面（自动锁车）
                menu_index = current_state;
                break;
                
            default:
                break;
        }
        need_redraw = 1;
        key_clear_state(MENU_KEY_RIGHT);
    }
    
    // ==========================================================
    // 5. 上下键：调整数值
    // ==========================================================
    else if (key_get_state(MENU_KEY_UP) == KEY_SHORT_PRESS && current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] > 1) { menu_values[menu_index]--; }
            else { menu_values[menu_index] = menu_max_values[menu_index]; }
            need_redraw = 1;
        }
        key_clear_state(MENU_KEY_UP);
    }
    else if (key_get_state(MENU_KEY_DOWN) == KEY_SHORT_PRESS && current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] < menu_max_values[menu_index]) { menu_values[menu_index]++; }
            else { menu_values[menu_index] = 1; }
            need_redraw = 1;
        }
        key_clear_state(MENU_KEY_DOWN);
    }
}

// ==================== 初始化 ====================
void Menu_Init(void)
{
    current_state = MENU_STATE_MAIN;
    menu_index = MENU_STATE_SUBJECT;
    //action_step = 0;
    menu_local_start_pending = 0;
    menu_local_start_delay_ticks = 0;
    
    for (uint8_t i = 0; i < MENU_STATE_COUNT; i++)
    {
        menu_values[i] = 1;
    }
    
    need_redraw = 1;
}
