/***************************************************************************//**
* \file cy_syswdt.h
* \version 1.0
*
*  This file provides constants and parameter values for the WDT driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*
*******************************************************************************/

/**
* \defgroup group_wdt Watchdog Timer (SysWdt)
* \{
*
* The Watchdog timer (WDT) has a 16-bit free-running up-counter. The WDT can 
* issue counter match interrupts, and a device reset if its interrupts are not 
* handled. Use the Watchdog timer for two main purposes.<br>
* The <b> First use case </b> is recovering from a CPU or firmware failure.
* A timeout period is set up in the Watchdog timer, and if a timeout occurs, the
* device is reset (WRES). <br>
* The <b>Second use case</b> is to generate periodic interrupts.
*
* A "reset cause" register exists, and the firmware should check this register 
* at a start-up. An appropriate action can be taken if a WRES reset is detected.
*
* The user's firmware periodically resets the timeout period (clears or "feeds" 
* the watchdog) before a timeout occurs. If the firmware fails to do so, that is
* considered to be a CPU crash or a firmware failure, and the reason for a 
* device reset.
* The WDT can generate an interrupt instead of a device reset. The Interrupt 
* Service Routine (ISR) can handle the interrupt either as a periodic interrupt,
* or as an early indication of a firmware failure and respond accordingly.
* However, it is not recommended to use the WDT for periodic interrupt 
* generation. The Multi-counter Watchdog Timers (MCWDT) can be used to generate 
* periodic interrupts if such are presented in the device.
*
* <b> Functional Description </b> <br>
* The WDT generates an interrupt when the count value in the counter equals the 
* configured match value.
*
* Note that the counter is not reset on a match. In such case the WDT
* reset period is:
* WDT_Reset_Period = ILO_Period * (2*2^(16-IgnoreBits) + MatchValue);
* When the counter reaches a match value, it generates an interrupt and then 
* keeps counting up until it overflows and rolls back to zero and reaches the 
* match value again, at which point another interrupt is generated.
*
* To use a WDT to generate a periodic interrupt, the match value should be 
* incremented in the ISR. As a result, the next WDT interrupt is generated when 
* the counter reaches a new match value.
*
* You can also reduce the entire WDT counter period by 
* specifying the number of most significant bits that are ignored in the WDT 
* counter. For example, if the Cy_WDT_SetIgnoreBits() function is called with
* parameter 3, the WDT counter becomes a 13-bit free-running up-counter.
*
* <b> Power Modes </b> <br>
* WDT can operate in all possible low power modes. 
* Operation during Hibernate mode is possible because the logic and 
* high-voltage internal low oscillator (ILO) are supplied by the external 
* high-voltage supply (Vddd). The WDT can be configured to wake the device from 
* Hibernate mode.
*
* In Active or LPActive mode, an interrupt request from the WDT is sent to the 
* CPU via IRQ 22. In Sleep, LPSleep or Deep Sleep power mode, the CPU subsystem 
* is powered down, so the interrupt request from the WDT is sent directly to the
* WakeUp Interrupt Controller (WIC) which will then wake up the CPU. The 
* CPU then acknowledges the interrupt request and executes the ISR.
*
* <b> Clock Source </b> <br>
* The WDT is clocked by the ILO. The WDT must be disabled before disabling 
* the ILO. According to the device datasheet, the ILO accuracy is +/-30% over 
* voltage and temperature. This means that the timeout period may vary by 30% 
* from the configured value. Appropriate margins should be added while 
* configuring WDT intervals to make sure that unwanted device resets do not 
* occur on some devices.
* 
* Refer to the device datasheet for more information on the oscillator accuracy.
*
* <b> Register Locking </b> <br>
* You can prevent accidental corruption of the WDT configuration by calling 
* the Cy_WDT_Lock() function. When the WDT is locked, any writing to the WDT_*, 
* CLK_ILO_CONFIG, CLK_SELECT.LFCLK_SEL, and CLK_TRIM_ILO_CTL registers is 
* ignored.
* Call the Cy_WDT_Unlock() function to allow WDT registers modification.
*
* <b> Clearing WDT </b> <br>
* The ILO clock is asynchronous to the SysClk. Therefore it generally 
* takes three ILO cycles for WDT register changes to come into effect. It is 
* important to remember that a WDT should be cleared at least four cycles 
* (3 + 1 for sure) before a timeout occurs, especially when small 
* match values / low-toggle bit numbers are used.
*
* \warning It may happen that a WDT reset can be generated 
* faster than a device start-up. To prevent this, calculate the 
* start-up time and WDT reset time. The WDT reset time should be always smaller 
* than device start-up time.
*
* <b> Reset Detection </b> <br>
* Use the Cy_SysReset_GetResetReason() function to detect whether the WDT has 
* triggered a device reset.
*
* <b> Interrupt Configuration </b> <br>
* The Global Signal Reference and Interrupt components can be used for ISR 
* configuration. If the WDT is configured to generate an interrupt, pending 
* interrupts must be cleared within the ISR (otherwise, the interrupt will be 
* generated continuously).
* A pending interrupt to the WDT block must be cleared by calling the 
* Cy_WDT_ClearInterrupt() function. The call to the function will clear the 
* unhandled WDT interrupt counter.
*
* Use the WDT ISR as a timer to trigger certain actions 
* and to change a next WDT match value.
*
* Ensure that the interrupts from the WDT are passed to the CPU to avoid 
* unregistered interrupts. Unregistered WDT interrupts result in a continuous 
* device reset. To avoid this, call Cy_WDT_UnmaskInterrupt().
* After that, call the WDT API functions for interrupt
* handling/clearing.
*
* \section group_wdt_configuration Configuration Considerations
*
* To start the WDT, make sure that ILO is enabled.
* After the ILO is enabled, ensure that the WDT is unlocked and disabled by 
* calling the Cy_WDT_Unlock() and Cy_WDT_Disable() functions. Set the WDT match 
* value by calling Cy_WDT_SetMatch() with the required match value. If needed, 
* set the ignore bits for reducing the WDT counter period by calling 
* Cy_WDT_SetIgnoreBits() function. After the WDT configuration is set, 
* call Cy_WDT_Enable().
*
* \note Enable a WDT if the power supply can produce
* sudden brownout events that may compromise the CPU functionality. This 
* ensures that the system can recover after a brownout.
*
* When the WDT is used to protect against system crashes, the 
* WDT interrupt should be cleared by a portion of the code that is not directly 
* associated with the WDT interrupt.
* Otherwise, it is possible that the main firmware loop has crashed or is in an 
* endless loop, but the WDT interrupt vector continues to operate and service 
* the WDT. The user should:
* * Feed the watchdog by clearing the interrupt bit regularly in the main body 
* of the firmware code.
*
* * Guarantee that the interrupt is cleared at least once every WDT period.
*
* * Use the WDT ISR only as a timer to trigger certain actions and to change the
* next match value.
*
* \section group_wdt_section_more_information More Information
*
* For more information on the WDT peripheral, refer to the technical reference
* manual (TRM).
*
* \section group_wdt_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_wdt_macro Macro
* \defgroup group_wdt_functions Functions
*
*/

