/***************************************************************************//**
* \file cy_mcwdt.h
* \version 1.0
*
* Provides an API declaration of the Cypress PDL 3.0 MCWDT driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mcwdt Multi-Counter Watchdog (MCWDT)
* \{
* MCWDT has two 16-bit counters and one 32-bit counter. 
* You can use this driver to create a free-running
* timer or generate periodic interrupts. The driver also
* includes support for the watchdog function to recover from CPU or
* firmware failures.
*
* There are two primary use cases for MCWDT: generating periodic CPU interrupts;
* and implementing a free-running timer. Both have many applications in
* embedded systems:
* * Measuring time between events
* * Generating periodic events
* * Synchronizing actions
* * Real-time clocking
* * Polling
*
* An additional use case is to implement a watchdog used for recovering from a CPU or
* firmware failure.
* 
* \section group_mcwdt_configuration Configuration Considerations
*
* Each MCWDT may be configured for a particular product. 
* One MCWDT block can be associated with only one CPU during runtime. 
* A single MCWDT is not intended to be used by multiple CPUs simultaneously. 
* Each block contains three sub-counters, each of which can be configured for 
* various system utility functions - free running counter, periodic interrupts, 
* watchdog reset, or three interrupts followed by a watchdog reset.
* All counters are clocked by either LFCLK (nominal 32 kHz) or by a cascaded 
* counter.
* A simplified diagram of the MCWDT hardware is shown below:
* \image html mcwdt.png
* The frequency of the periodic interrupts can be configured using the Match 
* value with combining Clear on match option, which can be set individually 
* for each counter using Cy_MCWDT_SetClearOnMatch(). When the Clear on match option 
* is not set, the periodic interrupts of the C0 and C1 16-bit sub-counters occur 
* after 65535 counts and the match value defines the shift between interrupts 
* (see the figure below). The enabled Clear on match option 
* resets the counter when the interrupt occurs.
* \image html mcwdt_counters.png
* 32-bit sub-counter C2 does not have Clear on match option. 
* The interrupt of counter C2 occurs when the counts equal 
* 2<sup>Toggle bit</sup> value.
* \image html mcwdt_subcounters.png
* To set up an MCWDT, provide the configuration parameters in the 
* cy_stc_mcwdt_config_t structure. Then call  
* Cy_MCWDT_Init() to initialize the driver. 
* Call Cy_MCWDT_Enable() to enable all specified counters.
*
* You can also set the mode of operation for any counter. If you choose
* interrupt mode, use Cy_MCWDT_SetInterruptMask() with the 
* parameter for the masks described in Macro Section. 
* Additionally, enable the Global interrupts and initialize the referenced 
* interrupt by setting the priority and the interrupt vector using  
* \ref Cy_SysInt_Init() of the sysint driver.
* 
* The values of the MCWDT counters can be monitored using 
* Cy_MCWDT_GetCount().
*
* \note In addition to the MCWDTs, each device has a separate watchdog timer
* (WDT) that can also be used to generate a watchdog reset or periodic
* interrupts. For more information on the WDT, see the appropriate section
* of the PDL.
*
* \section group_mcwdt_more_information More Information
*
* For more information on the MCWDT peripheral, refer to 
* the technical reference manual (TRM).
*
* \section group_mcwdt_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_mcwdt_macro Macro
* \defgroup group_mcwdt_functions Functions
* \defgroup group_mcwdt_data_structures Data structures
* \defgroup group_mcwdt_enums Enumerated Types
*/

#ifndef CY_MCWDT_H
#define CY_MCWDT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"

#ifndef CY_IP_MXS40SRSS_MCWDT
    #error "The MCWDT driver is not supported on this device"
#endif

/**
* \addtogroup group_mcwdt_macro
* \{
*/

/** Driver major version */
#define CY_MCWDT_DRV_VERSION_MAJOR       (1ul)

/** Driver minor version */
#define CY_MCWDT_DRV_VERSION_MINOR       (0ul)

/** \cond INTERNAL_MACROS */

/***************************************
*        Registers Constants
***************************************/

#define CY_MCWDT_LOCK_CLR0      (1ul)
#define CY_MCWDT_LOCK_CLR1      (2ul)
#define CY_MCWDT_LOCK_SET01     (3ul)

#define CY_MCWDT_BYTE_SHIFT     (8ul)
#define CY_MCWDT_C0C1_MODE_MASK (3ul)
#define CY_MCWDT_C2_MODE_MASK   (1ul)


/***************************************
*            API Constants
***************************************/

#define CY_MCWDT_NUM_OF_SUBCOUNTER      (2ul)

#define CY_MCWDT_ALL_WDT_ENABLE_Msk (MCWDT_CTL_WDT_ENABLE0_Msk | MCWDT_CTL_WDT_ENABLE1_Msk | \
                                  MCWDT_CTL_WDT_ENABLE2_Msk)

#define CY_MCWDT_CTR0_Pos (0ul)
#define CY_MCWDT_CTR1_Pos (1ul)
#define CY_MCWDT_CTR2_Pos (2ul)
#define CY_MCWDT_CTR_Pos  (0ul)

/** \endcond */

#define CY_MCWDT_ID       CY_PDL_DRV_ID(0x35ul)                      /**< MCWDT PDL ID */

