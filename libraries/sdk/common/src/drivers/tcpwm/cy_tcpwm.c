/***************************************************************************//**
* \file cy_tcpwm.c
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

#include "cy_tcpwm.h"

#if defined(__cplusplus)
extern "C" {
#endif




/*******************************************************************************
* Function Name: Cy_Tcpwm_TriggerStart
****************************************************************************//**
*
* \brief  Triggers a software start on the selected TCPWMs.
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
void Cy_Tcpwm_TriggerStart(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_CMD.stcField.u1START = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_TriggerReloadOrIndex
****************************************************************************//**
*
* \brief  Triggers a software reload event (or index in QuadDec mode).
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
void Cy_Tcpwm_TriggerReloadOrIndex(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_CMD.stcField.u1RELOAD = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_TriggerStopOrKill
****************************************************************************//**
*
* \brief  Triggers a stop in the Timer Counter mode, or a kill in the PWM mode.
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
void Cy_Tcpwm_TriggerStopOrKill(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_CMD.stcField.u1STOP = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_TriggerCapture0
****************************************************************************//**
*
* \brief  Triggers a Capture0 in the Timer Counter mode, and a Swap in the PWM mode.
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
void Cy_Tcpwm_TriggerCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_CMD.stcField.u1CAPTURE0 = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_TriggerCapture1
****************************************************************************//**
*
* \brief  Triggers a Capture1 in the Timer Counter mode, and a Swap in the PWM mode.
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
void Cy_Tcpwm_TriggerCapture1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    ptscTCPWM->unTR_CMD.stcField.u1CAPTURE1 = 0x1ul;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_GetInterruptStatus
****************************************************************************//**
*
* \brief  Returns which event triggered the interrupt.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return see \ref cy_en_tcpwm_interrupt_sources
*
*******************************************************************************/
uint32_t Cy_Tcpwm_GetInterruptStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unINTR.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_ClearInterrupt
****************************************************************************//**
*
* \brief  Clears Active Interrupt Source
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  source   : source to clear see \ref cy_en_tcpwm_interrupt_sources
*
*******************************************************************************/
void Cy_Tcpwm_ClearInterrupt(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t source)
{
    ptscTCPWM->unINTR.u32Register = source;
	(void)ptscTCPWM->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_SetInterrupt
****************************************************************************//**
*
* \brief  Triggers an interrupt via a software write.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  source    : The source to set an interrupt of see \ref group_tcpwm_interrupt_sources.
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_SetInterrupt(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t source)
{
    ptscTCPWM->unINTR_SET.u32Register = source;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_SetInterruptMask
****************************************************************************//**
*
* \brief  Sets an interrupt mask. A 1 means that when the event occurs, it will cause an interrupt;
*         a 0 means no interrupt will be triggered.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
* \param  mask      : see \ref cy_en_tcpwm_interrupt_sources
*
* \return None
*
*******************************************************************************/
void Cy_Tcpwm_SetInterruptMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t mask)
{
    ptscTCPWM->unINTR_MASK.u32Register = mask;
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_GetInterruptMask
****************************************************************************//**
*
* \brief  Returns the interrupt mask.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Interrupt Mask, see \ref cy_en_tcpwm_interrupt_sources
*
*******************************************************************************/
uint32_t Cy_Tcpwm_GetInterruptMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unINTR_MASK.u32Register);
}

/*******************************************************************************
* Function Name: Cy_Tcpwm_GetInterruptStatusMasked
****************************************************************************//**
*
* \brief  Returns which masked interrupt triggered the interrupt.
*
* \param  ptscTCPWM : The pointer to a counter instance.
*             Group0 - 16-bit timer(63 counters, 0-62)
*             Group1 - 16-bit timer with motor control(12 counters, 0-11)
*             Group2 - 32-bit timer(4 counters, 0-3)
*(Ex - TCPWM0_GRP0_CNT34 = Group0 is selected and its 34th counter is in use)
*
* \return Interrupt Mask, see \ref cy_en_tcpwm_interrupt_sources
*
*******************************************************************************/
uint32_t Cy_Tcpwm_GetInterruptStatusMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM)
{
    return(ptscTCPWM->unINTR_MASKED.u32Register);
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
