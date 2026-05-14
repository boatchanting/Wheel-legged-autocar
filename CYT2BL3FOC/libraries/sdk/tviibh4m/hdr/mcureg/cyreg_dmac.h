/***************************************************************************//**
* \file cyreg_dmac.h
*
* \brief
* DMAC register definition header
*
* \note
* Generator version: 1.6.0.217
* Database revision: TVIIBE4M_WW2014_BTO
*
********************************************************************************
* \copyright
* Copyright 2016-2020, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CYREG_DMAC_H_
#define _CYREG_DMAC_H_

#include "cyip_dmac_v2.h"

/**
  * \brief DMA controller channel (DMAC_CH0)
  */
#define CYREG_DMAC_CH0_CTL              ((volatile un_DMAC_CH_CTL_t*) 0x402A1000UL)
#define CYREG_DMAC_CH0_IDX              ((volatile un_DMAC_CH_IDX_t*) 0x402A1010UL)
#define CYREG_DMAC_CH0_SRC              ((volatile un_DMAC_CH_SRC_t*) 0x402A1014UL)
#define CYREG_DMAC_CH0_DST              ((volatile un_DMAC_CH_DST_t*) 0x402A1018UL)
#define CYREG_DMAC_CH0_CURR             ((volatile un_DMAC_CH_CURR_t*) 0x402A1020UL)
#define CYREG_DMAC_CH0_TR_CMD           ((volatile un_DMAC_CH_TR_CMD_t*) 0x402A1028UL)
#define CYREG_DMAC_CH0_DESCR_STATUS     ((volatile un_DMAC_CH_DESCR_STATUS_t*) 0x402A1040UL)
#define CYREG_DMAC_CH0_DESCR_CTL        ((volatile un_DMAC_CH_DESCR_CTL_t*) 0x402A1060UL)
#define CYREG_DMAC_CH0_DESCR_SRC        ((volatile un_DMAC_CH_DESCR_SRC_t*) 0x402A1064UL)
#define CYREG_DMAC_CH0_DESCR_DST        ((volatile un_DMAC_CH_DESCR_DST_t*) 0x402A1068UL)
#define CYREG_DMAC_CH0_DESCR_X_SIZE     ((volatile un_DMAC_CH_DESCR_X_SIZE_t*) 0x402A106CUL)
#define CYREG_DMAC_CH0_DESCR_X_INCR     ((volatile un_DMAC_CH_DESCR_X_INCR_t*) 0x402A1070UL)
#define CYREG_DMAC_CH0_DESCR_Y_SIZE     ((volatile un_DMAC_CH_DESCR_Y_SIZE_t*) 0x402A1074UL)
#define CYREG_DMAC_CH0_DESCR_Y_INCR     ((volatile un_DMAC_CH_DESCR_Y_INCR_t*) 0x402A1078UL)
#define CYREG_DMAC_CH0_DESCR_NEXT       ((volatile un_DMAC_CH_DESCR_NEXT_t*) 0x402A107CUL)
#define CYREG_DMAC_CH0_INTR             ((volatile un_DMAC_CH_INTR_t*) 0x402A1080UL)
#define CYREG_DMAC_CH0_INTR_SET         ((volatile un_DMAC_CH_INTR_SET_t*) 0x402A1084UL)
#define CYREG_DMAC_CH0_INTR_MASK        ((volatile un_DMAC_CH_INTR_MASK_t*) 0x402A1088UL)
#define CYREG_DMAC_CH0_INTR_MASKED      ((volatile un_DMAC_CH_INTR_MASKED_t*) 0x402A108CUL)

/**
  * \brief DMA controller channel (DMAC_CH1)
  */
