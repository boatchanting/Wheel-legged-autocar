/***************************************************************************//**
* \file cy_interrupt_map_cm7_1.h
*
* \brief
* Provides the table with system interrupt handlers defines.
*
* \note
* Generator version: 1.5.0.1277
* Database revision: TVIIBH4M_CMR
*
********************************************************************************
* \copyright
* Copyright 2016-2019, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef _CY_INTERRUPT_MAP_CM7_1_H_
#define _CY_INTERRUPT_MAP_CM7_1_H_

#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#define CY_LINK_SYSTEM_IRQ_TABLE_TO_RAM

#define DEFAULT_HANDLER_NAME                            Cy_DefaultUserHandler

/** Macro to assign an IRQ to the default IRQ handler, implemented **/
#define INTERRUPTS_IRQ_DEFAULT_HANDLER                  DEFAULT_HANDLER_NAME

#define  CPUSS_INTERRUPTS_IPC_0_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ0                   
#define  CPUSS_INTERRUPTS_IPC_1_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ1                   
#define  CPUSS_INTERRUPTS_IPC_2_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ2                   
#define  CPUSS_INTERRUPTS_IPC_3_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ3                   
#define  CPUSS_INTERRUPTS_IPC_4_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ4                   
#define  CPUSS_INTERRUPTS_IPC_5_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ5                   
#define  CPUSS_INTERRUPTS_IPC_6_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ6                   
#define  CPUSS_INTERRUPTS_IPC_7_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ7                   
#define  CPUSS_INTERRUPTS_FAULT_0_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ8                   
#define  CPUSS_INTERRUPTS_FAULT_1_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ9                   
#define  CPUSS_INTERRUPTS_FAULT_2_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ10                  
#define  CPUSS_INTERRUPTS_FAULT_3_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ11                  
#define  SRSS_INTERRUPT_BACKUP_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ12                  
#define  SRSS_INTERRUPT_MCWDT_0_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ13                  
#define  SRSS_INTERRUPT_MCWDT_1_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ14                  
#define  SRSS_INTERRUPT_MCWDT_2_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ15                  
#define  SRSS_INTERRUPT_WDT_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ16                  
#define  SRSS_INTERRUPT_HANDLER                        INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ17                  
#define  SCB_0_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ18                  
#define  EVTGEN_0_INTERRUPT_DPSLP_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ19                  
#define  IOSS_INTERRUPT_VDD_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ20                  
#define  IOSS_INTERRUPT_GPIO_DPSLP_HANDLER             INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ21                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_0_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ22                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_1_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ23                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_2_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ24                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_3_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ25                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_4_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ26                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_5_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ27                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_6_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ28                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_7_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ29                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_8_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ30                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_9_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ31                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_10_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ32                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_11_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ33                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_12_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ34                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_13_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ35                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_14_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ36                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_15_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ37                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_16_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ38                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_17_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ39                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_18_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ40                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_19_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ41                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_20_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ42                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_21_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ43                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_22_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ44                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_23_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ45                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_28_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ46                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_29_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ47                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_30_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ48                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_31_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ49                  
#define  IOSS_INTERRUPTS_GPIO_DPSLP_32_HANDLER         INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ50                  
#define  IOSS_INTERRUPT_GPIO_ACT_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ51                  
#define  IOSS_INTERRUPTS_GPIO_ACT_24_HANDLER           INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ52                  
#define  IOSS_INTERRUPTS_GPIO_ACT_25_HANDLER           INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ53                  
#define  IOSS_INTERRUPTS_GPIO_ACT_26_HANDLER           INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ54                  
#define  IOSS_INTERRUPTS_GPIO_ACT_27_HANDLER           INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ55                  
#define  CPUSS_INTERRUPT_CRYPTO_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ56                  
#define  CPUSS_INTERRUPT_FM_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ57                  
#define  CPUSS_INTERRUPTS_CM7_0_FP_HANDLER             INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ58                  
#define  CPUSS_INTERRUPTS_CM7_1_FP_HANDLER             INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ59                  
#define  CPUSS_INTERRUPTS_CM0_CTI_0_HANDLER            INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ60                  
#define  CPUSS_INTERRUPTS_CM0_CTI_1_HANDLER            INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ61                  
#define  CPUSS_INTERRUPTS_CM7_0_CTI_0_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ62                  
#define  CPUSS_INTERRUPTS_CM7_0_CTI_1_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ63                  
#define  CPUSS_INTERRUPTS_CM7_1_CTI_0_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ64                  
#define  CPUSS_INTERRUPTS_CM7_1_CTI_1_HANDLER          INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ65                  
#define  EVTGEN_0_INTERRUPT_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ66                  
#define  CANFD_0_INTERRUPT0_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ67                  
#define  CANFD_0_INTERRUPT1_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ68                  
#define  CANFD_1_INTERRUPT0_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ69                  
#define  CANFD_1_INTERRUPT1_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ70                  
#define  CANFD_0_INTERRUPTS0_0_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ71                  
#define  CANFD_0_INTERRUPTS0_1_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ72                  
#define  CANFD_0_INTERRUPTS0_2_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ73                  
#define  CANFD_0_INTERRUPTS0_3_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ74                  
#define  CANFD_0_INTERRUPTS1_0_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ75                  
#define  CANFD_0_INTERRUPTS1_1_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ76                  
#define  CANFD_0_INTERRUPTS1_2_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ77                  
#define  CANFD_0_INTERRUPTS1_3_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ78                  
#define  CANFD_1_INTERRUPTS0_0_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ79                  
#define  CANFD_1_INTERRUPTS0_1_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ80                  
#define  CANFD_1_INTERRUPTS0_2_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ81                  
#define  CANFD_1_INTERRUPTS0_3_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ82                  
#define  CANFD_1_INTERRUPTS1_0_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ83                  
#define  CANFD_1_INTERRUPTS1_1_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ84                  
#define  CANFD_1_INTERRUPTS1_2_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ85                  
#define  CANFD_1_INTERRUPTS1_3_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ86                  
#define  LIN_0_INTERRUPTS_0_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ87                  
#define  LIN_0_INTERRUPTS_1_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ88                  
#define  LIN_0_INTERRUPTS_2_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ89                  
#define  LIN_0_INTERRUPTS_3_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ90                  
#define  LIN_0_INTERRUPTS_4_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ91                  
#define  LIN_0_INTERRUPTS_5_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ92                  
#define  LIN_0_INTERRUPTS_6_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ93                  
#define  LIN_0_INTERRUPTS_7_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ94                  
#define  LIN_0_INTERRUPTS_8_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ95                  
#define  LIN_0_INTERRUPTS_9_HANDLER                    INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ96                  
#define  LIN_0_INTERRUPTS_10_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ97                  
#define  LIN_0_INTERRUPTS_11_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ98                  
#define  LIN_0_INTERRUPTS_12_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ99                  
#define  LIN_0_INTERRUPTS_13_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ100                 
#define  LIN_0_INTERRUPTS_14_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ101                 
#define  LIN_0_INTERRUPTS_15_HANDLER                   INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ102                 
#define  SCB_1_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ103                 
#define  SCB_2_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ104                 
#define  SCB_3_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ105                 
#define  SCB_4_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ106                 
#define  SCB_5_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ107                 
#define  SCB_6_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ108                 
#define  SCB_7_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ109                 
#define  SCB_8_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ110                 
#define  SCB_9_INTERRUPT_HANDLER                       INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ111                 
#define  SCB_10_INTERRUPT_HANDLER                      INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ112                 
#define  PASS_0_INTERRUPTS_SAR_0_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ113                 
#define  PASS_0_INTERRUPTS_SAR_1_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ114                 
#define  PASS_0_INTERRUPTS_SAR_2_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ115                 
#define  PASS_0_INTERRUPTS_SAR_3_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ116                 
#define  PASS_0_INTERRUPTS_SAR_4_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ117                 
#define  PASS_0_INTERRUPTS_SAR_5_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ118                 
#define  PASS_0_INTERRUPTS_SAR_6_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ119                 
#define  PASS_0_INTERRUPTS_SAR_7_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ120                 
#define  PASS_0_INTERRUPTS_SAR_8_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ121                 
#define  PASS_0_INTERRUPTS_SAR_9_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ122                 
#define  PASS_0_INTERRUPTS_SAR_10_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ123                 
#define  PASS_0_INTERRUPTS_SAR_11_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ124                 
#define  PASS_0_INTERRUPTS_SAR_12_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ125                 
#define  PASS_0_INTERRUPTS_SAR_13_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ126                 
#define  PASS_0_INTERRUPTS_SAR_14_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ127                 
#define  PASS_0_INTERRUPTS_SAR_15_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ128                 
#define  PASS_0_INTERRUPTS_SAR_16_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ129                 
#define  PASS_0_INTERRUPTS_SAR_17_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ130                 
#define  PASS_0_INTERRUPTS_SAR_18_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ131                 
#define  PASS_0_INTERRUPTS_SAR_19_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ132                 
#define  PASS_0_INTERRUPTS_SAR_20_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ133                 
#define  PASS_0_INTERRUPTS_SAR_21_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ134                 
#define  PASS_0_INTERRUPTS_SAR_22_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ135                 
#define  PASS_0_INTERRUPTS_SAR_23_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ136                 
#define  PASS_0_INTERRUPTS_SAR_24_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ137                 
#define  PASS_0_INTERRUPTS_SAR_25_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ138                 
#define  PASS_0_INTERRUPTS_SAR_26_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ139                 
#define  PASS_0_INTERRUPTS_SAR_27_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ140                 
#define  PASS_0_INTERRUPTS_SAR_28_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ141                 
#define  PASS_0_INTERRUPTS_SAR_29_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ142                 
#define  PASS_0_INTERRUPTS_SAR_30_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ143                 
#define  PASS_0_INTERRUPTS_SAR_31_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ144                 
#define  PASS_0_INTERRUPTS_SAR_32_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ145                 
#define  PASS_0_INTERRUPTS_SAR_33_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ146                 
#define  PASS_0_INTERRUPTS_SAR_34_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ147                 
#define  PASS_0_INTERRUPTS_SAR_35_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ148                 
#define  PASS_0_INTERRUPTS_SAR_36_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ149                 
#define  PASS_0_INTERRUPTS_SAR_37_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ150                 
#define  PASS_0_INTERRUPTS_SAR_38_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ151                 
#define  PASS_0_INTERRUPTS_SAR_39_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ152                 
#define  PASS_0_INTERRUPTS_SAR_40_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ153                 
#define  PASS_0_INTERRUPTS_SAR_41_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ154                 
#define  PASS_0_INTERRUPTS_SAR_42_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ155                 
#define  PASS_0_INTERRUPTS_SAR_43_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ156                 
#define  PASS_0_INTERRUPTS_SAR_44_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ157                 
#define  PASS_0_INTERRUPTS_SAR_45_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ158                 
#define  PASS_0_INTERRUPTS_SAR_46_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ159                 
#define  PASS_0_INTERRUPTS_SAR_47_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ160                 
#define  PASS_0_INTERRUPTS_SAR_48_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ161                 
#define  PASS_0_INTERRUPTS_SAR_49_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ162                 
#define  PASS_0_INTERRUPTS_SAR_50_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ163                 
#define  PASS_0_INTERRUPTS_SAR_51_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ164                 
#define  PASS_0_INTERRUPTS_SAR_52_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ165                 
#define  PASS_0_INTERRUPTS_SAR_53_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ166                 
#define  PASS_0_INTERRUPTS_SAR_54_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ167                 
#define  PASS_0_INTERRUPTS_SAR_55_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ168                 
#define  PASS_0_INTERRUPTS_SAR_56_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ169                 
#define  PASS_0_INTERRUPTS_SAR_57_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ170                 
#define  PASS_0_INTERRUPTS_SAR_58_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ171                 
#define  PASS_0_INTERRUPTS_SAR_59_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ172                 
#define  PASS_0_INTERRUPTS_SAR_60_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ173                 
#define  PASS_0_INTERRUPTS_SAR_61_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ174                 
#define  PASS_0_INTERRUPTS_SAR_62_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ175                 
#define  PASS_0_INTERRUPTS_SAR_63_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ176                 
#define  PASS_0_INTERRUPTS_SAR_64_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ177                 
#define  PASS_0_INTERRUPTS_SAR_65_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ178                 
#define  PASS_0_INTERRUPTS_SAR_66_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ179                 
#define  PASS_0_INTERRUPTS_SAR_67_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ180                 
#define  PASS_0_INTERRUPTS_SAR_68_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ181                 
#define  PASS_0_INTERRUPTS_SAR_69_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ182                 
#define  PASS_0_INTERRUPTS_SAR_70_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ183                 
#define  PASS_0_INTERRUPTS_SAR_71_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ184                 
#define  CPUSS_INTERRUPTS_DMAC_0_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ185                 
#define  CPUSS_INTERRUPTS_DMAC_1_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ186                 
#define  CPUSS_INTERRUPTS_DMAC_2_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ187                 
#define  CPUSS_INTERRUPTS_DMAC_3_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ188                 
#define  CPUSS_INTERRUPTS_DMAC_4_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ189                 
#define  CPUSS_INTERRUPTS_DMAC_5_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ190                 
#define  CPUSS_INTERRUPTS_DMAC_6_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ191                 
#define  CPUSS_INTERRUPTS_DMAC_7_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ192                 
#define  CPUSS_INTERRUPTS_DW0_0_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ193                 
#define  CPUSS_INTERRUPTS_DW0_1_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ194                 
#define  CPUSS_INTERRUPTS_DW0_2_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ195                 
#define  CPUSS_INTERRUPTS_DW0_3_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ196                 
#define  CPUSS_INTERRUPTS_DW0_4_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ197                 
#define  CPUSS_INTERRUPTS_DW0_5_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ198                 
#define  CPUSS_INTERRUPTS_DW0_6_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ199                 
#define  CPUSS_INTERRUPTS_DW0_7_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ200                 
#define  CPUSS_INTERRUPTS_DW0_8_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ201                 
#define  CPUSS_INTERRUPTS_DW0_9_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ202                 
#define  CPUSS_INTERRUPTS_DW0_10_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ203                 
#define  CPUSS_INTERRUPTS_DW0_11_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ204                 
#define  CPUSS_INTERRUPTS_DW0_12_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ205                 
#define  CPUSS_INTERRUPTS_DW0_13_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ206                 
#define  CPUSS_INTERRUPTS_DW0_14_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ207                 
#define  CPUSS_INTERRUPTS_DW0_15_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ208                 
#define  CPUSS_INTERRUPTS_DW0_16_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ209                 
#define  CPUSS_INTERRUPTS_DW0_17_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ210                 
#define  CPUSS_INTERRUPTS_DW0_18_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ211                 
#define  CPUSS_INTERRUPTS_DW0_19_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ212                 
#define  CPUSS_INTERRUPTS_DW0_20_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ213                 
#define  CPUSS_INTERRUPTS_DW0_21_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ214                 
#define  CPUSS_INTERRUPTS_DW0_22_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ215                 
#define  CPUSS_INTERRUPTS_DW0_23_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ216                 
#define  CPUSS_INTERRUPTS_DW0_24_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ217                 
#define  CPUSS_INTERRUPTS_DW0_25_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ218                 
#define  CPUSS_INTERRUPTS_DW0_26_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ219                 
#define  CPUSS_INTERRUPTS_DW0_27_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ220                 
#define  CPUSS_INTERRUPTS_DW0_28_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ221                 
#define  CPUSS_INTERRUPTS_DW0_29_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ222                 
#define  CPUSS_INTERRUPTS_DW0_30_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ223                 
#define  CPUSS_INTERRUPTS_DW0_31_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ224                 
#define  CPUSS_INTERRUPTS_DW0_32_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ225                 
#define  CPUSS_INTERRUPTS_DW0_33_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ226                 
#define  CPUSS_INTERRUPTS_DW0_34_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ227                 
#define  CPUSS_INTERRUPTS_DW0_35_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ228                 
#define  CPUSS_INTERRUPTS_DW0_36_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ229                 
#define  CPUSS_INTERRUPTS_DW0_37_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ230                 
#define  CPUSS_INTERRUPTS_DW0_38_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ231                 
#define  CPUSS_INTERRUPTS_DW0_39_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ232                 
#define  CPUSS_INTERRUPTS_DW0_40_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ233                 
#define  CPUSS_INTERRUPTS_DW0_41_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ234                 
#define  CPUSS_INTERRUPTS_DW0_42_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ235                 
#define  CPUSS_INTERRUPTS_DW0_43_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ236                 
#define  CPUSS_INTERRUPTS_DW0_44_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ237                 
#define  CPUSS_INTERRUPTS_DW0_45_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ238                 
#define  CPUSS_INTERRUPTS_DW0_46_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ239                 
#define  CPUSS_INTERRUPTS_DW0_47_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ240                 
#define  CPUSS_INTERRUPTS_DW0_48_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ241                 
#define  CPUSS_INTERRUPTS_DW0_49_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ242                 
#define  CPUSS_INTERRUPTS_DW0_50_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ243                 
#define  CPUSS_INTERRUPTS_DW0_51_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ244                 
#define  CPUSS_INTERRUPTS_DW0_52_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ245                 
#define  CPUSS_INTERRUPTS_DW0_53_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ246                 
#define  CPUSS_INTERRUPTS_DW0_54_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ247                 
#define  CPUSS_INTERRUPTS_DW0_55_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ248                 
#define  CPUSS_INTERRUPTS_DW0_56_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ249                 
#define  CPUSS_INTERRUPTS_DW0_57_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ250                 
#define  CPUSS_INTERRUPTS_DW0_58_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ251                 
#define  CPUSS_INTERRUPTS_DW0_59_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ252                 
#define  CPUSS_INTERRUPTS_DW0_60_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ253                 
#define  CPUSS_INTERRUPTS_DW0_61_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ254                 
#define  CPUSS_INTERRUPTS_DW0_62_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ255                 
#define  CPUSS_INTERRUPTS_DW0_63_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ256                 
#define  CPUSS_INTERRUPTS_DW0_64_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ257                 
#define  CPUSS_INTERRUPTS_DW0_65_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ258                 
#define  CPUSS_INTERRUPTS_DW0_66_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ259                 
#define  CPUSS_INTERRUPTS_DW0_67_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ260                 
#define  CPUSS_INTERRUPTS_DW0_68_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ261                 
#define  CPUSS_INTERRUPTS_DW0_69_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ262                 
#define  CPUSS_INTERRUPTS_DW0_70_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ263                 
#define  CPUSS_INTERRUPTS_DW0_71_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ264                 
#define  CPUSS_INTERRUPTS_DW0_72_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ265                 
#define  CPUSS_INTERRUPTS_DW0_73_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ266                 
#define  CPUSS_INTERRUPTS_DW0_74_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ267                 
#define  CPUSS_INTERRUPTS_DW0_75_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ268                 
#define  CPUSS_INTERRUPTS_DW0_76_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ269                 
#define  CPUSS_INTERRUPTS_DW0_77_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ270                 
#define  CPUSS_INTERRUPTS_DW0_78_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ271                 
#define  CPUSS_INTERRUPTS_DW0_79_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ272                 
#define  CPUSS_INTERRUPTS_DW0_80_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ273                 
#define  CPUSS_INTERRUPTS_DW0_81_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ274                 
#define  CPUSS_INTERRUPTS_DW0_82_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ275                 
#define  CPUSS_INTERRUPTS_DW0_83_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ276                 
#define  CPUSS_INTERRUPTS_DW0_84_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ277                 
#define  CPUSS_INTERRUPTS_DW0_85_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ278                 
#define  CPUSS_INTERRUPTS_DW0_86_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ279                 
#define  CPUSS_INTERRUPTS_DW0_87_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ280                 
#define  CPUSS_INTERRUPTS_DW0_88_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ281                 
#define  CPUSS_INTERRUPTS_DW0_89_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ282                 
#define  CPUSS_INTERRUPTS_DW0_90_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ283                 
#define  CPUSS_INTERRUPTS_DW0_91_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ284                 
#define  CPUSS_INTERRUPTS_DW0_92_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ285                 
#define  CPUSS_INTERRUPTS_DW0_93_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ286                 
#define  CPUSS_INTERRUPTS_DW0_94_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ287                 
#define  CPUSS_INTERRUPTS_DW0_95_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ288                 
#define  CPUSS_INTERRUPTS_DW0_96_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ289                 
#define  CPUSS_INTERRUPTS_DW0_97_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ290                 
#define  CPUSS_INTERRUPTS_DW0_98_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ291                 
#define  CPUSS_INTERRUPTS_DW0_99_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ292                 
#define  CPUSS_INTERRUPTS_DW1_0_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ293                 
#define  CPUSS_INTERRUPTS_DW1_1_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ294                 
#define  CPUSS_INTERRUPTS_DW1_2_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ295                 
#define  CPUSS_INTERRUPTS_DW1_3_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ296                 
#define  CPUSS_INTERRUPTS_DW1_4_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ297                 
#define  CPUSS_INTERRUPTS_DW1_5_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ298                 
#define  CPUSS_INTERRUPTS_DW1_6_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ299                 
#define  CPUSS_INTERRUPTS_DW1_7_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ300                 
#define  CPUSS_INTERRUPTS_DW1_8_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ301                 
#define  CPUSS_INTERRUPTS_DW1_9_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ302                 
#define  CPUSS_INTERRUPTS_DW1_10_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ303                 
#define  CPUSS_INTERRUPTS_DW1_11_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ304                 
#define  CPUSS_INTERRUPTS_DW1_12_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ305                 
#define  CPUSS_INTERRUPTS_DW1_13_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ306                 
#define  CPUSS_INTERRUPTS_DW1_14_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ307                 
#define  CPUSS_INTERRUPTS_DW1_15_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ308                 
#define  CPUSS_INTERRUPTS_DW1_16_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ309                 
#define  CPUSS_INTERRUPTS_DW1_17_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ310                 
#define  CPUSS_INTERRUPTS_DW1_18_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ311                 
#define  CPUSS_INTERRUPTS_DW1_19_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ312                 
#define  CPUSS_INTERRUPTS_DW1_20_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ313                 
#define  CPUSS_INTERRUPTS_DW1_21_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ314                 
#define  CPUSS_INTERRUPTS_DW1_22_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ315                 
#define  CPUSS_INTERRUPTS_DW1_23_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ316                 
#define  CPUSS_INTERRUPTS_DW1_24_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ317                 
#define  CPUSS_INTERRUPTS_DW1_25_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ318                 
#define  CPUSS_INTERRUPTS_DW1_26_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ319                 
#define  CPUSS_INTERRUPTS_DW1_27_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ320                 
#define  CPUSS_INTERRUPTS_DW1_28_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ321                 
#define  CPUSS_INTERRUPTS_DW1_29_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ322                 
#define  CPUSS_INTERRUPTS_DW1_30_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ323                 
#define  CPUSS_INTERRUPTS_DW1_31_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ324                 
#define  CPUSS_INTERRUPTS_DW1_32_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ325                 
#define  CPUSS_INTERRUPTS_DW1_33_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ326                 
#define  CPUSS_INTERRUPTS_DW1_34_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ327                 
#define  CPUSS_INTERRUPTS_DW1_35_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ328                 
#define  CPUSS_INTERRUPTS_DW1_36_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ329                 
#define  CPUSS_INTERRUPTS_DW1_37_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ330                 
#define  CPUSS_INTERRUPTS_DW1_38_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ331                 
#define  CPUSS_INTERRUPTS_DW1_39_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ332                 
#define  CPUSS_INTERRUPTS_DW1_40_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ333                 
#define  CPUSS_INTERRUPTS_DW1_41_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ334                 
#define  CPUSS_INTERRUPTS_DW1_42_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ335                 
#define  CPUSS_INTERRUPTS_DW1_43_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ336                 
#define  CPUSS_INTERRUPTS_DW1_44_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ337                 
#define  CPUSS_INTERRUPTS_DW1_45_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ338                 
#define  CPUSS_INTERRUPTS_DW1_46_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ339                 
#define  CPUSS_INTERRUPTS_DW1_47_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ340                 
#define  CPUSS_INTERRUPTS_DW1_48_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ341                 
#define  CPUSS_INTERRUPTS_DW1_49_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ342                 
#define  CPUSS_INTERRUPTS_DW1_50_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ343                 
#define  CPUSS_INTERRUPTS_DW1_51_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ344                 
#define  CPUSS_INTERRUPTS_DW1_52_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ345                 
#define  CPUSS_INTERRUPTS_DW1_53_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ346                 
#define  CPUSS_INTERRUPTS_DW1_54_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ347                 
#define  CPUSS_INTERRUPTS_DW1_55_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ348                 
#define  CPUSS_INTERRUPTS_DW1_56_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ349                 
#define  CPUSS_INTERRUPTS_DW1_57_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ350                 
#define  TCPWM_0_INTERRUPTS_0_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ351                 
#define  TCPWM_0_INTERRUPTS_1_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ352                 
#define  TCPWM_0_INTERRUPTS_2_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ353                 
#define  TCPWM_0_INTERRUPTS_3_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ354                 
#define  TCPWM_0_INTERRUPTS_4_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ355                 
#define  TCPWM_0_INTERRUPTS_5_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ356                 
#define  TCPWM_0_INTERRUPTS_6_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ357                 
#define  TCPWM_0_INTERRUPTS_7_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ358                 
#define  TCPWM_0_INTERRUPTS_8_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ359                 
#define  TCPWM_0_INTERRUPTS_9_HANDLER                  INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ360                 
#define  TCPWM_0_INTERRUPTS_10_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ361                 
#define  TCPWM_0_INTERRUPTS_11_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ362                 
#define  TCPWM_0_INTERRUPTS_12_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ363                 
#define  TCPWM_0_INTERRUPTS_13_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ364                 
#define  TCPWM_0_INTERRUPTS_14_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ365                 
#define  TCPWM_0_INTERRUPTS_15_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ366                 
#define  TCPWM_0_INTERRUPTS_16_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ367                 
#define  TCPWM_0_INTERRUPTS_17_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ368                 
#define  TCPWM_0_INTERRUPTS_18_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ369                 
#define  TCPWM_0_INTERRUPTS_19_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ370                 
#define  TCPWM_0_INTERRUPTS_20_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ371                 
#define  TCPWM_0_INTERRUPTS_21_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ372                 
#define  TCPWM_0_INTERRUPTS_22_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ373                 
#define  TCPWM_0_INTERRUPTS_23_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ374                 
#define  TCPWM_0_INTERRUPTS_24_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ375                 
#define  TCPWM_0_INTERRUPTS_25_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ376                 
#define  TCPWM_0_INTERRUPTS_26_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ377                 
#define  TCPWM_0_INTERRUPTS_27_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ378                 
#define  TCPWM_0_INTERRUPTS_28_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ379                 
#define  TCPWM_0_INTERRUPTS_29_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ380                 
#define  TCPWM_0_INTERRUPTS_30_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ381                 
#define  TCPWM_0_INTERRUPTS_31_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ382                 
#define  TCPWM_0_INTERRUPTS_32_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ383                 
#define  TCPWM_0_INTERRUPTS_33_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ384                 
#define  TCPWM_0_INTERRUPTS_34_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ385                 
#define  TCPWM_0_INTERRUPTS_35_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ386                 
#define  TCPWM_0_INTERRUPTS_36_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ387                 
#define  TCPWM_0_INTERRUPTS_37_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ388                 
#define  TCPWM_0_INTERRUPTS_38_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ389                 
#define  TCPWM_0_INTERRUPTS_39_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ390                 
#define  TCPWM_0_INTERRUPTS_40_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ391                 
#define  TCPWM_0_INTERRUPTS_41_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ392                 
#define  TCPWM_0_INTERRUPTS_42_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ393                 
#define  TCPWM_0_INTERRUPTS_43_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ394                 
#define  TCPWM_0_INTERRUPTS_44_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ395                 
#define  TCPWM_0_INTERRUPTS_45_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ396                 
#define  TCPWM_0_INTERRUPTS_46_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ397                 
#define  TCPWM_0_INTERRUPTS_47_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ398                 
#define  TCPWM_0_INTERRUPTS_48_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ399                 
#define  TCPWM_0_INTERRUPTS_49_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ400                 
#define  TCPWM_0_INTERRUPTS_50_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ401                 
#define  TCPWM_0_INTERRUPTS_51_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ402                 
#define  TCPWM_0_INTERRUPTS_52_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ403                 
#define  TCPWM_0_INTERRUPTS_53_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ404                 
#define  TCPWM_0_INTERRUPTS_54_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ405                 
#define  TCPWM_0_INTERRUPTS_55_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ406                 
#define  TCPWM_0_INTERRUPTS_56_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ407                 
#define  TCPWM_0_INTERRUPTS_57_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ408                 
#define  TCPWM_0_INTERRUPTS_58_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ409                 
#define  TCPWM_0_INTERRUPTS_59_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ410                 
#define  TCPWM_0_INTERRUPTS_60_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ411                 
#define  TCPWM_0_INTERRUPTS_61_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ412                 
#define  TCPWM_0_INTERRUPTS_62_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ413                 
#define  TCPWM_0_INTERRUPTS_256_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ414                 
#define  TCPWM_0_INTERRUPTS_257_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ415                 
#define  TCPWM_0_INTERRUPTS_258_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ416                 
#define  TCPWM_0_INTERRUPTS_259_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ417                 
#define  TCPWM_0_INTERRUPTS_260_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ418                 
#define  TCPWM_0_INTERRUPTS_261_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ419                 
#define  TCPWM_0_INTERRUPTS_262_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ420                 
#define  TCPWM_0_INTERRUPTS_263_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ421                 
#define  TCPWM_0_INTERRUPTS_264_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ422                 
#define  TCPWM_0_INTERRUPTS_265_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ423                 
#define  TCPWM_0_INTERRUPTS_266_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ424                 
#define  TCPWM_0_INTERRUPTS_267_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ425                 
#define  TCPWM_0_INTERRUPTS_512_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ426                 
#define  TCPWM_0_INTERRUPTS_513_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ427                 
#define  TCPWM_0_INTERRUPTS_514_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ428                 
#define  TCPWM_0_INTERRUPTS_515_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ429                 
#define  TCPWM_0_INTERRUPTS_516_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ430                 
#define  TCPWM_0_INTERRUPTS_517_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ431                 
#define  TCPWM_0_INTERRUPTS_518_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ432                 
#define  TCPWM_0_INTERRUPTS_519_HANDLER                INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ433                 
#define  SMIF_0_INTERRUPT_HANDLER                      INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ434                 
#define  ETH_0_INTERRUPT_ETH_0_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ435                 
#define  ETH_0_INTERRUPT_ETH_2_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ436                 
#define  ETH_0_INTERRUPT_ETH_1_HANDLER                 INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ437                 
#define  SDHC_0_INTERRUPT_GENERAL_HANDLER              INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ438                 
#define  SDHC_0_INTERRUPT_WAKEUP_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ439                 
#define  AUDIOSS_0_INTERRUPT_I2S_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ440                 
#define  AUDIOSS_1_INTERRUPT_I2S_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ441                 
#define  AUDIOSS_2_INTERRUPT_I2S_HANDLER               INTERRUPTS_IRQ_DEFAULT_HANDLER          // IRQ442                 

