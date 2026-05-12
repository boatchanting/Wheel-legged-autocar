/***************************************************************************//**
* \file cxpi.h
* \version 1.0
*
* \brief
* Provides an API declaration of the CXPI driver
*
* \note
* Generator version: draft verision
* Database revision: https://svn.design.cypress.com:18080/svn/chips/trunk/s40/MXS40/Platform/MXS40-SAS/Configuration/MXS40-IP-CXPI.xlsm
 [rev.185188]
*
********************************************************************************
* \copyright
* Copyright 2017-2018, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_cxpi Clock eXtension Peripheral Interface (CXPI)
* \{
* This driver provides global CXPI defines and API function definitions.
*
* \defgroup group_cxpi_macros Macro
* \defgroup group_cxpi_functions Functions
* \defgroup group_cxpi_data_structures Data structures
* \defgroup group_cxpi_enums Enumerated types
*/

#if !defined(CY_CXPI_H)
#define CY_CXPI_H

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_cxpi_macros
* \{
*/

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/** Driver major version */
#define CY_CXPI_DRV_VERSION_MAJOR    1

/** Driver minor version */
#define CY_CXPI_DRV_VERSION_MINOR    0

/** CXPI driver ID */
#define CY_CXPI_ID CY_PDL_DRV_ID(0x37u)  //need modified

/** \} group_cxpi_macros */

/**
* \addtogroup group_cxpi_enums
* \{
*/

/** CXPI API status definition */
typedef enum
{
    CY_CXPI_SUCCESS   = 0x00u,                                     /**< Returned successful */
    //CY_CXPI_BAD_PARAM = CY_CXPI_ID | CY_PDL_STATUS_ERROR | 0x01u,   /**< Bad parameter was passed */
    CY_CXPI_BAD_PARAM = 0x01u,
    //CY_CXPI_BUSY      = CY_CXPI_ID | CY_PDL_STATUS_ERROR | 0x02u,   /**< Change settings while tx/rx on-going */
    CY_CXPI_BUSY = 0x02u,
    //CY_CXPI_FRAME_NOT_RECEIVED = CY_CXPI_ID | CY_PDL_STATUS_ERROR | 0x03u,   /**< No frame received */
    CY_CXPI_FRAME_NOT_RECEIVED = 0x03u,
} cy_en_cxpi_status_t;

/** \} group_cxpi_enums */

/**
* \addtogroup group_cxpi_macros
* \{
*/

/** Maximum reversed ID */
#define CXPI_REV_ID_MAX                         (0x79)
/** Maximum data length */
#define CXPI_DATA_LENGTH_MAX                    (255u)
/** Minimum data length */
#define CXPI_DATA_LENGTH_MIN                    (1u)
/** Maximum counter = 3 */
#define CXPI_COUNTER_MAX                        (3u)
/** Maximum time out = 10Tbits*/
#define CXPI_TIMEOUT_MAX                        (10u)

/** Maximum wake up pulse low period = 50Tbits*/
#define CXPI_T_WAKEUP_LENGTH_MAX                (50u)
/** Minimum wake up pulse low period = 1Tbits*/
#define CXPI_T_WAKEUP_LENGTH_MIN                (1u)

/** Maximum low count for logic "1" = 400 Tbits*/
#define CXPI_T_LOW1_MAX                         (400u)
/** Minimum low count for logic "1" = 1 Tbits*/
#define CXPI_T_LOW1_MIN                         (1u)
/** Maximim low count for logic "0" = 400 Tbits*/
#define CXPI_T_LOW0_MAX                         (400u)
/** Minimum low count for logic "0" = 1 Tbits*/
#define CXPI_T_LOW0_MIN                         (1u)
/** Maximim offset value = 400 Tbits*/
#define CXPI_T_OFFSET_MAX                       (400u)
/** Minimum offset value = 1 Tbits*/
#define CXPI_T_OFFSET_MIN                       (1u)
/** Maximum retries after arbitration lost = 3 times */
#define CXPI_RETRY_MAX                          (3u)
/**Maximum number of entries of TX_FIFO */
#define CXPI_TX_FIFO_ENTRIESNUM_MAX             (16u)
/**Maximum number of entries of RX_FIFO */
#define CXPI_RX_FIFO_ENTRIESNUM_MAX             (16u)
/** Hardware TX_FIFO size */
#define CXPI_TX_FIFO_SIZE                       (16u)
/** Hardware RX_FIFO size */
#define CXPI_RX_FIFO_SIZE                       (16u)

#define CXPI_PTYPE                              (0x0)

#define CXPI_SLEEP_MODE                         (0x0)
#define CXPI_STANDBY_MODE                       (0x1)
#define CXPI_NORMAL_MODE                        (0x2)

/**
* Specifies the parameter values passed to CXPI command API
*/
#define CXPI_CMD_TX_HEADER                   (CXPI_CH_CMD_TX_HEADER_Msk)
#define CXPI_CMD_RX_HEADER                   (CXPI_CH_CMD_RX_HEADER_Msk)
#define CXPI_CMD_TX_HEADER_RX_HEADER         (CXPI_CH_CMD_TX_HEADER_Msk | CXPI_CH_CMD_RX_HEADER_Msk)
#define CXPI_CMD_TX_RESPONSE                 (CXPI_CH_CMD_TX_RESPONSE_Msk)
#define CXPI_CMD_RX_RESPONSE                 (CXPI_CH_CMD_RX_RESPONSE_Msk)
#define CXPI_CMD_TX_HEADER_TX_RESPONSE       (CXPI_CH_CMD_TX_HEADER_Msk | CXPI_CH_CMD_TX_RESPONSE_Msk)
#define CXPI_CMD_TX_HEADER_RX_RESPONSE       (CXPI_CH_CMD_TX_HEADER_Msk | CXPI_CH_CMD_RX_RESPONSE_Msk)
#define CXPI_CMD_RX_HEADER_RX_RESPONSE       (CXPI_CH_CMD_RX_HEADER_Msk | CXPI_CH_CMD_RX_RESPONSE_Msk)
#define CXPI_CMD_TX_WAKEUP_PULSE             (CXPI_CH_CMD_TX_WAKEUP_PULSE_Msk)
#define CXPI_CMD_SLEEP                       (CXPI_CH_CMD_SLEEP_Msk)
#define CXPI_CMD_WAKE_TO_STANDBY             (CXPI_CH_CMD_WAKE_TO_STANDBY_Msk)
#define CXPI_CMD_IFS_WAIT                    (CXPI_CH_CMD_IFS_WAIT_Msk)
/** \} group_cxpi_cmd_macro */

/**
* Specifies the mask value for interrupt status/mask
*/
#define CXPI_INTR_TX_HEADER_DONE            (CXPI_CH_INTR_TX_HEADER_DONE_Msk)
#define CXPI_INTR_TX_RESPONSE_DONE          (CXPI_CH_INTR_TX_RESPONSE_DONE_Msk)
#define CXPI_INTR_TX_WAKEUP_DONE            (CXPI_CH_INTR_TX_WAKEUP_DONE_Msk)
#define CXPI_INTR_TX_FIFO_TRIGGER           (CXPI_CH_INTR_TX_FIFO_TRIGGER_Msk)
#define CXPI_INTR_RX_HEADER_DONE            (CXPI_CH_INTR_RX_HEADER_DONE_Msk)
#define CXPI_INTR_RX_RESPONSE_DONE          (CXPI_CH_INTR_RX_RESPONSE_DONE_Msk)
#define CXPI_INTR_RX_WAKEUP_DETECT          (CXPI_CH_INTR_RX_WAKEUP_DETECT_Msk)
#define CXPI_INTR_RX_FIFO_TRIGGER           (CXPI_CH_INTR_RX_FIFO_TRIGGER_Msk)
#define CXPI_INTR_RX_HEADER_PID_DONE        (CXPI_CH_INTR_RX_HEADER_PID_DONE_Msk)
#define CXPI_INTR_TXRX_COMPLETE             (CXPI_CH_INTR_TXRX_COMPLETE_Msk)
#define CXPI_INTR_TIMEOUT                   (CXPI_CH_INTR_TIMEOUT_Msk)
#define CXPI_INTR_TX_HEADER_ARB_LOST        (CXPI_CH_INTR_TX_HEADER_ARB_LOST_Msk)
#define CXPI_INTR_TX_BIT_ERROR              (CXPI_CH_INTR_TX_BIT_ERROR_Msk)
#define CXPI_INTR_RX_CRC_ERROR              (CXPI_CH_INTR_RX_CRC_ERROR_Msk)
#define CXPI_INTR_RX_HEADER_PARITY_ERROR    (CXPI_CH_INTR_RX_HEADER_PARITY_ERROR_Msk)
#define CXPI_INTR_RX_DATA_LENGTH_ERROR      (CXPI_CH_INTR_RX_DATA_LENGTH_ERROR_Msk)
#define CXPI_INTR_TX_DATA_LENGTH_ERROR      (CXPI_CH_INTR_TX_DATA_LENGTH_ERROR_Msk)
#define CXPI_INTR_RX_OVERFLOW_ERROR         (CXPI_CH_INTR_RX_OVERFLOW_ERROR_Msk)
#define CXPI_INTR_TX_OVERFLOW_ERROR         (CXPI_CH_INTR_TX_OVERFLOW_ERROR_Msk)
#define CXPI_INTR_RX_UNDERFLOW_ERROR        (CXPI_CH_INTR_RX_UNDERFLOW_ERROR_Msk)
#define CXPI_INTR_TX_UNDERFLOW_ERROR        (CXPI_CH_INTR_TX_UNDERFLOW_ERROR_Msk)
#define CXPI_INTR_RX_FRAME_ERROR            (CXPI_CH_INTR_RX_FRAME_ERROR_Msk)
#define CXPI_INTR_TX_FRAME_ERROR            (CXPI_CH_INTR_TX_FRAME_ERROR_Msk)


#define CXPI_INTR_ALL_ERROR_MASK            (CXPI_CH_INTR_TX_BIT_ERROR_Msk           |\
                                             CXPI_CH_INTR_RX_CRC_ERROR_Msk           |\
                                             CXPI_CH_INTR_RX_HEADER_PARITY_ERROR_Msk |\
                                             CXPI_CH_INTR_RX_DATA_LENGTH_ERROR_Msk   |\
                                             CXPI_CH_INTR_TX_DATA_LENGTH_ERROR_Msk   |\
                                             CXPI_CH_INTR_RX_OVERFLOW_ERROR_Msk      |\
                                             CXPI_CH_INTR_TX_OVERFLOW_ERROR_Msk      |\
                                             CXPI_CH_INTR_RX_UNDERFLOW_ERROR_Msk     |\
                                             CXPI_CH_INTR_TX_UNDERFLOW_ERROR_Msk     |\
                                             CXPI_CH_INTR_RX_FRAME_ERROR_Msk         |\
                                             CXPI_CH_INTR_TX_FRAME_ERROR_Msk)
                                             
