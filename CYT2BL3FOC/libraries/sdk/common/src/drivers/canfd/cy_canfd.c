/***************************************************************************//**
* \file cy_canfd.c
* \version 1.0
*
* \brief
* Provides an API implementation of the CAN FD driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_canfd.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define SIZE_OF_SID_FILTER_IN_WORD      1u
#define SIZE_OF_EXTID_FILTER_IN_WORD    2u
#define SIZE_OF_TXEVENT_FIFO_IN_WORD    2u

/** Macro to return the number of enabled CAN-FD channels */
#define CY_CANFD_CHANNEL_COUNT (uint32_t)(sizeof(m_astcCanFDChannelDataLut) / sizeof(m_astcCanFDChannelDataLut[0]))

/***************************************
*       Configuration Structures
***************************************/

/**
 *****************************************************************************
 ** \brief CAN FD message RAM configuration
 ** 
 ** This structure is used to get the CAN FD message RAM offset and size for 
 ** CAN FD channel. 
 *****************************************************************************/
typedef struct
{
    uint16_t offset;     /*!< offset of MRAM for the CAN channel within a CAN instance. */
    uint32_t size;       /*!< MRAM size for the channel. */
} cy_stc_canfd_msgram_config_t;

/**
 *****************************************************************************
 ** \brief CAN-FD channel data type 
 *****************************************************************************/
 typedef struct
{
    cy_pstc_canfd_type_t       channel;    /* pointer to registers of a channel */
    cy_stc_canfd_context_t     context;    /* module internal context of channel */
} cy_stc_canfd_channel_data_t;

/**
 ***********************************************************************************
 ** \brief Look-up table for all enabled CAN FD channels and their internal context
 ***********************************************************************************/
static cy_stc_canfd_channel_data_t m_astcCanFDChannelDataLut[] =
{
    { 
        CY_CANFD0_0_TYPE,   /* channel */
        { {0} }             /* context */
    },
    { 
        CY_CANFD0_1_TYPE,   /* channel */
        { {0} }             /* context */
    },
#if defined (tviibe1m) || defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m) || defined (tviibh4m) || defined (tviibh16m)
    { 
        CY_CANFD0_2_TYPE,   /* channel */
        { {0} }             /* context */
    },
#endif
#if defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m) || defined (tviibh4m) || defined (tviibh16m)
    { 
        CY_CANFD0_3_TYPE,   /* channel */
        { {0} }             /* context */
    },
#if defined (tviibh8m) || defined (tviibh16m)
    { 
        CY_CANFD0_4_TYPE,   /* channel */
        { {0} }             /* context */
    },
#endif
#endif
    { 
        CY_CANFD1_0_TYPE,   /* channel */
        { {0} }             /* context */
    },
    { 
        CY_CANFD1_1_TYPE,   /* channel */
        { {0} }             /* context */
    },
#if defined (tviibe1m) || defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m) || defined (tviibh4m) || defined (tviibh16m)
    { 
        CY_CANFD1_2_TYPE,   /* channel */
        { {0} }             /* context */
    },
#endif
#if defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m) || defined (tviibh4m) || defined (tviibh16m)
    { 
        CY_CANFD1_3_TYPE,   /* channel */
        { {0} }             /* context */
    },
#if defined (tviibh8m) || defined (tviibh16m)
    { 
        CY_CANFD1_4_TYPE,   /* channel */
        { {0} }             /* context */
    },
#endif
#endif
};

/**
 ***********************************************************************************
 ** \brief Error Log area 
 ***********************************************************************************/
static volatile uint16_t  m_au16LastErrorCode[CY_CANFD_CHANNEL_COUNT];

/*****************************************************************************
 ** \brief DLC-Word conversion
 *****************************************************************************/
static const uint8_t dataBufferSizeInWord[] = {
	2u,			/*!<  8 bytes */
	3u,			/*!< 12 bytes */
	4u,			/*!< 16 bytes */
	5u,			/*!< 20 bytes */
	6u,			/*!< 24 bytes */
	8u,			/*!< 32 bytes */
	12u,		/*!< 48 bytes */
	16u			/*!< 64 bytes */
};

/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/
static cy_en_canfd_status_t Cy_CANFD_GetMsgramOffset(cy_pstc_canfd_type_t pstcCanFD, cy_stc_canfd_msgram_config_t *pstcMsgramConfig);
static cy_stc_canfd_context_t* Cy_CANFD_GetContext(cy_pstc_canfd_type_t pstcCanFD);
static uint32_t* Cy_CANFD_CalcRxBufAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf);
static uint32_t* Cy_CANFD_CalcTxBufAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf);
static uint32_t* Cy_CANFD_CalcRxFifoAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8FifoNumber, uint32_t u32GetIndex);
static uint8_t Cy_CANFD_CalculateChannelIdx(cy_pstc_canfd_type_t pstcCanFD);
static void Cy_CANFD_ErrorHandling(cy_pstc_canfd_type_t pstcCanFD);

/*******************************************************************************
* Function Name: Cy_CANFD_GetMsgramOffset
****************************************************************************//**
*
* \brief Get the CAN FD channel MRAM offset and size.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param pstcMsgramConfig
* Pointer to a structure inside which the MRAM offset and size will be returned for the CAN FD channel.
*
* \return CY_CANFD_SUCCESS      
* \return CY_CANFD_BAD_PARAM    If one of the following conditions are met:
*                               - pstcCanFD == NULL
*                               - pstcConfig == NULL
*******************************************************************************/
static cy_en_canfd_status_t Cy_CANFD_GetMsgramOffset(cy_pstc_canfd_type_t pstcCanFD, cy_stc_canfd_msgram_config_t *pstcMsgramConfig) 
{
    /* Check for NULL pointers */
    if( pstcCanFD == NULL ||
        pstcMsgramConfig == NULL
      )
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    if (pstcCanFD == CY_CANFD0_0_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD0_0_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD0_0_MSGRAM_SIZE;
    }
    else if (pstcCanFD == CY_CANFD0_1_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD0_1_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD0_1_MSGRAM_SIZE;
    }
#if defined (tviibe1m) || defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m)|| defined (tviibh4m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD0_2_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD0_2_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD0_2_MSGRAM_SIZE;
    }
#endif
#if defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m)|| defined (tviibh4m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD0_3_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD0_3_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD0_3_MSGRAM_SIZE;
    }
#endif
#if defined (tviibh8m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD0_4_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD0_4_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD0_4_MSGRAM_SIZE;
    }
#endif
    else if (pstcCanFD == CY_CANFD1_0_TYPE)
    {        
        pstcMsgramConfig->offset = CY_CANFD1_0_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD1_0_MSGRAM_SIZE;
    }
    else if (pstcCanFD == CY_CANFD1_1_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD1_1_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD1_1_MSGRAM_SIZE;
    }
#if defined (tviibe1m) || defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m)|| defined (tviibh4m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD1_2_TYPE) 
    {   
        pstcMsgramConfig->offset = CY_CANFD1_2_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD1_2_MSGRAM_SIZE;
    }
#endif
#if defined (tviibh8m) || defined (tviibe2m) || defined (tviibe4m)|| defined (tviibh4m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD1_3_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD1_3_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD1_3_MSGRAM_SIZE;
    }
#endif
#if defined (tviibh8m) || defined (tviibh16m)
    else if (pstcCanFD == CY_CANFD1_4_TYPE)
    {
        pstcMsgramConfig->offset = CY_CANFD1_4_MSGRAM_OFFSET;
        pstcMsgramConfig->size   = CY_CANFD1_4_MSGRAM_SIZE;
    }
#endif
    else
    {
        return CY_CANFD_BAD_PARAM;
    }
    return CY_CANFD_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_CANFD_GetContext
