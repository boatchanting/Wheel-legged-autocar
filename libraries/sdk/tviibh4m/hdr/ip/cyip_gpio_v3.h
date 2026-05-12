/***************************************************************************//**
* \file cyip_gpio_v3.h
*
* \brief
* GPIO IP definitions
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

#ifndef _CYIP_GPIO_V3_H_
#define _CYIP_GPIO_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     GPIO
*******************************************************************************/

#define GPIO_PRT_SECTION_SIZE                   0x00000080UL
#define GPIO_SECTION_SIZE                       0x00010000UL

/**
  * \brief Port output data register (GPIO_PRT_OUT)
  */
typedef struct stc_GPIO_PRT_OUT_field {
  uint32_t                         u1OUT0:1;
  uint32_t                         u1OUT1:1;
  uint32_t                         u1OUT2:1;
  uint32_t                         u1OUT3:1;
  uint32_t                         u1OUT4:1;
  uint32_t                         u1OUT5:1;
  uint32_t                         u1OUT6:1;
  uint32_t                         u1OUT7:1;
  uint32_t                         :24;
} stc_GPIO_PRT_OUT_field_t;

typedef union un_GPIO_PRT_OUT {
  uint32_t                         u32Register;
  stc_GPIO_PRT_OUT_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_OUT_t;

/**
  * \brief Port output data clear register (GPIO_PRT_OUT_CLR)
  */
typedef struct stc_GPIO_PRT_OUT_CLR_field {
  uint32_t                         u1OUT0:1;
  uint32_t                         u1OUT1:1;
  uint32_t                         u1OUT2:1;
  uint32_t                         u1OUT3:1;
  uint32_t                         u1OUT4:1;
  uint32_t                         u1OUT5:1;
  uint32_t                         u1OUT6:1;
  uint32_t                         u1OUT7:1;
  uint32_t                         :24;
} stc_GPIO_PRT_OUT_CLR_field_t;

typedef union un_GPIO_PRT_OUT_CLR {
  uint32_t                         u32Register;
  stc_GPIO_PRT_OUT_CLR_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_OUT_CLR_t;

/**
  * \brief Port output data set register (GPIO_PRT_OUT_SET)
  */
typedef struct stc_GPIO_PRT_OUT_SET_field {
  uint32_t                         u1OUT0:1;
  uint32_t                         u1OUT1:1;
  uint32_t                         u1OUT2:1;
  uint32_t                         u1OUT3:1;
  uint32_t                         u1OUT4:1;
  uint32_t                         u1OUT5:1;
  uint32_t                         u1OUT6:1;
  uint32_t                         u1OUT7:1;
  uint32_t                         :24;
} stc_GPIO_PRT_OUT_SET_field_t;

typedef union un_GPIO_PRT_OUT_SET {
  uint32_t                         u32Register;
  stc_GPIO_PRT_OUT_SET_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_OUT_SET_t;

/**
  * \brief Port output data invert register (GPIO_PRT_OUT_INV)
  */
typedef struct stc_GPIO_PRT_OUT_INV_field {
  uint32_t                         u1OUT0:1;
  uint32_t                         u1OUT1:1;
  uint32_t                         u1OUT2:1;
  uint32_t                         u1OUT3:1;
  uint32_t                         u1OUT4:1;
  uint32_t                         u1OUT5:1;
  uint32_t                         u1OUT6:1;
  uint32_t                         u1OUT7:1;
  uint32_t                         :24;
} stc_GPIO_PRT_OUT_INV_field_t;

typedef union un_GPIO_PRT_OUT_INV {
  uint32_t                         u32Register;
  stc_GPIO_PRT_OUT_INV_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_OUT_INV_t;

/**
  * \brief Port input state register (GPIO_PRT_IN)
  */
typedef struct stc_GPIO_PRT_IN_field {
  uint32_t                         u1IN0:1;
  uint32_t                         u1IN1:1;
  uint32_t                         u1IN2:1;
  uint32_t                         u1IN3:1;
  uint32_t                         u1IN4:1;
  uint32_t                         u1IN5:1;
  uint32_t                         u1IN6:1;
  uint32_t                         u1IN7:1;
  uint32_t                         u1FLT_IN:1;
  uint32_t                         :23;
} stc_GPIO_PRT_IN_field_t;

typedef union un_GPIO_PRT_IN {
  uint32_t                         u32Register;
  stc_GPIO_PRT_IN_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_IN_t;

/**
  * \brief Port interrupt status register (GPIO_PRT_INTR)
  */
typedef struct stc_GPIO_PRT_INTR_field {
  uint32_t                         u1EDGE0:1;
  uint32_t                         u1EDGE1:1;
  uint32_t                         u1EDGE2:1;
  uint32_t                         u1EDGE3:1;
  uint32_t                         u1EDGE4:1;
  uint32_t                         u1EDGE5:1;
  uint32_t                         u1EDGE6:1;
  uint32_t                         u1EDGE7:1;
  uint32_t                         u1FLT_EDGE:1;
  uint32_t                         :7;
  uint32_t                         u1IN_IN0:1;
  uint32_t                         u1IN_IN1:1;
  uint32_t                         u1IN_IN2:1;
  uint32_t                         u1IN_IN3:1;
  uint32_t                         u1IN_IN4:1;
  uint32_t                         u1IN_IN5:1;
  uint32_t                         u1IN_IN6:1;
  uint32_t                         u1IN_IN7:1;
  uint32_t                         u1FLT_IN_IN:1;
  uint32_t                         :7;
} stc_GPIO_PRT_INTR_field_t;

typedef union un_GPIO_PRT_INTR {
  uint32_t                         u32Register;
  stc_GPIO_PRT_INTR_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_INTR_t;

/**
  * \brief Port interrupt mask register (GPIO_PRT_INTR_MASK)
  */
typedef struct stc_GPIO_PRT_INTR_MASK_field {
  uint32_t                         u1EDGE0:1;
  uint32_t                         u1EDGE1:1;
  uint32_t                         u1EDGE2:1;
  uint32_t                         u1EDGE3:1;
  uint32_t                         u1EDGE4:1;
  uint32_t                         u1EDGE5:1;
  uint32_t                         u1EDGE6:1;
  uint32_t                         u1EDGE7:1;
  uint32_t                         u1FLT_EDGE:1;
  uint32_t                         :23;
} stc_GPIO_PRT_INTR_MASK_field_t;

typedef union un_GPIO_PRT_INTR_MASK {
  uint32_t                         u32Register;
  stc_GPIO_PRT_INTR_MASK_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_INTR_MASK_t;

/**
  * \brief Port interrupt masked status register (GPIO_PRT_INTR_MASKED)
  */
typedef struct stc_GPIO_PRT_INTR_MASKED_field {
  uint32_t                         u1EDGE0:1;
  uint32_t                         u1EDGE1:1;
  uint32_t                         u1EDGE2:1;
  uint32_t                         u1EDGE3:1;
  uint32_t                         u1EDGE4:1;
  uint32_t                         u1EDGE5:1;
  uint32_t                         u1EDGE6:1;
  uint32_t                         u1EDGE7:1;
  uint32_t                         u1FLT_EDGE:1;
  uint32_t                         :23;
} stc_GPIO_PRT_INTR_MASKED_field_t;

typedef union un_GPIO_PRT_INTR_MASKED {
  uint32_t                         u32Register;
  stc_GPIO_PRT_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_INTR_MASKED_t;

/**
  * \brief Port interrupt set register (GPIO_PRT_INTR_SET)
  */
typedef struct stc_GPIO_PRT_INTR_SET_field {
  uint32_t                         u1EDGE0:1;
  uint32_t                         u1EDGE1:1;
  uint32_t                         u1EDGE2:1;
  uint32_t                         u1EDGE3:1;
  uint32_t                         u1EDGE4:1;
  uint32_t                         u1EDGE5:1;
  uint32_t                         u1EDGE6:1;
  uint32_t                         u1EDGE7:1;
  uint32_t                         u1FLT_EDGE:1;
  uint32_t                         :23;
} stc_GPIO_PRT_INTR_SET_field_t;

typedef union un_GPIO_PRT_INTR_SET {
  uint32_t                         u32Register;
  stc_GPIO_PRT_INTR_SET_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_INTR_SET_t;

/**
  * \brief Port interrupt configuration register (GPIO_PRT_INTR_CFG)
  */
typedef struct stc_GPIO_PRT_INTR_CFG_field {
  uint32_t                         u2EDGE0_SEL:2;
  uint32_t                         u2EDGE1_SEL:2;
  uint32_t                         u2EDGE2_SEL:2;
  uint32_t                         u2EDGE3_SEL:2;
  uint32_t                         u2EDGE4_SEL:2;
  uint32_t                         u2EDGE5_SEL:2;
  uint32_t                         u2EDGE6_SEL:2;
  uint32_t                         u2EDGE7_SEL:2;
  uint32_t                         u2FLT_EDGE_SEL:2;
  uint32_t                         u3FLT_SEL:3;
  uint32_t                         :11;
} stc_GPIO_PRT_INTR_CFG_field_t;

typedef union un_GPIO_PRT_INTR_CFG {
  uint32_t                         u32Register;
  stc_GPIO_PRT_INTR_CFG_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_INTR_CFG_t;

/**
  * \brief Port configuration register (GPIO_PRT_CFG)
  */
typedef struct stc_GPIO_PRT_CFG_field {
  uint32_t                         u3DRIVE_MODE0:3;
  uint32_t                         u1IN_EN0:1;
  uint32_t                         u3DRIVE_MODE1:3;
  uint32_t                         u1IN_EN1:1;
  uint32_t                         u3DRIVE_MODE2:3;
  uint32_t                         u1IN_EN2:1;
  uint32_t                         u3DRIVE_MODE3:3;
  uint32_t                         u1IN_EN3:1;
  uint32_t                         u3DRIVE_MODE4:3;
  uint32_t                         u1IN_EN4:1;
  uint32_t                         u3DRIVE_MODE5:3;
  uint32_t                         u1IN_EN5:1;
  uint32_t                         u3DRIVE_MODE6:3;
  uint32_t                         u1IN_EN6:1;
  uint32_t                         u3DRIVE_MODE7:3;
  uint32_t                         u1IN_EN7:1;
} stc_GPIO_PRT_CFG_field_t;

typedef union un_GPIO_PRT_CFG {
  uint32_t                         u32Register;
  stc_GPIO_PRT_CFG_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_CFG_t;

/**
  * \brief Port input buffer configuration register (GPIO_PRT_CFG_IN)
  */
typedef struct stc_GPIO_PRT_CFG_IN_field {
  uint32_t                         u1VTRIP_SEL0_0:1;
  uint32_t                         u1VTRIP_SEL1_0:1;
  uint32_t                         u1VTRIP_SEL2_0:1;
  uint32_t                         u1VTRIP_SEL3_0:1;
  uint32_t                         u1VTRIP_SEL4_0:1;
  uint32_t                         u1VTRIP_SEL5_0:1;
  uint32_t                         u1VTRIP_SEL6_0:1;
  uint32_t                         u1VTRIP_SEL7_0:1;
  uint32_t                         :24;
} stc_GPIO_PRT_CFG_IN_field_t;

typedef union un_GPIO_PRT_CFG_IN {
  uint32_t                         u32Register;
  stc_GPIO_PRT_CFG_IN_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_CFG_IN_t;

/**
  * \brief Port output buffer configuration register (GPIO_PRT_CFG_OUT)
  */
typedef struct stc_GPIO_PRT_CFG_OUT_field {
  uint32_t                         u1SLOW0:1;
  uint32_t                         u1SLOW1:1;
  uint32_t                         u1SLOW2:1;
  uint32_t                         u1SLOW3:1;
  uint32_t                         u1SLOW4:1;
  uint32_t                         u1SLOW5:1;
  uint32_t                         u1SLOW6:1;
  uint32_t                         u1SLOW7:1;
  uint32_t                         :8;
  uint32_t                         u2DRIVE_SEL0:2;
  uint32_t                         u2DRIVE_SEL1:2;
  uint32_t                         u2DRIVE_SEL2:2;
  uint32_t                         u2DRIVE_SEL3:2;
  uint32_t                         u2DRIVE_SEL4:2;
  uint32_t                         u2DRIVE_SEL5:2;
  uint32_t                         u2DRIVE_SEL6:2;
  uint32_t                         u2DRIVE_SEL7:2;
} stc_GPIO_PRT_CFG_OUT_field_t;

typedef union un_GPIO_PRT_CFG_OUT {
  uint32_t                         u32Register;
  stc_GPIO_PRT_CFG_OUT_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_CFG_OUT_t;

/**
  * \brief Port SIO configuration register (GPIO_PRT_CFG_SIO)
  */
typedef struct stc_GPIO_PRT_CFG_SIO_field {
  uint32_t                         u1VREG_EN01:1;
  uint32_t                         u1IBUF_SEL01:1;
  uint32_t                         u1VTRIP_SEL01:1;
  uint32_t                         u2VREF_SEL01:2;
  uint32_t                         u3VOH_SEL01:3;
  uint32_t                         u1VREG_EN23:1;
  uint32_t                         u1IBUF_SEL23:1;
  uint32_t                         u1VTRIP_SEL23:1;
  uint32_t                         u2VREF_SEL23:2;
  uint32_t                         u3VOH_SEL23:3;
  uint32_t                         u1VREG_EN45:1;
  uint32_t                         u1IBUF_SEL45:1;
  uint32_t                         u1VTRIP_SEL45:1;
  uint32_t                         u2VREF_SEL45:2;
  uint32_t                         u3VOH_SEL45:3;
  uint32_t                         u1VREG_EN67:1;
  uint32_t                         u1IBUF_SEL67:1;
  uint32_t                         u1VTRIP_SEL67:1;
  uint32_t                         u2VREF_SEL67:2;
  uint32_t                         u3VOH_SEL67:3;
} stc_GPIO_PRT_CFG_SIO_field_t;

typedef union un_GPIO_PRT_CFG_SIO {
  uint32_t                         u32Register;
  stc_GPIO_PRT_CFG_SIO_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_CFG_SIO_t;

/**
  * \brief Port input buffer AUTOLVL configuration register (GPIO_PRT_CFG_IN_AUTOLVL)
  */
typedef struct stc_GPIO_PRT_CFG_IN_AUTOLVL_field {
  uint32_t                         u1VTRIP_SEL0_1:1;
  uint32_t                         u1VTRIP_SEL1_1:1;
  uint32_t                         u1VTRIP_SEL2_1:1;
  uint32_t                         u1VTRIP_SEL3_1:1;
  uint32_t                         u1VTRIP_SEL4_1:1;
  uint32_t                         u1VTRIP_SEL5_1:1;
  uint32_t                         u1VTRIP_SEL6_1:1;
  uint32_t                         u1VTRIP_SEL7_1:1;
  uint32_t                         :24;
} stc_GPIO_PRT_CFG_IN_AUTOLVL_field_t;

typedef union un_GPIO_PRT_CFG_IN_AUTOLVL {
  uint32_t                         u32Register;
  stc_GPIO_PRT_CFG_IN_AUTOLVL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_PRT_CFG_IN_AUTOLVL_t;



/**
  * \brief Interrupt port cause register 0 (GPIO_INTR_CAUSE0)
  */
typedef struct stc_GPIO_INTR_CAUSE0_field {
  uint32_t                         u32PORT_INT:32;
} stc_GPIO_INTR_CAUSE0_field_t;

typedef union un_GPIO_INTR_CAUSE0 {
  uint32_t                         u32Register;
  stc_GPIO_INTR_CAUSE0_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_INTR_CAUSE0_t;

/**
  * \brief Interrupt port cause register 1 (GPIO_INTR_CAUSE1)
  */
typedef struct stc_GPIO_INTR_CAUSE1_field {
  uint32_t                         u32PORT_INT:32;
} stc_GPIO_INTR_CAUSE1_field_t;

typedef union un_GPIO_INTR_CAUSE1 {
  uint32_t                         u32Register;
  stc_GPIO_INTR_CAUSE1_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_INTR_CAUSE1_t;

/**
  * \brief Interrupt port cause register 2 (GPIO_INTR_CAUSE2)
  */
typedef struct stc_GPIO_INTR_CAUSE2_field {
  uint32_t                         u32PORT_INT:32;
} stc_GPIO_INTR_CAUSE2_field_t;

typedef union un_GPIO_INTR_CAUSE2 {
  uint32_t                         u32Register;
  stc_GPIO_INTR_CAUSE2_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_INTR_CAUSE2_t;

/**
  * \brief Interrupt port cause register 3 (GPIO_INTR_CAUSE3)
  */
typedef struct stc_GPIO_INTR_CAUSE3_field {
  uint32_t                         u32PORT_INT:32;
} stc_GPIO_INTR_CAUSE3_field_t;

typedef union un_GPIO_INTR_CAUSE3 {
  uint32_t                         u32Register;
  stc_GPIO_INTR_CAUSE3_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_INTR_CAUSE3_t;

/**
  * \brief Extern power supply detection register (GPIO_VDD_ACTIVE)
  */
typedef struct stc_GPIO_VDD_ACTIVE_field {
  uint32_t                         u16VDDIO_ACTIVE:16;
  uint32_t                         :14;
  uint32_t                         u1VDDA_ACTIVE:1;
  uint32_t                         u1VDDD_ACTIVE:1;
} stc_GPIO_VDD_ACTIVE_field_t;

typedef union un_GPIO_VDD_ACTIVE {
  uint32_t                         u32Register;
  stc_GPIO_VDD_ACTIVE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_VDD_ACTIVE_t;

/**
  * \brief Supply detection interrupt register (GPIO_VDD_INTR)
  */
typedef struct stc_GPIO_VDD_INTR_field {
  uint32_t                         u16VDDIO_ACTIVE:16;
  uint32_t                         :14;
  uint32_t                         u1VDDA_ACTIVE:1;
  uint32_t                         u1VDDD_ACTIVE:1;
} stc_GPIO_VDD_INTR_field_t;

typedef union un_GPIO_VDD_INTR {
  uint32_t                         u32Register;
  stc_GPIO_VDD_INTR_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_VDD_INTR_t;

/**
  * \brief Supply detection interrupt mask register (GPIO_VDD_INTR_MASK)
  */
typedef struct stc_GPIO_VDD_INTR_MASK_field {
  uint32_t                         u16VDDIO_ACTIVE:16;
  uint32_t                         :14;
  uint32_t                         u1VDDA_ACTIVE:1;
  uint32_t                         u1VDDD_ACTIVE:1;
} stc_GPIO_VDD_INTR_MASK_field_t;

typedef union un_GPIO_VDD_INTR_MASK {
  uint32_t                         u32Register;
  stc_GPIO_VDD_INTR_MASK_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_VDD_INTR_MASK_t;

/**
  * \brief Supply detection interrupt masked register (GPIO_VDD_INTR_MASKED)
  */
typedef struct stc_GPIO_VDD_INTR_MASKED_field {
  uint32_t                         u16VDDIO_ACTIVE:16;
  uint32_t                         :14;
  uint32_t                         u1VDDA_ACTIVE:1;
  uint32_t                         u1VDDD_ACTIVE:1;
} stc_GPIO_VDD_INTR_MASKED_field_t;

typedef union un_GPIO_VDD_INTR_MASKED {
  uint32_t                         u32Register;
  stc_GPIO_VDD_INTR_MASKED_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_VDD_INTR_MASKED_t;

/**
  * \brief Supply detection interrupt set register (GPIO_VDD_INTR_SET)
  */
typedef struct stc_GPIO_VDD_INTR_SET_field {
  uint32_t                         u16VDDIO_ACTIVE:16;
  uint32_t                         :14;
  uint32_t                         u1VDDA_ACTIVE:1;
  uint32_t                         u1VDDD_ACTIVE:1;
} stc_GPIO_VDD_INTR_SET_field_t;

typedef union un_GPIO_VDD_INTR_SET {
  uint32_t                         u32Register;
  stc_GPIO_VDD_INTR_SET_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_GPIO_VDD_INTR_SET_t;



/**
  * \brief GPIO port registers (GPIO_PRT)
  */
typedef struct stc_GPIO_PRT {
  un_GPIO_PRT_OUT_t                unOUT;              /*!< 0x00000000 Port output data register */
  un_GPIO_PRT_OUT_CLR_t            unOUT_CLR;          /*!< 0x00000004 Port output data clear register */
  un_GPIO_PRT_OUT_SET_t            unOUT_SET;          /*!< 0x00000008 Port output data set register */
  un_GPIO_PRT_OUT_INV_t            unOUT_INV;          /*!< 0x0000000C Port output data invert register */
  un_GPIO_PRT_IN_t                 unIN;               /*!< 0x00000010 Port input state register */
  un_GPIO_PRT_INTR_t               unINTR;             /*!< 0x00000014 Port interrupt status register */
  un_GPIO_PRT_INTR_MASK_t          unINTR_MASK;        /*!< 0x00000018 Port interrupt mask register */
  un_GPIO_PRT_INTR_MASKED_t        unINTR_MASKED;      /*!< 0x0000001C Port interrupt masked status register */
  un_GPIO_PRT_INTR_SET_t           unINTR_SET;         /*!< 0x00000020 Port interrupt set register */
  uint32_t                         au32Reserved[7];
  un_GPIO_PRT_INTR_CFG_t           unINTR_CFG;         /*!< 0x00000040 Port interrupt configuration register */
  un_GPIO_PRT_CFG_t                unCFG;              /*!< 0x00000044 Port configuration register */
  un_GPIO_PRT_CFG_IN_t             unCFG_IN;           /*!< 0x00000048 Port input buffer configuration register */
  un_GPIO_PRT_CFG_OUT_t            unCFG_OUT;          /*!< 0x0000004C Port output buffer configuration register */
  un_GPIO_PRT_CFG_SIO_t            unCFG_SIO;          /*!< 0x00000050 Port SIO configuration register */
  uint32_t                         u32Reserved1;
  un_GPIO_PRT_CFG_IN_AUTOLVL_t     unCFG_IN_AUTOLVL;   /*!< 0x00000058 Port input buffer AUTOLVL configuration register */
  uint32_t                         au32Reserved2[9];
} stc_GPIO_PRT_t;                                      /*!< Size = 128 (0x80) */

/**
  * \brief GPIO port control/configuration (GPIO)
  */
typedef struct stc_GPIO {
  stc_GPIO_PRT_t                   PRT[128];           /*!< 0x00000000 GPIO port registers */
  un_GPIO_INTR_CAUSE0_t            unINTR_CAUSE0;      /*!< 0x00004000 Interrupt port cause register 0 */
  un_GPIO_INTR_CAUSE1_t            unINTR_CAUSE1;      /*!< 0x00004004 Interrupt port cause register 1 */
  un_GPIO_INTR_CAUSE2_t            unINTR_CAUSE2;      /*!< 0x00004008 Interrupt port cause register 2 */
  un_GPIO_INTR_CAUSE3_t            unINTR_CAUSE3;      /*!< 0x0000400C Interrupt port cause register 3 */
  un_GPIO_VDD_ACTIVE_t             unVDD_ACTIVE;       /*!< 0x00004010 Extern power supply detection register */
  un_GPIO_VDD_INTR_t               unVDD_INTR;         /*!< 0x00004014 Supply detection interrupt register */
  un_GPIO_VDD_INTR_MASK_t          unVDD_INTR_MASK;    /*!< 0x00004018 Supply detection interrupt mask register */
  un_GPIO_VDD_INTR_MASKED_t        unVDD_INTR_MASKED;  /*!< 0x0000401C Supply detection interrupt masked register */
  un_GPIO_VDD_INTR_SET_t           unVDD_INTR_SET;     /*!< 0x00004020 Supply detection interrupt set register */
} stc_GPIO_t;                                          /*!< Size = 16420 (0x4024) */


/* GPIO_PRT.OUT */
#define GPIO_PRT_OUT_OUT0_Pos                   0UL
#define GPIO_PRT_OUT_OUT0_Msk                   0x1UL
#define GPIO_PRT_OUT_OUT1_Pos                   1UL
#define GPIO_PRT_OUT_OUT1_Msk                   0x2UL
#define GPIO_PRT_OUT_OUT2_Pos                   2UL
#define GPIO_PRT_OUT_OUT2_Msk                   0x4UL
#define GPIO_PRT_OUT_OUT3_Pos                   3UL
#define GPIO_PRT_OUT_OUT3_Msk                   0x8UL
#define GPIO_PRT_OUT_OUT4_Pos                   4UL
#define GPIO_PRT_OUT_OUT4_Msk                   0x10UL
#define GPIO_PRT_OUT_OUT5_Pos                   5UL
#define GPIO_PRT_OUT_OUT5_Msk                   0x20UL
#define GPIO_PRT_OUT_OUT6_Pos                   6UL
#define GPIO_PRT_OUT_OUT6_Msk                   0x40UL
#define GPIO_PRT_OUT_OUT7_Pos                   7UL
#define GPIO_PRT_OUT_OUT7_Msk                   0x80UL
/* GPIO_PRT.OUT_CLR */
#define GPIO_PRT_OUT_CLR_OUT0_Pos               0UL
#define GPIO_PRT_OUT_CLR_OUT0_Msk               0x1UL
#define GPIO_PRT_OUT_CLR_OUT1_Pos               1UL
#define GPIO_PRT_OUT_CLR_OUT1_Msk               0x2UL
#define GPIO_PRT_OUT_CLR_OUT2_Pos               2UL
#define GPIO_PRT_OUT_CLR_OUT2_Msk               0x4UL
#define GPIO_PRT_OUT_CLR_OUT3_Pos               3UL
#define GPIO_PRT_OUT_CLR_OUT3_Msk               0x8UL
#define GPIO_PRT_OUT_CLR_OUT4_Pos               4UL
#define GPIO_PRT_OUT_CLR_OUT4_Msk               0x10UL
#define GPIO_PRT_OUT_CLR_OUT5_Pos               5UL
#define GPIO_PRT_OUT_CLR_OUT5_Msk               0x20UL
#define GPIO_PRT_OUT_CLR_OUT6_Pos               6UL
#define GPIO_PRT_OUT_CLR_OUT6_Msk               0x40UL
#define GPIO_PRT_OUT_CLR_OUT7_Pos               7UL
#define GPIO_PRT_OUT_CLR_OUT7_Msk               0x80UL
/* GPIO_PRT.OUT_SET */
#define GPIO_PRT_OUT_SET_OUT0_Pos               0UL
#define GPIO_PRT_OUT_SET_OUT0_Msk               0x1UL
#define GPIO_PRT_OUT_SET_OUT1_Pos               1UL
#define GPIO_PRT_OUT_SET_OUT1_Msk               0x2UL
#define GPIO_PRT_OUT_SET_OUT2_Pos               2UL
#define GPIO_PRT_OUT_SET_OUT2_Msk               0x4UL
#define GPIO_PRT_OUT_SET_OUT3_Pos               3UL
#define GPIO_PRT_OUT_SET_OUT3_Msk               0x8UL
#define GPIO_PRT_OUT_SET_OUT4_Pos               4UL
#define GPIO_PRT_OUT_SET_OUT4_Msk               0x10UL
#define GPIO_PRT_OUT_SET_OUT5_Pos               5UL
#define GPIO_PRT_OUT_SET_OUT5_Msk               0x20UL
#define GPIO_PRT_OUT_SET_OUT6_Pos               6UL
#define GPIO_PRT_OUT_SET_OUT6_Msk               0x40UL
#define GPIO_PRT_OUT_SET_OUT7_Pos               7UL
#define GPIO_PRT_OUT_SET_OUT7_Msk               0x80UL
/* GPIO_PRT.OUT_INV */
#define GPIO_PRT_OUT_INV_OUT0_Pos               0UL
#define GPIO_PRT_OUT_INV_OUT0_Msk               0x1UL
#define GPIO_PRT_OUT_INV_OUT1_Pos               1UL
#define GPIO_PRT_OUT_INV_OUT1_Msk               0x2UL
#define GPIO_PRT_OUT_INV_OUT2_Pos               2UL
#define GPIO_PRT_OUT_INV_OUT2_Msk               0x4UL
#define GPIO_PRT_OUT_INV_OUT3_Pos               3UL
#define GPIO_PRT_OUT_INV_OUT3_Msk               0x8UL
#define GPIO_PRT_OUT_INV_OUT4_Pos               4UL
#define GPIO_PRT_OUT_INV_OUT4_Msk               0x10UL
#define GPIO_PRT_OUT_INV_OUT5_Pos               5UL
#define GPIO_PRT_OUT_INV_OUT5_Msk               0x20UL
#define GPIO_PRT_OUT_INV_OUT6_Pos               6UL
#define GPIO_PRT_OUT_INV_OUT6_Msk               0x40UL
#define GPIO_PRT_OUT_INV_OUT7_Pos               7UL
#define GPIO_PRT_OUT_INV_OUT7_Msk               0x80UL
/* GPIO_PRT.IN */
#define GPIO_PRT_IN_IN0_Pos                     0UL
#define GPIO_PRT_IN_IN0_Msk                     0x1UL
#define GPIO_PRT_IN_IN1_Pos                     1UL
#define GPIO_PRT_IN_IN1_Msk                     0x2UL
#define GPIO_PRT_IN_IN2_Pos                     2UL
#define GPIO_PRT_IN_IN2_Msk                     0x4UL
#define GPIO_PRT_IN_IN3_Pos                     3UL
#define GPIO_PRT_IN_IN3_Msk                     0x8UL
#define GPIO_PRT_IN_IN4_Pos                     4UL
#define GPIO_PRT_IN_IN4_Msk                     0x10UL
#define GPIO_PRT_IN_IN5_Pos                     5UL
#define GPIO_PRT_IN_IN5_Msk                     0x20UL
#define GPIO_PRT_IN_IN6_Pos                     6UL
#define GPIO_PRT_IN_IN6_Msk                     0x40UL
#define GPIO_PRT_IN_IN7_Pos                     7UL
#define GPIO_PRT_IN_IN7_Msk                     0x80UL
#define GPIO_PRT_IN_FLT_IN_Pos                  8UL
#define GPIO_PRT_IN_FLT_IN_Msk                  0x100UL
/* GPIO_PRT.INTR */
#define GPIO_PRT_INTR_EDGE0_Pos                 0UL
#define GPIO_PRT_INTR_EDGE0_Msk                 0x1UL
#define GPIO_PRT_INTR_EDGE1_Pos                 1UL
#define GPIO_PRT_INTR_EDGE1_Msk                 0x2UL
#define GPIO_PRT_INTR_EDGE2_Pos                 2UL
#define GPIO_PRT_INTR_EDGE2_Msk                 0x4UL
#define GPIO_PRT_INTR_EDGE3_Pos                 3UL
#define GPIO_PRT_INTR_EDGE3_Msk                 0x8UL
#define GPIO_PRT_INTR_EDGE4_Pos                 4UL
#define GPIO_PRT_INTR_EDGE4_Msk                 0x10UL
#define GPIO_PRT_INTR_EDGE5_Pos                 5UL
#define GPIO_PRT_INTR_EDGE5_Msk                 0x20UL
#define GPIO_PRT_INTR_EDGE6_Pos                 6UL
#define GPIO_PRT_INTR_EDGE6_Msk                 0x40UL
#define GPIO_PRT_INTR_EDGE7_Pos                 7UL
#define GPIO_PRT_INTR_EDGE7_Msk                 0x80UL
#define GPIO_PRT_INTR_FLT_EDGE_Pos              8UL
#define GPIO_PRT_INTR_FLT_EDGE_Msk              0x100UL
#define GPIO_PRT_INTR_IN_IN0_Pos                16UL
#define GPIO_PRT_INTR_IN_IN0_Msk                0x10000UL
#define GPIO_PRT_INTR_IN_IN1_Pos                17UL
#define GPIO_PRT_INTR_IN_IN1_Msk                0x20000UL
#define GPIO_PRT_INTR_IN_IN2_Pos                18UL
#define GPIO_PRT_INTR_IN_IN2_Msk                0x40000UL
#define GPIO_PRT_INTR_IN_IN3_Pos                19UL
#define GPIO_PRT_INTR_IN_IN3_Msk                0x80000UL
#define GPIO_PRT_INTR_IN_IN4_Pos                20UL
#define GPIO_PRT_INTR_IN_IN4_Msk                0x100000UL
#define GPIO_PRT_INTR_IN_IN5_Pos                21UL
#define GPIO_PRT_INTR_IN_IN5_Msk                0x200000UL
#define GPIO_PRT_INTR_IN_IN6_Pos                22UL
#define GPIO_PRT_INTR_IN_IN6_Msk                0x400000UL
#define GPIO_PRT_INTR_IN_IN7_Pos                23UL
#define GPIO_PRT_INTR_IN_IN7_Msk                0x800000UL
#define GPIO_PRT_INTR_FLT_IN_IN_Pos             24UL
#define GPIO_PRT_INTR_FLT_IN_IN_Msk             0x1000000UL
/* GPIO_PRT.INTR_MASK */
#define GPIO_PRT_INTR_MASK_EDGE0_Pos            0UL
#define GPIO_PRT_INTR_MASK_EDGE0_Msk            0x1UL
#define GPIO_PRT_INTR_MASK_EDGE1_Pos            1UL
#define GPIO_PRT_INTR_MASK_EDGE1_Msk            0x2UL
#define GPIO_PRT_INTR_MASK_EDGE2_Pos            2UL
#define GPIO_PRT_INTR_MASK_EDGE2_Msk            0x4UL
#define GPIO_PRT_INTR_MASK_EDGE3_Pos            3UL
#define GPIO_PRT_INTR_MASK_EDGE3_Msk            0x8UL
#define GPIO_PRT_INTR_MASK_EDGE4_Pos            4UL
#define GPIO_PRT_INTR_MASK_EDGE4_Msk            0x10UL
#define GPIO_PRT_INTR_MASK_EDGE5_Pos            5UL
#define GPIO_PRT_INTR_MASK_EDGE5_Msk            0x20UL
#define GPIO_PRT_INTR_MASK_EDGE6_Pos            6UL
#define GPIO_PRT_INTR_MASK_EDGE6_Msk            0x40UL
#define GPIO_PRT_INTR_MASK_EDGE7_Pos            7UL
#define GPIO_PRT_INTR_MASK_EDGE7_Msk            0x80UL
#define GPIO_PRT_INTR_MASK_FLT_EDGE_Pos         8UL
#define GPIO_PRT_INTR_MASK_FLT_EDGE_Msk         0x100UL
/* GPIO_PRT.INTR_MASKED */
#define GPIO_PRT_INTR_MASKED_EDGE0_Pos          0UL
#define GPIO_PRT_INTR_MASKED_EDGE0_Msk          0x1UL
#define GPIO_PRT_INTR_MASKED_EDGE1_Pos          1UL
#define GPIO_PRT_INTR_MASKED_EDGE1_Msk          0x2UL
#define GPIO_PRT_INTR_MASKED_EDGE2_Pos          2UL
#define GPIO_PRT_INTR_MASKED_EDGE2_Msk          0x4UL
#define GPIO_PRT_INTR_MASKED_EDGE3_Pos          3UL
#define GPIO_PRT_INTR_MASKED_EDGE3_Msk          0x8UL
#define GPIO_PRT_INTR_MASKED_EDGE4_Pos          4UL
#define GPIO_PRT_INTR_MASKED_EDGE4_Msk          0x10UL
#define GPIO_PRT_INTR_MASKED_EDGE5_Pos          5UL
#define GPIO_PRT_INTR_MASKED_EDGE5_Msk          0x20UL
#define GPIO_PRT_INTR_MASKED_EDGE6_Pos          6UL
#define GPIO_PRT_INTR_MASKED_EDGE6_Msk          0x40UL
#define GPIO_PRT_INTR_MASKED_EDGE7_Pos          7UL
#define GPIO_PRT_INTR_MASKED_EDGE7_Msk          0x80UL
#define GPIO_PRT_INTR_MASKED_FLT_EDGE_Pos       8UL
#define GPIO_PRT_INTR_MASKED_FLT_EDGE_Msk       0x100UL
/* GPIO_PRT.INTR_SET */
#define GPIO_PRT_INTR_SET_EDGE0_Pos             0UL
#define GPIO_PRT_INTR_SET_EDGE0_Msk             0x1UL
#define GPIO_PRT_INTR_SET_EDGE1_Pos             1UL
#define GPIO_PRT_INTR_SET_EDGE1_Msk             0x2UL
#define GPIO_PRT_INTR_SET_EDGE2_Pos             2UL
#define GPIO_PRT_INTR_SET_EDGE2_Msk             0x4UL
#define GPIO_PRT_INTR_SET_EDGE3_Pos             3UL
#define GPIO_PRT_INTR_SET_EDGE3_Msk             0x8UL
#define GPIO_PRT_INTR_SET_EDGE4_Pos             4UL
#define GPIO_PRT_INTR_SET_EDGE4_Msk             0x10UL
#define GPIO_PRT_INTR_SET_EDGE5_Pos             5UL
#define GPIO_PRT_INTR_SET_EDGE5_Msk             0x20UL
#define GPIO_PRT_INTR_SET_EDGE6_Pos             6UL
#define GPIO_PRT_INTR_SET_EDGE6_Msk             0x40UL
#define GPIO_PRT_INTR_SET_EDGE7_Pos             7UL
#define GPIO_PRT_INTR_SET_EDGE7_Msk             0x80UL
#define GPIO_PRT_INTR_SET_FLT_EDGE_Pos          8UL
#define GPIO_PRT_INTR_SET_FLT_EDGE_Msk          0x100UL
/* GPIO_PRT.INTR_CFG */
#define GPIO_PRT_INTR_CFG_EDGE0_SEL_Pos         0UL
#define GPIO_PRT_INTR_CFG_EDGE0_SEL_Msk         0x3UL
#define GPIO_PRT_INTR_CFG_EDGE1_SEL_Pos         2UL
#define GPIO_PRT_INTR_CFG_EDGE1_SEL_Msk         0xCUL
#define GPIO_PRT_INTR_CFG_EDGE2_SEL_Pos         4UL
#define GPIO_PRT_INTR_CFG_EDGE2_SEL_Msk         0x30UL
#define GPIO_PRT_INTR_CFG_EDGE3_SEL_Pos         6UL
#define GPIO_PRT_INTR_CFG_EDGE3_SEL_Msk         0xC0UL
#define GPIO_PRT_INTR_CFG_EDGE4_SEL_Pos         8UL
#define GPIO_PRT_INTR_CFG_EDGE4_SEL_Msk         0x300UL
#define GPIO_PRT_INTR_CFG_EDGE5_SEL_Pos         10UL
#define GPIO_PRT_INTR_CFG_EDGE5_SEL_Msk         0xC00UL
#define GPIO_PRT_INTR_CFG_EDGE6_SEL_Pos         12UL
#define GPIO_PRT_INTR_CFG_EDGE6_SEL_Msk         0x3000UL
#define GPIO_PRT_INTR_CFG_EDGE7_SEL_Pos         14UL
#define GPIO_PRT_INTR_CFG_EDGE7_SEL_Msk         0xC000UL
#define GPIO_PRT_INTR_CFG_FLT_EDGE_SEL_Pos      16UL
#define GPIO_PRT_INTR_CFG_FLT_EDGE_SEL_Msk      0x30000UL
#define GPIO_PRT_INTR_CFG_FLT_SEL_Pos           18UL
#define GPIO_PRT_INTR_CFG_FLT_SEL_Msk           0x1C0000UL
/* GPIO_PRT.CFG */
#define GPIO_PRT_CFG_DRIVE_MODE0_Pos            0UL
#define GPIO_PRT_CFG_DRIVE_MODE0_Msk            0x7UL
#define GPIO_PRT_CFG_IN_EN0_Pos                 3UL
#define GPIO_PRT_CFG_IN_EN0_Msk                 0x8UL
#define GPIO_PRT_CFG_DRIVE_MODE1_Pos            4UL
#define GPIO_PRT_CFG_DRIVE_MODE1_Msk            0x70UL
#define GPIO_PRT_CFG_IN_EN1_Pos                 7UL
#define GPIO_PRT_CFG_IN_EN1_Msk                 0x80UL
#define GPIO_PRT_CFG_DRIVE_MODE2_Pos            8UL
#define GPIO_PRT_CFG_DRIVE_MODE2_Msk            0x700UL
#define GPIO_PRT_CFG_IN_EN2_Pos                 11UL
#define GPIO_PRT_CFG_IN_EN2_Msk                 0x800UL
#define GPIO_PRT_CFG_DRIVE_MODE3_Pos            12UL
#define GPIO_PRT_CFG_DRIVE_MODE3_Msk            0x7000UL
#define GPIO_PRT_CFG_IN_EN3_Pos                 15UL
#define GPIO_PRT_CFG_IN_EN3_Msk                 0x8000UL
#define GPIO_PRT_CFG_DRIVE_MODE4_Pos            16UL
#define GPIO_PRT_CFG_DRIVE_MODE4_Msk            0x70000UL
#define GPIO_PRT_CFG_IN_EN4_Pos                 19UL
#define GPIO_PRT_CFG_IN_EN4_Msk                 0x80000UL
#define GPIO_PRT_CFG_DRIVE_MODE5_Pos            20UL
#define GPIO_PRT_CFG_DRIVE_MODE5_Msk            0x700000UL
#define GPIO_PRT_CFG_IN_EN5_Pos                 23UL
#define GPIO_PRT_CFG_IN_EN5_Msk                 0x800000UL
#define GPIO_PRT_CFG_DRIVE_MODE6_Pos            24UL
#define GPIO_PRT_CFG_DRIVE_MODE6_Msk            0x7000000UL
#define GPIO_PRT_CFG_IN_EN6_Pos                 27UL
#define GPIO_PRT_CFG_IN_EN6_Msk                 0x8000000UL
#define GPIO_PRT_CFG_DRIVE_MODE7_Pos            28UL
#define GPIO_PRT_CFG_DRIVE_MODE7_Msk            0x70000000UL
#define GPIO_PRT_CFG_IN_EN7_Pos                 31UL
#define GPIO_PRT_CFG_IN_EN7_Msk                 0x80000000UL
/* GPIO_PRT.CFG_IN */
#define GPIO_PRT_CFG_IN_VTRIP_SEL0_0_Pos        0UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL0_0_Msk        0x1UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL1_0_Pos        1UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL1_0_Msk        0x2UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL2_0_Pos        2UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL2_0_Msk        0x4UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL3_0_Pos        3UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL3_0_Msk        0x8UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL4_0_Pos        4UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL4_0_Msk        0x10UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL5_0_Pos        5UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL5_0_Msk        0x20UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL6_0_Pos        6UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL6_0_Msk        0x40UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL7_0_Pos        7UL
#define GPIO_PRT_CFG_IN_VTRIP_SEL7_0_Msk        0x80UL
/* GPIO_PRT.CFG_OUT */
#define GPIO_PRT_CFG_OUT_SLOW0_Pos              0UL
#define GPIO_PRT_CFG_OUT_SLOW0_Msk              0x1UL
#define GPIO_PRT_CFG_OUT_SLOW1_Pos              1UL
#define GPIO_PRT_CFG_OUT_SLOW1_Msk              0x2UL
#define GPIO_PRT_CFG_OUT_SLOW2_Pos              2UL
#define GPIO_PRT_CFG_OUT_SLOW2_Msk              0x4UL
#define GPIO_PRT_CFG_OUT_SLOW3_Pos              3UL
#define GPIO_PRT_CFG_OUT_SLOW3_Msk              0x8UL
#define GPIO_PRT_CFG_OUT_SLOW4_Pos              4UL
#define GPIO_PRT_CFG_OUT_SLOW4_Msk              0x10UL
#define GPIO_PRT_CFG_OUT_SLOW5_Pos              5UL
#define GPIO_PRT_CFG_OUT_SLOW5_Msk              0x20UL
#define GPIO_PRT_CFG_OUT_SLOW6_Pos              6UL
#define GPIO_PRT_CFG_OUT_SLOW6_Msk              0x40UL
#define GPIO_PRT_CFG_OUT_SLOW7_Pos              7UL
#define GPIO_PRT_CFG_OUT_SLOW7_Msk              0x80UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL0_Pos         16UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL0_Msk         0x30000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL1_Pos         18UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL1_Msk         0xC0000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL2_Pos         20UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL2_Msk         0x300000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL3_Pos         22UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL3_Msk         0xC00000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL4_Pos         24UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL4_Msk         0x3000000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL5_Pos         26UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL5_Msk         0xC000000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL6_Pos         28UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL6_Msk         0x30000000UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL7_Pos         30UL
#define GPIO_PRT_CFG_OUT_DRIVE_SEL7_Msk         0xC0000000UL
/* GPIO_PRT.CFG_SIO */
#define GPIO_PRT_CFG_SIO_VREG_EN01_Pos          0UL
#define GPIO_PRT_CFG_SIO_VREG_EN01_Msk          0x1UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL01_Pos         1UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL01_Msk         0x2UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL01_Pos        2UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL01_Msk        0x4UL
#define GPIO_PRT_CFG_SIO_VREF_SEL01_Pos         3UL
#define GPIO_PRT_CFG_SIO_VREF_SEL01_Msk         0x18UL
#define GPIO_PRT_CFG_SIO_VOH_SEL01_Pos          5UL
#define GPIO_PRT_CFG_SIO_VOH_SEL01_Msk          0xE0UL
#define GPIO_PRT_CFG_SIO_VREG_EN23_Pos          8UL
#define GPIO_PRT_CFG_SIO_VREG_EN23_Msk          0x100UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL23_Pos         9UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL23_Msk         0x200UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL23_Pos        10UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL23_Msk        0x400UL
#define GPIO_PRT_CFG_SIO_VREF_SEL23_Pos         11UL
#define GPIO_PRT_CFG_SIO_VREF_SEL23_Msk         0x1800UL
#define GPIO_PRT_CFG_SIO_VOH_SEL23_Pos          13UL
#define GPIO_PRT_CFG_SIO_VOH_SEL23_Msk          0xE000UL
#define GPIO_PRT_CFG_SIO_VREG_EN45_Pos          16UL
#define GPIO_PRT_CFG_SIO_VREG_EN45_Msk          0x10000UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL45_Pos         17UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL45_Msk         0x20000UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL45_Pos        18UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL45_Msk        0x40000UL
#define GPIO_PRT_CFG_SIO_VREF_SEL45_Pos         19UL
#define GPIO_PRT_CFG_SIO_VREF_SEL45_Msk         0x180000UL
#define GPIO_PRT_CFG_SIO_VOH_SEL45_Pos          21UL
#define GPIO_PRT_CFG_SIO_VOH_SEL45_Msk          0xE00000UL
#define GPIO_PRT_CFG_SIO_VREG_EN67_Pos          24UL
#define GPIO_PRT_CFG_SIO_VREG_EN67_Msk          0x1000000UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL67_Pos         25UL
#define GPIO_PRT_CFG_SIO_IBUF_SEL67_Msk         0x2000000UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL67_Pos        26UL
#define GPIO_PRT_CFG_SIO_VTRIP_SEL67_Msk        0x4000000UL
#define GPIO_PRT_CFG_SIO_VREF_SEL67_Pos         27UL
#define GPIO_PRT_CFG_SIO_VREF_SEL67_Msk         0x18000000UL
#define GPIO_PRT_CFG_SIO_VOH_SEL67_Pos          29UL
#define GPIO_PRT_CFG_SIO_VOH_SEL67_Msk          0xE0000000UL
/* GPIO_PRT.CFG_IN_AUTOLVL */
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL0_1_Pos 0UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL0_1_Msk 0x1UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL1_1_Pos 1UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL1_1_Msk 0x2UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL2_1_Pos 2UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL2_1_Msk 0x4UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL3_1_Pos 3UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL3_1_Msk 0x8UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL4_1_Pos 4UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL4_1_Msk 0x10UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL5_1_Pos 5UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL5_1_Msk 0x20UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL6_1_Pos 6UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL6_1_Msk 0x40UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL7_1_Pos 7UL
#define GPIO_PRT_CFG_IN_AUTOLVL_VTRIP_SEL7_1_Msk 0x80UL


/* GPIO.INTR_CAUSE0 */
#define GPIO_INTR_CAUSE0_PORT_INT_Pos           0UL
#define GPIO_INTR_CAUSE0_PORT_INT_Msk           0xFFFFFFFFUL
/* GPIO.INTR_CAUSE1 */
#define GPIO_INTR_CAUSE1_PORT_INT_Pos           0UL
#define GPIO_INTR_CAUSE1_PORT_INT_Msk           0xFFFFFFFFUL
/* GPIO.INTR_CAUSE2 */
#define GPIO_INTR_CAUSE2_PORT_INT_Pos           0UL
#define GPIO_INTR_CAUSE2_PORT_INT_Msk           0xFFFFFFFFUL
/* GPIO.INTR_CAUSE3 */
#define GPIO_INTR_CAUSE3_PORT_INT_Pos           0UL
#define GPIO_INTR_CAUSE3_PORT_INT_Msk           0xFFFFFFFFUL
/* GPIO.VDD_ACTIVE */
#define GPIO_VDD_ACTIVE_VDDIO_ACTIVE_Pos        0UL
#define GPIO_VDD_ACTIVE_VDDIO_ACTIVE_Msk        0xFFFFUL
#define GPIO_VDD_ACTIVE_VDDA_ACTIVE_Pos         30UL
#define GPIO_VDD_ACTIVE_VDDA_ACTIVE_Msk         0x40000000UL
#define GPIO_VDD_ACTIVE_VDDD_ACTIVE_Pos         31UL
#define GPIO_VDD_ACTIVE_VDDD_ACTIVE_Msk         0x80000000UL
/* GPIO.VDD_INTR */
#define GPIO_VDD_INTR_VDDIO_ACTIVE_Pos          0UL
#define GPIO_VDD_INTR_VDDIO_ACTIVE_Msk          0xFFFFUL
#define GPIO_VDD_INTR_VDDA_ACTIVE_Pos           30UL
#define GPIO_VDD_INTR_VDDA_ACTIVE_Msk           0x40000000UL
#define GPIO_VDD_INTR_VDDD_ACTIVE_Pos           31UL
#define GPIO_VDD_INTR_VDDD_ACTIVE_Msk           0x80000000UL
/* GPIO.VDD_INTR_MASK */
#define GPIO_VDD_INTR_MASK_VDDIO_ACTIVE_Pos     0UL
#define GPIO_VDD_INTR_MASK_VDDIO_ACTIVE_Msk     0xFFFFUL
#define GPIO_VDD_INTR_MASK_VDDA_ACTIVE_Pos      30UL
#define GPIO_VDD_INTR_MASK_VDDA_ACTIVE_Msk      0x40000000UL
#define GPIO_VDD_INTR_MASK_VDDD_ACTIVE_Pos      31UL
#define GPIO_VDD_INTR_MASK_VDDD_ACTIVE_Msk      0x80000000UL
/* GPIO.VDD_INTR_MASKED */
#define GPIO_VDD_INTR_MASKED_VDDIO_ACTIVE_Pos   0UL
#define GPIO_VDD_INTR_MASKED_VDDIO_ACTIVE_Msk   0xFFFFUL
#define GPIO_VDD_INTR_MASKED_VDDA_ACTIVE_Pos    30UL
#define GPIO_VDD_INTR_MASKED_VDDA_ACTIVE_Msk    0x40000000UL
#define GPIO_VDD_INTR_MASKED_VDDD_ACTIVE_Pos    31UL
#define GPIO_VDD_INTR_MASKED_VDDD_ACTIVE_Msk    0x80000000UL
/* GPIO.VDD_INTR_SET */
#define GPIO_VDD_INTR_SET_VDDIO_ACTIVE_Pos      0UL
#define GPIO_VDD_INTR_SET_VDDIO_ACTIVE_Msk      0xFFFFUL
#define GPIO_VDD_INTR_SET_VDDA_ACTIVE_Pos       30UL
#define GPIO_VDD_INTR_SET_VDDA_ACTIVE_Msk       0x40000000UL
#define GPIO_VDD_INTR_SET_VDDD_ACTIVE_Pos       31UL
#define GPIO_VDD_INTR_SET_VDDD_ACTIVE_Msk       0x80000000UL


#endif /* _CYIP_GPIO_V3_H_ */


/* [] END OF FILE */
