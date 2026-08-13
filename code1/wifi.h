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
* 文件名称          wifi.h
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
* 2026-1-21      boatchanting        拆分为模块
********************************************************************************************************************/

#ifndef __CODE1_WIFI_H__
#define __CODE1_WIFI_H__

#include "zf_common_headfile.h"

// 【新增】引入 PVC 视觉模块头文件
// 目的：为了获取 PVC_IMAGE_W (94) 和 PVC_IMAGE_H (60) 的宏定义，用于压缩数组的大小声明
#include "vision/pvc_vision.h"
#include "vision/bridge_v2_arbiter.h"
#include "vision/bridge_output_filter.h"
#include  "vision/bumpy_vision.h"
// *************************** 例程使用步骤说明 ***************************
// 1.根据硬件连接说明连接好模块，使用电源供电(下载器供电会导致模块电压不足)
//
// 2.查看电脑所连接的wifi，记录wifi名称，密码，IP地址
//
// 3.在下方的代码区域中修改宏定义，WIFI_SSID_TEST为wifi名称，WIFI_PASSWORD_TEST为wifi密码
//
// 4.打开zf_device_wifi_spi.h，修改WIFI_SPI_TARGET_IP宏定义，设置为电脑wifi的IP地址
//
// 5.下载例程到单片机中，打开逐飞助手上位机，打开下载器的串口
//
// 6.打开逐飞科技的逐飞助手软件，选择图像传输功能
//
// 7.选择网络，设置为TCP Server，本机地址中选择WIFI网络然后点击链接


// *************************** 例程测试说明 ***************************
// 1.本例程会通过 Debug 串口输出测试信息 请务必接好调试串口以便获取测试信息
//
// 2.连接好模块和核心板后（尽量使用配套主板测试以避免供电不足的问题） 烧录本例程 按下复位后程序开始运行
//
// 3.如果模块未能正常初始化 会通过 DEBUG 串口输出未能成功初始化的原因 随后程序会尝试重新初始化 一般情况下重试会成功
//
// 4.如果一直在 Debug 串口输出报错 就需要检查报错内容 并查看本文件下方的常见问题列表进行排查
//
// 5.程序默认不开启 WIFI_SPI_AUTO_CONNECT 宏定义 通过 main 函数中的接口建立网络链接 如果需要固定自行建立链接 可以开启该宏定义
//
// 6.当模块初始化完成后会通过 DEBUG 串口输出当前模块的主要信息：固件版本、IP信息、MAC信息、PORT信息
//
// 7.本例程是 TCP Client 例程 模块会被配置为 TCP Client 需要连接到局域网内的 TCP Server 才能进行通信
//   目标连接的 TCP Server 的 IP 与端口默认使用 zf_device_wifi_spi.h 中 WIFI_SPI_TARGET_IP 与 WIFI_SPI_TARGET_PORT 定义
//   实际测试需要根据现场 TCP Server 的实际 IP 地址与端口设置
//
// 8.当本机设备主动连接到 TCP Server （例如电脑使用逐飞助手上位机进入 TCP Server 模式 然后本机连接到电脑的 IP 与端口）
//   本例程会采集总钻风图像并发送到逐飞助手上位机
//
// 9.默认情况下逐飞助手显示摄像头的图像帧率可以达到50帧，如果无线网络比较复杂例如附近有较多的WIFI热点，可能会导致显示帧率较低
//
//
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查


// **************************** 代码区域 ****************************

//0：不包含边界信息
//1：包含三条边线信息，边线信息只包含横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点
//2：包含三条边线信息，边界信息只含有纵轴坐标，横轴坐标由图像宽度得到，意味着每个边界在一列中只会有一个点，一般来说很少有这样的使用需求
//3：包含三条边线信息，边界信息含有横纵轴坐标，意味着你可以指定每个点的横纵坐标，边线的数量也可以大于或者小于图像的高度，通常来说边线数量大于图像的高度，一般是搜线算法能找出回弯的情况
//4：没有图像信息，仅包含三条边线信息，边线信息只包含横轴坐标，纵轴坐标由图像高度得到，意味着每个边界在一行中只会有一个点，这样的方式可以极大的降低传输的数据量
#define INCLUDE_BOUNDARY_TYPE   0

