/***************************************************************************//**
* \file cyreg_srss.h
*
* \brief
* SRSS register definition header
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

#ifndef _CYREG_SRSS_H_
#define _CYREG_SRSS_H_

#include "cyip_srss_v3.h"

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV0)
  */
#define CYREG_CSV_HF_CSV0_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261400UL)
#define CYREG_CSV_HF_CSV0_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261404UL)
#define CYREG_CSV_HF_CSV0_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261408UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV1)
  */
#define CYREG_CSV_HF_CSV1_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261410UL)
#define CYREG_CSV_HF_CSV1_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261414UL)
#define CYREG_CSV_HF_CSV1_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261418UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV2)
  */
#define CYREG_CSV_HF_CSV2_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261420UL)
#define CYREG_CSV_HF_CSV2_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261424UL)
#define CYREG_CSV_HF_CSV2_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261428UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV3)
  */
#define CYREG_CSV_HF_CSV3_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261430UL)
#define CYREG_CSV_HF_CSV3_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261434UL)
#define CYREG_CSV_HF_CSV3_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261438UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV4)
  */
#define CYREG_CSV_HF_CSV4_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261440UL)
#define CYREG_CSV_HF_CSV4_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261444UL)
#define CYREG_CSV_HF_CSV4_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261448UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV5)
  */
#define CYREG_CSV_HF_CSV5_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261450UL)
#define CYREG_CSV_HF_CSV5_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261454UL)
#define CYREG_CSV_HF_CSV5_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261458UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV6)
  */
#define CYREG_CSV_HF_CSV6_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261460UL)
#define CYREG_CSV_HF_CSV6_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261464UL)
#define CYREG_CSV_HF_CSV6_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261468UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers (CSV_HF_CSV7)
  */
#define CYREG_CSV_HF_CSV7_REF_CTL       ((volatile un_CSV_HF_CSV_REF_CTL_t*) 0x40261470UL)
#define CYREG_CSV_HF_CSV7_REF_LIMIT     ((volatile un_CSV_HF_CSV_REF_LIMIT_t*) 0x40261474UL)
#define CYREG_CSV_HF_CSV7_MON_CTL       ((volatile un_CSV_HF_CSV_MON_CTL_t*) 0x40261478UL)

/**
  * \brief Active domain Clock Supervisor (CSV) registers for CSV Reference clock (CSV_REF_CSV0)
  */
#define CYREG_CSV_REF_CSV_REF_CTL       ((volatile un_CSV_REF_CSV_REF_CTL_t*) 0x40261710UL)
#define CYREG_CSV_REF_CSV_REF_LIMIT     ((volatile un_CSV_REF_CSV_REF_LIMIT_t*) 0x40261714UL)
#define CYREG_CSV_REF_CSV_MON_CTL       ((volatile un_CSV_REF_CSV_MON_CTL_t*) 0x40261718UL)

/**
  * \brief LF clock Clock Supervisor registers (CSV_LF_CSV0)
  */
#define CYREG_CSV_LF_CSV_REF_CTL        ((volatile un_CSV_LF_CSV_REF_CTL_t*) 0x40261720UL)
#define CYREG_CSV_LF_CSV_REF_LIMIT      ((volatile un_CSV_LF_CSV_REF_LIMIT_t*) 0x40261724UL)
#define CYREG_CSV_LF_CSV_MON_CTL        ((volatile un_CSV_LF_CSV_MON_CTL_t*) 0x40261728UL)

/**
  * \brief ILO0 clock DeepSleep domain Clock Supervisor registers (CSV_ILO_CSV0)
  */
#define CYREG_CSV_ILO_CSV_REF_CTL       ((volatile un_CSV_ILO_CSV_REF_CTL_t*) 0x40261730UL)
#define CYREG_CSV_ILO_CSV_REF_LIMIT     ((volatile un_CSV_ILO_CSV_REF_LIMIT_t*) 0x40261734UL)
#define CYREG_CSV_ILO_CSV_MON_CTL       ((volatile un_CSV_ILO_CSV_MON_CTL_t*) 0x40261738UL)

