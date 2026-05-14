/***************************************************************************/ /**
 * \file cy_cxpi.c
 * \version 1.0
 *
 * \brief
 * Provides an API declaration of the CXPI driver
 *
 ********************************************************************************
 * \copyright
 * Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
 * You may use this file only in accordance with the license, terms, conditions,
 * disclaimers, and limitations in the end user license agreement accompanying
 * the software package with which this file was provided.
 *******************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include "cy_cxpi.h"

/*****************************************************************************/
/* Local pre-processor symbols/macros ('#define')                            */
/*****************************************************************************/

/*****************************************************************************/
/* Global variable definitions (declared in header file with 'extern')       */
/*****************************************************************************/

/*****************************************************************************/
/* Local type definitions ('typedef')                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Local variable definitions ('static')                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Local function prototypes ('static')                                      */
/*****************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

/**
 *****************************************************************************
 ** \brief DeInitialisation of a CXPI channel.
 ** This Function deinitialises the selected CXPI channel.
 **
 ** \param [in]  pstcCXPI       Pointer to CXPI instance register area
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_DeInit(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;

    /* Check if pointers are valid */
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }

    /** Disable the CXPI channel
     *  If a channel is disabled, CMD, STATUS, INTR MMIO registers
     *  will have their fields reset to their default value.
     **/
    pstcCXPI->unCTL0.stcField.u1ENABLED = 0u;

    /** Set CTL0 register to default value
     */
    un_CXPI_CH_CTL0_t unDeCTL0;
    unDeCTL0.u32Register = 0uL;
    /* Assign the default value for CXPI Transceiver Auto Enable */
    unDeCTL0.stcField.u1AUTO_EN = 1u;
    pstcCXPI->unCTL0.u32Register = unDeCTL0.u32Register;

    /** Set CTL1 register to default value
     */
    un_CXPI_CH_CTL1_t unDeCTL1;
    unDeCTL1.u32Register = 0uL;
    pstcCXPI->unCTL1.u32Register = unDeCTL1.u32Register;

    /** Set CTL2 register to default value
     */
    un_CXPI_CH_CTL2_t unDeCTL2;
    unDeCTL2.u32Register = 0uL;
    pstcCXPI->unCTL2.u32Register = unDeCTL2.u32Register;

    /** Clear the TXPID_FI field
      */
    pstcCXPI->unTXPID_FI.u32Register = 0uL;

    /** Clear the TXPID_FI field
      */
    pstcCXPI->unRXPID_FI.u32Register = 0uL;

    /** Clear the TX_FIFO *
      */
    pstcCXPI->unTX_FIFO_CTL.stcField.u1CLEAR = 1u;

    /** Clear the RX_FIFO
      */
    pstcCXPI->unRX_FIFO_CTL.stcField.u1CLEAR = 1u;

    /** Clear the INTR_MASK field
      */
    pstcCXPI->unINTR_MASK.u32Register = 0uL;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Initialisation of a CXPI channel.
 ** This Function initialises the CXPI according the configuration in the
 ** passed Config Struct. Several Checkings are done before that and an error
 ** is returned if invalid Modes are requested.
 **
 ** \pre The Application must configure corresponding CXPI pins
 **      according to requirements and settings of CXPI instance.
 **      And must set baudrate using SysClk. CXPI I/F has internal fixed
 **      oversampling value (400). Therefore CXPI clock / 400 is actual
 **      baudrate.
 **
 ** \param cxpiCtx          [in]
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 ** \param [in]  pstcConfig     CXPI channel configuration. See #stc_cxpi_config_t.
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t
Cy_CXPI_Init(volatile stc_CXPI_CH_t* pstcCXPI, const stc_cxpi_config_t* pstcConfig, stc_CXPI_context_t* pCxpiCtx)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if((NULL == pstcCXPI) || (NULL == pstcConfig)) {
        ret = CY_CXPI_BAD_PARAM;
    } else if((pstcConfig->stcCxpiChCtl1Config.u16TLow1 > CXPI_T_LOW1_MAX) ||
        (pstcConfig->stcCxpiChCtl1Config.u16TLow0 > CXPI_T_LOW0_MAX) ||
        (pstcConfig->stcCxpiChCtl1Config.u16TOffset > CXPI_T_OFFSET_MAX) ||
        (pstcConfig->stcCxpiChCtl2Config.enRetriesNumber > CXPI_RETRY_MAX) ||
        (pstcConfig->stcCxpiChCtl2Config.u8TimeoutLength > CXPI_TIMEOUT_MAX)) {
        ret = CY_CXPI_BAD_PARAM;
    } else {
        pstcCXPI->unCTL0.stcField.u1ENABLED = 0u;

        un_CXPI_CH_CTL0_t unInitCtl0;
        un_CXPI_CH_CTL1_t unInitCtl1;
        un_CXPI_CH_CTL2_t unInitCtl2;
        unInitCtl0.u32Register = pstcCXPI->unCTL0.u32Register;
        unInitCtl1.u32Register = pstcCXPI->unCTL1.u32Register;
        unInitCtl2.u32Register = pstcCXPI->unCTL2.u32Register;
        

        unInitCtl0.stcField.u1MODE = pstcConfig->stcCxpiChCtl0Config.enMode;
        unInitCtl0.stcField.u1AUTO_EN = pstcConfig->stcCxpiChCtl0Config.bCxpiTransceiverAutoEnable;
        unInitCtl0.stcField.u1RXPIDZERO_CHECK_EN = pstcConfig->stcCxpiChCtl0Config.bRxPIDZeroAutoCheckEnable;
        unInitCtl0.stcField.u1FILTER_EN = pstcConfig->stcCxpiChCtl0Config.bFilterEnable;
        unInitCtl0.stcField.u5IFS = pstcConfig->stcCxpiChCtl0Config.u8IFS;
        unInitCtl0.stcField.u4IBS = pstcConfig->stcCxpiChCtl0Config.u8IBS;
        unInitCtl0.stcField.u1BIT_ERROR_IGNORE = pstcConfig->stcCxpiChCtl0Config.enBitErrorIgnore;
        unInitCtl0.stcField.u1MASTER = pstcConfig->stcCxpiChCtl0Config.enCxpiMasterMode;

        unInitCtl1.stcField.u9T_LOW1 = pstcConfig->stcCxpiChCtl1Config.u16TLow1;
        unInitCtl1.stcField.u9T_LOW0 = pstcConfig->stcCxpiChCtl1Config.u16TLow0;
        unInitCtl1.stcField.u9T_OFFSET = pstcConfig->stcCxpiChCtl1Config.u16TOffset;

        unInitCtl2.stcField.u2RETRY = 0; //retransmission is fully controlled by software
        unInitCtl2.stcField.u6T_WAKEUP_LENGTH = pstcConfig->stcCxpiChCtl2Config.u8TWakeUpLength;
        unInitCtl2.stcField.u4TIMEOUT_LENGTH = pstcConfig->stcCxpiChCtl2Config.u8TimeoutLength;
        unInitCtl2.stcField.u2TIMEOUT_SEL = pstcConfig->stcCxpiChCtl2Config.enTimeoutSel;
        
        pstcCXPI->unCTL0.u32Register = unInitCtl0.u32Register;
        pstcCXPI->unCTL1.u32Register = unInitCtl1.u32Register;
        pstcCXPI->unCTL2.u32Register = unInitCtl2.u32Register;
        
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        pCxpiCtx->arbState = CXPI_ARB_PENDING;
        pCxpiCtx->retriesCount = 0;
        
        pCxpiCtx->pAcceptedPIDList = pstcConfig->pAcceptedPIDList;
        pCxpiCtx->numberOfAcceptedPID = pstcConfig->numberOfAcceptedPID;
        pCxpiCtx->maximumRetries = pstcConfig->maximumRetries;
    
        pCxpiCtx->pNotifyTrasmittedWakeupPulse = pstcConfig->pNotifyTrasmittedWakeupPulse;
        pCxpiCtx->pNotifyRXFallingEdgeDetected = pstcConfig->pNotifyRXFallingEdgeDetected;
        pCxpiCtx->pNotifyError = pstcConfig->pNotifyError;
        pCxpiCtx->pNotifySentPTYPE = pstcConfig->pNotifySentPTYPE;
        pCxpiCtx->pNotifyReceivedPTYPE = pstcConfig->pNotifyReceivedPTYPE;
        pCxpiCtx->pNotifyReceivedPID = pstcConfig->pNotifyReceivedPID;
        pCxpiCtx->pNotifyReceivedResponse = pstcConfig->pNotifyReceivedResponse;
        pCxpiCtx->pNotifyArbitrationLost = pstcConfig->pNotifyArbitrationLost;

        Cy_CXPI_SetInterruptMask(pstcCXPI, 0x0u);
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup CXPI operation command
 **
 ** \note As a normal usage, following comobinations are used.
 **         Transmit hearder only                   : CXPI_CMD_TX_HEADER
 **         Receive hearder only                    : CXPI_CMD_RX_HEADER
 **         Transmit hearder and transmit response  : CXPI_CMD_TX_HEADER_TX_RESPONSE
 **         Transmit hearder and receive response   : CXPI_CMD_TX_HEADER_RX_RESPONSE
 **         Transmit response                       : CXPI_CMD_TX_RESPONSE
 **         Receive response                        : Already set when header RX
 **         As a typical usage, RX response is always set at header RX phase.
 **       - Wakeup frame                            : CXPI_CMD_TX_WAKEUP
 **       Instead of above macro, you can use raw definition LIN_CH_CMD_*_Msk defined in IO header file.
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 ** \param  command  [in]  Required operation command. Refer \ref group_lin_cmd_macro
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetCmd(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t command)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unCMD.u32Register = command;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup CXPI channel status
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  status    [out]  CXPI channel status.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetStatus(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* status)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *status = pstcCXPI->unSTATUS.u32Register;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Set header protected identifier (PID) for tx header operation.
 **        This API can be used by SW to send PType byte as the HW handles both PID and PType the same way
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  id        [out]  Frame ID. Software does not need to program the parity bit.
 **                          HW will compute the parity bit based on passed frame ID.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetPID(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t id)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTXPID_FI.stcField.u8PID = id;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Get header protected identifier (PID) for rx header operation.
 **        This API can be used by SW to check PType byte as the HW handles both PID and PType the same way
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  id        [out]  Received frame ID.
 ** \param  parity    [out]  parity bit of the received frame ID
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetPID(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t* id, uint8_t* parity)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    uint8_t temp = pstcCXPI->unRXPID_FI.stcField.u8PID;
    *id = temp & 0x7F;
    *parity = temp >> 7;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Set Frame Information
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  fi        [in]   Frame Information.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetFI(volatile stc_CXPI_CH_t* pstcCXPI, stc_cxpi_frame_info_t fi)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if((NULL == pstcCXPI) || (fi.counter > CXPI_COUNTER_MAX)) {
        return CY_CXPI_BAD_PARAM;
    }
    uint8_t temp = 0u;
    if(fi.datalength <= 12u) {
        temp = temp | ((fi.datalength & 0x0fu) << 4);
        temp = temp | ((fi.WakeupInd & 0x01u) << 3);
        temp = temp | ((fi.SleepInd & 0x01u) << 2);
        temp = temp | (fi.counter & 0x03u);
        pstcCXPI->unTXPID_FI.stcField.u8FI = temp;
        // pstcCXPI->unTXPID_FI.stcField.u8DLCEXT = 0u;
    } else {
        temp = temp | ((0x0fu) << 4);
        temp = temp | ((fi.WakeupInd & 0x01u) << 3);
        temp = temp | ((fi.SleepInd & 0x01u) << 2);
        temp = temp | (fi.counter & 0x03u);
        pstcCXPI->unTXPID_FI.stcField.u8FI = temp;
        pstcCXPI->unTXPID_FI.stcField.u8DLCEXT = fi.datalength;
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Get Frame Information
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  fi        [out]  Frame Information.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetFI(volatile stc_CXPI_CH_t* pstcCXPI, stc_cxpi_frame_info_t* fi)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    uint8_t temp = pstcCXPI->unRXPID_FI.stcField.u8FI;
    if((temp >> 4) == 0xfu) {
        fi->datalength = pstcCXPI->unRXPID_FI.stcField.u8DLCEXT;
        fi->WakeupInd = (temp << 4u) >> 7u;
        fi->SleepInd = (temp << 5u) >> 7u;
        fi->counter = temp & 0x03u;
    } else {
        fi->datalength = temp >> 4u;
        fi->WakeupInd = (temp << 4u) >> 7u;
        fi->SleepInd = (temp << 5u) >> 7u;
        fi->counter = temp & 0x03u;
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Get CRC for RX and TX
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  CRCdata   [out]  Return CRC bytes for RX and TX.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetCRC(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* CRCdata)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *CRCdata = pstcCXPI->unCRC.u32Register;
    return ret;
}

/*******************************************************************************
* Function Name: Cy_CXPI_SetTriggerLevelTxFIFO
****************************************************************************/ /**
     * \brief  Set trigger level for TX_FIFO. When the TX FIFO has less entries
     * than the number of this field, a transmitter trigger event is generated:
     * -INTR.TX_FIFO_TRIGGER = (number of entries in TX_FIFO < TRIGGER_LEVEL)
     *
     * \param  pstcCXPI        [in]
     * Pointer to CXPI instance register area.
     *
     * \param  triggerLevel    [in]
     * Trigger level of TX_FIFO
     *
     * \retval Refer \ref cy_en_lin_status_t
     *
     ********************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetTriggerLevelTxFIFO(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t triggerLevel)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if((NULL == pstcCXPI) || (triggerLevel > CXPI_TX_FIFO_ENTRIESNUM_MAX)) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTX_FIFO_CTL.stcField.u5TRIGGER_LEVEL = triggerLevel;
    return ret;
}

/*******************************************************************************
* Function Name: Cy_CXPI_FreezeTxFIFO
****************************************************************************/ /**
     * \brief  Clear all data out of the TX_FIFO
     *
     * \param  pstcCXPI        [in]
     * Pointer to CXPI instance register area.
     *
     * \retval Refer \ref cy_en_lin_status_t
     *
     ********************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_ClearTxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTX_FIFO_CTL.stcField.u1CLEAR = 1u;
    while(pstcCXPI->unTX_FIFO_STATUS.stcField.u5USED != 0u) {
    };
    pstcCXPI->unTX_FIFO_CTL.stcField.u1CLEAR = 0u;
    return ret;
}

/*******************************************************************************
* Function Name: Cy_CXPI_FreezeTxFIFO
****************************************************************************/ /**
     * \brief  Freeze TX_FIFO. HW read from TX FIFO does not pop the data from the TX FIFO.
     * Note: Freeze functionality is for debug purpose only.
     *
     * \param  pstcCXPI        [in]
     * Pointer to CXPI instance register area.
     *
     * \retval Refer \ref cy_en_lin_status_t
     *
     ********************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_FreezeTxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTX_FIFO_CTL.stcField.u1FREEZE = 1u;
    return ret;
}

/*******************************************************************************
* Function Name: Cy_CXPI_UnfreezeTxFIFO
****************************************************************************/ /**
     * \brief Unfreeze TX_FIFO. HW uses TX FIFO data and pops the data from the
     * TX FIFO for every HW read.
     *
     * \param  pstcCXPI        [in]
     * Pointer to CXPI instance register area.
     *
     * \retval Refer \ref cy_en_lin_status_t
     *
     ********************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_UnfreezeTxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTX_FIFO_CTL.stcField.u1FREEZE = 0u;
    return ret;
}

cy_en_cxpi_status_t
Cy_CXPI_GetTxFIFOStatus(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* usedEnntries, uint32_t* availEntries)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    uint32_t temp;
    temp = pstcCXPI->unTX_FIFO_STATUS.u32Register;
    *usedEnntries = (temp & CXPI_CH_TX_FIFO_STATUS_USED_Msk) >> CXPI_CH_TX_FIFO_STATUS_USED_Pos;
    *availEntries = (temp & CXPI_CH_TX_FIFO_STATUS_AVAIL_Msk) >> CXPI_CH_TX_FIFO_STATUS_AVAIL_Pos;
    return ret;
}

/*******************************************************************************
* Function Name: Cy_CXPI_WriteTxFIFO
****************************************************************************/ /**
     * \brief Places a single data element in the TX_FIFO.
     *
     * \param  pstcCXPI        [in]
     * Pointer to CXPI instance register area.
     *
     * \param  data            [in]
     * Data to put in the TX_FIFO.
     * The item size is fixed to 1 byte
     *
     * \retval Refer \ref cy_en_lin_status_t
     *
     ********************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_WriteTxFIFO(volatile stc_CXPI_CH_t* pstcCXPI, const uint8_t data)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unTX_FIFO_WR.stcField.u8DATA = data;
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_SetTriggerLevelRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t triggerLevel)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if((NULL == pstcCXPI) || (triggerLevel > CXPI_RX_FIFO_ENTRIESNUM_MAX)) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unRX_FIFO_CTL.stcField.u5TRIGGER_LEVEL = triggerLevel;
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_ClearRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unRX_FIFO_CTL.stcField.u1CLEAR = 1u;
    while(pstcCXPI->unRX_FIFO_STATUS.stcField.u5USED != 0u) {
    };
    pstcCXPI->unRX_FIFO_CTL.stcField.u1CLEAR = 0u;
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_FreezeRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unRX_FIFO_CTL.stcField.u1FREEZE = 1u;
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_UnfreezeRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unRX_FIFO_CTL.stcField.u1FREEZE = 0u;
    return ret;
}

cy_en_cxpi_status_t
Cy_CXPI_GetRxFIFOStatus(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* usedEnntries, uint32_t* availEntries)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    uint32_t temp;
    temp = pstcCXPI->unRX_FIFO_STATUS.u32Register;
    *usedEnntries = (temp & CXPI_CH_RX_FIFO_STATUS_USED_Msk) >> CXPI_CH_RX_FIFO_STATUS_USED_Pos;
    *availEntries = (temp & CXPI_CH_RX_FIFO_STATUS_AVAIL_Msk) >> CXPI_CH_RX_FIFO_STATUS_AVAIL_Pos;
    return ret;
}
cy_en_cxpi_status_t Cy_CXPI_ReadRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t* data)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *data = pstcCXPI->unRX_FIFO_RD.stcField.u8DATA;
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_SilentReadRxFIFO(volatile stc_CXPI_CH_t* pstcCXPI, uint8_t* data)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *data = pstcCXPI->unRX_FIFO_RD_SILENT.stcField.u8DATA;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup interrupt source to be accepted
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  mask      [in]   The mask with the OR of the interrupt source to be accepted.
 **                          See \ref group_cxpi_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetInterruptMask(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t mask)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unINTR_MASK.u32Register = mask;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Get interrupt source to be accepted
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  mask      [out]  The mask with the OR of the interrupt source to be accepted.
 **                          See \ref group_cxpi_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetInterruptMask(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* mask)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *mask = pstcCXPI->unINTR_MASK.u32Register;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Return the status of all mask enabled interrupt causes.
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  status    [out]  Reflects a bitwise AND between the INTR and INTR_MASK registers.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetInterruptMaskedStatus(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* status)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *status = pstcCXPI->unINTR_MASKED.u32Register;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Return interupt raw status.
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  status    [out]  Reflects status of unmasked interrupt.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_GetInterruptStatus(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t* status)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    *status = pstcCXPI->unINTR.u32Register;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Clear interrupt sources in the interrupt request register.
 **
 ** \param  pstcCXPI  [in]   Pointer to CXPI instance register area.
 ** \param  status    [out]  Reflects status of unmasked interrupt.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_ClearInterrupt(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t mask)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unINTR.u32Register = mask;
    return ret;
}

/**
*****************************************************************************
**
** Send a wake-up pulse low period in Tbits that is transmitted during Standby mode
**
** \param pstcCXPI
** Pointer to CXPI instance register area.
**
** \param tWakeupLength          [in]
** Specifies the wake up pulse low period in Tbits.
** Allowable range is from 1 Tbits to 50 Tbits.
**
** \retval Refer \ref cy_en_cxpi_status_t
**
*****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SendWakeupPulse(volatile stc_CXPI_CH_t* pstcCXPI, uint32_t tWakeupLength)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if((NULL == pstcCXPI) | (tWakeupLength < CXPI_T_WAKEUP_LENGTH_MIN) | (tWakeupLength > CXPI_T_WAKEUP_LENGTH_MAX)) {
        return CY_CXPI_BAD_PARAM;
    }
    //Cy_CXPI_SetCmd(pstcCXPI,CXPI_CMD_WAKE_TO_STANDBY);
    pstcCXPI->unCTL2.stcField.u6T_WAKEUP_LENGTH = tWakeupLength - 1;
    pstcCXPI->unCMD.stcField.u1TX_WAKE_PULSE = 1;
    
    Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_WAKEUP_DONE);
    
    return ret;
}

cy_en_cxpi_status_t Cy_CXPI_IFSWait(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    pstcCXPI->unCMD.stcField.u1IFS_WAIT = 1u;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Enable CXPI channel.
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_Enable(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    /** Enable the CXPI channel
     **/
    pstcCXPI->unCTL0.stcField.u1ENABLED = 1u;
    // Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_RX_HEADER);
    // Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_ALL_ERROR_MASK);
    Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_WAKEUP_DETECT | CXPI_INTR_ALL_ERROR_MASK);
    return ret;
}

