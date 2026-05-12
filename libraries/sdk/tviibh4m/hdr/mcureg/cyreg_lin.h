/***************************************************************************//**
* \file cyreg_lin.h
*
* \brief
* LIN register definition header
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

#ifndef _CYREG_LIN_H_
#define _CYREG_LIN_H_

#include "cyip_lin.h"

/**
  * \brief LIN channel structure (LIN_CH0)
  */
#define CYREG_LIN0_CH0_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508000UL)
#define CYREG_LIN0_CH0_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508004UL)
#define CYREG_LIN0_CH0_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508008UL)
#define CYREG_LIN0_CH0_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508010UL)
#define CYREG_LIN0_CH0_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508060UL)
#define CYREG_LIN0_CH0_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508080UL)
#define CYREG_LIN0_CH0_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508084UL)
#define CYREG_LIN0_CH0_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508088UL)
#define CYREG_LIN0_CH0_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405080C0UL)
#define CYREG_LIN0_CH0_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405080C4UL)
#define CYREG_LIN0_CH0_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405080C8UL)
#define CYREG_LIN0_CH0_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405080CCUL)

/**
  * \brief LIN channel structure (LIN_CH1)
  */
#define CYREG_LIN0_CH1_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508100UL)
#define CYREG_LIN0_CH1_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508104UL)
#define CYREG_LIN0_CH1_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508108UL)
#define CYREG_LIN0_CH1_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508110UL)
#define CYREG_LIN0_CH1_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508160UL)
#define CYREG_LIN0_CH1_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508180UL)
#define CYREG_LIN0_CH1_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508184UL)
#define CYREG_LIN0_CH1_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508188UL)
#define CYREG_LIN0_CH1_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405081C0UL)
#define CYREG_LIN0_CH1_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405081C4UL)
#define CYREG_LIN0_CH1_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405081C8UL)
#define CYREG_LIN0_CH1_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405081CCUL)

/**
  * \brief LIN channel structure (LIN_CH2)
  */
#define CYREG_LIN0_CH2_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508200UL)
#define CYREG_LIN0_CH2_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508204UL)
#define CYREG_LIN0_CH2_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508208UL)
#define CYREG_LIN0_CH2_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508210UL)
#define CYREG_LIN0_CH2_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508260UL)
#define CYREG_LIN0_CH2_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508280UL)
#define CYREG_LIN0_CH2_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508284UL)
#define CYREG_LIN0_CH2_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508288UL)
#define CYREG_LIN0_CH2_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405082C0UL)
#define CYREG_LIN0_CH2_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405082C4UL)
#define CYREG_LIN0_CH2_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405082C8UL)
#define CYREG_LIN0_CH2_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405082CCUL)

/**
  * \brief LIN channel structure (LIN_CH3)
  */
#define CYREG_LIN0_CH3_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508300UL)
#define CYREG_LIN0_CH3_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508304UL)
#define CYREG_LIN0_CH3_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508308UL)
#define CYREG_LIN0_CH3_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508310UL)
#define CYREG_LIN0_CH3_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508360UL)
#define CYREG_LIN0_CH3_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508380UL)
#define CYREG_LIN0_CH3_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508384UL)
#define CYREG_LIN0_CH3_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508388UL)
#define CYREG_LIN0_CH3_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405083C0UL)
#define CYREG_LIN0_CH3_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405083C4UL)
#define CYREG_LIN0_CH3_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405083C8UL)
#define CYREG_LIN0_CH3_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405083CCUL)

/**
  * \brief LIN channel structure (LIN_CH4)
  */
#define CYREG_LIN0_CH4_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508400UL)
#define CYREG_LIN0_CH4_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508404UL)
#define CYREG_LIN0_CH4_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508408UL)
#define CYREG_LIN0_CH4_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508410UL)
#define CYREG_LIN0_CH4_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508460UL)
#define CYREG_LIN0_CH4_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508480UL)
#define CYREG_LIN0_CH4_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508484UL)
#define CYREG_LIN0_CH4_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508488UL)
#define CYREG_LIN0_CH4_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405084C0UL)
#define CYREG_LIN0_CH4_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405084C4UL)
#define CYREG_LIN0_CH4_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405084C8UL)
#define CYREG_LIN0_CH4_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405084CCUL)

/**
  * \brief LIN channel structure (LIN_CH5)
  */
#define CYREG_LIN0_CH5_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508500UL)
#define CYREG_LIN0_CH5_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508504UL)
#define CYREG_LIN0_CH5_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508508UL)
#define CYREG_LIN0_CH5_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508510UL)
#define CYREG_LIN0_CH5_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508560UL)
#define CYREG_LIN0_CH5_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508580UL)
#define CYREG_LIN0_CH5_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508584UL)
#define CYREG_LIN0_CH5_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508588UL)
#define CYREG_LIN0_CH5_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405085C0UL)
#define CYREG_LIN0_CH5_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405085C4UL)
#define CYREG_LIN0_CH5_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405085C8UL)
#define CYREG_LIN0_CH5_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405085CCUL)

/**
  * \brief LIN channel structure (LIN_CH6)
  */
