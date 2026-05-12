/***************************************************************************//**
* \file cyreg_gpio.h
*
* \brief
* GPIO register definition header
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

#ifndef _CYREG_GPIO_H_
#define _CYREG_GPIO_H_

#include "cyip_gpio_v3.h"

/**
  * \brief GPIO port registers (GPIO_PRT0)
  */
#define CYREG_GPIO_PRT0_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310000UL)
#define CYREG_GPIO_PRT0_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310004UL)
#define CYREG_GPIO_PRT0_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310008UL)
#define CYREG_GPIO_PRT0_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031000CUL)
#define CYREG_GPIO_PRT0_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310010UL)
#define CYREG_GPIO_PRT0_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310014UL)
#define CYREG_GPIO_PRT0_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310018UL)
#define CYREG_GPIO_PRT0_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031001CUL)
#define CYREG_GPIO_PRT0_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310020UL)
#define CYREG_GPIO_PRT0_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310040UL)
#define CYREG_GPIO_PRT0_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x40310044UL)
#define CYREG_GPIO_PRT0_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310048UL)
#define CYREG_GPIO_PRT0_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031004CUL)
#define CYREG_GPIO_PRT0_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310058UL)

/**
  * \brief GPIO port registers (GPIO_PRT1)
  */
#define CYREG_GPIO_PRT1_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310080UL)
#define CYREG_GPIO_PRT1_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310084UL)
#define CYREG_GPIO_PRT1_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310088UL)
#define CYREG_GPIO_PRT1_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031008CUL)
#define CYREG_GPIO_PRT1_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310090UL)
#define CYREG_GPIO_PRT1_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310094UL)
#define CYREG_GPIO_PRT1_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310098UL)
#define CYREG_GPIO_PRT1_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031009CUL)
#define CYREG_GPIO_PRT1_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403100A0UL)
#define CYREG_GPIO_PRT1_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403100C0UL)
#define CYREG_GPIO_PRT1_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x403100C4UL)
#define CYREG_GPIO_PRT1_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403100C8UL)
#define CYREG_GPIO_PRT1_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403100CCUL)
#define CYREG_GPIO_PRT1_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403100D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT2)
  */
#define CYREG_GPIO_PRT2_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310100UL)
#define CYREG_GPIO_PRT2_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310104UL)
#define CYREG_GPIO_PRT2_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310108UL)
#define CYREG_GPIO_PRT2_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031010CUL)
#define CYREG_GPIO_PRT2_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310110UL)
#define CYREG_GPIO_PRT2_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310114UL)
#define CYREG_GPIO_PRT2_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310118UL)
#define CYREG_GPIO_PRT2_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031011CUL)
#define CYREG_GPIO_PRT2_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310120UL)
#define CYREG_GPIO_PRT2_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310140UL)
#define CYREG_GPIO_PRT2_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x40310144UL)
#define CYREG_GPIO_PRT2_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310148UL)
#define CYREG_GPIO_PRT2_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031014CUL)
#define CYREG_GPIO_PRT2_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310158UL)

/**
  * \brief GPIO port registers (GPIO_PRT3)
  */
#define CYREG_GPIO_PRT3_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310180UL)
#define CYREG_GPIO_PRT3_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310184UL)
#define CYREG_GPIO_PRT3_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310188UL)
#define CYREG_GPIO_PRT3_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031018CUL)
#define CYREG_GPIO_PRT3_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310190UL)
#define CYREG_GPIO_PRT3_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310194UL)
#define CYREG_GPIO_PRT3_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310198UL)
#define CYREG_GPIO_PRT3_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031019CUL)
#define CYREG_GPIO_PRT3_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403101A0UL)
#define CYREG_GPIO_PRT3_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403101C0UL)
#define CYREG_GPIO_PRT3_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x403101C4UL)
#define CYREG_GPIO_PRT3_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403101C8UL)
#define CYREG_GPIO_PRT3_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403101CCUL)
#define CYREG_GPIO_PRT3_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403101D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT4)
  */
