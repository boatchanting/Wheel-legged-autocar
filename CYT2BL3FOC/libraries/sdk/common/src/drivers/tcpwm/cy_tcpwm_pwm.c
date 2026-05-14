/***************************************************************************//**
* \file cy_tcpwm_pwm.c
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

#include "cy_tcpwm_pwm.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_Init
****************************************************************************//**
*
* \brief  Initializes the TCPWM for the PWM operation.
*
* \param  ptscTCPWM The pointer to a counter instance.
*                   Group0 - 16-bit timer(63 counters, 0-62)
*                   Group1 - 16-bit timer with motor control(12 counters, 0-11)
*                   Group2 - 32-bit timer(4 counters, 0-3)
*                   (Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  config The pointer to a configuration structure see \ref cy_stc_tcpwm_pwm_config_t.
*
* \return CY_RET_SUCCESS: configuration completed successfully
*         CY_RET_BAD_PARAM: One or more invalid parameters
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_Init(volatile stc_TCPWM_GRP_CNT_t* ptscTCPWM, cy_stc_tcpwm_pwm_config_t const* config)
{
    uint32_t status = CY_RET_BAD_PARAM;

    if((NULL != ptscTCPWM) && (NULL != config))
    {

        un_TCPWM_GRP_CNT_CTRL_t workCTRL = {.u32Register = 0ul};
        workCTRL.stcField.u1ONE_SHOT             = config->runMode;
        workCTRL.stcField.u2UP_DOWN_MODE         = config->countDirection;
        workCTRL.stcField.u3MODE                 = config->pwmMode;
        workCTRL.stcField.u1DBG_FREEZE_EN        = config->debug_pause;
        workCTRL.stcField.u1AUTO_RELOAD_CC0      = config->enableCompare0Swap;
        workCTRL.stcField.u1AUTO_RELOAD_CC1      = config->enableCompare1Swap;
        workCTRL.stcField.u1AUTO_RELOAD_PERIOD   = config->enablePeriodSwap;
        workCTRL.stcField.u1AUTO_RELOAD_LINE_SEL = config->enableLineSelSwap;
        workCTRL.stcField.u1PWM_SYNC_KILL        = config->killMode;
        workCTRL.stcField.u1PWM_STOP_ON_KILL     = (config->killMode>>1ul);
        workCTRL.stcField.u1PWM_IMM_KILL         = config->immediateKill;
        workCTRL.stcField.u2PWM_DISABLE_MODE     = config->pwmOnDisable;
        ptscTCPWM->unCTRL.u32Register            = workCTRL.u32Register;

        if(CY_TCPWM_COUNTER_COUNT_UP == config->runMode)
        {
            ptscTCPWM->unCOUNTER.u32Register = CY_TCPWM_CNT_UP_INIT_VAL;
        }
        else if(CY_TCPWM_COUNTER_COUNT_DOWN == config->runMode)
        {
            ptscTCPWM->unCOUNTER.u32Register = config->period;
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

        // Should not be set here (YOTS)
        ptscTCPWM->unPERIOD_BUFF.u32Register = config->period_buff;

        un_TCPWM_GRP_CNT_TR_IN_SEL0_t workTR_IN_SEL0 = {.u32Register = 0ul};
        workTR_IN_SEL0.stcField.u8CAPTURE0_SEL = config->switchInput;
        workTR_IN_SEL0.stcField.u8RELOAD_SEL   = config->reloadInput;
        workTR_IN_SEL0.stcField.u8STOP_SEL     = config->kill0Input;
        workTR_IN_SEL0.stcField.u8COUNT_SEL    = config->countInput;
        ptscTCPWM->unTR_IN_SEL0.u32Register    = workTR_IN_SEL0.u32Register;

        un_TCPWM_GRP_CNT_TR_IN_SEL1_t workTR_IN_SEL1 = {.u32Register = 0ul};
        workTR_IN_SEL1.stcField.u8CAPTURE1_SEL = config->kill1Input;
        workTR_IN_SEL1.stcField.u8START_SEL    = config->startInput;
        ptscTCPWM->unTR_IN_SEL1.u32Register    = workTR_IN_SEL1.u32Register;

        un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_t workTR_IN_EDGE_SEL = {.u32Register = 0ul};
        workTR_IN_EDGE_SEL.stcField.u2CAPTURE0_EDGE = config->switchInputMode;
        workTR_IN_EDGE_SEL.stcField.u2CAPTURE1_EDGE = config->kill1InputMode;
        workTR_IN_EDGE_SEL.stcField.u2RELOAD_EDGE   = config->reloadInputMode;
        workTR_IN_EDGE_SEL.stcField.u2START_EDGE    = config->startInputMode;
        workTR_IN_EDGE_SEL.stcField.u2STOP_EDGE     = config->kill0InputMode;
        workTR_IN_EDGE_SEL.stcField.u2COUNT_EDGE    = config->countInputMode;
        ptscTCPWM->unTR_IN_EDGE_SEL.u32Register     = workTR_IN_EDGE_SEL.u32Register;
        
        un_TCPWM_GRP_CNT_TR_OUT_SEL_t workTR_OUT_SEL = {.u32Register = 0ul};
        workTR_OUT_SEL.stcField.u3OUT0      = config->trigger0EventCfg;
        workTR_OUT_SEL.stcField.u3OUT1      = config->trigger1EventCfg;
        ptscTCPWM->unTR_OUT_SEL.u32Register = workTR_OUT_SEL.u32Register;

        ptscTCPWM->unINTR_MASK.u32Register = config->interruptSources;

        un_TCPWM_GRP_CNT_TR_PWM_CTRL_t workTR_PWM_CTRL = {.u32Register = 0ul};
        workTR_PWM_CTRL.stcField.u2CC0_MATCH_MODE = config->cc0MatchMode;
        workTR_PWM_CTRL.stcField.u2CC1_MATCH_MODE = config->cc1MatchMode;
        workTR_PWM_CTRL.stcField.u2OVERFLOW_MODE  = config->overflowMode;
        workTR_PWM_CTRL.stcField.u2UNDERFLOW_MODE = config->underflowMode;
        ptscTCPWM->unTR_PWM_CTRL.u32Register      = workTR_PWM_CTRL.u32Register;

        un_TCPWM_GRP_CNT_DT_t workDT = {.u32Register = 0ul};
        workDT.stcField.u16DT_LINE_COMPL_OUT = config->deadTimeComp;
        workDT.stcField.u8DT_LINE_OUT_H      = (config->deadTime>>8ul);
        if(config->pwmMode == CY_TCPWM_PWM_MODE_DEADTIME)
        {
            workDT.stcField.u8DT_LINE_OUT_L = config->deadTime;
        }
        else
        {
            workDT.stcField.u8DT_LINE_OUT_L = config->clockPrescaler;
        }
        ptscTCPWM->unDT.u32Register          = workDT.u32Register;

        status = CY_RET_SUCCESS;

    }

    return (status);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_DeInit
****************************************************************************//**
*
* \brief  De-initializes the TCPWM block, returns register values to default.
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
void Cy_Tcpwm_Pwm_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
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
* Function Name: Cy_Tcpwm_Pwm_Enable
****************************************************************************//**
*
* \brief  Enables the TCPWM block for the PWM operation.
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
void Cy_Tcpwm_Pwm_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_Disable
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
void Cy_Tcpwm_Pwm_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unCTRL.stcField.u1ENABLED = 0x0ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetStatus
****************************************************************************//**
*
* \brief  Returns the status of the PWM.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return The status, see \ref group_tcpwm_pwm_status
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unSTATUS.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetCompare0
****************************************************************************//**
*
* \brief  Sets the compare value for Compare0 when the compare mode enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare0 : The compare value - value 0.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC0.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetCompare0
****************************************************************************//**
*
* \brief  Returns compare value 0.
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
uint32_t Cy_Tcpwm_Pwm_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetCompare0_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare1 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare0  : The compare value - value 1.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0)
{
    ptscTCPWM->unCC0_BUFF.u32Register = compare0;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetCompare0_Buff
****************************************************************************//**
*
* \brief  Returns compare value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCC0_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_EnableCompare0Swap
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
void Cy_Tcpwm_Pwm_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
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
* Function Name: Cy_Tcpwm_Pwm_SetCompare1
****************************************************************************//**
*
* \brief  Sets the compare value for Compare0 when the compare mode enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  param compare1 : The compare value - value 1.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC1.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetCompare1
****************************************************************************//**
*
* \brief  Returns compare value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (ptscTCPWM->unCC1.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetCompare1_Buff
****************************************************************************//**
*
* \brief  Sets the compare value for Compare1 when the compare mode is enabled.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  compare1  : The compare value - value 1.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1)
{
    ptscTCPWM->unCC1_BUFF.u32Register = compare1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetCompare1_Buff
****************************************************************************//**
*
* \brief  Returns compare value 1.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Compare value 1.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (ptscTCPWM->unCC1_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_EnableCompare1Swap
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
void Cy_Tcpwm_Pwm_EnableCompare1Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
{
    if (enable)
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC1 =  0x1ul;
    }
    else
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_CC1 =  0x0ul;
    }
}


/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetCounter
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
void Cy_Tcpwm_Pwm_SetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t count)
{
    ptscTCPWM->unCOUNTER.u32Register = count;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetCounter
****************************************************************************//**
*
* \brief  Returns the value in the counter.
*
* \param  ptscTCPWM : The pointer to a TCPWM instance.
*
* \return The current counter value.
*
*******************************************************************************/
uint32_t Cy_Tcpwm_Pwm_GetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unCOUNTER.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetPeriod
****************************************************************************//**
*
* \brief  Sets the value of the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  period    : The value to write into a period.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period)
{
    ptscTCPWM->unPERIOD.u32Register = period;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetPeriod
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
uint32_t Cy_Tcpwm_Pwm_GetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unPERIOD.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_SetPeriodBuf
****************************************************************************//**
*
* \brief  Sets the value of the period register.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*  \param  period1   : The value to write into a period1.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_SetPeriodBuf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period1)
{
    ptscTCPWM->unPERIOD_BUFF.u32Register = period1;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_GetPeriodBuf
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
uint32_t Cy_Tcpwm_Pwm_GetPeriodBuf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return (ptscTCPWM->unPERIOD_BUFF.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_EnablePeriodSwap
****************************************************************************//**
*
* \brief  Enables a period swap on OV and/or UN, depending on the PWM alignment
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*  \param  enable    : true: enable period swap. false: disable period swap.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_EnablePeriodSwap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable)
{
    if (enable)
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_PERIOD =  0x1ul;
    }
    else
    {
        ptscTCPWM->unCTRL.stcField.u1AUTO_RELOAD_PERIOD =  0x0ul;
    }

}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_LeftAlign
****************************************************************************//**
*
* \brief  Set PWM alignment in Left aligned mode
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
void Cy_Tcpwm_Pwm_LeftAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2CC0_MATCH_MODE = CY_TCPWM_PWM_TR_CTRL2_CLEAR;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2OVERFLOW_MODE  = CY_TCPWM_PWM_TR_CTRL2_SET;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2UNDERFLOW_MODE = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_RightAlign
****************************************************************************//**
*
* \brief  Set PWM alignment in Right aligned mode
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
void Cy_Tcpwm_Pwm_RightAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2CC0_MATCH_MODE = CY_TCPWM_PWM_TR_CTRL2_SET;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2OVERFLOW_MODE  = CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2UNDERFLOW_MODE = CY_TCPWM_PWM_TR_CTRL2_CLEAR;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_CentreAlign
****************************************************************************//**
*
* \brief  Set PWM alignment in Center aligned mode
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
void Cy_Tcpwm_Pwm_CentreAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2CC0_MATCH_MODE = CY_TCPWM_PWM_TR_CTRL2_INVERT;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2OVERFLOW_MODE  = CY_TCPWM_PWM_TR_CTRL2_SET;
    ptscTCPWM->unTR_PWM_CTRL.stcField.u2UNDERFLOW_MODE = CY_TCPWM_PWM_TR_CTRL2_CLEAR;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_Pwm_LSFRConfig
****************************************************************************//**
*
* \brief  Sets the length of LSFR i.e. GRP_CNT_WIDTH.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*              Group0 - 16-bit timer(63 counters, 0-62)
*              Group1 - 16-bit timer with motor control(12 counters, 0-11)
*              Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*  \param  tap_value : TAP value to be programmed to PERIOD_BUFF register
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_Pwm_LSFRConfig(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t tap_value)
{
    ptscTCPWM->unPERIOD_BUFF.u32Register = tap_value;
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
