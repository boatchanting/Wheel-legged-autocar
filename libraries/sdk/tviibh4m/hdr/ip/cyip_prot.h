/***************************************************************************//**
* \file cyip_prot.h
*
* \brief
* PROT IP definitions
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

#ifndef _CYIP_PROT_H_
#define _CYIP_PROT_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     PROT
*******************************************************************************/

#define PROT_SMPU_SMPU_STRUCT_SECTION_SIZE      0x00000040UL
#define PROT_SMPU_SECTION_SIZE                  0x00004000UL
#define PROT_MPU_MPU_STRUCT_SECTION_SIZE        0x00000020UL
#define PROT_MPU_SECTION_SIZE                   0x00000400UL
#define PROT_SECTION_SIZE                       0x00010000UL

/**
  * \brief SMPU region address 0 (slave structure) (PROT_SMPU_SMPU_STRUCT_ADDR0)
  */
typedef struct stc_PROT_SMPU_SMPU_STRUCT_ADDR0_field {
  uint32_t                         u8SUBREGION_DISABLE:8;
  uint32_t                         u24ADDR24:24;
} stc_PROT_SMPU_SMPU_STRUCT_ADDR0_field_t;

typedef union un_PROT_SMPU_SMPU_STRUCT_ADDR0 {
  uint32_t                         u32Register;
  stc_PROT_SMPU_SMPU_STRUCT_ADDR0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_SMPU_STRUCT_ADDR0_t;

/**
  * \brief SMPU region attributes 0 (slave structure) (PROT_SMPU_SMPU_STRUCT_ATT0)
  */
typedef struct stc_PROT_SMPU_SMPU_STRUCT_ATT0_field {
  uint32_t                         u1UR:1;
  uint32_t                         u1UW:1;
  uint32_t                         u1UX:1;
  uint32_t                         u1PR:1;
  uint32_t                         u1PW:1;
  uint32_t                         u1PX:1;
  uint32_t                         u1NS:1;
  uint32_t                         :1;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :1;
  uint32_t                         u1PC_MATCH:1;
  uint32_t                         u1ENABLED:1;
} stc_PROT_SMPU_SMPU_STRUCT_ATT0_field_t;

typedef union un_PROT_SMPU_SMPU_STRUCT_ATT0 {
  uint32_t                         u32Register;
  stc_PROT_SMPU_SMPU_STRUCT_ATT0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_SMPU_STRUCT_ATT0_t;

/**
  * \brief SMPU region address 1 (master structure) (PROT_SMPU_SMPU_STRUCT_ADDR1)
  */
typedef struct stc_PROT_SMPU_SMPU_STRUCT_ADDR1_field {
  uint32_t                         u8SUBREGION_DISABLE:8;
  uint32_t                         u24ADDR24:24;
} stc_PROT_SMPU_SMPU_STRUCT_ADDR1_field_t;

typedef union un_PROT_SMPU_SMPU_STRUCT_ADDR1 {
  uint32_t                         u32Register;
  stc_PROT_SMPU_SMPU_STRUCT_ADDR1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_SMPU_STRUCT_ADDR1_t;

/**
  * \brief SMPU region attributes 1 (master structure) (PROT_SMPU_SMPU_STRUCT_ATT1)
  */
typedef struct stc_PROT_SMPU_SMPU_STRUCT_ATT1_field {
  uint32_t                         u1UR:1;
  uint32_t                         u1UW:1;
  uint32_t                         u1UX:1;
  uint32_t                         u1PR:1;
  uint32_t                         u1PW:1;
  uint32_t                         u1PX:1;
  uint32_t                         u1NS:1;
  uint32_t                         :1;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :1;
  uint32_t                         u1PC_MATCH:1;
  uint32_t                         u1ENABLED:1;
} stc_PROT_SMPU_SMPU_STRUCT_ATT1_field_t;

typedef union un_PROT_SMPU_SMPU_STRUCT_ATT1 {
  uint32_t                         u32Register;
  stc_PROT_SMPU_SMPU_STRUCT_ATT1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_SMPU_STRUCT_ATT1_t;



/**
  * \brief Master 0 protection context control (PROT_SMPU_MS0_CTL)
  */
typedef struct stc_PROT_SMPU_MS0_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS0_CTL_field_t;

typedef union un_PROT_SMPU_MS0_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS0_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS0_CTL_t;

/**
  * \brief Master 1 protection context control (PROT_SMPU_MS1_CTL)
  */
typedef struct stc_PROT_SMPU_MS1_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS1_CTL_field_t;

typedef union un_PROT_SMPU_MS1_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS1_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS1_CTL_t;

/**
  * \brief Master 2 protection context control (PROT_SMPU_MS2_CTL)
  */
typedef struct stc_PROT_SMPU_MS2_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS2_CTL_field_t;

typedef union un_PROT_SMPU_MS2_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS2_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS2_CTL_t;

/**
  * \brief Master 3 protection context control (PROT_SMPU_MS3_CTL)
  */
typedef struct stc_PROT_SMPU_MS3_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS3_CTL_field_t;

typedef union un_PROT_SMPU_MS3_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS3_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS3_CTL_t;

/**
  * \brief Master 4 protection context control (PROT_SMPU_MS4_CTL)
  */
typedef struct stc_PROT_SMPU_MS4_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS4_CTL_field_t;

typedef union un_PROT_SMPU_MS4_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS4_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS4_CTL_t;

/**
  * \brief Master 5 protection context control (PROT_SMPU_MS5_CTL)
  */
typedef struct stc_PROT_SMPU_MS5_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS5_CTL_field_t;

typedef union un_PROT_SMPU_MS5_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS5_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS5_CTL_t;

/**
  * \brief Master 6 protection context control (PROT_SMPU_MS6_CTL)
  */
typedef struct stc_PROT_SMPU_MS6_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS6_CTL_field_t;

typedef union un_PROT_SMPU_MS6_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS6_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS6_CTL_t;

/**
  * \brief Master 7 protection context control (PROT_SMPU_MS7_CTL)
  */
typedef struct stc_PROT_SMPU_MS7_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS7_CTL_field_t;

typedef union un_PROT_SMPU_MS7_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS7_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS7_CTL_t;

/**
  * \brief Master 8 protection context control (PROT_SMPU_MS8_CTL)
  */
typedef struct stc_PROT_SMPU_MS8_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS8_CTL_field_t;

typedef union un_PROT_SMPU_MS8_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS8_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS8_CTL_t;

/**
  * \brief Master 9 protection context control (PROT_SMPU_MS9_CTL)
  */
typedef struct stc_PROT_SMPU_MS9_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS9_CTL_field_t;

typedef union un_PROT_SMPU_MS9_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS9_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS9_CTL_t;

/**
  * \brief Master 10 protection context control (PROT_SMPU_MS10_CTL)
  */
typedef struct stc_PROT_SMPU_MS10_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS10_CTL_field_t;

typedef union un_PROT_SMPU_MS10_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS10_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS10_CTL_t;

/**
  * \brief Master 11 protection context control (PROT_SMPU_MS11_CTL)
  */
typedef struct stc_PROT_SMPU_MS11_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS11_CTL_field_t;

typedef union un_PROT_SMPU_MS11_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS11_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS11_CTL_t;

/**
  * \brief Master 12 protection context control (PROT_SMPU_MS12_CTL)
  */
typedef struct stc_PROT_SMPU_MS12_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS12_CTL_field_t;

typedef union un_PROT_SMPU_MS12_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS12_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS12_CTL_t;

/**
  * \brief Master 13 protection context control (PROT_SMPU_MS13_CTL)
  */
typedef struct stc_PROT_SMPU_MS13_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS13_CTL_field_t;

typedef union un_PROT_SMPU_MS13_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS13_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS13_CTL_t;

/**
  * \brief Master 14 protection context control (PROT_SMPU_MS14_CTL)
  */
typedef struct stc_PROT_SMPU_MS14_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS14_CTL_field_t;

typedef union un_PROT_SMPU_MS14_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS14_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS14_CTL_t;

/**
  * \brief Master 15 protection context control (PROT_SMPU_MS15_CTL)
  */
typedef struct stc_PROT_SMPU_MS15_CTL_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :6;
  uint32_t                         u2PRIO:2;
  uint32_t                         :6;
  uint32_t                         u1PC_MASK_0:1;
  uint32_t                         u15PC_MASK_15_TO_1:15;
} stc_PROT_SMPU_MS15_CTL_field_t;

typedef union un_PROT_SMPU_MS15_CTL {
  uint32_t                         u32Register;
  stc_PROT_SMPU_MS15_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_SMPU_MS15_CTL_t;



/**
  * \brief MPU region address (PROT_MPU_MPU_STRUCT_ADDR)
  */
typedef struct stc_PROT_MPU_MPU_STRUCT_ADDR_field {
  uint32_t                         u8SUBREGION_DISABLE:8;
  uint32_t                         u24ADDR24:24;
} stc_PROT_MPU_MPU_STRUCT_ADDR_field_t;

typedef union un_PROT_MPU_MPU_STRUCT_ADDR {
  uint32_t                         u32Register;
  stc_PROT_MPU_MPU_STRUCT_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_MPU_MPU_STRUCT_ADDR_t;

/**
  * \brief MPU region attrributes (PROT_MPU_MPU_STRUCT_ATT)
  */
typedef struct stc_PROT_MPU_MPU_STRUCT_ATT_field {
  uint32_t                         u1UR:1;
  uint32_t                         u1UW:1;
  uint32_t                         u1UX:1;
  uint32_t                         u1PR:1;
  uint32_t                         u1PW:1;
  uint32_t                         u1PX:1;
  uint32_t                         u1NS:1;
  uint32_t                         :17;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :2;
  uint32_t                         u1ENABLED:1;
} stc_PROT_MPU_MPU_STRUCT_ATT_field_t;

typedef union un_PROT_MPU_MPU_STRUCT_ATT {
  uint32_t                         u32Register;
  stc_PROT_MPU_MPU_STRUCT_ATT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_MPU_MPU_STRUCT_ATT_t;



/**
  * \brief Master control (PROT_MPU_MS_CTL)
  */
typedef struct stc_PROT_MPU_MS_CTL_field {
  uint32_t                         u4PC:4;
  uint32_t                         :12;
  uint32_t                         u4PC_SAVED:4;
  uint32_t                         :12;
} stc_PROT_MPU_MS_CTL_field_t;

typedef union un_PROT_MPU_MS_CTL {
  uint32_t                         u32Register;
  stc_PROT_MPU_MS_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_MPU_MS_CTL_t;

/**
  * \brief Master control read mirror (PROT_MPU_MS_CTL_READ_MIR)
  */
typedef struct stc_PROT_MPU_MS_CTL_READ_MIR_field {
  uint32_t                         u4PC:4;
  uint32_t                         :12;
  uint32_t                         u4PC_SAVED:4;
  uint32_t                         :12;
} stc_PROT_MPU_MS_CTL_READ_MIR_field_t;

typedef union un_PROT_MPU_MS_CTL_READ_MIR {
  uint32_t                         u32Register;
  stc_PROT_MPU_MS_CTL_READ_MIR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PROT_MPU_MS_CTL_READ_MIR_t;



/**
  * \brief SMPU structure (PROT_SMPU_SMPU_STRUCT)
  */
typedef struct stc_PROT_SMPU_SMPU_STRUCT {
  un_PROT_SMPU_SMPU_STRUCT_ADDR0_t unADDR0;            /*!< 0x00000000 SMPU region address 0 (slave structure) */
  un_PROT_SMPU_SMPU_STRUCT_ATT0_t  unATT0;             /*!< 0x00000004 SMPU region attributes 0 (slave structure) */
  uint32_t                         au32Reserved[6];
  un_PROT_SMPU_SMPU_STRUCT_ADDR1_t unADDR1;            /*!< 0x00000020 SMPU region address 1 (master structure) */
  un_PROT_SMPU_SMPU_STRUCT_ATT1_t  unATT1;             /*!< 0x00000024 SMPU region attributes 1 (master structure) */
  uint32_t                         au32Reserved1[6];
} stc_PROT_SMPU_SMPU_STRUCT_t;                         /*!< Size = 64 (0x40) */

/**
  * \brief SMPU (PROT_SMPU)
  */
typedef struct stc_PROT_SMPU {
  un_PROT_SMPU_MS0_CTL_t           unMS0_CTL;          /*!< 0x00000000 Master 0 protection context control */
  un_PROT_SMPU_MS1_CTL_t           unMS1_CTL;          /*!< 0x00000004 Master 1 protection context control */
  un_PROT_SMPU_MS2_CTL_t           unMS2_CTL;          /*!< 0x00000008 Master 2 protection context control */
  un_PROT_SMPU_MS3_CTL_t           unMS3_CTL;          /*!< 0x0000000C Master 3 protection context control */
  un_PROT_SMPU_MS4_CTL_t           unMS4_CTL;          /*!< 0x00000010 Master 4 protection context control */
  un_PROT_SMPU_MS5_CTL_t           unMS5_CTL;          /*!< 0x00000014 Master 5 protection context control */
  un_PROT_SMPU_MS6_CTL_t           unMS6_CTL;          /*!< 0x00000018 Master 6 protection context control */
  un_PROT_SMPU_MS7_CTL_t           unMS7_CTL;          /*!< 0x0000001C Master 7 protection context control */
  un_PROT_SMPU_MS8_CTL_t           unMS8_CTL;          /*!< 0x00000020 Master 8 protection context control */
  un_PROT_SMPU_MS9_CTL_t           unMS9_CTL;          /*!< 0x00000024 Master 9 protection context control */
  un_PROT_SMPU_MS10_CTL_t          unMS10_CTL;         /*!< 0x00000028 Master 10 protection context control */
  un_PROT_SMPU_MS11_CTL_t          unMS11_CTL;         /*!< 0x0000002C Master 11 protection context control */
  un_PROT_SMPU_MS12_CTL_t          unMS12_CTL;         /*!< 0x00000030 Master 12 protection context control */
  un_PROT_SMPU_MS13_CTL_t          unMS13_CTL;         /*!< 0x00000034 Master 13 protection context control */
  un_PROT_SMPU_MS14_CTL_t          unMS14_CTL;         /*!< 0x00000038 Master 14 protection context control */
  un_PROT_SMPU_MS15_CTL_t          unMS15_CTL;         /*!< 0x0000003C Master 15 protection context control */
  uint32_t                         au32Reserved[2032];
  stc_PROT_SMPU_SMPU_STRUCT_t      SMPU_STRUCT[32];    /*!< 0x00002000 SMPU structure */
  uint32_t                         au32Reserved1[1536];
} stc_PROT_SMPU_t;                                     /*!< Size = 16384 (0x4000) */

/**
  * \brief MPU structure (PROT_MPU_MPU_STRUCT)
  */
typedef struct stc_PROT_MPU_MPU_STRUCT {
  un_PROT_MPU_MPU_STRUCT_ADDR_t    unADDR;             /*!< 0x00000000 MPU region address */
  un_PROT_MPU_MPU_STRUCT_ATT_t     unATT;              /*!< 0x00000004 MPU region attrributes */
  uint32_t                         au32Reserved[6];
} stc_PROT_MPU_MPU_STRUCT_t;                           /*!< Size = 32 (0x20) */

/**
  * \brief MPU (PROT_MPU)
  */
typedef struct stc_PROT_MPU {
  un_PROT_MPU_MS_CTL_t             unMS_CTL;           /*!< 0x00000000 Master control */
  un_PROT_MPU_MS_CTL_READ_MIR_t    unMS_CTL_READ_MIR[127]; /*!< 0x00000004 Master control read mirror */
  stc_PROT_MPU_MPU_STRUCT_t        MPU_STRUCT[16];     /*!< 0x00000200 MPU structure */
} stc_PROT_MPU_t;                                      /*!< Size = 1024 (0x400) */

/**
  * \brief Protection (PROT)
  */
typedef struct stc_PROT {
  stc_PROT_SMPU_t                  SMPU;               /*!< 0x00000000 SMPU */
  stc_PROT_MPU_t                   CYMPU[16];          /*!< 0x00004000 MPU */
} stc_PROT_t;                                          /*!< Size = 32768 (0x8000) */


/* PROT_SMPU_SMPU_STRUCT.ADDR0 */
#define PROT_SMPU_SMPU_STRUCT_ADDR0_SUBREGION_DISABLE_Pos 0UL
#define PROT_SMPU_SMPU_STRUCT_ADDR0_SUBREGION_DISABLE_Msk 0xFFUL
#define PROT_SMPU_SMPU_STRUCT_ADDR0_ADDR24_Pos  8UL
#define PROT_SMPU_SMPU_STRUCT_ADDR0_ADDR24_Msk  0xFFFFFF00UL
/* PROT_SMPU_SMPU_STRUCT.ATT0 */
#define PROT_SMPU_SMPU_STRUCT_ATT0_UR_Pos       0UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_UR_Msk       0x1UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_UW_Pos       1UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_UW_Msk       0x2UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_UX_Pos       2UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_UX_Msk       0x4UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PR_Pos       3UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PR_Msk       0x8UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PW_Pos       4UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PW_Msk       0x10UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PX_Pos       5UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PX_Msk       0x20UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_NS_Pos       6UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_NS_Msk       0x40UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MASK_0_Pos 8UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MASK_0_Msk 0x100UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MASK_15_TO_1_Pos 9UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MASK_15_TO_1_Msk 0xFFFE00UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_REGION_SIZE_Pos 24UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_REGION_SIZE_Msk 0x1F000000UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MATCH_Pos 30UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_PC_MATCH_Msk 0x40000000UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_ENABLED_Pos  31UL
#define PROT_SMPU_SMPU_STRUCT_ATT0_ENABLED_Msk  0x80000000UL
/* PROT_SMPU_SMPU_STRUCT.ADDR1 */
#define PROT_SMPU_SMPU_STRUCT_ADDR1_SUBREGION_DISABLE_Pos 0UL
#define PROT_SMPU_SMPU_STRUCT_ADDR1_SUBREGION_DISABLE_Msk 0xFFUL
#define PROT_SMPU_SMPU_STRUCT_ADDR1_ADDR24_Pos  8UL
#define PROT_SMPU_SMPU_STRUCT_ADDR1_ADDR24_Msk  0xFFFFFF00UL
/* PROT_SMPU_SMPU_STRUCT.ATT1 */
#define PROT_SMPU_SMPU_STRUCT_ATT1_UR_Pos       0UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_UR_Msk       0x1UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_UW_Pos       1UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_UW_Msk       0x2UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_UX_Pos       2UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_UX_Msk       0x4UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PR_Pos       3UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PR_Msk       0x8UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PW_Pos       4UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PW_Msk       0x10UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PX_Pos       5UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PX_Msk       0x20UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_NS_Pos       6UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_NS_Msk       0x40UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MASK_0_Pos 8UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MASK_0_Msk 0x100UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MASK_15_TO_1_Pos 9UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MASK_15_TO_1_Msk 0xFFFE00UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_REGION_SIZE_Pos 24UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_REGION_SIZE_Msk 0x1F000000UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MATCH_Pos 30UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_PC_MATCH_Msk 0x40000000UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_ENABLED_Pos  31UL
#define PROT_SMPU_SMPU_STRUCT_ATT1_ENABLED_Msk  0x80000000UL


/* PROT_SMPU.MS0_CTL */
#define PROT_SMPU_MS0_CTL_P_Pos                 0UL
#define PROT_SMPU_MS0_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS0_CTL_NS_Pos                1UL
#define PROT_SMPU_MS0_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS0_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS0_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS0_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS0_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS0_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS0_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS1_CTL */
#define PROT_SMPU_MS1_CTL_P_Pos                 0UL
#define PROT_SMPU_MS1_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS1_CTL_NS_Pos                1UL
#define PROT_SMPU_MS1_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS1_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS1_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS1_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS1_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS1_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS1_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS2_CTL */
#define PROT_SMPU_MS2_CTL_P_Pos                 0UL
#define PROT_SMPU_MS2_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS2_CTL_NS_Pos                1UL
#define PROT_SMPU_MS2_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS2_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS2_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS2_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS2_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS2_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS2_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS3_CTL */
#define PROT_SMPU_MS3_CTL_P_Pos                 0UL
#define PROT_SMPU_MS3_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS3_CTL_NS_Pos                1UL
#define PROT_SMPU_MS3_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS3_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS3_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS3_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS3_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS3_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS3_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS4_CTL */
#define PROT_SMPU_MS4_CTL_P_Pos                 0UL
#define PROT_SMPU_MS4_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS4_CTL_NS_Pos                1UL
#define PROT_SMPU_MS4_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS4_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS4_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS4_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS4_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS4_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS4_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS5_CTL */
#define PROT_SMPU_MS5_CTL_P_Pos                 0UL
#define PROT_SMPU_MS5_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS5_CTL_NS_Pos                1UL
#define PROT_SMPU_MS5_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS5_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS5_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS5_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS5_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS5_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS5_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS6_CTL */
#define PROT_SMPU_MS6_CTL_P_Pos                 0UL
#define PROT_SMPU_MS6_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS6_CTL_NS_Pos                1UL
#define PROT_SMPU_MS6_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS6_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS6_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS6_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS6_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS6_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS6_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS7_CTL */
#define PROT_SMPU_MS7_CTL_P_Pos                 0UL
#define PROT_SMPU_MS7_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS7_CTL_NS_Pos                1UL
#define PROT_SMPU_MS7_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS7_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS7_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS7_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS7_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS7_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS7_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS8_CTL */
#define PROT_SMPU_MS8_CTL_P_Pos                 0UL
#define PROT_SMPU_MS8_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS8_CTL_NS_Pos                1UL
#define PROT_SMPU_MS8_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS8_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS8_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS8_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS8_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS8_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS8_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS9_CTL */
#define PROT_SMPU_MS9_CTL_P_Pos                 0UL
#define PROT_SMPU_MS9_CTL_P_Msk                 0x1UL
#define PROT_SMPU_MS9_CTL_NS_Pos                1UL
#define PROT_SMPU_MS9_CTL_NS_Msk                0x2UL
#define PROT_SMPU_MS9_CTL_PRIO_Pos              8UL
#define PROT_SMPU_MS9_CTL_PRIO_Msk              0x300UL
#define PROT_SMPU_MS9_CTL_PC_MASK_0_Pos         16UL
#define PROT_SMPU_MS9_CTL_PC_MASK_0_Msk         0x10000UL
#define PROT_SMPU_MS9_CTL_PC_MASK_15_TO_1_Pos   17UL
#define PROT_SMPU_MS9_CTL_PC_MASK_15_TO_1_Msk   0xFFFE0000UL
/* PROT_SMPU.MS10_CTL */
#define PROT_SMPU_MS10_CTL_P_Pos                0UL
#define PROT_SMPU_MS10_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS10_CTL_NS_Pos               1UL
#define PROT_SMPU_MS10_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS10_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS10_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS10_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS10_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS10_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS10_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL
/* PROT_SMPU.MS11_CTL */
#define PROT_SMPU_MS11_CTL_P_Pos                0UL
#define PROT_SMPU_MS11_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS11_CTL_NS_Pos               1UL
#define PROT_SMPU_MS11_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS11_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS11_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS11_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS11_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS11_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS11_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL
/* PROT_SMPU.MS12_CTL */
#define PROT_SMPU_MS12_CTL_P_Pos                0UL
#define PROT_SMPU_MS12_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS12_CTL_NS_Pos               1UL
#define PROT_SMPU_MS12_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS12_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS12_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS12_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS12_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS12_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS12_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL
/* PROT_SMPU.MS13_CTL */
#define PROT_SMPU_MS13_CTL_P_Pos                0UL
#define PROT_SMPU_MS13_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS13_CTL_NS_Pos               1UL
#define PROT_SMPU_MS13_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS13_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS13_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS13_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS13_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS13_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS13_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL
/* PROT_SMPU.MS14_CTL */
#define PROT_SMPU_MS14_CTL_P_Pos                0UL
#define PROT_SMPU_MS14_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS14_CTL_NS_Pos               1UL
#define PROT_SMPU_MS14_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS14_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS14_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS14_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS14_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS14_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS14_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL
/* PROT_SMPU.MS15_CTL */
#define PROT_SMPU_MS15_CTL_P_Pos                0UL
#define PROT_SMPU_MS15_CTL_P_Msk                0x1UL
#define PROT_SMPU_MS15_CTL_NS_Pos               1UL
#define PROT_SMPU_MS15_CTL_NS_Msk               0x2UL
#define PROT_SMPU_MS15_CTL_PRIO_Pos             8UL
#define PROT_SMPU_MS15_CTL_PRIO_Msk             0x300UL
#define PROT_SMPU_MS15_CTL_PC_MASK_0_Pos        16UL
#define PROT_SMPU_MS15_CTL_PC_MASK_0_Msk        0x10000UL
#define PROT_SMPU_MS15_CTL_PC_MASK_15_TO_1_Pos  17UL
#define PROT_SMPU_MS15_CTL_PC_MASK_15_TO_1_Msk  0xFFFE0000UL


/* PROT_MPU_MPU_STRUCT.ADDR */
#define PROT_MPU_MPU_STRUCT_ADDR_SUBREGION_DISABLE_Pos 0UL
#define PROT_MPU_MPU_STRUCT_ADDR_SUBREGION_DISABLE_Msk 0xFFUL
#define PROT_MPU_MPU_STRUCT_ADDR_ADDR24_Pos     8UL
#define PROT_MPU_MPU_STRUCT_ADDR_ADDR24_Msk     0xFFFFFF00UL
/* PROT_MPU_MPU_STRUCT.ATT */
#define PROT_MPU_MPU_STRUCT_ATT_UR_Pos          0UL
#define PROT_MPU_MPU_STRUCT_ATT_UR_Msk          0x1UL
#define PROT_MPU_MPU_STRUCT_ATT_UW_Pos          1UL
#define PROT_MPU_MPU_STRUCT_ATT_UW_Msk          0x2UL
#define PROT_MPU_MPU_STRUCT_ATT_UX_Pos          2UL
#define PROT_MPU_MPU_STRUCT_ATT_UX_Msk          0x4UL
#define PROT_MPU_MPU_STRUCT_ATT_PR_Pos          3UL
#define PROT_MPU_MPU_STRUCT_ATT_PR_Msk          0x8UL
#define PROT_MPU_MPU_STRUCT_ATT_PW_Pos          4UL
#define PROT_MPU_MPU_STRUCT_ATT_PW_Msk          0x10UL
#define PROT_MPU_MPU_STRUCT_ATT_PX_Pos          5UL
#define PROT_MPU_MPU_STRUCT_ATT_PX_Msk          0x20UL
#define PROT_MPU_MPU_STRUCT_ATT_NS_Pos          6UL
#define PROT_MPU_MPU_STRUCT_ATT_NS_Msk          0x40UL
#define PROT_MPU_MPU_STRUCT_ATT_REGION_SIZE_Pos 24UL
#define PROT_MPU_MPU_STRUCT_ATT_REGION_SIZE_Msk 0x1F000000UL
#define PROT_MPU_MPU_STRUCT_ATT_ENABLED_Pos     31UL
#define PROT_MPU_MPU_STRUCT_ATT_ENABLED_Msk     0x80000000UL


/* PROT_MPU.MS_CTL */
#define PROT_MPU_MS_CTL_PC_Pos                  0UL
#define PROT_MPU_MS_CTL_PC_Msk                  0xFUL
#define PROT_MPU_MS_CTL_PC_SAVED_Pos            16UL
#define PROT_MPU_MS_CTL_PC_SAVED_Msk            0xF0000UL
/* PROT_MPU.MS_CTL_READ_MIR */
#define PROT_MPU_MS_CTL_READ_MIR_PC_Pos         0UL
#define PROT_MPU_MS_CTL_READ_MIR_PC_Msk         0xFUL
#define PROT_MPU_MS_CTL_READ_MIR_PC_SAVED_Pos   16UL
#define PROT_MPU_MS_CTL_READ_MIR_PC_SAVED_Msk   0xF0000UL


#endif /* _CYIP_PROT_H_ */


/* [] END OF FILE */
