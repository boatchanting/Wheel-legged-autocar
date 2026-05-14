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

#include "sensorless_control.h"


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感pwm通道初始化
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_pwm_output_init(&motor_right);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void sensorless_pwm_output_init(motor_struct *motor_value)
{  
    /*************************配置 左侧无感 三相控制端口*************************/ 
    if(motor_value->motor_type == LEFT_MOTOR)
    {
        pwm_init(MOTOR_LEFT_SENSORLESS_A_PHASE_H, 50000, 0);                                        // 上桥统一初始化为 PWM 输出 频率默认50k 0占空比
    
        pwm_init(MOTOR_LEFT_SENSORLESS_B_PHASE_H, 50000, 0);
        
        pwm_init(MOTOR_LEFT_SENSORLESS_C_PHASE_H, 50000, 0);
        
        gpio_init(MOTOR_LEFT_SENSORLESS_A_PHASE_L, GPO, 0, GPO_PUSH_PULL);                          // 下桥统一初始化为 GPIO 输出 默认低电平
        
        gpio_init(MOTOR_LEFT_SENSORLESS_B_PHASE_L, GPO, 0, GPO_PUSH_PULL);
        
        gpio_init(MOTOR_LEFT_SENSORLESS_C_PHASE_L, GPO, 0, GPO_PUSH_PULL);   
    }
    /*************************配置 左侧无感 三相控制端口*************************/ 
    
    
    /*************************配置 右侧无感 三相控制端口*************************/ 
    if(motor_value->motor_type == RIGHT_MOTOR)
    {
        pwm_init(MOTOR_RIGHT_SENSORLESS_A_PHASE_H, 50000, 0);                                       // 上桥统一初始化为 PWM 输出 频率默认50k 0占空比
        
        pwm_init(MOTOR_RIGHT_SENSORLESS_B_PHASE_H, 50000, 0);
        
        pwm_init(MOTOR_RIGHT_SENSORLESS_C_PHASE_H, 50000, 0);
        
        gpio_init(MOTOR_RIGHT_SENSORLESS_A_PHASE_L, GPO, 0, GPO_PUSH_PULL);                         // 下桥统一初始化为 GPIO 输出 默认低电平
        
        gpio_init(MOTOR_RIGHT_SENSORLESS_B_PHASE_L, GPO, 0, GPO_PUSH_PULL);
        
        gpio_init(MOTOR_RIGHT_SENSORLESS_C_PHASE_L, GPO, 0, GPO_PUSH_PULL);   
    }
    /*************************配置 右侧无感 三相控制端口*************************/ 
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式 正弦 开环牵引
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_start_sine();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 sensorless_start_sine(motor_struct *motor_value)
{   
    static int32 location_time_use      = 1000;                                                        // 启动前强制定位50ms
    
    static int32 traction_time_use      = func_limit_ab((int32)SENSERLESS_OPENLOOP_SINE_TURNS * SENSERLESS_OPENLOOP_SINE_TIME * 20, SENSERLESS_OPENLOOP_SINE_TIME * 20, SENSERLESS_OPENLOOP_SINE_TIME * 600);         
    
    static float sine_duty              = func_limit_ab((float)SENSERLESS_OPENLOOP_SINE_DUTY / 10000.0f, 0.05, 0.2);
    
    uint8 sine_output_state             = 0;
    
    uint16 simulate_location            = 0;
    
    if(motor_value->motor_type == LEFT_MOTOR)                                                           // 判断是否为左侧电机
    {
        if(motor_value->sine_output_count == 0)                                                         // 初次执行正弦牵引 配置定时器、正弦控制功能
        {    
            fast_foc_init(&motor_left_foc_driver,  ENCODER_PRECISION, OUTPUT_DUTY_MAX, 1,  0,  1);      // 配置 FOST-FOC 控制功能
        
            motor_left_output_init(PWM_PRIOD_LOAD, 0);                                                  // 配置定时器互补输出  不开启定时器更新中断
        }
        
        if((motor_value->sine_output_count) < location_time_use)                                        // 执行时间小于定位时间 则执行定位控制
        {
            fast_foc_calculate(&motor_left_foc_driver, 0, sine_duty, 0);                                // FAST-FOC 固定占空比输出0角度位置 计算三相所需输出值  
        }
        else                                                                                            // 否则根据时间计算需要牵引输出的电角度
        {
            if(motor_value->motor_duty >= 0.0f)
            {
                simulate_location = ((motor_value->sine_output_count - location_time_use) * ENCODER_PRECISION / (SENSERLESS_OPENLOOP_SINE_TIME * 20)) % ENCODER_PRECISION;
            }
            else
            {
                simulate_location = ENCODER_PRECISION - ((motor_value->sine_output_count - location_time_use) * ENCODER_PRECISION / (SENSERLESS_OPENLOOP_SINE_TIME * 20)) % ENCODER_PRECISION;
            }
            
            fast_foc_calculate(&motor_left_foc_driver, simulate_location, sine_duty, 0);                // FAST-FOC 计算 三相输出值  
        }   
        
        motor_left_duty_set(motor_left_foc_driver.ouput_duty[0], motor_left_foc_driver.ouput_duty[1], motor_left_foc_driver.ouput_duty[2]);         // 输出三相占空比

        if((++ motor_value->sine_output_count)> (traction_time_use + location_time_use))                // 执行时间超过 定位时间 + 牵引时间 退出正弦牵引
        {
            motor_left_channel_set(0, 0, 0);                                                            // 关闭定时器输出
            
            sensorless_pwm_output_init(motor_value);                                                    // 重新配置定时器
            
            motor_value->sine_output_count = 0;                                                         // 清除本次牵引计数值
            
            sine_output_state = 1;                                                                      // 牵引结束标志位置位
        }
    }
    else
    {
        if(motor_value->sine_output_count == 0)                                                         // 初次执行正弦牵引 配置定时器、正弦控制功能
        {    
            fast_foc_init(&motor_right_foc_driver,  ENCODER_PRECISION, OUTPUT_DUTY_MAX, 1,  0,  1);     // 配置 FOST-FOC 控制功能
        
            motor_right_output_init(PWM_PRIOD_LOAD, 0);                                                 // 配置定时器互补输出  不开启定时器更新中断
        }
        
        if((motor_value->sine_output_count) < location_time_use)                                        // 执行时间小于定位时间 则执行定位控制
        {
            fast_foc_calculate(&motor_right_foc_driver, 0, sine_duty, 0);                               // FAST-FOC 固定占空比输出0角度位置 计算三相所需输出值  
        }
        else                                                                                            // 否则根据时间计算需要牵引输出的电角度
        {
            if(motor_value->motor_duty >= 0.0f)
            {
                simulate_location = ((motor_value->sine_output_count - location_time_use) * ENCODER_PRECISION / (SENSERLESS_OPENLOOP_SINE_TIME * 20)) % ENCODER_PRECISION;
            }
            else
            {
                simulate_location = ENCODER_PRECISION - ((motor_value->sine_output_count - location_time_use) * ENCODER_PRECISION / (SENSERLESS_OPENLOOP_SINE_TIME * 20)) % ENCODER_PRECISION;
            }
            
            fast_foc_calculate(&motor_right_foc_driver, simulate_location, sine_duty, 0);               // FAST-FOC 计算 三相输出值  
        }   
        
        motor_right_duty_set(motor_right_foc_driver.ouput_duty[0], motor_right_foc_driver.ouput_duty[1], motor_right_foc_driver.ouput_duty[2]);         // 输出三相占空比

        if((++ motor_value->sine_output_count) > (traction_time_use + location_time_use))               // 执行时间超过 定位时间 + 牵引时间 退出正弦牵引
        {
            motor_right_channel_set(0, 0, 0);                                                           // 关闭定时器输出
            
            sensorless_pwm_output_init(motor_value);                                                    // 重新配置定时器
            
            motor_value->sine_output_count = 0;                                                         // 清除本次牵引计数值
            
            sine_output_state = 1;                                                                      // 牵引结束标志位置位
        }
    }
    return sine_output_state;                                                                           // 返回牵引状态
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式 过零信号触发 使能
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_trig_disable(&motor_left);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void sensorless_motor_trig_enable(motor_struct *motor_value)
{
    if(motor_value->motor_type == LEFT_MOTOR)                                   // 判断是否为左侧电机
    {
        exti_enable(MOTOR_LEFT_SENSORLESS_A_TRIG);                              // 开启 A相过零信号触发 中断

        exti_enable(MOTOR_LEFT_SENSORLESS_B_TRIG);                              // 开启 B相过零信号触发 中断

        exti_enable(MOTOR_LEFT_SENSORLESS_C_TRIG);                              // 开启 C相过零信号触发 中断
      
        get_port(MOTOR_LEFT_SENSORLESS_A_TRIG)->unINTR.stcField.u1IN_IN0 = 0;   // 清除中断信号（疑似CYT芯片BUG，此处操作实测可清除该端口所有外部中断信号，若移植至其他芯片，可删除这句） 
    }
    else
    {
        exti_enable(MOTOR_RIGHT_SENSORLESS_A_TRIG);                             // 开启 A相过零信号触发 中断

        exti_enable(MOTOR_RIGHT_SENSORLESS_B_TRIG);                             // 开启 B相过零信号触发 中断

        exti_enable(MOTOR_RIGHT_SENSORLESS_C_TRIG);                             // 开启 C相过零信号触发 中断
      
        get_port(MOTOR_RIGHT_SENSORLESS_A_TRIG)->unINTR.stcField.u1IN_IN0 = 0;  // 清除中断信号（疑似CYT芯片BUG，此处操作实测可清除该端口所有外部中断信号，若移植至其他芯片，可删除这句） 
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式 过零信号触发 失能
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_trig_disable(&motor_left);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void sensorless_motor_trig_disable(motor_struct *motor_value)
{
    if(motor_value->motor_type == LEFT_MOTOR)                                   // 判断是否为左侧电机
    {
        exti_disable(MOTOR_LEFT_SENSORLESS_A_TRIG);                             // 关闭 A相过零信号触发 中断
                                                                                
        exti_disable(MOTOR_LEFT_SENSORLESS_B_TRIG);                             // 关闭 B相过零信号触发 中断
                                                                                
        exti_disable(MOTOR_LEFT_SENSORLESS_C_TRIG);                             // 关闭 C相过零信号触发 中断
        
        get_port(MOTOR_LEFT_SENSORLESS_A_TRIG)->unINTR.stcField.u1IN_IN0 = 0;   // 清除中断信号（疑似CYT芯片BUG，此处操作实测可清除该端口所有外部中断信号，若移植至其他芯片，可删除这句）
    }
    else
    {
        exti_disable(MOTOR_RIGHT_SENSORLESS_A_TRIG);                            // 关闭 A相过零信号触发 中断
                                                                                
        exti_disable(MOTOR_RIGHT_SENSORLESS_B_TRIG);                            // 关闭 B相过零信号触发 中断
                                                                                
        exti_disable(MOTOR_RIGHT_SENSORLESS_C_TRIG);                            // 关闭 C相过零信号触发 中断
        
        get_port(MOTOR_RIGHT_SENSORLESS_A_TRIG)->unINTR.stcField.u1IN_IN0 = 0;  // 清除中断信号（疑似CYT芯片BUG，此处操作实测可清除该端口所有外部中断信号，若移植至其他芯片，可删除这句）
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     电机换相步骤自增
// 参数说明     motor_value         电机结构体
// 返回参数     void
// 使用示例     motor_next_step(&motor_left);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_next_step(motor_struct *motor_value)
{
    if(motor_value->motor_duty != 0)
    {
        motor_value->save_duty = motor_value->motor_duty;
    }

    if(motor_value->motor_duty >= 0 && motor_value->save_duty > 0)
    {
        motor_value->sensorless_step ++;                                        // 电机步骤自增

        if(6 < motor_value->sensorless_step)                                    // 如果大于最大步骤
        {
            motor_value->sensorless_step = 1;                                   // 循环回到第1步
        }
    }
    else if(motor_value->motor_duty <= 0 && motor_value->save_duty < 0)
    {
        motor_value->sensorless_step --;                                        // 电机步骤自增

        if(1 > motor_value->sensorless_step)                                    // 如果大于最大步骤
        {
            motor_value->sensorless_step = 6;                                   // 循环回到第6步
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式左侧电机占空比输出函数
// 参数说明     phase_a     A相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 参数说明     phase_b     B相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 参数说明     phase_c     C相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 返回参数     void
// 使用示例     motor_left_sensorless_pwm_output(ouput_duty, 0, -1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_sensorless_pwm_output(int16 phase_a, int16 phase_b, int16 phase_c)
{      
    phase_a = func_limit_ab(phase_a, -1, 10000);                                // A相输出限幅
                                                                                                
    phase_b = func_limit_ab(phase_b, -1, 10000);                                // B相输出限幅
                                                                                
    phase_c = func_limit_ab(phase_c, -1, 10000);                                // C相输出限幅
  
    gpio_low(MOTOR_LEFT_SENSORLESS_A_PHASE_L);                                  // 关闭A相下桥    
    
    gpio_low(MOTOR_LEFT_SENSORLESS_B_PHASE_L);                                  // 关闭B相下桥        
    
    gpio_low(MOTOR_LEFT_SENSORLESS_C_PHASE_L);                                  // 关闭C相下桥        
    
    if(phase_a > 0)
    {
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_B_PHASE_H, 0);                       // 配置B相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_C_PHASE_H, 0);                       // 配置C相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_A_PHASE_H, phase_a);                 // 配置A相上桥占空比    
    }                           
    else if(phase_b > 0)                                
    {                           
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_A_PHASE_H, 0);                       // 配置A相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_C_PHASE_H, 0);                       // 配置C相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_B_PHASE_H, phase_b);                 // 配置B相上桥占空比
    }                           
    else if(phase_c > 0)                                
    {                           
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_A_PHASE_H, 0);                       // 配置A相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_B_PHASE_H, 0);                       // 配置B相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_C_PHASE_H, phase_c);                 // 配置C相上桥占空比
    }                           
    else                                
    {                           
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_A_PHASE_H, 0);                       // 配置A相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_B_PHASE_H, 0);                       // 配置C相上桥占空比 
                        
        pwm_set_duty(MOTOR_LEFT_SENSORLESS_C_PHASE_H, 0);                       // 配置B相上桥占空比
    }
    
    if(phase_a > 0 || phase_b > 0 || phase_c > 0)
    {
        if(phase_a == 0)                                                        // 判断是否需要开启A相下桥
        {   
            gpio_high(MOTOR_LEFT_SENSORLESS_A_PHASE_L);                         // 开启A相下桥
        }
        else if(phase_b == 0)                                                   // 判断是否需要开启B相下桥
        {                                                                        
            gpio_high(MOTOR_LEFT_SENSORLESS_B_PHASE_L);                         // 开启B相下桥
        }
        else if(phase_c == 0)                                                   // 判断是否需要开启C相下桥
        {                                                                        
            gpio_high(MOTOR_LEFT_SENSORLESS_C_PHASE_L);                         // 开启C相下桥
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式右侧电机占空比输出函数
// 参数说明     phase_a     A相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 参数说明     phase_b     B相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 参数说明     phase_c     C相占空比输出   0：关闭上桥 开启下桥   1~10000：上桥输出占空比 关闭下桥；   -1：关闭上下桥
// 返回参数     void
// 使用示例     motor_right_sensorless_pwm_output(ouput_duty, 0, -1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_sensorless_pwm_output(int16 phase_a, int16 phase_b, int16 phase_c)
{      
    phase_a = func_limit_ab(phase_a, -1, 10000);                                // A相输出限幅
                                                                                
    phase_b = func_limit_ab(phase_b, -1, 10000);                                // B相输出限幅
                                                                                
    phase_c = func_limit_ab(phase_c, -1, 10000);                                // C相输出限幅
  
    gpio_low(MOTOR_RIGHT_SENSORLESS_A_PHASE_L);                                 // 关闭A相下桥    
    
    gpio_low(MOTOR_RIGHT_SENSORLESS_B_PHASE_L);                                 // 关闭B相下桥        
    
    gpio_low(MOTOR_RIGHT_SENSORLESS_C_PHASE_L);                                 // 关闭C相下桥        

    if(phase_a > 0)
    {
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_B_PHASE_H, 0);                      // 配置B相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_C_PHASE_H, 0);                      // 配置C相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_A_PHASE_H, phase_a);                // 配置A相上桥占空比    
    }
    else if(phase_b > 0)
    {
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_A_PHASE_H, 0);                      // 配置A相上桥占空比 
            
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_C_PHASE_H, 0);                      // 配置C相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_B_PHASE_H, phase_b);                // 配置B相上桥占空比
    }
    else if(phase_c > 0)
    {
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_A_PHASE_H, 0);                      // 配置A相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_B_PHASE_H, 0);                      // 配置B相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_C_PHASE_H, phase_c);                // 配置C相上桥占空比
    }
    else
    {
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_A_PHASE_H, 0);                      // 配置A相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_B_PHASE_H, 0);                      // 配置C相上桥占空比 
        
        pwm_set_duty(MOTOR_RIGHT_SENSORLESS_C_PHASE_H, 0);                      // 配置B相上桥占空比
    }
    
    if(phase_a > 0 || phase_b > 0 || phase_c > 0)
    {
        if(phase_a == 0)                                                        // 判断是否需要开启A相下桥
        {   
            gpio_high(MOTOR_RIGHT_SENSORLESS_A_PHASE_L);                        // 开启A相下桥
        }
        else if(phase_b == 0)                                                   // 判断是否需要开启B相下桥
        {                                                                        
            gpio_high(MOTOR_RIGHT_SENSORLESS_B_PHASE_L);                        // 开启B相下桥
        }
        else if(phase_c == 0)                                                   // 判断是否需要开启C相下桥
        {                                                                        
            gpio_high(MOTOR_RIGHT_SENSORLESS_C_PHASE_L);                        // 开启C相下桥
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     电机换相函数
// 参数说明     motor_value         电机结构体
// 参数说明     ouput_duty          需要输出的占空比数据
// 参数说明     trig_state          是否开启对应的过零信号触发中断  0 不开启  1 开启
// 返回参数     void
// 使用示例     motor_commutation(&motor_left, (int16)motor_left.sensorless_duty, 1);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_commutation(motor_struct *motor_value, int16 ouput_duty, uint8 trig_state)
{
    if(motor_value->motor_type == LEFT_MOTOR)                                   // 判断是否为左侧电机
    {
        switch(motor_value->sensorless_step)                                    // 判断当前需要输出的相位
        {
            case 1:
            {
                motor_left_sensorless_pwm_output(ouput_duty, 0, -1);            // A 上桥  →  B 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {
                    exti_enable(MOTOR_LEFT_SENSORLESS_C_TRIG);                  // 使能对应相位的 过零信号触发中断
                }
                
            }break;
            
            case 2:
            {
                motor_left_sensorless_pwm_output(ouput_duty, -1, 0);            // A 上桥  →  C 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_LEFT_SENSORLESS_B_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 3:
            {
                motor_left_sensorless_pwm_output(-1, ouput_duty, 0);            // B 上桥  →  C 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_LEFT_SENSORLESS_A_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 4:
            {
                motor_left_sensorless_pwm_output(0, ouput_duty, -1);            // B 上桥  →  A 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_LEFT_SENSORLESS_C_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 5:
            {
                motor_left_sensorless_pwm_output(0, -1, ouput_duty);            // C 上桥  →  A 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_LEFT_SENSORLESS_B_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 6:
            {
                motor_left_sensorless_pwm_output(-1, 0, ouput_duty);            // C 上桥  →  B 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_LEFT_SENSORLESS_A_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            default:break;
        }
    }
    else
    {
        switch(motor_value->sensorless_step)                                    // 判断当前需要输出的相位
        {
            case 1:
            {
                motor_right_sensorless_pwm_output(ouput_duty, 0, -1);            // A 上桥  →  B 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {
                    exti_enable(MOTOR_RIGHT_SENSORLESS_C_TRIG);                  // 使能对应相位的 过零信号触发中断
                }
                
            }break;
            
            case 2:
            {
                motor_right_sensorless_pwm_output(ouput_duty, -1, 0);            // A 上桥  →  C 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_RIGHT_SENSORLESS_B_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 3:
            {
                motor_right_sensorless_pwm_output(-1, ouput_duty, 0);            // B 上桥  →  C 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_RIGHT_SENSORLESS_A_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 4:
            {
                motor_right_sensorless_pwm_output(0, ouput_duty, -1);            // B 上桥  →  A 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_RIGHT_SENSORLESS_C_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 5:
            {
                motor_right_sensorless_pwm_output(0, -1, ouput_duty);            // C 上桥  →  A 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_RIGHT_SENSORLESS_B_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            case 6:
            {
                motor_right_sensorless_pwm_output(-1, 0, ouput_duty);            // C 上桥  →  B 下桥
                
                if(trig_state)                                                  // 判断是否开启对应的 过零信号触发中断
                {                                                                 
                    exti_enable(MOTOR_RIGHT_SENSORLESS_A_TRIG);                  // 使能对应相位的 过零信号触发中断 
                }

            }break;
            
            default:break;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式左侧电机延时换相配置
// 参数说明     delay_time          延时换相时间
// 返回参数     void
// 使用示例     motor_left_sensorless_delay_output(20);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_left_sensorless_delay_output(uint16 delay_time)
{    
    Cy_Tcpwm_Counter_SetPeriod(MOTOR_LEFT_SENSORLESS_DELAY_TIMER, delay_time);  // 配置定时器周期为延时时长 

    Cy_Tcpwm_Counter_SetCounter(MOTOR_LEFT_SENSORLESS_DELAY_TIMER, 0);          // 清除当前定时器计数值

    Cy_Tcpwm_Counter_Enable(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);                 // 使能延时换相定时器
    
    Cy_Tcpwm_TriggerStart(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);                   // 触发定时器启动计数
}           


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式左侧电机触发换相中断
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_left_trig_isr();
// 备注信息     过零信号触发中断，用于换相
//-------------------------------------------------------------------------------------------------------------------
void motor_left_sensorless_delay_isr(void)
{    
    Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);           // 清除延迟换相中断标志位
    
    Cy_Tcpwm_Counter_Disable(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);                // 失能延时换相定时器
    
    if(motor_left.sensorless_state == SENSORLESS_CLOSE_LOOP)                    // 判断当前是否在闭环模式  若不在闭环模式则不执行任何 控制程序
    {
        motor_next_step(&motor_left);                                           // 输出相位自增
            
        motor_commutation(&motor_left, (int16)(func_abs(motor_left.sensorless_duty) * 10000), 1);            // 输出当前相位 打开对应的过零信号检测中断
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式左侧电机触发换相中断
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_left_trig_isr();
// 备注信息     过零信号触发中断，用于换相
//-------------------------------------------------------------------------------------------------------------------
void sensorless_motor_left_trig_isr(void)
{       
    motor_left.isr_trig_count ++;                                               // 过零信号 触发计数自增
    
    exti_flag_get(MOTOR_LEFT_SENSORLESS_A_TRIG);                                // 清除 A相 触发信号
    
    exti_flag_get(MOTOR_LEFT_SENSORLESS_B_TRIG);                                // 清除 B相 触发信号
    
    exti_flag_get(MOTOR_LEFT_SENSORLESS_C_TRIG);                                // 清除 C相 触发信号
    
    if(motor_left.sensorless_state == SENSORLESS_CLOSE_LOOP)                    // 判断当前是否在闭环模式  若不在闭环模式则不执行任何 控制程序
    {        
        motor_left.step_change_time_us = timer_get(LEFT_MOTOR_COUNT_TIMER);     // 获取本次换相时间 单位 us
        
        timer_clear(LEFT_MOTOR_COUNT_TIMER);                                    // 清除计数值
        
        motor_left.commutation_time_sum -= motor_left.commutation_time[motor_left.sensorless_step - 1];                                         // 移除 最早记录的 换相时间

        motor_left.commutation_time[motor_left.sensorless_step - 1] = (motor_left.step_change_time_us > 6000 ? 6000 : motor_left.step_change_time_us);                // 记录本次换相时间 限幅 6000us

        motor_left.commutation_time_sum += motor_left.commutation_time[motor_left.sensorless_step - 1];                                         // 叠加本次换相时间 总共将记录 最近6次的换相耗时(对应360°电角度)

        motor_left.filter_commutation_time_sum = (motor_left.filter_commutation_time_sum * 3 + motor_left.commutation_time_sum * 1) / 4;        // 对总时长进行低通滤波
               
        sensorless_motor_trig_disable(&motor_left);                             // 失能 过零信号 触发中断（将在输出函数中打开指定相位的触发中断）
        
        // 以下为延时换相部分，电机的带载不同则延时换相参数可能不同

        // 调节延时换相时间可实现修改电机工作效率
        
        if(motor_left.isr_trig_count < 100)                                     // 若当前 过零信号 触发计数值小于 100，则认为刚进入闭环 执行默认延时换相
        {
            motor_left_sensorless_delay_output(20);                             // 延时 20us 后执行换相
        }
        else if(motor_left.sensorless_duty < 0.4f)                             // 若用户控制占空比低于 40%，延时换相控制 换相角度 至 132°
        {
            motor_left_sensorless_delay_output(motor_left.filter_commutation_time_sum * 3 / 60);                                                // 执行延时换相
        }
        else                                                                    // 若用户控制占空比高于 40%，延时换相控制 换相角度 逐渐从 132°递增到 144° 对应 40% ~ 100% 占空比
        {            
            float wait_time = 30.0f - (motor_left.sensorless_duty - 0.4f) * 30.0f / 0.6f;                                                       // 计算 延时换相时间
            
            motor_left_sensorless_delay_output((uint32)((float)motor_left.filter_commutation_time_sum * wait_time / 600.0f));                   // 执行延时换相
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式右侧电机延时换相配置
// 参数说明     delay_time          延时换相时间
// 返回参数     void
// 使用示例     motor_right_sensorless_delay_output(20);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void motor_right_sensorless_delay_output(uint16 delay_time)
{    
    Cy_Tcpwm_Counter_SetPeriod(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER, delay_time);  // 配置定时器周期为延时时长 

    Cy_Tcpwm_Counter_SetCounter(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER, 0);          // 清除当前定时器计数值

    Cy_Tcpwm_Counter_Enable(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);                 // 使能延时换相定时器
    
    Cy_Tcpwm_TriggerStart(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);                   // 触发定时器启动计数
}  

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式右侧电机触发换相中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_right_sensorless_delay_isr();
// 备注信息     过零信号触发中断，用于换相
//-------------------------------------------------------------------------------------------------------------------
void motor_right_sensorless_delay_isr(void)
{    
    Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);          // 清除延迟换相中断标志位
    
    Cy_Tcpwm_Counter_Disable(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);               // 失能延时换相定时器
    
    if(motor_right.sensorless_state == SENSORLESS_CLOSE_LOOP)                   // 判断当前是否在闭环模式  若不在闭环模式则不执行任何 控制程序
    {      
        motor_next_step(&motor_right);                                          // 输出相位自增
            
        motor_commutation(&motor_right, (int16)(func_abs(motor_right.sensorless_duty) * 10000), 1);          // 输出当前相位 打开对应的过零信号检测中断
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式右侧电机触发换相中断
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_right_trig_isr();
// 备注信息     过零信号触发中断，用于换相
//-------------------------------------------------------------------------------------------------------------------
void sensorless_motor_right_trig_isr(void)
{
    motor_right.isr_trig_count ++;                                              // 过零信号 触发计数自增
    
    exti_flag_get(MOTOR_RIGHT_SENSORLESS_A_TRIG);                               // 清除 A相 触发信号
    
    exti_flag_get(MOTOR_RIGHT_SENSORLESS_B_TRIG);                               // 清除 B相 触发信号
    
    exti_flag_get(MOTOR_RIGHT_SENSORLESS_C_TRIG);                               // 清除 C相 触发信号
    
    if(motor_right.sensorless_state == SENSORLESS_CLOSE_LOOP)                   // 判断当前是否在闭环模式  若不在闭环模式则不执行任何 控制程序
    {        
        motor_right.step_change_time_us = timer_get(RIGHT_MOTOR_COUNT_TIMER);   // 获取本次换相时间 单位 us
        
        timer_clear(RIGHT_MOTOR_COUNT_TIMER);                                   // 清除计数值
        
        motor_right.commutation_time_sum -= motor_right.commutation_time[motor_right.sensorless_step - 1];                                         // 移除 最早记录的 换相时间

        motor_right.commutation_time[motor_right.sensorless_step - 1] = (motor_right.step_change_time_us > 6000 ? 6000 : motor_right.step_change_time_us);                // 记录本次换相时间 限幅 6000us

        motor_right.commutation_time_sum += motor_right.commutation_time[motor_right.sensorless_step - 1];                                         // 叠加本次换相时间 总共将记录 最近6次的换相耗时(对应360°电角度)

        motor_right.filter_commutation_time_sum = (motor_right.filter_commutation_time_sum * 3 + motor_right.commutation_time_sum) / 4;            // 对总时长进行低通滤波
        
        sensorless_motor_trig_disable(&motor_right);                            // 失能 过零信号 触发中断（将在输出函数中打开指定相位的触发中断）
        
        // 以下为延时换相部分，电机的带载不同则延时换相参数可能不同

        // 调节延时换相时间可实现修改电机工作效率
        if(motor_right.isr_trig_count < 100)                                    // 若当前 过零信号 触发计数值小于 100，则认为刚进入闭环 执行默认延时换相
        {
            motor_right_sensorless_delay_output(20);                            // 延时 20us 后执行换相
        }
        else if(motor_right.sensorless_duty < 0.4f)                             // 若用户控制占空比低于 40%，延时换相控制 换相角度 至 132°
        {
            motor_right_sensorless_delay_output(motor_right.filter_commutation_time_sum * 3 / 60);                                                // 执行延时换相
        }
        else                                                                    // 若用户控制占空比高于 40%，延时换相控制 换相角度 逐渐从 132°递增到 144° 对应 40% ~ 100% 占空比
        {            
            float wait_time = 30.0f - (motor_right.sensorless_duty - 0.4f) * 30.0f / 0.6f;                                                      // 计算 延时换相时间
            
            motor_right_sensorless_delay_output((uint32)((float)motor_right.filter_commutation_time_sum * wait_time / 600.0f));                 // 执行延时换相
        }
    }
  
  
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式 周期中断
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_motor_isr();
// 备注信息     周期时间 50us，其中部分参数依赖周期时间，不可修改该中断执行周期
//-------------------------------------------------------------------------------------------------------------------
void sensorless_motor_isr(void)
{         
    /*************************左侧无感驱动模式 周期循环执行内容*************************/ 
    motor_left.pit_trig_count ++;                                               // 计数值自增 
    
#if USER_CONTROL_MODE == 1                                                      // pwm脉宽控制模式
    if(motor_left_pwm_in_data.pwm_high_level >= 1050 && motor_left_pwm_in_data.pwm_high_level < 2020)           // 判断控制脉宽的高电平是否大于 1.05ms 并且小于 2.02ms
    {
        motor_left.motor_duty = func_limit_ab((motor_left_pwm_in_data.pwm_high_level - 1000) / 1000.0f, 0, 1);  // 计算目标占空比

        motor_left.motor_duty  = motor_left.motor_duty * (SENSERLESS_MOTOR_LEFT_DIR == 0 ? 1 : -1); 
    }
    else
    {
        motor_left.motor_duty = 0;                                                                     
    }
#endif    
    
    switch(motor_left.sensorless_state)                                         // 判断当前 无感 驱动状态
    {                                                                           
        case SENSORLESS_OPEN_LOOP_SINE:                                         // 开环 正弦牵引状态 
        {                                                                       
            if(motor_left.pit_trig_count <= 500)                                // 启动的前25ms 检测电机是否在旋转 如果在旋转则直接进入开环BLDC牵引 无需开环正弦牵引
            {
                motor_left.sensorless_value_last = motor_left.sensorless_value_now;                                   // 保存上次 过零信号 相位值
            
                motor_left.sensorless_value_now =  gpio_get_level(MOTOR_LEFT_SENSORLESS_A_TRIG) * 4 +                 // 计算本次 过零信号 相位值  
                                                   gpio_get_level(MOTOR_LEFT_SENSORLESS_B_TRIG) * 2 +
                                                   gpio_get_level(MOTOR_LEFT_SENSORLESS_C_TRIG);    
                
                if(motor_left.sensorless_value_last != motor_left.sensorless_value_now)                               // 检测到过零换相
                {
                    motor_left.common_change_phase_count ++;                    // 换相计数值自增
                }                                                                
                                                                                 
                if(motor_left.pit_trig_count == 500)                            // 25ms整 判断换相次数
                {                                                                
                    if(motor_left.common_change_phase_count > 50)               // 若换相超过50次(电机转速超过 2858 RPM)  则直接进入开环BLDC牵引
                    {                                                            
                        sensorless_pwm_output_init(&motor_left);                // 重新配置定时器
                                                                                 
                        motor_left.overtime_change_phase_count = 0;             // 清除换相超时计数值
                                                                                 
                        motor_left.common_change_phase_count = 0;               // 清除正常换相计数值
                                                                                 
                        timer_start(LEFT_MOTOR_COUNT_TIMER);                    // 启动 微秒计数 定时器
                        
                        motor_left.sensorless_state = SENSORLESS_OPEN_LOOP_BLDC;// 驱动状态 修正为 开环 BLDC 牵引状态
                    }
                }
            }
            else                                                                // 否则正常执行正弦牵引
            {
                if(sensorless_start_sine(&motor_left))                          // 执行正弦牵引函数 判断其是否牵引完成
                {
                    motor_left.overtime_change_phase_count = 0;                 // 清除换相超时计数值
                    
                    motor_left.common_change_phase_count = 0;                   // 清除正常换相计数值
                    
                    timer_start(LEFT_MOTOR_COUNT_TIMER);                        // 启动 微秒计数 定时器
                        
                    motor_left.sensorless_state = SENSORLESS_OPEN_LOOP_BLDC;           // 驱动状态 修正为 开环 BLDC 牵引状态
                }
            }
        }break;
        
        case SENSORLESS_OPEN_LOOP_BLDC:                                         // 开环 BLDC 牵引状态
        {         
            motor_left.sensorless_value_last = motor_left.sensorless_value_now; // 保存上次 过零信号 相位值
            
            motor_left.sensorless_value_now = gpio_get_level(MOTOR_LEFT_SENSORLESS_A_TRIG) * 4 +                                // 计算本次 过零信号 相位值  
                                              gpio_get_level(MOTOR_LEFT_SENSORLESS_B_TRIG) * 2 +
                                              gpio_get_level(MOTOR_LEFT_SENSORLESS_C_TRIG);         
            
            motor_left.step_change_time_us = timer_get(LEFT_MOTOR_COUNT_TIMER); // 获取当前 定时器 计数时间 
            
            if(motor_left.sensorless_value_last != motor_left.sensorless_value_now || motor_left.step_change_time_us > 5000)    // 出现换相 或者 计数时间超过5000微秒 切换输出 
            {         
                timer_clear(LEFT_MOTOR_COUNT_TIMER);                            // 清除定时器计数值
              
                if(motor_left.step_change_time_us > 5000 || motor_left.step_change_time_us < 500)                               // 若超过 5000 微秒 或者 低于 500微秒 则认为换相超时(错误)  
                {
                    motor_left.overtime_change_phase_count ++;                  // 换相超时 计数值自增
                }
                else                                                            // 否则为正常换相
                {       
                    motor_left.common_change_phase_count ++;                    // 正常换相 计数值自增
                }
                
                motor_next_step(&motor_left);                                   // 切换下一步 
                
                motor_commutation(&motor_left, SENSERLESS_OPENLOOP_BLDC_DUTY, 0);// 按照当前相位输出 不开启换相过零检测中断
                
                if(motor_left.overtime_change_phase_count > ((SENSERLESS_OPENLOOP_BLDC_TURNS - SENSERLESS_CLOSELOOP_CHECK) * 6))// 若换相超时计数值 超过 250次 认为牵引启动失败 结束本次开环牵引 (此为启动保护 确保能够正确开环牵引)
                {
                    motor_left.overtime_change_phase_count = 0;                 // 清除换相超时计数值
                    
                    motor_left.pit_trig_count = 0;                              // 清除当前周期定时器计数值
                    
                    motor_commutation(&motor_left, 0, 0);                       // 关闭输出
                    
                    motor_left.sensorless_state = SENSORLESS_RESTART;           // 驱动状态 修正为 重启状态
                }
                  
                if(motor_left.common_change_phase_count > (SENSERLESS_CLOSELOOP_CHECK * 6))                                     // 若正常换相次数超过 50 次 在认为可以进入闭环模式 结束本次开环牵引 
                {                    
                    if(motor_left.motor_duty >= 0)
                    {
                        motor_left.sensorless_duty =  (SENSERLESS_OPENLOOP_BLDC_DUTY / 10000.0f); // 输出占空比赋初值 从10%占空比启动
                    }
                    else
                    {
                        motor_left.sensorless_duty = -(SENSERLESS_OPENLOOP_BLDC_DUTY / 10000.0f); // 输出占空比赋初值 从10%占空比启动
                    }
                    motor_left.pit_trig_count = 0;                              // 清除当前周期定时器计数值
                    
                    motor_left.isr_trig_count = 0;                              // 清除当前过零信号触发计数值
                    
                    sensorless_motor_trig_enable(&motor_left);                  // 使能 过零信号 触发中断        
                    
                    timer_clear(LEFT_MOTOR_COUNT_TIMER);                        // 清除定时器计数值
                    
                    motor_left.sensorless_state = SENSORLESS_CLOSE_LOOP;        // 驱动状态 修正为 闭环驱动状态
                }
            }
        }break;
        
        case SENSORLESS_CLOSE_LOOP:                                             // 闭环驱动状态
        {      
            if(motor_left.isr_trig_count > 500)
            {
                motor_left.motor_speed = 60000000 / (motor_left.filter_commutation_time_sum * 7);                               // 计算当前电机转速

                if(motor_left.motor_duty < 0.0f)
                {
                    motor_left.motor_speed = -motor_left.motor_speed;
                }
                
                motor_left.motor_speed_filter = (motor_left.motor_speed + motor_left.motor_speed_filter * 99.0f) / 100.0f;      // 对转速进行低通滤波
            }
            
            if(motor_left.pit_trig_count > 1000 && motor_left.pit_trig_count % SENSERLESS_SPEED_INCREMENTAL == 0)               // 缓加速 & 缓减速 进入闭环 50ms 后 根据用户配置的加减速步长 执行加减速
            {
                if(motor_left.sensorless_duty < motor_left.motor_duty)
                {
                    motor_left.sensorless_duty += (func_limit_ab(func_abs(motor_left.sensorless_duty) * 0.0003, 0.0001, 0.0003));  
                }
                else if(motor_left.sensorless_duty > motor_left.motor_duty)
                {
                    motor_left.sensorless_duty -= (func_limit_ab(func_abs(motor_left.sensorless_duty) * 0.0003, 0.0001, 0.0003));  
                }
                
                motor_left.sensorless_duty = func_limit_ab(motor_left.sensorless_duty, -1.0f, 1.0f);
            }
            
            if(motor_left.pit_trig_count % 20 == 0)                             // 1ms 执行一次 记录闭环速度数据 用于高速抗干扰保护
            {
                motor_left.speed_record[motor_left.speed_record_count] = (int32)func_abs(motor_left.motor_speed_filter);// 保存当前速度
                
                motor_left.speed_change   = motor_left.speed_record[motor_left.speed_record_count] -                    // 计算 100ms 内的速度 变化
                                            motor_left.speed_record[(motor_left.speed_record_count == 99 ? 0 : motor_left.speed_record_count + 1)];
                
                if(++ motor_left.speed_record_count >= 100)                     // 速度保存数组 循环计数
                {
                    motor_left.speed_record_count = 0;
                }     
                
                motor_left.duty_change = (int32)(motor_left.motor_duty * 10000) - motor_left.duty_record; // 计算占空比 变化
                
                motor_left.duty_record = (int32)(motor_left.motor_duty * 10000);// 保存当前占空比数据
                
                if(func_abs(motor_left.duty_change) >= 100)
                {
                    motor_left.duty_record_count = 1;
                }
                else
                {
                    if(motor_left.duty_record_count > 0 && motor_left.duty_record_count < 1000)
                    {
                        motor_left.duty_change = 100;
                        
                        if(++ motor_left.duty_record_count == 1000)
                        {
                            motor_left.duty_record_count = 0;
                            
                            motor_left.duty_change = 0;
                        }
                    }
                }
            }
            
            if(motor_left.isr_trig_count_save == motor_left.isr_trig_count)     // 若 过零换相中断 50微秒内未触发 则记录
            {
                motor_left.overtime_change_phase_count ++;                      // 换相超时 计数值自增
            }
            else                                                                // 若 50微秒内触发 过零换相中断 则清除记录
            {
                motor_left.overtime_change_phase_count = 0;                     // 换相超时 计数值清除
            }
            
            if((motor_left.filter_commutation_time_sum > 5000 && motor_left.pit_trig_count > 5000) ||                                   // 闭环运行 0.5s 后 若电机转速低于 285RPM 则认为 转速过低 关闭输出 
#if     MOTOR_DISTURB_PROTECT == DRIVER_ENABLE            
               (motor_left.speed_change < -MOTOR_DISTURB_SPEED && func_abs(motor_left.duty_change) < 100 && motor_left.pit_trig_count > 5000) ||       // 闭环运行 0.5s 后 若电机转速突然下降(默认 500RPM) 则认为 受到异常干扰 关闭输出 
#endif                 
               (motor_left.overtime_change_phase_count > 1000)          ||                                                              // 若 50ms 未触发过零换相 则认为 电机堵转 关闭输出                
                
               (func_abs(motor_left.sensorless_duty) < 0.05)            ||                                                              // 若 占空比低于 5%  则认为 输出过低 关闭输出
                 
               (motor_left.motor_protect_state  == PROTECT_MODE)        ||                                                              // 若电机进入保护状态 则关闭输出                 
                 
               (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR) ||                                     // 若电池电压进入保护状态 且使能有效 则关闭输出   
                   
               ((motor_left.sensorless_duty * 10000.0f) * motor_left.motor_duty < 0.0f))                                                // 若占空比方向改变 则关闭输出                                                           
            {
                motor_left.sensorless_state = SENSORLESS_RESTART;               // 驱动状态 修正为 重启状态
                
                Cy_Tcpwm_Counter_Disable(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);    // 关闭 延时换相 定时器
                
                sensorless_motor_trig_disable(&motor_left);                     // 关闭 过零信号触发中断
                
                motor_commutation(&motor_left, 0, 0);                           // 关闭 输出
                
                motor_left.pit_trig_count = 0;                                  // 清除当前周期定时器计数值
                
                motor_left.speed_change = 0;                                    // 清除电机转速变化记录值
                
                motor_left.duty_change = 0;                                     // 清除电机占空比变化记录值
                
                motor_left.motor_speed = 0;                                     // 清除电机转速数据
                
                motor_left.motor_speed_filter = 0;                              // 清除电机转速滤波数据
            }
            
            motor_left.isr_trig_count_save = motor_left.isr_trig_count;         // 保存当前 过零信号触发计数值
            

        }break;
        
        case SENSORLESS_RESTART:                                                // 电机重启状态
        {            
            motor_commutation(&motor_left, 0, 0);                               // 按照当前相位输出  0占空比  关闭电机
            
            if(func_abs(motor_left.motor_duty) > 0.06 && motor_left.pit_trig_count > SENSERLESS_RESTART_TIME * 20)// 如果占空比有效(大于6%) 并且已经等待了指定时长  尝试重新启动电机
            {
                motor_left.pit_trig_count = 0;                                  // 清除当前周期定时器计数值
                
                motor_left.sensorless_state = SENSORLESS_OPEN_LOOP_SINE;        // 驱动状态 修正为 开环正弦启动状态
            }
            if(motor_left.motor_duty == 0                               ||      // 如果占空比无效  则停止驱动
               
              (motor_left.motor_protect_state  == PROTECT_MODE)         ||      // 若电机进入保护状态 则停止驱动   
                
              (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))    // 若电池电压进入保护状态 且使能有效 则停止驱动                                                   
            {
                motor_left.sensorless_state = SENSORLESS_STOP_STATE;            // 驱动状态 修正为 停止状态
            }
        }break;
        
        case SENSORLESS_STOP_STATE:                                             // 电机停止状态
        {            
            motor_commutation(&motor_left, 0, 0);                               // 按照当前相位输出  0占空比  关闭电机
            
            if(func_abs(motor_left.motor_duty) > 0.06 && (motor_left.motor_protect_state  != PROTECT_MODE))  // 如果占空比有效(大于6%) 电机未进入保护状态 尝试重新启动电机
            {   
                motor_left.pit_trig_count = 0;                                  // 清除当前周期定时器计数值
                
                motor_left.sensorless_state = SENSORLESS_OPEN_LOOP_SINE;        // 驱动状态 修正为 开环正弦启动状态
            }
        }break;
        
        default:break;
    }
    /*************************左侧无感驱动模式 周期循环执行内容*************************/ 
    
    
    /*************************右侧无感驱动模式 周期循环执行内容*************************/ 
    
    motor_right.pit_trig_count ++;                                              // 计数值自增 
    
#if USER_CONTROL_MODE == 1                                                      // pwm脉宽控制模式
    if(motor_right_pwm_in_data.pwm_high_level >= 1050 && motor_right_pwm_in_data.pwm_high_level < 2020)                // 判断控制脉宽的高电平是否大于 1.05ms 并且小于 2.02ms
    {
        motor_right.motor_duty = func_limit_ab((motor_right_pwm_in_data.pwm_high_level - 1000) / 1000.0f, 0, 1);       // 计算目标占空比
        
        motor_right.motor_duty = motor_right.motor_duty * (SENSERLESS_MOTOR_RIGHT_DIR == 0 ? 1 : -1); 
    }
    else
    {
        motor_right.motor_duty = 0;                                                                     
    }
#endif
    
    switch(motor_right.sensorless_state)                                       // 判断当前 无感 驱动状态
    {  
        case SENSORLESS_OPEN_LOOP_SINE:                                         // 开环 正弦牵引状态 
        {
            if(motor_right.pit_trig_count <= 500)                               // 启动的前25ms 检测电机是否在旋转 如果在旋转则直接进入开环BLDC牵引 无需开环正弦牵引
            {
                motor_right.sensorless_value_last = motor_right.sensorless_value_now;                                   // 保存上次 过零信号 相位值
            
                motor_right.sensorless_value_now = gpio_get_level(MOTOR_RIGHT_SENSORLESS_A_TRIG) * 4 +                  // 计算本次 过零信号 相位值  
                                                   gpio_get_level(MOTOR_RIGHT_SENSORLESS_B_TRIG) * 2 +
                                                   gpio_get_level(MOTOR_RIGHT_SENSORLESS_C_TRIG);    
                
                if(motor_right.sensorless_value_last != motor_right.sensorless_value_now)                               // 检测到过零换相
                {
                    motor_right.common_change_phase_count ++;                   // 换相计数值自增
                }
                
                if(motor_right.pit_trig_count == 500)                           // 25ms整 判断换相次数
                {
                    if(motor_right.common_change_phase_count > 50)              // 若换相超过50次(电机转速超过 2858 RPM)  则直接进入开环BLDC牵引
                    {
                        sensorless_pwm_output_init(&motor_right);               // 重新配置定时器
                        
                        motor_right.overtime_change_phase_count = 0;            // 清除换相超时计数值
                    
                        motor_right.common_change_phase_count = 0;              // 清除正常换相计数值
                        
                        timer_start(RIGHT_MOTOR_COUNT_TIMER);                   // 启动 微秒计数 定时器
                        
                        motor_right.sensorless_state = SENSORLESS_OPEN_LOOP_BLDC;// 驱动状态 修正为 开环 BLDC 牵引状态
                    }
                       
                }
            }
            else                                                                // 否则正常执行正弦牵引
            {
                if(sensorless_start_sine(&motor_right))                         // 执行正弦牵引函数 判断其是否牵引完成
                {
                    motor_right.overtime_change_phase_count = 0;                // 清除换相超时计数值
                    
                    motor_right.common_change_phase_count = 0;                  // 清除正常换相计数值
                    
                    timer_start(RIGHT_MOTOR_COUNT_TIMER);                       // 启动 微秒计数 定时器
                    
                    motor_right.sensorless_state = SENSORLESS_OPEN_LOOP_BLDC;   // 驱动状态 修正为 开环 BLDC 牵引状态
                }
            }
            
            
        }break;
        
        case SENSORLESS_OPEN_LOOP_BLDC:                                         // 开环 BLDC 牵引状态
        {         
            motor_right.sensorless_value_last = motor_right.sensorless_value_now;// 保存上次 过零信号 相位值
            
            motor_right.sensorless_value_now = gpio_get_level(MOTOR_RIGHT_SENSORLESS_A_TRIG) * 4 +                                // 计算本次 过零信号 相位值  
                                               gpio_get_level(MOTOR_RIGHT_SENSORLESS_B_TRIG) * 2 +
                                               gpio_get_level(MOTOR_RIGHT_SENSORLESS_C_TRIG);         
            
            motor_right.step_change_time_us = timer_get(RIGHT_MOTOR_COUNT_TIMER);// 获取当前 定时器 计数时间 
            
            if((motor_right.sensorless_value_last != motor_right.sensorless_value_now || motor_right.step_change_time_us > 5000)) // 出现换相 或者 计数时间超过5000微秒 切换输出 
            {         
                timer_clear(RIGHT_MOTOR_COUNT_TIMER);                           // 清除定时器计数值
              
                if(motor_right.step_change_time_us > 5000 || motor_right.step_change_time_us < 500)                               // 若超过 5000 微秒 或者 低于 500微秒 则认为换相超时(错误)  
                {
                    motor_right.overtime_change_phase_count ++;                 // 换相超时 计数值自增
                }
                else                                                            // 否则为正常换相
                {       
                    motor_right.common_change_phase_count ++;                   // 正常换相 计数值自增
                }
                
                motor_next_step(&motor_right);                                  // 切换下一步 
                
                motor_commutation(&motor_right, SENSERLESS_OPENLOOP_BLDC_DUTY, 0);// 按照当前相位输出 不开启换相过零检测中断
                
                if(motor_right.overtime_change_phase_count > ((SENSERLESS_OPENLOOP_BLDC_TURNS - SENSERLESS_CLOSELOOP_CHECK) * 6)) // 若换相超时计数值 超过 250次 认为牵引启动失败 结束本次开环牵引 (此为启动保护 确保能够正确开环牵引)
                {
                    
                    motor_right.overtime_change_phase_count = 0;                // 清除换相超时计数值
                    
                    motor_right.pit_trig_count = 0;                             // 清除当前周期定时器计数值
                    
                    motor_commutation(&motor_right, 0, 0);                      // 关闭输出
                    
                    motor_right.sensorless_state = SENSORLESS_RESTART;          // 驱动状态 修正为 重启状态

                }
                  
                if(motor_right.common_change_phase_count > (SENSERLESS_CLOSELOOP_CHECK * 6))                                      // 若正常换相次数超过 50 次 在认为可以进入闭环模式 结束本次开环牵引 
                {  
                    if(motor_right.motor_duty >= 0.0f)
                    {
                        motor_right.sensorless_duty =  (SENSERLESS_OPENLOOP_BLDC_DUTY / 10000.0f);// 输出占空比赋初值 从10%占空比启动
                    }
                    else
                    {
                        motor_right.sensorless_duty = -(SENSERLESS_OPENLOOP_BLDC_DUTY / 10000.0f);// 输出占空比赋初值 从10%占空比启动
                    }
                    motor_right.pit_trig_count = 0;                             // 清除当前周期定时器计数值
                    
                    motor_right.isr_trig_count = 0;                             // 清除当前过零信号触发计数值
                    
                    sensorless_motor_trig_enable(&motor_right);                 // 使能 过零信号 触发中断        
                    
                    timer_clear(RIGHT_MOTOR_COUNT_TIMER);                       // 清除定时器计数值
                    
                    motor_right.sensorless_state = SENSORLESS_CLOSE_LOOP;       // 驱动状态 修正为 闭环驱动状态
                }
            }
        }break;
        
        case SENSORLESS_CLOSE_LOOP:                                             // 闭环驱动状态
        {         
            if(motor_right.isr_trig_count > 500)
            {
                motor_right.motor_speed = 60000000 / (motor_right.filter_commutation_time_sum * 7);                             // 计算当前电机转速

                if(motor_right.motor_duty < 0.0f)
                {
                    motor_right.motor_speed = -motor_right.motor_speed;
                }
                
                motor_right.motor_speed_filter = (motor_right.motor_speed + motor_right.motor_speed_filter * 99.0f) / 100.0f;    // 对转速进行低通滤波
            }
            
            if(motor_right.pit_trig_count > 1000 && motor_right.pit_trig_count % SENSERLESS_SPEED_INCREMENTAL == 0)              // 缓加速 & 缓减速 进入闭环 50ms 后 根据用户配置的加减速步长 执行加减速
            {
                if(motor_right.sensorless_duty < motor_right.motor_duty)
                {
                    motor_right.sensorless_duty += (func_limit_ab(func_abs(motor_right.sensorless_duty) * 0.0003, 0.00005, 0.0003));  
                }
                else if(motor_right.sensorless_duty > motor_right.motor_duty)
                {
                    motor_right.sensorless_duty -= (func_limit_ab(func_abs(motor_right.sensorless_duty) * 0.0003, 0.00005, 0.0003));
                }
                
                motor_right.sensorless_duty = func_limit_ab(motor_right.sensorless_duty, -1.0f, 1.0f);
            }
            
            if(motor_right.pit_trig_count % 20 == 0)                            // 1ms 执行一次 记录闭环速度数据 用于高速抗干扰保护
            {
                motor_right.speed_record[motor_right.speed_record_count] = (int32)func_abs(motor_right.motor_speed_filter);     // 保存当前速度
                
                motor_right.speed_change   = motor_right.speed_record[motor_right.speed_record_count] -                         // 计算 100ms 内的速度 变化
                                            motor_right.speed_record[(motor_right.speed_record_count == 99 ? 0 : motor_right.speed_record_count + 1)];
                
                if(++ motor_right.speed_record_count >= 100)                    // 速度保存数组 循环计数
                {
                    motor_right.speed_record_count = 0;
                }     
                
                motor_right.duty_change = (int32)(motor_right.motor_duty * 10000) - motor_right.duty_record; // 计算占空比 变化
                
                motor_right.duty_record = (int32)(motor_right.motor_duty * 10000);// 保存当前占空比数据
                
                if(func_abs(motor_right.duty_change) >= 100)
                {
                    motor_right.duty_record_count = 1;
                }
                else
                {
                    if(motor_right.duty_record_count > 0 && motor_right.duty_record_count < 1000)
                    {
                        motor_right.duty_change = 100;
                        
                        if(++ motor_right.duty_record_count == 1000)
                        {
                            motor_right.duty_record_count = 0;
                            
                            motor_right.duty_change = 0;
                        }
                    }
                }   
            }
            
            if(motor_right.isr_trig_count_save == motor_right.isr_trig_count)   // 若 过零换相中断 50微秒内未触发 则记录
            {
                motor_right.overtime_change_phase_count ++;                     // 换相超时 计数值自增
            }
            else                                                                // 若 50微秒内触发 过零换相中断 则清除记录
            {
                motor_right.overtime_change_phase_count = 0;                    // 换相超时 计数值清除
            }
            
            if((motor_right.filter_commutation_time_sum > 5000  && motor_right.pit_trig_count > 5000) ||                                // 闭环运行 0.5s 后 若电机转速低于 285RPM 则认为 转速过低 关闭输出 
#if     MOTOR_DISTURB_PROTECT == DRIVER_ENABLE                 
               (motor_right.speed_change < -MOTOR_DISTURB_SPEED && func_abs(motor_right.duty_change) < 100 && motor_right.pit_trig_count > 5000) ||       // 闭环运行 0.5s 后 若电机转速突然下降(默认 500RPM) 则认为 受到异常干扰 关闭输出 
#endif                 
               (motor_right.overtime_change_phase_count > 1000)         ||                                                              // 若 50ms 未触发过零换相 则认为 电机堵转 关闭输出                
                
               (func_abs(motor_right.sensorless_duty) < 0.05)           ||                                                              // 若 占空比低于 5%  则认为 输出过低 关闭输出
                 
               (motor_right.motor_protect_state  == PROTECT_MODE)       ||                                                              // 若电机进入保护状态 则关闭输出                 
                 
               (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR) ||                                     // 若电池电压进入保护状态 且使能有效 则关闭输出   
                   
               ((motor_right.sensorless_duty * 10000.0f) * motor_right.motor_duty < 0.0f))                                              // 若占空比方向改变 则关闭输出    
            {
                motor_right.sensorless_state = SENSORLESS_RESTART;              // 驱动状态 修正为 重启状态
                    
                Cy_Tcpwm_Counter_Disable(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);   // 关闭 延时换相 定时器
                
                sensorless_motor_trig_disable(&motor_right);                    // 关闭 过零信号触发中断
                
                motor_commutation(&motor_right, 0, 0);                          // 关闭 输出
                
                motor_right.pit_trig_count = 0;                                 // 清除当前周期定时器计数值
                
                motor_right.speed_change = 0;                                   // 清除电机转速变化记录值
                
                motor_right.duty_change = 0;                                    // 清除电机占空比变化记录值
                
                motor_right.motor_speed = 0;                                    // 清除电机转速数据
                
                motor_right.motor_speed_filter = 0;                             // 清除电机转速滤波数据
            }
            
            motor_right.isr_trig_count_save = motor_right.isr_trig_count;       // 保存当前 过零信号触发计数值
        }break;
        
        case SENSORLESS_RESTART:                                                // 电机重启状态
        {            
            motor_commutation(&motor_right, 0, 0);                              // 按照当前相位输出  0占空比  关闭电机
            
            if(func_abs(motor_right.motor_duty) > 0.06 && motor_right.pit_trig_count > SENSERLESS_RESTART_TIME * 20)// 如果占空比有效(大于6%) 并且已经等待了指定时长  尝试重新启动电机
            {
                motor_right.pit_trig_count = 0;                                 // 清除当前周期定时器计数值
                
                motor_right.sensorless_state = SENSORLESS_OPEN_LOOP_SINE;       // 驱动状态 修正为 开环正弦启动状态
            }
            if(motor_right.motor_duty == 0                              ||      // 如果占空比无效  则停止驱动
               
              (motor_right.motor_protect_state  == PROTECT_MODE)        ||      // 若电机进入保护状态 则停止驱动   
                
              (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))    // 若电池电压进入保护状态 且使能有效 则停止驱动                                                   
            {
                motor_right.sensorless_state = SENSORLESS_STOP_STATE;           // 驱动状态 修正为 停止状态
            }
        }break;
        
        case SENSORLESS_STOP_STATE:                                             // 电机停止状态
        {           
            motor_commutation(&motor_right, 0, 0);                              // 按照当前相位输出  0占空比  关闭电机
            
            if(func_abs(motor_right.motor_duty) > 0.06 && (motor_right.motor_protect_state  != PROTECT_MODE))  // 如果占空比有效(大于6%) 电机未进入保护状态 尝试重新启动电机
            {   
                motor_right.pit_trig_count = 0;                                 // 清除当前周期定时器计数值
                
                motor_right.sensorless_state = SENSORLESS_OPEN_LOOP_SINE;       // 驱动状态 修正为 开环正弦启动状态
            }
        }break;
        
        default:break;
    }
    /*************************右侧无感驱动模式 周期循环执行内容*************************/ 
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感模式 启动时三相检测
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_start_check();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void sensorless_start_check(void)
{
#if SENSERLESS_WORK_MUSIC == DRIVER_ENABLE     
    
    motor_voice_output();                                                       // 电机输出音频信号
    
    motor_left_phase_check(300);                                                // 左侧三相 MOS 及 预驱 功能检测 由于没有检测三相电流 因此需要人为判断是否响三声
        
    motor_right_phase_check(300);                                               // 右侧三相 MOS 及 预驱 功能检测 由于没有检测三相电流 因此需要人为判断是否响三声
#endif

}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     无感 触发初始化
// 参数说明     void
// 返回参数     void
// 使用示例     sensorless_trig_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void sensorless_trig_init(void)
{
    cy_stc_gpio_pin_config_t            exti_pin_cfg    = {0};                                  // 定义无感过零信号触发引脚对象
    
    cy_stc_sysint_irq_t                 exti_irq_cfg;                                           // 定义无感过零信号触发中断对象
    
    cy_stc_tcpwm_counter_config_t       sensorless_delay_config;                                // 定义无感延时换相定时器对象
        
    cy_stc_sysint_irq_t                 sensorless_delay_irq_cfg;                               // 定义无感延时换相中断对象
        
        
    /*************************配置 左侧无感 过零信号信号触发*************************/           
    exti_pin_cfg.driveMode              = CY_GPIO_DM_HIGHZ;                                     // IO配置为高阻态
        
    exti_pin_cfg.intEdge                = EXTI_TRIGGER_BOTH;                                    // 触发方式为双边触发
        
    exti_pin_cfg.intMask                = 1ul;                                                  // 触发使能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_SENSORLESS_A_TRIG), (MOTOR_LEFT_SENSORLESS_A_TRIG % 8), &exti_pin_cfg);        // 配置 A相 过零信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_SENSORLESS_B_TRIG), (MOTOR_LEFT_SENSORLESS_B_TRIG % 8), &exti_pin_cfg);        // 配置 B相 过零信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_LEFT_SENSORLESS_C_TRIG), (MOTOR_LEFT_SENSORLESS_C_TRIG % 8), &exti_pin_cfg);        // 配置 C相 过零信号 IO功能
    
    exti_irq_cfg.intIdx     = CPUIntIdx5_IRQn;                                                  // 选择中断号
        
    exti_irq_cfg.isEnabled  = true;                                                             // 中断使能
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_SENSORLESS_A_TRIG / 8);           // 选择 A相 过零信号 中断外设
    
    interrupt_init(&exti_irq_cfg, sensorless_motor_left_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
  
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_SENSORLESS_B_TRIG / 8);           // 选择 B相 过零信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, sensorless_motor_left_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_LEFT_SENSORLESS_C_TRIG / 8);           // 选择 C相 过零信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, sensorless_motor_left_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
    
    exti_disable(MOTOR_LEFT_SENSORLESS_A_TRIG);                                                 // 暂时失能 A相 过零信号 触发中断
        
    exti_disable(MOTOR_LEFT_SENSORLESS_B_TRIG);                                                 // 暂时失能 A相 过零信号 触发中断
        
    exti_disable(MOTOR_LEFT_SENSORLESS_C_TRIG);                                                 // 暂时失能 A相 过零信号 触发中断
    /*************************配置 左侧无感 过零信号信号触发*************************/
    
    
    
    /*************************配置 左侧无感 延迟换相定时器时钟*************************/
    // 初始化延迟换相控制时钟  定时器通道0   使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_LEFT_SENSORLESS_DELAY_CLOCK, CY_SYSCLK_DIV_16_BIT, 0);  // 分配时钟信号 选择16位0通道输出到 定时器时钟
      
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9);                                     // 配置时钟分配系数 当前10分频（频率8Mhz  周期125ns）
      
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);                                     // 使能时钟
    /*************************配置 左侧无感 延迟换相定时器时钟*************************/ 
      
      
    /*************************配置 左侧无感 延迟换相定时器*************************/   
    sensorless_delay_config.period               = 0xffff                        ;              // 定时器周期为65535
      
    sensorless_delay_config.compare0             = 0xffff                        ;              // 定时器比较值为65535
      
    sensorless_delay_config.compare1             = 0xffff                        ;              // 定时器比较值为65535
      
    sensorless_delay_config.clockPrescaler       = CY_TCPWM_PRESCALER_DIVBY_8    ;              // 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
      
    sensorless_delay_config.runMode              = CY_TCPWM_COUNTER_ONESHOT      ;              // 仅工作一次 溢出则停止运行
      
    sensorless_delay_config.countDirection       = CY_TCPWM_COUNTER_COUNT_UP     ;              // 向上计数
      
    sensorless_delay_config.compareOrCapture     = CY_TCPWM_COUNTER_MODE_COMPARE ;              // 比较模式
      
    sensorless_delay_config.countInputMode       = CY_TCPWM_INPUT_LEVEL          ;              // 计数事件为任意
      
    sensorless_delay_config.countInput           = 1uL                           ;              // 计数事件输入高电平
      
    sensorless_delay_config.trigger0EventCfg     = CY_TCPWM_COUNTER_OVERFLOW     ;              // 中断触发事件0为溢出时触发中断
      
    sensorless_delay_config.trigger1EventCfg     = CY_TCPWM_COUNTER_OVERFLOW     ;              // 中断触发事件1为溢出时触发中断
              
    Cy_Tcpwm_Counter_Init(MOTOR_LEFT_SENSORLESS_DELAY_TIMER, &sensorless_delay_config);         // 初始化定时器参数
      
    Cy_Tcpwm_Counter_Enable(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);                                 // 使能定时器
      
    Cy_Tcpwm_Counter_SetCounter(MOTOR_LEFT_SENSORLESS_DELAY_TIMER, 0);                          // 清除定时器计数值
      
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_LEFT_SENSORLESS_DELAY_TIMER);                         // 使能定时器中断
    /*************************配置 左侧无感 延迟换相定时器*************************/   
      
      
    /***********************配置 左侧无感 延迟换相定时器中断***********************/     
    sensorless_delay_irq_cfg.sysIntSrc          = MOTOR_LEFT_SENSORLESS_DELAY_ISR;              // 选择中断触发源为定时器0的通道20
      
    sensorless_delay_irq_cfg.intIdx             = CPUIntIdx6_IRQn                ;              // 选择中断触发通道为用户中断6
      
    sensorless_delay_irq_cfg.isEnabled          = true                           ;              // 允许中断触发
      
    interrupt_init(&sensorless_delay_irq_cfg, motor_left_sensorless_delay_isr, 0);              // 中断初始化 优先级 0  （最高优先级）
    /***********************配置 左侧无感 延迟换相定时器中断***********************/     
    
    
    
    /*************************配置 右侧无感 过零信号信号触发*************************/           
    exti_pin_cfg.driveMode              = CY_GPIO_DM_HIGHZ;                                     // IO配置为高阻态
        
    exti_pin_cfg.intEdge                = EXTI_TRIGGER_BOTH;                                    // 触发方式为双边触发
        
    exti_pin_cfg.intMask                = 1ul;                                                  // 触发使能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_SENSORLESS_A_TRIG), (MOTOR_RIGHT_SENSORLESS_A_TRIG % 8), &exti_pin_cfg);        // 配置 A相 过零信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_SENSORLESS_B_TRIG), (MOTOR_RIGHT_SENSORLESS_B_TRIG % 8), &exti_pin_cfg);        // 配置 B相 过零信号 IO功能
    
    Cy_GPIO_Pin_Init(get_port(MOTOR_RIGHT_SENSORLESS_C_TRIG), (MOTOR_RIGHT_SENSORLESS_C_TRIG % 8), &exti_pin_cfg);        // 配置 C相 过零信号 IO功能
    
    exti_irq_cfg.intIdx     = CPUIntIdx5_IRQn;                                                  // 选择中断号
        
    exti_irq_cfg.isEnabled  = true;                                                             // 中断使能
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_SENSORLESS_A_TRIG / 8);           // 选择 A相 过零信号 中断外设
    
    interrupt_init(&exti_irq_cfg, sensorless_motor_right_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
  
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_SENSORLESS_B_TRIG / 8);           // 选择 B相 过零信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, sensorless_motor_right_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + MOTOR_RIGHT_SENSORLESS_C_TRIG / 8);           // 选择 C相 过零信号 中断外设
                                                                                                                        
    interrupt_init(&exti_irq_cfg, sensorless_motor_right_trig_isr, 0);                                                   // 中断初始化 优先级 0  （最高优先级）
    
    exti_disable(MOTOR_RIGHT_SENSORLESS_A_TRIG);                                                // 暂时失能 A相 过零信号 触发中断
        
    exti_disable(MOTOR_RIGHT_SENSORLESS_B_TRIG);                                                // 暂时失能 A相 过零信号 触发中断
        
    exti_disable(MOTOR_RIGHT_SENSORLESS_C_TRIG);                                                // 暂时失能 A相 过零信号 触发中断
    /*************************配置 右侧无感 过零信号信号触发*************************/
    
    
    
    /*************************配置 右侧无感 延迟换相定时器时钟*************************/
    // 初始化延迟换相控制时钟  定时器通道0   使用16位分频器的通道0 10分频 输出8Mhz时钟到定时器 
    Cy_SysClk_PeriphAssignDivider(MOTOR_RIGHT_SENSORLESS_DELAY_CLOCK, CY_SYSCLK_DIV_16_BIT, 0); // 分配时钟信号 选择16位0通道输出到 定时器时钟
      
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0, 9);                                     // 配置时钟分配系数 当前10分频（频率8Mhz  周期125ns）
      
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0);                                     // 使能时钟
    /*************************配置 右侧无感 延迟换相定时器时钟*************************/ 
      
      
    /*************************配置 右侧无感 延迟换相定时器*************************/   
    sensorless_delay_config.period               = 0xffff                        ;              // 定时器周期为65535
      
    sensorless_delay_config.compare0             = 0xffff                        ;              // 定时器比较值为65535
      
    sensorless_delay_config.compare1             = 0xffff                        ;              // 定时器比较值为65535
      
    sensorless_delay_config.clockPrescaler       = CY_TCPWM_PRESCALER_DIVBY_8    ;              // 时钟8分频 8Mhz输入时钟被8分频为1Mhz 因此定时器计数单位为微秒
      
    sensorless_delay_config.runMode              = CY_TCPWM_COUNTER_ONESHOT      ;              // 仅工作一次 溢出则停止运行
      
    sensorless_delay_config.countDirection       = CY_TCPWM_COUNTER_COUNT_UP     ;              // 向上计数
      
    sensorless_delay_config.compareOrCapture     = CY_TCPWM_COUNTER_MODE_COMPARE ;              // 比较模式
      
    sensorless_delay_config.countInputMode       = CY_TCPWM_INPUT_LEVEL          ;              // 计数事件为任意
      
    sensorless_delay_config.countInput           = 1uL                           ;              // 计数事件输入高电平
      
    sensorless_delay_config.trigger0EventCfg     = CY_TCPWM_COUNTER_OVERFLOW     ;              // 中断触发事件0为溢出时触发中断
      
    sensorless_delay_config.trigger1EventCfg     = CY_TCPWM_COUNTER_OVERFLOW     ;              // 中断触发事件1为溢出时触发中断
              
    Cy_Tcpwm_Counter_Init(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER, &sensorless_delay_config);        // 初始化定时器参数
      
    Cy_Tcpwm_Counter_Enable(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);                                // 使能定时器
      
    Cy_Tcpwm_Counter_SetCounter(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER, 0);                         // 清除定时器计数值
      
    Cy_Tcpwm_Counter_SetTC_IntrMask(MOTOR_RIGHT_SENSORLESS_DELAY_TIMER);                        // 使能定时器中断
    /*************************配置 右侧无感 延迟换相定时器*************************/   
      
      
    /***********************配置 右侧无感 延迟换相定时器中断***********************/     
    sensorless_delay_irq_cfg.sysIntSrc          = MOTOR_RIGHT_SENSORLESS_DELAY_ISR;             // 选择中断触发源为定时器0的通道20
      
    sensorless_delay_irq_cfg.intIdx             = CPUIntIdx6_IRQn                ;              // 选择中断触发通道为用户中断6
      
    sensorless_delay_irq_cfg.isEnabled          = true                           ;              // 允许中断触发
      
    interrupt_init(&sensorless_delay_irq_cfg, motor_right_sensorless_delay_isr, 0);             // 中断初始化 优先级 0  （最高优先级）
    /***********************配置 右侧无感 延迟换相定时器中断***********************/     
          
    timer_init(LEFT_MOTOR_COUNT_TIMER, TIMER_US);                                               // 初始化 左侧电机计数定时器
    
    timer_init(RIGHT_MOTOR_COUNT_TIMER, TIMER_US);                                              // 初始化 右侧电机计数定时器
    
    pit_us_init(LEFT_MOTOR_PIT_TIMER, 50);                                                      // 初始化 左侧电机 轮询周期中断 
}









