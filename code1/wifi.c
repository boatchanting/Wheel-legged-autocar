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

void wifi_reconnect_tcp_server(void)
{
    if(1 != WIFI_SPI_AUTO_CONNECT)
    {
        wifi_spi_socket_disconnect();
        system_delay_ms(10);
        // 使用非阻塞(短超时)连接，避免在脱机时卡死主循环
        wifi_spi_socket_connect_timeout(
            "TCP",
            TCP_TARGET_IP,
            TCP_TARGET_PORT,
            WIFI_LOCAL_PORT,
            10000 // 单位为100us，10000即1秒
        );
    }
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
    // 根据 WIFI_CAMERA_SEND_MODE 选择发送压缩图或原图。
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, WIFI_CAMERA_SEND_IMAGE_PTR, WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H);

#elif(1 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息只含有横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点)
    // 对边界数组写入数据
    for(int i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, WIFI_CAMERA_SEND_IMAGE_PTR, WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H);
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
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, WIFI_CAMERA_SEND_IMAGE_PTR, WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H);
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
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, WIFI_CAMERA_SEND_IMAGE_PTR, WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H);
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
    // 类型 4 不发送图像，仅发送边界；尺寸跟随当前发送模式，便于上位机坐标系保持一致。
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, NULL, WIFI_CAMERA_SEND_W, WIFI_CAMERA_SEND_H);
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