#define CYREG_DMAC_CH1_CTL              ((volatile un_DMAC_CH_CTL_t*) 0x402A1100UL)
#define CYREG_DMAC_CH1_IDX              ((volatile un_DMAC_CH_IDX_t*) 0x402A1110UL)
#define CYREG_DMAC_CH1_SRC              ((volatile un_DMAC_CH_SRC_t*) 0x402A1114UL)
#define CYREG_DMAC_CH1_DST              ((volatile un_DMAC_CH_DST_t*) 0x402A1118UL)
#define CYREG_DMAC_CH1_CURR             ((volatile un_DMAC_CH_CURR_t*) 0x402A1120UL)
#define CYREG_DMAC_CH1_TR_CMD           ((volatile un_DMAC_CH_TR_CMD_t*) 0x402A1128UL)
#define CYREG_DMAC_CH1_DESCR_STATUS     ((volatile un_DMAC_CH_DESCR_STATUS_t*) 0x402A1140UL)
#define CYREG_DMAC_CH1_DESCR_CTL        ((volatile un_DMAC_CH_DESCR_CTL_t*) 0x402A1160UL)
#define CYREG_DMAC_CH1_DESCR_SRC        ((volatile un_DMAC_CH_DESCR_SRC_t*) 0x402A1164UL)
#define CYREG_DMAC_CH1_DESCR_DST        ((volatile un_DMAC_CH_DESCR_DST_t*) 0x402A1168UL)
#define CYREG_DMAC_CH1_DESCR_X_SIZE     ((volatile un_DMAC_CH_DESCR_X_SIZE_t*) 0x402A116CUL)
#define CYREG_DMAC_CH1_DESCR_X_INCR     ((volatile un_DMAC_CH_DESCR_X_INCR_t*) 0x402A1170UL)
#define CYREG_DMAC_CH1_DESCR_Y_SIZE     ((volatile un_DMAC_CH_DESCR_Y_SIZE_t*) 0x402A1174UL)
#define CYREG_DMAC_CH1_DESCR_Y_INCR     ((volatile un_DMAC_CH_DESCR_Y_INCR_t*) 0x402A1178UL)
#define CYREG_DMAC_CH1_DESCR_NEXT       ((volatile un_DMAC_CH_DESCR_NEXT_t*) 0x402A117CUL)
#define CYREG_DMAC_CH1_INTR             ((volatile un_DMAC_CH_INTR_t*) 0x402A1180UL)
#define CYREG_DMAC_CH1_INTR_SET         ((volatile un_DMAC_CH_INTR_SET_t*) 0x402A1184UL)
#define CYREG_DMAC_CH1_INTR_MASK        ((volatile un_DMAC_CH_INTR_MASK_t*) 0x402A1188UL)
#define CYREG_DMAC_CH1_INTR_MASKED      ((volatile un_DMAC_CH_INTR_MASKED_t*) 0x402A118CUL)

/**
  * \brief DMA controller channel (DMAC_CH2)
  */
#define CYREG_DMAC_CH2_CTL              ((volatile un_DMAC_CH_CTL_t*) 0x402A1200UL)
#define CYREG_DMAC_CH2_IDX              ((volatile un_DMAC_CH_IDX_t*) 0x402A1210UL)
#define CYREG_DMAC_CH2_SRC              ((volatile un_DMAC_CH_SRC_t*) 0x402A1214UL)
#define CYREG_DMAC_CH2_DST              ((volatile un_DMAC_CH_DST_t*) 0x402A1218UL)
#define CYREG_DMAC_CH2_CURR             ((volatile un_DMAC_CH_CURR_t*) 0x402A1220UL)
#define CYREG_DMAC_CH2_TR_CMD           ((volatile un_DMAC_CH_TR_CMD_t*) 0x402A1228UL)
#define CYREG_DMAC_CH2_DESCR_STATUS     ((volatile un_DMAC_CH_DESCR_STATUS_t*) 0x402A1240UL)
#define CYREG_DMAC_CH2_DESCR_CTL        ((volatile un_DMAC_CH_DESCR_CTL_t*) 0x402A1260UL)
#define CYREG_DMAC_CH2_DESCR_SRC        ((volatile un_DMAC_CH_DESCR_SRC_t*) 0x402A1264UL)
#define CYREG_DMAC_CH2_DESCR_DST        ((volatile un_DMAC_CH_DESCR_DST_t*) 0x402A1268UL)
#define CYREG_DMAC_CH2_DESCR_X_SIZE     ((volatile un_DMAC_CH_DESCR_X_SIZE_t*) 0x402A126CUL)
#define CYREG_DMAC_CH2_DESCR_X_INCR     ((volatile un_DMAC_CH_DESCR_X_INCR_t*) 0x402A1270UL)
#define CYREG_DMAC_CH2_DESCR_Y_SIZE     ((volatile un_DMAC_CH_DESCR_Y_SIZE_t*) 0x402A1274UL)
#define CYREG_DMAC_CH2_DESCR_Y_INCR     ((volatile un_DMAC_CH_DESCR_Y_INCR_t*) 0x402A1278UL)
#define CYREG_DMAC_CH2_DESCR_NEXT       ((volatile un_DMAC_CH_DESCR_NEXT_t*) 0x402A127CUL)
#define CYREG_DMAC_CH2_INTR             ((volatile un_DMAC_CH_INTR_t*) 0x402A1280UL)
#define CYREG_DMAC_CH2_INTR_SET         ((volatile un_DMAC_CH_INTR_SET_t*) 0x402A1284UL)
#define CYREG_DMAC_CH2_INTR_MASK        ((volatile un_DMAC_CH_INTR_MASK_t*) 0x402A1288UL)
#define CYREG_DMAC_CH2_INTR_MASKED      ((volatile un_DMAC_CH_INTR_MASKED_t*) 0x402A128CUL)

