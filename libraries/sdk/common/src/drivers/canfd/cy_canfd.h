/***************************************************************************//**
* \file cy_canfd.h
* \version 1.0
*
* \brief
* Provides an API declaration of the CAN FD driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
/**
* \defgroup CanFDGroup CAN with Flexible Data rate (CAN FD)
* \{
* \brief This section describes the interface for the CAN with Flexible Data rate.
*
* Provided functions of CAN FD module:
* - Cy_CANFD_Init() (Initialization of CAN FD module)
* - Cy_CANFD_UpdateAndTransmitMsgBuffer() (Used to update the data in a message buffer and to transmit a message)
* - Cy_CANFD_DeInit() (Deinitialization of CAN FD module)
* - Cy_CANFD_GetTxBufferStatus() (Used to retreive the Tx message buffer status)
*
* \brief How to use the CAN FD module:
* First, to initialise a CAN FD module, Cy_CANFD_Init() must be called. 
* Initialize the CAN FD module with the configuration parameters, and start 
* the CAN FD communication.
* The callback functions are optional, but recommended, otherwise there is no 
* report to the API in case of any error and transmission/reception. 
*
* For transmitting CAN FD messages the function CanFD_UpdateAndTransmitMsgBuffer()
* has to be used. The buffer status can be retreived by Cy_CANFD_GetTxBufferStatus().
* It is possible to set a callback function which will be notified whenever a
* message has been received. 
*
* \description
* Specification of sample software is as follows:
*   - Selectable CAN-FD mode or CAN mode.
*   - Configurable CAN clock
*   - Configurable CAN Bit rate
*   - Configurable CAN-FD Bit rate
*   - Configurable Standard ID filter/s
*   - Configurable Extended Message ID Filter/s
*   - Tx/Rx Element Size : 8 byte (CAN mode)
*     Tx/Rx Element Size : 64 byte (CAN-FD mode)
*   - Tx/Rx FIFO : Not used
*   - Interrupt : DRXE=1 (Message stored to Dedicated Rx Buffer)
*                 (Inactive other)
*
* Send data of ID:0x200 periodically in the main().
* Echoback by incrementing the ID of the Rx data received by
* the receiving interrupt.(CanFDIrqHandler() --- CanFDCallbackRx())
* 
* \note Following settings are not implemented.
*         - Using Tx/warning/error interrupt
*         - Using Tx/Rx FIFO
*
*/

#if !defined(CY_CANFD_H)
#define CY_CANFD_H

