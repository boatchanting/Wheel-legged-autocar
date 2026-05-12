/***************************************************************************//**
* \file cy_i2s.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* The source code file for the I2S driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "audioss/cy_i2s.h"

#ifdef CY_IP_MXAUDIOSS

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_I2S_InitClock
****************************************************************************//**
*
* Initializes the clock setting of I2S module in accordance with a
* configuration structure.
*
* \param base      The pointer to the I2S instance address.
*
* \param config    The pointer to a configuration structure.
*
* \return error / status code. See \ref cy_en_i2s_status_t.
*
*******************************************************************************/
cy_en_i2s_status_t Cy_I2S_InitClock(volatile stc_I2S_t * base, cy_stc_i2s_config_clk_t const * config)
{
    if(base == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }
    if(config == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }
    if(config->clkDiv == 0u)
    {
        return CY_I2S_BAD_PARAM;
    }

    un_I2S_CLOCK_CTL_t temp_CLOCK_CTL;
    temp_CLOCK_CTL.u32Register          = base->unCLOCK_CTL.u32Register;
    temp_CLOCK_CTL.stcField.u6CLOCK_DIV = (uint32_t)config->clkDiv - 1ul;
    temp_CLOCK_CTL.stcField.u1CLOCK_SEL = (uint32_t)config->extClk;
    temp_CLOCK_CTL.stcField.u2MCLK_DIV  = (uint32_t)config->mclkDiv;
    temp_CLOCK_CTL.stcField.u1MCLK_EN   = (uint32_t)config->mclkEn;
    base->unCLOCK_CTL.u32Register       = temp_CLOCK_CTL.u32Register;

    return (CY_I2S_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_I2S_InitTx
****************************************************************************//**
*
* Initializes the TX settings of I2S module in accordance with a
* configuration structure.
*
* \param base      The pointer to the I2S instance address.
*
* \param config    The pointer to a configuration structure.
*
* \return error / status code. See \ref cy_en_i2s_status_t.
*
*******************************************************************************/
cy_en_i2s_status_t Cy_I2S_InitTx(volatile stc_I2S_t * base, cy_stc_i2s_config_tx_t const * config)
{
    if(base == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }
    if(config == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }

    cy_en_i2s_ws_pw_t wsPulseWidth;
    cy_en_i2s_len_t   channelLength;
    uint32_t          channels;
    if ((CY_I2S_TDM_MODE_A == config->txAlignment) || (CY_I2S_TDM_MODE_B == config->txAlignment))
    {
        if((config->txChannels - 1ul) == 0ul)
        {
            return CY_I2S_BAD_PARAM;
        }
        channels      = (uint32_t)config->txChannels - 1ul;
        wsPulseWidth  = config->txWsPulseWidth;
        channelLength = CY_I2S_LEN32;
    }
    else
    {
        channels      = 1ul;
        wsPulseWidth  = CY_I2S_WS_ONE_CHANNEL_LENGTH;
        channelLength = config->txChannelLength;
    }

    un_I2S_TX_CTL_t temp_TX_CTL;
    temp_TX_CTL.u32Register            = base->unTX_CTL.u32Register;
    temp_TX_CTL.stcField.u1B_CLOCK_INV = (uint32_t)config->txSdoLatchingTime;
    temp_TX_CTL.stcField.u3CH_NR       = channels;
    temp_TX_CTL.stcField.u1MS          = (uint32_t)config->txMasterMode;
    temp_TX_CTL.stcField.u2I2S_MODE    = (uint32_t)config->txAlignment;
    temp_TX_CTL.stcField.u1WS_PULSE    = (uint32_t)wsPulseWidth;
    temp_TX_CTL.stcField.u1OVHDATA     = (uint32_t)config->txOverheadValue;
    temp_TX_CTL.stcField.u1WD_EN       = (uint32_t)config->txWatchdogEnable;
    temp_TX_CTL.stcField.u3CH_LEN      = (uint32_t)channelLength;
    temp_TX_CTL.stcField.u3WORD_LEN    = (uint32_t)config->txWordLength;
    temp_TX_CTL.stcField.u1SCKO_POL    = (uint32_t)config->txSckoInversion;
    temp_TX_CTL.stcField.u1SCKI_POL    = (uint32_t)config->txSckiInversion;
    base->unTX_CTL.u32Register         = temp_TX_CTL.u32Register;

    /* The I2S watch dog setting */
    base->unTX_WATCHDOG.stcField.u32WD_COUNTER = config->txWatchdogValue;

    /* The I2S enable setting */
    base->unCTL.stcField.u1TX_ENABLED = (uint32_t)(config->txEnabled);

    /* The FIFO setting */
    base->unTX_FIFO_CTL.stcField.u8TRIGGER_LEVEL = (uint32_t)config->txFifoTriggerLevel;

    /* DMA request enabling setting */
    base->unTR_CTL.stcField.u1TX_REQ_EN          = (uint32_t)config->txDmaTrigger;

    return (CY_I2S_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_I2S_InitRx
****************************************************************************//**
*
* Initializes the RX settings of I2S module in accordance with a
* configuration structure.
*
* \param base      The pointer to the I2S instance address.
*
* \param config    The pointer to a configuration structure.
*
* \return error / status code. See \ref cy_en_i2s_status_t.
*
*******************************************************************************/
cy_en_i2s_status_t Cy_I2S_InitRx(volatile stc_I2S_t * base, cy_stc_i2s_config_rx_t const * config)
{
    if(base == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }
    if(config == NULL)
    {
        return CY_I2S_BAD_PARAM;
    }

    cy_en_i2s_ws_pw_t wsPulseWidth;
    cy_en_i2s_len_t   channelLength;
    uint32_t          channels;
    if ((CY_I2S_TDM_MODE_A == config->rxAlignment) || (CY_I2S_TDM_MODE_B == config->rxAlignment))
    {
        if((config->rxChannels - 1ul) == 0ul)
        {
            return CY_I2S_BAD_PARAM;
        }
        channels      = (uint32_t)config->rxChannels - 1ul;
        wsPulseWidth  = config->rxWsPulseWidth;
        channelLength = CY_I2S_LEN32;
    }
    else
    {
        channels      = 1ul;
        wsPulseWidth  = CY_I2S_WS_ONE_CHANNEL_LENGTH;
        channelLength = config->rxChannelLength;
    }

    un_I2S_RX_CTL_t temp_RX_CTL;
    temp_RX_CTL.u32Register              = base->unRX_CTL.u32Register;
    temp_RX_CTL.stcField.u1B_CLOCK_INV   = (uint32_t)config->rxSdiLatchingTime;
    temp_RX_CTL.stcField.u3CH_NR         = channels;
    temp_RX_CTL.stcField.u1MS            = (uint32_t)config->rxMasterMode;
    temp_RX_CTL.stcField.u2I2S_MODE      = (uint32_t)config->rxAlignment;
    temp_RX_CTL.stcField.u1WS_PULSE      = (uint32_t)wsPulseWidth;
    temp_RX_CTL.stcField.u1WD_EN         = (uint32_t)config->rxWatchdogEnable;
    temp_RX_CTL.stcField.u3CH_LEN        = (uint32_t)channelLength;
    temp_RX_CTL.stcField.u3WORD_LEN      = (uint32_t)config->rxWordLength;
    temp_RX_CTL.stcField.u1BIT_EXTENSION = (uint32_t)config->rxSignExtension;
    temp_RX_CTL.stcField.u1SCKO_POL      = (uint32_t)config->rxSckoInversion;
    temp_RX_CTL.stcField.u1SCKI_POL      = (uint32_t)config->rxSckiInversion;
    base->unRX_CTL.u32Register           = temp_RX_CTL.u32Register;

    /* The I2S watch dog setting */
    base->unRX_WATCHDOG.stcField.u32WD_COUNTER = config->rxWatchdogValue;

    /* The I2S enable setting */
    base->unCTL.stcField.u1RX_ENABLED = (uint32_t)(config->rxEnabled);

    /* The FIFO setting */
    base->unRX_FIFO_CTL.stcField.u8TRIGGER_LEVEL = (uint32_t)config->rxFifoTriggerLevel;

    /* DMA request enabling setting */
    base->unTR_CTL.stcField.u1RX_REQ_EN = (uint32_t)config->rxDmaTrigger;

    return (CY_I2S_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_I2S_DeInit
****************************************************************************//**
*
* De-initializes the I2S module (reverts default register values).
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
void Cy_I2S_DeInit(volatile stc_I2S_t * base)
{
    /* Disable interrupts prior to stopping the operation */
    base->unINTR_MASK.u32Register   = 0ul;
    base->unCMD.u32Register         = 0ul;
    base->unTR_CTL.u32Register      = 0ul;
    base->unTX_FIFO_CTL.u32Register = 0ul;
    base->unRX_FIFO_CTL.u32Register = 0ul;
    base->unCTL.u32Register         = 0ul;
    base->unTX_CTL.u32Register      = CY_I2S_TX_CTL_DEFAULT;
    base->unRX_CTL.u32Register      = CY_I2S_TX_CTL_DEFAULT;
    base->unTX_WATCHDOG.u32Register = 0ul;
    base->unRX_WATCHDOG.u32Register = 0ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_DeInitTx
****************************************************************************//**
*
* De-initializes TX settings of the I2S module (reverts default register values).
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
void Cy_I2S_DeInitTx(volatile stc_I2S_t * base)
{
    /* Disable interrupts prior to stopping the operation */
    un_I2S_INTR_MASK_t temp_INTR_MASK;
    temp_INTR_MASK.u32Register             = base->unINTR_MASK.u32Register;
    temp_INTR_MASK.stcField.u1TX_EMPTY     = 0ul;
    temp_INTR_MASK.stcField.u1TX_NOT_FULL  = 0ul;
    temp_INTR_MASK.stcField.u1TX_OVERFLOW  = 0ul;
    temp_INTR_MASK.stcField.u1TX_TRIGGER   = 0ul;
    temp_INTR_MASK.stcField.u1TX_UNDERFLOW = 0ul;
    temp_INTR_MASK.stcField.u1TX_WD        = 0ul;
    base->unINTR_MASK.u32Register = temp_INTR_MASK.u32Register;

    un_I2S_CMD_t temp_CMD;
    temp_CMD.u32Register         = base->unCMD.u32Register;
    temp_CMD.stcField.u1TX_START = 0ul;
    temp_CMD.stcField.u1TX_PAUSE = 0ul;
    base->unCMD.u32Register      = temp_CMD.u32Register;

    base->unTR_CTL.stcField.u1TX_REQ_EN    = 0ul;

    base->unTX_FIFO_CTL.u32Register        = 0ul;

    base->unCTL.stcField.u1TX_ENABLED      = 0ul;

    base->unTX_CTL.u32Register             = CY_I2S_TX_CTL_DEFAULT;

    base->unTX_WATCHDOG.u32Register        = 0ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_DeInitRx
****************************************************************************//**
*
* De-initializes RX settings of the I2S module (reverts default register values).
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
void Cy_I2S_DeInitRx(volatile stc_I2S_t * base)
{
    /* Disable interrupts prior to stopping the operation */
    un_I2S_INTR_MASK_t temp_INTR_MASK;
    temp_INTR_MASK.u32Register             = base->unINTR_MASK.u32Register;
    temp_INTR_MASK.stcField.u1RX_NOT_EMPTY = 0ul;
    temp_INTR_MASK.stcField.u1RX_FULL      = 0ul;
    temp_INTR_MASK.stcField.u1RX_OVERFLOW  = 0ul;
    temp_INTR_MASK.stcField.u1RX_TRIGGER   = 0ul;
    temp_INTR_MASK.stcField.u1RX_UNDERFLOW = 0ul;
    temp_INTR_MASK.stcField.u1RX_WD        = 0ul;
    base->unINTR_MASK.u32Register          = temp_INTR_MASK.u32Register;

    base->unCMD.stcField.u1RX_START        = 0ul;

    base->unTR_CTL.stcField.u1RX_REQ_EN    = 0ul;

    base->unRX_FIFO_CTL.u32Register        = 0ul;

    base->unCTL.stcField.u1RX_ENABLED      = 0ul;

    base->unRX_CTL.u32Register             = CY_I2S_RX_CTL_DEFAULT;

    base->unRX_WATCHDOG.u32Register        = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_DeepSleepCallback
****************************************************************************//**
*
* This is a callback function to be used at the application layer to
* manage an I2S operation during the Deep-Sleep cycle. It stores the I2S state 
* (Tx/Rx enabled/disabled/paused) into the context structure and stops the 
* communication before entering into Deep-Sleep power mode and restores the I2S
* state after waking up.
*
* \param 
* callbackParams - The pointer to the callback parameters structure, 
* see \ref cy_stc_syspm_callback_params_t.
*
* \return the SysPm callback status \ref cy_en_syspm_status_t.
*
* \note Use the \ref cy_stc_i2s_context_t data type for definition of the 
* *context element of the \ref cy_stc_syspm_callback_params_t structure.
*
*******************************************************************************/
cy_en_syspm_status_t Cy_I2S_DeepSleepCallback(cy_stc_syspm_callback_params_t * callbackParams)
{
    cy_en_syspm_status_t ret = CY_SYSPM_SUCCESS;
    CY_ASSERT_L1(NULL != callbackParams->context);
    stc_I2S_t * locBase = (stc_I2S_t*) callbackParams->base;
    uint32_t * locInterruptMask = (uint32_t*) &(((cy_stc_i2s_context_t*)(callbackParams->context))->interruptMask);
    uint32_t * locState = (uint32_t*) &(((cy_stc_i2s_context_t*)(callbackParams->context))->enableState);
    
    switch(callbackParams->mode)
    {
    case CY_SYSPM_CHECK_READY:
    case CY_SYSPM_CHECK_FAIL:
        break;

    case CY_SYSPM_BEFORE_ENTER: //rmkn CY_SYSPM_BEFORE_TRANSITION:
        *locInterruptMask = Cy_I2S_GetInterruptMask(locBase); /* Store I2S interrupts */
        *locState = Cy_I2S_GetCurrentState(locBase); /* Store I2S state */
        if(0ul != (*locState & I2S_CMD_TX_START_Msk))
        {
            Cy_I2S_DisableTx(locBase); /* Stop TX operation */
        }
        if(0ul != (*locState & I2S_CMD_RX_START_Msk))
        {
            Cy_I2S_DisableRx(locBase); /* Stop RX operation */
        }
        Cy_I2S_SetInterruptMask(locBase, 0ul); /* Disable I2S interrupts */
        /* Unload FIFOs in order not to lose data (if needed) */
        break;

    case CY_SYSPM_AFTER_EXIT: //rmkn CY_SYSPM_AFTER_TRANSITION:
        if(0ul != (*locState & I2S_CMD_RX_START_Msk))
        {
            Cy_I2S_ClearRxFifo(locBase); /* Clear the RX FIFO */
            Cy_I2S_EnableRx(locBase); /* Start RX operation */
        }
        if(0ul != (*locState & I2S_CMD_TX_START_Msk))
        {
            Cy_I2S_ClearTxFifo(locBase); /* Clear the TX FIFO */
            Cy_I2S_WriteTxData(locBase, 0ul); /* Fill at least one TX frame */
            Cy_I2S_WriteTxData(locBase, 0ul);
            if(0ul != (*locState & I2S_CMD_TX_PAUSE_Msk))
            {
                Cy_I2S_PauseTx(locBase); /* Restore the TX paused state */
            }
            Cy_I2S_EnableTx(locBase); /* Start TX operation */
        }
        Cy_I2S_ClearInterrupt(locBase, *locInterruptMask); /* Clear possible pending I2S interrupts */
        Cy_I2S_SetInterruptMask(locBase, *locInterruptMask); /* Restore I2S interrupts */
        break;

    default:
        ret = CY_SYSPM_FAIL;
        break;
    }

    return(ret);
}


#ifdef __cplusplus
}
#endif

#endif /* CY_IP_MXAUDIOSS */

/* [] END OF FILE */
