/*********************************************************************************************************************
* CYT4BB Opensourec Library 即（ CYT4BB 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB 开源库的一部分
*
* CYT4BB 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          main_cm7_0
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"//【提醒！！！】导入了新模块添加到这个文件里
#include "config/config.h"//【提醒】配置请在这里修改
#include "tools/runtime_profiler.h"
#include "vision/vision_ipc_core0.h"
#include "tools/telemetry_ipc_core0.h"
#include "vision/vision_pvc_control.h"
#include "vision/vision_bumpy_control.h"
#include "vision/vision_bridge_control.h"
#include "vision/vision_three_stage_control.h"


// **************************** uart配置区域 **************************** 
#define UART_INDEX              (DEBUG_UART_INDEX    )                           // 默认 UART_0
#define UART_BAUDRATE           (DEBUG_UART_BAUDRATE)                           // 默认 115200
#define UART_TX_PIN             (DEBUG_UART_TX_PIN  )                           // 默认 UART0_TX_P00_1
#define UART_RX_PIN             (DEBUG_UART_RX_PIN  )                           // 默认 UART0_RX_P00_0
uint8 uart_get_data[64];                                                        // 串口接收数据缓冲区
uint8 fifo_get_data[64];                                                        // fifo 输出读出缓冲区
uint8  get_data = 0;                                                            // 接收数据变量
uint32 fifo_data_count = 0;                                                     // fifo 数据个数
fifo_struct uart_data_fifo;
// **************************** 无刷电机配置区域 **************************** 
#define MAX_DUTY            (30 )                                               // 最大 MAX_DUTY% 占空比
int8 duty = 0;
bool dir = true;
// **************************** ips200屏幕配置区域 ****************************                                    
#define IPS200_TYPE     (IPS200_TYPE_SPI)   // 八位并口两寸屏 这里宏定义填写 IPS200_TYPE_PARALLEL8  定义屏幕接口类型    
// SPI 串口两寸屏 这里宏定义填写 IPS200_TYPE_SPI
// **************************** LED配置区域 ****************************
#define LED1                    (P19_0)                                         // SPI 串口 SPI 两寸屏 这里宏定义填写 IPS200_TYPE_SPI

//  **************************** 中断配置区域 ****************************
#define PIT_NUM         (PIT_CH0) // 使用定时器通道0       用于平衡控制，1ms
#define PIT_NUM_1         (PIT_CH1) // 使用定时器通道1     用于遥控器，10ms
#define PIT_NUM_10         (PIT_CH10) // 使用定时器通道10  用于遥控器，10ms
volatile uint8 pit_state = 0;  //通道0中断标志位

uint8 pit_state_1 = 0;//通道1中断标志位
volatile runtime_profiler_t g_ekf_profiler = {0};


// *************************** EKF中断声明 ***************************
extern void IMU_Calibrate_All_Gyro(void); // 校准陀螺仪声明
extern void EKF_Init(void);
extern void EKF_UpData(void);
extern EulerAngles euler_angle; // 引用 ekf.c 中计算出的角度
// =================================================================================
// PID控制中间变量开始
float pid_out_speed = 0.0f; // 速度环输出 (角度调整量)
float pid_out_angle = 0.0f; // 角度环输出 (期望角速度)
float pid_out_pwm   = 0.0f; // 角速度环输出 (电机占空比)
int g_motor_enable = 1; // 电机使能安全开关，1为使能，0为关机
// =================================================================================

// =================================================================================
// 导航记录控制标志位
volatile uint8_t g_nav_recording = 0;       // 1: 正在记录 RAM, 0: 停止记录
volatile uint8_t g_nav_start_recording = 0;  // 1: 请求开始录制，创建内存区
volatile uint8_t g_save_flash_request = 0;  // 1: 请求将 RAM 数据存入 Flash
volatile uint8_t g_load_flash_request = 0;      // 1: 请求从 Flash 加载数据
volatile uint8_t g_replay_start_request = 0;
volatile uint8_t g_replay_stop_request = 0;
volatile uint8_t vision_detected_bumpy_point = 0; // 模拟视觉检测到“颠簸入口”
// =================================================================================
int main(void)
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
    
    // 1. 初始化定时器中断，周期 1ms (必须与ekf.c中的dt=0.005对应)
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
 uint8 display_count = 0; // 用于屏幕刷新分频
 uint8 ekf_print_div = 0; // 50ms*10 = 500ms 

vision_detected_marker = 0;//雷区调用,测试用
vision_detected_bumpy_point = 0;//颠簸路段调用,测试用
    //-------------------------------------------------------------------
    //******************************系统初始化结束************************
    //-------------------------------------------------------------------
    

    while(true)
    {
        // 检查中断标志位 (由 isr.c 中的 pit0_ch0_isr 置位)
        if(pit_state == 1)//10mswifi，100ms屏幕刷新
        {
            // 如果需要 WiFi 发送，建议也放在这里(50ms一次)，或者放在5ms的逻辑里
            pit_state = 0; // 清除标志   

            // ekf_print_div++;
            // if(ekf_print_div >= 10)
            // {
            //     uint32 ekf_cnt = g_ekf_profiler.count;
            //     if(ekf_cnt > 0U)
            //     {
            //         printf("[EKF] cnt=%lu, avg=%lu us, min=%lu us, max=%lu us, last=%lu us\r\n",
            //                (unsigned long)g_ekf_profiler.count,
            //                (unsigned long)g_ekf_profiler.avg_us,
            //                (unsigned long)g_ekf_profiler.min_us,
            //                (unsigned long)g_ekf_profiler.max_us,
            //                (unsigned long)g_ekf_profiler.last_us);
            //     }
            //     ekf_print_div = 0;
            // }
            #if WIFI_CORE0_CUSTOM_PROTOCOL
                wifi_protocol_send_data();//自定义wifi协议（惯导/GNSS/打点状态）
            #endif
                //TelemetryIpc_Core0_PublishPvcDefault();
            #if WIFI_CORE0_ASSISTANT
                //逐飞助手示波器发送代码        
                // //1.【调试直立环，左右轮，俯仰角，角速度环输出，角度环输出，舵机环输出，翻滚角，偏航角】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                // seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;
                // seekfree_assistant_oscilloscope_data.data[3] = (float)gyro_loop_out;
                // seekfree_assistant_oscilloscope_data.data[4] = (float)pid_angle.output;
                // seekfree_assistant_oscilloscope_data.data[5] = (float)pid_servo_speed.error_integral;
                // seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.roll;
                // seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw;


                // 2.【调试转向环，左右轮，偏航角，转向角速度环输出，转向角度环输出，舵机环输出，翻滚角，俯仰角】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                // seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;
                // seekfree_assistant_oscilloscope_data.data[3] = (float)pid_gyro.output;
                // seekfree_assistant_oscilloscope_data.data[4] = (float)pid_turn_angle.output;
                // seekfree_assistant_oscilloscope_data.data[5] = (float)pid_servo_speed.output;
                // seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.roll;
                // seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw;


                // //3.【调试遥控器，前六个通道】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)uart_receiver.channel[0];
                // seekfree_assistant_oscilloscope_data.data[1] =(float)uart_receiver.channel[1];
                // seekfree_assistant_oscilloscope_data.data[2] = (float)uart_receiver.channel[2];
                // seekfree_assistant_oscilloscope_data.data[3] = (float)uart_receiver.channel[3];
                // seekfree_assistant_oscilloscope_data.data[4] =(float)uart_receiver.channel[4];
                // seekfree_assistant_oscilloscope_data.data[5] = (float)uart_receiver.channel[5];
                // seekfree_assistant_oscilloscope_data.data[6] = 0.0f;//(float)uart_receiver.channel[6];
                // seekfree_assistant_oscilloscope_data.data[7] = 0.0f;//(float)uart_receiver.channel[7];

                // 4.【调节颠簸路段状态机】
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
                // seekfree_assistant_oscilloscope_data.data[2] = (float)(gyro_loop_out + turn_gyro_loop_out); 
                // seekfree_assistant_oscilloscope_data.data[3] = (float)(-gyro_loop_out + turn_gyro_loop_out); 
                // seekfree_assistant_oscilloscope_data.data[4] = (float)target_speed_set;
                // seekfree_assistant_oscilloscope_data.data[5] = (float)err_degree;
                // seekfree_assistant_oscilloscope_data.data[6] = (float)euler_angle.pitch;
                // seekfree_assistant_oscilloscope_data.data[7] = (float)euler_angle.yaw; 

                // 4.【调节pvc识别】
                // data[0] 左轮速度
                // seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;

                // // data[1] 右轮速度
                // seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;

                // // data[2] 航向误差 (PVC 第一版暂时为 0)
                // seekfree_assistant_oscilloscope_data.data[2] = (float)g_vision_ipc_latest.pvc_yaw_error_deg_x100 / 100.0f;

                // // data[3] 横向偏差
                // seekfree_assistant_oscilloscope_data.data[3] = (float)g_vision_ipc_latest.pvc_lateral_mm;

                // // data[4] 目标速度
                // seekfree_assistant_oscilloscope_data.data[4] = (float)target_speed_set;

                // // data[5] 置信度 (0~1000)
                // seekfree_assistant_oscilloscope_data.data[5] = (float)g_vision_ipc_latest.pvc_confidence_u16;

                // // data[6] 近端白边行号
                // seekfree_assistant_oscilloscope_data.data[6] = (float)g_vision_ipc_latest.pvc_entry_bottom_y;

                // // data[7] 远端白边行号
                // seekfree_assistant_oscilloscope_data.data[7] = (float)g_vision_ipc_latest.pvc_entry_top_y;

                //     // 4. 设置本次发送的通道数量 (一共8个数据)
                // seekfree_assistant_oscilloscope_data.channel_num = 8;
                    
                //     // 5. 调用发送函数
                // seekfree_assistant_oscilloscope_send(&seekfree_assistant_oscilloscope_data);

                // //用于上位机向小车发送pid信息
                // wifi_update_pid_params(); 
            #endif
            //下面撰写的是100ms执行一次的代码
            // --- 屏幕刷新逻辑 (降频处理) ---
            display_count++;
            if(display_count >= 10) // 10* 10 ms = 100ms 刷新一次屏幕
            {
                display_count = 0;    
                #if DEBUG_DISPLAY
                    Menu_ShowStatic();    // 静态显示
                    Menu_ShowDynamic();   // 动态显示
                #endif    
            }
        }


        // fifo_data_count = fifo_used(&uart_data_fifo);                           // 查看 fifo 是否有数据
        // if(fifo_data_count != 0)                                                // 读取到数据了
        // {
        //     fifo_read_buffer(&uart_data_fifo, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);    // 将 fifo 中数据读出并清空 fifo 挂载的缓冲
        //     uart_write_string(UART_INDEX, "\r\nUART get data:");                // 输出测试信息
        //     uart_write_buffer(UART_INDEX, fifo_get_data, fifo_data_count);      // 将读取到的数据发送出去
        // }

        #if WIFI_CORE0_USE && WIFI_IMAGE_SEND//wifi开关和图像发送开关均开启则发送图像
        // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

            // 发送图像
            seekfree_assistant_camera_send();
            // 如果使用UDP协议传输数据则推荐在数据全部发送到模块之后立即调用wifi_spi_udp_send_now()函数，以告知模块立即将收到的数据发送到网络上
            // 如果没有立即调用则模块会在持续2毫秒未收到数据后，将数据发送到网络上
            // 调用wifi_spi_udp_send_now()前传输给模块的数据数量建议不要超过40960字节
            // wifi_spi_udp_send_now();
        }
        #endif

        if (vision_detected_marker == 1) {
            minefield_flag = 1; // 触发旋转
            vision_detected_marker = 0;
        }//雷区旋转调用，测试用

        //模拟视觉触发跳跃测试
        if (vision_detected_jump_point == 1) 
        {
            jump_trigger(); // <--- 只需要调用这一句
            vision_detected_jump_point = 0; // 清除标志位，防止连续触发
        }

         // 2. 模拟视觉触发【三级跳】测试
        if (vision_detected_three_jump_point == 1) 
        {
            // 判断当前是否处于空闲状态，防止跳跃中途重复触发打断动作
            if (!VisionThreeStageControl_IsActive()) 
            {
                VisionThreeStageControl_Start(); // <--- 启动三级跳状态机
            }
            vision_detected_three_jump_point = 0; // 清除标志位
        }

        // 模拟视觉触发颠簸测试
        if (vision_detected_bumpy_point == 1)
        {
            BumpyRoad_Trigger();             // <--- 只需要调用这一句
            vision_detected_bumpy_point = 0; // 清除标志位，防止连续触发
        }

        // 模拟视觉触发单边桥测试
        if (vision_detected_bridge_point == 1) 
        {
            // 判断当前是否处于空闲状态，防止测试中途重复触发打断动作
            if (!Bridge_Test_Triple_SingleSide_Is_Active()) 
            {
                Bridge_Test_Triple_SingleSide_Start(); // 启动单边桥测试状态机
            }
            vision_detected_bridge_point = 0; // 清除标志位，避免重复触发
        }



        // ---------------------------------------------------------
        // ---------------- 【nav.1】初始化惯性导航模块和打点模块 ----------------
        // ---------------------------------------------------------
        if (g_nav_start_recording)
        {            
            InertialNav_Init();
            NavRam_Init();
            #if DEBUG_LOG_ENABLE
                printf("[NAV] Init OK: x=%.2f y=%.2f yaw=%.2f\r\n",
                       inertial_nav.x,
                       inertial_nav.y,
                       inertial_nav.relative_yaw);
            #endif
            Buzzer_Beep_By_PointType(2);//叫三次
            g_nav_start_recording = 0;//初始化后置0处理
        }


        // ---------------------------------------------------------
        // ---------------- 【nav.2】打点处理 ----------------
        // ---------------------------------------------------------
        // if (robot_ctrl.mark_trigger)
        // {
        //     uint8_t ret;

        //     // 写入 RAM
        //     ret = NavRam_RecordPoint(robot_ctrl.point_type);

        //     if (ret == 0)
        //     {
        //         // 写入成功 → 蜂鸣器反馈
        //         Buzzer_Beep_By_PointType(robot_ctrl.point_type);

        //     #if DEBUG_LOG_ENABLE
        //         printf("[NAV] Record OK: idx=%d type=%d x=%.2f y=%.2f\r\n",
        //                NavRam_GetPointCount() - 1,
        //                robot_ctrl.point_type,
        //                inertial_nav.x,
        //                inertial_nav.y);
        //     #endif
        //     }
        //     else
        //     {
        //         // RAM 满
        //         #if DEBUG_LOG_ENABLE
        //             printf("[NAV] Record FAILED: RAM FULL\r\n");
        //         #endif
        //     }

        //     // ★ 必须清零，否则会重复写入 ★
        //     robot_ctrl.mark_trigger = 0;
        // }

    // ---------------------------------------------------------
    //  【nav.3】静态点表模式：忽略 Flash 保存请求
    // ---------------------------------------------------------
    if(g_save_flash_request == 1)
    {
        #if DEBUG_LOG_ENABLE
        printf("Main: Flash save disabled (static route mode).\r\n");
        #endif
        g_save_flash_request = 0;
    }

    // ---------------------------------------------------------
    //  【nav.4】静态点表模式：加载 C 点表并开始复现
    // ---------------------------------------------------------
    if (g_motor_enable == 1 && g_load_flash_request == 1)
    {
        g_load_flash_request = 0;

        InertialNav_Init();

        NavReplay_Start();//start replay
        #if DEBUG_LOG_ENABLE
        printf("Main: Static route loaded.\r\n");
        printf("Main: Starting Inertial Navigation...\r\n");
        #endif

        Buzzer_Beep_By_PointType(2);//beep x3
    }

    #if GNSS_NAV == 1
    // ---------------------------------------------------------
    //  【gps-nav】静态点表模式：开始纯GPS复刻
    // ---------------------------------------------------------
    if (g_motor_enable == 1 && g_replay_start_request == 1)
    {
        g_replay_start_request = 0;
        Gnss_Transform_Reset_Origin();
        GpsNavReplay_Start();
        #if DEBUG_LOG_ENABLE
        printf("Main: Starting pure GPS replay...\r\n");
        #endif
        Buzzer_Beep_By_PointType(2);
    }

    // ---------------------------------------------------------
    //  【gps-nav】停止纯GPS复刻
    // ---------------------------------------------------------
    if (g_replay_stop_request == 1)
    {
        g_replay_stop_request = 0;
        GpsNavReplay_Stop();
        #if DEBUG_LOG_ENABLE
        printf("Main: Pure GPS replay stopped.\r\n");
        #endif
    }
    #endif


        // 此处编写需要循环执行的代码
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       UART_INDEX 的接收中断处理函数 这个函数将在 UART_INDEX 对应的中断调用
// 参数说明       void
// 返回参数       void
// 使用示例       uart_rx_interrupt_handler();
//-------------------------------------------------------------------------------------------------------------------
void uart_rx_interrupt_handler (void)
{
//    get_data = uart_read_byte(UART_INDEX);                                      // 接收数据 while 等待式 不建议在中断使用
    if(uart_query_byte(UART_INDEX, &get_data))                                  // 接收数据 查询式 有数据会返回 TRUE 没有数据会返回 FALSE
    {
        fifo_write_buffer(&uart_data_fifo, &get_data, 1);                       // 将数据写入 fifo 中
    }
}


