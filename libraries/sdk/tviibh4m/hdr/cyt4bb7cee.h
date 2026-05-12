/***************************************************************************//**
* \file cyt4bb7cee.h
*
* \brief
* CYT4BB7CEE device header
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

#ifndef _CYT4BB7CEE_H_
#define _CYT4BB7CEE_H_

/**
* \addtogroup group_device CYT4BB7CEE
* \{
*/

/**
* \addtogroup Configuration_of_CMSIS
* \{
*/

/* CYT4BB7CEE peripheral interrupts */
typedef enum {
  cpuss_interrupts_ipc_0_IRQn       =   0,      /*!<   0 [DeepSleep] CPUSS Inter Process Communication Interrupt #0 */
  cpuss_interrupts_ipc_1_IRQn       =   1,      /*!<   1 [DeepSleep] CPUSS Inter Process Communication Interrupt #1 */
  cpuss_interrupts_ipc_2_IRQn       =   2,      /*!<   2 [DeepSleep] CPUSS Inter Process Communication Interrupt #2 */
  cpuss_interrupts_ipc_3_IRQn       =   3,      /*!<   3 [DeepSleep] CPUSS Inter Process Communication Interrupt #3 */
  cpuss_interrupts_ipc_4_IRQn       =   4,      /*!<   4 [DeepSleep] CPUSS Inter Process Communication Interrupt #4 */
  cpuss_interrupts_ipc_5_IRQn       =   5,      /*!<   5 [DeepSleep] CPUSS Inter Process Communication Interrupt #5 */
  cpuss_interrupts_ipc_6_IRQn       =   6,      /*!<   6 [DeepSleep] CPUSS Inter Process Communication Interrupt #6 */
  cpuss_interrupts_ipc_7_IRQn       =   7,      /*!<   7 [DeepSleep] CPUSS Inter Process Communication Interrupt #7 */
  cpuss_interrupts_fault_0_IRQn     =   8,      /*!<   8 [DeepSleep] CPUSS Fault Structure Interrupt #0 */
  cpuss_interrupts_fault_1_IRQn     =   9,      /*!<   9 [DeepSleep] CPUSS Fault Structure Interrupt #1 */
  cpuss_interrupts_fault_2_IRQn     =  10,      /*!<  10 [DeepSleep] CPUSS Fault Structure Interrupt #2 */
  cpuss_interrupts_fault_3_IRQn     =  11,      /*!<  11 [DeepSleep] CPUSS Fault Structure Interrupt #3 */
  srss_interrupt_backup_IRQn        =  12,      /*!<  12 [DeepSleep] Interrupt for BACKUP domain */
  srss_interrupt_mcwdt_0_IRQn       =  13,      /*!<  13 [DeepSleep] Multi Counter Watchdog Timer interrupt */
  srss_interrupt_mcwdt_1_IRQn       =  14,      /*!<  14 [DeepSleep] Multi Counter Watchdog Timer interrupt */
  srss_interrupt_mcwdt_2_IRQn       =  15,      /*!<  15 [DeepSleep] Multi Counter Watchdog Timer interrupt */
  srss_interrupt_wdt_IRQn           =  16,      /*!<  16 [DeepSleep] Hardware Watchdog Timer interrupt */
  srss_interrupt_IRQn               =  17,      /*!<  17 [DeepSleep] Other combined Interrupts for SRSS (LVD, CLKCAL) */
  scb_0_interrupt_IRQn              =  18,      /*!<  18 [DeepSleep] Serial Communication Block #0 (DeepSleep capable) */
  evtgen_0_interrupt_dpslp_IRQn     =  19,      /*!<  19 [DeepSleep] Event gen Deep-sleep domain interrupt */
  ioss_interrupt_vdd_IRQn           =  20,      /*!<  20 [DeepSleep] GPIO Supply Detect Interrupt */
  ioss_interrupt_gpio_dpslp_IRQn    =  21,      /*!<  21 [DeepSleep] Consolidated Interrupt for GPIO, All Ports */
  ioss_interrupts_gpio_dpslp_0_IRQn =  22,      /*!<  22 [DeepSleep] GPIO Port Interrupt #0 */
  ioss_interrupts_gpio_dpslp_1_IRQn =  23,      /*!<  23 [DeepSleep] GPIO Port Interrupt #1 */
  ioss_interrupts_gpio_dpslp_2_IRQn =  24,      /*!<  24 [DeepSleep] GPIO Port Interrupt #2 */
  ioss_interrupts_gpio_dpslp_3_IRQn =  25,      /*!<  25 [DeepSleep] GPIO Port Interrupt #3 */
  ioss_interrupts_gpio_dpslp_4_IRQn =  26,      /*!<  26 [DeepSleep] GPIO Port Interrupt #4 */
  ioss_interrupts_gpio_dpslp_5_IRQn =  27,      /*!<  27 [DeepSleep] GPIO Port Interrupt #5 */
  ioss_interrupts_gpio_dpslp_6_IRQn =  28,      /*!<  28 [DeepSleep] GPIO Port Interrupt #6 */
  ioss_interrupts_gpio_dpslp_7_IRQn =  29,      /*!<  29 [DeepSleep] GPIO Port Interrupt #7 */
  ioss_interrupts_gpio_dpslp_8_IRQn =  30,      /*!<  30 [DeepSleep] GPIO Port Interrupt #8 */
  ioss_interrupts_gpio_dpslp_9_IRQn =  31,      /*!<  31 [DeepSleep] GPIO Port Interrupt #9 */
  ioss_interrupts_gpio_dpslp_10_IRQn =  32,     /*!<  32 [DeepSleep] GPIO Port Interrupt #10 */
  ioss_interrupts_gpio_dpslp_11_IRQn =  33,     /*!<  33 [DeepSleep] GPIO Port Interrupt #11 */
  ioss_interrupts_gpio_dpslp_12_IRQn =  34,     /*!<  34 [DeepSleep] GPIO Port Interrupt #12 */
  ioss_interrupts_gpio_dpslp_13_IRQn =  35,     /*!<  35 [DeepSleep] GPIO Port Interrupt #13 */
  ioss_interrupts_gpio_dpslp_14_IRQn =  36,     /*!<  36 [DeepSleep] GPIO Port Interrupt #14 */
  ioss_interrupts_gpio_dpslp_15_IRQn =  37,     /*!<  37 [DeepSleep] GPIO Port Interrupt #15 */
  ioss_interrupts_gpio_dpslp_16_IRQn =  38,     /*!<  38 [DeepSleep] GPIO Port Interrupt #16 */
  ioss_interrupts_gpio_dpslp_17_IRQn =  39,     /*!<  39 [DeepSleep] GPIO Port Interrupt #17 */
  ioss_interrupts_gpio_dpslp_18_IRQn =  40,     /*!<  40 [DeepSleep] GPIO Port Interrupt #18 */
  ioss_interrupts_gpio_dpslp_19_IRQn =  41,     /*!<  41 [DeepSleep] GPIO Port Interrupt #19 */
  ioss_interrupts_gpio_dpslp_20_IRQn =  42,     /*!<  42 [DeepSleep] GPIO Port Interrupt #20 */
  ioss_interrupts_gpio_dpslp_21_IRQn =  43,     /*!<  43 [DeepSleep] GPIO Port Interrupt #21 */
  ioss_interrupts_gpio_dpslp_22_IRQn =  44,     /*!<  44 [DeepSleep] GPIO Port Interrupt #22 */
  ioss_interrupts_gpio_dpslp_23_IRQn =  45,     /*!<  45 [DeepSleep] GPIO Port Interrupt #23 */
  ioss_interrupts_gpio_dpslp_28_IRQn =  46,     /*!<  46 [DeepSleep] GPIO Port Interrupt #28 */
  ioss_interrupts_gpio_dpslp_29_IRQn =  47,     /*!<  47 [DeepSleep] GPIO Port Interrupt #29 */
  ioss_interrupts_gpio_dpslp_30_IRQn =  48,     /*!<  48 [DeepSleep] GPIO Port Interrupt #30 */
  ioss_interrupts_gpio_dpslp_31_IRQn =  49,     /*!<  49 [DeepSleep] GPIO Port Interrupt #31 */
  ioss_interrupts_gpio_dpslp_32_IRQn =  50,     /*!<  50 [DeepSleep] GPIO Port Interrupt #32 */
  ioss_interrupt_gpio_act_IRQn      =  51,      /*!<  51 [Active] Consolidated Interrupt for HSIO, All Ports */
  ioss_interrupts_gpio_act_24_IRQn  =  52,      /*!<  52 [Active] HSIO Port Interrupt #24 */
  ioss_interrupts_gpio_act_25_IRQn  =  53,      /*!<  53 [Active] HSIO Port Interrupt #25 */
  ioss_interrupts_gpio_act_26_IRQn  =  54,      /*!<  54 [Active] HSIO Port Interrupt #26 */
  ioss_interrupts_gpio_act_27_IRQn  =  55,      /*!<  55 [Active] HSIO Port Interrupt #27 */
  cpuss_interrupt_crypto_IRQn       =  56,      /*!<  56 [Active] CRYPTO Accelerator Interrupt */
  cpuss_interrupt_fm_IRQn           =  57,      /*!<  57 [Active] FLASH Macro Interrupt */
  cpuss_interrupts_cm7_0_fp_IRQn    =  58,      /*!<  58 [Active] CM7 #0 Floating Point operation fault */
  cpuss_interrupts_cm7_1_fp_IRQn    =  59,      /*!<  59 [Active] CM7 #1 Floating Point operation fault */
  cpuss_interrupts_cm0_cti_0_IRQn   =  60,      /*!<  60 [Active] CM0+ CTI #0 */
  cpuss_interrupts_cm0_cti_1_IRQn   =  61,      /*!<  61 [Active] CM0+ CTI #1 */
  cpuss_interrupts_cm7_0_cti_0_IRQn =  62,      /*!<  62 [Active] CM7 #0 CTI #0 */
  cpuss_interrupts_cm7_0_cti_1_IRQn =  63,      /*!<  63 [Active] CM7 #0 CTI #1 */
  cpuss_interrupts_cm7_1_cti_0_IRQn =  64,      /*!<  64 [Active] CM7 #1 CTI #0 */
  cpuss_interrupts_cm7_1_cti_1_IRQn =  65,      /*!<  65 [Active] CM7 #1 CTI #1 */
  evtgen_0_interrupt_IRQn           =  66,      /*!<  66 [Active] Event gen Active domain interrupt */
  canfd_0_interrupt0_IRQn           =  67,      /*!<  67 [Active] Can #0, Consolidated interrupt #0 for all three channels */
  canfd_0_interrupt1_IRQn           =  68,      /*!<  68 [Active] Can #0, Consolidated interrupt #1 for all three channels */
  canfd_1_interrupt0_IRQn           =  69,      /*!<  69 [Active] Can #1, Consolidated interrupt #0 for all three channels */
  canfd_1_interrupt1_IRQn           =  70,      /*!<  70 [Active] Can #1, Consolidated interrupt #1 for all three channels */
  canfd_0_interrupts0_0_IRQn        =  71,      /*!<  71 [Active] CAN #0, Interrupt #0, Channel #0 */
  canfd_0_interrupts0_1_IRQn        =  72,      /*!<  72 [Active] CAN #0, Interrupt #0, Channel #1 */
  canfd_0_interrupts0_2_IRQn        =  73,      /*!<  73 [Active] CAN #0, Interrupt #0, Channel #2 */
  canfd_0_interrupts0_3_IRQn        =  74,      /*!<  74 [Active] CAN #0, Interrupt #0, Channel #3 */
  canfd_0_interrupts1_0_IRQn        =  75,      /*!<  75 [Active] CAN #0, Interrupt #1, Channel #0 */
  canfd_0_interrupts1_1_IRQn        =  76,      /*!<  76 [Active] CAN #0, Interrupt #1, Channel #1 */
  canfd_0_interrupts1_2_IRQn        =  77,      /*!<  77 [Active] CAN #0, Interrupt #1, Channel #2 */
  canfd_0_interrupts1_3_IRQn        =  78,      /*!<  78 [Active] CAN #0, Interrupt #1, Channel #3 */
  canfd_1_interrupts0_0_IRQn        =  79,      /*!<  79 [Active] CAN #1, Interrupt #0, Channel #0 */
  canfd_1_interrupts0_1_IRQn        =  80,      /*!<  80 [Active] CAN #1, Interrupt #0, Channel #1 */
  canfd_1_interrupts0_2_IRQn        =  81,      /*!<  81 [Active] CAN #1, Interrupt #0, Channel #2 */
  canfd_1_interrupts0_3_IRQn        =  82,      /*!<  82 [Active] CAN #1, Interrupt #0, Channel #3 */
  canfd_1_interrupts1_0_IRQn        =  83,      /*!<  83 [Active] CAN #1, Interrupt #1, Channel #0 */
  canfd_1_interrupts1_1_IRQn        =  84,      /*!<  84 [Active] CAN #1, Interrupt #1, Channel #1 */
  canfd_1_interrupts1_2_IRQn        =  85,      /*!<  85 [Active] CAN #1, Interrupt #1, Channel #2 */
  canfd_1_interrupts1_3_IRQn        =  86,      /*!<  86 [Active] CAN #1, Interrupt #1, Channel #3 */
  lin_0_interrupts_0_IRQn           =  87,      /*!<  87 [Active] LIN Interrupt, Channel #0 */
  lin_0_interrupts_1_IRQn           =  88,      /*!<  88 [Active] LIN Interrupt, Channel #1 */
  lin_0_interrupts_2_IRQn           =  89,      /*!<  89 [Active] LIN Interrupt, Channel #2 */
  lin_0_interrupts_3_IRQn           =  90,      /*!<  90 [Active] LIN Interrupt, Channel #3 */
  lin_0_interrupts_4_IRQn           =  91,      /*!<  91 [Active] LIN Interrupt, Channel #4 */
  lin_0_interrupts_5_IRQn           =  92,      /*!<  92 [Active] LIN Interrupt, Channel #5 */
  lin_0_interrupts_6_IRQn           =  93,      /*!<  93 [Active] LIN Interrupt, Channel #6 */
  lin_0_interrupts_7_IRQn           =  94,      /*!<  94 [Active] LIN Interrupt, Channel #7 */
  lin_0_interrupts_8_IRQn           =  95,      /*!<  95 [Active] LIN Interrupt, Channel #8 */
  lin_0_interrupts_9_IRQn           =  96,      /*!<  96 [Active] LIN Interrupt, Channel #9 */
  lin_0_interrupts_10_IRQn          =  97,      /*!<  97 [Active] LIN Interrupt, Channel #10 */
  lin_0_interrupts_11_IRQn          =  98,      /*!<  98 [Active] LIN Interrupt, Channel #11 */
  lin_0_interrupts_12_IRQn          =  99,      /*!<  99 [Active] LIN Interrupt, Channel #12 */
  lin_0_interrupts_13_IRQn          = 100,      /*!< 100 [Active] LIN Interrupt, Channel #13 */
  lin_0_interrupts_14_IRQn          = 101,      /*!< 101 [Active] LIN Interrupt, Channel #14 */
  lin_0_interrupts_15_IRQn          = 102,      /*!< 102 [Active] LIN Interrupt, Channel #15 */
  scb_1_interrupt_IRQn              = 103,      /*!< 103 [Active] Serial Communication Block #1 */
  scb_2_interrupt_IRQn              = 104,      /*!< 104 [Active] Serial Communication Block #2 */
  scb_3_interrupt_IRQn              = 105,      /*!< 105 [Active] Serial Communication Block #3 */
  scb_4_interrupt_IRQn              = 106,      /*!< 106 [Active] Serial Communication Block #4 */
  scb_5_interrupt_IRQn              = 107,      /*!< 107 [Active] Serial Communication Block #5 */
  scb_6_interrupt_IRQn              = 108,      /*!< 108 [Active] Serial Communication Block #6 */
  scb_7_interrupt_IRQn              = 109,      /*!< 109 [Active] Serial Communication Block #7 */
  scb_8_interrupt_IRQn              = 110,      /*!< 110 [Active] Serial Communication Block #8 */
  scb_9_interrupt_IRQn              = 111,      /*!< 111 [Active] Serial Communication Block #9 */
  scb_10_interrupt_IRQn             = 112,      /*!< 112 [Active] Serial Communication Block #10 */
  pass_0_interrupts_sar_0_IRQn      = 113,      /*!< 113 [Active] PASS interrupts, ADC #0, Channel #0 */
  pass_0_interrupts_sar_1_IRQn      = 114,      /*!< 114 [Active] PASS interrupts, ADC #0, Channel #1 */
  pass_0_interrupts_sar_2_IRQn      = 115,      /*!< 115 [Active] PASS interrupts, ADC #0, Channel #2 */
  pass_0_interrupts_sar_3_IRQn      = 116,      /*!< 116 [Active] PASS interrupts, ADC #0, Channel #3 */
  pass_0_interrupts_sar_4_IRQn      = 117,      /*!< 117 [Active] PASS interrupts, ADC #0, Channel #4 */
  pass_0_interrupts_sar_5_IRQn      = 118,      /*!< 118 [Active] PASS interrupts, ADC #0, Channel #5 */
  pass_0_interrupts_sar_6_IRQn      = 119,      /*!< 119 [Active] PASS interrupts, ADC #0, Channel #6 */
  pass_0_interrupts_sar_7_IRQn      = 120,      /*!< 120 [Active] PASS interrupts, ADC #0, Channel #7 */
  pass_0_interrupts_sar_8_IRQn      = 121,      /*!< 121 [Active] PASS interrupts, ADC #0, Channel #8 */
  pass_0_interrupts_sar_9_IRQn      = 122,      /*!< 122 [Active] PASS interrupts, ADC #0, Channel #9 */
  pass_0_interrupts_sar_10_IRQn     = 123,      /*!< 123 [Active] PASS interrupts, ADC #0, Channel #10 */
  pass_0_interrupts_sar_11_IRQn     = 124,      /*!< 124 [Active] PASS interrupts, ADC #0, Channel #11 */
  pass_0_interrupts_sar_12_IRQn     = 125,      /*!< 125 [Active] PASS interrupts, ADC #0, Channel #12 */
  pass_0_interrupts_sar_13_IRQn     = 126,      /*!< 126 [Active] PASS interrupts, ADC #0, Channel #13 */
  pass_0_interrupts_sar_14_IRQn     = 127,      /*!< 127 [Active] PASS interrupts, ADC #0, Channel #14 */
  pass_0_interrupts_sar_15_IRQn     = 128,      /*!< 128 [Active] PASS interrupts, ADC #0, Channel #15 */
  pass_0_interrupts_sar_16_IRQn     = 129,      /*!< 129 [Active] PASS interrupts, ADC #0, Channel #16 */
  pass_0_interrupts_sar_17_IRQn     = 130,      /*!< 130 [Active] PASS interrupts, ADC #0, Channel #17 */
  pass_0_interrupts_sar_18_IRQn     = 131,      /*!< 131 [Active] PASS interrupts, ADC #0, Channel #18 */
  pass_0_interrupts_sar_19_IRQn     = 132,      /*!< 132 [Active] PASS interrupts, ADC #0, Channel #19 */
  pass_0_interrupts_sar_20_IRQn     = 133,      /*!< 133 [Active] PASS interrupts, ADC #0, Channel #20 */
  pass_0_interrupts_sar_21_IRQn     = 134,      /*!< 134 [Active] PASS interrupts, ADC #0, Channel #21 */
  pass_0_interrupts_sar_22_IRQn     = 135,      /*!< 135 [Active] PASS interrupts, ADC #0, Channel #22 */
  pass_0_interrupts_sar_23_IRQn     = 136,      /*!< 136 [Active] PASS interrupts, ADC #0, Channel #23 */
  pass_0_interrupts_sar_24_IRQn     = 137,      /*!< 137 [Active] PASS interrupts, ADC #0, Channel #24 */
  pass_0_interrupts_sar_25_IRQn     = 138,      /*!< 138 [Active] PASS interrupts, ADC #0, Channel #25 */
  pass_0_interrupts_sar_26_IRQn     = 139,      /*!< 139 [Active] PASS interrupts, ADC #0, Channel #26 */
  pass_0_interrupts_sar_27_IRQn     = 140,      /*!< 140 [Active] PASS interrupts, ADC #0, Channel #27 */
  pass_0_interrupts_sar_28_IRQn     = 141,      /*!< 141 [Active] PASS interrupts, ADC #0, Channel #28 */
  pass_0_interrupts_sar_29_IRQn     = 142,      /*!< 142 [Active] PASS interrupts, ADC #0, Channel #29 */
  pass_0_interrupts_sar_30_IRQn     = 143,      /*!< 143 [Active] PASS interrupts, ADC #0, Channel #30 */
  pass_0_interrupts_sar_31_IRQn     = 144,      /*!< 144 [Active] PASS interrupts, ADC #0, Channel #31 */
  pass_0_interrupts_sar_32_IRQn     = 145,      /*!< 145 [Active] PASS interrupts, ADC #1, Channel #0 */
  pass_0_interrupts_sar_33_IRQn     = 146,      /*!< 146 [Active] PASS interrupts, ADC #1, Channel #1 */
  pass_0_interrupts_sar_34_IRQn     = 147,      /*!< 147 [Active] PASS interrupts, ADC #1, Channel #2 */
  pass_0_interrupts_sar_35_IRQn     = 148,      /*!< 148 [Active] PASS interrupts, ADC #1, Channel #3 */
  pass_0_interrupts_sar_36_IRQn     = 149,      /*!< 149 [Active] PASS interrupts, ADC #1, Channel #4 */
  pass_0_interrupts_sar_37_IRQn     = 150,      /*!< 150 [Active] PASS interrupts, ADC #1, Channel #5 */
  pass_0_interrupts_sar_38_IRQn     = 151,      /*!< 151 [Active] PASS interrupts, ADC #1, Channel #6 */
  pass_0_interrupts_sar_39_IRQn     = 152,      /*!< 152 [Active] PASS interrupts, ADC #1, Channel #7 */
  pass_0_interrupts_sar_40_IRQn     = 153,      /*!< 153 [Active] PASS interrupts, ADC #1, Channel #8 */
  pass_0_interrupts_sar_41_IRQn     = 154,      /*!< 154 [Active] PASS interrupts, ADC #1, Channel #9 */
  pass_0_interrupts_sar_42_IRQn     = 155,      /*!< 155 [Active] PASS interrupts, ADC #1, Channel #10 */
  pass_0_interrupts_sar_43_IRQn     = 156,      /*!< 156 [Active] PASS interrupts, ADC #1, Channel #11 */
  pass_0_interrupts_sar_44_IRQn     = 157,      /*!< 157 [Active] PASS interrupts, ADC #1, Channel #12 */
  pass_0_interrupts_sar_45_IRQn     = 158,      /*!< 158 [Active] PASS interrupts, ADC #1, Channel #13 */
  pass_0_interrupts_sar_46_IRQn     = 159,      /*!< 159 [Active] PASS interrupts, ADC #1, Channel #14 */
  pass_0_interrupts_sar_47_IRQn     = 160,      /*!< 160 [Active] PASS interrupts, ADC #1, Channel #15 */
  pass_0_interrupts_sar_48_IRQn     = 161,      /*!< 161 [Active] PASS interrupts, ADC #1, Channel #16 */
  pass_0_interrupts_sar_49_IRQn     = 162,      /*!< 162 [Active] PASS interrupts, ADC #1, Channel #17 */
  pass_0_interrupts_sar_50_IRQn     = 163,      /*!< 163 [Active] PASS interrupts, ADC #1, Channel #18 */
  pass_0_interrupts_sar_51_IRQn     = 164,      /*!< 164 [Active] PASS interrupts, ADC #1, Channel #19 */
  pass_0_interrupts_sar_52_IRQn     = 165,      /*!< 165 [Active] PASS interrupts, ADC #1, Channel #20 */
  pass_0_interrupts_sar_53_IRQn     = 166,      /*!< 166 [Active] PASS interrupts, ADC #1, Channel #21 */
  pass_0_interrupts_sar_54_IRQn     = 167,      /*!< 167 [Active] PASS interrupts, ADC #1, Channel #22 */
  pass_0_interrupts_sar_55_IRQn     = 168,      /*!< 168 [Active] PASS interrupts, ADC #1, Channel #23 */
  pass_0_interrupts_sar_56_IRQn     = 169,      /*!< 169 [Active] PASS interrupts, ADC #1, Channel #24 */
  pass_0_interrupts_sar_57_IRQn     = 170,      /*!< 170 [Active] PASS interrupts, ADC #1, Channel #25 */
  pass_0_interrupts_sar_58_IRQn     = 171,      /*!< 171 [Active] PASS interrupts, ADC #1, Channel #26 */
  pass_0_interrupts_sar_59_IRQn     = 172,      /*!< 172 [Active] PASS interrupts, ADC #1, Channel #27 */
  pass_0_interrupts_sar_60_IRQn     = 173,      /*!< 173 [Active] PASS interrupts, ADC #1, Channel #28 */
  pass_0_interrupts_sar_61_IRQn     = 174,      /*!< 174 [Active] PASS interrupts, ADC #1, Channel #29 */
  pass_0_interrupts_sar_62_IRQn     = 175,      /*!< 175 [Active] PASS interrupts, ADC #1, Channel #30 */
  pass_0_interrupts_sar_63_IRQn     = 176,      /*!< 176 [Active] PASS interrupts, ADC #1, Channel #31 */
  pass_0_interrupts_sar_64_IRQn     = 177,      /*!< 177 [Active] PASS interrupts, ADC #2, Channel #0 */
  pass_0_interrupts_sar_65_IRQn     = 178,      /*!< 178 [Active] PASS interrupts, ADC #2, Channel #1 */
  pass_0_interrupts_sar_66_IRQn     = 179,      /*!< 179 [Active] PASS interrupts, ADC #2, Channel #2 */
  pass_0_interrupts_sar_67_IRQn     = 180,      /*!< 180 [Active] PASS interrupts, ADC #2, Channel #3 */
  pass_0_interrupts_sar_68_IRQn     = 181,      /*!< 181 [Active] PASS interrupts, ADC #2, Channel #4 */
  pass_0_interrupts_sar_69_IRQn     = 182,      /*!< 182 [Active] PASS interrupts, ADC #2, Channel #5 */
  pass_0_interrupts_sar_70_IRQn     = 183,      /*!< 183 [Active] PASS interrupts, ADC #2, Channel #6 */
  pass_0_interrupts_sar_71_IRQn     = 184,      /*!< 184 [Active] PASS interrupts, ADC #2, Channel #7 */
  cpuss_interrupts_dmac_0_IRQn      = 185,      /*!< 185 [Active] CPUSS DMAC, Channel #0 */
  cpuss_interrupts_dmac_1_IRQn      = 186,      /*!< 186 [Active] CPUSS DMAC, Channel #1 */
  cpuss_interrupts_dmac_2_IRQn      = 187,      /*!< 187 [Active] CPUSS DMAC, Channel #2 */
  cpuss_interrupts_dmac_3_IRQn      = 188,      /*!< 188 [Active] CPUSS DMAC, Channel #3 */
  cpuss_interrupts_dmac_4_IRQn      = 189,      /*!< 189 [Active] CPUSS DMAC, Channel #4 */
  cpuss_interrupts_dmac_5_IRQn      = 190,      /*!< 190 [Active] CPUSS DMAC, Channel #5 */
  cpuss_interrupts_dmac_6_IRQn      = 191,      /*!< 191 [Active] CPUSS DMAC, Channel #6 */
  cpuss_interrupts_dmac_7_IRQn      = 192,      /*!< 192 [Active] CPUSS DMAC, Channel #7 */
  cpuss_interrupts_dw0_0_IRQn       = 193,      /*!< 193 [Active] CPUSS DataWire #0, Channel #0 */
  cpuss_interrupts_dw0_1_IRQn       = 194,      /*!< 194 [Active] CPUSS DataWire #0, Channel #1 */
  cpuss_interrupts_dw0_2_IRQn       = 195,      /*!< 195 [Active] CPUSS DataWire #0, Channel #2 */
  cpuss_interrupts_dw0_3_IRQn       = 196,      /*!< 196 [Active] CPUSS DataWire #0, Channel #3 */
  cpuss_interrupts_dw0_4_IRQn       = 197,      /*!< 197 [Active] CPUSS DataWire #0, Channel #4 */
  cpuss_interrupts_dw0_5_IRQn       = 198,      /*!< 198 [Active] CPUSS DataWire #0, Channel #5 */
  cpuss_interrupts_dw0_6_IRQn       = 199,      /*!< 199 [Active] CPUSS DataWire #0, Channel #6 */
  cpuss_interrupts_dw0_7_IRQn       = 200,      /*!< 200 [Active] CPUSS DataWire #0, Channel #7 */
  cpuss_interrupts_dw0_8_IRQn       = 201,      /*!< 201 [Active] CPUSS DataWire #0, Channel #8 */
  cpuss_interrupts_dw0_9_IRQn       = 202,      /*!< 202 [Active] CPUSS DataWire #0, Channel #9 */
  cpuss_interrupts_dw0_10_IRQn      = 203,      /*!< 203 [Active] CPUSS DataWire #0, Channel #10 */
  cpuss_interrupts_dw0_11_IRQn      = 204,      /*!< 204 [Active] CPUSS DataWire #0, Channel #11 */
  cpuss_interrupts_dw0_12_IRQn      = 205,      /*!< 205 [Active] CPUSS DataWire #0, Channel #12 */
  cpuss_interrupts_dw0_13_IRQn      = 206,      /*!< 206 [Active] CPUSS DataWire #0, Channel #13 */
  cpuss_interrupts_dw0_14_IRQn      = 207,      /*!< 207 [Active] CPUSS DataWire #0, Channel #14 */
  cpuss_interrupts_dw0_15_IRQn      = 208,      /*!< 208 [Active] CPUSS DataWire #0, Channel #15 */
  cpuss_interrupts_dw0_16_IRQn      = 209,      /*!< 209 [Active] CPUSS DataWire #0, Channel #16 */
  cpuss_interrupts_dw0_17_IRQn      = 210,      /*!< 210 [Active] CPUSS DataWire #0, Channel #17 */
  cpuss_interrupts_dw0_18_IRQn      = 211,      /*!< 211 [Active] CPUSS DataWire #0, Channel #18 */
  cpuss_interrupts_dw0_19_IRQn      = 212,      /*!< 212 [Active] CPUSS DataWire #0, Channel #19 */
  cpuss_interrupts_dw0_20_IRQn      = 213,      /*!< 213 [Active] CPUSS DataWire #0, Channel #20 */
  cpuss_interrupts_dw0_21_IRQn      = 214,      /*!< 214 [Active] CPUSS DataWire #0, Channel #21 */
  cpuss_interrupts_dw0_22_IRQn      = 215,      /*!< 215 [Active] CPUSS DataWire #0, Channel #22 */
  cpuss_interrupts_dw0_23_IRQn      = 216,      /*!< 216 [Active] CPUSS DataWire #0, Channel #23 */
  cpuss_interrupts_dw0_24_IRQn      = 217,      /*!< 217 [Active] CPUSS DataWire #0, Channel #24 */
  cpuss_interrupts_dw0_25_IRQn      = 218,      /*!< 218 [Active] CPUSS DataWire #0, Channel #25 */
  cpuss_interrupts_dw0_26_IRQn      = 219,      /*!< 219 [Active] CPUSS DataWire #0, Channel #26 */
  cpuss_interrupts_dw0_27_IRQn      = 220,      /*!< 220 [Active] CPUSS DataWire #0, Channel #27 */
  cpuss_interrupts_dw0_28_IRQn      = 221,      /*!< 221 [Active] CPUSS DataWire #0, Channel #28 */
  cpuss_interrupts_dw0_29_IRQn      = 222,      /*!< 222 [Active] CPUSS DataWire #0, Channel #29 */
  cpuss_interrupts_dw0_30_IRQn      = 223,      /*!< 223 [Active] CPUSS DataWire #0, Channel #30 */
  cpuss_interrupts_dw0_31_IRQn      = 224,      /*!< 224 [Active] CPUSS DataWire #0, Channel #31 */
  cpuss_interrupts_dw0_32_IRQn      = 225,      /*!< 225 [Active] CPUSS DataWire #0, Channel #32 */
  cpuss_interrupts_dw0_33_IRQn      = 226,      /*!< 226 [Active] CPUSS DataWire #0, Channel #33 */
  cpuss_interrupts_dw0_34_IRQn      = 227,      /*!< 227 [Active] CPUSS DataWire #0, Channel #34 */
  cpuss_interrupts_dw0_35_IRQn      = 228,      /*!< 228 [Active] CPUSS DataWire #0, Channel #35 */
  cpuss_interrupts_dw0_36_IRQn      = 229,      /*!< 229 [Active] CPUSS DataWire #0, Channel #36 */
  cpuss_interrupts_dw0_37_IRQn      = 230,      /*!< 230 [Active] CPUSS DataWire #0, Channel #37 */
  cpuss_interrupts_dw0_38_IRQn      = 231,      /*!< 231 [Active] CPUSS DataWire #0, Channel #38 */
  cpuss_interrupts_dw0_39_IRQn      = 232,      /*!< 232 [Active] CPUSS DataWire #0, Channel #39 */
  cpuss_interrupts_dw0_40_IRQn      = 233,      /*!< 233 [Active] CPUSS DataWire #0, Channel #40 */
  cpuss_interrupts_dw0_41_IRQn      = 234,      /*!< 234 [Active] CPUSS DataWire #0, Channel #41 */
  cpuss_interrupts_dw0_42_IRQn      = 235,      /*!< 235 [Active] CPUSS DataWire #0, Channel #42 */
  cpuss_interrupts_dw0_43_IRQn      = 236,      /*!< 236 [Active] CPUSS DataWire #0, Channel #43 */
  cpuss_interrupts_dw0_44_IRQn      = 237,      /*!< 237 [Active] CPUSS DataWire #0, Channel #44 */
  cpuss_interrupts_dw0_45_IRQn      = 238,      /*!< 238 [Active] CPUSS DataWire #0, Channel #45 */
  cpuss_interrupts_dw0_46_IRQn      = 239,      /*!< 239 [Active] CPUSS DataWire #0, Channel #46 */
  cpuss_interrupts_dw0_47_IRQn      = 240,      /*!< 240 [Active] CPUSS DataWire #0, Channel #47 */
  cpuss_interrupts_dw0_48_IRQn      = 241,      /*!< 241 [Active] CPUSS DataWire #0, Channel #48 */
  cpuss_interrupts_dw0_49_IRQn      = 242,      /*!< 242 [Active] CPUSS DataWire #0, Channel #49 */
  cpuss_interrupts_dw0_50_IRQn      = 243,      /*!< 243 [Active] CPUSS DataWire #0, Channel #50 */
  cpuss_interrupts_dw0_51_IRQn      = 244,      /*!< 244 [Active] CPUSS DataWire #0, Channel #51 */
  cpuss_interrupts_dw0_52_IRQn      = 245,      /*!< 245 [Active] CPUSS DataWire #0, Channel #52 */
  cpuss_interrupts_dw0_53_IRQn      = 246,      /*!< 246 [Active] CPUSS DataWire #0, Channel #53 */
  cpuss_interrupts_dw0_54_IRQn      = 247,      /*!< 247 [Active] CPUSS DataWire #0, Channel #54 */
  cpuss_interrupts_dw0_55_IRQn      = 248,      /*!< 248 [Active] CPUSS DataWire #0, Channel #55 */
  cpuss_interrupts_dw0_56_IRQn      = 249,      /*!< 249 [Active] CPUSS DataWire #0, Channel #56 */
  cpuss_interrupts_dw0_57_IRQn      = 250,      /*!< 250 [Active] CPUSS DataWire #0, Channel #57 */
  cpuss_interrupts_dw0_58_IRQn      = 251,      /*!< 251 [Active] CPUSS DataWire #0, Channel #58 */
  cpuss_interrupts_dw0_59_IRQn      = 252,      /*!< 252 [Active] CPUSS DataWire #0, Channel #59 */
  cpuss_interrupts_dw0_60_IRQn      = 253,      /*!< 253 [Active] CPUSS DataWire #0, Channel #60 */
  cpuss_interrupts_dw0_61_IRQn      = 254,      /*!< 254 [Active] CPUSS DataWire #0, Channel #61 */
  cpuss_interrupts_dw0_62_IRQn      = 255,      /*!< 255 [Active] CPUSS DataWire #0, Channel #62 */
  cpuss_interrupts_dw0_63_IRQn      = 256,      /*!< 256 [Active] CPUSS DataWire #0, Channel #63 */
  cpuss_interrupts_dw0_64_IRQn      = 257,      /*!< 257 [Active] CPUSS DataWire #0, Channel #64 */
  cpuss_interrupts_dw0_65_IRQn      = 258,      /*!< 258 [Active] CPUSS DataWire #0, Channel #65 */
  cpuss_interrupts_dw0_66_IRQn      = 259,      /*!< 259 [Active] CPUSS DataWire #0, Channel #66 */
  cpuss_interrupts_dw0_67_IRQn      = 260,      /*!< 260 [Active] CPUSS DataWire #0, Channel #67 */
  cpuss_interrupts_dw0_68_IRQn      = 261,      /*!< 261 [Active] CPUSS DataWire #0, Channel #68 */
  cpuss_interrupts_dw0_69_IRQn      = 262,      /*!< 262 [Active] CPUSS DataWire #0, Channel #69 */
  cpuss_interrupts_dw0_70_IRQn      = 263,      /*!< 263 [Active] CPUSS DataWire #0, Channel #70 */
  cpuss_interrupts_dw0_71_IRQn      = 264,      /*!< 264 [Active] CPUSS DataWire #0, Channel #71 */
  cpuss_interrupts_dw0_72_IRQn      = 265,      /*!< 265 [Active] CPUSS DataWire #0, Channel #72 */
  cpuss_interrupts_dw0_73_IRQn      = 266,      /*!< 266 [Active] CPUSS DataWire #0, Channel #73 */
  cpuss_interrupts_dw0_74_IRQn      = 267,      /*!< 267 [Active] CPUSS DataWire #0, Channel #74 */
  cpuss_interrupts_dw0_75_IRQn      = 268,      /*!< 268 [Active] CPUSS DataWire #0, Channel #75 */
  cpuss_interrupts_dw0_76_IRQn      = 269,      /*!< 269 [Active] CPUSS DataWire #0, Channel #76 */
  cpuss_interrupts_dw0_77_IRQn      = 270,      /*!< 270 [Active] CPUSS DataWire #0, Channel #77 */
  cpuss_interrupts_dw0_78_IRQn      = 271,      /*!< 271 [Active] CPUSS DataWire #0, Channel #78 */
  cpuss_interrupts_dw0_79_IRQn      = 272,      /*!< 272 [Active] CPUSS DataWire #0, Channel #79 */
  cpuss_interrupts_dw0_80_IRQn      = 273,      /*!< 273 [Active] CPUSS DataWire #0, Channel #80 */
  cpuss_interrupts_dw0_81_IRQn      = 274,      /*!< 274 [Active] CPUSS DataWire #0, Channel #81 */
  cpuss_interrupts_dw0_82_IRQn      = 275,      /*!< 275 [Active] CPUSS DataWire #0, Channel #82 */
  cpuss_interrupts_dw0_83_IRQn      = 276,      /*!< 276 [Active] CPUSS DataWire #0, Channel #83 */
  cpuss_interrupts_dw0_84_IRQn      = 277,      /*!< 277 [Active] CPUSS DataWire #0, Channel #84 */
  cpuss_interrupts_dw0_85_IRQn      = 278,      /*!< 278 [Active] CPUSS DataWire #0, Channel #85 */
  cpuss_interrupts_dw0_86_IRQn      = 279,      /*!< 279 [Active] CPUSS DataWire #0, Channel #86 */
  cpuss_interrupts_dw0_87_IRQn      = 280,      /*!< 280 [Active] CPUSS DataWire #0, Channel #87 */
  cpuss_interrupts_dw0_88_IRQn      = 281,      /*!< 281 [Active] CPUSS DataWire #0, Channel #88 */
  cpuss_interrupts_dw0_89_IRQn      = 282,      /*!< 282 [Active] CPUSS DataWire #0, Channel #89 */
  cpuss_interrupts_dw0_90_IRQn      = 283,      /*!< 283 [Active] CPUSS DataWire #0, Channel #90 */
  cpuss_interrupts_dw0_91_IRQn      = 284,      /*!< 284 [Active] CPUSS DataWire #0, Channel #91 */
  cpuss_interrupts_dw0_92_IRQn      = 285,      /*!< 285 [Active] CPUSS DataWire #0, Channel #92 */
  cpuss_interrupts_dw0_93_IRQn      = 286,      /*!< 286 [Active] CPUSS DataWire #0, Channel #93 */
  cpuss_interrupts_dw0_94_IRQn      = 287,      /*!< 287 [Active] CPUSS DataWire #0, Channel #94 */
  cpuss_interrupts_dw0_95_IRQn      = 288,      /*!< 288 [Active] CPUSS DataWire #0, Channel #95 */
  cpuss_interrupts_dw0_96_IRQn      = 289,      /*!< 289 [Active] CPUSS DataWire #0, Channel #96 */
  cpuss_interrupts_dw0_97_IRQn      = 290,      /*!< 290 [Active] CPUSS DataWire #0, Channel #97 */
  cpuss_interrupts_dw0_98_IRQn      = 291,      /*!< 291 [Active] CPUSS DataWire #0, Channel #98 */
  cpuss_interrupts_dw0_99_IRQn      = 292,      /*!< 292 [Active] CPUSS DataWire #0, Channel #99 */
  cpuss_interrupts_dw1_0_IRQn       = 293,      /*!< 293 [Active] CPUSS DataWire #1, Channel #0 */
  cpuss_interrupts_dw1_1_IRQn       = 294,      /*!< 294 [Active] CPUSS DataWire #1, Channel #1 */
  cpuss_interrupts_dw1_2_IRQn       = 295,      /*!< 295 [Active] CPUSS DataWire #1, Channel #2 */
  cpuss_interrupts_dw1_3_IRQn       = 296,      /*!< 296 [Active] CPUSS DataWire #1, Channel #3 */
  cpuss_interrupts_dw1_4_IRQn       = 297,      /*!< 297 [Active] CPUSS DataWire #1, Channel #4 */
  cpuss_interrupts_dw1_5_IRQn       = 298,      /*!< 298 [Active] CPUSS DataWire #1, Channel #5 */
  cpuss_interrupts_dw1_6_IRQn       = 299,      /*!< 299 [Active] CPUSS DataWire #1, Channel #6 */
  cpuss_interrupts_dw1_7_IRQn       = 300,      /*!< 300 [Active] CPUSS DataWire #1, Channel #7 */
  cpuss_interrupts_dw1_8_IRQn       = 301,      /*!< 301 [Active] CPUSS DataWire #1, Channel #8 */
  cpuss_interrupts_dw1_9_IRQn       = 302,      /*!< 302 [Active] CPUSS DataWire #1, Channel #9 */
  cpuss_interrupts_dw1_10_IRQn      = 303,      /*!< 303 [Active] CPUSS DataWire #1, Channel #10 */
  cpuss_interrupts_dw1_11_IRQn      = 304,      /*!< 304 [Active] CPUSS DataWire #1, Channel #11 */
  cpuss_interrupts_dw1_12_IRQn      = 305,      /*!< 305 [Active] CPUSS DataWire #1, Channel #12 */
  cpuss_interrupts_dw1_13_IRQn      = 306,      /*!< 306 [Active] CPUSS DataWire #1, Channel #13 */
  cpuss_interrupts_dw1_14_IRQn      = 307,      /*!< 307 [Active] CPUSS DataWire #1, Channel #14 */
  cpuss_interrupts_dw1_15_IRQn      = 308,      /*!< 308 [Active] CPUSS DataWire #1, Channel #15 */
  cpuss_interrupts_dw1_16_IRQn      = 309,      /*!< 309 [Active] CPUSS DataWire #1, Channel #16 */
  cpuss_interrupts_dw1_17_IRQn      = 310,      /*!< 310 [Active] CPUSS DataWire #1, Channel #17 */
  cpuss_interrupts_dw1_18_IRQn      = 311,      /*!< 311 [Active] CPUSS DataWire #1, Channel #18 */
  cpuss_interrupts_dw1_19_IRQn      = 312,      /*!< 312 [Active] CPUSS DataWire #1, Channel #19 */
  cpuss_interrupts_dw1_20_IRQn      = 313,      /*!< 313 [Active] CPUSS DataWire #1, Channel #20 */
  cpuss_interrupts_dw1_21_IRQn      = 314,      /*!< 314 [Active] CPUSS DataWire #1, Channel #21 */
  cpuss_interrupts_dw1_22_IRQn      = 315,      /*!< 315 [Active] CPUSS DataWire #1, Channel #22 */
  cpuss_interrupts_dw1_23_IRQn      = 316,      /*!< 316 [Active] CPUSS DataWire #1, Channel #23 */
  cpuss_interrupts_dw1_24_IRQn      = 317,      /*!< 317 [Active] CPUSS DataWire #1, Channel #24 */
  cpuss_interrupts_dw1_25_IRQn      = 318,      /*!< 318 [Active] CPUSS DataWire #1, Channel #25 */
  cpuss_interrupts_dw1_26_IRQn      = 319,      /*!< 319 [Active] CPUSS DataWire #1, Channel #26 */
  cpuss_interrupts_dw1_27_IRQn      = 320,      /*!< 320 [Active] CPUSS DataWire #1, Channel #27 */
  cpuss_interrupts_dw1_28_IRQn      = 321,      /*!< 321 [Active] CPUSS DataWire #1, Channel #28 */
  cpuss_interrupts_dw1_29_IRQn      = 322,      /*!< 322 [Active] CPUSS DataWire #1, Channel #29 */
  cpuss_interrupts_dw1_30_IRQn      = 323,      /*!< 323 [Active] CPUSS DataWire #1, Channel #30 */
  cpuss_interrupts_dw1_31_IRQn      = 324,      /*!< 324 [Active] CPUSS DataWire #1, Channel #31 */
  cpuss_interrupts_dw1_32_IRQn      = 325,      /*!< 325 [Active] CPUSS DataWire #1, Channel #32 */
  cpuss_interrupts_dw1_33_IRQn      = 326,      /*!< 326 [Active] CPUSS DataWire #1, Channel #33 */
  cpuss_interrupts_dw1_34_IRQn      = 327,      /*!< 327 [Active] CPUSS DataWire #1, Channel #34 */
  cpuss_interrupts_dw1_35_IRQn      = 328,      /*!< 328 [Active] CPUSS DataWire #1, Channel #35 */
  cpuss_interrupts_dw1_36_IRQn      = 329,      /*!< 329 [Active] CPUSS DataWire #1, Channel #36 */
  cpuss_interrupts_dw1_37_IRQn      = 330,      /*!< 330 [Active] CPUSS DataWire #1, Channel #37 */
  cpuss_interrupts_dw1_38_IRQn      = 331,      /*!< 331 [Active] CPUSS DataWire #1, Channel #38 */
  cpuss_interrupts_dw1_39_IRQn      = 332,      /*!< 332 [Active] CPUSS DataWire #1, Channel #39 */
  cpuss_interrupts_dw1_40_IRQn      = 333,      /*!< 333 [Active] CPUSS DataWire #1, Channel #40 */
  cpuss_interrupts_dw1_41_IRQn      = 334,      /*!< 334 [Active] CPUSS DataWire #1, Channel #41 */
  cpuss_interrupts_dw1_42_IRQn      = 335,      /*!< 335 [Active] CPUSS DataWire #1, Channel #42 */
  cpuss_interrupts_dw1_43_IRQn      = 336,      /*!< 336 [Active] CPUSS DataWire #1, Channel #43 */
  cpuss_interrupts_dw1_44_IRQn      = 337,      /*!< 337 [Active] CPUSS DataWire #1, Channel #44 */
  cpuss_interrupts_dw1_45_IRQn      = 338,      /*!< 338 [Active] CPUSS DataWire #1, Channel #45 */
  cpuss_interrupts_dw1_46_IRQn      = 339,      /*!< 339 [Active] CPUSS DataWire #1, Channel #46 */
  cpuss_interrupts_dw1_47_IRQn      = 340,      /*!< 340 [Active] CPUSS DataWire #1, Channel #47 */
  cpuss_interrupts_dw1_48_IRQn      = 341,      /*!< 341 [Active] CPUSS DataWire #1, Channel #48 */
  cpuss_interrupts_dw1_49_IRQn      = 342,      /*!< 342 [Active] CPUSS DataWire #1, Channel #49 */
  cpuss_interrupts_dw1_50_IRQn      = 343,      /*!< 343 [Active] CPUSS DataWire #1, Channel #50 */
  cpuss_interrupts_dw1_51_IRQn      = 344,      /*!< 344 [Active] CPUSS DataWire #1, Channel #51 */
  cpuss_interrupts_dw1_52_IRQn      = 345,      /*!< 345 [Active] CPUSS DataWire #1, Channel #52 */
  cpuss_interrupts_dw1_53_IRQn      = 346,      /*!< 346 [Active] CPUSS DataWire #1, Channel #53 */
  cpuss_interrupts_dw1_54_IRQn      = 347,      /*!< 347 [Active] CPUSS DataWire #1, Channel #54 */
  cpuss_interrupts_dw1_55_IRQn      = 348,      /*!< 348 [Active] CPUSS DataWire #1, Channel #55 */
  cpuss_interrupts_dw1_56_IRQn      = 349,      /*!< 349 [Active] CPUSS DataWire #1, Channel #56 */
  cpuss_interrupts_dw1_57_IRQn      = 350,      /*!< 350 [Active] CPUSS DataWire #1, Channel #57 */
  tcpwm_0_interrupts_0_IRQn         = 351,      /*!< 351 [Active] tcpwm[0] Group #0, Counter #0 */
  tcpwm_0_interrupts_1_IRQn         = 352,      /*!< 352 [Active] tcpwm[0] Group #0, Counter #1 */
  tcpwm_0_interrupts_2_IRQn         = 353,      /*!< 353 [Active] tcpwm[0] Group #0, Counter #2 */
  tcpwm_0_interrupts_3_IRQn         = 354,      /*!< 354 [Active] tcpwm[0] Group #0, Counter #3 */
  tcpwm_0_interrupts_4_IRQn         = 355,      /*!< 355 [Active] tcpwm[0] Group #0, Counter #4 */
  tcpwm_0_interrupts_5_IRQn         = 356,      /*!< 356 [Active] tcpwm[0] Group #0, Counter #5 */
  tcpwm_0_interrupts_6_IRQn         = 357,      /*!< 357 [Active] tcpwm[0] Group #0, Counter #6 */
  tcpwm_0_interrupts_7_IRQn         = 358,      /*!< 358 [Active] tcpwm[0] Group #0, Counter #7 */
  tcpwm_0_interrupts_8_IRQn         = 359,      /*!< 359 [Active] tcpwm[0] Group #0, Counter #8 */
  tcpwm_0_interrupts_9_IRQn         = 360,      /*!< 360 [Active] tcpwm[0] Group #0, Counter #9 */
  tcpwm_0_interrupts_10_IRQn        = 361,      /*!< 361 [Active] tcpwm[0] Group #0, Counter #10 */
  tcpwm_0_interrupts_11_IRQn        = 362,      /*!< 362 [Active] tcpwm[0] Group #0, Counter #11 */
  tcpwm_0_interrupts_12_IRQn        = 363,      /*!< 363 [Active] tcpwm[0] Group #0, Counter #12 */
  tcpwm_0_interrupts_13_IRQn        = 364,      /*!< 364 [Active] tcpwm[0] Group #0, Counter #13 */
  tcpwm_0_interrupts_14_IRQn        = 365,      /*!< 365 [Active] tcpwm[0] Group #0, Counter #14 */
  tcpwm_0_interrupts_15_IRQn        = 366,      /*!< 366 [Active] tcpwm[0] Group #0, Counter #15 */
  tcpwm_0_interrupts_16_IRQn        = 367,      /*!< 367 [Active] tcpwm[0] Group #0, Counter #16 */
  tcpwm_0_interrupts_17_IRQn        = 368,      /*!< 368 [Active] tcpwm[0] Group #0, Counter #17 */
  tcpwm_0_interrupts_18_IRQn        = 369,      /*!< 369 [Active] tcpwm[0] Group #0, Counter #18 */
  tcpwm_0_interrupts_19_IRQn        = 370,      /*!< 370 [Active] tcpwm[0] Group #0, Counter #19 */
  tcpwm_0_interrupts_20_IRQn        = 371,      /*!< 371 [Active] tcpwm[0] Group #0, Counter #20 */
  tcpwm_0_interrupts_21_IRQn        = 372,      /*!< 372 [Active] tcpwm[0] Group #0, Counter #21 */
  tcpwm_0_interrupts_22_IRQn        = 373,      /*!< 373 [Active] tcpwm[0] Group #0, Counter #22 */
  tcpwm_0_interrupts_23_IRQn        = 374,      /*!< 374 [Active] tcpwm[0] Group #0, Counter #23 */
  tcpwm_0_interrupts_24_IRQn        = 375,      /*!< 375 [Active] tcpwm[0] Group #0, Counter #24 */
  tcpwm_0_interrupts_25_IRQn        = 376,      /*!< 376 [Active] tcpwm[0] Group #0, Counter #25 */
  tcpwm_0_interrupts_26_IRQn        = 377,      /*!< 377 [Active] tcpwm[0] Group #0, Counter #26 */
  tcpwm_0_interrupts_27_IRQn        = 378,      /*!< 378 [Active] tcpwm[0] Group #0, Counter #27 */
  tcpwm_0_interrupts_28_IRQn        = 379,      /*!< 379 [Active] tcpwm[0] Group #0, Counter #28 */
  tcpwm_0_interrupts_29_IRQn        = 380,      /*!< 380 [Active] tcpwm[0] Group #0, Counter #29 */
  tcpwm_0_interrupts_30_IRQn        = 381,      /*!< 381 [Active] tcpwm[0] Group #0, Counter #30 */
  tcpwm_0_interrupts_31_IRQn        = 382,      /*!< 382 [Active] tcpwm[0] Group #0, Counter #31 */
  tcpwm_0_interrupts_32_IRQn        = 383,      /*!< 383 [Active] tcpwm[0] Group #0, Counter #32 */
  tcpwm_0_interrupts_33_IRQn        = 384,      /*!< 384 [Active] tcpwm[0] Group #0, Counter #33 */
  tcpwm_0_interrupts_34_IRQn        = 385,      /*!< 385 [Active] tcpwm[0] Group #0, Counter #34 */
  tcpwm_0_interrupts_35_IRQn        = 386,      /*!< 386 [Active] tcpwm[0] Group #0, Counter #35 */
  tcpwm_0_interrupts_36_IRQn        = 387,      /*!< 387 [Active] tcpwm[0] Group #0, Counter #36 */
  tcpwm_0_interrupts_37_IRQn        = 388,      /*!< 388 [Active] tcpwm[0] Group #0, Counter #37 */
  tcpwm_0_interrupts_38_IRQn        = 389,      /*!< 389 [Active] tcpwm[0] Group #0, Counter #38 */
  tcpwm_0_interrupts_39_IRQn        = 390,      /*!< 390 [Active] tcpwm[0] Group #0, Counter #39 */
  tcpwm_0_interrupts_40_IRQn        = 391,      /*!< 391 [Active] tcpwm[0] Group #0, Counter #40 */
  tcpwm_0_interrupts_41_IRQn        = 392,      /*!< 392 [Active] tcpwm[0] Group #0, Counter #41 */
  tcpwm_0_interrupts_42_IRQn        = 393,      /*!< 393 [Active] tcpwm[0] Group #0, Counter #42 */
  tcpwm_0_interrupts_43_IRQn        = 394,      /*!< 394 [Active] tcpwm[0] Group #0, Counter #43 */
  tcpwm_0_interrupts_44_IRQn        = 395,      /*!< 395 [Active] tcpwm[0] Group #0, Counter #44 */
  tcpwm_0_interrupts_45_IRQn        = 396,      /*!< 396 [Active] tcpwm[0] Group #0, Counter #45 */
  tcpwm_0_interrupts_46_IRQn        = 397,      /*!< 397 [Active] tcpwm[0] Group #0, Counter #46 */
  tcpwm_0_interrupts_47_IRQn        = 398,      /*!< 398 [Active] tcpwm[0] Group #0, Counter #47 */
  tcpwm_0_interrupts_48_IRQn        = 399,      /*!< 399 [Active] tcpwm[0] Group #0, Counter #48 */
  tcpwm_0_interrupts_49_IRQn        = 400,      /*!< 400 [Active] tcpwm[0] Group #0, Counter #49 */
  tcpwm_0_interrupts_50_IRQn        = 401,      /*!< 401 [Active] tcpwm[0] Group #0, Counter #50 */
  tcpwm_0_interrupts_51_IRQn        = 402,      /*!< 402 [Active] tcpwm[0] Group #0, Counter #51 */
  tcpwm_0_interrupts_52_IRQn        = 403,      /*!< 403 [Active] tcpwm[0] Group #0, Counter #52 */
  tcpwm_0_interrupts_53_IRQn        = 404,      /*!< 404 [Active] tcpwm[0] Group #0, Counter #53 */
  tcpwm_0_interrupts_54_IRQn        = 405,      /*!< 405 [Active] tcpwm[0] Group #0, Counter #54 */
  tcpwm_0_interrupts_55_IRQn        = 406,      /*!< 406 [Active] tcpwm[0] Group #0, Counter #55 */
  tcpwm_0_interrupts_56_IRQn        = 407,      /*!< 407 [Active] tcpwm[0] Group #0, Counter #56 */
  tcpwm_0_interrupts_57_IRQn        = 408,      /*!< 408 [Active] tcpwm[0] Group #0, Counter #57 */
  tcpwm_0_interrupts_58_IRQn        = 409,      /*!< 409 [Active] tcpwm[0] Group #0, Counter #58 */
  tcpwm_0_interrupts_59_IRQn        = 410,      /*!< 410 [Active] tcpwm[0] Group #0, Counter #59 */
  tcpwm_0_interrupts_60_IRQn        = 411,      /*!< 411 [Active] tcpwm[0] Group #0, Counter #60 */
  tcpwm_0_interrupts_61_IRQn        = 412,      /*!< 412 [Active] tcpwm[0] Group #0, Counter #61 */
  tcpwm_0_interrupts_62_IRQn        = 413,      /*!< 413 [Active] tcpwm[0] Group #0, Counter #62 */
  tcpwm_0_interrupts_256_IRQn       = 414,      /*!< 414 [Active] tcpwm[0] Group #1, Counter #0 */
  tcpwm_0_interrupts_257_IRQn       = 415,      /*!< 415 [Active] tcpwm[0] Group #1, Counter #1 */
  tcpwm_0_interrupts_258_IRQn       = 416,      /*!< 416 [Active] tcpwm[0] Group #1, Counter #2 */
  tcpwm_0_interrupts_259_IRQn       = 417,      /*!< 417 [Active] tcpwm[0] Group #1, Counter #3 */
  tcpwm_0_interrupts_260_IRQn       = 418,      /*!< 418 [Active] tcpwm[0] Group #1, Counter #4 */
  tcpwm_0_interrupts_261_IRQn       = 419,      /*!< 419 [Active] tcpwm[0] Group #1, Counter #5 */
  tcpwm_0_interrupts_262_IRQn       = 420,      /*!< 420 [Active] tcpwm[0] Group #1, Counter #6 */
  tcpwm_0_interrupts_263_IRQn       = 421,      /*!< 421 [Active] tcpwm[0] Group #1, Counter #7 */
  tcpwm_0_interrupts_264_IRQn       = 422,      /*!< 422 [Active] tcpwm[0] Group #1, Counter #8 */
  tcpwm_0_interrupts_265_IRQn       = 423,      /*!< 423 [Active] tcpwm[0] Group #1, Counter #9 */
  tcpwm_0_interrupts_266_IRQn       = 424,      /*!< 424 [Active] tcpwm[0] Group #1, Counter #10 */
  tcpwm_0_interrupts_267_IRQn       = 425,      /*!< 425 [Active] tcpwm[0] Group #1, Counter #11 */
  tcpwm_0_interrupts_512_IRQn       = 426,      /*!< 426 [Active] tcpwm[0] Group #2, Counter #0 */
  tcpwm_0_interrupts_513_IRQn       = 427,      /*!< 427 [Active] tcpwm[0] Group #2, Counter #1 */
  tcpwm_0_interrupts_514_IRQn       = 428,      /*!< 428 [Active] tcpwm[0] Group #2, Counter #2 */
  tcpwm_0_interrupts_515_IRQn       = 429,      /*!< 429 [Active] tcpwm[0] Group #2, Counter #3 */
  tcpwm_0_interrupts_516_IRQn       = 430,      /*!< 430 [Active] tcpwm[0] Group #2, Counter #4 */
  tcpwm_0_interrupts_517_IRQn       = 431,      /*!< 431 [Active] tcpwm[0] Group #2, Counter #5 */
  tcpwm_0_interrupts_518_IRQn       = 432,      /*!< 432 [Active] tcpwm[0] Group #2, Counter #6 */
  tcpwm_0_interrupts_519_IRQn       = 433,      /*!< 433 [Active] tcpwm[0] Group #2, Counter #7 */
  smif_0_interrupt_IRQn             = 434,      /*!< 434 [Active] SMIF(QSPI) interrupt */
  eth_0_interrupt_eth_0_IRQn        = 435,      /*!< 435 [Active] Ethernet #0 priority queue[0] */
  eth_0_interrupt_eth_2_IRQn        = 436,      /*!< 436 [Active] Ethernet #0 priority queue[2] */
  eth_0_interrupt_eth_1_IRQn        = 437,      /*!< 437 [Active] Ethernet #0 priority queue[1] */
  sdhc_0_interrupt_general_IRQn     = 438,      /*!< 438 [Active] SDHC interupt */
  sdhc_0_interrupt_wakeup_IRQn      = 439,      /*!< 439 [Active] SDHC interupt */
  audioss_0_interrupt_i2s_IRQn      = 440,      /*!< 440 [Active] AUDIOSS I2S#0 interrupt */
  audioss_1_interrupt_i2s_IRQn      = 441,      /*!< 441 [Active] AUDIOSS I2S#1 interrupt */
  audioss_2_interrupt_i2s_IRQn      = 442,      /*!< 442 [Active] AUDIOSS I2S#2 interrupt */
  disconnected_IRQn                 =1023       /*!< 1023 Disconnected */
} cy_en_intr_t;