/**
 *****************************************************************************
 ** \brief Disable CXPI channel.
 **        Disabling CXPI channel causes clearing NonRetention registers.
 **        e.g) CMD register, INTR register, STATUS regiser.
 **        This behavior can be uses re-initialization after error,
 **        abort RX operation.
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_Disable(volatile stc_CXPI_CH_t* pstcCXPI)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    /** Disable the CXPI channel
     *  If a channel is disabled, CMD, STATUS, INTR MMIO registers
     *  will have their fields reset to their default value.
     **/
    Cy_CXPI_SetCmd(pstcCXPI, 0x0);
    pstcCXPI->unCTL0.stcField.u1ENABLED = 0u;
    return ret;
}

/**
 *****************************************************************************
 ** \brief Change mode of CXPI channel.
 **        Set CXPI channel to sleep mode, standy mode or normal mode
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param  mode  [in]      Desired mode of CXPI channel
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SetMode(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx, uint32_t mode)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    switch(mode) {
    case CXPI_SLEEP_MODE:
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_SLEEP);
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_WAKEUP_DETECT);
        break;
    case CXPI_STANDBY_MODE:
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_WAKE_TO_STANDBY);
        break;
    case CXPI_NORMAL_MODE:
        Cy_CXPI_SetCmd(pstcCXPI,0x0);
        cxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
            CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
            
        pstcCXPI->unCMD.stcField.u1RX_HEADER = 1;
        pstcCXPI->unCMD.stcField.u1RX_RESPONSE = 1;
        break;
    }
    return ret;
}


/**
 *****************************************************************************
 ** \brief Send PTYPE 
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in]
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.

 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SendPTYPE(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* cxpiCtx)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    cxpiCtx->state = CXPI_STATE_TX_PTYPE;
    Cy_CXPI_SetPID(pstcCXPI, CXPI_PTYPE);
    Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TX_HEADER_DONE |
            CXPI_INTR_TX_HEADER_ARB_LOST | CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_TXRX_COMPLETE |
            CXPI_INTR_ALL_ERROR_MASK);
    Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_IFS_WAIT | CXPI_CMD_TX_HEADER_RX_HEADER | CXPI_CMD_RX_RESPONSE);
    return ret;
}

/**
 *****************************************************************************
 ** \brief Send PID with option of continuing sending response
 **        Set withResponse to true if you want to send PID continued by sending response.
 **
 ** \param  pstcCXPI        [in]   Pointer to CXPI instance register area.
 ** \param cxpiCtx          [in]
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 ** \param  frameID         [in]   Frame ID that will be sent
 ** \param  withResponse    [in]   True: send response after finishing transmiting PID, false: no response
 ** \param  pResponse       [in]   Pointer to response which sent after transmiting PID, in the case "withResponse" is
 *true.
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SendPID(volatile stc_CXPI_CH_t* pstcCXPI,
    stc_CXPI_context_t* cxpiCtx,
    uint8_t frameID,
    bool withResponse,
    cxpi_response_t* pResponse,
    bool IFSwait)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    if(cxpiCtx->state != CXPI_STATE_TXRX_PENDING) {
        return CY_CXPI_BAD_PARAM;
    }
    //Cy_CXPI_SetCmd(pstcCXPI, 0x0u);
    if(!withResponse) {
        cxpiCtx->state = CXPI_STATE_TX_HDR_RX_RESP;
        /* Clear RX FIFO before hand */
        Cy_CXPI_ClearRxFIFO(pstcCXPI);
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        cxpiCtx->receivedFrame.id = frameID;
        Cy_CXPI_SetPID(pstcCXPI, frameID);
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_HEADER_DONE | CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_RX_RESPONSE_DONE |
                CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_TXRX_COMPLETE |
                CXPI_INTR_ALL_ERROR_MASK);
        if(IFSwait) {
            Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_IFS_WAIT | CXPI_CMD_TX_HEADER | CXPI_CMD_RX_HEADER | CXPI_CMD_RX_RESPONSE);
        } else {
            Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_TX_HEADER_RX_RESPONSE);
        }
    } else {    
        cxpiCtx->state = CXPI_STATE_TX_HDR_TX_RESP;
        cxpiCtx->transmittingFrame.id = frameID;
        cxpiCtx->pTransmittingFrameResponse = pResponse;

        Cy_CXPI_ClearTxFIFO(pstcCXPI);
        Cy_CXPI_ClearRxFIFO(pstcCXPI);
        Cy_CXPI_SetPID(pstcCXPI, frameID);
        Cy_CXPI_SetFI(pstcCXPI, pResponse->frameIF);
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        if(pResponse->frameIF.datalength <= CXPI_TX_FIFO_SIZE) {
            uint32_t k = 0;
            for(k = 0; k < pResponse->frameIF.datalength; k++) {
                Cy_CXPI_WriteTxFIFO(pstcCXPI, pResponse->dataBuffer[k]);
            }
            cxpiCtx->txBuf = pResponse->dataBuffer;
            cxpiCtx->txBuffCount = pResponse->frameIF.datalength;
            cxpiCtx->txBufIdx = pResponse->frameIF.datalength;
            Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_HEADER_DONE | CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TX_RESPONSE_DONE |
                    CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TXRX_COMPLETE |
                    CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
        } else {
            uint32_t k = 0;
            for(k = 0; k < CXPI_TX_FIFO_SIZE; k++) {
                Cy_CXPI_WriteTxFIFO(pstcCXPI, pResponse->dataBuffer[k]);
            }
            cxpiCtx->txBuf = pResponse->dataBuffer + CXPI_TX_FIFO_SIZE;
            cxpiCtx->txBuffCount = pResponse->frameIF.datalength - CXPI_TX_FIFO_SIZE;
            cxpiCtx->txBufIdx = CXPI_TX_FIFO_SIZE;
            Cy_CXPI_SetTriggerLevelTxFIFO(pstcCXPI, (CXPI_TX_FIFO_SIZE / 2));
            Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_HEADER_DONE | CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TX_RESPONSE_DONE |
                    CXPI_INTR_TX_FIFO_TRIGGER | CXPI_INTR_RX_RESPONSE_DONE |
                    CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_TXRX_COMPLETE | CXPI_INTR_ALL_ERROR_MASK);
        }
        cxpiCtx->rxBufIdx = 0;
        
        if(IFSwait) {
            Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_IFS_WAIT | CXPI_CMD_TX_HEADER | CXPI_CMD_TX_RESPONSE | CXPI_CMD_RX_RESPONSE | CXPI_CMD_RX_HEADER);
        } else {
            Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_TX_HEADER | CXPI_CMD_TX_RESPONSE | CXPI_CMD_RX_RESPONSE);
        }
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Send response
 **
 ** \param  pstcCXPI        [in]   Pointer to CXPI instance register area.
 ** \param  cxpiCtx         [in]
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 ** \param  pResponse       [in]   Pointer to response which need to be sent
 **
 ** \retval Refer \ref cy_en_cxpi_status_t
 **
 *****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_SendResp(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* cxpiCtx, cxpi_response_t* pResponse)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;

    if(NULL == pstcCXPI) {
        return CY_CXPI_BAD_PARAM;
    }
    cxpiCtx->state = CXPI_STATE_RX_HDR_TX_RESP;
    //cxpiCtx->pTransmittingFrameResponse = pResponse;
    Cy_CXPI_ClearTxFIFO(pstcCXPI);
    Cy_CXPI_SetFI(pstcCXPI, pResponse->frameIF);
    if(pResponse->frameIF.datalength <= CXPI_TX_FIFO_SIZE) {
        uint32_t k = 0;
        for(k = 0; k < pResponse->frameIF.datalength; k++) {
          Cy_CXPI_WriteTxFIFO(pstcCXPI, pResponse->dataBuffer[k]);
        }
        cxpiCtx->txBuf = pResponse->dataBuffer;
        cxpiCtx->txBuffCount = pResponse->frameIF.datalength;
        cxpiCtx->txBufIdx = pResponse->frameIF.datalength;
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_RESPONSE_DONE | CXPI_INTR_TIMEOUT | CXPI_INTR_TXRX_COMPLETE | CXPI_INTR_ALL_ERROR_MASK);
     } else {
        uint32_t k = 0;
        for(k = 0; k < CXPI_TX_FIFO_SIZE; k++) {
          Cy_CXPI_WriteTxFIFO(pstcCXPI, pResponse->dataBuffer[k]);
        }
        cxpiCtx->txBuf = pResponse->dataBuffer + CXPI_TX_FIFO_SIZE;
        cxpiCtx->txBuffCount = pResponse->frameIF.datalength - CXPI_TX_FIFO_SIZE;
        cxpiCtx->txBufIdx = CXPI_TX_FIFO_SIZE;
        Cy_CXPI_SetTriggerLevelTxFIFO(pstcCXPI, (CXPI_TX_FIFO_SIZE / 2u));
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_TX_FIFO_TRIGGER | CXPI_INTR_TX_RESPONSE_DONE | CXPI_INTR_TIMEOUT | CXPI_INTR_TXRX_COMPLETE | CXPI_INTR_ALL_ERROR_MASK);
    }
    Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_TX_RESPONSE);  
    return ret;
}

