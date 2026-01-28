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
#define WIFI_USE 1 // 【全局开关】选择是否使用WIFI模块，0表示不使用，1表示使用
#define WIFI_IMAGE_SEND 0 // 【全局开关】选择是否使用WIFI回传摄像机图像，0表示不使用，1表示使用。只有当WIFI_USE和它均为1时有效
#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭



// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完


// *************************** UART例程硬件连接说明 ***************************
// 使用逐飞科技 CMSIS-DAP 调试下载器连接
//      直接将下载器正确连接在核心板的调试下载接口即可
// 使用 USB-TTL 模块连接
//      模块管脚            单片机管脚
//      USB-TTL-RX          查看 zf_common_debug.h 文件中 DEBUG_UART_TX_PIN 宏定义的引脚 默认 P14_0
//      USB-TTL-TX          查看 zf_common_debug.h 文件中 DEBUG_UART_RX_PIN 宏定义的引脚 默认 P14_1
//      USB-TTL-GND         核心板电源地 GND
//      USB-TTL-3V3         核心板 3V3 电源

//================================特别注意================================
// 串口接线时一定要接GND 否则无法正常通讯
//================================特别注意================================
//================================特别注意================================
// 串口接线时一定要接GND 否则无法正常通讯
//================================特别注意================================
//================================特别注意================================
// 串口接线时一定要接GND 否则无法正常通讯
//================================特别注意================================

// ***************************** 例程测试说明 *****************************
// 1.核心板烧录完成本例程，单独使用核心板与调试下载器或者 USB-TTL 模块，在断电情况下完成连接
// 2.将调试下载器或者 USB-TTL 模块连接电脑，完成上电
// 3.电脑上使用串口助手打开对应的串口，串口波特率为 DEBUG_UART_BAUDRATE 宏定义 默认 115200，核心板按下复位按键
// 4.可以在串口助手上看到如下串口信息：
//      UART Text.
// 5.通过串口助手发送数据，会收到相同的反馈数据
//      UART get data:.......
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// ******************************* 代码区域 *******************************
// uart配置
#define UART_INDEX              (DEBUG_UART_INDEX    )                           // 默认 UART_0
#define UART_BAUDRATE           (DEBUG_UART_BAUDRATE)                           // 默认 115200
#define UART_TX_PIN             (DEBUG_UART_TX_PIN  )                           // 默认 UART0_TX_P00_1
#define UART_RX_PIN             (DEBUG_UART_RX_PIN  )                           // 默认 UART0_RX_P00_0
uint8 uart_get_data[64];                                                        // 串口接收数据缓冲区
uint8 fifo_get_data[64];                                                        // fifo 输出读出缓冲区
uint8  get_data = 0;                                                            // 接收数据变量
uint32 fifo_data_count = 0;                                                     // fifo 数据个数
fifo_struct uart_data_fifo;
// uart配置结束

// *************************** 4bb7无刷电机例程硬件连接说明 ***************************
// 使用逐飞科技 tc264 V2.6主板 按照下述方式进行接线
//      模块引脚    单片机引脚
//      RX          查看 small_driver_uart_control.h 中 SMALL_DRIVER_TX  宏定义 默认 P10_1
//      TX          查看 small_driver_uart_control.h 中 SMALL_DRIVER_RX  宏定义 默认 P10_0
//      GND         GND

// *************************** 例程测试说明 ***************************
// 1.核心板烧录完成本例程 主板电池供电 连接 CYT2BL3 FOC 双驱
// 2.如果初次使用 请先点击双驱上的MODE按键 以矫正零点位置 矫正时 电机会发出音乐
// 3.可以在逐飞助手上位机上看到如下串口信息：
//      left speed:xxxx, right speed:xxxx
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************
// 无刷电机配置
#define MAX_DUTY            (30 )                                               // 最大 MAX_DUTY% 占空比
int8 duty = 0;
bool dir = true;
//无刷电机配置结束

