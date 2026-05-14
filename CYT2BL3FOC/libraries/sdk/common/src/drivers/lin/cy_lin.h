/***************************************************************************//**
* \file lin.h
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

/**
* \defgroup group_lin Local Interconnect Network (LIN)
* \{
* The Local Interconnect Network (LIN) block of TVII supports the 
* autonomous transfer of the LIN frame to reduce CPU processing. Supports
* master and slave functionality. 
*
* One LIN bit length corresponds to 16 clock cycles; that is, 16 oversample 
* counters are executed.
*
* This driver provides global LIN defines and API function definitions.
*
* \section group_lin_more_information More Information
* Refer to the technical reference manual (TRM) and the device datasheet.
*
* \section group_lin_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>6.6.0</td>
*     <td>API prototype correction, prefixed Cy_ for APIs.
*         Global macros prefixed with CY_LIN </td>
*     <td>Proto type alignment with other modules</td>
*   </tr>
* </table>
*
* \defgroup group_lin_macro             Macro
* \defgroup group_lin_functions         Functions
* \defgroup group_lin_data_structures   Data structures
* \defgroup group_lin_enums Enumerated  types
*/

#if !defined(CY_LIN_H)
#define CY_LIN_H

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
* \addtogroup group_lin_macros
* \{
*/

/*****************************************************************************/
/* Global pre-processor symbols/macros ('#define')                           */
/*****************************************************************************/
/** Driver major version */
#define CY_LIN_DRV_VERSION_MAJOR    1

/** Driver minor version */
#define CY_LIN_DRV_VERSION_MINOR    0

/** LIN driver ID */
#define CY_LIN_ID CY_PDL_DRV_ID(0x37u)

/** Maximum ID */
#define CY_LIN_ID_MAX                              (0x3Fu)
/** Maximum data length */
#define CY_LIN_DATA_LENGTH_MAX                     (8u)
/** Minimum data length */
#define CY_LIN_DATA_LENGTH_MIN                     (1u)
/** Maximum time out length */
#define CY_LIN_TIMEOUT_LENGTH_MAX                  (255u)
/** Minimum wakeup peroid value = 250usec*/
#define CY_LIN_WAKEUP_PERIOD_MIN                   (250u)
/** Maximum break_wakeup period bit length */
#define CY_LIN_BREAK_WAKEUP_LENGTH_BITS_MAX        (31u)
/** LIN Master minimum break filed detection length */
#define CY_LIN_MASTER_BREAK_FILED_LENGTH_MIN       (13u)

/**
* LIN command type: Specifies the parameter values passed to LIN command API
*/
#define CY_LIN_CMD_TX_HEADER                   (LIN_CH_CMD_TX_HEADER_Msk)
#define CY_LIN_CMD_TX_RESPONSE                 (LIN_CH_CMD_TX_RESPONSE_Msk)
#define CY_LIN_CMD_RX_RESPONSE                 (LIN_CH_CMD_RX_RESPONSE_Msk)
#define CY_LIN_CMD_TX_HEADER_TX_RESPONSE       (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_TX_RESPONSE_Msk)
#define CY_LIN_CMD_TX_HEADER_RX_RESPONSE       (LIN_CH_CMD_TX_HEADER_Msk | LIN_CH_CMD_RX_RESPONSE_Msk)
#define CY_LIN_CMD_RX_HEADER_RX_RESPONSE       (LIN_CH_CMD_RX_HEADER_Msk | LIN_CH_CMD_RX_RESPONSE_Msk)
#define CY_LIN_CMD_TX_WAKEUP                   (LIN_CH_CMD_TX_WAKEUP_Msk)
/** \} group_lin_cmd_macro */

/**
* LIN ALL error mask: Specifies the mask value for interrupt status/mask
*/
#define CY_LIN_INTR_TX_HEADER_DONE             (LIN_CH_INTR_TX_HEADER_DONE_Msk)
#define CY_LIN_INTR_TX_RESPONSE_DONE           (LIN_CH_INTR_TX_RESPONSE_DONE_Msk)
#define CY_LIN_INTR_TX_WAKEUP_DONE             (LIN_CH_INTR_TX_WAKEUP_DONE_Msk)
#define CY_LIN_INTR_RX_HEADER_DONE             (LIN_CH_INTR_RX_HEADER_DONE_Msk)
#define CY_LIN_INTR_RX_RESPONSE_DONE           (LIN_CH_INTR_RX_RESPONSE_DONE_Msk)
#define CY_LIN_INTR_RX_WAKEUP_DONE             (LIN_CH_INTR_RX_WAKEUP_DONE_Msk)
#define CY_LIN_INTR_RX_HEADER_SYNC_DONE        (LIN_CH_INTR_RX_HEADER_SYNC_DONE_Msk)
#define CY_LIN_INTR_RX_NOISE_DETECT            (LIN_CH_INTR_RX_NOISE_DETECT_Msk)
#define CY_LIN_INTR_TIMEOUT                    (LIN_CH_INTR_TIMEOUT_Msk)
#define CY_LIN_INTR_TX_HEADER_BIT_ERROR        (LIN_CH_INTR_TX_HEADER_BIT_ERROR_Msk)
#define CY_LIN_INTR_TX_RESPONSE_BIT_ERROR      (LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Msk)
#define CY_LIN_INTR_RX_HEADER_FRAME_ERROR      (LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Msk)
#define CY_LIN_INTR_RX_HEADER_SYNC_ERROR       (LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Msk)
#define CY_LIN_INTR_RX_HEADER_PARITY_ERROR     (LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Msk)
#define CY_LIN_INTR_RX_RESPONSE_FRAME_ERROR    (LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Msk)
#define CY_LIN_INTR_RX_RESPONSE_CHECKSUM_ERROR (LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Msk)
#define CY_LIN_INTR_ALL_ERROR_MASK_SLAVE       (LIN_CH_INTR_RX_NOISE_DETECT_Msk           |\
                                             	LIN_CH_INTR_TIMEOUT_Msk                   |\
                                             	LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Msk     |\
                                             	LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Msk     |\
                                             	LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Msk      |\
                                             	LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Msk    |\
                                             	LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Msk   |\
                                             	LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Msk)
