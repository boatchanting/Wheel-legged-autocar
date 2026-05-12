/***************************************************************************//**
* \file cyip_sdhc.h
*
* \brief
* SDHC IP definitions
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

#ifndef _CYIP_SDHC_H_
#define _CYIP_SDHC_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     SDHC
*******************************************************************************/

#define SDHC_WRAP_SECTION_SIZE                  0x00000020UL
#define SDHC_CORE_SECTION_SIZE                  0x00001000UL
#define SDHC_SECTION_SIZE                       0x00010000UL

/**
  * \brief Top level wrapper control (SDHC_WRAP_CTL)
  */
typedef struct stc_SDHC_WRAP_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLE:1;
} stc_SDHC_WRAP_CTL_field_t;

typedef union un_SDHC_WRAP_CTL {
  uint32_t                         u32Register;
  stc_SDHC_WRAP_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_WRAP_CTL_t;



/**
  * \brief SDMA System Address register (SDHC_CORE_SDMASA_R)
  */
typedef struct stc_SDHC_CORE_SDMASA_R_field {
  uint32_t                         u32BLOCKCNT_SDMASA:32;
} stc_SDHC_CORE_SDMASA_R_field_t;

typedef union un_SDHC_CORE_SDMASA_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_SDMASA_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_SDMASA_R_t;

/**
  * \brief Block Size register (SDHC_CORE_BLOCKSIZE_R)
  */
typedef struct stc_SDHC_CORE_BLOCKSIZE_R_field {
  uint16_t                         u12XFER_BLOCK_SIZE:12;
  uint16_t                         u3SDMA_BUF_BDARY:3;
} stc_SDHC_CORE_BLOCKSIZE_R_field_t;

typedef union un_SDHC_CORE_BLOCKSIZE_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_BLOCKSIZE_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_BLOCKSIZE_R_t;

/**
  * \brief 16-bit Block Count register (SDHC_CORE_BLOCKCOUNT_R)
  */
typedef struct stc_SDHC_CORE_BLOCKCOUNT_R_field {
  uint16_t                         u16BLOCK_CNT:16;
} stc_SDHC_CORE_BLOCKCOUNT_R_field_t;

typedef union un_SDHC_CORE_BLOCKCOUNT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_BLOCKCOUNT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_BLOCKCOUNT_R_t;

/**
  * \brief Argument register (SDHC_CORE_ARGUMENT_R)
  */
typedef struct stc_SDHC_CORE_ARGUMENT_R_field {
  uint32_t                         u32ARGUMENT:32;
} stc_SDHC_CORE_ARGUMENT_R_field_t;

typedef union un_SDHC_CORE_ARGUMENT_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_ARGUMENT_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_ARGUMENT_R_t;

/**
  * \brief Transfer Mode register (SDHC_CORE_XFER_MODE_R)
  */
typedef struct stc_SDHC_CORE_XFER_MODE_R_field {
  uint16_t                         u1DMA_ENABLE:1;
  uint16_t                         u1BLOCK_COUNT_ENABLE:1;
  uint16_t                         u2AUTO_CMD_ENABLE:2;
  uint16_t                         u1DATA_XFER_DIR:1;
  uint16_t                         u1MULTI_BLK_SEL:1;
  uint16_t                         u1RESP_TYPE:1;
  uint16_t                         u1RESP_ERR_CHK_ENABLE:1;
  uint16_t                         u1RESP_INT_DISABLE:1;
  uint16_t                         :7;
} stc_SDHC_CORE_XFER_MODE_R_field_t;

typedef union un_SDHC_CORE_XFER_MODE_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_XFER_MODE_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_XFER_MODE_R_t;

/**
  * \brief Command register (SDHC_CORE_CMD_R)
  */
typedef struct stc_SDHC_CORE_CMD_R_field {
  uint16_t                         u2RESP_TYPE_SELECT:2;
  uint16_t                         u1SUB_CMD_FLAG:1;
  uint16_t                         u1CMD_CRC_CHK_ENABLE:1;
  uint16_t                         u1CMD_IDX_CHK_ENABLE:1;
  uint16_t                         u1DATA_PRESENT_SEL:1;
  uint16_t                         u2CMD_TYPE:2;
  uint16_t                         u6CMD_INDEX:6;
  uint16_t                         :2;
} stc_SDHC_CORE_CMD_R_field_t;

typedef union un_SDHC_CORE_CMD_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_CMD_R_field_t      stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_CMD_R_t;

/**
  * \brief Response Register 0/1 (SDHC_CORE_RESP01_R)
  */
typedef struct stc_SDHC_CORE_RESP01_R_field {
  uint32_t                         u32RESP01:32;
} stc_SDHC_CORE_RESP01_R_field_t;

typedef union un_SDHC_CORE_RESP01_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_RESP01_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_RESP01_R_t;

/**
  * \brief Response Register 2/3 (SDHC_CORE_RESP23_R)
  */
typedef struct stc_SDHC_CORE_RESP23_R_field {
  uint32_t                         u32RESP23:32;
} stc_SDHC_CORE_RESP23_R_field_t;

typedef union un_SDHC_CORE_RESP23_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_RESP23_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_RESP23_R_t;

/**
  * \brief Response Register 4/5 (SDHC_CORE_RESP45_R)
  */
typedef struct stc_SDHC_CORE_RESP45_R_field {
  uint32_t                         u32RESP45:32;
} stc_SDHC_CORE_RESP45_R_field_t;

typedef union un_SDHC_CORE_RESP45_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_RESP45_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_RESP45_R_t;

/**
  * \brief Response Register 6/7 (SDHC_CORE_RESP67_R)
  */
typedef struct stc_SDHC_CORE_RESP67_R_field {
  uint32_t                         u32RESP67:32;
} stc_SDHC_CORE_RESP67_R_field_t;

typedef union un_SDHC_CORE_RESP67_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_RESP67_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_RESP67_R_t;

/**
  * \brief Buffer Data Port Register (SDHC_CORE_BUF_DATA_R)
  */
typedef struct stc_SDHC_CORE_BUF_DATA_R_field {
  uint32_t                         u32BUF_DATA:32;
} stc_SDHC_CORE_BUF_DATA_R_field_t;

typedef union un_SDHC_CORE_BUF_DATA_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_BUF_DATA_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_BUF_DATA_R_t;

/**
  * \brief Present State Register (SDHC_CORE_PSTATE_REG)
  */
typedef struct stc_SDHC_CORE_PSTATE_REG_field {
  uint32_t                         u1CMD_INHIBIT:1;
  uint32_t                         u1CMD_INHIBIT_DAT:1;
  uint32_t                         u1DAT_LINE_ACTIVE:1;
  uint32_t                         :1;
  uint32_t                         u4DAT_7_4:4;
  uint32_t                         u1WR_XFER_ACTIVE:1;
  uint32_t                         u1RD_XFER_ACTIVE:1;
  uint32_t                         u1BUF_WR_ENABLE:1;
  uint32_t                         u1BUF_RD_ENABLE:1;
  uint32_t                         :4;
  uint32_t                         u1CARD_INSERTED:1;
  uint32_t                         u1CARD_STABLE:1;
  uint32_t                         u1CARD_DETECT_PIN_LEVEL:1;
  uint32_t                         u1WR_PROTECT_SW_LVL:1;
  uint32_t                         u4DAT_3_0:4;
  uint32_t                         u1CMD_LINE_LVL:1;
  uint32_t                         u1HOST_REG_VOL:1;
  uint32_t                         :1;
  uint32_t                         u1CMD_ISSU_ERR:1;
  uint32_t                         u1SUB_CMD_STAT:1;
  uint32_t                         :3;
} stc_SDHC_CORE_PSTATE_REG_field_t;

typedef union un_SDHC_CORE_PSTATE_REG {
  uint32_t                         u32Register;
  stc_SDHC_CORE_PSTATE_REG_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_PSTATE_REG_t;

/**
  * \brief Host Control 1 Register (SDHC_CORE_HOST_CTRL1_R)
  */
typedef struct stc_SDHC_CORE_HOST_CTRL1_R_field {
  uint8_t                          u1LED_CTRL:1;
  uint8_t                          u1DAT_XFER_WIDTH:1;
  uint8_t                          u1HIGH_SPEED_EN:1;
  uint8_t                          u2DMA_SEL:2;
  uint8_t                          u1EXT_DAT_XFER:1;
  uint8_t                          u1CARD_DETECT_TEST_LVL:1;
  uint8_t                          u1CARD_DETECT_SIG_SEL:1;
} stc_SDHC_CORE_HOST_CTRL1_R_field_t;

typedef union un_SDHC_CORE_HOST_CTRL1_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_HOST_CTRL1_R_field_t stcField;
} un_SDHC_CORE_HOST_CTRL1_R_t;

/**
  * \brief Power Control Register (SDHC_CORE_PWR_CTRL_R)
  */
typedef struct stc_SDHC_CORE_PWR_CTRL_R_field {
  uint8_t                          u1SD_BUS_PWR_VDD1:1;
  uint8_t                          u3SD_BUS_VOL_VDD1:3;
  uint8_t                          :4;
} stc_SDHC_CORE_PWR_CTRL_R_field_t;

typedef union un_SDHC_CORE_PWR_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_PWR_CTRL_R_field_t stcField;
} un_SDHC_CORE_PWR_CTRL_R_t;

/**
  * \brief Block Gap Control Register (SDHC_CORE_BGAP_CTRL_R)
  */
typedef struct stc_SDHC_CORE_BGAP_CTRL_R_field {
  uint8_t                          u1STOP_BG_REQ:1;
  uint8_t                          u1CONTINUE_REQ:1;
  uint8_t                          u1RD_WAIT_CTRL:1;
  uint8_t                          u1INT_AT_BGAP:1;
  uint8_t                          :4;
} stc_SDHC_CORE_BGAP_CTRL_R_field_t;

typedef union un_SDHC_CORE_BGAP_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_BGAP_CTRL_R_field_t stcField;
} un_SDHC_CORE_BGAP_CTRL_R_t;

/**
  * \brief Wakeup Control Register (SDHC_CORE_WUP_CTRL_R)
  */
typedef struct stc_SDHC_CORE_WUP_CTRL_R_field {
  uint8_t                          u1WUP_CARD_INT:1;
  uint8_t                          u1WUP_CARD_INSERT:1;
  uint8_t                          u1WUP_CARD_REMOVAL:1;
  uint8_t                          :5;
} stc_SDHC_CORE_WUP_CTRL_R_field_t;

typedef union un_SDHC_CORE_WUP_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_WUP_CTRL_R_field_t stcField;
} un_SDHC_CORE_WUP_CTRL_R_t;

/**
  * \brief Clock Control Register (SDHC_CORE_CLK_CTRL_R)
  */
typedef struct stc_SDHC_CORE_CLK_CTRL_R_field {
  uint16_t                         u1INTERNAL_CLK_EN:1;
  uint16_t                         u1INTERNAL_CLK_STABLE:1;
  uint16_t                         u1SD_CLK_EN:1;
  uint16_t                         u1PLL_ENABLE:1;
  uint16_t                         :1;
  uint16_t                         u1CLK_GEN_SELECT:1;
  uint16_t                         u2UPPER_FREQ_SEL:2;
  uint16_t                         u8FREQ_SEL:8;
} stc_SDHC_CORE_CLK_CTRL_R_field_t;

typedef union un_SDHC_CORE_CLK_CTRL_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_CLK_CTRL_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_CLK_CTRL_R_t;

/**
  * \brief Timeout Control Register (SDHC_CORE_TOUT_CTRL_R)
  */
typedef struct stc_SDHC_CORE_TOUT_CTRL_R_field {
  uint8_t                          u4TOUT_CNT:4;
  uint8_t                          :4;
} stc_SDHC_CORE_TOUT_CTRL_R_field_t;

typedef union un_SDHC_CORE_TOUT_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_TOUT_CTRL_R_field_t stcField;
} un_SDHC_CORE_TOUT_CTRL_R_t;

/**
  * \brief Software Reset Register (SDHC_CORE_SW_RST_R)
  */
typedef struct stc_SDHC_CORE_SW_RST_R_field {
  uint8_t                          u1SW_RST_ALL:1;
  uint8_t                          u1SW_RST_CMD:1;
  uint8_t                          u1SW_RST_DAT:1;
  uint8_t                          :5;
} stc_SDHC_CORE_SW_RST_R_field_t;

typedef union un_SDHC_CORE_SW_RST_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_SW_RST_R_field_t   stcField;
} un_SDHC_CORE_SW_RST_R_t;

/**
  * \brief Normal Interrupt Status Register (SDHC_CORE_NORMAL_INT_STAT_R)
  */
typedef struct stc_SDHC_CORE_NORMAL_INT_STAT_R_field {
  uint16_t                         u1CMD_COMPLETE:1;
  uint16_t                         u1XFER_COMPLETE:1;
  uint16_t                         u1BGAP_EVENT:1;
  uint16_t                         u1DMA_INTERRUPT:1;
  uint16_t                         u1BUF_WR_READY:1;
  uint16_t                         u1BUF_RD_READY:1;
  uint16_t                         u1CARD_INSERTION:1;
  uint16_t                         u1CARD_REMOVAL:1;
  uint16_t                         u1CARD_INTERRUPT:1;
  uint16_t                         :4;
  uint16_t                         u1FX_EVENT:1;
  uint16_t                         u1CQE_EVENT:1;
  uint16_t                         u1ERR_INTERRUPT:1;
} stc_SDHC_CORE_NORMAL_INT_STAT_R_field_t;

typedef union un_SDHC_CORE_NORMAL_INT_STAT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_NORMAL_INT_STAT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_NORMAL_INT_STAT_R_t;

/**
  * \brief Error Interrupt Status Register (SDHC_CORE_ERROR_INT_STAT_R)
  */
typedef struct stc_SDHC_CORE_ERROR_INT_STAT_R_field {
  uint16_t                         u1CMD_TOUT_ERR:1;
  uint16_t                         u1CMD_CRC_ERR:1;
  uint16_t                         u1CMD_END_BIT_ERR:1;
  uint16_t                         u1CMD_IDX_ERR:1;
  uint16_t                         u1DATA_TOUT_ERR:1;
  uint16_t                         u1DATA_CRC_ERR:1;
  uint16_t                         u1DATA_END_BIT_ERR:1;
  uint16_t                         u1CUR_LMT_ERR:1;
  uint16_t                         u1AUTO_CMD_ERR:1;
  uint16_t                         u1ADMA_ERR:1;
  uint16_t                         u1TUNING_ERR:1;
  uint16_t                         u1RESP_ERR:1;
  uint16_t                         u1BOOT_ACK_ERR:1;
  uint16_t                         :3;
} stc_SDHC_CORE_ERROR_INT_STAT_R_field_t;