****************************************************************************//**
*
* \brief Return the internal data for a certain CAN FD channel.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \return Pointer to internal context or NULL if channel is not known
*
*******************************************************************************/
static cy_stc_canfd_context_t* Cy_CANFD_GetContext(cy_pstc_canfd_type_t pstcCanFD) 
{
    uint32_t u32Channel;
   
    for (u32Channel = 0; u32Channel < CY_CANFD_CHANNEL_COUNT; u32Channel++)
    {
        if (pstcCanFD == m_astcCanFDChannelDataLut[u32Channel].channel)
        {
            return &m_astcCanFDChannelDataLut[u32Channel].context;
        }
    }

    return NULL;
}

/*******************************************************************************
* Function Name: Cy_CANFD_CalcRxBufAdrs
****************************************************************************//**
*
* \brief Message buffer address calculation for reception.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param u8MsgBuf
* Message buffer index for reception.(0-63)
*
* \return pu32Adrs      Message buffer address corresponding to the index.
*                       Can be NULL if the index is invalid.
*
*******************************************************************************/
static uint32_t* Cy_CANFD_CalcRxBufAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf)
{
    uint32_t* pu32Adrs;

    if (u8MsgBuf > 63)
    {
        /* Set 0 to the return value if the index is invalid */
        pu32Adrs = NULL;
    }
    else
    {
        /* Set the message buffer address to the return value if the index is available */ 
        pu32Adrs =  (uint32_t*)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + (uint32_t)CY_CANFD_MSGRAM_START);
        pu32Adrs += pstcCanFD->M_TTCAN.unRXBC.stcField.u14RBSA;
        pu32Adrs += u8MsgBuf * (2 + dataBufferSizeInWord[pstcCanFD->M_TTCAN.unRXESC.stcField.u3RBDS]);
    }
    return pu32Adrs;
}

/*******************************************************************************
* Function Name: Cy_CANFD_CalcTxBufAdrs
****************************************************************************//**
*
* \brief Message buffer address calculation for transmission.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param u8MsgBuf
* Message buffer index for transmission.(0-31)
*
* \return pu32Adrs      Message buffer address corresponding to the index.
*                       Can be NULL if the index is invalid.
*
*******************************************************************************/
static uint32_t* Cy_CANFD_CalcTxBufAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf)
{
    uint32_t* pu32Adrs;

    if ( u8MsgBuf > 31)
    {
        /* Set 0 to the return value if the index is invalid */
        pu32Adrs = NULL;
    }
    else
    {
        /* Set the message buffer address to the return value if the index is available */
        pu32Adrs = (uint32_t*)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + (uint32_t)CY_CANFD_MSGRAM_START);
        pu32Adrs += pstcCanFD->M_TTCAN.unTXBC.stcField.u14TBSA;
        pu32Adrs += u8MsgBuf * (2 + dataBufferSizeInWord[pstcCanFD->M_TTCAN.unTXESC.stcField.u3TBDS]);
    }
    return pu32Adrs;
}

/**
 *****************************************************************************
 ** \brief Message buffer address calculation for reception.
 **
 ** \param [in] pstcCanFD           Pointer to register area of a CAN-FD unit.
 ** \param [in] u8FifoNumber        FIFO number (0 or 1)
 ** \param [in] u8MsgBuf            Message buffer index for reception.(0-63)

 ** \retval pu32Adrs                Message buffer address corresponding to the index.
 **                                 Can be 0 if the index is invalid.
 *****************************************************************************/
static uint32_t* Cy_CANFD_CalcRxFifoAdrs(cy_pstc_canfd_type_t pstcCanFD, uint8_t u8FifoNumber, uint32_t u32GetIndex)
{
    uint32_t* pu32Adrs;

    if(u8FifoNumber > 1)
    {
        /* Set 0 to the return value if the FIFO number is invalid */
        pu32Adrs = NULL;
    }
    else
    {
        /* Set the message buffer address to the return value if the index is available */
        pu32Adrs = (uint32_t*)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + (uint32_t)CY_CANFD_MSGRAM_START);
        pu32Adrs += (u8FifoNumber == 0) ? pstcCanFD->M_TTCAN.unRXF0C.stcField.u14F0SA : pstcCanFD->M_TTCAN.unRXF1C.stcField.u14F1SA;
        pu32Adrs += u32GetIndex * (2 + dataBufferSizeInWord[(u8FifoNumber == 0) ? pstcCanFD->M_TTCAN.unRXESC.stcField.u3F0DS : pstcCanFD->M_TTCAN.unRXESC.stcField.u3F1DS]);
    }
    
    return pu32Adrs;
}

/*******************************************************************************
* Function Name: Cy_CANFD_CalculateChannelIdx
****************************************************************************//**
*
* \brief Calculates the unique index for each channel for internal purpose.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \return uint8_t      0 if pstcCanFD == CY_CANFD0_0_TYPE
*                      1 if pstcCanFD == CY_CANFD0_1_TYPE
*                      2 if pstcCanFD == CY_CANFD0_2_TYPE
*                      3 if pstcCanFD == CY_CANFD0_3_TYPE
*                      4 if pstcCanFD == CY_CANFD0_4_TYPE
*                      5 if pstcCanFD == CY_CANFD1_0_TYPE
*                      6 if pstcCanFD == CY_CANFD1_1_TYPE
*                      7 if pstcCanFD == CY_CANFD1_2_TYPE
*                      8 if pstcCanFD == CY_CANFD1_3_TYPE
*                      9 if pstcCanFD == CY_CANFD1_4_TYPE
*
*******************************************************************************/
static uint8_t Cy_CANFD_CalculateChannelIdx(cy_pstc_canfd_type_t pstcCanFD)
{
#if defined (tviibe512k)
    uint8_t u8Idx = (((uint32_t)pstcCanFD > (uint32_t)CY_CANFD0_1_TYPE) ? 2 : 0) + ((uint32_t)pstcCanFD & 0xFFFFul) / 0x200;
#endif
#if defined (tviibe1m)
    uint8_t u8Idx = (((uint32_t)pstcCanFD > (uint32_t)CY_CANFD0_2_TYPE) ? 3 : 0) + ((uint32_t)pstcCanFD & 0xFFFFul) / 0x200;
#endif
#if defined (tviibe2m) || defined (tviibe4m) || defined (tviibh4m)
    uint8_t u8Idx = (((uint32_t)pstcCanFD > (uint32_t)CY_CANFD0_3_TYPE) ? 4 : 0) + ((uint32_t)pstcCanFD & 0xFFFFul) / 0x200;
#endif
#if defined (tviibh8m) || defined (tviibh16m)
    uint8_t u8Idx = (((uint32_t)pstcCanFD > (uint32_t)CY_CANFD0_4_TYPE) ? 5 : 0) + ((uint32_t)pstcCanFD & 0xFFFFul) / 0x200;
#endif
#if defined (tviic2d4m) || defined (tviic2d6m) || defined (tviic2d6mddr) || defined (tviice4m)
    uint8_t u8Idx = (((uint32_t)pstcCanFD > (uint32_t)CY_CANFD0_1_TYPE) ? 2 : 0) + ((uint32_t)pstcCanFD & 0xFFFFul) / 0x200;
#endif
    return u8Idx;
}

/**
 ******************************************************************************
 ** \brief Set error code to log area.
 **
 ** \param [in] pstcCanFD           Pointer to register area of a CAN FD channel.
 **
 ******************************************************************************/
static void Cy_CANFD_ErrorHandling(cy_pstc_canfd_type_t pstcCanFD)
{
    uint8_t u8Idx = 0;
    
    if(pstcCanFD == NULL)
    {
        return;
    }
    
    u8Idx = Cy_CANFD_CalculateChannelIdx(pstcCanFD);
    
    /* Access to Reserved Address */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1ARA == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0001;
    }

    /* Protocol Error in Data Phase */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1PED == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0002;
    }

    /* Protocol Error in Arbitration Phase */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1PEA == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0003;
    }

    /* Watchdog Interrupt */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1WDI == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0004;
    }

    /* Bus_Off Status Interrupt */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1BO_ == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0005;
    }

    /* Bit Error Uncorrected Interrupt */
    if (pstcCanFD->M_TTCAN.unIR.stcField.u1BEU == 1)
    {
        m_au16LastErrorCode[u8Idx] = 0x0006;
    }

    return;

} /* ErrorHandling */