#undef INTERRUPTS_IRQ_DEFAULT_HANDLER
#define INTERRUPTS_IRQ_DEFAULT_HANDLER 0xFFFFFFFF

/* suppress warning "zero used for undefined preprocessing identifier" */
#ifdef __ghs__
    #pragma ghs nowarning 193
#endif
#ifdef __ICCARM__
    #pragma diag_suppress=Pe193
#endif

#if (CPUSS_INTERRUPTS_IPC_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ0                   
     extern void CPUSS_INTERRUPTS_IPC_0_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ1                   
     extern void CPUSS_INTERRUPTS_IPC_1_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ2                   
     extern void CPUSS_INTERRUPTS_IPC_2_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ3                   
     extern void CPUSS_INTERRUPTS_IPC_3_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ4                   
     extern void CPUSS_INTERRUPTS_IPC_4_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ5                   
     extern void CPUSS_INTERRUPTS_IPC_5_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ6                   
     extern void CPUSS_INTERRUPTS_IPC_6_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_IPC_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ7                   
     extern void CPUSS_INTERRUPTS_IPC_7_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_FAULT_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ8                   
     extern void CPUSS_INTERRUPTS_FAULT_0_HANDLER(void);                             
#endif
#if (CPUSS_INTERRUPTS_FAULT_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ9                   
     extern void CPUSS_INTERRUPTS_FAULT_1_HANDLER(void);                             