#if !defined(_WDT_H_)
#define _WDT_H_

#include <stdint.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
*       Function Constants
*******************************************************************************/

/**
* \addtogroup group_wdt_macros
* \{
*/

/** The driver major version */
#define CY_WDT_DRV_VERSION_MAJOR                       (1ul)

/** The driver minor version */
#define CY_WDT_DRV_VERSION_MINOR                       (0ul)

/** The WDT default match value */
#define CY_WDT_DEFAULT_MATCH_VALUE                     (32000ul) // 1 sec when clk_lf = 32KHz

/** The WDT driver identifier */
#define CY_WDT_ID                                      CY_PDL_DRV_ID(0x34u)

/** \} group_wdt_macros */

/**
* \addtogroup group_wdt_enums
* \{
*/

/** The wdt lower/upper limit actions. */
typedef enum
{
    CY_WDT_LOW_UPP_ACTION_NONE,
    CY_WDT_LOW_UPP_ACTION_RESET
} cy_en_wdt_lower_upper_action_t;

/** The wdt lower/upper limit actions. */
typedef enum
{
    CY_WDT_WARN_ACTION_NONE,
    CY_WDT_WARN_ACTION_INT
} cy_en_wdt_warn_action_t;

typedef enum
{
    CY_WDT_DISABLE,
    CY_WDT_ENABLE
} cy_en_wdt_enable_t;

