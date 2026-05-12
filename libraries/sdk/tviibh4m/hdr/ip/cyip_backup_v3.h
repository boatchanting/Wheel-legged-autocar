/***************************************************************************//**
* \file cyip_backup_v3.h
*
* \brief
* BACKUP IP definitions
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

#ifndef _CYIP_BACKUP_V3_H_
#define _CYIP_BACKUP_V3_H_

#include "cyip_headers.h"

/*******************************************************************************
*                                    BACKUP
*******************************************************************************/

#define BACKUP_SECTION_SIZE                     0x00010000UL

/**
  * \brief Control (BACKUP_CTL)
  */
typedef struct stc_BACKUP_CTL_field {
  uint32_t                         :3;
  uint32_t                         u1WCO_EN:1;
  uint32_t                         :4;
  uint32_t                         u2CLK_SEL:2;
  uint32_t                         :2;
  uint32_t                         u2PRESCALER:2;
  uint32_t                         :2;
  uint32_t                         u1WCO_BYPASS:1;
  uint32_t                         u2VDDBAK_CTL:2;
  uint32_t                         u1VBACKUP_MEAS:1;
  uint32_t                         :4;
  uint32_t                         u8EN_CHARGE_KEY:8;
} stc_BACKUP_CTL_field_t;

typedef union un_BACKUP_CTL {
  uint32_t                         u32Register;
  stc_BACKUP_CTL_field_t           stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_CTL_t;

/**
  * \brief RTC Read Write register (BACKUP_RTC_RW)
  */
typedef struct stc_BACKUP_RTC_RW_field {
  uint32_t                         u1READ:1;
  uint32_t                         u1WRITE:1;
  uint32_t                         :30;
} stc_BACKUP_RTC_RW_field_t;

typedef union un_BACKUP_RTC_RW {
  uint32_t                         u32Register;
  stc_BACKUP_RTC_RW_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_RTC_RW_t;

/**
  * \brief Oscillator calibration for absolute frequency (BACKUP_CAL_CTL)
  */
typedef struct stc_BACKUP_CAL_CTL_field {
  uint32_t                         u6CALIB_VAL:6;
  uint32_t                         u1CALIB_SIGN:1;
  uint32_t                         :21;
  uint32_t                         u2CAL_SEL:2;
  uint32_t                         :1;
  uint32_t                         u1CAL_OUT:1;
} stc_BACKUP_CAL_CTL_field_t;

typedef union un_BACKUP_CAL_CTL {
  uint32_t                         u32Register;
  stc_BACKUP_CAL_CTL_field_t       stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_CAL_CTL_t;

/**
  * \brief Status (BACKUP_STATUS)
  */
typedef struct stc_BACKUP_STATUS_field {
  uint32_t                         u1RTC_BUSY:1;
  uint32_t                         :1;
  uint32_t                         u1WCO_OK:1;
  uint32_t                         :29;
} stc_BACKUP_STATUS_field_t;

typedef union un_BACKUP_STATUS {
  uint32_t                         u32Register;
  stc_BACKUP_STATUS_field_t        stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_STATUS_t;

/**
  * \brief Calendar Seconds, Minutes, Hours, Day of Week (BACKUP_RTC_TIME)
  */
typedef struct stc_BACKUP_RTC_TIME_field {
  uint32_t                         u6RTC_SEC:6;
  uint32_t                         :2;
  uint32_t                         u6RTC_MIN:6;
  uint32_t                         :2;
  uint32_t                         u5RTC_HOUR:5;
  uint32_t                         :1;
  uint32_t                         u1CTRL_12HR:1;
  uint32_t                         :1;
  uint32_t                         u3RTC_DAY:3;
  uint32_t                         :5;
} stc_BACKUP_RTC_TIME_field_t;

typedef union un_BACKUP_RTC_TIME {
  uint32_t                         u32Register;
  stc_BACKUP_RTC_TIME_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_RTC_TIME_t;

/**
  * \brief Calendar Day of Month, Month,  Year (BACKUP_RTC_DATE)
  */
typedef struct stc_BACKUP_RTC_DATE_field {
  uint32_t                         u5RTC_DATE:5;
  uint32_t                         :3;
  uint32_t                         u4RTC_MON:4;
  uint32_t                         :4;
  uint32_t                         u7RTC_YEAR:7;
  uint32_t                         :9;
} stc_BACKUP_RTC_DATE_field_t;

typedef union un_BACKUP_RTC_DATE {
  uint32_t                         u32Register;
  stc_BACKUP_RTC_DATE_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_RTC_DATE_t;

/**
  * \brief Alarm 1 Seconds, Minute, Hours, Day of Week (BACKUP_ALM1_TIME)
  */
typedef struct stc_BACKUP_ALM1_TIME_field {
  uint32_t                         u6ALM_SEC:6;
  uint32_t                         :1;
  uint32_t                         u1ALM_SEC_EN:1;
  uint32_t                         u6ALM_MIN:6;
  uint32_t                         :1;
  uint32_t                         u1ALM_MIN_EN:1;
  uint32_t                         u5ALM_HOUR:5;
  uint32_t                         :2;
  uint32_t                         u1ALM_HOUR_EN:1;
  uint32_t                         u3ALM_DAY:3;
  uint32_t                         :4;
  uint32_t                         u1ALM_DAY_EN:1;
} stc_BACKUP_ALM1_TIME_field_t;

typedef union un_BACKUP_ALM1_TIME {
  uint32_t                         u32Register;
  stc_BACKUP_ALM1_TIME_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_ALM1_TIME_t;

/**
  * \brief Alarm 1 Day of Month, Month (BACKUP_ALM1_DATE)
  */
typedef struct stc_BACKUP_ALM1_DATE_field {
  uint32_t                         u5ALM_DATE:5;
  uint32_t                         :2;
  uint32_t                         u1ALM_DATE_EN:1;
  uint32_t                         u4ALM_MON:4;
  uint32_t                         :3;
  uint32_t                         u1ALM_MON_EN:1;
  uint32_t                         :15;
  uint32_t                         u1ALM_EN:1;
} stc_BACKUP_ALM1_DATE_field_t;

typedef union un_BACKUP_ALM1_DATE {
  uint32_t                         u32Register;
  stc_BACKUP_ALM1_DATE_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_ALM1_DATE_t;

/**
  * \brief Alarm 2 Seconds, Minute, Hours, Day of Week (BACKUP_ALM2_TIME)
  */
typedef struct stc_BACKUP_ALM2_TIME_field {
  uint32_t                         u6ALM_SEC:6;
  uint32_t                         :1;
  uint32_t                         u1ALM_SEC_EN:1;
  uint32_t                         u6ALM_MIN:6;
  uint32_t                         :1;
  uint32_t                         u1ALM_MIN_EN:1;
  uint32_t                         u5ALM_HOUR:5;
  uint32_t                         :2;
  uint32_t                         u1ALM_HOUR_EN:1;
  uint32_t                         u3ALM_DAY:3;
  uint32_t                         :4;
  uint32_t                         u1ALM_DAY_EN:1;
} stc_BACKUP_ALM2_TIME_field_t;

typedef union un_BACKUP_ALM2_TIME {
  uint32_t                         u32Register;
  stc_BACKUP_ALM2_TIME_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_ALM2_TIME_t;

/**
  * \brief Alarm 2 Day of Month, Month (BACKUP_ALM2_DATE)
  */
typedef struct stc_BACKUP_ALM2_DATE_field {
  uint32_t                         u5ALM_DATE:5;
  uint32_t                         :2;
  uint32_t                         u1ALM_DATE_EN:1;
  uint32_t                         u4ALM_MON:4;
  uint32_t                         :3;
  uint32_t                         u1ALM_MON_EN:1;
  uint32_t                         :15;
  uint32_t                         u1ALM_EN:1;
} stc_BACKUP_ALM2_DATE_field_t;

typedef union un_BACKUP_ALM2_DATE {
  uint32_t                         u32Register;
  stc_BACKUP_ALM2_DATE_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_ALM2_DATE_t;

/**
  * \brief Interrupt request register (BACKUP_INTR)
  */
typedef struct stc_BACKUP_INTR_field {
  uint32_t                         u1ALARM1:1;
  uint32_t                         u1ALARM2:1;
  uint32_t                         u1CENTURY:1;
  uint32_t                         :29;
} stc_BACKUP_INTR_field_t;

typedef union un_BACKUP_INTR {
  uint32_t                         u32Register;
  stc_BACKUP_INTR_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_INTR_t;

/**
  * \brief Interrupt set request register (BACKUP_INTR_SET)
  */
typedef struct stc_BACKUP_INTR_SET_field {
  uint32_t                         u1ALARM1:1;
  uint32_t                         u1ALARM2:1;
  uint32_t                         u1CENTURY:1;
  uint32_t                         :29;
} stc_BACKUP_INTR_SET_field_t;

typedef union un_BACKUP_INTR_SET {
  uint32_t                         u32Register;
  stc_BACKUP_INTR_SET_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_INTR_SET_t;

/**
  * \brief Interrupt mask register (BACKUP_INTR_MASK)
  */
typedef struct stc_BACKUP_INTR_MASK_field {
  uint32_t                         u1ALARM1:1;
  uint32_t                         u1ALARM2:1;
  uint32_t                         u1CENTURY:1;
  uint32_t                         :29;
} stc_BACKUP_INTR_MASK_field_t;

typedef union un_BACKUP_INTR_MASK {
  uint32_t                         u32Register;
  stc_BACKUP_INTR_MASK_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_INTR_MASK_t;

/**
  * \brief Interrupt masked request register (BACKUP_INTR_MASKED)
  */
typedef struct stc_BACKUP_INTR_MASKED_field {
  uint32_t                         u1ALARM1:1;
  uint32_t                         u1ALARM2:1;
  uint32_t                         u1CENTURY:1;
  uint32_t                         :29;
} stc_BACKUP_INTR_MASKED_field_t;

typedef union un_BACKUP_INTR_MASKED {
  uint32_t                         u32Register;
  stc_BACKUP_INTR_MASKED_field_t   stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_INTR_MASKED_t;

/**
  * \brief PMIC control register (BACKUP_PMIC_CTL)
  */
typedef struct stc_BACKUP_PMIC_CTL_field {
  uint32_t                         :8;
  uint32_t                         u8UNLOCK:8;
  uint32_t                         u1POLARITY:1;
  uint32_t                         :12;
  uint32_t                         u1PMIC_EN_OUTEN:1;
  uint32_t                         u1PMIC_ALWAYSEN:1;
  uint32_t                         u1PMIC_EN:1;
} stc_BACKUP_PMIC_CTL_field_t;

typedef union un_BACKUP_PMIC_CTL {
  uint32_t                         u32Register;
  stc_BACKUP_PMIC_CTL_field_t      stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_PMIC_CTL_t;

/**
  * \brief Backup reset register (BACKUP_RESET)
  */
typedef struct stc_BACKUP_RESET_field {
  uint32_t                         :31;
  uint32_t                         u1RESET:1;
} stc_BACKUP_RESET_field_t;

typedef union un_BACKUP_RESET {
  uint32_t                         u32Register;
  stc_BACKUP_RESET_field_t         stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_RESET_t;

/**
  * \brief Low-power external crystal oscillator control (BACKUP_LPECO_CTL)
  */
typedef struct stc_BACKUP_LPECO_CTL_field {
  uint32_t                         :4;
  uint32_t                         u2LPECO_CRANGE:2;
  uint32_t                         :2;
  uint32_t                         u1LPECO_FRANGE:1;
  uint32_t                         :3;
  uint32_t                         u1LPECO_AMP_SEL:1;
  uint32_t                         :15;
  uint32_t                         u1LPECO_DIV_ENABLE:1;
  uint32_t                         :1;
  uint32_t                         u1LPECO_AMPDET_EN:1;
  uint32_t                         u1LPECO_EN:1;
} stc_BACKUP_LPECO_CTL_field_t;

typedef union un_BACKUP_LPECO_CTL {
  uint32_t                         u32Register;
  stc_BACKUP_LPECO_CTL_field_t     stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_LPECO_CTL_t;

/**
  * \brief Low-power external crystal oscillator prescaler (BACKUP_LPECO_PRESCALE)
  */
typedef struct stc_BACKUP_LPECO_PRESCALE_field {
  uint32_t                         u1LPECO_DIV_ENABLED:1;
  uint32_t                         :7;
  uint32_t                         u8LPECO_FRAC_DIV:8;
  uint32_t                         u10LPECO_INT_DIV:10;
  uint32_t                         :6;
} stc_BACKUP_LPECO_PRESCALE_field_t;

typedef union un_BACKUP_LPECO_PRESCALE {
  uint32_t                         u32Register;
  stc_BACKUP_LPECO_PRESCALE_field_t stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_LPECO_PRESCALE_t;

/**
  * \brief Low-power external crystal oscillator status (BACKUP_LPECO_STATUS)
  */
typedef struct stc_BACKUP_LPECO_STATUS_field {
  uint32_t                         u1LPECO_AMPDET_OK:1;
  uint32_t                         u1LPECO_READY:1;
  uint32_t                         :30;
} stc_BACKUP_LPECO_STATUS_field_t;

typedef union un_BACKUP_LPECO_STATUS {
  uint32_t                         u32Register;
  stc_BACKUP_LPECO_STATUS_field_t  stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_LPECO_STATUS_t;

/**
  * \brief Backup register region (BACKUP_BREG)
  */
typedef struct stc_BACKUP_BREG_field {
  uint32_t                         u32BREG:32;
} stc_BACKUP_BREG_field_t;

typedef union un_BACKUP_BREG {
  uint32_t                         u32Register;
  stc_BACKUP_BREG_field_t          stcField;
  uint8_t                          au8Byte[4];
  uint16_t                         au16Halfword[2];
} un_BACKUP_BREG_t;



/**
  * \brief SRSS Backup Domain (ver3) (BACKUP)
  */
typedef struct stc_BACKUP {
  un_BACKUP_CTL_t                  unCTL;              /*!< 0x00000000 Control */
  uint32_t                         u32Reserved;
  un_BACKUP_RTC_RW_t               unRTC_RW;           /*!< 0x00000008 RTC Read Write register */
  un_BACKUP_CAL_CTL_t              unCAL_CTL;          /*!< 0x0000000C Oscillator calibration for absolute frequency */
  un_BACKUP_STATUS_t               unSTATUS;           /*!< 0x00000010 Status */
  un_BACKUP_RTC_TIME_t             unRTC_TIME;         /*!< 0x00000014 Calendar Seconds, Minutes, Hours, Day of Week */
  un_BACKUP_RTC_DATE_t             unRTC_DATE;         /*!< 0x00000018 Calendar Day of Month, Month,  Year */
  un_BACKUP_ALM1_TIME_t            unALM1_TIME;        /*!< 0x0000001C Alarm 1 Seconds, Minute, Hours, Day of Week */
  un_BACKUP_ALM1_DATE_t            unALM1_DATE;        /*!< 0x00000020 Alarm 1 Day of Month, Month */
  un_BACKUP_ALM2_TIME_t            unALM2_TIME;        /*!< 0x00000024 Alarm 2 Seconds, Minute, Hours, Day of Week */
  un_BACKUP_ALM2_DATE_t            unALM2_DATE;        /*!< 0x00000028 Alarm 2 Day of Month, Month */
  un_BACKUP_INTR_t                 unINTR;             /*!< 0x0000002C Interrupt request register */
  un_BACKUP_INTR_SET_t             unINTR_SET;         /*!< 0x00000030 Interrupt set request register */
  un_BACKUP_INTR_MASK_t            unINTR_MASK;        /*!< 0x00000034 Interrupt mask register */
  un_BACKUP_INTR_MASKED_t          unINTR_MASKED;      /*!< 0x00000038 Interrupt masked request register */
  uint32_t                         au32Reserved1[2];
  un_BACKUP_PMIC_CTL_t             unPMIC_CTL;         /*!< 0x00000044 PMIC control register */
  un_BACKUP_RESET_t                unRESET;            /*!< 0x00000048 Backup reset register */
  uint32_t                         au32Reserved2[13];
  un_BACKUP_LPECO_CTL_t            unLPECO_CTL;        /*!< 0x00000080 Low-power external crystal oscillator control */
  un_BACKUP_LPECO_PRESCALE_t       unLPECO_PRESCALE;   /*!< 0x00000084 Low-power external crystal oscillator prescaler */
  un_BACKUP_LPECO_STATUS_t         unLPECO_STATUS;     /*!< 0x00000088 Low-power external crystal oscillator status */
  uint32_t                         au32Reserved3[989];
  un_BACKUP_BREG_t                 unBREG[64];         /*!< 0x00001000 Backup register region */
} stc_BACKUP_t;                                        /*!< Size = 4352 (0x1100) */


/* BACKUP.CTL */
#define BACKUP_CTL_WCO_EN_Pos                   3UL
#define BACKUP_CTL_WCO_EN_Msk                   0x8UL
#define BACKUP_CTL_CLK_SEL_Pos                  8UL
#define BACKUP_CTL_CLK_SEL_Msk                  0x300UL
#define BACKUP_CTL_PRESCALER_Pos                12UL
#define BACKUP_CTL_PRESCALER_Msk                0x3000UL
#define BACKUP_CTL_WCO_BYPASS_Pos               16UL
#define BACKUP_CTL_WCO_BYPASS_Msk               0x10000UL
#define BACKUP_CTL_VDDBAK_CTL_Pos               17UL
#define BACKUP_CTL_VDDBAK_CTL_Msk               0x60000UL
#define BACKUP_CTL_VBACKUP_MEAS_Pos             19UL
#define BACKUP_CTL_VBACKUP_MEAS_Msk             0x80000UL
#define BACKUP_CTL_EN_CHARGE_KEY_Pos            24UL
#define BACKUP_CTL_EN_CHARGE_KEY_Msk            0xFF000000UL
/* BACKUP.RTC_RW */
#define BACKUP_RTC_RW_READ_Pos                  0UL
#define BACKUP_RTC_RW_READ_Msk                  0x1UL
#define BACKUP_RTC_RW_WRITE_Pos                 1UL
#define BACKUP_RTC_RW_WRITE_Msk                 0x2UL
/* BACKUP.CAL_CTL */
#define BACKUP_CAL_CTL_CALIB_VAL_Pos            0UL
#define BACKUP_CAL_CTL_CALIB_VAL_Msk            0x3FUL
#define BACKUP_CAL_CTL_CALIB_SIGN_Pos           6UL
#define BACKUP_CAL_CTL_CALIB_SIGN_Msk           0x40UL
#define BACKUP_CAL_CTL_CAL_SEL_Pos              28UL
#define BACKUP_CAL_CTL_CAL_SEL_Msk              0x30000000UL
#define BACKUP_CAL_CTL_CAL_OUT_Pos              31UL
#define BACKUP_CAL_CTL_CAL_OUT_Msk              0x80000000UL
/* BACKUP.STATUS */
#define BACKUP_STATUS_RTC_BUSY_Pos              0UL
#define BACKUP_STATUS_RTC_BUSY_Msk              0x1UL
#define BACKUP_STATUS_WCO_OK_Pos                2UL
#define BACKUP_STATUS_WCO_OK_Msk                0x4UL
/* BACKUP.RTC_TIME */
#define BACKUP_RTC_TIME_RTC_SEC_Pos             0UL
#define BACKUP_RTC_TIME_RTC_SEC_Msk             0x3FUL
#define BACKUP_RTC_TIME_RTC_MIN_Pos             8UL
#define BACKUP_RTC_TIME_RTC_MIN_Msk             0x3F00UL
#define BACKUP_RTC_TIME_RTC_HOUR_Pos            16UL
#define BACKUP_RTC_TIME_RTC_HOUR_Msk            0x1F0000UL
#define BACKUP_RTC_TIME_CTRL_12HR_Pos           22UL
#define BACKUP_RTC_TIME_CTRL_12HR_Msk           0x400000UL
#define BACKUP_RTC_TIME_RTC_DAY_Pos             24UL
#define BACKUP_RTC_TIME_RTC_DAY_Msk             0x7000000UL
/* BACKUP.RTC_DATE */
#define BACKUP_RTC_DATE_RTC_DATE_Pos            0UL
#define BACKUP_RTC_DATE_RTC_DATE_Msk            0x1FUL
#define BACKUP_RTC_DATE_RTC_MON_Pos             8UL
#define BACKUP_RTC_DATE_RTC_MON_Msk             0xF00UL
#define BACKUP_RTC_DATE_RTC_YEAR_Pos            16UL
#define BACKUP_RTC_DATE_RTC_YEAR_Msk            0x7F0000UL
/* BACKUP.ALM1_TIME */
#define BACKUP_ALM1_TIME_ALM_SEC_Pos            0UL
#define BACKUP_ALM1_TIME_ALM_SEC_Msk            0x3FUL
#define BACKUP_ALM1_TIME_ALM_SEC_EN_Pos         7UL
#define BACKUP_ALM1_TIME_ALM_SEC_EN_Msk         0x80UL
#define BACKUP_ALM1_TIME_ALM_MIN_Pos            8UL
#define BACKUP_ALM1_TIME_ALM_MIN_Msk            0x3F00UL
#define BACKUP_ALM1_TIME_ALM_MIN_EN_Pos         15UL
#define BACKUP_ALM1_TIME_ALM_MIN_EN_Msk         0x8000UL
#define BACKUP_ALM1_TIME_ALM_HOUR_Pos           16UL
#define BACKUP_ALM1_TIME_ALM_HOUR_Msk           0x1F0000UL
#define BACKUP_ALM1_TIME_ALM_HOUR_EN_Pos        23UL
#define BACKUP_ALM1_TIME_ALM_HOUR_EN_Msk        0x800000UL
#define BACKUP_ALM1_TIME_ALM_DAY_Pos            24UL
#define BACKUP_ALM1_TIME_ALM_DAY_Msk            0x7000000UL
#define BACKUP_ALM1_TIME_ALM_DAY_EN_Pos         31UL
#define BACKUP_ALM1_TIME_ALM_DAY_EN_Msk         0x80000000UL
/* BACKUP.ALM1_DATE */
#define BACKUP_ALM1_DATE_ALM_DATE_Pos           0UL
#define BACKUP_ALM1_DATE_ALM_DATE_Msk           0x1FUL
#define BACKUP_ALM1_DATE_ALM_DATE_EN_Pos        7UL
#define BACKUP_ALM1_DATE_ALM_DATE_EN_Msk        0x80UL
#define BACKUP_ALM1_DATE_ALM_MON_Pos            8UL
#define BACKUP_ALM1_DATE_ALM_MON_Msk            0xF00UL
#define BACKUP_ALM1_DATE_ALM_MON_EN_Pos         15UL
#define BACKUP_ALM1_DATE_ALM_MON_EN_Msk         0x8000UL
#define BACKUP_ALM1_DATE_ALM_EN_Pos             31UL
#define BACKUP_ALM1_DATE_ALM_EN_Msk             0x80000000UL
/* BACKUP.ALM2_TIME */
#define BACKUP_ALM2_TIME_ALM_SEC_Pos            0UL
#define BACKUP_ALM2_TIME_ALM_SEC_Msk            0x3FUL
#define BACKUP_ALM2_TIME_ALM_SEC_EN_Pos         7UL
#define BACKUP_ALM2_TIME_ALM_SEC_EN_Msk         0x80UL
#define BACKUP_ALM2_TIME_ALM_MIN_Pos            8UL
#define BACKUP_ALM2_TIME_ALM_MIN_Msk            0x3F00UL
#define BACKUP_ALM2_TIME_ALM_MIN_EN_Pos         15UL
#define BACKUP_ALM2_TIME_ALM_MIN_EN_Msk         0x8000UL
#define BACKUP_ALM2_TIME_ALM_HOUR_Pos           16UL
#define BACKUP_ALM2_TIME_ALM_HOUR_Msk           0x1F0000UL
#define BACKUP_ALM2_TIME_ALM_HOUR_EN_Pos        23UL
#define BACKUP_ALM2_TIME_ALM_HOUR_EN_Msk        0x800000UL
#define BACKUP_ALM2_TIME_ALM_DAY_Pos            24UL
#define BACKUP_ALM2_TIME_ALM_DAY_Msk            0x7000000UL
#define BACKUP_ALM2_TIME_ALM_DAY_EN_Pos         31UL
#define BACKUP_ALM2_TIME_ALM_DAY_EN_Msk         0x80000000UL
/* BACKUP.ALM2_DATE */
#define BACKUP_ALM2_DATE_ALM_DATE_Pos           0UL
#define BACKUP_ALM2_DATE_ALM_DATE_Msk           0x1FUL
#define BACKUP_ALM2_DATE_ALM_DATE_EN_Pos        7UL
#define BACKUP_ALM2_DATE_ALM_DATE_EN_Msk        0x80UL
#define BACKUP_ALM2_DATE_ALM_MON_Pos            8UL
#define BACKUP_ALM2_DATE_ALM_MON_Msk            0xF00UL
#define BACKUP_ALM2_DATE_ALM_MON_EN_Pos         15UL
#define BACKUP_ALM2_DATE_ALM_MON_EN_Msk         0x8000UL
#define BACKUP_ALM2_DATE_ALM_EN_Pos             31UL
#define BACKUP_ALM2_DATE_ALM_EN_Msk             0x80000000UL
/* BACKUP.INTR */
#define BACKUP_INTR_ALARM1_Pos                  0UL
#define BACKUP_INTR_ALARM1_Msk                  0x1UL
#define BACKUP_INTR_ALARM2_Pos                  1UL
#define BACKUP_INTR_ALARM2_Msk                  0x2UL
#define BACKUP_INTR_CENTURY_Pos                 2UL
#define BACKUP_INTR_CENTURY_Msk                 0x4UL
/* BACKUP.INTR_SET */
#define BACKUP_INTR_SET_ALARM1_Pos              0UL
#define BACKUP_INTR_SET_ALARM1_Msk              0x1UL
#define BACKUP_INTR_SET_ALARM2_Pos              1UL
#define BACKUP_INTR_SET_ALARM2_Msk              0x2UL
#define BACKUP_INTR_SET_CENTURY_Pos             2UL
#define BACKUP_INTR_SET_CENTURY_Msk             0x4UL
/* BACKUP.INTR_MASK */
#define BACKUP_INTR_MASK_ALARM1_Pos             0UL
#define BACKUP_INTR_MASK_ALARM1_Msk             0x1UL
#define BACKUP_INTR_MASK_ALARM2_Pos             1UL
#define BACKUP_INTR_MASK_ALARM2_Msk             0x2UL
#define BACKUP_INTR_MASK_CENTURY_Pos            2UL
#define BACKUP_INTR_MASK_CENTURY_Msk            0x4UL
/* BACKUP.INTR_MASKED */
#define BACKUP_INTR_MASKED_ALARM1_Pos           0UL
#define BACKUP_INTR_MASKED_ALARM1_Msk           0x1UL
#define BACKUP_INTR_MASKED_ALARM2_Pos           1UL
#define BACKUP_INTR_MASKED_ALARM2_Msk           0x2UL
#define BACKUP_INTR_MASKED_CENTURY_Pos          2UL
#define BACKUP_INTR_MASKED_CENTURY_Msk          0x4UL
/* BACKUP.PMIC_CTL */
#define BACKUP_PMIC_CTL_UNLOCK_Pos              8UL
#define BACKUP_PMIC_CTL_UNLOCK_Msk              0xFF00UL
#define BACKUP_PMIC_CTL_POLARITY_Pos            16UL
#define BACKUP_PMIC_CTL_POLARITY_Msk            0x10000UL
#define BACKUP_PMIC_CTL_PMIC_EN_OUTEN_Pos       29UL
#define BACKUP_PMIC_CTL_PMIC_EN_OUTEN_Msk       0x20000000UL
#define BACKUP_PMIC_CTL_PMIC_ALWAYSEN_Pos       30UL
#define BACKUP_PMIC_CTL_PMIC_ALWAYSEN_Msk       0x40000000UL
#define BACKUP_PMIC_CTL_PMIC_EN_Pos             31UL
#define BACKUP_PMIC_CTL_PMIC_EN_Msk             0x80000000UL
/* BACKUP.RESET */
#define BACKUP_RESET_RESET_Pos                  31UL
#define BACKUP_RESET_RESET_Msk                  0x80000000UL
/* BACKUP.LPECO_CTL */
#define BACKUP_LPECO_CTL_LPECO_CRANGE_Pos       4UL
#define BACKUP_LPECO_CTL_LPECO_CRANGE_Msk       0x30UL
#define BACKUP_LPECO_CTL_LPECO_FRANGE_Pos       8UL
#define BACKUP_LPECO_CTL_LPECO_FRANGE_Msk       0x100UL
#define BACKUP_LPECO_CTL_LPECO_AMP_SEL_Pos      12UL
#define BACKUP_LPECO_CTL_LPECO_AMP_SEL_Msk      0x1000UL
#define BACKUP_LPECO_CTL_LPECO_DIV_ENABLE_Pos   28UL
#define BACKUP_LPECO_CTL_LPECO_DIV_ENABLE_Msk   0x10000000UL
#define BACKUP_LPECO_CTL_LPECO_AMPDET_EN_Pos    30UL
#define BACKUP_LPECO_CTL_LPECO_AMPDET_EN_Msk    0x40000000UL
#define BACKUP_LPECO_CTL_LPECO_EN_Pos           31UL
#define BACKUP_LPECO_CTL_LPECO_EN_Msk           0x80000000UL
/* BACKUP.LPECO_PRESCALE */
#define BACKUP_LPECO_PRESCALE_LPECO_DIV_ENABLED_Pos 0UL
#define BACKUP_LPECO_PRESCALE_LPECO_DIV_ENABLED_Msk 0x1UL
#define BACKUP_LPECO_PRESCALE_LPECO_FRAC_DIV_Pos 8UL
#define BACKUP_LPECO_PRESCALE_LPECO_FRAC_DIV_Msk 0xFF00UL
#define BACKUP_LPECO_PRESCALE_LPECO_INT_DIV_Pos 16UL
#define BACKUP_LPECO_PRESCALE_LPECO_INT_DIV_Msk 0x3FF0000UL
/* BACKUP.LPECO_STATUS */
#define BACKUP_LPECO_STATUS_LPECO_AMPDET_OK_Pos 0UL
#define BACKUP_LPECO_STATUS_LPECO_AMPDET_OK_Msk 0x1UL
#define BACKUP_LPECO_STATUS_LPECO_READY_Pos     1UL
#define BACKUP_LPECO_STATUS_LPECO_READY_Msk     0x2UL
/* BACKUP.BREG */
#define BACKUP_BREG_BREG_Pos                    0UL
#define BACKUP_BREG_BREG_Msk                    0xFFFFFFFFUL


#endif /* _CYIP_BACKUP_V3_H_ */


/* [] END OF FILE */
