/***************************************************************************//**
* \file cyip_cpuss.h
*
* \brief
* CPUSS IP definitions
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

#ifndef _CYIP_CPUSS_H_
#define _CYIP_CPUSS_H_

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
  * \brief CM7 0 status (CPUSS_CM7_0_STATUS)
  */
typedef struct stc_CPUSS_CM7_0_STATUS_field {
  uint32_t                         u1SLEEPING:1;
  uint32_t                         u1SLEEPDEEP:1;
  uint32_t                         :2;
  uint32_t                         u1PWR_DONE:1;
  uint32_t                         :4;
  uint32_t                         u1TCMC_CM7_1_MS:1;
  uint32_t                         u1TCMC_EXT_MS_2_TO_0:1;
  uint32_t                         u1TCMC_EXT_MS_3:1;
  uint32_t                         u1TCMC_AHB_MS:1;
  uint32_t                         :19;
} stc_CPUSS_CM7_0_STATUS_field_t;

typedef union un_CPUSS_CM7_0_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_STATUS_t;

/**
  * \brief Fast 0 clock control (CPUSS_FAST_0_CLOCK_CTL)
  */
typedef struct stc_CPUSS_FAST_0_CLOCK_CTL_field {
  uint32_t                         :3;
  uint32_t                         u5FRAC_DIV:5;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_FAST_0_CLOCK_CTL_field_t;

typedef union un_CPUSS_FAST_0_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_FAST_0_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_FAST_0_CLOCK_CTL_t;

/**
  * \brief CM7 0 control (CPUSS_CM7_0_CTL)
  */
typedef struct stc_CPUSS_CM7_0_CTL_field {
  uint32_t                         u4PPB_LOCK:4;
  uint32_t                         u1CPU_WAIT:1;
  uint32_t                         :3;
  uint32_t                         u2INIT_TCM_EN:2;
  uint32_t                         u2INIT_RMW_EN:2;
  uint32_t                         :4;
  uint32_t                         u1ITCM_ECC_EN:1;
  uint32_t                         u1ITCM_ECC_INJ_EN:1;
  uint32_t                         u1ITCM_READ_WS:1;
  uint32_t                         u1ITCM_ECC_CHECK_DIS:1;
  uint32_t                         u1DTCM_ECC_EN:1;
  uint32_t                         u1DTCM_ECC_INJ_EN:1;
  uint32_t                         u1DTCM_READ_WS:1;
  uint32_t                         u1TCMC_EN:1;
  uint32_t                         u1IOC_MASK:1;
  uint32_t                         u1DZC_MASK:1;
  uint32_t                         u1OFC_MASK:1;
  uint32_t                         u1UFC_MASK:1;
  uint32_t                         u1IXC_MASK:1;
  uint32_t                         :2;
  uint32_t                         u1IDC_MASK:1;
} stc_CPUSS_CM7_0_CTL_field_t;

typedef union un_CPUSS_CM7_0_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_CTL_t;

/**
  * \brief CM7 0 interrupt status (CPUSS_CM7_0_INT_STATUS)
  */
typedef struct stc_CPUSS_CM7_0_INT_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM7_0_INT_STATUS_field_t;

typedef union un_CPUSS_CM7_0_INT_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_INT_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_INT_STATUS_t;

/**
  * \brief CM7 0 vector table base (CPUSS_CM7_0_VECTOR_TABLE_BASE)
  */
typedef struct stc_CPUSS_CM7_0_VECTOR_TABLE_BASE_field {
  uint32_t                         :7;
  uint32_t                         u25ADDR25:25;
} stc_CPUSS_CM7_0_VECTOR_TABLE_BASE_field_t;

typedef union un_CPUSS_CM7_0_VECTOR_TABLE_BASE {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_VECTOR_TABLE_BASE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_VECTOR_TABLE_BASE_t;

/**
  * \brief CM7 0 NMI control (CPUSS_CM7_0_NMI_CTL)
  */
typedef struct stc_CPUSS_CM7_0_NMI_CTL_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :22;
} stc_CPUSS_CM7_0_NMI_CTL_field_t;

typedef union un_CPUSS_CM7_0_NMI_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_NMI_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_NMI_CTL_t;

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
  * \brief Trace and debug clock control (CPUSS_TRC_DBG_CLOCK_CTL)
  */
typedef struct stc_CPUSS_TRC_DBG_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_TRC_DBG_CLOCK_CTL_field_t;

typedef union un_CPUSS_TRC_DBG_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_TRC_DBG_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_TRC_DBG_CLOCK_CTL_t;

/**
  * \brief CM7 1status (CPUSS_CM7_1_STATUS)
  */
typedef struct stc_CPUSS_CM7_1_STATUS_field {
  uint32_t                         u1SLEEPING:1;
  uint32_t                         u1SLEEPDEEP:1;
  uint32_t                         :2;
  uint32_t                         u1PWR_DONE:1;
  uint32_t                         :3;
  uint32_t                         u1TCMC_CM7_0_MS:1;
  uint32_t                         :1;
  uint32_t                         u1TCMC_EXT_MS_2_TO_0:1;
  uint32_t                         u1TCMC_EXT_MS_3:1;
  uint32_t                         u1TCMC_AHB_MS:1;
  uint32_t                         :19;
} stc_CPUSS_CM7_1_STATUS_field_t;

typedef union un_CPUSS_CM7_1_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_STATUS_t;

/**
  * \brief Fast 1 clock control (CPUSS_FAST_1_CLOCK_CTL)
  */
typedef struct stc_CPUSS_FAST_1_CLOCK_CTL_field {
  uint32_t                         :3;
  uint32_t                         u5FRAC_DIV:5;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_FAST_1_CLOCK_CTL_field_t;

typedef union un_CPUSS_FAST_1_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_FAST_1_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_FAST_1_CLOCK_CTL_t;

/**
  * \brief CM7 1 control (CPUSS_CM7_1_CTL)
  */
typedef struct stc_CPUSS_CM7_1_CTL_field {
  uint32_t                         u4PPB_LOCK:4;
  uint32_t                         u1CPU_WAIT:1;
  uint32_t                         :3;
  uint32_t                         u2INIT_TCM_EN:2;
  uint32_t                         u2INIT_RMW_EN:2;
  uint32_t                         :4;
  uint32_t                         u1ITCM_ECC_EN:1;
  uint32_t                         u1ITCM_ECC_INJ_EN:1;
  uint32_t                         u1ITCM_READ_WS:1;
  uint32_t                         u1ITCM_ECC_CHECK_DIS:1;
  uint32_t                         u1DTCM_ECC_EN:1;
  uint32_t                         u1DTCM_ECC_INJ_EN:1;
  uint32_t                         u1DTCM_READ_WS:1;
  uint32_t                         u1TCMC_EN:1;
  uint32_t                         u1IOC_MASK:1;
  uint32_t                         u1DZC_MASK:1;
  uint32_t                         u1OFC_MASK:1;
  uint32_t                         u1UFC_MASK:1;
  uint32_t                         u1IXC_MASK:1;
  uint32_t                         :2;
  uint32_t                         u1IDC_MASK:1;
} stc_CPUSS_CM7_1_CTL_field_t;

typedef union un_CPUSS_CM7_1_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_CTL_t;

/**
  * \brief CM7 1 interrupt status (CPUSS_CM7_1_INT_STATUS)
  */
typedef struct stc_CPUSS_CM7_1_INT_STATUS_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :21;
  uint32_t                         u1SYSTEM_INT_VALID:1;
} stc_CPUSS_CM7_1_INT_STATUS_field_t;

typedef union un_CPUSS_CM7_1_INT_STATUS {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_INT_STATUS_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_INT_STATUS_t;

/**
  * \brief CM7 1 vector table base (CPUSS_CM7_1_VECTOR_TABLE_BASE)
  */
typedef struct stc_CPUSS_CM7_1_VECTOR_TABLE_BASE_field {
  uint32_t                         :7;
  uint32_t                         u25ADDR25:25;
} stc_CPUSS_CM7_1_VECTOR_TABLE_BASE_field_t;

typedef union un_CPUSS_CM7_1_VECTOR_TABLE_BASE {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_VECTOR_TABLE_BASE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_VECTOR_TABLE_BASE_t;

/**
  * \brief CM7 1 NMI control (CPUSS_CM7_1_NMI_CTL)
  */
typedef struct stc_CPUSS_CM7_1_NMI_CTL_field {
  uint32_t                         u10SYSTEM_INT_IDX:10;
  uint32_t                         :22;
} stc_CPUSS_CM7_1_NMI_CTL_field_t;

typedef union un_CPUSS_CM7_1_NMI_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_NMI_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_NMI_CTL_t;

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
  * \brief Slow clock control (CPUSS_SLOW_CLOCK_CTL)
  */
typedef struct stc_CPUSS_SLOW_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_SLOW_CLOCK_CTL_field_t;

typedef union un_CPUSS_SLOW_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_SLOW_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_SLOW_CLOCK_CTL_t;

/**
  * \brief Peripheral interconnect clock control (CPUSS_PERI_CLOCK_CTL)
  */
typedef struct stc_CPUSS_PERI_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_PERI_CLOCK_CTL_field_t;

typedef union un_CPUSS_PERI_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_PERI_CLOCK_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_PERI_CLOCK_CTL_t;

/**
  * \brief Memory clock control (CPUSS_MEM_CLOCK_CTL)
  */
typedef struct stc_CPUSS_MEM_CLOCK_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8INT_DIV:8;
  uint32_t                         :16;
} stc_CPUSS_MEM_CLOCK_CTL_field_t;

typedef union un_CPUSS_MEM_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_MEM_CLOCK_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_MEM_CLOCK_CTL_t;

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
  * \brief CM7 0 power control (CPUSS_CM7_0_PWR_CTL)
  */
typedef struct stc_CPUSS_CM7_0_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_CM7_0_PWR_CTL_field_t;

typedef union un_CPUSS_CM7_0_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_PWR_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_PWR_CTL_t;

/**
  * \brief CM7 0 power delay control (CPUSS_CM7_0_PWR_DELAY_CTL)
  */
typedef struct stc_CPUSS_CM7_0_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CPUSS_CM7_0_PWR_DELAY_CTL_field_t;

typedef union un_CPUSS_CM7_0_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_PWR_DELAY_CTL_t;

/**
  * \brief CM7 1 power control (CPUSS_CM7_1_PWR_CTL)
  */
typedef struct stc_CPUSS_CM7_1_PWR_CTL_field {
  uint32_t                         u2PWR_MODE:2;
  uint32_t                         :14;
  uint32_t                         u16VECTKEYSTAT:16;
} stc_CPUSS_CM7_1_PWR_CTL_field_t;

typedef union un_CPUSS_CM7_1_PWR_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_PWR_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_PWR_CTL_t;

/**
  * \brief CM7 1 power delay control (CPUSS_CM7_1_PWR_DELAY_CTL)
  */
typedef struct stc_CPUSS_CM7_1_PWR_DELAY_CTL_field {
  uint32_t                         u10UP:10;
  uint32_t                         :22;
} stc_CPUSS_CM7_1_PWR_DELAY_CTL_field_t;

typedef union un_CPUSS_CM7_1_PWR_DELAY_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_PWR_DELAY_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_PWR_DELAY_CTL_t;

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
  uint32_t                         u1ECC_CHECK_DIS:1;
  uint32_t                         :12;
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
  uint32_t                         u1ECC_CHECK_DIS:1;
  uint32_t                         :12;
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
  uint32_t                         u1ECC_CHECK_DIS:1;
  uint32_t                         :12;
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
  uint32_t                         u24WORD_ADDR:24;
  uint32_t                         u8PARITY:8;
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
  uint32_t                         u1CM7_ENABLE:1;
  uint32_t                         u1SYS_ENABLE:1;
  uint32_t                         :13;
  uint32_t                         u1CM0_DISABLE:1;
  uint32_t                         u1CM7_DISABLE:1;
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
  * \brief RAM trim control for less than 100MHz SRAMs (CPUSS_TRIM_RAM_CTL)
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
  * \brief RAM trim control for 100MHz - 200MHz SRAMs (CPUSS_TRIM_RAM200_CTL)
  */
typedef struct stc_CPUSS_TRIM_RAM200_CTL_field {
  uint32_t                         u32TRIM:32;
} stc_CPUSS_TRIM_RAM200_CTL_field_t;

typedef union un_CPUSS_TRIM_RAM200_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_TRIM_RAM200_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_TRIM_RAM200_CTL_t;

/**
  * \brief RAM trim control for more than 200MHz SRAMs (CPUSS_TRIM_RAM350_CTL)
  */
typedef struct stc_CPUSS_TRIM_RAM350_CTL_field {
  uint32_t                         u32TRIM:32;
} stc_CPUSS_TRIM_RAM350_CTL_field_t;

typedef union un_CPUSS_TRIM_RAM350_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_TRIM_RAM350_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_TRIM_RAM350_CTL_t;

/**
  * \brief CM0+ system interrupt control (CPUSS_CM0_SYSTEM_INT_CTL)
  */
typedef struct stc_CPUSS_CM0_SYSTEM_INT_CTL_field {
  uint32_t                         u3CM0_CPU_INT_IDX:3;
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
  * \brief CM7 0 system interrupt control (CPUSS_CM7_0_SYSTEM_INT_CTL)
  */
typedef struct stc_CPUSS_CM7_0_SYSTEM_INT_CTL_field {
  uint32_t                         u4CPU_INT_IDX:4;
  uint32_t                         :27;
  uint32_t                         u1CPU_INT_VALID:1;
} stc_CPUSS_CM7_0_SYSTEM_INT_CTL_field_t;

typedef union un_CPUSS_CM7_0_SYSTEM_INT_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_0_SYSTEM_INT_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_0_SYSTEM_INT_CTL_t;

/**
  * \brief CM7 1 system interrupt control (CPUSS_CM7_1_SYSTEM_INT_CTL)
  */
typedef struct stc_CPUSS_CM7_1_SYSTEM_INT_CTL_field {
  uint32_t                         u4CPU_INT_IDX:4;
  uint32_t                         :27;
  uint32_t                         u1CPU_INT_VALID:1;
} stc_CPUSS_CM7_1_SYSTEM_INT_CTL_field_t;

typedef union un_CPUSS_CM7_1_SYSTEM_INT_CTL {
  uint32_t                         u32Register;
  stc_CPUSS_CM7_1_SYSTEM_INT_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CPUSS_CM7_1_SYSTEM_INT_CTL_t;



/**
  * \brief CPU subsystem (CPUSS) (CPUSS)
  */
typedef struct stc_CPUSS {
  un_CPUSS_IDENTITY_t              unIDENTITY;         /*!< 0x00000000 Identity */
  un_CPUSS_CM7_0_STATUS_t          unCM7_0_STATUS;     /*!< 0x00000004 CM7 0 status */
  un_CPUSS_FAST_0_CLOCK_CTL_t      unFAST_0_CLOCK_CTL; /*!< 0x00000008 Fast 0 clock control */
  un_CPUSS_CM7_0_CTL_t             unCM7_0_CTL;        /*!< 0x0000000C CM7 0 control */
  uint32_t                         au32Reserved[60];
  un_CPUSS_CM7_0_INT_STATUS_t      unCM7_0_INT_STATUS[16]; /*!< 0x00000100 CM7 0 interrupt status */
  uint32_t                         au32Reserved1[48];
  un_CPUSS_CM7_0_VECTOR_TABLE_BASE_t unCM7_0_VECTOR_TABLE_BASE; /*!< 0x00000200 CM7 0 vector table base */
  uint32_t                         au32Reserved2[15];
  un_CPUSS_CM7_0_NMI_CTL_t         unCM7_0_NMI_CTL[4]; /*!< 0x00000240 CM7 0 NMI control */
  uint32_t                         au32Reserved3[44];
  un_CPUSS_UDB_PWR_CTL_t           unUDB_PWR_CTL;      /*!< 0x00000300 UDB power control */
  un_CPUSS_UDB_PWR_DELAY_CTL_t     unUDB_PWR_DELAY_CTL; /*!< 0x00000304 UDB power control */
  uint32_t                         au32Reserved4[6];
  un_CPUSS_TRC_DBG_CLOCK_CTL_t     unTRC_DBG_CLOCK_CTL; /*!< 0x00000320 Trace and debug clock control */
  uint32_t                         au32Reserved5[56];
  un_CPUSS_CM7_1_STATUS_t          unCM7_1_STATUS;     /*!< 0x00000404 CM7 1status */
  un_CPUSS_FAST_1_CLOCK_CTL_t      unFAST_1_CLOCK_CTL; /*!< 0x00000408 Fast 1 clock control */
  un_CPUSS_CM7_1_CTL_t             unCM7_1_CTL;        /*!< 0x0000040C CM7 1 control */
  uint32_t                         au32Reserved6[60];
  un_CPUSS_CM7_1_INT_STATUS_t      unCM7_1_INT_STATUS[16]; /*!< 0x00000500 CM7 1 interrupt status */
  uint32_t                         au32Reserved7[48];
  un_CPUSS_CM7_1_VECTOR_TABLE_BASE_t unCM7_1_VECTOR_TABLE_BASE; /*!< 0x00000600 CM7 1 vector table base */
  uint32_t                         au32Reserved8[15];
  un_CPUSS_CM7_1_NMI_CTL_t         unCM7_1_NMI_CTL[4]; /*!< 0x00000640 CM7 1 NMI control */
  uint32_t                         au32Reserved9[620];
  un_CPUSS_CM0_CTL_t               unCM0_CTL;          /*!< 0x00001000 CM0+ control */
  un_CPUSS_CM0_STATUS_t            unCM0_STATUS;       /*!< 0x00001004 CM0+ status */
  un_CPUSS_SLOW_CLOCK_CTL_t        unSLOW_CLOCK_CTL;   /*!< 0x00001008 Slow clock control */
  un_CPUSS_PERI_CLOCK_CTL_t        unPERI_CLOCK_CTL;   /*!< 0x0000100C Peripheral interconnect clock control */
  un_CPUSS_MEM_CLOCK_CTL_t         unMEM_CLOCK_CTL;    /*!< 0x00001010 Memory clock control */
  uint32_t                         au32Reserved10[59];
  un_CPUSS_CM0_INT0_STATUS_t       unCM0_INT0_STATUS;  /*!< 0x00001100 CM0+ interrupt 0 status */
  un_CPUSS_CM0_INT1_STATUS_t       unCM0_INT1_STATUS;  /*!< 0x00001104 CM0+ interrupt 1 status */
  un_CPUSS_CM0_INT2_STATUS_t       unCM0_INT2_STATUS;  /*!< 0x00001108 CM0+ interrupt 2 status */
  un_CPUSS_CM0_INT3_STATUS_t       unCM0_INT3_STATUS;  /*!< 0x0000110C CM0+ interrupt 3 status */
  un_CPUSS_CM0_INT4_STATUS_t       unCM0_INT4_STATUS;  /*!< 0x00001110 CM0+ interrupt 4 status */
  un_CPUSS_CM0_INT5_STATUS_t       unCM0_INT5_STATUS;  /*!< 0x00001114 CM0+ interrupt 5 status */
  un_CPUSS_CM0_INT6_STATUS_t       unCM0_INT6_STATUS;  /*!< 0x00001118 CM0+ interrupt 6 status */
  un_CPUSS_CM0_INT7_STATUS_t       unCM0_INT7_STATUS;  /*!< 0x0000111C CM0+ interrupt 7 status */
  un_CPUSS_CM0_VECTOR_TABLE_BASE_t unCM0_VECTOR_TABLE_BASE; /*!< 0x00001120 CM0+ vector table base */
  uint32_t                         au32Reserved11[7];
  un_CPUSS_CM0_NMI_CTL_t           unCM0_NMI_CTL[4];   /*!< 0x00001140 CM0+ NMI control */
  uint32_t                         au32Reserved12[44];
  un_CPUSS_CM7_0_PWR_CTL_t         unCM7_0_PWR_CTL;    /*!< 0x00001200 CM7 0 power control */
  un_CPUSS_CM7_0_PWR_DELAY_CTL_t   unCM7_0_PWR_DELAY_CTL; /*!< 0x00001204 CM7 0 power delay control */
  uint32_t                         au32Reserved13[2];
  un_CPUSS_CM7_1_PWR_CTL_t         unCM7_1_PWR_CTL;    /*!< 0x00001210 CM7 1 power control */
  un_CPUSS_CM7_1_PWR_DELAY_CTL_t   unCM7_1_PWR_DELAY_CTL; /*!< 0x00001214 CM7 1 power delay control */
  uint32_t                         au32Reserved14[58];
  un_CPUSS_RAM0_CTL0_t             unRAM0_CTL0;        /*!< 0x00001300 RAM 0 control */
  un_CPUSS_RAM0_STATUS_t           unRAM0_STATUS;      /*!< 0x00001304 RAM 0 status */
  uint32_t                         au32Reserved15[14];
  un_CPUSS_RAM0_PWR_MACRO_CTL_t    unRAM0_PWR_MACRO_CTL[16]; /*!< 0x00001340 RAM 0 power control */
  un_CPUSS_RAM1_CTL0_t             unRAM1_CTL0;        /*!< 0x00001380 RAM 1 control */
  un_CPUSS_RAM1_STATUS_t           unRAM1_STATUS;      /*!< 0x00001384 RAM 1 status */
  un_CPUSS_RAM1_PWR_CTL_t          unRAM1_PWR_CTL;     /*!< 0x00001388 RAM 1 power control */
  uint32_t                         au32Reserved16[5];
  un_CPUSS_RAM2_CTL0_t             unRAM2_CTL0;        /*!< 0x000013A0 RAM 2 control */
  un_CPUSS_RAM2_STATUS_t           unRAM2_STATUS;      /*!< 0x000013A4 RAM 2 status */
  un_CPUSS_RAM2_PWR_CTL_t          unRAM2_PWR_CTL;     /*!< 0x000013A8 RAM 2 power control */
  uint32_t                         au32Reserved17[5];
  un_CPUSS_RAM_PWR_DELAY_CTL_t     unRAM_PWR_DELAY_CTL; /*!< 0x000013C0 Power up delay used for all SRAM power domains */
  un_CPUSS_ROM_CTL_t               unROM_CTL;          /*!< 0x000013C4 ROM control */
  un_CPUSS_ECC_CTL_t               unECC_CTL;          /*!< 0x000013C8 ECC control */
  uint32_t                         au32Reserved18[13];
  un_CPUSS_PRODUCT_ID_t            unPRODUCT_ID;       /*!< 0x00001400 Product identifier and version (same as CoreSight
                                                                                RomTables) */
  uint32_t                         au32Reserved19[3];
  un_CPUSS_DP_STATUS_t             unDP_STATUS;        /*!< 0x00001410 Debug port status */
  un_CPUSS_AP_CTL_t                unAP_CTL;           /*!< 0x00001414 Access port control */
  uint32_t                         au32Reserved20[58];
  un_CPUSS_BUFF_CTL_t              unBUFF_CTL;         /*!< 0x00001500 Buffer control */
  uint32_t                         au32Reserved21[63];
  un_CPUSS_SYSTICK_CTL_t           unSYSTICK_CTL;      /*!< 0x00001600 SysTick timer control */
  uint32_t                         au32Reserved22[64];
  un_CPUSS_MBIST_STAT_t            unMBIST_STAT;       /*!< 0x00001704 Memory BIST status */
  uint32_t                         au32Reserved23[62];
  un_CPUSS_CAL_SUP_SET_t           unCAL_SUP_SET;      /*!< 0x00001800 Calibration support set and read */
  un_CPUSS_CAL_SUP_CLR_t           unCAL_SUP_CLR;      /*!< 0x00001804 Calibration support clear and reset */
  uint32_t                         au32Reserved24[510];
  un_CPUSS_CM0_PC_CTL_t            unCM0_PC_CTL;       /*!< 0x00002000 CM0+ protection context control */
  uint32_t                         au32Reserved25[15];
  un_CPUSS_CM0_PC0_HANDLER_t       unCM0_PC0_HANDLER;  /*!< 0x00002040 CM0+ protection context 0 handler */
  un_CPUSS_CM0_PC1_HANDLER_t       unCM0_PC1_HANDLER;  /*!< 0x00002044 CM0+ protection context 1 handler */
  un_CPUSS_CM0_PC2_HANDLER_t       unCM0_PC2_HANDLER;  /*!< 0x00002048 CM0+ protection context 2 handler */
  un_CPUSS_CM0_PC3_HANDLER_t       unCM0_PC3_HANDLER;  /*!< 0x0000204C CM0+ protection context 3 handler */
  uint32_t                         au32Reserved26[29];
  un_CPUSS_PROTECTION_t            unPROTECTION;       /*!< 0x000020C4 Protection status */
  uint32_t                         au32Reserved27[14];
  un_CPUSS_TRIM_ROM_CTL_t          unTRIM_ROM_CTL;     /*!< 0x00002100 ROM trim control */
  un_CPUSS_TRIM_RAM_CTL_t          unTRIM_RAM_CTL;     /*!< 0x00002104 RAM trim control for less than 100MHz SRAMs */
  un_CPUSS_TRIM_RAM200_CTL_t       unTRIM_RAM200_CTL;  /*!< 0x00002108 RAM trim control for 100MHz - 200MHz SRAMs */
  un_CPUSS_TRIM_RAM350_CTL_t       unTRIM_RAM350_CTL;  /*!< 0x0000210C RAM trim control for more than 200MHz SRAMs */
  uint32_t                         au32Reserved28[6076];
  un_CPUSS_CM0_SYSTEM_INT_CTL_t    unCM0_SYSTEM_INT_CTL[1023]; /*!< 0x00008000 CM0+ system interrupt control */
  uint32_t                         au32Reserved29[1025];
  un_CPUSS_CM7_0_SYSTEM_INT_CTL_t  unCM7_0_SYSTEM_INT_CTL[1023]; /*!< 0x0000A000 CM7 0 system interrupt control */
  uint32_t                         au32Reserved30[1025];
  un_CPUSS_CM7_1_SYSTEM_INT_CTL_t  unCM7_1_SYSTEM_INT_CTL[1023]; /*!< 0x0000C000 CM7 1 system interrupt control */
} stc_CPUSS_t;                                         /*!< Size = 53244 (0xCFFC) */


/* CPUSS.IDENTITY */
#define CPUSS_IDENTITY_P_Pos                    0UL
#define CPUSS_IDENTITY_P_Msk                    0x1UL
#define CPUSS_IDENTITY_NS_Pos                   1UL
#define CPUSS_IDENTITY_NS_Msk                   0x2UL
#define CPUSS_IDENTITY_PC_Pos                   4UL
#define CPUSS_IDENTITY_PC_Msk                   0xF0UL
#define CPUSS_IDENTITY_MS_Pos                   8UL
#define CPUSS_IDENTITY_MS_Msk                   0xF00UL
/* CPUSS.CM7_0_STATUS */
#define CPUSS_CM7_0_STATUS_SLEEPING_Pos         0UL
#define CPUSS_CM7_0_STATUS_SLEEPING_Msk         0x1UL
#define CPUSS_CM7_0_STATUS_SLEEPDEEP_Pos        1UL
#define CPUSS_CM7_0_STATUS_SLEEPDEEP_Msk        0x2UL
#define CPUSS_CM7_0_STATUS_PWR_DONE_Pos         4UL
#define CPUSS_CM7_0_STATUS_PWR_DONE_Msk         0x10UL
#define CPUSS_CM7_0_STATUS_TCMC_CM7_1_MS_Pos    9UL
#define CPUSS_CM7_0_STATUS_TCMC_CM7_1_MS_Msk    0x200UL
#define CPUSS_CM7_0_STATUS_TCMC_EXT_MS_2_TO_0_Pos 10UL
#define CPUSS_CM7_0_STATUS_TCMC_EXT_MS_2_TO_0_Msk 0x400UL
#define CPUSS_CM7_0_STATUS_TCMC_EXT_MS_3_Pos    11UL
#define CPUSS_CM7_0_STATUS_TCMC_EXT_MS_3_Msk    0x800UL
#define CPUSS_CM7_0_STATUS_TCMC_AHB_MS_Pos      12UL
#define CPUSS_CM7_0_STATUS_TCMC_AHB_MS_Msk      0x1000UL
/* CPUSS.FAST_0_CLOCK_CTL */
#define CPUSS_FAST_0_CLOCK_CTL_FRAC_DIV_Pos     3UL
#define CPUSS_FAST_0_CLOCK_CTL_FRAC_DIV_Msk     0xF8UL
#define CPUSS_FAST_0_CLOCK_CTL_INT_DIV_Pos      8UL
#define CPUSS_FAST_0_CLOCK_CTL_INT_DIV_Msk      0xFF00UL
/* CPUSS.CM7_0_CTL */
#define CPUSS_CM7_0_CTL_PPB_LOCK_Pos            0UL
#define CPUSS_CM7_0_CTL_PPB_LOCK_Msk            0xFUL
#define CPUSS_CM7_0_CTL_CPU_WAIT_Pos            4UL
#define CPUSS_CM7_0_CTL_CPU_WAIT_Msk            0x10UL
#define CPUSS_CM7_0_CTL_INIT_TCM_EN_Pos         8UL
#define CPUSS_CM7_0_CTL_INIT_TCM_EN_Msk         0x300UL
#define CPUSS_CM7_0_CTL_INIT_RMW_EN_Pos         10UL
#define CPUSS_CM7_0_CTL_INIT_RMW_EN_Msk         0xC00UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_EN_Pos         16UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_EN_Msk         0x10000UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_INJ_EN_Pos     17UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_INJ_EN_Msk     0x20000UL
#define CPUSS_CM7_0_CTL_ITCM_READ_WS_Pos        18UL
#define CPUSS_CM7_0_CTL_ITCM_READ_WS_Msk        0x40000UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_CHECK_DIS_Pos  19UL
#define CPUSS_CM7_0_CTL_ITCM_ECC_CHECK_DIS_Msk  0x80000UL
#define CPUSS_CM7_0_CTL_DTCM_ECC_EN_Pos         20UL
#define CPUSS_CM7_0_CTL_DTCM_ECC_EN_Msk         0x100000UL
#define CPUSS_CM7_0_CTL_DTCM_ECC_INJ_EN_Pos     21UL
#define CPUSS_CM7_0_CTL_DTCM_ECC_INJ_EN_Msk     0x200000UL
#define CPUSS_CM7_0_CTL_DTCM_READ_WS_Pos        22UL
#define CPUSS_CM7_0_CTL_DTCM_READ_WS_Msk        0x400000UL
#define CPUSS_CM7_0_CTL_TCMC_EN_Pos             23UL
#define CPUSS_CM7_0_CTL_TCMC_EN_Msk             0x800000UL
#define CPUSS_CM7_0_CTL_IOC_MASK_Pos            24UL
#define CPUSS_CM7_0_CTL_IOC_MASK_Msk            0x1000000UL
#define CPUSS_CM7_0_CTL_DZC_MASK_Pos            25UL
#define CPUSS_CM7_0_CTL_DZC_MASK_Msk            0x2000000UL
#define CPUSS_CM7_0_CTL_OFC_MASK_Pos            26UL
#define CPUSS_CM7_0_CTL_OFC_MASK_Msk            0x4000000UL
#define CPUSS_CM7_0_CTL_UFC_MASK_Pos            27UL
#define CPUSS_CM7_0_CTL_UFC_MASK_Msk            0x8000000UL
#define CPUSS_CM7_0_CTL_IXC_MASK_Pos            28UL
#define CPUSS_CM7_0_CTL_IXC_MASK_Msk            0x10000000UL
#define CPUSS_CM7_0_CTL_IDC_MASK_Pos            31UL
#define CPUSS_CM7_0_CTL_IDC_MASK_Msk            0x80000000UL
/* CPUSS.CM7_0_INT_STATUS */
#define CPUSS_CM7_0_INT_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM7_0_INT_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM7_0_INT_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM7_0_INT_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM7_0_VECTOR_TABLE_BASE */
#define CPUSS_CM7_0_VECTOR_TABLE_BASE_ADDR25_Pos 7UL
#define CPUSS_CM7_0_VECTOR_TABLE_BASE_ADDR25_Msk 0xFFFFFF80UL
/* CPUSS.CM7_0_NMI_CTL */
#define CPUSS_CM7_0_NMI_CTL_SYSTEM_INT_IDX_Pos  0UL
#define CPUSS_CM7_0_NMI_CTL_SYSTEM_INT_IDX_Msk  0x3FFUL
/* CPUSS.UDB_PWR_CTL */
#define CPUSS_UDB_PWR_CTL_PWR_MODE_Pos          0UL
#define CPUSS_UDB_PWR_CTL_PWR_MODE_Msk          0x3UL
#define CPUSS_UDB_PWR_CTL_VECTKEYSTAT_Pos       16UL
#define CPUSS_UDB_PWR_CTL_VECTKEYSTAT_Msk       0xFFFF0000UL
/* CPUSS.UDB_PWR_DELAY_CTL */
#define CPUSS_UDB_PWR_DELAY_CTL_UP_Pos          0UL
#define CPUSS_UDB_PWR_DELAY_CTL_UP_Msk          0x3FFUL
/* CPUSS.TRC_DBG_CLOCK_CTL */
#define CPUSS_TRC_DBG_CLOCK_CTL_INT_DIV_Pos     8UL
#define CPUSS_TRC_DBG_CLOCK_CTL_INT_DIV_Msk     0xFF00UL
/* CPUSS.CM7_1_STATUS */
#define CPUSS_CM7_1_STATUS_SLEEPING_Pos         0UL
#define CPUSS_CM7_1_STATUS_SLEEPING_Msk         0x1UL
#define CPUSS_CM7_1_STATUS_SLEEPDEEP_Pos        1UL
#define CPUSS_CM7_1_STATUS_SLEEPDEEP_Msk        0x2UL
#define CPUSS_CM7_1_STATUS_PWR_DONE_Pos         4UL
#define CPUSS_CM7_1_STATUS_PWR_DONE_Msk         0x10UL
#define CPUSS_CM7_1_STATUS_TCMC_CM7_0_MS_Pos    8UL
#define CPUSS_CM7_1_STATUS_TCMC_CM7_0_MS_Msk    0x100UL
#define CPUSS_CM7_1_STATUS_TCMC_EXT_MS_2_TO_0_Pos 10UL
#define CPUSS_CM7_1_STATUS_TCMC_EXT_MS_2_TO_0_Msk 0x400UL
#define CPUSS_CM7_1_STATUS_TCMC_EXT_MS_3_Pos    11UL
#define CPUSS_CM7_1_STATUS_TCMC_EXT_MS_3_Msk    0x800UL
#define CPUSS_CM7_1_STATUS_TCMC_AHB_MS_Pos      12UL
#define CPUSS_CM7_1_STATUS_TCMC_AHB_MS_Msk      0x1000UL
/* CPUSS.FAST_1_CLOCK_CTL */
#define CPUSS_FAST_1_CLOCK_CTL_FRAC_DIV_Pos     3UL
#define CPUSS_FAST_1_CLOCK_CTL_FRAC_DIV_Msk     0xF8UL
#define CPUSS_FAST_1_CLOCK_CTL_INT_DIV_Pos      8UL
#define CPUSS_FAST_1_CLOCK_CTL_INT_DIV_Msk      0xFF00UL
/* CPUSS.CM7_1_CTL */
#define CPUSS_CM7_1_CTL_PPB_LOCK_Pos            0UL
#define CPUSS_CM7_1_CTL_PPB_LOCK_Msk            0xFUL
#define CPUSS_CM7_1_CTL_CPU_WAIT_Pos            4UL
#define CPUSS_CM7_1_CTL_CPU_WAIT_Msk            0x10UL
#define CPUSS_CM7_1_CTL_INIT_TCM_EN_Pos         8UL
#define CPUSS_CM7_1_CTL_INIT_TCM_EN_Msk         0x300UL
#define CPUSS_CM7_1_CTL_INIT_RMW_EN_Pos         10UL
#define CPUSS_CM7_1_CTL_INIT_RMW_EN_Msk         0xC00UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_EN_Pos         16UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_EN_Msk         0x10000UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_INJ_EN_Pos     17UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_INJ_EN_Msk     0x20000UL
#define CPUSS_CM7_1_CTL_ITCM_READ_WS_Pos        18UL
#define CPUSS_CM7_1_CTL_ITCM_READ_WS_Msk        0x40000UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_CHECK_DIS_Pos  19UL
#define CPUSS_CM7_1_CTL_ITCM_ECC_CHECK_DIS_Msk  0x80000UL
#define CPUSS_CM7_1_CTL_DTCM_ECC_EN_Pos         20UL
#define CPUSS_CM7_1_CTL_DTCM_ECC_EN_Msk         0x100000UL
#define CPUSS_CM7_1_CTL_DTCM_ECC_INJ_EN_Pos     21UL
#define CPUSS_CM7_1_CTL_DTCM_ECC_INJ_EN_Msk     0x200000UL
#define CPUSS_CM7_1_CTL_DTCM_READ_WS_Pos        22UL
#define CPUSS_CM7_1_CTL_DTCM_READ_WS_Msk        0x400000UL
#define CPUSS_CM7_1_CTL_TCMC_EN_Pos             23UL
#define CPUSS_CM7_1_CTL_TCMC_EN_Msk             0x800000UL
#define CPUSS_CM7_1_CTL_IOC_MASK_Pos            24UL
#define CPUSS_CM7_1_CTL_IOC_MASK_Msk            0x1000000UL
#define CPUSS_CM7_1_CTL_DZC_MASK_Pos            25UL
#define CPUSS_CM7_1_CTL_DZC_MASK_Msk            0x2000000UL
#define CPUSS_CM7_1_CTL_OFC_MASK_Pos            26UL
#define CPUSS_CM7_1_CTL_OFC_MASK_Msk            0x4000000UL
#define CPUSS_CM7_1_CTL_UFC_MASK_Pos            27UL
#define CPUSS_CM7_1_CTL_UFC_MASK_Msk            0x8000000UL
#define CPUSS_CM7_1_CTL_IXC_MASK_Pos            28UL
#define CPUSS_CM7_1_CTL_IXC_MASK_Msk            0x10000000UL
#define CPUSS_CM7_1_CTL_IDC_MASK_Pos            31UL
#define CPUSS_CM7_1_CTL_IDC_MASK_Msk            0x80000000UL
/* CPUSS.CM7_1_INT_STATUS */
#define CPUSS_CM7_1_INT_STATUS_SYSTEM_INT_IDX_Pos 0UL
#define CPUSS_CM7_1_INT_STATUS_SYSTEM_INT_IDX_Msk 0x3FFUL
#define CPUSS_CM7_1_INT_STATUS_SYSTEM_INT_VALID_Pos 31UL
#define CPUSS_CM7_1_INT_STATUS_SYSTEM_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM7_1_VECTOR_TABLE_BASE */
#define CPUSS_CM7_1_VECTOR_TABLE_BASE_ADDR25_Pos 7UL
#define CPUSS_CM7_1_VECTOR_TABLE_BASE_ADDR25_Msk 0xFFFFFF80UL
/* CPUSS.CM7_1_NMI_CTL */
#define CPUSS_CM7_1_NMI_CTL_SYSTEM_INT_IDX_Pos  0UL
#define CPUSS_CM7_1_NMI_CTL_SYSTEM_INT_IDX_Msk  0x3FFUL
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
/* CPUSS.SLOW_CLOCK_CTL */
#define CPUSS_SLOW_CLOCK_CTL_INT_DIV_Pos        8UL
#define CPUSS_SLOW_CLOCK_CTL_INT_DIV_Msk        0xFF00UL
/* CPUSS.PERI_CLOCK_CTL */
#define CPUSS_PERI_CLOCK_CTL_INT_DIV_Pos        8UL
#define CPUSS_PERI_CLOCK_CTL_INT_DIV_Msk        0xFF00UL
/* CPUSS.MEM_CLOCK_CTL */
#define CPUSS_MEM_CLOCK_CTL_INT_DIV_Pos         8UL
#define CPUSS_MEM_CLOCK_CTL_INT_DIV_Msk         0xFF00UL
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
/* CPUSS.CM7_0_PWR_CTL */
#define CPUSS_CM7_0_PWR_CTL_PWR_MODE_Pos        0UL
#define CPUSS_CM7_0_PWR_CTL_PWR_MODE_Msk        0x3UL
#define CPUSS_CM7_0_PWR_CTL_VECTKEYSTAT_Pos     16UL
#define CPUSS_CM7_0_PWR_CTL_VECTKEYSTAT_Msk     0xFFFF0000UL
/* CPUSS.CM7_0_PWR_DELAY_CTL */
#define CPUSS_CM7_0_PWR_DELAY_CTL_UP_Pos        0UL
#define CPUSS_CM7_0_PWR_DELAY_CTL_UP_Msk        0x3FFUL
/* CPUSS.CM7_1_PWR_CTL */
#define CPUSS_CM7_1_PWR_CTL_PWR_MODE_Pos        0UL
#define CPUSS_CM7_1_PWR_CTL_PWR_MODE_Msk        0x3UL
#define CPUSS_CM7_1_PWR_CTL_VECTKEYSTAT_Pos     16UL
#define CPUSS_CM7_1_PWR_CTL_VECTKEYSTAT_Msk     0xFFFF0000UL
/* CPUSS.CM7_1_PWR_DELAY_CTL */
#define CPUSS_CM7_1_PWR_DELAY_CTL_UP_Pos        0UL
#define CPUSS_CM7_1_PWR_DELAY_CTL_UP_Msk        0x3FFUL
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
#define CPUSS_RAM0_CTL0_ECC_CHECK_DIS_Pos       19UL
#define CPUSS_RAM0_CTL0_ECC_CHECK_DIS_Msk       0x80000UL
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
#define CPUSS_RAM1_CTL0_ECC_CHECK_DIS_Pos       19UL
#define CPUSS_RAM1_CTL0_ECC_CHECK_DIS_Msk       0x80000UL
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
#define CPUSS_RAM2_CTL0_ECC_CHECK_DIS_Pos       19UL
#define CPUSS_RAM2_CTL0_ECC_CHECK_DIS_Msk       0x80000UL
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
#define CPUSS_ECC_CTL_WORD_ADDR_Msk             0xFFFFFFUL
#define CPUSS_ECC_CTL_PARITY_Pos                24UL
#define CPUSS_ECC_CTL_PARITY_Msk                0xFF000000UL
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
#define CPUSS_AP_CTL_CM7_ENABLE_Pos             1UL
#define CPUSS_AP_CTL_CM7_ENABLE_Msk             0x2UL
#define CPUSS_AP_CTL_SYS_ENABLE_Pos             2UL
#define CPUSS_AP_CTL_SYS_ENABLE_Msk             0x4UL
#define CPUSS_AP_CTL_CM0_DISABLE_Pos            16UL
#define CPUSS_AP_CTL_CM0_DISABLE_Msk            0x10000UL
#define CPUSS_AP_CTL_CM7_DISABLE_Pos            17UL
#define CPUSS_AP_CTL_CM7_DISABLE_Msk            0x20000UL
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
/* CPUSS.TRIM_RAM200_CTL */
#define CPUSS_TRIM_RAM200_CTL_TRIM_Pos          0UL
#define CPUSS_TRIM_RAM200_CTL_TRIM_Msk          0xFFFFFFFFUL
/* CPUSS.TRIM_RAM350_CTL */
#define CPUSS_TRIM_RAM350_CTL_TRIM_Pos          0UL
#define CPUSS_TRIM_RAM350_CTL_TRIM_Msk          0xFFFFFFFFUL
/* CPUSS.CM0_SYSTEM_INT_CTL */
#define CPUSS_CM0_SYSTEM_INT_CTL_CM0_CPU_INT_IDX_Pos 0UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CM0_CPU_INT_IDX_Msk 0x7UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_VALID_Pos 31UL
#define CPUSS_CM0_SYSTEM_INT_CTL_CPU_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM7_0_SYSTEM_INT_CTL */
#define CPUSS_CM7_0_SYSTEM_INT_CTL_CPU_INT_IDX_Pos 0UL
#define CPUSS_CM7_0_SYSTEM_INT_CTL_CPU_INT_IDX_Msk 0xFUL
#define CPUSS_CM7_0_SYSTEM_INT_CTL_CPU_INT_VALID_Pos 31UL
#define CPUSS_CM7_0_SYSTEM_INT_CTL_CPU_INT_VALID_Msk 0x80000000UL
/* CPUSS.CM7_1_SYSTEM_INT_CTL */
#define CPUSS_CM7_1_SYSTEM_INT_CTL_CPU_INT_IDX_Pos 0UL
#define CPUSS_CM7_1_SYSTEM_INT_CTL_CPU_INT_IDX_Msk 0xFUL
#define CPUSS_CM7_1_SYSTEM_INT_CTL_CPU_INT_VALID_Pos 31UL
#define CPUSS_CM7_1_SYSTEM_INT_CTL_CPU_INT_VALID_Msk 0x80000000UL


#endif /* _CYIP_CPUSS_H_ */


/* [] END OF FILE */
