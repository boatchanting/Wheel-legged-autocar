/***************************************************************************//**
* \file cy_adc.c
* \version 1.0
*
* \brief
* Provides an API declaration of the Adc driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <math.h>
#include "adc/cy_adc.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_Adc_Init
****************************************************************************//**
*
* \brief Initializes the ADC module.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_config_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Init(volatile stc_PASS_SAR_t * base, const cy_stc_adc_config_t * config)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CTL_t unSarCtl = { 0 };
    if (NULL != config)
    {
        /* CTL register setting */
        base->unPRECOND_CTL.stcField.u4PRECOND_TIME = config->preconditionTime;

        /* CTL register setting */
        unSarCtl.stcField.u8PWRUP_TIME  = config->powerupTime;
        unSarCtl.stcField.u1IDLE_PWRDWN = config->enableIdlePowerDown ? 1ul : 0ul;
        unSarCtl.stcField.u1MSB_STRETCH = config->msbStretchMode;
        unSarCtl.stcField.u1HALF_LSB    = config->enableHalfLsbConv ? 1ul : 0ul;
        unSarCtl.stcField.u1SARMUX_EN   = config->sarMuxEnable ? 1ul : 0ul;
        unSarCtl.stcField.u1ADC_EN      = config->adcEnable ? 1ul : 0ul;
        unSarCtl.stcField.u1ENABLED     = config->sarIpEnable ? 1ul : 0ul;
        base->unCTL.u32Register         = unSarCtl.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }

    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_DeInit