// *************************** ips200屏幕例程硬件连接说明 ***************************
//      模块管脚            单片机管脚
//      BL                  查看 zf_device_ips200_parallel8.h 中 IPS200_BL_PIN 宏定义 
//      CS                  查看 zf_device_ips200_parallel8.h 中 IPS200_CS_PIN 宏定义 
//      RST                 查看 zf_device_ips200_parallel8.h 中 IPS200_RST_PIN 宏定义
//      RS                  查看 zf_device_ips200_parallel8.h 中 IPS200_RS_PIN 宏定义 
//      WR                  查看 zf_device_ips200_parallel8.h 中 IPS200_WR_PIN 宏定义 
//      RD                  查看 zf_device_ips200_parallel8.h 中 IPS200_RD_PIN 宏定义 
//      D0-D7               查看 zf_device_ips200_parallel8.h 中 IPS200_Dx_PIN 宏定义 
//      GND                 核心板电源地 GND
//      3V3                 核心板 3V3 电源



// *************************** 例程测试说明 ***************************
// 1.核心板烧录本例程 插在主板上 2寸IPS 显示模块插在主板的屏幕接口排座上 请注意引脚对应 不要插错
// 2.电池供电 上电后 2寸IPS 屏幕亮起 显示字符数字浮点数和波形图
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** ips200屏幕配置区域 ****************************                                    
#define IPS200_TYPE     (IPS200_TYPE_SPI)   // 八位并口两寸屏 这里宏定义填写 IPS200_TYPE_PARALLEL8  定义屏幕接口类型
//#define DEBUG_DISPLAY 1                  // 【全局开关】1:开启屏幕调试显示  0:关闭        放在前面了，它属于这里                                                                        // SPI 串口两寸屏 这里宏定义填写 IPS200_TYPE_SPI
// *************************** imu660ra例程测试说明 ***************************
// 1.核心板烧录完成本例程，单独使用核心板与调试下载器或者 USB-TTL 模块，并连接好编码器，在断电情况下完成连接
// 2.将调试下载器或者 USB-TTL 模块连接电脑 完成上电 正常 H2 LED 会闪烁
// 3.电脑上使用 逐飞助手 打开对应的串口，串口波特率为 zf_common_debug.h 文件中 DEBUG_UART_BAUDRATE 宏定义 默认 115200，核心板按下复位按键
// 4.可以在 逐飞助手 上看到如下串口信息：
//      imu660ra acc data: x-..., y-..., z-...
//      imu660ra gyro data: x-..., y-..., z-...
// 5.移动旋转 imu660ra 就会看到数值变化
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************
#define LED1                    (P19_0)                                         // SPI 串口 SPI 两寸屏 这里宏定义填写 IPS200_TYPE_SPI

// *************************** EKF中断声明 ***************************
extern void IMU_Calibrate_All_Gyro(void); // 校准陀螺仪声明
extern void EKF_Init(void);
extern void EKF_UpData(void);
extern EulerAngles euler_angle; // 引用 ekf.c 中计算出的角度
volatile uint8 pit_state = 0;
// --- EKF宏定义 ---
#define PIT_NUM         (PIT_CH0) // 使用定时器通道0
// =================================================================================
// PID控制中间变量开始
// =================================================================================
float pid_out_speed = 0.0f; // 速度环输出 (角度调整量)
float pid_out_angle = 0.0f; // 角度环输出 (期望角速度)
float pid_out_pwm   = 0.0f; // 角速度环输出 (电机占空比)
int g_motor_enable = 0; // 电机使能安全开关
// =============================================
// PID控制中间变量结束
// ===============================================


int main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等

    // 初始化 PID 参数 (必须最先调用)
    // -------------------------------------------------------------------------
    target_speed_set = 0.0f;//目标速度，暂时未调用

    


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

#if WIFI_USE    
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
    
    // 初始化摄像头和逐飞助手
    wifi_camera_init();                                                         // 初始化摄像头和逐飞助手
    uart_write_string(UART_INDEX, "Camera Initialized.");                       // 输出摄像头初始化完成信息
    uart_write_byte(UART_INDEX, '\r');                                          // 输出回车
    uart_write_byte(UART_INDEX, '\n');
    //初始化摄像头和通信模块结束
#endif

// --- 屏幕打印 WiFi 初始化完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "WiFi Init OK");
    disp_y += 16;
