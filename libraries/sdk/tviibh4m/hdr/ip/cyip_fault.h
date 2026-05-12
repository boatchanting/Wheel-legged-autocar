/***************************************************************************//**
* \file cyip_fault.h
*
* \brief
* FAULT IP definitions
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

#ifndef _CYIP_FAULT_H_
#define _CYIP_FAULT_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    FAULT
*******************************************************************************/

#define FAULT_STRUCT_SECTION_SIZE               0x00000100UL
#define FAULT_SECTION_SIZE                      0x00010000UL

/**
  * \brief Fault control (FAULT_STRUCT_CTL)
  */
typedef struct stc_FAULT_STRUCT_CTL_field {
  uint32_t                         u1TR_EN:1;
  uint32_t                         u1OUT_EN:1;
  uint32_t                         u1RESET_REQ_EN:1;
  uint32_t                         :29;
} stc_FAULT_STRUCT_CTL_field_t;

typedef union un_FAULT_STRUCT_CTL {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_CTL_t;

/**
  * \brief Fault status (FAULT_STRUCT_STATUS)
  */
typedef struct stc_FAULT_STRUCT_STATUS_field {
  uint32_t                         u7IDX:7;
  uint32_t                         :24;
  uint32_t                         u1VALID:1;
} stc_FAULT_STRUCT_STATUS_field_t;

typedef union un_FAULT_STRUCT_STATUS {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_STATUS_t;

/**
  * \brief Fault data (FAULT_STRUCT_DATA)
  */
typedef struct stc_FAULT_STRUCT_DATA_field {
  uint32_t                         u32DATA:32;
} stc_FAULT_STRUCT_DATA_field_t;

typedef union un_FAULT_STRUCT_DATA {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_DATA_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_DATA_t;

/**
  * \brief Fault pending 0 (FAULT_STRUCT_PENDING0)
  */
typedef struct stc_FAULT_STRUCT_PENDING0_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_PENDING0_field_t;

typedef union un_FAULT_STRUCT_PENDING0 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_PENDING0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_PENDING0_t;

/**
  * \brief Fault pending 1 (FAULT_STRUCT_PENDING1)
  */
typedef struct stc_FAULT_STRUCT_PENDING1_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_PENDING1_field_t;

typedef union un_FAULT_STRUCT_PENDING1 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_PENDING1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_PENDING1_t;

/**
  * \brief Fault pending 2 (FAULT_STRUCT_PENDING2)
  */
typedef struct stc_FAULT_STRUCT_PENDING2_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_PENDING2_field_t;

typedef union un_FAULT_STRUCT_PENDING2 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_PENDING2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_PENDING2_t;

/**
  * \brief Fault mask 0 (FAULT_STRUCT_MASK0)
  */
typedef struct stc_FAULT_STRUCT_MASK0_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_MASK0_field_t;

typedef union un_FAULT_STRUCT_MASK0 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_MASK0_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_MASK0_t;

/**
  * \brief Fault mask 1 (FAULT_STRUCT_MASK1)
  */
typedef struct stc_FAULT_STRUCT_MASK1_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_MASK1_field_t;

typedef union un_FAULT_STRUCT_MASK1 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_MASK1_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_MASK1_t;

/**
  * \brief Fault mask 2 (FAULT_STRUCT_MASK2)
  */
typedef struct stc_FAULT_STRUCT_MASK2_field {
  uint32_t                         u32SOURCE:32;
} stc_FAULT_STRUCT_MASK2_field_t;

typedef union un_FAULT_STRUCT_MASK2 {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_MASK2_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_MASK2_t;

/**
  * \brief Interrupt (FAULT_STRUCT_INTR)
  */
typedef struct stc_FAULT_STRUCT_INTR_field {
  uint32_t                         u1FAULT:1;
  uint32_t                         :31;
} stc_FAULT_STRUCT_INTR_field_t;

typedef union un_FAULT_STRUCT_INTR {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_INTR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_INTR_t;

/**
  * \brief Interrupt set (FAULT_STRUCT_INTR_SET)
  */
typedef struct stc_FAULT_STRUCT_INTR_SET_field {
  uint32_t                         u1FAULT:1;
  uint32_t                         :31;
} stc_FAULT_STRUCT_INTR_SET_field_t;

typedef union un_FAULT_STRUCT_INTR_SET {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_INTR_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_INTR_SET_t;

/**
  * \brief Interrupt mask (FAULT_STRUCT_INTR_MASK)
  */
typedef struct stc_FAULT_STRUCT_INTR_MASK_field {
  uint32_t                         u1FAULT:1;
  uint32_t                         :31;
} stc_FAULT_STRUCT_INTR_MASK_field_t;

typedef union un_FAULT_STRUCT_INTR_MASK {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_INTR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_INTR_MASK_t;

/**
  * \brief Interrupt masked (FAULT_STRUCT_INTR_MASKED)
  */
typedef struct stc_FAULT_STRUCT_INTR_MASKED_field {
  uint32_t                         u1FAULT:1;
  uint32_t                         :31;
} stc_FAULT_STRUCT_INTR_MASKED_field_t;

typedef union un_FAULT_STRUCT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_FAULT_STRUCT_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_FAULT_STRUCT_INTR_MASKED_t;



/**
  * \brief Fault structure (FAULT_STRUCT)
  */
typedef struct stc_FAULT_STRUCT {
  un_FAULT_STRUCT_CTL_t            unCTL;              /*!< 0x00000000 Fault control */
  uint32_t                         au32Reserved[2];
  un_FAULT_STRUCT_STATUS_t         unSTATUS;           /*!< 0x0000000C Fault status */
  un_FAULT_STRUCT_DATA_t           unDATA[4];          /*!< 0x00000010 Fault data */
  uint32_t                         au32Reserved1[8];
  un_FAULT_STRUCT_PENDING0_t       unPENDING0;         /*!< 0x00000040 Fault pending 0 */
  un_FAULT_STRUCT_PENDING1_t       unPENDING1;         /*!< 0x00000044 Fault pending 1 */
  un_FAULT_STRUCT_PENDING2_t       unPENDING2;         /*!< 0x00000048 Fault pending 2 */
  uint32_t                         u32Reserved2;
  un_FAULT_STRUCT_MASK0_t          unMASK0;            /*!< 0x00000050 Fault mask 0 */
  un_FAULT_STRUCT_MASK1_t          unMASK1;            /*!< 0x00000054 Fault mask 1 */
  un_FAULT_STRUCT_MASK2_t          unMASK2;            /*!< 0x00000058 Fault mask 2 */
  uint32_t                         au32Reserved3[25];
  un_FAULT_STRUCT_INTR_t           unINTR;             /*!< 0x000000C0 Interrupt */
  un_FAULT_STRUCT_INTR_SET_t       unINTR_SET;         /*!< 0x000000C4 Interrupt set */
  un_FAULT_STRUCT_INTR_MASK_t      unINTR_MASK;        /*!< 0x000000C8 Interrupt mask */
  un_FAULT_STRUCT_INTR_MASKED_t    unINTR_MASKED;      /*!< 0x000000CC Interrupt masked */
  uint32_t                         au32Reserved4[12];
} stc_FAULT_STRUCT_t;                                  /*!< Size = 256 (0x100) */

/**
  * \brief Fault structures (FAULT)
  */
typedef struct stc_FAULT {
  stc_FAULT_STRUCT_t               STRUCT[4];          /*!< 0x00000000 Fault structure */
} stc_FAULT_t;                                         /*!< Size = 1024 (0x400) */


/* FAULT_STRUCT.CTL */
#define FAULT_STRUCT_CTL_TR_EN_Pos              0UL
#define FAULT_STRUCT_CTL_TR_EN_Msk              0x1UL
#define FAULT_STRUCT_CTL_OUT_EN_Pos             1UL
#define FAULT_STRUCT_CTL_OUT_EN_Msk             0x2UL
#define FAULT_STRUCT_CTL_RESET_REQ_EN_Pos       2UL
#define FAULT_STRUCT_CTL_RESET_REQ_EN_Msk       0x4UL
/* FAULT_STRUCT.STATUS */
#define FAULT_STRUCT_STATUS_IDX_Pos             0UL
#define FAULT_STRUCT_STATUS_IDX_Msk             0x7FUL
#define FAULT_STRUCT_STATUS_VALID_Pos           31UL
#define FAULT_STRUCT_STATUS_VALID_Msk           0x80000000UL
/* FAULT_STRUCT.DATA */
#define FAULT_STRUCT_DATA_DATA_Pos              0UL
#define FAULT_STRUCT_DATA_DATA_Msk              0xFFFFFFFFUL
/* FAULT_STRUCT.PENDING0 */
#define FAULT_STRUCT_PENDING0_SOURCE_Pos        0UL
#define FAULT_STRUCT_PENDING0_SOURCE_Msk        0xFFFFFFFFUL
/* FAULT_STRUCT.PENDING1 */
#define FAULT_STRUCT_PENDING1_SOURCE_Pos        0UL
#define FAULT_STRUCT_PENDING1_SOURCE_Msk        0xFFFFFFFFUL
/* FAULT_STRUCT.PENDING2 */
#define FAULT_STRUCT_PENDING2_SOURCE_Pos        0UL
#define FAULT_STRUCT_PENDING2_SOURCE_Msk        0xFFFFFFFFUL
/* FAULT_STRUCT.MASK0 */
#define FAULT_STRUCT_MASK0_SOURCE_Pos           0UL
#define FAULT_STRUCT_MASK0_SOURCE_Msk           0xFFFFFFFFUL
/* FAULT_STRUCT.MASK1 */
#define FAULT_STRUCT_MASK1_SOURCE_Pos           0UL
#define FAULT_STRUCT_MASK1_SOURCE_Msk           0xFFFFFFFFUL
/* FAULT_STRUCT.MASK2 */
#define FAULT_STRUCT_MASK2_SOURCE_Pos           0UL
#define FAULT_STRUCT_MASK2_SOURCE_Msk           0xFFFFFFFFUL
/* FAULT_STRUCT.INTR */
#define FAULT_STRUCT_INTR_FAULT_Pos             0UL
#define FAULT_STRUCT_INTR_FAULT_Msk             0x1UL
/* FAULT_STRUCT.INTR_SET */
#define FAULT_STRUCT_INTR_SET_FAULT_Pos         0UL
#define FAULT_STRUCT_INTR_SET_FAULT_Msk         0x1UL
/* FAULT_STRUCT.INTR_MASK */
#define FAULT_STRUCT_INTR_MASK_FAULT_Pos        0UL
#define FAULT_STRUCT_INTR_MASK_FAULT_Msk        0x1UL
/* FAULT_STRUCT.INTR_MASKED */
#define FAULT_STRUCT_INTR_MASKED_FAULT_Pos      0UL
#define FAULT_STRUCT_INTR_MASKED_FAULT_Msk      0x1UL


#endif /* _CYIP_FAULT_H_ */


/* [] END OF FILE */
