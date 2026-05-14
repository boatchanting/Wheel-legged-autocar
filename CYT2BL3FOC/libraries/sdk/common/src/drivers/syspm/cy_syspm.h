/***************************************************************************//**
* \file cy_syspm.h
* \version 1.0
*
* Provides the function definitions for the power management API.
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
* \defgroup group_syspm System Power Management (SysPm)
* \{
* Power Modes
*
* Devices support the following power modes (in the order of
* high-to-low power consumption): Active, Sleep, Low Power (LP) Active,
* LP Sleep, DeepSleep, and Hibernate.
* Active and Sleep are the standard ARM-defined power modes, supported by the
* ARM CPUs.
* DeepSleep is a lower-power mode where high-frequency clocks are disabled.
* Most of the register state is retained and the platform supports saving a
* configurable amount of the SRAM state.
* Hibernate is an even lower power mode that is entered from firmware,
* just like DeepSleep. However, on a wakeup the CPU and all peripherals go
* through a full reset.
*
* The syspm driver is used to enter low-power states to reduce system
* power consumption in power sensitive designs. For multi-core devices this 
* library allows the user to individually power down each CPU into Sleep or
* DeepSleep.
*
* <table class="doxtable">
*   <tr><th>CM0+ Mode</th><th>CM4 Mode(*2)</th><th>Power Mode (SRSS)</th>
*   <th>Mode Description</th></tr>
*   <tr>
*     <td>Active</td>
*     <td>Active</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate or optionally can
*         be disabled. Current load limitation is set as for
*         active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>Sleep</td>
*     <td>Active</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate or optionally can
*         be disabled. Current load limitation is set as for
*         active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>Active</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate or optionally can
*         be disabled. Current load limitation is set as for
*         active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>Sleep</td>
*     <td>Sleep</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate, CPUs Off. Current load
*         limitation is set as for active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>Sleep</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate, CPUs Off. Current load
*         limitation is set as for active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>Active</td>
*     <td>Sleep</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate or optionally can
*         be disabled. Current load limitation is set as for
*         active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>Active</td>
*     <td>DeepSleep</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate or optionally can
*         be disabled. Current load limitation is set as for
*         active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>Sleep</td>
*     <td>DeepSleep</td>
*     <td>Normal</td>
*     <td>All Clocks and peripherals may operate, CPUs Off. Current load
*         limitation is set as for active power mode(*1).</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>DeepSleep</td>
*     <td>DeepSleep</td>
*     <td>All the MHz clocks are off, but so is the DSI routing, and most Analog
*         and voltage references are off by default. Only ILO/WCO may operate,
*         limited peripherals.</td>
*   </tr>
*   <tr>
*     <td>Active (LPActive)</td>
*     <td>Active (LPActive)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode.</td>
*   </tr>
*   <tr>
*     <td>Sleep (LPSleep)</td>
*     <td>Active (LPActive)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode.</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>Active (LPActive)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode.</td>
*   </tr>
*   <tr>
*     <td>Active (LPActive)</td>
*     <td>Sleep (LPSleep)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode.</td>
*   </tr>
*   <tr>
*     <td>Sleep (LPSleep)</td>
*     <td>Sleep (LPSleep)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive mode. The CPUs are off.</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>Sleep (LPSleep)</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive. The CPUs are off</td>
*   </tr>
*   <tr>
*     <td>Active (LPActive)</td>
*     <td>DeepSleep</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode.</td>
*   </tr>
*   <tr>
*     <td>Sleep (LPSleep)</td>
*     <td>DeepSleep</td>
*     <td>Low power</td>
*     <td>All the MHz clocks frequencies should be decreased, if needed, to meet
*         current load limitations (*1). Peripherals should be slowed,
*         if needed, or/and optionally disabled to meet current load limitations
*         in LPActive power mode. The CPUs are off</td>
*   </tr>
*   <tr>
*     <td>DeepSleep</td>
*     <td>DeepSleep</td>
*     <td>DeepSleep</td>
*     <td>The entire chip is in a DeepSleep, since it forces the MHz
*         clocks, DSI, UDBs, most peripherals, analog, etc., to be shut down.
*         Only ILO/WCO may operate, limited peripherals.</td>
*   </tr>
*   <tr>
*     <td>-</td>
*     <td>-</td>
*     <td>Hibernate</td>
*     <td>Once Hibernate is set, everything goes into the hibernate power mode.
*         Only ILO may operate to assert wake-up device using WDT or RTC.</td>
*   </tr>
* </table>
*
* (*1) While device switch into LPActive power mode the SysPm driver does not 
* decrease clocks or peripheral. SysPm driver only switch the supply regulators 
* into low power mode. It is user responsibility to configure clocks
* or/and peripheral to meet current load limitation in LPActive power mode.
* For more details about power modes and current load limitations refer to
* the device Technical Reference Manual (TRM).
*
* (*2) In condition that CM4 is presented in device.
*
* Power modes:
* * Active - In this mode the CPU executes code, and all logic and memories
* are powered. Firmware may decide to disable clocks for specific peripherals
* and to power down specific analog power domains. The chip enters Active upon
* initial reset. For internally-generated resets, the chip enters the most
* recently used power mode among Active and LPActive.
*
* * Sleep - In this mode the CPU does not execute code and its clock is
* stopped. It is identical to ACTIVE from a peripheral point of view.
*
* * LPActive - LPActive (Low Power Active) is similar to Active, with clock
* restrictions and limited/slowed peripheral to achieve a lower system current.
*
* * LPSleep - LPSleep (Low Power Sleep) mode operates the same as Sleep mode
* with the clocking limitations of LPActive mode to further reduce system power.
*
* * DeepSleep - In the deep sleep mode the main system clock is off for CPU, 
* Flash, and high-speed logic. Optionally, the low frequency (32.768 kHz) clock
* remains on, and low-frequency peripherals continue operating.
* Peripherals that do not need a clock or receive a clock from their external
* interface (e.g., I2C/SPI) continue operating, and all circuits that draw
* current from the Vccdpslp supply are subject to the current sourcing
* limitations of the DeepSleep Regulator (if the core is supplied by the
* Linear core regulator). If the core is supplied by the SIMO Buck regulator all
* deep sleep peripherals are supplied by Vbuck1.
* A normal wakeup from the deep sleep returns to either LPActive, LPSleep, 
* Active, or Sleep, depending on the previous state and programmed behavior for
* the configured wakeup interrupt.
* Likewise, a debug wakeup from deep sleep returns to LPSleep or Sleep, 
* depending on which mode was used to enter the deep sleep power mode.
*
* * Hibernate - Is the lowest power mode available when external supplies are
* present. It is intended for applications in a dormant state.
* Hibernate is implemented using high voltage logic and switches off all
* internal regulators. The Backup domain continues functioning normally.
* The I/O's state is frozen so that the output driver state is held.
* Note in this mode the CPU and all peripherals lose state, so the system and
* firmware reboot on a wakeup event.
* Wakeup is supported from device specific pin(s) with programmable
* polarity. Additionally, unregulated peripherals can wake the device under
* some conditions. For example, a low-power comparator can wake the device by
* comparing two external voltages but may not support comparison to an
* internally-generated voltage. The Backup domain remains functional, and if
* present it can schedule an alarm to wake the device from Hibernate using RTC.
* The combination of Hibernate and Backup alarms can yield low current duty
* cycling for applications, such as waking once per second for a CapSense
* scan. Alternatively, the Watchdog Timer (WDT) can be configured to wake-up the
* device by WDT interrupt.
* Backup memory (BMEM) can be used to store system state for use on the
* next reboot.
* 
* The system can wakeup from hibernate power mode by configuring:
* * Wakeup pins
* * WDT interrupt
* * LPComparator
* * WDT reset - this can be configured without using the syspm driver. Just
* configure and enable the WDT before entering the hibernate power mode. All
* other events require use of the API from the syspm driver.
*
* * XRES - is the state of the part when XRES is applied. This is obviously a
* reset condition, but it is also considered a power mode because some low-power
* applications, such as remote controls, expect very low current consumption
* when XRES is asserted.
*
* * OFF - The OFF state simply represents the state of the part with no power
* applied to it (except perhaps the Backup domain). In some applications, power
* to the device will be switched on and off  by another device in the system to
* make the product implement the lowest power consumption level.
*
* Wakeup time after a reset is device-dependent because of the boot run-time.
* The wakeup time varies depending on the number and type of trim parameters and
* the size of device Flash.
*
* For ARM-based devices, an interrupt is required for the CPU to
* wake up.
*
* * All pending interrupts should be cleared before the device is put into a
* low-power mode, even if they are masked.
*
* For more details about power modes switching and debug, refer to
* the device Technical Reference Manual (TRM).
*
* \section group_syspm_section_configuration Configuration Considerations
* After initial device boot the device goes into Active power mode. After some
* code execution the device could be sent into low power mode.
*
* For multi-core devices the Cy_SysPm_Sleep() and Cy_SysPm_DeepSleep() 
* functions only switch the core that calls the function into the sleep or the 
* deep sleep power mode. To set the whole device the sleep or the deep sleep 
* power mode, ensure that each core called the Cy_SysPm_Sleep() or
* Cy_SysPm_DeepSleep() functions.
* There are situations when the device does not switch into the deep sleep power
* mode immediately after the second core calls Cy_SysPm_DeepSleep(). The device
* will switch into the deep sleep mode automatically a little bit later, 
* after the low-power circuits are ready to switch into the deep sleep.
* Refer to the Cy_SysPm_DeepSleep() description for more details.
*
* If you call Cy_SysPm_Hibernate() the device will be switched
* into the hibernate power mode directly, as there is no handshake 
* between cores.
*
* Before switching into LPActive power mode ensure that the device meets the 
* current load limitation by decreasing the Mhz clocks, and slowing or disabling
* peripherals.
* Refer to the device TRM for more details about current load limitations in
* different low-power modes.
*
* The syspm driver implements a syspm callbacks mechanism. Using a
* syspm callback, you can perform actions right before entering
* low-power mode, and directly after switching out of low-power mode. With the 
* callback mechanism you can prevent switching into low power mode if, for 
* example, some required peripheral is not ready to enter into requested 
* low-power mode.
* 
* Fill in the cy_stc_syspm_callback_t structure for each callback function. 
* Each callback function is of the type Cy_SysPmCallback. You assign
* the mode for each callback function. Refer to cy_en_syspm_callback_mode_t.
* Also refer to Cy_SysPm_Sleep(), Cy_SysPm_DeepSleep(), Cy_SysPm_EnterLpMode(),
* Cy_SysPm_ExitLpMode(), Cy_SysPm_Hibernate() for more details.
*
* Refer to Cy_SysPm_RegisterCallback() description for more details
* about registering a callback. You may have up to 32 callback functions.
*
* \note The function callback type Cy_SysPmCallback requires implementing
* the function with \ref cy_stc_syspm_callback_params_t parameter. Note that 
* elements of this structure *base and *context are optional and could be not 
* used inside the function.
*
* The syspm driver implements the functions to configure the external power 
* management integrated circuit (PMIC).
*
* \section group_syspm_section_more_information More Information
*
* For more information on the Power Modes (syspm) driver,
* refer to the Technical Reference Manual (TRM).
*
* \section group_syspm_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>7.1.0</td>
*     <td>Added all 10 PIN wakeup support for Hibernate mode, 
*         Removed SIMO Buck related stuffs as not supported by TVII devices</td>
*     <td>Corrections and updates</td>
*   </tr>
* </table>
*
* \defgroup group_syspm_macros Macros
* \defgroup group_syspm_functions Functions
* \defgroup group_syspm_data_structures Data Structures
* \defgroup group_syspm_data_enumerates Enumerated types
*/

