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
* 文件名称          cm7_0_isr
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT4BB
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-9      pudding            first version
* 2024-5-14     pudding            新增12个pit周期中断 增加部分注释说明
********************************************************************************************************************/

#include "zf_common_headfile.h"

// 声明外部函数

// 加上 volatile，告诉编译器这个变量会在中断中突变
extern volatile uint8 pit_state; 
extern volatile uint8 pit_state1; 
// 声明外部函数，确保编译器能找到 ekf.c 中的函数
extern void EKF_UpData(void);//卡尔曼滤波

volatile uint32_t control_tick = 0;        // 1ms计数器
extern volatile float pid_out_speed;  // 速度环输出（目标角度）
extern volatile float pid_out_angle;// 角度环输出（目标角速度）
extern volatile float pid_out_pwm;// 角速度环输出（PWM值）      
extern volatile float mechanical_zero_angle; // 机械零点（需校准）暂时未调用

volatile struct {
    float angle;
    float gyro;
    float speed;
} sensor_data = {0};

uint32_t loop_counter = 0;
// **************************** PIT中断函数 ****************************
void pit0_ch0_isr()                     // 定时器通道 0 周期中断服务函数      
{
    // 1. 清除中断标志位 (必须第一步做)
    pit_isr_flag_clear(PIT_CH0);
    loop_counter++;

    // ==========================================================
    // 步骤 2: 速度环(舵机控制) (20ms 跑一次)
    // ==========================================================
    if (loop_counter % 20 == 0)
    {
         // 2.1 获取编码器速度
        //small_driver_get_speed();//这句话应该不用，它只要调用一次，逐飞的库里写了
        float left_speed = (float)motor_value.receive_left_speed_data;
        float right_speed = (float)motor_value.receive_right_speed_data;
        float current_actual_speed = 0.5f * (right_speed - left_speed);

        // 2.2 调用速度控制器，计算 Duty Cycle 调整量
        float duty_adjustment = Servo_Speed_Control(target_speed_set, current_actual_speed);
        int32 duty_adjustment_val = (int32)duty_adjustment;

        // 2.3 计算基础姿态的 Duty
        high_control_table(INIT_HEIGHT);
        if (pwm_high != 10000)
        {
        int32 base_duty_lf = SERVO_MOTOR_PWM1_90 + pwm_high;
        int32 base_duty_rf = SERVO_MOTOR_PWM2_90 - pwm_high;
        int32 base_duty_rr = SERVO_MOTOR_PWM3_90 - pwm_high;
        int32 base_duty_lr = SERVO_MOTOR_PWM4_90 + pwm_high;
        
        // 2.4 计算最终 Duty
        // 模型: 前倾加速 = 前腿伸展 + 后腿收缩

        // 前腿伸展
        int32 duty_lf = base_duty_lf + duty_adjustment_val; // ++舵机, 伸展是加
        int32 duty_rf = base_duty_rf - duty_adjustment_val; // --舵机, 伸展是减

        // 后腿收缩
        int32 duty_rr = base_duty_rr + duty_adjustment_val; // --舵机, 收缩是加
        int32 duty_lr = base_duty_lr - duty_adjustment_val; // ++舵机, 收缩是减

        // 2.5 调用【新的】底层驱动函数，执行控制
        servo_write_duty(SERVO_MOTOR_PWM1, duty_lf); // 左前
        servo_write_duty(SERVO_MOTOR_PWM2, duty_rf); // 右前
        servo_write_duty(SERVO_MOTOR_PWM3, duty_rr); // 右后
        servo_write_duty(SERVO_MOTOR_PWM4, duty_lr); // 左后
        }
    }

    // ==========================================================
    // 步骤 3: 角度环 (5ms 跑一次)
    // ==========================================================
    if (loop_counter % 5 == 0)
    {
        // 运行姿态解算 (EKF / 互补滤波)
        EKF_UpData(); 
        now_angle = euler_angle.pitch; // 获取解算后的角度 (单位：度)

        // --- [调用优化] ---
        // 变化点：删除了第3个参数 "mechanical_zero"，因为它已经包含在 pid_angle.compensation 中了
        // 参数1: speed_loop_out (速度环算出来的目标角度)
        // 参数2: now_angle (当前角度)
        // 返回: angle_loop_out (期望的角速度)
        angle_loop_out = Angle_Loop_Control(speed_loop_out, now_angle);
    }

    // ==========================================================
    // 步骤 4: 角速度环 (1ms 跑一次，最内环)
    // ==========================================================
    
    // 4.1 获取原始陀螺仪数据
    imu660ra_get_gyro(); 
    int16 raw_gyro_y = -imu660ra_gyro_x; // 根据实际安装方向调整符号

    // 4.2 传感器底噪过滤 (这是为了防止静止时数值跳动，保留)
    float gyro_val = (float)raw_gyro_y;
    if (fabs(gyro_val) < 5.0f) gyro_val = 0;

    // 4.3 单位转换 [重要]
    // 既然 pid.c 里限幅是 3000 (这显然是度/秒或者LSB，不可能是弧度)，
    // 建议统一转换为 【度/秒 (deg/s)】。
    // 假设灵敏度是 16.384 LSB/(dps) (即±2000dps量程)
    float now_gyro_deg = gyro_val / 16.384f; 

    // 4.4 简单的低通滤波 (平滑噪声)
    now_gyro = 0.8f * now_gyro + 0.2f * now_gyro_deg;

    // --- [调用优化] ---
    // 变化点：移除了手动减零偏逻辑 (GYRO_SENSOR_OFFSET 宏在函数内处理)
    // 变化点：移除了手动死区补偿 (GYR_DEAD_ZONE 宏在函数内处理)
    // 参数1: angle_loop_out (角度环算出来的期望角速度)
    // 参数2: now_gyro (当前实际角速度)
    // 返回: gyro_loop_out (最终PWM)
    gyro_loop_out = Gyro_Loop_Control(angle_loop_out, now_gyro);


    // ==========================================================
    // 步骤 5: 安全保护 (倒地停止)
    // ==========================================================
    // 如果角度过大（例如超过 30 度），强制关闭电机
    if (now_angle > 30.0f || now_angle < -30.0f) 
    {
        gyro_loop_out = 0; // PWM置0        
        // 清除 PID 的所有参数，否则扶起来的瞬间电机还是全速旋转
        //PID_Data_Reset(); 
    }
//     if(g_motor_enable==0)
//    {
//        gyro_loop_out = 0;
//        PID_Data_Reset();
//    }
    // ==========================================================
    // 步骤 6: 电机输出
    // ==========================================================
    final_motor_pwm = gyro_loop_out; // 更新全局变量，方便调试查看
    int pwm_val = (int)final_motor_pwm;

    // 这里的限幅已经在 Gyro_Loop_Control 内部做了 (依靠 PWM_MAX_LIMIT 宏)
    // 直接输出即可
    small_driver_set_duty(-pwm_val, pwm_val); 

    // ==========================================================
    // 步骤 7: 系统心跳
    // ==========================================================
    if(loop_counter % 50 == 0) 
    {
        pit_state = 1; 
    }
    
}

