/***************************************************************************//**
* \file cyreg_sdhc.h
*
* \brief
* SDHC register definition header
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

#ifndef _CYREG_SDHC_H_
#define _CYREG_SDHC_H_

#include "cyip_sdhc.h"

/**
  * \brief MMIO at SDHC wrapper level (SDHC_WRAP0)
  */
#define CYREG_SDHC0_WRAP_CTL            ((volatile un_SDHC_WRAP_CTL_t*) 0x40460000UL)

/**
  * \brief MMIO for Synopsys Mobile Storage Host Controller IP (SDHC_CORE0)
  */
#define CYREG_SDHC0_CORE_SDMASA_R       ((volatile un_SDHC_CORE_SDMASA_R_t*) 0x40461000UL)
#define CYREG_SDHC0_CORE_BLOCKSIZE_R    ((volatile un_SDHC_CORE_BLOCKSIZE_R_t*) 0x40461004UL)
#define CYREG_SDHC0_CORE_BLOCKCOUNT_R   ((volatile un_SDHC_CORE_BLOCKCOUNT_R_t*) 0x40461006UL)
#define CYREG_SDHC0_CORE_ARGUMENT_R     ((volatile un_SDHC_CORE_ARGUMENT_R_t*) 0x40461008UL)
#define CYREG_SDHC0_CORE_XFER_MODE_R    ((volatile un_SDHC_CORE_XFER_MODE_R_t*) 0x4046100CUL)
#define CYREG_SDHC0_CORE_CMD_R          ((volatile un_SDHC_CORE_CMD_R_t*) 0x4046100EUL)
#define CYREG_SDHC0_CORE_RESP01_R       ((volatile un_SDHC_CORE_RESP01_R_t*) 0x40461010UL)
#define CYREG_SDHC0_CORE_RESP23_R       ((volatile un_SDHC_CORE_RESP23_R_t*) 0x40461014UL)
#define CYREG_SDHC0_CORE_RESP45_R       ((volatile un_SDHC_CORE_RESP45_R_t*) 0x40461018UL)
#define CYREG_SDHC0_CORE_RESP67_R       ((volatile un_SDHC_CORE_RESP67_R_t*) 0x4046101CUL)
#define CYREG_SDHC0_CORE_BUF_DATA_R     ((volatile un_SDHC_CORE_BUF_DATA_R_t*) 0x40461020UL)
#define CYREG_SDHC0_CORE_PSTATE_REG     ((volatile un_SDHC_CORE_PSTATE_REG_t*) 0x40461024UL)
#define CYREG_SDHC0_CORE_HOST_CTRL1_R   ((volatile un_SDHC_CORE_HOST_CTRL1_R_t*) 0x40461028UL)
#define CYREG_SDHC0_CORE_PWR_CTRL_R     ((volatile un_SDHC_CORE_PWR_CTRL_R_t*) 0x40461029UL)
#define CYREG_SDHC0_CORE_BGAP_CTRL_R    ((volatile un_SDHC_CORE_BGAP_CTRL_R_t*) 0x4046102AUL)
#define CYREG_SDHC0_CORE_WUP_CTRL_R     ((volatile un_SDHC_CORE_WUP_CTRL_R_t*) 0x4046102BUL)
#define CYREG_SDHC0_CORE_CLK_CTRL_R     ((volatile un_SDHC_CORE_CLK_CTRL_R_t*) 0x4046102CUL)
#define CYREG_SDHC0_CORE_TOUT_CTRL_R    ((volatile un_SDHC_CORE_TOUT_CTRL_R_t*) 0x4046102EUL)
#define CYREG_SDHC0_CORE_SW_RST_R       ((volatile un_SDHC_CORE_SW_RST_R_t*) 0x4046102FUL)
#define CYREG_SDHC0_CORE_NORMAL_INT_STAT_R ((volatile un_SDHC_CORE_NORMAL_INT_STAT_R_t*) 0x40461030UL)
#define CYREG_SDHC0_CORE_ERROR_INT_STAT_R ((volatile un_SDHC_CORE_ERROR_INT_STAT_R_t*) 0x40461032UL)
#define CYREG_SDHC0_CORE_NORMAL_INT_STAT_EN_R ((volatile un_SDHC_CORE_NORMAL_INT_STAT_EN_R_t*) 0x40461034UL)
#define CYREG_SDHC0_CORE_ERROR_INT_STAT_EN_R ((volatile un_SDHC_CORE_ERROR_INT_STAT_EN_R_t*) 0x40461036UL)
#define CYREG_SDHC0_CORE_NORMAL_INT_SIGNAL_EN_R ((volatile un_SDHC_CORE_NORMAL_INT_SIGNAL_EN_R_t*) 0x40461038UL)
#define CYREG_SDHC0_CORE_ERROR_INT_SIGNAL_EN_R ((volatile un_SDHC_CORE_ERROR_INT_SIGNAL_EN_R_t*) 0x4046103AUL)
#define CYREG_SDHC0_CORE_AUTO_CMD_STAT_R ((volatile un_SDHC_CORE_AUTO_CMD_STAT_R_t*) 0x4046103CUL)
#define CYREG_SDHC0_CORE_HOST_CTRL2_R   ((volatile un_SDHC_CORE_HOST_CTRL2_R_t*) 0x4046103EUL)
#define CYREG_SDHC0_CORE_CAPABILITIES1_R ((volatile un_SDHC_CORE_CAPABILITIES1_R_t*) 0x40461040UL)
#define CYREG_SDHC0_CORE_CAPABILITIES2_R ((volatile un_SDHC_CORE_CAPABILITIES2_R_t*) 0x40461044UL)
#define CYREG_SDHC0_CORE_CURR_CAPABILITIES1_R ((volatile un_SDHC_CORE_CURR_CAPABILITIES1_R_t*) 0x40461048UL)
#define CYREG_SDHC0_CORE_CURR_CAPABILITIES2_R ((volatile un_SDHC_CORE_CURR_CAPABILITIES2_R_t*) 0x4046104CUL)
#define CYREG_SDHC0_CORE_FORCE_AUTO_CMD_STAT_R ((volatile un_SDHC_CORE_FORCE_AUTO_CMD_STAT_R_t*) 0x40461050UL)
#define CYREG_SDHC0_CORE_FORCE_ERROR_INT_STAT_R ((volatile un_SDHC_CORE_FORCE_ERROR_INT_STAT_R_t*) 0x40461052UL)
#define CYREG_SDHC0_CORE_ADMA_ERR_STAT_R ((volatile un_SDHC_CORE_ADMA_ERR_STAT_R_t*) 0x40461054UL)
#define CYREG_SDHC0_CORE_ADMA_SA_LOW_R  ((volatile un_SDHC_CORE_ADMA_SA_LOW_R_t*) 0x40461058UL)
#define CYREG_SDHC0_CORE_ADMA_ID_LOW_R  ((volatile un_SDHC_CORE_ADMA_ID_LOW_R_t*) 0x40461078UL)
#define CYREG_SDHC0_CORE_HOST_CNTRL_VERS_R ((volatile un_SDHC_CORE_HOST_CNTRL_VERS_R_t*) 0x404610FEUL)
#define CYREG_SDHC0_CORE_MSHC_VER_ID_R  ((volatile un_SDHC_CORE_MSHC_VER_ID_R_t*) 0x40461500UL)
#define CYREG_SDHC0_CORE_MSHC_VER_TYPE_R ((volatile un_SDHC_CORE_MSHC_VER_TYPE_R_t*) 0x40461504UL)
#define CYREG_SDHC0_CORE_MSHC_CTRL_R    ((volatile un_SDHC_CORE_MSHC_CTRL_R_t*) 0x40461508UL)
#define CYREG_SDHC0_CORE_MBIU_CTRL_R    ((volatile un_SDHC_CORE_MBIU_CTRL_R_t*) 0x40461510UL)
#define CYREG_SDHC0_CORE_EMMC_CTRL_R    ((volatile un_SDHC_CORE_EMMC_CTRL_R_t*) 0x4046152CUL)
#define CYREG_SDHC0_CORE_BOOT_CTRL_R    ((volatile un_SDHC_CORE_BOOT_CTRL_R_t*) 0x4046152EUL)
#define CYREG_SDHC0_CORE_GP_IN_R        ((volatile un_SDHC_CORE_GP_IN_R_t*) 0x40461530UL)
#define CYREG_SDHC0_CORE_GP_OUT_R       ((volatile un_SDHC_CORE_GP_OUT_R_t*) 0x40461534UL)

#endif /* _CYREG_SDHC_H_ */


/* [] END OF FILE */
