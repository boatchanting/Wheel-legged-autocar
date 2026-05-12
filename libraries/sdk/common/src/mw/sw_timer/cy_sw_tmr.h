/***************************************************************************//**
* \file cy_sw_tmr.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Software Timer (SwTmr) middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_swtmr Middleware Software Timer (MW SwTmr)
* \{
* Offers CY_SWTMR_MAX_TIMERS timers with a 1 ms resolution
*
* After Cy_SwTmr_Init is called the application can start SW timers by
* calling Cy_SwTmr_Start. The timer ID which must be used to identify the
* SW timer is in the range of 0 to (CY_SWTMR_MAX_TIMERS - 1).
*
* The module takes exclusive ownership of the Cortex-M SysTick and Cy_SysTick
* driver. The callback from SysTick ISR will handle all enabled SW timers.
*
* The function Cy_SwTmr_IsElapsed can be used to check, whether a SW timer
* is elapsed or not. Use Cy_SwTmr_Remaining to get the remaining time until
* a timer elapses. Cy_SwTmr_Wait can be used for synchronously wait for
* a desired duration, including hardware watchdog clearing (if desired).
*
* The timer module also supports callback functions to prevent polling
* the timer status. To prevent long interrupt duration due to multiple
* callback functions, the function Cy_SwTmr_Main must be called cyclically
* within the application main loop. The callback functions of timers that
* have been started by Cy_SwTmr_Start are called from the Cy_SwTmr_Main
* function (in task level, outside interrupt handler).
* Timers that have been started by Cy_SwTmr_StartHighPrio will be called
* directly from the interrupt context.
* 
* \defgroup group_mw_swtmr_macros Macro
* \defgroup group_mw_swtmr_functions Functions
* \defgroup group_mw_swtmr_data_structures Data structures
* \defgroup group_mw_swtmr_enums Enumerated types
*/


#if !defined(CY_SWTMR_H)
#define CY_SWTMR_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mw_swtmr_macros
* \{
*/

#define CY_SWTMR_MAX_TIMERS       5  // must be <= 256


/** Define base time tick interval in microseconds. Default value is 1000 us = 1 ms.
 ** This value will determine the counting resolution of the tick count returned
 ** by Cy_SwTmr_GetTickCountUs(). It will also influence the interrupt load so handle
 ** with care!
 ** Min/Max values: 20 <= CY_SWTMR_TICK_INTERVAL_US <= 1000 */
#define CY_SWTMR_TICK_INTERVAL_US          1000

/** \} group_mw_swtmr_macros */

/**
* \addtogroup group_mw_swtmr_data_structures
* \{
*/

/** Function pointer type to void/void function for SW Timer Callbacks*/
typedef void (*cy_sw_tmr_callback_t)(void);

/** Function pointer type to void/void function that will handle the watchdog during Cy_SwTmr_Wait calls*/
typedef void (*cy_sw_tmr_wdg_handle_callout_t)(void);

/** \} group_mw_swtmr_data_structures */

/**
* \addtogroup group_mw_swtmr_enums
* \{
*/

typedef enum
{
    CY_SWTMR_SUCCESS   = 0x00u, /**< Returned successful */
    CY_SWTMR_BAD_PARAM = 0x01u, /**< Bad parameter was passed */
} cy_swtmr_en_result_t;

/** \} group_mw_swtmr_enums */

/***************************************
*       Function Prototypes
***************************************/

/**
* \addtogroup group_mw_swtmr_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SwTmr_Init
****************************************************************************//**
*
* \brief  Initializes the SW Timer
*
* \param  pstcSwTmrCfg : The pointer to the init configuration struct
*
* \return None
*
*******************************************************************************/
void Cy_SwTmr_Init(void);

/*******************************************************************************
* Function Name: Cy_SwTmr_Disable
****************************************************************************//**
*
* \brief  Disables the SW Timer
*
* \param  None
*
* \return None
*
*******************************************************************************/
void Cy_SwTmr_Disable(void);

cy_swtmr_en_result_t Cy_SwTmr_Start(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback);
cy_swtmr_en_result_t Cy_SwTmr_StartHighPrio(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback);
cy_swtmr_en_result_t Cy_SwTmr_Stop(uint8_t u8TimerID);
int32_t Cy_SwTmr_Remaining(uint8_t u8TimerID);
cy_swtmr_en_result_t Cy_SwTmr_Wait(uint8_t u8TimerID, int32_t i32TimeMs, cy_sw_tmr_wdg_handle_callout_t pfnWatchdogCallout);
void Cy_SwTmr_Main(void);
bool Cy_SwTmr_IsElapsed(uint8_t u8TimerID, bool bClearElapsedFlag);
uint64_t Cy_SwTmr_GetTickCountUs(void);
bool Cy_SwTmr_IsTimeSpanElapsed(uint64_t u64TickCountUs, uint32_t u32TimeSpanUs);


/** \} group_mw_swtmr_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_SWTMR_H */

/** \} group_swtmr */

/* [] END OF FILE */