#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"
#include "canfd/cy_canfd_config.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** \addtogroup group_canfd_macro
* \{
*/

/** Driver major version */
#define CY_CANFD_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_CANFD_DRV_VERSION_MINOR       0

/** CANFD driver ID */
#define CY_CANFD_ID CY_PDL_DRV_ID(0x18u)

/** \} group_canfd_macro */


/***************************************
*        Constants
***************************************/

/** Size of Word data bytes in a receive or transmit operation (16word = 64byte) */
#define CY_CANFD_MESSAGE_DATA_BUFFER_SIZE     16

/** Rx FIFO Numbers */
#define CY_CANFD_RX_FIFO0   0
#define CY_CANFD_RX_FIFO1   1

#define CY_CANFD_MSGRAM_START           (0x00010000ul)
  
/**
 ******************************************************************************
 ** \brief The macro for configuration the standard ID filter to store RX buffer.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      Id == RxId
 ** When the RxId matches the condition, the message will be store into a rx buffer
 ** that specified by RxBuffIdx.
 ******************************************************************************/
#define CANFD_CONFIG_STD_ID_FILTER_CLASSIC_RXBUFF(Id, RxBuffIdx)                            \
    {                                                                                       \
        .sfid2 = (0u << 9u) | (uint32_t)(RxBuffIdx),                                        \
        .sfid1 = (uint32_t)(Id),                                                            \
        .sfec  = (uint32_t)CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXBUFF_OR_DEBUGMSG,       \
        .sft   = (uint32_t)CY_CANFD_STD_ID_FILTER_TYPE_CLASSIC,                             \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the standard ID filter as the classic filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      (Id & IdMask) == (RxId & IdMask)
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_STD_ID_FILTER_CLASSIC(Id, IdMask, ElementConfig)       \
    {                                                                       \
        .sfid2 = (uint32_t)(IdMask),                                        \
        .sfid1 = (uint32_t)(Id),                                            \
        .sfec  = (uint32_t)(ElementConfig),                                 \
        .sft   = (uint32_t)CY_CANFD_STD_ID_FILTER_TYPE_CLASSIC,             \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the standard ID filter as the range filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      MinId <= RxId <= MaxId
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_STD_ID_FILTER_RANGE(MinId, MaxId, ElementConfig)       \
    {                                                                       \
        .sfid2 = (uint32_t)(MaxId),                                         \
        .sfid1 = (uint32_t)(MinId),                                         \
        .sfec  = (uint32_t)(ElementConfig),                                 \
        .sft   = (uint32_t)CY_CANFD_STD_ID_FILTER_TYPE_RANGE,               \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the standard ID filter as the dual filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      Id1 == RxId
 **      OR
 **      Id2 == RxId
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_STD_ID_FILTER_DUAL(Id1, Id2, ElementConfig)            \
    {                                                                       \
        .sfid2 = (uint32_t)(Id2),                                           \
        .sfid1 = (uint32_t)(Id1),                                           \
        .sfec  = (uint32_t)(ElementConfig),                                 \
        .sft   = (uint32_t)CY_CANFD_STD_ID_FILTER_TYPE_DUAL,                \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the extended ID filter to store RX buffer.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      Id == (RxId & extIDANDMask)
 ** When the RxId matches the condition, the message will be store into a rx buffer
 ** that specified by RxBuffIdx.
 ******************************************************************************/
#define CANFD_CONFIG_EXT_ID_FILTER_CLASSIC_RXBUFF(Id, RxBuffIdx)                            \
    {                                                                                       \
        .f0_f =                                                                             \
        {                                                                                   \
            .efid1  = (uint32_t)(Id),                                                       \
            .efec   = (uint32_t)CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXBUFF_OR_DEBUGMSG,  \
        },                                                                                  \
        .f1_f =                                                                             \
        {                                                                                   \
            .efid2  = (0u << 9u) | (uint32_t)(RxBuffIdx),                                   \
            .eft    = (uint32_t)CY_CANFD_EXT_ID_FILTER_TYPE_CLASSIC,                        \
        },                                                                                  \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the extended ID filter as the classic filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      (Id & IdMask) == ((RxId & extIDANDMask) & IdMask)
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_EXT_ID_FILTER_CLASSIC(Id, IdMask, ElementConfig)           \
    {                                                                           \
        .f0_f =                                                                 \
        {                                                                       \
            .efid1  = (uint32_t)(Id),                                           \
            .efec   = (uint32_t)(ElementConfig),                                \
        },                                                                      \
        .f1_f =                                                                 \
        {                                                                       \
            .efid2  = (uint32_t)(IdMask),                                       \
            .eft    = (uint32_t)CY_CANFD_EXT_ID_FILTER_TYPE_CLASSIC,            \
        },                                                                      \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the extended ID filter as the range filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      MinId <= RxId <= MaxId
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_EXT_ID_FILTER_RANGE(MinId, MaxId, ElementConfig)           \
    {                                                                           \
        .f0_f =                                                                 \
        {                                                                       \
            .efid1  = (uint32_t)(MinId),                                        \
            .efec   = (uint32_t)(ElementConfig),                                \
        },                                                                      \
        .f1_f =                                                                 \
        {                                                                       \
            .efid2  = (uint32_t)(MaxId),                                        \
            .eft    = (uint32_t)CY_CANFD_EXT_ID_FILTER_TYPE_RANGE               \
        },                                                                      \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the extended ID filter as the range and mask filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      MinId <= (RxId & extIDANDMask) <= MaxId
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_EXT_ID_FILTER_RANGE_AND_MASK(MinId, MaxId, ElementConfig)  \
    {                                                                           \
        .f0_f =                                                                 \
        {                                                                       \
            .efid1  = (uint32_t)(MinId),                                        \
            .efec   = (uint32_t)(ElementConfig),                                \
        },                                                                      \
        .f1_f =                                                                 \
        {                                                                       \
            .efid2  = (uint32_t)(MaxId),                                        \
            .eft    = (uint32_t)CY_CANFD_EXT_ID_FILTER_TYPE_RANGE_AND_MASK      \
        },                                                                      \
    }

/**
 ******************************************************************************
 ** \brief The macro for configuration the extended ID filter as the dual filter.
 ** If RxId fulfills the following condition, RxId matches in this filter.
 **      Id1 == (RxId & extIDANDMask)
 **      OR
 **      Id2 == (RxId & extIDANDMask)
 ** When the RxId matches the condition, the message will be treated in accordance
 ** with ElementConfig.
 ** ElementConfig can be selected from following definitions:
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0
 **     - CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1
 ******************************************************************************/
#define CANFD_CONFIG_EXT_ID_FILTER_DUAL(Id1, Id2, ElementConfig)                \
    {                                                                           \
        .f0_f =                                                                 \
        {                                                                       \
            .efid1  = (uint32_t)(Id1),                                          \
            .efec   = (uint32_t)(ElementConfig),                                \
        },                                                                      \
        .f1_f =                                                                 \
        {                                                                       \
            .efid2  = (uint32_t)(Id2),                                          \
            .eft    = (uint32_t)CY_CANFD_EXT_ID_FILTER_TYPE_DUAL                \
        },                                                                      \
    }

/***************************************
*       Enumerations
***************************************/
/**
* \addtogroup group_canfd_enums
* \{
*/

/**
 *****************************************************************************
 ** \brief CAN FD driver error codes.
 ** 
 *****************************************************************************/
typedef enum
{
    CY_CANFD_SUCCESS   = 0x00u,                                     /**< Returned successful */
    CY_CANFD_BAD_PARAM = CY_CANFD_ID | CY_PDL_STATUS_ERROR | 0x01u, /**< Bad parameter was passed */
} cy_en_canfd_status_t;

/**
 *****************************************************************************
 ** \brief CAN FD Tx Buffer status.
 ** 
 *****************************************************************************/
typedef enum
{
    CY_CANFD_TX_BUFFER_IDLE = 0u,
    CY_CANFD_TX_BUFFER_PENDING = 1u,     /**< Pending transmission> */   
    CY_CANFD_TX_BUFFER_TRANSMIT_OCCURRED = 2u, /**< Transmission occurred> */
    CY_CANFD_TX_BUFFER_CANCEL_REQUESTED = 3u, /**< Cancellation requested> */
    CY_CANFD_TX_BUFFER_CANCEL_FINISHED = 4u, /**< Cancellation finished> */
} cy_en_canfd_tx_buffer_status_t;

/**
 *****************************************************************************
 ** \brief CAN FD error.
 ** 
 ** These error values are used to report any CAN FD related error to the API via
 ** the CAN FD error callback function (canfd_error_func_ptr_t).
 *****************************************************************************/
typedef enum
{
    CY_CANFD_BUSOFF        = 0,    /*!< The CAN FD module is in busoff state. */
    CY_CANFD_WARNING       = 1     /*!< At least one error counter has reached error warning. */
} cy_en_canfd_bus_error_t;

/**
 *****************************************************************************
 ** \brief CAN FD instance Clock source.
 ** 
 ** These values are used to specify the clock source for the CAN FD instance
 *****************************************************************************/
typedef enum
{
    CY_CANFD_CLOCK_SOURCE_MAIN  = 0,    // TODO: DENA. Clocks SAS still not finalized. Will be done later
} cy_en_canfd_clock_source_t; 

/**
 *****************************************************************************
 ** \brief CAN FD Bus status.
 ** 
 ** These state values represent the current CAN FD state, and are used in the 
 ** CAN FD status changed callback function (canfd_status_chg_func_ptr_t).
 *****************************************************************************/
typedef enum
{
    CY_CANFD_NO_ERROR    = 0,   /*!< No error pending. */
    CY_CANFD_STUFF_ERROR = 1,   /*!< More than 5 equal bits in a sequence have occurred in a part of a received message. */
    CY_CANFD_FORM_ERROR  = 2,   /*!< A fixed format part of a received frame has the wrong format. */
    CY_CANFD_ACK_ERROR   = 3,   /*!< The message this CAN FD Core transmitted was not acknowledged by another node. */
    CY_CANFD_BIT1_ERROR  = 4,   /*!< While trying to send a recessive bit (1) a dominant level (0) was sampled. */
    CY_CANFD_BIT0_ERROR  = 5,   /*!< While trying to send a dominant bit (0) a recessive level (1) was sampled. */
    CY_CANFD_CRC_ERROR   = 6    /*!< The CRC checksum was incorrect. */
} cy_en_canfd_bus_error_status_t;

/**
 *****************************************************************************
 ** \brief CAN FD TX/RX buffer element size.
 ** 
 ** The number of data bytes belonging to an Rx, Tx Buffer / Rx FIFO element. 
 *****************************************************************************/
typedef enum
{
    CY_CANFD_BUFFER_DATA_SIZE_8  = 0,   /*!< 8 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_12 = 1,   /*!< 12 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_16 = 2,   /*!< 16 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_20 = 3,   /*!< 20 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_24 = 4,   /*!< 24 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_32 = 5,   /*!< 32 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_48 = 6,   /*!< 48 byte data field. */
    CY_CANFD_BUFFER_DATA_SIZE_64 = 7    /*!< 64 byte data field. */
} cy_en_canfd_buffer_data_size_t;

/**
 *****************************************************************************
 ** \brief CAN FD RX FIFO operating mode.
 *****************************************************************************/
typedef enum
{
    CY_CANFD_FIFO_MODE_BLOCKING  = 0,   /*!< FIFO blocking mode. */
    CY_CANFD_FIFO_MODE_OVERWRITE = 1    /*!< FIFO overwrite mode. */
} cy_en_canfd_fifo_mode_t;

/**
 *****************************************************************************
 ** \brief CAN FD accept non matching frames.
 *****************************************************************************/
typedef enum
{
    CY_CANFD_ACCEPT_IN_RXFIFO_0  = 0,
    CY_CANFD_ACCEPT_IN_RXFIFO_1  = 1,
    CY_CANFD_REJECT_NON_MATCHING = 2
} cy_en_accept_non_matching_frames_t;

/**
 ******************************************************************************
 ** \brief ID Filter Element Configuration Type
 ******************************************************************************/
typedef enum
{
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_DISABLE_FILETER_ELEMENT    = 0x00u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO0              = 0x01u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXFIFO1              = 0x02u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_REJECT_ID                  = 0x03u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PRIORIY                = 0x04u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO0  = 0x05u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_SET_PIORITY_STORE_RXFIFO1  = 0x06u,
    CY_CANFD_ID_FILTER_ELEMNT_CONFIG_STORE_RXBUFF_OR_DEBUGMSG   = 0x07u,
}cy_en_canfd_id_filter_element_confifg_t;

/**
 ******************************************************************************
 ** \brief Standard ID Filter Type
 ******************************************************************************/
typedef enum
{
    CY_CANFD_STD_ID_FILTER_TYPE_RANGE           = 0x00u,
    CY_CANFD_STD_ID_FILTER_TYPE_DUAL            = 0x01u,
    CY_CANFD_STD_ID_FILTER_TYPE_CLASSIC         = 0x02u,
}cy_en_canfd_std_id_filter_type_t;

/**
 ******************************************************************************
 ** \brief Extended ID Filter Type
 ******************************************************************************/
typedef enum
{
    CY_CANFD_EXT_ID_FILTER_TYPE_RANGE_AND_MASK  = 0x00u,
    CY_CANFD_EXT_ID_FILTER_TYPE_DUAL            = 0x01u,
    CY_CANFD_EXT_ID_FILTER_TYPE_CLASSIC         = 0x02u,
    CY_CANFD_EXT_ID_FILTER_TYPE_RANGE           = 0x03u,
}cy_en_canfd_ext_id_filter_type_t;

/**
 ******************************************************************************
 ** \brief Method of receiving the message
 ******************************************************************************/
typedef enum
{
    CY_CANFD_RECEIVED_IN_DEDICATED_BUFFER               = 0x00u,
    CY_CANFD_RECEIVED_IN_FIFO_TOP_POINTER_NOT_USED      = 0x01u,
    CY_CANFD_RECEIVED_IN_FIFO_TOP_POINTER_USED          = 0x02u,
}cy_en_canfd_rx_received_type_t;

/** \} group_canfd_enums */

/***************************************
*       Configuration Structures
***************************************/

/**
* \addtogroup group_canfd_data_structures
* \{
*/
/**
 ******************************************************************************
 ** \brief Redefinition of R0 register bitfields
 ******************************************************************************/
typedef struct
{
    volatile uint32_t  id         :29;  /*!< Identifier */
    volatile uint32_t  rtr        : 1;  /*!< Remote Transmission Request. 0:data frame, 1:remote frame */
    volatile uint32_t  xtd        : 1;  /*!< Extended Identifier. 0:11-bit standard identifier, 1:29-bit extended identifier */
    volatile uint32_t  esi        : 1;  /*!< Error State Indicator. 0:error active, 1:error passive */
} cy_stc_canfd_r0_t;

/**
 ******************************************************************************
 ** \brief Redefinition of R1 register bitfields
 ******************************************************************************/
typedef struct
{
    volatile uint32_t  rxts       :16;  /*!< Rx Timestamp */
    volatile uint32_t  dlc        : 4;  /*!< Data Length Code */
    volatile uint32_t  brs        : 1;  /*!< Bit Rate Switch. 0:without bit rate switching, 1:with bit rate switching */
    volatile uint32_t  fdf        : 1;  /*!< Extended Data Length. 0:Classic CAN frame format, 1:CAN FD frame format */
             uint32_t  reserved1  : 2;  /*!< Reserved bits */
    volatile uint32_t  fidx       : 7;  /*!< Filter Index */
    volatile uint32_t  anmf       : 1;  /*!< Accepted Non-matching Frame. 0:matching filter index FIDX, 1:did not match any Rx filter element */
} cy_stc_canfd_r1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of Rx Buffer Register
 ******************************************************************************/
typedef struct
{
    cy_stc_canfd_r0_t   r0_f;               /*!< Rx buffer element for R0. See #cy_stc_canfd_r0_t. */
    cy_stc_canfd_r1_t   r1_f;               /*!< Rx buffer element for R1. See #cy_stc_canfd_r1_t. */
    uint32_t            data_area_f[16];    /*!< Rx buffer element for Rn */
} cy_stc_canfd_rx_buffer_t;


/**
 ******************************************************************************
 ** \brief Redefinition of T0 register bitfields
 ******************************************************************************/
typedef struct
{
    volatile uint32_t  id         :29;  /*!< Identifier */
    volatile uint32_t  rtr        : 1;  /*!< Remote Transmission Request. 0:data frame, 1:remote frame */
    volatile uint32_t  xtd        : 1;  /*!< Extended Identifier. 0:11-bit standard identifier, 1:29-bit extended identifier */
    volatile uint32_t  esi        : 1;  /*!< Error State Indicator. 0:ESI bit in CAN FD format depends only on error passive flag, 1:ESI bit in CAN FD format transmitted recessive. */
} cy_stc_canfd_t0_t;

/**
 ******************************************************************************
 ** \brief Redefinition of T1 register bitfields
 ******************************************************************************/
typedef struct
{
             uint32_t  reserved1  :16;  /*!< Reserved bits */
    volatile uint32_t  dlc        : 4;  /*!< Data Length Code */
    volatile uint32_t  brs        : 1;  /*!< Bit Rate Switching. 0:CAN FD frames transmitted without bit rate switching, CAN FD frames transmitted with bit rate switching */
    volatile uint32_t  fdf        : 1;  /*!< FD Format. 0:Frame transmitted in Classic CAN format, 1:Frame transmitted in CAN FD format */
    volatile uint32_t  reserved2  : 1;  /*!< Reserved bit*/ // Added by DENA
    volatile uint32_t  efc        : 1;  /*!< Event FIFO Control. 0:Tx events not stored, 1:Store Tx events */
    volatile uint32_t  mm         : 8;  /*!< Message Marker */
} cy_stc_canfd_t1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of Tx Buffer Register
 ******************************************************************************/
typedef struct
{
    cy_stc_canfd_t0_t   t0_f;               /*!< Tx buffer element for T0. See #cy_stc_canfd_t0_t. */
    cy_stc_canfd_t1_t   t1_f;               /*!< Tx buffer element for T1. See #cy_stc_canfd_t1_t. */
    uint32_t            data_area_f[16];    /*!< Tx buffer element for Tn */
} cy_stc_canfd_tx_buffer_t;


/**
 ******************************************************************************
 ** \brief Redefinition of Message ID Filter register
 ** This filter can be configured with CANFD_CONFIG_STD_ID_FILTER_* macros.
 ******************************************************************************/
typedef struct
{
    volatile uint32_t sfid2      :11;   /*!< Standard Filter ID 2 */
             uint32_t reserved4  : 5;   /*!< Reserved bits */
    volatile uint32_t sfid1      :11;   /*!< Standard Filter ID 1 */
    volatile uint32_t sfec       : 3;   /*!< Standard Filter Element Configuration */
    volatile uint32_t sft        : 2;   /*!< Standard Filter Type */
} cy_stc_id_filter_t;

/**
 ******************************************************************************
 ** \brief Redefinition of F0 register bitfields
 ******************************************************************************/
typedef struct
{
    volatile uint32_t efid1      :29;   /*!< Extended Filter ID 1 */
    volatile uint32_t efec       : 3;   /*!< Extended Filter Element Configuration */
} cy_stc_canfd_f0_t;

/**
 ******************************************************************************
 ** \brief Redefinition of F1 register bitfields
 ******************************************************************************/
typedef struct
{
    volatile uint32_t efid2      :29;   /*!< Extended Filter ID 2 */
             uint32_t reserved1  : 1;   /*!< Reserved bits */
    volatile uint32_t eft        : 2;   /*!< Extended Filter Type */
} cy_stc_canfd_f1_t;

/**
 ******************************************************************************
 ** \brief Redefinition of Extended Message ID Filter register
 ** This filter can be configured with CANFD_CONFIG_EXT_ID_FILTER_* macros.
 ******************************************************************************/
typedef struct
{
    cy_stc_canfd_f0_t  f0_f;   /*!< Extended Message ID Filter Element for F0. See #stc_canfd_f0_t. */
    cy_stc_canfd_f1_t  f1_f;   /*!< Extended Message ID Filter Element for F1. See #stc_canfd_f1_t. */
} cy_stc_extid_filter_t;


/**
 *****************************************************************************
 ** \brief CAN FD bitrate.
 ** 
 ** This structure is used to set the bitrate register.
 *****************************************************************************/
typedef struct
{
    uint16_t  prescaler;     /*!< Baud Rate Prescaler */
    uint8_t   timeSegment1;  /*!< Time segment before sample point */
    uint8_t   timeSegment2;  /*!< Time segment after sample point */
    uint8_t   syncJumpWidth; /*!< Synchronization Jump Width */
} cy_stc_canfd_bitrate_t;

/**
 *****************************************************************************
 ** \brief CAN FD transceiver delay compensation offset configuration.
 *****************************************************************************/
typedef struct
{
    bool    tdcEnabled;
    uint8_t tdcOffset;
    uint8_t tdcFilterWindow;
} cy_stc_canfd_transceiver_delay_compensation_t;

/**
 *****************************************************************************
 ** \brief CAN FD message identifier
 ** 
 ** This structure is used to set the CAN FD message identifier for transmit and 
 ** receive operations. 
 *****************************************************************************/
typedef struct
{
    uint32_t  identifier;     /*!< 11- or 29-bit identifier. The valid bit length depends on bExtended. */
    bool      extended;       /*!< TRUE: 29-bit identifier,  FALSE: 11-bit identifier. */
} cy_stc_canfd_msg_id_t;


/**
 *****************************************************************************
 ** \brief CAN FD message data.
 ** 
 ** Data structure for transmit and receive operations
 *****************************************************************************/
typedef struct
{
    uint32_t data[CY_CANFD_MESSAGE_DATA_BUFFER_SIZE];  /*!< Data of CAN FD message. */
    uint8_t  dataLengthCode;  /*!< Number of valid bytes in data and DLC of CAN FD message. */
} cy_stc_canfd_msg_data_t;

/**
 *****************************************************************************
 ** \brief CAN FD message.
 ** 
 ** This structure stores a CAN FD message, including the identifier, data 
 ** and data length code (DLC).
 *****************************************************************************/
typedef struct
{
    bool                    canFDFormat;   /*!< TRUE:CAN FD format, FALSE:Classic CAN format. */
    cy_stc_canfd_msg_id_t   idConfig;  /*!< 11- or 29-bit identifier (ID) */
    cy_stc_canfd_msg_data_t dataConfig;        /*!< Data and DLC */
} cy_stc_canfd_msg_t;

/**
 *****************************************************************************
 ** \brief Standard ID filter configuration.
 ** 
 ** Data structure for SID flter configuration.
 *****************************************************************************/
typedef struct
{
    uint8_t             numberOfSIDFilters;
    const cy_stc_id_filter_t* sidFilter;
} cy_stc_canfd_sid_filter_config_t;

/**
 *****************************************************************************
 ** \brief Extended ID filter configuration.
 ** 
 ** Data structure for EXTID flter configuration.
 *****************************************************************************/
typedef struct
{
    uint8_t                 numberOfEXTIDFilters;
    const cy_stc_extid_filter_t*  extidFilter;
    uint32_t                extIDANDMask;
} cy_stc_canfd_extid_filter_config_t;

/**
 *****************************************************************************
 ** \brief Global filter configuration.
 ** 
 ** Data structure for Global flter configuration.
 *****************************************************************************/
typedef struct
{
  cy_en_accept_non_matching_frames_t nonMatchingFramesStandard;
  cy_en_accept_non_matching_frames_t nonMatchingFramesExtended;
  bool                               rejectRemoteFramesStandard;
  bool                               rejectRemoteFramesExtended;    
} cy_stc_canfd_global_filter_config_t;

/**
 *****************************************************************************
 ** \brief Rx Fifo configuration.
 ** 
 ** Data structure for Rx Fifo configuration.
 *****************************************************************************/
typedef struct
{
    cy_en_canfd_fifo_mode_t mode;
    uint8_t                 watermark;
    uint8_t                 numberOfFifoElements;
    bool                    topPointerLogicEnabled;
}cy_en_canfd_fifo_config_t;

/**
 *****************************************************************************
 ** \brief Message transmission complete callback function (cy_canfd_tx_msg_func_ptr_t).
 **
 ** Signals a successful completed transmission.
 *****************************************************************************/
typedef void (*cy_canfd_tx_msg_func_ptr_t)( void );


/**
 *****************************************************************************
 ** \brief Message reception callback function for message received in dedicated 
 **        Rx Buffer or in Rx Fifo when Rx Fifo top pointer is not used (cy_canfd_rx_msg_func_ptr_t)
 **
 ** Signals that CAN FD has received a new message.
 ** -- If Message was received in dedicated Rx Buffer (0 - 63) then bRxFifoMsg is False 
 **     and u8MsgBufOrFifoNum indicates the Rx Buffer number
 ** -- If Message was received in Rx Fifo (0/1 and Rx Fifo top pointer is not used)
 **       then bRxFifoMsg is True and u8MsgBufOrFifoNum indicates  the FIFO number.
 *****************************************************************************/
typedef void (*cy_canfd_rx_msg_func_ptr_t)(bool bRxFifoMsg,
                                           uint8_t u8MsgBufOrRxFifoNum, 
                                           cy_stc_canfd_msg_t* pstcCanFDmsg
                                           );

/**
 *****************************************************************************
 ** \brief Message reception callback function for message received in Rx Fifo 
 **        when Rx Fifo Top pointer is used (cy_canfd_rx_msg_func_ptr_t).
 **
 *****************************************************************************/
typedef void (*cy_canfd_rx_fifo_msg_with_top_func_ptr_t)(uint8_t   u8FifoNum,
                                                         uint8_t   u8BufferSizeInWord,
                                                         uint32_t* pu32RxBuf
                                                         );

/**
 *****************************************************************************
 ** \brief Status changed callback function (cy_canfd_status_chg_func_ptr_t).
 **
 ** Any status change will be reported to the API (see #cy_en_canfd_bus_error_status_t for 
 ** possible status change codes).
 *****************************************************************************/
typedef void (*cy_canfd_status_chg_func_ptr_t)(cy_en_canfd_bus_error_status_t enCanFDStatus);


/**
 *****************************************************************************
 ** \brief Error callback function (cy_canfd_error_func_ptr_t).
 **
 ** Any error will be reported to the API (see #cy_en_canfd_bus_error_t for 
 ** possible error codes).
 *****************************************************************************/
typedef void (*cy_canfd_error_func_ptr_t)(cy_en_canfd_bus_error_t enCanFDError);

/**
 *****************************************************************************
 ** \brief CAN FD configuration.
 ** 
 ** Is used to set the bitrate and select CAN mode or CAN FD mode.
 ** Optionally Callbacks notification can be set.  
 *****************************************************************************/
typedef struct
{
    cy_canfd_tx_msg_func_ptr_t                  txCallback;        /*!< Callback function for transmit completed, can be NULL. */
    cy_canfd_rx_msg_func_ptr_t                  rxCallback;        /*!< Callback function for receive completed, can be NULL. */
    cy_canfd_rx_fifo_msg_with_top_func_ptr_t    rxFifoWithTopCallback; /*!< Callback function for receive in Fifo completed with Fifo Top pointer logic enabled */
    cy_canfd_status_chg_func_ptr_t              statusCallback;    /*!< Callback function for CAN status changes, can be NULL. */
    cy_canfd_error_func_ptr_t                   errorCallback;     /*!< Callback function for CAN related errors, can be NULL. */
    bool                                        canFDMode;             /*!< TRUE:CAN FD mode, FALSE:Classic CAN mode. */
    cy_stc_canfd_bitrate_t                      bitrate;               /*!< CAN bitrate setting.  */
    cy_stc_canfd_bitrate_t                      fastBitrate;           /*!< CAN Fast bitrate setting.  */
    cy_stc_canfd_transceiver_delay_compensation_t tdcConfig; /*!< CAN transceiver delay compensation setting.  */
    
    cy_stc_canfd_sid_filter_config_t    sidFilterConfig;    /*!< CAN Standard ID filter setting.  */
    cy_stc_canfd_extid_filter_config_t  extidFilterConfig;  /*!< CAN Extended ID filter setting.  */
    cy_stc_canfd_global_filter_config_t globalFilterConfig; /*!< CAN global filter setting.  */
    
    cy_en_canfd_buffer_data_size_t      rxBufferDataSize;  
    cy_en_canfd_buffer_data_size_t      rxFifo1DataSize;
    cy_en_canfd_buffer_data_size_t      rxFifo0DataSize;    
    cy_en_canfd_buffer_data_size_t      txBufferDataSize;
    
    cy_en_canfd_fifo_config_t           rxFifo0Config;
    cy_en_canfd_fifo_config_t           rxFifo1Config;
    uint8_t                             noOfRxBuffers;  /*!< Max 64> */
    uint8_t                             noOfTxBuffers;  /*!< Max 32> */
} cy_stc_canfd_config_t;

/**
 *****************************************************************************
 ** \brief CAN FD interrupt pointer structure
 **
 ** Holds some pointers to callback functions and buffer
 *****************************************************************************/
typedef struct
{
    cy_canfd_tx_msg_func_ptr_t               canFDTxInterruptFunction; /*!< pointer to transmit interrupt callback */
    cy_canfd_rx_msg_func_ptr_t               canFDRxInterruptFunction; /*!< pointer to receive interrupt callback (dedicated Rx Buffer or RxFifo without Top pointer logic)*/
    cy_canfd_rx_fifo_msg_with_top_func_ptr_t canFDRxWithTopPtrInterruptFunction; /*!< pointer to receive interrupt callback (RxFifo when Top pointer logic used)*/
} cy_stc_canfd_interrupt_handling_t;

/**
 *****************************************************************************
 ** \brief CAN FD notification pointer structure
 **
 ** Holds some pointers to callback functions for status and error notification
 *****************************************************************************/
typedef struct
{
    cy_canfd_status_chg_func_ptr_t canFDStatusInterruptFunction ; /*!< pointer to status interrupt callback */
    cy_canfd_error_func_ptr_t      canFDErrorInterruptFunction ;  /*!< pointer to error interrupt callback */
} cy_stc_canfd_notification_t;

/**
 *****************************************************************************
 ** \brief CAN FD internal data structure
 **
 ** Pointer to callback function interrupt handler
 *****************************************************************************/
typedef struct
{
    cy_stc_canfd_interrupt_handling_t canFDInterruptHandling;   /*!< Interrupt callback. See #stc_canfd_interrupt_handling_t. */
    cy_stc_canfd_notification_t       canFDNotificationCb;      /*!< Interrupt callback. See #stc_canfd_notification_t. */
} cy_stc_canfd_context_t;

/** \} group_canfd_data_structures */

/**
 *****************************************************************************
 ** \brief CAN FD Unit Type
 *****************************************************************************/
typedef  volatile stc_CANFD_CH_t*  cy_pstc_canfd_type_t;

/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_canfd_functions
* \{
*/
void Cy_CANFD_IrqHandler( cy_pstc_canfd_type_t pstcCanFD );

cy_en_canfd_status_t Cy_CANFD_Init( cy_pstc_canfd_type_t pstcCanFD, const cy_stc_canfd_config_t* pstcConfig) ;


cy_en_canfd_status_t Cy_CANFD_DeInit(cy_pstc_canfd_type_t pstcCanFD);
                             
cy_en_canfd_status_t Cy_CANFD_UpdateAndTransmitMsgBuffer( cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf, cy_stc_canfd_msg_t* pstcMsg ) ;

cy_en_canfd_tx_buffer_status_t Cy_CANFD_GetTxBufferStatus( cy_pstc_canfd_type_t pstcCanFD, uint8_t u8MsgBuf);
void Cy_CANFD_ExtractMsgFromRXBuffer(cy_stc_canfd_rx_buffer_t *pstcRxBufferAddr, cy_stc_canfd_msg_t *pstcCanFDmsg);

cy_en_canfd_status_t Cy_CANFD_ClockInit( cy_pstc_canfd_type_t pstcCanFD, 
                                         uint8_t u8CanFDPrescaler,               // TODO: DENA. Clock SAS still not finalized. Will be done later
                                         cy_en_canfd_clock_source_t enCanFDClockSource // TODO: DENA. Clock SAS still not finalized. Will be done later
                                        ) ;
/** \} group_canfd_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_CANFD_H */

/** \} group_canfd */

/* [] END OF FILE */
