/***************************************************************************//**
* \file cyip_pass.h
*
* \brief
* PASS IP definitions
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

#ifndef _CYIP_PASS_H_
#define _CYIP_PASS_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     PASS
*******************************************************************************/

#define PASS_SAR_CH_SECTION_SIZE                0x00000040UL
#define PASS_SAR_SECTION_SIZE                   0x00001000UL
#define PASS_EPASS_MMIO_SECTION_SIZE            0x00001000UL
#define PASS_SECTION_SIZE                       0x00100000UL

/**
  * \brief Trigger control. (PASS_SAR_CH_TR_CTL)
  */
typedef struct stc_PASS_SAR_CH_TR_CTL_field {
  uint32_t                         u3SEL:3;
  uint32_t                         :1;
  uint32_t                         u3PRIO:3;
  uint32_t                         :1;
  uint32_t                         u2PREEMPT_TYPE:2;
  uint32_t                         :1;
  uint32_t                         u1GROUP_END:1;
  uint32_t                         :19;
  uint32_t                         u1DONE_LEVEL:1;
} stc_PASS_SAR_CH_TR_CTL_field_t;

typedef union un_PASS_SAR_CH_TR_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_TR_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_TR_CTL_t;

/**
  * \brief Sample control. (PASS_SAR_CH_SAMPLE_CTL)
  */
typedef struct stc_PASS_SAR_CH_SAMPLE_CTL_field {
  uint32_t                         u6PIN_ADDR:6;
  uint32_t                         u2PORT_ADDR:2;
  uint32_t                         u3EXT_MUX_SEL:3;
  uint32_t                         u1EXT_MUX_EN:1;
  uint32_t                         u2PRECOND_MODE:2;
  uint32_t                         u2OVERLAP_DIAG:2;
  uint32_t                         u12SAMPLE_TIME:12;
  uint32_t                         :3;
  uint32_t                         u1ALT_CAL:1;
} stc_PASS_SAR_CH_SAMPLE_CTL_field_t;

typedef union un_PASS_SAR_CH_SAMPLE_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_SAMPLE_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_SAMPLE_CTL_t;

/**
  * \brief Post processing control (PASS_SAR_CH_POST_CTL)
  */
typedef struct stc_PASS_SAR_CH_POST_CTL_field {
  uint32_t                         u3POST_PROC:3;
  uint32_t                         :3;
  uint32_t                         u1LEFT_ALIGN:1;
  uint32_t                         u1SIGN_EXT:1;
  uint32_t                         u8AVG_CNT:8;
  uint32_t                         u5SHIFT_R:5;
  uint32_t                         :1;
  uint32_t                         u2RANGE_MODE:2;
  uint32_t                         :1;
  uint32_t                         u1TR_DONE_GRP_VIO:1;
  uint32_t                         :6;
} stc_PASS_SAR_CH_POST_CTL_field_t;

typedef union un_PASS_SAR_CH_POST_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_POST_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_POST_CTL_t;

/**
  * \brief Range thresholds (PASS_SAR_CH_RANGE_CTL)
  */
typedef struct stc_PASS_SAR_CH_RANGE_CTL_field {
  uint32_t                         u16RANGE_LO:16;
  uint32_t                         u16RANGE_HI:16;
} stc_PASS_SAR_CH_RANGE_CTL_field_t;

typedef union un_PASS_SAR_CH_RANGE_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_RANGE_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_RANGE_CTL_t;

/**
  * \brief Interrupt request register. (PASS_SAR_CH_INTR)
  */
typedef struct stc_PASS_SAR_CH_INTR_field {
  uint32_t                         u1GRP_DONE:1;
  uint32_t                         u1GRP_CANCELLED:1;
  uint32_t                         u1GRP_OVERFLOW:1;
  uint32_t                         :5;
  uint32_t                         u1CH_RANGE:1;
  uint32_t                         u1CH_PULSE:1;
  uint32_t                         u1CH_OVERFLOW:1;
  uint32_t                         :21;
} stc_PASS_SAR_CH_INTR_field_t;

