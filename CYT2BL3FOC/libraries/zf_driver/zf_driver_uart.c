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
* 文件名称          zf_driver_uart
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          IAR 9.40.1
* 适用平台          CYT2BL3
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2024-1-5       pudding            first version
* 2024-3-2       pudding            修复多个串口波特率差距过大导致波特率异常的问题
********************************************************************************************************************/

#include "system_cyt2bl.h"
#include "sysclk/cy_sysclk.h"
#include "gpio/cy_gpio.h"
#include "zf_common_interrupt.h"
#include "zf_common_debug.h"
#include "zf_driver_gpio.h"
#include "zf_driver_uart.h"
#include "zf_common_fifo.h"

#define UART_FREQ       CY_INITIAL_TARGET_PERI_FREQ                             // 串口模块时钟 默认80M
#define UART_USE_ISR    CPUIntIdx7_IRQn

void uart0_isr (void);
void uart1_isr (void);
void uart2_isr (void);
void uart3_isr (void);
void uart4_isr (void);

typedef struct
{ 
    volatile stc_GPIO_PRT_t     *rx_port;
    uint8                       rx_pin;
    en_hsiom_sel_t              rx_hsiom;
    volatile stc_GPIO_PRT_t     *tx_port;
    uint8                       tx_pin;
    en_hsiom_sel_t              tx_hsiom;
    en_clk_dst_t                uart_pclk;
    cy_en_intr_t                uart_irqn;
}uart_config_struct;

void (*uart_isr_func[5])() = {uart0_isr, uart1_isr, uart2_isr, uart3_isr, uart4_isr};
cy_stc_scb_uart_context_t  uart_context[5] = {0}; 
volatile stc_SCB_t* scb_module[5] = {SCB0, SCB4, SCB5, SCB3, SCB2};

static fifo_struct      uart_data_fifo_interior[5];              
static uint8            uart_data_buffer_interior[5][64] = {0};
   
