/***************************************************************************//**
* \file
* \version 1.0
*
* \brief Trigger mux APIs.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "cy_trigmux.h"


/*******************************************************************************
* Function Name: Cy_TrigMux_IsOutput
****************************************************************************//**
*
* This function checks whether trigger parameter is output type or input type.
*
* \param trig
* An input/output trigger parameter to be checked.
*
* \return
* - true : input trigger parameter is output type 
* - false: input trigger parameter is input type 
*
********************************************************************************/
static bool Cy_TrigMux_IsOutput(uint32_t trig)
{
    // Check output bit field
    if((trig & CY_TR_OUT_MASK) == 0)
    {
        // Input trigger parameter
        return false;
    }
    else
    {
        // Output trigger parameter
        return true;
    }
}

/*******************************************************************************
* Function Name: Cy_TrigMux_IsOneToOne
****************************************************************************//**
*
* This function checks whether trigger parameter is for One-To-One trigger or not.
*
* \param trig
* An input/output trigger parameter to be checked.
*
* \return
* - true : input trigger parameter is for One-To-One 
* - false: input trigger parameter is not for One-To-One
*
********************************************************************************/
static bool Cy_TrigMux_IsOneToOne(uint32_t trig)
{
    // Check trigger type bit field
    if((trig & CY_TR_TYPE_MASK) == 0)
    {
        // Trigger type normal (not one to one)
        return false;
    }
    else
    {
        // Trigger type one to one
        return true;
    }
}

/*******************************************************************************
* Function Name: Cy_TrigMux_GetGroup
****************************************************************************//**
*
* This function distills group value from input trigger parameter.
*
* \param trig
* An input/output trigger parameter. group value of this parameter will be returned.
*
* \return
* group value of input trigger parameter.
*
********************************************************************************/
static uint8_t Cy_TrigMux_GetGroup(uint32_t trig)
{
    // Distill group field of input parameter
    return ((trig & CY_TR_GROUP_MASK) >> CY_TR_GROUP_SHIFT);
}

/*******************************************************************************
* Function Name: Cy_TrigMux_GetNo
****************************************************************************//**
*
* This function distills trigger No from input trigger parameter.
*
* \param trig
* An input/output trigger parameter. Trigger No of this parameter will be returned.
*
* \return
* Trigger No of input trigger parameter.
*
********************************************************************************/
static uint8_t Cy_TrigMux_GetNo(uint32_t trig)
{
    // Distill trigger selection field of input parameter
    return ((trig & CY_TR_MASK) >> CY_TR_SHIFT);
}

/*******************************************************************************
* Function Name: Cy_TrigMux_Connect
****************************************************************************//**
*
* This function connects an input trigger source and output trigger.
*
* \param inTrig
* An input selection for the trigger mux.
* - Bits 11:8 represent the trigger group selection.
* - Bits 7:0 select the input trigger number in the trigger group.
*
* \param outTrig
* The output of the trigger mux. This refers to the consumer of the trigger mux. 
* - Bits 11:8 represent the trigger group selection.
* - Bits 7:0 select the output trigger number in the trigger group.
*
* \param invert
* - CY_TR_MUX_TR_INV_ENABLE: The output trigger is inverted.
* - CY_TR_MUX_TR_INV_DISABLE: The output trigger is not inverted.
*
* \param trigType
* - TRIGGER_TYPE_EDGE: The trigger is synchronized to the consumer blocks clock
*   and a two-cycle pulse is generated on this clock.
* - TRIGGER_TYPE_LEVEL: The trigger is a simple level output.
*
* \param dbg_frz_en
* - 1: Output trigger is blocked in DBG mode
* - 0: Output trigger is not blocked in DBG mode
*
* \return
* A status
* - 0: Successful connection made.
* - 1: An invalid input selection corresponding to the output. 
*      Generally when the trigger groups do not match.
*  
********************************************************************************/
cy_en_trigmux_status_t Cy_TrigMux_Connect(uint32_t inTrig, uint32_t outTrig, uint32_t invert, en_trig_type_t trigType, uint32_t dbg_frz_en)
{
    cy_en_trigmux_status_t retVal = CY_TRIGMUX_BAD_PARAM;

    /* Validate input trigger */
    if(Cy_TrigMux_IsOutput(inTrig) == true)
    {
        /* Input trigger parameter is output type */
        return retVal;
    }

    /* Validate output trigger */
    if(Cy_TrigMux_IsOutput(outTrig) == false)
    {
        /* Output trigger parameter is input type */
        return retVal;
    }

    /* Distill group and trigger No value from input parameters */
    uint8_t inGroup   = Cy_TrigMux_GetGroup(inTrig);
    uint8_t outGroup  = Cy_TrigMux_GetGroup(outTrig);
    uint8_t inTrigNo  = Cy_TrigMux_GetNo(inTrig);
    uint8_t outTrigNo = Cy_TrigMux_GetNo(outTrig);

    /* Check if output and input group is same */
    if(inGroup != outGroup)
    {
        /* input group and output group mis-matched */
        return retVal;
    }

    /* Get a pointer to target trigger setting register */
    volatile stc_PERI_TR_GR_TR_CTL_field_t* pTR_CTL;
    pTR_CTL =   &(PERI->TR_GR[outGroup].unTR_CTL[outTrigNo].stcField);

    /* Set input parameters to the register */
    pTR_CTL->u8TR_SEL        = inTrigNo;
    pTR_CTL->u1TR_INV        = invert;
    pTR_CTL->u1TR_EDGE       = trigType;
    pTR_CTL->u1DBG_FREEZE_EN = dbg_frz_en;

    /* Return success status */
    retVal                   = CY_TRIGMUX_SUCCESS;
    return retVal;
}

