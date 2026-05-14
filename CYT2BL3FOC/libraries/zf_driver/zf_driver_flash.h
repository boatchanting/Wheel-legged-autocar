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
* 文件名称          zf_driver_flash
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-8       pudding            first version
********************************************************************************************************************/

#ifndef _zf_driver_flash_h_
#define _zf_driver_flash_h_

#include "flash/cy_mw_flash.h"
#include "flash/cy_flash.h"
#include "zf_common_typedef.h"

#define FLASH_BASE_ADDR             (CY_WFLASH_LG_SBM_TOP)			                // flash 起始地址
#define FLASH_SIZE                  (CY_WFLASH_LG_SBM_SIZE)            				// 一共有 96KB
#define FLASH_PAGE_SIZE             (CY_WORK_LES_SIZE_IN_BYTE)					// 每页大小为 2kb 
#define FLASH_PAGE_NUM              (CY_WFLASH_LG_SBM_SIZE / FLASH_PAGE_SIZE)   	        // 96KB 分为了 48 页

#define FLASH_DATA_SIZE             (4)							        // 数据大小为 4 byte
#define FLASH_PAGE_LENGTH           (FLASH_PAGE_SIZE / 4)         				// 每页可以存 512 个 uint32 类型的数据


typedef union                                                                   		// 固定的数据缓冲单元格式
{
    float   float_type;                                                         		// float  类型
    uint32  uint32_type;                                                        		// uint32 类型
    int32   int32_type;                                                         		// int32  类型
    uint16  uint16_type;                                                        		// uint16 类型
    int16   int16_type;                                                         		// int16  类型
    uint8   uint8_type;                                                         		// uint8  类型
    int8    int8_type;                                                          		// int8   类型
}flash_data_union;                                                              		// 所有类型数据共用同一个 32bit 地址

extern vuint8 flash_write_state;                                                                // flash 写入标志位

extern flash_data_union flash_union_buffer[FLASH_PAGE_LENGTH];                                  // flash缓冲区


//====================================================FLASH 基础函数====================================================
uint8   flash_check                     (uint32 sector_num, uint32 page_num);                                    // 校验FLASH页是否有数据
void    flash_erase_page                (uint32 sector_num, uint32 page_num);                                   // 擦除页
void    flash_read_page                 (uint32 sector_num, uint32 page_num, uint32 *buf, uint32 len);          // 读取一页
void    flash_write_page                (uint32 sector_num, uint32 page_num, const uint32 *buf, uint32 len);   // 编程一页
void    flash_read_page_to_buffer       (uint32 sector_num, uint32 page_num, uint32 len);                       // 从指定 FLASH 的指定页码读取数据到缓冲区
uint8   flash_write_page_from_buffer    (uint32 sector_num, uint32 page_num, uint32 len);                       // 向指定 FLASH 的扇区的指定页码写入缓冲区的数据
void    flash_buffer_clear              (void);                                                                // 清空数据缓冲区
void 	flash_init 						(void);						// flash初始化
//====================================================FALSH 基础函数====================================================


#endif
