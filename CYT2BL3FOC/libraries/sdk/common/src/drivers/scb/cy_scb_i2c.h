/***************************************************************************//**
* \file cy_scb_i2c.h
* \version 1.0
*
* Provides I2C API declarations of the SCB driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \addtogroup group_scb_i2c
* \{
* Driver API for I2C Bus Peripheral
*
* I2C - The Inter-Integrated Circuit (I2C) bus is an industry-standard.
* The two-wire hardware interface was developed by Philips Semiconductors
* (now NXP Semiconductors).
*
* The I2C peripheral driver provides an API to implement I2C slave, master,
* or master-slave devices based on the SCB hardware block.
* I2C devices based on SCB hardware are compatible with I2C
* Standard-mode, Fast-mode, and Fast-mode Plus specifications as defined in
* the I2C-bus specification.
*
* Features:
* * An industry-standard I2C bus interface
* * Supports slave, master, and master-slave operation
* * Supports standard data rates of 100/400/1000 kbps
* * Hardware Address Match, multiple addresses
* * Wake From DeepSleep on Address Match
*
* \section group_scb_i2c_configuration Configuration Considerations
*
* To set up the I2C driver, provide the configuration parameters in the
* \ref cy_stc_scb_i2c_config_t structure. For master configuration,
* provide i2cMode, useRxFifo, and useTxFifo. For slave mode also provide the
* slaveAddress and slaveAddressMask. The other parameters are optional for
* operation.
* To initialize the driver, call the \ref Cy_SCB_I2C_Init function providing the
* filled \ref cy_stc_scb_i2c_config_t structure and allocated
* \ref cy_stc_scb_i2c_context_t. Before calling \ref Cy_SCB_I2C_Enable to start
* the I2C operation, the \ref Cy_SCB_I2C_SetDataRate must be called to set
* the data rate.
*
* Master mode operation: \n
* The master API is divided into two categories: 
* \ref group_scb_i2c_master_high_level_functions and 
* \ref group_scb_i2c_master_low_level_functions. Therefore there are two 
* methods for initiating I2C master transactions using either low-level or 
* high-level API. These two methods are described below. Only one method should 
* be used at a time. They should not be mixed.
*
* * Call \ref Cy_SCB_I2C_MasterRead or \ref Cy_SCB_I2C_MasterWrite to
*   communicate with the slave. These functions do not block and only
*   start a transaction. After a transfer is started, the
*   \ref Cy_SCB_I2C_Interrupt handles the further data transfer until its
*   completion. Therefore, \ref Cy_SCB_I2C_Interrupt must be called inside the
*   user interrupt handler to make the API above work. To monitor the transfer,
*   use \ref Cy_SCB_I2C_MasterGetStatus or \ref Cy_SCB_I2C_RegisterEventCallback
*   to register callback function to be notified about
*   \ref group_scb_i2c_macro_callback_events.
*
* * Call \ref Cy_SCB_I2C_MasterSendStart to generate a start, send an address
*   with the Read/Write direction bit, and receive acknowledgment. After the
*   address is ACKed by the slave, the transaction can be continued by calling
*   \ref Cy_SCB_I2C_MasterReadByte or \ref Cy_SCB_I2C_MasterWriteByte depending
*   on its direction. These functions handle one byte per call, therefore
*   they should be called for each byte in the transaction. Note that for the
*   Read transaction the last byte must be NAKed. To complete the current
*   transaction, call \ref Cy_SCB_I2C_MasterSendStop or call
*   \ref Cy_SCB_I2C_MasterSendReStart to complete the current transaction and
*   start a new one. Typically, do a restart to change the transaction
*   direction without releasing the bus from the master control.
*   Note that these functions are blocking and do not require calling
*   \ref Cy_SCB_I2C_Interrupt inside the user interrupt handler. Using these
*   functions requires extensive knowledge of the I2C protocol to execute
*   transactions correctly.
*
* Slave mode operation: \n
* The slave operation is based on the \ref Cy_SCB_I2C_Interrupt that must be
* called inside the user interrupt handler. The Read and Write buffer must
* be provided for the slave to enable communication with the master. Use
* \ref Cy_SCB_I2C_SlaveConfigReadBuf and \ref Cy_SCB_I2C_SlaveConfigWriteBuf
* for this purpose. Note that after transaction completion the buffer must be
* configured again. Otherwise, the same buffer is used starting from the point
* where the master stopped a previous transaction.
* For example: The Read buffer is configured to be 10 bytes and the master Read
* is 8 bytes. If the Read buffer is not configured again, the next master Read
* will start from the 9th byte.
* To monitor the transfer status, use \ref Cy_SCB_I2C_SlaveGetStatus or
* use \ref Cy_SCB_I2C_RegisterEventCallback to register a callback function
* to be notified about \ref group_scb_i2c_macro_callback_events.
*
* \note 
* All slave API (except \ref Cy_SCB_I2C_SlaveAbortRead and
* \ref Cy_SCB_I2C_SlaveAbortWrite) are not interrupt-protected and to
* prevent a race condition, they should be protected from the I2C interruption
* in the place where they are called.
*
* \section group_scb_i2c_more_information More Information
*
* For more information on the SCB peripheral, refer to the technical reference
* manual (TRM).
*
* \section group_scb_i2c_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_scb_i2c_macro Macro
* \defgroup group_scb_i2c_functions Functions
* \{
* \defgroup group_scb_i2c_general_functions General
* \defgroup group_scb_i2c_slave_functions Slave
* \defgroup group_scb_i2c_master_high_level_functions Master High-Level
* \defgroup group_scb_i2c_master_low_level_functions Master Low-Level
* \defgroup group_scb_i2c_interrupt_functions Interrupt
* \defgroup group_scb_i2c_low_power_functions Low Power Callbacks
* \}
* \defgroup group_scb_i2c_data_structures Data Structures
* \defgroup group_scb_i2c_enums Enumerated Types
*/

