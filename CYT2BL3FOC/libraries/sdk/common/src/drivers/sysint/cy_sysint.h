/***************************************************************************//**
* \file cy_sysint.h
* \version 1.0
*
* \brief
* Provides an API declaration of the SysInt driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sysint System Interrupt (SysInt)
* \{
* The SysInt driver provides an API to configure the device peripheral interrupts.
* It provides a lightweight interface to complement the CMSIS core NVIC API.
* The provided functions are applicable for all cores in a device and they can
* be used to configure and connect device peripheral interrupts to one or more
* cores.
*
* \n
* <b> Initialization </b>
*
* To configure an interrupt, call Cy_SysInt_Init().
* Populate the configuration structure (cy_stc_sysint_t) and pass it as a parameter
* along with the ISR address. This initializes the interrupt and
* instructs the CPU to jump to the specified ISR vector upon a valid trigger.
*
* Populating the interrupt configuration structure differs slightly for the system
* interrupt. Both of CM0+ and CM4 supports only up to 8 CPU interrupt and system
* (peripheral) interrupts are mapped to those.
* To allow all device interrupts to be routable to the NVIC of this core,
* there exists a 1023:1 multiplexer at each of the 8 NVIC channels.
* To configure system interrupt, call Cy_SysInt_InitIRQ().
*
* \n
* <b> Enable </b>
*
* After initializing an interrupt, use the CMSIS Core NVIC_EnableIRQ() function to
* enable it. Given an initialization structure named config, the function should be
* called as follows:
*
* NVIC_EnableIRQ(config.intIdx)
*
* \n
* <b>Writing an interrupt service routine</b>
*
* Servicing interrupts in the Peripheral Drivers should follow a prescribed
* recipe to ensure all interrupts are serviced and duplicate interrupts are not
* received. Any peripheral-specific register that must be written to clear the
* source of the interrupt should be written as soon as possible in the interrupt
* service routine. However, note that due to buffering on the output bus to the
* peripherals, the write clearing the interrupt may be delayed. After performing
* the normal interrupt service that should respond to the interrupting
* condition, the interrupt register that was initially written to clear the
* register should be read before returning from the interrupt service routine.
* This read ensures that the initial write has been flushed out to the hardware.
* Note, no additional processing should be performed based on the result of this
* read, as this read is just intended to ensure the write operation is flushed.
*
* This final read may indicate a pending interrupt. What this means is that in
* the interval between when the write actually happened at the peripheral and
* when the read actually happened at the peripheral, an interrupting condition
* occurred. This is ok and a return from the interrupt is still the correct
* action. As soon as conditions warrant, meaning interrupts are enabled and
* there are no higher priority interrupts pending, the interrupt will be
* triggered again to service the additional condition.
*
* \section group_sysint_more_information More Information
*
* Refer to the technical reference manual (TRM) and the device datasheet.
*
* \section group_sysint_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_sysint_macro Macro
* \defgroup group_sysint_globals Global variables
* \defgroup group_sysint_functions Functions
* \defgroup group_sysint_data_structures Data structures
* \defgroup group_sysint_enums Enumerated Types
*/


#if !defined(CY_SYSINT_H)
#define CY_SYSINT_H

#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef void (*cy_systemIntr_Handler)(void);


/***************************************
*       Global Variable
***************************************/

/**
* \addtogroup group_sysint_globals
* \{
*/

extern const cy_israddress __Vectors[]; /**< Vector table in Flash */
extern       cy_israddress __ramVectors[]; /**< Relocated vector table in SRAM */


/** \} group_sysint_globals */


/***************************************
*       Global Interrupt
***************************************/

/**
* \addtogroup group_sysint_macro
* \{
*/

/** Driver major version */
#define CY_SYSINT_DRV_VERSION_MAJOR    (0x01u)

/** Driver minor version */
#define CY_SYSINT_DRV_VERSION_MINOR    (0x00u)

/** SysInt driver ID */
#define CY_SYSINT_ID                   CY_PDL_DRV_ID(0x15u)

/** \} group_sysint_macro */


/***************************************
*       Enumeration
***************************************/

/**
* \addtogroup group_sysint_enums
* \{
*/

