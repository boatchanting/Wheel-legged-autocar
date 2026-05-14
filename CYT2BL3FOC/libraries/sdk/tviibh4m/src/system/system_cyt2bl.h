/***************************************************************************//**
* \file system_cyt2bl.h
* \version 1.0
*
* \brief Device system header file.
*
********************************************************************************
* \copyright
* Copyright 2018-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

 /**
* \defgroup group_system_cfg System Configuration (System)
* \{
* This driver provides global system_cfg defines and API function definitions.
*
* \defgroup group_system_cfg_macros Macro
* \defgroup group_system_cfg_functions Functions
* \defgroup group_system_cfg_data_structures Data structures
* \defgroup group_system_cfg_enums Enumerated types
*/

#ifndef SYSTEM_TVIIBE4M_H
#define SYSTEM_TVIIBE4M_H

#include "zf_common_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**  Start address of the Cortex-M4 application */
#ifndef CY_CORTEX_M4_APPL_ADDR
    // This symbol is defined in the linker script
    extern char *  __cm4_vector_base_linker_symbol;
    #define CY_CORTEX_M4_APPL_ADDR      ((uint32_t) &__cm4_vector_base_linker_symbol)
#endif

// Macro to provide a delay option
#define DELAY(loop_till)     do { for( volatile uint32_t loop_cnt = 0; loop_cnt < loop_till; loop_cnt++); } while(0)

// This cycle value should be less than 16777216 (0x01000000, 24bits)
#define DELAY_CORE_CYCLE(cycle) {\
    SysTick->CTRL = 0;\
    SysTick->LOAD = cycle;\
    SysTick->VAL  = 0;\
    SysTick->CTRL = 5;\
    while((SysTick->CTRL & 0x00010000) == 0);\
    SysTick->CTRL = 0;\
}\


/**
* \addtogroup group_system_cfg_macros
* \{
*/
#define CY_SYS_CM_STATUS_OFF                    (0u)    /**< The Cortex-M core is off. */
#define CY_SYS_CM_STATUS_RESET                  (1u)    /**< The Cortex-M core is in reset. */
#define CY_SYS_CM_STATUS_RETAINED               (2u)    /**< The Cortex-M core is retained. */
#define CY_SYS_CM_STATUS_ENABLED                (3u)    /**< The Cortex-M core is enabled */



/*******************************************************************************
* System Clock Initialization
*******************************************************************************/
#if (CY_USE_PSVP == 1)
    #define CY_INITIAL_TARGET_FAST_FREQ         ( 24000000UL)

    #define CY_INITIAL_TARGET_PERI_FREQ         ( 24000000UL)

    #define CY_INITIAL_TARGET_SLOW_FREQ         ( 24000000UL)

    #define CY_CLK_ECO_FREQ_HZ                  ( 10000000UL)

    #define CY_CLK_IMO_FREQ_HZ                  ( 24000000UL)

    /** WCO frequency in Hz */
    #define CY_CLK_WCO_FREQ_HZ                  (    32900UL)

#else
    #define CY_INITIAL_TARGET_FAST_FREQ         (160000000UL)

    #define CY_INITIAL_TARGET_PERI_FREQ         ( 80000000UL)

    #define CY_INITIAL_TARGET_SLOW_FREQ         ( 80000000UL)

    #define CY_CLK_ECO_FREQ_HZ                  ( BOARD_XTAL_FREQ)

    #define CY_CLK_IMO_FREQ_HZ                  (  8000000UL)

    /** WCO frequency in Hz */
    #define CY_CLK_WCO_FREQ_HZ                  (    32768UL)
#endif

/** HVILO0 frequency in Hz */
#define CY_CLK_HVILO0_FREQ_HZ                   (    32768UL)

/** HVILO1 frequency in Hz */
#define CY_CLK_HVILO1_FREQ_HZ                   (    32768UL)

#define CY_CLK_EXT_FREQ_HZ                      (        0UL) //TODO

/* Do not use these definitions directly in your application */
#define CY_DELAY_MS_OVERFLOW_THRESHOLD          (0x8000u)
#define CY_DELAY_1K_THRESHOLD                   (1000u)
#define CY_DELAY_1K_MINUS_1_THRESHOLD           (CY_DELAY_1K_THRESHOLD - 1u)
#define CY_DELAY_1M_THRESHOLD                   (1000000u)
#define CY_DELAY_1M_MINUS_1_THRESHOLD           (CY_DELAY_1M_THRESHOLD - 1u)

    
/************************User Configurable Macro Definitions*******************/

// Macro to select the Clock Source
#define CY_SYSTEM_USE_IMO                       0
#define CY_SYSTEM_USE_EXT                       1
#define CY_SYSTEM_USE_ECO                       2

#define CY_SYSTEM_USE_CLOCK                     CY_SYSTEM_USE_ECO

// Definition whether WCO enabled or not
#define CY_SYSTEM_WCO_DISABLE                      // This would be ignored if CY_SYSTEM_USE_CLOCK == CY_SYSTEM_USE_LPECO

// Definition of this macro disables the WDT (Enabled in SROM)
#define CY_SYSTEM_WDT_DISABLE

/** \} group_system_cfg_macros */

/**************************Definition of global variables**********************/

/** \addtogroup group_system_cfg_data_structures
* \{
*/
  
extern uint32_t SystemCoreClock;
extern uint32_t cy_delayFreqHz;
extern uint32_t cy_delayFreqKhz;
extern uint32_t cy_delayFreqMhz;
extern uint32_t cy_delay32kMs;

/** \} group_system_cfg_data_structures */

/******************************Definition of global functions******************/

/**
* \addtogroup group_system_cfg_functions
* \{
*/

/**
  \brief Setup the microcontroller system.

   Initialize the System and update the SystemCoreClock variable.
 */
extern void     SystemInit (void);

extern uint32_t Cy_SaveIRQ(void);
extern void     Cy_RestoreIRQ(uint32_t saved);

/**
  \brief  Update SystemCoreClock variable.

   Updates the SystemCoreClock with current core Clock retrieved from cpu registers.
 */
extern void SystemCoreClockUpdate (void);

extern void Cy_SystemInitFpuEnable(void);

extern uint32_t Cy_SysGetApplCoreStatus(void);
extern void     Cy_SysEnableApplCore(uint32_t vectorTableOffset);
extern void     Cy_SysDisableApplCore(void);
extern void     Cy_SysRetainApplCore(void);
extern void     Cy_SysResetApplCore(void);

/** \} group_system_cfg_functions */

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_TVIIBE4M_H */