#define CY_MCWDT_CTR0     (1ul << CY_MCWDT_CTR0_Pos)                /**< The sub-counter#0 mask. This macro is used with functions 
                                                                   that handle multiple counters, including Cy_MCWDT_Enable(),
                                                                   Cy_MCWDT_Disable(), Cy_MCWDT_ClearInterrupt() and Cy_MCWDT_ResetCounters(). */
#define CY_MCWDT_CTR1     (1ul << CY_MCWDT_CTR1_Pos)                /**< The sub-counter#1 mask. This macro is used with functions 
                                                                   that handle multiple counters, including Cy_MCWDT_Enable(),
                                                                   Cy_MCWDT_Disable(), Cy_MCWDT_ClearInterrupt() and Cy_MCWDT_ResetCounters(). */
#define CY_MCWDT_CTR2     (1ul << CY_MCWDT_CTR2_Pos)                /**< The sub-counter#2 mask. This macro is used with functions 
                                                                   that handle multiple counters, including Cy_MCWDT_Enable(),
                                                                   Cy_MCWDT_Disable(), Cy_MCWDT_ClearInterrupt() and Cy_MCWDT_ResetCounters(). */
#define CY_MCWDT_CTR_Msk  (CY_MCWDT_CTR0 | CY_MCWDT_CTR1 | CY_MCWDT_CTR2) /**< The mask for all sub-counters. This macro is used with functions 
                                                                   that handle multiple counters, including Cy_MCWDT_Enable(),
                                                                   Cy_MCWDT_Disable(), Cy_MCWDT_ClearInterrupt() and Cy_MCWDT_ResetCounters(). */

/** \} group_mcwdt_macro */


/**
* \addtogroup group_mcwdt_enums
* \{
*/

typedef enum 
{
    CY_MCWDT_DISABLE = 0u,
    CY_MCWDT_ENABLE,
} cy_en_mcwdt_enable_t;

typedef enum
{
    CY_MCWDT_PAUSED_BY_DPSLP_CM0       = 0u,
    CY_MCWDT_PAUSED_BY_DPSLP_CM4_CM7_0 = 1u,
    CY_MCWDT_PAUSED_BY_DPSLP_CM7_1     = 2u,
    CY_MCWDT_PAUSED_BY_DPSLP_CM7_2     = 3u,
    CY_MCWDT_PAUSED_BY_NO_CORE         = 4u,
} cy_en_mcwdt_select_core_t;

/** The mcwdt sub-counter identifiers. */
typedef enum
{
    CY_MCWDT_COUNTER0,  /**< Sub-counter#0 identifier. */
    CY_MCWDT_COUNTER1,  /**< Sub-counter#1 identifier. */
    CY_MCWDT_COUNTER2   /**< Sub-counter#2 identifier. */
} cy_en_mcwdtctr_t;

/** The mcwdt lower and upper actions. */
typedef enum
{
    CY_MCWDT_ACTION_NONE,                 /**< The No action mode. It is used for Set/GetMode functions. */
    CY_MCWDT_ACTION_FAULT,                /**< The Fault mode. It is used for Set/GetMode functions. */
    CY_MCWDT_ACTION_FAULT_THEN_RESET,     /**< The Reset mode. It is used for Set/GetMode functions. */
} cy_en_mcwdt_lower_upper_action_t;

/** The mcwdt warn actions. */
typedef enum
{
    CY_MCWDT_WARN_ACTION_NONE,
    CY_MCWDT_WARN_ACTION_INT,
} cy_en_mcwdt_warn_action_t;

/** The mcwdt subcounter 2 actions. */
typedef enum
{
    CY_MCWDT_CNT2_ACTION_NONE,
    CY_MCWDT_CNT2_ACTION_INT,
} cy_en_mcwdt_cnt2_action_t;

/** The MCWDT error codes. */
typedef enum 
{
    CY_MCWDT_SUCCESS = 0x00u,                                            /**< Successful */
    CY_MCWDT_BAD_PARAM = CY_MCWDT_ID | CY_PDL_STATUS_ERROR | 0x01u,     /**< One or more invalid parameters */
} cy_en_mcwdt_status_t;

