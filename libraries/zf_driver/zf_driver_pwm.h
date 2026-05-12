/*********************************************************************************************************************
* CYT4BB Opensourec Library 即（ CYT4BB 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 CYT4BB 开源库的一部分
*
* CYT4BB 开源库 是免费软件
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
* 文件名称          zf_driver_pwm
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-8       pudding            first version
********************************************************************************************************************/

#ifndef _zf_driver_pwm_h_
#define _zf_driver_pwm_h_

#include "zf_common_typedef.h"

#define PWM_DUTY_MAX     10000                 // PWM最大占空比  最大占空比越大占空比的步进值越小

// 此枚举定义不允许用户修改
typedef enum // 枚举PWM引脚
{
	TCPWM_CH00_P03_1 ,	TCPWM_CH00_P06_1 ,
	TCPWM_CH01_P03_0 ,	TCPWM_CH01_P06_3 ,
	TCPWM_CH02_P06_5 ,	                 
	TCPWM_CH03_P02_4 , 	TCPWM_CH03_P06_7 ,
	TCPWM_CH04_P02_3 , 	TCPWM_CH04_P04_0 ,
	TCPWM_CH05_P02_2 , 	TCPWM_CH05_P04_1 ,
	TCPWM_CH06_P02_1 , 
	TCPWM_CH07_P02_0 , 
	TCPWM_CH09_P05_0 , 
	TCPWM_CH10_P05_1 , 
	TCPWM_CH11_P05_2 , 	TCPWM_CH11_P01_1 ,		
	TCPWM_CH12_P05_3 , 	TCPWM_CH12_P01_0 ,		
	TCPWM_CH13_P05_4 , 	TCPWM_CH13_P00_3 ,			
	TCPWM_CH14_P00_2 , 
	TCPWM_CH15_P07_1 , 
	TCPWM_CH16_P07_3 , 
	TCPWM_CH17_P07_5 , 	TCPWM_CH17_P00_1 , 	
	TCPWM_CH18_P07_7 , 	TCPWM_CH18_P00_0 , 	
	TCPWM_CH19_P08_0 ,	
	TCPWM_CH20_P08_1 ,	
	TCPWM_CH21_P08_2 ,	
	TCPWM_CH22_P08_3 , 	TCPWM_CH22_P23_7 ,	
	TCPWM_CH24_P09_0 ,			 
	TCPWM_CH25_P09_1 , 	TCPWM_CH25_P23_4 ,		
	TCPWM_CH26_P19_1 ,                       
	TCPWM_CH27_P19_2 ,                       
	TCPWM_CH28_P10_0 ,	TCPWM_CH28_P19_3 ,	TCPWM_CH28_P22_6 ,		
	TCPWM_CH29_P10_1 ,	TCPWM_CH29_P19_4 ,	TCPWM_CH29_P22_5 ,		
	TCPWM_CH30_P10_2 ,	TCPWM_CH30_P20_0 ,	TCPWM_CH30_P22_4 ,		
	TCPWM_CH31_P10_3 ,	TCPWM_CH31_P22_3 ,		
	TCPWM_CH32_P10_4 ,	TCPWM_CH32_P22_2 ,		
	TCPWM_CH33_P22_1 ,                       
	TCPWM_CH34_P21_5 ,                       
	TCPWM_CH36_P12_0 ,	TCPWM_CH36_P21_6 ,		
	TCPWM_CH37_P12_1 ,	TCPWM_CH37_P21_5 ,		
	TCPWM_CH38_P12_2 ,	                 
	TCPWM_CH39_P12_3 ,	TCPWM_CH39_P21_3 ,		
	TCPWM_CH40_P12_4 ,	TCPWM_CH40_P21_2 ,		
	TCPWM_CH41_P12_5 ,	TCPWM_CH41_P21_1 ,		
	TCPWM_CH42_P21_0 ,	                 
	TCPWM_CH44_P13_1 ,	                 
	TCPWM_CH45_P13_3 ,	                 
	TCPWM_CH46_P13_5 ,	                 
	TCPWM_CH47_P13_7 ,	TCPWM_CH47_P20_3 ,		
	TCPWM_CH48_P14_0 ,	TCPWM_CH48_P20_2 ,	 	
	TCPWM_CH49_P14_1 ,	TCPWM_CH49_P20_1 ,	 	
	TCPWM_CH50_P18_7 , 	                 
	TCPWM_CH51_P18_6 ,	                 
	TCPWM_CH52_P14_4 ,	TCPWM_CH52_P18_5 ,		
	TCPWM_CH53_P14_5 ,	TCPWM_CH53_P18_4 ,		
	TCPWM_CH54_P18_3 ,	                 
	TCPWM_CH55_P18_2 ,	                 
	TCPWM_CH56_P15_0 ,	                 
	TCPWM_CH57_P15_1 ,	TCPWM_CH57_P17_4 ,		
	TCPWM_CH58_P15_2 ,	TCPWM_CH58_P17_3 ,		
	TCPWM_CH59_P15_3 ,	TCPWM_CH59_P17_2 ,	TCPWM_CH59_P11_2 ,		
	TCPWM_CH60_P11_1 ,	TCPWM_CH60_P17_1 ,		
	TCPWM_CH61_P11_0 ,	TCPWM_CH61_P17_0 ,	
	
	TCPWM_CH_NUM	 ,
}pwm_channel_enum;


//====================================================PWM 基础函数====================================================
void pwm_all_channel_close      (void);
void pwm_set_duty               (pwm_channel_enum pwmch, uint32 duty);
void pwm_init                   (pwm_channel_enum pwmch, uint32 freq, uint32 duty);
//====================================================PWM 基础函数====================================================

#endif
