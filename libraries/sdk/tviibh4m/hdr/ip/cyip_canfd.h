/***************************************************************************//**
* \file cyip_canfd.h
*
* \brief
* CANFD IP definitions
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

#ifndef _CYIP_CANFD_H_
#define _CYIP_CANFD_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    CANFD
*******************************************************************************/

#define CANFD_CH_M_TTCAN_SECTION_SIZE           0x00000180UL
#define CANFD_CH_SECTION_SIZE                   0x00000200UL
#define CANFD_SECTION_SIZE                      0x00020000UL

/**
  * \brief Core Release Register (CANFD_CH_CREL)
  */
typedef struct stc_CANFD_CH_CREL_field {
  uint32_t                         u8DAY:8;
  uint32_t                         u8MON:8;
  uint32_t                         u4YEAR:4;
  uint32_t                         u4SUBSTEP:4;
  uint32_t                         u4STEP:4;
  uint32_t                         u4REL:4;
} stc_CANFD_CH_CREL_field_t;

typedef union un_CANFD_CH_CREL {
  uint32_t                         u32Register;
  stc_CANFD_CH_CREL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_CREL_t;

/**
  * \brief Endian Register (CANFD_CH_ENDN)
  */
typedef struct stc_CANFD_CH_ENDN_field {
  uint32_t                         u32ETV:32;
} stc_CANFD_CH_ENDN_field_t;

typedef union un_CANFD_CH_ENDN {
  uint32_t                         u32Register;
  stc_CANFD_CH_ENDN_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_ENDN_t;

/**
  * \brief Data Bit Timing & Prescaler Register (CANFD_CH_DBTP)
  */
typedef struct stc_CANFD_CH_DBTP_field {
  uint32_t                         u4DSJW:4;
  uint32_t                         u4DTSEG2:4;
  uint32_t                         u5DTSEG1:5;
  uint32_t                         :3;
  uint32_t                         u5DBRP:5;
  uint32_t                         :2;
  uint32_t                         u1TDC:1;
  uint32_t                         :8;
} stc_CANFD_CH_DBTP_field_t;

typedef union un_CANFD_CH_DBTP {
  uint32_t                         u32Register;
  stc_CANFD_CH_DBTP_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_DBTP_t;

/**
  * \brief Test Register (CANFD_CH_TEST)
  */
typedef struct stc_CANFD_CH_TEST_field {
  uint32_t                         u1TAM:1;
  uint32_t                         u1TAT:1;
  uint32_t                         u1CAM:1;
  uint32_t                         u1CAT:1;
  uint32_t                         u1LBCK:1;
  uint32_t                         u2TX:2;
  uint32_t                         u1RX:1;
  uint32_t                         :24;
} stc_CANFD_CH_TEST_field_t;

typedef union un_CANFD_CH_TEST {
  uint32_t                         u32Register;
  stc_CANFD_CH_TEST_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TEST_t;

/**
  * \brief RAM Watchdog (CANFD_CH_RWD)
  */
typedef struct stc_CANFD_CH_RWD_field {
  uint32_t                         u8WDC:8;
  uint32_t                         u8WDV:8;
  uint32_t                         :16;
} stc_CANFD_CH_RWD_field_t;

typedef union un_CANFD_CH_RWD {
  uint32_t                         u32Register;
  stc_CANFD_CH_RWD_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RWD_t;

/**
  * \brief CC Control Register (CANFD_CH_CCCR)
  */
typedef struct stc_CANFD_CH_CCCR_field {
  uint32_t                         u1INIT:1;
  uint32_t                         u1CCE:1;
  uint32_t                         u1ASM:1;
  uint32_t                         u1CSA:1;
  uint32_t                         u1CSR:1;
  uint32_t                         u1MON_:1;
  uint32_t                         u1DAR:1;
  uint32_t                         u1TEST:1;
  uint32_t                         u1FDOE:1;
  uint32_t                         u1BRSE:1;
  uint32_t                         :2;
  uint32_t                         u1PXHD:1;
  uint32_t                         u1EFBI:1;
  uint32_t                         u1TXP:1;
  uint32_t                         u1NISO:1;
  uint32_t                         :16;
} stc_CANFD_CH_CCCR_field_t;

typedef union un_CANFD_CH_CCCR {
  uint32_t                         u32Register;
  stc_CANFD_CH_CCCR_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_CCCR_t;

/**
  * \brief Nominal Bit Timing & Prescaler Register (CANFD_CH_NBTP)
  */
typedef struct stc_CANFD_CH_NBTP_field {
  uint32_t                         u7NTSEG2:7;
  uint32_t                         :1;
  uint32_t                         u8NTSEG1:8;
  uint32_t                         u9NBRP:9;
  uint32_t                         u7NSJW:7;
} stc_CANFD_CH_NBTP_field_t;

typedef union un_CANFD_CH_NBTP {
  uint32_t                         u32Register;
  stc_CANFD_CH_NBTP_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_NBTP_t;

/**
  * \brief Timestamp Counter Configuration (CANFD_CH_TSCC)
  */
typedef struct stc_CANFD_CH_TSCC_field {
  uint32_t                         u2TSS:2;
  uint32_t                         :14;
  uint32_t                         u4TCP:4;
  uint32_t                         :12;
} stc_CANFD_CH_TSCC_field_t;

typedef union un_CANFD_CH_TSCC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TSCC_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TSCC_t;

/**
  * \brief Timestamp Counter Value (CANFD_CH_TSCV)
  */
typedef struct stc_CANFD_CH_TSCV_field {
  uint32_t                         u16TSC:16;
  uint32_t                         :16;
} stc_CANFD_CH_TSCV_field_t;

typedef union un_CANFD_CH_TSCV {
  uint32_t                         u32Register;
  stc_CANFD_CH_TSCV_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TSCV_t;

/**
  * \brief Timeout Counter Configuration (CANFD_CH_TOCC)
  */
typedef struct stc_CANFD_CH_TOCC_field {
  uint32_t                         u1ETOC:1;
  uint32_t                         u2TOS:2;
  uint32_t                         :13;
  uint32_t                         u16TOP:16;
} stc_CANFD_CH_TOCC_field_t;

typedef union un_CANFD_CH_TOCC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TOCC_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TOCC_t;

/**
  * \brief Timeout Counter Value (CANFD_CH_TOCV)
  */
typedef struct stc_CANFD_CH_TOCV_field {
  uint32_t                         u16TOC:16;
  uint32_t                         :16;
} stc_CANFD_CH_TOCV_field_t;

typedef union un_CANFD_CH_TOCV {
  uint32_t                         u32Register;
  stc_CANFD_CH_TOCV_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TOCV_t;

/**
  * \brief Error Counter Register (CANFD_CH_ECR)
  */
typedef struct stc_CANFD_CH_ECR_field {
  uint32_t                         u8TEC:8;
  uint32_t                         u7REC:7;
  uint32_t                         u1RP:1;
  uint32_t                         u8CEL:8;
  uint32_t                         :8;
} stc_CANFD_CH_ECR_field_t;

typedef union un_CANFD_CH_ECR {
  uint32_t                         u32Register;
  stc_CANFD_CH_ECR_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_ECR_t;

/**
  * \brief Protocol Status Register (CANFD_CH_PSR)
  */
typedef struct stc_CANFD_CH_PSR_field {
  uint32_t                         u3LEC:3;
  uint32_t                         u2ACT:2;
  uint32_t                         u1EP:1;
  uint32_t                         u1EW:1;
  uint32_t                         u1BO:1;
  uint32_t                         u3DLEC:3;
  uint32_t                         u1RESI:1;
  uint32_t                         u1RBRS:1;
  uint32_t                         u1RFDF:1;
  uint32_t                         u1PXE:1;
  uint32_t                         :1;
  uint32_t                         u7TDCV:7;
  uint32_t                         :9;
} stc_CANFD_CH_PSR_field_t;

typedef union un_CANFD_CH_PSR {
  uint32_t                         u32Register;
  stc_CANFD_CH_PSR_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_PSR_t;

/**
  * \brief Transmitter Delay Compensation Register (CANFD_CH_TDCR)
  */
typedef struct stc_CANFD_CH_TDCR_field {
  uint32_t                         u7TDCF:7;
  uint32_t                         :1;
  uint32_t                         u7TDCO:7;
  uint32_t                         :17;
} stc_CANFD_CH_TDCR_field_t;

typedef union un_CANFD_CH_TDCR {
  uint32_t                         u32Register;
  stc_CANFD_CH_TDCR_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TDCR_t;

/**
  * \brief Interrupt Register (CANFD_CH_IR)
  */
typedef struct stc_CANFD_CH_IR_field {
  uint32_t                         u1RF0N:1;
  uint32_t                         u1RF0W:1;
  uint32_t                         u1RF0F:1;
  uint32_t                         u1RF0L_:1;
  uint32_t                         u1RF1N:1;
  uint32_t                         u1RF1W:1;
  uint32_t                         u1RF1F:1;
  uint32_t                         u1RF1L_:1;
  uint32_t                         u1HPM:1;
  uint32_t                         u1TC:1;
  uint32_t                         u1TCF:1;
  uint32_t                         u1TFE:1;
  uint32_t                         u1TEFN:1;
  uint32_t                         u1TEFW:1;
  uint32_t                         u1TEFF:1;
  uint32_t                         u1TEFL_:1;
  uint32_t                         u1TSW:1;
  uint32_t                         u1MRAF:1;
  uint32_t                         u1TOO:1;
  uint32_t                         u1DRX:1;
  uint32_t                         u1BEC:1;
  uint32_t                         u1BEU:1;
  uint32_t                         u1ELO:1;
  uint32_t                         u1EP_:1;
  uint32_t                         u1EW_:1;
  uint32_t                         u1BO_:1;
  uint32_t                         u1WDI:1;
  uint32_t                         u1PEA:1;
  uint32_t                         u1PED:1;
  uint32_t                         u1ARA:1;
  uint32_t                         :2;
} stc_CANFD_CH_IR_field_t;

typedef union un_CANFD_CH_IR {
  uint32_t                         u32Register;
  stc_CANFD_CH_IR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_IR_t;

/**
  * \brief Interrupt Enable (CANFD_CH_IE)
  */
typedef struct stc_CANFD_CH_IE_field {
  uint32_t                         u1RF0NE:1;
  uint32_t                         u1RF0WE:1;
  uint32_t                         u1RF0FE:1;
  uint32_t                         u1RF0LE:1;
  uint32_t                         u1RF1NE:1;
  uint32_t                         u1RF1WE:1;
  uint32_t                         u1RF1FE:1;
  uint32_t                         u1RF1LE:1;
  uint32_t                         u1HPME:1;
  uint32_t                         u1TCE:1;
  uint32_t                         u1TCFE:1;
  uint32_t                         u1TFEE:1;
  uint32_t                         u1TEFNE:1;
  uint32_t                         u1TEFWE:1;
  uint32_t                         u1TEFFE:1;
  uint32_t                         u1TEFLE:1;
  uint32_t                         u1TSWE:1;
  uint32_t                         u1MRAFE:1;
  uint32_t                         u1TOOE:1;
  uint32_t                         u1DRXE:1;
  uint32_t                         u1BECE:1;
  uint32_t                         u1BEUE:1;
  uint32_t                         u1ELOE:1;
  uint32_t                         u1EPE:1;
  uint32_t                         u1EWE:1;
  uint32_t                         u1BOE:1;
  uint32_t                         u1WDIE:1;
  uint32_t                         u1PEAE:1;
  uint32_t                         u1PEDE:1;
  uint32_t                         u1ARAE:1;
  uint32_t                         :2;
} stc_CANFD_CH_IE_field_t;

typedef union un_CANFD_CH_IE {
  uint32_t                         u32Register;
  stc_CANFD_CH_IE_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_IE_t;

/**
  * \brief Interrupt Line Select (CANFD_CH_ILS)
  */
typedef struct stc_CANFD_CH_ILS_field {
  uint32_t                         u1RF0NL:1;
  uint32_t                         u1RF0WL:1;
  uint32_t                         u1RF0FL:1;
  uint32_t                         u1RF0LL:1;
  uint32_t                         u1RF1NL:1;
  uint32_t                         u1RF1WL:1;
  uint32_t                         u1RF1FL:1;
  uint32_t                         u1RF1LL:1;
  uint32_t                         u1HPML:1;
  uint32_t                         u1TCL:1;
  uint32_t                         u1TCFL:1;
  uint32_t                         u1TFEL:1;
  uint32_t                         u1TEFNL:1;
  uint32_t                         u1TEFWL:1;
  uint32_t                         u1TEFFL:1;
  uint32_t                         u1TEFLL:1;
  uint32_t                         u1TSWL:1;
  uint32_t                         u1MRAFL:1;
  uint32_t                         u1TOOL:1;
  uint32_t                         u1DRXL:1;
  uint32_t                         u1BECL:1;
  uint32_t                         u1BEUL:1;
  uint32_t                         u1ELOL:1;
  uint32_t                         u1EPL:1;
  uint32_t                         u1EWL:1;
  uint32_t                         u1BOL:1;
  uint32_t                         u1WDIL:1;
  uint32_t                         u1PEAL:1;
  uint32_t                         u1PEDL:1;
  uint32_t                         u1ARAL:1;
  uint32_t                         :2;
} stc_CANFD_CH_ILS_field_t;

typedef union un_CANFD_CH_ILS {
  uint32_t                         u32Register;
  stc_CANFD_CH_ILS_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_ILS_t;

/**
  * \brief Interrupt Line Enable (CANFD_CH_ILE)
  */
typedef struct stc_CANFD_CH_ILE_field {
  uint32_t                         u1EINT0:1;
  uint32_t                         u1EINT1:1;
  uint32_t                         :30;
} stc_CANFD_CH_ILE_field_t;

typedef union un_CANFD_CH_ILE {
  uint32_t                         u32Register;
  stc_CANFD_CH_ILE_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_ILE_t;

/**
  * \brief Global Filter Configuration (CANFD_CH_GFC)
  */
typedef struct stc_CANFD_CH_GFC_field {
  uint32_t                         u1RRFE:1;
  uint32_t                         u1RRFS:1;
  uint32_t                         u2ANFE:2;
  uint32_t                         u2ANFS:2;
  uint32_t                         :26;
} stc_CANFD_CH_GFC_field_t;

typedef union un_CANFD_CH_GFC {
  uint32_t                         u32Register;
  stc_CANFD_CH_GFC_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_GFC_t;

/**
  * \brief Standard ID Filter Configuration (CANFD_CH_SIDFC)
  */
typedef struct stc_CANFD_CH_SIDFC_field {
  uint32_t                         :2;
  uint32_t                         u14FLSSA:14;
  uint32_t                         u8LSS:8;
  uint32_t                         :8;
} stc_CANFD_CH_SIDFC_field_t;

typedef union un_CANFD_CH_SIDFC {
  uint32_t                         u32Register;
  stc_CANFD_CH_SIDFC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_SIDFC_t;

/**
  * \brief Extended ID Filter Configuration (CANFD_CH_XIDFC)
  */
typedef struct stc_CANFD_CH_XIDFC_field {
  uint32_t                         :2;
  uint32_t                         u14FLESA:14;
  uint32_t                         u7LSE:7;
  uint32_t                         :9;
} stc_CANFD_CH_XIDFC_field_t;

typedef union un_CANFD_CH_XIDFC {
  uint32_t                         u32Register;
  stc_CANFD_CH_XIDFC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_XIDFC_t;

/**
  * \brief Extended ID AND Mask (CANFD_CH_XIDAM)
  */
typedef struct stc_CANFD_CH_XIDAM_field {
  uint32_t                         u29EIDM:29;
  uint32_t                         :3;
} stc_CANFD_CH_XIDAM_field_t;

typedef union un_CANFD_CH_XIDAM {
  uint32_t                         u32Register;
  stc_CANFD_CH_XIDAM_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_XIDAM_t;

/**
  * \brief High Priority Message Status (CANFD_CH_HPMS)
  */
typedef struct stc_CANFD_CH_HPMS_field {
  uint32_t                         u6BIDX:6;
  uint32_t                         u2MSI:2;
  uint32_t                         u7FIDX:7;
  uint32_t                         u1FLST:1;
  uint32_t                         :16;
} stc_CANFD_CH_HPMS_field_t;

typedef union un_CANFD_CH_HPMS {
  uint32_t                         u32Register;
  stc_CANFD_CH_HPMS_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_HPMS_t;

/**
  * \brief New Data 1 (CANFD_CH_NDAT1)
  */
typedef struct stc_CANFD_CH_NDAT1_field {
  uint32_t                         u32ND:32;
} stc_CANFD_CH_NDAT1_field_t;

typedef union un_CANFD_CH_NDAT1 {
  uint32_t                         u32Register;
  stc_CANFD_CH_NDAT1_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_NDAT1_t;

/**
  * \brief New Data 2 (CANFD_CH_NDAT2)
  */
typedef struct stc_CANFD_CH_NDAT2_field {
  uint32_t                         u32ND:32;
} stc_CANFD_CH_NDAT2_field_t;

typedef union un_CANFD_CH_NDAT2 {
  uint32_t                         u32Register;
  stc_CANFD_CH_NDAT2_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_NDAT2_t;

/**
  * \brief Rx FIFO 0 Configuration (CANFD_CH_RXF0C)
  */
typedef struct stc_CANFD_CH_RXF0C_field {
  uint32_t                         :2;
  uint32_t                         u14F0SA:14;
  uint32_t                         u7F0S:7;
  uint32_t                         :1;
  uint32_t                         u7F0WM:7;
  uint32_t                         u1F0OM:1;
} stc_CANFD_CH_RXF0C_field_t;

typedef union un_CANFD_CH_RXF0C {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF0C_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF0C_t;

/**
  * \brief Rx FIFO 0 Status (CANFD_CH_RXF0S)
  */
typedef struct stc_CANFD_CH_RXF0S_field {
  uint32_t                         u7F0FL:7;
  uint32_t                         :1;
  uint32_t                         u6F0GI:6;
  uint32_t                         :2;
  uint32_t                         u6F0PI:6;
  uint32_t                         :2;
  uint32_t                         u1F0F:1;
  uint32_t                         u1RF0L:1;
  uint32_t                         :6;
} stc_CANFD_CH_RXF0S_field_t;

typedef union un_CANFD_CH_RXF0S {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF0S_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF0S_t;

/**
  * \brief Rx FIFO 0 Acknowledge (CANFD_CH_RXF0A)
  */
typedef struct stc_CANFD_CH_RXF0A_field {
  uint32_t                         u6F0AI:6;
  uint32_t                         :26;
} stc_CANFD_CH_RXF0A_field_t;

typedef union un_CANFD_CH_RXF0A {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF0A_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF0A_t;

/**
  * \brief Rx Buffer Configuration (CANFD_CH_RXBC)
  */
typedef struct stc_CANFD_CH_RXBC_field {
  uint32_t                         :2;
  uint32_t                         u14RBSA:14;
  uint32_t                         :16;
} stc_CANFD_CH_RXBC_field_t;

typedef union un_CANFD_CH_RXBC {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXBC_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXBC_t;

/**
  * \brief Rx FIFO 1 Configuration (CANFD_CH_RXF1C)
  */
typedef struct stc_CANFD_CH_RXF1C_field {
  uint32_t                         :2;
  uint32_t                         u14F1SA:14;
  uint32_t                         u7F1S:7;
  uint32_t                         :1;
  uint32_t                         u7F1WM:7;
  uint32_t                         u1F1OM:1;
} stc_CANFD_CH_RXF1C_field_t;

typedef union un_CANFD_CH_RXF1C {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF1C_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF1C_t;

/**
  * \brief Rx FIFO 1 Status (CANFD_CH_RXF1S)
  */
typedef struct stc_CANFD_CH_RXF1S_field {
  uint32_t                         u7F1FL:7;
  uint32_t                         :1;
  uint32_t                         u6F1GI:6;
  uint32_t                         :2;
  uint32_t                         u6F1PI:6;
  uint32_t                         :2;
  uint32_t                         u1F1F:1;
  uint32_t                         u1RF1L:1;
  uint32_t                         :4;
  uint32_t                         u2DMS:2;
} stc_CANFD_CH_RXF1S_field_t;

typedef union un_CANFD_CH_RXF1S {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF1S_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF1S_t;

/**
  * \brief Rx FIFO 1 Acknowledge (CANFD_CH_RXF1A)
  */
typedef struct stc_CANFD_CH_RXF1A_field {
  uint32_t                         u6F1AI:6;
  uint32_t                         :26;
} stc_CANFD_CH_RXF1A_field_t;

typedef union un_CANFD_CH_RXF1A {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXF1A_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXF1A_t;

/**
  * \brief Rx Buffer / FIFO Element Size Configuration (CANFD_CH_RXESC)
  */
typedef struct stc_CANFD_CH_RXESC_field {
  uint32_t                         u3F0DS:3;
  uint32_t                         :1;
  uint32_t                         u3F1DS:3;
  uint32_t                         :1;
  uint32_t                         u3RBDS:3;
  uint32_t                         :21;
} stc_CANFD_CH_RXESC_field_t;

typedef union un_CANFD_CH_RXESC {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXESC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXESC_t;

/**
  * \brief Tx Buffer Configuration (CANFD_CH_TXBC)
  */
typedef struct stc_CANFD_CH_TXBC_field {
  uint32_t                         :2;
  uint32_t                         u14TBSA:14;
  uint32_t                         u6NDTB:6;
  uint32_t                         :2;
  uint32_t                         u6TFQS:6;
  uint32_t                         u1TFQM:1;
} stc_CANFD_CH_TXBC_field_t;

typedef union un_CANFD_CH_TXBC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBC_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBC_t;

/**
  * \brief Tx FIFO/Queue Status (CANFD_CH_TXFQS)
  */
typedef struct stc_CANFD_CH_TXFQS_field {
  uint32_t                         u6TFFL:6;
  uint32_t                         :2;
  uint32_t                         u5TFGI:5;
  uint32_t                         :3;
  uint32_t                         u5TFQPI:5;
  uint32_t                         u1TFQF:1;
  uint32_t                         :10;
} stc_CANFD_CH_TXFQS_field_t;

typedef union un_CANFD_CH_TXFQS {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXFQS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXFQS_t;

/**
  * \brief Tx Buffer Element Size Configuration (CANFD_CH_TXESC)
  */
typedef struct stc_CANFD_CH_TXESC_field {
  uint32_t                         u3TBDS:3;
  uint32_t                         :29;
} stc_CANFD_CH_TXESC_field_t;

typedef union un_CANFD_CH_TXESC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXESC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXESC_t;

/**
  * \brief Tx Buffer Request Pending (CANFD_CH_TXBRP)
  */
typedef struct stc_CANFD_CH_TXBRP_field {
  uint32_t                         u32TRP:32;
} stc_CANFD_CH_TXBRP_field_t;

typedef union un_CANFD_CH_TXBRP {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBRP_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBRP_t;

/**
  * \brief Tx Buffer Add Request (CANFD_CH_TXBAR)
  */
typedef struct stc_CANFD_CH_TXBAR_field {
  uint32_t                         u32AR:32;
} stc_CANFD_CH_TXBAR_field_t;

typedef union un_CANFD_CH_TXBAR {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBAR_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBAR_t;

/**
  * \brief Tx Buffer Cancellation Request (CANFD_CH_TXBCR)
  */
typedef struct stc_CANFD_CH_TXBCR_field {
  uint32_t                         u32CR:32;
} stc_CANFD_CH_TXBCR_field_t;

typedef union un_CANFD_CH_TXBCR {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBCR_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBCR_t;

/**
  * \brief Tx Buffer Transmission Occurred (CANFD_CH_TXBTO)
  */
typedef struct stc_CANFD_CH_TXBTO_field {
  uint32_t                         u32TO:32;
} stc_CANFD_CH_TXBTO_field_t;

typedef union un_CANFD_CH_TXBTO {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBTO_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBTO_t;

/**
  * \brief Tx Buffer Cancellation Finished (CANFD_CH_TXBCF)
  */
typedef struct stc_CANFD_CH_TXBCF_field {
  uint32_t                         u32CF:32;
} stc_CANFD_CH_TXBCF_field_t;

typedef union un_CANFD_CH_TXBCF {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBCF_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBCF_t;

/**
  * \brief Tx Buffer Transmission Interrupt Enable (CANFD_CH_TXBTIE)
  */
typedef struct stc_CANFD_CH_TXBTIE_field {
  uint32_t                         u32TIE:32;
} stc_CANFD_CH_TXBTIE_field_t;

typedef union un_CANFD_CH_TXBTIE {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBTIE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBTIE_t;

/**
  * \brief Tx Buffer Cancellation Finished Interrupt Enable (CANFD_CH_TXBCIE)
  */
typedef struct stc_CANFD_CH_TXBCIE_field {
  uint32_t                         u32CFIE:32;
} stc_CANFD_CH_TXBCIE_field_t;

typedef union un_CANFD_CH_TXBCIE {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXBCIE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXBCIE_t;

/**
  * \brief Tx Event FIFO Configuration (CANFD_CH_TXEFC)
  */
typedef struct stc_CANFD_CH_TXEFC_field {
  uint32_t                         :2;
  uint32_t                         u14EFSA:14;
  uint32_t                         u6EFS:6;
  uint32_t                         :2;
  uint32_t                         u6EFWM:6;
  uint32_t                         :2;
} stc_CANFD_CH_TXEFC_field_t;

typedef union un_CANFD_CH_TXEFC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXEFC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXEFC_t;

/**
  * \brief Tx Event FIFO Status (CANFD_CH_TXEFS)
  */
typedef struct stc_CANFD_CH_TXEFS_field {
  uint32_t                         u6EFFL:6;
  uint32_t                         :2;
  uint32_t                         u5EFGI:5;
  uint32_t                         :3;
  uint32_t                         u5EFPI:5;
  uint32_t                         :3;
  uint32_t                         u1EFF:1;
  uint32_t                         u1TEFL:1;
  uint32_t                         :6;
} stc_CANFD_CH_TXEFS_field_t;

typedef union un_CANFD_CH_TXEFS {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXEFS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXEFS_t;

/**
  * \brief Tx Event FIFO Acknowledge (CANFD_CH_TXEFA)
  */
typedef struct stc_CANFD_CH_TXEFA_field {
  uint32_t                         u5EFAI:5;
  uint32_t                         :27;
} stc_CANFD_CH_TXEFA_field_t;

typedef union un_CANFD_CH_TXEFA {
  uint32_t                         u32Register;
  stc_CANFD_CH_TXEFA_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TXEFA_t;

/**
  * \brief TT Trigger Memory Configuration (CANFD_CH_TTTMC)
  */
typedef struct stc_CANFD_CH_TTTMC_field {
  uint32_t                         :2;
  uint32_t                         u14TMSA:14;
  uint32_t                         u7TME:7;
  uint32_t                         :9;
} stc_CANFD_CH_TTTMC_field_t;

typedef union un_CANFD_CH_TTTMC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTTMC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTTMC_t;

/**
  * \brief TT Reference Message Configuration (CANFD_CH_TTRMC)
  */
typedef struct stc_CANFD_CH_TTRMC_field {
  uint32_t                         u29RID:29;
  uint32_t                         :1;
  uint32_t                         u1XTD:1;
  uint32_t                         u1RMPS:1;
} stc_CANFD_CH_TTRMC_field_t;

typedef union un_CANFD_CH_TTRMC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTRMC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTRMC_t;

/**
  * \brief TT Operation Configuration (CANFD_CH_TTOCF)
  */
typedef struct stc_CANFD_CH_TTOCF_field {
  uint32_t                         u2OM:2;
  uint32_t                         :1;
  uint32_t                         u1GEN:1;
  uint32_t                         u1TM:1;
  uint32_t                         u3LDSDL:3;
  uint32_t                         u7IRTO:7;
  uint32_t                         u1EECS:1;
  uint32_t                         u8AWL:8;
  uint32_t                         u1EGTF:1;
  uint32_t                         u1ECC:1;
  uint32_t                         u1EVTP:1;
  uint32_t                         :5;
} stc_CANFD_CH_TTOCF_field_t;

typedef union un_CANFD_CH_TTOCF {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTOCF_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTOCF_t;

/**
  * \brief TT Matrix Limits (CANFD_CH_TTMLM)
  */
typedef struct stc_CANFD_CH_TTMLM_field {
  uint32_t                         u6CCM:6;
  uint32_t                         u2CSS:2;
  uint32_t                         u4TXEW:4;
  uint32_t                         :4;
  uint32_t                         u12ENTT:12;
  uint32_t                         :4;
} stc_CANFD_CH_TTMLM_field_t;

typedef union un_CANFD_CH_TTMLM {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTMLM_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTMLM_t;

/**
  * \brief TUR Configuration (CANFD_CH_TURCF)
  */
typedef struct stc_CANFD_CH_TURCF_field {
  uint32_t                         u16NCL:16;
  uint32_t                         u14DC:14;
  uint32_t                         :1;
  uint32_t                         u1ELT:1;
} stc_CANFD_CH_TURCF_field_t;

typedef union un_CANFD_CH_TURCF {
  uint32_t                         u32Register;
  stc_CANFD_CH_TURCF_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TURCF_t;

/**
  * \brief TT Operation Control (CANFD_CH_TTOCN)
  */
typedef struct stc_CANFD_CH_TTOCN_field {
  uint32_t                         u1SGT:1;
  uint32_t                         u1ECS:1;
  uint32_t                         u1SWP:1;
  uint32_t                         u2SWS:2;
  uint32_t                         u1RTIE:1;
  uint32_t                         u2TMC:2;
  uint32_t                         u1TTIE:1;
  uint32_t                         u1GCS:1;
  uint32_t                         u1FGP:1;
  uint32_t                         u1TMG:1;
  uint32_t                         u1NIG:1;
  uint32_t                         u1ESCN:1;
  uint32_t                         :1;
  uint32_t                         u1LCKC:1;
  uint32_t                         :16;
} stc_CANFD_CH_TTOCN_field_t;

typedef union un_CANFD_CH_TTOCN {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTOCN_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTOCN_t;

/**
  * \brief TT Global Time Preset (CANFD_CH_TTGTP)
  */
typedef struct stc_CANFD_CH_TTGTP_field {
  uint32_t                         u16TP:16;
  uint32_t                         u16CTP:16;
} stc_CANFD_CH_TTGTP_field_t;

typedef union un_CANFD_CH_TTGTP {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTGTP_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTGTP_t;

/**
  * \brief TT Time Mark (CANFD_CH_TTTMK)
  */
typedef struct stc_CANFD_CH_TTTMK_field {
  uint32_t                         u16TM_:16;
  uint32_t                         u7TICC:7;
  uint32_t                         :8;
  uint32_t                         u1LCKM:1;
} stc_CANFD_CH_TTTMK_field_t;

typedef union un_CANFD_CH_TTTMK {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTTMK_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTTMK_t;

/**
  * \brief TT Interrupt Register (CANFD_CH_TTIR)
  */
typedef struct stc_CANFD_CH_TTIR_field {
  uint32_t                         u1SBC:1;
  uint32_t                         u1SMC:1;
  uint32_t                         u1CSM_:1;
  uint32_t                         u1SOG:1;
  uint32_t                         u1RTMI:1;
  uint32_t                         u1TTMI:1;
  uint32_t                         u1SWE:1;
  uint32_t                         u1GTW:1;
  uint32_t                         u1GTD:1;
  uint32_t                         u1GTE:1;
  uint32_t                         u1TXU:1;
  uint32_t                         u1TXO:1;
  uint32_t                         u1SE1:1;
  uint32_t                         u1SE2:1;
  uint32_t                         u1ELC:1;
  uint32_t                         u1IWT:1;
  uint32_t                         u1WT:1;
  uint32_t                         u1AW:1;
  uint32_t                         u1CER:1;
  uint32_t                         :13;
} stc_CANFD_CH_TTIR_field_t;

typedef union un_CANFD_CH_TTIR {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTIR_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTIR_t;

/**
  * \brief TT Interrupt Enable (CANFD_CH_TTIE)
  */
typedef struct stc_CANFD_CH_TTIE_field {
  uint32_t                         u1SBCE:1;
  uint32_t                         u1SMCE:1;
  uint32_t                         u1CSME:1;
  uint32_t                         u1SOGE:1;
  uint32_t                         u1RTMIE:1;
  uint32_t                         u1TTMIE:1;
  uint32_t                         u1SWEE:1;
  uint32_t                         u1GTWE:1;
  uint32_t                         u1GTDE:1;
  uint32_t                         u1GTEE:1;
  uint32_t                         u1TXUE:1;
  uint32_t                         u1TXOE:1;
  uint32_t                         u1SE1E:1;
  uint32_t                         u1SE2E:1;
  uint32_t                         u1ELCE:1;
  uint32_t                         u1IWTE:1;
  uint32_t                         u1WTE:1;
  uint32_t                         u1AWE_:1;
  uint32_t                         u1CERE:1;
  uint32_t                         :13;
} stc_CANFD_CH_TTIE_field_t;

typedef union un_CANFD_CH_TTIE {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTIE_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTIE_t;

/**
  * \brief TT Interrupt Line Select (CANFD_CH_TTILS)
  */
typedef struct stc_CANFD_CH_TTILS_field {
  uint32_t                         u1SBCL:1;
  uint32_t                         u1SMCL:1;
  uint32_t                         u1CSML:1;
  uint32_t                         u1SOGL:1;
  uint32_t                         u1RTMIL:1;
  uint32_t                         u1TTMIL:1;
  uint32_t                         u1SWEL:1;
  uint32_t                         u1GTWL:1;
  uint32_t                         u1GTDL:1;
  uint32_t                         u1GTEL:1;
  uint32_t                         u1TXUL:1;
  uint32_t                         u1TXOL:1;
  uint32_t                         u1SE1L:1;
  uint32_t                         u1SE2L:1;
  uint32_t                         u1ELCL:1;
  uint32_t                         u1IWTL:1;
  uint32_t                         u1WTL:1;
  uint32_t                         u1AWL_:1;
  uint32_t                         u1CERL:1;
  uint32_t                         :13;
} stc_CANFD_CH_TTILS_field_t;

typedef union un_CANFD_CH_TTILS {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTILS_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTILS_t;

/**
  * \brief TT Operation Status (CANFD_CH_TTOST)
  */
typedef struct stc_CANFD_CH_TTOST_field {
  uint32_t                         u2EL:2;
  uint32_t                         u2MS:2;
  uint32_t                         u2SYS:2;
  uint32_t                         u1QGTP:1;
  uint32_t                         u1QCS:1;
  uint32_t                         u8RTO:8;
  uint32_t                         :6;
  uint32_t                         u1WGTD:1;
  uint32_t                         u1GFI:1;
  uint32_t                         u3TMP:3;
  uint32_t                         u1GSI:1;
  uint32_t                         u1WFE:1;
  uint32_t                         u1AWE:1;
  uint32_t                         u1WECS:1;
  uint32_t                         u1SPL:1;
} stc_CANFD_CH_TTOST_field_t;

typedef union un_CANFD_CH_TTOST {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTOST_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTOST_t;

/**
  * \brief TUR Numerator Actual (CANFD_CH_TURNA)
  */
typedef struct stc_CANFD_CH_TURNA_field {
  uint32_t                         u18NAV:18;
  uint32_t                         :14;
} stc_CANFD_CH_TURNA_field_t;

typedef union un_CANFD_CH_TURNA {
  uint32_t                         u32Register;
  stc_CANFD_CH_TURNA_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TURNA_t;

/**
  * \brief TT Local & Global Time (CANFD_CH_TTLGT)
  */
typedef struct stc_CANFD_CH_TTLGT_field {
  uint32_t                         u16LT:16;
  uint32_t                         u16GT:16;
} stc_CANFD_CH_TTLGT_field_t;

typedef union un_CANFD_CH_TTLGT {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTLGT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTLGT_t;

/**
  * \brief TT Cycle Time & Count (CANFD_CH_TTCTC)
  */
typedef struct stc_CANFD_CH_TTCTC_field {
  uint32_t                         u16CT:16;
  uint32_t                         u6CC:6;
  uint32_t                         :10;
} stc_CANFD_CH_TTCTC_field_t;

typedef union un_CANFD_CH_TTCTC {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTCTC_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTCTC_t;

/**
  * \brief TT Capture Time (CANFD_CH_TTCPT)
  */
typedef struct stc_CANFD_CH_TTCPT_field {
  uint32_t                         u6CCV:6;
  uint32_t                         :10;
  uint32_t                         u16SWV:16;
} stc_CANFD_CH_TTCPT_field_t;

typedef union un_CANFD_CH_TTCPT {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTCPT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTCPT_t;

/**
  * \brief TT Cycle Sync Mark (CANFD_CH_TTCSM)
  */
typedef struct stc_CANFD_CH_TTCSM_field {
  uint32_t                         u16CSM:16;
  uint32_t                         :16;
} stc_CANFD_CH_TTCSM_field_t;

typedef union un_CANFD_CH_TTCSM {
  uint32_t                         u32Register;
  stc_CANFD_CH_TTCSM_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_TTCSM_t;



/**
  * \brief Receive FIFO Top control (CANFD_CH_RXFTOP_CTL)
  */
typedef struct stc_CANFD_CH_RXFTOP_CTL_field {
  uint32_t                         u1F0TPE:1;
  uint32_t                         u1F1TPE:1;
  uint32_t                         :30;
} stc_CANFD_CH_RXFTOP_CTL_field_t;

typedef union un_CANFD_CH_RXFTOP_CTL {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXFTOP_CTL_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXFTOP_CTL_t;

/**
  * \brief Receive FIFO 0 Top Status (CANFD_CH_RXFTOP0_STAT)
  */
typedef struct stc_CANFD_CH_RXFTOP0_STAT_field {
  uint32_t                         u16F0TA:16;
  uint32_t                         :16;
} stc_CANFD_CH_RXFTOP0_STAT_field_t;

typedef union un_CANFD_CH_RXFTOP0_STAT {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXFTOP0_STAT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXFTOP0_STAT_t;

/**
  * \brief Receive FIFO 0 Top Data (CANFD_CH_RXFTOP0_DATA)
  */
typedef struct stc_CANFD_CH_RXFTOP0_DATA_field {
  uint32_t                         u32F0TD:32;
} stc_CANFD_CH_RXFTOP0_DATA_field_t;

typedef union un_CANFD_CH_RXFTOP0_DATA {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXFTOP0_DATA_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXFTOP0_DATA_t;

/**
  * \brief Receive FIFO 1 Top Status (CANFD_CH_RXFTOP1_STAT)
  */
typedef struct stc_CANFD_CH_RXFTOP1_STAT_field {
  uint32_t                         u16F1TA:16;
  uint32_t                         :16;
} stc_CANFD_CH_RXFTOP1_STAT_field_t;

typedef union un_CANFD_CH_RXFTOP1_STAT {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXFTOP1_STAT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXFTOP1_STAT_t;

/**
  * \brief Receive FIFO 1 Top Data (CANFD_CH_RXFTOP1_DATA)
  */
typedef struct stc_CANFD_CH_RXFTOP1_DATA_field {
  uint32_t                         u32F1TD:32;
} stc_CANFD_CH_RXFTOP1_DATA_field_t;

typedef union un_CANFD_CH_RXFTOP1_DATA {
  uint32_t                         u32Register;
  stc_CANFD_CH_RXFTOP1_DATA_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CH_RXFTOP1_DATA_t;



/**
  * \brief Global CAN control register (CANFD_CTL)
  */
typedef struct stc_CANFD_CTL_field {
  uint32_t                         u8STOP_REQ:8;
  uint32_t                         :23;
  uint32_t                         u1MRAM_OFF:1;
} stc_CANFD_CTL_field_t;

typedef union un_CANFD_CTL {
  uint32_t                         u32Register;
  stc_CANFD_CTL_field_t            stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_CTL_t;

/**
  * \brief Global CAN status register (CANFD_STATUS)
  */
typedef struct stc_CANFD_STATUS_field {
  uint32_t                         u8STOP_ACK:8;
  uint32_t                         :24;
} stc_CANFD_STATUS_field_t;

typedef union un_CANFD_STATUS {
  uint32_t                         u32Register;
  stc_CANFD_STATUS_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_STATUS_t;

/**
  * \brief Consolidated interrupt0 cause register (CANFD_INTR0_CAUSE)
  */
typedef struct stc_CANFD_INTR0_CAUSE_field {
  uint32_t                         u8INT0:8;
  uint32_t                         :24;
} stc_CANFD_INTR0_CAUSE_field_t;

typedef union un_CANFD_INTR0_CAUSE {
  uint32_t                         u32Register;
  stc_CANFD_INTR0_CAUSE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_INTR0_CAUSE_t;

/**
  * \brief Consolidated interrupt1 cause register (CANFD_INTR1_CAUSE)
  */
typedef struct stc_CANFD_INTR1_CAUSE_field {
  uint32_t                         u8INT1:8;
  uint32_t                         :24;
} stc_CANFD_INTR1_CAUSE_field_t;

typedef union un_CANFD_INTR1_CAUSE {
  uint32_t                         u32Register;
  stc_CANFD_INTR1_CAUSE_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_INTR1_CAUSE_t;

/**
  * \brief Time Stamp control register (CANFD_TS_CTL)
  */
typedef struct stc_CANFD_TS_CTL_field {
  uint32_t                         u16PRESCALE:16;
  uint32_t                         :15;
  uint32_t                         u1ENABLED:1;
} stc_CANFD_TS_CTL_field_t;

typedef union un_CANFD_TS_CTL {
  uint32_t                         u32Register;
  stc_CANFD_TS_CTL_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_TS_CTL_t;

/**
  * \brief Time Stamp counter value (CANFD_TS_CNT)
  */
typedef struct stc_CANFD_TS_CNT_field {
  uint32_t                         u16VALUE:16;
  uint32_t                         :16;
} stc_CANFD_TS_CNT_field_t;

typedef union un_CANFD_TS_CNT {
  uint32_t                         u32Register;
  stc_CANFD_TS_CNT_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_TS_CNT_t;

/**
  * \brief ECC control (CANFD_ECC_CTL)
  */
typedef struct stc_CANFD_ECC_CTL_field {
  uint32_t                         :16;
  uint32_t                         u1ECC_EN:1;
  uint32_t                         :15;
} stc_CANFD_ECC_CTL_field_t;

typedef union un_CANFD_ECC_CTL {
  uint32_t                         u32Register;
  stc_CANFD_ECC_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_ECC_CTL_t;

/**
  * \brief ECC error injection (CANFD_ECC_ERR_INJ)
  */
typedef struct stc_CANFD_ECC_ERR_INJ_field {
  uint32_t                         :2;
  uint32_t                         u14ERR_ADDR:14;
  uint32_t                         :4;
  uint32_t                         u1ERR_EN:1;
  uint32_t                         :3;
  uint32_t                         u7ERR_PAR:7;
} stc_CANFD_ECC_ERR_INJ_field_t;

typedef union un_CANFD_ECC_ERR_INJ {
  uint32_t                         u32Register;
  stc_CANFD_ECC_ERR_INJ_field_t    stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_CANFD_ECC_ERR_INJ_t;



/**
  * \brief TTCAN 3PIP, includes FD (CANFD_CH_M_TTCAN)
  */
typedef struct stc_CANFD_CH_M_TTCAN {
  un_CANFD_CH_CREL_t               unCREL;             /*!< 0x00000000 Core Release Register */
  un_CANFD_CH_ENDN_t               unENDN;             /*!< 0x00000004 Endian Register */
  uint32_t                         u32Reserved;
  un_CANFD_CH_DBTP_t               unDBTP;             /*!< 0x0000000C Data Bit Timing & Prescaler Register */
  un_CANFD_CH_TEST_t               unTEST;             /*!< 0x00000010 Test Register */
  un_CANFD_CH_RWD_t                unRWD;              /*!< 0x00000014 RAM Watchdog */
  un_CANFD_CH_CCCR_t               unCCCR;             /*!< 0x00000018 CC Control Register */
  un_CANFD_CH_NBTP_t               unNBTP;             /*!< 0x0000001C Nominal Bit Timing & Prescaler Register */
  un_CANFD_CH_TSCC_t               unTSCC;             /*!< 0x00000020 Timestamp Counter Configuration */
  un_CANFD_CH_TSCV_t               unTSCV;             /*!< 0x00000024 Timestamp Counter Value */
  un_CANFD_CH_TOCC_t               unTOCC;             /*!< 0x00000028 Timeout Counter Configuration */
  un_CANFD_CH_TOCV_t               unTOCV;             /*!< 0x0000002C Timeout Counter Value */
  uint32_t                         au32Reserved1[4];
  un_CANFD_CH_ECR_t                unECR;              /*!< 0x00000040 Error Counter Register */
  un_CANFD_CH_PSR_t                unPSR;              /*!< 0x00000044 Protocol Status Register */
  un_CANFD_CH_TDCR_t               unTDCR;             /*!< 0x00000048 Transmitter Delay Compensation Register */
  uint32_t                         u32Reserved2;
  un_CANFD_CH_IR_t                 unIR;               /*!< 0x00000050 Interrupt Register */
  un_CANFD_CH_IE_t                 unIE;               /*!< 0x00000054 Interrupt Enable */
  un_CANFD_CH_ILS_t                unILS;              /*!< 0x00000058 Interrupt Line Select */
  un_CANFD_CH_ILE_t                unILE;              /*!< 0x0000005C Interrupt Line Enable */
  uint32_t                         au32Reserved3[8];
  un_CANFD_CH_GFC_t                unGFC;              /*!< 0x00000080 Global Filter Configuration */
  un_CANFD_CH_SIDFC_t              unSIDFC;            /*!< 0x00000084 Standard ID Filter Configuration */
  un_CANFD_CH_XIDFC_t              unXIDFC;            /*!< 0x00000088 Extended ID Filter Configuration */
  uint32_t                         u32Reserved4;
  un_CANFD_CH_XIDAM_t              unXIDAM;            /*!< 0x00000090 Extended ID AND Mask */
  un_CANFD_CH_HPMS_t               unHPMS;             /*!< 0x00000094 High Priority Message Status */
  un_CANFD_CH_NDAT1_t              unNDAT1;            /*!< 0x00000098 New Data 1 */
  un_CANFD_CH_NDAT2_t              unNDAT2;            /*!< 0x0000009C New Data 2 */
  un_CANFD_CH_RXF0C_t              unRXF0C;            /*!< 0x000000A0 Rx FIFO 0 Configuration */
  un_CANFD_CH_RXF0S_t              unRXF0S;            /*!< 0x000000A4 Rx FIFO 0 Status */
  un_CANFD_CH_RXF0A_t              unRXF0A;            /*!< 0x000000A8 Rx FIFO 0 Acknowledge */
  un_CANFD_CH_RXBC_t               unRXBC;             /*!< 0x000000AC Rx Buffer Configuration */
  un_CANFD_CH_RXF1C_t              unRXF1C;            /*!< 0x000000B0 Rx FIFO 1 Configuration */
  un_CANFD_CH_RXF1S_t              unRXF1S;            /*!< 0x000000B4 Rx FIFO 1 Status */
  un_CANFD_CH_RXF1A_t              unRXF1A;            /*!< 0x000000B8 Rx FIFO 1 Acknowledge */
  un_CANFD_CH_RXESC_t              unRXESC;            /*!< 0x000000BC Rx Buffer / FIFO Element Size Configuration */
  un_CANFD_CH_TXBC_t               unTXBC;             /*!< 0x000000C0 Tx Buffer Configuration */
  un_CANFD_CH_TXFQS_t              unTXFQS;            /*!< 0x000000C4 Tx FIFO/Queue Status */
  un_CANFD_CH_TXESC_t              unTXESC;            /*!< 0x000000C8 Tx Buffer Element Size Configuration */
  un_CANFD_CH_TXBRP_t              unTXBRP;            /*!< 0x000000CC Tx Buffer Request Pending */
  un_CANFD_CH_TXBAR_t              unTXBAR;            /*!< 0x000000D0 Tx Buffer Add Request */
  un_CANFD_CH_TXBCR_t              unTXBCR;            /*!< 0x000000D4 Tx Buffer Cancellation Request */
  un_CANFD_CH_TXBTO_t              unTXBTO;            /*!< 0x000000D8 Tx Buffer Transmission Occurred */
  un_CANFD_CH_TXBCF_t              unTXBCF;            /*!< 0x000000DC Tx Buffer Cancellation Finished */
  un_CANFD_CH_TXBTIE_t             unTXBTIE;           /*!< 0x000000E0 Tx Buffer Transmission Interrupt Enable */
  un_CANFD_CH_TXBCIE_t             unTXBCIE;           /*!< 0x000000E4 Tx Buffer Cancellation Finished Interrupt Enable */
  uint32_t                         au32Reserved5[2];
  un_CANFD_CH_TXEFC_t              unTXEFC;            /*!< 0x000000F0 Tx Event FIFO Configuration */
  un_CANFD_CH_TXEFS_t              unTXEFS;            /*!< 0x000000F4 Tx Event FIFO Status */
  un_CANFD_CH_TXEFA_t              unTXEFA;            /*!< 0x000000F8 Tx Event FIFO Acknowledge */
  uint32_t                         u32Reserved6;
  un_CANFD_CH_TTTMC_t              unTTTMC;            /*!< 0x00000100 TT Trigger Memory Configuration */
  un_CANFD_CH_TTRMC_t              unTTRMC;            /*!< 0x00000104 TT Reference Message Configuration */
  un_CANFD_CH_TTOCF_t              unTTOCF;            /*!< 0x00000108 TT Operation Configuration */
  un_CANFD_CH_TTMLM_t              unTTMLM;            /*!< 0x0000010C TT Matrix Limits */
  un_CANFD_CH_TURCF_t              unTURCF;            /*!< 0x00000110 TUR Configuration */
  un_CANFD_CH_TTOCN_t              unTTOCN;            /*!< 0x00000114 TT Operation Control */
  un_CANFD_CH_TTGTP_t              unTTGTP;            /*!< 0x00000118 TT Global Time Preset */
  un_CANFD_CH_TTTMK_t              unTTTMK;            /*!< 0x0000011C TT Time Mark */
  un_CANFD_CH_TTIR_t               unTTIR;             /*!< 0x00000120 TT Interrupt Register */
  un_CANFD_CH_TTIE_t               unTTIE;             /*!< 0x00000124 TT Interrupt Enable */
  un_CANFD_CH_TTILS_t              unTTILS;            /*!< 0x00000128 TT Interrupt Line Select */
  un_CANFD_CH_TTOST_t              unTTOST;            /*!< 0x0000012C TT Operation Status */
  un_CANFD_CH_TURNA_t              unTURNA;            /*!< 0x00000130 TUR Numerator Actual */
  un_CANFD_CH_TTLGT_t              unTTLGT;            /*!< 0x00000134 TT Local & Global Time */
  un_CANFD_CH_TTCTC_t              unTTCTC;            /*!< 0x00000138 TT Cycle Time & Count */
  un_CANFD_CH_TTCPT_t              unTTCPT;            /*!< 0x0000013C TT Capture Time */
  un_CANFD_CH_TTCSM_t              unTTCSM;            /*!< 0x00000140 TT Cycle Sync Mark */
  uint32_t                         au32Reserved7[15];
} stc_CANFD_CH_M_TTCAN_t;                              /*!< Size = 384 (0x180) */

/**
  * \brief FIFO wrapper around M_TTCAN 3PIP, to enable DMA (CANFD_CH)
  */
typedef struct stc_CANFD_CH {
  stc_CANFD_CH_M_TTCAN_t           M_TTCAN;            /*!< 0x00000000 TTCAN 3PIP, includes FD */
  un_CANFD_CH_RXFTOP_CTL_t         unRXFTOP_CTL;       /*!< 0x00000180 Receive FIFO Top control */
  uint32_t                         au32Reserved[7];
  un_CANFD_CH_RXFTOP0_STAT_t       unRXFTOP0_STAT;     /*!< 0x000001A0 Receive FIFO 0 Top Status */
  uint32_t                         u32Reserved1;
  un_CANFD_CH_RXFTOP0_DATA_t       unRXFTOP0_DATA;     /*!< 0x000001A8 Receive FIFO 0 Top Data */
  uint32_t                         u32Reserved2;
  un_CANFD_CH_RXFTOP1_STAT_t       unRXFTOP1_STAT;     /*!< 0x000001B0 Receive FIFO 1 Top Status */
  uint32_t                         u32Reserved3;
  un_CANFD_CH_RXFTOP1_DATA_t       unRXFTOP1_DATA;     /*!< 0x000001B8 Receive FIFO 1 Top Data */
  uint32_t                         au32Reserved4[17];
} stc_CANFD_CH_t;                                      /*!< Size = 512 (0x200) */

/**
  * \brief CAN Controller (CANFD)
  */
typedef struct stc_CANFD {
  stc_CANFD_CH_t                   CH[8];              /*!< 0x00000000 FIFO wrapper around M_TTCAN 3PIP, to enable DMA */
  un_CANFD_CTL_t                   unCTL;              /*!< 0x00001000 Global CAN control register */
  un_CANFD_STATUS_t                unSTATUS;           /*!< 0x00001004 Global CAN status register */
  uint32_t                         au32Reserved[2];
  un_CANFD_INTR0_CAUSE_t           unINTR0_CAUSE;      /*!< 0x00001010 Consolidated interrupt0 cause register */
  un_CANFD_INTR1_CAUSE_t           unINTR1_CAUSE;      /*!< 0x00001014 Consolidated interrupt1 cause register */
  uint32_t                         au32Reserved1[2];
  un_CANFD_TS_CTL_t                unTS_CTL;           /*!< 0x00001020 Time Stamp control register */
  un_CANFD_TS_CNT_t                unTS_CNT;           /*!< 0x00001024 Time Stamp counter value */
  uint32_t                         au32Reserved2[22];
  un_CANFD_ECC_CTL_t               unECC_CTL;          /*!< 0x00001080 ECC control */
  un_CANFD_ECC_ERR_INJ_t           unECC_ERR_INJ;      /*!< 0x00001084 ECC error injection */
} stc_CANFD_t;                                         /*!< Size = 4232 (0x1088) */


/* CANFD_CH_M_TTCAN.CREL */
#define CANFD_CH_M_TTCAN_CREL_DAY_Pos           0UL
#define CANFD_CH_M_TTCAN_CREL_DAY_Msk           0xFFUL
#define CANFD_CH_M_TTCAN_CREL_MON_Pos           8UL
#define CANFD_CH_M_TTCAN_CREL_MON_Msk           0xFF00UL
#define CANFD_CH_M_TTCAN_CREL_YEAR_Pos          16UL
#define CANFD_CH_M_TTCAN_CREL_YEAR_Msk          0xF0000UL
#define CANFD_CH_M_TTCAN_CREL_SUBSTEP_Pos       20UL
#define CANFD_CH_M_TTCAN_CREL_SUBSTEP_Msk       0xF00000UL
#define CANFD_CH_M_TTCAN_CREL_STEP_Pos          24UL
#define CANFD_CH_M_TTCAN_CREL_STEP_Msk          0xF000000UL
#define CANFD_CH_M_TTCAN_CREL_REL_Pos           28UL
#define CANFD_CH_M_TTCAN_CREL_REL_Msk           0xF0000000UL
/* CANFD_CH_M_TTCAN.ENDN */
#define CANFD_CH_M_TTCAN_ENDN_ETV_Pos           0UL
#define CANFD_CH_M_TTCAN_ENDN_ETV_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.DBTP */
#define CANFD_CH_M_TTCAN_DBTP_DSJW_Pos          0UL
#define CANFD_CH_M_TTCAN_DBTP_DSJW_Msk          0xFUL
#define CANFD_CH_M_TTCAN_DBTP_DTSEG2_Pos        4UL
#define CANFD_CH_M_TTCAN_DBTP_DTSEG2_Msk        0xF0UL
#define CANFD_CH_M_TTCAN_DBTP_DTSEG1_Pos        8UL
#define CANFD_CH_M_TTCAN_DBTP_DTSEG1_Msk        0x1F00UL
#define CANFD_CH_M_TTCAN_DBTP_DBRP_Pos          16UL
#define CANFD_CH_M_TTCAN_DBTP_DBRP_Msk          0x1F0000UL
#define CANFD_CH_M_TTCAN_DBTP_TDC_Pos           23UL
#define CANFD_CH_M_TTCAN_DBTP_TDC_Msk           0x800000UL
/* CANFD_CH_M_TTCAN.TEST */
#define CANFD_CH_M_TTCAN_TEST_TAM_Pos           0UL
#define CANFD_CH_M_TTCAN_TEST_TAM_Msk           0x1UL
#define CANFD_CH_M_TTCAN_TEST_TAT_Pos           1UL
#define CANFD_CH_M_TTCAN_TEST_TAT_Msk           0x2UL
#define CANFD_CH_M_TTCAN_TEST_CAM_Pos           2UL
#define CANFD_CH_M_TTCAN_TEST_CAM_Msk           0x4UL
#define CANFD_CH_M_TTCAN_TEST_CAT_Pos           3UL
#define CANFD_CH_M_TTCAN_TEST_CAT_Msk           0x8UL
#define CANFD_CH_M_TTCAN_TEST_LBCK_Pos          4UL
#define CANFD_CH_M_TTCAN_TEST_LBCK_Msk          0x10UL
#define CANFD_CH_M_TTCAN_TEST_TX_Pos            5UL
#define CANFD_CH_M_TTCAN_TEST_TX_Msk            0x60UL
#define CANFD_CH_M_TTCAN_TEST_RX_Pos            7UL
#define CANFD_CH_M_TTCAN_TEST_RX_Msk            0x80UL
/* CANFD_CH_M_TTCAN.RWD */
#define CANFD_CH_M_TTCAN_RWD_WDC_Pos            0UL
#define CANFD_CH_M_TTCAN_RWD_WDC_Msk            0xFFUL
#define CANFD_CH_M_TTCAN_RWD_WDV_Pos            8UL
#define CANFD_CH_M_TTCAN_RWD_WDV_Msk            0xFF00UL
/* CANFD_CH_M_TTCAN.CCCR */
#define CANFD_CH_M_TTCAN_CCCR_INIT_Pos          0UL
#define CANFD_CH_M_TTCAN_CCCR_INIT_Msk          0x1UL
#define CANFD_CH_M_TTCAN_CCCR_CCE_Pos           1UL
#define CANFD_CH_M_TTCAN_CCCR_CCE_Msk           0x2UL
#define CANFD_CH_M_TTCAN_CCCR_ASM_Pos           2UL
#define CANFD_CH_M_TTCAN_CCCR_ASM_Msk           0x4UL
#define CANFD_CH_M_TTCAN_CCCR_CSA_Pos           3UL
#define CANFD_CH_M_TTCAN_CCCR_CSA_Msk           0x8UL
#define CANFD_CH_M_TTCAN_CCCR_CSR_Pos           4UL
#define CANFD_CH_M_TTCAN_CCCR_CSR_Msk           0x10UL
#define CANFD_CH_M_TTCAN_CCCR_MON__Pos          5UL
#define CANFD_CH_M_TTCAN_CCCR_MON__Msk          0x20UL
#define CANFD_CH_M_TTCAN_CCCR_DAR_Pos           6UL
#define CANFD_CH_M_TTCAN_CCCR_DAR_Msk           0x40UL
#define CANFD_CH_M_TTCAN_CCCR_TEST_Pos          7UL
#define CANFD_CH_M_TTCAN_CCCR_TEST_Msk          0x80UL
#define CANFD_CH_M_TTCAN_CCCR_FDOE_Pos          8UL
#define CANFD_CH_M_TTCAN_CCCR_FDOE_Msk          0x100UL
#define CANFD_CH_M_TTCAN_CCCR_BRSE_Pos          9UL
#define CANFD_CH_M_TTCAN_CCCR_BRSE_Msk          0x200UL
#define CANFD_CH_M_TTCAN_CCCR_PXHD_Pos          12UL
#define CANFD_CH_M_TTCAN_CCCR_PXHD_Msk          0x1000UL
#define CANFD_CH_M_TTCAN_CCCR_EFBI_Pos          13UL
#define CANFD_CH_M_TTCAN_CCCR_EFBI_Msk          0x2000UL
#define CANFD_CH_M_TTCAN_CCCR_TXP_Pos           14UL
#define CANFD_CH_M_TTCAN_CCCR_TXP_Msk           0x4000UL
#define CANFD_CH_M_TTCAN_CCCR_NISO_Pos          15UL
#define CANFD_CH_M_TTCAN_CCCR_NISO_Msk          0x8000UL
/* CANFD_CH_M_TTCAN.NBTP */
#define CANFD_CH_M_TTCAN_NBTP_NTSEG2_Pos        0UL
#define CANFD_CH_M_TTCAN_NBTP_NTSEG2_Msk        0x7FUL
#define CANFD_CH_M_TTCAN_NBTP_NTSEG1_Pos        8UL
#define CANFD_CH_M_TTCAN_NBTP_NTSEG1_Msk        0xFF00UL
#define CANFD_CH_M_TTCAN_NBTP_NBRP_Pos          16UL
#define CANFD_CH_M_TTCAN_NBTP_NBRP_Msk          0x1FF0000UL
#define CANFD_CH_M_TTCAN_NBTP_NSJW_Pos          25UL
#define CANFD_CH_M_TTCAN_NBTP_NSJW_Msk          0xFE000000UL
/* CANFD_CH_M_TTCAN.TSCC */
#define CANFD_CH_M_TTCAN_TSCC_TSS_Pos           0UL
#define CANFD_CH_M_TTCAN_TSCC_TSS_Msk           0x3UL
#define CANFD_CH_M_TTCAN_TSCC_TCP_Pos           16UL
#define CANFD_CH_M_TTCAN_TSCC_TCP_Msk           0xF0000UL
/* CANFD_CH_M_TTCAN.TSCV */
#define CANFD_CH_M_TTCAN_TSCV_TSC_Pos           0UL
#define CANFD_CH_M_TTCAN_TSCV_TSC_Msk           0xFFFFUL
/* CANFD_CH_M_TTCAN.TOCC */
#define CANFD_CH_M_TTCAN_TOCC_ETOC_Pos          0UL
#define CANFD_CH_M_TTCAN_TOCC_ETOC_Msk          0x1UL
#define CANFD_CH_M_TTCAN_TOCC_TOS_Pos           1UL
#define CANFD_CH_M_TTCAN_TOCC_TOS_Msk           0x6UL
#define CANFD_CH_M_TTCAN_TOCC_TOP_Pos           16UL
#define CANFD_CH_M_TTCAN_TOCC_TOP_Msk           0xFFFF0000UL
/* CANFD_CH_M_TTCAN.TOCV */
#define CANFD_CH_M_TTCAN_TOCV_TOC_Pos           0UL
#define CANFD_CH_M_TTCAN_TOCV_TOC_Msk           0xFFFFUL
/* CANFD_CH_M_TTCAN.ECR */
#define CANFD_CH_M_TTCAN_ECR_TEC_Pos            0UL
#define CANFD_CH_M_TTCAN_ECR_TEC_Msk            0xFFUL
#define CANFD_CH_M_TTCAN_ECR_REC_Pos            8UL
#define CANFD_CH_M_TTCAN_ECR_REC_Msk            0x7F00UL
#define CANFD_CH_M_TTCAN_ECR_RP_Pos             15UL
#define CANFD_CH_M_TTCAN_ECR_RP_Msk             0x8000UL
#define CANFD_CH_M_TTCAN_ECR_CEL_Pos            16UL
#define CANFD_CH_M_TTCAN_ECR_CEL_Msk            0xFF0000UL
/* CANFD_CH_M_TTCAN.PSR */
#define CANFD_CH_M_TTCAN_PSR_LEC_Pos            0UL
#define CANFD_CH_M_TTCAN_PSR_LEC_Msk            0x7UL
#define CANFD_CH_M_TTCAN_PSR_ACT_Pos            3UL
#define CANFD_CH_M_TTCAN_PSR_ACT_Msk            0x18UL
#define CANFD_CH_M_TTCAN_PSR_EP_Pos             5UL
#define CANFD_CH_M_TTCAN_PSR_EP_Msk             0x20UL
#define CANFD_CH_M_TTCAN_PSR_EW_Pos             6UL
#define CANFD_CH_M_TTCAN_PSR_EW_Msk             0x40UL
#define CANFD_CH_M_TTCAN_PSR_BO_Pos             7UL
#define CANFD_CH_M_TTCAN_PSR_BO_Msk             0x80UL
#define CANFD_CH_M_TTCAN_PSR_DLEC_Pos           8UL
#define CANFD_CH_M_TTCAN_PSR_DLEC_Msk           0x700UL
#define CANFD_CH_M_TTCAN_PSR_RESI_Pos           11UL
#define CANFD_CH_M_TTCAN_PSR_RESI_Msk           0x800UL
#define CANFD_CH_M_TTCAN_PSR_RBRS_Pos           12UL
#define CANFD_CH_M_TTCAN_PSR_RBRS_Msk           0x1000UL
#define CANFD_CH_M_TTCAN_PSR_RFDF_Pos           13UL
#define CANFD_CH_M_TTCAN_PSR_RFDF_Msk           0x2000UL
#define CANFD_CH_M_TTCAN_PSR_PXE_Pos            14UL
#define CANFD_CH_M_TTCAN_PSR_PXE_Msk            0x4000UL
#define CANFD_CH_M_TTCAN_PSR_TDCV_Pos           16UL
#define CANFD_CH_M_TTCAN_PSR_TDCV_Msk           0x7F0000UL
/* CANFD_CH_M_TTCAN.TDCR */
#define CANFD_CH_M_TTCAN_TDCR_TDCF_Pos          0UL
#define CANFD_CH_M_TTCAN_TDCR_TDCF_Msk          0x7FUL
#define CANFD_CH_M_TTCAN_TDCR_TDCO_Pos          8UL
#define CANFD_CH_M_TTCAN_TDCR_TDCO_Msk          0x7F00UL
/* CANFD_CH_M_TTCAN.IR */
#define CANFD_CH_M_TTCAN_IR_RF0N_Pos            0UL
#define CANFD_CH_M_TTCAN_IR_RF0N_Msk            0x1UL
#define CANFD_CH_M_TTCAN_IR_RF0W_Pos            1UL
#define CANFD_CH_M_TTCAN_IR_RF0W_Msk            0x2UL
#define CANFD_CH_M_TTCAN_IR_RF0F_Pos            2UL
#define CANFD_CH_M_TTCAN_IR_RF0F_Msk            0x4UL
#define CANFD_CH_M_TTCAN_IR_RF0L__Pos           3UL
#define CANFD_CH_M_TTCAN_IR_RF0L__Msk           0x8UL
#define CANFD_CH_M_TTCAN_IR_RF1N_Pos            4UL
#define CANFD_CH_M_TTCAN_IR_RF1N_Msk            0x10UL
#define CANFD_CH_M_TTCAN_IR_RF1W_Pos            5UL
#define CANFD_CH_M_TTCAN_IR_RF1W_Msk            0x20UL
#define CANFD_CH_M_TTCAN_IR_RF1F_Pos            6UL
#define CANFD_CH_M_TTCAN_IR_RF1F_Msk            0x40UL
#define CANFD_CH_M_TTCAN_IR_RF1L__Pos           7UL
#define CANFD_CH_M_TTCAN_IR_RF1L__Msk           0x80UL
#define CANFD_CH_M_TTCAN_IR_HPM_Pos             8UL
#define CANFD_CH_M_TTCAN_IR_HPM_Msk             0x100UL
#define CANFD_CH_M_TTCAN_IR_TC_Pos              9UL
#define CANFD_CH_M_TTCAN_IR_TC_Msk              0x200UL
#define CANFD_CH_M_TTCAN_IR_TCF_Pos             10UL
#define CANFD_CH_M_TTCAN_IR_TCF_Msk             0x400UL
#define CANFD_CH_M_TTCAN_IR_TFE_Pos             11UL
#define CANFD_CH_M_TTCAN_IR_TFE_Msk             0x800UL
#define CANFD_CH_M_TTCAN_IR_TEFN_Pos            12UL
#define CANFD_CH_M_TTCAN_IR_TEFN_Msk            0x1000UL
#define CANFD_CH_M_TTCAN_IR_TEFW_Pos            13UL
#define CANFD_CH_M_TTCAN_IR_TEFW_Msk            0x2000UL
#define CANFD_CH_M_TTCAN_IR_TEFF_Pos            14UL
#define CANFD_CH_M_TTCAN_IR_TEFF_Msk            0x4000UL
#define CANFD_CH_M_TTCAN_IR_TEFL__Pos           15UL
#define CANFD_CH_M_TTCAN_IR_TEFL__Msk           0x8000UL
#define CANFD_CH_M_TTCAN_IR_TSW_Pos             16UL
#define CANFD_CH_M_TTCAN_IR_TSW_Msk             0x10000UL
#define CANFD_CH_M_TTCAN_IR_MRAF_Pos            17UL
#define CANFD_CH_M_TTCAN_IR_MRAF_Msk            0x20000UL
#define CANFD_CH_M_TTCAN_IR_TOO_Pos             18UL
#define CANFD_CH_M_TTCAN_IR_TOO_Msk             0x40000UL
#define CANFD_CH_M_TTCAN_IR_DRX_Pos             19UL
#define CANFD_CH_M_TTCAN_IR_DRX_Msk             0x80000UL
#define CANFD_CH_M_TTCAN_IR_BEC_Pos             20UL
#define CANFD_CH_M_TTCAN_IR_BEC_Msk             0x100000UL
#define CANFD_CH_M_TTCAN_IR_BEU_Pos             21UL
#define CANFD_CH_M_TTCAN_IR_BEU_Msk             0x200000UL
#define CANFD_CH_M_TTCAN_IR_ELO_Pos             22UL
#define CANFD_CH_M_TTCAN_IR_ELO_Msk             0x400000UL
#define CANFD_CH_M_TTCAN_IR_EP__Pos             23UL
#define CANFD_CH_M_TTCAN_IR_EP__Msk             0x800000UL
#define CANFD_CH_M_TTCAN_IR_EW__Pos             24UL
#define CANFD_CH_M_TTCAN_IR_EW__Msk             0x1000000UL
#define CANFD_CH_M_TTCAN_IR_BO__Pos             25UL
#define CANFD_CH_M_TTCAN_IR_BO__Msk             0x2000000UL
#define CANFD_CH_M_TTCAN_IR_WDI_Pos             26UL
#define CANFD_CH_M_TTCAN_IR_WDI_Msk             0x4000000UL
#define CANFD_CH_M_TTCAN_IR_PEA_Pos             27UL
#define CANFD_CH_M_TTCAN_IR_PEA_Msk             0x8000000UL
#define CANFD_CH_M_TTCAN_IR_PED_Pos             28UL
#define CANFD_CH_M_TTCAN_IR_PED_Msk             0x10000000UL
#define CANFD_CH_M_TTCAN_IR_ARA_Pos             29UL
#define CANFD_CH_M_TTCAN_IR_ARA_Msk             0x20000000UL
/* CANFD_CH_M_TTCAN.IE */
#define CANFD_CH_M_TTCAN_IE_RF0NE_Pos           0UL
#define CANFD_CH_M_TTCAN_IE_RF0NE_Msk           0x1UL
#define CANFD_CH_M_TTCAN_IE_RF0WE_Pos           1UL
#define CANFD_CH_M_TTCAN_IE_RF0WE_Msk           0x2UL
#define CANFD_CH_M_TTCAN_IE_RF0FE_Pos           2UL
#define CANFD_CH_M_TTCAN_IE_RF0FE_Msk           0x4UL
#define CANFD_CH_M_TTCAN_IE_RF0LE_Pos           3UL
#define CANFD_CH_M_TTCAN_IE_RF0LE_Msk           0x8UL
#define CANFD_CH_M_TTCAN_IE_RF1NE_Pos           4UL
#define CANFD_CH_M_TTCAN_IE_RF1NE_Msk           0x10UL
#define CANFD_CH_M_TTCAN_IE_RF1WE_Pos           5UL
#define CANFD_CH_M_TTCAN_IE_RF1WE_Msk           0x20UL
#define CANFD_CH_M_TTCAN_IE_RF1FE_Pos           6UL
#define CANFD_CH_M_TTCAN_IE_RF1FE_Msk           0x40UL
#define CANFD_CH_M_TTCAN_IE_RF1LE_Pos           7UL
#define CANFD_CH_M_TTCAN_IE_RF1LE_Msk           0x80UL
#define CANFD_CH_M_TTCAN_IE_HPME_Pos            8UL
#define CANFD_CH_M_TTCAN_IE_HPME_Msk            0x100UL
#define CANFD_CH_M_TTCAN_IE_TCE_Pos             9UL
#define CANFD_CH_M_TTCAN_IE_TCE_Msk             0x200UL
#define CANFD_CH_M_TTCAN_IE_TCFE_Pos            10UL
#define CANFD_CH_M_TTCAN_IE_TCFE_Msk            0x400UL
#define CANFD_CH_M_TTCAN_IE_TFEE_Pos            11UL
#define CANFD_CH_M_TTCAN_IE_TFEE_Msk            0x800UL
#define CANFD_CH_M_TTCAN_IE_TEFNE_Pos           12UL
#define CANFD_CH_M_TTCAN_IE_TEFNE_Msk           0x1000UL
#define CANFD_CH_M_TTCAN_IE_TEFWE_Pos           13UL
#define CANFD_CH_M_TTCAN_IE_TEFWE_Msk           0x2000UL
#define CANFD_CH_M_TTCAN_IE_TEFFE_Pos           14UL
#define CANFD_CH_M_TTCAN_IE_TEFFE_Msk           0x4000UL
#define CANFD_CH_M_TTCAN_IE_TEFLE_Pos           15UL
#define CANFD_CH_M_TTCAN_IE_TEFLE_Msk           0x8000UL
#define CANFD_CH_M_TTCAN_IE_TSWE_Pos            16UL
#define CANFD_CH_M_TTCAN_IE_TSWE_Msk            0x10000UL
#define CANFD_CH_M_TTCAN_IE_MRAFE_Pos           17UL
#define CANFD_CH_M_TTCAN_IE_MRAFE_Msk           0x20000UL
#define CANFD_CH_M_TTCAN_IE_TOOE_Pos            18UL
#define CANFD_CH_M_TTCAN_IE_TOOE_Msk            0x40000UL
#define CANFD_CH_M_TTCAN_IE_DRXE_Pos            19UL
#define CANFD_CH_M_TTCAN_IE_DRXE_Msk            0x80000UL
#define CANFD_CH_M_TTCAN_IE_BECE_Pos            20UL
#define CANFD_CH_M_TTCAN_IE_BECE_Msk            0x100000UL
#define CANFD_CH_M_TTCAN_IE_BEUE_Pos            21UL
#define CANFD_CH_M_TTCAN_IE_BEUE_Msk            0x200000UL
#define CANFD_CH_M_TTCAN_IE_ELOE_Pos            22UL
#define CANFD_CH_M_TTCAN_IE_ELOE_Msk            0x400000UL
#define CANFD_CH_M_TTCAN_IE_EPE_Pos             23UL
#define CANFD_CH_M_TTCAN_IE_EPE_Msk             0x800000UL
#define CANFD_CH_M_TTCAN_IE_EWE_Pos             24UL
#define CANFD_CH_M_TTCAN_IE_EWE_Msk             0x1000000UL
#define CANFD_CH_M_TTCAN_IE_BOE_Pos             25UL
#define CANFD_CH_M_TTCAN_IE_BOE_Msk             0x2000000UL
#define CANFD_CH_M_TTCAN_IE_WDIE_Pos            26UL
#define CANFD_CH_M_TTCAN_IE_WDIE_Msk            0x4000000UL
#define CANFD_CH_M_TTCAN_IE_PEAE_Pos            27UL
#define CANFD_CH_M_TTCAN_IE_PEAE_Msk            0x8000000UL
#define CANFD_CH_M_TTCAN_IE_PEDE_Pos            28UL
#define CANFD_CH_M_TTCAN_IE_PEDE_Msk            0x10000000UL
#define CANFD_CH_M_TTCAN_IE_ARAE_Pos            29UL
#define CANFD_CH_M_TTCAN_IE_ARAE_Msk            0x20000000UL
/* CANFD_CH_M_TTCAN.ILS */
#define CANFD_CH_M_TTCAN_ILS_RF0NL_Pos          0UL
#define CANFD_CH_M_TTCAN_ILS_RF0NL_Msk          0x1UL
#define CANFD_CH_M_TTCAN_ILS_RF0WL_Pos          1UL
#define CANFD_CH_M_TTCAN_ILS_RF0WL_Msk          0x2UL
#define CANFD_CH_M_TTCAN_ILS_RF0FL_Pos          2UL
#define CANFD_CH_M_TTCAN_ILS_RF0FL_Msk          0x4UL
#define CANFD_CH_M_TTCAN_ILS_RF0LL_Pos          3UL
#define CANFD_CH_M_TTCAN_ILS_RF0LL_Msk          0x8UL
#define CANFD_CH_M_TTCAN_ILS_RF1NL_Pos          4UL
#define CANFD_CH_M_TTCAN_ILS_RF1NL_Msk          0x10UL
#define CANFD_CH_M_TTCAN_ILS_RF1WL_Pos          5UL
#define CANFD_CH_M_TTCAN_ILS_RF1WL_Msk          0x20UL
#define CANFD_CH_M_TTCAN_ILS_RF1FL_Pos          6UL
#define CANFD_CH_M_TTCAN_ILS_RF1FL_Msk          0x40UL
#define CANFD_CH_M_TTCAN_ILS_RF1LL_Pos          7UL
#define CANFD_CH_M_TTCAN_ILS_RF1LL_Msk          0x80UL
#define CANFD_CH_M_TTCAN_ILS_HPML_Pos           8UL
#define CANFD_CH_M_TTCAN_ILS_HPML_Msk           0x100UL
#define CANFD_CH_M_TTCAN_ILS_TCL_Pos            9UL
#define CANFD_CH_M_TTCAN_ILS_TCL_Msk            0x200UL
#define CANFD_CH_M_TTCAN_ILS_TCFL_Pos           10UL
#define CANFD_CH_M_TTCAN_ILS_TCFL_Msk           0x400UL
#define CANFD_CH_M_TTCAN_ILS_TFEL_Pos           11UL
#define CANFD_CH_M_TTCAN_ILS_TFEL_Msk           0x800UL
#define CANFD_CH_M_TTCAN_ILS_TEFNL_Pos          12UL
#define CANFD_CH_M_TTCAN_ILS_TEFNL_Msk          0x1000UL
#define CANFD_CH_M_TTCAN_ILS_TEFWL_Pos          13UL
#define CANFD_CH_M_TTCAN_ILS_TEFWL_Msk          0x2000UL
#define CANFD_CH_M_TTCAN_ILS_TEFFL_Pos          14UL
#define CANFD_CH_M_TTCAN_ILS_TEFFL_Msk          0x4000UL
#define CANFD_CH_M_TTCAN_ILS_TEFLL_Pos          15UL
#define CANFD_CH_M_TTCAN_ILS_TEFLL_Msk          0x8000UL
#define CANFD_CH_M_TTCAN_ILS_TSWL_Pos           16UL
#define CANFD_CH_M_TTCAN_ILS_TSWL_Msk           0x10000UL
#define CANFD_CH_M_TTCAN_ILS_MRAFL_Pos          17UL
#define CANFD_CH_M_TTCAN_ILS_MRAFL_Msk          0x20000UL
#define CANFD_CH_M_TTCAN_ILS_TOOL_Pos           18UL
#define CANFD_CH_M_TTCAN_ILS_TOOL_Msk           0x40000UL
#define CANFD_CH_M_TTCAN_ILS_DRXL_Pos           19UL
#define CANFD_CH_M_TTCAN_ILS_DRXL_Msk           0x80000UL
#define CANFD_CH_M_TTCAN_ILS_BECL_Pos           20UL
#define CANFD_CH_M_TTCAN_ILS_BECL_Msk           0x100000UL
#define CANFD_CH_M_TTCAN_ILS_BEUL_Pos           21UL
#define CANFD_CH_M_TTCAN_ILS_BEUL_Msk           0x200000UL
#define CANFD_CH_M_TTCAN_ILS_ELOL_Pos           22UL
#define CANFD_CH_M_TTCAN_ILS_ELOL_Msk           0x400000UL
#define CANFD_CH_M_TTCAN_ILS_EPL_Pos            23UL
#define CANFD_CH_M_TTCAN_ILS_EPL_Msk            0x800000UL
#define CANFD_CH_M_TTCAN_ILS_EWL_Pos            24UL
#define CANFD_CH_M_TTCAN_ILS_EWL_Msk            0x1000000UL
#define CANFD_CH_M_TTCAN_ILS_BOL_Pos            25UL
#define CANFD_CH_M_TTCAN_ILS_BOL_Msk            0x2000000UL
#define CANFD_CH_M_TTCAN_ILS_WDIL_Pos           26UL
#define CANFD_CH_M_TTCAN_ILS_WDIL_Msk           0x4000000UL
#define CANFD_CH_M_TTCAN_ILS_PEAL_Pos           27UL
#define CANFD_CH_M_TTCAN_ILS_PEAL_Msk           0x8000000UL
#define CANFD_CH_M_TTCAN_ILS_PEDL_Pos           28UL
#define CANFD_CH_M_TTCAN_ILS_PEDL_Msk           0x10000000UL
#define CANFD_CH_M_TTCAN_ILS_ARAL_Pos           29UL
#define CANFD_CH_M_TTCAN_ILS_ARAL_Msk           0x20000000UL
/* CANFD_CH_M_TTCAN.ILE */
#define CANFD_CH_M_TTCAN_ILE_EINT0_Pos          0UL
#define CANFD_CH_M_TTCAN_ILE_EINT0_Msk          0x1UL
#define CANFD_CH_M_TTCAN_ILE_EINT1_Pos          1UL
#define CANFD_CH_M_TTCAN_ILE_EINT1_Msk          0x2UL
/* CANFD_CH_M_TTCAN.GFC */
#define CANFD_CH_M_TTCAN_GFC_RRFE_Pos           0UL
#define CANFD_CH_M_TTCAN_GFC_RRFE_Msk           0x1UL
#define CANFD_CH_M_TTCAN_GFC_RRFS_Pos           1UL
#define CANFD_CH_M_TTCAN_GFC_RRFS_Msk           0x2UL
#define CANFD_CH_M_TTCAN_GFC_ANFE_Pos           2UL
#define CANFD_CH_M_TTCAN_GFC_ANFE_Msk           0xCUL
#define CANFD_CH_M_TTCAN_GFC_ANFS_Pos           4UL
#define CANFD_CH_M_TTCAN_GFC_ANFS_Msk           0x30UL
/* CANFD_CH_M_TTCAN.SIDFC */
#define CANFD_CH_M_TTCAN_SIDFC_FLSSA_Pos        2UL
#define CANFD_CH_M_TTCAN_SIDFC_FLSSA_Msk        0xFFFCUL
#define CANFD_CH_M_TTCAN_SIDFC_LSS_Pos          16UL
#define CANFD_CH_M_TTCAN_SIDFC_LSS_Msk          0xFF0000UL
/* CANFD_CH_M_TTCAN.XIDFC */
#define CANFD_CH_M_TTCAN_XIDFC_FLESA_Pos        2UL
#define CANFD_CH_M_TTCAN_XIDFC_FLESA_Msk        0xFFFCUL
#define CANFD_CH_M_TTCAN_XIDFC_LSE_Pos          16UL
#define CANFD_CH_M_TTCAN_XIDFC_LSE_Msk          0x7F0000UL
/* CANFD_CH_M_TTCAN.XIDAM */
#define CANFD_CH_M_TTCAN_XIDAM_EIDM_Pos         0UL
#define CANFD_CH_M_TTCAN_XIDAM_EIDM_Msk         0x1FFFFFFFUL
/* CANFD_CH_M_TTCAN.HPMS */
#define CANFD_CH_M_TTCAN_HPMS_BIDX_Pos          0UL
#define CANFD_CH_M_TTCAN_HPMS_BIDX_Msk          0x3FUL
#define CANFD_CH_M_TTCAN_HPMS_MSI_Pos           6UL
#define CANFD_CH_M_TTCAN_HPMS_MSI_Msk           0xC0UL
#define CANFD_CH_M_TTCAN_HPMS_FIDX_Pos          8UL
#define CANFD_CH_M_TTCAN_HPMS_FIDX_Msk          0x7F00UL
#define CANFD_CH_M_TTCAN_HPMS_FLST_Pos          15UL
#define CANFD_CH_M_TTCAN_HPMS_FLST_Msk          0x8000UL
/* CANFD_CH_M_TTCAN.NDAT1 */
#define CANFD_CH_M_TTCAN_NDAT1_ND_Pos           0UL
#define CANFD_CH_M_TTCAN_NDAT1_ND_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.NDAT2 */
#define CANFD_CH_M_TTCAN_NDAT2_ND_Pos           0UL
#define CANFD_CH_M_TTCAN_NDAT2_ND_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.RXF0C */
#define CANFD_CH_M_TTCAN_RXF0C_F0SA_Pos         2UL
#define CANFD_CH_M_TTCAN_RXF0C_F0SA_Msk         0xFFFCUL
#define CANFD_CH_M_TTCAN_RXF0C_F0S_Pos          16UL
#define CANFD_CH_M_TTCAN_RXF0C_F0S_Msk          0x7F0000UL
#define CANFD_CH_M_TTCAN_RXF0C_F0WM_Pos         24UL
#define CANFD_CH_M_TTCAN_RXF0C_F0WM_Msk         0x7F000000UL
#define CANFD_CH_M_TTCAN_RXF0C_F0OM_Pos         31UL
#define CANFD_CH_M_TTCAN_RXF0C_F0OM_Msk         0x80000000UL
/* CANFD_CH_M_TTCAN.RXF0S */
#define CANFD_CH_M_TTCAN_RXF0S_F0FL_Pos         0UL
#define CANFD_CH_M_TTCAN_RXF0S_F0FL_Msk         0x7FUL
#define CANFD_CH_M_TTCAN_RXF0S_F0GI_Pos         8UL
#define CANFD_CH_M_TTCAN_RXF0S_F0GI_Msk         0x3F00UL
#define CANFD_CH_M_TTCAN_RXF0S_F0PI_Pos         16UL
#define CANFD_CH_M_TTCAN_RXF0S_F0PI_Msk         0x3F0000UL
#define CANFD_CH_M_TTCAN_RXF0S_F0F_Pos          24UL
#define CANFD_CH_M_TTCAN_RXF0S_F0F_Msk          0x1000000UL
#define CANFD_CH_M_TTCAN_RXF0S_RF0L_Pos         25UL
#define CANFD_CH_M_TTCAN_RXF0S_RF0L_Msk         0x2000000UL
/* CANFD_CH_M_TTCAN.RXF0A */
#define CANFD_CH_M_TTCAN_RXF0A_F0AI_Pos         0UL
#define CANFD_CH_M_TTCAN_RXF0A_F0AI_Msk         0x3FUL
/* CANFD_CH_M_TTCAN.RXBC */
#define CANFD_CH_M_TTCAN_RXBC_RBSA_Pos          2UL
#define CANFD_CH_M_TTCAN_RXBC_RBSA_Msk          0xFFFCUL
/* CANFD_CH_M_TTCAN.RXF1C */
#define CANFD_CH_M_TTCAN_RXF1C_F1SA_Pos         2UL
#define CANFD_CH_M_TTCAN_RXF1C_F1SA_Msk         0xFFFCUL
#define CANFD_CH_M_TTCAN_RXF1C_F1S_Pos          16UL
#define CANFD_CH_M_TTCAN_RXF1C_F1S_Msk          0x7F0000UL
#define CANFD_CH_M_TTCAN_RXF1C_F1WM_Pos         24UL
#define CANFD_CH_M_TTCAN_RXF1C_F1WM_Msk         0x7F000000UL
#define CANFD_CH_M_TTCAN_RXF1C_F1OM_Pos         31UL
#define CANFD_CH_M_TTCAN_RXF1C_F1OM_Msk         0x80000000UL
/* CANFD_CH_M_TTCAN.RXF1S */
#define CANFD_CH_M_TTCAN_RXF1S_F1FL_Pos         0UL
#define CANFD_CH_M_TTCAN_RXF1S_F1FL_Msk         0x7FUL
#define CANFD_CH_M_TTCAN_RXF1S_F1GI_Pos         8UL
#define CANFD_CH_M_TTCAN_RXF1S_F1GI_Msk         0x3F00UL
#define CANFD_CH_M_TTCAN_RXF1S_F1PI_Pos         16UL
#define CANFD_CH_M_TTCAN_RXF1S_F1PI_Msk         0x3F0000UL
#define CANFD_CH_M_TTCAN_RXF1S_F1F_Pos          24UL
#define CANFD_CH_M_TTCAN_RXF1S_F1F_Msk          0x1000000UL
#define CANFD_CH_M_TTCAN_RXF1S_RF1L_Pos         25UL
#define CANFD_CH_M_TTCAN_RXF1S_RF1L_Msk         0x2000000UL
#define CANFD_CH_M_TTCAN_RXF1S_DMS_Pos          30UL
#define CANFD_CH_M_TTCAN_RXF1S_DMS_Msk          0xC0000000UL
/* CANFD_CH_M_TTCAN.RXF1A */
#define CANFD_CH_M_TTCAN_RXF1A_F1AI_Pos         0UL
#define CANFD_CH_M_TTCAN_RXF1A_F1AI_Msk         0x3FUL
/* CANFD_CH_M_TTCAN.RXESC */
#define CANFD_CH_M_TTCAN_RXESC_F0DS_Pos         0UL
#define CANFD_CH_M_TTCAN_RXESC_F0DS_Msk         0x7UL
#define CANFD_CH_M_TTCAN_RXESC_F1DS_Pos         4UL
#define CANFD_CH_M_TTCAN_RXESC_F1DS_Msk         0x70UL
#define CANFD_CH_M_TTCAN_RXESC_RBDS_Pos         8UL
#define CANFD_CH_M_TTCAN_RXESC_RBDS_Msk         0x700UL
/* CANFD_CH_M_TTCAN.TXBC */
#define CANFD_CH_M_TTCAN_TXBC_TBSA_Pos          2UL
#define CANFD_CH_M_TTCAN_TXBC_TBSA_Msk          0xFFFCUL
#define CANFD_CH_M_TTCAN_TXBC_NDTB_Pos          16UL
#define CANFD_CH_M_TTCAN_TXBC_NDTB_Msk          0x3F0000UL
#define CANFD_CH_M_TTCAN_TXBC_TFQS_Pos          24UL
#define CANFD_CH_M_TTCAN_TXBC_TFQS_Msk          0x3F000000UL
#define CANFD_CH_M_TTCAN_TXBC_TFQM_Pos          30UL
#define CANFD_CH_M_TTCAN_TXBC_TFQM_Msk          0x40000000UL
/* CANFD_CH_M_TTCAN.TXFQS */
#define CANFD_CH_M_TTCAN_TXFQS_TFFL_Pos         0UL
#define CANFD_CH_M_TTCAN_TXFQS_TFFL_Msk         0x3FUL
#define CANFD_CH_M_TTCAN_TXFQS_TFGI_Pos         8UL
#define CANFD_CH_M_TTCAN_TXFQS_TFGI_Msk         0x1F00UL
#define CANFD_CH_M_TTCAN_TXFQS_TFQPI_Pos        16UL
#define CANFD_CH_M_TTCAN_TXFQS_TFQPI_Msk        0x1F0000UL
#define CANFD_CH_M_TTCAN_TXFQS_TFQF_Pos         21UL
#define CANFD_CH_M_TTCAN_TXFQS_TFQF_Msk         0x200000UL
/* CANFD_CH_M_TTCAN.TXESC */
#define CANFD_CH_M_TTCAN_TXESC_TBDS_Pos         0UL
#define CANFD_CH_M_TTCAN_TXESC_TBDS_Msk         0x7UL
/* CANFD_CH_M_TTCAN.TXBRP */
#define CANFD_CH_M_TTCAN_TXBRP_TRP_Pos          0UL
#define CANFD_CH_M_TTCAN_TXBRP_TRP_Msk          0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBAR */
#define CANFD_CH_M_TTCAN_TXBAR_AR_Pos           0UL
#define CANFD_CH_M_TTCAN_TXBAR_AR_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBCR */
#define CANFD_CH_M_TTCAN_TXBCR_CR_Pos           0UL
#define CANFD_CH_M_TTCAN_TXBCR_CR_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBTO */
#define CANFD_CH_M_TTCAN_TXBTO_TO_Pos           0UL
#define CANFD_CH_M_TTCAN_TXBTO_TO_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBCF */
#define CANFD_CH_M_TTCAN_TXBCF_CF_Pos           0UL
#define CANFD_CH_M_TTCAN_TXBCF_CF_Msk           0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBTIE */
#define CANFD_CH_M_TTCAN_TXBTIE_TIE_Pos         0UL
#define CANFD_CH_M_TTCAN_TXBTIE_TIE_Msk         0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXBCIE */
#define CANFD_CH_M_TTCAN_TXBCIE_CFIE_Pos        0UL
#define CANFD_CH_M_TTCAN_TXBCIE_CFIE_Msk        0xFFFFFFFFUL
/* CANFD_CH_M_TTCAN.TXEFC */
#define CANFD_CH_M_TTCAN_TXEFC_EFSA_Pos         2UL
#define CANFD_CH_M_TTCAN_TXEFC_EFSA_Msk         0xFFFCUL
#define CANFD_CH_M_TTCAN_TXEFC_EFS_Pos          16UL
#define CANFD_CH_M_TTCAN_TXEFC_EFS_Msk          0x3F0000UL
#define CANFD_CH_M_TTCAN_TXEFC_EFWM_Pos         24UL
#define CANFD_CH_M_TTCAN_TXEFC_EFWM_Msk         0x3F000000UL
/* CANFD_CH_M_TTCAN.TXEFS */
#define CANFD_CH_M_TTCAN_TXEFS_EFFL_Pos         0UL
#define CANFD_CH_M_TTCAN_TXEFS_EFFL_Msk         0x3FUL
#define CANFD_CH_M_TTCAN_TXEFS_EFGI_Pos         8UL
#define CANFD_CH_M_TTCAN_TXEFS_EFGI_Msk         0x1F00UL
#define CANFD_CH_M_TTCAN_TXEFS_EFPI_Pos         16UL
#define CANFD_CH_M_TTCAN_TXEFS_EFPI_Msk         0x1F0000UL
#define CANFD_CH_M_TTCAN_TXEFS_EFF_Pos          24UL
#define CANFD_CH_M_TTCAN_TXEFS_EFF_Msk          0x1000000UL
#define CANFD_CH_M_TTCAN_TXEFS_TEFL_Pos         25UL
#define CANFD_CH_M_TTCAN_TXEFS_TEFL_Msk         0x2000000UL
/* CANFD_CH_M_TTCAN.TXEFA */
#define CANFD_CH_M_TTCAN_TXEFA_EFAI_Pos         0UL
#define CANFD_CH_M_TTCAN_TXEFA_EFAI_Msk         0x1FUL
/* CANFD_CH_M_TTCAN.TTTMC */
#define CANFD_CH_M_TTCAN_TTTMC_TMSA_Pos         2UL
#define CANFD_CH_M_TTCAN_TTTMC_TMSA_Msk         0xFFFCUL
#define CANFD_CH_M_TTCAN_TTTMC_TME_Pos          16UL
#define CANFD_CH_M_TTCAN_TTTMC_TME_Msk          0x7F0000UL
/* CANFD_CH_M_TTCAN.TTRMC */
#define CANFD_CH_M_TTCAN_TTRMC_RID_Pos          0UL
#define CANFD_CH_M_TTCAN_TTRMC_RID_Msk          0x1FFFFFFFUL
#define CANFD_CH_M_TTCAN_TTRMC_XTD_Pos          30UL
#define CANFD_CH_M_TTCAN_TTRMC_XTD_Msk          0x40000000UL
#define CANFD_CH_M_TTCAN_TTRMC_RMPS_Pos         31UL
#define CANFD_CH_M_TTCAN_TTRMC_RMPS_Msk         0x80000000UL
/* CANFD_CH_M_TTCAN.TTOCF */
#define CANFD_CH_M_TTCAN_TTOCF_OM_Pos           0UL
#define CANFD_CH_M_TTCAN_TTOCF_OM_Msk           0x3UL
#define CANFD_CH_M_TTCAN_TTOCF_GEN_Pos          3UL
#define CANFD_CH_M_TTCAN_TTOCF_GEN_Msk          0x8UL
#define CANFD_CH_M_TTCAN_TTOCF_TM_Pos           4UL
#define CANFD_CH_M_TTCAN_TTOCF_TM_Msk           0x10UL
#define CANFD_CH_M_TTCAN_TTOCF_LDSDL_Pos        5UL
#define CANFD_CH_M_TTCAN_TTOCF_LDSDL_Msk        0xE0UL
#define CANFD_CH_M_TTCAN_TTOCF_IRTO_Pos         8UL
#define CANFD_CH_M_TTCAN_TTOCF_IRTO_Msk         0x7F00UL
#define CANFD_CH_M_TTCAN_TTOCF_EECS_Pos         15UL
#define CANFD_CH_M_TTCAN_TTOCF_EECS_Msk         0x8000UL
#define CANFD_CH_M_TTCAN_TTOCF_AWL_Pos          16UL
#define CANFD_CH_M_TTCAN_TTOCF_AWL_Msk          0xFF0000UL
#define CANFD_CH_M_TTCAN_TTOCF_EGTF_Pos         24UL
#define CANFD_CH_M_TTCAN_TTOCF_EGTF_Msk         0x1000000UL
#define CANFD_CH_M_TTCAN_TTOCF_ECC_Pos          25UL
#define CANFD_CH_M_TTCAN_TTOCF_ECC_Msk          0x2000000UL
#define CANFD_CH_M_TTCAN_TTOCF_EVTP_Pos         26UL
#define CANFD_CH_M_TTCAN_TTOCF_EVTP_Msk         0x4000000UL
/* CANFD_CH_M_TTCAN.TTMLM */
#define CANFD_CH_M_TTCAN_TTMLM_CCM_Pos          0UL
#define CANFD_CH_M_TTCAN_TTMLM_CCM_Msk          0x3FUL
#define CANFD_CH_M_TTCAN_TTMLM_CSS_Pos          6UL
#define CANFD_CH_M_TTCAN_TTMLM_CSS_Msk          0xC0UL
#define CANFD_CH_M_TTCAN_TTMLM_TXEW_Pos         8UL
#define CANFD_CH_M_TTCAN_TTMLM_TXEW_Msk         0xF00UL
#define CANFD_CH_M_TTCAN_TTMLM_ENTT_Pos         16UL
#define CANFD_CH_M_TTCAN_TTMLM_ENTT_Msk         0xFFF0000UL
/* CANFD_CH_M_TTCAN.TURCF */
#define CANFD_CH_M_TTCAN_TURCF_NCL_Pos          0UL
#define CANFD_CH_M_TTCAN_TURCF_NCL_Msk          0xFFFFUL
#define CANFD_CH_M_TTCAN_TURCF_DC_Pos           16UL
#define CANFD_CH_M_TTCAN_TURCF_DC_Msk           0x3FFF0000UL
#define CANFD_CH_M_TTCAN_TURCF_ELT_Pos          31UL
#define CANFD_CH_M_TTCAN_TURCF_ELT_Msk          0x80000000UL
/* CANFD_CH_M_TTCAN.TTOCN */
#define CANFD_CH_M_TTCAN_TTOCN_SGT_Pos          0UL
#define CANFD_CH_M_TTCAN_TTOCN_SGT_Msk          0x1UL
#define CANFD_CH_M_TTCAN_TTOCN_ECS_Pos          1UL
#define CANFD_CH_M_TTCAN_TTOCN_ECS_Msk          0x2UL
#define CANFD_CH_M_TTCAN_TTOCN_SWP_Pos          2UL
#define CANFD_CH_M_TTCAN_TTOCN_SWP_Msk          0x4UL
#define CANFD_CH_M_TTCAN_TTOCN_SWS_Pos          3UL
#define CANFD_CH_M_TTCAN_TTOCN_SWS_Msk          0x18UL
#define CANFD_CH_M_TTCAN_TTOCN_RTIE_Pos         5UL
#define CANFD_CH_M_TTCAN_TTOCN_RTIE_Msk         0x20UL
#define CANFD_CH_M_TTCAN_TTOCN_TMC_Pos          6UL
#define CANFD_CH_M_TTCAN_TTOCN_TMC_Msk          0xC0UL
#define CANFD_CH_M_TTCAN_TTOCN_TTIE_Pos         8UL
#define CANFD_CH_M_TTCAN_TTOCN_TTIE_Msk         0x100UL
#define CANFD_CH_M_TTCAN_TTOCN_GCS_Pos          9UL
#define CANFD_CH_M_TTCAN_TTOCN_GCS_Msk          0x200UL
#define CANFD_CH_M_TTCAN_TTOCN_FGP_Pos          10UL
#define CANFD_CH_M_TTCAN_TTOCN_FGP_Msk          0x400UL
#define CANFD_CH_M_TTCAN_TTOCN_TMG_Pos          11UL
#define CANFD_CH_M_TTCAN_TTOCN_TMG_Msk          0x800UL
#define CANFD_CH_M_TTCAN_TTOCN_NIG_Pos          12UL
#define CANFD_CH_M_TTCAN_TTOCN_NIG_Msk          0x1000UL
#define CANFD_CH_M_TTCAN_TTOCN_ESCN_Pos         13UL
#define CANFD_CH_M_TTCAN_TTOCN_ESCN_Msk         0x2000UL
#define CANFD_CH_M_TTCAN_TTOCN_LCKC_Pos         15UL
#define CANFD_CH_M_TTCAN_TTOCN_LCKC_Msk         0x8000UL
/* CANFD_CH_M_TTCAN.TTGTP */
#define CANFD_CH_M_TTCAN_TTGTP_TP_Pos           0UL
#define CANFD_CH_M_TTCAN_TTGTP_TP_Msk           0xFFFFUL
#define CANFD_CH_M_TTCAN_TTGTP_CTP_Pos          16UL
#define CANFD_CH_M_TTCAN_TTGTP_CTP_Msk          0xFFFF0000UL
/* CANFD_CH_M_TTCAN.TTTMK */
#define CANFD_CH_M_TTCAN_TTTMK_TM__Pos          0UL
#define CANFD_CH_M_TTCAN_TTTMK_TM__Msk          0xFFFFUL
#define CANFD_CH_M_TTCAN_TTTMK_TICC_Pos         16UL
#define CANFD_CH_M_TTCAN_TTTMK_TICC_Msk         0x7F0000UL
#define CANFD_CH_M_TTCAN_TTTMK_LCKM_Pos         31UL
#define CANFD_CH_M_TTCAN_TTTMK_LCKM_Msk         0x80000000UL
/* CANFD_CH_M_TTCAN.TTIR */
#define CANFD_CH_M_TTCAN_TTIR_SBC_Pos           0UL
#define CANFD_CH_M_TTCAN_TTIR_SBC_Msk           0x1UL
#define CANFD_CH_M_TTCAN_TTIR_SMC_Pos           1UL
#define CANFD_CH_M_TTCAN_TTIR_SMC_Msk           0x2UL
#define CANFD_CH_M_TTCAN_TTIR_CSM__Pos          2UL
#define CANFD_CH_M_TTCAN_TTIR_CSM__Msk          0x4UL
#define CANFD_CH_M_TTCAN_TTIR_SOG_Pos           3UL
#define CANFD_CH_M_TTCAN_TTIR_SOG_Msk           0x8UL
#define CANFD_CH_M_TTCAN_TTIR_RTMI_Pos          4UL
#define CANFD_CH_M_TTCAN_TTIR_RTMI_Msk          0x10UL
#define CANFD_CH_M_TTCAN_TTIR_TTMI_Pos          5UL
#define CANFD_CH_M_TTCAN_TTIR_TTMI_Msk          0x20UL
#define CANFD_CH_M_TTCAN_TTIR_SWE_Pos           6UL
#define CANFD_CH_M_TTCAN_TTIR_SWE_Msk           0x40UL
#define CANFD_CH_M_TTCAN_TTIR_GTW_Pos           7UL
#define CANFD_CH_M_TTCAN_TTIR_GTW_Msk           0x80UL
#define CANFD_CH_M_TTCAN_TTIR_GTD_Pos           8UL
#define CANFD_CH_M_TTCAN_TTIR_GTD_Msk           0x100UL
#define CANFD_CH_M_TTCAN_TTIR_GTE_Pos           9UL
#define CANFD_CH_M_TTCAN_TTIR_GTE_Msk           0x200UL
#define CANFD_CH_M_TTCAN_TTIR_TXU_Pos           10UL
#define CANFD_CH_M_TTCAN_TTIR_TXU_Msk           0x400UL
#define CANFD_CH_M_TTCAN_TTIR_TXO_Pos           11UL
#define CANFD_CH_M_TTCAN_TTIR_TXO_Msk           0x800UL
#define CANFD_CH_M_TTCAN_TTIR_SE1_Pos           12UL
#define CANFD_CH_M_TTCAN_TTIR_SE1_Msk           0x1000UL
#define CANFD_CH_M_TTCAN_TTIR_SE2_Pos           13UL
#define CANFD_CH_M_TTCAN_TTIR_SE2_Msk           0x2000UL
#define CANFD_CH_M_TTCAN_TTIR_ELC_Pos           14UL
#define CANFD_CH_M_TTCAN_TTIR_ELC_Msk           0x4000UL
#define CANFD_CH_M_TTCAN_TTIR_IWT_Pos           15UL
#define CANFD_CH_M_TTCAN_TTIR_IWT_Msk           0x8000UL
#define CANFD_CH_M_TTCAN_TTIR_WT_Pos            16UL
#define CANFD_CH_M_TTCAN_TTIR_WT_Msk            0x10000UL
#define CANFD_CH_M_TTCAN_TTIR_AW_Pos            17UL
#define CANFD_CH_M_TTCAN_TTIR_AW_Msk            0x20000UL
#define CANFD_CH_M_TTCAN_TTIR_CER_Pos           18UL
#define CANFD_CH_M_TTCAN_TTIR_CER_Msk           0x40000UL
/* CANFD_CH_M_TTCAN.TTIE */
#define CANFD_CH_M_TTCAN_TTIE_SBCE_Pos          0UL
#define CANFD_CH_M_TTCAN_TTIE_SBCE_Msk          0x1UL
#define CANFD_CH_M_TTCAN_TTIE_SMCE_Pos          1UL
#define CANFD_CH_M_TTCAN_TTIE_SMCE_Msk          0x2UL
#define CANFD_CH_M_TTCAN_TTIE_CSME_Pos          2UL
#define CANFD_CH_M_TTCAN_TTIE_CSME_Msk          0x4UL
#define CANFD_CH_M_TTCAN_TTIE_SOGE_Pos          3UL
#define CANFD_CH_M_TTCAN_TTIE_SOGE_Msk          0x8UL
#define CANFD_CH_M_TTCAN_TTIE_RTMIE_Pos         4UL
#define CANFD_CH_M_TTCAN_TTIE_RTMIE_Msk         0x10UL
#define CANFD_CH_M_TTCAN_TTIE_TTMIE_Pos         5UL
#define CANFD_CH_M_TTCAN_TTIE_TTMIE_Msk         0x20UL
#define CANFD_CH_M_TTCAN_TTIE_SWEE_Pos          6UL
#define CANFD_CH_M_TTCAN_TTIE_SWEE_Msk          0x40UL
#define CANFD_CH_M_TTCAN_TTIE_GTWE_Pos          7UL
#define CANFD_CH_M_TTCAN_TTIE_GTWE_Msk          0x80UL
#define CANFD_CH_M_TTCAN_TTIE_GTDE_Pos          8UL
#define CANFD_CH_M_TTCAN_TTIE_GTDE_Msk          0x100UL
#define CANFD_CH_M_TTCAN_TTIE_GTEE_Pos          9UL
#define CANFD_CH_M_TTCAN_TTIE_GTEE_Msk          0x200UL
#define CANFD_CH_M_TTCAN_TTIE_TXUE_Pos          10UL
#define CANFD_CH_M_TTCAN_TTIE_TXUE_Msk          0x400UL
#define CANFD_CH_M_TTCAN_TTIE_TXOE_Pos          11UL
#define CANFD_CH_M_TTCAN_TTIE_TXOE_Msk          0x800UL
#define CANFD_CH_M_TTCAN_TTIE_SE1E_Pos          12UL
#define CANFD_CH_M_TTCAN_TTIE_SE1E_Msk          0x1000UL
#define CANFD_CH_M_TTCAN_TTIE_SE2E_Pos          13UL
#define CANFD_CH_M_TTCAN_TTIE_SE2E_Msk          0x2000UL
#define CANFD_CH_M_TTCAN_TTIE_ELCE_Pos          14UL
#define CANFD_CH_M_TTCAN_TTIE_ELCE_Msk          0x4000UL
#define CANFD_CH_M_TTCAN_TTIE_IWTE_Pos          15UL
#define CANFD_CH_M_TTCAN_TTIE_IWTE_Msk          0x8000UL
#define CANFD_CH_M_TTCAN_TTIE_WTE_Pos           16UL
#define CANFD_CH_M_TTCAN_TTIE_WTE_Msk           0x10000UL
#define CANFD_CH_M_TTCAN_TTIE_AWE__Pos          17UL
#define CANFD_CH_M_TTCAN_TTIE_AWE__Msk          0x20000UL
#define CANFD_CH_M_TTCAN_TTIE_CERE_Pos          18UL
#define CANFD_CH_M_TTCAN_TTIE_CERE_Msk          0x40000UL
/* CANFD_CH_M_TTCAN.TTILS */
#define CANFD_CH_M_TTCAN_TTILS_SBCL_Pos         0UL
#define CANFD_CH_M_TTCAN_TTILS_SBCL_Msk         0x1UL
#define CANFD_CH_M_TTCAN_TTILS_SMCL_Pos         1UL
#define CANFD_CH_M_TTCAN_TTILS_SMCL_Msk         0x2UL
#define CANFD_CH_M_TTCAN_TTILS_CSML_Pos         2UL
#define CANFD_CH_M_TTCAN_TTILS_CSML_Msk         0x4UL
#define CANFD_CH_M_TTCAN_TTILS_SOGL_Pos         3UL
#define CANFD_CH_M_TTCAN_TTILS_SOGL_Msk         0x8UL
#define CANFD_CH_M_TTCAN_TTILS_RTMIL_Pos        4UL
#define CANFD_CH_M_TTCAN_TTILS_RTMIL_Msk        0x10UL
#define CANFD_CH_M_TTCAN_TTILS_TTMIL_Pos        5UL
#define CANFD_CH_M_TTCAN_TTILS_TTMIL_Msk        0x20UL
#define CANFD_CH_M_TTCAN_TTILS_SWEL_Pos         6UL
#define CANFD_CH_M_TTCAN_TTILS_SWEL_Msk         0x40UL
#define CANFD_CH_M_TTCAN_TTILS_GTWL_Pos         7UL
#define CANFD_CH_M_TTCAN_TTILS_GTWL_Msk         0x80UL
#define CANFD_CH_M_TTCAN_TTILS_GTDL_Pos         8UL
#define CANFD_CH_M_TTCAN_TTILS_GTDL_Msk         0x100UL
#define CANFD_CH_M_TTCAN_TTILS_GTEL_Pos         9UL
#define CANFD_CH_M_TTCAN_TTILS_GTEL_Msk         0x200UL
#define CANFD_CH_M_TTCAN_TTILS_TXUL_Pos         10UL
#define CANFD_CH_M_TTCAN_TTILS_TXUL_Msk         0x400UL
#define CANFD_CH_M_TTCAN_TTILS_TXOL_Pos         11UL
#define CANFD_CH_M_TTCAN_TTILS_TXOL_Msk         0x800UL
#define CANFD_CH_M_TTCAN_TTILS_SE1L_Pos         12UL
#define CANFD_CH_M_TTCAN_TTILS_SE1L_Msk         0x1000UL
#define CANFD_CH_M_TTCAN_TTILS_SE2L_Pos         13UL
#define CANFD_CH_M_TTCAN_TTILS_SE2L_Msk         0x2000UL
#define CANFD_CH_M_TTCAN_TTILS_ELCL_Pos         14UL
#define CANFD_CH_M_TTCAN_TTILS_ELCL_Msk         0x4000UL
#define CANFD_CH_M_TTCAN_TTILS_IWTL_Pos         15UL
#define CANFD_CH_M_TTCAN_TTILS_IWTL_Msk         0x8000UL
#define CANFD_CH_M_TTCAN_TTILS_WTL_Pos          16UL
#define CANFD_CH_M_TTCAN_TTILS_WTL_Msk          0x10000UL
#define CANFD_CH_M_TTCAN_TTILS_AWL__Pos         17UL
#define CANFD_CH_M_TTCAN_TTILS_AWL__Msk         0x20000UL
#define CANFD_CH_M_TTCAN_TTILS_CERL_Pos         18UL
#define CANFD_CH_M_TTCAN_TTILS_CERL_Msk         0x40000UL
/* CANFD_CH_M_TTCAN.TTOST */
#define CANFD_CH_M_TTCAN_TTOST_EL_Pos           0UL
#define CANFD_CH_M_TTCAN_TTOST_EL_Msk           0x3UL
#define CANFD_CH_M_TTCAN_TTOST_MS_Pos           2UL
#define CANFD_CH_M_TTCAN_TTOST_MS_Msk           0xCUL
#define CANFD_CH_M_TTCAN_TTOST_SYS_Pos          4UL
#define CANFD_CH_M_TTCAN_TTOST_SYS_Msk          0x30UL
#define CANFD_CH_M_TTCAN_TTOST_QGTP_Pos         6UL
#define CANFD_CH_M_TTCAN_TTOST_QGTP_Msk         0x40UL
#define CANFD_CH_M_TTCAN_TTOST_QCS_Pos          7UL
#define CANFD_CH_M_TTCAN_TTOST_QCS_Msk          0x80UL
#define CANFD_CH_M_TTCAN_TTOST_RTO_Pos          8UL
#define CANFD_CH_M_TTCAN_TTOST_RTO_Msk          0xFF00UL
#define CANFD_CH_M_TTCAN_TTOST_WGTD_Pos         22UL
#define CANFD_CH_M_TTCAN_TTOST_WGTD_Msk         0x400000UL
#define CANFD_CH_M_TTCAN_TTOST_GFI_Pos          23UL
#define CANFD_CH_M_TTCAN_TTOST_GFI_Msk          0x800000UL
#define CANFD_CH_M_TTCAN_TTOST_TMP_Pos          24UL
#define CANFD_CH_M_TTCAN_TTOST_TMP_Msk          0x7000000UL
#define CANFD_CH_M_TTCAN_TTOST_GSI_Pos          27UL
#define CANFD_CH_M_TTCAN_TTOST_GSI_Msk          0x8000000UL
#define CANFD_CH_M_TTCAN_TTOST_WFE_Pos          28UL
#define CANFD_CH_M_TTCAN_TTOST_WFE_Msk          0x10000000UL
#define CANFD_CH_M_TTCAN_TTOST_AWE_Pos          29UL
#define CANFD_CH_M_TTCAN_TTOST_AWE_Msk          0x20000000UL
#define CANFD_CH_M_TTCAN_TTOST_WECS_Pos         30UL
#define CANFD_CH_M_TTCAN_TTOST_WECS_Msk         0x40000000UL
#define CANFD_CH_M_TTCAN_TTOST_SPL_Pos          31UL
#define CANFD_CH_M_TTCAN_TTOST_SPL_Msk          0x80000000UL
/* CANFD_CH_M_TTCAN.TURNA */
#define CANFD_CH_M_TTCAN_TURNA_NAV_Pos          0UL
#define CANFD_CH_M_TTCAN_TURNA_NAV_Msk          0x3FFFFUL
/* CANFD_CH_M_TTCAN.TTLGT */
#define CANFD_CH_M_TTCAN_TTLGT_LT_Pos           0UL
#define CANFD_CH_M_TTCAN_TTLGT_LT_Msk           0xFFFFUL
#define CANFD_CH_M_TTCAN_TTLGT_GT_Pos           16UL
#define CANFD_CH_M_TTCAN_TTLGT_GT_Msk           0xFFFF0000UL
/* CANFD_CH_M_TTCAN.TTCTC */
#define CANFD_CH_M_TTCAN_TTCTC_CT_Pos           0UL
#define CANFD_CH_M_TTCAN_TTCTC_CT_Msk           0xFFFFUL
#define CANFD_CH_M_TTCAN_TTCTC_CC_Pos           16UL
#define CANFD_CH_M_TTCAN_TTCTC_CC_Msk           0x3F0000UL
/* CANFD_CH_M_TTCAN.TTCPT */
#define CANFD_CH_M_TTCAN_TTCPT_CCV_Pos          0UL
#define CANFD_CH_M_TTCAN_TTCPT_CCV_Msk          0x3FUL
#define CANFD_CH_M_TTCAN_TTCPT_SWV_Pos          16UL
#define CANFD_CH_M_TTCAN_TTCPT_SWV_Msk          0xFFFF0000UL
/* CANFD_CH_M_TTCAN.TTCSM */
#define CANFD_CH_M_TTCAN_TTCSM_CSM_Pos          0UL
#define CANFD_CH_M_TTCAN_TTCSM_CSM_Msk          0xFFFFUL


/* CANFD_CH.RXFTOP_CTL */
#define CANFD_CH_RXFTOP_CTL_F0TPE_Pos           0UL
#define CANFD_CH_RXFTOP_CTL_F0TPE_Msk           0x1UL
#define CANFD_CH_RXFTOP_CTL_F1TPE_Pos           1UL
#define CANFD_CH_RXFTOP_CTL_F1TPE_Msk           0x2UL
/* CANFD_CH.RXFTOP0_STAT */
#define CANFD_CH_RXFTOP0_STAT_F0TA_Pos          0UL
#define CANFD_CH_RXFTOP0_STAT_F0TA_Msk          0xFFFFUL
/* CANFD_CH.RXFTOP0_DATA */
#define CANFD_CH_RXFTOP0_DATA_F0TD_Pos          0UL
#define CANFD_CH_RXFTOP0_DATA_F0TD_Msk          0xFFFFFFFFUL
/* CANFD_CH.RXFTOP1_STAT */
#define CANFD_CH_RXFTOP1_STAT_F1TA_Pos          0UL
#define CANFD_CH_RXFTOP1_STAT_F1TA_Msk          0xFFFFUL
/* CANFD_CH.RXFTOP1_DATA */
#define CANFD_CH_RXFTOP1_DATA_F1TD_Pos          0UL
#define CANFD_CH_RXFTOP1_DATA_F1TD_Msk          0xFFFFFFFFUL


/* CANFD.CTL */
#define CANFD_CTL_STOP_REQ_Pos                  0UL
#define CANFD_CTL_STOP_REQ_Msk                  0xFFUL
#define CANFD_CTL_MRAM_OFF_Pos                  31UL
#define CANFD_CTL_MRAM_OFF_Msk                  0x80000000UL
/* CANFD.STATUS */
#define CANFD_STATUS_STOP_ACK_Pos               0UL
#define CANFD_STATUS_STOP_ACK_Msk               0xFFUL
/* CANFD.INTR0_CAUSE */
#define CANFD_INTR0_CAUSE_INT0_Pos              0UL
#define CANFD_INTR0_CAUSE_INT0_Msk              0xFFUL
/* CANFD.INTR1_CAUSE */
#define CANFD_INTR1_CAUSE_INT1_Pos              0UL
#define CANFD_INTR1_CAUSE_INT1_Msk              0xFFUL
/* CANFD.TS_CTL */
#define CANFD_TS_CTL_PRESCALE_Pos               0UL
#define CANFD_TS_CTL_PRESCALE_Msk               0xFFFFUL
#define CANFD_TS_CTL_ENABLED_Pos                31UL
#define CANFD_TS_CTL_ENABLED_Msk                0x80000000UL
/* CANFD.TS_CNT */
#define CANFD_TS_CNT_VALUE_Pos                  0UL
#define CANFD_TS_CNT_VALUE_Msk                  0xFFFFUL
/* CANFD.ECC_CTL */
#define CANFD_ECC_CTL_ECC_EN_Pos                16UL
#define CANFD_ECC_CTL_ECC_EN_Msk                0x10000UL
/* CANFD.ECC_ERR_INJ */
#define CANFD_ECC_ERR_INJ_ERR_ADDR_Pos          2UL
#define CANFD_ECC_ERR_INJ_ERR_ADDR_Msk          0xFFFCUL
#define CANFD_ECC_ERR_INJ_ERR_EN_Pos            20UL
#define CANFD_ECC_ERR_INJ_ERR_EN_Msk            0x100000UL
#define CANFD_ECC_ERR_INJ_ERR_PAR_Pos           24UL
#define CANFD_ECC_ERR_INJ_ERR_PAR_Msk           0x7F000000UL


#endif /* _CYIP_CANFD_H_ */


/* [] END OF FILE */
