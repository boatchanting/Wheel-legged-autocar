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
* 文件名称          motor_flash
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-01-03       pudding            first version
********************************************************************************************************************/

#include "motor_flash.h"



//-------------------------------------------------------------------------------------------------------------------
// 函数简介     电机 FLASH 参数 读取
// 参数说明     void
// 返回参数     void
// 使用示例     motor_flash_read();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_flash_read(void)
{
    flash_read_page_to_buffer(MOTOR_FLASH_SECTOR, MOTOR_FLASH_PAGE, SAVE_FLASH_LENGTH);         // 读取 flash 页

    if(flash_union_buffer[SAVE_FLASH_LENGTH - 1].uint32_type != 0x4068)
    {
        motor_flash_write();
    }
    else
    {
         motor_left.zero_location                    =    flash_union_buffer[0].uint32_type ;  
         motor_left.rotation_direction               =    flash_union_buffer[1].uint32_type ;  
         motor_left.pole_pairs                       =    flash_union_buffer[2].uint32_type ;  
         motor_right.zero_location                   =    flash_union_buffer[3].uint32_type ;  
         motor_right.rotation_direction              =    flash_union_buffer[4].uint32_type ;  
         motor_right.pole_pairs                      =    flash_union_buffer[5].uint32_type ;  
    }

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     电机 FLASH 参数 写入
// 参数说明     void
// 返回参数     void
// 使用示例     motor_flash_write();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_flash_write(void)
{
    flash_union_buffer[0].uint32_type  = (uint32)motor_left.zero_location                ;  
    flash_union_buffer[1].uint32_type  = (uint32)motor_left.rotation_direction           ;  
    flash_union_buffer[2].uint32_type  = (uint32)motor_left.pole_pairs                   ;  
    flash_union_buffer[3].uint32_type  = (uint32)motor_right.zero_location               ;  
    flash_union_buffer[4].uint32_type  = (uint32)motor_right.rotation_direction          ;  
    flash_union_buffer[5].uint32_type  = (uint32)motor_right.pole_pairs                  ;  
    
    flash_union_buffer[SAVE_FLASH_LENGTH - 1].uint32_type = 0x4068;                             // flash 校验数据 若读取flash时校验位错误 则向flash写入默认参数
    
    flash_erase_page(MOTOR_FLASH_SECTOR, MOTOR_FLASH_PAGE);                                     // 擦除 flash 页

    flash_write_page_from_buffer(MOTOR_FLASH_SECTOR, MOTOR_FLASH_PAGE, SAVE_FLASH_LENGTH);      // 写入 flash 页
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     电机 FLASH 参数初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_flash_init();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_flash_init(void)
{    
    flash_init();
    
    // 未擦除过扇区就直接读取 会触发NOROOT的总线报错 该驱动程序库已特殊处理 不会卡死
    motor_flash_read();                                                                          // 读取 flash
}










