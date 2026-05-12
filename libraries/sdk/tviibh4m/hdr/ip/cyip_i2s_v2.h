/***************************************************************************//**
* \file cyip_i2s_v2.h
*
* \brief
* I2S IP definitions
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

#ifndef _CYIP_I2S_V2_H_
#define _CYIP_I2S_V2_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                     I2S
*******************************************************************************/

#define I2S_SECTION_SIZE                        0x00001000UL

/**
  * \brief Control (I2S_CTL)
  */
typedef struct stc_I2S_CTL_field {
  uint32_t                         :30;
  uint32_t                         u1TX_ENABLED:1;
  uint32_t                         u1RX_ENABLED:1;
} stc_I2S_CTL_field_t;

typedef union un_I2S_CTL {
  uint32_t                         u32Register;
  stc_I2S_CTL_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_CTL_t;

/**
  * \brief Clock control (I2S_CLOCK_CTL)
  */
typedef struct stc_I2S_CLOCK_CTL_field {
  uint32_t                         u6CLOCK_DIV:6;
  uint32_t                         :2;
  uint32_t                         u1CLOCK_SEL:1;
  uint32_t                         :3;
  uint32_t                         u2MCLK_DIV:2;
  uint32_t                         :2;
  uint32_t                         u1MCLK_EN:1;
  uint32_t                         :15;
} stc_I2S_CLOCK_CTL_field_t;

typedef union un_I2S_CLOCK_CTL {
  uint32_t                         u32Register;
  stc_I2S_CLOCK_CTL_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_CLOCK_CTL_t;

/**
  * \brief Clock Status (I2S_CLOCK_STAT)
  */
typedef struct stc_I2S_CLOCK_STAT_field {
  uint32_t                         u1MCLK_DIV_OFF:1;
  uint32_t                         :31;
} stc_I2S_CLOCK_STAT_field_t;

typedef union un_I2S_CLOCK_STAT {
  uint32_t                         u32Register;
  stc_I2S_CLOCK_STAT_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_CLOCK_STAT_t;

/**
  * \brief Command (I2S_CMD)
  */
typedef struct stc_I2S_CMD_field {
  uint32_t                         u1TX_START:1;
  uint32_t                         :7;
  uint32_t                         u1TX_PAUSE:1;
  uint32_t                         :7;
  uint32_t                         u1RX_START:1;
  uint32_t                         :15;
} stc_I2S_CMD_field_t;

typedef union un_I2S_CMD {
  uint32_t                         u32Register;
  stc_I2S_CMD_field_t              stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_CMD_t;

/**
  * \brief Trigger control (I2S_TR_CTL)
  */
typedef struct stc_I2S_TR_CTL_field {
  uint32_t                         u1TX_REQ_EN:1;
  uint32_t                         :15;
  uint32_t                         u1RX_REQ_EN:1;
  uint32_t                         :15;
} stc_I2S_TR_CTL_field_t;

typedef union un_I2S_TR_CTL {
  uint32_t                         u32Register;
  stc_I2S_TR_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TR_CTL_t;

/**
  * \brief Transmitter control (I2S_TX_CTL)
  */
typedef struct stc_I2S_TX_CTL_field {
  uint32_t                         :3;
  uint32_t                         u1B_CLOCK_INV:1;
  uint32_t                         u3CH_NR:3;
  uint32_t                         u1MS:1;
  uint32_t                         u2I2S_MODE:2;
  uint32_t                         u1WS_PULSE:1;
  uint32_t                         :1;
  uint32_t                         u1OVHDATA:1;
  uint32_t                         u1WD_EN:1;
  uint32_t                         :2;
  uint32_t                         u3CH_LEN:3;
  uint32_t                         :1;
  uint32_t                         u3WORD_LEN:3;
  uint32_t                         :1;
  uint32_t                         u1SCKO_POL:1;
  uint32_t                         u1SCKI_POL:1;
  uint32_t                         :6;
} stc_I2S_TX_CTL_field_t;

typedef union un_I2S_TX_CTL {
  uint32_t                         u32Register;
  stc_I2S_TX_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TX_CTL_t;

/**
  * \brief Transmitter watchdog (I2S_TX_WATCHDOG)
  */
typedef struct stc_I2S_TX_WATCHDOG_field {
  uint32_t                         u32WD_COUNTER:32;
} stc_I2S_TX_WATCHDOG_field_t;

typedef union un_I2S_TX_WATCHDOG {
  uint32_t                         u32Register;
  stc_I2S_TX_WATCHDOG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TX_WATCHDOG_t;

/**
  * \brief Receiver control (I2S_RX_CTL)
  */
typedef struct stc_I2S_RX_CTL_field {
  uint32_t                         :3;
  uint32_t                         u1B_CLOCK_INV:1;
  uint32_t                         u3CH_NR:3;
  uint32_t                         u1MS:1;
  uint32_t                         u2I2S_MODE:2;
  uint32_t                         u1WS_PULSE:1;
  uint32_t                         :2;
  uint32_t                         u1WD_EN:1;
  uint32_t                         :2;
  uint32_t                         u3CH_LEN:3;
  uint32_t                         :1;
  uint32_t                         u3WORD_LEN:3;
  uint32_t                         u1BIT_EXTENSION:1;
  uint32_t                         u1SCKO_POL:1;
  uint32_t                         u1SCKI_POL:1;
  uint32_t                         :6;
} stc_I2S_RX_CTL_field_t;

typedef union un_I2S_RX_CTL {
  uint32_t                         u32Register;
  stc_I2S_RX_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_CTL_t;

/**
  * \brief Receiver watchdog (I2S_RX_WATCHDOG)
  */
typedef struct stc_I2S_RX_WATCHDOG_field {
  uint32_t                         u32WD_COUNTER:32;
} stc_I2S_RX_WATCHDOG_field_t;

typedef union un_I2S_RX_WATCHDOG {
  uint32_t                         u32Register;
  stc_I2S_RX_WATCHDOG_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_WATCHDOG_t;

/**
  * \brief TX FIFO control (I2S_TX_FIFO_CTL)
  */
typedef struct stc_I2S_TX_FIFO_CTL_field {
  uint32_t                         u8TRIGGER_LEVEL:8;
  uint32_t                         :8;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_I2S_TX_FIFO_CTL_field_t;

typedef union un_I2S_TX_FIFO_CTL {
  uint32_t                         u32Register;
  stc_I2S_TX_FIFO_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TX_FIFO_CTL_t;

/**
  * \brief TX FIFO status (I2S_TX_FIFO_STATUS)
  */
typedef struct stc_I2S_TX_FIFO_STATUS_field {
  uint32_t                         u9USED:9;
  uint32_t                         :7;
  uint32_t                         u8RD_PTR:8;
  uint32_t                         u8WR_PTR:8;
} stc_I2S_TX_FIFO_STATUS_field_t;

typedef union un_I2S_TX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_I2S_TX_FIFO_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TX_FIFO_STATUS_t;

/**
  * \brief TX FIFO write (I2S_TX_FIFO_WR)
  */
typedef struct stc_I2S_TX_FIFO_WR_field {
  uint32_t                         u32DATA:32;
} stc_I2S_TX_FIFO_WR_field_t;

typedef union un_I2S_TX_FIFO_WR {
  uint32_t                         u32Register;
  stc_I2S_TX_FIFO_WR_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_TX_FIFO_WR_t;

/**
  * \brief RX FIFO control (I2S_RX_FIFO_CTL)
  */
typedef struct stc_I2S_RX_FIFO_CTL_field {
  uint32_t                         u8TRIGGER_LEVEL:8;
  uint32_t                         :8;
  uint32_t                         u1CLEAR:1;
  uint32_t                         u1FREEZE:1;
  uint32_t                         :14;
} stc_I2S_RX_FIFO_CTL_field_t;

typedef union un_I2S_RX_FIFO_CTL {
  uint32_t                         u32Register;
  stc_I2S_RX_FIFO_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_FIFO_CTL_t;

/**
  * \brief RX FIFO status (I2S_RX_FIFO_STATUS)
  */
typedef struct stc_I2S_RX_FIFO_STATUS_field {
  uint32_t                         u9USED:9;
  uint32_t                         :7;
  uint32_t                         u8RD_PTR:8;
  uint32_t                         u8WR_PTR:8;
} stc_I2S_RX_FIFO_STATUS_field_t;

typedef union un_I2S_RX_FIFO_STATUS {
  uint32_t                         u32Register;
  stc_I2S_RX_FIFO_STATUS_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_FIFO_STATUS_t;

/**
  * \brief RX FIFO read (I2S_RX_FIFO_RD)
  */
typedef struct stc_I2S_RX_FIFO_RD_field {
  uint32_t                         u32DATA:32;
} stc_I2S_RX_FIFO_RD_field_t;

typedef union un_I2S_RX_FIFO_RD {
  uint32_t                         u32Register;
  stc_I2S_RX_FIFO_RD_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_FIFO_RD_t;

/**
  * \brief RX FIFO silent read (I2S_RX_FIFO_RD_SILENT)
  */
typedef struct stc_I2S_RX_FIFO_RD_SILENT_field {
  uint32_t                         u32DATA:32;
} stc_I2S_RX_FIFO_RD_SILENT_field_t;

typedef union un_I2S_RX_FIFO_RD_SILENT {
  uint32_t                         u32Register;
  stc_I2S_RX_FIFO_RD_SILENT_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_RX_FIFO_RD_SILENT_t;

/**
  * \brief Interrupt register (I2S_INTR)
  */
typedef struct stc_I2S_INTR_field {
  uint32_t                         u1TX_TRIGGER:1;
  uint32_t                         u1TX_NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1TX_EMPTY:1;
  uint32_t                         u1TX_OVERFLOW:1;
  uint32_t                         u1TX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WD:1;
  uint32_t                         :7;
  uint32_t                         u1RX_TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOT_EMPTY:1;
  uint32_t                         u1RX_FULL:1;
  uint32_t                         :1;
  uint32_t                         u1RX_OVERFLOW:1;
  uint32_t                         u1RX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1RX_WD:1;
  uint32_t                         :7;
} stc_I2S_INTR_field_t;

typedef union un_I2S_INTR {
  uint32_t                         u32Register;
  stc_I2S_INTR_field_t             stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_INTR_t;

/**
  * \brief Interrupt set register (I2S_INTR_SET)
  */
typedef struct stc_I2S_INTR_SET_field {
  uint32_t                         u1TX_TRIGGER:1;
  uint32_t                         u1TX_NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1TX_EMPTY:1;
  uint32_t                         u1TX_OVERFLOW:1;
  uint32_t                         u1TX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WD:1;
  uint32_t                         :7;
  uint32_t                         u1RX_TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOT_EMPTY:1;
  uint32_t                         u1RX_FULL:1;
  uint32_t                         :1;
  uint32_t                         u1RX_OVERFLOW:1;
  uint32_t                         u1RX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1RX_WD:1;
  uint32_t                         :7;
} stc_I2S_INTR_SET_field_t;

typedef union un_I2S_INTR_SET {
  uint32_t                         u32Register;
  stc_I2S_INTR_SET_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_INTR_SET_t;

/**
  * \brief Interrupt mask register (I2S_INTR_MASK)
  */
typedef struct stc_I2S_INTR_MASK_field {
  uint32_t                         u1TX_TRIGGER:1;
  uint32_t                         u1TX_NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1TX_EMPTY:1;
  uint32_t                         u1TX_OVERFLOW:1;
  uint32_t                         u1TX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WD:1;
  uint32_t                         :7;
  uint32_t                         u1RX_TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOT_EMPTY:1;
  uint32_t                         u1RX_FULL:1;
  uint32_t                         :1;
  uint32_t                         u1RX_OVERFLOW:1;
  uint32_t                         u1RX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1RX_WD:1;
  uint32_t                         :7;
} stc_I2S_INTR_MASK_field_t;

typedef union un_I2S_INTR_MASK {
  uint32_t                         u32Register;
  stc_I2S_INTR_MASK_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_INTR_MASK_t;

/**
  * \brief Interrupt masked register (I2S_INTR_MASKED)
  */
typedef struct stc_I2S_INTR_MASKED_field {
  uint32_t                         u1TX_TRIGGER:1;
  uint32_t                         u1TX_NOT_FULL:1;
  uint32_t                         :2;
  uint32_t                         u1TX_EMPTY:1;
  uint32_t                         u1TX_OVERFLOW:1;
  uint32_t                         u1TX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1TX_WD:1;
  uint32_t                         :7;
  uint32_t                         u1RX_TRIGGER:1;
  uint32_t                         :1;
  uint32_t                         u1RX_NOT_EMPTY:1;
  uint32_t                         u1RX_FULL:1;
  uint32_t                         :1;
  uint32_t                         u1RX_OVERFLOW:1;
  uint32_t                         u1RX_UNDERFLOW:1;
  uint32_t                         :1;
  uint32_t                         u1RX_WD:1;
  uint32_t                         :7;
} stc_I2S_INTR_MASKED_field_t;

typedef union un_I2S_INTR_MASKED {
  uint32_t                         u32Register;
  stc_I2S_INTR_MASKED_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_I2S_INTR_MASKED_t;



/**
  * \brief I2S registers (I2S)
  */
typedef struct stc_I2S {
  un_I2S_CTL_t                     unCTL;              /*!< 0x00000000 Control */
  uint32_t                         au32Reserved[3];
  un_I2S_CLOCK_CTL_t               unCLOCK_CTL;        /*!< 0x00000010 Clock control */
  un_I2S_CLOCK_STAT_t              unCLOCK_STAT;       /*!< 0x00000014 Clock Status */
  uint32_t                         au32Reserved1[2];
  un_I2S_CMD_t                     unCMD;              /*!< 0x00000020 Command */
  uint32_t                         au32Reserved2[7];
  un_I2S_TR_CTL_t                  unTR_CTL;           /*!< 0x00000040 Trigger control */
  uint32_t                         au32Reserved3[15];
  un_I2S_TX_CTL_t                  unTX_CTL;           /*!< 0x00000080 Transmitter control */
  un_I2S_TX_WATCHDOG_t             unTX_WATCHDOG;      /*!< 0x00000084 Transmitter watchdog */
  uint32_t                         au32Reserved4[6];
  un_I2S_RX_CTL_t                  unRX_CTL;           /*!< 0x000000A0 Receiver control */
  un_I2S_RX_WATCHDOG_t             unRX_WATCHDOG;      /*!< 0x000000A4 Receiver watchdog */
  uint32_t                         au32Reserved5[86];
  un_I2S_TX_FIFO_CTL_t             unTX_FIFO_CTL;      /*!< 0x00000200 TX FIFO control */
  un_I2S_TX_FIFO_STATUS_t          unTX_FIFO_STATUS;   /*!< 0x00000204 TX FIFO status */
  un_I2S_TX_FIFO_WR_t              unTX_FIFO_WR;       /*!< 0x00000208 TX FIFO write */
  uint32_t                         au32Reserved6[61];
  un_I2S_RX_FIFO_CTL_t             unRX_FIFO_CTL;      /*!< 0x00000300 RX FIFO control */
  un_I2S_RX_FIFO_STATUS_t          unRX_FIFO_STATUS;   /*!< 0x00000304 RX FIFO status */
  un_I2S_RX_FIFO_RD_t              unRX_FIFO_RD;       /*!< 0x00000308 RX FIFO read */
  un_I2S_RX_FIFO_RD_SILENT_t       unRX_FIFO_RD_SILENT; /*!< 0x0000030C RX FIFO silent read */
  uint32_t                         au32Reserved7[764];
  un_I2S_INTR_t                    unINTR;             /*!< 0x00000F00 Interrupt register */
  un_I2S_INTR_SET_t                unINTR_SET;         /*!< 0x00000F04 Interrupt set register */
  un_I2S_INTR_MASK_t               unINTR_MASK;        /*!< 0x00000F08 Interrupt mask register */
  un_I2S_INTR_MASKED_t             unINTR_MASKED;      /*!< 0x00000F0C Interrupt masked register */
} stc_I2S_t;                                           /*!< Size = 3856 (0xF10) */


/* I2S.CTL */
#define I2S_CTL_TX_ENABLED_Pos                  30UL
#define I2S_CTL_TX_ENABLED_Msk                  0x40000000UL
#define I2S_CTL_RX_ENABLED_Pos                  31UL
#define I2S_CTL_RX_ENABLED_Msk                  0x80000000UL
/* I2S.CLOCK_CTL */
#define I2S_CLOCK_CTL_CLOCK_DIV_Pos             0UL
#define I2S_CLOCK_CTL_CLOCK_DIV_Msk             0x3FUL
#define I2S_CLOCK_CTL_CLOCK_SEL_Pos             8UL
#define I2S_CLOCK_CTL_CLOCK_SEL_Msk             0x100UL
#define I2S_CLOCK_CTL_MCLK_DIV_Pos              12UL
#define I2S_CLOCK_CTL_MCLK_DIV_Msk              0x3000UL
#define I2S_CLOCK_CTL_MCLK_EN_Pos               16UL
#define I2S_CLOCK_CTL_MCLK_EN_Msk               0x10000UL
/* I2S.CLOCK_STAT */
#define I2S_CLOCK_STAT_MCLK_DIV_OFF_Pos         0UL
#define I2S_CLOCK_STAT_MCLK_DIV_OFF_Msk         0x1UL
/* I2S.CMD */
#define I2S_CMD_TX_START_Pos                    0UL
#define I2S_CMD_TX_START_Msk                    0x1UL
#define I2S_CMD_TX_PAUSE_Pos                    8UL
#define I2S_CMD_TX_PAUSE_Msk                    0x100UL
#define I2S_CMD_RX_START_Pos                    16UL
#define I2S_CMD_RX_START_Msk                    0x10000UL
/* I2S.TR_CTL */
#define I2S_TR_CTL_TX_REQ_EN_Pos                0UL
#define I2S_TR_CTL_TX_REQ_EN_Msk                0x1UL
#define I2S_TR_CTL_RX_REQ_EN_Pos                16UL
#define I2S_TR_CTL_RX_REQ_EN_Msk                0x10000UL
/* I2S.TX_CTL */
#define I2S_TX_CTL_B_CLOCK_INV_Pos              3UL
#define I2S_TX_CTL_B_CLOCK_INV_Msk              0x8UL
#define I2S_TX_CTL_CH_NR_Pos                    4UL
#define I2S_TX_CTL_CH_NR_Msk                    0x70UL
#define I2S_TX_CTL_MS_Pos                       7UL
#define I2S_TX_CTL_MS_Msk                       0x80UL
#define I2S_TX_CTL_I2S_MODE_Pos                 8UL
#define I2S_TX_CTL_I2S_MODE_Msk                 0x300UL
#define I2S_TX_CTL_WS_PULSE_Pos                 10UL
#define I2S_TX_CTL_WS_PULSE_Msk                 0x400UL
#define I2S_TX_CTL_OVHDATA_Pos                  12UL
#define I2S_TX_CTL_OVHDATA_Msk                  0x1000UL
#define I2S_TX_CTL_WD_EN_Pos                    13UL
#define I2S_TX_CTL_WD_EN_Msk                    0x2000UL
#define I2S_TX_CTL_CH_LEN_Pos                   16UL
#define I2S_TX_CTL_CH_LEN_Msk                   0x70000UL
#define I2S_TX_CTL_WORD_LEN_Pos                 20UL
#define I2S_TX_CTL_WORD_LEN_Msk                 0x700000UL
#define I2S_TX_CTL_SCKO_POL_Pos                 24UL
#define I2S_TX_CTL_SCKO_POL_Msk                 0x1000000UL
#define I2S_TX_CTL_SCKI_POL_Pos                 25UL
#define I2S_TX_CTL_SCKI_POL_Msk                 0x2000000UL
/* I2S.TX_WATCHDOG */
#define I2S_TX_WATCHDOG_WD_COUNTER_Pos          0UL
#define I2S_TX_WATCHDOG_WD_COUNTER_Msk          0xFFFFFFFFUL
/* I2S.RX_CTL */
#define I2S_RX_CTL_B_CLOCK_INV_Pos              3UL
#define I2S_RX_CTL_B_CLOCK_INV_Msk              0x8UL
#define I2S_RX_CTL_CH_NR_Pos                    4UL
#define I2S_RX_CTL_CH_NR_Msk                    0x70UL
#define I2S_RX_CTL_MS_Pos                       7UL
#define I2S_RX_CTL_MS_Msk                       0x80UL
#define I2S_RX_CTL_I2S_MODE_Pos                 8UL
#define I2S_RX_CTL_I2S_MODE_Msk                 0x300UL
#define I2S_RX_CTL_WS_PULSE_Pos                 10UL
#define I2S_RX_CTL_WS_PULSE_Msk                 0x400UL
#define I2S_RX_CTL_WD_EN_Pos                    13UL
#define I2S_RX_CTL_WD_EN_Msk                    0x2000UL
#define I2S_RX_CTL_CH_LEN_Pos                   16UL
#define I2S_RX_CTL_CH_LEN_Msk                   0x70000UL
#define I2S_RX_CTL_WORD_LEN_Pos                 20UL
#define I2S_RX_CTL_WORD_LEN_Msk                 0x700000UL
#define I2S_RX_CTL_BIT_EXTENSION_Pos            23UL
#define I2S_RX_CTL_BIT_EXTENSION_Msk            0x800000UL
#define I2S_RX_CTL_SCKO_POL_Pos                 24UL
#define I2S_RX_CTL_SCKO_POL_Msk                 0x1000000UL
#define I2S_RX_CTL_SCKI_POL_Pos                 25UL
#define I2S_RX_CTL_SCKI_POL_Msk                 0x2000000UL
/* I2S.RX_WATCHDOG */
#define I2S_RX_WATCHDOG_WD_COUNTER_Pos          0UL
#define I2S_RX_WATCHDOG_WD_COUNTER_Msk          0xFFFFFFFFUL
/* I2S.TX_FIFO_CTL */
#define I2S_TX_FIFO_CTL_TRIGGER_LEVEL_Pos       0UL
#define I2S_TX_FIFO_CTL_TRIGGER_LEVEL_Msk       0xFFUL
#define I2S_TX_FIFO_CTL_CLEAR_Pos               16UL
#define I2S_TX_FIFO_CTL_CLEAR_Msk               0x10000UL
#define I2S_TX_FIFO_CTL_FREEZE_Pos              17UL
#define I2S_TX_FIFO_CTL_FREEZE_Msk              0x20000UL
/* I2S.TX_FIFO_STATUS */
#define I2S_TX_FIFO_STATUS_USED_Pos             0UL
#define I2S_TX_FIFO_STATUS_USED_Msk             0x1FFUL
#define I2S_TX_FIFO_STATUS_RD_PTR_Pos           16UL
#define I2S_TX_FIFO_STATUS_RD_PTR_Msk           0xFF0000UL
#define I2S_TX_FIFO_STATUS_WR_PTR_Pos           24UL
#define I2S_TX_FIFO_STATUS_WR_PTR_Msk           0xFF000000UL
/* I2S.TX_FIFO_WR */
#define I2S_TX_FIFO_WR_DATA_Pos                 0UL
#define I2S_TX_FIFO_WR_DATA_Msk                 0xFFFFFFFFUL
/* I2S.RX_FIFO_CTL */
#define I2S_RX_FIFO_CTL_TRIGGER_LEVEL_Pos       0UL
#define I2S_RX_FIFO_CTL_TRIGGER_LEVEL_Msk       0xFFUL
#define I2S_RX_FIFO_CTL_CLEAR_Pos               16UL
#define I2S_RX_FIFO_CTL_CLEAR_Msk               0x10000UL
#define I2S_RX_FIFO_CTL_FREEZE_Pos              17UL
#define I2S_RX_FIFO_CTL_FREEZE_Msk              0x20000UL
/* I2S.RX_FIFO_STATUS */
#define I2S_RX_FIFO_STATUS_USED_Pos             0UL
#define I2S_RX_FIFO_STATUS_USED_Msk             0x1FFUL
#define I2S_RX_FIFO_STATUS_RD_PTR_Pos           16UL
#define I2S_RX_FIFO_STATUS_RD_PTR_Msk           0xFF0000UL
#define I2S_RX_FIFO_STATUS_WR_PTR_Pos           24UL
#define I2S_RX_FIFO_STATUS_WR_PTR_Msk           0xFF000000UL
/* I2S.RX_FIFO_RD */
#define I2S_RX_FIFO_RD_DATA_Pos                 0UL
#define I2S_RX_FIFO_RD_DATA_Msk                 0xFFFFFFFFUL
/* I2S.RX_FIFO_RD_SILENT */
#define I2S_RX_FIFO_RD_SILENT_DATA_Pos          0UL
#define I2S_RX_FIFO_RD_SILENT_DATA_Msk          0xFFFFFFFFUL
/* I2S.INTR */
#define I2S_INTR_TX_TRIGGER_Pos                 0UL
#define I2S_INTR_TX_TRIGGER_Msk                 0x1UL
#define I2S_INTR_TX_NOT_FULL_Pos                1UL
#define I2S_INTR_TX_NOT_FULL_Msk                0x2UL
#define I2S_INTR_TX_EMPTY_Pos                   4UL
#define I2S_INTR_TX_EMPTY_Msk                   0x10UL
#define I2S_INTR_TX_OVERFLOW_Pos                5UL
#define I2S_INTR_TX_OVERFLOW_Msk                0x20UL
#define I2S_INTR_TX_UNDERFLOW_Pos               6UL
#define I2S_INTR_TX_UNDERFLOW_Msk               0x40UL
#define I2S_INTR_TX_WD_Pos                      8UL
#define I2S_INTR_TX_WD_Msk                      0x100UL
#define I2S_INTR_RX_TRIGGER_Pos                 16UL
#define I2S_INTR_RX_TRIGGER_Msk                 0x10000UL
#define I2S_INTR_RX_NOT_EMPTY_Pos               18UL
#define I2S_INTR_RX_NOT_EMPTY_Msk               0x40000UL
#define I2S_INTR_RX_FULL_Pos                    19UL
#define I2S_INTR_RX_FULL_Msk                    0x80000UL
#define I2S_INTR_RX_OVERFLOW_Pos                21UL
#define I2S_INTR_RX_OVERFLOW_Msk                0x200000UL
#define I2S_INTR_RX_UNDERFLOW_Pos               22UL
#define I2S_INTR_RX_UNDERFLOW_Msk               0x400000UL
#define I2S_INTR_RX_WD_Pos                      24UL
#define I2S_INTR_RX_WD_Msk                      0x1000000UL
/* I2S.INTR_SET */
#define I2S_INTR_SET_TX_TRIGGER_Pos             0UL
#define I2S_INTR_SET_TX_TRIGGER_Msk             0x1UL
#define I2S_INTR_SET_TX_NOT_FULL_Pos            1UL
#define I2S_INTR_SET_TX_NOT_FULL_Msk            0x2UL
#define I2S_INTR_SET_TX_EMPTY_Pos               4UL
#define I2S_INTR_SET_TX_EMPTY_Msk               0x10UL
#define I2S_INTR_SET_TX_OVERFLOW_Pos            5UL
#define I2S_INTR_SET_TX_OVERFLOW_Msk            0x20UL
#define I2S_INTR_SET_TX_UNDERFLOW_Pos           6UL
#define I2S_INTR_SET_TX_UNDERFLOW_Msk           0x40UL
#define I2S_INTR_SET_TX_WD_Pos                  8UL
#define I2S_INTR_SET_TX_WD_Msk                  0x100UL
#define I2S_INTR_SET_RX_TRIGGER_Pos             16UL
#define I2S_INTR_SET_RX_TRIGGER_Msk             0x10000UL
#define I2S_INTR_SET_RX_NOT_EMPTY_Pos           18UL
#define I2S_INTR_SET_RX_NOT_EMPTY_Msk           0x40000UL
#define I2S_INTR_SET_RX_FULL_Pos                19UL
#define I2S_INTR_SET_RX_FULL_Msk                0x80000UL
#define I2S_INTR_SET_RX_OVERFLOW_Pos            21UL
#define I2S_INTR_SET_RX_OVERFLOW_Msk            0x200000UL
#define I2S_INTR_SET_RX_UNDERFLOW_Pos           22UL
#define I2S_INTR_SET_RX_UNDERFLOW_Msk           0x400000UL
#define I2S_INTR_SET_RX_WD_Pos                  24UL
#define I2S_INTR_SET_RX_WD_Msk                  0x1000000UL
/* I2S.INTR_MASK */
#define I2S_INTR_MASK_TX_TRIGGER_Pos            0UL
#define I2S_INTR_MASK_TX_TRIGGER_Msk            0x1UL
#define I2S_INTR_MASK_TX_NOT_FULL_Pos           1UL
#define I2S_INTR_MASK_TX_NOT_FULL_Msk           0x2UL
#define I2S_INTR_MASK_TX_EMPTY_Pos              4UL
#define I2S_INTR_MASK_TX_EMPTY_Msk              0x10UL
#define I2S_INTR_MASK_TX_OVERFLOW_Pos           5UL
#define I2S_INTR_MASK_TX_OVERFLOW_Msk           0x20UL
#define I2S_INTR_MASK_TX_UNDERFLOW_Pos          6UL
#define I2S_INTR_MASK_TX_UNDERFLOW_Msk          0x40UL
#define I2S_INTR_MASK_TX_WD_Pos                 8UL
#define I2S_INTR_MASK_TX_WD_Msk                 0x100UL
#define I2S_INTR_MASK_RX_TRIGGER_Pos            16UL
#define I2S_INTR_MASK_RX_TRIGGER_Msk            0x10000UL
#define I2S_INTR_MASK_RX_NOT_EMPTY_Pos          18UL
#define I2S_INTR_MASK_RX_NOT_EMPTY_Msk          0x40000UL
#define I2S_INTR_MASK_RX_FULL_Pos               19UL
#define I2S_INTR_MASK_RX_FULL_Msk               0x80000UL
#define I2S_INTR_MASK_RX_OVERFLOW_Pos           21UL
#define I2S_INTR_MASK_RX_OVERFLOW_Msk           0x200000UL
#define I2S_INTR_MASK_RX_UNDERFLOW_Pos          22UL
#define I2S_INTR_MASK_RX_UNDERFLOW_Msk          0x400000UL
#define I2S_INTR_MASK_RX_WD_Pos                 24UL
#define I2S_INTR_MASK_RX_WD_Msk                 0x1000000UL
/* I2S.INTR_MASKED */
#define I2S_INTR_MASKED_TX_TRIGGER_Pos          0UL
#define I2S_INTR_MASKED_TX_TRIGGER_Msk          0x1UL
#define I2S_INTR_MASKED_TX_NOT_FULL_Pos         1UL
#define I2S_INTR_MASKED_TX_NOT_FULL_Msk         0x2UL
#define I2S_INTR_MASKED_TX_EMPTY_Pos            4UL
#define I2S_INTR_MASKED_TX_EMPTY_Msk            0x10UL
#define I2S_INTR_MASKED_TX_OVERFLOW_Pos         5UL
#define I2S_INTR_MASKED_TX_OVERFLOW_Msk         0x20UL
#define I2S_INTR_MASKED_TX_UNDERFLOW_Pos        6UL
#define I2S_INTR_MASKED_TX_UNDERFLOW_Msk        0x40UL
#define I2S_INTR_MASKED_TX_WD_Pos               8UL
#define I2S_INTR_MASKED_TX_WD_Msk               0x100UL
#define I2S_INTR_MASKED_RX_TRIGGER_Pos          16UL
#define I2S_INTR_MASKED_RX_TRIGGER_Msk          0x10000UL
#define I2S_INTR_MASKED_RX_NOT_EMPTY_Pos        18UL
#define I2S_INTR_MASKED_RX_NOT_EMPTY_Msk        0x40000UL
#define I2S_INTR_MASKED_RX_FULL_Pos             19UL
#define I2S_INTR_MASKED_RX_FULL_Msk             0x80000UL
#define I2S_INTR_MASKED_RX_OVERFLOW_Pos         21UL
#define I2S_INTR_MASKED_RX_OVERFLOW_Msk         0x200000UL
#define I2S_INTR_MASKED_RX_UNDERFLOW_Pos        22UL
#define I2S_INTR_MASKED_RX_UNDERFLOW_Msk        0x400000UL
#define I2S_INTR_MASKED_RX_WD_Pos               24UL
#define I2S_INTR_MASKED_RX_WD_Msk               0x1000000UL


#endif /* _CYIP_I2S_V2_H_ */


/* [] END OF FILE */
