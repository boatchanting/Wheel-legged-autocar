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
* 文件名称          zf_driver_spi
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-9       pudding            first version
********************************************************************************************************************/
#ifndef _zf_driver_spi_h_
#define _zf_driver_spi_h_

#include "zf_common_typedef.h"

typedef enum        				// SPI模块号
{
    SPI_1,
    SPI_2,
    SPI_3,
    SPI_4,
}spi_index_enum;

typedef enum        				// 枚举 SPI 模式 此枚举定义不允许用户修改
{
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3,
}spi_mode_enum;

typedef enum                           	// 枚举SPI CLK引脚 此枚举定义不允许用户修改
{
    SPI1_CLK_P06_2,                		// SPI1 CLK 引脚
    SPI2_CLK_P07_2,             		// SPI2 CLK 引脚
    SPI3_CLK_P13_2,				// SPI3 CLK 引脚
    SPI4_CLK_P14_2,				// SPI4 CLK 引脚
}spi_clk_pin_enum;

typedef enum                       		// 枚举SPI MOSI引脚 此枚举定义不允许用户修改
{
    SPI1_MOSI_P06_1,                    	// SPI1 MOSI 引脚 
    SPI2_MOSI_P07_1,                  	        // SPI2 MOSI 引脚   
    SPI3_MOSI_P13_1,                  	        // SPI3 MOSI 引脚     
    SPI4_MOSI_P14_1,                  	        // SPI4 MOSI 引脚 
}spi_mosi_pin_enum;

typedef enum             			// 枚举SPI MISO引脚 此枚举定义不允许用户修改
{
    SPI1_MISO_P06_0,	                 	// SPI1 MISO 引脚
    SPI2_MISO_P07_0,              		// SPI2 MISO 引脚
    SPI3_MISO_P13_0, 				// SPI3 MISO 引脚    
    SPI4_MISO_P14_0, 				// SPI4 MISO 引脚	
    SPI_MISO_NULL,
}spi_miso_pin_enum;

typedef enum                       		// 枚举SPI CS引脚 此枚举定义不允许用户修改
{
    SPI1_CS0_P06_3,	        		// SPI1 CS 引脚
    SPI2_CS0_P11_0,	               		// SPI2 CS 引脚
    SPI3_CS0_P13_3,                 	        // SPI3 CS 引脚
    SPI4_CS0_P23_7,                             // SPI4 CS 引脚
    SPI_CS_NULL,
}spi_cs_pin_enum;

//====================================================SPI 基础函数====================================================
void        spi_write_8bit                  (spi_index_enum spi_n, const uint8 data);
void        spi_write_8bit_array            (spi_index_enum spi_n, const uint8 *data, uint32 len);

void        spi_write_16bit                 (spi_index_enum spi_n, const uint16 data);
void        spi_write_16bit_array           (spi_index_enum spi_n, const uint16 *data, uint32 len);

void        spi_write_8bit_register         (spi_index_enum spi_n, const uint8 register_name, const uint8 data);
void        spi_write_8bit_registers        (spi_index_enum spi_n, const uint8 register_name, const uint8 *data, uint32 len);

void        spi_write_16bit_register        (spi_index_enum spi_n, const uint16 register_name, const uint16 data);
void        spi_write_16bit_registers       (spi_index_enum spi_n, const uint16 register_name, const uint16 *data, uint32 len);

uint8       spi_read_8bit                   (spi_index_enum spi_n);
void        spi_read_8bit_array             (spi_index_enum spi_n, uint8 *data, uint32 len);

uint16      spi_read_16bit                  (spi_index_enum spi_n);
void        spi_read_16bit_array            (spi_index_enum spi_n, uint16 *data, uint32 len);

uint8       spi_read_8bit_register          (spi_index_enum spi_n, const uint8 register_name);
void        spi_read_8bit_registers         (spi_index_enum spi_n, const uint8 register_name, uint8 *data, uint32 len);

uint16      spi_read_16bit_register         (spi_index_enum spi_n, const uint16 register_name);
void        spi_read_16bit_registers        (spi_index_enum spi_n, const uint16 register_name, uint16 *data, uint32 len);

void        spi_transfer_8bit               (spi_index_enum spi_n, const uint8 *write_buffer, uint8 *read_buffer, uint32 len);
void        spi_transfer_16bit              (spi_index_enum spi_n, const uint16 *write_buffer, uint16 *read_buffer, uint32 len);

void        spi_init                        (spi_index_enum spi_n, spi_mode_enum mode, uint32 baud, spi_clk_pin_enum sck_pin, spi_mosi_pin_enum mosi_pin, spi_miso_pin_enum miso_pin, spi_cs_pin_enum cs_pin);
//====================================================SPI 基础函数====================================================

#endif