//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口中断预处理 
// 参数说明       uart_n         串口号
// 返回参数       uint8          0：发送中断    1：接收中断  
// 使用示例       uart_isr_mask(UART_0);                  
// 备注信息       内部函数 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_isr_mask(uart_index_enum uart_n)
{
    uint8 isr_type = 1;                                                                     // 中断类型  0：发送中断   1：接收中断  
    
    uint8 receiv_limit = 10;
    
    if(Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & CY_SCB_UART_RX_NOT_EMPTY)            // 串口接收中断
    {
        while(Cy_SCB_GetNumInRxFifo(scb_module[uart_n]))
        {
            fifo_write_element(&uart_data_fifo_interior[uart_n], (uint8)Cy_SCB_ReadRxFifo(scb_module[uart_n]));                   // 将数据写入 fifo
            if((-- receiv_limit) == 0)
            {
                break;
            }
        }
        Cy_SCB_ClearRxInterrupt(scb_module[uart_n], CY_SCB_UART_RX_NOT_EMPTY);              // 清除接收中断标志位
    }
    else if(Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & CY_SCB_UART_TX_DONE)            // 串口0发送中断
    {           
        Cy_SCB_ClearTxInterrupt(scb_module[uart_n], CY_SCB_UART_TX_DONE);                   // 清除接收中断标志位
        
        isr_type = 0;
    }
    
    return isr_type;
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口中断函数调度
// 参数说明       isr_num         中断号
// 返回参数       void
// 使用示例       uart_isr_function(CPUSS->unCM4_INT7_STATUS.stcField.u10SYSTEM_INT_IDX);                  
// 备注信息       内部函数 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
void uart_isr_function(uint32 isr_num)
{
    switch(isr_num)
    {
        case 17: uart0_isr(); break;
        case 92: uart1_isr(); break;
        case 93: uart2_isr(); break;
        case 91: uart3_isr(); break;
        case 90: uart4_isr(); break;
        default:break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       获取串口配置信息
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       tx_pin          串口发送引脚
// 参数说明       rx_pin          串口接收引脚
// 返回参数       void
// 使用示例       get_uart_config(&uart_pin_config, tx_pin, rx_pin);                  
// 备注信息       内部函数 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
static void get_uart_config(uart_config_struct *config_struct, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    switch(tx_pin)
    {
        case  	UART0_TX_P00_1:
        {
            config_struct->tx_port  = GPIO_PRT0 ;  	
            config_struct->tx_pin   = 1;      
            config_struct->tx_hsiom = P0_1_SCB0_UART_TX;  
            config_struct->uart_pclk= PCLK_SCB0_CLOCK;
            config_struct->uart_irqn= scb_0_interrupt_IRQn;
        }break;
        case	UART1_TX_P06_1:
        {
            config_struct->tx_port  = GPIO_PRT6 ;  	
            config_struct->tx_pin   = 1;      
            config_struct->tx_hsiom = P6_1_SCB4_UART_TX; 
            config_struct->uart_pclk= PCLK_SCB4_CLOCK;
            config_struct->uart_irqn= scb_4_interrupt_IRQn;
        }break;
        case	UART2_TX_P07_1:
        {
            config_struct->tx_port  = GPIO_PRT7;  	
            config_struct->tx_pin   = 1;      
            config_struct->tx_hsiom = P7_1_SCB5_UART_TX; 
            config_struct->uart_pclk= PCLK_SCB5_CLOCK;
            config_struct->uart_irqn= scb_5_interrupt_IRQn;
        }break;
        case  	UART3_TX_P13_1: 
        {
            config_struct->tx_port  = GPIO_PRT13;  	
            config_struct->tx_pin   = 1;      
            config_struct->tx_hsiom = P13_1_SCB3_UART_TX; 
            config_struct->uart_pclk= PCLK_SCB3_CLOCK;
            config_struct->uart_irqn= scb_3_interrupt_IRQn;
        }break;
        case  	UART4_TX_P14_1:
        {
            config_struct->tx_port  = GPIO_PRT14;  	
            config_struct->tx_pin   = 1;      
            config_struct->tx_hsiom = P14_1_SCB2_UART_TX; 
            config_struct->uart_pclk= PCLK_SCB2_CLOCK;
            config_struct->uart_irqn= scb_2_interrupt_IRQn;
        }break;
        default: zf_assert(0);     break;
    }
    switch(rx_pin)
    {
        case  	UART0_RX_P00_0:
        {
            config_struct->rx_port  = GPIO_PRT0 ;  	
            config_struct->rx_pin   = 0;      
            config_struct->rx_hsiom = P0_0_SCB0_UART_RX;  
            config_struct->uart_pclk= PCLK_SCB0_CLOCK;
            config_struct->uart_irqn= scb_0_interrupt_IRQn;
        }break;
        case	UART1_RX_P06_0: 
        {
            config_struct->rx_port  = GPIO_PRT6 ;  	
            config_struct->rx_pin   = 0;      
            config_struct->rx_hsiom = P6_0_SCB4_UART_RX;
            config_struct->uart_pclk= PCLK_SCB4_CLOCK;
            config_struct->uart_irqn= scb_4_interrupt_IRQn;
        }break;
        case	UART2_RX_P07_0: 
        {
            config_struct->rx_port  = GPIO_PRT7;  	
            config_struct->rx_pin   = 0;      
            config_struct->rx_hsiom = P7_0_SCB5_UART_RX; 
            config_struct->uart_pclk= PCLK_SCB5_CLOCK;
            config_struct->uart_irqn= scb_5_interrupt_IRQn;
        }break;
        case  	UART3_RX_P13_0: 
        {
            config_struct->rx_port  = GPIO_PRT13;  	
            config_struct->rx_pin   = 0;      
            config_struct->rx_hsiom = P13_0_SCB3_UART_RX; 
            config_struct->uart_pclk= PCLK_SCB3_CLOCK;
            config_struct->uart_irqn= scb_3_interrupt_IRQn;
        }break;
        case  	UART4_RX_P14_0: 
        {
            config_struct->rx_port  = GPIO_PRT14;  	
            config_struct->rx_pin   = 0;      
            config_struct->rx_hsiom = P14_0_SCB2_UART_RX; 
            config_struct->uart_pclk= PCLK_SCB2_CLOCK;
            config_struct->uart_irqn= scb_2_interrupt_IRQn;
        }break;
        default: zf_assert(0);     break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       获取串口SCB模块号
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 返回参数       stc_SCB_t       模块结构体
// 使用示例       get_scb_module(UART_0);                   
// 备注信息       内部函数 用户无需关心
//-------------------------------------------------------------------------------------------------------------------
volatile stc_SCB_t* get_scb_module(uart_index_enum uart_n)
{
    volatile stc_SCB_t* temp_module;
    switch(uart_n)
    {
        case UART_0: temp_module = SCB0; break;
        case UART_1: temp_module = SCB4; break;
        case UART_2: temp_module = SCB5; break;
        case UART_3: temp_module = SCB3; break;
        case UART_4: temp_module = SCB2; break;
        default: zf_assert(0);     break;
    }
    return temp_module;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送写入
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       dat             需要发送的字节
// 返回参数       void
// 使用示例       uart_write_byte(UART_0, 0xA5);                    // 往串口1的发送缓冲区写入0xA5，写入后仍然会发送数据，但是会减少CPU在串口的执行时
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_byte (uart_index_enum uart_n, const uint8 dat)
{
    Cy_SCB_WriteTxFifo(scb_module[uart_n], dat);	
    while(Cy_SCB_IsTxComplete(scb_module[uart_n]) == 0);
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送数组
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *buff           要发送的数组地址
// 参数说明       len             发送长度
// 返回参数       void
// 使用示例       uart_write_buffer(UART_0, &a[0], 5);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_buffer (uart_index_enum uart_n, const uint8 *buff, uint32 len)
{
    while(len)
    {
        uart_write_byte(uart_n, *buff);
        len--;
        buff++;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送字符串
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *str            要发送的字符串地址
// 返回参数       void
// 使用示例       uart_write_string(UART_0, "seekfree");
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_write_string (uart_index_enum uart_n, const char *str)
{
    while(*str)
    {
        uart_write_byte(uart_n, *str++);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       读取串口接收的数据（whlie等待）
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *dat            接收数据的地址
// 返回参数       uint8           接收的数据
// 使用示例       uint8 dat = uart_read_byte(UART_0);             // 接收 UART_1 数据  存在在 dat 变量里
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_read_byte (uart_index_enum uart_n)
{
    uint8 read_uart_data;
    
    while(fifo_used(&uart_data_fifo_interior[uart_n]) == 0);
    
    fifo_read_element(&uart_data_fifo_interior[uart_n], &read_uart_data, FIFO_READ_AND_CLEAN); // 读取第一个数据
    
    return read_uart_data;	
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介       读取串口接收的数据（查询接收）
// 参数说明       uart_n          串口模块号 参照 zf_driver_uart.h 内 uart_index_enum 枚举体定义
// 参数说明       *dat            接收数据的地址
// 返回参数       uint8           1：接收成功   0：未接收到数据
// 使用示例       uint8 dat; uart_query_byte(UART_0, &dat);       // 接收 UART_1 数据  存在在 dat 变量里
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 uart_query_byte (uart_index_enum uart_n, uint8 *dat)
{
    uint8 return_data = 0;
    
    if(fifo_used(&uart_data_fifo_interior[uart_n]) > 0)
    {
        fifo_read_element(&uart_data_fifo_interior[uart_n], dat, FIFO_READ_AND_CLEAN); // 读取第一个数据
 
        return_data = 1;
    }
    else
    {
        return_data = 0;
    }
    return return_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口发送中断设置
// 参数说明       uart_n           串口模块号
// 参数说明       status          1：打开中断   0：关闭中断
// 返回参数       void
// 使用示例       uart_tx_interrupt(UART_0, 1);                   // 打开串口1发送中断
// 备注信息	      接收中断和发送中断只能选择一个，不能同时启用，最后配置的为有效
//-------------------------------------------------------------------------------------------------------------------
void uart_tx_interrupt (uart_index_enum uart_n, uint32 status)
{
    
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_TRIGGER);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_NOT_FULL);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_OVERFLOW);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_UNDERFLOW);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_EMPTY);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_NACK);
    Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_ARB_LOST);
    if(status)
    {
        Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) | CY_SCB_UART_TX_DONE);
    }
    else
    {
        Cy_SCB_SetTxInterruptMask(scb_module[uart_n], Cy_SCB_GetTxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_TX_DONE);
    }	
}
//-------------------------------------------------------------------------------------------------------------------
// 函数简介       串口接收中断设置
// 参数说明       uart_n           串口模块号
// 参数说明       status          1：打开中断   0：关闭中断
// 返回参数       void
// 使用示例       uart_rx_interrupt(UART_0, 1);                   // 打开串口1接收中断
// 备注信息	      接收中断和发送中断只能选择一个，不能同时启用，最后配置的为有效
//-------------------------------------------------------------------------------------------------------------------
void uart_rx_interrupt (uart_index_enum uart_n, uint32 status)
{
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_TRIGGER);
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_FULL);
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_OVERFLOW);
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_ERR_FRAME);
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_ERR_PARITY);
    Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_BREAK_DETECT);
    if(status)
    {
        Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) | CY_SCB_UART_RX_NOT_EMPTY);
    }
    else
    {
        Cy_SCB_SetRxInterruptMask(scb_module[uart_n], Cy_SCB_GetRxInterruptMask(scb_module[uart_n]) & ~CY_SCB_UART_RX_NOT_EMPTY);
    }	
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      SBUS串口初始化
//  参数说明      uartn           SBUS串口模块号(UART_0,UART_1,UART_2,UART_3)
//  参数说明      baud            SBUS串口波特率
//  参数说明      tx_pin          SBUS串口发送引脚
//  参数说明      rx_pin          SBUS串口接收引脚
//  返回参数      void
//  使用示例      uart_sbus_init(UART_4, 100000, UART4_TX_P14_1, UART4_RX_P14_0);       // 初始化串口4 波特率100000 发送引脚使用 14_1 接收引脚使用 P14_0
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_sbus_init (uart_index_enum uart_n, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    // 醒醒，串口号和端口都不对应怎么能初始化呢？
	
    zf_assert((uint8)uart_n == (uint8)tx_pin ? 1 : 0);
    zf_assert((uint8)uart_n == (uint8)rx_pin ? 1 : 0);
	
    uart_config_struct          uart_pin_config                 = {0};
    cy_stc_gpio_pin_config_t    gpio_pin_config                 = {0};
    cy_stc_scb_uart_config_t    g_stc_uart_config               = {0};
    uint64_t                    targetFreq                      = 8 * baud;
    uint64_t                    sourceFreq_fp5                  = ((uint64_t)UART_FREQ << 6ull);
    uint32_t                    divSetting_fp5                  = (uint32_t)(sourceFreq_fp5 / targetFreq);
    
    get_uart_config(&uart_pin_config, tx_pin, rx_pin);
    
    gpio_pin_config.driveMode           = CY_GPIO_DM_HIGHZ;
    gpio_pin_config.hsiom               = uart_pin_config.rx_hsiom;
    Cy_GPIO_Pin_Init(uart_pin_config.rx_port, uart_pin_config.rx_pin, &gpio_pin_config);
    
    gpio_pin_config.driveMode           = CY_GPIO_DM_STRONG_IN_OFF;
    gpio_pin_config.hsiom               = uart_pin_config.tx_hsiom;
    Cy_GPIO_Pin_Init(uart_pin_config.tx_port, uart_pin_config.tx_pin, &gpio_pin_config);

    
    g_stc_uart_config.uartMode          = CY_SCB_UART_STANDARD;
    g_stc_uart_config.oversample        = 8;
    g_stc_uart_config.dataWidth         = 9;
    g_stc_uart_config.stopBits          = CY_SCB_UART_STOP_BITS_1;
    g_stc_uart_config.parity            = CY_SCB_UART_PARITY_EVEN;
    g_stc_uart_config.ctsPolarity       = CY_SCB_UART_ACTIVE_LOW;  
    g_stc_uart_config.rtsPolarity       = CY_SCB_UART_ACTIVE_LOW;
   
    Cy_SCB_UART_DeInit(scb_module[uart_n]);
    Cy_SCB_UART_Init(scb_module[uart_n], &g_stc_uart_config, &uart_context[uart_n]);
    Cy_SCB_UART_Enable(scb_module[uart_n]);  
    
    Cy_SysClk_PeriphAssignDivider(uart_pin_config.uart_pclk, CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n);
    Cy_SysClk_PeriphSetFracDivider(CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n, ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul), (divSetting_fp5 & 0x0000001Ful));
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n);
    
    cy_stc_sysint_irq_t                 stc_sysint_irq_cfg_uart;
    stc_sysint_irq_cfg_uart.sysIntSrc = uart_pin_config.uart_irqn;
    stc_sysint_irq_cfg_uart.intIdx    = UART_USE_ISR;
    stc_sysint_irq_cfg_uart.isEnabled = true;
    Cy_SysInt_InitIRQ(&stc_sysint_irq_cfg_uart);
    Cy_SysInt_SetSystemIrqVector(stc_sysint_irq_cfg_uart.sysIntSrc, (cy_systemIntr_Handler)uart_isr_func[uart_n]);
    NVIC_EnableIRQ(stc_sysint_irq_cfg_uart.intIdx);
    
    uart_rx_interrupt(uart_n, 0);
    uart_tx_interrupt(uart_n, 0);
}

