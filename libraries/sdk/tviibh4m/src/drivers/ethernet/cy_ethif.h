/***************************************************************************//**
* \file cy_ethif.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Ethernet Interface [Cy_EthIf] driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_ETHIF_H
#define CY_ETHIF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "cdn_errno.h"
#include "cedi.h"
#include "cps_v2.h"
#include "emac_regs_macro.h"
#include "edd_int.h"

/** \addtogroup group_ethif_macros
* \{
*/

/***************************************
*  Device Depending Parameters
***************************************/

/** Ethernet configurations constants  */
#define CY_ETH_STANDARD_BD                 (0)                // Standard Buffer Descriptor Mode
#define CY_ETH_EXTENDED_BD                 (1)                // Extended Buffer Descriptor Mode
#define CY_ETH_TX_FSFM                     (0)                // Full Store and Forward Mode [FSFM] for Tx
#define CY_ETH_TX_PSFM                     (1)                // Partial Store and Forward Mode [PSFM] for Tx
#define CY_ETH_RX_FSFM                     (0)                // Full Store and Forward Mode for Rx
#define CY_ETH_RX_PSFM                     (1)                // Partial Store and Forward Mode for Rx

/** Ethernet channel  */
#define CY_ETH_DEFINE_NUM_IP               (1)                // Number of Ethernet channels intended to use
#ifdef ETH1
  #undef CY_ETH_DEFINE_NUM_IP
  #define CY_ETH_DEFINE_NUM_IP             (2)
#endif

#define CY_ETH_DEFINE_NUM_RXQS             (2)                // Number of queues intended to use, maximum 3
#define CY_ETH_DEFINE_NUM_TXQS             (3)                // Number of queues intended to use, maximum 3 
/** Ethernet Operation Mode */
#define CY_ETH_DEFINE_TX_OP_MODE           CY_ETH_TX_FSFM     // FSFM OR PSFM
#define CY_ETH_DEFINE_RX_OP_MODE           CY_ETH_RX_FSFM     // FSFM OR PSFM
#define CY_ETH_DEFINE_TX_WATERMARK         (50)               // Watermark level for Tx
#define CY_ETH_DEFINE_RX_WATERMARK         (50)               // Watermark level for Rx
/** Buffer Descriptor mode  */
#define CY_ETH_DEFINE_BD                   CY_ETH_EXTENDED_BD // standard or extended
/** Memory Allocation defines */                                       // Frame Buffers are allocated based on number of buffer descriptors
#define CY_ETH_DEFINE_TOTAL_BD_PER_TXQUEUE (12)               // Total number of Buffer Descriptors allocated for each transmit queue
#define CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE (4)                // Total number of Buffer Descriptors allocated for each receive queue

/*  Device Depending Parameters End */

/** Driver major version */
#define CY_ETHIF_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_ETHIF_DRV_VERSION_MINOR       0

/** EthIf driver ID */
#define CY_ETHIF_ID                     CY_PDL_DRV_ID(0xXXu)    // TODO: CY_PDL_DRV_ID for EtherIf


/***************************************
*        Constants
***************************************/
/** Ethernet configurations constants  */
#define CY_ETH_TX_PBUF_SIZE         (1)                         // 1: Use full configured addressable space 
#define CY_ETH_RX_PBUF_SIZE         (3)                         // 3: Use full configure addressable space

#define CY_ETH_ADDR_REG_BASE ((uint32_t)ETH0)
#ifdef ETH1
  #define CY_ETH_ADDR_REG_OFFSET ((uint32_t)ETH1 - (uint32_t)ETH0)
#else
  #define CY_ETH_ADDR_REG_OFFSET (0)
#endif
#define CY_ETH_GEMGXL_ADDR_OFFSET (0x1000ul)
 
/** Driver constants    */
#define CY_ETH_HEADER_SIZE          (14)
#define CY_ETH_SIZE_MIN_FRAME       (64)
#define CY_ETH_SIZE_MAX_FRAME       (1536)
#define CY_ETH_SIZE_BUF_TXQ_RXQ     (1536)

