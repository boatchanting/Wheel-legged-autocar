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
* 文件名称          zf_driver_spi
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
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
    SPI_0,
    SPI_1,
    SPI_2,
    SPI_3,
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
    SPI0_CLK_P02_2,                		// SPI0 CLK 引脚可选范围

    SPI1_CLK_P12_2,             		// SPI1 CLK 引脚可选范围
	
    SPI2_CLK_P15_2,				// SPI2 CLK 引脚可选范围
    
    SPI3_CLK_P03_2,				// SPI3 CLK 引脚可选范围
}spi_clk_pin_enum;

typedef enum                       		// 枚举SPI MOSI引脚 此枚举定义不允许用户修改
{
    SPI0_MOSI_P02_1,                    	// SPI0 MOSI引脚可选范围

    SPI1_MOSI_P12_1,                  	        // SPI1 MOSI引脚可选范围
  
    SPI2_MOSI_P15_1,                  	        // SPI2 MOSI引脚可选范围
    
    SPI3_MOSI_P03_1,                  	        // SPI3 MOSI引脚可选范围
}spi_mosi_pin_enum;

typedef enum             			// 枚举SPI MISO引脚 此枚举定义不允许用户修改
{
    SPI0_MISO_P02_0,	                 	// SPI0 MISO引脚可选范围

    SPI1_MISO_P12_0,              		// SPI1 MISO引脚可选范围

    SPI2_MISO_P15_0, 				// SPI2 MISO引脚可选范围
    
    SPI3_MISO_P03_0, 				// SPI3 MISO引脚可选范围
	
    SPI_MISO_NULL,
}spi_miso_pin_enum;

typedef enum                       		// 枚举SPI CS引脚 此枚举定义不允许用户修改
{
    SPI0_CS0_P02_3,	        		// SPI0 CS0 引脚可选范围
    SPI0_CS1_P02_4,	
    
    SPI1_CS0_P12_3,	               		// SPI1 CS0 引脚可选范围
    SPI1_CS1_P12_4,	
            
    SPI2_CS0_P15_3,                 	        // SPI2 CS0 引脚可选范围
    SPI2_CS3_P05_1,
	
    SPI3_CS0_P03_3,                             // SPI3 CS0 引脚可选范围
    SPI3_CS1_P03_4,
    
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