/**
* SysInt Driver error codes
*/
typedef enum
{
    CY_SYSINT_SUCCESS   = 0x00u,                                      /**< Returned successful */
    CY_SYSINT_BAD_PARAM = CY_SYSINT_ID | CY_PDL_STATUS_ERROR | 0x01u, /**< Bad parameter was passed */
} cy_en_sysint_status_t;

/** \} group_sysint_enums */


/***************************************
*       Configuration Structure
***************************************/

/**
* \addtogroup group_sysint_data_structures
* \{
*/

/**
* Initialization configuration structure for a single core interrupt channel
*/
typedef struct {
    IRQn_Type       intrSrc;        /**< Interrupt source */
    uint32_t        intrPriority;   /**< Interrupt priority number (Refer to __NVIC_PRIO_BITS) */
} cy_stc_sysint_t;

/**
* Initialization configuration structure for a single system interrupt channel
*/
typedef struct {
    cy_en_intr_t    sysIntSrc;      /**< System Interrupt source */
    IRQn_Type       intIdx;         /**< CPU interrupt index whom system interrupt source is mapped to.
                                     *   Note: Only CPUIntIdxX_IRQn [X = 0..7] is valid.
                                     */
    bool            isEnabled;      /**< Enabling/disabling system interruption */
} cy_stc_sysint_irq_t;

/** \} group_sysint_data_structures */


/***************************************
*              Constants
***************************************/

/** \cond INTERNAL */

#define CY_INT_IRQ_BASE            (0x10ul) /**< Start location of interrupts in the vector table */
#define CY_SYSINT_STATE_MASK       (0x01ul) /**< Mask for interrupt state */
#define CY_SYSINT_STIR_MASK        (0xfful) /**< Mask for software trigger interrupt register */

/** \endcond */


/***************************************
*       Function Prototypes
***************************************/

/**
* \addtogroup group_sysint_functions
* \{
*/
cy_en_sysint_status_t Cy_SysInt_Init(const cy_stc_sysint_t * config, cy_israddress userIsr);
cy_en_sysint_status_t Cy_SysInt_InitIRQ(const cy_stc_sysint_irq_t* config);

void Cy_SysInt_SetVector(IRQn_Type intrSrc, cy_israddress userIsr);
cy_israddress Cy_SysInt_GetVector(IRQn_Type intrSrc);
void Cy_SysInt_SetVectorIRQ(IRQn_Type intIdx, cy_israddress userIsr);
cy_israddress Cy_SysInt_GetVectorIRQ(IRQn_Type intIdx);
void  Cy_SysInt_SetSystemIrqVector(cy_en_intr_t sysIntSrc, cy_systemIntr_Handler userIsr);
cy_systemIntr_Handler  Cy_SysInt_GetSystemIrqVector(cy_en_intr_t intrSrc);
void Cy_SysInt_EnableIRQ(cy_en_intr_t sysIntSrc);
void Cy_SysInt_DisableIRQ(cy_en_intr_t sysIntSrc);
cy_en_intr_t Cy_SysInt_GetIntStatus(IRQn_Type intrSrc);
void Cy_SysInt_SetIntSourceNMI(uint8_t nmiNum, cy_en_intr_t sysIntSrc);
cy_en_intr_t Cy_SysInt_GetIntSourceNMI(uint8_t nmiNum);

#if (CY_CPU_CORTEX_M0P)
#else
    __STATIC_INLINE void Cy_SysInt_SoftwareTrig(IRQn_Type intrSrc);
#endif


#if (!CY_CPU_CORTEX_M0P) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_SysInt_SoftwareTrig
****************************************************************************//**
*
* \brief Triggers an interrupt using software (Not applicable for CM0+).
*
* <b>Note</b> Only privileged software can enable unprivileged access to the
* Software Trigger Interrupt Register (STIR).
*
* \param intrSrc
* Interrupt source
*
*******************************************************************************/
__STATIC_INLINE void Cy_SysInt_SoftwareTrig(IRQn_Type intrSrc)
{
    NVIC->STIR = intrSrc & CY_SYSINT_STIR_MASK;
}

#endif

/** \} group_sysint_functions */



#if defined(__cplusplus)
}
#endif

#endif /* CY_SYSINT_H */

/** \} group_sysint */

/* [] END OF FILE */