#if VISION_IMAGE_RENDER_ENABLE
static int clamp_int_to_range(int value, int min_value, int max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static int abs_int_value(int value)
{
    return (value < 0) ? -value : value;
}

static void draw_hline_on_image(int x0, int x1, int y, uint8 color)
{
    int start = (x0 < x1) ? x0 : x1;
    int end = (x0 < x1) ? x1 : x0;

    for (int x = start; x <= end; x++)
    {
        draw_point_on_image(x, y, color);
    }
}

static void draw_vline_on_image(int x, int y0, int y1, uint8 color)
{
    int start = (y0 < y1) ? y0 : y1;
    int end = (y0 < y1) ? y1 : y0;

    for (int y = start; y <= end; y++)
    {
        draw_point_on_image(x, y, color);
    }
}

static void draw_bar_on_image(int x, int y, int width, int value, int full_value, uint8 color)
{
    int fill_width;

    if (full_value <= 0)
    {
        return;
    }

    value = clamp_int_to_range(value, 0, full_value);
    fill_width = (value * width + full_value / 2) / full_value;

    draw_hline_on_image(x, x + width - 1, y, 180U);
    if (fill_width > 0)
    {
        draw_hline_on_image(x, x + fill_width - 1, y, color);
    }
}

static void draw_status_block_on_image(int x, int y, uint8 enabled, uint8 color)
{
    const uint8 off_color = 180U;
    uint8 draw_color = enabled ? color : off_color;

    draw_point_on_image(x,     y,     draw_color);
    draw_point_on_image(x + 1, y,     draw_color);
    draw_point_on_image(x,     y + 1, draw_color);
    draw_point_on_image(x + 1, y + 1, draw_color);
}

#if VISION_IMAGE_RENDER_NUMERIC_ENABLE
static const uint8 s_digit_3x5[10][5] =
{
    {7U, 5U, 5U, 5U, 7U},
    {2U, 6U, 2U, 2U, 7U},
    {7U, 1U, 7U, 4U, 7U},
    {7U, 1U, 7U, 1U, 7U},
    {5U, 5U, 7U, 1U, 1U},
    {7U, 4U, 7U, 1U, 7U},
    {7U, 4U, 7U, 5U, 7U},
    {7U, 1U, 1U, 1U, 1U},
    {7U, 5U, 7U, 5U, 7U},
    {7U, 5U, 7U, 1U, 7U}
};

static int draw_digit3x5_on_image(int x, int y, uint8 digit, uint8 color)
{
    if (digit > 9U)
    {
        return x;
    }

    for (int row = 0; row < 5; row++)
    {
        uint8 bits = s_digit_3x5[digit][row];
        for (int col = 0; col < 3; col++)
        {
            if ((bits & (uint8)(1U << (2 - col))) != 0U)
            {
                draw_point_on_image(x + col, y + row, color);
            }
        }
    }
    return x + 4;
}

static int draw_minus3x5_on_image(int x, int y, uint8 color)
{
    draw_hline_on_image(x, x + 2, y + 2, color);
    return x + 4;
}

static int draw_uint3x5_on_image(int x, int y, uint32 value, uint8 color)
{
    char digits[10];
    int count = 0;

    if (value == 0U)
    {
        return draw_digit3x5_on_image(x, y, 0U, color);
    }

    while ((value > 0U) && (count < (int)sizeof(digits)))
    {
        digits[count++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (count > 0)
    {
        count--;
        x = draw_digit3x5_on_image(x, y, (uint8)(digits[count] - '0'), color);
    }

    return x;
}

static void draw_int3x5_on_image(int x, int y, int value, uint8 color)
{
    if (value < 0)
    {
        x = draw_minus3x5_on_image(x, y, color);
    }
    (void)draw_uint3x5_on_image(x, y, (uint32)abs_int_value(value), color);
}
#endif

static void render_common_status_strip(uint8 raw_detected,
                                       uint8 stable_detected,
                                       uint8 aux_raw_detected,
                                       uint8 aux_stable_detected)
{
    draw_status_block_on_image(PVC_IMAGE_W - 10, 0, raw_detected, 0U);
    draw_status_block_on_image(PVC_IMAGE_W - 7, 0, stable_detected, 0U);
    draw_status_block_on_image(PVC_IMAGE_W - 4, 0, aux_raw_detected, 255U);
    draw_status_block_on_image(PVC_IMAGE_W - 1, 0, aux_stable_detected, 255U);
}
#endif

/**
 * @brief  将 PVC 视觉识别的结果直接渲染到 compressed_image_copy 图像数组中。
 * @note   由于当前算法就是基于 94x60 输出，我们也是把框画在 94x60 压缩图上供上位机显示，
 *         因此此处坐标1:1对应，不再需要 *2 还原。
 */
void render_pvc_vision_to_image(void) 
{
    const volatile pvc_vision_output_t *pvc_out = &g_pvc_vision_output;
    pvc_vision_frame_result_t result;
    uint8 has_target = 0U;

#if VISION_IMAGE_RENDER_ENABLE
    render_common_status_strip(pvc_out->raw_detected, pvc_out->stable_detected, 0U, 0U);
#endif

    if (pvc_out->stable_detected)
    {
        result = pvc_out->stable;
        has_target = 1U;
    }
    else if (pvc_out->raw_detected)
    {
        result = pvc_out->raw;
        has_target = 1U;
    }

#if VISION_IMAGE_RENDER_ENABLE
    {
        int conf_x1000 = clamp_int_to_range((int)(pvc_out->raw.confidence * 1000.0f), 0, 1000);
        int bbox_ratio_x1000 = 0;
        int cost_us = (int)g_pvc_vision_cost_profiler.last_us;

        if (has_target && result.bbox_xmin != 0xFFU)
        {
            int bbox_w = (int)result.bbox_xmax - (int)result.bbox_xmin + 1;
            int bbox_h = (int)result.bbox_ymax - (int)result.bbox_ymin + 1;
            bbox_ratio_x1000 = clamp_int_to_range((bbox_w * bbox_h * 1000) / (int)PVC_IMAGE_SIZE, 0, 1000);
        }

        draw_bar_on_image(0, 0, 22, conf_x1000, 1000, 0U);
        draw_bar_on_image(0, 2, 22, bbox_ratio_x1000, 1000, 0U);
        draw_bar_on_image(0, 4, 22, clamp_int_to_range(cost_us, 0, 10000), 10000, 0U);

#if VISION_IMAGE_RENDER_NUMERIC_ENABLE
        (void)draw_uint3x5_on_image(0, 6, (uint32)conf_x1000, 0U);
        if (has_target)
        {
            draw_int3x5_on_image(25, 6, (int)result.forward_mm, 0U);
            draw_int3x5_on_image(55, 6, (int)result.lateral_mm, 0U);
        }
#endif
    }
#endif

    if (has_target)
    {
        uint8 xmin = result.bbox_xmin;
        uint8 ymin = result.bbox_ymin;
        uint8 xmax = result.bbox_xmax;
        uint8 ymax = result.bbox_ymax;
        uint8 box_color = pvc_out->stable_detected ? 0U : 180U;
        int cx = (int)result.centroid_x;
        int cy = (int)result.centroid_y;
        uint8 entry_y = result.entry_bottom_y;

        draw_rect_on_image(xmin, ymin, xmax, ymax, box_color);
        draw_cross_on_image(cx, cy, 3, 0U);

        for (int x = 0; x < PVC_IMAGE_W; x += 2) 
        { 
            draw_point_on_image(x, entry_y, 0U);
        }

#if VISION_IMAGE_RENDER_ENABLE
        draw_vline_on_image(cx, ymax, PVC_IMAGE_H - 1, 0U);
        draw_hline_on_image(PVC_IMAGE_W / 2 - 5, PVC_IMAGE_W / 2 + 5, PVC_IMAGE_H - 1, 0U);
#endif
    }
}

/* Debug rendering is kept outside bridge_vision so it never affects detector
 * input.  Call it only after bridge_vision_process_camera_frame(). */
static void draw_line_on_image(int x0, int y0, int x1, int y1, uint8 color)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y0 - y1) : (y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (1)
    {
        draw_point_on_image(x0, y0, color);
        if ((x0 == x1) && (y0 == y1))
        {
            break;
        }
        {
            int e2 = err * 2;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }
}

void render_bridge_vision_to_image(void)
{
    const volatile bridge_vision_output_t *bridge_out = bridge_vision_get_output();
    bridge_vision_frame_result_t result;

    if ((bridge_out->bridge_stable_detected != 0U) ||
        (bridge_out->stable_detected != 0U))
    {
        result = bridge_out->stable;
    }
    else
    {
        result = bridge_out->raw;
    }

    if ((result.left_line_x0 >= 0) && (result.left_line_y0 >= 0) &&
        (result.left_line_x1 >= 0) && (result.left_line_y1 >= 0))
    {
        draw_line_on_image((int)result.left_line_x0,
                           (int)result.left_line_y0,
                           (int)result.left_line_x1,
                           (int)result.left_line_y1,
                           0U);
    }

    if ((result.right_line_x0 >= 0) && (result.right_line_y0 >= 0) &&
        (result.right_line_x1 >= 0) && (result.right_line_y1 >= 0))
    {
        draw_line_on_image((int)result.right_line_x0,
                           (int)result.right_line_y0,
                           (int)result.right_line_x1,
                           (int)result.right_line_y1,
                           0U);
    }

    if ((result.up_line_x0 >= 0) && (result.up_line_y0 >= 0) &&
        (result.up_line_x1 >= 0) && (result.up_line_y1 >= 0))
    {
        draw_line_on_image((int)result.up_line_x0,
                           (int)result.up_line_y0,
                           (int)result.up_line_x1,
                           (int)result.up_line_y1,
                           0U);
    }

    if ((result.down_line_x0 >= 0) && (result.down_line_y0 >= 0) &&
        (result.down_line_x1 >= 0) && (result.down_line_y1 >= 0))
    {
        draw_line_on_image((int)result.down_line_x0,
                           (int)result.down_line_y0,
                           (int)result.down_line_x1,
                           (int)result.down_line_y1,
                           0U);
    }

    /* 中线是控制真正使用的几何输出，最后单独强调画出来。 */
    if (result.geometry_valid != 0U)
    {
        draw_line_on_image((int)result.center_line_x0,
                           (int)result.center_line_y0,
                           (int)result.center_line_x1,
                           (int)result.center_line_y1,
                           0U);
        draw_cross_on_image((int)result.center_line_x1,
                            (int)result.center_line_y1,
                            2,
                            0U);
    }
}

#if 0
/* Removed legacy line-vision renderer retained only as disabled history. */
void render_line_vision_to_image(void)
{
    const volatile line_vision_output_t *line_out = &g_line_vision_output;
    line_vision_frame_result_t result;
    const int y_min = (int)((uint32)LINE_IMAGE_H * LINE_VISION_ROI_TOP_RATIO_X100 / 100U);
    const int y_max = (int)(LINE_IMAGE_H - 2U);

    if (line_out->stable_detected || line_out->bridge_stable_detected)
    {
        result = line_out->stable;
    }
    else
    {
        result = line_out->raw;
    }

#if VISION_IMAGE_RENDER_ENABLE
    render_common_status_strip(line_out->raw_detected,
                               line_out->stable_detected,
                               line_out->bridge_raw_detected,
                               line_out->bridge_stable_detected);
    {
        int line_conf_x1000 = clamp_int_to_range((int)(result.confidence * 1000.0f), 0, 1000);
        int bridge_conf_x1000 = clamp_int_to_range((int)(result.bridge_confidence * 1000.0f), 0, 1000);
        int roi_white_x1000 = clamp_int_to_range((int)(result.roi_white_ratio * 1000.0f), 0, 1000);
        int cost_us = (int)g_line_vision_cost_profiler.last_us;

        draw_bar_on_image(0, 0, 22, line_conf_x1000, 1000, 0U);
        draw_bar_on_image(0, 2, 22, bridge_conf_x1000, 1000, 0U);
        draw_bar_on_image(0, 4, 22, roi_white_x1000, 1000, 0U);
        draw_bar_on_image(24, 0, 16, clamp_int_to_range(cost_us, 0, 10000), 10000, 0U);

#if VISION_IMAGE_RENDER_NUMERIC_ENABLE
        (void)draw_uint3x5_on_image(0, 6, (uint32)line_conf_x1000, 0U);
        draw_int3x5_on_image(25, 6, (int)result.yaw_error_deg, 0U);
        draw_int3x5_on_image(45, 6, (int)result.lateral_error_px, 0U);
        (void)draw_uint3x5_on_image(68, 6, (uint32)bridge_conf_x1000, 255U);
#endif
    }
#endif

    for (int x = 0; x < LINE_IMAGE_W; x += 2)
    {
        draw_point_on_image(x, y_min, 0U);
    }
    for (int y = y_min; y <= y_max; y += 2)
    {
        draw_point_on_image(LINE_IMAGE_W / 2, y, 0U);
    }

#if VISION_IMAGE_RENDER_ENABLE
    {
        const int lookahead_y = (int)((uint32)LINE_IMAGE_H * 62U / 100U);
        for (int x = 0; x < LINE_IMAGE_W; x += 3)
        {
            draw_point_on_image(x, lookahead_y, 180U);
        }
    }
#endif

    if (line_out->bridge_stable_detected || line_out->bridge_raw_detected)
    {
        if (result.bridge_bbox_xmin != 0xFFU)
        {
            draw_rect_on_image(result.bridge_bbox_xmin,
                               result.bridge_bbox_ymin,
                               result.bridge_bbox_xmax,
                               result.bridge_bbox_ymax,
                               255U);
        }
    }

    if (line_out->stable_detected)
    {
        int x_bottom = (int)(result.line_x_bottom + 0.5f);
        int x_lookahead = (int)(result.line_x_lookahead + 0.5f);
        const int lookahead_y = (int)((uint32)LINE_IMAGE_H * 62U / 100U);

        /* 1. 利用两点算出斜率变化率 dx/dy */
        float dx_dy = (float)(x_lookahead - x_bottom) / (float)(lookahead_y - y_max);
        
        /* 2. 推算出画面最顶部(y_min)应该在哪个 X 坐标 */
        int x_top = x_bottom + (int)(dx_dy * (float)(y_min - y_max));

        /* 3. 画一条从屏幕最底下，直插屏幕最顶部的“长矛”引导线 */
        draw_line_on_image(x_top, y_min, x_bottom, y_max, 0U); 

        /* 4. 在重要位置画十字靶心 */
        draw_cross_on_image(x_bottom, y_max, 3, 0U);       // 底部控制点
        draw_cross_on_image(x_lookahead, lookahead_y, 3, 0U); // 预瞄控制点

#if VISION_IMAGE_RENDER_ENABLE
        draw_vline_on_image(x_bottom, y_max - 5, y_max, 0U);
        draw_vline_on_image(x_lookahead, lookahead_y - 3, lookahead_y + 3, 0U);
#endif
    }
}

#endif

void render_bumpy_vision_to_image(void)
{
    const volatile bumpy_vision_output_t *bumpy_out = &g_bumpy_vision_output;
    bumpy_vision_frame_result_t result;

    /* 1. 决定画稳定数据还是原始数据 */
    if (bumpy_out->stable_detected)
    {
        result = bumpy_out->stable;
    }
    else
    {
        result = bumpy_out->raw;
    }

    if ((bumpy_out->stable_detected != 0U) || (bumpy_out->raw_detected != 0U))
    {
        const int center_x = BUMPY_IMAGE_W / 2;
        const int center_y = BUMPY_IMAGE_H / 2;
        const int line_length = (BUMPY_IMAGE_H * 46) / 100;
        const int x0 = clamp_int_to_range(center_x - (int)(result.direction_x * line_length),
                                          0,
                                          BUMPY_IMAGE_W - 1);
        const int y0 = clamp_int_to_range(center_y - (int)(result.direction_y * line_length),
                                          0,
                                          BUMPY_IMAGE_H - 1);
        const int x1 = clamp_int_to_range(center_x + (int)(result.direction_x * line_length),
                                          0,
                                          BUMPY_IMAGE_W - 1);
        const int y1 = clamp_int_to_range(center_y + (int)(result.direction_y * line_length),
                                          0,
                                          BUMPY_IMAGE_H - 1);

        draw_line_on_image(x0, y0, x1, y1, 0U);
    }

    return;

#if 0
#if VISION_IMAGE_RENDER_ENABLE
    /* 画顶部的通用状态指示灯 (借用原有函数，没有桥梁则后两个参数填 0) */
    render_common_status_strip(bumpy_out->raw_detected,
                               bumpy_out->stable_detected,
                               0U, 
                               0U);

    {
        /* 提取需要显示的指标并限制范围 */
        int conf = clamp_int_to_range((int)result.confidence_u16, 0, 1000);
        int cost_us = (int)g_bumpy_vision_cost_profiler.last_us;
        int phase = (int)result.phase;
        int rib_count = (int)result.rib_count;
        int dir_x_x100 = (int)(result.direction_x * 100.0f);
        int dir_y_x100 = (int)(result.direction_y * 100.0f);

        /* 画进度条：置信度 (左侧) 和 耗时 (右侧) */
        draw_bar_on_image(0, 0, 22, conf, 1000, 0U);
        draw_bar_on_image(24, 0, 16, clamp_int_to_range(cost_us, 0, 10000), 10000, 0U);
        draw_int3x5_on_image(70, 4, dir_x_x100, 0U);
        draw_int3x5_on_image(82, 4, dir_y_x100, 0U);

#if VISION_IMAGE_RENDER_NUMERIC_ENABLE
        /* 打印具体微型数字：置信度 | 阶段Phase | 识别到的黑条数 | 转向误差 */
        (void)draw_uint3x5_on_image(0, 4, (uint32)conf, 0U);
        (void)draw_uint3x5_on_image(25, 4, (uint32)phase, 0U);           /* 关键状态机参数 */
        (void)draw_uint3x5_on_image(35, 4, (uint32)rib_count, 0U);       /* 颠簸黑条数 */
        draw_int3x5_on_image(45, 4, (int)(result.steer_error_px_x100 / 100), 0U); /* 偏差 */
#endif
    }
#endif

    /* 2. 画出检测的 ROI (感兴趣区域) 边界虚线，明确算法看哪里 */
    for (int x = 0; x < BUMPY_IMAGE_W; x += 3)
    {
        draw_point_on_image(x, BUMPY_ROI_Y0, 0U); /* 顶部边界 */
        draw_point_on_image(x, BUMPY_ROI_Y1, 0U); /* 底部边界 */
    }

    /* 3. 画出识别到的特征结果 */
    if (bumpy_out->stable_detected || bumpy_out->raw_detected)
    {
        /* --- 画出颠簸白色底板的包围盒 (Bounding Box) --- */
        if (result.bbox_xmin != 0xFFU && result.bbox_xmax != 0xFFU)
        {
            /* 用白色 (255U) 画一个框，把找到的白色大板块框起来 */
            draw_rect_on_image(result.bbox_xmin,
                               result.bbox_ymin,
                               result.bbox_xmax,
                               result.bbox_ymax,
                               255U);
        }

        /* --- 画出引导目标线 (Centerline/Target) --- */
        if (result.target_x_px_x100 != 0) 
        {
            /* 将放大了100倍的坐标还原回实际像素 */
            int target_x = (int)(result.target_x_px_x100 / 100.0f + 0.5f);
            
            /* 确定线条的上下边界，如果没有有效边界就用 ROI 边界 */
            int top_y = (result.centerline_top_y != 0xFFU) ? result.centerline_top_y : BUMPY_ROI_Y0;
            int bottom_y = (result.centerline_bottom_y != 0xFFU) ? result.centerline_bottom_y : BUMPY_ROI_Y1;

            /* 画一条垂直的中心虚线，代表车要对准的 X 坐标 */
            for (int y = top_y; y <= bottom_y; y += 2)
            {
                draw_point_on_image(target_x, y, 0U);
            }
            
            /* 在底部画一个大十字准星，代表我们要转向的目标瞄准点 */
            draw_cross_on_image(target_x, bottom_y, 3, 0U);
        }
    }
#endif
}