/** \} group_cxpi_macros */

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/

/**
* \addtogroup group_cxpi_enums
* \{
*/

/**
 *****************************************************************************
 ** \brief CXPI mode of operation
 **
 ** This configuration is effective only when corresponding channel = master mode.
 *****************************************************************************/
typedef enum en_cxpi_mode{
    CXPI_NRZ_Mode = 0,                  /*!< NRZ mode */
    CXPI_PWM_Mode = 1,                  /*!< PWM mode */
}en_cxpi_mode_t;

typedef enum en_bit_error_ignore{
    MESSAGE_TRANFER_ABORTED = 0,                    /*!< Message transfer is aborted. */
    MESSAGE_TRANFER_NOT_ABORTED = 1,                /*!< Message transfer is NOT aborted. */
}en_bit_error_ignore_t;

typedef enum en_master_mode{
    CXPI_SLAVE_MODE = 0,                   /*!< Indicates CXPI as slave node. */
    CXPI_MASTER_MODE = 1,                /*!< Indicates CXPI as master node. */
}en_master_mode_t;
/**
 *****************************************************************************
 ** \brief number of retries after arbitration lost
 **
 *****************************************************************************/
typedef enum en_cxpi_retries
{
    CxpiNoRetries = 0,              /*!< no retries */
    CxpiRetriesOneTime = 1,         /*!< 1 retries */
    CxpiRetriesTwoTime = 2,         /*!< 2 retries */
    CxpiRetriesThreeTime = 3        /*!< 3 retries */
}en_cxpi_retries_t;

