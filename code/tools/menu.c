#include "menu.h"


// ==================== 全局变量定义 ====================
MenuState_t current_state = MENU_STATE_MAIN;
uint8_t menu_index = MENU_STATE_SUBJECT;

uint8_t menu_values[MENU_STATE_COUNT] = {0};
const uint8_t menu_max_values[MENU_STATE_COUNT] = {
    1,  // MENU_STATE_MAIN
    3,  // MENU_STATE_SUBJECT: 1-3
    2,  // MENU_STATE_CALIBRATION: 1=自动, 2=手动
    2,  // MENU_STATE_ACTION_SELECT: 1=Record, 2=Start
    1,  // MENU_STATE_ACTION_CONFIRM
    1,  // MENU_STATE_ACTION_RUNNING
    1   // MENU_STATE_ACTION_COMPLETE
};

uint8_t need_redraw = 1;
float motor_speeds[2] = {0.0f, 0.0f};
float current_angles[4] = {0.0f, 0.0f, 0.0f, 0.0f};

// 动作步骤计数器
//static uint8_t //action_step = 0;

// ==================== 辅助函数 ====================
static void Menu_ClearScreen(void)
{
    ips200_clear();
}

static void Menu_ShowMainScreen(void)
{
    Menu_ClearScreen();
    if(current_state == MENU_STATE_MAIN)
    {
    // 基本UI框架
    ips200_show_string(0, 0,  "EKF Monitor");
    ips200_show_string(0, 30, "Pitch:");
    ips200_show_string(0, 50, "Roll :");
    ips200_show_string(0, 70, "Yaw  :");
    ips200_show_string(0, 100,"Freq : 20Hz");
    ips200_show_string(0, 120, "Servo Angles:");
    ips200_show_string(0, 135, "RF:");
    ips200_show_string(0, 150, "RR:");
    ips200_show_string(0, 165, "LF:");
    ips200_show_string(0, 180, "LR:");
    ips200_show_string(0, 200, "Motor Speed:");
    ips200_show_string(0, 215, "L:");
    ips200_show_string(80, 215, "R:");
    ips200_show_string(0, 230, "gyro.kp");
    ips200_show_string(0, 245, "gyro.kd");
    ips200_show_string(0, 260, "g_motor_enable");
    // 提示信息
    ips200_show_string(0, 280, "Press any key to menu");
    }
    else
    {
    ips200_show_string(0, 30, "Pitch:");
    ips200_show_string(0, 50, "Roll :");
    ips200_show_string(0, 70, "Yaw  :");
    ips200_show_string(0, 100,"Freq : 20Hz");
    ips200_show_string(0, 120, "Servo Angles:");
    ips200_show_string(0, 135, "RF:");
    ips200_show_string(0, 150, "RR:");
    ips200_show_string(0, 165, "LF:");
    ips200_show_string(0, 180, "LR:");
    ips200_show_string(0, 200, "Motor Speed:");
    ips200_show_string(0, 215, "L:");
    ips200_show_string(80, 215, "R:");
    ips200_show_string(0, 230, "gyro.kp");
    ips200_show_string(0, 245, "gyro.kd");
    ips200_show_string(0, 260, "g_motor_enable");
    
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
    ips200_show_string(0, 60, "Subject 2");
    
    ips200_set_color((subject == 3) ? RGB565_RED : RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 80, "Subject 3");
    
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_draw_line(10, 100, 230, 100, RGB565_RED);
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
    ips200_show_string(0, 60, "Manual Calibration");
    
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
    ips200_show_string(0, 60, "Start");
    
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_draw_line(10, 80, 230, 80, RGB565_RED);
}

static void Menu_ShowActionConfirmScreen(void)
{
    // 显示选择的动作
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    
    ips200_show_string(80, 0, (action == 1) ? "Record Mode" : "Start Mode");
    ips200_draw_line(10, 20, 230, 20, RGB565_RED);
    
    // 显示动态数据（和主界面一样）
    ips200_show_float(60, 30, euler_angle.pitch, 3, 2);
    ips200_show_float(60, 50, euler_angle.roll, 3, 2);
    ips200_show_float(60, 70, euler_angle.yaw, 3, 2);
    
    ips200_show_float(25, 135, current_angles[0], 3, 1);
    ips200_show_float(25, 150, current_angles[1], 3, 1);
    ips200_show_float(25, 165, current_angles[2], 3, 1);
    ips200_show_float(25, 180, current_angles[3], 3, 1);
    
    ips200_show_float(25, 215, motor_speeds[0], 5, 1);
    ips200_show_float(105, 215, motor_speeds[1], 5, 1);
    
    ips200_show_float(25, 230, pid_gyro.kp, 4, 2);
    ips200_show_float(25, 245, pid_gyro.kd, 4, 2);
    
    ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    
    // 下面打印"未发车"
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(0, 280, "Not Started          ");
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
}

static void Menu_ShowActionRunningScreen(void)
{
    // 显示动态数据（和ACTION_CONFIRM一样）
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    
    ips200_show_string(80, 0, (action == 1) ? "Recording..." : "Starting...");
    ips200_draw_line(10, 20, 230, 20, RGB565_BLUE);
    
    ips200_show_float(60, 30, euler_angle.pitch, 3, 2);
    ips200_show_float(60, 50, euler_angle.roll, 3, 2);
    ips200_show_float(60, 70, euler_angle.yaw, 3, 2);
    
    ips200_show_float(25, 135, current_angles[0], 3, 1);
    ips200_show_float(25, 150, current_angles[1], 3, 1);
    ips200_show_float(25, 165, current_angles[2], 3, 1);
    ips200_show_float(25, 180, current_angles[3], 3, 1);
    
    ips200_show_float(25, 215, motor_speeds[0], 5, 1);
    ips200_show_float(105, 215, motor_speeds[1], 5, 1);
    
    ips200_show_float(25, 230, pid_gyro.kp, 4, 2);
    ips200_show_float(25, 245, pid_gyro.kd, 4, 2);
    
    ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    
    // 下面打印"发车成功"
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(0, 280, "Started Successfully");
}

static void Menu_ShowActionCompleteScreen(void)
{
    Menu_ClearScreen();
    
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    
    if (action == 1)  // Record
    {
        ips200_show_string(0, 10, "Record Complete");
        ips200_show_string(0, 30, "Data saved successfully");
    }
    else if (action == 2)  // Start
    {
        ips200_show_string(0, 10, "Start Complete");
        ips200_show_string(0, 30, "System paused");
    }
     ips200_show_string(0, 50, "Press any key to return");

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
            if(action==1)
            {        
                gpio_toggle_level(P19_0);       // 指示灯切换
                if (g_motor_enable && g_yaw_initialized)
                {      
                g_nav_start_recording = 1;//开始录制，会在main中调用惯导系统初始化和点的记录的初始化,然后再变回1
                g_nav_recording = 1;
                }
            }
            if(action==2)
            {
                gpio_toggle_level(P19_0);       // 指示灯切换
                if(g_motor_enable)
                {
                g_load_flash_request = 1;      // 请求读取测试
                g_save_flash_request = 0;     // 清除保存请求
                g_nav_recording = 0;          // 确保停止录制
            }
            }
            Menu_ShowMainScreen(); // 先显示主界面框架
            break;
        case MENU_STATE_ACTION_COMPLETE:
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
        ips200_show_float(60, 30, euler_angle.pitch, 3, 2);
        ips200_show_float(60, 50, euler_angle.roll, 3, 2);
        ips200_show_float(60, 70, euler_angle.yaw, 3, 2);
        
        ips200_show_float(25, 135, current_angles[0], 3, 1);
        ips200_show_float(25, 150, current_angles[1], 3, 1);
        ips200_show_float(25, 165, current_angles[2], 3, 1);
        ips200_show_float(25, 180, current_angles[3], 3, 1);
        
        ips200_show_float(25, 215, motor_speeds[0], 5, 1);
        ips200_show_float(105, 215, motor_speeds[1], 5, 1);
        
        ips200_show_float(25, 230, pid_gyro.kp, 4, 2);
        ips200_show_float(25, 245, pid_gyro.kd, 4, 2);
        
        ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    }
}

