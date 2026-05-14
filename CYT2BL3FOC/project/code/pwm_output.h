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
* 文件名称          pwm_output
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

#ifndef _PWM_OUTPUT_H_
#define _PWM_OUTPUT_H_

#include "zf_common_headfile.h"

#define FPWM                               (uint16)(20000)                      // PWM频率
#define PWM_PRIOD_LOAD                     (uint16)(4000)                       // PWM周期装载值

#define MOTOR_LEFT_A_PHASE_PORT_H          (GPIO_PRT14)                         // 左电机A相上桥控制引脚端口
#define MOTOR_LEFT_A_PHASE_PIN_H           (0)                                  // 左电机A相上桥控制引脚号
#define MOTOR_LEFT_A_PIN_HSIOM_H           (P14_0_TCPWM0_LINE48)                // 左电机A相上桥引脚复用功能

#define MOTOR_LEFT_A_PHASE_PORT_L          (GPIO_PRT14)                         // 左电机A相下桥控制引脚端口
#define MOTOR_LEFT_A_PHASE_PIN_L           (1)                                  // 左电机A相下桥控制引脚号
#define MOTOR_LEFT_A_PIN_HSIOM_L           (P14_1_TCPWM0_LINE_COMPL48)          // 左电机A相下桥引脚复用功能

#define MOTOR_LEFT_B_PHASE_PORT_H          (GPIO_PRT18)                         // 左电机B相上桥控制引脚端口
#define MOTOR_LEFT_B_PHASE_PIN_H           (4)                                  // 左电机B相上桥控制引脚号
#define MOTOR_LEFT_B_PIN_HSIOM_H           (P18_4_TCPWM0_LINE53)                // 左电机B相上桥引脚复用功能

#define MOTOR_LEFT_B_PHASE_PORT_L          (GPIO_PRT18)                         // 左电机B相下桥控制引脚端口
#define MOTOR_LEFT_B_PHASE_PIN_L           (5)                                  // 左电机B相下桥控制引脚号
#define MOTOR_LEFT_B_PIN_HSIOM_L           (P18_5_TCPWM0_LINE_COMPL53)          // 左电机B相下桥引脚复用功能

#define MOTOR_LEFT_C_PHASE_PORT_H          (GPIO_PRT18)                         // 左电机C相上桥控制引脚端口
#define MOTOR_LEFT_C_PHASE_PIN_H           (6)                                  // 左电机C相上桥控制引脚号
#define MOTOR_LEFT_C_PIN_HSIOM_H           (P18_6_TCPWM0_LINE51)                // 左电机C相上桥引脚复用功能

#define MOTOR_LEFT_C_PHASE_PORT_L          (GPIO_PRT18)                         // 左电机C相下桥控制引脚端口
#define MOTOR_LEFT_C_PHASE_PIN_L           (7)                                  // 左电机C相下桥控制引脚号
#define MOTOR_LEFT_C_PIN_HSIOM_L           (P18_7_TCPWM0_LINE_COMPL51)          // 左电机C相下桥引脚复用功能

#define MOTOR_LEFT_A_PHASE_TCPWM_TIMER     (TCPWM0_GRP0_CNT48)                  // 左电机A相定时器
#define MOTOR_LEFT_A_PHASE_TCPWM_CLOCK     (PCLK_TCPWM0_CLOCKS48)               // 左电机A相定时器时钟通道
#define MOTOR_LEFT_A_PHASE_TCPWM_ADDR      (0x40381800)                         // 左电机A相定时器通道地址(寄存器地址)

#define MOTOR_LEFT_B_PHASE_TCPWM_TIMER     (TCPWM0_GRP0_CNT53)                  // 左电机B相定时器
#define MOTOR_LEFT_B_PHASE_TCPWM_CLOCK     (PCLK_TCPWM0_CLOCKS53)               // 左电机B相定时器时钟通道
#define MOTOR_LEFT_B_PHASE_TCPWM_ADDR      (0x40381A80)                         // 左电机B相定时器通道地址(寄存器地址)

#define MOTOR_LEFT_C_PHASE_TCPWM_TIMER     (TCPWM0_GRP0_CNT51)                  // 左电机C相定时器
#define MOTOR_LEFT_C_PHASE_TCPWM_CLOCK     (PCLK_TCPWM0_CLOCKS51)               // 左电机C相定时器时钟通道
#define MOTOR_LEFT_C_PHASE_TCPWM_ADDR      (0x40381980)                         // 左电机C相定时器通道地址(寄存器地址)

#define MOTOR_LEFT_TCPWM_ISR_CHANNEL       (tcpwm_0_interrupts_48_IRQn)         // 以A相定时器通道更新中断作为整体更新中断     


#define MOTOR_RIGHT_A_PHASE_PORT_H         (GPIO_PRT0)                          // 右电机A相上桥控制引脚端口
#define MOTOR_RIGHT_A_PHASE_PIN_H          (2)                                  // 右电机A相上桥控制引脚号
#define MOTOR_RIGHT_A_PIN_HSIOM_H          (P0_2_TCPWM0_LINE14)                 // 右电机A相上桥引脚复用功能
                                                                                