#define CYREG_GPIO_PRT4_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310200UL)
#define CYREG_GPIO_PRT4_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310204UL)
#define CYREG_GPIO_PRT4_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310208UL)
#define CYREG_GPIO_PRT4_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031020CUL)
#define CYREG_GPIO_PRT4_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310210UL)
#define CYREG_GPIO_PRT4_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310214UL)
#define CYREG_GPIO_PRT4_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310218UL)
#define CYREG_GPIO_PRT4_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031021CUL)
#define CYREG_GPIO_PRT4_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310220UL)
#define CYREG_GPIO_PRT4_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310240UL)
#define CYREG_GPIO_PRT4_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x40310244UL)
#define CYREG_GPIO_PRT4_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310248UL)
#define CYREG_GPIO_PRT4_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031024CUL)
#define CYREG_GPIO_PRT4_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310258UL)

/**
  * \brief GPIO port registers (GPIO_PRT5)
  */
#define CYREG_GPIO_PRT5_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310280UL)
#define CYREG_GPIO_PRT5_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310284UL)
#define CYREG_GPIO_PRT5_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310288UL)
#define CYREG_GPIO_PRT5_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031028CUL)
#define CYREG_GPIO_PRT5_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310290UL)
#define CYREG_GPIO_PRT5_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310294UL)
#define CYREG_GPIO_PRT5_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310298UL)
#define CYREG_GPIO_PRT5_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031029CUL)
#define CYREG_GPIO_PRT5_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403102A0UL)
#define CYREG_GPIO_PRT5_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403102C0UL)
#define CYREG_GPIO_PRT5_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x403102C4UL)
#define CYREG_GPIO_PRT5_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403102C8UL)
#define CYREG_GPIO_PRT5_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403102CCUL)
#define CYREG_GPIO_PRT5_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403102D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT6)
  */
#define CYREG_GPIO_PRT6_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310300UL)
#define CYREG_GPIO_PRT6_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310304UL)
#define CYREG_GPIO_PRT6_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310308UL)
#define CYREG_GPIO_PRT6_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031030CUL)
#define CYREG_GPIO_PRT6_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310310UL)
#define CYREG_GPIO_PRT6_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310314UL)
#define CYREG_GPIO_PRT6_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310318UL)
#define CYREG_GPIO_PRT6_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031031CUL)
#define CYREG_GPIO_PRT6_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310320UL)
#define CYREG_GPIO_PRT6_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310340UL)
#define CYREG_GPIO_PRT6_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x40310344UL)
#define CYREG_GPIO_PRT6_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310348UL)
#define CYREG_GPIO_PRT6_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031034CUL)
#define CYREG_GPIO_PRT6_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310358UL)

/**
  * \brief GPIO port registers (GPIO_PRT7)
  */
#define CYREG_GPIO_PRT7_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310380UL)
#define CYREG_GPIO_PRT7_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310384UL)
#define CYREG_GPIO_PRT7_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310388UL)
#define CYREG_GPIO_PRT7_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031038CUL)
#define CYREG_GPIO_PRT7_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310390UL)
#define CYREG_GPIO_PRT7_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310394UL)
#define CYREG_GPIO_PRT7_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310398UL)
#define CYREG_GPIO_PRT7_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031039CUL)
#define CYREG_GPIO_PRT7_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403103A0UL)
#define CYREG_GPIO_PRT7_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403103C0UL)
#define CYREG_GPIO_PRT7_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x403103C4UL)
#define CYREG_GPIO_PRT7_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403103C8UL)
#define CYREG_GPIO_PRT7_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403103CCUL)
#define CYREG_GPIO_PRT7_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403103D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT8)
  */
#define CYREG_GPIO_PRT8_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310400UL)
#define CYREG_GPIO_PRT8_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310404UL)
#define CYREG_GPIO_PRT8_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310408UL)
#define CYREG_GPIO_PRT8_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031040CUL)
#define CYREG_GPIO_PRT8_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310410UL)
#define CYREG_GPIO_PRT8_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310414UL)
#define CYREG_GPIO_PRT8_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310418UL)
#define CYREG_GPIO_PRT8_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031041CUL)
#define CYREG_GPIO_PRT8_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310420UL)
#define CYREG_GPIO_PRT8_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310440UL)
#define CYREG_GPIO_PRT8_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x40310444UL)
#define CYREG_GPIO_PRT8_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310448UL)
#define CYREG_GPIO_PRT8_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031044CUL)
#define CYREG_GPIO_PRT8_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310458UL)

/**
  * \brief GPIO port registers (GPIO_PRT9)
  */
