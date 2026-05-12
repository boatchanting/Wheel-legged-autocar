/***************************************************************************//**
* \file cyip_lin.h
*
* \brief
* LIN IP definitions
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

#ifndef _CYIP_LIN_H_
#define _CYIP_LIN_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     LIN
*******************************************************************************/

#define LIN_CH_SECTION_SIZE                     0x00000100UL
#define LIN_SECTION_SIZE                        0x00010000UL

/**
  * \brief Control 0 (LIN_CH_CTL0)
  */
typedef struct stc_LIN_CH_CTL0_field {
  uint32_t                         u2STOP_BITS:2;
  uint32_t                         :2;
  uint32_t                         u1AUTO_EN:1;
  uint32_t                         :3;
  uint32_t                         u2BREAK_DELIMITER_LENGTH:2;
  uint32_t                         :6;
  uint32_t                         u5BREAK_WAKEUP_LENGTH:5;
  uint32_t                         :3;
  uint32_t                         u1MODE:1;
  uint32_t                         :2;
  uint32_t                         u1BIT_ERROR_IGNORE:1;
  uint32_t                         u1PARITY:1;
  uint32_t                         u1PARITY_EN:1;
  uint32_t                         u1FILTER_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_LIN_CH_CTL0_field_t;

typedef union un_LIN_CH_CTL0 {
  uint32_t                         u32Register;
  stc_LIN_CH_CTL0_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_CTL0_t;

/**
  * \brief Control 1 (LIN_CH_CTL1)
  */
typedef struct stc_LIN_CH_CTL1_field {
  uint32_t                         u3DATA_NR:3;
  uint32_t                         :5;
  uint32_t                         u1CHECKSUM_ENHANCED:1;
  uint32_t                         :7;
  uint32_t                         u8FRAME_TIMEOUT:8;
  uint32_t                         u2FRAME_TIMEOUT_SEL:2;
  uint32_t                         :6;
} stc_LIN_CH_CTL1_field_t;

typedef union un_LIN_CH_CTL1 {
  uint32_t                         u32Register;
  stc_LIN_CH_CTL1_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_CTL1_t;

/**
  * \brief Status (LIN_CH_STATUS)
  */
typedef struct stc_LIN_CH_STATUS_field {
  uint32_t                         u4DATA_IDX:4;
  uint32_t                         u1HEADER_RESPONSE:1;
  uint32_t                         u1RX_DATA0_FRAME_ERROR:1;
  uint32_t                         :2;
  uint32_t                         u1TX_BUSY:1;
  uint32_t                         u1RX_BUSY:1;
  uint32_t                         :2;
  uint32_t                         u1TX_DONE:1;
  uint32_t                         u1RX_DONE:1;
  uint32_t                         :2;
  uint32_t                         u1TX_HEADER_BIT_ERROR:1;
  uint32_t                         u1TX_RESPONSE_BIT_ERROR:1;
  uint32_t                         :6;
  uint32_t                         u1RX_HEADER_FRAME_ERROR:1;
  uint32_t                         u1RX_HEADER_SYNC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_RESPONSE_FRAME_ERROR:1;
  uint32_t                         u1RX_RESPONSE_CHECKSUM_ERROR:1;
  uint32_t                         :3;
} stc_LIN_CH_STATUS_field_t;

typedef union un_LIN_CH_STATUS {
  uint32_t                         u32Register;
  stc_LIN_CH_STATUS_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_STATUS_t;

/**
  * \brief Command (LIN_CH_CMD)
  */
typedef struct stc_LIN_CH_CMD_field {
  uint32_t                         u1TX_HEADER:1;
  uint32_t                         u1TX_RESPONSE:1;
  uint32_t                         u1TX_WAKEUP:1;
  uint32_t                         :5;
  uint32_t                         u1RX_HEADER:1;
  uint32_t                         u1RX_RESPONSE:1;
  uint32_t                         :22;
} stc_LIN_CH_CMD_field_t;

typedef union un_LIN_CH_CMD {
  uint32_t                         u32Register;
  stc_LIN_CH_CMD_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_CMD_t;

/**
  * \brief TX/RX status (LIN_CH_TX_RX_STATUS)
  */
typedef struct stc_LIN_CH_TX_RX_STATUS_field {
  uint32_t                         u8SYNC_COUNTER:8;
  uint32_t                         :8;
  uint32_t                         u1TX_IN:1;
  uint32_t                         u1RX_IN:1;
  uint32_t                         :6;
  uint32_t                         u1TX_OUT:1;
  uint32_t                         :1;
  uint32_t                         u1EN_OUT:1;
  uint32_t                         :5;
} stc_LIN_CH_TX_RX_STATUS_field_t;

typedef union un_LIN_CH_TX_RX_STATUS {
  uint32_t                         u32Register;
  stc_LIN_CH_TX_RX_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_TX_RX_STATUS_t;

/**
  * \brief PID and checksum (LIN_CH_PID_CHECKSUM)
  */
typedef struct stc_LIN_CH_PID_CHECKSUM_field {
  uint32_t                         u8PID:8;
  uint32_t                         u8CHECKSUM:8;
  uint32_t                         :16;
} stc_LIN_CH_PID_CHECKSUM_field_t;

typedef union un_LIN_CH_PID_CHECKSUM {
  uint32_t                         u32Register;
  stc_LIN_CH_PID_CHECKSUM_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_PID_CHECKSUM_t;

/**
  * \brief Response data 0 (LIN_CH_DATA0)
  */
typedef struct stc_LIN_CH_DATA0_field {
  uint32_t                         u8DATA1:8;
  uint32_t                         u8DATA2:8;
  uint32_t                         u8DATA3:8;
  uint32_t                         u8DATA4:8;
} stc_LIN_CH_DATA0_field_t;

typedef union un_LIN_CH_DATA0 {
  uint32_t                         u32Register;
  stc_LIN_CH_DATA0_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_DATA0_t;

/**
  * \brief Response data 1 (LIN_CH_DATA1)
  */
typedef struct stc_LIN_CH_DATA1_field {
  uint32_t                         u8DATA5:8;
  uint32_t                         u8DATA6:8;
  uint32_t                         u8DATA7:8;
  uint32_t                         u8DATA8:8;
} stc_LIN_CH_DATA1_field_t;

typedef union un_LIN_CH_DATA1 {
  uint32_t                         u32Register;
  stc_LIN_CH_DATA1_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_DATA1_t;

/**
  * \brief Interrupt (LIN_CH_INTR)
  */
typedef struct stc_LIN_CH_INTR_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         :5;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_BREAK_WAKEUP_DONE:1;
  uint32_t                         u1RX_HEADER_SYNC_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOISE_DETECT:1;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         :1;
  uint32_t                         u1TX_HEADER_BIT_ERROR:1;
  uint32_t                         u1TX_RESPONSE_BIT_ERROR:1;
  uint32_t                         :6;
  uint32_t                         u1RX_HEADER_FRAME_ERROR:1;
  uint32_t                         u1RX_HEADER_SYNC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_RESPONSE_FRAME_ERROR:1;
  uint32_t                         u1RX_RESPONSE_CHECKSUM_ERROR:1;
  uint32_t                         :3;
} stc_LIN_CH_INTR_field_t;

typedef union un_LIN_CH_INTR {
  uint32_t                         u32Register;
  stc_LIN_CH_INTR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_INTR_t;

/**
  * \brief Interrupt set (LIN_CH_INTR_SET)
  */
typedef struct stc_LIN_CH_INTR_SET_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         :5;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_BREAK_WAKEUP_DONE:1;
  uint32_t                         u1RX_HEADER_SYNC_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOISE_DETECT:1;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         :1;
  uint32_t                         u1TX_HEADER_BIT_ERROR:1;
  uint32_t                         u1TX_RESPONSE_BIT_ERROR:1;
  uint32_t                         :6;
  uint32_t                         u1RX_HEADER_FRAME_ERROR:1;
  uint32_t                         u1RX_HEADER_SYNC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_RESPONSE_FRAME_ERROR:1;
  uint32_t                         u1RX_RESPONSE_CHECKSUM_ERROR:1;
  uint32_t                         :3;
} stc_LIN_CH_INTR_SET_field_t;

typedef union un_LIN_CH_INTR_SET {
  uint32_t                         u32Register;
  stc_LIN_CH_INTR_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_INTR_SET_t;

/**
  * \brief Interrupt mask (LIN_CH_INTR_MASK)
  */
typedef struct stc_LIN_CH_INTR_MASK_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         :5;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_BREAK_WAKEUP_DONE:1;
  uint32_t                         u1RX_HEADER_SYNC_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOISE_DETECT:1;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         :1;
  uint32_t                         u1TX_HEADER_BIT_ERROR:1;
  uint32_t                         u1TX_RESPONSE_BIT_ERROR:1;
  uint32_t                         :6;
  uint32_t                         u1RX_HEADER_FRAME_ERROR:1;
  uint32_t                         u1RX_HEADER_SYNC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_RESPONSE_FRAME_ERROR:1;
  uint32_t                         u1RX_RESPONSE_CHECKSUM_ERROR:1;
  uint32_t                         :3;
} stc_LIN_CH_INTR_MASK_field_t;

typedef union un_LIN_CH_INTR_MASK {
  uint32_t                         u32Register;
  stc_LIN_CH_INTR_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_INTR_MASK_t;

/**
  * \brief Interrupt masked (LIN_CH_INTR_MASKED)
  */
typedef struct stc_LIN_CH_INTR_MASKED_field {
  uint32_t                         u1TX_HEADER_DONE:1;
  uint32_t                         u1TX_RESPONSE_DONE:1;
  uint32_t                         u1TX_WAKEUP_DONE:1;
  uint32_t                         :5;
  uint32_t                         u1RX_HEADER_DONE:1;
  uint32_t                         u1RX_RESPONSE_DONE:1;
  uint32_t                         u1RX_BREAK_WAKEUP_DONE:1;
  uint32_t                         u1RX_HEADER_SYNC_DONE:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOISE_DETECT:1;
  uint32_t                         u1TIMEOUT:1;
  uint32_t                         :1;
  uint32_t                         u1TX_HEADER_BIT_ERROR:1;
  uint32_t                         u1TX_RESPONSE_BIT_ERROR:1;
  uint32_t                         :6;
  uint32_t                         u1RX_HEADER_FRAME_ERROR:1;
  uint32_t                         u1RX_HEADER_SYNC_ERROR:1;
  uint32_t                         u1RX_HEADER_PARITY_ERROR:1;
  uint32_t                         u1RX_RESPONSE_FRAME_ERROR:1;
  uint32_t                         u1RX_RESPONSE_CHECKSUM_ERROR:1;
  uint32_t                         :3;
} stc_LIN_CH_INTR_MASKED_field_t;

typedef union un_LIN_CH_INTR_MASKED {
  uint32_t                         u32Register;
  stc_LIN_CH_INTR_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_CH_INTR_MASKED_t;



/**
  * \brief Error control (LIN_ERROR_CTL)
  */
typedef struct stc_LIN_ERROR_CTL_field {
  uint32_t                         u5CH_IDX:5;
  uint32_t                         :11;
  uint32_t                         u1TX_SYNC_ERROR:1;
  uint32_t                         u1TX_SYNC_STOP_ERROR:1;
  uint32_t                         u1TX_PARITY_ERROR:1;
  uint32_t                         u1TX_PID_STOP_ERROR:1;
  uint32_t                         :1;
  uint32_t                         u1TX_DATA_STOP_ERROR:1;
  uint32_t                         u1TX_CHECKSUM_ERROR:1;
  uint32_t                         u1TX_CHECKSUM_STOP_ERROR:1;
  uint32_t                         :7;
  uint32_t                         u1ENABLED:1;
} stc_LIN_ERROR_CTL_field_t;

typedef union un_LIN_ERROR_CTL {
  uint32_t                         u32Register;
  stc_LIN_ERROR_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_ERROR_CTL_t;

/**
  * \brief Test control (LIN_TEST_CTL)
  */
typedef struct stc_LIN_TEST_CTL_field {
  uint32_t                         u5CH_IDX:5;
  uint32_t                         :11;
  uint32_t                         u1MODE:1;
  uint32_t                         :14;
  uint32_t                         u1ENABLED:1;
} stc_LIN_TEST_CTL_field_t;

typedef union un_LIN_TEST_CTL {
  uint32_t                         u32Register;
  stc_LIN_TEST_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_LIN_TEST_CTL_t;



/**
  * \brief LIN channel structure (LIN_CH)
  */
typedef struct stc_LIN_CH {
  un_LIN_CH_CTL0_t                 unCTL0;             /*!< 0x00000000 Control 0 */
  un_LIN_CH_CTL1_t                 unCTL1;             /*!< 0x00000004 Control 1 */
  un_LIN_CH_STATUS_t               unSTATUS;           /*!< 0x00000008 Status */
  uint32_t                         u32Reserved;
  un_LIN_CH_CMD_t                  unCMD;              /*!< 0x00000010 Command */
  uint32_t                         au32Reserved1[19];
  un_LIN_CH_TX_RX_STATUS_t         unTX_RX_STATUS;     /*!< 0x00000060 TX/RX status */
  uint32_t                         au32Reserved2[7];
  un_LIN_CH_PID_CHECKSUM_t         unPID_CHECKSUM;     /*!< 0x00000080 PID and checksum */
  un_LIN_CH_DATA0_t                unDATA0;            /*!< 0x00000084 Response data 0 */
  un_LIN_CH_DATA1_t                unDATA1;            /*!< 0x00000088 Response data 1 */
  uint32_t                         au32Reserved3[13];
  un_LIN_CH_INTR_t                 unINTR;             /*!< 0x000000C0 Interrupt */
  un_LIN_CH_INTR_SET_t             unINTR_SET;         /*!< 0x000000C4 Interrupt set */
  un_LIN_CH_INTR_MASK_t            unINTR_MASK;        /*!< 0x000000C8 Interrupt mask */
  un_LIN_CH_INTR_MASKED_t          unINTR_MASKED;      /*!< 0x000000CC Interrupt masked */
  uint32_t                         au32Reserved4[12];
} stc_LIN_CH_t;                                        /*!< Size = 256 (0x100) */

/**
  * \brief LIN (LIN)
  */
typedef struct stc_LIN {
  un_LIN_ERROR_CTL_t               unERROR_CTL;        /*!< 0x00000000 Error control */
  un_LIN_TEST_CTL_t                unTEST_CTL;         /*!< 0x00000004 Test control */
  uint32_t                         au32Reserved[8190];
  stc_LIN_CH_t                     CH[32];             /*!< 0x00008000 LIN channel structure */
} stc_LIN_t;                                           /*!< Size = 40960 (0xA000) */


/* LIN_CH.CTL0 */
#define LIN_CH_CTL0_STOP_BITS_Pos               0UL
#define LIN_CH_CTL0_STOP_BITS_Msk               0x3UL
#define LIN_CH_CTL0_AUTO_EN_Pos                 4UL
#define LIN_CH_CTL0_AUTO_EN_Msk                 0x10UL
#define LIN_CH_CTL0_BREAK_DELIMITER_LENGTH_Pos  8UL
#define LIN_CH_CTL0_BREAK_DELIMITER_LENGTH_Msk  0x300UL
#define LIN_CH_CTL0_BREAK_WAKEUP_LENGTH_Pos     16UL
#define LIN_CH_CTL0_BREAK_WAKEUP_LENGTH_Msk     0x1F0000UL
#define LIN_CH_CTL0_MODE_Pos                    24UL
#define LIN_CH_CTL0_MODE_Msk                    0x1000000UL
#define LIN_CH_CTL0_BIT_ERROR_IGNORE_Pos        27UL
#define LIN_CH_CTL0_BIT_ERROR_IGNORE_Msk        0x8000000UL
#define LIN_CH_CTL0_PARITY_Pos                  28UL
#define LIN_CH_CTL0_PARITY_Msk                  0x10000000UL
#define LIN_CH_CTL0_PARITY_EN_Pos               29UL
#define LIN_CH_CTL0_PARITY_EN_Msk               0x20000000UL
#define LIN_CH_CTL0_FILTER_EN_Pos               30UL
#define LIN_CH_CTL0_FILTER_EN_Msk               0x40000000UL
#define LIN_CH_CTL0_ENABLED_Pos                 31UL
#define LIN_CH_CTL0_ENABLED_Msk                 0x80000000UL
/* LIN_CH.CTL1 */
#define LIN_CH_CTL1_DATA_NR_Pos                 0UL
#define LIN_CH_CTL1_DATA_NR_Msk                 0x7UL
#define LIN_CH_CTL1_CHECKSUM_ENHANCED_Pos       8UL
#define LIN_CH_CTL1_CHECKSUM_ENHANCED_Msk       0x100UL
#define LIN_CH_CTL1_FRAME_TIMEOUT_Pos           16UL
#define LIN_CH_CTL1_FRAME_TIMEOUT_Msk           0xFF0000UL
#define LIN_CH_CTL1_FRAME_TIMEOUT_SEL_Pos       24UL
#define LIN_CH_CTL1_FRAME_TIMEOUT_SEL_Msk       0x3000000UL
/* LIN_CH.STATUS */
#define LIN_CH_STATUS_DATA_IDX_Pos              0UL
#define LIN_CH_STATUS_DATA_IDX_Msk              0xFUL
#define LIN_CH_STATUS_HEADER_RESPONSE_Pos       4UL
#define LIN_CH_STATUS_HEADER_RESPONSE_Msk       0x10UL
#define LIN_CH_STATUS_RX_DATA0_FRAME_ERROR_Pos  5UL
#define LIN_CH_STATUS_RX_DATA0_FRAME_ERROR_Msk  0x20UL
#define LIN_CH_STATUS_TX_BUSY_Pos               8UL
#define LIN_CH_STATUS_TX_BUSY_Msk               0x100UL
#define LIN_CH_STATUS_RX_BUSY_Pos               9UL
#define LIN_CH_STATUS_RX_BUSY_Msk               0x200UL
#define LIN_CH_STATUS_TX_DONE_Pos               12UL
#define LIN_CH_STATUS_TX_DONE_Msk               0x1000UL
#define LIN_CH_STATUS_RX_DONE_Pos               13UL
#define LIN_CH_STATUS_RX_DONE_Msk               0x2000UL
#define LIN_CH_STATUS_TX_HEADER_BIT_ERROR_Pos   16UL
#define LIN_CH_STATUS_TX_HEADER_BIT_ERROR_Msk   0x10000UL
#define LIN_CH_STATUS_TX_RESPONSE_BIT_ERROR_Pos 17UL
#define LIN_CH_STATUS_TX_RESPONSE_BIT_ERROR_Msk 0x20000UL
#define LIN_CH_STATUS_RX_HEADER_FRAME_ERROR_Pos 24UL
#define LIN_CH_STATUS_RX_HEADER_FRAME_ERROR_Msk 0x1000000UL
#define LIN_CH_STATUS_RX_HEADER_SYNC_ERROR_Pos  25UL
#define LIN_CH_STATUS_RX_HEADER_SYNC_ERROR_Msk  0x2000000UL
#define LIN_CH_STATUS_RX_HEADER_PARITY_ERROR_Pos 26UL
#define LIN_CH_STATUS_RX_HEADER_PARITY_ERROR_Msk 0x4000000UL
#define LIN_CH_STATUS_RX_RESPONSE_FRAME_ERROR_Pos 27UL
#define LIN_CH_STATUS_RX_RESPONSE_FRAME_ERROR_Msk 0x8000000UL
#define LIN_CH_STATUS_RX_RESPONSE_CHECKSUM_ERROR_Pos 28UL
#define LIN_CH_STATUS_RX_RESPONSE_CHECKSUM_ERROR_Msk 0x10000000UL
/* LIN_CH.CMD */
#define LIN_CH_CMD_TX_HEADER_Pos                0UL
#define LIN_CH_CMD_TX_HEADER_Msk                0x1UL
#define LIN_CH_CMD_TX_RESPONSE_Pos              1UL
#define LIN_CH_CMD_TX_RESPONSE_Msk              0x2UL
#define LIN_CH_CMD_TX_WAKEUP_Pos                2UL
#define LIN_CH_CMD_TX_WAKEUP_Msk                0x4UL
#define LIN_CH_CMD_RX_HEADER_Pos                8UL
#define LIN_CH_CMD_RX_HEADER_Msk                0x100UL
#define LIN_CH_CMD_RX_RESPONSE_Pos              9UL
#define LIN_CH_CMD_RX_RESPONSE_Msk              0x200UL
/* LIN_CH.TX_RX_STATUS */
#define LIN_CH_TX_RX_STATUS_SYNC_COUNTER_Pos    0UL
#define LIN_CH_TX_RX_STATUS_SYNC_COUNTER_Msk    0xFFUL
#define LIN_CH_TX_RX_STATUS_TX_IN_Pos           16UL
#define LIN_CH_TX_RX_STATUS_TX_IN_Msk           0x10000UL
#define LIN_CH_TX_RX_STATUS_RX_IN_Pos           17UL
#define LIN_CH_TX_RX_STATUS_RX_IN_Msk           0x20000UL
#define LIN_CH_TX_RX_STATUS_TX_OUT_Pos          24UL
#define LIN_CH_TX_RX_STATUS_TX_OUT_Msk          0x1000000UL
#define LIN_CH_TX_RX_STATUS_EN_OUT_Pos          26UL
#define LIN_CH_TX_RX_STATUS_EN_OUT_Msk          0x4000000UL
/* LIN_CH.PID_CHECKSUM */
#define LIN_CH_PID_CHECKSUM_PID_Pos             0UL
#define LIN_CH_PID_CHECKSUM_PID_Msk             0xFFUL
#define LIN_CH_PID_CHECKSUM_CHECKSUM_Pos        8UL
#define LIN_CH_PID_CHECKSUM_CHECKSUM_Msk        0xFF00UL
/* LIN_CH.DATA0 */
#define LIN_CH_DATA0_DATA1_Pos                  0UL
#define LIN_CH_DATA0_DATA1_Msk                  0xFFUL
#define LIN_CH_DATA0_DATA2_Pos                  8UL
#define LIN_CH_DATA0_DATA2_Msk                  0xFF00UL
#define LIN_CH_DATA0_DATA3_Pos                  16UL
#define LIN_CH_DATA0_DATA3_Msk                  0xFF0000UL
#define LIN_CH_DATA0_DATA4_Pos                  24UL
#define LIN_CH_DATA0_DATA4_Msk                  0xFF000000UL
/* LIN_CH.DATA1 */
#define LIN_CH_DATA1_DATA5_Pos                  0UL
#define LIN_CH_DATA1_DATA5_Msk                  0xFFUL
#define LIN_CH_DATA1_DATA6_Pos                  8UL
#define LIN_CH_DATA1_DATA6_Msk                  0xFF00UL
#define LIN_CH_DATA1_DATA7_Pos                  16UL
#define LIN_CH_DATA1_DATA7_Msk                  0xFF0000UL
#define LIN_CH_DATA1_DATA8_Pos                  24UL
#define LIN_CH_DATA1_DATA8_Msk                  0xFF000000UL
/* LIN_CH.INTR */
#define LIN_CH_INTR_TX_HEADER_DONE_Pos          0UL
#define LIN_CH_INTR_TX_HEADER_DONE_Msk          0x1UL
#define LIN_CH_INTR_TX_RESPONSE_DONE_Pos        1UL
#define LIN_CH_INTR_TX_RESPONSE_DONE_Msk        0x2UL
#define LIN_CH_INTR_TX_WAKEUP_DONE_Pos          2UL
#define LIN_CH_INTR_TX_WAKEUP_DONE_Msk          0x4UL
#define LIN_CH_INTR_RX_HEADER_DONE_Pos          8UL
#define LIN_CH_INTR_RX_HEADER_DONE_Msk          0x100UL
#define LIN_CH_INTR_RX_RESPONSE_DONE_Pos        9UL
#define LIN_CH_INTR_RX_RESPONSE_DONE_Msk        0x200UL
#define LIN_CH_INTR_RX_BREAK_WAKEUP_DONE_Pos    10UL
#define LIN_CH_INTR_RX_BREAK_WAKEUP_DONE_Msk    0x400UL
#define LIN_CH_INTR_RX_HEADER_SYNC_DONE_Pos     11UL
#define LIN_CH_INTR_RX_HEADER_SYNC_DONE_Msk     0x800UL
#define LIN_CH_INTR_RX_NOISE_DETECT_Pos         13UL
#define LIN_CH_INTR_RX_NOISE_DETECT_Msk         0x2000UL
#define LIN_CH_INTR_TIMEOUT_Pos                 14UL
#define LIN_CH_INTR_TIMEOUT_Msk                 0x4000UL
#define LIN_CH_INTR_TX_HEADER_BIT_ERROR_Pos     16UL
#define LIN_CH_INTR_TX_HEADER_BIT_ERROR_Msk     0x10000UL
#define LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Pos   17UL
#define LIN_CH_INTR_TX_RESPONSE_BIT_ERROR_Msk   0x20000UL
#define LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Pos   24UL
#define LIN_CH_INTR_RX_HEADER_FRAME_ERROR_Msk   0x1000000UL
#define LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Pos    25UL
#define LIN_CH_INTR_RX_HEADER_SYNC_ERROR_Msk    0x2000000UL
#define LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Pos  26UL
#define LIN_CH_INTR_RX_HEADER_PARITY_ERROR_Msk  0x4000000UL
#define LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Pos 27UL
#define LIN_CH_INTR_RX_RESPONSE_FRAME_ERROR_Msk 0x8000000UL
#define LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Pos 28UL
#define LIN_CH_INTR_RX_RESPONSE_CHECKSUM_ERROR_Msk 0x10000000UL
/* LIN_CH.INTR_SET */
#define LIN_CH_INTR_SET_TX_HEADER_DONE_Pos      0UL
#define LIN_CH_INTR_SET_TX_HEADER_DONE_Msk      0x1UL
#define LIN_CH_INTR_SET_TX_RESPONSE_DONE_Pos    1UL
#define LIN_CH_INTR_SET_TX_RESPONSE_DONE_Msk    0x2UL
#define LIN_CH_INTR_SET_TX_WAKEUP_DONE_Pos      2UL
#define LIN_CH_INTR_SET_TX_WAKEUP_DONE_Msk      0x4UL
#define LIN_CH_INTR_SET_RX_HEADER_DONE_Pos      8UL
#define LIN_CH_INTR_SET_RX_HEADER_DONE_Msk      0x100UL
#define LIN_CH_INTR_SET_RX_RESPONSE_DONE_Pos    9UL
#define LIN_CH_INTR_SET_RX_RESPONSE_DONE_Msk    0x200UL
#define LIN_CH_INTR_SET_RX_BREAK_WAKEUP_DONE_Pos 10UL
#define LIN_CH_INTR_SET_RX_BREAK_WAKEUP_DONE_Msk 0x400UL
#define LIN_CH_INTR_SET_RX_HEADER_SYNC_DONE_Pos 11UL
#define LIN_CH_INTR_SET_RX_HEADER_SYNC_DONE_Msk 0x800UL
#define LIN_CH_INTR_SET_RX_NOISE_DETECT_Pos     13UL
#define LIN_CH_INTR_SET_RX_NOISE_DETECT_Msk     0x2000UL
#define LIN_CH_INTR_SET_TIMEOUT_Pos             14UL
#define LIN_CH_INTR_SET_TIMEOUT_Msk             0x4000UL
#define LIN_CH_INTR_SET_TX_HEADER_BIT_ERROR_Pos 16UL
#define LIN_CH_INTR_SET_TX_HEADER_BIT_ERROR_Msk 0x10000UL
#define LIN_CH_INTR_SET_TX_RESPONSE_BIT_ERROR_Pos 17UL
#define LIN_CH_INTR_SET_TX_RESPONSE_BIT_ERROR_Msk 0x20000UL
#define LIN_CH_INTR_SET_RX_HEADER_FRAME_ERROR_Pos 24UL
#define LIN_CH_INTR_SET_RX_HEADER_FRAME_ERROR_Msk 0x1000000UL
#define LIN_CH_INTR_SET_RX_HEADER_SYNC_ERROR_Pos 25UL
#define LIN_CH_INTR_SET_RX_HEADER_SYNC_ERROR_Msk 0x2000000UL
#define LIN_CH_INTR_SET_RX_HEADER_PARITY_ERROR_Pos 26UL
#define LIN_CH_INTR_SET_RX_HEADER_PARITY_ERROR_Msk 0x4000000UL
#define LIN_CH_INTR_SET_RX_RESPONSE_FRAME_ERROR_Pos 27UL
#define LIN_CH_INTR_SET_RX_RESPONSE_FRAME_ERROR_Msk 0x8000000UL
#define LIN_CH_INTR_SET_RX_RESPONSE_CHECKSUM_ERROR_Pos 28UL
#define LIN_CH_INTR_SET_RX_RESPONSE_CHECKSUM_ERROR_Msk 0x10000000UL
/* LIN_CH.INTR_MASK */
#define LIN_CH_INTR_MASK_TX_HEADER_DONE_Pos     0UL
#define LIN_CH_INTR_MASK_TX_HEADER_DONE_Msk     0x1UL
#define LIN_CH_INTR_MASK_TX_RESPONSE_DONE_Pos   1UL
#define LIN_CH_INTR_MASK_TX_RESPONSE_DONE_Msk   0x2UL
#define LIN_CH_INTR_MASK_TX_WAKEUP_DONE_Pos     2UL
#define LIN_CH_INTR_MASK_TX_WAKEUP_DONE_Msk     0x4UL
#define LIN_CH_INTR_MASK_RX_HEADER_DONE_Pos     8UL
#define LIN_CH_INTR_MASK_RX_HEADER_DONE_Msk     0x100UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_DONE_Pos   9UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_DONE_Msk   0x200UL
#define LIN_CH_INTR_MASK_RX_BREAK_WAKEUP_DONE_Pos 10UL
#define LIN_CH_INTR_MASK_RX_BREAK_WAKEUP_DONE_Msk 0x400UL
#define LIN_CH_INTR_MASK_RX_HEADER_SYNC_DONE_Pos 11UL
#define LIN_CH_INTR_MASK_RX_HEADER_SYNC_DONE_Msk 0x800UL
#define LIN_CH_INTR_MASK_RX_NOISE_DETECT_Pos    13UL
#define LIN_CH_INTR_MASK_RX_NOISE_DETECT_Msk    0x2000UL
#define LIN_CH_INTR_MASK_TIMEOUT_Pos            14UL
#define LIN_CH_INTR_MASK_TIMEOUT_Msk            0x4000UL
#define LIN_CH_INTR_MASK_TX_HEADER_BIT_ERROR_Pos 16UL
#define LIN_CH_INTR_MASK_TX_HEADER_BIT_ERROR_Msk 0x10000UL
#define LIN_CH_INTR_MASK_TX_RESPONSE_BIT_ERROR_Pos 17UL
#define LIN_CH_INTR_MASK_TX_RESPONSE_BIT_ERROR_Msk 0x20000UL
#define LIN_CH_INTR_MASK_RX_HEADER_FRAME_ERROR_Pos 24UL
#define LIN_CH_INTR_MASK_RX_HEADER_FRAME_ERROR_Msk 0x1000000UL
#define LIN_CH_INTR_MASK_RX_HEADER_SYNC_ERROR_Pos 25UL
#define LIN_CH_INTR_MASK_RX_HEADER_SYNC_ERROR_Msk 0x2000000UL
#define LIN_CH_INTR_MASK_RX_HEADER_PARITY_ERROR_Pos 26UL
#define LIN_CH_INTR_MASK_RX_HEADER_PARITY_ERROR_Msk 0x4000000UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_FRAME_ERROR_Pos 27UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_FRAME_ERROR_Msk 0x8000000UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_CHECKSUM_ERROR_Pos 28UL
#define LIN_CH_INTR_MASK_RX_RESPONSE_CHECKSUM_ERROR_Msk 0x10000000UL
/* LIN_CH.INTR_MASKED */
#define LIN_CH_INTR_MASKED_TX_HEADER_DONE_Pos   0UL
#define LIN_CH_INTR_MASKED_TX_HEADER_DONE_Msk   0x1UL
#define LIN_CH_INTR_MASKED_TX_RESPONSE_DONE_Pos 1UL
#define LIN_CH_INTR_MASKED_TX_RESPONSE_DONE_Msk 0x2UL
#define LIN_CH_INTR_MASKED_TX_WAKEUP_DONE_Pos   2UL
#define LIN_CH_INTR_MASKED_TX_WAKEUP_DONE_Msk   0x4UL
#define LIN_CH_INTR_MASKED_RX_HEADER_DONE_Pos   8UL
#define LIN_CH_INTR_MASKED_RX_HEADER_DONE_Msk   0x100UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_DONE_Pos 9UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_DONE_Msk 0x200UL
#define LIN_CH_INTR_MASKED_RX_BREAK_WAKEUP_DONE_Pos 10UL
#define LIN_CH_INTR_MASKED_RX_BREAK_WAKEUP_DONE_Msk 0x400UL
#define LIN_CH_INTR_MASKED_RX_HEADER_SYNC_DONE_Pos 11UL
#define LIN_CH_INTR_MASKED_RX_HEADER_SYNC_DONE_Msk 0x800UL
#define LIN_CH_INTR_MASKED_RX_NOISE_DETECT_Pos  13UL
#define LIN_CH_INTR_MASKED_RX_NOISE_DETECT_Msk  0x2000UL
#define LIN_CH_INTR_MASKED_TIMEOUT_Pos          14UL
#define LIN_CH_INTR_MASKED_TIMEOUT_Msk          0x4000UL
#define LIN_CH_INTR_MASKED_TX_HEADER_BIT_ERROR_Pos 16UL
#define LIN_CH_INTR_MASKED_TX_HEADER_BIT_ERROR_Msk 0x10000UL
#define LIN_CH_INTR_MASKED_TX_RESPONSE_BIT_ERROR_Pos 17UL
#define LIN_CH_INTR_MASKED_TX_RESPONSE_BIT_ERROR_Msk 0x20000UL
#define LIN_CH_INTR_MASKED_RX_HEADER_FRAME_ERROR_Pos 24UL
#define LIN_CH_INTR_MASKED_RX_HEADER_FRAME_ERROR_Msk 0x1000000UL
#define LIN_CH_INTR_MASKED_RX_HEADER_SYNC_ERROR_Pos 25UL
#define LIN_CH_INTR_MASKED_RX_HEADER_SYNC_ERROR_Msk 0x2000000UL
#define LIN_CH_INTR_MASKED_RX_HEADER_PARITY_ERROR_Pos 26UL
#define LIN_CH_INTR_MASKED_RX_HEADER_PARITY_ERROR_Msk 0x4000000UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_FRAME_ERROR_Pos 27UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_FRAME_ERROR_Msk 0x8000000UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_CHECKSUM_ERROR_Pos 28UL
#define LIN_CH_INTR_MASKED_RX_RESPONSE_CHECKSUM_ERROR_Msk 0x10000000UL


/* LIN.ERROR_CTL */
#define LIN_ERROR_CTL_CH_IDX_Pos                0UL
#define LIN_ERROR_CTL_CH_IDX_Msk                0x1FUL
#define LIN_ERROR_CTL_TX_SYNC_ERROR_Pos         16UL
#define LIN_ERROR_CTL_TX_SYNC_ERROR_Msk         0x10000UL
#define LIN_ERROR_CTL_TX_SYNC_STOP_ERROR_Pos    17UL
#define LIN_ERROR_CTL_TX_SYNC_STOP_ERROR_Msk    0x20000UL
#define LIN_ERROR_CTL_TX_PARITY_ERROR_Pos       18UL
#define LIN_ERROR_CTL_TX_PARITY_ERROR_Msk       0x40000UL
#define LIN_ERROR_CTL_TX_PID_STOP_ERROR_Pos     19UL
#define LIN_ERROR_CTL_TX_PID_STOP_ERROR_Msk     0x80000UL
#define LIN_ERROR_CTL_TX_DATA_STOP_ERROR_Pos    21UL
#define LIN_ERROR_CTL_TX_DATA_STOP_ERROR_Msk    0x200000UL
#define LIN_ERROR_CTL_TX_CHECKSUM_ERROR_Pos     22UL
#define LIN_ERROR_CTL_TX_CHECKSUM_ERROR_Msk     0x400000UL
#define LIN_ERROR_CTL_TX_CHECKSUM_STOP_ERROR_Pos 23UL
#define LIN_ERROR_CTL_TX_CHECKSUM_STOP_ERROR_Msk 0x800000UL
#define LIN_ERROR_CTL_ENABLED_Pos               31UL
#define LIN_ERROR_CTL_ENABLED_Msk               0x80000000UL
/* LIN.TEST_CTL */
#define LIN_TEST_CTL_CH_IDX_Pos                 0UL
#define LIN_TEST_CTL_CH_IDX_Msk                 0x1FUL
#define LIN_TEST_CTL_MODE_Pos                   16UL
#define LIN_TEST_CTL_MODE_Msk                   0x10000UL
#define LIN_TEST_CTL_ENABLED_Pos                31UL
#define LIN_TEST_CTL_ENABLED_Msk                0x80000000UL


#endif /* _CYIP_LIN_H_ */


/* [] END OF FILE */