#if !defined(CY_SCB_I2C_H)
#define CY_SCB_I2C_H

#include "cy_scb_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*          Enumerated Types
***************************************/

/**
* \addtogroup group_scb_i2c_enums
* \{
*/

/** I2C status codes */
typedef enum
{
    /** Operation completed successfully */
    CY_SCB_I2C_SUCCESS = 0u,

    /** One or more of input parameters are invalid */
    CY_SCB_I2C_BAD_PARAM = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 1u),

    /**
    * The master is not ready to start a new transaction.
    * Either the master is still processing a previous transaction or in the
    * master-slave mode the slave operation is in progress. Call this function
    * again once that operation is completed or aborted.
    */
    CY_SCB_I2C_MASTER_NOT_READY = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 2u),

    /**
    * The master operation timed out before completing. Applicable only for
    * the \ref group_scb_i2c_master_low_level_functions functions.
    */
    CY_SCB_I2C_MASTER_MANUAL_TIMEOUT = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 3u),

    /** The slave NACKed the address. Applicable only for the
    * \ref group_scb_i2c_master_low_level_functions functions.
    */
    CY_SCB_I2C_MASTER_MANUAL_ADDR_NAK = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 4u),

    /** The slave NACKed the data byte.  Applicable only for the 
    * \ref group_scb_i2c_master_low_level_functions. 
    */
    CY_SCB_I2C_MASTER_MANUAL_NAK = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 5u),

    /**
    * The master lost arbitration, the transaction was aborted. Applicable only 
    * for the \ref group_scb_i2c_master_low_level_functions functions.
    */
    CY_SCB_I2C_MASTER_MANUAL_ARB_LOST = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 6u),

    /**
    * The master detected an erroneous start or stop, the transaction was
    * aborted. Applicable only for the  
    * \ref group_scb_i2c_master_low_level_functions functions.
    */
    CY_SCB_I2C_MASTER_MANUAL_BUS_ERR = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 7u),

    /**
    * The master transaction was aborted and the slave transaction is on-going
    * because the slave was addressed before the master generated a start.
    * Applicable only for the \ref group_scb_i2c_master_low_level_functions 
    * functions.
    */
    CY_SCB_I2C_MASTER_MANUAL_ABORT_START = (CY_SCB_ID | CY_PDL_STATUS_ERROR | CY_SCB_I2C_ID | 8u)
} cy_en_scb_i2c_status_t;
/** \} group_scb_i2c_enums */


/***************************************
*       Type Definitions
***************************************/

/**
* \addtogroup group_scb_i2c_data_structures
* \{
*/

/**
* Provides the typedef for the callback function called in the
* \ref Cy_SCB_I2C_Interrupt to notify the user about occurrences of
* \ref group_scb_i2c_macro_callback_events.
*/
typedef void (* scb_i2c_handle_events_t)(uint32_t event);

/**
* Provides the typedef for the callback function called in the
* \ref Cy_SCB_I2C_Interrupt to notify the user about occurrences of
* \ref group_scb_i2c_macro_addr_callback_events.
* This callback must return a decision to ACK (continue transaction) or
* NAK (end transaction) the received address.
* See \ref group_scb_i2c_macro_ack_nack for the set of constants.
* Note if the slave configured to accept an address in RX FIFO, it must read
* from it using the \ref Cy_SCB_ReadRxFifo function.
*/
typedef uint32_t (* scb_i2c_handle_addr_t)(uint32_t event);

/** I2C configuration structure */
typedef struct cy_stc_scb_i2c_config
{
    /**
    * Specifies the mode of operation (See \ref group_scb_i2c_macro_modes
    * for the set of constants)
    */
    uint32_t i2cMode;

    /**
    * If the RX FIFO is used, the possibility of clock stretching is greatly
    * reduced, and the interrupt overhead during an I2C transaction is also
    * reduced. The reason: the hardware is configured to automatically ACK
    * incoming data, and the interrupt is configured to take data out of the
    * RX FIFO.
    * * For the master mode, the drawback is that the master may receive more
    *   data than desired due to the interrupt latency. An interrupt fires
    *   when the second-to-last byte has been received, this interrupt tells
    *   the hardware to stop receiving data. If the latency of this interrupt
    *   is longer than 1 transfer of the byte on the I2C bus, then the
    *   hardware automatically ACKs the following bytes until the interrupt
    *   is serviced or the RX FIFO becomes full.
    * * For the slave mode, the drawback is that the slave will only NACKs
    *   the master when the RX FIFO becomes full, NOT when the slave write
    *   firmware buffer becomes full.
    * Note that all received extra bytes will be dropped in any mode.
    * To avoid these drawbacks, do not enable this option. Then, hardware is
    * configured to trigger an interrupt when a byte is available in the
    * RX FIFO. After the byte is read, an ACK or NACK decision is made.
    * Therefore, each received bytes requires the interrupt service to
    * continue the transfer and that leads to increasing the interrupt
    * overhead and the possibility of clock stretching.
    * This parameter is not available if f acceptAddrInFifo is true.
    */
    bool useRxFifo;

    /**
    * If the TX FIFO is used, the possibility of clock stretching is greatly
    * reduced, and the interrupt overhead during an I2C transaction is
    * also reduced. The reason: the TX FIFO is fully loaded with data and
    * the interrupt is configured to keep the TX FIFO loaded.
    * The drawback of using this option is that the abort operation clears
    * the TX FIFO. The TX FIFO clear operation also clears the shift
    * register so that the shifter can be cleared in the middle of a byte
    * transfer, corrupting it. The remaining bits to transfer within the
    * corrupted byte are complemented with 1s. If this is an issue,
    * then do not enable this option.
    * If the TX FIFO is not used, then data is written to the TX FIFO byte
    * by byte. This leads to more interrupts and also increases the
    * possibility of clock stretching.
    * Use the TX FIFO if the abort function is not used.
    */
    bool useTxFifo;

    /**
    * The 7-bit right justified slave address, used only for the slave mode
    */
    uint8_t slaveAddress;

    /**
    * The slave address mask, bit 0, must always be 0. It's used only for the
    * slave mode
    */
    uint8_t slaveAddressMask;

    /**
    * True - the slave address is accepted in the RX FIFO, false - the slave
    * addresses are not accepted in the RX FIFO
    */
    bool acceptAddrInFifo;

    /**
    * True - accept the general call address; false - ignore the general
    * call address.
    */
    bool ackGeneralAddr;

    /**
    * When set, the slave will wake the device from deep sleep on an address
    * match (the device datasheet must be consulted to determine which SCBs
    * support this mode)
    */
    bool enableWakeFromSleep;
} cy_stc_scb_i2c_config_t;

