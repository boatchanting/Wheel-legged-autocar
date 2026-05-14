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
#include "user_pwm_in.h"

user_in_struct motor_left_pwm_in_data;
user_in_struct motor_right_pwm_in_data;


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧 pwm脉宽捕获 超时 中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_left_pwm_in_over_time_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_pwm_in_over_time_isr(void)
{
    if(Cy_Tcpwm_Counter_GetTC_IntrMasked(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER))     // 检查是否是捕获超时中断标志位
    {
        Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER);        // 清除捕获中断标志位
      
        motor_left_pwm_in_data.pwm_high_level = 0;                              // 捕获超时则认为关闭输出
        
        motor_left_pwm_in_data.pwm_low_level = 0;
        
        motor_left_pwm_in_data.pwm_period_num = 0;
          
        motor_left_pwm_in_data.get_duty = 0;                                   
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧 pwm脉宽捕获中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_left_pwm_in_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_pwm_in_isr(void)
{
    if(Cy_Tcpwm_Counter_GetCC0_IntrMasked(MOTOR_LEFT_PWM_IN_TIMER))             // 比较器0触发中断 代表检测到下降沿 
    {
        Cy_Tcpwm_Counter_ClearCC0_Intr(MOTOR_LEFT_PWM_IN_TIMER);                // 清除比较器0触发中断 
    }
    
    if(Cy_Tcpwm_Counter_GetCC1_IntrMasked(MOTOR_LEFT_PWM_IN_TIMER))             // 比较器1触发中断 代表检测到上升沿 
    {
        Cy_Tcpwm_Counter_ClearCC1_Intr(MOTOR_LEFT_PWM_IN_TIMER);                // 清除比较器1触发中断 
        
        motor_left_pwm_in_data.pwm_high_level = Cy_Tcpwm_Counter_GetCompare0(MOTOR_LEFT_PWM_IN_TIMER) + 2;      // 获取比较器0保存的计数值(检测到下降沿则说明保存的是高电平时长) 加2是因为后面清除计数额外用了2us
        
        motor_left_pwm_in_data.pwm_period_num = Cy_Tcpwm_Counter_GetCompare1(MOTOR_LEFT_PWM_IN_TIMER) + 2;      // 获取比较器1保存的计数值(检测到上升沿则说明保存的是脉冲周期时长) 加2是因为后面清除计数额外用了2us
        
        Cy_Tcpwm_Counter_SetCounter(MOTOR_LEFT_PWM_IN_TIMER, 0);                // 清除捕获定时器计数寄存器值
        
        Cy_Tcpwm_Counter_SetCounter(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER, 0);       // 清除捕获超时定时器计数器值(运行到这里表示正确捕获到脉冲信号 超时则需要重新计时)
        
        if(motor_left_pwm_in_data.pwm_high_level > motor_left_pwm_in_data.pwm_period_num) motor_left_pwm_in_data.pwm_high_level = 0;
        
        if(motor_left_pwm_in_data.pwm_period_num > 25000 || motor_left_pwm_in_data.pwm_period_num < 3000)      // 要求控制脉冲必须在 40Hz ~ 300Hz 之间
        {
            motor_left_pwm_in_data.pwm_high_level = 0;
            
            motor_left_pwm_in_data.pwm_period_num = 0;
            
            motor_left_pwm_in_data.get_duty = 0;
        }
        else
        {
            if((motor_left_pwm_in_data.pwm_high_level % 10) >= 7 )                                              // 输入脉冲的毛刺、脉宽波动优化
            {
                motor_left_pwm_in_data.pwm_high_level = (motor_left_pwm_in_data.pwm_high_level / 10 + 1) * 10;  
            }
            else if((motor_left_pwm_in_data.pwm_high_level % 10) <= 3)
            {
                motor_left_pwm_in_data.pwm_high_level = (motor_left_pwm_in_data.pwm_high_level / 10) * 10;
            }
          
            motor_left_pwm_in_data.pwm_low_level = motor_left_pwm_in_data.pwm_period_num - motor_left_pwm_in_data.pwm_high_level;                          // 计算低电平时长(低电平时长并没有用到，可以作为数据观察)
            
            motor_left_pwm_in_data.get_duty = (float)((float)motor_left_pwm_in_data.pwm_high_level / (float)motor_left_pwm_in_data.pwm_period_num);       // 计算占空比 范围 0~1 浮点类型
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧 pwm脉宽捕获 超时 中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_right_pwm_in_over_time_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_pwm_in_over_time_isr(void)
{
    if(Cy_Tcpwm_Counter_GetTC_IntrMasked(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER))    // 检查是否是捕获超时中断标志位
    {
        Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER);       // 清除捕获中断标志位
      
        motor_right_pwm_in_data.pwm_high_level = 0;                             // 捕获超时则认为关闭输出
        
        motor_right_pwm_in_data.pwm_low_level = 0;
        
        motor_right_pwm_in_data.pwm_period_num = 0;
          
        motor_right_pwm_in_data.get_duty = 0;  
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧 pwm脉宽捕获中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_right_pwm_in_isr();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_pwm_in_isr(void)
{
    if(Cy_Tcpwm_Counter_GetCC0_IntrMasked(MOTOR_RIGHT_PWM_IN_TIMER))            // 比较器0触发中断 代表检测到下降沿 
    {
        Cy_Tcpwm_Counter_ClearCC0_Intr(MOTOR_RIGHT_PWM_IN_TIMER);               // 清除比较器0触发中断 
    }
    

    if(Cy_Tcpwm_Counter_GetCC1_IntrMasked(MOTOR_RIGHT_PWM_IN_TIMER))            // 比较器1触发中断 代表检测到上升沿 
    {
        Cy_Tcpwm_Counter_ClearCC1_Intr(MOTOR_RIGHT_PWM_IN_TIMER);               // 清除比较器1触发中断 
        
        motor_right_pwm_in_data.pwm_high_level = Cy_Tcpwm_Counter_GetCompare0(MOTOR_RIGHT_PWM_IN_TIMER) + 2;    // 获取比较器0保存的计数值(检测到下降沿则说明保存的是高电平时长) 加2是因为后面清除计数额外用了2us
          
        motor_right_pwm_in_data.pwm_period_num = Cy_Tcpwm_Counter_GetCompare1(MOTOR_RIGHT_PWM_IN_TIMER) + 2;    // 获取比较器1保存的计数值(检测到上升沿则说明保存的是脉冲周期时长) 加2是因为后面清除计数额外用了2us
        
        Cy_Tcpwm_Counter_SetCounter(MOTOR_RIGHT_PWM_IN_TIMER, 0);               // 清除捕获定时器计数寄存器值
        
        Cy_Tcpwm_Counter_SetCounter(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER, 0);      // 清除捕获超时定时器计数器值(运行到这里表示正确捕获到脉冲信号 超时则需要重新计时)
        
        if(motor_right_pwm_in_data.pwm_high_level > motor_right_pwm_in_data.pwm_period_num) motor_right_pwm_in_data.pwm_high_level = 0;
        
        if(motor_right_pwm_in_data.pwm_period_num > 25000 || motor_right_pwm_in_data.pwm_period_num < 3000)    // 要求控制脉冲必须在 40Hz ~ 300Hz 之间
        {
            motor_right_pwm_in_data.pwm_high_level = 0;
            
            motor_right_pwm_in_data.pwm_period_num = 0;
            
            motor_right_pwm_in_data.get_duty = 0;
        }
        else
        {

            if((motor_right_pwm_in_data.pwm_high_level % 10) >= 7 )                                             // 输入脉冲的毛刺、脉宽波动优化
            {
                motor_right_pwm_in_data.pwm_high_level = (motor_right_pwm_in_data.pwm_high_level / 10 + 1) * 10;
            }
            else if((motor_right_pwm_in_data.pwm_high_level % 10) <= 3)
            {
                motor_right_pwm_in_data.pwm_high_level = (motor_right_pwm_in_data.pwm_high_level / 10) * 10;
            }
            
            motor_right_pwm_in_data.pwm_low_level = motor_right_pwm_in_data.pwm_period_num - motor_right_pwm_in_data.pwm_high_level;                          // 计算低电平时长(低电平时长并没有用到，可以作为数据观察)
            
            motor_right_pwm_in_data.get_duty = (float)((float)motor_right_pwm_in_data.pwm_high_level / (float)motor_right_pwm_in_data.pwm_period_num);       // 计算占空比 范围0-1 浮点类型
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PWM脉宽输入捕获初始化 
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_isr();
// 备注信息     用户使用pwm作为控制信号时，需要使用到此脉宽捕获
//-------------------------------------------------------------------------------------------------------------------
void pwm_in_init(void)
{
    /******************************定义局部参数******************************/
    cy_stc_gpio_pin_config_t            pwm_in_pin_cfg;                         // 定义脉冲捕获引脚参数对象
    cy_stc_tcpwm_counter_config_t       pwm_in_counter_cfg;                     // 定义脉冲捕获定时器参数对象
    cy_stc_tcpwm_counter_config_t       pwm_in_over_time_cfg;                   // 定义脉冲捕获超时定时器参数对象
    cy_stc_sysint_irq_t                 pwm_in_irq_cfg;                         // 定义脉冲捕获中断参数对象
    cy_stc_sysint_irq_t                 pwm_in_over_time_irq_cfg;               // 定义脉冲捕获超时中断参数对象
    /******************************定义局部参数******************************/
    
    
    /************************配置左侧脉冲捕获输入引脚************************/
    memset(&pwm_in_pin_cfg, 0 , sizeof(pwm_in_pin_cfg));                        // 清除脉冲捕获引脚对象参数
    
    pwm_in_pin_cfg.driveMode = CY_GPIO_DM_PULLDOWN;                               // 配置引脚模式为强驱动模式
    pwm_in_pin_cfg.hsiom     = MOTOR_LEFT_PWM_IN_HSIOM;                         // 配置引脚复用为定时器触发功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_PWM_IN_PIN), (MOTOR_LEFT_PWM_IN_PIN % 8), &pwm_in_pin_cfg);  // 初始化脉冲捕获引脚
    /************************配置左侧脉冲捕获输入引脚************************/
    
    
    /***********************配置左侧脉冲捕获定时器时钟***********************/
    // 初始化捕获通道时钟 使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_PWM_IN_CLOCK, CY_SYSCLK_DIV_16_BIT, 0);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);
    // 初始化捕获超时通道时钟 使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_PWM_IN_OVERTIME_CLOCK, CY_SYSCLK_DIV_16_BIT, 0);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);
    /***********************配置左侧脉冲捕获定时器时钟***********************/

    
    /************************配置左侧脉冲捕获定时器**************************/
    memset(&pwm_in_counter_cfg, 0, sizeof(pwm_in_counter_cfg));                // 清除脉冲捕获定时器对象参数          
                                                                                
    pwm_in_counter_cfg.period             = 0xFFFF                       ;      // 定时器周期为65535                       
    pwm_in_counter_cfg.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_8   ; 	// 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
    pwm_in_counter_cfg.runMode            = CY_TCPWM_COUNTER_CONTINUOUS  ;      // 持续运行模式
    pwm_in_counter_cfg.countDirection     = CY_TCPWM_COUNTER_COUNT_UP    ;      // 向上计数 
    pwm_in_counter_cfg.debug_pause        = false                        ;      // 关闭“调试时暂停计数”
    pwm_in_counter_cfg.compareOrCapture   = CY_TCPWM_COUNTER_MODE_CAPTURE;      // 捕获模式
    pwm_in_counter_cfg.interruptSources   = CY_TCPWM_INT_NONE            ;      // 自定义中断资源
    pwm_in_counter_cfg.capture0InputMode  = CY_TCPWM_INPUT_FALLING_EDGE  ;	// 捕获寄存器0捕获事件触发在下降沿	
    pwm_in_counter_cfg.capture0Input      = 2uL                          ;      // 选择定时器通道的复用引脚
    pwm_in_counter_cfg.capture1InputMode  = CY_TCPWM_INPUT_RISING_EDGE   ;      // 捕获寄存器1捕获事件触发在上升沿   
    pwm_in_counter_cfg.capture1Input      = 2uL                          ;      // 选择定时器通道的复用引脚
    pwm_in_counter_cfg.countInputMode     = CY_TCPWM_INPUT_LEVEL         ;      // 计数触发为水平信号
    pwm_in_counter_cfg.countInput         = 1uL                          ;      // 触发输入为持续高电平(直接计数)
    pwm_in_counter_cfg.trigger0EventCfg   = CY_TCPWM_COUNTER_CC0_MATCH   ;      // 中断事件0配置为捕获寄存器0触发
    pwm_in_counter_cfg.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;      // 中断事件1配置为溢出时触发
    
    
    Cy_Tcpwm_Counter_Init(MOTOR_LEFT_PWM_IN_TIMER, &pwm_in_counter_cfg);        // 初始化定时器参数
    Cy_Tcpwm_Counter_SetCC0_IntrMask(MOTOR_LEFT_PWM_IN_TIMER);                  // 使能捕获寄存器0中断
    Cy_Tcpwm_Counter_SetCC1_IntrMask(MOTOR_LEFT_PWM_IN_TIMER);                  // 使能捕获寄存器1中断
    Cy_Tcpwm_Counter_Enable(MOTOR_LEFT_PWM_IN_TIMER);                           // 使能定时器
    Cy_Tcpwm_TriggerStart(MOTOR_LEFT_PWM_IN_TIMER);                             // 触发定时器计数
    /************************配置左侧脉冲捕获定时器**************************/
    
    
    /*************************配置左侧脉冲捕获中断***************************/
    pwm_in_irq_cfg.sysIntSrc  = MOTOR_LEFT_PWM_IN_ISR                    ;      // 选择中断触发源为TCPWM定时器1的通道7
    pwm_in_irq_cfg.intIdx     = CPUIntIdx0_IRQn                          ;      // 选择中断触发通道为用户中断0
    pwm_in_irq_cfg.isEnabled  = true                                     ;      // 允许中断触发
    interrupt_init(&pwm_in_irq_cfg, motor_left_pwm_in_isr, 0)            ;      // 中断初始化 优先级 0
    /*************************配置左侧脉冲捕获中断***************************/

    
    /**********************配置左侧脉冲捕获超时定时器************************/
    memset(&pwm_in_over_time_cfg, 0, sizeof(pwm_in_over_time_cfg));            // 清除脉冲捕获超时定时器对象参数       
    
    pwm_in_over_time_cfg.period             = 0xFFFF                       ;    // 定时器周期为65535                           
    pwm_in_over_time_cfg.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_8   ;    // 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
    pwm_in_over_time_cfg.runMode            = CY_TCPWM_COUNTER_CONTINUOUS  ;    // 持续运行模式
    pwm_in_over_time_cfg.countDirection     = CY_TCPWM_COUNTER_COUNT_UP    ;    // 向上计数 
    pwm_in_over_time_cfg.compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE;    // 比较模式
    pwm_in_over_time_cfg.countInputMode     = CY_TCPWM_INPUT_LEVEL         ;    // 选择定时器通道的复用引脚
    pwm_in_over_time_cfg.countInput         = 1uL                          ;    // 计数触发为水平信号
    pwm_in_over_time_cfg.trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;    // 中断事件0配置为溢出时触发
    pwm_in_over_time_cfg.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;    // 中断事件1配置为溢出时触发
                                                                                   
    Cy_Tcpwm_Counter_Init(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER, &pwm_in_over_time_cfg);     // 初始化定时器参数 
    Cy_Tcpwm_Counter_Enable(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER);                          // 使能定时器 
    Cy_Tcpwm_TriggerStart(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER);                            // 触发定时器计数 
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_LEFT_PWM_IN_OVERTIME_TIMER);                  // 使能计数器中断 
    /**********************配置左侧脉冲捕获超时定时器************************/   
    
    
    /***********************配置左侧脉冲捕获超时中断*************************/
    pwm_in_over_time_irq_cfg.sysIntSrc  = MOTOR_LEFT_PWM_IN_OVERTIME_ISR ;      // 选择中断触发源为TCPWM定时器0的通道18
    pwm_in_over_time_irq_cfg.intIdx     = CPUIntIdx0_IRQn                ;      // 选择中断触发通道为用户中断0
    pwm_in_over_time_irq_cfg.isEnabled  = true                           ;      // 允许中断触发
    interrupt_init(&pwm_in_over_time_irq_cfg, motor_left_pwm_in_over_time_isr, 0);      // 中断初始化 优先级 0
    /***********************配置左侧脉冲捕获超时中断*************************/
    
    
    
    
    /************************配置右侧脉冲捕获输入引脚************************/
    memset(&pwm_in_pin_cfg, 0 , sizeof(pwm_in_pin_cfg));                        // 清除脉冲捕获引脚对象参数
    
    pwm_in_pin_cfg.driveMode = CY_GPIO_DM_PULLDOWN;                               // 配置引脚模式为强驱动模式
    pwm_in_pin_cfg.hsiom     = MOTOR_RIGHT_PWM_IN_HSIOM;                        // 配置引脚复用为定时器触发功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_PWM_IN_PIN), (MOTOR_RIGHT_PWM_IN_PIN % 8), &pwm_in_pin_cfg);  // 初始化脉冲捕获引脚
    /************************配置右侧脉冲捕获输入引脚************************/
    
    
    /***********************配置右侧脉冲捕获定时器时钟***********************/
    // 初始化捕获通道时钟 使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_PWM_IN_CLOCK, CY_SYSCLK_DIV_16_BIT, 0);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);
    // 初始化捕获超时通道时钟 使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_PWM_IN_OVERTIME_CLOCK, CY_SYSCLK_DIV_16_BIT, 0);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);
    /***********************配置右侧脉冲捕获定时器时钟***********************/

    
    /************************配置右侧脉冲捕获定时器**************************/
    memset(&pwm_in_counter_cfg, 0, sizeof(pwm_in_counter_cfg));                // 清除脉冲捕获定时器对象参数          
                                                                                
    pwm_in_counter_cfg.period             = 0xFFFF                       ;      // 定时器周期为65535                       
    pwm_in_counter_cfg.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_8   ; 	// 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
    pwm_in_counter_cfg.runMode            = CY_TCPWM_COUNTER_CONTINUOUS  ;      // 持续运行模式
    pwm_in_counter_cfg.countDirection     = CY_TCPWM_COUNTER_COUNT_UP    ;      // 向上计数 
    pwm_in_counter_cfg.debug_pause        = false                        ;      // 关闭“调试时暂停计数”
    pwm_in_counter_cfg.compareOrCapture   = CY_TCPWM_COUNTER_MODE_CAPTURE;      // 捕获模式
    pwm_in_counter_cfg.interruptSources   = CY_TCPWM_INT_NONE            ;      // 自定义中断资源
    pwm_in_counter_cfg.capture0InputMode  = CY_TCPWM_INPUT_FALLING_EDGE  ;	// 捕获寄存器0捕获事件触发在下降沿	
    pwm_in_counter_cfg.capture0Input      = 2uL                          ;      // 选择定时器通道的复用引脚
    pwm_in_counter_cfg.capture1InputMode  = CY_TCPWM_INPUT_RISING_EDGE   ;      // 捕获寄存器1捕获事件触发在上升沿   
    pwm_in_counter_cfg.capture1Input      = 2uL                          ;      // 选择定时器通道的复用引脚
    pwm_in_counter_cfg.countInputMode     = CY_TCPWM_INPUT_LEVEL         ;      // 计数触发为水平信号
    pwm_in_counter_cfg.countInput         = 1uL                          ;      // 触发输入为持续高电平(直接计数)
    pwm_in_counter_cfg.trigger0EventCfg   = CY_TCPWM_COUNTER_CC0_MATCH   ;      // 中断事件0配置为捕获寄存器0触发
    pwm_in_counter_cfg.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;      // 中断事件1配置为溢出时触发
    
    
    Cy_Tcpwm_Counter_Init(MOTOR_RIGHT_PWM_IN_TIMER, &pwm_in_counter_cfg);       // 初始化定时器参数
    Cy_Tcpwm_Counter_SetCC0_IntrMask(MOTOR_RIGHT_PWM_IN_TIMER);                 // 使能捕获寄存器0中断
    Cy_Tcpwm_Counter_SetCC1_IntrMask(MOTOR_RIGHT_PWM_IN_TIMER);                 // 使能捕获寄存器1中断
    Cy_Tcpwm_Counter_Enable(MOTOR_RIGHT_PWM_IN_TIMER);                          // 使能定时器
    Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_PWM_IN_TIMER);                            // 触发定时器计数
    /************************配置右侧脉冲捕获定时器**************************/
    
    
    /*************************配置右侧脉冲捕获中断***************************/
    pwm_in_irq_cfg.sysIntSrc  = MOTOR_RIGHT_PWM_IN_ISR                    ;     // 选择中断触发源为TCPWM定时器1的通道7
    pwm_in_irq_cfg.intIdx     = CPUIntIdx0_IRQn                          ;      // 选择中断触发通道为用户中断0
    pwm_in_irq_cfg.isEnabled  = true                                     ;      // 允许中断触发
    interrupt_init(&pwm_in_irq_cfg, motor_right_pwm_in_isr, 0)            ;     // 中断初始化 优先级 0
    /*************************配置右侧脉冲捕获中断***************************/

    
    /**********************配置右侧脉冲捕获超时定时器************************/
    memset(&pwm_in_over_time_cfg, 0, sizeof(pwm_in_over_time_cfg));            // 清除脉冲捕获超时定时器对象参数       
    
    pwm_in_over_time_cfg.period             = 0xFFFF                       ;    // 定时器周期为65535                           
    pwm_in_over_time_cfg.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_8   ;    // 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
    pwm_in_over_time_cfg.runMode            = CY_TCPWM_COUNTER_CONTINUOUS  ;    // 持续运行模式
    pwm_in_over_time_cfg.countDirection     = CY_TCPWM_COUNTER_COUNT_UP    ;    // 向上计数 
    pwm_in_over_time_cfg.compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE;    // 比较模式
    pwm_in_over_time_cfg.countInputMode     = CY_TCPWM_INPUT_LEVEL         ;    // 选择定时器通道的复用引脚
    pwm_in_over_time_cfg.countInput         = 1uL                          ;    // 计数触发为水平信号
    pwm_in_over_time_cfg.trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;    // 中断事件0配置为溢出时触发
    pwm_in_over_time_cfg.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW    ;    // 中断事件1配置为溢出时触发
                                                                                   
    Cy_Tcpwm_Counter_Init(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER, &pwm_in_over_time_cfg);    // 初始化定时器参数 
    Cy_Tcpwm_Counter_Enable(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER);                         // 使能定时器 
    Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER);                           // 触发定时器计数 
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_RIGHT_PWM_IN_OVERTIME_TIMER);                 // 使能计数器中断 
    /**********************配置右侧脉冲捕获超时定时器************************/   
    
    
    /***********************配置右侧脉冲捕获超时中断*************************/
    pwm_in_over_time_irq_cfg.sysIntSrc  = MOTOR_RIGHT_PWM_IN_OVERTIME_ISR;      // 选择中断触发源为TCPWM定时器0的通道18
    pwm_in_over_time_irq_cfg.intIdx     = CPUIntIdx0_IRQn                ;      // 选择中断触发通道为用户中断0
    pwm_in_over_time_irq_cfg.isEnabled  = true                           ;      // 允许中断触发
    interrupt_init(&pwm_in_over_time_irq_cfg, motor_right_pwm_in_over_time_isr, 0);     // 中断初始化 优先级 0
    /***********************配置右侧脉冲捕获超时中断*************************/
}