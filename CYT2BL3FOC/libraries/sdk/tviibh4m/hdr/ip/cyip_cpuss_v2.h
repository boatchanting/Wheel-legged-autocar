/***************************************************************************//**
* \file cyip_cpuss_v2.h
*
* \brief
* CPUSS IP definitions
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

#ifndef _CYIP_CPUSS_V2_H_
#define _CYIP_CPUSS_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    CPUSS
*******************************************************************************/

#define CPUSS_SECTION_SIZE                      0x00010000UL

/**
  * \brief Identity (CPUSS_IDENTITY)
  */
typedef struct stc_CPUSS_IDENTITY_field {
  uint32_t                         u1P:1;
  uint32_t                         u1NS:1;
  uint32_t                         :2;
  uint32_t                         u4PC:4;
  uint32_t                         u4MS:4;
  uint32_t                         :20;
} stc_CPUSS_IDENTITY_field_t;

typedef union un_CPUSS_IDENTITY {
  uint32_t                         u32Register;
  stc_CPUSS_IDENTITY_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_IDENTITY_t;

/**
  * \brief CM4 status (CPUSS_CM4_STATUS)
  */
typedef struct stc_CPUSS_CM4_STATUS_field {
  uint32_t                         u1SLEEPING:1;
  uint32_t                         u1SLEEPDEEP:1;
  uint32_t                         :2;
  uint32_t                         u1PWR_DONE:1;
  uint32_t                         :27;
} stc_CPUSS_CM4_STATUS_field_t;

typedef union un_CPUSS_CM4_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_STATUS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_STATUS_t;

/**
  * \brief CM4 clock control (CPUSS_CM4_CLOCK_CTL)
  */
typedef struct stc_CPUSS_CM4_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8FAST_INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_CM4_CLOCK_CTL_field_t;

typedef union un_CPUSS_CM4_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_CLOCK_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_CLOCK_CTL_t;

/**
  * \brief CM4 control (CPUSS_CM4_CTL)
  */
typedef struct stc_CPUSS_CM4_CTL_field {
  uint32_t                         :24;
  uint32_t                         u1IOC_MASK:1;
  uint32_t                         u1DZC_MASK:1;
  uint32_t                         u1OFC_MASK:1;
  uint32_t                         u1UFC_MASK:1;
  uint32_t                         u1IXC_MASK:1;
  uint32_t                         :2;
  uint32_t                         u1IDC_MASK:1;
} stc_CPUSS_CM4_CTL_field_t;

typedef union un_CPUSS_CM4_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_CTL_t;

/**
  * \brief CM4 interrupt 0 status (CPUSS_CM4_INT0_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT0_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT0_STATUS_field_t;

typedef union un_CPUSS_CM4_INT0_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT0_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT0_STATUS_t;

/**
  * \brief CM4 interrupt 1 status (CPUSS_CM4_INT1_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT1_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT1_STATUS_field_t;

typedef union un_CPUSS_CM4_INT1_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT1_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT1_STATUS_t;

/**
  * \brief CM4 interrupt 2 status (CPUSS_CM4_INT2_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT2_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT2_STATUS_field_t;

typedef union un_CPUSS_CM4_INT2_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT2_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT2_STATUS_t;

/**
  * \brief CM4 interrupt 3 status (CPUSS_CM4_INT3_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT3_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT3_STATUS_field_t;

typedef union un_CPUSS_CM4_INT3_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT3_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT3_STATUS_t;

/**
  * \brief CM4 interrupt 4 status (CPUSS_CM4_INT4_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT4_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT4_STATUS_field_t;

typedef union un_CPUSS_CM4_INT4_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT4_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT4_STATUS_t;

/**
  * \brief CM4 interrupt 5 status (CPUSS_CM4_INT5_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT5_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT5_STATUS_field_t;

typedef union un_CPUSS_CM4_INT5_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT5_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT5_STATUS_t;

/**
  * \brief CM4 interrupt 6 status (CPUSS_CM4_INT6_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT6_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT6_STATUS_field_t;

typedef union un_CPUSS_CM4_INT6_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT6_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT6_STATUS_t;

/**
  * \brief CM4 interrupt 7 status (CPUSS_CM4_INT7_STATUS)
  */
typedef struct stc_CPUSS_CM4_INT7_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM4_INT7_STATUS_field_t;

typedef union un_CPUSS_CM4_INT7_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_INT7_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_INT7_STATUS_t;

/**
  * \brief CM4 vector table base (CPUSS_CM4_VECTOR_TABLE_BASE)
  */
typedef struct stc_CPUSS_CM4_VECTOR_TABLE_BASE_field {
  uint32_t                         :10;
  uint32_t                         u22ADDR22:22;
} stc_CPUSS_CM4_VECTOR_TABLE_BASE_field_t;

typedef union un_CPUSS_CM4_VECTOR_TABLE_BASE {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_VECTOR_TABLE_BASE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_VECTOR_TABLE_BASE_t;

/**
  * \brief CM4 NMI control (CPUSS_CM4_NMI_CTL)
  */
typedef struct stc_CPUSS_CM4_NMI_CTL_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :22;
} stc_CPUSS_CM4_NMI_CTL_field_t;

typedef union un_CPUSS_CM4_NMI_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_NMI_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_NMI_CTL_t;

/**
  * \brief UDB power control (CPUSS_UDB_PWR_CTL)
  */
typedef struct stc_CPUSS_UDB_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_UDB_PWR_CTL_field_t;

typedef union un_CPUSS_UDB_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_UDB_PWR_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_UDB_PWR_CTL_t;

/**
  * \brief UDB power control (CPUSS_UDB_PWR_DELAY_CTL)
  */
typedef struct stc_CPUSS_UDB_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CPUSS_UDB_PWR_DELAY_CTL_field_t;

typedef union un_CPUSS_UDB_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_UDB_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_UDB_PWR_DELAY_CTL_t;

/**
  * \brief CM0+ control (CPUSS_CM0_CTL)
  */
typedef struct stc_CPUSS_CM0_CTL_field {
  uint32_t                         u1SLV_STALL:1;
  uint32_t                         u1ENABLED:1;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_CM0_CTL_field_t;

typedef union un_CPUSS_CM0_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_CTL_t;

/**
  * \brief CM0+ status (CPUSS_CM0_STATUS)
  */
typedef struct stc_CPUSS_CM0_STATUS_field {
  uint32_t                         u1SLEEPING:1;
  uint32_t                         u1SLEEPDEEP:1;
  uint32_t                         :30;
} stc_CPUSS_CM0_STATUS_field_t;

typedef union un_CPUSS_CM0_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_STATUS_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_STATUS_t;

/**
  * \brief CM0+ clock control (CPUSS_CM0_CLOCK_CTL)
  */
typedef struct stc_CPUSS_CM0_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8SLOW_INT_DIV:8;
  uint32_t                         :8;
  uint32_t                         u8PERI_INT_DIV:8;
} stc_CPUSS_CM0_CLOCK_CTL_field_t;

typedef union un_CPUSS_CM0_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_CLOCK_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_CLOCK_CTL_t;

/**
  * \brief CM0+ interrupt 0 status (CPUSS_CM0_INT0_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT0_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT0_STATUS_field_t;

typedef union un_CPUSS_CM0_INT0_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT0_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT0_STATUS_t;

/**
  * \brief CM0+ interrupt 1 status (CPUSS_CM0_INT1_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT1_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT1_STATUS_field_t;

typedef union un_CPUSS_CM0_INT1_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT1_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT1_STATUS_t;

/**
  * \brief CM0+ interrupt 2 status (CPUSS_CM0_INT2_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT2_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT2_STATUS_field_t;

typedef union un_CPUSS_CM0_INT2_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT2_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT2_STATUS_t;

/**
  * \brief CM0+ interrupt 3 status (CPUSS_CM0_INT3_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT3_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT3_STATUS_field_t;

typedef union un_CPUSS_CM0_INT3_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT3_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT3_STATUS_t;

/**
  * \brief CM0+ interrupt 4 status (CPUSS_CM0_INT4_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT4_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT4_STATUS_field_t;

typedef union un_CPUSS_CM0_INT4_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT4_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT4_STATUS_t;

/**
  * \brief CM0+ interrupt 5 status (CPUSS_CM0_INT5_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT5_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT5_STATUS_field_t;

typedef union un_CPUSS_CM0_INT5_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT5_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT5_STATUS_t;

/**
  * \brief CM0+ interrupt 6 status (CPUSS_CM0_INT6_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT6_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT6_STATUS_field_t;

typedef union un_CPUSS_CM0_INT6_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT6_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT6_STATUS_t;

/**
  * \brief CM0+ interrupt 7 status (CPUSS_CM0_INT7_STATUS)
  */
typedef struct stc_CPUSS_CM0_INT7_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM0_INT7_STATUS_field_t;

typedef union un_CPUSS_CM0_INT7_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_INT7_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_INT7_STATUS_t;

/**
  * \brief CM0+ vector table base (CPUSS_CM0_VECTOR_TABLE_BASE)
  */
typedef struct stc_CPUSS_CM0_VECTOR_TABLE_BASE_field {
  uint32_t                         :8;
  uint32_t                         u24ADDR24:24;
} stc_CPUSS_CM0_VECTOR_TABLE_BASE_field_t;

typedef union un_CPUSS_CM0_VECTOR_TABLE_BASE {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_VECTOR_TABLE_BASE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_VECTOR_TABLE_BASE_t;

/**
  * \brief CM0+ NMI control (CPUSS_CM0_NMI_CTL)
  */
typedef struct stc_CPUSS_CM0_NMI_CTL_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :22;
} stc_CPUSS_CM0_NMI_CTL_field_t;

typedef union un_CPUSS_CM0_NMI_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_NMI_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_NMI_CTL_t;

/**
  * \brief CM4 power control (CPUSS_CM4_PWR_CTL)
  */
typedef struct stc_CPUSS_CM4_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_CM4_PWR_CTL_field_t;

typedef union un_CPUSS_CM4_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_PWR_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_PWR_CTL_t;

/**
  * \brief CM4 power control (CPUSS_CM4_PWR_DELAY_CTL)
  */
typedef struct stc_CPUSS_CM4_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CPUSS_CM4_PWR_DELAY_CTL_field_t;

typedef union un_CPUSS_CM4_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_PWR_DELAY_CTL_t;

/**
  * \brief RAM 0 control (CPUSS_RAM0_CTL0)
  */
typedef struct stc_CPUSS_RAM0_CTL0_field {
  uint32_t                         u2SLOW_WS:2;
  uint32_t                         :6;
  uint32_t                         u2FAST_WS:2;
  uint32_t                         :6;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         u1ECC_AUTO_CORRECT:1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :13;
} stc_CPUSS_RAM0_CTL0_field_t;

typedef union un_CPUSS_RAM0_CTL0 {
  uint32_t                         u32Register;
  stc_CPUSS_RAM0_CTL0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM0_CTL0_t;

/**
  * \brief RAM 0 status (CPUSS_RAM0_STATUS)
  */
typedef struct stc_CPUSS_RAM0_STATUS_field {
  uint32_t                         u1WB_EMPTY:1;
  uint32_t                         :31;
} stc_CPUSS_RAM0_STATUS_field_t;

typedef union un_CPUSS_RAM0_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_RAM0_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM0_STATUS_t;

/**
  * \brief RAM 0 power control (CPUSS_RAM0_PWR_MACRO_CTL)
  */
typedef struct stc_CPUSS_RAM0_PWR_MACRO_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_RAM0_PWR_MACRO_CTL_field_t;

typedef union un_CPUSS_RAM0_PWR_MACRO_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_RAM0_PWR_MACRO_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM0_PWR_MACRO_CTL_t;

/**
  * \brief RAM 1 control (CPUSS_RAM1_CTL0)
  */
typedef struct stc_CPUSS_RAM1_CTL0_field {
  uint32_t                         u2SLOW_WS:2;
  uint32_t                         :6;
  uint32_t                         u2FAST_WS:2;
  uint32_t                         :6;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         u1ECC_AUTO_CORRECT:1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :13;
} stc_CPUSS_RAM1_CTL0_field_t;

typedef union un_CPUSS_RAM1_CTL0 {
  uint32_t                         u32Register;
  stc_CPUSS_RAM1_CTL0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM1_CTL0_t;

/**
  * \brief RAM 1 status (CPUSS_RAM1_STATUS)
  */
typedef struct stc_CPUSS_RAM1_STATUS_field {
  uint32_t                         u1WB_EMPTY:1;
  uint32_t                         :31;
} stc_CPUSS_RAM1_STATUS_field_t;

typedef union un_CPUSS_RAM1_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_RAM1_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM1_STATUS_t;

/**
  * \brief RAM 1 power control (CPUSS_RAM1_PWR_CTL)
  */
typedef struct stc_CPUSS_RAM1_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_RAM1_PWR_CTL_field_t;

typedef union un_CPUSS_RAM1_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_RAM1_PWR_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM1_PWR_CTL_t;

/**
  * \brief RAM 2 control (CPUSS_RAM2_CTL0)
  */
typedef struct stc_CPUSS_RAM2_CTL0_field {
  uint32_t                         u2SLOW_WS:2;
  uint32_t                         :6;
  uint32_t                         u2FAST_WS:2;
  uint32_t                         :6;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         u1ECC_AUTO_CORRECT:1;
  uint32_t                         u1ECC_INJ_EN:1;
  uint32_t                         :13;
} stc_CPUSS_RAM2_CTL0_field_t;

typedef union un_CPUSS_RAM2_CTL0 {
  uint32_t                         u32Register;
  stc_CPUSS_RAM2_CTL0_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM2_CTL0_t;

/**
  * \brief RAM 2 status (CPUSS_RAM2_STATUS)
  */
typedef struct stc_CPUSS_RAM2_STATUS_field {
  uint32_t                         u1WB_EMPTY:1;
  uint32_t                         :31;
} stc_CPUSS_RAM2_STATUS_field_t;

typedef union un_CPUSS_RAM2_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_RAM2_STATUS_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM2_STATUS_t;

/**
  * \brief RAM 2 power control (CPUSS_RAM2_PWR_CTL)
  */
typedef struct stc_CPUSS_RAM2_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_RAM2_PWR_CTL_field_t;

typedef union un_CPUSS_RAM2_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_RAM2_PWR_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM2_PWR_CTL_t;

/**
  * \brief Power up delay used for all SRAM power domains (CPUSS_RAM_PWR_DELAY_CTL)
  */
typedef struct stc_CPUSS_RAM_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CPUSS_RAM_PWR_DELAY_CTL_field_t;

typedef union un_CPUSS_RAM_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_RAM_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_RAM_PWR_DELAY_CTL_t;

/**
  * \brief ROM control (CPUSS_ROM_CTL)
  */
typedef struct stc_CPUSS_ROM_CTL_field {
  uint32_t                         u2SLOW_WS:2;
  uint32_t                         :6;
  uint32_t                         u2FAST_WS:2;
  uint32_t                         :22;
} stc_CPUSS_ROM_CTL_field_t;

typedef union un_CPUSS_ROM_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_ROM_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_ROM_CTL_t;

/**
  * \brief ECC control (CPUSS_ECC_CTL)
  */
typedef struct stc_CPUSS_ECC_CTL_field {
  uint32_t                         u25WORD_ADDR:25;
  uint32_t                         u7PARITY:7;
} stc_CPUSS_ECC_CTL_field_t;

typedef union un_CPUSS_ECC_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_ECC_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_ECC_CTL_t;

/**
  * \brief Product identifier and version (same as CoreSight RomTables) (CPUSS_PRODUCT_ID)
  */
typedef struct stc_CPUSS_PRODUCT_ID_field {
  uint32_t                         u12FAMILY_ID:12;
  uint32_t                         :4;
  uint32_t                         u4MAJOR_REV:4;
  uint32_t                         u4MINOR_REV:4;
  uint32_t                         :8;
} stc_CPUSS_PRODUCT_ID_field_t;

typedef union un_CPUSS_PRODUCT_ID {
  uint32_t                         u32Register;
  stc_CPUSS_PRODUCT_ID_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_PRODUCT_ID_t;

/**
  * \brief Debug port status (CPUSS_DP_STATUS)
  */
typedef struct stc_CPUSS_DP_STATUS_field {
  uint32_t                         u1SWJ_CONNECTED:1;
  uint32_t                         u1SWJ_DEBUG_EN:1;
  uint32_t                         u1SWJ_JTAG_SEL:1;
  uint32_t                         :29;
} stc_CPUSS_DP_STATUS_field_t;

typedef union un_CPUSS_DP_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_DP_STATUS_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_DP_STATUS_t;

/**
  * \brief Access port control (CPUSS_AP_CTL)
  */
typedef struct stc_CPUSS_AP_CTL_field {
  uint32_t                         u1CM0_ENABLE:1;
  uint32_t                         u1CM4_ENABLE:1;
  uint32_t                         u1SYS_ENABLE:1;
  uint32_t                         :13;
  uint32_t                         u1CM0_DISABLE:1;
  uint32_t                         u1CM4_DISABLE:1;
  uint32_t                         u1SYS_DISABLE:1;
  uint32_t                         :13;
} stc_CPUSS_AP_CTL_field_t;

typedef union un_CPUSS_AP_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_AP_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_AP_CTL_t;

/**
  * \brief Buffer control (CPUSS_BUFF_CTL)
  */
typedef struct stc_CPUSS_BUFF_CTL_field {
  uint32_t                         u1WRITE_BUFF:1;
  uint32_t                         :31;
} stc_CPUSS_BUFF_CTL_field_t;

typedef union un_CPUSS_BUFF_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_BUFF_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_BUFF_CTL_t;

/**
  * \brief SysTick timer control (CPUSS_SYSTICK_CTL)
  */
typedef struct stc_CPUSS_SYSTICK_CTL_field {
  uint32_t                         u24TENMS:24;
  uint32_t                         u2CLOCK_SOURCE:2;
  uint32_t                         :4;
  uint32_t                         u1SKEW:1;
  uint32_t                         u1NOREF:1;
} stc_CPUSS_SYSTICK_CTL_field_t;

typedef union un_CPUSS_SYSTICK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_SYSTICK_CTL_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_SYSTICK_CTL_t;

/**
  * \brief Memory BIST status (CPUSS_MBIST_STAT)
  */
typedef struct stc_CPUSS_MBIST_STAT_field {
  uint32_t                         u1SFP_READY:1;
  uint32_t                         u1SFP_FAIL:1;
  uint32_t                         :30;
} stc_CPUSS_MBIST_STAT_field_t;

typedef union un_CPUSS_MBIST_STAT {
  uint32_t                         u32Register;
  stc_CPUSS_MBIST_STAT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_MBIST_STAT_t;

/**
  * \brief Calibration support set and read (CPUSS_CAL_SUP_SET)
  */
typedef struct stc_CPUSS_CAL_SUP_SET_field {
  uint32_t                         u32DATA:32;
} stc_CPUSS_CAL_SUP_SET_field_t;

typedef union un_CPUSS_CAL_SUP_SET {
  uint32_t                         u32Register;
  stc_CPUSS_CAL_SUP_SET_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CAL_SUP_SET_t;

/**
  * \brief Calibration support clear and reset (CPUSS_CAL_SUP_CLR)
  */
typedef struct stc_CPUSS_CAL_SUP_CLR_field {
  uint32_t                         u32DATA:32;
} stc_CPUSS_CAL_SUP_CLR_field_t;

typedef union un_CPUSS_CAL_SUP_CLR {
  uint32_t                         u32Register;
  stc_CPUSS_CAL_SUP_CLR_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CAL_SUP_CLR_t;

/**
  * \brief CM0+ protection context control (CPUSS_CM0_PC_CTL)
  */
typedef struct stc_CPUSS_CM0_PC_CTL_field {
  uint32_t                         u4VALID:4;
  uint32_t                         :28;
} stc_CPUSS_CM0_PC_CTL_field_t;

typedef union un_CPUSS_CM0_PC_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_PC_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_PC_CTL_t;

/**
  * \brief CM0+ protection context 0 handler (CPUSS_CM0_PC0_HANDLER)
  */
typedef struct stc_CPUSS_CM0_PC0_HANDLER_field {
  uint32_t                         u32ADDR:32;
} stc_CPUSS_CM0_PC0_HANDLER_field_t;

typedef union un_CPUSS_CM0_PC0_HANDLER {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_PC0_HANDLER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_PC0_HANDLER_t;

/**
  * \brief CM0+ protection context 1 handler (CPUSS_CM0_PC1_HANDLER)
  */
typedef struct stc_CPUSS_CM0_PC1_HANDLER_field {
  uint32_t                         u32ADDR:32;
} stc_CPUSS_CM0_PC1_HANDLER_field_t;

typedef union un_CPUSS_CM0_PC1_HANDLER {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_PC1_HANDLER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_PC1_HANDLER_t;

/**
  * \brief CM0+ protection context 2 handler (CPUSS_CM0_PC2_HANDLER)
  */
typedef struct stc_CPUSS_CM0_PC2_HANDLER_field {
  uint32_t                         u32ADDR:32;
} stc_CPUSS_CM0_PC2_HANDLER_field_t;

typedef union un_CPUSS_CM0_PC2_HANDLER {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_PC2_HANDLER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_PC2_HANDLER_t;

/**
  * \brief CM0+ protection context 3 handler (CPUSS_CM0_PC3_HANDLER)
  */
typedef struct stc_CPUSS_CM0_PC3_HANDLER_field {
  uint32_t                         u32ADDR:32;
} stc_CPUSS_CM0_PC3_HANDLER_field_t;

typedef union un_CPUSS_CM0_PC3_HANDLER {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_PC3_HANDLER_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_PC3_HANDLER_t;

/**
  * \brief Protection status (CPUSS_PROTECTION)
  */
typedef struct stc_CPUSS_PROTECTION_field {
  uint32_t                         u3STATE:3;
  uint32_t                         :29;
} stc_CPUSS_PROTECTION_field_t;

typedef union un_CPUSS_PROTECTION {
  uint32_t                         u32Register;
  stc_CPUSS_PROTECTION_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_PROTECTION_t;

/**
  * \brief ROM trim control (CPUSS_TRIM_ROM_CTL)
  */
typedef struct stc_CPUSS_TRIM_ROM_CTL_field {
  uint32_t                         u32TRIM:32;
} stc_CPUSS_TRIM_ROM_CTL_field_t;

typedef union un_CPUSS_TRIM_ROM_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_TRIM_ROM_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_TRIM_ROM_CTL_t;

/**
  * \brief RAM trim control (CPUSS_TRIM_RAM_CTL)
  */
typedef struct stc_CPUSS_TRIM_RAM_CTL_field {
  uint32_t                         u32TRIM:32;
} stc_CPUSS_TRIM_RAM_CTL_field_t;

typedef union un_CPUSS_TRIM_RAM_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_TRIM_RAM_CTL_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_TRIM_RAM_CTL_t;

/**
  * \brief CM0+ system interrupt control (CPUSS_CM0_SYSTEM_INT_CTL)
  */
typedef struct stc_CPUSS_CM0_SYSTEM_INT_CTL_field {
  uint32_t                         u3CPU_INT_IDX:3;
  uint32_t                         :28;
  uint32_t                         u1CPU_INT_VALID:1;
} stc_CPUSS_CM0_SYSTEM_INT_CTL_field_t;

typedef union un_CPUSS_CM0_SYSTEM_INT_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM0_SYSTEM_INT_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM0_SYSTEM_INT_CTL_t;

/**
  * \brief CM4 system interrupt control (CPUSS_CM4_SYSTEM_INT_CTL)
  */
typedef struct stc_CPUSS_CM4_SYSTEM_INT_CTL_field {
  uint32_t                         u3CPU_INT_IDX:3;
  uint32_t                         :28;
  uint32_t                         u1CPU_INT_VALID:1;
} stc_CPUSS_CM4_SYSTEM_INT_CTL_field_t;

typedef union un_CPUSS_CM4_SYSTEM_INT_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM4_SYSTEM_INT_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM4_SYSTEM_INT_CTL_t;



/**
  * \brief CPU subsystem (CPUSS) (CPUSS)
  */
typedef struct stc_CPUSS {
  un_CPUSS_IDENTITY_t              unIDENTITY;         /*!< 0x00000000 Identity */
  un_CPUSS_CM4_STATUS_t            unCM4_STATUS;       /*!< 0x00000004 CM4 status */
  un_CPUSS_CM4_CLOCK_CTL_t         unCM4_CLOCK_CTL;    /*!< 0x00000008 CM4 clock control */
  un_CPUSS_CM4_CTL_t               unCM4_CTL;          /*!< 0x0000000C CM4 control */
  uint32_t                         au32Reserved[60];
  un_CPUSS_CM4_INT0_STATUS_t       unCM4_INT0_STATUS;  /*!< 0x00000100 CM4 interrupt 0 status */
  un_CPUSS_CM4_INT1_STATUS_t       unCM4_INT1_STATUS;  /*!< 0x00000104 CM4 interrupt 1 status */
  un_CPUSS_CM4_INT2_STATUS_t       unCM4_INT2_STATUS;  /*!< 0x00000108 CM4 interrupt 2 status */
  un_CPUSS_CM4_INT3_STATUS_t       unCM4_INT3_STATUS;  /*!< 0x0000010C CM4 interrupt 3 status */
  un_CPUSS_CM4_INT4_STATUS_t       unCM4_INT4_STATUS;  /*!< 0x00000110 CM4 interrupt 4 status */
  un_CPUSS_CM4_INT5_STATUS_t       unCM4_INT5_STATUS;  /*!< 0x00000114 CM4 interrupt 5 status */
  un_CPUSS_CM4_INT6_STATUS_t       unCM4_INT6_STATUS;  /*!< 0x00000118 CM4 interrupt 6 status */
  un_CPUSS_CM4_INT7_STATUS_t       unCM4_INT7_STATUS;  /*!< 0x0000011C CM4 interrupt 7 status */
  uint32_t                         au32Reserved1[56];
  un_CPUSS_CM4_VECTOR_TABLE_BASE_t unCM4_VECTOR_TABLE_BASE; /*!< 0x00000200 CM4 vector table base */
  uint32_t                         au32Reserved2[15];
  un_CPUSS_CM4_NMI_CTL_t           unCM4_NMI_CTL[4];   /*!< 0x00000240 CM4 NMI control */
  uint32_t                         au32Reserved3[44];
  un_CPUSS_UDB_PWR_CTL_t           unUDB_PWR_CTL;      /*!< 0x00000300 UDB power control */
  un_CPUSS_UDB_PWR_DELAY_CTL_t     unUDB_PWR_DELAY_CTL; /*!< 0x00000304 UDB power control */
  uint32_t                         au32Reserved4[830];
  un_CPUSS_CM0_CTL_t               unCM0_CTL;          /*!< 0x00001000 CM0+ control */
  un_CPUSS_CM0_STATUS_t            unCM0_STATUS;       /*!< 0x00001004 CM0+ status */
  un_CPUSS_CM0_CLOCK_CTL_t         unCM0_CLOCK_CTL;    /*!< 0x00001008 CM0+ clock control */
  uint32_t                         au32Reserved5[61];
  un_CPUSS_CM0_INT0_STATUS_t       unCM0_INT0_STATUS;  /*!< 0x00001100 CM0+ interrupt 0 status */
  un_CPUSS_CM0_INT1_STATUS_t       unCM0_INT1_STATUS;  /*!< 0x00001104 CM0+ interrupt 1 status */
  un_CPUSS_CM0_INT2_STATUS_t       unCM0_INT2_STATUS;  /*!< 0x00001108 CM0+ interrupt 2 status */
  un_CPUSS_CM0_INT3_STATUS_t       unCM0_INT3_STATUS;  /*!< 0x0000110C CM0+ interrupt 3 status */
  un_CPUSS_CM0_INT4_STATUS_t       unCM0_INT4_STATUS;  /*!< 0x00001110 CM0+ interrupt 4 status */
  un_CPUSS_CM0_INT5_STATUS_t       unCM0_INT5_STATUS;  /*!< 0x00001114 CM0+ interrupt 5 status */
  un_CPUSS_CM0_INT6_STATUS_t       unCM0_INT6_STATUS;  /*!< 0x00001118 CM0+ interrupt 6 status */
  un_CPUSS_CM0_INT7_STATUS_t       unCM0_INT7_STATUS;  /*!< 0x0000111C CM0+ interrupt 7 status */
  un_CPUSS_CM0_VECTOR_TABLE_BASE_t unCM0_VECTOR_TABLE_BASE; /*!< 0x00001120 CM0+ vector table base */
  uint32_t                         au32Reserved6[7];
  un_CPUSS_CM0_NMI_CTL_t           unCM0_NMI_CTL[4];   /*!< 0x00001140 CM0+ NMI control */
  uint32_t                         au32Reserved7[44];
  un_CPUSS_CM4_PWR_CTL_t           unCM4_PWR_CTL;      /*!< 0x00001200 CM4 power control */
  un_CPUSS_CM4_PWR_DELAY_CTL_t     unCM4_PWR_DELAY_CTL; /*!< 0x00001204 CM4 power control */
  uint32_t                         au32Reserved8[62];
  un_CPUSS_RAM0_CTL0_t             unRAM0_CTL0;        /*!< 0x00001300 RAM 0 control */
  un_CPUSS_RAM0_STATUS_t           unRAM0_STATUS;      /*!< 0x00001304 RAM 0 status */
  uint32_t                         au32Reserved9[14];
  un_CPUSS_RAM0_PWR_MACRO_CTL_t    unRAM0_PWR_MACRO_CTL[16]; /*!< 0x00001340 RAM 0 power control */
  un_CPUSS_RAM1_CTL0_t             unRAM1_CTL0;        /*!< 0x00001380 RAM 1 control */
  un_CPUSS_RAM1_STATUS_t           unRAM1_STATUS;      /*!< 0x00001384 RAM 1 status */
  un_CPUSS_RAM1_PWR_CTL_t          unRAM1_PWR_CTL;     /*!< 0x00001388 RAM 1 power control */
  uint32_t                         au32Reserved10[5];
  un_CPUSS_RAM2_CTL0_t             unRAM2_CTL0;        /*!< 0x000013A0 RAM 2 control */
  un_CPUSS_RAM2_STATUS_t           unRAM2_STATUS;      /*!< 0x000013A4 RAM 2 status */
  un_CPUSS_RAM2_PWR_CTL_t          unRAM2_PWR_CTL;     /*!< 0x000013A8 RAM 2 power control */
  uint32_t                         au32Reserved11[5];
  un_CPUSS_RAM_PWR_DELAY_CTL_t     unRAM_PWR_DELAY_CTL; /*!< 0x000013C0 Power up delay used for all SRAM power domains */
  un_CPUSS_ROM_CTL_t               unROM_CTL;          /*!< 0x000013C4 ROM control */
  un_CPUSS_ECC_CTL_t               unECC_CTL;          /*!< 0x000013C8 ECC control */
  uint32_t                         au32Reserved12[13];
  un_CPUSS_PRODUCT_ID_t            unPRODUCT_ID;       /*!< 0x00001400 Product identifier and version (same as CoreSight
                                                                                RomTables) */
  uint32_t                         au32Reserved13[3];
  un_CPUSS_DP_STATUS_t             unDP_STATUS;        /*!< 0x00001410 Debug port status */
  un_CPUSS_AP_CTL_t                unAP_CTL;           /*!< 0x00001414 Access port control */
  uint32_t                         au32Reserved14[58];
  un_CPUSS_BUFF_CTL_t              unBUFF_CTL;         /*!< 0x00001500 Buffer control */
  uint32_t                         au32Reserved15[63];
  un_CPUSS_SYSTICK_CTL_t           unSYSTICK_CTL;      /*!< 0x00001600 SysTick timer control */
  uint32_t                         au32Reserved16[64];
  un_CPUSS_MBIST_STAT_t            unMBIST_STAT;       /*!< 0x00001704 Memory BIST status */
  uint32_t                         au32Reserved17[62];
  un_CPUSS_CAL_SUP_SET_t           unCAL_SUP_SET;      /*!< 0x00001800 Calibration support set and read */
  un_CPUSS_CAL_SUP_CLR_t           unCAL_SUP_CLR;      /*!< 0x00001804 Calibration support clear and reset */
  uint32_t                         au32Reserved18[510];
  un_CPUSS_CM0_PC_CTL_t            unCM0_PC_CTL;       /*!< 0x00002000 CM0+ protection context control */
  uint32_t                         au32Reserved19[15];
  un_CPUSS_CM0_PC0_HANDLER_t       unCM0_PC0_HANDLER;  /*!< 0x00002040 CM0+ protection context 0 handler */
  un_CPUSS_CM0_PC1_HANDLER_t       unCM0_PC1_HANDLER;  /*!< 0x00002044 CM0+ protection context 1 handler */
  un_CPUSS_CM0_PC2_HANDLER_t       unCM0_PC2_HANDLER;  /*!< 0x00002048 CM0+ protection context 2 handler */
  un_CPUSS_CM0_PC3_HANDLER_t       unCM0_PC3_HANDLER;  /*!< 0x0000204C CM0+ protection context 3 handler */
  uint32_t                         au32Reserved20[29];
  un_CPUSS_PROTECTION_t            unPROTECTION;       /*!< 0x000020C4 Protection status */
  uint32_t                         au32Reserved21[14];
  un_CPUSS_TRIM_ROM_CTL_t          unTRIM_ROM_CTL;     /*!< 0x00002100 ROM trim control */
  un_CPUSS_TRIM_RAM_CTL_t          unTRIM_RAM_CTL;     /*!< 0x00002104 RAM trim control */
  uint32_t                         au32Reserved22[6078];
  un_CPUSS_CM0_SYSTEM_INT_CTL_t    unCM0_SYSTEM_INT_CTL[1023]; /*!< 0x00008000 CM0+ system interrupt control */
  uint32_t                         au32Reserved23[1025];
  un_CPUSS_CM4_SYSTEM_INT_CTL_t    unCM4_SYSTEM_INT_CTL[1023]; /*!< 0x0000A000 CM4 system interrupt control */
} stc_CPUSS_t;                                         /*!< Size = 45052 (0xAFFC) */


/* CPUSS.IDENTITY */
#define CPUSS_IDENTITY_P_Pos                    0UL
#define CPUSS_IDENTITY_P_Msk                    0x1UL
#define CPUSS_IDENTITY_NS_Pos                   1UL
#define CPUSS_IDENTITY_NS_Msk                   0x2UL
#define CPUSS_IDENTITY_PC_Pos                   4UL
#define CPUSS_IDENTITY_PC_Msk                   0xF0UL
#define CPUSS_IDENTITY_MS_Pos                   8UL
#define CPUSS_IDENTITY_MS_Msk                   0xF00UL
/* CPUSS.CM4_STATUS */
#define CPUSS_CM4_STATUS_SLEEPING_Pos           0UL
#define CPUSS_CM4_STATUS_SLEEPING_Msk           0x1UL
#define CPUSS_CM4_STATUS_SLEEPDEEP_Pos          1UL
#define CPUSS_CM4_STATUS_SLEEPDEEP_Msk          0x2UL
#define CPUSS_CM4_STATUS_PWR_DONE_Pos           4UL
#define CPUSS_CM4_STATUS_PWR_DONE_Msk           0x10UL
/* CPUSS.CM4_CLOCK_CTL */
#define CPUSS_CM4_CLOCK_CTL_FAST_INT_DIV_Pos    8UL
#define CPUSS_CM4_CLOCK_CTL_FAST_INT_DIV_Msk    0xFF00UL
/* CPUSS.CM4_CTL */
#define CPUSS_CM4_CTL_IOC_MASK_Pos              24UL
#define CPUSS_CM4_CTL_IOC_MASK_Msk              0x1000000UL
#define CPUSS_CM4_CTL_DZC_MASK_Pos              25UL
#define CPUSS_CM4_CTL_DZC_MASK_Msk              0x2000000UL
#define CPUSS_CM4_CTL_OFC_MASK_Pos              26UL
#define CPUSS_CM4_CTL_OFC_MASK_Msk              0x4000000UL
#define CPUSS_CM4_CTL_UFC_MASK_Pos              27UL
#define CPUSS_CM4_CTL_UFC_MASK_Msk              0x8000000UL
#define CPUSS_CM4_CTL_IXC_MASK_Pos              28UL
#define CPUSS_CM4_CTL_IXC_MASK_Msk              0x10000000UL
#define CPUSS_CM4_CTL_IDC_MASK_Pos              31UL
#define CPUSS_CM4_CTL_IDC_MASK_Msk              0x80000000UL
/* CPUSS.CM4_INT0_STATUS */
#define CPUSS_CM4_INT0_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT0_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT0_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT0_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT1_STATUS */
#define CPUSS_CM4_INT1_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT1_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT1_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT1_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT2_STATUS */
#define CPUSS_CM4_INT2_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT2_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT2_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT2_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT3_STATUS */
#define CPUSS_CM4_INT3_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT3_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT3_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT3_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT4_STATUS */
#define CPUSS_CM4_INT4_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT4_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT4_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT4_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT5_STATUS */
#define CPUSS_CM4_INT5_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT5_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT5_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT5_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT6_STATUS */
#define CPUSS_CM4_INT6_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT6_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT6_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT6_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_INT7_STATUS */
#define CPUSS_CM4_INT7_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM4_INT7_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM4_INT7_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM4_INT7_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_VECTOR_TABLE_BASE */
#define CPUSS_CM4_VECTOR_TABLE_BASE_ADDR22_Pos  10UL
#define CPUSS_CM4_VECTOR_TABLE_BASE_ADDR22_Msk  0xFFFFFC00UL
/* CPUSS.CM4_NMI_CTL */
#define CPUSS_CM4_NMI_CTL_SYSTEM_INT_IDX_Pos    0UL
#define CPUSS_CM4_NMI_CTL_SYSTEM_INT_IDX_Msk    0x3FFUL
/* CPUSS.UDB_PWR_CTL */
#define CPUSS_UDB_PWR_CTL_PWR_MODE_Pos          0UL
#define CPUSS_UDB_PWR_CTL_PWR_MODE_Msk          0x3UL
#define CPUSS_UDB_PWR_CTL_VECTKEYSTAT_Pos       16UL
#define CPUSS_UDB_PWR_CTL_VECTKEYSTAT_Msk       0xFFFF0000UL
/* CPUSS.UDB_PWR_DELAY_CTL */
#define CPUSS_UDB_PWR_DELAY_CTL_UP_Pos          0UL
#define CPUSS_UDB_PWR_DELAY_CTL_UP_Msk          0x3FFUL
/* CPUSS.CM0_CTL */
#define CPUSS_CM0_CTL_SLV_STALL_Pos             0UL
#define CPUSS_CM0_CTL_SLV_STALL_Msk             0x1UL
#define CPUSS_CM0_CTL_ENABLED_Pos               1UL
#define CPUSS_CM0_CTL_ENABLED_Msk               0x2UL
#define CPUSS_CM0_CTL_VECTKEYSTAT_Pos           16UL
#define CPUSS_CM0_CTL_VECTKEYSTAT_Msk           0xFFFF0000UL
/* CPUSS.CM0_STATUS */
#define CPUSS_CM0_STATUS_SLEEPING_Pos           0UL
#define CPUSS_CM0_STATUS_SLEEPING_Msk           0x1UL
#define CPUSS_CM0_STATUS_SLEEPDEEP_Pos          1UL
#define CPUSS_CM0_STATUS_SLEEPDEEP_Msk          0x2UL
/* CPUSS.CM0_CLOCK_CTL */
#define CPUSS_CM0_CLOCK_CTL_SLOW_INT_DIV_Pos    8UL
#define CPUSS_CM0_CLOCK_CTL_SLOW_INT_DIV_Msk    0xFF00UL
#define CPUSS_CM0_CLOCK_CTL_PERI_INT_DIV_Pos    24UL
#define CPUSS_CM0_CLOCK_CTL_PERI_INT_DIV_Msk    0xFF000000UL
/* CPUSS.CM0_INT0_STATUS */
#define CPUSS_CM0_INT0_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT0_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT0_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT0_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT1_STATUS */
#define CPUSS_CM0_INT1_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT1_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT1_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT1_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT2_STATUS */
#define CPUSS_CM0_INT2_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT2_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT2_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT2_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT3_STATUS */
#define CPUSS_CM0_INT3_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT3_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT3_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT3_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT4_STATUS */
#define CPUSS_CM0_INT4_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT4_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT4_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT4_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT5_STATUS */
#define CPUSS_CM0_INT5_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT5_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT5_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT5_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT6_STATUS */
#define CPUSS_CM0_INT6_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT6_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT6_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT6_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_INT7_STATUS */
#define CPUSS_CM0_INT7_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM0_INT7_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM0_INT7_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM0_INT7_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM0_VECTOR_TABLE_BASE */
#define CPUSS_CM0_VECTOR_TABLE_BASE_ADDR24_Pos  8UL
#define CPUSS_CM0_VECTOR_TABLE_BASE_ADDR24_Msk  0xFFFFFF00UL
/* CPUSS.CM0_NMI_CTL */
#define CPUSS_CM0_NMI_CTL_SYSTEM_INT_IDX_Pos    0UL
#define CPUSS_CM0_NMI_CTL_SYSTEM_INT_IDX_Msk    0x3FFUL
/* CPUSS.CM4_PWR_CTL */
#define CPUSS_CM4_PWR_CTL_PWR_MODE_Pos          0UL
#define CPUSS_CM4_PWR_CTL_PWR_MODE_Msk          0x3UL
#define CPUSS_CM4_PWR_CTL_VECTKEYSTAT_Pos       16UL
#define CPUSS_CM4_PWR_CTL_VECTKEYSTAT_Msk       0xFFFF0000UL
/* CPUSS.CM4_PWR_DELAY_CTL */
#define CPUSS_CM4_PWR_DELAY_CTL_UP_Pos          0UL
#define CPUSS_CM4_PWR_DELAY_CTL_UP_Msk          0x3FFUL
/* CPUSS.RAM0_CTL0 */
#define CPUSS_RAM0_CTL0_SLOW_WS_Pos             0UL
#define CPUSS_RAM0_CTL0_SLOW_WS_Msk             0x3UL
#define CPUSS_RAM0_CTL0_FAST_WS_Pos             8UL
#define CPUSS_RAM0_CTL0_FAST_WS_Msk             0x300UL
#define CPUSS_RAM0_CTL0_ECC_EN_Pos              16UL
#define CPUSS_RAM0_CTL0_ECC_EN_Msk              0x10000UL
#define CPUSS_RAM0_CTL0_ECC_AUTO_CORRECT_Pos    17UL
#define CPUSS_RAM0_CTL0_ECC_AUTO_CORRECT_Msk    0x20000UL
#define CPUSS_RAM0_CTL0_ECC_INJ_EN_Pos          18UL
#define CPUSS_RAM0_CTL0_ECC_INJ_EN_Msk          0x40000UL
/* CPUSS.RAM0_STATUS */
#define CPUSS_RAM0_STATUS_WB_EMPTY_Pos          0UL
#define CPUSS_RAM0_STATUS_WB_EMPTY_Msk          0x1UL
/* CPUSS.RAM0_PWR_MACRO_CTL */
#define CPUSS_RAM0_PWR_MACRO_CTL_PWR_MODE_Pos   0UL
#define CPUSS_RAM0_PWR_MACRO_CTL_PWR_MODE_Msk   0x3UL
#define CPUSS_RAM0_PWR_MACRO_CTL_VECTKEYSTAT_Pos 16UL
#define CPUSS_RAM0_PWR_MACRO_CTL_VECTKEYSTAT_Msk 0xFFFF0000UL
/* CPUSS.RAM1_CTL0 */
#define CPUSS_RAM1_CTL0_SLOW_WS_Pos             0UL
#define CPUSS_RAM1_CTL0_SLOW_WS_Msk             0x3UL
#define CPUSS_RAM1_CTL0_FAST_WS_Pos             8UL
#define CPUSS_RAM1_CTL0_FAST_WS_Msk             0x300UL
#define CPUSS_RAM1_CTL0_ECC_EN_Pos              16UL
#define CPUSS_RAM1_CTL0_ECC_EN_Msk              0x10000UL
#define CPUSS_RAM1_CTL0_ECC_AUTO_CORRECT_Pos    17UL
#define CPUSS_RAM1_CTL0_ECC_AUTO_CORRECT_Msk    0x20000UL
#define CPUSS_RAM1_CTL0_ECC_INJ_EN_Pos          18UL
#define CPUSS_RAM1_CTL0_ECC_INJ_EN_Msk          0x40000UL
/* CPUSS.RAM1_STATUS */
#define CPUSS_RAM1_STATUS_WB_EMPTY_Pos          0UL
#define CPUSS_RAM1_STATUS_WB_EMPTY_Msk          0x1UL
/* CPUSS.RAM1_PWR_CTL */
#define CPUSS_RAM1_PWR_CTL_PWR_MODE_Pos         0UL
#define CPUSS_RAM1_PWR_CTL_PWR_MODE_Msk         0x3UL
#define CPUSS_RAM1_PWR_CTL_VECTKEYSTAT_Pos      16UL
#define CPUSS_RAM1_PWR_CTL_VECTKEYSTAT_Msk      0xFFFF0000UL
/* CPUSS.RAM2_CTL0 */
#define CPUSS_RAM2_CTL0_SLOW_WS_Pos             0UL
#define CPUSS_RAM2_CTL0_SLOW_WS_Msk             0x3UL
#define CPUSS_RAM2_CTL0_FAST_WS_Pos             8UL
#define CPUSS_RAM2_CTL0_FAST_WS_Msk             0x300UL
#define CPUSS_RAM2_CTL0_ECC_EN_Pos              16UL
#define CPUSS_RAM2_CTL0_ECC_EN_Msk              0x10000UL
#define CPUSS_RAM2_CTL0_ECC_AUTO_CORRECT_Pos    17UL
#define CPUSS_RAM2_CTL0_ECC_AUTO_CORRECT_Msk    0x20000UL
#define CPUSS_RAM2_CTL0_ECC_INJ_EN_Pos          18UL
#define CPUSS_RAM2_CTL0_ECC_INJ_EN_Msk          0x40000UL
/* CPUSS.RAM2_STATUS */
#define CPUSS_RAM2_STATUS_WB_EMPTY_Pos          0UL
#define CPUSS_RAM2_STATUS_WB_EMPTY_Msk          0x1UL
/* CPUSS.RAM2_PWR_CTL */
#define CPUSS_RAM2_PWR_CTL_PWR_MODE_Pos         0UL
#define CPUSS_RAM2_PWR_CTL_PWR_MODE_Msk         0x3UL
#define CPUSS_RAM2_PWR_CTL_VECTKEYSTAT_Pos      16UL
#define CPUSS_RAM2_PWR_CTL_VECTKEYSTAT_Msk      0xFFFF0000UL
/* CPUSS.RAM_PWR_DELAY_CTL */
#define CPUSS_RAM_PWR_DELAY_CTL_UP_Pos          0UL
#define CPUSS_RAM_PWR_DELAY_CTL_UP_Msk          0x3FFUL
/* CPUSS.ROM_CTL */
#define CPUSS_ROM_CTL_SLOW_WS_Pos               0UL
#define CPUSS_ROM_CTL_SLOW_WS_Msk               0x3UL
#define CPUSS_ROM_CTL_FAST_WS_Pos               8UL
#define CPUSS_ROM_CTL_FAST_WS_Msk               0x300UL
/* CPUSS.ECC_CTL */
#define CPUSS_ECC_CTL_WORD_ADDR_Pos             0UL
#define CPUSS_ECC_CTL_WORD_ADDR_Msk             0x1FFFFFFUL
#define CPUSS_ECC_CTL_PARITY_Pos                25UL
#define CPUSS_ECC_CTL_PARITY_Msk                0xFE000000UL
/* CPUSS.PRODUCT_ID */
#define CPUSS_PRODUCT_ID_FAMILY_ID_Pos          0UL
#define CPUSS_PRODUCT_ID_FAMILY_ID_Msk          0xFFFUL
#define CPUSS_PRODUCT_ID_MAJOR_REV_Pos          16UL
#define CPUSS_PRODUCT_ID_MAJOR_REV_Msk          0xF0000UL
#define CPUSS_PRODUCT_ID_MINOR_REV_Pos          20UL
#define CPUSS_PRODUCT_ID_MINOR_REV_Msk          0xF00000UL
/* CPUSS.DP_STATUS */
#define CPUSS_DP_STATUS_SWJ_CONNECTED_Pos       0UL
#define CPUSS_DP_STATUS_SWJ_CONNECTED_Msk       0x1UL
#define CPUSS_DP_STATUS_SWJ_DEBUG_EN_Pos        1UL
#define CPUSS_DP_STATUS_SWJ_DEBUG_EN_Msk        0x2UL
#define CPUSS_DP_STATUS_SWJ_JTAG_SEL_Pos        2UL
#define CPUSS_DP_STATUS_SWJ_JTAG_SEL_Msk        0x4UL
/* CPUSS.AP_CTL */
#define CPUSS_AP_CTL_CM0_ENABLE_Pos             0UL
#define CPUSS_AP_CTL_CM0_ENABLE_Msk             0x1UL
#define CPUSS_AP_CTL_CM4_ENABLE_Pos             1UL
#define CPUSS_AP_CTL_CM4_ENABLE_Msk             0x2UL
#define CPUSS_AP_CTL_SYS_ENABLE_Pos             2UL
#define CPUSS_AP_CTL_SYS_ENABLE_Msk             0x4UL
#define CPUSS_AP_CTL_CM0_DISABLE_Pos            16UL
#define CPUSS_AP_CTL_CM0_DISABLE_Msk            0x10000UL
#define CPUSS_AP_CTL_CM4_DISABLE_Pos            17UL
#define CPUSS_AP_CTL_CM4_DISABLE_Msk            0x20000UL
#define CPUSS_AP_CTL_SYS_DISABLE_Pos            18UL
#define CPUSS_AP_CTL_SYS_DISABLE_Msk            0x40000UL
/* CPUSS.BUFF_CTL */
#define CPUSS_BUFF_CTL_WRITE_BUFF_Pos           0UL
#define CPUSS_BUFF_CTL_WRITE_BUFF_Msk           0x1UL
/* CPUSS.SYSTICK_CTL */
#define CPUSS_SYSTICK_CTL_TENMS_Pos             0UL
#define CPUSS_SYSTICK_CTL_TENMS_Msk             0xFFFFFFUL
#define CPUSS_SYSTICK_CTL_CLOCK_SOURCE_Pos      24UL
#define CPUSS_SYSTICK_CTL_CLOCK_SOURCE_Msk      0x3000000UL
#define CPUSS_SYSTICK_CTL_SKEW_Pos              30UL
#define CPUSS_SYSTICK_CTL_SKEW_Msk              0x40000000UL
#define CPUSS_SYSTICK_CTL_NOREF_Pos             31UL
#define CPUSS_SYSTICK_CTL_NOREF_Msk             0x80000000UL
/* CPUSS.MBIST_STAT */
#define CPUSS_MBIST_STAT_SFP_READY_Pos          0UL
#define CPUSS_MBIST_STAT_SFP_READY_Msk          0x1UL
#define CPUSS_MBIST_STAT_SFP_FAIL_Pos           1UL
#define CPUSS_MBIST_STAT_SFP_FAIL_Msk           0x2UL
/* CPUSS.CAL_SUP_SET */
#define CPUSS_CAL_SUP_SET_DATA_Pos              0UL
#define CPUSS_CAL_SUP_SET_DATA_Msk              0xFFFFFFFFUL
/* CPUSS.CAL_SUP_CLR */
#define CPUSS_CAL_SUP_CLR_DATA_Pos              0UL
#define CPUSS_CAL_SUP_CLR_DATA_Msk              0xFFFFFFFFUL
/* CPUSS.CM0_PC_CTL */
#define CPUSS_CM0_PC_CTL_VALID_Pos              0UL
#define CPUSS_CM0_PC_CTL_VALID_Msk              0xFUL
/* CPUSS.CM0_PC0_HANDLER */
#define CPUSS_CM0_PC0_HANDLER_ADDR_Pos          0UL
#define CPUSS_CM0_PC0_HANDLER_ADDR_Msk          0xFFFFFFFFUL
/* CPUSS.CM0_PC1_HANDLER */
#define CPUSS_CM0_PC1_HANDLER_ADDR_Pos          0UL
#define CPUSS_CM0_PC1_HANDLER_ADDR_Msk          0xFFFFFFFFUL
/* CPUSS.CM0_PC2_HANDLER */
#define CPUSS_CM0_PC2_HANDLER_ADDR_Pos          0UL
#define CPUSS_CM0_PC2_HANDLER_ADDR_Msk          0xFFFFFFFFUL
/* CPUSS.CM0_PC3_HANDLER */
#define CPUSS_CM0_PC3_HANDLER_ADDR_Pos          0UL
#define CPUSS_CM0_PC3_HANDLER_ADDR_Msk          0xFFFFFFFFUL
/* CPUSS.PROTECTION */
#define CPUSS_PROTECTION_STATE_Pos              0UL
#define CPUSS_PROTECTION_STATE_Msk              0x7UL
/* CPUSS.TRIM_ROM_CTL */
#define CPUSS_TRIM_ROM_CTL_TRIM_Pos             0UL
#define CPUSS_TRIM_ROM_CTL_TRIM_Msk             0xFFFFFFFFUL
/* CPUSS.TRIM_RAM_CTL */
#define CPUSS_TRIM_RAM_CTL_TRIM_Pos             0UL
#define CPUSS_TRIM_RAM_CTL_TRIM_Msk             0xFFFFFFFFUL
/* CPUSS.CM0_SYSTEM_INT_CTL */
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_IDX_Pos 0UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_IDX_Msk 0x7UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_VALID_Pos 31UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM4_SYSTEM_INT_CTL */
#define CPUSS_CM4_SYSTEM_INT_CTL_CPU_INT_IDX_Pos 0UL
#define CPUSS_CM4_SYSTEM_INT_CTL_CPU_INT_IDX_Msk 0x7UL
#define CPUSS_CM4_SYSTEM_INT_CTL_CPU_INT_VALID_Pos 31UL
#define CPUSS_CM4_SYSTEM_INT_CTL_CPU_INT_VALID_Msk 0x80000000UL


#endif /* _CYIP_CPUSS_V2_H_ */


/* [] END OF FILE */