/** I2C context structure */
typedef struct cy_stc_scb_i2c_context
{
    bool useRxFifo;             /**< Stores RX FIFO configuration */
    bool useTxFifo;             /**< Stores TX FIFO configuration */

    uint32_t state;             /**< The driver state */

    uint32_t masterStatus;      /**< The master status */
    bool     masterPause;       /**< Stores how the master ends the transfer */
    bool     masterRdDir;       /**< The direction of the master transfer */

    uint8_t  *masterBuffer;     /**< The pointer to the master buffer (either for a transmit or a receive operation) */
    uint32_t  masterBufferIdx;  /**< The current location in the master buffer */
    uint32_t  masterBufferSize; /**< The current master buffer size */
    uint32_t  masterNumBytes;   /**< The number of bytes to send or receive */

    uint32_t  slaveStatus;      /**< The slave status */
    bool      slaveRdBufEmpty;  /**< Tracks slave Read buffer empty event */

    uint8_t  *slaveTxBuffer;     /**< The pointer to the slave transmit buffer (a master reads from it) */
    uint32_t  slaveTxBufferIdx;  /**< The current location in the slave buffer */
    uint32_t  slaveTxBufferSize; /**< The current slave transmit buffer size */
    uint32_t  slaveTxBufferCnt;  /**< The number of transferred bytes */

    uint8_t  *slaveRxBuffer;     /**< The pointer to the slave receive buffer (a master writes into it) */
    uint32_t  slaveRxBufferIdx;  /**< The current location in the slave buffer */
    uint32_t  slaveRxBufferSize; /**< The current slave receive buffer size */

    /**
    * The pointer to an event callback that is called when any of
    * \ref group_scb_i2c_macro_callback_events occurs
    */
    scb_i2c_handle_events_t cbEvents;

    /**
    * The pointer to an address callback that is called when any of
    * \ref group_scb_i2c_macro_addr_callback_events occurs (applicable only
    * for the slave)
    */
    scb_i2c_handle_addr_t   cbAddr;
} cy_stc_scb_i2c_context_t;

/** The I2C Master transfer structure */
typedef struct cy_stc_scb_i2c_master_xfer_config
{
    /** The 7-bit right justified slave address to communicate with */
    uint8_t  slaveAddress;

    /**
    * The pointer to the buffer for data to read from the slave or with
    * data to write into the slave
    */
    uint8_t  *buffer;

    /** The size of the buffer */
    uint32_t bufferSize;

    /**
    * The transfer operation is pending - the stop condition will not
    * be generated
    */
    bool     xferPending;
} cy_stc_scb_i2c_master_xfer_config_t;
/** \} group_scb_i2c_data_structures */