/**
*****************************************************************************
**
** This is the interrupt function for the CXPI channel.
** This function must be called inside the  user-defined interrupt service
** routine in order for \ref Cy_CXPI_SendPTYPE, \ref Cy_CXPI_SendPID and
** \ref Cy_CXPI_SendResp  to work.
**
** \param pstcCXPI
** Pointer to CXPI instance register area.
**
** \param cxpiCtx          [in]
** The pointer to the context structure \ref stc_CXPI_context_t allocated
** by the user. The structure is used during the CXPI operation for internal
** configuration and data keeping. The user must not modify anything
** in this structure.
**
** \retval Refer \ref cy_en_cxpi_status_t
**
*****************************************************************************/
cy_en_cxpi_status_t Cy_CXPI_Interrupt(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx)
{
    cy_en_cxpi_status_t ret = CY_CXPI_SUCCESS;
    uint32_t maskedStatus = 0;
    Cy_CXPI_GetInterruptMaskedStatus(pstcCXPI, &maskedStatus);
    // Clear accepted interrupt
    Cy_CXPI_ClearInterrupt(pstcCXPI, maskedStatus);

    if((CXPI_INTR_TX_WAKEUP_DONE & maskedStatus) != 0) {
        // wakeup pulse transmited
        if(pCxpiCtx->pNotifyTrasmittedWakeupPulse != NULL) {
            pCxpiCtx->pNotifyTrasmittedWakeupPulse();
        }
    }
    
    if((CXPI_INTR_RX_WAKEUP_DETECT & maskedStatus) != 0) {
        if(pCxpiCtx->pNotifyRXFallingEdgeDetected != NULL) {
            pCxpiCtx->pNotifyRXFallingEdgeDetected();
        }
    }
    
    if((CXPI_INTR_ALL_ERROR_MASK & maskedStatus) != 0) {
        /* There are some error */
        /* Handle error if needed. */
        Cy_CXPI_SetCmd(pstcCXPI, 0x0);

        pCxpiCtx->retriesCount = 0;
        pCxpiCtx->arbState = CXPI_ARB_PENDING;
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;

        Cy_CXPI_ClearTxFIFO(pstcCXPI);
        Cy_CXPI_ClearRxFIFO(pstcCXPI);
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        pstcCXPI->unCTL0.stcField.u5IFS = 20u;
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
        CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_RX_HEADER_RX_RESPONSE | CXPI_CMD_IFS_WAIT);
        if(pCxpiCtx->pNotifyError != NULL) {
            (pCxpiCtx->pNotifyError)(CXPI_INTR_ALL_ERROR_MASK & maskedStatus);
        }
    } else if((CXPI_INTR_TIMEOUT & maskedStatus) != 0){
        // time out!
        Cy_CXPI_ClearTxFIFO(pstcCXPI);
        Cy_CXPI_ClearRxFIFO(pstcCXPI);
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
        CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
        pCxpiCtx->arbState = CXPI_ARB_PENDING;
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_RX_HEADER_RX_RESPONSE);
    } else {
        if (pCxpiCtx->state == CXPI_STATE_TX_PTYPE){
            //PTYPE is sent
            pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
            //notify user
            if (pCxpiCtx->pNotifySentPTYPE != NULL){
                pCxpiCtx->pNotifySentPTYPE();
            }
        } else if (pCxpiCtx->state == CXPI_STATE_TXRX_PENDING){
            //Received header
            HeaderReceptionOperation(pstcCXPI, pCxpiCtx);
        } else if ((pCxpiCtx->state == CXPI_STATE_TX_HDR_TX_RESP) || (pCxpiCtx->state == CXPI_STATE_TX_HDR_RX_RESP)){            
            if((CXPI_INTR_RX_HEADER_PID_DONE & maskedStatus) != 0){
                // received header while attemping to transmit header
                // check if arbitration lost
                if (pstcCXPI->unINTR.stcField.u1TX_HEADER_ARB_LOST == 1){
                    //arbitration lost
                    //clear INTR.TX_HEADER_ARB_LOST
                    pstcCXPI->unINTR.stcField.u1TX_HEADER_ARB_LOST = 1;
                    //set arbitration state
                    if(pCxpiCtx->state == CXPI_STATE_TX_HDR_RX_RESP) {
                        pCxpiCtx->arbState = CXPI_ARB_LOST_RX_RESP;
                    } else if(pCxpiCtx->state == CXPI_STATE_TX_HDR_TX_RESP) {
                        pCxpiCtx->arbState = CXPI_ARB_LOST_TX_RESP;
                    }
                }
                HeaderReceptionOperation(pstcCXPI, pCxpiCtx);
            }  else if ((CXPI_INTR_TX_FIFO_TRIGGER & maskedStatus) != 0){
                TXHandler(pstcCXPI, pCxpiCtx);
            }  else if ((CXPI_INTR_TX_RESPONSE_DONE & maskedStatus) != 0){
                // response transmission is done
                // donothing but wait for INTR.TXRX_COMPLETE
            } else if ((CXPI_INTR_RX_FIFO_TRIGGER & maskedStatus) != 0){
                RXHandler(pstcCXPI, pCxpiCtx);
                if((CXPI_INTR_RX_RESPONSE_DONE & maskedStatus) != 0) {
                    // response received
                    RXResponseDoneHandler(pstcCXPI, pCxpiCtx);
                }
            } else if((CXPI_INTR_RX_RESPONSE_DONE & maskedStatus) != 0) {
                // response received
                RXResponseDoneHandler(pstcCXPI, pCxpiCtx);
            } else if((CXPI_INTR_TXRX_COMPLETE & maskedStatus) != 0) {
                TXRXCompletionOperation(pstcCXPI, pCxpiCtx);
            }
        } else if ((pCxpiCtx->state == CXPI_STATE_RX_HDR_TX_RESP) || (pCxpiCtx->state == CXPI_STATE_RX_HDR_RX_RESP)){
            if ((CXPI_INTR_TX_FIFO_TRIGGER & maskedStatus) != 0){
                TXHandler(pstcCXPI, pCxpiCtx);
            }  else if ((CXPI_INTR_TX_RESPONSE_DONE & maskedStatus) != 0){
                // response transmission is done
                // donothing but wait for INTR.TXRX_COMPLETE
            } else if ((CXPI_INTR_RX_FIFO_TRIGGER & maskedStatus) != 0){
                RXHandler(pstcCXPI, pCxpiCtx);
                if((CXPI_INTR_RX_RESPONSE_DONE & maskedStatus) != 0) {
                    // response received
                    RXResponseDoneHandler(pstcCXPI, pCxpiCtx);
                }
            } else if((CXPI_INTR_RX_RESPONSE_DONE & maskedStatus) != 0) {
                // response received
                RXResponseDoneHandler(pstcCXPI, pCxpiCtx);
            } else if((CXPI_INTR_TXRX_COMPLETE & maskedStatus) != 0) {
                TXRXCompletionOperation(pstcCXPI, pCxpiCtx);
            }
        }
    }
    return ret;
}


