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
* 文件名称          wifi.c
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
int flash_write = 0;      // flash写使能标志位
int flash_write_flag = 0; // flash写标志位
#include "wifi.h"
#include "zf_common_headfile.h"

// 只有X边界
uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];

// 只有Y边界
uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];

// X Y边界都是单独指定的
uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];

// 图像备份数组，在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
// 【保持原样】这里是原图大小 188 * 120 的备份，其他不相干算法仍可以使用它
uint8 image_copy[MT9V03X_H][MT9V03X_W];
uint8 compressed_image_copy[PVC_IMAGE_H][PVC_IMAGE_W];

// =================================================================================
// ★★★ 新增：图像降采样压缩函数 ★★★
// =================================================================================
/**
 * @brief  将 188*120 的原图压缩成 94*60 的图像 (2x2均值池化)
 * @note   请在主循环(摄像头采集完成回调)中，memcpy 到 image_copy 后调用本函数，
 *         然后在将 compressed_image_copy 传给算法和进行渲染
 */
void compress_image_to_target(void) 
{
    for (int y = 0; y < PVC_IMAGE_H; y++) 
    {
        int src_y = y * 2; // 原图 188x120 对应的起始行
        for (int x = 0; x < PVC_IMAGE_W; x++) 
        {
            int src_x = x * 2; // 原图 188x120 对应的起始列
            
            // 获取 2x2 区域的 4 个像素灰度值，求和后右移 2 位实现快速求平均值
            uint32 sum = image_copy[src_y][src_x] +
                         image_copy[src_y][src_x + 1] +
                         image_copy[src_y + 1][src_x] +
                         image_copy[src_y + 1][src_x + 1];
                         
            compressed_image_copy[y][x] = (uint8)(sum >> 2); 
        }
    }
}


// 函数实现
void wifi_init(void)
{
    while(wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST))
    {
        printf("\r\n connect wifi failed. \r\n");
        system_delay_ms(100);                                                   // 初始化失败 等待 100ms
    }

    printf("\r\n module version:%s",wifi_spi_version);                          // 模块固件版本
    printf("\r\n module mac    :%s",wifi_spi_mac_addr);                         // 模块 MAC 信息
    printf("\r\n module ip     :%s",wifi_spi_ip_addr_port);                     // 模块 IP 地址
}

void wifi_connect_tcp_server(void)
{
    // zf_device_wifi_spi.h 文件内的宏定义可以更改模块连接(建立) WIFI 之后，是否自动连接 TCP 服务器、创建 UDP 连接
    if(1 != WIFI_SPI_AUTO_CONNECT)                                              // 如果没有开启自动连接 就需要手动连接目标 IP
    {
        while(wifi_spi_socket_connect(                                          // 向指定目标 IP 的端口建立 TCP 连接
            "TCP",                                                              // 指定使用TCP方式通讯
            TCP_TARGET_IP,                                                      // 指定远端的IP地址，填写上位机的IP地址
            TCP_TARGET_PORT,                                                    // 指定远端的端口号，填写上位机的端口号，通常上位机默认是8080
            WIFI_LOCAL_PORT))                                                   // 指定本机的端口号
        {
            // 如果一直建立失败 考虑一下是不是没有接硬件复位
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);                                               // 建立连接失败 等待 100ms
        }
    }
}

void wifi_camera_init(void)
{
    // 推荐先初始化摄像头，后初始化逐飞助手
    mt9v03x_init();

    // 逐飞助手初始化 数据传输使用高速WIFI SPI
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);

    // 如果要发送图像信息，则务必调用seekfree_assistant_camera_information_config函数进行必要的参数设置
    // 如果需要发送边线则还需调用seekfree_assistant_camera_boundary_config函数设置边线的信息

#if(0 == INCLUDE_BOUNDARY_TYPE)
    // 【修改】发送的图像从 image_copy 更改为 compressed_image_copy (带有识别框的压缩图)，尺寸改为 PVC_IMAGE_W 和 PVC_IMAGE_H
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, compressed_image_copy[0], PVC_IMAGE_W, PVC_IMAGE_H);

