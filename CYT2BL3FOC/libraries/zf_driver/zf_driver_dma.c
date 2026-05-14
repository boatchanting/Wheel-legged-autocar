/*********************************************************************************************************************
* CYT2BL37CEE Opensourec Library 即（CYT2BL37CEE 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2023 SEEKFREE 逐飞科技
*
* 本文件是 CYT2BL37CEE 开源库的一部分
*
* CYT2BL37CEE 开源库 是免费软件
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
* 文件名称          zf_driver_dma
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MDK 5.38a
* 适用平台          CYT2BL37CEE
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2023-11-30       pudding            first version
********************************************************************************************************************/


#include "zf_common_debug.h"
#include "zf_driver_exti.h"
#include "zf_driver_dma.h"

//#define BUFFER_SIZE (32)
//#define PERIPHERAL_FIFO_ADDR (0x4001000)
//
//
//void dma_event_callback(void* callback_arg, cyhal_dma_event_t event);
//cyhal_dma_t     dma_obj;
//cyhal_source_t dma_source;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     dma 传输禁止
// 参数说明     ch              选择 dma 通道 (详见 zf_driver_dma.h 中枚举 dma_channel_enum 定义)
// 返回参数     void
// 使用示例     dma_disable();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_disable (void)
{
//	Cy_DMA_Channel_Disable(DW0, dma_obj.resource.channel_num); // 失能DMA模块																
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     dma 传输使能
// 参数说明     ch              选择 dma 通道 (详见 zf_driver_dma.h 中枚举 dma_channel_enum 定义)
// 返回参数     void
// 使用示例     dma_enable();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_enable (void)
{
//    Cy_DMA_Channel_Enable(DW1, dma_obj.resource.channel_num); // 使能DMA模块
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介      dma配置修改
// 参数说明      source_addr         设置源地址
// 参数说明      destination_addr    设置目的地址
// 参数说明      dma_count           设置dma搬移次数
// 返回参数      uint8
// 使用示例      dma_config(MT9V03X_DATA_ADD, mt9v03x_image[0]);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_config (uint8 *source_addr, uint8 *destination_addr)
{
//	dma_obj.descriptor.dw.dst = (uint32_t)destination_addr;
//	dma_obj.descriptor.dw.src = (uint32_t)source_addr;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介      dma初始化
// 参数说明      dma_ch              选择DMA通道
// 参数说明      source_addr         设置源地址
// 参数说明      destination_addr    设置目的地址
// 参数说明      exti_pin            设置触发的eru通道
// 参数说明      trigger             设置触发方式
// 参数说明      dma_count           设置dma搬移次数
// 返回参数      uint8
// 使用示例      dma_init(MT9V03X_DATA_ADD, mt9v03x_image[0], MT9V03X_PCLK_PIN, EXTI_TRIGGER_RISING, MT9V03X_IMAGE_SIZE);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void dma_init (uint8 *source_addr, uint8 *destination_addr, gpio_pin_enum exti_pin, exti_trigger_enum trigger, uint16 dma_count)
{
    
  
  
  
}



