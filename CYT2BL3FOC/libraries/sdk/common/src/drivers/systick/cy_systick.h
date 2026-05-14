/***************************************************************************//**
* \file 		gpio.h
* \version 		`$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
* \platform		Microcontroller family or derivative or all platforms
* \brief		Provides an API declaration of the GPIO driver
* \description	The GPIO driver provides an API to configure and access device 
*               Input/Output pins. IO pins include all general purpose types 
*               such as GPIO, SIO, HSIO, AUXIO, and their variants.
* 				Once the pin/port initialization is complete, each pin can be 
*				accessed by  specifying the port (GPIO_PRT_Type) and the pin 
*				(0-7) in the provided API functions.
*
*******************************************************************************/

/*******************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/******************************************************************************/
/* Version History:                                                           */
/******************************************************************************/
/* $Log:$ 																	  */

#ifndef _CY_SYSTICK_H_
#define _CY_SYSTICK_H_

/**
* \defgroup group_systick Core System Tick Timer (SysTick)
* \{
* Provides SysTick APIs.
*
* \section group_systick_configuration Configuration Considerations
*
* The SysTick timer is part of the CPU. The timer is a down counter with a
* 24-bit reload/tick value that is clocked by the FastClk/SlowClk. The timer has
* the capability to generate an interrupt when the set number of ticks expires
* and the counter is reloaded. This interrupt is available as part of the Nested
* Vectored Interrupt Controller (NVIC) for service by the CPU and can be used
* for general purpose timing control in user code.
*
* Changing the SysTick clock source and/or its frequency will change the
* interrupt interval and therefore SysTick_SetReload() should be called to
* compensate for this change.
*
* Because the timer is independent of the CPU (except for the clock), this can be
* handy in applications requiring precise timing that do not have a dedicated
* timer/counter available for the job.
*
* \section group_systick_more_information More Information
*
* Refer to the SysTick section of the ARM reference guide for complete details
* on the registers and their use.
*
* \section group_systick_changelog Changelog
*
* <table class="doxtable">
* <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
* <tr>
* <td>1.0</td>
* <td>Initial version</td>
* <td></td>
* </tr>
* </table>
*
* \defgroup group_systick_macro Macro
* \defgroup group_systick_functions Functions
* \defgroup group_systick_data_structures Data Structures
*/

#include <stdint.h>
#include <stdbool.h>
#include "syslib/cy_syslib.h"