typedef union un_SDHC_CORE_ERROR_INT_STAT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_ERROR_INT_STAT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_ERROR_INT_STAT_R_t;

/**
  * \brief Normal Interrupt Status Enable Register (SDHC_CORE_NORMAL_INT_STAT_EN_R)
  */
typedef struct stc_SDHC_CORE_NORMAL_INT_STAT_EN_R_field {
  uint16_t                         u1CMD_COMPLETE_STAT_EN:1;
  uint16_t                         u1XFER_COMPLETE_STAT_EN:1;
  uint16_t                         u1BGAP_EVENT_STAT_EN:1;
  uint16_t                         u1DMA_INTERRUPT_STAT_EN:1;
  uint16_t                         u1BUF_WR_READY_STAT_EN:1;
  uint16_t                         u1BUF_RD_READY_STAT_EN:1;
  uint16_t                         u1CARD_INSERTION_STAT_EN:1;
  uint16_t                         u1CARD_REMOVAL_STAT_EN:1;
  uint16_t                         u1CARD_INTERRUPT_STAT_EN:1;
  uint16_t                         u1INT_A_STAT_EN:1;
  uint16_t                         u1INT_B_STAT_EN:1;
  uint16_t                         u1INT_C_STAT_EN:1;
  uint16_t                         u1RE_TUNE_EVENT_STAT_EN:1;
  uint16_t                         u1FX_EVENT_STAT_EN:1;
  uint16_t                         u1CQE_EVENT_STAT_EN:1;
} stc_SDHC_CORE_NORMAL_INT_STAT_EN_R_field_t;

typedef union un_SDHC_CORE_NORMAL_INT_STAT_EN_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_NORMAL_INT_STAT_EN_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_NORMAL_INT_STAT_EN_R_t;

/**
  * \brief Error Interrupt Status Enable Register (SDHC_CORE_ERROR_INT_STAT_EN_R)
  */
typedef struct stc_SDHC_CORE_ERROR_INT_STAT_EN_R_field {
  uint16_t                         u1CMD_TOUT_ERR_STAT_EN:1;
  uint16_t                         u1CMD_CRC_ERR_STAT_EN:1;
  uint16_t                         u1CMD_END_BIT_ERR_STAT_EN:1;
  uint16_t                         u1CMD_IDX_ERR_STAT_EN:1;
  uint16_t                         u1DATA_TOUT_ERR_STAT_EN:1;
  uint16_t                         u1DATA_CRC_ERR_STAT_EN:1;
  uint16_t                         u1DATA_END_BIT_ERR_STAT_EN:1;
  uint16_t                         u1CUR_LMT_ERR_STAT_EN:1;
  uint16_t                         u1AUTO_CMD_ERR_STAT_EN:1;
  uint16_t                         u1ADMA_ERR_STAT_EN:1;
  uint16_t                         u1TUNING_ERR_STAT_EN:1;
  uint16_t                         u1RESP_ERR_STAT_EN:1;
  uint16_t                         u1BOOT_ACK_ERR_STAT_EN:1;
  uint16_t                         u1VENDOR_ERR_STAT_EN1:1;
  uint16_t                         u1VENDOR_ERR_STAT_EN2:1;
  uint16_t                         u1VENDOR_ERR_STAT_EN3:1;
} stc_SDHC_CORE_ERROR_INT_STAT_EN_R_field_t;

typedef union un_SDHC_CORE_ERROR_INT_STAT_EN_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_ERROR_INT_STAT_EN_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_ERROR_INT_STAT_EN_R_t;

/**
  * \brief Normal Interrupt Signal Enable Register (SDHC_CORE_NORMAL_INT_SIGNAL_EN_R)
  */
typedef struct stc_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_field {
  uint16_t                         u1CMD_COMPLETE_SIGNAL_EN:1;
  uint16_t                         u1XFER_COMPLETE_SIGNAL_EN:1;
  uint16_t                         u1BGAP_EVENT_SIGNAL_EN:1;
  uint16_t                         u1DMA_INTERRUPT_SIGNAL_EN:1;
  uint16_t                         u1BUF_WR_READY_SIGNAL_EN:1;
  uint16_t                         u1BUF_RD_READY_SIGNAL_EN:1;
  uint16_t                         u1CARD_INSERTION_SIGNAL_EN:1;
  uint16_t                         u1CARD_REMOVAL_SIGNAL_EN:1;
  uint16_t                         u1CARD_INTERRUPT_SIGNAL_EN:1;
  uint16_t                         u1INT_A_SIGNAL_EN:1;
  uint16_t                         u1INT_B_SIGNAL_EN:1;
  uint16_t                         u1INT_C_SIGNAL_EN:1;
  uint16_t                         u1RE_TUNE_EVENT_SIGNAL_EN:1;
  uint16_t                         u1FX_EVENT_SIGNAL_EN:1;
  uint16_t                         u1CQE_EVENT_SIGNAL_EN:1;
} stc_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_field_t;

typedef union un_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_t;

/**
  * \brief Error Interrupt Signal Enable Register (SDHC_CORE_ERROR_INT_SIGNAL_EN_R)
  */
typedef struct stc_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_field {
  uint16_t                         u1CMD_TOUT_ERR_SIGNAL_EN:1;
  uint16_t                         u1CMD_CRC_ERR_SIGNAL_EN:1;
  uint16_t                         u1CMD_END_BIT_ERR_SIGNAL_EN:1;
  uint16_t                         u1CMD_IDX_ERR_SIGNAL_EN:1;
  uint16_t                         u1DATA_TOUT_ERR_SIGNAL_EN:1;
  uint16_t                         u1DATA_CRC_ERR_SIGNAL_EN:1;
  uint16_t                         u1DATA_END_BIT_ERR_SIGNAL_EN:1;
  uint16_t                         u1CUR_LMT_ERR_SIGNAL_EN:1;
  uint16_t                         u1AUTO_CMD_ERR_SIGNAL_EN:1;
  uint16_t                         u1ADMA_ERR_SIGNAL_EN:1;
  uint16_t                         u1TUNING_ERR_SIGNAL_EN:1;
  uint16_t                         u1RESP_ERR_SIGNAL_EN:1;
  uint16_t                         u1BOOT_ACK_ERR_SIGNAL_EN:1;
  uint16_t                         u1VENDOR_ERR_SIGNAL_EN1:1;
  uint16_t                         u1VENDOR_ERR_SIGNAL_EN2:1;
  uint16_t                         u1VENDOR_ERR_SIGNAL_EN3:1;
} stc_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_field_t;

typedef union un_SDHC_CORE_ERROR_INT_SIGNAL_EN_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_t;

/**
  * \brief Auto CMD Status Register (SDHC_CORE_AUTO_CMD_STAT_R)
  */
typedef struct stc_SDHC_CORE_AUTO_CMD_STAT_R_field {
  uint16_t                         u1AUTO_CMD12_NOT_EXEC:1;
  uint16_t                         u1AUTO_CMD_TOUT_ERR:1;
  uint16_t                         u1AUTO_CMD_CRC_ERR:1;
  uint16_t                         u1AUTO_CMD_EBIT_ERR:1;
  uint16_t                         u1AUTO_CMD_IDX_ERR:1;
  uint16_t                         u1AUTO_CMD_RESP_ERR:1;
  uint16_t                         :1;
  uint16_t                         u1CMD_NOT_ISSUED_AUTO_CMD12:1;
  uint16_t                         :8;
} stc_SDHC_CORE_AUTO_CMD_STAT_R_field_t;

typedef union un_SDHC_CORE_AUTO_CMD_STAT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_AUTO_CMD_STAT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_AUTO_CMD_STAT_R_t;

/**
  * \brief Host Control 2 Register (SDHC_CORE_HOST_CTRL2_R)
  */
typedef struct stc_SDHC_CORE_HOST_CTRL2_R_field {
  uint16_t                         u3UHS_MODE_SEL:3;
  uint16_t                         u1SIGNALING_EN:1;
  uint16_t                         u2DRV_STRENGTH_SEL:2;
  uint16_t                         u1EXEC_TUNING:1;
  uint16_t                         u1SAMPLE_CLK_SEL:1;
  uint16_t                         u1UHS2_IF_ENABLE:1;
  uint16_t                         :1;
  uint16_t                         u1ADMA2_LEN_MODE:1;
  uint16_t                         u1CMD23_ENABLE:1;
  uint16_t                         u1HOST_VER4_ENABLE:1;
  uint16_t                         u1ADDRESSING:1;
  uint16_t                         u1ASYNC_INT_ENABLE:1;
  uint16_t                         u1PRESET_VAL_ENABLE:1;
} stc_SDHC_CORE_HOST_CTRL2_R_field_t;

typedef union un_SDHC_CORE_HOST_CTRL2_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_HOST_CTRL2_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_HOST_CTRL2_R_t;

/**
  * \brief Capabilities 1 Register - 0 to 31 (SDHC_CORE_CAPABILITIES1_R)
  */
typedef struct stc_SDHC_CORE_CAPABILITIES1_R_field {
  uint32_t                         u6TOUT_CLK_FREQ:6;
  uint32_t                         :1;
  uint32_t                         u1TOUT_CLK_UNIT:1;
  uint32_t                         u8BASE_CLK_FREQ:8;
  uint32_t                         u2MAX_BLK_LEN:2;
  uint32_t                         u1EMBEDDED_8_BIT:1;
  uint32_t                         u1ADMA2_SUPPORT:1;
  uint32_t                         :1;
  uint32_t                         u1HIGH_SPEED_SUPPORT:1;
  uint32_t                         u1SDMA_SUPPORT:1;
  uint32_t                         u1SUS_RES_SUPPORT:1;
  uint32_t                         u1VOLT_33:1;
  uint32_t                         u1VOLT_30:1;
  uint32_t                         u1VOLT_18:1;
  uint32_t                         u1SYS_ADDR_64_V4:1;
  uint32_t                         u1SYS_ADDR_64_V3:1;
  uint32_t                         u1ASYNC_INT_SUPPORT:1;
  uint32_t                         u2SLOT_TYPE_R:2;
} stc_SDHC_CORE_CAPABILITIES1_R_field_t;

typedef union un_SDHC_CORE_CAPABILITIES1_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CAPABILITIES1_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CAPABILITIES1_R_t;

/**
  * \brief Capabilities Register - 32 to 63 (SDHC_CORE_CAPABILITIES2_R)
  */
typedef struct stc_SDHC_CORE_CAPABILITIES2_R_field {
  uint32_t                         u1SDR50_SUPPORT:1;
  uint32_t                         u1SDR104_SUPPORT:1;
  uint32_t                         u1DDR50_SUPPORT:1;
  uint32_t                         u1UHS2_SUPPORT:1;
  uint32_t                         u1DRV_TYPEA:1;
  uint32_t                         u1DRV_TYPEC:1;
  uint32_t                         u1DRV_TYPED:1;
  uint32_t                         :1;
  uint32_t                         u4RETUNE_CNT:4;
  uint32_t                         :1;
  uint32_t                         u1USE_TUNING_SDR50:1;
  uint32_t                         u2RE_TUNING_MODES:2;
  uint32_t                         u8CLK_MUL:8;
  uint32_t                         :3;
  uint32_t                         u1ADMA3_SUPPORT:1;
  uint32_t                         u1VDD2_18V_SUPPORT:1;
  uint32_t                         :3;
} stc_SDHC_CORE_CAPABILITIES2_R_field_t;

typedef union un_SDHC_CORE_CAPABILITIES2_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CAPABILITIES2_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CAPABILITIES2_R_t;

/**
  * \brief Current Capabilities Register - 0 to 31 (SDHC_CORE_CURR_CAPABILITIES1_R)
  */
typedef struct stc_SDHC_CORE_CURR_CAPABILITIES1_R_field {
  uint32_t                         u8MAX_CUR_33V:8;
  uint32_t                         u8MAX_CUR_30V:8;
  uint32_t                         u8MAX_CUR_18V:8;
  uint32_t                         :8;
} stc_SDHC_CORE_CURR_CAPABILITIES1_R_field_t;

typedef union un_SDHC_CORE_CURR_CAPABILITIES1_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CURR_CAPABILITIES1_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CURR_CAPABILITIES1_R_t;

/**
  * \brief Maximum Current Capabilities Register - 32 to 63 (SDHC_CORE_CURR_CAPABILITIES2_R)
  */
typedef struct stc_SDHC_CORE_CURR_CAPABILITIES2_R_field {
  uint32_t                         u8MAX_CUR_VDD2_18V:8;
  uint32_t                         :24;
} stc_SDHC_CORE_CURR_CAPABILITIES2_R_field_t;

typedef union un_SDHC_CORE_CURR_CAPABILITIES2_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CURR_CAPABILITIES2_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CURR_CAPABILITIES2_R_t;

/**
  * \brief Force Event Register for Auto CMD Error Status register (SDHC_CORE_FORCE_AUTO_CMD_STAT_R)
  */
typedef struct stc_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_field {
  uint16_t                         u1FORCE_AUTO_CMD12_NOT_EXEC:1;
  uint16_t                         u1FORCE_AUTO_CMD_TOUT_ERR:1;
  uint16_t                         u1FORCE_AUTO_CMD_CRC_ERR:1;
  uint16_t                         u1FORCE_AUTO_CMD_EBIT_ERR:1;
  uint16_t                         u1FORCE_AUTO_CMD_IDX_ERR:1;
  uint16_t                         u1FORCE_AUTO_CMD_RESP_ERR:1;
  uint16_t                         :1;
  uint16_t                         u1FORCE_CMD_NOT_ISSUED_AUTO_CMD12:1;
  uint16_t                         :8;
} stc_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_field_t;

typedef union un_SDHC_CORE_FORCE_AUTO_CMD_STAT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_t;

/**
  * \brief Force Event Register for Error Interrupt Status (SDHC_CORE_FORCE_ERROR_INT_STAT_R)
  */
typedef struct stc_SDHC_CORE_FORCE_ERROR_INT_STAT_R_field {
  uint16_t                         u1FORCE_CMD_TOUT_ERR:1;
  uint16_t                         u1FORCE_CMD_CRC_ERR:1;
  uint16_t                         u1FORCE_CMD_END_BIT_ERR:1;
  uint16_t                         u1FORCE_CMD_IDX_ERR:1;
  uint16_t                         u1FORCE_DATA_TOUT_ERR:1;
  uint16_t                         u1FORCE_DATA_CRC_ERR:1;
  uint16_t                         u1FORCE_DATA_END_BIT_ERR:1;
  uint16_t                         u1FORCE_CUR_LMT_ERR:1;
  uint16_t                         u1FORCE_AUTO_CMD_ERR:1;
  uint16_t                         u1FORCE_ADMA_ERR:1;
  uint16_t                         u1FORCE_TUNING_ERR:1;
  uint16_t                         u1FORCE_RESP_ERR:1;
  uint16_t                         u1FORCE_BOOT_ACK_ERR:1;
  uint16_t                         u1FORCE_VENDOR_ERR1:1;
  uint16_t                         u1FORCE_VENDOR_ERR2:1;
  uint16_t                         u1FORCE_VENDOR_ERR3:1;
} stc_SDHC_CORE_FORCE_ERROR_INT_STAT_R_field_t;

