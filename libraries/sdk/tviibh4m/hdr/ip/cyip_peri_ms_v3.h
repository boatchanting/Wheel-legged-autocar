/***************************************************************************//**
* \file cyip_peri_ms_v3.h
*
* \brief
* PERI_MS IP definitions
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

#ifndef _CYIP_PERI_MS_V3_H_
#define _CYIP_PERI_MS_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                   PERI_MS
*******************************************************************************/

#define PERI_MS_PPU_PR_SECTION_SIZE             0x00000040UL
#define PERI_MS_PPU_FX_SECTION_SIZE             0x00000040UL
#define PERI_MS_SECTION_SIZE                    0x00020000UL

/**
  * \brief Slave region, base address (PERI_MS_PPU_PR_SL_ADDR)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_ADDR_field {
  uint32_t                         :2;
  uint32_t                         u30ADDR30:30;
} stc_PERI_MS_PPU_PR_SL_ADDR_field_t;

typedef union un_PERI_MS_PPU_PR_SL_ADDR {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_ADDR_t;

/**
  * \brief Slave region, size (PERI_MS_PPU_PR_SL_SIZE)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_SIZE_field {
  uint32_t                         :24;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :2;
  uint32_t                         u1VALID:1;
} stc_PERI_MS_PPU_PR_SL_SIZE_field_t;

typedef union un_PERI_MS_PPU_PR_SL_SIZE {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_SIZE_t;

/**
  * \brief Slave attributes 0 (PERI_MS_PPU_PR_SL_ATT0)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_ATT0_field {
  uint32_t                         u1PC0_UR:1;
  uint32_t                         u1PC0_UW:1;
  uint32_t                         u1PC0_PR:1;
  uint32_t                         u1PC0_PW:1;
  uint32_t                         u1PC0_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC1_UR:1;
  uint32_t                         u1PC1_UW:1;
  uint32_t                         u1PC1_PR:1;
  uint32_t                         u1PC1_PW:1;
  uint32_t                         u1PC1_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC2_UR:1;
  uint32_t                         u1PC2_UW:1;
  uint32_t                         u1PC2_PR:1;
  uint32_t                         u1PC2_PW:1;
  uint32_t                         u1PC2_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC3_UR:1;
  uint32_t                         u1PC3_UW:1;
  uint32_t                         u1PC3_PR:1;
  uint32_t                         u1PC3_PW:1;
  uint32_t                         u1PC3_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_SL_ATT0_field_t;

typedef union un_PERI_MS_PPU_PR_SL_ATT0 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_ATT0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_ATT0_t;

/**
  * \brief Slave attributes 1 (PERI_MS_PPU_PR_SL_ATT1)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_ATT1_field {
  uint32_t                         u1PC4_UR:1;
  uint32_t                         u1PC4_UW:1;
  uint32_t                         u1PC4_PR:1;
  uint32_t                         u1PC4_PW:1;
  uint32_t                         u1PC4_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC5_UR:1;
  uint32_t                         u1PC5_UW:1;
  uint32_t                         u1PC5_PR:1;
  uint32_t                         u1PC5_PW:1;
  uint32_t                         u1PC5_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC6_UR:1;
  uint32_t                         u1PC6_UW:1;
  uint32_t                         u1PC6_PR:1;
  uint32_t                         u1PC6_PW:1;
  uint32_t                         u1PC6_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC7_UR:1;
  uint32_t                         u1PC7_UW:1;
  uint32_t                         u1PC7_PR:1;
  uint32_t                         u1PC7_PW:1;
  uint32_t                         u1PC7_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_SL_ATT1_field_t;

typedef union un_PERI_MS_PPU_PR_SL_ATT1 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_ATT1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_ATT1_t;

/**
  * \brief Slave attributes 2 (PERI_MS_PPU_PR_SL_ATT2)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_ATT2_field {
  uint32_t                         u1PC8_UR:1;
  uint32_t                         u1PC8_UW:1;
  uint32_t                         u1PC8_PR:1;
  uint32_t                         u1PC8_PW:1;
  uint32_t                         u1PC8_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC9_UR:1;
  uint32_t                         u1PC9_UW:1;
  uint32_t                         u1PC9_PR:1;
  uint32_t                         u1PC9_PW:1;
  uint32_t                         u1PC9_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC10_UR:1;
  uint32_t                         u1PC10_UW:1;
  uint32_t                         u1PC10_PR:1;
  uint32_t                         u1PC10_PW:1;
  uint32_t                         u1PC10_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC11_UR:1;
  uint32_t                         u1PC11_UW:1;
  uint32_t                         u1PC11_PR:1;
  uint32_t                         u1PC11_PW:1;
  uint32_t                         u1PC11_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_SL_ATT2_field_t;

typedef union un_PERI_MS_PPU_PR_SL_ATT2 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_ATT2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_ATT2_t;

/**
  * \brief Slave attributes 3 (PERI_MS_PPU_PR_SL_ATT3)
  */
