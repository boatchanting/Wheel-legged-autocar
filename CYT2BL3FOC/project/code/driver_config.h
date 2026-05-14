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
* 文件名称          driver_config
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
#ifndef _driver_config_h_
#define _driver_config_h_


#include "zf_common_headfile.h"

#define  DRIVER_ENABLE                  (1)             // 使能

#define  DRIVER_DISABLE                 (0)             // 失能


/************************** 驱动模式配置 **************************/
#define  FAST_FOC_MODE                  (0)             // 快速FOC驱动模式            配套使用磁编码器

#define  HALL_SIX_STEP_MODE             (1)             // 霍尔六步换相驱动模式       配套使用霍尔信号转接板    

#define  SENSERLESS_MODE                (2)             // 无感驱动模式               配套使用过零检测转接板        

#define  MOTOR_DRIVER_MODE              FAST_FOC_MODE   // 驱动工作模式 默认为 快速FOC驱动模式

#define  USER_CONTROL_MODE              (0)             // 0 串口通讯控制    1 PWM控制(1~2ms高电平时间，限制50~300hz，无感电调模式专用)
/************************** 驱动模式配置 **************************/


/************************** 驱动参数配置 **************************/
#define  DRIVER_ACTIVE_OUTPUT_SPEED     DRIVER_DISABLE  // 串口通讯控制模式下  DRIVER_ENABLE: 上电就主动输出转速数据    DRIVER_DISABLE: 收到获取转速指令再输出转速数据

#define  DRIVER_RESPONSE_CYCLE          (10)            // 驱动响应周期    用于主循环延时  单位ms  建议范围 1 ~ 20  默认10ms周期  包含普通指令响应、信息打印、ADC读取、GPIO操作


#define  FOC_MOTOR_KV_NUM               (800.0f)        // 驱动的电机KV值  用于超前角的计算  大幅度减小 高转速时的发热情况

#define  FOC_FORWARD_TRACTION_ANGLE     (90.0f)         // 快速FOC模式下   正转的牵引角度，默认为90°

#define  FOC_REVERSAL_TRACTION_ANGLE    (90.0f)         // 快速FOC模式下   反转的牵引角度，默认为90°

#define  FOC_FORWARD_PREACT_ANGLE       (70.0f)         // 快速FOC模式下   由于高转速时的牵引滞后导致异常发热电流  通过超前角的方式防止该情况 大幅度减小 高转速时的发热情况 默认70° （要求电机KV值准确）

#define  FOC_REVERSAL_PREACT_ANGLE      (70.0f)         // 快速FOC模式下   由于高转速时的牵引滞后导致异常发热电流  通过超前角的方式防止该情况 大幅度减小 高转速时的发热情况 默认70° （要求电机KV值准确）
 
#define  BLDC_DEBUG_OUTPUT_PHASE        (0)             // BLDC模式下      固定只输出某个相位  0:不启用   1~6:输出指定相位  
                                                        // 仅用于调试      可测试单步输出是否正常，已限制输出占空比(最大10%)

#define  BLDC_GLIDING_MODE              DRIVER_DISABLE  // BLDC模式下      滑行模式  DRIVER_ENABLE: 给0占空比时，电机滑行    DRIVER_DISABLE: 给0占空比时，电机刹车

#define  SENSERLESS_WORK_MUSIC          DRIVER_ENABLE   // 无感电调模式下  DRIVER_ENABLE: 开启上电音乐(可用于判断三相控制是否正常)    DRIVER_DISABLE: 关闭上电音乐

#define  SENSERLESS_MOTOR_LEFT_DIR      (0)             // 无感电调模式下  左侧电机的默认旋转方向(修改此处无需重新焊接电机线)   0 正转    1 反转

#define  SENSERLESS_MOTOR_RIGHT_DIR     (0)             // 无感电调模式下  右侧电机的默认旋转方向(修改此处无需重新焊接电机线)   0 正转    1 反转

#define  SENSERLESS_OPENLOOP_SINE_DUTY  (1200)          // 无感电调模式下  开环正弦牵引的占空比  范围500 ~ 1500  对应占空比 5% ~ 15%
                     
#define  SENSERLESS_OPENLOOP_SINE_TIME  (30)            // 无感电调模式下  开环正弦牵引的单圈(电角度)耗时 默认为30ms

#define  SENSERLESS_OPENLOOP_SINE_TURNS (3)             // 无感电调模式下  开环正弦牵引的圈数(电角度)  若电机极对数为7  那么电机转一圈 等于 电角度7圈
                                                                           
#define  SENSERLESS_OPENLOOP_BLDC_DUTY  (1000)          // 无感电调模式下  开环BLDC牵引的占空比  范围500 ~ 1500  对应占空比 5% ~ 15%
                                                                           
#define  SENSERLESS_OPENLOOP_BLDC_TURNS (50)            // 无感电调模式下  开环BLDC牵引的圈数(电角度)
                                                                           
