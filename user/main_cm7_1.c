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
#include "code1/system_monitor.h"
    system_monitor_init();
        system_monitor_loop_begin();

        system_monitor_loop_end();
        system_monitor_update();

        {
            const system_monitor_info_t *sys_info = system_monitor_get_info();
            static float last_cpu_usage = -1.0f;
            if(sys_info->cpu_usage_percent != last_cpu_usage)
            {
                last_cpu_usage = sys_info->cpu_usage_percent;
                printf("[CM7_1] CPU: %.2f%%, RAM: %lu B",
                       sys_info->cpu_usage_percent,
                       (unsigned long)sys_info->ram_used_bytes);
                if(sys_info->ram_total_bytes > 0U)
                {
                    printf(" / %lu B (%.2f%%)",
                           (unsigned long)sys_info->ram_total_bytes,
                           sys_info->ram_usage_percent);
                }
                printf("\r\n");
            }
        }

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
* 文件名称          main_cm7_1
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

#include "zf_common_headfile.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设

// **************************** 代码区域 ****************************

int main(void)
{
    clock_init(SYSTEM_CLOCK_250M); 	// 时钟配置及系统初始化<务必保留>
    debug_info_init();                  // 调试串口信息初始化
     
    // 此处编写用户代码 例如外设初始化代码等
    
     // 初始化 WiFi 模块
    wifi_init();                                                                // 初始化WIFI模块

    // 连接TCP服务器
    wifi_connect_tcp_server();                                                  // 连接TCP服务器
    
    // 初始化摄像头和逐飞助手
    wifi_camera_init();                                                         // 初始化摄像头和逐飞助手
    
    

    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        // 此处编写需要循环执行的代码
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
        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************