typedef enum
{
    CY_MCWDT_CNT2_MONITORED_BIT0 = 0u, // means 1 count period
    CY_MCWDT_CNT2_MONITORED_BIT1,      // means 2 count period
    CY_MCWDT_CNT2_MONITORED_BIT2,      // means 4 count period
    CY_MCWDT_CNT2_MONITORED_BIT3,      // means 8 count period
    CY_MCWDT_CNT2_MONITORED_BIT4,      // means 16 count period
    CY_MCWDT_CNT2_MONITORED_BIT5,      // means 32 count period
    CY_MCWDT_CNT2_MONITORED_BIT6,      // means 64 count period
    CY_MCWDT_CNT2_MONITORED_BIT7,      // means 128 count period
    CY_MCWDT_CNT2_MONITORED_BIT8,      // means 256 count period
    CY_MCWDT_CNT2_MONITORED_BIT9,      // means 512 count period
    CY_MCWDT_CNT2_MONITORED_BIT10,     // means 1024 count period
    CY_MCWDT_CNT2_MONITORED_BIT11,     // means 2048 count period
    CY_MCWDT_CNT2_MONITORED_BIT12,     // means 4096 count period
    CY_MCWDT_CNT2_MONITORED_BIT13,     // means 8192 count period
    CY_MCWDT_CNT2_MONITORED_BIT14,     // means 16384 count period
    CY_MCWDT_CNT2_MONITORED_BIT15,     // means 32768 count period
    CY_MCWDT_CNT2_MONITORED_BIT16,     // means 65536 count period
    CY_MCWDT_CNT2_MONITORED_BIT17,     // means 131072 count period
    CY_MCWDT_CNT2_MONITORED_BIT18,     // means 262144 count period
    CY_MCWDT_CNT2_MONITORED_BIT19,     // means 524288 count period
    CY_MCWDT_CNT2_MONITORED_BIT20,     // means 1048576 count period
    CY_MCWDT_CNT2_MONITORED_BIT21,     // means 2097152 count period
    CY_MCWDT_CNT2_MONITORED_BIT22,     // means 4194304 count period
    CY_MCWDT_CNT2_MONITORED_BIT23,     // means 8388608 count period
    CY_MCWDT_CNT2_MONITORED_BIT24,     // means 16777216 count period
    CY_MCWDT_CNT2_MONITORED_BIT25,     // means 33554432 count period
    CY_MCWDT_CNT2_MONITORED_BIT26,     // means 67108864 count period
    CY_MCWDT_CNT2_MONITORED_BIT27,     // means 134217728 count period
    CY_MCWDT_CNT2_MONITORED_BIT28,     // means 268435456 count period
    CY_MCWDT_CNT2_MONITORED_BIT29,     // means 536870912 count period
    CY_MCWDT_CNT2_MONITORED_BIT30,     // means 1073741824 count period
    CY_MCWDT_CNT2_MONITORED_BIT31,     // means 2147483648 count period
} cy_en_mcwdcnt2_bit_monitored_t;


/** \} group_mcwdt_enums */

/**
* \addtogroup group_mcwdt_data_structures
* \{
*/

/** The MCWDT component configuration structure. */
typedef struct
{
    uint16_t c0LowerLimit;                              /**< The sub-counter#0 lower limit value, for reset or fault. 
                                                        Range: 0 - 65535 for c0AutoService = 0 and 1 - 65535 for c0AutoService = 1. */
    uint16_t c0UpperLimit;                              /**< The sub-counter#0 upper limit value, for reset or fault. 
                                                        Range: 0 - 65535 for c0AutoService = 0 and 1 - 65535 for c0AutoService = 1. */
    uint16_t c0WarnLimit;                               /**< The sub-counter#0 warn limit value, for interrupt. 
                                                        Range: 0 - 65535 for c0AutoService = 0 and 1 - 65535 for c0AutoService = 1. */
    cy_en_mcwdt_lower_upper_action_t  c0LowerAction;    /**< The sub-counter#0 action. It can have the following values:
                                                        \ref cy_en_mcwdt_lower_upper_action_t */
    cy_en_mcwdt_lower_upper_action_t  c0UpperAction;    /**< The sub-counter#0 action. It can have the following values:
                                                        \ref cy_en_mcwdt_lower_upper_action_t */
    cy_en_mcwdt_warn_action_t  c0WarnAction;            /**< The sub-counter#0 warn action. It can have the following values:
                                                        \ref cy_en_mcwdt_warn_action_t */
    cy_en_mcwdt_enable_t     c0AutoService;             /**< The sub-counter#0 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdt_enable_t     c0SleepDeepPause;          /**< The sub-counter#0 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdt_enable_t     c0DebugRun;                /**< The sub-counter#0 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    uint16_t c1LowerLimit;                              /**< The sub-counter#0 lower limit value, for reset or fault. 
                                                        Range: 0 - 65535 for c1AutoService = 0 and 1 - 65535 for c1AutoService = 1. */
    uint16_t c1UpperLimit;                              /**< The sub-counter#0 upper limit value, for reset or fault. 
                                                        Range: 0 - 65535 for c1AutoService = 0 and 1 - 65535 for c1AutoService = 1. */
    uint16_t c1WarnLimit;                               /**< The sub-counter#0 warn limit value, for interrupt. 
                                                        Range: 0 - 65535 for c1AutoService = 0 and 1 - 65535 for c1AutoService = 1. */
    cy_en_mcwdt_lower_upper_action_t  c1LowerAction;    /**< The sub-counter#1 action. It can have the following values:
                                                        \ref cy_en_mcwdt_lower_upper_action_t */
    cy_en_mcwdt_lower_upper_action_t  c1UpperAction;    /**< The sub-counter#1 action. It can have the following values:
                                                        \ref cy_en_mcwdt_lower_upper_action_t */
    cy_en_mcwdt_warn_action_t  c1WarnAction;            /**< The sub-counter#1 warn action. It can have the following values:
                                                        \ref cy_en_mcwdt_warn_action_t */
    cy_en_mcwdt_enable_t     c1AutoService;             /**< The sub-counter#1 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdt_enable_t     c1SleepDeepPause;          /**< The sub-counter#1 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdt_enable_t     c1DebugRun;                /**< The sub-counter#1 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdcnt2_bit_monitored_t  c2ToggleBit;        /**< The sub-counter#2 Period / Toggle Bit value.
                                                        \ref cy_en_mcwdcnt2_bit_monitored_t */
    cy_en_mcwdt_cnt2_action_t  c2Action;                /**< The sub-counter#2 mode. It can have the following values:
                                                        \ref cy_en_mcwdt_cnt2_action_t  */
    cy_en_mcwdt_enable_t     c2SleepDeepPause;          /**< The sub-counter#2 Auto Service parameter enabled/disabled. 
                                                        Range: FALSE - TRUE. */
    cy_en_mcwdt_enable_t     c2DebugRun;                /**< The sub-counter#2 Auto Service parameter enabled/disabled. **/

    cy_en_mcwdt_select_core_t coreSelect;               /**< Selecting the the core. The DeepSleep of the core pauses this MCWDT counter >**/
} cy_stc_mcwdt_config_t;