void pit0_ch1_isr()                     // 定时器通道 1 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH1);

}

void pit0_ch2_isr()                     // 定时器通道 2 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH2);
    
}

void pit0_ch10_isr()                    // 定时器通道 10 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH10);
    
}

void pit0_ch11_isr()                    // 定时器通道 11 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH11);
    
}

void pit0_ch12_isr()                    // 定时器通道 12 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH12);
    
}

void pit0_ch13_isr()                    // 定时器通道 13 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH13);
    
}

void pit0_ch14_isr()                    // 定时器通道 14 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH14);
    
}

void pit0_ch15_isr()                    // 定时器通道 15 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH15);
    
}

void pit0_ch16_isr()                    // 定时器通道 16 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH16);
    
}

void pit0_ch17_isr()                    // 定时器通道 17 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH17);
    
}

void pit0_ch18_isr()                    // 定时器通道 18 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH18);
    
}

void pit0_ch19_isr()                    // 定时器通道 19 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH19);
    
}

void pit0_ch20_isr()                    // 定时器通道 20 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH20);
    
}

void pit0_ch21_isr()                    // 定时器通道 21 周期中断服务函数      
{
    pit_isr_flag_clear(PIT_CH21);
    tsl1401_collect_pit_handler();
}
// **************************** PIT中断函数 ****************************


// **************************** 外部中断函数 ****************************
void gpio_0_exti_isr()                  // 外部 GPIO_0 中断服务函数     
{
    
  
  
}

