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
#include "../code1/wifi.h"
#include "../code1/wifi_diff_stream.h"
#include "../code1/wifi_protocol.h"
#include "../code1/vision/pvc_vision.h"
#include "../code1/vision/bumpy_vision.h"
#include "../code1/vision/vision_ipc_core1.h"
#include "../code1/vision/telemetry_ipc_core1.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完

// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设
// 本例程是开源库空工程 可用作移植或者测试各类内外设

// **************************** 代码区域 ****************************
#define VISION_IPC_PIT_NUM     (PIT_CH2)

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
    wifi_diff_stream_init(PVC_IMAGE_W, PVC_IMAGE_H, 100U, 2U);                 // init realtime diff stream
    // mt9v03x_init();//初始化摄像头
    pvc_vision_init();                                                          // 初始化 PVC 入口视觉检测与帧率/耗时统计
    line_vision_init();                                                         // 初始化任务区直线/单边桥视觉检测
    bumpy_vision_init();                                                        // 初始化颠簸路段视觉检测
    VisionIpc_Core1_Init();                                                     // 初始化1核视觉共享内存结果发布
    pit_ms_init(VISION_IPC_PIT_NUM, 2);                                          // 2ms 中断中处理0/1核视觉通信
    interrupt_global_enable(0);


    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        wifi_protocol_poll_rx();
        wifi_protocol_send_oscilloscope();
        // 此处编写需要循环执行的代码
                // 处理摄像头图像数据
        if(mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;
            // 在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

            compress_image_to_target();// 将原图压缩至 compressed_image_copy (94*60)

            if(VisionIpc_Core1_TakePvcResetRequest())
            {
                pvc_vision_reset_filter();
            }
            if(VisionIpc_Core1_TakeLineResetRequest())
            {
                line_vision_reset_filter();
            }
            if(VisionIpc_Core1_TakeBumpyResetRequest())
            {
                bumpy_vision_reset_filter();
            }

            if(VisionIpc_Core1_ShouldRunPvc())
            {

                pvc_vision_process_camera_frame(compressed_image_copy[0]);//将压缩图像输入到 PVC 检测算法中

                // 4. 将 PVC 检测框直接画在 compressed_image_copy[0] 上，供 WIFI 发送显示
                render_pvc_vision_to_image();//算法执行完毕后，将 PVC 检测框画在 image_copy 上,必须放在这！如果放在算法前面，画的黑线会破坏算法寻找白色的逻辑
            }
            if(VisionIpc_Core1_ShouldRunBridgeLine())
            {
                line_vision_process_camera_frame(compressed_image_copy[0]);
                render_line_vision_to_image();
            }
            if(VisionIpc_Core1_ShouldRunBumpy())
            {
                bumpy_vision_process_camera_frame(compressed_image_copy[0]);
                render_bumpy_vision_to_image();
            }
            // 发送图像
            wifi_diff_stream_send_gray_frame((const uint8 *)compressed_image_copy[0]);
            // 如果使用UDP协议传输数据则推荐在数据全部发送到模块之后立即调用wifi_spi_udp_send_now()函数，以告知模块立即将收到的数据发送到网络上
            // 如果没有立即调用则模块会在持续2毫秒未收到数据后，将数据发送到网络上
            // 调用wifi_spi_udp_send_now()前传输给模块的数据数量建议不要超过40960字节
            // wifi_spi_udp_send_now();
        }
        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************

