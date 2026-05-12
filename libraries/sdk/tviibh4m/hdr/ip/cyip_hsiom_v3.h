/***************************************************************************//**
* \file cyip_hsiom_v3.h
*
* \brief
* HSIOM IP definitions
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

#ifndef _CYIP_HSIOM_V3_H_
#define _CYIP_HSIOM_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    HSIOM
*******************************************************************************/

#define HSIOM_PRT_SECTION_SIZE                  0x00000010UL
#define HSIOM_SECTION_SIZE                      0x00004000UL

/**
  * \brief Port selection 0 (HSIOM_PRT_PORT_SEL0)
  */
typedef struct stc_HSIOM_PRT_PORT_SEL0_field {
  uint32_t                         u5IO0_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO1_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO2_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO3_SEL:5;
  uint32_t                         :3;
} stc_HSIOM_PRT_PORT_SEL0_field_t;

typedef union un_HSIOM_PRT_PORT_SEL0 {
  uint32_t                         u32Register;
  stc_HSIOM_PRT_PORT_SEL0_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_PRT_PORT_SEL0_t;

/**
  * \brief Port selection 1 (HSIOM_PRT_PORT_SEL1)
  */
typedef struct stc_HSIOM_PRT_PORT_SEL1_field {
  uint32_t                         u5IO4_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO5_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO6_SEL:5;
  uint32_t                         :3;
  uint32_t                         u5IO7_SEL:5;
  uint32_t                         :3;
} stc_HSIOM_PRT_PORT_SEL1_field_t;

typedef union un_HSIOM_PRT_PORT_SEL1 {
  uint32_t                         u32Register;
  stc_HSIOM_PRT_PORT_SEL1_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_PRT_PORT_SEL1_t;



/**
  * \brief AMUX splitter cell control (HSIOM_AMUX_SPLIT_CTL)
  */
typedef struct stc_HSIOM_AMUX_SPLIT_CTL_field {
  uint32_t                         u1SWITCH_AA_SL:1;
  uint32_t                         u1SWITCH_AA_SR:1;
  uint32_t                         u1SWITCH_AA_S0:1;
  uint32_t                         :1;
  uint32_t                         u1SWITCH_BB_SL:1;
  uint32_t                         u1SWITCH_BB_SR:1;
  uint32_t                         u1SWITCH_BB_S0:1;
  uint32_t                         :25;
} stc_HSIOM_AMUX_SPLIT_CTL_field_t;

typedef union un_HSIOM_AMUX_SPLIT_CTL {
  uint32_t                         u32Register;
  stc_HSIOM_AMUX_SPLIT_CTL_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_AMUX_SPLIT_CTL_t;

/**
  * \brief Power/Ground Monitor cell control 0 (HSIOM_MONITOR_CTL_0)
  */
typedef struct stc_HSIOM_MONITOR_CTL_0_field {
  uint32_t                         u32MONITOR_EN:32;
} stc_HSIOM_MONITOR_CTL_0_field_t;

typedef union un_HSIOM_MONITOR_CTL_0 {
  uint32_t                         u32Register;
  stc_HSIOM_MONITOR_CTL_0_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_MONITOR_CTL_0_t;

/**
  * \brief Power/Ground Monitor cell control 1 (HSIOM_MONITOR_CTL_1)
  */
typedef struct stc_HSIOM_MONITOR_CTL_1_field {
  uint32_t                         u32MONITOR_EN:32;
} stc_HSIOM_MONITOR_CTL_1_field_t;

typedef union un_HSIOM_MONITOR_CTL_1 {
  uint32_t                         u32Register;
  stc_HSIOM_MONITOR_CTL_1_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_MONITOR_CTL_1_t;

/**
  * \brief Power/Ground Monitor cell control 2 (HSIOM_MONITOR_CTL_2)
  */
typedef struct stc_HSIOM_MONITOR_CTL_2_field {
  uint32_t                         u32MONITOR_EN:32;
} stc_HSIOM_MONITOR_CTL_2_field_t;

typedef union un_HSIOM_MONITOR_CTL_2 {
  uint32_t                         u32Register;
  stc_HSIOM_MONITOR_CTL_2_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_MONITOR_CTL_2_t;

/**
  * \brief Power/Ground Monitor cell control 3 (HSIOM_MONITOR_CTL_3)
  */
typedef struct stc_HSIOM_MONITOR_CTL_3_field {
  uint32_t                         u32MONITOR_EN:32;
} stc_HSIOM_MONITOR_CTL_3_field_t;

typedef union un_HSIOM_MONITOR_CTL_3 {
  uint32_t                         u32Register;
  stc_HSIOM_MONITOR_CTL_3_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_MONITOR_CTL_3_t;

/**
  * \brief Alternate JTAG IF selection register (HSIOM_ALT_JTAG_EN)
  */
typedef struct stc_HSIOM_ALT_JTAG_EN_field {
  uint32_t                         :31;
  uint32_t                         u1ENABLE:1;
} stc_HSIOM_ALT_JTAG_EN_field_t;

typedef union un_HSIOM_ALT_JTAG_EN {
  uint32_t                         u32Register;
  stc_HSIOM_ALT_JTAG_EN_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_HSIOM_ALT_JTAG_EN_t;



/**
  * \brief HSIOM port registers (HSIOM_PRT)
  */
typedef struct stc_HSIOM_PRT {
  un_HSIOM_PRT_PORT_SEL0_t         unPORT_SEL0;        /*!< 0x00000000 Port selection 0 */
  un_HSIOM_PRT_PORT_SEL1_t         unPORT_SEL1;        /*!< 0x00000004 Port selection 1 */
  uint32_t                         au32Reserved[2];
} stc_HSIOM_PRT_t;                                     /*!< Size = 16 (0x10) */

/**
  * \brief High Speed IO Matrix (HSIOM) (HSIOM)
  */
typedef struct stc_HSIOM {
  stc_HSIOM_PRT_t                  PRT[128];           /*!< 0x00000000 HSIOM port registers */
  uint32_t                         au32Reserved[1536];
  un_HSIOM_AMUX_SPLIT_CTL_t        unAMUX_SPLIT_CTL[64]; /*!< 0x00002000 AMUX splitter cell control */
  uint32_t                         au32Reserved1[64];
  un_HSIOM_MONITOR_CTL_0_t         unMONITOR_CTL_0;    /*!< 0x00002200 Power/Ground Monitor cell control 0 */
  un_HSIOM_MONITOR_CTL_1_t         unMONITOR_CTL_1;    /*!< 0x00002204 Power/Ground Monitor cell control 1 */
  un_HSIOM_MONITOR_CTL_2_t         unMONITOR_CTL_2;    /*!< 0x00002208 Power/Ground Monitor cell control 2 */
  un_HSIOM_MONITOR_CTL_3_t         unMONITOR_CTL_3;    /*!< 0x0000220C Power/Ground Monitor cell control 3 */
  uint32_t                         au32Reserved2[12];
  un_HSIOM_ALT_JTAG_EN_t           unALT_JTAG_EN;      /*!< 0x00002240 Alternate JTAG IF selection register */
} stc_HSIOM_t;                                         /*!< Size = 8772 (0x2244) */


/* HSIOM_PRT.PORT_SEL0 */
#define HSIOM_PRT_PORT_SEL0_IO0_SEL_Pos         0UL
#define HSIOM_PRT_PORT_SEL0_IO0_SEL_Msk         0x1FUL
#define HSIOM_PRT_PORT_SEL0_IO1_SEL_Pos         8UL
#define HSIOM_PRT_PORT_SEL0_IO1_SEL_Msk         0x1F00UL
#define HSIOM_PRT_PORT_SEL0_IO2_SEL_Pos         16UL
#define HSIOM_PRT_PORT_SEL0_IO2_SEL_Msk         0x1F0000UL
#define HSIOM_PRT_PORT_SEL0_IO3_SEL_Pos         24UL
#define HSIOM_PRT_PORT_SEL0_IO3_SEL_Msk         0x1F000000UL
/* HSIOM_PRT.PORT_SEL1 */
#define HSIOM_PRT_PORT_SEL1_IO4_SEL_Pos         0UL
#define HSIOM_PRT_PORT_SEL1_IO4_SEL_Msk         0x1FUL
#define HSIOM_PRT_PORT_SEL1_IO5_SEL_Pos         8UL
#define HSIOM_PRT_PORT_SEL1_IO5_SEL_Msk         0x1F00UL
#define HSIOM_PRT_PORT_SEL1_IO6_SEL_Pos         16UL
#define HSIOM_PRT_PORT_SEL1_IO6_SEL_Msk         0x1F0000UL
#define HSIOM_PRT_PORT_SEL1_IO7_SEL_Pos         24UL
#define HSIOM_PRT_PORT_SEL1_IO7_SEL_Msk         0x1F000000UL


/* HSIOM.AMUX_SPLIT_CTL */
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_SL_Pos   0UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_SL_Msk   0x1UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_SR_Pos   1UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_SR_Msk   0x2UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_S0_Pos   2UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_AA_S0_Msk   0x4UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_SL_Pos   4UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_SL_Msk   0x10UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_SR_Pos   5UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_SR_Msk   0x20UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_S0_Pos   6UL
#define HSIOM_AMUX_SPLIT_CTL_SWITCH_BB_S0_Msk   0x40UL
/* HSIOM.MONITOR_CTL_0 */
#define HSIOM_MONITOR_CTL_0_MONITOR_EN_Pos      0UL
#define HSIOM_MONITOR_CTL_0_MONITOR_EN_Msk      0xFFFFFFFFUL
/* HSIOM.MONITOR_CTL_1 */
#define HSIOM_MONITOR_CTL_1_MONITOR_EN_Pos      0UL
#define HSIOM_MONITOR_CTL_1_MONITOR_EN_Msk      0xFFFFFFFFUL
/* HSIOM.MONITOR_CTL_2 */
#define HSIOM_MONITOR_CTL_2_MONITOR_EN_Pos      0UL
#define HSIOM_MONITOR_CTL_2_MONITOR_EN_Msk      0xFFFFFFFFUL
/* HSIOM.MONITOR_CTL_3 */
#define HSIOM_MONITOR_CTL_3_MONITOR_EN_Pos      0UL
#define HSIOM_MONITOR_CTL_3_MONITOR_EN_Msk      0xFFFFFFFFUL
/* HSIOM.ALT_JTAG_EN */
#define HSIOM_ALT_JTAG_EN_ENABLE_Pos            31UL
#define HSIOM_ALT_JTAG_EN_ENABLE_Msk            0x80000000UL


#endif /* _CYIP_HSIOM_V3_H_ */


/* [] END OF FILE */