#ifndef _CY_SYSPM_H_
#define _CY_SYSPM_H_

#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*       Register Constants
*******************************************************************************/

/**
* \addtogroup group_syspm_macros
* \{
*/

/** Driver major version */
#define CY_SYSPM_DRV_VERSION_MAJOR       2

/** Driver minor version */
#define CY_SYSPM_DRV_VERSION_MINOR       0

/** \cond INTERNAL */

/** The internal define of the delay for the SysPm_DeepSleep() function */
#define CY_SYSPM_PWR_TRIM_WAKE_CNTS                      (0x8Bu)

/** The internal define of the unlock value for the PMIC functions */
#define CY_SYSPM_PMIC_UNLOCK_KEY                         (0x3Au)

/** The internal define of the tries number in the Cy_SysPm_ExitLpMode() 
* function
 */
#define CY_SYSPM_WAIT_DELAY_TRYES                        (100u)

/** The internal define of the wakeup pin bit used in the 
* Cy_SysPm_SetHibWakeupSource() function
* Note: Check device datasheet to see how many wakeup pins are supported
* and use based on the number.
*/
#define CY_SYSPM_WAKEUP_PIN0_BIT                          (1ul)
#define CY_SYSPM_WAKEUP_PIN1_BIT                          (2ul)
#define CY_SYSPM_WAKEUP_PIN2_BIT                          (3ul)
#define CY_SYSPM_WAKEUP_PIN3_BIT                          (4ul)
#define CY_SYSPM_WAKEUP_PIN4_BIT                          (5ul)
#define CY_SYSPM_WAKEUP_PIN5_BIT                          (6ul)
#define CY_SYSPM_WAKEUP_PIN6_BIT                          (7ul)
#define CY_SYSPM_WAKEUP_PIN7_BIT                          (8ul)
#define CY_SYSPM_WAKEUP_PIN8_BIT                          (9ul)
#define CY_SYSPM_WAKEUP_PIN9_BIT                          (10ul)  

/**
* The internal define of the wake-up pin value 
* used in the Cy_SysPm_SetHibWakeupSource() function
*/
#define CY_SYSPM_WAKEUP_PIN0       ((uint32_t) CY_SYSPM_WAKEUP_PIN0_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN1       ((uint32_t) CY_SYSPM_WAKEUP_PIN1_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN2       ((uint32_t) CY_SYSPM_WAKEUP_PIN2_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN3       ((uint32_t) CY_SYSPM_WAKEUP_PIN3_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN4       ((uint32_t) CY_SYSPM_WAKEUP_PIN4_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN5       ((uint32_t) CY_SYSPM_WAKEUP_PIN5_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN6       ((uint32_t) CY_SYSPM_WAKEUP_PIN6_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN7       ((uint32_t) CY_SYSPM_WAKEUP_PIN7_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN8       ((uint32_t) CY_SYSPM_WAKEUP_PIN8_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN9       ((uint32_t) CY_SYSPM_WAKEUP_PIN9_BIT << \
                                               SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)

/** The internal define for the wake-up pin polarity configuration */
#define CY_SYSPM_WAKEUP_PIN0_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN0_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN1_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN1_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN2_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN2_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN3_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN3_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN4_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN4_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN5_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN5_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN6_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN6_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN7_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN7_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN8_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN8_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)
#define CY_SYSPM_WAKEUP_PIN9_POLARITY_HIGH      ((uint32_t) CY_SYSPM_WAKEUP_PIN9_BIT << \
                                                             SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos)

/** \endcond */

/** syspm driver identifier */
#define CY_SYSPM_ID                             (CY_PDL_DRV_ID(0x10u))

/**
* \defgroup group_syspm_return_status The Power Mode Status Defines
* \{
* The defines of the core(s) and device power mode status.
*/
#if (__CORTEX_M == 0) || (defined (__CM0P_PRESENT) && (__CM0P_PRESENT == 1))

    /** The CM0p is Active */
    #define CY_SYSPM_STATUS_CM0_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 0u))

    /** The CM0p is in Sleep */
    #define CY_SYSPM_STATUS_CM0_SLEEP        ((uint32_t) ((uint32_t)0x02u << 0u))

    /** The CM0p is in DeepSleep */
    #define CY_SYSPM_STATUS_CM0_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 0u))

    /** The CM0p is in low power mode */
    #define CY_SYSPM_STATUS_CM0_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 0u))

#endif

#ifdef CY_IP_M4CPUSS

    /** The CM4 is Active */
    #define CY_SYSPM_STATUS_CM4_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 4u))

    /** The CM4 is in Sleep */
    #define CY_SYSPM_STATUS_CM4_SLEEP        ((uint32_t) ((uint32_t)0x02u << 4u))

    /** The CM4 is in DeepSleep */
    #define CY_SYSPM_STATUS_CM4_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 4u))

    /** The CM4 is Low Power mode */
    #define CY_SYSPM_STATUS_CM4_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 4u))
    
#endif /* CY_IP_M4CPUSS */

#ifdef CY_IP_M7CPUSS

    /** The CM7_0 is Active */
    #define CY_SYSPM_STATUS_CM7_0_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 4u))

    /** The CM7_0 is in Sleep */
    #define CY_SYSPM_STATUS_CM7_0_SLEEP        ((uint32_t) ((uint32_t)0x02u << 4u))

    /** The CM7_0 is in DeepSleep */
    #define CY_SYSPM_STATUS_CM7_0_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 4u))

    /** The CM7_0 is Low Power mode */
    #define CY_SYSPM_STATUS_CM7_0_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 4u))

    /** The CM7_1 is Active */
    #define CY_SYSPM_STATUS_CM7_1_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 8u))

    /** The CM7_1 is in Sleep */
    #define CY_SYSPM_STATUS_CM7_1_SLEEP        ((uint32_t) ((uint32_t)0x02u << 8u))

    /** The CM7_1 is in DeepSleep */
    #define CY_SYSPM_STATUS_CM7_1_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 8u))

    /** The CM7_1 is Low Power mode */
    #define CY_SYSPM_STATUS_CM7_1_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 8u))

    /** The CM7_2 is Active */
    #define CY_SYSPM_STATUS_CM7_2_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 12u))

    /** The CM7_2 is in Sleep */
    #define CY_SYSPM_STATUS_CM7_2_SLEEP        ((uint32_t) ((uint32_t)0x02u << 12u))

    /** The CM7_2 is in DeepSleep */
    #define CY_SYSPM_STATUS_CM7_2_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 12u))

    /** The CM7_2 is Low Power mode */
    #define CY_SYSPM_STATUS_CM7_2_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 12u))

    /** The CM7_3 is Active */
    #define CY_SYSPM_STATUS_CM7_3_ACTIVE       ((uint32_t) ((uint32_t)0x01u << 16u))

    /** The CM7_3 is in Sleep */
    #define CY_SYSPM_STATUS_CM7_3_SLEEP        ((uint32_t) ((uint32_t)0x02u << 16u))

    /** The CM7_3 is in DeepSleep */
    #define CY_SYSPM_STATUS_CM7_3_DEEPSLEEP    ((uint32_t) ((uint32_t)0x04u << 16u))

    /** The CM7_3 is Low Power mode */
    #define CY_SYSPM_STATUS_CM7_3_LOWPOWER     ((uint32_t) ((uint32_t)0x08u << 16u))