typedef union un_SDHC_CORE_FORCE_ERROR_INT_STAT_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_FORCE_ERROR_INT_STAT_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_FORCE_ERROR_INT_STAT_R_t;

/**
  * \brief ADMA Error Status Register (SDHC_CORE_ADMA_ERR_STAT_R)
  */
typedef struct stc_SDHC_CORE_ADMA_ERR_STAT_R_field {
  uint8_t                          u2ADMA_ERR_STATES:2;
  uint8_t                          u1ADMA_LEN_ERR:1;
  uint8_t                          :5;
} stc_SDHC_CORE_ADMA_ERR_STAT_R_field_t;

typedef union un_SDHC_CORE_ADMA_ERR_STAT_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_ADMA_ERR_STAT_R_field_t stcField;
} un_SDHC_CORE_ADMA_ERR_STAT_R_t;

/**
  * \brief ADMA System Address Register - Low (SDHC_CORE_ADMA_SA_LOW_R)
  */
typedef struct stc_SDHC_CORE_ADMA_SA_LOW_R_field {
  uint32_t                         u32ADMA_SA_LOW:32;
} stc_SDHC_CORE_ADMA_SA_LOW_R_field_t;

typedef union un_SDHC_CORE_ADMA_SA_LOW_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_ADMA_SA_LOW_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_ADMA_SA_LOW_R_t;

/**
  * \brief ADMA3 Integrated Descriptor Address Register - Low (SDHC_CORE_ADMA_ID_LOW_R)
  */
typedef struct stc_SDHC_CORE_ADMA_ID_LOW_R_field {
  uint32_t                         u32ADMA_ID_LOW:32;
} stc_SDHC_CORE_ADMA_ID_LOW_R_field_t;

typedef union un_SDHC_CORE_ADMA_ID_LOW_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_ADMA_ID_LOW_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_ADMA_ID_LOW_R_t;

/**
  * \brief Host Controller Version (SDHC_CORE_HOST_CNTRL_VERS_R)
  */
typedef struct stc_SDHC_CORE_HOST_CNTRL_VERS_R_field {
  uint16_t                         u8SPEC_VERSION_NUM:8;
  uint16_t                         u8VENDOR_VERSION_NUM:8;
} stc_SDHC_CORE_HOST_CNTRL_VERS_R_field_t;

typedef union un_SDHC_CORE_HOST_CNTRL_VERS_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_HOST_CNTRL_VERS_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_HOST_CNTRL_VERS_R_t;

/**
  * \brief Command Queuing Version register (SDHC_CORE_CQVER)
  */
typedef struct stc_SDHC_CORE_CQVER_field {
  uint32_t                         u4EMMC_VER_SUFFIX:4;
  uint32_t                         u4EMMC_VER_MINOR:4;
  uint32_t                         u4EMMC_VER_MAJOR:4;
  uint32_t                         :20;
} stc_SDHC_CORE_CQVER_field_t;

