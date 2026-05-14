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


#include "zf_common_debug.h"
#include "zf_driver_flash.h"


flash_data_union flash_union_buffer[FLASH_PAGE_LENGTH];                    // FLASH 操作的数据缓冲区
static vuint8 flash_init_flag = 0;
vuint8 flash_write_state = 0;


//-------------------------------------------------------------------------------------------------------------------
//  函数简介      校验FLASH页是否有数据
//  参数说明      sector_num    仅可填写0  此处扇区编号并无实际作用，只是留出接口
//  参数说明      page_num      需要写入的页编号   参数范围 <0 - 47>
//  返回参数      返回1有数据，返回0没有数据，如果需要对有数据的区域写入新的数据则应该对所在扇区进行擦除操作
//  使用示例      flash_check(0, 0); 	// 校验0页是否有数据
//  备注信息			
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_check (uint32 sector_num, uint32 page_num)
{
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错

    return !Cy_WorkFlashBlankCheck((page_num * FLASH_PAGE_SIZE + FLASH_BASE_ADDR), CY_FLASH_DRIVER_BLOCKING);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      擦除页
//  参数说明      sector_num    仅可填写0  此处扇区编号并无实际作用，只是留出接口
//  参数说明      page_num      需要写入的页编号   参数范围 <0 - 47>
//  返回参数      void
//  使用示例      flash_erase_page(0, 0);
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void flash_erase_page (uint32 sector_num, uint32 page_num)
{
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错

    Cy_FlashSectorErase((page_num * FLASH_PAGE_SIZE + FLASH_BASE_ADDR), CY_FLASH_DRIVER_BLOCKING);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     读取一页
// 参数说明     sector_num      仅可填写0  此处扇区编号并无实际作用，只是留出接口
// 参数说明     page_num        当前扇区页的编号   参数范围 <0 - 47>
// 参数说明     buf             需要读取的数据地址   传入的数组类型必须为uint32
// 参数说明     len             需要写入的数据长度   参数范围 1 - 511
// 返回参数     void
// 使用示例     flash_read_page(0, 0, data_buffer, 256);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void flash_read_page(uint32 sector_num, uint32 page_num, uint32 *buf, uint32 len)
{
    uint32 data_cont = 0;
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(FLASH_PAGE_LENGTH >= len);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错
    
    uint32 * flash_addr = (uint32 *)FLASH_BASE_ADDR;
    flash_addr = flash_addr + (page_num * FLASH_PAGE_SIZE / FLASH_DATA_SIZE);
    
    flash_write_state = 1;
    
    for(data_cont = 0; data_cont < len; data_cont ++)
    {
        *buf ++ = *(flash_addr + (data_cont * FLASH_DATA_SIZE));
        if(flash_write_state == 2)                      // 首次读取flash触发报错故障 直接退出读取
        {
            break;
        }
    }
    
    flash_write_state = 0;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      编程一页
//  参数说明      sector_num      仅可填写0  此处扇区编号并无实际作用，只是留出接口
//  参数说明      page_num        当前扇区页的编号    参数范围 <0 - 47>
//  参数说明      buf             需要写入的数据地址   传入的数组类型必须为 uint32
//  参数说明      len             需要写入的数据长度   参数范围 1 - 511
//  返回参数      void
//  使用示例      flash_write_page(0, 0, buf, 10);
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void flash_write_page (uint32 sector_num, uint32 page_num, const uint32 *buf, uint32 len)
{
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(FLASH_PAGE_LENGTH >= len);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错
    
    if(0 != flash_check(sector_num, page_num))
    {
        flash_erase_page(sector_num, page_num);
    }
    uint32 flash_addr = page_num * FLASH_PAGE_SIZE + FLASH_BASE_ADDR;
    
    for(int i = 0; i < len; i ++)
    {
      
        Cy_FlashWriteWork(flash_addr, buf, CY_FLASH_DRIVER_BLOCKING);
        flash_addr += 4;
        buf += 1;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     从指定 FLASH 的扇区的指定页码读取数据到缓冲区
// 参数说明     sector_num      仅可填写0  此处扇区编号并无实际作用，只是留出接口
// 参数说明     page_num        当前扇区页的编号   参数范围 <0 - 47>
// 返回参数     void
// 使用示例     flash_read_page_to_buffer(0, 0, 1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void flash_read_page_to_buffer (uint32 sector_num, uint32 page_num, uint32 len)
{
    uint32 data_cont = 0;
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(FLASH_PAGE_LENGTH >= len);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错
	
    uint32 * flash_addr = (uint32 *)FLASH_BASE_ADDR;
    flash_addr = flash_addr + (page_num * FLASH_PAGE_SIZE / FLASH_DATA_SIZE);

    flash_write_state = 1;
    
    for(data_cont = 0; data_cont < len; data_cont ++)
    {
        flash_union_buffer[data_cont].uint32_type = flash_addr[data_cont];
        if(flash_write_state == 2)                      // 首次读取flash触发报错故障 直接退出读取
        {
            break;
        }
    }
    
    flash_write_state = 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     向指定 FLASH 的扇区的指定页码写入缓冲区的数据
// 参数说明     sector_num      仅可填写0  此处扇区编号并无实际作用，只是留出接口
// 参数说明     page_num        当前扇区页的编号   参数范围 <0 - 47>
// 返回参数     uint8           1-表示失败 0-表示成功
// 使用示例     flash_write_page_from_buffer(0, 0, 1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 flash_write_page_from_buffer (uint32 sector_num, uint32 page_num, uint32 len)
{
    zf_assert(FLASH_PAGE_NUM > page_num);
    zf_assert(FLASH_PAGE_LENGTH >= len);
    zf_assert(flash_init_flag);				// 用户未初始化flash则断言报错
	
    uint32 *data_pointer = (uint32 *)flash_union_buffer;

    flash_write_page(0, page_num, data_pointer, len);
    
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清空数据缓冲区
// 参数说明     void
// 返回参数     void
// 使用示例     flash_buffer_clear();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void flash_buffer_clear (void)
{
    memset(flash_union_buffer, 0xFF, FLASH_PAGE_LENGTH);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     flash初始化
// 参数说明     void
// 返回参数     void
// 使用示例     flash_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void flash_init (void)
{
    Cy_FlashInit(false);
    flash_init_flag = 1;
}