/*******************************************************************************
*                    Processor and Core Peripheral Section
*******************************************************************************/

#if ((defined(__GNUC__)        && (__ARM_ARCH == 6) && (__ARM_ARCH_6M__ == 1)) || \
     (defined(__ICCARM__)      && (__CORE__ == __ARM6M__)) || \
     (defined(__ARMCC_VERSION) && defined(__TARGET_ARCH_THUMB) && (__TARGET_ARCH_THUMB == 3)) || \
     (defined(__ghs__)         && defined(__CORE_CORTEXM0PLUS__)))

#include "cyip_coredefines.h"
#include "core_cm0plus.h"                       /*!< ARM Cortex-M0+ processor and core peripherals */
#else

#include "cyip_coredefines.h"
#include "core_cm7.h"                           /*!< ARM Cortex-M7 processor and core peripherals */
#endif

/* Memory Blocks */
#define CY_ROM_BASE                     0x00000000UL
#define CY_ROM_SIZE                     0x00010000UL
#define CY_ROM_MIRROR_BASE              0x01000000UL
#define CY_ROM_MIRROR_SIZE              0x00010000UL
#define CY_FLASH_LG_SBM_BASE            0x10000000UL
#define CY_FLASH_LG_SBM_SIZE            0x003F0000UL
#define CY_FLASH_LG_DBM0_BASE           0x10000000UL
#define CY_FLASH_LG_DBM0_SIZE           0x001F8000UL
#define CY_FLASH_SM_DBM0_BASE           0x101F8000UL
#define CY_FLASH_SM_DBM0_SIZE           0x00010000UL
#define CY_FLASH_SM_SBM_BASE            0x103F0000UL
#define CY_FLASH_SM_SBM_SIZE            0x00020000UL
#define CY_FLASH_LG_DBM1_BASE           0x12000000UL
#define CY_FLASH_LG_DBM1_SIZE           0x001F8000UL
#define CY_FLASH_SM_DBM1_BASE           0x121F8000UL
#define CY_FLASH_SM_DBM1_SIZE           0x00010000UL
#define CY_WFLASH_LG_SBM_BASE           0x14000000UL
#define CY_WFLASH_LG_SBM_SIZE           0x00030000UL
#define CY_WFLASH_LG_DBM0_BASE          0x14000000UL
#define CY_WFLASH_LG_DBM0_SIZE          0x00018000UL
#define CY_WFLASH_SM_DBM0_BASE          0x14018000UL
#define CY_WFLASH_SM_DBM0_SIZE          0x00008000UL
#define CY_WFLASH_SM_SBM_BASE           0x14030000UL
#define CY_WFLASH_SM_SBM_SIZE           0x00010000UL
#define CY_WFLASH_LG_DBM1_BASE          0x15000000UL
#define CY_WFLASH_LG_DBM1_SIZE          0x00018000UL
#define CY_WFLASH_SM_DBM1_BASE          0x15018000UL
#define CY_WFLASH_SM_DBM1_SIZE          0x00008000UL
#define CY_OTPFLASH_BASE                0x16000000UL
#define CY_OTPFLASH_SIZE                0x00008000UL
#define CY_SFLASH_BASE                  0x17000000UL
#define CY_SFLASH_SIZE                  0x00008000UL
#define CY_SFLASH1_BASE                 0x17800000UL
#define CY_SFLASH1_SIZE                 0x00008000UL
#define CY_EFUSE_BASE                   0x402C0800UL
#define CY_EFUSE_SIZE                   0x00000200UL
#define CY_CAN0MRAM_BASE                0x40530000UL
#define CY_CAN0MRAM_SIZE                0x00010000UL
#define CY_CAN1MRAM_BASE                0x40550000UL
#define CY_CAN1MRAM_SIZE                0x00010000UL
#define CY_SMIF_XIP_BASE                0x60000000UL
#define CY_SMIF_XIP_SIZE                0x08000000UL
#define CY_CM7_0_ITCM_BASE              0xA0000000UL
#define CY_CM7_0_ITCM_SIZE              0x00004000UL
#define CY_CM7_0_DTCM_BASE              0xA0010000UL
#define CY_CM7_0_DTCM_SIZE              0x00004000UL
#define CY_CM7_1_ITCM_BASE              0xA0100000UL
#define CY_CM7_1_ITCM_SIZE              0x00004000UL
#define CY_CM7_1_DTCM_BASE              0xA0110000UL
#define CY_CM7_1_DTCM_SIZE              0x00004000UL
#define CY_SRAM0_BASE                   0x28000000UL
#define CY_SRAM0_SIZE                   0x00080000UL
#define CY_SRAM1_BASE                   0x28080000UL
#define CY_SRAM1_SIZE                   0x00040000UL

