/***************************************************************************//**
* \file cyip_ipc.h
*
* \brief
* IPC IP definitions
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

#ifndef _CYIP_IPC_H_
#define _CYIP_IPC_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     IPC
*******************************************************************************/

#define IPC_STRUCT_SECTION_SIZE                 0x00000020UL
#define IPC_INTR_STRUCT_SECTION_SIZE            0x00000020UL
#define IPC_SECTION_SIZE                        0x00010000UL

/**
  * \brief IPC acquire (IPC_STRUCT_ACQUIRE)
  */
typedef struct stc_IPC_STRUCT_ACQUIRE_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :2;
  uint32_t                         u4PC:4;
  uint32_t                         u4MS:4;
  uint32_t                         :19;
  uint32_t                         u1SUCCESS:1;
} stc_IPC_STRUCT_ACQUIRE_field_t;

typedef union un_IPC_STRUCT_ACQUIRE {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_ACQUIRE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_ACQUIRE_t;

/**
  * \brief IPC release (IPC_STRUCT_RELEASE)
  */
typedef struct stc_IPC_STRUCT_RELEASE_field {
  uint32_t                         u16INTR_RELEASE:16;
  uint32_t                         :16;
} stc_IPC_STRUCT_RELEASE_field_t;

typedef union un_IPC_STRUCT_RELEASE {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_RELEASE_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_RELEASE_t;

/**
  * \brief IPC notification (IPC_STRUCT_NOTIFY)
  */
typedef struct stc_IPC_STRUCT_NOTIFY_field {
  uint32_t                         u16INTR_NOTIFY:16;
  uint32_t                         :16;
} stc_IPC_STRUCT_NOTIFY_field_t;

typedef union un_IPC_STRUCT_NOTIFY {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_NOTIFY_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_NOTIFY_t;

/**
  * \brief IPC data 0 (IPC_STRUCT_DATA0)
  */
typedef struct stc_IPC_STRUCT_DATA0_field {
  uint32_t                         u32DATA:32;
} stc_IPC_STRUCT_DATA0_field_t;

typedef union un_IPC_STRUCT_DATA0 {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_DATA0_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_DATA0_t;

/**
  * \brief IPC data 1 (IPC_STRUCT_DATA1)
  */
typedef struct stc_IPC_STRUCT_DATA1_field {
  uint32_t                         u32DATA:32;
} stc_IPC_STRUCT_DATA1_field_t;

typedef union un_IPC_STRUCT_DATA1 {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_DATA1_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_DATA1_t;

/**
  * \brief IPC lock status (IPC_STRUCT_LOCK_STATUS)
  */
typedef struct stc_IPC_STRUCT_LOCK_STATUS_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :2;
  uint32_t                         u4PC:4;
  uint32_t                         u4MS:4;
  uint32_t                         :19;
  uint32_t                         u1ACQUIRED:1;
} stc_IPC_STRUCT_LOCK_STATUS_field_t;

typedef union un_IPC_STRUCT_LOCK_STATUS {
  uint32_t                         u32Register;
  stc_IPC_STRUCT_LOCK_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_STRUCT_LOCK_STATUS_t;



/**
  * \brief Interrupt (IPC_INTR_STRUCT_INTR)
  */
typedef struct stc_IPC_INTR_STRUCT_INTR_field {
  uint32_t                         u16RELEASE:16;
  uint32_t                         u16NOTIFY:16;
} stc_IPC_INTR_STRUCT_INTR_field_t;

typedef union un_IPC_INTR_STRUCT_INTR {
  uint32_t                         u32Register;
  stc_IPC_INTR_STRUCT_INTR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_INTR_STRUCT_INTR_t;

/**
  * \brief Interrupt set (IPC_INTR_STRUCT_INTR_SET)
  */
typedef struct stc_IPC_INTR_STRUCT_INTR_SET_field {
  uint32_t                         u16RELEASE:16;
  uint32_t                         u16NOTIFY:16;
} stc_IPC_INTR_STRUCT_INTR_SET_field_t;

typedef union un_IPC_INTR_STRUCT_INTR_SET {
  uint32_t                         u32Register;
  stc_IPC_INTR_STRUCT_INTR_SET_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_INTR_STRUCT_INTR_SET_t;

/**
  * \brief Interrupt mask (IPC_INTR_STRUCT_INTR_MASK)
  */
typedef struct stc_IPC_INTR_STRUCT_INTR_MASK_field {
  uint32_t                         u16RELEASE:16;
  uint32_t                         u16NOTIFY:16;
} stc_IPC_INTR_STRUCT_INTR_MASK_field_t;

typedef union un_IPC_INTR_STRUCT_INTR_MASK {
  uint32_t                         u32Register;
  stc_IPC_INTR_STRUCT_INTR_MASK_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_INTR_STRUCT_INTR_MASK_t;

/**
  * \brief Interrupt masked (IPC_INTR_STRUCT_INTR_MASKED)
  */
typedef struct stc_IPC_INTR_STRUCT_INTR_MASKED_field {
  uint32_t                         u16RELEASE:16;
  uint32_t                         u16NOTIFY:16;
} stc_IPC_INTR_STRUCT_INTR_MASKED_field_t;

typedef union un_IPC_INTR_STRUCT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_IPC_INTR_STRUCT_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_IPC_INTR_STRUCT_INTR_MASKED_t;



/**
  * \brief IPC structure (IPC_STRUCT)
  */
typedef struct stc_IPC_STRUCT {
  un_IPC_STRUCT_ACQUIRE_t          unACQUIRE;          /*!< 0x00000000 IPC acquire */
  un_IPC_STRUCT_RELEASE_t          unRELEASE;          /*!< 0x00000004 IPC release */
  un_IPC_STRUCT_NOTIFY_t           unNOTIFY;           /*!< 0x00000008 IPC notification */
  un_IPC_STRUCT_DATA0_t            unDATA0;            /*!< 0x0000000C IPC data 0 */
  un_IPC_STRUCT_DATA1_t            unDATA1;            /*!< 0x00000010 IPC data 1 */
  uint32_t                         au32Reserved[2];
  un_IPC_STRUCT_LOCK_STATUS_t      unLOCK_STATUS;      /*!< 0x0000001C IPC lock status */
} stc_IPC_STRUCT_t;                                    /*!< Size = 32 (0x20) */

/**
  * \brief IPC interrupt structure (IPC_INTR_STRUCT)
  */
typedef struct stc_IPC_INTR_STRUCT {
  un_IPC_INTR_STRUCT_INTR_t        unINTR;             /*!< 0x00000000 Interrupt */
  un_IPC_INTR_STRUCT_INTR_SET_t    unINTR_SET;         /*!< 0x00000004 Interrupt set */
  un_IPC_INTR_STRUCT_INTR_MASK_t   unINTR_MASK;        /*!< 0x00000008 Interrupt mask */
  un_IPC_INTR_STRUCT_INTR_MASKED_t unINTR_MASKED;      /*!< 0x0000000C Interrupt masked */
  uint32_t                         au32Reserved[4];
} stc_IPC_INTR_STRUCT_t;                               /*!< Size = 32 (0x20) */

/**
  * \brief IPC (IPC)
  */
typedef struct stc_IPC {
  stc_IPC_STRUCT_t                 STRUCT[16];         /*!< 0x00000000 IPC structure */
  uint32_t                         au32Reserved[896];
  stc_IPC_INTR_STRUCT_t            INTR_STRUCT[16];    /*!< 0x00001000 IPC interrupt structure */
} stc_IPC_t;                                           /*!< Size = 4608 (0x1200) */


/* IPC_STRUCT.ACQUIRE */
#define IPC_STRUCT_ACQUIRE_P_Pos                0UL
#define IPC_STRUCT_ACQUIRE_P_Msk                0x1UL
#define IPC_STRUCT_ACQUIRE_NS_Pos               1UL
#define IPC_STRUCT_ACQUIRE_NS_Msk               0x2UL
#define IPC_STRUCT_ACQUIRE_PC_Pos               4UL
#define IPC_STRUCT_ACQUIRE_PC_Msk               0xF0UL
#define IPC_STRUCT_ACQUIRE_MS_Pos               8UL
#define IPC_STRUCT_ACQUIRE_MS_Msk               0xF00UL
#define IPC_STRUCT_ACQUIRE_SUCCESS_Pos          31UL
#define IPC_STRUCT_ACQUIRE_SUCCESS_Msk          0x80000000UL
/* IPC_STRUCT.RELEASE */
#define IPC_STRUCT_RELEASE_INTR_RELEASE_Pos     0UL
#define IPC_STRUCT_RELEASE_INTR_RELEASE_Msk     0xFFFFUL
/* IPC_STRUCT.NOTIFY */
#define IPC_STRUCT_NOTIFY_INTR_NOTIFY_Pos       0UL
#define IPC_STRUCT_NOTIFY_INTR_NOTIFY_Msk       0xFFFFUL
/* IPC_STRUCT.DATA0 */
#define IPC_STRUCT_DATA0_DATA_Pos               0UL
#define IPC_STRUCT_DATA0_DATA_Msk               0xFFFFFFFFUL
/* IPC_STRUCT.DATA1 */
#define IPC_STRUCT_DATA1_DATA_Pos               0UL
#define IPC_STRUCT_DATA1_DATA_Msk               0xFFFFFFFFUL
/* IPC_STRUCT.LOCK_STATUS */
#define IPC_STRUCT_LOCK_STATUS_P_Pos            0UL
#define IPC_STRUCT_LOCK_STATUS_P_Msk            0x1UL
#define IPC_STRUCT_LOCK_STATUS_NS_Pos           1UL
#define IPC_STRUCT_LOCK_STATUS_NS_Msk           0x2UL
#define IPC_STRUCT_LOCK_STATUS_PC_Pos           4UL
#define IPC_STRUCT_LOCK_STATUS_PC_Msk           0xF0UL
#define IPC_STRUCT_LOCK_STATUS_MS_Pos           8UL
#define IPC_STRUCT_LOCK_STATUS_MS_Msk           0xF00UL
#define IPC_STRUCT_LOCK_STATUS_ACQUIRED_Pos     31UL
#define IPC_STRUCT_LOCK_STATUS_ACQUIRED_Msk     0x80000000UL


/* IPC_INTR_STRUCT.INTR */
#define IPC_INTR_STRUCT_INTR_RELEASE_Pos        0UL
#define IPC_INTR_STRUCT_INTR_RELEASE_Msk        0xFFFFUL
#define IPC_INTR_STRUCT_INTR_NOTIFY_Pos         16UL
#define IPC_INTR_STRUCT_INTR_NOTIFY_Msk         0xFFFF0000UL
/* IPC_INTR_STRUCT.INTR_SET */
#define IPC_INTR_STRUCT_INTR_SET_RELEASE_Pos    0UL
#define IPC_INTR_STRUCT_INTR_SET_RELEASE_Msk    0xFFFFUL
#define IPC_INTR_STRUCT_INTR_SET_NOTIFY_Pos     16UL
#define IPC_INTR_STRUCT_INTR_SET_NOTIFY_Msk     0xFFFF0000UL
/* IPC_INTR_STRUCT.INTR_MASK */
#define IPC_INTR_STRUCT_INTR_MASK_RELEASE_Pos   0UL
#define IPC_INTR_STRUCT_INTR_MASK_RELEASE_Msk   0xFFFFUL
#define IPC_INTR_STRUCT_INTR_MASK_NOTIFY_Pos    16UL
#define IPC_INTR_STRUCT_INTR_MASK_NOTIFY_Msk    0xFFFF0000UL
/* IPC_INTR_STRUCT.INTR_MASKED */
#define IPC_INTR_STRUCT_INTR_MASKED_RELEASE_Pos 0UL
#define IPC_INTR_STRUCT_INTR_MASKED_RELEASE_Msk 0xFFFFUL
#define IPC_INTR_STRUCT_INTR_MASKED_NOTIFY_Pos  16UL
#define IPC_INTR_STRUCT_INTR_MASKED_NOTIFY_Msk  0xFFFF0000UL


#endif /* _CYIP_IPC_H_ */


/* [] END OF FILE */
