/***************************************************************************//**
* \file cy_scb_common.h
* \version 1.0
*
* Provides common API declarations of the SCB driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_scb Serial Communication Block (SCB)
* \{
* \defgroup group_scb_common Common
* \defgroup group_scb_ezi2c  EZI2C (SCB)
* \defgroup group_scb_i2c    I2C (SCB)
* \defgroup group_scb_spi    SPI (SCB)
* \defgroup group_scb_uart   UART (SCB)
* \} */

/**
* \addtogroup group_scb_common
* \{
*
* Common API for the Serial Communication Block.
*
* This is the common API that provides an interface to the SCB hardware.
* The I2C, SPI, and UART drivers use this common API.
* Most users will use individual drivers and do not need to use the common
* API for the SCB. However, you can use the common SCB API to implement
* a custom driver based on the SCB hardware.
*
* \section group_scb_common_configuration Configuration Considerations
*
* This is not a driver and it does not require configuration.
*
* \section group_scb_common_more_information More Information
*
* Refer to the SCB chapter of the technical reference manual (TRM).
*
* \section group_scb_common_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_scb_common_macro Macro
* \defgroup group_scb_common_functions Functions
* \defgroup group_scb_common_data_structures Data Structures
*
*/

#if !defined(CY_SCB_COMMON_H)
#define CY_SCB_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"

#ifndef CY_IP_MXSCB
    #error "The SCB driver is not supported on this device"
#endif