#endif
#if (CPUSS_INTERRUPTS_FAULT_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ10                  
     extern void CPUSS_INTERRUPTS_FAULT_2_HANDLER(void);                             
#endif
#if (CPUSS_INTERRUPTS_FAULT_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ11                  
     extern void CPUSS_INTERRUPTS_FAULT_3_HANDLER(void);                             
#endif
#if (SRSS_INTERRUPT_BACKUP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ12                  
     extern void SRSS_INTERRUPT_BACKUP_HANDLER(void);                                
#endif
#if (SRSS_INTERRUPT_MCWDT_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ13                  
     extern void SRSS_INTERRUPT_MCWDT_0_HANDLER(void);                               
#endif
#if (SRSS_INTERRUPT_MCWDT_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ14                  
     extern void SRSS_INTERRUPT_MCWDT_1_HANDLER(void);                               
#endif
#if (SRSS_INTERRUPT_MCWDT_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ15                  
     extern void SRSS_INTERRUPT_MCWDT_2_HANDLER(void);                               
#endif
#if (SRSS_INTERRUPT_WDT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ16                  
     extern void SRSS_INTERRUPT_WDT_HANDLER(void);                                   
#endif
#if (SRSS_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                  // IRQ17                  
     extern void SRSS_INTERRUPT_HANDLER(void);                                       
