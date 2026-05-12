/***************************************************************************//**
* \file
* \version 1.0
*
* \brief
* Provides an API implementation of the SysFault driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_sysflt.h"

#if defined(__cplusplus)
extern "C" {
#endif

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
void Cy_SysFlt_Init(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,const cy_stc_sysflt_t * config)
{
    if (config->TriggerEnable)
    {
        ptscSYSFLT->unCTL.stcField.u1TR_EN =  true;
    }
    else
    {
        ptscSYSFLT->unCTL.stcField.u1TR_EN = false;
    }

    if (config->OutputEnable)
    {
        ptscSYSFLT->unCTL.stcField.u1OUT_EN = true;
    }
    else
    {
        ptscSYSFLT->unCTL.stcField.u1OUT_EN = false;
    }

    if (config->ResetEnable)
    {
        ptscSYSFLT->unCTL.stcField.u1RESET_REQ_EN = true;
    }
    else
    {
        ptscSYSFLT->unCTL.stcField.u1RESET_REQ_EN = false;
    }
}

/*******************************************************************************
* Function Name: Cy_SysFlt_ClearStatus
****************************************************************************//**
*
* \brief  Clear fault status register
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
*******************************************************************************/
void Cy_SysFlt_ClearStatus(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    ptscSYSFLT->unSTATUS.u32Register = 0x00000000UL;
}

/*******************************************************************************
* Function Name: Cy_SysFlt_GetErrorSource
****************************************************************************//**
*
* \brief  Returns the source of error for the fault.
*
* \param  ptscSYSFLT : The pointer to a structure instance.
*
* \return see sysflt source
*
*******************************************************************************/
cy_en_sysflt_source_t Cy_SysFlt_GetErrorSource(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    if(ptscSYSFLT->unSTATUS.stcField.u1VALID == 1u)
    {
        return((cy_en_sysflt_source_t)(ptscSYSFLT->unSTATUS.stcField.u7IDX));
    }
    else
    {
        return CY_SYSFLT_NO_FAULT;
    }

}

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
uint32_t Cy_SysFlt_GetData0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unDATA[0].u32Register);
}

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
uint32_t Cy_SysFlt_GetData1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unDATA[1].u32Register);
}

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
uint32_t Cy_SysFlt_GetData2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unDATA[2].u32Register);
}

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
uint32_t Cy_SysFlt_GetData3(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unDATA[3].u32Register);
}

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
uint32_t Cy_SysFlt_Pending0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unPENDING0.u32Register);
}

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
uint32_t Cy_SysFlt_Pending1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unPENDING1.u32Register);
}

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
uint32_t Cy_SysFlt_Pending2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unPENDING2.u32Register);
}

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
void Cy_SysFlt_Mask0(volatile stc_FAULT_STRUCT_t *ptscSYSFLT, uint32_t enable)
{
    ptscSYSFLT->unMASK0.stcField.u32SOURCE = enable;
}

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
void Cy_SysFlt_Mask1(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,uint32_t enable)
{
     ptscSYSFLT->unMASK1.stcField.u32SOURCE = enable;
}

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
void Cy_SysFlt_Mask2(volatile stc_FAULT_STRUCT_t *ptscSYSFLT,uint32_t enable)
{
     ptscSYSFLT->unMASK2.stcField.u32SOURCE = enable;
}

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
void Cy_SysFlt_SetMaskByIdx(volatile stc_FAULT_STRUCT_t *ptscSYSFLT, cy_en_sysflt_source_t idx)
{
    switch(idx / 32)
    {
    case 0:
        ptscSYSFLT->unMASK0.u32Register |= (1UL << (uint32_t)idx);
        break;
    case 1:
        ptscSYSFLT->unMASK1.u32Register |= (1UL << ((uint32_t)idx - 32UL));
        break;
    case 2:
        ptscSYSFLT->unMASK2.u32Register |= (1UL << ((uint32_t)idx - 64UL));
        break;
    default:
        break;
    }
    return;
}

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
void Cy_SysFlt_ClearMaskByIdx(volatile stc_FAULT_STRUCT_t *ptscSYSFLT, cy_en_sysflt_source_t idx)
{
    switch(idx / 32)
    {
    case 0:
        ptscSYSFLT->unMASK0.u32Register &= ~(1UL << (uint32_t)idx);
        break;
    case 1:
        ptscSYSFLT->unMASK1.u32Register &= ~(1UL << ((uint32_t)idx - 32UL));
        break;
    case 2:
        ptscSYSFLT->unMASK2.u32Register &= ~(1UL << ((uint32_t)idx - 64UL));
        break;
    default:
        break;
    }
    return;
}


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
uint32_t Cy_SysFlt_GetInterruptStatus(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unINTR.stcField.u1FAULT);
}

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
void Cy_SysFlt_ClearInterrupt(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    ptscSYSFLT->unINTR.stcField.u1FAULT = true;
    (void)ptscSYSFLT->unINTR.u32Register;
}

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
void Cy_SysFlt_SetInterrupt(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    ptscSYSFLT->unINTR_SET.stcField.u1FAULT = true;
}

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
void Cy_SysFlt_SetInterruptMask(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    ptscSYSFLT->unINTR_MASK.stcField.u1FAULT = true;
}

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
uint32_t Cy_SysFlt_GetInterruptMask(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unINTR_MASK.u32Register);
}

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
uint32_t Cy_SysFlt_GetInterruptStatusMasked(volatile stc_FAULT_STRUCT_t *ptscSYSFLT)
{
    return(ptscSYSFLT->unINTR_MASKED.u32Register);
}

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
