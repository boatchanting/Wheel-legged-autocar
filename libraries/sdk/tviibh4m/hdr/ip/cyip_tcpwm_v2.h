/***************************************************************************//**
* \file cyip_tcpwm_v2.h
*
* \brief
* TCPWM IP definitions
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

#ifndef _CYIP_TCPWM_V2_H_
#define _CYIP_TCPWM_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    TCPWM
*******************************************************************************/

#define TCPWM_GRP_CNT_SECTION_SIZE              0x00000080UL
#define TCPWM_GRP_SECTION_SIZE                  0x00008000UL
#define TCPWM_SECTION_SIZE                      0x00020000UL

/**
  * \brief Counter control register (TCPWM_GRP_CNT_CTRL)
  */
typedef struct stc_TCPWM_GRP_CNT_CTRL_field {
  uint32_t                         u1AUTO_RELOAD_CC0:1;
  uint32_t                         u1AUTO_RELOAD_CC1:1;
  uint32_t                         u1AUTO_RELOAD_PERIOD:1;
  uint32_t                         u1AUTO_RELOAD_LINE_SEL:1;
  uint32_t                         u1CC0_MATCH_UP_EN:1;
  uint32_t                         u1CC0_MATCH_DOWN_EN:1;
  uint32_t                         u1CC1_MATCH_UP_EN:1;
  uint32_t                         u1CC1_MATCH_DOWN_EN:1;
  uint32_t                         u1PWM_IMM_KILL:1;
  uint32_t                         u1PWM_STOP_ON_KILL:1;
  uint32_t                         u1PWM_SYNC_KILL:1;
  uint32_t                         :1;
  uint32_t                         u2PWM_DISABLE_MODE:2;
  uint32_t                         :2;
  uint32_t                         u2UP_DOWN_MODE:2;
  uint32_t                         u1ONE_SHOT:1;
  uint32_t                         :1;
  uint32_t                         u2QUAD_ENCODING_MODE:2;
  uint32_t                         :2;
  uint32_t                         u3MODE:3;
  uint32_t                         :3;
  uint32_t                         u1DBG_FREEZE_EN:1;
  uint32_t                         u1ENABLED:1;
} stc_TCPWM_GRP_CNT_CTRL_field_t;