/**
 *****************************************************************************
 ** \brief timeout selection.
 **
 *****************************************************************************/
typedef enum en_cxpi_timeout_sel
{
    CxpiTimeoutDisabled = 0,        /*!< Timeout check is disabled */
    CxpiTimeoutCheckHdrRes = 1,     /*!< Timeout check is enabled and HW will refer to TIMEOUT_LENGTH as number of Tbits allowed between header and response */
    CxpiTimeoutCheckAllByte = 2,    /*!< Timeout check is enabled to check all bytes within a message frame to be space within TIMEOUT_LENGTH bit time */
}en_cxpi_timeout_sel_t;

/* CXPI state type */
typedef enum
{
    CXPI_STATE_TXRX_PENDING,
    CXPI_STATE_TX_PTYPE,
    CXPI_STATE_TX_HDR_RX_RESP,
    CXPI_STATE_TX_HDR_TX_RESP,
    CXPI_STATE_RX_HDR_TX_RESP,
    CXPI_STATE_RX_HDR_RX_RESP,
} en_cxpi_state_t;

/* Arbitration state type */
typedef enum
{
    CXPI_ARB_PENDING,
    CXPI_ARB_LOST_TX_RESP,
    CXPI_ARB_LOST_RX_RESP,
} en_cxpi_arb_state_t;