/**
 *****************************************************************************
 ** \brief Header reception operation. Read and check if the received PID is 
 ** a relevant PID.
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in] Pointer to the CXPI context
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 **
 *****************************************************************************/
void HeaderReceptionOperation(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx){
    bool respond = false;
    bool retransmit = false;
    // pid received
    // todo: check received pid and coresspond if need
    uint8_t id, parity;
    Cy_CXPI_GetPID(pstcCXPI, &id, &parity);

    if(id == CXPI_PTYPE) {
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        if(pCxpiCtx->pNotifyReceivedPTYPE != NULL) {
            pCxpiCtx->pNotifyReceivedPTYPE();
        }
    } else {
        // read received id
        pCxpiCtx->receivedFrame.id = id;
        // check if this node should responde to the received PID
        for (uint32_t tempIdx = 0; tempIdx < pCxpiCtx->numberOfAcceptedPID; tempIdx++){
            if (*(pCxpiCtx->pAcceptedPIDList + tempIdx) == id){
                respond = true;
            }
        }
        if (respond){
            // should receive or transmit response
            pCxpiCtx->state = CXPI_STATE_RX_HDR_RX_RESP;
            if(pCxpiCtx->pNotifyReceivedPID != NULL) {
                pCxpiCtx->pNotifyReceivedPID(id);
            }
        } else {
            // ignore the received PID
            // clear CMD.RX_RESPONSE to stop receiving response
            pstcCXPI->unCMD.stcField.u1RX_RESPONSE = 0;
            //check if arbitration lost and retransmission is needed or not
            if ((pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) || (pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP)){
                pCxpiCtx->retriesCount++;
                if (pCxpiCtx->retriesCount <= pCxpiCtx->maximumRetries){
                    retransmit = true;
                } else {
                    pCxpiCtx->retriesCount = 0;
                }
            }
            if (retransmit){
                // need to perform retransmission because of arbitration loss
                //move machine state back to CXPI_STATE_TXRX_PENDING
                pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
                pstcCXPI->unCTL0.stcField.u5IFS = 20;
                if(pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) {
                    Cy_CXPI_SendPID(pstcCXPI, pCxpiCtx, pCxpiCtx->transmittingFrame.id, true, pCxpiCtx->pTransmittingFrameResponse, true);
                } else if(pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP) {
                    Cy_CXPI_SendPID(pstcCXPI, pCxpiCtx, pCxpiCtx->transmittingFrame.id, false, NULL, true);
                }
            } else {
                // enable message frame reception and move machine state back to CXPI_STATE_TXRX_PENDING
                pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
                Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
                pstcCXPI->unCTL0.stcField.u5IFS = 20;
                Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
                CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
                Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_IFS_WAIT|CXPI_CMD_RX_HEADER_RX_RESPONSE);
                
                if ((pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) || (pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP)){
                    pCxpiCtx->arbState = CXPI_ARB_PENDING;
                    if(pCxpiCtx->pNotifyArbitrationLost != NULL) {
                        pCxpiCtx->pNotifyArbitrationLost();
                    }
                }
            }
        }
    }
}

