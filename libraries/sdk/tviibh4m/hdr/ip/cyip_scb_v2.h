/***************************************************************************//**
* \file cyip_scb_v2.h
*
* \brief
* SCB IP definitions
*
* \note
* Generator version: 1.6.0.481
* Database revision: TVIIBH4M_PR3_0
*
********************************************************************************
* \copyright
* Copyright 2016-2021, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYIP_SCB_V2_H_
#define _CYIP_SCB_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     SCB
*******************************************************************************/

#define SCB_SECTION_SIZE                        0x00010000UL

/**
  * \brief Generic control (SCB_CTRL)
  */
typedef struct stc_SCB_CTRL_field {
  uint32_t                         u4OVS:4;
  uint32_t                         :4;
  uint32_t                         u1EC_AM_MODE:1;
  uint32_t                         u1EC_OP_MODE:1;
  uint32_t                         u1EZ_MODE:1;
  uint32_t                         :1;
  uint32_t                         u1CMD_RESP_MODE:1;
  uint32_t                         :1;
  uint32_t                         u2MEM_WIDTH:2;
  uint32_t                         u1ADDR_ACCEPT:1;
  uint32_t                         u1BLOCK:1;
  uint32_t                         :6;
  uint32_t                         u2MODE:2;
  uint32_t                         :2;
  uint32_t                         u1EC_ACCESS:1;
  uint32_t                         :2;
  uint32_t                         u1ENABLED:1;
} stc_SCB_CTRL_field_t;

