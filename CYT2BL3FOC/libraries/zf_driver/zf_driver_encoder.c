/*********************************************************************************************************************
* CYT2BL37CEE Opensourec Library 即（CYT2BL37CEE 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2023 SEEKFREE 逐飞科技
*
* 本文件是 CYT2BL37CEE 开源库的一部分
*
* CYT2BL37CEE 开源库 是免费软件
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
* 文件名称          zf_driver_encoder
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MDK 5.38a
* 适用平台          CYT2BL37CEE
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2023-11-30       pudding            first version
* 2023-05-21       pudding            新增支持正交编码器
********************************************************************************************************************/

#include "sysclk/cy_sysclk.h"
#include "tcpwm/cy_tcpwm_quaddec.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "gpio/cy_gpio.h"
#include "trigmux/cy_trigmux.h"
#include "zf_driver_gpio.h"
#include "zf_common_debug.h"
#include "zf_driver_encoder.h"


static uint8 			encoder_mode[4]; 
en_clk_dst_t   encoder_clk[4] = {PCLK_TCPWM0_CLOCKS7, PCLK_TCPWM0_CLOCKS9, PCLK_TCPWM0_CLOCKS19, PCLK_TCPWM0_CLOCKS36};
volatile stc_TCPWM_GRP_CNT_t *encoder_module[4] = {TCPWM0_GRP0_CNT7, TCPWM0_GRP0_CNT9, TCPWM0_GRP0_CNT19, TCPWM0_GRP0_CNT36};
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取通道1引脚值
// 参数说明     ch1_pin       
// 返回参数     gpio_pin_enum
// 使用示例     get_ch1_pin(TIM_CH7_ENCODER_CH1_P7_6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum get_ch1_pin (encoder_channel1_enum ch1_pin)
{
    gpio_pin_enum temp_pin;
    switch(ch1_pin)
    {
        case TC_CH07_ENCODER_CH1_P02_0: 	temp_pin = P02_0; 	break;
        case TC_CH09_ENCODER_CH1_P05_0: 	temp_pin = P05_0; 	break;
        case TC_CH19_ENCODER_CH1_P08_0: 	temp_pin = P08_0; 	break;
        case TC_CH36_ENCODER_CH1_P12_0: 	temp_pin = P12_0; 	break;
    }
    return temp_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取通道1引脚复用关系
// 参数说明     ch1_pin       
// 返回参数     en_hsiom_sel_t
// 使用示例     get_ch1_pin(TIM_CH7_ENCODER_CH1_P7_6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t get_ch1_hsiom (encoder_channel1_enum ch1_pin)
{
    en_hsiom_sel_t temp_pin;
    switch(ch1_pin)
    {
        case TC_CH07_ENCODER_CH1_P02_0: 	temp_pin = P2_0_TCPWM0_TR_ONE_CNT_IN21; 	break;
        case TC_CH09_ENCODER_CH1_P05_0: 	temp_pin = P5_0_TCPWM0_TR_ONE_CNT_IN27; 	break;
        case TC_CH19_ENCODER_CH1_P08_0: 	temp_pin = P8_0_TCPWM0_TR_ONE_CNT_IN57; 	break;
        case TC_CH36_ENCODER_CH1_P12_0: 	temp_pin = P12_0_TCPWM0_TR_ONE_CNT_IN108; 	break;
    }
    return temp_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取通道2引脚值
// 参数说明     ch2_pin       
// 返回参数     gpio_pin_enum
// 使用示例     get_ch2_pin(TIM_CH7_ENCODER_CH1_P7_6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum get_ch2_pin (encoder_channel2_enum ch2_pin)
{
    gpio_pin_enum temp_pin;
    switch(ch2_pin)
    {
        case TC_CH07_ENCODER_CH2_P02_1: 	temp_pin = P02_1; 	break;
        case TC_CH09_ENCODER_CH2_P05_1: 	temp_pin = P05_1; 	break;
        case TC_CH19_ENCODER_CH2_P08_1: 	temp_pin = P08_1; 	break;
        case TC_CH36_ENCODER_CH2_P12_1: 	temp_pin = P12_1; 	break;
    }
    return temp_pin;
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取通道2引脚复用关系
// 参数说明     ch2_pin       
// 返回参数     en_hsiom_sel_t
// 使用示例     get_ch2_pin(TIM_CH7_ENCODER_CH1_P7_6);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static en_hsiom_sel_t get_ch2_hsiom (encoder_channel2_enum ch2_pin)
{
    en_hsiom_sel_t temp_pin;
    switch(ch2_pin)
    {
        case TC_CH07_ENCODER_CH2_P02_1: 	temp_pin = P2_1_TCPWM0_TR_ONE_CNT_IN22; 	break;
        case TC_CH09_ENCODER_CH2_P05_1: 	temp_pin = P5_1_TCPWM0_TR_ONE_CNT_IN28; 	break;
        case TC_CH19_ENCODER_CH2_P08_1: 	temp_pin = P8_1_TCPWM0_TR_ONE_CNT_IN58; 	break;
        case TC_CH36_ENCODER_CH2_P12_1: 	temp_pin = P12_1_TCPWM0_TR_ONE_CNT_IN109; 	break;
    }
    return temp_pin;
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      编码器数据采集
//  参数说明      gptn            选择所使用的定时器
//  返回参数      void
//  使用示例      int16 speed; speed = encoder_get_count(TIM2_ENCODER); // 使用T2定时器
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
int16 encoder_get_count (encoder_index_enum encoder_n)
{
    int16 encoder_data = 0;
    if(encoder_mode[encoder_n])
    {
        encoder_data = (int16)(Cy_Tcpwm_Counter_GetCounter(encoder_module[encoder_n]));
        encoder_data = gpio_get_level(get_ch2_pin((encoder_channel2_enum)encoder_n)) == 1? encoder_data : -encoder_data;
    }
    else
    {
        encoder_data = (int16)(0x00008000 - Cy_Tcpwm_QuadDec_GetCounter(encoder_module[encoder_n]));
        Cy_Tcpwm_QuadDec_SetCounter(encoder_module[encoder_n], 0x00008000);
        
    }
    return encoder_data;
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      编码器计数清除
//  参数说明      gptn            选择所使用的定时器
//  返回参数      void
//  使用示例      encoder_clear_count(TIM2_ENCODER);// 使用T2定时器
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void encoder_clear_count (encoder_index_enum encoder_n)
{
    if(encoder_mode[encoder_n])
    {
        Cy_Tcpwm_Counter_SetCounter(encoder_module[encoder_n], 0);
    }
    else
    {
        Cy_Tcpwm_QuadDec_SetCounter(encoder_module[encoder_n], 0);
    }
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      正交编码器采集初始化
//  参数说明      encoder_n       选择所使用的GPT12定时器
//  参数说明      ch1_pin         ENCODER 通道 1
//  参数说明      ch2_pin         ENCODER 通道 2
//  返回参数      void
//  使用示例      encoder_quad_init(TC_CH07_ENCODER, TC_CH07_ENCODER_CH1_P02_0, TC_CH07_ENCODER_CH2_P02_1);// 使用tcpwm模块的通道7   P7_6 引脚为A通道    P7_7引脚为B通道
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void encoder_quad_init (encoder_index_enum encoder_n, encoder_channel1_enum ch1_pin, encoder_channel2_enum ch2_pin)
{
    cy_stc_gpio_pin_config_t  pwm_pin_config    = {0};
    cy_stc_tcpwm_quaddec_config_t encoder_quad_config;
    
    pwm_pin_config.driveMode                    = CY_GPIO_DM_HIGHZ;
    pwm_pin_config.hsiom                        = get_ch1_hsiom(ch1_pin);
    Cy_GPIO_Pin_Init(get_port(get_ch1_pin(ch1_pin)), (get_ch1_pin(ch1_pin) % 8), &pwm_pin_config);
    
    pwm_pin_config.driveMode                    = CY_GPIO_DM_HIGHZ;
    pwm_pin_config.hsiom                        = get_ch2_hsiom(ch2_pin);
    Cy_GPIO_Pin_Init(get_port(get_ch2_pin(ch2_pin)), (get_ch2_pin(ch2_pin) % 8), &pwm_pin_config);
    
    Cy_SysClk_PeriphAssignDivider(encoder_clk[encoder_n], CY_SYSCLK_DIV_16_BIT, 0ul);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0ul, 9u); // 80Mhz时钟被10分频为8Mhz
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0ul);
   
    
    Cy_Tcpwm_QuadDec_DeInit(encoder_module[encoder_n]);
    
    encoder_quad_config.encoding_mode           = CY_TCPWM_QUADDEC_X1           ;
    encoder_quad_config.range_mode              = CY_TCPWM_QUADDEC_RANGE_0      ;
    encoder_quad_config.interruptSources        = CY_TCPWM_INT_NONE             ;
    encoder_quad_config.indexInputMode          = CY_TCPWM_INPUT_RISING_EDGE    ;
    encoder_quad_config.indexInput              = CY_TCPWM_INPUT0               ;
    encoder_quad_config.stopInputMode           = CY_TCPWM_INPUT_LEVEL          ;
    encoder_quad_config.stopInput               = CY_TCPWM_INPUT0               ;
    encoder_quad_config.phiAInput               = CY_TCPWM_INPUT_TRIG0          ;
    encoder_quad_config.phiBInput               = CY_TCPWM_INPUT_TRIG1          ;  
    
    Cy_Tcpwm_QuadDec_Init(encoder_module[encoder_n], &encoder_quad_config);
    Cy_Tcpwm_QuadDec_Enable(encoder_module[encoder_n]);
    Cy_Tcpwm_TriggerStart(encoder_module[encoder_n]);
    Cy_Tcpwm_TriggerReloadOrIndex(encoder_module[encoder_n]);
    
    encoder_mode[encoder_n]  = 0;
}       

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      方向编码器采集初始化
//  参数说明      encoder_n       选择所使用的GPT12定时器
//  参数说明      ch1_pin         设置计数引脚
//  参数说明      ch2_pin         设置方向引脚
//  返回参数      void
//  使用示例      encoder_dir_init(TC_CH7_ENCODER, TC_CH7_ENCODER_CH1_P7_6, TC_CH7_ENCODER_CH2_P7_7);// 使用TCPWM定时器   P7_6引脚进行计数    计数方向使用P7_7引脚
//  备注信息      
//-------------------------------------------------------------------------------------------------------------------
void encoder_dir_init (encoder_index_enum encoder_n, encoder_channel1_enum count_pin, encoder_channel2_enum dir_pin)
{
    cy_stc_gpio_pin_config_t  pwm_pin_config    = {0};
    cy_stc_tcpwm_counter_config_t encoder_dir_config;
    
    // 醒醒，模块号和端口都不对应怎么能初始化呢？
    zf_assert((uint8)encoder_n == (uint8)count_pin ? 1 : 0);
    zf_assert((uint8)count_pin == (uint8)dir_pin ? 1 : 0);
    
    Cy_SysClk_PeriphAssignDivider(encoder_clk[encoder_n], CY_SYSCLK_DIV_16_BIT, 0ul);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 0ul, 9u); // 80Mhz时钟被10分频为8Mhz
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 0ul);
  
    pwm_pin_config.driveMode                    = CY_GPIO_DM_STRONG;
    pwm_pin_config.hsiom                        = get_ch1_hsiom(count_pin);
    Cy_GPIO_Pin_Init(get_port(get_ch1_pin(count_pin)), (get_ch1_pin(count_pin) % 8), &pwm_pin_config);
  
    pwm_pin_config.driveMode                    = CY_GPIO_DM_PULLDOWN;
    pwm_pin_config.hsiom                        = HSIOM_SEL_GPIO;
    Cy_GPIO_Pin_Init(get_port(get_ch2_pin(dir_pin)), (get_ch2_pin(dir_pin) % 8), &pwm_pin_config);
  
    memset(&encoder_dir_config, 0, sizeof(encoder_dir_config));
    
    encoder_dir_config.period             = 0xffff                             ;  
    encoder_dir_config.compare0           = 0xffff                             ;
    encoder_dir_config.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1         ;
    encoder_dir_config.runMode            = CY_TCPWM_COUNTER_CONTINUOUS        ; 
    encoder_dir_config.countDirection     = CY_TCPWM_COUNTER_COUNT_UP          ;
    encoder_dir_config.compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE      ;
    encoder_dir_config.countInputMode     = CY_TCPWM_INPUT_RISING_EDGE         ;
    encoder_dir_config.countInput         = 2uL                                ;
  
    Cy_Tcpwm_Counter_Init(encoder_module[encoder_n], &encoder_dir_config);
    Cy_Tcpwm_Counter_Enable(encoder_module[encoder_n]);
    Cy_Tcpwm_TriggerStart(encoder_module[encoder_n]);
    Cy_Tcpwm_Counter_SetCounter(encoder_module[encoder_n], 0);
    
    encoder_mode[encoder_n]  = 1;
}

