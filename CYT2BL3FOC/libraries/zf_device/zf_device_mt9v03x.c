/*********************************************************************************************************************
* MM32F527X-E9P Opensource Library 即（MM32F527X-E9P 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
* 
* 本文件是 MM32F527X-E9P 开源库的一部分
* 
* MM32F527X-E9P 开源库 是免费软件
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
* 文件名称          zf_device_mt9v03x
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2024-11-19       pudding            first version
********************************************************************************************************************/
/*********************************************************************************************************************
* 接线定义：
*                   ------------------------------------
*                   模块管脚            单片机管脚
*                   TXD                 查看 zf_device_mt9v03x.h 中 MT9V03X_COF_UART_TX 宏定义
*                   RXD                 查看 zf_device_mt9v03x.h 中 MT9V03X_COF_UART_RX 宏定义
*                   PCLK                查看 zf_device_mt9v03x.h 中 MT9V03X_PCLK_PIN 宏定义
*                   VSY                 查看 zf_device_mt9v03x.h 中 MT9V03X_VSYNC_PIN 宏定义
*                   D0-D7               查看 zf_device_mt9v03x.h 中 MT9V03X_DATA_PIN 宏定义 从该定义开始的连续八个引脚
*                   VCC                 3.3V电源
*                   GND                 电源地
*                   其余引脚悬空
*                   ------------------------------------
********************************************************************************************************************/

#include "sysclk/cy_sysclk.h"
#include "tcpwm/cy_tcpwm_pwm.h"
#include "zf_common_interrupt.h"

#include "zf_driver_soft_iic.h"
#include "zf_device_config.h"
#include "zf_device_mt9v03x.h"

vuint8 mt9v03x_finish_flag = 0;                                                 // 一场图像采集完成标志位
uint8 mt9v03x_image[MT9V03X_H][MT9V03X_W];     

#pragma location = 0x08020658                                                    // 将下面这个数组定义到指定的RAM地址
__no_init uint8  mt9v03x_image_temp[MT9V03X_H][MT9V03X_W];                      

void camera_finish_callback(void)
{
    Cy_Tcpwm_Counter_ClearTC_Intr(TCPWM0_GRP0_CNT1);

    memcpy(mt9v03x_image[0], mt9v03x_image_temp[0], MT9V03X_IMAGE_SIZE);
    
    mt9v03x_finish_flag = 1;
}