/**
 ******************************************************************************
 ** \brief Extract the message content from RX Buffer.
 **
 ** \param [in] pstcRxBufferAddr        Rx Buffer address in CAN Message RAM.
 **
 ** \param [out] pstcCanFDmsg           Pointer to the structure containing the
 **                                     CAN Message extracted form Rx Buffer
 **
 ******************************************************************************/
void Cy_CANFD_ExtractMsgFromRXBuffer(cy_stc_canfd_rx_buffer_t *pstcRxBufferAddr, cy_stc_canfd_msg_t *pstcCanFDmsg)
{
    uint16_t     u16Count = 0;
    uint16_t     u16DlcTemp = 0;
    
    if(0 == pstcRxBufferAddr)
    {
        return;
    }
    
    /* Save received data */
    /* XTD : Extended Identifier */
    pstcCanFDmsg->idConfig.extended = pstcRxBufferAddr->r0_f.xtd;

    /* ID : RxID */
    if (pstcCanFDmsg->idConfig.extended == false)
    {
        pstcCanFDmsg->idConfig.identifier = pstcRxBufferAddr->r0_f.id >> 18;
    }
    else
    {
        pstcCanFDmsg->idConfig.identifier = pstcRxBufferAddr->r0_f.id;
    }

    /* FDF : Extended Data Length */
    pstcCanFDmsg->canFDFormat = pstcRxBufferAddr->r1_f.fdf;

    /* DLC : Data Length Code */
    pstcCanFDmsg->dataConfig.dataLengthCode = pstcRxBufferAddr->r1_f.dlc;

    /* Copy 0-64 byte of data area */
    if (pstcCanFDmsg->dataConfig.dataLengthCode < 8)
    {
        u16DlcTemp = 0;
    }
    else
    {
        u16DlcTemp = pstcCanFDmsg->dataConfig.dataLengthCode - 8;
    } 

    for (u16Count = 0; u16Count < dataBufferSizeInWord[u16DlcTemp]; u16Count++)
    {
        pstcCanFDmsg->dataConfig.data[u16Count] = pstcRxBufferAddr->data_area_f[u16Count];
    }
}  /* Cy_CANFD_ExtractMsgFromRXBuffer*/

/*******************************************************************************
* Function Name: Cy_CANFD_IrqHandler
****************************************************************************//**
*
* \brief CAN-FD (Status/Error/Rx/Tx) interrupt ISR.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
*******************************************************************************/
void Cy_CANFD_IrqHandler( cy_pstc_canfd_type_t pstcCanFD )
{
    uint32_t*                       pu32Adrs = 0;
    uint8_t                         u8MessageBufferNumber ;
    cy_stc_canfd_msg_t              stcCanFDmsg;
    volatile stc_CANFD_CH_M_TTCAN_t*  pstcCanFDChMTTCAN;
    cy_stc_canfd_context_t*         pstcCanFDContext;
    uint8_t                         u8BufferSizeTemp = 0;
    uint32_t                        au32RxBuf[18];
    
    /* Get pointer to internal data structure */
    pstcCanFDContext = Cy_CANFD_GetContext(pstcCanFD);

    /* Check for NULL */
    if (pstcCanFDContext == NULL)
    {
        return;
    }

    /* Get the pointer to M_TTCAN of the CAN FD channel */
    pstcCanFDChMTTCAN = &pstcCanFD->M_TTCAN;

    /* Other than Tx/Rx interrupt occured */
    if (pstcCanFDChMTTCAN->unIR.u32Register & 0x3ff7E0EE)
    {
        Cy_CANFD_ErrorHandling(pstcCanFD);
    }
    
    if(pstcCanFDChMTTCAN->unIR.stcField.u1DRX == 1)     // At least one received message stored into an Rx Buffer.
    {
        /* Clear the Message stored to Dedicated Rx Buffer flag */
        pstcCanFDChMTTCAN->unIR.stcField.u1DRX = 1UL;
        
        if(pstcCanFDChMTTCAN->unNDAT1.u32Register != 0)       // Message buffers 0-31
        {
            for(u8MessageBufferNumber = 0; u8MessageBufferNumber < 32; u8MessageBufferNumber++)
            {
                if((pstcCanFDChMTTCAN->unNDAT1.u32Register & (1ul << u8MessageBufferNumber)) != 0)
                {
                    // Calculate Rx Buffer address
                    pu32Adrs = Cy_CANFD_CalcRxBufAdrs(pstcCanFD, u8MessageBufferNumber);
                    
                    // Clear NDAT1 register
                    pstcCanFDChMTTCAN->unNDAT1.u32Register = (1ul << u8MessageBufferNumber);
                    
                    break;
                }
            }
        }
        else if(pstcCanFDChMTTCAN->unNDAT2.u32Register != 0)  // Message buffers 32-63
        {
            for(u8MessageBufferNumber = 0; u8MessageBufferNumber < 32; u8MessageBufferNumber++)
            {
                if((pstcCanFDChMTTCAN->unNDAT2.u32Register & (1ul << u8MessageBufferNumber)) != 0)
                {
                    u8MessageBufferNumber += 32;
                    
                    // Calculate Rx Buffer address
                    pu32Adrs = Cy_CANFD_CalcRxBufAdrs(pstcCanFD, u8MessageBufferNumber);
                    
                    // Clear NDAT2 register
                    pstcCanFDChMTTCAN->unNDAT2.u32Register = (1ul << (u8MessageBufferNumber - 32));
                    
                    break;
                }
            }
        }
        
        if(pu32Adrs)
        {
            Cy_CANFD_ExtractMsgFromRXBuffer((cy_stc_canfd_rx_buffer_t *) pu32Adrs, &stcCanFDmsg);

            /* CAN-FD message received, check if there is a callback function */
            /* Call callback function if it was set previously. */
            if (pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction != NULL)
            {
                pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction(false, u8MessageBufferNumber, &stcCanFDmsg);
            }
        }
    }
    else if(pstcCanFDChMTTCAN->unIR.stcField.u1RF0N == 1)   // New message stored into RxFIFO 0
    {
        pstcCanFDChMTTCAN->unIR.stcField.u1RF0N = 1;        // Clear the new message interrupt flag
        
        while(pstcCanFDChMTTCAN->unRXF0S.stcField.u7F0FL > 0)
        {
            if(pstcCanFD->unRXFTOP_CTL.stcField.u1F0TPE == 1)   // RxFifo Top pointer logic is used
            {           
                u8BufferSizeTemp = 2 + dataBufferSizeInWord[pstcCanFD->M_TTCAN.unRXESC.stcField.u3F0DS];
                
                // Now read the RX FIFO Top Data register to copy the content of received message
                for(uint8_t u8LoopVar = 0u; u8LoopVar < u8BufferSizeTemp; u8LoopVar++)
                {
                    au32RxBuf[u8LoopVar] = pstcCanFD->unRXFTOP0_DATA.u32Register;
                }
                
                /* CAN-FD message received, check if there is a callback function */
                /* Call callback function if it was set previously. */
                if (pstcCanFDContext->canFDInterruptHandling.canFDRxWithTopPtrInterruptFunction != NULL)
                {
                    pstcCanFDContext->canFDInterruptHandling.canFDRxWithTopPtrInterruptFunction(CY_CANFD_RX_FIFO0, u8BufferSizeTemp, &au32RxBuf[0]);
                }
            }
            else        // RxFifo Top pointer logic is not used
            {
                un_CANFD_CH_RXF0S_t unRXF0S;
                unRXF0S.u32Register = pstcCanFDChMTTCAN->unRXF0S.u32Register;
                
                pu32Adrs = Cy_CANFD_CalcRxFifoAdrs(pstcCanFD, CY_CANFD_RX_FIFO0, unRXF0S.stcField.u6F0GI);
                
                if(pu32Adrs)
                {
                    // Extract the received message from Buffer
                    Cy_CANFD_ExtractMsgFromRXBuffer((cy_stc_canfd_rx_buffer_t *) pu32Adrs, &stcCanFDmsg);
                
                    // Acknowledge the FIFO message
                    pstcCanFDChMTTCAN->unRXF0A.stcField.u6F0AI = unRXF0S.stcField.u6F0GI;

                    /* CAN-FD message received, check if there is a callback function */
                    /* Call callback function if it was set previously. */
                    if (pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction != NULL)
                    {
                        pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction(true, CY_CANFD_RX_FIFO0, &stcCanFDmsg);
                    }
                }
            }
        }
    }
    else if(pstcCanFDChMTTCAN->unIR.stcField.u1RF1N == 1)   // New message stored into RxFIFO 1
    {
        pstcCanFDChMTTCAN->unIR.stcField.u1RF1N = 1;        // Clear the new message interrupt flag
        
        while(pstcCanFDChMTTCAN->unRXF1S.stcField.u7F1FL > 0)
        {
            if(pstcCanFD->unRXFTOP_CTL.stcField.u1F1TPE == 1)   // RxFifo Top pointer logic is used
            {           
                u8BufferSizeTemp = 2 + dataBufferSizeInWord[pstcCanFD->M_TTCAN.unRXESC.stcField.u3F1DS];
                
                // Now read the RX FIFO Top Data register to copy the content of last received message
                for(uint8_t u8LoopVar = 0u; u8LoopVar < u8BufferSizeTemp; u8LoopVar++)
                {
                    au32RxBuf[u8LoopVar] = pstcCanFD->unRXFTOP1_DATA.u32Register;
                }
                
                /* CAN-FD message received, check if there is a callback function */
                /* Call callback function if it was set previously. */
                if (pstcCanFDContext->canFDInterruptHandling.canFDRxWithTopPtrInterruptFunction != NULL)
                {
                    pstcCanFDContext->canFDInterruptHandling.canFDRxWithTopPtrInterruptFunction(CY_CANFD_RX_FIFO1, u8BufferSizeTemp, &au32RxBuf[0]);
                }
            }
            else        // RxFifo Top pointer logic is not used
            {
                un_CANFD_CH_RXF1S_t unRXF1S;
                unRXF1S.u32Register = pstcCanFDChMTTCAN->unRXF1S.u32Register;
                
                pu32Adrs = Cy_CANFD_CalcRxFifoAdrs(pstcCanFD, CY_CANFD_RX_FIFO1, unRXF1S.stcField.u6F1GI);
                
                if(pu32Adrs)
                {
                    // Extract the received message from Buffer
                    Cy_CANFD_ExtractMsgFromRXBuffer((cy_stc_canfd_rx_buffer_t *) pu32Adrs, &stcCanFDmsg);
                
                    // Acknowledge the FIFO message
                    pstcCanFDChMTTCAN->unRXF1A.stcField.u6F1AI = unRXF1S.stcField.u6F1GI;
                    
                    /* CAN-FD message received, check if there is a callback function */
                    /* Call callback function if it was set previously. */
                    if (pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction != NULL)
                    {
                        pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction(true, CY_CANFD_RX_FIFO1, &stcCanFDmsg);
                    }
                }
            }
        }
    }
    else if(pstcCanFDChMTTCAN->unIR.stcField.u1TC == 1) // Transmission Completed
    {
        /* CAN FD message succesfully transmitted. */
        /* Clear the Transmission completed flag */
        pstcCanFDChMTTCAN->unIR.stcField.u1TC = 1UL;
        
        /* Call callback function if it was set previously. */
        if (pstcCanFDContext->canFDInterruptHandling.canFDTxInterruptFunction != NULL)
        {
            pstcCanFDContext->canFDInterruptHandling.canFDTxInterruptFunction();
        }
    }    
}