#endif /* CY_IP_M7CPUSS */

#ifdef CY_IP_M4CPUSS

    /** The device is in the low power mode define */
    #define CY_SYSPM_STATUS_SYSTEM_LOWPOWER    ((uint32_t) (CY_SYSPM_STATUS_CM0_LOWPOWER | \
                                                           (CY_SYSPM_STATUS_CM4_LOWPOWER)))
#elif CY_IP_M7CPUSS

    /** The device is in the low power mode define */
    #define CY_SYSPM_STATUS_SYSTEM_LOWPOWER    ((uint32_t) (CY_SYSPM_STATUS_CM0_LOWPOWER | \
                                                           (CY_SYSPM_STATUS_CM7_0_LOWPOWER) | \
                                                           (CY_SYSPM_STATUS_CM7_1_LOWPOWER)))
#else

    /** The device is in the low power mode define */
    #define CY_SYSPM_STATUS_SYSTEM_LOWPOWER    CY_SYSPM_STATUS_CM0_LOWPOWER

#endif /* CY_IP_M4CPUSS */

/* The define of retained power mode of the CM4 or CM7_x */
#define CY_SYSPM_CM4_7_PWR_CTL_PWR_MODE_RETAINED        (2ul)

/** \} group_syspm_return_status */

/** The define to update the token to indicate the transition into Hibernate */
#define CY_SYSPM_PWR_TOKEN_HIBERNATE      ((uint32_t) 0x1Bu << SRSS_PWR_HIBERNATE_TOKEN_Pos)

/** The wait time for transition of the device from the Active into 
* the LPActive
*/
#define CY_SYSPM_ACTIVE_TO_LP_WAIT_US           (5u)

/** The wait delay time which occurs before the Active reference is settled.
* This delay is used in transition of the device from Active into the LPActive 
* low-power mode 
*/
#define CY_SYSPM_LP_TO_ACTIVE_WAIT_BEFORE_US    (8u)

/** The wait delay time which occurs after the Active reference is settled. 
* This delay is used in transition the device from Active into the LPActive 
* low-power mode 
*/
#define CY_SYSPM_LP_TO_ACTIVE_WAIT_AFTER_US    (5u)

/** The maximum callbacks number */
#define CY_SYSPM_MAX_CALLBACKS_NUMBER         (32u)

/** The mask to unlock the Hibernate power mode */
#define CY_SYSPM_PWR_HIBERNATE_UNLOCK             ((uint32_t) 0x3Au << SRSS_PWR_HIBERNATE_UNLOCK_Pos)

/** The mask to set the Hibernate power mode */
#define CY_SYSPM_PWR_SET_HIBERNATE                 ((uint32_t) CY_SYSPM_PWR_HIBERNATE_UNLOCK |\
                                                                   SRSS_PWR_HIBERNATE_FREEZE_Msk |\
                                                                   SRSS_PWR_HIBERNATE_HIBERNATE_Msk)

/** The mask to retain the Hibernate power mode status */
#define CY_SYSPM_PWR_RETAIN_HIBERNATE_STATUS      ((uint32_t) SRSS_PWR_HIBERNATE_TOKEN_Msk |\
                                                              SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk |\
                                                              SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk |\
                                                              SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk |\
                                                              SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk)

/** The mask for the wakeup sources */
#define CY_SYSPM_PWR_WAKEUP_HIB_MASK              ((uint32_t) SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk |\
                                                              SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk |\
                                                              SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk |\
                                                              SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk)


/** The maximum value of the callbacks */
#define CY_SYSPM_PWR_MAX_PLL_NUMBER               (SRSS_NUM_PLL)

/**
* \defgroup group_syspm_skip_callback_modes The defines to skip the callbacks modes
* \{
* The defines of the syspm callbacks modes that can be skipped during execution.
* For more information about callbacks modes refer 
* to \ref cy_en_syspm_callback_mode_t.
*/
#define CY_SYSPM_SKIP_CHECK_READY                   (0x01u)   /**< The define to skip the check ready mode in the syspm callback */
#define CY_SYSPM_SKIP_CHECK_FAIL                    (0x02u)   /**< The define to skip the check fail mode in the syspm callback */
#define CY_SYSPM_SKIP_BEFORE_TRANSITION             (0x04u)   /**< The define to skip the before transition mode in the syspm callback */
#define CY_SYSPM_SKIP_AFTER_TRANSITION              (0x08u)   /**< The define to skip the after transition mode in the syspm callback */
/** \} group_syspm_skip_callback_modes */

#if(0)

    /** The definition for the delay of the SIMO Buck supply regulator 
    * stabilization after it was configured with enabled SIMO Buck output 1 */
    #define CY_SYSPM_SIMO_BUCK_CORE_SUPPLY_STABLE_US       (900u)

    /** The definition for the delay of the SIMO Buck supply regulator 
    * stabilization after it was configured with enabled SIMO Buck 
    * output 2 only
    */
    #define CY_SYSPM_SIMO_BUCK_BLE_SUPPLY_STABLE_US        (600u)

    /** The definition for the delay of the Linear Regulator after its output 
    * voltage is changed
    */
    #define CY_SYSPM_LDO_IS_STABLE_US                      (9u)

    /** The definition for the delay of the SIMO Buck regulator after its output
    * voltage is changed
    */
    #define CY_SYSPM_SIMO_BUCK_IS_STABLE_US                (200u)

#endif /* (0u != SRSS_SIMOBUCK_PRESENT) */

//rmkn: copied from the above group under SIMO buck.
/** The definition for the delay of the Linear Regulator after its output 
* voltage is changed
*/
#define CY_SYSPM_LDO_IS_STABLE_US                      (9u)

/** \} group_syspm_macros */

/*******************************************************************************
*       Configuration Structures
*******************************************************************************/

/**
* \addtogroup group_syspm_data_enumerates
* \{
*/

/** This enumeration is used to configure sources for wakeup from Hibernate 
*   power mode
*/
typedef enum
{
    /** Configure the RTC alarm */
    CY_SYSPM_HIBALARM = SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk,
    
    /** Configure the WDT interrupt */
    CY_SYSPM_HIBWDT = SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk,

    /** Configure a low level for the first wakeup-pin */
    CY_SYSPM_HIBPIN0_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN0_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
    /** Configure a high level for the first wakeup-pin */
    CY_SYSPM_HIBPIN0_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN0_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN0_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),

    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN1_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN1_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN1_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN1_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN1_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN2_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN2_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN2_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN2_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN2_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN3_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN3_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN3_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN3_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN3_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN4_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN4_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN4_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN4_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN4_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN5_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN5_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN5_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN5_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN5_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN6_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN6_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN6_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN6_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN6_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN7_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN7_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN7_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN7_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN7_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN8_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN8_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN8_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN8_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN8_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
    
    /** Configure a low level for the second wakeup-pin */
    CY_SYSPM_HIBPIN9_LOW = ((uint32_t) CY_SYSPM_WAKEUP_PIN9_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos),
    
   /** Configure a high level for the second wakeup-pin */
    CY_SYSPM_HIBPIN9_HIGH =
    ((uint32_t) ((uint32_t) CY_SYSPM_WAKEUP_PIN9_BIT << SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos) | 
                ((uint32_t) CY_SYSPM_WAKEUP_PIN9_BIT << SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos)),
} cy_en_syspm_hib_wakeup_source_t;

/**
* This enumeration is used to initialize a wait action - an interrupt or
* an event. Refer to the CMSIS for the WFE and WFI instruction explanations.
*/
typedef enum
{
    CY_SYSPM_WAIT_FOR_INTERRUPT,      /**< Wait for an interrupt */
    CY_SYSPM_WAIT_FOR_EVENT           /**< Wait for an event */
} cy_en_syspm_waitfor_t;

/** The enumeration is used to select output voltage for the Linear Regulator */
typedef enum
{
    CY_SYSPM_LDO_VOLTAGE_0_9V = 0x07u,         /**< 0.9 V nominal core voltage */
    CY_SYSPM_LDO_VOLTAGE_1_1V = 0x17u          /**< 1.1 V nominal core voltage */
} cy_en_syspm_ldo_voltage_t;

#if(0)

    /**
    *  The enumeration is used to select the output voltage for the SIMO Buck 
    *  output 1, which can supply a core(s).
    */
    typedef enum
    {
        CY_SYSPM_SIMO_BUCK_OUT1_VOLTAGE_0_9V = 0x01u,    /**< 0.9 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT1_VOLTAGE_1_1V = 0x05u     /**< 1.1 V nominal core voltage */
    } cy_en_syspm_simo_buck_voltage1_t;

    /**
    * The enumeration is used to select the output voltage for the SIMO Buck 
    * output 2, which can BLE HW block.
    */
    typedef enum
    {
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_0_95V = 0u,    /**< 0.95 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_0V,          /**< 1.0 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_05V,         /**< 1.05 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_1V,          /**< 1.1 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_2V,          /**< 1.2 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_3V,          /**< 1.3 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_4V,          /**< 1.4 V nominal core voltage */
        CY_SYSPM_SIMO_BUCK_OUT2_VOLTAGE_1_5V,          /**< 1.5 V nominal core voltage */
    } cy_en_syspm_simo_buck_voltage2_t;

    /**
    * The enumerations are used to select the SIMO Buck regulator outputs
    */
    typedef enum
    {
        CY_SYSPM_BUCK_VBUCK_1,    /**< SIMO Buck output 1 Voltage (Vback1) */
        CY_SYSPM_BUCK_VRF         /**< SIMO Buck out 2 Voltage (Vbackrf) */
    } cy_en_syspm_buck_out_t;

