/***************************************************************************//**
* \file cy_interrupt_map.c
*
* \brief
* Provides the table with system interrupt handlers
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
/*
* MPN Hash                       : ADE124FF1E3BA50B6B710DE106AC6BFA504C5AB3B0A76E3A4843DC15F9301C9E
* Product File Hash              : 510AF0AD4A8C80EC3EAB24A87F8EBD906B61D1EEF76B5AFF766D771B86DC780E
* Unrolled reg map File Hash     : 553E97D1FC291A9164CE06A913646372894FA5F7D67BA1499D610AE04494AC9A
* Not Unrolled reg map File Hash : E66F2D96278F057F24AC7263B4716DA2320343A1933D13C78476D1D73D620498
*/

#include "syslib/cy_syslib.h"
#include "sysint/cy_sysint.h"
#include "motor_control.h"

#if CY_CPU_CORTEX_M0P
    #include "cy_interrupt_map_cm0plus.h"
#else
    #include "cy_interrupt_map_cm4.h"
#endif

extern void Cy_DefaultUserHandler(void);


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! DO NOT EDIT THIS TABLE DIRECTLY BUT USE THE CORE SPECIFIC HEADER FILE (cy_interrupt_map_<core>.h !
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/** Table of all user specific IRQ vectors */

#ifndef CY_LINK_SYSTEM_IRQ_TABLE_TO_RAM
const
#endif
       cy_systemIntr_Handler Cy_SystemIrqUserTable[] =
{
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_0_HANDLER,               // IRQ0                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_1_HANDLER,               // IRQ1                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_2_HANDLER,               // IRQ2                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_3_HANDLER,               // IRQ3                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_4_HANDLER,               // IRQ4                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_5_HANDLER,               // IRQ5                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_6_HANDLER,               // IRQ6                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_IPC_7_HANDLER,               // IRQ7                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_FAULT_0_HANDLER,             // IRQ8                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_FAULT_1_HANDLER,             // IRQ9                   
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_FAULT_2_HANDLER,             // IRQ10                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_FAULT_3_HANDLER,             // IRQ11                  
     (cy_systemIntr_Handler) SRSS_INTERRUPT_BACKUP_HANDLER,                // IRQ12                  
     (cy_systemIntr_Handler) SRSS_INTERRUPT_MCWDT_0_HANDLER,               // IRQ13                  
     (cy_systemIntr_Handler) SRSS_INTERRUPT_MCWDT_1_HANDLER,               // IRQ14                  
     (cy_systemIntr_Handler) SRSS_INTERRUPT_WDT_HANDLER,                   // IRQ15                  
     (cy_systemIntr_Handler) SRSS_INTERRUPT_HANDLER,                       // IRQ16                  
     (cy_systemIntr_Handler) SCB_0_INTERRUPT_HANDLER,                      // IRQ17                  
     (cy_systemIntr_Handler) EVTGEN_0_INTERRUPT_DPSLP_HANDLER,             // IRQ18                  
     (cy_systemIntr_Handler) IOSS_INTERRUPT_VDD_HANDLER,                   // IRQ19                  
     (cy_systemIntr_Handler) IOSS_INTERRUPT_GPIO_HANDLER,                  // IRQ20                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_0_HANDLER,               // IRQ21                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_1_HANDLER,               // IRQ22                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_2_HANDLER,               // IRQ23                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_3_HANDLER,               // IRQ24                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_4_HANDLER,               // IRQ25                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_5_HANDLER,               // IRQ26                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_6_HANDLER,               // IRQ27                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_7_HANDLER,               // IRQ28                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_8_HANDLER,               // IRQ29                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_9_HANDLER,               // IRQ30                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_10_HANDLER,              // IRQ31                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_11_HANDLER,              // IRQ32                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_12_HANDLER,              // IRQ33                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_13_HANDLER,              // IRQ34                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_14_HANDLER,              // IRQ35                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_15_HANDLER,              // IRQ36                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_16_HANDLER,              // IRQ37                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_17_HANDLER,              // IRQ38                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_18_HANDLER,              // IRQ39                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_19_HANDLER,              // IRQ40                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_20_HANDLER,              // IRQ41                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_21_HANDLER,              // IRQ42                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_22_HANDLER,              // IRQ43                  
     (cy_systemIntr_Handler) IOSS_INTERRUPTS_GPIO_23_HANDLER,              // IRQ44                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPT_CRYPTO_HANDLER,               // IRQ45                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPT_FM_HANDLER,                   // IRQ46                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_CM4_FP_HANDLER,              // IRQ47                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_CM0_CTI_0_HANDLER,           // IRQ48                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_CM0_CTI_1_HANDLER,           // IRQ49                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_CM4_CTI_0_HANDLER,           // IRQ50                  
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_CM4_CTI_1_HANDLER,           // IRQ51                  
     (cy_systemIntr_Handler) EVTGEN_0_INTERRUPT_HANDLER,                   // IRQ52                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPT0_HANDLER,                   // IRQ53                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPT1_HANDLER,                   // IRQ54                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPT0_HANDLER,                   // IRQ55                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPT1_HANDLER,                   // IRQ56                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS0_0_HANDLER,                // IRQ57                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS0_1_HANDLER,                // IRQ58                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS0_2_HANDLER,                // IRQ59                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS0_3_HANDLER,                // IRQ60                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS1_0_HANDLER,                // IRQ61                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS1_1_HANDLER,                // IRQ62                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS1_2_HANDLER,                // IRQ63                  
     (cy_systemIntr_Handler) CANFD_0_INTERRUPTS1_3_HANDLER,                // IRQ64                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS0_0_HANDLER,                // IRQ65                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS0_1_HANDLER,                // IRQ66                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS0_2_HANDLER,                // IRQ67                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS0_3_HANDLER,                // IRQ68                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS1_0_HANDLER,                // IRQ69                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS1_1_HANDLER,                // IRQ70                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS1_2_HANDLER,                // IRQ71                  
     (cy_systemIntr_Handler) CANFD_1_INTERRUPTS1_3_HANDLER,                // IRQ72                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_0_HANDLER,                   // IRQ73                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_1_HANDLER,                   // IRQ74                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_2_HANDLER,                   // IRQ75                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_3_HANDLER,                   // IRQ76                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_4_HANDLER,                   // IRQ77                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_5_HANDLER,                   // IRQ78                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_6_HANDLER,                   // IRQ79                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_7_HANDLER,                   // IRQ80                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_8_HANDLER,                   // IRQ81                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_9_HANDLER,                   // IRQ82                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_10_HANDLER,                  // IRQ83                  
     (cy_systemIntr_Handler) LIN_0_INTERRUPTS_11_HANDLER,                  // IRQ84                  
     (cy_systemIntr_Handler) CXPI_0_INTERRUPTS_0_HANDLER,                  // IRQ85                  
     (cy_systemIntr_Handler) CXPI_0_INTERRUPTS_1_HANDLER,                  // IRQ86                  
     (cy_systemIntr_Handler) CXPI_0_INTERRUPTS_2_HANDLER,                  // IRQ87                  
     (cy_systemIntr_Handler) CXPI_0_INTERRUPTS_3_HANDLER,                  // IRQ88                  
     (cy_systemIntr_Handler) SCB_1_INTERRUPT_HANDLER,                      // IRQ89                  
     (cy_systemIntr_Handler) SCB_2_INTERRUPT_HANDLER,                      // IRQ90                  
     (cy_systemIntr_Handler) SCB_3_INTERRUPT_HANDLER,                      // IRQ91                  
     (cy_systemIntr_Handler) SCB_4_INTERRUPT_HANDLER,                      // IRQ92                  
     (cy_systemIntr_Handler) SCB_5_INTERRUPT_HANDLER,                      // IRQ93                  
     (cy_systemIntr_Handler) SCB_6_INTERRUPT_HANDLER,                      // IRQ94                  
     (cy_systemIntr_Handler) SCB_7_INTERRUPT_HANDLER,                      // IRQ95                  
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_0_HANDLER,              // IRQ96                  
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_1_HANDLER,              // IRQ97                  
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_2_HANDLER,              // IRQ98                  
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_3_HANDLER,              // IRQ99                  
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_4_HANDLER,              // IRQ100                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_5_HANDLER,              // IRQ101                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_6_HANDLER,              // IRQ102                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_7_HANDLER,              // IRQ103                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_8_HANDLER,              // IRQ104                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_9_HANDLER,              // IRQ105                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_10_HANDLER,             // IRQ106                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_11_HANDLER,             // IRQ107                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_12_HANDLER,             // IRQ108                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_13_HANDLER,             // IRQ109                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_14_HANDLER,             // IRQ110                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_15_HANDLER,             // IRQ111                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_16_HANDLER,             // IRQ112                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_17_HANDLER,             // IRQ113                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_18_HANDLER,             // IRQ114                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_19_HANDLER,             // IRQ115                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_20_HANDLER,             // IRQ116                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_21_HANDLER,             // IRQ117                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_22_HANDLER,             // IRQ118                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_23_HANDLER,             // IRQ119                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_32_HANDLER,             // IRQ120                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_33_HANDLER,             // IRQ121                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_34_HANDLER,             // IRQ122                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_35_HANDLER,             // IRQ123                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_36_HANDLER,             // IRQ124                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_37_HANDLER,             // IRQ125                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_38_HANDLER,             // IRQ126                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_39_HANDLER,             // IRQ127                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_40_HANDLER,             // IRQ128                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_41_HANDLER,             // IRQ129                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_42_HANDLER,             // IRQ130                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_43_HANDLER,             // IRQ131                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_44_HANDLER,             // IRQ132                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_45_HANDLER,             // IRQ133                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_46_HANDLER,             // IRQ134                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_47_HANDLER,             // IRQ135                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_48_HANDLER,             // IRQ136                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_49_HANDLER,             // IRQ137                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_50_HANDLER,             // IRQ138                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_51_HANDLER,             // IRQ139                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_52_HANDLER,             // IRQ140                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_53_HANDLER,             // IRQ141                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_54_HANDLER,             // IRQ142                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_55_HANDLER,             // IRQ143                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_56_HANDLER,             // IRQ144                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_57_HANDLER,             // IRQ145                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_58_HANDLER,             // IRQ146                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_59_HANDLER,             // IRQ147                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_60_HANDLER,             // IRQ148                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_61_HANDLER,             // IRQ149                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_62_HANDLER,             // IRQ150                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_63_HANDLER,             // IRQ151                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_64_HANDLER,             // IRQ152                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_65_HANDLER,             // IRQ153                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_66_HANDLER,             // IRQ154                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_67_HANDLER,             // IRQ155                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_68_HANDLER,             // IRQ156                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_69_HANDLER,             // IRQ157                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_70_HANDLER,             // IRQ158                 
     (cy_systemIntr_Handler) PASS_0_INTERRUPTS_SAR_71_HANDLER,             // IRQ159                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DMAC_0_HANDLER,              // IRQ160                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DMAC_1_HANDLER,              // IRQ161                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DMAC_2_HANDLER,              // IRQ162                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DMAC_3_HANDLER,              // IRQ163                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_0_HANDLER,               // IRQ164                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_1_HANDLER,               // IRQ165                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_2_HANDLER,               // IRQ166                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_3_HANDLER,               // IRQ167                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_4_HANDLER,               // IRQ168                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_5_HANDLER,               // IRQ169                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_6_HANDLER,               // IRQ170                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_7_HANDLER,               // IRQ171                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_8_HANDLER,               // IRQ172                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_9_HANDLER,               // IRQ173                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_10_HANDLER,              // IRQ174                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_11_HANDLER,              // IRQ175                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_12_HANDLER,              // IRQ176                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_13_HANDLER,              // IRQ177                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_14_HANDLER,              // IRQ178                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_15_HANDLER,              // IRQ179                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_16_HANDLER,              // IRQ180                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_17_HANDLER,              // IRQ181                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_18_HANDLER,              // IRQ182                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_19_HANDLER,              // IRQ183                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_20_HANDLER,              // IRQ184                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_21_HANDLER,              // IRQ185                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_22_HANDLER,              // IRQ186                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_23_HANDLER,              // IRQ187                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_24_HANDLER,              // IRQ188                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_25_HANDLER,              // IRQ189                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_26_HANDLER,              // IRQ190                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_27_HANDLER,              // IRQ191                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_28_HANDLER,              // IRQ192                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_29_HANDLER,              // IRQ193                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_30_HANDLER,              // IRQ194                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_31_HANDLER,              // IRQ195                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_32_HANDLER,              // IRQ196                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_33_HANDLER,              // IRQ197                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_34_HANDLER,              // IRQ198                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_35_HANDLER,              // IRQ199                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_36_HANDLER,              // IRQ200                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_37_HANDLER,              // IRQ201                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_38_HANDLER,              // IRQ202                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_39_HANDLER,              // IRQ203                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_40_HANDLER,              // IRQ204                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_41_HANDLER,              // IRQ205                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_42_HANDLER,              // IRQ206                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_43_HANDLER,              // IRQ207                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_44_HANDLER,              // IRQ208                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_45_HANDLER,              // IRQ209                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_46_HANDLER,              // IRQ210                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_47_HANDLER,              // IRQ211                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_48_HANDLER,              // IRQ212                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_49_HANDLER,              // IRQ213                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_50_HANDLER,              // IRQ214                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_51_HANDLER,              // IRQ215                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_52_HANDLER,              // IRQ216                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_53_HANDLER,              // IRQ217                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_54_HANDLER,              // IRQ218                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_55_HANDLER,              // IRQ219                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_56_HANDLER,              // IRQ220                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_57_HANDLER,              // IRQ221                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_58_HANDLER,              // IRQ222                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_59_HANDLER,              // IRQ223                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_60_HANDLER,              // IRQ224                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_61_HANDLER,              // IRQ225                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_62_HANDLER,              // IRQ226                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_63_HANDLER,              // IRQ227                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_64_HANDLER,              // IRQ228                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_65_HANDLER,              // IRQ229                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_66_HANDLER,              // IRQ230                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_67_HANDLER,              // IRQ231                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_68_HANDLER,              // IRQ232                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_69_HANDLER,              // IRQ233                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_70_HANDLER,              // IRQ234                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_71_HANDLER,              // IRQ235                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_72_HANDLER,              // IRQ236                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_73_HANDLER,              // IRQ237                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_74_HANDLER,              // IRQ238                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_75_HANDLER,              // IRQ239                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_76_HANDLER,              // IRQ240                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_77_HANDLER,              // IRQ241                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_78_HANDLER,              // IRQ242                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_79_HANDLER,              // IRQ243                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_80_HANDLER,              // IRQ244                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_81_HANDLER,              // IRQ245                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_82_HANDLER,              // IRQ246                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_83_HANDLER,              // IRQ247                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_84_HANDLER,              // IRQ248                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_85_HANDLER,              // IRQ249                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_86_HANDLER,              // IRQ250                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_87_HANDLER,              // IRQ251                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_88_HANDLER,              // IRQ252                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_89_HANDLER,              // IRQ253                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_90_HANDLER,              // IRQ254                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW0_91_HANDLER,              // IRQ255                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_0_HANDLER,               // IRQ256                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_1_HANDLER,               // IRQ257                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_2_HANDLER,               // IRQ258                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_3_HANDLER,               // IRQ259                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_4_HANDLER,               // IRQ260                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_5_HANDLER,               // IRQ261                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_6_HANDLER,               // IRQ262                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_7_HANDLER,               // IRQ263                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_8_HANDLER,               // IRQ264                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_9_HANDLER,               // IRQ265                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_10_HANDLER,              // IRQ266                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_11_HANDLER,              // IRQ267                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_12_HANDLER,              // IRQ268                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_13_HANDLER,              // IRQ269                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_14_HANDLER,              // IRQ270                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_15_HANDLER,              // IRQ271                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_16_HANDLER,              // IRQ272                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_17_HANDLER,              // IRQ273                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_18_HANDLER,              // IRQ274                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_19_HANDLER,              // IRQ275                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_20_HANDLER,              // IRQ276                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_21_HANDLER,              // IRQ277                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_22_HANDLER,              // IRQ278                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_23_HANDLER,              // IRQ279                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_24_HANDLER,              // IRQ280                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_25_HANDLER,              // IRQ281                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_26_HANDLER,              // IRQ282                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_27_HANDLER,              // IRQ283                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_28_HANDLER,              // IRQ284                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_29_HANDLER,              // IRQ285                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_30_HANDLER,              // IRQ286                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_31_HANDLER,              // IRQ287                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_32_HANDLER,              // IRQ288                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_33_HANDLER,              // IRQ289                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_34_HANDLER,              // IRQ290                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_35_HANDLER,              // IRQ291                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_36_HANDLER,              // IRQ292                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_37_HANDLER,              // IRQ293                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_38_HANDLER,              // IRQ294                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_39_HANDLER,              // IRQ295                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_40_HANDLER,              // IRQ296                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_41_HANDLER,              // IRQ297                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_42_HANDLER,              // IRQ298                 
     (cy_systemIntr_Handler) CPUSS_INTERRUPTS_DW1_43_HANDLER,              // IRQ299                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_0_HANDLER,                 // IRQ300                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_1_HANDLER,                 // IRQ301                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_2_HANDLER,                 // IRQ302                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_3_HANDLER,                 // IRQ303                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_4_HANDLER,                 // IRQ304                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_5_HANDLER,                 // IRQ305                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_6_HANDLER,                 // IRQ306                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_7_HANDLER,                 // IRQ307                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_8_HANDLER,                 // IRQ308                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_9_HANDLER,                 // IRQ309                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_10_HANDLER,                // IRQ310                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_11_HANDLER,                // IRQ311                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_12_HANDLER,                // IRQ312                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_13_HANDLER,                // IRQ313                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_14_HANDLER,                // IRQ314                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_15_HANDLER,                // IRQ315                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_16_HANDLER,                // IRQ316                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_17_HANDLER,                // IRQ317                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_18_HANDLER,                // IRQ318                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_19_HANDLER,                // IRQ319                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_20_HANDLER,                // IRQ320                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_21_HANDLER,                // IRQ321                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_22_HANDLER,                // IRQ322                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_23_HANDLER,                // IRQ323                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_24_HANDLER,                // IRQ324                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_25_HANDLER,                // IRQ325                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_26_HANDLER,                // IRQ326                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_27_HANDLER,                // IRQ327                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_28_HANDLER,                // IRQ328                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_29_HANDLER,                // IRQ329                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_30_HANDLER,                // IRQ330                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_31_HANDLER,                // IRQ331                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_32_HANDLER,                // IRQ332                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_33_HANDLER,                // IRQ333                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_34_HANDLER,                // IRQ334                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_35_HANDLER,                // IRQ335                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_36_HANDLER,                // IRQ336                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_37_HANDLER,                // IRQ337                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_38_HANDLER,                // IRQ338                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_39_HANDLER,                // IRQ339                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_40_HANDLER,                // IRQ340                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_41_HANDLER,                // IRQ341                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_42_HANDLER,                // IRQ342                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_43_HANDLER,                // IRQ343                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_44_HANDLER,                // IRQ344                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_45_HANDLER,                // IRQ345                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_46_HANDLER,                // IRQ346                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_47_HANDLER,                // IRQ347                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_48_HANDLER,                // IRQ348                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_49_HANDLER,                // IRQ349                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_50_HANDLER,                // IRQ350                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_51_HANDLER,                // IRQ351                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_52_HANDLER,                // IRQ352                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_53_HANDLER,                // IRQ353                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_54_HANDLER,                // IRQ354                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_55_HANDLER,                // IRQ355                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_56_HANDLER,                // IRQ356                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_57_HANDLER,                // IRQ357                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_58_HANDLER,                // IRQ358                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_59_HANDLER,                // IRQ359                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_60_HANDLER,                // IRQ360                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_61_HANDLER,                // IRQ361                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_62_HANDLER,                // IRQ362                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_256_HANDLER,               // IRQ363                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_257_HANDLER,               // IRQ364                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_258_HANDLER,               // IRQ365                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_259_HANDLER,               // IRQ366                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_260_HANDLER,               // IRQ367                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_261_HANDLER,               // IRQ368                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_262_HANDLER,               // IRQ369                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_263_HANDLER,               // IRQ370                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_264_HANDLER,               // IRQ371                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_265_HANDLER,               // IRQ372                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_266_HANDLER,               // IRQ373                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_267_HANDLER,               // IRQ374                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_512_HANDLER,               // IRQ375                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_513_HANDLER,               // IRQ376                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_514_HANDLER,               // IRQ377                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_515_HANDLER,               // IRQ378                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_516_HANDLER,               // IRQ379                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_517_HANDLER,               // IRQ380                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_518_HANDLER,               // IRQ381                 
     (cy_systemIntr_Handler) TCPWM_0_INTERRUPTS_519_HANDLER,               // IRQ382                 
};

#ifdef CY_LINK_SYSTEM_IRQ_TABLE_TO_RAM
cy_systemIntr_Handler * Cy_SysInt_SystemIrqUserTableRamPointer = Cy_SystemIrqUserTable;
#endif      


void Cy_DefaultUserHandler(void)
{
    // This IRQ occurred because CPU attempted to call IRQ that has not been mapped to user function
    while(1);
}


void CpuUserInt0_Handler(void)
{
    un_CPUSS_CM0_INT0_STATUS_t system_int_status = {0};

#if CY_CPU_CORTEX_M0P
    system_int_status.u32Register = CPUSS->unCM0_INT0_STATUS.u32Register;
#else
    system_int_status.u32Register = CPUSS->unCM4_INT0_STATUS.u32Register;
#endif

    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
    {
       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    }
    
    NVIC_ClearPendingIRQ(CPUIntIdx0_IRQn);
}


void CpuUserInt1_Handler(void)
{
    un_CPUSS_CM0_INT1_STATUS_t system_int_status = {0};

#if CY_CPU_CORTEX_M0P
    system_int_status.u32Register = CPUSS->unCM0_INT1_STATUS.u32Register;
#else
    system_int_status.u32Register = CPUSS->unCM4_INT1_STATUS.u32Register;
#endif

    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
    {
       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    }
    
    NVIC_ClearPendingIRQ(CPUIntIdx1_IRQn);
}


void CpuUserInt2_Handler(void)
{
    un_CPUSS_CM0_INT2_STATUS_t system_int_status = {0};

#if CY_CPU_CORTEX_M0P
    system_int_status.u32Register = CPUSS->unCM0_INT2_STATUS.u32Register;
#else
    system_int_status.u32Register = CPUSS->unCM4_INT2_STATUS.u32Register;
#endif

    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
    {
       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    } 
    NVIC_ClearPendingIRQ(CPUIntIdx2_IRQn);
}


void CpuUserInt3_Handler(void)
{
//    un_CPUSS_CM0_INT3_STATUS_t system_int_status = {0};
//
//#if CY_CPU_CORTEX_M0P
//    system_int_status.u32Register = CPUSS->unCM0_INT3_STATUS.u32Register;
//#else
//    system_int_status.u32Register = CPUSS->unCM4_INT3_STATUS.u32Register;
//#endif
//
//    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
//    {
//       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
//    }
//    else
//    {
//          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
//          // interrupt flag but didn't clear the Pending flag at NVIC)
//    }
  
    motor_left_update_isr();
    
    NVIC_ClearPendingIRQ(CPUIntIdx3_IRQn);
}


void CpuUserInt4_Handler(void)
{
//    un_CPUSS_CM0_INT4_STATUS_t system_int_status = {0};
//
//#if CY_CPU_CORTEX_M0P
//    system_int_status.u32Register = CPUSS->unCM0_INT4_STATUS.u32Register;
//#else
//    system_int_status.u32Register = CPUSS->unCM4_INT4_STATUS.u32Register;
//#endif
//
//    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
//    {
//       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
//    }
//    else
//    {
//          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
//          // interrupt flag but didn't clear the Pending flag at NVIC)
//    }
  
    motor_right_update_isr();
    
    NVIC_ClearPendingIRQ(CPUIntIdx4_IRQn);
}


void CpuUserInt5_Handler(void)
{
    un_CPUSS_CM0_INT5_STATUS_t system_int_status = {0};

#if CY_CPU_CORTEX_M0P
    system_int_status.u32Register = CPUSS->unCM0_INT5_STATUS.u32Register;
#else
    system_int_status.u32Register = CPUSS->unCM4_INT5_STATUS.u32Register;
#endif

    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
    {
       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    }
    
    NVIC_ClearPendingIRQ(CPUIntIdx5_IRQn);
}


void CpuUserInt6_Handler(void)
{
    un_CPUSS_CM0_INT6_STATUS_t system_int_status = {0};

#if CY_CPU_CORTEX_M0P
    system_int_status.u32Register = CPUSS->unCM0_INT6_STATUS.u32Register;
#else
    system_int_status.u32Register = CPUSS->unCM4_INT6_STATUS.u32Register;
#endif

    if(system_int_status.stcField.u1SYSTEM_INT_VALID)
    {
       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    }
    
    NVIC_ClearPendingIRQ(CPUIntIdx6_IRQn);
}

void uart_isr_function(uint32 isr_num);

void CpuUserInt7_Handler(void)
{
//    un_CPUSS_CM0_INT7_STATUS_t system_int_status = {0};
//#if CY_CPU_CORTEX_M0P
//    system_int_status.u32Register = CPUSS->unCM0_INT7_STATUS.u32Register;
//#else
//    system_int_status.u32Register = CPUSS->unCM4_INT7_STATUS.u32Register;
//#endif

    if(CPUSS->unCM4_INT7_STATUS.stcField.u1SYSTEM_INT_VALID)
    {
        uart_isr_function(CPUSS->unCM4_INT7_STATUS.stcField.u10SYSTEM_INT_IDX);
//       Cy_SystemIrqUserTable[system_int_status.stcField.u10SYSTEM_INT_IDX](); // jump to system interrupt handler
    }
    else
    {
          // Triggered by SW or due to SW clear error (SW cleared a peripheral 
          // interrupt flag but didn't clear the Pending flag at NVIC)
    }
    
    
  
    NVIC_ClearPendingIRQ(CPUIntIdx7_IRQn);
}



/* [] END OF FILE */