#define CYREG_LIN0_CH6_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508600UL)
#define CYREG_LIN0_CH6_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508604UL)
#define CYREG_LIN0_CH6_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508608UL)
#define CYREG_LIN0_CH6_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508610UL)
#define CYREG_LIN0_CH6_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508660UL)
#define CYREG_LIN0_CH6_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508680UL)
#define CYREG_LIN0_CH6_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508684UL)
#define CYREG_LIN0_CH6_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508688UL)
#define CYREG_LIN0_CH6_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405086C0UL)
#define CYREG_LIN0_CH6_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405086C4UL)
#define CYREG_LIN0_CH6_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405086C8UL)
#define CYREG_LIN0_CH6_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405086CCUL)

/**
  * \brief LIN channel structure (LIN_CH7)
  */
#define CYREG_LIN0_CH7_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508700UL)
#define CYREG_LIN0_CH7_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508704UL)
#define CYREG_LIN0_CH7_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508708UL)
#define CYREG_LIN0_CH7_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508710UL)
#define CYREG_LIN0_CH7_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508760UL)
#define CYREG_LIN0_CH7_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508780UL)
#define CYREG_LIN0_CH7_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508784UL)
#define CYREG_LIN0_CH7_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508788UL)
#define CYREG_LIN0_CH7_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405087C0UL)
#define CYREG_LIN0_CH7_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405087C4UL)
#define CYREG_LIN0_CH7_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405087C8UL)
#define CYREG_LIN0_CH7_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405087CCUL)

/**
  * \brief LIN channel structure (LIN_CH8)
  */
#define CYREG_LIN0_CH8_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508800UL)
#define CYREG_LIN0_CH8_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508804UL)
#define CYREG_LIN0_CH8_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508808UL)
#define CYREG_LIN0_CH8_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508810UL)
#define CYREG_LIN0_CH8_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508860UL)
#define CYREG_LIN0_CH8_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508880UL)
#define CYREG_LIN0_CH8_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508884UL)
#define CYREG_LIN0_CH8_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508888UL)
#define CYREG_LIN0_CH8_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405088C0UL)
#define CYREG_LIN0_CH8_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405088C4UL)
#define CYREG_LIN0_CH8_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405088C8UL)
#define CYREG_LIN0_CH8_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405088CCUL)

/**
  * \brief LIN channel structure (LIN_CH9)
  */
#define CYREG_LIN0_CH9_CTL0             ((volatile un_LIN_CH_CTL0_t*) 0x40508900UL)
#define CYREG_LIN0_CH9_CTL1             ((volatile un_LIN_CH_CTL1_t*) 0x40508904UL)
#define CYREG_LIN0_CH9_STATUS           ((volatile un_LIN_CH_STATUS_t*) 0x40508908UL)
#define CYREG_LIN0_CH9_CMD              ((volatile un_LIN_CH_CMD_t*) 0x40508910UL)
#define CYREG_LIN0_CH9_TX_RX_STATUS     ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508960UL)
#define CYREG_LIN0_CH9_PID_CHECKSUM     ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508980UL)
#define CYREG_LIN0_CH9_DATA0            ((volatile un_LIN_CH_DATA0_t*) 0x40508984UL)
#define CYREG_LIN0_CH9_DATA1            ((volatile un_LIN_CH_DATA1_t*) 0x40508988UL)
#define CYREG_LIN0_CH9_INTR             ((volatile un_LIN_CH_INTR_t*) 0x405089C0UL)
#define CYREG_LIN0_CH9_INTR_SET         ((volatile un_LIN_CH_INTR_SET_t*) 0x405089C4UL)
#define CYREG_LIN0_CH9_INTR_MASK        ((volatile un_LIN_CH_INTR_MASK_t*) 0x405089C8UL)
#define CYREG_LIN0_CH9_INTR_MASKED      ((volatile un_LIN_CH_INTR_MASKED_t*) 0x405089CCUL)

/**
  * \brief LIN channel structure (LIN_CH10)
  */
#define CYREG_LIN0_CH10_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508A00UL)
#define CYREG_LIN0_CH10_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508A04UL)
#define CYREG_LIN0_CH10_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508A08UL)
#define CYREG_LIN0_CH10_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508A10UL)
#define CYREG_LIN0_CH10_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508A60UL)
#define CYREG_LIN0_CH10_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508A80UL)
#define CYREG_LIN0_CH10_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508A84UL)
#define CYREG_LIN0_CH10_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508A88UL)
#define CYREG_LIN0_CH10_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508AC0UL)
#define CYREG_LIN0_CH10_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508AC4UL)
#define CYREG_LIN0_CH10_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508AC8UL)
#define CYREG_LIN0_CH10_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508ACCUL)

/**
  * \brief LIN channel structure (LIN_CH11)
  */