#endif



// 此处编写用户代码 例如外设初始化代码等
while(1)//检测imu660ra是否初始化成功
{
    if(imu660ra_init())
    {
        printf("\r\n imu660ra init error.");                                 // imu660ra 初始化失败
    }
    else
    {
        break;
    }
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

flash_init();   // 使用flash前先调用flash初始化 ，包含pid初始化
PID_Param_Init();//pid其余参数初始化
param_read_from_flash(); // 从 Flash 读取参数
// param_save_to_flash()   ;     // 将当前参数保存到 Flash 
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "Flash Init OK");
    disp_y += 16;
#endif

    uart_rx_interrupt(UART_INDEX, 1);                                           // 开启 UART_INDEX 的接收中断
    // --- 屏幕打印uart中断完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "UART INTERRUPT Init OK");
    disp_y += 16;
#endif

// *****************关键新增步骤*****************
    
    // 1. 初始化定时器中断，周期 1ms (必须与ekf.c中的dt=0.005对应)
    pit_ms_init(PIT_NUM, 1);
    
    // 2. 开启全局中断 (没有这一步，中断函数永远不会执行)
    interrupt_global_enable(0); 

#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "PIT & INT OK");
    disp_y += 16;
    
    // 延时一会儿让人看清启动信息，然后清屏准备显示数据
    system_delay_ms(1000); 
    ips200_clear();
    
    // 绘制静态UI标签 (避免循环里重复绘制浪费时间)
    ips200_show_string(0, 0,  "EKF Monitor");
    ips200_show_string(0, 30, "Pitch:");
    ips200_show_string(0, 50, "Roll :");
    ips200_show_string(0, 70, "Yaw  :");
    ips200_show_string(0, 100,"Freq : 20Hz");
    // 添加舵机角度显示标签
    ips200_show_string(0, 120, "Servo Angles:");
    ips200_show_string(0, 135, "RF:");  // 右前
    ips200_show_string(0, 150, "RR:");  // 右后
    ips200_show_string(0, 165, "LF:");  // 左前
    ips200_show_string(0, 180, "LR:");  // 左后
    // 添加电机转速显示标签
    ips200_show_string(0, 200, "Motor Speed:");
    ips200_show_string(0, 215, "L:");  // 左电机
    ips200_show_string(80, 215, "R:");  // 右电机
    ips200_show_string(0, 230, "gyro.kp");  // 右电机
    ips200_show_string(0, 245, "gyro.kd");  // 右电机
