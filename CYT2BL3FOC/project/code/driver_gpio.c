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
* 文件名称          driver_gpio
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

#include "driver_gpio.h"

uint32 gpio_loop_count = 0;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     驱动的 GPIO循环检测函数
// 参数说明     void
// 返回参数     void  
// 使用示例     driver_gpio_loop();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void driver_gpio_loop(void)
{
    static uint8 high_freq_count = (100 / DRIVER_RESPONSE_CYCLE);
    static uint8 low_freq_count  = (500 / DRIVER_RESPONSE_CYCLE);
    
    gpio_loop_count ++;
    
    key_scanner();                                                              // 按键扫描
      
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)                                 // 如果按键按下 则执行内容
    {
        key_clear_state(KEY_1);                                                 // 清除按下标志位

        motor_zero_calibration();                                               // 电机零位矫正
    } 
      
    if(battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR)     // 电池低电压保护  
    {
        if(gpio_loop_count % high_freq_count == 0)                              // 100ms高频闪烁 (主函数默认延时10ms)
        {
            gpio_toggle_level(BOARD_LED);                                       
        }
    }
    else if(motor_left.encoder_state == ENCODER_ERROR || motor_right.encoder_state == ENCODER_ERROR)          
    {   
        if(gpio_loop_count % low_freq_count == 0)                                           
        {
            gpio_toggle_level(BOARD_LED);                                       // 500ms低频闪烁 (主函数默认延时10ms)
        }
    }
    else if((motor_left.locked_value.protect_flag == 1) && (motor_left.motor_protect_state == PROTECT_MODE || motor_right.motor_protect_state == PROTECT_MODE))  
    {
        if(gpio_loop_count % low_freq_count == 0)                                           
        {
            gpio_toggle_level(BOARD_LED);                                       // 500ms低频闪烁 (主函数默认延时10ms)
        }       
    }
    else if((motor_left.driver_mode == SENSORLESS) && (motor_left.sensorless_state != SENSORLESS_CLOSE_LOOP || motor_right.sensorless_state != SENSORLESS_CLOSE_LOOP))  
    {
        if(gpio_loop_count % low_freq_count == 0)                                           
        {
            gpio_toggle_level(BOARD_LED);                                       // 500ms低频闪烁 (主函数默认延时10ms)
        }       
    }
    else     
    {
        gpio_set_level(BOARD_LED, 0);                                           // 正常运行则 常亮
    }
    
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     驱动的 GPIO 功能初始化
// 参数说明     void
// 返回参数     void  
// 使用示例     driver_gpio_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void driver_gpio_init(void)
{
    gpio_init(BOARD_LED, GPO, 0, GPO_PUSH_PULL);                                // 板载LED灯初始化
    
    key_init(DRIVER_RESPONSE_CYCLE);                                            // 板载按键初始化
}







