/***************************************************************************//**
* \file tcpwm_counter.c
* \version 1.0
*
* \brief
*  The source file of the tcpwm driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_tcpwm_counter.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_Init
****************************************************************************//**
*
* \brief  Initializes the TCPWM for the Counter operation.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  config    :The pointer to configuration structure see \ref cy_stc_tcpwm_counter_config_t.
*
* \return CY_RET_SUCCESS   : Configuration completed successfully
*         CY_RET_BAD_PARAM : One or more invalid parameters
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_counter_config_t const *config)
{
    uint32_t status = CY_RET_BAD_PARAM;

    if (config->trigger0EventCfg > 0x04ul || config->trigger1EventCfg > 0x04ul)
    {
        return status;
    }    

    if ((NULL != ptscTCPWM) && (NULL != config))
    {
        un_TCPWM_GRP_CNT_CTRL_t workCTRL    = { .u32Register = 0ul };
        workCTRL.stcField.u1ONE_SHOT        = config->runMode;
        workCTRL.stcField.u2UP_DOWN_MODE    = config->countDirection;
        workCTRL.stcField.u3MODE            = config->compareOrCapture;
        workCTRL.stcField.u1DBG_FREEZE_EN   = config->debug_pause;
        workCTRL.stcField.u1AUTO_RELOAD_CC0 = config->enableCompare0Swap;
        workCTRL.stcField.u1AUTO_RELOAD_CC1 = config->enableCompare1Swap;
        ptscTCPWM->unCTRL.u32Register       = workCTRL.u32Register;

        ptscTCPWM->unDT.stcField.u8DT_LINE_OUT_L     = config->clockPrescaler;

        // Need to check (AKSF)
        if (CY_TCPWM_COUNTER_ONESHOT == config->runMode)
        {
            if (CY_TCPWM_COUNTER_COUNT_UP == config->countDirection)
            {
                ptscTCPWM->unCOUNTER.u32Register = CY_TCPWM_CNT_UP_INIT_VAL;
            }
            else if (CY_TCPWM_COUNTER_COUNT_DOWN == config->countDirection)
            {
                ptscTCPWM->unCOUNTER.u32Register = config->period;
            }
        }
        else
        {
            ptscTCPWM->unCOUNTER.u32Register = CY_TCPWM_CNT_UP_DOWN_INIT_VAL;
        }

        ptscTCPWM->unCC0.u32Register = config->compare0;

        // Should not be set here (YOTS)
        ptscTCPWM->unCC0_BUFF.u32Register = config->compare0_buff;

        ptscTCPWM->unCC1.u32Register = config->compare1;

        // Should not be set here (YOTS)
        ptscTCPWM->unCC1_BUFF.u32Register = config->compare1_buff;

        ptscTCPWM->unPERIOD.u32Register = config->period;

        un_TCPWM_GRP_CNT_TR_IN_SEL0_t workTR_IN_SEL0 = {.u32Register = 0ul};
        workTR_IN_SEL0.stcField.u8CAPTURE0_SEL = config->capture0Input;
        workTR_IN_SEL0.stcField.u8RELOAD_SEL   = config->reloadInput;
        workTR_IN_SEL0.stcField.u8STOP_SEL     = config->stopInput;
        workTR_IN_SEL0.stcField.u8COUNT_SEL    = config->countInput;
        ptscTCPWM->unTR_IN_SEL0.u32Register    = workTR_IN_SEL0.u32Register;

        un_TCPWM_GRP_CNT_TR_IN_SEL1_t workTR_IN_SEL1 = {.u32Register = 0ul};
        workTR_IN_SEL1.stcField.u8START_SEL    = config->startInput;
        workTR_IN_SEL1.stcField.u8CAPTURE1_SEL = config->capture1Input;
        ptscTCPWM->unTR_IN_SEL1.u32Register    = workTR_IN_SEL1.u32Register;

        un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_t workTR_IN_EDGE_SEL = {.u32Register = 0ul};
        workTR_IN_EDGE_SEL.stcField.u2CAPTURE0_EDGE = config->capture0InputMode;
        workTR_IN_EDGE_SEL.stcField.u2CAPTURE1_EDGE = config->capture1InputMode;
        workTR_IN_EDGE_SEL.stcField.u2RELOAD_EDGE   = config->reloadInputMode;
        workTR_IN_EDGE_SEL.stcField.u2START_EDGE    = config->startInputMode;
        workTR_IN_EDGE_SEL.stcField.u2STOP_EDGE     = config->stopInputMode;
        workTR_IN_EDGE_SEL.stcField.u2COUNT_EDGE    = config->countInputMode;
        ptscTCPWM->unTR_IN_EDGE_SEL.u32Register     = workTR_IN_EDGE_SEL.u32Register;

        un_TCPWM_GRP_CNT_TR_OUT_SEL_t workTR_OUT_SEL = {.u32Register = 0ul};
        workTR_OUT_SEL.stcField.u3OUT0      = config->trigger0EventCfg;
        workTR_OUT_SEL.stcField.u3OUT1      = config->trigger1EventCfg;
        ptscTCPWM->unTR_OUT_SEL.u32Register = workTR_OUT_SEL.u32Register;

        ptscTCPWM->unINTR_MASK.u32Register = config->interruptSources;

        status = CY_RET_SUCCESS;
    }

    return(status);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_DeInit
****************************************************************************//**
*
* \breif  De-initializes the TCPWM block, returns register values to default.
*
* \param ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{

    ptscTCPWM->unCTRL.u32Register           = CY_TCPWM_GRP_CNT_CTRL_DEFAULT;
    ptscTCPWM->unCOUNTER.u32Register        = CY_TCPWM_GRP_CNT_COUNTER_DEFAULT;
    ptscTCPWM->unCC0.u32Register            = CY_TCPWM_GRP_CNT_CC0_DEFAULT;
    ptscTCPWM->unCC0_BUFF.u32Register       = CY_TCPWM_GRP_CNT_CC0_BUFF_DEFAULT;
    ptscTCPWM->unCC1.u32Register            = CY_TCPWM_GRP_CNT_CC1_DEFAULT;
    ptscTCPWM->unCC1_BUFF.u32Register       = CY_TCPWM_GRP_CNT_CC1_BUFF_DEFAULT;
    ptscTCPWM->unPERIOD.u32Register         = CY_TCPWM_GRP_CNT_PERIOD_DEFAULT;
    ptscTCPWM->unPERIOD_BUFF.u32Register    = CY_TCPWM_GRP_CNT_PERIOD_BUFF_DEFAULT;
    ptscTCPWM->unTR_IN_SEL0.u32Register     = CY_TCPWM_GRP_CNT_TR_IN_SEL0_DEFAULT;
    ptscTCPWM->unTR_IN_SEL1.u32Register     = CY_TCPWM_GRP_CNT_TR_IN_SEL1_DEFAULT;
    ptscTCPWM->unTR_IN_EDGE_SEL.u32Register = CY_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_DEFAULT;
    ptscTCPWM->unTR_OUT_SEL.u32Register     = CY_TCPWM_GRP_CNT_TR_OUT_SEL_DEFAULT;
    ptscTCPWM->unTR_PWM_CTRL.u32Register    = CY_TCPWM_GRP_CNT_TR_PWM_CTRL_DEFAULT;
    ptscTCPWM->unINTR.u32Register           = CY_TCPWM_GRP_CNT_INTR_DEFAULT;
    ptscTCPWM->unINTR_SET.u32Register       = CY_TCPWM_GRP_CNT_INTR_SET_DEFAULT;
    ptscTCPWM->unINTR_MASK.u32Register      = CY_TCPWM_GRP_CNT_INTR_MASK_DEFAULT;

}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_Enable
****************************************************************************//**
*
* \brief  Enables the TCPWM block for the Counter operation.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_Disable
****************************************************************************//**
*
* \brief  Disables the TCPWM block.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x0ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetStatus
****************************************************************************//**
*
* \brief  Returns the status of the Counter Timer.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The status, see \ref group_tcpwm_counter_status
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
   return(ptscTCPWM->unSTATUS.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCapture0
****************************************************************************//**
*
* \brief  Returns the capture0 value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCapture1
****************************************************************************//**
*
* \brief  Returns the capture1 value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \return The capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCapture1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC1.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCapture0Buf
****************************************************************************//**
*
* \brief  Returns the buffered capture0 value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The buffered capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCapture0Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    // Need to check this function makes sense (YOTS).
    return(ptscTCPWM->unCC0_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCapture1Buf
****************************************************************************//**
*
* \brief  Returns the buffered capture1 value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The buffered capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCapture1Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    // Need to check this function makes sense (YOTS).
    return(ptscTCPWM->unCC1_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetCompare0
****************************************************************************//**
*
* \brief  Sets the compare0 value for Compare0 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*      compare0  : The compare value - 0 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC0.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCompare0
****************************************************************************//**
*
* \brief  Returns compare0 value 0.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 0.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Counter_SetCompare0_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare0_Buff when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*      compare1  : The compare value - the 1 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC0_BUFF.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCompare0_Buff
****************************************************************************//**
*
* \brief  Returns compare0_Buff value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selcted and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetCompare1
****************************************************************************//**
*
* \brief  Sets the compare1 value for Compare0 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*      compare0  : The compare value - 0 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC1.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCompare1
****************************************************************************//**
*
* \brief  Returns compare1 value 0.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 0.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC1.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Counter_SetCompare1_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare1_Buff when the compare mode is enabled.
*
* \param ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*      compare1  : The compare value - the 1 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC1_BUFF.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCompare1_Buff
****************************************************************************//**
*
* \brief  Returns compare1_Buff value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selcted and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC1_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_EnableCompare0Swap
****************************************************************************//**
*
* \brief  Enables the comparison swap when the comparison value is true.
*
* \param ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param enable    : true = swap enabled, false = swap disabled
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
{
    if (enable)
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC0 =  0x1ul;
    }
    else
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC0 =  0x0ul;
    }
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_EnableCompare1Swap
****************************************************************************//**
*
* \brief  Enables the comparison swap when the comparison value is true.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  enable    : true = swap enabled, false = swap disabled
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_EnableCompare1Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
{
    if (enable)
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC1 = 0x1ul;
    }
    else
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC1 = 0x0ul;
    }
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetCounter
****************************************************************************//**
*
* \brief  Sets the value of the counter.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  count     : The value to write into the counter.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t count)
{
    ptscTCPWM->unCOUNTER.u32Register = count;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCounter
****************************************************************************//**
*
* \brief  Returns the value in the counter.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The current counter value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCOUNTER.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Counter_SetPeriod
****************************************************************************//**
*
* \brief  Sets the value of the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  period    :  The value to write into a period.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period)
{
   ptscTCPWM->unPERIOD.u32Register = period;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetPeriod
****************************************************************************//**
*
* \brief  Returns the value in the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The current period value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unPERIOD.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Counter_SetPeriod_Buff
****************************************************************************//**
*
* \brief  Sets the value of the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*      period    :  The value to write into a period.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period)
{
    ptscTCPWM->unPERIOD_BUFF.u32Register = period;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetPeriod_Buff
****************************************************************************//**
*
* \brief  Returns the value in the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The current period value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Counter_GetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unPERIOD_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetTC_IntrMask
****************************************************************************//**
*
* \brief  Set TC interrupt mask.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetTC_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR_MASK.stcField.u1TC = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetCC0_IntrMask
****************************************************************************//**
*
* \brief  Set CCO interrupt mask.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selcted and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCC0_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR_MASK.stcField.u1CC0_MATCH = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_SetCC1_IntrMask
****************************************************************************//**
*
* \brief  Set CC1 interrupt mask.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_SetCC1_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR_MASK.stcField.u1CC1_MATCH = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetTC_IntrMasked
****************************************************************************//**
*
* \brief  Get TC interrupt masked.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
uint8_t Cy_Tcpwm_Counter_GetTC_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (uint8_t)(ptscTCPWM->unINTR_MASKED.stcField.u1TC);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCC0_IntrMasked
****************************************************************************//**
*
* \brief  Get CC0 interrupt masked.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
uint8_t Cy_Tcpwm_Counter_GetCC0_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (uint8_t)(ptscTCPWM->unINTR_MASKED.stcField.u1CC0_MATCH);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_GetCC1_IntrMasked
****************************************************************************//**
*
* \brief  Get CC1 interrupt masked.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
uint8_t Cy_Tcpwm_Counter_GetCC1_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (uint8_t)(ptscTCPWM->unINTR_MASKED.stcField.u1CC1_MATCH);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_ClearTC_Intr
****************************************************************************//**
*
* \brief  Clear TC interrupt.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_ClearTC_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR.stcField.u1TC = 1ul;
    ptscTCPWM->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_ClearCC0_Intr
****************************************************************************//**
*
* \brief  Clear CC0 interrupt.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_ClearCC0_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR.stcField.u1CC0_MATCH = 1ul;
    ptscTCPWM->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Counter_ClearCC1_Intr
****************************************************************************//**
*
* \brief  Clear CC1 interrupt.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
*******************************************************************************/
void Cy_Tcpwm_Counter_ClearCC1_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unINTR.stcField.u1CC1_MATCH = 1ul;
    ptscTCPWM->unINTR.u32Register;
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
