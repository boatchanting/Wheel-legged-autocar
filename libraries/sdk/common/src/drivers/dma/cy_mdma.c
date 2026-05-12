/***************************************************************************//**
* \file cy_mdma.c
* \version 1.0
*
* \brief
* The MDMA driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_mdma.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_Init
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
* This function returns /ref cy_en_mdma_status_t.
*
*******************************************************************************/
cy_en_mdma_status_t Cy_MDMA_Descr_Init(cy_stc_mdma_descr_t* descriptor, const cy_stc_mdma_descr_config_t* config)
{
    cy_en_mdma_status_t retVal = CY_MDMA_ERR_UNC;

    if ((descriptor != NULL) && (config != NULL))
    {
        /* Descriptor[0] */
        descriptor->unMDMA_DESCR_CTL.stcField.u2WAIT_FOR_DEACT     =       config->deact;
        descriptor->unMDMA_DESCR_CTL.stcField.u2INTR_TYPE          =       config->intrType;
        descriptor->unMDMA_DESCR_CTL.stcField.u2TR_OUT_TYPE        =       config->trigoutType;
        descriptor->unMDMA_DESCR_CTL.stcField.u2TR_IN_TYPE         =       config->triginType;
        descriptor->unMDMA_DESCR_CTL.stcField.u1DATA_PREFETCH      =       config->dataPrefetch;
        descriptor->unMDMA_DESCR_CTL.stcField.u2DATA_SIZE          =       config->dataSize;
        descriptor->unMDMA_DESCR_CTL.stcField.u1CH_DISABLE         =       config->chStateAtCmplt;
        descriptor->unMDMA_DESCR_CTL.stcField.u1SRC_TRANSFER_SIZE  =       config->srcTxfrSize;
        descriptor->unMDMA_DESCR_CTL.stcField.u1DST_TRANSFER_SIZE  =       config->destTxfrSize;
        descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE         =       config->descrType;

        /* Descriptor[1] */
        descriptor->u32MDMA_DESCR_SRC                              =       (uint32_t)config->srcAddr;

        /* after 3rd word of descriptor depends on descriptor type */

        switch(config->descrType)
        {
        case (uint32_t)CY_MDMA_SINGLE_TRANSFER:
        {
            /* Descriptor[2] */
            descriptor->u32MDMA_DESCR_DST                           =       (uint32_t)config->destAddr;

            /* Descriptor[3] -> NEXT_PTR */
            descriptor->unMDMA_DESCR_X_SIZE.u32Register             =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_MDMA_1D_TRANSFER:
        {
            /* Descriptor[2] */
            descriptor->u32MDMA_DESCR_DST                           =       (uint32_t)config->destAddr;

            /* Descriptor[3] */
            descriptor->unMDMA_DESCR_X_SIZE.stcField.u16X_COUNT     =       (uint32_t)((config->xCount) - 1ul);

            /* Descriptor[4] */
            descriptor->unMDMA_DESCR_X_INCR.stcField.u16SRC_X_INCR  =       (uint32_t)config->srcXincr;
            descriptor->unMDMA_DESCR_X_INCR.stcField.u16DST_X_INCR  =       (uint32_t)config->destXincr;

            /* Descriptor[5] -> NEXT_PTR */
            descriptor->unMDMA_DESCR_Y_SIZE.u32Register             =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_MDMA_2D_TRANSFER:
        {
            /* Descriptor[2] */
            descriptor->u32MDMA_DESCR_DST                           =       (uint32_t)config->destAddr;

            /* Descriptor[3] */
            descriptor->unMDMA_DESCR_X_SIZE.stcField.u16X_COUNT     =       (uint32_t)((config->xCount) - 1ul);

            /* Descriptor[4] */
            descriptor->unMDMA_DESCR_X_INCR.stcField.u16SRC_X_INCR  =       (uint32_t)config->srcXincr;
            descriptor->unMDMA_DESCR_X_INCR.stcField.u16DST_X_INCR  =       (uint32_t)config->destXincr;

            /* Descriptor[5] */
            descriptor->unMDMA_DESCR_Y_SIZE.stcField.u16Y_COUNT     =       (uint32_t)((config->yCount) - 1ul);

            /* Descriptor[6] */
            descriptor->unMDMA_DESCR_Y_INCR.stcField.u16SRC_Y_INCR  =       (uint32_t)config->srcYincr;
            descriptor->unMDMA_DESCR_Y_INCR.stcField.u16DST_Y_INCR  =       (uint32_t)config->destYincr;

            /* Descriptor[7] */
            descriptor->u32MDMA_DESCR_NEXT_PTR                      =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_MDMA_MEMORY_COPY_TRANSFER:
        {
            /* Descriptor[2] */
            descriptor->u32MDMA_DESCR_DST                           =       (uint32_t)config->destAddr;

            /* Descriptor[3] */
            descriptor->unMDMA_DESCR_X_SIZE.stcField.u16X_COUNT     =       (uint32_t)((config->xCount) - 1ul);

            /* Descriptor[4] -> NEXT_PTR */
            descriptor->unMDMA_DESCR_X_INCR.u32Register             =       (uint32_t)config->descrNext;
            break;
        }
        case (uint32_t)CY_MDMA_SCATTER_TRANSFER:
        {
            /* Descriptor[2] -> X_SIZE */
            descriptor->u32MDMA_DESCR_DST                           =       (uint32_t)((config->xCount) - 1ul);

            /* Descriptor[3] -> NEXT_PTR */
            descriptor->unMDMA_DESCR_X_SIZE.u32Register             =       (uint32_t)config->descrNext;
            break;
        }
        default:
        {
            /* Unsupported type of descriptor */
            break;
        }
        }

        retVal = CY_MDMA_SUCCESS;
    }
    else
    {
        retVal = CY_MDMA_INVALID_INPUT_PARAMETERS;
    }

    return retVal;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_DeInit
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
void Cy_MDMA_Descr_DeInit(cy_stc_mdma_descr_t* descriptor)
{
    descriptor->unMDMA_DESCR_CTL.u32Register    = 0ul;
    descriptor->u32MDMA_DESCR_SRC               = 0ul;
    descriptor->u32MDMA_DESCR_DST               = 0ul;
    descriptor->unMDMA_DESCR_X_SIZE.u32Register = 0ul;
    descriptor->unMDMA_DESCR_X_INCR.u32Register = 0ul;
    descriptor->unMDMA_DESCR_Y_SIZE.u32Register = 0ul;
    descriptor->unMDMA_DESCR_Y_INCR.u32Register = 0ul;
    descriptor->u32MDMA_DESCR_NEXT_PTR          = 0ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetTxfrWidth
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
* \ref cy_en_mdma_datasize_t
*
* \param srcTxfrWidth
* This parameter provides the source transfer size parameters of the descriptor.
* \ref cy_en_mdma_txfrsize_t
*
* \param dstTxfrWidth
* This parameter provides the destination transfer size parameters of the
* descriptor.
* \ref cy_en_mdma_txfrsize_t
*
*******************************************************************************/
void Cy_MDMA_Descr_SetTxfrWidth(cy_stc_mdma_descr_t* descriptor,
                            uint32_t dataElementSize,
                            uint32_t srcTxfrWidth,
                            uint32_t dstTxfrWidth)
{
    descriptor->unMDMA_DESCR_CTL.stcField.u2DATA_SIZE           =       dataElementSize;
    descriptor->unMDMA_DESCR_CTL.stcField.u1SRC_TRANSFER_SIZE   =       srcTxfrWidth;
    descriptor->unMDMA_DESCR_CTL.stcField.u1DST_TRANSFER_SIZE   =       dstTxfrWidth;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_Init
****************************************************************************//**
*
* Initializes the MDMA channel with a descriptor and other parameters.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
* \param chnlConfig
* This parameter is a structure that has the initialization information for the
* channel.
*
* \return
* This function returns /ref cy_en_mdma_status_t.
*
*******************************************************************************/
cy_en_mdma_status_t Cy_MDMA_Chnl_Init(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum, const cy_stc_mdma_chnl_config_t* chnlConfig)
{
    cy_en_mdma_status_t retVal = CY_MDMA_ERR_UNC;

    if ((pstcMDMA != NULL) && (chnlConfig != NULL))
    {
        /* Initialize the X/Y indices (which are undefined after reset) */
        pstcMDMA->CH[chNum].unIDX.u32Register    =  0;

        /* Set current descriptor */
        pstcMDMA->CH[chNum].unCURR.u32Register   =  (uint32_t)chnlConfig->MDMA_Descriptor;

        /* Set if the channel is preemptable  */
        /* There is no the parameter in MDMA */

        /* Set channel priority */
        pstcMDMA->CH[chNum].unCTL.stcField.u2PRIO    =   chnlConfig->priority;

        /* Set enabled status */
        pstcMDMA->CH[chNum].unCTL.stcField.u1ENABLED =   chnlConfig->enable;

        retVal = CY_MDMA_SUCCESS;
    }
    else
    {
        retVal = CY_MDMA_INVALID_INPUT_PARAMETERS;
    }

    return (retVal);
}


/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_DeInit
****************************************************************************//**
*
* Clears all the content of registers corresponding to the channel.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
*******************************************************************************/
void Cy_MDMA_Chnl_DeInit(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    pstcMDMA->CH[chNum].unCTL.u32Register               = 0ul;
    pstcMDMA->CH[chNum].unIDX.u32Register               = 0ul;
    pstcMDMA->CH[chNum].unCURR.u32Register              = 0ul;
    pstcMDMA->CH[chNum].unINTR_MASK.u32Register         = 0ul;
    pstcMDMA->CH[chNum].unINTR_SET.u32Register          = 0ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Enable
****************************************************************************//**
*
* This function enables the MDMA block.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
*******************************************************************************/
void Cy_MDMA_Enable(volatile stc_DMAC_t *pstcMDMA)
{
    pstcMDMA->unCTL.stcField.u1ENABLED  =       1ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Disable
****************************************************************************//**
*
* This function disables the MDMA block.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
*******************************************************************************/
void Cy_MDMA_Disable(volatile stc_DMAC_t *pstcMDMA)
{
    pstcMDMA->unCTL.stcField.u1ENABLED  =       0ul;
}


/*******************************************************************************
* Function Name: Cy_MDMA_GetActiveChnl
****************************************************************************//**
*
* This function used to determine the channels that are active/pending in
* the MDMA block.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
* \return
* Returns a bit-field with all of the currently active/pending channels in the
* MDMA block.
*
*******************************************************************************/
uint32_t Cy_MDMA_GetActiveChnl(const volatile stc_DMAC_t *pstcMDMA)
{
    return pstcMDMA->unACTIVE.u32Register;
}

/*******************************************************************************
* Function Name: Cy_MDMA_Chnl_ClearInterrupt
****************************************************************************//**
*
* This function clears the interrupt status.
*
* \param ptscMDMA
* Pointer to the hardware MDMA block.
*
* \param chNum
* Channel number of the MDMA.
*
*******************************************************************************/
void Cy_MDMA_Chnl_ClearInterrupt(volatile stc_DMAC_t *pstcMDMA, uint32_t chNum)
{
    pstcMDMA->CH[chNum].unINTR.u32Register = CY_MDMA_INTR_BIT_MASK;
    
    /* Readback of the register is required by hardware. */
    (void) pstcMDMA->CH[chNum].unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_MDMA_Descr_SetNxtDescr
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
void Cy_MDMA_Descr_SetNxtDescr(cy_stc_mdma_descr_t* descriptor, const cy_stc_mdma_descr_t* nxtDescriptor)
{
    if (descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE == (uint32_t)CY_MDMA_SINGLE_TRANSFER)
    {
        descriptor->unMDMA_DESCR_X_SIZE.u32Register      = (uint32_t)nxtDescriptor;
    }
    else if (descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE == (uint32_t)CY_MDMA_1D_TRANSFER)
    {
        descriptor->unMDMA_DESCR_Y_SIZE.u32Register      = (uint32_t)nxtDescriptor;
    }
    else if (descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE == (uint32_t)CY_MDMA_2D_TRANSFER)
    {
        descriptor->u32MDMA_DESCR_NEXT_PTR               = (uint32_t)nxtDescriptor;
    }
    else if (descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE == (uint32_t)CY_MDMA_MEMORY_COPY_TRANSFER)
    {
        descriptor->unMDMA_DESCR_X_INCR.u32Register      = (uint32_t)nxtDescriptor;
    }
    else if (descriptor->unMDMA_DESCR_CTL.stcField.u3DESCR_TYPE == (uint32_t)CY_MDMA_SCATTER_TRANSFER)
    {
        descriptor->unMDMA_DESCR_X_SIZE.u32Register      = (uint32_t)nxtDescriptor;
    }
    else
    {
        /* Unsupported type of descriptor */
    }
}

#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