/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_scb_i2c_general_functions
* \{
*/
cy_en_scb_i2c_status_t Cy_SCB_I2C_Init(volatile stc_SCB_t *base, cy_stc_scb_i2c_config_t const *config,
                                       cy_stc_scb_i2c_context_t *context);
void Cy_SCB_I2C_DeInit(volatile stc_SCB_t *base);
__STATIC_INLINE void Cy_SCB_I2C_Enable(volatile stc_SCB_t *base);
void Cy_SCB_I2C_Disable(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);

uint32_t Cy_SCB_I2C_SetDataRate(volatile stc_SCB_t *base, uint32_t dataRateHz, uint32_t scbClockHz);
uint32_t Cy_SCB_I2C_GetDataRate(volatile stc_SCB_t const *base, uint32_t scbClockHz);

__STATIC_INLINE void     Cy_SCB_I2C_SlaveSetAddress(volatile stc_SCB_t *base, uint8_t addr);
__STATIC_INLINE uint32_t Cy_SCB_I2C_SlaveGetAddress(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_I2C_SlaveSetAddressMask(volatile stc_SCB_t *base, uint8_t addrMask);
__STATIC_INLINE uint32_t Cy_SCB_I2C_SlaveGetAddressMask(volatile stc_SCB_t const *base);

__STATIC_INLINE void Cy_SCB_I2C_MasterSetLowPhaseDutyCycle (volatile stc_SCB_t *base, uint32_t clockCycles);
__STATIC_INLINE void Cy_SCB_I2C_MasterSetHighPhaseDutyCycle(volatile stc_SCB_t *base, uint32_t clockCycles);

__STATIC_INLINE bool Cy_SCB_I2C_IsBusBusy(volatile stc_SCB_t const *base);
/** \} group_scb_i2c_general_functions */

/**
* \addtogroup group_scb_i2c_slave_functions
* \{
*/
void Cy_SCB_I2C_SlaveConfigReadBuf (volatile stc_SCB_t const *base, uint8_t *rdBuf, uint32_t size, 
                                    cy_stc_scb_i2c_context_t *context);
void Cy_SCB_I2C_SlaveAbortRead     (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);
void Cy_SCB_I2C_SlaveConfigWriteBuf(volatile stc_SCB_t const *base, uint8_t *wrBuf, uint32_t size, 
                                    cy_stc_scb_i2c_context_t *context);
void Cy_SCB_I2C_SlaveAbortWrite    (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);

uint32_t Cy_SCB_I2C_SlaveGetStatus       (volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context);
uint32_t Cy_SCB_I2C_SlaveClearReadStatus (volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t *context);
uint32_t Cy_SCB_I2C_SlaveClearWriteStatus(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t *context);

uint32_t Cy_SCB_I2C_SlaveGetReadTransferCount (volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context);
uint32_t Cy_SCB_I2C_SlaveGetWriteTransferCount(volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context);
/** \} group_scb_i2c_slave_functions */

/**
* \addtogroup group_scb_i2c_master_high_level_functions
* \{
*/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterWrite(volatile stc_SCB_t *base, cy_stc_scb_i2c_master_xfer_config_t *masterXferConfig,
                                              cy_stc_scb_i2c_context_t *context);
void     Cy_SCB_I2C_MasterAbortWrite         (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);                                              
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterRead (volatile stc_SCB_t *base, cy_stc_scb_i2c_master_xfer_config_t* masterXferConfig,
                                              cy_stc_scb_i2c_context_t *context);
void     Cy_SCB_I2C_MasterAbortRead          (volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);                                           
uint32_t Cy_SCB_I2C_MasterGetStatus          (volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context);
uint32_t Cy_SCB_I2C_MasterGetTransferCount   (volatile stc_SCB_t const *base, cy_stc_scb_i2c_context_t const *context);
/** \} group_scb_i2c_master_low_high_functions */

/**
* \addtogroup group_scb_i2c_master_low_level_functions
* \{
*/
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendStart  (volatile stc_SCB_t *base, uint32_t address, uint32_t bitRnW, 
                                                    uint32_t timeoutMs, cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendReStart(volatile stc_SCB_t *base, uint32_t address, uint32_t bitRnW, 
                                                    uint32_t timeoutMs, cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendStop   (volatile stc_SCB_t *base,uint32_t timeoutMs, 
                                                    cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendWriteStop(volatile stc_SCB_t *base,uint32_t timeoutMs, cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterSendReadStop(volatile stc_SCB_t *base,uint32_t timeoutMs, cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterReadByte   (volatile stc_SCB_t *base, uint32_t ackNack, uint8_t *byte,
                                                    uint32_t timeoutMs, cy_stc_scb_i2c_context_t *context);
cy_en_scb_i2c_status_t Cy_SCB_I2C_MasterWriteByte  (volatile stc_SCB_t *base, uint8_t theByte, uint32_t timeoutMs,
                                                    cy_stc_scb_i2c_context_t *context);
/** \} group_scb_i2c_master_low_level_functions */

/**
* \addtogroup group_scb_i2c_interrupt_functions
* \{
*/
void Cy_SCB_I2C_Interrupt(volatile stc_SCB_t *base, cy_stc_scb_i2c_context_t *context);

__STATIC_INLINE void Cy_SCB_I2C_RegisterEventCallback(volatile stc_SCB_t const *base, scb_i2c_handle_events_t callback,
                                                      cy_stc_scb_i2c_context_t *context);
__STATIC_INLINE void Cy_SCB_I2C_RegisterAddrCallback (volatile stc_SCB_t const *base, scb_i2c_handle_addr_t callback,
                                                      cy_stc_scb_i2c_context_t *context);
/** \} group_scb_i2c_interrupt_functions */
                                                      
/**
* \addtogroup group_scb_i2c_low_power_functions
* \{
*/
cy_en_syspm_status_t Cy_I2C_DeepSleepCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode);
cy_en_syspm_status_t Cy_I2C_HibernateCallback(void *base, void *context, cy_en_syspm_callback_mode_t mode);
/** \} group_scb_i2c_low_power_functions */


/***************************************
*            API Constants
***************************************/

/**
* \addtogroup group_scb_i2c_macro
* \{
*/

/**
* \defgroup group_scb_i2c_macro_modes I2C Modes
* \{
*/
#define CY_SCB_I2C_SLAVE           (1u)    /**< Configures SCB for I2C Slave operation */
#define CY_SCB_I2C_MASTER          (2u)    /**< Configures SCB for I2C Master operation */
#define CY_SCB_I2C_MASTER_SLAVE    (3u)    /**< Configures SCB for I2C Master-Slave operation */
/** \} group_scb_i2c_macro_modes */

/**
* \defgroup group_scb_i2c_macro_direction I2C Direction
* \{
*/
#define CY_SCB_I2C_WRITE_XFER  (0ul)   /**< Current transaction is Write */
#define CY_SCB_I2C_READ_XFER   (1ul)   /**< Current transaction is Read */
/** \} group_scb_i2c_macro_direction */

/**
* \defgroup group_scb_i2c_macro_ack_nack I2C ACK NAK
* \{
*/
#define CY_SCB_I2C_ACK (SCB_I2C_S_CMD_S_ACK_Msk)  /**< Send ACK to current byte */
#define CY_SCB_I2C_NAK (SCB_I2C_S_CMD_S_NACK_Msk) /**< Send NAK to current byte */
/** \} group_scb_i2c_macro_ack_nack */

/**
* \defgroup group_scb_i2c_macro_slave_status I2C Slave Status
* Each I2C slave status is encoded in a separate bit, therefore multiple bits
* may be set to indicate the current status.
* \{
*/
/** There is a Read transaction in progress */
#define CY_SCB_I2C_SLAVE_RD_BUSY       (0x00000001ul)

/**
* All read data has been loaded into the TX FIFO, applicable only if
* the TX FIFO is used
*/
#define CY_SCB_I2C_SLAVE_RD_IN_FIFO    (0x00000002ul)

/**
* The master has finished reading data from the slave
*/
#define CY_SCB_I2C_SLAVE_RD_CMPLT      (0x00000004ul)

/**
* Set when the master tried to read more bytes than available in the configured
* read buffer. The slave is not able to finish the transaction and sends
* \ref CY_SCB_I2C_DEFAULT_TX.
*/
#define CY_SCB_I2C_SLAVE_RD_UNDRFL     (0x00000008ul)

/** There is a Write transaction in progress */
#define CY_SCB_I2C_SLAVE_WR_BUSY       (0x00000010ul)

/**
* The master has finished writing data into the slave
*/
#define CY_SCB_I2C_SLAVE_WR_CMPLT      (0x00000020ul)

/**
* The master attempted to write more bytes than space available in the
* configured Write buffer. Note that all subsequent bytes are dropped.
*/
#define CY_SCB_I2C_SLAVE_WR_OVRFL      (0x00000040ul)

/** The slave lost arbitration, the transaction was aborted */
#define CY_SCB_I2C_SLAVE_ARB_LOST      (0x00000080ul)

/**
* The slave captured an error on the bus during a master transaction (sources
* of error is misplaced Start or Stop).
*/
#define CY_SCB_I2C_SLAVE_BUS_ERR       (0x00000100ul)
/** \} group_scb_i2c_macro_slave_status */

/**
* \defgroup group_scb_i2c_macro_master_status I2C Master Status
* Each I2C master status is encoded in a separate bit, therefore multiple
* bits may be set to indicate the current status.
* \{
*/

/**
* The master is busy executing operation started by \ref Cy_SCB_I2C_MasterRead
* or \ref Cy_SCB_I2C_MasterWrite
*/
#define CY_SCB_I2C_MASTER_BUSY         (0x00010000ul)

/**
* All Write data has been loaded into the TX FIFO
*/
#define CY_SCB_I2C_MASTER_WR_IN_FIFO   (0x00020000ul)

/** The slave NACKed the address. */
#define CY_SCB_I2C_MASTER_ADDR_NAK     (0x00100000ul)

/** Write completed before all bytes were sent (last byte was NAKed)
*/
#define CY_SCB_I2C_MASTER_DATA_NAK     (0x00200000ul)

/** The master lost arbitration, the transaction was aborted */
#define CY_SCB_I2C_MASTER_ARB_LOST     (0x00400000ul)

/**
* The master detected an erroneous start or stop, the transaction was aborted
*/
#define CY_SCB_I2C_MASTER_BUS_ERR      (0x00800000ul)

/**
* The master transaction was aborted and the slave transaction is on-going
* because the slave was addressed before the master generated a start
*/
#define CY_SCB_I2C_MASTER_ABORT_START  (0x01000000ul)
/** \} group_scb_i2c_macro_master_status */

/**
* \defgroup group_scb_i2c_macro_callback_events I2C Callback Events
* \{
* Each event is encoded in a separate bit, and therefore it is possible to
* notify about multiple events.
*/
/**
* Indicates that the slave was addressed and the master wants to read data.
* This event can be used to configure the slave Read buffer.
*/
#define CY_SCB_I2C_SLAVE_READ_EVENT            (0x00000001ul)

/**
* Indicates that the slave was addressed and the master wants to write data.
* This event can be used to configure the slave Write buffer.
*/
#define CY_SCB_I2C_SLAVE_WRITE_EVENT           (0x00000002ul)

/**
* All slave data from the configured Read buffer has been loaded into the
* TX FIFO. The content of the Read buffer can be modified. Applicable only
* if the TX FIFO is used.
*/
#define CY_SCB_I2C_SLAVE_RD_IN_FIFO_EVENT      (0x00000004ul)

/**
* The master has read all data out of the configured Read buffer.
* This event can be used to configure the next Read buffer. If the buffer
* remains empty, the \ref CY_SCB_I2C_DEFAULT_TX bytes are returned to the master.
*/
#define CY_SCB_I2C_SLAVE_RD_BUF_EMPTY_EVENT    (0x00000008ul)

/**
* Indicates the master completed reading from the slave (set by the master NAK
* or Stop)
*/
#define CY_SCB_I2C_SLAVE_RD_CMPLT_EVENT        (0x00000010ul)

/**
* Indicates the master completed writing to the slave (set by the master Stop
* or Restart)
*/
#define CY_SCB_I2C_SLAVE_WR_CMPLT_EVENT        (0x00000020ul)

/**
* Indicates the I2C hardware detected an error.
* Check \ref Cy_SCB_I2C_SlaveGetStatus to determine the source of the error.
*/
#define CY_SCB_I2C_SLAVE_ERR_EVENT             (0x00000040ul)

/**
* All data specified by \ref Cy_SCB_I2C_MasterWrite have been loaded
* into the TX FIFO. The content of the master buffer can be modified.
* Applicable only if the TX FIFO is used.
*/
#define CY_SCB_I2C_MASTER_WR_IN_FIFO_EVENT     (0x00010000ul)

/** The master write started by \ref Cy_SCB_I2C_MasterWrite is complete */
#define CY_SCB_I2C_MASTER_WR_CMPLT_EVENT       (0x00020000ul)

/** The master read started by \ref Cy_SCB_I2C_MasterRead is complete */
#define CY_SCB_I2C_MASTER_RD_CMPLT_EVENT       (0x00040000ul)

/**
* Indicates the I2C hardware detected an error. It occurs together with
* \ref CY_SCB_I2C_MASTER_RD_CMPLT_EVENT or \ref CY_SCB_I2C_MASTER_WR_CMPLT_EVENT
* depends on the direction of the transfer.
* Check \ref Cy_SCB_I2C_MasterGetStatus to determine the source of the error.
*/
#define CY_SCB_I2C_MASTER_ERR_EVENT            (0x00080000ul)
/** \} group_scb_i2c_macro_callback_events */

/**
* \defgroup group_scb_i2c_macro_addr_callback_events I2C Address Callback Events
* Each event is encoded in a separate bit and therefore it is possible to
* notify about multiple events.
* \{
*/
/**
* Indicates the slave was addressed by the general call address
*/
#define CY_SCB_I2C_GENERAL_CALL_EVENT      (0x01ul)

/**
* The slave address is in the RX FIFO.
* Note that the address must be read from the RX FIFO using the
* \ref Cy_SCB_ReadRxFifo function.
*/
#define CY_SCB_I2C_ADDR_IN_FIFO_EVENT      (0x02ul)
/** \} group_scb_i2c_macro_addr_callback_events */

/**
* This value is returned by the slave when there is no data in the
* Read buffer
*/
#define CY_SCB_I2C_DEFAULT_TX  (0xFFul)


/***************************************
*         Internal Constants
***************************************/

/** \cond INTERNAL */

/* Slave statuses */
#define CY_SCB_I2C_SLAVE_RD_CLEAR  (CY_SCB_I2C_SLAVE_RD_CMPLT  | CY_SCB_I2C_SLAVE_RD_IN_FIFO | \
                                    CY_SCB_I2C_SLAVE_RD_UNDRFL | CY_SCB_I2C_SLAVE_ARB_LOST   | \
                                    CY_SCB_I2C_SLAVE_BUS_ERR)

#define CY_SCB_I2C_SLAVE_WR_CLEAR  (CY_SCB_I2C_SLAVE_WR_CMPLT | CY_SCB_I2C_SLAVE_WR_OVRFL | \
                                    CY_SCB_I2C_SLAVE_ARB_LOST | CY_SCB_I2C_SLAVE_BUS_ERR)

/* Master error statuses */
#define CY_SCB_I2C_MASTER_ERR (CY_SCB_I2C_MASTER_ABORT_START | CY_SCB_I2C_MASTER_ADDR_NAK | \
                               CY_SCB_I2C_MASTER_DATA_NAK    | CY_SCB_I2C_MASTER_BUS_ERR  | \
                               CY_SCB_I2C_MASTER_ARB_LOST)

/* Master interrupt masks */
#define CY_SCB_I2C_MASTER_INTR     (CY_SCB_MASTER_INTR_I2C_ARB_LOST | CY_SCB_MASTER_INTR_I2C_BUS_ERROR | \
                                    CY_SCB_MASTER_INTR_I2C_NACK     | CY_SCB_MASTER_INTR_I2C_STOP)          // 0x00000001, 0x00000100, 0x00000002, 0x00000010

#define CY_SCB_I2C_MASTER_INTR_ALL   (CY_SCB_I2C_MASTER_INTR | CY_SCB_MASTER_INTR_I2C_ACK)                  // CY_SCB_I2C_MASTER_INTR, 0x00000004

#define CY_SCB_I2C_MASTER_INTR_ERR   (CY_SCB_MASTER_INTR_I2C_BUS_ERROR | CY_SCB_MASTER_INTR_I2C_ARB_LOST)   // 0x00000100, 0x00000001

#define CY_SCB_I2C_MASTER_INTR_CMPLT (CY_SCB_I2C_MASTER_INTR_ERR | CY_SCB_MASTER_INTR_I2C_STOP)             // CY_SCB_I2C_MASTER_INTR_ERR, 0x00000010

/* Master statuses. */
#define CY_SCB_I2C_MASTER_TX_BYTE_DONE (CY_SCB_MASTER_INTR_I2C_ACK       | CY_SCB_MASTER_INTR_I2C_NACK | \
                                        CY_SCB_MASTER_INTR_I2C_BUS_ERROR | CY_SCB_MASTER_INTR_I2C_ARB_LOST) // 0x00000004, 0x00000002, 0x00000001, 0x00000100

#define CY_SCB_I2C_MASTER_RX_BYTE_DONE (CY_SCB_MASTER_INTR_I2C_BUS_ERROR | CY_SCB_MASTER_INTR_I2C_ARB_LOST) // 0x00000001, 0x00000100

#define CY_SCB_I2C_MASTER_STOP_DONE    (CY_SCB_MASTER_INTR_I2C_STOP      | \
                                        CY_SCB_MASTER_INTR_I2C_BUS_ERROR | CY_SCB_MASTER_INTR_I2C_ARB_LOST) // 0x00000010, 0x00000100, 0x00000001

