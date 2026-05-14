/***************************************************************************//**
* \file cyip_cxpi.h
*
* \brief
* CXPI IP definitions
*
* \note
* Generator version: 1.6.0.217
* Database revision: TVIIBE4M_WW2014_BTO
*
********************************************************************************
* \copyright
* Copyright 2016-2020, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYIP_CXPI_H_
#define _CYIP_CXPI_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     CXPI
*******************************************************************************/

#define CXPI_CH_SECTION_SIZE                    0x00000100UL
#define CXPI_SECTION_SIZE                       0x00010000UL

/**
  * \brief Control 0 (CXPI_CH_CTL0)
  */
typedef struct stc_CXPI_CH_CTL0_field {
  uint32_t                         u1MODE:1;
  uint32_t                         :3;
  uint32_t                         u1AUTO_EN:1;
  uint32_t                         :2;
  uint32_t                         u1RXPIDZERO_CHECK_EN:1;
  uint32_t                         u1FILTER_EN:1;
  uint32_t                         :7;
  uint32_t                         u5IFS:5;
  uint32_t                         u4IBS:4;
  uint32_t                         :2;
  uint32_t                         u1BIT_ERROR_IGNORE:1;
  uint32_t                         :2;
  uint32_t                         u1MASTER:1;
  uint32_t                         u1ENABLED:1;
} stc_CXPI_CH_CTL0_field_t;