/*******************************************************************************
* Function Name: Cy_CANFD_Init
****************************************************************************//**
*
* \brief Initializes the CAN FD module.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param pstcConfig
* CAN-FD configuration parameters.
*
* \return CY_CANFD_SUCCESS      CAN-FD module has been successfully initialized.
* \return CY_CANFD_BAD_PARAM    If one of the following conditions are met:
*                               - pstcCanFD == NULL
*                               - pstcConfig == NULL
*                               - pstcCanFDContext == NULL (invalid or disabled CAN-FD unit)
*
* \note 
* This function initialises one specific CAN FD module with the parameters
* provided in the given config sturcture.
*
* CanFD_Init() has to be called with the parameter pstcConfig of type
* stc_canfd_config_t the CAN FD settings, the CAN baudrate, the CAN FD baudrate,
* and the transmission and reception and the error and status change callback
* function can be set.
*
* All values in pstcConfig have to be in valid range. The callback functions
* can be NULL. In this case no information will be reported to the API.
*
*******************************************************************************/
cy_en_canfd_status_t Cy_CANFD_Init(cy_pstc_canfd_type_t pstcCanFD, const cy_stc_canfd_config_t* pstcConfig)
{
    // Local variable declarations
    cy_stc_canfd_context_t* pstcCanFDContext;
    uint32_t*               pu32Adrs;
    uint32_t                u32Count;
    uint32_t                u32SizeInWord;
    
    cy_stc_id_filter_t*     pstcSIDFilter;
    cy_stc_extid_filter_t*  pstcEXTIDFilter;
    cy_stc_canfd_msgram_config_t stcMsgramConfig;
    volatile stc_CANFD_CH_M_TTCAN_t* pstcCanFDChMTTCAN;
    
    /* Shadow data to avoid RMW and speed up HW access */
    un_CANFD_CH_SIDFC_t unSIDFC = { 0 };
    un_CANFD_CH_XIDFC_t unXIDFC = { 0 };
    un_CANFD_CH_XIDAM_t unXIDAM = { 0 };
    un_CANFD_CH_RXF0C_t unRXF0C = { 0 };
    un_CANFD_CH_RXF1C_t unRXF1C = { 0 };
    un_CANFD_CH_RXBC_t  unRXBC  = { 0 };
    un_CANFD_CH_TXEFC_t unTXEFC = { 0 };
    un_CANFD_CH_TXBC_t  unTXBC  = { 0 };
    un_CANFD_CH_CCCR_t  unCCCR  = { 0 };
    un_CANFD_CH_NBTP_t  unNBTP  = { 0 };
    un_CANFD_CH_DBTP_t  unDBTP  = { 0 };
    un_CANFD_CH_TDCR_t  unTDCR  = { 0 };
    un_CANFD_CH_GFC_t   unGFC   = { 0 };
    un_CANFD_CH_RXESC_t unRXESC = { 0 };
    un_CANFD_CH_TXESC_t unTXESC = { 0 };
    un_CANFD_CH_IE_t    unIE    = { 0 };
    un_CANFD_CH_ILS_t   unILS   = { 0 };
    un_CANFD_CH_ILE_t   unILE   = { 0 };
    un_CANFD_CH_RXFTOP_CTL_t unRXFTOP_CTL = { 0 };
      
    /* Check for NULL pointers */
    if ( pstcCanFD  == NULL ||
         pstcConfig == NULL ||
         ((pstcConfig->sidFilterConfig.numberOfSIDFilters != 0) && (pstcConfig->sidFilterConfig.sidFilter == NULL)) ||
         ((pstcConfig->extidFilterConfig.numberOfEXTIDFilters != 0) && (pstcConfig->extidFilterConfig.extidFilter == NULL))
       )
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Get pointer to internal data structure */
    pstcCanFDContext = Cy_CANFD_GetContext(pstcCanFD);
    
    /* Check for NULL */
    if (pstcCanFDContext == NULL)
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Set notification callback functions */
    pstcCanFDContext->canFDInterruptHandling.canFDTxInterruptFunction  = pstcConfig->txCallback;
    pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction  = pstcConfig->rxCallback;
    pstcCanFDContext->canFDInterruptHandling.canFDRxWithTopPtrInterruptFunction = pstcConfig->rxFifoWithTopCallback;
    pstcCanFDContext->canFDNotificationCb.canFDStatusInterruptFunction = pstcConfig->statusCallback;
    pstcCanFDContext->canFDNotificationCb.canFDErrorInterruptFunction  = pstcConfig->errorCallback;
    
    /* Get the pointer to M_TTCAN of the CAN FD channel */
    pstcCanFDChMTTCAN = &pstcCanFD->M_TTCAN;
    
    /* Set CCCR.INIT to 1 and wait until it will be updated. */
    pstcCanFDChMTTCAN->unCCCR.u32Register = 0x1ul;
    while(pstcCanFDChMTTCAN->unCCCR.stcField.u1INIT != 1)
    {
    }
    
    /* Cancel protection by setting CCE */
    pstcCanFDChMTTCAN->unCCCR.u32Register = 0x3ul;
    
    /* Get message RAM offset and size for this CAN FD channel */
    if(CY_CANFD_SUCCESS != Cy_CANFD_GetMsgramOffset(pstcCanFD, &stcMsgramConfig))
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Standard ID filter */
    unSIDFC.stcField.u8LSS = pstcConfig->sidFilterConfig.numberOfSIDFilters;     // Number of SID filters
    unSIDFC.stcField.u14FLSSA = stcMsgramConfig.offset >> 2;    // Start address (word) of SID filter configuration in message RAM
    pstcCanFDChMTTCAN->unSIDFC.u32Register = unSIDFC.u32Register;
    
    /* Extended ID filter */
    unXIDFC.stcField.u7LSE = pstcConfig->extidFilterConfig.numberOfEXTIDFilters; // Number of ext id filters 
    unXIDFC.stcField.u14FLESA = pstcCanFDChMTTCAN->unSIDFC.stcField.u14FLSSA + 
      (pstcConfig->sidFilterConfig.numberOfSIDFilters * SIZE_OF_SID_FILTER_IN_WORD);  // Start address (word) of ext id filter configuration in message RAM
    pstcCanFDChMTTCAN->unXIDFC.u32Register = unXIDFC.u32Register;
    
    /* Extended ID AND Mask */
    unXIDAM.stcField.u29EIDM = pstcConfig->extidFilterConfig.extIDANDMask;
    pstcCanFDChMTTCAN->unXIDAM.u32Register = unXIDAM.u32Register;
    
    /* Configuration of Rx Buffer and Rx FIFO */
    unRXESC.stcField.u3RBDS = pstcConfig->rxBufferDataSize;
    unRXESC.stcField.u3F1DS = pstcConfig->rxFifo1DataSize;
    unRXESC.stcField.u3F0DS = pstcConfig->rxFifo0DataSize; 
    pstcCanFDChMTTCAN->unRXESC.u32Register = unRXESC.u32Register;
    
    /* Rx FIFO 0 */
    unRXF0C.stcField.u1F0OM = pstcConfig->rxFifo0Config.mode;
    unRXF0C.stcField.u7F0WM = pstcConfig->rxFifo0Config.watermark;
    unRXF0C.stcField.u7F0S = pstcConfig->rxFifo0Config.numberOfFifoElements;
    unRXF0C.stcField.u14F0SA = pstcCanFDChMTTCAN->unXIDFC.stcField.u14FLESA + 
      (pstcConfig->extidFilterConfig.numberOfEXTIDFilters * SIZE_OF_EXTID_FILTER_IN_WORD);
    pstcCanFDChMTTCAN->unRXF0C.u32Register = unRXF0C.u32Register;
    
    /* Rx FIFO 1 */
    unRXF1C.stcField.u1F1OM = pstcConfig->rxFifo1Config.mode;
    unRXF1C.stcField.u7F1WM = pstcConfig->rxFifo1Config.watermark;
    unRXF1C.stcField.u7F1S = pstcConfig->rxFifo1Config.numberOfFifoElements;
    unRXF1C.stcField.u14F1SA = pstcCanFDChMTTCAN->unRXF0C.stcField.u14F0SA + 
      (pstcConfig->rxFifo0Config.numberOfFifoElements * (2 + dataBufferSizeInWord[pstcConfig->rxFifo0DataSize]));
    pstcCanFDChMTTCAN->unRXF1C.u32Register = unRXF1C.u32Register;
    
    /* RxFIFO 0,1 Top pointer logic config*/
    unRXFTOP_CTL.stcField.u1F0TPE = (pstcConfig->rxFifo0Config.topPointerLogicEnabled == false) ? 0 : 1;
    unRXFTOP_CTL.stcField.u1F1TPE = (pstcConfig->rxFifo1Config.topPointerLogicEnabled == false) ? 0 : 1;
    pstcCanFD->unRXFTOP_CTL.u32Register = unRXFTOP_CTL.u32Register;
      
    /* Rx buffer */
    unRXBC.stcField.u14RBSA = pstcCanFDChMTTCAN->unRXF1C.stcField.u14F1SA + 
      (pstcConfig->rxFifo1Config.numberOfFifoElements * (2 + dataBufferSizeInWord[pstcConfig->rxFifo1DataSize]));
    pstcCanFDChMTTCAN->unRXBC.u32Register = unRXBC.u32Register;
    
    /* Configuration of Tx Buffer and Tx FIFO/Queue */
    unTXESC.stcField.u3TBDS = pstcConfig->txBufferDataSize;
    pstcCanFDChMTTCAN->unTXESC.u32Register = unTXESC.u32Register;
    
    /* Tx FIFO/QUEUE (not use) */
    unTXEFC.stcField.u6EFWM = 0; /* Watermark interrupt disabled. */
    unTXEFC.stcField.u6EFS = 0;   /* Tx Event FIFO disabled. */
    unTXEFC.stcField.u14EFSA = pstcCanFDChMTTCAN->unRXBC.stcField.u14RBSA + 
      (pstcConfig->noOfRxBuffers * (2 + dataBufferSizeInWord[pstcConfig->rxBufferDataSize]));
    pstcCanFDChMTTCAN->unTXEFC.u32Register = unTXEFC.u32Register;
    
    /* Tx buffer */
    unTXBC.stcField.u1TFQM = 0;   /* Tx FIFO operation */
    unTXBC.stcField.u6TFQS = 0;   /* No Tx FIFO/Queue */
    unTXBC.stcField.u6NDTB = pstcConfig->noOfTxBuffers;   /* Number of Dedicated Tx Buffers*/
    unTXBC.stcField.u14TBSA = pstcCanFDChMTTCAN->unTXEFC.stcField.u14EFSA + 
      (10 * SIZE_OF_TXEVENT_FIFO_IN_WORD);    // Reserving memory for 10 TxEvent Fifo elements for easy future use
    pstcCanFDChMTTCAN->unTXBC.u32Register = unTXBC.u32Register;
    
    /* Initialize message RAM area(Entire region zeroing) */
    pu32Adrs = (uint32_t *)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + (uint32_t)CY_CANFD_MSGRAM_START + stcMsgramConfig.offset);
    u32SizeInWord = stcMsgramConfig.size >> 2;
    for(u32Count = 0; u32Count < u32SizeInWord; u32Count++)
    {
        *pu32Adrs++ = 0ul;
    }
    
    /* Configuration of CAN bus */
    /* CCCR register */
    unCCCR.stcField.u1INIT = 1;   /* Keep in initialization state */
    unCCCR.stcField.u1CCE = 1;    /* Keep CCE to enabled*/
    unCCCR.stcField.u1TXP = 0;    /* Transmit pause disabled. */
    unCCCR.stcField.u1BRSE = ((pstcConfig->canFDMode == true) ? 1 : 0);    /* Bit rate switch */
    unCCCR.stcField.u1FDOE = ((pstcConfig->canFDMode == true) ? 1 : 0);    /* FD operation */
    unCCCR.stcField.u1TEST = 0;   /* Normal operation */
    unCCCR.stcField.u1DAR = 0;    /* Automatic retransmission enabled. */
    unCCCR.stcField.u1MON_ = 0;    /* Bus Monitoring Mode is disabled. */
    unCCCR.stcField.u1CSR = 0;    /* No clock stop is requested. */
    unCCCR.stcField.u1ASM = 0;     /* Normal CAN operation. */
    pstcCanFDChMTTCAN->unCCCR.u32Register = unCCCR.u32Register;
    
    /* Nominal Bit Timing & Prescaler Register */
    unNBTP.stcField.u9NBRP = pstcConfig->bitrate.prescaler;
    unNBTP.stcField.u8NTSEG1 = pstcConfig->bitrate.timeSegment1;
    unNBTP.stcField.u7NTSEG2 = pstcConfig->bitrate.timeSegment2;
    unNBTP.stcField.u7NSJW = pstcConfig->bitrate.syncJumpWidth;
    pstcCanFDChMTTCAN->unNBTP.u32Register = unNBTP.u32Register;
    
    if(pstcConfig->canFDMode == true)
    {
        /* Data Bit Timing & Prescaler */
        unDBTP.stcField.u5DBRP = pstcConfig->fastBitrate.prescaler;
        unDBTP.stcField.u5DTSEG1 = pstcConfig->fastBitrate.timeSegment1;
        unDBTP.stcField.u4DTSEG2 = pstcConfig->fastBitrate.timeSegment2;
        unDBTP.stcField.u4DSJW = pstcConfig->fastBitrate.syncJumpWidth;
        unDBTP.stcField.u1TDC = ((pstcConfig->tdcConfig.tdcEnabled == true) ? 1 : 0); /* Transceiver Delay Compensation enabled. */
        pstcCanFDChMTTCAN->unDBTP.u32Register = unDBTP.u32Register;
        
        /* Transmitter Delay Compensation */        
        unTDCR.stcField.u7TDCO = pstcConfig->tdcConfig.tdcOffset;  /* Transmitter Delay Compensation Offset */
        unTDCR.stcField.u7TDCF = pstcConfig->tdcConfig.tdcFilterWindow;   /* Transmitter Delay Compensation Filter Window Length */
        pstcCanFDChMTTCAN->unTDCR.u32Register = unTDCR.u32Register;
    } 
    
    /* Configuration of Global Filter */
    unGFC.stcField.u2ANFS = pstcConfig->globalFilterConfig.nonMatchingFramesStandard;
    unGFC.stcField.u2ANFE = pstcConfig->globalFilterConfig.nonMatchingFramesExtended;
    unGFC.stcField.u1RRFS = ((pstcConfig->globalFilterConfig.rejectRemoteFramesStandard == true) ? 1 : 0);
    unGFC.stcField.u1RRFE = ((pstcConfig->globalFilterConfig.rejectRemoteFramesExtended == true) ? 1 : 0);
    pstcCanFDChMTTCAN->unGFC.u32Register = unGFC.u32Register;
  
    /* Standard Message ID Filters */
    pstcSIDFilter = (cy_stc_id_filter_t *)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + 
                                           (uint32_t)CY_CANFD_MSGRAM_START      + 
                                           (pstcCanFDChMTTCAN->unSIDFC.stcField.u14FLSSA << 2u));
    
    for(u32Count = 0; u32Count < pstcConfig->sidFilterConfig.numberOfSIDFilters; u32Count++)
    {
        pstcSIDFilter[u32Count] = pstcConfig->sidFilterConfig.sidFilter[u32Count];
    }
    
    /* Extended Message ID Filters */
    pstcEXTIDFilter = (cy_stc_extid_filter_t *)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + 
                                                (uint32_t)CY_CANFD_MSGRAM_START      + 
                                                (pstcCanFDChMTTCAN->unXIDFC.stcField.u14FLESA << 2u));
    
    for(u32Count = 0; u32Count < pstcConfig->extidFilterConfig.numberOfEXTIDFilters; u32Count++)
    {
        pstcEXTIDFilter[u32Count] = pstcConfig->extidFilterConfig.extidFilter[u32Count];
    }    
    
    /* Configuration of Interrupt */
    /* Interrupt Enable */
    unIE.stcField.u1ARAE  = 0; /* Access to Reserved Address */
    unIE.stcField.u1PEDE  = 0; /* Protocol Error in Data Phase */
    unIE.stcField.u1PEAE  = 0; /* Protocol Error in Arbitration Phase */
    unIE.stcField.u1WDIE  = 0; /* Watchdog */
    unIE.stcField.u1BOE   = 0; /* Bus_Off Status */
    unIE.stcField.u1EWE   = 0; /* Warning Status */
    unIE.stcField.u1EPE   = 0; /* Error Passive */
    unIE.stcField.u1ELOE  = 0; /* Error Logging Overflow */
    unIE.stcField.u1BEUE  = 0; /* Bit Error Uncorrected */
    unIE.stcField.u1BECE  = 0; /* Bit Error Corrected */
    unIE.stcField.u1DRXE  = 1; /* Message stored to Dedicated Rx Buffer */
    unIE.stcField.u1TOOE  = 0; /* Timeout Occurred */
    unIE.stcField.u1MRAFE = 0; /* Message RAM Access Failure */
    unIE.stcField.u1TSWE  = 0; /* Timestamp Wraparound */
    unIE.stcField.u1TEFLE = 0; /* Tx Event FIFO Event Lost */
    unIE.stcField.u1TEFFE = 0; /* Tx Event FIFO Full */
    unIE.stcField.u1TEFWE = 0; /* Tx Event FIFO Watermark Reached */
    unIE.stcField.u1TEFNE = 0; /* Tx Event FIFO New Entry */
    unIE.stcField.u1TFEE  = 0; /* Tx FIFO Empty */
    unIE.stcField.u1TCFE  = 0; /* Transmission Cancellation Finished */
    unIE.stcField.u1TCE   = 0; /* Transmission Completed */
    unIE.stcField.u1HPME  = 0; /* High Priority Message */
    unIE.stcField.u1RF1LE = 0; /* Rx FIFO 1 Message Lost */
    unIE.stcField.u1RF1FE = 0; /* Rx FIFO 1 Full */
    unIE.stcField.u1RF1WE = 0; /* Rx FIFO 1 Watermark Reached */
    unIE.stcField.u1RF1NE = 1; /* Rx FIFO 1 New Message */
    unIE.stcField.u1RF0LE = 0; /* Rx FIFO 0 Message Lost */
    unIE.stcField.u1RF0FE = 0; /* Rx FIFO 0 Full */
    unIE.stcField.u1RF0WE = 0; /* Rx FIFO 0 Watermark Reached */
    unIE.stcField.u1RF0NE = 1; /* Rx FIFO 0 New Message */
    pstcCanFDChMTTCAN->unIE.u32Register = unIE.u32Register;
    
    /* Interrupt Line Select */
    unILS.stcField.u1ARAL  = 0; /* Access to Reserved Address */
    unILS.stcField.u1PEDL  = 0; /* Protocol Error in Data Phase */
    unILS.stcField.u1PEAL  = 0; /* Protocol Error in Arbitration Phase */
    unILS.stcField.u1WDIL  = 0; /* Watchdog */
    unILS.stcField.u1BOL   = 0; /* Bus_Off Status */
    unILS.stcField.u1EWL   = 0; /* Warning Status */
    unILS.stcField.u1EPL   = 0; /* Error Passive */
    unILS.stcField.u1ELOL  = 0; /* Error Logging Overflow */
    unILS.stcField.u1BEUL  = 0; /* Bit Error Uncorrected */
    unILS.stcField.u1BECL  = 0; /* Bit Error Corrected */
    unILS.stcField.u1DRXL  = 0; /* Message stored to Dedicated Rx Buffer */
    unILS.stcField.u1TOOL  = 0; /* Timeout Occurred */
    unILS.stcField.u1MRAFL = 0; /* Message RAM Access Failure */
    unILS.stcField.u1TSWL  = 0; /* Timestamp Wraparound */
    unILS.stcField.u1TEFLL = 0; /* Tx Event FIFO Event Lost */
    unILS.stcField.u1TEFFL = 0; /* Tx Event FIFO Full */
    unILS.stcField.u1TEFWL = 0; /* Tx Event FIFO Watermark Reached */
    unILS.stcField.u1TEFNL = 0; /* Tx Event FIFO New Entry */
    unILS.stcField.u1TFEL  = 0; /* Tx FIFO Empty */
    unILS.stcField.u1TCFL  = 0; /* Transmission Cancellation Finished */
    unILS.stcField.u1TCL   = 0; /* Transmission Completed */
    unILS.stcField.u1HPML  = 0; /* High Priority Message */
    unILS.stcField.u1RF1LL = 0; /* Rx FIFO 1 Message Lost */
    unILS.stcField.u1RF1FL = 0; /* Rx FIFO 1 Full */
    unILS.stcField.u1RF1WL = 0; /* Rx FIFO 1 Wotermark Reached */
    unILS.stcField.u1RF1NL = 0; /* Rx FIFO 1 New Message */
    unILS.stcField.u1RF0LL = 0; /* Rx FIFO 0 Message Lost */
    unILS.stcField.u1RF0FL = 0; /* Rx FIFO 0 Full */
    unILS.stcField.u1RF0WL = 0; /* Rx FIFO 0 Wotermark Reached */
    unILS.stcField.u1RF0NL = 0; /* Rx FIFO 0 New Message */
    pstcCanFDChMTTCAN->unILS.u32Register = unILS.u32Register;
                             
    /* Interrupt Line Enable */
    unILE.stcField.u1EINT0 = 1;  /* Enable Interrupt Line 0 */
    unILE.stcField.u1EINT1 = 0;   /* Disable Interrupt Line 1 */
    pstcCanFDChMTTCAN->unILE.u32Register = unILE.u32Register;
    
    /* Initialize the Last error code data */
    m_au16LastErrorCode[Cy_CANFD_CalculateChannelIdx(pstcCanFD)] = 0;
    
    /* CAN-FD operation start */
    /* Set CCCR.INIT to 0 and wait until it will be updated. */
    unCCCR.stcField.u1INIT = 0;
    pstcCanFDChMTTCAN->unCCCR.u32Register = unCCCR.u32Register;
    while(pstcCanFDChMTTCAN->unCCCR.stcField.u1INIT != 0)
    {
    }
    
    return CY_CANFD_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_CANFD_DeInit
