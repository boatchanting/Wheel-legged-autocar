/***************************************************************************//**
* \file cy_lvd.h
* \version 1.0
* 
* The header file of the LVD driver.
*
********************************************************************************
* \copyright
* Copyright 2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \addtogroup group_lvd Low Voltage Detector (LVD)
* \{
* The LVD driver provides an API to manage the Low Voltage Detection block. 
* The LVD block provides a status of currently observed VDDD voltage 
* and triggers an interrupt when the observed voltage crosses an adjusted
* threshold.
*
* \section group_lvd_configuration_considerations Configuration Considerations
* To set up an LVD, configure the voltage threshold by the 
* \ref Cy_LVD_SetThreshold function, ensure that the LVD block itself and LVD 
* interrupt are disabled (by the \ref Cy_LVD_Disable and 
* \ref Cy_LVD_ClearInterruptMask functions correspondingly) before changing the
* threshold to prevent propagating a false interrupt. 
* Then configure interrupts by the \ref Cy_LVD_SetInterruptConfig function, do
* not forget to initialize an interrupt handler (the interrupt source number 
* is srss_interrupt_IRQn).
* Then enable LVD by the \ref Cy_LVD_Enable function, then wait for at least 8us
* to get the circuit stabilized and clear the possible false interrupts by the
* \ref Cy_LVD_ClearInterrupt, and finally the LVD interrupt can be enabled by
* the \ref Cy_LVD_SetInterruptMask function.
*
* Note that the LVD circuit is available only in Active, LPACTIVE, Sleep, and
* LPSLEEP power modes. If an LVD is required in DeepSleep mode, then the device
* should be configured to periodically wake up from deep sleep using a
* Deep-Sleep wakeup source. This makes sure a LVD check is performed during
* Active/LPACTIVE mode.
*
* \section group_lvd_more_information More Information
* See the LVD chapter of the device technical reference manual (TRM).
*
* \section group_lvd_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason of Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial Version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_lvd_macros Macros
* \defgroup group_lvd_functions Functions
* \defgroup group_lvd_enums Enumerated types
*/


#if !defined CY_LVD_H
#define CY_LVD_H
    