#if defined(__cplusplus)
extern "C" {
#endif


/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_scb_common_functions
* \{
*/
__STATIC_INLINE uint32_t Cy_SCB_ReadRxFifo    (volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetRxFifoLevel(volatile stc_SCB_t *base, uint32_t rxLevel);
__STATIC_INLINE uint32_t Cy_SCB_GetNumInRxFifo(volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetRxSrValid  (volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearRxFifo   (volatile stc_SCB_t *base);

__STATIC_INLINE void     Cy_SCB_WriteTxFifo   (volatile stc_SCB_t *base, uint32_t data);
__STATIC_INLINE void     Cy_SCB_SetTxFifoLevel(volatile stc_SCB_t *base, uint32_t txLevel);
__STATIC_INLINE uint32_t Cy_SCB_GetNumInTxFifo(volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetTxSrValid  (volatile stc_SCB_t const *base);
__STATIC_INLINE bool     Cy_SCB_IsTxComplete  (volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearTxFifo   (volatile stc_SCB_t *base);

__STATIC_INLINE void     Cy_SCB_SetEzMode(volatile stc_SCB_t *base, bool ezMode);

__STATIC_INLINE uint32_t Cy_SCB_GetInterruptCause(volatile stc_SCB_t const *base);

__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetRxInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearRxInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE void     Cy_SCB_SetRxInterrupt      (volatile stc_SCB_t *base, uint32_t interruptMask);

__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetTxInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearTxInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE void     Cy_SCB_SetTxInterrupt      (volatile stc_SCB_t *base, uint32_t interruptMask);

__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetMasterInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearMasterInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE void     Cy_SCB_SetMasterInterrupt      (volatile stc_SCB_t *base, uint32_t interruptMask);

__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetSlaveInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearSlaveInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE void     Cy_SCB_SetSlaveInterrupt      (volatile stc_SCB_t *base, uint32_t interruptMask);

__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetI2CInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearI2CInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);

__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptStatus(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_SetSpiInterruptMask  (volatile stc_SCB_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptMask  (volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptStatusMasked(volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_ClearSpiInterrupt    (volatile stc_SCB_t *base, uint32_t interruptMask);


/***************************************
*     Internal Function Prototypes
***************************************/

/** \cond INTERNAL */
void     Cy_SCB_ReadArrayNoCheck  (volatile stc_SCB_t const *base, void *rxBuf, uint32_t size);
uint32_t Cy_SCB_ReadArray         (volatile stc_SCB_t const *base, void *rxBuf, uint32_t size);
void     Cy_SCB_ReadArrayBlocking (volatile stc_SCB_t const *base, void *rxBuf, uint32_t size);
uint32_t Cy_SCB_Write             (volatile stc_SCB_t *base, uint32_t data);
void     Cy_SCB_WriteArrayNoCheck (volatile stc_SCB_t *base, void *txBuf, uint32_t size);
uint32_t Cy_SCB_WriteArray        (volatile stc_SCB_t *base, void *txBuf, uint32_t size);
void     Cy_SCB_WriteArrayBlocking(volatile stc_SCB_t *base, void *txBuf, uint32_t size);
void     Cy_SCB_WriteString       (volatile stc_SCB_t *base, char_t const string[]);
void     Cy_SCB_WriteDefaultArrayNoCheck(volatile stc_SCB_t *base, uint32_t txData, uint32_t size);
uint32_t Cy_SCB_WriteDefaultArray (volatile stc_SCB_t *base, uint32_t txData, uint32_t size);

__STATIC_INLINE uint32_t Cy_SCB_GetFifoSize (volatile stc_SCB_t const *base);
__STATIC_INLINE void     Cy_SCB_FwBlockReset(volatile stc_SCB_t *base);
__STATIC_INLINE bool     Cy_SCB_IsRxDataWidthByte(volatile stc_SCB_t const *base);
__STATIC_INLINE bool     Cy_SCB_IsTxDataWidthByte(volatile stc_SCB_t const *base);
__STATIC_INLINE uint32_t Cy_SCB_GetRxFifoLevel   (volatile stc_SCB_t const *base);
/** \endcond */

/** \} group_scb_common_functions */

/***************************************
*            API Constants
***************************************/

/**
* \addtogroup group_scb_common_macro
* \{
*/

/** Driver major version */
#define CY_SCB_DRV_VERSION_MAJOR    (1)

/** Driver minor version */
#define CY_SCB_DRV_VERSION_MINOR    (0)

/** SCB driver identifier */
#define CY_SCB_ID   CY_PDL_DRV_ID(0x2Au)

/** Position for SCB driver sub mode */
#define CY_SCB_SUB_MODE_Pos (13ul)

/** EZI2C mode identifier */
#define CY_SCB_EZI2C_ID     (0x0ul << CY_SCB_SUB_MODE_Pos)

/** EZI2C mode identifier */
#define CY_SCB_I2C_ID       (0x1ul << CY_SCB_SUB_MODE_Pos)

/** EZI2C mode identifier */
#define CY_SCB_SPI_ID       (0x2ul << CY_SCB_SUB_MODE_Pos)

/** EZI2C mode identifier */
#define CY_SCB_UART_ID      (0x3ul << CY_SCB_SUB_MODE_Pos)

/**
* \defgroup group_scb_common_macro_intr_cause SCB Interrupt Causes
* \{
*/
/** Interrupt from Master interrupt sources */
#define CY_SCB_MASTER_INTR SCB_INTR_CAUSE_M_Msk

/** Interrupt from Slave interrupt sources  */
#define CY_SCB_SLAVE_INTR  SCB_INTR_CAUSE_S_Msk

/** Interrupt from TX interrupt sources */
#define CY_SCB_TX_INTR     SCB_INTR_CAUSE_TX_Msk

/** Interrupt from RX interrupt sources */
#define CY_SCB_RX_INTR     SCB_INTR_CAUSE_RX_Msk

/** Interrupt from I2C externally clocked interrupt sources */
#define CY_SCB_I2C_INTR    SCB_INTR_CAUSE_I2C_EC_Msk

/** Interrupt from SPI externally clocked interrupt sources */
#define CY_SCB_SPI_INTR    SCB_INTR_CAUSE_SPI_EC_Msk
/** \} group_scb_common_macro_intr_cause */

/**
* \defgroup group_scb_common_macro_tx_intr TX Interrupt Statuses
* \{
*/
/**
* The number of data elements in the TX FIFO is less than the value
* of the TX FIFO level
*/
#define CY_SCB_TX_INTR_LEVEL           SCB_INTR_TX_TRIGGER_Msk

/** The TX FIFO is not full */
#define CY_SCB_TX_INTR_NOT_FULL        SCB_INTR_TX_NOT_FULL_Msk

/** The TX FIFO is empty */
#define CY_SCB_TX_INTR_EMPTY           SCB_INTR_TX_EMPTY_Msk

/** An attempt to write to a full TX FIFO */
#define CY_SCB_TX_INTR_OVERFLOW        SCB_INTR_TX_OVERFLOW_Msk

/** An attempt to read from an empty TX FIFO */
#define CY_SCB_TX_INTR_UNDERFLOW       SCB_INTR_TX_UNDERFLOW_Msk

/** The UART transfer is complete: all data elements from the TX FIFO are sent
*/
#define CY_SCB_TX_INTR_UART_DONE       SCB_INTR_TX_UART_DONE_Msk

/** SmartCard only: UART received a NACK */
#define CY_SCB_TX_INTR_UART_NACK       SCB_INTR_TX_UART_NACK_Msk

/**
* SmartCard only: the value on the TX line of the UART does not match the
* value on the RX line
*/
#define CY_SCB_TX_INTR_UART_ARB_LOST   SCB_INTR_TX_UART_ARB_LOST_Msk
/** \} group_scb_common_macro_tx_intr */

/**
* \defgroup group_scb_common_macro_rx_intr RX Interrupt Statuses
* \{
*/
/**
* The number of data elements in the RX FIFO is greater than the value of the
* RX FIFO level
*/
#define CY_SCB_RX_INTR_LEVEL             SCB_INTR_RX_TRIGGER_Msk

/** The RX FIFO is not empty */
#define CY_SCB_RX_INTR_NOT_EMPTY         SCB_INTR_RX_NOT_EMPTY_Msk

/** The RX FIFO is full */
#define CY_SCB_RX_INTR_FULL              SCB_INTR_RX_FULL_Msk

/** An attempt to write to a full RX FIFO */
#define CY_SCB_RX_INTR_OVERFLOW          SCB_INTR_RX_OVERFLOW_Msk

/** An attempt to read from an empty RX FIFO */
#define CY_SCB_RX_INTR_UNDERFLOW         SCB_INTR_RX_UNDERFLOW_Msk

/** A UART framing error detected */
#define CY_SCB_RX_INTR_UART_FRAME_ERROR  SCB_INTR_RX_FRAME_ERROR_Msk

/** A UART parity error detected  */
#define CY_SCB_RX_INTR_UART_PARITY_ERROR SCB_INTR_RX_PARITY_ERROR_Msk

/** A UART break detected */
#define CY_SCB_RX_INTR_UART_BREAK_DETECT SCB_INTR_RX_BREAK_DETECT_Msk
/** \} group_scb_common_macro_rx_intr */

/**
* \defgroup group_scb_common_macro_slave_intr Slave Interrupt Statuses
* \{
*/
/**
* I2C slave lost arbitration: the value driven on the SDA line is not the same
* as the value observed on the SDA line
*/
#define CY_SCB_SLAVE_INTR_I2C_ARB_LOST      SCB_INTR_S_I2C_ARB_LOST_Msk

/** The I2C slave received a NAK */
#define CY_SCB_SLAVE_INTR_I2C_NACK          SCB_INTR_S_I2C_NACK_Msk

/** The I2C slave received  an ACK */
#define CY_SCB_SLAVE_INTR_I2C_ACK           SCB_INTR_S_I2C_ACK_Msk

/**
* A Stop or Repeated Start event for a write transfer intended for this slave
* was detected.
*/
#define CY_SCB_SLAVE_INTR_I2C_WRITE_STOP    SCB_INTR_S_I2C_WRITE_STOP_Msk

/** A Stop or Repeated Start event intended for this slave was detected */
#define CY_SCB_SLAVE_INTR_I2C_STOP          SCB_INTR_S_I2C_STOP_Msk

/** The I2C slave received a Start condition */
#define CY_SCB_SLAVE_INTR_I2C_START         SCB_INTR_S_I2C_START_Msk

/** The I2C slave received the matching address */
#define CY_SCB_SLAVE_INTR_I2C_ADDR_MATCH    SCB_INTR_S_I2C_ADDR_MATCH_Msk

/** The I2C Slave received the general call address */
#define CY_SCB_SLAVE_INTR_I2C_GENERAL_ADDR  SCB_INTR_S_I2C_GENERAL_Msk

/** The I2C slave bus error (detection of unexpected Start or Stop condition) */
#define CY_SCB_SLAVE_INTR_I2C_BUS_ERROR     SCB_INTR_S_I2C_BUS_ERROR_Msk

/**
* The SPI slave select line is deselected at an expected time during an
* SPI transfer.
*/
#define CY_SCB_SLAVE_INTR_SPI_BUS_ERROR     SCB_INTR_S_SPI_BUS_ERROR_Msk
/** \} group_scb_common_macro_slave_intr */

/**
* \defgroup group_scb_common_macro_master_intr Master Interrupt Statuses
* \{
*/
/** The I2C master's lost arbitration */
#define CY_SCB_MASTER_INTR_I2C_ARB_LOST    SCB_INTR_M_I2C_ARB_LOST_Msk

/** The I2C master received a NACK */
#define CY_SCB_MASTER_INTR_I2C_NACK        SCB_INTR_M_I2C_NACK_Msk

/** The I2C master received an ACK */
#define CY_SCB_MASTER_INTR_I2C_ACK         SCB_INTR_M_I2C_ACK_Msk

/** The I2C master generated a Stop */
#define CY_SCB_MASTER_INTR_I2C_STOP        SCB_INTR_M_I2C_STOP_Msk

/** The I2C master bus error (detection of unexpected START or STOP condition)
*/
#define CY_SCB_MASTER_INTR_I2C_BUS_ERROR   SCB_INTR_M_I2C_BUS_ERROR_Msk

/**
* The SPI master transfer is complete: all data elements transferred from the
* TX FIFO and TX shift register.
*/
#define CY_SCB_MASTER_INTR_SPI_DONE        SCB_INTR_M_SPI_DONE_Msk
/** \} group_scb_common_macro_master_intr */

/**
* \defgroup group_scb_common_macro_i2c_intr I2C Interrupt Statuses
* \{
*/
/**
* Wake up request: the I2C slave received the matching address.
* Note that this interrupt source triggers in active mode.
*/
#define CY_SCB_I2C_INTR_WAKEUP     SCB_INTR_I2C_EC_WAKE_UP_Msk
/** \} group_scb_common_macro_i2c_intr */

/**
* \defgroup group_scb_common_macro_SpiIntrStatuses SPI Interrupt Statuses
* \{
*/
/**
* Wake up request: the SPI slave detected an active edge of the slave select
* signal. Note that this interrupt source triggers in active mode.
*/
#define CY_SCB_SPI_INTR_WAKEUP     SCB_INTR_SPI_EC_WAKE_UP_Msk
/** \} group_scb_common_macro_SpiIntrStatuses */


/***************************************
*         Internal Constants
***************************************/

/** \cond INTERNAL */

/* Default registers values */
#define CY_SCB_CTRL_DEF_VAL         (0x0300000Ful)
#define CY_SCB_I2C_CTRL_DEF_VAL     (0x00003F88ul)
#define CY_SCB_I2C_CFG_DEF_VAL      (0x002A1013ul)
#define CY_SCB_SPI_CTRL_DEF_VAL     (0x03000000ul)
#define CY_SCB_UART_CTRL_DEF_VAL    (0x00300000ul)
#define CY_SCB_UART_RX_CTRL_DEF_VAL (0x000A0002ul)
#define CY_SCB_UART_TX_CTRL_DEF_VAL (0x00000002ul)
#define CY_SCB_RX_CTRL_DEF_VAL      (0x00000107ul)
#define CY_SCB_TX_CTRL_DEF_VAL      (0x00000107ul)

/* SCB CTRL modes */
#define CY_SCB_CTRL_MODE_I2C   (0ul)
#define CY_SCB_CTRL_MODE_SPI   (1ul)
#define CY_SCB_CTRL_MODE_UART  (2ul)

/* The position and mask to set the I2C mode */
#define CY_SCB_I2C_CTRL_MODE_Pos    SCB_I2C_CTRL_SLAVE_MODE_Pos
#define CY_SCB_I2C_CTRL_MODE_Msk    (SCB_I2C_CTRL_SLAVE_MODE_Msk | \
                                     SCB_I2C_CTRL_MASTER_MODE_Msk)
/* I2C has fixed data width */
#define CY_SCB_I2C_DATA_WIDTH   (7ul)

/* RX and TX control register values */
#define CY_SCB_I2C_RX_CTRL      (SCB_RX_CTRL_DATA_WIDTH_Msk | SCB_RX_CTRL_MSB_FIRST_Msk)
#define CY_SCB_I2C_TX_CTRL      (CY_SCB_I2C_RX_CTRL | SCB_TX_CTRL_OPEN_DRAIN_Msk)

/* The position and mask to make an address byte */
#define CY_SCB_I2C_ADDRESS_Pos  (1ul)
#define CY_SCB_I2C_ADDRESS_Msk  (0xFEul)

/* SPI slave select polarity */
#define CY_SCB_SPI_CTRL_SSEL_POLARITY_Pos   SCB_SPI_CTRL_SSEL_POLARITY0_Pos
#define CY_SCB_SPI_CTRL_SSEL_POLARITY_Msk   (SCB_SPI_CTRL_SSEL_POLARITY0_Msk | \
                                             SCB_SPI_CTRL_SSEL_POLARITY1_Msk | \
                                             SCB_SPI_CTRL_SSEL_POLARITY2_Msk | \
                                             SCB_SPI_CTRL_SSEL_POLARITY3_Msk)

/* SPI clock modes: CPHA and CPOL */
#define CY_SCB_SPI_CTRL_CLK_MODE_Pos    SCB_SPI_CTRL_CPHA_Pos
#define CY_SCB_SPI_CTRL_CLK_MODE_Msk    (SCB_SPI_CTRL_CPHA_Msk | SCB_SPI_CTRL_CPOL_Msk)

/* UART parity and parity enable combination */
#define CY_SCB_UART_RX_CTRL_SET_PARITY_Msk      (SCB_UART_RX_CTRL_PARITY_ENABLED_Msk | \
                                                 SCB_UART_RX_CTRL_PARITY_Msk)
#define CY_SCB_UART_RX_CTRL_SET_PARITY_Pos      SCB_UART_RX_CTRL_PARITY_Pos

#define CY_SCB_UART_TX_CTRL_SET_PARITY_Msk      (SCB_UART_TX_CTRL_PARITY_ENABLED_Msk | \
                                                    SCB_UART_TX_CTRL_PARITY_Msk)
#define CY_SCB_UART_TX_CTRL_SET_PARITY_Pos      SCB_UART_TX_CTRL_PARITY_Pos

/* Max number of bits for byte mode */
#define CY_SCB_BYTE_WIDTH     (8ul)
#define CY_SCB_HALFWORD_WIDTH (16ul)
#define CY_SCB_WORD_WIDTH     (32ul)

/* Single unit to wait */
#define CY_SCB_WAIT_1_UNIT  (1u)

/* Clear interrupt sources */
#define CY_SCB_CLEAR_ALL_INTR_SRC   (0ul)

/* Hardware FIFO size */
#define CY_SCB_FIFO_SIZE    (128ul)
/** \endcond */

/** \} group_scb_common_macro */


/***************************************
* Inline Function Implementation
***************************************/

/**
* \addtogroup group_scb_common_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SCB_ReadRxFifo
****************************************************************************//**
*
* Reads a data element directly out of the RX FIFO.
* This function does not check if the RX FIFO has data before reading it.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Data from RX FIFO.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_ReadRxFifo(volatile stc_SCB_t const *base)
{
    return (base->unRX_FIFO_RD.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SCB_SetRxFifoLevel
****************************************************************************//**
*
* Sets the RX FIFO level. When there are more data elements in the RX FIFO than
* this level, the RX FIFO level interrupt is triggered.
*
* \param base
* The pointer to the SCB instance.
*
* \param rxLevel
* When there are more data elements in the FIFO than this level, the RX level
* interrupt is triggered.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetRxFifoLevel(volatile stc_SCB_t *base, uint32_t rxLevel)
{
    base->unRX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL = rxLevel;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetNumInRxFifo
****************************************************************************//**
*
* Returns the number of data elements currently in the RX FIFO.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The number or data elements in RX FIFO.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetNumInRxFifo(volatile stc_SCB_t const *base)
{
    return (base->unRX_FIFO_STATUS.stcField.u9USED);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetRxSrValid
****************************************************************************//**
*
* Returns the status of the RX FIFO Shift Register valid bit.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* 1 - RX shift register valid, 0 - RX shift register not valid.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetRxSrValid(volatile stc_SCB_t const *base)
{
    return (base->unRX_FIFO_STATUS.stcField.u1SR_VALID);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearRxFifo
****************************************************************************//**
*
* Clears the RX FIFO and shifter.
*
* \param base
* The pointer to the SCB instance.
*
* \note
* If there is partial data in the shifter, it gets cleared and lost.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearRxFifo(volatile stc_SCB_t* base)
{
    base->unRX_FIFO_CTRL.stcField.u1CLEAR = 1ul;
    base->unRX_FIFO_CTRL.stcField.u1CLEAR = 0ul;
    (void) base->unRX_FIFO_CTRL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_WriteTxFifo
****************************************************************************//**
*
* Writes data directly into the TX FIFO.
* This function does not check if the TX FIFO is not full before writing
* into it.
*
* \param base
* The pointer to the SCB instance.
*
* \param data
* Data to write to the TX FIFO.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_WriteTxFifo(volatile stc_SCB_t* base, uint32_t data)
{
    base->unTX_FIFO_WR.u32Register = data;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetTxFifoLevel
****************************************************************************//**
*
* Sets the TX FIFO level, when there are fewer data elements in the TX FIFO than
* this level, the TX FIFO level interrupt is triggered.
*
* \param base
* The pointer to the SCB instance.
*
* \param txLevel
* When there are fewer data elements in the FIFO than this level, the TX level
* interrupt is triggered.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetTxFifoLevel(volatile stc_SCB_t *base, uint32_t txLevel)
{
    base->unTX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL    = txLevel;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetNumInTxFifo
****************************************************************************//**
*
* Returns the number of data elements currently in the TX FIFO.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The number or data elements in the TX FIFO.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetNumInTxFifo(volatile stc_SCB_t const *base)
{
    return (base->unTX_FIFO_STATUS.stcField.u9USED);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetTxSrValid
****************************************************************************//**
*
* Returns the status of the TX FIFO Shift Register valid bit.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* 1 - TX shift register valid, 0 - TX shift register not valid.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetTxSrValid(volatile stc_SCB_t const *base)
{
    return (base->unTX_FIFO_STATUS.stcField.u1SR_VALID);
}


/*******************************************************************************
* Function Name: Cy_SCB_IsTxComplete
****************************************************************************//**
*
* Checks if the TX FIFO and Shifter are empty and there is no more data to send.
*
* \param base
* Pointer to SPI the SCB instance.
*
* \return
* If true, transmission complete. If false, transmission is not complete.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SCB_IsTxComplete(volatile stc_SCB_t const *base)
{
     return (0ul == (Cy_SCB_GetNumInTxFifo(base) + Cy_SCB_GetTxSrValid(base)));
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearTxFifo
****************************************************************************//**
*
* Clears the TX FIFO.
*
* \param base
* The pointer to the SCB instance.
*
* \note
* The TX FIFO clear operation also clears the shift register, thus the shifter
* could be cleared in the middle of a data element transfer; resulting in
* "ones" being sent on the bus for the remainder of the transfer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearTxFifo(volatile stc_SCB_t *base)
{
    base->unTX_FIFO_CTRL.stcField.u1CLEAR = 1ul;
    base->unTX_FIFO_CTRL.stcField.u1CLEAR = 0ul;
    (void) base->unTX_FIFO_CTRL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetEzMode
****************************************************************************//**
*
* Sets EZ mode.
* EZ mode is only used for synchronous serial interface protocols: SPI and I2C.
* This mode is only applicable to slave functionality.
* In EZ mode, the slave can read from and write to an addressable memory 
* structure of up to 256 bytes.
*
* \param base
* The pointer to the SCB instance.
*
* \param ezMode
* If true, HW EZ mode is enable. If false, HW EZ mode is disable.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetEzMode(volatile stc_SCB_t *base, bool ezMode)
{
    if (ezMode)
    {
        base->unCTRL.stcField.u1EZ_MODE = 1ul;
    }
    else
    {
        base->unCTRL.stcField.u1EZ_MODE = 0ul;
    }
}


/*******************************************************************************
* Function Name: Cy_SCB_GetInterruptCause
****************************************************************************//**
*
* Returns the mask of bits showing the source of the current triggered
* interrupt. This is useful for modes of operation where an interrupt can
* be generated by conditions in multiple interrupt source registers.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The mask with the OR of the following conditions that have been triggered.
* See \ref group_scb_common_macro_intr_cause for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetInterruptCause(volatile stc_SCB_t const *base)
{
    return (base->unINTR_CAUSE.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetRxInterruptStatus
****************************************************************************//**
*
* Returns the RX interrupt request register. This register contains the current
* status of the RX interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of the RX interrupt sources. Each constant is a bit field
* value. The value returned may have multiple bits set to indicate the
* current status.
* See \ref group_scb_common_macro_rx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptStatus(volatile stc_SCB_t const *base)
{
    return (base->unINTR_RX.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetRxInterruptMask
****************************************************************************//**
*
* Writes the RX interrupt mask register. This register configures which bits
* from the RX interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Enabled RX interrupt sources.
* See \ref group_scb_common_macro_rx_intr.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetRxInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_RX_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetRxInterruptMask
****************************************************************************//**
*
* Returns the RX interrupt mask register. This register specifies which bits
* from the RX interrupt request register trigger can an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled RX interrupt sources..
* See \ref group_scb_common_macro_rx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_RX_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetRxInterruptStatusMasked
****************************************************************************//**
*
* Returns the RX interrupt masked request register. This register contains
* a logical AND of corresponding bits from the RX interrupt request and
* mask registers.
* This function is intended to be used in the interrupt service routine to
* identify which of the enabled RX interrupt sources caused the interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled RX interrupt sources.
* See \ref group_scb_common_macro_rx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetRxInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_RX_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearRxInterrupt
****************************************************************************//**
*
* Clears the RX interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The RX interrupt sources to be cleared.
* See \ref group_scb_common_macro_rx_intr for the set of constants.
*
* \note
*  - CY_SCB_INTR_RX_FIFO_LEVEL interrupt source is not cleared when
*    the RX FIFO has more entries than the level.
*  - CY_SCB_INTR_RX_NOT_EMPTY interrupt source is not cleared when the
*    RX FIFO is not empty.
*  - CY_SCB_INTR_RX_FULL interrupt source is not cleared when the
*    RX FIFO is full.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearRxInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_RX.u32Register = interruptMask;
    (void) base->unINTR_RX.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetRxInterrupt
****************************************************************************//**
*
* Sets the RX interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The RX interrupt sources to set in the RX interrupt request register.
* See \ref group_scb_common_macro_rx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetRxInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_RX_SET.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetTxInterruptStatus
****************************************************************************//**
*
* Returns the TX interrupt request register. This register contains the current
* status of the TX interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of TX interrupt sources.
* Each constant is a bit field value. The value returned may have multiple
* bits set to indicate the current status.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptStatus(volatile stc_SCB_t const *base)
{
    return (base->unINTR_TX.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetTxInterruptMask
****************************************************************************//**
*
* Writes the TX interrupt mask register. This register configures which bits
* from the TX interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Enabled TX interrupt sources.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetTxInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_TX_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetTxInterruptMask
****************************************************************************//**
*
* Returns the TX interrupt mask register. This register specifies which
* bits from the TX interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled TX interrupt sources.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_TX_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetTxInterruptStatusMasked
****************************************************************************//**
*
* Returns the TX interrupt masked request register. This register contains
* a logical AND of corresponding bits from the TX interrupt request and
* mask registers.
* This function is intended to be used in the interrupt service routine to
* identify which of enabled TX interrupt sources caused the interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled TX interrupt sources.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetTxInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_TX_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearTxInterrupt
****************************************************************************//**
*
* Clears the TX interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The TX interrupt sources to be cleared.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
* \note
*  - CY_SCB_INTR_TX_FIFO_LEVEL interrupt source is not cleared when the
*    TX FIFO has less entries than the TX level.
*  - CY_SCB_INTR_TX_NOT_FULL interrupt source is not cleared when the
*    TX FIFO has empty entries in the TX FIFO.
*  - CY_SCB_INTR_TX_EMPTY interrupt source is not cleared when the
*    TX FIFO is empty.
*  - CY_SCB_INTR_TX_UNDERFLOW interrupt source is not cleared when the
*    TX FIFO is empty. Put data into the TX FIFO before clearing it.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearTxInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_TX.u32Register = interruptMask;
    (void) base->unINTR_TX.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetTxInterrupt
****************************************************************************//**
*
* Sets TX interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The TX interrupt sources to set in the TX interrupt request register.
* See \ref group_scb_common_macro_tx_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetTxInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_TX_SET.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetMasterInterruptStatus
****************************************************************************//**
*
* Returns the master interrupt request register. This register contains the current
* status of the master interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of the master interrupt sources.
* Each constant is a bit field value. The value returned may have multiple
* bits set to indicate the current status.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptStatus (volatile stc_SCB_t const *base)
{
    return (base->unINTR_M.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetMasterInterruptMask
****************************************************************************//**
*
* Writes the master interrupt mask register. This register specifies which bits
* from the master interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The master interrupt sources to be enable.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetMasterInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_M_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetMasterInterruptMask
****************************************************************************//**
*
* Returns the master interrupt mask register. This register specifies which bits
* from the master interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled master interrupt sources.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_M_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetMasterInterruptStatusMasked
****************************************************************************//**
*
* Returns the master interrupt masked request register. This register contains a
* logical AND of corresponding bits from the master interrupt request and mask
* registers.
* This function is intended to be used in the interrupt service routine to
* identify which of the enabled master interrupt sources caused the interrupt
* event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled master interrupt sources.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetMasterInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_M_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearMasterInterrupt
****************************************************************************//**
*
* Clears master interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The master interrupt sources to be cleared.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearMasterInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_M.u32Register = interruptMask;
    (void) base->unINTR_M.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetMasterInterrupt
****************************************************************************//**
*
* Sets master interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The master interrupt sources to set in the master interrupt request register.
* See \ref group_scb_common_macro_master_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetMasterInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_M_SET.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSlaveInterruptStatus
****************************************************************************//**
*
* Returns the slave interrupt request register. This register contains the current
* status of the slave interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of the slave interrupt sources.
* Each constant is a bit field value. The value returned may have multiple
* bits set to indicate the current status.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptStatus(volatile stc_SCB_t const *base)
{
    return (base->unINTR_S.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetSlaveInterruptMask
****************************************************************************//**
*
* Writes slave interrupt mask register.
* This register specifies which bits from the slave interrupt request register
* can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Enabled slave interrupt sources.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetSlaveInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_S_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSlaveInterruptMask
****************************************************************************//**
*
* Returns the slave interrupt mask register.
* This register specifies which bits from the slave interrupt request register
* can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled slave interrupt sources.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_S_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSlaveInterruptStatusMasked
****************************************************************************//**
*
* Returns the slave interrupt masked request register. This register contains a
* logical AND of corresponding bits from the slave interrupt request and mask
* registers.
* This function is intended to be used in the interrupt service routine to
* identify which of enabled slave interrupt sources caused the interrupt
* event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled slave interrupt sources.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSlaveInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_S_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearSlaveInterrupt
****************************************************************************//**
*
* Clears the slave interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Slave interrupt sources to be cleared.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearSlaveInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_S.u32Register = interruptMask;
    (void) base->unINTR_S.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_SetSlaveInterrupt
****************************************************************************//**
*
* Sets slave interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The slave interrupt sources to set in the slave interrupt request register
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetSlaveInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_S_SET.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetI2CInterruptStatus
****************************************************************************//**
*
* Returns the I2C interrupt request register. This register contains the
* current status of the I2C interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of the I2C interrupt sources. Each constant is a bit
* field value.
* The value returned may have multiple bits set to indicate the current status.
* See \ref group_scb_common_macro_slave_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptStatus(volatile stc_SCB_t const *base)
{
    return (base->unINTR_I2C_EC.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetI2CInterruptMask
****************************************************************************//**
*
* Writes the I2C interrupt mask register. This register specifies which bits
* from the I2C interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Enabled I2C interrupt sources.
* See \ref group_scb_common_macro_i2c_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetI2CInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_I2C_EC_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetI2CInterruptMask
****************************************************************************//**
*
* Returns the I2C interrupt mask register. This register specifies which bits
* from the I2C interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled I2C interrupt sources.
* See \ref group_scb_common_macro_i2c_intr.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_I2C_EC_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetI2CInterruptStatusMasked
****************************************************************************//**
*
* Returns the I2C interrupt masked request register. This register contains
* a logical AND of corresponding bits from I2C interrupt request and mask
* registers.
* This function is intended to be used in the interrupt service routine to
* identify which of enabled I2C interrupt sources caused the interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled I2C interrupt sources.
* See \ref group_scb_common_macro_i2c_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetI2CInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_I2C_EC_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearI2CInterrupt
****************************************************************************//**
*
* Clears I2C interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The I2C interrupt sources to be cleared.
* See \ref group_scb_common_macro_i2c_intr for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearI2CInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_I2C_EC.u32Register = interruptMask;
    (void) base->unINTR_I2C_EC.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSpiInterruptStatus
****************************************************************************//**
*
* Returns the SPI interrupt request register. This register contains the current
* status of the SPI interrupt sources.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of SPI interrupt sources. Each constant is a bit field value.
* The value returned may have multiple bits set to indicate the current status
* See \ref group_scb_common_macro_SpiIntrStatuses for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptStatus(volatile stc_SCB_t const *base)
{
    return (base->unINTR_SPI_EC.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_SetSpiInterruptMask
****************************************************************************//**
*
* Writes the SPI interrupt mask register. This register specifies which
* bits from the SPI interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* Enabled SPI interrupt sources.
* See \ref group_scb_common_macro_SpiIntrStatuses for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_SetSpiInterruptMask(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_SPI_EC_MASK.u32Register = interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSpiInterruptMask
****************************************************************************//**
*
* Returns the SPI interrupt mask register. This register specifies which bits
* from the SPI interrupt request register can trigger an interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* Enabled SPI interrupt sources.
* See \ref group_scb_common_macro_SpiIntrStatuses for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptMask(volatile stc_SCB_t const *base)
{
    return (base->unINTR_SPI_EC_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_GetSpiInterruptStatusMasked
****************************************************************************//**
*
* Returns the SPI interrupt masked request register. This register contains
* a logical AND of corresponding bits from the SPI interrupt request and
* mask registers.
* This function is intended to be used in the interrupt service routine to
* identify which of enabled SPI interrupt sources caused the interrupt event.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* The current status of enabled SPI interrupt sources.
* See \ref group_scb_common_macro_SpiIntrStatuses for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetSpiInterruptStatusMasked(volatile stc_SCB_t const *base)
{
    return (base->unINTR_SPI_EC_MASKED.u32Register);
}


/*******************************************************************************
* Function Name: Cy_SCB_ClearSpiInterrupt
****************************************************************************//**
*
* Clears SPI interrupt sources in the interrupt request register.
*
* \param base
* The pointer to the SCB instance.
*
* \param interruptMask
* The SPI interrupt sources to be cleared.
* See \ref group_scb_common_macro_SpiIntrStatuses for the set of constants.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_ClearSpiInterrupt(volatile stc_SCB_t *base, uint32_t interruptMask)
{
    base->unINTR_SPI_EC.u32Register = interruptMask;
    (void) base->unINTR_SPI_EC.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetFifoSize
****************************************************************************//**
*
* Returns the RX and TX FIFO depth.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* FIFO depth.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetFifoSize(volatile stc_SCB_t const *base)
{
    return ((0ul == base->unCTRL.stcField.u2MEM_WIDTH) ?
               (CY_SCB_FIFO_SIZE) : (CY_SCB_FIFO_SIZE / 2ul));
}


/*******************************************************************************
* Function Name: Cy_SCB_IsRxDataWidthByte
****************************************************************************//**
*
* Returns true if the RX data width is a byte (8 bits).
*
* \param base
* The pointer to the SCB instance.
*
* \return
* True if the RX data width is a byte (8 bits).
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SCB_IsRxDataWidthByte(volatile stc_SCB_t const *base)
{
    return (base->unRX_CTRL.stcField.u5DATA_WIDTH < CY_SCB_BYTE_WIDTH);
}


/*******************************************************************************
* Function Name: Cy_SCB_IsTxDataWidthByte
****************************************************************************//**
*
* Returns true if the TX data width is a byte (8 bits).
*
* \param base
* The pointer to the SCB instance.
*
* \return
* If ture, the TX data width is a byte (8 bits). Otherwise, false.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SCB_IsTxDataWidthByte(volatile stc_SCB_t const *base)
{
    return (base->unTX_CTRL.stcField.u5DATA_WIDTH < CY_SCB_BYTE_WIDTH);
}


/*******************************************************************************
* Function Name: Cy_SCB_FwBlockReset
****************************************************************************//**
*
* Disables and enables the block to return it into the known state (default):
* FIFOs and interrupt statuses are cleared.
*
* \param base
* The pointer to the SCB instance.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SCB_FwBlockReset(volatile stc_SCB_t *base)
{
    base->unCTRL.stcField.u1ENABLED = 0ul;

    /* Clean-up command registers */
    base->unI2C_M_CMD.u32Register = 0ul;
    base->unI2C_S_CMD.u32Register = 0ul;

    base->unCTRL.stcField.u1ENABLED = 1ul;

    (void) base->unCTRL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SCB_GetRxFifoLevel
****************************************************************************//**
*
* Returns the RX FIFO level when there are more words in the RX FIFO than the
* level, the RX FIFO level interrupt is triggered.
*
* \param base
* The pointer to the SCB instance.
*
* \return
* RX FIFO level.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SCB_GetRxFifoLevel(volatile stc_SCB_t const *base)
{
    return (base->unRX_FIFO_CTRL.stcField.u8TRIGGER_LEVEL);
}
/** \} group_scb_common_functions */

#if defined(__cplusplus)
}
#endif

/** \} group_scb_common */

#endif /* (CY_SCB_COMMON_H) */


/* [] END OF FILE */