/** \} group_wdt_enums */


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_wdt_functions
* @{
*/
/* WDT API */
void Cy_WDT_Init(void);
void Cy_WDT_Lock(void);
void Cy_WDT_Unlock(void);
void Cy_WDT_Enable(void);
void Cy_WDT_Disable(void);
void Cy_WDT_ClearInterrupt(void);
void Cy_WDT_ClearWatchdog(void);

__STATIC_INLINE void Cy_WDT_SetLowerLimit(uint32_t match);
__STATIC_INLINE void Cy_WDT_SetUpperLimit(uint32_t match);
__STATIC_INLINE void Cy_WDT_SetWarnLimit(uint32_t match);
__STATIC_INLINE void Cy_WDT_SetLowerAction(cy_en_wdt_lower_upper_action_t action);
__STATIC_INLINE void Cy_WDT_SetUpperAction(cy_en_wdt_lower_upper_action_t action);
__STATIC_INLINE void Cy_WDT_SetWarnAction(cy_en_wdt_warn_action_t action);
__STATIC_INLINE void Cy_WDT_SetAutoService(cy_en_wdt_enable_t enable);
__STATIC_INLINE void Cy_WDT_SetDeepSleepPause(cy_en_wdt_enable_t enable);
__STATIC_INLINE void Cy_WDT_SetHibarnatePause(cy_en_wdt_enable_t enable);
__STATIC_INLINE void Cy_WDT_SetDebugRun(cy_en_wdt_enable_t enable);
__STATIC_INLINE uint32_t Cy_WDT_GetCount(void);
__STATIC_INLINE void Cy_WDT_SetService();
__STATIC_INLINE void Cy_WDT_MaskInterrupt(void);
__STATIC_INLINE void Cy_WDT_UnmaskInterrupt(void);


/*******************************************************************************
* Function Name: Cy_WDT_SetLowerLimit
****************************************************************************//**
*
* Set lower limit for watchdog.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetLowerLimit(uint32_t match)
{
    WDT->unLOWER_LIMIT.stcField.u32LOWER_LIMIT = match;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetUpperLimit
****************************************************************************//**
*
* Set upper limit for watchdog.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetUpperLimit(uint32_t match)
{
    WDT->unUPPER_LIMIT.stcField.u32UPPER_LIMIT = match;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetWarnLimit
****************************************************************************//**
*
* Set warn limit for watchdog.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetWarnLimit(uint32_t match)
{
    WDT->unWARN_LIMIT.stcField.u32WARN_LIMIT = match;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetLowerAction
****************************************************************************//**
*
* Set action taken if this watchdog is serviced before LOWER_LIMIT is reached.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetLowerAction(cy_en_wdt_lower_upper_action_t action)
{
    WDT->unCONFIG.stcField.u1LOWER_ACTION = action;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetUpperAction
****************************************************************************//**
*
* Set action taken if this watchdog is not serviced before UPPER_LIMIT is reached.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetUpperAction(cy_en_wdt_lower_upper_action_t action)
{
    WDT->unCONFIG.stcField.u1UPPER_ACTION = action;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetWarnAction
****************************************************************************//**
*
* Set action taken when the count value reaches WARN_LIMIT.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetWarnAction(cy_en_wdt_warn_action_t action)
{
    WDT->unCONFIG.stcField.u1WARN_ACTION = action;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetAutoService
****************************************************************************//**
*
* Set automatically service when the count value reaches WARN_LIMIT.
* This function is used for WARN_LIMIT.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetAutoService(cy_en_wdt_enable_t enable)
{
    WDT->unCONFIG.stcField.u1AUTO_SERVICE = enable;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetDeepSleepPause
****************************************************************************//**
*
* Set whether the WDT counter runs/pauses when the system is in DEEPSLEEP.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetDeepSleepPause(cy_en_wdt_enable_t enable)
{
    WDT->unCONFIG.stcField.u1DPSLP_PAUSE = enable;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetHibarnatePause
****************************************************************************//**
*
* Set whether the WDT counter runs/pauses when the system is in HIBERNATE.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetHibarnatePause(cy_en_wdt_enable_t enable)
{
    WDT->unCONFIG.stcField.u1HIB_PAUSE = enable;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetDebugRun
****************************************************************************//**
*
* Set the debug run value. It needs when using debugger.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetDebugRun(cy_en_wdt_enable_t enable)
{
    WDT->unCONFIG.stcField.u1DEBUG_RUN = enable;
}


/*******************************************************************************
* Function Name: Cy_WDT_GetCount
****************************************************************************//**
*
* Reads the current WDT counter value.
*
* \return A live counter value.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_WDT_GetCount(void)
{
    return WDT->unCNT.stcField.u32CNT;
}


/*******************************************************************************
* Function Name: Cy_WDT_SetService
****************************************************************************//**
*
* Set service of the watchdog. This resets the count value to zero.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_SetService()
{
    WDT->unSERVICE.stcField.u1SERVICE = 1ul;
}


/*******************************************************************************
* Function Name: Cy_WDT_MaskInterrupt
****************************************************************************//**
*
* After masking interrupts from the WDT, they are passed to CPU.
* This function does not impact the reset generation.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_MaskInterrupt(void)
{
    WDT->unINTR_MASK.stcField.u1WDT = 1ul;
}


/*******************************************************************************
* Function Name: Cy_WDT_UnmaskInterrupt
****************************************************************************//**
*
* After unmasking interrupts from the WDT, they are not passed to the CPU.
* This function does not disable the WDT-reset generation.
*
*******************************************************************************/
__STATIC_INLINE void Cy_WDT_UnmaskInterrupt(void)
{
    WDT->unINTR_MASK.stcField.u1WDT = 0ul;
}


/** \} group_wdt_functions */

#if defined(__cplusplus)
}
#endif

#endif /* _WDT_H_ */

/** \} group_wdt */


/* [] END OF FILE */
