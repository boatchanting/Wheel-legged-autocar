/***************************************************************************//**
* \file cyip_dmac.h
*
* \brief
* DMAC IP definitions
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

#ifndef _CYIP_DMAC_H_
#define _CYIP_DMAC_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     DMAC
*******************************************************************************/

#define DMAC_CH_SECTION_SIZE                    0x00000100UL
#define DMAC_SECTION_SIZE                       0x00010000UL

/**
  * \brief Channel control (DMAC_CH_CTL)
  */
typedef struct stc_DMAC_CH_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         u1B:1;
  uint32_t                         :1;
  uint32_t                         u4PC:4;
  uint32_t                         u2PRIO:2;
  uint32_t                         :21;
  uint32_t                         u1ENABLED:1;
} stc_DMAC_CH_CTL_field_t;

typedef union un_DMAC_CH_CTL {
  uint32_t                         u32Register;
  stc_DMAC_CH_CTL_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_CTL_t;

/**
  * \brief Channel current indices (DMAC_CH_IDX)
  */
typedef struct stc_DMAC_CH_IDX_field {
  uint32_t                         u16X:16;
  uint32_t                         u16Y:16;
} stc_DMAC_CH_IDX_field_t;

typedef union un_DMAC_CH_IDX {
  uint32_t                         u32Register;
  stc_DMAC_CH_IDX_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_IDX_t;

/**
  * \brief Channel current source address (DMAC_CH_SRC)
  */
typedef struct stc_DMAC_CH_SRC_field {
  uint32_t                         u32ADDR:32;
} stc_DMAC_CH_SRC_field_t;

typedef union un_DMAC_CH_SRC {
  uint32_t                         u32Register;
  stc_DMAC_CH_SRC_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_SRC_t;

/**
  * \brief Channel current destination address (DMAC_CH_DST)
  */
typedef struct stc_DMAC_CH_DST_field {
  uint32_t                         u32ADDR:32;
} stc_DMAC_CH_DST_field_t;

typedef union un_DMAC_CH_DST {
  uint32_t                         u32Register;
  stc_DMAC_CH_DST_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DST_t;

/**
  * \brief Channel current descriptor pointer (DMAC_CH_CURR)
  */
typedef struct stc_DMAC_CH_CURR_field {
  uint32_t                         :2;
  uint32_t                         u30PTR:30;
} stc_DMAC_CH_CURR_field_t;

typedef union un_DMAC_CH_CURR {
  uint32_t                         u32Register;
  stc_DMAC_CH_CURR_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_CURR_t;

/**
  * \brief Channle software trigger (DMAC_CH_TR_CMD)
  */
typedef struct stc_DMAC_CH_TR_CMD_field {
  uint32_t                         u1ACTIVATE:1;
  uint32_t                         :31;
} stc_DMAC_CH_TR_CMD_field_t;

typedef union un_DMAC_CH_TR_CMD {
  uint32_t                         u32Register;
  stc_DMAC_CH_TR_CMD_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_TR_CMD_t;

/**
  * \brief Channel descriptor status (DMAC_CH_DESCR_STATUS)
  */
typedef struct stc_DMAC_CH_DESCR_STATUS_field {
  uint32_t                         :31;
  uint32_t                         u1VALID:1;
} stc_DMAC_CH_DESCR_STATUS_field_t;

typedef union un_DMAC_CH_DESCR_STATUS {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_STATUS_t;

/**
  * \brief Channel descriptor control (DMAC_CH_DESCR_CTL)
  */
typedef struct stc_DMAC_CH_DESCR_CTL_field {
  uint32_t                         u2WAIT_FOR_DEACT:2;
  uint32_t                         u2INTR_TYPE:2;
  uint32_t                         u2TR_OUT_TYPE:2;
  uint32_t                         u2TR_IN_TYPE:2;
  uint32_t                         u1DATA_PREFETCH:1;
  uint32_t                         :7;
  uint32_t                         u2DATA_SIZE:2;
  uint32_t                         :6;
  uint32_t                         u1CH_DISABLE:1;
  uint32_t                         :1;
  uint32_t                         u1SRC_TRANSFER_SIZE:1;
  uint32_t                         u1DST_TRANSFER_SIZE:1;
  uint32_t                         u3DESCR_TYPE:3;
} stc_DMAC_CH_DESCR_CTL_field_t;

typedef union un_DMAC_CH_DESCR_CTL {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_CTL_t;

/**
  * \brief Channel descriptor source (DMAC_CH_DESCR_SRC)
  */
typedef struct stc_DMAC_CH_DESCR_SRC_field {
  uint32_t                         u32ADDR:32;
} stc_DMAC_CH_DESCR_SRC_field_t;

typedef union un_DMAC_CH_DESCR_SRC {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_SRC_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_SRC_t;

/**
  * \brief Channel descriptor destination (DMAC_CH_DESCR_DST)
  */
typedef struct stc_DMAC_CH_DESCR_DST_field {
  uint32_t                         u32ADDR:32;
} stc_DMAC_CH_DESCR_DST_field_t;

typedef union un_DMAC_CH_DESCR_DST {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_DST_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_DST_t;

/**
  * \brief Channel descriptor X size (DMAC_CH_DESCR_X_SIZE)
  */
typedef struct stc_DMAC_CH_DESCR_X_SIZE_field {
  uint32_t                         u16X_COUNT:16;
  uint32_t                         :16;
} stc_DMAC_CH_DESCR_X_SIZE_field_t;

typedef union un_DMAC_CH_DESCR_X_SIZE {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_X_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_X_SIZE_t;

/**
  * \brief Channel descriptor X increment (DMAC_CH_DESCR_X_INCR)
  */
typedef struct stc_DMAC_CH_DESCR_X_INCR_field {
  uint32_t                         u16SRC_X:16;
  uint32_t                         u16DST_X:16;
} stc_DMAC_CH_DESCR_X_INCR_field_t;

typedef union un_DMAC_CH_DESCR_X_INCR {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_X_INCR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_X_INCR_t;

/**
  * \brief Channel descriptor Y size (DMAC_CH_DESCR_Y_SIZE)
  */
typedef struct stc_DMAC_CH_DESCR_Y_SIZE_field {
  uint32_t                         u16Y_COUNT:16;
  uint32_t                         :16;
} stc_DMAC_CH_DESCR_Y_SIZE_field_t;

typedef union un_DMAC_CH_DESCR_Y_SIZE {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_Y_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_Y_SIZE_t;

/**
  * \brief Channel descriptor Y increment (DMAC_CH_DESCR_Y_INCR)
  */
typedef struct stc_DMAC_CH_DESCR_Y_INCR_field {
  uint32_t                         u16SRC_Y:16;
  uint32_t                         u16DST_Y:16;
} stc_DMAC_CH_DESCR_Y_INCR_field_t;

typedef union un_DMAC_CH_DESCR_Y_INCR {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_Y_INCR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_Y_INCR_t;

/**
  * \brief Channel descriptor next pointer (DMAC_CH_DESCR_NEXT)
  */
typedef struct stc_DMAC_CH_DESCR_NEXT_field {
  uint32_t                         :2;
  uint32_t                         u30PTR:30;
} stc_DMAC_CH_DESCR_NEXT_field_t;

typedef union un_DMAC_CH_DESCR_NEXT {
  uint32_t                         u32Register;
  stc_DMAC_CH_DESCR_NEXT_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_DESCR_NEXT_t;

/**
  * \brief Interrupt (DMAC_CH_INTR)
  */
typedef struct stc_DMAC_CH_INTR_field {
  uint32_t                         u1COMPLETION:1;
  uint32_t                         u1SRC_BUS_ERROR:1;
  uint32_t                         u1DST_BUS_ERROR:1;
  uint32_t                         u1SRC_MISAL:1;
  uint32_t                         u1DST_MISAL:1;
  uint32_t                         u1CURR_PTR_NULL:1;
  uint32_t                         u1ACTIVE_CH_DISABLED:1;
  uint32_t                         u1DESCR_BUS_ERROR:1;
  uint32_t                         :24;
} stc_DMAC_CH_INTR_field_t;

typedef union un_DMAC_CH_INTR {
  uint32_t                         u32Register;
  stc_DMAC_CH_INTR_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_INTR_t;

/**
  * \brief Interrupt set (DMAC_CH_INTR_SET)
  */
typedef struct stc_DMAC_CH_INTR_SET_field {
  uint32_t                         u1COMPLETION:1;
  uint32_t                         u1SRC_BUS_ERROR:1;
  uint32_t                         u1DST_BUS_ERROR:1;
  uint32_t                         u1SRC_MISAL:1;
  uint32_t                         u1DST_MISAL:1;
  uint32_t                         u1CURR_PTR_NULL:1;
  uint32_t                         u1ACTIVE_CH_DISABLED:1;
  uint32_t                         u1DESCR_BUS_ERROR:1;
  uint32_t                         :24;
} stc_DMAC_CH_INTR_SET_field_t;

typedef union un_DMAC_CH_INTR_SET {
  uint32_t                         u32Register;
  stc_DMAC_CH_INTR_SET_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_INTR_SET_t;

/**
  * \brief Interrupt mask (DMAC_CH_INTR_MASK)
  */
typedef struct stc_DMAC_CH_INTR_MASK_field {
  uint32_t                         u1COMPLETION:1;
  uint32_t                         u1SRC_BUS_ERROR:1;
  uint32_t                         u1DST_BUS_ERROR:1;
  uint32_t                         u1SRC_MISAL:1;
  uint32_t                         u1DST_MISAL:1;
  uint32_t                         u1CURR_PTR_NULL:1;
  uint32_t                         u1ACTIVE_CH_DISABLED:1;
  uint32_t                         u1DESCR_BUS_ERROR:1;
  uint32_t                         :24;
} stc_DMAC_CH_INTR_MASK_field_t;

typedef union un_DMAC_CH_INTR_MASK {
  uint32_t                         u32Register;
  stc_DMAC_CH_INTR_MASK_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_INTR_MASK_t;

/**
  * \brief Interrupt masked (DMAC_CH_INTR_MASKED)
  */
typedef struct stc_DMAC_CH_INTR_MASKED_field {
  uint32_t                         u1COMPLETION:1;
  uint32_t                         u1SRC_BUS_ERROR:1;
  uint32_t                         u1DST_BUS_ERROR:1;
  uint32_t                         u1SRC_MISAL:1;
  uint32_t                         u1DST_MISAL:1;
  uint32_t                         u1CURR_PTR_NULL:1;
  uint32_t                         u1ACTIVE_CH_DISABLED:1;
  uint32_t                         u1DESCR_BUS_ERROR:1;
  uint32_t                         :24;
} stc_DMAC_CH_INTR_MASKED_field_t;

typedef union un_DMAC_CH_INTR_MASKED {
  uint32_t                         u32Register;
  stc_DMAC_CH_INTR_MASKED_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CH_INTR_MASKED_t;



/**
  * \brief Control (DMAC_CTL)
  */
typedef struct stc_DMAC_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLED:1;
} stc_DMAC_CTL_field_t;

typedef union un_DMAC_CTL {
  uint32_t                         u32Register;
  stc_DMAC_CTL_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_CTL_t;

/**
  * \brief Active channels (DMAC_ACTIVE)
  */
typedef struct stc_DMAC_ACTIVE_field {
  uint32_t                         u8ACTIVE:8;
  uint32_t                         :24;
} stc_DMAC_ACTIVE_field_t;

typedef union un_DMAC_ACTIVE {
  uint32_t                         u32Register;
  stc_DMAC_ACTIVE_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_DMAC_ACTIVE_t;



/**
  * \brief DMA controller channel (DMAC_CH)
  */
typedef struct stc_DMAC_CH {
  un_DMAC_CH_CTL_t                 unCTL;              /*!< 0x00000000 Channel control */
  uint32_t                         au32Reserved[3];
  un_DMAC_CH_IDX_t                 unIDX;              /*!< 0x00000010 Channel current indices */
  un_DMAC_CH_SRC_t                 unSRC;              /*!< 0x00000014 Channel current source address */
  un_DMAC_CH_DST_t                 unDST;              /*!< 0x00000018 Channel current destination address */
  uint32_t                         u32Reserved1;
  un_DMAC_CH_CURR_t                unCURR;             /*!< 0x00000020 Channel current descriptor pointer */
  uint32_t                         u32Reserved2;
  un_DMAC_CH_TR_CMD_t              unTR_CMD;           /*!< 0x00000028 Channle software trigger */
  uint32_t                         au32Reserved3[5];
  un_DMAC_CH_DESCR_STATUS_t        unDESCR_STATUS;     /*!< 0x00000040 Channel descriptor status */
  uint32_t                         au32Reserved4[7];
  un_DMAC_CH_DESCR_CTL_t           unDESCR_CTL;        /*!< 0x00000060 Channel descriptor control */
  un_DMAC_CH_DESCR_SRC_t           unDESCR_SRC;        /*!< 0x00000064 Channel descriptor source */
  un_DMAC_CH_DESCR_DST_t           unDESCR_DST;        /*!< 0x00000068 Channel descriptor destination */
  un_DMAC_CH_DESCR_X_SIZE_t        unDESCR_X_SIZE;     /*!< 0x0000006C Channel descriptor X size */
  un_DMAC_CH_DESCR_X_INCR_t        unDESCR_X_INCR;     /*!< 0x00000070 Channel descriptor X increment */
  un_DMAC_CH_DESCR_Y_SIZE_t        unDESCR_Y_SIZE;     /*!< 0x00000074 Channel descriptor Y size */
  un_DMAC_CH_DESCR_Y_INCR_t        unDESCR_Y_INCR;     /*!< 0x00000078 Channel descriptor Y increment */
  un_DMAC_CH_DESCR_NEXT_t          unDESCR_NEXT;       /*!< 0x0000007C Channel descriptor next pointer */
  un_DMAC_CH_INTR_t                unINTR;             /*!< 0x00000080 Interrupt */
  un_DMAC_CH_INTR_SET_t            unINTR_SET;         /*!< 0x00000084 Interrupt set */
  un_DMAC_CH_INTR_MASK_t           unINTR_MASK;        /*!< 0x00000088 Interrupt mask */
  un_DMAC_CH_INTR_MASKED_t         unINTR_MASKED;      /*!< 0x0000008C Interrupt masked */
  uint32_t                         au32Reserved5[28];
} stc_DMAC_CH_t;                                       /*!< Size = 256 (0x100) */

/**
  * \brief DMAC (DMAC)
  */
typedef struct stc_DMAC {
  un_DMAC_CTL_t                    unCTL;              /*!< 0x00000000 Control */
  uint32_t                         u32Reserved;
  un_DMAC_ACTIVE_t                 unACTIVE;           /*!< 0x00000008 Active channels */
  uint32_t                         au32Reserved1[1021];
  stc_DMAC_CH_t                    CH[8];              /*!< 0x00001000 DMA controller channel */
} stc_DMAC_t;                                          /*!< Size = 6144 (0x1800) */


/* DMAC_CH.CTL */
#define DMAC_CH_CTL_P_Pos                       0UL
#define DMAC_CH_CTL_P_Msk                       0x1UL
#define DMAC_CH_CTL_NS_Pos                      1UL
#define DMAC_CH_CTL_NS_Msk                      0x2UL
#define DMAC_CH_CTL_B_Pos                       2UL
#define DMAC_CH_CTL_B_Msk                       0x4UL
#define DMAC_CH_CTL_PC_Pos                      4UL
#define DMAC_CH_CTL_PC_Msk                      0xF0UL
#define DMAC_CH_CTL_PRIO_Pos                    8UL
#define DMAC_CH_CTL_PRIO_Msk                    0x300UL
#define DMAC_CH_CTL_ENABLED_Pos                 31UL
#define DMAC_CH_CTL_ENABLED_Msk                 0x80000000UL
/* DMAC_CH.IDX */
#define DMAC_CH_IDX_X_Pos                       0UL
#define DMAC_CH_IDX_X_Msk                       0xFFFFUL
#define DMAC_CH_IDX_Y_Pos                       16UL
#define DMAC_CH_IDX_Y_Msk                       0xFFFF0000UL
/* DMAC_CH.SRC */
#define DMAC_CH_SRC_ADDR_Pos                    0UL
#define DMAC_CH_SRC_ADDR_Msk                    0xFFFFFFFFUL
/* DMAC_CH.DST */
#define DMAC_CH_DST_ADDR_Pos                    0UL
#define DMAC_CH_DST_ADDR_Msk                    0xFFFFFFFFUL
/* DMAC_CH.CURR */
#define DMAC_CH_CURR_PTR_Pos                    2UL
#define DMAC_CH_CURR_PTR_Msk                    0xFFFFFFFCUL
/* DMAC_CH.TR_CMD */
#define DMAC_CH_TR_CMD_ACTIVATE_Pos             0UL
#define DMAC_CH_TR_CMD_ACTIVATE_Msk             0x1UL
/* DMAC_CH.DESCR_STATUS */
#define DMAC_CH_DESCR_STATUS_VALID_Pos          31UL
#define DMAC_CH_DESCR_STATUS_VALID_Msk          0x80000000UL
/* DMAC_CH.DESCR_CTL */
#define DMAC_CH_DESCR_CTL_WAIT_FOR_DEACT_Pos    0UL
#define DMAC_CH_DESCR_CTL_WAIT_FOR_DEACT_Msk    0x3UL
#define DMAC_CH_DESCR_CTL_INTR_TYPE_Pos         2UL
#define DMAC_CH_DESCR_CTL_INTR_TYPE_Msk         0xCUL
#define DMAC_CH_DESCR_CTL_TR_OUT_TYPE_Pos       4UL
#define DMAC_CH_DESCR_CTL_TR_OUT_TYPE_Msk       0x30UL
#define DMAC_CH_DESCR_CTL_TR_IN_TYPE_Pos        6UL
#define DMAC_CH_DESCR_CTL_TR_IN_TYPE_Msk        0xC0UL
#define DMAC_CH_DESCR_CTL_DATA_PREFETCH_Pos     8UL
#define DMAC_CH_DESCR_CTL_DATA_PREFETCH_Msk     0x100UL
#define DMAC_CH_DESCR_CTL_DATA_SIZE_Pos         16UL
#define DMAC_CH_DESCR_CTL_DATA_SIZE_Msk         0x30000UL
#define DMAC_CH_DESCR_CTL_CH_DISABLE_Pos        24UL
#define DMAC_CH_DESCR_CTL_CH_DISABLE_Msk        0x1000000UL
#define DMAC_CH_DESCR_CTL_SRC_TRANSFER_SIZE_Pos 26UL
#define DMAC_CH_DESCR_CTL_SRC_TRANSFER_SIZE_Msk 0x4000000UL
#define DMAC_CH_DESCR_CTL_DST_TRANSFER_SIZE_Pos 27UL
#define DMAC_CH_DESCR_CTL_DST_TRANSFER_SIZE_Msk 0x8000000UL
#define DMAC_CH_DESCR_CTL_DESCR_TYPE_Pos        28UL
#define DMAC_CH_DESCR_CTL_DESCR_TYPE_Msk        0x70000000UL
/* DMAC_CH.DESCR_SRC */
#define DMAC_CH_DESCR_SRC_ADDR_Pos              0UL
#define DMAC_CH_DESCR_SRC_ADDR_Msk              0xFFFFFFFFUL
/* DMAC_CH.DESCR_DST */
#define DMAC_CH_DESCR_DST_ADDR_Pos              0UL
#define DMAC_CH_DESCR_DST_ADDR_Msk              0xFFFFFFFFUL
/* DMAC_CH.DESCR_X_SIZE */
#define DMAC_CH_DESCR_X_SIZE_X_COUNT_Pos        0UL
#define DMAC_CH_DESCR_X_SIZE_X_COUNT_Msk        0xFFFFUL
/* DMAC_CH.DESCR_X_INCR */
#define DMAC_CH_DESCR_X_INCR_SRC_X_Pos          0UL
#define DMAC_CH_DESCR_X_INCR_SRC_X_Msk          0xFFFFUL
#define DMAC_CH_DESCR_X_INCR_DST_X_Pos          16UL
#define DMAC_CH_DESCR_X_INCR_DST_X_Msk          0xFFFF0000UL
/* DMAC_CH.DESCR_Y_SIZE */
#define DMAC_CH_DESCR_Y_SIZE_Y_COUNT_Pos        0UL
#define DMAC_CH_DESCR_Y_SIZE_Y_COUNT_Msk        0xFFFFUL
/* DMAC_CH.DESCR_Y_INCR */
#define DMAC_CH_DESCR_Y_INCR_SRC_Y_Pos          0UL
#define DMAC_CH_DESCR_Y_INCR_SRC_Y_Msk          0xFFFFUL
#define DMAC_CH_DESCR_Y_INCR_DST_Y_Pos          16UL
#define DMAC_CH_DESCR_Y_INCR_DST_Y_Msk          0xFFFF0000UL
/* DMAC_CH.DESCR_NEXT */
#define DMAC_CH_DESCR_NEXT_PTR_Pos              2UL
#define DMAC_CH_DESCR_NEXT_PTR_Msk              0xFFFFFFFCUL
/* DMAC_CH.INTR */
#define DMAC_CH_INTR_COMPLETION_Pos             0UL
#define DMAC_CH_INTR_COMPLETION_Msk             0x1UL
#define DMAC_CH_INTR_SRC_BUS_ERROR_Pos          1UL
#define DMAC_CH_INTR_SRC_BUS_ERROR_Msk          0x2UL
#define DMAC_CH_INTR_DST_BUS_ERROR_Pos          2UL
#define DMAC_CH_INTR_DST_BUS_ERROR_Msk          0x4UL
#define DMAC_CH_INTR_SRC_MISAL_Pos              3UL
#define DMAC_CH_INTR_SRC_MISAL_Msk              0x8UL
#define DMAC_CH_INTR_DST_MISAL_Pos              4UL
#define DMAC_CH_INTR_DST_MISAL_Msk              0x10UL
#define DMAC_CH_INTR_CURR_PTR_NULL_Pos          5UL
#define DMAC_CH_INTR_CURR_PTR_NULL_Msk          0x20UL
#define DMAC_CH_INTR_ACTIVE_CH_DISABLED_Pos     6UL
#define DMAC_CH_INTR_ACTIVE_CH_DISABLED_Msk     0x40UL
#define DMAC_CH_INTR_DESCR_BUS_ERROR_Pos        7UL
#define DMAC_CH_INTR_DESCR_BUS_ERROR_Msk        0x80UL
/* DMAC_CH.INTR_SET */
#define DMAC_CH_INTR_SET_COMPLETION_Pos         0UL
#define DMAC_CH_INTR_SET_COMPLETION_Msk         0x1UL
#define DMAC_CH_INTR_SET_SRC_BUS_ERROR_Pos      1UL
#define DMAC_CH_INTR_SET_SRC_BUS_ERROR_Msk      0x2UL
#define DMAC_CH_INTR_SET_DST_BUS_ERROR_Pos      2UL
#define DMAC_CH_INTR_SET_DST_BUS_ERROR_Msk      0x4UL
#define DMAC_CH_INTR_SET_SRC_MISAL_Pos          3UL
#define DMAC_CH_INTR_SET_SRC_MISAL_Msk          0x8UL
#define DMAC_CH_INTR_SET_DST_MISAL_Pos          4UL
#define DMAC_CH_INTR_SET_DST_MISAL_Msk          0x10UL
#define DMAC_CH_INTR_SET_CURR_PTR_NULL_Pos      5UL
#define DMAC_CH_INTR_SET_CURR_PTR_NULL_Msk      0x20UL
#define DMAC_CH_INTR_SET_ACTIVE_CH_DISABLED_Pos 6UL
#define DMAC_CH_INTR_SET_ACTIVE_CH_DISABLED_Msk 0x40UL
#define DMAC_CH_INTR_SET_DESCR_BUS_ERROR_Pos    7UL
#define DMAC_CH_INTR_SET_DESCR_BUS_ERROR_Msk    0x80UL
/* DMAC_CH.INTR_MASK */
#define DMAC_CH_INTR_MASK_COMPLETION_Pos        0UL
#define DMAC_CH_INTR_MASK_COMPLETION_Msk        0x1UL
#define DMAC_CH_INTR_MASK_SRC_BUS_ERROR_Pos     1UL
#define DMAC_CH_INTR_MASK_SRC_BUS_ERROR_Msk     0x2UL
#define DMAC_CH_INTR_MASK_DST_BUS_ERROR_Pos     2UL
#define DMAC_CH_INTR_MASK_DST_BUS_ERROR_Msk     0x4UL
#define DMAC_CH_INTR_MASK_SRC_MISAL_Pos         3UL
#define DMAC_CH_INTR_MASK_SRC_MISAL_Msk         0x8UL
#define DMAC_CH_INTR_MASK_DST_MISAL_Pos         4UL
#define DMAC_CH_INTR_MASK_DST_MISAL_Msk         0x10UL
#define DMAC_CH_INTR_MASK_CURR_PTR_NULL_Pos     5UL
#define DMAC_CH_INTR_MASK_CURR_PTR_NULL_Msk     0x20UL
#define DMAC_CH_INTR_MASK_ACTIVE_CH_DISABLED_Pos 6UL
#define DMAC_CH_INTR_MASK_ACTIVE_CH_DISABLED_Msk 0x40UL
#define DMAC_CH_INTR_MASK_DESCR_BUS_ERROR_Pos   7UL
#define DMAC_CH_INTR_MASK_DESCR_BUS_ERROR_Msk   0x80UL
/* DMAC_CH.INTR_MASKED */
#define DMAC_CH_INTR_MASKED_COMPLETION_Pos      0UL
#define DMAC_CH_INTR_MASKED_COMPLETION_Msk      0x1UL
#define DMAC_CH_INTR_MASKED_SRC_BUS_ERROR_Pos   1UL
#define DMAC_CH_INTR_MASKED_SRC_BUS_ERROR_Msk   0x2UL
#define DMAC_CH_INTR_MASKED_DST_BUS_ERROR_Pos   2UL
#define DMAC_CH_INTR_MASKED_DST_BUS_ERROR_Msk   0x4UL
#define DMAC_CH_INTR_MASKED_SRC_MISAL_Pos       3UL
#define DMAC_CH_INTR_MASKED_SRC_MISAL_Msk       0x8UL
#define DMAC_CH_INTR_MASKED_DST_MISAL_Pos       4UL
#define DMAC_CH_INTR_MASKED_DST_MISAL_Msk       0x10UL
#define DMAC_CH_INTR_MASKED_CURR_PTR_NULL_Pos   5UL
#define DMAC_CH_INTR_MASKED_CURR_PTR_NULL_Msk   0x20UL
#define DMAC_CH_INTR_MASKED_ACTIVE_CH_DISABLED_Pos 6UL
#define DMAC_CH_INTR_MASKED_ACTIVE_CH_DISABLED_Msk 0x40UL
#define DMAC_CH_INTR_MASKED_DESCR_BUS_ERROR_Pos 7UL
#define DMAC_CH_INTR_MASKED_DESCR_BUS_ERROR_Msk 0x80UL


/* DMAC.CTL */
#define DMAC_CTL_ENABLED_Pos                    31UL
#define DMAC_CTL_ENABLED_Msk                    0x80000000UL
/* DMAC.ACTIVE */
#define DMAC_ACTIVE_ACTIVE_Pos                  0UL
#define DMAC_ACTIVE_ACTIVE_Msk                  0xFFUL


#endif /* _CYIP_DMAC_H_ */


/* [] END OF FILE */