/**
 *****************************************************************************
 ** \brief TXRX Completion Operation. Frame transmission or reception is completed
 ** and EOF is confirmed by hardware. Enable new frame reception or perform 
 ** retransmission if arbitration is lost and
 ** number of retransmissions does not exceed allowed number. 
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in] Pointer to the CXPI context
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 **
 *****************************************************************************/
void TXRXCompletionOperation(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx){
    bool retransmit = false;
    //clear pending command
    Cy_CXPI_SetCmd(pstcCXPI, 0);
    
    if ((pCxpiCtx->state == CXPI_STATE_RX_HDR_TX_RESP) || (pCxpiCtx->state == CXPI_STATE_RX_HDR_RX_RESP)){
        if ((pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) || (pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP)){
         pCxpiCtx->retriesCount++;
            if (pCxpiCtx->retriesCount <= pCxpiCtx->maximumRetries){
                retransmit = true;
            } else {
                pCxpiCtx->retriesCount = 0;
            }
        }
    } else {
        pCxpiCtx->arbState = CXPI_ARB_PENDING;
        pCxpiCtx->retriesCount = 0;
    }

    if((pCxpiCtx->state == CXPI_STATE_TX_HDR_TX_RESP) || (pCxpiCtx->state == CXPI_STATE_RX_HDR_TX_RESP)){
        // enable message frame reception and 
        //move machine state back to CXPI_STATE_TXRX_PENDING
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
            CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_RX_HEADER_RX_RESPONSE);
    } else {
        // if this node received response, notify user and pass the received response
        if ((pCxpiCtx->state == CXPI_STATE_TX_HDR_RX_RESP) || (pCxpiCtx->state == CXPI_STATE_RX_HDR_RX_RESP)){
        // enable message frame reception and 
        //move machine state back to CXPI_STATE_TXRX_PENDING
        pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
        Cy_CXPI_SetTriggerLevelRxFIFO(pstcCXPI, (CXPI_RX_FIFO_SIZE / 2u));
        Cy_CXPI_SetInterruptMask(pstcCXPI, CXPI_INTR_RX_HEADER_PID_DONE | CXPI_INTR_TXRX_COMPLETE |
            CXPI_INTR_RX_RESPONSE_DONE | CXPI_INTR_RX_FIFO_TRIGGER | CXPI_INTR_TIMEOUT | CXPI_INTR_ALL_ERROR_MASK);
        Cy_CXPI_SetCmd(pstcCXPI, CXPI_CMD_RX_HEADER_RX_RESPONSE);
        if(pCxpiCtx->pNotifyReceivedResponse != NULL) {
            pCxpiCtx->pNotifyReceivedResponse(&pCxpiCtx->receivedFrame);
            }
        }
    }

    if (pCxpiCtx->state == CXPI_STATE_TXRX_PENDING) {
        if (retransmit){
        // need to perform retransmission because of arbitration loss
        // move machine state back to CXPI_STATE_TXRX_PENDING
            pCxpiCtx->state = CXPI_STATE_TXRX_PENDING;
            pstcCXPI->unCTL0.stcField.u5IFS = 10;
            if(pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) {
                Cy_CXPI_SendPID(pstcCXPI, pCxpiCtx, pCxpiCtx->transmittingFrame.id, true, pCxpiCtx->pTransmittingFrameResponse, true);
            } else if(pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP) {
                Cy_CXPI_SendPID(pstcCXPI, pCxpiCtx, pCxpiCtx->transmittingFrame.id, false, NULL, true);
            }
        } else {
            if ((pCxpiCtx->arbState == CXPI_ARB_LOST_TX_RESP) || (pCxpiCtx->arbState == CXPI_ARB_LOST_RX_RESP)){
                pCxpiCtx->arbState = CXPI_ARB_PENDING;
                if(pCxpiCtx->pNotifyArbitrationLost != NULL) {
                    pCxpiCtx->pNotifyArbitrationLost();
                }
            }
        }
    } else {
        pCxpiCtx->retriesCount = 0;
        pCxpiCtx->arbState = CXPI_ARB_PENDING;
    }
}

