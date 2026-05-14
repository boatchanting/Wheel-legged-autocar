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
* 文件名称          sensorless_control
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

#ifndef _SENSORLESS_CONTROL_H_
#define _SENSORLESS_CONTROL_H_

#include "zf_common_headfile.h"


#define MOTOR_LEFT_SENSORLESS_A_PHASE_H      TCPWM_CH48_P14_0                   // 左侧电机 A相上桥 控制引脚（输出PWM）

#define MOTOR_LEFT_SENSORLESS_A_PHASE_L      P14_1                              // 左侧电机 A相下桥 控制引脚（常开 or 常闭）

#define MOTOR_LEFT_SENSORLESS_B_PHASE_H      TCPWM_CH53_P18_4                   // 左侧电机 B相上桥 控制引脚（输出PWM）

#define MOTOR_LEFT_SENSORLESS_B_PHASE_L      P18_5                              // 左侧电机 B相下桥 控制引脚（常开 or 常闭）

#define MOTOR_LEFT_SENSORLESS_C_PHASE_H      TCPWM_CH51_P18_6                   // 左侧电机 C相上桥 控制引脚（输出PWM）

#define MOTOR_LEFT_SENSORLESS_C_PHASE_L      P18_7                              // 左侧电机 C相下桥 控制引脚（常开 or 常闭）


#define MOTOR_LEFT_SENSORLESS_A_TRIG         P06_0                              // 左侧电机 A相 过零信号触发引脚

#define MOTOR_LEFT_SENSORLESS_B_TRIG         P06_1                              // 左侧电机 B相 过零信号触发引脚

#define MOTOR_LEFT_SENSORLESS_C_TRIG         P06_2                              // 左侧电机 C相 过零信号触发引脚


#define MOTOR_LEFT_SENSORLESS_DELAY_CLOCK    PCLK_TCPWM0_CLOCKS42               // 左侧电机 延时换相定时器 时钟通道

#define MOTOR_LEFT_SENSORLESS_DELAY_ISR      tcpwm_0_interrupts_42_IRQn         // 左侧电机 延时换相定时器 中断通道

#define MOTOR_LEFT_SENSORLESS_DELAY_TIMER    TCPWM0_GRP0_CNT42                  // 左侧电机 延时换相定时器 通道




#define MOTOR_RIGHT_SENSORLESS_A_PHASE_H     TCPWM_CH14_P00_2                   // 右侧电机 A相上桥 控制引脚（输出PWM）

#define MOTOR_RIGHT_SENSORLESS_A_PHASE_L     P00_3                              // 右侧电机 A相下桥 控制引脚（常开 or 常闭）

#define MOTOR_RIGHT_SENSORLESS_B_PHASE_H     TCPWM_CH07_P02_0                   // 右侧电机 B相上桥 控制引脚（输出PWM）

#define MOTOR_RIGHT_SENSORLESS_B_PHASE_L     P02_1                              // 右侧电机 B相下桥 控制引脚（常开 or 常闭）

#define MOTOR_RIGHT_SENSORLESS_C_PHASE_H     TCPWM_CH09_P05_0                   // 右侧电机 C相上桥 控制引脚（输出PWM）

#define MOTOR_RIGHT_SENSORLESS_C_PHASE_L     P05_1                              // 右侧电机 C相下桥 控制引脚（常开 or 常闭）

#define MOTOR_RIGHT_SENSORLESS_A_TRIG        P13_0                              // 右侧电机 A相 过零信号触发引脚

#define MOTOR_RIGHT_SENSORLESS_B_TRIG        P13_1                              // 右侧电机 B相 过零信号触发引脚

#define MOTOR_RIGHT_SENSORLESS_C_TRIG        P13_2                              // 右侧电机 C相 过零信号触发引脚

#define MOTOR_RIGHT_SENSORLESS_DELAY_CLOCK   PCLK_TCPWM0_CLOCKS41               // 右侧电机 延时换相定时器 时钟通道

#define MOTOR_RIGHT_SENSORLESS_DELAY_ISR     tcpwm_0_interrupts_41_IRQn         // 右侧电机 延时换相定时器 中断通道

#define MOTOR_RIGHT_SENSORLESS_DELAY_TIMER   TCPWM0_GRP0_CNT41                  // 右侧电机 延时换相定时器 通道




void sensorless_motor_isr(void);

void sensorless_start_check(void);

void sensorless_trig_init(void);









#endif