#endif
#if (SCB_0_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ18                  
     extern void SCB_0_INTERRUPT_HANDLER(void);                                      
#endif
#if (EVTGEN_0_INTERRUPT_DPSLP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ19                  
     extern void EVTGEN_0_INTERRUPT_DPSLP_HANDLER(void);                             
#endif
#if (IOSS_INTERRUPT_VDD_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ20                  
     extern void IOSS_INTERRUPT_VDD_HANDLER(void);                                   
#endif
#if (IOSS_INTERRUPT_GPIO_DPSLP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)       // IRQ21                  
     extern void IOSS_INTERRUPT_GPIO_DPSLP_HANDLER(void);                            
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ22                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_0_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ23                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_1_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ24                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_2_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ25                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_3_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ26                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_4_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ27                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_5_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ28                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_6_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ29                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_7_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ30                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_8_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ31                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_9_HANDLER(void);                         
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ32                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_10_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ33                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_11_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ34                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_12_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ35                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_13_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ36                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_14_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ37                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_15_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_16_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ38                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_16_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_17_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ39                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_17_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_18_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ40                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_18_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_19_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ41                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_19_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_20_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ42                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_20_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_21_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ43                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_21_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_22_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ44                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_22_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_23_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ45                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_23_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_28_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ46                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_28_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_29_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ47                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_29_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_30_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ48                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_30_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_31_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ49                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_31_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPTS_GPIO_DPSLP_32_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)   // IRQ50                  
     extern void IOSS_INTERRUPTS_GPIO_DPSLP_32_HANDLER(void);                        
#endif
#if (IOSS_INTERRUPT_GPIO_ACT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ51                  
     extern void IOSS_INTERRUPT_GPIO_ACT_HANDLER(void);                              
#endif
#if (IOSS_INTERRUPTS_GPIO_ACT_24_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)     // IRQ52                  
     extern void IOSS_INTERRUPTS_GPIO_ACT_24_HANDLER(void);                          
#endif
#if (IOSS_INTERRUPTS_GPIO_ACT_25_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)     // IRQ53                  
     extern void IOSS_INTERRUPTS_GPIO_ACT_25_HANDLER(void);                          
#endif
#if (IOSS_INTERRUPTS_GPIO_ACT_26_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)     // IRQ54                  
     extern void IOSS_INTERRUPTS_GPIO_ACT_26_HANDLER(void);                          
