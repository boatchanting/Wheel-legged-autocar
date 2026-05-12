/***************************************************************************//**
* \file cy_semihosting.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Semihosting middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_semihosting.h"

#if defined(__cplusplus)
extern "C" {
#endif
  
#define CY_SEMIHOSTING_DISABLED (1)
  
#ifdef CY_SEMIHOSTING_DISABLED

typedef char dummy_t; // prevent compiler warning because of empty file

#else


// Currently 2 flavors of BSP header file defines exist, convert them to the same type for use by the code
#if defined CY_BSP_USB_UART_TYPE
    #define CY_USB_SCB_UART_TYPE        CY_BSP_USB_UART_TYPE                
    #define CY_USB_SCB_UART_RX_PORT     CY_BSP_USB_UART_RX_PORT
    #define CY_USB_SCB_UART_RX_PIN      CY_BSP_USB_UART_RX_PIN
    #define CY_USB_SCB_UART_RX_PIN_MUX  CY_BSP_USB_UART_RX_PIN_MUX
    #define CY_USB_SCB_UART_TX_PORT     CY_BSP_USB_UART_TX_PORT
    #define CY_USB_SCB_UART_TX_PIN      CY_BSP_USB_UART_TX_PIN
    #define CY_USB_SCB_UART_TX_PIN_MUX  CY_BSP_USB_UART_TX_PIN_MUX
    #define CY_USB_SCB_UART_PCLK        CY_BSP_USB_UART_PCLK
    #define CY_USB_SCB_UART_IRQN        CY_BSP_USB_UART_IRQN
#endif


/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/
#define SEMIHOSTING_SCB_OVERSAMPLING_FACTOR        8

#define E_UART_RX_INTR_FACTER     (                              \
                                 CY_SCB_UART_RX_TRIGGER      |   \
                               /*CY_SCB_UART_RX_NOT_EMPTY    | */\
                               /*CY_SCB_UART_RX_FULL         | */\
                                 CY_SCB_UART_RX_OVERFLOW     |   \
                                 CY_SCB_UART_RX_UNDERFLOW    |   \
                                 CY_SCB_UART_RX_ERR_FRAME    |   \
                                 CY_SCB_UART_RX_ERR_PARITY   |   \
                                 CY_SCB_UART_RX_BREAK_DETECT |   \
                                 0                               \
                                )
#define E_UART_TX_INTR_FACTER     (                              \
                                 CY_SCB_UART_TX_TRIGGER      |   \
                               /*CY_SCB_UART_TX_NOT_FULL     | */\
                               /*CY_SCB_UART_TX_EMPTY        | */\
                                 CY_SCB_UART_TX_OVERFLOW     |   \
                               /*CY_SCB_UART_TX_UNDERFLOW    | */\
                                 CY_SCB_UART_TX_DONE         |   \
                               /*CY_SCB_UART_TX_NACK         | */\
                               /*CY_SCB_UART_TX_ARB_LOST     | */\
                                 0                               \
                                )

/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/

/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/

/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/

static cy_semihosting_wdg_handle_callout_t  m_pfnWatchdogCallout = NULL;

static volatile stc_SCB_t* m_pScbInstance = NULL;

static const cy_stc_scb_uart_config_t    m_stcScbConfig = 
{
   .uartMode                   = CY_SCB_UART_STANDARD,
   .oversample                 = SEMIHOSTING_SCB_OVERSAMPLING_FACTOR,
   .dataWidth                  = 8,
   .enableMsbFirst             = false,
   .stopBits                   = CY_SCB_UART_STOP_BITS_1,
   .parity                     = CY_SCB_UART_PARITY_NONE,
   .enableInputFilter          = false,
   .dropOnParityError          = false,
   .dropOnFrameError           = false,
   .enableMutliProcessorMode   = false,
   .receiverAddress            = 0,
   .receiverAddressMask        = 0,
   .acceptAddrInFifo           = false,
   .irdaInvertRx               = false,
   .irdaEnableLowPowerReceiver = false,
   .smartCardRetryOnNack       = false,
   .enableCts                  = false,
   .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
   .rtsRxFifoLevel             = 0,
   .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
   .breakWidth                 = 0,
   .rxFifoTriggerLevel         = 0,
   .rxFifoIntEnableMask        = E_UART_RX_INTR_FACTER,
   .txFifoTriggerLevel         = 0,
   .txFifoIntEnableMask        = 0
};


/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/

/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/