#if CY_ETH_DEFINE_BD == CY_ETH_EXTENDED_BD
#define CY_ETH_BD_SIZE              (16)        // Bytes
#else
#define CY_ETH_BD_SIZE              (8)         // Bytes
#endif

#define CY_ETH_DISABLE_0            (0)
#define CY_ETH_ENABLE_1             (1)
#define CY_ETH_MDIO_BUSY_0          (0)
#define CY_ETH_MDIO_IDLE_1          (1)
#define CY_ETH_QS0_0                (0)
#define CY_ETH_QS1_1                (1)
#define CY_ETH_QS2_2                (2)
#define CY_ETH_QS3_3                (3)
#define CY_ETH_NOT_INITIALIZE_0     (0)
#define CY_ETH_INITIALIZE_1         (1)
#define CY_ETHIF_BUFFER_CLEARED_0   (0)

#define CY_ETH_TOTAL_TX_BUF         (CY_ETH_DEFINE_NUM_IP * CY_ETH_DEFINE_NUM_TXQS * CY_ETH_DEFINE_TOTAL_BD_PER_TXQUEUE)
#define CY_ETH_TOTAL_RX_BUF         (CY_ETH_DEFINE_NUM_IP * CY_ETH_DEFINE_NUM_RXQS * CY_ETH_DEFINE_TOTAL_BD_PER_RXQUEUE * 2)   // *2 is to replace the already provided buffer to BDs

#define CY_ETHIF_NO_BUFFER_AVAILABLE (0xFFFFFFFF)
#define CY_ETHIF_BUFFER_AVAILABLE    (0xFFFFFFFE)

#define CY_ETHIF_MDIO_READ_FAILED   (2)
#define CY_ETHIF_PHY_FLAG           (0)                         // defines clause 22

#define CY_ETHIF_TX_STATUS_CLEAR    (0x000001FF)

#define CY_ETHIF_PAUSE_P0           (0)
#define CY_ETHIF_PAUSE_P1           (1)
#define CY_ETHIF_PAUSE_P2           (2)
#define CY_ETHIF_PAUSE_P3           (3)
#define CY_ETHIF_PAUSE_P4           (4)
#define CY_ETHIF_PAUSE_P5           (5)
#define CY_ETHIF_PAUSE_P6           (6)
#define CY_ETHIF_PAUSE_P7           (7)

/** \} group_ethif_macros */

/***************************************
*        Enumerations
***************************************/
/**
* \addtogroup group_ethif_enums
* \{
*/

/**
 *****************************************************************************
 ** \brief EthIf driver error codes.
 ** 
 *****************************************************************************/
typedef enum
{
    CY_ETHIF_SUCCESS = 0,               /** Returned successful */
    CY_ETHIF_BAD_PARAM,                 /** Bad parameter was passed */
    CY_ETHIF_MEMORY_NOT_ENOUGH,         /** Assigned memory for BDs not enough */
    CY_ETHIF_LINK_DOWN,                 /** Link between nodes is not up */
    CY_ETHIF_LINK_UP,                   /** Link between nodes is up */
    CY_ETHIF_BUFFER_NOT_AVAILABLE,      /** No local buffer available to send the frame */
} cy_en_ethif_status_t;

/**
 *****************************************************************************
 ** \brief EthIf driver error codes.
 ** 
 *****************************************************************************/
typedef enum 
{
    CY_ETHIF_BUFFER_UNINIT = 0,         /** Buffer Uninitialized */
    CY_ETHIF_BUFFER_FREE,               /** Buffer free  */
    CY_ETHIF_BUFFER_OCCUPIED,           /** Buffer has been allocated to BD or in case of Tx loaded with the source data */
    CY_ETHIF_BUFFER_RELEASED,           /** Buffer has been used */
} cy_en_ethif_buffer_status;

/**
 *****************************************************************************
 ** \brief EMAC Drive Modes.
 ** 
 *****************************************************************************/
