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
* 文件名称          zf_driver_pit
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-9       pudding            first version
********************************************************************************************************************/

#include "sysclk/cy_sysclk.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "zf_driver_timer.h"


static uint8 driver_timer_mode[3];

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIMER 开始计时
// 参数说明     index           TIMER 外设模块号
// 返回参数     void
// 使用示例     timer_start(TC_TIME2_CH0);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void timer_start (timer_index_enum index)
{
    Cy_Tcpwm_Counter_Enable((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3]);
    Cy_Tcpwm_TriggerStart((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3]);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIMER 停止计时
// 参数说明     index           TIMER 外设模块号
// 返回参数     void
// 使用示例     timer_stop(TC_TIME2_CH0);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void timer_stop (timer_index_enum index)
{
    Cy_Tcpwm_Counter_Disable((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3]);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIMER 获取计时器计数
// 参数说明     index           TIMER 外设模块号
// 返回参数     uint16          计数值
// 使用示例     timer_get(TC_TIME2_CH0);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint32 timer_get (timer_index_enum index)
{
    return ((Cy_Tcpwm_Counter_GetCounter((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3])) / (driver_timer_mode[index] == TIMER_MS ? 8000 : 8));
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIMER 清除计时器计数
// 参数说明     index           TIMER 外设模块号
// 返回参数     void
// 使用示例     timer_clear(TC_TIME2_CH0);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void timer_clear (timer_index_enum index)
{
    Cy_Tcpwm_Counter_SetCounter((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3], 0);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIMER 计时器初始化
// 参数说明     index           TIMER 外设模块号
// 参数说明     mode            计时方式
// 返回参数     void
// 使用示例     timer_init(TC_TIME2_CH0, TIMER_MS);
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
void timer_init (timer_index_enum index, timer_mode_enum mode)
{
    cy_stc_tcpwm_counter_config_t timer_config  = {0};
    driver_timer_mode[index]                    = mode;
    
    Cy_SysClk_PeriphAssignDivider((en_clk_dst_t)((uint32)index + (uint32)PCLK_TCPWM0_CLOCKS515), (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul);
    Cy_SysClk_PeriphSetDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul, 9u); // 80Mhz时钟被10分频为8Mhz
    Cy_SysClk_PeriphEnableDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul);
    
    timer_config.period             = 0xffffffff                         ;        
    timer_config.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1         ;
    timer_config.runMode            = CY_TCPWM_COUNTER_CONTINUOUS        ; 
    timer_config.countDirection     = CY_TCPWM_COUNTER_COUNT_UP          ;
    timer_config.compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE      ;
    timer_config.countInputMode     = CY_TCPWM_INPUT_LEVEL               ;
    timer_config.countInput         = 1uL                                ;
    timer_config.debug_pause        = 1uL                                ;
    
    Cy_Tcpwm_Counter_Init((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3], &timer_config);
    Cy_Tcpwm_Counter_Disable((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3]);
    Cy_Tcpwm_Counter_SetCounter((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[index + 3], 0);
}