#define CYREG_GPIO_PRT9_OUT             ((volatile un_GPIO_PRT_OUT_t*) 0x40310480UL)
#define CYREG_GPIO_PRT9_OUT_CLR         ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310484UL)
#define CYREG_GPIO_PRT9_OUT_SET         ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310488UL)
#define CYREG_GPIO_PRT9_OUT_INV         ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031048CUL)
#define CYREG_GPIO_PRT9_IN              ((volatile un_GPIO_PRT_IN_t*) 0x40310490UL)
#define CYREG_GPIO_PRT9_INTR            ((volatile un_GPIO_PRT_INTR_t*) 0x40310494UL)
#define CYREG_GPIO_PRT9_INTR_MASK       ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310498UL)
#define CYREG_GPIO_PRT9_INTR_MASKED     ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031049CUL)
#define CYREG_GPIO_PRT9_INTR_SET        ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403104A0UL)
#define CYREG_GPIO_PRT9_INTR_CFG        ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403104C0UL)
#define CYREG_GPIO_PRT9_CFG             ((volatile un_GPIO_PRT_CFG_t*) 0x403104C4UL)
#define CYREG_GPIO_PRT9_CFG_IN          ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403104C8UL)
#define CYREG_GPIO_PRT9_CFG_OUT         ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403104CCUL)
#define CYREG_GPIO_PRT9_CFG_IN_AUTOLVL  ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403104D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT10)
  */
#define CYREG_GPIO_PRT10_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310500UL)
#define CYREG_GPIO_PRT10_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310504UL)
#define CYREG_GPIO_PRT10_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310508UL)
#define CYREG_GPIO_PRT10_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031050CUL)
#define CYREG_GPIO_PRT10_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310510UL)
#define CYREG_GPIO_PRT10_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310514UL)
#define CYREG_GPIO_PRT10_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310518UL)
#define CYREG_GPIO_PRT10_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031051CUL)
#define CYREG_GPIO_PRT10_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310520UL)
#define CYREG_GPIO_PRT10_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310540UL)
#define CYREG_GPIO_PRT10_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310544UL)
#define CYREG_GPIO_PRT10_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310548UL)
#define CYREG_GPIO_PRT10_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031054CUL)
#define CYREG_GPIO_PRT10_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310558UL)

/**
  * \brief GPIO port registers (GPIO_PRT11)
  */
#define CYREG_GPIO_PRT11_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310580UL)
#define CYREG_GPIO_PRT11_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310584UL)
#define CYREG_GPIO_PRT11_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310588UL)
#define CYREG_GPIO_PRT11_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031058CUL)
#define CYREG_GPIO_PRT11_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310590UL)
#define CYREG_GPIO_PRT11_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310594UL)
#define CYREG_GPIO_PRT11_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310598UL)
#define CYREG_GPIO_PRT11_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031059CUL)
#define CYREG_GPIO_PRT11_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403105A0UL)
#define CYREG_GPIO_PRT11_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403105C0UL)
#define CYREG_GPIO_PRT11_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x403105C4UL)
#define CYREG_GPIO_PRT11_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403105C8UL)
#define CYREG_GPIO_PRT11_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403105CCUL)
#define CYREG_GPIO_PRT11_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403105D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT12)
  */
#define CYREG_GPIO_PRT12_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310600UL)
#define CYREG_GPIO_PRT12_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310604UL)
#define CYREG_GPIO_PRT12_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310608UL)
#define CYREG_GPIO_PRT12_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031060CUL)
#define CYREG_GPIO_PRT12_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310610UL)
#define CYREG_GPIO_PRT12_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310614UL)
#define CYREG_GPIO_PRT12_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310618UL)
#define CYREG_GPIO_PRT12_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031061CUL)
#define CYREG_GPIO_PRT12_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310620UL)
#define CYREG_GPIO_PRT12_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310640UL)
#define CYREG_GPIO_PRT12_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310644UL)
#define CYREG_GPIO_PRT12_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310648UL)
#define CYREG_GPIO_PRT12_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031064CUL)
#define CYREG_GPIO_PRT12_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310658UL)

/**
  * \brief GPIO port registers (GPIO_PRT13)
  */
