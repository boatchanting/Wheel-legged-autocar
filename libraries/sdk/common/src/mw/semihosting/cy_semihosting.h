/***************************************************************************//**
* \file cy_semihosting.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Semihosting middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_semihosting Middleware UART based Debugging (MW Semihosting)
* \{
* The Semihosting middleware provides an API to setup 
* Low level access routines for file I/O routed to a selected SCB instance which
* is operated in UART mode, 8N1 with the chosen bitrate.
*
* Using this module allows to use standard I/O functions like printf()
* and scanf(). 
*
* The SCB instance and bitrate to be used must be set by calling
* Semihosting_Init(). 
*
* SCB port pin and clock configuration is not in the scope of the Semihosting driver
*
* \defgroup group_mw_semihosting_macros Macro
* \defgroup group_mw_semihosting_functions Functions
* \defgroup group_mw_semihosting_data_structures Data structures
* \defgroup group_mw_semihosting_enums Enumerated types
*/


#if !defined(CY_SEMIHOSTING_H)
#define CY_SEMIHOSTING_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if defined(__cplusplus)
extern "C" {
#endif



/*****************************************************************************/
// Define CY_SEMIHOSTING_DISABLED here or via compiler command line to disable
// semihosting via SCB. In case of GreenHills semihosting via debugger is
// activated then.

// #define CY_SEMIHOSTING_DISABLED



/**
* Peripheral clock divider type used for SCB clock init in Cy_Semihosting_InitAll
* If commented, CY_SYSCLK_DIV_24_5_BIT will be used. If uncommented, CY_SEMIHOSTING_PCLK_DIV_NUM must be uncommented as well!
* Note: Since this is a common driver, the chosen CY_SEMIHOSTING_PCLK_DIV_TYPE may not be available on all TVII derivates.
*/
// #define CY_SEMIHOSTING_PCLK_DIV_TYPE         CY_SYSCLK_DIV_24_5_BIT  // choose from cy_en_divider_types_t in cy_sysclk driver, must be a fractional divider!

/**
* Peripheral clock divider instance of type CY_SEMIHOSTING_PCLK_DIV_TYPE used for SCB clock init in Cy_Semihosting_InitAll
* If commented, the last CY_SYSCLK_DIV_24_5_BIT will be used. If uncommented, CY_SEMIHOSTING_PCLK_DIV_TYPE must be uncommented as well
* Note: Since this is a common driver, the chosen CY_SEMIHOSTING_PCLK_DIV_NUM may not be available on all TVII derivates.
*/
// #define CY_SEMIHOSTING_PCLK_DIV_NUM          0


/***************************************
*       Enumeration
***************************************/

/**
* \addtogroup group_mw_semihosting_enums
* \{
*/

/**
* Function return values
*/
typedef enum
{
    CY_SEMIHOSTING_SUCCESS   = 0x00u, /**< Returned successful */
    CY_SEMIHOSTING_ERROR     = 0x01u, /**< General error */
    CY_SEMIHOSTING_BAD_PARAM = 0x02u, /**< Bad parameter was passed */
} cy_semihosting_en_result_t;

/** \} group_mw_semihosting_enums */

/**
* \addtogroup group_mw_semihosting_data_structures
* \{
*/

/** Function pointer type to void/void function that will handle the watchdog during write/read execution*/
typedef void (*cy_semihosting_wdg_handle_callout_t)(void);

/** \} group_mw_semihosting_data_structures */

#ifdef CY_SEMIHOSTING_DISABLED

#define Cy_Semihosting_Init(usart,bitrate,wdg_handle,dummy_init)          ((cy_semihosting_en_result_t) 0)  // Ok
#define Cy_Semihosting_InitAll(usart,bitrate,wdg_handle,dummy_init)       ((cy_semihosting_en_result_t) 0)  // Ok

#else

    
    


/***************************************
*       Function Prototypes
***************************************/

/**
* \addtogroup group_mw_semihosting_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_Semihosting_Init
****************************************************************************//**
*
* \brief  Initializes the Semihosting
*
* \param  pstcUsart          : The pointer to an SCB structure instance.
* \param  u32Bitrate         : Baudrate in Bit/s    <<< unused by this API! >>>
* \param  pfnWatchdogCallout : Optional watchdog handler or NULL. Called during certain loops
* \param  bIsDummyInit       : If one SCB channel is shared across cores, the non-
*                              first cores should set this parameter to 'true' when 
*                              calling this function to skip certain steps

*
* \return None
*
*******************************************************************************/
    
cy_semihosting_en_result_t Cy_Semihosting_Init(volatile stc_SCB_t *pScbInstance, uint32_t u32Bitrate, cy_semihosting_wdg_handle_callout_t pfnWatchdogCallout, bool bIsDummyInit);


/*******************************************************************************
* Function Name: Cy_Semihosting_InitAll
****************************************************************************//**
*
* \brief  Initializes the Semihosting and required port pins and peripheral clock
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
    
cy_semihosting_en_result_t Cy_Semihosting_InitAll(volatile stc_SCB_t *pScbInstance, uint32_t u32Bitrate, cy_semihosting_wdg_handle_callout_t pfnWatchdogCallout, bool bIsDummyInit);

#endif //CY_SEMIHOSTING_DISABLED



/** \} group_mw_semihosting_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_SEMIHOSTING_H */

/** \} group_semihosting */

/* [] END OF FILE */


