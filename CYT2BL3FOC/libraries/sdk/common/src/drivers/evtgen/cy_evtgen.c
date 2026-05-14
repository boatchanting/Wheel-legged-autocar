/***************************************************************************//**
* \file cy_evtgen.c
* \version 1.0
*
* \brief
* Provides the public functions for the API for the Traveo II Event generator Driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_evtgen.h"
#include "syslib/cy_syslib.h"

static cy_stc_evtgen_struct_context_t *evtgenContext[EVTGEN_COMP_STRUCT_NR];
uint32_t mapUsed;
/*******************************************************************************
* Function Name: Cy_Evtgen_Initialize
****************************************************************************//**
*
* \brief Initialize the event generator
*
* \param config
* Pointer to the event generator config structure base address
*
* \return
* Event generator initialization status
*
*******************************************************************************/
cy_en_evtgendrv_status_t Cy_Evtgen_Initialize(volatile stc_EVTGEN_t *base, const cy_stc_evtgen_config_t* config)
{
    uint16_t refDiv;

    un_EVTGEN_RATIO_CTL_t ratioCtl;

    /* 1. Checking input parameter valid */
    if(config == NULL)
    {
        return CY_EVTGEN_ERR;
    }

    /* 2. Initialize internal variable */
    for(uint32_t i = 0; i < EVTGEN_COMP_STRUCT_NR; i++)
    {
        evtgenContext[i] = NULL;
    }
    mapUsed = 0;

    Cy_Evtgen_Enable(base);

    /* 2. Setting divider value of clk_ref */
    refDiv = config->frequencyRef / config->frequencyTick;
    if(config->frequencyRef % config->frequencyTick != 0)
    {
        return CY_EVTGEN_ERR;
    }
    else if(refDiv > 256 || refDiv  < 1)
    {
        return CY_EVTGEN_ERR;
    }
    else
    {
        base->unREF_CLOCK_CTL.stcField.u8INT_DIV = refDiv - 1u;
    }

    /* 3. Setting ratio operation */
    if(config->ratioControlMode == CY_EVTGEN_RATIO_CONTROL_SW)
    {
        /* SW controll: setting value for ratio value should be ratio between tick_ref_div and clk_lf. */
        uint64_t temp = (uint64_t)(config->frequencyRef / refDiv) << EVTGEN_RATIO_INT16_Pos;
        temp = temp / (uint64_t)(config->frequencyLf);
        base->unRATIO.u32Register = ((uint32_t)temp) & (EVTGEN_RATIO_INT16_Msk | EVTGEN_RATIO_FRAC8_Msk);

        base->unRATIO_CTL.stcField.u1DYNAMIC = 0u;

        /* SW controll: valid bit should be set manually. */
        base->unRATIO_CTL.stcField.u1VALID = 1u; /* Set VALID bit */
    }
    else
    {
        /* HW controll: */
        ratioCtl.u32Register = base->unRATIO_CTL.u32Register;
        ratioCtl.stcField.u1DYNAMIC = 1u; /* Set Dynamic bit */
        ratioCtl.stcField.u3DYNAMIC_MODE = config->ratioValueDynamicMode; /* Set Dynamic bit */
        base->unRATIO_CTL.u32Register = ratioCtl.u32Register; 

        /* Waiting until valid bit is set. */
        while(base->unRATIO_CTL.stcField.u1VALID == 0u);
    }

    /* Waiting until counter become ready. */
    while(base->unCOUNTER_STATUS.stcField.u1VALID == 0u);

    return CY_EVTGEN_OK;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_Deinitialize
****************************************************************************//**
*
* \brief Deinitialize event generator
*
*******************************************************************************/
void Cy_Evtgen_Deinitialize(volatile stc_EVTGEN_t *base)
{
    base->unCTL.u32Register = 0;
    base->unREF_CLOCK_CTL.u32Register = 0;
    base->unRATIO.u32Register = 0;
    base->unRATIO_CTL.u32Register = 0;
    base->unINTR_MASK.u32Register = 0;
    base->unINTR_DPSLP_MASK.u32Register = 0;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_InitializeCompStruct
****************************************************************************//**
*
* \brief Initialize a comparator structure
*
* \param config
* Number of structure to be initilized
*
* \param configStruct
* Pointer to the event generator structure config structure base address
*
* \param context
* Pointer to internal context
*
* \return
* Event generator initialization status
*
*******************************************************************************/
cy_en_evtgendrv_status_t Cy_Evtgen_InitializeCompStruct(volatile stc_EVTGEN_t *base, 
                                                        uint8_t structNum, 
                                                        const cy_stc_evtgen_struct_config_t* configStruct, 
                                                        cy_stc_evtgen_struct_context_t* context)
{
    un_EVTGEN_COMP_STRUCT_COMP_CTL_t compCtr;
    uint64_t tempCounterValue;
    uint32_t savedIntrStatus;

    /* Checking input parameter valid */
    if(configStruct == NULL)
    {
        return CY_EVTGEN_ERR;
    }
    
    if(structNum >= EVTGEN_COMP_STRUCT_NR)
    {
        return CY_EVTGEN_ERR;
    }
    
    if(configStruct->functionalitySelection != CY_EVTGEN_DEEPSLEEP_FUNCTIONALITY)
    {
        if(context == NULL)
        {
            return CY_EVTGEN_ERR;
        }
        evtgenContext[structNum] = context;
        evtgenContext[structNum]->addValueForCOMP0 = configStruct->valueActiveComparator;
        evtgenContext[structNum]->userIsr = configStruct->userIsr;
        mapUsed |= 1 << structNum;
    }

    compCtr.u32Register = base->COMP_STRUCT[structNum].unCOMP_CTL.u32Register;

    if(configStruct->functionalitySelection == CY_EVTGEN_DEEPSLEEP_FUNCTIONALITY)
    {
        compCtr.stcField.u1COMP1_EN = 1u;
    }

    compCtr.stcField.u1COMP0_EN = 1u;

    compCtr.stcField.u1TR_OUT_EDGE = configStruct->triggerOutEdge;

    compCtr.stcField.u1ENABLED = 1u;

    savedIntrStatus = Cy_SysLib_EnterCriticalSection();

    tempCounterValue = (uint64_t)Cy_Evtgen_GetCounterValue(base);

    /* Setting active comparator value */
    base->COMP_STRUCT[structNum].unCOMP0.stcField.u32INT32 = (uint32_t)(tempCounterValue + (uint64_t)configStruct->valueActiveComparator);

    /* Setting deep sleep comparator value */
    if(configStruct->functionalitySelection == CY_EVTGEN_DEEPSLEEP_FUNCTIONALITY)
    {
        base->COMP_STRUCT[structNum].unCOMP1.stcField.u32INT32 = (uint32_t)(tempCounterValue + (uint64_t)configStruct->valueDeepSleepComparator);
    }

    Cy_SysLib_ExitCriticalSection(savedIntrStatus);

    /* Setting comparator struct controll parameter */
    base->COMP_STRUCT[structNum].unCOMP_CTL.u32Register = compCtr.u32Register;

    Cy_Evtgen_SetInterruptMask(base, structNum);
    if(configStruct->functionalitySelection == CY_EVTGEN_DEEPSLEEP_FUNCTIONALITY)
    {
        Cy_Evtgen_SetInterruptDeepSleepMask(base, structNum);
    }

    return CY_EVTGEN_OK;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_DeinitializeCompStruct
****************************************************************************//**
*
* \brief Deinitialize event generator struct
*
* \param structNum
* Number of structure to be de-initilized
*
*******************************************************************************/
void Cy_Evtgen_DeinitializeCompStruct(volatile stc_EVTGEN_t *base, uint8_t structNum)
{
    base->COMP_STRUCT[structNum].unCOMP_CTL.u32Register = 0;
    base->COMP_STRUCT[structNum].unCOMP0.stcField.u32INT32 = 0;
    base->COMP_STRUCT[structNum].unCOMP1.stcField.u32INT32 = 0;
    evtgenContext[structNum] = NULL;
}

/*******************************************************************************
* Function Name: Cy_Evtgen_Interrupt
****************************************************************************//**
*
* \brief Interrupt handler for event generator
*
* \note Please call this function in the interrupt handler of event generator.
*       This function can clear interrupt factors multiple time therefore the user 
*       needs to clear pending status of NVIC at the end of the handler to avoid
*       hard fault exception.
*
*******************************************************************************/
void Cy_Evtgen_Interrupt(volatile stc_EVTGEN_t *base)
{
    uint64_t tempCompValue;
    uint32_t masked = base->unINTR_MASKED.u32Register;
    uint32_t temp_masked;
    uint32_t operatingStructNum = 0;

    masked &= mapUsed;
    temp_masked = masked;

    // Clear all interrupt flags used in active functionality.
    Cy_Evtgen_ClearInterrupt(base, temp_masked);

    while (temp_masked != 0)
    {
        if (temp_masked & 0x01)
        {
            tempCompValue = (uint64_t)base->COMP_STRUCT[operatingStructNum].unCOMP0.stcField.u32INT32;
            tempCompValue += (uint64_t)evtgenContext[operatingStructNum]->addValueForCOMP0;
            base->COMP_STRUCT[operatingStructNum].unCOMP_CTL.stcField.u1COMP0_EN = 0u;
            base->COMP_STRUCT[operatingStructNum].unCOMP0.stcField.u32INT32 = (uint32_t)tempCompValue;
            base->COMP_STRUCT[operatingStructNum].unCOMP_CTL.stcField.u1COMP0_EN = 1u; 
        }
        temp_masked >>= 1;
        operatingStructNum++;
    }

    operatingStructNum = 0;
    temp_masked = masked;
    while(temp_masked != 0)
    {
        if(temp_masked & 0x01)
        {
            if(evtgenContext[operatingStructNum]->userIsr != NULL)
            {
                evtgenContext[operatingStructNum]->userIsr();
            }
        }
        temp_masked >>= 1;
        operatingStructNum++;
    }

}

/* [] END OF FILE */