#endif /* (0u != SRSS_SIMOBUCK_PRESENT) */

/** The SysPm status definitions */
typedef enum
{
    CY_SYSPM_SUCCESS       = 0x00u,                                        /**< Successful */
    CY_SYSPM_BAD_PARAM     = CY_SYSPM_ID | CY_PDL_STATUS_ERROR | 0x01u,    /**< One or more invalid parameters */
    CY_SYSPM_TIMEOUT       = CY_SYSPM_ID | CY_PDL_STATUS_ERROR | 0x02u,    /**< A time-out occurs */
    CY_SYSPM_INVALID_STATE = CY_SYSPM_ID | CY_PDL_STATUS_ERROR | 0x03u,    /**< The operation is not setup or is in an
                                                                                improper state */
    CY_SYSPM_FAIL          = CY_SYSPM_ID | CY_PDL_STATUS_ERROR | 0xFFu     /**< An unknown failure */
} cy_en_syspm_status_t;

/**
* This enumeration is used for selecting the low power mode on which the
* appropriate registered callback handler will be executed. For example,
* the registered callback of the type CY_SYSPM_SLEEP will be executed while
* switching into the Sleep power mode.
*/
typedef enum
{
    CY_SYSPM_SLEEP,         /**< The Sleep enum callback type */
    CY_SYSPM_DEEPSLEEP,     /**< The DeepSleep enum callback type */
    CY_SYSPM_HIBERNATE,     /**< The Hibernate enum callback type */
    CY_SYSPM_ENTER_LP_MODE, /**< The enter into the LPActive mode enum callback type */
    CY_SYSPM_EXIT_LP_MODE,  /**< The exit out of the LPActive mode enum callback type */
} cy_en_syspm_callback_type_t;

/** The callback mode enumeration. This enum defines the callback mode */
typedef enum
{
    CY_SYSPM_CHECK_READY        = 0x01u,    /**< Callbacks with this mode are executed before entering into the 
                                                 low-power mode. Callback function check if the device is ready
                                                 to enter the low-power mode */
    CY_SYSPM_CHECK_FAIL         = 0x02u,    /**< Callbacks with this mode are executed after the previous callbacks
                                                 execution with CY_SYSPM_CHECK_READY return CY_SYSPM_FAIL. The callback
                                                 with the CY_SYSPM_CHECK_FAIL mode should roll back the actions done in
                                                 the callbacks executed previously with CY_SYSPM_CHECK_READY */
    CY_SYSPM_BEFORE_ENTER  = 0x04u,    /**< The actions to be done before entering into the low-power mode */
    CY_SYSPM_AFTER_EXIT   = 0x08u,    /**< The actions to be done after exiting the low-power mode */
} cy_en_syspm_callback_mode_t;

/** \} group_syspm_data_enumerates */

/**
* \addtogroup group_syspm_data_structures
* \{
*/

/** The structure with the syspm callback parameters */
typedef struct
{
    cy_en_syspm_callback_mode_t mode;   /**< The callback mode, element for internal usage, see 
                                             \ref cy_en_syspm_callback_mode_t. This element should not be defined 
                                             as it is updated every time before the callback function is executed */
    void *base;                         /**< The base address of a HW instance, matches name of the driver in
                                             the API for the base address. Can be not defined if not required */
    void *context;                      /**< The context for the handler function. This item can be 
                                             skipped if not required. Can be not defined if not required */

} cy_stc_syspm_callback_params_t;

/** The type for syspm callbacks */
typedef cy_en_syspm_status_t (*Cy_SysPmCallback) (cy_stc_syspm_callback_params_t *callbackParams);

/** The structure with the syspm callback configuration elements */
typedef struct cy_stc_syspm_callback
{
    Cy_SysPmCallback callback;                         /**< The callback handler function */
    cy_en_syspm_callback_type_t type;                  /**< The callback type, see \ref cy_en_syspm_callback_type_t */
    uint32_t skipMode;                                 /**< The mask of modes to be skipped during callback 
                                                             execution, see \ref group_syspm_skip_callback_modes. The 
                                                             corresponding callback mode won't execute if the 
                                                             appropriate define is set. These values can be ORed.   
                                                             If all modes are required to be executed this element
                                                             should be equal to zero. */
    cy_stc_syspm_callback_params_t *callbackParams;    /**< The base address of a HW instance, matches name of the 
                                                            driver in the API for the base address */

    struct cy_stc_syspm_callback *prevItm;              /**< The previous list item. This element is for internal usage
                                                             to link this structure to the next registered structure. 
                                                             It will be updated during callback registering */
                                                             
    struct cy_stc_syspm_callback *nextItm;              /**< The next list item. This element is for internal usage to 
                                                             link this structure to the previous registered structure.
                                                             It will be updated during callback registering */
} cy_stc_syspm_callback_t;


/**
* Selects the voltage threshold for OVD on VDDD
*/
typedef enum
{
    CY_SYSPM_OVD_VDDD_5_5V      = 0x0u,                 /**< OVD -> VDDD > 5.5 V */
    CY_SYSPM_OVD_VDDD_5_0V      = 0x01u   	        /**< OVD -> VDDD > 5.0 V  */
} cy_en_syspm_ovd_vddd_sel_t;

/**
* Selects the voltage threshold for OVD on VDDA
*/
typedef enum
{
    CY_SYSPM_OVD_VDDA_5_5V      = 0x0u,	                /**< OVD -> VDDA > 5.5 V */
    CY_SYSPM_OVD_VDDA_5_0V      = 0x01u                 /**< OVD -> VDDA > 5.0 V  */
} cy_en_syspm_ovd_vdda_sel_t;

/**
* Selects OVD over VDDD or VDDA or VCCD
*/
typedef enum
{
    CY_SYSPM_OVD_ON_VDDD        = 0x0u,                 /**< OVD -> VDDD */
    CY_SYSPM_OVD_ON_VDDA        = 0x01u,                /**< OVD -> VDDA */
    CY_SYSPM_OVD_ON_VCCD        = 0x02u                 /**< OVD -> VCCD */
} cy_en_syspm_ovd_sel_t;

/**
 * OVD action select configuration.
 */
typedef enum
{
    CY_SYSPM_OVD_ACTION_NOTHING = 0x0u,  	        /**<Select OVD action select: nothing */
    CY_SYSPM_OVD_ACTION_FAULT  	= 0x1u,  	        /**<Select OVD action select: fault */
    CY_SYSPM_OVD_ACTION_RESET   = 0x2u,  	        /**<Select OVD action select: reset */
} cy_en_syspm_ovd_action_select_t;

/**
* Selects the voltage threshold for BOD on VDDD
*/
typedef enum
{
    CY_SYSPM_BOD_VDDD_2_7V      = 0x0u,                 /**< BOD -> VDDD < 2.7 V */
    CY_SYSPM_BOD_VDDD_3_0V      = 0x01u   	        /**< BOD -> VDDD < 3.0 V  */
} cy_en_syspm_bod_vddd_sel_t;

/**
* Selects the voltage threshold for BOD on VDDA
*/
typedef enum
{
    CY_SYSPM_BOD_VDDA_2_7V      = 0x0u,	                /**< BOD -> VDDA < 2.7 V */
    CY_SYSPM_BOD_VDDA_3_0V      = 0x01u                 /**< BOD -> VDDA < 3.0 V  */
} cy_en_syspm_bod_vdda_sel_t;

/**
* Selects BOD over VDDD or VDDA or VCCD
*/
typedef enum
{
    CY_SYSPM_BOD_ON_VDDD        = 0x0u,                 /**< BOD -> VDDD */
    CY_SYSPM_BOD_ON_VDDA        = 0x01u,                /**< BOD -> VDDA */
    CY_SYSPM_BOD_ON_VCCD        = 0x02u                 /**< BOD -> VCCD */
} cy_en_syspm_bod_sel_t;

/**
 * BOD action select configuration.
 */
typedef enum
{
    CY_SYSPM_BOD_ACTION_NOTHING = 0x0u,  	        /**<Select BOD action select: nothing */
    CY_SYSPM_BOD_ACTION_FAULT  	= 0x1u,  	        /**<Select BOD action select: fault */
    CY_SYSPM_BOD_ACTION_RESET   = 0x2u,  	        /**<Select BOD action select: reset */
} cy_en_syspm_bod_action_select_t;

/**
 * Supply Supervision Status
 */
typedef enum
{
    CY_SYSPM_ENTITY_BOD_VDDD    = 0x0u,  	        /**<BOD indicates VDDD is OK */
    CY_SYSPM_ENTITY_BOD_VDDA  	= 0x1u,  	        /**<BOD indicates VDDA is OK */
    CY_SYSPM_ENTITY_BOD_VCCD    = 0x2u,  	        /**<BOD indicates VCCD is OK */
    CY_SYSPM_ENTITY_OVD_VDDD    = 0x3u,  	        /**<OVD indicates VDDD is OK */
    CY_SYSPM_ENTITY_OVD_VDDA  	= 0x4u,  	        /**<OVD indicates VDDA is OK */
    CY_SYSPM_ENTITY_OVD_VCCD    = 0x5u,  	        /**<OVD indicates VCCD is OK */
} cy_en_syspm_supply_entity_select_t;