//-------------------------------------------------------------------------------------------------------------------
//  函数简介      串口初始化
//  参数说明      uartn           串口模块号(UART_0,UART_1,UART_2,UART_3)
//  参数说明      baud            串口波特率
//  参数说明      tx_pin          串口发送引脚
//  参数说明      rx_pin          串口接收引脚
//  返回参数      void
//  使用示例      uart_init(UART_0, 115200, UART0_TX_P00_1, UART0_RX_P00_0);       // 初始化串口0 波特率115200 发送引脚使用 P0_1 接收引脚使用 P0_0
//  备注信息
//-------------------------------------------------------------------------------------------------------------------
void uart_init (uart_index_enum uart_n, uint32 baud, uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin)
{
    // 醒醒，串口号和端口都不对应怎么能初始化呢？
	
    zf_assert((uint8)uart_n == (uint8)tx_pin ? 1 : 0);
    zf_assert((uint8)uart_n == (uint8)rx_pin ? 1 : 0);
    
    uart_config_struct          uart_pin_config                 = {0};
    cy_stc_gpio_pin_config_t    gpio_pin_config                 = {0};
    cy_stc_scb_uart_config_t    g_stc_uart_config               = {0};
    uint64_t                    targetFreq                      = 8 * baud;
    uint64_t                    sourceFreq_fp5                  = ((uint64_t)UART_FREQ << 6ull);
    uint32_t                    divSetting_fp5                  = (uint32_t)(sourceFreq_fp5 / targetFreq);
    
    get_uart_config(&uart_pin_config, tx_pin, rx_pin);
    
    gpio_pin_config.driveMode           = CY_GPIO_DM_HIGHZ;
    gpio_pin_config.hsiom               = uart_pin_config.rx_hsiom;
    Cy_GPIO_Pin_Init(uart_pin_config.rx_port, uart_pin_config.rx_pin, &gpio_pin_config);
    Cy_GPIO_Set(uart_pin_config.rx_port, uart_pin_config.rx_pin);
    
    gpio_pin_config.driveMode           = CY_GPIO_DM_STRONG_IN_OFF;
    gpio_pin_config.hsiom               = uart_pin_config.tx_hsiom;
    Cy_GPIO_Pin_Init(uart_pin_config.tx_port, uart_pin_config.tx_pin, &gpio_pin_config);

    
    g_stc_uart_config.uartMode          = CY_SCB_UART_STANDARD;
    g_stc_uart_config.oversample        = 8;
    g_stc_uart_config.dataWidth         = 8;
    g_stc_uart_config.stopBits          = CY_SCB_UART_STOP_BITS_1;
    g_stc_uart_config.parity            = CY_SCB_UART_PARITY_NONE;
    g_stc_uart_config.ctsPolarity       = CY_SCB_UART_ACTIVE_LOW;  
    g_stc_uart_config.rtsPolarity       = CY_SCB_UART_ACTIVE_LOW;
    
    Cy_SCB_UART_DeInit(scb_module[uart_n]);
    Cy_SCB_UART_Init(scb_module[uart_n], &g_stc_uart_config, &uart_context[uart_n]);
    Cy_SCB_UART_Enable(scb_module[uart_n]);  
    
    Cy_SysClk_PeriphAssignDivider(uart_pin_config.uart_pclk, CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n);
    Cy_SysClk_PeriphSetFracDivider(CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n, ((divSetting_fp5 & 0x1FFFFFE0ul) >> 5ul), (divSetting_fp5 & 0x0000001Ful));
    Cy_SysClk_PeriphEnableDivider(CY_SYSCLK_DIV_24_5_BIT, (uint8)uart_n);
    
    uart_rx_interrupt(uart_n, 0);
    uart_tx_interrupt(uart_n, 0);
    
    cy_stc_sysint_irq_t                 uart_irq_cfg;
    uart_irq_cfg.sysIntSrc = uart_pin_config.uart_irqn;
    uart_irq_cfg.intIdx    = UART_USE_ISR;
    uart_irq_cfg.isEnabled = true;
    interrupt_init(&uart_irq_cfg, uart_isr_func[uart_n], 0);
    
    fifo_init(&uart_data_fifo_interior[uart_n], FIFO_DATA_8BIT, &uart_data_buffer_interior[uart_n][0], 64);  
}










