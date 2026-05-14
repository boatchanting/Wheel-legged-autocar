/***************************************************************************//**
* \file cy_tcpwm_sr.c
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

#include "cy_tcpwm_sr.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_Init
****************************************************************************//**
*
* \brief  Initializes the TCPWM for the Shift register mode operation.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  config    :The pointer to configuration structure see \ref stc_tcpwm_counter_config_t.
*
* \return RET_SUCCESS   : Configuration completed successfully
*         RET_BAD_PARAM : One or more invalid parameters
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_sr_config_t const *config)
{
    uint32_t status = CY_RET_BAD_PARAM;

    if ((NULL != ptscTCPWM) && (NULL != config))
    {

        un_TCPWM_GRP_CNT_CTRL_t workCTRL = {.u32Register = 0ul};
        workCTRL.stcField.u1ONE_SHOT        = config->runMode;
        workCTRL.stcField.u3MODE            = CY_TCPWM_QUADDEC_CTRL_SR_MODE;
        workCTRL.stcField.u1DBG_FREEZE_EN   = config->debug_pause;
        workCTRL.stcField.u1AUTO_RELOAD_CC0 = config->enableCompare0Swap;
        workCTRL.stcField.u1AUTO_RELOAD_CC1 = config->enableCompare1Swap;
        ptscTCPWM->unCTRL.u32Register       = workCTRL.u32Register;

        ptscTCPWM->unDT.stcField.u8DT_LINE_OUT_L = config->clockPrescaler;

        ptscTCPWM->unCC0.u32Register             = config->compare0;

        ptscTCPWM->unCC0_BUFF.u32Register        = config->compare0_buff;

        ptscTCPWM->unCC1.u32Register             = config->compare1;

        ptscTCPWM->unCC1_BUFF.u32Register        = config->compare1_buff;

        ptscTCPWM->unPERIOD.u32Register          = config->period;

        ptscTCPWM->unPERIOD_BUFF.u32Register     = config->period_buff;

        un_TCPWM_GRP_CNT_TR_IN_SEL0_t workTR_IN_SEL0 = {.u32Register = 0ul};
        workTR_IN_SEL0.stcField.u8CAPTURE0_SEL = config->serialInInput;
        workTR_IN_SEL0.stcField.u8RELOAD_SEL   = config->reloadInput;
        workTR_IN_SEL0.stcField.u8STOP_SEL     = config->stopInput;
        workTR_IN_SEL0.stcField.u8COUNT_SEL    = config->shiftInput;
        ptscTCPWM->unTR_IN_SEL0.u32Register    = workTR_IN_SEL0.u32Register;

        ptscTCPWM->unTR_IN_SEL1.stcField.u8START_SEL = config->startInput;

        un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_t workTR_IN_EDGE_SEL = {.u32Register = 0ul};
        workTR_IN_EDGE_SEL.stcField.u2CAPTURE0_EDGE = config->serialInInputMode;
        workTR_IN_EDGE_SEL.stcField.u2RELOAD_EDGE   = config->reloadInputMode;
        workTR_IN_EDGE_SEL.stcField.u2START_EDGE    = config->startInputMode;
        workTR_IN_EDGE_SEL.stcField.u2STOP_EDGE     = config->stopInputMode;
        workTR_IN_EDGE_SEL.stcField.u2COUNT_EDGE    = config->shiftInputMode;
        ptscTCPWM->unTR_IN_EDGE_SEL.u32Register     = workTR_IN_EDGE_SEL.u32Register;

        //un_TCPWM_GRP_CNT_TR_OUT_SEL_t workTR_OUT_SEL = {.u32Register = 0ul};
        //workTR_OUT_SEL.stcField.u3OUT0      = 0x05ul;
        //workTR_OUT_SEL.stcField.u3OUT1      = 0x05ul;
        //ptscTCPWM->unTR_OUT_SEL.u32Register = workTR_OUT_SEL.u32Register;
        
        un_TCPWM_GRP_CNT_TR_OUT_SEL_t workTR_OUT_SEL = {.u32Register = 0ul};
        workTR_OUT_SEL.stcField.u3OUT0      = config->trigger0EventCfg;
        workTR_OUT_SEL.stcField.u3OUT1      = config->trigger1EventCfg;
        ptscTCPWM->unTR_OUT_SEL.u32Register = workTR_OUT_SEL.u32Register;

        un_TCPWM_GRP_CNT_LINE_SEL_t workLINE_SEL = {.u32Register = 0ul};
        workLINE_SEL.stcField.u3OUT_SEL       = 0x02;
        workLINE_SEL.stcField.u3COMPL_OUT_SEL = 0x03;
        ptscTCPWM->unLINE_SEL.u32Register = workLINE_SEL.u32Register;

        ptscTCPWM->unINTR_MASK.u32Register = config->interruptSources;

       status = CY_RET_SUCCESS;

    }

    return(status);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_DeInit
****************************************************************************//**
*
* \breif  De-initializes the TCPWM block, returns register values to default.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
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
* Function Name: Cy_Tcpwm_Sr_Enable
****************************************************************************//**
*
* \brief  Enables the TCPWM block for the Counter operation.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_Disable
****************************************************************************//**
*
* \brief  Disables the TCPWM block.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x0ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetStatus
****************************************************************************//**
*
* \brief  Returns the status of the Shift Register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The status, see \ref group_tcpwm_counter_status
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unSTATUS.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCapture0
****************************************************************************//**
*
* \brief  Returns the capture value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCapture0Buf
****************************************************************************//**
*
* \brief  Returns the buffered capture value when the capture mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The buffered capture value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCapture0Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_SetCompare0
****************************************************************************//**
*
* \brief  Sets the compare value for Compare0 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare0  : The compare value - 0 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC0.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCompare0
****************************************************************************//**
*
* \brief  Returns compare value 0.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 0.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Counter_SetCompare0_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare1 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare1  : The compare value - the 1 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC0_BUFF.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCompare0_Buff
****************************************************************************//**
*
* \brief  Returns compare value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_EnableCompare0Swap
****************************************************************************//**
*
* \brief  Enables the comparison swap when the comparison value is true.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  enable   : true = swap enabled, false = swap disabled
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
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
* Function Name: Cy_Tcpwm_Sr_SetCompare1
****************************************************************************//**
*
* \brief  Sets the compare value for Compare0 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare0  : The compare value - 0 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC1.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCompare1
****************************************************************************//**
*
* \brief  Returns compare value 0.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 0.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (ptscTCPWM->unCC1.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Sr_SetCompare1_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare1 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare1  : The compare value - the 1 value.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC1_BUFF.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetCompare1_Buff
****************************************************************************//**
*
* \brief  Returns compare value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC1_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_SetShiftRegister
****************************************************************************//**
*
* \brief  Sets the value of the counter.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  sr        : The value to write into the shift register.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetShiftRegister(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t sr)
{
    ptscTCPWM->unCOUNTER.u32Register = sr;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetShiftRegister
****************************************************************************//**
*
* \brief  Returns the value in the Shift Register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The current shift register value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetShiftRegister(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCOUNTER.u32Register);
}

/*******************************************************************************
* Function Name: Cy_TCPWM_Sr_SetPeriod_Buff
****************************************************************************//**
*
* \brief  Sets the value of the period buffer register which is used to know counter taps.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  period :  The value to write into a period_buff.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Sr_SetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period)
{
    ptscTCPWM->unPERIOD_BUFF.u32Register = period;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Sr_GetPeriod_Buff
****************************************************************************//**
*
* \brief  Returns the value in the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The current period value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Sr_GetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unPERIOD_BUFF.u32Register);
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