#endif
 uint8 display_count = 0; // 用于屏幕刷新分频

    //-------------------------------------------------------------------
    //******************************系统初始化结束************************
    //-------------------------------------------------------------------
    

    while(true)
    {

        // 此处编写需要循环执行的代码
        // 检查中断标志位 (由 isr.c 中的 pit0_ch0_isr 置位)
        if(pit_state == 1)
        {
            pit_state = 0; // 清除标志   
            
            //下面撰写的是50ms执行一次的代码

            // --- 屏幕刷新逻辑 (降频处理) ---
            display_count++;
            if(display_count >= 2) // 2* 50 ms = 100ms 刷新一次屏幕
            {
                display_count = 0;
                // 在这里获取舵机角度，而不是在显示时获取
                float current_angles[4];
                servo_get_current_angles(current_angles);
                // 获取电机速度数据
                float motor_speeds[2];
                //small_driver_get_speed();
                motor_speeds[0] = motor_value.receive_left_speed_data;
                motor_speeds[1] = motor_value.receive_right_speed_data;

                // gpio_toggle_level(LED1); // LED闪烁指示系统正在运行
                
                #if DEBUG_DISPLAY
                    // 显示 Pitch (俯仰角)
                    // 参数：X坐标, Y坐标, 浮点数值, 整数位宽, 小数位数
                    ips200_show_float(60, 30, euler_angle.pitch, 3, 2);
                    
                    // 显示 Roll (横滚角)
                    ips200_show_float(60, 50, euler_angle.roll, 3, 2);
                    
                    // 显示 Yaw (偏航角)
                    ips200_show_float(60, 70, euler_angle.yaw, 3, 2);

                    // 显示已获取的舵机角度
                    ips200_show_float(25, 135, current_angles[0], 3, 1);
                    ips200_show_float(25, 150, current_angles[1], 3, 1);
                    ips200_show_float(25, 165, current_angles[2], 3, 1);
                    ips200_show_float(25, 180, current_angles[3], 3, 1);

                    // 显示电机速度
                    ips200_show_float(25, 215, motor_speeds[0], 5, 1); 
                    ips200_show_float(105, 215, motor_speeds[1], 5, 1);  
                    //显示角速度环pid输出
                     ips200_show_float(25, 230, pid_gyro.kp, 4, 2);  
                     ips200_show_float(25, 245, pid_gyro.kd, 4, 2); 

                #endif
                
                // 如果需要 WiFi 发送，建议也放在这里(50ms一次)，或者放在5ms的逻辑里
                #if WIFI_USE
                // 逐飞助手示波器发送代码        
                // 1. 填充速度数据 (通道 0-1)
                seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
                seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;

                
                // 通道 2: Pitch (俯仰角)
                seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;

                // 通道 3: imu660ra_gyro_z
                // seekfree_assistant_oscilloscope_data.data[3] = (float)imu660ra_gyro_z;
                // // 通道 4: imu660ra_gyro_y
                // seekfree_assistant_oscilloscope_data.data[4] = (float)imu660ra_gyro_y;
                // // 通道 3: 角速度环输出
                // seekfree_assistant_oscilloscope_data.data[3] = (float)pid_gyro.output;
                // // 通道 4: 角度环输出
                // seekfree_assistant_oscilloscope_data.data[4] = (float)pid_angle.output;
                // 通道 3: 转向角速度环输出
                seekfree_assistant_oscilloscope_data.data[3] = (float)pid_turn_gyro.output;
                // 通道 4: 转向角度环输出
                seekfree_assistant_oscilloscope_data.data[4] = (float)pid_turn_angle.output;
                //通道 5：舵机速度环输出
                seekfree_assistant_oscilloscope_data.data[5] = (float)pid_servo_speed.output;
                // // 通道 3: Roll (横滚角)
                // seekfree_assistant_oscilloscope_data.data[3] = (float)euler_angle.roll;
                // // 通道 4: Yaw (偏航角)
                // seekfree_assistant_oscilloscope_data.data[4] = (float)euler_angle.yaw;
                
                //3. 填充陀螺仪数据 (通道 5-7)
                
                seekfree_assistant_oscilloscope_data.data[6] = (float)pid_servo_speed.error_integral;

                seekfree_assistant_oscilloscope_data.data[7] = (float)gyro_loop_out;
                
                // 4. 设置本次发送的通道数量 (一共8个数据)
                seekfree_assistant_oscilloscope_data.channel_num = 8;
                
                // 5. 调用发送函数
                seekfree_assistant_oscilloscope_send(&seekfree_assistant_oscilloscope_data);

                // 用于上位机向小车发送pid信息
                wifi_update_pid_params(); 
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

        // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
        #if WIFI_USE && WIFI_IMAGE_SEND//wifi开关和图像发送开关均开启则发送图像
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

            // 发送图像
            seekfree_assistant_camera_send();
            // 如果使用UDP协议传输数据则推荐在数据全部发送到模块之后立即调用wifi_spi_udp_send_now()函数，以告知模块立即将收到的数据发送到网络上
            // 如果没有立即调用则模块会在持续2毫秒未收到数据后，将数据发送到网络上
            // 调用wifi_spi_udp_send_now()前传输给模块的数据数量建议不要超过40960字节
            // wifi_spi_udp_send_now();
        #endif
        }


        
        // ips200_show_int(0, 0, duty,2);

        // small_driver_set_duty(duty * (PWM_DUTY_MAX / 100), -duty * (PWM_DUTY_MAX / 100));   // 计算占空比输出

        // if(dir)                                                                 // 根据方向判断计数方向 本例程仅作参考
        // {
        //     duty ++;                                                            // 正向计数
        //     if(duty >= MAX_DUTY)                                                // 达到最大值
        //     {
        //         dir = false;                                                    // 变更计数方向
        //     }
        // }
        // else
        // {
        //     duty --;                                                            // 反向计数
        //     if(duty <= -MAX_DUTY)                                               // 达到最小值
        //     {
        //         dir = true;                                                     // 变更计数方向
        //     }
        // }
        // printf("motor\r\n");
        // printf("left speed:%d, right speed:%d\r\n", motor_value.receive_left_speed_data, motor_value.receive_right_speed_data);
        // imu660ra_get_acc();                                                     // 获取 imu660ra 的加速度测量数值，已经集成到EKF_UpData();
        // imu660ra_get_gyro();                                                    // 获取 imu660ra 的角速度测量数值，已经集成到EKF_UpData();
        
        // printf("\r\nimu660ra acc data:  x=%5d, y=%5d, z=%5d\r\n", imu660ra_acc_x,  imu660ra_acc_y,  imu660ra_acc_z);
        // printf("\r\nimu660ra gyro data: x=%5d, y=%5d, z=%5d\r\n", imu660ra_gyro_x, imu660ra_gyro_y, imu660ra_gyro_z);
        //gpio_toggle_level(LED1);                                                // 翻转 LED 引脚输出电平 控制 LED 亮灭
        // #if WIFI_USE
        //     // 逐飞助手示波器发送代码        
        //     // 1. 填充速度数据 (通道 0-1)
        //     // 建议强制转换为 float 或 int (取决于库定义，通常 float 通用性更好)
        //     seekfree_assistant_oscilloscope_data.data[0] = (float)motor_value.receive_left_speed_data;
        //     seekfree_assistant_oscilloscope_data.data[1] = (float)motor_value.receive_right_speed_data;
        //      // 通道 2: Pitch (俯仰角)
        //     seekfree_assistant_oscilloscope_data.data[2] = (float)euler_angle.pitch;
        //     // 通道 3: Roll (横滚角)
        //     seekfree_assistant_oscilloscope_data.data[3] = (float)euler_angle.roll;
        //     // 通道 4: Yaw (偏航角)
        //     seekfree_assistant_oscilloscope_data.data[4] = (float)euler_angle.yaw;
        //     // 2. 填充加速度计数据 (通道 2-4)
        //     // seekfree_assistant_oscilloscope_data.data[2] = (float)imu660ra_acc_x;
        //     // seekfree_assistant_oscilloscope_data.data[3] = (float)imu660ra_acc_y;
        //     // seekfree_assistant_oscilloscope_data.data[4] = (float)imu660ra_acc_z;
        //     // 3. 填充陀螺仪数据 (通道 5-7)
        //     seekfree_assistant_oscilloscope_data.data[5] = (float)imu660ra_gyro_x;
        //     seekfree_assistant_oscilloscope_data.data[6] = (float)imu660ra_gyro_y;
        //     seekfree_assistant_oscilloscope_data.data[7] = (float)imu660ra_gyro_z;
        //     // 4. 设置本次发送的通道数量 (一共8个数据)
        //     seekfree_assistant_oscilloscope_data.channel_num = 8;
        //     // 5. 调用发送函数
        //     seekfree_assistant_oscilloscope_send(&seekfree_assistant_oscilloscope_data);
        // #endif

        // system_delay_ms(50);


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

// **************************** 代码区域 ****************************
// **************************** 串口例程常见问题说明 ****************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：串口没有数据
//      查看串口助手打开的是否是正确的串口，检查打开的 COM 口是否对应的是调试下载器或者 USB-TTL 模块的 COM 口
//      如果是使用逐飞科技 CMSIS-DAP 调试下载器连接，那么检查下载器线是否松动，检查核心板串口跳线是否已经焊接，串口跳线查看核心板原理图即可找到
//      如果是使用 USB-TTL 模块连接，那么检查连线是否正常是否松动，模块 TX 是否连接的核心板的 RX，模块 RX 是否连接的核心板的 TX
// 问题2：串口数据乱码
//      查看串口助手设置的波特率是否与程序设置一致，程序中 zf_common_debug.h 文件中 DEBUG_UART_BAUDRATE 宏定义为 debug uart 使用的串口波特率




