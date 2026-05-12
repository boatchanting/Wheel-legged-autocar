/***************************************************************************//**
* \file cyreg_i2s.h
*
* \brief
* I2S register definition header
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

#ifndef _CYREG_I2S_H_
#define _CYREG_I2S_H_

#include "cyip_i2s_v2.h"

/**
  * \brief I2S registers (I2S0)
  */
#define CYREG_I2S0_CTL                  ((volatile un_I2S_CTL_t*) 0x40800000UL)
#define CYREG_I2S0_CLOCK_CTL            ((volatile un_I2S_CLOCK_CTL_t*) 0x40800010UL)
#define CYREG_I2S0_CLOCK_STAT           ((volatile un_I2S_CLOCK_STAT_t*) 0x40800014UL)
#define CYREG_I2S0_CMD                  ((volatile un_I2S_CMD_t*) 0x40800020UL)
#define CYREG_I2S0_TR_CTL               ((volatile un_I2S_TR_CTL_t*) 0x40800040UL)
#define CYREG_I2S0_TX_CTL               ((volatile un_I2S_TX_CTL_t*) 0x40800080UL)
#define CYREG_I2S0_TX_WATCHDOG          ((volatile un_I2S_TX_WATCHDOG_t*) 0x40800084UL)
#define CYREG_I2S0_RX_CTL               ((volatile un_I2S_RX_CTL_t*) 0x408000A0UL)
#define CYREG_I2S0_RX_WATCHDOG          ((volatile un_I2S_RX_WATCHDOG_t*) 0x408000A4UL)
#define CYREG_I2S0_TX_FIFO_CTL          ((volatile un_I2S_TX_FIFO_CTL_t*) 0x40800200UL)
#define CYREG_I2S0_TX_FIFO_STATUS       ((volatile un_I2S_TX_FIFO_STATUS_t*) 0x40800204UL)
#define CYREG_I2S0_TX_FIFO_WR           ((volatile un_I2S_TX_FIFO_WR_t*) 0x40800208UL)
#define CYREG_I2S0_RX_FIFO_CTL          ((volatile un_I2S_RX_FIFO_CTL_t*) 0x40800300UL)
#define CYREG_I2S0_RX_FIFO_STATUS       ((volatile un_I2S_RX_FIFO_STATUS_t*) 0x40800304UL)
#define CYREG_I2S0_RX_FIFO_RD           ((volatile un_I2S_RX_FIFO_RD_t*) 0x40800308UL)
#define CYREG_I2S0_RX_FIFO_RD_SILENT    ((volatile un_I2S_RX_FIFO_RD_SILENT_t*) 0x4080030CUL)
#define CYREG_I2S0_INTR                 ((volatile un_I2S_INTR_t*) 0x40800F00UL)
#define CYREG_I2S0_INTR_SET             ((volatile un_I2S_INTR_SET_t*) 0x40800F04UL)
#define CYREG_I2S0_INTR_MASK            ((volatile un_I2S_INTR_MASK_t*) 0x40800F08UL)
#define CYREG_I2S0_INTR_MASKED          ((volatile un_I2S_INTR_MASKED_t*) 0x40800F0CUL)

/**
  * \brief I2S registers (I2S1)
  */
