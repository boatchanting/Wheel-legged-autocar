/***************************************************************************//**
* \file cyip_evtgen.h
*
* \brief
* EVTGEN IP definitions
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

#ifndef _CYIP_EVTGEN_H_
#define _CYIP_EVTGEN_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    EVTGEN
*******************************************************************************/

#define EVTGEN_COMP_STRUCT_SECTION_SIZE         0x00000020UL
#define EVTGEN_SECTION_SIZE                     0x00001000UL

/**
  * \brief Comparator control (EVTGEN_COMP_STRUCT_COMP_CTL)
  */
typedef struct stc_EVTGEN_COMP_STRUCT_COMP_CTL_field {
  uint32_t                         u1COMP0_EN:1;
  uint32_t                         u1COMP1_EN:1;
  uint32_t                         :14;
  uint32_t                         u1TR_OUT_EDGE:1;
  uint32_t                         :14;
  uint32_t                         u1ENABLED:1;
} stc_EVTGEN_COMP_STRUCT_COMP_CTL_field_t;

typedef union un_EVTGEN_COMP_STRUCT_COMP_CTL {
  uint32_t                         u32Register;
  stc_EVTGEN_COMP_STRUCT_COMP_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COMP_STRUCT_COMP_CTL_t;

/**
  * \brief Comparator 0 (Active functionality) (EVTGEN_COMP_STRUCT_COMP0)
  */
typedef struct stc_EVTGEN_COMP_STRUCT_COMP0_field {
  uint32_t                         u32INT32:32;
} stc_EVTGEN_COMP_STRUCT_COMP0_field_t;

typedef union un_EVTGEN_COMP_STRUCT_COMP0 {
  uint32_t                         u32Register;
  stc_EVTGEN_COMP_STRUCT_COMP0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COMP_STRUCT_COMP0_t;

/**
  * \brief Comparator 1 (DeepSleep functionality) (EVTGEN_COMP_STRUCT_COMP1)
  */
typedef struct stc_EVTGEN_COMP_STRUCT_COMP1_field {
  uint32_t                         u32INT32:32;
} stc_EVTGEN_COMP_STRUCT_COMP1_field_t;

typedef union un_EVTGEN_COMP_STRUCT_COMP1 {
  uint32_t                         u32Register;
  stc_EVTGEN_COMP_STRUCT_COMP1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COMP_STRUCT_COMP1_t;



/**
  * \brief Control (EVTGEN_CTL)
  */
typedef struct stc_EVTGEN_CTL_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLED:1;
} stc_EVTGEN_CTL_field_t;

typedef union un_EVTGEN_CTL {
  uint32_t                         u32Register;
  stc_EVTGEN_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_CTL_t;

/**
  * \brief Comparator structures comparator 0 status (EVTGEN_COMP0_STATUS)
  */
typedef struct stc_EVTGEN_COMP0_STATUS_field {
  uint32_t                         u16COMP0_OUT:16;
  uint32_t                         :16;
} stc_EVTGEN_COMP0_STATUS_field_t;

typedef union un_EVTGEN_COMP0_STATUS {
  uint32_t                         u32Register;
  stc_EVTGEN_COMP0_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COMP0_STATUS_t;

/**
  * \brief Comparator structures comparator 1 status (EVTGEN_COMP1_STATUS)
  */
typedef struct stc_EVTGEN_COMP1_STATUS_field {
  uint32_t                         u16COMP1_OUT:16;
  uint32_t                         :16;
} stc_EVTGEN_COMP1_STATUS_field_t;

typedef union un_EVTGEN_COMP1_STATUS {
  uint32_t                         u32Register;
  stc_EVTGEN_COMP1_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COMP1_STATUS_t;

/**
  * \brief Counter status (EVTGEN_COUNTER_STATUS)
  */
typedef struct stc_EVTGEN_COUNTER_STATUS_field {
  uint32_t                         :31;
  uint32_t                         u1VALID:1;
} stc_EVTGEN_COUNTER_STATUS_field_t;

typedef union un_EVTGEN_COUNTER_STATUS {
  uint32_t                         u32Register;
  stc_EVTGEN_COUNTER_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COUNTER_STATUS_t;

/**
  * \brief Counter (EVTGEN_COUNTER)
  */
typedef struct stc_EVTGEN_COUNTER_field {
  uint32_t                         u32INT32:32;
} stc_EVTGEN_COUNTER_field_t;

typedef union un_EVTGEN_COUNTER {
  uint32_t                         u32Register;
  stc_EVTGEN_COUNTER_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_COUNTER_t;

/**
  * \brief Ratio control (EVTGEN_RATIO_CTL)
  */
typedef struct stc_EVTGEN_RATIO_CTL_field {
  uint32_t                         :16;
  uint32_t                         u3DYNAMIC_MODE:3;
  uint32_t                         :11;
  uint32_t                         u1DYNAMIC:1;
  uint32_t                         u1VALID:1;
} stc_EVTGEN_RATIO_CTL_field_t;

typedef union un_EVTGEN_RATIO_CTL {
  uint32_t                         u32Register;
  stc_EVTGEN_RATIO_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_RATIO_CTL_t;

/**
  * \brief Ratio (EVTGEN_RATIO)
  */
typedef struct stc_EVTGEN_RATIO_field {
  uint32_t                         :8;
  uint32_t                         u8FRAC8:8;
  uint32_t                         u16INT16:16;
} stc_EVTGEN_RATIO_field_t;

typedef union un_EVTGEN_RATIO {
  uint32_t                         u32Register;
  stc_EVTGEN_RATIO_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_RATIO_t;

/**
  * \brief Reference clock control (EVTGEN_REF_CLOCK_CTL)
  */
typedef struct stc_EVTGEN_REF_CLOCK_CTL_field {
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :24;
} stc_EVTGEN_REF_CLOCK_CTL_field_t;

typedef union un_EVTGEN_REF_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_EVTGEN_REF_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_REF_CLOCK_CTL_t;

/**
  * \brief Interrupt (EVTGEN_INTR)
  */
typedef struct stc_EVTGEN_INTR_field {
  uint32_t                         u16COMP0:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_field_t;

typedef union un_EVTGEN_INTR {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_t;

/**
  * \brief Interrupt set (EVTGEN_INTR_SET)
  */
typedef struct stc_EVTGEN_INTR_SET_field {
  uint32_t                         u16COMP0:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_SET_field_t;

typedef union un_EVTGEN_INTR_SET {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_SET_t;

/**
  * \brief Interrupt mask (EVTGEN_INTR_MASK)
  */
typedef struct stc_EVTGEN_INTR_MASK_field {
  uint32_t                         u16COMP0:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_MASK_field_t;

typedef union un_EVTGEN_INTR_MASK {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_MASK_t;

/**
  * \brief Interrupt masked (EVTGEN_INTR_MASKED)
  */
typedef struct stc_EVTGEN_INTR_MASKED_field {
  uint32_t                         u16COMP0:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_MASKED_field_t;

typedef union un_EVTGEN_INTR_MASKED {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_MASKED_t;

/**
  * \brief DeepSleep interrupt (EVTGEN_INTR_DPSLP)
  */
typedef struct stc_EVTGEN_INTR_DPSLP_field {
  uint32_t                         u16COMP1:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_DPSLP_field_t;

typedef union un_EVTGEN_INTR_DPSLP {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_DPSLP_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_DPSLP_t;

/**
  * \brief DeepSleep interrupt set (EVTGEN_INTR_DPSLP_SET)
  */
typedef struct stc_EVTGEN_INTR_DPSLP_SET_field {
  uint32_t                         u16COMP1:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_DPSLP_SET_field_t;

typedef union un_EVTGEN_INTR_DPSLP_SET {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_DPSLP_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_DPSLP_SET_t;

/**
  * \brief DeepSleep interrupt mask (EVTGEN_INTR_DPSLP_MASK)
  */
typedef struct stc_EVTGEN_INTR_DPSLP_MASK_field {
  uint32_t                         u16COMP1:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_DPSLP_MASK_field_t;

typedef union un_EVTGEN_INTR_DPSLP_MASK {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_DPSLP_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_DPSLP_MASK_t;

/**
  * \brief DeepSleep interrupt masked (EVTGEN_INTR_DPSLP_MASKED)
  */
typedef struct stc_EVTGEN_INTR_DPSLP_MASKED_field {
  uint32_t                         u16COMP1:16;
  uint32_t                         :16;
} stc_EVTGEN_INTR_DPSLP_MASKED_field_t;

typedef union un_EVTGEN_INTR_DPSLP_MASKED {
  uint32_t                         u32Register;
  stc_EVTGEN_INTR_DPSLP_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_EVTGEN_INTR_DPSLP_MASKED_t;



/**
  * \brief Comparator structure (EVTGEN_COMP_STRUCT)
  */
typedef struct stc_EVTGEN_COMP_STRUCT {
  un_EVTGEN_COMP_STRUCT_COMP_CTL_t unCOMP_CTL;         /*!< 0x00000000 Comparator control */
  un_EVTGEN_COMP_STRUCT_COMP0_t    unCOMP0;            /*!< 0x00000004 Comparator 0 (Active functionality) */
  un_EVTGEN_COMP_STRUCT_COMP1_t    unCOMP1;            /*!< 0x00000008 Comparator 1 (DeepSleep functionality) */
  uint32_t                         au32Reserved[5];
} stc_EVTGEN_COMP_STRUCT_t;                            /*!< Size = 32 (0x20) */

/**
  * \brief Event generator (EVTGEN)
  */
typedef struct stc_EVTGEN {
  un_EVTGEN_CTL_t                  unCTL;              /*!< 0x00000000 Control */
  un_EVTGEN_COMP0_STATUS_t         unCOMP0_STATUS;     /*!< 0x00000004 Comparator structures comparator 0 status */
  un_EVTGEN_COMP1_STATUS_t         unCOMP1_STATUS;     /*!< 0x00000008 Comparator structures comparator 1 status */
  uint32_t                         u32Reserved;
  un_EVTGEN_COUNTER_STATUS_t       unCOUNTER_STATUS;   /*!< 0x00000010 Counter status */
  un_EVTGEN_COUNTER_t              unCOUNTER;          /*!< 0x00000014 Counter */
  uint32_t                         au32Reserved1[2];
  un_EVTGEN_RATIO_CTL_t            unRATIO_CTL;        /*!< 0x00000020 Ratio control */
  un_EVTGEN_RATIO_t                unRATIO;            /*!< 0x00000024 Ratio */
  uint32_t                         au32Reserved2[2];
  un_EVTGEN_REF_CLOCK_CTL_t        unREF_CLOCK_CTL;    /*!< 0x00000030 Reference clock control */
  uint32_t                         au32Reserved3[435];
  un_EVTGEN_INTR_t                 unINTR;             /*!< 0x00000700 Interrupt */
  un_EVTGEN_INTR_SET_t             unINTR_SET;         /*!< 0x00000704 Interrupt set */
  un_EVTGEN_INTR_MASK_t            unINTR_MASK;        /*!< 0x00000708 Interrupt mask */
  un_EVTGEN_INTR_MASKED_t          unINTR_MASKED;      /*!< 0x0000070C Interrupt masked */
  un_EVTGEN_INTR_DPSLP_t           unINTR_DPSLP;       /*!< 0x00000710 DeepSleep interrupt */
  un_EVTGEN_INTR_DPSLP_SET_t       unINTR_DPSLP_SET;   /*!< 0x00000714 DeepSleep interrupt set */
  un_EVTGEN_INTR_DPSLP_MASK_t      unINTR_DPSLP_MASK;  /*!< 0x00000718 DeepSleep interrupt mask */
  un_EVTGEN_INTR_DPSLP_MASKED_t    unINTR_DPSLP_MASKED; /*!< 0x0000071C DeepSleep interrupt masked */
  uint32_t                         au32Reserved4[56];
  stc_EVTGEN_COMP_STRUCT_t         COMP_STRUCT[32];    /*!< 0x00000800 Comparator structure */
} stc_EVTGEN_t;                                        /*!< Size = 3072 (0xC00) */


/* EVTGEN_COMP_STRUCT.COMP_CTL */
#define EVTGEN_COMP_STRUCT_COMP_CTL_COMP0_EN_Pos 0UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_COMP0_EN_Msk 0x1UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_COMP1_EN_Pos 1UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_COMP1_EN_Msk 0x2UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_TR_OUT_EDGE_Pos 16UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_TR_OUT_EDGE_Msk 0x10000UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_ENABLED_Pos 31UL
#define EVTGEN_COMP_STRUCT_COMP_CTL_ENABLED_Msk 0x80000000UL
/* EVTGEN_COMP_STRUCT.COMP0 */
#define EVTGEN_COMP_STRUCT_COMP0_INT32_Pos      0UL
#define EVTGEN_COMP_STRUCT_COMP0_INT32_Msk      0xFFFFFFFFUL
/* EVTGEN_COMP_STRUCT.COMP1 */
#define EVTGEN_COMP_STRUCT_COMP1_INT32_Pos      0UL
#define EVTGEN_COMP_STRUCT_COMP1_INT32_Msk      0xFFFFFFFFUL


/* EVTGEN.CTL */
#define EVTGEN_CTL_ENABLED_Pos                  31UL
#define EVTGEN_CTL_ENABLED_Msk                  0x80000000UL
/* EVTGEN.COMP0_STATUS */
#define EVTGEN_COMP0_STATUS_COMP0_OUT_Pos       0UL
#define EVTGEN_COMP0_STATUS_COMP0_OUT_Msk       0xFFFFUL
/* EVTGEN.COMP1_STATUS */
#define EVTGEN_COMP1_STATUS_COMP1_OUT_Pos       0UL
#define EVTGEN_COMP1_STATUS_COMP1_OUT_Msk       0xFFFFUL
/* EVTGEN.COUNTER_STATUS */
#define EVTGEN_COUNTER_STATUS_VALID_Pos         31UL
#define EVTGEN_COUNTER_STATUS_VALID_Msk         0x80000000UL
/* EVTGEN.COUNTER */
#define EVTGEN_COUNTER_INT32_Pos                0UL
#define EVTGEN_COUNTER_INT32_Msk                0xFFFFFFFFUL
/* EVTGEN.RATIO_CTL */
#define EVTGEN_RATIO_CTL_DYNAMIC_MODE_Pos       16UL
#define EVTGEN_RATIO_CTL_DYNAMIC_MODE_Msk       0x70000UL
#define EVTGEN_RATIO_CTL_DYNAMIC_Pos            30UL
#define EVTGEN_RATIO_CTL_DYNAMIC_Msk            0x40000000UL
#define EVTGEN_RATIO_CTL_VALID_Pos              31UL
#define EVTGEN_RATIO_CTL_VALID_Msk              0x80000000UL
/* EVTGEN.RATIO */
#define EVTGEN_RATIO_FRAC8_Pos                  8UL
#define EVTGEN_RATIO_FRAC8_Msk                  0xFF00UL
#define EVTGEN_RATIO_INT16_Pos                  16UL
#define EVTGEN_RATIO_INT16_Msk                  0xFFFF0000UL
/* EVTGEN.REF_CLOCK_CTL */
#define EVTGEN_REF_CLOCK_CTL_INT_DIV_Pos        0UL
#define EVTGEN_REF_CLOCK_CTL_INT_DIV_Msk        0xFFUL
/* EVTGEN.INTR */
#define EVTGEN_INTR_COMP0_Pos                   0UL
#define EVTGEN_INTR_COMP0_Msk                   0xFFFFUL
/* EVTGEN.INTR_SET */
#define EVTGEN_INTR_SET_COMP0_Pos               0UL
#define EVTGEN_INTR_SET_COMP0_Msk               0xFFFFUL
/* EVTGEN.INTR_MASK */
#define EVTGEN_INTR_MASK_COMP0_Pos              0UL
#define EVTGEN_INTR_MASK_COMP0_Msk              0xFFFFUL
/* EVTGEN.INTR_MASKED */
#define EVTGEN_INTR_MASKED_COMP0_Pos            0UL
#define EVTGEN_INTR_MASKED_COMP0_Msk            0xFFFFUL
/* EVTGEN.INTR_DPSLP */
#define EVTGEN_INTR_DPSLP_COMP1_Pos             0UL
#define EVTGEN_INTR_DPSLP_COMP1_Msk             0xFFFFUL
/* EVTGEN.INTR_DPSLP_SET */
#define EVTGEN_INTR_DPSLP_SET_COMP1_Pos         0UL
#define EVTGEN_INTR_DPSLP_SET_COMP1_Msk         0xFFFFUL
/* EVTGEN.INTR_DPSLP_MASK */
#define EVTGEN_INTR_DPSLP_MASK_COMP1_Pos        0UL
#define EVTGEN_INTR_DPSLP_MASK_COMP1_Msk        0xFFFFUL
/* EVTGEN.INTR_DPSLP_MASKED */
#define EVTGEN_INTR_DPSLP_MASKED_COMP1_Pos      0UL
#define EVTGEN_INTR_DPSLP_MASKED_COMP1_Msk      0xFFFFUL


#endif /* _CYIP_EVTGEN_H_ */


/* [] END OF FILE */