/*******************************************************************************
* Function Name: Cy_Semihosting_Init
****************************************************************************//**
*
* \brief  Initializes the semihosting module.
*
* \param  pstcUsart          : The pointer to an SCB structure instance.
* \param  u32Bitrate         : Baudrate in Bit/s   <<< unused by this API! >>>
* \param  pfnWatchdogCallout : Optional watchdog handler or NULL. Called during certain loops
* \param  bIsDummyInit       : If one SCB channel is shared across cores, the non-
*                              first cores should set this parameter to 'true' when 
*                              calling this function to skip certain steps
*
* \return None
*
*******************************************************************************/
cy_semihosting_en_result_t Cy_Semihosting_Init(volatile stc_SCB_t *pScbInstance, uint32_t u32Bitrate, cy_semihosting_wdg_handle_callout_t pfnWatchdogCallout, bool bIsDummyInit)
{
    cy_semihosting_en_result_t enSemihostingResult = CY_SEMIHOSTING_SUCCESS;
    cy_en_scb_uart_status_t    enScbResult = CY_SCB_UART_SUCCESS;
    
    if (pScbInstance == NULL)
    {
        return CY_SEMIHOSTING_BAD_PARAM;
    }

    if(bIsDummyInit == false)
    {
        enScbResult = Cy_SCB_UART_Init(pScbInstance, &m_stcScbConfig, NULL); // Context not needed because UART low-level APIs are used
        
        if (enScbResult == CY_SCB_UART_SUCCESS)
        {
            Cy_SCB_UART_Enable(pScbInstance);
        }
    }
    
    if (enScbResult == CY_SCB_UART_SUCCESS)
    {
        m_pScbInstance = pScbInstance;
        m_pfnWatchdogCallout = pfnWatchdogCallout;
    }
    else
    {
        m_pScbInstance = NULL;
        enSemihostingResult = CY_SEMIHOSTING_ERROR;
    }

    return enSemihostingResult;
}


/*******************************************************************************
* Function Name: Cy_Semihosting_InitAll
****************************************************************************//**
*
* \brief  Initializes the semihosting module.
*
* \param  pstcUsart          : The pointer to an SCB structure instance.
* \param  u32Bitrate         : Baudrate in Bit/s
* \param  pfnWatchdogCallout : Optional watchdog handler or NULL. Called during certain loops
* \param  bIsDummyInit       : If one SCB channel is shared across cores, the non-
*                              first cores should set this parameter to 'true' when 
*                              calling this function to skip certain steps
*
* \return None
*
*******************************************************************************/
cy_semihosting_en_result_t Cy_Semihosting_InitAll(volatile stc_SCB_t *pScbInstance, uint32_t u32Bitrate, cy_semihosting_wdg_handle_callout_t pfnWatchdogCallout, bool bIsDummyInit)
{
    
#ifdef CY_USB_SCB_UART_TYPE

    cy_semihosting_en_result_t enSemihostingResult = CY_SEMIHOSTING_SUCCESS;
    
    // This function only supports the SCB instance which is defined in the BSP header because it needs the corresponding pin and PCLK definitions
    if (pScbInstance != CY_USB_SCB_UART_TYPE)
    {
        return CY_SEMIHOSTING_BAD_PARAM;
    }
    
    if(bIsDummyInit == false)
    {
        uint32_t scbInputFreqHz = u32Bitrate * SEMIHOSTING_SCB_OVERSAMPLING_FACTOR;
        
      #if defined(CY_SEMIHOSTING_PCLK_DIV_TYPE) && defined(CY_SEMIHOSTING_PCLK_DIV_NUM)
        cy_en_divider_types_t divType = CY_SEMIHOSTING_PCLK_DIV_TYPE;
        uint8_t               divNum  = CY_SEMIHOSTING_PCLK_DIV_NUM;        
      #else
        cy_en_divider_types_t divType = CY_SYSCLK_DIV_24_5_BIT;
        #ifdef PERI_PERI_PCLK_PCLK_GROUP_NR // Device has multiple PCLK groups
        uint8_t               divNum  = PERI_PERI_PCLK_PCLK_GROUP_NR1_GR_DIV_24_5_VECT - 1; // Use last 24.5 fractional divider of the COMM PCLK group
        #else // Device has only a single PCLK group
        uint8_t               divNum  = PERI_DIV_24_5_NR - 1; // Use last 24.5 fractional divider
        #endif
      #endif
        
        Cy_SysClk_PeriphAssignDivider(CY_USB_SCB_UART_PCLK, divType, divNum);
        
      #ifdef PERI_PERI_PCLK_PCLK_GROUP_NR // Device has multiple PCLK groups
        Cy_SysClk_PeriphSetFracDividerAuto(Cy_SysClk_GetClockGroup(CY_USB_SCB_UART_PCLK), divType, divNum, scbInputFreqHz, NULL);
        Cy_SysClk_PeriphEnableDivider     (Cy_SysClk_GetClockGroup(CY_USB_SCB_UART_PCLK), divType, divNum);
      #else // Device has only a single PCLK group
        Cy_SysClk_PeriphSetFracDividerAuto(divType, divNum, scbInputFreqHz, NULL);
        Cy_SysClk_PeriphEnableDivider     (divType, divNum);
      #endif
        
        Cy_GPIO_Pin_FastInit(CY_USB_SCB_UART_TX_PORT, CY_USB_SCB_UART_TX_PIN, CY_GPIO_DM_STRONG, 0, CY_USB_SCB_UART_TX_PIN_MUX);
        Cy_GPIO_Pin_FastInit(CY_USB_SCB_UART_RX_PORT, CY_USB_SCB_UART_RX_PIN, CY_GPIO_DM_HIGHZ,  0, CY_USB_SCB_UART_RX_PIN_MUX);
    
    }
    
    enSemihostingResult = Cy_Semihosting_Init(pScbInstance, u32Bitrate, pfnWatchdogCallout, bIsDummyInit);
        
    return enSemihostingResult;
    
#else    
    
    // UART related defines from BSP header are needed, but they do not seem to be defined.
    // Maybe because device part number has been selected for which no definitions are available in BSP header.
    
    return CY_SEMIHOSTING_ERROR;
    
#endif
}



