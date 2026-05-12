/***************************************************************************//**
* \file cyreg_efuse.h
*
* \brief
* EFUSE register definition header
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

#ifndef _CYREG_EFUSE_H_
#define _CYREG_EFUSE_H_

#include "cyip_efuse_v2.h"

/**
  * \brief EFUSE MXS40 registers (EFUSE)
  */
#define CYREG_EFUSE_CTL                 ((volatile un_EFUSE_CTL_t*) 0x402C0000UL)
#define CYREG_EFUSE_TEST                ((volatile un_EFUSE_TEST_t*) 0x402C0004UL)
#define CYREG_EFUSE_CMD                 ((volatile un_EFUSE_CMD_t*) 0x402C0010UL)
#define CYREG_EFUSE_SEQ_DEFAULT         ((volatile un_EFUSE_SEQ_DEFAULT_t*) 0x402C0020UL)
#define CYREG_EFUSE_SEQ_READ_CTL_0      ((volatile un_EFUSE_SEQ_READ_CTL_0_t*) 0x402C0040UL)
#define CYREG_EFUSE_SEQ_READ_CTL_1      ((volatile un_EFUSE_SEQ_READ_CTL_1_t*) 0x402C0044UL)
#define CYREG_EFUSE_SEQ_READ_CTL_2      ((volatile un_EFUSE_SEQ_READ_CTL_2_t*) 0x402C0048UL)
#define CYREG_EFUSE_SEQ_READ_CTL_3      ((volatile un_EFUSE_SEQ_READ_CTL_3_t*) 0x402C004CUL)
#define CYREG_EFUSE_SEQ_READ_CTL_4      ((volatile un_EFUSE_SEQ_READ_CTL_4_t*) 0x402C0050UL)
#define CYREG_EFUSE_SEQ_READ_CTL_5      ((volatile un_EFUSE_SEQ_READ_CTL_5_t*) 0x402C0054UL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_0   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_0_t*) 0x402C0060UL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_1   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_1_t*) 0x402C0064UL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_2   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_2_t*) 0x402C0068UL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_3   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_3_t*) 0x402C006CUL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_4   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_4_t*) 0x402C0070UL)
#define CYREG_EFUSE_SEQ_PROGRAM_CTL_5   ((volatile un_EFUSE_SEQ_PROGRAM_CTL_5_t*) 0x402C0074UL)

#endif /* _CYREG_EFUSE_H_ */


/* [] END OF FILE */
