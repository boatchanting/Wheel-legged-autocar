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


// **************************** PIT中断函数 ****************************
void pit0_ch0_isr()                     // 定时器通道 0 周期中断服务函数 (默认每1ms进入一次)
{
    // ------------------------------------------------------------------
    // 1. 基础系统维护
    // ------------------------------------------------------------------
    pit_isr_flag_clear(PIT_CH0); // 清除中断标志位。这是必须的操作，否则无法触发下一次中断

    // TC系列特有的 interrupt_global_enable(0) 在这里不需要，ARM Cortex-M7 有硬件中断优先级管理

    system_count ++; // 系统时间戳计数器，每1ms加1，用于后续的时间分片调度

    // ------------------ 搬运原本的逻辑开始 ------------------

    //    if(system_count%8000==0)
    //       target_v=0; // (已注释) 原逻辑：每8秒将目标速度归零，可能是停车调试用

    // ------------------------------------------------------------------
    // 2. 调试功能：周期性跳跃动作
    // ------------------------------------------------------------------
    // 注意：jump_tf 如果未定义会报错，请确认它在哪里定义的
    if(jump_tf != 0)      // 判断跳跃调试开关是否开启
       count++;           // 如果开启，计数器count随时间累加 (每1ms +1)
    else
       count=0;           // 如果关闭，计数器清零，确保下次开启时从头计时

    if (count>=10000)     // 当计数器累加到 10000 (即 10000ms = 10秒)
    {
       jump_flag=2;       // 触发跳跃标志位，主循环或控制函数检测到此标志会执行跳跃动作
       count=0;           // 计数器清零，开始下一个10秒的计时循环
    }

    // ------------------------------------------------------------------
    // 3. 核心控制逻辑调用
    // ------------------------------------------------------------------
    if (run_flag == 1)  // 判断系统运行标志位是否开启 ( && baohu_flag==0 可能是保护逻辑)
    {
        yuansu_control(); // 调用主要控制函数（元素控制），处理传感器融合、状态机等上层逻辑
    }

    // ------------------------------------------------------------------
    // 4. 分频任务调度 (利用 system_count 取模实现不同频率的控制)
    // ------------------------------------------------------------------
    
    // --- 速度环控制 (频率: 50Hz, 周期: 20ms) ---
    if (system_count % 20 == 0) 
    {
        // 注意：avg_speed, motor_value 可以在这里报错，需要 extern 或者包含头文件
        // 级联PID速度环：输入为当前平均速度和目标速度，输出用于舵机控制的PWM值
        // 通常是用于通过舵机调整重心或转向来间接控制速度
        PWM1S = CascadePID_speed_S(avg_speed, (int16)target_v);
        
        servo_control_speed(PWM1S); // 将计算出的PWM值赋给舵机接口
    }

    // --- 转向角度环控制 (频率: ~166Hz, 周期: 6ms) ---
    if (system_count % 6 == 0) 
    {
        // 级联PID转向角度环：输入为当前偏差角度(err_degree)，输出为转向控制量
        // 这个输出通常作为后续陀螺仪环的参考输入
        PWM_angle = CascadePID_Turn_Angle(err_degree);
    }

    // --- 姿态解算与直立环控制 (频率: 200Hz, 周期: 5ms) ---
    if (system_count % 5 == 0) 
    {
        EKF_UpData(); // 扩展卡尔曼滤波数据更新。利用加速度计和陀螺仪数据融合，解算出准确的姿态角
        
        // 提取并处理欧拉角数据
        // euler_angle 结构体需要定义，通常包含 roll(横滚), pitch(俯仰), yaw(偏航)
        degree_z = euler_angle.roll; // 获取横滚角
        degree_z = degree_z + (degree_z>0 ? -180 : 180); // 角度坐标变换，可能为了适应控制算法的坐标系
        degree_y = euler_angle.yaw + 180; // 偏航角处理
        degree_x = euler_angle.pitch-5.5;  // 俯仰角处理，减去5.5可能是机械安装零点误差补偿

        Now_Speed_Filter(); // 读取编码器数值并进行低通滤波，得到平滑的当前速度

        // 级联PID直立角度环：输入为当前横滚角、机械零点、期望角度(0)
        // 输出 PWM2 用于控制电机维持直立
        PWM2 = CascadePID_angle(degree_z, zero_point, 0);

        // 特殊路况处理 (如坡道/起伏路)
        if (high_flag && (danbianqiao_flag==0 || danbianqiao_flag==99))
        {
            // 级联PID特殊角度控制：根据俯仰角(degree_x)和目标角度计算补偿量
            // 注意：C语言大小写敏感，这里使用了小写的 pwm_angle 进行累加，需要确认变量名定义
            pwm_angle += CascadePID_Rolling_Angle(degree_x, rolling_target_degree);   
            
            // 限幅处理，防止PWM值超出执行器范围
            if (pwm_angle>1000) pwm_angle=1000;
            else if (pwm_angle<-1000) pwm_angle=-1000;
        }
    }

    // --- 陀螺仪环控制 (频率: 500Hz, 周期: 2ms) ---
    if (system_count % 2 == 0) 
    {
        imu660ra_get_gyro(); // ！！重点！！：从IMU传感器读取陀螺仪原始角速度数据
        // 级联PID转向陀螺仪环：输入为Z轴角速度(取负)和角度环输出PWM_angle(取负)
        // 这构成了串级PID：外环是角度，内环是角速度，提高转向的响应速度和稳定性
        PWM_gyro = CascadePID_Turn_Gyro(-imu660ra_gyro_z, -PWM_angle);
    }

    // --- 直立陀螺仪环控制 (频率: 1000Hz, 周期: 1ms) ---
    // 这是最内层的控制环，响应最快，直接决定平衡的稳定性
    PWM3 = CascadePID_Gyro(-imu660ra_gyro_x, PWM2); // 输入为X轴角速度(取负)和直立环输出PWM2
    PWM3 = (int16)PWM3; // 类型转换，确保输出格式正确
    
    // ------------------ 搬运原本的逻辑结束 ------------------
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

// **************************** 串口中断函数 ****************************
// 串口0默认作为调试串口
void uart0_isr (void)
{
    if(Cy_SCB_GetRxInterruptMask(get_scb_module(UART_0)) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口0接收中断
    {
        Cy_SCB_ClearRxInterrupt(get_scb_module(UART_0), CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
        
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

        
        uart_receiver_handler();                                                                // 串口接收机回调函数
        
        
    }
    else if(Cy_SCB_GetTxInterruptMask(get_scb_module(UART_4)) & CY_SCB_UART_TX_DONE)            // 串口4发送中断
    {
        Cy_SCB_ClearTxInterrupt(get_scb_module(UART_4), CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        
        
    }
}
// **************************** 串口中断函数 ****************************