void gpio_1_exti_isr()                  // 外部 GPIO_1 中断服务函数     
{
    if(exti_flag_get(P01_0))		// 示例P1_0端口外部中断判断
    {

      
      
            
    }
    if(exti_flag_get(P01_1))
    {

            
            
    }
}

void gpio_2_exti_isr()                  // 外部 GPIO_2 中断服务函数     
{
    if(exti_flag_get(P02_0))
    {
            
            
    }
    if(exti_flag_get(P02_4))
    {
            
            
    }

}

void gpio_3_exti_isr()                  // 外部 GPIO_3 中断服务函数     
{



}

void gpio_4_exti_isr()                  // 外部 GPIO_4 中断服务函数     
{



}

void gpio_5_exti_isr()                  // 外部 GPIO_5 中断服务函数     
{



}

void gpio_6_exti_isr()                  // 外部 GPIO_6 中断服务函数     
{
	


}

void gpio_7_exti_isr()                  // 外部 GPIO_7 中断服务函数     
{



}

void gpio_8_exti_isr()                  // 外部 GPIO_8 中断服务函数     
{



}

void gpio_9_exti_isr()                  // 外部 GPIO_9 中断服务函数     
{



}

void gpio_10_exti_isr()                  // 外部 GPIO_10 中断服务函数     
{



}

void gpio_11_exti_isr()                  // 外部 GPIO_11 中断服务函数     
{



}

void gpio_12_exti_isr()                  // 外部 GPIO_12 中断服务函数     
{



}

void gpio_13_exti_isr()                  // 外部 GPIO_13 中断服务函数     
{



}

void gpio_14_exti_isr()                  // 外部 GPIO_14 中断服务函数     
{



}

void gpio_15_exti_isr()                  // 外部 GPIO_15 中断服务函数     
{



}

void gpio_16_exti_isr()                  // 外部 GPIO_16 中断服务函数     
{



}

void gpio_17_exti_isr()                  // 外部 GPIO_17 中断服务函数     
{



}

void gpio_18_exti_isr()                  // 外部 GPIO_18 中断服务函数     
{



}

void gpio_19_exti_isr()                  // 外部 GPIO_19 中断服务函数     
{



}

void gpio_20_exti_isr()                  // 外部 GPIO_20 中断服务函数     
{



}

void gpio_21_exti_isr()                  // 外部 GPIO_21 中断服务函数     
{



}

void gpio_22_exti_isr()                  // 外部 GPIO_22 中断服务函数     
{



}

void gpio_23_exti_isr()                  // 外部 GPIO_23 中断服务函数     
{



}
// **************************** 外部中断函数 ****************************

//// **************************** DMA中断函数 ****************************
//void dma_event_callback(void* callback_arg, cyhal_dma_event_t event)
//{
//    CY_UNUSED_PARAMETER(event);
//	
//
//	
//	
//}
// **************************** DMA中断函数 ****************************

void uart_rx_interrupt_handler (void);
// **************************** 串口中断函数 ****************************
// 串口0默认作为调试串口
void uart0_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_0)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口0接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_0), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
        uart_rx_interrupt_handler();
#if DEBUG_UART_USE_INTERRUPT                        				                // 如果开启 debug 串口中断
        debug_interrupr_handler();                  				                // 调用 debug 串口接收处理函数 数据会被 debug 环形缓冲区读取
#endif                                              				                // 如果修改了 DEBUG_UART_INDEX 那这段代码需要放到对应的串口中断去
      
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_0)) & CY_SCB_UART_TX_DONE)            // 串口0发送中断
    {           
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_0), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}


void uart1_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_1)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口1接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_1), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
      
        wireless_module_uart_handler();
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_1)) & CY_SCB_UART_TX_DONE)            // 串口1发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_1), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart2_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_2)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口2接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_2), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        gnss_uart_callback();
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_2)) & CY_SCB_UART_TX_DONE)            // 串口2发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_2), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart3_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_3)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口3接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_3), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_3)) & CY_SCB_UART_TX_DONE)            // 串口3发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_3), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}

void uart4_isr (void)
{
    
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_4)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口4接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_4), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位

        uart_control_callback();  
        uart_receiver_handler();                                                                // 串口接收机回调函数
        
                                                            
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_4)) & CY_SCB_UART_TX_DONE)            // 串口4发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_4), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}
// **************************** 串口中断函数 ****************************