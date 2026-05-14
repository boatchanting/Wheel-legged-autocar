/***************************************************************************//**
* \file cyip_peri_v2.h
*
* \brief
* PERI IP definitions
*
* \note
* Generator version: 1.6.0.217
* Database revision: TVIIBE4M_WW2014_BTO
*
********************************************************************************
* \copyright
* Copyright 2016-2020, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYIP_PERI_V2_H_
#define _CYIP_PERI_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     PERI
*******************************************************************************/

#define PERI_GR_SECTION_SIZE                    0x00000020UL
#define PERI_TR_GR_SECTION_SIZE                 0x00000400UL
#define PERI_TR_1TO1_GR_SECTION_SIZE            0x00000400UL
#define PERI_SECTION_SIZE                       0x00010000UL

/**
  * \brief Clock control (PERI_GR_CLOCK_CTL)
  */
typedef struct stc_PERI_GR_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8INT8_DIV:8;
  uint32_t                         :16;
} stc_PERI_GR_CLOCK_CTL_field_t;

typedef union un_PERI_GR_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_PERI_GR_CLOCK_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_GR_CLOCK_CTL_t;

/**
  * \brief Slave control (PERI_GR_SL_CTL)
  */
typedef struct stc_PERI_GR_SL_CTL_field {
  uint32_t                         u1ENABLED_0:1;
  uint32_t                         u1ENABLED_1:1;
  uint32_t                         u1ENABLED_2:1;
  uint32_t                         u1ENABLED_3:1;
  uint32_t                         u1ENABLED_4:1;
  uint32_t                         u1ENABLED_5:1;
  uint32_t                         u1ENABLED_6:1;
  uint32_t                         u1ENABLED_7:1;
  uint32_t                         u1ENABLED_8:1;
  uint32_t                         u1ENABLED_9:1;
  uint32_t                         u1ENABLED_10:1;
  uint32_t                         u1ENABLED_11:1;
  uint32_t                         u1ENABLED_12:1;
  uint32_t                         u1ENABLED_13:1;
  uint32_t                         u1ENABLED_14:1;
  uint32_t                         u1ENABLED_15:1;
  uint32_t                         u1DISABLED_0:1;
  uint32_t                         u1DISABLED_1:1;
  uint32_t                         u1DISABLED_2:1;
  uint32_t                         u1DISABLED_3:1;
  uint32_t                         u1DISABLED_4:1;
  uint32_t                         u1DISABLED_5:1;
  uint32_t                         u1DISABLED_6:1;
  uint32_t                         u1DISABLED_7:1;
  uint32_t                         u1DISABLED_8:1;
  uint32_t                         u1DISABLED_9:1;
  uint32_t                         u1DISABLED_10:1;
  uint32_t                         u1DISABLED_11:1;
  uint32_t                         u1DISABLED_12:1;
  uint32_t                         u1DISABLED_13:1;
  uint32_t                         u1DISABLED_14:1;
  uint32_t                         u1DISABLED_15:1;
} stc_PERI_GR_SL_CTL_field_t;

