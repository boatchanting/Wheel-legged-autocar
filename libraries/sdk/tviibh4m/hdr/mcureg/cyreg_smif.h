/***************************************************************************//**
* \file cyreg_smif.h
*
* \brief
* SMIF register definition header
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

#ifndef _CYREG_SMIF_H_
#define _CYREG_SMIF_H_

#include "cyip_smif_v2.h"

/**
  * \brief Device (only used in XIP mode) (SMIF_DEVICE0)
  */
#define CYREG_SMIF0_DEVICE0_CTL         ((volatile un_SMIF_DEVICE_CTL_t*) 0x40420800UL)
#define CYREG_SMIF0_DEVICE0_ADDR        ((volatile un_SMIF_DEVICE_ADDR_t*) 0x40420808UL)
#define CYREG_SMIF0_DEVICE0_MASK        ((volatile un_SMIF_DEVICE_MASK_t*) 0x4042080CUL)
#define CYREG_SMIF0_DEVICE0_ADDR_CTL    ((volatile un_SMIF_DEVICE_ADDR_CTL_t*) 0x40420820UL)
#define CYREG_SMIF0_DEVICE0_RD_CMD_CTL  ((volatile un_SMIF_DEVICE_RD_CMD_CTL_t*) 0x40420840UL)
#define CYREG_SMIF0_DEVICE0_RD_ADDR_CTL ((volatile un_SMIF_DEVICE_RD_ADDR_CTL_t*) 0x40420844UL)
#define CYREG_SMIF0_DEVICE0_RD_MODE_CTL ((volatile un_SMIF_DEVICE_RD_MODE_CTL_t*) 0x40420848UL)
#define CYREG_SMIF0_DEVICE0_RD_DUMMY_CTL ((volatile un_SMIF_DEVICE_RD_DUMMY_CTL_t*) 0x4042084CUL)
#define CYREG_SMIF0_DEVICE0_RD_DATA_CTL ((volatile un_SMIF_DEVICE_RD_DATA_CTL_t*) 0x40420850UL)
#define CYREG_SMIF0_DEVICE0_RD_BOUND_CTL ((volatile un_SMIF_DEVICE_RD_BOUND_CTL_t*) 0x40420858UL)
#define CYREG_SMIF0_DEVICE0_WR_CMD_CTL  ((volatile un_SMIF_DEVICE_WR_CMD_CTL_t*) 0x40420860UL)
#define CYREG_SMIF0_DEVICE0_WR_ADDR_CTL ((volatile un_SMIF_DEVICE_WR_ADDR_CTL_t*) 0x40420864UL)
#define CYREG_SMIF0_DEVICE0_WR_MODE_CTL ((volatile un_SMIF_DEVICE_WR_MODE_CTL_t*) 0x40420868UL)
#define CYREG_SMIF0_DEVICE0_WR_DUMMY_CTL ((volatile un_SMIF_DEVICE_WR_DUMMY_CTL_t*) 0x4042086CUL)
#define CYREG_SMIF0_DEVICE0_WR_DATA_CTL ((volatile un_SMIF_DEVICE_WR_DATA_CTL_t*) 0x40420870UL)

/**
  * \brief Device (only used in XIP mode) (SMIF_DEVICE1)
  */