#define CYREG_I2S1_CTL                  ((volatile un_I2S_CTL_t*) 0x40801000UL)
#define CYREG_I2S1_CLOCK_CTL            ((volatile un_I2S_CLOCK_CTL_t*) 0x40801010UL)
#define CYREG_I2S1_CLOCK_STAT           ((volatile un_I2S_CLOCK_STAT_t*) 0x40801014UL)
#define CYREG_I2S1_CMD                  ((volatile un_I2S_CMD_t*) 0x40801020UL)
#define CYREG_I2S1_TR_CTL               ((volatile un_I2S_TR_CTL_t*) 0x40801040UL)
#define CYREG_I2S1_TX_CTL               ((volatile un_I2S_TX_CTL_t*) 0x40801080UL)
#define CYREG_I2S1_TX_WATCHDOG          ((volatile un_I2S_TX_WATCHDOG_t*) 0x40801084UL)
#define CYREG_I2S1_RX_CTL               ((volatile un_I2S_RX_CTL_t*) 0x408010A0UL)
#define CYREG_I2S1_RX_WATCHDOG          ((volatile un_I2S_RX_WATCHDOG_t*) 0x408010A4UL)
#define CYREG_I2S1_TX_FIFO_CTL          ((volatile un_I2S_TX_FIFO_CTL_t*) 0x40801200UL)
#define CYREG_I2S1_TX_FIFO_STATUS       ((volatile un_I2S_TX_FIFO_STATUS_t*) 0x40801204UL)
#define CYREG_I2S1_TX_FIFO_WR           ((volatile un_I2S_TX_FIFO_WR_t*) 0x40801208UL)
#define CYREG_I2S1_RX_FIFO_CTL          ((volatile un_I2S_RX_FIFO_CTL_t*) 0x40801300UL)
#define CYREG_I2S1_RX_FIFO_STATUS       ((volatile un_I2S_RX_FIFO_STATUS_t*) 0x40801304UL)
#define CYREG_I2S1_RX_FIFO_RD           ((volatile un_I2S_RX_FIFO_RD_t*) 0x40801308UL)
#define CYREG_I2S1_RX_FIFO_RD_SILENT    ((volatile un_I2S_RX_FIFO_RD_SILENT_t*) 0x4080130CUL)
#define CYREG_I2S1_INTR                 ((volatile un_I2S_INTR_t*) 0x40801F00UL)
#define CYREG_I2S1_INTR_SET             ((volatile un_I2S_INTR_SET_t*) 0x40801F04UL)
#define CYREG_I2S1_INTR_MASK            ((volatile un_I2S_INTR_MASK_t*) 0x40801F08UL)
#define CYREG_I2S1_INTR_MASKED          ((volatile un_I2S_INTR_MASKED_t*) 0x40801F0CUL)

/**
  * \brief I2S registers (I2S2)
  */
#define CYREG_I2S2_CTL                  ((volatile un_I2S_CTL_t*) 0x40802000UL)
#define CYREG_I2S2_CLOCK_CTL            ((volatile un_I2S_CLOCK_CTL_t*) 0x40802010UL)
#define CYREG_I2S2_CLOCK_STAT           ((volatile un_I2S_CLOCK_STAT_t*) 0x40802014UL)
#define CYREG_I2S2_CMD                  ((volatile un_I2S_CMD_t*) 0x40802020UL)
#define CYREG_I2S2_TR_CTL               ((volatile un_I2S_TR_CTL_t*) 0x40802040UL)
#define CYREG_I2S2_TX_CTL               ((volatile un_I2S_TX_CTL_t*) 0x40802080UL)
#define CYREG_I2S2_TX_WATCHDOG          ((volatile un_I2S_TX_WATCHDOG_t*) 0x40802084UL)
#define CYREG_I2S2_RX_CTL               ((volatile un_I2S_RX_CTL_t*) 0x408020A0UL)
#define CYREG_I2S2_RX_WATCHDOG          ((volatile un_I2S_RX_WATCHDOG_t*) 0x408020A4UL)
#define CYREG_I2S2_TX_FIFO_CTL          ((volatile un_I2S_TX_FIFO_CTL_t*) 0x40802200UL)
#define CYREG_I2S2_TX_FIFO_STATUS       ((volatile un_I2S_TX_FIFO_STATUS_t*) 0x40802204UL)
#define CYREG_I2S2_TX_FIFO_WR           ((volatile un_I2S_TX_FIFO_WR_t*) 0x40802208UL)
#define CYREG_I2S2_RX_FIFO_CTL          ((volatile un_I2S_RX_FIFO_CTL_t*) 0x40802300UL)
#define CYREG_I2S2_RX_FIFO_STATUS       ((volatile un_I2S_RX_FIFO_STATUS_t*) 0x40802304UL)
#define CYREG_I2S2_RX_FIFO_RD           ((volatile un_I2S_RX_FIFO_RD_t*) 0x40802308UL)
#define CYREG_I2S2_RX_FIFO_RD_SILENT    ((volatile un_I2S_RX_FIFO_RD_SILENT_t*) 0x4080230CUL)
#define CYREG_I2S2_INTR                 ((volatile un_I2S_INTR_t*) 0x40802F00UL)
#define CYREG_I2S2_INTR_SET             ((volatile un_I2S_INTR_SET_t*) 0x40802F04UL)
#define CYREG_I2S2_INTR_MASK            ((volatile un_I2S_INTR_MASK_t*) 0x40802F08UL)
#define CYREG_I2S2_INTR_MASKED          ((volatile un_I2S_INTR_MASKED_t*) 0x40802F0CUL)

#endif /* _CYREG_I2S_H_ */


/* [] END OF FILE */
