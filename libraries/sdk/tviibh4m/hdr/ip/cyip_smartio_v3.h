/***************************************************************************//**
* \file cyip_smartio_v3.h
*
* \brief
* SMARTIO IP definitions
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

#ifndef _CYIP_SMARTIO_V3_H_
#define _CYIP_SMARTIO_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                   SMARTIO
*******************************************************************************/

#define SMARTIO_PRT_SECTION_SIZE                0x00000100UL
#define SMARTIO_SECTION_SIZE                    0x00010000UL

/**
  * \brief Control register (SMARTIO_PRT_CTL)
  */
typedef struct stc_SMARTIO_PRT_CTL_field {
  uint32_t                         u8BYPASS:8;
  uint32_t                         u5CLOCK_SRC:5;
  uint32_t                         :11;
  uint32_t                         u1HLD_OVR:1;
  uint32_t                         u1PIPELINE_EN:1;
  uint32_t                         :5;
  uint32_t                         u1ENABLED:1;
} stc_SMARTIO_PRT_CTL_field_t;

typedef union un_SMARTIO_PRT_CTL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_CTL_t;

/**
  * \brief Synchronization control register (SMARTIO_PRT_SYNC_CTL)
  */
typedef struct stc_SMARTIO_PRT_SYNC_CTL_field {
  uint32_t                         u8IO_SYNC_EN:8;
  uint32_t                         u8CHIP_SYNC_EN:8;
  uint32_t                         :16;
} stc_SMARTIO_PRT_SYNC_CTL_field_t;

typedef union un_SMARTIO_PRT_SYNC_CTL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_SYNC_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_SYNC_CTL_t;

/**
  * \brief LUT component input selection (SMARTIO_PRT_LUT_SEL)
  */
typedef struct stc_SMARTIO_PRT_LUT_SEL_field {
  uint32_t                         u4LUT_TR0_SEL:4;
  uint32_t                         :4;
  uint32_t                         u4LUT_TR1_SEL:4;
  uint32_t                         :4;
  uint32_t                         u4LUT_TR2_SEL:4;
  uint32_t                         :12;
} stc_SMARTIO_PRT_LUT_SEL_field_t;

typedef union un_SMARTIO_PRT_LUT_SEL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_LUT_SEL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_LUT_SEL_t;

/**
  * \brief LUT component control register (SMARTIO_PRT_LUT_CTL)
  */
typedef struct stc_SMARTIO_PRT_LUT_CTL_field {
  uint32_t                         u8LUT:8;
  uint32_t                         u2LUT_OPC:2;
  uint32_t                         :22;
} stc_SMARTIO_PRT_LUT_CTL_field_t;

typedef union un_SMARTIO_PRT_LUT_CTL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_LUT_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_LUT_CTL_t;

/**
  * \brief Data unit component input selection (SMARTIO_PRT_DU_SEL)
  */
typedef struct stc_SMARTIO_PRT_DU_SEL_field {
  uint32_t                         u4DU_TR0_SEL:4;
  uint32_t                         :4;
  uint32_t                         u4DU_TR1_SEL:4;
  uint32_t                         :4;
  uint32_t                         u4DU_TR2_SEL:4;
  uint32_t                         :4;
  uint32_t                         u2DU_DATA0_SEL:2;
  uint32_t                         :2;
  uint32_t                         u2DU_DATA1_SEL:2;
  uint32_t                         :2;
} stc_SMARTIO_PRT_DU_SEL_field_t;

typedef union un_SMARTIO_PRT_DU_SEL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_DU_SEL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_DU_SEL_t;

/**
  * \brief Data unit component control register (SMARTIO_PRT_DU_CTL)
  */
typedef struct stc_SMARTIO_PRT_DU_CTL_field {
  uint32_t                         u3DU_SIZE:3;
  uint32_t                         :5;
  uint32_t                         u4DU_OPC:4;
  uint32_t                         :20;
} stc_SMARTIO_PRT_DU_CTL_field_t;