typedef enum 
{
    /**=================================================================================================*/
    /** CTL.ETH_MODE  |	Network_config[0] |  Network_config[10]   |	    PHY mode                        */
    /**               |     (speed)       | (gigabit_mode_enable) |                                     */
    /**=================================================================================================*/
    /**       0       |          0        |          0            |   MII   - 10Mbps                    */
    /**       0       |          1        |          0            |   MII   - 100Mbps                   */
    /**       1       |          0        |          1            |   GMII  - 1000Mbps                  */
    /**       2       |          0        |          0            |   RGMII - 10Mbps (4bits/Cycle)      */
    /**       2       |          1        |          0            |   RGMII - 100Mbps (4bits/Cycle)     */
    /**       2       |          0        |          1            |   RGMII - 1000Mbps (8bits/Cycle)    */
    /**       3       |          0        |          0            |   RMII  - 10Mbps                    */
    /**       3       |          1        |          0            |   RMII  - 100Mbps                   */
    /**=================================================================================================*/

    CY_ETHIF_CTL_MII_10         = 0,
    CY_ETHIF_CTL_MII_100        = 1,
    CY_ETHIF_CTL_GMII_1000      = 2,
    CY_ETHIF_CTL_RGMII_10       = 3,
    CY_ETHIF_CTL_RGMII_100      = 4,
    CY_ETHIF_CTL_RGMII_1000     = 5,
    CY_ETHIF_CTL_RMII_10        = 6,
    CY_ETHIF_CTL_RMII_100       = 7,
} cy_en_ethif_Interface_Speed_Sel;

/**
 *****************************************************************************
 ** \brief Reference Clock for RMII/RGMII.
 ** 
 *****************************************************************************/
typedef enum 
{
    CY_ETHIF_EXTERNAL_HSIO = 0,         /** Clock coming from HSIO */
    CY_ETHIF_INTERNAL_PLL,              /** Clock referenced from Internal PLL source */
} cy_en_ethif_clock_ref;

typedef enum
{
    CY_ETHIF_PTP_EVENT_PRIMARY = 0, /* event for Sync and Delay_Req */
    CY_ETHIF_PTP_EVENT_PEER    = 1, /* event for Pdelay_Req and Pdelay_Resp */
} cy_en_ethif_ptp_event_type_t;

/** \} group_ethif_enums */

/***************************************
*        Configuration Structures
***************************************/
/**
* \addtogroup group_ethif_data_structures
* \{
*/

/**
 *****************************************************************************
 ** \brief Ethernet MAC Unit Type
 *****************************************************************************/
typedef  volatile stc_ETH_t*  cy_pstc_eth_type_t;

/**
 *****************************************************************************
 ** \brief Wrapper Configuration 
 *****************************************************************************/
typedef struct
{
    cy_en_ethif_Interface_Speed_Sel stcInterfaceSel;    /** Interface selection for ENET    */
    cy_en_ethif_clock_ref bRefClockSource;              /** Reference clock selection   */     
    uint8_t u8RefClkDiv;                                /** Actual division would be u8RefClkDiv + 1    */
} cy_str_ethif_wrapper_config;

/**
 *****************************************************************************
 ** \brief Queue status
 *****************************************************************************/
typedef struct
{
    bool bTxQueueDisable[3];                              /** Tx Queue0-2 || 0: Queue Enabled, 1: Queue Disabled */
    bool bRxQueueDisable[3];                              /** Rx Queue0-2 || 0: Queue Enabled, 1: Queue Disabled */    
} cy_str_ethif_queueDisablestatus;

/**
 *****************************************************************************
 ** \brief Buffer status
 *****************************************************************************/
typedef struct
{
    CEDI_BuffAddr               cy_ethif_bufaddr;       /** Buffer address as per requirement by Cadence driver */
    cy_en_ethif_buffer_status   enBufStatus;            /** Buffer status */
} cy_str_ethif_bufstatus;

/**
 *****************************************************************************
 ** \brief Time Stamp Unit Init
 *****************************************************************************/
typedef struct
{
    CEDI_1588TimerVal * pstcTimerValue;                 /** TSU Timer value     */
    CEDI_TimerIncrement * pstcTimerIncValue;            /** TSU Timer increment value   */
    bool bOneStepTxSyncEnable;                          /** One step sync enable    */
    CEDI_TxTsMode enTxDescStoreTimeStamp;               /** Store Time stamp value in Tx descriptor */
    CEDI_RxTsMode enRxDescStoreTimeStamp;               /** Store Time stamp value in Rx descriptor */
    bool bStoreNSinRxDesc;                              /** Store NanoSecond field of TSU in Rx Descriptor  */
} cy_str_ethif_TSU_Config;