#define CYREG_SMIF0_DEVICE1_CTL         ((volatile un_SMIF_DEVICE_CTL_t*) 0x40420880UL)
#define CYREG_SMIF0_DEVICE1_ADDR        ((volatile un_SMIF_DEVICE_ADDR_t*) 0x40420888UL)
#define CYREG_SMIF0_DEVICE1_MASK        ((volatile un_SMIF_DEVICE_MASK_t*) 0x4042088CUL)
#define CYREG_SMIF0_DEVICE1_ADDR_CTL    ((volatile un_SMIF_DEVICE_ADDR_CTL_t*) 0x404208A0UL)
#define CYREG_SMIF0_DEVICE1_RD_CMD_CTL  ((volatile un_SMIF_DEVICE_RD_CMD_CTL_t*) 0x404208C0UL)
#define CYREG_SMIF0_DEVICE1_RD_ADDR_CTL ((volatile un_SMIF_DEVICE_RD_ADDR_CTL_t*) 0x404208C4UL)
#define CYREG_SMIF0_DEVICE1_RD_MODE_CTL ((volatile un_SMIF_DEVICE_RD_MODE_CTL_t*) 0x404208C8UL)
#define CYREG_SMIF0_DEVICE1_RD_DUMMY_CTL ((volatile un_SMIF_DEVICE_RD_DUMMY_CTL_t*) 0x404208CCUL)
#define CYREG_SMIF0_DEVICE1_RD_DATA_CTL ((volatile un_SMIF_DEVICE_RD_DATA_CTL_t*) 0x404208D0UL)
#define CYREG_SMIF0_DEVICE1_RD_BOUND_CTL ((volatile un_SMIF_DEVICE_RD_BOUND_CTL_t*) 0x404208D8UL)
#define CYREG_SMIF0_DEVICE1_WR_CMD_CTL  ((volatile un_SMIF_DEVICE_WR_CMD_CTL_t*) 0x404208E0UL)
#define CYREG_SMIF0_DEVICE1_WR_ADDR_CTL ((volatile un_SMIF_DEVICE_WR_ADDR_CTL_t*) 0x404208E4UL)
#define CYREG_SMIF0_DEVICE1_WR_MODE_CTL ((volatile un_SMIF_DEVICE_WR_MODE_CTL_t*) 0x404208E8UL)
#define CYREG_SMIF0_DEVICE1_WR_DUMMY_CTL ((volatile un_SMIF_DEVICE_WR_DUMMY_CTL_t*) 0x404208ECUL)
#define CYREG_SMIF0_DEVICE1_WR_DATA_CTL ((volatile un_SMIF_DEVICE_WR_DATA_CTL_t*) 0x404208F0UL)

/**
  * \brief Serial Memory Interface (SMIF0)
  */