#include "system_cyt4bb.h"                      /*!< CYT4BB System */

/* IP List */
#define CY_IP_MXAUDIOSS                 1u
#define CY_IP_MXAUDIOSS_INSTANCES       3u
#define CY_IP_MXAUDIOSS_VERSION         2u
#define CY_IP_MXTTCANFD                 1u
#define CY_IP_MXTTCANFD_INSTANCES       2u
#define CY_IP_MXTTCANFD_VERSION         1u
#define CY_IP_M7CPUSS                   1u
#define CY_IP_M7CPUSS_INSTANCES         1u
#define CY_IP_M7CPUSS_VERSION           1u
#define CY_IP_M7CPUSS_DMA               1u
#define CY_IP_M7CPUSS_DMA_INSTANCES     2u
#define CY_IP_M7CPUSS_DMA_VERSION       1u
#define CY_IP_MXCRYPTO                  1u
#define CY_IP_MXCRYPTO_INSTANCES        1u
#define CY_IP_MXCRYPTO_VERSION          1u
#define CY_IP_MXDFT                     1u
#define CY_IP_MXDFT_INSTANCES           1u
#define CY_IP_MXDFT_VERSION             1u
#define CY_IP_MXEFUSE                   1u
#define CY_IP_MXEFUSE_INSTANCES         1u
#define CY_IP_MXEFUSE_VERSION           2u
#define CY_IP_MXETH                     1u
#define CY_IP_MXETH_INSTANCES           1u
#define CY_IP_MXETH_VERSION             2u
#define CY_IP_MXEVTGEN                  1u
#define CY_IP_MXEVTGEN_INSTANCES        1u
#define CY_IP_MXEVTGEN_VERSION          1u
#define CY_IP_MXS40IOSS                 1u
#define CY_IP_MXS40IOSS_INSTANCES       1u
#define CY_IP_MXS40IOSS_VERSION         3u
#define CY_IP_MXLIN                     1u
#define CY_IP_MXLIN_INSTANCES           1u
#define CY_IP_MXLIN_VERSION             1u
#define CY_IP_MXS40EPASS                1u
#define CY_IP_MXS40EPASS_INSTANCES      1u
#define CY_IP_MXS40EPASS_VERSION        1u
#define CY_IP_MXS40EPASS_ESAR           1u
#define CY_IP_MXS40EPASS_ESAR_INSTANCES 3u
#define CY_IP_MXS40EPASS_ESAR_VERSION   1u
#define CY_IP_MXPERI                    1u
#define CY_IP_MXPERI_INSTANCES          1u
#define CY_IP_MXPERI_VERSION            3u
#define CY_IP_MXPERI_TR                 1u
#define CY_IP_MXPERI_TR_INSTANCES       1u
#define CY_IP_MXPERI_TR_VERSION         3u
#define CY_IP_MXSCB                     1u
#define CY_IP_MXSCB_INSTANCES           11u
#define CY_IP_MXSCB_VERSION             2u
#define CY_IP_MXSDHC                    1u
#define CY_IP_MXSDHC_INSTANCES          1u
#define CY_IP_MXSDHC_VERSION            1u
#define CY_IP_MXSMIF                    1u
#define CY_IP_MXSMIF_INSTANCES          1u
#define CY_IP_MXSMIF_VERSION            2u
#define CY_IP_MXS40SRSS                 1u
#define CY_IP_MXS40SRSS_INSTANCES       1u
#define CY_IP_MXS40SRSS_VERSION         3u
#define CY_IP_MXS40SRSS_RTC             1u
#define CY_IP_MXS40SRSS_RTC_INSTANCES   1u
#define CY_IP_MXS40SRSS_RTC_VERSION     3u
#define CY_IP_MXS40SRSS_MCWDT           1u
#define CY_IP_MXS40SRSS_MCWDT_INSTANCES 3u
#define CY_IP_MXS40SRSS_MCWDT_VERSION   3u
#define CY_IP_MXTCPWM                   1u
#define CY_IP_MXTCPWM_INSTANCES         1u
#define CY_IP_MXTCPWM_VERSION           2u