/** \} group_mcwdt_data_structures */

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_mcwdt_functions
* \{
*/
cy_en_mcwdt_status_t     Cy_MCWDT_Init(volatile stc_MCWDT_t *base, cy_stc_mcwdt_config_t const *config);
                void     Cy_MCWDT_DeInit(volatile stc_MCWDT_t *base);
void Cy_MCWDT_ClearWatchdog(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);


__STATIC_INLINE void Cy_MCWDT_Enable(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs);
__STATIC_INLINE void Cy_MCWDT_Disable(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs);
__STATIC_INLINE uint32_t Cy_MCWDT_GetEnabledStatus(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE void Cy_MCWDT_Lock(volatile stc_MCWDT_t *base);
__STATIC_INLINE void Cy_MCWDT_Unlock(volatile stc_MCWDT_t *base);
__STATIC_INLINE uint32_t Cy_MCWDT_GetLockedStatus(volatile stc_MCWDT_t const *base);
__STATIC_INLINE void Cy_MCWDT_SetLowerAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_lower_upper_action_t action);
__STATIC_INLINE void Cy_MCWDT_SetUpperAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_lower_upper_action_t action);
__STATIC_INLINE void Cy_MCWDT_SetWarnAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_warn_action_t action);
__STATIC_INLINE void Cy_MCWDT_SetSubCounter2Action(volatile stc_MCWDT_t *base, cy_en_mcwdt_cnt2_action_t action);
__STATIC_INLINE cy_en_mcwdt_lower_upper_action_t Cy_MCWDT_GetLowerAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE cy_en_mcwdt_lower_upper_action_t  Cy_MCWDT_GetUpperAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE cy_en_mcwdt_warn_action_t Cy_MCWDT_GetWarnAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE cy_en_mcwdt_cnt2_action_t Cy_MCWDT_GetSubCounter2Action(volatile stc_MCWDT_t *base);
__STATIC_INLINE void Cy_MCWDT_SetAutoService(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable);
__STATIC_INLINE uint32_t Cy_MCWDT_GetAutoService(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE uint32_t Cy_MCWDT_GetSleepDeepPause(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE void Cy_MCWDT_SetSleepDeepPause(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable);
__STATIC_INLINE void Cy_MCWDT_SetDebugRun(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable);
__STATIC_INLINE uint32_t Cy_MCWDT_GetDebugRun(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE void Cy_MCWDT_SetLowerLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs);
__STATIC_INLINE void Cy_MCWDT_SetUpperLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs);
__STATIC_INLINE void Cy_MCWDT_SetWarnLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs);
__STATIC_INLINE uint16_t Cy_MCWDT_GetUpperLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE uint16_t Cy_MCWDT_GetWarnLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE void Cy_MCWDT_SetToggleBit(volatile stc_MCWDT_t *base, cy_en_mcwdcnt2_bit_monitored_t bit);
__STATIC_INLINE cy_en_mcwdcnt2_bit_monitored_t Cy_MCWDT_GetToggleBit(volatile stc_MCWDT_t const *base);
__STATIC_INLINE uint32_t Cy_MCWDT_GetCount(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter);
__STATIC_INLINE void Cy_MCWDT_ResetCounters(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs);
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptStatus(volatile stc_MCWDT_t const *base);
__STATIC_INLINE void Cy_MCWDT_ClearInterrupt(volatile stc_MCWDT_t *base, uint32_t counters);
__STATIC_INLINE void Cy_MCWDT_SetInterrupt(volatile stc_MCWDT_t *base, uint32_t counters);
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptMask(volatile stc_MCWDT_t const *base);
__STATIC_INLINE void Cy_MCWDT_SetInterruptMask(volatile stc_MCWDT_t *base, uint32_t counters);
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptStatusMasked(volatile stc_MCWDT_t const *base);


//TODO: make CPU select function (YOTS)

