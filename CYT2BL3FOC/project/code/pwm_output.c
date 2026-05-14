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

#include "pwm_output.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左电机PWM通道初始化
// 参数说明     timer_period        // 定时器周期值
// 参数说明     isr_state           // 是否启用更新中断  0 不启用  1 启用
// 返回参数     void
// 使用示例     motor_left_output_init(PWM_PRIOD_LOAD, 1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_output_init(uint16 timer_period, uint8 isr_state)
{
    /******************************定义局部参数******************************/
    cy_stc_gpio_pin_config_t            pwm_pin_config;                         // 定义PWM引脚配置对象
    cy_stc_tcpwm_pwm_config_t           tcpwm_pwm_config;                       // 定义PWM定时器配置对象    
    cy_stc_sysint_irq_t                 tcpwm_irq_cfg;                          // 定义PWM更新中断对象
    /******************************定义局部参数******************************/
    
    
    /******************************配置PWM引脚******************************/
    memset(&pwm_pin_config, 0, sizeof(pwm_pin_config));                         // 清除PWM引脚对象参数 
    
    pwm_pin_config.driveMode                     = CY_GPIO_DM_STRONG;           // 配置PWM输出引脚模式为强驱动模式
    // 初始化A相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_A_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_A_PHASE_PORT_H, MOTOR_LEFT_A_PHASE_PIN_H, &pwm_pin_config);
    // 初始化A相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_A_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_A_PHASE_PORT_L, MOTOR_LEFT_A_PHASE_PIN_L, &pwm_pin_config);
    // 初始化B相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_B_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_B_PHASE_PORT_H, MOTOR_LEFT_B_PHASE_PIN_H, &pwm_pin_config);
    // 初始化B相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_B_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_B_PHASE_PORT_L, MOTOR_LEFT_B_PHASE_PIN_L, &pwm_pin_config);
    // 初始化C相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_C_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_C_PHASE_PORT_H, MOTOR_LEFT_C_PHASE_PIN_H, &pwm_pin_config);
    // 初始化C相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_LEFT_C_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_LEFT_C_PHASE_PORT_L, MOTOR_LEFT_C_PHASE_PIN_L, &pwm_pin_config);
    /******************************配置PWM引脚******************************/

    /****************************配置PWM定时器时钟***************************/
    // 初始化A相时钟  使用16位分频器的通道2 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_A_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    // 初始化B相时钟  使用16位分频器的通道2 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_B_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    // 初始化C相时钟  使用16位分频器的通道2 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_C_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    /****************************配置PWM定时器时钟***************************/
    
    /******************************配置PWM中断******************************/
    if(isr_state) 
    {
        tcpwm_irq_cfg.sysIntSrc  = MOTOR_LEFT_TCPWM_ISR_CHANNEL;                            // 选择中断触发源
        tcpwm_irq_cfg.intIdx     = CPUIntIdx3_IRQn;                                         // 选择中断触发通道为用户中断7
        tcpwm_irq_cfg.isEnabled  = true;                                                    // 允许中断触发
        
        Cy_SysInt_InitIRQ(&tcpwm_irq_cfg);                                                  // 初始化中断配置参数
        Cy_SysInt_SetSystemIrqVector(tcpwm_irq_cfg.sysIntSrc, motor_left_update_isr);       // 配置当前中断回调函数为 motor_left_updat_isr
        NVIC_SetPriority(tcpwm_irq_cfg.intIdx, 0u);                                         // 配置中断优先级为0 (最高优先级)
        NVIC_ClearPendingIRQ(tcpwm_irq_cfg.intIdx);                                         // 清除中断请求的挂起状态
        NVIC_EnableIRQ(tcpwm_irq_cfg.intIdx);                                               // 使能当前中断
    }
    /******************************配置PWM中断******************************/
    
    /*****************************配置PWM定时器******************************/
    memset(&tcpwm_pwm_config, 0, sizeof(tcpwm_pwm_config));                     // 清除定时器配置参数
    
    tcpwm_pwm_config.pwmMode            = CY_TCPWM_PWM_MODE_DEADTIME      ;     // 当前模式为PWM死区模式 可以互补输出并且配置死区时间
    tcpwm_pwm_config.countDirection     = CY_TCPWM_COUNTER_COUNT_UP       ;     // 计数器向上计数
    tcpwm_pwm_config.cc0MatchMode       = CY_TCPWM_PWM_TR_CTRL2_SET       ;     // 当比较寄存器0与计数器匹配时 拉高引脚输出
    tcpwm_pwm_config.cc1MatchMode       = CY_TCPWM_PWM_TR_CTRL2_CLEAR     ;     // 当比较寄存器1与计数器匹配时 拉低引脚输出
    tcpwm_pwm_config.overflowMode       = CY_TCPWM_PWM_TR_CTRL2_CLEAR     ;     // 当计数器上溢时 拉低引脚输出
    tcpwm_pwm_config.underflowMode      = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE ;     // 当计数器下溢时 引脚无需变化(向上计数模式不会触发计数下溢)
    tcpwm_pwm_config.deadTime           = 10                              ;     // 死区时间为10个时钟周期 80mhz时钟的10个周期为125ns
    tcpwm_pwm_config.deadTimeComp       = 10                              ;     // 互补通道死区时间也为10个时钟周期 
    tcpwm_pwm_config.runMode            = CY_TCPWM_PWM_CONTINUOUS         ;     // 持续运行 持续输出PWM信号
    tcpwm_pwm_config.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1      ;     // 时钟1分频 80Mhz时钟直接作用于定时器 单位时钟周期为12.5ns
    tcpwm_pwm_config.period             = (timer_period - 1)              ;     // 定时器周期为timer_period 80Mhz除以周期得到频率
    tcpwm_pwm_config.compare0           = 0                               ;     // 比较寄存器0清空
    tcpwm_pwm_config.compare1           = 0                               ;     // 比较寄存器1清空
    tcpwm_pwm_config.enableCompare0Swap = true                            ;     // 比较器0交换使能 此处为预装载功能实现 用户的比较数据写入缓冲区 溢出事件触发主动交换缓冲区以及实际比较寄存器的值
    tcpwm_pwm_config.enableCompare1Swap = true                            ;     // 比较器1交换使能
    tcpwm_pwm_config.killMode           = CY_TCPWM_PWM_STOP_ON_KILL       ;     // 关闭定时器时停止计数
    tcpwm_pwm_config.countInputMode     = CY_TCPWM_INPUT_LEVEL            ;     // 计数事件为任意
    tcpwm_pwm_config.countInput         = 1uL                             ;     // 计数事件输入高电平
    tcpwm_pwm_config.trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW       ;     // 中断触发事件0为溢出时触发中断
    tcpwm_pwm_config.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW       ;     // 中断触发事件1为溢出时触发中断

    // 初始化A相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_LEFT_A_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_LEFT_A_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_LEFT_A_PHASE_TCPWM_TIMER);            // 使能定时器中断
    if(isr_state) 
    {
        Cy_Tcpwm_TriggerStart(MOTOR_LEFT_A_PHASE_TCPWM_TIMER);
    }
    // 初始化B相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_LEFT_B_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_LEFT_B_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_TriggerStart(MOTOR_LEFT_B_PHASE_TCPWM_TIMER);
    
    // 初始化C相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_LEFT_C_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_LEFT_C_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_TriggerStart(MOTOR_LEFT_C_PHASE_TCPWM_TIMER);
    /*****************************配置PWM定时器******************************/
    
    /***************************定时器计数同步配置****************************/
    // 由于三个通道都是独立的定时器 那么counter寄存器的计数值也是各自计数
    // 如果counter不同步的话 输出的三路PWM波形就无法对齐
    // 此处为最简单的伪同步方法：以最快的转速清除三路counter寄存器数据
    // 首先获取counter寄存器的地址
    uint32_t pwm_a_count_address = MOTOR_LEFT_A_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    uint32_t pwm_b_count_address = MOTOR_LEFT_B_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    uint32_t pwm_c_count_address = MOTOR_LEFT_C_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    // 通过指针访问寄存器地址
    volatile uint32_t *pwm_a_count_pointer = (volatile uint32_t *)pwm_a_count_address;
    volatile uint32_t *pwm_b_count_pointer = (volatile uint32_t *)pwm_b_count_address;
    volatile uint32_t *pwm_c_count_pointer = (volatile uint32_t *)pwm_c_count_address;
    // 直接对指针指向的寄存器赋值0
    *pwm_a_count_pointer     = 0;    
    *pwm_b_count_pointer     = 0;
    *pwm_c_count_pointer     = 0;
    /***************************定时器计数同步配置****************************/
    
    motor_left_channel_set(1,1,1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧电机三相通道占空比配置
// 参数说明     a_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 参数说明     b_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 参数说明     c_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 返回参数     void
// 使用示例     motor_left_duty_set(200, 300, 400);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_duty_set(uint16 a_duty, uint16 b_duty, uint16 c_duty)
{
    MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - a_duty) / 2;  // 配置A相通道比较器1的值
    MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + a_duty) / 2;  // 配置A相通道比较器2的值
    MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - b_duty) / 2;  // 配置B相通道比较器1的值
    MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + b_duty) / 2;  // 配置B相通道比较器2的值
    MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - c_duty) / 2;  // 配置C相通道比较器1的值
    MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + c_duty) / 2;  // 配置C相通道比较器2的值
    MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置A相通道交换使能
    MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置B相通道交换使能
    MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置C相通道交换使能
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧电机三相通道使能选择
// 参数说明     a_channel       0：关闭A相输出通道  1：开启A相输出通道  
// 参数说明     b_channel       0：关闭B相输出通道  1：开启B相输出通道  
// 参数说明     c_channel       0：关闭C相输出通道  1：开启C相输出通道  
// 返回参数     void
// 使用示例     motor_left_channel_set(1,0,1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_channel_set(uint8 a_channel, uint8 b_channel, uint8 c_channel)
{     
    if(a_channel)                                                               // 当A相通道使能则开启通道
    {
        MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unCTRL.u32Register       |= 0x80000000;              // 使能定时器
        MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unTR_CMD.u32Register      = 0x00000004;              // 启动计数
    }
    else                                                                        // 否则关闭A相通道使能
    {
        MOTOR_LEFT_A_PHASE_TCPWM_TIMER->unCTRL.u32Register       &= 0x7FFFFFFF;              // 关闭A相通道使能
    }   
    if(b_channel)                                                               // 当B相通道使能则开启通道
    {                                                                             
        MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unCTRL.u32Register        |= 0x80000000;              // 使能定时器 
        MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unTR_CMD.u32Register       = 0x00000004;              // 启动计数 
    }                                                                             
    else                                                                        // 否则关闭B相通道使能
    {                                                                             
        MOTOR_LEFT_B_PHASE_TCPWM_TIMER->unCTRL.u32Register        &= 0x7FFFFFFF;              // 关闭B相通道使能 
    }
    if(c_channel)                                                               // 当C相通道使能则开启通道
    {                                                                               
        MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unCTRL.u32Register       |= 0x80000000;              // 使能定时器  
        MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unTR_CMD.u32Register      = 0x00000004;              // 启动计数  
    }                                                                               
    else                                                                        // 否则关闭C相通道使能
    {                                                                               
        MOTOR_LEFT_C_PHASE_TCPWM_TIMER->unCTRL.u32Register       &= 0x7FFFFFFF;              // 关闭C相通道使能  
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左侧电机三相MOS及预驱功能测试 
// 返回参数     void
// 使用示例     motor_left_phase_check(300);
// 备注信息     该函数会通过控制不同通道的MOS，使电机发出不同频率的声音，便于测试硬件功能，调用后需重新主动初始化PWM定时器模块  
//-------------------------------------------------------------------------------------------------------------------
void motor_left_phase_check(uint16 delay_time)
{
    uint16 check_period;
    
    uint16 interval_time = 50;
    
    // 响一声  A相上桥 → B相下桥
    check_period = 36000;                       // 2.222khz
    
    motor_left_output_init(check_period, 0);
    
    motor_left_channel_set(1,1,0);
    
    motor_left_duty_set(check_period / 50, 0, 0);
    
    system_delay_ms(interval_time);
    
    motor_left_duty_set(0, 0, 0);
    
    system_delay_ms(delay_time);
    
    // 响二声  B相上桥 → C相下桥
    check_period = 35000;                       // 2.285khz
    
    motor_left_output_init(check_period, 0);
    
    motor_left_channel_set(0,1,1);
    
    for(int i = 0; i < 2; i ++)
    {
        motor_left_duty_set(0, check_period / 50, 0);
    
        system_delay_ms(interval_time);
        
        motor_left_duty_set(0, 0, 0);
        
        system_delay_ms(interval_time);
    }
    
    system_delay_ms(delay_time);
    
    // 响三声  C相上桥 → A相下桥
    check_period = 34000;                       // 2.352khz
    
    motor_left_output_init(check_period, 0);
    
    motor_left_channel_set(1,0,1);
    
    for(int i = 0; i < 3; i ++)
    {
        motor_left_duty_set(0, 0, check_period / 50);
        
        system_delay_ms(interval_time);
        
        motor_left_duty_set(0, 0, 0);
        
        system_delay_ms(interval_time);
    }
    
    Cy_Tcpwm_Pwm_DeInit(MOTOR_LEFT_A_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Pwm_DeInit(MOTOR_LEFT_B_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Pwm_DeInit(MOTOR_LEFT_C_PHASE_TCPWM_TIMER);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧电机PWM通道初始化
// 参数说明     timer_period        // 定时器周期值
// 参数说明     isr_state           // 是否启用更新中断  0 不启用  1 启用
// 返回参数     void
// 使用示例     motor_right_output_init(PWM_PRIOD_LOAD, 1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_output_init(uint16 timer_period, uint8 isr_state)
{
    /******************************定义局部参数******************************/
    cy_stc_gpio_pin_config_t            pwm_pin_config;                         // 定义PWM引脚配置对象
    cy_stc_tcpwm_pwm_config_t           tcpwm_pwm_config;                       // 定义PWM定时器配置对象
    cy_stc_sysint_irq_t                 tcpwm_irq_cfg;                          // 定义PWM更新中断对象
    /******************************定义局部参数******************************/
    
    
    /******************************配置PWM引脚******************************/
    memset(&pwm_pin_config, 0, sizeof(pwm_pin_config));                         // 清除PWM引脚对象参数 
    
    pwm_pin_config.driveMode                     = CY_GPIO_DM_STRONG;           // 配置PWM输出引脚模式为强驱动模式
    // 初始化A相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_A_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_A_PHASE_PORT_H, MOTOR_RIGHT_A_PHASE_PIN_H, &pwm_pin_config);
    // 初始化A相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_A_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_A_PHASE_PORT_L, MOTOR_RIGHT_A_PHASE_PIN_L, &pwm_pin_config);
    // 初始化B相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_B_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_B_PHASE_PORT_H, MOTOR_RIGHT_B_PHASE_PIN_H, &pwm_pin_config);
    // 初始化B相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_B_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_B_PHASE_PORT_L, MOTOR_RIGHT_B_PHASE_PIN_L, &pwm_pin_config);
    // 初始化C相上桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_C_PIN_HSIOM_H;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_C_PHASE_PORT_H, MOTOR_RIGHT_C_PHASE_PIN_H, &pwm_pin_config);
    // 初始化C相下桥引脚
    pwm_pin_config.hsiom                         = MOTOR_RIGHT_C_PIN_HSIOM_L;
    Cy_GPIO_Pin_Init(MOTOR_RIGHT_C_PHASE_PORT_L, MOTOR_RIGHT_C_PHASE_PIN_L, &pwm_pin_config);
    /******************************配置PWM引脚******************************/

    /****************************配置PWM定时器时钟***************************/
    // 初始化A相时钟  使用16位分频器的通道1 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_A_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    // 初始化B相时钟  使用16位分频器的通道1 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_B_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    // 初始化C相时钟  使用16位分频器的通道1 不分频直接输出80Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_C_PHASE_TCPWM_CLOCK, CY_SYSCLK_DIV_16_BIT, 2);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 2, 0); 
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 2);
    /****************************配置PWM定时器时钟***************************/
    
    /******************************配置PWM中断******************************/
    if(isr_state)       
    { 
        tcpwm_irq_cfg.sysIntSrc  = MOTOR_RIGHT_TCPWM_ISR_CHANNEL;                           // 选择中断触发源
        tcpwm_irq_cfg.intIdx     = CPUIntIdx4_IRQn;                                         // 选择中断触发通道为用户中断7
        tcpwm_irq_cfg.isEnabled  = true;                                                    // 允许中断触发
        
        Cy_SysInt_InitIRQ(&tcpwm_irq_cfg);                                                  // 初始化中断配置参数
        Cy_SysInt_SetSystemIrqVector(tcpwm_irq_cfg.sysIntSrc, motor_right_update_isr);      // 配置当前中断回调函数为 motor_right_updat_isr
        NVIC_SetPriority(tcpwm_irq_cfg.intIdx, 0u);                                         // 配置中断优先级为0 (最高优先级)
        NVIC_ClearPendingIRQ(tcpwm_irq_cfg.intIdx);                                         // 清除中断请求的挂起状态
        NVIC_EnableIRQ(tcpwm_irq_cfg.intIdx);                                               // 使能当前中断
    }
    /******************************配置PWM中断******************************/
    
    /*****************************配置PWM定时器******************************/
    memset(&tcpwm_pwm_config, 0, sizeof(tcpwm_pwm_config));                     // 清除定时器配置参数
    
    tcpwm_pwm_config.pwmMode            = CY_TCPWM_PWM_MODE_DEADTIME      ;     // 当前模式为PWM死区模式 可以互补输出并且配置死区时间
    tcpwm_pwm_config.countDirection     = CY_TCPWM_COUNTER_COUNT_UP       ;     // 计数器向上计数
    tcpwm_pwm_config.cc0MatchMode       = CY_TCPWM_PWM_TR_CTRL2_SET       ;     // 当比较寄存器0与计数器匹配时 拉高引脚输出
    tcpwm_pwm_config.cc1MatchMode       = CY_TCPWM_PWM_TR_CTRL2_CLEAR     ;     // 当比较寄存器1与计数器匹配时 拉低引脚输出
    tcpwm_pwm_config.overflowMode       = CY_TCPWM_PWM_TR_CTRL2_CLEAR     ;     // 当计数器上溢时 拉低引脚输出
    tcpwm_pwm_config.underflowMode      = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE ;     // 当计数器下溢时 引脚无需变化(向上计数模式不会触发计数下溢)
    tcpwm_pwm_config.deadTime           = 10                              ;     // 死区时间为10个时钟周期 80mhz时钟的10个周期为125ns
    tcpwm_pwm_config.deadTimeComp       = 10                              ;     // 互补通道死区时间也为10个时钟周期 
    tcpwm_pwm_config.runMode            = CY_TCPWM_PWM_CONTINUOUS         ;     // 持续运行 持续输出PWM信号
    tcpwm_pwm_config.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1      ;     // 时钟1分频 80Mhz时钟直接作用于定时器 单位时钟周期为12.5ns
    tcpwm_pwm_config.period             = (timer_period - 1)              ;     // 定时器周期为timer_period 80Mhz除以周期得到频率
    tcpwm_pwm_config.compare0           = 0                               ;     // 比较寄存器0清空
    tcpwm_pwm_config.compare1           = 0                               ;     // 比较寄存器1清空
    tcpwm_pwm_config.enableCompare0Swap = true                            ;     // 比较器0交换使能 此处为预装载功能实现 用户的比较数据写入缓冲区 溢出事件触发主动交换缓冲区以及实际比较寄存器的值
    tcpwm_pwm_config.enableCompare1Swap = true                            ;     // 比较器1交换使能
    tcpwm_pwm_config.killMode           = CY_TCPWM_PWM_STOP_ON_KILL       ;     // 关闭定时器时停止计数
    tcpwm_pwm_config.countInputMode     = CY_TCPWM_INPUT_LEVEL            ;     // 计数事件为任意
    tcpwm_pwm_config.countInput         = 1uL                             ;     // 计数事件输入高电平
    tcpwm_pwm_config.trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW       ;     // 中断触发事件0为溢出时触发中断
    tcpwm_pwm_config.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW       ;     // 中断触发事件1为溢出时触发中断
    // 初始化A相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER);            // 使能定时器中断
    if(isr_state)       
    {  
        Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER);
    }
    // 初始化B相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_RIGHT_B_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_RIGHT_B_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_B_PHASE_TCPWM_TIMER);
    
    // 初始化C相PWM通道   由于三个输出通道配置参数一致 则直接使用同一个配置对象对三个通道完成参数配置
    Cy_Tcpwm_Pwm_Init(MOTOR_RIGHT_C_PHASE_TCPWM_TIMER, &tcpwm_pwm_config);
    Cy_Tcpwm_Pwm_Enable(MOTOR_RIGHT_C_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_C_PHASE_TCPWM_TIMER);
    /*****************************配置PWM定时器******************************/
    
    /***************************定时器计数同步配置****************************/
    // 由于三个通道都是独立的定时器 那么counter寄存器的计数值也是各自计数
    // 如果counter不同步的话 输出的三路PWM波形就无法对齐
    // 此处为最简单的伪同步方法：以最快的转速清除三路counter寄存器数据
    // 首先获取counter寄存器的地址
    uint32_t pwm_a_count_address = MOTOR_RIGHT_A_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    uint32_t pwm_b_count_address = MOTOR_RIGHT_B_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    uint32_t pwm_c_count_address = MOTOR_RIGHT_C_PHASE_TCPWM_ADDR + 0x00000008;                    // 通道地址 + COUNTER寄存器偏移量
    // 通过指针访问寄存器地址
    volatile uint32_t *pwm_a_count_pointer = (volatile uint32_t *)pwm_a_count_address;
    volatile uint32_t *pwm_b_count_pointer = (volatile uint32_t *)pwm_b_count_address;
    volatile uint32_t *pwm_c_count_pointer = (volatile uint32_t *)pwm_c_count_address;
    // 直接对指针指向的寄存器赋值0
    *pwm_a_count_pointer     = 0;    
    *pwm_b_count_pointer     = 0;
    *pwm_c_count_pointer     = 0;
    /***************************定时器计数同步配置****************************/
    
    motor_right_channel_set(1,1,1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧电机三相通道占空比配置
// 参数说明     a_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 参数说明     b_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 参数说明     c_duty       A相占空比信息 最大值 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 返回参数     void
// 使用示例     motor_left_duty_set(200, 300, 400);
// 备注信息     更新比较器并且将计数器清零
//-------------------------------------------------------------------------------------------------------------------
void motor_right_duty_set(uint16 a_duty, uint16 b_duty, uint16 c_duty)
{
    MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - a_duty) / 2;  // 配置A相通道比较器1的值
    MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + a_duty) / 2;  // 配置A相通道比较器2的值
    MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - b_duty) / 2;  // 配置B相通道比较器1的值
    MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + b_duty) / 2;  // 配置B相通道比较器2的值
    MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unCC0_BUFF.u32Register = (PWM_PRIOD_LOAD - c_duty) / 2;  // 配置C相通道比较器1的值
    MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unCC1_BUFF.u32Register = (PWM_PRIOD_LOAD + c_duty) / 2;  // 配置C相通道比较器2的值
    MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置A相通道交换使能
    MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置B相通道交换使能
    MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unTR_CMD.u32Register  |= 1;  // 配置C相通道交换使能
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧电机三相通道使能选择
// 参数说明     a_channel       0：关闭A相输出通道  1：开启A相输出通道  
// 参数说明     b_channel       0：关闭B相输出通道  1：开启B相输出通道  
// 参数说明     c_channel       0：关闭C相输出通道  1：开启C相输出通道  
// 返回参数     void
// 使用示例     motor_channel_set(1,0,1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_channel_set(uint8 a_channel, uint8 b_channel, uint8 c_channel)
{     
    if(a_channel)                                                               // 当A相通道使能则开启通道
    {
        MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unCTRL.u32Register       |= 0x80000000;              // 使能定时器
        MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unTR_CMD.u32Register      = 0x00000004;              // 启动计数
    }
    else                                                                        // 否则关闭A相通道使能
    {
        MOTOR_RIGHT_A_PHASE_TCPWM_TIMER->unCTRL.u32Register       &= 0x7FFFFFFF;              // 关闭A相通道使能
    }   
    if(b_channel)                                                               // 当B相通道使能则开启通道
    {                                                                             
        MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unCTRL.u32Register        |= 0x80000000;              // 使能定时器 
        MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unTR_CMD.u32Register       = 0x00000004;              // 启动计数 
    }                                                                             
    else                                                                        // 否则关闭B相通道使能
    {                                                                             
        MOTOR_RIGHT_B_PHASE_TCPWM_TIMER->unCTRL.u32Register        &= 0x7FFFFFFF;              // 关闭B相通道使能 
    }
    if(c_channel)                                                               // 当C相通道使能则开启通道
    {                                                                               
        MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unCTRL.u32Register       |= 0x80000000;              // 使能定时器  
        MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unTR_CMD.u32Register      = 0x00000004;              // 启动计数  
    }                                                                               
    else                                                                        // 否则关闭C相通道使能
    {                                                                               
        MOTOR_RIGHT_C_PHASE_TCPWM_TIMER->unCTRL.u32Register       &= 0x7FFFFFFF;              // 关闭C相通道使能  
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右侧电机三相MOS及预驱功能测试 
// 返回参数     void
// 使用示例     motor_right_phase_check();
// 备注信息     该函数会通过控制不同通道的MOS，使电机发出不同频率的声音，便于测试硬件功能，调用后需重新主动初始化PWM定时器模块  
//-------------------------------------------------------------------------------------------------------------------
void motor_right_phase_check(uint16 delay_time)
{
    uint16 check_period;
    
    uint16 interval_time = 50;
    
    // 响一声  A相上桥→B相下桥
    check_period = 36000;               // 2.222khz
      
    motor_right_output_init(check_period, 0);      
    
    motor_right_channel_set(1,1,0);
    
    motor_right_duty_set(check_period / 50, 0, 0);
    
    system_delay_ms(interval_time);
    
    motor_right_duty_set(0, 0, 0);
    
    system_delay_ms(delay_time);
    
    // 响二声  B相上桥→C相下桥
    check_period = 35000;               // 2.285khz
    
    motor_right_output_init(check_period, 0);
    
    motor_right_channel_set(0,1,1);
    
    for(int i = 0; i < 2; i ++)
    {    
        motor_right_duty_set(0, check_period / 50, 0);
    
        system_delay_ms(interval_time);
    
        motor_right_duty_set(0, 0, 0);
        
        system_delay_ms(interval_time);
    }
    
    system_delay_ms(delay_time);
    
    
    // 响三声  C相上桥→A相下桥
    check_period = 34000;               // 2.352khz
    
    motor_right_output_init(check_period, 0);
    
    motor_right_channel_set(1,0,1);
    
    for(int i = 0; i < 3; i ++)
    {    
        motor_right_duty_set(0, 0, check_period / 50);
        
        system_delay_ms(interval_time);
        
        motor_right_duty_set(0, 0, 0);
        
        system_delay_ms(interval_time);
    }
    system_delay_ms(delay_time);
    
    Cy_Tcpwm_Pwm_DeInit(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Pwm_DeInit(MOTOR_RIGHT_B_PHASE_TCPWM_TIMER);
    Cy_Tcpwm_Pwm_DeInit(MOTOR_RIGHT_C_PHASE_TCPWM_TIMER);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 A相以及B相下桥MOS 关闭B相上桥以及C相MOS A相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 返回参数     void
// 使用示例     left_mos_q1q4_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q1q4_open(uint16 duty)
{
    motor_left_channel_set(1, 1, 0);         
    motor_left_duty_set(duty, 0, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 A相以及C相下桥MOS 关闭C相上桥以及B相MOS A相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     left_mos_q1q6_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q1q6_open(uint16 duty)
{
    motor_left_channel_set(1, 0, 1);
    motor_left_duty_set(duty, 0, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 B相以及A相下桥MOS 关闭A相上桥以及C相MOS B相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     left_mos_q3q2_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q3q2_open(uint16 duty)
{
    motor_left_channel_set(1, 1, 0);
    motor_left_duty_set(0, duty, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 B相以及C相下桥MOS 关闭C相上桥以及A相MOS B相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     left_mos_q3q6_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q3q6_open(uint16 duty)
{
    motor_left_channel_set(0, 1, 1);
    motor_left_duty_set(0, duty, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 C相以及A相下桥MOS 关闭A相上桥以及B相MOS C相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     left_mos_q5q2_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q5q2_open(uint16 duty)
{
    motor_left_channel_set(1, 0, 1);
    motor_left_duty_set(0, 0, duty);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 左侧 C相以及B相下桥MOS 关闭B相上桥以及A相MOS C相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     left_mos_q5q4_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_q5q4_open(uint16 duty)
{
    motor_left_channel_set(0, 1, 1);
    motor_left_duty_set(0, 0, duty);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     关闭 左侧 所有MOS
// 参数说明     void        
// 返回参数     void
// 使用示例     left_mos_close();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void left_mos_close(void)
{
    motor_left_duty_set(0, 0, 0);
    motor_left_channel_set(0, 0, 0);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 A相以及B相下桥MOS 关闭B相上桥以及C相MOS A相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)
// 返回参数     void
// 使用示例     right_mos_q1q4_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q1q4_open(uint16 duty)
{
    motor_right_channel_set(1, 1, 0);         
    motor_right_duty_set(duty, 0, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 A相以及C相下桥MOS 关闭C相上桥以及B相MOS A相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     right_mos_q1q6_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q1q6_open(uint16 duty)
{
    motor_right_channel_set(1, 0, 1);
    motor_right_duty_set(duty, 0, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 B相以及A相下桥MOS 关闭A相上桥以及C相MOS B相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     right_mos_q3q2_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q3q2_open(uint16 duty)
{
    motor_right_channel_set(1, 1, 0);
    motor_right_duty_set(0, duty, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 B相以及C相下桥MOS 关闭C相上桥以及A相MOS B相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     right_mos_q3q6_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q3q6_open(uint16 duty)
{
    motor_right_channel_set(0, 1, 1);
    motor_right_duty_set(0, duty, 0);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 C相以及A相下桥MOS 关闭A相上桥以及B相MOS C相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     right_mos_q5q2_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q5q2_open(uint16 duty)
{
    motor_right_channel_set(1, 0, 1);
    motor_right_duty_set(0, 0, duty);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     开启 右侧 C相以及B相下桥MOS 关闭B相上桥以及A相MOS C相输出占空比波形
// 参数说明     duty        占空比信息 PWM_PRIOD_LOAD (motor.h头文件有宏定义)        
// 返回参数     void
// 使用示例     right_mos_q5q4_open(200);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_q5q4_open(uint16 duty)
{
    motor_right_channel_set(0, 1, 1);
    motor_right_duty_set(0, 0, duty);
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     关闭 右侧 所有MOS
// 参数说明     void        
// 返回参数     void
// 使用示例     right_mos_close();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void right_mos_close(void)
{
    motor_right_duty_set(0, 0, 0);
    motor_right_channel_set(0, 0, 0);
}