#include "syspm/cy_syspm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup group_lvd_macros
* \{
*/

/** Driver major version */
#define CY_LVD_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_LVD_DRV_VERSION_MINOR       0

/** LVD driver identifier */
#define CY_LVD_ID                      (CY_PDL_DRV_ID(0x39u))

/** Interrupt mask for \ref Cy_LVD_GetInterruptStatus(),
                       \ref Cy_LVD_GetInterruptMask() and 
                       \ref Cy_LVD_GetInterruptStatusMasked() */
#define CY_LVD_INTR        (SRSS_SRSS_INTR_HVLVD1_Msk)

/** \} group_lvd_macros */


/** \addtogroup group_lvd_enums
* \{
*/

/**
 * LVD Type Selection.
 */
typedef enum
{
    CY_LVD_TYPE_1   = 0x0u,  /**<Select LVD Type 1 */
    CY_LVD_TYPE_2   = 0x1u,  /**<Select LVD Type 2 */
} cy_en_lvd_type_select_t;

/**
 * LVD reference voltage select.
 */
typedef enum
{
    CY_LVD_THRESHOLD_2_8_V    = 0x00u, /**<Select LVD reference voltage: 2.8V */
    CY_LVD_THRESHOLD_2_9_V    = 0x01u, /**<Select LVD reference voltage: 2.9V */
    CY_LVD_THRESHOLD_3_0_V    = 0x02u, /**<Select LVD reference voltage: 3.0V */
    CY_LVD_THRESHOLD_3_1_V    = 0x03u, /**<Select LVD reference voltage: 3.1V */
    CY_LVD_THRESHOLD_3_2_V    = 0x04u, /**<Select LVD reference voltage: 3.2V */
    CY_LVD_THRESHOLD_3_3_V    = 0x05u, /**<Select LVD reference voltage: 3.3V */
    CY_LVD_THRESHOLD_3_4_V    = 0x06u, /**<Select LVD reference voltage: 3.4V */
    CY_LVD_THRESHOLD_3_5_V    = 0x07u, /**<Select LVD reference voltage: 3.5V */
    CY_LVD_THRESHOLD_3_6_V    = 0x08u, /**<Select LVD reference voltage: 3.6V */
    CY_LVD_THRESHOLD_3_7_V    = 0x09u, /**<Select LVD reference voltage: 3.7V */
    CY_LVD_THRESHOLD_3_8_V    = 0x0Au, /**<Select LVD reference voltage: 3.8V */
    CY_LVD_THRESHOLD_3_9_V    = 0x0Bu, /**<Select LVD reference voltage: 3.9V */
    CY_LVD_THRESHOLD_4_0_V    = 0x0Cu, /**<Select LVD reference voltage: 4.0V */
    CY_LVD_THRESHOLD_4_1_V    = 0x0Du, /**<Select LVD reference voltage: 4.1V */
    CY_LVD_THRESHOLD_4_2_V    = 0x0Eu, /**<Select LVD reference voltage: 4.2V */
    CY_LVD_THRESHOLD_4_3_V    = 0x0Fu, /**<Select LVD reference voltage: 4.3V */
    CY_LVD_THRESHOLD_4_4_V    = 0x10u, /**<Select LVD reference voltage: 4.4V */
    CY_LVD_THRESHOLD_4_5_V    = 0x11u, /**<Select LVD reference voltage: 4.5V */
    CY_LVD_THRESHOLD_4_6_V    = 0x12u, /**<Select LVD reference voltage: 4.6V */
    CY_LVD_THRESHOLD_4_7_V    = 0x13u, /**<Select LVD reference voltage: 4.7V */
    CY_LVD_THRESHOLD_4_8_V    = 0x14u, /**<Select LVD reference voltage: 4.8V */
    CY_LVD_THRESHOLD_4_9_V    = 0x15u, /**<Select LVD reference voltage: 4.9V */
    CY_LVD_THRESHOLD_5_0_V    = 0x16u, /**<Select LVD reference voltage: 5.0V */
    CY_LVD_THRESHOLD_5_1_V    = 0x17u, /**<Select LVD reference voltage: 5.1V */
    CY_LVD_THRESHOLD_5_2_V    = 0x18u, /**<Select LVD reference voltage: 5.2V */
    CY_LVD_THRESHOLD_5_3_V    = 0x19u  /**<Select LVD reference voltage: 5.3V */
} cy_en_lvd_tripsel_t;

/**
 * LVD edge select configuration.
 */
typedef enum
{
    CY_LVD_EDGE_DISABLE = 0x0u,  /**<Select LVD edge select: disabled */
    CY_LVD_EDGE_RISING  = 0x1u,  /**<Select LVD edge select: rising edge */
    CY_LVD_EDGE_FALLING = 0x2u,  /**<Select LVD edge select: falling edge */
    CY_LVD_EDGE_BOTH    = 0x3u,  /**<Select LVD edge select: both edges */
} cy_en_lvd_edge_select_t;

/**
 * LVD action select configuration.
 */
typedef enum
{
    CY_LVD_ACTION_INTR  = 0x0u,  /**<Select LVD action select: interrupt */
    CY_LVD_ACTION_FAULT = 0x1u,  /**<Select LVD action select: fault */
} cy_en_lvd_action_select_t;

/**
 * LVD output status.
 */
typedef enum
{
    CY_LVD_STATUS_BELOW   = 0x0u,  /**<The voltage is below the threshold */
    CY_LVD_STATUS_ABOVE   = 0x1u,  /**<The voltage is above the threshold */
} cy_en_lvd_status_t;

/** \} group_lvd_enums */

/** \cond internal */
/* Macros for conditions used by CY_ASSERT calls */
#define CY_LVD_CHECK_TRIPSEL(threshold)  (((threshold) == CY_LVD_THRESHOLD_2_8_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_2_9_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_0_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_1_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_2_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_3_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_4_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_5_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_6_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_7_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_8_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_3_9_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_0_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_1_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_2_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_3_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_4_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_5_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_6_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_7_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_8_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_4_9_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_5_0_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_5_1_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_5_2_V) || \
                                          ((threshold) == CY_LVD_THRESHOLD_5_3_V))
                                          
