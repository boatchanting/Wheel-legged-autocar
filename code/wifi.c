/*********************************************************************************************************************
* 文件名称          wifi.c
* 功能说明          WiFi 通信模块封装实现
* 备注信息          包含 WiFi 连接、TCP Socket 建立及逐飞助手协议配置
********************************************************************************************************************/
#include "wifi.h"

// ================== 【新增】变量定义 ==================
// Flash 存储配置 (请确认 CYT4BB 的 Flash 扇区是否可用，通常沿用即可)
#define FLASH_SECTION_INDEX       (0)     // 存储数据用的扇区
#define FLASH_PAGE_INDEX          (8)     // 存储数据用的页码

// 全局控制变量
int jump_tf = 0;              // 你的 ISR 需要这个变量
float target_v = 0;
float err_degree = 0;
float rolling_target_degree = 0;
int run_flag = 0;
int high_flag = 0;
float high = 0;               // 临时变量

// 参数与通信变量
float data[8] = {0};
int sum = 0;
seekfree_assistant_oscilloscope_struct oscilloscope_data;

// ------------------------------------------ 静态全局变量 ------------------------------------------

// 边界数组定义
static uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
static uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];
static uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];
static uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];

// 图像备份数组：用于解决 DMA 传输与 WiFi 发送不同步导致的"图像撕裂"问题
static uint8 image_copy[MT9V03X_H][MT9V03X_W];

/**
 * @brief  内部函数：配置逐飞助手协议的边界数据
 */
static void wifi_assistant_config(void) {
    int32 i, j;

    // 1. 初始化逐飞助手接口，指定使用 WIFI_SPI 模式
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);

    // 2. 根据 INCLUDE_BOUNDARY_TYPE 宏定义配置发送协议
#if(0 == INCLUDE_BOUNDARY_TYPE)
    // 仅发送原始图像
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);

#elif(1 == INCLUDE_BOUNDARY_TYPE)
    // 示例：生成模拟 X 边界数据
    for(i = 0; i < MT9V03X_H; i++) {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL, NULL);

#elif(3 == INCLUDE_BOUNDARY_TYPE)
    // 示例：生成模拟 XY 回弯边界数据
    j = 0;
    for(i = MT9V03X_H - 1; i >= 0; i--) {
        xy_x1_boundary[j] = 34; xy_y1_boundary[j] = i;
        xy_x2_boundary[j] = 47; xy_y2_boundary[j] = i;
        xy_x3_boundary[j] = 60; xy_y3_boundary[j] = i;
        j++;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(XY_BOUNDARY, BOUNDARY_NUM, xy_x1_boundary, xy_x2_boundary, xy_x3_boundary, xy_y1_boundary, xy_y2_boundary, xy_y3_boundary);

#elif(4 == INCLUDE_BOUNDARY_TYPE)
    // 仅发送边界，不发送图像 (NULL)
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, NULL, MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL, NULL);
#endif
}

/**
 * @brief  WiFi 初始化全流程
 */
void wifi_init_all(void) {
    // 1. 连接 WiFi 热点
    while(wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST)) {
        printf("\r\n[WIFI] Connect to AP failed, retrying...");
        system_delay_ms(100);
    }

    // 打印模块基础信息
    printf("\r\n[WIFI] Module Version: %s", wifi_spi_version);
    printf("\r\n[WIFI] Module MAC    : %s", wifi_spi_mac_addr);
    printf("\r\n[WIFI] Module IP     : %s", wifi_spi_ip_addr_port);

    // 2. 建立 TCP Socket 连接 (如果没开启自动连接)
    if(1 != WIFI_SPI_AUTO_CONNECT) {
        while(wifi_spi_socket_connect("TCP", TCP_TARGET_IP, TCP_TARGET_PORT, WIFI_LOCAL_PORT)) {
            printf("\r\n[WIFI] TCP Connect Error, check Server IP/Port.");
            system_delay_ms(100);
        }
    }
    printf("\r\n[WIFI] TCP Connected successfully.");

    // 3. 配置上位机通信协议
    wifi_assistant_config();

    // 4. 【新增】初始化示波器通道名称和结构体
    oscilloscope_data.channel_num = 8;
    // 可以在这里给通道命名，例如:
    // memcpy(oscilloscope_data.channel_name[0], "Speed", 10);
}

