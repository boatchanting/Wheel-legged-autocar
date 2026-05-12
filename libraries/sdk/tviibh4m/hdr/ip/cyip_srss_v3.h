/***************************************************************************//**
* \file cyip_srss_v3.h
*
* \brief
* SRSS IP definitions
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

#ifndef _CYIP_SRSS_V3_H_
#define _CYIP_SRSS_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     SRSS
*******************************************************************************/

#define CSV_HF_CSV_SECTION_SIZE                 0x00000010UL
#define CSV_HF_SECTION_SIZE                     0x00000100UL
#define CSV_REF_CSV_SECTION_SIZE                0x00000010UL
#define CSV_REF_SECTION_SIZE                    0x00000010UL
#define CSV_LF_CSV_SECTION_SIZE                 0x00000010UL
#define CSV_LF_SECTION_SIZE                     0x00000010UL
#define CSV_ILO_CSV_SECTION_SIZE                0x00000010UL
#define CSV_ILO_SECTION_SIZE                    0x00000010UL
#define CLK_PLL400M_SECTION_SIZE                0x00000010UL
#define MCWDT_CTR_SECTION_SIZE                  0x00000020UL
#define MCWDT_SECTION_SIZE                      0x00000100UL
#define WDT_SECTION_SIZE                        0x00000080UL
#define SRSS_SECTION_SIZE                       0x00010000UL

/**
  * \brief Clock Supervision Reference Control (CSV_HF_CSV_REF_CTL)
  */
typedef struct stc_CSV_HF_CSV_REF_CTL_field {
  uint32_t                         u16STARTUP:16;
  uint32_t                         :14;
  uint32_t                         u1CSV_ACTION:1;
  uint32_t                         u1CSV_EN:1;
} stc_CSV_HF_CSV_REF_CTL_field_t;