#define CY_LVD_CHECK_EDGE_CFG(edgeCfg)   (((edgeCfg) == CY_LVD_EDGE_DISABLE) || \
                                          ((edgeCfg) == CY_LVD_EDGE_RISING) || \
                                          ((edgeCfg) == CY_LVD_EDGE_FALLING) || \
                                          ((edgeCfg) == CY_LVD_EDGE_BOTH))     

#define CY_LVD_CHECK_ACTION_CFG(actCfg)  (((actCfg) == CY_LVD_ACTION_INTR) || \
                                          ((actCfg) == CY_LVD_ACTION_FAULT)) 

/** \endcond */

/**
* \addtogroup group_lvd_functions
* \{
*/
__STATIC_INLINE void Cy_LVD_Enable(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_Disable(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_SetThreshold(cy_en_lvd_type_select_t lvdType, cy_en_lvd_tripsel_t threshold);
__STATIC_INLINE cy_en_lvd_status_t Cy_LVD_GetStatus(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptStatus(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_ClearInterrupt(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_SetInterrupt(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptMask(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_SetInterruptMask(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_ClearInterruptMask(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptStatusMasked(cy_en_lvd_type_select_t lvdType);
__STATIC_INLINE void Cy_LVD_SetEdgeSelect(cy_en_lvd_type_select_t lvdType, cy_en_lvd_edge_select_t lvdEdgeSelect);
__STATIC_INLINE void Cy_LVD_SetActionSelect(cy_en_lvd_type_select_t lvdType, cy_en_lvd_action_select_t lvdActionSelect);
cy_en_syspm_status_t Cy_LVD_DeepSleepCallback(cy_en_lvd_type_select_t lvdType, cy_stc_syspm_callback_params_t * callbackParams);


/*******************************************************************************
* Function Name: Cy_LVD_Enable
****************************************************************************//**
*
*  Enables the output of the LVD block when the VDDD voltage is   
*  at or below the threshold.
*  See the Configuration Considerations section for details.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_Enable(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unPWR_LVD_CTL.u32Register |= SRSS_PWR_LVD_CTL_HVLVD1_EN_HT_Msk;
    }
    else
    {
        SRSS->unPWR_LVD_CTL2.u32Register |= SRSS_PWR_LVD_CTL2_HVLVD2_EN_HT_Msk;
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_Disable
****************************************************************************//**
*
*  Disables the LVD block. A low voltage detection interrupt is disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_Disable(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unPWR_LVD_CTL.u32Register &= (uint32_t) ~SRSS_PWR_LVD_CTL_HVLVD1_EN_HT_Msk;
    }
    else
    {
        SRSS->unPWR_LVD_CTL2.u32Register &= (uint32_t) ~SRSS_PWR_LVD_CTL2_HVLVD2_EN_HT_Msk;
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_SetThreshold
****************************************************************************//**
*
*  Sets a threshold for monitoring the VDDD voltage.
*  To prevent propagating a false interrupt, before changing the threshold 
*  ensure that the LVD block itself and LVD interrupt are disabled by the 
*  \ref Cy_LVD_Disable and \ref Cy_LVD_ClearInterruptMask functions 
*  correspondingly.
*
*  \param threshold 
*  Threshold selection for Low Voltage Detect circuit, \ref cy_en_lvd_tripsel_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_SetThreshold(cy_en_lvd_type_select_t lvdType, cy_en_lvd_tripsel_t threshold)
{
    CY_ASSERT(CY_LVD_CHECK_TRIPSEL(threshold));
    
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unPWR_LVD_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL.u32Register, SRSS_PWR_LVD_CTL_HVLVD1_TRIPSEL_HT, threshold);
    }
    else
    {
        SRSS->unPWR_LVD_CTL2.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL2.u32Register, SRSS_PWR_LVD_CTL2_HVLVD2_TRIPSEL_HT, threshold);
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_GetStatus
****************************************************************************//**
*
*  Returns the status of LVD.
*  SRSS LVD Status Register (PWR_LVD_STATUS).
*  
*  \return LVD status, \ref cy_en_lvd_status_t.
* 
*******************************************************************************/
__STATIC_INLINE cy_en_lvd_status_t Cy_LVD_GetStatus(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        return ((cy_en_lvd_status_t) _FLD2VAL(SRSS_PWR_LVD_STATUS_HVLVD1_OUT, SRSS->unPWR_LVD_STATUS.u32Register));
    }
    else
    {
        return ((cy_en_lvd_status_t) _FLD2VAL(SRSS_PWR_LVD_STATUS2_HVLVD2_OUT, SRSS->unPWR_LVD_STATUS2.u32Register));
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_GetInterruptStatus
****************************************************************************//**
*
*  Returns the status of LVD interrupt. 
*  SRSS Interrupt Register (SRSS_INTR).
*  
*  \return SRSS Interrupt status, \ref CY_LVD_INTR.
* 
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptStatus(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        return (SRSS->unSRSS_INTR.u32Register & SRSS_SRSS_INTR_HVLVD1_Msk);
    }
    else
    {
        return (SRSS->unSRSS_INTR.u32Register & SRSS_SRSS_INTR_HVLVD2_Msk);
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_ClearInterrupt
****************************************************************************//**
*
*  Clears LVD interrupt. 
*  SRSS Interrupt Register (SRSS_INTR).
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_ClearInterrupt(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unSRSS_INTR.u32Register = SRSS_SRSS_INTR_HVLVD1_Msk;
    }
    else
    {
        SRSS->unSRSS_INTR.u32Register = SRSS_SRSS_INTR_HVLVD2_Msk;
    }
    
    (void) SRSS->unSRSS_INTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_LVD_SetInterrupt
****************************************************************************//**
*
*  Triggers the device to generate interrupt for LVD.
*  SRSS Interrupt Set Register (SRSS_INTR_SET).
* 
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_SetInterrupt(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unSRSS_INTR_SET.u32Register = SRSS_SRSS_INTR_SET_HVLVD1_Msk;
    }
    else
    {
        SRSS->unSRSS_INTR_SET.u32Register = SRSS_SRSS_INTR_SET_HVLVD2_Msk;
    }
}


/*******************************************************************************
* Function Name:  Cy_LVD_GetInterruptMask
****************************************************************************//**
*
*  Returns the mask value of LVD interrupts.
*  SRSS Interrupt Mask Register (SRSS_INTR_MASK).
*
*  \return SRSS Interrupt Mask value, \ref CY_LVD_INTR.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptMask(cy_en_lvd_type_select_t lvdType)
{ 
    if(lvdType == CY_LVD_TYPE_1)
    {
        return (SRSS->unSRSS_INTR_MASK.u32Register & SRSS_SRSS_INTR_MASK_HVLVD1_Msk);
    }
    else
    {
        return (SRSS->unSRSS_INTR_MASK.u32Register & SRSS_SRSS_INTR_MASK_HVLVD2_Msk);
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_SetInterruptMask
****************************************************************************//**
*
* Enables LVD interrupts. 
* Sets the LVD interrupt mask in the SRSS_INTR_MASK register.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_SetInterruptMask(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unSRSS_INTR_MASK.u32Register |= SRSS_SRSS_INTR_MASK_HVLVD1_Msk;
    }
    else
    {
        SRSS->unSRSS_INTR_MASK.u32Register |= SRSS_SRSS_INTR_MASK_HVLVD2_Msk;
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_ClearInterruptMask
****************************************************************************//**
*
* Disables LVD interrupts. 
* Clears the LVD interrupt mask in the SRSS_INTR_MASK register.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_ClearInterruptMask(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unSRSS_INTR_MASK.u32Register &= (uint32_t) ~SRSS_SRSS_INTR_MASK_HVLVD1_Msk;
    }
    else
    {
        SRSS->unSRSS_INTR_MASK.u32Register &= (uint32_t) ~SRSS_SRSS_INTR_MASK_HVLVD2_Msk;
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_GetInterruptStatusMasked
****************************************************************************//**
*
*  Returns the masked interrupt status which is a bitwise AND between the 
*  interrupt status and interrupt mask registers.
*  SRSS Interrupt Masked Register (SRSS_INTR_MASKED).
*  
*  \return SRSS Interrupt Masked value, \ref CY_LVD_INTR.
* 
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_LVD_GetInterruptStatusMasked(cy_en_lvd_type_select_t lvdType)
{
    if(lvdType == CY_LVD_TYPE_1)
    {
        return (SRSS->unSRSS_INTR_MASKED.u32Register & SRSS_SRSS_INTR_MASKED_HVLVD1_Msk);
    }
    else
    {
        return (SRSS->unSRSS_INTR_MASKED.u32Register & SRSS_SRSS_INTR_MASKED_HVLVD2_Msk);
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_SetEdgeSelect
****************************************************************************//**
*
*  Sets which edge(s) will trigger as action when the threshold is crossed.
*  Note: Before calling this function ensure that the LVD block itself and LVD interrupts 
*  are disabled
*  
*  \param lvdSelect \ref cy_en_lvd_edge_select_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_SetEdgeSelect(cy_en_lvd_type_select_t lvdType, cy_en_lvd_edge_select_t lvdEdgeSelect)
{
    CY_ASSERT(CY_LVD_CHECK_EDGE_CFG(lvdEdgeSelect));
    
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unPWR_LVD_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL.u32Register, SRSS_PWR_LVD_CTL_HVLVD1_EDGE_SEL, lvdEdgeSelect);
    }
    else
    {
        SRSS->unPWR_LVD_CTL2.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL2.u32Register, SRSS_PWR_LVD_CTL2_HVLVD2_EDGE_SEL, lvdEdgeSelect);
    }
}


/*******************************************************************************
* Function Name: Cy_LVD_SetActionSelect
****************************************************************************//**
*
*  Sets the action taken when the threshold is crossed in the programmed directions(s).
*  
*  \param lvdActionSelect \ref cy_en_lvd_action_select_t.
*
*******************************************************************************/
__STATIC_INLINE void Cy_LVD_SetActionSelect(cy_en_lvd_type_select_t lvdType, cy_en_lvd_action_select_t lvdActionSelect)
{
    CY_ASSERT(CY_LVD_CHECK_ACTION_CFG(lvdActionSelect));
    
    if(lvdType == CY_LVD_TYPE_1)
    {
        SRSS->unPWR_LVD_CTL.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL.u32Register, SRSS_PWR_LVD_CTL_HVLVD1_ACTION, lvdActionSelect);
    }
    else
    {
        SRSS->unPWR_LVD_CTL2.u32Register = _CLR_SET_FLD32U(SRSS->unPWR_LVD_CTL2.u32Register, SRSS_PWR_LVD_CTL2_HVLVD2_ACTION, lvdActionSelect);
    }
}


/** \} group_lvd_functions */

#ifdef __cplusplus
}
#endif

#endif /* CY_LVD_H */


/** \} group_lvd */


/* [] END OF FILE */