#define CYREG_SMIF0_CTL                 ((volatile un_SMIF_CTL_t*) 0x40420000UL)
#define CYREG_SMIF0_STATUS              ((volatile un_SMIF_STATUS_t*) 0x40420004UL)
#define CYREG_SMIF0_DELAY_TAP_SEL       ((volatile un_SMIF_DELAY_TAP_SEL_t*) 0x40420030UL)
#define CYREG_SMIF0_TX_CMD_FIFO_STATUS  ((volatile un_SMIF_TX_CMD_FIFO_STATUS_t*) 0x40420044UL)
#define CYREG_SMIF0_TX_CMD_FIFO_WR      ((volatile un_SMIF_TX_CMD_FIFO_WR_t*) 0x40420050UL)
#define CYREG_SMIF0_TX_DATA_FIFO_CTL    ((volatile un_SMIF_TX_DATA_FIFO_CTL_t*) 0x40420080UL)
#define CYREG_SMIF0_TX_DATA_FIFO_STATUS ((volatile un_SMIF_TX_DATA_FIFO_STATUS_t*) 0x40420084UL)
#define CYREG_SMIF0_TX_DATA_FIFO_WR1    ((volatile un_SMIF_TX_DATA_FIFO_WR1_t*) 0x40420090UL)
#define CYREG_SMIF0_TX_DATA_FIFO_WR2    ((volatile un_SMIF_TX_DATA_FIFO_WR2_t*) 0x40420094UL)
#define CYREG_SMIF0_TX_DATA_FIFO_WR4    ((volatile un_SMIF_TX_DATA_FIFO_WR4_t*) 0x40420098UL)
#define CYREG_SMIF0_TX_DATA_FIFO_WR1ODD ((volatile un_SMIF_TX_DATA_FIFO_WR1ODD_t*) 0x4042009CUL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_CTL ((volatile un_SMIF_RX_DATA_MMIO_FIFO_CTL_t*) 0x404200C0UL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_STATUS ((volatile un_SMIF_RX_DATA_MMIO_FIFO_STATUS_t*) 0x404200C4UL)
#define CYREG_SMIF0_RX_DATA_FIFO_STATUS ((volatile un_SMIF_RX_DATA_FIFO_STATUS_t*) 0x404200C8UL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_RD1 ((volatile un_SMIF_RX_DATA_MMIO_FIFO_RD1_t*) 0x404200D0UL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_RD2 ((volatile un_SMIF_RX_DATA_MMIO_FIFO_RD2_t*) 0x404200D4UL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_RD4 ((volatile un_SMIF_RX_DATA_MMIO_FIFO_RD4_t*) 0x404200D8UL)
#define CYREG_SMIF0_RX_DATA_MMIO_FIFO_RD1_SILENT ((volatile un_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_t*) 0x404200E0UL)
#define CYREG_SMIF0_SLOW_CA_CTL         ((volatile un_SMIF_SLOW_CA_CTL_t*) 0x40420100UL)
#define CYREG_SMIF0_SLOW_CA_CMD         ((volatile un_SMIF_SLOW_CA_CMD_t*) 0x40420108UL)
#define CYREG_SMIF0_CRYPTO_CMD          ((volatile un_SMIF_CRYPTO_CMD_t*) 0x40420200UL)
#define CYREG_SMIF0_CRYPTO_INPUT0       ((volatile un_SMIF_CRYPTO_INPUT0_t*) 0x40420220UL)
#define CYREG_SMIF0_CRYPTO_INPUT1       ((volatile un_SMIF_CRYPTO_INPUT1_t*) 0x40420224UL)
#define CYREG_SMIF0_CRYPTO_INPUT2       ((volatile un_SMIF_CRYPTO_INPUT2_t*) 0x40420228UL)
#define CYREG_SMIF0_CRYPTO_INPUT3       ((volatile un_SMIF_CRYPTO_INPUT3_t*) 0x4042022CUL)
#define CYREG_SMIF0_CRYPTO_KEY0         ((volatile un_SMIF_CRYPTO_KEY0_t*) 0x40420240UL)
#define CYREG_SMIF0_CRYPTO_KEY1         ((volatile un_SMIF_CRYPTO_KEY1_t*) 0x40420244UL)
#define CYREG_SMIF0_CRYPTO_KEY2         ((volatile un_SMIF_CRYPTO_KEY2_t*) 0x40420248UL)
#define CYREG_SMIF0_CRYPTO_KEY3         ((volatile un_SMIF_CRYPTO_KEY3_t*) 0x4042024CUL)
#define CYREG_SMIF0_CRYPTO_OUTPUT0      ((volatile un_SMIF_CRYPTO_OUTPUT0_t*) 0x40420260UL)
#define CYREG_SMIF0_CRYPTO_OUTPUT1      ((volatile un_SMIF_CRYPTO_OUTPUT1_t*) 0x40420264UL)
#define CYREG_SMIF0_CRYPTO_OUTPUT2      ((volatile un_SMIF_CRYPTO_OUTPUT2_t*) 0x40420268UL)
#define CYREG_SMIF0_CRYPTO_OUTPUT3      ((volatile un_SMIF_CRYPTO_OUTPUT3_t*) 0x4042026CUL)
#define CYREG_SMIF0_INTR                ((volatile un_SMIF_INTR_t*) 0x404207C0UL)
#define CYREG_SMIF0_INTR_SET            ((volatile un_SMIF_INTR_SET_t*) 0x404207C4UL)
#define CYREG_SMIF0_INTR_MASK           ((volatile un_SMIF_INTR_MASK_t*) 0x404207C8UL)
#define CYREG_SMIF0_INTR_MASKED         ((volatile un_SMIF_INTR_MASKED_t*) 0x404207CCUL)

#endif /* _CYREG_SMIF_H_ */


/* [] END OF FILE */