****************************************************************************//**
*
* \brief  De-initializes the SAR block, returns register values to default.
*
* \param base : The pointer to a SAR block.
*
* \return None
*
*******************************************************************************/
void Cy_Adc_DeInit(volatile stc_PASS_SAR_t * base)
{
    base->unCTL.u32Register         = 0ul;
    base->unPRECOND_CTL.u32Register = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetPendingStatus
****************************************************************************//**
*
* \brief Returns trigger pending status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Pending status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0 is pending for trigger.
*
*******************************************************************************/
uint32_t Cy_Adc_GetPendingStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unTR_PEND.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetWorkValidStatus
****************************************************************************//**
*
* \brief Returns work register valid status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Work register valid status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0's work register is valid.
*
*******************************************************************************/
uint32_t Cy_Adc_GetWorkValidStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unWORK_VALID.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetWorkRangeStatus
****************************************************************************//**
*
* \brief Returns work register range status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Work register range status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0 detected range.
*
*******************************************************************************/
uint32_t Cy_Adc_GetWorkRangeStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unWORK_RANGE.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetWorkRangeHiStatus
****************************************************************************//**
*
* \brief Returns work register range hi status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Work register range hi status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0 detected out of range and the value was above the Hi threshold.
*
*******************************************************************************/
uint32_t Cy_Adc_GetWorkRangeHiStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unWORK_RANGE_HI.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetWorkPulseStatus
****************************************************************************//**
*
* \brief Returns work register pulse status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Work register pulse status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0 detected a pulse.
*
*******************************************************************************/
uint32_t Cy_Adc_GetWorkPulseStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unWORK_PULSE.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetResultValidStatus
****************************************************************************//**
*
* \brief Returns result register valid status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Result register pulse status. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0's result register is valid.
*
*******************************************************************************/
uint32_t Cy_Adc_GetResultValidStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unRESULT_VALID.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetResultRangeHiStatus
****************************************************************************//**
*
* \brief Returns result register range hi status.
*
* \param base
* The pointer to the SAR instance.
*
* \return Result register range hi. Each bit is correspond a channel, i.e.
* If bit0 is 1, ch.0 detected out of range and the value was above the Hi threshold.
*
*******************************************************************************/
uint32_t Cy_Adc_GetResultRangeHiStatus(const volatile stc_PASS_SAR_t * base)
{
    return base->unRESULT_RANGE_HI.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_Init
****************************************************************************//**
*
* \brief Initializes an ADC channel.
* This function doesn't enable ADC channel. To enable ADC channel,
* call Cy_Adc_Channel_Enable after calling this function,
*
* \param base
* The pointer to an SAR channel instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_channel_config_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_Init(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_channel_config_t * config)
{
    cy_en_adc_status_t          ret         = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_TR_CTL_t     unTrCtl     = { 0ul };
    un_PASS_SAR_CH_SAMPLE_CTL_t unSampleCtl = { 0ul };
    un_PASS_SAR_CH_POST_CTL_t   unPostCtl   = { 0ul };
    un_PASS_SAR_CH_RANGE_CTL_t  unRangeCtl  = { 0ul };
    un_PASS_SAR_CH_INTR_t       unIntr      = { 0ul };

    if (NULL != config)
    {
        /* At first disable the channel */
        Cy_Adc_Channel_Disable(base);
        /* Clear whole interrupt flags */
        unIntr.stcField.u1CH_OVERFLOW       = 1ul;
        unIntr.stcField.u1CH_PULSE          = 1ul;
        unIntr.stcField.u1CH_RANGE          = 1ul;
        unIntr.stcField.u1GRP_CANCELLED     = 1ul;
        unIntr.stcField.u1GRP_DONE          = 1ul;
        unIntr.stcField.u1GRP_OVERFLOW      = 1ul;
        base->unINTR.u32Register            = unIntr.u32Register;

        unTrCtl.stcField.u3SEL              = config->triggerSelection;
        unTrCtl.stcField.u3PRIO             = config->channelPriority;
        unTrCtl.stcField.u2PREEMPT_TYPE     = config->preenptionType;
        unTrCtl.stcField.u1GROUP_END        = config->isGroupEnd ? 1ul : 0ul;
        unTrCtl.stcField.u1DONE_LEVEL       = config->doneLevel ? 1ul : 0ul;
        base->unTR_CTL.u32Register          = unTrCtl.u32Register;

        unSampleCtl.stcField.u6PIN_ADDR     = config->pinAddress;
        unSampleCtl.stcField.u2PORT_ADDR    = config->portAddress;
        unSampleCtl.stcField.u3EXT_MUX_SEL  = config->extMuxSelect;
        unSampleCtl.stcField.u1EXT_MUX_EN   = config->extMuxEnable ? 1u : 0u;
        unSampleCtl.stcField.u2PRECOND_MODE = config->preconditionMode;
        unSampleCtl.stcField.u2OVERLAP_DIAG = config->overlapDiagMode;
        unSampleCtl.stcField.u12SAMPLE_TIME = config->sampleTime;
        unSampleCtl.stcField.u1ALT_CAL      = config->calibrationValueSelect;
        base->unSAMPLE_CTL.u32Register      = unSampleCtl.u32Register;

        unPostCtl.stcField.u3POST_PROC      = config->postProcessingMode;
        unPostCtl.stcField.u1LEFT_ALIGN     = config->resultAlignment;
        unPostCtl.stcField.u1SIGN_EXT       = config->signExtention;
        unPostCtl.stcField.u8AVG_CNT        = config->averageCount;
        unPostCtl.stcField.u5SHIFT_R        = config->rightShift;
        unPostCtl.stcField.u2RANGE_MODE     = config->rangeDetectionMode;
        base->unPOST_CTL.u32Register        = unPostCtl.u32Register;

        unRangeCtl.stcField.u16RANGE_LO     = config->rangeDetectionLoThreshold;
        unRangeCtl.stcField.u16RANGE_HI     = config->rangeDetectionHiThreshold;
        base->unRANGE_CTL.u32Register       = unRangeCtl.u32Register;

        Cy_Adc_Channel_SetInterruptMask(base, &config->mask);
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_DeInit
****************************************************************************//**
*
* \brief Stop and de-initialize ADC channel register.
*
* \param base
* The pointer to an SAR channel instance.
*
*******************************************************************************/
void Cy_Adc_Channel_DeInit(volatile stc_PASS_SAR_CH_t * base)
{
    un_PASS_SAR_CH_INTR_t unIntr = { 0 };

    base->unTR_CTL.u32Register = 0x00000800ul;
    base->unENABLE.u32Register = 0x00000000ul;
    /* Clear whole interrupt flag */
    unIntr.stcField.u1CH_OVERFLOW   = 1ul;
    unIntr.stcField.u1CH_PULSE      = 1ul;
    unIntr.stcField.u1CH_RANGE      = 1ul;
    unIntr.stcField.u1GRP_CANCELLED = 1ul;
    unIntr.stcField.u1GRP_DONE      = 1ul;
    unIntr.stcField.u1GRP_OVERFLOW  = 1ul;
    base->unINTR.u32Register        = unIntr.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_Enable
****************************************************************************//**
*
* \brief  Enable corresponding channel.
*
* \param  base: Pointer to the hardware SAR channel block.
*
* \return None
*
* \note  To enable a group either start with enabling the last channel first
* and the first channel last, or start the trigger after all channels are enabled.
*
*******************************************************************************/
void Cy_Adc_Channel_Enable(volatile stc_PASS_SAR_CH_t * base)
{
    base->unENABLE.stcField.u1CHAN_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_Disable
****************************************************************************//**
*
* \brief  Disable corresponding channel.
*
* \param  base: Pointer to the hardware SAR channel block.
*
* \return None
*
* \note To disable a group either stop the trigger first or begin with disabling
* the lowest channel first.
*
*******************************************************************************/
void Cy_Adc_Channel_Disable(volatile stc_PASS_SAR_CH_t * base)
{
    base->unENABLE.stcField.u1CHAN_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_SoftwareTrigger
****************************************************************************//**
*
* \brief  Issue software start trigger.
*
* \param  base: Pointer to the hardware SAR channel block.
*
* \return None
*
*******************************************************************************/
void Cy_Adc_Channel_SoftwareTrigger(volatile stc_PASS_SAR_CH_t * base)
{
    base->unTR_CMD.stcField.u1START = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetResult
****************************************************************************//**
*
* \brief Returns conversion result and status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param result
* The pointer to be stored conversion value.
*
* \param status
* The pointer to be stored conversion status.
* ORed of \ref cy_stc_adc_ch_status_t.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetResult(const volatile stc_PASS_SAR_CH_t * base, uint16_t * result, cy_stc_adc_ch_status_t * status)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_RESULT_t value;

    if ((NULL != result) && (NULL != status))
    {
        value.u32Register = base->unRESULT.u32Register;
        *result           = value.stcField.u16RESULT;
        status->aboveHi   = (value.stcField.u1ABOVE_HI_MIR   != 0ul) ? true : false;
        status->pulseIntr = (value.stcField.u1PULSE_INTR_MIR != 0ul) ? true : false;
        status->rangeIntr = (value.stcField.u1RANGE_INTR_MIR != 0ul) ? true : false;
        status->valid     = (value.stcField.u1VALID_MIR      != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetWorkingData
****************************************************************************//**
*
* \brief Returns working data and status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param result
* The pointer to be stored conversion value.
*
* \param status
* The pointer to be stored conversion status.
* ORed of \ref cy_stc_adc_ch_status_t.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetWorkingData(const volatile stc_PASS_SAR_CH_t * base, uint16_t * result, cy_stc_adc_ch_status_t * status)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_WORK_t value;

    if ((NULL != result) && (NULL != status))
    {
        value.u32Register = base->unWORK.u32Register;
        *result           = value.stcField.u16WORK;
        status->aboveHi   = (value.stcField.u1ABOVE_HI_MIR != 0ul) ? true : false;
        status->pulseIntr = (value.stcField.u1PULSE_MIR    != 0ul) ? true : false;
        status->rangeIntr = (value.stcField.u1RANGE_MIR    != 0ul) ? true : false;
        status->valid     = (value.stcField.u1VALID_MIR    != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetGroupStatus
****************************************************************************//**
*
* \brief Returns group conversion status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param status
* The pointer to be stored conversion status.
* ORed of \ref cy_stc_adc_group_status_t.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetGroupStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_group_status_t * status)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_GRP_STAT_t unHwStat = { 0ul };

    if (NULL != status)
    {
        unHwStat.u32Register    = base->unGRP_STAT.u32Register;
        status->chOverflow      = (unHwStat.stcField.u1CH_OVERFLOW       != 0ul) ? true : false;
        status->chPulseComplete = (unHwStat.stcField.u1CH_PULSE_COMPLETE != 0ul) ? true : false;
        status->chRangeComplete = (unHwStat.stcField.u1CH_RANGE_COMPLETE != 0ul) ? true : false;
        status->grpBusy         = (unHwStat.stcField.u1GRP_BUSY          != 0ul) ? true : false;
        status->grpCancelled    = (unHwStat.stcField.u1GRP_CANCELLED     != 0ul) ? true : false;
        status->grpComplete     = (unHwStat.stcField.u1GRP_COMPLETE      != 0ul) ? true : false;
        status->grpOverflow     = (unHwStat.stcField.u1GRP_OVERFLOW      != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_SetInterruptMask
****************************************************************************//**
*
* \brief Configure channel interrupt.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param mask
* Interrupt enable selection. \ref cy_stc_adc_interrupt_source_t.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_SetInterruptMask(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_interrupt_source_t * mask)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_INTR_MASK_t unMask = { 0ul };
    if (NULL != mask)
    {
        unMask.stcField.u1CH_OVERFLOW_MASK   = mask->chOverflow   ? 1ul : 0ul;
        unMask.stcField.u1CH_PULSE_MASK      = mask->chPulse      ? 1ul : 0ul;
        unMask.stcField.u1CH_RANGE_MASK      = mask->chRange      ? 1ul : 0ul;
        unMask.stcField.u1GRP_CANCELLED_MASK = mask->grpCancelled ? 1ul : 0ul;
        unMask.stcField.u1GRP_DONE_MASK      = mask->grpDone      ? 1ul : 0ul;
        unMask.stcField.u1GRP_OVERFLOW_MASK  = mask->grpOverflow  ? 1ul : 0ul;
        base->unINTR_MASK.u32Register        = unMask.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetInterruptMask
****************************************************************************//**
*
* \brief Returns interrupt mask configuration.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param mask
* \ref cy_stc_adc_interrupt_source_t.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptMask(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * mask)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_INTR_MASK_t unMask;
    if (NULL != mask)
    {
        unMask.u32Register = base->unINTR_MASK.u32Register;
        mask->chOverflow   = (unMask.stcField.u1CH_OVERFLOW_MASK   != 0ul) ? true : false;
        mask->chPulse      = (unMask.stcField.u1CH_PULSE_MASK      != 0ul) ? true : false;
        mask->chRange      = (unMask.stcField.u1CH_RANGE_MASK      != 0ul) ? true : false;
        mask->grpCancelled = (unMask.stcField.u1GRP_CANCELLED_MASK != 0ul) ? true : false;
        mask->grpDone      = (unMask.stcField.u1GRP_DONE_MASK      != 0ul) ? true : false;
        mask->grpOverflow  = (unMask.stcField.u1GRP_OVERFLOW_MASK  != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_ClearInterruptStatus
****************************************************************************//**
*
* \brief Clear corresponding channel interrupt status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param source
* \ref cy_stc_adc_interrupt_source_t. If a bit is true corresponding interrupt
* will be cleared.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_ClearInterruptStatus(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_interrupt_source_t * source)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_INTR_t unIntr = { 0ul };
    if (NULL != source)
    {
        unIntr.stcField.u1CH_OVERFLOW   = source->chOverflow   ? 1ul : 0ul;
        unIntr.stcField.u1CH_PULSE      = source->chPulse      ? 1ul : 0ul;
        unIntr.stcField.u1CH_RANGE      = source->chRange      ? 1ul : 0ul;
        unIntr.stcField.u1GRP_CANCELLED = source->grpCancelled ? 1ul : 0ul;
        unIntr.stcField.u1GRP_DONE      = source->grpDone      ? 1ul : 0ul;
        unIntr.stcField.u1GRP_OVERFLOW  = source->grpOverflow  ? 1ul : 0ul;
        base->unINTR.u32Register        = unIntr.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetInterruptStatus
****************************************************************************//**
*
* \brief Returns interrupt status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param status
* \ref cy_stc_adc_interrupt_source_t. If a bit is true corresponding inerrupt
* occurs. It doesnot indicate a request to the NVIC is generated.
* The requet to the NVIC can get calling Cy_Adc_Channel_GetInterruptMaskedStatus.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * status)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_INTR_t unStat;

    if (NULL != status)
    {
        unStat.u32Register   = base->unINTR.u32Register;
        status->chOverflow   = (unStat.stcField.u1CH_OVERFLOW   != 0ul) ? true : false;
        status->chPulse      = (unStat.stcField.u1CH_PULSE      != 0ul) ? true : false;
        status->chRange      = (unStat.stcField.u1CH_RANGE      != 0ul) ? true : false;
        status->grpCancelled = (unStat.stcField.u1GRP_CANCELLED != 0ul) ? true : false;
        status->grpDone      = (unStat.stcField.u1GRP_DONE      != 0ul) ? true : false;
        status->grpOverflow  = (unStat.stcField.u1GRP_OVERFLOW  != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Channel_GetInterruptMaskedStatus
****************************************************************************//**
*
* \brief Returns interrupt status.
*
* \param base
* The pointer to the SAR channel instance.
*
* \param status
* \ref cy_stc_adc_interrupt_source_t. If a bit is true corresponding inerrupt
* occurs and the request to the NVIC is generated.
*
* \return \ref cy_en_adc_status_t.
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptMaskedStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * status)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_CH_INTR_MASKED_t unStat;

    if (NULL != status)
    {
        unStat.u32Register   = base->unINTR_MASKED.u32Register;
        status->chOverflow   = (unStat.stcField.u1CH_OVERFLOW_MASKED   != 0ul) ? true : false;
        status->chPulse      = (unStat.stcField.u1CH_PULSE_MASKED      != 0ul) ? true : false;
        status->chRange      = (unStat.stcField.u1CH_RANGE_MASKED      != 0ul) ? true : false;
        status->grpCancelled = (unStat.stcField.u1GRP_CANCELLED_MASKED != 0ul) ? true : false;
        status->grpDone      = (unStat.stcField.u1GRP_DONE_MASKED      != 0ul) ? true : false;
        status->grpOverflow  = (unStat.stcField.u1GRP_OVERFLOW_MASKED  != 0ul) ? true : false;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Diag_Init
****************************************************************************//**
*
* \brief Initializes the diagnosis function.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_diag_config_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_Diag_Init(volatile stc_PASS_SAR_t * base, const cy_stc_adc_diag_config_t * config)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    if (NULL != config)
    {
        base->unDIAG_CTL.stcField.u4DIAG_SEL = config->referenceSelect;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_Diag_Enable
****************************************************************************//**
*
* \brief  Enable diag function.
*
* \param  base: Pointer to the hardware SAR block.
*
* \return None
*
*******************************************************************************/
void Cy_Adc_Diag_Enable(volatile stc_PASS_SAR_t * base)
{
    base->unDIAG_CTL.stcField.u1DIAG_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_Diag_Disable
****************************************************************************//**
*
* \brief  Disable diag function.
*
* \param  base: Pointer to the hardware SAR block.
*
* \return None
*
*******************************************************************************/
void Cy_Adc_Diag_Disable(volatile stc_PASS_SAR_t * base)
{
    base->unDIAG_CTL.stcField.u1DIAG_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_TriggerCalibrationUpdate
****************************************************************************//**
*
* \brief  Trigger calibration update.
*         After triggering, coherently copy values from alternate calibration
*         regs to current calibration regs.
* \param  base: Pointer to the hardware SAR block.
*
* \return None
*
* \note Alternate calibration value has to be set before calling this function.
*       After triggering, calibration update will be done as soon as the ADC is
*       idle or a triggered group completes.
*       Whether calibration update is done or not can be called
*       Cy_Adc_IsCalibrationUpdateDone function.
*******************************************************************************/
void Cy_Adc_TriggerCalibrationUpdate(volatile stc_PASS_SAR_t * base)
{
    base->unCAL_UPD_CMD.stcField.u1UPDATE = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Adc_IsCalibrationUpdateDone
****************************************************************************//**
*
* \brief  Get the status of calibration update.
*
* \param  base: Pointer to the hardware SAR block.
*
* \return If true, calibration update is done.
*         If false, calibration update is not yet done.
*
*******************************************************************************/
bool Cy_Adc_IsCalibrationUpdateDone(const volatile stc_PASS_SAR_t * base)
{
    bool ret;
    if (0ul != base->unCAL_UPD_CMD.stcField.u1UPDATE)
    {
        ret = false;
    }
    else
    {
        ret = true;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetDigitalCalibrationValue
****************************************************************************//**
*
* \brief Configure ADC digital calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_digital_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetDigitalCalibrationValue(volatile stc_PASS_SAR_t * base, const cy_stc_adc_digital_calibration_conifg_t * config)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    un_PASS_SAR_DIG_CAL_t unDigCal = { 0ul };

    if (NULL != config)
    {
        unDigCal.stcField.u12DOFFSET = config->offset;
        unDigCal.stcField.u6DGAIN    = config->gain;
        base->unDIG_CAL.u32Register  = unDigCal.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetDigitalCalibrationValue
****************************************************************************//**
*
* \brief Get ADC digital calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The utput pointer to the configuration structure
* \ref cy_stc_adc_digital_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_GetDigitalCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_digital_calibration_conifg_t * config)
{
    cy_en_adc_status_t    ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_DIG_CAL_t unDigCal;
    unDigCal.u32Register = base->unDIG_CAL.u32Register;

    if (NULL != config)
    {
        config->offset  = unDigCal.stcField.u12DOFFSET;
        config->gain    = unDigCal.stcField.u6DGAIN;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetAltDigitalCalibrationValue
****************************************************************************//**
*
* \brief Configure alternate ADC digital calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_digital_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetAltDigitalCalibrationValue(volatile stc_PASS_SAR_t * base, const cy_stc_adc_digital_calibration_conifg_t * config)
{
    cy_en_adc_status_t        ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_DIG_CAL_ALT_t unDigCal = { 0ul };

    if (NULL != config)
    {
        unDigCal.stcField.u12DOFFSET    = config->offset;
        unDigCal.stcField.u6DGAIN       = config->gain;
        base->unDIG_CAL_ALT.u32Register = unDigCal.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetAltDigitalCalibrationValue
****************************************************************************//**
*
* \brief Get alternate ADC digital calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The utput pointer to the configuration structure
* \ref cy_stc_adc_digital_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_GetAltDigitalCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_digital_calibration_conifg_t * config)
{
    cy_en_adc_status_t        ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_DIG_CAL_ALT_t unDigCal;
    unDigCal.u32Register = base->unDIG_CAL_ALT.u32Register;


    if (NULL != config)
    {
        config->offset = unDigCal.stcField.u12DOFFSET;
        config->gain   = unDigCal.stcField.u6DGAIN;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetAnalogCalibrationValue
****************************************************************************//**
*
* \brief Configure ADC analog calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_analog_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetAnalogCalibrationValue(volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config)
{
    cy_en_adc_status_t    ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_ANA_CAL_t unAnaCal = { 0ul };

    if (NULL != config)
    {
        unAnaCal.stcField.u8AOFFSET = config->offset;
        unAnaCal.stcField.u5AGAIN   = config->gain;
        base->unANA_CAL.u32Register = unAnaCal.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetAnalogCalibrationValue
****************************************************************************//**
*
* \brief Get ADC analog calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The utput pointer to the configuration structure
* \ref cy_stc_adc_analog_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_GetAnalogCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config)
{
    cy_en_adc_status_t    ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_ANA_CAL_t unAnaCal;
    unAnaCal.u32Register = base->unANA_CAL.u32Register;

    if (NULL != config)
    {
        config->offset = (int8_t)unAnaCal.stcField.u8AOFFSET;
        config->gain   = (int8_t)unAnaCal.stcField.u5AGAIN;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetAltAnalogCalibrationValue
****************************************************************************//**
*
* \brief Configure alternate ADC analog calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_adc_analog_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetAltAnalogCalibrationValue(volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config)
{
    cy_en_adc_status_t        ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_ANA_CAL_ALT_t unAnaCal = { 0ul };

    if (NULL != config)
    {
        unAnaCal.stcField.u8AOFFSET     = config->offset;
        unAnaCal.stcField.u5AGAIN       = config->gain;
        base->unANA_CAL_ALT.u32Register = unAnaCal.u32Register;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetAltAnalogCalibrationValue
****************************************************************************//**
*
* \brief Get alternate ADC analog calibration value.
*
* \param base
* The pointer to the SAR instance.
*
* \param config
* The utput pointer to the configuration structure
* \ref cy_stc_adc_analog_calibration_conifg_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_GetAltAnalogCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config)
{
    cy_en_adc_status_t        ret      = CY_ADC_SUCCESS;
    un_PASS_SAR_ANA_CAL_ALT_t unAnaCal;
    unAnaCal.u32Register = base->unANA_CAL_ALT.u32Register;

    if (NULL != config)
    {
        config->offset  = unAnaCal.stcField.u8AOFFSET;
        config->gain    = unAnaCal.stcField.u5AGAIN;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetDebugFreezeMode
****************************************************************************//**
*
* \brief Configure debug pause feature.
*
* \param base
* The pointer to the PASS instance.
*
* \param config
* The utput pointer to the configuration structure
* \ref cy_stc_adc_debug_freeze_config_t.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetDebugFreezeMode(volatile stc_PASS_EPASS_MMIO_t * base, const cy_stc_adc_debug_freeze_config_t * config)
{
    cy_en_adc_status_t ret  = CY_ADC_SUCCESS;
    uint32_t           temp = 0ul;
    if (NULL != config)
    {
        temp |= (config->enableFreezeAdc0) ? 1ul : 0ul;
        temp |= (config->enableFreezeAdc1) ? 2ul : 0ul;
        temp |= (config->enableFreezeAdc2) ? 4ul : 0ul;
        temp |= (config->enableFreezeAdc3) ? 8ul : 0ul;
        base->unPASS_CTL.stcField.u4DBG_FREEZE_EN = temp;
    }
    else
    {
        ret = CY_ADC_BAD_PARAM;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetGenericTriggerInput
****************************************************************************//**
*
* \brief Configure generic trigger input selection.
*
* \param base
* The pointer to the PASS instance.
*
* \param numOfAdc
* ADC channel number. Valid range is [0..3]
*
* \param triggerInputNumber
* Generic input trigger number to be setup. Valid range is [0..4]
*
* \param genericTriggerValue
* Generic input trigger to be used as a corresponding trigger input #.
* Valid range is [0..15].
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetGenericTriggerInput(volatile stc_PASS_EPASS_MMIO_t * base, uint8_t numOfAdc, uint8_t triggerInputNumber, uint8_t genericTriggerValue)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    if ((numOfAdc > 4u) ||
        (triggerInputNumber >= 5u) ||
        (genericTriggerValue >= 16u))
    {
        ret = CY_ADC_BAD_PARAM;
    }
    else
    {
        switch(triggerInputNumber)
        {
        case 0u:
            base->unSAR_TR_IN_SEL[numOfAdc].stcField.u4IN0_SEL = genericTriggerValue;
            break;
        case 1u:
            base->unSAR_TR_IN_SEL[numOfAdc].stcField.u4IN1_SEL = genericTriggerValue;
            break;
        case 2u:
            base->unSAR_TR_IN_SEL[numOfAdc].stcField.u4IN2_SEL = genericTriggerValue;
            break;
        case 3u:
            base->unSAR_TR_IN_SEL[numOfAdc].stcField.u4IN3_SEL = genericTriggerValue;
            break;
        case 4u:
            base->unSAR_TR_IN_SEL[numOfAdc].stcField.u4IN4_SEL = genericTriggerValue;
            break;
        default:
            break;
        }
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetGenericOutpuTrigger
****************************************************************************//**
*
* \brief Configure generic output trigger selection.
*
* \param base
* The pointer to the PASS instance.
*
* \param numOfAdc
* ADC channel number. Valid range is [0..3]
*
* \param outputTriggerNumber
* Generic input trigger number to be setup. Valid range is [0..1]
*
* \param genericTriggerValue
* Generic input trigger to be used as a corresponding trigger input #.
* Valid range is [0..63]. [0..31] means select a tr_sar_ch_done trigger.
* [32..63] means select a tr_sar_ch_rangvio trigger.
*
* \return
* \ref cy_en_adc_status_t
*
*******************************************************************************/
cy_en_adc_status_t Cy_Adc_SetGenericOutpuTrigger(volatile stc_PASS_EPASS_MMIO_t * base, uint8_t numOfAdc, uint8_t outputTriggerNumber, uint8_t genericTriggerValue)
{
    cy_en_adc_status_t ret = CY_ADC_SUCCESS;
    if ((numOfAdc > 4u) ||
        (outputTriggerNumber >= 2u) ||
        (genericTriggerValue >= 64u))
    {
        ret = CY_ADC_BAD_PARAM;
    }
    else
    {
        switch(outputTriggerNumber)
        {
        case 0u:
            base->unSAR_TR_OUT_SEL[numOfAdc].stcField.u6OUT0_SEL = genericTriggerValue;
            break;
        case 1u:
            base->unSAR_TR_OUT_SEL[numOfAdc].stcField.u6OUT1_SEL = genericTriggerValue;
            break;
        default:
            break;
        }
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_Adc_SetReferenceBufferMode
****************************************************************************//**
*
* \brief Set ePASS MMIO reference buffer mode.
*
* \param base
* The pointer to the PASS instance.
*
* \param mode
* Reference buffer mode number. Valid range is [0,1,3]
*
* \return
* None
*
*******************************************************************************/
void Cy_Adc_SetReferenceBufferMode(volatile stc_PASS_EPASS_MMIO_t * base, cy_en_adc_ref_buf_mode_t mode)
{
    base->unPASS_CTL.stcField.u2REFBUF_MODE = mode;
}

/*******************************************************************************
* Function Name: Cy_Adc_GetReferenceBufferMode
****************************************************************************//**
*
* \brief Get ePASS MMIO reference buffer mode.
*
* \param base
* The pointer to the PASS instance.
*
* \return
* \ref cy_en_adc_ref_buf_mode_t
*
*******************************************************************************/
cy_en_adc_ref_buf_mode_t Cy_Adc_GetReferenceBufferMode(volatile stc_PASS_EPASS_MMIO_t * base)
{
    return (cy_en_adc_ref_buf_mode_t)base->unPASS_CTL.stcField.u2REFBUF_MODE;
}

/*******************************************************************************
* Function Name: Cy_Adc_CalculateDieTemperature
****************************************************************************//**
*
* \brief Calculate the internal temperature from ADC core using Vtemp.
* The api is implemented to calculate the temperature from the 2nd order 
* equation of sort data with corrected class data. Api will convert sort data
* into [3][3] matrix and class data into [2][2] matrix.
*
* \param refValue
* The pointer to the refValues for the ADC temperature sensor read from sFLASH.
*
* \param numOfAdc
* The pointer to the rawValue of Vbg and Vtemp after adc conversion.
*
* \return
* \ref temperature Value
*
*******************************************************************************/
double Cy_Adc_CalculateDieTemperature(cy_stc_adc_temp_ref_t *refValue, cy_stc_adc_temp_raw_t *rawValue)
{
    double determinant = 0.0;
    double coefficientMatrixForSort[3][3] = {{0.0},{0.0},{0.0}};
    
    double sortTempMatrixA[3][3];
    double sortTempValueMatrixB[3][1];
    double sortTempCoefficeintsX[3] = {0.0};
    
    double coeffA, coeffB, coeffC;
    double bSqrMin4ac = 0.0;
    double tempRefValue = 0.0;
    double tempScaleValue = 0.0;
    double correctionFactor = 0.0;
    double tempRootPos, tempRootNeg = 0.0;
    double tempRangeOffset = 10; /* relate to +/- 10 degC */
    double tempInDegreeC = 0.0;

    bool isApproxMeasDone = false;
   
    /*************************************************************************** 
     * Update the class and sort matrix from the sFLASH values
     **************************************************************************/
    
    /* Update the sort temperature value matrix A */
    sortTempMatrixA[0][0] = 1.0;
    sortTempMatrixA[0][1] = refValue->adcTempValues.coldValue;
    sortTempMatrixA[0][2] = pow(refValue->adcTempValues.coldValue, 2.0);
    sortTempMatrixA[1][0] = 1.0;
    sortTempMatrixA[1][1] = refValue->adcTempValues.roomValue;
    sortTempMatrixA[1][2] = pow(refValue->adcTempValues.roomValue, 2.0);
    sortTempMatrixA[2][0] = 1.0;
    sortTempMatrixA[2][1] = refValue->adcTempValues.hotValue;
    sortTempMatrixA[2][2] = pow(refValue->adcTempValues.hotValue, 2.0);
    
    /* Update the sort temperature adc value matrix B */
    sortTempValueMatrixB[0][0] = refValue->adcDiodeValues.coldValue;
    sortTempValueMatrixB[1][0] = refValue->adcDiodeValues.roomValue;
    sortTempValueMatrixB[2][0] = refValue->adcDiodeValues.hotValue;
    
    /*************************************************************************** 
     * Get the 2nd order coefficient for sort value matrix
     **************************************************************************/
    
    /* Get the determinant of sort temperature matrix A */
    for(uint8_t i = 0u; i < 3u; i++)
    {
        determinant = determinant + (sortTempMatrixA[0][i]*(sortTempMatrixA[1][(i+1)%3]*sortTempMatrixA[2][(i+2)%3] - sortTempMatrixA[1][(i+2)%3]*sortTempMatrixA[2][(i+1)%3]));
    }
  
    /* Get the inverse of sort temperature matrix A */
    for(uint8_t i = 0u; i < 3u; i++)
    {
        for(uint8_t j = 0u; j < 3u; j++)
        {
            coefficientMatrixForSort[i][j] = ((sortTempMatrixA[(i+1)%3][(j+1)%3] * sortTempMatrixA[(i+2)%3][(j+2)%3]) - (sortTempMatrixA[(i+1)%3][(j+2)%3]*sortTempMatrixA[(i+2)%3][(j+1)%3]))/ determinant;
        }
    }
    for(uint8_t i = 0u; i < 3u; i++)
    {
        for(uint8_t j = 0u; j < 3u; j++)
        {
            sortTempMatrixA[i][j] = coefficientMatrixForSort[j][i];
        }
    }
    
    /* Calculate sort temperature coefficient matrix X = (invA)*B */
    for(uint8_t i = 0u; i < 3u; i++)
    {
        for(uint8_t j = 0u; j < 3u; j++)
        {
            sortTempCoefficeintsX[i] += (sortTempValueMatrixB[j][0]*sortTempMatrixA[i][j]);
        }
    }
    
    /*************************************************************************** 
     * Get the temperature value from the 2nd order equation
     **************************************************************************/
    
    /* Rearrange the coefficients for the predicted temperature formula */
    coeffA = sortTempCoefficeintsX[2];
    coeffB = sortTempCoefficeintsX[1];
    coeffC = sortTempCoefficeintsX[0];

    /* -40degC -- SORT2, 27degC -- SORT3, 130degC -- CHI and by default reference value is SORT3 */
    tempRefValue = refValue->adcVbgValues.roomValue;

    /* Conditional label for recalculating roots */
    RECALCULATE:

        /* Calculate the correction factor (k) to remove dependency of the ADC on the reference voltage */
        correctionFactor = (tempRefValue) / (rawValue->adcVbgRawValue);
        
        /* Scale the data in raw with k */
        tempScaleValue = (correctionFactor) * (rawValue->adcVtempRawValue);
        
        /* Calculate the predicted temperature */
        bSqrMin4ac = ( pow(coeffB, 2.0)) - (((4.0)*(coeffA))*(coeffC - tempScaleValue));
        tempRootNeg = ((-coeffB)-(sqrt(bSqrMin4ac))) / ((2.0)*(coeffA));
        tempRootPos = ((-coeffB)+(sqrt(bSqrMin4ac))) / ((2.0)*(coeffA));     // this can be minimize, kept for comparition
        
        /* Select the root that lies between the Hot and Cold temperature sort values [-40 degC, 150 degC] */
        if((tempRootPos < (refValue->adcTempValues.hotValue)) && (tempRootPos > (refValue->adcTempValues.coldValue)))
        {
            /* Temperature value is positive root of the curve */
            tempInDegreeC = tempRootPos;
        }
        else if((tempRootNeg < (refValue->adcTempValues.hotValue)) && (tempRootNeg > (refValue->adcTempValues.coldValue)))
        {
            /* Temperature value is negetive root of the curve */
            tempInDegreeC = tempRootNeg;
        }
        else
        {
            /* Apt value is not found */
            tempInDegreeC = 0.0;
        }

        /* Check for the close proximity of calculated temperature with the reference temeprature values */
        if (tempInDegreeC <= ((refValue->adcTempValues.coldValue) + tempRangeOffset))
        {
            /* Use SORT2 value to scale the measured temperature */
            tempRefValue = refValue->adcVbgValues.coldValue;

            /* check approximation, if done end the measurement loop */
            if (isApproxMeasDone == false)
            {
                isApproxMeasDone = true;   
                goto RECALCULATE;
            }
        }
        else if (tempInDegreeC >= ((refValue->adcTempValues.hotValue) - tempRangeOffset))
        {
            /* Use CHI value to scale the measured temperature */
            tempRefValue = refValue->adcVbgValues.hotValue;

            /* check approximation, if done end the measurement loop */
            if (isApproxMeasDone == false)
            {
                isApproxMeasDone = true;   
                goto RECALCULATE;
            }
        }
        else if ((tempInDegreeC <= ((refValue->adcTempValues.roomValue) + tempRangeOffset))
            && (tempInDegreeC >= ((refValue->adcTempValues.roomValue) - tempRangeOffset)))
        {
            /* Use SORT3 value to scale the measured temperature */
            tempRefValue = refValue->adcVbgValues.roomValue;
        }
        else
        {
            /* Fail safe case : Use SORT2 value to scale the measured temperature */
            /* Commented this block, leading to an infinite loop for one of the corner case */           
            //tempRefValue = refValue->adcVbgValues.coldValue;
            //if (isApproxMeasDone == false)
            //{
            //    isApproxMeasDone = true;   
            //    goto RECALCULATE;
            //}
        }

    return tempInDegreeC;
}

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
