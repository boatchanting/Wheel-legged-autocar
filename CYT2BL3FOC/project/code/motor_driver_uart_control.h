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
* 文件名称          motor_driver_uart_control
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

#ifndef _motor_driver_uart_control_h_
#define _motor_driver_uart_control_h_


#include "zf_common_headfile.h"


#define MOTOR_DRIVER_UART      UART_2 
#define MOTOR_DRIVER_TX        UART2_RX_P07_0 
#define MOTOR_DRIVER_RX        UART2_TX_P07_1 

typedef enum
{
    NULL_CMD,
    SET_DUTY,
    GET_SPEED,
    SET_ZERO,
    TEST_PHASE,
    GET_PHASE,
    SET_PHASE,
    GET_ENCODER,
    GET_ANGLE,
    GET_PHASE_DUTY,
    GET_VOLTAGE,
    ERROR_CMD
}control_mode_enum;

typedef enum
{
    BYTE_TYPE,
    STRING_TYPE
}control_cmd_type_enum;

typedef enum
{
    CMD_NULL            = 0,    // 无命令
    CMD_FORTHWITH       = 1,    // 即时命令 解析完成后马上响应
    CMD_LOOP            = 2,    // 循环命令 周期调用的查询响应
}refresh_state_enum;

typedef struct
{
      uint8                     send_data_buffer[7];
      
      uint8                     select_motor;
      
      int16                     receive_left_phase;
      int16                     receive_left_duty_data;

      int16                     receive_right_phase;
      int16                     receive_right_duty_data;
      
      control_mode_enum         immediate_command;
      control_mode_enum         continue_command;
      
      refresh_state_enum        refresh_flag;
      
      control_cmd_type_enum     cmd_type;
}small_device_value_struct;



extern small_device_value_struct motor_value;

void motor_driver_control_callback(small_device_value_struct *device_value);

void motor_driver_control_loop(small_device_value_struct *device_value);

void motor_driver_set_duty(small_device_value_struct *device_value, int left_duty, int right_duty);

void motor_driver_send_speed(small_device_value_struct *device_value, int left_speed, int right_speed);

void motor_driver_uart_init(void);

#endif