/* Include IP definitions */
#include "cyip_sflash_v2.h"
#include "cyip_peri_v3.h"
#include "cyip_peri_ms_v3.h"
#include "cyip_peri_pclk_v3.h"
#include "cyip_crypto.h"
#include "cyip_cpuss.h"
#include "cyip_fault.h"
#include "cyip_ipc.h"
#include "cyip_prot.h"
#include "cyip_flashc.h"
#include "cyip_srss_v3.h"
#include "cyip_backup_v3.h"
#include "cyip_dw.h"
#include "cyip_dmac.h"
#include "cyip_efuse_v2.h"
#include "cyip_efuse_data_v2.h"
#include "cyip_hsiom_v3.h"
#include "cyip_gpio_v3.h"
#include "cyip_smartio_v3.h"
#include "cyip_evtgen.h"
#include "cyip_smif_v2.h"
#include "cyip_sdhc.h"
#include "cyip_eth_v2.h"
#include "cyip_lin.h"
#include "cyip_canfd.h"
#include "cyip_tcpwm_v2.h"
#include "cyip_scb_v2.h"
#include "cyip_i2s_v2.h"
#include "cyip_pass.h"

#include "cyt4bb_config.h"
#include "gpio_cyt4bb_144_teqfp.h"

#define CY_DEVICE_TVIIBH4M
#define CY_SILICON_ID                   0x0000E91EUL
#define CY_HF_CLK_MAX_FREQ              250000000UL


/* Include register definitions */
#include "cyreg_sflash.h"
#include "cyreg_peri.h"
#include "cyreg_peri_ms.h"
#include "cyreg_peri_pclk.h"
#include "cyreg_crypto.h"
#include "cyreg_cpuss.h"
#include "cyreg_fault.h"
#include "cyreg_ipc.h"
#include "cyreg_prot.h"
#include "cyreg_flashc.h"
#include "cyreg_srss.h"
#include "cyreg_backup.h"
#include "cyreg_dw.h"
#include "cyreg_dmac.h"
#include "cyreg_efuse.h"
#include "cyreg_efuse_data.h"
#include "cyreg_hsiom.h"
#include "cyreg_gpio.h"
#include "cyreg_smartio.h"
#include "cyreg_evtgen.h"
#include "cyreg_smif.h"
#include "cyreg_sdhc.h"
#include "cyreg_eth.h"
#include "cyreg_lin.h"
#include "cyreg_canfd.h"
#include "cyreg_tcpwm.h"
#include "cyreg_scb.h"
#include "cyreg_i2s.h"
#include "cyreg_pass.h"

/*******************************************************************************
*                                    SFLASH
*******************************************************************************/

#define SFLASH_BASE                             0x17000000UL
#define SFLASH                                  ((volatile stc_SFLASH_t*) SFLASH_BASE)                            /* 0x17000000 */

/*******************************************************************************
*                                     PERI
*******************************************************************************/

#define PERI_BASE                               0x40000000UL
#define PERI                                    ((volatile stc_PERI_t*) PERI_BASE)                                /* 0x40000000 */
#define PERI_GR0                                ((volatile stc_PERI_GR_t*) &PERI->GR[0])                          /* 0x40004000 */
#define PERI_GR1                                ((volatile stc_PERI_GR_t*) &PERI->GR[1])                          /* 0x40004040 */
#define PERI_GR2                                ((volatile stc_PERI_GR_t*) &PERI->GR[2])                          /* 0x40004080 */
#define PERI_GR3                                ((volatile stc_PERI_GR_t*) &PERI->GR[3])                          /* 0x400040C0 */
#define PERI_GR4                                ((volatile stc_PERI_GR_t*) &PERI->GR[4])                          /* 0x40004100 */
#define PERI_GR5                                ((volatile stc_PERI_GR_t*) &PERI->GR[5])                          /* 0x40004140 */
#define PERI_GR6                                ((volatile stc_PERI_GR_t*) &PERI->GR[6])                          /* 0x40004180 */
#define PERI_GR8                                ((volatile stc_PERI_GR_t*) &PERI->GR[8])                          /* 0x40004200 */
#define PERI_GR9                                ((volatile stc_PERI_GR_t*) &PERI->GR[9])                          /* 0x40004240 */
#define PERI_TR_GR0                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[0])                    /* 0x40008000 */
#define PERI_TR_GR1                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[1])                    /* 0x40008400 */
#define PERI_TR_GR2                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[2])                    /* 0x40008800 */
#define PERI_TR_GR3                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[3])                    /* 0x40008C00 */
#define PERI_TR_GR4                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[4])                    /* 0x40009000 */
#define PERI_TR_GR5                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[5])                    /* 0x40009400 */
#define PERI_TR_GR6                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[6])                    /* 0x40009800 */
#define PERI_TR_GR7                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[7])                    /* 0x40009C00 */
#define PERI_TR_GR8                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[8])                    /* 0x4000A000 */
#define PERI_TR_GR9                             ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[9])                    /* 0x4000A400 */
#define PERI_TR_GR10                            ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[10])                   /* 0x4000A800 */
#define PERI_TR_GR11                            ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[11])                   /* 0x4000AC00 */
#define PERI_TR_GR12                            ((volatile stc_PERI_TR_GR_t*) &PERI->TR_GR[12])                   /* 0x4000B000 */
#define PERI_TR_1TO1_GR0                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[0])          /* 0x4000C000 */
#define PERI_TR_1TO1_GR1                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[1])          /* 0x4000C400 */
#define PERI_TR_1TO1_GR2                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[2])          /* 0x4000C800 */
#define PERI_TR_1TO1_GR3                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[3])          /* 0x4000CC00 */
#define PERI_TR_1TO1_GR4                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[4])          /* 0x4000D000 */
#define PERI_TR_1TO1_GR5                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[5])          /* 0x4000D400 */
#define PERI_TR_1TO1_GR6                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[6])          /* 0x4000D800 */
#define PERI_TR_1TO1_GR7                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[7])          /* 0x4000DC00 */
#define PERI_TR_1TO1_GR8                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[8])          /* 0x4000E000 */
#define PERI_TR_1TO1_GR9                        ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[9])          /* 0x4000E400 */
#define PERI_TR_1TO1_GR10                       ((volatile stc_PERI_TR_1TO1_GR_t*) &PERI->TR_1TO1_GR[10])         /* 0x4000E800 */

/*******************************************************************************
*                                   PERI_MS
*******************************************************************************/