#define CY_SCB_I2C_MASTER_TIMEOUT_DONE (0x80000000ul)

/* The slave interrupt mask */
#define CY_SCB_I2C_SLAVE_INTR      (CY_SCB_SLAVE_INTR_I2C_ADDR_MATCH | CY_SCB_SLAVE_INTR_I2C_GENERAL_ADDR | \
                                    CY_SCB_SLAVE_INTR_I2C_STOP       | CY_SCB_SLAVE_INTR_I2C_BUS_ERROR    | \
                                    CY_SCB_SLAVE_INTR_I2C_ARB_LOST)                                             // 0x00000040, 0x00000080, 0x00000010, 0x00000100, 0x00000001

#define CY_SCB_I2C_SLAVE_INTR_NO_STOP   (CY_SCB_I2C_SLAVE_INTR & (uint32_t) ~CY_SCB_SLAVE_INTR_I2C_STOP)        // CY_SCB_I2C_SLAVE_INTR & ~0x00000010

#define CY_SCB_I2C_SLAVE_INTR_ADDR      (CY_SCB_SLAVE_INTR_I2C_ADDR_MATCH | CY_SCB_SLAVE_INTR_I2C_GENERAL_ADDR) // 0x00000040, 0x00000080

#define CY_SCB_I2C_SLAVE_ADDR_DONE      (CY_SCB_I2C_SLAVE_INTR_ADDR)

