/*********************************************************************************************************************
* 文件名称          wifi.c
* 功能说明          WiFi 通信模块封装实现
* 备注信息          包含 WiFi 连接、TCP Socket 建立及逐飞助手协议配置
********************************************************************************************************************/
#include "wifi.h"

// ------------------------------------------ 静态全局变量 ------------------------------------------

// 边界数组定义
static uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
static uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];
static uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];
static uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];

// 图像备份数组：用于解决 DMA 传输与 WiFi 发送不同步导致的“图像撕裂”问题
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
}

/**
 * @brief  数据发送处理
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