#define CY_LIN_INTR_ALL_ERROR_MASK_MASTER      (CY_LIN_INTR_ALL_ERROR_MASK_SLAVE | LIN_CH_INTR_TX_HEADER_BIT_ERROR_Msk)

/** \} group_lin_macros */

/*****************************************************************************/
/* Global type definitions ('typedef')                                       */
/*****************************************************************************/

/**
* \addtogroup group_lin_enums
* \{
*/

/** LIN API status definition */
typedef enum
{
    CY_LIN_SUCCESS   = 0x00u,                                     /**< Returned successful */
    CY_LIN_BAD_PARAM = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x01u,   /**< Bad parameter was passed */
    CY_LIN_BUSY      = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x02u,   /**< Change settings while tx/rx on-going */
    CY_LIN_FRAME_NOT_RECEIVED = CY_LIN_ID | CY_PDL_STATUS_ERROR | 0x03u,   /**< No frame received */
} cy_en_lin_status_t;

/**
* LIN break delimiter length.
* This configuration is effective only when corresponding channel = master mode.
*/
typedef enum {
    LinBreakDelimiterLength1bits = 0,   /**< 1-bit length */
    LinBreakDelimiterLength2bits = 1,   /**< 2-bit length */
    LinBreakDelimiterLength3bits = 2,   /**< 3-bit length */
    LinBreakDelimiterLength4bits = 3    /**< 4-bit length */
} en_lin_break_delimiter_length_t;

/**
* Stop bit selection.
*/
typedef enum
{
    LinOneStopBit           = 1,    /**< 1 stop bit */
    LinTwoStopBit           = 3     /**< 2 stop bits */
} en_lin_stopbit_t;

/**
* Checksum type selection.
*/
typedef enum
{
    LinChecksumTypeNormal = 0,      /**< Normal (classic) checksum */
    LinChecksumTypeExtended = 1     /**< Extended (enhanced) checksum */
} en_lin_checksum_type_t;

/**
* Timeout type selection.
*/
typedef enum
{
    LinTimeoutNone = 0,                     /**< No timeout */
    LinTimeoutFromBreakToChecksum = 1,      /**< Frame mode: from the start of break field to checksum field STOP bits */
    LinTimeoutFromBreakToPID = 2,           /**< Frame header mode: detects timeout from the start of break field to PID field STOP bits */
    LinTimeoutFromResponseToChecksum = 3    /**< Frame response mode: detects timeout from the first data field START bit to checksum field STOP bits. */
} en_lin_timeout_sel_type_t;

