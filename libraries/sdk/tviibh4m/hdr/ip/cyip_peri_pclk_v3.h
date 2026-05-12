/***************************************************************************//**
* \file cyip_peri_pclk_v3.h
*
* \brief
* PERI_PCLK IP definitions
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

#ifndef _CYIP_PERI_PCLK_V3_H_
#define _CYIP_PERI_PCLK_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                  PERI_PCLK
*******************************************************************************/

#define PERI_PCLK_GR_SECTION_SIZE               0x00002000UL
#define PERI_PCLK_SECTION_SIZE                  0x00020000UL

/**
  * \brief Divider command (PERI_PCLK_GR_DIV_CMD)
  */
typedef struct stc_PERI_PCLK_GR_DIV_CMD_field {
  uint32_t                         u8DIV_SEL:8;
  uint32_t                         u2TYPE_SEL:2;
  uint32_t                         :6;
  uint32_t                         u8PA_DIV_SEL:8;
  uint32_t                         u2PA_TYPE_SEL:2;
  uint32_t                         :4;
  uint32_t                         u1DISABLE:1;
  uint32_t                         u1ENABLE:1;
} stc_PERI_PCLK_GR_DIV_CMD_field_t;

typedef union un_PERI_PCLK_GR_DIV_CMD {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_DIV_CMD_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_DIV_CMD_t;

/**
  * \brief Clock control (PERI_PCLK_GR_CLOCK_CTL)
  */
typedef struct stc_PERI_PCLK_GR_CLOCK_CTL_field {
  uint32_t                         u8DIV_SEL:8;
  uint32_t                         u2TYPE_SEL:2;
  uint32_t                         :22;
} stc_PERI_PCLK_GR_CLOCK_CTL_field_t;

typedef union un_PERI_PCLK_GR_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_CLOCK_CTL_t;

/**
  * \brief Divider control (for 8.0 divider) (PERI_PCLK_GR_DIV_8_CTL)
  */
typedef struct stc_PERI_PCLK_GR_DIV_8_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :7;
  uint32_t                         u8INT8_DIV:8;
  uint32_t                         :16;
} stc_PERI_PCLK_GR_DIV_8_CTL_field_t;

typedef union un_PERI_PCLK_GR_DIV_8_CTL {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_DIV_8_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_DIV_8_CTL_t;

/**
  * \brief Divider control (for 16.0 divider) (PERI_PCLK_GR_DIV_16_CTL)
  */
typedef struct stc_PERI_PCLK_GR_DIV_16_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :7;
  uint32_t                         u16INT16_DIV:16;
  uint32_t                         :8;
} stc_PERI_PCLK_GR_DIV_16_CTL_field_t;

typedef union un_PERI_PCLK_GR_DIV_16_CTL {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_DIV_16_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_DIV_16_CTL_t;

/**
  * \brief Divider control (for 16.5 divider) (PERI_PCLK_GR_DIV_16_5_CTL)
  */
typedef struct stc_PERI_PCLK_GR_DIV_16_5_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :2;
  uint32_t                         u5FRAC5_DIV:5;
  uint32_t                         u16INT16_DIV:16;
  uint32_t                         :8;
} stc_PERI_PCLK_GR_DIV_16_5_CTL_field_t;

