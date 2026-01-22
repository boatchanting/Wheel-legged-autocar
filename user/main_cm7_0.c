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
#define WIFI_USE 0 // 【全局开关】选择是否使用WIFI模块，0表示不使用，1表示使用
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

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等

    // *************************** 屏幕初始化开始 ***************************
    // 定义一个变量用于记录屏幕打印的Y坐标（行号）
    uint16 disp_y = 0; 

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

    uart_rx_interrupt(UART_INDEX, 1);                                           // 开启 UART_INDEX 的接收中断
    // --- 屏幕打印无刷电机初始化完成 ---
#if DEBUG_DISPLAY
    ips200_show_string(0, disp_y, "UART INTERRUPT Init OK");
    disp_y += 16;
#endif

    //-------------------------------------------------------------------
    //******************************系统初始化结束************************
    //-------------------------------------------------------------------

    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        // 此处编写需要循环执行的代码

        fifo_data_count = fifo_used(&uart_data_fifo);                           // 查看 fifo 是否有数据
        if(fifo_data_count != 0)                                                // 读取到数据了
        {
            fifo_read_buffer(&uart_data_fifo, fifo_get_data, &fifo_data_count, FIFO_READ_AND_CLEAN);    // 将 fifo 中数据读出并清空 fifo 挂载的缓冲
            uart_write_string(UART_INDEX, "\r\nUART get data:");                // 输出测试信息
            uart_write_buffer(UART_INDEX, fifo_get_data, fifo_data_count);      // 将读取到的数据发送出去
        }

        // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
        #if WIFI_USE
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


        
        ips200_show_int(0, 0, duty,2);

        small_driver_set_duty(duty * (PWM_DUTY_MAX / 100), -duty * (PWM_DUTY_MAX / 100));   // 计算占空比输出

        if(dir)                                                                 // 根据方向判断计数方向 本例程仅作参考
        {
            duty ++;                                                            // 正向计数
            if(duty >= MAX_DUTY)                                                // 达到最大值
            {
                dir = false;                                                    // 变更计数方向
            }
        }
        else
        {
            duty --;                                                            // 反向计数
            if(duty <= -MAX_DUTY)                                               // 达到最小值
            {
                dir = true;                                                     // 变更计数方向
            }
        }
        // printf("motor\r\n");
        // printf("left speed:%d, right speed:%d\r\n", motor_value.receive_left_speed_data, motor_value.receive_right_speed_data);




        system_delay_ms(50);


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