#endif
#if (IOSS_INTERRUPTS_GPIO_ACT_27_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)     // IRQ55                  
     extern void IOSS_INTERRUPTS_GPIO_ACT_27_HANDLER(void);                          
#endif
#if (CPUSS_INTERRUPT_CRYPTO_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ56                  
     extern void CPUSS_INTERRUPT_CRYPTO_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPT_FM_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ57                  
     extern void CPUSS_INTERRUPT_FM_HANDLER(void);                                   
#endif
#if (CPUSS_INTERRUPTS_CM7_0_FP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)       // IRQ58                  
     extern void CPUSS_INTERRUPTS_CM7_0_FP_HANDLER(void);                            
#endif
#if (CPUSS_INTERRUPTS_CM7_1_FP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)       // IRQ59                  
     extern void CPUSS_INTERRUPTS_CM7_1_FP_HANDLER(void);                            
#endif
#if (CPUSS_INTERRUPTS_CM0_CTI_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)      // IRQ60                  
     extern void CPUSS_INTERRUPTS_CM0_CTI_0_HANDLER(void);                           
#endif
#if (CPUSS_INTERRUPTS_CM0_CTI_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)      // IRQ61                  
     extern void CPUSS_INTERRUPTS_CM0_CTI_1_HANDLER(void);                           
#endif
#if (CPUSS_INTERRUPTS_CM7_0_CTI_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ62                  
     extern void CPUSS_INTERRUPTS_CM7_0_CTI_0_HANDLER(void);                         
#endif
#if (CPUSS_INTERRUPTS_CM7_0_CTI_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ63                  
     extern void CPUSS_INTERRUPTS_CM7_0_CTI_1_HANDLER(void);                         
#endif
#if (CPUSS_INTERRUPTS_CM7_1_CTI_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ64                  
     extern void CPUSS_INTERRUPTS_CM7_1_CTI_0_HANDLER(void);                         
#endif
#if (CPUSS_INTERRUPTS_CM7_1_CTI_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)    // IRQ65                  
     extern void CPUSS_INTERRUPTS_CM7_1_CTI_1_HANDLER(void);                         
#endif
#if (EVTGEN_0_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ66                  
     extern void EVTGEN_0_INTERRUPT_HANDLER(void);                                   
#endif
#if (CANFD_0_INTERRUPT0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ67                  
     extern void CANFD_0_INTERRUPT0_HANDLER(void);                                   
#endif
#if (CANFD_0_INTERRUPT1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ68                  
     extern void CANFD_0_INTERRUPT1_HANDLER(void);                                   
#endif
#if (CANFD_1_INTERRUPT0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ69                  
     extern void CANFD_1_INTERRUPT0_HANDLER(void);                                   
#endif
#if (CANFD_1_INTERRUPT1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ70                  
     extern void CANFD_1_INTERRUPT1_HANDLER(void);                                   
#endif
#if (CANFD_0_INTERRUPTS0_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ71                  
     extern void CANFD_0_INTERRUPTS0_0_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS0_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ72                  
     extern void CANFD_0_INTERRUPTS0_1_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS0_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ73                  
     extern void CANFD_0_INTERRUPTS0_2_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS0_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ74                  
     extern void CANFD_0_INTERRUPTS0_3_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS1_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ75                  
     extern void CANFD_0_INTERRUPTS1_0_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS1_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ76                  
     extern void CANFD_0_INTERRUPTS1_1_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS1_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ77                  
     extern void CANFD_0_INTERRUPTS1_2_HANDLER(void);                                
#endif
#if (CANFD_0_INTERRUPTS1_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ78                  
     extern void CANFD_0_INTERRUPTS1_3_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS0_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ79                  
     extern void CANFD_1_INTERRUPTS0_0_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS0_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ80                  
     extern void CANFD_1_INTERRUPTS0_1_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS0_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ81                  
     extern void CANFD_1_INTERRUPTS0_2_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS0_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ82                  
     extern void CANFD_1_INTERRUPTS0_3_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS1_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ83                  
     extern void CANFD_1_INTERRUPTS1_0_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS1_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ84                  
     extern void CANFD_1_INTERRUPTS1_1_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS1_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ85                  
     extern void CANFD_1_INTERRUPTS1_2_HANDLER(void);                                
#endif
#if (CANFD_1_INTERRUPTS1_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ86                  
     extern void CANFD_1_INTERRUPTS1_3_HANDLER(void);                                
#endif
#if (LIN_0_INTERRUPTS_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ87                  
     extern void LIN_0_INTERRUPTS_0_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ88                  
     extern void LIN_0_INTERRUPTS_1_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ89                  
     extern void LIN_0_INTERRUPTS_2_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ90                  
     extern void LIN_0_INTERRUPTS_3_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ91                  
     extern void LIN_0_INTERRUPTS_4_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ92                  
     extern void LIN_0_INTERRUPTS_5_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ93                  
     extern void LIN_0_INTERRUPTS_6_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ94                  
     extern void LIN_0_INTERRUPTS_7_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ95                  
     extern void LIN_0_INTERRUPTS_8_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)              // IRQ96                  
     extern void LIN_0_INTERRUPTS_9_HANDLER(void);                                   
#endif
#if (LIN_0_INTERRUPTS_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ97                  
     extern void LIN_0_INTERRUPTS_10_HANDLER(void);                                  
#endif
#if (LIN_0_INTERRUPTS_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ98                  
     extern void LIN_0_INTERRUPTS_11_HANDLER(void);                                  
#endif
#if (LIN_0_INTERRUPTS_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ99                  
     extern void LIN_0_INTERRUPTS_12_HANDLER(void);                                  
#endif
#if (LIN_0_INTERRUPTS_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ100                 
     extern void LIN_0_INTERRUPTS_13_HANDLER(void);                                  
#endif
#if (LIN_0_INTERRUPTS_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ101                 
     extern void LIN_0_INTERRUPTS_14_HANDLER(void);                                  
#endif
#if (LIN_0_INTERRUPTS_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)             // IRQ102                 
     extern void LIN_0_INTERRUPTS_15_HANDLER(void);                                  
#endif
#if (SCB_1_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ103                 
     extern void SCB_1_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_2_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ104                 
     extern void SCB_2_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_3_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ105                 
     extern void SCB_3_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_4_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ106                 
     extern void SCB_4_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_5_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ107                 
     extern void SCB_5_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_6_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ108                 
     extern void SCB_6_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_7_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ109                 
     extern void SCB_7_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_8_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ110                 
     extern void SCB_8_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_9_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                 // IRQ111                 
     extern void SCB_9_INTERRUPT_HANDLER(void);                                      
#endif
#if (SCB_10_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                // IRQ112                 
     extern void SCB_10_INTERRUPT_HANDLER(void);                                     
#endif
#if (PASS_0_INTERRUPTS_SAR_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ113                 
     extern void PASS_0_INTERRUPTS_SAR_0_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ114                 
     extern void PASS_0_INTERRUPTS_SAR_1_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ115                 
     extern void PASS_0_INTERRUPTS_SAR_2_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ116                 
     extern void PASS_0_INTERRUPTS_SAR_3_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ117                 
     extern void PASS_0_INTERRUPTS_SAR_4_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ118                 
     extern void PASS_0_INTERRUPTS_SAR_5_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ119                 
     extern void PASS_0_INTERRUPTS_SAR_6_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ120                 
     extern void PASS_0_INTERRUPTS_SAR_7_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ121                 
     extern void PASS_0_INTERRUPTS_SAR_8_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ122                 
     extern void PASS_0_INTERRUPTS_SAR_9_HANDLER(void);                              