/**
  * \brief 400MHz PLL Configuration Register (CLK_PLL400M0)
  */
#define CYREG_CLK_PLL400M0_CONFIG       ((volatile un_CLK_PLL400M_CONFIG_t*) 0x40261900UL)
#define CYREG_CLK_PLL400M0_CONFIG2      ((volatile un_CLK_PLL400M_CONFIG2_t*) 0x40261904UL)
#define CYREG_CLK_PLL400M0_CONFIG3      ((volatile un_CLK_PLL400M_CONFIG3_t*) 0x40261908UL)
#define CYREG_CLK_PLL400M0_STATUS       ((volatile un_CLK_PLL400M_STATUS_t*) 0x4026190CUL)

/**
  * \brief 400MHz PLL Configuration Register (CLK_PLL400M1)
  */
#define CYREG_CLK_PLL400M1_CONFIG       ((volatile un_CLK_PLL400M_CONFIG_t*) 0x40261910UL)
#define CYREG_CLK_PLL400M1_CONFIG2      ((volatile un_CLK_PLL400M_CONFIG2_t*) 0x40261914UL)
#define CYREG_CLK_PLL400M1_CONFIG3      ((volatile un_CLK_PLL400M_CONFIG3_t*) 0x40261918UL)
#define CYREG_CLK_PLL400M1_STATUS       ((volatile un_CLK_PLL400M_STATUS_t*) 0x4026191CUL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR0)
  */
#define CYREG_MCWDT0_CTR0_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268000UL)
#define CYREG_MCWDT0_CTR0_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268004UL)
#define CYREG_MCWDT0_CTR0_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268008UL)
#define CYREG_MCWDT0_CTR0_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026800CUL)
#define CYREG_MCWDT0_CTR0_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268010UL)
#define CYREG_MCWDT0_CTR0_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268014UL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR1)
  */
#define CYREG_MCWDT0_CTR1_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268020UL)
#define CYREG_MCWDT0_CTR1_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268024UL)
#define CYREG_MCWDT0_CTR1_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268028UL)
#define CYREG_MCWDT0_CTR1_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026802CUL)
#define CYREG_MCWDT0_CTR1_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268030UL)
#define CYREG_MCWDT0_CTR1_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268034UL)

/**
  * \brief Multi-Counter Watchdog Timer (MCWDT0)
  */
#define CYREG_MCWDT0_CPU_SELECT         ((volatile un_MCWDT_CPU_SELECT_t*) 0x40268040UL)
#define CYREG_MCWDT0_CTR2_CTL           ((volatile un_MCWDT_CTR2_CTL_t*) 0x40268080UL)
#define CYREG_MCWDT0_CTR2_CONFIG        ((volatile un_MCWDT_CTR2_CONFIG_t*) 0x40268084UL)
#define CYREG_MCWDT0_CTR2_CNT           ((volatile un_MCWDT_CTR2_CNT_t*) 0x40268088UL)
#define CYREG_MCWDT0_LOCK               ((volatile un_MCWDT_LOCK_t*) 0x40268090UL)
#define CYREG_MCWDT0_SERVICE            ((volatile un_MCWDT_SERVICE_t*) 0x40268094UL)
#define CYREG_MCWDT0_INTR               ((volatile un_MCWDT_INTR_t*) 0x402680A0UL)
#define CYREG_MCWDT0_INTR_SET           ((volatile un_MCWDT_INTR_SET_t*) 0x402680A4UL)
#define CYREG_MCWDT0_INTR_MASK          ((volatile un_MCWDT_INTR_MASK_t*) 0x402680A8UL)
#define CYREG_MCWDT0_INTR_MASKED        ((volatile un_MCWDT_INTR_MASKED_t*) 0x402680ACUL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR0)
  */