#define PERI_MS_BASE                            0x40020000UL
#define PERI_MS                                 ((volatile stc_PERI_MS_t*) PERI_MS_BASE)                          /* 0x40020000 */
#define PERI_MS_PPU_PR0                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[0])            /* 0x40020000 */
#define PERI_MS_PPU_PR1                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[1])            /* 0x40020040 */
#define PERI_MS_PPU_PR2                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[2])            /* 0x40020080 */
#define PERI_MS_PPU_PR3                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[3])            /* 0x400200C0 */
#define PERI_MS_PPU_PR4                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[4])            /* 0x40020100 */
#define PERI_MS_PPU_PR5                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[5])            /* 0x40020140 */
#define PERI_MS_PPU_PR6                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[6])            /* 0x40020180 */
#define PERI_MS_PPU_PR7                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[7])            /* 0x400201C0 */
#define PERI_MS_PPU_PR8                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[8])            /* 0x40020200 */
#define PERI_MS_PPU_PR9                         ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[9])            /* 0x40020240 */
#define PERI_MS_PPU_PR10                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[10])           /* 0x40020280 */
#define PERI_MS_PPU_PR11                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[11])           /* 0x400202C0 */
#define PERI_MS_PPU_PR12                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[12])           /* 0x40020300 */
#define PERI_MS_PPU_PR13                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[13])           /* 0x40020340 */
#define PERI_MS_PPU_PR14                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[14])           /* 0x40020380 */
#define PERI_MS_PPU_PR15                        ((volatile stc_PERI_MS_PPU_PR_t*) &PERI_MS->PPU_PR[15])           /* 0x400203C0 */
#define PERI_MS_PPU_FX_PERI_MAIN                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[0])            /* 0x40020800 */
#define PERI_MS_PPU_FX_PERI_SECURE              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[1])            /* 0x40020840 */
#define PERI_MS_PPU_FX_PERI_GR0_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[2])            /* 0x40020880 */
#define PERI_MS_PPU_FX_PERI_GR1_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[3])            /* 0x400208C0 */
#define PERI_MS_PPU_FX_PERI_GR2_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[4])            /* 0x40020900 */
#define PERI_MS_PPU_FX_PERI_GR3_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[5])            /* 0x40020940 */
#define PERI_MS_PPU_FX_PERI_GR4_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[6])            /* 0x40020980 */
#define PERI_MS_PPU_FX_PERI_GR5_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[7])            /* 0x400209C0 */
#define PERI_MS_PPU_FX_PERI_GR6_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[8])            /* 0x40020A00 */
#define PERI_MS_PPU_FX_PERI_GR8_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[9])            /* 0x40020A40 */
#define PERI_MS_PPU_FX_PERI_GR9_GROUP           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[10])           /* 0x40020A80 */
#define PERI_MS_PPU_FX_PERI_GR0_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[11])           /* 0x40020AC0 */
#define PERI_MS_PPU_FX_PERI_GR1_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[12])           /* 0x40020B00 */
#define PERI_MS_PPU_FX_PERI_GR2_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[13])           /* 0x40020B40 */
#define PERI_MS_PPU_FX_PERI_GR3_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[14])           /* 0x40020B80 */
#define PERI_MS_PPU_FX_PERI_GR4_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[15])           /* 0x40020BC0 */
#define PERI_MS_PPU_FX_PERI_GR5_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[16])           /* 0x40020C00 */
#define PERI_MS_PPU_FX_PERI_GR6_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[17])           /* 0x40020C40 */
#define PERI_MS_PPU_FX_PERI_GR8_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[18])           /* 0x40020C80 */
#define PERI_MS_PPU_FX_PERI_GR9_BOOT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[19])           /* 0x40020CC0 */
#define PERI_MS_PPU_FX_PERI_TR                  ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[20])           /* 0x40020D00 */
#define PERI_MS_PPU_FX_PERI_MS_BOOT             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[21])           /* 0x40020D40 */
#define PERI_MS_PPU_FX_PERI_PCLK_MAIN           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[22])           /* 0x40020D80 */
#define PERI_MS_PPU_FX_CRYPTO_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[23])           /* 0x40020DC0 */
#define PERI_MS_PPU_FX_CRYPTO_CRYPTO            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[24])           /* 0x40020E00 */
#define PERI_MS_PPU_FX_CRYPTO_BOOT              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[25])           /* 0x40020E40 */
#define PERI_MS_PPU_FX_CRYPTO_KEY0              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[26])           /* 0x40020E80 */
#define PERI_MS_PPU_FX_CRYPTO_KEY1              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[27])           /* 0x40020EC0 */
#define PERI_MS_PPU_FX_CRYPTO_BUF               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[28])           /* 0x40020F00 */
#define PERI_MS_PPU_FX_CPUSS_CM7_0              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[29])           /* 0x40020F40 */
#define PERI_MS_PPU_FX_CPUSS_CM7_1              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[30])           /* 0x40020F80 */
#define PERI_MS_PPU_FX_CPUSS_CM0                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[31])           /* 0x40020FC0 */
#define PERI_MS_PPU_FX_CPUSS_BOOT               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[32])           /* 0x40021000 */
#define PERI_MS_PPU_FX_CPUSS_CM0_INT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[33])           /* 0x40021040 */
#define PERI_MS_PPU_FX_CPUSS_CM7_0_INT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[34])           /* 0x40021080 */
#define PERI_MS_PPU_FX_CPUSS_CM7_1_INT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[35])           /* 0x400210C0 */
#define PERI_MS_PPU_FX_FAULT_STRUCT0_MAIN       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[36])           /* 0x40021100 */
#define PERI_MS_PPU_FX_FAULT_STRUCT1_MAIN       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[37])           /* 0x40021140 */
#define PERI_MS_PPU_FX_FAULT_STRUCT2_MAIN       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[38])           /* 0x40021180 */
#define PERI_MS_PPU_FX_FAULT_STRUCT3_MAIN       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[39])           /* 0x400211C0 */
#define PERI_MS_PPU_FX_IPC_STRUCT0_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[40])           /* 0x40021200 */
#define PERI_MS_PPU_FX_IPC_STRUCT1_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[41])           /* 0x40021240 */
#define PERI_MS_PPU_FX_IPC_STRUCT2_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[42])           /* 0x40021280 */
#define PERI_MS_PPU_FX_IPC_STRUCT3_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[43])           /* 0x400212C0 */
#define PERI_MS_PPU_FX_IPC_STRUCT4_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[44])           /* 0x40021300 */
#define PERI_MS_PPU_FX_IPC_STRUCT5_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[45])           /* 0x40021340 */
#define PERI_MS_PPU_FX_IPC_STRUCT6_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[46])           /* 0x40021380 */
#define PERI_MS_PPU_FX_IPC_STRUCT7_IPC          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[47])           /* 0x400213C0 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT0_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[48])           /* 0x40021400 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT1_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[49])           /* 0x40021440 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT2_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[50])           /* 0x40021480 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT3_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[51])           /* 0x400214C0 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT4_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[52])           /* 0x40021500 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT5_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[53])           /* 0x40021540 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT6_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[54])           /* 0x40021580 */
#define PERI_MS_PPU_FX_IPC_INTR_STRUCT7_INTR    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[55])           /* 0x400215C0 */
#define PERI_MS_PPU_FX_PROT_SMPU_MAIN           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[56])           /* 0x40021600 */
#define PERI_MS_PPU_FX_PROT_MPU0_MAIN           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[57])           /* 0x40021640 */
#define PERI_MS_PPU_FX_PROT_MPU5_MAIN           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[58])           /* 0x40021680 */
#define PERI_MS_PPU_FX_PROT_MPU6_MAIN           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[59])           /* 0x400216C0 */
#define PERI_MS_PPU_FX_PROT_MPU13_MAIN          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[60])           /* 0x40021700 */
#define PERI_MS_PPU_FX_PROT_MPU14_MAIN          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[61])           /* 0x40021740 */
#define PERI_MS_PPU_FX_PROT_MPU15_MAIN          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[62])           /* 0x40021780 */
#define PERI_MS_PPU_FX_FLASHC_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[63])           /* 0x400217C0 */
#define PERI_MS_PPU_FX_FLASHC_CMD               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[64])           /* 0x40021800 */
#define PERI_MS_PPU_FX_FLASHC_DFT               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[65])           /* 0x40021840 */
#define PERI_MS_PPU_FX_FLASHC_CM0               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[66])           /* 0x40021880 */
#define PERI_MS_PPU_FX_FLASHC_CM7_0             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[67])           /* 0x400218C0 */
#define PERI_MS_PPU_FX_FLASHC_CM7_1             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[68])           /* 0x40021900 */
#define PERI_MS_PPU_FX_FLASHC_CRYPTO            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[69])           /* 0x40021940 */
#define PERI_MS_PPU_FX_FLASHC_DW0               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[70])           /* 0x40021980 */
#define PERI_MS_PPU_FX_FLASHC_DW1               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[71])           /* 0x400219C0 */
#define PERI_MS_PPU_FX_FLASHC_DMAC              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[72])           /* 0x40021A00 */
#define PERI_MS_PPU_FX_FLASHC_SLOW0             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[73])           /* 0x40021A40 */
#define PERI_MS_PPU_FX_FLASHC_SLOW1             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[74])           /* 0x40021A80 */
#define PERI_MS_PPU_FX_FLASHC_FlashMgmt         ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[75])           /* 0x40021AC0 */
#define PERI_MS_PPU_FX_FLASHC_MainSafety        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[76])           /* 0x40021B00 */
#define PERI_MS_PPU_FX_FLASHC_WorkSafety        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[77])           /* 0x40021B40 */
#define PERI_MS_PPU_FX_SRSS_GENERAL             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[78])           /* 0x40021B80 */
#define PERI_MS_PPU_FX_SRSS_MAIN                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[79])           /* 0x40021BC0 */
#define PERI_MS_PPU_FX_SRSS_SECURE              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[80])           /* 0x40021C00 */
#define PERI_MS_PPU_FX_MCWDT0_CONFIG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[81])           /* 0x40021C40 */
#define PERI_MS_PPU_FX_MCWDT1_CONFIG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[82])           /* 0x40021C80 */
#define PERI_MS_PPU_FX_MCWDT2_CONFIG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[83])           /* 0x40021CC0 */
#define PERI_MS_PPU_FX_MCWDT0_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[84])           /* 0x40021D00 */
#define PERI_MS_PPU_FX_MCWDT1_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[85])           /* 0x40021D40 */
#define PERI_MS_PPU_FX_MCWDT2_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[86])           /* 0x40021D80 */
#define PERI_MS_PPU_FX_WDT_CONFIG               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[87])           /* 0x40021DC0 */
#define PERI_MS_PPU_FX_WDT_MAIN                 ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[88])           /* 0x40021E00 */
#define PERI_MS_PPU_FX_BACKUP_BACKUP            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[89])           /* 0x40021E40 */
#define PERI_MS_PPU_FX_DW0_DW                   ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[90])           /* 0x40021E80 */
#define PERI_MS_PPU_FX_DW1_DW                   ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[91])           /* 0x40021EC0 */
#define PERI_MS_PPU_FX_DW0_DW_CRC               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[92])           /* 0x40021F00 */
#define PERI_MS_PPU_FX_DW1_DW_CRC               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[93])           /* 0x40021F40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT0_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[94])           /* 0x40021F80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT1_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[95])           /* 0x40021FC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT2_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[96])           /* 0x40022000 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT3_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[97])           /* 0x40022040 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT4_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[98])           /* 0x40022080 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT5_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[99])           /* 0x400220C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT6_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[100])          /* 0x40022100 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT7_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[101])          /* 0x40022140 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT8_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[102])          /* 0x40022180 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT9_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[103])          /* 0x400221C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT10_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[104])          /* 0x40022200 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT11_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[105])          /* 0x40022240 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT12_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[106])          /* 0x40022280 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT13_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[107])          /* 0x400222C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT14_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[108])          /* 0x40022300 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT15_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[109])          /* 0x40022340 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT16_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[110])          /* 0x40022380 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT17_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[111])          /* 0x400223C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT18_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[112])          /* 0x40022400 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT19_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[113])          /* 0x40022440 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT20_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[114])          /* 0x40022480 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT21_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[115])          /* 0x400224C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT22_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[116])          /* 0x40022500 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT23_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[117])          /* 0x40022540 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT24_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[118])          /* 0x40022580 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT25_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[119])          /* 0x400225C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT26_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[120])          /* 0x40022600 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT27_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[121])          /* 0x40022640 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT28_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[122])          /* 0x40022680 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT29_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[123])          /* 0x400226C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT30_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[124])          /* 0x40022700 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT31_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[125])          /* 0x40022740 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT32_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[126])          /* 0x40022780 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT33_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[127])          /* 0x400227C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT34_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[128])          /* 0x40022800 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT35_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[129])          /* 0x40022840 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT36_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[130])          /* 0x40022880 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT37_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[131])          /* 0x400228C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT38_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[132])          /* 0x40022900 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT39_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[133])          /* 0x40022940 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT40_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[134])          /* 0x40022980 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT41_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[135])          /* 0x400229C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT42_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[136])          /* 0x40022A00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT43_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[137])          /* 0x40022A40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT44_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[138])          /* 0x40022A80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT45_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[139])          /* 0x40022AC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT46_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[140])          /* 0x40022B00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT47_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[141])          /* 0x40022B40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT48_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[142])          /* 0x40022B80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT49_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[143])          /* 0x40022BC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT50_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[144])          /* 0x40022C00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT51_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[145])          /* 0x40022C40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT52_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[146])          /* 0x40022C80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT53_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[147])          /* 0x40022CC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT54_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[148])          /* 0x40022D00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT55_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[149])          /* 0x40022D40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT56_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[150])          /* 0x40022D80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT57_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[151])          /* 0x40022DC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT58_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[152])          /* 0x40022E00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT59_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[153])          /* 0x40022E40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT60_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[154])          /* 0x40022E80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT61_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[155])          /* 0x40022EC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT62_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[156])          /* 0x40022F00 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT63_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[157])          /* 0x40022F40 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT64_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[158])          /* 0x40022F80 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT65_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[159])          /* 0x40022FC0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT66_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[160])          /* 0x40023000 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT67_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[161])          /* 0x40023040 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT68_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[162])          /* 0x40023080 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT69_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[163])          /* 0x400230C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT70_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[164])          /* 0x40023100 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT71_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[165])          /* 0x40023140 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT72_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[166])          /* 0x40023180 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT73_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[167])          /* 0x400231C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT74_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[168])          /* 0x40023200 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT75_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[169])          /* 0x40023240 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT76_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[170])          /* 0x40023280 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT77_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[171])          /* 0x400232C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT78_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[172])          /* 0x40023300 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT79_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[173])          /* 0x40023340 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT80_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[174])          /* 0x40023380 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT81_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[175])          /* 0x400233C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT82_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[176])          /* 0x40023400 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT83_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[177])          /* 0x40023440 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT84_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[178])          /* 0x40023480 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT85_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[179])          /* 0x400234C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT86_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[180])          /* 0x40023500 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT87_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[181])          /* 0x40023540 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT88_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[182])          /* 0x40023580 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT89_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[183])          /* 0x400235C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT90_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[184])          /* 0x40023600 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT91_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[185])          /* 0x40023640 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT92_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[186])          /* 0x40023680 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT93_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[187])          /* 0x400236C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT94_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[188])          /* 0x40023700 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT95_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[189])          /* 0x40023740 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT96_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[190])          /* 0x40023780 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT97_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[191])          /* 0x400237C0 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT98_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[192])          /* 0x40023800 */
#define PERI_MS_PPU_FX_DW0_CH_STRUCT99_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[193])          /* 0x40023840 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT0_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[194])          /* 0x40023880 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT1_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[195])          /* 0x400238C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT2_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[196])          /* 0x40023900 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT3_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[197])          /* 0x40023940 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT4_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[198])          /* 0x40023980 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT5_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[199])          /* 0x400239C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT6_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[200])          /* 0x40023A00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT7_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[201])          /* 0x40023A40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT8_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[202])          /* 0x40023A80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT9_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[203])          /* 0x40023AC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT10_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[204])          /* 0x40023B00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT11_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[205])          /* 0x40023B40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT12_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[206])          /* 0x40023B80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT13_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[207])          /* 0x40023BC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT14_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[208])          /* 0x40023C00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT15_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[209])          /* 0x40023C40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT16_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[210])          /* 0x40023C80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT17_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[211])          /* 0x40023CC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT18_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[212])          /* 0x40023D00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT19_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[213])          /* 0x40023D40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT20_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[214])          /* 0x40023D80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT21_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[215])          /* 0x40023DC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT22_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[216])          /* 0x40023E00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT23_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[217])          /* 0x40023E40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT24_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[218])          /* 0x40023E80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT25_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[219])          /* 0x40023EC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT26_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[220])          /* 0x40023F00 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT27_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[221])          /* 0x40023F40 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT28_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[222])          /* 0x40023F80 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT29_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[223])          /* 0x40023FC0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT30_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[224])          /* 0x40024000 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT31_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[225])          /* 0x40024040 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT32_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[226])          /* 0x40024080 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT33_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[227])          /* 0x400240C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT34_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[228])          /* 0x40024100 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT35_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[229])          /* 0x40024140 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT36_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[230])          /* 0x40024180 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT37_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[231])          /* 0x400241C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT38_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[232])          /* 0x40024200 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT39_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[233])          /* 0x40024240 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT40_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[234])          /* 0x40024280 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT41_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[235])          /* 0x400242C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT42_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[236])          /* 0x40024300 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT43_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[237])          /* 0x40024340 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT44_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[238])          /* 0x40024380 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT45_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[239])          /* 0x400243C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT46_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[240])          /* 0x40024400 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT47_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[241])          /* 0x40024440 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT48_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[242])          /* 0x40024480 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT49_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[243])          /* 0x400244C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT50_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[244])          /* 0x40024500 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT51_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[245])          /* 0x40024540 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT52_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[246])          /* 0x40024580 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT53_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[247])          /* 0x400245C0 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT54_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[248])          /* 0x40024600 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT55_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[249])          /* 0x40024640 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT56_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[250])          /* 0x40024680 */
#define PERI_MS_PPU_FX_DW1_CH_STRUCT57_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[251])          /* 0x400246C0 */
#define PERI_MS_PPU_FX_DMAC_TOP                 ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[252])          /* 0x40024700 */
#define PERI_MS_PPU_FX_DMAC_CH0_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[253])          /* 0x40024740 */
#define PERI_MS_PPU_FX_DMAC_CH1_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[254])          /* 0x40024780 */
#define PERI_MS_PPU_FX_DMAC_CH2_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[255])          /* 0x400247C0 */
#define PERI_MS_PPU_FX_DMAC_CH3_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[256])          /* 0x40024800 */
#define PERI_MS_PPU_FX_DMAC_CH4_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[257])          /* 0x40024840 */
#define PERI_MS_PPU_FX_DMAC_CH5_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[258])          /* 0x40024880 */
#define PERI_MS_PPU_FX_DMAC_CH6_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[259])          /* 0x400248C0 */
#define PERI_MS_PPU_FX_DMAC_CH7_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[260])          /* 0x40024900 */
#define PERI_MS_PPU_FX_EFUSE_CTL                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[261])          /* 0x40024940 */
#define PERI_MS_PPU_FX_EFUSE_DATA               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[262])          /* 0x40024980 */
#define PERI_MS_PPU_FX_BIST                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[263])          /* 0x400249C0 */
#define PERI_MS_PPU_FX_HSIOM_PRT0_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[264])          /* 0x40024A00 */
#define PERI_MS_PPU_FX_HSIOM_PRT1_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[265])          /* 0x40024A40 */
#define PERI_MS_PPU_FX_HSIOM_PRT2_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[266])          /* 0x40024A80 */
#define PERI_MS_PPU_FX_HSIOM_PRT3_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[267])          /* 0x40024AC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT4_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[268])          /* 0x40024B00 */
#define PERI_MS_PPU_FX_HSIOM_PRT5_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[269])          /* 0x40024B40 */
#define PERI_MS_PPU_FX_HSIOM_PRT6_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[270])          /* 0x40024B80 */
#define PERI_MS_PPU_FX_HSIOM_PRT7_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[271])          /* 0x40024BC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT8_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[272])          /* 0x40024C00 */
#define PERI_MS_PPU_FX_HSIOM_PRT9_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[273])          /* 0x40024C40 */
#define PERI_MS_PPU_FX_HSIOM_PRT10_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[274])          /* 0x40024C80 */
#define PERI_MS_PPU_FX_HSIOM_PRT11_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[275])          /* 0x40024CC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT12_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[276])          /* 0x40024D00 */
#define PERI_MS_PPU_FX_HSIOM_PRT13_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[277])          /* 0x40024D40 */
#define PERI_MS_PPU_FX_HSIOM_PRT14_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[278])          /* 0x40024D80 */
#define PERI_MS_PPU_FX_HSIOM_PRT15_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[279])          /* 0x40024DC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT16_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[280])          /* 0x40024E00 */
#define PERI_MS_PPU_FX_HSIOM_PRT17_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[281])          /* 0x40024E40 */
#define PERI_MS_PPU_FX_HSIOM_PRT18_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[282])          /* 0x40024E80 */
#define PERI_MS_PPU_FX_HSIOM_PRT19_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[283])          /* 0x40024EC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT20_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[284])          /* 0x40024F00 */
#define PERI_MS_PPU_FX_HSIOM_PRT21_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[285])          /* 0x40024F40 */
#define PERI_MS_PPU_FX_HSIOM_PRT22_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[286])          /* 0x40024F80 */
#define PERI_MS_PPU_FX_HSIOM_PRT23_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[287])          /* 0x40024FC0 */
#define PERI_MS_PPU_FX_HSIOM_PRT24_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[288])          /* 0x40025000 */
#define PERI_MS_PPU_FX_HSIOM_PRT25_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[289])          /* 0x40025040 */
#define PERI_MS_PPU_FX_HSIOM_PRT26_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[290])          /* 0x40025080 */
#define PERI_MS_PPU_FX_HSIOM_PRT27_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[291])          /* 0x400250C0 */
#define PERI_MS_PPU_FX_HSIOM_PRT28_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[292])          /* 0x40025100 */
#define PERI_MS_PPU_FX_HSIOM_PRT29_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[293])          /* 0x40025140 */
#define PERI_MS_PPU_FX_HSIOM_PRT30_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[294])          /* 0x40025180 */
#define PERI_MS_PPU_FX_HSIOM_PRT31_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[295])          /* 0x400251C0 */
#define PERI_MS_PPU_FX_HSIOM_PRT32_PRT          ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[296])          /* 0x40025200 */
#define PERI_MS_PPU_FX_HSIOM_AMUX               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[297])          /* 0x40025240 */
#define PERI_MS_PPU_FX_HSIOM_MON                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[298])          /* 0x40025280 */
#define PERI_MS_PPU_FX_HSIOM_ALTJTAG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[299])          /* 0x400252C0 */
#define PERI_MS_PPU_FX_GPIO_PRT0_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[300])          /* 0x40025300 */
#define PERI_MS_PPU_FX_GPIO_PRT1_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[301])          /* 0x40025340 */
#define PERI_MS_PPU_FX_GPIO_PRT2_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[302])          /* 0x40025380 */
#define PERI_MS_PPU_FX_GPIO_PRT3_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[303])          /* 0x400253C0 */
#define PERI_MS_PPU_FX_GPIO_PRT4_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[304])          /* 0x40025400 */
#define PERI_MS_PPU_FX_GPIO_PRT5_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[305])          /* 0x40025440 */
#define PERI_MS_PPU_FX_GPIO_PRT6_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[306])          /* 0x40025480 */
#define PERI_MS_PPU_FX_GPIO_PRT7_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[307])          /* 0x400254C0 */
#define PERI_MS_PPU_FX_GPIO_PRT8_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[308])          /* 0x40025500 */
#define PERI_MS_PPU_FX_GPIO_PRT9_PRT            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[309])          /* 0x40025540 */
#define PERI_MS_PPU_FX_GPIO_PRT10_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[310])          /* 0x40025580 */
#define PERI_MS_PPU_FX_GPIO_PRT11_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[311])          /* 0x400255C0 */
#define PERI_MS_PPU_FX_GPIO_PRT12_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[312])          /* 0x40025600 */
#define PERI_MS_PPU_FX_GPIO_PRT13_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[313])          /* 0x40025640 */
#define PERI_MS_PPU_FX_GPIO_PRT14_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[314])          /* 0x40025680 */
#define PERI_MS_PPU_FX_GPIO_PRT15_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[315])          /* 0x400256C0 */
#define PERI_MS_PPU_FX_GPIO_PRT16_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[316])          /* 0x40025700 */
#define PERI_MS_PPU_FX_GPIO_PRT17_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[317])          /* 0x40025740 */
#define PERI_MS_PPU_FX_GPIO_PRT18_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[318])          /* 0x40025780 */
#define PERI_MS_PPU_FX_GPIO_PRT19_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[319])          /* 0x400257C0 */
#define PERI_MS_PPU_FX_GPIO_PRT20_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[320])          /* 0x40025800 */
#define PERI_MS_PPU_FX_GPIO_PRT21_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[321])          /* 0x40025840 */
#define PERI_MS_PPU_FX_GPIO_PRT22_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[322])          /* 0x40025880 */
#define PERI_MS_PPU_FX_GPIO_PRT23_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[323])          /* 0x400258C0 */
#define PERI_MS_PPU_FX_GPIO_PRT24_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[324])          /* 0x40025900 */
#define PERI_MS_PPU_FX_GPIO_PRT25_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[325])          /* 0x40025940 */
#define PERI_MS_PPU_FX_GPIO_PRT26_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[326])          /* 0x40025980 */
#define PERI_MS_PPU_FX_GPIO_PRT27_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[327])          /* 0x400259C0 */
#define PERI_MS_PPU_FX_GPIO_PRT28_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[328])          /* 0x40025A00 */
#define PERI_MS_PPU_FX_GPIO_PRT29_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[329])          /* 0x40025A40 */
#define PERI_MS_PPU_FX_GPIO_PRT30_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[330])          /* 0x40025A80 */
#define PERI_MS_PPU_FX_GPIO_PRT31_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[331])          /* 0x40025AC0 */
#define PERI_MS_PPU_FX_GPIO_PRT32_PRT           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[332])          /* 0x40025B00 */
#define PERI_MS_PPU_FX_GPIO_PRT0_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[333])          /* 0x40025B40 */
#define PERI_MS_PPU_FX_GPIO_PRT1_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[334])          /* 0x40025B80 */
#define PERI_MS_PPU_FX_GPIO_PRT2_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[335])          /* 0x40025BC0 */
#define PERI_MS_PPU_FX_GPIO_PRT3_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[336])          /* 0x40025C00 */
#define PERI_MS_PPU_FX_GPIO_PRT4_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[337])          /* 0x40025C40 */
#define PERI_MS_PPU_FX_GPIO_PRT5_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[338])          /* 0x40025C80 */
#define PERI_MS_PPU_FX_GPIO_PRT6_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[339])          /* 0x40025CC0 */
#define PERI_MS_PPU_FX_GPIO_PRT7_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[340])          /* 0x40025D00 */
#define PERI_MS_PPU_FX_GPIO_PRT8_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[341])          /* 0x40025D40 */
#define PERI_MS_PPU_FX_GPIO_PRT9_CFG            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[342])          /* 0x40025D80 */
#define PERI_MS_PPU_FX_GPIO_PRT10_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[343])          /* 0x40025DC0 */
#define PERI_MS_PPU_FX_GPIO_PRT11_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[344])          /* 0x40025E00 */
#define PERI_MS_PPU_FX_GPIO_PRT12_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[345])          /* 0x40025E40 */
#define PERI_MS_PPU_FX_GPIO_PRT13_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[346])          /* 0x40025E80 */
#define PERI_MS_PPU_FX_GPIO_PRT14_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[347])          /* 0x40025EC0 */
#define PERI_MS_PPU_FX_GPIO_PRT15_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[348])          /* 0x40025F00 */
#define PERI_MS_PPU_FX_GPIO_PRT16_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[349])          /* 0x40025F40 */
#define PERI_MS_PPU_FX_GPIO_PRT17_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[350])          /* 0x40025F80 */
#define PERI_MS_PPU_FX_GPIO_PRT18_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[351])          /* 0x40025FC0 */
#define PERI_MS_PPU_FX_GPIO_PRT19_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[352])          /* 0x40026000 */
#define PERI_MS_PPU_FX_GPIO_PRT20_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[353])          /* 0x40026040 */
#define PERI_MS_PPU_FX_GPIO_PRT21_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[354])          /* 0x40026080 */
#define PERI_MS_PPU_FX_GPIO_PRT22_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[355])          /* 0x400260C0 */
#define PERI_MS_PPU_FX_GPIO_PRT23_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[356])          /* 0x40026100 */
#define PERI_MS_PPU_FX_GPIO_PRT24_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[357])          /* 0x40026140 */
#define PERI_MS_PPU_FX_GPIO_PRT25_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[358])          /* 0x40026180 */
#define PERI_MS_PPU_FX_GPIO_PRT26_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[359])          /* 0x400261C0 */
#define PERI_MS_PPU_FX_GPIO_PRT27_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[360])          /* 0x40026200 */
#define PERI_MS_PPU_FX_GPIO_PRT28_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[361])          /* 0x40026240 */
#define PERI_MS_PPU_FX_GPIO_PRT29_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[362])          /* 0x40026280 */
#define PERI_MS_PPU_FX_GPIO_PRT30_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[363])          /* 0x400262C0 */
#define PERI_MS_PPU_FX_GPIO_PRT31_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[364])          /* 0x40026300 */
#define PERI_MS_PPU_FX_GPIO_PRT32_CFG           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[365])          /* 0x40026340 */
#define PERI_MS_PPU_FX_GPIO_GPIO                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[366])          /* 0x40026380 */
#define PERI_MS_PPU_FX_GPIO_TEST                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[367])          /* 0x400263C0 */
#define PERI_MS_PPU_FX_SMARTIO_PRT12_PRT        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[368])          /* 0x40026400 */
#define PERI_MS_PPU_FX_SMARTIO_PRT13_PRT        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[369])          /* 0x40026440 */
#define PERI_MS_PPU_FX_SMARTIO_PRT14_PRT        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[370])          /* 0x40026480 */
#define PERI_MS_PPU_FX_SMARTIO_PRT15_PRT        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[371])          /* 0x400264C0 */
#define PERI_MS_PPU_FX_SMARTIO_PRT17_PRT        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[372])          /* 0x40026500 */
#define PERI_MS_PPU_FX_EVTGEN0                  ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[373])          /* 0x40026540 */
#define PERI_MS_PPU_FX_SMIF0                    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[374])          /* 0x40026580 */
#define PERI_MS_PPU_FX_SDHC0                    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[375])          /* 0x400265C0 */
#define PERI_MS_PPU_FX_ETH0                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[376])          /* 0x40026600 */
#define PERI_MS_PPU_FX_LIN0_MAIN                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[377])          /* 0x40026640 */
#define PERI_MS_PPU_FX_LIN0_CH0_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[378])          /* 0x40026680 */
#define PERI_MS_PPU_FX_LIN0_CH1_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[379])          /* 0x400266C0 */
#define PERI_MS_PPU_FX_LIN0_CH2_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[380])          /* 0x40026700 */
#define PERI_MS_PPU_FX_LIN0_CH3_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[381])          /* 0x40026740 */
#define PERI_MS_PPU_FX_LIN0_CH4_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[382])          /* 0x40026780 */
#define PERI_MS_PPU_FX_LIN0_CH5_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[383])          /* 0x400267C0 */
#define PERI_MS_PPU_FX_LIN0_CH6_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[384])          /* 0x40026800 */
#define PERI_MS_PPU_FX_LIN0_CH7_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[385])          /* 0x40026840 */
#define PERI_MS_PPU_FX_LIN0_CH8_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[386])          /* 0x40026880 */
#define PERI_MS_PPU_FX_LIN0_CH9_CH              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[387])          /* 0x400268C0 */
#define PERI_MS_PPU_FX_LIN0_CH10_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[388])          /* 0x40026900 */
#define PERI_MS_PPU_FX_LIN0_CH11_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[389])          /* 0x40026940 */
#define PERI_MS_PPU_FX_LIN0_CH12_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[390])          /* 0x40026980 */
#define PERI_MS_PPU_FX_LIN0_CH13_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[391])          /* 0x400269C0 */
#define PERI_MS_PPU_FX_LIN0_CH14_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[392])          /* 0x40026A00 */
#define PERI_MS_PPU_FX_LIN0_CH15_CH             ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[393])          /* 0x40026A40 */
#define PERI_MS_PPU_FX_CANFD0_CH0_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[394])          /* 0x40026A80 */
#define PERI_MS_PPU_FX_CANFD0_CH1_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[395])          /* 0x40026AC0 */
#define PERI_MS_PPU_FX_CANFD0_CH2_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[396])          /* 0x40026B00 */
#define PERI_MS_PPU_FX_CANFD0_CH3_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[397])          /* 0x40026B40 */
#define PERI_MS_PPU_FX_CANFD1_CH0_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[398])          /* 0x40026B80 */
#define PERI_MS_PPU_FX_CANFD1_CH1_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[399])          /* 0x40026BC0 */
#define PERI_MS_PPU_FX_CANFD1_CH2_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[400])          /* 0x40026C00 */
#define PERI_MS_PPU_FX_CANFD1_CH3_CH            ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[401])          /* 0x40026C40 */
#define PERI_MS_PPU_FX_CANFD0_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[402])          /* 0x40026C80 */
#define PERI_MS_PPU_FX_CANFD1_MAIN              ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[403])          /* 0x40026CC0 */
#define PERI_MS_PPU_FX_CANFD0_BUF               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[404])          /* 0x40026D00 */
#define PERI_MS_PPU_FX_CANFD1_BUF               ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[405])          /* 0x40026D40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT0_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[406])          /* 0x40026D80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT1_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[407])          /* 0x40026DC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT2_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[408])          /* 0x40026E00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT3_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[409])          /* 0x40026E40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT4_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[410])          /* 0x40026E80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT5_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[411])          /* 0x40026EC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT6_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[412])          /* 0x40026F00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT7_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[413])          /* 0x40026F40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT8_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[414])          /* 0x40026F80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT9_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[415])          /* 0x40026FC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT10_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[416])          /* 0x40027000 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT11_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[417])          /* 0x40027040 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT12_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[418])          /* 0x40027080 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT13_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[419])          /* 0x400270C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT14_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[420])          /* 0x40027100 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT15_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[421])          /* 0x40027140 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT16_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[422])          /* 0x40027180 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT17_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[423])          /* 0x400271C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT18_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[424])          /* 0x40027200 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT19_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[425])          /* 0x40027240 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT20_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[426])          /* 0x40027280 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT21_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[427])          /* 0x400272C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT22_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[428])          /* 0x40027300 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT23_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[429])          /* 0x40027340 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT24_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[430])          /* 0x40027380 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT25_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[431])          /* 0x400273C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT26_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[432])          /* 0x40027400 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT27_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[433])          /* 0x40027440 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT28_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[434])          /* 0x40027480 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT29_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[435])          /* 0x400274C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT30_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[436])          /* 0x40027500 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT31_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[437])          /* 0x40027540 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT32_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[438])          /* 0x40027580 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT33_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[439])          /* 0x400275C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT34_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[440])          /* 0x40027600 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT35_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[441])          /* 0x40027640 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT36_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[442])          /* 0x40027680 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT37_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[443])          /* 0x400276C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT38_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[444])          /* 0x40027700 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT39_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[445])          /* 0x40027740 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT40_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[446])          /* 0x40027780 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT41_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[447])          /* 0x400277C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT42_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[448])          /* 0x40027800 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT43_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[449])          /* 0x40027840 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT44_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[450])          /* 0x40027880 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT45_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[451])          /* 0x400278C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT46_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[452])          /* 0x40027900 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT47_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[453])          /* 0x40027940 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT48_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[454])          /* 0x40027980 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT49_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[455])          /* 0x400279C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT50_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[456])          /* 0x40027A00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT51_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[457])          /* 0x40027A40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT52_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[458])          /* 0x40027A80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT53_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[459])          /* 0x40027AC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT54_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[460])          /* 0x40027B00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT55_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[461])          /* 0x40027B40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT56_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[462])          /* 0x40027B80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT57_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[463])          /* 0x40027BC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT58_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[464])          /* 0x40027C00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT59_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[465])          /* 0x40027C40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT60_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[466])          /* 0x40027C80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT61_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[467])          /* 0x40027CC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP0_CNT62_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[468])          /* 0x40027D00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT0_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[469])          /* 0x40027D40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT1_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[470])          /* 0x40027D80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT2_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[471])          /* 0x40027DC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT3_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[472])          /* 0x40027E00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT4_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[473])          /* 0x40027E40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT5_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[474])          /* 0x40027E80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT6_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[475])          /* 0x40027EC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT7_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[476])          /* 0x40027F00 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT8_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[477])          /* 0x40027F40 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT9_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[478])          /* 0x40027F80 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT10_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[479])          /* 0x40027FC0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP1_CNT11_CNT    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[480])          /* 0x40028000 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT0_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[481])          /* 0x40028040 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT1_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[482])          /* 0x40028080 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT2_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[483])          /* 0x400280C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT3_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[484])          /* 0x40028100 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT4_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[485])          /* 0x40028140 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT5_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[486])          /* 0x40028180 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT6_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[487])          /* 0x400281C0 */
#define PERI_MS_PPU_FX_TCPWM0_GRP2_CNT7_CNT     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[488])          /* 0x40028200 */
#define PERI_MS_PPU_FX_SCB0                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[489])          /* 0x40028240 */
#define PERI_MS_PPU_FX_SCB1                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[490])          /* 0x40028280 */
#define PERI_MS_PPU_FX_SCB2                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[491])          /* 0x400282C0 */
#define PERI_MS_PPU_FX_SCB3                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[492])          /* 0x40028300 */
#define PERI_MS_PPU_FX_SCB4                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[493])          /* 0x40028340 */
#define PERI_MS_PPU_FX_SCB5                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[494])          /* 0x40028380 */
#define PERI_MS_PPU_FX_SCB6                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[495])          /* 0x400283C0 */
#define PERI_MS_PPU_FX_SCB7                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[496])          /* 0x40028400 */
#define PERI_MS_PPU_FX_SCB8                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[497])          /* 0x40028440 */
#define PERI_MS_PPU_FX_SCB9                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[498])          /* 0x40028480 */
#define PERI_MS_PPU_FX_SCB10                    ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[499])          /* 0x400284C0 */
#define PERI_MS_PPU_FX_I2S0                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[500])          /* 0x40028500 */
#define PERI_MS_PPU_FX_I2S1                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[501])          /* 0x40028540 */
#define PERI_MS_PPU_FX_I2S2                     ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[502])          /* 0x40028580 */
#define PERI_MS_PPU_FX_PASS0_SAR0_SAR           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[503])          /* 0x400285C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_SAR           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[504])          /* 0x40028600 */
#define PERI_MS_PPU_FX_PASS0_SAR2_SAR           ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[505])          /* 0x40028640 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH0_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[506])          /* 0x40028680 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH1_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[507])          /* 0x400286C0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH2_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[508])          /* 0x40028700 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH3_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[509])          /* 0x40028740 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH4_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[510])          /* 0x40028780 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH5_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[511])          /* 0x400287C0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH6_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[512])          /* 0x40028800 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH7_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[513])          /* 0x40028840 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH8_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[514])          /* 0x40028880 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH9_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[515])          /* 0x400288C0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH10_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[516])          /* 0x40028900 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH11_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[517])          /* 0x40028940 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH12_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[518])          /* 0x40028980 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH13_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[519])          /* 0x400289C0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH14_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[520])          /* 0x40028A00 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH15_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[521])          /* 0x40028A40 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH16_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[522])          /* 0x40028A80 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH17_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[523])          /* 0x40028AC0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH18_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[524])          /* 0x40028B00 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH19_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[525])          /* 0x40028B40 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH20_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[526])          /* 0x40028B80 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH21_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[527])          /* 0x40028BC0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH22_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[528])          /* 0x40028C00 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH23_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[529])          /* 0x40028C40 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH24_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[530])          /* 0x40028C80 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH25_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[531])          /* 0x40028CC0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH26_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[532])          /* 0x40028D00 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH27_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[533])          /* 0x40028D40 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH28_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[534])          /* 0x40028D80 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH29_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[535])          /* 0x40028DC0 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH30_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[536])          /* 0x40028E00 */
#define PERI_MS_PPU_FX_PASS0_SAR0_CH31_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[537])          /* 0x40028E40 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH0_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[538])          /* 0x40028E80 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH1_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[539])          /* 0x40028EC0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH2_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[540])          /* 0x40028F00 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH3_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[541])          /* 0x40028F40 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH4_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[542])          /* 0x40028F80 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH5_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[543])          /* 0x40028FC0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH6_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[544])          /* 0x40029000 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH7_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[545])          /* 0x40029040 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH8_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[546])          /* 0x40029080 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH9_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[547])          /* 0x400290C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH10_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[548])          /* 0x40029100 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH11_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[549])          /* 0x40029140 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH12_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[550])          /* 0x40029180 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH13_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[551])          /* 0x400291C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH14_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[552])          /* 0x40029200 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH15_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[553])          /* 0x40029240 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH16_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[554])          /* 0x40029280 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH17_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[555])          /* 0x400292C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH18_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[556])          /* 0x40029300 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH19_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[557])          /* 0x40029340 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH20_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[558])          /* 0x40029380 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH21_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[559])          /* 0x400293C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH22_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[560])          /* 0x40029400 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH23_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[561])          /* 0x40029440 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH24_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[562])          /* 0x40029480 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH25_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[563])          /* 0x400294C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH26_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[564])          /* 0x40029500 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH27_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[565])          /* 0x40029540 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH28_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[566])          /* 0x40029580 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH29_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[567])          /* 0x400295C0 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH30_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[568])          /* 0x40029600 */
#define PERI_MS_PPU_FX_PASS0_SAR1_CH31_CH       ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[569])          /* 0x40029640 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH0_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[570])          /* 0x40029680 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH1_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[571])          /* 0x400296C0 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH2_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[572])          /* 0x40029700 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH3_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[573])          /* 0x40029740 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH4_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[574])          /* 0x40029780 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH5_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[575])          /* 0x400297C0 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH6_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[576])          /* 0x40029800 */
#define PERI_MS_PPU_FX_PASS0_SAR2_CH7_CH        ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[577])          /* 0x40029840 */
#define PERI_MS_PPU_FX_PASS0_TOP                ((volatile stc_PERI_MS_PPU_FX_t*) &PERI_MS->PPU_FX[578])          /* 0x40029880 */

