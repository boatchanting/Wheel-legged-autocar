/***************************************************************************//**
* \file cyip_crypto.h
*
* \brief
* CRYPTO IP definitions
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

#ifndef _CYIP_CRYPTO_H_
#define _CYIP_CRYPTO_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    CRYPTO
*******************************************************************************/

#define CRYPTO_SECTION_SIZE                     0x00010000UL

/**
  * \brief Control (CRYPTO_CTL)
  */
typedef struct stc_CRYPTO_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :2;
  uint32_t                         u4PC:4;
  uint32_t                         :8;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :13;
  uint32_t                         u1ENABLED:1;
} stc_CRYPTO_CTL_field_t;

typedef union un_CRYPTO_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CTL_t;

/**
  * \brief SRAM power control (CRYPTO_RAM_PWR_CTL)
  */
typedef struct stc_CRYPTO_RAM_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :30;
} stc_CRYPTO_RAM_PWR_CTL_field_t;

typedef union un_CRYPTO_RAM_PWR_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_RAM_PWR_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_RAM_PWR_CTL_t;

/**
  * \brief SRAM power delay control (CRYPTO_RAM_PWR_DELAY_CTL)
  */
typedef struct stc_CRYPTO_RAM_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CRYPTO_RAM_PWR_DELAY_CTL_field_t;

typedef union un_CRYPTO_RAM_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_RAM_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_RAM_PWR_DELAY_CTL_t;

/**
  * \brief ECC control (CRYPTO_ECC_CTL)
  */
typedef struct stc_CRYPTO_ECC_CTL_field {
  uint32_t                         u13WORD_ADDR:13;
  uint32_t                         :12;
  uint32_t                         u7PARITY:7;
} stc_CRYPTO_ECC_CTL_field_t;

typedef union un_CRYPTO_ECC_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_ECC_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_ECC_CTL_t;

/**
  * \brief Error status 0 (CRYPTO_ERROR_STATUS0)
  */
typedef struct stc_CRYPTO_ERROR_STATUS0_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_ERROR_STATUS0_field_t;

