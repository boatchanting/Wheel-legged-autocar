/***************************************************************************//**
* \file cyip_dw.h
*
* \brief
* DW IP definitions
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

#ifndef _CYIP_DW_H_
#define _CYIP_DW_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                      DW
*******************************************************************************/

#define DW_CH_STRUCT_SECTION_SIZE               0x00000040UL
#define DW_SECTION_SIZE                         0x00010000UL

/**
  * \brief Channel control (DW_CH_STRUCT_CH_CTL)
  */
typedef struct stc_DW_CH_STRUCT_CH_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         u1B:1;
  uint32_t                         :1;
  uint32_t                         u4PC:4;
  uint32_t                         u2PRIO:2;
  uint32_t                         :1;
  uint32_t                         u1PREEMPTABLE:1;
  uint32_t                         :19;
  uint32_t                         u1ENABLED:1;
} stc_DW_CH_STRUCT_CH_CTL_field_t;

typedef union un_DW_CH_STRUCT_CH_CTL {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_CH_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_CH_CTL_t;

/**
  * \brief Channel status (DW_CH_STRUCT_CH_STATUS)
  */
typedef struct stc_DW_CH_STRUCT_CH_STATUS_field {
  uint32_t                         u4INTR_CAUSE:4;
  uint32_t                         :27;
  uint32_t                         u1PENDING:1;
} stc_DW_CH_STRUCT_CH_STATUS_field_t;

typedef union un_DW_CH_STRUCT_CH_STATUS {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_CH_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_CH_STATUS_t;

/**
  * \brief Channel current indices (DW_CH_STRUCT_CH_IDX)
  */
typedef struct stc_DW_CH_STRUCT_CH_IDX_field {
  uint32_t                         u8X_IDX:8;
  uint32_t                         u8Y_IDX:8;
  uint32_t                         :16;
} stc_DW_CH_STRUCT_CH_IDX_field_t;

typedef union un_DW_CH_STRUCT_CH_IDX {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_CH_IDX_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_CH_IDX_t;

/**
  * \brief Channel current descriptor pointer (DW_CH_STRUCT_CH_CURR_PTR)
  */
typedef struct stc_DW_CH_STRUCT_CH_CURR_PTR_field {
  uint32_t                         :2;
  uint32_t                         u30ADDR:30;
} stc_DW_CH_STRUCT_CH_CURR_PTR_field_t;

typedef union un_DW_CH_STRUCT_CH_CURR_PTR {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_CH_CURR_PTR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_CH_CURR_PTR_t;

/**
  * \brief Interrupt (DW_CH_STRUCT_INTR)
  */
typedef struct stc_DW_CH_STRUCT_INTR_field {
  uint32_t                         u1CH:1;
  uint32_t                         :31;
} stc_DW_CH_STRUCT_INTR_field_t;

typedef union un_DW_CH_STRUCT_INTR {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_INTR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_INTR_t;

/**
  * \brief Interrupt set (DW_CH_STRUCT_INTR_SET)
  */
typedef struct stc_DW_CH_STRUCT_INTR_SET_field {
  uint32_t                         u1CH:1;
  uint32_t                         :31;
} stc_DW_CH_STRUCT_INTR_SET_field_t;

typedef union un_DW_CH_STRUCT_INTR_SET {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_INTR_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_INTR_SET_t;

/**
  * \brief Interrupt mask (DW_CH_STRUCT_INTR_MASK)
  */
typedef struct stc_DW_CH_STRUCT_INTR_MASK_field {
  uint32_t                         u1CH:1;
  uint32_t                         :31;
} stc_DW_CH_STRUCT_INTR_MASK_field_t;

typedef union un_DW_CH_STRUCT_INTR_MASK {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_INTR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_INTR_MASK_t;

/**
  * \brief Interrupt masked (DW_CH_STRUCT_INTR_MASKED)
  */
typedef struct stc_DW_CH_STRUCT_INTR_MASKED_field {
  uint32_t                         u1CH:1;
  uint32_t                         :31;
} stc_DW_CH_STRUCT_INTR_MASKED_field_t;

typedef union un_DW_CH_STRUCT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_INTR_MASKED_t;

/**
  * \brief SRAM data 0 (DW_CH_STRUCT_SRAM_DATA0)
  */
typedef struct stc_DW_CH_STRUCT_SRAM_DATA0_field {
  uint32_t                         u32DATA:32;
} stc_DW_CH_STRUCT_SRAM_DATA0_field_t;

typedef union un_DW_CH_STRUCT_SRAM_DATA0 {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_SRAM_DATA0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_SRAM_DATA0_t;

/**
  * \brief SRAM data 1 (DW_CH_STRUCT_SRAM_DATA1)
  */
typedef struct stc_DW_CH_STRUCT_SRAM_DATA1_field {
  uint32_t                         u32DATA:32;
} stc_DW_CH_STRUCT_SRAM_DATA1_field_t;

typedef union un_DW_CH_STRUCT_SRAM_DATA1 {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_SRAM_DATA1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_SRAM_DATA1_t;

/**
  * \brief Channel software trigger (DW_CH_STRUCT_TR_CMD)
  */
typedef struct stc_DW_CH_STRUCT_TR_CMD_field {
  uint32_t                         u1ACTIVATE:1;
  uint32_t                         :31;
} stc_DW_CH_STRUCT_TR_CMD_field_t;

typedef union un_DW_CH_STRUCT_TR_CMD {
  uint32_t                         u32Register;
  stc_DW_CH_STRUCT_TR_CMD_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CH_STRUCT_TR_CMD_t;



/**
  * \brief Control (DW_CTL)
  */
typedef struct stc_DW_CTL_field {
  uint32_t                         u1ECC_EN:1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :29;
  uint32_t                         u1ENABLED:1;
} stc_DW_CTL_field_t;

typedef union un_DW_CTL {
  uint32_t                         u32Register;
  stc_DW_CTL_field_t               stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CTL_t;

/**
  * \brief Status (DW_STATUS)
  */
typedef struct stc_DW_STATUS_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         u1B:1;
  uint32_t                         :1;
  uint32_t                         u4PC:4;
  uint32_t                         u2PRIO:2;
  uint32_t                         :1;
  uint32_t                         u1PREEMPTABLE:1;
  uint32_t                         :4;
  uint32_t                         u9CH_IDX:9;
  uint32_t                         :3;
  uint32_t                         u3STATE:3;
  uint32_t                         u1ACTIVE:1;
} stc_DW_STATUS_field_t;

typedef union un_DW_STATUS {
  uint32_t                         u32Register;
  stc_DW_STATUS_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_STATUS_t;

/**
  * \brief Active descriptor control (DW_ACT_DESCR_CTL)
  */
typedef struct stc_DW_ACT_DESCR_CTL_field {
  uint32_t                         u32DATA:32;
} stc_DW_ACT_DESCR_CTL_field_t;

typedef union un_DW_ACT_DESCR_CTL {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_CTL_t;

/**
  * \brief Active descriptor source (DW_ACT_DESCR_SRC)
  */
typedef struct stc_DW_ACT_DESCR_SRC_field {
  uint32_t                         u32DATA:32;
} stc_DW_ACT_DESCR_SRC_field_t;

typedef union un_DW_ACT_DESCR_SRC {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_SRC_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_SRC_t;

/**
  * \brief Active descriptor destination (DW_ACT_DESCR_DST)
  */
typedef struct stc_DW_ACT_DESCR_DST_field {
  uint32_t                         u32DATA:32;
} stc_DW_ACT_DESCR_DST_field_t;

typedef union un_DW_ACT_DESCR_DST {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_DST_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_DST_t;

/**
  * \brief Active descriptor X loop control (DW_ACT_DESCR_X_CTL)
  */
typedef struct stc_DW_ACT_DESCR_X_CTL_field {
  uint32_t                         u32DATA:32;
} stc_DW_ACT_DESCR_X_CTL_field_t;

typedef union un_DW_ACT_DESCR_X_CTL {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_X_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_X_CTL_t;

/**
  * \brief Active descriptor Y loop control (DW_ACT_DESCR_Y_CTL)
  */
typedef struct stc_DW_ACT_DESCR_Y_CTL_field {
  uint32_t                         u32DATA:32;
} stc_DW_ACT_DESCR_Y_CTL_field_t;

typedef union un_DW_ACT_DESCR_Y_CTL {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_Y_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_Y_CTL_t;

/**
  * \brief Active descriptor next pointer (DW_ACT_DESCR_NEXT_PTR)
  */
typedef struct stc_DW_ACT_DESCR_NEXT_PTR_field {
  uint32_t                         :2;
  uint32_t                         u30ADDR:30;
} stc_DW_ACT_DESCR_NEXT_PTR_field_t;

typedef union un_DW_ACT_DESCR_NEXT_PTR {
  uint32_t                         u32Register;
  stc_DW_ACT_DESCR_NEXT_PTR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DESCR_NEXT_PTR_t;

/**
  * \brief Active source (DW_ACT_SRC)
  */
typedef struct stc_DW_ACT_SRC_field {
  uint32_t                         u32SRC_ADDR:32;
} stc_DW_ACT_SRC_field_t;

typedef union un_DW_ACT_SRC {
  uint32_t                         u32Register;
  stc_DW_ACT_SRC_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_SRC_t;

/**
  * \brief Active destination (DW_ACT_DST)
  */
typedef struct stc_DW_ACT_DST_field {
  uint32_t                         u32DST_ADDR:32;
} stc_DW_ACT_DST_field_t;

typedef union un_DW_ACT_DST {
  uint32_t                         u32Register;
  stc_DW_ACT_DST_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ACT_DST_t;

/**
  * \brief ECC control (DW_ECC_CTL)
  */
typedef struct stc_DW_ECC_CTL_field {
  uint32_t                         u10WORD_ADDR:10;
  uint32_t                         :15;
  uint32_t                         u7PARITY:7;
} stc_DW_ECC_CTL_field_t;

typedef union un_DW_ECC_CTL {
  uint32_t                         u32Register;
  stc_DW_ECC_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_ECC_CTL_t;

/**
  * \brief CRC control (DW_CRC_CTL)
  */
typedef struct stc_DW_CRC_CTL_field {
  uint32_t                         u1DATA_REVERSE:1;
  uint32_t                         :7;
  uint32_t                         u1REM_REVERSE:1;
  uint32_t                         :23;
} stc_DW_CRC_CTL_field_t;

typedef union un_DW_CRC_CTL {
  uint32_t                         u32Register;
  stc_DW_CRC_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_CTL_t;

/**
  * \brief CRC data control (DW_CRC_DATA_CTL)
  */
typedef struct stc_DW_CRC_DATA_CTL_field {
  uint32_t                         u8DATA_XOR:8;
  uint32_t                         :24;
} stc_DW_CRC_DATA_CTL_field_t;

typedef union un_DW_CRC_DATA_CTL {
  uint32_t                         u32Register;
  stc_DW_CRC_DATA_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_DATA_CTL_t;

/**
  * \brief CRC polynomial control (DW_CRC_POL_CTL)
  */
typedef struct stc_DW_CRC_POL_CTL_field {
  uint32_t                         u32POLYNOMIAL:32;
} stc_DW_CRC_POL_CTL_field_t;

typedef union un_DW_CRC_POL_CTL {
  uint32_t                         u32Register;
  stc_DW_CRC_POL_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_POL_CTL_t;

/**
  * \brief CRC LFSR control (DW_CRC_LFSR_CTL)
  */
typedef struct stc_DW_CRC_LFSR_CTL_field {
  uint32_t                         u32LFSR32:32;
} stc_DW_CRC_LFSR_CTL_field_t;

typedef union un_DW_CRC_LFSR_CTL {
  uint32_t                         u32Register;
  stc_DW_CRC_LFSR_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_LFSR_CTL_t;

/**
  * \brief CRC remainder control (DW_CRC_REM_CTL)
  */
typedef struct stc_DW_CRC_REM_CTL_field {
  uint32_t                         u32REM_XOR:32;
} stc_DW_CRC_REM_CTL_field_t;

typedef union un_DW_CRC_REM_CTL {
  uint32_t                         u32Register;
  stc_DW_CRC_REM_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_REM_CTL_t;

/**
  * \brief CRC remainder result (DW_CRC_REM_RESULT)
  */
typedef struct stc_DW_CRC_REM_RESULT_field {
  uint32_t                         u32REM:32;
} stc_DW_CRC_REM_RESULT_field_t;

typedef union un_DW_CRC_REM_RESULT {
  uint32_t                         u32Register;
  stc_DW_CRC_REM_RESULT_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DW_CRC_REM_RESULT_t;



/**
  * \brief DW channel structure (DW_CH_STRUCT)
  */
typedef struct stc_DW_CH_STRUCT {
  un_DW_CH_STRUCT_CH_CTL_t         unCH_CTL;           /*!< 0x00000000 Channel control */
  un_DW_CH_STRUCT_CH_STATUS_t      unCH_STATUS;        /*!< 0x00000004 Channel status */
  un_DW_CH_STRUCT_CH_IDX_t         unCH_IDX;           /*!< 0x00000008 Channel current indices */
  un_DW_CH_STRUCT_CH_CURR_PTR_t    unCH_CURR_PTR;      /*!< 0x0000000C Channel current descriptor pointer */
  un_DW_CH_STRUCT_INTR_t           unINTR;             /*!< 0x00000010 Interrupt */
  un_DW_CH_STRUCT_INTR_SET_t       unINTR_SET;         /*!< 0x00000014 Interrupt set */
  un_DW_CH_STRUCT_INTR_MASK_t      unINTR_MASK;        /*!< 0x00000018 Interrupt mask */
  un_DW_CH_STRUCT_INTR_MASKED_t    unINTR_MASKED;      /*!< 0x0000001C Interrupt masked */
  un_DW_CH_STRUCT_SRAM_DATA0_t     unSRAM_DATA0;       /*!< 0x00000020 SRAM data 0 */
  un_DW_CH_STRUCT_SRAM_DATA1_t     unSRAM_DATA1;       /*!< 0x00000024 SRAM data 1 */
  un_DW_CH_STRUCT_TR_CMD_t         unTR_CMD;           /*!< 0x00000028 Channel software trigger */
  uint32_t                         au32Reserved[5];
} stc_DW_CH_STRUCT_t;                                  /*!< Size = 64 (0x40) */

/**
  * \brief Datawire Controller (DW)
  */
typedef struct stc_DW {
  un_DW_CTL_t                      unCTL;              /*!< 0x00000000 Control */
  un_DW_STATUS_t                   unSTATUS;           /*!< 0x00000004 Status */
  uint32_t                         au32Reserved[6];
  un_DW_ACT_DESCR_CTL_t            unACT_DESCR_CTL;    /*!< 0x00000020 Active descriptor control */
  un_DW_ACT_DESCR_SRC_t            unACT_DESCR_SRC;    /*!< 0x00000024 Active descriptor source */
  un_DW_ACT_DESCR_DST_t            unACT_DESCR_DST;    /*!< 0x00000028 Active descriptor destination */
  uint32_t                         u32Reserved1;
  un_DW_ACT_DESCR_X_CTL_t          unACT_DESCR_X_CTL;  /*!< 0x00000030 Active descriptor X loop control */
  un_DW_ACT_DESCR_Y_CTL_t          unACT_DESCR_Y_CTL;  /*!< 0x00000034 Active descriptor Y loop control */
  un_DW_ACT_DESCR_NEXT_PTR_t       unACT_DESCR_NEXT_PTR; /*!< 0x00000038 Active descriptor next pointer */
  uint32_t                         u32Reserved2;
  un_DW_ACT_SRC_t                  unACT_SRC;          /*!< 0x00000040 Active source */
  un_DW_ACT_DST_t                  unACT_DST;          /*!< 0x00000044 Active destination */
  uint32_t                         au32Reserved3[14];
  un_DW_ECC_CTL_t                  unECC_CTL;          /*!< 0x00000080 ECC control */
  uint32_t                         au32Reserved4[31];
  un_DW_CRC_CTL_t                  unCRC_CTL;          /*!< 0x00000100 CRC control */
  uint32_t                         au32Reserved5[3];
  un_DW_CRC_DATA_CTL_t             unCRC_DATA_CTL;     /*!< 0x00000110 CRC data control */
  uint32_t                         au32Reserved6[3];
  un_DW_CRC_POL_CTL_t              unCRC_POL_CTL;      /*!< 0x00000120 CRC polynomial control */
  uint32_t                         au32Reserved7[3];
  un_DW_CRC_LFSR_CTL_t             unCRC_LFSR_CTL;     /*!< 0x00000130 CRC LFSR control */
  uint32_t                         au32Reserved8[3];
  un_DW_CRC_REM_CTL_t              unCRC_REM_CTL;      /*!< 0x00000140 CRC remainder control */
  uint32_t                         u32Reserved9;
  un_DW_CRC_REM_RESULT_t           unCRC_REM_RESULT;   /*!< 0x00000148 CRC remainder result */
  uint32_t                         au32Reserved10[8109];
  stc_DW_CH_STRUCT_t               CH_STRUCT[512];     /*!< 0x00008000 DW channel structure */
} stc_DW_t;                                            /*!< Size = 65536 (0x10000) */


/* DW_CH_STRUCT.CH_CTL */
#define DW_CH_STRUCT_CH_CTL_P_Pos               0UL
#define DW_CH_STRUCT_CH_CTL_P_Msk               0x1UL
#define DW_CH_STRUCT_CH_CTL_NS_Pos              1UL
#define DW_CH_STRUCT_CH_CTL_NS_Msk              0x2UL
#define DW_CH_STRUCT_CH_CTL_B_Pos               2UL
#define DW_CH_STRUCT_CH_CTL_B_Msk               0x4UL
#define DW_CH_STRUCT_CH_CTL_PC_Pos              4UL
#define DW_CH_STRUCT_CH_CTL_PC_Msk              0xF0UL
#define DW_CH_STRUCT_CH_CTL_PRIO_Pos            8UL
#define DW_CH_STRUCT_CH_CTL_PRIO_Msk            0x300UL
#define DW_CH_STRUCT_CH_CTL_PREEMPTABLE_Pos     11UL
#define DW_CH_STRUCT_CH_CTL_PREEMPTABLE_Msk     0x800UL
#define DW_CH_STRUCT_CH_CTL_ENABLED_Pos         31UL
#define DW_CH_STRUCT_CH_CTL_ENABLED_Msk         0x80000000UL
/* DW_CH_STRUCT.CH_STATUS */
#define DW_CH_STRUCT_CH_STATUS_INTR_CAUSE_Pos   0UL
#define DW_CH_STRUCT_CH_STATUS_INTR_CAUSE_Msk   0xFUL
#define DW_CH_STRUCT_CH_STATUS_PENDING_Pos      31UL
#define DW_CH_STRUCT_CH_STATUS_PENDING_Msk      0x80000000UL
/* DW_CH_STRUCT.CH_IDX */
#define DW_CH_STRUCT_CH_IDX_X_IDX_Pos           0UL
#define DW_CH_STRUCT_CH_IDX_X_IDX_Msk           0xFFUL
#define DW_CH_STRUCT_CH_IDX_Y_IDX_Pos           8UL
#define DW_CH_STRUCT_CH_IDX_Y_IDX_Msk           0xFF00UL
/* DW_CH_STRUCT.CH_CURR_PTR */
#define DW_CH_STRUCT_CH_CURR_PTR_ADDR_Pos       2UL
#define DW_CH_STRUCT_CH_CURR_PTR_ADDR_Msk       0xFFFFFFFCUL
/* DW_CH_STRUCT.INTR */
#define DW_CH_STRUCT_INTR_CH_Pos                0UL
#define DW_CH_STRUCT_INTR_CH_Msk                0x1UL
/* DW_CH_STRUCT.INTR_SET */
#define DW_CH_STRUCT_INTR_SET_CH_Pos            0UL
#define DW_CH_STRUCT_INTR_SET_CH_Msk            0x1UL
/* DW_CH_STRUCT.INTR_MASK */
#define DW_CH_STRUCT_INTR_MASK_CH_Pos           0UL
#define DW_CH_STRUCT_INTR_MASK_CH_Msk           0x1UL
/* DW_CH_STRUCT.INTR_MASKED */
#define DW_CH_STRUCT_INTR_MASKED_CH_Pos         0UL
#define DW_CH_STRUCT_INTR_MASKED_CH_Msk         0x1UL
/* DW_CH_STRUCT.SRAM_DATA0 */
#define DW_CH_STRUCT_SRAM_DATA0_DATA_Pos        0UL
#define DW_CH_STRUCT_SRAM_DATA0_DATA_Msk        0xFFFFFFFFUL
/* DW_CH_STRUCT.SRAM_DATA1 */
#define DW_CH_STRUCT_SRAM_DATA1_DATA_Pos        0UL
#define DW_CH_STRUCT_SRAM_DATA1_DATA_Msk        0xFFFFFFFFUL
/* DW_CH_STRUCT.TR_CMD */
#define DW_CH_STRUCT_TR_CMD_ACTIVATE_Pos        0UL
#define DW_CH_STRUCT_TR_CMD_ACTIVATE_Msk        0x1UL


/* DW.CTL */
#define DW_CTL_ECC_EN_Pos                       0UL
#define DW_CTL_ECC_EN_Msk                       0x1UL
#define DW_CTL_ECC_INJ_EN_Pos                   1UL
#define DW_CTL_ECC_INJ_EN_Msk                   0x2UL
#define DW_CTL_ENABLED_Pos                      31UL
#define DW_CTL_ENABLED_Msk                      0x80000000UL
/* DW.STATUS */
#define DW_STATUS_P_Pos                         0UL
#define DW_STATUS_P_Msk                         0x1UL
#define DW_STATUS_NS_Pos                        1UL
#define DW_STATUS_NS_Msk                        0x2UL
#define DW_STATUS_B_Pos                         2UL
#define DW_STATUS_B_Msk                         0x4UL
#define DW_STATUS_PC_Pos                        4UL
#define DW_STATUS_PC_Msk                        0xF0UL
#define DW_STATUS_PRIO_Pos                      8UL
#define DW_STATUS_PRIO_Msk                      0x300UL
#define DW_STATUS_PREEMPTABLE_Pos               11UL
#define DW_STATUS_PREEMPTABLE_Msk               0x800UL
#define DW_STATUS_CH_IDX_Pos                    16UL
#define DW_STATUS_CH_IDX_Msk                    0x1FF0000UL
#define DW_STATUS_STATE_Pos                     28UL
#define DW_STATUS_STATE_Msk                     0x70000000UL
#define DW_STATUS_ACTIVE_Pos                    31UL
#define DW_STATUS_ACTIVE_Msk                    0x80000000UL
/* DW.ACT_DESCR_CTL */
#define DW_ACT_DESCR_CTL_DATA_Pos               0UL
#define DW_ACT_DESCR_CTL_DATA_Msk               0xFFFFFFFFUL
/* DW.ACT_DESCR_SRC */
#define DW_ACT_DESCR_SRC_DATA_Pos               0UL
#define DW_ACT_DESCR_SRC_DATA_Msk               0xFFFFFFFFUL
/* DW.ACT_DESCR_DST */
#define DW_ACT_DESCR_DST_DATA_Pos               0UL
#define DW_ACT_DESCR_DST_DATA_Msk               0xFFFFFFFFUL
/* DW.ACT_DESCR_X_CTL */
#define DW_ACT_DESCR_X_CTL_DATA_Pos             0UL
#define DW_ACT_DESCR_X_CTL_DATA_Msk             0xFFFFFFFFUL
/* DW.ACT_DESCR_Y_CTL */
#define DW_ACT_DESCR_Y_CTL_DATA_Pos             0UL
#define DW_ACT_DESCR_Y_CTL_DATA_Msk             0xFFFFFFFFUL
/* DW.ACT_DESCR_NEXT_PTR */
#define DW_ACT_DESCR_NEXT_PTR_ADDR_Pos          2UL
#define DW_ACT_DESCR_NEXT_PTR_ADDR_Msk          0xFFFFFFFCUL
/* DW.ACT_SRC */
#define DW_ACT_SRC_SRC_ADDR_Pos                 0UL
#define DW_ACT_SRC_SRC_ADDR_Msk                 0xFFFFFFFFUL
/* DW.ACT_DST */
#define DW_ACT_DST_DST_ADDR_Pos                 0UL
#define DW_ACT_DST_DST_ADDR_Msk                 0xFFFFFFFFUL
/* DW.ECC_CTL */
#define DW_ECC_CTL_WORD_ADDR_Pos                0UL
#define DW_ECC_CTL_WORD_ADDR_Msk                0x3FFUL
#define DW_ECC_CTL_PARITY_Pos                   25UL
#define DW_ECC_CTL_PARITY_Msk                   0xFE000000UL
/* DW.CRC_CTL */
#define DW_CRC_CTL_DATA_REVERSE_Pos             0UL
#define DW_CRC_CTL_DATA_REVERSE_Msk             0x1UL
#define DW_CRC_CTL_REM_REVERSE_Pos              8UL
#define DW_CRC_CTL_REM_REVERSE_Msk              0x100UL
/* DW.CRC_DATA_CTL */
#define DW_CRC_DATA_CTL_DATA_XOR_Pos            0UL
#define DW_CRC_DATA_CTL_DATA_XOR_Msk            0xFFUL
/* DW.CRC_POL_CTL */
#define DW_CRC_POL_CTL_POLYNOMIAL_Pos           0UL
#define DW_CRC_POL_CTL_POLYNOMIAL_Msk           0xFFFFFFFFUL
/* DW.CRC_LFSR_CTL */
#define DW_CRC_LFSR_CTL_LFSR32_Pos              0UL
#define DW_CRC_LFSR_CTL_LFSR32_Msk              0xFFFFFFFFUL
/* DW.CRC_REM_CTL */
#define DW_CRC_REM_CTL_REM_XOR_Pos              0UL
#define DW_CRC_REM_CTL_REM_XOR_Msk              0xFFFFFFFFUL
/* DW.CRC_REM_RESULT */
#define DW_CRC_REM_RESULT_REM_Pos               0UL
#define DW_CRC_REM_RESULT_REM_Msk               0xFFFFFFFFUL


#endif /* _CYIP_DW_H_ */


/* [] END OF FILE */