/*******************************************************************************
*                                  PERI_PCLK
*******************************************************************************/

#define PERI_PCLK_BASE                          0x40040000UL
#define PERI_PCLK                               ((volatile stc_PERI_PCLK_t*) PERI_PCLK_BASE)                      /* 0x40040000 */
#define PERI_PCLK_GR0                           ((volatile stc_PERI_PCLK_GR_t*) &PERI_PCLK->GR[0])                /* 0x40040000 */
#define PERI_PCLK_GR1                           ((volatile stc_PERI_PCLK_GR_t*) &PERI_PCLK->GR[1])                /* 0x40042000 */

/*******************************************************************************
*                                    CRYPTO
*******************************************************************************/

#define CRYPTO_BASE                             0x40100000UL
#define CRYPTO                                  ((volatile stc_CRYPTO_t*) CRYPTO_BASE)                            /* 0x40100000 */

/*******************************************************************************
*                                    CPUSS
*******************************************************************************/

#define CPUSS_BASE                              0x40200000UL
#define CPUSS                                   ((volatile stc_CPUSS_t*) CPUSS_BASE)                              /* 0x40200000 */

/*******************************************************************************
*                                    FAULT
*******************************************************************************/

#define FAULT_BASE                              0x40210000UL
#define FAULT                                   ((volatile stc_FAULT_t*) FAULT_BASE)                              /* 0x40210000 */
#define FAULT_STRUCT0                           ((volatile stc_FAULT_STRUCT_t*) &FAULT->STRUCT[0])                /* 0x40210000 */
#define FAULT_STRUCT1                           ((volatile stc_FAULT_STRUCT_t*) &FAULT->STRUCT[1])                /* 0x40210100 */
#define FAULT_STRUCT2                           ((volatile stc_FAULT_STRUCT_t*) &FAULT->STRUCT[2])                /* 0x40210200 */
#define FAULT_STRUCT3                           ((volatile stc_FAULT_STRUCT_t*) &FAULT->STRUCT[3])                /* 0x40210300 */

/*******************************************************************************
*                                     IPC
*******************************************************************************/

#define IPC_BASE                                0x40220000UL
#define IPC                                     ((volatile stc_IPC_t*) IPC_BASE)                                  /* 0x40220000 */
#define IPC_STRUCT0                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[0])                    /* 0x40220000 */
#define IPC_STRUCT1                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[1])                    /* 0x40220020 */
#define IPC_STRUCT2                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[2])                    /* 0x40220040 */
#define IPC_STRUCT3                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[3])                    /* 0x40220060 */
#define IPC_STRUCT4                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[4])                    /* 0x40220080 */
#define IPC_STRUCT5                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[5])                    /* 0x402200A0 */
#define IPC_STRUCT6                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[6])                    /* 0x402200C0 */
#define IPC_STRUCT7                             ((volatile stc_IPC_STRUCT_t*) &IPC->STRUCT[7])                    /* 0x402200E0 */
#define IPC_INTR_STRUCT0                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[0])          /* 0x40221000 */
#define IPC_INTR_STRUCT1                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[1])          /* 0x40221020 */
#define IPC_INTR_STRUCT2                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[2])          /* 0x40221040 */
#define IPC_INTR_STRUCT3                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[3])          /* 0x40221060 */
#define IPC_INTR_STRUCT4                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[4])          /* 0x40221080 */
#define IPC_INTR_STRUCT5                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[5])          /* 0x402210A0 */
#define IPC_INTR_STRUCT6                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[6])          /* 0x402210C0 */
#define IPC_INTR_STRUCT7                        ((volatile stc_IPC_INTR_STRUCT_t*) &IPC->INTR_STRUCT[7])          /* 0x402210E0 */

/*******************************************************************************
*                                     PROT
*******************************************************************************/

#define PROT_BASE                               0x40230000UL
#define PROT                                    ((volatile stc_PROT_t*) PROT_BASE)                                /* 0x40230000 */
#define PROT_SMPU                               ((volatile stc_PROT_SMPU_t*) &PROT->SMPU)                         /* 0x40230000 */
#define PROT_SMPU_SMPU_STRUCT0                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[0]) /* 0x40232000 */
#define PROT_SMPU_SMPU_STRUCT1                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[1]) /* 0x40232040 */
#define PROT_SMPU_SMPU_STRUCT2                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[2]) /* 0x40232080 */
#define PROT_SMPU_SMPU_STRUCT3                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[3]) /* 0x402320C0 */
#define PROT_SMPU_SMPU_STRUCT4                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[4]) /* 0x40232100 */
#define PROT_SMPU_SMPU_STRUCT5                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[5]) /* 0x40232140 */
#define PROT_SMPU_SMPU_STRUCT6                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[6]) /* 0x40232180 */
#define PROT_SMPU_SMPU_STRUCT7                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[7]) /* 0x402321C0 */
#define PROT_SMPU_SMPU_STRUCT8                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[8]) /* 0x40232200 */
#define PROT_SMPU_SMPU_STRUCT9                  ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[9]) /* 0x40232240 */
#define PROT_SMPU_SMPU_STRUCT10                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[10]) /* 0x40232280 */
#define PROT_SMPU_SMPU_STRUCT11                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[11]) /* 0x402322C0 */
#define PROT_SMPU_SMPU_STRUCT12                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[12]) /* 0x40232300 */
#define PROT_SMPU_SMPU_STRUCT13                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[13]) /* 0x40232340 */
#define PROT_SMPU_SMPU_STRUCT14                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[14]) /* 0x40232380 */
#define PROT_SMPU_SMPU_STRUCT15                 ((volatile stc_PROT_SMPU_SMPU_STRUCT_t*) &PROT->SMPU.SMPU_STRUCT[15]) /* 0x402323C0 */
#define PROT_MPU0                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[0])                      /* 0x40234000 */
#define PROT_MPU1                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[1])                      /* 0x40234400 */
#define PROT_MPU2                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[2])                      /* 0x40234800 */
#define PROT_MPU3                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[3])                      /* 0x40234C00 */
#define PROT_MPU4                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[4])                      /* 0x40235000 */
#define PROT_MPU5                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[5])                      /* 0x40235400 */
#define PROT_MPU6                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[6])                      /* 0x40235800 */
#define PROT_MPU7                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[7])                      /* 0x40235C00 */
#define PROT_MPU8                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[8])                      /* 0x40236000 */
#define PROT_MPU9                               ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[9])                      /* 0x40236400 */
#define PROT_MPU10                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[10])                     /* 0x40236800 */
#define PROT_MPU11                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[11])                     /* 0x40236C00 */
#define PROT_MPU12                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[12])                     /* 0x40237000 */
#define PROT_MPU13                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[13])                     /* 0x40237400 */
#define PROT_MPU14                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[14])                     /* 0x40237800 */
#define PROT_MPU15                              ((volatile stc_PROT_MPU_t*) &PROT->CYMPU[15])                     /* 0x40237C00 */
#define PROT_MPU5_MPU_STRUCT0                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[0]) /* 0x40235600 */
#define PROT_MPU5_MPU_STRUCT1                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[1]) /* 0x40235620 */
#define PROT_MPU5_MPU_STRUCT2                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[2]) /* 0x40235640 */
#define PROT_MPU5_MPU_STRUCT3                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[3]) /* 0x40235660 */
#define PROT_MPU5_MPU_STRUCT4                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[4]) /* 0x40235680 */
#define PROT_MPU5_MPU_STRUCT5                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[5]) /* 0x402356A0 */
#define PROT_MPU5_MPU_STRUCT6                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[6]) /* 0x402356C0 */
#define PROT_MPU5_MPU_STRUCT7                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[5].MPU_STRUCT[7]) /* 0x402356E0 */
#define PROT_MPU6_MPU_STRUCT0                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[0]) /* 0x40235A00 */
#define PROT_MPU6_MPU_STRUCT1                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[1]) /* 0x40235A20 */
#define PROT_MPU6_MPU_STRUCT2                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[2]) /* 0x40235A40 */
#define PROT_MPU6_MPU_STRUCT3                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[3]) /* 0x40235A60 */
#define PROT_MPU6_MPU_STRUCT4                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[4]) /* 0x40235A80 */
#define PROT_MPU6_MPU_STRUCT5                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[5]) /* 0x40235AA0 */
#define PROT_MPU6_MPU_STRUCT6                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[6]) /* 0x40235AC0 */
#define PROT_MPU6_MPU_STRUCT7                   ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[6].MPU_STRUCT[7]) /* 0x40235AE0 */
#define PROT_MPU15_MPU_STRUCT0                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[0]) /* 0x40237E00 */
#define PROT_MPU15_MPU_STRUCT1                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[1]) /* 0x40237E20 */
#define PROT_MPU15_MPU_STRUCT2                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[2]) /* 0x40237E40 */
#define PROT_MPU15_MPU_STRUCT3                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[3]) /* 0x40237E60 */
#define PROT_MPU15_MPU_STRUCT4                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[4]) /* 0x40237E80 */
#define PROT_MPU15_MPU_STRUCT5                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[5]) /* 0x40237EA0 */
#define PROT_MPU15_MPU_STRUCT6                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[6]) /* 0x40237EC0 */
#define PROT_MPU15_MPU_STRUCT7                  ((volatile stc_PROT_MPU_MPU_STRUCT_t*) &PROT->CYMPU[15].MPU_STRUCT[7]) /* 0x40237EE0 */

/*******************************************************************************
*                                    FLASHC
*******************************************************************************/

#define FLASHC_BASE                             0x40240000UL
#define FLASHC                                  ((volatile stc_FLASHC_t*) FLASHC_BASE)                            /* 0x40240000 */
#define FLASHC_FM_CTL_ECT                       ((volatile stc_FLASHC_FM_CTL_ECT_t*) &FLASHC->FM_CTL_ECT)         /* 0x4024F000 */

/*******************************************************************************
*                                     SRSS
*******************************************************************************/

#define SRSS_BASE                               0x40260000UL
#define SRSS                                    ((volatile stc_SRSS_t*) SRSS_BASE)                                /* 0x40260000 */
#define CSV_HF                                  ((volatile stc_CSV_HF_t*) &SRSS->CSV_HF)                          /* 0x40261400 */
#define CSV_HF_CSV0                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[0])               /* 0x40261400 */
#define CSV_HF_CSV1                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[1])               /* 0x40261410 */
#define CSV_HF_CSV2                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[2])               /* 0x40261420 */
#define CSV_HF_CSV3                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[3])               /* 0x40261430 */
#define CSV_HF_CSV4                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[4])               /* 0x40261440 */
#define CSV_HF_CSV5                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[5])               /* 0x40261450 */
#define CSV_HF_CSV6                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[6])               /* 0x40261460 */
#define CSV_HF_CSV7                             ((volatile stc_CSV_HF_CSV_t*) &SRSS->CSV_HF.CSV[7])               /* 0x40261470 */
#define CSV_REF                                 ((volatile stc_CSV_REF_t*) &SRSS->CSV_REF)                        /* 0x40261710 */
#define CSV_REF_CSV                             ((volatile stc_CSV_REF_CSV_t*) &SRSS->CSV_REF.CSV)                /* 0x40261710 */
#define CSV_LF                                  ((volatile stc_CSV_LF_t*) &SRSS->CSV_LF)                          /* 0x40261720 */
#define CSV_LF_CSV                              ((volatile stc_CSV_LF_CSV_t*) &SRSS->CSV_LF.CSV)                  /* 0x40261720 */
#define CSV_ILO                                 ((volatile stc_CSV_ILO_t*) &SRSS->CSV_ILO)                        /* 0x40261730 */
#define CSV_ILO_CSV                             ((volatile stc_CSV_ILO_CSV_t*) &SRSS->CSV_ILO.CSV)                /* 0x40261730 */
#define CLK_PLL400M0                            ((volatile stc_CLK_PLL400M_t*) &SRSS->CLK_PLL400M[0])             /* 0x40261900 */
#define CLK_PLL400M1                            ((volatile stc_CLK_PLL400M_t*) &SRSS->CLK_PLL400M[1])             /* 0x40261910 */
#define MCWDT0                                  ((volatile stc_MCWDT_t*) &SRSS->MCWDT[0])                         /* 0x40268000 */
#define MCWDT1                                  ((volatile stc_MCWDT_t*) &SRSS->MCWDT[1])                         /* 0x40268100 */
#define MCWDT2                                  ((volatile stc_MCWDT_t*) &SRSS->MCWDT[2])                         /* 0x40268200 */
#define MCWDT0_CTR0                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[0].CTR[0])              /* 0x40268000 */
#define MCWDT0_CTR1                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[0].CTR[1])              /* 0x40268020 */
#define MCWDT1_CTR0                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[1].CTR[0])              /* 0x40268100 */
#define MCWDT1_CTR1                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[1].CTR[1])              /* 0x40268120 */
#define MCWDT2_CTR0                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[2].CTR[0])              /* 0x40268200 */
#define MCWDT2_CTR1                             ((volatile stc_MCWDT_CTR_t*) &SRSS->MCWDT[2].CTR[1])              /* 0x40268220 */
#define WDT                                     ((volatile stc_WDT_t*) &SRSS->WDT)                                /* 0x4026C000 */

/*******************************************************************************
*                                    BACKUP
*******************************************************************************/

#define BACKUP_BASE                             0x40270000UL
#define BACKUP                                  ((volatile stc_BACKUP_t*) BACKUP_BASE)                            /* 0x40270000 */

/*******************************************************************************
*                                      DW
*******************************************************************************/

