/*********************************************************************************************************************
* 文件名称          wifi.h
* 功能说明          WiFi 通信模块封装头文件
* 备注信息          基于逐飞科技 CYT4BB 开源库
********************************************************************************************************************/
#ifndef _WIFI_H_
#define _WIFI_H_

#include "zf_common_headfile.h"

// ========================================== WiFi 基础配置 ==========================================
#define WIFI_SSID_TEST          "11111111"          // WiFi 名称
#define WIFI_PASSWORD_TEST      "00000000"          // WiFi 密码 (如果没有密码，请设置为 NULL)
#define TCP_TARGET_IP           "192.168.161.192"     // 【提醒】目标服务器（电脑）IP 地址，更改此处需要同步更改zf_device/zf_device_wifi_spi.h中对应行 WIFI_SPI_TARGET_IP 的定义，后面可以把这个功能合并到这里
#define TCP_TARGET_PORT         "8086"              // 目标服务器端口
#define WIFI_LOCAL_PORT         "6666"              // 本机端口 (2048-65535，0表示随机)

// ========================================== 图像/边界配置 ==========================================
// 0：仅图像 
// 1：图像+X边界 
// 2：图像+Y边界 
// 3：图像+XY边界(回弯) 
// 4：仅X边界
#define INCLUDE_BOUNDARY_TYPE   0

// 边界点数量定义 (用于回弯等复杂情况)
#define BOUNDARY_NUM            (MT9V03X_H * 3 / 2)

// ==================全局变量声明 (供 ISR 使用) ==================
extern int jump_tf;            // 弹跳调试标志位
extern float target_v;         // 目标速度
extern float err_degree;       // 角度误差
extern float rolling_target_degree; 
extern int run_flag;           // 运行标志
extern int high_flag;
extern float data[8];          // 参数数组
extern seekfree_assistant_oscilloscope_struct oscilloscope_data; // 示波器结构体

// ========================================== 函数接口 ==========================================

/**
 * @brief  WiFi 及上位机协议初始化封装
 * @return void
 */
void wifi_init_all(void);

/**
 * @brief  WiFi 图像发送处理函数
 * @note   在主循环中检测到摄像头采集完成时调用
 */
void wifi_send_process(void);

/**
 * @brief  WiFi 数据接收处理函数
 * @note   在主循环中持续调用，处理接收到的数据
 */
void wifi_data_exchange(void);

#endif /* _WIFI_H_ */