#endif
#if (PASS_0_INTERRUPTS_SAR_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ123                 
     extern void PASS_0_INTERRUPTS_SAR_10_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ124                 
     extern void PASS_0_INTERRUPTS_SAR_11_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ125                 
     extern void PASS_0_INTERRUPTS_SAR_12_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ126                 
     extern void PASS_0_INTERRUPTS_SAR_13_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ127                 
     extern void PASS_0_INTERRUPTS_SAR_14_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ128                 
     extern void PASS_0_INTERRUPTS_SAR_15_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_16_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ129                 
     extern void PASS_0_INTERRUPTS_SAR_16_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_17_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ130                 
     extern void PASS_0_INTERRUPTS_SAR_17_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_18_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ131                 
     extern void PASS_0_INTERRUPTS_SAR_18_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_19_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ132                 
     extern void PASS_0_INTERRUPTS_SAR_19_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_20_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ133                 
     extern void PASS_0_INTERRUPTS_SAR_20_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_21_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ134                 
     extern void PASS_0_INTERRUPTS_SAR_21_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_22_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ135                 
     extern void PASS_0_INTERRUPTS_SAR_22_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_23_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ136                 
     extern void PASS_0_INTERRUPTS_SAR_23_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_24_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ137                 
     extern void PASS_0_INTERRUPTS_SAR_24_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_25_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ138                 
     extern void PASS_0_INTERRUPTS_SAR_25_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_26_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ139                 
     extern void PASS_0_INTERRUPTS_SAR_26_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_27_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ140                 
     extern void PASS_0_INTERRUPTS_SAR_27_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_28_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ141                 
     extern void PASS_0_INTERRUPTS_SAR_28_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_29_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ142                 
     extern void PASS_0_INTERRUPTS_SAR_29_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_30_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ143                 
     extern void PASS_0_INTERRUPTS_SAR_30_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_31_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ144                 
     extern void PASS_0_INTERRUPTS_SAR_31_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_32_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ145                 
     extern void PASS_0_INTERRUPTS_SAR_32_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_33_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ146                 
     extern void PASS_0_INTERRUPTS_SAR_33_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_34_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ147                 
     extern void PASS_0_INTERRUPTS_SAR_34_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_35_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ148                 
     extern void PASS_0_INTERRUPTS_SAR_35_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_36_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ149                 
     extern void PASS_0_INTERRUPTS_SAR_36_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_37_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ150                 
     extern void PASS_0_INTERRUPTS_SAR_37_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_38_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ151                 
     extern void PASS_0_INTERRUPTS_SAR_38_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_39_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ152                 
     extern void PASS_0_INTERRUPTS_SAR_39_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_40_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ153                 
     extern void PASS_0_INTERRUPTS_SAR_40_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_41_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ154                 
     extern void PASS_0_INTERRUPTS_SAR_41_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_42_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ155                 
     extern void PASS_0_INTERRUPTS_SAR_42_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_43_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ156                 
     extern void PASS_0_INTERRUPTS_SAR_43_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_44_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ157                 
     extern void PASS_0_INTERRUPTS_SAR_44_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_45_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ158                 
     extern void PASS_0_INTERRUPTS_SAR_45_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_46_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ159                 
     extern void PASS_0_INTERRUPTS_SAR_46_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_47_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ160                 
     extern void PASS_0_INTERRUPTS_SAR_47_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_48_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ161                 
     extern void PASS_0_INTERRUPTS_SAR_48_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_49_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ162                 
     extern void PASS_0_INTERRUPTS_SAR_49_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_50_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ163                 
     extern void PASS_0_INTERRUPTS_SAR_50_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_51_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ164                 
     extern void PASS_0_INTERRUPTS_SAR_51_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_52_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ165                 
     extern void PASS_0_INTERRUPTS_SAR_52_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_53_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ166                 
     extern void PASS_0_INTERRUPTS_SAR_53_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_54_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ167                 
     extern void PASS_0_INTERRUPTS_SAR_54_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_55_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ168                 
     extern void PASS_0_INTERRUPTS_SAR_55_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_56_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ169                 
     extern void PASS_0_INTERRUPTS_SAR_56_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_57_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ170                 
     extern void PASS_0_INTERRUPTS_SAR_57_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_58_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ171                 
     extern void PASS_0_INTERRUPTS_SAR_58_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_59_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ172                 
     extern void PASS_0_INTERRUPTS_SAR_59_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_60_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ173                 
     extern void PASS_0_INTERRUPTS_SAR_60_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_61_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ174                 
     extern void PASS_0_INTERRUPTS_SAR_61_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_62_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ175                 
     extern void PASS_0_INTERRUPTS_SAR_62_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_63_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ176                 
     extern void PASS_0_INTERRUPTS_SAR_63_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_64_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ177                 
     extern void PASS_0_INTERRUPTS_SAR_64_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_65_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ178                 
     extern void PASS_0_INTERRUPTS_SAR_65_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_66_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ179                 
     extern void PASS_0_INTERRUPTS_SAR_66_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_67_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ180                 
     extern void PASS_0_INTERRUPTS_SAR_67_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_68_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ181                 
     extern void PASS_0_INTERRUPTS_SAR_68_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_69_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ182                 
     extern void PASS_0_INTERRUPTS_SAR_69_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_70_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ183                 
     extern void PASS_0_INTERRUPTS_SAR_70_HANDLER(void);                             
#endif
#if (PASS_0_INTERRUPTS_SAR_71_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ184                 
     extern void PASS_0_INTERRUPTS_SAR_71_HANDLER(void);                             
#endif
#if (CPUSS_INTERRUPTS_DMAC_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ185                 
     extern void CPUSS_INTERRUPTS_DMAC_0_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ186                 
     extern void CPUSS_INTERRUPTS_DMAC_1_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ187                 
     extern void CPUSS_INTERRUPTS_DMAC_2_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ188                 
     extern void CPUSS_INTERRUPTS_DMAC_3_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ189                 
     extern void CPUSS_INTERRUPTS_DMAC_4_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ190                 
     extern void CPUSS_INTERRUPTS_DMAC_5_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ191                 
     extern void CPUSS_INTERRUPTS_DMAC_6_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DMAC_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ192                 
     extern void CPUSS_INTERRUPTS_DMAC_7_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ193                 
     extern void CPUSS_INTERRUPTS_DW0_0_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ194                 
     extern void CPUSS_INTERRUPTS_DW0_1_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ195                 
     extern void CPUSS_INTERRUPTS_DW0_2_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ196                 
     extern void CPUSS_INTERRUPTS_DW0_3_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ197                 
     extern void CPUSS_INTERRUPTS_DW0_4_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ198                 
     extern void CPUSS_INTERRUPTS_DW0_5_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ199                 
     extern void CPUSS_INTERRUPTS_DW0_6_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ200                 
     extern void CPUSS_INTERRUPTS_DW0_7_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ201                 
     extern void CPUSS_INTERRUPTS_DW0_8_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ202                 
     extern void CPUSS_INTERRUPTS_DW0_9_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW0_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ203                 
     extern void CPUSS_INTERRUPTS_DW0_10_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ204                 
     extern void CPUSS_INTERRUPTS_DW0_11_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ205                 
     extern void CPUSS_INTERRUPTS_DW0_12_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ206                 
     extern void CPUSS_INTERRUPTS_DW0_13_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ207                 
     extern void CPUSS_INTERRUPTS_DW0_14_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ208                 
     extern void CPUSS_INTERRUPTS_DW0_15_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_16_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ209                 
     extern void CPUSS_INTERRUPTS_DW0_16_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_17_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ210                 
     extern void CPUSS_INTERRUPTS_DW0_17_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_18_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ211                 
     extern void CPUSS_INTERRUPTS_DW0_18_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_19_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ212                 
     extern void CPUSS_INTERRUPTS_DW0_19_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_20_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ213                 
     extern void CPUSS_INTERRUPTS_DW0_20_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_21_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ214                 
     extern void CPUSS_INTERRUPTS_DW0_21_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_22_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ215                 
     extern void CPUSS_INTERRUPTS_DW0_22_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_23_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ216                 
     extern void CPUSS_INTERRUPTS_DW0_23_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_24_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ217                 
     extern void CPUSS_INTERRUPTS_DW0_24_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_25_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ218                 
     extern void CPUSS_INTERRUPTS_DW0_25_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_26_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ219                 
     extern void CPUSS_INTERRUPTS_DW0_26_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_27_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ220                 
     extern void CPUSS_INTERRUPTS_DW0_27_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_28_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ221                 
     extern void CPUSS_INTERRUPTS_DW0_28_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_29_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ222                 
     extern void CPUSS_INTERRUPTS_DW0_29_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_30_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ223                 
     extern void CPUSS_INTERRUPTS_DW0_30_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_31_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ224                 
     extern void CPUSS_INTERRUPTS_DW0_31_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_32_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ225                 
     extern void CPUSS_INTERRUPTS_DW0_32_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_33_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ226                 
     extern void CPUSS_INTERRUPTS_DW0_33_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_34_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ227                 
     extern void CPUSS_INTERRUPTS_DW0_34_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_35_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ228                 
     extern void CPUSS_INTERRUPTS_DW0_35_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_36_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ229                 
     extern void CPUSS_INTERRUPTS_DW0_36_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_37_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ230                 
     extern void CPUSS_INTERRUPTS_DW0_37_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_38_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ231                 
     extern void CPUSS_INTERRUPTS_DW0_38_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_39_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ232                 
     extern void CPUSS_INTERRUPTS_DW0_39_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_40_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ233                 
     extern void CPUSS_INTERRUPTS_DW0_40_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_41_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ234                 
     extern void CPUSS_INTERRUPTS_DW0_41_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_42_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ235                 
     extern void CPUSS_INTERRUPTS_DW0_42_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_43_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ236                 
     extern void CPUSS_INTERRUPTS_DW0_43_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_44_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ237                 
     extern void CPUSS_INTERRUPTS_DW0_44_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_45_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ238                 
     extern void CPUSS_INTERRUPTS_DW0_45_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_46_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ239                 
     extern void CPUSS_INTERRUPTS_DW0_46_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_47_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ240                 
     extern void CPUSS_INTERRUPTS_DW0_47_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_48_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ241                 
     extern void CPUSS_INTERRUPTS_DW0_48_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_49_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ242                 
     extern void CPUSS_INTERRUPTS_DW0_49_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_50_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ243                 
     extern void CPUSS_INTERRUPTS_DW0_50_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_51_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ244                 
     extern void CPUSS_INTERRUPTS_DW0_51_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_52_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ245                 
     extern void CPUSS_INTERRUPTS_DW0_52_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_53_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ246                 
     extern void CPUSS_INTERRUPTS_DW0_53_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_54_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ247                 
     extern void CPUSS_INTERRUPTS_DW0_54_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_55_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ248                 
     extern void CPUSS_INTERRUPTS_DW0_55_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_56_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ249                 
     extern void CPUSS_INTERRUPTS_DW0_56_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_57_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ250                 
     extern void CPUSS_INTERRUPTS_DW0_57_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_58_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ251                 
     extern void CPUSS_INTERRUPTS_DW0_58_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_59_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ252                 
     extern void CPUSS_INTERRUPTS_DW0_59_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_60_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ253                 
     extern void CPUSS_INTERRUPTS_DW0_60_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_61_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ254                 
     extern void CPUSS_INTERRUPTS_DW0_61_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_62_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ255                 
     extern void CPUSS_INTERRUPTS_DW0_62_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_63_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ256                 
     extern void CPUSS_INTERRUPTS_DW0_63_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_64_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ257                 
     extern void CPUSS_INTERRUPTS_DW0_64_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_65_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ258                 
     extern void CPUSS_INTERRUPTS_DW0_65_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_66_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ259                 
     extern void CPUSS_INTERRUPTS_DW0_66_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_67_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ260                 
     extern void CPUSS_INTERRUPTS_DW0_67_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_68_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ261                 
     extern void CPUSS_INTERRUPTS_DW0_68_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_69_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ262                 
     extern void CPUSS_INTERRUPTS_DW0_69_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_70_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ263                 
     extern void CPUSS_INTERRUPTS_DW0_70_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_71_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ264                 
     extern void CPUSS_INTERRUPTS_DW0_71_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_72_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ265                 
     extern void CPUSS_INTERRUPTS_DW0_72_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_73_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ266                 
     extern void CPUSS_INTERRUPTS_DW0_73_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_74_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ267                 
     extern void CPUSS_INTERRUPTS_DW0_74_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_75_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ268                 
     extern void CPUSS_INTERRUPTS_DW0_75_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_76_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ269                 
     extern void CPUSS_INTERRUPTS_DW0_76_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_77_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ270                 
     extern void CPUSS_INTERRUPTS_DW0_77_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_78_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ271                 
     extern void CPUSS_INTERRUPTS_DW0_78_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_79_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ272                 
     extern void CPUSS_INTERRUPTS_DW0_79_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_80_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ273                 
     extern void CPUSS_INTERRUPTS_DW0_80_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_81_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ274                 
     extern void CPUSS_INTERRUPTS_DW0_81_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_82_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ275                 
     extern void CPUSS_INTERRUPTS_DW0_82_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_83_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ276                 
     extern void CPUSS_INTERRUPTS_DW0_83_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_84_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ277                 
     extern void CPUSS_INTERRUPTS_DW0_84_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_85_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ278                 
     extern void CPUSS_INTERRUPTS_DW0_85_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_86_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ279                 
     extern void CPUSS_INTERRUPTS_DW0_86_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_87_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ280                 
     extern void CPUSS_INTERRUPTS_DW0_87_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_88_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ281                 
     extern void CPUSS_INTERRUPTS_DW0_88_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_89_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ282                 
     extern void CPUSS_INTERRUPTS_DW0_89_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_90_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ283                 
     extern void CPUSS_INTERRUPTS_DW0_90_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_91_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ284                 
     extern void CPUSS_INTERRUPTS_DW0_91_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_92_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ285                 
     extern void CPUSS_INTERRUPTS_DW0_92_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_93_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ286                 
     extern void CPUSS_INTERRUPTS_DW0_93_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_94_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ287                 
     extern void CPUSS_INTERRUPTS_DW0_94_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_95_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ288                 
     extern void CPUSS_INTERRUPTS_DW0_95_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_96_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ289                 
     extern void CPUSS_INTERRUPTS_DW0_96_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_97_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ290                 
     extern void CPUSS_INTERRUPTS_DW0_97_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_98_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ291                 
     extern void CPUSS_INTERRUPTS_DW0_98_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW0_99_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ292                 
     extern void CPUSS_INTERRUPTS_DW0_99_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ293                 
     extern void CPUSS_INTERRUPTS_DW1_0_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ294                 
     extern void CPUSS_INTERRUPTS_DW1_1_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ295                 
     extern void CPUSS_INTERRUPTS_DW1_2_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ296                 
     extern void CPUSS_INTERRUPTS_DW1_3_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ297                 
     extern void CPUSS_INTERRUPTS_DW1_4_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ298                 
     extern void CPUSS_INTERRUPTS_DW1_5_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ299                 
     extern void CPUSS_INTERRUPTS_DW1_6_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ300                 
     extern void CPUSS_INTERRUPTS_DW1_7_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ301                 
     extern void CPUSS_INTERRUPTS_DW1_8_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ302                 
     extern void CPUSS_INTERRUPTS_DW1_9_HANDLER(void);                               