#define CYREG_MCWDT1_CTR0_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268100UL)
#define CYREG_MCWDT1_CTR0_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268104UL)
#define CYREG_MCWDT1_CTR0_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268108UL)
#define CYREG_MCWDT1_CTR0_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026810CUL)
#define CYREG_MCWDT1_CTR0_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268110UL)
#define CYREG_MCWDT1_CTR0_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268114UL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR1)
  */
#define CYREG_MCWDT1_CTR1_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268120UL)
#define CYREG_MCWDT1_CTR1_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268124UL)
#define CYREG_MCWDT1_CTR1_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268128UL)
#define CYREG_MCWDT1_CTR1_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026812CUL)
#define CYREG_MCWDT1_CTR1_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268130UL)
#define CYREG_MCWDT1_CTR1_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268134UL)

/**
  * \brief Multi-Counter Watchdog Timer (MCWDT1)
  */
#define CYREG_MCWDT1_CPU_SELECT         ((volatile un_MCWDT_CPU_SELECT_t*) 0x40268140UL)
#define CYREG_MCWDT1_CTR2_CTL           ((volatile un_MCWDT_CTR2_CTL_t*) 0x40268180UL)
#define CYREG_MCWDT1_CTR2_CONFIG        ((volatile un_MCWDT_CTR2_CONFIG_t*) 0x40268184UL)
#define CYREG_MCWDT1_CTR2_CNT           ((volatile un_MCWDT_CTR2_CNT_t*) 0x40268188UL)
#define CYREG_MCWDT1_LOCK               ((volatile un_MCWDT_LOCK_t*) 0x40268190UL)
#define CYREG_MCWDT1_SERVICE            ((volatile un_MCWDT_SERVICE_t*) 0x40268194UL)
#define CYREG_MCWDT1_INTR               ((volatile un_MCWDT_INTR_t*) 0x402681A0UL)
#define CYREG_MCWDT1_INTR_SET           ((volatile un_MCWDT_INTR_SET_t*) 0x402681A4UL)
#define CYREG_MCWDT1_INTR_MASK          ((volatile un_MCWDT_INTR_MASK_t*) 0x402681A8UL)
#define CYREG_MCWDT1_INTR_MASKED        ((volatile un_MCWDT_INTR_MASKED_t*) 0x402681ACUL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR0)
  */
#define CYREG_MCWDT2_CTR0_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268200UL)
#define CYREG_MCWDT2_CTR0_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268204UL)
#define CYREG_MCWDT2_CTR0_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268208UL)
#define CYREG_MCWDT2_CTR0_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026820CUL)
#define CYREG_MCWDT2_CTR0_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268210UL)
#define CYREG_MCWDT2_CTR0_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268214UL)

/**
  * \brief MCWDT Configuration for Subcounter 0 and 1 (MCWDT_CTR1)
  */
#define CYREG_MCWDT2_CTR1_CTL           ((volatile un_MCWDT_CTR_CTL_t*) 0x40268220UL)
#define CYREG_MCWDT2_CTR1_LOWER_LIMIT   ((volatile un_MCWDT_CTR_LOWER_LIMIT_t*) 0x40268224UL)
#define CYREG_MCWDT2_CTR1_UPPER_LIMIT   ((volatile un_MCWDT_CTR_UPPER_LIMIT_t*) 0x40268228UL)
#define CYREG_MCWDT2_CTR1_WARN_LIMIT    ((volatile un_MCWDT_CTR_WARN_LIMIT_t*) 0x4026822CUL)
#define CYREG_MCWDT2_CTR1_CONFIG        ((volatile un_MCWDT_CTR_CONFIG_t*) 0x40268230UL)
#define CYREG_MCWDT2_CTR1_CNT           ((volatile un_MCWDT_CTR_CNT_t*) 0x40268234UL)

/**
  * \brief Multi-Counter Watchdog Timer (MCWDT2)
  */
