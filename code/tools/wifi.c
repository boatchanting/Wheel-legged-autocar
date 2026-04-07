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
uint8 image_copy[MT9V03X_H][MT9V03X_W];

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
    // 发送总钻风图像信息(仅包含原始图像信息)
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);


#elif(1 == INCLUDE_BOUNDARY_TYPE)
    // 发送总钻风图像信息(并且包含三条边界信息，边界信息只含有横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点)
    // 对边界数组写入数据
    for(int i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
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
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
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
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
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
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, NULL, MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL ,NULL);


#endif
}


// =================================================================================
// ★★★ 上位机参数调节映射 (PC -> MCU) ★★★
// =================================================================================
// 参数 0: 角速度环 Kp (pid_gyro.kp)
// 参数 1: 角速度环 Kd (pid_gyro.kd)
// 参数 2: 角度环   Kp (pid_angle.kp)
// 参数 3: 角度环   Kd (pid_angle.kd)
// 参数 4: 速度环   Kp (pid_speed.kp)
// 参数 5: 速度环   Ki (pid_speed.ki)
// 参数 6: 期望速度 (target_speed_set)
// 参数 7: 电机使能 (g_motor_enable) -> 1.0f为使能, 0.0f为失能
/**
 * @brief  检查并更新从上位机接收到的PID等参数
 * @param  void
 * @return void
 * @note   此函数应在主循环中被周期性调用
 */
void wifi_update_pid_params(void)
{
    // 调用库函数，解析WiFi数据流
    seekfree_assistant_data_analysis();

    // 遍历所有参数，检查是否有更新标志
    for(uint8_t i = 0; i < SEEKFREE_ASSISTANT_SET_PARAMETR_COUNT; i++)
    {
        if(seekfree_assistant_parameter_update_flag[i])
        {
            // 清除更新标志，防止重复执行
            seekfree_assistant_parameter_update_flag[i] = 0;

            // 根据参数索引(i)更新对应的全局变量
            // 这个映射关系需要在逐飞助手软件上对应设置
            switch(i)
            {
                // //【调节直立环】
                // // 参数 0: 角速度环 Kp (pid_gyro.kp)
                // case 0: pid_gyro.kp  = seekfree_assistant_parameter[i]; break;
                // // 参数 1: 角度环Kp
                // case 1: pid_angle.kp  = seekfree_assistant_parameter[i]; break;
                // // 参数 2: 角度环kd
                // case 2: pid_angle.kd  = seekfree_assistant_parameter[i]; break;
                // // 参数 3: 舵机速度环kp 
                // case 3: pid_servo_speed.kp = seekfree_assistant_parameter[i]; break;
                // // 参数 4: 舵机速度环ki 
                // case 4: pid_servo_speed.ki = seekfree_assistant_parameter[i]; break;
                // // 参数 5: 跳跃 
                // case 5: vision_detected_jump_point = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                // // 参数 6: 机械零点
                // case 6:pid_angle.compensation = seekfree_assistant_parameter[i]; break;
                // // 参数 7: 电机使能 (1.0f为使能, 0.0f为失能)
                // case 7: g_motor_enable = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                // default: break;
                                
                ////【调节转向环】
                // 注意调试的时候需要将isr中 调节pid转向角度环时使用【调试pid打开】
                // // 参数 0: 转向角速度环 Kp 
                // case 0:pid_turn_gyro.kp  = seekfree_assistant_parameter[i]; break;
                // // 参数 1: 转向角速度环kd
                // case 1: pid_turn_gyro.kd = seekfree_assistant_parameter[i]; break;
                // // 参数 2: 转向角度环kp
                // case 2:  pid_turn_angle.kp = seekfree_assistant_parameter[i]; break;
                // // 参数 3:  
                // case 3: vision_detected_marker = seekfree_assistant_parameter[i]; break;
                // // 参数 4: 
                // case 4: pid_servo_speed.ki = seekfree_assistant_parameter[i]; break;
                // // 参数 5: 跳跃 
                // case 5: vision_detected_jump_point = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                // // 参数 6: 旋转
                // case 6:vision_detected_marker = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                // // 参数 7: 电机使能 (1.0f为使能, 0.0f为失能)
                // case 7: g_motor_enable = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                // default: break;

                ////【调节跳跃参数】
                case 0:g_jump_profile.t_launch  = seekfree_assistant_parameter[i]; break;
                case 1:g_jump_profile.t_flight = seekfree_assistant_parameter[i]; break;
                case 2:g_jump_profile.t_landing= seekfree_assistant_parameter[i]; break;
                case 3: g_jump_profile.t_recovery = seekfree_assistant_parameter[i]; break;
                case 4: g_jump_profile.offset_launch = seekfree_assistant_parameter[i]; break;
                case 5: g_jump_profile.offset_flight = seekfree_assistant_parameter[i]; break;
                case 6: g_jump_profile.offset_land  =  seekfree_assistant_parameter[i]; break;
                case 7:vision_detected_jump_point = (seekfree_assistant_parameter[i] > 0.5f) ? 1 : 0; break;
                default: break;
  
            }
        }
    }
}

// 辅助函数：安全地将 double 拆分为两个 float（作为位容器）
// 逐飞助手传输的是 float 型数据，如果想要传输 double 型数据，需要将其拆分为两个 float，请直接调用该函数，示例如下
// encode_double_to_two_floats(gnss.latitude,
//                            &seekfree_assistant_oscilloscope_data.data[1], // 纬度高32位 → 通道1
//                            &seekfree_assistant_oscilloscope_data.data[2]); // 纬度低32位 → 通道2
void encode_double_to_two_floats(double value, float* out_high, float* out_low) {
    uint64_t u64;
    memcpy(&u64, &value, sizeof(double));          // 获取 double 的 64 位表示

    uint32_t high = (uint32_t)(u64 >> 32);         // 高 32 位
    uint32_t low  = (uint32_t)(u64 & 0xFFFFFFFFU); // 低 32 位

    memcpy(out_high, &high, sizeof(uint32_t));     // 将 high 的位模式写入 float
    memcpy(out_low,  &low,  sizeof(uint32_t));     // 将 low 的位模式写入 float
}