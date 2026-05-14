/***************************************************************************//**
* \file cy_sysreset.h
* \version 1.0
*
* Provides an API declaration of the sysreset driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sysreset System RESET (SysReset)
* \{
* The system reset provide APIs that can be called in the user application
* to handle the system resets.
*
* sysreset driver contains a set of reset related system functions. These functions
* can be called in the application routine. Some features of the system reset:
* * Software reset
* * Reading a reset cause
*
* \section group_sysreset_configuration Configuration Considerations
* There are no general sysreset configuration concerns.
*
* \section group_sysreset_more_information More Information
* Refer to the technical reference manual (TRM).
*
* \section group_sysreset_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_sysreset_macro Macro
* \defgroup group_sysreset_functions Functions
* \defgroup group_sysreset_data_structures Data Structures
*
*/

#if !defined(_CY_SYSRESET_H_)
#define _CY_SYSRESET_H_

#if !defined(NDEBUG)
    #include <string.h>
#endif /* NDEBUG */
#include <stdint.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */


/**
* \addtogroup group_sysreset_macro
* \{
*/

/******************************************************************************
* Macros
*****************************************************************************/

/** \} group_sysreset_macro */

/**
* \addtogroup group_sysreset_data_structures
* \{
*/

/** \} group_sysreset_data_structures */

/**
* \addtogroup group_sysreset_macro
* \{
*/


/******************************************************************************
* Constants
*****************************************************************************/

/**
* \defgroup group_sysreset_macro_reset_cause Reset cause
* \{
* Define RESET_CAUSE mask values
*/
/** A basic WatchDog Timer (WDT) reset has occurred since the last power cycle. */
#define CY_SYSRESET_WDT                 (0x00000001u)
/** The fault logging system requested a reset from its Active logic. */
#define CY_SYSRESET_ACT_FAULT           (0x00000002u)
/** The fault logging system requested a reset from its DeepSleep logic. */
#define CY_SYSRESET_DPSLP_FAULT         (0x00000004u)
/** Test controller or debugger asserted reset. */
#define CY_SYSRESET_TC_DBGRESET         (0x00000008u)
/** The CPU requested a system reset through it's SYSRESETREQ. This can be done via a debugger probe or in firmware. */
#define CY_SYSRESET_SOFT                (0x00000010u)
/** The Multi-Counter Watchdog timer #0 reset has occurred since last power cycle. */
#define CY_SYSRESET_MCWDT0              (0x00000020u)
/** The Multi-Counter Watchdog timer #1 reset has occurred since last power cycle. */
#define CY_SYSRESET_MCWDT1              (0x00000040u)
/** The Multi-Counter Watchdog timer #2 reset has occurred since last power cycle. */
#define CY_SYSRESET_MCWDT2              (0x00000080u)
/** The Multi-Counter Watchdog timer #3 reset has occurred since last power cycle. */
#define CY_SYSRESET_MCWDT3              (0x00000100u)
/** External XRES pin assertion. */
#define CY_SYSRESET_XRES                (0x00010000u)
/** Reset due to external VDDD supply crossing brown-out limit. */
#define CY_SYSRESET_BODVDDD             (0x00020000u)
/** Reset due to external VDDA supply crossed the brown-out limit. */
#define CY_SYSRESET_BODVDDA             (0x00040000u)
/** Reset due to internal VCCD core supply crossed the brown-out limit. */
#define CY_SYSRESET_BODVCCD             (0x00080000u)
/** Reset due to over voltage detection on the external VDDD supply. */
#define CY_SYSRESET_OVDVDDD             (0x00100000u)
/** Reset due to over voltage detection on the external VDDA supply.. */
#define CY_SYSRESET_OVDVDDA             (0x00200000u)
/** Reset due to over voltage detection on the internal core VCCD supply. */
#define CY_SYSRESET_OVDVCCD             (0x00400000u)
/** Reset due to over current detection on the internal VCCD supply when supplied by the ACTIVE power mode linear regulator. */
#define CY_SYSRESET_OCD_ACT_LINREG      (0x00800000u)
/** Reset due to over current detection on the internal VCCD supply when supplied by the DEEPSLEEP power mode linear regulator. */
#define CY_SYSRESET_OCD_DPSLP_LINREG    (0x01000000u)
/** Reset due to over current detection from REGHC (if present). */
#define CY_SYSRESET_OCD_REGHC           (0x02000000u)
/** PMIC status triggered a reset. */
#define CY_SYSRESET_PMIC                (0x04000000u)
/** Programmable XRES triggered. */
#define CY_SYSRESET_PXRES               (0x08000000u)
/** Structural reset was asserted. */
#define CY_SYSRESET_STRUCT_XRES         (0x20000000u)
/** Indicator that a POR occurred. */
#define CY_SYSRESET_PORVDDD             (0x40000000u)

/** Violation of a high-frequency clock has occurred since last power cycle. */
#define CY_SYSRESET_CSV_HF              (0x00000200u)
/** Violation of a reference clock has occurred since last power cycle. */
#define CY_SYSRESET_CSV_REF             (0x00000400u)

/** The reset has occurred on wakeup from a Hibernate power mode. */
#define CY_SYSRESET_HIB_WAKEUP          (0x80000000u)

/** \} group_sysreset_macro_reset_cause */

  /** Bit[31:24] Opcode = 0x1B (SoftReset)
 *  Bit[7:1]   Type   = 1    (Only CM4 reset)
 */
#define CY_IPC_DATA_FOR_CM4_SOFT_RESET  (0x1B000002UL)

/** \} group_sysreset_macro */


  
/******************************************************************************
* Function prototypes
******************************************************************************/

/**
* \addtogroup group_sysreset_functions
* \{
*/

uint32_t Cy_SysReset_GetResetReason(void);
uint32_t Cy_SysReset_GetNumHfclkResetCause(void);
bool Cy_SysReset_IsResetDueToHibWakeup(void);
void Cy_SysReset_ClearAllResetReasons(void);

#if (CY_CPU_CORTEX_M0P)
    void Cy_SysReset_SoftResetCM4(void);
#endif /* CY_CPU_CORTEX_M0P */


/** \} group_sysreset_functions */

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* _CY_SYSRESET_H_ */

/** \} group_sysreset */

/* [] END OF FILE */
