/***************************************************************************//**
* \file cyip_flashc.h
*
* \brief
* FLASHC IP definitions
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

#ifndef _CYIP_FLASHC_H_
#define _CYIP_FLASHC_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    FLASHC
*******************************************************************************/

#define FLASHC_FM_CTL_ECT_SECTION_SIZE          0x00001000UL
#define FLASHC_SECTION_SIZE                     0x00010000UL

/**
  * \brief Flash Macro Control (FLASHC_FM_CTL)
  */
typedef struct stc_FLASHC_FM_CTL_field {
  uint32_t                         u5FM_MODE:5;
  uint32_t                         :26;
  uint32_t                         u1EMB_START:1;
} stc_FLASHC_FM_CTL_field_t;

typedef union un_FLASHC_FM_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_FM_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_CTL_t;

/**
  * \brief Flash Macro Margin Mode on Code Flash (FLASHC_FM_CODE_MARGIN)
  */
typedef struct stc_FLASHC_FM_CODE_MARGIN_field {
  uint32_t                         u9MARGIN_DCS_TRIM:9;
  uint32_t                         u1MARGIN_DCS_TRIM_EN:1;
  uint32_t                         u6MARGIN_RDREG_TRIM:6;
  uint32_t                         :13;
  uint32_t                         u1MARGIN_PGM_ERS_B:1;
  uint32_t                         u1MARGIN_MODE_RDREG_CHNG_EN:1;
  uint32_t                         u1MARGIN_MODE_EN:1;
} stc_FLASHC_FM_CODE_MARGIN_field_t;