#define MOTOR_RIGHT_A_PHASE_PORT_L         (GPIO_PRT0)                          // 右电机A相下桥控制引脚端口
#define MOTOR_RIGHT_A_PHASE_PIN_L          (3)                                  // 右电机A相下桥控制引脚号
#define MOTOR_RIGHT_A_PIN_HSIOM_L          (P0_3_TCPWM0_LINE_COMPL14)           // 右电机A相下桥引脚复用功能
                                                                     
#define MOTOR_RIGHT_B_PHASE_PORT_H         (GPIO_PRT2)                          // 右电机B相上桥控制引脚端口
#define MOTOR_RIGHT_B_PHASE_PIN_H          (0)                                  // 右电机B相上桥控制引脚号
#define MOTOR_RIGHT_B_PIN_HSIOM_H          (P2_0_TCPWM0_LINE7)                  // 右电机B相上桥引脚复用功能
                              
#define MOTOR_RIGHT_B_PHASE_PORT_L         (GPIO_PRT2)                          // 右电机B相下桥控制引脚端口
#define MOTOR_RIGHT_B_PHASE_PIN_L          (1)                                  // 右电机B相下桥控制引脚号
#define MOTOR_RIGHT_B_PIN_HSIOM_L          (P2_1_TCPWM0_LINE_COMPL7)            // 右电机B相下桥引脚复用功能
                        
#define MOTOR_RIGHT_C_PHASE_PORT_H         (GPIO_PRT5)                          // 右电机C相上桥控制引脚端口
#define MOTOR_RIGHT_C_PHASE_PIN_H          (0)                                  // 右电机C相上桥控制引脚号
#define MOTOR_RIGHT_C_PIN_HSIOM_H          (P5_0_TCPWM0_LINE9)                  // 右电机C相上桥引脚复用功能
                     
#define MOTOR_RIGHT_C_PHASE_PORT_L         (GPIO_PRT5)                          // 右电机C相下桥控制引脚端口
#define MOTOR_RIGHT_C_PHASE_PIN_L          (1)                                  // 右电机C相下桥控制引脚号
#define MOTOR_RIGHT_C_PIN_HSIOM_L          (P5_1_TCPWM0_LINE_COMPL9)            // 右电机C相下桥引脚复用功能
                                                                                
#define MOTOR_RIGHT_A_PHASE_TCPWM_TIMER    (TCPWM0_GRP0_CNT14)                  // 右电机A相定时器
#define MOTOR_RIGHT_A_PHASE_TCPWM_CLOCK    (PCLK_TCPWM0_CLOCKS14)               // 右电机A相定时器时钟通道
#define MOTOR_RIGHT_A_PHASE_TCPWM_ADDR     (0x40380700)                         // 右电机A相定时器通道地址(寄存器地址)
                                                                                
#define MOTOR_RIGHT_B_PHASE_TCPWM_TIMER    (TCPWM0_GRP0_CNT7)                   // 右电机B相定时器
#define MOTOR_RIGHT_B_PHASE_TCPWM_CLOCK    (PCLK_TCPWM0_CLOCKS7)                // 右电机B相定时器时钟通道
#define MOTOR_RIGHT_B_PHASE_TCPWM_ADDR     (0x40380380)                         // 右电机B相定时器通道地址(寄存器地址)
                                                                                
#define MOTOR_RIGHT_C_PHASE_TCPWM_TIMER    (TCPWM0_GRP0_CNT9)                   // 右电机C相定时器
#define MOTOR_RIGHT_C_PHASE_TCPWM_CLOCK    (PCLK_TCPWM0_CLOCKS9)                // 右电机C相定时器时钟通道
#define MOTOR_RIGHT_C_PHASE_TCPWM_ADDR     (0x40380480)                         // 右电机C相定时器通道地址(寄存器地址)
                                                                                
#define MOTOR_RIGHT_TCPWM_ISR_CHANNEL      (tcpwm_0_interrupts_14_IRQn)         // 以A相定时器通道更新中断作为整体更新中断        


void motor_left_phase_check     (uint16 delay_time);
void motor_left_duty_set        (uint16 a_duty, uint16 b_duty, uint16 c_duty);
void motor_left_channel_set     (uint8 a_channel, uint8 b_channel, uint8 c_channel);
void motor_left_output_init     (uint16 timer_period, uint8 isr_state);

void motor_right_phase_check    (uint16 delay_time);
void motor_right_duty_set       (uint16 a_duty, uint16 b_duty, uint16 c_duty);
void motor_right_channel_set    (uint8 a_channel, uint8 b_channel, uint8 c_channel);
void motor_right_output_init    (uint16 timer_period, uint8 isr_state);

void left_mos_q1q4_open         (uint16 duty);
void left_mos_q1q6_open         (uint16 duty);
void left_mos_q3q2_open         (uint16 duty);
void left_mos_q3q6_open         (uint16 duty);
void left_mos_q5q2_open         (uint16 duty);
void left_mos_q5q4_open         (uint16 duty);
void left_mos_close             (void);

void right_mos_q1q4_open        (uint16 duty);
void right_mos_q1q6_open        (uint16 duty);
void right_mos_q3q2_open        (uint16 duty);
void right_mos_q3q6_open        (uint16 duty);
void right_mos_q5q2_open        (uint16 duty);
void right_mos_q5q4_open        (uint16 duty);
void right_mos_close            (void);


#endif