/** \} group_syspm_data_structures */


#define CY_OVD_CHECK_ACTION_CFG(ovdActionSelect)        (((ovdActionSelect) == CY_SYSPM_OVD_ACTION_NOTHING) || \
                                                         ((ovdActionSelect) == CY_SYSPM_OVD_ACTION_FAULT) || \
                                                         ((ovdActionSelect) == CY_SYSPM_OVD_ACTION_RESET)) 

#define CY_BOD_CHECK_ACTION_CFG(bodActionSelect)        (((bodActionSelect) == CY_SYSPM_BOD_ACTION_NOTHING) || \
                                                         ((bodActionSelect) == CY_SYSPM_BOD_ACTION_FAULT) || \
                                                         ((bodActionSelect) == CY_SYSPM_BOD_ACTION_RESET)) 

#define CY_OVD_CHECK_SUPPLY_ENTITY(supplyEntitySelect)  (((supplyEntitySelect) == CY_SYSPM_ENTITY_BOD_VDDD) || \
                                                         ((supplyEntitySelect) == CY_SYSPM_ENTITY_BOD_VDDA) || \
                                                         ((supplyEntitySelect) == CY_SYSPM_ENTITY_BOD_VCCD) || \
                                                         ((supplyEntitySelect) == CY_SYSPM_ENTITY_OVD_VDDD) || \
                                                         ((supplyEntitySelect) == CY_SYSPM_ENTITY_OVD_VDDA) || \
                                                         ((supplyEntitySelect) == CY_SYSPM_ENTITY_OVD_VCCD))

/**
* \addtogroup group_syspm_functions
* \{
*/

#ifdef CY_IP_M4CPUSS

    __STATIC_INLINE bool Cy_SysPm_Cm4IsActive(void);
    __STATIC_INLINE bool Cy_SysPm_Cm4IsSleep(void);
    __STATIC_INLINE bool Cy_SysPm_Cm4IsDeepSleep(void);
    __STATIC_INLINE bool Cy_SysPm_Cm4IsLowPower(void);

#endif /* CY_IP_M4CPUSS */
    
#ifdef CY_IP_M7CPUSS

    __STATIC_INLINE bool Cy_SysPm_Cm7IsActive(uint8_t core);
    __STATIC_INLINE bool Cy_SysPm_Cm7IsSleep(uint8_t core);
    __STATIC_INLINE bool Cy_SysPm_Cm7IsDeepSleep(uint8_t core);
    __STATIC_INLINE bool Cy_SysPm_Cm7IsLowPower(uint8_t core);

#endif /* CY_IP_M4CPUSS */

__STATIC_INLINE bool Cy_SysPm_Cm0IsActive(void);
__STATIC_INLINE bool Cy_SysPm_Cm0IsSleep(void);
__STATIC_INLINE bool Cy_SysPm_Cm0IsDeepSleep(void);
__STATIC_INLINE bool Cy_SysPm_Cm0IsLowPower(void);
__STATIC_INLINE bool Cy_SysPm_IsLowPower(void);

__STATIC_INLINE cy_en_syspm_ldo_voltage_t Cy_SysPm_LdoGetVoltage(void);
__STATIC_INLINE bool Cy_SysPm_LdoIsEnabled(void);
__STATIC_INLINE bool Cy_SysPm_GetIoFreezeStatus(void);

__STATIC_INLINE void Cy_SysPm_EnablePmic(void);
__STATIC_INLINE void Cy_SysPm_DisablePmic(uint32_t polarity); 
__STATIC_INLINE void Cy_SysPm_AlwaysEnablePmic(void);
__STATIC_INLINE void Cy_SysPm_EnablePmicOutput(void);
__STATIC_INLINE void Cy_SysPm_DisablePmicOutput(void); 
__STATIC_INLINE void Cy_SysPm_LockPmic(void);
__STATIC_INLINE void Cy_SysPm_UnlockPmic(void);
__STATIC_INLINE bool Cy_SysPm_IsPmicEnabled(void);
__STATIC_INLINE bool Cy_SysPm_IsPmicOutputEnabled(void);
__STATIC_INLINE bool Cy_SysPm_IsPmicLocked(void);

#if(0u != SRSS_SIMOBUCK_PRESENT)

    __STATIC_INLINE cy_en_syspm_simo_buck_voltage1_t Cy_SysPm_SimoBuckGetVoltage1(void);
    __STATIC_INLINE cy_en_syspm_simo_buck_voltage2_t Cy_SysPm_SimoBuckGetVoltage2(void);
    __STATIC_INLINE void Cy_SysPm_DisableVoltage2(void);
    __STATIC_INLINE bool Cy_SysPm_SimoBuckIsEnabled(void);
    __STATIC_INLINE void Cy_SysPm_SimoBuckSetHwControl(bool hwControl);
    __STATIC_INLINE bool Cy_SysPm_SimoBuckGetHwControl(void);

    void Cy_SysPm_SwitchToSimoBuck(void);
    void Cy_SysPm_SimoBuckSetVoltage1(cy_en_syspm_simo_buck_voltage1_t voltage);
    void Cy_SysPm_EnableVoltage2(void);
    void Cy_SysPm_SimoBuckSetVoltage2(cy_en_syspm_simo_buck_voltage2_t voltage, bool waitToSettle);
    bool Cy_SysPm_SimoBuckOutputIsEnabled(cy_en_syspm_buck_out_t output);

#endif /* (0u != SRSS_SIMOBUCK_PRESENT) */

uint32_t Cy_SysPm_ReadStatus(void);
cy_en_syspm_status_t Cy_SysPm_Sleep(cy_en_syspm_waitfor_t waitFor);
cy_en_syspm_status_t Cy_SysPm_DeepSleep(cy_en_syspm_waitfor_t waitFor);
cy_en_syspm_status_t Cy_SysPm_Hibernate(void);
cy_en_syspm_status_t Cy_SysPm_EnterLpMode(void);
cy_en_syspm_status_t Cy_SysPm_ExitLpMode(void);
void Cy_SysPm_SleepOnExit(bool enable);

void Cy_SysPm_SetHibWakeupSource(cy_en_syspm_hib_wakeup_source_t wakeupSource);
void Cy_SysPm_ClearHibWakeupSource(cy_en_syspm_hib_wakeup_source_t wakeupSource);
void Cy_SysPm_LdoSetVoltage(cy_en_syspm_ldo_voltage_t voltage);
bool Cy_SysPm_RegisterCallback(cy_stc_syspm_callback_t* handler);
bool Cy_SysPm_UnregisterCallback(cy_stc_syspm_callback_t* handler);
cy_en_syspm_status_t Cy_SysPm_ExecuteCallback(cy_en_syspm_callback_type_t type, cy_en_syspm_callback_mode_t mode);
void Cy_SysPm_IoFreeze(void);
void Cy_SysPm_IoUnfreeze(void);

__STATIC_INLINE void Cy_SysPm_OvdEnable(cy_en_syspm_ovd_sel_t ovdSel);
__STATIC_INLINE void Cy_SysPm_OvdDisable(cy_en_syspm_ovd_sel_t ovdSel);
__STATIC_INLINE void Cy_SysPm_OvdVdddSelect(cy_en_syspm_ovd_vddd_sel_t ovdVdddSel);
__STATIC_INLINE void Cy_SysPm_OvdVddaSelect(cy_en_syspm_ovd_vdda_sel_t ovdVddaSel);
__STATIC_INLINE void Cy_Syspm_OvdActionSelect(cy_en_syspm_ovd_action_select_t ovdActionSelect);

__STATIC_INLINE void Cy_SysPm_BodEnable(cy_en_syspm_bod_sel_t bodSel);
__STATIC_INLINE void Cy_SysPm_BodDisable(cy_en_syspm_bod_sel_t bodSel);
__STATIC_INLINE void Cy_SysPm_BodVdddSelect(cy_en_syspm_bod_vddd_sel_t bodVdddSel);
__STATIC_INLINE void Cy_SysPm_BodVddaSelect(cy_en_syspm_bod_vdda_sel_t bodVddaSel);
__STATIC_INLINE void Cy_Syspm_BodActionSelect(cy_en_syspm_bod_action_select_t bodActionSelect);

__STATIC_INLINE bool Cy_Syspm_SupplySupervisionStatus(cy_en_syspm_supply_entity_select_t supplyEntitySelect);

#ifdef CY_IP_M4CPUSS
    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm4IsActive
    ****************************************************************************//**
    *
    * Checks if CM4 is in the active mode.
    *
    * \return
    * true - if CM4 is in the active mode, false - if the CM4 is not in active mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm4IsActive(void)
    {
        return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM4_ACTIVE) != 0u);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm4IsSleep
    ****************************************************************************//**
    *
    * Checks if the CM4 is in the sleep mode.
    *
    * \return
    * true - if the CM4 is in the sleep mode, 
    * false - if the CM4 is not in the sleep mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm4IsSleep(void)
    {
        return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM4_SLEEP) != 0u);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm4IsDeepSleep
    ****************************************************************************//**
    *
    * Checks if the CM4 is in the deep sleep mode.
    *
    * \return
    * true - if CM4 is in the deep sleep mode, false - if the CM4 is not in 
    * the deep sleep mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm4IsDeepSleep(void)
    {
        return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM4_DEEPSLEEP) != 0u);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm4IsLowPower
    ****************************************************************************//**
    *
    * Checks if the CM4 is in the low power mode.
    *
    * \return
    * true - if the CM4 is in the low power mode, 
    * false - if the CM4 is not in the low power mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm4IsLowPower(void)
    {
        return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM4_LOWPOWER) != 0u);
    }