/**
 *****************************************************************************
 ** \brief Loads TX FIFO with data
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in] Pointer to the CXPI context
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 **
 *****************************************************************************/
void TXHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx){
    // tx_fifo trigger is generated
    // todo: add data to tx_fifo
    uint32_t numToWrite;
    uint32_t temp;
    Cy_CXPI_GetTxFIFOStatus(pstcCXPI, &temp, &numToWrite);
    numToWrite = (numToWrite > pCxpiCtx->txBuffCount) ? pCxpiCtx->txBuffCount : numToWrite;
    pCxpiCtx->txBufIdx += numToWrite;
    pCxpiCtx->txBuffCount -= numToWrite;
    uint32_t idx = 0;
    uint8_t writeData = 0;
    uint8_t* buf = (uint8_t*)pCxpiCtx->txBuf;
    for(idx = 0; idx < numToWrite; idx++) {
        writeData = *(buf + idx);
        Cy_CXPI_WriteTxFIFO(pstcCXPI, writeData);
    }
    pCxpiCtx->txBuf = buf + numToWrite;
    if(pCxpiCtx->txBuffCount == 0) {
        /*Disable the TX level interrupt */
        pstcCXPI->unINTR_MASK.stcField.u1TX_FIFO_TRIGGER = 0;
    }
}

