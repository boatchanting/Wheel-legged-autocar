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
* 文件名称          zf_driver_gpio
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-4       pudding            first version
********************************************************************************************************************/

#include "gpio/cy_gpio.h"
#include "zf_driver_gpio.h"




//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 输出设置
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 参数说明     dat         0：低电平 1：高电平
// 返回参数     void
// 使用示例     gpio_set_level(P00_0, 1);// P00_0 输出高电平
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void gpio_set_level (gpio_pin_enum pin, uint8 dat)
{
    if(dat)
    {
        Cy_GPIO_Set(get_port(pin), (pin % 8));
    }
    else
    {
        Cy_GPIO_Clr(get_port(pin), (pin % 8));
    }
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 电平获取
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     uint8       引脚当前电平
// 使用示例     uint8 status = gpio_get_level(P00_0);// 获取P00_0引脚电平
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 gpio_get_level (gpio_pin_enum pin)
{
    return (uint8)Cy_GPIO_Read(get_port(pin), (pin % 8));
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 翻转电平
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     gpio_toggle_level(P00_0);// P00_0引脚电平翻转
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void gpio_toggle_level (gpio_pin_enum pin)
{
    Cy_GPIO_Inv(get_port(pin), (pin % 8));
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 方向设置
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 参数说明     dir         引脚的方向   输出：GPO   输入：GPI
// 参数说明     mode        引脚的模式 (可选择范围由 zf_driver_gpio.h 内 gpio_mode_enum 枚举值确定)
// 返回参数     void
// 使用示例     gpio_set_dir(P00_0, GPO, GPO_PUSH_PULL);// 设置P00_0为推挽输出模式
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void gpio_set_dir (gpio_pin_enum pin, gpio_dir_enum dir, gpio_mode_enum pinmode)
{
    uint32 port_mode;
    if(dir == GPI)
    {
        switch(pinmode)
        {
            case GPI_FLOATING_IN:port_mode = CY_GPIO_DM_PULLUP_DOWN;		break; // 浮空输入
            case GPI_PULL_DOWN  :port_mode = CY_GPIO_DM_PULLDOWN;       	break; // 下拉输入
            default:             port_mode = CY_GPIO_DM_PULLUP;       		break; // 默认为上拉输入
        }
    }
    else
    {
        switch(pinmode)
        {
            case GPO_OPEN_DTAIN :port_mode = CY_GPIO_DM_OD_DRIVESLOW; 		break;// 开漏输出
            default:             port_mode = CY_GPIO_DM_STRONG ;   			break;// 默认为推挽输出
        }
		port_mode &= (0x07UL);
    }

    Cy_GPIO_SetDrivemode(get_port(pin), (pin % 8), port_mode);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     gpio 初始化
// 参数说明     pin         选择的引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 参数说明     mode        引脚的方向 [GPI/GPIO]
// 参数说明     dat         引脚初始化时设置的电平状态，输出时有效 0：低电平 1：高电平 仅在设置为输出模式时有效
// 参数说明     mode        引脚的模式 (可选择范围由 zf_driver_gpio.h 内 gpio_mode_enum 枚举值确定)
// 返回参数     void
// 使用示例     gpio_init(P23_7, GPO, 0, GPO_PUSH_PULL); // P23_7 初始化为GPIO功能、输出模式、输出低电平、推挽输出
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void gpio_init (gpio_pin_enum pin, gpio_dir_enum dir, uint8 dat, gpio_mode_enum pinmode)
{
    cy_stc_gpio_pin_config_t gpio_pin_config = {0};
    
    if(dir == GPI)
    {
        switch(pinmode)
        {
            case GPI_FLOATING_IN:gpio_pin_config.driveMode = CY_GPIO_DM_PULLUP_DOWN;	break; // 浮空输入
            case GPI_PULL_DOWN  :gpio_pin_config.driveMode = CY_GPIO_DM_HIGHZ;       break; // 下拉输入
            default:             gpio_pin_config.driveMode = CY_GPIO_DM_HIGHZ;       	break; // 默认为上拉输入
        }
    }
    else
    {
        switch(pinmode)
        {
            case GPO_OPEN_DTAIN :gpio_pin_config.driveMode = CY_GPIO_DM_PULLUP_DOWN; 		break;// 开漏输出
            default:             gpio_pin_config.driveMode = CY_GPIO_DM_STRONG ;   		break;// 默认为推挽输出
        }
    }
    if(pin != NC)
    {
        Cy_GPIO_Pin_Init(get_port(pin), (pin % 8), &gpio_pin_config);
    }
    gpio_set_level(pin, dat);
}