/** \} group_cxpi_enums */

/**
* \addtogroup group_cxpi_data_structures
* \{
*/

/**
 *****************************************************************************
 ** \brief CXPI_CH_CLT0 configuration for modes, offset, or master/slave
 **
 *****************************************************************************/
typedef struct stc_cxpi_ch_ctl0_config
{
    /** Mode of operation */
    en_cxpi_mode_t                  enMode;
    /** If TRUE, corresponding CXPI channel transceiver is enabled automatically, 
     *  the TX_RX_STATUS.EN_OUT field is controlled by HW.
     *  If FALSE, firmware has to handle the transceiver enable signal manually 
     */
    bool                            bCxpiTransceiverAutoEnable;
    /** Receive PID Zero Check Enable
     *  If TRUE, If received PID[6:0] = 0 and PID[7]=1'b1, HW does not clear CMD.RX_HEADER 
     *  and will anticipate receiving header again. Should be TRUE where polling is enabled 
     *  and CXPI controller is configured as slave.
     *  If FALSE, no action if received PID[6:0] = 0 and PID[7]=1'b1. 
     */
     bool                           bRxPIDZeroAutoCheckEnable;
    /** If TRUE, cxpi_rx_in filter operates.
     *  Median 3 operates on the last three "cxpi_rx_in" values.
     *  The sequences '000', '001', '010' and '100' result in a filtered value '0'.
     *  The sequences '111', '110', '101' and '011' result in a filtered value '1'.
     */
    bool                            bFilterEnable;
    /** Inter Frame Space in bit periods */
    uint8_t                         u8IFS;
    /** Inter Byte Space in bit periods */
    uint8_t                         u8IBS;
    /** Specifies behavior on a detected bit error during header or response transmission.
     * If TRUE, message transfer is NOT aborted.
     * If FALSE, message transfer is NOT aborted.
     */
    en_bit_error_ignore_t           enBitErrorIgnore;
    /** CXPI master mode.
     * If TRUE, use CXPI as master node.
     * If FALSE, use CXPI as slave node.
     */
    en_master_mode_t                enCxpiMasterMode;     
}stc_cxpi_ch_ctl0_config_t;

/**
 *****************************************************************************
 ** \brief CXPI_CH_CLT1 configuration for timing controls at Data link layer.
 **
 *****************************************************************************/
typedef struct stc_cxpi_ch_ctl1_config
{
    /** Low count for logic 1.
     * The count value here indicates the number of clocks per clk_cxpi_ch 
     * to drive a '0' at CXPI bus before releasing it to indicate a logical '1'. */
    uint16_t                        u16TLow1;
    /** Low count for logic 0. 
     * The count value here indicates the number of clocks per clk_cxpi_ch 
     * to drive a '0' at CXPI bus before releasing it to indicate a logical '1'. */
    uint16_t                        u16TLow0;
    /** The value of offset that is used for sampling the 'rx'. */
    uint16_t                        u16TOffset;    
}stc_cxpi_ch_ctl1_config_t;

/**
 *****************************************************************************
 ** \brief CXPI_CH_CLT2 configuration for retry, wakeup pulse length, time out.
 **
 *****************************************************************************/