typedef union un_CSV_HF_CSV_REF_CTL {
  uint32_t                         u32Register;
  stc_CSV_HF_CSV_REF_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_HF_CSV_REF_CTL_t;

/**
  * \brief Clock Supervision Reference Limits (CSV_HF_CSV_REF_LIMIT)
  */
typedef struct stc_CSV_HF_CSV_REF_LIMIT_field {
  uint32_t                         u16LOWER:16;
  uint32_t                         u16UPPER:16;
} stc_CSV_HF_CSV_REF_LIMIT_field_t;

typedef union un_CSV_HF_CSV_REF_LIMIT {
  uint32_t                         u32Register;
  stc_CSV_HF_CSV_REF_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_HF_CSV_REF_LIMIT_t;

/**
  * \brief Clock Supervision Monitor Control (CSV_HF_CSV_MON_CTL)
  */
typedef struct stc_CSV_HF_CSV_MON_CTL_field {
  uint32_t                         u16PERIOD:16;
  uint32_t                         :16;
} stc_CSV_HF_CSV_MON_CTL_field_t;

typedef union un_CSV_HF_CSV_MON_CTL {
  uint32_t                         u32Register;
  stc_CSV_HF_CSV_MON_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_HF_CSV_MON_CTL_t;



/**
  * \brief Clock Supervision Reference Control (CSV_REF_CSV_REF_CTL)
  */
typedef struct stc_CSV_REF_CSV_REF_CTL_field {
  uint32_t                         u16STARTUP:16;
  uint32_t                         :14;
  uint32_t                         u1CSV_ACTION:1;
  uint32_t                         u1CSV_EN:1;
} stc_CSV_REF_CSV_REF_CTL_field_t;

typedef union un_CSV_REF_CSV_REF_CTL {
  uint32_t                         u32Register;
  stc_CSV_REF_CSV_REF_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_REF_CSV_REF_CTL_t;

/**
  * \brief Clock Supervision Reference Limits (CSV_REF_CSV_REF_LIMIT)
  */
typedef struct stc_CSV_REF_CSV_REF_LIMIT_field {
  uint32_t                         u16LOWER:16;
  uint32_t                         u16UPPER:16;
} stc_CSV_REF_CSV_REF_LIMIT_field_t;

typedef union un_CSV_REF_CSV_REF_LIMIT {
  uint32_t                         u32Register;
  stc_CSV_REF_CSV_REF_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_REF_CSV_REF_LIMIT_t;

/**
  * \brief Clock Supervision Monitor Control (CSV_REF_CSV_MON_CTL)
  */
typedef struct stc_CSV_REF_CSV_MON_CTL_field {
  uint32_t                         u16PERIOD:16;
  uint32_t                         :16;
} stc_CSV_REF_CSV_MON_CTL_field_t;

typedef union un_CSV_REF_CSV_MON_CTL {
  uint32_t                         u32Register;
  stc_CSV_REF_CSV_MON_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_REF_CSV_MON_CTL_t;



/**
  * \brief Clock Supervision Reference Control (CSV_LF_CSV_REF_CTL)
  */
typedef struct stc_CSV_LF_CSV_REF_CTL_field {
  uint32_t                         u8STARTUP:8;
  uint32_t                         :23;
  uint32_t                         u1CSV_EN:1;
} stc_CSV_LF_CSV_REF_CTL_field_t;

typedef union un_CSV_LF_CSV_REF_CTL {
  uint32_t                         u32Register;
  stc_CSV_LF_CSV_REF_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_LF_CSV_REF_CTL_t;

/**
  * \brief Clock Supervision Reference Limits (CSV_LF_CSV_REF_LIMIT)
  */
typedef struct stc_CSV_LF_CSV_REF_LIMIT_field {
  uint32_t                         u8LOWER:8;
  uint32_t                         :8;
  uint32_t                         u8UPPER:8;
  uint32_t                         :8;
} stc_CSV_LF_CSV_REF_LIMIT_field_t;

typedef union un_CSV_LF_CSV_REF_LIMIT {
  uint32_t                         u32Register;
  stc_CSV_LF_CSV_REF_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_LF_CSV_REF_LIMIT_t;

/**
  * \brief Clock Supervision Monitor Control (CSV_LF_CSV_MON_CTL)
  */
typedef struct stc_CSV_LF_CSV_MON_CTL_field {
  uint32_t                         u8PERIOD:8;
  uint32_t                         :24;
} stc_CSV_LF_CSV_MON_CTL_field_t;

typedef union un_CSV_LF_CSV_MON_CTL {
  uint32_t                         u32Register;
  stc_CSV_LF_CSV_MON_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_LF_CSV_MON_CTL_t;



/**
  * \brief Clock Supervision Reference Control (CSV_ILO_CSV_REF_CTL)
  */
typedef struct stc_CSV_ILO_CSV_REF_CTL_field {
  uint32_t                         u8STARTUP:8;
  uint32_t                         :23;
  uint32_t                         u1CSV_EN:1;
} stc_CSV_ILO_CSV_REF_CTL_field_t;

typedef union un_CSV_ILO_CSV_REF_CTL {
  uint32_t                         u32Register;
  stc_CSV_ILO_CSV_REF_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_ILO_CSV_REF_CTL_t;

/**
  * \brief Clock Supervision Reference Limits (CSV_ILO_CSV_REF_LIMIT)
  */
typedef struct stc_CSV_ILO_CSV_REF_LIMIT_field {
  uint32_t                         u8LOWER:8;
  uint32_t                         :8;
  uint32_t                         u8UPPER:8;
  uint32_t                         :8;
} stc_CSV_ILO_CSV_REF_LIMIT_field_t;

typedef union un_CSV_ILO_CSV_REF_LIMIT {
  uint32_t                         u32Register;
  stc_CSV_ILO_CSV_REF_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_ILO_CSV_REF_LIMIT_t;

/**
  * \brief Clock Supervision Monitor Control (CSV_ILO_CSV_MON_CTL)
  */
typedef struct stc_CSV_ILO_CSV_MON_CTL_field {
  uint32_t                         u8PERIOD:8;
  uint32_t                         :24;
} stc_CSV_ILO_CSV_MON_CTL_field_t;

typedef union un_CSV_ILO_CSV_MON_CTL {
  uint32_t                         u32Register;
  stc_CSV_ILO_CSV_MON_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_ILO_CSV_MON_CTL_t;



/**
  * \brief 400MHz PLL Configuration Register (CLK_PLL400M_CONFIG)
  */
typedef struct stc_CLK_PLL400M_CONFIG_field {
  uint32_t                         u8FEEDBACK_DIV:8;
  uint32_t                         u5REFERENCE_DIV:5;
  uint32_t                         :3;
  uint32_t                         u5OUTPUT_DIV:5;
  uint32_t                         :4;
  uint32_t                         u2LOCK_DELAY:2;
  uint32_t                         :1;
  uint32_t                         u2BYPASS_SEL:2;
  uint32_t                         :1;
  uint32_t                         u1ENABLE:1;
} stc_CLK_PLL400M_CONFIG_field_t;

typedef union un_CLK_PLL400M_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_PLL400M_CONFIG_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL400M_CONFIG_t;

/**
  * \brief 400MHz PLL Configuration Register 2 (CLK_PLL400M_CONFIG2)
  */
typedef struct stc_CLK_PLL400M_CONFIG2_field {
  uint32_t                         u24FRAC_DIV:24;
  uint32_t                         :4;
  uint32_t                         u3FRAC_DITHER_EN:3;
  uint32_t                         u1FRAC_EN:1;
} stc_CLK_PLL400M_CONFIG2_field_t;

typedef union un_CLK_PLL400M_CONFIG2 {
  uint32_t                         u32Register;
  stc_CLK_PLL400M_CONFIG2_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL400M_CONFIG2_t;

/**
  * \brief 400MHz PLL Configuration Register 3 (CLK_PLL400M_CONFIG3)
  */
typedef struct stc_CLK_PLL400M_CONFIG3_field {
  uint32_t                         u10SSCG_DEPTH:10;
  uint32_t                         :6;
  uint32_t                         u3SSCG_RATE:3;
  uint32_t                         :5;
  uint32_t                         u1SSCG_DITHER_EN:1;
  uint32_t                         :3;
  uint32_t                         u1SSCG_MODE:1;
  uint32_t                         :2;
  uint32_t                         u1SSCG_EN:1;
} stc_CLK_PLL400M_CONFIG3_field_t;

typedef union un_CLK_PLL400M_CONFIG3 {
  uint32_t                         u32Register;
  stc_CLK_PLL400M_CONFIG3_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL400M_CONFIG3_t;

/**
  * \brief 400MHz PLL Status Register (CLK_PLL400M_STATUS)
  */
typedef struct stc_CLK_PLL400M_STATUS_field {
  uint32_t                         u1LOCKED:1;
  uint32_t                         u1UNLOCK_OCCURRED:1;
  uint32_t                         :30;
} stc_CLK_PLL400M_STATUS_field_t;

typedef union un_CLK_PLL400M_STATUS {
  uint32_t                         u32Register;
  stc_CLK_PLL400M_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL400M_STATUS_t;



/**
  * \brief MCWDT Subcounter Control Register (MCWDT_CTR_CTL)
  */
typedef struct stc_MCWDT_CTR_CTL_field {
  uint32_t                         u1ENABLED:1;
  uint32_t                         :30;
  uint32_t                         u1ENABLE:1;
} stc_MCWDT_CTR_CTL_field_t;

typedef union un_MCWDT_CTR_CTL {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_CTL_t;

/**
  * \brief MCWDT Subcounter Lower Limit Register (MCWDT_CTR_LOWER_LIMIT)
  */
typedef struct stc_MCWDT_CTR_LOWER_LIMIT_field {
  uint32_t                         u16LOWER_LIMIT:16;
  uint32_t                         :16;
} stc_MCWDT_CTR_LOWER_LIMIT_field_t;

typedef union un_MCWDT_CTR_LOWER_LIMIT {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_LOWER_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_LOWER_LIMIT_t;

/**
  * \brief MCWDT Subcounter Upper Limit Register (MCWDT_CTR_UPPER_LIMIT)
  */
typedef struct stc_MCWDT_CTR_UPPER_LIMIT_field {
  uint32_t                         u16UPPER_LIMIT:16;
  uint32_t                         :16;
} stc_MCWDT_CTR_UPPER_LIMIT_field_t;

typedef union un_MCWDT_CTR_UPPER_LIMIT {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_UPPER_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_UPPER_LIMIT_t;

/**
  * \brief MCWDT Subcounter Warn Limit Register (MCWDT_CTR_WARN_LIMIT)
  */
typedef struct stc_MCWDT_CTR_WARN_LIMIT_field {
  uint32_t                         u16WARN_LIMIT:16;
  uint32_t                         :16;
} stc_MCWDT_CTR_WARN_LIMIT_field_t;

typedef union un_MCWDT_CTR_WARN_LIMIT {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_WARN_LIMIT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_WARN_LIMIT_t;

/**
  * \brief MCWDT Subcounter Configuration Register (MCWDT_CTR_CONFIG)
  */
typedef struct stc_MCWDT_CTR_CONFIG_field {
  uint32_t                         u2LOWER_ACTION:2;
  uint32_t                         :2;
  uint32_t                         u2UPPER_ACTION:2;
  uint32_t                         :2;
  uint32_t                         u1WARN_ACTION:1;
  uint32_t                         :3;
  uint32_t                         u1AUTO_SERVICE:1;
  uint32_t                         :15;
  uint32_t                         u1DEBUG_TRIGGER_EN:1;
  uint32_t                         :1;
  uint32_t                         u1SLEEPDEEP_PAUSE:1;
  uint32_t                         u1DEBUG_RUN:1;
} stc_MCWDT_CTR_CONFIG_field_t;

typedef union un_MCWDT_CTR_CONFIG {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_CONFIG_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_CONFIG_t;

/**
  * \brief MCWDT Subcounter Count Register (MCWDT_CTR_CNT)
  */
typedef struct stc_MCWDT_CTR_CNT_field {
  uint32_t                         u16CNT:16;
  uint32_t                         :16;
} stc_MCWDT_CTR_CNT_field_t;

typedef union un_MCWDT_CTR_CNT {
  uint32_t                         u32Register;
  stc_MCWDT_CTR_CNT_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR_CNT_t;



/**
  * \brief MCWDT CPU selection register (MCWDT_CPU_SELECT)
  */
typedef struct stc_MCWDT_CPU_SELECT_field {
  uint32_t                         u2CPU_SEL:2;
  uint32_t                         :30;
} stc_MCWDT_CPU_SELECT_field_t;

typedef union un_MCWDT_CPU_SELECT {
  uint32_t                         u32Register;
  stc_MCWDT_CPU_SELECT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CPU_SELECT_t;

/**
  * \brief MCWDT Subcounter 2 Control register (MCWDT_CTR2_CTL)
  */
typedef struct stc_MCWDT_CTR2_CTL_field {
  uint32_t                         u1ENABLED:1;
  uint32_t                         :30;
  uint32_t                         u1ENABLE:1;
} stc_MCWDT_CTR2_CTL_field_t;

typedef union un_MCWDT_CTR2_CTL {
  uint32_t                         u32Register;
  stc_MCWDT_CTR2_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR2_CTL_t;

/**
  * \brief MCWDT Subcounter 2 Configuration register (MCWDT_CTR2_CONFIG)
  */
typedef struct stc_MCWDT_CTR2_CONFIG_field {
  uint32_t                         u1ACTION:1;
  uint32_t                         :15;
  uint32_t                         u5BITS:5;
  uint32_t                         :7;
  uint32_t                         u1DEBUG_TRIGGER_EN:1;
  uint32_t                         :1;
  uint32_t                         u1SLEEPDEEP_PAUSE:1;
  uint32_t                         u1DEBUG_RUN:1;
} stc_MCWDT_CTR2_CONFIG_field_t;

typedef union un_MCWDT_CTR2_CONFIG {
  uint32_t                         u32Register;
  stc_MCWDT_CTR2_CONFIG_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR2_CONFIG_t;

/**
  * \brief MCWDT Subcounter 2 Count Register (MCWDT_CTR2_CNT)
  */
typedef struct stc_MCWDT_CTR2_CNT_field {
  uint32_t                         u32CNT2:32;
} stc_MCWDT_CTR2_CNT_field_t;

typedef union un_MCWDT_CTR2_CNT {
  uint32_t                         u32Register;
  stc_MCWDT_CTR2_CNT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_CTR2_CNT_t;

/**
  * \brief MCWDT Lock Register (MCWDT_LOCK)
  */
typedef struct stc_MCWDT_LOCK_field {
  uint32_t                         u2MCWDT_LOCK:2;
  uint32_t                         :30;
} stc_MCWDT_LOCK_field_t;

typedef union un_MCWDT_LOCK {
  uint32_t                         u32Register;
  stc_MCWDT_LOCK_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_LOCK_t;

/**
  * \brief MCWDT Service Register (MCWDT_SERVICE)
  */
typedef struct stc_MCWDT_SERVICE_field {
  uint32_t                         u1CTR0_SERVICE:1;
  uint32_t                         u1CTR1_SERVICE:1;
  uint32_t                         :30;
} stc_MCWDT_SERVICE_field_t;

typedef union un_MCWDT_SERVICE {
  uint32_t                         u32Register;
  stc_MCWDT_SERVICE_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_SERVICE_t;

/**
  * \brief MCWDT Interrupt Register (MCWDT_INTR)
  */
typedef struct stc_MCWDT_INTR_field {
  uint32_t                         u1CTR0_INT:1;
  uint32_t                         u1CTR1_INT:1;
  uint32_t                         u1CTR2_INT:1;
  uint32_t                         :29;
} stc_MCWDT_INTR_field_t;

typedef union un_MCWDT_INTR {
  uint32_t                         u32Register;
  stc_MCWDT_INTR_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_INTR_t;

/**
  * \brief MCWDT Interrupt Set Register (MCWDT_INTR_SET)
  */
typedef struct stc_MCWDT_INTR_SET_field {
  uint32_t                         u1CTR0_INT:1;
  uint32_t                         u1CTR1_INT:1;
  uint32_t                         u1CTR2_INT:1;
  uint32_t                         :29;
} stc_MCWDT_INTR_SET_field_t;

typedef union un_MCWDT_INTR_SET {
  uint32_t                         u32Register;
  stc_MCWDT_INTR_SET_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_INTR_SET_t;

/**
  * \brief MCWDT Interrupt Mask Register (MCWDT_INTR_MASK)
  */
typedef struct stc_MCWDT_INTR_MASK_field {
  uint32_t                         u1CTR0_INT:1;
  uint32_t                         u1CTR1_INT:1;
  uint32_t                         u1CTR2_INT:1;
  uint32_t                         :29;
} stc_MCWDT_INTR_MASK_field_t;

typedef union un_MCWDT_INTR_MASK {
  uint32_t                         u32Register;
  stc_MCWDT_INTR_MASK_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_INTR_MASK_t;

/**
  * \brief MCWDT Interrupt Masked Register (MCWDT_INTR_MASKED)
  */
typedef struct stc_MCWDT_INTR_MASKED_field {
  uint32_t                         u1CTR0_INT:1;
  uint32_t                         u1CTR1_INT:1;
  uint32_t                         u1CTR2_INT:1;
  uint32_t                         :29;
} stc_MCWDT_INTR_MASKED_field_t;

typedef union un_MCWDT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_MCWDT_INTR_MASKED_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_MCWDT_INTR_MASKED_t;



/**
  * \brief WDT Control Register (WDT_CTL)
  */
typedef struct stc_WDT_CTL_field {
  uint32_t                         u1ENABLED:1;
  uint32_t                         :30;
  uint32_t                         u1ENABLE:1;
} stc_WDT_CTL_field_t;

typedef union un_WDT_CTL {
  uint32_t                         u32Register;
  stc_WDT_CTL_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_CTL_t;

/**
  * \brief WDT Lower Limit Register (WDT_LOWER_LIMIT)
  */
typedef struct stc_WDT_LOWER_LIMIT_field {
  uint32_t                         u32LOWER_LIMIT:32;
} stc_WDT_LOWER_LIMIT_field_t;

typedef union un_WDT_LOWER_LIMIT {
  uint32_t                         u32Register;
  stc_WDT_LOWER_LIMIT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_LOWER_LIMIT_t;

/**
  * \brief WDT Upper Limit Register (WDT_UPPER_LIMIT)
  */
typedef struct stc_WDT_UPPER_LIMIT_field {
  uint32_t                         u32UPPER_LIMIT:32;
} stc_WDT_UPPER_LIMIT_field_t;

typedef union un_WDT_UPPER_LIMIT {
  uint32_t                         u32Register;
  stc_WDT_UPPER_LIMIT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_UPPER_LIMIT_t;

/**
  * \brief WDT Warn Limit Register (WDT_WARN_LIMIT)
  */
typedef struct stc_WDT_WARN_LIMIT_field {
  uint32_t                         u32WARN_LIMIT:32;
} stc_WDT_WARN_LIMIT_field_t;

typedef union un_WDT_WARN_LIMIT {
  uint32_t                         u32Register;
  stc_WDT_WARN_LIMIT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_WARN_LIMIT_t;

/**
  * \brief WDT Configuration Register (WDT_CONFIG)
  */
typedef struct stc_WDT_CONFIG_field {
  uint32_t                         u1LOWER_ACTION:1;
  uint32_t                         :3;
  uint32_t                         u1UPPER_ACTION:1;
  uint32_t                         :3;
  uint32_t                         u1WARN_ACTION:1;
  uint32_t                         :3;
  uint32_t                         u1AUTO_SERVICE:1;
  uint32_t                         :15;
  uint32_t                         u1DEBUG_TRIGGER_EN:1;
  uint32_t                         u1DPSLP_PAUSE:1;
  uint32_t                         u1HIB_PAUSE:1;
  uint32_t                         u1DEBUG_RUN:1;
} stc_WDT_CONFIG_field_t;

typedef union un_WDT_CONFIG {
  uint32_t                         u32Register;
  stc_WDT_CONFIG_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_CONFIG_t;

/**
  * \brief WDT Count Register (WDT_CNT)
  */
typedef struct stc_WDT_CNT_field {
  uint32_t                         u32CNT:32;
} stc_WDT_CNT_field_t;

typedef union un_WDT_CNT {
  uint32_t                         u32Register;
  stc_WDT_CNT_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_CNT_t;

/**
  * \brief WDT Lock register (WDT_LOCK)
  */
typedef struct stc_WDT_LOCK_field {
  uint32_t                         u2WDT_LOCK:2;
  uint32_t                         :30;
} stc_WDT_LOCK_field_t;

typedef union un_WDT_LOCK {
  uint32_t                         u32Register;
  stc_WDT_LOCK_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_LOCK_t;

/**
  * \brief WDT Service register (WDT_SERVICE)
  */
typedef struct stc_WDT_SERVICE_field {
  uint32_t                         u1SERVICE:1;
  uint32_t                         :31;
} stc_WDT_SERVICE_field_t;

typedef union un_WDT_SERVICE {
  uint32_t                         u32Register;
  stc_WDT_SERVICE_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_SERVICE_t;

/**
  * \brief WDT Interrupt Register (WDT_INTR)
  */
typedef struct stc_WDT_INTR_field {
  uint32_t                         u1WDT:1;
  uint32_t                         :31;
} stc_WDT_INTR_field_t;

typedef union un_WDT_INTR {
  uint32_t                         u32Register;
  stc_WDT_INTR_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_INTR_t;

/**
  * \brief WDT Interrupt Set Register (WDT_INTR_SET)
  */
typedef struct stc_WDT_INTR_SET_field {
  uint32_t                         u1WDT:1;
  uint32_t                         :31;
} stc_WDT_INTR_SET_field_t;

typedef union un_WDT_INTR_SET {
  uint32_t                         u32Register;
  stc_WDT_INTR_SET_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_INTR_SET_t;

/**
  * \brief WDT Interrupt Mask Register (WDT_INTR_MASK)
  */
typedef struct stc_WDT_INTR_MASK_field {
  uint32_t                         u1WDT:1;
  uint32_t                         :31;
} stc_WDT_INTR_MASK_field_t;

typedef union un_WDT_INTR_MASK {
  uint32_t                         u32Register;
  stc_WDT_INTR_MASK_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_INTR_MASK_t;

/**
  * \brief WDT Interrupt Masked Register (WDT_INTR_MASKED)
  */
typedef struct stc_WDT_INTR_MASKED_field {
  uint32_t                         u1WDT:1;
  uint32_t                         :31;
} stc_WDT_INTR_MASKED_field_t;

typedef union un_WDT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_WDT_INTR_MASKED_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_WDT_INTR_MASKED_t;



/**
  * \brief High Voltage / Low Voltage Detector (HVLVD) Status Register (PWR_LVD_STATUS)
  */
typedef struct stc_PWR_LVD_STATUS_field {
  uint32_t                         u1HVLVD1_OUT:1;
  uint32_t                         :31;
} stc_PWR_LVD_STATUS_field_t;

typedef union un_PWR_LVD_STATUS {
  uint32_t                         u32Register;
  stc_PWR_LVD_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_LVD_STATUS_t;

/**
  * \brief High Voltage / Low Voltage Detector (HVLVD) Status Register #2 (PWR_LVD_STATUS2)
  */
typedef struct stc_PWR_LVD_STATUS2_field {
  uint32_t                         u1HVLVD2_OUT:1;
  uint32_t                         :31;
} stc_PWR_LVD_STATUS2_field_t;

typedef union un_PWR_LVD_STATUS2 {
  uint32_t                         u32Register;
  stc_PWR_LVD_STATUS2_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_LVD_STATUS2_t;

/**
  * \brief Clock DSI Select Register (CLK_DSI_SELECT)
  */
typedef struct stc_CLK_DSI_SELECT_field {
  uint32_t                         u5DSI_MUX:5;
  uint32_t                         :27;
} stc_CLK_DSI_SELECT_field_t;

typedef union un_CLK_DSI_SELECT {
  uint32_t                         u32Register;
  stc_CLK_DSI_SELECT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_DSI_SELECT_t;

/**
  * \brief Fast Clock Output Select Register (CLK_OUTPUT_FAST)
  */
typedef struct stc_CLK_OUTPUT_FAST_field {
  uint32_t                         u4FAST_SEL0:4;
  uint32_t                         u4PATH_SEL0:4;
  uint32_t                         u4HFCLK_SEL0:4;
  uint32_t                         :4;
  uint32_t                         u4FAST_SEL1:4;
  uint32_t                         u4PATH_SEL1:4;
  uint32_t                         u4HFCLK_SEL1:4;
  uint32_t                         :4;
} stc_CLK_OUTPUT_FAST_field_t;

typedef union un_CLK_OUTPUT_FAST {
  uint32_t                         u32Register;
  stc_CLK_OUTPUT_FAST_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_OUTPUT_FAST_t;

/**
  * \brief Slow Clock Output Select Register (CLK_OUTPUT_SLOW)
  */
typedef struct stc_CLK_OUTPUT_SLOW_field {
  uint32_t                         u4SLOW_SEL0:4;
  uint32_t                         u4SLOW_SEL1:4;
  uint32_t                         :24;
} stc_CLK_OUTPUT_SLOW_field_t;

typedef union un_CLK_OUTPUT_SLOW {
  uint32_t                         u32Register;
  stc_CLK_OUTPUT_SLOW_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_OUTPUT_SLOW_t;

/**
  * \brief Clock Calibration Counter 1 (CLK_CAL_CNT1)
  */
typedef struct stc_CLK_CAL_CNT1_field {
  uint32_t                         u24CAL_COUNTER1:24;
  uint32_t                         :7;
  uint32_t                         u1CAL_COUNTER_DONE:1;
} stc_CLK_CAL_CNT1_field_t;

typedef union un_CLK_CAL_CNT1 {
  uint32_t                         u32Register;
  stc_CLK_CAL_CNT1_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_CAL_CNT1_t;

/**
  * \brief Clock Calibration Counter 2 (CLK_CAL_CNT2)
  */
typedef struct stc_CLK_CAL_CNT2_field {
  uint32_t                         u24CAL_COUNTER2:24;
  uint32_t                         :8;
} stc_CLK_CAL_CNT2_field_t;

typedef union un_CLK_CAL_CNT2 {
  uint32_t                         u32Register;
  stc_CLK_CAL_CNT2_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_CAL_CNT2_t;

/**
  * \brief SRSS Interrupt Register (SRSS_INTR)
  */
typedef struct stc_SRSS_INTR_field {
  uint32_t                         :1;
  uint32_t                         u1HVLVD1:1;
  uint32_t                         u1HVLVD2:1;
  uint32_t                         :2;
  uint32_t                         u1CLK_CAL:1;
  uint32_t                         :26;
} stc_SRSS_INTR_field_t;

typedef union un_SRSS_INTR {
  uint32_t                         u32Register;
  stc_SRSS_INTR_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SRSS_INTR_t;

/**
  * \brief SRSS Interrupt Set Register (SRSS_INTR_SET)
  */
typedef struct stc_SRSS_INTR_SET_field {
  uint32_t                         :1;
  uint32_t                         u1HVLVD1:1;
  uint32_t                         u1HVLVD2:1;
  uint32_t                         :2;
  uint32_t                         u1CLK_CAL:1;
  uint32_t                         :26;
} stc_SRSS_INTR_SET_field_t;

typedef union un_SRSS_INTR_SET {
  uint32_t                         u32Register;
  stc_SRSS_INTR_SET_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SRSS_INTR_SET_t;

/**
  * \brief SRSS Interrupt Mask Register (SRSS_INTR_MASK)
  */
typedef struct stc_SRSS_INTR_MASK_field {
  uint32_t                         :1;
  uint32_t                         u1HVLVD1:1;
  uint32_t                         u1HVLVD2:1;
  uint32_t                         :2;
  uint32_t                         u1CLK_CAL:1;
  uint32_t                         :26;
} stc_SRSS_INTR_MASK_field_t;

typedef union un_SRSS_INTR_MASK {
  uint32_t                         u32Register;
  stc_SRSS_INTR_MASK_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SRSS_INTR_MASK_t;

/**
  * \brief SRSS Interrupt Masked Register (SRSS_INTR_MASKED)
  */
typedef struct stc_SRSS_INTR_MASKED_field {
  uint32_t                         :1;
  uint32_t                         u1HVLVD1:1;
  uint32_t                         u1HVLVD2:1;
  uint32_t                         :2;
  uint32_t                         u1CLK_CAL:1;
  uint32_t                         :26;
} stc_SRSS_INTR_MASKED_field_t;

typedef union un_SRSS_INTR_MASKED {
  uint32_t                         u32Register;
  stc_SRSS_INTR_MASKED_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SRSS_INTR_MASKED_t;

/**
  * \brief Power Mode Control (PWR_CTL)
  */
typedef struct stc_PWR_CTL_field {
  uint32_t                         u2POWER_MODE:2;
  uint32_t                         :2;
  uint32_t                         u1DEBUG_SESSION:1;
  uint32_t                         u1LPM_READY:1;
  uint32_t                         :26;
} stc_PWR_CTL_field_t;

typedef union un_PWR_CTL {
  uint32_t                         u32Register;
  stc_PWR_CTL_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_CTL_t;

/**
  * \brief Power Mode Control 2 (PWR_CTL2)
  */
typedef struct stc_PWR_CTL2_field {
  uint32_t                         u1LINREG_DIS:1;
  uint32_t                         u1LINREG_OK:1;
  uint32_t                         u1LINREG_LPMODE:1;
  uint32_t                         :1;
  uint32_t                         u1DPSLP_REG_DIS:1;
  uint32_t                         :3;
  uint32_t                         u1RET_REG_DIS:1;
  uint32_t                         :3;
  uint32_t                         u1NWELL_REG_DIS:1;
  uint32_t                         :3;
  uint32_t                         u1REFV_DIS:1;
  uint32_t                         u1REFV_OK:1;
  uint32_t                         :2;
  uint32_t                         u1REFVBUF_DIS:1;
  uint32_t                         u1REFVBUF_OK:1;
  uint32_t                         u1REFVBUF_LPMODE:1;
  uint32_t                         :1;
  uint32_t                         u1REFI_DIS:1;
  uint32_t                         u1REFI_OK:1;
  uint32_t                         u1REFI_LPMODE:1;
  uint32_t                         u1PORBOD_LPMODE:1;
  uint32_t                         u1BGREF_LPMODE:1;
  uint32_t                         :2;
  uint32_t                         u1PLL_LS_BYPASS:1;
} stc_PWR_CTL2_field_t;

typedef union un_PWR_CTL2 {
  uint32_t                         u32Register;
  stc_PWR_CTL2_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_CTL2_t;

/**
  * \brief HIBERNATE Mode Register (PWR_HIBERNATE)
  */
typedef struct stc_PWR_HIBERNATE_field {
  uint32_t                         u8TOKEN:8;
  uint32_t                         u8UNLOCK:8;
  uint32_t                         :1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         u1MASK_HIBALARM:1;
  uint32_t                         u1MASK_HIBWDT:1;
  uint32_t                         u4POLARITY_HIBPIN:4;
  uint32_t                         u4MASK_HIBPIN:4;
  uint32_t                         :2;
  uint32_t                         u1HIBERNATE_DISABLE:1;
  uint32_t                         u1HIBERNATE:1;
} stc_PWR_HIBERNATE_field_t;

typedef union un_PWR_HIBERNATE {
  uint32_t                         u32Register;
  stc_PWR_HIBERNATE_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_HIBERNATE_t;

/**
  * \brief Buck Control Register (PWR_BUCK_CTL)
  */
typedef struct stc_PWR_BUCK_CTL_field {
  uint32_t                         u3BUCK_OUT1_SEL:3;
  uint32_t                         :27;
  uint32_t                         u1BUCK_EN:1;
  uint32_t                         u1BUCK_OUT1_EN:1;
} stc_PWR_BUCK_CTL_field_t;

typedef union un_PWR_BUCK_CTL {
  uint32_t                         u32Register;
  stc_PWR_BUCK_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_BUCK_CTL_t;

/**
  * \brief Buck Control Register 2 (PWR_BUCK_CTL2)
  */
typedef struct stc_PWR_BUCK_CTL2_field {
  uint32_t                         u3BUCK_OUT2_SEL:3;
  uint32_t                         :27;
  uint32_t                         u1BUCK_OUT2_HW_SEL:1;
  uint32_t                         u1BUCK_OUT2_EN:1;
} stc_PWR_BUCK_CTL2_field_t;

typedef union un_PWR_BUCK_CTL2 {
  uint32_t                         u32Register;
  stc_PWR_BUCK_CTL2_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_BUCK_CTL2_t;

/**
  * \brief Supply Supervision Control Register (PWR_SSV_CTL)
  */
typedef struct stc_PWR_SSV_CTL_field {
  uint32_t                         u1BODVDDD_VSEL:1;
  uint32_t                         :2;
  uint32_t                         u1BODVDDD_ENABLE:1;
  uint32_t                         u1BODVDDA_VSEL:1;
  uint32_t                         :1;
  uint32_t                         u2BODVDDA_ACTION:2;
  uint32_t                         u1BODVDDA_ENABLE:1;
  uint32_t                         :2;
  uint32_t                         u1BODVCCD_ENABLE:1;
  uint32_t                         :4;
  uint32_t                         u1OVDVDDD_VSEL:1;
  uint32_t                         :2;
  uint32_t                         u1OVDVDDD_ENABLE:1;
  uint32_t                         u1OVDVDDA_VSEL:1;
  uint32_t                         :1;
  uint32_t                         u2OVDVDDA_ACTION:2;
  uint32_t                         u1OVDVDDA_ENABLE:1;
  uint32_t                         :2;
  uint32_t                         u1OVDVCCD_ENABLE:1;
  uint32_t                         :4;
} stc_PWR_SSV_CTL_field_t;

typedef union un_PWR_SSV_CTL {
  uint32_t                         u32Register;
  stc_PWR_SSV_CTL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_SSV_CTL_t;

/**
  * \brief Supply Supervision Status Register (PWR_SSV_STATUS)
  */
typedef struct stc_PWR_SSV_STATUS_field {
  uint32_t                         u1BODVDDD_OK:1;
  uint32_t                         u1BODVDDA_OK:1;
  uint32_t                         u1BODVCCD_OK:1;
  uint32_t                         :5;
  uint32_t                         u1OVDVDDD_OK:1;
  uint32_t                         u1OVDVDDA_OK:1;
  uint32_t                         u1OVDVCCD_OK:1;
  uint32_t                         :5;
  uint32_t                         u1OCD_ACT_LINREG_OK:1;
  uint32_t                         u1OCD_DPSLP_REG_OK:1;
  uint32_t                         :14;
} stc_PWR_SSV_STATUS_field_t;

typedef union un_PWR_SSV_STATUS {
  uint32_t                         u32Register;
  stc_PWR_SSV_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_SSV_STATUS_t;

/**
  * \brief High Voltage / Low Voltage Detector (HVLVD) Configuration Register (PWR_LVD_CTL)
  */
typedef struct stc_PWR_LVD_CTL_field {
  uint32_t                         u4HVLVD1_TRIPSEL:4;
  uint32_t                         u3HVLVD1_SRCSEL:3;
  uint32_t                         u1HVLVD1_EN:1;
  uint32_t                         u5HVLVD1_TRIPSEL_HT:5;
  uint32_t                         :1;
  uint32_t                         u1HVLVD1_DPSLP_EN_HT:1;
  uint32_t                         u1HVLVD1_EN_HT:1;
  uint32_t                         u2HVLVD1_EDGE_SEL:2;
  uint32_t                         u1HVLVD1_ACTION:1;
  uint32_t                         :13;
} stc_PWR_LVD_CTL_field_t;

typedef union un_PWR_LVD_CTL {
  uint32_t                         u32Register;
  stc_PWR_LVD_CTL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_LVD_CTL_t;

/**
  * \brief High Voltage / Low Voltage Detector (HVLVD) Configuration Register #2 (PWR_LVD_CTL2)
  */
typedef struct stc_PWR_LVD_CTL2_field {
  uint32_t                         :8;
  uint32_t                         u5HVLVD2_TRIPSEL_HT:5;
  uint32_t                         :1;
  uint32_t                         u1HVLVD2_DPSLP_EN_HT:1;
  uint32_t                         u1HVLVD2_EN_HT:1;
  uint32_t                         u2HVLVD2_EDGE_SEL:2;
  uint32_t                         u1HVLVD2_ACTION:1;
  uint32_t                         :13;
} stc_PWR_LVD_CTL2_field_t;

typedef union un_PWR_LVD_CTL2 {
  uint32_t                         u32Register;
  stc_PWR_LVD_CTL2_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_LVD_CTL2_t;

/**
  * \brief REGHC Control Register (PWR_REGHC_CTL)
  */
typedef struct stc_PWR_REGHC_CTL_field {
  uint32_t                         u1REGHC_MODE:1;
  uint32_t                         :1;
  uint32_t                         u2REGHC_PMIC_DRV_VOUT:2;
  uint32_t                         u5REGHC_VADJ:5;
  uint32_t                         :1;
  uint32_t                         u1REGHC_PMIC_USE_LINREG:1;
  uint32_t                         u1REGHC_PMIC_USE_RADJ:1;
  uint32_t                         u3REGHC_PMIC_RADJ:3;
  uint32_t                         :1;
  uint32_t                         u1REGHC_PMIC_CTL_OUTEN:1;
  uint32_t                         u1REGHC_PMIC_CTL_POLARITY:1;
  uint32_t                         u1REGHC_PMIC_STATUS_INEN:1;
  uint32_t                         u1REGHC_PMIC_STATUS_POLARITY:1;
  uint32_t                         u10REGHC_PMIC_STATUS_WAIT:10;
  uint32_t                         u1REGHC_TRANS_USE_OCD:1;
  uint32_t                         u1REGHC_CONFIGURED:1;
} stc_PWR_REGHC_CTL_field_t;

typedef union un_PWR_REGHC_CTL {
  uint32_t                         u32Register;
  stc_PWR_REGHC_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_REGHC_CTL_t;

/**
  * \brief REGHC Status Register (PWR_REGHC_STATUS)
  */
typedef struct stc_PWR_REGHC_STATUS_field {
  uint32_t                         u1REGHC_ENABLED:1;
  uint32_t                         u1REGHC_OCD_OK:1;
  uint32_t                         u1REGHC_CKT_OK:1;
  uint32_t                         :5;
  uint32_t                         u1REGHC_UV_OUT:1;
  uint32_t                         u1REGHC_OV_OUT:1;
  uint32_t                         :2;
  uint32_t                         u1REGHC_PMIC_STATUS_OK:1;
  uint32_t                         :18;
  uint32_t                         u1REGHC_SEQ_BUSY:1;
} stc_PWR_REGHC_STATUS_field_t;

typedef union un_PWR_REGHC_STATUS {
  uint32_t                         u32Register;
  stc_PWR_REGHC_STATUS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_REGHC_STATUS_t;

/**
  * \brief REGHC Control Register 2 (PWR_REGHC_CTL2)
  */
typedef struct stc_PWR_REGHC_CTL2_field {
  uint32_t                         u8REGHC_PMIC_STATUS_TIMEOUT:8;
  uint32_t                         :23;
  uint32_t                         u1REGHC_EN:1;
} stc_PWR_REGHC_CTL2_field_t;

typedef union un_PWR_REGHC_CTL2 {
  uint32_t                         u32Register;
  stc_PWR_REGHC_CTL2_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_REGHC_CTL2_t;

/**
  * \brief REGHC Control Register 4 (PWR_REGHC_CTL4)
  */
typedef struct stc_PWR_REGHC_CTL4_field {
  uint32_t                         :30;
  uint32_t                         u1REGHC_PMIC_VADJ_DIS:1;
  uint32_t                         u1REGHC_PMIC_DPSLP:1;
} stc_PWR_REGHC_CTL4_field_t;

typedef union un_PWR_REGHC_CTL4 {
  uint32_t                         u32Register;
  stc_PWR_REGHC_CTL4_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_REGHC_CTL4_t;

/**
  * \brief HIBERNATE Data Register (PWR_HIB_DATA)
  */
typedef struct stc_PWR_HIB_DATA_field {
  uint32_t                         u32HIB_DATA:32;
} stc_PWR_HIB_DATA_field_t;

typedef union un_PWR_HIB_DATA {
  uint32_t                         u32Register;
  stc_PWR_HIB_DATA_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_HIB_DATA_t;

/**
  * \brief PMIC Control Register (PWR_PMIC_CTL)
  */
typedef struct stc_PWR_PMIC_CTL_field {
  uint32_t                         :2;
  uint32_t                         u2PMIC_VREF:2;
  uint32_t                         u5PMIC_VADJ:5;
  uint32_t                         :1;
  uint32_t                         u1PMIC_USE_LINREG:1;
  uint32_t                         :4;
  uint32_t                         u1PMIC_VADJ_BUF_EN:1;
  uint32_t                         u1PMIC_CTL_OUTEN:1;
  uint32_t                         u1PMIC_CTL_POLARITY:1;
  uint32_t                         u1PMIC_STATUS_INEN:1;
  uint32_t                         u1PMIC_STATUS_POLARITY:1;
  uint32_t                         u10PMIC_STATUS_WAIT:10;
  uint32_t                         :1;
  uint32_t                         u1PMIC_CONFIGURED:1;
} stc_PWR_PMIC_CTL_field_t;

typedef union un_PWR_PMIC_CTL {
  uint32_t                         u32Register;
  stc_PWR_PMIC_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_PMIC_CTL_t;

/**
  * \brief PMIC Status Register (PWR_PMIC_STATUS)
  */
typedef struct stc_PWR_PMIC_STATUS_field {
  uint32_t                         u1PMIC_ENABLED:1;
  uint32_t                         :11;
  uint32_t                         u1PMIC_STATUS_OK:1;
  uint32_t                         :18;
  uint32_t                         u1PMIC_SEQ_BUSY:1;
} stc_PWR_PMIC_STATUS_field_t;

typedef union un_PWR_PMIC_STATUS {
  uint32_t                         u32Register;
  stc_PWR_PMIC_STATUS_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_PMIC_STATUS_t;

/**
  * \brief PMIC Control Register 2 (PWR_PMIC_CTL2)
  */
typedef struct stc_PWR_PMIC_CTL2_field {
  uint32_t                         u8PMIC_STATUS_TIMEOUT:8;
  uint32_t                         :23;
  uint32_t                         u1PMIC_EN:1;
} stc_PWR_PMIC_CTL2_field_t;

typedef union un_PWR_PMIC_CTL2 {
  uint32_t                         u32Register;
  stc_PWR_PMIC_CTL2_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_PMIC_CTL2_t;

/**
  * \brief PMIC Control Register 4 (PWR_PMIC_CTL4)
  */
typedef struct stc_PWR_PMIC_CTL4_field {
  uint32_t                         :30;
  uint32_t                         u1PMIC_VADJ_DIS:1;
  uint32_t                         u1PMIC_DPSLP:1;
} stc_PWR_PMIC_CTL4_field_t;

typedef union un_PWR_PMIC_CTL4 {
  uint32_t                         u32Register;
  stc_PWR_PMIC_CTL4_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_PMIC_CTL4_t;

/**
  * \brief Clock Path Select Register (CLK_PATH_SELECT)
  */
typedef struct stc_CLK_PATH_SELECT_field {
  uint32_t                         u3PATH_MUX:3;
  uint32_t                         :29;
} stc_CLK_PATH_SELECT_field_t;

typedef union un_CLK_PATH_SELECT {
  uint32_t                         u32Register;
  stc_CLK_PATH_SELECT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PATH_SELECT_t;

/**
  * \brief Clock Root Select Register (CLK_ROOT_SELECT)
  */
typedef struct stc_CLK_ROOT_SELECT_field {
  uint32_t                         u4ROOT_MUX:4;
  uint32_t                         u2ROOT_DIV:2;
  uint32_t                         :2;
  uint32_t                         u1DIRECT_MUX:1;
  uint32_t                         :22;
  uint32_t                         u1ENABLE:1;
} stc_CLK_ROOT_SELECT_field_t;

typedef union un_CLK_ROOT_SELECT {
  uint32_t                         u32Register;
  stc_CLK_ROOT_SELECT_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ROOT_SELECT_t;

/**
  * \brief Clock selection register (CLK_SELECT)
  */
typedef struct stc_CLK_SELECT_field {
  uint32_t                         u3LFCLK_SEL:3;
  uint32_t                         :5;
  uint32_t                         u4PUMP_SEL:4;
  uint32_t                         u3PUMP_DIV:3;
  uint32_t                         u1PUMP_ENABLE:1;
  uint32_t                         :16;
} stc_CLK_SELECT_field_t;

typedef union un_CLK_SELECT {
  uint32_t                         u32Register;
  stc_CLK_SELECT_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_SELECT_t;

/**
  * \brief Timer Clock Control Register (CLK_TIMER_CTL)
  */
typedef struct stc_CLK_TIMER_CTL_field {
  uint32_t                         u1TIMER_SEL:1;
  uint32_t                         :7;
  uint32_t                         u2TIMER_HF0_DIV:2;
  uint32_t                         :6;
  uint32_t                         u8TIMER_DIV:8;
  uint32_t                         :7;
  uint32_t                         u1ENABLE:1;
} stc_CLK_TIMER_CTL_field_t;

typedef union un_CLK_TIMER_CTL {
  uint32_t                         u32Register;
  stc_CLK_TIMER_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TIMER_CTL_t;

/**
  * \brief ILO0 Configuration (CLK_ILO0_CONFIG)
  */
typedef struct stc_CLK_ILO0_CONFIG_field {
  uint32_t                         u1ILO0_BACKUP:1;
  uint32_t                         :29;
  uint32_t                         u1ILO0_MON_ENABLE:1;
  uint32_t                         u1ENABLE:1;
} stc_CLK_ILO0_CONFIG_field_t;

typedef union un_CLK_ILO0_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_ILO0_CONFIG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ILO0_CONFIG_t;

/**
  * \brief ILO1 Configuration (CLK_ILO1_CONFIG)
  */
typedef struct stc_CLK_ILO1_CONFIG_field {
  uint32_t                         :30;
  uint32_t                         u1ILO1_MON_ENABLE:1;
  uint32_t                         u1ENABLE:1;
} stc_CLK_ILO1_CONFIG_field_t;

typedef union un_CLK_ILO1_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_ILO1_CONFIG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ILO1_CONFIG_t;

/**
  * \brief IMO Configuration (CLK_IMO_CONFIG)
  */
typedef struct stc_CLK_IMO_CONFIG_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLE:1;
} stc_CLK_IMO_CONFIG_field_t;

typedef union un_CLK_IMO_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_IMO_CONFIG_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_IMO_CONFIG_t;

/**
  * \brief ECO Configuration Register (CLK_ECO_CONFIG)
  */
typedef struct stc_CLK_ECO_CONFIG_field {
  uint32_t                         :1;
  uint32_t                         u1AGC_EN:1;
  uint32_t                         :25;
  uint32_t                         u1ECO_DIV_DISABLE:1;
  uint32_t                         u1ECO_DIV_ENABLE:1;
  uint32_t                         :2;
  uint32_t                         u1ECO_EN:1;
} stc_CLK_ECO_CONFIG_field_t;

typedef union un_CLK_ECO_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_ECO_CONFIG_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ECO_CONFIG_t;

/**
  * \brief ECO Prescaler Configuration Register (CLK_ECO_PRESCALE)
  */
typedef struct stc_CLK_ECO_PRESCALE_field {
  uint32_t                         u1ECO_DIV_ENABLED:1;
  uint32_t                         :7;
  uint32_t                         u8ECO_FRAC_DIV:8;
  uint32_t                         u10ECO_INT_DIV:10;
  uint32_t                         :6;
} stc_CLK_ECO_PRESCALE_field_t;

typedef union un_CLK_ECO_PRESCALE {
  uint32_t                         u32Register;
  stc_CLK_ECO_PRESCALE_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ECO_PRESCALE_t;

/**
  * \brief ECO Status Register (CLK_ECO_STATUS)
  */
typedef struct stc_CLK_ECO_STATUS_field {
  uint32_t                         u1ECO_OK:1;
  uint32_t                         u1ECO_READY:1;
  uint32_t                         :30;
} stc_CLK_ECO_STATUS_field_t;

typedef union un_CLK_ECO_STATUS {
  uint32_t                         u32Register;
  stc_CLK_ECO_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ECO_STATUS_t;

/**
  * \brief Precision ILO Configuration Register (CLK_PILO_CONFIG)
  */
typedef struct stc_CLK_PILO_CONFIG_field {
  uint32_t                         u10PILO_FFREQ:10;
  uint32_t                         :19;
  uint32_t                         u1PILO_CLK_EN:1;
  uint32_t                         u1PILO_RESET_N:1;
  uint32_t                         u1PILO_EN:1;
} stc_CLK_PILO_CONFIG_field_t;

typedef union un_CLK_PILO_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_PILO_CONFIG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PILO_CONFIG_t;

/**
  * \brief FLL Configuration Register (CLK_FLL_CONFIG)
  */
typedef struct stc_CLK_FLL_CONFIG_field {
  uint32_t                         u18FLL_MULT:18;
  uint32_t                         :6;
  uint32_t                         u1FLL_OUTPUT_DIV:1;
  uint32_t                         :6;
  uint32_t                         u1FLL_ENABLE:1;
} stc_CLK_FLL_CONFIG_field_t;

typedef union un_CLK_FLL_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_FLL_CONFIG_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_FLL_CONFIG_t;

/**
  * \brief FLL Configuration Register 2 (CLK_FLL_CONFIG2)
  */
typedef struct stc_CLK_FLL_CONFIG2_field {
  uint32_t                         u13FLL_REF_DIV:13;
  uint32_t                         :3;
  uint32_t                         u8LOCK_TOL:8;
  uint32_t                         u8UPDATE_TOL:8;
} stc_CLK_FLL_CONFIG2_field_t;

typedef union un_CLK_FLL_CONFIG2 {
  uint32_t                         u32Register;
  stc_CLK_FLL_CONFIG2_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_FLL_CONFIG2_t;

/**
  * \brief FLL Configuration Register 3 (CLK_FLL_CONFIG3)
  */
typedef struct stc_CLK_FLL_CONFIG3_field {
  uint32_t                         u4FLL_LF_IGAIN:4;
  uint32_t                         u4FLL_LF_PGAIN:4;
  uint32_t                         u13SETTLING_COUNT:13;
  uint32_t                         :7;
  uint32_t                         u2BYPASS_SEL:2;
  uint32_t                         :2;
} stc_CLK_FLL_CONFIG3_field_t;

typedef union un_CLK_FLL_CONFIG3 {
  uint32_t                         u32Register;
  stc_CLK_FLL_CONFIG3_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_FLL_CONFIG3_t;

/**
  * \brief FLL Configuration Register 4 (CLK_FLL_CONFIG4)
  */
typedef struct stc_CLK_FLL_CONFIG4_field {
  uint32_t                         u8CCO_LIMIT:8;
  uint32_t                         u3CCO_RANGE:3;
  uint32_t                         :5;
  uint32_t                         u9CCO_FREQ:9;
  uint32_t                         :5;
  uint32_t                         u1CCO_HW_UPDATE_DIS:1;
  uint32_t                         u1CCO_ENABLE:1;
} stc_CLK_FLL_CONFIG4_field_t;

typedef union un_CLK_FLL_CONFIG4 {
  uint32_t                         u32Register;
  stc_CLK_FLL_CONFIG4_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_FLL_CONFIG4_t;

/**
  * \brief FLL Status Register (CLK_FLL_STATUS)
  */
typedef struct stc_CLK_FLL_STATUS_field {
  uint32_t                         u1LOCKED:1;
  uint32_t                         u1UNLOCK_OCCURRED:1;
  uint32_t                         u1CCO_READY:1;
  uint32_t                         :29;
} stc_CLK_FLL_STATUS_field_t;

typedef union un_CLK_FLL_STATUS {
  uint32_t                         u32Register;
  stc_CLK_FLL_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_FLL_STATUS_t;

/**
  * \brief ECO Configuration Register 2 (CLK_ECO_CONFIG2)
  */
typedef struct stc_CLK_ECO_CONFIG2_field {
  uint32_t                         u3WDTRIM:3;
  uint32_t                         :1;
  uint32_t                         u4ATRIM:4;
  uint32_t                         u2FTRIM:2;
  uint32_t                         u2RTRIM:2;
  uint32_t                         u3GTRIM:3;
  uint32_t                         :17;
} stc_CLK_ECO_CONFIG2_field_t;

typedef union un_CLK_ECO_CONFIG2 {
  uint32_t                         u32Register;
  stc_CLK_ECO_CONFIG2_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_ECO_CONFIG2_t;

/**
  * \brief PLL Configuration Register (CLK_PLL_CONFIG)
  */
typedef struct stc_CLK_PLL_CONFIG_field {
  uint32_t                         u7FEEDBACK_DIV:7;
  uint32_t                         :1;
  uint32_t                         u5REFERENCE_DIV:5;
  uint32_t                         :3;
  uint32_t                         u5OUTPUT_DIV:5;
  uint32_t                         :4;
  uint32_t                         u2LOCK_DELAY:2;
  uint32_t                         u1PLL_LF_MODE:1;
  uint32_t                         u2BYPASS_SEL:2;
  uint32_t                         :1;
  uint32_t                         u1ENABLE:1;
} stc_CLK_PLL_CONFIG_field_t;

typedef union un_CLK_PLL_CONFIG {
  uint32_t                         u32Register;
  stc_CLK_PLL_CONFIG_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL_CONFIG_t;

/**
  * \brief PLL Status Register (CLK_PLL_STATUS)
  */
typedef struct stc_CLK_PLL_STATUS_field {
  uint32_t                         u1LOCKED:1;
  uint32_t                         u1UNLOCK_OCCURRED:1;
  uint32_t                         :30;
} stc_CLK_PLL_STATUS_field_t;

typedef union un_CLK_PLL_STATUS {
  uint32_t                         u32Register;
  stc_CLK_PLL_STATUS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_PLL_STATUS_t;

/**
  * \brief Select CSV Reference clock for Active domain (CSV_REF_SEL)
  */
typedef struct stc_CSV_REF_SEL_field {
  uint32_t                         u3REF_MUX:3;
  uint32_t                         :29;
} stc_CSV_REF_SEL_field_t;

typedef union un_CSV_REF_SEL {
  uint32_t                         u32Register;
  stc_CSV_REF_SEL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CSV_REF_SEL_t;

/**
  * \brief Reset Cause Observation Register (RES_CAUSE)
  */
typedef struct stc_RES_CAUSE_field {
  uint32_t                         u1RESET_WDT:1;
  uint32_t                         u1RESET_ACT_FAULT:1;
  uint32_t                         u1RESET_DPSLP_FAULT:1;
  uint32_t                         u1RESET_TC_DBGRESET:1;
  uint32_t                         u1RESET_SOFT:1;
  uint32_t                         u1RESET_MCWDT0:1;
  uint32_t                         u1RESET_MCWDT1:1;
  uint32_t                         u1RESET_MCWDT2:1;
  uint32_t                         u1RESET_MCWDT3:1;
  uint32_t                         :7;
  uint32_t                         u1RESET_XRES:1;
  uint32_t                         u1RESET_BODVDDD:1;
  uint32_t                         u1RESET_BODVDDA:1;
  uint32_t                         u1RESET_BODVCCD:1;
  uint32_t                         u1RESET_OVDVDDD:1;
  uint32_t                         u1RESET_OVDVDDA:1;
  uint32_t                         u1RESET_OVDVCCD:1;
  uint32_t                         u1RESET_OCD_ACT_LINREG:1;
  uint32_t                         u1RESET_OCD_DPSLP_LINREG:1;
  uint32_t                         u1RESET_OCD_REGHC:1;
  uint32_t                         u1RESET_PMIC:1;
  uint32_t                         :1;
  uint32_t                         u1RESET_PXRES:1;
  uint32_t                         u1RESET_STRUCT_XRES:1;
  uint32_t                         u1RESET_PORVDDD:1;
} stc_RES_CAUSE_field_t;

typedef union un_RES_CAUSE {
  uint32_t                         u32Register;
  stc_RES_CAUSE_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_RES_CAUSE_t;

/**
  * \brief Reset Cause Observation Register 2 (RES_CAUSE2)
  */
typedef struct stc_RES_CAUSE2_field {
  uint32_t                         u16RESET_CSV_HF:16;
  uint32_t                         u1RESET_CSV_REF:1;
  uint32_t                         :15;
} stc_RES_CAUSE2_field_t;

typedef union un_RES_CAUSE2 {
  uint32_t                         u32Register;
  stc_RES_CAUSE2_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_RES_CAUSE2_t;

/**
  * \brief ILO0 Trim Register (CLK_TRIM_ILO0_CTL)
  */
typedef struct stc_CLK_TRIM_ILO0_CTL_field {
  uint32_t                         u6ILO0_FTRIM:6;
  uint32_t                         :2;
  uint32_t                         u4ILO0_MONTRIM:4;
  uint32_t                         :20;
} stc_CLK_TRIM_ILO0_CTL_field_t;

typedef union un_CLK_TRIM_ILO0_CTL {
  uint32_t                         u32Register;
  stc_CLK_TRIM_ILO0_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TRIM_ILO0_CTL_t;

/**
  * \brief Power System Trim Register (PWR_TRIM_PWRSYS_CTL)
  */
typedef struct stc_PWR_TRIM_PWRSYS_CTL_field {
  uint32_t                         u5ACT_REG_TRIM:5;
  uint32_t                         :25;
  uint32_t                         u2ACT_REG_BOOST:2;
} stc_PWR_TRIM_PWRSYS_CTL_field_t;

typedef union un_PWR_TRIM_PWRSYS_CTL {
  uint32_t                         u32Register;
  stc_PWR_TRIM_PWRSYS_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PWR_TRIM_PWRSYS_CTL_t;

/**
  * \brief PILO Trim Register (CLK_TRIM_PILO_CTL)
  */
typedef struct stc_CLK_TRIM_PILO_CTL_field {
  uint32_t                         u6PILO_CFREQ:6;
  uint32_t                         :6;
  uint32_t                         u3PILO_OSC_TRIM:3;
  uint32_t                         :1;
  uint32_t                         u2PILO_COMP_TRIM:2;
  uint32_t                         u2PILO_NBIAS_TRIM:2;
  uint32_t                         u5PILO_RES_TRIM:5;
  uint32_t                         :1;
  uint32_t                         u2PILO_ISLOPE_TRIM:2;
  uint32_t                         u3PILO_VTDIFF_TRIM:3;
} stc_CLK_TRIM_PILO_CTL_field_t;

typedef union un_CLK_TRIM_PILO_CTL {
  uint32_t                         u32Register;
  stc_CLK_TRIM_PILO_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TRIM_PILO_CTL_t;

/**
  * \brief PILO Trim Register 2 (CLK_TRIM_PILO_CTL2)
  */
typedef struct stc_CLK_TRIM_PILO_CTL2_field {
  uint32_t                         u8PILO_VREF_TRIM:8;
  uint32_t                         u5PILO_IREFBM_TRIM:5;
  uint32_t                         :3;
  uint32_t                         u8PILO_IREF_TRIM:8;
  uint32_t                         :8;
} stc_CLK_TRIM_PILO_CTL2_field_t;

typedef union un_CLK_TRIM_PILO_CTL2 {
  uint32_t                         u32Register;
  stc_CLK_TRIM_PILO_CTL2_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TRIM_PILO_CTL2_t;

/**
  * \brief PILO Trim Register 3 (CLK_TRIM_PILO_CTL3)
  */
typedef struct stc_CLK_TRIM_PILO_CTL3_field {
  uint32_t                         u16PILO_ENGOPT:16;
  uint32_t                         :16;
} stc_CLK_TRIM_PILO_CTL3_field_t;

typedef union un_CLK_TRIM_PILO_CTL3 {
  uint32_t                         u32Register;
  stc_CLK_TRIM_PILO_CTL3_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TRIM_PILO_CTL3_t;

/**
  * \brief ILO1 Trim Register (CLK_TRIM_ILO1_CTL)
  */
typedef struct stc_CLK_TRIM_ILO1_CTL_field {
  uint32_t                         u6ILO1_FTRIM:6;
  uint32_t                         :2;
  uint32_t                         u4ILO1_MONTRIM:4;
  uint32_t                         :20;
} stc_CLK_TRIM_ILO1_CTL_field_t;

typedef union un_CLK_TRIM_ILO1_CTL {
  uint32_t                         u32Register;
  stc_CLK_TRIM_ILO1_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CLK_TRIM_ILO1_CTL_t;



/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV)
  */
typedef struct stc_CSV_HF_CSV {
  un_CSV_HF_CSV_REF_CTL_t          unREF_CTL;          /*!< 0x00000000 Clock Supervision Reference Control */
  un_CSV_HF_CSV_REF_LIMIT_t        unREF_LIMIT;        /*!< 0x00000004 Clock Supervision Reference Limits */
  un_CSV_HF_CSV_MON_CTL_t          unMON_CTL;          /*!< 0x00000008 Clock Supervision Monitor Control */
  uint32_t                         u32Reserved;
} stc_CSV_HF_CSV_t;                                    /*!< Size = 16 (0x10) */

/**
  * \brief Clock Supervisor (CSV) registers for Root clocks (CSV_HF)
  */
typedef struct stc_CSV_HF {
  stc_CSV_HF_CSV_t                 CSV[16];            /*!< 0x00000000 Active domain Clock Supervisor (CSV) registers */
} stc_CSV_HF_t;                                        /*!< Size = 256 (0x100) */

/**
  * \brief Active domain Clock Supervisor (CSV) registers for CSV Reference clock (CSV_REF_CSV)
  */
typedef struct stc_CSV_REF_CSV {
  un_CSV_REF_CSV_REF_CTL_t         unREF_CTL;          /*!< 0x00000000 Clock Supervision Reference Control */
  un_CSV_REF_CSV_REF_LIMIT_t       unREF_LIMIT;        /*!< 0x00000004 Clock Supervision Reference Limits */
  un_CSV_REF_CSV_MON_CTL_t         unMON_CTL;          /*!< 0x00000008 Clock Supervision Monitor Control */
  uint32_t                         u32Reserved;
} stc_CSV_REF_CSV_t;                                   /*!< Size = 16 (0x10) */

/**
  * \brief CSV registers for the CSV Reference clock (CSV_REF)
  */
typedef struct stc_CSV_REF {
  stc_CSV_REF_CSV_t                CSV;                /*!< 0x00000000 Active domain Clock Supervisor (CSV) registers for CSV
                                                                                Reference clock */
} stc_CSV_REF_t;                                       /*!< Size = 16 (0x10) */

/**
  * \brief LF clock Clock Supervisor registers (CSV_LF_CSV)
  */
typedef struct stc_CSV_LF_CSV {
  un_CSV_LF_CSV_REF_CTL_t          unREF_CTL;          /*!< 0x00000000 Clock Supervision Reference Control */
  un_CSV_LF_CSV_REF_LIMIT_t        unREF_LIMIT;        /*!< 0x00000004 Clock Supervision Reference Limits */
  un_CSV_LF_CSV_MON_CTL_t          unMON_CTL;          /*!< 0x00000008 Clock Supervision Monitor Control */
  uint32_t                         u32Reserved;
} stc_CSV_LF_CSV_t;                                    /*!< Size = 16 (0x10) */

/**
  * \brief CSV registers for LF clock (CSV_LF)
  */
typedef struct stc_CSV_LF {
  stc_CSV_LF_CSV_t                 CSV;                /*!< 0x00000000 LF clock Clock Supervisor registers */
} stc_CSV_LF_t;                                        /*!< Size = 16 (0x10) */

/**
  * \brief ILO0 clock DeepSleep domain Clock Supervisor registers (CSV_ILO_CSV)
  */
typedef struct stc_CSV_ILO_CSV {
  un_CSV_ILO_CSV_REF_CTL_t         unREF_CTL;          /*!< 0x00000000 Clock Supervision Reference Control */
  un_CSV_ILO_CSV_REF_LIMIT_t       unREF_LIMIT;        /*!< 0x00000004 Clock Supervision Reference Limits */
  un_CSV_ILO_CSV_MON_CTL_t         unMON_CTL;          /*!< 0x00000008 Clock Supervision Monitor Control */
  uint32_t                         u32Reserved;
} stc_CSV_ILO_CSV_t;                                   /*!< Size = 16 (0x10) */

/**
  * \brief CSV registers for HVILO clock (CSV_ILO)
  */
typedef struct stc_CSV_ILO {
  stc_CSV_ILO_CSV_t                CSV;                /*!< 0x00000000 ILO0 clock DeepSleep domain Clock Supervisor registers */
} stc_CSV_ILO_t;                                       /*!< Size = 16 (0x10) */

/**
  * \brief 400MHz PLL Configuration Register (CLK_PLL400M)
  */
typedef struct stc_CLK_PLL400M {
  un_CLK_PLL400M_CONFIG_t          unCONFIG;           /*!< 0x00000000 400MHz PLL Configuration Register */
  un_CLK_PLL400M_CONFIG2_t         unCONFIG2;          /*!< 0x00000004 400MHz PLL Configuration Register 2 */
  un_CLK_PLL400M_CONFIG3_t         unCONFIG3;          /*!< 0x00000008 400MHz PLL Configuration Register 3 */
  un_CLK_PLL400M_STATUS_t          unSTATUS;           /*!< 0x0000000C 400MHz PLL Status Register */
} stc_CLK_PLL400M_t;                                   /*!< Size = 16 (0x10) */

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR)
  */
typedef struct stc_MCWDT_CTR {
  un_MCWDT_CTR_CTL_t               unCTL;              /*!< 0x00000000 MCWDT Subcounter Control Register */
  un_MCWDT_CTR_LOWER_LIMIT_t       unLOWER_LIMIT;      /*!< 0x00000004 MCWDT Subcounter Lower Limit Register */
  un_MCWDT_CTR_UPPER_LIMIT_t       unUPPER_LIMIT;      /*!< 0x00000008 MCWDT Subcounter Upper Limit Register */
  un_MCWDT_CTR_WARN_LIMIT_t        unWARN_LIMIT;       /*!< 0x0000000C MCWDT Subcounter Warn Limit Register */
  un_MCWDT_CTR_CONFIG_t            unCONFIG;           /*!< 0x00000010 MCWDT Subcounter Configuration Register */
  un_MCWDT_CTR_CNT_t               unCNT;              /*!< 0x00000014 MCWDT Subcounter Count Register */
  uint32_t                         au32Reserved[2];
} stc_MCWDT_CTR_t;                                     /*!< Size = 32 (0x20) */

/**
  * \brief Multi-Counter Watchdog Timer (MCWDT)
  */
typedef struct stc_MCWDT {
  stc_MCWDT_CTR_t                  CTR[2];             /*!< 0x00000000 MCWDT Configuration for Subcounter 0 and 1 */
  un_MCWDT_CPU_SELECT_t            unCPU_SELECT;       /*!< 0x00000040 MCWDT CPU selection register */
  uint32_t                         au32Reserved[15];
  un_MCWDT_CTR2_CTL_t              unCTR2_CTL;         /*!< 0x00000080 MCWDT Subcounter 2 Control register */
  un_MCWDT_CTR2_CONFIG_t           unCTR2_CONFIG;      /*!< 0x00000084 MCWDT Subcounter 2 Configuration register */
  un_MCWDT_CTR2_CNT_t              unCTR2_CNT;         /*!< 0x00000088 MCWDT Subcounter 2 Count Register */
  uint32_t                         u32Reserved1;
  un_MCWDT_LOCK_t                  unLOCK;             /*!< 0x00000090 MCWDT Lock Register */
  un_MCWDT_SERVICE_t               unSERVICE;          /*!< 0x00000094 MCWDT Service Register */
  uint32_t                         au32Reserved2[2];
  un_MCWDT_INTR_t                  unINTR;             /*!< 0x000000A0 MCWDT Interrupt Register */
  un_MCWDT_INTR_SET_t              unINTR_SET;         /*!< 0x000000A4 MCWDT Interrupt Set Register */
  un_MCWDT_INTR_MASK_t             unINTR_MASK;        /*!< 0x000000A8 MCWDT Interrupt Mask Register */
  un_MCWDT_INTR_MASKED_t           unINTR_MASKED;      /*!< 0x000000AC MCWDT Interrupt Masked Register */
  uint32_t                         au32Reserved3[20];
} stc_MCWDT_t;                                         /*!< Size = 256 (0x100) */

/**
  * \brief Watchdog Timer (WDT)
  */
typedef struct stc_WDT {
  un_WDT_CTL_t                     unCTL;              /*!< 0x00000000 WDT Control Register */
  un_WDT_LOWER_LIMIT_t             unLOWER_LIMIT;      /*!< 0x00000004 WDT Lower Limit Register */
  un_WDT_UPPER_LIMIT_t             unUPPER_LIMIT;      /*!< 0x00000008 WDT Upper Limit Register */
  un_WDT_WARN_LIMIT_t              unWARN_LIMIT;       /*!< 0x0000000C WDT Warn Limit Register */
  un_WDT_CONFIG_t                  unCONFIG;           /*!< 0x00000010 WDT Configuration Register */
  un_WDT_CNT_t                     unCNT;              /*!< 0x00000014 WDT Count Register */
  uint32_t                         au32Reserved[10];
  un_WDT_LOCK_t                    unLOCK;             /*!< 0x00000040 WDT Lock register */
  un_WDT_SERVICE_t                 unSERVICE;          /*!< 0x00000044 WDT Service register */
  uint32_t                         au32Reserved1[2];
  un_WDT_INTR_t                    unINTR;             /*!< 0x00000050 WDT Interrupt Register */
  un_WDT_INTR_SET_t                unINTR_SET;         /*!< 0x00000054 WDT Interrupt Set Register */
  un_WDT_INTR_MASK_t               unINTR_MASK;        /*!< 0x00000058 WDT Interrupt Mask Register */
  un_WDT_INTR_MASKED_t             unINTR_MASKED;      /*!< 0x0000005C WDT Interrupt Masked Register */
  uint32_t                         au32Reserved2[8];
} stc_WDT_t;                                           /*!< Size = 128 (0x80) */

/**
  * \brief SRSS Core Registers (ver3) (SRSS)
  */
typedef struct stc_SRSS {
  uint32_t                         au32Reserved[16];
  un_PWR_LVD_STATUS_t              unPWR_LVD_STATUS;   /*!< 0x00000040 High Voltage / Low Voltage Detector (HVLVD) Status
                                                                                Register */
  un_PWR_LVD_STATUS2_t             unPWR_LVD_STATUS2;  /*!< 0x00000044 High Voltage / Low Voltage Detector (HVLVD) Status
                                                                                Register #2 */
  uint32_t                         au32Reserved1[46];
  un_CLK_DSI_SELECT_t              unCLK_DSI_SELECT[16]; /*!< 0x00000100 Clock DSI Select Register */
  un_CLK_OUTPUT_FAST_t             unCLK_OUTPUT_FAST;  /*!< 0x00000140 Fast Clock Output Select Register */
  un_CLK_OUTPUT_SLOW_t             unCLK_OUTPUT_SLOW;  /*!< 0x00000144 Slow Clock Output Select Register */
  un_CLK_CAL_CNT1_t                unCLK_CAL_CNT1;     /*!< 0x00000148 Clock Calibration Counter 1 */
  un_CLK_CAL_CNT2_t                unCLK_CAL_CNT2;     /*!< 0x0000014C Clock Calibration Counter 2 */
  uint32_t                         au32Reserved2[44];
  un_SRSS_INTR_t                   unSRSS_INTR;        /*!< 0x00000200 SRSS Interrupt Register */
  un_SRSS_INTR_SET_t               unSRSS_INTR_SET;    /*!< 0x00000204 SRSS Interrupt Set Register */
  un_SRSS_INTR_MASK_t              unSRSS_INTR_MASK;   /*!< 0x00000208 SRSS Interrupt Mask Register */
  un_SRSS_INTR_MASKED_t            unSRSS_INTR_MASKED; /*!< 0x0000020C SRSS Interrupt Masked Register */
  uint32_t                         au32Reserved3[892];
  un_PWR_CTL_t                     unPWR_CTL;          /*!< 0x00001000 Power Mode Control */
  un_PWR_CTL2_t                    unPWR_CTL2;         /*!< 0x00001004 Power Mode Control 2 */
  un_PWR_HIBERNATE_t               unPWR_HIBERNATE;    /*!< 0x00001008 HIBERNATE Mode Register */
  uint32_t                         u32Reserved4;
  un_PWR_BUCK_CTL_t                unPWR_BUCK_CTL;     /*!< 0x00001010 Buck Control Register */
  un_PWR_BUCK_CTL2_t               unPWR_BUCK_CTL2;    /*!< 0x00001014 Buck Control Register 2 */
  un_PWR_SSV_CTL_t                 unPWR_SSV_CTL;      /*!< 0x00001018 Supply Supervision Control Register */
  un_PWR_SSV_STATUS_t              unPWR_SSV_STATUS;   /*!< 0x0000101C Supply Supervision Status Register */
  un_PWR_LVD_CTL_t                 unPWR_LVD_CTL;      /*!< 0x00001020 High Voltage / Low Voltage Detector (HVLVD)
                                                                                Configuration Register */
  un_PWR_LVD_CTL2_t                unPWR_LVD_CTL2;     /*!< 0x00001024 High Voltage / Low Voltage Detector (HVLVD)
                                                                                Configuration Register #2 */
  un_PWR_REGHC_CTL_t               unPWR_REGHC_CTL;    /*!< 0x00001028 REGHC Control Register */
  un_PWR_REGHC_STATUS_t            unPWR_REGHC_STATUS; /*!< 0x0000102C REGHC Status Register */
  un_PWR_REGHC_CTL2_t              unPWR_REGHC_CTL2;   /*!< 0x00001030 REGHC Control Register 2 */
  uint32_t                         u32Reserved5;
  un_PWR_REGHC_CTL4_t              unPWR_REGHC_CTL4;   /*!< 0x00001038 REGHC Control Register 4 */
  uint32_t                         u32Reserved6;
  un_PWR_HIB_DATA_t                unPWR_HIB_DATA[16]; /*!< 0x00001040 HIBERNATE Data Register */
  uint32_t                         au32Reserved7[16];
  un_PWR_PMIC_CTL_t                unPWR_PMIC_CTL;     /*!< 0x000010C0 PMIC Control Register */
  un_PWR_PMIC_STATUS_t             unPWR_PMIC_STATUS;  /*!< 0x000010C4 PMIC Status Register */
  un_PWR_PMIC_CTL2_t               unPWR_PMIC_CTL2;    /*!< 0x000010C8 PMIC Control Register 2 */
  uint32_t                         u32Reserved8;
  un_PWR_PMIC_CTL4_t               unPWR_PMIC_CTL4;    /*!< 0x000010D0 PMIC Control Register 4 */
  uint32_t                         au32Reserved9[75];
  un_CLK_PATH_SELECT_t             unCLK_PATH_SELECT[16]; /*!< 0x00001200 Clock Path Select Register */
  un_CLK_ROOT_SELECT_t             unCLK_ROOT_SELECT[16]; /*!< 0x00001240 Clock Root Select Register */
  uint32_t                         au32Reserved10[96];
  stc_CSV_HF_t                     CSV_HF;             /*!< 0x00001400 Clock Supervisor (CSV) registers for Root clocks */
  un_CLK_SELECT_t                  unCLK_SELECT;       /*!< 0x00001500 Clock selection register */
  un_CLK_TIMER_CTL_t               unCLK_TIMER_CTL;    /*!< 0x00001504 Timer Clock Control Register */
  un_CLK_ILO0_CONFIG_t             unCLK_ILO0_CONFIG;  /*!< 0x00001508 ILO0 Configuration */
  un_CLK_ILO1_CONFIG_t             unCLK_ILO1_CONFIG;  /*!< 0x0000150C ILO1 Configuration */
  uint32_t                         au32Reserved11[2];
  un_CLK_IMO_CONFIG_t              unCLK_IMO_CONFIG;   /*!< 0x00001518 IMO Configuration */
  un_CLK_ECO_CONFIG_t              unCLK_ECO_CONFIG;   /*!< 0x0000151C ECO Configuration Register */
  un_CLK_ECO_PRESCALE_t            unCLK_ECO_PRESCALE; /*!< 0x00001520 ECO Prescaler Configuration Register */
  un_CLK_ECO_STATUS_t              unCLK_ECO_STATUS;   /*!< 0x00001524 ECO Status Register */
  un_CLK_PILO_CONFIG_t             unCLK_PILO_CONFIG;  /*!< 0x00001528 Precision ILO Configuration Register */
  uint32_t                         u32Reserved12;
  un_CLK_FLL_CONFIG_t              unCLK_FLL_CONFIG;   /*!< 0x00001530 FLL Configuration Register */
  un_CLK_FLL_CONFIG2_t             unCLK_FLL_CONFIG2;  /*!< 0x00001534 FLL Configuration Register 2 */
  un_CLK_FLL_CONFIG3_t             unCLK_FLL_CONFIG3;  /*!< 0x00001538 FLL Configuration Register 3 */
  un_CLK_FLL_CONFIG4_t             unCLK_FLL_CONFIG4;  /*!< 0x0000153C FLL Configuration Register 4 */
  un_CLK_FLL_STATUS_t              unCLK_FLL_STATUS;   /*!< 0x00001540 FLL Status Register */
  un_CLK_ECO_CONFIG2_t             unCLK_ECO_CONFIG2;  /*!< 0x00001544 ECO Configuration Register 2 */
  uint32_t                         au32Reserved13[46];
  un_CLK_PLL_CONFIG_t              unCLK_PLL_CONFIG[15]; /*!< 0x00001600 PLL Configuration Register */
  uint32_t                         u32Reserved14;
  un_CLK_PLL_STATUS_t              unCLK_PLL_STATUS[15]; /*!< 0x00001640 PLL Status Register */
  uint32_t                         au32Reserved15[33];
  un_CSV_REF_SEL_t                 unCSV_REF_SEL;      /*!< 0x00001700 Select CSV Reference clock for Active domain */
  uint32_t                         au32Reserved16[3];
  stc_CSV_REF_t                    CSV_REF;            /*!< 0x00001710 CSV registers for the CSV Reference clock */
  stc_CSV_LF_t                     CSV_LF;             /*!< 0x00001720 CSV registers for LF clock */
  stc_CSV_ILO_t                    CSV_ILO;            /*!< 0x00001730 CSV registers for HVILO clock */
  uint32_t                         au32Reserved17[48];
  un_RES_CAUSE_t                   unRES_CAUSE;        /*!< 0x00001800 Reset Cause Observation Register */
  un_RES_CAUSE2_t                  unRES_CAUSE2;       /*!< 0x00001804 Reset Cause Observation Register 2 */
  uint32_t                         au32Reserved18[62];
  stc_CLK_PLL400M_t                CLK_PLL400M[15];    /*!< 0x00001900 400MHz PLL Configuration Register */
  uint32_t                         au32Reserved19[1417];
  un_CLK_TRIM_ILO0_CTL_t           unCLK_TRIM_ILO0_CTL; /*!< 0x00003014 ILO0 Trim Register */
  uint32_t                         au32Reserved20[60];
  un_PWR_TRIM_PWRSYS_CTL_t         unPWR_TRIM_PWRSYS_CTL; /*!< 0x00003108 Power System Trim Register */
  uint32_t                         au32Reserved21[2];
  un_CLK_TRIM_PILO_CTL_t           unCLK_TRIM_PILO_CTL; /*!< 0x00003114 PILO Trim Register */
  un_CLK_TRIM_PILO_CTL2_t          unCLK_TRIM_PILO_CTL2; /*!< 0x00003118 PILO Trim Register 2 */
  un_CLK_TRIM_PILO_CTL3_t          unCLK_TRIM_PILO_CTL3; /*!< 0x0000311C PILO Trim Register 3 */
  uint32_t                         au32Reserved22[64];
  un_CLK_TRIM_ILO1_CTL_t           unCLK_TRIM_ILO1_CTL; /*!< 0x00003220 ILO1 Trim Register */
  uint32_t                         au32Reserved23[4983];
  stc_MCWDT_t                      MCWDT[4];           /*!< 0x00008000 Multi-Counter Watchdog Timer */
  uint32_t                         au32Reserved24[3840];
  stc_WDT_t                        WDT;                /*!< 0x0000C000 Watchdog Timer */
} stc_SRSS_t;                                          /*!< Size = 49280 (0xC080) */


/* CSV_HF_CSV.REF_CTL */
#define CSV_HF_CSV_REF_CTL_STARTUP_Pos          0UL
#define CSV_HF_CSV_REF_CTL_STARTUP_Msk          0xFFFFUL
#define CSV_HF_CSV_REF_CTL_CSV_ACTION_Pos       30UL
#define CSV_HF_CSV_REF_CTL_CSV_ACTION_Msk       0x40000000UL
#define CSV_HF_CSV_REF_CTL_CSV_EN_Pos           31UL
#define CSV_HF_CSV_REF_CTL_CSV_EN_Msk           0x80000000UL
/* CSV_HF_CSV.REF_LIMIT */
#define CSV_HF_CSV_REF_LIMIT_LOWER_Pos          0UL
#define CSV_HF_CSV_REF_LIMIT_LOWER_Msk          0xFFFFUL
#define CSV_HF_CSV_REF_LIMIT_UPPER_Pos          16UL
#define CSV_HF_CSV_REF_LIMIT_UPPER_Msk          0xFFFF0000UL
/* CSV_HF_CSV.MON_CTL */
#define CSV_HF_CSV_MON_CTL_PERIOD_Pos           0UL
#define CSV_HF_CSV_MON_CTL_PERIOD_Msk           0xFFFFUL


/* CSV_REF_CSV.REF_CTL */
#define CSV_REF_CSV_REF_CTL_STARTUP_Pos         0UL
#define CSV_REF_CSV_REF_CTL_STARTUP_Msk         0xFFFFUL
#define CSV_REF_CSV_REF_CTL_CSV_ACTION_Pos      30UL
#define CSV_REF_CSV_REF_CTL_CSV_ACTION_Msk      0x40000000UL
#define CSV_REF_CSV_REF_CTL_CSV_EN_Pos          31UL
#define CSV_REF_CSV_REF_CTL_CSV_EN_Msk          0x80000000UL
/* CSV_REF_CSV.REF_LIMIT */
#define CSV_REF_CSV_REF_LIMIT_LOWER_Pos         0UL
#define CSV_REF_CSV_REF_LIMIT_LOWER_Msk         0xFFFFUL
#define CSV_REF_CSV_REF_LIMIT_UPPER_Pos         16UL
#define CSV_REF_CSV_REF_LIMIT_UPPER_Msk         0xFFFF0000UL
/* CSV_REF_CSV.MON_CTL */
#define CSV_REF_CSV_MON_CTL_PERIOD_Pos          0UL
#define CSV_REF_CSV_MON_CTL_PERIOD_Msk          0xFFFFUL


/* CSV_LF_CSV.REF_CTL */
#define CSV_LF_CSV_REF_CTL_STARTUP_Pos          0UL
#define CSV_LF_CSV_REF_CTL_STARTUP_Msk          0xFFUL
#define CSV_LF_CSV_REF_CTL_CSV_EN_Pos           31UL
#define CSV_LF_CSV_REF_CTL_CSV_EN_Msk           0x80000000UL
/* CSV_LF_CSV.REF_LIMIT */
#define CSV_LF_CSV_REF_LIMIT_LOWER_Pos          0UL
#define CSV_LF_CSV_REF_LIMIT_LOWER_Msk          0xFFUL
#define CSV_LF_CSV_REF_LIMIT_UPPER_Pos          16UL
#define CSV_LF_CSV_REF_LIMIT_UPPER_Msk          0xFF0000UL
/* CSV_LF_CSV.MON_CTL */
#define CSV_LF_CSV_MON_CTL_PERIOD_Pos           0UL
#define CSV_LF_CSV_MON_CTL_PERIOD_Msk           0xFFUL


/* CSV_ILO_CSV.REF_CTL */
#define CSV_ILO_CSV_REF_CTL_STARTUP_Pos         0UL
#define CSV_ILO_CSV_REF_CTL_STARTUP_Msk         0xFFUL
#define CSV_ILO_CSV_REF_CTL_CSV_EN_Pos          31UL
#define CSV_ILO_CSV_REF_CTL_CSV_EN_Msk          0x80000000UL
/* CSV_ILO_CSV.REF_LIMIT */
#define CSV_ILO_CSV_REF_LIMIT_LOWER_Pos         0UL
#define CSV_ILO_CSV_REF_LIMIT_LOWER_Msk         0xFFUL
#define CSV_ILO_CSV_REF_LIMIT_UPPER_Pos         16UL
#define CSV_ILO_CSV_REF_LIMIT_UPPER_Msk         0xFF0000UL
/* CSV_ILO_CSV.MON_CTL */
#define CSV_ILO_CSV_MON_CTL_PERIOD_Pos          0UL
#define CSV_ILO_CSV_MON_CTL_PERIOD_Msk          0xFFUL


/* CLK_PLL400M.CONFIG */
#define CLK_PLL400M_CONFIG_FEEDBACK_DIV_Pos     0UL
#define CLK_PLL400M_CONFIG_FEEDBACK_DIV_Msk     0xFFUL
#define CLK_PLL400M_CONFIG_REFERENCE_DIV_Pos    8UL
#define CLK_PLL400M_CONFIG_REFERENCE_DIV_Msk    0x1F00UL
#define CLK_PLL400M_CONFIG_OUTPUT_DIV_Pos       16UL
#define CLK_PLL400M_CONFIG_OUTPUT_DIV_Msk       0x1F0000UL
#define CLK_PLL400M_CONFIG_LOCK_DELAY_Pos       25UL
#define CLK_PLL400M_CONFIG_LOCK_DELAY_Msk       0x6000000UL
#define CLK_PLL400M_CONFIG_BYPASS_SEL_Pos       28UL
#define CLK_PLL400M_CONFIG_BYPASS_SEL_Msk       0x30000000UL
#define CLK_PLL400M_CONFIG_ENABLE_Pos           31UL
#define CLK_PLL400M_CONFIG_ENABLE_Msk           0x80000000UL
/* CLK_PLL400M.CONFIG2 */
#define CLK_PLL400M_CONFIG2_FRAC_DIV_Pos        0UL
#define CLK_PLL400M_CONFIG2_FRAC_DIV_Msk        0xFFFFFFUL
#define CLK_PLL400M_CONFIG2_FRAC_DITHER_EN_Pos  28UL
#define CLK_PLL400M_CONFIG2_FRAC_DITHER_EN_Msk  0x70000000UL
#define CLK_PLL400M_CONFIG2_FRAC_EN_Pos         31UL
#define CLK_PLL400M_CONFIG2_FRAC_EN_Msk         0x80000000UL
/* CLK_PLL400M.CONFIG3 */
#define CLK_PLL400M_CONFIG3_SSCG_DEPTH_Pos      0UL
#define CLK_PLL400M_CONFIG3_SSCG_DEPTH_Msk      0x3FFUL
#define CLK_PLL400M_CONFIG3_SSCG_RATE_Pos       16UL
#define CLK_PLL400M_CONFIG3_SSCG_RATE_Msk       0x70000UL
#define CLK_PLL400M_CONFIG3_SSCG_DITHER_EN_Pos  24UL
#define CLK_PLL400M_CONFIG3_SSCG_DITHER_EN_Msk  0x1000000UL
#define CLK_PLL400M_CONFIG3_SSCG_MODE_Pos       28UL
#define CLK_PLL400M_CONFIG3_SSCG_MODE_Msk       0x10000000UL
#define CLK_PLL400M_CONFIG3_SSCG_EN_Pos         31UL
#define CLK_PLL400M_CONFIG3_SSCG_EN_Msk         0x80000000UL
/* CLK_PLL400M.STATUS */
#define CLK_PLL400M_STATUS_LOCKED_Pos           0UL
#define CLK_PLL400M_STATUS_LOCKED_Msk           0x1UL
#define CLK_PLL400M_STATUS_UNLOCK_OCCURRED_Pos  1UL
#define CLK_PLL400M_STATUS_UNLOCK_OCCURRED_Msk  0x2UL


/* MCWDT_CTR.CTL */
#define MCWDT_CTR_CTL_ENABLED_Pos               0UL
#define MCWDT_CTR_CTL_ENABLED_Msk               0x1UL
#define MCWDT_CTR_CTL_ENABLE_Pos                31UL
#define MCWDT_CTR_CTL_ENABLE_Msk                0x80000000UL
/* MCWDT_CTR.LOWER_LIMIT */
#define MCWDT_CTR_LOWER_LIMIT_LOWER_LIMIT_Pos   0UL
#define MCWDT_CTR_LOWER_LIMIT_LOWER_LIMIT_Msk   0xFFFFUL
/* MCWDT_CTR.UPPER_LIMIT */
#define MCWDT_CTR_UPPER_LIMIT_UPPER_LIMIT_Pos   0UL
#define MCWDT_CTR_UPPER_LIMIT_UPPER_LIMIT_Msk   0xFFFFUL
/* MCWDT_CTR.WARN_LIMIT */
#define MCWDT_CTR_WARN_LIMIT_WARN_LIMIT_Pos     0UL
#define MCWDT_CTR_WARN_LIMIT_WARN_LIMIT_Msk     0xFFFFUL
/* MCWDT_CTR.CONFIG */
#define MCWDT_CTR_CONFIG_LOWER_ACTION_Pos       0UL
#define MCWDT_CTR_CONFIG_LOWER_ACTION_Msk       0x3UL
#define MCWDT_CTR_CONFIG_UPPER_ACTION_Pos       4UL
#define MCWDT_CTR_CONFIG_UPPER_ACTION_Msk       0x30UL
#define MCWDT_CTR_CONFIG_WARN_ACTION_Pos        8UL
#define MCWDT_CTR_CONFIG_WARN_ACTION_Msk        0x100UL
#define MCWDT_CTR_CONFIG_AUTO_SERVICE_Pos       12UL
#define MCWDT_CTR_CONFIG_AUTO_SERVICE_Msk       0x1000UL
#define MCWDT_CTR_CONFIG_DEBUG_TRIGGER_EN_Pos   28UL
#define MCWDT_CTR_CONFIG_DEBUG_TRIGGER_EN_Msk   0x10000000UL
#define MCWDT_CTR_CONFIG_SLEEPDEEP_PAUSE_Pos    30UL
#define MCWDT_CTR_CONFIG_SLEEPDEEP_PAUSE_Msk    0x40000000UL
#define MCWDT_CTR_CONFIG_DEBUG_RUN_Pos          31UL
#define MCWDT_CTR_CONFIG_DEBUG_RUN_Msk          0x80000000UL
/* MCWDT_CTR.CNT */
#define MCWDT_CTR_CNT_CNT_Pos                   0UL
#define MCWDT_CTR_CNT_CNT_Msk                   0xFFFFUL


/* MCWDT.CPU_SELECT */
#define MCWDT_CPU_SELECT_CPU_SEL_Pos            0UL
#define MCWDT_CPU_SELECT_CPU_SEL_Msk            0x3UL
/* MCWDT.CTR2_CTL */
#define MCWDT_CTR2_CTL_ENABLED_Pos              0UL
#define MCWDT_CTR2_CTL_ENABLED_Msk              0x1UL
#define MCWDT_CTR2_CTL_ENABLE_Pos               31UL
#define MCWDT_CTR2_CTL_ENABLE_Msk               0x80000000UL
/* MCWDT.CTR2_CONFIG */
#define MCWDT_CTR2_CONFIG_ACTION_Pos            0UL
#define MCWDT_CTR2_CONFIG_ACTION_Msk            0x1UL
#define MCWDT_CTR2_CONFIG_BITS_Pos              16UL
#define MCWDT_CTR2_CONFIG_BITS_Msk              0x1F0000UL
#define MCWDT_CTR2_CONFIG_DEBUG_TRIGGER_EN_Pos  28UL
#define MCWDT_CTR2_CONFIG_DEBUG_TRIGGER_EN_Msk  0x10000000UL
#define MCWDT_CTR2_CONFIG_SLEEPDEEP_PAUSE_Pos   30UL
#define MCWDT_CTR2_CONFIG_SLEEPDEEP_PAUSE_Msk   0x40000000UL
#define MCWDT_CTR2_CONFIG_DEBUG_RUN_Pos         31UL
#define MCWDT_CTR2_CONFIG_DEBUG_RUN_Msk         0x80000000UL
/* MCWDT.CTR2_CNT */
#define MCWDT_CTR2_CNT_CNT2_Pos                 0UL
#define MCWDT_CTR2_CNT_CNT2_Msk                 0xFFFFFFFFUL
/* MCWDT.LOCK */
#define MCWDT_LOCK_MCWDT_LOCK_Pos               0UL
#define MCWDT_LOCK_MCWDT_LOCK_Msk               0x3UL
/* MCWDT.SERVICE */
#define MCWDT_SERVICE_CTR0_SERVICE_Pos          0UL
#define MCWDT_SERVICE_CTR0_SERVICE_Msk          0x1UL
#define MCWDT_SERVICE_CTR1_SERVICE_Pos          1UL
#define MCWDT_SERVICE_CTR1_SERVICE_Msk          0x2UL
/* MCWDT.INTR */
#define MCWDT_INTR_CTR0_INT_Pos                 0UL
#define MCWDT_INTR_CTR0_INT_Msk                 0x1UL
#define MCWDT_INTR_CTR1_INT_Pos                 1UL
#define MCWDT_INTR_CTR1_INT_Msk                 0x2UL
#define MCWDT_INTR_CTR2_INT_Pos                 2UL
#define MCWDT_INTR_CTR2_INT_Msk                 0x4UL
/* MCWDT.INTR_SET */
#define MCWDT_INTR_SET_CTR0_INT_Pos             0UL
#define MCWDT_INTR_SET_CTR0_INT_Msk             0x1UL
#define MCWDT_INTR_SET_CTR1_INT_Pos             1UL
#define MCWDT_INTR_SET_CTR1_INT_Msk             0x2UL
#define MCWDT_INTR_SET_CTR2_INT_Pos             2UL
#define MCWDT_INTR_SET_CTR2_INT_Msk             0x4UL
/* MCWDT.INTR_MASK */
#define MCWDT_INTR_MASK_CTR0_INT_Pos            0UL
#define MCWDT_INTR_MASK_CTR0_INT_Msk            0x1UL
#define MCWDT_INTR_MASK_CTR1_INT_Pos            1UL
#define MCWDT_INTR_MASK_CTR1_INT_Msk            0x2UL
#define MCWDT_INTR_MASK_CTR2_INT_Pos            2UL
#define MCWDT_INTR_MASK_CTR2_INT_Msk            0x4UL
/* MCWDT.INTR_MASKED */
#define MCWDT_INTR_MASKED_CTR0_INT_Pos          0UL
#define MCWDT_INTR_MASKED_CTR0_INT_Msk          0x1UL
#define MCWDT_INTR_MASKED_CTR1_INT_Pos          1UL
#define MCWDT_INTR_MASKED_CTR1_INT_Msk          0x2UL
#define MCWDT_INTR_MASKED_CTR2_INT_Pos          2UL
#define MCWDT_INTR_MASKED_CTR2_INT_Msk          0x4UL


/* WDT.CTL */
#define WDT_CTL_ENABLED_Pos                     0UL
#define WDT_CTL_ENABLED_Msk                     0x1UL
#define WDT_CTL_ENABLE_Pos                      31UL
#define WDT_CTL_ENABLE_Msk                      0x80000000UL
/* WDT.LOWER_LIMIT */
#define WDT_LOWER_LIMIT_LOWER_LIMIT_Pos         0UL
#define WDT_LOWER_LIMIT_LOWER_LIMIT_Msk         0xFFFFFFFFUL
/* WDT.UPPER_LIMIT */
#define WDT_UPPER_LIMIT_UPPER_LIMIT_Pos         0UL
#define WDT_UPPER_LIMIT_UPPER_LIMIT_Msk         0xFFFFFFFFUL
/* WDT.WARN_LIMIT */
#define WDT_WARN_LIMIT_WARN_LIMIT_Pos           0UL
#define WDT_WARN_LIMIT_WARN_LIMIT_Msk           0xFFFFFFFFUL
/* WDT.CONFIG */
#define WDT_CONFIG_LOWER_ACTION_Pos             0UL
#define WDT_CONFIG_LOWER_ACTION_Msk             0x1UL
#define WDT_CONFIG_UPPER_ACTION_Pos             4UL
#define WDT_CONFIG_UPPER_ACTION_Msk             0x10UL
#define WDT_CONFIG_WARN_ACTION_Pos              8UL
#define WDT_CONFIG_WARN_ACTION_Msk              0x100UL
#define WDT_CONFIG_AUTO_SERVICE_Pos             12UL
#define WDT_CONFIG_AUTO_SERVICE_Msk             0x1000UL
#define WDT_CONFIG_DEBUG_TRIGGER_EN_Pos         28UL
#define WDT_CONFIG_DEBUG_TRIGGER_EN_Msk         0x10000000UL
#define WDT_CONFIG_DPSLP_PAUSE_Pos              29UL
#define WDT_CONFIG_DPSLP_PAUSE_Msk              0x20000000UL
#define WDT_CONFIG_HIB_PAUSE_Pos                30UL
#define WDT_CONFIG_HIB_PAUSE_Msk                0x40000000UL
#define WDT_CONFIG_DEBUG_RUN_Pos                31UL
#define WDT_CONFIG_DEBUG_RUN_Msk                0x80000000UL
/* WDT.CNT */
#define WDT_CNT_CNT_Pos                         0UL
#define WDT_CNT_CNT_Msk                         0xFFFFFFFFUL
/* WDT.LOCK */
#define WDT_LOCK_WDT_LOCK_Pos                   0UL
#define WDT_LOCK_WDT_LOCK_Msk                   0x3UL
/* WDT.SERVICE */
#define WDT_SERVICE_SERVICE_Pos                 0UL
#define WDT_SERVICE_SERVICE_Msk                 0x1UL
/* WDT.INTR */
#define WDT_INTR_WDT_Pos                        0UL
#define WDT_INTR_WDT_Msk                        0x1UL
/* WDT.INTR_SET */
#define WDT_INTR_SET_WDT_Pos                    0UL
#define WDT_INTR_SET_WDT_Msk                    0x1UL
/* WDT.INTR_MASK */
#define WDT_INTR_MASK_WDT_Pos                   0UL
#define WDT_INTR_MASK_WDT_Msk                   0x1UL
/* WDT.INTR_MASKED */
#define WDT_INTR_MASKED_WDT_Pos                 0UL
#define WDT_INTR_MASKED_WDT_Msk                 0x1UL


/* SRSS.PWR_LVD_STATUS */
#define SRSS_PWR_LVD_STATUS_HVLVD1_OUT_Pos      0UL
#define SRSS_PWR_LVD_STATUS_HVLVD1_OUT_Msk      0x1UL
/* SRSS.PWR_LVD_STATUS2 */
#define SRSS_PWR_LVD_STATUS2_HVLVD2_OUT_Pos     0UL
#define SRSS_PWR_LVD_STATUS2_HVLVD2_OUT_Msk     0x1UL
/* SRSS.CLK_DSI_SELECT */
#define SRSS_CLK_DSI_SELECT_DSI_MUX_Pos         0UL
#define SRSS_CLK_DSI_SELECT_DSI_MUX_Msk         0x1FUL
/* SRSS.CLK_OUTPUT_FAST */
#define SRSS_CLK_OUTPUT_FAST_FAST_SEL0_Pos      0UL
#define SRSS_CLK_OUTPUT_FAST_FAST_SEL0_Msk      0xFUL
#define SRSS_CLK_OUTPUT_FAST_PATH_SEL0_Pos      4UL
#define SRSS_CLK_OUTPUT_FAST_PATH_SEL0_Msk      0xF0UL
#define SRSS_CLK_OUTPUT_FAST_HFCLK_SEL0_Pos     8UL
#define SRSS_CLK_OUTPUT_FAST_HFCLK_SEL0_Msk     0xF00UL
#define SRSS_CLK_OUTPUT_FAST_FAST_SEL1_Pos      16UL
#define SRSS_CLK_OUTPUT_FAST_FAST_SEL1_Msk      0xF0000UL
#define SRSS_CLK_OUTPUT_FAST_PATH_SEL1_Pos      20UL
#define SRSS_CLK_OUTPUT_FAST_PATH_SEL1_Msk      0xF00000UL
#define SRSS_CLK_OUTPUT_FAST_HFCLK_SEL1_Pos     24UL
#define SRSS_CLK_OUTPUT_FAST_HFCLK_SEL1_Msk     0xF000000UL
/* SRSS.CLK_OUTPUT_SLOW */
#define SRSS_CLK_OUTPUT_SLOW_SLOW_SEL0_Pos      0UL
#define SRSS_CLK_OUTPUT_SLOW_SLOW_SEL0_Msk      0xFUL
#define SRSS_CLK_OUTPUT_SLOW_SLOW_SEL1_Pos      4UL
#define SRSS_CLK_OUTPUT_SLOW_SLOW_SEL1_Msk      0xF0UL
/* SRSS.CLK_CAL_CNT1 */
#define SRSS_CLK_CAL_CNT1_CAL_COUNTER1_Pos      0UL
#define SRSS_CLK_CAL_CNT1_CAL_COUNTER1_Msk      0xFFFFFFUL
#define SRSS_CLK_CAL_CNT1_CAL_COUNTER_DONE_Pos  31UL
#define SRSS_CLK_CAL_CNT1_CAL_COUNTER_DONE_Msk  0x80000000UL
/* SRSS.CLK_CAL_CNT2 */
#define SRSS_CLK_CAL_CNT2_CAL_COUNTER2_Pos      0UL
#define SRSS_CLK_CAL_CNT2_CAL_COUNTER2_Msk      0xFFFFFFUL
/* SRSS.SRSS_INTR */
#define SRSS_SRSS_INTR_HVLVD1_Pos               1UL
#define SRSS_SRSS_INTR_HVLVD1_Msk               0x2UL
#define SRSS_SRSS_INTR_HVLVD2_Pos               2UL
#define SRSS_SRSS_INTR_HVLVD2_Msk               0x4UL
#define SRSS_SRSS_INTR_CLK_CAL_Pos              5UL
#define SRSS_SRSS_INTR_CLK_CAL_Msk              0x20UL
/* SRSS.SRSS_INTR_SET */
#define SRSS_SRSS_INTR_SET_HVLVD1_Pos           1UL
#define SRSS_SRSS_INTR_SET_HVLVD1_Msk           0x2UL
#define SRSS_SRSS_INTR_SET_HVLVD2_Pos           2UL
#define SRSS_SRSS_INTR_SET_HVLVD2_Msk           0x4UL
#define SRSS_SRSS_INTR_SET_CLK_CAL_Pos          5UL
#define SRSS_SRSS_INTR_SET_CLK_CAL_Msk          0x20UL
/* SRSS.SRSS_INTR_MASK */
#define SRSS_SRSS_INTR_MASK_HVLVD1_Pos          1UL
#define SRSS_SRSS_INTR_MASK_HVLVD1_Msk          0x2UL
#define SRSS_SRSS_INTR_MASK_HVLVD2_Pos          2UL
#define SRSS_SRSS_INTR_MASK_HVLVD2_Msk          0x4UL
#define SRSS_SRSS_INTR_MASK_CLK_CAL_Pos         5UL
#define SRSS_SRSS_INTR_MASK_CLK_CAL_Msk         0x20UL
/* SRSS.SRSS_INTR_MASKED */
#define SRSS_SRSS_INTR_MASKED_HVLVD1_Pos        1UL
#define SRSS_SRSS_INTR_MASKED_HVLVD1_Msk        0x2UL
#define SRSS_SRSS_INTR_MASKED_HVLVD2_Pos        2UL
#define SRSS_SRSS_INTR_MASKED_HVLVD2_Msk        0x4UL
#define SRSS_SRSS_INTR_MASKED_CLK_CAL_Pos       5UL
#define SRSS_SRSS_INTR_MASKED_CLK_CAL_Msk       0x20UL
/* SRSS.PWR_CTL */
#define SRSS_PWR_CTL_POWER_MODE_Pos             0UL
#define SRSS_PWR_CTL_POWER_MODE_Msk             0x3UL
#define SRSS_PWR_CTL_DEBUG_SESSION_Pos          4UL
#define SRSS_PWR_CTL_DEBUG_SESSION_Msk          0x10UL
#define SRSS_PWR_CTL_LPM_READY_Pos              5UL
#define SRSS_PWR_CTL_LPM_READY_Msk              0x20UL
/* SRSS.PWR_CTL2 */
#define SRSS_PWR_CTL2_LINREG_DIS_Pos            0UL
#define SRSS_PWR_CTL2_LINREG_DIS_Msk            0x1UL
#define SRSS_PWR_CTL2_LINREG_OK_Pos             1UL
#define SRSS_PWR_CTL2_LINREG_OK_Msk             0x2UL
#define SRSS_PWR_CTL2_LINREG_LPMODE_Pos         2UL
#define SRSS_PWR_CTL2_LINREG_LPMODE_Msk         0x4UL
#define SRSS_PWR_CTL2_DPSLP_REG_DIS_Pos         4UL
#define SRSS_PWR_CTL2_DPSLP_REG_DIS_Msk         0x10UL
#define SRSS_PWR_CTL2_RET_REG_DIS_Pos           8UL
#define SRSS_PWR_CTL2_RET_REG_DIS_Msk           0x100UL
#define SRSS_PWR_CTL2_NWELL_REG_DIS_Pos         12UL
#define SRSS_PWR_CTL2_NWELL_REG_DIS_Msk         0x1000UL
#define SRSS_PWR_CTL2_REFV_DIS_Pos              16UL
#define SRSS_PWR_CTL2_REFV_DIS_Msk              0x10000UL
#define SRSS_PWR_CTL2_REFV_OK_Pos               17UL
#define SRSS_PWR_CTL2_REFV_OK_Msk               0x20000UL
#define SRSS_PWR_CTL2_REFVBUF_DIS_Pos           20UL
#define SRSS_PWR_CTL2_REFVBUF_DIS_Msk           0x100000UL
#define SRSS_PWR_CTL2_REFVBUF_OK_Pos            21UL
#define SRSS_PWR_CTL2_REFVBUF_OK_Msk            0x200000UL
#define SRSS_PWR_CTL2_REFVBUF_LPMODE_Pos        22UL
#define SRSS_PWR_CTL2_REFVBUF_LPMODE_Msk        0x400000UL
#define SRSS_PWR_CTL2_REFI_DIS_Pos              24UL
#define SRSS_PWR_CTL2_REFI_DIS_Msk              0x1000000UL
#define SRSS_PWR_CTL2_REFI_OK_Pos               25UL
#define SRSS_PWR_CTL2_REFI_OK_Msk               0x2000000UL
#define SRSS_PWR_CTL2_REFI_LPMODE_Pos           26UL
#define SRSS_PWR_CTL2_REFI_LPMODE_Msk           0x4000000UL
#define SRSS_PWR_CTL2_PORBOD_LPMODE_Pos         27UL
#define SRSS_PWR_CTL2_PORBOD_LPMODE_Msk         0x8000000UL
#define SRSS_PWR_CTL2_BGREF_LPMODE_Pos          28UL
#define SRSS_PWR_CTL2_BGREF_LPMODE_Msk          0x10000000UL
#define SRSS_PWR_CTL2_PLL_LS_BYPASS_Pos         31UL
#define SRSS_PWR_CTL2_PLL_LS_BYPASS_Msk         0x80000000UL
/* SRSS.PWR_HIBERNATE */
#define SRSS_PWR_HIBERNATE_TOKEN_Pos            0UL
#define SRSS_PWR_HIBERNATE_TOKEN_Msk            0xFFUL
#define SRSS_PWR_HIBERNATE_UNLOCK_Pos           8UL
#define SRSS_PWR_HIBERNATE_UNLOCK_Msk           0xFF00UL
#define SRSS_PWR_HIBERNATE_FREEZE_Pos           17UL
#define SRSS_PWR_HIBERNATE_FREEZE_Msk           0x20000UL
#define SRSS_PWR_HIBERNATE_MASK_HIBALARM_Pos    18UL
#define SRSS_PWR_HIBERNATE_MASK_HIBALARM_Msk    0x40000UL
#define SRSS_PWR_HIBERNATE_MASK_HIBWDT_Pos      19UL
#define SRSS_PWR_HIBERNATE_MASK_HIBWDT_Msk      0x80000UL
#define SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Pos  20UL
#define SRSS_PWR_HIBERNATE_POLARITY_HIBPIN_Msk  0xF00000UL
#define SRSS_PWR_HIBERNATE_MASK_HIBPIN_Pos      24UL
#define SRSS_PWR_HIBERNATE_MASK_HIBPIN_Msk      0xF000000UL
#define SRSS_PWR_HIBERNATE_HIBERNATE_DISABLE_Pos 30UL
#define SRSS_PWR_HIBERNATE_HIBERNATE_DISABLE_Msk 0x40000000UL
#define SRSS_PWR_HIBERNATE_HIBERNATE_Pos        31UL
#define SRSS_PWR_HIBERNATE_HIBERNATE_Msk        0x80000000UL
/* SRSS.PWR_BUCK_CTL */
#define SRSS_PWR_BUCK_CTL_BUCK_OUT1_SEL_Pos     0UL
#define SRSS_PWR_BUCK_CTL_BUCK_OUT1_SEL_Msk     0x7UL
#define SRSS_PWR_BUCK_CTL_BUCK_EN_Pos           30UL
#define SRSS_PWR_BUCK_CTL_BUCK_EN_Msk           0x40000000UL
#define SRSS_PWR_BUCK_CTL_BUCK_OUT1_EN_Pos      31UL
#define SRSS_PWR_BUCK_CTL_BUCK_OUT1_EN_Msk      0x80000000UL
/* SRSS.PWR_BUCK_CTL2 */
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_SEL_Pos    0UL
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_SEL_Msk    0x7UL
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_HW_SEL_Pos 30UL
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_HW_SEL_Msk 0x40000000UL
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_EN_Pos     31UL
#define SRSS_PWR_BUCK_CTL2_BUCK_OUT2_EN_Msk     0x80000000UL
/* SRSS.PWR_SSV_CTL */
#define SRSS_PWR_SSV_CTL_BODVDDD_VSEL_Pos       0UL
#define SRSS_PWR_SSV_CTL_BODVDDD_VSEL_Msk       0x1UL
#define SRSS_PWR_SSV_CTL_BODVDDD_ENABLE_Pos     3UL
#define SRSS_PWR_SSV_CTL_BODVDDD_ENABLE_Msk     0x8UL
#define SRSS_PWR_SSV_CTL_BODVDDA_VSEL_Pos       4UL
#define SRSS_PWR_SSV_CTL_BODVDDA_VSEL_Msk       0x10UL
#define SRSS_PWR_SSV_CTL_BODVDDA_ACTION_Pos     6UL
#define SRSS_PWR_SSV_CTL_BODVDDA_ACTION_Msk     0xC0UL
#define SRSS_PWR_SSV_CTL_BODVDDA_ENABLE_Pos     8UL
#define SRSS_PWR_SSV_CTL_BODVDDA_ENABLE_Msk     0x100UL
#define SRSS_PWR_SSV_CTL_BODVCCD_ENABLE_Pos     11UL
#define SRSS_PWR_SSV_CTL_BODVCCD_ENABLE_Msk     0x800UL
#define SRSS_PWR_SSV_CTL_OVDVDDD_VSEL_Pos       16UL
#define SRSS_PWR_SSV_CTL_OVDVDDD_VSEL_Msk       0x10000UL
#define SRSS_PWR_SSV_CTL_OVDVDDD_ENABLE_Pos     19UL
#define SRSS_PWR_SSV_CTL_OVDVDDD_ENABLE_Msk     0x80000UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_VSEL_Pos       20UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_VSEL_Msk       0x100000UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_ACTION_Pos     22UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_ACTION_Msk     0xC00000UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_ENABLE_Pos     24UL
#define SRSS_PWR_SSV_CTL_OVDVDDA_ENABLE_Msk     0x1000000UL
#define SRSS_PWR_SSV_CTL_OVDVCCD_ENABLE_Pos     27UL
#define SRSS_PWR_SSV_CTL_OVDVCCD_ENABLE_Msk     0x8000000UL
/* SRSS.PWR_SSV_STATUS */
#define SRSS_PWR_SSV_STATUS_BODVDDD_OK_Pos      0UL
#define SRSS_PWR_SSV_STATUS_BODVDDD_OK_Msk      0x1UL
#define SRSS_PWR_SSV_STATUS_BODVDDA_OK_Pos      1UL
#define SRSS_PWR_SSV_STATUS_BODVDDA_OK_Msk      0x2UL
#define SRSS_PWR_SSV_STATUS_BODVCCD_OK_Pos      2UL
#define SRSS_PWR_SSV_STATUS_BODVCCD_OK_Msk      0x4UL
#define SRSS_PWR_SSV_STATUS_OVDVDDD_OK_Pos      8UL
#define SRSS_PWR_SSV_STATUS_OVDVDDD_OK_Msk      0x100UL
#define SRSS_PWR_SSV_STATUS_OVDVDDA_OK_Pos      9UL
#define SRSS_PWR_SSV_STATUS_OVDVDDA_OK_Msk      0x200UL
#define SRSS_PWR_SSV_STATUS_OVDVCCD_OK_Pos      10UL
#define SRSS_PWR_SSV_STATUS_OVDVCCD_OK_Msk      0x400UL
#define SRSS_PWR_SSV_STATUS_OCD_ACT_LINREG_OK_Pos 16UL
#define SRSS_PWR_SSV_STATUS_OCD_ACT_LINREG_OK_Msk 0x10000UL
#define SRSS_PWR_SSV_STATUS_OCD_DPSLP_REG_OK_Pos 17UL
#define SRSS_PWR_SSV_STATUS_OCD_DPSLP_REG_OK_Msk 0x20000UL
/* SRSS.PWR_LVD_CTL */
#define SRSS_PWR_LVD_CTL_HVLVD1_TRIPSEL_Pos     0UL
#define SRSS_PWR_LVD_CTL_HVLVD1_TRIPSEL_Msk     0xFUL
#define SRSS_PWR_LVD_CTL_HVLVD1_SRCSEL_Pos      4UL
#define SRSS_PWR_LVD_CTL_HVLVD1_SRCSEL_Msk      0x70UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EN_Pos          7UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EN_Msk          0x80UL
#define SRSS_PWR_LVD_CTL_HVLVD1_TRIPSEL_HT_Pos  8UL
#define SRSS_PWR_LVD_CTL_HVLVD1_TRIPSEL_HT_Msk  0x1F00UL
#define SRSS_PWR_LVD_CTL_HVLVD1_DPSLP_EN_HT_Pos 14UL
#define SRSS_PWR_LVD_CTL_HVLVD1_DPSLP_EN_HT_Msk 0x4000UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EN_HT_Pos       15UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EN_HT_Msk       0x8000UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EDGE_SEL_Pos    16UL
#define SRSS_PWR_LVD_CTL_HVLVD1_EDGE_SEL_Msk    0x30000UL
#define SRSS_PWR_LVD_CTL_HVLVD1_ACTION_Pos      18UL
#define SRSS_PWR_LVD_CTL_HVLVD1_ACTION_Msk      0x40000UL
/* SRSS.PWR_LVD_CTL2 */
#define SRSS_PWR_LVD_CTL2_HVLVD2_TRIPSEL_HT_Pos 8UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_TRIPSEL_HT_Msk 0x1F00UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_DPSLP_EN_HT_Pos 14UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_DPSLP_EN_HT_Msk 0x4000UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_EN_HT_Pos      15UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_EN_HT_Msk      0x8000UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_EDGE_SEL_Pos   16UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_EDGE_SEL_Msk   0x30000UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_ACTION_Pos     18UL
#define SRSS_PWR_LVD_CTL2_HVLVD2_ACTION_Msk     0x40000UL
/* SRSS.PWR_REGHC_CTL */
#define SRSS_PWR_REGHC_CTL_REGHC_MODE_Pos       0UL
#define SRSS_PWR_REGHC_CTL_REGHC_MODE_Msk       0x1UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_DRV_VOUT_Pos 2UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_DRV_VOUT_Msk 0xCUL
#define SRSS_PWR_REGHC_CTL_REGHC_VADJ_Pos       4UL
#define SRSS_PWR_REGHC_CTL_REGHC_VADJ_Msk       0x1F0UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_USE_LINREG_Pos 10UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_USE_LINREG_Msk 0x400UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_USE_RADJ_Pos 11UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_USE_RADJ_Msk 0x800UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_RADJ_Pos  12UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_RADJ_Msk  0x7000UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_CTL_OUTEN_Pos 16UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_CTL_OUTEN_Msk 0x10000UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_CTL_POLARITY_Pos 17UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_CTL_POLARITY_Msk 0x20000UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_INEN_Pos 18UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_INEN_Msk 0x40000UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_POLARITY_Pos 19UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_POLARITY_Msk 0x80000UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_WAIT_Pos 20UL
#define SRSS_PWR_REGHC_CTL_REGHC_PMIC_STATUS_WAIT_Msk 0x3FF00000UL
#define SRSS_PWR_REGHC_CTL_REGHC_TRANS_USE_OCD_Pos 30UL
#define SRSS_PWR_REGHC_CTL_REGHC_TRANS_USE_OCD_Msk 0x40000000UL
#define SRSS_PWR_REGHC_CTL_REGHC_CONFIGURED_Pos 31UL
#define SRSS_PWR_REGHC_CTL_REGHC_CONFIGURED_Msk 0x80000000UL
/* SRSS.PWR_REGHC_STATUS */
#define SRSS_PWR_REGHC_STATUS_REGHC_ENABLED_Pos 0UL
#define SRSS_PWR_REGHC_STATUS_REGHC_ENABLED_Msk 0x1UL
#define SRSS_PWR_REGHC_STATUS_REGHC_OCD_OK_Pos  1UL
#define SRSS_PWR_REGHC_STATUS_REGHC_OCD_OK_Msk  0x2UL
#define SRSS_PWR_REGHC_STATUS_REGHC_CKT_OK_Pos  2UL
#define SRSS_PWR_REGHC_STATUS_REGHC_CKT_OK_Msk  0x4UL
#define SRSS_PWR_REGHC_STATUS_REGHC_UV_OUT_Pos  8UL
#define SRSS_PWR_REGHC_STATUS_REGHC_UV_OUT_Msk  0x100UL
#define SRSS_PWR_REGHC_STATUS_REGHC_OV_OUT_Pos  9UL
#define SRSS_PWR_REGHC_STATUS_REGHC_OV_OUT_Msk  0x200UL
#define SRSS_PWR_REGHC_STATUS_REGHC_PMIC_STATUS_OK_Pos 12UL
#define SRSS_PWR_REGHC_STATUS_REGHC_PMIC_STATUS_OK_Msk 0x1000UL
#define SRSS_PWR_REGHC_STATUS_REGHC_SEQ_BUSY_Pos 31UL
#define SRSS_PWR_REGHC_STATUS_REGHC_SEQ_BUSY_Msk 0x80000000UL
/* SRSS.PWR_REGHC_CTL2 */
#define SRSS_PWR_REGHC_CTL2_REGHC_PMIC_STATUS_TIMEOUT_Pos 0UL
#define SRSS_PWR_REGHC_CTL2_REGHC_PMIC_STATUS_TIMEOUT_Msk 0xFFUL
#define SRSS_PWR_REGHC_CTL2_REGHC_EN_Pos        31UL
#define SRSS_PWR_REGHC_CTL2_REGHC_EN_Msk        0x80000000UL
/* SRSS.PWR_REGHC_CTL4 */
#define SRSS_PWR_REGHC_CTL4_REGHC_PMIC_VADJ_DIS_Pos 30UL
#define SRSS_PWR_REGHC_CTL4_REGHC_PMIC_VADJ_DIS_Msk 0x40000000UL
#define SRSS_PWR_REGHC_CTL4_REGHC_PMIC_DPSLP_Pos 31UL
#define SRSS_PWR_REGHC_CTL4_REGHC_PMIC_DPSLP_Msk 0x80000000UL
/* SRSS.PWR_HIB_DATA */
#define SRSS_PWR_HIB_DATA_HIB_DATA_Pos          0UL
#define SRSS_PWR_HIB_DATA_HIB_DATA_Msk          0xFFFFFFFFUL
/* SRSS.PWR_PMIC_CTL */
#define SRSS_PWR_PMIC_CTL_PMIC_VREF_Pos         2UL
#define SRSS_PWR_PMIC_CTL_PMIC_VREF_Msk         0xCUL
#define SRSS_PWR_PMIC_CTL_PMIC_VADJ_Pos         4UL
#define SRSS_PWR_PMIC_CTL_PMIC_VADJ_Msk         0x1F0UL
#define SRSS_PWR_PMIC_CTL_PMIC_USE_LINREG_Pos   10UL
#define SRSS_PWR_PMIC_CTL_PMIC_USE_LINREG_Msk   0x400UL
#define SRSS_PWR_PMIC_CTL_PMIC_VADJ_BUF_EN_Pos  15UL
#define SRSS_PWR_PMIC_CTL_PMIC_VADJ_BUF_EN_Msk  0x8000UL
#define SRSS_PWR_PMIC_CTL_PMIC_CTL_OUTEN_Pos    16UL
#define SRSS_PWR_PMIC_CTL_PMIC_CTL_OUTEN_Msk    0x10000UL
#define SRSS_PWR_PMIC_CTL_PMIC_CTL_POLARITY_Pos 17UL
#define SRSS_PWR_PMIC_CTL_PMIC_CTL_POLARITY_Msk 0x20000UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_INEN_Pos  18UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_INEN_Msk  0x40000UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_POLARITY_Pos 19UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_POLARITY_Msk 0x80000UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_WAIT_Pos  20UL
#define SRSS_PWR_PMIC_CTL_PMIC_STATUS_WAIT_Msk  0x3FF00000UL
#define SRSS_PWR_PMIC_CTL_PMIC_CONFIGURED_Pos   31UL
#define SRSS_PWR_PMIC_CTL_PMIC_CONFIGURED_Msk   0x80000000UL
/* SRSS.PWR_PMIC_STATUS */
#define SRSS_PWR_PMIC_STATUS_PMIC_ENABLED_Pos   0UL
#define SRSS_PWR_PMIC_STATUS_PMIC_ENABLED_Msk   0x1UL
#define SRSS_PWR_PMIC_STATUS_PMIC_STATUS_OK_Pos 12UL
#define SRSS_PWR_PMIC_STATUS_PMIC_STATUS_OK_Msk 0x1000UL
#define SRSS_PWR_PMIC_STATUS_PMIC_SEQ_BUSY_Pos  31UL
#define SRSS_PWR_PMIC_STATUS_PMIC_SEQ_BUSY_Msk  0x80000000UL
/* SRSS.PWR_PMIC_CTL2 */
#define SRSS_PWR_PMIC_CTL2_PMIC_STATUS_TIMEOUT_Pos 0UL
#define SRSS_PWR_PMIC_CTL2_PMIC_STATUS_TIMEOUT_Msk 0xFFUL
#define SRSS_PWR_PMIC_CTL2_PMIC_EN_Pos          31UL
#define SRSS_PWR_PMIC_CTL2_PMIC_EN_Msk          0x80000000UL
/* SRSS.PWR_PMIC_CTL4 */
#define SRSS_PWR_PMIC_CTL4_PMIC_VADJ_DIS_Pos    30UL
#define SRSS_PWR_PMIC_CTL4_PMIC_VADJ_DIS_Msk    0x40000000UL
#define SRSS_PWR_PMIC_CTL4_PMIC_DPSLP_Pos       31UL
#define SRSS_PWR_PMIC_CTL4_PMIC_DPSLP_Msk       0x80000000UL
/* SRSS.CLK_PATH_SELECT */
#define SRSS_CLK_PATH_SELECT_PATH_MUX_Pos       0UL
#define SRSS_CLK_PATH_SELECT_PATH_MUX_Msk       0x7UL
/* SRSS.CLK_ROOT_SELECT */
#define SRSS_CLK_ROOT_SELECT_ROOT_MUX_Pos       0UL
#define SRSS_CLK_ROOT_SELECT_ROOT_MUX_Msk       0xFUL
#define SRSS_CLK_ROOT_SELECT_ROOT_DIV_Pos       4UL
#define SRSS_CLK_ROOT_SELECT_ROOT_DIV_Msk       0x30UL
#define SRSS_CLK_ROOT_SELECT_DIRECT_MUX_Pos     8UL
#define SRSS_CLK_ROOT_SELECT_DIRECT_MUX_Msk     0x100UL
#define SRSS_CLK_ROOT_SELECT_ENABLE_Pos         31UL
#define SRSS_CLK_ROOT_SELECT_ENABLE_Msk         0x80000000UL
/* SRSS.CLK_SELECT */
#define SRSS_CLK_SELECT_LFCLK_SEL_Pos           0UL
#define SRSS_CLK_SELECT_LFCLK_SEL_Msk           0x7UL
#define SRSS_CLK_SELECT_PUMP_SEL_Pos            8UL
#define SRSS_CLK_SELECT_PUMP_SEL_Msk            0xF00UL
#define SRSS_CLK_SELECT_PUMP_DIV_Pos            12UL
#define SRSS_CLK_SELECT_PUMP_DIV_Msk            0x7000UL
#define SRSS_CLK_SELECT_PUMP_ENABLE_Pos         15UL
#define SRSS_CLK_SELECT_PUMP_ENABLE_Msk         0x8000UL
/* SRSS.CLK_TIMER_CTL */
#define SRSS_CLK_TIMER_CTL_TIMER_SEL_Pos        0UL
#define SRSS_CLK_TIMER_CTL_TIMER_SEL_Msk        0x1UL
#define SRSS_CLK_TIMER_CTL_TIMER_HF0_DIV_Pos    8UL
#define SRSS_CLK_TIMER_CTL_TIMER_HF0_DIV_Msk    0x300UL
#define SRSS_CLK_TIMER_CTL_TIMER_DIV_Pos        16UL
#define SRSS_CLK_TIMER_CTL_TIMER_DIV_Msk        0xFF0000UL
#define SRSS_CLK_TIMER_CTL_ENABLE_Pos           31UL
#define SRSS_CLK_TIMER_CTL_ENABLE_Msk           0x80000000UL
/* SRSS.CLK_ILO0_CONFIG */
#define SRSS_CLK_ILO0_CONFIG_ILO0_BACKUP_Pos    0UL
#define SRSS_CLK_ILO0_CONFIG_ILO0_BACKUP_Msk    0x1UL
#define SRSS_CLK_ILO0_CONFIG_ILO0_MON_ENABLE_Pos 30UL
#define SRSS_CLK_ILO0_CONFIG_ILO0_MON_ENABLE_Msk 0x40000000UL
#define SRSS_CLK_ILO0_CONFIG_ENABLE_Pos         31UL
#define SRSS_CLK_ILO0_CONFIG_ENABLE_Msk         0x80000000UL
/* SRSS.CLK_ILO1_CONFIG */
#define SRSS_CLK_ILO1_CONFIG_ILO1_MON_ENABLE_Pos 30UL
#define SRSS_CLK_ILO1_CONFIG_ILO1_MON_ENABLE_Msk 0x40000000UL
#define SRSS_CLK_ILO1_CONFIG_ENABLE_Pos         31UL
#define SRSS_CLK_ILO1_CONFIG_ENABLE_Msk         0x80000000UL
/* SRSS.CLK_IMO_CONFIG */
#define SRSS_CLK_IMO_CONFIG_ENABLE_Pos          31UL
#define SRSS_CLK_IMO_CONFIG_ENABLE_Msk          0x80000000UL
/* SRSS.CLK_ECO_CONFIG */
#define SRSS_CLK_ECO_CONFIG_AGC_EN_Pos          1UL
#define SRSS_CLK_ECO_CONFIG_AGC_EN_Msk          0x2UL
#define SRSS_CLK_ECO_CONFIG_ECO_DIV_DISABLE_Pos 27UL
#define SRSS_CLK_ECO_CONFIG_ECO_DIV_DISABLE_Msk 0x8000000UL
#define SRSS_CLK_ECO_CONFIG_ECO_DIV_ENABLE_Pos  28UL
#define SRSS_CLK_ECO_CONFIG_ECO_DIV_ENABLE_Msk  0x10000000UL
#define SRSS_CLK_ECO_CONFIG_ECO_EN_Pos          31UL
#define SRSS_CLK_ECO_CONFIG_ECO_EN_Msk          0x80000000UL
/* SRSS.CLK_ECO_PRESCALE */
#define SRSS_CLK_ECO_PRESCALE_ECO_DIV_ENABLED_Pos 0UL
#define SRSS_CLK_ECO_PRESCALE_ECO_DIV_ENABLED_Msk 0x1UL
#define SRSS_CLK_ECO_PRESCALE_ECO_FRAC_DIV_Pos  8UL
#define SRSS_CLK_ECO_PRESCALE_ECO_FRAC_DIV_Msk  0xFF00UL
#define SRSS_CLK_ECO_PRESCALE_ECO_INT_DIV_Pos   16UL
#define SRSS_CLK_ECO_PRESCALE_ECO_INT_DIV_Msk   0x3FF0000UL
/* SRSS.CLK_ECO_STATUS */
#define SRSS_CLK_ECO_STATUS_ECO_OK_Pos          0UL
#define SRSS_CLK_ECO_STATUS_ECO_OK_Msk          0x1UL
#define SRSS_CLK_ECO_STATUS_ECO_READY_Pos       1UL
#define SRSS_CLK_ECO_STATUS_ECO_READY_Msk       0x2UL
/* SRSS.CLK_PILO_CONFIG */
#define SRSS_CLK_PILO_CONFIG_PILO_FFREQ_Pos     0UL
#define SRSS_CLK_PILO_CONFIG_PILO_FFREQ_Msk     0x3FFUL
#define SRSS_CLK_PILO_CONFIG_PILO_CLK_EN_Pos    29UL
#define SRSS_CLK_PILO_CONFIG_PILO_CLK_EN_Msk    0x20000000UL
#define SRSS_CLK_PILO_CONFIG_PILO_RESET_N_Pos   30UL
#define SRSS_CLK_PILO_CONFIG_PILO_RESET_N_Msk   0x40000000UL
#define SRSS_CLK_PILO_CONFIG_PILO_EN_Pos        31UL
#define SRSS_CLK_PILO_CONFIG_PILO_EN_Msk        0x80000000UL
/* SRSS.CLK_FLL_CONFIG */
#define SRSS_CLK_FLL_CONFIG_FLL_MULT_Pos        0UL
#define SRSS_CLK_FLL_CONFIG_FLL_MULT_Msk        0x3FFFFUL
#define SRSS_CLK_FLL_CONFIG_FLL_OUTPUT_DIV_Pos  24UL
#define SRSS_CLK_FLL_CONFIG_FLL_OUTPUT_DIV_Msk  0x1000000UL
#define SRSS_CLK_FLL_CONFIG_FLL_ENABLE_Pos      31UL
#define SRSS_CLK_FLL_CONFIG_FLL_ENABLE_Msk      0x80000000UL
/* SRSS.CLK_FLL_CONFIG2 */
#define SRSS_CLK_FLL_CONFIG2_FLL_REF_DIV_Pos    0UL
#define SRSS_CLK_FLL_CONFIG2_FLL_REF_DIV_Msk    0x1FFFUL
#define SRSS_CLK_FLL_CONFIG2_LOCK_TOL_Pos       16UL
#define SRSS_CLK_FLL_CONFIG2_LOCK_TOL_Msk       0xFF0000UL
#define SRSS_CLK_FLL_CONFIG2_UPDATE_TOL_Pos     24UL
#define SRSS_CLK_FLL_CONFIG2_UPDATE_TOL_Msk     0xFF000000UL
/* SRSS.CLK_FLL_CONFIG3 */
#define SRSS_CLK_FLL_CONFIG3_FLL_LF_IGAIN_Pos   0UL
#define SRSS_CLK_FLL_CONFIG3_FLL_LF_IGAIN_Msk   0xFUL
#define SRSS_CLK_FLL_CONFIG3_FLL_LF_PGAIN_Pos   4UL
#define SRSS_CLK_FLL_CONFIG3_FLL_LF_PGAIN_Msk   0xF0UL
#define SRSS_CLK_FLL_CONFIG3_SETTLING_COUNT_Pos 8UL
#define SRSS_CLK_FLL_CONFIG3_SETTLING_COUNT_Msk 0x1FFF00UL
#define SRSS_CLK_FLL_CONFIG3_BYPASS_SEL_Pos     28UL
#define SRSS_CLK_FLL_CONFIG3_BYPASS_SEL_Msk     0x30000000UL
/* SRSS.CLK_FLL_CONFIG4 */
#define SRSS_CLK_FLL_CONFIG4_CCO_LIMIT_Pos      0UL
#define SRSS_CLK_FLL_CONFIG4_CCO_LIMIT_Msk      0xFFUL
#define SRSS_CLK_FLL_CONFIG4_CCO_RANGE_Pos      8UL
#define SRSS_CLK_FLL_CONFIG4_CCO_RANGE_Msk      0x700UL
#define SRSS_CLK_FLL_CONFIG4_CCO_FREQ_Pos       16UL
#define SRSS_CLK_FLL_CONFIG4_CCO_FREQ_Msk       0x1FF0000UL
#define SRSS_CLK_FLL_CONFIG4_CCO_HW_UPDATE_DIS_Pos 30UL
#define SRSS_CLK_FLL_CONFIG4_CCO_HW_UPDATE_DIS_Msk 0x40000000UL
#define SRSS_CLK_FLL_CONFIG4_CCO_ENABLE_Pos     31UL
#define SRSS_CLK_FLL_CONFIG4_CCO_ENABLE_Msk     0x80000000UL
/* SRSS.CLK_FLL_STATUS */
#define SRSS_CLK_FLL_STATUS_LOCKED_Pos          0UL
#define SRSS_CLK_FLL_STATUS_LOCKED_Msk          0x1UL
#define SRSS_CLK_FLL_STATUS_UNLOCK_OCCURRED_Pos 1UL
#define SRSS_CLK_FLL_STATUS_UNLOCK_OCCURRED_Msk 0x2UL
#define SRSS_CLK_FLL_STATUS_CCO_READY_Pos       2UL
#define SRSS_CLK_FLL_STATUS_CCO_READY_Msk       0x4UL
/* SRSS.CLK_ECO_CONFIG2 */
#define SRSS_CLK_ECO_CONFIG2_WDTRIM_Pos         0UL
#define SRSS_CLK_ECO_CONFIG2_WDTRIM_Msk         0x7UL
#define SRSS_CLK_ECO_CONFIG2_ATRIM_Pos          4UL
#define SRSS_CLK_ECO_CONFIG2_ATRIM_Msk          0xF0UL
#define SRSS_CLK_ECO_CONFIG2_FTRIM_Pos          8UL
#define SRSS_CLK_ECO_CONFIG2_FTRIM_Msk          0x300UL
#define SRSS_CLK_ECO_CONFIG2_RTRIM_Pos          10UL
#define SRSS_CLK_ECO_CONFIG2_RTRIM_Msk          0xC00UL
#define SRSS_CLK_ECO_CONFIG2_GTRIM_Pos          12UL
#define SRSS_CLK_ECO_CONFIG2_GTRIM_Msk          0x7000UL
/* SRSS.CLK_PLL_CONFIG */
#define SRSS_CLK_PLL_CONFIG_FEEDBACK_DIV_Pos    0UL
#define SRSS_CLK_PLL_CONFIG_FEEDBACK_DIV_Msk    0x7FUL
#define SRSS_CLK_PLL_CONFIG_REFERENCE_DIV_Pos   8UL
#define SRSS_CLK_PLL_CONFIG_REFERENCE_DIV_Msk   0x1F00UL
#define SRSS_CLK_PLL_CONFIG_OUTPUT_DIV_Pos      16UL
#define SRSS_CLK_PLL_CONFIG_OUTPUT_DIV_Msk      0x1F0000UL
#define SRSS_CLK_PLL_CONFIG_LOCK_DELAY_Pos      25UL
#define SRSS_CLK_PLL_CONFIG_LOCK_DELAY_Msk      0x6000000UL
#define SRSS_CLK_PLL_CONFIG_PLL_LF_MODE_Pos     27UL
#define SRSS_CLK_PLL_CONFIG_PLL_LF_MODE_Msk     0x8000000UL
#define SRSS_CLK_PLL_CONFIG_BYPASS_SEL_Pos      28UL
#define SRSS_CLK_PLL_CONFIG_BYPASS_SEL_Msk      0x30000000UL
#define SRSS_CLK_PLL_CONFIG_ENABLE_Pos          31UL
#define SRSS_CLK_PLL_CONFIG_ENABLE_Msk          0x80000000UL
/* SRSS.CLK_PLL_STATUS */
#define SRSS_CLK_PLL_STATUS_LOCKED_Pos          0UL
#define SRSS_CLK_PLL_STATUS_LOCKED_Msk          0x1UL
#define SRSS_CLK_PLL_STATUS_UNLOCK_OCCURRED_Pos 1UL
#define SRSS_CLK_PLL_STATUS_UNLOCK_OCCURRED_Msk 0x2UL
/* SRSS.CSV_REF_SEL */
#define SRSS_CSV_REF_SEL_REF_MUX_Pos            0UL
#define SRSS_CSV_REF_SEL_REF_MUX_Msk            0x7UL
/* SRSS.RES_CAUSE */
#define SRSS_RES_CAUSE_RESET_WDT_Pos            0UL
#define SRSS_RES_CAUSE_RESET_WDT_Msk            0x1UL
#define SRSS_RES_CAUSE_RESET_ACT_FAULT_Pos      1UL
#define SRSS_RES_CAUSE_RESET_ACT_FAULT_Msk      0x2UL
#define SRSS_RES_CAUSE_RESET_DPSLP_FAULT_Pos    2UL
#define SRSS_RES_CAUSE_RESET_DPSLP_FAULT_Msk    0x4UL
#define SRSS_RES_CAUSE_RESET_TC_DBGRESET_Pos    3UL
#define SRSS_RES_CAUSE_RESET_TC_DBGRESET_Msk    0x8UL
#define SRSS_RES_CAUSE_RESET_SOFT_Pos           4UL
#define SRSS_RES_CAUSE_RESET_SOFT_Msk           0x10UL
#define SRSS_RES_CAUSE_RESET_MCWDT0_Pos         5UL
#define SRSS_RES_CAUSE_RESET_MCWDT0_Msk         0x20UL
#define SRSS_RES_CAUSE_RESET_MCWDT1_Pos         6UL
#define SRSS_RES_CAUSE_RESET_MCWDT1_Msk         0x40UL
#define SRSS_RES_CAUSE_RESET_MCWDT2_Pos         7UL
#define SRSS_RES_CAUSE_RESET_MCWDT2_Msk         0x80UL
#define SRSS_RES_CAUSE_RESET_MCWDT3_Pos         8UL
#define SRSS_RES_CAUSE_RESET_MCWDT3_Msk         0x100UL
#define SRSS_RES_CAUSE_RESET_XRES_Pos           16UL
#define SRSS_RES_CAUSE_RESET_XRES_Msk           0x10000UL
#define SRSS_RES_CAUSE_RESET_BODVDDD_Pos        17UL
#define SRSS_RES_CAUSE_RESET_BODVDDD_Msk        0x20000UL
#define SRSS_RES_CAUSE_RESET_BODVDDA_Pos        18UL
#define SRSS_RES_CAUSE_RESET_BODVDDA_Msk        0x40000UL
#define SRSS_RES_CAUSE_RESET_BODVCCD_Pos        19UL
#define SRSS_RES_CAUSE_RESET_BODVCCD_Msk        0x80000UL
#define SRSS_RES_CAUSE_RESET_OVDVDDD_Pos        20UL
#define SRSS_RES_CAUSE_RESET_OVDVDDD_Msk        0x100000UL
#define SRSS_RES_CAUSE_RESET_OVDVDDA_Pos        21UL
#define SRSS_RES_CAUSE_RESET_OVDVDDA_Msk        0x200000UL
#define SRSS_RES_CAUSE_RESET_OVDVCCD_Pos        22UL
#define SRSS_RES_CAUSE_RESET_OVDVCCD_Msk        0x400000UL
#define SRSS_RES_CAUSE_RESET_OCD_ACT_LINREG_Pos 23UL
#define SRSS_RES_CAUSE_RESET_OCD_ACT_LINREG_Msk 0x800000UL
#define SRSS_RES_CAUSE_RESET_OCD_DPSLP_LINREG_Pos 24UL
#define SRSS_RES_CAUSE_RESET_OCD_DPSLP_LINREG_Msk 0x1000000UL
#define SRSS_RES_CAUSE_RESET_OCD_REGHC_Pos      25UL
#define SRSS_RES_CAUSE_RESET_OCD_REGHC_Msk      0x2000000UL
#define SRSS_RES_CAUSE_RESET_PMIC_Pos           26UL
#define SRSS_RES_CAUSE_RESET_PMIC_Msk           0x4000000UL
#define SRSS_RES_CAUSE_RESET_PXRES_Pos          28UL
#define SRSS_RES_CAUSE_RESET_PXRES_Msk          0x10000000UL
#define SRSS_RES_CAUSE_RESET_STRUCT_XRES_Pos    29UL
#define SRSS_RES_CAUSE_RESET_STRUCT_XRES_Msk    0x20000000UL
#define SRSS_RES_CAUSE_RESET_PORVDDD_Pos        30UL
#define SRSS_RES_CAUSE_RESET_PORVDDD_Msk        0x40000000UL
/* SRSS.RES_CAUSE2 */
#define SRSS_RES_CAUSE2_RESET_CSV_HF_Pos        0UL
#define SRSS_RES_CAUSE2_RESET_CSV_HF_Msk        0xFFFFUL
#define SRSS_RES_CAUSE2_RESET_CSV_REF_Pos       16UL
#define SRSS_RES_CAUSE2_RESET_CSV_REF_Msk       0x10000UL
/* SRSS.CLK_TRIM_ILO0_CTL */
#define SRSS_CLK_TRIM_ILO0_CTL_ILO0_FTRIM_Pos   0UL
#define SRSS_CLK_TRIM_ILO0_CTL_ILO0_FTRIM_Msk   0x3FUL
#define SRSS_CLK_TRIM_ILO0_CTL_ILO0_MONTRIM_Pos 8UL
#define SRSS_CLK_TRIM_ILO0_CTL_ILO0_MONTRIM_Msk 0xF00UL
/* SRSS.PWR_TRIM_PWRSYS_CTL */
#define SRSS_PWR_TRIM_PWRSYS_CTL_ACT_REG_TRIM_Pos 0UL
#define SRSS_PWR_TRIM_PWRSYS_CTL_ACT_REG_TRIM_Msk 0x1FUL
#define SRSS_PWR_TRIM_PWRSYS_CTL_ACT_REG_BOOST_Pos 30UL
#define SRSS_PWR_TRIM_PWRSYS_CTL_ACT_REG_BOOST_Msk 0xC0000000UL
/* SRSS.CLK_TRIM_PILO_CTL */
#define SRSS_CLK_TRIM_PILO_CTL_PILO_CFREQ_Pos   0UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_CFREQ_Msk   0x3FUL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_OSC_TRIM_Pos 12UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_OSC_TRIM_Msk 0x7000UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_COMP_TRIM_Pos 16UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_COMP_TRIM_Msk 0x30000UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_NBIAS_TRIM_Pos 18UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_NBIAS_TRIM_Msk 0xC0000UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_RES_TRIM_Pos 20UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_RES_TRIM_Msk 0x1F00000UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_ISLOPE_TRIM_Pos 26UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_ISLOPE_TRIM_Msk 0xC000000UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_VTDIFF_TRIM_Pos 28UL
#define SRSS_CLK_TRIM_PILO_CTL_PILO_VTDIFF_TRIM_Msk 0x70000000UL
/* SRSS.CLK_TRIM_PILO_CTL2 */
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_VREF_TRIM_Pos 0UL
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_VREF_TRIM_Msk 0xFFUL
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_IREFBM_TRIM_Pos 8UL
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_IREFBM_TRIM_Msk 0x1F00UL
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_IREF_TRIM_Pos 16UL
#define SRSS_CLK_TRIM_PILO_CTL2_PILO_IREF_TRIM_Msk 0xFF0000UL
/* SRSS.CLK_TRIM_PILO_CTL3 */
#define SRSS_CLK_TRIM_PILO_CTL3_PILO_ENGOPT_Pos 0UL
#define SRSS_CLK_TRIM_PILO_CTL3_PILO_ENGOPT_Msk 0xFFFFUL
/* SRSS.CLK_TRIM_ILO1_CTL */
#define SRSS_CLK_TRIM_ILO1_CTL_ILO1_FTRIM_Pos   0UL
#define SRSS_CLK_TRIM_ILO1_CTL_ILO1_FTRIM_Msk   0x3FUL
#define SRSS_CLK_TRIM_ILO1_CTL_ILO1_MONTRIM_Pos 8UL
#define SRSS_CLK_TRIM_ILO1_CTL_ILO1_MONTRIM_Msk 0xF00UL


#endif /* _CYIP_SRSS_V3_H_ */


/* [] END OF FILE */