#define  SENSERLESS_CLOSELOOP_CHECK     (8)             // 无感电调模式下  若开环BLDC成功牵引该值电角度(默认4圈)，则进入闭环模式
                                                                           
#define  SENSERLESS_SPEED_INCREMENTAL   (1)             // 无感电调模式下  加减速响应速度  1为最快  越高越慢
                                                                           
#define  SENSERLESS_RESTART_TIME        (1000)          // 无感电调模式下  重启的间隔时长，默认1000ms
/************************** 驱动参数配置 **************************/



/**************************** 电池保护 ****************************/
#define  BATTERY_PROTECT                DRIVER_ENABLE   // 是否开启电池保护功能(默认开启)      DRIVER_ENABLE: 开启电池保护    DRIVER_DISABLE: 不开启电池保护 
         
#define  BATTERY_PROTECT_VALUE_MAX      (4.3f)          // 允许的单节电芯最大电压 

#define  BATTERY_WARNING_VALUE          (3.9f)          // 单节电芯电压报警阈值 (暂未做出警报动作) 

#define  BATTERY_PROTECT_VALUE_MIN      (3.5f)          // 允许的单节电芯最小电压 低于此值将直接关闭电机  驱动板蓝灯高频闪烁(100ms）
         
#define  BATTERY_RECTIFY_COEFFICIENT    (0.9959968)     // 电池电压校准系数 根据实际硬件调整    
/**************************** 电池保护 ****************************/



/**************************** 堵转保护 ****************************/
#define  MOTOR_LOCKED_PROTECT           DRIVER_ENABLE   // 是否开启堵转保护功能(默认开启)      DRIVER_ENABLE: 开启堵转保护    DRIVER_DISABLE: 不开启堵转保护 
         
#define  MOTOR_LOCKED_DUTY_MAX          (0.2f)          // 堵转检测最大占空比(默认20%)  若超过此值并且电机未旋转 则认为可能堵转
         
#define  MOTOR_LOCKED_TIME              (500)           // 堵转检测时长(默认500ms)      若认为电机可能堵转 并且持续时长超过此值 则认定为堵转状态
/**************************** 堵转保护 ****************************/



/**************************** 过载保护 ****************************/
#define  MOTOR_OVERLOAD_PROTECT         DRIVER_ENABLE   // 是否开启过载保护功能(默认开启)   DRIVER_ENABLE: 开启过载保护    DRIVER_DISABLE: 不开启过载保护
         
#define  MOTOR_OVERLOAD_DUTY_MIN        (0.2f)          // 判断过载保护的最低占空比  低于该值则不进行过载判断(允许电机堵转)  默认 0.2（20% 占空比）

#define  MOTOR_OVERLOAD_TIME            (2000)          // 过载检测时长(默认2000ms)   若认为电机可能过载 并且持续时长超过此值 则认定为过载状态

#define  MOTOR_OVERLOAD_COEFFICIENT     (0.7f)          // 过载系数  使用速度与占空比的差比和值  超过该阈值则认为当前电机过载  默认0.4

                                                        // 举例：电机KV值为500，使用12v供电，占空比设置80%，但电机实际转速只有1500
                                                        //       那么差比和结果为：(0.8 - 0.25) / (0.8 + 0.25) = 0.5238，如果该值超过保护阈值，则触发保护，关闭电机输出  

                                                        // 该保护要求电机KV值正确设置    
                                                        // 该保护要求电机KV值正确设置
                                                        // 该保护要求电机KV值正确设置(重要的事情说三遍)

                                                        // 常见的可能触发情况：FOC模式并且没有正确矫正零点强制驱动电机、FOC模式并且电机背面径向磁铁松动、电机实际负载过大(外部强制增加负载)等
/**************************** 过载保护 ****************************/



/**************************** 失控保护 ****************************/
#define  MOTOR_LOSE_CONTROL_PROTECT     DRIVER_DISABLE   // 是否开启失控保护功能(默认关闭)      DRIVER_ENABLE: 开启失控保护    DRIVER_DISABLE: 不开启失控保护 

#define  MOTOR_LOSE_CONTROL_TIME        (500)            // 失控检测时长(默认500ms)     当驱动连续该时长未收到占空比设置指令，则认为电机失控，关闭输出
/**************************** 失控保护 ****************************/



/**************************** 干扰保护 ****************************/
#define  MOTOR_DISTURB_PROTECT          DRIVER_ENABLE   // 是否开启干扰保护功能(默认开启)      DRIVER_ENABLE: 开启干扰保护    DRIVER_DISABLE: 不开启干扰保护 

#define  MOTOR_DISTURB_SPEED            (500)           // 干扰保护检测占空比(默认500RPM)      当占空比改变较小(几乎无改变)，但电机转速在100ms内下降该阈值的转速，则触发干扰保护
/**************************** 干扰保护 ****************************/

#endif