#define CY_SCB_I2C_SLAVE_INTR_NO_ADDR   (CY_SCB_I2C_SLAVE_INTR & (uint32_t) ~CY_SCB_I2C_SLAVE_INTR_ADDR)

#define CY_SCB_I2C_SLAVE_INTR_TX        (CY_SCB_TX_INTR_LEVEL | CY_SCB_TX_INTR_UNDERFLOW)

#define CY_SCB_I2C_SLAVE_INTR_ERROR     (CY_SCB_SLAVE_INTR_I2C_BUS_ERROR | CY_SCB_SLAVE_INTR_I2C_ARB_LOST)      // 0x00000100, 0x00000001

/* I2C states */
#define CY_SCB_I2C_IDLE              (0x10000000ul)
#define CY_SCB_I2C_IDLE_MASK         (0x10000000ul)

/* Master states */
#define CY_SCB_I2C_MASTER_ACTIVE     (0x00100000ul)
#define CY_SCB_I2C_MASTER_WAIT       (0x10100000ul)
#define CY_SCB_I2C_MASTER_RX0        (0x00110000ul)
#define CY_SCB_I2C_MASTER_RX1        (0x00120000ul)
#define CY_SCB_I2C_MASTER_ADDR       (0x10130000ul)
#define CY_SCB_I2C_MASTER_TX         (0x00140000ul)
#define CY_SCB_I2C_MASTER_TX_DONE    (0x00150000ul)
#define CY_SCB_I2C_MASTER_STOP       (0x00160000ul)
#define CY_SCB_I2C_MASTER_WAIT_STOP  (0x00170000ul)
#define CY_SCB_I2C_MASTER_CMPLT      (0x00180000ul)