typedef union un_CRYPTO_ERROR_STATUS0 {
  uint32_t                         u32Register;
  stc_CRYPTO_ERROR_STATUS0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_ERROR_STATUS0_t;

/**
  * \brief Error status 1 (CRYPTO_ERROR_STATUS1)
  */
typedef struct stc_CRYPTO_ERROR_STATUS1_field {
  uint32_t                         u24DATA24:24;
  uint32_t                         u3IDX:3;
  uint32_t                         :4;
  uint32_t                         u1VALID:1;
} stc_CRYPTO_ERROR_STATUS1_field_t;

typedef union un_CRYPTO_ERROR_STATUS1 {
  uint32_t                         u32Register;
  stc_CRYPTO_ERROR_STATUS1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_ERROR_STATUS1_t;

/**
  * \brief Interrupt register (CRYPTO_INTR)
  */
typedef struct stc_CRYPTO_INTR_field {
  uint32_t                         u1INSTR_FF_LEVEL:1;
  uint32_t                         u1INSTR_FF_OVERFLOW:1;
  uint32_t                         u1TR_INITIALIZED:1;
  uint32_t                         u1TR_DATA_AVAILABLE:1;
  uint32_t                         u1PR_DATA_AVAILABLE:1;
  uint32_t                         :11;
  uint32_t                         u1INSTR_OPC_ERROR:1;
  uint32_t                         u1INSTR_CC_ERROR:1;
  uint32_t                         u1BUS_ERROR:1;
  uint32_t                         u1TR_AP_DETECT_ERROR:1;
  uint32_t                         u1TR_RC_DETECT_ERROR:1;
  uint32_t                         u1INSTR_DEV_KEY_ERROR:1;
  uint32_t                         :10;
} stc_CRYPTO_INTR_field_t;

typedef union un_CRYPTO_INTR {
  uint32_t                         u32Register;
  stc_CRYPTO_INTR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INTR_t;

/**
  * \brief Interrupt set register (CRYPTO_INTR_SET)
  */
typedef struct stc_CRYPTO_INTR_SET_field {
  uint32_t                         u1INSTR_FF_LEVEL:1;
  uint32_t                         u1INSTR_FF_OVERFLOW:1;
  uint32_t                         u1TR_INITIALIZED:1;
  uint32_t                         u1TR_DATA_AVAILABLE:1;
  uint32_t                         u1PR_DATA_AVAILABLE:1;
  uint32_t                         :11;
  uint32_t                         u1INSTR_OPC_ERROR:1;
  uint32_t                         u1INSTR_CC_ERROR:1;
  uint32_t                         u1BUS_ERROR:1;
  uint32_t                         u1TR_AP_DETECT_ERROR:1;
  uint32_t                         u1TR_RC_DETECT_ERROR:1;
  uint32_t                         u1INSTR_DEV_KEY_ERROR:1;
  uint32_t                         :10;
} stc_CRYPTO_INTR_SET_field_t;

typedef union un_CRYPTO_INTR_SET {
  uint32_t                         u32Register;
  stc_CRYPTO_INTR_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INTR_SET_t;

/**
  * \brief Interrupt mask register (CRYPTO_INTR_MASK)
  */
typedef struct stc_CRYPTO_INTR_MASK_field {
  uint32_t                         u1INSTR_FF_LEVEL:1;
  uint32_t                         u1INSTR_FF_OVERFLOW:1;
  uint32_t                         u1TR_INITIALIZED:1;
  uint32_t                         u1TR_DATA_AVAILABLE:1;
  uint32_t                         u1PR_DATA_AVAILABLE:1;
  uint32_t                         :11;
  uint32_t                         u1INSTR_OPC_ERROR:1;
  uint32_t                         u1INSTR_CC_ERROR:1;
  uint32_t                         u1BUS_ERROR:1;
  uint32_t                         u1TR_AP_DETECT_ERROR:1;
  uint32_t                         u1TR_RC_DETECT_ERROR:1;
  uint32_t                         u1INSTR_DEV_KEY_ERROR:1;
  uint32_t                         :10;
} stc_CRYPTO_INTR_MASK_field_t;

typedef union un_CRYPTO_INTR_MASK {
  uint32_t                         u32Register;
  stc_CRYPTO_INTR_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INTR_MASK_t;

/**
  * \brief Interrupt masked register (CRYPTO_INTR_MASKED)
  */
typedef struct stc_CRYPTO_INTR_MASKED_field {
  uint32_t                         u1INSTR_FF_LEVEL:1;
  uint32_t                         u1INSTR_FF_OVERFLOW:1;
  uint32_t                         u1TR_INITIALIZED:1;
  uint32_t                         u1TR_DATA_AVAILABLE:1;
  uint32_t                         u1PR_DATA_AVAILABLE:1;
  uint32_t                         :11;
  uint32_t                         u1INSTR_OPC_ERROR:1;
  uint32_t                         u1INSTR_CC_ERROR:1;
  uint32_t                         u1BUS_ERROR:1;
  uint32_t                         u1TR_AP_DETECT_ERROR:1;
  uint32_t                         u1TR_RC_DETECT_ERROR:1;
  uint32_t                         u1INSTR_DEV_KEY_ERROR:1;
  uint32_t                         :10;
} stc_CRYPTO_INTR_MASKED_field_t;

typedef union un_CRYPTO_INTR_MASKED {
  uint32_t                         u32Register;
  stc_CRYPTO_INTR_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INTR_MASKED_t;

/**
  * \brief Pseudo random LFSR control 0 (CRYPTO_PR_LFSR_CTL0)
  */
typedef struct stc_CRYPTO_PR_LFSR_CTL0_field {
  uint32_t                         u32LFSR32:32;
} stc_CRYPTO_PR_LFSR_CTL0_field_t;

typedef union un_CRYPTO_PR_LFSR_CTL0 {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_LFSR_CTL0_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_LFSR_CTL0_t;

/**
  * \brief Pseudo random LFSR control 1 (CRYPTO_PR_LFSR_CTL1)
  */
typedef struct stc_CRYPTO_PR_LFSR_CTL1_field {
  uint32_t                         u31LFSR31:31;
} stc_CRYPTO_PR_LFSR_CTL1_field_t;

typedef union un_CRYPTO_PR_LFSR_CTL1 {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_LFSR_CTL1_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_LFSR_CTL1_t;

/**
  * \brief Pseudo random LFSR control 2 (CRYPTO_PR_LFSR_CTL2)
  */
typedef struct stc_CRYPTO_PR_LFSR_CTL2_field {
  uint32_t                         u29LFSR29:29;
  uint32_t                         :3;
} stc_CRYPTO_PR_LFSR_CTL2_field_t;

typedef union un_CRYPTO_PR_LFSR_CTL2 {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_LFSR_CTL2_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_LFSR_CTL2_t;

/**
  * \brief Pseudo random maximum control (CRYPTO_PR_MAX_CTL)
  */
typedef struct stc_CRYPTO_PR_MAX_CTL_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_PR_MAX_CTL_field_t;

typedef union un_CRYPTO_PR_MAX_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_MAX_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_MAX_CTL_t;

/**
  * \brief Pseudo random command (CRYPTO_PR_CMD)
  */
typedef struct stc_CRYPTO_PR_CMD_field {
  uint32_t                         u1START:1;
  uint32_t                         :31;
} stc_CRYPTO_PR_CMD_field_t;

typedef union un_CRYPTO_PR_CMD {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_CMD_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_CMD_t;

/**
  * \brief Pseudo random result (CRYPTO_PR_RESULT)
  */
typedef struct stc_CRYPTO_PR_RESULT_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_PR_RESULT_field_t;

typedef union un_CRYPTO_PR_RESULT {
  uint32_t                         u32Register;
  stc_CRYPTO_PR_RESULT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_PR_RESULT_t;

/**
  * \brief True random control 0 (CRYPTO_TR_CTL0)
  */
typedef struct stc_CRYPTO_TR_CTL0_field {
  uint32_t                         u8SAMPLE_CLOCK_DIV:8;
  uint32_t                         u8RED_CLOCK_DIV:8;
  uint32_t                         u8INIT_DELAY:8;
  uint32_t                         u1VON_NEUMANN_CORR:1;
  uint32_t                         :3;
  uint32_t                         u1STOP_ON_AP_DETECT:1;
  uint32_t                         u1STOP_ON_RC_DETECT:1;
  uint32_t                         :2;
} stc_CRYPTO_TR_CTL0_field_t;

typedef union un_CRYPTO_TR_CTL0 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_CTL0_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_CTL0_t;

/**
  * \brief True random control 1 (CRYPTO_TR_CTL1)
  */
typedef struct stc_CRYPTO_TR_CTL1_field {
  uint32_t                         u1RO11_EN:1;
  uint32_t                         u1RO15_EN:1;
  uint32_t                         u1GARO15_EN:1;
  uint32_t                         u1GARO31_EN:1;
  uint32_t                         u1FIRO15_EN:1;
  uint32_t                         u1FIRO31_EN:1;
  uint32_t                         :26;
} stc_CRYPTO_TR_CTL1_field_t;

typedef union un_CRYPTO_TR_CTL1 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_CTL1_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_CTL1_t;

/**
  * \brief True random control 2 (CRYPTO_TR_CTL2)
  */
typedef struct stc_CRYPTO_TR_CTL2_field {
  uint32_t                         u6SIZE:6;
  uint32_t                         :26;
} stc_CRYPTO_TR_CTL2_field_t;

typedef union un_CRYPTO_TR_CTL2 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_CTL2_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_CTL2_t;

/**
  * \brief True random status (CRYPTO_TR_STATUS)
  */
typedef struct stc_CRYPTO_TR_STATUS_field {
  uint32_t                         u1INITIALIZED:1;
  uint32_t                         :31;
} stc_CRYPTO_TR_STATUS_field_t;

typedef union un_CRYPTO_TR_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_STATUS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_STATUS_t;

/**
  * \brief True random command (CRYPTO_TR_CMD)
  */
typedef struct stc_CRYPTO_TR_CMD_field {
  uint32_t                         u1START:1;
  uint32_t                         :31;
} stc_CRYPTO_TR_CMD_field_t;

typedef union un_CRYPTO_TR_CMD {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_CMD_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_CMD_t;

/**
  * \brief True random result (CRYPTO_TR_RESULT)
  */
typedef struct stc_CRYPTO_TR_RESULT_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_TR_RESULT_field_t;

typedef union un_CRYPTO_TR_RESULT {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_RESULT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_RESULT_t;

/**
  * \brief True random GARO control (CRYPTO_TR_GARO_CTL)
  */
typedef struct stc_CRYPTO_TR_GARO_CTL_field {
  uint32_t                         u31POLYNOMIAL31:31;
} stc_CRYPTO_TR_GARO_CTL_field_t;

typedef union un_CRYPTO_TR_GARO_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_GARO_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_GARO_CTL_t;

/**
  * \brief True random FIRO control (CRYPTO_TR_FIRO_CTL)
  */
typedef struct stc_CRYPTO_TR_FIRO_CTL_field {
  uint32_t                         u31POLYNOMIAL31:31;
} stc_CRYPTO_TR_FIRO_CTL_field_t;

typedef union un_CRYPTO_TR_FIRO_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_FIRO_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_FIRO_CTL_t;

/**
  * \brief True random monitor control (CRYPTO_TR_MON_CTL)
  */
typedef struct stc_CRYPTO_TR_MON_CTL_field {
  uint32_t                         u2BITSTREAM_SEL:2;
  uint32_t                         :30;
} stc_CRYPTO_TR_MON_CTL_field_t;

typedef union un_CRYPTO_TR_MON_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_CTL_t;

/**
  * \brief True random monitor command (CRYPTO_TR_MON_CMD)
  */
typedef struct stc_CRYPTO_TR_MON_CMD_field {
  uint32_t                         u1START_AP:1;
  uint32_t                         u1START_RC:1;
  uint32_t                         :30;
} stc_CRYPTO_TR_MON_CMD_field_t;

typedef union un_CRYPTO_TR_MON_CMD {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_CMD_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_CMD_t;

/**
  * \brief True random monitor RC control (CRYPTO_TR_MON_RC_CTL)
  */
typedef struct stc_CRYPTO_TR_MON_RC_CTL_field {
  uint32_t                         u8CUTOFF_COUNT8:8;
  uint32_t                         :24;
} stc_CRYPTO_TR_MON_RC_CTL_field_t;

typedef union un_CRYPTO_TR_MON_RC_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_RC_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_RC_CTL_t;

/**
  * \brief True random monitor RC status 0 (CRYPTO_TR_MON_RC_STATUS0)
  */
typedef struct stc_CRYPTO_TR_MON_RC_STATUS0_field {
  uint32_t                         u1BIT:1;
  uint32_t                         :31;
} stc_CRYPTO_TR_MON_RC_STATUS0_field_t;

typedef union un_CRYPTO_TR_MON_RC_STATUS0 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_RC_STATUS0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_RC_STATUS0_t;

/**
  * \brief True random monitor RC status 1 (CRYPTO_TR_MON_RC_STATUS1)
  */
typedef struct stc_CRYPTO_TR_MON_RC_STATUS1_field {
  uint32_t                         u8REP_COUNT:8;
  uint32_t                         :24;
} stc_CRYPTO_TR_MON_RC_STATUS1_field_t;

typedef union un_CRYPTO_TR_MON_RC_STATUS1 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_RC_STATUS1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_RC_STATUS1_t;

/**
  * \brief True random monitor AP control (CRYPTO_TR_MON_AP_CTL)
  */
typedef struct stc_CRYPTO_TR_MON_AP_CTL_field {
  uint32_t                         u16CUTOFF_COUNT16:16;
  uint32_t                         u16WINDOW_SIZE:16;
} stc_CRYPTO_TR_MON_AP_CTL_field_t;

typedef union un_CRYPTO_TR_MON_AP_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_AP_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_AP_CTL_t;

/**
  * \brief True random monitor AP status 0 (CRYPTO_TR_MON_AP_STATUS0)
  */
typedef struct stc_CRYPTO_TR_MON_AP_STATUS0_field {
  uint32_t                         u1BIT:1;
  uint32_t                         :31;
} stc_CRYPTO_TR_MON_AP_STATUS0_field_t;

typedef union un_CRYPTO_TR_MON_AP_STATUS0 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_AP_STATUS0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_AP_STATUS0_t;

/**
  * \brief True random monitor AP status 1 (CRYPTO_TR_MON_AP_STATUS1)
  */
typedef struct stc_CRYPTO_TR_MON_AP_STATUS1_field {
  uint32_t                         u16OCC_COUNT:16;
  uint32_t                         u16WINDOW_INDEX:16;
} stc_CRYPTO_TR_MON_AP_STATUS1_field_t;

typedef union un_CRYPTO_TR_MON_AP_STATUS1 {
  uint32_t                         u32Register;
  stc_CRYPTO_TR_MON_AP_STATUS1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_TR_MON_AP_STATUS1_t;

/**
  * \brief Status (CRYPTO_STATUS)
  */
typedef struct stc_CRYPTO_STATUS_field {
  uint32_t                         :31;
  uint32_t                         u1BUSY:1;
} stc_CRYPTO_STATUS_field_t;

typedef union un_CRYPTO_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_STATUS_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_STATUS_t;

/**
  * \brief Instruction FIFO control (CRYPTO_INSTR_FF_CTL)
  */
typedef struct stc_CRYPTO_INSTR_FF_CTL_field {
  uint32_t                         u3EVENT_LEVEL:3;
  uint32_t                         :13;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1BLOCK:1;
  uint32_t                         :14;
} stc_CRYPTO_INSTR_FF_CTL_field_t;

typedef union un_CRYPTO_INSTR_FF_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_INSTR_FF_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INSTR_FF_CTL_t;

/**
  * \brief Instruction FIFO status (CRYPTO_INSTR_FF_STATUS)
  */
typedef struct stc_CRYPTO_INSTR_FF_STATUS_field {
  uint32_t                         u4USED:4;
  uint32_t                         :12;
  uint32_t                         u1EVENT:1;
  uint32_t                         :15;
} stc_CRYPTO_INSTR_FF_STATUS_field_t;

typedef union un_CRYPTO_INSTR_FF_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_INSTR_FF_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INSTR_FF_STATUS_t;

/**
  * \brief Instruction FIFO write (CRYPTO_INSTR_FF_WR)
  */
typedef struct stc_CRYPTO_INSTR_FF_WR_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_INSTR_FF_WR_field_t;

typedef union un_CRYPTO_INSTR_FF_WR {
  uint32_t                         u32Register;
  stc_CRYPTO_INSTR_FF_WR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_INSTR_FF_WR_t;

/**
  * \brief Load 0 FIFO status (CRYPTO_LOAD0_FF_STATUS)
  */
typedef struct stc_CRYPTO_LOAD0_FF_STATUS_field {
  uint32_t                         u5USED5:5;
  uint32_t                         :26;
  uint32_t                         u1BUSY:1;
} stc_CRYPTO_LOAD0_FF_STATUS_field_t;

typedef union un_CRYPTO_LOAD0_FF_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_LOAD0_FF_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_LOAD0_FF_STATUS_t;

/**
  * \brief Load 1 FIFO status (CRYPTO_LOAD1_FF_STATUS)
  */
typedef struct stc_CRYPTO_LOAD1_FF_STATUS_field {
  uint32_t                         u5USED5:5;
  uint32_t                         :26;
  uint32_t                         u1BUSY:1;
} stc_CRYPTO_LOAD1_FF_STATUS_field_t;

typedef union un_CRYPTO_LOAD1_FF_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_LOAD1_FF_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_LOAD1_FF_STATUS_t;

/**
  * \brief Store FIFO status (CRYPTO_STORE_FF_STATUS)
  */
typedef struct stc_CRYPTO_STORE_FF_STATUS_field {
  uint32_t                         u5USED5:5;
  uint32_t                         :26;
  uint32_t                         u1BUSY:1;
} stc_CRYPTO_STORE_FF_STATUS_field_t;

typedef union un_CRYPTO_STORE_FF_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_STORE_FF_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_STORE_FF_STATUS_t;

/**
  * \brief AES control (CRYPTO_AES_CTL)
  */
typedef struct stc_CRYPTO_AES_CTL_field {
  uint32_t                         u2KEY_SIZE:2;
  uint32_t                         :30;
} stc_CRYPTO_AES_CTL_field_t;

typedef union un_CRYPTO_AES_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_AES_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_AES_CTL_t;

/**
  * \brief Result (CRYPTO_RESULT)
  */
typedef struct stc_CRYPTO_RESULT_field {
  uint32_t                         u32DATA:32;
} stc_CRYPTO_RESULT_field_t;

typedef union un_CRYPTO_RESULT {
  uint32_t                         u32Register;
  stc_CRYPTO_RESULT_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_RESULT_t;

/**
  * \brief CRC control (CRYPTO_CRC_CTL)
  */
typedef struct stc_CRYPTO_CRC_CTL_field {
  uint32_t                         u1DATA_REVERSE:1;
  uint32_t                         :7;
  uint32_t                         u1REM_REVERSE:1;
  uint32_t                         :23;
} stc_CRYPTO_CRC_CTL_field_t;

typedef union un_CRYPTO_CRC_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_CRC_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CRC_CTL_t;

/**
  * \brief CRC data control (CRYPTO_CRC_DATA_CTL)
  */
typedef struct stc_CRYPTO_CRC_DATA_CTL_field {
  uint32_t                         u8DATA_XOR:8;
  uint32_t                         :24;
} stc_CRYPTO_CRC_DATA_CTL_field_t;

typedef union un_CRYPTO_CRC_DATA_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_CRC_DATA_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CRC_DATA_CTL_t;

/**
  * \brief CRC polynomial control (CRYPTO_CRC_POL_CTL)
  */
typedef struct stc_CRYPTO_CRC_POL_CTL_field {
  uint32_t                         u32POLYNOMIAL:32;
} stc_CRYPTO_CRC_POL_CTL_field_t;

typedef union un_CRYPTO_CRC_POL_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_CRC_POL_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CRC_POL_CTL_t;

/**
  * \brief CRC remainder control (CRYPTO_CRC_REM_CTL)
  */
typedef struct stc_CRYPTO_CRC_REM_CTL_field {
  uint32_t                         u32REM_XOR:32;
} stc_CRYPTO_CRC_REM_CTL_field_t;

typedef union un_CRYPTO_CRC_REM_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_CRC_REM_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CRC_REM_CTL_t;

/**
  * \brief CRC remainder result (CRYPTO_CRC_REM_RESULT)
  */
typedef struct stc_CRYPTO_CRC_REM_RESULT_field {
  uint32_t                         u32REM:32;
} stc_CRYPTO_CRC_REM_RESULT_field_t;

typedef union un_CRYPTO_CRC_REM_RESULT {
  uint32_t                         u32Register;
  stc_CRYPTO_CRC_REM_RESULT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_CRC_REM_RESULT_t;

/**
  * \brief Vector unit control 0 (CRYPTO_VU_CTL0)
  */
typedef struct stc_CRYPTO_VU_CTL0_field {
  uint32_t                         u1ALWAYS_EXECUTE:1;
  uint32_t                         :31;
} stc_CRYPTO_VU_CTL0_field_t;

typedef union un_CRYPTO_VU_CTL0 {
  uint32_t                         u32Register;
  stc_CRYPTO_VU_CTL0_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_VU_CTL0_t;

/**
  * \brief Vector unit control 1 (CRYPTO_VU_CTL1)
  */
typedef struct stc_CRYPTO_VU_CTL1_field {
  uint32_t                         :8;
  uint32_t                         u24ADDR24:24;
} stc_CRYPTO_VU_CTL1_field_t;

typedef union un_CRYPTO_VU_CTL1 {
  uint32_t                         u32Register;
  stc_CRYPTO_VU_CTL1_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_VU_CTL1_t;

/**
  * \brief Vector unit control 2 (CRYPTO_VU_CTL2)
  */
typedef struct stc_CRYPTO_VU_CTL2_field {
  uint32_t                         :8;
  uint32_t                         u7MASK:7;
  uint32_t                         :17;
} stc_CRYPTO_VU_CTL2_field_t;

typedef union un_CRYPTO_VU_CTL2 {
  uint32_t                         u32Register;
  stc_CRYPTO_VU_CTL2_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_VU_CTL2_t;

/**
  * \brief Vector unit status (CRYPTO_VU_STATUS)
  */
typedef struct stc_CRYPTO_VU_STATUS_field {
  uint32_t                         u1CARRY:1;
  uint32_t                         u1EVEN:1;
  uint32_t                         u1ZERO:1;
  uint32_t                         u1ONE:1;
  uint32_t                         :28;
} stc_CRYPTO_VU_STATUS_field_t;

typedef union un_CRYPTO_VU_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_VU_STATUS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_VU_STATUS_t;

/**
  * \brief Vector unit register-file (CRYPTO_VU_RF_DATA)
  */
typedef struct stc_CRYPTO_VU_RF_DATA_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_VU_RF_DATA_field_t;

typedef union un_CRYPTO_VU_RF_DATA {
  uint32_t                         u32Register;
  stc_CRYPTO_VU_RF_DATA_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_VU_RF_DATA_t;

/**
  * \brief Device key address 0 control (CRYPTO_DEV_KEY_ADDR0_CTL)
  */
typedef struct stc_CRYPTO_DEV_KEY_ADDR0_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1VALID:1;
} stc_CRYPTO_DEV_KEY_ADDR0_CTL_field_t;

typedef union un_CRYPTO_DEV_KEY_ADDR0_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_ADDR0_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_ADDR0_CTL_t;

/**
  * \brief Device key address 0 (CRYPTO_DEV_KEY_ADDR0)
  */
typedef struct stc_CRYPTO_DEV_KEY_ADDR0_field {
  uint32_t                         u32ADDR32:32;
} stc_CRYPTO_DEV_KEY_ADDR0_field_t;

typedef union un_CRYPTO_DEV_KEY_ADDR0 {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_ADDR0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_ADDR0_t;

/**
  * \brief Device key address 1 control (CRYPTO_DEV_KEY_ADDR1_CTL)
  */
typedef struct stc_CRYPTO_DEV_KEY_ADDR1_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1VALID:1;
} stc_CRYPTO_DEV_KEY_ADDR1_CTL_field_t;

typedef union un_CRYPTO_DEV_KEY_ADDR1_CTL {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_ADDR1_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_ADDR1_CTL_t;

/**
  * \brief Device key address 1 control (CRYPTO_DEV_KEY_ADDR1)
  */
typedef struct stc_CRYPTO_DEV_KEY_ADDR1_field {
  uint32_t                         u32ADDR32:32;
} stc_CRYPTO_DEV_KEY_ADDR1_field_t;

typedef union un_CRYPTO_DEV_KEY_ADDR1 {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_ADDR1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_ADDR1_t;

/**
  * \brief Device key status (CRYPTO_DEV_KEY_STATUS)
  */
typedef struct stc_CRYPTO_DEV_KEY_STATUS_field {
  uint32_t                         u1LOADED:1;
  uint32_t                         :31;
} stc_CRYPTO_DEV_KEY_STATUS_field_t;

typedef union un_CRYPTO_DEV_KEY_STATUS {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_STATUS_t;

/**
  * \brief Device key control 0 (CRYPTO_DEV_KEY_CTL0)
  */
typedef struct stc_CRYPTO_DEV_KEY_CTL0_field {
  uint32_t                         u1ALLOWED:1;
  uint32_t                         :31;
} stc_CRYPTO_DEV_KEY_CTL0_field_t;

typedef union un_CRYPTO_DEV_KEY_CTL0 {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_CTL0_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_CTL0_t;

/**
  * \brief Device key control 1 (CRYPTO_DEV_KEY_CTL1)
  */
typedef struct stc_CRYPTO_DEV_KEY_CTL1_field {
  uint32_t                         u1ALLOWED:1;
  uint32_t                         :31;
} stc_CRYPTO_DEV_KEY_CTL1_field_t;

typedef union un_CRYPTO_DEV_KEY_CTL1 {
  uint32_t                         u32Register;
  stc_CRYPTO_DEV_KEY_CTL1_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_DEV_KEY_CTL1_t;

/**
  * \brief Memory buffer (CRYPTO_MEM_BUFF)
  */
typedef struct stc_CRYPTO_MEM_BUFF_field {
  uint32_t                         u32DATA32:32;
} stc_CRYPTO_MEM_BUFF_field_t;

typedef union un_CRYPTO_MEM_BUFF {
  uint32_t                         u32Register;
  stc_CRYPTO_MEM_BUFF_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CRYPTO_MEM_BUFF_t;



/**
  * \brief Cryptography component (CRYPTO)
  */
typedef struct stc_CRYPTO {
  un_CRYPTO_CTL_t                  unCTL;              /*!< 0x00000000 Control */
  uint32_t                         u32Reserved;
  un_CRYPTO_RAM_PWR_CTL_t          unRAM_PWR_CTL;      /*!< 0x00000008 SRAM power control */
  un_CRYPTO_RAM_PWR_DELAY_CTL_t    unRAM_PWR_DELAY_CTL; /*!< 0x0000000C SRAM power delay control */
  un_CRYPTO_ECC_CTL_t              unECC_CTL;          /*!< 0x00000010 ECC control */
  uint32_t                         au32Reserved1[3];
  un_CRYPTO_ERROR_STATUS0_t        unERROR_STATUS0;    /*!< 0x00000020 Error status 0 */
  un_CRYPTO_ERROR_STATUS1_t        unERROR_STATUS1;    /*!< 0x00000024 Error status 1 */
  uint32_t                         au32Reserved2[54];
  un_CRYPTO_INTR_t                 unINTR;             /*!< 0x00000100 Interrupt register */
  un_CRYPTO_INTR_SET_t             unINTR_SET;         /*!< 0x00000104 Interrupt set register */
  un_CRYPTO_INTR_MASK_t            unINTR_MASK;        /*!< 0x00000108 Interrupt mask register */
  un_CRYPTO_INTR_MASKED_t          unINTR_MASKED;      /*!< 0x0000010C Interrupt masked register */
  uint32_t                         au32Reserved3[60];
  un_CRYPTO_PR_LFSR_CTL0_t         unPR_LFSR_CTL0;     /*!< 0x00000200 Pseudo random LFSR control 0 */
  un_CRYPTO_PR_LFSR_CTL1_t         unPR_LFSR_CTL1;     /*!< 0x00000204 Pseudo random LFSR control 1 */
  un_CRYPTO_PR_LFSR_CTL2_t         unPR_LFSR_CTL2;     /*!< 0x00000208 Pseudo random LFSR control 2 */
  un_CRYPTO_PR_MAX_CTL_t           unPR_MAX_CTL;       /*!< 0x0000020C Pseudo random maximum control */
  un_CRYPTO_PR_CMD_t               unPR_CMD;           /*!< 0x00000210 Pseudo random command */
  uint32_t                         u32Reserved4;
  un_CRYPTO_PR_RESULT_t            unPR_RESULT;        /*!< 0x00000218 Pseudo random result */
  uint32_t                         au32Reserved5[25];
  un_CRYPTO_TR_CTL0_t              unTR_CTL0;          /*!< 0x00000280 True random control 0 */
  un_CRYPTO_TR_CTL1_t              unTR_CTL1;          /*!< 0x00000284 True random control 1 */
  un_CRYPTO_TR_CTL2_t              unTR_CTL2;          /*!< 0x00000288 True random control 2 */
  un_CRYPTO_TR_STATUS_t            unTR_STATUS;        /*!< 0x0000028C True random status */
  un_CRYPTO_TR_CMD_t               unTR_CMD;           /*!< 0x00000290 True random command */
  uint32_t                         u32Reserved6;
  un_CRYPTO_TR_RESULT_t            unTR_RESULT;        /*!< 0x00000298 True random result */
  uint32_t                         u32Reserved7;
  un_CRYPTO_TR_GARO_CTL_t          unTR_GARO_CTL;      /*!< 0x000002A0 True random GARO control */
  un_CRYPTO_TR_FIRO_CTL_t          unTR_FIRO_CTL;      /*!< 0x000002A4 True random FIRO control */
  uint32_t                         au32Reserved8[6];
  un_CRYPTO_TR_MON_CTL_t           unTR_MON_CTL;       /*!< 0x000002C0 True random monitor control */
  uint32_t                         u32Reserved9;
  un_CRYPTO_TR_MON_CMD_t           unTR_MON_CMD;       /*!< 0x000002C8 True random monitor command */
  uint32_t                         u32Reserved10;
  un_CRYPTO_TR_MON_RC_CTL_t        unTR_MON_RC_CTL;    /*!< 0x000002D0 True random monitor RC control */
  uint32_t                         u32Reserved11;
  un_CRYPTO_TR_MON_RC_STATUS0_t    unTR_MON_RC_STATUS0; /*!< 0x000002D8 True random monitor RC status 0 */
  un_CRYPTO_TR_MON_RC_STATUS1_t    unTR_MON_RC_STATUS1; /*!< 0x000002DC True random monitor RC status 1 */
  un_CRYPTO_TR_MON_AP_CTL_t        unTR_MON_AP_CTL;    /*!< 0x000002E0 True random monitor AP control */
  uint32_t                         u32Reserved12;
  un_CRYPTO_TR_MON_AP_STATUS0_t    unTR_MON_AP_STATUS0; /*!< 0x000002E8 True random monitor AP status 0 */
  un_CRYPTO_TR_MON_AP_STATUS1_t    unTR_MON_AP_STATUS1; /*!< 0x000002EC True random monitor AP status 1 */
  uint32_t                         au32Reserved13[837];
  un_CRYPTO_STATUS_t               unSTATUS;           /*!< 0x00001004 Status */
  uint32_t                         au32Reserved14[14];
  un_CRYPTO_INSTR_FF_CTL_t         unINSTR_FF_CTL;     /*!< 0x00001040 Instruction FIFO control */
  un_CRYPTO_INSTR_FF_STATUS_t      unINSTR_FF_STATUS;  /*!< 0x00001044 Instruction FIFO status */
  un_CRYPTO_INSTR_FF_WR_t          unINSTR_FF_WR;      /*!< 0x00001048 Instruction FIFO write */
  uint32_t                         au32Reserved15[29];
  un_CRYPTO_LOAD0_FF_STATUS_t      unLOAD0_FF_STATUS;  /*!< 0x000010C0 Load 0 FIFO status */
  uint32_t                         au32Reserved16[3];
  un_CRYPTO_LOAD1_FF_STATUS_t      unLOAD1_FF_STATUS;  /*!< 0x000010D0 Load 1 FIFO status */
  uint32_t                         au32Reserved17[7];
  un_CRYPTO_STORE_FF_STATUS_t      unSTORE_FF_STATUS;  /*!< 0x000010F0 Store FIFO status */
  uint32_t                         au32Reserved18[3];
  un_CRYPTO_AES_CTL_t              unAES_CTL;          /*!< 0x00001100 AES control */
  uint32_t                         au32Reserved19[31];
  un_CRYPTO_RESULT_t               unRESULT;           /*!< 0x00001180 Result */
  uint32_t                         au32Reserved20[159];
  un_CRYPTO_CRC_CTL_t              unCRC_CTL;          /*!< 0x00001400 CRC control */
  uint32_t                         au32Reserved21[3];
  un_CRYPTO_CRC_DATA_CTL_t         unCRC_DATA_CTL;     /*!< 0x00001410 CRC data control */
  uint32_t                         au32Reserved22[3];
  un_CRYPTO_CRC_POL_CTL_t          unCRC_POL_CTL;      /*!< 0x00001420 CRC polynomial control */
  uint32_t                         au32Reserved23[7];
  un_CRYPTO_CRC_REM_CTL_t          unCRC_REM_CTL;      /*!< 0x00001440 CRC remainder control */
  uint32_t                         u32Reserved24;
  un_CRYPTO_CRC_REM_RESULT_t       unCRC_REM_RESULT;   /*!< 0x00001448 CRC remainder result */
  uint32_t                         au32Reserved25[13];
  un_CRYPTO_VU_CTL0_t              unVU_CTL0;          /*!< 0x00001480 Vector unit control 0 */
  un_CRYPTO_VU_CTL1_t              unVU_CTL1;          /*!< 0x00001484 Vector unit control 1 */
  un_CRYPTO_VU_CTL2_t              unVU_CTL2;          /*!< 0x00001488 Vector unit control 2 */
  uint32_t                         u32Reserved26;
  un_CRYPTO_VU_STATUS_t            unVU_STATUS;        /*!< 0x00001490 Vector unit status */
  uint32_t                         au32Reserved27[11];
  un_CRYPTO_VU_RF_DATA_t           unVU_RF_DATA[16];   /*!< 0x000014C0 Vector unit register-file */
  uint32_t                         au32Reserved28[704];
  un_CRYPTO_DEV_KEY_ADDR0_CTL_t    unDEV_KEY_ADDR0_CTL; /*!< 0x00002000 Device key address 0 control */
  un_CRYPTO_DEV_KEY_ADDR0_t        unDEV_KEY_ADDR0;    /*!< 0x00002004 Device key address 0 */
  uint32_t                         au32Reserved29[6];
  un_CRYPTO_DEV_KEY_ADDR1_CTL_t    unDEV_KEY_ADDR1_CTL; /*!< 0x00002020 Device key address 1 control */
  un_CRYPTO_DEV_KEY_ADDR1_t        unDEV_KEY_ADDR1;    /*!< 0x00002024 Device key address 1 control */
  uint32_t                         au32Reserved30[22];
  un_CRYPTO_DEV_KEY_STATUS_t       unDEV_KEY_STATUS;   /*!< 0x00002080 Device key status */
  uint32_t                         au32Reserved31[31];
  un_CRYPTO_DEV_KEY_CTL0_t         unDEV_KEY_CTL0;     /*!< 0x00002100 Device key control 0 */
  uint32_t                         au32Reserved32[7];
  un_CRYPTO_DEV_KEY_CTL1_t         unDEV_KEY_CTL1;     /*!< 0x00002120 Device key control 1 */
  uint32_t                         au32Reserved33[6071];
  un_CRYPTO_MEM_BUFF_t             unMEM_BUFF[8192];   /*!< 0x00008000 Memory buffer */
} stc_CRYPTO_t;                                        /*!< Size = 65536 (0x10000) */


/* CRYPTO.CTL */
#define CRYPTO_CTL_P_Pos                        0UL
#define CRYPTO_CTL_P_Msk                        0x1UL
#define CRYPTO_CTL_NS_Pos                       1UL
#define CRYPTO_CTL_NS_Msk                       0x2UL
#define CRYPTO_CTL_PC_Pos                       4UL
#define CRYPTO_CTL_PC_Msk                       0xF0UL
#define CRYPTO_CTL_ECC_EN_Pos                   16UL
#define CRYPTO_CTL_ECC_EN_Msk                   0x10000UL
#define CRYPTO_CTL_ECC_INJ_EN_Pos               17UL
#define CRYPTO_CTL_ECC_INJ_EN_Msk               0x20000UL
#define CRYPTO_CTL_ENABLED_Pos                  31UL
#define CRYPTO_CTL_ENABLED_Msk                  0x80000000UL
/* CRYPTO.RAM_PWR_CTL */
#define CRYPTO_RAM_PWR_CTL_PWR_MODE_Pos         0UL
#define CRYPTO_RAM_PWR_CTL_PWR_MODE_Msk         0x3UL
/* CRYPTO.RAM_PWR_DELAY_CTL */
#define CRYPTO_RAM_PWR_DELAY_CTL_UP_Pos         0UL
#define CRYPTO_RAM_PWR_DELAY_CTL_UP_Msk         0x3FFUL
/* CRYPTO.ECC_CTL */
#define CRYPTO_ECC_CTL_WORD_ADDR_Pos            0UL
#define CRYPTO_ECC_CTL_WORD_ADDR_Msk            0x1FFFUL
#define CRYPTO_ECC_CTL_PARITY_Pos               25UL
#define CRYPTO_ECC_CTL_PARITY_Msk               0xFE000000UL
/* CRYPTO.ERROR_STATUS0 */
#define CRYPTO_ERROR_STATUS0_DATA32_Pos         0UL
#define CRYPTO_ERROR_STATUS0_DATA32_Msk         0xFFFFFFFFUL
/* CRYPTO.ERROR_STATUS1 */
#define CRYPTO_ERROR_STATUS1_DATA24_Pos         0UL
#define CRYPTO_ERROR_STATUS1_DATA24_Msk         0xFFFFFFUL
#define CRYPTO_ERROR_STATUS1_IDX_Pos            24UL
#define CRYPTO_ERROR_STATUS1_IDX_Msk            0x7000000UL
#define CRYPTO_ERROR_STATUS1_VALID_Pos          31UL
#define CRYPTO_ERROR_STATUS1_VALID_Msk          0x80000000UL
/* CRYPTO.INTR */
#define CRYPTO_INTR_INSTR_FF_LEVEL_Pos          0UL
#define CRYPTO_INTR_INSTR_FF_LEVEL_Msk          0x1UL
#define CRYPTO_INTR_INSTR_FF_OVERFLOW_Pos       1UL
#define CRYPTO_INTR_INSTR_FF_OVERFLOW_Msk       0x2UL
#define CRYPTO_INTR_TR_INITIALIZED_Pos          2UL
#define CRYPTO_INTR_TR_INITIALIZED_Msk          0x4UL
#define CRYPTO_INTR_TR_DATA_AVAILABLE_Pos       3UL
#define CRYPTO_INTR_TR_DATA_AVAILABLE_Msk       0x8UL
#define CRYPTO_INTR_PR_DATA_AVAILABLE_Pos       4UL
#define CRYPTO_INTR_PR_DATA_AVAILABLE_Msk       0x10UL
#define CRYPTO_INTR_INSTR_OPC_ERROR_Pos         16UL
#define CRYPTO_INTR_INSTR_OPC_ERROR_Msk         0x10000UL
#define CRYPTO_INTR_INSTR_CC_ERROR_Pos          17UL
#define CRYPTO_INTR_INSTR_CC_ERROR_Msk          0x20000UL
#define CRYPTO_INTR_BUS_ERROR_Pos               18UL
#define CRYPTO_INTR_BUS_ERROR_Msk               0x40000UL
#define CRYPTO_INTR_TR_AP_DETECT_ERROR_Pos      19UL
#define CRYPTO_INTR_TR_AP_DETECT_ERROR_Msk      0x80000UL
#define CRYPTO_INTR_TR_RC_DETECT_ERROR_Pos      20UL
#define CRYPTO_INTR_TR_RC_DETECT_ERROR_Msk      0x100000UL
#define CRYPTO_INTR_INSTR_DEV_KEY_ERROR_Pos     21UL
#define CRYPTO_INTR_INSTR_DEV_KEY_ERROR_Msk     0x200000UL
/* CRYPTO.INTR_SET */
#define CRYPTO_INTR_SET_INSTR_FF_LEVEL_Pos      0UL
#define CRYPTO_INTR_SET_INSTR_FF_LEVEL_Msk      0x1UL
#define CRYPTO_INTR_SET_INSTR_FF_OVERFLOW_Pos   1UL
#define CRYPTO_INTR_SET_INSTR_FF_OVERFLOW_Msk   0x2UL
#define CRYPTO_INTR_SET_TR_INITIALIZED_Pos      2UL
#define CRYPTO_INTR_SET_TR_INITIALIZED_Msk      0x4UL
#define CRYPTO_INTR_SET_TR_DATA_AVAILABLE_Pos   3UL
#define CRYPTO_INTR_SET_TR_DATA_AVAILABLE_Msk   0x8UL
#define CRYPTO_INTR_SET_PR_DATA_AVAILABLE_Pos   4UL
#define CRYPTO_INTR_SET_PR_DATA_AVAILABLE_Msk   0x10UL
#define CRYPTO_INTR_SET_INSTR_OPC_ERROR_Pos     16UL
#define CRYPTO_INTR_SET_INSTR_OPC_ERROR_Msk     0x10000UL
#define CRYPTO_INTR_SET_INSTR_CC_ERROR_Pos      17UL
#define CRYPTO_INTR_SET_INSTR_CC_ERROR_Msk      0x20000UL
#define CRYPTO_INTR_SET_BUS_ERROR_Pos           18UL
#define CRYPTO_INTR_SET_BUS_ERROR_Msk           0x40000UL
#define CRYPTO_INTR_SET_TR_AP_DETECT_ERROR_Pos  19UL
#define CRYPTO_INTR_SET_TR_AP_DETECT_ERROR_Msk  0x80000UL
#define CRYPTO_INTR_SET_TR_RC_DETECT_ERROR_Pos  20UL
#define CRYPTO_INTR_SET_TR_RC_DETECT_ERROR_Msk  0x100000UL
#define CRYPTO_INTR_SET_INSTR_DEV_KEY_ERROR_Pos 21UL
#define CRYPTO_INTR_SET_INSTR_DEV_KEY_ERROR_Msk 0x200000UL
/* CRYPTO.INTR_MASK */
#define CRYPTO_INTR_MASK_INSTR_FF_LEVEL_Pos     0UL
#define CRYPTO_INTR_MASK_INSTR_FF_LEVEL_Msk     0x1UL
#define CRYPTO_INTR_MASK_INSTR_FF_OVERFLOW_Pos  1UL
#define CRYPTO_INTR_MASK_INSTR_FF_OVERFLOW_Msk  0x2UL
#define CRYPTO_INTR_MASK_TR_INITIALIZED_Pos     2UL
#define CRYPTO_INTR_MASK_TR_INITIALIZED_Msk     0x4UL
#define CRYPTO_INTR_MASK_TR_DATA_AVAILABLE_Pos  3UL
#define CRYPTO_INTR_MASK_TR_DATA_AVAILABLE_Msk  0x8UL
#define CRYPTO_INTR_MASK_PR_DATA_AVAILABLE_Pos  4UL
#define CRYPTO_INTR_MASK_PR_DATA_AVAILABLE_Msk  0x10UL
#define CRYPTO_INTR_MASK_INSTR_OPC_ERROR_Pos    16UL
#define CRYPTO_INTR_MASK_INSTR_OPC_ERROR_Msk    0x10000UL
#define CRYPTO_INTR_MASK_INSTR_CC_ERROR_Pos     17UL
#define CRYPTO_INTR_MASK_INSTR_CC_ERROR_Msk     0x20000UL
#define CRYPTO_INTR_MASK_BUS_ERROR_Pos          18UL
#define CRYPTO_INTR_MASK_BUS_ERROR_Msk          0x40000UL
#define CRYPTO_INTR_MASK_TR_AP_DETECT_ERROR_Pos 19UL
#define CRYPTO_INTR_MASK_TR_AP_DETECT_ERROR_Msk 0x80000UL
#define CRYPTO_INTR_MASK_TR_RC_DETECT_ERROR_Pos 20UL
#define CRYPTO_INTR_MASK_TR_RC_DETECT_ERROR_Msk 0x100000UL
#define CRYPTO_INTR_MASK_INSTR_DEV_KEY_ERROR_Pos 21UL
#define CRYPTO_INTR_MASK_INSTR_DEV_KEY_ERROR_Msk 0x200000UL
/* CRYPTO.INTR_MASKED */
#define CRYPTO_INTR_MASKED_INSTR_FF_LEVEL_Pos   0UL
#define CRYPTO_INTR_MASKED_INSTR_FF_LEVEL_Msk   0x1UL
#define CRYPTO_INTR_MASKED_INSTR_FF_OVERFLOW_Pos 1UL
#define CRYPTO_INTR_MASKED_INSTR_FF_OVERFLOW_Msk 0x2UL
#define CRYPTO_INTR_MASKED_TR_INITIALIZED_Pos   2UL
#define CRYPTO_INTR_MASKED_TR_INITIALIZED_Msk   0x4UL
#define CRYPTO_INTR_MASKED_TR_DATA_AVAILABLE_Pos 3UL
#define CRYPTO_INTR_MASKED_TR_DATA_AVAILABLE_Msk 0x8UL
#define CRYPTO_INTR_MASKED_PR_DATA_AVAILABLE_Pos 4UL
#define CRYPTO_INTR_MASKED_PR_DATA_AVAILABLE_Msk 0x10UL
#define CRYPTO_INTR_MASKED_INSTR_OPC_ERROR_Pos  16UL
#define CRYPTO_INTR_MASKED_INSTR_OPC_ERROR_Msk  0x10000UL
#define CRYPTO_INTR_MASKED_INSTR_CC_ERROR_Pos   17UL
#define CRYPTO_INTR_MASKED_INSTR_CC_ERROR_Msk   0x20000UL
#define CRYPTO_INTR_MASKED_BUS_ERROR_Pos        18UL
#define CRYPTO_INTR_MASKED_BUS_ERROR_Msk        0x40000UL
#define CRYPTO_INTR_MASKED_TR_AP_DETECT_ERROR_Pos 19UL
#define CRYPTO_INTR_MASKED_TR_AP_DETECT_ERROR_Msk 0x80000UL
#define CRYPTO_INTR_MASKED_TR_RC_DETECT_ERROR_Pos 20UL
#define CRYPTO_INTR_MASKED_TR_RC_DETECT_ERROR_Msk 0x100000UL
#define CRYPTO_INTR_MASKED_INSTR_DEV_KEY_ERROR_Pos 21UL
#define CRYPTO_INTR_MASKED_INSTR_DEV_KEY_ERROR_Msk 0x200000UL
/* CRYPTO.PR_LFSR_CTL0 */
#define CRYPTO_PR_LFSR_CTL0_LFSR32_Pos          0UL
#define CRYPTO_PR_LFSR_CTL0_LFSR32_Msk          0xFFFFFFFFUL
/* CRYPTO.PR_LFSR_CTL1 */
#define CRYPTO_PR_LFSR_CTL1_LFSR31_Pos          0UL
#define CRYPTO_PR_LFSR_CTL1_LFSR31_Msk          0x7FFFFFFFUL
/* CRYPTO.PR_LFSR_CTL2 */
#define CRYPTO_PR_LFSR_CTL2_LFSR29_Pos          0UL
#define CRYPTO_PR_LFSR_CTL2_LFSR29_Msk          0x1FFFFFFFUL
/* CRYPTO.PR_MAX_CTL */
#define CRYPTO_PR_MAX_CTL_DATA32_Pos            0UL
#define CRYPTO_PR_MAX_CTL_DATA32_Msk            0xFFFFFFFFUL
/* CRYPTO.PR_CMD */
#define CRYPTO_PR_CMD_START_Pos                 0UL
#define CRYPTO_PR_CMD_START_Msk                 0x1UL
/* CRYPTO.PR_RESULT */
#define CRYPTO_PR_RESULT_DATA32_Pos             0UL
#define CRYPTO_PR_RESULT_DATA32_Msk             0xFFFFFFFFUL
/* CRYPTO.TR_CTL0 */
#define CRYPTO_TR_CTL0_SAMPLE_CLOCK_DIV_Pos     0UL
#define CRYPTO_TR_CTL0_SAMPLE_CLOCK_DIV_Msk     0xFFUL
#define CRYPTO_TR_CTL0_RED_CLOCK_DIV_Pos        8UL
#define CRYPTO_TR_CTL0_RED_CLOCK_DIV_Msk        0xFF00UL
#define CRYPTO_TR_CTL0_INIT_DELAY_Pos           16UL
#define CRYPTO_TR_CTL0_INIT_DELAY_Msk           0xFF0000UL
#define CRYPTO_TR_CTL0_VON_NEUMANN_CORR_Pos     24UL
#define CRYPTO_TR_CTL0_VON_NEUMANN_CORR_Msk     0x1000000UL
#define CRYPTO_TR_CTL0_STOP_ON_AP_DETECT_Pos    28UL
#define CRYPTO_TR_CTL0_STOP_ON_AP_DETECT_Msk    0x10000000UL
#define CRYPTO_TR_CTL0_STOP_ON_RC_DETECT_Pos    29UL
#define CRYPTO_TR_CTL0_STOP_ON_RC_DETECT_Msk    0x20000000UL
/* CRYPTO.TR_CTL1 */
#define CRYPTO_TR_CTL1_RO11_EN_Pos              0UL
#define CRYPTO_TR_CTL1_RO11_EN_Msk              0x1UL
#define CRYPTO_TR_CTL1_RO15_EN_Pos              1UL
#define CRYPTO_TR_CTL1_RO15_EN_Msk              0x2UL
#define CRYPTO_TR_CTL1_GARO15_EN_Pos            2UL
#define CRYPTO_TR_CTL1_GARO15_EN_Msk            0x4UL
#define CRYPTO_TR_CTL1_GARO31_EN_Pos            3UL
#define CRYPTO_TR_CTL1_GARO31_EN_Msk            0x8UL
#define CRYPTO_TR_CTL1_FIRO15_EN_Pos            4UL
#define CRYPTO_TR_CTL1_FIRO15_EN_Msk            0x10UL
#define CRYPTO_TR_CTL1_FIRO31_EN_Pos            5UL
#define CRYPTO_TR_CTL1_FIRO31_EN_Msk            0x20UL
/* CRYPTO.TR_CTL2 */
#define CRYPTO_TR_CTL2_SIZE_Pos                 0UL
#define CRYPTO_TR_CTL2_SIZE_Msk                 0x3FUL
/* CRYPTO.TR_STATUS */
#define CRYPTO_TR_STATUS_INITIALIZED_Pos        0UL
#define CRYPTO_TR_STATUS_INITIALIZED_Msk        0x1UL
/* CRYPTO.TR_CMD */
#define CRYPTO_TR_CMD_START_Pos                 0UL
#define CRYPTO_TR_CMD_START_Msk                 0x1UL
/* CRYPTO.TR_RESULT */
#define CRYPTO_TR_RESULT_DATA32_Pos             0UL
#define CRYPTO_TR_RESULT_DATA32_Msk             0xFFFFFFFFUL
/* CRYPTO.TR_GARO_CTL */
#define CRYPTO_TR_GARO_CTL_POLYNOMIAL31_Pos     0UL
#define CRYPTO_TR_GARO_CTL_POLYNOMIAL31_Msk     0x7FFFFFFFUL
/* CRYPTO.TR_FIRO_CTL */
#define CRYPTO_TR_FIRO_CTL_POLYNOMIAL31_Pos     0UL
#define CRYPTO_TR_FIRO_CTL_POLYNOMIAL31_Msk     0x7FFFFFFFUL
/* CRYPTO.TR_MON_CTL */
#define CRYPTO_TR_MON_CTL_BITSTREAM_SEL_Pos     0UL
#define CRYPTO_TR_MON_CTL_BITSTREAM_SEL_Msk     0x3UL
/* CRYPTO.TR_MON_CMD */
#define CRYPTO_TR_MON_CMD_START_AP_Pos          0UL
#define CRYPTO_TR_MON_CMD_START_AP_Msk          0x1UL
#define CRYPTO_TR_MON_CMD_START_RC_Pos          1UL
#define CRYPTO_TR_MON_CMD_START_RC_Msk          0x2UL
/* CRYPTO.TR_MON_RC_CTL */
#define CRYPTO_TR_MON_RC_CTL_CUTOFF_COUNT8_Pos  0UL
#define CRYPTO_TR_MON_RC_CTL_CUTOFF_COUNT8_Msk  0xFFUL
/* CRYPTO.TR_MON_RC_STATUS0 */
#define CRYPTO_TR_MON_RC_STATUS0_BIT_Pos        0UL
#define CRYPTO_TR_MON_RC_STATUS0_BIT_Msk        0x1UL
/* CRYPTO.TR_MON_RC_STATUS1 */
#define CRYPTO_TR_MON_RC_STATUS1_REP_COUNT_Pos  0UL
#define CRYPTO_TR_MON_RC_STATUS1_REP_COUNT_Msk  0xFFUL
/* CRYPTO.TR_MON_AP_CTL */
#define CRYPTO_TR_MON_AP_CTL_CUTOFF_COUNT16_Pos 0UL
#define CRYPTO_TR_MON_AP_CTL_CUTOFF_COUNT16_Msk 0xFFFFUL
#define CRYPTO_TR_MON_AP_CTL_WINDOW_SIZE_Pos    16UL
#define CRYPTO_TR_MON_AP_CTL_WINDOW_SIZE_Msk    0xFFFF0000UL
/* CRYPTO.TR_MON_AP_STATUS0 */
#define CRYPTO_TR_MON_AP_STATUS0_BIT_Pos        0UL
#define CRYPTO_TR_MON_AP_STATUS0_BIT_Msk        0x1UL
/* CRYPTO.TR_MON_AP_STATUS1 */
#define CRYPTO_TR_MON_AP_STATUS1_OCC_COUNT_Pos  0UL
#define CRYPTO_TR_MON_AP_STATUS1_OCC_COUNT_Msk  0xFFFFUL
#define CRYPTO_TR_MON_AP_STATUS1_WINDOW_INDEX_Pos 16UL
#define CRYPTO_TR_MON_AP_STATUS1_WINDOW_INDEX_Msk 0xFFFF0000UL
/* CRYPTO.STATUS */
#define CRYPTO_STATUS_BUSY_Pos                  31UL
#define CRYPTO_STATUS_BUSY_Msk                  0x80000000UL
/* CRYPTO.INSTR_FF_CTL */
#define CRYPTO_INSTR_FF_CTL_EVENT_LEVEL_Pos     0UL
#define CRYPTO_INSTR_FF_CTL_EVENT_LEVEL_Msk     0x7UL
#define CRYPTO_INSTR_FF_CTL_CLEAR_Pos           16UL
#define CRYPTO_INSTR_FF_CTL_CLEAR_Msk           0x10000UL
#define CRYPTO_INSTR_FF_CTL_BLOCK_Pos           17UL
#define CRYPTO_INSTR_FF_CTL_BLOCK_Msk           0x20000UL
/* CRYPTO.INSTR_FF_STATUS */
#define CRYPTO_INSTR_FF_STATUS_USED_Pos         0UL
#define CRYPTO_INSTR_FF_STATUS_USED_Msk         0xFUL
#define CRYPTO_INSTR_FF_STATUS_EVENT_Pos        16UL
#define CRYPTO_INSTR_FF_STATUS_EVENT_Msk        0x10000UL
/* CRYPTO.INSTR_FF_WR */
#define CRYPTO_INSTR_FF_WR_DATA32_Pos           0UL
#define CRYPTO_INSTR_FF_WR_DATA32_Msk           0xFFFFFFFFUL
/* CRYPTO.LOAD0_FF_STATUS */
#define CRYPTO_LOAD0_FF_STATUS_USED5_Pos        0UL
#define CRYPTO_LOAD0_FF_STATUS_USED5_Msk        0x1FUL
#define CRYPTO_LOAD0_FF_STATUS_BUSY_Pos         31UL
#define CRYPTO_LOAD0_FF_STATUS_BUSY_Msk         0x80000000UL
/* CRYPTO.LOAD1_FF_STATUS */
#define CRYPTO_LOAD1_FF_STATUS_USED5_Pos        0UL
#define CRYPTO_LOAD1_FF_STATUS_USED5_Msk        0x1FUL
#define CRYPTO_LOAD1_FF_STATUS_BUSY_Pos         31UL
#define CRYPTO_LOAD1_FF_STATUS_BUSY_Msk         0x80000000UL
/* CRYPTO.STORE_FF_STATUS */
#define CRYPTO_STORE_FF_STATUS_USED5_Pos        0UL
#define CRYPTO_STORE_FF_STATUS_USED5_Msk        0x1FUL
#define CRYPTO_STORE_FF_STATUS_BUSY_Pos         31UL
#define CRYPTO_STORE_FF_STATUS_BUSY_Msk         0x80000000UL
/* CRYPTO.AES_CTL */
#define CRYPTO_AES_CTL_KEY_SIZE_Pos             0UL
#define CRYPTO_AES_CTL_KEY_SIZE_Msk             0x3UL
/* CRYPTO.RESULT */
#define CRYPTO_RESULT_DATA_Pos                  0UL
#define CRYPTO_RESULT_DATA_Msk                  0xFFFFFFFFUL
/* CRYPTO.CRC_CTL */
#define CRYPTO_CRC_CTL_DATA_REVERSE_Pos         0UL
#define CRYPTO_CRC_CTL_DATA_REVERSE_Msk         0x1UL
#define CRYPTO_CRC_CTL_REM_REVERSE_Pos          8UL
#define CRYPTO_CRC_CTL_REM_REVERSE_Msk          0x100UL
/* CRYPTO.CRC_DATA_CTL */
#define CRYPTO_CRC_DATA_CTL_DATA_XOR_Pos        0UL
#define CRYPTO_CRC_DATA_CTL_DATA_XOR_Msk        0xFFUL
/* CRYPTO.CRC_POL_CTL */
#define CRYPTO_CRC_POL_CTL_POLYNOMIAL_Pos       0UL
#define CRYPTO_CRC_POL_CTL_POLYNOMIAL_Msk       0xFFFFFFFFUL
/* CRYPTO.CRC_REM_CTL */
#define CRYPTO_CRC_REM_CTL_REM_XOR_Pos          0UL
#define CRYPTO_CRC_REM_CTL_REM_XOR_Msk          0xFFFFFFFFUL
/* CRYPTO.CRC_REM_RESULT */
#define CRYPTO_CRC_REM_RESULT_REM_Pos           0UL
#define CRYPTO_CRC_REM_RESULT_REM_Msk           0xFFFFFFFFUL
/* CRYPTO.VU_CTL0 */
#define CRYPTO_VU_CTL0_ALWAYS_EXECUTE_Pos       0UL
#define CRYPTO_VU_CTL0_ALWAYS_EXECUTE_Msk       0x1UL
/* CRYPTO.VU_CTL1 */
#define CRYPTO_VU_CTL1_ADDR24_Pos               8UL
#define CRYPTO_VU_CTL1_ADDR24_Msk               0xFFFFFF00UL
/* CRYPTO.VU_CTL2 */
#define CRYPTO_VU_CTL2_MASK_Pos                 8UL
#define CRYPTO_VU_CTL2_MASK_Msk                 0x7F00UL
/* CRYPTO.VU_STATUS */
#define CRYPTO_VU_STATUS_CARRY_Pos              0UL
#define CRYPTO_VU_STATUS_CARRY_Msk              0x1UL
#define CRYPTO_VU_STATUS_EVEN_Pos               1UL
#define CRYPTO_VU_STATUS_EVEN_Msk               0x2UL
#define CRYPTO_VU_STATUS_ZERO_Pos               2UL
#define CRYPTO_VU_STATUS_ZERO_Msk               0x4UL
#define CRYPTO_VU_STATUS_ONE_Pos                3UL
#define CRYPTO_VU_STATUS_ONE_Msk                0x8UL
/* CRYPTO.VU_RF_DATA */
#define CRYPTO_VU_RF_DATA_DATA32_Pos            0UL
#define CRYPTO_VU_RF_DATA_DATA32_Msk            0xFFFFFFFFUL
/* CRYPTO.DEV_KEY_ADDR0_CTL */
#define CRYPTO_DEV_KEY_ADDR0_CTL_VALID_Pos      31UL
#define CRYPTO_DEV_KEY_ADDR0_CTL_VALID_Msk      0x80000000UL
/* CRYPTO.DEV_KEY_ADDR0 */
#define CRYPTO_DEV_KEY_ADDR0_ADDR32_Pos         0UL
#define CRYPTO_DEV_KEY_ADDR0_ADDR32_Msk         0xFFFFFFFFUL
/* CRYPTO.DEV_KEY_ADDR1_CTL */
#define CRYPTO_DEV_KEY_ADDR1_CTL_VALID_Pos      31UL
#define CRYPTO_DEV_KEY_ADDR1_CTL_VALID_Msk      0x80000000UL
/* CRYPTO.DEV_KEY_ADDR1 */
#define CRYPTO_DEV_KEY_ADDR1_ADDR32_Pos         0UL
#define CRYPTO_DEV_KEY_ADDR1_ADDR32_Msk         0xFFFFFFFFUL
/* CRYPTO.DEV_KEY_STATUS */
#define CRYPTO_DEV_KEY_STATUS_LOADED_Pos        0UL
#define CRYPTO_DEV_KEY_STATUS_LOADED_Msk        0x1UL
/* CRYPTO.DEV_KEY_CTL0 */
#define CRYPTO_DEV_KEY_CTL0_ALLOWED_Pos         0UL
#define CRYPTO_DEV_KEY_CTL0_ALLOWED_Msk         0x1UL
/* CRYPTO.DEV_KEY_CTL1 */
#define CRYPTO_DEV_KEY_CTL1_ALLOWED_Pos         0UL
#define CRYPTO_DEV_KEY_CTL1_ALLOWED_Msk         0x1UL
/* CRYPTO.MEM_BUFF */
#define CRYPTO_MEM_BUFF_DATA32_Pos              0UL
#define CRYPTO_MEM_BUFF_DATA32_Msk              0xFFFFFFFFUL


#endif /* _CYIP_CRYPTO_H_ */


/* [] END OF FILE */
