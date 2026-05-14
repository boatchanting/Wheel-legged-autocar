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
* 文件名称          zf_driver_adc
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

#ifndef _zf_driver_adc_h
#define _zf_driver_adc_h

#include "zf_common_typedef.h"

typedef enum    // 枚举ADC通道
{
      // ADC0可选引脚
      ADC0_CH00_P06_0 = 0 * 32 + 0	,
      ADC0_CH01_P06_1 = 0 * 32 + 1	,
      ADC0_CH02_P06_2 = 0 * 32 + 2	,
      ADC0_CH03_P06_3 = 0 * 32 + 3	,
      ADC0_CH04_P06_4 = 0 * 32 + 4	,
      ADC0_CH05_P06_5 = 0 * 32 + 5	,
      ADC0_CH06_P06_6 = 0 * 32 + 6	,
      ADC0_CH16_P07_0 = 0 * 32 + 8	,
      ADC0_CH17_P07_1 = 0 * 32 + 9	,
      ADC0_CH18_P07_2 = 0 * 32 + 10	,
      ADC0_CH24_P08_1 = 0 * 32 + 16	,

      // ADC1可选引脚	
      ADC1_CH04_P12_0 = 1 * 32 + 4	,
      ADC1_CH05_P12_1 = 1 * 32 + 5	,
      ADC1_CH12_P13_0 = 1 * 32 + 12	,
      ADC1_CH13_P13_1 = 1 * 32 + 13	,
      ADC1_CH14_P13_2 = 1 * 32 + 14	,
      ADC1_CH15_P13_3 = 1 * 32 + 15	,
      ADC1_CH20_P14_0 = 1 * 32 + 20	,
      ADC1_CH21_P14_1 = 1 * 32 + 21	,
      ADC1_CH22_P14_2 = 1 * 32 + 22	,	

      // ADC2可选引脚		
      ADC2_CH00_P18_0 = 2 * 32 + 0      ,
      ADC2_CH01_P18_1 = 2 * 32 + 1	,
      ADC2_CH03_P18_3 = 2 * 32 + 3	,
      ADC2_CH04_P18_4 = 2 * 32 + 4	,
      ADC2_CH05_P18_5 = 2 * 32 + 5	,
      ADC2_CH06_P18_6 = 2 * 32 + 6	,
      ADC2_CH07_P18_7 = 2 * 32 + 7	,
	
      ADC_CHANNEL_NUM,
}adc_channel_enum;

// 此枚举定义不允许用户修改
typedef enum        // 枚举ADC通道
{
    ADC_8BIT,       // 8位分辨率
    ADC_10BIT,      // 10位分辨率
    ADC_12BIT,      // 12位分辨率
}adc_resolution_enum;

//====================================================ADC 基础函数====================================================
uint16  adc_convert             (adc_channel_enum vadc_chn);                                    // ADC转换一次
uint16  adc_mean_filter_convert (adc_channel_enum vadc_chn, uint8 count);                       // ADC均值滤波
void    adc_init                (adc_channel_enum vadc_chn, adc_resolution_enum resolution);    // ADC初始化
//====================================================ADC 基础函数====================================================


#endif