typedef union un_SMARTIO_PRT_DU_CTL {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_DU_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_DU_CTL_t;

/**
  * \brief Data register (SMARTIO_PRT_DATA)
  */
typedef struct stc_SMARTIO_PRT_DATA_field {
  uint32_t                         u8DATA:8;
  uint32_t                         :24;
} stc_SMARTIO_PRT_DATA_field_t;

typedef union un_SMARTIO_PRT_DATA {
  uint32_t                         u32Register;
  stc_SMARTIO_PRT_DATA_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_SMARTIO_PRT_DATA_t;



/**
  * \brief Programmable IO port registers (SMARTIO_PRT)
  */
typedef struct stc_SMARTIO_PRT {
  un_SMARTIO_PRT_CTL_t             unCTL;              /*!< 0x00000000 Control register */
  uint32_t                         au32Reserved[3];
  un_SMARTIO_PRT_SYNC_CTL_t        unSYNC_CTL;         /*!< 0x00000010 Synchronization control register */
  uint32_t                         au32Reserved1[3];
  un_SMARTIO_PRT_LUT_SEL_t         unLUT_SEL[8];       /*!< 0x00000020 LUT component input selection */
  un_SMARTIO_PRT_LUT_CTL_t         unLUT_CTL[8];       /*!< 0x00000040 LUT component control register */
  uint32_t                         au32Reserved2[24];
  un_SMARTIO_PRT_DU_SEL_t          unDU_SEL;           /*!< 0x000000C0 Data unit component input selection */
  un_SMARTIO_PRT_DU_CTL_t          unDU_CTL;           /*!< 0x000000C4 Data unit component control register */
  uint32_t                         au32Reserved3[10];
  un_SMARTIO_PRT_DATA_t            unDATA;             /*!< 0x000000F0 Data register */
  uint32_t                         au32Reserved4[3];
} stc_SMARTIO_PRT_t;                                   /*!< Size = 256 (0x100) */

/**
  * \brief Programmable IO configuration (SMARTIO)
  */
typedef struct stc_SMARTIO {
  stc_SMARTIO_PRT_t                PRT[128];           /*!< 0x00000000 Programmable IO port registers */
} stc_SMARTIO_t;                                       /*!< Size = 32768 (0x8000) */


/* SMARTIO_PRT.CTL */
#define SMARTIO_PRT_CTL_BYPASS_Pos              0UL
#define SMARTIO_PRT_CTL_BYPASS_Msk              0xFFUL
#define SMARTIO_PRT_CTL_CLOCK_SRC_Pos           8UL
#define SMARTIO_PRT_CTL_CLOCK_SRC_Msk           0x1F00UL
#define SMARTIO_PRT_CTL_HLD_OVR_Pos             24UL
#define SMARTIO_PRT_CTL_HLD_OVR_Msk             0x1000000UL
#define SMARTIO_PRT_CTL_PIPELINE_EN_Pos         25UL
#define SMARTIO_PRT_CTL_PIPELINE_EN_Msk         0x2000000UL
#define SMARTIO_PRT_CTL_ENABLED_Pos             31UL
#define SMARTIO_PRT_CTL_ENABLED_Msk             0x80000000UL
/* SMARTIO_PRT.SYNC_CTL */
#define SMARTIO_PRT_SYNC_CTL_IO_SYNC_EN_Pos     0UL
#define SMARTIO_PRT_SYNC_CTL_IO_SYNC_EN_Msk     0xFFUL
#define SMARTIO_PRT_SYNC_CTL_CHIP_SYNC_EN_Pos   8UL
#define SMARTIO_PRT_SYNC_CTL_CHIP_SYNC_EN_Msk   0xFF00UL
/* SMARTIO_PRT.LUT_SEL */
#define SMARTIO_PRT_LUT_SEL_LUT_TR0_SEL_Pos     0UL
#define SMARTIO_PRT_LUT_SEL_LUT_TR0_SEL_Msk     0xFUL
#define SMARTIO_PRT_LUT_SEL_LUT_TR1_SEL_Pos     8UL
#define SMARTIO_PRT_LUT_SEL_LUT_TR1_SEL_Msk     0xF00UL
#define SMARTIO_PRT_LUT_SEL_LUT_TR2_SEL_Pos     16UL
#define SMARTIO_PRT_LUT_SEL_LUT_TR2_SEL_Msk     0xF0000UL
/* SMARTIO_PRT.LUT_CTL */
#define SMARTIO_PRT_LUT_CTL_LUT_Pos             0UL
#define SMARTIO_PRT_LUT_CTL_LUT_Msk             0xFFUL
#define SMARTIO_PRT_LUT_CTL_LUT_OPC_Pos         8UL
#define SMARTIO_PRT_LUT_CTL_LUT_OPC_Msk         0x300UL
/* SMARTIO_PRT.DU_SEL */
#define SMARTIO_PRT_DU_SEL_DU_TR0_SEL_Pos       0UL
#define SMARTIO_PRT_DU_SEL_DU_TR0_SEL_Msk       0xFUL
#define SMARTIO_PRT_DU_SEL_DU_TR1_SEL_Pos       8UL
#define SMARTIO_PRT_DU_SEL_DU_TR1_SEL_Msk       0xF00UL
#define SMARTIO_PRT_DU_SEL_DU_TR2_SEL_Pos       16UL
#define SMARTIO_PRT_DU_SEL_DU_TR2_SEL_Msk       0xF0000UL
#define SMARTIO_PRT_DU_SEL_DU_DATA0_SEL_Pos     24UL
#define SMARTIO_PRT_DU_SEL_DU_DATA0_SEL_Msk     0x3000000UL
#define SMARTIO_PRT_DU_SEL_DU_DATA1_SEL_Pos     28UL
#define SMARTIO_PRT_DU_SEL_DU_DATA1_SEL_Msk     0x30000000UL
/* SMARTIO_PRT.DU_CTL */
#define SMARTIO_PRT_DU_CTL_DU_SIZE_Pos          0UL
#define SMARTIO_PRT_DU_CTL_DU_SIZE_Msk          0x7UL
#define SMARTIO_PRT_DU_CTL_DU_OPC_Pos           8UL
#define SMARTIO_PRT_DU_CTL_DU_OPC_Msk           0xF00UL
/* SMARTIO_PRT.DATA */
#define SMARTIO_PRT_DATA_DATA_Pos               0UL
#define SMARTIO_PRT_DATA_DATA_Msk               0xFFUL


#endif /* _CYIP_SMARTIO_V3_H_ */


/* [] END OF FILE */