****************************************************************************//**
*
* \brief De-initializes the CAN FD block, returns register values to default.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
*******************************************************************************/
cy_en_canfd_status_t Cy_CANFD_DeInit(cy_pstc_canfd_type_t pstcCanFD)
{
    // Local variable declarations
    cy_stc_canfd_context_t* pstcCanFDContext;
    uint32_t*               pu32Adrs;
    uint32_t                u32Count;
    uint32_t                u32SizeInWord;
    cy_stc_canfd_msgram_config_t stcMsgramConfig;
    volatile stc_CANFD_CH_M_TTCAN_t* pstcCanFDChMTTCAN;
    
    /* Check for NULL pointers */
    if (pstcCanFD  == NULL)
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Get pointer to internal data structure */
    pstcCanFDContext = Cy_CANFD_GetContext(pstcCanFD);
    
    /* Check for NULL */
    if (pstcCanFDContext == NULL)
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Reset notification callback functions */
    pstcCanFDContext->canFDInterruptHandling.canFDTxInterruptFunction  = NULL;
    pstcCanFDContext->canFDInterruptHandling.canFDRxInterruptFunction  = NULL;
    pstcCanFDContext->canFDNotificationCb.canFDStatusInterruptFunction = NULL;
    pstcCanFDContext->canFDNotificationCb.canFDErrorInterruptFunction  = NULL;
    
    /* Get the pointer to M_TTCAN of the CAN FD channel */
    pstcCanFDChMTTCAN = &pstcCanFD->M_TTCAN;
    
    /* Set CCCR.INIT to 1 and wait until it will be updated. */
    pstcCanFDChMTTCAN->unCCCR.u32Register = 0x1ul;
    while(pstcCanFDChMTTCAN->unCCCR.stcField.u1INIT != 1)
    {
    }
    
    /* Cancel protection by setting CCE */
    pstcCanFDChMTTCAN->unCCCR.u32Register = 0x3ul;
    
    /* Get message RAM offset and size for this CAN FD channel */
    if(CY_CANFD_SUCCESS != Cy_CANFD_GetMsgramOffset(pstcCanFD, &stcMsgramConfig))
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Standard ID filter */
    pstcCanFDChMTTCAN->unSIDFC.u32Register = 0ul;
    /* Extended ID filter */
    pstcCanFDChMTTCAN->unXIDFC.u32Register = 0ul;
    /* Extended ID AND Mask */
    pstcCanFDChMTTCAN->unXIDAM.u32Register = 0x1FFFFFFFul;
    /* Configuration of Rx Buffer and Rx FIFO */
    pstcCanFDChMTTCAN->unRXESC.u32Register = 0ul;
    /* Rx FIFO 0 */
    pstcCanFDChMTTCAN->unRXF0C.u32Register = 0ul;
    /* Rx FIFO 1 */
    pstcCanFDChMTTCAN->unRXF1C.u32Register = 0ul;
    /* Rx FIFO Top pointer logic*/
    pstcCanFD->unRXFTOP_CTL.u32Register = 0ul;
    /* Rx buffer */
    pstcCanFDChMTTCAN->unRXBC.u32Register = 0ul;
    /* Configuration of Tx Buffer and Tx FIFO/Queue */
    pstcCanFDChMTTCAN->unTXESC.u32Register = 0ul;
    /* Tx FIFO/QUEUE (not use) */
    pstcCanFDChMTTCAN->unTXEFC.u32Register = 0ul;
    /* Tx buffer */
    pstcCanFDChMTTCAN->unTXBC.u32Register = 0ul;

    /* Initialize message RAM area(Entire region zeroing) */
    pu32Adrs = (uint32_t *)(((uint32_t)pstcCanFD & 0xFFFF0000ul) + (uint32_t)CY_CANFD_MSGRAM_START + stcMsgramConfig.offset);
    u32SizeInWord = stcMsgramConfig.size >> 2;
    for(u32Count = 0; u32Count < u32SizeInWord; u32Count++)
    {
        *pu32Adrs++ = 0ul;
    }

    /* Configuration of CAN bus */
    /* CCCR register */
    pstcCanFDChMTTCAN->unCCCR.u32Register = 0x3ul;
    /* Nominal Bit Timing & Prescaler Register */
    pstcCanFDChMTTCAN->unNBTP.u32Register = 0x06000A03ul;
    /* Data Bit Timing & Prescaler */
    pstcCanFDChMTTCAN->unDBTP.u32Register = 0x00000A33ul;
    /* Transmitter Delay Compensation */        
    pstcCanFDChMTTCAN->unTDCR.u32Register = 0ul;
    /* Configuration of Global Filter */
    pstcCanFDChMTTCAN->unGFC.u32Register = 0ul;
    /* Configuration of Interrupt */
    /* Interrupt Enable */
    pstcCanFDChMTTCAN->unIE.u32Register = 0ul;
    /* Interrupt Line Select */
    pstcCanFDChMTTCAN->unILS.u32Register = 0ul;

    /* Interrupt Line Enable */
    pstcCanFDChMTTCAN->unILE.u32Register = 0ul;
    
    /* Initialize the Last error code data */
    m_au16LastErrorCode[Cy_CANFD_CalculateChannelIdx(pstcCanFD)] = 0;     
    return CY_CANFD_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_CANFD_UpdateAndTransmitMsgBuffer
****************************************************************************//**
*
* \brief Updates the message data of a message buffer and start transmission.
* Transmits the message immediately.
* Function CanFD_Init() must be called before setup the identifier and enable
* this message buffer.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param u8MsgBuf
* Message buffer index (0-31)
*
* \param pstcMsg
* CAN-FD message data
*
* \return CY_CANFD_SUCCESS      Message buffer has been succesfully updated.
* \return CY_CANFD_BAD_PARAM    If one of the following conditions are met:
*                               - pstcCanFD == NULL
*                               - pstcMsg == NULL
*                               - pstcCanFDTxBuffer == NULL
*
*******************************************************************************/
cy_en_canfd_status_t Cy_CANFD_UpdateAndTransmitMsgBuffer( cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf, cy_stc_canfd_msg_t* pstcMsg )
{
    // Local variable declarations
    cy_stc_canfd_context_t* pstcCanFDContext;
    uint16_t u16DlcTemp;
    uint16_t u16Count;
    uint8_t u8DataLengthWord;
    
    cy_stc_canfd_tx_buffer_t* pstcCanFDTxBuffer;
    volatile stc_CANFD_CH_M_TTCAN_t* pstcCanFDChMTTCAN;
    
    /* Check for NULL pointers */
    if ( pstcCanFD  == NULL ||
         pstcMsg == NULL
       )
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    if(u8MsgBuf > 31)
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    /* Get pointer to internal data structure */
    pstcCanFDContext = Cy_CANFD_GetContext(pstcCanFD);
    
    /* Check for NULL */
    if (pstcCanFDContext == NULL)
    {
        return CY_CANFD_BAD_PARAM;
    }

    /* Get the pointer to M_TTCAN of the CAN FD channel */
    pstcCanFDChMTTCAN = &pstcCanFD->M_TTCAN;

    /* Check if CAN FD controller is in not in INIT state and Tx buffer is empty or not */
    if((pstcCanFDChMTTCAN->unCCCR.stcField.u1INIT != 0) ||
       ((pstcCanFDChMTTCAN->unTXBRP.u32Register & (1ul << u8MsgBuf)) != 0)
      )
    {
        return CY_CANFD_BAD_PARAM;
    }

    /* Get Tx Buffer address */
    pstcCanFDTxBuffer = (cy_stc_canfd_tx_buffer_t*)Cy_CANFD_CalcTxBufAdrs(pstcCanFD, u8MsgBuf);
    
    if(pstcCanFDTxBuffer == NULL)
    {
        return CY_CANFD_BAD_PARAM;
    }
    
    pstcCanFDTxBuffer->t0_f.rtr = 0; /* Transmit data frame. */
    pstcCanFDTxBuffer->t0_f.xtd = (pstcMsg->idConfig.extended == true) ? 1 : 0;
    pstcCanFDTxBuffer->t0_f.id  = (pstcCanFDTxBuffer->t0_f.xtd == 0) ? 
                                            (pstcMsg->idConfig.identifier << 18) : pstcMsg->idConfig.identifier;
    
    pstcCanFDTxBuffer->t1_f.efc = 0; /* Tx Event Fifo not used */
    pstcCanFDTxBuffer->t1_f.mm  = 0; /* Not used */
    pstcCanFDTxBuffer->t1_f.dlc = pstcMsg->dataConfig.dataLengthCode;
    pstcCanFDTxBuffer->t1_f.fdf = (pstcMsg->canFDFormat == true) ? 1 : 0;
    pstcCanFDTxBuffer->t1_f.brs = (pstcMsg->canFDFormat == true) ? 1 : 0;
    
    /* Convert the DLC to data byte word */
    if (pstcMsg->dataConfig.dataLengthCode < 8 )
    {
        u16DlcTemp = 0;
    }
    else
    {
        u16DlcTemp = pstcMsg->dataConfig.dataLengthCode - 8;
    }
    u8DataLengthWord = dataBufferSizeInWord[u16DlcTemp];

    /* Data set */
    for ( u16Count = 0; u16Count < u8DataLengthWord; u16Count++ )
    {
        pstcCanFDTxBuffer->data_area_f[u16Count] = pstcMsg->dataConfig.data[u16Count];
    }

    pstcCanFDChMTTCAN->unTXBAR.u32Register = 1ul << u8MsgBuf;     // Transmit buffer add request
        
    return CY_CANFD_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_CANFD_GetTxBufferStatus
****************************************************************************//**
*
* \brief Get the status of CAN FD Tx Buffer.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param u8MsgBuf
* Message buffer index (0-31)
*
* \param pstcMsg
* CAN-FD message data
*
* \return CY_CANFD_TX_BUFFER_PENDING           - Pending transmission request and no      
                                                 cancel request issued
* \return CY_CANFD_TX_BUFFER_TRANSMIT_OCCURRED - Transmission completed
* \return CY_CANFD_TX_BUFFER_CANCEL_REQUESTED  - Cancel request issued
* \return CY_CANFD_TX_BUFFER_CANCEL_FINISHED   - Transmission cancelled                                       
* \return CY_CANFD_TX_BUFFER_IDLE              - If none of the above conditions are satisfied or
*                                              - If one of the following conditions are met:
*                                                   - pstcCanFD == NULL
*                                                   - u8MsgBuf > 31
*******************************************************************************/
cy_en_canfd_tx_buffer_status_t Cy_CANFD_GetTxBufferStatus( cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf)
{
    volatile stc_CANFD_CH_M_TTCAN_t* pstcCanFDChMTTCAN;
    cy_en_canfd_tx_buffer_status_t enTxBufferStatus;
    
    if ((pstcCanFD  == NULL ) ||
        (u8MsgBuf > 31)
       )
    {
        return CY_CANFD_TX_BUFFER_IDLE;
    }
    
    /* Get the pointer to M_TTCAN of the CAN FD channel */
    pstcCanFDChMTTCAN = &pstcCanFD->M_TTCAN;
    
    /* Initialize the return value */
    enTxBufferStatus = CY_CANFD_TX_BUFFER_IDLE;
    
    if((pstcCanFDChMTTCAN->unTXBRP.u32Register & (1ul << u8MsgBuf)) != 0)     // Pending transmission request. 
    {
        /* Check if cancel request issued*/
        if((pstcCanFDChMTTCAN->unTXBCR.u32Register & (1ul << u8MsgBuf)) == 0) 
        {
            // No cancellation request
            enTxBufferStatus = CY_CANFD_TX_BUFFER_PENDING;
        }
        else        
        {
            // Cancel request issued
            enTxBufferStatus = CY_CANFD_TX_BUFFER_CANCEL_REQUESTED;
        }
    }
    else if((pstcCanFDChMTTCAN->unTXBTO.u32Register & (1ul << u8MsgBuf)) != 0)    // Transmission occurred
    {
        enTxBufferStatus = CY_CANFD_TX_BUFFER_TRANSMIT_OCCURRED;
    }
    else if((pstcCanFDChMTTCAN->unTXBCF.u32Register & (1ul << u8MsgBuf)) != 0)    // Cancellation finished
    {
        enTxBufferStatus = CY_CANFD_TX_BUFFER_CANCEL_FINISHED;
    }
    
    return enTxBufferStatus;
}

/*******************************************************************************
* Function Name: Cy_CANFD_ClockInit
****************************************************************************//**
*
* \brief Initializes the CAN FD clock.
*
* \param pstcCanFD
* Pointer to register area of a CAN FD channel.
*
* \param u8CanFDPrescaler
*       //TODO: DENA.
*
* \param enCanFDClockSource
*  CAN FD channel clock source
*
* \return CY_CANFD_SUCCESS      CAN FD clock succesfully initialized.
* \return CY_CANFD_BAD_PARAM    If one of the following conditions are met:
*                               - pstcCanFD == NULL
*                               - u8CanFDPrescaler   //TODO: DENA
*                               - enCanFDClockSource //TODO: DENA
*
*******************************************************************************/
cy_en_canfd_status_t Cy_CANFD_ClockInit( cy_pstc_canfd_type_t pstcCanFD, uint8_t u8CanFDPrescaler, cy_en_canfd_clock_source_t enCanFDClockSource )
{
    //TODO: Implementation
    return CY_CANFD_SUCCESS;
}

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
