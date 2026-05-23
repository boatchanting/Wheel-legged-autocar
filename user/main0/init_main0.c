#include "main0/init_main0.h"

#pragma location = 0x28026024
__root __no_init uint8 mt9v03x_dma_reserved_for_core1[0x5820];

void Main0_Init(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等

    
    target_speed_set = 0.0f;//目标速度，负数代表向前，和rpm数量级相当，参数为-60时小车大概以20m/s向前行驶

    // *************************** 屏幕初始化开始 ***************************
    // 定义一个变量用于记录屏幕打印的Y坐标（行号）
    uint16 disp_y = 0; 
    gpio_init(LED1, GPO, GPIO_HIGH, GPO_PUSH_PULL);                             // 初始化 LED1 输出 默认高电平 推挽输出模式（用于检测imu660ra是否初始化成功）
#if DEBUG_DISPLAY
    // 1. 设置屏幕方向（竖屏）
    ips200_set_dir(IPS200_PORTAIT);
    // 2. 设置颜色：绿色文字，黑色背景 (像黑客终端一样)
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    // 3. 初始化屏幕硬件
    ips200_init(IPS200_TYPE);
    // 4. 清屏
    ips200_clear();
    
    // 打印系统启动信息
    ips200_show_string(0, disp_y, "System Booting...");
    disp_y += 16; // 换行（假设字体高度16）
#endif
// *************************** 屏幕初始化结束 ***************************

    fifo_init(&uart_data_fifo, FIFO_DATA_8BIT, uart_get_data, 64);              // 初始化 fifo 挂载缓冲区

    uart_init(UART_INDEX, UART_BAUDRATE, UART_TX_PIN, UART_RX_PIN);             // 初始化串口
    uart_rx_interrupt(UART_INDEX, 1);                                           // 开启 UART_INDEX 的接收中断

    uart_write_string(UART_INDEX, "UART Text.");                                // 输出测试信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');                                          // 输出换行

// --- 屏幕打印 UART 初始化完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "UART Init OK");
    disp_y += 16;
#endif

    // 初始化无刷电机
    small_driver_uart_init();		// 初始化驱动通讯功能
    uart_write_string(UART_INDEX, "Brushless Motor Initialized.");              // 输出无刷电机初始化完成信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');

    // --- 屏幕打印无刷电机初始化完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Brushless Motor Init OK");
    disp_y += 16;
#endif

servo_executor_init();
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Servo Init OK");
    disp_y += 16;
#endif

#if WIFI_CORE0_USE
    // 初始化 WiFi 模块
    wifi_init();                                                                // 初始化WIFI模块
    uart_write_string(UART_INDEX, "WiFi Module Initialized.");                  // 输出WIFI初始化完成信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');                                              // 输出换行
    
    // 【调试用】查看WiFi模块的连接状态
    uart_write_string(UART_INDEX, "WiFi SSID: ");
    uart_write_string(UART_INDEX, WIFI_SSID_TEST);
    uart_write_byte(UART_INDEX, '\r');
    uart_write_byte(UART_INDEX, '\n');
    uart_write_string(UART_INDEX, "WiFi IP: ");
    uart_write_string(UART_INDEX, wifi_spi_ip_addr_port);
    uart_write_byte(UART_INDEX, '\r');
    uart_write_byte(UART_INDEX, '\n');

    // 连接TCP服务器
    wifi_connect_tcp_server();                                                  // 连接TCP服务器
    uart_write_string(UART_INDEX, "TCP Server Connected.");                     // 输出TCP连接成功信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');                                          // 输出换行

    #if WIFI_CORE0_ASSISTANT
    // 初始化逐飞助手数据接口
    wifi_assistant_init();                                                         // 初始化逐飞助手数据接口
    uart_write_string(UART_INDEX, "Assistant Initialized.");                       // 输出逐飞助手初始化完成信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');
    //初始化逐飞助手通信结束
    #endif
#endif
 gpio_init(BUZZER_PIN, GPO, GPIO_LOW, GPO_PUSH_PULL);                             // 初始化 蜂鸣器 引脚 低电平 默认 推挽输出模式
// --- 屏幕打印 WiFi 初始化完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "WiFi Init OK");
    disp_y += 16;
#endif
system_delay_ms(1000);
while(1)//检测imu660ra是否初始化成功
{
    #if IMU_CATEGORY == 1 //如果小车不同再对小车加&&加以区分
    if(imu660ra_init())
    {
        printf("\r\n imu660ra init error.");                                 // imu660ra 初始化失败
    }
    else{
        break;
    }   
    #endif
    #if IMU_CATEGORY == 3
    if(imu963ra_init())
    {
        printf("\r\n imu963ra init error.");                                 // imu963ra 初始化失败
    }
    else{
        break;
    }
    #endif
    gpio_toggle_level(LED1);                                                // 翻转 LED 引脚输出电平 控制 LED 亮灭 初始化出错这个灯会闪的很慢
}
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "IMU Init OK");
    disp_y += 16;
