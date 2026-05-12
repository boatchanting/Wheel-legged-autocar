/***************************************************************************//**
* \file cy_lin.c
* \version 1.0
*
* \brief
* Provides an API declaration of the LIN driver
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
#include "cy_lin.h"

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
 ** \brief DeInitialisation of a LIN module.
 ** This Function deinitialises the selected LIN channel.
 **
 ** \param [in]  pstcLin       Pointer to LIN instance register area
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_DeInit( volatile stc_LIN_CH_t* pstcLin )
{
    cy_en_lin_status_t status = CY_LIN_SUCCESS;
    un_LIN_CH_CTL0_t ctl0 = { 0u };

    /* Check if pointers are valid */
    if ( NULL == pstcLin )
    {
        return CY_LIN_BAD_PARAM;
    }

    /* Disable the LIN Channel */
    pstcLin->unCTL0.stcField.u1ENABLED = 0u;

    /* Clear the data registers */
    pstcLin->unDATA0.u32Register = 0uL;
    pstcLin->unDATA1.u32Register = 0uL;

    /* Clear the PID filed */
    pstcLin->unPID_CHECKSUM.stcField.u8CHECKSUM = 0u;

    ctl0.stcField.u1ENABLED = 0u;
    /* Assign the default value for Stop bit length */
    ctl0.stcField.u2STOP_BITS = LinOneStopBit;
    /* Assign the default value for LIN Transceiver Auto Enable */
    ctl0.stcField.u1AUTO_EN = 1u;
    /* Default Break Delimiter Length: 1 */
    ctl0.stcField.u2BREAK_DELIMITER_LENGTH = 1u;
    /* Default Break field length: 12  */
    ctl0.stcField.u5BREAK_WAKEUP_LENGTH = 12u;
    /* Filter setting  */
    ctl0.stcField.u1FILTER_EN= 1u;
    pstcLin->unCTL0.u32Register = ctl0.u32Register;

    return status;
}

/**
 *****************************************************************************
 ** \brief Initialisation of a LIN module.
 ** This Function initialises the LIN according the Options setup in the
 ** passed Config Struct. Several Checkings are done before that and an error
 ** is returned if invalid Modes are requested.
 **
 ** \pre The Application must configure corresponding LIN pins (SIN, SOT)
 **      according to requirements and settings of LIN instance.
 **      And must set baudrate using SysClk. LIN I/F has internal fixed
 **      oversampling value (16). Therefore LIN clock / 16 is actual
 **      baudrate.
 **
 ** \param [in]  pstcLin        Pointer to LIN instance register area
 ** \param [in]  pstcConfig     LIN module configuration. See #stc_lin_config_t.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_Init( volatile stc_LIN_CH_t* pstcLin, const stc_lin_config_t *pstcConfig)
{
    cy_en_lin_status_t status = CY_LIN_SUCCESS;

    /* Check if pointers are valid */
    if ( ( NULL == pstcLin )          ||
         ( NULL == pstcConfig ) )
    {
        status = CY_LIN_BAD_PARAM;
    }
    else if (pstcConfig->bMasterMode &&
             ((CY_LIN_MASTER_BREAK_FILED_LENGTH_MIN > pstcConfig->u8BreakFieldLength) ||
              (CY_LIN_BREAK_WAKEUP_LENGTH_BITS_MAX < pstcConfig->u8BreakFieldLength)))
    {
        status = CY_LIN_BAD_PARAM;
    }
    else if (CY_LIN_BREAK_WAKEUP_LENGTH_BITS_MAX < pstcConfig->u8BreakFieldLength)
    {
        status = CY_LIN_BAD_PARAM;
    }
    else
    {
        un_LIN_CH_CTL0_t ctl0 = { 0u };

        /* Stop bit length */
        ctl0.stcField.u2STOP_BITS = pstcConfig->enStopBit;
        /* LIN Transceiver Auto Enable by Hardware */
        ctl0.stcField.u1AUTO_EN = pstcConfig->bLinTransceiverAutoEnable;
        /* Break field length */
        ctl0.stcField.u5BREAK_WAKEUP_LENGTH = pstcConfig->u8BreakFieldLength - 1u;
        /* Break Delimiter Length: Bit8-9 */
        /* This field effect only master node header transmission. */
        ctl0.stcField.u2BREAK_DELIMITER_LENGTH = pstcConfig->enBreakDelimiterLength;
        /* Mode of Operation: Bit 24: 0 -> LIN Mode, 1 -> UART Mode */
        ctl0.stcField.u1MODE = 0u;
        /* Enable the LIN Channel */
        ctl0.stcField.u1ENABLED = 1u;
        /* Filter setting */
        ctl0.stcField.u1FILTER_EN = pstcConfig->bFilterEnable;
        /* Other settings are set to default */
        ctl0.stcField.u1BIT_ERROR_IGNORE = 0u;
        ctl0.stcField.u1PARITY = 0u;
        ctl0.stcField.u1PARITY_EN = 0u;
        pstcLin->unCTL0.u32Register = ctl0.u32Register;
    }

    return status;
}