typedef union un_PERI_GR_SL_CTL {
  uint32_t                         u32Register;
  stc_PERI_GR_SL_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_GR_SL_CTL_t;



/**
  * \brief Trigger control register (PERI_TR_GR_TR_CTL)
  */
typedef struct stc_PERI_TR_GR_TR_CTL_field {
  uint32_t                         u8TR_SEL:8;
  uint32_t                         u1TR_INV:1;
  uint32_t                         u1TR_EDGE:1;
  uint32_t                         :2;
  uint32_t                         u1DBG_FREEZE_EN:1;
  uint32_t                         :19;
} stc_PERI_TR_GR_TR_CTL_field_t;

typedef union un_PERI_TR_GR_TR_CTL {
  uint32_t                         u32Register;
  stc_PERI_TR_GR_TR_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_TR_GR_TR_CTL_t;



/**
  * \brief Trigger control register (PERI_TR_1TO1_GR_TR_CTL)
  */
typedef struct stc_PERI_TR_1TO1_GR_TR_CTL_field {
  uint32_t                         u1TR_SEL:1;
  uint32_t                         :7;
  uint32_t                         u1TR_INV:1;
  uint32_t                         u1TR_EDGE:1;
  uint32_t                         :2;
  uint32_t                         u1DBG_FREEZE_EN:1;
  uint32_t                         :19;
} stc_PERI_TR_1TO1_GR_TR_CTL_field_t;

typedef union un_PERI_TR_1TO1_GR_TR_CTL {
  uint32_t                         u32Register;
  stc_PERI_TR_1TO1_GR_TR_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_TR_1TO1_GR_TR_CTL_t;



/**
  * \brief Timeout control (PERI_TIMEOUT_CTL)
  */
typedef struct stc_PERI_TIMEOUT_CTL_field {
  uint32_t                         u16TIMEOUT:16;
  uint32_t                         :16;
} stc_PERI_TIMEOUT_CTL_field_t;

typedef union un_PERI_TIMEOUT_CTL {
  uint32_t                         u32Register;
  stc_PERI_TIMEOUT_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_TIMEOUT_CTL_t;

/**
  * \brief Trigger command (PERI_TR_CMD)
  */
typedef struct stc_PERI_TR_CMD_field {
  uint32_t                         u8TR_SEL:8;
  uint32_t                         u5GROUP_SEL:5;
  uint32_t                         :16;
  uint32_t                         u1TR_EDGE:1;
  uint32_t                         u1OUT_SEL:1;
  uint32_t                         u1ACTIVATE:1;
} stc_PERI_TR_CMD_field_t;

typedef union un_PERI_TR_CMD {
  uint32_t                         u32Register;
  stc_PERI_TR_CMD_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_TR_CMD_t;

/**
  * \brief Divider command (PERI_DIV_CMD)
  */
typedef struct stc_PERI_DIV_CMD_field {
  uint32_t                         u8DIV_SEL:8;
  uint32_t                         u2TYPE_SEL:2;
  uint32_t                         :6;
  uint32_t                         u8PA_DIV_SEL:8;
  uint32_t                         u2PA_TYPE_SEL:2;
  uint32_t                         :4;
  uint32_t                         u1DISABLE:1;
  uint32_t                         u1ENABLE:1;
} stc_PERI_DIV_CMD_field_t;

typedef union un_PERI_DIV_CMD {
  uint32_t                         u32Register;
  stc_PERI_DIV_CMD_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_DIV_CMD_t;

/**
  * \brief Clock control (PERI_CLOCK_CTL)
  */
typedef struct stc_PERI_CLOCK_CTL_field {
  uint32_t                         u8DIV_SEL:8;
  uint32_t                         u2TYPE_SEL:2;
  uint32_t                         :22;
} stc_PERI_CLOCK_CTL_field_t;

typedef union un_PERI_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_PERI_CLOCK_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_CLOCK_CTL_t;

/**
  * \brief Divider control (for 8.0 divider) (PERI_DIV_8_CTL)
  */
typedef struct stc_PERI_DIV_8_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :7;
  uint32_t                         u8INT8_DIV:8;
  uint32_t                         :16;
} stc_PERI_DIV_8_CTL_field_t;

typedef union un_PERI_DIV_8_CTL {
  uint32_t                         u32Register;
  stc_PERI_DIV_8_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_DIV_8_CTL_t;

/**
  * \brief Divider control (for 16.0 divider) (PERI_DIV_16_CTL)
  */
typedef struct stc_PERI_DIV_16_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :7;
  uint32_t                         u16INT16_DIV:16;
  uint32_t                         :8;
} stc_PERI_DIV_16_CTL_field_t;

typedef union un_PERI_DIV_16_CTL {
  uint32_t                         u32Register;
  stc_PERI_DIV_16_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_DIV_16_CTL_t;

/**
  * \brief Divider control (for 16.5 divider) (PERI_DIV_16_5_CTL)
  */
typedef struct stc_PERI_DIV_16_5_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :2;
  uint32_t                         u5FRAC5_DIV:5;
  uint32_t                         u16INT16_DIV:16;
  uint32_t                         :8;
} stc_PERI_DIV_16_5_CTL_field_t;

typedef union un_PERI_DIV_16_5_CTL {
  uint32_t                         u32Register;
  stc_PERI_DIV_16_5_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_DIV_16_5_CTL_t;

/**
  * \brief Divider control (for 24.5 divider) (PERI_DIV_24_5_CTL)
  */
typedef struct stc_PERI_DIV_24_5_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :2;
  uint32_t                         u5FRAC5_DIV:5;
  uint32_t                         u24INT24_DIV:24;
} stc_PERI_DIV_24_5_CTL_field_t;

typedef union un_PERI_DIV_24_5_CTL {
  uint32_t                         u32Register;
  stc_PERI_DIV_24_5_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_DIV_24_5_CTL_t;

/**
  * \brief ECC control (PERI_ECC_CTL)
  */
typedef struct stc_PERI_ECC_CTL_field {
  uint32_t                         u11WORD_ADDR:11;
  uint32_t                         :5;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         :1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :5;
  uint32_t                         u8PARITY:8;
} stc_PERI_ECC_CTL_field_t;

typedef union un_PERI_ECC_CTL {
  uint32_t                         u32Register;
  stc_PERI_ECC_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_ECC_CTL_t;



/**
  * \brief Peripheral group structure (PERI_GR)
  */
typedef struct stc_PERI_GR {
  un_PERI_GR_CLOCK_CTL_t           unCLOCK_CTL;        /*!< 0x00000000 Clock control */
  uint32_t                         au32Reserved[3];
  un_PERI_GR_SL_CTL_t              unSL_CTL;           /*!< 0x00000010 Slave control */
  uint32_t                         au32Reserved1[3];
} stc_PERI_GR_t;                                       /*!< Size = 32 (0x20) */

/**
  * \brief Trigger group (PERI_TR_GR)
  */
typedef struct stc_PERI_TR_GR {
  un_PERI_TR_GR_TR_CTL_t           unTR_CTL[256];      /*!< 0x00000000 Trigger control register */
} stc_PERI_TR_GR_t;                                    /*!< Size = 1024 (0x400) */

/**
  * \brief Trigger 1-to-1 group (PERI_TR_1TO1_GR)
  */
typedef struct stc_PERI_TR_1TO1_GR {
  un_PERI_TR_1TO1_GR_TR_CTL_t      unTR_CTL[256];      /*!< 0x00000000 Trigger control register */
} stc_PERI_TR_1TO1_GR_t;                               /*!< Size = 1024 (0x400) */

/**
  * \brief Peripheral interconnect (PERI)
  */
typedef struct stc_PERI {
  uint32_t                         au32Reserved[128];
  un_PERI_TIMEOUT_CTL_t            unTIMEOUT_CTL;      /*!< 0x00000200 Timeout control */
  uint32_t                         au32Reserved1[7];
  un_PERI_TR_CMD_t                 unTR_CMD;           /*!< 0x00000220 Trigger command */
  uint32_t                         au32Reserved2[119];
  un_PERI_DIV_CMD_t                unDIV_CMD;          /*!< 0x00000400 Divider command */
  uint32_t                         au32Reserved3[511];
  un_PERI_CLOCK_CTL_t              unCLOCK_CTL[256];   /*!< 0x00000C00 Clock control */
  un_PERI_DIV_8_CTL_t              unDIV_8_CTL[256];   /*!< 0x00001000 Divider control (for 8.0 divider) */
  un_PERI_DIV_16_CTL_t             unDIV_16_CTL[256];  /*!< 0x00001400 Divider control (for 16.0 divider) */
  un_PERI_DIV_16_5_CTL_t           unDIV_16_5_CTL[256]; /*!< 0x00001800 Divider control (for 16.5 divider) */
  un_PERI_DIV_24_5_CTL_t           unDIV_24_5_CTL[255]; /*!< 0x00001C00 Divider control (for 24.5 divider) */
  uint32_t                         u32Reserved4;
  un_PERI_ECC_CTL_t                unECC_CTL;          /*!< 0x00002000 ECC control */
  uint32_t                         au32Reserved5[2047];
  stc_PERI_GR_t                    GR[16];             /*!< 0x00004000 Peripheral group structure */
  uint32_t                         au32Reserved6[3968];
  stc_PERI_TR_GR_t                 TR_GR[16];          /*!< 0x00008000 Trigger group */
  stc_PERI_TR_1TO1_GR_t            TR_1TO1_GR[16];     /*!< 0x0000C000 Trigger 1-to-1 group */
} stc_PERI_t;                                          /*!< Size = 65536 (0x10000) */


/* PERI_GR.CLOCK_CTL */
#define PERI_GR_CLOCK_CTL_INT8_DIV_Pos          8UL
#define PERI_GR_CLOCK_CTL_INT8_DIV_Msk          0xFF00UL
/* PERI_GR.SL_CTL */
#define PERI_GR_SL_CTL_ENABLED_0_Pos            0UL
#define PERI_GR_SL_CTL_ENABLED_0_Msk            0x1UL
#define PERI_GR_SL_CTL_ENABLED_1_Pos            1UL
#define PERI_GR_SL_CTL_ENABLED_1_Msk            0x2UL
#define PERI_GR_SL_CTL_ENABLED_2_Pos            2UL
#define PERI_GR_SL_CTL_ENABLED_2_Msk            0x4UL
#define PERI_GR_SL_CTL_ENABLED_3_Pos            3UL
#define PERI_GR_SL_CTL_ENABLED_3_Msk            0x8UL
#define PERI_GR_SL_CTL_ENABLED_4_Pos            4UL
#define PERI_GR_SL_CTL_ENABLED_4_Msk            0x10UL
#define PERI_GR_SL_CTL_ENABLED_5_Pos            5UL
#define PERI_GR_SL_CTL_ENABLED_5_Msk            0x20UL
#define PERI_GR_SL_CTL_ENABLED_6_Pos            6UL
#define PERI_GR_SL_CTL_ENABLED_6_Msk            0x40UL
#define PERI_GR_SL_CTL_ENABLED_7_Pos            7UL
#define PERI_GR_SL_CTL_ENABLED_7_Msk            0x80UL
#define PERI_GR_SL_CTL_ENABLED_8_Pos            8UL
#define PERI_GR_SL_CTL_ENABLED_8_Msk            0x100UL
#define PERI_GR_SL_CTL_ENABLED_9_Pos            9UL
#define PERI_GR_SL_CTL_ENABLED_9_Msk            0x200UL
#define PERI_GR_SL_CTL_ENABLED_10_Pos           10UL
#define PERI_GR_SL_CTL_ENABLED_10_Msk           0x400UL
#define PERI_GR_SL_CTL_ENABLED_11_Pos           11UL
#define PERI_GR_SL_CTL_ENABLED_11_Msk           0x800UL
#define PERI_GR_SL_CTL_ENABLED_12_Pos           12UL
#define PERI_GR_SL_CTL_ENABLED_12_Msk           0x1000UL
#define PERI_GR_SL_CTL_ENABLED_13_Pos           13UL
#define PERI_GR_SL_CTL_ENABLED_13_Msk           0x2000UL
#define PERI_GR_SL_CTL_ENABLED_14_Pos           14UL
#define PERI_GR_SL_CTL_ENABLED_14_Msk           0x4000UL
#define PERI_GR_SL_CTL_ENABLED_15_Pos           15UL
#define PERI_GR_SL_CTL_ENABLED_15_Msk           0x8000UL
#define PERI_GR_SL_CTL_DISABLED_0_Pos           16UL
#define PERI_GR_SL_CTL_DISABLED_0_Msk           0x10000UL
#define PERI_GR_SL_CTL_DISABLED_1_Pos           17UL
#define PERI_GR_SL_CTL_DISABLED_1_Msk           0x20000UL
#define PERI_GR_SL_CTL_DISABLED_2_Pos           18UL
#define PERI_GR_SL_CTL_DISABLED_2_Msk           0x40000UL
#define PERI_GR_SL_CTL_DISABLED_3_Pos           19UL
#define PERI_GR_SL_CTL_DISABLED_3_Msk           0x80000UL
#define PERI_GR_SL_CTL_DISABLED_4_Pos           20UL
#define PERI_GR_SL_CTL_DISABLED_4_Msk           0x100000UL
#define PERI_GR_SL_CTL_DISABLED_5_Pos           21UL
#define PERI_GR_SL_CTL_DISABLED_5_Msk           0x200000UL
#define PERI_GR_SL_CTL_DISABLED_6_Pos           22UL
#define PERI_GR_SL_CTL_DISABLED_6_Msk           0x400000UL
#define PERI_GR_SL_CTL_DISABLED_7_Pos           23UL
#define PERI_GR_SL_CTL_DISABLED_7_Msk           0x800000UL
#define PERI_GR_SL_CTL_DISABLED_8_Pos           24UL
#define PERI_GR_SL_CTL_DISABLED_8_Msk           0x1000000UL
#define PERI_GR_SL_CTL_DISABLED_9_Pos           25UL
#define PERI_GR_SL_CTL_DISABLED_9_Msk           0x2000000UL
#define PERI_GR_SL_CTL_DISABLED_10_Pos          26UL
#define PERI_GR_SL_CTL_DISABLED_10_Msk          0x4000000UL
#define PERI_GR_SL_CTL_DISABLED_11_Pos          27UL
#define PERI_GR_SL_CTL_DISABLED_11_Msk          0x8000000UL
#define PERI_GR_SL_CTL_DISABLED_12_Pos          28UL
#define PERI_GR_SL_CTL_DISABLED_12_Msk          0x10000000UL
#define PERI_GR_SL_CTL_DISABLED_13_Pos          29UL
#define PERI_GR_SL_CTL_DISABLED_13_Msk          0x20000000UL
#define PERI_GR_SL_CTL_DISABLED_14_Pos          30UL
#define PERI_GR_SL_CTL_DISABLED_14_Msk          0x40000000UL
#define PERI_GR_SL_CTL_DISABLED_15_Pos          31UL
#define PERI_GR_SL_CTL_DISABLED_15_Msk          0x80000000UL


/* PERI_TR_GR.TR_CTL */
#define PERI_TR_GR_TR_CTL_TR_SEL_Pos            0UL
#define PERI_TR_GR_TR_CTL_TR_SEL_Msk            0xFFUL
#define PERI_TR_GR_TR_CTL_TR_INV_Pos            8UL
#define PERI_TR_GR_TR_CTL_TR_INV_Msk            0x100UL
#define PERI_TR_GR_TR_CTL_TR_EDGE_Pos           9UL
#define PERI_TR_GR_TR_CTL_TR_EDGE_Msk           0x200UL
#define PERI_TR_GR_TR_CTL_DBG_FREEZE_EN_Pos     12UL
#define PERI_TR_GR_TR_CTL_DBG_FREEZE_EN_Msk     0x1000UL


/* PERI_TR_1TO1_GR.TR_CTL */
#define PERI_TR_1TO1_GR_TR_CTL_TR_SEL_Pos       0UL
#define PERI_TR_1TO1_GR_TR_CTL_TR_SEL_Msk       0x1UL
#define PERI_TR_1TO1_GR_TR_CTL_TR_INV_Pos       8UL
#define PERI_TR_1TO1_GR_TR_CTL_TR_INV_Msk       0x100UL
#define PERI_TR_1TO1_GR_TR_CTL_TR_EDGE_Pos      9UL
#define PERI_TR_1TO1_GR_TR_CTL_TR_EDGE_Msk      0x200UL
#define PERI_TR_1TO1_GR_TR_CTL_DBG_FREEZE_EN_Pos 12UL
#define PERI_TR_1TO1_GR_TR_CTL_DBG_FREEZE_EN_Msk 0x1000UL


/* PERI.TIMEOUT_CTL */
#define PERI_TIMEOUT_CTL_TIMEOUT_Pos            0UL
#define PERI_TIMEOUT_CTL_TIMEOUT_Msk            0xFFFFUL
/* PERI.TR_CMD */
#define PERI_TR_CMD_TR_SEL_Pos                  0UL
#define PERI_TR_CMD_TR_SEL_Msk                  0xFFUL
#define PERI_TR_CMD_GROUP_SEL_Pos               8UL
#define PERI_TR_CMD_GROUP_SEL_Msk               0x1F00UL
#define PERI_TR_CMD_TR_EDGE_Pos                 29UL
#define PERI_TR_CMD_TR_EDGE_Msk                 0x20000000UL
#define PERI_TR_CMD_OUT_SEL_Pos                 30UL
#define PERI_TR_CMD_OUT_SEL_Msk                 0x40000000UL
#define PERI_TR_CMD_ACTIVATE_Pos                31UL
#define PERI_TR_CMD_ACTIVATE_Msk                0x80000000UL
/* PERI.DIV_CMD */
#define PERI_DIV_CMD_DIV_SEL_Pos                0UL
#define PERI_DIV_CMD_DIV_SEL_Msk                0xFFUL
#define PERI_DIV_CMD_TYPE_SEL_Pos               8UL
#define PERI_DIV_CMD_TYPE_SEL_Msk               0x300UL
#define PERI_DIV_CMD_PA_DIV_SEL_Pos             16UL
#define PERI_DIV_CMD_PA_DIV_SEL_Msk             0xFF0000UL
#define PERI_DIV_CMD_PA_TYPE_SEL_Pos            24UL
#define PERI_DIV_CMD_PA_TYPE_SEL_Msk            0x3000000UL
#define PERI_DIV_CMD_DISABLE_Pos                30UL
#define PERI_DIV_CMD_DISABLE_Msk                0x40000000UL
#define PERI_DIV_CMD_ENABLE_Pos                 31UL
#define PERI_DIV_CMD_ENABLE_Msk                 0x80000000UL
/* PERI.CLOCK_CTL */
#define PERI_CLOCK_CTL_DIV_SEL_Pos              0UL
#define PERI_CLOCK_CTL_DIV_SEL_Msk              0xFFUL
#define PERI_CLOCK_CTL_TYPE_SEL_Pos             8UL
#define PERI_CLOCK_CTL_TYPE_SEL_Msk             0x300UL
/* PERI.DIV_8_CTL */
#define PERI_DIV_8_CTL_EN_Pos                   0UL
#define PERI_DIV_8_CTL_EN_Msk                   0x1UL
#define PERI_DIV_8_CTL_INT8_DIV_Pos             8UL
#define PERI_DIV_8_CTL_INT8_DIV_Msk             0xFF00UL
/* PERI.DIV_16_CTL */
#define PERI_DIV_16_CTL_EN_Pos                  0UL
#define PERI_DIV_16_CTL_EN_Msk                  0x1UL
#define PERI_DIV_16_CTL_INT16_DIV_Pos           8UL
#define PERI_DIV_16_CTL_INT16_DIV_Msk           0xFFFF00UL
/* PERI.DIV_16_5_CTL */
#define PERI_DIV_16_5_CTL_EN_Pos                0UL
#define PERI_DIV_16_5_CTL_EN_Msk                0x1UL
#define PERI_DIV_16_5_CTL_FRAC5_DIV_Pos         3UL
#define PERI_DIV_16_5_CTL_FRAC5_DIV_Msk         0xF8UL
#define PERI_DIV_16_5_CTL_INT16_DIV_Pos         8UL
#define PERI_DIV_16_5_CTL_INT16_DIV_Msk         0xFFFF00UL
/* PERI.DIV_24_5_CTL */
#define PERI_DIV_24_5_CTL_EN_Pos                0UL
#define PERI_DIV_24_5_CTL_EN_Msk                0x1UL
#define PERI_DIV_24_5_CTL_FRAC5_DIV_Pos         3UL
#define PERI_DIV_24_5_CTL_FRAC5_DIV_Msk         0xF8UL
#define PERI_DIV_24_5_CTL_INT24_DIV_Pos         8UL
#define PERI_DIV_24_5_CTL_INT24_DIV_Msk         0xFFFFFF00UL
/* PERI.ECC_CTL */
#define PERI_ECC_CTL_WORD_ADDR_Pos              0UL
#define PERI_ECC_CTL_WORD_ADDR_Msk              0x7FFUL
#define PERI_ECC_CTL_ECC_EN_Pos                 16UL
#define PERI_ECC_CTL_ECC_EN_Msk                 0x10000UL
#define PERI_ECC_CTL_ECC_INJ_EN_Pos             18UL
#define PERI_ECC_CTL_ECC_INJ_EN_Msk             0x40000UL
#define PERI_ECC_CTL_PARITY_Pos                 24UL
#define PERI_ECC_CTL_PARITY_Msk                 0xFF000000UL


#endif /* _CYIP_PERI_V2_H_ */


/* [] END OF FILE */