#define CYREG_MCWDT2_CPU_SELECT         ((volatile un_MCWDT_CPU_SELECT_t*) 0x40268240UL)
#define CYREG_MCWDT2_CTR2_CTL           ((volatile un_MCWDT_CTR2_CTL_t*) 0x40268280UL)
#define CYREG_MCWDT2_CTR2_CONFIG        ((volatile un_MCWDT_CTR2_CONFIG_t*) 0x40268284UL)
#define CYREG_MCWDT2_CTR2_CNT           ((volatile un_MCWDT_CTR2_CNT_t*) 0x40268288UL)
#define CYREG_MCWDT2_LOCK               ((volatile un_MCWDT_LOCK_t*) 0x40268290UL)
#define CYREG_MCWDT2_SERVICE            ((volatile un_MCWDT_SERVICE_t*) 0x40268294UL)
#define CYREG_MCWDT2_INTR               ((volatile un_MCWDT_INTR_t*) 0x402682A0UL)
#define CYREG_MCWDT2_INTR_SET           ((volatile un_MCWDT_INTR_SET_t*) 0x402682A4UL)
#define CYREG_MCWDT2_INTR_MASK          ((volatile un_MCWDT_INTR_MASK_t*) 0x402682A8UL)
#define CYREG_MCWDT2_INTR_MASKED        ((volatile un_MCWDT_INTR_MASKED_t*) 0x402682ACUL)

/**
  * \brief Watchdog Timer (WDT0)
  */
#define CYREG_WDT_CTL                   ((volatile un_WDT_CTL_t*) 0x4026C000UL)
#define CYREG_WDT_LOWER_LIMIT           ((volatile un_WDT_LOWER_LIMIT_t*) 0x4026C004UL)
#define CYREG_WDT_UPPER_LIMIT           ((volatile un_WDT_UPPER_LIMIT_t*) 0x4026C008UL)
#define CYREG_WDT_WARN_LIMIT            ((volatile un_WDT_WARN_LIMIT_t*) 0x4026C00CUL)
#define CYREG_WDT_CONFIG                ((volatile un_WDT_CONFIG_t*) 0x4026C010UL)
#define CYREG_WDT_CNT                   ((volatile un_WDT_CNT_t*) 0x4026C014UL)
#define CYREG_WDT_LOCK                  ((volatile un_WDT_LOCK_t*) 0x4026C040UL)
#define CYREG_WDT_SERVICE               ((volatile un_WDT_SERVICE_t*) 0x4026C044UL)
#define CYREG_WDT_INTR                  ((volatile un_WDT_INTR_t*) 0x4026C050UL)
#define CYREG_WDT_INTR_SET              ((volatile un_WDT_INTR_SET_t*) 0x4026C054UL)
#define CYREG_WDT_INTR_MASK             ((volatile un_WDT_INTR_MASK_t*) 0x4026C058UL)
#define CYREG_WDT_INTR_MASKED           ((volatile un_WDT_INTR_MASKED_t*) 0x4026C05CUL)

/**
  * \brief SRSS Core Registers (ver3) (SRSS0)
  */