typedef union un_FLASHC_FM_CODE_MARGIN {
  uint32_t                         u32Register;
  stc_FLASHC_FM_CODE_MARGIN_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_CODE_MARGIN_t;

/**
  * \brief Flash Macro Address (FLASHC_FM_ADDR)
  */
typedef struct stc_FLASHC_FM_ADDR_field {
  uint32_t                         u32FM_ADDR:32;
} stc_FLASHC_FM_ADDR_field_t;

typedef union un_FLASHC_FM_ADDR {
  uint32_t                         u32Register;
  stc_FLASHC_FM_ADDR_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_ADDR_t;

/**
  * \brief Interrupt (FLASHC_INTR)
  */
typedef struct stc_FLASHC_INTR_field {
  uint32_t                         u1INTR:1;
  uint32_t                         :31;
} stc_FLASHC_INTR_field_t;

typedef union un_FLASHC_INTR {
  uint32_t                         u32Register;
  stc_FLASHC_INTR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_INTR_t;

/**
  * \brief Interrupt Set (FLASHC_INTR_SET)
  */
typedef struct stc_FLASHC_INTR_SET_field {
  uint32_t                         u1INTR_SET:1;
  uint32_t                         :31;
} stc_FLASHC_INTR_SET_field_t;

typedef union un_FLASHC_INTR_SET {
  uint32_t                         u32Register;
  stc_FLASHC_INTR_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_INTR_SET_t;

/**
  * \brief Interrupt Mask (FLASHC_INTR_MASK)
  */
typedef struct stc_FLASHC_INTR_MASK_field {
  uint32_t                         u1INTR_MASK:1;
  uint32_t                         :31;
} stc_FLASHC_INTR_MASK_field_t;

typedef union un_FLASHC_INTR_MASK {
  uint32_t                         u32Register;
  stc_FLASHC_INTR_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_INTR_MASK_t;

/**
  * \brief Interrupt Masked (FLASHC_INTR_MASKED)
  */
typedef struct stc_FLASHC_INTR_MASKED_field {
  uint32_t                         u1INTR_MASKED:1;
  uint32_t                         :31;
} stc_FLASHC_INTR_MASKED_field_t;

typedef union un_FLASHC_INTR_MASKED {
  uint32_t                         u32Register;
  stc_FLASHC_INTR_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_INTR_MASKED_t;

/**
  * \brief ECC Data In override information and control bits (FLASHC_ECC_OVERRIDE)
  */
typedef struct stc_FLASHC_ECC_OVERRIDE_field {
  uint32_t                         u8ECC_OVERRIDE_SYNDROME:8;
  uint32_t                         :22;
  uint32_t                         u1ECC_OVERRIDE_WORK:1;
  uint32_t                         u1ECC_OVERRIDE_CODE:1;
} stc_FLASHC_ECC_OVERRIDE_field_t;

typedef union un_FLASHC_ECC_OVERRIDE {
  uint32_t                         u32Register;
  stc_FLASHC_ECC_OVERRIDE_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_ECC_OVERRIDE_t;

/**
  * \brief Flash macro data_in[31 to 0] both Code and Work Flash (FLASHC_FM_DATA)
  */
typedef struct stc_FLASHC_FM_DATA_field {
  uint32_t                         u32FM_DATA:32;
} stc_FLASHC_FM_DATA_field_t;

typedef union un_FLASHC_FM_DATA {
  uint32_t                         u32Register;
  stc_FLASHC_FM_DATA_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_DATA_t;

/**
  * \brief Bookmark register - keeps the current FW HV seq (FLASHC_BOOKMARK)
  */
typedef struct stc_FLASHC_BOOKMARK_field {
  uint32_t                         u32BOOKMARK:32;
} stc_FLASHC_BOOKMARK_field_t;

typedef union un_FLASHC_BOOKMARK {
  uint32_t                         u32Register;
  stc_FLASHC_BOOKMARK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_BOOKMARK_t;

/**
  * \brief Main (Code) Flash Security enable (FLASHC_MAIN_FLASH_SAFETY)
  */
typedef struct stc_FLASHC_MAIN_FLASH_SAFETY_field {
  uint32_t                         u1MAINFLASHWRITEENABLE:1;
  uint32_t                         :31;
} stc_FLASHC_MAIN_FLASH_SAFETY_field_t;

typedef union un_FLASHC_MAIN_FLASH_SAFETY {
  uint32_t                         u32Register;
  stc_FLASHC_MAIN_FLASH_SAFETY_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_MAIN_FLASH_SAFETY_t;

/**
  * \brief Status read from Flash Macro (FLASHC_STATUS)
  */
typedef struct stc_FLASHC_STATUS_field {
  uint32_t                         u1PGM_CODE:1;
  uint32_t                         u1PGM_WORK:1;
  uint32_t                         u1ERASE_CODE:1;
  uint32_t                         u1ERASE_WORK:1;
  uint32_t                         u1ERS_SUSPEND:1;
  uint32_t                         u1BLANK_CHECK_WORK:1;
  uint32_t                         u1BLANK_CHCEK_PASS:1;
  uint32_t                         :20;
  uint32_t                         u1POR_1B_ECC_CORRECTED:1;
  uint32_t                         u1POR_2B_ECC_ERROR:1;
  uint32_t                         u1NATIVE_POR:1;
  uint32_t                         u1HANG:1;
  uint32_t                         u1BUSY:1;
} stc_FLASHC_STATUS_field_t;

typedef union un_FLASHC_STATUS {
  uint32_t                         u32Register;
  stc_FLASHC_STATUS_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_STATUS_t;

/**
  * \brief Work Flash Security enable (FLASHC_WORK_FLASH_SAFETY)
  */
typedef struct stc_FLASHC_WORK_FLASH_SAFETY_field {
  uint32_t                         u1WORKFLASHWRITEENABLE:1;
  uint32_t                         :31;
} stc_FLASHC_WORK_FLASH_SAFETY_field_t;

typedef union un_FLASHC_WORK_FLASH_SAFETY {
  uint32_t                         u32Register;
  stc_FLASHC_WORK_FLASH_SAFETY_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_WORK_FLASH_SAFETY_t;



/**
  * \brief Control (FLASHC_FLASH_CTL)
  */
typedef struct stc_FLASHC_FLASH_CTL_field {
  uint32_t                         u4WS:4;
  uint32_t                         :4;
  uint32_t                         u1MAIN_MAP:1;
  uint32_t                         u1WORK_MAP:1;
  uint32_t                         :2;
  uint32_t                         u1MAIN_BANK_MODE:1;
  uint32_t                         u1WORK_BANK_MODE:1;
  uint32_t                         :2;
  uint32_t                         u1MAIN_ECC_EN:1;
  uint32_t                         u1MAIN_ECC_INJ_EN:1;
  uint32_t                         u1MAIN_ERR_SILENT:1;
  uint32_t                         :1;
  uint32_t                         u1WORK_ECC_EN:1;
  uint32_t                         u1WORK_ECC_INJ_EN:1;
  uint32_t                         u1WORK_ERR_SILENT:1;
  uint32_t                         :1;
  uint32_t                         u1WORK_SEQ_RD_EN:1;
  uint32_t                         :7;
} stc_FLASHC_FLASH_CTL_field_t;

typedef union un_FLASHC_FLASH_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_FLASH_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FLASH_CTL_t;

/**
  * \brief Flash power control (FLASHC_FLASH_PWR_CTL)
  */
typedef struct stc_FLASHC_FLASH_PWR_CTL_field {
  uint32_t                         u1ENABLE:1;
  uint32_t                         u1ENABLE_HV:1;
  uint32_t                         :30;
} stc_FLASHC_FLASH_PWR_CTL_field_t;

typedef union un_FLASHC_FLASH_PWR_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_FLASH_PWR_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FLASH_PWR_CTL_t;

/**
  * \brief Command (FLASHC_FLASH_CMD)
  */
typedef struct stc_FLASHC_FLASH_CMD_field {
  uint32_t                         u1INV:1;
  uint32_t                         u1BUFF_INV:1;
  uint32_t                         :30;
} stc_FLASHC_FLASH_CMD_field_t;

typedef union un_FLASHC_FLASH_CMD {
  uint32_t                         u32Register;
  stc_FLASHC_FLASH_CMD_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FLASH_CMD_t;

/**
  * \brief ECC control (FLASHC_ECC_CTL)
  */
typedef struct stc_FLASHC_ECC_CTL_field {
  uint32_t                         u24WORD_ADDR:24;
  uint32_t                         u8PARITY:8;
} stc_FLASHC_ECC_CTL_field_t;

typedef union un_FLASHC_ECC_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_ECC_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_ECC_CTL_t;

/**
  * \brief eCT Flash SRAM ECC control 0 (FLASHC_FM_SRAM_ECC_CTL0)
  */
typedef struct stc_FLASHC_FM_SRAM_ECC_CTL0_field {
  uint32_t                         u32ECC_INJ_DATA:32;
} stc_FLASHC_FM_SRAM_ECC_CTL0_field_t;

typedef union un_FLASHC_FM_SRAM_ECC_CTL0 {
  uint32_t                         u32Register;
  stc_FLASHC_FM_SRAM_ECC_CTL0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_SRAM_ECC_CTL0_t;

/**
  * \brief eCT Flash SRAM ECC control 1 (FLASHC_FM_SRAM_ECC_CTL1)
  */
typedef struct stc_FLASHC_FM_SRAM_ECC_CTL1_field {
  uint32_t                         u7ECC_INJ_PARITY:7;
  uint32_t                         :25;
} stc_FLASHC_FM_SRAM_ECC_CTL1_field_t;

typedef union un_FLASHC_FM_SRAM_ECC_CTL1 {
  uint32_t                         u32Register;
  stc_FLASHC_FM_SRAM_ECC_CTL1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_SRAM_ECC_CTL1_t;

/**
  * \brief eCT Flash SRAM ECC control 2 (FLASHC_FM_SRAM_ECC_CTL2)
  */
typedef struct stc_FLASHC_FM_SRAM_ECC_CTL2_field {
  uint32_t                         u32CORRECTED_DATA:32;
} stc_FLASHC_FM_SRAM_ECC_CTL2_field_t;

typedef union un_FLASHC_FM_SRAM_ECC_CTL2 {
  uint32_t                         u32Register;
  stc_FLASHC_FM_SRAM_ECC_CTL2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_SRAM_ECC_CTL2_t;

/**
  * \brief eCT Flash SRAM ECC control 3 (FLASHC_FM_SRAM_ECC_CTL3)
  */
typedef struct stc_FLASHC_FM_SRAM_ECC_CTL3_field {
  uint32_t                         u1ECC_ENABLE:1;
  uint32_t                         :3;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :3;
  uint32_t                         u1ECC_TEST_FAIL:1;
  uint32_t                         :23;
} stc_FLASHC_FM_SRAM_ECC_CTL3_field_t;

typedef union un_FLASHC_FM_SRAM_ECC_CTL3 {
  uint32_t                         u32Register;
  stc_FLASHC_FM_SRAM_ECC_CTL3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_FM_SRAM_ECC_CTL3_t;

/**
  * \brief CM0+ cache control (FLASHC_CM0_CA_CTL0)
  */
typedef struct stc_FLASHC_CM0_CA_CTL0_field {
  uint32_t                         u1RAM_ECC_EN:1;
  uint32_t                         u1RAM_ECC_INJ_EN:1;
  uint32_t                         :14;
  uint32_t                         u2WAY:2;
  uint32_t                         :6;
  uint32_t                         u3SET_ADDR:3;
  uint32_t                         :3;
  uint32_t                         u1PREF_EN:1;
  uint32_t                         u1CA_EN:1;
} stc_FLASHC_CM0_CA_CTL0_field_t;

typedef union un_FLASHC_CM0_CA_CTL0 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_CTL0_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_CTL0_t;

/**
  * \brief CM0+ cache control (FLASHC_CM0_CA_CTL1)
  */
typedef struct stc_FLASHC_CM0_CA_CTL1_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_FLASHC_CM0_CA_CTL1_field_t;

typedef union un_FLASHC_CM0_CA_CTL1 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_CTL1_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_CTL1_t;

/**
  * \brief CM0+ cache control (FLASHC_CM0_CA_CTL2)
  */
typedef struct stc_FLASHC_CM0_CA_CTL2_field {
  uint32_t                         u10PWRUP_DELAY:10;
  uint32_t                         :22;
} stc_FLASHC_CM0_CA_CTL2_field_t;

typedef union un_FLASHC_CM0_CA_CTL2 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_CTL2_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_CTL2_t;

/**
  * \brief CM0+ cache status 0 (FLASHC_CM0_CA_STATUS0)
  */
typedef struct stc_FLASHC_CM0_CA_STATUS0_field {
  uint32_t                         u32VALID32:32;
} stc_FLASHC_CM0_CA_STATUS0_field_t;

typedef union un_FLASHC_CM0_CA_STATUS0 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_STATUS0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_STATUS0_t;

/**
  * \brief CM0+ cache status 1 (FLASHC_CM0_CA_STATUS1)
  */
typedef struct stc_FLASHC_CM0_CA_STATUS1_field {
  uint32_t                         u32TAG:32;
} stc_FLASHC_CM0_CA_STATUS1_field_t;

typedef union un_FLASHC_CM0_CA_STATUS1 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_STATUS1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_STATUS1_t;

/**
  * \brief CM0+ cache status 2 (FLASHC_CM0_CA_STATUS2)
  */
typedef struct stc_FLASHC_CM0_CA_STATUS2_field {
  uint32_t                         u6LRU:6;
  uint32_t                         :26;
} stc_FLASHC_CM0_CA_STATUS2_field_t;

typedef union un_FLASHC_CM0_CA_STATUS2 {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_CA_STATUS2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_CA_STATUS2_t;

/**
  * \brief CM0+ interface status (FLASHC_CM0_STATUS)
  */
typedef struct stc_FLASHC_CM0_STATUS_field {
  uint32_t                         u1MAIN_INTERNAL_ERR:1;
  uint32_t                         u1WORK_INTERNAL_ERR:1;
  uint32_t                         :30;
} stc_FLASHC_CM0_STATUS_field_t;

typedef union un_FLASHC_CM0_STATUS {
  uint32_t                         u32Register;
  stc_FLASHC_CM0_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM0_STATUS_t;

/**
  * \brief CM7 #0 interface status (FLASHC_CM7_0_STATUS)
  */
typedef struct stc_FLASHC_CM7_0_STATUS_field {
  uint32_t                         u1MAIN_INTERNAL_ERR:1;
  uint32_t                         u1WORK_INTERNAL_ERR:1;
  uint32_t                         :30;
} stc_FLASHC_CM7_0_STATUS_field_t;

typedef union un_FLASHC_CM7_0_STATUS {
  uint32_t                         u32Register;
  stc_FLASHC_CM7_0_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM7_0_STATUS_t;

/**
  * \brief CM7 #1 interface status (FLASHC_CM7_1_STATUS)
  */
typedef struct stc_FLASHC_CM7_1_STATUS_field {
  uint32_t                         u1MAIN_INTERNAL_ERR:1;
  uint32_t                         u1WORK_INTERNAL_ERR:1;
  uint32_t                         :30;
} stc_FLASHC_CM7_1_STATUS_field_t;

typedef union un_FLASHC_CM7_1_STATUS {
  uint32_t                         u32Register;
  stc_FLASHC_CM7_1_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CM7_1_STATUS_t;

/**
  * \brief Cryptography buffer control (FLASHC_CRYPTO_BUFF_CTL)
  */
typedef struct stc_FLASHC_CRYPTO_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_CRYPTO_BUFF_CTL_field_t;

typedef union un_FLASHC_CRYPTO_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_CRYPTO_BUFF_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_CRYPTO_BUFF_CTL_t;

/**
  * \brief Datawire 0 buffer control (FLASHC_DW0_BUFF_CTL)
  */
typedef struct stc_FLASHC_DW0_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_DW0_BUFF_CTL_field_t;

typedef union un_FLASHC_DW0_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_DW0_BUFF_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_DW0_BUFF_CTL_t;

/**
  * \brief Datawire 1 buffer control (FLASHC_DW1_BUFF_CTL)
  */
typedef struct stc_FLASHC_DW1_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_DW1_BUFF_CTL_field_t;

typedef union un_FLASHC_DW1_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_DW1_BUFF_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_DW1_BUFF_CTL_t;

/**
  * \brief DMA controller buffer control (FLASHC_DMAC_BUFF_CTL)
  */
typedef struct stc_FLASHC_DMAC_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_DMAC_BUFF_CTL_field_t;

typedef union un_FLASHC_DMAC_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_DMAC_BUFF_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_DMAC_BUFF_CTL_t;

/**
  * \brief Slow external master 0 buffer control (FLASHC_SLOW0_MS_BUFF_CTL)
  */
typedef struct stc_FLASHC_SLOW0_MS_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_SLOW0_MS_BUFF_CTL_field_t;

typedef union un_FLASHC_SLOW0_MS_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_SLOW0_MS_BUFF_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_SLOW0_MS_BUFF_CTL_t;

/**
  * \brief Slow external master 1 buffer control (FLASHC_SLOW1_MS_BUFF_CTL)
  */
typedef struct stc_FLASHC_SLOW1_MS_BUFF_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1PREF_EN:1;
} stc_FLASHC_SLOW1_MS_BUFF_CTL_field_t;

typedef union un_FLASHC_SLOW1_MS_BUFF_CTL {
  uint32_t                         u32Register;
  stc_FLASHC_SLOW1_MS_BUFF_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FLASHC_SLOW1_MS_BUFF_CTL_t;



/**
  * \brief Flash Macro Registers (FLASHC_FM_CTL_ECT)
  */
typedef struct stc_FLASHC_FM_CTL_ECT {
  un_FLASHC_FM_CTL_t               unFM_CTL;           /*!< 0x00000000 Flash Macro Control */
  un_FLASHC_FM_CODE_MARGIN_t       unFM_CODE_MARGIN;   /*!< 0x00000004 Flash Macro Margin Mode on Code Flash */
  un_FLASHC_FM_ADDR_t              unFM_ADDR;          /*!< 0x00000008 Flash Macro Address */
  uint32_t                         au32Reserved[5];
  un_FLASHC_INTR_t                 unINTR;             /*!< 0x00000020 Interrupt */
  un_FLASHC_INTR_SET_t             unINTR_SET;         /*!< 0x00000024 Interrupt Set */
  un_FLASHC_INTR_MASK_t            unINTR_MASK;        /*!< 0x00000028 Interrupt Mask */
  un_FLASHC_INTR_MASKED_t          unINTR_MASKED;      /*!< 0x0000002C Interrupt Masked */
  un_FLASHC_ECC_OVERRIDE_t         unECC_OVERRIDE;     /*!< 0x00000030 ECC Data In override information and control bits */
  uint32_t                         au32Reserved1[3];
  un_FLASHC_FM_DATA_t              unFM_DATA;          /*!< 0x00000040 Flash macro data_in[31 to 0] both Code and Work Flash */
  uint32_t                         au32Reserved2[8];
  un_FLASHC_BOOKMARK_t             unBOOKMARK;         /*!< 0x00000064 Bookmark register - keeps the current FW HV seq */
  uint32_t                         au32Reserved3[230];
  un_FLASHC_MAIN_FLASH_SAFETY_t    unMAIN_FLASH_SAFETY; /*!< 0x00000400 Main (Code) Flash Security enable */
  un_FLASHC_STATUS_t               unSTATUS;           /*!< 0x00000404 Status read from Flash Macro */
  uint32_t                         au32Reserved4[62];
  un_FLASHC_WORK_FLASH_SAFETY_t    unWORK_FLASH_SAFETY; /*!< 0x00000500 Work Flash Security enable */
  uint32_t                         au32Reserved5[703];
} stc_FLASHC_FM_CTL_ECT_t;                             /*!< Size = 4096 (0x1000) */

/**
  * \brief Flash controller (FLASHC)
  */
typedef struct stc_FLASHC {
  un_FLASHC_FLASH_CTL_t            unFLASH_CTL;        /*!< 0x00000000 Control */
  un_FLASHC_FLASH_PWR_CTL_t        unFLASH_PWR_CTL;    /*!< 0x00000004 Flash power control */
  un_FLASHC_FLASH_CMD_t            unFLASH_CMD;        /*!< 0x00000008 Command */
  uint32_t                         au32Reserved[165];
  un_FLASHC_ECC_CTL_t              unECC_CTL;          /*!< 0x000002A0 ECC control */
  uint32_t                         au32Reserved1[3];
  un_FLASHC_FM_SRAM_ECC_CTL0_t     unFM_SRAM_ECC_CTL0; /*!< 0x000002B0 eCT Flash SRAM ECC control 0 */
  un_FLASHC_FM_SRAM_ECC_CTL1_t     unFM_SRAM_ECC_CTL1; /*!< 0x000002B4 eCT Flash SRAM ECC control 1 */
  un_FLASHC_FM_SRAM_ECC_CTL2_t     unFM_SRAM_ECC_CTL2; /*!< 0x000002B8 eCT Flash SRAM ECC control 2 */
  un_FLASHC_FM_SRAM_ECC_CTL3_t     unFM_SRAM_ECC_CTL3; /*!< 0x000002BC eCT Flash SRAM ECC control 3 */
  uint32_t                         au32Reserved2[80];
  un_FLASHC_CM0_CA_CTL0_t          unCM0_CA_CTL0;      /*!< 0x00000400 CM0+ cache control */
  un_FLASHC_CM0_CA_CTL1_t          unCM0_CA_CTL1;      /*!< 0x00000404 CM0+ cache control */
  un_FLASHC_CM0_CA_CTL2_t          unCM0_CA_CTL2;      /*!< 0x00000408 CM0+ cache control */
  uint32_t                         au32Reserved3[13];
  un_FLASHC_CM0_CA_STATUS0_t       unCM0_CA_STATUS0;   /*!< 0x00000440 CM0+ cache status 0 */
  un_FLASHC_CM0_CA_STATUS1_t       unCM0_CA_STATUS1;   /*!< 0x00000444 CM0+ cache status 1 */
  un_FLASHC_CM0_CA_STATUS2_t       unCM0_CA_STATUS2;   /*!< 0x00000448 CM0+ cache status 2 */
  uint32_t                         au32Reserved4[5];
  un_FLASHC_CM0_STATUS_t           unCM0_STATUS;       /*!< 0x00000460 CM0+ interface status */
  uint32_t                         au32Reserved5[31];
  un_FLASHC_CM7_0_STATUS_t         unCM7_0_STATUS;     /*!< 0x000004E0 CM7 #0 interface status */
  uint32_t                         au32Reserved6[31];
  un_FLASHC_CM7_1_STATUS_t         unCM7_1_STATUS;     /*!< 0x00000560 CM7 #1 interface status */
  uint32_t                         au32Reserved7[7];
  un_FLASHC_CRYPTO_BUFF_CTL_t      unCRYPTO_BUFF_CTL;  /*!< 0x00000580 Cryptography buffer control */
  uint32_t                         au32Reserved8[31];
  un_FLASHC_DW0_BUFF_CTL_t         unDW0_BUFF_CTL;     /*!< 0x00000600 Datawire 0 buffer control */
  uint32_t                         au32Reserved9[31];
  un_FLASHC_DW1_BUFF_CTL_t         unDW1_BUFF_CTL;     /*!< 0x00000680 Datawire 1 buffer control */
  uint32_t                         au32Reserved10[31];
  un_FLASHC_DMAC_BUFF_CTL_t        unDMAC_BUFF_CTL;    /*!< 0x00000700 DMA controller buffer control */
  uint32_t                         au32Reserved11[31];
  un_FLASHC_SLOW0_MS_BUFF_CTL_t    unSLOW0_MS_BUFF_CTL; /*!< 0x00000780 Slow external master 0 buffer control */
  uint32_t                         au32Reserved12[31];
  un_FLASHC_SLOW1_MS_BUFF_CTL_t    unSLOW1_MS_BUFF_CTL; /*!< 0x00000800 Slow external master 1 buffer control */
  uint32_t                         au32Reserved13[14847];
  stc_FLASHC_FM_CTL_ECT_t          FM_CTL_ECT;         /*!< 0x0000F000 Flash Macro Registers */
} stc_FLASHC_t;                                        /*!< Size = 65536 (0x10000) */


/* FLASHC_FM_CTL_ECT.FM_CTL */
#define FLASHC_FM_CTL_ECT_FM_CTL_FM_MODE_Pos    0UL
#define FLASHC_FM_CTL_ECT_FM_CTL_FM_MODE_Msk    0x1FUL
#define FLASHC_FM_CTL_ECT_FM_CTL_EMB_START_Pos  31UL
#define FLASHC_FM_CTL_ECT_FM_CTL_EMB_START_Msk  0x80000000UL
/* FLASHC_FM_CTL_ECT.FM_CODE_MARGIN */
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_DCS_TRIM_Pos 0UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_DCS_TRIM_Msk 0x1FFUL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_DCS_TRIM_EN_Pos 9UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_DCS_TRIM_EN_Msk 0x200UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_RDREG_TRIM_Pos 10UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_RDREG_TRIM_Msk 0xFC00UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_PGM_ERS_B_Pos 29UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_PGM_ERS_B_Msk 0x20000000UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_MODE_RDREG_CHNG_EN_Pos 30UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_MODE_RDREG_CHNG_EN_Msk 0x40000000UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_MODE_EN_Pos 31UL
#define FLASHC_FM_CTL_ECT_FM_CODE_MARGIN_MARGIN_MODE_EN_Msk 0x80000000UL
/* FLASHC_FM_CTL_ECT.FM_ADDR */
#define FLASHC_FM_CTL_ECT_FM_ADDR_FM_ADDR_Pos   0UL
#define FLASHC_FM_CTL_ECT_FM_ADDR_FM_ADDR_Msk   0xFFFFFFFFUL
/* FLASHC_FM_CTL_ECT.INTR */
#define FLASHC_FM_CTL_ECT_INTR_INTR_Pos         0UL
#define FLASHC_FM_CTL_ECT_INTR_INTR_Msk         0x1UL
/* FLASHC_FM_CTL_ECT.INTR_SET */
#define FLASHC_FM_CTL_ECT_INTR_SET_INTR_SET_Pos 0UL
#define FLASHC_FM_CTL_ECT_INTR_SET_INTR_SET_Msk 0x1UL
/* FLASHC_FM_CTL_ECT.INTR_MASK */
#define FLASHC_FM_CTL_ECT_INTR_MASK_INTR_MASK_Pos 0UL
#define FLASHC_FM_CTL_ECT_INTR_MASK_INTR_MASK_Msk 0x1UL
/* FLASHC_FM_CTL_ECT.INTR_MASKED */
#define FLASHC_FM_CTL_ECT_INTR_MASKED_INTR_MASKED_Pos 0UL
#define FLASHC_FM_CTL_ECT_INTR_MASKED_INTR_MASKED_Msk 0x1UL
/* FLASHC_FM_CTL_ECT.ECC_OVERRIDE */
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_SYNDROME_Pos 0UL
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_SYNDROME_Msk 0xFFUL
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_WORK_Pos 30UL
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_WORK_Msk 0x40000000UL
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_CODE_Pos 31UL
#define FLASHC_FM_CTL_ECT_ECC_OVERRIDE_ECC_OVERRIDE_CODE_Msk 0x80000000UL
/* FLASHC_FM_CTL_ECT.FM_DATA */
#define FLASHC_FM_CTL_ECT_FM_DATA_FM_DATA_Pos   0UL
#define FLASHC_FM_CTL_ECT_FM_DATA_FM_DATA_Msk   0xFFFFFFFFUL
/* FLASHC_FM_CTL_ECT.BOOKMARK */
#define FLASHC_FM_CTL_ECT_BOOKMARK_BOOKMARK_Pos 0UL
#define FLASHC_FM_CTL_ECT_BOOKMARK_BOOKMARK_Msk 0xFFFFFFFFUL
/* FLASHC_FM_CTL_ECT.MAIN_FLASH_SAFETY */
#define FLASHC_FM_CTL_ECT_MAIN_FLASH_SAFETY_MAINFLASHWRITEENABLE_Pos 0UL
#define FLASHC_FM_CTL_ECT_MAIN_FLASH_SAFETY_MAINFLASHWRITEENABLE_Msk 0x1UL
/* FLASHC_FM_CTL_ECT.STATUS */
#define FLASHC_FM_CTL_ECT_STATUS_PGM_CODE_Pos   0UL
#define FLASHC_FM_CTL_ECT_STATUS_PGM_CODE_Msk   0x1UL
#define FLASHC_FM_CTL_ECT_STATUS_PGM_WORK_Pos   1UL
#define FLASHC_FM_CTL_ECT_STATUS_PGM_WORK_Msk   0x2UL
#define FLASHC_FM_CTL_ECT_STATUS_ERASE_CODE_Pos 2UL
#define FLASHC_FM_CTL_ECT_STATUS_ERASE_CODE_Msk 0x4UL
#define FLASHC_FM_CTL_ECT_STATUS_ERASE_WORK_Pos 3UL
#define FLASHC_FM_CTL_ECT_STATUS_ERASE_WORK_Msk 0x8UL
#define FLASHC_FM_CTL_ECT_STATUS_ERS_SUSPEND_Pos 4UL
#define FLASHC_FM_CTL_ECT_STATUS_ERS_SUSPEND_Msk 0x10UL
#define FLASHC_FM_CTL_ECT_STATUS_BLANK_CHECK_WORK_Pos 5UL
#define FLASHC_FM_CTL_ECT_STATUS_BLANK_CHECK_WORK_Msk 0x20UL
#define FLASHC_FM_CTL_ECT_STATUS_BLANK_CHCEK_PASS_Pos 6UL
#define FLASHC_FM_CTL_ECT_STATUS_BLANK_CHCEK_PASS_Msk 0x40UL
#define FLASHC_FM_CTL_ECT_STATUS_POR_1B_ECC_CORRECTED_Pos 27UL
#define FLASHC_FM_CTL_ECT_STATUS_POR_1B_ECC_CORRECTED_Msk 0x8000000UL
#define FLASHC_FM_CTL_ECT_STATUS_POR_2B_ECC_ERROR_Pos 28UL
#define FLASHC_FM_CTL_ECT_STATUS_POR_2B_ECC_ERROR_Msk 0x10000000UL
#define FLASHC_FM_CTL_ECT_STATUS_NATIVE_POR_Pos 29UL
#define FLASHC_FM_CTL_ECT_STATUS_NATIVE_POR_Msk 0x20000000UL
#define FLASHC_FM_CTL_ECT_STATUS_HANG_Pos       30UL
#define FLASHC_FM_CTL_ECT_STATUS_HANG_Msk       0x40000000UL
#define FLASHC_FM_CTL_ECT_STATUS_BUSY_Pos       31UL
#define FLASHC_FM_CTL_ECT_STATUS_BUSY_Msk       0x80000000UL
/* FLASHC_FM_CTL_ECT.WORK_FLASH_SAFETY */
#define FLASHC_FM_CTL_ECT_WORK_FLASH_SAFETY_WORKFLASHWRITEENABLE_Pos 0UL
#define FLASHC_FM_CTL_ECT_WORK_FLASH_SAFETY_WORKFLASHWRITEENABLE_Msk 0x1UL


/* FLASHC.FLASH_CTL */
#define FLASHC_FLASH_CTL_WS_Pos                 0UL
#define FLASHC_FLASH_CTL_WS_Msk                 0xFUL
#define FLASHC_FLASH_CTL_MAIN_MAP_Pos           8UL
#define FLASHC_FLASH_CTL_MAIN_MAP_Msk           0x100UL
#define FLASHC_FLASH_CTL_WORK_MAP_Pos           9UL
#define FLASHC_FLASH_CTL_WORK_MAP_Msk           0x200UL
#define FLASHC_FLASH_CTL_MAIN_BANK_MODE_Pos     12UL
#define FLASHC_FLASH_CTL_MAIN_BANK_MODE_Msk     0x1000UL
#define FLASHC_FLASH_CTL_WORK_BANK_MODE_Pos     13UL
#define FLASHC_FLASH_CTL_WORK_BANK_MODE_Msk     0x2000UL
#define FLASHC_FLASH_CTL_MAIN_ECC_EN_Pos        16UL
#define FLASHC_FLASH_CTL_MAIN_ECC_EN_Msk        0x10000UL
#define FLASHC_FLASH_CTL_MAIN_ECC_INJ_EN_Pos    17UL
#define FLASHC_FLASH_CTL_MAIN_ECC_INJ_EN_Msk    0x20000UL
#define FLASHC_FLASH_CTL_MAIN_ERR_SILENT_Pos    18UL
#define FLASHC_FLASH_CTL_MAIN_ERR_SILENT_Msk    0x40000UL
#define FLASHC_FLASH_CTL_WORK_ECC_EN_Pos        20UL
#define FLASHC_FLASH_CTL_WORK_ECC_EN_Msk        0x100000UL
#define FLASHC_FLASH_CTL_WORK_ECC_INJ_EN_Pos    21UL
#define FLASHC_FLASH_CTL_WORK_ECC_INJ_EN_Msk    0x200000UL
#define FLASHC_FLASH_CTL_WORK_ERR_SILENT_Pos    22UL
#define FLASHC_FLASH_CTL_WORK_ERR_SILENT_Msk    0x400000UL
#define FLASHC_FLASH_CTL_WORK_SEQ_RD_EN_Pos     24UL
#define FLASHC_FLASH_CTL_WORK_SEQ_RD_EN_Msk     0x1000000UL
/* FLASHC.FLASH_PWR_CTL */
#define FLASHC_FLASH_PWR_CTL_ENABLE_Pos         0UL
#define FLASHC_FLASH_PWR_CTL_ENABLE_Msk         0x1UL
#define FLASHC_FLASH_PWR_CTL_ENABLE_HV_Pos      1UL
#define FLASHC_FLASH_PWR_CTL_ENABLE_HV_Msk      0x2UL
/* FLASHC.FLASH_CMD */
#define FLASHC_FLASH_CMD_INV_Pos                0UL
#define FLASHC_FLASH_CMD_INV_Msk                0x1UL
#define FLASHC_FLASH_CMD_BUFF_INV_Pos           1UL
#define FLASHC_FLASH_CMD_BUFF_INV_Msk           0x2UL
/* FLASHC.ECC_CTL */
#define FLASHC_ECC_CTL_WORD_ADDR_Pos            0UL
#define FLASHC_ECC_CTL_WORD_ADDR_Msk            0xFFFFFFUL
#define FLASHC_ECC_CTL_PARITY_Pos               24UL
#define FLASHC_ECC_CTL_PARITY_Msk               0xFF000000UL
/* FLASHC.FM_SRAM_ECC_CTL0 */
#define FLASHC_FM_SRAM_ECC_CTL0_ECC_INJ_DATA_Pos 0UL
#define FLASHC_FM_SRAM_ECC_CTL0_ECC_INJ_DATA_Msk 0xFFFFFFFFUL
/* FLASHC.FM_SRAM_ECC_CTL1 */
#define FLASHC_FM_SRAM_ECC_CTL1_ECC_INJ_PARITY_Pos 0UL
#define FLASHC_FM_SRAM_ECC_CTL1_ECC_INJ_PARITY_Msk 0x7FUL
/* FLASHC.FM_SRAM_ECC_CTL2 */
#define FLASHC_FM_SRAM_ECC_CTL2_CORRECTED_DATA_Pos 0UL
#define FLASHC_FM_SRAM_ECC_CTL2_CORRECTED_DATA_Msk 0xFFFFFFFFUL
/* FLASHC.FM_SRAM_ECC_CTL3 */
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_ENABLE_Pos  0UL
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_ENABLE_Msk  0x1UL
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_INJ_EN_Pos  4UL
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_INJ_EN_Msk  0x10UL
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_TEST_FAIL_Pos 8UL
#define FLASHC_FM_SRAM_ECC_CTL3_ECC_TEST_FAIL_Msk 0x100UL
/* FLASHC.CM0_CA_CTL0 */
#define FLASHC_CM0_CA_CTL0_RAM_ECC_EN_Pos       0UL
#define FLASHC_CM0_CA_CTL0_RAM_ECC_EN_Msk       0x1UL
#define FLASHC_CM0_CA_CTL0_RAM_ECC_INJ_EN_Pos   1UL
#define FLASHC_CM0_CA_CTL0_RAM_ECC_INJ_EN_Msk   0x2UL
#define FLASHC_CM0_CA_CTL0_WAY_Pos              16UL
#define FLASHC_CM0_CA_CTL0_WAY_Msk              0x30000UL
#define FLASHC_CM0_CA_CTL0_SET_ADDR_Pos         24UL
#define FLASHC_CM0_CA_CTL0_SET_ADDR_Msk         0x7000000UL
#define FLASHC_CM0_CA_CTL0_PREF_EN_Pos          30UL
#define FLASHC_CM0_CA_CTL0_PREF_EN_Msk          0x40000000UL
#define FLASHC_CM0_CA_CTL0_CA_EN_Pos            31UL
#define FLASHC_CM0_CA_CTL0_CA_EN_Msk            0x80000000UL
/* FLASHC.CM0_CA_CTL1 */
#define FLASHC_CM0_CA_CTL1_PWR_MODE_Pos         0UL
#define FLASHC_CM0_CA_CTL1_PWR_MODE_Msk         0x3UL
#define FLASHC_CM0_CA_CTL1_VECTKEYSTAT_Pos      16UL
#define FLASHC_CM0_CA_CTL1_VECTKEYSTAT_Msk      0xFFFF0000UL
/* FLASHC.CM0_CA_CTL2 */
#define FLASHC_CM0_CA_CTL2_PWRUP_DELAY_Pos      0UL
#define FLASHC_CM0_CA_CTL2_PWRUP_DELAY_Msk      0x3FFUL
/* FLASHC.CM0_CA_STATUS0 */
#define FLASHC_CM0_CA_STATUS0_VALID32_Pos       0UL
#define FLASHC_CM0_CA_STATUS0_VALID32_Msk       0xFFFFFFFFUL
/* FLASHC.CM0_CA_STATUS1 */
#define FLASHC_CM0_CA_STATUS1_TAG_Pos           0UL
#define FLASHC_CM0_CA_STATUS1_TAG_Msk           0xFFFFFFFFUL
/* FLASHC.CM0_CA_STATUS2 */
#define FLASHC_CM0_CA_STATUS2_LRU_Pos           0UL
#define FLASHC_CM0_CA_STATUS2_LRU_Msk           0x3FUL
/* FLASHC.CM0_STATUS */
#define FLASHC_CM0_STATUS_MAIN_INTERNAL_ERR_Pos 0UL
#define FLASHC_CM0_STATUS_MAIN_INTERNAL_ERR_Msk 0x1UL
#define FLASHC_CM0_STATUS_WORK_INTERNAL_ERR_Pos 1UL
#define FLASHC_CM0_STATUS_WORK_INTERNAL_ERR_Msk 0x2UL
/* FLASHC.CM7_0_STATUS */
#define FLASHC_CM7_0_STATUS_MAIN_INTERNAL_ERR_Pos 0UL
#define FLASHC_CM7_0_STATUS_MAIN_INTERNAL_ERR_Msk 0x1UL
#define FLASHC_CM7_0_STATUS_WORK_INTERNAL_ERR_Pos 1UL
#define FLASHC_CM7_0_STATUS_WORK_INTERNAL_ERR_Msk 0x2UL
/* FLASHC.CM7_1_STATUS */
#define FLASHC_CM7_1_STATUS_MAIN_INTERNAL_ERR_Pos 0UL
#define FLASHC_CM7_1_STATUS_MAIN_INTERNAL_ERR_Msk 0x1UL
#define FLASHC_CM7_1_STATUS_WORK_INTERNAL_ERR_Pos 1UL
#define FLASHC_CM7_1_STATUS_WORK_INTERNAL_ERR_Msk 0x2UL
/* FLASHC.CRYPTO_BUFF_CTL */
#define FLASHC_CRYPTO_BUFF_CTL_PREF_EN_Pos      30UL
#define FLASHC_CRYPTO_BUFF_CTL_PREF_EN_Msk      0x40000000UL
/* FLASHC.DW0_BUFF_CTL */
#define FLASHC_DW0_BUFF_CTL_PREF_EN_Pos         30UL
#define FLASHC_DW0_BUFF_CTL_PREF_EN_Msk         0x40000000UL
/* FLASHC.DW1_BUFF_CTL */
#define FLASHC_DW1_BUFF_CTL_PREF_EN_Pos         30UL
#define FLASHC_DW1_BUFF_CTL_PREF_EN_Msk         0x40000000UL
/* FLASHC.DMAC_BUFF_CTL */
#define FLASHC_DMAC_BUFF_CTL_PREF_EN_Pos        30UL
#define FLASHC_DMAC_BUFF_CTL_PREF_EN_Msk        0x40000000UL
/* FLASHC.SLOW0_MS_BUFF_CTL */
#define FLASHC_SLOW0_MS_BUFF_CTL_PREF_EN_Pos    30UL
#define FLASHC_SLOW0_MS_BUFF_CTL_PREF_EN_Msk    0x40000000UL
/* FLASHC.SLOW1_MS_BUFF_CTL */
#define FLASHC_SLOW1_MS_BUFF_CTL_PREF_EN_Pos    30UL
#define FLASHC_SLOW1_MS_BUFF_CTL_PREF_EN_Msk    0x40000000UL


#endif /* _CYIP_FLASHC_H_ */


/* [] END OF FILE */