typedef union un_PERI_PCLK_GR_DIV_16_5_CTL {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_DIV_16_5_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_DIV_16_5_CTL_t;

/**
  * \brief Divider control (for 24.5 divider) (PERI_PCLK_GR_DIV_24_5_CTL)
  */
typedef struct stc_PERI_PCLK_GR_DIV_24_5_CTL_field {
  uint32_t                         u1EN:1;
  uint32_t                         :2;
  uint32_t                         u5FRAC5_DIV:5;
  uint32_t                         u24INT24_DIV:24;
} stc_PERI_PCLK_GR_DIV_24_5_CTL_field_t;

typedef union un_PERI_PCLK_GR_DIV_24_5_CTL {
  uint32_t                         u32Register;
  stc_PERI_PCLK_GR_DIV_24_5_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_PCLK_GR_DIV_24_5_CTL_t;



/**
  * \brief PERI clock domains (PERI_PCLK_GR)
  */
typedef struct stc_PERI_PCLK_GR {
  un_PERI_PCLK_GR_DIV_CMD_t        unDIV_CMD;          /*!< 0x00000000 Divider command */
  uint32_t                         au32Reserved[767];
  un_PERI_PCLK_GR_CLOCK_CTL_t      unCLOCK_CTL[256];   /*!< 0x00000C00 Clock control */
  un_PERI_PCLK_GR_DIV_8_CTL_t      unDIV_8_CTL[256];   /*!< 0x00001000 Divider control (for 8.0 divider) */
  un_PERI_PCLK_GR_DIV_16_CTL_t     unDIV_16_CTL[256];  /*!< 0x00001400 Divider control (for 16.0 divider) */
  un_PERI_PCLK_GR_DIV_16_5_CTL_t   unDIV_16_5_CTL[256]; /*!< 0x00001800 Divider control (for 16.5 divider) */
  un_PERI_PCLK_GR_DIV_24_5_CTL_t   unDIV_24_5_CTL[255]; /*!< 0x00001C00 Divider control (for 24.5 divider) */
  uint32_t                         u32Reserved1;
} stc_PERI_PCLK_GR_t;                                  /*!< Size = 8192 (0x2000) */

/**
  * \brief Peripheral PCLK groups (PERI_PCLK)
  */
typedef struct stc_PERI_PCLK {
  stc_PERI_PCLK_GR_t               GR[16];             /*!< 0x00000000 PERI clock domains */
} stc_PERI_PCLK_t;                                     /*!< Size = 131072 (0x20000) */


/* PERI_PCLK_GR.DIV_CMD */
#define PERI_PCLK_GR_DIV_CMD_DIV_SEL_Pos        0UL
#define PERI_PCLK_GR_DIV_CMD_DIV_SEL_Msk        0xFFUL
#define PERI_PCLK_GR_DIV_CMD_TYPE_SEL_Pos       8UL
#define PERI_PCLK_GR_DIV_CMD_TYPE_SEL_Msk       0x300UL
#define PERI_PCLK_GR_DIV_CMD_PA_DIV_SEL_Pos     16UL
#define PERI_PCLK_GR_DIV_CMD_PA_DIV_SEL_Msk     0xFF0000UL
#define PERI_PCLK_GR_DIV_CMD_PA_TYPE_SEL_Pos    24UL
#define PERI_PCLK_GR_DIV_CMD_PA_TYPE_SEL_Msk    0x3000000UL
#define PERI_PCLK_GR_DIV_CMD_DISABLE_Pos        30UL
#define PERI_PCLK_GR_DIV_CMD_DISABLE_Msk        0x40000000UL
#define PERI_PCLK_GR_DIV_CMD_ENABLE_Pos         31UL
#define PERI_PCLK_GR_DIV_CMD_ENABLE_Msk         0x80000000UL
/* PERI_PCLK_GR.CLOCK_CTL */
#define PERI_PCLK_GR_CLOCK_CTL_DIV_SEL_Pos      0UL
#define PERI_PCLK_GR_CLOCK_CTL_DIV_SEL_Msk      0xFFUL
#define PERI_PCLK_GR_CLOCK_CTL_TYPE_SEL_Pos     8UL
#define PERI_PCLK_GR_CLOCK_CTL_TYPE_SEL_Msk     0x300UL
/* PERI_PCLK_GR.DIV_8_CTL */
#define PERI_PCLK_GR_DIV_8_CTL_EN_Pos           0UL
#define PERI_PCLK_GR_DIV_8_CTL_EN_Msk           0x1UL
#define PERI_PCLK_GR_DIV_8_CTL_INT8_DIV_Pos     8UL
#define PERI_PCLK_GR_DIV_8_CTL_INT8_DIV_Msk     0xFF00UL
/* PERI_PCLK_GR.DIV_16_CTL */
#define PERI_PCLK_GR_DIV_16_CTL_EN_Pos          0UL
#define PERI_PCLK_GR_DIV_16_CTL_EN_Msk          0x1UL
#define PERI_PCLK_GR_DIV_16_CTL_INT16_DIV_Pos   8UL
#define PERI_PCLK_GR_DIV_16_CTL_INT16_DIV_Msk   0xFFFF00UL
/* PERI_PCLK_GR.DIV_16_5_CTL */
#define PERI_PCLK_GR_DIV_16_5_CTL_EN_Pos        0UL
#define PERI_PCLK_GR_DIV_16_5_CTL_EN_Msk        0x1UL
#define PERI_PCLK_GR_DIV_16_5_CTL_FRAC5_DIV_Pos 3UL
#define PERI_PCLK_GR_DIV_16_5_CTL_FRAC5_DIV_Msk 0xF8UL
#define PERI_PCLK_GR_DIV_16_5_CTL_INT16_DIV_Pos 8UL
#define PERI_PCLK_GR_DIV_16_5_CTL_INT16_DIV_Msk 0xFFFF00UL
/* PERI_PCLK_GR.DIV_24_5_CTL */
#define PERI_PCLK_GR_DIV_24_5_CTL_EN_Pos        0UL
#define PERI_PCLK_GR_DIV_24_5_CTL_EN_Msk        0x1UL
#define PERI_PCLK_GR_DIV_24_5_CTL_FRAC5_DIV_Pos 3UL
#define PERI_PCLK_GR_DIV_24_5_CTL_FRAC5_DIV_Msk 0xF8UL
#define PERI_PCLK_GR_DIV_24_5_CTL_INT24_DIV_Pos 8UL
#define PERI_PCLK_GR_DIV_24_5_CTL_INT24_DIV_Msk 0xFFFFFF00UL


#endif /* _CYIP_PERI_PCLK_V3_H_ */


/* [] END OF FILE */