typedef struct stc_cxpi_ch_ctl2_config
{
    /** Number of retries after arbitration lost. */
    en_cxpi_retries_t               enRetriesNumber;
    /** Specifies the wake up pulse low period in Tbits that is transmitted during Standby mode. */
    uint8_t                         u8TWakeUpLength;
    /** Timeout Length (in Tbits). Specifies the number of Tbits to exceed timeout between frame bytes 
     * within a message frame. CXPI spec states that the maximum allowed inter byte space (IBS) is 9Tbits 
     */
    uint8_t                         u8TimeoutLength;
    /** Timeout Select */
    en_cxpi_timeout_sel_t           enTimeoutSel;
}stc_cxpi_ch_ctl2_config_t;


typedef struct stc_cxpi_frame_info{
    uint8_t datalength;
    uint8_t SleepInd;
    uint8_t WakeupInd;
    uint8_t counter;
} stc_cxpi_frame_info_t;

/* CXPI frame configuration */

typedef struct cxpi_response
{
    stc_cxpi_frame_info_t frameIF;
    uint8_t dataBuffer[CXPI_DATA_LENGTH_MAX];
} cxpi_response_t;

typedef struct cxpi_frame_context
{
    uint8_t id;
    cxpi_response_t response;
} cxpi_frame_context_t;

typedef struct stc_CXPI_context
{
    volatile stc_CXPI_CH_t *pstcInstanceCxpi;
    
    uint8_t* pAcceptedPIDList;
    uint32_t numberOfAcceptedPID;
    
    uint8_t    *rxBuf;                /**< The pointer to the receive buffer */
    uint8_t volatile rxBuffCount;    /**< The receive buffer size */
    uint8_t volatile rxBufIdx;     /**< The current location in the receive buffer */
    
    uint8_t    *txBuf;                /**< The pointer to the transmit buffer */
    uint8_t volatile txBuffCount;    /**< The transmit buffer size */
    uint8_t volatile txBufIdx;     /**< The current location in the transmit buffer */    
    
    en_cxpi_state_t volatile state;
    en_cxpi_arb_state_t volatile arbState;
    
    uint8_t maximumRetries;
    uint8_t volatile retriesCount;
 
    uint8_t transmittingFrameId;
    cxpi_response_t* pTransmittingFrameResponse;
    cxpi_frame_context_t transmittingFrame;
    cxpi_frame_context_t receivedFrame;
    
    void (*pNotifyTrasmittedWakeupPulse)(void);
    void (*pNotifyRXFallingEdgeDetected)(void);
    void (*pNotifyError)(uint32_t);
    void (*pNotifySentPTYPE)(void);
    void (*pNotifyReceivedPTYPE)(void);
    void (*pNotifyReceivedPID)(uint8_t);
    void (*pNotifyReceivedResponse)(cxpi_frame_context_t*);
    void (*pNotifyArbitrationLost)(void);
}stc_CXPI_context_t;


/**
 *****************************************************************************
 ** \brief CXPI configuration
 **        This settings are per CXPI instance.
 *****************************************************************************/
typedef struct stc_cxpi_config
{
    stc_cxpi_ch_ctl0_config_t       stcCxpiChCtl0Config;
    stc_cxpi_ch_ctl1_config_t       stcCxpiChCtl1Config;    
    stc_cxpi_ch_ctl2_config_t       stcCxpiChCtl2Config;
    uint8_t* pAcceptedPIDList;
    uint32_t numberOfAcceptedPID;
    uint32_t maximumRetries;
    void (*pNotifyTrasmittedWakeupPulse)(void);
    void (*pNotifyRXFallingEdgeDetected)(void);
    void (*pNotifyError)(uint32_t);
    void (*pNotifySentPTYPE)(void);
    void (*pNotifyReceivedPTYPE)(void);
    void (*pNotifyReceivedPID)(uint8_t);
    void (*pNotifyReceivedResponse)(cxpi_frame_context_t*);
    void (*pNotifyArbitrationLost)(void);
}stc_cxpi_config_t;
    
/** \} group_cxpi_data_structures */

/**
 *****************************************************************************
 ** \brief  CXPI context structure
 **
 *****************************************************************************/

/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/

/**
* \addtogroup group_cxpi_functions
* \{
*/

cy_en_cxpi_status_t Cy_CXPI_DeInit( volatile stc_CXPI_CH_t *pstcCXPI ); //done
cy_en_cxpi_status_t Cy_CXPI_Init( volatile stc_CXPI_CH_t *pstcCXPI, const stc_cxpi_config_t *pstcConfig, stc_CXPI_context_t *pCxpiCtx); //done w/o comment