/**
 *****************************************************************************
 ** \brief Frame transmission complete callback function (cy_ethif_tx_msg_func_ptr_t).
 **
 ** Signals a successful completed transmission.
 *****************************************************************************/
 typedef void (*cy_ethif_tx_msg_func_ptr_t)( cy_pstc_eth_type_t pstcEth, uint8_t u8QueueIndex , uint32_t bufIdx);

 /**
 *****************************************************************************
 ** \brief Frame transmission Error callback function (cy_ethif_tx_error_func_ptr_t).
 **
 ** Signals a transmission error.
 *****************************************************************************/
 typedef void (*cy_ethif_tx_error_func_ptr_t)( cy_pstc_eth_type_t pstcEth, uint8_t u8QueueIndex );
 
 /**
 *****************************************************************************
 ** \brief Frame received callback function (cy_ethif_rx_frame_func_ptr_t).
 **
 ** Signals a successful reception of frame.
 *****************************************************************************/
 typedef void (*cy_ethif_rx_frame_func_ptr_t)( cy_pstc_eth_type_t pstcEth, uint8_t * u8RxBuffer, uint32_t u32Length );
 
 /**
 *****************************************************************************
 ** \brief TSU Second counter increment callback function (cy_ethif_rx_frame_func_ptr_t).
 **
 ** Signals a One second increment in the TSU.
 *****************************************************************************/
 typedef void (*cy_ethif_tsu_inc_func_ptr_t)( cy_pstc_eth_type_t pstcEth );
 
 /**
 *****************************************************************************
 ** \brief TSU counter compare match callback function
 **
 ** Signals a compare register match in the TSU.
 *****************************************************************************/
 typedef void (*cy_ethif_tsu_match_func_ptr_t)( cy_pstc_eth_type_t pstcEth );

 /**
 *****************************************************************************
 ** \brief Ethernet MAC call back handlers
 *****************************************************************************/
 typedef struct
 {
    cy_ethif_tx_msg_func_ptr_t    txcompletecb;     /** Transmit complete    */
    cy_ethif_tx_error_func_ptr_t  txerrorcb;        /** Tx Error    */
    cy_ethif_rx_frame_func_ptr_t  rxframecb;        /** Frame Received */ 
    cy_ethif_tsu_inc_func_ptr_t   tsuSecondInccb;   /** TSU timer Second counter incremented */
    cy_ethif_tsu_match_func_ptr_t tsuCompMatchcb;   /** TSU compare match */
 } cy_ethif_callbackfunctions;
 
/**
 *****************************************************************************
 ** \brief Ethernet MAC detailed configurations 
 *****************************************************************************/
