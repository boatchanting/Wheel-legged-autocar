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
*                   SCL/SPC           查看 zf_device_menc15a.h 中 MENC15A_SPC_PIN 宏定义
*                   SDA/DSI           查看 zf_device_menc15a.h 中 MENC15A_SDI_PIN 宏定义
*                   SA0/SDO           查看 zf_device_menc15a.h 中 MENC15A_SDO_PIN 宏定义
*                   CS                查看 zf_device_menc15a.h 中 MENC15A_CS_PIN 宏定义
*                   VCC               3.3V电源
*                   GND               电源地
*                   其余引脚悬空
*                   ------------------------------------
********************************************************************************************************************/

#include "zf_common_debug.h"
#include "zf_common_function.h"
#include "zf_device_config.h"
#include "zf_driver_delay.h"
#include "zf_driver_gpio.h"
#include "zf_driver_spi.h"

#include "zf_device_menc15a.h"

uint16 menc15a_absolute_data[2] = {0};

int16  menc15a_absolute_offset_data[2] = {0};

int16  menc15a_speed_data[2] = {0};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     计算编码器旋转偏差 
// 参数说明     encoder_max     编码器精度  填写十进制数据
// 参数说明     now_location    当前位置信息
// 参数说明     last_location   上一次的位置信息
// 返回参数     int32           计算的偏差值    
// 使用示例     magnetic_encoder_get_offset();                                  
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
static int32 magnetic_encoder_get_offset (int32 encoder_max, int32 now_location, int32 last_location)
{
    int32 result_data = 0;
	
    if((encoder_max / 2) < func_abs(now_location - last_location))
    {
        result_data = ((encoder_max / 2) < now_location ? (now_location - encoder_max - last_location) : (now_location + encoder_max - last_location));
    }
    else
    {
        result_data = (now_location - last_location);
    }
	
    return result_data;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取 MENC15A 磁编码器 的 绝对值 角度数据
// 参数说明     menc15a_module      磁编码器 模块号
// 返回参数     uint16              绝对值 角度数据       
// 使用示例     menc15a_get_absolute_data(menc15a_2_module);                                   
// 备注信息     执行该函数后，可直接使用返回值 也可以通过查询对应变量获取结果
//-------------------------------------------------------------------------------------------------------------------
uint16 menc15a_get_absolute_data(menc15a_module_enum menc15a_module)
{
    uint16 read_data = 0;
  
    uint16 data_last = menc15a_absolute_data[menc15a_module];
    
    if(menc15a_module == menc15a_1_module)
    {
        MENC15A_1_CS(0);
      
        read_data = spi_read_16bit_register(MENC15A_1_SPI, 0x8021);
        
        MENC15A_1_CS(1);
    }
    else
    {
        MENC15A_2_CS(0);
      
        read_data = spi_read_16bit_register(MENC15A_2_SPI, 0x8021);
        
        MENC15A_2_CS(1);
    }
  
    menc15a_absolute_data[menc15a_module] = (read_data & 0x7fff);
    
    menc15a_absolute_offset_data[menc15a_module] = magnetic_encoder_get_offset(32768, menc15a_absolute_data[menc15a_module], data_last);
    
    return menc15a_absolute_data[menc15a_module];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取 MENC15A 磁编码器 的 转速数据
// 参数说明     menc15a_module      磁编码器 模块号
// 返回参数     int16               转速数据(每秒旋转弧度 rad/s)       
// 使用示例     menc15a_1_get_speed_data();                                  
// 备注信息     执行该函数后，可直接使用返回值 也可以通过查询对应变量获取结果
//-------------------------------------------------------------------------------------------------------------------
int16 menc15a_get_speed_data(menc15a_module_enum menc15a_module)
{
    int16 read_data = 0;
    
    if(menc15a_module == menc15a_1_module)
    {
        MENC15A_1_CS(0);
      
        read_data = spi_read_16bit_register(MENC15A_1_SPI, 0x8032);
        
        MENC15A_1_CS(1);
    }
    else
    {
        MENC15A_2_CS(0);
      
        read_data = spi_read_16bit_register(MENC15A_2_SPI, 0x8032);
        
        MENC15A_2_CS(1);
    }
  
    read_data = (read_data & 0x7fff);
    
    if(read_data > 16384)
    {
        read_data = read_data - 32768;
    }
    
    menc15a_speed_data[menc15a_module] = (int16)((float)read_data * 1.917476f);
    
    return menc15a_speed_data[menc15a_module];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     初始化 MENC15A
// 参数说明     void
// 返回参数     uint8           1-初始化失败 0-初始化成功
// 使用示例     menc15a_init();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 menc15a_init(void)
{
    spi_init(MENC15A_1_SPI, SPI_MODE0, MENC15A_1_SPI_SPEED, MENC15A_1_CLK_PIN, MENC15A_1_MOSI_PIN, MENC15A_1_MISO_PIN, SPI_CS_NULL);    // 配置 MENC15A-1 的 SPI端口
    gpio_init(MENC15A_1_CS_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);                                                                         // 配置 MENC15A-1 的 CS端口
    
    spi_init(MENC15A_2_SPI, SPI_MODE0, MENC15A_2_SPI_SPEED, MENC15A_2_CLK_PIN, MENC15A_2_MOSI_PIN, MENC15A_2_MISO_PIN, SPI_CS_NULL);    // 配置 MENC15A-2 的 SPI端口
    gpio_init(MENC15A_2_CS_PIN, GPO, GPIO_HIGH, GPO_PUSH_PULL);                                                                         // 配置 MENC15A-2 的 CS端口
    
    return 0;
}