#endif
#if (CPUSS_INTERRUPTS_DW1_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ303                 
     extern void CPUSS_INTERRUPTS_DW1_10_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ304                 
     extern void CPUSS_INTERRUPTS_DW1_11_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ305                 
     extern void CPUSS_INTERRUPTS_DW1_12_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ306                 
     extern void CPUSS_INTERRUPTS_DW1_13_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ307                 
     extern void CPUSS_INTERRUPTS_DW1_14_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ308                 
     extern void CPUSS_INTERRUPTS_DW1_15_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_16_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ309                 
     extern void CPUSS_INTERRUPTS_DW1_16_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_17_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ310                 
     extern void CPUSS_INTERRUPTS_DW1_17_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_18_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ311                 
     extern void CPUSS_INTERRUPTS_DW1_18_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_19_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ312                 
     extern void CPUSS_INTERRUPTS_DW1_19_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_20_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ313                 
     extern void CPUSS_INTERRUPTS_DW1_20_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_21_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ314                 
     extern void CPUSS_INTERRUPTS_DW1_21_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_22_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ315                 
     extern void CPUSS_INTERRUPTS_DW1_22_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_23_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ316                 
     extern void CPUSS_INTERRUPTS_DW1_23_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_24_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ317                 
     extern void CPUSS_INTERRUPTS_DW1_24_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_25_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ318                 
     extern void CPUSS_INTERRUPTS_DW1_25_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_26_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ319                 
     extern void CPUSS_INTERRUPTS_DW1_26_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_27_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ320                 
     extern void CPUSS_INTERRUPTS_DW1_27_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_28_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ321                 
     extern void CPUSS_INTERRUPTS_DW1_28_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_29_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ322                 
     extern void CPUSS_INTERRUPTS_DW1_29_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_30_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ323                 
     extern void CPUSS_INTERRUPTS_DW1_30_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_31_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ324                 
     extern void CPUSS_INTERRUPTS_DW1_31_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_32_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ325                 
     extern void CPUSS_INTERRUPTS_DW1_32_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_33_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ326                 
     extern void CPUSS_INTERRUPTS_DW1_33_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_34_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ327                 
     extern void CPUSS_INTERRUPTS_DW1_34_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_35_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ328                 
     extern void CPUSS_INTERRUPTS_DW1_35_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_36_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ329                 
     extern void CPUSS_INTERRUPTS_DW1_36_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_37_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ330                 
     extern void CPUSS_INTERRUPTS_DW1_37_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_38_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ331                 
     extern void CPUSS_INTERRUPTS_DW1_38_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_39_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ332                 
     extern void CPUSS_INTERRUPTS_DW1_39_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_40_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ333                 
     extern void CPUSS_INTERRUPTS_DW1_40_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_41_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ334                 
     extern void CPUSS_INTERRUPTS_DW1_41_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_42_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ335                 
     extern void CPUSS_INTERRUPTS_DW1_42_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_43_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ336                 
     extern void CPUSS_INTERRUPTS_DW1_43_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_44_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ337                 
     extern void CPUSS_INTERRUPTS_DW1_44_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_45_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ338                 
     extern void CPUSS_INTERRUPTS_DW1_45_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_46_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ339                 
     extern void CPUSS_INTERRUPTS_DW1_46_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_47_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ340                 
     extern void CPUSS_INTERRUPTS_DW1_47_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_48_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ341                 
     extern void CPUSS_INTERRUPTS_DW1_48_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_49_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ342                 
     extern void CPUSS_INTERRUPTS_DW1_49_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_50_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ343                 
     extern void CPUSS_INTERRUPTS_DW1_50_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_51_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ344                 
     extern void CPUSS_INTERRUPTS_DW1_51_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_52_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ345                 
     extern void CPUSS_INTERRUPTS_DW1_52_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_53_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ346                 
     extern void CPUSS_INTERRUPTS_DW1_53_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_54_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ347                 
     extern void CPUSS_INTERRUPTS_DW1_54_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_55_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ348                 
     extern void CPUSS_INTERRUPTS_DW1_55_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_56_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ349                 
     extern void CPUSS_INTERRUPTS_DW1_56_HANDLER(void);                              
#endif
#if (CPUSS_INTERRUPTS_DW1_57_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ350                 
     extern void CPUSS_INTERRUPTS_DW1_57_HANDLER(void);                              