#ifdef __cplusplus
extern "C" {
#endif

extern cy_israddress __ramVectors[];
/**
* \addtogroup group_systick_data_structures
* \{
*/

typedef void (*Cy_SysTick_Callback)(void);

typedef enum
{
    /** The low frequency clock clk_lf is selected. The precision of this clock depends on whether the low frequency
    clock source is a SRSS internal RC oscillator (imprecise) or a device external crystal oscillator (precise) */
    CY_SYSTICK_CLOCK_SOURCE_CLK_LF    = 0u,

    /** The internal main oscillator (IMO) clock clk_imo is selected. */
    CY_SYSTICK_CLOCK_SOURCE_CLK_IMO   = 1u,

    /** The external crystal oscillator (ECO) clock clk_eco is selected. */
    CY_SYSTICK_CLOCK_SOURCE_CLK_ECO   = 2u,

    /** The SRSS clk_timer is selected. The clk_timer is a divided/gated version of clk_hf or clk_imo. */
    CY_SYSTICK_CLOCK_SOURCE_CLK_TIMER = 3u,

    /** The CPU clock is selected. */
    CY_SYSTICK_CLOCK_SOURCE_CLK_CPU   = 4u,
} cy_en_systick_clock_source_t;

/** \} group_systick_data_structures */


/**
* \addtogroup group_systick_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SysTick_Init
****************************************************************************//**
*
* \brief  Initializes the callback addresses with pointers to NULL, associates the
* 	  SysTick system vector with the function responsible for calling
* 	  registered callback functions, configures the SysTick timer to generate an
* 	  interrupt periodically.
*
* \param  clockSource : The SysTick clock source \ref en_systick_clock_source_t
* \param  interval    : The SysTick reload value.
*
* \return None
* 
* \note   Clears the SysTick count flag if it was set.
*
*******************************************************************************/
void Cy_SysTick_Init(cy_en_systick_clock_source_t clockSource, uint32_t interval);

/*******************************************************************************
* Function Name: Cy_SysTick_Enable
****************************************************************************//**
*
* \brief  Enables the SysTick timer and its interrupt.
*
* \param  None
*
* \return None
*
* \note   Clears the SysTick count flag if it was set
*
*******************************************************************************/
void Cy_SysTick_Enable(void);

/*******************************************************************************
* Function Name: SysTick_Disable
****************************************************************************//**
*
* \brief  Disables the SysTick timer and its interrupt.
*
* \param  None
*
* \return None
*
* \note   Clears the SysTick count flag if it was set
*
*******************************************************************************/
void Cy_SysTick_Disable(void);

/*******************************************************************************
* Function Name: Cy_SysTick_EnableInterrupt
****************************************************************************//**
*
* \brief  Enables the SysTick interrupt.
*
* \param  None
*
* \return None
*
* \note   Clears the SysTick count flag if it was set
*
*******************************************************************************/
void Cy_SysTick_EnableInterrupt(void);

/*******************************************************************************
* Function Name: Cy_SysTick_DisableInterrupt
****************************************************************************//**
*
* \brief  Disables the SysTick interrupt.
*
* \param  None
*
* \return None
*
* \note   Clears the SysTick count flag if it was set
*
*******************************************************************************/
void Cy_SysTick_DisableInterrupt(void);

/*******************************************************************************
* Function Name: Cy_SysTick_SetReload
****************************************************************************//**
*
* \brief  Sets the value the counter is set to on a startup and after it reaches zero.
* 	  This function does not change or reset the current sysTick counter value, so
* 	  it should be cleared using the SysTick_Clear() API.
*
* \param  value: The valid range is [0x0-0x00FFFFFF]. The counter reset value.
*
*
* \return None
*******************************************************************************/
void Cy_SysTick_SetReload(uint32_t value);

/*******************************************************************************
* Function Name: Cy_SysTick_Set10msCalibration
****************************************************************************//**
*
* \brief  Sets the value that Specifies the number of clock source cycles 
*         (minus 1) that make up 10 ms for the selected clock source.
* 	  It is SW's responsibility to provide the correct value to get 10 ms
*         timeout for the selected clock source.
*
* \param  value: The valid range is [0x0-0x00FFFFFF].
*
*
* \return None
*******************************************************************************/
void Cy_SysTick_Set10msCalibration(uint32_t value);

/*******************************************************************************
* Function Name: Cy_SysTick_Get10msCalibration
****************************************************************************//**
*
* \brief  Sets the value that Specifies the number of clock source cycles 
*         (plus 1) that make up 10 ms for the selected clock source.
*
* \param  None
*
* \return The clock source cycles to make up 10ms value.
*
*******************************************************************************/
uint32_t Cy_SysTick_Get10msCalibration(void);

/*******************************************************************************
* Function Name: Cy_SysTick_GetReload
****************************************************************************//**
*
* \brief  Gets the value the counter is set to on a startup and after it reaches zero.
*
* \param  None
*
* \return The counter reset value.
*
*******************************************************************************/
uint32_t Cy_SysTick_GetReload(void);

/*******************************************************************************
* Function Name: Cy_SysTick_GetValue
****************************************************************************//**
*
* \brief  Gets the current SysTick counter value.
*
* \param  None
*
* \return The current SysTick counter value.
*
*******************************************************************************/
uint32_t Cy_SysTick_GetValue(void);

/*******************************************************************************
* Function Name: Cy_SysTick_SetCallback
****************************************************************************//**
*
* \brief  This function allows up to five user-defined interrupt service routine
* 	  functions to be associated with the SysTick interrupt. These are specified
* 	  through the use of pointers to the function.
*
* \param  number : The number of the callback function addresses to be set.
* 	           The valid range is from 0 to 4.
*
* \param  function: The pointer to the function that will be associated with the
* 	 	   SysTick ISR for the specified number.
*
* \return Returns the address of the previous callback function.
* 	  The NULL is returned if the specified address in not set.
*
* \note  The registered callback functions will be executed in the interrupt.
*
*******************************************************************************/
Cy_SysTick_Callback Cy_SysTick_SetCallback(uint32_t number, Cy_SysTick_Callback function);

/*******************************************************************************
* Function Name: Cy_SysTick_GetCallback
****************************************************************************//**
*
* \brief  The function gets the specified callback pointer.
*
* \param  number : The number of the callback function address to get. The valid
* 		  range is from 0 to 4.
*
* \return Returns the address of the specified callback function.
* 	  The NULL is returned if the specified address in not initialized.
*
*******************************************************************************/
Cy_SysTick_Callback SysTick_GetCallback(uint32_t number);

/*******************************************************************************
* Function Name: Cy_SysTick_SetClockSource
****************************************************************************//**
*
* \brief  Sets the clock source for the SysTick counter.
*
* Clears the SysTick count flag if it was set. If the clock source is not ready
* this function call will have no effect. After changing the clock source to the
* low frequency clock, the counter and reload register values will remain
* unchanged so the time to the interrupt will be significantly longer and vice
* versa.
*
* Changing the SysTick clock source and/or its frequency will change
* the interrupt interval and that SysTick_SetReload() should be
* called to compensate this change.
*
* \param clockSource : \ref en_systick_clock_source_t Clock source.
*
*******************************************************************************/
void Cy_SysTick_SetClockSource(cy_en_systick_clock_source_t clockSource);

/*******************************************************************************
* Function Name: Cy_SysTick_GetClockSource
****************************************************************************//**
*
* \brief   Gets the clock source for the SysTick counter.
*
* \param   None
*
* \returns \ref en_systick_clock_source_t Clock source
*
*******************************************************************************/
cy_en_systick_clock_source_t Cy_SysTick_GetClockSource(void);

/*******************************************************************************
* Function Name: Cy_SysTick_GetCountFlag
****************************************************************************//**
*
* \brief  The count flag is set, once the SysTick counter reaches zero.
* 	  The flag is cleared on read.
*
* \param  None
*
* \return Returns a non-zero value if a flag is set, otherwise a zero is
* 	  returned.
*
* \note   Clears the SysTick count flag if it was set.
*
*******************************************************************************/
uint32_t Cy_SysTick_GetCountFlag(void);

/*******************************************************************************
* Function Name: Cy_SysTick_Clear
****************************************************************************//**
*
* \brief  Clears the SysTick counter for a well-defined startup.
*
* \param  None
*
* \return None
*
*******************************************************************************/
void Cy_SysTick_Clear(void);

/*******************************************************************************
* Function Name: Cy_SysTick_DelayCoreCycle
****************************************************************************//**
*
* Delay input core cycle. This function can be input whole uint32_t value, while
* DELAY_CORE_CYCLE macro can be input only 24bit value.
* Note: This function is not re-entrant hand in glove with Cy_SysTick_DelayInUs.
*
* \param cycle core cycle to be delayed.
*
*******************************************************************************/
void Cy_SysTick_DelayCoreCycle(uint64_t cycle);

/*******************************************************************************
* Function Name: Cy_SysTick_DelayInUs
****************************************************************************//**
*
* Delay input value in micro second.
* Note: This function is not re-entrant hand in glove with Cy_SysTick_DelayCoreCycle.
* Note: Before calling this function SystemInit() have to be called.
*
* \param us micro second value to be delayed
*
*******************************************************************************/
void Cy_SysTick_DelayInUs(uint32_t us);

/** \} group_systick_functions */


/**
* \addtogroup group_systick_macro
* \{
*/

/** Driver major version */
#define SYSTICK_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define SYSTICK_DRV_VERSION_MINOR       0

#define CY_SYS_SYST_NUM_OF_CALLBACKS         (5u)
#define CY_SYSTICK_IRQ_NUM                   (15u)

/** \} group_systick_macro */


#ifdef __cplusplus
}
#endif

#endif /* _CY_SYSTICK_H_ */

/** \} group_systick */


/* [] END OF FILE */