/* Slave states */
#define CY_SCB_I2C_SLAVE_ACTIVE      (0x00001000ul)
#define CY_SCB_I2C_SLAVE_RX_MASK     (0x00001001ul)
#define CY_SCB_I2C_SLAVE_RX          (0x00001001ul)
#define CY_SCB_I2C_SLAVE_TX          (0x00001002ul)

/* FIFO size */
#define CY_SCB_I2C_FIFO_SIZE         (CY_SCB_FIFO_SIZE)
#define CY_SCB_I2C_HALF_FIFO_SIZE    (CY_SCB_FIFO_SIZE / 2ul)

#define CY_SCB_I2C_DEFAULT_RETURN    (0xFFul)

/* I2C data rates max (Hz): standard, fast and fast plus modes */
#define CY_SCB_I2C_STD_DATA_RATE    (100000u)
#define CY_SCB_I2C_FST_DATA_RATE    (400000u)
#define CY_SCB_I2C_FSTP_DATA_RATE  (1000000u)

/* Slave clock limits */
#define CY_SCB_I2C_SLAVE_STD_CLK_MIN    (1550000u)
#define CY_SCB_I2C_SLAVE_STD_CLK_MAX   (12800000u)
#define CY_SCB_I2C_SLAVE_FST_CLK_MIN    (7820000u)
#define CY_SCB_I2C_SLAVE_FST_CLK_MAX   (15380000u)
#define CY_SCB_I2C_SLAVE_FSTP_CLK_MIN  (15840000u)
#define CY_SCB_I2C_SLAVE_FSTP_CLK_MAX  (89000000u)

/* Master clock (Hz) and duty cycle limits for standard mode */
#define CY_SCB_I2C_MASTER_STD_CLK_MIN           (1550000u)
#define CY_SCB_I2C_MASTER_STD_CLK_MAX           (3200000u)
#define CY_SCB_I2C_MASTER_STD_LOW_PHASE_MIN     (8u)
#define CY_SCB_I2C_MASTER_STD_HIGH_PHASE_MIN    (8u)

/* Master clock (Hz) and duty cycle limits for fast mode */
#define CY_SCB_I2C_MASTER_FST_CLK_MIN           (7820000u)
#define CY_SCB_I2C_MASTER_FST_CLK_MAX           (10000000u)
#define CY_SCB_I2C_MASTER_FST_LOW_PHASE_MIN     (13u)
#define CY_SCB_I2C_MASTER_FST_HIGH_PHASE_MIN    (8u)

/* Master clock (Hz) and duty cycle limits for fast plus mode */
#define CY_SCB_I2C_MASTER_FSTP_CLK_MIN          (14320000u)
#define CY_SCB_I2C_MASTER_FSTP_CLK_MAX          (25800000u)
#define CY_SCB_I2C_MASTER_FSTP_LOW_PHASE_MIN    (9u)
#define CY_SCB_I2C_MASTER_FSTP_HIGH_PHASE_MIN   (6u)

/* SCL low and high time in ns. Takes into account tF and tR */
#define CY_SCB_I2C_MASTER_STD_SCL_LOW   (5000u) /* tLOW  + tF = 4700 + 300  */
#define CY_SCB_I2C_MASTER_STD_SCL_HIGH  (5000u) /* tHIGH + tR = 4000 + 1000 */
#define CY_SCB_I2C_MASTER_FST_SCL_LOW   (1600u) /* tLOW  + tF = 1300 + 300  */
#define CY_SCB_I2C_MASTER_FST_SCL_HIGH   (900u) /* tHIGH + tR = 600 + 300   */
#define CY_SCB_I2C_MASTER_FSTP_SCL_LOW   (620u) /* tLOW  + tF = 500 + 120   */
#define CY_SCB_I2C_MASTER_FSTP_SCL_HIGH  (380u) /* tHIGH + tR = 260 + 120   */

/* Master duty cycle limits */
#define CY_SCB_I2C_LOW_PHASE_MAX    (16u)
#define CY_SCB_I2C_HIGH_PHASE_MAX   (16u)
#define CY_SCB_I2C_DUTY_CYCLE_MAX   (CY_SCB_I2C_LOW_PHASE_MAX + CY_SCB_I2C_HIGH_PHASE_MAX)

/* Analog filter settings. */
#define CY_SCB_I2C_ENABLE_ANALOG_FITLER    (CY_SCB_I2C_CFG_DEF_VAL)
#define CY_SCB_I2C_DISABLE_ANALOG_FITLER   (CY_SCB_I2C_CFG_DEF_VAL & \
                                            (uint32_t) ~(SCB_I2C_CFG_SDA_IN_FILT_SEL_Msk | SCB_I2C_CFG_SCL_IN_FILT_SEL_Msk))
/** \endcond */

/** \} group_scb_i2c_macro */


/***************************************
*    In-line Function Implementation
***************************************/