/*******************************************************************************
* Function Name: Cy_MCWDT_Enable
****************************************************************************//**
*
*  Enables all specified counters.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counters
*  OR of all counters to enable. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, 
*  because the counter begins counting after two lf_clk cycles pass. 
*  The recommended value is 93 us.
*  \note
*  Setting this parameter to a zero means No wait. In this case, it is
*  the user's responsibility to check whether the selected counters were enabled 
*  immediately after the function call. This can be done by the 
*  Cy_MCWDT_GetEnabledStatus() API.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_Enable(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->CTR[0].unCTL.stcField.u1ENABLE = 1ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->CTR[1].unCTL.stcField.u1ENABLE = 1ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        base->unCTR2_CTL.stcField.u1ENABLE = 1ul;
    }
    Cy_SysLib_DelayUs(waitUs);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_Disable
****************************************************************************//**
*
*  Disables all specified counters.
*
*  \param base
*  The base pointer to a structure describing registers.
*
*  \param counters
*  OR of all counters to disable. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2 macros.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, 
*  because the counter stops counting after two lf_clk cycles pass. 
*  The recommended value is 93 us.
*  \note
*  Setting this parameter to a zero means No wait. In this case, it is 
*  the user's responsibility to check whether the selected counters were disabled 
*  immediately after the function call. This can be done by the 
*  Cy_MCWDT_GetEnabledStatus() API.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_Disable(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->CTR[0].unCTL.stcField.u1ENABLE = 0ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->CTR[1].unCTL.stcField.u1ENABLE = 0ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        base->unCTR2_CTL.stcField.u1ENABLE = 0ul;
    }
    Cy_SysLib_DelayUs(waitUs);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetEnabledStatus
****************************************************************************//**
*
*  Reports the enabled status of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the MCWDT counter. The valid range is [0-2].
*
*  \return
*  The status of the MCWDT counter: 0 = disabled, 1 = enabled.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetEnabledStatus(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter)
{
    uint32_t status = 0ul;
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        status = base->CTR[0].unCTL.stcField.u1ENABLED;
        break;
    case CY_MCWDT_COUNTER1:
        status = base->CTR[1].unCTL.stcField.u1ENABLED;
        break;
    case CY_MCWDT_COUNTER2:
        status = base->unCTR2_CTL.stcField.u1ENABLED;
        break;
    default:
        break;
    }
    return (status);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_CpuSelectForDpSlpPauseAction
****************************************************************************//**
*
*  Select deep sleep of which core for pausing this counter
*
*  \param core
*  The selected core. Deep sleep of the core pause this counter.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_CpuSelectForDpSlpPauseAction(volatile stc_MCWDT_t *base, cy_en_mcwdt_select_core_t core)
{
    base->unCPU_SELECT.u32Register  = core;
}



/*******************************************************************************
* Function Name: Cy_MCWDT_Lock
****************************************************************************//**
*
*  Locks out configuration changes to all MCWDT registers.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_Lock(volatile stc_MCWDT_t *base)
{
    uint32_t interruptState;

    interruptState = Cy_SysLib_EnterCriticalSection();

    base->unLOCK.stcField.u2MCWDT_LOCK = CY_MCWDT_LOCK_SET01;

    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_Unlock
****************************************************************************//**
*
*  Unlocks the MCWDT configuration registers.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_Unlock(volatile stc_MCWDT_t *base)
{
    uint32_t interruptState;

    interruptState = Cy_SysLib_EnterCriticalSection();

    base->unLOCK.stcField.u2MCWDT_LOCK = CY_MCWDT_LOCK_CLR0;
    base->unLOCK.stcField.u2MCWDT_LOCK = CY_MCWDT_LOCK_CLR1;

    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetLockStatus
****************************************************************************//**
*
*  Reports the locked/unlocked state of the MCWDT.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The state of the MCWDT counter: 0 = unlocked, 1 = locked.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetLockedStatus(volatile stc_MCWDT_t const *base)
{
    return ((0ul != base->unLOCK.stcField.u2MCWDT_LOCK) ? 1ul : 0ul);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_SetLowerAction
****************************************************************************//**
*
*  Sets the lower limit action of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \param action
*  The action of operation for the counter. See enum typedef cy_en_mcwdt_lower_upper_action_t.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetLowerAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_lower_upper_action_t action)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u2LOWER_ACTION = action;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u2LOWER_ACTION = action;
        break;
    default:
        break;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetUpperAction
****************************************************************************//**
*
*  Sets the upper limit action of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \param action
*  The action of operation for the counter. See enum typedef cy_en_mcwdt_lower_upper_action_t.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetUpperAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_lower_upper_action_t action)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u2UPPER_ACTION = action;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u2UPPER_ACTION = action;
        break;
    default:
        break;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetUpperAction
****************************************************************************//**
*
*  Sets the warn limit action of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \param action
*  The action of operation for the counter. See enum typedef cy_en_mcwdt_warn_action_t.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetWarnAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, cy_en_mcwdt_warn_action_t action)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u1WARN_ACTION = action;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u1WARN_ACTION = action;
        break;
    default:
        break;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetSubCounter2Action
****************************************************************************//**
*
*  Sets the counter 2 action of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param action
*  The action of operation for the counter. See enum typedef cy_en_mcwdt_cnt2_action_t.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetSubCounter2Action(volatile stc_MCWDT_t *base, cy_en_mcwdt_cnt2_action_t action)
{
    base->unCTR2_CONFIG.stcField.u1ACTION = action;
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetLowerAction
****************************************************************************//**
*
*  Reports the lower limit action type of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \return
*  The current lower limit action type of the counter.
*  See enum typedef cy_en_mcwdt_lower_upper_action_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_mcwdt_lower_upper_action_t Cy_MCWDT_GetLowerAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (cy_en_mcwdt_lower_upper_action_t)(base->CTR[0].unCONFIG.stcField.u2LOWER_ACTION);
    case CY_MCWDT_COUNTER1:
        return (cy_en_mcwdt_lower_upper_action_t)(base->CTR[1].unCONFIG.stcField.u2LOWER_ACTION);
    default:
        return (cy_en_mcwdt_lower_upper_action_t) 0;
    }    
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetUpperAction
****************************************************************************//**
*
*  Reports the upper limit action type of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \return
*  The current upper limit action type of the counter.
*  See enum typedef cy_en_mcwdt_lower_upper_action_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_mcwdt_lower_upper_action_t  Cy_MCWDT_GetUpperAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (cy_en_mcwdt_lower_upper_action_t)(base->CTR[0].unCONFIG.stcField.u2UPPER_ACTION);
    case CY_MCWDT_COUNTER1:
        return (cy_en_mcwdt_lower_upper_action_t)(base->CTR[1].unCONFIG.stcField.u2UPPER_ACTION);
    default:
        return (cy_en_mcwdt_lower_upper_action_t) 0;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetUpperAction
****************************************************************************//**
*
*  Reports the warn limit action type of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \return
*  The current warn limit action type of the counter.
*  See enum typedef cy_en_mcwdt_warn_action_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_mcwdt_warn_action_t Cy_MCWDT_GetWarnAction(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (cy_en_mcwdt_warn_action_t)(base->CTR[0].unCONFIG.stcField.u1WARN_ACTION);
    case CY_MCWDT_COUNTER1:
        return (cy_en_mcwdt_warn_action_t)(base->CTR[1].unCONFIG.stcField.u1WARN_ACTION);
    default:
        return (cy_en_mcwdt_warn_action_t) 0;
    }    
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetUpperAction
****************************************************************************//**
*
*  Reports the counter 2 action type of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The current counter 2 action type of the counter.
*  See enum typedef cy_en_mcwdt_cnt2_action_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_mcwdt_cnt2_action_t Cy_MCWDT_GetSubCounter2Action(volatile stc_MCWDT_t *base)
{
    return (cy_en_mcwdt_cnt2_action_t)(base->unCTR2_CONFIG.stcField.u1ACTION);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetAutoService
****************************************************************************//**
*
*  Sets the Auto service option for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-1].
*
*  \note
*  The auto service option is not supported by Counter 2.
*
*  \param enable
*  Set 0 to disable; 1 to enable.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetAutoService(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u1AUTO_SERVICE = enable;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u1AUTO_SERVICE = enable;
        break;
    default:        
        break;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetAutoService
****************************************************************************//**
*
*  Reports the Auto service setting for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \return
*  The Auto service status: 1 = enabled, 0 = disabled.
*
*  \note
*  The Auto service option is not supported by Counter 2.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetAutoService(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return base->CTR[0].unCONFIG.stcField.u1AUTO_SERVICE;
    case CY_MCWDT_COUNTER1:
        return base->CTR[1].unCONFIG.stcField.u1AUTO_SERVICE;
    default:
        return 0;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetSleepDeepPause
****************************************************************************//**
*
*  Sets the Sleep deep pause option for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-2].
*
*  \param enable
*  Set 0 to disable; 1 to enable.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetSleepDeepPause(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable)
{
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u1SLEEPDEEP_PAUSE = enable;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u1SLEEPDEEP_PAUSE = enable;
        break;
    case CY_MCWDT_COUNTER2:
        base->unCTR2_CONFIG.stcField.u1SLEEPDEEP_PAUSE = enable;
        break;
    default:
        break;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetSleepDeepPause
****************************************************************************//**
*
*  Reports the Sleep deep pause setting for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-2].
*
*  \return
*  The Auto service status: 1 = enabled, 0 = disabled.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetSleepDeepPause(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter)
{
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return base->CTR[0].unCONFIG.stcField.u1SLEEPDEEP_PAUSE;
    case CY_MCWDT_COUNTER1:
        return base->CTR[1].unCONFIG.stcField.u1SLEEPDEEP_PAUSE;
    case CY_MCWDT_COUNTER2:
        return base->unCTR2_CONFIG.stcField.u1SLEEPDEEP_PAUSE;
    default:
        return 0;
    }    
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetDebugRun
****************************************************************************//**
*
*  Sets the Debug run option for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-2].
*
*  \param enable
*  Set 0 to disable; 1 to enable.
*
*  \note
*  This API must not be called while the counters are running. 
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetDebugRun(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint32_t enable)
{
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unCONFIG.stcField.u1DEBUG_RUN = enable;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unCONFIG.stcField.u1DEBUG_RUN = enable;
        break;
    case CY_MCWDT_COUNTER2:
        base->unCTR2_CONFIG.stcField.u1DEBUG_RUN = enable;
        break;
    default:
        break;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetDebugRun
****************************************************************************//**
*
*  Reports the Debug run setting for the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-2].
*
*  \return
*  The Auto service status: 1 = enabled, 0 = disabled.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetDebugRun(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter)
{
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return base->CTR[0].unCONFIG.stcField.u1DEBUG_RUN;
    case CY_MCWDT_COUNTER1:
        return base->CTR[1].unCONFIG.stcField.u1DEBUG_RUN;
    case CY_MCWDT_COUNTER2:
        return base->unCTR2_CONFIG.stcField.u1DEBUG_RUN;
    default:
        return 0;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_SetLowerLimit
****************************************************************************//**
*
*  Sets the lower limit value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-1].
*
*  \param match
*  The value to lower limit against the counter. 
*  The valid range is [0-65535] when Auto service is disabled and [1-65535] when 
*  Auto service enabled.
*
*  \note
*  The lower limit value is not supported by Counter 2.
*
*  \note
*  Action on lower limit is taken on the next increment after the counter value 
*  equal to lower limit value.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, 
*  because the match affects after two lf_clk cycles pass. The recommended 
*  value is 93 us.
*  \note
*  Setting this parameter to a zero means No wait. This must be taken
*  into account when changing the match values on the running counters.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetLowerLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unLOWER_LIMIT.stcField.u16LOWER_LIMIT = limit;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unLOWER_LIMIT.stcField.u16LOWER_LIMIT = limit;
        break;
    default:
        break;
    }
    Cy_SysLib_DelayUs(waitUs);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetUpperLimit
****************************************************************************//**
*
*  Sets the upper limit value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-1].
*
*  \param match
*  The value to upper limit against the counter. 
*  The valid range is [0-65535] when Auto service is disabled and [1-65535] when 
*  Auto service enabled.
*
*  \note
*  The upper limit value is not supported by Counter 2.
*
*  \note
*  Action on upper limit is taken on the next increment after the counter value 
*  equal to upper limit value.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, 
*  because the match affects after two lf_clk cycles pass. The recommended 
*  value is 93 us.
*  \note
*  Setting this parameter to a zero means No wait. This must be taken
*  into account when changing the match values on the running counters.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetUpperLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unUPPER_LIMIT.stcField.u16UPPER_LIMIT = limit;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unUPPER_LIMIT.stcField.u16UPPER_LIMIT = limit;
        break;
    default:
        break;
    }
    Cy_SysLib_DelayUs(waitUs);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_SetWarnLimit
****************************************************************************//**
*
*  Sets the warn limit value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*   The number of the WDT counter. The valid range is [0-1].
*
*  \param match
*  The value to warn limit against the counter. 
*  The valid range is [0-65535] when Auto service is disabled and [1-65535] when 
*  Auto service enabled.
*
*  \note
*  The warn limit value is not supported by Counter 2.
*
*  \note
*  Action on warn limit is taken on the next increment after the counter value 
*  equal to warn limit value.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, 
*  because the match affects after two lf_clk cycles pass. The recommended 
*  value is 93 us.
*  \note
*  Setting this parameter to a zero means No wait. This must be taken
*  into account when changing the match values on the running counters.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetWarnLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter, uint16_t limit, uint16_t waitUs)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        base->CTR[0].unWARN_LIMIT.stcField.u16WARN_LIMIT = limit;
        break;
    case CY_MCWDT_COUNTER1:
        base->CTR[1].unWARN_LIMIT.stcField.u16WARN_LIMIT = limit;
        break;
    default:
        break;
    }
    Cy_SysLib_DelayUs(waitUs);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetLowerLimit
****************************************************************************//**
*
*  Reports the lower limit comparison value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \note
*  The lower limit value is not supported by Counter 2.
*
*  \return
*  A 16-bit lower limit value.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_MCWDT_GetLowerLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (uint16_t)(base->CTR[0].unLOWER_LIMIT.stcField.u16LOWER_LIMIT);
    case CY_MCWDT_COUNTER1:
        return (uint16_t)(base->CTR[1].unLOWER_LIMIT.stcField.u16LOWER_LIMIT);
    default:
        return 0;
    }    
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetUpperLimit
****************************************************************************//**
*
*  Reports the upper limit comparison value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \note
*  The upper limit value is not supported by Counter 2.
*
*  \return
*  A 16-bit upper limit value.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_MCWDT_GetUpperLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (uint16_t)(base->CTR[0].unUPPER_LIMIT.stcField.u16UPPER_LIMIT);
    case CY_MCWDT_COUNTER1:
        return (uint16_t)(base->CTR[1].unUPPER_LIMIT.stcField.u16UPPER_LIMIT);
    default:
        return 0;
    }
}

/*******************************************************************************
* Function Name: Cy_MCWDT_GetWarnLimit
****************************************************************************//**
*
*  Reports the warn limit comparison value for the specified counter (0 or 1).
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-1].
*
*  \note
*  The warn limit value is not supported by Counter 2.
*
*  \return
*  A 16-bit warn limit value.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_MCWDT_GetWarnLimit(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (uint16_t)(base->CTR[0].unWARN_LIMIT.stcField.u16WARN_LIMIT);
    case CY_MCWDT_COUNTER1:
        return (uint16_t)(base->CTR[1].unWARN_LIMIT.stcField.u16WARN_LIMIT);
    default:
        return 0;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_SetToggleBit
****************************************************************************//**
*
*  Sets a bit in Counter 2 to monitor for a toggle.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param bit
*  The Counter 2 bit is set to monitor for a toggle. The valid range [0-31].
*
*  \note
*  This API must not be called when counters are running.
*  Prior to calling this API, the counter must be disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetToggleBit(volatile stc_MCWDT_t *base, cy_en_mcwdcnt2_bit_monitored_t bit)
{
    base->unCTR2_CONFIG.stcField.u5BITS = bit;
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetToggleBit
****************************************************************************//**
*
*  Reports which bit in Counter 2 is monitored for a toggle.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The bit that is monitored (range 0 to 31).
*
*******************************************************************************/
__STATIC_INLINE cy_en_mcwdcnt2_bit_monitored_t Cy_MCWDT_GetToggleBit(volatile stc_MCWDT_t const *base)
{
    return (cy_en_mcwdcnt2_bit_monitored_t)(base->unCTR2_CONFIG.stcField.u5BITS);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetCount
****************************************************************************//**
*
*  Reports the current counter value of the specified counter.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the WDT counter. The valid range is [0-2].
*
*  \return
*  A live counter value. Counters 0 and 1 are 16-bit counters and Counter 2 is
*  a 32-bit counter.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetCount(volatile stc_MCWDT_t const *base, cy_en_mcwdtctr_t counter)
{
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        return (uint32_t)(base->CTR[0].unCNT.stcField.u16CNT);
    case CY_MCWDT_COUNTER1:
        return (uint32_t)(base->CTR[1].unCNT.stcField.u16CNT);
    case CY_MCWDT_COUNTER2:
        return base->unCTR2_CNT.stcField.u32CNT2;
    default:
        return 0;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_ResetCounters
****************************************************************************//**
*
*  Resets all specified counters.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counters
*  OR of all counters to reset. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*  \param waitUs
*  The function waits for some delay in microseconds before returning, because 
*  a reset occurs after one lf_clk cycle passes. The recommended value is 62 us.
*  \note
*  Setting this parameter to a zero means No wait. In this case, it is the 
*  user's responsibility to check whether the selected counters were reset 
*  immediately after the function call. This can be done by the 
*  Cy_MCWDT_GetCount() API.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_ResetCounters(volatile stc_MCWDT_t *base, uint32_t counters, uint16_t waitUs)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->unSERVICE.stcField.u1CTR0_SERVICE = 1ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->unSERVICE.stcField.u1CTR1_SERVICE = 1ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        // No reset functionality for CNT2
    }
    Cy_SysLib_DelayUs(waitUs);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_WaitForCounterReset
****************************************************************************//**
*
*  wait completion of counter reset.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counter
*  The number of the MCWDT counter. The valid range is [0-1].
*
*
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_WaitForCounterReset(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    CY_ASSERT(counter != CY_MCWDT_COUNTER2);
    
    switch (counter)
    {
    case CY_MCWDT_COUNTER0:
        while(base->unSERVICE.stcField.u1CTR0_SERVICE != 0ul);
        break;
    case CY_MCWDT_COUNTER1:
        while(base->unSERVICE.stcField.u1CTR1_SERVICE != 0ul);
        break;
    default:
        break;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetInterruptStatus
****************************************************************************//**
*
*  Reports the state of all MCWDT interrupts.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The OR'd state of the interrupts. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2 macros.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptStatus(volatile stc_MCWDT_t const *base)
{
    return base->unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_MCWDT_ClearInterrupt
****************************************************************************//**
*
*  Clears all specified MCWDT interrupts.
*
*  All the WDT interrupts must be cleared by the firmware; otherwise
*  interrupts are generated continuously.
*
*  \param base
*  The base pointer to a structure describes registers.
*
*  \param counters
*  OR of all interrupt sources to clear. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_ClearInterrupt(volatile stc_MCWDT_t *base, uint32_t counters)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->unINTR.stcField.u1CTR0_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->unINTR.stcField.u1CTR1_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        base->unINTR.stcField.u1CTR2_INT = 1ul;
    }

    /* Read the interrupt register to ensure that the initial clearing write has
    * been flushed out to the hardware.
    */
    (void)base->unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_MCWDT_SetInterrupt
****************************************************************************//**
*
*  Sets MCWDT interrupt sources in the interrupt request register.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counters
*  OR of all interrupt sources to set. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetInterrupt(volatile stc_MCWDT_t *base, uint32_t counters)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->unINTR_SET.stcField.u1CTR0_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->unINTR_SET.stcField.u1CTR1_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        base->unINTR_SET.stcField.u1CTR2_INT = 1ul;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetInterruptMask
****************************************************************************//**
*
* Returns the CWDT interrupt mask register. This register specifies which bits
* from the MCWDT interrupt request register will trigger an interrupt event.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The OR'd state of the interrupt masks. See the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptMask(volatile stc_MCWDT_t const *base)
{
    return base->unINTR_MASK.u32Register;
}


/*******************************************************************************
* Function Name: Cy_MCWDT_SetInterruptMask
****************************************************************************//**
*
* Writes MCWDT interrupt mask register. This register configures which bits
* from MCWDT interrupt request register will trigger an interrupt event.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \param counters
*  OR of all interrupt masks to set. See \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and
*  CY_MCWDT_CTR2  macros.
*
*******************************************************************************/
__STATIC_INLINE void Cy_MCWDT_SetInterruptMask(volatile stc_MCWDT_t *base, uint32_t counters)
{
    if (counters & CY_MCWDT_CTR0)
    {
        base->unINTR_MASK.stcField.u1CTR0_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR1)
    {
        base->unINTR_MASK.stcField.u1CTR1_INT = 1ul;
    }
    if (counters & CY_MCWDT_CTR2)
    {
        base->unINTR_MASK.stcField.u1CTR2_INT = 1ul;
    }
}


/*******************************************************************************
* Function Name: Cy_MCWDT_GetInterruptStatusMasked
****************************************************************************//**
*
* Returns the MCWDT interrupt masked request register. This register contains
* the logical AND of corresponding bits from the MCWDT interrupt request and 
* mask registers.
* In the interrupt service routine, this function identifies which of the 
* enabled MCWDT interrupt sources caused an interrupt event.
*
*  \param base
*  The base pointer to a structure that describes registers.
*
*  \return
*  The current status of enabled MCWDT interrupt sources. See
*  the \ref CY_MCWDT_CTR0, CY_MCWDT_CTR1, and CY_MCWDT_CTR2 macros.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_MCWDT_GetInterruptStatusMasked(volatile stc_MCWDT_t const *base)
{
    return base->unINTR_MASKED.u32Register;
}

/** \} group_mcwdt_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_MCWDT_H */

/** \} group_mcwdt */

/* [] END OF FILE */