cy_en_cxpi_status_t Cy_CXPI_Enable(volatile stc_CXPI_CH_t *pstcCXPI);  //done
cy_en_cxpi_status_t Cy_CXPI_Disable(volatile stc_CXPI_CH_t *pstcCXPI); //done

cy_en_cxpi_status_t Cy_CXPI_SetCmd(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t command);

cy_en_cxpi_status_t Cy_CXPI_GetStatus(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *status); //done

cy_en_cxpi_status_t Cy_CXPI_SetPID(volatile stc_CXPI_CH_t *pstcCXPI, uint8_t id); //done
cy_en_cxpi_status_t Cy_CXPI_GetPID(volatile stc_CXPI_CH_t *pstcCXPI, uint8_t* id, uint8_t* parity); //done

cy_en_cxpi_status_t Cy_CXPI_SetFI(volatile stc_CXPI_CH_t *pstcCXPI, stc_cxpi_frame_info_t fi);  //done
cy_en_cxpi_status_t Cy_CXPI_GetFI(volatile stc_CXPI_CH_t *pstcCXPI, stc_cxpi_frame_info_t *fi);  //done

cy_en_cxpi_status_t Cy_CXPI_GetCRC(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *CRCdata); //done

cy_en_cxpi_status_t Cy_CXPI_SetTriggerLevelTxFIFO(volatile stc_CXPI_CH_t *pstcCXPI, uint8_t triggerLevel); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_ClearTxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_FreezeTxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_UnfreezeTxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment

cy_en_cxpi_status_t Cy_CXPI_GetTxFIFOStatus(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *usedEnntries, uint32_t *availEntries); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_WriteTxFIFO( volatile stc_CXPI_CH_t *pstcCXPI, const uint8_t data);
                                  
cy_en_cxpi_status_t Cy_CXPI_SetTriggerLevelRxFIFO(volatile stc_CXPI_CH_t *pstcCXPI, uint8_t triggerLevel); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_ClearRxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_FreezeRxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_UnfreezeRxFIFO(volatile stc_CXPI_CH_t *pstcCXPI); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_GetRxFIFOStatus(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *usedEnntries, uint32_t *availEntries); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_ReadRxFIFO( volatile stc_CXPI_CH_t *pstcCXPI, uint8_t *data); //done w/o comment
cy_en_cxpi_status_t Cy_CXPI_SilentReadRxFIFO(volatile stc_CXPI_CH_t *pstcCXPI, uint8_t *data); //done w/o comment

cy_en_cxpi_status_t Cy_CXPI_SetInterruptMask(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t mask); //done
cy_en_cxpi_status_t Cy_CXPI_GetInterruptMask(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *mask); //done
cy_en_cxpi_status_t Cy_CXPI_GetInterruptMaskedStatus(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *status); //done

cy_en_cxpi_status_t Cy_CXPI_GetInterruptStatus(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t *status); //done
cy_en_cxpi_status_t Cy_CXPI_ClearInterrupt(volatile stc_CXPI_CH_t *pstcCXPI, uint32_t mask); //done
cy_en_cxpi_status_t Cy_CXPI_SendWakeupPulse(volatile stc_CXPI_CH_t  *pstcCXPI, uint32_t tWakeupLength);
cy_en_cxpi_status_t Cy_CXPI_IFSWait(volatile stc_CXPI_CH_t  *pstcCXPI);
cy_en_cxpi_status_t Cy_CXPI_SetMode(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx, uint32_t mode);
cy_en_cxpi_status_t Cy_CXPI_SendPTYPE(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx);
cy_en_cxpi_status_t Cy_CXPI_SendPID(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx, uint8_t frameID, bool withResponse, cxpi_response_t *pResponse, bool IFSwait);
cy_en_cxpi_status_t Cy_CXPI_SendResp(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx, cxpi_response_t *pResponse);
cy_en_cxpi_status_t Cy_CXPI_Interrupt(volatile stc_CXPI_CH_t *pstcCXPI, stc_CXPI_context_t *cxpiCtx);

void HeaderReceptionOperation(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx);
void TXRXCompletionOperation(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx);
void TXHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx);
void RXHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx);
void RXResponseDoneHandler(volatile stc_CXPI_CH_t* pstcCXPI, stc_CXPI_context_t* pCxpiCtx);

/** \} group_cxpi_functions */

#ifdef __cplusplus
}
#endif

#endif /* __CY_CXPI_H__ */
/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
