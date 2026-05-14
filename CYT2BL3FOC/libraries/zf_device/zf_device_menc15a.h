/*********************************************************************************************************************
* CYT2BL3 Opensourec Library 即（ CYT2BL3 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT2BL3 开源库的一部分
*
* CYT2BL3 开源库 是免费软件
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
* 文件名称          zf_device_menc15a
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-11-19       pudding            first version
********************************************************************************************************************/
/*********************************************************************************************************************
* 接线定义：
*                   ------------------------------------
*                   模块管脚            单片机管脚
*                   // 硬件 SPI 引脚
*                   CLK               查看 zf_device_menc15a.h 中 MENC15A_CLK_PIN   宏定义
*                   MOSI              查看 zf_device_menc15a.h 中 MENC15A_MOSI_PIN  宏定义
*                   MISO              查看 zf_device_menc15a.h 中 MENC15A_MISO_PIN  宏定义
*                   CS                查看 zf_device_menc15a.h 中 MENC15A_CS_PIN    宏定义
*                   VCC               3.3V电源
*                   GND               电源地
*                   其余引脚悬空
*                   ------------------------------------
********************************************************************************************************************/


#ifndef _zf_device_menc15a_h_
#define _zf_device_menc15a_h_

#include "zf_common_typedef.h"

//================================================定义 MENC15A 基本配置================================================

//====================================================硬件 SPI 驱动====================================================
#define MENC15A_1_SPI_SPEED            (20 * 1000 * 1000)                        // 磁编码器1 硬件 SPI 速率
#define MENC15A_1_SPI                  (SPI_1           )                        // 磁编码器1 硬件 SPI 号
#define MENC15A_1_CLK_PIN              (SPI1_CLK_P06_2  )                        // 磁编码器1 硬件 SPI SCK 引脚
#define MENC15A_1_MOSI_PIN             (SPI1_MOSI_P06_1 )                        // 磁编码器1 硬件 SPI MOSI 引脚
#define MENC15A_1_MISO_PIN             (SPI1_MISO_P06_0 )                        // 磁编码器1 硬件 SPI MISO 引脚
#define MENC15A_1_CS_PIN               (P06_3)                                   // 磁编码器1 CS 片选引脚
#define MENC15A_1_CS(x)                ((x) ? (gpio_high(MENC15A_1_CS_PIN)) : (gpio_low(MENC15A_1_CS_PIN)))


#define MENC15A_2_SPI_SPEED           (20 * 1000 * 1000)                        // 磁编码器2 硬件 SPI 速率
#define MENC15A_2_SPI                 (SPI_3           )                        // 磁编码器2 硬件 SPI 号
#define MENC15A_2_CLK_PIN             (SPI3_CLK_P13_2  )                        // 磁编码器2 硬件 SPI SCK 引脚
#define MENC15A_2_MOSI_PIN            (SPI3_MOSI_P13_1 )                        // 磁编码器2 硬件 SPI MOSI 引脚
#define MENC15A_2_MISO_PIN            (SPI3_MISO_P13_0 )                        // 磁编码器2 硬件 SPI MISO 引脚
#define MENC15A_2_CS_PIN              (P13_3)                                   // 磁编码器2 CS 片选引脚
#define MENC15A_2_CS(x)               ((x) ? (gpio_high(MENC15A_2_CS_PIN)) : (gpio_low(MENC15A_2_CS_PIN)))
//====================================================硬件 SPI 驱动====================================================

// 枚举磁编码模块
typedef enum 
{
	menc15a_1_module = 0,
        menc15a_2_module = 1,
        
}menc15a_module_enum;

//================================================定义 MENC15A 基本配置================================================


extern uint16 menc15a_absolute_data[2];

extern int16 menc15a_absolute_offset_data[2];

extern int16  menc15a_speed_data[2];


uint16 menc15a_get_absolute_data(menc15a_module_enum menc15a_module);
int16  menc15a_get_speed_data(menc15a_module_enum menc15a_module);

uint8  menc15a_init(void);


#endif