typedef union un_SDHC_CORE_CQVER {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQVER_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQVER_t;

/**
  * \brief Command Queuing Capabilities register (SDHC_CORE_CQCAP)
  */
typedef struct stc_SDHC_CORE_CQCAP_field {
  uint32_t                         u10ITCFVAL:10;
  uint32_t                         :2;
  uint32_t                         u4ITCFMUL:4;
  uint32_t                         :12;
  uint32_t                         u1CRYPTO_SUPPORT:1;
  uint32_t                         :3;
} stc_SDHC_CORE_CQCAP_field_t;

typedef union un_SDHC_CORE_CQCAP {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCAP_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCAP_t;

/**
  * \brief Command Queuing Configuration register (SDHC_CORE_CQCFG)
  */
typedef struct stc_SDHC_CORE_CQCFG_field {
  uint32_t                         u1CQ_EN:1;
  uint32_t                         u1CR_GENERAL_EN:1;
  uint32_t                         :6;
  uint32_t                         u1TASK_DESC_SIZE:1;
  uint32_t                         :3;
  uint32_t                         u1DCMD_EN:1;
  uint32_t                         :19;
} stc_SDHC_CORE_CQCFG_field_t;

typedef union un_SDHC_CORE_CQCFG {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCFG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCFG_t;

/**
  * \brief Command Queuing Control register (SDHC_CORE_CQCTL)
  */
typedef struct stc_SDHC_CORE_CQCTL_field {
  uint32_t                         u1HALT:1;
  uint32_t                         :7;
  uint32_t                         u1CLR_ALL_TASKS:1;
  uint32_t                         :23;
} stc_SDHC_CORE_CQCTL_field_t;

typedef union un_SDHC_CORE_CQCTL {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCTL_t;

/**
  * \brief Command Queuing Interrupt Status register (SDHC_CORE_CQIS)
  */
typedef struct stc_SDHC_CORE_CQIS_field {
  uint32_t                         u1HAC:1;
  uint32_t                         u1TCC:1;
  uint32_t                         u1RED:1;
  uint32_t                         u1TCL:1;
  uint32_t                         u1GCE:1;
  uint32_t                         u1ICCE:1;
  uint32_t                         :26;
} stc_SDHC_CORE_CQIS_field_t;

typedef union un_SDHC_CORE_CQIS {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQIS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQIS_t;

/**
  * \brief Command Queuing Interrupt Status Enable register (SDHC_CORE_CQISE)
  */
typedef struct stc_SDHC_CORE_CQISE_field {
  uint32_t                         u1HAC_STE:1;
  uint32_t                         u1TCC_STE:1;
  uint32_t                         u1RED_STE:1;
  uint32_t                         u1TCL_STE:1;
  uint32_t                         u1GCE_STE:1;
  uint32_t                         u1ICCE_STE:1;
  uint32_t                         :26;
} stc_SDHC_CORE_CQISE_field_t;

typedef union un_SDHC_CORE_CQISE {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQISE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQISE_t;

/**
  * \brief Command Queuing Interrupt signal enable register (SDHC_CORE_CQISGE)
  */
typedef struct stc_SDHC_CORE_CQISGE_field {
  uint32_t                         u1HAC_SGE:1;
  uint32_t                         u1TCC_SGE:1;
  uint32_t                         u1RED_SGE:1;
  uint32_t                         u1TCL_SGE:1;
  uint32_t                         u1GCE_SGE:1;
  uint32_t                         u1ICCE_SGE:1;
  uint32_t                         :26;
} stc_SDHC_CORE_CQISGE_field_t;

typedef union un_SDHC_CORE_CQISGE {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQISGE_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQISGE_t;

/**
  * \brief Command Queuing Interrupt Coalescing register (SDHC_CORE_CQIC)
  */
typedef struct stc_SDHC_CORE_CQIC_field {
  uint32_t                         u7TOUT_VAL:7;
  uint32_t                         u1TOUT_VAL_WEN:1;
  uint32_t                         u5INTC_TH:5;
  uint32_t                         :2;
  uint32_t                         u1INTC_TH_WEN:1;
  uint32_t                         u1INTC_RST:1;
  uint32_t                         :3;
  uint32_t                         u1INTC_STAT:1;
  uint32_t                         :10;
  uint32_t                         u1INTC_EN:1;
} stc_SDHC_CORE_CQIC_field_t;

typedef union un_SDHC_CORE_CQIC {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQIC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQIC_t;

/**
  * \brief Command Queuing Task Descriptor List Base Address register (SDHC_CORE_CQTDLBA)
  */
typedef struct stc_SDHC_CORE_CQTDLBA_field {
  uint32_t                         u32TDLBA:32;
} stc_SDHC_CORE_CQTDLBA_field_t;

typedef union un_SDHC_CORE_CQTDLBA {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQTDLBA_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQTDLBA_t;

/**
  * \brief Command Queuing DoorBell register (SDHC_CORE_CQTDBR)
  */
typedef struct stc_SDHC_CORE_CQTDBR_field {
  uint32_t                         u32DBR:32;
} stc_SDHC_CORE_CQTDBR_field_t;

typedef union un_SDHC_CORE_CQTDBR {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQTDBR_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQTDBR_t;

/**
  * \brief Command Queuing TaskClear Notification register (SDHC_CORE_CQTCN)
  */
typedef struct stc_SDHC_CORE_CQTCN_field {
  uint32_t                         u32TCN:32;
} stc_SDHC_CORE_CQTCN_field_t;

typedef union un_SDHC_CORE_CQTCN {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQTCN_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQTCN_t;

/**
  * \brief Device queue status register (SDHC_CORE_CQDQS)
  */
typedef struct stc_SDHC_CORE_CQDQS_field {
  uint32_t                         u32DQS:32;
} stc_SDHC_CORE_CQDQS_field_t;

typedef union un_SDHC_CORE_CQDQS {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQDQS_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQDQS_t;

/**
  * \brief Device pending tasks register (SDHC_CORE_CQDPT)
  */
typedef struct stc_SDHC_CORE_CQDPT_field {
  uint32_t                         u32DPT:32;
} stc_SDHC_CORE_CQDPT_field_t;

typedef union un_SDHC_CORE_CQDPT {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQDPT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQDPT_t;

/**
  * \brief Command Queuing DoorBell register (SDHC_CORE_CQTCLR)
  */
typedef struct stc_SDHC_CORE_CQTCLR_field {
  uint32_t                         u32TCLR:32;
} stc_SDHC_CORE_CQTCLR_field_t;

typedef union un_SDHC_CORE_CQTCLR {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQTCLR_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQTCLR_t;

/**
  * \brief CQ Send Status Configuration 1 register (SDHC_CORE_CQSSC1)
  */
typedef struct stc_SDHC_CORE_CQSSC1_field {
  uint32_t                         u16SQSCMD_IDLE_TMR:16;
  uint32_t                         u4SQSCMD_BLK_CNT:4;
  uint32_t                         :12;
} stc_SDHC_CORE_CQSSC1_field_t;

typedef union un_SDHC_CORE_CQSSC1 {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQSSC1_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQSSC1_t;

/**
  * \brief CQ Send Status Configuration 2 register (SDHC_CORE_CQSSC2)
  */
typedef struct stc_SDHC_CORE_CQSSC2_field {
  uint32_t                         u16SQSCMD_RCA:16;
  uint32_t                         :16;
} stc_SDHC_CORE_CQSSC2_field_t;

typedef union un_SDHC_CORE_CQSSC2 {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQSSC2_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQSSC2_t;

/**
  * \brief Command response for direct command register (SDHC_CORE_CQCRDCT)
  */
typedef struct stc_SDHC_CORE_CQCRDCT_field {
  uint32_t                         u32DCMD_RESP:32;
} stc_SDHC_CORE_CQCRDCT_field_t;

typedef union un_SDHC_CORE_CQCRDCT {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCRDCT_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCRDCT_t;

/**
  * \brief Command response mode error mask register (SDHC_CORE_CQRMEM)
  */
typedef struct stc_SDHC_CORE_CQRMEM_field {
  uint32_t                         u32RESP_ERR_MASK:32;
} stc_SDHC_CORE_CQRMEM_field_t;

typedef union un_SDHC_CORE_CQRMEM {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQRMEM_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQRMEM_t;

/**
  * \brief CQ Task Error Information register (SDHC_CORE_CQTERRI)
  */
typedef struct stc_SDHC_CORE_CQTERRI_field {
  uint32_t                         u6RESP_ERR_CMD_INDX:6;
  uint32_t                         :2;
  uint32_t                         u5RESP_ERR_TASKID:5;
  uint32_t                         :2;
  uint32_t                         u1RESP_ERR_FIELDS_VALID:1;
  uint32_t                         u6TRANS_ERR_CMD_INDX:6;
  uint32_t                         :2;
  uint32_t                         u5TRANS_ERR_TASKID:5;
  uint32_t                         :2;
  uint32_t                         u1TRANS_ERR_FIELDS_VALID:1;
} stc_SDHC_CORE_CQTERRI_field_t;

typedef union un_SDHC_CORE_CQTERRI {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQTERRI_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQTERRI_t;

/**
  * \brief CQ Command response index (SDHC_CORE_CQCRI)
  */
typedef struct stc_SDHC_CORE_CQCRI_field {
  uint32_t                         u6CMD_RESP_INDX:6;
  uint32_t                         :26;
} stc_SDHC_CORE_CQCRI_field_t;

typedef union un_SDHC_CORE_CQCRI {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCRI_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCRI_t;

/**
  * \brief CQ Command response argument register (SDHC_CORE_CQCRA)
  */
typedef struct stc_SDHC_CORE_CQCRA_field {
  uint32_t                         u32CMD_RESP_ARG:32;
} stc_SDHC_CORE_CQCRA_field_t;

typedef union un_SDHC_CORE_CQCRA {
  uint32_t                         u32Register;
  stc_SDHC_CORE_CQCRA_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_CQCRA_t;

/**
  * \brief MSHC version (SDHC_CORE_MSHC_VER_ID_R)
  */
typedef struct stc_SDHC_CORE_MSHC_VER_ID_R_field {
  uint32_t                         u32MSHC_VER_ID:32;
} stc_SDHC_CORE_MSHC_VER_ID_R_field_t;

typedef union un_SDHC_CORE_MSHC_VER_ID_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_MSHC_VER_ID_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_MSHC_VER_ID_R_t;

/**
  * \brief MSHC version type (SDHC_CORE_MSHC_VER_TYPE_R)
  */
typedef struct stc_SDHC_CORE_MSHC_VER_TYPE_R_field {
  uint32_t                         u32MSHC_VER_TYPE:32;
} stc_SDHC_CORE_MSHC_VER_TYPE_R_field_t;

typedef union un_SDHC_CORE_MSHC_VER_TYPE_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_MSHC_VER_TYPE_R_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_MSHC_VER_TYPE_R_t;

/**
  * \brief MSHC Control register (SDHC_CORE_MSHC_CTRL_R)
  */
typedef struct stc_SDHC_CORE_MSHC_CTRL_R_field {
  uint8_t                          u1CMD_CONFLICT_CHECK:1;
  uint8_t                          :3;
  uint8_t                          u1SW_CG_DIS:1;
  uint8_t                          :3;
} stc_SDHC_CORE_MSHC_CTRL_R_field_t;

typedef union un_SDHC_CORE_MSHC_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_MSHC_CTRL_R_field_t stcField;
} un_SDHC_CORE_MSHC_CTRL_R_t;

/**
  * \brief MBIU Control register (SDHC_CORE_MBIU_CTRL_R)
  */
typedef struct stc_SDHC_CORE_MBIU_CTRL_R_field {
  uint8_t                          u1UNDEFL_INCR_EN:1;
  uint8_t                          u1BURST_INCR4_EN:1;
  uint8_t                          u1BURST_INCR8_EN:1;
  uint8_t                          u1BURST_INCR16_EN:1;
  uint8_t                          :4;
} stc_SDHC_CORE_MBIU_CTRL_R_field_t;

typedef union un_SDHC_CORE_MBIU_CTRL_R {
  uint8_t                          u8Register;
  stc_SDHC_CORE_MBIU_CTRL_R_field_t stcField;
} un_SDHC_CORE_MBIU_CTRL_R_t;

/**
  * \brief eMMC Control register (SDHC_CORE_EMMC_CTRL_R)
  */
typedef struct stc_SDHC_CORE_EMMC_CTRL_R_field {
  uint16_t                         u1CARD_IS_EMMC:1;
  uint16_t                         u1DISABLE_DATA_CRC_CHK:1;
  uint16_t                         u1EMMC_RST_N:1;
  uint16_t                         u1EMMC_RST_N_OE:1;
  uint16_t                         :5;
  uint16_t                         u1CQE_ALGO_SEL:1;
  uint16_t                         u1CQE_PREFETCH_DISABLE:1;
  uint16_t                         :5;
} stc_SDHC_CORE_EMMC_CTRL_R_field_t;

typedef union un_SDHC_CORE_EMMC_CTRL_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_EMMC_CTRL_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_EMMC_CTRL_R_t;

/**
  * \brief eMMC Boot Control register (SDHC_CORE_BOOT_CTRL_R)
  */
typedef struct stc_SDHC_CORE_BOOT_CTRL_R_field {
  uint16_t                         u1MAN_BOOT_EN:1;
  uint16_t                         :6;
  uint16_t                         u1VALIDATE_BOOT:1;
  uint16_t                         u1BOOT_ACK_ENABLE:1;
  uint16_t                         :3;
  uint16_t                         u4BOOT_TOUT_CNT:4;
} stc_SDHC_CORE_BOOT_CTRL_R_field_t;

typedef union un_SDHC_CORE_BOOT_CTRL_R {
  uint16_t                         u16Register;
  stc_SDHC_CORE_BOOT_CTRL_R_field_t stcField;
  uint8_t                          au8Byte[2];
} un_SDHC_CORE_BOOT_CTRL_R_t;

/**
  * \brief General Purpose Input register (SDHC_CORE_GP_IN_R)
  */
typedef struct stc_SDHC_CORE_GP_IN_R_field {
  uint32_t                         u1GP_IN:1;
  uint32_t                         :31;
} stc_SDHC_CORE_GP_IN_R_field_t;

typedef union un_SDHC_CORE_GP_IN_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_GP_IN_R_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_GP_IN_R_t;

/**
  * \brief General Purpose Output register (SDHC_CORE_GP_OUT_R)
  */
typedef struct stc_SDHC_CORE_GP_OUT_R_field {
  uint32_t                         u1CARD_DETECT_EN:1;
  uint32_t                         u1CARD_MECH_WRITE_PROT_EN:1;
  uint32_t                         u1LED_CTRL_OE:1;
  uint32_t                         u1CARD_CLOCK_OE:1;
  uint32_t                         u1CARD_IF_PWR_EN_OE:1;
  uint32_t                         u1IO_VOLT_SEL_OE:1;
  uint32_t                         u2CARD_CLOCK_OUT_DLY:2;
  uint32_t                         u2CARD_CLOCK_IN_DLY:2;
  uint32_t                         :22;
} stc_SDHC_CORE_GP_OUT_R_field_t;

typedef union un_SDHC_CORE_GP_OUT_R {
  uint32_t                         u32Register;
  stc_SDHC_CORE_GP_OUT_R_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SDHC_CORE_GP_OUT_R_t;



/**
  * \brief MMIO at SDHC wrapper level (SDHC_WRAP)
  */
typedef struct stc_SDHC_WRAP {
  un_SDHC_WRAP_CTL_t               unCTL;              /*!< 0x00000000 Top level wrapper control */
  uint32_t                         au32Reserved[7];
} stc_SDHC_WRAP_t;                                     /*!< Size = 32 (0x20) */

/**
  * \brief MMIO for Synopsys Mobile Storage Host Controller IP (SDHC_CORE)
  */
typedef struct stc_SDHC_CORE {
  un_SDHC_CORE_SDMASA_R_t          unSDMASA_R;         /*!< 0x00000000 SDMA System Address register */
  un_SDHC_CORE_BLOCKSIZE_R_t       unBLOCKSIZE_R;      /*!< 0x00000004 Block Size register */
  un_SDHC_CORE_BLOCKCOUNT_R_t      unBLOCKCOUNT_R;     /*!< 0x00000006 16-bit Block Count register */
  un_SDHC_CORE_ARGUMENT_R_t        unARGUMENT_R;       /*!< 0x00000008 Argument register */
  un_SDHC_CORE_XFER_MODE_R_t       unXFER_MODE_R;      /*!< 0x0000000C Transfer Mode register */
  un_SDHC_CORE_CMD_R_t             unCMD_R;            /*!< 0x0000000E Command register */
  un_SDHC_CORE_RESP01_R_t          unRESP01_R;         /*!< 0x00000010 Response Register 0/1 */
  un_SDHC_CORE_RESP23_R_t          unRESP23_R;         /*!< 0x00000014 Response Register 2/3 */
  un_SDHC_CORE_RESP45_R_t          unRESP45_R;         /*!< 0x00000018 Response Register 4/5 */
  un_SDHC_CORE_RESP67_R_t          unRESP67_R;         /*!< 0x0000001C Response Register 6/7 */
  un_SDHC_CORE_BUF_DATA_R_t        unBUF_DATA_R;       /*!< 0x00000020 Buffer Data Port Register */
  un_SDHC_CORE_PSTATE_REG_t        unPSTATE_REG;       /*!< 0x00000024 Present State Register */
  un_SDHC_CORE_HOST_CTRL1_R_t      unHOST_CTRL1_R;     /*!< 0x00000028 Host Control 1 Register */
  un_SDHC_CORE_PWR_CTRL_R_t        unPWR_CTRL_R;       /*!< 0x00000029 Power Control Register */
  un_SDHC_CORE_BGAP_CTRL_R_t       unBGAP_CTRL_R;      /*!< 0x0000002A Block Gap Control Register */
  un_SDHC_CORE_WUP_CTRL_R_t        unWUP_CTRL_R;       /*!< 0x0000002B Wakeup Control Register */
  un_SDHC_CORE_CLK_CTRL_R_t        unCLK_CTRL_R;       /*!< 0x0000002C Clock Control Register */
  un_SDHC_CORE_TOUT_CTRL_R_t       unTOUT_CTRL_R;      /*!< 0x0000002E Timeout Control Register */
  un_SDHC_CORE_SW_RST_R_t          unSW_RST_R;         /*!< 0x0000002F Software Reset Register */
  un_SDHC_CORE_NORMAL_INT_STAT_R_t unNORMAL_INT_STAT_R; /*!< 0x00000030 Normal Interrupt Status Register */
  un_SDHC_CORE_ERROR_INT_STAT_R_t  unERROR_INT_STAT_R; /*!< 0x00000032 Error Interrupt Status Register */
  un_SDHC_CORE_NORMAL_INT_STAT_EN_R_t unNORMAL_INT_STAT_EN_R; /*!< 0x00000034 Normal Interrupt Status Enable Register */
  un_SDHC_CORE_ERROR_INT_STAT_EN_R_t unERROR_INT_STAT_EN_R; /*!< 0x00000036 Error Interrupt Status Enable Register */
  un_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_t unNORMAL_INT_SIGNAL_EN_R; /*!< 0x00000038 Normal Interrupt Signal Enable Register */
  un_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_t unERROR_INT_SIGNAL_EN_R; /*!< 0x0000003A Error Interrupt Signal Enable Register */
  un_SDHC_CORE_AUTO_CMD_STAT_R_t   unAUTO_CMD_STAT_R;  /*!< 0x0000003C Auto CMD Status Register */
  un_SDHC_CORE_HOST_CTRL2_R_t      unHOST_CTRL2_R;     /*!< 0x0000003E Host Control 2 Register */
  un_SDHC_CORE_CAPABILITIES1_R_t   unCAPABILITIES1_R;  /*!< 0x00000040 Capabilities 1 Register - 0 to 31 */
  un_SDHC_CORE_CAPABILITIES2_R_t   unCAPABILITIES2_R;  /*!< 0x00000044 Capabilities Register - 32 to 63 */
  un_SDHC_CORE_CURR_CAPABILITIES1_R_t unCURR_CAPABILITIES1_R; /*!< 0x00000048 Current Capabilities Register - 0 to 31 */
  un_SDHC_CORE_CURR_CAPABILITIES2_R_t unCURR_CAPABILITIES2_R; /*!< 0x0000004C Maximum Current Capabilities Register - 32 to 63 */
  un_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_t unFORCE_AUTO_CMD_STAT_R; /*!< 0x00000050 Force Event Register for Auto CMD Error Status register */
  un_SDHC_CORE_FORCE_ERROR_INT_STAT_R_t unFORCE_ERROR_INT_STAT_R; /*!< 0x00000052 Force Event Register for Error Interrupt Status */
  un_SDHC_CORE_ADMA_ERR_STAT_R_t   unADMA_ERR_STAT_R;  /*!< 0x00000054 ADMA Error Status Register */
  uint8_t                          au8Reserved[3];
  un_SDHC_CORE_ADMA_SA_LOW_R_t     unADMA_SA_LOW_R;    /*!< 0x00000058 ADMA System Address Register - Low */
  uint32_t                         au32Reserved1[7];
  un_SDHC_CORE_ADMA_ID_LOW_R_t     unADMA_ID_LOW_R;    /*!< 0x00000078 ADMA3 Integrated Descriptor Address Register - Low */
  uint16_t                         au16Reserved2[65];
  un_SDHC_CORE_HOST_CNTRL_VERS_R_t unHOST_CNTRL_VERS_R; /*!< 0x000000FE Host Controller Version */
  uint32_t                         au32Reserved3[32];
  un_SDHC_CORE_CQVER_t             unCQVER;            /*!< 0x00000180 Command Queuing Version register */
  un_SDHC_CORE_CQCAP_t             unCQCAP;            /*!< 0x00000184 Command Queuing Capabilities register */
  un_SDHC_CORE_CQCFG_t             unCQCFG;            /*!< 0x00000188 Command Queuing Configuration register */
  un_SDHC_CORE_CQCTL_t             unCQCTL;            /*!< 0x0000018C Command Queuing Control register */
  un_SDHC_CORE_CQIS_t              unCQIS;             /*!< 0x00000190 Command Queuing Interrupt Status register */
  un_SDHC_CORE_CQISE_t             unCQISE;            /*!< 0x00000194 Command Queuing Interrupt Status Enable register */
  un_SDHC_CORE_CQISGE_t            unCQISGE;           /*!< 0x00000198 Command Queuing Interrupt signal enable register */
  un_SDHC_CORE_CQIC_t              unCQIC;             /*!< 0x0000019C Command Queuing Interrupt Coalescing register */
  un_SDHC_CORE_CQTDLBA_t           unCQTDLBA;          /*!< 0x000001A0 Command Queuing Task Descriptor List Base Address
                                                                                register */
  uint32_t                         u32Reserved4;
  un_SDHC_CORE_CQTDBR_t            unCQTDBR;           /*!< 0x000001A8 Command Queuing DoorBell register */
  un_SDHC_CORE_CQTCN_t             unCQTCN;            /*!< 0x000001AC Command Queuing TaskClear Notification register */
  un_SDHC_CORE_CQDQS_t             unCQDQS;            /*!< 0x000001B0 Device queue status register */
  un_SDHC_CORE_CQDPT_t             unCQDPT;            /*!< 0x000001B4 Device pending tasks register */
  un_SDHC_CORE_CQTCLR_t            unCQTCLR;           /*!< 0x000001B8 Command Queuing DoorBell register */
  uint32_t                         u32Reserved5;
  un_SDHC_CORE_CQSSC1_t            unCQSSC1;           /*!< 0x000001C0 CQ Send Status Configuration 1 register */
  un_SDHC_CORE_CQSSC2_t            unCQSSC2;           /*!< 0x000001C4 CQ Send Status Configuration 2 register */
  un_SDHC_CORE_CQCRDCT_t           unCQCRDCT;          /*!< 0x000001C8 Command response for direct command register */
  uint32_t                         u32Reserved6;
  un_SDHC_CORE_CQRMEM_t            unCQRMEM;           /*!< 0x000001D0 Command response mode error mask register */
  un_SDHC_CORE_CQTERRI_t           unCQTERRI;          /*!< 0x000001D4 CQ Task Error Information register */
  un_SDHC_CORE_CQCRI_t             unCQCRI;            /*!< 0x000001D8 CQ Command response index */
  un_SDHC_CORE_CQCRA_t             unCQCRA;            /*!< 0x000001DC CQ Command response argument register */
  uint32_t                         au32Reserved7[200];
  un_SDHC_CORE_MSHC_VER_ID_R_t     unMSHC_VER_ID_R;    /*!< 0x00000500 MSHC version */
  un_SDHC_CORE_MSHC_VER_TYPE_R_t   unMSHC_VER_TYPE_R;  /*!< 0x00000504 MSHC version type */
  un_SDHC_CORE_MSHC_CTRL_R_t       unMSHC_CTRL_R;      /*!< 0x00000508 MSHC Control register */
  uint8_t                          au8Reserved8[7];
  un_SDHC_CORE_MBIU_CTRL_R_t       unMBIU_CTRL_R;      /*!< 0x00000510 MBIU Control register */
  uint8_t                          au8Reserved9[27];
  un_SDHC_CORE_EMMC_CTRL_R_t       unEMMC_CTRL_R;      /*!< 0x0000052C eMMC Control register */
  un_SDHC_CORE_BOOT_CTRL_R_t       unBOOT_CTRL_R;      /*!< 0x0000052E eMMC Boot Control register */
  un_SDHC_CORE_GP_IN_R_t           unGP_IN_R;          /*!< 0x00000530 General Purpose Input register */
  un_SDHC_CORE_GP_OUT_R_t          unGP_OUT_R;         /*!< 0x00000534 General Purpose Output register */
  uint32_t                         au32Reserved10[690];
} stc_SDHC_CORE_t;                                     /*!< Size = 4096 (0x1000) */

/**
  * \brief SD/eMMC Host Controller (SDHC)
  */
typedef struct stc_SDHC {
  stc_SDHC_WRAP_t                  WRAP;               /*!< 0x00000000 MMIO at SDHC wrapper level */
  uint32_t                         au32Reserved[1016];
  stc_SDHC_CORE_t                  CORE;               /*!< 0x00001000 MMIO for Synopsys Mobile Storage Host Controller IP */
} stc_SDHC_t;                                          /*!< Size = 8192 (0x2000) */


/* SDHC_WRAP.CTL */
#define SDHC_WRAP_CTL_ENABLE_Pos                31UL
#define SDHC_WRAP_CTL_ENABLE_Msk                0x80000000UL


/* SDHC_CORE.SDMASA_R */
#define SDHC_CORE_SDMASA_R_BLOCKCNT_SDMASA_Pos  0UL
#define SDHC_CORE_SDMASA_R_BLOCKCNT_SDMASA_Msk  0xFFFFFFFFUL
/* SDHC_CORE.BLOCKSIZE_R */
#define SDHC_CORE_BLOCKSIZE_R_XFER_BLOCK_SIZE_Pos 0UL
#define SDHC_CORE_BLOCKSIZE_R_XFER_BLOCK_SIZE_Msk 0xFFFUL
#define SDHC_CORE_BLOCKSIZE_R_SDMA_BUF_BDARY_Pos 12UL
#define SDHC_CORE_BLOCKSIZE_R_SDMA_BUF_BDARY_Msk 0x7000UL
/* SDHC_CORE.BLOCKCOUNT_R */
#define SDHC_CORE_BLOCKCOUNT_R_BLOCK_CNT_Pos    0UL
#define SDHC_CORE_BLOCKCOUNT_R_BLOCK_CNT_Msk    0xFFFFUL
/* SDHC_CORE.ARGUMENT_R */
#define SDHC_CORE_ARGUMENT_R_ARGUMENT_Pos       0UL
#define SDHC_CORE_ARGUMENT_R_ARGUMENT_Msk       0xFFFFFFFFUL
/* SDHC_CORE.XFER_MODE_R */
#define SDHC_CORE_XFER_MODE_R_DMA_ENABLE_Pos    0UL
#define SDHC_CORE_XFER_MODE_R_DMA_ENABLE_Msk    0x1UL
#define SDHC_CORE_XFER_MODE_R_BLOCK_COUNT_ENABLE_Pos 1UL
#define SDHC_CORE_XFER_MODE_R_BLOCK_COUNT_ENABLE_Msk 0x2UL
#define SDHC_CORE_XFER_MODE_R_AUTO_CMD_ENABLE_Pos 2UL
#define SDHC_CORE_XFER_MODE_R_AUTO_CMD_ENABLE_Msk 0xCUL
#define SDHC_CORE_XFER_MODE_R_DATA_XFER_DIR_Pos 4UL
#define SDHC_CORE_XFER_MODE_R_DATA_XFER_DIR_Msk 0x10UL
#define SDHC_CORE_XFER_MODE_R_MULTI_BLK_SEL_Pos 5UL
#define SDHC_CORE_XFER_MODE_R_MULTI_BLK_SEL_Msk 0x20UL
#define SDHC_CORE_XFER_MODE_R_RESP_TYPE_Pos     6UL
#define SDHC_CORE_XFER_MODE_R_RESP_TYPE_Msk     0x40UL
#define SDHC_CORE_XFER_MODE_R_RESP_ERR_CHK_ENABLE_Pos 7UL
#define SDHC_CORE_XFER_MODE_R_RESP_ERR_CHK_ENABLE_Msk 0x80UL
#define SDHC_CORE_XFER_MODE_R_RESP_INT_DISABLE_Pos 8UL
#define SDHC_CORE_XFER_MODE_R_RESP_INT_DISABLE_Msk 0x100UL
/* SDHC_CORE.CMD_R */
#define SDHC_CORE_CMD_R_RESP_TYPE_SELECT_Pos    0UL
#define SDHC_CORE_CMD_R_RESP_TYPE_SELECT_Msk    0x3UL
#define SDHC_CORE_CMD_R_SUB_CMD_FLAG_Pos        2UL
#define SDHC_CORE_CMD_R_SUB_CMD_FLAG_Msk        0x4UL
#define SDHC_CORE_CMD_R_CMD_CRC_CHK_ENABLE_Pos  3UL
#define SDHC_CORE_CMD_R_CMD_CRC_CHK_ENABLE_Msk  0x8UL
#define SDHC_CORE_CMD_R_CMD_IDX_CHK_ENABLE_Pos  4UL
#define SDHC_CORE_CMD_R_CMD_IDX_CHK_ENABLE_Msk  0x10UL
#define SDHC_CORE_CMD_R_DATA_PRESENT_SEL_Pos    5UL
#define SDHC_CORE_CMD_R_DATA_PRESENT_SEL_Msk    0x20UL
#define SDHC_CORE_CMD_R_CMD_TYPE_Pos            6UL
#define SDHC_CORE_CMD_R_CMD_TYPE_Msk            0xC0UL
#define SDHC_CORE_CMD_R_CMD_INDEX_Pos           8UL
#define SDHC_CORE_CMD_R_CMD_INDEX_Msk           0x3F00UL
/* SDHC_CORE.RESP01_R */
#define SDHC_CORE_RESP01_R_RESP01_Pos           0UL
#define SDHC_CORE_RESP01_R_RESP01_Msk           0xFFFFFFFFUL
/* SDHC_CORE.RESP23_R */
#define SDHC_CORE_RESP23_R_RESP23_Pos           0UL
#define SDHC_CORE_RESP23_R_RESP23_Msk           0xFFFFFFFFUL
/* SDHC_CORE.RESP45_R */
#define SDHC_CORE_RESP45_R_RESP45_Pos           0UL
#define SDHC_CORE_RESP45_R_RESP45_Msk           0xFFFFFFFFUL
/* SDHC_CORE.RESP67_R */
#define SDHC_CORE_RESP67_R_RESP67_Pos           0UL
#define SDHC_CORE_RESP67_R_RESP67_Msk           0xFFFFFFFFUL
/* SDHC_CORE.BUF_DATA_R */
#define SDHC_CORE_BUF_DATA_R_BUF_DATA_Pos       0UL
#define SDHC_CORE_BUF_DATA_R_BUF_DATA_Msk       0xFFFFFFFFUL
/* SDHC_CORE.PSTATE_REG */
#define SDHC_CORE_PSTATE_REG_CMD_INHIBIT_Pos    0UL
#define SDHC_CORE_PSTATE_REG_CMD_INHIBIT_Msk    0x1UL
#define SDHC_CORE_PSTATE_REG_CMD_INHIBIT_DAT_Pos 1UL
#define SDHC_CORE_PSTATE_REG_CMD_INHIBIT_DAT_Msk 0x2UL
#define SDHC_CORE_PSTATE_REG_DAT_LINE_ACTIVE_Pos 2UL
#define SDHC_CORE_PSTATE_REG_DAT_LINE_ACTIVE_Msk 0x4UL
#define SDHC_CORE_PSTATE_REG_DAT_7_4_Pos        4UL
#define SDHC_CORE_PSTATE_REG_DAT_7_4_Msk        0xF0UL
#define SDHC_CORE_PSTATE_REG_WR_XFER_ACTIVE_Pos 8UL
#define SDHC_CORE_PSTATE_REG_WR_XFER_ACTIVE_Msk 0x100UL
#define SDHC_CORE_PSTATE_REG_RD_XFER_ACTIVE_Pos 9UL
#define SDHC_CORE_PSTATE_REG_RD_XFER_ACTIVE_Msk 0x200UL
#define SDHC_CORE_PSTATE_REG_BUF_WR_ENABLE_Pos  10UL
#define SDHC_CORE_PSTATE_REG_BUF_WR_ENABLE_Msk  0x400UL
#define SDHC_CORE_PSTATE_REG_BUF_RD_ENABLE_Pos  11UL
#define SDHC_CORE_PSTATE_REG_BUF_RD_ENABLE_Msk  0x800UL
#define SDHC_CORE_PSTATE_REG_CARD_INSERTED_Pos  16UL
#define SDHC_CORE_PSTATE_REG_CARD_INSERTED_Msk  0x10000UL
#define SDHC_CORE_PSTATE_REG_CARD_STABLE_Pos    17UL
#define SDHC_CORE_PSTATE_REG_CARD_STABLE_Msk    0x20000UL
#define SDHC_CORE_PSTATE_REG_CARD_DETECT_PIN_LEVEL_Pos 18UL
#define SDHC_CORE_PSTATE_REG_CARD_DETECT_PIN_LEVEL_Msk 0x40000UL
#define SDHC_CORE_PSTATE_REG_WR_PROTECT_SW_LVL_Pos 19UL
#define SDHC_CORE_PSTATE_REG_WR_PROTECT_SW_LVL_Msk 0x80000UL
#define SDHC_CORE_PSTATE_REG_DAT_3_0_Pos        20UL
#define SDHC_CORE_PSTATE_REG_DAT_3_0_Msk        0xF00000UL
#define SDHC_CORE_PSTATE_REG_CMD_LINE_LVL_Pos   24UL
#define SDHC_CORE_PSTATE_REG_CMD_LINE_LVL_Msk   0x1000000UL
#define SDHC_CORE_PSTATE_REG_HOST_REG_VOL_Pos   25UL
#define SDHC_CORE_PSTATE_REG_HOST_REG_VOL_Msk   0x2000000UL
#define SDHC_CORE_PSTATE_REG_CMD_ISSU_ERR_Pos   27UL
#define SDHC_CORE_PSTATE_REG_CMD_ISSU_ERR_Msk   0x8000000UL
#define SDHC_CORE_PSTATE_REG_SUB_CMD_STAT_Pos   28UL
#define SDHC_CORE_PSTATE_REG_SUB_CMD_STAT_Msk   0x10000000UL
/* SDHC_CORE.HOST_CTRL1_R */
#define SDHC_CORE_HOST_CTRL1_R_LED_CTRL_Pos     0UL
#define SDHC_CORE_HOST_CTRL1_R_LED_CTRL_Msk     0x1UL
#define SDHC_CORE_HOST_CTRL1_R_DAT_XFER_WIDTH_Pos 1UL
#define SDHC_CORE_HOST_CTRL1_R_DAT_XFER_WIDTH_Msk 0x2UL
#define SDHC_CORE_HOST_CTRL1_R_HIGH_SPEED_EN_Pos 2UL
#define SDHC_CORE_HOST_CTRL1_R_HIGH_SPEED_EN_Msk 0x4UL
#define SDHC_CORE_HOST_CTRL1_R_DMA_SEL_Pos      3UL
#define SDHC_CORE_HOST_CTRL1_R_DMA_SEL_Msk      0x18UL
#define SDHC_CORE_HOST_CTRL1_R_EXT_DAT_XFER_Pos 5UL
#define SDHC_CORE_HOST_CTRL1_R_EXT_DAT_XFER_Msk 0x20UL
#define SDHC_CORE_HOST_CTRL1_R_CARD_DETECT_TEST_LVL_Pos 6UL
#define SDHC_CORE_HOST_CTRL1_R_CARD_DETECT_TEST_LVL_Msk 0x40UL
#define SDHC_CORE_HOST_CTRL1_R_CARD_DETECT_SIG_SEL_Pos 7UL
#define SDHC_CORE_HOST_CTRL1_R_CARD_DETECT_SIG_SEL_Msk 0x80UL
/* SDHC_CORE.PWR_CTRL_R */
#define SDHC_CORE_PWR_CTRL_R_SD_BUS_PWR_VDD1_Pos 0UL
#define SDHC_CORE_PWR_CTRL_R_SD_BUS_PWR_VDD1_Msk 0x1UL
#define SDHC_CORE_PWR_CTRL_R_SD_BUS_VOL_VDD1_Pos 1UL
#define SDHC_CORE_PWR_CTRL_R_SD_BUS_VOL_VDD1_Msk 0xEUL
/* SDHC_CORE.BGAP_CTRL_R */
#define SDHC_CORE_BGAP_CTRL_R_STOP_BG_REQ_Pos   0UL
#define SDHC_CORE_BGAP_CTRL_R_STOP_BG_REQ_Msk   0x1UL
#define SDHC_CORE_BGAP_CTRL_R_CONTINUE_REQ_Pos  1UL
#define SDHC_CORE_BGAP_CTRL_R_CONTINUE_REQ_Msk  0x2UL
#define SDHC_CORE_BGAP_CTRL_R_RD_WAIT_CTRL_Pos  2UL
#define SDHC_CORE_BGAP_CTRL_R_RD_WAIT_CTRL_Msk  0x4UL
#define SDHC_CORE_BGAP_CTRL_R_INT_AT_BGAP_Pos   3UL
#define SDHC_CORE_BGAP_CTRL_R_INT_AT_BGAP_Msk   0x8UL
/* SDHC_CORE.WUP_CTRL_R */
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_INT_Pos   0UL
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_INT_Msk   0x1UL
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_INSERT_Pos 1UL
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_INSERT_Msk 0x2UL
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_REMOVAL_Pos 2UL
#define SDHC_CORE_WUP_CTRL_R_WUP_CARD_REMOVAL_Msk 0x4UL
/* SDHC_CORE.CLK_CTRL_R */
#define SDHC_CORE_CLK_CTRL_R_INTERNAL_CLK_EN_Pos 0UL
#define SDHC_CORE_CLK_CTRL_R_INTERNAL_CLK_EN_Msk 0x1UL
#define SDHC_CORE_CLK_CTRL_R_INTERNAL_CLK_STABLE_Pos 1UL
#define SDHC_CORE_CLK_CTRL_R_INTERNAL_CLK_STABLE_Msk 0x2UL
#define SDHC_CORE_CLK_CTRL_R_SD_CLK_EN_Pos      2UL
#define SDHC_CORE_CLK_CTRL_R_SD_CLK_EN_Msk      0x4UL
#define SDHC_CORE_CLK_CTRL_R_PLL_ENABLE_Pos     3UL
#define SDHC_CORE_CLK_CTRL_R_PLL_ENABLE_Msk     0x8UL
#define SDHC_CORE_CLK_CTRL_R_CLK_GEN_SELECT_Pos 5UL
#define SDHC_CORE_CLK_CTRL_R_CLK_GEN_SELECT_Msk 0x20UL
#define SDHC_CORE_CLK_CTRL_R_UPPER_FREQ_SEL_Pos 6UL
#define SDHC_CORE_CLK_CTRL_R_UPPER_FREQ_SEL_Msk 0xC0UL
#define SDHC_CORE_CLK_CTRL_R_FREQ_SEL_Pos       8UL
#define SDHC_CORE_CLK_CTRL_R_FREQ_SEL_Msk       0xFF00UL
/* SDHC_CORE.TOUT_CTRL_R */
#define SDHC_CORE_TOUT_CTRL_R_TOUT_CNT_Pos      0UL
#define SDHC_CORE_TOUT_CTRL_R_TOUT_CNT_Msk      0xFUL
/* SDHC_CORE.SW_RST_R */
#define SDHC_CORE_SW_RST_R_SW_RST_ALL_Pos       0UL
#define SDHC_CORE_SW_RST_R_SW_RST_ALL_Msk       0x1UL
#define SDHC_CORE_SW_RST_R_SW_RST_CMD_Pos       1UL
#define SDHC_CORE_SW_RST_R_SW_RST_CMD_Msk       0x2UL
#define SDHC_CORE_SW_RST_R_SW_RST_DAT_Pos       2UL
#define SDHC_CORE_SW_RST_R_SW_RST_DAT_Msk       0x4UL
/* SDHC_CORE.NORMAL_INT_STAT_R */
#define SDHC_CORE_NORMAL_INT_STAT_R_CMD_COMPLETE_Pos 0UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CMD_COMPLETE_Msk 0x1UL
#define SDHC_CORE_NORMAL_INT_STAT_R_XFER_COMPLETE_Pos 1UL
#define SDHC_CORE_NORMAL_INT_STAT_R_XFER_COMPLETE_Msk 0x2UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BGAP_EVENT_Pos 2UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BGAP_EVENT_Msk 0x4UL
#define SDHC_CORE_NORMAL_INT_STAT_R_DMA_INTERRUPT_Pos 3UL
#define SDHC_CORE_NORMAL_INT_STAT_R_DMA_INTERRUPT_Msk 0x8UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BUF_WR_READY_Pos 4UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BUF_WR_READY_Msk 0x10UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BUF_RD_READY_Pos 5UL
#define SDHC_CORE_NORMAL_INT_STAT_R_BUF_RD_READY_Msk 0x20UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_INSERTION_Pos 6UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_INSERTION_Msk 0x40UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_REMOVAL_Pos 7UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_REMOVAL_Msk 0x80UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_INTERRUPT_Pos 8UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CARD_INTERRUPT_Msk 0x100UL
#define SDHC_CORE_NORMAL_INT_STAT_R_FX_EVENT_Pos 13UL
#define SDHC_CORE_NORMAL_INT_STAT_R_FX_EVENT_Msk 0x2000UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CQE_EVENT_Pos 14UL
#define SDHC_CORE_NORMAL_INT_STAT_R_CQE_EVENT_Msk 0x4000UL
#define SDHC_CORE_NORMAL_INT_STAT_R_ERR_INTERRUPT_Pos 15UL
#define SDHC_CORE_NORMAL_INT_STAT_R_ERR_INTERRUPT_Msk 0x8000UL
/* SDHC_CORE.ERROR_INT_STAT_R */
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_TOUT_ERR_Pos 0UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_TOUT_ERR_Msk 0x1UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_CRC_ERR_Pos 1UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_CRC_ERR_Msk 0x2UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_END_BIT_ERR_Pos 2UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_END_BIT_ERR_Msk 0x4UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_IDX_ERR_Pos 3UL
#define SDHC_CORE_ERROR_INT_STAT_R_CMD_IDX_ERR_Msk 0x8UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_TOUT_ERR_Pos 4UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_TOUT_ERR_Msk 0x10UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_CRC_ERR_Pos 5UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_CRC_ERR_Msk 0x20UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_END_BIT_ERR_Pos 6UL
#define SDHC_CORE_ERROR_INT_STAT_R_DATA_END_BIT_ERR_Msk 0x40UL
#define SDHC_CORE_ERROR_INT_STAT_R_CUR_LMT_ERR_Pos 7UL
#define SDHC_CORE_ERROR_INT_STAT_R_CUR_LMT_ERR_Msk 0x80UL
#define SDHC_CORE_ERROR_INT_STAT_R_AUTO_CMD_ERR_Pos 8UL
#define SDHC_CORE_ERROR_INT_STAT_R_AUTO_CMD_ERR_Msk 0x100UL
#define SDHC_CORE_ERROR_INT_STAT_R_ADMA_ERR_Pos 9UL
#define SDHC_CORE_ERROR_INT_STAT_R_ADMA_ERR_Msk 0x200UL
#define SDHC_CORE_ERROR_INT_STAT_R_TUNING_ERR_Pos 10UL
#define SDHC_CORE_ERROR_INT_STAT_R_TUNING_ERR_Msk 0x400UL
#define SDHC_CORE_ERROR_INT_STAT_R_RESP_ERR_Pos 11UL
#define SDHC_CORE_ERROR_INT_STAT_R_RESP_ERR_Msk 0x800UL
#define SDHC_CORE_ERROR_INT_STAT_R_BOOT_ACK_ERR_Pos 12UL
#define SDHC_CORE_ERROR_INT_STAT_R_BOOT_ACK_ERR_Msk 0x1000UL
/* SDHC_CORE.NORMAL_INT_STAT_EN_R */
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CMD_COMPLETE_STAT_EN_Pos 0UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CMD_COMPLETE_STAT_EN_Msk 0x1UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_XFER_COMPLETE_STAT_EN_Pos 1UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_XFER_COMPLETE_STAT_EN_Msk 0x2UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BGAP_EVENT_STAT_EN_Pos 2UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BGAP_EVENT_STAT_EN_Msk 0x4UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_DMA_INTERRUPT_STAT_EN_Pos 3UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_DMA_INTERRUPT_STAT_EN_Msk 0x8UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BUF_WR_READY_STAT_EN_Pos 4UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BUF_WR_READY_STAT_EN_Msk 0x10UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BUF_RD_READY_STAT_EN_Pos 5UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_BUF_RD_READY_STAT_EN_Msk 0x20UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_INSERTION_STAT_EN_Pos 6UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_INSERTION_STAT_EN_Msk 0x40UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_REMOVAL_STAT_EN_Pos 7UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_REMOVAL_STAT_EN_Msk 0x80UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_INTERRUPT_STAT_EN_Pos 8UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CARD_INTERRUPT_STAT_EN_Msk 0x100UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_A_STAT_EN_Pos 9UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_A_STAT_EN_Msk 0x200UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_B_STAT_EN_Pos 10UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_B_STAT_EN_Msk 0x400UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_C_STAT_EN_Pos 11UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_INT_C_STAT_EN_Msk 0x800UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_RE_TUNE_EVENT_STAT_EN_Pos 12UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_RE_TUNE_EVENT_STAT_EN_Msk 0x1000UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_FX_EVENT_STAT_EN_Pos 13UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_FX_EVENT_STAT_EN_Msk 0x2000UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CQE_EVENT_STAT_EN_Pos 14UL
#define SDHC_CORE_NORMAL_INT_STAT_EN_R_CQE_EVENT_STAT_EN_Msk 0x4000UL
/* SDHC_CORE.ERROR_INT_STAT_EN_R */
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_TOUT_ERR_STAT_EN_Pos 0UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_TOUT_ERR_STAT_EN_Msk 0x1UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_CRC_ERR_STAT_EN_Pos 1UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_CRC_ERR_STAT_EN_Msk 0x2UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_END_BIT_ERR_STAT_EN_Pos 2UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_END_BIT_ERR_STAT_EN_Msk 0x4UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_IDX_ERR_STAT_EN_Pos 3UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CMD_IDX_ERR_STAT_EN_Msk 0x8UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_TOUT_ERR_STAT_EN_Pos 4UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_TOUT_ERR_STAT_EN_Msk 0x10UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_CRC_ERR_STAT_EN_Pos 5UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_CRC_ERR_STAT_EN_Msk 0x20UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_END_BIT_ERR_STAT_EN_Pos 6UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_DATA_END_BIT_ERR_STAT_EN_Msk 0x40UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CUR_LMT_ERR_STAT_EN_Pos 7UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_CUR_LMT_ERR_STAT_EN_Msk 0x80UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_AUTO_CMD_ERR_STAT_EN_Pos 8UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_AUTO_CMD_ERR_STAT_EN_Msk 0x100UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_ADMA_ERR_STAT_EN_Pos 9UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_ADMA_ERR_STAT_EN_Msk 0x200UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_TUNING_ERR_STAT_EN_Pos 10UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_TUNING_ERR_STAT_EN_Msk 0x400UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_RESP_ERR_STAT_EN_Pos 11UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_RESP_ERR_STAT_EN_Msk 0x800UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_BOOT_ACK_ERR_STAT_EN_Pos 12UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_BOOT_ACK_ERR_STAT_EN_Msk 0x1000UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN1_Pos 13UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN1_Msk 0x2000UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN2_Pos 14UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN2_Msk 0x4000UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN3_Pos 15UL
#define SDHC_CORE_ERROR_INT_STAT_EN_R_VENDOR_ERR_STAT_EN3_Msk 0x8000UL
/* SDHC_CORE.NORMAL_INT_SIGNAL_EN_R */
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CMD_COMPLETE_SIGNAL_EN_Pos 0UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CMD_COMPLETE_SIGNAL_EN_Msk 0x1UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_XFER_COMPLETE_SIGNAL_EN_Pos 1UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_XFER_COMPLETE_SIGNAL_EN_Msk 0x2UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BGAP_EVENT_SIGNAL_EN_Pos 2UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BGAP_EVENT_SIGNAL_EN_Msk 0x4UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_DMA_INTERRUPT_SIGNAL_EN_Pos 3UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_DMA_INTERRUPT_SIGNAL_EN_Msk 0x8UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BUF_WR_READY_SIGNAL_EN_Pos 4UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BUF_WR_READY_SIGNAL_EN_Msk 0x10UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BUF_RD_READY_SIGNAL_EN_Pos 5UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_BUF_RD_READY_SIGNAL_EN_Msk 0x20UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_INSERTION_SIGNAL_EN_Pos 6UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_INSERTION_SIGNAL_EN_Msk 0x40UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_REMOVAL_SIGNAL_EN_Pos 7UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_REMOVAL_SIGNAL_EN_Msk 0x80UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_INTERRUPT_SIGNAL_EN_Pos 8UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CARD_INTERRUPT_SIGNAL_EN_Msk 0x100UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_A_SIGNAL_EN_Pos 9UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_A_SIGNAL_EN_Msk 0x200UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_B_SIGNAL_EN_Pos 10UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_B_SIGNAL_EN_Msk 0x400UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_C_SIGNAL_EN_Pos 11UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_INT_C_SIGNAL_EN_Msk 0x800UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_RE_TUNE_EVENT_SIGNAL_EN_Pos 12UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_RE_TUNE_EVENT_SIGNAL_EN_Msk 0x1000UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_FX_EVENT_SIGNAL_EN_Pos 13UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_FX_EVENT_SIGNAL_EN_Msk 0x2000UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CQE_EVENT_SIGNAL_EN_Pos 14UL
#define SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_CQE_EVENT_SIGNAL_EN_Msk 0x4000UL
/* SDHC_CORE.ERROR_INT_SIGNAL_EN_R */
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_TOUT_ERR_SIGNAL_EN_Pos 0UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_TOUT_ERR_SIGNAL_EN_Msk 0x1UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_CRC_ERR_SIGNAL_EN_Pos 1UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_CRC_ERR_SIGNAL_EN_Msk 0x2UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_END_BIT_ERR_SIGNAL_EN_Pos 2UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_END_BIT_ERR_SIGNAL_EN_Msk 0x4UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_IDX_ERR_SIGNAL_EN_Pos 3UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CMD_IDX_ERR_SIGNAL_EN_Msk 0x8UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_TOUT_ERR_SIGNAL_EN_Pos 4UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_TOUT_ERR_SIGNAL_EN_Msk 0x10UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_CRC_ERR_SIGNAL_EN_Pos 5UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_CRC_ERR_SIGNAL_EN_Msk 0x20UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_END_BIT_ERR_SIGNAL_EN_Pos 6UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_DATA_END_BIT_ERR_SIGNAL_EN_Msk 0x40UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CUR_LMT_ERR_SIGNAL_EN_Pos 7UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_CUR_LMT_ERR_SIGNAL_EN_Msk 0x80UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_AUTO_CMD_ERR_SIGNAL_EN_Pos 8UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_AUTO_CMD_ERR_SIGNAL_EN_Msk 0x100UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_ADMA_ERR_SIGNAL_EN_Pos 9UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_ADMA_ERR_SIGNAL_EN_Msk 0x200UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_TUNING_ERR_SIGNAL_EN_Pos 10UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_TUNING_ERR_SIGNAL_EN_Msk 0x400UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_RESP_ERR_SIGNAL_EN_Pos 11UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_RESP_ERR_SIGNAL_EN_Msk 0x800UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_BOOT_ACK_ERR_SIGNAL_EN_Pos 12UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_BOOT_ACK_ERR_SIGNAL_EN_Msk 0x1000UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN1_Pos 13UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN1_Msk 0x2000UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN2_Pos 14UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN2_Msk 0x4000UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN3_Pos 15UL
#define SDHC_CORE_ERROR_INT_SIGNAL_EN_R_VENDOR_ERR_SIGNAL_EN3_Msk 0x8000UL
/* SDHC_CORE.AUTO_CMD_STAT_R */
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD12_NOT_EXEC_Pos 0UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD12_NOT_EXEC_Msk 0x1UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_TOUT_ERR_Pos 1UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_TOUT_ERR_Msk 0x2UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_CRC_ERR_Pos 2UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_CRC_ERR_Msk 0x4UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_EBIT_ERR_Pos 3UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_EBIT_ERR_Msk 0x8UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_IDX_ERR_Pos 4UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_IDX_ERR_Msk 0x10UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_RESP_ERR_Pos 5UL
#define SDHC_CORE_AUTO_CMD_STAT_R_AUTO_CMD_RESP_ERR_Msk 0x20UL
#define SDHC_CORE_AUTO_CMD_STAT_R_CMD_NOT_ISSUED_AUTO_CMD12_Pos 7UL
#define SDHC_CORE_AUTO_CMD_STAT_R_CMD_NOT_ISSUED_AUTO_CMD12_Msk 0x80UL
/* SDHC_CORE.HOST_CTRL2_R */
#define SDHC_CORE_HOST_CTRL2_R_UHS_MODE_SEL_Pos 0UL
#define SDHC_CORE_HOST_CTRL2_R_UHS_MODE_SEL_Msk 0x7UL
#define SDHC_CORE_HOST_CTRL2_R_SIGNALING_EN_Pos 3UL
#define SDHC_CORE_HOST_CTRL2_R_SIGNALING_EN_Msk 0x8UL
#define SDHC_CORE_HOST_CTRL2_R_DRV_STRENGTH_SEL_Pos 4UL
#define SDHC_CORE_HOST_CTRL2_R_DRV_STRENGTH_SEL_Msk 0x30UL
#define SDHC_CORE_HOST_CTRL2_R_EXEC_TUNING_Pos  6UL
#define SDHC_CORE_HOST_CTRL2_R_EXEC_TUNING_Msk  0x40UL
#define SDHC_CORE_HOST_CTRL2_R_SAMPLE_CLK_SEL_Pos 7UL
#define SDHC_CORE_HOST_CTRL2_R_SAMPLE_CLK_SEL_Msk 0x80UL
#define SDHC_CORE_HOST_CTRL2_R_UHS2_IF_ENABLE_Pos 8UL
#define SDHC_CORE_HOST_CTRL2_R_UHS2_IF_ENABLE_Msk 0x100UL
#define SDHC_CORE_HOST_CTRL2_R_ADMA2_LEN_MODE_Pos 10UL
#define SDHC_CORE_HOST_CTRL2_R_ADMA2_LEN_MODE_Msk 0x400UL
#define SDHC_CORE_HOST_CTRL2_R_CMD23_ENABLE_Pos 11UL
#define SDHC_CORE_HOST_CTRL2_R_CMD23_ENABLE_Msk 0x800UL
#define SDHC_CORE_HOST_CTRL2_R_HOST_VER4_ENABLE_Pos 12UL
#define SDHC_CORE_HOST_CTRL2_R_HOST_VER4_ENABLE_Msk 0x1000UL
#define SDHC_CORE_HOST_CTRL2_R_ADDRESSING_Pos   13UL
#define SDHC_CORE_HOST_CTRL2_R_ADDRESSING_Msk   0x2000UL
#define SDHC_CORE_HOST_CTRL2_R_ASYNC_INT_ENABLE_Pos 14UL
#define SDHC_CORE_HOST_CTRL2_R_ASYNC_INT_ENABLE_Msk 0x4000UL
#define SDHC_CORE_HOST_CTRL2_R_PRESET_VAL_ENABLE_Pos 15UL
#define SDHC_CORE_HOST_CTRL2_R_PRESET_VAL_ENABLE_Msk 0x8000UL
/* SDHC_CORE.CAPABILITIES1_R */
#define SDHC_CORE_CAPABILITIES1_R_TOUT_CLK_FREQ_Pos 0UL
#define SDHC_CORE_CAPABILITIES1_R_TOUT_CLK_FREQ_Msk 0x3FUL
#define SDHC_CORE_CAPABILITIES1_R_TOUT_CLK_UNIT_Pos 7UL
#define SDHC_CORE_CAPABILITIES1_R_TOUT_CLK_UNIT_Msk 0x80UL
#define SDHC_CORE_CAPABILITIES1_R_BASE_CLK_FREQ_Pos 8UL
#define SDHC_CORE_CAPABILITIES1_R_BASE_CLK_FREQ_Msk 0xFF00UL
#define SDHC_CORE_CAPABILITIES1_R_MAX_BLK_LEN_Pos 16UL
#define SDHC_CORE_CAPABILITIES1_R_MAX_BLK_LEN_Msk 0x30000UL
#define SDHC_CORE_CAPABILITIES1_R_EMBEDDED_8_BIT_Pos 18UL
#define SDHC_CORE_CAPABILITIES1_R_EMBEDDED_8_BIT_Msk 0x40000UL
#define SDHC_CORE_CAPABILITIES1_R_ADMA2_SUPPORT_Pos 19UL
#define SDHC_CORE_CAPABILITIES1_R_ADMA2_SUPPORT_Msk 0x80000UL
#define SDHC_CORE_CAPABILITIES1_R_HIGH_SPEED_SUPPORT_Pos 21UL
#define SDHC_CORE_CAPABILITIES1_R_HIGH_SPEED_SUPPORT_Msk 0x200000UL
#define SDHC_CORE_CAPABILITIES1_R_SDMA_SUPPORT_Pos 22UL
#define SDHC_CORE_CAPABILITIES1_R_SDMA_SUPPORT_Msk 0x400000UL
#define SDHC_CORE_CAPABILITIES1_R_SUS_RES_SUPPORT_Pos 23UL
#define SDHC_CORE_CAPABILITIES1_R_SUS_RES_SUPPORT_Msk 0x800000UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_33_Pos   24UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_33_Msk   0x1000000UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_30_Pos   25UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_30_Msk   0x2000000UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_18_Pos   26UL
#define SDHC_CORE_CAPABILITIES1_R_VOLT_18_Msk   0x4000000UL
#define SDHC_CORE_CAPABILITIES1_R_SYS_ADDR_64_V4_Pos 27UL
#define SDHC_CORE_CAPABILITIES1_R_SYS_ADDR_64_V4_Msk 0x8000000UL
#define SDHC_CORE_CAPABILITIES1_R_SYS_ADDR_64_V3_Pos 28UL
#define SDHC_CORE_CAPABILITIES1_R_SYS_ADDR_64_V3_Msk 0x10000000UL
#define SDHC_CORE_CAPABILITIES1_R_ASYNC_INT_SUPPORT_Pos 29UL
#define SDHC_CORE_CAPABILITIES1_R_ASYNC_INT_SUPPORT_Msk 0x20000000UL
#define SDHC_CORE_CAPABILITIES1_R_SLOT_TYPE_R_Pos 30UL
#define SDHC_CORE_CAPABILITIES1_R_SLOT_TYPE_R_Msk 0xC0000000UL
/* SDHC_CORE.CAPABILITIES2_R */
#define SDHC_CORE_CAPABILITIES2_R_SDR50_SUPPORT_Pos 0UL
#define SDHC_CORE_CAPABILITIES2_R_SDR50_SUPPORT_Msk 0x1UL
#define SDHC_CORE_CAPABILITIES2_R_SDR104_SUPPORT_Pos 1UL
#define SDHC_CORE_CAPABILITIES2_R_SDR104_SUPPORT_Msk 0x2UL
#define SDHC_CORE_CAPABILITIES2_R_DDR50_SUPPORT_Pos 2UL
#define SDHC_CORE_CAPABILITIES2_R_DDR50_SUPPORT_Msk 0x4UL
#define SDHC_CORE_CAPABILITIES2_R_UHS2_SUPPORT_Pos 3UL
#define SDHC_CORE_CAPABILITIES2_R_UHS2_SUPPORT_Msk 0x8UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPEA_Pos 4UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPEA_Msk 0x10UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPEC_Pos 5UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPEC_Msk 0x20UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPED_Pos 6UL
#define SDHC_CORE_CAPABILITIES2_R_DRV_TYPED_Msk 0x40UL
#define SDHC_CORE_CAPABILITIES2_R_RETUNE_CNT_Pos 8UL
#define SDHC_CORE_CAPABILITIES2_R_RETUNE_CNT_Msk 0xF00UL
#define SDHC_CORE_CAPABILITIES2_R_USE_TUNING_SDR50_Pos 13UL
#define SDHC_CORE_CAPABILITIES2_R_USE_TUNING_SDR50_Msk 0x2000UL
#define SDHC_CORE_CAPABILITIES2_R_RE_TUNING_MODES_Pos 14UL
#define SDHC_CORE_CAPABILITIES2_R_RE_TUNING_MODES_Msk 0xC000UL
#define SDHC_CORE_CAPABILITIES2_R_CLK_MUL_Pos   16UL
#define SDHC_CORE_CAPABILITIES2_R_CLK_MUL_Msk   0xFF0000UL
#define SDHC_CORE_CAPABILITIES2_R_ADMA3_SUPPORT_Pos 27UL
#define SDHC_CORE_CAPABILITIES2_R_ADMA3_SUPPORT_Msk 0x8000000UL
#define SDHC_CORE_CAPABILITIES2_R_VDD2_18V_SUPPORT_Pos 28UL
#define SDHC_CORE_CAPABILITIES2_R_VDD2_18V_SUPPORT_Msk 0x10000000UL
/* SDHC_CORE.CURR_CAPABILITIES1_R */
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_33V_Pos 0UL
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_33V_Msk 0xFFUL
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_30V_Pos 8UL
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_30V_Msk 0xFF00UL
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_18V_Pos 16UL
#define SDHC_CORE_CURR_CAPABILITIES1_R_MAX_CUR_18V_Msk 0xFF0000UL
/* SDHC_CORE.CURR_CAPABILITIES2_R */
#define SDHC_CORE_CURR_CAPABILITIES2_R_MAX_CUR_VDD2_18V_Pos 0UL
#define SDHC_CORE_CURR_CAPABILITIES2_R_MAX_CUR_VDD2_18V_Msk 0xFFUL
/* SDHC_CORE.FORCE_AUTO_CMD_STAT_R */
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD12_NOT_EXEC_Pos 0UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD12_NOT_EXEC_Msk 0x1UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_TOUT_ERR_Pos 1UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_TOUT_ERR_Msk 0x2UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_CRC_ERR_Pos 2UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_CRC_ERR_Msk 0x4UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_EBIT_ERR_Pos 3UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_EBIT_ERR_Msk 0x8UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_IDX_ERR_Pos 4UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_IDX_ERR_Msk 0x10UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_RESP_ERR_Pos 5UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_AUTO_CMD_RESP_ERR_Msk 0x20UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_CMD_NOT_ISSUED_AUTO_CMD12_Pos 7UL
#define SDHC_CORE_FORCE_AUTO_CMD_STAT_R_FORCE_CMD_NOT_ISSUED_AUTO_CMD12_Msk 0x80UL
/* SDHC_CORE.FORCE_ERROR_INT_STAT_R */
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_TOUT_ERR_Pos 0UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_TOUT_ERR_Msk 0x1UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_CRC_ERR_Pos 1UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_CRC_ERR_Msk 0x2UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_END_BIT_ERR_Pos 2UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_END_BIT_ERR_Msk 0x4UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_IDX_ERR_Pos 3UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CMD_IDX_ERR_Msk 0x8UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_TOUT_ERR_Pos 4UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_TOUT_ERR_Msk 0x10UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_CRC_ERR_Pos 5UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_CRC_ERR_Msk 0x20UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_END_BIT_ERR_Pos 6UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_DATA_END_BIT_ERR_Msk 0x40UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CUR_LMT_ERR_Pos 7UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_CUR_LMT_ERR_Msk 0x80UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_AUTO_CMD_ERR_Pos 8UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_AUTO_CMD_ERR_Msk 0x100UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_ADMA_ERR_Pos 9UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_ADMA_ERR_Msk 0x200UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_TUNING_ERR_Pos 10UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_TUNING_ERR_Msk 0x400UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_RESP_ERR_Pos 11UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_RESP_ERR_Msk 0x800UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_BOOT_ACK_ERR_Pos 12UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_BOOT_ACK_ERR_Msk 0x1000UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR1_Pos 13UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR1_Msk 0x2000UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR2_Pos 14UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR2_Msk 0x4000UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR3_Pos 15UL
#define SDHC_CORE_FORCE_ERROR_INT_STAT_R_FORCE_VENDOR_ERR3_Msk 0x8000UL
/* SDHC_CORE.ADMA_ERR_STAT_R */
#define SDHC_CORE_ADMA_ERR_STAT_R_ADMA_ERR_STATES_Pos 0UL
#define SDHC_CORE_ADMA_ERR_STAT_R_ADMA_ERR_STATES_Msk 0x3UL
#define SDHC_CORE_ADMA_ERR_STAT_R_ADMA_LEN_ERR_Pos 2UL
#define SDHC_CORE_ADMA_ERR_STAT_R_ADMA_LEN_ERR_Msk 0x4UL
/* SDHC_CORE.ADMA_SA_LOW_R */
#define SDHC_CORE_ADMA_SA_LOW_R_ADMA_SA_LOW_Pos 0UL
#define SDHC_CORE_ADMA_SA_LOW_R_ADMA_SA_LOW_Msk 0xFFFFFFFFUL
/* SDHC_CORE.ADMA_ID_LOW_R */
#define SDHC_CORE_ADMA_ID_LOW_R_ADMA_ID_LOW_Pos 0UL
#define SDHC_CORE_ADMA_ID_LOW_R_ADMA_ID_LOW_Msk 0xFFFFFFFFUL
/* SDHC_CORE.HOST_CNTRL_VERS_R */
#define SDHC_CORE_HOST_CNTRL_VERS_R_SPEC_VERSION_NUM_Pos 0UL
#define SDHC_CORE_HOST_CNTRL_VERS_R_SPEC_VERSION_NUM_Msk 0xFFUL
#define SDHC_CORE_HOST_CNTRL_VERS_R_VENDOR_VERSION_NUM_Pos 8UL
#define SDHC_CORE_HOST_CNTRL_VERS_R_VENDOR_VERSION_NUM_Msk 0xFF00UL
/* SDHC_CORE.CQVER */
#define SDHC_CORE_CQVER_EMMC_VER_SUFFIX_Pos     0UL
#define SDHC_CORE_CQVER_EMMC_VER_SUFFIX_Msk     0xFUL
#define SDHC_CORE_CQVER_EMMC_VER_MINOR_Pos      4UL
#define SDHC_CORE_CQVER_EMMC_VER_MINOR_Msk      0xF0UL
#define SDHC_CORE_CQVER_EMMC_VER_MAJOR_Pos      8UL
#define SDHC_CORE_CQVER_EMMC_VER_MAJOR_Msk      0xF00UL
/* SDHC_CORE.CQCAP */
#define SDHC_CORE_CQCAP_ITCFVAL_Pos             0UL
#define SDHC_CORE_CQCAP_ITCFVAL_Msk             0x3FFUL
#define SDHC_CORE_CQCAP_ITCFMUL_Pos             12UL
#define SDHC_CORE_CQCAP_ITCFMUL_Msk             0xF000UL
#define SDHC_CORE_CQCAP_CRYPTO_SUPPORT_Pos      28UL
#define SDHC_CORE_CQCAP_CRYPTO_SUPPORT_Msk      0x10000000UL
/* SDHC_CORE.CQCFG */
#define SDHC_CORE_CQCFG_CQ_EN_Pos               0UL
#define SDHC_CORE_CQCFG_CQ_EN_Msk               0x1UL
#define SDHC_CORE_CQCFG_CR_GENERAL_EN_Pos       1UL
#define SDHC_CORE_CQCFG_CR_GENERAL_EN_Msk       0x2UL
#define SDHC_CORE_CQCFG_TASK_DESC_SIZE_Pos      8UL
#define SDHC_CORE_CQCFG_TASK_DESC_SIZE_Msk      0x100UL
#define SDHC_CORE_CQCFG_DCMD_EN_Pos             12UL
#define SDHC_CORE_CQCFG_DCMD_EN_Msk             0x1000UL
/* SDHC_CORE.CQCTL */
#define SDHC_CORE_CQCTL_HALT_Pos                0UL
#define SDHC_CORE_CQCTL_HALT_Msk                0x1UL
#define SDHC_CORE_CQCTL_CLR_ALL_TASKS_Pos       8UL
#define SDHC_CORE_CQCTL_CLR_ALL_TASKS_Msk       0x100UL
/* SDHC_CORE.CQIS */
#define SDHC_CORE_CQIS_HAC_Pos                  0UL
#define SDHC_CORE_CQIS_HAC_Msk                  0x1UL
#define SDHC_CORE_CQIS_TCC_Pos                  1UL
#define SDHC_CORE_CQIS_TCC_Msk                  0x2UL
#define SDHC_CORE_CQIS_RED_Pos                  2UL
#define SDHC_CORE_CQIS_RED_Msk                  0x4UL
#define SDHC_CORE_CQIS_TCL_Pos                  3UL
#define SDHC_CORE_CQIS_TCL_Msk                  0x8UL
#define SDHC_CORE_CQIS_GCE_Pos                  4UL
#define SDHC_CORE_CQIS_GCE_Msk                  0x10UL
#define SDHC_CORE_CQIS_ICCE_Pos                 5UL
#define SDHC_CORE_CQIS_ICCE_Msk                 0x20UL
/* SDHC_CORE.CQISE */
#define SDHC_CORE_CQISE_HAC_STE_Pos             0UL
#define SDHC_CORE_CQISE_HAC_STE_Msk             0x1UL
#define SDHC_CORE_CQISE_TCC_STE_Pos             1UL
#define SDHC_CORE_CQISE_TCC_STE_Msk             0x2UL
#define SDHC_CORE_CQISE_RED_STE_Pos             2UL
#define SDHC_CORE_CQISE_RED_STE_Msk             0x4UL
#define SDHC_CORE_CQISE_TCL_STE_Pos             3UL
#define SDHC_CORE_CQISE_TCL_STE_Msk             0x8UL
#define SDHC_CORE_CQISE_GCE_STE_Pos             4UL
#define SDHC_CORE_CQISE_GCE_STE_Msk             0x10UL
#define SDHC_CORE_CQISE_ICCE_STE_Pos            5UL
#define SDHC_CORE_CQISE_ICCE_STE_Msk            0x20UL
/* SDHC_CORE.CQISGE */
#define SDHC_CORE_CQISGE_HAC_SGE_Pos            0UL
#define SDHC_CORE_CQISGE_HAC_SGE_Msk            0x1UL
#define SDHC_CORE_CQISGE_TCC_SGE_Pos            1UL
#define SDHC_CORE_CQISGE_TCC_SGE_Msk            0x2UL
#define SDHC_CORE_CQISGE_RED_SGE_Pos            2UL
#define SDHC_CORE_CQISGE_RED_SGE_Msk            0x4UL
#define SDHC_CORE_CQISGE_TCL_SGE_Pos            3UL
#define SDHC_CORE_CQISGE_TCL_SGE_Msk            0x8UL
#define SDHC_CORE_CQISGE_GCE_SGE_Pos            4UL
#define SDHC_CORE_CQISGE_GCE_SGE_Msk            0x10UL
#define SDHC_CORE_CQISGE_ICCE_SGE_Pos           5UL
#define SDHC_CORE_CQISGE_ICCE_SGE_Msk           0x20UL
/* SDHC_CORE.CQIC */
#define SDHC_CORE_CQIC_TOUT_VAL_Pos             0UL
#define SDHC_CORE_CQIC_TOUT_VAL_Msk             0x7FUL
#define SDHC_CORE_CQIC_TOUT_VAL_WEN_Pos         7UL
#define SDHC_CORE_CQIC_TOUT_VAL_WEN_Msk         0x80UL
#define SDHC_CORE_CQIC_INTC_TH_Pos              8UL
#define SDHC_CORE_CQIC_INTC_TH_Msk              0x1F00UL
#define SDHC_CORE_CQIC_INTC_TH_WEN_Pos          15UL
#define SDHC_CORE_CQIC_INTC_TH_WEN_Msk          0x8000UL
#define SDHC_CORE_CQIC_INTC_RST_Pos             16UL
#define SDHC_CORE_CQIC_INTC_RST_Msk             0x10000UL
#define SDHC_CORE_CQIC_INTC_STAT_Pos            20UL
#define SDHC_CORE_CQIC_INTC_STAT_Msk            0x100000UL
#define SDHC_CORE_CQIC_INTC_EN_Pos              31UL
#define SDHC_CORE_CQIC_INTC_EN_Msk              0x80000000UL
/* SDHC_CORE.CQTDLBA */
#define SDHC_CORE_CQTDLBA_TDLBA_Pos             0UL
#define SDHC_CORE_CQTDLBA_TDLBA_Msk             0xFFFFFFFFUL
/* SDHC_CORE.CQTDBR */
#define SDHC_CORE_CQTDBR_DBR_Pos                0UL
#define SDHC_CORE_CQTDBR_DBR_Msk                0xFFFFFFFFUL
/* SDHC_CORE.CQTCN */
#define SDHC_CORE_CQTCN_TCN_Pos                 0UL
#define SDHC_CORE_CQTCN_TCN_Msk                 0xFFFFFFFFUL
/* SDHC_CORE.CQDQS */
#define SDHC_CORE_CQDQS_DQS_Pos                 0UL
#define SDHC_CORE_CQDQS_DQS_Msk                 0xFFFFFFFFUL
/* SDHC_CORE.CQDPT */
#define SDHC_CORE_CQDPT_DPT_Pos                 0UL
#define SDHC_CORE_CQDPT_DPT_Msk                 0xFFFFFFFFUL
/* SDHC_CORE.CQTCLR */
#define SDHC_CORE_CQTCLR_TCLR_Pos               0UL
#define SDHC_CORE_CQTCLR_TCLR_Msk               0xFFFFFFFFUL
/* SDHC_CORE.CQSSC1 */
#define SDHC_CORE_CQSSC1_SQSCMD_IDLE_TMR_Pos    0UL
#define SDHC_CORE_CQSSC1_SQSCMD_IDLE_TMR_Msk    0xFFFFUL
#define SDHC_CORE_CQSSC1_SQSCMD_BLK_CNT_Pos     16UL
#define SDHC_CORE_CQSSC1_SQSCMD_BLK_CNT_Msk     0xF0000UL
/* SDHC_CORE.CQSSC2 */
#define SDHC_CORE_CQSSC2_SQSCMD_RCA_Pos         0UL
#define SDHC_CORE_CQSSC2_SQSCMD_RCA_Msk         0xFFFFUL
/* SDHC_CORE.CQCRDCT */
#define SDHC_CORE_CQCRDCT_DCMD_RESP_Pos         0UL
#define SDHC_CORE_CQCRDCT_DCMD_RESP_Msk         0xFFFFFFFFUL
/* SDHC_CORE.CQRMEM */
#define SDHC_CORE_CQRMEM_RESP_ERR_MASK_Pos      0UL
#define SDHC_CORE_CQRMEM_RESP_ERR_MASK_Msk      0xFFFFFFFFUL
/* SDHC_CORE.CQTERRI */
#define SDHC_CORE_CQTERRI_RESP_ERR_CMD_INDX_Pos 0UL
#define SDHC_CORE_CQTERRI_RESP_ERR_CMD_INDX_Msk 0x3FUL
#define SDHC_CORE_CQTERRI_RESP_ERR_TASKID_Pos   8UL
#define SDHC_CORE_CQTERRI_RESP_ERR_TASKID_Msk   0x1F00UL
#define SDHC_CORE_CQTERRI_RESP_ERR_FIELDS_VALID_Pos 15UL
#define SDHC_CORE_CQTERRI_RESP_ERR_FIELDS_VALID_Msk 0x8000UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_CMD_INDX_Pos 16UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_CMD_INDX_Msk 0x3F0000UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_TASKID_Pos  24UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_TASKID_Msk  0x1F000000UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_FIELDS_VALID_Pos 31UL
#define SDHC_CORE_CQTERRI_TRANS_ERR_FIELDS_VALID_Msk 0x80000000UL
/* SDHC_CORE.CQCRI */
#define SDHC_CORE_CQCRI_CMD_RESP_INDX_Pos       0UL
#define SDHC_CORE_CQCRI_CMD_RESP_INDX_Msk       0x3FUL
/* SDHC_CORE.CQCRA */
#define SDHC_CORE_CQCRA_CMD_RESP_ARG_Pos        0UL
#define SDHC_CORE_CQCRA_CMD_RESP_ARG_Msk        0xFFFFFFFFUL
/* SDHC_CORE.MSHC_VER_ID_R */
#define SDHC_CORE_MSHC_VER_ID_R_MSHC_VER_ID_Pos 0UL
#define SDHC_CORE_MSHC_VER_ID_R_MSHC_VER_ID_Msk 0xFFFFFFFFUL
/* SDHC_CORE.MSHC_VER_TYPE_R */
#define SDHC_CORE_MSHC_VER_TYPE_R_MSHC_VER_TYPE_Pos 0UL
#define SDHC_CORE_MSHC_VER_TYPE_R_MSHC_VER_TYPE_Msk 0xFFFFFFFFUL
/* SDHC_CORE.MSHC_CTRL_R */
#define SDHC_CORE_MSHC_CTRL_R_CMD_CONFLICT_CHECK_Pos 0UL
#define SDHC_CORE_MSHC_CTRL_R_CMD_CONFLICT_CHECK_Msk 0x1UL
#define SDHC_CORE_MSHC_CTRL_R_SW_CG_DIS_Pos     4UL
#define SDHC_CORE_MSHC_CTRL_R_SW_CG_DIS_Msk     0x10UL
/* SDHC_CORE.MBIU_CTRL_R */
#define SDHC_CORE_MBIU_CTRL_R_UNDEFL_INCR_EN_Pos 0UL
#define SDHC_CORE_MBIU_CTRL_R_UNDEFL_INCR_EN_Msk 0x1UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR4_EN_Pos 1UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR4_EN_Msk 0x2UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR8_EN_Pos 2UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR8_EN_Msk 0x4UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR16_EN_Pos 3UL
#define SDHC_CORE_MBIU_CTRL_R_BURST_INCR16_EN_Msk 0x8UL
/* SDHC_CORE.EMMC_CTRL_R */
#define SDHC_CORE_EMMC_CTRL_R_CARD_IS_EMMC_Pos  0UL
#define SDHC_CORE_EMMC_CTRL_R_CARD_IS_EMMC_Msk  0x1UL
#define SDHC_CORE_EMMC_CTRL_R_DISABLE_DATA_CRC_CHK_Pos 1UL
#define SDHC_CORE_EMMC_CTRL_R_DISABLE_DATA_CRC_CHK_Msk 0x2UL
#define SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N_Pos    2UL
#define SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N_Msk    0x4UL
#define SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N_OE_Pos 3UL
#define SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N_OE_Msk 0x8UL
#define SDHC_CORE_EMMC_CTRL_R_CQE_ALGO_SEL_Pos  9UL
#define SDHC_CORE_EMMC_CTRL_R_CQE_ALGO_SEL_Msk  0x200UL
#define SDHC_CORE_EMMC_CTRL_R_CQE_PREFETCH_DISABLE_Pos 10UL
#define SDHC_CORE_EMMC_CTRL_R_CQE_PREFETCH_DISABLE_Msk 0x400UL
/* SDHC_CORE.BOOT_CTRL_R */
#define SDHC_CORE_BOOT_CTRL_R_MAN_BOOT_EN_Pos   0UL
#define SDHC_CORE_BOOT_CTRL_R_MAN_BOOT_EN_Msk   0x1UL
#define SDHC_CORE_BOOT_CTRL_R_VALIDATE_BOOT_Pos 7UL
#define SDHC_CORE_BOOT_CTRL_R_VALIDATE_BOOT_Msk 0x80UL
#define SDHC_CORE_BOOT_CTRL_R_BOOT_ACK_ENABLE_Pos 8UL
#define SDHC_CORE_BOOT_CTRL_R_BOOT_ACK_ENABLE_Msk 0x100UL
#define SDHC_CORE_BOOT_CTRL_R_BOOT_TOUT_CNT_Pos 12UL
#define SDHC_CORE_BOOT_CTRL_R_BOOT_TOUT_CNT_Msk 0xF000UL
/* SDHC_CORE.GP_IN_R */
#define SDHC_CORE_GP_IN_R_GP_IN_Pos             0UL
#define SDHC_CORE_GP_IN_R_GP_IN_Msk             0x1UL
/* SDHC_CORE.GP_OUT_R */
#define SDHC_CORE_GP_OUT_R_CARD_DETECT_EN_Pos   0UL
#define SDHC_CORE_GP_OUT_R_CARD_DETECT_EN_Msk   0x1UL
#define SDHC_CORE_GP_OUT_R_CARD_MECH_WRITE_PROT_EN_Pos 1UL
#define SDHC_CORE_GP_OUT_R_CARD_MECH_WRITE_PROT_EN_Msk 0x2UL
#define SDHC_CORE_GP_OUT_R_LED_CTRL_OE_Pos      2UL
#define SDHC_CORE_GP_OUT_R_LED_CTRL_OE_Msk      0x4UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_OE_Pos    3UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_OE_Msk    0x8UL
#define SDHC_CORE_GP_OUT_R_CARD_IF_PWR_EN_OE_Pos 4UL
#define SDHC_CORE_GP_OUT_R_CARD_IF_PWR_EN_OE_Msk 0x10UL
#define SDHC_CORE_GP_OUT_R_IO_VOLT_SEL_OE_Pos   5UL
#define SDHC_CORE_GP_OUT_R_IO_VOLT_SEL_OE_Msk   0x20UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_OUT_DLY_Pos 6UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_OUT_DLY_Msk 0xC0UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_IN_DLY_Pos 8UL
#define SDHC_CORE_GP_OUT_R_CARD_CLOCK_IN_DLY_Msk 0x300UL


#endif /* _CYIP_SDHC_H_ */


/* [] END OF FILE */