#endif /* CY_IP_M4CPUSS */
    
#ifdef CY_IP_M7CPUSS
    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm7IsActive
    ****************************************************************************//**
    *
    * Checks if CM7 is in the active mode.
    *
    * \return
    * true - if CM7 is in the active mode, false - if the CM7 is not in active mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm7IsActive(uint8_t core)
    {
        bool status = false;
        
	if(core == CORE_CM7_0)
	{           
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_0_ACTIVE) != 0u);
        } 
	else if(core == CORE_CM7_1)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_1_ACTIVE) != 0u);
	}        
#if defined (tviibh16m)
	else if(core == CORE_CM7_2)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_2_ACTIVE) != 0u);
	} 
	else if(core == CORE_CM7_3)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_3_ACTIVE) != 0u);
	} 
#endif        
        return status;
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm7IsSleep
    ****************************************************************************//**
    *
    * Checks if the CM4 is in the sleep mode.
    *
    * \return
    * true - if the CM7 is in the sleep mode, 
    * false - if the CM7 is not in the sleep mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm7IsSleep(uint8_t core)
    {
        bool status = false;
        
	if(core == CORE_CM7_0)
	{           
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_0_SLEEP) != 0u);
        } 
	else if(core == CORE_CM7_1)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_1_SLEEP) != 0u);
	}   
#if defined (tviibh16m)        
	else if(core == CORE_CM7_2)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_2_SLEEP) != 0u);
	}  
        else if(core == CORE_CM7_3)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_3_SLEEP) != 0u);
	}  
#endif
        return status;
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm7IsDeepSleep
    ****************************************************************************//**
    *
    * Checks if the CM7 is in the deep sleep mode.
    *
    * \return
    * true - if CM7 is in the deep sleep mode, false - if the CM7 is not in 
    * the deep sleep mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm7IsDeepSleep(uint8_t core)
    {
        bool status = false;
        
	if(core == CORE_CM7_0)
	{           
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_0_DEEPSLEEP) != 0u);
        } 
	else if(core == CORE_CM7_1)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_1_DEEPSLEEP) != 0u);
	}        
#if defined (tviibh16m)        
	else if(core == CORE_CM7_2)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_2_DEEPSLEEP) != 0u);
	}
	else if(core == CORE_CM7_3)
	{            
            status = ((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_3_DEEPSLEEP) != 0u);
	}
#endif        
        return status;
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_Cm4IsLowPower
    ****************************************************************************//**
    *
    * Checks if the CM7 is in the low power mode.
    *
    * \return
    * true - if the CM7 is in the low power mode, 
    * false - if the CM7 is not in the low power mode.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_Cm7IsLowPower(uint8_t core)
    {
        bool status = false;
        
	if(core == CORE_CM7_0)
	{           
            return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_0_LOWPOWER) != 0u);
        } 
	else if(core == CORE_CM7_1)
	{            
            return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_1_LOWPOWER) != 0u);
	}
#if defined (tviibh16m)        
	else if(core == CORE_CM7_2)
	{            
            return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_2_LOWPOWER) != 0u);
	}
	else if(core == CORE_CM7_3)
	{            
            return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM7_3_LOWPOWER) != 0u);
	}
#endif        
        return status;
    }

#endif /* CY_IP_M7CPUSS */


