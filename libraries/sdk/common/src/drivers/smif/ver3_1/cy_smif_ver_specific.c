/***************************************************************************//**
* \file cy_smif_ver_specific.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the SMIF driver APIs.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "smif/ver3_1/cy_smif_ver_specific.h"

#ifdef CY_IP_MXSMIF

#if defined(__cplusplus)
extern "C" {
#endif

/*******************************************************************************
* Function Name: Cy_SMIF_Init
****************************************************************************//**
*
* This function initializes the SMIF block as a communication block. The user
* must ensure that the SMIF interrupt is disabled while this function
* is called. Enabling the interrupts can lead to triggering in the middle
* of the initialization operation, which can lead to erroneous initialization.
*
* As parameters, this function takes the SMIF register base address and a
* context structure along with the configuration needed for the SMIF block,
* stored in a config
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param config
* Passes a configuration structure that configures the SMIF block for operation.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \param timeout
* A timeout in microseconds for blocking APIs in use.
*
*
* \note Make sure that the interrupts are initialized and disabled.
*
* \return
*     - \ref CY_SMIF_BAD_PARAM
*     - \ref CY_SMIF_SUCCESS
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Init(volatile cy_stc_smif_reg_t *base, 
                                    cy_stc_smif_config_t const *config,
                                    uint32_t timeout,
                                    cy_stc_smif_context_t *context)
{
    if((NULL == base) || (NULL == config) || (NULL == context))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(config->dlpAuto == CY_SMIF_DLP_UPDATE_AUTO)
    {
        if((config->rxClk != CY_SMIF_INTERNAL_CLK) && (config->rxClk != CY_SMIF_INV_INTERNAL_CLK))
        {
            return CY_SMIF_BAD_PARAM;
        }

        if(config->delaySel == CY_SMIF_NO_DELAY_SEL)
        {
            return CY_SMIF_BAD_PARAM;
        }
    }

    /* Global variables initialization */
    context->txBufferAddress = NULL;
    context->txBufferCounter = 0;
    context->rxBufferAddress = NULL;
    context->rxBufferCounter = 0;
    context->transferStatus  = CY_SMIF_STARTED;
    context->txCmpltCb       = NULL;
    context->rxCmpltCb       = NULL;
    context->preCmdDataRate  = CY_SMIF_SDR;
    context->preCmdWidth     = CY_SMIF_WIDTH_SINGLE;
    context->txCmdBufCounter = 0;

    /* Configure the initial interrupt mask */
    /* Disable the TR_TX_REQ and TR_RX_REQ interrupts */
    cy_un_smif_reg_intr_mask_t tempIntrMask;
    tempIntrMask.u32Register = base->unINTR_MASK.u32Register;
    tempIntrMask.stcField.u1TR_TX_REQ = 0u;
    tempIntrMask.stcField.u1TR_RX_REQ = 0u;
    base->unINTR_MASK.u32Register = tempIntrMask.u32Register;

    /* Configure the SMIF interface */
    cy_un_smif_reg_ctl_t tempCtl;
    tempCtl.u32Register = base->unCTL.u32Register;
    tempCtl.stcField.u1CLOCK_IF_TX_SEL          = config->txClk;
    tempCtl.stcField.u3CLOCK_IF_RX_SEL          = config->rxClk;
    if(config->delaySel == CY_SMIF_NO_DELAY_SEL)
    {
        tempCtl.stcField.u1DELAY_TAP_ENABLED    = 0u;
    }
    else
    {
        tempCtl.stcField.u1DELAY_TAP_ENABLED    = 1u;
        tempCtl.stcField.u3DELAY_LINE_SEL       = config->delaySel;
    }
    tempCtl.stcField.u1INT_CLOCK_DL_ENABLED     = (uint8_t)config->dlpAuto;
    tempCtl.stcField.u1XIP_MODE                 = config->mode;
    tempCtl.stcField.u2INT_CLOCK_CAPTURE_CYCLE  = config->capDelay;
    tempCtl.stcField.u3DESELECT_DELAY           = config->deselectDelay;
    tempCtl.stcField.u2SELECT_SETUP_DELAY       = config->setupDelay;
    tempCtl.stcField.u2SELECT_HOLD_DELAY        = config->holdDelay;
    tempCtl.stcField.u1BLOCK                    = config->blockEvent;
    tempCtl.stcField.u1CLOCK_IF_SEL             = config->clkIfSrc;

    base->unCTL.u32Register = tempCtl.u32Register;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeInit
****************************************************************************//**
*
* This function de-initializes the SMIF block to default values.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \note The SMIF must be disabled before calling the function. Call
*  \ref Cy_SMIF_Disable
*
*******************************************************************************/
void Cy_SMIF_DeInit(volatile cy_stc_smif_reg_t *base)
{
    /* Configure the SMIF interface to default values. 
    * The default value is 0. 
    */
    base->unCTL.u32Register = CY_SMIF_CTL_REG_DEFAULT;
    base->unTX_DATA_MMIO_FIFO_CTL.u32Register      = 0U;
    base->unRX_DATA_MMIO_FIFO_CTL.u32Register = 0U;
    base->unINTR_MASK.u32Register             = 0U;
    for(uint32_t idx = 0UL; idx < CY_SMIF_GetDeviceNumber(base); idx++)
    {
        base->DEVICE[idx].unCTL.u32Register = 0U;
        /* Read the register to flush the buffer */
        (void) base->DEVICE[idx].unCTL.u32Register;
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_Encrypt()
****************************************************************************//**
*
* Uses the Encryption engine to create an encrypted result when the input, key
* and data arrays are provided. The AES-128 encryption of the address with the
* key, fetching the result and XOR with the data array are all done in the
* function. The operational scheme is the following:
*                   data = XOR(AES128(address, key), data)
* Decryption is done using the input data-array identically to the encryption.
* In the XIP mode, encryption and decryption are done without calling this
* function. The operational scheme in the XIP mode is the same. The address
* parameter in the XIP mode equals the actual address in the TVII memory map.
* The SMIF encryption engine is designed for code storage.
* For data storage, the encryption key can be changed.
* For sensitive data, the Crypto block is used.
*
* \note The API does not have access to the encryption key. The key must be
* placed in the register before calling this API. The crypto routine
* that can access the key storage area is recommended. This crypto routine is
* typically a protection context 0 function.
*
* \note This is a blocking API. The API waits for encryption completion. Will
* exit if a timeout is set (not equal to 0) and expired.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param address
* The address that gets encrypted is a masked 16-byte block address. The 32-bit
* address with the last 4 bits masked is placed as the last 4 bytes in the
* 128-bit input. The rest of the higher bit for the 128 bits are padded zeros.
* PA[127:0]:
* PA[3:0] = 0
* PA[7:4] = ADDR[7:4].
* PA[15:8] = ADDR[15:8].
* PA[23:16] = ADDR[23:16].
* PA[31:24] = ADDR[31:24].
* The other twelve of the sixteen plain text address bytes of PA[127:0] are "0":
* PA[127:32] = "0".
*
* \param dataInOut
* This is the location where the input data-array is passed while the function
* is called. This array gets populated with the result after encryption is
* completed.
*
* \param sizeInByte
* Provides a size of the array.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of the command transmit.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_Encrypt(volatile cy_stc_smif_reg_t *base,
                                        uint32_t address,
                                        uint8_t dataInOut[],
                                        uint32_t sizeInByte,
                                        cy_stc_smif_context_t const *context)
{
    if((sizeInByte == 0UL) ||
       (sizeInByte % CY_SMIF_AES128_BYTES) != 0UL) // The user needs to do padding in application.
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(dataInOut == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(base->SMIF_CRYPTO[0].unCRYPTO_CMD.stcField.u1START != CY_SMIF_CRYPTO_COMPLETED)
    {
        return CY_SMIF_BAD_STATUS;
    }

    /* Fill the output array */
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    uint32_t blockNum          = (sizeInByte / CY_SMIF_AES128_BYTES);
    for(uint32_t i_AES_Block = 0U; i_AES_Block < blockNum; i_AES_Block++)
    {
        uint32_t timeoutUnits      = context->timeout;
        uint32_t addrOffset = i_AES_Block * CY_SMIF_AES128_BYTES;

        /* Fill the input field */
        uint32_t validPosOfINPUT0 = base->SMIF_CRYPTO[0].unCRYPTO_INPUT0.u32Register & (~CY_SMIF_CRYPTO_ADDR_MASK);
        base->SMIF_CRYPTO[0].unCRYPTO_INPUT0.u32Register = ((address + addrOffset) & CY_SMIF_CRYPTO_ADDR_MASK) + validPosOfINPUT0;

        /* Start the encryption */
        base->SMIF_CRYPTO[0].unCRYPTO_CMD.stcField.u1START = CY_SMIF_CRYPTO_START;

        while((base->SMIF_CRYPTO[0].unCRYPTO_CMD.stcField.u1START != CY_SMIF_CRYPTO_COMPLETED) && (CY_SMIF_EXCEED_TIMEOUT != status))
        {
            /* Wait until the encryption is completed and check the 
            * timeout 
            */
            status = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }

        if (status == CY_SMIF_EXCEED_TIMEOUT)
        {
            break;
        }

        *(uint32_t*)&dataInOut[addrOffset]        ^= base->SMIF_CRYPTO[0].unCRYPTO_OUTPUT0.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 4ul]  ^= base->SMIF_CRYPTO[0].unCRYPTO_OUTPUT1.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 8ul]  ^= base->SMIF_CRYPTO[0].unCRYPTO_OUTPUT2.u32Register;
        *(uint32_t*)&dataInOut[addrOffset + 12ul] ^= base->SMIF_CRYPTO[0].unCRYPTO_OUTPUT3.u32Register;
    }
    return (status);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetDelayLineForFreq
****************************************************************************//**
*
* Selects the right delay line for the specified frequency, so that the taps of
* that delay line should give enough possibilities to fine tune the delay in a 
* subsequent calibration step.
*
* \note Expects that SMIF is not busy!
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param freqMhz
* The frequency of the SMIF clock (external)
*
* \param isDDR
* If the protocol has double data rate (e.g. in case of HyperBus)
*
* \param calculatedTap
* Caller can optionally retrieve the tap number index that was calculated by this 
* function (value: 0 .. (SMIFn_DELAY_TAPS_NR - 1)). Pass NULL if value is not needed
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetDelayLineForFreq(volatile cy_stc_smif_reg_t *base, uint8_t freqMhz, bool isDDR, uint8_t *calculatedTap)
{
    // TODO: Device dependent delays (and number of delay lines)
    static const float32_t minDelayPerTapInNs[CY_SMIF_DRV_SMIF0_DELAY_LINES_NR] = {0.0559, 0.1197, 0.1935, 0.3973};
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    uint8_t tapNumCalculated;
    uint8_t delayLine;
    uint8_t maxTaps = CY_SMIF_GetDelayTapsNumber(base);

    // Calculate the offset to the nominal center of the data eye
    float32_t clockPeriodNs = 1000.0 / freqMhz;
    float32_t dataPeriodNs = clockPeriodNs / ((isDDR == true) ? 2.0 : 1.0);
    float32_t dataPeriodCenterNs = dataPeriodNs / 2.0;

    // Check all scan lines whether the number of taps required to hit the nominal center of the data eye are below the actual number of taps per delay line
    for(delayLine = 0; delayLine < CY_SMIF_DRV_SMIF0_DELAY_LINES_NR; delayLine++)
    {
        tapNumCalculated = (uint8_t) ((dataPeriodCenterNs / minDelayPerTapInNs[delayLine]) + 0.5f);
        if(tapNumCalculated <= maxTaps)
        {
            break;
        }
    }

    // No matching delay line found, use the one with the largest delays and return error...
    if(delayLine == 4)
    {
        delayLine = 3;
        status = CY_SMIF_BAD_PARAM;
    }

    Cy_SMIF_Set_DelayLineSel(base, (cy_en_smif_delay_line_t)delayLine);

    if(calculatedTap != NULL)
    {
        // If no matching delay line has been found, return the maximum number of taps
        *calculatedTap = ((tapNumCalculated < maxTaps) ? tapNumCalculated : maxTaps) - 1;
    }

    return status;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMasterDLP
****************************************************************************//**
*
* This function sets Data Learning Pattern to be compared.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param dlp
* DLP value to be set.
*
*******************************************************************************/
void Cy_SMIF_SetMasterDLP(volatile cy_stc_smif_reg_t *base, uint16_t dlp)
{
    base->unDL_CTL.stcField.u16DLP = dlp;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetMasterDLP
****************************************************************************//**
*
* This function reads Data Learning Pattern which have been set.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Master DLP value.
*
*******************************************************************************/
uint16_t Cy_SMIF_GetMasterDLP(volatile cy_stc_smif_reg_t *base)
{
    return (uint16_t)(base->unDL_CTL.stcField.u16DLP);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMasterDLP_Size
****************************************************************************//**
*
* This function writes DLP size to the register.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param size
* DLP size value to be set to the register. Can be 1 ~ 16.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetMasterDLP_Size(volatile cy_stc_smif_reg_t *base, uint32_t size)
{
    if((size == 0) || (16 < size))
    {
        return CY_SMIF_BAD_PARAM;
    }

    base->unDL_CTL.stcField.u4DLP_SIZE = size - 1ul;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetMasterDLP_Size
****************************************************************************//**
*
* This function reads DLP size which have been set.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Master DLP size value.
*
*******************************************************************************/
uint8_t Cy_SMIF_GetMasterDLP_Size(volatile cy_stc_smif_reg_t *base)
{
    return (uint8_t)(base->unDL_CTL.stcField.u4DLP_SIZE);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMasterDLP_WarnLevel
****************************************************************************//**
*
* This function writes DLP warning level to the register.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param warnLevel
* Warning level value to be set to the resister.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetMasterDLP_WarnLevel(volatile cy_stc_smif_reg_t *base, uint32_t warnLevel)
{
    if(15 < warnLevel)
    {
        return CY_SMIF_BAD_PARAM;
    }

    base->unDL_CTL.stcField.u4DL_WARNING_LEVEL = warnLevel;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetMasterDLP_WarnLevel
****************************************************************************//**
*
* This function reads DLP warning level which have been set.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Master DLP warning level value.
*
*******************************************************************************/
uint8_t Cy_SMIF_GetMasterDLP_WarnLevel(volatile cy_stc_smif_reg_t *base)
{
    return (uint8_t)(base->unDL_CTL.stcField.u4DL_WARNING_LEVEL);
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetTapNumCapturedCorrectDLP
****************************************************************************//**
*
* This function return tap number indicated by input bit number.
* \note not tested yet. this function may cause hardfault.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param bit
* The tap number of the bit will be read.
*
* \return tap number
*
*******************************************************************************/
uint8_t Cy_SMIF_GetTapNumCapturedCorrectDLP(volatile cy_stc_smif_reg_t *base, uint8_t bit)
{
    CY_ASSERT_L2(bit < 8u);

    if(bit < 4u)
    {
        return(base->unDL_STATUS0.au8Byte[bit]);
    }
    else
    {
        return(base->unDL_STATUS1.au8Byte[bit - 4u]);
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_Set_DelayLineSel
****************************************************************************//**
*
* This function enable delay line, and set line selection which is input.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param lineSel
* delay line selection to be set
*
* \return status (see \ref cy_en_smif_status_t).
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Set_DelayLineSel(volatile cy_stc_smif_reg_t *base, cy_en_smif_delay_line_t lineSel)
{
    if(lineSel == CY_SMIF_NO_DELAY_SEL)
    {
        base->unCTL.stcField.u1DELAY_TAP_ENABLED = 0;
        return CY_SMIF_SUCCESS;
    }

    cy_un_smif_reg_ctl_t tempCTL;
    tempCTL.u32Register = base->unCTL.u32Register;

    tempCTL.stcField.u1DELAY_TAP_ENABLED = 1;
    tempCTL.stcField.u3DELAY_LINE_SEL    = lineSel;

    base->unCTL.u32Register = tempCTL.u32Register;
    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_Get_DelayLineSel
****************************************************************************//**
*
* This function returns the currently selected delay line
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return delay line (see \ref cy_en_smif_delay_line_t).
*
*******************************************************************************/
cy_en_smif_delay_line_t Cy_SMIF_Get_DelayLineSel(volatile cy_stc_smif_reg_t *base)
{
    if(base->unCTL.stcField.u1DELAY_TAP_ENABLED == 0)
    {
        return CY_SMIF_NO_DELAY_SEL;
    }
    else
    {
        return (cy_en_smif_delay_line_t) base->unCTL.stcField.u3DELAY_LINE_SEL;
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_Set_DelayTapSel
****************************************************************************//**
*
* This function sets delay tap number for the SMIF (common for all its devices).
* To ensure compatibility with users of this API independent of the current
* SMIF IP version, it accepts any SMIF pointer as parameter, e.g. a SMIF base
* pointer or a SMIF_DEVICE base pointer. This means this API can be called with
* a SMIF_DEVICE base pointer and the calling code will work with CY_SMIF_NEW_VERSION
* or CY_SMIF_OLD_VERSION
*
* \param smif_or_device_base
* Holds the base address of the SMIF block or SMIF_DEVICE block registers.
*
* \param tapSel
* delay tap selection to be set
*
* \return status (see \ref cy_en_smif_status_t).
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Set_DelayTapSel(volatile void *smif_or_device_base, uint8_t tapSel)
{
    if(tapSel > (CY_SMIF_GetDelayTapsNumber(smif_or_device_base) - 1))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if((smif_or_device_base != CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) && (smif_or_device_base != CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return CY_SMIF_BAD_PARAM;
    }

    ((volatile cy_stc_smif_reg_device_t*)smif_or_device_base)->unDELAY_TAP_SEL.stcField.u8SEL = tapSel;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_Get_DelayTapSel
****************************************************************************//**
*
* This function returns delay tap number which has been set for the SMIF (common for all its devices).
* To ensure compatibility with users of this API independent of the current
* SMIF IP version, it accepts any SMIF pointer as parameter, e.g. a SMIF base
* pointer or a SMIF_DEVICE base pointer. This means this API can be called with
* a SMIF_DEVICE base pointer and the calling code will work with CY_SMIF_NEW_VERSION
* or CY_SMIF_OLD_VERSION
*
* \param smif_or_device_base
* Holds the base address of the SMIF block or SMIF_DEVICE block registers.
*
* \return read tap selection
*
*******************************************************************************/
uint8_t Cy_SMIF_Get_DelayTapSel(volatile void *smif_or_device_base)
{
    if((smif_or_device_base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (smif_or_device_base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return (uint8_t)(((volatile cy_stc_smif_reg_device_t*)smif_or_device_base)->unDELAY_TAP_SEL.stcField.u8SEL);
    }
    else
    {
        // Bad parameter input.
        return 0u;
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetTxFifoTriggerLevel()
****************************************************************************//**
*
* This function is used to set a trigger level for the TX FIFO. This value must
* be an integer between 0 and 7. For the normal mode only.
* The triggering is active when TX_DATA_FIFO_STATUS <= level.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param level
* The trigger level to set (0-8).
*
*******************************************************************************/
void Cy_SMIF_SetTxFifoTriggerLevel(volatile cy_stc_smif_reg_t *base, uint32_t level)
{
    CY_ASSERT_L2(level < CY_SMIF_MAX_TX_TR_LEVEL);
    base->unTX_DATA_MMIO_FIFO_CTL.u32Register = level;
}

/*******************************************************************************
* Function Name:  Cy_SMIF_SetCryptoInput128
****************************************************************************//**
*
* This function sets 128 bit data of crypto input (plain text).
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param inData Address of input 16 byte input data to be crypted
*
*******************************************************************************/
void Cy_SMIF_SetCryptoInput128(volatile cy_stc_smif_reg_t *base, const uint8_t inData[])
{
    base->SMIF_CRYPTO[0].unCRYPTO_INPUT0.u32Register = *(uint32_t*)&inData[0u];
    base->SMIF_CRYPTO[0].unCRYPTO_INPUT1.u32Register = *(uint32_t*)&inData[4u];
    base->SMIF_CRYPTO[0].unCRYPTO_INPUT2.u32Register = *(uint32_t*)&inData[8u];
    base->SMIF_CRYPTO[0].unCRYPTO_INPUT3.u32Register = *(uint32_t*)&inData[12u];
}

/*******************************************************************************
* Function Name:  Cy_SMIF_SetCryptoKey128
****************************************************************************//**
*
* This function sets 128 bit data of crypto key.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param key Address of 16 byte AES key data
*
*******************************************************************************/
void Cy_SMIF_SetCryptoKey128(volatile cy_stc_smif_reg_t *base, const uint8_t key[])
{
    base->SMIF_CRYPTO[0].unCRYPTO_KEY0.u32Register = *(uint32_t*)&key[0u];
    base->SMIF_CRYPTO[0].unCRYPTO_KEY1.u32Register = *(uint32_t*)&key[4u];
    base->SMIF_CRYPTO[0].unCRYPTO_KEY2.u32Register = *(uint32_t*)&key[8u];
    base->SMIF_CRYPTO[0].unCRYPTO_KEY3.u32Register = *(uint32_t*)&key[12u];
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetTxFifoStatus()
****************************************************************************//**
*
* This function is used to read the status of the TX FIFO.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns the number of the entries in the TX FIFO.
*
*******************************************************************************/
uint32_t  Cy_SMIF_GetTxFifoStatus(volatile cy_stc_smif_reg_t *base)
{
    return (base->unTX_DATA_MMIO_FIFO_STATUS.stcField.u4USED4);
}

/*******************************************************************************
* Function Name: Cy_SMIF_PushCmdFifo()
****************************************************************************//**
*
*  Push input data to SMIF tx command fifo.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param data
* The data to be pushed.
*
* \param sizeInWord
* The size of the data to be pushed in words.
*
*******************************************************************************/
void Cy_SMIF_PushCmdFifo(volatile cy_stc_smif_reg_t *base, uint32_t data[], uint32_t sizeInWord)
{
    for(uint32_t idx = 0ul; idx < sizeInWord; idx++)
    {
        base->unTX_CMD_MMIO_FIFO_WR.u32Register = data[idx];
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_PushTxFifo
***************************************************************************//***
*
* \internal
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* This function writes data in the TX FIFO SMIF buffer by 4, 2, or 1 bytes based
* on the residual number of bytes and the available space in the TX FIFO.
*
*******************************************************************************/
void Cy_SMIF_PushTxFifo(volatile cy_stc_smif_reg_t *baseaddr, cy_stc_smif_context_t *context)
{
    /* The variable that shows which is smaller: the free FIFO size or amount of bytes to be sent */
    uint32_t freeFifoBytes = CY_SMIF_TX_DATA_FIFO_STATUS_RANGE - Cy_SMIF_GetTxFifoStatus(baseaddr);;
    uint32_t buffCounter   = context->txBufferCounter;

    // writeBytes = MIN(freeFifoBytes, buffCounter);
    uint32_t writeBytes    = (freeFifoBytes > buffCounter)? buffCounter: freeFifoBytes;

    // If it sends 2 byte per one clock, then it needs using TX_DATA_FIFO_WR1ODD when sending one byte.
    // Save proper one byte sending fifo address into ptr1ByteFifo accordingly.
    uint32_t* ptr1ByteFifo;
    if((context->preCmdDataRate == CY_SMIF_DDR) &&(context->preCmdWidth == CY_SMIF_WIDTH_OCTAL))
    {
        ptr1ByteFifo = (uint32_t*)&(baseaddr->unTX_DATA_MMIO_FIFO_WR1ODD);
    }
    else
    {
        ptr1ByteFifo = (uint32_t*)&(baseaddr->unTX_DATA_MMIO_FIFO_WR1);
    }

    /* Check that after a FIFO Write, no data/FIFO space remains */
    uint8_t *buff = (uint8_t*)context->txBufferAddress;
    while (writeBytes != 0U)
    {
        // Even if writeBytes == 1, do not send it. Wait more space becoming available.
        // Because 1 byte fifo (TX_DATA_FIFO_WR1) should be used only last 1 byte.
        // If bufferCounter == 1, it means it is the last byte. then send it.
        if((writeBytes >= 2U) || (buffCounter == 1))
        {
            if(writeBytes == 1)
            {
                // Enter here only when this is the last sending.
                *ptr1ByteFifo = buff[0U];
            }
            else if((writeBytes == 2u) || (writeBytes == 3u))
            {
                baseaddr->unTX_DATA_MMIO_FIFO_WR2.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], false);
                writeBytes = 2u;
            }
            else if((writeBytes == 4u) || (writeBytes == 5u))
            {
                baseaddr->unTX_DATA_MMIO_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
                writeBytes = 4u;
            }
            else if((writeBytes == 6u) || (writeBytes == 7u))
            {
                baseaddr->unTX_DATA_MMIO_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
                baseaddr->unTX_DATA_MMIO_FIFO_WR2.u32Register = Cy_SMIF_PackBytesArray(&buff[4U], false);
                writeBytes = 6u;
            }
            else /* writeBytes => 8*/
            {
                baseaddr->unTX_DATA_MMIO_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[0U], true);
                baseaddr->unTX_DATA_MMIO_FIFO_WR4.u32Register = Cy_SMIF_PackBytesArray(&buff[4U], true);
                writeBytes = 8u;
            }
            buff          = &buff[writeBytes];
            buffCounter  -= writeBytes;
        }

        /* Check if we already got new data in TX_FIFO*/
        freeFifoBytes = CY_SMIF_TX_DATA_FIFO_STATUS_RANGE - Cy_SMIF_GetTxFifoStatus(baseaddr);
        writeBytes    = (freeFifoBytes > buffCounter)? buffCounter: freeFifoBytes;
    }

    /* Save changes in the context */
    context->txBufferAddress = buff;
    context->txBufferCounter = buffCounter;

    /* Check if all bytes are sent */
    if (buffCounter == 0u)
    {
        /* Disable the TR_TX_REQ interrupt */
        baseaddr->unINTR_MASK.stcField.u1TR_TX_REQ = 0u;

        context->transferStatus = CY_SMIF_SEND_CMPLT;
        if (NULL != context->txCmpltCb)
        {
            context->txCmpltCb((uint32_t) CY_SMIF_SEND_CMPLT);
        }
    }
}

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

/* [] END OF FILE */