/** \} group_lin_enums */

/**
* \addtogroup group_lin_data_structures
* \{
*/

/**
 *****************************************************************************
 ** \brief LIN configuration
 **        This settings are per LIN instance.
 *****************************************************************************/
typedef struct stc_lin_config
{
    /**< If TRUE, corresponding channel = master mode, If FALSE, slave mode. */
    bool                            bMasterMode;
    /**< If TRUE, corresponding LIN channel transceiver is enabled automatically,
     ** If FALSE, firmware has to handle the transceiver enable signal manually */
    bool                            bLinTransceiverAutoEnable;
    /**< Break field length. */
    uint8_t                         u8BreakFieldLength;
    /**< Break delimiter length. See #en_lin_break_delimiter_length_t */
    en_lin_break_delimiter_length_t enBreakDelimiterLength;
    /**< Stop bit length. See #en_lin_stopbit_t. */
    en_lin_stopbit_t                enStopBit;
    /**< If TRUE, lin_rx_in filter operates.
     *  Median 3 operates on the last three "lin_rx_in" values.
     *  The sequences '000', '001', '010' and '100' result in a filtered value '0'.
     *  The sequences '111', '110', '101' and '011' result in a filtered value '1'.
     */
    bool                            bFilterEnable;
}stc_lin_config_t;

/** \} group_lin_data_structures */

/**
* \addtogroup group_lin_functions
* \{
*/

/*****************************************************************************/
/* Global variable declarations ('extern', definition in C source)           */
/*****************************************************************************/


/*****************************************************************************/
/* Global function prototypes ('extern', definition in C source)             */
/*****************************************************************************/

cy_en_lin_status_t Cy_Lin_DeInit( volatile stc_LIN_CH_t* pstcLin );
cy_en_lin_status_t Cy_Lin_Init( volatile stc_LIN_CH_t* pstcLin, const stc_lin_config_t *pstcConfig);
cy_en_lin_status_t Cy_Lin_ReadData( volatile stc_LIN_CH_t* pstcLin, uint8_t *u8Data);
cy_en_lin_status_t Cy_Lin_WriteData( volatile stc_LIN_CH_t* pstcLin, const uint8_t *au8Data,
                                  uint8_t u8DataLength );
cy_en_lin_status_t Cy_Lin_Enable(volatile stc_LIN_CH_t* pstcLin);
cy_en_lin_status_t Cy_Lin_Disable(volatile stc_LIN_CH_t* pstcLin);
cy_en_lin_status_t Cy_Lin_SetBreakWakeupFieldLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length);
cy_en_lin_status_t Cy_Lin_SetDataLength(volatile stc_LIN_CH_t* pstcLin, uint8_t length);
cy_en_lin_status_t Cy_Lin_SetChecksumType(volatile stc_LIN_CH_t* pstcLin, en_lin_checksum_type_t type);
cy_en_lin_status_t Cy_Lin_SetCmd(volatile stc_LIN_CH_t* pstcLin, uint32_t command);

cy_en_lin_status_t Cy_Lin_SetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t id);
cy_en_lin_status_t Cy_Lin_GetHeader(volatile stc_LIN_CH_t* pstcLin, uint8_t *id, uint8_t *parity);
cy_en_lin_status_t Cy_Lin_SetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t mask);
cy_en_lin_status_t Cy_Lin_GetInterruptMask(volatile stc_LIN_CH_t* pstcLin, uint32_t *mask);
cy_en_lin_status_t Cy_Lin_GetInterruptMaskedStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);
cy_en_lin_status_t Cy_Lin_GetInterruptStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);
cy_en_lin_status_t Cy_Lin_ClearInterrupt(volatile stc_LIN_CH_t* pstcLin, uint32_t mask);
cy_en_lin_status_t Cy_Lin_GetStatus(volatile stc_LIN_CH_t* pstcLin, uint32_t *status);

/** \} group_lin_functions */

#ifdef __cplusplus
}
#endif

#endif /* __CY_LIN_H__ */

/*****************************************************************************/
/* EOF (not truncated)                                                       */
/*****************************************************************************/