typedef struct
{
    bool bintrEnable;                              /** interrupts/events to enable on start   */
    CEDI_DmaDatBLen dmaDataBurstLen;                    /** fixed burst length for DMA data transfers */
                                                        /** bit4:0 amba_burst_length                                  */
                                                        /** 1xxxx: attempt use burst up to 16 (CEDI_DMA_DBUR_LEN_16)  */
                                                        /** 01xxx: attempt use burst up to  8 (CEDI_DMA_DBUR_LEN_8)   */
                                                        /** 001xx: attempt use burst up to  4 (CEDI_DMA_DBUR_LEN_4)   */
                                                        /** 0001x: always use single burst                            */
                                                        /** 00001: always use single burst    (CEDI_AMBD_BURST_LEN_1) */
                                                        /** 00000: best AXI burst up to 256 beats                     */
    uint8_t u8dmaCfgFlags;                              /** DMA config register bits 24, 25 & 26.                   */
                                                        /** OR the following bit-flags to set corresponding bits -  */
                                                        /** CEDI_CFG_DMA_DISC_RXP,              */
                                                        /** CEDI_CFG_DMA_FRCE_RX_BRST,          */
                                                        /** CEDI_CFG_DMA_FRCE_TX_BRST        */
    CEDI_MdcClkDiv mdcPclkDiv;                          /** divisor to generate MDC from pclk */
                                                        /** CEDI_MDC_DIV_BY_8 = 0       */
                                                        /** CEDI_MDC_DIV_BY_16 = 1      */
                                                        /** CEDI_MDC_DIV_BY_32 = 2      */
                                                        /** CEDI_MDC_DIV_BY_48 = 3      */
                                                        /** CEDI_MDC_DIV_BY_64 = 4      */
                                                        /** CEDI_MDC_DIV_BY_96 = 5      */
                                                        /** CEDI_MDC_DIV_BY_128 = 6     */
                                                        /** CEDI_MDC_DIV_BY_224 = 7     */
    uint8_t u8rxLenErrDisc;                             /** enable discard of frames with length field error */
    uint8_t u8disCopyPause;                             /** disable copying Rx pause frames to memory */
    uint8_t u8chkSumOffEn;                              /** enable checksum offload operation - OR bit-flags
                                                        * to enable in Tx and/or Rx:
                                                        * CEDI_CFG_CHK_OFF_TX,  CEDI_CFG_CHK_OFF_RX     */
    uint8_t u8rx1536ByteEn;                             /** enable Rx of frames up to 1536 bytes */
    uint8_t u8rxJumboFrEn;                              /** enable Rx of jumbo frames */
    uint8_t u8enRxBadPreamble;                          /** enable Rx frames with non-standard preamble */
    uint8_t u8ignoreIpgRxEr;                            /** ignore IPG rx_er (NetCfg b30) */
    uint8_t u8storeUdpTcpOffset;
    uint8_t u8aw2wMaxPipeline;                          /**
                                                        * Maximum number of outstanding AXI write requests,
                                                        * that can be issued by DMA via the AW channel.
                                                        * Must not be = 0 if using AXI as this would disable writes
                                                        */
    uint8_t u8ar2rMaxPipeline;                          /**
                                                        * Maximum number of outstanding AXI read requests,
                                                        * that can be issued by DMA via the AR channel.
                                                        * Must not be = 0 if using AXI as this would disable reads
                                                        */
    uint8_t u8pfcMultiQuantum;                          /** enable pfc multiple quantum (8 different priorities) */
    cy_str_ethif_wrapper_config * pstcWrapperConfig;    /** Configuration for Wrapper */
    cy_str_ethif_TSU_Config *     pstcTSUConfig;        /** Configuration for TSU */
    bool btxq0enable;                                   /** Tx Q0 Enable    */
    bool btxq1enable;                                   /** Tx Q1 Enable    */
    bool btxq2enable;                                   /** Tx Q2 Enable    */
    bool brxq0enable;                                   /** Rx Q0 Enable    */
    bool brxq1enable;                                   /** Rx Q1 Enable    */
    bool brxq2enable;                                   /** Rx Q2 Enable    */

} cy_stc_ethif_configuration_t;

/**
 ******************************************************************************
 ** \brief Interrupt configuration for Ethernet MAC
 ******************************************************************************/