#define CYREG_PWR_LVD_STATUS            ((volatile un_PWR_LVD_STATUS_t*) 0x40260040UL)
#define CYREG_PWR_LVD_STATUS2           ((volatile un_PWR_LVD_STATUS2_t*) 0x40260044UL)
#define CYREG_CLK_DSI_SELECT0           ((volatile un_CLK_DSI_SELECT_t*) 0x40260100UL)
#define CYREG_CLK_DSI_SELECT1           ((volatile un_CLK_DSI_SELECT_t*) 0x40260104UL)
#define CYREG_CLK_DSI_SELECT2           ((volatile un_CLK_DSI_SELECT_t*) 0x40260108UL)
#define CYREG_CLK_DSI_SELECT3           ((volatile un_CLK_DSI_SELECT_t*) 0x4026010CUL)
#define CYREG_CLK_DSI_SELECT4           ((volatile un_CLK_DSI_SELECT_t*) 0x40260110UL)
#define CYREG_CLK_DSI_SELECT5           ((volatile un_CLK_DSI_SELECT_t*) 0x40260114UL)
#define CYREG_CLK_DSI_SELECT6           ((volatile un_CLK_DSI_SELECT_t*) 0x40260118UL)
#define CYREG_CLK_OUTPUT_FAST           ((volatile un_CLK_OUTPUT_FAST_t*) 0x40260140UL)
#define CYREG_CLK_OUTPUT_SLOW           ((volatile un_CLK_OUTPUT_SLOW_t*) 0x40260144UL)
#define CYREG_CLK_CAL_CNT1              ((volatile un_CLK_CAL_CNT1_t*) 0x40260148UL)
#define CYREG_CLK_CAL_CNT2              ((volatile un_CLK_CAL_CNT2_t*) 0x4026014CUL)
#define CYREG_SRSS_INTR                 ((volatile un_SRSS_INTR_t*) 0x40260200UL)
#define CYREG_SRSS_INTR_SET             ((volatile un_SRSS_INTR_SET_t*) 0x40260204UL)
#define CYREG_SRSS_INTR_MASK            ((volatile un_SRSS_INTR_MASK_t*) 0x40260208UL)
#define CYREG_SRSS_INTR_MASKED          ((volatile un_SRSS_INTR_MASKED_t*) 0x4026020CUL)
#define CYREG_PWR_CTL                   ((volatile un_PWR_CTL_t*) 0x40261000UL)
#define CYREG_PWR_CTL2                  ((volatile un_PWR_CTL2_t*) 0x40261004UL)
#define CYREG_PWR_HIBERNATE             ((volatile un_PWR_HIBERNATE_t*) 0x40261008UL)
#define CYREG_PWR_SSV_CTL               ((volatile un_PWR_SSV_CTL_t*) 0x40261018UL)
#define CYREG_PWR_SSV_STATUS            ((volatile un_PWR_SSV_STATUS_t*) 0x4026101CUL)
#define CYREG_PWR_LVD_CTL               ((volatile un_PWR_LVD_CTL_t*) 0x40261020UL)
#define CYREG_PWR_LVD_CTL2              ((volatile un_PWR_LVD_CTL2_t*) 0x40261024UL)
#define CYREG_PWR_REGHC_CTL             ((volatile un_PWR_REGHC_CTL_t*) 0x40261028UL)
#define CYREG_PWR_REGHC_STATUS          ((volatile un_PWR_REGHC_STATUS_t*) 0x4026102CUL)
#define CYREG_PWR_REGHC_CTL2            ((volatile un_PWR_REGHC_CTL2_t*) 0x40261030UL)
#define CYREG_PWR_REGHC_CTL4            ((volatile un_PWR_REGHC_CTL4_t*) 0x40261038UL)
#define CYREG_PWR_HIB_DATA0             ((volatile un_PWR_HIB_DATA_t*) 0x40261040UL)
#define CYREG_CLK_PATH_SELECT0          ((volatile un_CLK_PATH_SELECT_t*) 0x40261200UL)
#define CYREG_CLK_PATH_SELECT1          ((volatile un_CLK_PATH_SELECT_t*) 0x40261204UL)
#define CYREG_CLK_PATH_SELECT2          ((volatile un_CLK_PATH_SELECT_t*) 0x40261208UL)
#define CYREG_CLK_PATH_SELECT3          ((volatile un_CLK_PATH_SELECT_t*) 0x4026120CUL)
#define CYREG_CLK_PATH_SELECT4          ((volatile un_CLK_PATH_SELECT_t*) 0x40261210UL)
#define CYREG_CLK_PATH_SELECT5          ((volatile un_CLK_PATH_SELECT_t*) 0x40261214UL)
#define CYREG_CLK_PATH_SELECT6          ((volatile un_CLK_PATH_SELECT_t*) 0x40261218UL)
#define CYREG_CLK_ROOT_SELECT0          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261240UL)
#define CYREG_CLK_ROOT_SELECT1          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261244UL)
#define CYREG_CLK_ROOT_SELECT2          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261248UL)
#define CYREG_CLK_ROOT_SELECT3          ((volatile un_CLK_ROOT_SELECT_t*) 0x4026124CUL)
#define CYREG_CLK_ROOT_SELECT4          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261250UL)
#define CYREG_CLK_ROOT_SELECT5          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261254UL)
#define CYREG_CLK_ROOT_SELECT6          ((volatile un_CLK_ROOT_SELECT_t*) 0x40261258UL)
#define CYREG_CLK_ROOT_SELECT7          ((volatile un_CLK_ROOT_SELECT_t*) 0x4026125CUL)
#define CYREG_CLK_SELECT                ((volatile un_CLK_SELECT_t*) 0x40261500UL)
#define CYREG_CLK_TIMER_CTL             ((volatile un_CLK_TIMER_CTL_t*) 0x40261504UL)
#define CYREG_CLK_ILO0_CONFIG           ((volatile un_CLK_ILO0_CONFIG_t*) 0x40261508UL)
#define CYREG_CLK_ILO1_CONFIG           ((volatile un_CLK_ILO1_CONFIG_t*) 0x4026150CUL)
#define CYREG_CLK_IMO_CONFIG            ((volatile un_CLK_IMO_CONFIG_t*) 0x40261518UL)
#define CYREG_CLK_ECO_CONFIG            ((volatile un_CLK_ECO_CONFIG_t*) 0x4026151CUL)
#define CYREG_CLK_ECO_PRESCALE          ((volatile un_CLK_ECO_PRESCALE_t*) 0x40261520UL)
#define CYREG_CLK_ECO_STATUS            ((volatile un_CLK_ECO_STATUS_t*) 0x40261524UL)
#define CYREG_CLK_FLL_CONFIG            ((volatile un_CLK_FLL_CONFIG_t*) 0x40261530UL)
#define CYREG_CLK_FLL_CONFIG2           ((volatile un_CLK_FLL_CONFIG2_t*) 0x40261534UL)
#define CYREG_CLK_FLL_CONFIG3           ((volatile un_CLK_FLL_CONFIG3_t*) 0x40261538UL)
#define CYREG_CLK_FLL_CONFIG4           ((volatile un_CLK_FLL_CONFIG4_t*) 0x4026153CUL)
#define CYREG_CLK_FLL_STATUS            ((volatile un_CLK_FLL_STATUS_t*) 0x40261540UL)
#define CYREG_CLK_ECO_CONFIG2           ((volatile un_CLK_ECO_CONFIG2_t*) 0x40261544UL)
#define CYREG_CLK_PLL_CONFIG0           ((volatile un_CLK_PLL_CONFIG_t*) 0x40261600UL)
#define CYREG_CLK_PLL_CONFIG1           ((volatile un_CLK_PLL_CONFIG_t*) 0x40261604UL)
#define CYREG_CLK_PLL_STATUS0           ((volatile un_CLK_PLL_STATUS_t*) 0x40261640UL)
#define CYREG_CLK_PLL_STATUS1           ((volatile un_CLK_PLL_STATUS_t*) 0x40261644UL)
#define CYREG_CSV_REF_SEL               ((volatile un_CSV_REF_SEL_t*) 0x40261700UL)
#define CYREG_RES_CAUSE                 ((volatile un_RES_CAUSE_t*) 0x40261800UL)
#define CYREG_RES_CAUSE2                ((volatile un_RES_CAUSE2_t*) 0x40261804UL)
#define CYREG_CLK_TRIM_ILO0_CTL         ((volatile un_CLK_TRIM_ILO0_CTL_t*) 0x40263014UL)
#define CYREG_CLK_TRIM_ILO1_CTL         ((volatile un_CLK_TRIM_ILO1_CTL_t*) 0x40263220UL)

#endif /* _CYREG_SRSS_H_ */


/* [] END OF FILE */