// WIFI 图像发送源选择:
// 0: 保持原流程，压缩为 compressed_image_copy (94x60)，跑视觉算法/渲染后发送。
// 1: 直接发送 image_copy 原图 (188x120)，主循环跳过图像压缩和视觉算法。
#define WIFI_CAMERA_SEND_MODE_COMPRESSED   0
#define WIFI_CAMERA_SEND_MODE_RAW          1
#ifndef WIFI_CAMERA_SEND_MODE
#define WIFI_CAMERA_SEND_MODE              WIFI_CAMERA_SEND_MODE_COMPRESSED
#endif


#define WIFI_SSID_TEST          "11111111"
#define WIFI_PASSWORD_TEST      "00000000"                  // 如果需要连接的WIFI 没有密码则需要将 这里 替换为 NULL
#define TCP_TARGET_IP           "192.168.137.1"             // 连接目标的 IP
#define TCP_TARGET_PORT         "8086"                      // 连接目标的端口
#define WIFI_LOCAL_PORT         "6666"                      // 本机的端口 0：随机  可设置范围2048-65535  默认 6666

// 边界的点数量远大于图像高度，便于保存回弯的情况
#define BOUNDARY_NUM            (MT9V03X_H * 3 / 2)

//外部变量声明
// 【保持原有】原始图像备份(188x120)，防止撕裂，供其它算法使用

extern uint8 image_copy[MT9V03X_H][MT9V03X_W];
// 【新增】压缩后的图像备份数组(94x60)，供 PVC 算法、渲染画框及 WIFI 发送使用
extern uint8 compressed_image_copy[PVC_IMAGE_H][PVC_IMAGE_W];

#if (WIFI_CAMERA_SEND_MODE == WIFI_CAMERA_SEND_MODE_RAW)
#define WIFI_CAMERA_SEND_IMAGE_PTR     (image_copy[0])
#define WIFI_CAMERA_SEND_W             (MT9V03X_W)
#define WIFI_CAMERA_SEND_H             (MT9V03X_H)
#else
#define WIFI_CAMERA_SEND_IMAGE_PTR     (compressed_image_copy[0])
#define WIFI_CAMERA_SEND_W             (PVC_IMAGE_W)
#define WIFI_CAMERA_SEND_H             (PVC_IMAGE_H)
#endif

extern int g_motor_enable;

// 函数声明
void wifi_init(void);
void wifi_reconnect_tcp_server(void);
void wifi_connect_tcp_server(void);
void wifi_camera_init(void);
void wifi_update_pid_params(void);//检查并更新从上位机接收到的PID等参数
void encode_double_to_two_floats(double value, float* out_high, float* out_low);//辅助函数：安全地将 double 拆分为两个 float（作为位容器）

// 【新增】原图像降采样压缩函数声明 (188x120 -> 94x60)
// 请在主循环/摄像头中断里：memcpy将原图像拷贝到 image_copy 之后调用此函数。
void compress_image_to_target(void);

// 【修改注释】图像渲染辅助函数，用于将检测指标画在 compressed_image_copy 上以便上位机观察
#define VISION_IMAGE_RENDER_ENABLE          (1)
#define VISION_IMAGE_RENDER_NUMERIC_ENABLE  (1)

void draw_point_on_image(int x, int y, uint8 color);
void draw_rect_on_image(int x_min, int y_min, int x_max, int y_max, uint8 color);
void draw_cross_on_image(int x, int y, int size, uint8 color);

// 【修改注释】将 PVC 指标（BoundingBox，质心十字等）渲染在压缩后的备份图像(compressed_image_copy)上。
// 请在调用 compress_image_to_target 以及 pvc_vision_process_camera_frame 之后，WIFI发送之前调用。
void render_pvc_vision_to_image(void);
void render_bridge_vision_to_image(void);
void render_bumpy_vision_to_image(void);

#endif // __CODE1_WIFI_H__