typedef union un_PASS_SAR_CH_INTR {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_INTR_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_INTR_t;

/**
  * \brief Interrupt set request register (PASS_SAR_CH_INTR_SET)
  */
typedef struct stc_PASS_SAR_CH_INTR_SET_field {
  uint32_t                         u1GRP_DONE_SET:1;
  uint32_t                         u1GRP_CANCELLED_SET:1;
  uint32_t                         u1GRP_OVERFLOW_SET:1;
  uint32_t                         :5;
  uint32_t                         u1CH_RANGE_SET:1;
  uint32_t                         u1CH_PULSE_SET:1;
  uint32_t                         u1CH_OVERFLOW_SET:1;
  uint32_t                         :21;
} stc_PASS_SAR_CH_INTR_SET_field_t;

typedef union un_PASS_SAR_CH_INTR_SET {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_INTR_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_INTR_SET_t;

/**
  * \brief Interrupt mask register. (PASS_SAR_CH_INTR_MASK)
  */
typedef struct stc_PASS_SAR_CH_INTR_MASK_field {
  uint32_t                         u1GRP_DONE_MASK:1;
  uint32_t                         u1GRP_CANCELLED_MASK:1;
  uint32_t                         u1GRP_OVERFLOW_MASK:1;
  uint32_t                         :5;
  uint32_t                         u1CH_RANGE_MASK:1;
  uint32_t                         u1CH_PULSE_MASK:1;
  uint32_t                         u1CH_OVERFLOW_MASK:1;
  uint32_t                         :21;
} stc_PASS_SAR_CH_INTR_MASK_field_t;

typedef union un_PASS_SAR_CH_INTR_MASK {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_INTR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_INTR_MASK_t;

/**
  * \brief Interrupt masked request register (PASS_SAR_CH_INTR_MASKED)
  */
typedef struct stc_PASS_SAR_CH_INTR_MASKED_field {
  uint32_t                         u1GRP_DONE_MASKED:1;
  uint32_t                         u1GRP_CANCELLED_MASKED:1;
  uint32_t                         u1GRP_OVERFLOW_MASKED:1;
  uint32_t                         :5;
  uint32_t                         u1CH_RANGE_MASKED:1;
  uint32_t                         u1CH_PULSE_MASKED:1;
  uint32_t                         u1CH_OVERFLOW_MASKED:1;
  uint32_t                         :21;
} stc_PASS_SAR_CH_INTR_MASKED_field_t;

typedef union un_PASS_SAR_CH_INTR_MASKED {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_INTR_MASKED_t;

/**
  * \brief Working data register (PASS_SAR_CH_WORK)
  */
typedef struct stc_PASS_SAR_CH_WORK_field {
  uint32_t                         u16WORK:16;
  uint32_t                         :12;
  uint32_t                         u1ABOVE_HI_MIR:1;
  uint32_t                         u1RANGE_MIR:1;
  uint32_t                         u1PULSE_MIR:1;
  uint32_t                         u1VALID_MIR:1;
} stc_PASS_SAR_CH_WORK_field_t;

typedef union un_PASS_SAR_CH_WORK {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_WORK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_WORK_t;

/**
  * \brief Result data register (PASS_SAR_CH_RESULT)
  */
typedef struct stc_PASS_SAR_CH_RESULT_field {
  uint32_t                         u16RESULT:16;
  uint32_t                         :12;
  uint32_t                         u1ABOVE_HI_MIR:1;
  uint32_t                         u1RANGE_INTR_MIR:1;
  uint32_t                         u1PULSE_INTR_MIR:1;
  uint32_t                         u1VALID_MIR:1;
} stc_PASS_SAR_CH_RESULT_field_t;

typedef union un_PASS_SAR_CH_RESULT {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_RESULT_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_RESULT_t;

/**
  * \brief Group status register (PASS_SAR_CH_GRP_STAT)
  */
typedef struct stc_PASS_SAR_CH_GRP_STAT_field {
  uint32_t                         u1GRP_COMPLETE:1;
  uint32_t                         u1GRP_CANCELLED:1;
  uint32_t                         u1GRP_OVERFLOW:1;
  uint32_t                         :5;
  uint32_t                         u1CH_RANGE_COMPLETE:1;
  uint32_t                         u1CH_PULSE_COMPLETE:1;
  uint32_t                         u1CH_OVERFLOW:1;
  uint32_t                         :5;
  uint32_t                         u1GRP_BUSY:1;
  uint32_t                         :15;
} stc_PASS_SAR_CH_GRP_STAT_field_t;

typedef union un_PASS_SAR_CH_GRP_STAT {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_GRP_STAT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_GRP_STAT_t;

/**
  * \brief Enable register (PASS_SAR_CH_ENABLE)
  */
typedef struct stc_PASS_SAR_CH_ENABLE_field {
  uint32_t                         u1CHAN_EN:1;
  uint32_t                         :31;
} stc_PASS_SAR_CH_ENABLE_field_t;

typedef union un_PASS_SAR_CH_ENABLE {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_ENABLE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_ENABLE_t;

/**
  * \brief Software triggers (PASS_SAR_CH_TR_CMD)
  */
typedef struct stc_PASS_SAR_CH_TR_CMD_field {
  uint32_t                         u1START:1;
  uint32_t                         :31;
} stc_PASS_SAR_CH_TR_CMD_field_t;

typedef union un_PASS_SAR_CH_TR_CMD {
  uint32_t                         u32Register;
  stc_PASS_SAR_CH_TR_CMD_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CH_TR_CMD_t;



/**
  * \brief Analog control register. (PASS_SAR_CTL)
  */
typedef struct stc_PASS_SAR_CTL_field {
  uint32_t                         u8PWRUP_TIME:8;
  uint32_t                         u1IDLE_PWRDWN:1;
  uint32_t                         u1MSB_STRETCH:1;
  uint32_t                         u1HALF_LSB:1;
  uint32_t                         :18;
  uint32_t                         u1SARMUX_EN:1;
  uint32_t                         u1ADC_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_PASS_SAR_CTL_field_t;

typedef union un_PASS_SAR_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CTL_t;

/**
  * \brief Diagnostic Reference control register. (PASS_SAR_DIAG_CTL)
  */
typedef struct stc_PASS_SAR_DIAG_CTL_field {
  uint32_t                         u4DIAG_SEL:4;
  uint32_t                         :27;
  uint32_t                         u1DIAG_EN:1;
} stc_PASS_SAR_DIAG_CTL_field_t;

typedef union un_PASS_SAR_DIAG_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_DIAG_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_DIAG_CTL_t;

/**
  * \brief Preconditioning control register. (PASS_SAR_PRECOND_CTL)
  */
typedef struct stc_PASS_SAR_PRECOND_CTL_field {
  uint32_t                         u4PRECOND_TIME:4;
  uint32_t                         :28;
} stc_PASS_SAR_PRECOND_CTL_field_t;

typedef union un_PASS_SAR_PRECOND_CTL {
  uint32_t                         u32Register;
  stc_PASS_SAR_PRECOND_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_PRECOND_CTL_t;

/**
  * \brief Current analog calibration values (PASS_SAR_ANA_CAL)
  */
typedef struct stc_PASS_SAR_ANA_CAL_field {
  uint32_t                         u8AOFFSET:8;
  uint32_t                         :8;
  uint32_t                         u5AGAIN:5;
  uint32_t                         :11;
} stc_PASS_SAR_ANA_CAL_field_t;

typedef union un_PASS_SAR_ANA_CAL {
  uint32_t                         u32Register;
  stc_PASS_SAR_ANA_CAL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_ANA_CAL_t;

/**
  * \brief Current digital calibration values (PASS_SAR_DIG_CAL)
  */
typedef struct stc_PASS_SAR_DIG_CAL_field {
  uint32_t                         u12DOFFSET:12;
  uint32_t                         :4;
  uint32_t                         u6DGAIN:6;
  uint32_t                         :10;
} stc_PASS_SAR_DIG_CAL_field_t;

typedef union un_PASS_SAR_DIG_CAL {
  uint32_t                         u32Register;
  stc_PASS_SAR_DIG_CAL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_DIG_CAL_t;

/**
  * \brief Alternate analog calibration values (PASS_SAR_ANA_CAL_ALT)
  */
typedef struct stc_PASS_SAR_ANA_CAL_ALT_field {
  uint32_t                         u8AOFFSET:8;
  uint32_t                         :8;
  uint32_t                         u5AGAIN:5;
  uint32_t                         :11;
} stc_PASS_SAR_ANA_CAL_ALT_field_t;

typedef union un_PASS_SAR_ANA_CAL_ALT {
  uint32_t                         u32Register;
  stc_PASS_SAR_ANA_CAL_ALT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_ANA_CAL_ALT_t;

/**
  * \brief Alternate digital calibration values (PASS_SAR_DIG_CAL_ALT)
  */
typedef struct stc_PASS_SAR_DIG_CAL_ALT_field {
  uint32_t                         u12DOFFSET:12;
  uint32_t                         :4;
  uint32_t                         u6DGAIN:6;
  uint32_t                         :10;
} stc_PASS_SAR_DIG_CAL_ALT_field_t;

typedef union un_PASS_SAR_DIG_CAL_ALT {
  uint32_t                         u32Register;
  stc_PASS_SAR_DIG_CAL_ALT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_DIG_CAL_ALT_t;

/**
  * \brief Calibration update command (PASS_SAR_CAL_UPD_CMD)
  */
typedef struct stc_PASS_SAR_CAL_UPD_CMD_field {
  uint32_t                         u1UPDATE:1;
  uint32_t                         :31;
} stc_PASS_SAR_CAL_UPD_CMD_field_t;

typedef union un_PASS_SAR_CAL_UPD_CMD {
  uint32_t                         u32Register;
  stc_PASS_SAR_CAL_UPD_CMD_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_CAL_UPD_CMD_t;

/**
  * \brief Trigger pending status (PASS_SAR_TR_PEND)
  */
typedef struct stc_PASS_SAR_TR_PEND_field {
  uint32_t                         u32TR_PEND:32;
} stc_PASS_SAR_TR_PEND_field_t;

typedef union un_PASS_SAR_TR_PEND {
  uint32_t                         u32Register;
  stc_PASS_SAR_TR_PEND_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_TR_PEND_t;

/**
  * \brief Channel working data register 'valid' bits (PASS_SAR_WORK_VALID)
  */
typedef struct stc_PASS_SAR_WORK_VALID_field {
  uint32_t                         u32WORK_VALID:32;
} stc_PASS_SAR_WORK_VALID_field_t;

typedef union un_PASS_SAR_WORK_VALID {
  uint32_t                         u32Register;
  stc_PASS_SAR_WORK_VALID_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_WORK_VALID_t;

/**
  * \brief Range detected (PASS_SAR_WORK_RANGE)
  */
typedef struct stc_PASS_SAR_WORK_RANGE_field {
  uint32_t                         u32RANGE:32;
} stc_PASS_SAR_WORK_RANGE_field_t;

typedef union un_PASS_SAR_WORK_RANGE {
  uint32_t                         u32Register;
  stc_PASS_SAR_WORK_RANGE_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_WORK_RANGE_t;

/**
  * \brief Range detect above Hi flag (PASS_SAR_WORK_RANGE_HI)
  */
typedef struct stc_PASS_SAR_WORK_RANGE_HI_field {
  uint32_t                         u32ABOVE_HI:32;
} stc_PASS_SAR_WORK_RANGE_HI_field_t;

typedef union un_PASS_SAR_WORK_RANGE_HI {
  uint32_t                         u32Register;
  stc_PASS_SAR_WORK_RANGE_HI_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_WORK_RANGE_HI_t;

/**
  * \brief Pulse detected (PASS_SAR_WORK_PULSE)
  */
typedef struct stc_PASS_SAR_WORK_PULSE_field {
  uint32_t                         u32PULSE:32;
} stc_PASS_SAR_WORK_PULSE_field_t;

typedef union un_PASS_SAR_WORK_PULSE {
  uint32_t                         u32Register;
  stc_PASS_SAR_WORK_PULSE_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_WORK_PULSE_t;

/**
  * \brief Channel result data register 'valid' bits (PASS_SAR_RESULT_VALID)
  */
typedef struct stc_PASS_SAR_RESULT_VALID_field {
  uint32_t                         u32RESULT_VALID:32;
} stc_PASS_SAR_RESULT_VALID_field_t;

typedef union un_PASS_SAR_RESULT_VALID {
  uint32_t                         u32Register;
  stc_PASS_SAR_RESULT_VALID_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_RESULT_VALID_t;

/**
  * \brief Channel Range above Hi flags (PASS_SAR_RESULT_RANGE_HI)
  */
typedef struct stc_PASS_SAR_RESULT_RANGE_HI_field {
  uint32_t                         u32ABOVE_HI:32;
} stc_PASS_SAR_RESULT_RANGE_HI_field_t;

typedef union un_PASS_SAR_RESULT_RANGE_HI {
  uint32_t                         u32Register;
  stc_PASS_SAR_RESULT_RANGE_HI_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_RESULT_RANGE_HI_t;

/**
  * \brief Current status of internal SAR registers (mostly for debug) (PASS_SAR_STATUS)
  */
typedef struct stc_PASS_SAR_STATUS_field {
  uint32_t                         u5CUR_CHAN:5;
  uint32_t                         :3;
  uint32_t                         u3CUR_PRIO:3;
  uint32_t                         :1;
  uint32_t                         u2CUR_PREEMPT_TYPE:2;
  uint32_t                         :15;
  uint32_t                         u1DBG_FREEZE:1;
  uint32_t                         u1PWRUP_BUSY:1;
  uint32_t                         u1BUSY:1;
} stc_PASS_SAR_STATUS_field_t;

typedef union un_PASS_SAR_STATUS {
  uint32_t                         u32Register;
  stc_PASS_SAR_STATUS_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_STATUS_t;

/**
  * \brief Current averaging status (for debug) (PASS_SAR_AVG_STAT)
  */
typedef struct stc_PASS_SAR_AVG_STAT_field {
  uint32_t                         u20CUR_AVG_ACCU:20;
  uint32_t                         :4;
  uint32_t                         u8CUR_AVG_CNT:8;
} stc_PASS_SAR_AVG_STAT_field_t;

typedef union un_PASS_SAR_AVG_STAT {
  uint32_t                         u32Register;
  stc_PASS_SAR_AVG_STAT_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_AVG_STAT_t;



/**
  * \brief PASS control register (PASS_PASS_CTL)
  */
typedef struct stc_PASS_PASS_CTL_field {
  uint32_t                         u1SUPPLY_MON_EN_A:1;
  uint32_t                         u1SUPPLY_MON_LVL_A:1;
  uint32_t                         :2;
  uint32_t                         u1SUPPLY_MON_EN_B:1;
  uint32_t                         u1SUPPLY_MON_LVL_B:1;
  uint32_t                         :15;
  uint32_t                         u2REFBUF_MODE:2;
  uint32_t                         :5;
  uint32_t                         u4DBG_FREEZE_EN:4;
} stc_PASS_PASS_CTL_field_t;

typedef union un_PASS_PASS_CTL {
  uint32_t                         u32Register;
  stc_PASS_PASS_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_PASS_CTL_t;

/**
  * \brief per SAR generic input trigger select (PASS_SAR_TR_IN_SEL)
  */
typedef struct stc_PASS_SAR_TR_IN_SEL_field {
  uint32_t                         u4IN0_SEL:4;
  uint32_t                         u4IN1_SEL:4;
  uint32_t                         u4IN2_SEL:4;
  uint32_t                         u4IN3_SEL:4;
  uint32_t                         u4IN4_SEL:4;
  uint32_t                         :12;
} stc_PASS_SAR_TR_IN_SEL_field_t;

typedef union un_PASS_SAR_TR_IN_SEL {
  uint32_t                         u32Register;
  stc_PASS_SAR_TR_IN_SEL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_TR_IN_SEL_t;

/**
  * \brief per SAR generic output trigger select (PASS_SAR_TR_OUT_SEL)
  */
typedef struct stc_PASS_SAR_TR_OUT_SEL_field {
  uint32_t                         u6OUT0_SEL:6;
  uint32_t                         :2;
  uint32_t                         u6OUT1_SEL:6;
  uint32_t                         :18;
} stc_PASS_SAR_TR_OUT_SEL_field_t;

typedef union un_PASS_SAR_TR_OUT_SEL {
  uint32_t                         u32Register;
  stc_PASS_SAR_TR_OUT_SEL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PASS_SAR_TR_OUT_SEL_t;



/**
  * \brief Channel structure (PASS_SAR_CH)
  */
typedef struct stc_PASS_SAR_CH {
  un_PASS_SAR_CH_TR_CTL_t          unTR_CTL;           /*!< 0x00000000 Trigger control. */
  un_PASS_SAR_CH_SAMPLE_CTL_t      unSAMPLE_CTL;       /*!< 0x00000004 Sample control. */
  un_PASS_SAR_CH_POST_CTL_t        unPOST_CTL;         /*!< 0x00000008 Post processing control */
  un_PASS_SAR_CH_RANGE_CTL_t       unRANGE_CTL;        /*!< 0x0000000C Range thresholds */
  un_PASS_SAR_CH_INTR_t            unINTR;             /*!< 0x00000010 Interrupt request register. */
  un_PASS_SAR_CH_INTR_SET_t        unINTR_SET;         /*!< 0x00000014 Interrupt set request register */
  un_PASS_SAR_CH_INTR_MASK_t       unINTR_MASK;        /*!< 0x00000018 Interrupt mask register. */
  un_PASS_SAR_CH_INTR_MASKED_t     unINTR_MASKED;      /*!< 0x0000001C Interrupt masked request register */
  un_PASS_SAR_CH_WORK_t            unWORK;             /*!< 0x00000020 Working data register */
  un_PASS_SAR_CH_RESULT_t          unRESULT;           /*!< 0x00000024 Result data register */
  un_PASS_SAR_CH_GRP_STAT_t        unGRP_STAT;         /*!< 0x00000028 Group status register */
  uint32_t                         au32Reserved[3];
  un_PASS_SAR_CH_ENABLE_t          unENABLE;           /*!< 0x00000038 Enable register */
  un_PASS_SAR_CH_TR_CMD_t          unTR_CMD;           /*!< 0x0000003C Software triggers */
} stc_PASS_SAR_CH_t;                                   /*!< Size = 64 (0x40) */

/**
  * \brief SAR ADC with Sequencer for S40E (PASS_SAR)
  */
typedef struct stc_PASS_SAR {
  un_PASS_SAR_CTL_t                unCTL;              /*!< 0x00000000 Analog control register. */
  un_PASS_SAR_DIAG_CTL_t           unDIAG_CTL;         /*!< 0x00000004 Diagnostic Reference control register. */
  uint32_t                         au32Reserved[2];
  un_PASS_SAR_PRECOND_CTL_t        unPRECOND_CTL;      /*!< 0x00000010 Preconditioning control register. */
  uint32_t                         au32Reserved1[27];
  un_PASS_SAR_ANA_CAL_t            unANA_CAL;          /*!< 0x00000080 Current analog calibration values */
  un_PASS_SAR_DIG_CAL_t            unDIG_CAL;          /*!< 0x00000084 Current digital calibration values */
  uint32_t                         au32Reserved2[2];
  un_PASS_SAR_ANA_CAL_ALT_t        unANA_CAL_ALT;      /*!< 0x00000090 Alternate analog calibration values */
  un_PASS_SAR_DIG_CAL_ALT_t        unDIG_CAL_ALT;      /*!< 0x00000094 Alternate digital calibration values */
  un_PASS_SAR_CAL_UPD_CMD_t        unCAL_UPD_CMD;      /*!< 0x00000098 Calibration update command */
  uint32_t                         au32Reserved3[25];
  un_PASS_SAR_TR_PEND_t            unTR_PEND;          /*!< 0x00000100 Trigger pending status */
  uint32_t                         au32Reserved4[31];
  un_PASS_SAR_WORK_VALID_t         unWORK_VALID;       /*!< 0x00000180 Channel working data register 'valid' bits */
  un_PASS_SAR_WORK_RANGE_t         unWORK_RANGE;       /*!< 0x00000184 Range detected */
  un_PASS_SAR_WORK_RANGE_HI_t      unWORK_RANGE_HI;    /*!< 0x00000188 Range detect above Hi flag */
  un_PASS_SAR_WORK_PULSE_t         unWORK_PULSE;       /*!< 0x0000018C Pulse detected */
  uint32_t                         au32Reserved5[4];
  un_PASS_SAR_RESULT_VALID_t       unRESULT_VALID;     /*!< 0x000001A0 Channel result data register 'valid' bits */
  un_PASS_SAR_RESULT_RANGE_HI_t    unRESULT_RANGE_HI;  /*!< 0x000001A4 Channel Range above Hi flags */
  uint32_t                         au32Reserved6[22];
  un_PASS_SAR_STATUS_t             unSTATUS;           /*!< 0x00000200 Current status of internal SAR registers (mostly for
                                                                                debug) */
  un_PASS_SAR_AVG_STAT_t           unAVG_STAT;         /*!< 0x00000204 Current averaging status (for debug) */
  uint32_t                         au32Reserved7[382];
  stc_PASS_SAR_CH_t                CH[32];             /*!< 0x00000800 Channel structure */
} stc_PASS_SAR_t;                                      /*!< Size = 4096 (0x1000) */

/**
  * \brief PASS top-level MMIO (Generic Triggers) (PASS_EPASS_MMIO)
  */
typedef struct stc_PASS_EPASS_MMIO {
  un_PASS_PASS_CTL_t               unPASS_CTL;         /*!< 0x00000000 PASS control register */
  uint32_t                         au32Reserved[7];
  un_PASS_SAR_TR_IN_SEL_t          unSAR_TR_IN_SEL[4]; /*!< 0x00000020 per SAR generic input trigger select */
  uint32_t                         au32Reserved1[4];
  un_PASS_SAR_TR_OUT_SEL_t         unSAR_TR_OUT_SEL[4]; /*!< 0x00000040 per SAR generic output trigger select */
  uint32_t                         au32Reserved2[1004];
} stc_PASS_EPASS_MMIO_t;                               /*!< Size = 4096 (0x1000) */

/**
  * \brief Programmable Analog Subsystem for S40E (PASS)
  */
typedef struct stc_PASS {
  stc_PASS_SAR_t                   SAR[4];             /*!< 0x00000000 SAR ADC with Sequencer for S40E */
  uint32_t                         au32Reserved[241664];
  stc_PASS_EPASS_MMIO_t            EPASS_MMIO;         /*!< 0x000F0000 PASS top-level MMIO (Generic Triggers) */
} stc_PASS_t;                                          /*!< Size = 987136 (0xF1000) */


/* PASS_SAR_CH.TR_CTL */
#define PASS_SAR_CH_TR_CTL_SEL_Pos              0UL
#define PASS_SAR_CH_TR_CTL_SEL_Msk              0x7UL
#define PASS_SAR_CH_TR_CTL_PRIO_Pos             4UL
#define PASS_SAR_CH_TR_CTL_PRIO_Msk             0x70UL
#define PASS_SAR_CH_TR_CTL_PREEMPT_TYPE_Pos     8UL
#define PASS_SAR_CH_TR_CTL_PREEMPT_TYPE_Msk     0x300UL
#define PASS_SAR_CH_TR_CTL_GROUP_END_Pos        11UL
#define PASS_SAR_CH_TR_CTL_GROUP_END_Msk        0x800UL
#define PASS_SAR_CH_TR_CTL_DONE_LEVEL_Pos       31UL
#define PASS_SAR_CH_TR_CTL_DONE_LEVEL_Msk       0x80000000UL
/* PASS_SAR_CH.SAMPLE_CTL */
#define PASS_SAR_CH_SAMPLE_CTL_PIN_ADDR_Pos     0UL
#define PASS_SAR_CH_SAMPLE_CTL_PIN_ADDR_Msk     0x3FUL
#define PASS_SAR_CH_SAMPLE_CTL_PORT_ADDR_Pos    6UL
#define PASS_SAR_CH_SAMPLE_CTL_PORT_ADDR_Msk    0xC0UL
#define PASS_SAR_CH_SAMPLE_CTL_EXT_MUX_SEL_Pos  8UL
#define PASS_SAR_CH_SAMPLE_CTL_EXT_MUX_SEL_Msk  0x700UL
#define PASS_SAR_CH_SAMPLE_CTL_EXT_MUX_EN_Pos   11UL
#define PASS_SAR_CH_SAMPLE_CTL_EXT_MUX_EN_Msk   0x800UL
#define PASS_SAR_CH_SAMPLE_CTL_PRECOND_MODE_Pos 12UL
#define PASS_SAR_CH_SAMPLE_CTL_PRECOND_MODE_Msk 0x3000UL
#define PASS_SAR_CH_SAMPLE_CTL_OVERLAP_DIAG_Pos 14UL
#define PASS_SAR_CH_SAMPLE_CTL_OVERLAP_DIAG_Msk 0xC000UL
#define PASS_SAR_CH_SAMPLE_CTL_SAMPLE_TIME_Pos  16UL
#define PASS_SAR_CH_SAMPLE_CTL_SAMPLE_TIME_Msk  0xFFF0000UL
#define PASS_SAR_CH_SAMPLE_CTL_ALT_CAL_Pos      31UL
#define PASS_SAR_CH_SAMPLE_CTL_ALT_CAL_Msk      0x80000000UL
/* PASS_SAR_CH.POST_CTL */
#define PASS_SAR_CH_POST_CTL_POST_PROC_Pos      0UL
#define PASS_SAR_CH_POST_CTL_POST_PROC_Msk      0x7UL
#define PASS_SAR_CH_POST_CTL_LEFT_ALIGN_Pos     6UL
#define PASS_SAR_CH_POST_CTL_LEFT_ALIGN_Msk     0x40UL
#define PASS_SAR_CH_POST_CTL_SIGN_EXT_Pos       7UL
#define PASS_SAR_CH_POST_CTL_SIGN_EXT_Msk       0x80UL
#define PASS_SAR_CH_POST_CTL_AVG_CNT_Pos        8UL
#define PASS_SAR_CH_POST_CTL_AVG_CNT_Msk        0xFF00UL
#define PASS_SAR_CH_POST_CTL_SHIFT_R_Pos        16UL
#define PASS_SAR_CH_POST_CTL_SHIFT_R_Msk        0x1F0000UL
#define PASS_SAR_CH_POST_CTL_RANGE_MODE_Pos     22UL
#define PASS_SAR_CH_POST_CTL_RANGE_MODE_Msk     0xC00000UL
#define PASS_SAR_CH_POST_CTL_TR_DONE_GRP_VIO_Pos 25UL
#define PASS_SAR_CH_POST_CTL_TR_DONE_GRP_VIO_Msk 0x2000000UL
/* PASS_SAR_CH.RANGE_CTL */
#define PASS_SAR_CH_RANGE_CTL_RANGE_LO_Pos      0UL
#define PASS_SAR_CH_RANGE_CTL_RANGE_LO_Msk      0xFFFFUL
#define PASS_SAR_CH_RANGE_CTL_RANGE_HI_Pos      16UL
#define PASS_SAR_CH_RANGE_CTL_RANGE_HI_Msk      0xFFFF0000UL
/* PASS_SAR_CH.INTR */
#define PASS_SAR_CH_INTR_GRP_DONE_Pos           0UL
#define PASS_SAR_CH_INTR_GRP_DONE_Msk           0x1UL
#define PASS_SAR_CH_INTR_GRP_CANCELLED_Pos      1UL
#define PASS_SAR_CH_INTR_GRP_CANCELLED_Msk      0x2UL
#define PASS_SAR_CH_INTR_GRP_OVERFLOW_Pos       2UL
#define PASS_SAR_CH_INTR_GRP_OVERFLOW_Msk       0x4UL
#define PASS_SAR_CH_INTR_CH_RANGE_Pos           8UL
#define PASS_SAR_CH_INTR_CH_RANGE_Msk           0x100UL
#define PASS_SAR_CH_INTR_CH_PULSE_Pos           9UL
#define PASS_SAR_CH_INTR_CH_PULSE_Msk           0x200UL
#define PASS_SAR_CH_INTR_CH_OVERFLOW_Pos        10UL
#define PASS_SAR_CH_INTR_CH_OVERFLOW_Msk        0x400UL
/* PASS_SAR_CH.INTR_SET */
#define PASS_SAR_CH_INTR_SET_GRP_DONE_SET_Pos   0UL
#define PASS_SAR_CH_INTR_SET_GRP_DONE_SET_Msk   0x1UL
#define PASS_SAR_CH_INTR_SET_GRP_CANCELLED_SET_Pos 1UL
#define PASS_SAR_CH_INTR_SET_GRP_CANCELLED_SET_Msk 0x2UL
#define PASS_SAR_CH_INTR_SET_GRP_OVERFLOW_SET_Pos 2UL
#define PASS_SAR_CH_INTR_SET_GRP_OVERFLOW_SET_Msk 0x4UL
#define PASS_SAR_CH_INTR_SET_CH_RANGE_SET_Pos   8UL
#define PASS_SAR_CH_INTR_SET_CH_RANGE_SET_Msk   0x100UL
#define PASS_SAR_CH_INTR_SET_CH_PULSE_SET_Pos   9UL
#define PASS_SAR_CH_INTR_SET_CH_PULSE_SET_Msk   0x200UL
#define PASS_SAR_CH_INTR_SET_CH_OVERFLOW_SET_Pos 10UL
#define PASS_SAR_CH_INTR_SET_CH_OVERFLOW_SET_Msk 0x400UL
/* PASS_SAR_CH.INTR_MASK */
#define PASS_SAR_CH_INTR_MASK_GRP_DONE_MASK_Pos 0UL
#define PASS_SAR_CH_INTR_MASK_GRP_DONE_MASK_Msk 0x1UL
#define PASS_SAR_CH_INTR_MASK_GRP_CANCELLED_MASK_Pos 1UL
#define PASS_SAR_CH_INTR_MASK_GRP_CANCELLED_MASK_Msk 0x2UL
#define PASS_SAR_CH_INTR_MASK_GRP_OVERFLOW_MASK_Pos 2UL
#define PASS_SAR_CH_INTR_MASK_GRP_OVERFLOW_MASK_Msk 0x4UL
#define PASS_SAR_CH_INTR_MASK_CH_RANGE_MASK_Pos 8UL
#define PASS_SAR_CH_INTR_MASK_CH_RANGE_MASK_Msk 0x100UL
#define PASS_SAR_CH_INTR_MASK_CH_PULSE_MASK_Pos 9UL
#define PASS_SAR_CH_INTR_MASK_CH_PULSE_MASK_Msk 0x200UL
#define PASS_SAR_CH_INTR_MASK_CH_OVERFLOW_MASK_Pos 10UL
#define PASS_SAR_CH_INTR_MASK_CH_OVERFLOW_MASK_Msk 0x400UL
/* PASS_SAR_CH.INTR_MASKED */
#define PASS_SAR_CH_INTR_MASKED_GRP_DONE_MASKED_Pos 0UL
#define PASS_SAR_CH_INTR_MASKED_GRP_DONE_MASKED_Msk 0x1UL
#define PASS_SAR_CH_INTR_MASKED_GRP_CANCELLED_MASKED_Pos 1UL
#define PASS_SAR_CH_INTR_MASKED_GRP_CANCELLED_MASKED_Msk 0x2UL
#define PASS_SAR_CH_INTR_MASKED_GRP_OVERFLOW_MASKED_Pos 2UL
#define PASS_SAR_CH_INTR_MASKED_GRP_OVERFLOW_MASKED_Msk 0x4UL
#define PASS_SAR_CH_INTR_MASKED_CH_RANGE_MASKED_Pos 8UL
#define PASS_SAR_CH_INTR_MASKED_CH_RANGE_MASKED_Msk 0x100UL
#define PASS_SAR_CH_INTR_MASKED_CH_PULSE_MASKED_Pos 9UL
#define PASS_SAR_CH_INTR_MASKED_CH_PULSE_MASKED_Msk 0x200UL
#define PASS_SAR_CH_INTR_MASKED_CH_OVERFLOW_MASKED_Pos 10UL
#define PASS_SAR_CH_INTR_MASKED_CH_OVERFLOW_MASKED_Msk 0x400UL
/* PASS_SAR_CH.WORK */
#define PASS_SAR_CH_WORK_WORK_Pos               0UL
#define PASS_SAR_CH_WORK_WORK_Msk               0xFFFFUL
#define PASS_SAR_CH_WORK_ABOVE_HI_MIR_Pos       28UL
#define PASS_SAR_CH_WORK_ABOVE_HI_MIR_Msk       0x10000000UL
#define PASS_SAR_CH_WORK_RANGE_MIR_Pos          29UL
#define PASS_SAR_CH_WORK_RANGE_MIR_Msk          0x20000000UL
#define PASS_SAR_CH_WORK_PULSE_MIR_Pos          30UL
#define PASS_SAR_CH_WORK_PULSE_MIR_Msk          0x40000000UL
#define PASS_SAR_CH_WORK_VALID_MIR_Pos          31UL
#define PASS_SAR_CH_WORK_VALID_MIR_Msk          0x80000000UL
/* PASS_SAR_CH.RESULT */
#define PASS_SAR_CH_RESULT_RESULT_Pos           0UL
#define PASS_SAR_CH_RESULT_RESULT_Msk           0xFFFFUL
#define PASS_SAR_CH_RESULT_ABOVE_HI_MIR_Pos     28UL
#define PASS_SAR_CH_RESULT_ABOVE_HI_MIR_Msk     0x10000000UL
#define PASS_SAR_CH_RESULT_RANGE_INTR_MIR_Pos   29UL
#define PASS_SAR_CH_RESULT_RANGE_INTR_MIR_Msk   0x20000000UL
#define PASS_SAR_CH_RESULT_PULSE_INTR_MIR_Pos   30UL
#define PASS_SAR_CH_RESULT_PULSE_INTR_MIR_Msk   0x40000000UL
#define PASS_SAR_CH_RESULT_VALID_MIR_Pos        31UL
#define PASS_SAR_CH_RESULT_VALID_MIR_Msk        0x80000000UL
/* PASS_SAR_CH.GRP_STAT */
#define PASS_SAR_CH_GRP_STAT_GRP_COMPLETE_Pos   0UL
#define PASS_SAR_CH_GRP_STAT_GRP_COMPLETE_Msk   0x1UL
#define PASS_SAR_CH_GRP_STAT_GRP_CANCELLED_Pos  1UL
#define PASS_SAR_CH_GRP_STAT_GRP_CANCELLED_Msk  0x2UL
#define PASS_SAR_CH_GRP_STAT_GRP_OVERFLOW_Pos   2UL
#define PASS_SAR_CH_GRP_STAT_GRP_OVERFLOW_Msk   0x4UL
#define PASS_SAR_CH_GRP_STAT_CH_RANGE_COMPLETE_Pos 8UL
#define PASS_SAR_CH_GRP_STAT_CH_RANGE_COMPLETE_Msk 0x100UL
#define PASS_SAR_CH_GRP_STAT_CH_PULSE_COMPLETE_Pos 9UL
#define PASS_SAR_CH_GRP_STAT_CH_PULSE_COMPLETE_Msk 0x200UL
#define PASS_SAR_CH_GRP_STAT_CH_OVERFLOW_Pos    10UL
#define PASS_SAR_CH_GRP_STAT_CH_OVERFLOW_Msk    0x400UL
#define PASS_SAR_CH_GRP_STAT_GRP_BUSY_Pos       16UL
#define PASS_SAR_CH_GRP_STAT_GRP_BUSY_Msk       0x10000UL
/* PASS_SAR_CH.ENABLE */
#define PASS_SAR_CH_ENABLE_CHAN_EN_Pos          0UL
#define PASS_SAR_CH_ENABLE_CHAN_EN_Msk          0x1UL
/* PASS_SAR_CH.TR_CMD */
#define PASS_SAR_CH_TR_CMD_START_Pos            0UL
#define PASS_SAR_CH_TR_CMD_START_Msk            0x1UL


/* PASS_SAR.CTL */
#define PASS_SAR_CTL_PWRUP_TIME_Pos             0UL
#define PASS_SAR_CTL_PWRUP_TIME_Msk             0xFFUL
#define PASS_SAR_CTL_IDLE_PWRDWN_Pos            8UL
#define PASS_SAR_CTL_IDLE_PWRDWN_Msk            0x100UL
#define PASS_SAR_CTL_MSB_STRETCH_Pos            9UL
#define PASS_SAR_CTL_MSB_STRETCH_Msk            0x200UL
#define PASS_SAR_CTL_HALF_LSB_Pos               10UL
#define PASS_SAR_CTL_HALF_LSB_Msk               0x400UL
#define PASS_SAR_CTL_SARMUX_EN_Pos              29UL
#define PASS_SAR_CTL_SARMUX_EN_Msk              0x20000000UL
#define PASS_SAR_CTL_ADC_EN_Pos                 30UL
#define PASS_SAR_CTL_ADC_EN_Msk                 0x40000000UL
#define PASS_SAR_CTL_ENABLED_Pos                31UL
#define PASS_SAR_CTL_ENABLED_Msk                0x80000000UL
/* PASS_SAR.DIAG_CTL */
#define PASS_SAR_DIAG_CTL_DIAG_SEL_Pos          0UL
#define PASS_SAR_DIAG_CTL_DIAG_SEL_Msk          0xFUL
#define PASS_SAR_DIAG_CTL_DIAG_EN_Pos           31UL
#define PASS_SAR_DIAG_CTL_DIAG_EN_Msk           0x80000000UL
/* PASS_SAR.PRECOND_CTL */
#define PASS_SAR_PRECOND_CTL_PRECOND_TIME_Pos   0UL
#define PASS_SAR_PRECOND_CTL_PRECOND_TIME_Msk   0xFUL
/* PASS_SAR.ANA_CAL */
#define PASS_SAR_ANA_CAL_AOFFSET_Pos            0UL
#define PASS_SAR_ANA_CAL_AOFFSET_Msk            0xFFUL
#define PASS_SAR_ANA_CAL_AGAIN_Pos              16UL
#define PASS_SAR_ANA_CAL_AGAIN_Msk              0x1F0000UL
/* PASS_SAR.DIG_CAL */
#define PASS_SAR_DIG_CAL_DOFFSET_Pos            0UL
#define PASS_SAR_DIG_CAL_DOFFSET_Msk            0xFFFUL
#define PASS_SAR_DIG_CAL_DGAIN_Pos              16UL
#define PASS_SAR_DIG_CAL_DGAIN_Msk              0x3F0000UL
/* PASS_SAR.ANA_CAL_ALT */
#define PASS_SAR_ANA_CAL_ALT_AOFFSET_Pos        0UL
#define PASS_SAR_ANA_CAL_ALT_AOFFSET_Msk        0xFFUL
#define PASS_SAR_ANA_CAL_ALT_AGAIN_Pos          16UL
#define PASS_SAR_ANA_CAL_ALT_AGAIN_Msk          0x1F0000UL
/* PASS_SAR.DIG_CAL_ALT */
#define PASS_SAR_DIG_CAL_ALT_DOFFSET_Pos        0UL
#define PASS_SAR_DIG_CAL_ALT_DOFFSET_Msk        0xFFFUL
#define PASS_SAR_DIG_CAL_ALT_DGAIN_Pos          16UL
#define PASS_SAR_DIG_CAL_ALT_DGAIN_Msk          0x3F0000UL
/* PASS_SAR.CAL_UPD_CMD */
#define PASS_SAR_CAL_UPD_CMD_UPDATE_Pos         0UL
#define PASS_SAR_CAL_UPD_CMD_UPDATE_Msk         0x1UL
/* PASS_SAR.TR_PEND */
#define PASS_SAR_TR_PEND_TR_PEND_Pos            0UL
#define PASS_SAR_TR_PEND_TR_PEND_Msk            0xFFFFFFFFUL
/* PASS_SAR.WORK_VALID */
#define PASS_SAR_WORK_VALID_WORK_VALID_Pos      0UL
#define PASS_SAR_WORK_VALID_WORK_VALID_Msk      0xFFFFFFFFUL
/* PASS_SAR.WORK_RANGE */
#define PASS_SAR_WORK_RANGE_RANGE_Pos           0UL
#define PASS_SAR_WORK_RANGE_RANGE_Msk           0xFFFFFFFFUL
/* PASS_SAR.WORK_RANGE_HI */
#define PASS_SAR_WORK_RANGE_HI_ABOVE_HI_Pos     0UL
#define PASS_SAR_WORK_RANGE_HI_ABOVE_HI_Msk     0xFFFFFFFFUL
/* PASS_SAR.WORK_PULSE */
#define PASS_SAR_WORK_PULSE_PULSE_Pos           0UL
#define PASS_SAR_WORK_PULSE_PULSE_Msk           0xFFFFFFFFUL
/* PASS_SAR.RESULT_VALID */
#define PASS_SAR_RESULT_VALID_RESULT_VALID_Pos  0UL
#define PASS_SAR_RESULT_VALID_RESULT_VALID_Msk  0xFFFFFFFFUL
/* PASS_SAR.RESULT_RANGE_HI */
#define PASS_SAR_RESULT_RANGE_HI_ABOVE_HI_Pos   0UL
#define PASS_SAR_RESULT_RANGE_HI_ABOVE_HI_Msk   0xFFFFFFFFUL
/* PASS_SAR.STATUS */
#define PASS_SAR_STATUS_CUR_CHAN_Pos            0UL
#define PASS_SAR_STATUS_CUR_CHAN_Msk            0x1FUL
#define PASS_SAR_STATUS_CUR_PRIO_Pos            8UL
#define PASS_SAR_STATUS_CUR_PRIO_Msk            0x700UL
#define PASS_SAR_STATUS_CUR_PREEMPT_TYPE_Pos    12UL
#define PASS_SAR_STATUS_CUR_PREEMPT_TYPE_Msk    0x3000UL
#define PASS_SAR_STATUS_DBG_FREEZE_Pos          29UL
#define PASS_SAR_STATUS_DBG_FREEZE_Msk          0x20000000UL
#define PASS_SAR_STATUS_PWRUP_BUSY_Pos          30UL
#define PASS_SAR_STATUS_PWRUP_BUSY_Msk          0x40000000UL
#define PASS_SAR_STATUS_BUSY_Pos                31UL
#define PASS_SAR_STATUS_BUSY_Msk                0x80000000UL
/* PASS_SAR.AVG_STAT */
#define PASS_SAR_AVG_STAT_CUR_AVG_ACCU_Pos      0UL
#define PASS_SAR_AVG_STAT_CUR_AVG_ACCU_Msk      0xFFFFFUL
#define PASS_SAR_AVG_STAT_CUR_AVG_CNT_Pos       24UL
#define PASS_SAR_AVG_STAT_CUR_AVG_CNT_Msk       0xFF000000UL


/* PASS_EPASS_MMIO.PASS_CTL */
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_EN_A_Pos 0UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_EN_A_Msk 0x1UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_LVL_A_Pos 1UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_LVL_A_Msk 0x2UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_EN_B_Pos 4UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_EN_B_Msk 0x10UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_LVL_B_Pos 5UL
#define PASS_EPASS_MMIO_PASS_CTL_SUPPLY_MON_LVL_B_Msk 0x20UL
#define PASS_EPASS_MMIO_PASS_CTL_REFBUF_MODE_Pos 21UL
#define PASS_EPASS_MMIO_PASS_CTL_REFBUF_MODE_Msk 0x600000UL
#define PASS_EPASS_MMIO_PASS_CTL_DBG_FREEZE_EN_Pos 28UL
#define PASS_EPASS_MMIO_PASS_CTL_DBG_FREEZE_EN_Msk 0xF0000000UL
/* PASS_EPASS_MMIO.SAR_TR_IN_SEL */
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN0_SEL_Pos 0UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN0_SEL_Msk 0xFUL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN1_SEL_Pos 4UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN1_SEL_Msk 0xF0UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN2_SEL_Pos 8UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN2_SEL_Msk 0xF00UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN3_SEL_Pos 12UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN3_SEL_Msk 0xF000UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN4_SEL_Pos 16UL
#define PASS_EPASS_MMIO_SAR_TR_IN_SEL_IN4_SEL_Msk 0xF0000UL
/* PASS_EPASS_MMIO.SAR_TR_OUT_SEL */
#define PASS_EPASS_MMIO_SAR_TR_OUT_SEL_OUT0_SEL_Pos 0UL
#define PASS_EPASS_MMIO_SAR_TR_OUT_SEL_OUT0_SEL_Msk 0x3FUL
#define PASS_EPASS_MMIO_SAR_TR_OUT_SEL_OUT1_SEL_Pos 8UL
#define PASS_EPASS_MMIO_SAR_TR_OUT_SEL_OUT1_SEL_Msk 0x3F00UL


#endif /* _CYIP_PASS_H_ */


/* [] END OF FILE */