/*******************************************************************************
* Function Name: Cy_SysPm_Cm0IsActive
****************************************************************************//**
*
* Checks if the CM0+ is in the active mode.
*
* \return
* true - if the CM0+ is in the active mode, 
* false - if the CM0+ is not in the active mode.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_Cm0IsActive(void)
{
    return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM0_ACTIVE) != 0u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_Cm0IsSleep
****************************************************************************//**
*
* Checks if the CM0+ is in the sleep mode.
*
* \return
* true - if the CM0+ is in the sleep mode, 
* false - if the CM0+ is not in the sleep mode.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_Cm0IsSleep(void)
{
    return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM0_SLEEP) != 0u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_Cm0IsDeepSleep
****************************************************************************//**
*
* Checks if the CM0+ is in the deep sleep mode.
*
* \return
* true - if the CM0+ is in the deep sleep mode, 
* false - if the CM0+ is not in the deep sleep mode.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_Cm0IsDeepSleep(void)
{
    return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM0_DEEPSLEEP) != 0u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_Cm0IsLowPower
****************************************************************************//**
*
* Checks if the CM0+ is in the low power mode.
*
* \return
* true - if the CM0+ is in the low power mode, 
* false - if the CM0+ is not in the low power mode.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_Cm0IsLowPower(void)
{
    return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_CM0_LOWPOWER) != 0u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_IsLowPower
****************************************************************************//**
*
* Checks if the device is in the low power mode.
*
* \return
* true - the system is in the low power mode,
* false - the system is is not in the low power mode.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_IsLowPower(void)
{
    return((Cy_SysPm_ReadStatus() & CY_SYSPM_STATUS_SYSTEM_LOWPOWER) != 0u);
}

#if(0)
    /*******************************************************************************
    * Function Name: Cy_SysPm_SimoBuckIsEnabled
    ****************************************************************************//**
    *
    * Get the current status of the SIMO Buck regulator.
    *
    * \return
    * The current state of the SIMO Buck regulator. True if the SIMO Buck regulator
    * is enabled, false if it is disabled
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_SimoBuckIsEnabled(void)
    {
        return((0u !=_FLD2VAL(SRSS_PWR_BUCK_CTL_BUCK_EN, SRSS->unPWR_BUCK_CTL.u32Register)) ? true : false);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_SimoBuckGetVoltage2
    ****************************************************************************//**
    *
    * Get the current output 2 nominal voltage (Vbuckrf) of the SIMO Buck regulator.
    *
    * \note The actual device output 2 voltage (Vbuckrf) can be different from the 
    * nominal voltage because the actual voltage value depends on the conditions 
    * including the load current.
    *
    * \return
    * The nominal output voltage of the SIMO Buck regulator output 2 
    * voltage (Vbuckrf).
    * See \ref cy_en_syspm_simo_buck_voltage2_t.
    *
    *******************************************************************************/
    __STATIC_INLINE cy_en_syspm_simo_buck_voltage2_t Cy_SysPm_SimoBuckGetVoltage2(void)
    {
        uint32_t retVal;
        retVal = _FLD2VAL(SRSS_PWR_BUCK_CTL2_BUCK_OUT2_SEL, SRSS->unPWR_BUCK_CTL2.u32Register);

        return((cy_en_syspm_simo_buck_voltage2_t) retVal);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_SimoBuckGetVoltage1
    ****************************************************************************//**
    *
    * Get the current nominal output 1 voltage (Vccbuck1) of 
    * the SIMO Buck regulator.
    *
    * \note The actual device output 1 voltage (Vccbuck1) can be different from
    * the actual voltage because the actual voltage value depends on the 
    * conditions including the load current.
    *
    * \note The actual device output 1 voltage (Vccbuck1) can be different from the 
    * nominal voltage because the actual voltage value depends on the conditions
    * including the load current.
    *
    * \return
    * The nominal output voltage 1 (Vccbuck1) of the SIMO Buck regulator.
    * See \ref cy_en_syspm_simo_buck_voltage1_t.
    *
    *******************************************************************************/
    __STATIC_INLINE cy_en_syspm_simo_buck_voltage1_t Cy_SysPm_SimoBuckGetVoltage1(void)
    {
        uint32_t retVal;
        retVal = _FLD2VAL(SRSS_PWR_BUCK_CTL_BUCK_OUT1_SEL, SRSS->unPWR_BUCK_CTL.u32Register);

        return((cy_en_syspm_simo_buck_voltage1_t) retVal);
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_DisableVoltage2
    ****************************************************************************//**
    *
    * Disable the output 2 voltage (Vbuckrf) of the SIMO Buck regulator. The 
    * output 2 voltage (Vbuckrf) of the SIMO Buck regulator is used to supply 
    * the BLE HW block.
    *
    * \note The function does not have effect if the SIMO Buck regulator is
    * switched off.
    *
    * \note Ensure that the new voltage supply for the BLE HW block is settled 
    * and it is stable before calling the Cy_SysPm_DisableVoltage2() function.
    *
    *******************************************************************************/
    __STATIC_INLINE void Cy_SysPm_DisableVoltage2(void)
    {
        /* Disable the Vbuck2 output */
        SRSS->unPWR_BUCK_CTL2.u32Register &= ((uint32_t) ~ (_VAL2FLD(SRSS_PWR_BUCK_CTL2_BUCK_OUT2_EN, 1u)));
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_SimoBuckSetHwControl
    ****************************************************************************//**
    *
    * Sets the hardware control for the SIMO Buck output 2 (Vbuckrf).
    *
    * The hardware control for the Vbuckrf output. When this bit is set, the 
    * value in BUCK_OUT2_EN is ignored and hardware signal is used instead. If the 
    * product has supporting hardware, it can directly control the enable signal
    * for Vbuckrf.
    *
    * \param hwControl
    * Enables/disables hardware control for the SIMO Buck output 2
    *
    * Function does not have an effect if SIMO Buck regulator is disabled.
    *
    *******************************************************************************/
    __STATIC_INLINE void Cy_SysPm_SimoBuckSetHwControl(bool hwControl)
    {
        if(Cy_SysPm_SimoBuckIsEnabled())
        {
            if(hwControl)
            {
                SRSS->unPWR_BUCK_CTL2.u32Register |= _VAL2FLD(SRSS_PWR_BUCK_CTL2_BUCK_OUT2_HW_SEL, 1u);
            }
            else
            {
                SRSS->unPWR_BUCK_CTL2.u32Register &= ((uint32_t)~(_VAL2FLD(SRSS_PWR_BUCK_CTL2_BUCK_OUT2_HW_SEL, 1u)));
            }
        }
    }


    /*******************************************************************************
    * Function Name: Cy_SysPm_SimoBuckGetHwControl
    ****************************************************************************//**
    *
    * Gets the hardware control for Vbuckrf.
    *
    * The hardware control for the Vbuckrf output. When this bit is set, the 
    * value in BUCK_OUT2_EN is ignored and hardware signal is used instead. If the 
    * product has supporting hardware, it can directly control the enable signal
    * for Vbuckrf.
    *
    * \return
    * True if the HW control is set, false if the FW control is set for the 
    * SIMO Buck output 2.
    *
    *******************************************************************************/
    __STATIC_INLINE bool Cy_SysPm_SimoBuckGetHwControl(void)
    {
        return((0u != _FLD2VAL(SRSS_PWR_BUCK_CTL2_BUCK_OUT2_HW_SEL, SRSS->unPWR_BUCK_CTL2.u32Register)) ? true : false);
    }

#endif /* (0u != SRSS_SIMOBUCK_PRESENT) */

/*******************************************************************************
* Function Name: Cy_SysPm_LdoGetVoltage
****************************************************************************//**
*
* Get the current output voltage value of the Linear regulator.
*
* \note The actual device Vccd voltage can be different from the 
* nominal voltage because the actual voltage value depends on the conditions 
* including the load current.
*
* \return
* The nominal output voltage of the Linear regulator. 
* See \ref cy_en_syspm_ldo_voltage_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_syspm_ldo_voltage_t Cy_SysPm_LdoGetVoltage(void)
{
    uint32_t retVal;
    retVal = _FLD2VAL(SRSS_PWR_TRIM_PWRSYS_CTL_ACT_REG_TRIM, SRSS->unPWR_TRIM_PWRSYS_CTL.u32Register);
    
    return((cy_en_syspm_ldo_voltage_t) retVal);
}


/*******************************************************************************
* Function Name: Cy_SysPm_LdoIsEnabled
****************************************************************************//**
*
* Reads the current status of the Linear regulator.
*
* \return
* True means the Linear regulator is enabled. False means it is disabled
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_LdoIsEnabled(void)
{
    return((0u != _FLD2VAL(SRSS_PWR_CTL2_LINREG_DIS, SRSS->unPWR_CTL.u32Register)) ? false : true);
}


/*******************************************************************************
* Function Name: Cy_SysPm_GetIoFreezeStatus
****************************************************************************//**
*
* Checks whether IoFreeze is enabled.
*
* \return Returns True if IOs are frozen <br> False if IOs are unfrozen.
*
******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_GetIoFreezeStatus(void)
{
    return(0u != _FLD2VAL(SRSS_PWR_HIBERNATE_FREEZE, SRSS->unPWR_HIBERNATE.u32Register));
}


/*******************************************************************************
* Function Name: Cy_SysPm_EnablePmic
****************************************************************************//**
*
* Enable the external PMIC that supplies Vddd (if present).  
*
* Configure the PMIC input pin polarity and enable the signal through the PMIC
* output pin. The  PMIC input pin has programmable polarity to directly 
* interface to all types of external regulators.
*
* For information about the PMIC input and output pins and their assignment in 
* the specific families devices, refer to the appropriate device TRM.
* 
* The function is not effective when the PMIC is locked. Call 
* Cy_SysPm_UnlockPmic() before enabling the PMIC.
* 
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_EnablePmic(void)
{
    if(CY_SYSPM_PMIC_UNLOCK_KEY == _FLD2VAL(BACKUP_PMIC_CTL_UNLOCK, BACKUP->unPMIC_CTL.u32Register))
    {
        BACKUP->unPMIC_CTL.u32Register =
        _VAL2FLD(BACKUP_PMIC_CTL_UNLOCK, CY_SYSPM_PMIC_UNLOCK_KEY) |
        _VAL2FLD(BACKUP_PMIC_CTL_PMIC_EN_OUTEN, 1u) |
        _VAL2FLD(BACKUP_PMIC_CTL_PMIC_EN, 1u);
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_DisablePmic
****************************************************************************//**
*
* Disables the PMIC and keeps the output pin enabled. Also the function does 
* not change the PMIC input pin polarity.
*
* The function is not effective when the PMIC is locked. Call 
* Cy_SysPm_UnlockPmic() before enabling the PMIC.
*
* \param polarity
* Configures the PMIC wakeup input pin to be active low or active 
* high. The valid range is 0-1. 0 to set active low, and 1 to set active high.
*
* The function is not effective when the active level of PMIC input pin 
* is equal to configured PMIC polarity. 
*
* The PMIC will be enabled automatically by any of RTC alarm or PMIC wakeup 
* event regardless of the PMIC lock state.
*
* For information about the PMIC input and output pins and their assignment in 
* the specific families devices, refer to the appropriate device TRM.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_DisablePmic(uint32_t polarity)
{
    if(CY_SYSPM_PMIC_UNLOCK_KEY == _FLD2VAL(BACKUP_PMIC_CTL_UNLOCK, BACKUP->unPMIC_CTL.u32Register))
    {
        BACKUP->unPMIC_CTL.u32Register = 
        (_VAL2FLD(BACKUP_PMIC_CTL_UNLOCK, CY_SYSPM_PMIC_UNLOCK_KEY) | 
         _CLR_SET_FLD32U(BACKUP->unPMIC_CTL.u32Register, BACKUP_PMIC_CTL_POLARITY, polarity)) &
        ((uint32_t) ~ _VAL2FLD(BACKUP_PMIC_CTL_PMIC_EN, 1u));
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_AlwaysEnablePmic
****************************************************************************//**
*
* Configure the PMIC input pin polarity and enables the signal through the PMIC 
* output pin. The PMIC input pin has programmable polarity to directly 
* interface to all types of external regulators. This is a Write once API, 
* ensure that the PMIC cannot be disabled or polarity changed until a next 
* device reset.
* 
* For information about the PMIC input and output pins and their assignment in 
* the specific families devices, refer to the appropriate device TRM.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_AlwaysEnablePmic(void)
{
    BACKUP->unPMIC_CTL.u32Register |= _VAL2FLD(BACKUP_PMIC_CTL_PMIC_ALWAYSEN, 1u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_EnablePmicOutput
****************************************************************************//**
*
* Enable the PMIC output. 
*
* The function is not effective when the PMIC is locked. Call 
* Cy_SysPm_UnlockPmic() before enabling the PMIC.
*
* For information about the PMIC output pin and it assignment in 
* the specific families devices, refer to the appropriate device TRM.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_EnablePmicOutput(void)
{
    if(CY_SYSPM_PMIC_UNLOCK_KEY == _FLD2VAL(BACKUP_PMIC_CTL_UNLOCK, BACKUP->unPMIC_CTL.u32Register))
    {
        BACKUP->unPMIC_CTL.u32Register |=
        _VAL2FLD(BACKUP_PMIC_CTL_UNLOCK, CY_SYSPM_PMIC_UNLOCK_KEY) | _VAL2FLD(BACKUP_PMIC_CTL_PMIC_EN_OUTEN, 1u);
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_EnablePmicOutput
****************************************************************************//**
*
* Disable the PMIC output. 
*
* When PMIC output pin is disabled the pmic output pin can be used for the 
* another purpose.
*
* The function is not effective when the PMIC is locked. Call 
* Cy_SysPm_UnlockPmic() before enabling the PMIC.
*
* For information about the PMIC output pin and it assignment in 
* the specific families devices, refer to the appropriate device TRM.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_DisablePmicOutput(void)
{
    if(CY_SYSPM_PMIC_UNLOCK_KEY == _FLD2VAL(BACKUP_PMIC_CTL_UNLOCK, BACKUP->unPMIC_CTL.u32Register))
    {
        BACKUP->unPMIC_CTL.u32Register = 
        (BACKUP->unPMIC_CTL.u32Register | _VAL2FLD(BACKUP_PMIC_CTL_UNLOCK, CY_SYSPM_PMIC_UNLOCK_KEY)) &
        ((uint32_t) ~ _VAL2FLD(BACKUP_PMIC_CTL_PMIC_EN_OUTEN, 1u));
    } 
}


/*******************************************************************************
* Function Name: Cy_SysPm_LockPmic
****************************************************************************//**
*
* Locks the PMIC control register so that no changes can be made. The changes 
* are related to the PMIC enabling/disabling and PMIC output signal 
* enabling/disabling.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_LockPmic(void)
{
    BACKUP->unPMIC_CTL.u32Register = _CLR_SET_FLD32U(BACKUP->unPMIC_CTL.u32Register, BACKUP_PMIC_CTL_UNLOCK, 0u);
}


/*******************************************************************************
* Function Name: Cy_SysPm_UnlockPmic
****************************************************************************//**
*
* Unlocks the PMIC control register so that changes can be made. The changes are
* related to the PMIC enabling/disabling and PMIC output signal 
* enabling/disabling.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_UnlockPmic(void)
{
    BACKUP->unPMIC_CTL.u32Register = _CLR_SET_FLD32U(BACKUP->unPMIC_CTL.u32Register, BACKUP_PMIC_CTL_UNLOCK, CY_SYSPM_PMIC_UNLOCK_KEY);
}


/*******************************************************************************
* Function Name: Cy_SysPm_IsPmicEnabled
****************************************************************************//**
* 
* The function returns the status of the PMIC.
*
* \return
* True - The PMIC is enabled. <br>
* False - The PMIC is disabled. <br>
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_IsPmicEnabled(void)
{
    return(0u != _FLD2VAL(BACKUP_PMIC_CTL_PMIC_EN, BACKUP->unPMIC_CTL.u32Register));
}


/*******************************************************************************
* Function Name: Cy_SysPm_IsPmicEnabled
****************************************************************************//**
* 
* The function returns the status of the PMIC output pin.
*
* \return
* True - The PMIC output is enabled. <br>
* False - The PMIC output is disabled. <br>
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_IsPmicOutputEnabled(void)
{
    return(0u != _FLD2VAL(BACKUP_PMIC_CTL_PMIC_EN_OUTEN, BACKUP->unPMIC_CTL.u32Register));
}


/*******************************************************************************
* Function Name: Cy_SysPm_IsPmicLocked
****************************************************************************//**
*
* Returns the PMIC lock status
*
* \return
* True - The PMIC is locked. <br>
* False - The PMIC is unlocked. <br>
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SysPm_IsPmicLocked(void)
{
    return((_FLD2VAL(BACKUP_PMIC_CTL_UNLOCK, BACKUP->unPMIC_CTL.u32Register) == CY_SYSPM_PMIC_UNLOCK_KEY) ? false : true);
}

/******************************************************************************/
/****                   OVD/BOD Implementation                            *****/
/******************************************************************************/

/*******************************************************************************
* Function Name: Cy_SysPm_OvdEnable
****************************************************************************//**
*
*  Enables OVD over either VDDD or VDDA or VCCD.
*  
*  \param ovdSel \ref cy_en_syspm_ovd_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_OvdEnable(cy_en_syspm_ovd_sel_t ovdSel)
{
    switch(ovdSel)
    {
    case CY_SYSPM_OVD_ON_VDDD:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_OVDVDDD_ENABLE_Msk;
      break;

    case CY_SYSPM_OVD_ON_VDDA:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_OVDVDDA_ENABLE_Msk;
      break;

    case CY_SYSPM_OVD_ON_VCCD:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_OVDVCCD_ENABLE_Msk;
      break;

    default:
      break;
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_OvdDisable
****************************************************************************//**
*
*  Disables OVD over either VDDD or VDDA or VCCD.
*  
*  \param ovdSel \ref cy_en_syspm_ovd_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_OvdDisable(cy_en_syspm_ovd_sel_t ovdSel)
{
    switch(ovdSel)
    {
    case CY_SYSPM_OVD_ON_VDDD:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_OVDVDDD_ENABLE_Msk;
      break;

    case CY_SYSPM_OVD_ON_VDDA:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_OVDVDDA_ENABLE_Msk;
      break;

    case CY_SYSPM_OVD_ON_VCCD:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_OVDVCCD_ENABLE_Msk;
      break;

    default:
      break;
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_OvdVdddSelect
****************************************************************************//**
*
*  Selects voltage threshold for OVD on VDDD
*  
*  \param ovdVdddSel \ref cy_en_syspm_ovd_vddd_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_OvdVdddSelect(cy_en_syspm_ovd_vddd_sel_t ovdVdddSel)
{
    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_OVDVDDD_VSEL, ovdVdddSel);
}


/*******************************************************************************
* Function Name: Cy_SysPm_OvdVddaSelect
****************************************************************************//**
*
*  Selects voltage threshold for OVD on VDDA
*  
*  \param ovdVddaSel \ref cy_en_syspm_ovd_vdda_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_OvdVddaSelect(cy_en_syspm_ovd_vdda_sel_t ovdVddaSel)
{
    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_OVDVDDA_VSEL, ovdVddaSel);
}


/*******************************************************************************
* Function Name: Cy_Syspm_OvdActionSelect
****************************************************************************//**
*
*  Sets the action taken when the threshold is crossed in the programmed directions(s).
*  
*  \param ovdActionSelect \ref cy_en_syspm_ovd_action_select_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_Syspm_OvdActionSelect(cy_en_syspm_ovd_action_select_t ovdActionSelect)
{
    CY_ASSERT(CY_OVD_CHECK_ACTION_CFG(ovdActionSelect));

    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_OVDVDDA_ACTION, ovdActionSelect);
}


/*******************************************************************************
* Function Name: Cy_SysPm_BodEnable
****************************************************************************//**
*
*  Enables BOD over either VDDD or VDDA or VCCD.
*  
*  \param bodSel \ref cy_en_syspm_bod_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_BodEnable(cy_en_syspm_bod_sel_t bodSel)
{
    switch(bodSel)
    {
    case CY_SYSPM_BOD_ON_VDDD:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_BODVDDD_ENABLE_Msk;
      break;

    case CY_SYSPM_BOD_ON_VDDA:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_BODVDDA_ENABLE_Msk;
      break;

    case CY_SYSPM_BOD_ON_VCCD:
      SRSS->unPWR_SSV_CTL.u32Register |= SRSS_PWR_SSV_CTL_BODVCCD_ENABLE_Msk;
      break;

    default:
      break;
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_BodDisable
****************************************************************************//**
*
*  Disables BOD over either VDDD or VDDA or VCCD.
*  
*  \param bodSel \ref cy_en_syspm_bod_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_BodDisable(cy_en_syspm_bod_sel_t bodSel)
{
    switch(bodSel)
    {
    case CY_SYSPM_BOD_ON_VDDD:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_BODVDDD_ENABLE_Msk;
      break;

    case CY_SYSPM_BOD_ON_VDDA:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_BODVDDA_ENABLE_Msk;
      break;

    case CY_SYSPM_BOD_ON_VCCD:
      SRSS->unPWR_SSV_CTL.u32Register &= (uint32_t) ~SRSS_PWR_SSV_CTL_BODVCCD_ENABLE_Msk;
      break;

    default:
      break;
    }
}


/*******************************************************************************
* Function Name: Cy_SysPm_BodVdddSelect
****************************************************************************//**
*
*  Selects voltage threshold for BOD on VDDD
*  
*  \param bodVdddSel \ref cy_en_syspm_bod_vddd_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_BodVdddSelect(cy_en_syspm_bod_vddd_sel_t bodVdddSel)
{
    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_BODVDDD_VSEL, bodVdddSel);
}


/*******************************************************************************
* Function Name: Cy_SysPm_BodVddaSelect
****************************************************************************//**
*
*  Selects voltage threshold for BOD on VDDA
*  
*  \param bodVddaSel \ref cy_en_syspm_bod_vdda_sel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysPm_BodVddaSelect(cy_en_syspm_bod_vdda_sel_t bodVddaSel)
{
    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_BODVDDA_VSEL, bodVddaSel);
}


/*******************************************************************************
* Function Name: Cy_Syspm_BodActionSelect
****************************************************************************//**
*
*  Sets the action taken when the threshold is crossed in the programmed directions(s).
*  
*  \param bodActionSelect \ref cy_en_syspm_bod_action_select_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_Syspm_BodActionSelect(cy_en_syspm_bod_action_select_t bodActionSelect)
{
    CY_ASSERT(CY_BOD_CHECK_ACTION_CFG(bodActionSelect));

    SRSS->unPWR_SSV_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_SSV_CTL.u32Register, SRSS_PWR_SSV_CTL_BODVDDA_ACTION, bodActionSelect);
}


/*******************************************************************************
* Function Name: Cy_Syspm_SupplySupervisionStatus
****************************************************************************//**
*
* Supply Status.
*
* \param supplyEntitySelect \ref cy_en_syspm_supply_entity_select_t.

* \return
* State of BOD/OVD on VDDD/VDDA/VCCD, whether OK or Not OK
*
*******************************************************************************/
__STATIC_INLINE bool Cy_Syspm_SupplySupervisionStatus(cy_en_syspm_supply_entity_select_t supplyEntitySelect)
{    
    bool returnStatus = false;
    
    CY_ASSERT(CY_OVD_CHECK_SUPPLY_ENTITY(supplyEntitySelect));
    
    switch(supplyEntitySelect)
    {
    case CY_SYSPM_ENTITY_BOD_VDDD:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_BODVDDD_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;

    case CY_SYSPM_ENTITY_BOD_VDDA:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_BODVDDA_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;

    case CY_SYSPM_ENTITY_BOD_VCCD:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_BODVCCD_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;

    case CY_SYSPM_ENTITY_OVD_VDDD:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_OVDVDDD_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;

    case CY_SYSPM_ENTITY_OVD_VDDA:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_OVDVDDA_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;

    case CY_SYSPM_ENTITY_OVD_VCCD:
      returnStatus = (0u != _FLD2VAL(SRSS_PWR_SSV_STATUS_OVDVCCD_OK, SRSS->unPWR_SSV_STATUS.u32Register));
      break;
      
    default:
      break;
    }    
    
    return (returnStatus);
}

#ifdef __cplusplus
}
#endif

#endif /* _CY_SYSPM_H_ */
/** \} group_syspm_functions*/
/** \} group_syspm */


/* [] END OF FILE */