#define CYREG_GPIO_PRT13_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310680UL)
#define CYREG_GPIO_PRT13_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310684UL)
#define CYREG_GPIO_PRT13_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310688UL)
#define CYREG_GPIO_PRT13_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031068CUL)
#define CYREG_GPIO_PRT13_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310690UL)
#define CYREG_GPIO_PRT13_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310694UL)
#define CYREG_GPIO_PRT13_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310698UL)
#define CYREG_GPIO_PRT13_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031069CUL)
#define CYREG_GPIO_PRT13_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403106A0UL)
#define CYREG_GPIO_PRT13_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403106C0UL)
#define CYREG_GPIO_PRT13_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x403106C4UL)
#define CYREG_GPIO_PRT13_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403106C8UL)
#define CYREG_GPIO_PRT13_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403106CCUL)
#define CYREG_GPIO_PRT13_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403106D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT14)
  */
#define CYREG_GPIO_PRT14_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310700UL)
#define CYREG_GPIO_PRT14_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310704UL)
#define CYREG_GPIO_PRT14_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310708UL)
#define CYREG_GPIO_PRT14_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031070CUL)
#define CYREG_GPIO_PRT14_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310710UL)
#define CYREG_GPIO_PRT14_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310714UL)
#define CYREG_GPIO_PRT14_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310718UL)
#define CYREG_GPIO_PRT14_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031071CUL)
#define CYREG_GPIO_PRT14_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310720UL)
#define CYREG_GPIO_PRT14_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310740UL)
#define CYREG_GPIO_PRT14_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310744UL)
#define CYREG_GPIO_PRT14_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310748UL)
#define CYREG_GPIO_PRT14_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031074CUL)
#define CYREG_GPIO_PRT14_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310758UL)

/**
  * \brief GPIO port registers (GPIO_PRT15)
  */
#define CYREG_GPIO_PRT15_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310780UL)
#define CYREG_GPIO_PRT15_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310784UL)
#define CYREG_GPIO_PRT15_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310788UL)
#define CYREG_GPIO_PRT15_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031078CUL)
#define CYREG_GPIO_PRT15_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310790UL)
#define CYREG_GPIO_PRT15_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310794UL)
#define CYREG_GPIO_PRT15_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310798UL)
#define CYREG_GPIO_PRT15_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031079CUL)
#define CYREG_GPIO_PRT15_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403107A0UL)
#define CYREG_GPIO_PRT15_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403107C0UL)
#define CYREG_GPIO_PRT15_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x403107C4UL)
#define CYREG_GPIO_PRT15_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403107C8UL)
#define CYREG_GPIO_PRT15_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403107CCUL)
#define CYREG_GPIO_PRT15_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403107D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT16)
  */
#define CYREG_GPIO_PRT16_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310800UL)
#define CYREG_GPIO_PRT16_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310804UL)
#define CYREG_GPIO_PRT16_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310808UL)
#define CYREG_GPIO_PRT16_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031080CUL)
#define CYREG_GPIO_PRT16_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310810UL)
#define CYREG_GPIO_PRT16_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310814UL)
#define CYREG_GPIO_PRT16_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310818UL)
#define CYREG_GPIO_PRT16_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031081CUL)
#define CYREG_GPIO_PRT16_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310820UL)
#define CYREG_GPIO_PRT16_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310840UL)
#define CYREG_GPIO_PRT16_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310844UL)
#define CYREG_GPIO_PRT16_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310848UL)
#define CYREG_GPIO_PRT16_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031084CUL)
#define CYREG_GPIO_PRT16_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310858UL)

/**
  * \brief GPIO port registers (GPIO_PRT17)
  */
#define CYREG_GPIO_PRT17_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310880UL)
#define CYREG_GPIO_PRT17_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310884UL)
#define CYREG_GPIO_PRT17_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310888UL)
#define CYREG_GPIO_PRT17_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031088CUL)
#define CYREG_GPIO_PRT17_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310890UL)
#define CYREG_GPIO_PRT17_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310894UL)
#define CYREG_GPIO_PRT17_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310898UL)
#define CYREG_GPIO_PRT17_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031089CUL)
#define CYREG_GPIO_PRT17_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403108A0UL)
#define CYREG_GPIO_PRT17_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403108C0UL)
#define CYREG_GPIO_PRT17_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x403108C4UL)
#define CYREG_GPIO_PRT17_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403108C8UL)
#define CYREG_GPIO_PRT17_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403108CCUL)
#define CYREG_GPIO_PRT17_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403108D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT18)
  */