typedef struct stc_PERI_MS_PPU_PR_SL_ATT3_field {
  uint32_t                         u1PC12_UR:1;
  uint32_t                         u1PC12_UW:1;
  uint32_t                         u1PC12_PR:1;
  uint32_t                         u1PC12_PW:1;
  uint32_t                         u1PC12_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC13_UR:1;
  uint32_t                         u1PC13_UW:1;
  uint32_t                         u1PC13_PR:1;
  uint32_t                         u1PC13_PW:1;
  uint32_t                         u1PC13_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC14_UR:1;
  uint32_t                         u1PC14_UW:1;
  uint32_t                         u1PC14_PR:1;
  uint32_t                         u1PC14_PW:1;
  uint32_t                         u1PC14_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC15_UR:1;
  uint32_t                         u1PC15_UW:1;
  uint32_t                         u1PC15_PR:1;
  uint32_t                         u1PC15_PW:1;
  uint32_t                         u1PC15_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_SL_ATT3_field_t;

typedef union un_PERI_MS_PPU_PR_SL_ATT3 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_SL_ATT3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_SL_ATT3_t;

/**
  * \brief Master region, base address (PERI_MS_PPU_PR_MS_ADDR)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_ADDR_field {
  uint32_t                         :6;
  uint32_t                         u26ADDR26:26;
} stc_PERI_MS_PPU_PR_MS_ADDR_field_t;

typedef union un_PERI_MS_PPU_PR_MS_ADDR {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_ADDR_t;

/**
  * \brief Master region, size (PERI_MS_PPU_PR_MS_SIZE)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_SIZE_field {
  uint32_t                         :24;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :2;
  uint32_t                         u1VALID:1;
} stc_PERI_MS_PPU_PR_MS_SIZE_field_t;

typedef union un_PERI_MS_PPU_PR_MS_SIZE {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_SIZE_t;

/**
  * \brief Master attributes 0 (PERI_MS_PPU_PR_MS_ATT0)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_ATT0_field {
  uint32_t                         u1PC0_UR:1;
  uint32_t                         u1PC0_UW:1;
  uint32_t                         u1PC0_PR:1;
  uint32_t                         u1PC0_PW:1;
  uint32_t                         u1PC0_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC1_UR:1;
  uint32_t                         u1PC1_UW:1;
  uint32_t                         u1PC1_PR:1;
  uint32_t                         u1PC1_PW:1;
  uint32_t                         u1PC1_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC2_UR:1;
  uint32_t                         u1PC2_UW:1;
  uint32_t                         u1PC2_PR:1;
  uint32_t                         u1PC2_PW:1;
  uint32_t                         u1PC2_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC3_UR:1;
  uint32_t                         u1PC3_UW:1;
  uint32_t                         u1PC3_PR:1;
  uint32_t                         u1PC3_PW:1;
  uint32_t                         u1PC3_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_MS_ATT0_field_t;

typedef union un_PERI_MS_PPU_PR_MS_ATT0 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_ATT0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_ATT0_t;

/**
  * \brief Master attributes 1 (PERI_MS_PPU_PR_MS_ATT1)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_ATT1_field {
  uint32_t                         u1PC4_UR:1;
  uint32_t                         u1PC4_UW:1;
  uint32_t                         u1PC4_PR:1;
  uint32_t                         u1PC4_PW:1;
  uint32_t                         u1PC4_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC5_UR:1;
  uint32_t                         u1PC5_UW:1;
  uint32_t                         u1PC5_PR:1;
  uint32_t                         u1PC5_PW:1;
  uint32_t                         u1PC5_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC6_UR:1;
  uint32_t                         u1PC6_UW:1;
  uint32_t                         u1PC6_PR:1;
  uint32_t                         u1PC6_PW:1;
  uint32_t                         u1PC6_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC7_UR:1;
  uint32_t                         u1PC7_UW:1;
  uint32_t                         u1PC7_PR:1;
  uint32_t                         u1PC7_PW:1;
  uint32_t                         u1PC7_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_MS_ATT1_field_t;

typedef union un_PERI_MS_PPU_PR_MS_ATT1 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_ATT1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_ATT1_t;

/**
  * \brief Master attributes 2 (PERI_MS_PPU_PR_MS_ATT2)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_ATT2_field {
  uint32_t                         u1PC8_UR:1;
  uint32_t                         u1PC8_UW:1;
  uint32_t                         u1PC8_PR:1;
  uint32_t                         u1PC8_PW:1;
  uint32_t                         u1PC8_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC9_UR:1;
  uint32_t                         u1PC9_UW:1;
  uint32_t                         u1PC9_PR:1;
  uint32_t                         u1PC9_PW:1;
  uint32_t                         u1PC9_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC10_UR:1;
  uint32_t                         u1PC10_UW:1;
  uint32_t                         u1PC10_PR:1;
  uint32_t                         u1PC10_PW:1;
  uint32_t                         u1PC10_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC11_UR:1;
  uint32_t                         u1PC11_UW:1;
  uint32_t                         u1PC11_PR:1;
  uint32_t                         u1PC11_PW:1;
  uint32_t                         u1PC11_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_MS_ATT2_field_t;

typedef union un_PERI_MS_PPU_PR_MS_ATT2 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_ATT2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_ATT2_t;

/**
  * \brief Master attributes 3 (PERI_MS_PPU_PR_MS_ATT3)
  */
typedef struct stc_PERI_MS_PPU_PR_MS_ATT3_field {
  uint32_t                         u1PC12_UR:1;
  uint32_t                         u1PC12_UW:1;
  uint32_t                         u1PC12_PR:1;
  uint32_t                         u1PC12_PW:1;
  uint32_t                         u1PC12_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC13_UR:1;
  uint32_t                         u1PC13_UW:1;
  uint32_t                         u1PC13_PR:1;
  uint32_t                         u1PC13_PW:1;
  uint32_t                         u1PC13_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC14_UR:1;
  uint32_t                         u1PC14_UW:1;
  uint32_t                         u1PC14_PR:1;
  uint32_t                         u1PC14_PW:1;
  uint32_t                         u1PC14_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC15_UR:1;
  uint32_t                         u1PC15_UW:1;
  uint32_t                         u1PC15_PR:1;
  uint32_t                         u1PC15_PW:1;
  uint32_t                         u1PC15_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_PR_MS_ATT3_field_t;

typedef union un_PERI_MS_PPU_PR_MS_ATT3 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_PR_MS_ATT3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_PR_MS_ATT3_t;



/**
  * \brief Slave region, base address (PERI_MS_PPU_FX_SL_ADDR)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_ADDR_field {
  uint32_t                         :2;
  uint32_t                         u30ADDR30:30;
} stc_PERI_MS_PPU_FX_SL_ADDR_field_t;

typedef union un_PERI_MS_PPU_FX_SL_ADDR {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_ADDR_t;

/**
  * \brief Slave region, size (PERI_MS_PPU_FX_SL_SIZE)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_SIZE_field {
  uint32_t                         :24;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :2;
  uint32_t                         u1VALID:1;
} stc_PERI_MS_PPU_FX_SL_SIZE_field_t;

typedef union un_PERI_MS_PPU_FX_SL_SIZE {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_SIZE_t;

/**
  * \brief Slave attributes 0 (PERI_MS_PPU_FX_SL_ATT0)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_ATT0_field {
  uint32_t                         u1PC0_UR:1;
  uint32_t                         u1PC0_UW:1;
  uint32_t                         u1PC0_PR:1;
  uint32_t                         u1PC0_PW:1;
  uint32_t                         u1PC0_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC1_UR:1;
  uint32_t                         u1PC1_UW:1;
  uint32_t                         u1PC1_PR:1;
  uint32_t                         u1PC1_PW:1;
  uint32_t                         u1PC1_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC2_UR:1;
  uint32_t                         u1PC2_UW:1;
  uint32_t                         u1PC2_PR:1;
  uint32_t                         u1PC2_PW:1;
  uint32_t                         u1PC2_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC3_UR:1;
  uint32_t                         u1PC3_UW:1;
  uint32_t                         u1PC3_PR:1;
  uint32_t                         u1PC3_PW:1;
  uint32_t                         u1PC3_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_SL_ATT0_field_t;

typedef union un_PERI_MS_PPU_FX_SL_ATT0 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_ATT0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_ATT0_t;

/**
  * \brief Slave attributes 1 (PERI_MS_PPU_FX_SL_ATT1)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_ATT1_field {
  uint32_t                         u1PC4_UR:1;
  uint32_t                         u1PC4_UW:1;
  uint32_t                         u1PC4_PR:1;
  uint32_t                         u1PC4_PW:1;
  uint32_t                         u1PC4_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC5_UR:1;
  uint32_t                         u1PC5_UW:1;
  uint32_t                         u1PC5_PR:1;
  uint32_t                         u1PC5_PW:1;
  uint32_t                         u1PC5_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC6_UR:1;
  uint32_t                         u1PC6_UW:1;
  uint32_t                         u1PC6_PR:1;
  uint32_t                         u1PC6_PW:1;
  uint32_t                         u1PC6_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC7_UR:1;
  uint32_t                         u1PC7_UW:1;
  uint32_t                         u1PC7_PR:1;
  uint32_t                         u1PC7_PW:1;
  uint32_t                         u1PC7_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_SL_ATT1_field_t;

typedef union un_PERI_MS_PPU_FX_SL_ATT1 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_ATT1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_ATT1_t;

/**
  * \brief Slave attributes 2 (PERI_MS_PPU_FX_SL_ATT2)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_ATT2_field {
  uint32_t                         u1PC8_UR:1;
  uint32_t                         u1PC8_UW:1;
  uint32_t                         u1PC8_PR:1;
  uint32_t                         u1PC8_PW:1;
  uint32_t                         u1PC8_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC9_UR:1;
  uint32_t                         u1PC9_UW:1;
  uint32_t                         u1PC9_PR:1;
  uint32_t                         u1PC9_PW:1;
  uint32_t                         u1PC9_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC10_UR:1;
  uint32_t                         u1PC10_UW:1;
  uint32_t                         u1PC10_PR:1;
  uint32_t                         u1PC10_PW:1;
  uint32_t                         u1PC10_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC11_UR:1;
  uint32_t                         u1PC11_UW:1;
  uint32_t                         u1PC11_PR:1;
  uint32_t                         u1PC11_PW:1;
  uint32_t                         u1PC11_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_SL_ATT2_field_t;

typedef union un_PERI_MS_PPU_FX_SL_ATT2 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_ATT2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_ATT2_t;

/**
  * \brief Slave attributes 3 (PERI_MS_PPU_FX_SL_ATT3)
  */
typedef struct stc_PERI_MS_PPU_FX_SL_ATT3_field {
  uint32_t                         u1PC12_UR:1;
  uint32_t                         u1PC12_UW:1;
  uint32_t                         u1PC12_PR:1;
  uint32_t                         u1PC12_PW:1;
  uint32_t                         u1PC12_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC13_UR:1;
  uint32_t                         u1PC13_UW:1;
  uint32_t                         u1PC13_PR:1;
  uint32_t                         u1PC13_PW:1;
  uint32_t                         u1PC13_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC14_UR:1;
  uint32_t                         u1PC14_UW:1;
  uint32_t                         u1PC14_PR:1;
  uint32_t                         u1PC14_PW:1;
  uint32_t                         u1PC14_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC15_UR:1;
  uint32_t                         u1PC15_UW:1;
  uint32_t                         u1PC15_PR:1;
  uint32_t                         u1PC15_PW:1;
  uint32_t                         u1PC15_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_SL_ATT3_field_t;

typedef union un_PERI_MS_PPU_FX_SL_ATT3 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_SL_ATT3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_SL_ATT3_t;

/**
  * \brief Master region, base address (PERI_MS_PPU_FX_MS_ADDR)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_ADDR_field {
  uint32_t                         :6;
  uint32_t                         u26ADDR26:26;
} stc_PERI_MS_PPU_FX_MS_ADDR_field_t;

typedef union un_PERI_MS_PPU_FX_MS_ADDR {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_ADDR_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_ADDR_t;

/**
  * \brief Master region, size (PERI_MS_PPU_FX_MS_SIZE)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_SIZE_field {
  uint32_t                         :24;
  uint32_t                         u5REGION_SIZE:5;
  uint32_t                         :2;
  uint32_t                         u1VALID:1;
} stc_PERI_MS_PPU_FX_MS_SIZE_field_t;

typedef union un_PERI_MS_PPU_FX_MS_SIZE {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_SIZE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_SIZE_t;

/**
  * \brief Master attributes 0 (PERI_MS_PPU_FX_MS_ATT0)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_ATT0_field {
  uint32_t                         u1PC0_UR:1;
  uint32_t                         u1PC0_UW:1;
  uint32_t                         u1PC0_PR:1;
  uint32_t                         u1PC0_PW:1;
  uint32_t                         u1PC0_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC1_UR:1;
  uint32_t                         u1PC1_UW:1;
  uint32_t                         u1PC1_PR:1;
  uint32_t                         u1PC1_PW:1;
  uint32_t                         u1PC1_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC2_UR:1;
  uint32_t                         u1PC2_UW:1;
  uint32_t                         u1PC2_PR:1;
  uint32_t                         u1PC2_PW:1;
  uint32_t                         u1PC2_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC3_UR:1;
  uint32_t                         u1PC3_UW:1;
  uint32_t                         u1PC3_PR:1;
  uint32_t                         u1PC3_PW:1;
  uint32_t                         u1PC3_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_MS_ATT0_field_t;

typedef union un_PERI_MS_PPU_FX_MS_ATT0 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_ATT0_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_ATT0_t;

/**
  * \brief Master attributes 1 (PERI_MS_PPU_FX_MS_ATT1)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_ATT1_field {
  uint32_t                         u1PC4_UR:1;
  uint32_t                         u1PC4_UW:1;
  uint32_t                         u1PC4_PR:1;
  uint32_t                         u1PC4_PW:1;
  uint32_t                         u1PC4_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC5_UR:1;
  uint32_t                         u1PC5_UW:1;
  uint32_t                         u1PC5_PR:1;
  uint32_t                         u1PC5_PW:1;
  uint32_t                         u1PC5_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC6_UR:1;
  uint32_t                         u1PC6_UW:1;
  uint32_t                         u1PC6_PR:1;
  uint32_t                         u1PC6_PW:1;
  uint32_t                         u1PC6_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC7_UR:1;
  uint32_t                         u1PC7_UW:1;
  uint32_t                         u1PC7_PR:1;
  uint32_t                         u1PC7_PW:1;
  uint32_t                         u1PC7_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_MS_ATT1_field_t;

typedef union un_PERI_MS_PPU_FX_MS_ATT1 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_ATT1_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_ATT1_t;

/**
  * \brief Master attributes 2 (PERI_MS_PPU_FX_MS_ATT2)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_ATT2_field {
  uint32_t                         u1PC8_UR:1;
  uint32_t                         u1PC8_UW:1;
  uint32_t                         u1PC8_PR:1;
  uint32_t                         u1PC8_PW:1;
  uint32_t                         u1PC8_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC9_UR:1;
  uint32_t                         u1PC9_UW:1;
  uint32_t                         u1PC9_PR:1;
  uint32_t                         u1PC9_PW:1;
  uint32_t                         u1PC9_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC10_UR:1;
  uint32_t                         u1PC10_UW:1;
  uint32_t                         u1PC10_PR:1;
  uint32_t                         u1PC10_PW:1;
  uint32_t                         u1PC10_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC11_UR:1;
  uint32_t                         u1PC11_UW:1;
  uint32_t                         u1PC11_PR:1;
  uint32_t                         u1PC11_PW:1;
  uint32_t                         u1PC11_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_MS_ATT2_field_t;

typedef union un_PERI_MS_PPU_FX_MS_ATT2 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_ATT2_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_ATT2_t;

/**
  * \brief Master attributes 3 (PERI_MS_PPU_FX_MS_ATT3)
  */
typedef struct stc_PERI_MS_PPU_FX_MS_ATT3_field {
  uint32_t                         u1PC12_UR:1;
  uint32_t                         u1PC12_UW:1;
  uint32_t                         u1PC12_PR:1;
  uint32_t                         u1PC12_PW:1;
  uint32_t                         u1PC12_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC13_UR:1;
  uint32_t                         u1PC13_UW:1;
  uint32_t                         u1PC13_PR:1;
  uint32_t                         u1PC13_PW:1;
  uint32_t                         u1PC13_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC14_UR:1;
  uint32_t                         u1PC14_UW:1;
  uint32_t                         u1PC14_PR:1;
  uint32_t                         u1PC14_PW:1;
  uint32_t                         u1PC14_NS:1;
  uint32_t                         :3;
  uint32_t                         u1PC15_UR:1;
  uint32_t                         u1PC15_UW:1;
  uint32_t                         u1PC15_PR:1;
  uint32_t                         u1PC15_PW:1;
  uint32_t                         u1PC15_NS:1;
  uint32_t                         :3;
} stc_PERI_MS_PPU_FX_MS_ATT3_field_t;

typedef union un_PERI_MS_PPU_FX_MS_ATT3 {
  uint32_t                         u32Register;
  stc_PERI_MS_PPU_FX_MS_ATT3_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_PERI_MS_PPU_FX_MS_ATT3_t;



/**
  * \brief Programmable protection structure pair (PERI_MS_PPU_PR)
  */
typedef struct stc_PERI_MS_PPU_PR {
  un_PERI_MS_PPU_PR_SL_ADDR_t      unSL_ADDR;          /*!< 0x00000000 Slave region, base address */
  un_PERI_MS_PPU_PR_SL_SIZE_t      unSL_SIZE;          /*!< 0x00000004 Slave region, size */
  uint32_t                         au32Reserved[2];
  un_PERI_MS_PPU_PR_SL_ATT0_t      unSL_ATT0;          /*!< 0x00000010 Slave attributes 0 */
  un_PERI_MS_PPU_PR_SL_ATT1_t      unSL_ATT1;          /*!< 0x00000014 Slave attributes 1 */
  un_PERI_MS_PPU_PR_SL_ATT2_t      unSL_ATT2;          /*!< 0x00000018 Slave attributes 2 */
  un_PERI_MS_PPU_PR_SL_ATT3_t      unSL_ATT3;          /*!< 0x0000001C Slave attributes 3 */
  un_PERI_MS_PPU_PR_MS_ADDR_t      unMS_ADDR;          /*!< 0x00000020 Master region, base address */
  un_PERI_MS_PPU_PR_MS_SIZE_t      unMS_SIZE;          /*!< 0x00000024 Master region, size */
  uint32_t                         au32Reserved1[2];
  un_PERI_MS_PPU_PR_MS_ATT0_t      unMS_ATT0;          /*!< 0x00000030 Master attributes 0 */
  un_PERI_MS_PPU_PR_MS_ATT1_t      unMS_ATT1;          /*!< 0x00000034 Master attributes 1 */
  un_PERI_MS_PPU_PR_MS_ATT2_t      unMS_ATT2;          /*!< 0x00000038 Master attributes 2 */
  un_PERI_MS_PPU_PR_MS_ATT3_t      unMS_ATT3;          /*!< 0x0000003C Master attributes 3 */
} stc_PERI_MS_PPU_PR_t;                                /*!< Size = 64 (0x40) */

/**
  * \brief Fixed protection structure pair (PERI_MS_PPU_FX)
  */
typedef struct stc_PERI_MS_PPU_FX {
  un_PERI_MS_PPU_FX_SL_ADDR_t      unSL_ADDR;          /*!< 0x00000000 Slave region, base address */
  un_PERI_MS_PPU_FX_SL_SIZE_t      unSL_SIZE;          /*!< 0x00000004 Slave region, size */
  uint32_t                         au32Reserved[2];
  un_PERI_MS_PPU_FX_SL_ATT0_t      unSL_ATT0;          /*!< 0x00000010 Slave attributes 0 */
  un_PERI_MS_PPU_FX_SL_ATT1_t      unSL_ATT1;          /*!< 0x00000014 Slave attributes 1 */
  un_PERI_MS_PPU_FX_SL_ATT2_t      unSL_ATT2;          /*!< 0x00000018 Slave attributes 2 */
  un_PERI_MS_PPU_FX_SL_ATT3_t      unSL_ATT3;          /*!< 0x0000001C Slave attributes 3 */
  un_PERI_MS_PPU_FX_MS_ADDR_t      unMS_ADDR;          /*!< 0x00000020 Master region, base address */
  un_PERI_MS_PPU_FX_MS_SIZE_t      unMS_SIZE;          /*!< 0x00000024 Master region, size */
  uint32_t                         au32Reserved1[2];
  un_PERI_MS_PPU_FX_MS_ATT0_t      unMS_ATT0;          /*!< 0x00000030 Master attributes 0 */
  un_PERI_MS_PPU_FX_MS_ATT1_t      unMS_ATT1;          /*!< 0x00000034 Master attributes 1 */
  un_PERI_MS_PPU_FX_MS_ATT2_t      unMS_ATT2;          /*!< 0x00000038 Master attributes 2 */
  un_PERI_MS_PPU_FX_MS_ATT3_t      unMS_ATT3;          /*!< 0x0000003C Master attributes 3 */
} stc_PERI_MS_PPU_FX_t;                                /*!< Size = 64 (0x40) */

/**
  * \brief Peripheral interconnect, master interface (PERI_MS)
  */
typedef struct stc_PERI_MS {
  stc_PERI_MS_PPU_PR_t             PPU_PR[32];         /*!< 0x00000000 Programmable protection structure pair */
  stc_PERI_MS_PPU_FX_t             PPU_FX[992];        /*!< 0x00000800 Fixed protection structure pair */
} stc_PERI_MS_t;                                       /*!< Size = 65536 (0x10000) */


/* PERI_MS_PPU_PR.SL_ADDR */
#define PERI_MS_PPU_PR_SL_ADDR_ADDR30_Pos       2UL
#define PERI_MS_PPU_PR_SL_ADDR_ADDR30_Msk       0xFFFFFFFCUL
/* PERI_MS_PPU_PR.SL_SIZE */
#define PERI_MS_PPU_PR_SL_SIZE_REGION_SIZE_Pos  24UL
#define PERI_MS_PPU_PR_SL_SIZE_REGION_SIZE_Msk  0x1F000000UL
#define PERI_MS_PPU_PR_SL_SIZE_VALID_Pos        31UL
#define PERI_MS_PPU_PR_SL_SIZE_VALID_Msk        0x80000000UL
/* PERI_MS_PPU_PR.SL_ATT0 */
#define PERI_MS_PPU_PR_SL_ATT0_PC0_UR_Pos       0UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_UW_Pos       1UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_PR_Pos       2UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_PW_Pos       3UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_NS_Pos       4UL
#define PERI_MS_PPU_PR_SL_ATT0_PC0_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_UR_Pos       8UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_UW_Pos       9UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_PR_Pos       10UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_PW_Pos       11UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_NS_Pos       12UL
#define PERI_MS_PPU_PR_SL_ATT0_PC1_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_UR_Pos       16UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_UR_Msk       0x10000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_UW_Pos       17UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_UW_Msk       0x20000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_PR_Pos       18UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_PR_Msk       0x40000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_PW_Pos       19UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_PW_Msk       0x80000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_NS_Pos       20UL
#define PERI_MS_PPU_PR_SL_ATT0_PC2_NS_Msk       0x100000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_UR_Pos       24UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_UR_Msk       0x1000000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_UW_Pos       25UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_UW_Msk       0x2000000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_PR_Pos       26UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_PR_Msk       0x4000000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_PW_Pos       27UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_PW_Msk       0x8000000UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_NS_Pos       28UL
#define PERI_MS_PPU_PR_SL_ATT0_PC3_NS_Msk       0x10000000UL
/* PERI_MS_PPU_PR.SL_ATT1 */
#define PERI_MS_PPU_PR_SL_ATT1_PC4_UR_Pos       0UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_UW_Pos       1UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_PR_Pos       2UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_PW_Pos       3UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_NS_Pos       4UL
#define PERI_MS_PPU_PR_SL_ATT1_PC4_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_UR_Pos       8UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_UW_Pos       9UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_PR_Pos       10UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_PW_Pos       11UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_NS_Pos       12UL
#define PERI_MS_PPU_PR_SL_ATT1_PC5_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_UR_Pos       16UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_UR_Msk       0x10000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_UW_Pos       17UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_UW_Msk       0x20000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_PR_Pos       18UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_PR_Msk       0x40000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_PW_Pos       19UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_PW_Msk       0x80000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_NS_Pos       20UL
#define PERI_MS_PPU_PR_SL_ATT1_PC6_NS_Msk       0x100000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_UR_Pos       24UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_UR_Msk       0x1000000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_UW_Pos       25UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_UW_Msk       0x2000000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_PR_Pos       26UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_PR_Msk       0x4000000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_PW_Pos       27UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_PW_Msk       0x8000000UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_NS_Pos       28UL
#define PERI_MS_PPU_PR_SL_ATT1_PC7_NS_Msk       0x10000000UL
/* PERI_MS_PPU_PR.SL_ATT2 */
#define PERI_MS_PPU_PR_SL_ATT2_PC8_UR_Pos       0UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_UW_Pos       1UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_PR_Pos       2UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_PW_Pos       3UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_NS_Pos       4UL
#define PERI_MS_PPU_PR_SL_ATT2_PC8_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_UR_Pos       8UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_UW_Pos       9UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_PR_Pos       10UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_PW_Pos       11UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_NS_Pos       12UL
#define PERI_MS_PPU_PR_SL_ATT2_PC9_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_UR_Pos      16UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_UR_Msk      0x10000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_UW_Pos      17UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_UW_Msk      0x20000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_PR_Pos      18UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_PR_Msk      0x40000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_PW_Pos      19UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_PW_Msk      0x80000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_NS_Pos      20UL
#define PERI_MS_PPU_PR_SL_ATT2_PC10_NS_Msk      0x100000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_UR_Pos      24UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_UR_Msk      0x1000000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_UW_Pos      25UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_UW_Msk      0x2000000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_PR_Pos      26UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_PR_Msk      0x4000000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_PW_Pos      27UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_PW_Msk      0x8000000UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_NS_Pos      28UL
#define PERI_MS_PPU_PR_SL_ATT2_PC11_NS_Msk      0x10000000UL
/* PERI_MS_PPU_PR.SL_ATT3 */
#define PERI_MS_PPU_PR_SL_ATT3_PC12_UR_Pos      0UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_UR_Msk      0x1UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_UW_Pos      1UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_UW_Msk      0x2UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_PR_Pos      2UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_PR_Msk      0x4UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_PW_Pos      3UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_PW_Msk      0x8UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_NS_Pos      4UL
#define PERI_MS_PPU_PR_SL_ATT3_PC12_NS_Msk      0x10UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_UR_Pos      8UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_UR_Msk      0x100UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_UW_Pos      9UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_UW_Msk      0x200UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_PR_Pos      10UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_PR_Msk      0x400UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_PW_Pos      11UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_PW_Msk      0x800UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_NS_Pos      12UL
#define PERI_MS_PPU_PR_SL_ATT3_PC13_NS_Msk      0x1000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_UR_Pos      16UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_UR_Msk      0x10000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_UW_Pos      17UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_UW_Msk      0x20000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_PR_Pos      18UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_PR_Msk      0x40000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_PW_Pos      19UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_PW_Msk      0x80000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_NS_Pos      20UL
#define PERI_MS_PPU_PR_SL_ATT3_PC14_NS_Msk      0x100000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_UR_Pos      24UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_UR_Msk      0x1000000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_UW_Pos      25UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_UW_Msk      0x2000000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_PR_Pos      26UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_PR_Msk      0x4000000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_PW_Pos      27UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_PW_Msk      0x8000000UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_NS_Pos      28UL
#define PERI_MS_PPU_PR_SL_ATT3_PC15_NS_Msk      0x10000000UL
/* PERI_MS_PPU_PR.MS_ADDR */
#define PERI_MS_PPU_PR_MS_ADDR_ADDR26_Pos       6UL
#define PERI_MS_PPU_PR_MS_ADDR_ADDR26_Msk       0xFFFFFFC0UL
/* PERI_MS_PPU_PR.MS_SIZE */
#define PERI_MS_PPU_PR_MS_SIZE_REGION_SIZE_Pos  24UL
#define PERI_MS_PPU_PR_MS_SIZE_REGION_SIZE_Msk  0x1F000000UL
#define PERI_MS_PPU_PR_MS_SIZE_VALID_Pos        31UL
#define PERI_MS_PPU_PR_MS_SIZE_VALID_Msk        0x80000000UL
/* PERI_MS_PPU_PR.MS_ATT0 */
#define PERI_MS_PPU_PR_MS_ATT0_PC0_UR_Pos       0UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_UW_Pos       1UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_PR_Pos       2UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_PW_Pos       3UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_NS_Pos       4UL
#define PERI_MS_PPU_PR_MS_ATT0_PC0_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_UR_Pos       8UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_UW_Pos       9UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_PR_Pos       10UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_PW_Pos       11UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_NS_Pos       12UL
#define PERI_MS_PPU_PR_MS_ATT0_PC1_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_UR_Pos       16UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_UR_Msk       0x10000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_UW_Pos       17UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_UW_Msk       0x20000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_PR_Pos       18UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_PR_Msk       0x40000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_PW_Pos       19UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_PW_Msk       0x80000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_NS_Pos       20UL
#define PERI_MS_PPU_PR_MS_ATT0_PC2_NS_Msk       0x100000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_UR_Pos       24UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_UR_Msk       0x1000000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_UW_Pos       25UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_UW_Msk       0x2000000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_PR_Pos       26UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_PR_Msk       0x4000000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_PW_Pos       27UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_PW_Msk       0x8000000UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_NS_Pos       28UL
#define PERI_MS_PPU_PR_MS_ATT0_PC3_NS_Msk       0x10000000UL
/* PERI_MS_PPU_PR.MS_ATT1 */
#define PERI_MS_PPU_PR_MS_ATT1_PC4_UR_Pos       0UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_UW_Pos       1UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_PR_Pos       2UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_PW_Pos       3UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_NS_Pos       4UL
#define PERI_MS_PPU_PR_MS_ATT1_PC4_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_UR_Pos       8UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_UW_Pos       9UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_PR_Pos       10UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_PW_Pos       11UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_NS_Pos       12UL
#define PERI_MS_PPU_PR_MS_ATT1_PC5_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_UR_Pos       16UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_UR_Msk       0x10000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_UW_Pos       17UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_UW_Msk       0x20000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_PR_Pos       18UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_PR_Msk       0x40000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_PW_Pos       19UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_PW_Msk       0x80000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_NS_Pos       20UL
#define PERI_MS_PPU_PR_MS_ATT1_PC6_NS_Msk       0x100000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_UR_Pos       24UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_UR_Msk       0x1000000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_UW_Pos       25UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_UW_Msk       0x2000000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_PR_Pos       26UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_PR_Msk       0x4000000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_PW_Pos       27UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_PW_Msk       0x8000000UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_NS_Pos       28UL
#define PERI_MS_PPU_PR_MS_ATT1_PC7_NS_Msk       0x10000000UL
/* PERI_MS_PPU_PR.MS_ATT2 */
#define PERI_MS_PPU_PR_MS_ATT2_PC8_UR_Pos       0UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_UR_Msk       0x1UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_UW_Pos       1UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_UW_Msk       0x2UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_PR_Pos       2UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_PR_Msk       0x4UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_PW_Pos       3UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_PW_Msk       0x8UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_NS_Pos       4UL
#define PERI_MS_PPU_PR_MS_ATT2_PC8_NS_Msk       0x10UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_UR_Pos       8UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_UR_Msk       0x100UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_UW_Pos       9UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_UW_Msk       0x200UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_PR_Pos       10UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_PR_Msk       0x400UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_PW_Pos       11UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_PW_Msk       0x800UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_NS_Pos       12UL
#define PERI_MS_PPU_PR_MS_ATT2_PC9_NS_Msk       0x1000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_UR_Pos      16UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_UR_Msk      0x10000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_UW_Pos      17UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_UW_Msk      0x20000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_PR_Pos      18UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_PR_Msk      0x40000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_PW_Pos      19UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_PW_Msk      0x80000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_NS_Pos      20UL
#define PERI_MS_PPU_PR_MS_ATT2_PC10_NS_Msk      0x100000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_UR_Pos      24UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_UR_Msk      0x1000000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_UW_Pos      25UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_UW_Msk      0x2000000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_PR_Pos      26UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_PR_Msk      0x4000000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_PW_Pos      27UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_PW_Msk      0x8000000UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_NS_Pos      28UL
#define PERI_MS_PPU_PR_MS_ATT2_PC11_NS_Msk      0x10000000UL
/* PERI_MS_PPU_PR.MS_ATT3 */
#define PERI_MS_PPU_PR_MS_ATT3_PC12_UR_Pos      0UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_UR_Msk      0x1UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_UW_Pos      1UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_UW_Msk      0x2UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_PR_Pos      2UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_PR_Msk      0x4UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_PW_Pos      3UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_PW_Msk      0x8UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_NS_Pos      4UL
#define PERI_MS_PPU_PR_MS_ATT3_PC12_NS_Msk      0x10UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_UR_Pos      8UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_UR_Msk      0x100UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_UW_Pos      9UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_UW_Msk      0x200UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_PR_Pos      10UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_PR_Msk      0x400UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_PW_Pos      11UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_PW_Msk      0x800UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_NS_Pos      12UL
#define PERI_MS_PPU_PR_MS_ATT3_PC13_NS_Msk      0x1000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_UR_Pos      16UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_UR_Msk      0x10000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_UW_Pos      17UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_UW_Msk      0x20000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_PR_Pos      18UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_PR_Msk      0x40000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_PW_Pos      19UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_PW_Msk      0x80000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_NS_Pos      20UL
#define PERI_MS_PPU_PR_MS_ATT3_PC14_NS_Msk      0x100000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_UR_Pos      24UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_UR_Msk      0x1000000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_UW_Pos      25UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_UW_Msk      0x2000000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_PR_Pos      26UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_PR_Msk      0x4000000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_PW_Pos      27UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_PW_Msk      0x8000000UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_NS_Pos      28UL
#define PERI_MS_PPU_PR_MS_ATT3_PC15_NS_Msk      0x10000000UL


/* PERI_MS_PPU_FX.SL_ADDR */
#define PERI_MS_PPU_FX_SL_ADDR_ADDR30_Pos       2UL
#define PERI_MS_PPU_FX_SL_ADDR_ADDR30_Msk       0xFFFFFFFCUL
/* PERI_MS_PPU_FX.SL_SIZE */
#define PERI_MS_PPU_FX_SL_SIZE_REGION_SIZE_Pos  24UL
#define PERI_MS_PPU_FX_SL_SIZE_REGION_SIZE_Msk  0x1F000000UL
#define PERI_MS_PPU_FX_SL_SIZE_VALID_Pos        31UL
#define PERI_MS_PPU_FX_SL_SIZE_VALID_Msk        0x80000000UL
/* PERI_MS_PPU_FX.SL_ATT0 */
#define PERI_MS_PPU_FX_SL_ATT0_PC0_UR_Pos       0UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_UW_Pos       1UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_PR_Pos       2UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_PW_Pos       3UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_NS_Pos       4UL
#define PERI_MS_PPU_FX_SL_ATT0_PC0_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_UR_Pos       8UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_UW_Pos       9UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_PR_Pos       10UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_PW_Pos       11UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_NS_Pos       12UL
#define PERI_MS_PPU_FX_SL_ATT0_PC1_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_UR_Pos       16UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_UR_Msk       0x10000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_UW_Pos       17UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_UW_Msk       0x20000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_PR_Pos       18UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_PR_Msk       0x40000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_PW_Pos       19UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_PW_Msk       0x80000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_NS_Pos       20UL
#define PERI_MS_PPU_FX_SL_ATT0_PC2_NS_Msk       0x100000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_UR_Pos       24UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_UR_Msk       0x1000000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_UW_Pos       25UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_UW_Msk       0x2000000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_PR_Pos       26UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_PR_Msk       0x4000000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_PW_Pos       27UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_PW_Msk       0x8000000UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_NS_Pos       28UL
#define PERI_MS_PPU_FX_SL_ATT0_PC3_NS_Msk       0x10000000UL
/* PERI_MS_PPU_FX.SL_ATT1 */
#define PERI_MS_PPU_FX_SL_ATT1_PC4_UR_Pos       0UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_UW_Pos       1UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_PR_Pos       2UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_PW_Pos       3UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_NS_Pos       4UL
#define PERI_MS_PPU_FX_SL_ATT1_PC4_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_UR_Pos       8UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_UW_Pos       9UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_PR_Pos       10UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_PW_Pos       11UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_NS_Pos       12UL
#define PERI_MS_PPU_FX_SL_ATT1_PC5_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_UR_Pos       16UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_UR_Msk       0x10000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_UW_Pos       17UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_UW_Msk       0x20000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_PR_Pos       18UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_PR_Msk       0x40000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_PW_Pos       19UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_PW_Msk       0x80000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_NS_Pos       20UL
#define PERI_MS_PPU_FX_SL_ATT1_PC6_NS_Msk       0x100000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_UR_Pos       24UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_UR_Msk       0x1000000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_UW_Pos       25UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_UW_Msk       0x2000000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_PR_Pos       26UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_PR_Msk       0x4000000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_PW_Pos       27UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_PW_Msk       0x8000000UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_NS_Pos       28UL
#define PERI_MS_PPU_FX_SL_ATT1_PC7_NS_Msk       0x10000000UL
/* PERI_MS_PPU_FX.SL_ATT2 */
#define PERI_MS_PPU_FX_SL_ATT2_PC8_UR_Pos       0UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_UW_Pos       1UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_PR_Pos       2UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_PW_Pos       3UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_NS_Pos       4UL
#define PERI_MS_PPU_FX_SL_ATT2_PC8_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_UR_Pos       8UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_UW_Pos       9UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_PR_Pos       10UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_PW_Pos       11UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_NS_Pos       12UL
#define PERI_MS_PPU_FX_SL_ATT2_PC9_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_UR_Pos      16UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_UR_Msk      0x10000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_UW_Pos      17UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_UW_Msk      0x20000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_PR_Pos      18UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_PR_Msk      0x40000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_PW_Pos      19UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_PW_Msk      0x80000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_NS_Pos      20UL
#define PERI_MS_PPU_FX_SL_ATT2_PC10_NS_Msk      0x100000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_UR_Pos      24UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_UR_Msk      0x1000000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_UW_Pos      25UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_UW_Msk      0x2000000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_PR_Pos      26UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_PR_Msk      0x4000000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_PW_Pos      27UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_PW_Msk      0x8000000UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_NS_Pos      28UL
#define PERI_MS_PPU_FX_SL_ATT2_PC11_NS_Msk      0x10000000UL
/* PERI_MS_PPU_FX.SL_ATT3 */
#define PERI_MS_PPU_FX_SL_ATT3_PC12_UR_Pos      0UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_UR_Msk      0x1UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_UW_Pos      1UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_UW_Msk      0x2UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_PR_Pos      2UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_PR_Msk      0x4UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_PW_Pos      3UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_PW_Msk      0x8UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_NS_Pos      4UL
#define PERI_MS_PPU_FX_SL_ATT3_PC12_NS_Msk      0x10UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_UR_Pos      8UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_UR_Msk      0x100UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_UW_Pos      9UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_UW_Msk      0x200UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_PR_Pos      10UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_PR_Msk      0x400UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_PW_Pos      11UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_PW_Msk      0x800UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_NS_Pos      12UL
#define PERI_MS_PPU_FX_SL_ATT3_PC13_NS_Msk      0x1000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_UR_Pos      16UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_UR_Msk      0x10000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_UW_Pos      17UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_UW_Msk      0x20000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_PR_Pos      18UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_PR_Msk      0x40000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_PW_Pos      19UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_PW_Msk      0x80000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_NS_Pos      20UL
#define PERI_MS_PPU_FX_SL_ATT3_PC14_NS_Msk      0x100000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_UR_Pos      24UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_UR_Msk      0x1000000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_UW_Pos      25UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_UW_Msk      0x2000000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_PR_Pos      26UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_PR_Msk      0x4000000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_PW_Pos      27UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_PW_Msk      0x8000000UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_NS_Pos      28UL
#define PERI_MS_PPU_FX_SL_ATT3_PC15_NS_Msk      0x10000000UL
/* PERI_MS_PPU_FX.MS_ADDR */
#define PERI_MS_PPU_FX_MS_ADDR_ADDR26_Pos       6UL
#define PERI_MS_PPU_FX_MS_ADDR_ADDR26_Msk       0xFFFFFFC0UL
/* PERI_MS_PPU_FX.MS_SIZE */
#define PERI_MS_PPU_FX_MS_SIZE_REGION_SIZE_Pos  24UL
#define PERI_MS_PPU_FX_MS_SIZE_REGION_SIZE_Msk  0x1F000000UL
#define PERI_MS_PPU_FX_MS_SIZE_VALID_Pos        31UL
#define PERI_MS_PPU_FX_MS_SIZE_VALID_Msk        0x80000000UL
/* PERI_MS_PPU_FX.MS_ATT0 */
#define PERI_MS_PPU_FX_MS_ATT0_PC0_UR_Pos       0UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_UW_Pos       1UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_PR_Pos       2UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_PW_Pos       3UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_NS_Pos       4UL
#define PERI_MS_PPU_FX_MS_ATT0_PC0_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_UR_Pos       8UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_UW_Pos       9UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_PR_Pos       10UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_PW_Pos       11UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_NS_Pos       12UL
#define PERI_MS_PPU_FX_MS_ATT0_PC1_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_UR_Pos       16UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_UR_Msk       0x10000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_UW_Pos       17UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_UW_Msk       0x20000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_PR_Pos       18UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_PR_Msk       0x40000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_PW_Pos       19UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_PW_Msk       0x80000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_NS_Pos       20UL
#define PERI_MS_PPU_FX_MS_ATT0_PC2_NS_Msk       0x100000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_UR_Pos       24UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_UR_Msk       0x1000000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_UW_Pos       25UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_UW_Msk       0x2000000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_PR_Pos       26UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_PR_Msk       0x4000000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_PW_Pos       27UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_PW_Msk       0x8000000UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_NS_Pos       28UL
#define PERI_MS_PPU_FX_MS_ATT0_PC3_NS_Msk       0x10000000UL
/* PERI_MS_PPU_FX.MS_ATT1 */
#define PERI_MS_PPU_FX_MS_ATT1_PC4_UR_Pos       0UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_UW_Pos       1UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_PR_Pos       2UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_PW_Pos       3UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_NS_Pos       4UL
#define PERI_MS_PPU_FX_MS_ATT1_PC4_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_UR_Pos       8UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_UW_Pos       9UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_PR_Pos       10UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_PW_Pos       11UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_NS_Pos       12UL
#define PERI_MS_PPU_FX_MS_ATT1_PC5_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_UR_Pos       16UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_UR_Msk       0x10000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_UW_Pos       17UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_UW_Msk       0x20000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_PR_Pos       18UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_PR_Msk       0x40000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_PW_Pos       19UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_PW_Msk       0x80000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_NS_Pos       20UL
#define PERI_MS_PPU_FX_MS_ATT1_PC6_NS_Msk       0x100000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_UR_Pos       24UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_UR_Msk       0x1000000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_UW_Pos       25UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_UW_Msk       0x2000000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_PR_Pos       26UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_PR_Msk       0x4000000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_PW_Pos       27UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_PW_Msk       0x8000000UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_NS_Pos       28UL
#define PERI_MS_PPU_FX_MS_ATT1_PC7_NS_Msk       0x10000000UL
/* PERI_MS_PPU_FX.MS_ATT2 */
#define PERI_MS_PPU_FX_MS_ATT2_PC8_UR_Pos       0UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_UR_Msk       0x1UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_UW_Pos       1UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_UW_Msk       0x2UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_PR_Pos       2UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_PR_Msk       0x4UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_PW_Pos       3UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_PW_Msk       0x8UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_NS_Pos       4UL
#define PERI_MS_PPU_FX_MS_ATT2_PC8_NS_Msk       0x10UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_UR_Pos       8UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_UR_Msk       0x100UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_UW_Pos       9UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_UW_Msk       0x200UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_PR_Pos       10UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_PR_Msk       0x400UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_PW_Pos       11UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_PW_Msk       0x800UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_NS_Pos       12UL
#define PERI_MS_PPU_FX_MS_ATT2_PC9_NS_Msk       0x1000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_UR_Pos      16UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_UR_Msk      0x10000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_UW_Pos      17UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_UW_Msk      0x20000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_PR_Pos      18UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_PR_Msk      0x40000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_PW_Pos      19UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_PW_Msk      0x80000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_NS_Pos      20UL
#define PERI_MS_PPU_FX_MS_ATT2_PC10_NS_Msk      0x100000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_UR_Pos      24UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_UR_Msk      0x1000000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_UW_Pos      25UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_UW_Msk      0x2000000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_PR_Pos      26UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_PR_Msk      0x4000000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_PW_Pos      27UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_PW_Msk      0x8000000UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_NS_Pos      28UL
#define PERI_MS_PPU_FX_MS_ATT2_PC11_NS_Msk      0x10000000UL
/* PERI_MS_PPU_FX.MS_ATT3 */
#define PERI_MS_PPU_FX_MS_ATT3_PC12_UR_Pos      0UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_UR_Msk      0x1UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_UW_Pos      1UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_UW_Msk      0x2UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_PR_Pos      2UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_PR_Msk      0x4UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_PW_Pos      3UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_PW_Msk      0x8UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_NS_Pos      4UL
#define PERI_MS_PPU_FX_MS_ATT3_PC12_NS_Msk      0x10UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_UR_Pos      8UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_UR_Msk      0x100UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_UW_Pos      9UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_UW_Msk      0x200UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_PR_Pos      10UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_PR_Msk      0x400UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_PW_Pos      11UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_PW_Msk      0x800UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_NS_Pos      12UL
#define PERI_MS_PPU_FX_MS_ATT3_PC13_NS_Msk      0x1000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_UR_Pos      16UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_UR_Msk      0x10000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_UW_Pos      17UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_UW_Msk      0x20000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_PR_Pos      18UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_PR_Msk      0x40000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_PW_Pos      19UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_PW_Msk      0x80000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_NS_Pos      20UL
#define PERI_MS_PPU_FX_MS_ATT3_PC14_NS_Msk      0x100000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_UR_Pos      24UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_UR_Msk      0x1000000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_UW_Pos      25UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_UW_Msk      0x2000000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_PR_Pos      26UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_PR_Msk      0x4000000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_PW_Pos      27UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_PW_Msk      0x8000000UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_NS_Pos      28UL
#define PERI_MS_PPU_FX_MS_ATT3_PC15_NS_Msk      0x10000000UL


#endif /* _CYIP_PERI_MS_V3_H_ */


/* [] END OF FILE */