#define CYREG_LIN0_CH11_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508B00UL)
#define CYREG_LIN0_CH11_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508B04UL)
#define CYREG_LIN0_CH11_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508B08UL)
#define CYREG_LIN0_CH11_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508B10UL)
#define CYREG_LIN0_CH11_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508B60UL)
#define CYREG_LIN0_CH11_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508B80UL)
#define CYREG_LIN0_CH11_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508B84UL)
#define CYREG_LIN0_CH11_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508B88UL)
#define CYREG_LIN0_CH11_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508BC0UL)
#define CYREG_LIN0_CH11_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508BC4UL)
#define CYREG_LIN0_CH11_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508BC8UL)
#define CYREG_LIN0_CH11_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508BCCUL)

/**
  * \brief LIN channel structure (LIN_CH12)
  */
#define CYREG_LIN0_CH12_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508C00UL)
#define CYREG_LIN0_CH12_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508C04UL)
#define CYREG_LIN0_CH12_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508C08UL)
#define CYREG_LIN0_CH12_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508C10UL)
#define CYREG_LIN0_CH12_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508C60UL)
#define CYREG_LIN0_CH12_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508C80UL)
#define CYREG_LIN0_CH12_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508C84UL)
#define CYREG_LIN0_CH12_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508C88UL)
#define CYREG_LIN0_CH12_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508CC0UL)
#define CYREG_LIN0_CH12_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508CC4UL)
#define CYREG_LIN0_CH12_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508CC8UL)
#define CYREG_LIN0_CH12_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508CCCUL)

/**
  * \brief LIN channel structure (LIN_CH13)
  */
#define CYREG_LIN0_CH13_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508D00UL)
#define CYREG_LIN0_CH13_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508D04UL)
#define CYREG_LIN0_CH13_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508D08UL)
#define CYREG_LIN0_CH13_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508D10UL)
#define CYREG_LIN0_CH13_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508D60UL)
#define CYREG_LIN0_CH13_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508D80UL)
#define CYREG_LIN0_CH13_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508D84UL)
#define CYREG_LIN0_CH13_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508D88UL)
#define CYREG_LIN0_CH13_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508DC0UL)
#define CYREG_LIN0_CH13_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508DC4UL)
#define CYREG_LIN0_CH13_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508DC8UL)
#define CYREG_LIN0_CH13_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508DCCUL)

/**
  * \brief LIN channel structure (LIN_CH14)
  */
#define CYREG_LIN0_CH14_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508E00UL)
#define CYREG_LIN0_CH14_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508E04UL)
#define CYREG_LIN0_CH14_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508E08UL)
#define CYREG_LIN0_CH14_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508E10UL)
#define CYREG_LIN0_CH14_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508E60UL)
#define CYREG_LIN0_CH14_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508E80UL)
#define CYREG_LIN0_CH14_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508E84UL)
#define CYREG_LIN0_CH14_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508E88UL)
#define CYREG_LIN0_CH14_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508EC0UL)
#define CYREG_LIN0_CH14_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508EC4UL)
#define CYREG_LIN0_CH14_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508EC8UL)
#define CYREG_LIN0_CH14_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508ECCUL)

/**
  * \brief LIN channel structure (LIN_CH15)
  */
#define CYREG_LIN0_CH15_CTL0            ((volatile un_LIN_CH_CTL0_t*) 0x40508F00UL)
#define CYREG_LIN0_CH15_CTL1            ((volatile un_LIN_CH_CTL1_t*) 0x40508F04UL)
#define CYREG_LIN0_CH15_STATUS          ((volatile un_LIN_CH_STATUS_t*) 0x40508F08UL)
#define CYREG_LIN0_CH15_CMD             ((volatile un_LIN_CH_CMD_t*) 0x40508F10UL)
#define CYREG_LIN0_CH15_TX_RX_STATUS    ((volatile un_LIN_CH_TX_RX_STATUS_t*) 0x40508F60UL)
#define CYREG_LIN0_CH15_PID_CHECKSUM    ((volatile un_LIN_CH_PID_CHECKSUM_t*) 0x40508F80UL)
#define CYREG_LIN0_CH15_DATA0           ((volatile un_LIN_CH_DATA0_t*) 0x40508F84UL)
#define CYREG_LIN0_CH15_DATA1           ((volatile un_LIN_CH_DATA1_t*) 0x40508F88UL)
#define CYREG_LIN0_CH15_INTR            ((volatile un_LIN_CH_INTR_t*) 0x40508FC0UL)
#define CYREG_LIN0_CH15_INTR_SET        ((volatile un_LIN_CH_INTR_SET_t*) 0x40508FC4UL)
#define CYREG_LIN0_CH15_INTR_MASK       ((volatile un_LIN_CH_INTR_MASK_t*) 0x40508FC8UL)
#define CYREG_LIN0_CH15_INTR_MASKED     ((volatile un_LIN_CH_INTR_MASKED_t*) 0x40508FCCUL)

/**
  * \brief LIN (LIN0)
  */
#define CYREG_LIN0_ERROR_CTL            ((volatile un_LIN_ERROR_CTL_t*) 0x40500000UL)
#define CYREG_LIN0_TEST_CTL             ((volatile un_LIN_TEST_CTL_t*) 0x40500004UL)

#endif /* _CYREG_LIN_H_ */


/* [] END OF FILE */