#define CYREG_GPIO_PRT18_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310900UL)
#define CYREG_GPIO_PRT18_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310904UL)
#define CYREG_GPIO_PRT18_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310908UL)
#define CYREG_GPIO_PRT18_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031090CUL)
#define CYREG_GPIO_PRT18_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310910UL)
#define CYREG_GPIO_PRT18_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310914UL)
#define CYREG_GPIO_PRT18_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310918UL)
#define CYREG_GPIO_PRT18_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031091CUL)
#define CYREG_GPIO_PRT18_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310920UL)
#define CYREG_GPIO_PRT18_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310940UL)
#define CYREG_GPIO_PRT18_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310944UL)
#define CYREG_GPIO_PRT18_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310948UL)
#define CYREG_GPIO_PRT18_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031094CUL)
#define CYREG_GPIO_PRT18_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310958UL)

/**
  * \brief GPIO port registers (GPIO_PRT19)
  */
#define CYREG_GPIO_PRT19_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310980UL)
#define CYREG_GPIO_PRT19_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310984UL)
#define CYREG_GPIO_PRT19_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310988UL)
#define CYREG_GPIO_PRT19_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031098CUL)
#define CYREG_GPIO_PRT19_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310990UL)
#define CYREG_GPIO_PRT19_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310994UL)
#define CYREG_GPIO_PRT19_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310998UL)
#define CYREG_GPIO_PRT19_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031099CUL)
#define CYREG_GPIO_PRT19_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x403109A0UL)
#define CYREG_GPIO_PRT19_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x403109C0UL)
#define CYREG_GPIO_PRT19_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x403109C4UL)
#define CYREG_GPIO_PRT19_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x403109C8UL)
#define CYREG_GPIO_PRT19_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x403109CCUL)
#define CYREG_GPIO_PRT19_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x403109D8UL)

/**
  * \brief GPIO port registers (GPIO_PRT20)
  */
#define CYREG_GPIO_PRT20_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310A00UL)
#define CYREG_GPIO_PRT20_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310A04UL)
#define CYREG_GPIO_PRT20_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310A08UL)
#define CYREG_GPIO_PRT20_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310A0CUL)
#define CYREG_GPIO_PRT20_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310A10UL)
#define CYREG_GPIO_PRT20_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310A14UL)
#define CYREG_GPIO_PRT20_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310A18UL)
#define CYREG_GPIO_PRT20_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310A1CUL)
#define CYREG_GPIO_PRT20_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310A20UL)
#define CYREG_GPIO_PRT20_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310A40UL)
#define CYREG_GPIO_PRT20_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310A44UL)
#define CYREG_GPIO_PRT20_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310A48UL)
#define CYREG_GPIO_PRT20_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310A4CUL)
#define CYREG_GPIO_PRT20_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310A58UL)

/**
  * \brief GPIO port registers (GPIO_PRT21)
  */
#define CYREG_GPIO_PRT21_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310A80UL)
#define CYREG_GPIO_PRT21_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310A84UL)
#define CYREG_GPIO_PRT21_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310A88UL)
#define CYREG_GPIO_PRT21_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310A8CUL)
#define CYREG_GPIO_PRT21_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310A90UL)
#define CYREG_GPIO_PRT21_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310A94UL)
#define CYREG_GPIO_PRT21_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310A98UL)
#define CYREG_GPIO_PRT21_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310A9CUL)
#define CYREG_GPIO_PRT21_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310AA0UL)
#define CYREG_GPIO_PRT21_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310AC0UL)
#define CYREG_GPIO_PRT21_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310AC4UL)
#define CYREG_GPIO_PRT21_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310AC8UL)
#define CYREG_GPIO_PRT21_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310ACCUL)
#define CYREG_GPIO_PRT21_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310AD8UL)

/**
  * \brief GPIO port registers (GPIO_PRT22)
  */
