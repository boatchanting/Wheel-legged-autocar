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
* 文件名称          zf_driver_exti
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-8       pudding            first version
********************************************************************************************************************/

#include "gpio/cy_gpio.h"
#include "zf_common_interrupt.h"
#include "zf_common_debug.h"
#include "zf_driver_exti.h"

#define EXTI_USE_ISR    CPUIntIdx5_IRQn


void gpio_0_exti_isr();
void gpio_1_exti_isr();
void gpio_2_exti_isr();
void gpio_3_exti_isr();
void gpio_4_exti_isr();
void gpio_5_exti_isr();
void gpio_6_exti_isr();
void gpio_7_exti_isr();
void gpio_8_exti_isr();
void gpio_9_exti_isr();
void gpio_10_exti_isr();
void gpio_11_exti_isr();
void gpio_12_exti_isr();
void gpio_13_exti_isr();
void gpio_14_exti_isr();
void gpio_15_exti_isr();
void gpio_16_exti_isr();
void gpio_17_exti_isr();
void gpio_18_exti_isr();
void gpio_19_exti_isr();
void gpio_20_exti_isr();
void gpio_21_exti_isr();
void gpio_22_exti_isr();
void gpio_23_exti_isr();

void (*exti_isr_func[24])() = {gpio_0_exti_isr, gpio_1_exti_isr, gpio_2_exti_isr, gpio_3_exti_isr, gpio_4_exti_isr, gpio_5_exti_isr, gpio_6_exti_isr, gpio_7_exti_isr, 
                                gpio_8_exti_isr, gpio_9_exti_isr, gpio_10_exti_isr, gpio_11_exti_isr, gpio_12_exti_isr, gpio_13_exti_isr, gpio_14_exti_isr, gpio_15_exti_isr, 
                                gpio_16_exti_isr, gpio_17_exti_isr, gpio_18_exti_isr, gpio_19_exti_isr, gpio_20_exti_isr, gpio_21_exti_isr, gpio_22_exti_isr, gpio_23_exti_isr};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取引脚是否中断
// 参数说明     exti_pin             选择 EXTI 引脚 (可选择范围由 zf_driver_exti.h 内 exti_pin_enum 枚举值确定)
// 返回参数     0：该端口未中断    其他：该端口触发中断
// 使用示例     exti_flag_get(P10_7);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 exti_flag_get(gpio_pin_enum exti_pin)
{
    uint8 return_data = 0;
    
    if(Cy_GPIO_GetInterruptStatusMasked(get_port(exti_pin), (exti_pin % 8)))
    {
        Cy_GPIO_ClearInterrupt(get_port(exti_pin), (exti_pin % 8));
        return_data =1;
    }
  
    return return_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断使能
// 参数说明     pin             选择 EXTI 引脚 (可选择范围由 zf_driver_exti.h 内 exti_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_enable(ERU_CH0_REQ4_P10_7);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_enable (gpio_pin_enum exti_pin)
{
    Cy_GPIO_SetInterruptMask(get_port(exti_pin), (exti_pin % 8), 1);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断失能
// 参数说明     pin             选择 EXTI 引脚 (可选择范围由 zf_driver_exti.h 内 exti_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_disable(A0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_disable (gpio_pin_enum exti_pin)
{
    Cy_GPIO_SetInterruptMask(get_port(exti_pin), (exti_pin % 8), 0);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      EXTI 失能
//  返回参数      void
//  使用示例      exti_disable(ERU_CH0_REQ0_P15_4);
//-------------------------------------------------------------------------------------------------------------------
void exti_all_close (void)
{
    for(int i = 0; i < 24; i ++)
    {
        for(int j = 0; j < 8; j ++)
        {
            Cy_GPIO_SetInterruptMask(get_port(i * 8), j, 0);
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      EXTI 中断初始化
//  参数说明      eru_pin         设置eru通道及引脚
//  参数说明      trigger         设置触发方式
//  返回参数      void
//  使用示例      exti_init(P15_4, EXTI_TRIGGER_RISING);   // 使用P15_4引脚，上升沿触发中断
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_init (gpio_pin_enum exti_pin, exti_trigger_enum trigger)
{
    cy_stc_gpio_pin_config_t    exti_pin_cfg    = {0};
    cy_stc_sysint_irq_t         exti_irq_cfg;
    
    exti_pin_cfg.driveMode = CY_GPIO_DM_HIGHZ;
    exti_pin_cfg.intEdge   = trigger;
    exti_pin_cfg.intMask   = 1ul;
    Cy_GPIO_Pin_Init(get_port(exti_pin), (exti_pin % 8), &exti_pin_cfg);
    
    exti_irq_cfg.sysIntSrc  = (cy_en_intr_t)(ioss_interrupts_gpio_0_IRQn + exti_pin / 8);
    exti_irq_cfg.intIdx     = EXTI_USE_ISR;
    exti_irq_cfg.isEnabled  = true;
    interrupt_init(&exti_irq_cfg, exti_isr_func[exti_pin / 8], 0);
}