typedef union un_CXPI_CH_CTL0 {
  uint32_t                         u32Register;
  stc_CXPI_CH_CTL0_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_CTL0_t;

/**
  * \brief Control 1 (CXPI_CH_CTL1)
  */
typedef struct stc_CXPI_CH_CTL1_field {
  uint32_t                         u9T_LOW1:9;
  uint32_t                         :3;
  uint32_t                         u9T_LOW0:9;
  uint32_t                         :1;
  uint32_t                         u9T_OFFSET:9;
} stc_CXPI_CH_CTL1_field_t;

typedef union un_CXPI_CH_CTL1 {
  uint32_t                         u32Register;
  stc_CXPI_CH_CTL1_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_CTL1_t;

/**
  * \brief Control 2 (CXPI_CH_CTL2)
  */
typedef struct stc_CXPI_CH_CTL2_field {
  uint32_t                         u2RETRY:2;
  uint32_t                         :6;
  uint32_t                         u6T_WAKEUP_LENGTH:6;
  uint32_t                         :2;
  uint32_t                         u4TIMEOUT_LENGTH:4;
  uint32_t                         :10;
  uint32_t                         u2TIMEOUT_SEL:2;
} stc_CXPI_CH_CTL2_field_t;

typedef union un_CXPI_CH_CTL2 {
  uint32_t                         u32Register;
  stc_CXPI_CH_CTL2_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_CTL2_t;

/**
  * \brief Status (CXPI_CH_STATUS)
  */
typedef struct stc_CXPI_CH_STATUS_field {
  uint32_t                         u2RETRIES_COUNT:2;
  uint32_t                         :2;
  uint32_t                         u1HEADER_RESPONSE:1;
  uint32_t                         :3;
  uint32_t                         u1TX_BUSY:1;
  uint32_t                         u1RX_BUSY:1;
  uint32_t                         :2;
  uint32_t                         u1TX_DONE:1;
  uint32_t                         u1RX_DONE:1;
  uint32_t                         :4;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         u1TX_HEADER_ARB_LOST:1;
  uint32_t                         u1TX_BIT_ERROR:1;
  uint32_t                         u1RX_CRC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1RX_OVERFLOW_ERROR:1;
  uint32_t                         u1TX_OVERFLOW_ERROR:1;
  uint32_t                         u1RX_UNDERFLOW_ERROR:1;
  uint32_t                         u1TX_UNDERFLOW_ERROR:1;
  uint32_t                         u1RX_FRAME_ERROR:1;
  uint32_t                         u1TX_FRAME_ERROR:1;
} stc_CXPI_CH_STATUS_field_t;

typedef union un_CXPI_CH_STATUS {
  uint32_t                         u32Register;
  stc_CXPI_CH_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_STATUS_t;

/**
  * \brief Command (CXPI_CH_CMD)
  */
typedef struct stc_CXPI_CH_CMD_field {
  uint32_t                         u1TX_HEADER:1;
  uint32_t                         u1TX_RESPONSE:1;
  uint32_t                         u1SLEEP:1;
  uint32_t                         u1WAKE_TO_STANDBY:1;
  uint32_t                         u1TX_WAKE_PULSE:1;
  uint32_t                         u1IFS_WAIT:1;
  uint32_t                         :2;
  uint32_t                         u1RX_HEADER:1;
  uint32_t                         u1RX_RESPONSE:1;
  uint32_t                         :22;
} stc_CXPI_CH_CMD_field_t;

typedef union un_CXPI_CH_CMD {
  uint32_t                         u32Register;
  stc_CXPI_CH_CMD_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_CMD_t;

/**
  * \brief TX/RX status (CXPI_CH_TX_RX_STATUS)
  */
typedef struct stc_CXPI_CH_TX_RX_STATUS_field {
  uint32_t                         :16;
  uint32_t                         u1TX_IN:1;
  uint32_t                         u1RX_IN:1;
  uint32_t                         :6;
  uint32_t                         u1TX_OUT:1;
  uint32_t                         :1;
  uint32_t                         u1EN_OUT:1;
  uint32_t                         :5;
} stc_CXPI_CH_TX_RX_STATUS_field_t;

typedef union un_CXPI_CH_TX_RX_STATUS {
  uint32_t                         u32Register;
  stc_CXPI_CH_TX_RX_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_TX_RX_STATUS_t;

/**
  * \brief TXPID and Frame Information (CXPI_CH_TXPID_FI)
  */
typedef struct stc_CXPI_CH_TXPID_FI_field {
  uint32_t                         u8PID:8;
  uint32_t                         u8FI:8;
  uint32_t                         u8DLCEXT:8;
  uint32_t                         :8;
} stc_CXPI_CH_TXPID_FI_field_t;

typedef union un_CXPI_CH_TXPID_FI {
  uint32_t                         u32Register;
  stc_CXPI_CH_TXPID_FI_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_TXPID_FI_t;

/**
  * \brief RXPID and Frame Information (CXPI_CH_RXPID_FI)
  */
typedef struct stc_CXPI_CH_RXPID_FI_field {
  uint32_t                         u8PID:8;
  uint32_t                         u8FI:8;
  uint32_t                         u8DLCEXT:8;
  uint32_t                         :8;
} stc_CXPI_CH_RXPID_FI_field_t;

typedef union un_CXPI_CH_RXPID_FI {
  uint32_t                         u32Register;
  stc_CXPI_CH_RXPID_FI_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_RXPID_FI_t;

/**
  * \brief CRC (CXPI_CH_CRC)
  */
typedef struct stc_CXPI_CH_CRC_field {
  uint32_t                         u8RXCRC1:8;
  uint32_t                         u8RXCRC2:8;
  uint32_t                         u8TXCRC1:8;
  uint32_t                         u8TXCRC2:8;
} stc_CXPI_CH_CRC_field_t;

typedef union un_CXPI_CH_CRC {
  uint32_t                         u32Register;
  stc_CXPI_CH_CRC_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_CRC_t;

/**
  * \brief TX FIFO control (CXPI_CH_TX_FIFO_CTL)
  */
typedef struct stc_CXPI_CH_TX_FIFO_CTL_field {
  uint32_t                         u5TRIGGER_LEVEL:5;
  uint32_t                         :11;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_CXPI_CH_TX_FIFO_CTL_field_t;

typedef union un_CXPI_CH_TX_FIFO_CTL {
  uint32_t                         u32Register;
  stc_CXPI_CH_TX_FIFO_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_TX_FIFO_CTL_t;

/**
  * \brief TX FIFO status (CXPI_CH_TX_FIFO_STATUS)
  */
typedef struct stc_CXPI_CH_TX_FIFO_STATUS_field {
  uint32_t                         u5USED:5;
  uint32_t                         :11;
  uint32_t                         u5AVAIL:5;
  uint32_t                         :11;
} stc_CXPI_CH_TX_FIFO_STATUS_field_t;

typedef union un_CXPI_CH_TX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_CXPI_CH_TX_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_TX_FIFO_STATUS_t;

/**
  * \brief TX FIFO write (CXPI_CH_TX_FIFO_WR)
  */
typedef struct stc_CXPI_CH_TX_FIFO_WR_field {
  uint32_t                         u8DATA:8;
  uint32_t                         :24;
} stc_CXPI_CH_TX_FIFO_WR_field_t;

typedef union un_CXPI_CH_TX_FIFO_WR {
  uint32_t                         u32Register;
  stc_CXPI_CH_TX_FIFO_WR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_TX_FIFO_WR_t;

/**
  * \brief RX FIFO control (CXPI_CH_RX_FIFO_CTL)
  */
typedef struct stc_CXPI_CH_RX_FIFO_CTL_field {
  uint32_t                         u5TRIGGER_LEVEL:5;
  uint32_t                         :11;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_CXPI_CH_RX_FIFO_CTL_field_t;

typedef union un_CXPI_CH_RX_FIFO_CTL {
  uint32_t                         u32Register;
  stc_CXPI_CH_RX_FIFO_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_RX_FIFO_CTL_t;

/**
  * \brief RX FIFO status (CXPI_CH_RX_FIFO_STATUS)
  */
typedef struct stc_CXPI_CH_RX_FIFO_STATUS_field {
  uint32_t                         u5USED:5;
  uint32_t                         :11;
  uint32_t                         u5AVAIL:5;
  uint32_t                         :11;
} stc_CXPI_CH_RX_FIFO_STATUS_field_t;

typedef union un_CXPI_CH_RX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_CXPI_CH_RX_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_RX_FIFO_STATUS_t;

/**
  * \brief RX FIFO read (CXPI_CH_RX_FIFO_RD)
  */
typedef struct stc_CXPI_CH_RX_FIFO_RD_field {
  uint32_t                         u8DATA:8;
  uint32_t                         :24;
} stc_CXPI_CH_RX_FIFO_RD_field_t;

typedef union un_CXPI_CH_RX_FIFO_RD {
  uint32_t                         u32Register;
  stc_CXPI_CH_RX_FIFO_RD_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_RX_FIFO_RD_t;

/**
  * \brief RX FIFO silent read (CXPI_CH_RX_FIFO_RD_SILENT)
  */
typedef struct stc_CXPI_CH_RX_FIFO_RD_SILENT_field {
  uint32_t                         u8DATA:8;
  uint32_t                         :24;
} stc_CXPI_CH_RX_FIFO_RD_SILENT_field_t;

typedef union un_CXPI_CH_RX_FIFO_RD_SILENT {
  uint32_t                         u32Register;
  stc_CXPI_CH_RX_FIFO_RD_SILENT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_RX_FIFO_RD_SILENT_t;

/**
  * \brief Interrupt (CXPI_CH_INTR)
  */
typedef struct stc_CXPI_CH_INTR_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         u1TX_FIFO_TRIGGER:1;
  uint32_t                         :3;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_WAKEUP_DETECT:1;
  uint32_t                         u1RX_FIFO_TRIGGER:1;
  uint32_t                         u1RX_HEADER_PID_DONE:1;
  uint32_t                         u1TXRX_COMPLETE:1;
  uint32_t                         :4;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         u1TX_HEADER_ARB_LOST:1;
  uint32_t                         u1TX_BIT_ERROR:1;
  uint32_t                         u1RX_CRC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1RX_OVERFLOW_ERROR:1;
  uint32_t                         u1TX_OVERFLOW_ERROR:1;
  uint32_t                         u1RX_UNDERFLOW_ERROR:1;
  uint32_t                         u1TX_UNDERFLOW_ERROR:1;
  uint32_t                         u1RX_FRAME_ERROR:1;
  uint32_t                         u1TX_FRAME_ERROR:1;
} stc_CXPI_CH_INTR_field_t;

typedef union un_CXPI_CH_INTR {
  uint32_t                         u32Register;
  stc_CXPI_CH_INTR_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_INTR_t;

/**
  * \brief Interrupt set (CXPI_CH_INTR_SET)
  */
typedef struct stc_CXPI_CH_INTR_SET_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         u1TX_FIFO_TRIGGER:1;
  uint32_t                         :3;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_WAKEUP_DETECT:1;
  uint32_t                         u1RX_FIFO_TRIGGER:1;
  uint32_t                         u1RX_HEADER_PID_DONE:1;
  uint32_t                         u1TXRX_COMPLETE:1;
  uint32_t                         :4;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         u1TX_HEADER_ARB_LOST:1;
  uint32_t                         u1TX_BIT_ERROR:1;
  uint32_t                         u1RX_CRC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1RX_OVERFLOW_ERROR:1;
  uint32_t                         u1TX_OVERFLOW_ERROR:1;
  uint32_t                         u1RX_UNDERFLOW_ERROR:1;
  uint32_t                         u1TX_UNDERFLOW_ERROR:1;
  uint32_t                         u1RX_FRAME_ERROR:1;
  uint32_t                         u1TX_FRAME_ERROR:1;
} stc_CXPI_CH_INTR_SET_field_t;

typedef union un_CXPI_CH_INTR_SET {
  uint32_t                         u32Register;
  stc_CXPI_CH_INTR_SET_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_INTR_SET_t;

/**
  * \brief Interrupt mask (CXPI_CH_INTR_MASK)
  */
typedef struct stc_CXPI_CH_INTR_MASK_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         u1TX_FIFO_TRIGGER:1;
  uint32_t                         :3;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_WAKEUP_DETECT:1;
  uint32_t                         u1RX_FIFO_TRIGGER:1;
  uint32_t                         u1RX_HEADER_PID_DONE:1;
  uint32_t                         u1TXRX_COMPLETE:1;
  uint32_t                         :4;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         u1TX_HEADER_ARB_LOST:1;
  uint32_t                         u1TX_BIT_ERROR:1;
  uint32_t                         u1RX_CRC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1RX_OVERFLOW_ERROR:1;
  uint32_t                         u1TX_OVERFLOW_ERROR:1;
  uint32_t                         u1RX_UNDERFLOW_ERROR:1;
  uint32_t                         u1TX_UNDERFLOW_ERROR:1;
  uint32_t                         u1RX_FRAME_ERROR:1;
  uint32_t                         u1TX_FRAME_ERROR:1;
} stc_CXPI_CH_INTR_MASK_field_t;

typedef union un_CXPI_CH_INTR_MASK {
  uint32_t                         u32Register;
  stc_CXPI_CH_INTR_MASK_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_INTR_MASK_t;

/**
  * \brief Interrupt masked (CXPI_CH_INTR_MASKED)
  */
typedef struct stc_CXPI_CH_INTR_MASKED_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         u1TX_FIFO_TRIGGER:1;
  uint32_t                         :3;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_WAKEUP_DETECT:1;
  uint32_t                         u1RX_FIFO_TRIGGER:1;
  uint32_t                         u1RX_HEADER_PID_DONE:1;
  uint32_t                         u1TXRX_COMPLETE:1;
  uint32_t                         :4;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         u1TX_HEADER_ARB_LOST:1;
  uint32_t                         u1TX_BIT_ERROR:1;
  uint32_t                         u1RX_CRC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         u1RX_OVERFLOW_ERROR:1;
  uint32_t                         u1TX_OVERFLOW_ERROR:1;
  uint32_t                         u1RX_UNDERFLOW_ERROR:1;
  uint32_t                         u1TX_UNDERFLOW_ERROR:1;
  uint32_t                         u1RX_FRAME_ERROR:1;
  uint32_t                         u1TX_FRAME_ERROR:1;
} stc_CXPI_CH_INTR_MASKED_field_t;

typedef union un_CXPI_CH_INTR_MASKED {
  uint32_t                         u32Register;
  stc_CXPI_CH_INTR_MASKED_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_CH_INTR_MASKED_t;



/**
  * \brief Error control (CXPI_ERROR_CTL)
  */
typedef struct stc_CXPI_ERROR_CTL_field {
  uint32_t                         u5CH_IDX:5;
  uint32_t                         :13;
  uint32_t                         u1TX_CRC_ERROR:1;
  uint32_t                         u1TX_PID_PARITY_ERROR:1;
  uint32_t                         u1TX_DATA_LENGTH_ERROR:1;
  uint32_t                         :4;
  uint32_t                         u1TX_DATA_STOP_ERROR:1;
  uint32_t                         :5;
  uint32_t                         u1ENABLED:1;
} stc_CXPI_ERROR_CTL_field_t;

typedef union un_CXPI_ERROR_CTL {
  uint32_t                         u32Register;
  stc_CXPI_ERROR_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_ERROR_CTL_t;

/**
  * \brief Test control (CXPI_TEST_CTL)
  */
typedef struct stc_CXPI_TEST_CTL_field {
  uint32_t                         u5CH_IDX:5;
  uint32_t                         :11;
  uint32_t                         u1MODE:1;
  uint32_t                         :14;
  uint32_t                         u1ENABLED:1;
} stc_CXPI_TEST_CTL_field_t;

typedef union un_CXPI_TEST_CTL {
  uint32_t                         u32Register;
  stc_CXPI_TEST_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CXPI_TEST_CTL_t;



/**
  * \brief CXPI channel structure (CXPI_CH)
  */
typedef struct stc_CXPI_CH {
  un_CXPI_CH_CTL0_t                unCTL0;             /*!< 0x00000000 Control 0 */
  un_CXPI_CH_CTL1_t                unCTL1;             /*!< 0x00000004 Control 1 */
  un_CXPI_CH_CTL2_t                unCTL2;             /*!< 0x00000008 Control 2 */
  un_CXPI_CH_STATUS_t              unSTATUS;           /*!< 0x0000000C Status */
  un_CXPI_CH_CMD_t                 unCMD;              /*!< 0x00000010 Command */
  uint32_t                         au32Reserved[11];
  un_CXPI_CH_TX_RX_STATUS_t        unTX_RX_STATUS;     /*!< 0x00000040 TX/RX status */
  uint32_t                         au32Reserved1[3];
  un_CXPI_CH_TXPID_FI_t            unTXPID_FI;         /*!< 0x00000050 TXPID and Frame Information */
  un_CXPI_CH_RXPID_FI_t            unRXPID_FI;         /*!< 0x00000054 RXPID and Frame Information */
  un_CXPI_CH_CRC_t                 unCRC;              /*!< 0x00000058 CRC */
  uint32_t                         au32Reserved2[9];
  un_CXPI_CH_TX_FIFO_CTL_t         unTX_FIFO_CTL;      /*!< 0x00000080 TX FIFO control */
  un_CXPI_CH_TX_FIFO_STATUS_t      unTX_FIFO_STATUS;   /*!< 0x00000084 TX FIFO status */
  un_CXPI_CH_TX_FIFO_WR_t          unTX_FIFO_WR;       /*!< 0x00000088 TX FIFO write */
  uint32_t                         au32Reserved3[5];
  un_CXPI_CH_RX_FIFO_CTL_t         unRX_FIFO_CTL;      /*!< 0x000000A0 RX FIFO control */
  un_CXPI_CH_RX_FIFO_STATUS_t      unRX_FIFO_STATUS;   /*!< 0x000000A4 RX FIFO status */
  un_CXPI_CH_RX_FIFO_RD_t          unRX_FIFO_RD;       /*!< 0x000000A8 RX FIFO read */
  un_CXPI_CH_RX_FIFO_RD_SILENT_t   unRX_FIFO_RD_SILENT; /*!< 0x000000AC RX FIFO silent read */
  uint32_t                         au32Reserved4[4];
  un_CXPI_CH_INTR_t                unINTR;             /*!< 0x000000C0 Interrupt */
  un_CXPI_CH_INTR_SET_t            unINTR_SET;         /*!< 0x000000C4 Interrupt set */
  un_CXPI_CH_INTR_MASK_t           unINTR_MASK;        /*!< 0x000000C8 Interrupt mask */
  un_CXPI_CH_INTR_MASKED_t         unINTR_MASKED;      /*!< 0x000000CC Interrupt masked */
  uint32_t                         au32Reserved5[12];
} stc_CXPI_CH_t;                                       /*!< Size = 256 (0x100) */

/**
  * \brief CXPI (CXPI)
  */
typedef struct stc_CXPI {
  un_CXPI_ERROR_CTL_t              unERROR_CTL;        /*!< 0x00000000 Error control */
  un_CXPI_TEST_CTL_t               unTEST_CTL;         /*!< 0x00000004 Test control */
  uint32_t                         au32Reserved[8190];
  stc_CXPI_CH_t                    CH[4];              /*!< 0x00008000 CXPI channel structure */
} stc_CXPI_t;                                          /*!< Size = 33792 (0x8400) */


/* CXPI_CH.CTL0 */
#define CXPI_CH_CTL0_MODE_Pos                   0UL
#define CXPI_CH_CTL0_MODE_Msk                   0x1UL
#define CXPI_CH_CTL0_AUTO_EN_Pos                4UL
#define CXPI_CH_CTL0_AUTO_EN_Msk                0x10UL
#define CXPI_CH_CTL0_RXPIDZERO_CHECK_EN_Pos     7UL
#define CXPI_CH_CTL0_RXPIDZERO_CHECK_EN_Msk     0x80UL
#define CXPI_CH_CTL0_FILTER_EN_Pos              8UL
#define CXPI_CH_CTL0_FILTER_EN_Msk              0x100UL
#define CXPI_CH_CTL0_IFS_Pos                    16UL
#define CXPI_CH_CTL0_IFS_Msk                    0x1F0000UL
#define CXPI_CH_CTL0_IBS_Pos                    21UL
#define CXPI_CH_CTL0_IBS_Msk                    0x1E00000UL
#define CXPI_CH_CTL0_BIT_ERROR_IGNORE_Pos       27UL
#define CXPI_CH_CTL0_BIT_ERROR_IGNORE_Msk       0x8000000UL
#define CXPI_CH_CTL0_MASTER_Pos                 30UL
#define CXPI_CH_CTL0_MASTER_Msk                 0x40000000UL
#define CXPI_CH_CTL0_ENABLED_Pos                31UL
#define CXPI_CH_CTL0_ENABLED_Msk                0x80000000UL
/* CXPI_CH.CTL1 */
#define CXPI_CH_CTL1_T_LOW1_Pos                 0UL
#define CXPI_CH_CTL1_T_LOW1_Msk                 0x1FFUL
#define CXPI_CH_CTL1_T_LOW0_Pos                 12UL
#define CXPI_CH_CTL1_T_LOW0_Msk                 0x1FF000UL
#define CXPI_CH_CTL1_T_OFFSET_Pos               22UL
#define CXPI_CH_CTL1_T_OFFSET_Msk               0x7FC00000UL
/* CXPI_CH.CTL2 */
#define CXPI_CH_CTL2_RETRY_Pos                  0UL
#define CXPI_CH_CTL2_RETRY_Msk                  0x3UL
#define CXPI_CH_CTL2_T_WAKEUP_LENGTH_Pos        8UL
#define CXPI_CH_CTL2_T_WAKEUP_LENGTH_Msk        0x3F00UL
#define CXPI_CH_CTL2_TIMEOUT_LENGTH_Pos         16UL
#define CXPI_CH_CTL2_TIMEOUT_LENGTH_Msk         0xF0000UL
#define CXPI_CH_CTL2_TIMEOUT_SEL_Pos            30UL
#define CXPI_CH_CTL2_TIMEOUT_SEL_Msk            0xC0000000UL
/* CXPI_CH.STATUS */
#define CXPI_CH_STATUS_RETRIES_COUNT_Pos        0UL
#define CXPI_CH_STATUS_RETRIES_COUNT_Msk        0x3UL
#define CXPI_CH_STATUS_HEADER_RESPONSE_Pos      4UL
#define CXPI_CH_STATUS_HEADER_RESPONSE_Msk      0x10UL
#define CXPI_CH_STATUS_TX_BUSY_Pos              8UL
#define CXPI_CH_STATUS_TX_BUSY_Msk              0x100UL
#define CXPI_CH_STATUS_RX_BUSY_Pos              9UL
#define CXPI_CH_STATUS_RX_BUSY_Msk              0x200UL
#define CXPI_CH_STATUS_TX_DONE_Pos              12UL
#define CXPI_CH_STATUS_TX_DONE_Msk              0x1000UL
#define CXPI_CH_STATUS_RX_DONE_Pos              13UL
#define CXPI_CH_STATUS_RX_DONE_Msk              0x2000UL
#define CXPI_CH_STATUS_TIMEOUT_Pos              18UL
#define CXPI_CH_STATUS_TIMEOUT_Msk              0x40000UL
#define CXPI_CH_STATUS_TX_HEADER_ARB_LOST_Pos   19UL
#define CXPI_CH_STATUS_TX_HEADER_ARB_LOST_Msk   0x80000UL
#define CXPI_CH_STATUS_TX_BIT_ERROR_Pos         20UL
#define CXPI_CH_STATUS_TX_BIT_ERROR_Msk         0x100000UL
#define CXPI_CH_STATUS_RX_CRC_ERROR_Pos         21UL
#define CXPI_CH_STATUS_RX_CRC_ERROR_Msk         0x200000UL
#define CXPI_CH_STATUS_RX_HEADER_PARITY_ERROR_Pos 22UL
#define CXPI_CH_STATUS_RX_HEADER_PARITY_ERROR_Msk 0x400000UL
#define CXPI_CH_STATUS_RX_DATA_LENGTH_ERROR_Pos 23UL
#define CXPI_CH_STATUS_RX_DATA_LENGTH_ERROR_Msk 0x800000UL
#define CXPI_CH_STATUS_TX_DATA_LENGTH_ERROR_Pos 24UL
#define CXPI_CH_STATUS_TX_DATA_LENGTH_ERROR_Msk 0x1000000UL
#define CXPI_CH_STATUS_RX_OVERFLOW_ERROR_Pos    25UL
#define CXPI_CH_STATUS_RX_OVERFLOW_ERROR_Msk    0x2000000UL
#define CXPI_CH_STATUS_TX_OVERFLOW_ERROR_Pos    26UL
#define CXPI_CH_STATUS_TX_OVERFLOW_ERROR_Msk    0x4000000UL
#define CXPI_CH_STATUS_RX_UNDERFLOW_ERROR_Pos   27UL
#define CXPI_CH_STATUS_RX_UNDERFLOW_ERROR_Msk   0x8000000UL
#define CXPI_CH_STATUS_TX_UNDERFLOW_ERROR_Pos   28UL
#define CXPI_CH_STATUS_TX_UNDERFLOW_ERROR_Msk   0x10000000UL
#define CXPI_CH_STATUS_RX_FRAME_ERROR_Pos       29UL
#define CXPI_CH_STATUS_RX_FRAME_ERROR_Msk       0x20000000UL
#define CXPI_CH_STATUS_TX_FRAME_ERROR_Pos       30UL
#define CXPI_CH_STATUS_TX_FRAME_ERROR_Msk       0x40000000UL
/* CXPI_CH.CMD */
#define CXPI_CH_CMD_TX_HEADER_Pos               0UL
#define CXPI_CH_CMD_TX_HEADER_Msk               0x1UL
#define CXPI_CH_CMD_TX_RESPONSE_Pos             1UL
#define CXPI_CH_CMD_TX_RESPONSE_Msk             0x2UL
#define CXPI_CH_CMD_SLEEP_Pos                   2UL
#define CXPI_CH_CMD_SLEEP_Msk                   0x4UL
#define CXPI_CH_CMD_WAKE_TO_STANDBY_Pos         3UL
#define CXPI_CH_CMD_WAKE_TO_STANDBY_Msk         0x8UL
#define CXPI_CH_CMD_TX_WAKE_PULSE_Pos           4UL
#define CXPI_CH_CMD_TX_WAKE_PULSE_Msk           0x10UL
#define CXPI_CH_CMD_IFS_WAIT_Pos                5UL
#define CXPI_CH_CMD_IFS_WAIT_Msk                0x20UL
#define CXPI_CH_CMD_RX_HEADER_Pos               8UL
#define CXPI_CH_CMD_RX_HEADER_Msk               0x100UL
#define CXPI_CH_CMD_RX_RESPONSE_Pos             9UL
#define CXPI_CH_CMD_RX_RESPONSE_Msk             0x200UL
/* CXPI_CH.TX_RX_STATUS */
#define CXPI_CH_TX_RX_STATUS_TX_IN_Pos          16UL
#define CXPI_CH_TX_RX_STATUS_TX_IN_Msk          0x10000UL
#define CXPI_CH_TX_RX_STATUS_RX_IN_Pos          17UL
#define CXPI_CH_TX_RX_STATUS_RX_IN_Msk          0x20000UL
#define CXPI_CH_TX_RX_STATUS_TX_OUT_Pos         24UL
#define CXPI_CH_TX_RX_STATUS_TX_OUT_Msk         0x1000000UL
#define CXPI_CH_TX_RX_STATUS_EN_OUT_Pos         26UL
#define CXPI_CH_TX_RX_STATUS_EN_OUT_Msk         0x4000000UL
/* CXPI_CH.TXPID_FI */
#define CXPI_CH_TXPID_FI_PID_Pos                0UL
#define CXPI_CH_TXPID_FI_PID_Msk                0xFFUL
#define CXPI_CH_TXPID_FI_FI_Pos                 8UL
#define CXPI_CH_TXPID_FI_FI_Msk                 0xFF00UL
#define CXPI_CH_TXPID_FI_DLCEXT_Pos             16UL
#define CXPI_CH_TXPID_FI_DLCEXT_Msk             0xFF0000UL
/* CXPI_CH.RXPID_FI */
#define CXPI_CH_RXPID_FI_PID_Pos                0UL
#define CXPI_CH_RXPID_FI_PID_Msk                0xFFUL
#define CXPI_CH_RXPID_FI_FI_Pos                 8UL
#define CXPI_CH_RXPID_FI_FI_Msk                 0xFF00UL
#define CXPI_CH_RXPID_FI_DLCEXT_Pos             16UL
#define CXPI_CH_RXPID_FI_DLCEXT_Msk             0xFF0000UL
/* CXPI_CH.CRC */
#define CXPI_CH_CRC_RXCRC1_Pos                  0UL
#define CXPI_CH_CRC_RXCRC1_Msk                  0xFFUL
#define CXPI_CH_CRC_RXCRC2_Pos                  8UL
#define CXPI_CH_CRC_RXCRC2_Msk                  0xFF00UL
#define CXPI_CH_CRC_TXCRC1_Pos                  16UL
#define CXPI_CH_CRC_TXCRC1_Msk                  0xFF0000UL
#define CXPI_CH_CRC_TXCRC2_Pos                  24UL
#define CXPI_CH_CRC_TXCRC2_Msk                  0xFF000000UL
/* CXPI_CH.TX_FIFO_CTL */
#define CXPI_CH_TX_FIFO_CTL_TRIGGER_LEVEL_Pos   0UL
#define CXPI_CH_TX_FIFO_CTL_TRIGGER_LEVEL_Msk   0x1FUL
#define CXPI_CH_TX_FIFO_CTL_CLEAR_Pos           16UL
#define CXPI_CH_TX_FIFO_CTL_CLEAR_Msk           0x10000UL
#define CXPI_CH_TX_FIFO_CTL_FREEZE_Pos          17UL
#define CXPI_CH_TX_FIFO_CTL_FREEZE_Msk          0x20000UL
/* CXPI_CH.TX_FIFO_STATUS */
#define CXPI_CH_TX_FIFO_STATUS_USED_Pos         0UL
#define CXPI_CH_TX_FIFO_STATUS_USED_Msk         0x1FUL
#define CXPI_CH_TX_FIFO_STATUS_AVAIL_Pos        16UL
#define CXPI_CH_TX_FIFO_STATUS_AVAIL_Msk        0x1F0000UL
/* CXPI_CH.TX_FIFO_WR */
#define CXPI_CH_TX_FIFO_WR_DATA_Pos             0UL
#define CXPI_CH_TX_FIFO_WR_DATA_Msk             0xFFUL
/* CXPI_CH.RX_FIFO_CTL */
#define CXPI_CH_RX_FIFO_CTL_TRIGGER_LEVEL_Pos   0UL
#define CXPI_CH_RX_FIFO_CTL_TRIGGER_LEVEL_Msk   0x1FUL
#define CXPI_CH_RX_FIFO_CTL_CLEAR_Pos           16UL
#define CXPI_CH_RX_FIFO_CTL_CLEAR_Msk           0x10000UL
#define CXPI_CH_RX_FIFO_CTL_FREEZE_Pos          17UL
#define CXPI_CH_RX_FIFO_CTL_FREEZE_Msk          0x20000UL
/* CXPI_CH.RX_FIFO_STATUS */
#define CXPI_CH_RX_FIFO_STATUS_USED_Pos         0UL
#define CXPI_CH_RX_FIFO_STATUS_USED_Msk         0x1FUL
#define CXPI_CH_RX_FIFO_STATUS_AVAIL_Pos        16UL
#define CXPI_CH_RX_FIFO_STATUS_AVAIL_Msk        0x1F0000UL
/* CXPI_CH.RX_FIFO_RD */
#define CXPI_CH_RX_FIFO_RD_DATA_Pos             0UL
#define CXPI_CH_RX_FIFO_RD_DATA_Msk             0xFFUL
/* CXPI_CH.RX_FIFO_RD_SILENT */
#define CXPI_CH_RX_FIFO_RD_SILENT_DATA_Pos      0UL
#define CXPI_CH_RX_FIFO_RD_SILENT_DATA_Msk      0xFFUL
/* CXPI_CH.INTR */
#define CXPI_CH_INTR_TX_HEADER_DONE_Pos         0UL
#define CXPI_CH_INTR_TX_HEADER_DONE_Msk         0x1UL
#define CXPI_CH_INTR_TX_RESPONSE_DONE_Pos       1UL
#define CXPI_CH_INTR_TX_RESPONSE_DONE_Msk       0x2UL
#define CXPI_CH_INTR_TX_WAKEUP_DONE_Pos         3UL
#define CXPI_CH_INTR_TX_WAKEUP_DONE_Msk         0x8UL
#define CXPI_CH_INTR_TX_FIFO_TRIGGER_Pos        4UL
#define CXPI_CH_INTR_TX_FIFO_TRIGGER_Msk        0x10UL
#define CXPI_CH_INTR_RX_HEADER_DONE_Pos         8UL
#define CXPI_CH_INTR_RX_HEADER_DONE_Msk         0x100UL
#define CXPI_CH_INTR_RX_RESPONSE_DONE_Pos       9UL
#define CXPI_CH_INTR_RX_RESPONSE_DONE_Msk       0x200UL
#define CXPI_CH_INTR_RX_WAKEUP_DETECT_Pos       10UL
#define CXPI_CH_INTR_RX_WAKEUP_DETECT_Msk       0x400UL
#define CXPI_CH_INTR_RX_FIFO_TRIGGER_Pos        11UL
#define CXPI_CH_INTR_RX_FIFO_TRIGGER_Msk        0x800UL
#define CXPI_CH_INTR_RX_HEADER_PID_DONE_Pos     12UL
#define CXPI_CH_INTR_RX_HEADER_PID_DONE_Msk     0x1000UL
#define CXPI_CH_INTR_TXRX_COMPLETE_Pos          13UL
#define CXPI_CH_INTR_TXRX_COMPLETE_Msk          0x2000UL
#define CXPI_CH_INTR_TIMEOUT_Pos                18UL
#define CXPI_CH_INTR_TIMEOUT_Msk                0x40000UL
#define CXPI_CH_INTR_TX_HEADER_ARB_LOST_Pos     19UL
#define CXPI_CH_INTR_TX_HEADER_ARB_LOST_Msk     0x80000UL
#define CXPI_CH_INTR_TX_BIT_ERROR_Pos           20UL
#define CXPI_CH_INTR_TX_BIT_ERROR_Msk           0x100000UL
#define CXPI_CH_INTR_RX_CRC_ERROR_Pos           21UL
#define CXPI_CH_INTR_RX_CRC_ERROR_Msk           0x200000UL
#define CXPI_CH_INTR_RX_HEADER_PARITY_ERROR_Pos 22UL
#define CXPI_CH_INTR_RX_HEADER_PARITY_ERROR_Msk 0x400000UL
#define CXPI_CH_INTR_RX_DATA_LENGTH_ERROR_Pos   23UL
#define CXPI_CH_INTR_RX_DATA_LENGTH_ERROR_Msk   0x800000UL
#define CXPI_CH_INTR_TX_DATA_LENGTH_ERROR_Pos   24UL
#define CXPI_CH_INTR_TX_DATA_LENGTH_ERROR_Msk   0x1000000UL
#define CXPI_CH_INTR_RX_OVERFLOW_ERROR_Pos      25UL
#define CXPI_CH_INTR_RX_OVERFLOW_ERROR_Msk      0x2000000UL
#define CXPI_CH_INTR_TX_OVERFLOW_ERROR_Pos      26UL
#define CXPI_CH_INTR_TX_OVERFLOW_ERROR_Msk      0x4000000UL
#define CXPI_CH_INTR_RX_UNDERFLOW_ERROR_Pos     27UL
#define CXPI_CH_INTR_RX_UNDERFLOW_ERROR_Msk     0x8000000UL
#define CXPI_CH_INTR_TX_UNDERFLOW_ERROR_Pos     28UL
#define CXPI_CH_INTR_TX_UNDERFLOW_ERROR_Msk     0x10000000UL
#define CXPI_CH_INTR_RX_FRAME_ERROR_Pos         29UL
#define CXPI_CH_INTR_RX_FRAME_ERROR_Msk         0x20000000UL
#define CXPI_CH_INTR_TX_FRAME_ERROR_Pos         30UL
#define CXPI_CH_INTR_TX_FRAME_ERROR_Msk         0x40000000UL
/* CXPI_CH.INTR_SET */
#define CXPI_CH_INTR_SET_TX_HEADER_DONE_Pos     0UL
#define CXPI_CH_INTR_SET_TX_HEADER_DONE_Msk     0x1UL
#define CXPI_CH_INTR_SET_TX_RESPONSE_DONE_Pos   1UL
#define CXPI_CH_INTR_SET_TX_RESPONSE_DONE_Msk   0x2UL
#define CXPI_CH_INTR_SET_TX_WAKEUP_DONE_Pos     3UL
#define CXPI_CH_INTR_SET_TX_WAKEUP_DONE_Msk     0x8UL
#define CXPI_CH_INTR_SET_TX_FIFO_TRIGGER_Pos    4UL
#define CXPI_CH_INTR_SET_TX_FIFO_TRIGGER_Msk    0x10UL
#define CXPI_CH_INTR_SET_RX_HEADER_DONE_Pos     8UL
#define CXPI_CH_INTR_SET_RX_HEADER_DONE_Msk     0x100UL
#define CXPI_CH_INTR_SET_RX_RESPONSE_DONE_Pos   9UL
#define CXPI_CH_INTR_SET_RX_RESPONSE_DONE_Msk   0x200UL
#define CXPI_CH_INTR_SET_RX_WAKEUP_DETECT_Pos   10UL
#define CXPI_CH_INTR_SET_RX_WAKEUP_DETECT_Msk   0x400UL
#define CXPI_CH_INTR_SET_RX_FIFO_TRIGGER_Pos    11UL
#define CXPI_CH_INTR_SET_RX_FIFO_TRIGGER_Msk    0x800UL
#define CXPI_CH_INTR_SET_RX_HEADER_PID_DONE_Pos 12UL
#define CXPI_CH_INTR_SET_RX_HEADER_PID_DONE_Msk 0x1000UL
#define CXPI_CH_INTR_SET_TXRX_COMPLETE_Pos      13UL
#define CXPI_CH_INTR_SET_TXRX_COMPLETE_Msk      0x2000UL
#define CXPI_CH_INTR_SET_TIMEOUT_Pos            18UL
#define CXPI_CH_INTR_SET_TIMEOUT_Msk            0x40000UL
#define CXPI_CH_INTR_SET_TX_HEADER_ARB_LOST_Pos 19UL
#define CXPI_CH_INTR_SET_TX_HEADER_ARB_LOST_Msk 0x80000UL
#define CXPI_CH_INTR_SET_TX_BIT_ERROR_Pos       20UL
#define CXPI_CH_INTR_SET_TX_BIT_ERROR_Msk       0x100000UL
#define CXPI_CH_INTR_SET_RX_CRC_ERROR_Pos       21UL
#define CXPI_CH_INTR_SET_RX_CRC_ERROR_Msk       0x200000UL
#define CXPI_CH_INTR_SET_RX_HEADER_PARITY_ERROR_Pos 22UL
#define CXPI_CH_INTR_SET_RX_HEADER_PARITY_ERROR_Msk 0x400000UL
#define CXPI_CH_INTR_SET_RX_DATA_LENGTH_ERROR_Pos 23UL
#define CXPI_CH_INTR_SET_RX_DATA_LENGTH_ERROR_Msk 0x800000UL
#define CXPI_CH_INTR_SET_TX_DATA_LENGTH_ERROR_Pos 24UL
#define CXPI_CH_INTR_SET_TX_DATA_LENGTH_ERROR_Msk 0x1000000UL
#define CXPI_CH_INTR_SET_RX_OVERFLOW_ERROR_Pos  25UL
#define CXPI_CH_INTR_SET_RX_OVERFLOW_ERROR_Msk  0x2000000UL
#define CXPI_CH_INTR_SET_TX_OVERFLOW_ERROR_Pos  26UL
#define CXPI_CH_INTR_SET_TX_OVERFLOW_ERROR_Msk  0x4000000UL
#define CXPI_CH_INTR_SET_RX_UNDERFLOW_ERROR_Pos 27UL
#define CXPI_CH_INTR_SET_RX_UNDERFLOW_ERROR_Msk 0x8000000UL
#define CXPI_CH_INTR_SET_TX_UNDERFLOW_ERROR_Pos 28UL
#define CXPI_CH_INTR_SET_TX_UNDERFLOW_ERROR_Msk 0x10000000UL
#define CXPI_CH_INTR_SET_RX_FRAME_ERROR_Pos     29UL
#define CXPI_CH_INTR_SET_RX_FRAME_ERROR_Msk     0x20000000UL
#define CXPI_CH_INTR_SET_TX_FRAME_ERROR_Pos     30UL
#define CXPI_CH_INTR_SET_TX_FRAME_ERROR_Msk     0x40000000UL
/* CXPI_CH.INTR_MASK */
#define CXPI_CH_INTR_MASK_TX_HEADER_DONE_Pos    0UL
#define CXPI_CH_INTR_MASK_TX_HEADER_DONE_Msk    0x1UL
#define CXPI_CH_INTR_MASK_TX_RESPONSE_DONE_Pos  1UL
#define CXPI_CH_INTR_MASK_TX_RESPONSE_DONE_Msk  0x2UL
#define CXPI_CH_INTR_MASK_TX_WAKEUP_DONE_Pos    3UL
#define CXPI_CH_INTR_MASK_TX_WAKEUP_DONE_Msk    0x8UL
#define CXPI_CH_INTR_MASK_TX_FIFO_TRIGGER_Pos   4UL
#define CXPI_CH_INTR_MASK_TX_FIFO_TRIGGER_Msk   0x10UL
#define CXPI_CH_INTR_MASK_RX_HEADER_DONE_Pos    8UL
#define CXPI_CH_INTR_MASK_RX_HEADER_DONE_Msk    0x100UL
#define CXPI_CH_INTR_MASK_RX_RESPONSE_DONE_Pos  9UL
#define CXPI_CH_INTR_MASK_RX_RESPONSE_DONE_Msk  0x200UL
#define CXPI_CH_INTR_MASK_RX_WAKEUP_DETECT_Pos  10UL
#define CXPI_CH_INTR_MASK_RX_WAKEUP_DETECT_Msk  0x400UL
#define CXPI_CH_INTR_MASK_RX_FIFO_TRIGGER_Pos   11UL
#define CXPI_CH_INTR_MASK_RX_FIFO_TRIGGER_Msk   0x800UL
#define CXPI_CH_INTR_MASK_RX_HEADER_PID_DONE_Pos 12UL
#define CXPI_CH_INTR_MASK_RX_HEADER_PID_DONE_Msk 0x1000UL
#define CXPI_CH_INTR_MASK_TXRX_COMPLETE_Pos     13UL
#define CXPI_CH_INTR_MASK_TXRX_COMPLETE_Msk     0x2000UL
#define CXPI_CH_INTR_MASK_TIMEOUT_Pos           18UL
#define CXPI_CH_INTR_MASK_TIMEOUT_Msk           0x40000UL
#define CXPI_CH_INTR_MASK_TX_HEADER_ARB_LOST_Pos 19UL
#define CXPI_CH_INTR_MASK_TX_HEADER_ARB_LOST_Msk 0x80000UL
#define CXPI_CH_INTR_MASK_TX_BIT_ERROR_Pos      20UL
#define CXPI_CH_INTR_MASK_TX_BIT_ERROR_Msk      0x100000UL
#define CXPI_CH_INTR_MASK_RX_CRC_ERROR_Pos      21UL
#define CXPI_CH_INTR_MASK_RX_CRC_ERROR_Msk      0x200000UL
#define CXPI_CH_INTR_MASK_RX_HEADER_PARITY_ERROR_Pos 22UL
#define CXPI_CH_INTR_MASK_RX_HEADER_PARITY_ERROR_Msk 0x400000UL
#define CXPI_CH_INTR_MASK_RX_DATA_LENGTH_ERROR_Pos 23UL
#define CXPI_CH_INTR_MASK_RX_DATA_LENGTH_ERROR_Msk 0x800000UL
#define CXPI_CH_INTR_MASK_TX_DATA_LENGTH_ERROR_Pos 24UL
#define CXPI_CH_INTR_MASK_TX_DATA_LENGTH_ERROR_Msk 0x1000000UL
#define CXPI_CH_INTR_MASK_RX_OVERFLOW_ERROR_Pos 25UL
#define CXPI_CH_INTR_MASK_RX_OVERFLOW_ERROR_Msk 0x2000000UL
#define CXPI_CH_INTR_MASK_TX_OVERFLOW_ERROR_Pos 26UL
#define CXPI_CH_INTR_MASK_TX_OVERFLOW_ERROR_Msk 0x4000000UL
#define CXPI_CH_INTR_MASK_RX_UNDERFLOW_ERROR_Pos 27UL
#define CXPI_CH_INTR_MASK_RX_UNDERFLOW_ERROR_Msk 0x8000000UL
#define CXPI_CH_INTR_MASK_TX_UNDERFLOW_ERROR_Pos 28UL
#define CXPI_CH_INTR_MASK_TX_UNDERFLOW_ERROR_Msk 0x10000000UL
#define CXPI_CH_INTR_MASK_RX_FRAME_ERROR_Pos    29UL
#define CXPI_CH_INTR_MASK_RX_FRAME_ERROR_Msk    0x20000000UL
#define CXPI_CH_INTR_MASK_TX_FRAME_ERROR_Pos    30UL
#define CXPI_CH_INTR_MASK_TX_FRAME_ERROR_Msk    0x40000000UL
/* CXPI_CH.INTR_MASKED */
#define CXPI_CH_INTR_MASKED_TX_HEADER_DONE_Pos  0UL
#define CXPI_CH_INTR_MASKED_TX_HEADER_DONE_Msk  0x1UL
#define CXPI_CH_INTR_MASKED_TX_RESPONSE_DONE_Pos 1UL
#define CXPI_CH_INTR_MASKED_TX_RESPONSE_DONE_Msk 0x2UL
#define CXPI_CH_INTR_MASKED_TX_WAKEUP_DONE_Pos  3UL
#define CXPI_CH_INTR_MASKED_TX_WAKEUP_DONE_Msk  0x8UL
#define CXPI_CH_INTR_MASKED_TX_FIFO_TRIGGER_Pos 4UL
#define CXPI_CH_INTR_MASKED_TX_FIFO_TRIGGER_Msk 0x10UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_DONE_Pos  8UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_DONE_Msk  0x100UL
#define CXPI_CH_INTR_MASKED_RX_RESPONSE_DONE_Pos 9UL
#define CXPI_CH_INTR_MASKED_RX_RESPONSE_DONE_Msk 0x200UL
#define CXPI_CH_INTR_MASKED_RX_WAKEUP_DETECT_Pos 10UL
#define CXPI_CH_INTR_MASKED_RX_WAKEUP_DETECT_Msk 0x400UL
#define CXPI_CH_INTR_MASKED_RX_FIFO_TRIGGER_Pos 11UL
#define CXPI_CH_INTR_MASKED_RX_FIFO_TRIGGER_Msk 0x800UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_PID_DONE_Pos 12UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_PID_DONE_Msk 0x1000UL
#define CXPI_CH_INTR_MASKED_TXRX_COMPLETE_Pos   13UL
#define CXPI_CH_INTR_MASKED_TXRX_COMPLETE_Msk   0x2000UL
#define CXPI_CH_INTR_MASKED_TIMEOUT_Pos         18UL
#define CXPI_CH_INTR_MASKED_TIMEOUT_Msk         0x40000UL
#define CXPI_CH_INTR_MASKED_TX_HEADER_ARB_LOST_Pos 19UL
#define CXPI_CH_INTR_MASKED_TX_HEADER_ARB_LOST_Msk 0x80000UL
#define CXPI_CH_INTR_MASKED_TX_BIT_ERROR_Pos    20UL
#define CXPI_CH_INTR_MASKED_TX_BIT_ERROR_Msk    0x100000UL
#define CXPI_CH_INTR_MASKED_RX_CRC_ERROR_Pos    21UL
#define CXPI_CH_INTR_MASKED_RX_CRC_ERROR_Msk    0x200000UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_PARITY_ERROR_Pos 22UL
#define CXPI_CH_INTR_MASKED_RX_HEADER_PARITY_ERROR_Msk 0x400000UL
#define CXPI_CH_INTR_MASKED_RX_DATA_LENGTH_ERROR_Pos 23UL
#define CXPI_CH_INTR_MASKED_RX_DATA_LENGTH_ERROR_Msk 0x800000UL
#define CXPI_CH_INTR_MASKED_TX_DATA_LENGTH_ERROR_Pos 24UL
#define CXPI_CH_INTR_MASKED_TX_DATA_LENGTH_ERROR_Msk 0x1000000UL
#define CXPI_CH_INTR_MASKED_RX_OVERFLOW_ERROR_Pos 25UL
#define CXPI_CH_INTR_MASKED_RX_OVERFLOW_ERROR_Msk 0x2000000UL
#define CXPI_CH_INTR_MASKED_TX_OVERFLOW_ERROR_Pos 26UL
#define CXPI_CH_INTR_MASKED_TX_OVERFLOW_ERROR_Msk 0x4000000UL
#define CXPI_CH_INTR_MASKED_RX_UNDERFLOW_ERROR_Pos 27UL
#define CXPI_CH_INTR_MASKED_RX_UNDERFLOW_ERROR_Msk 0x8000000UL
#define CXPI_CH_INTR_MASKED_TX_UNDERFLOW_ERROR_Pos 28UL
#define CXPI_CH_INTR_MASKED_TX_UNDERFLOW_ERROR_Msk 0x10000000UL
#define CXPI_CH_INTR_MASKED_RX_FRAME_ERROR_Pos  29UL
#define CXPI_CH_INTR_MASKED_RX_FRAME_ERROR_Msk  0x20000000UL
#define CXPI_CH_INTR_MASKED_TX_FRAME_ERROR_Pos  30UL
#define CXPI_CH_INTR_MASKED_TX_FRAME_ERROR_Msk  0x40000000UL


/* CXPI.ERROR_CTL */
#define CXPI_ERROR_CTL_CH_IDX_Pos               0UL
#define CXPI_ERROR_CTL_CH_IDX_Msk               0x1FUL
#define CXPI_ERROR_CTL_TX_CRC_ERROR_Pos         18UL
#define CXPI_ERROR_CTL_TX_CRC_ERROR_Msk         0x40000UL
#define CXPI_ERROR_CTL_TX_PID_PARITY_ERROR_Pos  19UL
#define CXPI_ERROR_CTL_TX_PID_PARITY_ERROR_Msk  0x80000UL
#define CXPI_ERROR_CTL_TX_DATA_LENGTH_ERROR_Pos 20UL
#define CXPI_ERROR_CTL_TX_DATA_LENGTH_ERROR_Msk 0x100000UL
#define CXPI_ERROR_CTL_TX_DATA_STOP_ERROR_Pos   25UL
#define CXPI_ERROR_CTL_TX_DATA_STOP_ERROR_Msk   0x2000000UL
#define CXPI_ERROR_CTL_ENABLED_Pos              31UL
#define CXPI_ERROR_CTL_ENABLED_Msk              0x80000000UL
/* CXPI.TEST_CTL */
#define CXPI_TEST_CTL_CH_IDX_Pos                0UL
#define CXPI_TEST_CTL_CH_IDX_Msk                0x1FUL
#define CXPI_TEST_CTL_MODE_Pos                  16UL
#define CXPI_TEST_CTL_MODE_Msk                  0x10000UL
#define CXPI_TEST_CTL_ENABLED_Pos               31UL
#define CXPI_TEST_CTL_ENABLED_Msk               0x80000000UL


#endif /* _CYIP_CXPI_H_ */


/* [] END OF FILE */