// Remap low-level IO function names to toolchain specific expected names
#if defined(__ghs__) // GreenHills
    #define _write  write
    #define _read   read
#elif defined(__ICCARM__) // IAR
    #define _write  __write
    #define _read   __read
    int __write(int fd, const void *buffer, unsigned int count);
    int __read(int fd, void *buffer, unsigned int count);
#elif defined(__DCC__) // Wind River DIAB
    #define _write  __write
    #define _read   __read
#endif


/*******************************************************************************
* Function Name: _write / write / __write
****************************************************************************//**
*
* \brief  Low level write function, called by file I/O functions
*
* \param fd       File descriptor (0 = stdin, 1 = stdout, 2 = stderr) - ignored
* \param buffer   Buffer with data to be written
* \param count    Number of bytes to be written
*
* \return Number of written bytes
*
*******************************************************************************/
int _write(int fd, const void *buffer, unsigned int count)
{
    // If no SCB instance available, return error
    if (NULL == m_pScbInstance)
    {
        return -1;
    }
    
    uint8_t * pu8BufStart = (uint8_t *) buffer;
    uint32_t u32TotalLength = count;
    uint32_t u32BufIndex = 0;
    uint32_t u32CurrentLength = 0;
    
    do
    {
        u32CurrentLength = Cy_SCB_GetFifoSize(m_pScbInstance) - Cy_SCB_GetNumInTxFifo(m_pScbInstance);    
        if((u32BufIndex + u32CurrentLength) > u32TotalLength)
        {
            u32CurrentLength = u32TotalLength - u32BufIndex;
        }
        if(u32CurrentLength > 0)
        {
            Cy_SCB_UART_PutArray(m_pScbInstance, &(pu8BufStart[u32BufIndex]), u32CurrentLength);
        }
        u32BufIndex += u32CurrentLength;

        // Call watchdog handle function if configured
        if (m_pfnWatchdogCallout != NULL)
        {
            m_pfnWatchdogCallout();
        }
        
    } while(u32BufIndex < u32TotalLength);
    
    // Wait until FIFO and shift register are empty, to have a real synchronous printf behavior
    while(Cy_SCB_UART_IsTxComplete(m_pScbInstance) == false)
    {
    }

    // Return number of written bytes
    return count;
}


/*******************************************************************************
* Function Name: _read / read / __read
****************************************************************************//**
*
* \brief  Low level read function, called by file I/O functions
*
* \param fd       File descriptor (0 = stdin, 1 = stdout, 2 = stderr) - ignored
* \param buffer   Buffer to receive read data
* \param count    Number of bytes to read
*
* \return Number of read bytes
*
*******************************************************************************/
int _read(int fd, void *buffer, unsigned int count)
{
    uint32_t u32ActualReadCount;
    
    // If no SCB instance available, return error
    if (NULL == m_pScbInstance)
    {
        return -1;
    }

    // Block until at least one byte has been received
    // Note: stdin must never return EOF (i.e. return 0), otherwise no further reads are possible    
    do
    {
        u32ActualReadCount = Cy_SCB_UART_GetArray(m_pScbInstance, buffer, count);
    } while (u32ActualReadCount == 0);

    return (int) u32ActualReadCount;
}


#endif // CY_SEMIHOSTING_DISABLED


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