typedef struct
{
    bool btsu_time_match;                  /** Time stamp unit time match event */
    bool bwol_rx;                          /** Wake on LAN event received */
    bool blpi_ch_rx;                       /** LPI indication status bit change received */    /* TODO: LPI, wait for conclusion?    */
    bool btsu_sec_inc;                     /** TSU seconds register increment */
    bool bptp_tx_pdly_rsp;                 /** PTP pdelay_resp frame transmitted */
    bool bptp_tx_pdly_req ;                /** PTP pdelay_req frame transmitted */
    bool bptp_rx_pdly_rsp;                 /** PTP pdelay_resp frame received */
    bool bptp_rx_pdly_req;                 /** PTP pdelay_req frame received */
    bool bptp_tx_sync;                     /** PTP sync frame transmitted */
    bool bptp_tx_dly_req;                  /** PTP delay_req frame transmitted */
    bool bptp_rx_sync;                     /** PTP sync frame received */
    bool bptp_rx_dly_req;                  /** PTP delay_req frame received */
    // bool bpcs_lp_page_rx;                  /** PCS auto-negotiation link partner page received */
    // bool bpcs_an_complete;                 /** PCS auto-negotiation has completed */
    bool bext_intr;                        /** External input interrupt detected */
    bool bpause_frame_tx;                  /** Pause frame transmitted */
    bool bpause_time_zero;                 /** Pause time reaches zero or zeroq pause frame received */
    bool bpause_nz_qu_rx;                  /** Pause frame with non-zero quantum received */
    bool bhresp_not_ok;                    /** DMA hresp not OK */
    bool brx_overrun;                      /** Rx overrun error */
    bool bpcs_link_change_det;             /** Link status change detected by PCS */           /* TODO: PCS, wait for conclusion   */
    bool btx_complete;                     /** Frame has been transmitted successfully */
    bool btx_fr_corrupt;                   /** Tx frame corruption */
    bool btx_retry_ex_late_coll;           /** Retry limit exceeded or late collision */
    bool btx_underrun;                     /** Tx underrun */
    bool btx_used_read;                    /** Used bit set has been read in Tx descriptor list */
    bool brx_used_read;                    /** Used bit set has been read in Rx descriptor list */
    bool brx_complete;                     /** Frame received successfully and stored */
    bool bman_frame;                       /** Management Frame Sent */
    
    /** Callback Function Pointers  */
    cy_ethif_rx_frame_func_ptr_t  rxframecb;    /** Frame Received  */  
    cy_ethif_tx_error_func_ptr_t  txerrorcb;    /** Transmit error  */
    cy_ethif_tx_msg_func_ptr_t    txcompletecb; /** Transmit complete */
    cy_ethif_tsu_inc_func_ptr_t   tsuSecondInccb; /** TSU timer Second counter incremented */
    cy_ethif_tsu_match_func_ptr_t tsuCompMatchcb; /** TSU timer compare match */
} cy_stc_ethif_interruptconfig_t;

/** \} group_ethif_data_structures */

/**
* \addtogroup group_ethif_enums
* \{
*/

/**
 ******************************************************************************
 ** \brief filter configuration for Ethernet MAC
 ******************************************************************************/
typedef enum
{
    CY_ETH_FILTER_TYPE_DESTINATION = 0,
    CY_ETH_FILTER_TYPE_SOURCE      = 1,
} cy_en_eth_filter_type_t;

typedef enum
{
    CY_ETH_FILTER_NUM_1 = 1,
    CY_ETH_FILTER_NUM_2 = 2,
    CY_ETH_FILTER_NUM_3 = 3,
    CY_ETH_FILTER_NUM_4 = 4,
    CY_ETH_FILTER_NUM_INV,
} cy_en_eth_filter_num_t;

/** \} group_ethif_enums */

/**
* \addtogroup group_ethif_data_structures
* \{
*/

typedef struct
{
    cy_en_eth_filter_type_t typeFilter;
    CEDI_MacAddress         filterAddr;
    uint8_t                 ignoreBytes; // e.g. ignoreBytes = 0x01 implies first byte received should not be compared.
                                         // e.g. ignoreBytes = 0x03 implies first and second byte received should not be compared.
} cy_stc_eth_filter_config_t;

/** \} group_ethif_data_structures */

/***************************************
*        Function Prototypes
***************************************/
/**
* \addtogroup group_ethif_functions
* \{
*/