/**
 *****************************************************************************
 ** \brief Loads data from RX FIFO
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in] Pointer to the CXPI context
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 **
 *****************************************************************************/
void RXHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx){
    // rx_fifo trigger is generated
    // todo: read rx_fifo
        uint32_t numToRead;
        uint32_t temp;
        Cy_CXPI_GetRxFIFOStatus(pstcCXPI, &numToRead, &temp);
        uint32_t idx = 0;
        uint8_t readData = 0;
        for(idx = 0; idx < numToRead; idx++) {
            Cy_CXPI_ReadRxFIFO(pstcCXPI, &readData);
            pCxpiCtx->receivedFrame.response.dataBuffer[pCxpiCtx->rxBufIdx + idx] = readData;
        }
        pCxpiCtx->rxBufIdx += numToRead;
}

/**
 *****************************************************************************
 ** \brief TXRX Completion Operation. Response reception is completed
 **
 ** \param  pstcCXPI  [in]  Pointer to CXPI instance register area.
 ** \param cxpiCtx    [in] Pointer to the CXPI context
 ** The pointer to the context structure \ref stc_CXPI_context_t allocated
 ** by the user. The structure is used during the CXPI operation for internal
 ** configuration and data keeping. The user must not modify anything
 ** in this structure.
 **
 *****************************************************************************/
void RXResponseDoneHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx){
    // response received
    uint32_t numToRead;
    uint32_t temp;
    uint8_t frameId, parity;
 
    stc_cxpi_frame_info_t frameInfo;
    Cy_CXPI_GetPID(pstcCXPI, &frameId, &parity);
    Cy_CXPI_GetFI(pstcCXPI, &frameInfo);

    pCxpiCtx->receivedFrame.response.frameIF.datalength = frameInfo.datalength;
    pCxpiCtx->receivedFrame.response.frameIF.SleepInd = frameInfo.SleepInd;
    pCxpiCtx->receivedFrame.response.frameIF.WakeupInd = frameInfo.WakeupInd;
    pCxpiCtx->receivedFrame.response.frameIF.counter = frameInfo.counter;
    
    Cy_CXPI_GetRxFIFOStatus(pstcCXPI, &numToRead, &temp);
    uint8_t readData = 0;
    if (numToRead > 0){
        for(uint32_t idx = 0; idx < numToRead; idx++) {
            Cy_CXPI_ReadRxFIFO(pstcCXPI, &readData);
            pCxpiCtx->receivedFrame.response.dataBuffer[pCxpiCtx->rxBufIdx + idx] = readData;
        }
    }
    pCxpiCtx->rxBufIdx = 0;
}
/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