#define CYREG_GPIO_PRT22_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310B00UL)
#define CYREG_GPIO_PRT22_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310B04UL)
#define CYREG_GPIO_PRT22_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310B08UL)
#define CYREG_GPIO_PRT22_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310B0CUL)
#define CYREG_GPIO_PRT22_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310B10UL)
#define CYREG_GPIO_PRT22_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310B14UL)
#define CYREG_GPIO_PRT22_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310B18UL)
#define CYREG_GPIO_PRT22_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310B1CUL)
#define CYREG_GPIO_PRT22_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310B20UL)
#define CYREG_GPIO_PRT22_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310B40UL)
#define CYREG_GPIO_PRT22_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310B44UL)
#define CYREG_GPIO_PRT22_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310B48UL)
#define CYREG_GPIO_PRT22_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310B4CUL)
#define CYREG_GPIO_PRT22_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310B58UL)

/**
  * \brief GPIO port registers (GPIO_PRT23)
  */
#define CYREG_GPIO_PRT23_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310B80UL)
#define CYREG_GPIO_PRT23_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310B84UL)
#define CYREG_GPIO_PRT23_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310B88UL)
#define CYREG_GPIO_PRT23_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310B8CUL)
#define CYREG_GPIO_PRT23_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310B90UL)
#define CYREG_GPIO_PRT23_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310B94UL)
#define CYREG_GPIO_PRT23_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310B98UL)
#define CYREG_GPIO_PRT23_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310B9CUL)
#define CYREG_GPIO_PRT23_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310BA0UL)
#define CYREG_GPIO_PRT23_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310BC0UL)
#define CYREG_GPIO_PRT23_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310BC4UL)
#define CYREG_GPIO_PRT23_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310BC8UL)
#define CYREG_GPIO_PRT23_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310BCCUL)
#define CYREG_GPIO_PRT23_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310BD8UL)

/**
  * \brief GPIO port registers (GPIO_PRT24)
  */
#define CYREG_GPIO_PRT24_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310C00UL)
#define CYREG_GPIO_PRT24_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310C04UL)
#define CYREG_GPIO_PRT24_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310C08UL)
#define CYREG_GPIO_PRT24_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310C0CUL)
#define CYREG_GPIO_PRT24_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310C10UL)
#define CYREG_GPIO_PRT24_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310C14UL)
#define CYREG_GPIO_PRT24_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310C18UL)
#define CYREG_GPIO_PRT24_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310C1CUL)
#define CYREG_GPIO_PRT24_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310C20UL)
#define CYREG_GPIO_PRT24_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310C40UL)
#define CYREG_GPIO_PRT24_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310C44UL)
#define CYREG_GPIO_PRT24_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310C48UL)
#define CYREG_GPIO_PRT24_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310C4CUL)
#define CYREG_GPIO_PRT24_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310C58UL)

/**
  * \brief GPIO port registers (GPIO_PRT25)
  */
#define CYREG_GPIO_PRT25_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310C80UL)
#define CYREG_GPIO_PRT25_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310C84UL)
#define CYREG_GPIO_PRT25_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310C88UL)
#define CYREG_GPIO_PRT25_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310C8CUL)
#define CYREG_GPIO_PRT25_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310C90UL)
#define CYREG_GPIO_PRT25_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310C94UL)
#define CYREG_GPIO_PRT25_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310C98UL)
#define CYREG_GPIO_PRT25_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310C9CUL)
#define CYREG_GPIO_PRT25_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310CA0UL)
#define CYREG_GPIO_PRT25_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310CC0UL)
#define CYREG_GPIO_PRT25_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310CC4UL)
#define CYREG_GPIO_PRT25_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310CC8UL)
#define CYREG_GPIO_PRT25_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310CCCUL)
#define CYREG_GPIO_PRT25_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310CD8UL)

/**
  * \brief GPIO port registers (GPIO_PRT26)
  */
#define CYREG_GPIO_PRT26_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310D00UL)
#define CYREG_GPIO_PRT26_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310D04UL)
#define CYREG_GPIO_PRT26_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310D08UL)
#define CYREG_GPIO_PRT26_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310D0CUL)
#define CYREG_GPIO_PRT26_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310D10UL)
#define CYREG_GPIO_PRT26_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310D14UL)
#define CYREG_GPIO_PRT26_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310D18UL)
#define CYREG_GPIO_PRT26_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310D1CUL)
#define CYREG_GPIO_PRT26_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310D20UL)
#define CYREG_GPIO_PRT26_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310D40UL)
#define CYREG_GPIO_PRT26_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310D44UL)
#define CYREG_GPIO_PRT26_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310D48UL)
#define CYREG_GPIO_PRT26_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310D4CUL)
#define CYREG_GPIO_PRT26_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310D58UL)