#endif

//★★★ 执行Z轴校准 ★★★
// 此时车模/设备必须保持静止！
IMU_Calibrate_All_Gyro();
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "IMU All Gyro Calibrated");
    disp_y += 16;
#endif

EKF_Init(); // 初始化扩展卡尔曼滤波
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "EKF Init OK");
    disp_y += 16;
#endif

#if REMOTE_CONTROL
    uart_receiver_init();//sbus接收机初始化
    Remote_Control_Init(); // 遥控器初始化函数声明
    #if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Remote Control Init OK");
    disp_y += 16;
    #endif
#endif

// 初始化 PID 参数 (必须最先调用)
flash_init();   // 使用flash前先调用flash初始化 ，包含pid初始化
PID_Param_Init();//pid其余参数初始化
Momentum_Wheel_Control_Init();//pid跳跃控制，动量轮控制参数初始化
//param_read_from_flash(); // 从 Flash 读取参数
// param_save_to_flash()   ;     // 将当前参数保存到 Flash 
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Flash Init OK");
    disp_y += 16;
#endif

InertialNav_Init();//惯性导航初始化
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "InertialNav Init OK");
    disp_y += 16;
#endif

gnss_init(TAU1201);//gnss导航初始化
Gnss_Transform_Init();//GNSS经纬度投影为相对平面坐标，供纯GPS打点/复刻使用
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "GNSS Init OK");
    disp_y += 16;
#endif

Bridge_Init();//【优化点】单边桥控制初始化，可以集成
BumpyRoad_Init();//颠簸路段状态机初始化
VisionIpc_Core0_Init();
TelemetryIpc_Core0_Init();
VisionPvcControl_Init(); // Bring-up: 0核通过2ms中断调度1核开启PVC入口检测，并用回传数据做入口引导
VisionBumpyControl_Init(); // 颠簸路段：0核读取1核视觉并生成方向控制量
VisionBridgeTask_Init();
VisionThreeStageControl_Init(); // three-stage vision jump state machine
//===============惯性导航初始化结束==================
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Button Init OK");
    disp_y += 16;
#endif


    uart_rx_interrupt(UART_INDEX, 1);                                           // 开启 UART_INDEX 的接收中断
    // --- 屏幕打印uart中断完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "UART INTERRUPT Init OK");
    disp_y += 16;
#endif

// *****************中断在这后面开*****************
    
    // 1. 初始化定时器中断，周期 1ms (必须与ekf.c中的dt=0.001对应)
    // EKF 运行时间测试
    timer_init(TC_TIME2_CH0, TIMER_US);
    timer_start(TC_TIME2_CH0);
    RUNTIME_PROFILE_RESET(&g_ekf_profiler);
    
    pit_ms_init(PIT_NUM, 1);
    #if REMOTE_CONTROL
    pit_ms_init(PIT_NUM_1, 10);                                                // 定时器通道1 初始化为 10ms 中断 用于 sbus 遥控器数据处理
    #endif
    pit_ms_init(PIT_NUM_10, 10);                                                // 定时器通道10 初始化为 10ms 中断 用于按键扫描
    key_init(10);  // 每10ms扫描一次
    // 2. 开启全局中断 (没有这一步，中断函数永远不会执行)
    interrupt_global_enable(0); 

#if DEBUG_DISPLAY    
    // 延时一会儿让人看清启动信息，然后清屏准备显示数据
    system_delay_ms(1000); 
    ips200_clear();
#endif

#if IMU_REFRESH_TEST_ENABLE
uint8 imu_refresh_test_printed = 0; // IMU刷新率测试结果只打印一次
#endif
    vision_detected_marker = 0;//雷区调用,测试用
    vision_detected_bumpy_point = 0;//颠簸路段调用,测试用
    //-------------------------------------------------------------------
    //******************************系统初始化结束************************
    //-------------------------------------------------------------------
}
