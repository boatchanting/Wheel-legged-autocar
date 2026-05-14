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
* 文件名称          driver_adc
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

#ifndef _driver_adc_h_
#define _driver_adc_h_

#include "zf_common_headfile.h"


#define BAT_ADC                         (ADC0_CH18_P07_2)               // 电池电压检测引脚


#define VOLTAGE_CONVERSION_COEFFICIENT  (0.00886446)                    // 电池电压转换系数 12位ADC采集值 直接乘以该值则可得出电池电压 


typedef enum
{
    BATTERY_HEALTH,                                                     // 电池电压正常    
    
    BATTERY_WARNING,                                                    // 电池低压警告
    
    BATTERY_ERROR                                                       // 电池电压异常
}battery_state_enum;


typedef struct
{
    uint8                       protect_flag;                           // 电池电压保护使能标志位               
    
    float                       protect_voltage_min;                    // 单节电芯最低电压值       默认3.5 
    
    float                       protect_voltage_max;                    // 单节电芯最高电压值       默认4.3     
    
    float                       worning_voltage;                        // 单节电芯警报电压值       默认3.9
    
    float                       voltage_rectify_coefficient;            // 电池电压校准系数 根据实际硬件调整 默认0.9959968

    float                       battery_voltage;                        // 电池当前电压
    
    battery_state_enum          battery_state;                          // 电池当前状态
    
}battery_value_struct;


extern battery_value_struct battery_value;


void driver_adc_loop(void);

void driver_adc_init(void);


#endif