typedef union un_TCPWM_GRP_CNT_CTRL {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_CTRL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_CTRL_t;

/**
  * \brief Counter status register (TCPWM_GRP_CNT_STATUS)
  */
typedef struct stc_TCPWM_GRP_CNT_STATUS_field {
  uint32_t                         u1DOWN:1;
  uint32_t                         :3;
  uint32_t                         u1TR_CAPTURE0:1;
  uint32_t                         u1TR_COUNT:1;
  uint32_t                         u1TR_RELOAD:1;
  uint32_t                         u1TR_STOP:1;
  uint32_t                         u1TR_START:1;
  uint32_t                         u1TR_CAPTURE1:1;
  uint32_t                         u1LINE_OUT:1;
  uint32_t                         u1LINE_COMPL_OUT:1;
  uint32_t                         :3;
  uint32_t                         u1RUNNING:1;
  uint32_t                         u8DT_CNT_L:8;
  uint32_t                         u8DT_CNT_H:8;
} stc_TCPWM_GRP_CNT_STATUS_field_t;

typedef union un_TCPWM_GRP_CNT_STATUS {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_STATUS_t;

/**
  * \brief Counter count register (TCPWM_GRP_CNT_COUNTER)
  */
typedef struct stc_TCPWM_GRP_CNT_COUNTER_field {
  uint32_t                         u32COUNTER:32;
} stc_TCPWM_GRP_CNT_COUNTER_field_t;

typedef union un_TCPWM_GRP_CNT_COUNTER {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_COUNTER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_COUNTER_t;

/**
  * \brief Counter compare/capture 0 register (TCPWM_GRP_CNT_CC0)
  */
typedef struct stc_TCPWM_GRP_CNT_CC0_field {
  uint32_t                         u32CC:32;
} stc_TCPWM_GRP_CNT_CC0_field_t;

typedef union un_TCPWM_GRP_CNT_CC0 {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_CC0_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_CC0_t;

/**
  * \brief Counter buffered compare/capture 0 register (TCPWM_GRP_CNT_CC0_BUFF)
  */
typedef struct stc_TCPWM_GRP_CNT_CC0_BUFF_field {
  uint32_t                         u32CC:32;
} stc_TCPWM_GRP_CNT_CC0_BUFF_field_t;

typedef union un_TCPWM_GRP_CNT_CC0_BUFF {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_CC0_BUFF_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_CC0_BUFF_t;

/**
  * \brief Counter compare/capture 1 register (TCPWM_GRP_CNT_CC1)
  */
typedef struct stc_TCPWM_GRP_CNT_CC1_field {
  uint32_t                         u32CC:32;
} stc_TCPWM_GRP_CNT_CC1_field_t;

typedef union un_TCPWM_GRP_CNT_CC1 {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_CC1_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_CC1_t;

/**
  * \brief Counter buffered compare/capture 1 register (TCPWM_GRP_CNT_CC1_BUFF)
  */
typedef struct stc_TCPWM_GRP_CNT_CC1_BUFF_field {
  uint32_t                         u32CC:32;
} stc_TCPWM_GRP_CNT_CC1_BUFF_field_t;

typedef union un_TCPWM_GRP_CNT_CC1_BUFF {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_CC1_BUFF_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_CC1_BUFF_t;

/**
  * \brief Counter period register (TCPWM_GRP_CNT_PERIOD)
  */
typedef struct stc_TCPWM_GRP_CNT_PERIOD_field {
  uint32_t                         u32PERIOD:32;
} stc_TCPWM_GRP_CNT_PERIOD_field_t;

typedef union un_TCPWM_GRP_CNT_PERIOD {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_PERIOD_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_PERIOD_t;

/**
  * \brief Counter buffered period register (TCPWM_GRP_CNT_PERIOD_BUFF)
  */
typedef struct stc_TCPWM_GRP_CNT_PERIOD_BUFF_field {
  uint32_t                         u32PERIOD:32;
} stc_TCPWM_GRP_CNT_PERIOD_BUFF_field_t;

typedef union un_TCPWM_GRP_CNT_PERIOD_BUFF {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_PERIOD_BUFF_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_PERIOD_BUFF_t;

/**
  * \brief Counter line selection register (TCPWM_GRP_CNT_LINE_SEL)
  */
typedef struct stc_TCPWM_GRP_CNT_LINE_SEL_field {
  uint32_t                         u3OUT_SEL:3;
  uint32_t                         :1;
  uint32_t                         u3COMPL_OUT_SEL:3;
  uint32_t                         :25;
} stc_TCPWM_GRP_CNT_LINE_SEL_field_t;

typedef union un_TCPWM_GRP_CNT_LINE_SEL {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_LINE_SEL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_LINE_SEL_t;

/**
  * \brief Counter buffered line selection register (TCPWM_GRP_CNT_LINE_SEL_BUFF)
  */
typedef struct stc_TCPWM_GRP_CNT_LINE_SEL_BUFF_field {
  uint32_t                         u3OUT_SEL:3;
  uint32_t                         :1;
  uint32_t                         u3COMPL_OUT_SEL:3;
  uint32_t                         :25;
} stc_TCPWM_GRP_CNT_LINE_SEL_BUFF_field_t;

typedef union un_TCPWM_GRP_CNT_LINE_SEL_BUFF {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_LINE_SEL_BUFF_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_LINE_SEL_BUFF_t;

/**
  * \brief Counter PWM dead time register (TCPWM_GRP_CNT_DT)
  */
typedef struct stc_TCPWM_GRP_CNT_DT_field {
  uint32_t                         u8DT_LINE_OUT_L:8;
  uint32_t                         u8DT_LINE_OUT_H:8;
  uint32_t                         u16DT_LINE_COMPL_OUT:16;
} stc_TCPWM_GRP_CNT_DT_field_t;

typedef union un_TCPWM_GRP_CNT_DT {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_DT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_DT_t;

/**
  * \brief Counter trigger command register (TCPWM_GRP_CNT_TR_CMD)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_CMD_field {
  uint32_t                         u1CAPTURE0:1;
  uint32_t                         :1;
  uint32_t                         u1RELOAD:1;
  uint32_t                         u1STOP:1;
  uint32_t                         u1START:1;
  uint32_t                         u1CAPTURE1:1;
  uint32_t                         :26;
} stc_TCPWM_GRP_CNT_TR_CMD_field_t;

typedef union un_TCPWM_GRP_CNT_TR_CMD {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_CMD_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_CMD_t;

/**
  * \brief Counter input trigger selection register 0 (TCPWM_GRP_CNT_TR_IN_SEL0)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_IN_SEL0_field {
  uint32_t                         u8CAPTURE0_SEL:8;
  uint32_t                         u8COUNT_SEL:8;
  uint32_t                         u8RELOAD_SEL:8;
  uint32_t                         u8STOP_SEL:8;
} stc_TCPWM_GRP_CNT_TR_IN_SEL0_field_t;

typedef union un_TCPWM_GRP_CNT_TR_IN_SEL0 {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_IN_SEL0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_IN_SEL0_t;

/**
  * \brief Counter input trigger selection register 1 (TCPWM_GRP_CNT_TR_IN_SEL1)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_IN_SEL1_field {
  uint32_t                         u8START_SEL:8;
  uint32_t                         u8CAPTURE1_SEL:8;
  uint32_t                         :16;
} stc_TCPWM_GRP_CNT_TR_IN_SEL1_field_t;

typedef union un_TCPWM_GRP_CNT_TR_IN_SEL1 {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_IN_SEL1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_IN_SEL1_t;

/**
  * \brief Counter input trigger edge selection register (TCPWM_GRP_CNT_TR_IN_EDGE_SEL)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_field {
  uint32_t                         u2CAPTURE0_EDGE:2;
  uint32_t                         u2COUNT_EDGE:2;
  uint32_t                         u2RELOAD_EDGE:2;
  uint32_t                         u2STOP_EDGE:2;
  uint32_t                         u2START_EDGE:2;
  uint32_t                         u2CAPTURE1_EDGE:2;
  uint32_t                         :20;
} stc_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_field_t;

typedef union un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_t;

/**
  * \brief Counter trigger PWM control register (TCPWM_GRP_CNT_TR_PWM_CTRL)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_PWM_CTRL_field {
  uint32_t                         u2CC0_MATCH_MODE:2;
  uint32_t                         u2OVERFLOW_MODE:2;
  uint32_t                         u2UNDERFLOW_MODE:2;
  uint32_t                         u2CC1_MATCH_MODE:2;
  uint32_t                         :24;
} stc_TCPWM_GRP_CNT_TR_PWM_CTRL_field_t;

typedef union un_TCPWM_GRP_CNT_TR_PWM_CTRL {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_PWM_CTRL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_PWM_CTRL_t;

/**
  * \brief Counter output trigger selection register (TCPWM_GRP_CNT_TR_OUT_SEL)
  */
typedef struct stc_TCPWM_GRP_CNT_TR_OUT_SEL_field {
  uint32_t                         u3OUT0:3;
  uint32_t                         :1;
  uint32_t                         u3OUT1:3;
  uint32_t                         :25;
} stc_TCPWM_GRP_CNT_TR_OUT_SEL_field_t;

typedef union un_TCPWM_GRP_CNT_TR_OUT_SEL {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_TR_OUT_SEL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_TR_OUT_SEL_t;

/**
  * \brief Interrupt request register (TCPWM_GRP_CNT_INTR)
  */
typedef struct stc_TCPWM_GRP_CNT_INTR_field {
  uint32_t                         u1TC:1;
  uint32_t                         u1CC0_MATCH:1;
  uint32_t                         u1CC1_MATCH:1;
  uint32_t                         :29;
} stc_TCPWM_GRP_CNT_INTR_field_t;

typedef union un_TCPWM_GRP_CNT_INTR {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_INTR_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_INTR_t;

/**
  * \brief Interrupt set request register (TCPWM_GRP_CNT_INTR_SET)
  */
typedef struct stc_TCPWM_GRP_CNT_INTR_SET_field {
  uint32_t                         u1TC:1;
  uint32_t                         u1CC0_MATCH:1;
  uint32_t                         u1CC1_MATCH:1;
  uint32_t                         :29;
} stc_TCPWM_GRP_CNT_INTR_SET_field_t;

typedef union un_TCPWM_GRP_CNT_INTR_SET {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_INTR_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_INTR_SET_t;

/**
  * \brief Interrupt mask register (TCPWM_GRP_CNT_INTR_MASK)
  */
typedef struct stc_TCPWM_GRP_CNT_INTR_MASK_field {
  uint32_t                         u1TC:1;
  uint32_t                         u1CC0_MATCH:1;
  uint32_t                         u1CC1_MATCH:1;
  uint32_t                         :29;
} stc_TCPWM_GRP_CNT_INTR_MASK_field_t;

typedef union un_TCPWM_GRP_CNT_INTR_MASK {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_INTR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_INTR_MASK_t;

/**
  * \brief Interrupt masked request register (TCPWM_GRP_CNT_INTR_MASKED)
  */
typedef struct stc_TCPWM_GRP_CNT_INTR_MASKED_field {
  uint32_t                         u1TC:1;
  uint32_t                         u1CC0_MATCH:1;
  uint32_t                         u1CC1_MATCH:1;
  uint32_t                         :29;
} stc_TCPWM_GRP_CNT_INTR_MASKED_field_t;

typedef union un_TCPWM_GRP_CNT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_TCPWM_GRP_CNT_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_TCPWM_GRP_CNT_INTR_MASKED_t;



/**
  * \brief Timer/Counter/PWM Counter Module (TCPWM_GRP_CNT)
  */
typedef struct stc_TCPWM_GRP_CNT {
  un_TCPWM_GRP_CNT_CTRL_t          unCTRL;             /*!< 0x00000000 Counter control register */
  un_TCPWM_GRP_CNT_STATUS_t        unSTATUS;           /*!< 0x00000004 Counter status register */
  un_TCPWM_GRP_CNT_COUNTER_t       unCOUNTER;          /*!< 0x00000008 Counter count register */
  uint32_t                         u32Reserved;
  un_TCPWM_GRP_CNT_CC0_t           unCC0;              /*!< 0x00000010 Counter compare/capture 0 register */
  un_TCPWM_GRP_CNT_CC0_BUFF_t      unCC0_BUFF;         /*!< 0x00000014 Counter buffered compare/capture 0 register */
  un_TCPWM_GRP_CNT_CC1_t           unCC1;              /*!< 0x00000018 Counter compare/capture 1 register */
  un_TCPWM_GRP_CNT_CC1_BUFF_t      unCC1_BUFF;         /*!< 0x0000001C Counter buffered compare/capture 1 register */
  un_TCPWM_GRP_CNT_PERIOD_t        unPERIOD;           /*!< 0x00000020 Counter period register */
  un_TCPWM_GRP_CNT_PERIOD_BUFF_t   unPERIOD_BUFF;      /*!< 0x00000024 Counter buffered period register */
  un_TCPWM_GRP_CNT_LINE_SEL_t      unLINE_SEL;         /*!< 0x00000028 Counter line selection register */
  un_TCPWM_GRP_CNT_LINE_SEL_BUFF_t unLINE_SEL_BUFF;    /*!< 0x0000002C Counter buffered line selection register */
  un_TCPWM_GRP_CNT_DT_t            unDT;               /*!< 0x00000030 Counter PWM dead time register */
  uint32_t                         au32Reserved1[3];
  un_TCPWM_GRP_CNT_TR_CMD_t        unTR_CMD;           /*!< 0x00000040 Counter trigger command register */
  un_TCPWM_GRP_CNT_TR_IN_SEL0_t    unTR_IN_SEL0;       /*!< 0x00000044 Counter input trigger selection register 0 */
  un_TCPWM_GRP_CNT_TR_IN_SEL1_t    unTR_IN_SEL1;       /*!< 0x00000048 Counter input trigger selection register 1 */
  un_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_t unTR_IN_EDGE_SEL;  /*!< 0x0000004C Counter input trigger edge selection register */
  un_TCPWM_GRP_CNT_TR_PWM_CTRL_t   unTR_PWM_CTRL;      /*!< 0x00000050 Counter trigger PWM control register */
  un_TCPWM_GRP_CNT_TR_OUT_SEL_t    unTR_OUT_SEL;       /*!< 0x00000054 Counter output trigger selection register */
  uint32_t                         au32Reserved2[6];
  un_TCPWM_GRP_CNT_INTR_t          unINTR;             /*!< 0x00000070 Interrupt request register */
  un_TCPWM_GRP_CNT_INTR_SET_t      unINTR_SET;         /*!< 0x00000074 Interrupt set request register */
  un_TCPWM_GRP_CNT_INTR_MASK_t     unINTR_MASK;        /*!< 0x00000078 Interrupt mask register */
  un_TCPWM_GRP_CNT_INTR_MASKED_t   unINTR_MASKED;      /*!< 0x0000007C Interrupt masked request register */
} stc_TCPWM_GRP_CNT_t;                                 /*!< Size = 128 (0x80) */

/**
  * \brief Group of counters (TCPWM_GRP)
  */
typedef struct stc_TCPWM_GRP {
  stc_TCPWM_GRP_CNT_t              CNT[256];           /*!< 0x00000000 Timer/Counter/PWM Counter Module */
} stc_TCPWM_GRP_t;                                     /*!< Size = 32768 (0x8000) */

/**
  * \brief Timer/Counter/PWM (TCPWM)
  */
typedef struct stc_TCPWM {
  stc_TCPWM_GRP_t                  GRP[4];             /*!< 0x00000000 Group of counters */
} stc_TCPWM_t;                                         /*!< Size = 131072 (0x20000) */


/* TCPWM_GRP_CNT.CTRL */
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_CC0_Pos  0UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_CC0_Msk  0x1UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_CC1_Pos  1UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_CC1_Msk  0x2UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_PERIOD_Pos 2UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_PERIOD_Msk 0x4UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_LINE_SEL_Pos 3UL
#define TCPWM_GRP_CNT_CTRL_AUTO_RELOAD_LINE_SEL_Msk 0x8UL
#define TCPWM_GRP_CNT_CTRL_CC0_MATCH_UP_EN_Pos  4UL
#define TCPWM_GRP_CNT_CTRL_CC0_MATCH_UP_EN_Msk  0x10UL
#define TCPWM_GRP_CNT_CTRL_CC0_MATCH_DOWN_EN_Pos 5UL
#define TCPWM_GRP_CNT_CTRL_CC0_MATCH_DOWN_EN_Msk 0x20UL
#define TCPWM_GRP_CNT_CTRL_CC1_MATCH_UP_EN_Pos  6UL
#define TCPWM_GRP_CNT_CTRL_CC1_MATCH_UP_EN_Msk  0x40UL
#define TCPWM_GRP_CNT_CTRL_CC1_MATCH_DOWN_EN_Pos 7UL
#define TCPWM_GRP_CNT_CTRL_CC1_MATCH_DOWN_EN_Msk 0x80UL
#define TCPWM_GRP_CNT_CTRL_PWM_IMM_KILL_Pos     8UL
#define TCPWM_GRP_CNT_CTRL_PWM_IMM_KILL_Msk     0x100UL
#define TCPWM_GRP_CNT_CTRL_PWM_STOP_ON_KILL_Pos 9UL
#define TCPWM_GRP_CNT_CTRL_PWM_STOP_ON_KILL_Msk 0x200UL
#define TCPWM_GRP_CNT_CTRL_PWM_SYNC_KILL_Pos    10UL
#define TCPWM_GRP_CNT_CTRL_PWM_SYNC_KILL_Msk    0x400UL
#define TCPWM_GRP_CNT_CTRL_PWM_DISABLE_MODE_Pos 12UL
#define TCPWM_GRP_CNT_CTRL_PWM_DISABLE_MODE_Msk 0x3000UL
#define TCPWM_GRP_CNT_CTRL_UP_DOWN_MODE_Pos     16UL
#define TCPWM_GRP_CNT_CTRL_UP_DOWN_MODE_Msk     0x30000UL
#define TCPWM_GRP_CNT_CTRL_ONE_SHOT_Pos         18UL
#define TCPWM_GRP_CNT_CTRL_ONE_SHOT_Msk         0x40000UL
#define TCPWM_GRP_CNT_CTRL_QUAD_ENCODING_MODE_Pos 20UL
#define TCPWM_GRP_CNT_CTRL_QUAD_ENCODING_MODE_Msk 0x300000UL
#define TCPWM_GRP_CNT_CTRL_MODE_Pos             24UL
#define TCPWM_GRP_CNT_CTRL_MODE_Msk             0x7000000UL
#define TCPWM_GRP_CNT_CTRL_DBG_FREEZE_EN_Pos    30UL
#define TCPWM_GRP_CNT_CTRL_DBG_FREEZE_EN_Msk    0x40000000UL
#define TCPWM_GRP_CNT_CTRL_ENABLED_Pos          31UL
#define TCPWM_GRP_CNT_CTRL_ENABLED_Msk          0x80000000UL
/* TCPWM_GRP_CNT.STATUS */
#define TCPWM_GRP_CNT_STATUS_DOWN_Pos           0UL
#define TCPWM_GRP_CNT_STATUS_DOWN_Msk           0x1UL
#define TCPWM_GRP_CNT_STATUS_TR_CAPTURE0_Pos    4UL
#define TCPWM_GRP_CNT_STATUS_TR_CAPTURE0_Msk    0x10UL
#define TCPWM_GRP_CNT_STATUS_TR_COUNT_Pos       5UL
#define TCPWM_GRP_CNT_STATUS_TR_COUNT_Msk       0x20UL
#define TCPWM_GRP_CNT_STATUS_TR_RELOAD_Pos      6UL
#define TCPWM_GRP_CNT_STATUS_TR_RELOAD_Msk      0x40UL
#define TCPWM_GRP_CNT_STATUS_TR_STOP_Pos        7UL
#define TCPWM_GRP_CNT_STATUS_TR_STOP_Msk        0x80UL
#define TCPWM_GRP_CNT_STATUS_TR_START_Pos       8UL
#define TCPWM_GRP_CNT_STATUS_TR_START_Msk       0x100UL
#define TCPWM_GRP_CNT_STATUS_TR_CAPTURE1_Pos    9UL
#define TCPWM_GRP_CNT_STATUS_TR_CAPTURE1_Msk    0x200UL
#define TCPWM_GRP_CNT_STATUS_LINE_OUT_Pos       10UL
#define TCPWM_GRP_CNT_STATUS_LINE_OUT_Msk       0x400UL
#define TCPWM_GRP_CNT_STATUS_LINE_COMPL_OUT_Pos 11UL
#define TCPWM_GRP_CNT_STATUS_LINE_COMPL_OUT_Msk 0x800UL
#define TCPWM_GRP_CNT_STATUS_RUNNING_Pos        15UL
#define TCPWM_GRP_CNT_STATUS_RUNNING_Msk        0x8000UL
#define TCPWM_GRP_CNT_STATUS_DT_CNT_L_Pos       16UL
#define TCPWM_GRP_CNT_STATUS_DT_CNT_L_Msk       0xFF0000UL
#define TCPWM_GRP_CNT_STATUS_DT_CNT_H_Pos       24UL
#define TCPWM_GRP_CNT_STATUS_DT_CNT_H_Msk       0xFF000000UL
/* TCPWM_GRP_CNT.COUNTER */
#define TCPWM_GRP_CNT_COUNTER_COUNTER_Pos       0UL
#define TCPWM_GRP_CNT_COUNTER_COUNTER_Msk       0xFFFFFFFFUL
/* TCPWM_GRP_CNT.CC0 */
#define TCPWM_GRP_CNT_CC0_CC_Pos                0UL
#define TCPWM_GRP_CNT_CC0_CC_Msk                0xFFFFFFFFUL
/* TCPWM_GRP_CNT.CC0_BUFF */
#define TCPWM_GRP_CNT_CC0_BUFF_CC_Pos           0UL
#define TCPWM_GRP_CNT_CC0_BUFF_CC_Msk           0xFFFFFFFFUL
/* TCPWM_GRP_CNT.CC1 */
#define TCPWM_GRP_CNT_CC1_CC_Pos                0UL
#define TCPWM_GRP_CNT_CC1_CC_Msk                0xFFFFFFFFUL
/* TCPWM_GRP_CNT.CC1_BUFF */
#define TCPWM_GRP_CNT_CC1_BUFF_CC_Pos           0UL
#define TCPWM_GRP_CNT_CC1_BUFF_CC_Msk           0xFFFFFFFFUL
/* TCPWM_GRP_CNT.PERIOD */
#define TCPWM_GRP_CNT_PERIOD_PERIOD_Pos         0UL
#define TCPWM_GRP_CNT_PERIOD_PERIOD_Msk         0xFFFFFFFFUL
/* TCPWM_GRP_CNT.PERIOD_BUFF */
#define TCPWM_GRP_CNT_PERIOD_BUFF_PERIOD_Pos    0UL
#define TCPWM_GRP_CNT_PERIOD_BUFF_PERIOD_Msk    0xFFFFFFFFUL
/* TCPWM_GRP_CNT.LINE_SEL */
#define TCPWM_GRP_CNT_LINE_SEL_OUT_SEL_Pos      0UL
#define TCPWM_GRP_CNT_LINE_SEL_OUT_SEL_Msk      0x7UL
#define TCPWM_GRP_CNT_LINE_SEL_COMPL_OUT_SEL_Pos 4UL
#define TCPWM_GRP_CNT_LINE_SEL_COMPL_OUT_SEL_Msk 0x70UL
/* TCPWM_GRP_CNT.LINE_SEL_BUFF */
#define TCPWM_GRP_CNT_LINE_SEL_BUFF_OUT_SEL_Pos 0UL
#define TCPWM_GRP_CNT_LINE_SEL_BUFF_OUT_SEL_Msk 0x7UL
#define TCPWM_GRP_CNT_LINE_SEL_BUFF_COMPL_OUT_SEL_Pos 4UL
#define TCPWM_GRP_CNT_LINE_SEL_BUFF_COMPL_OUT_SEL_Msk 0x70UL
/* TCPWM_GRP_CNT.DT */
#define TCPWM_GRP_CNT_DT_DT_LINE_OUT_L_Pos      0UL
#define TCPWM_GRP_CNT_DT_DT_LINE_OUT_L_Msk      0xFFUL
#define TCPWM_GRP_CNT_DT_DT_LINE_OUT_H_Pos      8UL
#define TCPWM_GRP_CNT_DT_DT_LINE_OUT_H_Msk      0xFF00UL
#define TCPWM_GRP_CNT_DT_DT_LINE_COMPL_OUT_Pos  16UL
#define TCPWM_GRP_CNT_DT_DT_LINE_COMPL_OUT_Msk  0xFFFF0000UL
/* TCPWM_GRP_CNT.TR_CMD */
#define TCPWM_GRP_CNT_TR_CMD_CAPTURE0_Pos       0UL
#define TCPWM_GRP_CNT_TR_CMD_CAPTURE0_Msk       0x1UL
#define TCPWM_GRP_CNT_TR_CMD_RELOAD_Pos         2UL
#define TCPWM_GRP_CNT_TR_CMD_RELOAD_Msk         0x4UL
#define TCPWM_GRP_CNT_TR_CMD_STOP_Pos           3UL
#define TCPWM_GRP_CNT_TR_CMD_STOP_Msk           0x8UL
#define TCPWM_GRP_CNT_TR_CMD_START_Pos          4UL
#define TCPWM_GRP_CNT_TR_CMD_START_Msk          0x10UL
#define TCPWM_GRP_CNT_TR_CMD_CAPTURE1_Pos       5UL
#define TCPWM_GRP_CNT_TR_CMD_CAPTURE1_Msk       0x20UL
/* TCPWM_GRP_CNT.TR_IN_SEL0 */
#define TCPWM_GRP_CNT_TR_IN_SEL0_CAPTURE0_SEL_Pos 0UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_CAPTURE0_SEL_Msk 0xFFUL
#define TCPWM_GRP_CNT_TR_IN_SEL0_COUNT_SEL_Pos  8UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_COUNT_SEL_Msk  0xFF00UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_RELOAD_SEL_Pos 16UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_RELOAD_SEL_Msk 0xFF0000UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_STOP_SEL_Pos   24UL
#define TCPWM_GRP_CNT_TR_IN_SEL0_STOP_SEL_Msk   0xFF000000UL
/* TCPWM_GRP_CNT.TR_IN_SEL1 */
#define TCPWM_GRP_CNT_TR_IN_SEL1_START_SEL_Pos  0UL
#define TCPWM_GRP_CNT_TR_IN_SEL1_START_SEL_Msk  0xFFUL
#define TCPWM_GRP_CNT_TR_IN_SEL1_CAPTURE1_SEL_Pos 8UL
#define TCPWM_GRP_CNT_TR_IN_SEL1_CAPTURE1_SEL_Msk 0xFF00UL
/* TCPWM_GRP_CNT.TR_IN_EDGE_SEL */
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_CAPTURE0_EDGE_Pos 0UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_CAPTURE0_EDGE_Msk 0x3UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_COUNT_EDGE_Pos 2UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_COUNT_EDGE_Msk 0xCUL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_RELOAD_EDGE_Pos 4UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_RELOAD_EDGE_Msk 0x30UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_STOP_EDGE_Pos 6UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_STOP_EDGE_Msk 0xC0UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_START_EDGE_Pos 8UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_START_EDGE_Msk 0x300UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_CAPTURE1_EDGE_Pos 10UL
#define TCPWM_GRP_CNT_TR_IN_EDGE_SEL_CAPTURE1_EDGE_Msk 0xC00UL
/* TCPWM_GRP_CNT.TR_PWM_CTRL */
#define TCPWM_GRP_CNT_TR_PWM_CTRL_CC0_MATCH_MODE_Pos 0UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_CC0_MATCH_MODE_Msk 0x3UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_OVERFLOW_MODE_Pos 2UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_OVERFLOW_MODE_Msk 0xCUL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_UNDERFLOW_MODE_Pos 4UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_UNDERFLOW_MODE_Msk 0x30UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_CC1_MATCH_MODE_Pos 6UL
#define TCPWM_GRP_CNT_TR_PWM_CTRL_CC1_MATCH_MODE_Msk 0xC0UL
/* TCPWM_GRP_CNT.TR_OUT_SEL */
#define TCPWM_GRP_CNT_TR_OUT_SEL_OUT0_Pos       0UL
#define TCPWM_GRP_CNT_TR_OUT_SEL_OUT0_Msk       0x7UL
#define TCPWM_GRP_CNT_TR_OUT_SEL_OUT1_Pos       4UL
#define TCPWM_GRP_CNT_TR_OUT_SEL_OUT1_Msk       0x70UL
/* TCPWM_GRP_CNT.INTR */
#define TCPWM_GRP_CNT_INTR_TC_Pos               0UL
#define TCPWM_GRP_CNT_INTR_TC_Msk               0x1UL
#define TCPWM_GRP_CNT_INTR_CC0_MATCH_Pos        1UL
#define TCPWM_GRP_CNT_INTR_CC0_MATCH_Msk        0x2UL
#define TCPWM_GRP_CNT_INTR_CC1_MATCH_Pos        2UL
#define TCPWM_GRP_CNT_INTR_CC1_MATCH_Msk        0x4UL
/* TCPWM_GRP_CNT.INTR_SET */
#define TCPWM_GRP_CNT_INTR_SET_TC_Pos           0UL
#define TCPWM_GRP_CNT_INTR_SET_TC_Msk           0x1UL
#define TCPWM_GRP_CNT_INTR_SET_CC0_MATCH_Pos    1UL
#define TCPWM_GRP_CNT_INTR_SET_CC0_MATCH_Msk    0x2UL
#define TCPWM_GRP_CNT_INTR_SET_CC1_MATCH_Pos    2UL
#define TCPWM_GRP_CNT_INTR_SET_CC1_MATCH_Msk    0x4UL
/* TCPWM_GRP_CNT.INTR_MASK */
#define TCPWM_GRP_CNT_INTR_MASK_TC_Pos          0UL
#define TCPWM_GRP_CNT_INTR_MASK_TC_Msk          0x1UL
#define TCPWM_GRP_CNT_INTR_MASK_CC0_MATCH_Pos   1UL
#define TCPWM_GRP_CNT_INTR_MASK_CC0_MATCH_Msk   0x2UL
#define TCPWM_GRP_CNT_INTR_MASK_CC1_MATCH_Pos   2UL
#define TCPWM_GRP_CNT_INTR_MASK_CC1_MATCH_Msk   0x4UL
/* TCPWM_GRP_CNT.INTR_MASKED */
#define TCPWM_GRP_CNT_INTR_MASKED_TC_Pos        0UL
#define TCPWM_GRP_CNT_INTR_MASKED_TC_Msk        0x1UL
#define TCPWM_GRP_CNT_INTR_MASKED_CC0_MATCH_Pos 1UL
#define TCPWM_GRP_CNT_INTR_MASKED_CC0_MATCH_Msk 0x2UL
#define TCPWM_GRP_CNT_INTR_MASKED_CC1_MATCH_Pos 2UL
#define TCPWM_GRP_CNT_INTR_MASKED_CC1_MATCH_Msk 0x4UL


#endif /* _CYIP_TCPWM_V2_H_ */


/* [] END OF FILE */
