/***************************************************************************//**
* \file cy_sysflt.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Sysfault driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sysflt System Fault (SysFlt)
* \{
* The SysFault driver provides an API to configure the fault reporting structure.
*
* \section group_sysflt_more_information More Information
*
* Refer to the technical reference manual (TRM) and the device datasheet.
* 
* \defgroup group_sysflt_enums Enumerated Types
*/


#if !defined(CY_SYSFLT_H)
#define CY_SYSFLT_H

#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"
#include "sysflt/cy_sysflt_config.h"

#if defined(__cplusplus)
extern "C" {
#endif





/***************************************
*       Enumeration
***************************************/

/**
* \addtogroup group_sysflt_enums
* \{
*/

typedef enum 
{
    CY_SYSFLT_SUCCESS   = 0x00u,                                     /**< Returned successful */
    CY_SYSFLT_BAD_PARAM = 0x01u, /**< Bad parameter was passed */
} cy_en_sysflt_status_t;

/** \} group_sysint_enums */


/***************************************
*       Configuration Structure
***************************************/

/**
* \addtogroup group_sysflt_data_structures
* \{
*/

/**
* Initialization configuration structure for a fault control register
*/
typedef struct {
    bool       TriggerEnable;    /**< Enables the trigger output when it is True */
    bool       OutputEnable;   	 /**< Enables the output signal when it is True */
    bool       ResetEnable;      /**< Enables the Reset request  when it is True */
} cy_stc_sysflt_t;


/** \} group_sysflt_data_structures */


/***************************************
*       Function Prototypes
***************************************/

/**
* \addtogroup group_sysflt_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SysFlt_Init
****************************************************************************//**
*
* \brief  Initializes the SysFault for recording faults.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_Init(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,const cy_stc_sysflt_t * config);

/*******************************************************************************
* Function Name: Cy_SysFlt_ClearStatus
****************************************************************************//**
*
* \brief  Clear fault status register
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
*******************************************************************************/
void Cy_SysFlt_ClearStatus(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetErrorSource
****************************************************************************//**
*
* \brief  Returns the source of error for the fault.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return see \ref cy_en_sysflt_source
*
*******************************************************************************/
cy_en_sysflt_source_t Cy_SysFlt_GetErrorSource(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetData0
****************************************************************************//**
*
* \brief  Returns the fault information.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Fault information
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetData0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetData1
****************************************************************************//**
*
* \brief  Returns the fault information.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Fault information
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetData1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetData2
****************************************************************************//**
*
* \brief  Returns the fault information.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Fault information
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetData2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetData3
****************************************************************************//**
*
* \brief  Returns the fault information.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Fault information
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetData3(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_Pending0
****************************************************************************//**
*
* \brief  Returns the sources of pending fault which are not captured .
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Source of pending fault.
*
*******************************************************************************/
uint32_t Cy_SysFlt_Pending0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_Pending1
****************************************************************************//**
*
* \brief  Returns the sources of pending fault which are not captured .
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Source of pending fault.
*
*******************************************************************************/
uint32_t Cy_SysFlt_Pending1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_Pending2
****************************************************************************//**
*
* \brief  Returns the sources of pending fault which are not captured .
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Source of pending fault.
*
*******************************************************************************/
uint32_t Cy_SysFlt_Pending2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_Mask0
****************************************************************************//**
*
* \brief  Enable the faults to be captured.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
* 	  enable     : Faults capturing enable.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_Mask0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,uint32_t enable);

/*******************************************************************************
* Function Name: Cy_SysFlt_Mask1
****************************************************************************//**
*
* \brief  Enable the faults to be captured.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
* 	  enable     : Faults capturing enable.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_Mask1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,uint32_t enable);

/*******************************************************************************
* Function Name: Cy_SysFlt_Mask2
****************************************************************************//**
*
* \brief  Enable the faults to be captured.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
* 	  enable     : Faults capturing enable.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_Mask2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,uint32_t enable);

/*******************************************************************************
* Function Name: Cy_SysFlt_SetMaskByIdx
****************************************************************************//**
*
* \brief  Enable the faults to be captured by id.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
* 
* \param  idx : The fault id to be set in the mask register.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_SetMaskByIdx(volatile stc_FAULT_STRUCT_t *ptscSYSFLT, cy_en_sysflt_source_t idx);

/*******************************************************************************
* Function Name: Cy_SysFlt_ClearMaskByIdx
****************************************************************************//**
*
* \brief  Disable the faults to be captured by id.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
* 
* \param  idx : The fault id to be clear in the mask register.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_ClearMaskByIdx(volatile stc_FAULT_STRUCT_t *ptscSYSFLT, cy_en_sysflt_source_t idx);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetInterruptStatus
****************************************************************************//**
*
* \brief  Returns the status of the interrupt.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Status
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetInterruptStatus(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_ClearInterrupt
****************************************************************************//**
*
* \brief  Clears Active Interrupt Source
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_ClearInterrupt(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_SetInterrupt
****************************************************************************//**
*
* \brief  Triggers an interrupt via a software write.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_SetInterrupt(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_SetInterruptMask
****************************************************************************//**
*
* \brief  Sets an interrupt mask. A 1 means that when the event occurs, it will cause an interrupt;
* 	  a 0 means no interrupt will be triggered.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return None
*
*******************************************************************************/
void Cy_SysFlt_SetInterruptMask(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetInterruptMask
****************************************************************************//**
*
* \brief  Returns the interrupt mask.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Interrupt Mask
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetInterruptMask(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);

/*******************************************************************************
* Function Name: Cy_SysFlt_GetInterruptStatusMasked
****************************************************************************//**
*
* \brief  Returns which masked interrupt triggered the interrupt.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return Interrupt Mask
*
*******************************************************************************/
uint32_t Cy_SysFlt_GetInterruptStatusMasked(volatile stc_FAULT_STRUCT_t *ptscSYSFLT);


/** \} group_sysflt_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_SYSFLT_H */

/** \} group_sysflt */

/* [] END OF FILE */