#define DW0_BASE                                0x40280000UL
#define DW1_BASE                                0x40290000UL
#define DW0                                     ((volatile stc_DW_t*) DW0_BASE)                                   /* 0x40280000 */
#define DW1                                     ((volatile stc_DW_t*) DW1_BASE)                                   /* 0x40290000 */
#define DW0_CH_STRUCT0                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[0])               /* 0x40288000 */
#define DW0_CH_STRUCT1                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[1])               /* 0x40288040 */
#define DW0_CH_STRUCT2                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[2])               /* 0x40288080 */
#define DW0_CH_STRUCT3                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[3])               /* 0x402880C0 */
#define DW0_CH_STRUCT4                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[4])               /* 0x40288100 */
#define DW0_CH_STRUCT5                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[5])               /* 0x40288140 */
#define DW0_CH_STRUCT6                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[6])               /* 0x40288180 */
#define DW0_CH_STRUCT7                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[7])               /* 0x402881C0 */
#define DW0_CH_STRUCT8                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[8])               /* 0x40288200 */
#define DW0_CH_STRUCT9                          ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[9])               /* 0x40288240 */
#define DW0_CH_STRUCT10                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[10])              /* 0x40288280 */
#define DW0_CH_STRUCT11                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[11])              /* 0x402882C0 */
#define DW0_CH_STRUCT12                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[12])              /* 0x40288300 */
#define DW0_CH_STRUCT13                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[13])              /* 0x40288340 */
#define DW0_CH_STRUCT14                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[14])              /* 0x40288380 */
#define DW0_CH_STRUCT15                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[15])              /* 0x402883C0 */
#define DW0_CH_STRUCT16                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[16])              /* 0x40288400 */
#define DW0_CH_STRUCT17                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[17])              /* 0x40288440 */
#define DW0_CH_STRUCT18                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[18])              /* 0x40288480 */
#define DW0_CH_STRUCT19                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[19])              /* 0x402884C0 */
#define DW0_CH_STRUCT20                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[20])              /* 0x40288500 */
#define DW0_CH_STRUCT21                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[21])              /* 0x40288540 */
#define DW0_CH_STRUCT22                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[22])              /* 0x40288580 */
#define DW0_CH_STRUCT23                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[23])              /* 0x402885C0 */
#define DW0_CH_STRUCT24                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[24])              /* 0x40288600 */
#define DW0_CH_STRUCT25                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[25])              /* 0x40288640 */
#define DW0_CH_STRUCT26                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[26])              /* 0x40288680 */
#define DW0_CH_STRUCT27                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[27])              /* 0x402886C0 */
#define DW0_CH_STRUCT28                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[28])              /* 0x40288700 */
#define DW0_CH_STRUCT29                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[29])              /* 0x40288740 */
#define DW0_CH_STRUCT30                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[30])              /* 0x40288780 */
#define DW0_CH_STRUCT31                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[31])              /* 0x402887C0 */
#define DW0_CH_STRUCT32                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[32])              /* 0x40288800 */
#define DW0_CH_STRUCT33                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[33])              /* 0x40288840 */
#define DW0_CH_STRUCT34                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[34])              /* 0x40288880 */
#define DW0_CH_STRUCT35                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[35])              /* 0x402888C0 */
#define DW0_CH_STRUCT36                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[36])              /* 0x40288900 */
#define DW0_CH_STRUCT37                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[37])              /* 0x40288940 */
#define DW0_CH_STRUCT38                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[38])              /* 0x40288980 */
#define DW0_CH_STRUCT39                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[39])              /* 0x402889C0 */
#define DW0_CH_STRUCT40                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[40])              /* 0x40288A00 */
#define DW0_CH_STRUCT41                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[41])              /* 0x40288A40 */
#define DW0_CH_STRUCT42                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[42])              /* 0x40288A80 */
#define DW0_CH_STRUCT43                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[43])              /* 0x40288AC0 */
#define DW0_CH_STRUCT44                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[44])              /* 0x40288B00 */
#define DW0_CH_STRUCT45                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[45])              /* 0x40288B40 */
#define DW0_CH_STRUCT46                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[46])              /* 0x40288B80 */
#define DW0_CH_STRUCT47                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[47])              /* 0x40288BC0 */
#define DW0_CH_STRUCT48                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[48])              /* 0x40288C00 */
#define DW0_CH_STRUCT49                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[49])              /* 0x40288C40 */
#define DW0_CH_STRUCT50                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[50])              /* 0x40288C80 */
#define DW0_CH_STRUCT51                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[51])              /* 0x40288CC0 */
#define DW0_CH_STRUCT52                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[52])              /* 0x40288D00 */
#define DW0_CH_STRUCT53                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[53])              /* 0x40288D40 */
#define DW0_CH_STRUCT54                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[54])              /* 0x40288D80 */
#define DW0_CH_STRUCT55                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[55])              /* 0x40288DC0 */
#define DW0_CH_STRUCT56                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[56])              /* 0x40288E00 */
#define DW0_CH_STRUCT57                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[57])              /* 0x40288E40 */
#define DW0_CH_STRUCT58                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[58])              /* 0x40288E80 */
#define DW0_CH_STRUCT59                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[59])              /* 0x40288EC0 */
#define DW0_CH_STRUCT60                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[60])              /* 0x40288F00 */
#define DW0_CH_STRUCT61                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[61])              /* 0x40288F40 */
#define DW0_CH_STRUCT62                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[62])              /* 0x40288F80 */
#define DW0_CH_STRUCT63                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[63])              /* 0x40288FC0 */
#define DW0_CH_STRUCT64                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[64])              /* 0x40289000 */
#define DW0_CH_STRUCT65                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[65])              /* 0x40289040 */
#define DW0_CH_STRUCT66                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[66])              /* 0x40289080 */
#define DW0_CH_STRUCT67                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[67])              /* 0x402890C0 */
#define DW0_CH_STRUCT68                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[68])              /* 0x40289100 */
#define DW0_CH_STRUCT69                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[69])              /* 0x40289140 */
#define DW0_CH_STRUCT70                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[70])              /* 0x40289180 */
#define DW0_CH_STRUCT71                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[71])              /* 0x402891C0 */
#define DW0_CH_STRUCT72                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[72])              /* 0x40289200 */
#define DW0_CH_STRUCT73                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[73])              /* 0x40289240 */
#define DW0_CH_STRUCT74                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[74])              /* 0x40289280 */
#define DW0_CH_STRUCT75                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[75])              /* 0x402892C0 */
#define DW0_CH_STRUCT76                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[76])              /* 0x40289300 */
#define DW0_CH_STRUCT77                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[77])              /* 0x40289340 */
#define DW0_CH_STRUCT78                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[78])              /* 0x40289380 */
#define DW0_CH_STRUCT79                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[79])              /* 0x402893C0 */
#define DW0_CH_STRUCT80                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[80])              /* 0x40289400 */
#define DW0_CH_STRUCT81                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[81])              /* 0x40289440 */
#define DW0_CH_STRUCT82                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[82])              /* 0x40289480 */
#define DW0_CH_STRUCT83                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[83])              /* 0x402894C0 */
#define DW0_CH_STRUCT84                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[84])              /* 0x40289500 */
#define DW0_CH_STRUCT85                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[85])              /* 0x40289540 */
#define DW0_CH_STRUCT86                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[86])              /* 0x40289580 */
#define DW0_CH_STRUCT87                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[87])              /* 0x402895C0 */
#define DW0_CH_STRUCT88                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[88])              /* 0x40289600 */
#define DW0_CH_STRUCT89                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[89])              /* 0x40289640 */
#define DW0_CH_STRUCT90                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[90])              /* 0x40289680 */
#define DW0_CH_STRUCT91                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[91])              /* 0x402896C0 */
#define DW0_CH_STRUCT92                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[92])              /* 0x40289700 */
#define DW0_CH_STRUCT93                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[93])              /* 0x40289740 */
#define DW0_CH_STRUCT94                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[94])              /* 0x40289780 */
#define DW0_CH_STRUCT95                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[95])              /* 0x402897C0 */
#define DW0_CH_STRUCT96                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[96])              /* 0x40289800 */
#define DW0_CH_STRUCT97                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[97])              /* 0x40289840 */
#define DW0_CH_STRUCT98                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[98])              /* 0x40289880 */
#define DW0_CH_STRUCT99                         ((volatile stc_DW_CH_STRUCT_t*) &DW0->CH_STRUCT[99])              /* 0x402898C0 */
#define DW1_CH_STRUCT0                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[0])               /* 0x40298000 */
#define DW1_CH_STRUCT1                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[1])               /* 0x40298040 */
#define DW1_CH_STRUCT2                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[2])               /* 0x40298080 */
#define DW1_CH_STRUCT3                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[3])               /* 0x402980C0 */
#define DW1_CH_STRUCT4                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[4])               /* 0x40298100 */
#define DW1_CH_STRUCT5                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[5])               /* 0x40298140 */
#define DW1_CH_STRUCT6                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[6])               /* 0x40298180 */
#define DW1_CH_STRUCT7                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[7])               /* 0x402981C0 */
#define DW1_CH_STRUCT8                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[8])               /* 0x40298200 */
#define DW1_CH_STRUCT9                          ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[9])               /* 0x40298240 */
#define DW1_CH_STRUCT10                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[10])              /* 0x40298280 */
#define DW1_CH_STRUCT11                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[11])              /* 0x402982C0 */
#define DW1_CH_STRUCT12                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[12])              /* 0x40298300 */
#define DW1_CH_STRUCT13                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[13])              /* 0x40298340 */
#define DW1_CH_STRUCT14                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[14])              /* 0x40298380 */
#define DW1_CH_STRUCT15                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[15])              /* 0x402983C0 */
#define DW1_CH_STRUCT16                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[16])              /* 0x40298400 */
#define DW1_CH_STRUCT17                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[17])              /* 0x40298440 */
#define DW1_CH_STRUCT18                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[18])              /* 0x40298480 */
#define DW1_CH_STRUCT19                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[19])              /* 0x402984C0 */
#define DW1_CH_STRUCT20                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[20])              /* 0x40298500 */
#define DW1_CH_STRUCT21                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[21])              /* 0x40298540 */
#define DW1_CH_STRUCT22                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[22])              /* 0x40298580 */
#define DW1_CH_STRUCT23                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[23])              /* 0x402985C0 */
#define DW1_CH_STRUCT24                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[24])              /* 0x40298600 */
#define DW1_CH_STRUCT25                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[25])              /* 0x40298640 */
#define DW1_CH_STRUCT26                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[26])              /* 0x40298680 */
#define DW1_CH_STRUCT27                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[27])              /* 0x402986C0 */
#define DW1_CH_STRUCT28                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[28])              /* 0x40298700 */
#define DW1_CH_STRUCT29                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[29])              /* 0x40298740 */
#define DW1_CH_STRUCT30                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[30])              /* 0x40298780 */
#define DW1_CH_STRUCT31                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[31])              /* 0x402987C0 */
#define DW1_CH_STRUCT32                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[32])              /* 0x40298800 */
#define DW1_CH_STRUCT33                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[33])              /* 0x40298840 */
#define DW1_CH_STRUCT34                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[34])              /* 0x40298880 */
#define DW1_CH_STRUCT35                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[35])              /* 0x402988C0 */
#define DW1_CH_STRUCT36                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[36])              /* 0x40298900 */
#define DW1_CH_STRUCT37                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[37])              /* 0x40298940 */
#define DW1_CH_STRUCT38                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[38])              /* 0x40298980 */
#define DW1_CH_STRUCT39                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[39])              /* 0x402989C0 */
#define DW1_CH_STRUCT40                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[40])              /* 0x40298A00 */
#define DW1_CH_STRUCT41                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[41])              /* 0x40298A40 */
#define DW1_CH_STRUCT42                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[42])              /* 0x40298A80 */
#define DW1_CH_STRUCT43                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[43])              /* 0x40298AC0 */
#define DW1_CH_STRUCT44                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[44])              /* 0x40298B00 */
#define DW1_CH_STRUCT45                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[45])              /* 0x40298B40 */
#define DW1_CH_STRUCT46                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[46])              /* 0x40298B80 */
#define DW1_CH_STRUCT47                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[47])              /* 0x40298BC0 */
#define DW1_CH_STRUCT48                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[48])              /* 0x40298C00 */
#define DW1_CH_STRUCT49                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[49])              /* 0x40298C40 */
#define DW1_CH_STRUCT50                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[50])              /* 0x40298C80 */
#define DW1_CH_STRUCT51                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[51])              /* 0x40298CC0 */
#define DW1_CH_STRUCT52                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[52])              /* 0x40298D00 */
#define DW1_CH_STRUCT53                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[53])              /* 0x40298D40 */
#define DW1_CH_STRUCT54                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[54])              /* 0x40298D80 */
#define DW1_CH_STRUCT55                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[55])              /* 0x40298DC0 */
#define DW1_CH_STRUCT56                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[56])              /* 0x40298E00 */
#define DW1_CH_STRUCT57                         ((volatile stc_DW_CH_STRUCT_t*) &DW1->CH_STRUCT[57])              /* 0x40298E40 */

/*******************************************************************************
*                                     DMAC
*******************************************************************************/

#define DMAC_BASE                               0x402A0000UL
#define DMAC                                    ((volatile stc_DMAC_t*) DMAC_BASE)                                /* 0x402A0000 */
#define DMAC_CH0                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[0])                          /* 0x402A1000 */
#define DMAC_CH1                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[1])                          /* 0x402A1100 */
#define DMAC_CH2                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[2])                          /* 0x402A1200 */
#define DMAC_CH3                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[3])                          /* 0x402A1300 */
#define DMAC_CH4                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[4])                          /* 0x402A1400 */
#define DMAC_CH5                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[5])                          /* 0x402A1500 */
#define DMAC_CH6                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[6])                          /* 0x402A1600 */
#define DMAC_CH7                                ((volatile stc_DMAC_CH_t*) &DMAC->CH[7])                          /* 0x402A1700 */

/*******************************************************************************
*                                    EFUSE
*******************************************************************************/

#define EFUSE_BASE                              0x402C0000UL
#define EFUSE                                   ((volatile stc_EFUSE_t*) EFUSE_BASE)                              /* 0x402C0000 */

/*******************************************************************************
*                                    HSIOM
*******************************************************************************/

#define HSIOM_BASE                              0x40300000UL
#define HSIOM                                   ((volatile stc_HSIOM_t*) HSIOM_BASE)                              /* 0x40300000 */
#define HSIOM_PRT0                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[0])                      /* 0x40300000 */
#define HSIOM_PRT1                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[1])                      /* 0x40300010 */
#define HSIOM_PRT2                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[2])                      /* 0x40300020 */
#define HSIOM_PRT3                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[3])                      /* 0x40300030 */
#define HSIOM_PRT4                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[4])                      /* 0x40300040 */
#define HSIOM_PRT5                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[5])                      /* 0x40300050 */
#define HSIOM_PRT6                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[6])                      /* 0x40300060 */
#define HSIOM_PRT7                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[7])                      /* 0x40300070 */
#define HSIOM_PRT8                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[8])                      /* 0x40300080 */
#define HSIOM_PRT9                              ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[9])                      /* 0x40300090 */
#define HSIOM_PRT10                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[10])                     /* 0x403000A0 */
#define HSIOM_PRT11                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[11])                     /* 0x403000B0 */
#define HSIOM_PRT12                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[12])                     /* 0x403000C0 */
#define HSIOM_PRT13                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[13])                     /* 0x403000D0 */
#define HSIOM_PRT14                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[14])                     /* 0x403000E0 */
#define HSIOM_PRT15                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[15])                     /* 0x403000F0 */
#define HSIOM_PRT16                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[16])                     /* 0x40300100 */
#define HSIOM_PRT17                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[17])                     /* 0x40300110 */
#define HSIOM_PRT18                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[18])                     /* 0x40300120 */
#define HSIOM_PRT19                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[19])                     /* 0x40300130 */
#define HSIOM_PRT20                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[20])                     /* 0x40300140 */
#define HSIOM_PRT21                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[21])                     /* 0x40300150 */
#define HSIOM_PRT22                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[22])                     /* 0x40300160 */
#define HSIOM_PRT23                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[23])                     /* 0x40300170 */
#define HSIOM_PRT24                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[24])                     /* 0x40300180 */
#define HSIOM_PRT25                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[25])                     /* 0x40300190 */
#define HSIOM_PRT26                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[26])                     /* 0x403001A0 */
#define HSIOM_PRT27                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[27])                     /* 0x403001B0 */
#define HSIOM_PRT28                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[28])                     /* 0x403001C0 */
#define HSIOM_PRT29                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[29])                     /* 0x403001D0 */
#define HSIOM_PRT30                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[30])                     /* 0x403001E0 */
#define HSIOM_PRT31                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[31])                     /* 0x403001F0 */
#define HSIOM_PRT32                             ((volatile stc_HSIOM_PRT_t*) &HSIOM->PRT[32])                     /* 0x40300200 */

/*******************************************************************************
*                                     GPIO
*******************************************************************************/

#define GPIO_BASE                               0x40310000UL
#define GPIO                                    ((volatile stc_GPIO_t*) GPIO_BASE)                                /* 0x40310000 */
#define GPIO_PRT0                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[0])                        /* 0x40310000 */
#define GPIO_PRT1                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[1])                        /* 0x40310080 */
#define GPIO_PRT2                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[2])                        /* 0x40310100 */
#define GPIO_PRT3                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[3])                        /* 0x40310180 */
#define GPIO_PRT4                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[4])                        /* 0x40310200 */
#define GPIO_PRT5                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[5])                        /* 0x40310280 */
#define GPIO_PRT6                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[6])                        /* 0x40310300 */
#define GPIO_PRT7                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[7])                        /* 0x40310380 */
#define GPIO_PRT8                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[8])                        /* 0x40310400 */
#define GPIO_PRT9                               ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[9])                        /* 0x40310480 */
#define GPIO_PRT10                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[10])                       /* 0x40310500 */
#define GPIO_PRT11                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[11])                       /* 0x40310580 */
#define GPIO_PRT12                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[12])                       /* 0x40310600 */
#define GPIO_PRT13                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[13])                       /* 0x40310680 */
#define GPIO_PRT14                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[14])                       /* 0x40310700 */
#define GPIO_PRT15                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[15])                       /* 0x40310780 */
#define GPIO_PRT16                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[16])                       /* 0x40310800 */
#define GPIO_PRT17                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[17])                       /* 0x40310880 */
#define GPIO_PRT18                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[18])                       /* 0x40310900 */
#define GPIO_PRT19                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[19])                       /* 0x40310980 */
#define GPIO_PRT20                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[20])                       /* 0x40310A00 */
#define GPIO_PRT21                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[21])                       /* 0x40310A80 */
#define GPIO_PRT22                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[22])                       /* 0x40310B00 */
#define GPIO_PRT23                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[23])                       /* 0x40310B80 */
#define GPIO_PRT24                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[24])                       /* 0x40310C00 */
#define GPIO_PRT25                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[25])                       /* 0x40310C80 */
#define GPIO_PRT26                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[26])                       /* 0x40310D00 */
#define GPIO_PRT27                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[27])                       /* 0x40310D80 */
#define GPIO_PRT28                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[28])                       /* 0x40310E00 */
#define GPIO_PRT29                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[29])                       /* 0x40310E80 */
#define GPIO_PRT30                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[30])                       /* 0x40310F00 */
#define GPIO_PRT31                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[31])                       /* 0x40310F80 */
#define GPIO_PRT32                              ((volatile stc_GPIO_PRT_t*) &GPIO->PRT[32])                       /* 0x40311000 */

/*******************************************************************************
*                                   SMARTIO
*******************************************************************************/

#define SMARTIO_BASE                            0x40320000UL
#define SMARTIO                                 ((volatile stc_SMARTIO_t*) SMARTIO_BASE)                          /* 0x40320000 */
#define SMARTIO_PRT12                           ((volatile stc_SMARTIO_PRT_t*) &SMARTIO->PRT[12])                 /* 0x40320C00 */
#define SMARTIO_PRT13                           ((volatile stc_SMARTIO_PRT_t*) &SMARTIO->PRT[13])                 /* 0x40320D00 */
#define SMARTIO_PRT14                           ((volatile stc_SMARTIO_PRT_t*) &SMARTIO->PRT[14])                 /* 0x40320E00 */
#define SMARTIO_PRT15                           ((volatile stc_SMARTIO_PRT_t*) &SMARTIO->PRT[15])                 /* 0x40320F00 */
#define SMARTIO_PRT17                           ((volatile stc_SMARTIO_PRT_t*) &SMARTIO->PRT[17])                 /* 0x40321100 */

/*******************************************************************************
*                                    EVTGEN
*******************************************************************************/

#define EVTGEN0_BASE                            0x403F0000UL
#define EVTGEN0                                 ((volatile stc_EVTGEN_t*) EVTGEN0_BASE)                           /* 0x403F0000 */
#define EVTGEN0_COMP_STRUCT0                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[0])   /* 0x403F0800 */
#define EVTGEN0_COMP_STRUCT1                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[1])   /* 0x403F0820 */
#define EVTGEN0_COMP_STRUCT2                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[2])   /* 0x403F0840 */
#define EVTGEN0_COMP_STRUCT3                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[3])   /* 0x403F0860 */
#define EVTGEN0_COMP_STRUCT4                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[4])   /* 0x403F0880 */
#define EVTGEN0_COMP_STRUCT5                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[5])   /* 0x403F08A0 */
#define EVTGEN0_COMP_STRUCT6                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[6])   /* 0x403F08C0 */
#define EVTGEN0_COMP_STRUCT7                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[7])   /* 0x403F08E0 */
#define EVTGEN0_COMP_STRUCT8                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[8])   /* 0x403F0900 */
#define EVTGEN0_COMP_STRUCT9                    ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[9])   /* 0x403F0920 */
#define EVTGEN0_COMP_STRUCT10                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[10])  /* 0x403F0940 */
#define EVTGEN0_COMP_STRUCT11                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[11])  /* 0x403F0960 */
#define EVTGEN0_COMP_STRUCT12                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[12])  /* 0x403F0980 */
#define EVTGEN0_COMP_STRUCT13                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[13])  /* 0x403F09A0 */
#define EVTGEN0_COMP_STRUCT14                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[14])  /* 0x403F09C0 */
#define EVTGEN0_COMP_STRUCT15                   ((volatile stc_EVTGEN_COMP_STRUCT_t*) &EVTGEN0->COMP_STRUCT[15])  /* 0x403F09E0 */

/*******************************************************************************
*                                     SMIF
*******************************************************************************/

#define SMIF0_BASE                              0x40420000UL
#define SMIF0                                   ((volatile stc_SMIF_t*) SMIF0_BASE)                               /* 0x40420000 */
#define SMIF0_DEVICE0                           ((volatile stc_SMIF_DEVICE_t*) &SMIF0->DEVICE[0])                 /* 0x40420800 */
#define SMIF0_DEVICE1                           ((volatile stc_SMIF_DEVICE_t*) &SMIF0->DEVICE[1])                 /* 0x40420880 */

/*******************************************************************************
*                                     SDHC
*******************************************************************************/