#endif
#if (TCPWM_0_INTERRUPTS_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ351                 
     extern void TCPWM_0_INTERRUPTS_0_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ352                 
     extern void TCPWM_0_INTERRUPTS_1_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ353                 
     extern void TCPWM_0_INTERRUPTS_2_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_3_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ354                 
     extern void TCPWM_0_INTERRUPTS_3_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_4_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ355                 
     extern void TCPWM_0_INTERRUPTS_4_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_5_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ356                 
     extern void TCPWM_0_INTERRUPTS_5_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_6_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ357                 
     extern void TCPWM_0_INTERRUPTS_6_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_7_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ358                 
     extern void TCPWM_0_INTERRUPTS_7_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_8_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ359                 
     extern void TCPWM_0_INTERRUPTS_8_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_9_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)            // IRQ360                 
     extern void TCPWM_0_INTERRUPTS_9_HANDLER(void);                                 
#endif
#if (TCPWM_0_INTERRUPTS_10_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ361                 
     extern void TCPWM_0_INTERRUPTS_10_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_11_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ362                 
     extern void TCPWM_0_INTERRUPTS_11_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_12_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ363                 
     extern void TCPWM_0_INTERRUPTS_12_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_13_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ364                 
     extern void TCPWM_0_INTERRUPTS_13_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_14_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ365                 
     extern void TCPWM_0_INTERRUPTS_14_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_15_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ366                 
     extern void TCPWM_0_INTERRUPTS_15_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_16_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ367                 
     extern void TCPWM_0_INTERRUPTS_16_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_17_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ368                 
     extern void TCPWM_0_INTERRUPTS_17_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_18_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ369                 
     extern void TCPWM_0_INTERRUPTS_18_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_19_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ370                 
     extern void TCPWM_0_INTERRUPTS_19_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_20_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ371                 
     extern void TCPWM_0_INTERRUPTS_20_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_21_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ372                 
     extern void TCPWM_0_INTERRUPTS_21_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_22_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ373                 
     extern void TCPWM_0_INTERRUPTS_22_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_23_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ374                 
     extern void TCPWM_0_INTERRUPTS_23_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_24_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ375                 
     extern void TCPWM_0_INTERRUPTS_24_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_25_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ376                 
     extern void TCPWM_0_INTERRUPTS_25_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_26_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ377                 
     extern void TCPWM_0_INTERRUPTS_26_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_27_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ378                 
     extern void TCPWM_0_INTERRUPTS_27_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_28_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ379                 
     extern void TCPWM_0_INTERRUPTS_28_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_29_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ380                 
     extern void TCPWM_0_INTERRUPTS_29_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_30_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ381                 
     extern void TCPWM_0_INTERRUPTS_30_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_31_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ382                 
     extern void TCPWM_0_INTERRUPTS_31_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_32_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ383                 
     extern void TCPWM_0_INTERRUPTS_32_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_33_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ384                 
     extern void TCPWM_0_INTERRUPTS_33_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_34_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ385                 
     extern void TCPWM_0_INTERRUPTS_34_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_35_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ386                 
     extern void TCPWM_0_INTERRUPTS_35_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_36_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ387                 
     extern void TCPWM_0_INTERRUPTS_36_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_37_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ388                 
     extern void TCPWM_0_INTERRUPTS_37_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_38_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ389                 
     extern void TCPWM_0_INTERRUPTS_38_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_39_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ390                 
     extern void TCPWM_0_INTERRUPTS_39_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_40_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ391                 
     extern void TCPWM_0_INTERRUPTS_40_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_41_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ392                 
     extern void TCPWM_0_INTERRUPTS_41_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_42_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ393                 
     extern void TCPWM_0_INTERRUPTS_42_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_43_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ394                 
     extern void TCPWM_0_INTERRUPTS_43_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_44_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ395                 
     extern void TCPWM_0_INTERRUPTS_44_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_45_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ396                 
     extern void TCPWM_0_INTERRUPTS_45_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_46_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ397                 
     extern void TCPWM_0_INTERRUPTS_46_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_47_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ398                 
     extern void TCPWM_0_INTERRUPTS_47_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_48_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ399                 
     extern void TCPWM_0_INTERRUPTS_48_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_49_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ400                 
     extern void TCPWM_0_INTERRUPTS_49_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_50_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ401                 
     extern void TCPWM_0_INTERRUPTS_50_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_51_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ402                 
     extern void TCPWM_0_INTERRUPTS_51_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_52_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ403                 
     extern void TCPWM_0_INTERRUPTS_52_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_53_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ404                 
     extern void TCPWM_0_INTERRUPTS_53_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_54_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ405                 
     extern void TCPWM_0_INTERRUPTS_54_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_55_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ406                 
     extern void TCPWM_0_INTERRUPTS_55_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_56_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ407                 
     extern void TCPWM_0_INTERRUPTS_56_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_57_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ408                 
     extern void TCPWM_0_INTERRUPTS_57_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_58_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ409                 
     extern void TCPWM_0_INTERRUPTS_58_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_59_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ410                 
     extern void TCPWM_0_INTERRUPTS_59_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_60_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ411                 
     extern void TCPWM_0_INTERRUPTS_60_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_61_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ412                 
     extern void TCPWM_0_INTERRUPTS_61_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_62_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ413                 
     extern void TCPWM_0_INTERRUPTS_62_HANDLER(void);                                
#endif
#if (TCPWM_0_INTERRUPTS_256_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ414                 
     extern void TCPWM_0_INTERRUPTS_256_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_257_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ415                 
     extern void TCPWM_0_INTERRUPTS_257_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_258_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ416                 
     extern void TCPWM_0_INTERRUPTS_258_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_259_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ417                 
     extern void TCPWM_0_INTERRUPTS_259_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_260_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ418                 
     extern void TCPWM_0_INTERRUPTS_260_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_261_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ419                 
     extern void TCPWM_0_INTERRUPTS_261_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_262_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ420                 
     extern void TCPWM_0_INTERRUPTS_262_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_263_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ421                 
     extern void TCPWM_0_INTERRUPTS_263_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_264_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ422                 
     extern void TCPWM_0_INTERRUPTS_264_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_265_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ423                 
     extern void TCPWM_0_INTERRUPTS_265_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_266_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ424                 
     extern void TCPWM_0_INTERRUPTS_266_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_267_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ425                 
     extern void TCPWM_0_INTERRUPTS_267_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_512_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ426                 
     extern void TCPWM_0_INTERRUPTS_512_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_513_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ427                 
     extern void TCPWM_0_INTERRUPTS_513_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_514_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ428                 
     extern void TCPWM_0_INTERRUPTS_514_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_515_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ429                 
     extern void TCPWM_0_INTERRUPTS_515_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_516_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ430                 
     extern void TCPWM_0_INTERRUPTS_516_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_517_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ431                 
     extern void TCPWM_0_INTERRUPTS_517_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_518_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ432                 
     extern void TCPWM_0_INTERRUPTS_518_HANDLER(void);                               
#endif
#if (TCPWM_0_INTERRUPTS_519_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)          // IRQ433                 
     extern void TCPWM_0_INTERRUPTS_519_HANDLER(void);                               
#endif
#if (SMIF_0_INTERRUPT_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)                // IRQ434                 
     extern void SMIF_0_INTERRUPT_HANDLER(void);                                     
#endif
#if (ETH_0_INTERRUPT_ETH_0_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ435                 
     extern void ETH_0_INTERRUPT_ETH_0_HANDLER(void);                                
#endif
#if (ETH_0_INTERRUPT_ETH_2_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ436                 
     extern void ETH_0_INTERRUPT_ETH_2_HANDLER(void);                                
#endif
#if (ETH_0_INTERRUPT_ETH_1_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)           // IRQ437                 
     extern void ETH_0_INTERRUPT_ETH_1_HANDLER(void);                                
#endif
#if (SDHC_0_INTERRUPT_GENERAL_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)        // IRQ438                 
     extern void SDHC_0_INTERRUPT_GENERAL_HANDLER(void);                             
#endif
#if (SDHC_0_INTERRUPT_WAKEUP_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ439                 
     extern void SDHC_0_INTERRUPT_WAKEUP_HANDLER(void);                              
#endif
#if (AUDIOSS_0_INTERRUPT_I2S_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ440                 
     extern void AUDIOSS_0_INTERRUPT_I2S_HANDLER(void);                              
#endif
#if (AUDIOSS_1_INTERRUPT_I2S_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ441                 
     extern void AUDIOSS_1_INTERRUPT_I2S_HANDLER(void);                              
#endif
#if (AUDIOSS_2_INTERRUPT_I2S_HANDLER !=  INTERRUPTS_IRQ_DEFAULT_HANDLER)         // IRQ442                 
     extern void AUDIOSS_2_INTERRUPT_I2S_HANDLER(void);                              
#endif

/* re-enable warning "zero used for undefined preprocessing identifier" */
#ifdef __ghs__
    #pragma ghs endnowarning
#endif
#ifdef __ICCARM__
    #pragma diag_default=Pe193
#endif

#undef INTERRUPTS_IRQ_DEFAULT_HANDLER
#define INTERRUPTS_IRQ_DEFAULT_HANDLER                    DEFAULT_HANDLER_NAME


#endif /* _CY_INTERRUPT_MAP_CM7_1_H_ */


/* [] END OF FILE */
