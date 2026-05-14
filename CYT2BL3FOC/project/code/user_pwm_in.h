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
* 文件名称          user_pwm_in
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2025-05-12       pudding            first version
********************************************************************************************************************/

#ifndef _USER_PWM_IN_H_
#define _USER_PWM_IN_H_

#include "zf_common_headfile.h"


#define MOTOR_LEFT_PWM_IN_PIN                   (P07_1)                         // 左侧脉宽捕获使用的GOIO

#define MOTOR_LEFT_PWM_IN_HSIOM                 (P7_1_TCPWM0_TR_ONE_CNT_IN45)   // 左侧脉宽捕获使用的GOIO的功能复用情况

#define MOTOR_LEFT_PWM_IN_CLOCK                 (PCLK_TCPWM0_CLOCKS15)          // 左侧脉宽捕获使用的定时器时钟通道

#define MOTOR_LEFT_PWM_IN_TIMER                 (TCPWM0_GRP0_CNT15)             // 左侧脉宽捕获使用的定时器通道

#define MOTOR_LEFT_PWM_IN_ISR                   (tcpwm_0_interrupts_15_IRQn)    // 左侧脉宽捕获使用的定时器中断通道

#define MOTOR_LEFT_PWM_IN_OVERTIME_CLOCK        (PCLK_TCPWM0_CLOCKS20)          // 左侧脉宽捕获 超时 使用的定时器时钟通道

#define MOTOR_LEFT_PWM_IN_OVERTIME_TIMER        (TCPWM0_GRP0_CNT20)             // 左侧脉宽捕获 超时 使用的定时器通道

#define MOTOR_LEFT_PWM_IN_OVERTIME_ISR          (tcpwm_0_interrupts_20_IRQn)    // 左侧脉宽捕获 超时 使用的定时器中断通道


#define MOTOR_RIGHT_PWM_IN_PIN                  (P07_0)                         // 右侧脉宽捕获使用的GOIO

#define MOTOR_RIGHT_PWM_IN_HSIOM                (P7_0_TCPWM0_TR_ONE_CNT_IN780)  // 右侧脉宽捕获使用的GOIO的功能复用情况

#define MOTOR_RIGHT_PWM_IN_CLOCK                (PCLK_TCPWM0_CLOCKS260)         // 右侧脉宽捕获使用的定时器时钟通道

#define MOTOR_RIGHT_PWM_IN_TIMER                (TCPWM0_GRP1_CNT4)              // 右侧脉宽捕获使用的定时器通道

#define MOTOR_RIGHT_PWM_IN_ISR                  (tcpwm_0_interrupts_260_IRQn)   // 右侧脉宽捕获使用的定时器中断通道

#define MOTOR_RIGHT_PWM_IN_OVERTIME_CLOCK       (PCLK_TCPWM0_CLOCKS19)          // 右侧脉宽捕获 超时 使用的定时器时钟通道

#define MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER       (TCPWM0_GRP0_CNT19)             // 右侧脉宽捕获 超时 使用的定时器通道

#define MOTOR_RIGHT_PWM_IN_OVERTIME_ISR         (tcpwm_0_interrupts_19_IRQn)    // 右侧脉宽捕获 超时 使用的定时器中断通道


typedef struct
{
    uint16 pwm_high_level;
    uint16 pwm_low_level;
    uint16 pwm_period_num;
    float  get_duty;
}user_in_struct;


extern user_in_struct motor_left_pwm_in_data;
extern user_in_struct motor_right_pwm_in_data;


void pwm_in_init(void);



#endif