#define SDHC0_BASE                              0x40460000UL
#define SDHC0                                   ((volatile stc_SDHC_t*) SDHC0_BASE)                               /* 0x40460000 */
#define SDHC0_WRAP                              ((volatile stc_SDHC_WRAP_t*) &SDHC0->WRAP)                        /* 0x40460000 */
#define SDHC0_CORE                              ((volatile stc_SDHC_CORE_t*) &SDHC0->CORE)                        /* 0x40461000 */

/*******************************************************************************
*                                     ETH
*******************************************************************************/

#define ETH0_BASE                               0x40480000UL
#define ETH0                                    ((volatile stc_ETH_t*) ETH0_BASE)                                 /* 0x40480000 */

/*******************************************************************************
*                                     LIN
*******************************************************************************/

#define LIN0_BASE                               0x40500000UL
#define LIN0                                    ((volatile stc_LIN_t*) LIN0_BASE)                                 /* 0x40500000 */
#define LIN0_CH0                                ((volatile stc_LIN_CH_t*) &LIN0->CH[0])                           /* 0x40508000 */
#define LIN0_CH1                                ((volatile stc_LIN_CH_t*) &LIN0->CH[1])                           /* 0x40508100 */
#define LIN0_CH2                                ((volatile stc_LIN_CH_t*) &LIN0->CH[2])                           /* 0x40508200 */
#define LIN0_CH3                                ((volatile stc_LIN_CH_t*) &LIN0->CH[3])                           /* 0x40508300 */
#define LIN0_CH4                                ((volatile stc_LIN_CH_t*) &LIN0->CH[4])                           /* 0x40508400 */
#define LIN0_CH5                                ((volatile stc_LIN_CH_t*) &LIN0->CH[5])                           /* 0x40508500 */
#define LIN0_CH6                                ((volatile stc_LIN_CH_t*) &LIN0->CH[6])                           /* 0x40508600 */
#define LIN0_CH7                                ((volatile stc_LIN_CH_t*) &LIN0->CH[7])                           /* 0x40508700 */
#define LIN0_CH8                                ((volatile stc_LIN_CH_t*) &LIN0->CH[8])                           /* 0x40508800 */
#define LIN0_CH9                                ((volatile stc_LIN_CH_t*) &LIN0->CH[9])                           /* 0x40508900 */
#define LIN0_CH10                               ((volatile stc_LIN_CH_t*) &LIN0->CH[10])                          /* 0x40508A00 */
#define LIN0_CH11                               ((volatile stc_LIN_CH_t*) &LIN0->CH[11])                          /* 0x40508B00 */
#define LIN0_CH12                               ((volatile stc_LIN_CH_t*) &LIN0->CH[12])                          /* 0x40508C00 */
#define LIN0_CH13                               ((volatile stc_LIN_CH_t*) &LIN0->CH[13])                          /* 0x40508D00 */
#define LIN0_CH14                               ((volatile stc_LIN_CH_t*) &LIN0->CH[14])                          /* 0x40508E00 */
#define LIN0_CH15                               ((volatile stc_LIN_CH_t*) &LIN0->CH[15])                          /* 0x40508F00 */

/*******************************************************************************
*                                    CANFD
*******************************************************************************/

#define CANFD0_BASE                             0x40520000UL
#define CANFD1_BASE                             0x40540000UL
#define CANFD0                                  ((volatile stc_CANFD_t*) CANFD0_BASE)                             /* 0x40520000 */
#define CANFD1                                  ((volatile stc_CANFD_t*) CANFD1_BASE)                             /* 0x40540000 */
#define CANFD0_CH0                              ((volatile stc_CANFD_CH_t*) &CANFD0->CH[0])                       /* 0x40520000 */
#define CANFD0_CH1                              ((volatile stc_CANFD_CH_t*) &CANFD0->CH[1])                       /* 0x40520200 */
#define CANFD0_CH2                              ((volatile stc_CANFD_CH_t*) &CANFD0->CH[2])                       /* 0x40520400 */
#define CANFD0_CH3                              ((volatile stc_CANFD_CH_t*) &CANFD0->CH[3])                       /* 0x40520600 */
#define CANFD1_CH0                              ((volatile stc_CANFD_CH_t*) &CANFD1->CH[0])                       /* 0x40540000 */
#define CANFD1_CH1                              ((volatile stc_CANFD_CH_t*) &CANFD1->CH[1])                       /* 0x40540200 */
#define CANFD1_CH2                              ((volatile stc_CANFD_CH_t*) &CANFD1->CH[2])                       /* 0x40540400 */
#define CANFD1_CH3                              ((volatile stc_CANFD_CH_t*) &CANFD1->CH[3])                       /* 0x40540600 */
#define CANFD0_CH0_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD0->CH[0].M_TTCAN)       /* 0x40520000 */
#define CANFD0_CH1_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD0->CH[1].M_TTCAN)       /* 0x40520200 */
#define CANFD0_CH2_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD0->CH[2].M_TTCAN)       /* 0x40520400 */
#define CANFD0_CH3_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD0->CH[3].M_TTCAN)       /* 0x40520600 */
#define CANFD1_CH0_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD1->CH[0].M_TTCAN)       /* 0x40540000 */
#define CANFD1_CH1_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD1->CH[1].M_TTCAN)       /* 0x40540200 */
#define CANFD1_CH2_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD1->CH[2].M_TTCAN)       /* 0x40540400 */
#define CANFD1_CH3_M_TTCAN                      ((volatile stc_CANFD_CH_M_TTCAN_t*) &CANFD1->CH[3].M_TTCAN)       /* 0x40540600 */

/*******************************************************************************
*                                    TCPWM
*******************************************************************************/

#define TCPWM0_BASE                             0x40580000UL
#define TCPWM0                                  ((volatile stc_TCPWM_t*) TCPWM0_BASE)                             /* 0x40580000 */
#define TCPWM0_GRP0                             ((volatile stc_TCPWM_GRP_t*) &TCPWM0->GRP[0])                     /* 0x40580000 */
#define TCPWM0_GRP1                             ((volatile stc_TCPWM_GRP_t*) &TCPWM0->GRP[1])                     /* 0x40588000 */
#define TCPWM0_GRP2                             ((volatile stc_TCPWM_GRP_t*) &TCPWM0->GRP[2])                     /* 0x40590000 */
#define TCPWM0_GRP0_CNT0                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[0])          /* 0x40580000 */
#define TCPWM0_GRP0_CNT1                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[1])          /* 0x40580080 */
#define TCPWM0_GRP0_CNT2                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[2])          /* 0x40580100 */
#define TCPWM0_GRP0_CNT3                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[3])          /* 0x40580180 */
#define TCPWM0_GRP0_CNT4                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[4])          /* 0x40580200 */
#define TCPWM0_GRP0_CNT5                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[5])          /* 0x40580280 */
#define TCPWM0_GRP0_CNT6                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[6])          /* 0x40580300 */
#define TCPWM0_GRP0_CNT7                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[7])          /* 0x40580380 */
#define TCPWM0_GRP0_CNT8                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[8])          /* 0x40580400 */
#define TCPWM0_GRP0_CNT9                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[9])          /* 0x40580480 */
#define TCPWM0_GRP0_CNT10                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[10])         /* 0x40580500 */
#define TCPWM0_GRP0_CNT11                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[11])         /* 0x40580580 */
#define TCPWM0_GRP0_CNT12                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[12])         /* 0x40580600 */
#define TCPWM0_GRP0_CNT13                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[13])         /* 0x40580680 */
#define TCPWM0_GRP0_CNT14                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[14])         /* 0x40580700 */
#define TCPWM0_GRP0_CNT15                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[15])         /* 0x40580780 */
#define TCPWM0_GRP0_CNT16                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[16])         /* 0x40580800 */
#define TCPWM0_GRP0_CNT17                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[17])         /* 0x40580880 */
#define TCPWM0_GRP0_CNT18                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[18])         /* 0x40580900 */
#define TCPWM0_GRP0_CNT19                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[19])         /* 0x40580980 */
#define TCPWM0_GRP0_CNT20                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[20])         /* 0x40580A00 */
#define TCPWM0_GRP0_CNT21                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[21])         /* 0x40580A80 */
#define TCPWM0_GRP0_CNT22                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[22])         /* 0x40580B00 */
#define TCPWM0_GRP0_CNT23                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[23])         /* 0x40580B80 */
#define TCPWM0_GRP0_CNT24                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[24])         /* 0x40580C00 */
#define TCPWM0_GRP0_CNT25                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[25])         /* 0x40580C80 */
#define TCPWM0_GRP0_CNT26                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[26])         /* 0x40580D00 */
#define TCPWM0_GRP0_CNT27                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[27])         /* 0x40580D80 */
#define TCPWM0_GRP0_CNT28                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[28])         /* 0x40580E00 */
#define TCPWM0_GRP0_CNT29                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[29])         /* 0x40580E80 */
#define TCPWM0_GRP0_CNT30                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[30])         /* 0x40580F00 */
#define TCPWM0_GRP0_CNT31                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[31])         /* 0x40580F80 */
#define TCPWM0_GRP0_CNT32                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[32])         /* 0x40581000 */
#define TCPWM0_GRP0_CNT33                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[33])         /* 0x40581080 */
#define TCPWM0_GRP0_CNT34                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[34])         /* 0x40581100 */
#define TCPWM0_GRP0_CNT35                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[35])         /* 0x40581180 */
#define TCPWM0_GRP0_CNT36                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[36])         /* 0x40581200 */
#define TCPWM0_GRP0_CNT37                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[37])         /* 0x40581280 */
#define TCPWM0_GRP0_CNT38                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[38])         /* 0x40581300 */
#define TCPWM0_GRP0_CNT39                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[39])         /* 0x40581380 */
#define TCPWM0_GRP0_CNT40                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[40])         /* 0x40581400 */
#define TCPWM0_GRP0_CNT41                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[41])         /* 0x40581480 */
#define TCPWM0_GRP0_CNT42                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[42])         /* 0x40581500 */
#define TCPWM0_GRP0_CNT43                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[43])         /* 0x40581580 */
#define TCPWM0_GRP0_CNT44                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[44])         /* 0x40581600 */
#define TCPWM0_GRP0_CNT45                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[45])         /* 0x40581680 */
#define TCPWM0_GRP0_CNT46                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[46])         /* 0x40581700 */
#define TCPWM0_GRP0_CNT47                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[47])         /* 0x40581780 */
#define TCPWM0_GRP0_CNT48                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[48])         /* 0x40581800 */
#define TCPWM0_GRP0_CNT49                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[49])         /* 0x40581880 */
#define TCPWM0_GRP0_CNT50                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[50])         /* 0x40581900 */
#define TCPWM0_GRP0_CNT51                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[51])         /* 0x40581980 */
#define TCPWM0_GRP0_CNT52                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[52])         /* 0x40581A00 */
#define TCPWM0_GRP0_CNT53                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[53])         /* 0x40581A80 */
#define TCPWM0_GRP0_CNT54                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[54])         /* 0x40581B00 */
#define TCPWM0_GRP0_CNT55                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[55])         /* 0x40581B80 */
#define TCPWM0_GRP0_CNT56                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[56])         /* 0x40581C00 */
#define TCPWM0_GRP0_CNT57                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[57])         /* 0x40581C80 */
#define TCPWM0_GRP0_CNT58                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[58])         /* 0x40581D00 */
#define TCPWM0_GRP0_CNT59                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[59])         /* 0x40581D80 */
#define TCPWM0_GRP0_CNT60                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[60])         /* 0x40581E00 */
#define TCPWM0_GRP0_CNT61                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[61])         /* 0x40581E80 */
#define TCPWM0_GRP0_CNT62                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[0].CNT[62])         /* 0x40581F00 */
#define TCPWM0_GRP1_CNT0                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[0])          /* 0x40588000 */
#define TCPWM0_GRP1_CNT1                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[1])          /* 0x40588080 */
#define TCPWM0_GRP1_CNT2                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[2])          /* 0x40588100 */
#define TCPWM0_GRP1_CNT3                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[3])          /* 0x40588180 */
#define TCPWM0_GRP1_CNT4                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[4])          /* 0x40588200 */
#define TCPWM0_GRP1_CNT5                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[5])          /* 0x40588280 */
#define TCPWM0_GRP1_CNT6                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[6])          /* 0x40588300 */
#define TCPWM0_GRP1_CNT7                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[7])          /* 0x40588380 */
#define TCPWM0_GRP1_CNT8                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[8])          /* 0x40588400 */
#define TCPWM0_GRP1_CNT9                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[9])          /* 0x40588480 */
#define TCPWM0_GRP1_CNT10                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[10])         /* 0x40588500 */
#define TCPWM0_GRP1_CNT11                       ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[1].CNT[11])         /* 0x40588580 */
#define TCPWM0_GRP2_CNT0                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[0])          /* 0x40590000 */
#define TCPWM0_GRP2_CNT1                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[1])          /* 0x40590080 */
#define TCPWM0_GRP2_CNT2                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[2])          /* 0x40590100 */
#define TCPWM0_GRP2_CNT3                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[3])          /* 0x40590180 */
#define TCPWM0_GRP2_CNT4                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[4])          /* 0x40590200 */
#define TCPWM0_GRP2_CNT5                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[5])          /* 0x40590280 */
#define TCPWM0_GRP2_CNT6                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[6])          /* 0x40590300 */
#define TCPWM0_GRP2_CNT7                        ((volatile stc_TCPWM_GRP_CNT_t*) &TCPWM0->GRP[2].CNT[7])          /* 0x40590380 */

/*******************************************************************************
*                                     SCB
*******************************************************************************/

#define SCB0_BASE                               0x40600000UL
#define SCB1_BASE                               0x40610000UL
#define SCB2_BASE                               0x40620000UL
#define SCB3_BASE                               0x40630000UL
#define SCB4_BASE                               0x40640000UL
#define SCB5_BASE                               0x40650000UL
#define SCB6_BASE                               0x40660000UL
#define SCB7_BASE                               0x40670000UL
#define SCB8_BASE                               0x40680000UL
#define SCB9_BASE                               0x40690000UL
#define SCB10_BASE                              0x406A0000UL
#define SCB0                                    ((volatile stc_SCB_t*) SCB0_BASE)                                 /* 0x40600000 */
#define SCB1                                    ((volatile stc_SCB_t*) SCB1_BASE)                                 /* 0x40610000 */
#define SCB2                                    ((volatile stc_SCB_t*) SCB2_BASE)                                 /* 0x40620000 */
#define SCB3                                    ((volatile stc_SCB_t*) SCB3_BASE)                                 /* 0x40630000 */
#define SCB4                                    ((volatile stc_SCB_t*) SCB4_BASE)                                 /* 0x40640000 */
#define SCB5                                    ((volatile stc_SCB_t*) SCB5_BASE)                                 /* 0x40650000 */
#define SCB6                                    ((volatile stc_SCB_t*) SCB6_BASE)                                 /* 0x40660000 */
#define SCB7                                    ((volatile stc_SCB_t*) SCB7_BASE)                                 /* 0x40670000 */
#define SCB8                                    ((volatile stc_SCB_t*) SCB8_BASE)                                 /* 0x40680000 */
#define SCB9                                    ((volatile stc_SCB_t*) SCB9_BASE)                                 /* 0x40690000 */
#define SCB10                                   ((volatile stc_SCB_t*) SCB10_BASE)                                /* 0x406A0000 */

/*******************************************************************************
*                                     I2S
*******************************************************************************/

#define I2S0_BASE                               0x40800000UL
#define I2S1_BASE                               0x40801000UL
#define I2S2_BASE                               0x40802000UL
#define I2S0                                    ((volatile stc_I2S_t*) I2S0_BASE)                                 /* 0x40800000 */
#define I2S1                                    ((volatile stc_I2S_t*) I2S1_BASE)                                 /* 0x40801000 */
#define I2S2                                    ((volatile stc_I2S_t*) I2S2_BASE)                                 /* 0x40802000 */

/*******************************************************************************
*                                     PASS
*******************************************************************************/

#define PASS0_BASE                              0x40900000UL
#define PASS0                                   ((volatile stc_PASS_t*) PASS0_BASE)                               /* 0x40900000 */
#define PASS0_SAR0                              ((volatile stc_PASS_SAR_t*) &PASS0->SAR[0])                       /* 0x40900000 */
#define PASS0_SAR1                              ((volatile stc_PASS_SAR_t*) &PASS0->SAR[1])                       /* 0x40901000 */
#define PASS0_SAR2                              ((volatile stc_PASS_SAR_t*) &PASS0->SAR[2])                       /* 0x40902000 */
#define PASS0_SAR0_CH0                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[0])              /* 0x40900800 */
#define PASS0_SAR0_CH1                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[1])              /* 0x40900840 */
#define PASS0_SAR0_CH2                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[2])              /* 0x40900880 */
#define PASS0_SAR0_CH3                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[3])              /* 0x409008C0 */
#define PASS0_SAR0_CH4                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[4])              /* 0x40900900 */
#define PASS0_SAR0_CH5                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[5])              /* 0x40900940 */
#define PASS0_SAR0_CH6                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[6])              /* 0x40900980 */
#define PASS0_SAR0_CH7                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[7])              /* 0x409009C0 */
#define PASS0_SAR0_CH8                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[8])              /* 0x40900A00 */
#define PASS0_SAR0_CH9                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[9])              /* 0x40900A40 */
#define PASS0_SAR0_CH10                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[10])             /* 0x40900A80 */
#define PASS0_SAR0_CH11                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[11])             /* 0x40900AC0 */
#define PASS0_SAR0_CH12                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[12])             /* 0x40900B00 */
#define PASS0_SAR0_CH13                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[13])             /* 0x40900B40 */
#define PASS0_SAR0_CH14                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[14])             /* 0x40900B80 */
#define PASS0_SAR0_CH15                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[15])             /* 0x40900BC0 */
#define PASS0_SAR0_CH16                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[16])             /* 0x40900C00 */
#define PASS0_SAR0_CH17                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[17])             /* 0x40900C40 */
#define PASS0_SAR0_CH18                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[18])             /* 0x40900C80 */
#define PASS0_SAR0_CH19                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[19])             /* 0x40900CC0 */
#define PASS0_SAR0_CH20                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[20])             /* 0x40900D00 */
#define PASS0_SAR0_CH21                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[21])             /* 0x40900D40 */
#define PASS0_SAR0_CH22                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[22])             /* 0x40900D80 */
#define PASS0_SAR0_CH23                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[23])             /* 0x40900DC0 */
#define PASS0_SAR0_CH24                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[24])             /* 0x40900E00 */
#define PASS0_SAR0_CH25                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[25])             /* 0x40900E40 */
#define PASS0_SAR0_CH26                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[26])             /* 0x40900E80 */
#define PASS0_SAR0_CH27                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[27])             /* 0x40900EC0 */
#define PASS0_SAR0_CH28                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[28])             /* 0x40900F00 */
#define PASS0_SAR0_CH29                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[29])             /* 0x40900F40 */
#define PASS0_SAR0_CH30                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[30])             /* 0x40900F80 */
#define PASS0_SAR0_CH31                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[0].CH[31])             /* 0x40900FC0 */
#define PASS0_SAR1_CH0                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[0])              /* 0x40901800 */
#define PASS0_SAR1_CH1                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[1])              /* 0x40901840 */
#define PASS0_SAR1_CH2                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[2])              /* 0x40901880 */
#define PASS0_SAR1_CH3                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[3])              /* 0x409018C0 */
#define PASS0_SAR1_CH4                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[4])              /* 0x40901900 */
#define PASS0_SAR1_CH5                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[5])              /* 0x40901940 */
#define PASS0_SAR1_CH6                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[6])              /* 0x40901980 */
#define PASS0_SAR1_CH7                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[7])              /* 0x409019C0 */
#define PASS0_SAR1_CH8                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[8])              /* 0x40901A00 */
#define PASS0_SAR1_CH9                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[9])              /* 0x40901A40 */
#define PASS0_SAR1_CH10                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[10])             /* 0x40901A80 */
#define PASS0_SAR1_CH11                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[11])             /* 0x40901AC0 */
#define PASS0_SAR1_CH12                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[12])             /* 0x40901B00 */
#define PASS0_SAR1_CH13                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[13])             /* 0x40901B40 */
#define PASS0_SAR1_CH14                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[14])             /* 0x40901B80 */
#define PASS0_SAR1_CH15                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[15])             /* 0x40901BC0 */
#define PASS0_SAR1_CH16                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[16])             /* 0x40901C00 */
#define PASS0_SAR1_CH17                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[17])             /* 0x40901C40 */
#define PASS0_SAR1_CH18                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[18])             /* 0x40901C80 */
#define PASS0_SAR1_CH19                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[19])             /* 0x40901CC0 */
#define PASS0_SAR1_CH20                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[20])             /* 0x40901D00 */
#define PASS0_SAR1_CH21                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[21])             /* 0x40901D40 */
#define PASS0_SAR1_CH22                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[22])             /* 0x40901D80 */
#define PASS0_SAR1_CH23                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[23])             /* 0x40901DC0 */
#define PASS0_SAR1_CH24                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[24])             /* 0x40901E00 */
#define PASS0_SAR1_CH25                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[25])             /* 0x40901E40 */
#define PASS0_SAR1_CH26                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[26])             /* 0x40901E80 */
#define PASS0_SAR1_CH27                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[27])             /* 0x40901EC0 */
#define PASS0_SAR1_CH28                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[28])             /* 0x40901F00 */
#define PASS0_SAR1_CH29                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[29])             /* 0x40901F40 */
#define PASS0_SAR1_CH30                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[30])             /* 0x40901F80 */
#define PASS0_SAR1_CH31                         ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[1].CH[31])             /* 0x40901FC0 */
#define PASS0_SAR2_CH0                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[0])              /* 0x40902800 */
#define PASS0_SAR2_CH1                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[1])              /* 0x40902840 */
#define PASS0_SAR2_CH2                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[2])              /* 0x40902880 */
#define PASS0_SAR2_CH3                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[3])              /* 0x409028C0 */
#define PASS0_SAR2_CH4                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[4])              /* 0x40902900 */
#define PASS0_SAR2_CH5                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[5])              /* 0x40902940 */
#define PASS0_SAR2_CH6                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[6])              /* 0x40902980 */
#define PASS0_SAR2_CH7                          ((volatile stc_PASS_SAR_CH_t*) &PASS0->SAR[2].CH[7])              /* 0x409029C0 */
#define PASS0_EPASS_MMIO                        ((volatile stc_PASS_EPASS_MMIO_t*) &PASS0->EPASS_MMIO)            /* 0x409F0000 */

/** \} CYT4BB7CEE */

#endif /* _CYT4BB7CEE_H_ */


/* [] END OF FILE */