/**
 * @brief  图像发送处理
 */
void wifi_send_process(void) {
    // 检查摄像头是否采集完成 (mt9v03x_finish_flag 是摄像头驱动中的全局变量)
    if(mt9v03x_finish_flag) {
        mt9v03x_finish_flag = 0;

        // 备份图像：防止发送过程中 DMA 修改缓冲区导致图像撕裂
        memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

        // 执行发送
        seekfree_assistant_camera_send();

        // 如果是 UDP 模式，建议取消下方注释强制推流
        // wifi_spi_udp_send_now();
    }
}

/**
 * @brief  【新增】数据交互函数：发送波形 + 接收参数 + Flash存储
 * @note   建议在 main 的 while(1) 中调用，或者定时调用
 */
void wifi_data_exchange(void)
{
    // 1. 如果需要读取 Flash 中的参数初始化 (通常在上电时做一次，这里为了搬运旧逻辑放在这里，但建议移到 Init 中)
    // 注意：频繁读取 Flash 影响效率，旧代码是每次调用都读吗？
    // 旧代码逻辑：flash_read_page_to_buffer 放在这里意味着它依赖 buffer 里的数据做比对？
    // 通常我们只在参数更新时写 Flash。
    
    // 发送示波器数据 (把你要看的数据赋值给 data[0]~[7])
    // 示例：把 target_v 放到通道0，err_degree 放到通道1
    oscilloscope_data.data[0] = target_v;
    oscilloscope_data.data[1] = err_degree; 
    oscilloscope_data.data[2] = (float)jump_tf; // 可以在示波器看这个标志位
    // ... 赋值其他通道
    
    seekfree_assistant_oscilloscope_send(&oscilloscope_data);

    // 2. 解析接收到的数据 (逐飞助手协议)
    seekfree_assistant_data_analysis();

    // 3. 检查是否有参数更新
    for(uint8_t i = 0; i < SEEKFREE_ASSISTANT_SET_PARAMETR_COUNT; i++)
    {
        if(seekfree_assistant_parameter_update_flag[i])
        {
            // 存入 Flash 缓冲 buffer
            flash_union_buffer[i].float_type  = seekfree_assistant_parameter[i];
            sum += seekfree_assistant_parameter_update_flag[i];
            
            // 清除标志位
            seekfree_assistant_parameter_update_flag[i] = 0;
            
            // 更新本地数组 data
            // 注意：这里 memcpy 可能会越界，如果 seekfree_assistant_parameter 比 data 大
            // 建议只拷贝前8个或者按需赋值
            // memcpy(data, seekfree_assistant_parameter, sizeof(data)); 
            
            // 【重点】参数映射逻辑
            // 必须确认上位机里发送的参数顺序和这里是一致的
            data[i] = seekfree_assistant_parameter[i]; // 简单赋值

            // 根据旧代码逻辑进行映射
            if(i == 0) target_v = data[0]; 
            if(i == 1) err_degree = data[1];
            if(i == 2) high = data[2];
            if(i == 3) rolling_target_degree = data[3];
            if(i == 4) jump_tf = (int)data[4]; // 假设参数4用来控制 jump_tf

            // 额外的逻辑判断
            if (target_v == 0)
            {
                run_flag = 0;
                high_flag = 0;
            }
            else
            {
                run_flag = 1;
                high_flag = 1;
            }
        }
    }

    // 4. 如果有参数变化，写入 Flash
    if(sum != 0)
    {
        flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
        sum = 0;
    }
    // flash_buffer_clear(); // 视具体库实现是否需要清除
}
