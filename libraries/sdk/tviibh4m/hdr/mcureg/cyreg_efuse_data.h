/***************************************************************************//**
* \file cyreg_efuse_data.h
*
* \brief
* EFUSE_DATA register definition header
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

#ifndef _CYREG_EFUSE_DATA_H_
#define _CYREG_EFUSE_DATA_H_

#include "cyip_efuse_data_v2.h"

/**
  * \brief eFUSE memory (EFUSE_DATA)
  */
#define CYREG_EFUSE_DATA_SECURE_HASH_WORD0 ((volatile un_EFUSE_DATA_SECURE_HASH_WORD0_t*) 0x402C082CUL)
#define CYREG_EFUSE_DATA_SECURE_HASH_WORD1 ((volatile un_EFUSE_DATA_SECURE_HASH_WORD1_t*) 0x402C0830UL)
#define CYREG_EFUSE_DATA_SECURE_HASH_WORD2 ((volatile un_EFUSE_DATA_SECURE_HASH_WORD2_t*) 0x402C0834UL)
#define CYREG_EFUSE_DATA_SECURE_HASH_WORD3 ((volatile un_EFUSE_DATA_SECURE_HASH_WORD3_t*) 0x402C0838UL)
#define CYREG_EFUSE_DATA_SECURE_ACCESS_RESTRICT ((volatile un_EFUSE_DATA_SECURE_ACCESS_RESTRICT_t*) 0x402C083CUL)
#define CYREG_EFUSE_DATA_SECURE_DEAD_ACCESS_RESTRICT_ZEROS ((volatile un_EFUSE_DATA_SECURE_DEAD_ACCESS_RESTRICT_ZEROS_t*) 0x402C0840UL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA0 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C0868UL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA1 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C086CUL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA2 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C0870UL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA3 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C0874UL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA4 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C0878UL)
#define CYREG_EFUSE_DATA_CUSTOMER_DATA5 ((volatile un_EFUSE_DATA_CUSTOMER_DATA_t*) 0x402C087CUL)

#endif /* _CYREG_EFUSE_DATA_H_ */


/* [] END OF FILE */
