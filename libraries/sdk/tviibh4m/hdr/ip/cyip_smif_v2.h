/***************************************************************************//**
* \file cyip_smif_v2.h
*
* \brief
* SMIF IP definitions
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

#ifndef _CYIP_SMIF_V2_H_
#define _CYIP_SMIF_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     SMIF
*******************************************************************************/

#define SMIF_DEVICE_SECTION_SIZE                0x00000080UL
#define SMIF_SECTION_SIZE                       0x00010000UL

/**
  * \brief Control (SMIF_DEVICE_CTL)
  */
typedef struct stc_SMIF_DEVICE_CTL_field {
  uint32_t                         u1WR_EN:1;
  uint32_t                         :3;
  uint32_t                         u1CRYPTO_EN:1;
  uint32_t                         :3;
  uint32_t                         u2DATA_SEL:2;
  uint32_t                         :2;
  uint32_t                         u3MERGE_TIMEOUT:3;
  uint32_t                         u1MERGE_EN:1;
  uint32_t                         u14TOTAL_TIMEOUT:14;
  uint32_t                         u1TOTAL_TIMEOUT_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_SMIF_DEVICE_CTL_field_t;

typedef union un_SMIF_DEVICE_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_CTL_t;

/**
  * \brief Device region base address (SMIF_DEVICE_ADDR)
  */
typedef struct stc_SMIF_DEVICE_ADDR_field {
  uint32_t                         :8;
  uint32_t                         u24ADDR:24;
} stc_SMIF_DEVICE_ADDR_field_t;

typedef union un_SMIF_DEVICE_ADDR {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_ADDR_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_ADDR_t;

/**
  * \brief Device region mask (SMIF_DEVICE_MASK)
  */
typedef struct stc_SMIF_DEVICE_MASK_field {
  uint32_t                         :8;
  uint32_t                         u24MASK:24;
} stc_SMIF_DEVICE_MASK_field_t;

typedef union un_SMIF_DEVICE_MASK {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_MASK_t;

/**
  * \brief Address control (SMIF_DEVICE_ADDR_CTL)
  */
typedef struct stc_SMIF_DEVICE_ADDR_CTL_field {
  uint32_t                         u3SIZE3:3;
  uint32_t                         :5;
  uint32_t                         u1DIV2:1;
  uint32_t                         :23;
} stc_SMIF_DEVICE_ADDR_CTL_field_t;

typedef union un_SMIF_DEVICE_ADDR_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_ADDR_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_ADDR_CTL_t;

/**
  * \brief Read status (SMIF_DEVICE_RD_STATUS)
  */
typedef struct stc_SMIF_DEVICE_RD_STATUS_field {
  uint32_t                         u8FS_STATUS:8;
  uint32_t                         :24;
} stc_SMIF_DEVICE_RD_STATUS_field_t;

typedef union un_SMIF_DEVICE_RD_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_STATUS_t;

/**
  * \brief Read command control (SMIF_DEVICE_RD_CMD_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_CMD_CTL_field {
  uint32_t                         u8CODE:8;
  uint32_t                         u8CODEH:8;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :11;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_RD_CMD_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_CMD_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_CMD_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_CMD_CTL_t;

/**
  * \brief Read address control (SMIF_DEVICE_RD_ADDR_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_ADDR_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :13;
} stc_SMIF_DEVICE_RD_ADDR_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_ADDR_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_ADDR_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_ADDR_CTL_t;

/**
  * \brief Read mode control (SMIF_DEVICE_RD_MODE_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_MODE_CTL_field {
  uint32_t                         u8CODE:8;
  uint32_t                         u8CODEH:8;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :11;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_RD_MODE_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_MODE_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_MODE_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_MODE_CTL_t;

/**
  * \brief Read dummy control (SMIF_DEVICE_RD_DUMMY_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_DUMMY_CTL_field {
  uint32_t                         u5SIZE5:5;
  uint32_t                         :25;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_RD_DUMMY_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_DUMMY_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_DUMMY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_DUMMY_CTL_t;

/**
  * \brief Read data control (SMIF_DEVICE_RD_DATA_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_DATA_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :13;
} stc_SMIF_DEVICE_RD_DATA_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_DATA_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_DATA_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_DATA_CTL_t;

/**
  * \brief Read Bus CRC control (SMIF_DEVICE_RD_CRC_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_CRC_CTL_field {
  uint32_t                         u8STATUS_CHECK_MASK:8;
  uint32_t                         u8STATUS_ERROR_POL:8;
  uint32_t                         u8DATA_CRC_INPUT_SIZE:8;
  uint32_t                         u2CMD_ADDR_CRC_WIDTH:2;
  uint32_t                         u1CMD_ADDR_CRC_DDR_MODE:1;
  uint32_t                         u1CMD_ADDR_CRC_INPUT:1;
  uint32_t                         u1DATA_CRC_CHECK:1;
  uint32_t                         :1;
  uint32_t                         u1CMD_ADDR_CRC_PRESENT:1;
  uint32_t                         u1DATA_CRC_PRESENT:1;
} stc_SMIF_DEVICE_RD_CRC_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_CRC_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_CRC_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_CRC_CTL_t;

/**
  * \brief Read boundary control (SMIF_DEVICE_RD_BOUND_CTL)
  */
typedef struct stc_SMIF_DEVICE_RD_BOUND_CTL_field {
  uint32_t                         u5SIZE5:5;
  uint32_t                         :11;
  uint32_t                         u2SUB_PAGE_SIZE:2;
  uint32_t                         :2;
  uint32_t                         u2SUB_PAGE_NR:2;
  uint32_t                         :6;
  uint32_t                         u1SUBSEQ_BOUND_EN:1;
  uint32_t                         :2;
  uint32_t                         u1PRESENT:1;
} stc_SMIF_DEVICE_RD_BOUND_CTL_field_t;

typedef union un_SMIF_DEVICE_RD_BOUND_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_RD_BOUND_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_RD_BOUND_CTL_t;

/**
  * \brief Write command control (SMIF_DEVICE_WR_CMD_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_CMD_CTL_field {
  uint32_t                         u8CODE:8;
  uint32_t                         u8CODEH:8;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :11;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_WR_CMD_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_CMD_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_CMD_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_CMD_CTL_t;

/**
  * \brief Write address control (SMIF_DEVICE_WR_ADDR_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_ADDR_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :13;
} stc_SMIF_DEVICE_WR_ADDR_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_ADDR_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_ADDR_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_ADDR_CTL_t;

/**
  * \brief Write mode control (SMIF_DEVICE_WR_MODE_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_MODE_CTL_field {
  uint32_t                         u8CODE:8;
  uint32_t                         u8CODEH:8;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :11;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_WR_MODE_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_MODE_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_MODE_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_MODE_CTL_t;

/**
  * \brief Write dummy control (SMIF_DEVICE_WR_DUMMY_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_DUMMY_CTL_field {
  uint32_t                         u5SIZE5:5;
  uint32_t                         :12;
  uint32_t                         u1RWDS_EN:1;
  uint32_t                         :12;
  uint32_t                         u2PRESENT2:2;
} stc_SMIF_DEVICE_WR_DUMMY_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_DUMMY_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_DUMMY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_DUMMY_CTL_t;

/**
  * \brief Write data control (SMIF_DEVICE_WR_DATA_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_DATA_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WIDTH:2;
  uint32_t                         u1DDR_MODE:1;
  uint32_t                         :13;
} stc_SMIF_DEVICE_WR_DATA_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_DATA_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_DATA_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_DATA_CTL_t;

/**
  * \brief Write Bus CRC control (SMIF_DEVICE_WR_CRC_CTL)
  */
typedef struct stc_SMIF_DEVICE_WR_CRC_CTL_field {
  uint32_t                         :16;
  uint32_t                         u8DATA_CRC_INPUT_SIZE:8;
  uint32_t                         u2CMD_ADDR_CRC_WIDTH:2;
  uint32_t                         u1CMD_ADDR_CRC_DDR_MODE:1;
  uint32_t                         u1CMD_ADDR_CRC_INPUT:1;
  uint32_t                         :2;
  uint32_t                         u1CMD_ADDR_CRC_PRESENT:1;
  uint32_t                         u1DATA_CRC_PRESENT:1;
} stc_SMIF_DEVICE_WR_CRC_CTL_field_t;

typedef union un_SMIF_DEVICE_WR_CRC_CTL {
  uint32_t                         u32Register;
  stc_SMIF_DEVICE_WR_CRC_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DEVICE_WR_CRC_CTL_t;



/**
  * \brief Control (SMIF_CTL)
  */
typedef struct stc_SMIF_CTL_field {
  uint32_t                         u1XIP_MODE:1;
  uint32_t                         :3;
  uint32_t                         u1CLOCK_IF_TX_SEL:1;
  uint32_t                         u3DELAY_LINE_SEL:3;
  uint32_t                         u1DELAY_TAP_ENABLED:1;
  uint32_t                         u1INT_CLOCK_DL_ENABLED:1;
  uint32_t                         u2INT_CLOCK_CAPTURE_CYCLE:2;
  uint32_t                         u3CLOCK_IF_RX_SEL:3;
  uint32_t                         :1;
  uint32_t                         u3DESELECT_DELAY:3;
  uint32_t                         :1;
  uint32_t                         u2SELECT_SETUP_DELAY:2;
  uint32_t                         u2SELECT_HOLD_DELAY:2;
  uint32_t                         u1BLOCK:1;
  uint32_t                         :6;
  uint32_t                         u1ENABLED:1;
} stc_SMIF_CTL_field_t;

typedef union un_SMIF_CTL {
  uint32_t                         u32Register;
  stc_SMIF_CTL_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CTL_t;

/**
  * \brief Status (SMIF_STATUS)
  */
typedef struct stc_SMIF_STATUS_field {
  uint32_t                         :31;
  uint32_t                         u1BUSY:1;
} stc_SMIF_STATUS_field_t;

typedef union un_SMIF_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_STATUS_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_STATUS_t;

/**
  * \brief Internal Clocking Delay Tap Select Register 0 (SMIF_INT_CLOCK_DELAY_TAP_SEL0)
  */
typedef struct stc_SMIF_INT_CLOCK_DELAY_TAP_SEL0_field {
  uint32_t                         u8DATA_BIT0:8;
  uint32_t                         u8DATA_BIT1:8;
  uint32_t                         u8DATA_BIT2:8;
  uint32_t                         u8DATA_BIT3:8;
} stc_SMIF_INT_CLOCK_DELAY_TAP_SEL0_field_t;

typedef union un_SMIF_INT_CLOCK_DELAY_TAP_SEL0 {
  uint32_t                         u32Register;
  stc_SMIF_INT_CLOCK_DELAY_TAP_SEL0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INT_CLOCK_DELAY_TAP_SEL0_t;

/**
  * \brief Internal Clocking Delay Tap Select Register 1 (SMIF_INT_CLOCK_DELAY_TAP_SEL1)
  */
typedef struct stc_SMIF_INT_CLOCK_DELAY_TAP_SEL1_field {
  uint32_t                         u8DATA_BIT4:8;
  uint32_t                         u8DATA_BIT5:8;
  uint32_t                         u8DATA_BIT6:8;
  uint32_t                         u8DATA_BIT7:8;
} stc_SMIF_INT_CLOCK_DELAY_TAP_SEL1_field_t;

typedef union un_SMIF_INT_CLOCK_DELAY_TAP_SEL1 {
  uint32_t                         u32Register;
  stc_SMIF_INT_CLOCK_DELAY_TAP_SEL1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INT_CLOCK_DELAY_TAP_SEL1_t;

/**
  * \brief Data Learning Pattern (SMIF_DLP)
  */
typedef struct stc_SMIF_DLP_field {
  uint32_t                         u8DLP:8;
  uint32_t                         :24;
} stc_SMIF_DLP_field_t;

typedef union un_SMIF_DLP {
  uint32_t                         u32Register;
  stc_SMIF_DLP_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DLP_t;

/**
  * \brief Data Learning Status Register 0 (SMIF_DL_STATUS0)
  */
typedef struct stc_SMIF_DL_STATUS0_field {
  uint32_t                         u8DATA_BIT0:8;
  uint32_t                         u8DATA_BIT1:8;
  uint32_t                         u8DATA_BIT2:8;
  uint32_t                         u8DATA_BIT3:8;
} stc_SMIF_DL_STATUS0_field_t;

typedef union un_SMIF_DL_STATUS0 {
  uint32_t                         u32Register;
  stc_SMIF_DL_STATUS0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DL_STATUS0_t;

/**
  * \brief Data Learning Status Register 1 (SMIF_DL_STATUS1)
  */
typedef struct stc_SMIF_DL_STATUS1_field {
  uint32_t                         u8DATA_BIT4:8;
  uint32_t                         u8DATA_BIT5:8;
  uint32_t                         u8DATA_BIT6:8;
  uint32_t                         u8DATA_BIT7:8;
} stc_SMIF_DL_STATUS1_field_t;

typedef union un_SMIF_DL_STATUS1 {
  uint32_t                         u32Register;
  stc_SMIF_DL_STATUS1_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DL_STATUS1_t;

/**
  * \brief Delay Tap Select Register (SMIF_DELAY_TAP_SEL)
  */
typedef struct stc_SMIF_DELAY_TAP_SEL_field {
  uint32_t                         u8SEL:8;
  uint32_t                         :24;
} stc_SMIF_DELAY_TAP_SEL_field_t;

typedef union un_SMIF_DELAY_TAP_SEL {
  uint32_t                         u32Register;
  stc_SMIF_DELAY_TAP_SEL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_DELAY_TAP_SEL_t;

/**
  * \brief Transmitter command FIFO status (SMIF_TX_CMD_FIFO_STATUS)
  */
typedef struct stc_SMIF_TX_CMD_FIFO_STATUS_field {
  uint32_t                         u4USED4:4;
  uint32_t                         :28;
} stc_SMIF_TX_CMD_FIFO_STATUS_field_t;

typedef union un_SMIF_TX_CMD_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_TX_CMD_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_CMD_FIFO_STATUS_t;

/**
  * \brief Transmitter command FIFO write (SMIF_TX_CMD_FIFO_WR)
  */
typedef struct stc_SMIF_TX_CMD_FIFO_WR_field {
  uint32_t                         u27DATA27:27;
  uint32_t                         :5;
} stc_SMIF_TX_CMD_FIFO_WR_field_t;

typedef union un_SMIF_TX_CMD_FIFO_WR {
  uint32_t                         u32Register;
  stc_SMIF_TX_CMD_FIFO_WR_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_CMD_FIFO_WR_t;

/**
  * \brief Transmitter data FIFO control (SMIF_TX_DATA_FIFO_CTL)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_CTL_field {
  uint32_t                         u3TX_TRIGGER_LEVEL:3;
  uint32_t                         :29;
} stc_SMIF_TX_DATA_FIFO_CTL_field_t;

typedef union un_SMIF_TX_DATA_FIFO_CTL {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_CTL_t;

/**
  * \brief Transmitter data FIFO status (SMIF_TX_DATA_FIFO_STATUS)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_STATUS_field {
  uint32_t                         u4USED4:4;
  uint32_t                         :28;
} stc_SMIF_TX_DATA_FIFO_STATUS_field_t;

typedef union un_SMIF_TX_DATA_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_STATUS_t;

/**
  * \brief Transmitter data FIFO write (SMIF_TX_DATA_FIFO_WR1)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_WR1_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         :24;
} stc_SMIF_TX_DATA_FIFO_WR1_field_t;

typedef union un_SMIF_TX_DATA_FIFO_WR1 {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_WR1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_WR1_t;

/**
  * \brief Transmitter data FIFO write (SMIF_TX_DATA_FIFO_WR2)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_WR2_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         u8DATA1:8;
  uint32_t                         :16;
} stc_SMIF_TX_DATA_FIFO_WR2_field_t;

typedef union un_SMIF_TX_DATA_FIFO_WR2 {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_WR2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_WR2_t;

/**
  * \brief Transmitter data FIFO write (SMIF_TX_DATA_FIFO_WR4)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_WR4_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         u8DATA1:8;
  uint32_t                         u8DATA2:8;
  uint32_t                         u8DATA3:8;
} stc_SMIF_TX_DATA_FIFO_WR4_field_t;

typedef union un_SMIF_TX_DATA_FIFO_WR4 {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_WR4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_WR4_t;

/**
  * \brief Transmitter data FIFO write (SMIF_TX_DATA_FIFO_WR1ODD)
  */
typedef struct stc_SMIF_TX_DATA_FIFO_WR1ODD_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         :24;
} stc_SMIF_TX_DATA_FIFO_WR1ODD_field_t;

typedef union un_SMIF_TX_DATA_FIFO_WR1ODD {
  uint32_t                         u32Register;
  stc_SMIF_TX_DATA_FIFO_WR1ODD_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_TX_DATA_FIFO_WR1ODD_t;

/**
  * \brief Receiver data MMIO FIFO control (SMIF_RX_DATA_MMIO_FIFO_CTL)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_CTL_field {
  uint32_t                         u3RX_TRIGGER_LEVEL:3;
  uint32_t                         :29;
} stc_SMIF_RX_DATA_MMIO_FIFO_CTL_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_CTL {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_CTL_t;

/**
  * \brief Receiver data MMIO FIFO status (SMIF_RX_DATA_MMIO_FIFO_STATUS)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_STATUS_field {
  uint32_t                         u4USED4:4;
  uint32_t                         :28;
} stc_SMIF_RX_DATA_MMIO_FIFO_STATUS_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_STATUS_t;

/**
  * \brief Receiver data FIFO status (SMIF_RX_DATA_FIFO_STATUS)
  */
typedef struct stc_SMIF_RX_DATA_FIFO_STATUS_field {
  uint32_t                         u5USED5:5;
  uint32_t                         :3;
  uint32_t                         u1RX_SR_USED:1;
  uint32_t                         :23;
} stc_SMIF_RX_DATA_FIFO_STATUS_field_t;

typedef union un_SMIF_RX_DATA_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_FIFO_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_FIFO_STATUS_t;

/**
  * \brief Receiver data MMIO FIFO read (SMIF_RX_DATA_MMIO_FIFO_RD1)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_RD1_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         :24;
} stc_SMIF_RX_DATA_MMIO_FIFO_RD1_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_RD1 {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_RD1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_RD1_t;

/**
  * \brief Receiver data MMIO FIFO read (SMIF_RX_DATA_MMIO_FIFO_RD2)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_RD2_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         u8DATA1:8;
  uint32_t                         :16;
} stc_SMIF_RX_DATA_MMIO_FIFO_RD2_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_RD2 {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_RD2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_RD2_t;

/**
  * \brief Receiver data MMIO FIFO read (SMIF_RX_DATA_MMIO_FIFO_RD4)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_RD4_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         u8DATA1:8;
  uint32_t                         u8DATA2:8;
  uint32_t                         u8DATA3:8;
} stc_SMIF_RX_DATA_MMIO_FIFO_RD4_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_RD4 {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_RD4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_RD4_t;

/**
  * \brief Receiver data MMIO FIFO silent read (SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT)
  */
typedef struct stc_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_field {
  uint32_t                         u8DATA0:8;
  uint32_t                         :24;
} stc_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_field_t;

typedef union un_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT {
  uint32_t                         u32Register;
  stc_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_t;

/**
  * \brief Slow cache control (SMIF_SLOW_CA_CTL)
  */
typedef struct stc_SMIF_SLOW_CA_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WAY:2;
  uint32_t                         :6;
  uint32_t                         u2SET_ADDR:2;
  uint32_t                         :4;
  uint32_t                         u1PREF_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_SMIF_SLOW_CA_CTL_field_t;

typedef union un_SMIF_SLOW_CA_CTL {
  uint32_t                         u32Register;
  stc_SMIF_SLOW_CA_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_SLOW_CA_CTL_t;

/**
  * \brief Slow cache command (SMIF_SLOW_CA_CMD)
  */
typedef struct stc_SMIF_SLOW_CA_CMD_field {
  uint32_t                         u1INV:1;
  uint32_t                         :31;
} stc_SMIF_SLOW_CA_CMD_field_t;

typedef union un_SMIF_SLOW_CA_CMD {
  uint32_t                         u32Register;
  stc_SMIF_SLOW_CA_CMD_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_SLOW_CA_CMD_t;

/**
  * \brief Fast cache control (SMIF_FAST_CA_CTL)
  */
typedef struct stc_SMIF_FAST_CA_CTL_field {
  uint32_t                         :16;
  uint32_t                         u2WAY:2;
  uint32_t                         :6;
  uint32_t                         u2SET_ADDR:2;
  uint32_t                         :4;
  uint32_t                         u1PREF_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_SMIF_FAST_CA_CTL_field_t;

typedef union un_SMIF_FAST_CA_CTL {
  uint32_t                         u32Register;
  stc_SMIF_FAST_CA_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_FAST_CA_CTL_t;

/**
  * \brief Fast cache command (SMIF_FAST_CA_CMD)
  */
typedef struct stc_SMIF_FAST_CA_CMD_field {
  uint32_t                         u1INV:1;
  uint32_t                         :31;
} stc_SMIF_FAST_CA_CMD_field_t;

typedef union un_SMIF_FAST_CA_CMD {
  uint32_t                         u32Register;
  stc_SMIF_FAST_CA_CMD_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_FAST_CA_CMD_t;

/**
  * \brief Cryptography Command (SMIF_CRYPTO_CMD)
  */
typedef struct stc_SMIF_CRYPTO_CMD_field {
  uint32_t                         u1START:1;
  uint32_t                         :31;
} stc_SMIF_CRYPTO_CMD_field_t;

typedef union un_SMIF_CRYPTO_CMD {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_CMD_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_CMD_t;

/**
  * \brief Cryptography input 0 (SMIF_CRYPTO_INPUT0)
  */
typedef struct stc_SMIF_CRYPTO_INPUT0_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRYPTO_INPUT0_field_t;

typedef union un_SMIF_CRYPTO_INPUT0 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_INPUT0_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_INPUT0_t;

/**
  * \brief Cryptography input 1 (SMIF_CRYPTO_INPUT1)
  */
typedef struct stc_SMIF_CRYPTO_INPUT1_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRYPTO_INPUT1_field_t;

typedef union un_SMIF_CRYPTO_INPUT1 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_INPUT1_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_INPUT1_t;

/**
  * \brief Cryptography input 2 (SMIF_CRYPTO_INPUT2)
  */
typedef struct stc_SMIF_CRYPTO_INPUT2_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRYPTO_INPUT2_field_t;

typedef union un_SMIF_CRYPTO_INPUT2 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_INPUT2_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_INPUT2_t;

/**
  * \brief Cryptography input 3 (SMIF_CRYPTO_INPUT3)
  */
typedef struct stc_SMIF_CRYPTO_INPUT3_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRYPTO_INPUT3_field_t;

typedef union un_SMIF_CRYPTO_INPUT3 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_INPUT3_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_INPUT3_t;

/**
  * \brief Cryptography key 0 (SMIF_CRYPTO_KEY0)
  */
typedef struct stc_SMIF_CRYPTO_KEY0_field {
  uint32_t                         u32KEY:32;
} stc_SMIF_CRYPTO_KEY0_field_t;

typedef union un_SMIF_CRYPTO_KEY0 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_KEY0_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_KEY0_t;

/**
  * \brief Cryptography key 1 (SMIF_CRYPTO_KEY1)
  */
typedef struct stc_SMIF_CRYPTO_KEY1_field {
  uint32_t                         u32KEY:32;
} stc_SMIF_CRYPTO_KEY1_field_t;

typedef union un_SMIF_CRYPTO_KEY1 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_KEY1_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_KEY1_t;

/**
  * \brief Cryptography key 2 (SMIF_CRYPTO_KEY2)
  */
typedef struct stc_SMIF_CRYPTO_KEY2_field {
  uint32_t                         u32KEY:32;
} stc_SMIF_CRYPTO_KEY2_field_t;

typedef union un_SMIF_CRYPTO_KEY2 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_KEY2_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_KEY2_t;

/**
  * \brief Cryptography key 3 (SMIF_CRYPTO_KEY3)
  */
typedef struct stc_SMIF_CRYPTO_KEY3_field {
  uint32_t                         u32KEY:32;
} stc_SMIF_CRYPTO_KEY3_field_t;

typedef union un_SMIF_CRYPTO_KEY3 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_KEY3_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_KEY3_t;

/**
  * \brief Cryptography output 0 (SMIF_CRYPTO_OUTPUT0)
  */
typedef struct stc_SMIF_CRYPTO_OUTPUT0_field {
  uint32_t                         u32OUTPUT:32;
} stc_SMIF_CRYPTO_OUTPUT0_field_t;

typedef union un_SMIF_CRYPTO_OUTPUT0 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_OUTPUT0_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_OUTPUT0_t;

/**
  * \brief Cryptography output 1 (SMIF_CRYPTO_OUTPUT1)
  */
typedef struct stc_SMIF_CRYPTO_OUTPUT1_field {
  uint32_t                         u32OUTPUT:32;
} stc_SMIF_CRYPTO_OUTPUT1_field_t;

typedef union un_SMIF_CRYPTO_OUTPUT1 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_OUTPUT1_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_OUTPUT1_t;

/**
  * \brief Cryptography output 2 (SMIF_CRYPTO_OUTPUT2)
  */
typedef struct stc_SMIF_CRYPTO_OUTPUT2_field {
  uint32_t                         u32OUTPUT:32;
} stc_SMIF_CRYPTO_OUTPUT2_field_t;

typedef union un_SMIF_CRYPTO_OUTPUT2 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_OUTPUT2_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_OUTPUT2_t;

/**
  * \brief Cryptography output 3 (SMIF_CRYPTO_OUTPUT3)
  */
typedef struct stc_SMIF_CRYPTO_OUTPUT3_field {
  uint32_t                         u32OUTPUT:32;
} stc_SMIF_CRYPTO_OUTPUT3_field_t;

typedef union un_SMIF_CRYPTO_OUTPUT3 {
  uint32_t                         u32Register;
  stc_SMIF_CRYPTO_OUTPUT3_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRYPTO_OUTPUT3_t;

/**
  * \brief CRC Command (SMIF_CRC_CMD)
  */
typedef struct stc_SMIF_CRC_CMD_field {
  uint32_t                         u1START:1;
  uint32_t                         u1CONTINUE:1;
  uint32_t                         :30;
} stc_SMIF_CRC_CMD_field_t;

typedef union un_SMIF_CRC_CMD {
  uint32_t                         u32Register;
  stc_SMIF_CRC_CMD_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRC_CMD_t;

/**
  * \brief CRC input 0 (SMIF_CRC_INPUT0)
  */
typedef struct stc_SMIF_CRC_INPUT0_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRC_INPUT0_field_t;

typedef union un_SMIF_CRC_INPUT0 {
  uint32_t                         u32Register;
  stc_SMIF_CRC_INPUT0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRC_INPUT0_t;

/**
  * \brief CRC input 1 (SMIF_CRC_INPUT1)
  */
typedef struct stc_SMIF_CRC_INPUT1_field {
  uint32_t                         u32INPUT:32;
} stc_SMIF_CRC_INPUT1_field_t;

typedef union un_SMIF_CRC_INPUT1 {
  uint32_t                         u32Register;
  stc_SMIF_CRC_INPUT1_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRC_INPUT1_t;

/**
  * \brief CRC output (SMIF_CRC_OUTPUT)
  */
typedef struct stc_SMIF_CRC_OUTPUT_field {
  uint32_t                         u8CRC_OUTPUT:8;
  uint32_t                         :24;
} stc_SMIF_CRC_OUTPUT_field_t;

typedef union un_SMIF_CRC_OUTPUT {
  uint32_t                         u32Register;
  stc_SMIF_CRC_OUTPUT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_CRC_OUTPUT_t;

/**
  * \brief Interrupt register (SMIF_INTR)
  */
typedef struct stc_SMIF_INTR_field {
  uint32_t                         u1TR_TX_REQ:1;
  uint32_t                         u1TR_RX_REQ:1;
  uint32_t                         u1XIP_ALIGNMENT_ERROR:1;
  uint32_t                         u1TX_CMD_FIFO_OVERFLOW:1;
  uint32_t                         u1TX_DATA_FIFO_OVERFLOW:1;
  uint32_t                         u1RX_DATA_MMIO_FIFO_UNDERFLOW:1;
  uint32_t                         :2;
  uint32_t                         u1DL_FAIL:1;
  uint32_t                         :3;
  uint32_t                         u1DL_WARNING:1;
  uint32_t                         :3;
  uint32_t                         u1CRC_ERROR:1;
  uint32_t                         u1FS_STATUS_ERROR:1;
  uint32_t                         :14;
} stc_SMIF_INTR_field_t;

typedef union un_SMIF_INTR {
  uint32_t                         u32Register;
  stc_SMIF_INTR_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INTR_t;

/**
  * \brief Interrupt set register (SMIF_INTR_SET)
  */
typedef struct stc_SMIF_INTR_SET_field {
  uint32_t                         u1TR_TX_REQ:1;
  uint32_t                         u1TR_RX_REQ:1;
  uint32_t                         u1XIP_ALIGNMENT_ERROR:1;
  uint32_t                         u1TX_CMD_FIFO_OVERFLOW:1;
  uint32_t                         u1TX_DATA_FIFO_OVERFLOW:1;
  uint32_t                         u1RX_DATA_MMIO_FIFO_UNDERFLOW:1;
  uint32_t                         :2;
  uint32_t                         u1DL_FAIL:1;
  uint32_t                         :3;
  uint32_t                         u1DL_WARNING:1;
  uint32_t                         :3;
  uint32_t                         u1CRC_ERROR:1;
  uint32_t                         u1FS_STATUS_ERROR:1;
  uint32_t                         :14;
} stc_SMIF_INTR_SET_field_t;

typedef union un_SMIF_INTR_SET {
  uint32_t                         u32Register;
  stc_SMIF_INTR_SET_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INTR_SET_t;

/**
  * \brief Interrupt mask register (SMIF_INTR_MASK)
  */
typedef struct stc_SMIF_INTR_MASK_field {
  uint32_t                         u1TR_TX_REQ:1;
  uint32_t                         u1TR_RX_REQ:1;
  uint32_t                         u1XIP_ALIGNMENT_ERROR:1;
  uint32_t                         u1TX_CMD_FIFO_OVERFLOW:1;
  uint32_t                         u1TX_DATA_FIFO_OVERFLOW:1;
  uint32_t                         u1RX_DATA_MMIO_FIFO_UNDERFLOW:1;
  uint32_t                         :2;
  uint32_t                         u1DL_FAIL:1;
  uint32_t                         :3;
  uint32_t                         u1DL_WARNING:1;
  uint32_t                         :3;
  uint32_t                         u1CRC_ERROR:1;
  uint32_t                         u1FS_STATUS_ERROR:1;
  uint32_t                         :14;
} stc_SMIF_INTR_MASK_field_t;

typedef union un_SMIF_INTR_MASK {
  uint32_t                         u32Register;
  stc_SMIF_INTR_MASK_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INTR_MASK_t;

/**
  * \brief Interrupt masked register (SMIF_INTR_MASKED)
  */
typedef struct stc_SMIF_INTR_MASKED_field {
  uint32_t                         u1TR_TX_REQ:1;
  uint32_t                         u1TR_RX_REQ:1;
  uint32_t                         u1XIP_ALIGNMENT_ERROR:1;
  uint32_t                         u1TX_CMD_FIFO_OVERFLOW:1;
  uint32_t                         u1TX_DATA_FIFO_OVERFLOW:1;
  uint32_t                         u1RX_DATA_MMIO_FIFO_UNDERFLOW:1;
  uint32_t                         :2;
  uint32_t                         u1DL_FAIL:1;
  uint32_t                         :3;
  uint32_t                         u1DL_WARNING:1;
  uint32_t                         :3;
  uint32_t                         u1CRC_ERROR:1;
  uint32_t                         u1FS_STATUS_ERROR:1;
  uint32_t                         :14;
} stc_SMIF_INTR_MASKED_field_t;

typedef union un_SMIF_INTR_MASKED {
  uint32_t                         u32Register;
  stc_SMIF_INTR_MASKED_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMIF_INTR_MASKED_t;



/**
  * \brief Device (only used in XIP mode) (SMIF_DEVICE)
  */
typedef struct stc_SMIF_DEVICE {
  un_SMIF_DEVICE_CTL_t             unCTL;              /*!< 0x00000000 Control */
  uint32_t                         u32Reserved;
  un_SMIF_DEVICE_ADDR_t            unADDR;             /*!< 0x00000008 Device region base address */
  un_SMIF_DEVICE_MASK_t            unMASK;             /*!< 0x0000000C Device region mask */
  uint32_t                         au32Reserved1[4];
  un_SMIF_DEVICE_ADDR_CTL_t        unADDR_CTL;         /*!< 0x00000020 Address control */
  uint32_t                         au32Reserved2[3];
  un_SMIF_DEVICE_RD_STATUS_t       unRD_STATUS;        /*!< 0x00000030 Read status */
  uint32_t                         au32Reserved3[3];
  un_SMIF_DEVICE_RD_CMD_CTL_t      unRD_CMD_CTL;       /*!< 0x00000040 Read command control */
  un_SMIF_DEVICE_RD_ADDR_CTL_t     unRD_ADDR_CTL;      /*!< 0x00000044 Read address control */
  un_SMIF_DEVICE_RD_MODE_CTL_t     unRD_MODE_CTL;      /*!< 0x00000048 Read mode control */
  un_SMIF_DEVICE_RD_DUMMY_CTL_t    unRD_DUMMY_CTL;     /*!< 0x0000004C Read dummy control */
  un_SMIF_DEVICE_RD_DATA_CTL_t     unRD_DATA_CTL;      /*!< 0x00000050 Read data control */
  un_SMIF_DEVICE_RD_CRC_CTL_t      unRD_CRC_CTL;       /*!< 0x00000054 Read Bus CRC control */
  un_SMIF_DEVICE_RD_BOUND_CTL_t    unRD_BOUND_CTL;     /*!< 0x00000058 Read boundary control */
  uint32_t                         u32Reserved4;
  un_SMIF_DEVICE_WR_CMD_CTL_t      unWR_CMD_CTL;       /*!< 0x00000060 Write command control */
  un_SMIF_DEVICE_WR_ADDR_CTL_t     unWR_ADDR_CTL;      /*!< 0x00000064 Write address control */
  un_SMIF_DEVICE_WR_MODE_CTL_t     unWR_MODE_CTL;      /*!< 0x00000068 Write mode control */
  un_SMIF_DEVICE_WR_DUMMY_CTL_t    unWR_DUMMY_CTL;     /*!< 0x0000006C Write dummy control */
  un_SMIF_DEVICE_WR_DATA_CTL_t     unWR_DATA_CTL;      /*!< 0x00000070 Write data control */
  un_SMIF_DEVICE_WR_CRC_CTL_t      unWR_CRC_CTL;       /*!< 0x00000074 Write Bus CRC control */
  uint32_t                         au32Reserved5[2];
} stc_SMIF_DEVICE_t;                                   /*!< Size = 128 (0x80) */

/**
  * \brief Serial Memory Interface (SMIF)
  */
typedef struct stc_SMIF {
  un_SMIF_CTL_t                    unCTL;              /*!< 0x00000000 Control */
  un_SMIF_STATUS_t                 unSTATUS;           /*!< 0x00000004 Status */
  uint32_t                         au32Reserved[2];
  un_SMIF_INT_CLOCK_DELAY_TAP_SEL0_t unINT_CLOCK_DELAY_TAP_SEL0; /*!< 0x00000010 Internal Clocking Delay Tap Select Register 0 */
  un_SMIF_INT_CLOCK_DELAY_TAP_SEL1_t unINT_CLOCK_DELAY_TAP_SEL1; /*!< 0x00000014 Internal Clocking Delay Tap Select Register 1 */
  un_SMIF_DLP_t                    unDLP;              /*!< 0x00000018 Data Learning Pattern */
  uint32_t                         u32Reserved1;
  un_SMIF_DL_STATUS0_t             unDL_STATUS0;       /*!< 0x00000020 Data Learning Status Register 0 */
  un_SMIF_DL_STATUS1_t             unDL_STATUS1;       /*!< 0x00000024 Data Learning Status Register 1 */
  uint32_t                         au32Reserved2[2];
  un_SMIF_DELAY_TAP_SEL_t          unDELAY_TAP_SEL;    /*!< 0x00000030 Delay Tap Select Register */
  uint32_t                         au32Reserved3[4];
  un_SMIF_TX_CMD_FIFO_STATUS_t     unTX_CMD_FIFO_STATUS; /*!< 0x00000044 Transmitter command FIFO status */
  uint32_t                         au32Reserved4[2];
  un_SMIF_TX_CMD_FIFO_WR_t         unTX_CMD_FIFO_WR;   /*!< 0x00000050 Transmitter command FIFO write */
  uint32_t                         au32Reserved5[11];
  un_SMIF_TX_DATA_FIFO_CTL_t       unTX_DATA_FIFO_CTL; /*!< 0x00000080 Transmitter data FIFO control */
  un_SMIF_TX_DATA_FIFO_STATUS_t    unTX_DATA_FIFO_STATUS; /*!< 0x00000084 Transmitter data FIFO status */
  uint32_t                         au32Reserved6[2];
  un_SMIF_TX_DATA_FIFO_WR1_t       unTX_DATA_FIFO_WR1; /*!< 0x00000090 Transmitter data FIFO write */
  un_SMIF_TX_DATA_FIFO_WR2_t       unTX_DATA_FIFO_WR2; /*!< 0x00000094 Transmitter data FIFO write */
  un_SMIF_TX_DATA_FIFO_WR4_t       unTX_DATA_FIFO_WR4; /*!< 0x00000098 Transmitter data FIFO write */
  un_SMIF_TX_DATA_FIFO_WR1ODD_t    unTX_DATA_FIFO_WR1ODD; /*!< 0x0000009C Transmitter data FIFO write */
  uint32_t                         au32Reserved7[8];
  un_SMIF_RX_DATA_MMIO_FIFO_CTL_t  unRX_DATA_MMIO_FIFO_CTL; /*!< 0x000000C0 Receiver data MMIO FIFO control */
  un_SMIF_RX_DATA_MMIO_FIFO_STATUS_t unRX_DATA_MMIO_FIFO_STATUS; /*!< 0x000000C4 Receiver data MMIO FIFO status */
  un_SMIF_RX_DATA_FIFO_STATUS_t    unRX_DATA_FIFO_STATUS; /*!< 0x000000C8 Receiver data FIFO status */
  uint32_t                         u32Reserved8;
  un_SMIF_RX_DATA_MMIO_FIFO_RD1_t  unRX_DATA_MMIO_FIFO_RD1; /*!< 0x000000D0 Receiver data MMIO FIFO read */
  un_SMIF_RX_DATA_MMIO_FIFO_RD2_t  unRX_DATA_MMIO_FIFO_RD2; /*!< 0x000000D4 Receiver data MMIO FIFO read */
  un_SMIF_RX_DATA_MMIO_FIFO_RD4_t  unRX_DATA_MMIO_FIFO_RD4; /*!< 0x000000D8 Receiver data MMIO FIFO read */
  uint32_t                         u32Reserved9;
  un_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_t unRX_DATA_MMIO_FIFO_RD1_SILENT; /*!< 0x000000E0 Receiver data MMIO FIFO silent read */
  uint32_t                         au32Reserved10[7];
  un_SMIF_SLOW_CA_CTL_t            unSLOW_CA_CTL;      /*!< 0x00000100 Slow cache control */
  uint32_t                         u32Reserved11;
  un_SMIF_SLOW_CA_CMD_t            unSLOW_CA_CMD;      /*!< 0x00000108 Slow cache command */
  uint32_t                         au32Reserved12[29];
  un_SMIF_FAST_CA_CTL_t            unFAST_CA_CTL;      /*!< 0x00000180 Fast cache control */
  uint32_t                         u32Reserved13;
  un_SMIF_FAST_CA_CMD_t            unFAST_CA_CMD;      /*!< 0x00000188 Fast cache command */
  uint32_t                         au32Reserved14[29];
  un_SMIF_CRYPTO_CMD_t             unCRYPTO_CMD;       /*!< 0x00000200 Cryptography Command */
  uint32_t                         au32Reserved15[7];
  un_SMIF_CRYPTO_INPUT0_t          unCRYPTO_INPUT0;    /*!< 0x00000220 Cryptography input 0 */
  un_SMIF_CRYPTO_INPUT1_t          unCRYPTO_INPUT1;    /*!< 0x00000224 Cryptography input 1 */
  un_SMIF_CRYPTO_INPUT2_t          unCRYPTO_INPUT2;    /*!< 0x00000228 Cryptography input 2 */
  un_SMIF_CRYPTO_INPUT3_t          unCRYPTO_INPUT3;    /*!< 0x0000022C Cryptography input 3 */
  uint32_t                         au32Reserved16[4];
  un_SMIF_CRYPTO_KEY0_t            unCRYPTO_KEY0;      /*!< 0x00000240 Cryptography key 0 */
  un_SMIF_CRYPTO_KEY1_t            unCRYPTO_KEY1;      /*!< 0x00000244 Cryptography key 1 */
  un_SMIF_CRYPTO_KEY2_t            unCRYPTO_KEY2;      /*!< 0x00000248 Cryptography key 2 */
  un_SMIF_CRYPTO_KEY3_t            unCRYPTO_KEY3;      /*!< 0x0000024C Cryptography key 3 */
  uint32_t                         au32Reserved17[4];
  un_SMIF_CRYPTO_OUTPUT0_t         unCRYPTO_OUTPUT0;   /*!< 0x00000260 Cryptography output 0 */
  un_SMIF_CRYPTO_OUTPUT1_t         unCRYPTO_OUTPUT1;   /*!< 0x00000264 Cryptography output 1 */
  un_SMIF_CRYPTO_OUTPUT2_t         unCRYPTO_OUTPUT2;   /*!< 0x00000268 Cryptography output 2 */
  un_SMIF_CRYPTO_OUTPUT3_t         unCRYPTO_OUTPUT3;   /*!< 0x0000026C Cryptography output 3 */
  uint32_t                         au32Reserved18[36];
  un_SMIF_CRC_CMD_t                unCRC_CMD;          /*!< 0x00000300 CRC Command */
  uint32_t                         au32Reserved19[7];
  un_SMIF_CRC_INPUT0_t             unCRC_INPUT0;       /*!< 0x00000320 CRC input 0 */
  un_SMIF_CRC_INPUT1_t             unCRC_INPUT1;       /*!< 0x00000324 CRC input 1 */
  uint32_t                         au32Reserved20[6];
  un_SMIF_CRC_OUTPUT_t             unCRC_OUTPUT;       /*!< 0x00000340 CRC output */
  uint32_t                         au32Reserved21[287];
  un_SMIF_INTR_t                   unINTR;             /*!< 0x000007C0 Interrupt register */
  un_SMIF_INTR_SET_t               unINTR_SET;         /*!< 0x000007C4 Interrupt set register */
  un_SMIF_INTR_MASK_t              unINTR_MASK;        /*!< 0x000007C8 Interrupt mask register */
  un_SMIF_INTR_MASKED_t            unINTR_MASKED;      /*!< 0x000007CC Interrupt masked register */
  uint32_t                         au32Reserved22[12];
  stc_SMIF_DEVICE_t                DEVICE[4];          /*!< 0x00000800 Device (only used in XIP mode) */
} stc_SMIF_t;                                          /*!< Size = 2560 (0xA00) */


/* SMIF_DEVICE.CTL */
#define SMIF_DEVICE_CTL_WR_EN_Pos               0UL
#define SMIF_DEVICE_CTL_WR_EN_Msk               0x1UL
#define SMIF_DEVICE_CTL_CRYPTO_EN_Pos           4UL
#define SMIF_DEVICE_CTL_CRYPTO_EN_Msk           0x10UL
#define SMIF_DEVICE_CTL_DATA_SEL_Pos            8UL
#define SMIF_DEVICE_CTL_DATA_SEL_Msk            0x300UL
#define SMIF_DEVICE_CTL_MERGE_TIMEOUT_Pos       12UL
#define SMIF_DEVICE_CTL_MERGE_TIMEOUT_Msk       0x7000UL
#define SMIF_DEVICE_CTL_MERGE_EN_Pos            15UL
#define SMIF_DEVICE_CTL_MERGE_EN_Msk            0x8000UL
#define SMIF_DEVICE_CTL_TOTAL_TIMEOUT_Pos       16UL
#define SMIF_DEVICE_CTL_TOTAL_TIMEOUT_Msk       0x3FFF0000UL
#define SMIF_DEVICE_CTL_TOTAL_TIMEOUT_EN_Pos    30UL
#define SMIF_DEVICE_CTL_TOTAL_TIMEOUT_EN_Msk    0x40000000UL
#define SMIF_DEVICE_CTL_ENABLED_Pos             31UL
#define SMIF_DEVICE_CTL_ENABLED_Msk             0x80000000UL
/* SMIF_DEVICE.ADDR */
#define SMIF_DEVICE_ADDR_ADDR_Pos               8UL
#define SMIF_DEVICE_ADDR_ADDR_Msk               0xFFFFFF00UL
/* SMIF_DEVICE.MASK */
#define SMIF_DEVICE_MASK_MASK_Pos               8UL
#define SMIF_DEVICE_MASK_MASK_Msk               0xFFFFFF00UL
/* SMIF_DEVICE.ADDR_CTL */
#define SMIF_DEVICE_ADDR_CTL_SIZE3_Pos          0UL
#define SMIF_DEVICE_ADDR_CTL_SIZE3_Msk          0x7UL
#define SMIF_DEVICE_ADDR_CTL_DIV2_Pos           8UL
#define SMIF_DEVICE_ADDR_CTL_DIV2_Msk           0x100UL
/* SMIF_DEVICE.RD_STATUS */
#define SMIF_DEVICE_RD_STATUS_FS_STATUS_Pos     0UL
#define SMIF_DEVICE_RD_STATUS_FS_STATUS_Msk     0xFFUL
/* SMIF_DEVICE.RD_CMD_CTL */
#define SMIF_DEVICE_RD_CMD_CTL_CODE_Pos         0UL
#define SMIF_DEVICE_RD_CMD_CTL_CODE_Msk         0xFFUL
#define SMIF_DEVICE_RD_CMD_CTL_CODEH_Pos        8UL
#define SMIF_DEVICE_RD_CMD_CTL_CODEH_Msk        0xFF00UL
#define SMIF_DEVICE_RD_CMD_CTL_WIDTH_Pos        16UL
#define SMIF_DEVICE_RD_CMD_CTL_WIDTH_Msk        0x30000UL
#define SMIF_DEVICE_RD_CMD_CTL_DDR_MODE_Pos     18UL
#define SMIF_DEVICE_RD_CMD_CTL_DDR_MODE_Msk     0x40000UL
#define SMIF_DEVICE_RD_CMD_CTL_PRESENT2_Pos     30UL
#define SMIF_DEVICE_RD_CMD_CTL_PRESENT2_Msk     0xC0000000UL
/* SMIF_DEVICE.RD_ADDR_CTL */
#define SMIF_DEVICE_RD_ADDR_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_RD_ADDR_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_RD_ADDR_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_RD_ADDR_CTL_DDR_MODE_Msk    0x40000UL
/* SMIF_DEVICE.RD_MODE_CTL */
#define SMIF_DEVICE_RD_MODE_CTL_CODE_Pos        0UL
#define SMIF_DEVICE_RD_MODE_CTL_CODE_Msk        0xFFUL
#define SMIF_DEVICE_RD_MODE_CTL_CODEH_Pos       8UL
#define SMIF_DEVICE_RD_MODE_CTL_CODEH_Msk       0xFF00UL
#define SMIF_DEVICE_RD_MODE_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_RD_MODE_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_RD_MODE_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_RD_MODE_CTL_DDR_MODE_Msk    0x40000UL
#define SMIF_DEVICE_RD_MODE_CTL_PRESENT2_Pos    30UL
#define SMIF_DEVICE_RD_MODE_CTL_PRESENT2_Msk    0xC0000000UL
/* SMIF_DEVICE.RD_DUMMY_CTL */
#define SMIF_DEVICE_RD_DUMMY_CTL_SIZE5_Pos      0UL
#define SMIF_DEVICE_RD_DUMMY_CTL_SIZE5_Msk      0x1FUL
#define SMIF_DEVICE_RD_DUMMY_CTL_PRESENT2_Pos   30UL
#define SMIF_DEVICE_RD_DUMMY_CTL_PRESENT2_Msk   0xC0000000UL
/* SMIF_DEVICE.RD_DATA_CTL */
#define SMIF_DEVICE_RD_DATA_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_RD_DATA_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_RD_DATA_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_RD_DATA_CTL_DDR_MODE_Msk    0x40000UL
/* SMIF_DEVICE.RD_CRC_CTL */
#define SMIF_DEVICE_RD_CRC_CTL_STATUS_CHECK_MASK_Pos 0UL
#define SMIF_DEVICE_RD_CRC_CTL_STATUS_CHECK_MASK_Msk 0xFFUL
#define SMIF_DEVICE_RD_CRC_CTL_STATUS_ERROR_POL_Pos 8UL
#define SMIF_DEVICE_RD_CRC_CTL_STATUS_ERROR_POL_Msk 0xFF00UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_INPUT_SIZE_Pos 16UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_INPUT_SIZE_Msk 0xFF0000UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_WIDTH_Pos 24UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_WIDTH_Msk 0x3000000UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_DDR_MODE_Pos 26UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_DDR_MODE_Msk 0x4000000UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_INPUT_Pos 27UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_INPUT_Msk 0x8000000UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_CHECK_Pos 28UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_CHECK_Msk 0x10000000UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_PRESENT_Pos 30UL
#define SMIF_DEVICE_RD_CRC_CTL_CMD_ADDR_CRC_PRESENT_Msk 0x40000000UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_PRESENT_Pos 31UL
#define SMIF_DEVICE_RD_CRC_CTL_DATA_CRC_PRESENT_Msk 0x80000000UL
/* SMIF_DEVICE.RD_BOUND_CTL */
#define SMIF_DEVICE_RD_BOUND_CTL_SIZE5_Pos      0UL
#define SMIF_DEVICE_RD_BOUND_CTL_SIZE5_Msk      0x1FUL
#define SMIF_DEVICE_RD_BOUND_CTL_SUB_PAGE_SIZE_Pos 16UL
#define SMIF_DEVICE_RD_BOUND_CTL_SUB_PAGE_SIZE_Msk 0x30000UL
#define SMIF_DEVICE_RD_BOUND_CTL_SUB_PAGE_NR_Pos 20UL
#define SMIF_DEVICE_RD_BOUND_CTL_SUB_PAGE_NR_Msk 0x300000UL
#define SMIF_DEVICE_RD_BOUND_CTL_SUBSEQ_BOUND_EN_Pos 28UL
#define SMIF_DEVICE_RD_BOUND_CTL_SUBSEQ_BOUND_EN_Msk 0x10000000UL
#define SMIF_DEVICE_RD_BOUND_CTL_PRESENT_Pos    31UL
#define SMIF_DEVICE_RD_BOUND_CTL_PRESENT_Msk    0x80000000UL
/* SMIF_DEVICE.WR_CMD_CTL */
#define SMIF_DEVICE_WR_CMD_CTL_CODE_Pos         0UL
#define SMIF_DEVICE_WR_CMD_CTL_CODE_Msk         0xFFUL
#define SMIF_DEVICE_WR_CMD_CTL_CODEH_Pos        8UL
#define SMIF_DEVICE_WR_CMD_CTL_CODEH_Msk        0xFF00UL
#define SMIF_DEVICE_WR_CMD_CTL_WIDTH_Pos        16UL
#define SMIF_DEVICE_WR_CMD_CTL_WIDTH_Msk        0x30000UL
#define SMIF_DEVICE_WR_CMD_CTL_DDR_MODE_Pos     18UL
#define SMIF_DEVICE_WR_CMD_CTL_DDR_MODE_Msk     0x40000UL
#define SMIF_DEVICE_WR_CMD_CTL_PRESENT2_Pos     30UL
#define SMIF_DEVICE_WR_CMD_CTL_PRESENT2_Msk     0xC0000000UL
/* SMIF_DEVICE.WR_ADDR_CTL */
#define SMIF_DEVICE_WR_ADDR_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_WR_ADDR_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_WR_ADDR_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_WR_ADDR_CTL_DDR_MODE_Msk    0x40000UL
/* SMIF_DEVICE.WR_MODE_CTL */
#define SMIF_DEVICE_WR_MODE_CTL_CODE_Pos        0UL
#define SMIF_DEVICE_WR_MODE_CTL_CODE_Msk        0xFFUL
#define SMIF_DEVICE_WR_MODE_CTL_CODEH_Pos       8UL
#define SMIF_DEVICE_WR_MODE_CTL_CODEH_Msk       0xFF00UL
#define SMIF_DEVICE_WR_MODE_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_WR_MODE_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_WR_MODE_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_WR_MODE_CTL_DDR_MODE_Msk    0x40000UL
#define SMIF_DEVICE_WR_MODE_CTL_PRESENT2_Pos    30UL
#define SMIF_DEVICE_WR_MODE_CTL_PRESENT2_Msk    0xC0000000UL
/* SMIF_DEVICE.WR_DUMMY_CTL */
#define SMIF_DEVICE_WR_DUMMY_CTL_SIZE5_Pos      0UL
#define SMIF_DEVICE_WR_DUMMY_CTL_SIZE5_Msk      0x1FUL
#define SMIF_DEVICE_WR_DUMMY_CTL_RWDS_EN_Pos    17UL
#define SMIF_DEVICE_WR_DUMMY_CTL_RWDS_EN_Msk    0x20000UL
#define SMIF_DEVICE_WR_DUMMY_CTL_PRESENT2_Pos   30UL
#define SMIF_DEVICE_WR_DUMMY_CTL_PRESENT2_Msk   0xC0000000UL
/* SMIF_DEVICE.WR_DATA_CTL */
#define SMIF_DEVICE_WR_DATA_CTL_WIDTH_Pos       16UL
#define SMIF_DEVICE_WR_DATA_CTL_WIDTH_Msk       0x30000UL
#define SMIF_DEVICE_WR_DATA_CTL_DDR_MODE_Pos    18UL
#define SMIF_DEVICE_WR_DATA_CTL_DDR_MODE_Msk    0x40000UL
/* SMIF_DEVICE.WR_CRC_CTL */
#define SMIF_DEVICE_WR_CRC_CTL_DATA_CRC_INPUT_SIZE_Pos 16UL
#define SMIF_DEVICE_WR_CRC_CTL_DATA_CRC_INPUT_SIZE_Msk 0xFF0000UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_WIDTH_Pos 24UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_WIDTH_Msk 0x3000000UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_DDR_MODE_Pos 26UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_DDR_MODE_Msk 0x4000000UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_INPUT_Pos 27UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_INPUT_Msk 0x8000000UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_PRESENT_Pos 30UL
#define SMIF_DEVICE_WR_CRC_CTL_CMD_ADDR_CRC_PRESENT_Msk 0x40000000UL
#define SMIF_DEVICE_WR_CRC_CTL_DATA_CRC_PRESENT_Pos 31UL
#define SMIF_DEVICE_WR_CRC_CTL_DATA_CRC_PRESENT_Msk 0x80000000UL


/* SMIF.CTL */
#define SMIF_CTL_XIP_MODE_Pos                   0UL
#define SMIF_CTL_XIP_MODE_Msk                   0x1UL
#define SMIF_CTL_CLOCK_IF_TX_SEL_Pos            4UL
#define SMIF_CTL_CLOCK_IF_TX_SEL_Msk            0x10UL
#define SMIF_CTL_DELAY_LINE_SEL_Pos             5UL
#define SMIF_CTL_DELAY_LINE_SEL_Msk             0xE0UL
#define SMIF_CTL_DELAY_TAP_ENABLED_Pos          8UL
#define SMIF_CTL_DELAY_TAP_ENABLED_Msk          0x100UL
#define SMIF_CTL_INT_CLOCK_DL_ENABLED_Pos       9UL
#define SMIF_CTL_INT_CLOCK_DL_ENABLED_Msk       0x200UL
#define SMIF_CTL_INT_CLOCK_CAPTURE_CYCLE_Pos    10UL
#define SMIF_CTL_INT_CLOCK_CAPTURE_CYCLE_Msk    0xC00UL
#define SMIF_CTL_CLOCK_IF_RX_SEL_Pos            12UL
#define SMIF_CTL_CLOCK_IF_RX_SEL_Msk            0x7000UL
#define SMIF_CTL_DESELECT_DELAY_Pos             16UL
#define SMIF_CTL_DESELECT_DELAY_Msk             0x70000UL
#define SMIF_CTL_SELECT_SETUP_DELAY_Pos         20UL
#define SMIF_CTL_SELECT_SETUP_DELAY_Msk         0x300000UL
#define SMIF_CTL_SELECT_HOLD_DELAY_Pos          22UL
#define SMIF_CTL_SELECT_HOLD_DELAY_Msk          0xC00000UL
#define SMIF_CTL_BLOCK_Pos                      24UL
#define SMIF_CTL_BLOCK_Msk                      0x1000000UL
#define SMIF_CTL_ENABLED_Pos                    31UL
#define SMIF_CTL_ENABLED_Msk                    0x80000000UL
/* SMIF.STATUS */
#define SMIF_STATUS_BUSY_Pos                    31UL
#define SMIF_STATUS_BUSY_Msk                    0x80000000UL
/* SMIF.INT_CLOCK_DELAY_TAP_SEL0 */
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT0_Pos 0UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT0_Msk 0xFFUL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT1_Pos 8UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT1_Msk 0xFF00UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT2_Pos 16UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT2_Msk 0xFF0000UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT3_Pos 24UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL0_DATA_BIT3_Msk 0xFF000000UL
/* SMIF.INT_CLOCK_DELAY_TAP_SEL1 */
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT4_Pos 0UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT4_Msk 0xFFUL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT5_Pos 8UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT5_Msk 0xFF00UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT6_Pos 16UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT6_Msk 0xFF0000UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT7_Pos 24UL
#define SMIF_INT_CLOCK_DELAY_TAP_SEL1_DATA_BIT7_Msk 0xFF000000UL
/* SMIF.DLP */
#define SMIF_DLP_DLP_Pos                        0UL
#define SMIF_DLP_DLP_Msk                        0xFFUL
/* SMIF.DL_STATUS0 */
#define SMIF_DL_STATUS0_DATA_BIT0_Pos           0UL
#define SMIF_DL_STATUS0_DATA_BIT0_Msk           0xFFUL
#define SMIF_DL_STATUS0_DATA_BIT1_Pos           8UL
#define SMIF_DL_STATUS0_DATA_BIT1_Msk           0xFF00UL
#define SMIF_DL_STATUS0_DATA_BIT2_Pos           16UL
#define SMIF_DL_STATUS0_DATA_BIT2_Msk           0xFF0000UL
#define SMIF_DL_STATUS0_DATA_BIT3_Pos           24UL
#define SMIF_DL_STATUS0_DATA_BIT3_Msk           0xFF000000UL
/* SMIF.DL_STATUS1 */
#define SMIF_DL_STATUS1_DATA_BIT4_Pos           0UL
#define SMIF_DL_STATUS1_DATA_BIT4_Msk           0xFFUL
#define SMIF_DL_STATUS1_DATA_BIT5_Pos           8UL
#define SMIF_DL_STATUS1_DATA_BIT5_Msk           0xFF00UL
#define SMIF_DL_STATUS1_DATA_BIT6_Pos           16UL
#define SMIF_DL_STATUS1_DATA_BIT6_Msk           0xFF0000UL
#define SMIF_DL_STATUS1_DATA_BIT7_Pos           24UL
#define SMIF_DL_STATUS1_DATA_BIT7_Msk           0xFF000000UL
/* SMIF.DELAY_TAP_SEL */
#define SMIF_DELAY_TAP_SEL_SEL_Pos              0UL
#define SMIF_DELAY_TAP_SEL_SEL_Msk              0xFFUL
/* SMIF.TX_CMD_FIFO_STATUS */
#define SMIF_TX_CMD_FIFO_STATUS_USED4_Pos       0UL
#define SMIF_TX_CMD_FIFO_STATUS_USED4_Msk       0xFUL
/* SMIF.TX_CMD_FIFO_WR */
#define SMIF_TX_CMD_FIFO_WR_DATA27_Pos          0UL
#define SMIF_TX_CMD_FIFO_WR_DATA27_Msk          0x7FFFFFFUL
/* SMIF.TX_DATA_FIFO_CTL */
#define SMIF_TX_DATA_FIFO_CTL_TX_TRIGGER_LEVEL_Pos 0UL
#define SMIF_TX_DATA_FIFO_CTL_TX_TRIGGER_LEVEL_Msk 0x7UL
/* SMIF.TX_DATA_FIFO_STATUS */
#define SMIF_TX_DATA_FIFO_STATUS_USED4_Pos      0UL
#define SMIF_TX_DATA_FIFO_STATUS_USED4_Msk      0xFUL
/* SMIF.TX_DATA_FIFO_WR1 */
#define SMIF_TX_DATA_FIFO_WR1_DATA0_Pos         0UL
#define SMIF_TX_DATA_FIFO_WR1_DATA0_Msk         0xFFUL
/* SMIF.TX_DATA_FIFO_WR2 */
#define SMIF_TX_DATA_FIFO_WR2_DATA0_Pos         0UL
#define SMIF_TX_DATA_FIFO_WR2_DATA0_Msk         0xFFUL
#define SMIF_TX_DATA_FIFO_WR2_DATA1_Pos         8UL
#define SMIF_TX_DATA_FIFO_WR2_DATA1_Msk         0xFF00UL
/* SMIF.TX_DATA_FIFO_WR4 */
#define SMIF_TX_DATA_FIFO_WR4_DATA0_Pos         0UL
#define SMIF_TX_DATA_FIFO_WR4_DATA0_Msk         0xFFUL
#define SMIF_TX_DATA_FIFO_WR4_DATA1_Pos         8UL
#define SMIF_TX_DATA_FIFO_WR4_DATA1_Msk         0xFF00UL
#define SMIF_TX_DATA_FIFO_WR4_DATA2_Pos         16UL
#define SMIF_TX_DATA_FIFO_WR4_DATA2_Msk         0xFF0000UL
#define SMIF_TX_DATA_FIFO_WR4_DATA3_Pos         24UL
#define SMIF_TX_DATA_FIFO_WR4_DATA3_Msk         0xFF000000UL
/* SMIF.TX_DATA_FIFO_WR1ODD */
#define SMIF_TX_DATA_FIFO_WR1ODD_DATA0_Pos      0UL
#define SMIF_TX_DATA_FIFO_WR1ODD_DATA0_Msk      0xFFUL
/* SMIF.RX_DATA_MMIO_FIFO_CTL */
#define SMIF_RX_DATA_MMIO_FIFO_CTL_RX_TRIGGER_LEVEL_Pos 0UL
#define SMIF_RX_DATA_MMIO_FIFO_CTL_RX_TRIGGER_LEVEL_Msk 0x7UL
/* SMIF.RX_DATA_MMIO_FIFO_STATUS */
#define SMIF_RX_DATA_MMIO_FIFO_STATUS_USED4_Pos 0UL
#define SMIF_RX_DATA_MMIO_FIFO_STATUS_USED4_Msk 0xFUL
/* SMIF.RX_DATA_FIFO_STATUS */
#define SMIF_RX_DATA_FIFO_STATUS_USED5_Pos      0UL
#define SMIF_RX_DATA_FIFO_STATUS_USED5_Msk      0x1FUL
#define SMIF_RX_DATA_FIFO_STATUS_RX_SR_USED_Pos 8UL
#define SMIF_RX_DATA_FIFO_STATUS_RX_SR_USED_Msk 0x100UL
/* SMIF.RX_DATA_MMIO_FIFO_RD1 */
#define SMIF_RX_DATA_MMIO_FIFO_RD1_DATA0_Pos    0UL
#define SMIF_RX_DATA_MMIO_FIFO_RD1_DATA0_Msk    0xFFUL
/* SMIF.RX_DATA_MMIO_FIFO_RD2 */
#define SMIF_RX_DATA_MMIO_FIFO_RD2_DATA0_Pos    0UL
#define SMIF_RX_DATA_MMIO_FIFO_RD2_DATA0_Msk    0xFFUL
#define SMIF_RX_DATA_MMIO_FIFO_RD2_DATA1_Pos    8UL
#define SMIF_RX_DATA_MMIO_FIFO_RD2_DATA1_Msk    0xFF00UL
/* SMIF.RX_DATA_MMIO_FIFO_RD4 */
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA0_Pos    0UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA0_Msk    0xFFUL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA1_Pos    8UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA1_Msk    0xFF00UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA2_Pos    16UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA2_Msk    0xFF0000UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA3_Pos    24UL
#define SMIF_RX_DATA_MMIO_FIFO_RD4_DATA3_Msk    0xFF000000UL
/* SMIF.RX_DATA_MMIO_FIFO_RD1_SILENT */
#define SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_DATA0_Pos 0UL
#define SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_DATA0_Msk 0xFFUL
/* SMIF.SLOW_CA_CTL */
#define SMIF_SLOW_CA_CTL_WAY_Pos                16UL
#define SMIF_SLOW_CA_CTL_WAY_Msk                0x30000UL
#define SMIF_SLOW_CA_CTL_SET_ADDR_Pos           24UL
#define SMIF_SLOW_CA_CTL_SET_ADDR_Msk           0x3000000UL
#define SMIF_SLOW_CA_CTL_PREF_EN_Pos            30UL
#define SMIF_SLOW_CA_CTL_PREF_EN_Msk            0x40000000UL
#define SMIF_SLOW_CA_CTL_ENABLED_Pos            31UL
#define SMIF_SLOW_CA_CTL_ENABLED_Msk            0x80000000UL
/* SMIF.SLOW_CA_CMD */
#define SMIF_SLOW_CA_CMD_INV_Pos                0UL
#define SMIF_SLOW_CA_CMD_INV_Msk                0x1UL
/* SMIF.FAST_CA_CTL */
#define SMIF_FAST_CA_CTL_WAY_Pos                16UL
#define SMIF_FAST_CA_CTL_WAY_Msk                0x30000UL
#define SMIF_FAST_CA_CTL_SET_ADDR_Pos           24UL
#define SMIF_FAST_CA_CTL_SET_ADDR_Msk           0x3000000UL
#define SMIF_FAST_CA_CTL_PREF_EN_Pos            30UL
#define SMIF_FAST_CA_CTL_PREF_EN_Msk            0x40000000UL
#define SMIF_FAST_CA_CTL_ENABLED_Pos            31UL
#define SMIF_FAST_CA_CTL_ENABLED_Msk            0x80000000UL
/* SMIF.FAST_CA_CMD */
#define SMIF_FAST_CA_CMD_INV_Pos                0UL
#define SMIF_FAST_CA_CMD_INV_Msk                0x1UL
/* SMIF.CRYPTO_CMD */
#define SMIF_CRYPTO_CMD_START_Pos               0UL
#define SMIF_CRYPTO_CMD_START_Msk               0x1UL
/* SMIF.CRYPTO_INPUT0 */
#define SMIF_CRYPTO_INPUT0_INPUT_Pos            0UL
#define SMIF_CRYPTO_INPUT0_INPUT_Msk            0xFFFFFFFFUL
/* SMIF.CRYPTO_INPUT1 */
#define SMIF_CRYPTO_INPUT1_INPUT_Pos            0UL
#define SMIF_CRYPTO_INPUT1_INPUT_Msk            0xFFFFFFFFUL
/* SMIF.CRYPTO_INPUT2 */
#define SMIF_CRYPTO_INPUT2_INPUT_Pos            0UL
#define SMIF_CRYPTO_INPUT2_INPUT_Msk            0xFFFFFFFFUL
/* SMIF.CRYPTO_INPUT3 */
#define SMIF_CRYPTO_INPUT3_INPUT_Pos            0UL
#define SMIF_CRYPTO_INPUT3_INPUT_Msk            0xFFFFFFFFUL
/* SMIF.CRYPTO_KEY0 */
#define SMIF_CRYPTO_KEY0_KEY_Pos                0UL
#define SMIF_CRYPTO_KEY0_KEY_Msk                0xFFFFFFFFUL
/* SMIF.CRYPTO_KEY1 */
#define SMIF_CRYPTO_KEY1_KEY_Pos                0UL
#define SMIF_CRYPTO_KEY1_KEY_Msk                0xFFFFFFFFUL
/* SMIF.CRYPTO_KEY2 */
#define SMIF_CRYPTO_KEY2_KEY_Pos                0UL
#define SMIF_CRYPTO_KEY2_KEY_Msk                0xFFFFFFFFUL
/* SMIF.CRYPTO_KEY3 */
#define SMIF_CRYPTO_KEY3_KEY_Pos                0UL
#define SMIF_CRYPTO_KEY3_KEY_Msk                0xFFFFFFFFUL
/* SMIF.CRYPTO_OUTPUT0 */
#define SMIF_CRYPTO_OUTPUT0_OUTPUT_Pos          0UL
#define SMIF_CRYPTO_OUTPUT0_OUTPUT_Msk          0xFFFFFFFFUL
/* SMIF.CRYPTO_OUTPUT1 */
#define SMIF_CRYPTO_OUTPUT1_OUTPUT_Pos          0UL
#define SMIF_CRYPTO_OUTPUT1_OUTPUT_Msk          0xFFFFFFFFUL
/* SMIF.CRYPTO_OUTPUT2 */
#define SMIF_CRYPTO_OUTPUT2_OUTPUT_Pos          0UL
#define SMIF_CRYPTO_OUTPUT2_OUTPUT_Msk          0xFFFFFFFFUL
/* SMIF.CRYPTO_OUTPUT3 */
#define SMIF_CRYPTO_OUTPUT3_OUTPUT_Pos          0UL
#define SMIF_CRYPTO_OUTPUT3_OUTPUT_Msk          0xFFFFFFFFUL
/* SMIF.CRC_CMD */
#define SMIF_CRC_CMD_START_Pos                  0UL
#define SMIF_CRC_CMD_START_Msk                  0x1UL
#define SMIF_CRC_CMD_CONTINUE_Pos               1UL
#define SMIF_CRC_CMD_CONTINUE_Msk               0x2UL
/* SMIF.CRC_INPUT0 */
#define SMIF_CRC_INPUT0_INPUT_Pos               0UL
#define SMIF_CRC_INPUT0_INPUT_Msk               0xFFFFFFFFUL
/* SMIF.CRC_INPUT1 */
#define SMIF_CRC_INPUT1_INPUT_Pos               0UL
#define SMIF_CRC_INPUT1_INPUT_Msk               0xFFFFFFFFUL
/* SMIF.CRC_OUTPUT */
#define SMIF_CRC_OUTPUT_CRC_OUTPUT_Pos          0UL
#define SMIF_CRC_OUTPUT_CRC_OUTPUT_Msk          0xFFUL
/* SMIF.INTR */
#define SMIF_INTR_TR_TX_REQ_Pos                 0UL
#define SMIF_INTR_TR_TX_REQ_Msk                 0x1UL
#define SMIF_INTR_TR_RX_REQ_Pos                 1UL
#define SMIF_INTR_TR_RX_REQ_Msk                 0x2UL
#define SMIF_INTR_XIP_ALIGNMENT_ERROR_Pos       2UL
#define SMIF_INTR_XIP_ALIGNMENT_ERROR_Msk       0x4UL
#define SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Pos      3UL
#define SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Msk      0x8UL
#define SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Pos     4UL
#define SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Msk     0x10UL
#define SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Pos 5UL
#define SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk 0x20UL
#define SMIF_INTR_DL_FAIL_Pos                   8UL
#define SMIF_INTR_DL_FAIL_Msk                   0x100UL
#define SMIF_INTR_DL_WARNING_Pos                12UL
#define SMIF_INTR_DL_WARNING_Msk                0x1000UL
#define SMIF_INTR_CRC_ERROR_Pos                 16UL
#define SMIF_INTR_CRC_ERROR_Msk                 0x10000UL
#define SMIF_INTR_FS_STATUS_ERROR_Pos           17UL
#define SMIF_INTR_FS_STATUS_ERROR_Msk           0x20000UL
/* SMIF.INTR_SET */
#define SMIF_INTR_SET_TR_TX_REQ_Pos             0UL
#define SMIF_INTR_SET_TR_TX_REQ_Msk             0x1UL
#define SMIF_INTR_SET_TR_RX_REQ_Pos             1UL
#define SMIF_INTR_SET_TR_RX_REQ_Msk             0x2UL
#define SMIF_INTR_SET_XIP_ALIGNMENT_ERROR_Pos   2UL
#define SMIF_INTR_SET_XIP_ALIGNMENT_ERROR_Msk   0x4UL
#define SMIF_INTR_SET_TX_CMD_FIFO_OVERFLOW_Pos  3UL
#define SMIF_INTR_SET_TX_CMD_FIFO_OVERFLOW_Msk  0x8UL
#define SMIF_INTR_SET_TX_DATA_FIFO_OVERFLOW_Pos 4UL
#define SMIF_INTR_SET_TX_DATA_FIFO_OVERFLOW_Msk 0x10UL
#define SMIF_INTR_SET_RX_DATA_MMIO_FIFO_UNDERFLOW_Pos 5UL
#define SMIF_INTR_SET_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk 0x20UL
#define SMIF_INTR_SET_DL_FAIL_Pos               8UL
#define SMIF_INTR_SET_DL_FAIL_Msk               0x100UL
#define SMIF_INTR_SET_DL_WARNING_Pos            12UL
#define SMIF_INTR_SET_DL_WARNING_Msk            0x1000UL
#define SMIF_INTR_SET_CRC_ERROR_Pos             16UL
#define SMIF_INTR_SET_CRC_ERROR_Msk             0x10000UL
#define SMIF_INTR_SET_FS_STATUS_ERROR_Pos       17UL
#define SMIF_INTR_SET_FS_STATUS_ERROR_Msk       0x20000UL
/* SMIF.INTR_MASK */
#define SMIF_INTR_MASK_TR_TX_REQ_Pos            0UL
#define SMIF_INTR_MASK_TR_TX_REQ_Msk            0x1UL
#define SMIF_INTR_MASK_TR_RX_REQ_Pos            1UL
#define SMIF_INTR_MASK_TR_RX_REQ_Msk            0x2UL
#define SMIF_INTR_MASK_XIP_ALIGNMENT_ERROR_Pos  2UL
#define SMIF_INTR_MASK_XIP_ALIGNMENT_ERROR_Msk  0x4UL
#define SMIF_INTR_MASK_TX_CMD_FIFO_OVERFLOW_Pos 3UL
#define SMIF_INTR_MASK_TX_CMD_FIFO_OVERFLOW_Msk 0x8UL
#define SMIF_INTR_MASK_TX_DATA_FIFO_OVERFLOW_Pos 4UL
#define SMIF_INTR_MASK_TX_DATA_FIFO_OVERFLOW_Msk 0x10UL
#define SMIF_INTR_MASK_RX_DATA_MMIO_FIFO_UNDERFLOW_Pos 5UL
#define SMIF_INTR_MASK_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk 0x20UL
#define SMIF_INTR_MASK_DL_FAIL_Pos              8UL
#define SMIF_INTR_MASK_DL_FAIL_Msk              0x100UL
#define SMIF_INTR_MASK_DL_WARNING_Pos           12UL
#define SMIF_INTR_MASK_DL_WARNING_Msk           0x1000UL
#define SMIF_INTR_MASK_CRC_ERROR_Pos            16UL
#define SMIF_INTR_MASK_CRC_ERROR_Msk            0x10000UL
#define SMIF_INTR_MASK_FS_STATUS_ERROR_Pos      17UL
#define SMIF_INTR_MASK_FS_STATUS_ERROR_Msk      0x20000UL
/* SMIF.INTR_MASKED */
#define SMIF_INTR_MASKED_TR_TX_REQ_Pos          0UL
#define SMIF_INTR_MASKED_TR_TX_REQ_Msk          0x1UL
#define SMIF_INTR_MASKED_TR_RX_REQ_Pos          1UL
#define SMIF_INTR_MASKED_TR_RX_REQ_Msk          0x2UL
#define SMIF_INTR_MASKED_XIP_ALIGNMENT_ERROR_Pos 2UL
#define SMIF_INTR_MASKED_XIP_ALIGNMENT_ERROR_Msk 0x4UL
#define SMIF_INTR_MASKED_TX_CMD_FIFO_OVERFLOW_Pos 3UL
#define SMIF_INTR_MASKED_TX_CMD_FIFO_OVERFLOW_Msk 0x8UL
#define SMIF_INTR_MASKED_TX_DATA_FIFO_OVERFLOW_Pos 4UL
#define SMIF_INTR_MASKED_TX_DATA_FIFO_OVERFLOW_Msk 0x10UL
#define SMIF_INTR_MASKED_RX_DATA_MMIO_FIFO_UNDERFLOW_Pos 5UL
#define SMIF_INTR_MASKED_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk 0x20UL
#define SMIF_INTR_MASKED_DL_FAIL_Pos            8UL
#define SMIF_INTR_MASKED_DL_FAIL_Msk            0x100UL
#define SMIF_INTR_MASKED_DL_WARNING_Pos         12UL
#define SMIF_INTR_MASKED_DL_WARNING_Msk         0x1000UL
#define SMIF_INTR_MASKED_CRC_ERROR_Pos          16UL
#define SMIF_INTR_MASKED_CRC_ERROR_Msk          0x10000UL
#define SMIF_INTR_MASKED_FS_STATUS_ERROR_Pos    17UL
#define SMIF_INTR_MASKED_FS_STATUS_ERROR_Msk    0x20000UL


#endif /* _CYIP_SMIF_V2_H_ */


/* [] END OF FILE */
