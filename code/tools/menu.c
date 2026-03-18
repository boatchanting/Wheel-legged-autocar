#include "menu.h"


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
    ips200_show_string(0,15*4, "Pitch:");
    ips200_show_string(0,15*5, "Roll :");
    ips200_show_string(0,15*6, "Yaw  :");
    // ips200宽240，高320，一个字符宽10，高14
    //字符部分
    // ips200_show_string(0,15*0, "Time:");
    // ips200_show_string(0,15*1, "HMS:");
   //ips200_show_string(90,15*2, "Lat:");

    // ips200_show_string(0,15*3, "Lon:"); 
    // ips200_show_string(0,15*4, "Spd:");
    // ips200_show_string(0,15*5, "Dir:"); 
    // ips200_show_string(0,15*6, "Sat:");
    // ips200_show_string(0,15*7, "Height:");
    //ips200_show_string(0, 100,"Freq : 20Hz");
    ips200_show_string(0,15*7, "State:"); 
    ips200_show_string(0, 120, "Servo Angles:");
    ips200_show_string(0, 135, "RF:");
    ips200_show_string(0, 150, "RR:");
    ips200_show_string(0, 165, "LF:");
    ips200_show_string(0, 180, "LR:");
    ips200_show_string(0, 200, "Motor Speed:");
    ips200_show_string(0, 215, "L:");
    ips200_show_string(100, 215, "R:");
    ips200_show_string(0, 260, "g_motor_enable");
    // 提示信息
    ips200_show_string(0, 280, "Press any key to menu");
    }
    else
    {
    ips200_show_string(0,15*4, "Pitch:");
    ips200_show_string(0,15*5, "Roll :");
    ips200_show_string(0,15*6, "Yaw  :");
    ips200_show_string(0,15*7, "State:"); 
    ips200_show_string(0, 120, "Servo Angles:");
    ips200_show_string(0, 135, "RF:");
    ips200_show_string(0, 150, "RR:");
    ips200_show_string(0, 165, "LF:");
    ips200_show_string(0, 180, "LR:");
    ips200_show_string(0, 200, "Motor Speed:");
    ips200_show_string(0, 215, "L:");
    ips200_show_string(100, 215, "R:");
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
        ips200_show_string(0, 280, "Pushing Phase Active"); // 显示为推车阶段
    } else {
        ips200_show_string(0, 280, "Not Started          ");
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
    
    ips200_show_float(60,15*4, euler_angle.pitch, 3, 2);
    ips200_show_float(60,15*5, euler_angle.roll, 3, 2);
    ips200_show_float(60,15*6, euler_angle.yaw, 3, 2);
    
    ips200_show_float(25, 135, current_angles[0], 3, 1);
    ips200_show_float(25, 150, current_angles[1], 3, 1);
    ips200_show_float(25, 165, current_angles[2], 3, 1);
    ips200_show_float(25, 180, current_angles[3], 3, 1);
    
    ips200_show_float(25, 215, motor_speeds[0], 5, 1);
    ips200_show_float(125, 215, motor_speeds[1], 5, 1);
    ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    
    // 下面打印"发车成功"
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    if (subject == 4) {
        ips200_show_string(0, 280, "Pushing Phase Active"); // 显示为推车阶段
    } else {
        ips200_show_string(0, 280, "Started Successfully");
    }
}

static void Menu_ShowActionCompleteScreen(void)
{
    Menu_ClearScreen();
    
    uint8_t action = menu_values[MENU_STATE_ACTION_SELECT];
    uint8_t subject = menu_values[MENU_STATE_SUBJECT]; // 获取当前科目
    
    if (subject == 4)
    {
        ips200_show_string(0, 10, "Push Mode Ended");
        ips200_show_string(0,15*2, "System returned");
    }
    else if (action == 1)  // Record
    {
        ips200_show_string(0, 10, "Record Complete");
        ips200_show_string(0,15*2, "Data saved successfully");
    }
    else if (action == 2)  // Start
    {
        ips200_show_string(0, 10, "Start Complete");
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
                gpio_toggle_level(P19_0);       // 指示灯切换
                if (g_motor_enable && g_yaw_initialized)
                {      
                g_nav_start_recording = 1;//开始录制，会在main中调用惯导系统初始化和点的记录的初始化,然后再变回1
                g_nav_recording = 1;
                }
            }
            else if(action==2)
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
        ips200_show_float(60,15*0, euler_angle.pitch, 3, 2);
        ips200_show_float(60,15*1, euler_angle.roll, 3, 2);
        ips200_show_float(60,15*2, euler_angle.yaw, 3, 2);
        //ips200_show_uint(50,15*2, gnss.state,5); 
        //ips200_show_uint(40,15*6,gnss.satellite_used,5);


        ips200_show_float(25, 135, current_angles[0], 3, 1);
        ips200_show_float(25, 150, current_angles[1], 3, 1);
        ips200_show_float(25, 165, current_angles[2], 3, 1);
        ips200_show_float(25, 180, current_angles[3], 3, 1);
        
        ips200_show_float(25, 215, motor_speeds[0], 5, 1);
        ips200_show_float(125, 215, motor_speeds[1], 5, 1);
        ips200_show_string(155, 260, g_motor_enable ? "Yes" : "No");
    }
}

// ==================== 按键处理（完全按照您的设计） ====================
// ==================== 按键处理（状态机重写版） ====================
void Menu_HandleKey(void)
{
    // 1. 在主界面：任意键进入菜单
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
            need_redraw = 1;
            key_clear_state(KEY_1); key_clear_state(KEY_2); key_clear_state(KEY_3); key_clear_state(KEY_4);
        }
        return;
    }
    
    // 2. 在完成界面：任意键返回主界面
    if (current_state == MENU_STATE_ACTION_COMPLETE)
    {
        if (key_get_state(KEY_1) == KEY_SHORT_PRESS ||
            key_get_state(KEY_2) == KEY_SHORT_PRESS ||
            key_get_state(KEY_3) == KEY_SHORT_PRESS ||
            key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            current_state = MENU_STATE_MAIN;
            menu_index = MENU_STATE_SUBJECT;
            need_redraw = 1;
            key_clear_state(KEY_1); key_clear_state(KEY_2); key_clear_state(KEY_3); key_clear_state(KEY_4);
        }
        return;
    }
    
    // ==========================================================
    // 3. 左键 (KEY_2)：返回 / 锁住
    // ==========================================================
    if (key_get_state(KEY_2) == KEY_SHORT_PRESS)
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
        key_clear_state(KEY_2);
    }
    
    // ==========================================================
    // 4. 右键 (KEY_4)：前进 / 开启推车模式
    // ==========================================================
    else if (key_get_state(KEY_4) == KEY_SHORT_PRESS)
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
        key_clear_state(KEY_4);
    }
    
    // ==========================================================
    // 5. 上下键 (KEY_1 / KEY_3)：调整数值
    // ==========================================================
    else if (key_get_state(KEY_1) == KEY_SHORT_PRESS && current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] > 1) { menu_values[menu_index]--; }
            else { menu_values[menu_index] = menu_max_values[menu_index]; }
            need_redraw = 1;
        }
        key_clear_state(KEY_1);
    }
    else if (key_get_state(KEY_3) == KEY_SHORT_PRESS && current_state <= MENU_STATE_ACTION_SELECT)
    {
        if (current_state != MENU_STATE_MAIN)
        {
            if (menu_values[menu_index] < menu_max_values[menu_index]) { menu_values[menu_index]++; }
            else { menu_values[menu_index] = 1; }
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