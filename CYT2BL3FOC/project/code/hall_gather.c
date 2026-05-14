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
* 文件名称          hall_gather
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

#include "hall_gather.h"


// 霍尔硬件换相序列
int8 hall_hw_order[7]                  = {0, 6, 2, 3, 1, 5, 4};        

// 霍尔硬件换相序列位置转换 
// 例：采集霍尔值为4 带入位置转换数组得出当前位置在6 对应霍尔硬件换相序列的第6个数据 
//     当前位置为6 如果我们想要驱动电机旋转则可以输出下一个位置或者上一个位置 
//     输出上一个位置则是5 带入霍尔硬件换相序列得出第5个数据值为5 则输出霍尔为5的相位
//     输出下一个位置则是1 带入霍尔硬件换相序列得出第1个数据值为6 则输出霍尔为6的相位
int8 hall_hw_order_transition[7]       = {0, 4, 2, 3, 6, 5, 1};      

int32  left_change_time = 0;           // 定义左侧电机霍尔换向间隔时长

int32 right_change_time = 0;           // 定义右侧电机霍尔换向间隔时长


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔数据采集
// 参数说明     void
// 返回参数     void
// 使用示例     hall_gather(&motor_left);
// 备注信息     该函数执行耗时 6 us
//-------------------------------------------------------------------------------------------------------------------
void hall_gather(motor_struct *motor_value)
{
    uint8 hall_data_get[7] = {0};                                       // 定义霍尔采集数组
    
    uint8 hall_data = 0;                                                // 定义霍尔数据变量

    motor_value->hall_value_last = motor_value->hall_value_now;         // 保存上一次的霍尔数据
     
    if(motor_value->motor_type == LEFT_MOTOR)                           // 判断是否为左侧电机
    {
        for(int i = 0; i < 5; i ++)                                     // 连续采集5次霍尔
        {
            hall_data = gpio_get_level(MOTOR_LEFT_HALL_A_TRIG) * 1 + gpio_get_level(MOTOR_LEFT_HALL_B_TRIG) * 2 + gpio_get_level(MOTOR_LEFT_HALL_C_TRIG) * 4;
            
            hall_data_get[hall_data] ++;
        }
        
        hall_data = 0;                                                  // 霍尔数据清零
        
        for(int i = 0; i < 7; i ++)                                     // 获取采集最多次数的霍尔值
        {                                                               // 此处操作主要是通过多次采样滤波 防止采集到偶尔的噪声数据
            if(hall_data < hall_data_get[i])
            {
                hall_data = hall_data_get[i];
                
                motor_value->hall_value_now = i;
            }
        }
    }
    else                                                                // 否则为右侧电机
    {
        for(int i = 0; i < 5; i ++)                                     // 连续采集5次霍尔
        {
            hall_data = gpio_get_level(MOTOR_RIGHT_HALL_A_TRIG) * 1 + gpio_get_level(MOTOR_RIGHT_HALL_B_TRIG) * 2 + gpio_get_level(MOTOR_RIGHT_HALL_C_TRIG) * 4;
            
            hall_data_get[hall_data] ++;
        }
        
        hall_data = 0;                                                  // 霍尔数据清零
                                                                    
        for(int i = 0; i < 7; i ++)                                     // 获取采集最多次数的霍尔值 
        {                                                               // 此处操作主要是通过多次采样滤波 防止采集到偶尔的噪声数据
            if(hall_data < hall_data_get[i])
            {
                hall_data = hall_data_get[i];
                
                motor_value->hall_value_now = i;
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 计算电机旋转方向
// 参数说明     hall_now_location     当前的霍尔位置
// 参数说明     hall_last_location    之前的霍尔位置
// 返回参数     motor_dir_enum        电机旋转位置  
// 使用示例     motor_left.motor_now_dir = motor_direction_calculate(hall_hw_order_transition[motor_left.hall_value_now], hall_hw_order_transition[motor_left.hall_value_last]);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
motor_dir_enum motor_direction_calculate(motor_struct *motor_value)
{
    int8 hall_now_location;
    
    int8 hall_last_location;
    
    int8 temp;
    
    hall_now_location = hall_hw_order_transition[motor_value->hall_value_now];
    
    hall_last_location = hall_hw_order_transition[motor_value->hall_value_last];
    
    temp = hall_now_location - hall_last_location;      
    
    if(func_abs(temp) > 3)
    {
        if(temp > 3)
        {
            temp -= 6;
        }
        else
        {
            temp += 6;
        }
    }

    return (temp > 0 ? FORWARD : REVERSE);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 左侧电机判断输出
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_left_hall_output();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_hall_output(void)
{
    int8  hall_output_temp = 0;
    int16 output_duty_temp = 0;
    
    if(motor_left.motor_duty >= 0)                                      // 设置电机占空比大于0时 执行正转偏移
    {
       // 加上实际霍尔的位置 可以得出需要输出的霍尔位置是多少
       hall_output_temp = hall_hw_order_transition[motor_left.hall_value_now] + 1;
       
       if(6 < hall_output_temp)                                              // 当输出超过最大位置时 减去最大位置实现循环
       {
            hall_output_temp -= 6; 
       }
    }
    else                                                                // 设置电机反转时执行反转的偏移方向
    {
       // 减去实际霍尔的位置 可以得出需要输出的霍尔位置是多少
       hall_output_temp = hall_hw_order_transition[motor_left.hall_value_now] - 2;
    
       if(1 > hall_output_temp)
       {
          hall_output_temp += 6;
       }
    }
    
    hall_output_temp = hall_hw_order[hall_output_temp];                        // 获取对应位置的霍尔编码（上面是位置偏移计算 计算完成应该根据位置查询正确相位）
    
    if(motor_left.motor_protect_state      == PROTECT_MODE         || 
      (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))
    {
        output_duty_temp = 0;                                          // 保护状态输出0占空比 刹车
    }
    else
    {
        output_duty_temp = func_abs((int16)(motor_left.motor_duty * OUTPUT_DUTY_MAX));
    }
    
#if BLDC_DEBUG_OUTPUT_PHASE != 0
    hall_output_temp = BLDC_DEBUG_OUTPUT_PHASE;
    
    output_duty_temp = func_limit_ab(output_duty_temp, 0, OUTPUT_DUTY_MAX / 10);
#endif
    
    output_duty_temp = func_limit_ab(output_duty_temp, 0, OUTPUT_DUTY_MAX * 99 / 100);
    
    if(output_duty_temp == 0 && BLDC_GLIDING_MODE == DRIVER_ENABLE)
    {
        left_mos_close();                                                       // 占空比给 0 则滑行
    }
    else
    {
        switch(hall_output_temp)                                                // 根据计算好的输出相位调用对应的输出函数 使电机旋转至指定位置
        {
            case 1:     left_mos_q3q2_open((uint16)output_duty_temp);           break;	// 1 
            case 2:     left_mos_q5q4_open((uint16)output_duty_temp);           break;	// 2  
            case 3:     left_mos_q5q2_open((uint16)output_duty_temp);           break;	// 3 
            case 4:     left_mos_q1q6_open((uint16)output_duty_temp);           break;	// 4
            case 5:     left_mos_q3q6_open((uint16)output_duty_temp);           break;	// 5 
            case 6:     left_mos_q1q4_open((uint16)output_duty_temp);           break;	// 6   
            default:    left_mos_close();			                break;  
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 右侧电机判断输出
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_left_hall_output();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_hall_output(void)
{
    int8  hall_output_temp = 0;
    int16 output_duty_temp = 0;
    
    if(motor_right.motor_duty >= 0)                                      // 设置电机占空比大于0时 执行正转偏移
    {
         // 加上实际霍尔的位置 可以得出需要输出的霍尔位置是多少
         hall_output_temp = hall_hw_order_transition[motor_right.hall_value_now] + 1;
         
         if(6 < hall_output_temp)                                              // 当输出超过最大位置时 减去最大位置实现循环
         {
              hall_output_temp -= 6; 
         }
         
         hall_output_temp = hall_hw_order[hall_output_temp];                        // 获取对应位置的霍尔编码（上面是位置偏移计算 计算完成应该根据位置查询正确相位）
    }
    else                                                                // 设置电机反转时执行反转的偏移方向
    {
         // 减去实际霍尔的位置 可以得出需要输出的霍尔位置是多少
         hall_output_temp = hall_hw_order_transition[motor_right.hall_value_now] - 2;
      
         if(1 > hall_output_temp)
         {
            hall_output_temp += 6;
         }
         hall_output_temp = hall_hw_order[hall_output_temp];  
    }
    
    if(motor_right.motor_protect_state      == PROTECT_MODE         || 
      (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))
    {
        output_duty_temp = 0;                                          // 保护状态输出0占空比 刹车
    }
    else
    {
        output_duty_temp = func_abs((int16)(motor_right.motor_duty * OUTPUT_DUTY_MAX));
    }
    
#if BLDC_DEBUG_OUTPUT_PHASE != 0
    hall_output_temp = BLDC_DEBUG_OUTPUT_PHASE;
    
    output_duty_temp = func_limit_ab(output_duty_temp, 0, OUTPUT_DUTY_MAX / 10);
#endif
    
    output_duty_temp = func_limit_ab(output_duty_temp, 0, OUTPUT_DUTY_MAX - 1);
    
    if(output_duty_temp == 0 && BLDC_GLIDING_MODE == DRIVER_ENABLE)
    {
        right_mos_close();                                                       // 占空比给 0 则滑行
    }
    else
    {
        switch(hall_output_temp)                                                // 根据计算好的输出相位调用对应的输出函数 使电机旋转至指定位置
        {
            case 1:     right_mos_q3q2_open((uint16)output_duty_temp);          break;	// 1 
            case 2:     right_mos_q5q4_open((uint16)output_duty_temp);          break;	// 2 
            case 3:     right_mos_q5q2_open((uint16)output_duty_temp);          break;	// 3
            case 4:     right_mos_q1q6_open((uint16)output_duty_temp);          break;	// 4
            case 5:     right_mos_q3q6_open((uint16)output_duty_temp);          break;	// 5 
            case 6:     right_mos_q1q4_open((uint16)output_duty_temp);          break;	// 6  
            default:    right_mos_close();			                break;  
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧电机 霍尔触发函数
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_left_hall_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_hall_isr(void)
{
    uint8 temp_hall_data = 0;
    
    exti_flag_get(MOTOR_LEFT_HALL_A_TRIG);                                      // 清除 A相 触发信号
        
    exti_flag_get(MOTOR_LEFT_HALL_B_TRIG);                                      // 清除 B相 触发信号
        
    exti_flag_get(MOTOR_LEFT_HALL_C_TRIG);                                      // 清除 C相 触发信号
    
    motor_left.isr_trig_count ++;
    
    hall_gather(&motor_left);                                                   // 采集霍尔数据
    
    if(motor_left.hall_value_now != motor_left.hall_value_last)                 // 判断是否换相
    {
        motor_left.step_change_time_us = timer_get(LEFT_MOTOR_COUNT_TIMER);     // 获取换相间隔时长
        
        timer_clear(LEFT_MOTOR_COUNT_TIMER);                                    // 清除定时器计数值
        
        motor_left.run_state = MOTOR_RUN;                                       // 当前成功换相 电机状态设置为运行
        
        temp_hall_data = hall_hw_order_transition[motor_left.hall_value_now] - 1;               // 计算数组索引
            
        motor_left.commutation_time_sum -= motor_left.commutation_time[temp_hall_data];         // 减去上次 此相位的换相时间
        
        motor_left.commutation_time[temp_hall_data] = motor_left.step_change_time_us;           // 保存本次 此相位的换相时间
        
        motor_left.commutation_time_sum += motor_left.commutation_time[temp_hall_data];         // 累加到最近6次的换相总时间
        
        motor_left.motor_speed = 8571428 / (float)motor_left.commutation_time_sum * motor_left.motor_now_dir;     // 计算电机转速
        
        motor_left.motor_now_dir = motor_direction_calculate(&motor_left);      // 计算运转方向
        
        motor_left_hall_output();                                               // 正常换相输出
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 左侧电机 1khz 周期回调函数
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_left_hall_callback();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_hall_callback(void)
{
    static uint32 left_protect_count = 0;
    
    if(timer_get(LEFT_MOTOR_COUNT_TIMER) > 10 * 1000)                           // 如果换相间隔大于10ms
    {
        hall_gather(&motor_left);                                               // 采集霍尔数据
        
        motor_left_hall_output();                                               // 主动换相输出

        motor_left.run_state = MOTOR_STOP;                                      // 电机运行状态更改为停止
    
        motor_left.motor_speed = 0;                                             // 速度数据归零
            
        timer_clear(LEFT_MOTOR_COUNT_TIMER);                                    // 清除定时器计数值
    }
    
    motor_left.motor_speed_filter = (motor_left.motor_speed_filter * 19 + motor_left.motor_speed) / 20; // 转速滤波
    
    if(motor_left.locked_value.protect_flag)
    {
        if(func_abs(motor_left.motor_speed_filter) < 10.0f && func_abs(motor_left.motor_duty) >= motor_left.locked_value.protect_duty_max)  // 判断是否堵转  占空比大于10%并且无转速数据持续 500ms
        {
            left_protect_count ++;
            
            if(left_protect_count > motor_left.locked_value.protect_check_time)
            {
                left_protect_count = motor_left.locked_value.protect_check_time;
                
                motor_left.motor_protect_state = PROTECT_MODE;                  // 条件成立  进入输出保护状态
            }
        }
        else
        {
            left_protect_count = 0;                                             // 否则清空保护计次
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧电机 霍尔触发函数
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_right_hall_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_hall_isr(void)
{
    uint8 temp_hall_data = 0;
    
    exti_flag_get(MOTOR_RIGHT_HALL_A_TRIG);                                     // 清除 A相 触发信号
        
    exti_flag_get(MOTOR_RIGHT_HALL_B_TRIG);                                     // 清除 B相 触发信号
        
    exti_flag_get(MOTOR_RIGHT_HALL_C_TRIG);                                     // 清除 C相 触发信号
    
    motor_right.isr_trig_count ++;
    
    hall_gather(&motor_right);                                                  // 采集霍尔数据
    
    if(motor_right.hall_value_now != motor_right.hall_value_last)               // 判断是否换相
    {
        motor_right.step_change_time_us = timer_get(RIGHT_MOTOR_COUNT_TIMER);   // 获取换相间隔时长
        
        timer_clear(RIGHT_MOTOR_COUNT_TIMER);                                   // 清除定时器计数值
        
        motor_right.run_state = MOTOR_RUN;                                      // 当前成功换相 电机状态设置为运行
        
        temp_hall_data = hall_hw_order_transition[motor_right.hall_value_now] - 1;              // 计算数组索引
            
        motor_right.commutation_time_sum -= motor_right.commutation_time[temp_hall_data];       // 减去上次 此相位的换相时间
        
        motor_right.commutation_time[temp_hall_data] = motor_right.step_change_time_us;         // 保存本次 此相位的换相时间
        
        motor_right.commutation_time_sum += motor_right.commutation_time[temp_hall_data];       // 累加到最近6次的换相总时间
        
        motor_right.motor_speed = 8571428 / (float)motor_right.commutation_time_sum * motor_right.motor_now_dir;     // 计算电机转速
        
        motor_right.motor_now_dir = motor_direction_calculate(&motor_right);    // 计算运转方向
        
        motor_right_hall_output();                                              // 正常换相输出
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 右侧电机 1khz 周期回调函数
// 参数说明     void
// 返回参数     void 
// 使用示例     motor_right_hall_callback();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_hall_callback(void)
{
    static uint32 right_protect_count = 0;                                      // 定义右侧电机堵转保护计次
    
    if(timer_get(RIGHT_MOTOR_COUNT_TIMER) > 10 * 1000)                          // 如果换相间隔大于10ms
    {
        hall_gather(&motor_right);                                              // 采集霍尔数据
        
        motor_right_hall_output();                                              // 主动换相输出
        
        motor_right.run_state = MOTOR_STOP;                                     // 电机运行状态更改为停止
    
        motor_right.motor_speed = 0;                                            // 速度数据归零
            
        timer_clear(RIGHT_MOTOR_COUNT_TIMER);                                   // 清除定时器计数值
    }
    
    motor_right.motor_speed_filter = (motor_right.motor_speed_filter * 19 + motor_right.motor_speed) / 20; // 转速滤波
    
    if(motor_right.locked_value.protect_flag)
    {
        if(func_abs(motor_right.motor_speed_filter) < 10.0f && func_abs(motor_right.motor_duty) >= motor_right.locked_value.protect_duty_max)  // 判断是否堵转  占空比大于10%并且无转速数据持续 500ms
        {
            right_protect_count ++;
            
            if(right_protect_count > motor_right.locked_value.protect_check_time)
            {
                right_protect_count = motor_right.locked_value.protect_check_time;
                
                motor_right.motor_protect_state = PROTECT_MODE;                  // 条件成立  进入输出保护状态
            }
        }
        else
        {
            right_protect_count = 0;                                             // 否则清空保护计次
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     霍尔 采集初始化
// 参数说明     void
// 返回参数     void 
// 使用示例     hall_gather_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void hall_gather_init(void)
{         
    cy_stc_gpio_pin_config_t            exti_pin_cfg    = {0};                                                  // 定义霍尔信号触发引脚对象
                    
    cy_stc_sysint_irq_t                 exti_irq_cfg;                                                           // 定义霍尔信号触发中断对象
                    
    /*************************配置 左侧 霍尔信号信号触发*************************/                                  
    exti_pin_cfg.driveMode              = CY_GPIO_DM_HIGHZ;                                                     // IO配置为高阻态
                        
    exti_pin_cfg.intEdge                = EXTI_TRIGGER_BOTH;                                                    // 触发方式为双边触发
                        
    exti_pin_cfg.intMask                = 1ul;                                                                  // 触发使能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_HALL_A_TRIG), (MOTOR_LEFT_HALL_A_TRIG % 8), &exti_pin_cfg);            // 配置 A相 霍尔信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_HALL_B_TRIG), (MOTOR_LEFT_HALL_B_TRIG % 8), &exti_pin_cfg);            // 配置 B相 霍尔信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_HALL_C_TRIG), (MOTOR_LEFT_HALL_C_TRIG % 8), &exti_pin_cfg);            // 配置 C相 霍尔信号 IO功能
    
    exti_irq_cfg.intIdx     = CPUIntIdx5_IRQn;                                                                  // 选择中断号
        
    exti_irq_cfg.isEnabled  = true;                                                                             // 中断使能
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_HALL_A_TRIG / 8);         // 选择 A相 霍尔信号 中断外设
    
    interrupt_init(&exti_irq_cfg, motor_left_hall_isr, 0);                                                      // 中断初始化 优先级 0  （最高优先级）
  
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_HALL_B_TRIG / 8);         // 选择 B相 霍尔信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, motor_left_hall_isr, 0);                                                      // 中断初始化 优先级 0  （最高优先级）
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_HALL_C_TRIG / 8);         // 选择 C相 霍尔信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, motor_left_hall_isr, 0);                                                      // 中断初始化 优先级 0  （最高优先级）
    
    exti_enable(MOTOR_LEFT_HALL_A_TRIG);                                                                        // 使能 A相 霍尔信号 触发中断
                                                                                                                        
    exti_enable(MOTOR_LEFT_HALL_B_TRIG);                                                                        // 使能 B相 霍尔信号 触发中断
                                                                                                                        
    exti_enable(MOTOR_LEFT_HALL_C_TRIG);                                                                        // 使能 C相 霍尔信号 触发中断
    /*************************配置 左侧 霍尔信号信号触发*************************/
    
    
    /***************************配置 左侧 定时器及中断***************************/
    timer_init(LEFT_MOTOR_COUNT_TIMER, TIMER_US);                               // 初始化 左侧电机计数定时器
            
    timer_start(LEFT_MOTOR_COUNT_TIMER);                                        // 启动定时器
    
    pit_us_init(LEFT_MOTOR_PIT_TIMER, 1000);                                    // 初始化 左侧电机 轮询周期中断    
    /***************************配置 左侧 定时器及中断***************************/
    
    
    /*************************配置 右侧 霍尔信号信号触发*************************/                                  
    exti_pin_cfg.driveMode              = CY_GPIO_DM_HIGHZ;                                                     // IO配置为高阻态
                        
    exti_pin_cfg.intEdge                = EXTI_TRIGGER_BOTH;                                                    // 触发方式为双边触发
                        
    exti_pin_cfg.intMask                = 1ul;                                                                  // 触发使能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_HALL_A_TRIG), (MOTOR_RIGHT_HALL_A_TRIG % 8), &exti_pin_cfg);          // 配置 A相 霍尔信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_HALL_B_TRIG), (MOTOR_RIGHT_HALL_B_TRIG % 8), &exti_pin_cfg);          // 配置 B相 霍尔信号 IO功能
        
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_HALL_C_TRIG), (MOTOR_RIGHT_HALL_C_TRIG % 8), &exti_pin_cfg);          // 配置 C相 霍尔信号 IO功能
    
    exti_irq_cfg.intIdx     = CPUIntIdx6_IRQn;                                                                  // 选择中断号
        
    exti_irq_cfg.isEnabled  = true;                                                                             // 中断使能
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_HALL_A_TRIG / 8);        // 选择 A相 霍尔信号 中断外设
    
    interrupt_init(&exti_irq_cfg, motor_right_hall_isr, 0);                                                     // 中断初始化 优先级 0  （最高优先级）
  
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_HALL_B_TRIG / 8);        // 选择 B相 霍尔信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, motor_right_hall_isr, 0);                                                     // 中断初始化 优先级 0  （最高优先级）
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_HALL_C_TRIG / 8);        // 选择 C相 霍尔信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, motor_right_hall_isr, 0);                                                     // 中断初始化 优先级 0  （最高优先级）
    
    exti_enable(MOTOR_RIGHT_HALL_A_TRIG);                                                                       // 使能 A相 霍尔信号 触发中断
                                                                                                                        
    exti_enable(MOTOR_RIGHT_HALL_B_TRIG);                                                                       // 使能 B相 霍尔信号 触发中断
                                                                                                                        
    exti_enable(MOTOR_RIGHT_HALL_C_TRIG);                                                                       // 使能 C相 霍尔信号 触发中断
    /*************************配置 右侧 霍尔信号信号触发*************************/
    
    /***************************配置 右侧 定时器及中断***************************/
    timer_init(RIGHT_MOTOR_COUNT_TIMER, TIMER_US);                              // 初始化 右侧电机计数定时器
            
    timer_start(RIGHT_MOTOR_COUNT_TIMER);                                       // 启动定时器
    
    pit_us_init(RIGHT_MOTOR_PIT_TIMER, 1000);                                   // 初始化 右侧电机 轮询周期中断    
    /***************************配置 右侧 定时器及中断***************************/
}   












