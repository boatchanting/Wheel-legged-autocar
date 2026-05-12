/***************************************************************************//**
* \file cy_pdma.c
* \version 1.0
*
* \brief
* The PDMA driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_pdma.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_Init
****************************************************************************//**
*
* This function initializes descriptor structure in SRAM from a pre-initialized
* configuration structure.
* This function initializes only the descriptor and not the channel.
* This function will operate on the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is defined in Flash, then the user must use the
* configuration defines to initialize it.
*
* \param descriptor
* This is an instance of the descriptor structure declared by the user and
* initialized by this function.
*
* \param config
* This is a configuration structure that has all initialization information for
* the descriptor.
*
* \return
* This function returns /ref cy_en_pdma_status_t.
*
*******************************************************************************/
cy_en_pdma_status_t Cy_PDMA_Descr_Init(cy_stc_pdma_descr_t* descriptor, const cy_stc_pdma_descr_config_t* config)
{
    cy_en_pdma_status_t retVal = CY_PDMA_ERR_UNC;

    if ((descriptor != NULL) && (config != NULL))
    {
        descriptor->unPDMA_DESCR_CTL.stcField.u2WAIT_FOR_DEACT          =       config->deact;
        descriptor->unPDMA_DESCR_CTL.stcField.u2INTR_TYPE               =       config->intrType;
        descriptor->unPDMA_DESCR_CTL.stcField.u2TR_OUT_TYPE             =       config->trigoutType;
        descriptor->unPDMA_DESCR_CTL.stcField.u2TR_IN_TYPE              =       config->triginType;
        descriptor->unPDMA_DESCR_CTL.stcField.u1SRC_TRANSFER_SIZE       =       config->srcTxfrSize;
        descriptor->unPDMA_DESCR_CTL.stcField.u1DST_TRANSFER_SIZE       =       config->destTxfrSize;
        descriptor->unPDMA_DESCR_CTL.stcField.u1CH_DISABLE              =       config->chStateAtCmplt;
        descriptor->unPDMA_DESCR_CTL.stcField.u2DATA_SIZE               =       config->dataSize;
        descriptor->unPDMA_DESCR_CTL.stcField.u2DESCR_TYPE              =       config->descrType;

        descriptor->u32PDMA_DESCR_SRC                                   =       (uint32_t)config->srcAddr;
        descriptor->u32PDMA_DESCR_DST                                   =       (uint32_t)config->destAddr;

        switch(config->descrType)
        {
        case (uint32_t)CY_PDMA_SINGLE_TRANSFER:
        {
            descriptor->unPDMA_DESCR_X_CTL.u32Register              =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_PDMA_1D_TRANSFER:
        {
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12SRC_X_INCR   =       (uint32_t)config->srcXincr;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12DST_X_INCR   =       (uint32_t)config->destXincr;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u8X_COUNT       =       (uint32_t)((config->xCount) - 1ul);
            descriptor->unPDMA_DESCR_Y_CTL.u32Register              =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_PDMA_CRC_TRANSFER:
        {
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12SRC_X_INCR   =       (uint32_t)config->srcXincr;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12DST_X_INCR   =       0ul;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u8X_COUNT       =       (uint32_t)((config->xCount) - 1ul);
            descriptor->unPDMA_DESCR_Y_CTL.u32Register              =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_PDMA_2D_TRANSFER:
        {
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12SRC_X_INCR   =       (uint32_t)config->srcXincr;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u12DST_X_INCR   =       (uint32_t)config->destXincr;
            descriptor->unPDMA_DESCR_X_CTL.stcField.u8X_COUNT       =       (uint32_t)((config->xCount) - 1ul);

            descriptor->unPDMA_DESCR_Y_CTL.stcField.u12SRC_Y_INCR   =       (uint32_t)config->srcYincr;
            descriptor->unPDMA_DESCR_Y_CTL.stcField.u12DST_Y_INCR   =       (uint32_t)config->destYincr;
            descriptor->unPDMA_DESCR_Y_CTL.stcField.u8Y_COUNT       =       (uint32_t)((config->yCount) - 1ul);

            descriptor->u32PDMA_DESCR_NEXT_PTR                      =       (uint32_t)config->descrNext;
            break;
        }
        default:
        {
            /* Unsupported type of descriptor */
            break;
        }
        }

        retVal = CY_PDMA_SUCCESS;
    }
    else
    {
        retVal = CY_PDMA_INVALID_INPUT_PARAMETERS;
    }

    return retVal;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_DeInit
****************************************************************************//**
*
* This function clears all the content of the descriptor structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is being defined in Flash, then the user can't use this
* function to deinitialize it.
*
* \param descriptor
* This is an instance of the descriptor structure to be cleared.
*
*******************************************************************************/
void Cy_PDMA_Descr_DeInit(cy_stc_pdma_descr_t* descriptor)
{
    descriptor->unPDMA_DESCR_CTL.u32Register    = 0ul;
    descriptor->u32PDMA_DESCR_SRC               = 0ul;
    descriptor->u32PDMA_DESCR_DST               = 0ul;
    descriptor->unPDMA_DESCR_X_CTL.u32Register  = 0ul;
    descriptor->unPDMA_DESCR_Y_CTL.u32Register  = 0ul;
    descriptor->u32PDMA_DESCR_NEXT_PTR          = 0ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetTxfrWidth
****************************************************************************//**
*
* This function sets the transfer widths for for the descriptor structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is defined in Flash, then the user must use the
* configuration defines to initialize it.
*
* \param descriptor
* This is a descriptor-structure instance declared by the user. This is the
* descriptor being modified.
*
* \param dataElementSize
* This parameter provides the data element size of the descriptor.
* \ref cy_en_pdma_datasize_t
*
* \param srcTxfrWidth
* This parameter provides the source transfer size parameters of the descriptor.
* \ref cy_en_pdma_txfrsize_t
*
* \param dstTxfrWidth
* This parameter provides the destination transfer size parameters of the
* descriptor.
* \ref cy_en_pdma_txfrsize_t
*
*******************************************************************************/
void Cy_PDMA_Descr_SetTxfrWidth(cy_stc_pdma_descr_t* descriptor,
                                uint32_t dataElementSize,
                                uint32_t srcTxfrWidth,
                                uint32_t dstTxfrWidth)
{
    descriptor->unPDMA_DESCR_CTL.stcField.u2DATA_SIZE           =       dataElementSize;
    descriptor->unPDMA_DESCR_CTL.stcField.u1SRC_TRANSFER_SIZE   =       srcTxfrWidth;
    descriptor->unPDMA_DESCR_CTL.stcField.u1DST_TRANSFER_SIZE   =       dstTxfrWidth;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_Init
****************************************************************************//**
*
* Initializes the PDMA channel with a descriptor and other parameters.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
* \param chnlConfig
* This parameter is a structure that has the initialization information for the
* channel.
*
* \return
* This function returns /ref cy_en_pdma_status_t.
*
*******************************************************************************/
cy_en_pdma_status_t Cy_PDMA_Chnl_Init(volatile stc_DW_t *pstcPDMA, uint32_t chNum, const cy_stc_pdma_chnl_config_t* chnlConfig)
{
    cy_en_pdma_status_t retVal = CY_PDMA_ERR_UNC;

    if ((pstcPDMA != NULL) && (chnlConfig != NULL))
    {
        /* Write this channel's SRAM data to initialize ECC properly */
        pstcPDMA->CH_STRUCT[chNum].unSRAM_DATA0.u32Register             =  0;
        pstcPDMA->CH_STRUCT[chNum].unSRAM_DATA1.u32Register             =  0;

        /* Initialize the X/Y indices (which are undefined after reset) */
        pstcPDMA->CH_STRUCT[chNum].unCH_IDX.u32Register                 =  0;
        
        /* Set current descriptor */
        pstcPDMA->CH_STRUCT[chNum].unCH_CURR_PTR.u32Register            =  (uint32_t)chnlConfig->PDMA_Descriptor;

        /* Set if the channel is preemtable */
        pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u1PREEMPTABLE      =  chnlConfig->preemptable;

        /* Set channel priority */
        pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u2PRIO             =  chnlConfig->priority;

        /* Set enabled status */
        pstcPDMA->CH_STRUCT[chNum].unCH_CTL.stcField.u1ENABLED          =  chnlConfig->enable;

        retVal = CY_PDMA_SUCCESS;
    }
    else
    {
        retVal = CY_PDMA_INVALID_INPUT_PARAMETERS;
    }

    return (retVal);
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_DeInit
****************************************************************************//**
*
* Clears all the content of registers corresponding to the channel.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
void Cy_PDMA_Chnl_DeInit(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unCH_CTL.u32Register            = 0ul;
    pstcPDMA->CH_STRUCT[chNum].unCH_IDX.u32Register            = 0ul;
    pstcPDMA->CH_STRUCT[chNum].unCH_CURR_PTR.u32Register       = 0ul;
    pstcPDMA->CH_STRUCT[chNum].unINTR_MASK.u32Register         = 0ul;
    pstcPDMA->CH_STRUCT[chNum].unINTR_SET.u32Register          = 0ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Enable
****************************************************************************//**
*
* This function enables the PDMA block.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
*******************************************************************************/
void Cy_PDMA_Enable(volatile stc_DW_t *pstcPDMA)
{
    pstcPDMA->unCTL.stcField.u1ENABLED  =       1ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_Disable
****************************************************************************//**
*
* This function disables the PDMA block.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
*******************************************************************************/
void Cy_PDMA_Disable(volatile stc_DW_t *pstcPDMA)
{
    pstcPDMA->unCTL.stcField.u1ENABLED  =       0ul;
}


/*******************************************************************************
* Function Name: Cy_PDMA_GetActiveChnl
****************************************************************************//**
*
* This function used to determine the channels that are active/pending in
* the PDMA block.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \return
* Returns a bit-field with all of the currently active/pending channels in the
* PDMA block.
*
*******************************************************************************/
uint32_t Cy_PDMA_GetActiveChnl(const volatile stc_DW_t *pstcPDMA)
{
    return pstcPDMA->unSTATUS.stcField.u9CH_IDX;
}


/*******************************************************************************
* Function Name: Cy_PDMA_GetActiveSrcAddr
****************************************************************************//**
*
* Returns the source address that is being used for current transfer.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \return
* Returns a pointer to the source of transfer.
*
*******************************************************************************/
void * Cy_PDMA_GetActiveSrcAddr(const volatile stc_DW_t *pstcPDMA)
{
    return ((void *)pstcPDMA->unACT_DESCR_SRC.u32Register);
}


/*******************************************************************************
* Function Name: Cy_PDMA_GetActiveDstAddr
****************************************************************************//**
*
* Returns the destination address that is being used for the current transfer.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \return
* Returns a pointer to the destination of transfer.
*
*******************************************************************************/
void * Cy_PDMA_GetActiveDstAddr(const volatile stc_DW_t *pstcPDMA)
{
    return ((void *)pstcPDMA->unACT_DESCR_DST.u32Register);
}


/*******************************************************************************
* Function Name: Cy_PDMA_Chnl_ClearInterrupt
****************************************************************************//**
*
* This function clears the interrupt status.
*
* \param ptscPDMA
* Pointer to the hardware PDMA block.
*
* \param chNum
* Channel number of the PDMA.
*
*******************************************************************************/
void Cy_PDMA_Chnl_ClearInterrupt(volatile stc_DW_t *pstcPDMA, uint32_t chNum)
{
    pstcPDMA->CH_STRUCT[chNum].unINTR.u32Register = CY_PDMA_INTR_BIT_MASK;
    
    /* Readback of the register is required by hardware. */
    (void) pstcPDMA->CH_STRUCT[chNum].unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_PDMA_Descr_SetNxtDescr
****************************************************************************//**
*
* Sets a next descriptor parameter for the descriptor structure.
* This function will operate in the descriptor only if it was defined as a RAM
* variable.
* If the descriptor is defined in Flash, then the user must use the
* configuration defines to initialize it.
* Based on descriptor type the offset of address for the next descriptor may
* vary. For a single transfer descriptor type, this register is at offset 0x0c.
* For a 1D transfer descriptor type, this register is at offset 0x10. For a 2D
* transfer descriptor type, this register is at offset 0x14.  
*
* \param descriptor
* This is the descriptor-structure instance declared by the user. The function
* modifies the destination-address parameter in this descriptor.
*
* \param nxtDescriptor
* The pointer to the next descriptor.
*
*******************************************************************************/
void Cy_PDMA_Descr_SetNxtDescr(cy_stc_pdma_descr_t* descriptor, const cy_stc_pdma_descr_t* nxtDescriptor)
{
    switch(descriptor->unPDMA_DESCR_CTL.stcField.u2DESCR_TYPE)
    {
        case (uint32_t)CY_PDMA_SINGLE_TRANSFER:
        {
            descriptor->unPDMA_DESCR_X_CTL.u32Register              =       (uint32_t)nxtDescriptor;
            break;
        }
        case (uint32_t)CY_PDMA_1D_TRANSFER:
        case (uint32_t)CY_PDMA_CRC_TRANSFER:
        {
            descriptor->unPDMA_DESCR_Y_CTL.u32Register              =       (uint32_t)nxtDescriptor;
            break;
        }
        case (uint32_t)CY_PDMA_2D_TRANSFER:
        {
            descriptor->u32PDMA_DESCR_NEXT_PTR                      =       (uint32_t)nxtDescriptor;
            break;
        }
        default:
        {
            /* Unsupported type of descriptor */
            break;
        }
    }
}

/*******************************************************************************
* Function Name: Cy_PDMA_CRC_Config
****************************************************************************//**
*******************************************************************************/
void Cy_PDMA_CRC_Config ( volatile stc_DW_t * pstcPDMA,
                          const cy_stc_pdma_crc_config_t*   pstcCrcConfig)
{
    pstcPDMA->unCRC_CTL.stcField.u1DATA_REVERSE     = pstcCrcConfig->data_reverse;
    pstcPDMA->unCRC_CTL.stcField.u1REM_REVERSE      = pstcCrcConfig->rem_reverse;
    pstcPDMA->unCRC_DATA_CTL.stcField.u8DATA_XOR    = pstcCrcConfig->data_xor;
    pstcPDMA->unCRC_LFSR_CTL.stcField.u32LFSR32     = pstcCrcConfig->lfsr32;
    pstcPDMA->unCRC_POL_CTL.stcField.u32POLYNOMIAL  = pstcCrcConfig->polynomial;
    pstcPDMA->unCRC_REM_CTL.stcField.u32REM_XOR     = pstcCrcConfig->rem_xor;
}

/*******************************************************************************
* Function Name: Cy_PDMA_CRC_Config
****************************************************************************//**
*******************************************************************************/
uint32_t Cy_PDMA_GetCrcRemainderResult ( volatile stc_DW_t * pstcPDMA)
{
    return pstcPDMA->unCRC_REM_RESULT.u32Register;
}


#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