/**
 *****************************************************************************
 ** \brief Read response data.
 **
 ** \pre Must be run after RX_RESPONSE_DONE.
 **
 ** \note This function return last received data set.
 **
 ** \param  pstcLin  [in]        Pointer to LIN instance register area.
 ** \param  u8Data   [out]       Pointer to received data.
 **                              Must have enough space for current Rx data.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_ReadData( volatile stc_LIN_CH_t* pstcLin, uint8_t  *u8Data )
{
    cy_en_lin_status_t status = CY_LIN_SUCCESS;
    uint8_t u8Cnt;
    uint8_t u8Length;

    /* Check if pointers are valid */
    if( ( NULL == pstcLin )           ||
        ( NULL == u8Data ))
    {
        status = CY_LIN_BAD_PARAM;
    }
    /* Check if the response is received successfully */
    else if( ( 0u == pstcLin->unCMD.stcField.u1RX_RESPONSE ) &&
             ( 0u == pstcLin->unSTATUS.stcField.u1RX_BUSY ) )
    {
        u8Length = pstcLin->unCTL1.stcField.u3DATA_NR + 1u;
        /* Copy the data in to u8Data array */
        un_LIN_CH_DATA0_t data0;
        un_LIN_CH_DATA1_t data1;
        data0.u32Register = pstcLin->unDATA0.u32Register;
        data1.u32Register = pstcLin->unDATA1.u32Register;
        
        for ( u8Cnt = 0; u8Cnt < u8Length; u8Cnt++ )
        {
            if( 4u > u8Cnt )
            {
                u8Data[u8Cnt] = data0.au8Byte[u8Cnt];
            }
            else
            {
                u8Data[u8Cnt] = data1.au8Byte[u8Cnt - 4u];
            }
        }
    }
    else
    {
        status = CY_LIN_BUSY;
    }

    return status;
}

/**
 *****************************************************************************
 ** \brief Write response data.
 **
 ** \param  pstcLin        [in]        Pointer to LIN instance register area.
 ** \param  au8Data        [in]        Pointer to response data.
 ** \param  u8DataLength   [in]        Data length in bytes.
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_WriteData( volatile stc_LIN_CH_t* pstcLin, const uint8_t *au8Data,
                                  uint8_t u8DataLength )
{
    cy_en_lin_status_t status = CY_LIN_SUCCESS;
    un_LIN_CH_DATA0_t data0 = { 0 };
    un_LIN_CH_DATA1_t data1 = { 0 };
    uint8_t u8Cnt;

    /* Check if NULL pointer */
    if( ( NULL == pstcLin ) ||
        ( NULL == au8Data ) )
    {
        status = CY_LIN_BAD_PARAM;
    }
    /* Check if data length is valid */
    else if( CY_LIN_DATA_LENGTH_MAX < u8DataLength )
    {
        status = CY_LIN_BAD_PARAM;
    }
    /* Check if the bus is free */
    else if( 0u == pstcLin->unSTATUS.stcField.u1TX_BUSY )
    {
        /* Write data in to the temp variables */
        for( u8Cnt = 0; u8Cnt < u8DataLength; u8Cnt++ )
        {
            if( 4u > u8Cnt )
            {
                data0.au8Byte[u8Cnt] = au8Data[u8Cnt];
            }
            else
            {
                data1.au8Byte[u8Cnt - 4u] = au8Data[u8Cnt];
            }
        }

        /* Write data to HW FIFO */
        pstcLin->unDATA0.u32Register = data0.u32Register;
        pstcLin->unDATA1.u32Register = data1.u32Register;
    }
    else
    {
        status = CY_LIN_BUSY;
        /* A requested operation could not be completed */
    }

    return status;
}