/**
  * \brief GPIO port registers (GPIO_PRT27)
  */
#define CYREG_GPIO_PRT27_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310D80UL)
#define CYREG_GPIO_PRT27_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310D84UL)
#define CYREG_GPIO_PRT27_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310D88UL)
#define CYREG_GPIO_PRT27_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310D8CUL)
#define CYREG_GPIO_PRT27_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310D90UL)
#define CYREG_GPIO_PRT27_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310D94UL)
#define CYREG_GPIO_PRT27_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310D98UL)
#define CYREG_GPIO_PRT27_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310D9CUL)
#define CYREG_GPIO_PRT27_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310DA0UL)
#define CYREG_GPIO_PRT27_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310DC0UL)
#define CYREG_GPIO_PRT27_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310DC4UL)
#define CYREG_GPIO_PRT27_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310DC8UL)
#define CYREG_GPIO_PRT27_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310DCCUL)
#define CYREG_GPIO_PRT27_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310DD8UL)

/**
  * \brief GPIO port registers (GPIO_PRT28)
  */
#define CYREG_GPIO_PRT28_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310E00UL)
#define CYREG_GPIO_PRT28_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310E04UL)
#define CYREG_GPIO_PRT28_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310E08UL)
#define CYREG_GPIO_PRT28_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310E0CUL)
#define CYREG_GPIO_PRT28_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310E10UL)
#define CYREG_GPIO_PRT28_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310E14UL)
#define CYREG_GPIO_PRT28_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310E18UL)
#define CYREG_GPIO_PRT28_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310E1CUL)
#define CYREG_GPIO_PRT28_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310E20UL)
#define CYREG_GPIO_PRT28_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310E40UL)
#define CYREG_GPIO_PRT28_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310E44UL)
#define CYREG_GPIO_PRT28_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310E48UL)
#define CYREG_GPIO_PRT28_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310E4CUL)
#define CYREG_GPIO_PRT28_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310E58UL)

/**
  * \brief GPIO port registers (GPIO_PRT29)
  */
#define CYREG_GPIO_PRT29_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310E80UL)
#define CYREG_GPIO_PRT29_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310E84UL)
#define CYREG_GPIO_PRT29_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310E88UL)
#define CYREG_GPIO_PRT29_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310E8CUL)
#define CYREG_GPIO_PRT29_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310E90UL)
#define CYREG_GPIO_PRT29_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310E94UL)
#define CYREG_GPIO_PRT29_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310E98UL)
#define CYREG_GPIO_PRT29_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310E9CUL)
#define CYREG_GPIO_PRT29_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310EA0UL)
#define CYREG_GPIO_PRT29_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310EC0UL)
#define CYREG_GPIO_PRT29_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310EC4UL)
#define CYREG_GPIO_PRT29_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310EC8UL)
#define CYREG_GPIO_PRT29_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310ECCUL)
#define CYREG_GPIO_PRT29_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310ED8UL)

/**
  * \brief GPIO port registers (GPIO_PRT30)
  */
#define CYREG_GPIO_PRT30_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310F00UL)
#define CYREG_GPIO_PRT30_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310F04UL)
#define CYREG_GPIO_PRT30_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310F08UL)
#define CYREG_GPIO_PRT30_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310F0CUL)
#define CYREG_GPIO_PRT30_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310F10UL)
#define CYREG_GPIO_PRT30_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310F14UL)
#define CYREG_GPIO_PRT30_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310F18UL)
#define CYREG_GPIO_PRT30_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310F1CUL)
#define CYREG_GPIO_PRT30_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310F20UL)
#define CYREG_GPIO_PRT30_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310F40UL)
#define CYREG_GPIO_PRT30_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310F44UL)
#define CYREG_GPIO_PRT30_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310F48UL)
#define CYREG_GPIO_PRT30_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310F4CUL)
#define CYREG_GPIO_PRT30_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310F58UL)

/**
  * \brief GPIO port registers (GPIO_PRT31)
  */
