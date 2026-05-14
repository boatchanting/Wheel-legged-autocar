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

#ifndef _motor_control_h_
#define _motor_control_h_


#include "zf_common_headfile.h"


#define ENCODER_PRECISION               (32767)                 // 磁编码器采样最大值  32767
#define OUTPUT_DUTY_MAX                 (PWM_PRIOD_LOAD)        // 占空比输出最大值

typedef enum
{
    LEFT_MOTOR  = 0,                            // 左侧电机
    RIGHT_MOTOR = 1,                            // 右侧电机
}motor_type_enum;

typedef enum
{
    REVERSE = -1,                               // 反转
    FORWARD = 1,                                // 正转
}motor_dir_enum;

typedef enum
{
    NORMAL_RUN_MODE,                            // 电机正常运行模式
    PROTECT_MODE,                               // 电机保护模式(磁编采值错误、电池电压过低)
}motor_protect_enum;

typedef enum
{
    NO_PROTECT,                                 // 未触发保护
    LOCKED_PROTECT,                             // 堵转保护
    OVERLOAD_PROTECT,                           // 过载保护
}motor_protect_cause_enum;


typedef enum
{
    MOTOR_STOP,                                 // 电机未运行
    MOTOR_RUN,                                  // 电机正在运行
}motor_run_state_enum;

typedef enum
{
    ENCODER_NORMAL,                             // 磁编码器工作正常
    ENCODER_ERROR,                              // 磁编码器采值错误
}magnetic_encoder_state_enum;

typedef enum
{
    FAST_FOC = 0,                               // 快速FOC驱动模式            配合磁编码器使用
    HALL_SIX_STEP,                              // 霍尔六步换相驱动模式       配合霍尔传感器使用  
    SENSORLESS,                                 // 无感电调模式               配合过零检测板使用
}motor_driver_mode_enum;

typedef enum
{
    SENSORLESS_IDLE = 0,                        // 无感模式 空闲状态
    SENSORLESS_OPEN_LOOP_SINE,                  // 无感模式 正弦开环驱动状态
    SENSORLESS_OPEN_LOOP_BLDC,                  // 无感模式 六步换相开环驱动状态
    SENSORLESS_CLOSE_LOOP ,                     // 无感模式 闭环状态
    SENSORLESS_STOP_STATE,                      // 无感模式 停止状态
    SENSORLESS_RESTART,                         // 无感模式 重启状态
}sensorless_state_enum;

typedef struct
{
    uint8   protect_flag;                       // 堵转保护使能标志位        
    
    float  protect_duty_max;                   // 堵转保护占空比  0~1
    
    uint16  protect_check_time;                 // 堵转检测时长
}motor_locked_struct;


typedef struct
{         
//====================================================电机配置参数==================================================== 
    motor_type_enum                     motor_type;                 // 当前电机类型    
  
    motor_driver_mode_enum              driver_mode;                // 驱动工作模式

    motor_locked_struct                 locked_value;               // 电机堵转保护配置参数 
    
    int16                               zero_location;              // 电机零点位置
    
    int16                               rotation_direction;         // 磁编与相位输出方向
    
    int16                               pole_pairs;                 // 电机极对数
//====================================================电机配置参数====================================================   

    
//====================================================电机实时参数====================================================         
    motor_run_state_enum                run_state;                  // 电机运行状态
    
    float                               motor_duty;                 // 当前电机占空比
    float                               save_duty;                  // 保留占空比
 
    motor_dir_enum                      motor_set_dir;              // 当前电机设置方向        
    motor_dir_enum                      motor_now_dir;              // 当前电机旋转方向
    
    uint8                              forward_traction_angle;      //  快速FOC模式下，正转的牵引角度  范围 60 ~ 120 
    uint8                              reversal_traction_angle;     //  快速FOC模式下，反转的牵引角度  范围 60 ~ 120 
       
    uint8                              forward_preact_angle;        //  快速FOC模式下，正转的超前角度  范围 0 ~ 90 
    uint8                              reversal_preact_angle;       //  快速FOC模式下，反转的超前角度  范围 0 ~ 90 
    
    float                              motor_speed;                // 电机当前转速            转速单位：RPM   
    float                              motor_speed_filter;         // 电机当前转速(滤波)      转速单位：RPM   
    
    uint32                              overload_count;             // 过载保护计次
    float                              overload_conefficient;       // 过载系数
    
    int8                                hall_value_now;             // 当前霍尔值
    int8                                hall_value_last;            // 上次霍尔值
        
    int32                               menc15a_value_now;          // 当前磁编码器采集值              
    int32                               menc15a_value_offset;       // 当前磁编码器单次偏差值          
    int32                               menc15a_offset_integral;    // 当前磁编码器偏差值积分
        
    motor_protect_enum                  motor_protect_state;        // 当前电机保护模式  
    motor_protect_cause_enum            motor_protect_cause;        // 当前电机保护原因
    magnetic_encoder_state_enum         encoder_state;              // 当前磁编码器状态    
  
    uint32                              lose_control_protect_count; // 失控保护计数值    
    
    sensorless_state_enum               sensorless_state;           // 无感模式下的当前状态   
    float                               sensorless_duty;           // 无感模式下的单独占空比参数    
    int8                                sensorless_step;            // 无感模式换相计步
    int8                                sensorless_value_now;       // 当前过零值
    int8                                sensorless_value_last;      // 上次过零值
    
    uint32                              step_change_time_us;        // 单步耗时记录
    uint16                              commutation_time[6];        // 最近6次换相时间
    uint32                              commutation_time_sum;       // 最近6次换相时间总和
    uint32                              commutation_num;            // 统计换相次数
    uint32                              filter_commutation_time_sum;// 滤波后的换相时间
    
    uint32                              sine_output_count;          // 正弦牵引计数
    uint32                              common_change_phase_count;  // 正常换相计数    
    uint32                              overtime_change_phase_count;// 换相超时计数
    uint32                              isr_trig_count;             // 信号触发计数  
    uint32                              isr_trig_count_save;        // 保存 信号触发计数值
    uint32                              pit_trig_count;             // 周期中断计数    
    
    int32                               speed_record[100];          // 转速记录数组
    int32                               speed_record_count;         // 转速记录计次    
    int32                               speed_change;               // 转速变化值
    int32                               duty_record;                // 占空比记录值   
    int32                               duty_record_count;          // 占空比记录计次
    int32                               duty_change;                // 占空比变化值    
//====================================================电机实时参数====================================================    
    
}motor_struct;

extern motor_struct motor_left;                 // 左侧电机参数结构体
extern motor_struct motor_right;                // 右侧电机参数结构体

void motor_left_update_isr(void);               // 左侧电机更新中断 
void motor_right_update_isr(void);              // 右侧电机更新中断 

uint8 motor_both_zero_calibration(void);         // 双电机零位矫正
void  motor_zero_calibration(void);              // 电机三相自检+零位矫正

void driver_cmd_forthwith(void);                // 驱动指令即时执行 
void driver_cmd_loop(void);                     // 驱动指令响应循环

void motor_control_init(void);                  // 双电机控制初始化



#endif