#elif(1 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息只含有横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点)
    // 对边界数组写入数据
    for(int i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    // 【修改】发送压缩后的图像
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, compressed_image_copy[0], PVC_IMAGE_W, PVC_IMAGE_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL ,NULL);

#elif(2 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息只含有纵轴坐标，横轴坐标由图像宽度得到，意味着每个边界在一列中只会有一个点)
    // 通常很少有这样的使用需求
    // 对边界数组写入数据
    for(int i = 0; i < MT9V03X_W; i++)
    {
        y1_boundary[i] = i * MT9V03X_H / MT9V03X_W;
        y2_boundary[i] = MT9V03X_H / 2;
        y3_boundary[i] = (MT9V03X_W - i) * MT9V03X_H / MT9V03X_W;
    }
    // 【修改】发送压缩后的图像
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, compressed_image_copy[0], PVC_IMAGE_W, PVC_IMAGE_H);
    seekfree_assistant_camera_boundary_config(Y_BOUNDARY, MT9V03X_W, NULL, NULL ,NULL, y1_boundary, y2_boundary, y3_boundary);

#elif(3 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息含有横纵轴坐标)
    // 这样的方式可以实现对于有回弯的边界显示
    int j = 0;
    for(int i = MT9V03X_H - 1; i >= MT9V03X_H / 2; i--)
    {
        // 直线部分
        xy_x1_boundary[j] = 34;
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = 47;
        xy_y2_boundary[j] = i;

        xy_x3_boundary[j] = 60;
        xy_y3_boundary[j] = i;
        j++;
    }

    for(int i = MT9V03X_H / 2 - 1; i >= 0; i--)
    {
        // 直线连接弯道部分
        xy_x1_boundary[j] = 34 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 34) / (MT9V03X_H / 2);
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = 47 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 47) / (MT9V03X_H / 2);
        xy_y2_boundary[j] = 15 + i * 3 / 4;

        xy_x3_boundary[j] = 60 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 60) / (MT9V03X_H / 2);
        xy_y3_boundary[j] = 30 + i / 2;
        j++;
    }

    for(int i = 0; i < MT9V03X_H / 2; i++)
    {
        // 回弯部分
        xy_x1_boundary[j] = MT9V03X_W / 2 + i * (138 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = MT9V03X_W / 2 + i * (133 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y2_boundary[j] = 15 + i * 3 / 4;

        xy_x3_boundary[j] = MT9V03X_W / 2 + i * (128 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y3_boundary[j] = 30 + i / 2;
        j++;
    }
    // 【修改】发送压缩后的图像
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, compressed_image_copy[0], PVC_IMAGE_W, PVC_IMAGE_H);
    seekfree_assistant_camera_boundary_config(XY_BOUNDARY, BOUNDARY_NUM, xy_x1_boundary, xy_x2_boundary, xy_x3_boundary, xy_y1_boundary, xy_y2_boundary, xy_y3_boundary);

#elif(4 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息只包含横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点)
    // 对边界数组写入数据
    for(int i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    // 【修改】由于原本发送 NULL，此处若无发图需求可不动，或者统一跟随
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, NULL, PVC_IMAGE_W, PVC_IMAGE_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL ,NULL);

#endif
}


// =================================================================================
// ★★★ 上位机参数调节映射 (PC -> MCU) ★★★
// =================================================================================
void wifi_update_pid_params(void)
{
    seekfree_assistant_data_analysis();
    for(uint8_t i = 0; i < SEEKFREE_ASSISTANT_SET_PARAMETR_COUNT; i++)
    {
        if(seekfree_assistant_parameter_update_flag[i])
        {
            seekfree_assistant_parameter_update_flag[i] = 0;
            switch(i)
            {
                case 0: g_jump_profile.t_launch =  (uint32_t)seekfree_assistant_parameter[i]; break;
                case 1:g_jump_profile.t_flight   = (uint32_t)seekfree_assistant_parameter[i]; break;
                case 2:g_jump_profile.t_landing  = (uint32_t)seekfree_assistant_parameter[i]; break;
                case 3:   g_jump_profile.t_recovery  = (uint32_t)seekfree_assistant_parameter[i]; break;
                case 4:g_jump_profile.offset_launch = (int32_t)seekfree_assistant_parameter[i]; break;
                case 5:  g_jump_profile.offset_flight = (int32_t)seekfree_assistant_parameter[i]; break;
                case 6: g_jump_profile.offset_land  = (int32_t)seekfree_assistant_parameter[i]; break;
                case 7:vision_detected_jump_point= (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                default: break;
            }
        }
    }
}

void encode_double_to_two_floats(double value, float* out_high, float* out_low) {
    uint64_t u64;
    memcpy(&u64, &value, sizeof(double));          

    uint32_t high = (uint32_t)(u64 >> 32);         
    uint32_t low  = (uint32_t)(u64 & 0xFFFFFFFFU); 

    memcpy(out_high, &high, sizeof(uint32_t));     
    memcpy(out_low,  &low,  sizeof(uint32_t));     
}


// =================================================================================
// ★★★ 图像渲染辅助函数 (用于在 compressed_image_copy 上绘制 PVC 检测框等) ★★★
// =================================================================================

/**
 * @brief  在 compressed_image_copy 上安全画点 (防止越界导致异常)
 * @param  x 图像 X 坐标
 * @param  y 图像 Y 坐标
 * @param  color 像素颜色（0 为纯黑，255 为纯白）
 */
void draw_point_on_image(int x, int y, uint8 color) 
{
    // 【修改】边界检查从原图宽/高更改为压缩算法图宽高: PVC_IMAGE_W 和 PVC_IMAGE_H
    if (x >= 0 && x < PVC_IMAGE_W && y >= 0 && y < PVC_IMAGE_H) 
    {
        // 【修改】渲染对象更改为 compressed_image_copy 
        compressed_image_copy[y][x] = color;
    }
}

void draw_rect_on_image(int x_min, int y_min, int x_max, int y_max, uint8 color) 
{
    for (int x = x_min; x <= x_max; x++) 
    {
        draw_point_on_image(x, y_min, color);
        draw_point_on_image(x, y_max, color);
    }
    for (int y = y_min; y <= y_max; y++) 
    {
        draw_point_on_image(x_min, y, color);
        draw_point_on_image(x_max, y, color);
    }
}

void draw_cross_on_image(int x, int y, int size, uint8 color) 
{
    for(int i = -size; i <= size; i++) 
    {
        draw_point_on_image(x + i, y, color);
        draw_point_on_image(x, y + i, color);
    }
}

/**
 * @brief  将 PVC 视觉识别的结果直接渲染到 compressed_image_copy 图像数组中。
 * @note   由于当前算法就是基于 94x60 输出，我们也是把框画在 94x60 压缩图上供上位机显示，
 *         因此此处坐标1:1对应，不再需要 *2 还原。
 */
void render_pvc_vision_to_image(void) 
{
    // 读取视觉模块的输出
    const volatile pvc_vision_output_t *pvc_out = &g_pvc_vision_output;

    // 只有当稳定检测到 PVC 目标时才在屏幕上画图
    if (pvc_out->stable_detected) 
    {
        // 1. 获取包围框 (Bounding Box) 坐标
        // 【修改】不需要做 *2 映射！直接在 94x60 图上画
        uint8 xmin = pvc_out->stable.bbox_xmin;
        uint8 ymin = pvc_out->stable.bbox_ymin;
        uint8 xmax = pvc_out->stable.bbox_xmax;
        uint8 ymax = pvc_out->stable.bbox_ymax;

        // 在图像上画一个纯黑色的矩形框
        draw_rect_on_image(xmin, ymin, xmax, ymax, 0);

        // 2. 获取目标质心坐标
        int cx = (int)pvc_out->stable.centroid_x;
        int cy = (int)pvc_out->stable.centroid_y;

        // 在质心位置画一个纯黑色的十字，臂长为 3 个像素
        draw_cross_on_image(cx, cy, 3, 0);

        // 3. （可选）绘制入口白边近端线 entry_bottom_y 
        uint8 entry_y = pvc_out->stable.entry_bottom_y;
        
        // 【修改】循环上限从 MT9V03X_W 修改为算法宽度 PVC_IMAGE_W
        for (int x = 0; x < PVC_IMAGE_W; x += 2) 
        { 
            draw_point_on_image(x, entry_y, 0);
        }
    }
}