cy_en_ethif_status_t Cy_EthIf_Init(cy_pstc_eth_type_t pstcEth, cy_stc_ethif_configuration_t * pstcEthIfConfig, cy_stc_ethif_interruptconfig_t * pstcInterruptList);
cy_en_ethif_status_t Cy_EthIf_LinkStatus_DP83867IR(uint8_t u8EthIfInstance);
void Cy_EthIf00_DecodeEvent(void);
#ifdef ETH1
void Cy_EthIf01_DecodeEvent(void);
#endif
uint32_t Cy_EthIf_FreeAllReleasedTxBuffer(void);
uint32_t Cy_EthIf_FreeOneReleasedTxBuffer(uint32_t* pFreedTxBufIdx);
uint32_t Cy_EthIf_FreeAllReleasedRxBuffer(void);
uint32_t Cy_EthIf_FreeOneReleasedRxBuffer(uint32_t* pFreedRxBufIdx);
uint8_t Cy_EthIF_ClearReleasedBuf (bool bClearAll, bool bTransmitBuf);
cy_en_ethif_status_t Cy_EthIf_TransmitFrame (cy_pstc_eth_type_t pstcEth, uint8_t * pu8TxBuffer, uint16_t u16Length, uint8_t u8QueueIndex, bool bEndBuffer);
cy_en_ethif_status_t Cy_EthIf_GetTimerValue (cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* stcRetTmrValue);
cy_en_ethif_status_t Cy_EthIf_SetTimerValue (cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal * pstcTmrValue);
cy_en_ethif_status_t Cy_EthIf_TxPauseFrame (cy_pstc_eth_type_t pstcEth, bool bZeroTQ);
cy_en_ethif_status_t Cy_EthIf_ConfigPause (cy_pstc_eth_type_t pstcEth, uint16_t u16PauseQuanta);
cy_en_ethif_status_t Cy_EthIf_PhyRegWrite(cy_pstc_eth_type_t pstcEth, uint8_t u8RegNo, uint16_t u16Data, uint8_t u8PHYAddr);
uint32_t Cy_EthIf_PhyRegRead(cy_pstc_eth_type_t pstcEth, uint8_t u8RegNo, uint8_t u8PHYAddr);
cy_en_ethif_status_t Cy_EthIf_SetCopyAllFrames(cy_pstc_eth_type_t pstcEth, bool toBeEnabled);
cy_en_ethif_status_t Cy_EthIf_SetNoBroadCast(cy_pstc_eth_type_t pstcEth, bool rejectBC);
void Cy_EthIf_SetPhysAddr(cy_pstc_eth_type_t pstcEth, uint8_t * PhysAddrPtr);
void Cy_EthIf_GetPhysAddr(cy_pstc_eth_type_t pstcEth, uint8_t * PhysAddrPtr);
cy_en_ethif_status_t Cy_EthIf_ProvideTxBuffer(uint8_t* bufIdxPtr, uint8_t** bufPtr);
cy_en_ethif_status_t Cy_EthIf_TransmitTrigger(cy_pstc_eth_type_t pstcEth, uint8_t bufIdx, uint32_t payLoadLen, uint8_t queueIndex, uint16_t frameType, uint8_t* physAddrPtr);
cy_en_ethif_status_t Cy_EthIf_SetFilterAddress(cy_pstc_eth_type_t pstcEth, cy_en_eth_filter_num_t filterNo, const cy_stc_eth_filter_config_t* config);
cy_en_ethif_status_t Cy_EthIf_RemoveFilter(cy_pstc_eth_type_t pstcEth, cy_en_eth_filter_num_t filterNo);
cy_en_ethif_status_t Cy_EthIf_RemoveFilterAddress(cy_pstc_eth_type_t pstcEth, CEDI_MacAddress* macAddr, cy_en_eth_filter_num_t* removedFilterIdx);
cy_en_ethif_status_t Cy_EthIf_SetCorrectionTime(cy_pstc_eth_type_t pstcEth, int32_t timeOffset);
cy_en_ethif_status_t Cy_EthIf_GetIngressTimestamp(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcRetTmrValue, cy_en_ethif_ptp_event_type_t eventType);
cy_en_ethif_status_t Cy_EthIf_GetEgressTimestamp(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcRetTmrValue, cy_en_ethif_ptp_event_type_t eventType);
cy_en_ethif_status_t Cy_EthIf_SetTimerCompValue(cy_pstc_eth_type_t pstcEth, CEDI_1588TimerVal* pstcTmrValue);
cy_en_ethif_status_t Cy_EthIf_GetAndClearStats(cy_pstc_eth_type_t pstcEth, CEDI_Statistics* pstcStats);
cy_en_ethif_status_t Cy_EthIf_ClearStats(cy_pstc_eth_type_t pstcEth);
cy_en_ethif_status_t Cy_EthIf_SetPauseEnable(cy_pstc_eth_type_t pstcEth, bool enable);
cy_en_ethif_status_t Cy_EthIf_SetPfcPriorityBasedPauseEnable(cy_pstc_eth_type_t pstcEth, bool enable);

/** \} group_ethif_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_ETHIF_H */

/** \} cy_ethif */

/* [] END OF FILE */