/**
* \addtogroup group_scb_i2c_general_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SCB_I2C_Enable
****************************************************************************//**
*
* Enables the SCB block for the I2C operation
*
* \param base
* The pointer to the I2C SCB instance.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_Enable(volatile stc_SCB_t *base)
{
    base->unCTRL.stcField.u1ENABLED = 1ul;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_IsBusBusy
****************************************************************************//**
*
* Checks if the I2C bus is busy.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \return
* A bus status: busy or not busy.
*
* \note
* After the SCB block is enabled or reset, the valid bus busy-status returns
* after half of the SCL period.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SCB_I2C_IsBusBusy(volatile stc_SCB_t const *base)
{
    return (base->unI2C_STATUS.stcField.u1BUS_BUSY != 0ul);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveSetAddress
****************************************************************************//**
*
* Sets the slave address for the I2C slave.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param addr
* The 7-bit right justified slave address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_SlaveSetAddress(volatile stc_SCB_t *base, uint8_t addr)
{
    base->unRX_MATCH.stcField.u8ADDR    = (uint32_t)((uint32_t) addr << 1ul);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveGetAddress
****************************************************************************//**
*
* Returns the slave address of the I2C slave.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \return
* The 7-bit right justified slave address.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_I2C_SlaveGetAddress(volatile stc_SCB_t const *base)
{
    return ((base->unRX_MATCH.stcField.u8ADDR) >> 1ul);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveSetAddressMask
****************************************************************************//**
*
* Sets the slave address mask for the I2C slave. The LSBit must always be 0.
* In all other bit positions a 1 indicates that the incoming address must match
* the corresponding bit in the slave address. A 0 in the mask means that the
* incoming address does not need to match.
* Example Slave Address = 0x0c. Slave Address Mask = 0x08. This means that the
* hardware will accept both 0x08 and 0x0c as valid addresses.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param addrMask
* The 8-bit address mask, the upper 7 bits correspond to the slave address.
* LSBit must always be 0.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_SlaveSetAddressMask(volatile stc_SCB_t *base, uint8_t addrMask)
{
    base->unRX_MATCH.stcField.u8MASK = (uint32_t) addrMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_SlaveGetAddressMask
****************************************************************************//**
*
* Returns the slave address mask.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \return
* The 8-bit address mask, the upper 7 bits correspond to the slave address.
* LSBit must always be 0.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_I2C_SlaveGetAddressMask(volatile stc_SCB_t const *base)
{
    return (base->unRX_MATCH.stcField.u8MASK);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterSetLowPhaseDutyCycle
****************************************************************************//**
*
* This function sets the number of SCB clock cycles in the low phase of SCL.
* If \ref Cy_SCB_I2C_SetDataRate is called after this function, the values
* specified in this function are overwritten.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param clockCycles
* The number of SCB clock cycles in the low phase of SCL.
* The valid range is 7 to 16.
*
* \note
* This function should be used at your own risk. Changing the number of clock
* cycles in a phase of SCL may violate the I2C specification. Make this
* change only while the block is disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_MasterSetLowPhaseDutyCycle(volatile stc_SCB_t *base, uint32_t clockCycles)
{
    base->unI2C_CTRL.stcField.u4LOW_PHASE_OVS = (clockCycles - 1ul);
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_MasterSetHighPhaseDutyCycle
****************************************************************************//**
*
* This function sets the number of SCB clock cycles in the high phase of SCL.
* If \ref Cy_SCB_I2C_SetDataRate is called after this function, the values
* specified in this function get overwritten.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param clockCycles
* The number of SCB clock cycles in the high phase of SCL.
* The valid range is 5 to 16.
*
* \note
* This function should be used at your own risk. Changing the number of clock
* cycles in a phase of SCL may violate the I2C specification. Make this
* change only while the block is disabled.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_MasterSetHighPhaseDutyCycle(volatile stc_SCB_t *base, uint32_t clockCycles)
{
    base->unI2C_CTRL.stcField.u4HIGH_PHASE_OVS = (clockCycles - 1ul);
}
/** \} group_scb_i2c_general_functions */

/**
* \addtogroup group_scb_i2c_interrupt_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SCB_I2C_RegisterEventCallback
****************************************************************************//**
*
* Registers a callback function that notifies that
* \ref group_scb_i2c_macro_callback_events occurred in the
* \ref Cy_SCB_I2C_Interrupt.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param callback
* The pointer to a callback function.
* See \ref scb_i2c_handle_events_t for the function prototype.
*
* \param context
* The pointer to context structure \ref cy_stc_scb_i2c_context_t allocated by
* the user. The structure is used while the I2C operation for internal
* configuration and data keeping. The user should not modify anything in
* this structure.
*
* \note
* To remove the callback, pass NULL as the pointer to the callback function.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_RegisterEventCallback(volatile stc_SCB_t const *base,
            scb_i2c_handle_events_t callback, cy_stc_scb_i2c_context_t *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    context->cbEvents = callback;
}


/*******************************************************************************
* Function Name: Cy_SCB_I2C_RegisterAddrCallback
****************************************************************************//**
*
* Registers a callback function that notifies that
* \ref group_scb_i2c_macro_addr_callback_events occurred in the
* \ref Cy_SCB_I2C_Interrupt.
*
* \param base
* The pointer to the I2C SCB instance.
*
* \param callback
* The pointer to a callback function.
* See \ref scb_i2c_handle_addr_t for the function prototype.
*
* \param context
* The pointer to context structure \ref cy_stc_scb_i2c_context_t allocated by
* the user. The structure is used during the I2C operation for internal
* configuration and data keeping. The user should not modify anything in
* this structure.
*
* \note
* To remove the callback, pass NULL as the pointer to a callback function.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_I2C_RegisterAddrCallback(volatile stc_SCB_t const *base,
              scb_i2c_handle_addr_t callback, cy_stc_scb_i2c_context_t *context)
{
    /* Suppress a compiler warning about unused variables */
    (void) base;

    context->cbAddr = callback;
}
/** \} group_scb_i2c_interrupt_functions */

#if defined(__cplusplus)
}
#endif

/** \} group_scb_i2c */

#endif /* (CY_SCB_I2C_H) */


/* [] END OF FILE */