/**
  * \brief DMA controller channel (DMAC_CH3)
  */
#define CYREG_DMAC_CH3_CTL              ((volatile un_DMAC_CH_CTL_t*) 0x402A1300UL)
#define CYREG_DMAC_CH3_IDX              ((volatile un_DMAC_CH_IDX_t*) 0x402A1310UL)
#define CYREG_DMAC_CH3_SRC              ((volatile un_DMAC_CH_SRC_t*) 0x402A1314UL)
#define CYREG_DMAC_CH3_DST              ((volatile un_DMAC_CH_DST_t*) 0x402A1318UL)
#define CYREG_DMAC_CH3_CURR             ((volatile un_DMAC_CH_CURR_t*) 0x402A1320UL)
#define CYREG_DMAC_CH3_TR_CMD           ((volatile un_DMAC_CH_TR_CMD_t*) 0x402A1328UL)
#define CYREG_DMAC_CH3_DESCR_STATUS     ((volatile un_DMAC_CH_DESCR_STATUS_t*) 0x402A1340UL)
#define CYREG_DMAC_CH3_DESCR_CTL        ((volatile un_DMAC_CH_DESCR_CTL_t*) 0x402A1360UL)
#define CYREG_DMAC_CH3_DESCR_SRC        ((volatile un_DMAC_CH_DESCR_SRC_t*) 0x402A1364UL)
#define CYREG_DMAC_CH3_DESCR_DST        ((volatile un_DMAC_CH_DESCR_DST_t*) 0x402A1368UL)
#define CYREG_DMAC_CH3_DESCR_X_SIZE     ((volatile un_DMAC_CH_DESCR_X_SIZE_t*) 0x402A136CUL)
#define CYREG_DMAC_CH3_DESCR_X_INCR     ((volatile un_DMAC_CH_DESCR_X_INCR_t*) 0x402A1370UL)
#define CYREG_DMAC_CH3_DESCR_Y_SIZE     ((volatile un_DMAC_CH_DESCR_Y_SIZE_t*) 0x402A1374UL)
#define CYREG_DMAC_CH3_DESCR_Y_INCR     ((volatile un_DMAC_CH_DESCR_Y_INCR_t*) 0x402A1378UL)
#define CYREG_DMAC_CH3_DESCR_NEXT       ((volatile un_DMAC_CH_DESCR_NEXT_t*) 0x402A137CUL)
#define CYREG_DMAC_CH3_INTR             ((volatile un_DMAC_CH_INTR_t*) 0x402A1380UL)
#define CYREG_DMAC_CH3_INTR_SET         ((volatile un_DMAC_CH_INTR_SET_t*) 0x402A1384UL)
#define CYREG_DMAC_CH3_INTR_MASK        ((volatile un_DMAC_CH_INTR_MASK_t*) 0x402A1388UL)
#define CYREG_DMAC_CH3_INTR_MASKED      ((volatile un_DMAC_CH_INTR_MASKED_t*) 0x402A138CUL)

/**
  * \brief DMAC (DMAC0)
  */
#define CYREG_DMAC_CTL                  ((volatile un_DMAC_CTL_t*) 0x402A0000UL)
#define CYREG_DMAC_ACTIVE               ((volatile un_DMAC_ACTIVE_t*) 0x402A0008UL)

#endif /* _CYREG_DMAC_H_ */


/* [] END OF FILE */
