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
* 2024-3-6       pudding            修复与串口的时钟冲突问题
********************************************************************************************************************/

#include "scb/cy_scb_spi.h"
#include "sysclk/cy_sysclk.h"
#include "gpio/cy_gpio.h"
#include "zf_common_debug.h"
#include "zf_driver_gpio.h"
#include "zf_driver_delay.h"
#include "zf_driver_spi.h"


#define SPI_FREQ       CY_INITIAL_TARGET_PERI_FREQ                             // 串口模块时钟 默认80M

volatile stc_SCB_t*        spi_module[4] = {SCB4, SCB5, SCB3, SCB2};
spi_cs_pin_enum             cs_pin_save[4];
//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取时钟引脚号
// 参数说明       clk_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_clk_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI0_CLK_P02_2);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum spi_get_clk_pin (spi_clk_pin_enum clk_pin)
{
    gpio_pin_enum temp_clk_pin = NC;
    
    switch(clk_pin)
    {
        case SPI1_CLK_P06_2: temp_clk_pin = P06_2; break;
        case SPI2_CLK_P07_2: temp_clk_pin = P07_2; break;
        case SPI3_CLK_P13_2: temp_clk_pin = P13_2; break;
        case SPI4_CLK_P14_2: temp_clk_pin = P14_2; break;
    }
    
    return temp_clk_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取MOSI引脚号
// 参数说明       mosi_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_mosi_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_mosi_pin(SPI0_MOSI_P02_1);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum spi_get_mosi_pin (spi_mosi_pin_enum mosi_pin)
{
    gpio_pin_enum temp_mosi_pin = NC;
    
    switch(mosi_pin)
    {
        case SPI1_MOSI_P06_1: temp_mosi_pin = P06_1; break;
        case SPI2_MOSI_P07_1: temp_mosi_pin = P07_1; break;
        case SPI3_MOSI_P13_1: temp_mosi_pin = P13_1; break;
        case SPI4_MOSI_P14_1: temp_mosi_pin = P14_1; break;
    }
    
    return temp_mosi_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取MISO引脚号
// 参数说明       miso_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_miso_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_miso_pin(SPI0_MISO_P02_0);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum spi_get_miso_pin (spi_miso_pin_enum miso_pin)
{
    gpio_pin_enum temp_miso_pin = NC;
    
    switch(miso_pin)
    {
        case SPI1_MISO_P06_0: temp_miso_pin = P06_0; break;
        case SPI2_MISO_P07_0: temp_miso_pin = P07_0; break;
        case SPI3_MISO_P13_0: temp_miso_pin = P13_0; break;
        case SPI4_MISO_P14_0: temp_miso_pin = P14_0; break;
        case SPI_MISO_NULL: break;
    }
    
    return temp_miso_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取CS引脚号
// 参数说明       clk_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_clk_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI0_CS0_P02_3);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum spi_get_cs_pin (spi_cs_pin_enum cs_pin)
{
    gpio_pin_enum temp_cs_pin = NC;
    switch(cs_pin)
    {
        case SPI1_CS0_P06_3: temp_cs_pin = P06_3; break;
        case SPI2_CS0_P11_0: temp_cs_pin = P11_0; break;
        case SPI3_CS0_P13_3: temp_cs_pin = P13_3; break;
        case SPI4_CS0_P23_7: temp_cs_pin = P23_7; break;
        case SPI_CS_NULL: break;
    }
    return temp_cs_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取时钟引脚复用关系
// 参数说明       clk_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_clk_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI0_CLK_P02_2);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t spi_get_clk_hsiom (spi_clk_pin_enum clk_pin)
{
    en_hsiom_sel_t temp_clk_hsiom = HSIOM_SEL_GPIO;
    
    switch(clk_pin)
    {
        case SPI1_CLK_P06_2: temp_clk_hsiom = P6_2_SCB4_SPI_CLK; break;     
        case SPI2_CLK_P07_2: temp_clk_hsiom = P7_2_SCB5_SPI_CLK; break;    
        case SPI3_CLK_P13_2: temp_clk_hsiom = P13_2_SCB3_SPI_CLK; break;    
        case SPI4_CLK_P14_2: temp_clk_hsiom = P14_2_SCB2_SPI_CLK; break;    
    }
    
    return temp_clk_hsiom;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取MOSI引脚复用关系
// 参数说明       clk_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_mosi_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_mosi_pin(SPI0_MOSI_P02_1);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t spi_get_mosi_hsiom (spi_mosi_pin_enum mosi_pin)
{
    en_hsiom_sel_t temp_mosi_hsiom = HSIOM_SEL_GPIO;
    
    switch(mosi_pin)
    {
        case SPI1_MOSI_P06_1: temp_mosi_hsiom =  P6_1_SCB4_SPI_MOSI; break;      
        case SPI2_MOSI_P07_1: temp_mosi_hsiom =  P7_1_SCB5_SPI_MOSI; break;     
        case SPI3_MOSI_P13_1: temp_mosi_hsiom =  P13_1_SCB3_SPI_MOSI; break;     
        case SPI4_MOSI_P14_1: temp_mosi_hsiom =  P14_1_SCB2_SPI_MOSI; break;     
    }
    
    return temp_mosi_hsiom;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取MISO引脚复用关系
// 参数说明       miso_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_miso_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_miso_pin(SPI0_MISO_P02_0);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t spi_get_miso_hsiom (spi_miso_pin_enum miso_pin)
{
    en_hsiom_sel_t temp_miso_hsiom = HSIOM_SEL_GPIO;
    
    switch(miso_pin)
    {
        case SPI1_MISO_P06_0: temp_miso_hsiom =  P6_0_SCB4_SPI_MISO; break;       
        case SPI2_MISO_P07_0: temp_miso_hsiom =  P7_0_SCB5_SPI_MISO; break;     
        case SPI3_MISO_P13_0: temp_miso_hsiom =  P13_0_SCB3_SPI_MISO; break;  
        case SPI4_MISO_P14_0: temp_miso_hsiom =  P14_0_SCB2_SPI_MISO; break; 
        case SPI_MISO_NULL: break;
    }
    
    return temp_miso_hsiom;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取CS引脚复用关系
// 参数说明       cs_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_clk_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI0_CS0_P02_3);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t spi_get_cs_hsiom (spi_cs_pin_enum cs_pin)
{
    en_hsiom_sel_t temp_cs_hsiom = HSIOM_SEL_GPIO;
    switch(cs_pin)
    {
        case SPI1_CS0_P06_3: temp_cs_hsiom =  P6_3_SCB4_SPI_SELECT0;  break;     
        case SPI2_CS0_P11_0: temp_cs_hsiom =  P11_0_GPIO;  break;      
        case SPI3_CS0_P13_3: temp_cs_hsiom =  P13_3_SCB3_SPI_SELECT0; break;     
        case SPI4_CS0_P23_7: temp_cs_hsiom =  P23_7_GPIO; break; 
        case SPI_CS_NULL: break;
    }
    return temp_cs_hsiom;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI获取CS引脚复用关系
// 参数说明       cs_pin     时钟引脚 参照 zf_driver_spi.h 内 spi_clk_pin_enum 枚举体定义
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI0_CS0_P02_3);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static en_clk_dst_t spi_get_module_clock (spi_index_enum spi_n)
{
    en_clk_dst_t temp_spi_module;
    switch(spi_n)
    {
        case SPI_1: temp_spi_module =  PCLK_SCB4_CLOCK;  break;     
        case SPI_2: temp_spi_module =  PCLK_SCB5_CLOCK;  break;      
        case SPI_3: temp_spi_module =  PCLK_SCB3_CLOCK; break;     
        case SPI_4: temp_spi_module =  PCLK_SCB2_CLOCK; break; 
    }
    return temp_spi_module;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI配置发送长度
// 参数说明       spi_n     SPI模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       length		需要配置的长度 仅支持 8 16
// 返回参数       void
// 使用示例       spi_get_clk_pin(SPI_0, 8);
// 备注信息       内部调用，用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static void switch_transition_length(spi_index_enum spi_n, uint8 length)
{
  static uint8 now_length[SPI_4 + 1] = {0};
  
    if(now_length[spi_n] != length)
    {
        switch(length)
        {
            case 8:
            {
                spi_module[spi_n]->unCTRL.u32Register &= 0xffff3fff;
                spi_module[spi_n]->unTX_CTRL.u32Register &= 0xffffffe0;
                spi_module[spi_n]->unTX_CTRL.u32Register |= 0x00000007;
                spi_module[spi_n]->unRX_CTRL.u32Register &= 0xffffffe0;
                spi_module[spi_n]->unRX_CTRL.u32Register |= 0x00000007;
            }break;
            case 16:
            {
                spi_module[spi_n]->unCTRL.u32Register &= 0xffff3fff;
                spi_module[spi_n]->unCTRL.u32Register |= 0x00004000;
                spi_module[spi_n]->unTX_CTRL.u32Register &= 0xffffffe0;
                spi_module[spi_n]->unTX_CTRL.u32Register |= 0x0000000F;
                spi_module[spi_n]->unRX_CTRL.u32Register &= 0xffffffe0;
                spi_module[spi_n]->unRX_CTRL.u32Register |= 0x0000000F;
            }break;
        }
        now_length[spi_n] = length;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口写 8bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       data            数据
// 返回参数       void
// 使用示例       spi_write_8bit(SPI_0, 0x11);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit (spi_index_enum spi_n, const uint8 data)
{
    switch_transition_length(spi_n, 8);					        // 切换通信长度为8位
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], data);                                // 发送数据
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口写 8bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_write_8bit_array(SPI_0, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_array (spi_index_enum spi_n, const uint8 *data, uint32 len)
{    
    switch_transition_length(spi_n, 8);					        // 切换通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    } 
    
    do
    {
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *data ++);                        // 发送数据
        while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
        len -= 1;						                // 发送长度自减	
    }while(len);
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)				        // 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口写 16bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       data            数据
// 返回参数       void
// 使用示例       spi_write_16bit(SPI_0, 0x1101);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit (spi_index_enum spi_n, const uint16 data)
{
    switch_transition_length(spi_n, 16);				        // 切换通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], data);                                // 发送数据
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }		
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口写 16bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_write_16bit_array(SPI_0, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_array (spi_index_enum spi_n, const uint16 *data, uint32 len)
{
    switch_transition_length(spi_n, 16);					// 切换通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    } 
    
    do
    {
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *data ++);                        // 发送数据
        while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
        len -= 1;						                // 发送长度自减	
    }while(len);
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)				        // 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口向传感器的寄存器写 8bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       data            数据
// 返回参数       void
// 使用示例        spi_write_8bit_register(SPI_0, 0x11, 0x01);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_register (spi_index_enum spi_n, const uint8 register_name, const uint8 data)
{
    switch_transition_length(spi_n, 8);						// 切换通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                        // 发送寄存器地址
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待

    Cy_SCB_WriteTxFifo(spi_module[spi_n], data);                                // 发送数据
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
            gpio_high(cs_pin_save[spi_n]);
    }	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口向传感器的寄存器写 8bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_write_8bit_registers(SPI_0, 0x11, data, 32);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_8bit_registers (spi_index_enum spi_n, const uint8 register_name, const uint8 *data, uint32 len)
{
    switch_transition_length(spi_n, 8);					        // 切换通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    } 
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                        // 发送寄存器地址
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待
    
    do
    {
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *data ++);                        // 发送数据
        while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
        len -= 1;						                // 发送长度自减	
    }while(len);
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)				        // 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口向传感器的寄存器写 16bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       data            数据
// 返回参数       void
// 使用示例       spi_write_16bit_register(SPI_0, 0x1011, 0x0101);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_register (spi_index_enum spi_n, const uint16 register_name, const uint16 data)
{
    switch_transition_length(spi_n, 16);				        // 切换通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                        // 发送寄存器地址
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待

    Cy_SCB_WriteTxFifo(spi_module[spi_n], data);                                // 发送数据
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
            gpio_high(cs_pin_save[spi_n]);
    }	
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口向传感器的寄存器写 16bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_write_16bit_registers(SPI_0, 0x1011, data, 32);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_write_16bit_registers (spi_index_enum spi_n, const uint16 register_name, const uint16 *data, uint32 len)
{
    switch_transition_length(spi_n, 16);					// 切换通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
            gpio_low(cs_pin_save[spi_n]);
    } 
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                        // 发送寄存器地址
    while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));           // 缓冲区满则等待
    
    do
    {
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *data ++);                        // 发送数据
        while(Cy_SCB_GetFifoSize(spi_module[spi_n]) == Cy_SCB_GetNumInTxFifo(spi_module[spi_n]));       // 缓冲区满则等待
        len -= 1;						                // 发送长度自减	
    }while(len);
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)				        // 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口读 8bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 返回参数       uint8           数据
// 使用示例       spi_read_8bit(SPI_0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 spi_read_8bit (spi_index_enum spi_n)
{
    uint8 read_data = 0;
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    switch_transition_length(spi_n, 8);						// 切换单次通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                                   // 发送空数据
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据
    
    read_data = (uint8)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);		// 读取数据
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
    
    return read_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口读 8bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             发送缓冲区长度
// 返回参数       void
// 使用示例       spi_read_8bit_array(SPI_0, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_read_8bit_array (spi_index_enum spi_n, uint8 *data, uint32 len)
{
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    switch_transition_length(spi_n, 8);						// 切换单次通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                               // 发送空数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据
        *data ++ = (uint8)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);	// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口读 16bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 返回参数       uint16          数据
// 使用示例       spi_read_16bit(SPI_0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 spi_read_16bit (spi_index_enum spi_n)
{
    uint16 read_data = 0;
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    switch_transition_length(spi_n, 16);					// 切换单次通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                                   // 发送空数据
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据
    read_data = (uint16)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);		// 读取数据
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
    
    return read_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口读 16bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             发送缓冲区长度
// 返回参数       void
// 使用示例       spi_read_16bit_array(SPI_0, data, 64);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_read_16bit_array (spi_index_enum spi_n, uint16 *data, uint32 len)
{
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    switch_transition_length(spi_n, 16);					// 切换单次通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                               // 发送空数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据
        *data ++ = (uint16)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);	// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口从传感器的寄存器读 8bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 返回参数       uint8           数据
// 使用示例       spi_read_8bit_register(SPI_0, 0x11);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 spi_read_8bit_register (spi_index_enum spi_n, const uint8 register_name)
{
    uint8 read_data = 0;
    
    switch_transition_length(spi_n, 8);						// 切换单次通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                       // 发送寄存器地址
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据    
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                                   // 发送空数据
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据
    read_data = (uint8)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);		// 读取数据
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
    
    return read_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口从传感器的寄存器读 8bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             发送缓冲区长度
// 返回参数       void
// 使用示例       spi_read_8bit_registers(SPI_0, 0x11, data, 32);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_read_8bit_registers (spi_index_enum spi_n, const uint8 register_name, uint8 *data, uint32 len)
{
    switch_transition_length(spi_n, 8);						// 切换单次通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                       // 发送寄存器地址
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据  
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                               // 发送空数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据
        *data ++ = (uint8)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);	// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口从传感器的寄存器读 16bit 数据
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 返回参数       uint16          数据
// 使用示例       spi_read_16bit_register(SPI_0, 0x1011);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 spi_read_16bit_register (spi_index_enum spi_n, const uint16 register_name)
{
    uint16 read_data = 0;
    
    switch_transition_length(spi_n, 16);					// 切换单次通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                       // 发送寄存器地址
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据    
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                                   // 发送空数据
    
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据
    
    read_data = (uint16)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);		// 读取数据
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }	
    
    return read_data;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 接口从传感器的寄存器读 16bit 数组
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       register_name   寄存器地址
// 参数说明       *data           数据存放缓冲区
// 参数说明       len             发送缓冲区长度
// 返回参数       void
// 使用示例       spi_read_16bit_registers(SPI_0, 0x1101, data, 32);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_read_16bit_registers (spi_index_enum spi_n, const uint16 register_name, uint16 *data, uint32 len)
{
    switch_transition_length(spi_n, 16);					// 切换单次通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_WriteTxFifo(spi_module[spi_n], register_name);                       // 发送寄存器地址
    while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                         // 等待数据发送完成
    while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		        // 等待接收到数据  
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], 0);                               // 发送空数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据
        *data ++ = (uint16)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);	// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 8bit 数据传输 发送与接收数据是同时进行的
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       write_buffer    发送的数据缓冲区地址
// 参数说明       read_buffer     发送数据时接收到的数据的存储地址(不需要接收则传 NULL)
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_transfer_8bit(SPI_0, buf, buf, 1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_transfer_8bit (spi_index_enum spi_n, const uint8 *write_buffer, uint8 *read_buffer, uint32 len)
{
    switch_transition_length(spi_n, 8);						// 切换单次通信长度为8位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *write_buffer ++);                // 发送数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据  
        *read_buffer ++ = (uint8)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);	// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       SPI 16bit 数据传输 发送与接收数据是同时进行的
// 参数说明       spi_n           SPI 模块号 参照 zf_driver_spi.h 内 spi_index_enum 枚举体定义
// 参数说明       write_buffer    发送的数据缓冲区地址
// 参数说明       read_buffer     发送数据时接收到的数据的存储地址(不需要接收则传 NULL)
// 参数说明       len             缓冲区长度
// 返回参数       void
// 使用示例       spi_transfer_16bit(SPI_0, buf, buf, 1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_transfer_16bit (spi_index_enum spi_n, const uint16 *write_buffer, uint16 *read_buffer, uint32 len)
{
    switch_transition_length(spi_n, 16);				        // 切换单次通信长度为16位
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉低CS
    {
        gpio_low(cs_pin_save[spi_n]);
    }
    
    Cy_SCB_SPI_ClearRxFifo(spi_module[spi_n]);					// 清除接收缓冲区
    
    do{
        Cy_SCB_WriteTxFifo(spi_module[spi_n], *write_buffer ++);                // 发送数据
        while(Cy_SCB_IsTxComplete(spi_module[spi_n]) == 0);                     // 等待数据发送完成
        while(Cy_SCB_SPI_GetNumInRxFifo(spi_module[spi_n]) == 0);		// 等待接收到数据  
        *read_buffer ++ = (uint16)(spi_module[spi_n]->unRX_FIFO_RD.u32Register);// 读取数据
        len -= 1;
    }while(len);
    
    if(cs_pin_save[spi_n] != SPI_CS_NULL)					// 若CS不为空 则拉高CS
    {
        gpio_high(cs_pin_save[spi_n]);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      SPI初始化
//  参数说明      spi_n           选择SPI模块(SPI_0-SPI_2)
//  参数说明      mode            SPI模式 0：CPOL=0 CPHA=0    1：CPOL=0 CPHA=1   2：CPOL=1 CPHA=0   3：CPOL=1 CPHA=1  // 具体细节可自行查阅资料
//  参数说明      baud            设置SPI的波特率
//  参数说明      cs_pin          选择SPI片选引脚
//  参数说明      sck_pin         选择SPI时钟引脚
//  参数说明      mosi_pin        选择SPI MOSI引脚
//  参数说明      miso_pin        选择SPI MISO引脚
//  返回参数      void
//  使用示例      spi_init(SPI_0, SPI_MODE0, 1*1000*1000, SPI0_CLK_P2_2, SPI0_MOSI_P2_1, SPI0_MISO_P2_0, SPI0_CS0_P2_3); // 硬件SPI初始化  模式0 波特率为1Mhz
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void spi_init (spi_index_enum spi_n, spi_mode_enum mode, uint32 baud, spi_clk_pin_enum clk_pin, spi_mosi_pin_enum mosi_pin, spi_miso_pin_enum miso_pin, spi_cs_pin_enum cs_pin)
{
    uint64_t                    targetFreq                      = 4 * baud;
    uint64_t                    sourceFreq_fp5                  = ((uint64_t)SPI_FREQ << 6ull);
    uint32_t                    divSetting_fp5                  = (uint32_t)(sourceFreq_fp5 / targetFreq);
    cy_stc_gpio_pin_config_t    spi_pin_cfg                     = {0};
    cy_stc_scb_spi_config_t     spi_config                      = {0};
    
    // 醒醒，模块号和端口都不对应怎么能初始化呢？
    zf_assert((uint8)spi_n == (uint8)clk_pin ? 1 : 0);
    zf_assert((uint8)clk_pin == (uint8)mosi_pin ? 1 : 0);

    spi_pin_cfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    spi_pin_cfg.hsiom = spi_get_mosi_hsiom(mosi_pin);
    Cy_GPIO_Pin_Init(get_port(spi_get_mosi_pin(mosi_pin)), (spi_get_mosi_pin(mosi_pin) % 8), &spi_pin_cfg);

    spi_pin_cfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
    spi_pin_cfg.hsiom = spi_get_clk_hsiom(clk_pin);
    Cy_GPIO_Pin_Init(get_port(spi_get_clk_pin(clk_pin)), (spi_get_clk_pin(clk_pin) % 8), &spi_pin_cfg);
    
    if(SPI_MISO_NULL != miso_pin)
    {
        spi_pin_cfg.driveMode = CY_GPIO_DM_HIGHZ;
        spi_pin_cfg.hsiom = spi_get_miso_hsiom(miso_pin);
        Cy_GPIO_Pin_Init(get_port(spi_get_miso_pin(miso_pin)), (spi_get_miso_pin(miso_pin) % 8), &spi_pin_cfg);
    }
    if(SPI_CS_NULL != cs_pin)
    {
        spi_pin_cfg.driveMode = CY_GPIO_DM_STRONG_IN_OFF;
        spi_pin_cfg.hsiom = spi_get_cs_hsiom(cs_pin);
        Cy_GPIO_Pin_Init(get_port(spi_get_cs_pin(cs_pin)), (spi_get_cs_pin(cs_pin) % 8), &spi_pin_cfg);
    }
    
    Cy_SysClk_PeriphAssignDivider(spi_get_module_clock(spi_n), CY_SYSCLK_DIV_24_5_BIT, ((uint8)spi_n + 1));
    Cy_SysClk_PeriphSetFracDivider(CY_SYSCLK_DIV_24_5_BIT, ((uint8)spi_n + 1), ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul), (divSetting_fp5 & 0x0000001Ful));
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_24_5_BIT, ((uint8)spi_n + 1));
    
    switch(mode)
    {
        case SPI_MODE0:spi_config.sclkMode = CY_SCB_SPI_CPHA0_CPOL0;      break;
        case SPI_MODE1:spi_config.sclkMode = CY_SCB_SPI_CPHA0_CPOL1;      break;
        case SPI_MODE2:spi_config.sclkMode = CY_SCB_SPI_CPHA1_CPOL0;      break;
        case SPI_MODE3:spi_config.sclkMode = CY_SCB_SPI_CPHA1_CPOL1;      break;
    }
    
    spi_config.spiMode                    = CY_SCB_SPI_MASTER     ;
    spi_config.subMode                    = CY_SCB_SPI_MOTOROLA   ;
    spi_config.oversample                 = 4                     ;
    spi_config.rxDataWidth                = 8                     ;
    spi_config.txDataWidth                = 8                     ;
    spi_config.enableMsbFirst             = true                  ;
    spi_config.enableMisoLateSample       = true                  ;
    
    Cy_SCB_SPI_Init(spi_module[spi_n], &spi_config, NULL);
    Cy_SCB_SPI_SetActiveSlaveSelect(spi_module[spi_n], 0ul);
    Cy_SCB_SPI_Enable(spi_module[spi_n]);
    
    cs_pin_save[spi_n] = cs_pin;    
}