static void mt9v03x_trig_init(void)
{   
    Cy_SysClk_PeriphAssignDivider(PCLK_TCPWM0_CLOCKS1, (cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul);
    Cy_SysClk_PeriphSetDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul, 9u); // 80Mhz时钟被10分频为8Mhz
    Cy_SysClk_PeriphEnableDivider((cy_en_divider_types_t)CY_SYSCLK_DIV_16_BIT, 0ul);

    cy_stc_sysint_irq_t mt9v03x_trig_irq_cfg;
    mt9v03x_trig_irq_cfg.sysIntSrc  = tcpwm_0_interrupts_1_IRQn; 
    mt9v03x_trig_irq_cfg.intIdx     = CPUIntIdx3_IRQn;
    mt9v03x_trig_irq_cfg.isEnabled  = true;
    interrupt_init(&mt9v03x_trig_irq_cfg, camera_finish_callback, 0);

    cy_stc_tcpwm_counter_config_t tcpwm_camera_config;
    memset(&tcpwm_camera_config, 0, sizeof(tcpwm_camera_config));
    tcpwm_camera_config.period             = 0x0                                ;        // pit周期计算
    tcpwm_camera_config.clockPrescaler     = CY_TCPWM_PRESCALER_DIVBY_1         ;
    tcpwm_camera_config.runMode            = CY_TCPWM_COUNTER_ONESHOT           ; 
    tcpwm_camera_config.countDirection     = CY_TCPWM_COUNTER_COUNT_UP          ;
    tcpwm_camera_config.compareOrCapture   = CY_TCPWM_COUNTER_MODE_COMPARE      ;
    tcpwm_camera_config.countInputMode     = CY_TCPWM_INPUT_LEVEL               ;
    tcpwm_camera_config.countInput         = 1uL                                ;
    tcpwm_camera_config.trigger0EventCfg   = CY_TCPWM_COUNTER_OVERFLOW          ;
    tcpwm_camera_config.trigger1EventCfg   = CY_TCPWM_COUNTER_OVERFLOW          ;
        
    Cy_Tcpwm_Counter_Init(TCPWM0_GRP0_CNT1, &tcpwm_camera_config);
    Cy_Tcpwm_Counter_Enable(TCPWM0_GRP0_CNT1);
    Cy_Tcpwm_Counter_SetTC_IntrMask(TCPWM0_GRP0_CNT1);
    Cy_Tcpwm_TriggerStart(TCPWM0_GRP0_CNT0);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     单独设置摄像头曝光时间
// 参数说明     light           设定曝光时间
// 返回参数     uint8           1-失败 0-成功
// 使用示例     mt9v03x_set_exposure_time(100);                 // 调用该函数前请先初始化串口
// 备注信息     设置曝光时间越大图像越亮
//              摄像头收到后会根据分辨率及FPS计算最大曝光时间如果设置的数据过大
//              那么摄像头将会设置这个最大值
//-------------------------------------------------------------------------------------------------------------------
uint8 mt9v03x_set_exposure_time (uint16 light)
{
    uint8   return_state        = 0;
    
    return_state = mt9v03x_sccb_set_exposure_time(light);

    return return_state;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     MT9V03X SCCB 初始化
// 参数说明     void
// 返回参数     uint8           0-成功 x-失败
// 使用示例     mt9v03x_sccb_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 mt9v03x_sccb_init (void)
{
    uint8 return_state  = 1;
    
    soft_iic_info_struct mt9v03x_iic_struct;
    soft_iic_init(
        &mt9v03x_iic_struct, 0,
        MT9V03X_COF_IIC_DELAY,
        MT9V03X_COF_IIC_SCL,
        MT9V03X_COF_IIC_SDA);
    
    if(!mt9v03x_sccb_check_id(&mt9v03x_iic_struct))
    {
        // 需要配置到摄像头的数据 不允许在这修改参数
        const int16 mt9v03x_set_confing_buffer[MT9V03X_CONFIG_FINISH][2]=
        {
            {MT9V03X_INIT,              0},                                     // 摄像头开始初始化

            {MT9V03X_AUTO_EXP,          MT9V03X_AUTO_EXP_DEF},                  // 自动曝光设置
            {MT9V03X_EXP_TIME,          MT9V03X_EXP_TIME_DEF},                  // 曝光时间
            {MT9V03X_FPS,               MT9V03X_FPS_DEF},                       // 图像帧率
            {MT9V03X_SET_COL,           MT9V03X_W},                             // 图像列数量
            {MT9V03X_SET_ROW,           MT9V03X_H},                             // 图像行数量
            {MT9V03X_LR_OFFSET,         MT9V03X_LR_OFFSET_DEF},                 // 图像左右偏移量
            {MT9V03X_UD_OFFSET,         MT9V03X_UD_OFFSET_DEF},                 // 图像上下偏移量
            {MT9V03X_GAIN,              MT9V03X_GAIN_DEF},                      // 图像增益
            {MT9V03X_PCLK_MODE,         MT9V03X_PCLK_MODE_DEF},                 // 像素时钟模式
        };
        return_state = mt9v03x_sccb_set_config(mt9v03x_set_confing_buffer);
    }
    
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     MT9V03X 摄像头初始化
// 参数说明     void
// 返回参数     uint8           0-成功 x-失败
// 使用示例     mt9v03x_init();
// 备注信息     
//-------------------------------------------------------------------------------------------------------------------
uint8 mt9v03x_init (void)
{
    uint8 return_state  = 0;

    do
    {
        return_state = mt9v03x_sccb_init();
        
        mt9v03x_trig_init();
        
        if(!return_state)
        {
            break;
        }

    }while(0);

    return return_state;
}