/*******************************************************************************
* Function Name: Cy_TrigMux_Connect1To1
****************************************************************************//**
*
* This function connects an input trigger source and output trigger.
*
* \param trig
* The output or input of the trigger mux.
* This refers to the consumer of the trigger mux. 
* - Bits 11:8 represent the trigger group selection.
* - Bits 7:0 select the output trigger number in the trigger group.
*
* \param invert
* - CY_TR_MUX_TR_INV_ENABLE: The output trigger is inverted.
* - CY_MUX_TR_INV_DISABLE: The output trigger is not inverted.
*
* \param trigType
* - TRIGGER_TYPE_EDGE: The trigger is synchronized to the consumer blocks clock
*   and a two-cycle pulse is generated on this clock.
* - TRIGGER_TYPE_LEVEL: The trigger is a simple level output.
*
* \param dbg_frz_en
* - 1: Output trigger is blocked in DBG mode
*   0: Output trigger is not blocked in DBG mode
*
* \return
* A status
* - 0: Successful connection made.
* - 1: An invalid input selection corresponding to the output. 
*      Generally when the trigger groups do not match.
*  
*******************************************************************************/
cy_en_trigmux_status_t Cy_TrigMux_Connect1To1(uint32_t trig, uint32_t invert, en_trig_type_t trigType, uint32_t dbg_frz_en)
{
    cy_en_trigmux_status_t retVal = CY_TRIGMUX_BAD_PARAM;
    
    /* Validate output trigger */
    if(Cy_TrigMux_IsOneToOne(trig) == false)
    {
        /* input trigger parameter is not One-To-One type */
        return retVal;
    }

    /* Distill group and trigger No value from input trigger parameter */
    uint8_t trigGroup = Cy_TrigMux_GetGroup(trig);
    uint8_t trigNo    = Cy_TrigMux_GetNo(trig);

    /* Get a pointer to target trigger setting register */
    volatile stc_PERI_TR_1TO1_GR_TR_CTL_field_t* pTR_CTL;
    pTR_CTL = &(PERI->TR_1TO1_GR[trigGroup].unTR_CTL[trigNo].stcField);

    /* Set input parameters to the register */
    pTR_CTL->u1TR_SEL        = true;
    pTR_CTL->u1TR_INV        = invert;
    pTR_CTL->u1TR_EDGE       = trigType;
    pTR_CTL->u1DBG_FREEZE_EN = dbg_frz_en;

    /* Return success status */
    retVal = CY_TRIGMUX_SUCCESS;
    return retVal;
}

/*******************************************************************************
* Function Name: Cy_TrigMux_SwTrigger
****************************************************************************//**
*
* This function generates a software trigger on an input trigger line. 
* All output triggers connected to this input trigger will be triggered. 
* The function also verifies that there is no activated trigger before 
* generating another activation. 
*
* \param trigLine
* The input of the trigger mux.
* - Bits 11:8 represent the trigger group selection.
* - Bits 7:0 select the output trigger number in the trigger group.
*
* \param trigType
* - TRIGGER_TYPE_EDGE: The trigger is synchronized to the consumer blocks clock
*   and a two-cycle pulse is generated on this clock.
* - TRIGGER_TYPE_LEVEL: The trigger is a simple level output.
*
* \param outSel
* "Specifies whether trigger activation is for a specific input or ouput trigger 
* of the trigger multiplexer. Activation of a specific input trigger, 
* will result in activation of all output triggers that have the specific input 
* trigger selected through their TR_OUT_CTL.TR_SEL  field. Activation of a 
* specific output trigger, will result in activation of the specified TR_SEL 
* output trigger only.
* '0': TR_SEL selection and trigger activation is for an input trigger to the trigger multiplexer.
* '1': TR_SEL selection and trigger activation is for an output trigger from the trigger multiplexer.
* Note: this field is not used for trigger 1-to-1 groups."
* 
* \return
* A status
* - 0: If there was not an already activated trigger.
* - 1: If there was an already activated trigger.
*
*******************************************************************************/
cy_en_trigmux_status_t Cy_TrigMux_SwTrigger(uint32_t trigLine, en_trig_type_t trigType, uint32_t outSel)
{
    cy_en_trigmux_status_t retVal = CY_TRIGMUX_INVALID_STATE;
        
    if (PERI->unTR_CMD.stcField.u1ACTIVATE == 0)
    {
        PERI->unTR_CMD.stcField.u8TR_SEL     = Cy_TrigMux_GetNo(trigLine);
        PERI->unTR_CMD.stcField.u5GROUP_SEL  = Cy_TrigMux_GetGroup(trigLine);
        PERI->unTR_CMD.stcField.u1TR_EDGE    = trigType;
        PERI->unTR_CMD.stcField.u1OUT_SEL    = outSel;
        PERI->unTR_CMD.stcField.u1ACTIVATE   = 1; // Workaround: Rise ACTIVATE bit at last individually.

        retVal = CY_TRIGMUX_SUCCESS;
    }
    return retVal;
}

/* [] END OF FILE */