// ==================== 按键处理（完全按照您的设计） ====================
void Menu_HandleKey(void)
{
    // 进入菜单：任意键短按
    if (current_state == MENU_STATE_MAIN)
    {
        if (key_get_state(KEY_1) == KEY_SHORT_PRESS ||
            key_get_state(KEY_2) == KEY_SHORT_PRESS ||
            key_get_state(KEY_3) == KEY_SHORT_PRESS ||
            key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            current_state = MENU_STATE_SUBJECT;
            menu_index = MENU_STATE_SUBJECT;
            menu_values[menu_index] = 1;
            ////action_step = 0;
            need_redraw = 1;
            
            key_clear_state(KEY_1);
            key_clear_state(KEY_2);
            key_clear_state(KEY_3);
            key_clear_state(KEY_4);
        }
        return;
    }
    
    // 完成界面：可以按任意键立即返回
    if (current_state == MENU_STATE_ACTION_COMPLETE)
    {
        if (key_get_state(KEY_1) == KEY_SHORT_PRESS ||
            key_get_state(KEY_2) == KEY_SHORT_PRESS ||
            key_get_state(KEY_3) == KEY_SHORT_PRESS ||
            key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            current_state = MENU_STATE_MAIN;
            menu_index = MENU_STATE_SUBJECT;
            //action_step = 0;
            need_redraw = 1;
            
            key_clear_state(KEY_1);
            key_clear_state(KEY_2);
            key_clear_state(KEY_3);
            key_clear_state(KEY_4);
        }
        return;
    }
    
    // 左键：返回上一步
    if (key_get_state(KEY_2) == KEY_SHORT_PRESS)
    {
        if (current_state == MENU_STATE_ACTION_CONFIRM)
        {
            // 从确认界面返回选择界面
            current_state = MENU_STATE_ACTION_SELECT;
            menu_index = current_state;
            //action_step = 0;
        }
        else if (current_state == MENU_STATE_ACTION_RUNNING)
        {
            // 从运行界面返回确认界面
            current_state = MENU_STATE_ACTION_CONFIRM;
            menu_index = current_state;
            //action_step = 1;  // 保持在第一步
        }
        else if (current_state > MENU_STATE_SUBJECT)
        {
            // 普通返回
            current_state--;
            menu_index = current_state;
            if (current_state <= MENU_STATE_ACTION_SELECT)
            {
                //action_step = 0;
            }
        }
        else if (current_state == MENU_STATE_SUBJECT)
        {
            // 返回到主界面
            current_state = MENU_STATE_MAIN;
            menu_index = MENU_STATE_SUBJECT;
            //action_step = 0;
        }
        
        need_redraw = 1;
        key_clear_state(KEY_2);
    }
    // 右键：前进
    else if (key_get_state(KEY_4) == KEY_SHORT_PRESS)
    {
        // ACTION_SELECT后的多步骤流程
        if (current_state == MENU_STATE_ACTION_SELECT)
        {
            // 第一步：进入确认界面（显示未发车）
            current_state = MENU_STATE_ACTION_CONFIRM;
            menu_index = current_state;
            //action_step = 1;
        }
        else if (current_state == MENU_STATE_ACTION_CONFIRM)
        {
            // 第二步：进入运行界面（显示发车成功）
            current_state = MENU_STATE_ACTION_RUNNING;
            menu_index = current_state;
            //action_step = 2;
        }
        else if (current_state == MENU_STATE_ACTION_RUNNING)
        {
            // 第三步：进入完成界面
            current_state = MENU_STATE_ACTION_COMPLETE;
            menu_index = current_state;
            //action_step = 0;
        }
        else if (current_state < MENU_STATE_ACTION_SELECT)
        {
            // 普通前进
            current_state++;
            menu_index = current_state;
            
            // 设置默认值
            if (menu_values[menu_index] == 0)
            {
                menu_values[menu_index] = 1;
            }
        }
        
        need_redraw = 1;
        key_clear_state(KEY_4);
    }
    // 上下键调整值（只在ACTION_SELECT之前有效）
    else if (key_get_state(KEY_1) == KEY_SHORT_PRESS && 
             current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] > 1)
            {
                menu_values[menu_index]--;
            }
            else
            {
                menu_values[menu_index] = menu_max_values[menu_index];
            }
            need_redraw = 1;
        }
        key_clear_state(KEY_1);
    }
    else if (key_get_state(KEY_3) == KEY_SHORT_PRESS && 
             current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] < menu_max_values[menu_index])
            {
                menu_values[menu_index]++;
            }
            else
            {
                menu_values[menu_index] = 1;
            }
            need_redraw = 1;
        }
        key_clear_state(KEY_3);
    }
}

// ==================== 初始化 ====================
void Menu_Init(void)
{
    current_state = MENU_STATE_MAIN;
    menu_index = MENU_STATE_SUBJECT;
    //action_step = 0;
    
    for (uint8_t i = 0; i < MENU_STATE_COUNT; i++)
    {
        menu_values[i] = 1;
    }
    
    need_redraw = 1;
}