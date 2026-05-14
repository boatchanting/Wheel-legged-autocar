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
* 文件名称          zf_driver_adc
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-8       pudding            first version
* 2024-1-29      pudding            优化提高采集速度 单次采集时间0.525us左右
********************************************************************************************************************/

#include "sysclk/cy_sysclk.h"
#include "gpio/cy_gpio.h"
#include "zf_driver_gpio.h"
#include "adc/cy_adc.h"
#include "zf_common_debug.h"
#include "zf_driver_adc.h"

#define ADC_FREQ                                CY_INITIAL_TARGET_PERI_FREQ     // ADC模块时钟 默认80M
#define ADC_OPERATION_FREQUENCY_MAX_IN_HZ       (13340000ul)                    // ADC工作频率 最高13.34M
#define ANALOG_IN_SAMPLING_TIME_MIN_IN_NS       (600ull)                        // ADC采样时间 最短600ns
#define DIV_ROUND_UP(a,b)                       (((a) + (b)/2) / (b))           // 分配系数计算 向上取整

static vuint8 adc_resolution[ADC_CHANNEL_NUM];
static volatile stc_PASS_SAR_t* sar_adc[3] = {PASS0_SAR0, PASS0_SAR1, PASS0_SAR2};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 端口号获取
// 参数说明     vadc_chn        选择 ADC 通道 (详见 zf_driver_adc.h 中枚举 adc_channel_enum 定义)
// 返回参数     gpio_pin_enum   ADC通道对应的GPIO引脚
// 使用示例     get_adc_pin(ADC0_CH0_P6_0);
// 备注信息     内部函数 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static gpio_pin_enum get_adc_pin(adc_channel_enum vadc_chn)
{
    gpio_pin_enum adc_pin;
    switch(vadc_chn)
    {
        case ADC0_CH00_P06_0	: adc_pin = P06_0; break;
        case ADC0_CH01_P06_1	: adc_pin = P06_1; break;
        case ADC0_CH02_P06_2	: adc_pin = P06_2; break;
        case ADC0_CH03_P06_3	: adc_pin = P06_3; break;
        case ADC0_CH04_P06_4	: adc_pin = P06_4; break;
        case ADC0_CH05_P06_5	: adc_pin = P06_5; break;
        case ADC0_CH06_P06_6	: adc_pin = P06_6; break;
        case ADC0_CH16_P07_0	: adc_pin = P07_0; break;
        case ADC0_CH17_P07_1	: adc_pin = P07_1; break;
        case ADC0_CH18_P07_2	: adc_pin = P07_2; break;
        case ADC0_CH24_P08_1	: adc_pin = P08_1; break;
        case ADC1_CH04_P12_0	: adc_pin = P12_0; break;
        case ADC1_CH05_P12_1	: adc_pin = P12_1; break;
        case ADC1_CH12_P13_0	: adc_pin = P13_0; break;
        case ADC1_CH13_P13_1	: adc_pin = P13_1; break;
        case ADC1_CH14_P13_2	: adc_pin = P13_2; break;
        case ADC1_CH15_P13_3	: adc_pin = P13_3; break;
        case ADC1_CH20_P14_0	: adc_pin = P14_0; break;
        case ADC1_CH21_P14_1	: adc_pin = P14_1; break;
        case ADC1_CH22_P14_2	: adc_pin = P14_2; break;
        case ADC2_CH00_P18_0	: adc_pin = P18_0; break;
        case ADC2_CH01_P18_1	: adc_pin = P18_1; break;
        case ADC2_CH03_P18_3	: adc_pin = P18_3; break;
        case ADC2_CH04_P18_4	: adc_pin = P18_4; break;
        case ADC2_CH05_P18_5	: adc_pin = P18_5; break;
        case ADC2_CH06_P18_6	: adc_pin = P18_6; break;
        case ADC2_CH07_P18_7	: adc_pin = P18_7; break;
        default:break;
    }
    return adc_pin;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 转换一次
// 参数说明     ch              选择 ADC 通道 (详见 zf_driver_adc.h 中枚举 adc_channel_enum 定义)
// 返回参数     uint16          转换的 ADC 值
// 使用示例     adc_convert(ADC0_CH00_P06_0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_convert (adc_channel_enum adc_chn)
{
    uint8 offset_temp;
    uint8 adc_port      = adc_chn / 32;
    uint8 adc_channel   = adc_chn % 32;
    uint16 adc_data;
    
    offset_temp = 4 - (adc_resolution[adc_chn] * 2);
    
    sar_adc[adc_port]->CH[adc_channel].unTR_CMD.stcField.u1START = 1ul;
    
    adc_data = sar_adc[adc_port]->CH[adc_channel].unRESULT.stcField.u16RESULT;
    
    adc_data = adc_data >> offset_temp;
    
    return adc_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 均值滤波转换
// 参数说明     ch              选择 ADC 通道 (详见 zf_driver_adc.h 中枚举 adc_channel_enum 定义)
// 参数说明     count           均值滤波次数
// 返回参数     uint16          转换的 ADC 值
// 使用示例     adc_mean_filter_convert(ADC0_CH00_P06_0, 5);                        // 采集5次 然后返回平均值
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_mean_filter_convert (adc_channel_enum vadc_chn, uint8 count)
{
    uint8 i;
    uint32 sum;

    zf_assert(count);// 断言次数不能为0

    sum = 0;
    for(i=0; i<count; i++)
    {
        sum += adc_convert(vadc_chn);
    }

    sum = sum/count;

    return (uint16)sum;
}


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     ADC 初始化
// 参数说明     ch              选择 ADC 通道 (详见 zf_driver_adc.h 中枚举 adc_channel_enum 定义)
// 参数说明     resolution      选择选择通道分辨率(如果同一个 ADC 模块初始化时设置了不同的分辨率 则最后一个初始化的分辨率生效)
// 返回参数     void
// 使用示例     adc_init(ADC0_CH00_P06_0, ADC_12BIT);                                // 初始化 A0 为 ADC1 的 channel0 输入功能 分辨率为8位
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void adc_init (adc_channel_enum adc_chn, adc_resolution_enum resolution)
{	
    uint32_t actual_adc_operation_freq          = 0;
    uint32_t div_num                            = 0;
    uint32_t sampling_cycle                     = 0;
    cy_stc_gpio_pin_config_t adc_pin_config     = {0};
    cy_stc_adc_config_t adc_config              = {0};
    adc_resolution[adc_chn]                     = resolution;
    cy_stc_adc_channel_config_t adc_channel_config;
    
    Cy_Adc_Channel_DeInit(&sar_adc[adc_chn / 32]->CH[adc_chn % 32]);
    
    adc_pin_config.driveMode    = CY_GPIO_DM_ANALOG;
    Cy_GPIO_Pin_Init(get_port(get_adc_pin(adc_chn)), (get_adc_pin(adc_chn) % 8), &adc_pin_config);
    
    div_num = DIV_ROUND_UP(ADC_FREQ, ADC_OPERATION_FREQUENCY_MAX_IN_HZ);
    actual_adc_operation_freq = ADC_FREQ / div_num;
    
    Cy_SysClk_PeriphAssignDivider((en_clk_dst_t)((adc_chn / 32) + PCLK_PASS0_CLOCK_SAR0), CY_SYSCLK_DIV_16_BIT, 1u);
    Cy_SysClk_PeriphSetDivider(CY_SYSCLK_DIV_16_BIT, 1u, (div_num - 1));
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_16_BIT, 1u);

    adc_config.msbStretchMode           = CY_ADC_MSB_STRETCH_MODE_1CYCLE;
    adc_config.sarMuxEnable             = true;
    adc_config.adcEnable                = true;
    adc_config.sarIpEnable              = true;
    Cy_Adc_Init(sar_adc[adc_chn / 32], &adc_config);
    
    sampling_cycle = (uint32_t)DIV_ROUND_UP((ANALOG_IN_SAMPLING_TIME_MIN_IN_NS * (uint64_t)actual_adc_operation_freq), 1000000000ul);
    
    memset(&adc_channel_config, 0, sizeof(adc_channel_config));
    adc_channel_config.preenptionType   = CY_ADC_PREEMPTION_FINISH_RESUME;
    adc_channel_config.pinAddress       = (cy_en_adc_pin_address_t)(adc_chn % 32);
    adc_channel_config.extMuxEnable     = true;
    adc_channel_config.sampleTime       = sampling_cycle;
    adc_channel_config.isGroupEnd       = true;
    adc_channel_config.mask.grpDone     = true;
    Cy_Adc_Channel_Init(&sar_adc[adc_chn / 32]->CH[adc_chn % 32], &adc_channel_config);
    Cy_Adc_Channel_Enable(&sar_adc[adc_chn / 32]->CH[adc_chn % 32]);
    Cy_Adc_Channel_SoftwareTrigger(&sar_adc[adc_chn / 32]->CH[adc_chn % 32]);
}