/**
 *****************************************************************************
 ** \brief Enable LIN channel.
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_Enable(volatile stc_LIN_CH_t* pstcLin)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unCTL0.stcField.u1ENABLED = 1u;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Disable LIN channel.
 **        Disabling LIN channel causes clearing none retained registers.
 **        e.g) CMD register, INTR register, STATUS regiser.
 **        This behavior can be uses re-initialization after error,
 **        abort RX operation.
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_Disable(volatile stc_LIN_CH_t* pstcLin)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unCTL0.stcField.u1ENABLED = 0u;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup LIN break/wakeup field length.
 **        Normaly this I/F is used for detection of the wakeup pulse.
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 ** \param  length   [in]  Bit length of the break/wakeup field.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetBreakWakeupFieldLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else if (CY_LIN_BREAK_WAKEUP_LENGTH_BITS_MAX < length)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unCTL0.stcField.u5BREAK_WAKEUP_LENGTH = length - 1u;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup LIN checksum type setting
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 ** \param  type     [in]  Checksum type. Refer \ref en_lin_checksum_type_t
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetChecksumType(volatile stc_LIN_CH_t* pstcLin, en_lin_checksum_type_t type)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unCTL1.stcField.u1CHECKSUM_ENHANCED = type;
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup LIN response field data length
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 ** \param  length   [in]  Data length in byte
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetDataLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) ||
        (length > CY_LIN_DATA_LENGTH_MAX) ||
        (length < CY_LIN_DATA_LENGTH_MIN))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unCTL1.stcField.u3DATA_NR = length - 1u;
    }
    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup LIN operation command
 **
 ** \note As a normal usage, following comobinations are used.
 **       - Master
 **         Header TX only          : LIN_CMD_TX_HEADER
 **         Header TX & TX response : LIN_CMD_TX_HEADER_TX_RESPONSE
 **         Header TX & RX response : LIN_CMD_TX_HEADER_RX_RESPONSE
 **       - Slave
 **         Header RX               : LIN_CMD_RX_HEADER_RX_RESPONSE
 **         TX response             : LIN_CMD_TX_RESPONSE
 **         RX response             : Already set when header RX
 **         As a typical usage, RX response is always set at header RX phase.
 **       - Wakeup frame            : LIN_CMD_TX_WAKEUP
 **       Instead of above macro, you can use raw definition LIN_CH_CMD_*_Msk defined in IO header file.
 **
 ** \param  pstcLin  [in]  Pointer to LIN instance register area.
 ** \param  command  [in]  Required operation command. Refer \ref group_lin_cmd_macro
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetCmd(volatile stc_LIN_CH_t* pstcLin, uint32_t command)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;
    un_LIN_CH_CMD_t cmdReg;
    cmdReg.u32Register = pstcLin->unCMD.u32Register;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else if (((command & (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_RX_HEADER_Msk))
                == (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_RX_HEADER_Msk))   ||
            (((command & LIN_CH_CMD_TX_WAKEUP_Msk) != 0) &&
             ((command & (LIN_CH_CMD_TX_HEADER_Msk |
                          LIN_CH_CMD_TX_RESPONSE_Msk |
                          LIN_CH_CMD_RX_HEADER_Msk |
                          LIN_CH_CMD_RX_RESPONSE_Msk)) != 0)))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else if (((cmdReg.stcField.u1TX_HEADER != 0) && (command & LIN_CH_CMD_RX_HEADER_Msk) != 0) ||
             ((cmdReg.stcField.u1RX_HEADER != 0) && (command & LIN_CH_CMD_TX_HEADER_Msk) != 0) ||
             ((cmdReg.stcField.u1TX_WAKEUP != 0) &&
                            ((command & (LIN_CH_CMD_TX_HEADER_Msk |
                                         LIN_CH_CMD_TX_RESPONSE_Msk |
                                         LIN_CH_CMD_RX_HEADER_Msk |
                                         LIN_CH_CMD_RX_RESPONSE_Msk)) != 0)))
    {
        ret = CY_LIN_BUSY;
    }
    else
    {
        pstcLin->unCMD.u32Register = command;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup LIN header for master tx header operation
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  id      [in]  ID value
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t id)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;
    uint8_t TempPID;
    uint8_t Parity_P1, Parity_P0;

    if ((NULL == pstcLin) ||
        (CY_LIN_ID_MAX < id))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        /* Calculate the Parity bits P0 & P1 */
        Parity_P0 = ((id) ^ (id>>1u) ^
                     (id>>2u) ^ (id>>4u)) & 0x01u;
        Parity_P1 = (~((id>>1u) ^ (id>>3u) ^
                       (id>>4u) ^ (id>>5u))) & 0x01u;
        /* Assign the Parity bits and the header values in to the TempPID */
        TempPID = id | ((uint8_t) Parity_P0<<6u) | ((uint8_t) Parity_P1<<7u);
        /* Write the TempID value in to the TX_HEADER register */
        pstcLin->unPID_CHECKSUM.stcField.u8PID = TempPID;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Return received LIN header
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  id      [out] Received ID value.
 ** \param  parity  [out] Received parity value.
 **                       Note that parity check is always done by HW automatically.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_GetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t *id, uint8_t *parity)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) ||
        (NULL == id)      ||
        (NULL == parity))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        /* Store received ID and parity bits */
        uint8_t temp = pstcLin->unPID_CHECKSUM.stcField.u8PID;
        *parity = (temp >> 6u);
        *id = (temp & CY_LIN_ID_MAX);
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Setup interrupt source to be accepted.
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  mask    [in]  The mask with the OR of the interrupt source to be accepted.
 **                       See \ref group_lin_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_SetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t mask)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unINTR_MASK.u32Register = mask;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Return interupt mask setting.
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  status  [out] The mask with the OR of the interrupt source which is masked.
 **                       See \ref group_lin_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_GetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t *mask)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) || 
        (NULL == mask))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        *mask = pstcLin->unINTR_MASK.u32Register;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Return interupt masked status.
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  status  [out] The mask with the OR of the interrupt source which occurs.
 **                       See \ref group_lin_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_GetInterruptMaskedStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) || 
        (NULL == status))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        *status = pstcLin->unINTR_MASKED.u32Register;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Return interupt raw status.
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  status  [out] The mask with the OR of the interrupt source which occurs.
 **                       See \ref group_lin_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_GetInterruptStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status)
{

    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) || 
        (NULL == status))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        *status = pstcLin->unINTR.u32Register;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Clear interupt status.
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  mask    [in]  The mask with the OR of the interrupt source to be cleared.
 **                       See \ref group_lin_intr_mask_macro for the set of constants.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_ClearInterrupt(volatile stc_LIN_CH_t* pstcLin, uint32_t mask)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if (NULL == pstcLin)
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        pstcLin->unINTR.u32Register = mask;
    }

    return ret;
}

/**
 *****************************************************************************
 ** \brief Return LIN module status
 **
 ** \param  pstcLin [in]  Pointer to LIN instance register area.
 ** \param  status  [out] LIN module status.
 **
 ** \retval Refer \ref cy_en_lin_status_t
 **
 *****************************************************************************/
cy_en_lin_status_t Cy_Lin_GetStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status)
{
    cy_en_lin_status_t ret = CY_LIN_SUCCESS;

    if ((NULL == pstcLin) || 
        (NULL == status))
    {
        ret = CY_LIN_BAD_PARAM;
    }
    else
    {
        *status = pstcLin->unSTATUS.u32Register;
    }

    return ret;
}


/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
