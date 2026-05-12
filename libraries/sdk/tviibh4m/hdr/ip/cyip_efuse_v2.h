/***************************************************************************//**
* \file cyip_efuse_v2.h
*
* \brief
* EFUSE IP definitions
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

#ifndef _CYIP_EFUSE_V2_H_
#define _CYIP_EFUSE_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    EFUSE
*******************************************************************************/

#define EFUSE_SECTION_SIZE                      0x00000200UL

/**
  * \brief Control (EFUSE_CTL)
  */
typedef struct stc_EFUSE_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLED:1;
} stc_EFUSE_CTL_field_t;

typedef union un_EFUSE_CTL {
  uint32_t                         u32Register;
  stc_EFUSE_CTL_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_CTL_t;

/**
  * \brief Test (EFUSE_TEST)
  */
typedef struct stc_EFUSE_TEST_field {
  uint32_t                         u2MARG_READ:2;
  uint32_t                         :30;
} stc_EFUSE_TEST_field_t;

typedef union un_EFUSE_TEST {
  uint32_t                         u32Register;
  stc_EFUSE_TEST_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_TEST_t;

/**
  * \brief Command (EFUSE_CMD)
  */
typedef struct stc_EFUSE_CMD_field {
  uint32_t                         u1BIT_DATA:1;
  uint32_t                         :3;
  uint32_t                         u3BIT_ADDR:3;
  uint32_t                         :1;
  uint32_t                         u5BYTE_ADDR:5;
  uint32_t                         :3;
  uint32_t                         u4MACRO_ADDR:4;
  uint32_t                         :11;
  uint32_t                         u1START:1;
} stc_EFUSE_CMD_field_t;

typedef union un_EFUSE_CMD {
  uint32_t                         u32Register;
  stc_EFUSE_CMD_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_CMD_t;

/**
  * \brief Sequencer Default value (EFUSE_SEQ_DEFAULT)
  */
typedef struct stc_EFUSE_SEQ_DEFAULT_field {
  uint32_t                         :16;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :9;
} stc_EFUSE_SEQ_DEFAULT_field_t;

typedef union un_EFUSE_SEQ_DEFAULT {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_DEFAULT_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_DEFAULT_t;

/**
  * \brief Sequencer read control 0 (EFUSE_SEQ_READ_CTL_0)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_0_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_0_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_0 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_0_t;

/**
  * \brief Sequencer read control 1 (EFUSE_SEQ_READ_CTL_1)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_1_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_1_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_1 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_1_t;

/**
  * \brief Sequencer read control 2 (EFUSE_SEQ_READ_CTL_2)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_2_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_2_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_2 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_2_t;

/**
  * \brief Sequencer read control 3 (EFUSE_SEQ_READ_CTL_3)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_3_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_3_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_3 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_3_t;

/**
  * \brief Sequencer read control 4 (EFUSE_SEQ_READ_CTL_4)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_4_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_4_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_4 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_4_t;

/**
  * \brief Sequencer read control 5 (EFUSE_SEQ_READ_CTL_5)
  */
typedef struct stc_EFUSE_SEQ_READ_CTL_5_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_READ_CTL_5_field_t;

typedef union un_EFUSE_SEQ_READ_CTL_5 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_READ_CTL_5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_READ_CTL_5_t;

/**
  * \brief Sequencer program control 0 (EFUSE_SEQ_PROGRAM_CTL_0)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_0_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_0_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_0 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_0_t;

/**
  * \brief Sequencer program control 1 (EFUSE_SEQ_PROGRAM_CTL_1)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_1_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_1_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_1 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_1_t;

/**
  * \brief Sequencer program control 2 (EFUSE_SEQ_PROGRAM_CTL_2)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_2_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_2_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_2 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_2_t;

/**
  * \brief Sequencer program control 3 (EFUSE_SEQ_PROGRAM_CTL_3)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_3_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_3_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_3 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_3_t;

/**
  * \brief Sequencer program control 4 (EFUSE_SEQ_PROGRAM_CTL_4)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_4_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_4_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_4 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_4_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_4_t;

/**
  * \brief Sequencer program control 5 (EFUSE_SEQ_PROGRAM_CTL_5)
  */
typedef struct stc_EFUSE_SEQ_PROGRAM_CTL_5_field {
  uint32_t                         u10CYCLES:10;
  uint32_t                         :6;
  uint32_t                         u1STROBE_A:1;
  uint32_t                         u1STROBE_B:1;
  uint32_t                         u1STROBE_C:1;
  uint32_t                         u1STROBE_D:1;
  uint32_t                         u1STROBE_E:1;
  uint32_t                         u1STROBE_F:1;
  uint32_t                         u1STROBE_G:1;
  uint32_t                         :8;
  uint32_t                         u1DONE:1;
} stc_EFUSE_SEQ_PROGRAM_CTL_5_field_t;

typedef union un_EFUSE_SEQ_PROGRAM_CTL_5 {
  uint32_t                         u32Register;
  stc_EFUSE_SEQ_PROGRAM_CTL_5_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EFUSE_SEQ_PROGRAM_CTL_5_t;



/**
  * \brief EFUSE MXS40 registers (EFUSE)
  */
typedef struct stc_EFUSE {
  un_EFUSE_CTL_t                   unCTL;              /*!< 0x00000000 Control */
  un_EFUSE_TEST_t                  unTEST;             /*!< 0x00000004 Test */
  uint32_t                         au32Reserved[2];
  un_EFUSE_CMD_t                   unCMD;              /*!< 0x00000010 Command */
  uint32_t                         au32Reserved1[3];
  un_EFUSE_SEQ_DEFAULT_t           unSEQ_DEFAULT;      /*!< 0x00000020 Sequencer Default value */
  uint32_t                         au32Reserved2[7];
  un_EFUSE_SEQ_READ_CTL_0_t        unSEQ_READ_CTL_0;   /*!< 0x00000040 Sequencer read control 0 */
  un_EFUSE_SEQ_READ_CTL_1_t        unSEQ_READ_CTL_1;   /*!< 0x00000044 Sequencer read control 1 */
  un_EFUSE_SEQ_READ_CTL_2_t        unSEQ_READ_CTL_2;   /*!< 0x00000048 Sequencer read control 2 */
  un_EFUSE_SEQ_READ_CTL_3_t        unSEQ_READ_CTL_3;   /*!< 0x0000004C Sequencer read control 3 */
  un_EFUSE_SEQ_READ_CTL_4_t        unSEQ_READ_CTL_4;   /*!< 0x00000050 Sequencer read control 4 */
  un_EFUSE_SEQ_READ_CTL_5_t        unSEQ_READ_CTL_5;   /*!< 0x00000054 Sequencer read control 5 */
  uint32_t                         au32Reserved3[2];
  un_EFUSE_SEQ_PROGRAM_CTL_0_t     unSEQ_PROGRAM_CTL_0; /*!< 0x00000060 Sequencer program control 0 */
  un_EFUSE_SEQ_PROGRAM_CTL_1_t     unSEQ_PROGRAM_CTL_1; /*!< 0x00000064 Sequencer program control 1 */
  un_EFUSE_SEQ_PROGRAM_CTL_2_t     unSEQ_PROGRAM_CTL_2; /*!< 0x00000068 Sequencer program control 2 */
  un_EFUSE_SEQ_PROGRAM_CTL_3_t     unSEQ_PROGRAM_CTL_3; /*!< 0x0000006C Sequencer program control 3 */
  un_EFUSE_SEQ_PROGRAM_CTL_4_t     unSEQ_PROGRAM_CTL_4; /*!< 0x00000070 Sequencer program control 4 */
  un_EFUSE_SEQ_PROGRAM_CTL_5_t     unSEQ_PROGRAM_CTL_5; /*!< 0x00000074 Sequencer program control 5 */
} stc_EFUSE_t;                                         /*!< Size = 120 (0x78) */


/* EFUSE.CTL */
#define EFUSE_CTL_ENABLED_Pos                   31UL
#define EFUSE_CTL_ENABLED_Msk                   0x80000000UL
/* EFUSE.TEST */
#define EFUSE_TEST_MARG_READ_Pos                0UL
#define EFUSE_TEST_MARG_READ_Msk                0x3UL
/* EFUSE.CMD */
#define EFUSE_CMD_BIT_DATA_Pos                  0UL
#define EFUSE_CMD_BIT_DATA_Msk                  0x1UL
#define EFUSE_CMD_BIT_ADDR_Pos                  4UL
#define EFUSE_CMD_BIT_ADDR_Msk                  0x70UL
#define EFUSE_CMD_BYTE_ADDR_Pos                 8UL
#define EFUSE_CMD_BYTE_ADDR_Msk                 0x1F00UL
#define EFUSE_CMD_MACRO_ADDR_Pos                16UL
#define EFUSE_CMD_MACRO_ADDR_Msk                0xF0000UL
#define EFUSE_CMD_START_Pos                     31UL
#define EFUSE_CMD_START_Msk                     0x80000000UL
/* EFUSE.SEQ_DEFAULT */
#define EFUSE_SEQ_DEFAULT_STROBE_A_Pos          16UL
#define EFUSE_SEQ_DEFAULT_STROBE_A_Msk          0x10000UL
#define EFUSE_SEQ_DEFAULT_STROBE_B_Pos          17UL
#define EFUSE_SEQ_DEFAULT_STROBE_B_Msk          0x20000UL
#define EFUSE_SEQ_DEFAULT_STROBE_C_Pos          18UL
#define EFUSE_SEQ_DEFAULT_STROBE_C_Msk          0x40000UL
#define EFUSE_SEQ_DEFAULT_STROBE_D_Pos          19UL
#define EFUSE_SEQ_DEFAULT_STROBE_D_Msk          0x80000UL
#define EFUSE_SEQ_DEFAULT_STROBE_E_Pos          20UL
#define EFUSE_SEQ_DEFAULT_STROBE_E_Msk          0x100000UL
#define EFUSE_SEQ_DEFAULT_STROBE_F_Pos          21UL
#define EFUSE_SEQ_DEFAULT_STROBE_F_Msk          0x200000UL
#define EFUSE_SEQ_DEFAULT_STROBE_G_Pos          22UL
#define EFUSE_SEQ_DEFAULT_STROBE_G_Msk          0x400000UL
/* EFUSE.SEQ_READ_CTL_0 */
#define EFUSE_SEQ_READ_CTL_0_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_0_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_0_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_0_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_0_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_0_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_READ_CTL_1 */
#define EFUSE_SEQ_READ_CTL_1_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_1_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_1_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_1_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_1_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_1_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_READ_CTL_2 */
#define EFUSE_SEQ_READ_CTL_2_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_2_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_2_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_2_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_2_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_2_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_READ_CTL_3 */
#define EFUSE_SEQ_READ_CTL_3_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_3_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_3_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_3_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_3_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_3_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_READ_CTL_4 */
#define EFUSE_SEQ_READ_CTL_4_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_4_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_4_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_4_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_4_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_4_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_READ_CTL_5 */
#define EFUSE_SEQ_READ_CTL_5_CYCLES_Pos         0UL
#define EFUSE_SEQ_READ_CTL_5_CYCLES_Msk         0x3FFUL
#define EFUSE_SEQ_READ_CTL_5_STROBE_A_Pos       16UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_A_Msk       0x10000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_B_Pos       17UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_B_Msk       0x20000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_C_Pos       18UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_C_Msk       0x40000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_D_Pos       19UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_D_Msk       0x80000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_E_Pos       20UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_E_Msk       0x100000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_F_Pos       21UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_F_Msk       0x200000UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_G_Pos       22UL
#define EFUSE_SEQ_READ_CTL_5_STROBE_G_Msk       0x400000UL
#define EFUSE_SEQ_READ_CTL_5_DONE_Pos           31UL
#define EFUSE_SEQ_READ_CTL_5_DONE_Msk           0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_0 */
#define EFUSE_SEQ_PROGRAM_CTL_0_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_0_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_0_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_0_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_0_DONE_Msk        0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_1 */
#define EFUSE_SEQ_PROGRAM_CTL_1_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_1_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_1_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_1_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_1_DONE_Msk        0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_2 */
#define EFUSE_SEQ_PROGRAM_CTL_2_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_2_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_2_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_2_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_2_DONE_Msk        0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_3 */
#define EFUSE_SEQ_PROGRAM_CTL_3_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_3_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_3_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_3_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_3_DONE_Msk        0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_4 */
#define EFUSE_SEQ_PROGRAM_CTL_4_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_4_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_4_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_4_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_4_DONE_Msk        0x80000000UL
/* EFUSE.SEQ_PROGRAM_CTL_5 */
#define EFUSE_SEQ_PROGRAM_CTL_5_CYCLES_Pos      0UL
#define EFUSE_SEQ_PROGRAM_CTL_5_CYCLES_Msk      0x3FFUL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_A_Pos    16UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_A_Msk    0x10000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_B_Pos    17UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_B_Msk    0x20000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_C_Pos    18UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_C_Msk    0x40000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_D_Pos    19UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_D_Msk    0x80000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_E_Pos    20UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_E_Msk    0x100000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_F_Pos    21UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_F_Msk    0x200000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_G_Pos    22UL
#define EFUSE_SEQ_PROGRAM_CTL_5_STROBE_G_Msk    0x400000UL
#define EFUSE_SEQ_PROGRAM_CTL_5_DONE_Pos        31UL
#define EFUSE_SEQ_PROGRAM_CTL_5_DONE_Msk        0x80000000UL


#endif /* _CYIP_EFUSE_V2_H_ */


/* [] END OF FILE */
