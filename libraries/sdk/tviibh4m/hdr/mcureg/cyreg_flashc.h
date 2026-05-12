/***************************************************************************//**
* \file cyreg_flashc.h
*
* \brief
* FLASHC register definition header
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

#ifndef _CYREG_FLASHC_H_
#define _CYREG_FLASHC_H_

#include "cyip_flashc.h"

/**
  * \brief Flash Macro Registers (FLASHC_FM_CTL_ECT0)
  */
#define CYREG_FLASHC_FM_CTL             ((volatile un_FLASHC_FM_CTL_t*) 0x4024F000UL)
#define CYREG_FLASHC_FM_CODE_MARGIN     ((volatile un_FLASHC_FM_CODE_MARGIN_t*) 0x4024F004UL)
#define CYREG_FLASHC_FM_ADDR            ((volatile un_FLASHC_FM_ADDR_t*) 0x4024F008UL)
#define CYREG_FLASHC_INTR               ((volatile un_FLASHC_INTR_t*) 0x4024F020UL)
#define CYREG_FLASHC_INTR_SET           ((volatile un_FLASHC_INTR_SET_t*) 0x4024F024UL)
#define CYREG_FLASHC_INTR_MASK          ((volatile un_FLASHC_INTR_MASK_t*) 0x4024F028UL)
#define CYREG_FLASHC_INTR_MASKED        ((volatile un_FLASHC_INTR_MASKED_t*) 0x4024F02CUL)
#define CYREG_FLASHC_ECC_OVERRIDE       ((volatile un_FLASHC_ECC_OVERRIDE_t*) 0x4024F030UL)
#define CYREG_FLASHC_FM_DATA            ((volatile un_FLASHC_FM_DATA_t*) 0x4024F040UL)
#define CYREG_FLASHC_BOOKMARK           ((volatile un_FLASHC_BOOKMARK_t*) 0x4024F064UL)
#define CYREG_FLASHC_MAIN_FLASH_SAFETY  ((volatile un_FLASHC_MAIN_FLASH_SAFETY_t*) 0x4024F400UL)
#define CYREG_FLASHC_STATUS             ((volatile un_FLASHC_STATUS_t*) 0x4024F404UL)
#define CYREG_FLASHC_WORK_FLASH_SAFETY  ((volatile un_FLASHC_WORK_FLASH_SAFETY_t*) 0x4024F500UL)

/**
  * \brief Flash controller (FLASHC0)
  */
#define CYREG_FLASHC_FLASH_CTL          ((volatile un_FLASHC_FLASH_CTL_t*) 0x40240000UL)
#define CYREG_FLASHC_FLASH_PWR_CTL      ((volatile un_FLASHC_FLASH_PWR_CTL_t*) 0x40240004UL)
#define CYREG_FLASHC_FLASH_CMD          ((volatile un_FLASHC_FLASH_CMD_t*) 0x40240008UL)
#define CYREG_FLASHC_ECC_CTL            ((volatile un_FLASHC_ECC_CTL_t*) 0x402402A0UL)
#define CYREG_FLASHC_FM_SRAM_ECC_CTL0   ((volatile un_FLASHC_FM_SRAM_ECC_CTL0_t*) 0x402402B0UL)
#define CYREG_FLASHC_FM_SRAM_ECC_CTL1   ((volatile un_FLASHC_FM_SRAM_ECC_CTL1_t*) 0x402402B4UL)
#define CYREG_FLASHC_FM_SRAM_ECC_CTL2   ((volatile un_FLASHC_FM_SRAM_ECC_CTL2_t*) 0x402402B8UL)
#define CYREG_FLASHC_FM_SRAM_ECC_CTL3   ((volatile un_FLASHC_FM_SRAM_ECC_CTL3_t*) 0x402402BCUL)
#define CYREG_FLASHC_CM0_CA_CTL0        ((volatile un_FLASHC_CM0_CA_CTL0_t*) 0x40240400UL)
#define CYREG_FLASHC_CM0_CA_CTL1        ((volatile un_FLASHC_CM0_CA_CTL1_t*) 0x40240404UL)
#define CYREG_FLASHC_CM0_CA_CTL2        ((volatile un_FLASHC_CM0_CA_CTL2_t*) 0x40240408UL)
#define CYREG_FLASHC_CM0_CA_STATUS0     ((volatile un_FLASHC_CM0_CA_STATUS0_t*) 0x40240440UL)
#define CYREG_FLASHC_CM0_CA_STATUS1     ((volatile un_FLASHC_CM0_CA_STATUS1_t*) 0x40240444UL)
#define CYREG_FLASHC_CM0_CA_STATUS2     ((volatile un_FLASHC_CM0_CA_STATUS2_t*) 0x40240448UL)
#define CYREG_FLASHC_CM0_STATUS         ((volatile un_FLASHC_CM0_STATUS_t*) 0x40240460UL)
#define CYREG_FLASHC_CM7_0_STATUS       ((volatile un_FLASHC_CM7_0_STATUS_t*) 0x402404E0UL)
#define CYREG_FLASHC_CM7_1_STATUS       ((volatile un_FLASHC_CM7_1_STATUS_t*) 0x40240560UL)
#define CYREG_FLASHC_CRYPTO_BUFF_CTL    ((volatile un_FLASHC_CRYPTO_BUFF_CTL_t*) 0x40240580UL)
#define CYREG_FLASHC_DW0_BUFF_CTL       ((volatile un_FLASHC_DW0_BUFF_CTL_t*) 0x40240600UL)
#define CYREG_FLASHC_DW1_BUFF_CTL       ((volatile un_FLASHC_DW1_BUFF_CTL_t*) 0x40240680UL)
#define CYREG_FLASHC_DMAC_BUFF_CTL      ((volatile un_FLASHC_DMAC_BUFF_CTL_t*) 0x40240700UL)
#define CYREG_FLASHC_SLOW0_MS_BUFF_CTL  ((volatile un_FLASHC_SLOW0_MS_BUFF_CTL_t*) 0x40240780UL)
#define CYREG_FLASHC_SLOW1_MS_BUFF_CTL  ((volatile un_FLASHC_SLOW1_MS_BUFF_CTL_t*) 0x40240800UL)

#endif /* _CYREG_FLASHC_H_ */


/* [] END OF FILE */
