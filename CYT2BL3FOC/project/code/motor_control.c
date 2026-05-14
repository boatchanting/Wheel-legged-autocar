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
* 文件名称          motor_control
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

#include "motor_control.h"

motor_struct motor_left;

motor_struct motor_right;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     左电机更新中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_left_updat_isr();
// 备注信息     当前执行耗时 8us  
//-------------------------------------------------------------------------------------------------------------------
void motor_left_update_isr(void)
{
    static uint8  left_speed_count = 0;                                         // 速度拟合计次

    static uint32 left_protect_count = 0;                                       // 输出保护计次
    
#if MOTOR_LOSE_CONTROL_PROTECT  == DRIVER_ENABLE && USER_CONTROL_MODE == 0   
    
    if(++ motor_left.lose_control_protect_count > (MOTOR_LOSE_CONTROL_TIME * 20))
    {
        motor_left.motor_duty       = 0.0f;
    }
    
#endif
    
#if MOTOR_OVERLOAD_PROTECT      == DRIVER_ENABLE 
    
    if(func_abs(motor_left.motor_duty) > MOTOR_OVERLOAD_DUTY_MIN && motor_left.driver_mode != SENSORLESS)
    {
        float speed_ratio = func_limit_ab((func_abs(motor_left.motor_speed_filter) / (FOC_MOTOR_KV_NUM * battery_value.battery_voltage)), 0.0f, 1.0f);
        
        motor_left.overload_conefficient = (func_abs(motor_left.motor_duty) - speed_ratio) / (func_abs(motor_left.motor_duty) + speed_ratio);

        if(motor_left.overload_conefficient >= MOTOR_OVERLOAD_COEFFICIENT)          // 判断是否过载 
        {
            motor_left.overload_count ++;
            
            if(MOTOR_DRIVER_MODE == HALL_SIX_STEP_MODE)
            {
                if(motor_left.overload_count > (MOTOR_OVERLOAD_TIME))
                {
                    motor_left.overload_count = (MOTOR_OVERLOAD_TIME);
                    
                    motor_left.motor_protect_state = PROTECT_MODE;              // 条件成立  进入输出保护状态
                    motor_left.motor_protect_cause = OVERLOAD_PROTECT;
                }
            }
            else
            {
                if(motor_left.overload_count > (MOTOR_OVERLOAD_TIME * 20))
                {
                    motor_left.overload_count = (MOTOR_OVERLOAD_TIME * 20);
                    
                    motor_left.motor_protect_state = PROTECT_MODE;              // 条件成立  进入输出保护状态
                    motor_left.motor_protect_cause = OVERLOAD_PROTECT;
                }
            }
        }
        else
        {
            motor_left.overload_count = 0;                                     // 否则清空保护计次
        }
    }
    else
    {
        motor_left.overload_conefficient = 0;
    }
    
#endif    
        
    if(motor_left.driver_mode == FAST_FOC)
    {
        Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_LEFT_A_PHASE_TCPWM_TIMER);              // 清除中断标志位  

        motor_left.menc15a_value_now = menc15a_get_absolute_data(menc15a_1_module); // 采集左侧电机磁编码器数值
        
        motor_left.menc15a_value_offset = menc15a_absolute_offset_data[0];          // 获取磁编码器较上一次的偏移值
        
        motor_left.menc15a_offset_integral += motor_left.menc15a_value_offset;      // 偏移值积分 用于速度计算
        
        if(++ left_speed_count >= (FPWM / 1000))                                    // 每毫秒拟合一次速度
        {
            left_speed_count = 0;
            
            motor_left.motor_speed = (motor_left.menc15a_offset_integral * 1.8310546875f);    // 速度数据拟合
            
            motor_left.motor_speed_filter = ((motor_left.motor_speed_filter * 19.0f + motor_left.motor_speed) / 20.0f);  // 速度数据低通滤波
            
            motor_left.menc15a_offset_integral = 0;                                 // 偏移积分归零
               
            if(motor_left.locked_value.protect_flag)
            {
                if(func_abs(motor_left.motor_speed_filter) < 10.0f && func_abs(motor_left.motor_duty) >= motor_left.locked_value.protect_duty_max)  // 判断是否堵转  占空比大于10%并且无转速数据持续 500ms
                {
                    left_protect_count ++;
                    
                    if(left_protect_count > motor_left.locked_value.protect_check_time)
                    {
                        left_protect_count = motor_left.locked_value.protect_check_time;
                        
                        motor_left.motor_protect_state = PROTECT_MODE;          // 条件成立  进入输出保护状态
                        motor_left.motor_protect_cause = LOCKED_PROTECT;
                    }
                }
                else
                {
                    left_protect_count = 0;                                     // 否则清空保护计次
                }
            }
        }
        
        float speed_offset = func_limit_ab((func_abs(motor_left.motor_speed_filter) / (FOC_MOTOR_KV_NUM * 12.0f)), 0.0f, 1.0f);
        
        motor_left.forward_preact_angle  = (uint8)(speed_offset * FOC_FORWARD_PREACT_ANGLE);
        
        motor_left.reversal_preact_angle = (uint8)(speed_offset * FOC_REVERSAL_PREACT_ANGLE);
          
        fast_foc_calculate(&motor_left_foc_driver,                                  // FAST-FOC 计算 三相输出值        
                           motor_left.menc15a_value_now, 
                           motor_left.motor_duty > 0 ? motor_left.motor_duty : -motor_left.motor_duty, 
                           motor_left.motor_duty > 0 ? (motor_left.forward_traction_angle + motor_left.forward_preact_angle): -(motor_left.reversal_traction_angle + motor_left.reversal_preact_angle));  
       
        if(motor_left.motor_protect_state       == PROTECT_MODE   || 
           motor_left.encoder_state             == ENCODER_ERROR  ||
           (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))
        {
            motor_left_duty_set(0, 0, 0);                                           // 输出保护状态 或者 磁编码器错误 则输出0占空比 刹车
        }
        else
        {
            motor_left_duty_set(motor_left_foc_driver.ouput_duty[0], motor_left_foc_driver.ouput_duty[1], motor_left_foc_driver.ouput_duty[2]); // 正常输出三相占空比
        }
    }
    else if(motor_left.driver_mode == HALL_SIX_STEP)
    {
        Cy_Tcpwm_Counter_ClearTC_Intr((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[(LEFT_MOTOR_PIT_TIMER - 3)]);              // 清除中断标志位  
        
        motor_left_hall_callback();                                                 // 左侧电机霍尔采集回调
    }
    else
    {
        Cy_Tcpwm_Counter_ClearTC_Intr((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[(LEFT_MOTOR_PIT_TIMER - 3)]);              // 清除中断标志位  
        
#if MOTOR_LOSE_CONTROL_PROTECT  == DRIVER_ENABLE && USER_CONTROL_MODE == 0     
    
    if(++ motor_right.lose_control_protect_count > (MOTOR_LOSE_CONTROL_TIME * 20))
    {
        motor_right.motor_duty       = 0.0f;
    }
    
#endif
        sensorless_motor_isr();
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     右电机更新中断
// 参数说明     void
// 返回参数     void
// 使用示例     motor_right_updat_isr();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_right_update_isr(void)
{       

    static uint8  right_speed_count = 0;                                        // 速度拟合计次

    static uint32 right_protect_count = 0;                                      // 输出保护计次

#if MOTOR_LOSE_CONTROL_PROTECT  == DRIVER_ENABLE && USER_CONTROL_MODE == 0     
    
    if(++ motor_right.lose_control_protect_count > (MOTOR_LOSE_CONTROL_TIME * 20))
    {
        motor_right.motor_duty       = 0.0f;
    }
    
#endif
    
#if MOTOR_OVERLOAD_PROTECT      == DRIVER_ENABLE 
    
    if(func_abs(motor_right.motor_duty) > MOTOR_OVERLOAD_DUTY_MIN && motor_left.driver_mode != SENSORLESS)
    {
        float speed_ratio = func_limit_ab((func_abs(motor_right.motor_speed_filter) / (FOC_MOTOR_KV_NUM * battery_value.battery_voltage)), 0.0f, 1.0f);
    
        motor_right.overload_conefficient = (func_abs(motor_right.motor_duty) - speed_ratio) / (func_abs(motor_right.motor_duty) + speed_ratio);

        if(motor_right.overload_conefficient >= MOTOR_OVERLOAD_COEFFICIENT)          // 判断是否过载 
        {
            motor_right.overload_count ++;
            
            if(MOTOR_DRIVER_MODE == HALL_SIX_STEP_MODE)
            {
                if(motor_right.overload_count > (MOTOR_OVERLOAD_TIME))
                {
                    motor_right.overload_count = (MOTOR_OVERLOAD_TIME);
                    
                    motor_right.motor_protect_state = PROTECT_MODE;             // 条件成立  进入输出保护状态
                    motor_right.motor_protect_cause = OVERLOAD_PROTECT;
                }
            }
            else
            {
                if(motor_right.overload_count > (MOTOR_OVERLOAD_TIME * 20))
                {
                    motor_right.overload_count = (MOTOR_OVERLOAD_TIME * 20);
                    
                    motor_right.motor_protect_state = PROTECT_MODE;             // 条件成立  进入输出保护状态
                    motor_right.motor_protect_cause = OVERLOAD_PROTECT;
                }
            }
        }
        else
        {
            motor_right.overload_count = 0;                                     // 否则清空保护计次
        }
    }
    else
    {
        motor_right.overload_conefficient = 0;
    }
    
#endif   
    
    if(motor_right.driver_mode == FAST_FOC)
    {
        Cy_Tcpwm_Counter_ClearTC_Intr(MOTOR_RIGHT_A_PHASE_TCPWM_TIMER);             // 清除中断标志位  
                                                                                    
        motor_right.menc15a_value_now = menc15a_get_absolute_data(menc15a_2_module);// 采集左侧电机磁编码器数值
                                                                                    
        motor_right.menc15a_value_offset = menc15a_absolute_offset_data[1];         // 获取磁编码器较上一次的偏移值
                                                                                    
        motor_right.menc15a_offset_integral += motor_right.menc15a_value_offset;    // 偏移值积分 用于速度计算
                                                                                    
        if(++ right_speed_count >= 20)                                              // 每毫秒拟合一次速度 
        {
            right_speed_count = 0;
            
            motor_right.motor_speed = (motor_right.menc15a_offset_integral * 1.8310546875f);    // 速度数据拟合
            
            motor_right.motor_speed_filter = ((motor_right.motor_speed_filter * 19.0f + motor_right.motor_speed) / 20.0f);     // 速度数据低通滤波
            
            motor_right.menc15a_offset_integral = 0;                                // 偏移积分归零        

            if(motor_right.locked_value.protect_flag)
            {
                if(func_abs(motor_right.motor_speed_filter) < 10.0f && func_abs(motor_right.motor_duty) >= motor_right.locked_value.protect_duty_max)  // 判断是否堵转  占空比大于 10% 并且无转速数据持续 500ms
                {
                    right_protect_count ++;
                    
                    if(right_protect_count > motor_right.locked_value.protect_check_time)
                    {
                        right_protect_count = motor_right.locked_value.protect_check_time;
                        
                        motor_right.motor_protect_state = PROTECT_MODE;             // 条件成立  进入输出保护状态
                        motor_right.motor_protect_cause = LOCKED_PROTECT;
                    }
                }
                else
                {
                    right_protect_count = 0;                                        // 否则清空保护计次
                }
            }
        }
          
        float speed_offset = func_limit_ab((func_abs(motor_right.motor_speed_filter) / (FOC_MOTOR_KV_NUM * 12.0f)), 0.0f, 1.0f);
        
        motor_right.forward_preact_angle  = (uint8)(speed_offset * (float)FOC_FORWARD_PREACT_ANGLE);
        
        motor_right.reversal_preact_angle = (uint8)(speed_offset * (float)FOC_REVERSAL_PREACT_ANGLE);
        
        fast_foc_calculate(&motor_right_foc_driver,                                 // FAST-FOC 计算 三相输出值        
                           motor_right.menc15a_value_now, 
                           motor_right.motor_duty > 0 ? motor_right.motor_duty : -motor_right.motor_duty, 
                           motor_right.motor_duty > 0 ? (motor_right.forward_traction_angle + motor_right.forward_preact_angle): -(motor_right.reversal_traction_angle + motor_right.reversal_preact_angle));  
        
        if(motor_right.motor_protect_state      == PROTECT_MODE         || 
           motor_right.encoder_state            == ENCODER_ERROR        ||
           (battery_value.protect_flag == 1 && battery_value.battery_state == BATTERY_ERROR))
        {
            motor_right_duty_set(0, 0, 0);                                          // 保护状态输出0占空比 刹车
        }
        else
        {
            motor_right_duty_set(motor_right_foc_driver.ouput_duty[0], motor_right_foc_driver.ouput_duty[1], motor_right_foc_driver.ouput_duty[2]);     // 输出三相占空比
        }
    }
    else if(motor_right.driver_mode == HALL_SIX_STEP)
    {
        Cy_Tcpwm_Counter_ClearTC_Intr((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[(RIGHT_MOTOR_PIT_TIMER - 3)]);             // 清除中断标志位  
        
        motor_right_hall_callback();                                                // 右侧电机霍尔采集回调
    }
    else
    {
        Cy_Tcpwm_Counter_ClearTC_Intr((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[(RIGHT_MOTOR_PIT_TIMER - 3)]);             // 清除中断标志位  
        
        sensorless_motor_isr();
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机零点及旋转方向同时矫正
// 参数说明     void
// 返回参数     void
// 使用示例     motor_both_zero_calibration();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
uint8 motor_both_zero_calibration(void)
{        
    int16  rotation_direction[2] = {0}; 
    int32  encoder_data_integral[2] = {0};
    uint16 stop_encoder_data[2] = {0};    
    uint8  calibration_state = 2;
    
    
    // 矫正零点时默认恢复电机工作保护状态
    motor_left.motor_protect_state   = NORMAL_RUN_MODE;     
    motor_right.motor_protect_state  = NORMAL_RUN_MODE;     
    
    // 初始化默认配置参数
    fast_foc_init(&motor_left_foc_driver,  ENCODER_PRECISION, OUTPUT_DUTY_MAX, 1, 0, 1);
    fast_foc_init(&motor_right_foc_driver, ENCODER_PRECISION, OUTPUT_DUTY_MAX, 1, 0, 1);    
    
    // 开环定位 20% 占空比
    fast_foc_calculate(&motor_left_foc_driver,  0, 0.2, 0);
    fast_foc_calculate(&motor_right_foc_driver, 0, 0.2, 0);                            
    
    // 输出占空比到电机
    motor_left_duty_set (motor_left_foc_driver.ouput_duty[0],  motor_left_foc_driver.ouput_duty[1],  motor_left_foc_driver.ouput_duty[2] );
    motor_right_duty_set(motor_right_foc_driver.ouput_duty[0], motor_right_foc_driver.ouput_duty[1], motor_right_foc_driver.ouput_duty[2]); 
    
    // 延时 200ms 等待电机回到零点
    system_delay_ms(200);                                                                                       
    
    // 读取当前磁编数据
    menc15a_get_absolute_data((menc15a_module_enum)LEFT_MOTOR); 
    menc15a_get_absolute_data((menc15a_module_enum)RIGHT_MOTOR);  
    
    // 开环牵引 100 次 
    for(uint16_t i = 0; i <= 100; i ++)                                                                         
    {
        // 以 10% 占空比循环牵引
        fast_foc_calculate(&motor_left_foc_driver,  ENCODER_PRECISION * i / 100, 0.2, 0);
        fast_foc_calculate(&motor_right_foc_driver, ENCODER_PRECISION * i / 100, 0.2, 0);     
        
        // 输出占空比到电机
        motor_left_duty_set (motor_left_foc_driver.ouput_duty[0],  motor_left_foc_driver.ouput_duty[1],  motor_left_foc_driver.ouput_duty[2] );
        motor_right_duty_set(motor_right_foc_driver.ouput_duty[0], motor_right_foc_driver.ouput_duty[1], motor_right_foc_driver.ouput_duty[2]);
        
        // 单次牵引间隔为（默认5ms） 
        system_delay_ms(5);                                                                                     
        
        // 获取当前磁编码器数值
        menc15a_get_absolute_data((menc15a_module_enum)LEFT_MOTOR);
        menc15a_get_absolute_data((menc15a_module_enum)RIGHT_MOTOR);                                                            
        
        // 累计单次牵引的旋转方向
        if(menc15a_absolute_offset_data[LEFT_MOTOR] > 0)                                                                 
        {
            rotation_direction[LEFT_MOTOR] ++;
        }
        else if(menc15a_absolute_offset_data[LEFT_MOTOR] < 0)
        {
            rotation_direction[LEFT_MOTOR] --;
        }
        if(menc15a_absolute_offset_data[RIGHT_MOTOR] > 0)                                                                 
        {
            rotation_direction[RIGHT_MOTOR] ++;
        }
        else if(menc15a_absolute_offset_data[RIGHT_MOTOR] < 0)
        {
            rotation_direction[RIGHT_MOTOR] --;
        }
        
        // 累积单次牵引的旋转数值
        encoder_data_integral[LEFT_MOTOR] += menc15a_absolute_offset_data[LEFT_MOTOR];
        encoder_data_integral[RIGHT_MOTOR] += menc15a_absolute_offset_data[RIGHT_MOTOR];
    }                                                                                      
    
    // 获取牵引结束时的磁编码器数值
    stop_encoder_data[LEFT_MOTOR] = menc15a_absolute_data[LEFT_MOTOR];
    stop_encoder_data[RIGHT_MOTOR] = menc15a_absolute_data[RIGHT_MOTOR];                    
    
    // 关闭输出
    motor_left_duty_set (0, 0, 0);    
    motor_right_duty_set(0, 0, 0);    
    
    // 判断是否正确读取到磁编数据 开环牵引积分小于 1000 则认为电机没有旋转  折算下来为11°意味着超过 32 对极的电机无法检测
    if(func_abs(encoder_data_integral[LEFT_MOTOR]) < 1000)
    {
        // 更改左侧磁编码器状态标志
        motor_left.encoder_state = ENCODER_ERROR;
        
        // 修正默认参数
        motor_left.zero_location = 0;
        motor_left.rotation_direction = 1;
        motor_left.pole_pairs = 7;
        
        // 错误信息打印到 串口
        printf("left motor magnetic encoder error!!\r\n");   
    }
    else
    {
        // 更改左侧磁编码器状态标志
        motor_left.encoder_state = ENCODER_NORMAL;
        
        // 旋转方向归一化 正转则为 1  反转则为 -1
        motor_left.rotation_direction  = rotation_direction[LEFT_MOTOR]  / (rotation_direction[LEFT_MOTOR]  > 0 ? rotation_direction[LEFT_MOTOR]  : -rotation_direction[LEFT_MOTOR]) ;  

        // 反转则需要将牵引结束的编码器数值反相
        if(motor_left.rotation_direction == -1)                                                                     
        {
            stop_encoder_data[LEFT_MOTOR] = ENCODER_PRECISION - stop_encoder_data[LEFT_MOTOR];
        }
        
        // 计算极对数  磁编最大值 除以 单圈电角度积分值 四舍五入
        motor_left.pole_pairs  = (uint16)round((float)ENCODER_PRECISION / func_abs((float)encoder_data_integral[LEFT_MOTOR]));
 
        // 计算零点位置  结束位置 除以 电角度的磁编数值范围 取余
        motor_left.zero_location  = stop_encoder_data[LEFT_MOTOR] % (ENCODER_PRECISION / motor_left.pole_pairs );

        // 按照计算的数据重新初始化fast_foc参数
        fast_foc_init(&motor_left_foc_driver,  ENCODER_PRECISION, OUTPUT_DUTY_MAX, motor_left.pole_pairs,  motor_left.zero_location,  motor_left.rotation_direction );

        // 矫正信息打印到 串口
        printf("LEFT MOTOR ZERO:%d, DIR:%s, POLE_PAIRS：%d\r\n",  motor_left_foc_driver.motor_zero_location, 
                                                                  motor_left_foc_driver.motor_rotation_direction  == 1 ? "forward" : "reverse", 
                                                                  motor_left.pole_pairs);    
        
        calibration_state --;
    }
    
    // 判断是否正确读取到磁编数据 开环牵引积分小于 1000 则认为电机没有旋转 折算下来为11°意味着超过 32 对极的电机无法检测
    if(func_abs(encoder_data_integral[RIGHT_MOTOR]) < 1000)
    {
        // 更改右侧磁编码器状态标志
        motor_right.encoder_state = ENCODER_ERROR;
        
        // 修正默认参数
        motor_right.zero_location = 0;
        motor_right.rotation_direction = 1;
        motor_right.pole_pairs = 7;
        
        // 错误信息打印到 串口
        printf("right motor magnetic encoder error!!\r\n");   
    }
    else
    {
        // 更改右侧磁编码器状态标志
        motor_right.encoder_state = ENCODER_NORMAL;
        
        // 旋转方向归一化 正转则为 1  反转则为 -1
        motor_right.rotation_direction = rotation_direction[RIGHT_MOTOR] / (rotation_direction[RIGHT_MOTOR] > 0 ? rotation_direction[RIGHT_MOTOR] : -rotation_direction[RIGHT_MOTOR]);   
        
        // 反转则需要将牵引结束的编码器数值反相
        if(motor_right.rotation_direction == -1)                                                                     
        {
            stop_encoder_data[RIGHT_MOTOR] = ENCODER_PRECISION - stop_encoder_data[RIGHT_MOTOR];
        }
        
        // 计算极对数  磁编最大值 除以 单圈电角度积分值 四舍五入
        motor_right.pole_pairs = (uint16)round((float)ENCODER_PRECISION / func_abs((float)encoder_data_integral[RIGHT_MOTOR]));
        
        // 计算零点位置  结束位置 除以 电角度的磁编数值范围 取余
        motor_right.zero_location = stop_encoder_data[RIGHT_MOTOR] % (ENCODER_PRECISION / motor_right.pole_pairs);    
        
        // 按照计算的数据重新初始化fast_foc参数
        fast_foc_init(&motor_right_foc_driver, ENCODER_PRECISION, OUTPUT_DUTY_MAX, motor_right.pole_pairs, motor_right.zero_location, motor_right.rotation_direction);       
        
        // 矫正信息打印到 串口
        printf("RIGHT MOTOR ZERO:%d, DIR:%s, POLE_PAIRS：%d\r\n", motor_right_foc_driver.motor_zero_location, 
                                                                  motor_right_foc_driver.motor_rotation_direction == 1 ? "forward" : "reverse", 
                                                                  motor_right.pole_pairs);    
        
        calibration_state --;
    }
    
    return calibration_state;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机零点及旋转方向矫正
// 参数说明     void
// 返回参数     void
// 使用示例     motor_zero_calibration();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_zero_calibration(void)
{
    interrupt_global_disable();							// 关闭全局中断
  
    if((motor_left.motor_speed_filter != 0.0f || motor_right.motor_speed_filter != 0.0f))       // 如果电机正在旋转 则刹车
    {
        motor_left_duty_set(0, 0, 0);                                           // 左侧电机刹车
        
        motor_right_duty_set(0, 0, 0);                                          // 右侧电机刹车

        system_delay_ms(1000);                                                  // 刹车等待
    }
    
    motor_left_phase_check(200);                                                // 左侧三相 MOS 及 预驱 功能检测 由于没有检测三相电流 因此需要人为判断是否响三声
    
    motor_right_phase_check(200);                                               // 右侧三相 MOS 及 预驱 功能检测 由于没有检测三相电流 因此需要人为判断是否响三声
    
    if(motor_left.driver_mode == FAST_FOC)                                      // 只有在 FAST-FOC 模式下才需要矫正零点
    {       
        motor_left_output_init(PWM_PRIOD_LOAD, 1);                              // 左侧电机三相 PWM 输出初始化
      
        motor_right_output_init(PWM_PRIOD_LOAD, 1);                             // 右侧电机三相 PWM 输出初始化
      
        uint8 calibration_state = motor_both_zero_calibration();                // 双电机零点及旋转方向同时矫正
        
        if(calibration_state == 0)                                              // 判断矫正结果
        {
            printf("motor zero calibration is complete\r\n");                   // 输出反馈信息
        }
        else
        {
            printf("motor zero calibration is failed\r\n");                     // 输出反馈信息
        }
        
        if(calibration_state <= 1)                                              // 至少矫正成功一个电机 则将零点写入flash
        {
            motor_flash_write();                                                // 矫正后的数据写入flash
        }
        
    }
    else  if(motor_left.driver_mode == HALL_SIX_STEP)      
    {
        motor_left_output_init(PWM_PRIOD_LOAD,  0);                             // 左侧电机三相 PWM 输出初始化
    
        motor_right_output_init(PWM_PRIOD_LOAD, 0);                             // 右侧电机三相 PWM 输出初始化
      
        printf("only the FOC mode requires zero calibration\r\n");              // 输出反馈信息
    }    
    else      
    {
        printf("only the FOC mode requires zero calibration\r\n");              // 输出反馈信息
    }
    
    interrupt_global_enable(0);							// 开启全局中断
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机FOC控制初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_foc_control_init();
// 备注信息     该功能需配合磁编码器使用    
//-------------------------------------------------------------------------------------------------------------------
void motor_foc_control_init(void)
{
    motor_left.forward_traction_angle  = FOC_FORWARD_TRACTION_ANGLE;
    
    motor_left.reversal_traction_angle = FOC_REVERSAL_TRACTION_ANGLE;
    
    motor_right.forward_traction_angle  = FOC_FORWARD_TRACTION_ANGLE;
    
    motor_right.reversal_traction_angle = FOC_REVERSAL_TRACTION_ANGLE;
    
    menc15a_init();                                             // 磁编码器初始化
      
    fast_foc_init(&motor_left_foc_driver,  ENCODER_PRECISION, OUTPUT_DUTY_MAX, motor_left.pole_pairs,  motor_left.zero_location,  motor_left.rotation_direction );     // 左侧电机 FAST_FOC 功能初始化
    
    fast_foc_init(&motor_right_foc_driver, ENCODER_PRECISION, OUTPUT_DUTY_MAX, motor_right.pole_pairs, motor_right.zero_location, motor_right.rotation_direction);     // 右侧电机 FAST_FOC 功能初始化
    
    motor_left_output_init(PWM_PRIOD_LOAD, 1);                  // 左侧电机三相 PWM 输出初始化
      
    motor_right_output_init(PWM_PRIOD_LOAD, 1);                 // 右侧电机三相 PWM 输出初始化
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机BLDC控制初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_bldc_control_init();
// 备注信息     该功能需配合霍尔滤波转接板使用    
//-------------------------------------------------------------------------------------------------------------------
void motor_bldc_control_init(void)
{  
    hall_gather_init();                                         // 霍尔采集初始化
  
    motor_left_output_init(PWM_PRIOD_LOAD,  0);                 // 左侧电机三相 PWM 输出初始化
    
    motor_right_output_init(PWM_PRIOD_LOAD, 0);                 // 右侧电机三相 PWM 输出初始化
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机无感控制初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_sensorless_control_init();
// 备注信息     该功能需配合过零检测板使用  
//-------------------------------------------------------------------------------------------------------------------
void motor_sensorless_control_init(void)
{
    sensorless_start_check();
  
    sensorless_trig_init();
    
    motor_left.sensorless_state = SENSORLESS_STOP_STATE;
    
    motor_right.sensorless_state = SENSORLESS_STOP_STATE;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     双电机控制初始化
// 参数说明     void
// 返回参数     void
// 使用示例     motor_control_init();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void motor_control_init(void)
{  
    motor_left.motor_type                       = LEFT_MOTOR;                 
    
    motor_left.driver_mode                      = (motor_driver_mode_enum)MOTOR_DRIVER_MODE;
    
    motor_left.locked_value.protect_flag        = MOTOR_LOCKED_PROTECT;
    
    motor_left.locked_value.protect_duty_max    = MOTOR_LOCKED_DUTY_MAX;
    
    motor_left.locked_value.protect_check_time  = MOTOR_LOCKED_TIME;
    
    motor_right.motor_type                      = RIGHT_MOTOR;
  
    motor_right.driver_mode                     = (motor_driver_mode_enum)MOTOR_DRIVER_MODE;
    
    motor_right.locked_value.protect_flag       = MOTOR_LOCKED_PROTECT;
    
    motor_right.locked_value.protect_duty_max   = MOTOR_LOCKED_DUTY_MAX;
    
    motor_right.locked_value.protect_check_time = MOTOR_LOCKED_TIME;
    
#if  USER_CONTROL_MODE          == 0    
    motor_driver_uart_init();                           // 驱动通讯串口初始化
#else
    pwm_in_init();                                      // 脉宽捕获初始化
#endif
    
#if  DRIVER_ACTIVE_OUTPUT_SPEED == DRIVER_ENABLE
    motor_value.continue_command = GET_SPEED;
#endif
    
    if(motor_left.driver_mode == FAST_FOC) 
    {
        motor_foc_control_init();                       // FAST_FOC 模式初始化
    }
    else if(motor_left.driver_mode == HALL_SIX_STEP) 
    {
        motor_bldc_control_init();                      // 霍尔 BLDC 模式初始化
    }
    else if(motor_left.driver_mode == SENSORLESS)
    {
        motor_sensorless_control_init();                // 无感电调模式初始化    
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     驱动指令即时执行 
// 参数说明     void
// 返回参数     void
// 使用示例     driver_cmd_forthwith();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void driver_cmd_forthwith(void)
{
    if(motor_value.refresh_flag == CMD_FORTHWITH)     // 判断是否需要循环执行                          
    {
        switch(motor_value.immediate_command)   // 判断即时指令类型
        {
            case SET_DUTY:                      // 设置占空比指令
            {
                motor_left.motor_duty  = func_limit_ab((float)(motor_value.receive_left_duty_data)  / 10000.0f, -1.0f, 1.0f);   // 设置左侧电机占空比
    
                motor_right.motor_duty = func_limit_ab((float)(motor_value.receive_right_duty_data) / 10000.0f, -1.0f, 1.0f);   // 设置右侧电机占空比
                
#if             MOTOR_DRIVER_MODE      ==    SENSERLESS_MODE
                
                motor_left.motor_duty  = motor_left.motor_duty  * (SENSERLESS_MOTOR_LEFT_DIR == 0 ? 1 : -1); 
                
                motor_right.motor_duty = motor_right.motor_duty * (SENSERLESS_MOTOR_RIGHT_DIR == 0 ? 1 : -1); 
                
#endif
                motor_left.lose_control_protect_count = 0;
                
                motor_right.lose_control_protect_count = 0;
                
            }break;
            
            default:break;
        }
        
        motor_value.immediate_command = NULL_CMD;// 清除指令
      
        motor_value.refresh_flag = CMD_NULL;     // 刷新标志位清除
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     驱动指令响应循环
// 参数说明     void
// 返回参数     void
// 使用示例     driver_cmd_loop();
// 备注信息       
//-------------------------------------------------------------------------------------------------------------------
void driver_cmd_loop(void)
{
    if(motor_value.refresh_flag == CMD_LOOP)     // 判断是否循环指令                          
    {
        switch(motor_value.immediate_command)   // 判断即时指令类型
        {
            case NULL_CMD:break;                // 空指令直接退出
                       
            case SET_ZERO:                      // 设置零位
            {
                motor_zero_calibration();       // 自校正零位
    
            }break;
                       
            case TEST_PHASE:                    // 测试 BLDC 相位
            {                   
                 
                
            }break;

            case SET_PHASE:                     // 设置 BLDC 相位
            {                   
                 
                
            }break;
            
            case ERROR_CMD:                     // 错误指令
            {            
                 printf("error cmd\r\n");       // 输出反馈信息
    
            }break;
            
            default:break;
        }
        motor_value.immediate_command = NULL_CMD;// 清除指令
      
        motor_value.refresh_flag = CMD_NULL;     // 刷新标志位清除
    }

#if USER_CONTROL_MODE == 0    
    switch(motor_value.continue_command)        // 判断持续指令类型
    {
        case GET_SPEED:                         // 持续输出速度信息
        {
            if(motor_value.cmd_type == STRING_TYPE)             // 判断用户通讯类型
            {   
                printf("%d,%d\r\n", (int32)motor_left.motor_speed_filter, (int32)motor_right.motor_speed_filter);     // 字符串通讯则直接打印字符串
            }
            else
            {
                motor_driver_send_speed(&motor_value, (int32)motor_left.motor_speed_filter, (int32)motor_right.motor_speed_filter);   // 字节包通讯则按照协议打包输出
            }

        }break;
        
        case GET_PHASE_DUTY:                    // 持续输出相位信息
        {
            if(motor_value.select_motor == LEFT_MOTOR)          // 判断输出哪一侧的电机
            {
                printf("%d,%d,%d\r\n", motor_left_foc_driver.ouput_duty[0], motor_left_foc_driver.ouput_duty[1], motor_left_foc_driver.ouput_duty[2]);
            }
            if(motor_value.select_motor == RIGHT_MOTOR)
            {
                printf("%d,%d,%d\r\n", motor_right_foc_driver.ouput_duty[0], motor_right_foc_driver.ouput_duty[1], motor_right_foc_driver.ouput_duty[2]);
            }

        }break;
        
        case GET_ENCODER:                       // 持续输出编码器原始数值
        {
          
#if     MOTOR_DRIVER_MODE  == FAST_FOC_MODE
            printf("%d,%d\r\n", menc15a_absolute_data[0], menc15a_absolute_data[1]);
#elif   MOTOR_DRIVER_MODE  == HALL_SIX_STEP_MODE
            printf("%d,%d\r\n", motor_left.hall_value_now, motor_right.hall_value_now);
#endif

        }break;
        
        case GET_ANGLE:                         // 持续输出编码器数据转角度数据
        {
            printf("%.2f,%.2f\r\n", (float)menc15a_absolute_data[0] / ENCODER_PRECISION * 360.0f, (float)menc15a_absolute_data[1] / ENCODER_PRECISION * 360.0f);

        }break;
        
        case GET_VOLTAGE:                       // 持续输出电池电压数据
        {
            printf("voltage:%.2f\r\n", battery_value.battery_voltage);
  
        }break;
            
        default:break;
    }
#endif   
}