#define CYREG_GPIO_PRT31_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40310F80UL)
#define CYREG_GPIO_PRT31_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40310F84UL)
#define CYREG_GPIO_PRT31_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40310F88UL)
#define CYREG_GPIO_PRT31_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x40310F8CUL)
#define CYREG_GPIO_PRT31_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40310F90UL)
#define CYREG_GPIO_PRT31_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40310F94UL)
#define CYREG_GPIO_PRT31_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40310F98UL)
#define CYREG_GPIO_PRT31_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x40310F9CUL)
#define CYREG_GPIO_PRT31_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40310FA0UL)
#define CYREG_GPIO_PRT31_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40310FC0UL)
#define CYREG_GPIO_PRT31_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40310FC4UL)
#define CYREG_GPIO_PRT31_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40310FC8UL)
#define CYREG_GPIO_PRT31_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x40310FCCUL)
#define CYREG_GPIO_PRT31_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40310FD8UL)

/**
  * \brief GPIO port registers (GPIO_PRT32)
  */
#define CYREG_GPIO_PRT32_OUT            ((volatile un_GPIO_PRT_OUT_t*) 0x40311000UL)
#define CYREG_GPIO_PRT32_OUT_CLR        ((volatile un_GPIO_PRT_OUT_CLR_t*) 0x40311004UL)
#define CYREG_GPIO_PRT32_OUT_SET        ((volatile un_GPIO_PRT_OUT_SET_t*) 0x40311008UL)
#define CYREG_GPIO_PRT32_OUT_INV        ((volatile un_GPIO_PRT_OUT_INV_t*) 0x4031100CUL)
#define CYREG_GPIO_PRT32_IN             ((volatile un_GPIO_PRT_IN_t*) 0x40311010UL)
#define CYREG_GPIO_PRT32_INTR           ((volatile un_GPIO_PRT_INTR_t*) 0x40311014UL)
#define CYREG_GPIO_PRT32_INTR_MASK      ((volatile un_GPIO_PRT_INTR_MASK_t*) 0x40311018UL)
#define CYREG_GPIO_PRT32_INTR_MASKED    ((volatile un_GPIO_PRT_INTR_MASKED_t*) 0x4031101CUL)
#define CYREG_GPIO_PRT32_INTR_SET       ((volatile un_GPIO_PRT_INTR_SET_t*) 0x40311020UL)
#define CYREG_GPIO_PRT32_INTR_CFG       ((volatile un_GPIO_PRT_INTR_CFG_t*) 0x40311040UL)
#define CYREG_GPIO_PRT32_CFG            ((volatile un_GPIO_PRT_CFG_t*) 0x40311044UL)
#define CYREG_GPIO_PRT32_CFG_IN         ((volatile un_GPIO_PRT_CFG_IN_t*) 0x40311048UL)
#define CYREG_GPIO_PRT32_CFG_OUT        ((volatile un_GPIO_PRT_CFG_OUT_t*) 0x4031104CUL)
#define CYREG_GPIO_PRT32_CFG_IN_AUTOLVL ((volatile un_GPIO_PRT_CFG_IN_AUTOLVL_t*) 0x40311058UL)

/**
  * \brief GPIO port control/configuration (GPIO0)
  */
#define CYREG_GPIO_INTR_CAUSE0          ((volatile un_GPIO_INTR_CAUSE0_t*) 0x40314000UL)
#define CYREG_GPIO_INTR_CAUSE1          ((volatile un_GPIO_INTR_CAUSE1_t*) 0x40314004UL)
#define CYREG_GPIO_VDD_ACTIVE           ((volatile un_GPIO_VDD_ACTIVE_t*) 0x40314010UL)
#define CYREG_GPIO_VDD_INTR             ((volatile un_GPIO_VDD_INTR_t*) 0x40314014UL)
#define CYREG_GPIO_VDD_INTR_MASK        ((volatile un_GPIO_VDD_INTR_MASK_t*) 0x40314018UL)
#define CYREG_GPIO_VDD_INTR_MASKED      ((volatile un_GPIO_VDD_INTR_MASKED_t*) 0x4031401CUL)
#define CYREG_GPIO_VDD_INTR_SET         ((volatile un_GPIO_VDD_INTR_SET_t*) 0x40314020UL)

#endif /* _CYREG_GPIO_H_ */


/* [] END OF FILE */