typedef union un_SCB_CTRL {
  uint32_t                         u32Register;
  stc_SCB_CTRL_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_CTRL_t;

/**
  * \brief Generic status (SCB_STATUS)
  */
typedef struct stc_SCB_STATUS_field {
  uint32_t                         u1EC_BUSY:1;
  uint32_t                         :31;
} stc_SCB_STATUS_field_t;

typedef union un_SCB_STATUS {
  uint32_t                         u32Register;
  stc_SCB_STATUS_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_STATUS_t;

/**
  * \brief Command/response control (SCB_CMD_RESP_CTRL)
  */
typedef struct stc_SCB_CMD_RESP_CTRL_field {
  uint32_t                         u9BASE_RD_ADDR:9;
  uint32_t                         :7;
  uint32_t                         u9BASE_WR_ADDR:9;
  uint32_t                         :7;
} stc_SCB_CMD_RESP_CTRL_field_t;

typedef union un_SCB_CMD_RESP_CTRL {
  uint32_t                         u32Register;
  stc_SCB_CMD_RESP_CTRL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_CMD_RESP_CTRL_t;

/**
  * \brief Command/response status (SCB_CMD_RESP_STATUS)
  */
typedef struct stc_SCB_CMD_RESP_STATUS_field {
  uint32_t                         u9CURR_RD_ADDR:9;
  uint32_t                         :7;
  uint32_t                         u9CURR_WR_ADDR:9;
  uint32_t                         :5;
  uint32_t                         u1CMD_RESP_EC_BUS_BUSY:1;
  uint32_t                         u1CMD_RESP_EC_BUSY:1;
} stc_SCB_CMD_RESP_STATUS_field_t;

typedef union un_SCB_CMD_RESP_STATUS {
  uint32_t                         u32Register;
  stc_SCB_CMD_RESP_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_CMD_RESP_STATUS_t;

/**
  * \brief SPI control (SCB_SPI_CTRL)
  */
typedef struct stc_SCB_SPI_CTRL_field {
  uint32_t                         u1SSEL_CONTINUOUS:1;
  uint32_t                         u1SELECT_PRECEDE:1;
  uint32_t                         u1CPHA:1;
  uint32_t                         u1CPOL:1;
  uint32_t                         u1LATE_MISO_SAMPLE:1;
  uint32_t                         u1SCLK_CONTINUOUS:1;
  uint32_t                         :2;
  uint32_t                         u1SSEL_POLARITY0:1;
  uint32_t                         u1SSEL_POLARITY1:1;
  uint32_t                         u1SSEL_POLARITY2:1;
  uint32_t                         u1SSEL_POLARITY3:1;
  uint32_t                         u1SSEL_SETUP_DEL:1;
  uint32_t                         u1SSEL_HOLD_DEL:1;
  uint32_t                         u1SSEL_INTER_FRAME_DEL:1;
  uint32_t                         :1;
  uint32_t                         u1LOOPBACK:1;
  uint32_t                         :7;
  uint32_t                         u2MODE:2;
  uint32_t                         u2SSEL:2;
  uint32_t                         :3;
  uint32_t                         u1MASTER_MODE:1;
} stc_SCB_SPI_CTRL_field_t;

typedef union un_SCB_SPI_CTRL {
  uint32_t                         u32Register;
  stc_SCB_SPI_CTRL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_SPI_CTRL_t;

/**
  * \brief SPI status (SCB_SPI_STATUS)
  */
typedef struct stc_SCB_SPI_STATUS_field {
  uint32_t                         u1BUS_BUSY:1;
  uint32_t                         u1SPI_EC_BUSY:1;
  uint32_t                         :6;
  uint32_t                         u8CURR_EZ_ADDR:8;
  uint32_t                         u8BASE_EZ_ADDR:8;
  uint32_t                         :8;
} stc_SCB_SPI_STATUS_field_t;

typedef union un_SCB_SPI_STATUS {
  uint32_t                         u32Register;
  stc_SCB_SPI_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_SPI_STATUS_t;

/**
  * \brief SPI transmitter control (SCB_SPI_TX_CTRL)
  */
typedef struct stc_SCB_SPI_TX_CTRL_field {
  uint32_t                         :4;
  uint32_t                         u1PARITY:1;
  uint32_t                         u1PARITY_ENABLED:1;
  uint32_t                         :26;
} stc_SCB_SPI_TX_CTRL_field_t;

typedef union un_SCB_SPI_TX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_SPI_TX_CTRL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_SPI_TX_CTRL_t;

/**
  * \brief SPI receiver control (SCB_SPI_RX_CTRL)
  */
typedef struct stc_SCB_SPI_RX_CTRL_field {
  uint32_t                         :4;
  uint32_t                         u1PARITY:1;
  uint32_t                         u1PARITY_ENABLED:1;
  uint32_t                         :2;
  uint32_t                         u1DROP_ON_PARITY_ERROR:1;
  uint32_t                         :23;
} stc_SCB_SPI_RX_CTRL_field_t;

typedef union un_SCB_SPI_RX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_SPI_RX_CTRL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_SPI_RX_CTRL_t;

/**
  * \brief UART control (SCB_UART_CTRL)
  */
typedef struct stc_SCB_UART_CTRL_field {
  uint32_t                         :16;
  uint32_t                         u1LOOPBACK:1;
  uint32_t                         :7;
  uint32_t                         u2MODE:2;
  uint32_t                         :6;
} stc_SCB_UART_CTRL_field_t;

typedef union un_SCB_UART_CTRL {
  uint32_t                         u32Register;
  stc_SCB_UART_CTRL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_UART_CTRL_t;

/**
  * \brief UART transmitter control (SCB_UART_TX_CTRL)
  */
typedef struct stc_SCB_UART_TX_CTRL_field {
  uint32_t                         u3STOP_BITS:3;
  uint32_t                         :1;
  uint32_t                         u1PARITY:1;
  uint32_t                         u1PARITY_ENABLED:1;
  uint32_t                         :2;
  uint32_t                         u1RETRY_ON_NACK:1;
  uint32_t                         :23;
} stc_SCB_UART_TX_CTRL_field_t;

typedef union un_SCB_UART_TX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_UART_TX_CTRL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_UART_TX_CTRL_t;

/**
  * \brief UART receiver control (SCB_UART_RX_CTRL)
  */
typedef struct stc_SCB_UART_RX_CTRL_field {
  uint32_t                         u3STOP_BITS:3;
  uint32_t                         :1;
  uint32_t                         u1PARITY:1;
  uint32_t                         u1PARITY_ENABLED:1;
  uint32_t                         u1POLARITY:1;
  uint32_t                         :1;
  uint32_t                         u1DROP_ON_PARITY_ERROR:1;
  uint32_t                         u1DROP_ON_FRAME_ERROR:1;
  uint32_t                         u1MP_MODE:1;
  uint32_t                         :1;
  uint32_t                         u1LIN_MODE:1;
  uint32_t                         u1SKIP_START:1;
  uint32_t                         :2;
  uint32_t                         u4BREAK_WIDTH:4;
  uint32_t                         :4;
  uint32_t                         u1BREAK_LEVEL:1;
  uint32_t                         :7;
} stc_SCB_UART_RX_CTRL_field_t;

typedef union un_SCB_UART_RX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_UART_RX_CTRL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_UART_RX_CTRL_t;

/**
  * \brief UART receiver status (SCB_UART_RX_STATUS)
  */
typedef struct stc_SCB_UART_RX_STATUS_field {
  uint32_t                         u12BR_COUNTER:12;
  uint32_t                         :20;
} stc_SCB_UART_RX_STATUS_field_t;

typedef union un_SCB_UART_RX_STATUS {
  uint32_t                         u32Register;
  stc_SCB_UART_RX_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_UART_RX_STATUS_t;

/**
  * \brief UART flow control (SCB_UART_FLOW_CTRL)
  */
typedef struct stc_SCB_UART_FLOW_CTRL_field {
  uint32_t                         u8TRIGGER_LEVEL:8;
  uint32_t                         :8;
  uint32_t                         u1RTS_POLARITY:1;
  uint32_t                         :7;
  uint32_t                         u1CTS_POLARITY:1;
  uint32_t                         u1CTS_ENABLED:1;
  uint32_t                         :6;
} stc_SCB_UART_FLOW_CTRL_field_t;

typedef union un_SCB_UART_FLOW_CTRL {
  uint32_t                         u32Register;
  stc_SCB_UART_FLOW_CTRL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_UART_FLOW_CTRL_t;

/**
  * \brief I2C control (SCB_I2C_CTRL)
  */
typedef struct stc_SCB_I2C_CTRL_field {
  uint32_t                         u4HIGH_PHASE_OVS:4;
  uint32_t                         u4LOW_PHASE_OVS:4;
  uint32_t                         u1M_READY_DATA_ACK:1;
  uint32_t                         u1M_NOT_READY_DATA_NACK:1;
  uint32_t                         :1;
  uint32_t                         u1S_GENERAL_IGNORE:1;
  uint32_t                         u1S_READY_ADDR_ACK:1;
  uint32_t                         u1S_READY_DATA_ACK:1;
  uint32_t                         u1S_NOT_READY_ADDR_NACK:1;
  uint32_t                         u1S_NOT_READY_DATA_NACK:1;
  uint32_t                         u1LOOPBACK:1;
  uint32_t                         :13;
  uint32_t                         u1SLAVE_MODE:1;
  uint32_t                         u1MASTER_MODE:1;
} stc_SCB_I2C_CTRL_field_t;

typedef union un_SCB_I2C_CTRL {
  uint32_t                         u32Register;
  stc_SCB_I2C_CTRL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_I2C_CTRL_t;

/**
  * \brief I2C status (SCB_I2C_STATUS)
  */
typedef struct stc_SCB_I2C_STATUS_field {
  uint32_t                         u1BUS_BUSY:1;
  uint32_t                         u1I2C_EC_BUSY:1;
  uint32_t                         u1I2CS_IC_BUSY:1;
  uint32_t                         :1;
  uint32_t                         u1S_READ:1;
  uint32_t                         u1M_READ:1;
  uint32_t                         :2;
  uint32_t                         u8CURR_EZ_ADDR:8;
  uint32_t                         u8BASE_EZ_ADDR:8;
  uint32_t                         :8;
} stc_SCB_I2C_STATUS_field_t;

typedef union un_SCB_I2C_STATUS {
  uint32_t                         u32Register;
  stc_SCB_I2C_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_I2C_STATUS_t;

/**
  * \brief I2C master command (SCB_I2C_M_CMD)
  */
typedef struct stc_SCB_I2C_M_CMD_field {
  uint32_t                         u1M_START:1;
  uint32_t                         u1M_START_ON_IDLE:1;
  uint32_t                         u1M_ACK:1;
  uint32_t                         u1M_NACK:1;
  uint32_t                         u1M_STOP:1;
  uint32_t                         :27;
} stc_SCB_I2C_M_CMD_field_t;

typedef union un_SCB_I2C_M_CMD {
  uint32_t                         u32Register;
  stc_SCB_I2C_M_CMD_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_I2C_M_CMD_t;

/**
  * \brief I2C slave command (SCB_I2C_S_CMD)
  */
typedef struct stc_SCB_I2C_S_CMD_field {
  uint32_t                         u1S_ACK:1;
  uint32_t                         u1S_NACK:1;
  uint32_t                         :30;
} stc_SCB_I2C_S_CMD_field_t;

typedef union un_SCB_I2C_S_CMD {
  uint32_t                         u32Register;
  stc_SCB_I2C_S_CMD_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_I2C_S_CMD_t;

/**
  * \brief I2C configuration (SCB_I2C_CFG)
  */
typedef struct stc_SCB_I2C_CFG_field {
  uint32_t                         u2SDA_IN_FILT_TRIM:2;
  uint32_t                         :2;
  uint32_t                         u1SDA_IN_FILT_SEL:1;
  uint32_t                         :3;
  uint32_t                         u2SCL_IN_FILT_TRIM:2;
  uint32_t                         :2;
  uint32_t                         u1SCL_IN_FILT_SEL:1;
  uint32_t                         :3;
  uint32_t                         u2SDA_OUT_FILT0_TRIM:2;
  uint32_t                         u2SDA_OUT_FILT1_TRIM:2;
  uint32_t                         u2SDA_OUT_FILT2_TRIM:2;
  uint32_t                         :6;
  uint32_t                         u2SDA_OUT_FILT_SEL:2;
  uint32_t                         :2;
} stc_SCB_I2C_CFG_field_t;

typedef union un_SCB_I2C_CFG {
  uint32_t                         u32Register;
  stc_SCB_I2C_CFG_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_I2C_CFG_t;

/**
  * \brief Transmitter control (SCB_TX_CTRL)
  */
typedef struct stc_SCB_TX_CTRL_field {
  uint32_t                         u5DATA_WIDTH:5;
  uint32_t                         :3;
  uint32_t                         u1MSB_FIRST:1;
  uint32_t                         :7;
  uint32_t                         u1OPEN_DRAIN:1;
  uint32_t                         :15;
} stc_SCB_TX_CTRL_field_t;

typedef union un_SCB_TX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_TX_CTRL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_TX_CTRL_t;

/**
  * \brief Transmitter FIFO control (SCB_TX_FIFO_CTRL)
  */
typedef struct stc_SCB_TX_FIFO_CTRL_field {
  uint32_t                         u8TRIGGER_LEVEL:8;
  uint32_t                         :8;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_SCB_TX_FIFO_CTRL_field_t;

typedef union un_SCB_TX_FIFO_CTRL {
  uint32_t                         u32Register;
  stc_SCB_TX_FIFO_CTRL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_TX_FIFO_CTRL_t;

/**
  * \brief Transmitter FIFO status (SCB_TX_FIFO_STATUS)
  */
typedef struct stc_SCB_TX_FIFO_STATUS_field {
  uint32_t                         u9USED:9;
  uint32_t                         :6;
  uint32_t                         u1SR_VALID:1;
  uint32_t                         u8RD_PTR:8;
  uint32_t                         u8WR_PTR:8;
} stc_SCB_TX_FIFO_STATUS_field_t;

typedef union un_SCB_TX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SCB_TX_FIFO_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_TX_FIFO_STATUS_t;

/**
  * \brief Transmitter FIFO write (SCB_TX_FIFO_WR)
  */
typedef struct stc_SCB_TX_FIFO_WR_field {
  uint32_t                         u32DATA:32;
} stc_SCB_TX_FIFO_WR_field_t;

typedef union un_SCB_TX_FIFO_WR {
  uint32_t                         u32Register;
  stc_SCB_TX_FIFO_WR_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_TX_FIFO_WR_t;

/**
  * \brief Receiver control (SCB_RX_CTRL)
  */
typedef struct stc_SCB_RX_CTRL_field {
  uint32_t                         u5DATA_WIDTH:5;
  uint32_t                         :3;
  uint32_t                         u1MSB_FIRST:1;
  uint32_t                         u1MEDIAN:1;
  uint32_t                         :22;
} stc_SCB_RX_CTRL_field_t;

typedef union un_SCB_RX_CTRL {
  uint32_t                         u32Register;
  stc_SCB_RX_CTRL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_CTRL_t;

/**
  * \brief Receiver FIFO control (SCB_RX_FIFO_CTRL)
  */
typedef struct stc_SCB_RX_FIFO_CTRL_field {
  uint32_t                         u8TRIGGER_LEVEL:8;
  uint32_t                         :8;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_SCB_RX_FIFO_CTRL_field_t;

typedef union un_SCB_RX_FIFO_CTRL {
  uint32_t                         u32Register;
  stc_SCB_RX_FIFO_CTRL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_FIFO_CTRL_t;

/**
  * \brief Receiver FIFO status (SCB_RX_FIFO_STATUS)
  */
typedef struct stc_SCB_RX_FIFO_STATUS_field {
  uint32_t                         u9USED:9;
  uint32_t                         :6;
  uint32_t                         u1SR_VALID:1;
  uint32_t                         u8RD_PTR:8;
  uint32_t                         u8WR_PTR:8;
} stc_SCB_RX_FIFO_STATUS_field_t;

typedef union un_SCB_RX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SCB_RX_FIFO_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_FIFO_STATUS_t;

/**
  * \brief Slave address and mask (SCB_RX_MATCH)
  */
typedef struct stc_SCB_RX_MATCH_field {
  uint32_t                         u8ADDR:8;
  uint32_t                         :8;
  uint32_t                         u8MASK:8;
  uint32_t                         :8;
} stc_SCB_RX_MATCH_field_t;

typedef union un_SCB_RX_MATCH {
  uint32_t                         u32Register;
  stc_SCB_RX_MATCH_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_MATCH_t;

/**
  * \brief Receiver FIFO read (SCB_RX_FIFO_RD)
  */
typedef struct stc_SCB_RX_FIFO_RD_field {
  uint32_t                         u32DATA:32;
} stc_SCB_RX_FIFO_RD_field_t;

typedef union un_SCB_RX_FIFO_RD {
  uint32_t                         u32Register;
  stc_SCB_RX_FIFO_RD_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_FIFO_RD_t;

/**
  * \brief Receiver FIFO read silent (SCB_RX_FIFO_RD_SILENT)
  */
typedef struct stc_SCB_RX_FIFO_RD_SILENT_field {
  uint32_t                         u32DATA:32;
} stc_SCB_RX_FIFO_RD_SILENT_field_t;

typedef union un_SCB_RX_FIFO_RD_SILENT {
  uint32_t                         u32Register;
  stc_SCB_RX_FIFO_RD_SILENT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_RX_FIFO_RD_SILENT_t;

/**
  * \brief Memory buffer (SCB_EZ_DATA)
  */
typedef struct stc_SCB_EZ_DATA_field {
  uint32_t                         u8EZ_DATA:8;
  uint32_t                         :24;
} stc_SCB_EZ_DATA_field_t;

typedef union un_SCB_EZ_DATA {
  uint32_t                         u32Register;
  stc_SCB_EZ_DATA_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_EZ_DATA_t;

/**
  * \brief Active clocked interrupt signal (SCB_INTR_CAUSE)
  */
typedef struct stc_SCB_INTR_CAUSE_field {
  uint32_t                         u1M:1;
  uint32_t                         u1S:1;
  uint32_t                         u1TX:1;
  uint32_t                         u1RX:1;
  uint32_t                         u1I2C_EC:1;
  uint32_t                         u1SPI_EC:1;
  uint32_t                         :26;
} stc_SCB_INTR_CAUSE_field_t;

typedef union un_SCB_INTR_CAUSE {
  uint32_t                         u32Register;
  stc_SCB_INTR_CAUSE_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_CAUSE_t;

/**
  * \brief Externally clocked I2C interrupt request (SCB_INTR_I2C_EC)
  */
typedef struct stc_SCB_INTR_I2C_EC_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_I2C_EC_field_t;

typedef union un_SCB_INTR_I2C_EC {
  uint32_t                         u32Register;
  stc_SCB_INTR_I2C_EC_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_I2C_EC_t;

/**
  * \brief Externally clocked I2C interrupt mask (SCB_INTR_I2C_EC_MASK)
  */
typedef struct stc_SCB_INTR_I2C_EC_MASK_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_I2C_EC_MASK_field_t;

typedef union un_SCB_INTR_I2C_EC_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_I2C_EC_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_I2C_EC_MASK_t;

/**
  * \brief Externally clocked I2C interrupt masked (SCB_INTR_I2C_EC_MASKED)
  */
typedef struct stc_SCB_INTR_I2C_EC_MASKED_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_I2C_EC_MASKED_field_t;

typedef union un_SCB_INTR_I2C_EC_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_I2C_EC_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_I2C_EC_MASKED_t;

/**
  * \brief Externally clocked SPI interrupt request (SCB_INTR_SPI_EC)
  */
typedef struct stc_SCB_INTR_SPI_EC_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_SPI_EC_field_t;

typedef union un_SCB_INTR_SPI_EC {
  uint32_t                         u32Register;
  stc_SCB_INTR_SPI_EC_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_SPI_EC_t;

/**
  * \brief Externally clocked SPI interrupt mask (SCB_INTR_SPI_EC_MASK)
  */
typedef struct stc_SCB_INTR_SPI_EC_MASK_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_SPI_EC_MASK_field_t;

typedef union un_SCB_INTR_SPI_EC_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_SPI_EC_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_SPI_EC_MASK_t;

/**
  * \brief Externally clocked SPI interrupt masked (SCB_INTR_SPI_EC_MASKED)
  */
typedef struct stc_SCB_INTR_SPI_EC_MASKED_field {
  uint32_t                         u1WAKE_UP:1;
  uint32_t                         u1EZ_STOP:1;
  uint32_t                         u1EZ_WRITE_STOP:1;
  uint32_t                         u1EZ_READ_STOP:1;
  uint32_t                         :28;
} stc_SCB_INTR_SPI_EC_MASKED_field_t;

typedef union un_SCB_INTR_SPI_EC_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_SPI_EC_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_SPI_EC_MASKED_t;

/**
  * \brief Master interrupt request (SCB_INTR_M)
  */
typedef struct stc_SCB_INTR_M_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         :1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         :3;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_DONE:1;
  uint32_t                         :22;
} stc_SCB_INTR_M_field_t;

typedef union un_SCB_INTR_M {
  uint32_t                         u32Register;
  stc_SCB_INTR_M_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_M_t;

/**
  * \brief Master interrupt set request (SCB_INTR_M_SET)
  */
typedef struct stc_SCB_INTR_M_SET_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         :1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         :3;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_DONE:1;
  uint32_t                         :22;
} stc_SCB_INTR_M_SET_field_t;

typedef union un_SCB_INTR_M_SET {
  uint32_t                         u32Register;
  stc_SCB_INTR_M_SET_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_M_SET_t;

/**
  * \brief Master interrupt mask (SCB_INTR_M_MASK)
  */
typedef struct stc_SCB_INTR_M_MASK_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         :1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         :3;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_DONE:1;
  uint32_t                         :22;
} stc_SCB_INTR_M_MASK_field_t;

typedef union un_SCB_INTR_M_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_M_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_M_MASK_t;

/**
  * \brief Master interrupt masked request (SCB_INTR_M_MASKED)
  */
typedef struct stc_SCB_INTR_M_MASKED_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         :1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         :3;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_DONE:1;
  uint32_t                         :22;
} stc_SCB_INTR_M_MASKED_field_t;

typedef union un_SCB_INTR_M_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_M_MASKED_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_M_MASKED_t;

/**
  * \brief Slave interrupt request (SCB_INTR_S)
  */
typedef struct stc_SCB_INTR_S_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         u1I2C_WRITE_STOP:1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         u1I2C_START:1;
  uint32_t                         u1I2C_ADDR_MATCH:1;
  uint32_t                         u1I2C_GENERAL:1;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_EZ_WRITE_STOP:1;
  uint32_t                         u1SPI_EZ_STOP:1;
  uint32_t                         u1SPI_BUS_ERROR:1;
  uint32_t                         :20;
} stc_SCB_INTR_S_field_t;

typedef union un_SCB_INTR_S {
  uint32_t                         u32Register;
  stc_SCB_INTR_S_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_S_t;

/**
  * \brief Slave interrupt set request (SCB_INTR_S_SET)
  */
typedef struct stc_SCB_INTR_S_SET_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         u1I2C_WRITE_STOP:1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         u1I2C_START:1;
  uint32_t                         u1I2C_ADDR_MATCH:1;
  uint32_t                         u1I2C_GENERAL:1;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_EZ_WRITE_STOP:1;
  uint32_t                         u1SPI_EZ_STOP:1;
  uint32_t                         u1SPI_BUS_ERROR:1;
  uint32_t                         :20;
} stc_SCB_INTR_S_SET_field_t;

typedef union un_SCB_INTR_S_SET {
  uint32_t                         u32Register;
  stc_SCB_INTR_S_SET_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_S_SET_t;

/**
  * \brief Slave interrupt mask (SCB_INTR_S_MASK)
  */
typedef struct stc_SCB_INTR_S_MASK_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         u1I2C_WRITE_STOP:1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         u1I2C_START:1;
  uint32_t                         u1I2C_ADDR_MATCH:1;
  uint32_t                         u1I2C_GENERAL:1;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_EZ_WRITE_STOP:1;
  uint32_t                         u1SPI_EZ_STOP:1;
  uint32_t                         u1SPI_BUS_ERROR:1;
  uint32_t                         :20;
} stc_SCB_INTR_S_MASK_field_t;

typedef union un_SCB_INTR_S_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_S_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_S_MASK_t;

/**
  * \brief Slave interrupt masked request (SCB_INTR_S_MASKED)
  */
typedef struct stc_SCB_INTR_S_MASKED_field {
  uint32_t                         u1I2C_ARB_LOST:1;
  uint32_t                         u1I2C_NACK:1;
  uint32_t                         u1I2C_ACK:1;
  uint32_t                         u1I2C_WRITE_STOP:1;
  uint32_t                         u1I2C_STOP:1;
  uint32_t                         u1I2C_START:1;
  uint32_t                         u1I2C_ADDR_MATCH:1;
  uint32_t                         u1I2C_GENERAL:1;
  uint32_t                         u1I2C_BUS_ERROR:1;
  uint32_t                         u1SPI_EZ_WRITE_STOP:1;
  uint32_t                         u1SPI_EZ_STOP:1;
  uint32_t                         u1SPI_BUS_ERROR:1;
  uint32_t                         :20;
} stc_SCB_INTR_S_MASKED_field_t;

typedef union un_SCB_INTR_S_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_S_MASKED_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_S_MASKED_t;

/**
  * \brief Transmitter interrupt request (SCB_INTR_TX)
  */
typedef struct stc_SCB_INTR_TX_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         u1NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1EMPTY:1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1UART_NACK:1;
  uint32_t                         u1UART_DONE:1;
  uint32_t                         u1UART_ARB_LOST:1;
  uint32_t                         :21;
} stc_SCB_INTR_TX_field_t;

typedef union un_SCB_INTR_TX {
  uint32_t                         u32Register;
  stc_SCB_INTR_TX_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_TX_t;

/**
  * \brief Transmitter interrupt set request (SCB_INTR_TX_SET)
  */
typedef struct stc_SCB_INTR_TX_SET_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         u1NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1EMPTY:1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1UART_NACK:1;
  uint32_t                         u1UART_DONE:1;
  uint32_t                         u1UART_ARB_LOST:1;
  uint32_t                         :21;
} stc_SCB_INTR_TX_SET_field_t;

typedef union un_SCB_INTR_TX_SET {
  uint32_t                         u32Register;
  stc_SCB_INTR_TX_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_TX_SET_t;

/**
  * \brief Transmitter interrupt mask (SCB_INTR_TX_MASK)
  */
typedef struct stc_SCB_INTR_TX_MASK_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         u1NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1EMPTY:1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1UART_NACK:1;
  uint32_t                         u1UART_DONE:1;
  uint32_t                         u1UART_ARB_LOST:1;
  uint32_t                         :21;
} stc_SCB_INTR_TX_MASK_field_t;

typedef union un_SCB_INTR_TX_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_TX_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_TX_MASK_t;

/**
  * \brief Transmitter interrupt masked request (SCB_INTR_TX_MASKED)
  */
typedef struct stc_SCB_INTR_TX_MASKED_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         u1NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1EMPTY:1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1UART_NACK:1;
  uint32_t                         u1UART_DONE:1;
  uint32_t                         u1UART_ARB_LOST:1;
  uint32_t                         :21;
} stc_SCB_INTR_TX_MASKED_field_t;

typedef union un_SCB_INTR_TX_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_TX_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_TX_MASKED_t;

/**
  * \brief Receiver interrupt request (SCB_INTR_RX)
  */
typedef struct stc_SCB_INTR_RX_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1NOT_EMPTY:1;
  uint32_t                         u1FULL:1;
  uint32_t                         :1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1FRAME_ERROR:1;
  uint32_t                         u1PARITY_ERROR:1;
  uint32_t                         u1BAUD_DETECT:1;
  uint32_t                         u1BREAK_DETECT:1;
  uint32_t                         :20;
} stc_SCB_INTR_RX_field_t;

typedef union un_SCB_INTR_RX {
  uint32_t                         u32Register;
  stc_SCB_INTR_RX_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_RX_t;

/**
  * \brief Receiver interrupt set request (SCB_INTR_RX_SET)
  */
typedef struct stc_SCB_INTR_RX_SET_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1NOT_EMPTY:1;
  uint32_t                         u1FULL:1;
  uint32_t                         :1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1FRAME_ERROR:1;
  uint32_t                         u1PARITY_ERROR:1;
  uint32_t                         u1BAUD_DETECT:1;
  uint32_t                         u1BREAK_DETECT:1;
  uint32_t                         :20;
} stc_SCB_INTR_RX_SET_field_t;

typedef union un_SCB_INTR_RX_SET {
  uint32_t                         u32Register;
  stc_SCB_INTR_RX_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_RX_SET_t;

/**
  * \brief Receiver interrupt mask (SCB_INTR_RX_MASK)
  */
typedef struct stc_SCB_INTR_RX_MASK_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1NOT_EMPTY:1;
  uint32_t                         u1FULL:1;
  uint32_t                         :1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1FRAME_ERROR:1;
  uint32_t                         u1PARITY_ERROR:1;
  uint32_t                         u1BAUD_DETECT:1;
  uint32_t                         u1BREAK_DETECT:1;
  uint32_t                         :20;
} stc_SCB_INTR_RX_MASK_field_t;

typedef union un_SCB_INTR_RX_MASK {
  uint32_t                         u32Register;
  stc_SCB_INTR_RX_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_RX_MASK_t;

/**
  * \brief Receiver interrupt masked request (SCB_INTR_RX_MASKED)
  */
typedef struct stc_SCB_INTR_RX_MASKED_field {
  uint32_t                         u1TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1NOT_EMPTY:1;
  uint32_t                         u1FULL:1;
  uint32_t                         :1;
  uint32_t                         u1OVERFLOW:1;
  uint32_t                         u1UNDERFLOW:1;
  uint32_t                         u1BLOCKED:1;
  uint32_t                         u1FRAME_ERROR:1;
  uint32_t                         u1PARITY_ERROR:1;
  uint32_t                         u1BAUD_DETECT:1;
  uint32_t                         u1BREAK_DETECT:1;
  uint32_t                         :20;
} stc_SCB_INTR_RX_MASKED_field_t;

typedef union un_SCB_INTR_RX_MASKED {
  uint32_t                         u32Register;
  stc_SCB_INTR_RX_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SCB_INTR_RX_MASKED_t;



/**
  * \brief Serial Communications Block (SPI/UART/I2C) (SCB)
  */
typedef struct stc_SCB {
  un_SCB_CTRL_t                    unCTRL;             /*!< 0x00000000 Generic control */
  un_SCB_STATUS_t                  unSTATUS;           /*!< 0x00000004 Generic status */
  un_SCB_CMD_RESP_CTRL_t           unCMD_RESP_CTRL;    /*!< 0x00000008 Command/response control */
  un_SCB_CMD_RESP_STATUS_t         unCMD_RESP_STATUS;  /*!< 0x0000000C Command/response status */
  uint32_t                         au32Reserved[4];
  un_SCB_SPI_CTRL_t                unSPI_CTRL;         /*!< 0x00000020 SPI control */
  un_SCB_SPI_STATUS_t              unSPI_STATUS;       /*!< 0x00000024 SPI status */
  un_SCB_SPI_TX_CTRL_t             unSPI_TX_CTRL;      /*!< 0x00000028 SPI transmitter control */
  un_SCB_SPI_RX_CTRL_t             unSPI_RX_CTRL;      /*!< 0x0000002C SPI receiver control */
  uint32_t                         au32Reserved1[4];
  un_SCB_UART_CTRL_t               unUART_CTRL;        /*!< 0x00000040 UART control */
  un_SCB_UART_TX_CTRL_t            unUART_TX_CTRL;     /*!< 0x00000044 UART transmitter control */
  un_SCB_UART_RX_CTRL_t            unUART_RX_CTRL;     /*!< 0x00000048 UART receiver control */
  un_SCB_UART_RX_STATUS_t          unUART_RX_STATUS;   /*!< 0x0000004C UART receiver status */
  un_SCB_UART_FLOW_CTRL_t          unUART_FLOW_CTRL;   /*!< 0x00000050 UART flow control */
  uint32_t                         au32Reserved2[3];
  un_SCB_I2C_CTRL_t                unI2C_CTRL;         /*!< 0x00000060 I2C control */
  un_SCB_I2C_STATUS_t              unI2C_STATUS;       /*!< 0x00000064 I2C status */
  un_SCB_I2C_M_CMD_t               unI2C_M_CMD;        /*!< 0x00000068 I2C master command */
  un_SCB_I2C_S_CMD_t               unI2C_S_CMD;        /*!< 0x0000006C I2C slave command */
  un_SCB_I2C_CFG_t                 unI2C_CFG;          /*!< 0x00000070 I2C configuration */
  uint32_t                         au32Reserved3[99];
  un_SCB_TX_CTRL_t                 unTX_CTRL;          /*!< 0x00000200 Transmitter control */
  un_SCB_TX_FIFO_CTRL_t            unTX_FIFO_CTRL;     /*!< 0x00000204 Transmitter FIFO control */
  un_SCB_TX_FIFO_STATUS_t          unTX_FIFO_STATUS;   /*!< 0x00000208 Transmitter FIFO status */
  uint32_t                         au32Reserved4[13];
  un_SCB_TX_FIFO_WR_t              unTX_FIFO_WR;       /*!< 0x00000240 Transmitter FIFO write */
  uint32_t                         au32Reserved5[47];
  un_SCB_RX_CTRL_t                 unRX_CTRL;          /*!< 0x00000300 Receiver control */
  un_SCB_RX_FIFO_CTRL_t            unRX_FIFO_CTRL;     /*!< 0x00000304 Receiver FIFO control */
  un_SCB_RX_FIFO_STATUS_t          unRX_FIFO_STATUS;   /*!< 0x00000308 Receiver FIFO status */
  uint32_t                         u32Reserved6;
  un_SCB_RX_MATCH_t                unRX_MATCH;         /*!< 0x00000310 Slave address and mask */
  uint32_t                         au32Reserved7[11];
  un_SCB_RX_FIFO_RD_t              unRX_FIFO_RD;       /*!< 0x00000340 Receiver FIFO read */
  un_SCB_RX_FIFO_RD_SILENT_t       unRX_FIFO_RD_SILENT; /*!< 0x00000344 Receiver FIFO read silent */
  uint32_t                         au32Reserved8[46];
  un_SCB_EZ_DATA_t                 unEZ_DATA[512];     /*!< 0x00000400 Memory buffer */
  uint32_t                         au32Reserved9[128];
  un_SCB_INTR_CAUSE_t              unINTR_CAUSE;       /*!< 0x00000E00 Active clocked interrupt signal */
  uint32_t                         au32Reserved10[31];
  un_SCB_INTR_I2C_EC_t             unINTR_I2C_EC;      /*!< 0x00000E80 Externally clocked I2C interrupt request */
  uint32_t                         u32Reserved11;
  un_SCB_INTR_I2C_EC_MASK_t        unINTR_I2C_EC_MASK; /*!< 0x00000E88 Externally clocked I2C interrupt mask */
  un_SCB_INTR_I2C_EC_MASKED_t      unINTR_I2C_EC_MASKED; /*!< 0x00000E8C Externally clocked I2C interrupt masked */
  uint32_t                         au32Reserved12[12];
  un_SCB_INTR_SPI_EC_t             unINTR_SPI_EC;      /*!< 0x00000EC0 Externally clocked SPI interrupt request */
  uint32_t                         u32Reserved13;
  un_SCB_INTR_SPI_EC_MASK_t        unINTR_SPI_EC_MASK; /*!< 0x00000EC8 Externally clocked SPI interrupt mask */
  un_SCB_INTR_SPI_EC_MASKED_t      unINTR_SPI_EC_MASKED; /*!< 0x00000ECC Externally clocked SPI interrupt masked */
  uint32_t                         au32Reserved14[12];
  un_SCB_INTR_M_t                  unINTR_M;           /*!< 0x00000F00 Master interrupt request */
  un_SCB_INTR_M_SET_t              unINTR_M_SET;       /*!< 0x00000F04 Master interrupt set request */
  un_SCB_INTR_M_MASK_t             unINTR_M_MASK;      /*!< 0x00000F08 Master interrupt mask */
  un_SCB_INTR_M_MASKED_t           unINTR_M_MASKED;    /*!< 0x00000F0C Master interrupt masked request */
  uint32_t                         au32Reserved15[12];
  un_SCB_INTR_S_t                  unINTR_S;           /*!< 0x00000F40 Slave interrupt request */
  un_SCB_INTR_S_SET_t              unINTR_S_SET;       /*!< 0x00000F44 Slave interrupt set request */
  un_SCB_INTR_S_MASK_t             unINTR_S_MASK;      /*!< 0x00000F48 Slave interrupt mask */
  un_SCB_INTR_S_MASKED_t           unINTR_S_MASKED;    /*!< 0x00000F4C Slave interrupt masked request */
  uint32_t                         au32Reserved16[12];
  un_SCB_INTR_TX_t                 unINTR_TX;          /*!< 0x00000F80 Transmitter interrupt request */
  un_SCB_INTR_TX_SET_t             unINTR_TX_SET;      /*!< 0x00000F84 Transmitter interrupt set request */
  un_SCB_INTR_TX_MASK_t            unINTR_TX_MASK;     /*!< 0x00000F88 Transmitter interrupt mask */
  un_SCB_INTR_TX_MASKED_t          unINTR_TX_MASKED;   /*!< 0x00000F8C Transmitter interrupt masked request */
  uint32_t                         au32Reserved17[12];
  un_SCB_INTR_RX_t                 unINTR_RX;          /*!< 0x00000FC0 Receiver interrupt request */
  un_SCB_INTR_RX_SET_t             unINTR_RX_SET;      /*!< 0x00000FC4 Receiver interrupt set request */
  un_SCB_INTR_RX_MASK_t            unINTR_RX_MASK;     /*!< 0x00000FC8 Receiver interrupt mask */
  un_SCB_INTR_RX_MASKED_t          unINTR_RX_MASKED;   /*!< 0x00000FCC Receiver interrupt masked request */
} stc_SCB_t;                                           /*!< Size = 4048 (0xFD0) */


/* SCB.CTRL */
#define SCB_CTRL_OVS_Pos                        0UL
#define SCB_CTRL_OVS_Msk                        0xFUL
#define SCB_CTRL_EC_AM_MODE_Pos                 8UL
#define SCB_CTRL_EC_AM_MODE_Msk                 0x100UL
#define SCB_CTRL_EC_OP_MODE_Pos                 9UL
#define SCB_CTRL_EC_OP_MODE_Msk                 0x200UL
#define SCB_CTRL_EZ_MODE_Pos                    10UL
#define SCB_CTRL_EZ_MODE_Msk                    0x400UL
#define SCB_CTRL_CMD_RESP_MODE_Pos              12UL
#define SCB_CTRL_CMD_RESP_MODE_Msk              0x1000UL
#define SCB_CTRL_MEM_WIDTH_Pos                  14UL
#define SCB_CTRL_MEM_WIDTH_Msk                  0xC000UL
#define SCB_CTRL_ADDR_ACCEPT_Pos                16UL
#define SCB_CTRL_ADDR_ACCEPT_Msk                0x10000UL
#define SCB_CTRL_BLOCK_Pos                      17UL
#define SCB_CTRL_BLOCK_Msk                      0x20000UL
#define SCB_CTRL_MODE_Pos                       24UL
#define SCB_CTRL_MODE_Msk                       0x3000000UL
#define SCB_CTRL_EC_ACCESS_Pos                  28UL
#define SCB_CTRL_EC_ACCESS_Msk                  0x10000000UL
#define SCB_CTRL_ENABLED_Pos                    31UL
#define SCB_CTRL_ENABLED_Msk                    0x80000000UL
/* SCB.STATUS */
#define SCB_STATUS_EC_BUSY_Pos                  0UL
#define SCB_STATUS_EC_BUSY_Msk                  0x1UL
/* SCB.CMD_RESP_CTRL */
#define SCB_CMD_RESP_CTRL_BASE_RD_ADDR_Pos      0UL
#define SCB_CMD_RESP_CTRL_BASE_RD_ADDR_Msk      0x1FFUL
#define SCB_CMD_RESP_CTRL_BASE_WR_ADDR_Pos      16UL
#define SCB_CMD_RESP_CTRL_BASE_WR_ADDR_Msk      0x1FF0000UL
/* SCB.CMD_RESP_STATUS */
#define SCB_CMD_RESP_STATUS_CURR_RD_ADDR_Pos    0UL
#define SCB_CMD_RESP_STATUS_CURR_RD_ADDR_Msk    0x1FFUL
#define SCB_CMD_RESP_STATUS_CURR_WR_ADDR_Pos    16UL
#define SCB_CMD_RESP_STATUS_CURR_WR_ADDR_Msk    0x1FF0000UL
#define SCB_CMD_RESP_STATUS_CMD_RESP_EC_BUS_BUSY_Pos 30UL
#define SCB_CMD_RESP_STATUS_CMD_RESP_EC_BUS_BUSY_Msk 0x40000000UL
#define SCB_CMD_RESP_STATUS_CMD_RESP_EC_BUSY_Pos 31UL
#define SCB_CMD_RESP_STATUS_CMD_RESP_EC_BUSY_Msk 0x80000000UL
/* SCB.SPI_CTRL */
#define SCB_SPI_CTRL_SSEL_CONTINUOUS_Pos        0UL
#define SCB_SPI_CTRL_SSEL_CONTINUOUS_Msk        0x1UL
#define SCB_SPI_CTRL_SELECT_PRECEDE_Pos         1UL
#define SCB_SPI_CTRL_SELECT_PRECEDE_Msk         0x2UL
#define SCB_SPI_CTRL_CPHA_Pos                   2UL
#define SCB_SPI_CTRL_CPHA_Msk                   0x4UL
#define SCB_SPI_CTRL_CPOL_Pos                   3UL
#define SCB_SPI_CTRL_CPOL_Msk                   0x8UL
#define SCB_SPI_CTRL_LATE_MISO_SAMPLE_Pos       4UL
#define SCB_SPI_CTRL_LATE_MISO_SAMPLE_Msk       0x10UL
#define SCB_SPI_CTRL_SCLK_CONTINUOUS_Pos        5UL
#define SCB_SPI_CTRL_SCLK_CONTINUOUS_Msk        0x20UL
#define SCB_SPI_CTRL_SSEL_POLARITY0_Pos         8UL
#define SCB_SPI_CTRL_SSEL_POLARITY0_Msk         0x100UL
#define SCB_SPI_CTRL_SSEL_POLARITY1_Pos         9UL
#define SCB_SPI_CTRL_SSEL_POLARITY1_Msk         0x200UL
#define SCB_SPI_CTRL_SSEL_POLARITY2_Pos         10UL
#define SCB_SPI_CTRL_SSEL_POLARITY2_Msk         0x400UL
#define SCB_SPI_CTRL_SSEL_POLARITY3_Pos         11UL
#define SCB_SPI_CTRL_SSEL_POLARITY3_Msk         0x800UL
#define SCB_SPI_CTRL_SSEL_SETUP_DEL_Pos         12UL
#define SCB_SPI_CTRL_SSEL_SETUP_DEL_Msk         0x1000UL
#define SCB_SPI_CTRL_SSEL_HOLD_DEL_Pos          13UL
#define SCB_SPI_CTRL_SSEL_HOLD_DEL_Msk          0x2000UL
#define SCB_SPI_CTRL_SSEL_INTER_FRAME_DEL_Pos   14UL
#define SCB_SPI_CTRL_SSEL_INTER_FRAME_DEL_Msk   0x4000UL
#define SCB_SPI_CTRL_LOOPBACK_Pos               16UL
#define SCB_SPI_CTRL_LOOPBACK_Msk               0x10000UL
#define SCB_SPI_CTRL_MODE_Pos                   24UL
#define SCB_SPI_CTRL_MODE_Msk                   0x3000000UL
#define SCB_SPI_CTRL_SSEL_Pos                   26UL
#define SCB_SPI_CTRL_SSEL_Msk                   0xC000000UL
#define SCB_SPI_CTRL_MASTER_MODE_Pos            31UL
#define SCB_SPI_CTRL_MASTER_MODE_Msk            0x80000000UL
/* SCB.SPI_STATUS */
#define SCB_SPI_STATUS_BUS_BUSY_Pos             0UL
#define SCB_SPI_STATUS_BUS_BUSY_Msk             0x1UL
#define SCB_SPI_STATUS_SPI_EC_BUSY_Pos          1UL
#define SCB_SPI_STATUS_SPI_EC_BUSY_Msk          0x2UL
#define SCB_SPI_STATUS_CURR_EZ_ADDR_Pos         8UL
#define SCB_SPI_STATUS_CURR_EZ_ADDR_Msk         0xFF00UL
#define SCB_SPI_STATUS_BASE_EZ_ADDR_Pos         16UL
#define SCB_SPI_STATUS_BASE_EZ_ADDR_Msk         0xFF0000UL
/* SCB.SPI_TX_CTRL */
#define SCB_SPI_TX_CTRL_PARITY_Pos              4UL
#define SCB_SPI_TX_CTRL_PARITY_Msk              0x10UL
#define SCB_SPI_TX_CTRL_PARITY_ENABLED_Pos      5UL
#define SCB_SPI_TX_CTRL_PARITY_ENABLED_Msk      0x20UL
/* SCB.SPI_RX_CTRL */
#define SCB_SPI_RX_CTRL_PARITY_Pos              4UL
#define SCB_SPI_RX_CTRL_PARITY_Msk              0x10UL
#define SCB_SPI_RX_CTRL_PARITY_ENABLED_Pos      5UL
#define SCB_SPI_RX_CTRL_PARITY_ENABLED_Msk      0x20UL
#define SCB_SPI_RX_CTRL_DROP_ON_PARITY_ERROR_Pos 8UL
#define SCB_SPI_RX_CTRL_DROP_ON_PARITY_ERROR_Msk 0x100UL
/* SCB.UART_CTRL */
#define SCB_UART_CTRL_LOOPBACK_Pos              16UL
#define SCB_UART_CTRL_LOOPBACK_Msk              0x10000UL
#define SCB_UART_CTRL_MODE_Pos                  24UL
#define SCB_UART_CTRL_MODE_Msk                  0x3000000UL
/* SCB.UART_TX_CTRL */
#define SCB_UART_TX_CTRL_STOP_BITS_Pos          0UL
#define SCB_UART_TX_CTRL_STOP_BITS_Msk          0x7UL
#define SCB_UART_TX_CTRL_PARITY_Pos             4UL
#define SCB_UART_TX_CTRL_PARITY_Msk             0x10UL
#define SCB_UART_TX_CTRL_PARITY_ENABLED_Pos     5UL
#define SCB_UART_TX_CTRL_PARITY_ENABLED_Msk     0x20UL
#define SCB_UART_TX_CTRL_RETRY_ON_NACK_Pos      8UL
#define SCB_UART_TX_CTRL_RETRY_ON_NACK_Msk      0x100UL
/* SCB.UART_RX_CTRL */
#define SCB_UART_RX_CTRL_STOP_BITS_Pos          0UL
#define SCB_UART_RX_CTRL_STOP_BITS_Msk          0x7UL
#define SCB_UART_RX_CTRL_PARITY_Pos             4UL
#define SCB_UART_RX_CTRL_PARITY_Msk             0x10UL
#define SCB_UART_RX_CTRL_PARITY_ENABLED_Pos     5UL
#define SCB_UART_RX_CTRL_PARITY_ENABLED_Msk     0x20UL
#define SCB_UART_RX_CTRL_POLARITY_Pos           6UL
#define SCB_UART_RX_CTRL_POLARITY_Msk           0x40UL
#define SCB_UART_RX_CTRL_DROP_ON_PARITY_ERROR_Pos 8UL
#define SCB_UART_RX_CTRL_DROP_ON_PARITY_ERROR_Msk 0x100UL
#define SCB_UART_RX_CTRL_DROP_ON_FRAME_ERROR_Pos 9UL
#define SCB_UART_RX_CTRL_DROP_ON_FRAME_ERROR_Msk 0x200UL
#define SCB_UART_RX_CTRL_MP_MODE_Pos            10UL
#define SCB_UART_RX_CTRL_MP_MODE_Msk            0x400UL
#define SCB_UART_RX_CTRL_LIN_MODE_Pos           12UL
#define SCB_UART_RX_CTRL_LIN_MODE_Msk           0x1000UL
#define SCB_UART_RX_CTRL_SKIP_START_Pos         13UL
#define SCB_UART_RX_CTRL_SKIP_START_Msk         0x2000UL
#define SCB_UART_RX_CTRL_BREAK_WIDTH_Pos        16UL
#define SCB_UART_RX_CTRL_BREAK_WIDTH_Msk        0xF0000UL
#define SCB_UART_RX_CTRL_BREAK_LEVEL_Pos        24UL
#define SCB_UART_RX_CTRL_BREAK_LEVEL_Msk        0x1000000UL
/* SCB.UART_RX_STATUS */
#define SCB_UART_RX_STATUS_BR_COUNTER_Pos       0UL
#define SCB_UART_RX_STATUS_BR_COUNTER_Msk       0xFFFUL
/* SCB.UART_FLOW_CTRL */
#define SCB_UART_FLOW_CTRL_TRIGGER_LEVEL_Pos    0UL
#define SCB_UART_FLOW_CTRL_TRIGGER_LEVEL_Msk    0xFFUL
#define SCB_UART_FLOW_CTRL_RTS_POLARITY_Pos     16UL
#define SCB_UART_FLOW_CTRL_RTS_POLARITY_Msk     0x10000UL
#define SCB_UART_FLOW_CTRL_CTS_POLARITY_Pos     24UL
#define SCB_UART_FLOW_CTRL_CTS_POLARITY_Msk     0x1000000UL
#define SCB_UART_FLOW_CTRL_CTS_ENABLED_Pos      25UL
#define SCB_UART_FLOW_CTRL_CTS_ENABLED_Msk      0x2000000UL
/* SCB.I2C_CTRL */
#define SCB_I2C_CTRL_HIGH_PHASE_OVS_Pos         0UL
#define SCB_I2C_CTRL_HIGH_PHASE_OVS_Msk         0xFUL
#define SCB_I2C_CTRL_LOW_PHASE_OVS_Pos          4UL
#define SCB_I2C_CTRL_LOW_PHASE_OVS_Msk          0xF0UL
#define SCB_I2C_CTRL_M_READY_DATA_ACK_Pos       8UL
#define SCB_I2C_CTRL_M_READY_DATA_ACK_Msk       0x100UL
#define SCB_I2C_CTRL_M_NOT_READY_DATA_NACK_Pos  9UL
#define SCB_I2C_CTRL_M_NOT_READY_DATA_NACK_Msk  0x200UL
#define SCB_I2C_CTRL_S_GENERAL_IGNORE_Pos       11UL
#define SCB_I2C_CTRL_S_GENERAL_IGNORE_Msk       0x800UL
#define SCB_I2C_CTRL_S_READY_ADDR_ACK_Pos       12UL
#define SCB_I2C_CTRL_S_READY_ADDR_ACK_Msk       0x1000UL
#define SCB_I2C_CTRL_S_READY_DATA_ACK_Pos       13UL
#define SCB_I2C_CTRL_S_READY_DATA_ACK_Msk       0x2000UL
#define SCB_I2C_CTRL_S_NOT_READY_ADDR_NACK_Pos  14UL
#define SCB_I2C_CTRL_S_NOT_READY_ADDR_NACK_Msk  0x4000UL
#define SCB_I2C_CTRL_S_NOT_READY_DATA_NACK_Pos  15UL
#define SCB_I2C_CTRL_S_NOT_READY_DATA_NACK_Msk  0x8000UL
#define SCB_I2C_CTRL_LOOPBACK_Pos               16UL
#define SCB_I2C_CTRL_LOOPBACK_Msk               0x10000UL
#define SCB_I2C_CTRL_SLAVE_MODE_Pos             30UL
#define SCB_I2C_CTRL_SLAVE_MODE_Msk             0x40000000UL
#define SCB_I2C_CTRL_MASTER_MODE_Pos            31UL
#define SCB_I2C_CTRL_MASTER_MODE_Msk            0x80000000UL
/* SCB.I2C_STATUS */
#define SCB_I2C_STATUS_BUS_BUSY_Pos             0UL
#define SCB_I2C_STATUS_BUS_BUSY_Msk             0x1UL
#define SCB_I2C_STATUS_I2C_EC_BUSY_Pos          1UL
#define SCB_I2C_STATUS_I2C_EC_BUSY_Msk          0x2UL
#define SCB_I2C_STATUS_I2CS_IC_BUSY_Pos         2UL
#define SCB_I2C_STATUS_I2CS_IC_BUSY_Msk         0x4UL
#define SCB_I2C_STATUS_S_READ_Pos               4UL
#define SCB_I2C_STATUS_S_READ_Msk               0x10UL
#define SCB_I2C_STATUS_M_READ_Pos               5UL
#define SCB_I2C_STATUS_M_READ_Msk               0x20UL
#define SCB_I2C_STATUS_CURR_EZ_ADDR_Pos         8UL
#define SCB_I2C_STATUS_CURR_EZ_ADDR_Msk         0xFF00UL
#define SCB_I2C_STATUS_BASE_EZ_ADDR_Pos         16UL
#define SCB_I2C_STATUS_BASE_EZ_ADDR_Msk         0xFF0000UL
/* SCB.I2C_M_CMD */
#define SCB_I2C_M_CMD_M_START_Pos               0UL
#define SCB_I2C_M_CMD_M_START_Msk               0x1UL
#define SCB_I2C_M_CMD_M_START_ON_IDLE_Pos       1UL
#define SCB_I2C_M_CMD_M_START_ON_IDLE_Msk       0x2UL
#define SCB_I2C_M_CMD_M_ACK_Pos                 2UL
#define SCB_I2C_M_CMD_M_ACK_Msk                 0x4UL
#define SCB_I2C_M_CMD_M_NACK_Pos                3UL
#define SCB_I2C_M_CMD_M_NACK_Msk                0x8UL
#define SCB_I2C_M_CMD_M_STOP_Pos                4UL
#define SCB_I2C_M_CMD_M_STOP_Msk                0x10UL
/* SCB.I2C_S_CMD */
#define SCB_I2C_S_CMD_S_ACK_Pos                 0UL
#define SCB_I2C_S_CMD_S_ACK_Msk                 0x1UL
#define SCB_I2C_S_CMD_S_NACK_Pos                1UL
#define SCB_I2C_S_CMD_S_NACK_Msk                0x2UL
/* SCB.I2C_CFG */
#define SCB_I2C_CFG_SDA_IN_FILT_TRIM_Pos        0UL
#define SCB_I2C_CFG_SDA_IN_FILT_TRIM_Msk        0x3UL
#define SCB_I2C_CFG_SDA_IN_FILT_SEL_Pos         4UL
#define SCB_I2C_CFG_SDA_IN_FILT_SEL_Msk         0x10UL
#define SCB_I2C_CFG_SCL_IN_FILT_TRIM_Pos        8UL
#define SCB_I2C_CFG_SCL_IN_FILT_TRIM_Msk        0x300UL
#define SCB_I2C_CFG_SCL_IN_FILT_SEL_Pos         12UL
#define SCB_I2C_CFG_SCL_IN_FILT_SEL_Msk         0x1000UL
#define SCB_I2C_CFG_SDA_OUT_FILT0_TRIM_Pos      16UL
#define SCB_I2C_CFG_SDA_OUT_FILT0_TRIM_Msk      0x30000UL
#define SCB_I2C_CFG_SDA_OUT_FILT1_TRIM_Pos      18UL
#define SCB_I2C_CFG_SDA_OUT_FILT1_TRIM_Msk      0xC0000UL
#define SCB_I2C_CFG_SDA_OUT_FILT2_TRIM_Pos      20UL
#define SCB_I2C_CFG_SDA_OUT_FILT2_TRIM_Msk      0x300000UL
#define SCB_I2C_CFG_SDA_OUT_FILT_SEL_Pos        28UL
#define SCB_I2C_CFG_SDA_OUT_FILT_SEL_Msk        0x30000000UL
/* SCB.TX_CTRL */
#define SCB_TX_CTRL_DATA_WIDTH_Pos              0UL
#define SCB_TX_CTRL_DATA_WIDTH_Msk              0x1FUL
#define SCB_TX_CTRL_MSB_FIRST_Pos               8UL
#define SCB_TX_CTRL_MSB_FIRST_Msk               0x100UL
#define SCB_TX_CTRL_OPEN_DRAIN_Pos              16UL
#define SCB_TX_CTRL_OPEN_DRAIN_Msk              0x10000UL
/* SCB.TX_FIFO_CTRL */
#define SCB_TX_FIFO_CTRL_TRIGGER_LEVEL_Pos      0UL
#define SCB_TX_FIFO_CTRL_TRIGGER_LEVEL_Msk      0xFFUL
#define SCB_TX_FIFO_CTRL_CLEAR_Pos              16UL
#define SCB_TX_FIFO_CTRL_CLEAR_Msk              0x10000UL
#define SCB_TX_FIFO_CTRL_FREEZE_Pos             17UL
#define SCB_TX_FIFO_CTRL_FREEZE_Msk             0x20000UL
/* SCB.TX_FIFO_STATUS */
#define SCB_TX_FIFO_STATUS_USED_Pos             0UL
#define SCB_TX_FIFO_STATUS_USED_Msk             0x1FFUL
#define SCB_TX_FIFO_STATUS_SR_VALID_Pos         15UL
#define SCB_TX_FIFO_STATUS_SR_VALID_Msk         0x8000UL
#define SCB_TX_FIFO_STATUS_RD_PTR_Pos           16UL
#define SCB_TX_FIFO_STATUS_RD_PTR_Msk           0xFF0000UL
#define SCB_TX_FIFO_STATUS_WR_PTR_Pos           24UL
#define SCB_TX_FIFO_STATUS_WR_PTR_Msk           0xFF000000UL
/* SCB.TX_FIFO_WR */
#define SCB_TX_FIFO_WR_DATA_Pos                 0UL
#define SCB_TX_FIFO_WR_DATA_Msk                 0xFFFFFFFFUL
/* SCB.RX_CTRL */
#define SCB_RX_CTRL_DATA_WIDTH_Pos              0UL
#define SCB_RX_CTRL_DATA_WIDTH_Msk              0x1FUL
#define SCB_RX_CTRL_MSB_FIRST_Pos               8UL
#define SCB_RX_CTRL_MSB_FIRST_Msk               0x100UL
#define SCB_RX_CTRL_MEDIAN_Pos                  9UL
#define SCB_RX_CTRL_MEDIAN_Msk                  0x200UL
/* SCB.RX_FIFO_CTRL */
#define SCB_RX_FIFO_CTRL_TRIGGER_LEVEL_Pos      0UL
#define SCB_RX_FIFO_CTRL_TRIGGER_LEVEL_Msk      0xFFUL
#define SCB_RX_FIFO_CTRL_CLEAR_Pos              16UL
#define SCB_RX_FIFO_CTRL_CLEAR_Msk              0x10000UL
#define SCB_RX_FIFO_CTRL_FREEZE_Pos             17UL
#define SCB_RX_FIFO_CTRL_FREEZE_Msk             0x20000UL
/* SCB.RX_FIFO_STATUS */
#define SCB_RX_FIFO_STATUS_USED_Pos             0UL
#define SCB_RX_FIFO_STATUS_USED_Msk             0x1FFUL
#define SCB_RX_FIFO_STATUS_SR_VALID_Pos         15UL
#define SCB_RX_FIFO_STATUS_SR_VALID_Msk         0x8000UL
#define SCB_RX_FIFO_STATUS_RD_PTR_Pos           16UL
#define SCB_RX_FIFO_STATUS_RD_PTR_Msk           0xFF0000UL
#define SCB_RX_FIFO_STATUS_WR_PTR_Pos           24UL
#define SCB_RX_FIFO_STATUS_WR_PTR_Msk           0xFF000000UL
/* SCB.RX_MATCH */
#define SCB_RX_MATCH_ADDR_Pos                   0UL
#define SCB_RX_MATCH_ADDR_Msk                   0xFFUL
#define SCB_RX_MATCH_MASK_Pos                   16UL
#define SCB_RX_MATCH_MASK_Msk                   0xFF0000UL
/* SCB.RX_FIFO_RD */
#define SCB_RX_FIFO_RD_DATA_Pos                 0UL
#define SCB_RX_FIFO_RD_DATA_Msk                 0xFFFFFFFFUL
/* SCB.RX_FIFO_RD_SILENT */
#define SCB_RX_FIFO_RD_SILENT_DATA_Pos          0UL
#define SCB_RX_FIFO_RD_SILENT_DATA_Msk          0xFFFFFFFFUL
/* SCB.EZ_DATA */
#define SCB_EZ_DATA_EZ_DATA_Pos                 0UL
#define SCB_EZ_DATA_EZ_DATA_Msk                 0xFFUL
/* SCB.INTR_CAUSE */
#define SCB_INTR_CAUSE_M_Pos                    0UL
#define SCB_INTR_CAUSE_M_Msk                    0x1UL
#define SCB_INTR_CAUSE_S_Pos                    1UL
#define SCB_INTR_CAUSE_S_Msk                    0x2UL
#define SCB_INTR_CAUSE_TX_Pos                   2UL
#define SCB_INTR_CAUSE_TX_Msk                   0x4UL
#define SCB_INTR_CAUSE_RX_Pos                   3UL
#define SCB_INTR_CAUSE_RX_Msk                   0x8UL
#define SCB_INTR_CAUSE_I2C_EC_Pos               4UL
#define SCB_INTR_CAUSE_I2C_EC_Msk               0x10UL
#define SCB_INTR_CAUSE_SPI_EC_Pos               5UL
#define SCB_INTR_CAUSE_SPI_EC_Msk               0x20UL
/* SCB.INTR_I2C_EC */
#define SCB_INTR_I2C_EC_WAKE_UP_Pos             0UL
#define SCB_INTR_I2C_EC_WAKE_UP_Msk             0x1UL
#define SCB_INTR_I2C_EC_EZ_STOP_Pos             1UL
#define SCB_INTR_I2C_EC_EZ_STOP_Msk             0x2UL
#define SCB_INTR_I2C_EC_EZ_WRITE_STOP_Pos       2UL
#define SCB_INTR_I2C_EC_EZ_WRITE_STOP_Msk       0x4UL
#define SCB_INTR_I2C_EC_EZ_READ_STOP_Pos        3UL
#define SCB_INTR_I2C_EC_EZ_READ_STOP_Msk        0x8UL
/* SCB.INTR_I2C_EC_MASK */
#define SCB_INTR_I2C_EC_MASK_WAKE_UP_Pos        0UL
#define SCB_INTR_I2C_EC_MASK_WAKE_UP_Msk        0x1UL
#define SCB_INTR_I2C_EC_MASK_EZ_STOP_Pos        1UL
#define SCB_INTR_I2C_EC_MASK_EZ_STOP_Msk        0x2UL
#define SCB_INTR_I2C_EC_MASK_EZ_WRITE_STOP_Pos  2UL
#define SCB_INTR_I2C_EC_MASK_EZ_WRITE_STOP_Msk  0x4UL
#define SCB_INTR_I2C_EC_MASK_EZ_READ_STOP_Pos   3UL
#define SCB_INTR_I2C_EC_MASK_EZ_READ_STOP_Msk   0x8UL
/* SCB.INTR_I2C_EC_MASKED */
#define SCB_INTR_I2C_EC_MASKED_WAKE_UP_Pos      0UL
#define SCB_INTR_I2C_EC_MASKED_WAKE_UP_Msk      0x1UL
#define SCB_INTR_I2C_EC_MASKED_EZ_STOP_Pos      1UL
#define SCB_INTR_I2C_EC_MASKED_EZ_STOP_Msk      0x2UL
#define SCB_INTR_I2C_EC_MASKED_EZ_WRITE_STOP_Pos 2UL
#define SCB_INTR_I2C_EC_MASKED_EZ_WRITE_STOP_Msk 0x4UL
#define SCB_INTR_I2C_EC_MASKED_EZ_READ_STOP_Pos 3UL
#define SCB_INTR_I2C_EC_MASKED_EZ_READ_STOP_Msk 0x8UL
/* SCB.INTR_SPI_EC */
#define SCB_INTR_SPI_EC_WAKE_UP_Pos             0UL
#define SCB_INTR_SPI_EC_WAKE_UP_Msk             0x1UL
#define SCB_INTR_SPI_EC_EZ_STOP_Pos             1UL
#define SCB_INTR_SPI_EC_EZ_STOP_Msk             0x2UL
#define SCB_INTR_SPI_EC_EZ_WRITE_STOP_Pos       2UL
#define SCB_INTR_SPI_EC_EZ_WRITE_STOP_Msk       0x4UL
#define SCB_INTR_SPI_EC_EZ_READ_STOP_Pos        3UL
#define SCB_INTR_SPI_EC_EZ_READ_STOP_Msk        0x8UL
/* SCB.INTR_SPI_EC_MASK */
#define SCB_INTR_SPI_EC_MASK_WAKE_UP_Pos        0UL
#define SCB_INTR_SPI_EC_MASK_WAKE_UP_Msk        0x1UL
#define SCB_INTR_SPI_EC_MASK_EZ_STOP_Pos        1UL
#define SCB_INTR_SPI_EC_MASK_EZ_STOP_Msk        0x2UL
#define SCB_INTR_SPI_EC_MASK_EZ_WRITE_STOP_Pos  2UL
#define SCB_INTR_SPI_EC_MASK_EZ_WRITE_STOP_Msk  0x4UL
#define SCB_INTR_SPI_EC_MASK_EZ_READ_STOP_Pos   3UL
#define SCB_INTR_SPI_EC_MASK_EZ_READ_STOP_Msk   0x8UL
/* SCB.INTR_SPI_EC_MASKED */
#define SCB_INTR_SPI_EC_MASKED_WAKE_UP_Pos      0UL
#define SCB_INTR_SPI_EC_MASKED_WAKE_UP_Msk      0x1UL
#define SCB_INTR_SPI_EC_MASKED_EZ_STOP_Pos      1UL
#define SCB_INTR_SPI_EC_MASKED_EZ_STOP_Msk      0x2UL
#define SCB_INTR_SPI_EC_MASKED_EZ_WRITE_STOP_Pos 2UL
#define SCB_INTR_SPI_EC_MASKED_EZ_WRITE_STOP_Msk 0x4UL
#define SCB_INTR_SPI_EC_MASKED_EZ_READ_STOP_Pos 3UL
#define SCB_INTR_SPI_EC_MASKED_EZ_READ_STOP_Msk 0x8UL
/* SCB.INTR_M */
#define SCB_INTR_M_I2C_ARB_LOST_Pos             0UL
#define SCB_INTR_M_I2C_ARB_LOST_Msk             0x1UL
#define SCB_INTR_M_I2C_NACK_Pos                 1UL
#define SCB_INTR_M_I2C_NACK_Msk                 0x2UL
#define SCB_INTR_M_I2C_ACK_Pos                  2UL
#define SCB_INTR_M_I2C_ACK_Msk                  0x4UL
#define SCB_INTR_M_I2C_STOP_Pos                 4UL
#define SCB_INTR_M_I2C_STOP_Msk                 0x10UL
#define SCB_INTR_M_I2C_BUS_ERROR_Pos            8UL
#define SCB_INTR_M_I2C_BUS_ERROR_Msk            0x100UL
#define SCB_INTR_M_SPI_DONE_Pos                 9UL
#define SCB_INTR_M_SPI_DONE_Msk                 0x200UL
/* SCB.INTR_M_SET */
#define SCB_INTR_M_SET_I2C_ARB_LOST_Pos         0UL
#define SCB_INTR_M_SET_I2C_ARB_LOST_Msk         0x1UL
#define SCB_INTR_M_SET_I2C_NACK_Pos             1UL
#define SCB_INTR_M_SET_I2C_NACK_Msk             0x2UL
#define SCB_INTR_M_SET_I2C_ACK_Pos              2UL
#define SCB_INTR_M_SET_I2C_ACK_Msk              0x4UL
#define SCB_INTR_M_SET_I2C_STOP_Pos             4UL
#define SCB_INTR_M_SET_I2C_STOP_Msk             0x10UL
#define SCB_INTR_M_SET_I2C_BUS_ERROR_Pos        8UL
#define SCB_INTR_M_SET_I2C_BUS_ERROR_Msk        0x100UL
#define SCB_INTR_M_SET_SPI_DONE_Pos             9UL
#define SCB_INTR_M_SET_SPI_DONE_Msk             0x200UL
/* SCB.INTR_M_MASK */
#define SCB_INTR_M_MASK_I2C_ARB_LOST_Pos        0UL
#define SCB_INTR_M_MASK_I2C_ARB_LOST_Msk        0x1UL
#define SCB_INTR_M_MASK_I2C_NACK_Pos            1UL
#define SCB_INTR_M_MASK_I2C_NACK_Msk            0x2UL
#define SCB_INTR_M_MASK_I2C_ACK_Pos             2UL
#define SCB_INTR_M_MASK_I2C_ACK_Msk             0x4UL
#define SCB_INTR_M_MASK_I2C_STOP_Pos            4UL
#define SCB_INTR_M_MASK_I2C_STOP_Msk            0x10UL
#define SCB_INTR_M_MASK_I2C_BUS_ERROR_Pos       8UL
#define SCB_INTR_M_MASK_I2C_BUS_ERROR_Msk       0x100UL
#define SCB_INTR_M_MASK_SPI_DONE_Pos            9UL
#define SCB_INTR_M_MASK_SPI_DONE_Msk            0x200UL
/* SCB.INTR_M_MASKED */
#define SCB_INTR_M_MASKED_I2C_ARB_LOST_Pos      0UL
#define SCB_INTR_M_MASKED_I2C_ARB_LOST_Msk      0x1UL
#define SCB_INTR_M_MASKED_I2C_NACK_Pos          1UL
#define SCB_INTR_M_MASKED_I2C_NACK_Msk          0x2UL
#define SCB_INTR_M_MASKED_I2C_ACK_Pos           2UL
#define SCB_INTR_M_MASKED_I2C_ACK_Msk           0x4UL
#define SCB_INTR_M_MASKED_I2C_STOP_Pos          4UL
#define SCB_INTR_M_MASKED_I2C_STOP_Msk          0x10UL
#define SCB_INTR_M_MASKED_I2C_BUS_ERROR_Pos     8UL
#define SCB_INTR_M_MASKED_I2C_BUS_ERROR_Msk     0x100UL
#define SCB_INTR_M_MASKED_SPI_DONE_Pos          9UL
#define SCB_INTR_M_MASKED_SPI_DONE_Msk          0x200UL
/* SCB.INTR_S */
#define SCB_INTR_S_I2C_ARB_LOST_Pos             0UL
#define SCB_INTR_S_I2C_ARB_LOST_Msk             0x1UL
#define SCB_INTR_S_I2C_NACK_Pos                 1UL
#define SCB_INTR_S_I2C_NACK_Msk                 0x2UL
#define SCB_INTR_S_I2C_ACK_Pos                  2UL
#define SCB_INTR_S_I2C_ACK_Msk                  0x4UL
#define SCB_INTR_S_I2C_WRITE_STOP_Pos           3UL
#define SCB_INTR_S_I2C_WRITE_STOP_Msk           0x8UL
#define SCB_INTR_S_I2C_STOP_Pos                 4UL
#define SCB_INTR_S_I2C_STOP_Msk                 0x10UL
#define SCB_INTR_S_I2C_START_Pos                5UL
#define SCB_INTR_S_I2C_START_Msk                0x20UL
#define SCB_INTR_S_I2C_ADDR_MATCH_Pos           6UL
#define SCB_INTR_S_I2C_ADDR_MATCH_Msk           0x40UL
#define SCB_INTR_S_I2C_GENERAL_Pos              7UL
#define SCB_INTR_S_I2C_GENERAL_Msk              0x80UL
#define SCB_INTR_S_I2C_BUS_ERROR_Pos            8UL
#define SCB_INTR_S_I2C_BUS_ERROR_Msk            0x100UL
#define SCB_INTR_S_SPI_EZ_WRITE_STOP_Pos        9UL
#define SCB_INTR_S_SPI_EZ_WRITE_STOP_Msk        0x200UL
#define SCB_INTR_S_SPI_EZ_STOP_Pos              10UL
#define SCB_INTR_S_SPI_EZ_STOP_Msk              0x400UL
#define SCB_INTR_S_SPI_BUS_ERROR_Pos            11UL
#define SCB_INTR_S_SPI_BUS_ERROR_Msk            0x800UL
/* SCB.INTR_S_SET */
#define SCB_INTR_S_SET_I2C_ARB_LOST_Pos         0UL
#define SCB_INTR_S_SET_I2C_ARB_LOST_Msk         0x1UL
#define SCB_INTR_S_SET_I2C_NACK_Pos             1UL
#define SCB_INTR_S_SET_I2C_NACK_Msk             0x2UL
#define SCB_INTR_S_SET_I2C_ACK_Pos              2UL
#define SCB_INTR_S_SET_I2C_ACK_Msk              0x4UL
#define SCB_INTR_S_SET_I2C_WRITE_STOP_Pos       3UL
#define SCB_INTR_S_SET_I2C_WRITE_STOP_Msk       0x8UL
#define SCB_INTR_S_SET_I2C_STOP_Pos             4UL
#define SCB_INTR_S_SET_I2C_STOP_Msk             0x10UL
#define SCB_INTR_S_SET_I2C_START_Pos            5UL
#define SCB_INTR_S_SET_I2C_START_Msk            0x20UL
#define SCB_INTR_S_SET_I2C_ADDR_MATCH_Pos       6UL
#define SCB_INTR_S_SET_I2C_ADDR_MATCH_Msk       0x40UL
#define SCB_INTR_S_SET_I2C_GENERAL_Pos          7UL
#define SCB_INTR_S_SET_I2C_GENERAL_Msk          0x80UL
#define SCB_INTR_S_SET_I2C_BUS_ERROR_Pos        8UL
#define SCB_INTR_S_SET_I2C_BUS_ERROR_Msk        0x100UL
#define SCB_INTR_S_SET_SPI_EZ_WRITE_STOP_Pos    9UL
#define SCB_INTR_S_SET_SPI_EZ_WRITE_STOP_Msk    0x200UL
#define SCB_INTR_S_SET_SPI_EZ_STOP_Pos          10UL
#define SCB_INTR_S_SET_SPI_EZ_STOP_Msk          0x400UL
#define SCB_INTR_S_SET_SPI_BUS_ERROR_Pos        11UL
#define SCB_INTR_S_SET_SPI_BUS_ERROR_Msk        0x800UL
/* SCB.INTR_S_MASK */
#define SCB_INTR_S_MASK_I2C_ARB_LOST_Pos        0UL
#define SCB_INTR_S_MASK_I2C_ARB_LOST_Msk        0x1UL
#define SCB_INTR_S_MASK_I2C_NACK_Pos            1UL
#define SCB_INTR_S_MASK_I2C_NACK_Msk            0x2UL
#define SCB_INTR_S_MASK_I2C_ACK_Pos             2UL
#define SCB_INTR_S_MASK_I2C_ACK_Msk             0x4UL
#define SCB_INTR_S_MASK_I2C_WRITE_STOP_Pos      3UL
#define SCB_INTR_S_MASK_I2C_WRITE_STOP_Msk      0x8UL
#define SCB_INTR_S_MASK_I2C_STOP_Pos            4UL
#define SCB_INTR_S_MASK_I2C_STOP_Msk            0x10UL
#define SCB_INTR_S_MASK_I2C_START_Pos           5UL
#define SCB_INTR_S_MASK_I2C_START_Msk           0x20UL
#define SCB_INTR_S_MASK_I2C_ADDR_MATCH_Pos      6UL
#define SCB_INTR_S_MASK_I2C_ADDR_MATCH_Msk      0x40UL
#define SCB_INTR_S_MASK_I2C_GENERAL_Pos         7UL
#define SCB_INTR_S_MASK_I2C_GENERAL_Msk         0x80UL
#define SCB_INTR_S_MASK_I2C_BUS_ERROR_Pos       8UL
#define SCB_INTR_S_MASK_I2C_BUS_ERROR_Msk       0x100UL
#define SCB_INTR_S_MASK_SPI_EZ_WRITE_STOP_Pos   9UL
#define SCB_INTR_S_MASK_SPI_EZ_WRITE_STOP_Msk   0x200UL
#define SCB_INTR_S_MASK_SPI_EZ_STOP_Pos         10UL
#define SCB_INTR_S_MASK_SPI_EZ_STOP_Msk         0x400UL
#define SCB_INTR_S_MASK_SPI_BUS_ERROR_Pos       11UL
#define SCB_INTR_S_MASK_SPI_BUS_ERROR_Msk       0x800UL
/* SCB.INTR_S_MASKED */
#define SCB_INTR_S_MASKED_I2C_ARB_LOST_Pos      0UL
#define SCB_INTR_S_MASKED_I2C_ARB_LOST_Msk      0x1UL
#define SCB_INTR_S_MASKED_I2C_NACK_Pos          1UL
#define SCB_INTR_S_MASKED_I2C_NACK_Msk          0x2UL
#define SCB_INTR_S_MASKED_I2C_ACK_Pos           2UL
#define SCB_INTR_S_MASKED_I2C_ACK_Msk           0x4UL
#define SCB_INTR_S_MASKED_I2C_WRITE_STOP_Pos    3UL
#define SCB_INTR_S_MASKED_I2C_WRITE_STOP_Msk    0x8UL
#define SCB_INTR_S_MASKED_I2C_STOP_Pos          4UL
#define SCB_INTR_S_MASKED_I2C_STOP_Msk          0x10UL
#define SCB_INTR_S_MASKED_I2C_START_Pos         5UL
#define SCB_INTR_S_MASKED_I2C_START_Msk         0x20UL
#define SCB_INTR_S_MASKED_I2C_ADDR_MATCH_Pos    6UL
#define SCB_INTR_S_MASKED_I2C_ADDR_MATCH_Msk    0x40UL
#define SCB_INTR_S_MASKED_I2C_GENERAL_Pos       7UL
#define SCB_INTR_S_MASKED_I2C_GENERAL_Msk       0x80UL
#define SCB_INTR_S_MASKED_I2C_BUS_ERROR_Pos     8UL
#define SCB_INTR_S_MASKED_I2C_BUS_ERROR_Msk     0x100UL
#define SCB_INTR_S_MASKED_SPI_EZ_WRITE_STOP_Pos 9UL
#define SCB_INTR_S_MASKED_SPI_EZ_WRITE_STOP_Msk 0x200UL
#define SCB_INTR_S_MASKED_SPI_EZ_STOP_Pos       10UL
#define SCB_INTR_S_MASKED_SPI_EZ_STOP_Msk       0x400UL
#define SCB_INTR_S_MASKED_SPI_BUS_ERROR_Pos     11UL
#define SCB_INTR_S_MASKED_SPI_BUS_ERROR_Msk     0x800UL
/* SCB.INTR_TX */
#define SCB_INTR_TX_TRIGGER_Pos                 0UL
#define SCB_INTR_TX_TRIGGER_Msk                 0x1UL
#define SCB_INTR_TX_NOT_FULL_Pos                1UL
#define SCB_INTR_TX_NOT_FULL_Msk                0x2UL
#define SCB_INTR_TX_EMPTY_Pos                   4UL
#define SCB_INTR_TX_EMPTY_Msk                   0x10UL
#define SCB_INTR_TX_OVERFLOW_Pos                5UL
#define SCB_INTR_TX_OVERFLOW_Msk                0x20UL
#define SCB_INTR_TX_UNDERFLOW_Pos               6UL
#define SCB_INTR_TX_UNDERFLOW_Msk               0x40UL
#define SCB_INTR_TX_BLOCKED_Pos                 7UL
#define SCB_INTR_TX_BLOCKED_Msk                 0x80UL
#define SCB_INTR_TX_UART_NACK_Pos               8UL
#define SCB_INTR_TX_UART_NACK_Msk               0x100UL
#define SCB_INTR_TX_UART_DONE_Pos               9UL
#define SCB_INTR_TX_UART_DONE_Msk               0x200UL
#define SCB_INTR_TX_UART_ARB_LOST_Pos           10UL
#define SCB_INTR_TX_UART_ARB_LOST_Msk           0x400UL
/* SCB.INTR_TX_SET */
#define SCB_INTR_TX_SET_TRIGGER_Pos             0UL
#define SCB_INTR_TX_SET_TRIGGER_Msk             0x1UL
#define SCB_INTR_TX_SET_NOT_FULL_Pos            1UL
#define SCB_INTR_TX_SET_NOT_FULL_Msk            0x2UL
#define SCB_INTR_TX_SET_EMPTY_Pos               4UL
#define SCB_INTR_TX_SET_EMPTY_Msk               0x10UL
#define SCB_INTR_TX_SET_OVERFLOW_Pos            5UL
#define SCB_INTR_TX_SET_OVERFLOW_Msk            0x20UL
#define SCB_INTR_TX_SET_UNDERFLOW_Pos           6UL
#define SCB_INTR_TX_SET_UNDERFLOW_Msk           0x40UL
#define SCB_INTR_TX_SET_BLOCKED_Pos             7UL
#define SCB_INTR_TX_SET_BLOCKED_Msk             0x80UL
#define SCB_INTR_TX_SET_UART_NACK_Pos           8UL
#define SCB_INTR_TX_SET_UART_NACK_Msk           0x100UL
#define SCB_INTR_TX_SET_UART_DONE_Pos           9UL
#define SCB_INTR_TX_SET_UART_DONE_Msk           0x200UL
#define SCB_INTR_TX_SET_UART_ARB_LOST_Pos       10UL
#define SCB_INTR_TX_SET_UART_ARB_LOST_Msk       0x400UL
/* SCB.INTR_TX_MASK */
#define SCB_INTR_TX_MASK_TRIGGER_Pos            0UL
#define SCB_INTR_TX_MASK_TRIGGER_Msk            0x1UL
#define SCB_INTR_TX_MASK_NOT_FULL_Pos           1UL
#define SCB_INTR_TX_MASK_NOT_FULL_Msk           0x2UL
#define SCB_INTR_TX_MASK_EMPTY_Pos              4UL
#define SCB_INTR_TX_MASK_EMPTY_Msk              0x10UL
#define SCB_INTR_TX_MASK_OVERFLOW_Pos           5UL
#define SCB_INTR_TX_MASK_OVERFLOW_Msk           0x20UL
#define SCB_INTR_TX_MASK_UNDERFLOW_Pos          6UL
#define SCB_INTR_TX_MASK_UNDERFLOW_Msk          0x40UL
#define SCB_INTR_TX_MASK_BLOCKED_Pos            7UL
#define SCB_INTR_TX_MASK_BLOCKED_Msk            0x80UL
#define SCB_INTR_TX_MASK_UART_NACK_Pos          8UL
#define SCB_INTR_TX_MASK_UART_NACK_Msk          0x100UL
#define SCB_INTR_TX_MASK_UART_DONE_Pos          9UL
#define SCB_INTR_TX_MASK_UART_DONE_Msk          0x200UL
#define SCB_INTR_TX_MASK_UART_ARB_LOST_Pos      10UL
#define SCB_INTR_TX_MASK_UART_ARB_LOST_Msk      0x400UL
/* SCB.INTR_TX_MASKED */
#define SCB_INTR_TX_MASKED_TRIGGER_Pos          0UL
#define SCB_INTR_TX_MASKED_TRIGGER_Msk          0x1UL
#define SCB_INTR_TX_MASKED_NOT_FULL_Pos         1UL
#define SCB_INTR_TX_MASKED_NOT_FULL_Msk         0x2UL
#define SCB_INTR_TX_MASKED_EMPTY_Pos            4UL
#define SCB_INTR_TX_MASKED_EMPTY_Msk            0x10UL
#define SCB_INTR_TX_MASKED_OVERFLOW_Pos         5UL
#define SCB_INTR_TX_MASKED_OVERFLOW_Msk         0x20UL
#define SCB_INTR_TX_MASKED_UNDERFLOW_Pos        6UL
#define SCB_INTR_TX_MASKED_UNDERFLOW_Msk        0x40UL
#define SCB_INTR_TX_MASKED_BLOCKED_Pos          7UL
#define SCB_INTR_TX_MASKED_BLOCKED_Msk          0x80UL
#define SCB_INTR_TX_MASKED_UART_NACK_Pos        8UL
#define SCB_INTR_TX_MASKED_UART_NACK_Msk        0x100UL
#define SCB_INTR_TX_MASKED_UART_DONE_Pos        9UL
#define SCB_INTR_TX_MASKED_UART_DONE_Msk        0x200UL
#define SCB_INTR_TX_MASKED_UART_ARB_LOST_Pos    10UL
#define SCB_INTR_TX_MASKED_UART_ARB_LOST_Msk    0x400UL
/* SCB.INTR_RX */
#define SCB_INTR_RX_TRIGGER_Pos                 0UL
#define SCB_INTR_RX_TRIGGER_Msk                 0x1UL
#define SCB_INTR_RX_NOT_EMPTY_Pos               2UL
#define SCB_INTR_RX_NOT_EMPTY_Msk               0x4UL
#define SCB_INTR_RX_FULL_Pos                    3UL
#define SCB_INTR_RX_FULL_Msk                    0x8UL
#define SCB_INTR_RX_OVERFLOW_Pos                5UL
#define SCB_INTR_RX_OVERFLOW_Msk                0x20UL
#define SCB_INTR_RX_UNDERFLOW_Pos               6UL
#define SCB_INTR_RX_UNDERFLOW_Msk               0x40UL
#define SCB_INTR_RX_BLOCKED_Pos                 7UL
#define SCB_INTR_RX_BLOCKED_Msk                 0x80UL
#define SCB_INTR_RX_FRAME_ERROR_Pos             8UL
#define SCB_INTR_RX_FRAME_ERROR_Msk             0x100UL
#define SCB_INTR_RX_PARITY_ERROR_Pos            9UL
#define SCB_INTR_RX_PARITY_ERROR_Msk            0x200UL
#define SCB_INTR_RX_BAUD_DETECT_Pos             10UL
#define SCB_INTR_RX_BAUD_DETECT_Msk             0x400UL
#define SCB_INTR_RX_BREAK_DETECT_Pos            11UL
#define SCB_INTR_RX_BREAK_DETECT_Msk            0x800UL
/* SCB.INTR_RX_SET */
#define SCB_INTR_RX_SET_TRIGGER_Pos             0UL
#define SCB_INTR_RX_SET_TRIGGER_Msk             0x1UL
#define SCB_INTR_RX_SET_NOT_EMPTY_Pos           2UL
#define SCB_INTR_RX_SET_NOT_EMPTY_Msk           0x4UL
#define SCB_INTR_RX_SET_FULL_Pos                3UL
#define SCB_INTR_RX_SET_FULL_Msk                0x8UL
#define SCB_INTR_RX_SET_OVERFLOW_Pos            5UL
#define SCB_INTR_RX_SET_OVERFLOW_Msk            0x20UL
#define SCB_INTR_RX_SET_UNDERFLOW_Pos           6UL
#define SCB_INTR_RX_SET_UNDERFLOW_Msk           0x40UL
#define SCB_INTR_RX_SET_BLOCKED_Pos             7UL
#define SCB_INTR_RX_SET_BLOCKED_Msk             0x80UL
#define SCB_INTR_RX_SET_FRAME_ERROR_Pos         8UL
#define SCB_INTR_RX_SET_FRAME_ERROR_Msk         0x100UL
#define SCB_INTR_RX_SET_PARITY_ERROR_Pos        9UL
#define SCB_INTR_RX_SET_PARITY_ERROR_Msk        0x200UL
#define SCB_INTR_RX_SET_BAUD_DETECT_Pos         10UL
#define SCB_INTR_RX_SET_BAUD_DETECT_Msk         0x400UL
#define SCB_INTR_RX_SET_BREAK_DETECT_Pos        11UL
#define SCB_INTR_RX_SET_BREAK_DETECT_Msk        0x800UL
/* SCB.INTR_RX_MASK */
#define SCB_INTR_RX_MASK_TRIGGER_Pos            0UL
#define SCB_INTR_RX_MASK_TRIGGER_Msk            0x1UL
#define SCB_INTR_RX_MASK_NOT_EMPTY_Pos          2UL
#define SCB_INTR_RX_MASK_NOT_EMPTY_Msk          0x4UL
#define SCB_INTR_RX_MASK_FULL_Pos               3UL
#define SCB_INTR_RX_MASK_FULL_Msk               0x8UL
#define SCB_INTR_RX_MASK_OVERFLOW_Pos           5UL
#define SCB_INTR_RX_MASK_OVERFLOW_Msk           0x20UL
#define SCB_INTR_RX_MASK_UNDERFLOW_Pos          6UL
#define SCB_INTR_RX_MASK_UNDERFLOW_Msk          0x40UL
#define SCB_INTR_RX_MASK_BLOCKED_Pos            7UL
#define SCB_INTR_RX_MASK_BLOCKED_Msk            0x80UL
#define SCB_INTR_RX_MASK_FRAME_ERROR_Pos        8UL
#define SCB_INTR_RX_MASK_FRAME_ERROR_Msk        0x100UL
#define SCB_INTR_RX_MASK_PARITY_ERROR_Pos       9UL
#define SCB_INTR_RX_MASK_PARITY_ERROR_Msk       0x200UL
#define SCB_INTR_RX_MASK_BAUD_DETECT_Pos        10UL
#define SCB_INTR_RX_MASK_BAUD_DETECT_Msk        0x400UL
#define SCB_INTR_RX_MASK_BREAK_DETECT_Pos       11UL
#define SCB_INTR_RX_MASK_BREAK_DETECT_Msk       0x800UL
/* SCB.INTR_RX_MASKED */
#define SCB_INTR_RX_MASKED_TRIGGER_Pos          0UL
#define SCB_INTR_RX_MASKED_TRIGGER_Msk          0x1UL
#define SCB_INTR_RX_MASKED_NOT_EMPTY_Pos        2UL
#define SCB_INTR_RX_MASKED_NOT_EMPTY_Msk        0x4UL
#define SCB_INTR_RX_MASKED_FULL_Pos             3UL
#define SCB_INTR_RX_MASKED_FULL_Msk             0x8UL
#define SCB_INTR_RX_MASKED_OVERFLOW_Pos         5UL
#define SCB_INTR_RX_MASKED_OVERFLOW_Msk         0x20UL
#define SCB_INTR_RX_MASKED_UNDERFLOW_Pos        6UL
#define SCB_INTR_RX_MASKED_UNDERFLOW_Msk        0x40UL
#define SCB_INTR_RX_MASKED_BLOCKED_Pos          7UL
#define SCB_INTR_RX_MASKED_BLOCKED_Msk          0x80UL
#define SCB_INTR_RX_MASKED_FRAME_ERROR_Pos      8UL
#define SCB_INTR_RX_MASKED_FRAME_ERROR_Msk      0x100UL
#define SCB_INTR_RX_MASKED_PARITY_ERROR_Pos     9UL
#define SCB_INTR_RX_MASKED_PARITY_ERROR_Msk     0x200UL
#define SCB_INTR_RX_MASKED_BAUD_DETECT_Pos      10UL
#define SCB_INTR_RX_MASKED_BAUD_DETECT_Msk      0x400UL
#define SCB_INTR_RX_MASKED_BREAK_DETECT_Pos     11UL
#define SCB_INTR_RX_MASKED_BREAK_DETECT_Msk     0x800UL


#endif /* _CYIP_SCB_V2_H_ */


/* [] END OF FILE */
