/***************************************************************************//**
* \file cy_sysflt_config.h
* \version 1.0
*
* \brief
* Provides the faults supported by the device via the structure. To be used
* with Sysfault driver module APIs.
*
* Attention: Automatically generated, do not modify anything manually!
*
********************************************************************************
* \copyright
* Copyright 2018-2021, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sysflt System Fault (SysFlt)
* \{
* The SysFault driver provides an API to configure the fault reporting structure.
*
* \section group_sysflt_more_information More Information
*
* Refer to the technical reference manual (TRM) and the device datasheet.
* 
* \defgroup group_sysflt_enums Enumerated Types
*/


#if !defined(CY_SYSFLT_CONFIG_H)
#define CY_SYSFLT_CONFIG_H

#if defined(__cplusplus)
extern "C" {
#endif


/***************************************
*       Enumeration
***************************************/

/**
* \addtogroup group_sysflt_enums
* \{
*/

/**
* SysFlt Driver errors
*/
typedef enum 
{
    CY_SYSFLT_CPUSS_MPU_VIO_0                         = 0x0000u,    /**< CM0+ MPU/SMPU violation */
    CY_SYSFLT_CPUSS_MPU_VIO_1                         = 0x0001u,    /**< CRYPTO MPU/SMPU violation */
    CY_SYSFLT_CPUSS_MPU_VIO_2                         = 0x0002u,    /**< DW0 MPU/SMPU violation */
    CY_SYSFLT_CPUSS_MPU_VIO_3                         = 0x0003u,    /**< DW1 MPU/SMPU violation */
    CY_SYSFLT_CPUSS_MPU_VIO_4                         = 0x0004u,    /**< Bus master #4 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_5                         = 0x0005u,    /**< Bus master #5 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_9                         = 0x0009u,    /**< Bus master #9 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_10                        = 0x000Au,    /**< Bus master #10 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_13                        = 0x000Du,    /**< Bus master #13 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_14                        = 0x000Eu,    /**< Bus master #14 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_MPU_VIO_15                        = 0x000Fu,    /**< Bus master #15 MPU/SMPU violation (when used) */
    CY_SYSFLT_CPUSS_CM7_1_TCM_C_ECC                   = 0x0010u,    /**< Correctable ECC error in CM7_1 TCM memory. See CM7_0_TCM_C_ECC descripption. */
    CY_SYSFLT_CPUSS_CM7_1_TCM_NC_ECC                  = 0x0011u,    /**< Non Correctable ECC error in CM7_1 TCM memory. See CM7_0_TCM_C_ECC descripption. */
    CY_SYSFLT_CPUSS_CM7_0_CACHE_C_ECC                 = 0x0012u,    /**< Correctable ECC error in CM7_0 Cache memories
                                                                           DATA0[16:2]: location information: Tag/Data SRAM, Way, Index and  line Offset, see CM7 UGRM IEBR0/DEBR0 description for details. 
                                                                           DATA0[31]: 0=Instruction cache, 1= Data cache */
    CY_SYSFLT_CPUSS_CM7_0_CACHE_NC_ECC                = 0x0013u,    /**< Non Correctable ECC error in CM7_0 Cache memories. See CM7_0_CACHE_C_ECC descripption */
    CY_SYSFLT_CPUSS_CM7_1_CACHE_C_ECC                 = 0x0014u,    /**< Correctable ECC error in CM7_1 Cache memories. See CM7_0_CACHE_C_ECC descripption. */
    CY_SYSFLT_CPUSS_CM7_1_CACHE_NC_ECC                = 0x0015u,    /**< Non Correctable ECC error in CM7_1 Cache memories. See CM7_0_CACHE_C_ECC descripption. */
    CY_SYSFLT_PERI_MS_VIO_4                           = 0x0019u,    /**< Peripheral interconnect, master interface 4 PPU. See MS_PPU_0 description. */
    CY_SYSFLT_PERI_PERI_C_ECC                         = 0x001Au,    /**< Peri protection SRAM correctable ECC violation */
    CY_SYSFLT_PERI_PERI_NC_ECC                        = 0x001Bu,    /**< Peri protection SRAM non-correctable ECC violation */
    CY_SYSFLT_PERI_MS_VIO_0                           = 0x001Cu,    /**< CM0+ Peripheral Master Interface PPU violation */
    CY_SYSFLT_PERI_MS_VIO_1                           = 0x001Du,    /**< CM7_0 Peripheral Master Interface PPU violation */
    CY_SYSFLT_PERI_MS_VIO_2                           = 0x001Eu,    /**< CM7_1 Peripheral Master Interface PPU violation */
    CY_SYSFLT_PERI_MS_VIO_3                           = 0x001Fu,    /**< DW0 Peripheral Master Interface PPU violation */
    CY_SYSFLT_PERI_GROUP_VIO_0                        = 0x0020u,    /**< Peripheral group #0 violation */
    CY_SYSFLT_PERI_GROUP_VIO_1                        = 0x0021u,    /**< Peripheral group #1 violation */
    CY_SYSFLT_PERI_GROUP_VIO_2                        = 0x0022u,    /**< Peripheral group #2 violation */
    CY_SYSFLT_PERI_GROUP_VIO_3                        = 0x0023u,    /**< Peripheral group #3 violation */
    CY_SYSFLT_PERI_GROUP_VIO_4                        = 0x0024u,    /**< Peripheral group #4 violation */
    CY_SYSFLT_PERI_GROUP_VIO_5                        = 0x0025u,    /**< Peripheral group #5 violation */
    CY_SYSFLT_PERI_GROUP_VIO_6                        = 0x0026u,    /**< Peripheral group #6 violation */
    CY_SYSFLT_PERI_GROUP_VIO_8                        = 0x0028u,    /**< Peripheral group #8 violation */
    CY_SYSFLT_PERI_GROUP_VIO_9                        = 0x0029u,    /**< Peripheral group #9 violation */
    CY_SYSFLT_CPUSS_FLASHC_MAIN_BUS_ERR               = 0x0030u,    /**< Flash controller main flash bus error */
    CY_SYSFLT_CPUSS_FLASHC_MAIN_C_ECC                 = 0x0031u,    /**< Flash controller main flash correctable ECC violation */
    CY_SYSFLT_CPUSS_FLASHC_MAIN_NC_ECC                = 0x0032u,    /**< Flash controller main flash non-correctable ECC violation */
    CY_SYSFLT_CPUSS_FLASHC_WORK_BUS_ERR               = 0x0033u,    /**< Flash controller work flash bus error */
    CY_SYSFLT_CPUSS_FLASHC_WORK_C_ECC                 = 0x0034u,    /**< Flash controller work flash correctable ECC violation */
    CY_SYSFLT_CPUSS_FLASHC_WORK_NC_ECC                = 0x0035u,    /**< Flash controller work flash non-correctable ECC violation */
    CY_SYSFLT_CPUSS_FLASHC_CM0_CA_C_ECC               = 0x0036u,    /**< Flash controller CM0+ cache correctable ECC violation */
    CY_SYSFLT_CPUSS_FLASHC_CM0_CA_NC_ECC              = 0x0037u,    /**< Flash controller CM0+ cache non-correctable ECC violation */
    CY_SYSFLT_CPUSS_CM7_0_TCM_C_ECC                   = 0x0038u,    /**< Correctable ECC error in CM7_0 TCM memory
                                                                           DATA0[23:2]: Violating address.
                                                                           DATA1[7:0]: Syndrome of code word (at address offset 0x0).
                                                                           DATA1[31:30]: 0= ITCM, 2=D0TCM, 3=D1TCM */
    CY_SYSFLT_CPUSS_CM7_0_TCM_NC_ECC                  = 0x0039u,    /**< Non Correctable ECC error in CM7_0 TCM memory. See CM7_0_TCM_C_ECC descripption. */
    CY_SYSFLT_CPUSS_RAMC0_C_ECC                       = 0x003Au,    /**< System SRAM 0 correctable ECC error:
                                                                           DATA0[31:0]: Violating address.
                                                                           DATA1[6:0]: Syndrome of 32-bit SRAM code word. */
    CY_SYSFLT_CPUSS_RAMC0_NC_ECC                      = 0x003Bu,    /**< System memory controller 0 non-correctable ECC violation */
    CY_SYSFLT_CPUSS_RAMC1_C_ECC                       = 0x003Cu,    /**< System memory controller 1 correctable ECC violation */
    CY_SYSFLT_CPUSS_RAMC1_NC_ECC                      = 0x003Du,    /**< System memory controller 1 non-correctable ECC violation */
    CY_SYSFLT_CPUSS_RAMC2_C_ECC                       = 0x003Eu,    /**< System memory controller 2 correctable ECC violation */
    CY_SYSFLT_CPUSS_RAMC2_NC_ECC                      = 0x003Fu,    /**< System memory controller 2 non-correctable ECC violation */
    CY_SYSFLT_CPUSS_CRYPTO_C_ECC                      = 0x0040u,    /**< Crypto memory correctable ECC violation */
    CY_SYSFLT_CPUSS_CRYPTO_NC_ECC                     = 0x0041u,    /**< Crypto memory non-correctable ECC violation */
    CY_SYSFLT_CPUSS_DW0_C_ECC                         = 0x0046u,    /**< DataWire 0 SRAM 1 correctable ECC error:
                                                                           DATA0[11:0]: Violating DW SRAM address (word address, assuming byte addressible).
                                                                           DATA1[6:0]: Syndrome of 32-bit SRAM code word. */
    CY_SYSFLT_CPUSS_DW0_NC_ECC                        = 0x0047u,    /**< Datawire0 memory non-correctable ECC violation */
    CY_SYSFLT_CPUSS_DW1_C_ECC                         = 0x0048u,    /**< Datawire1 memory correctable ECC violation */
    CY_SYSFLT_CPUSS_DW1_NC_ECC                        = 0x0049u,    /**< Datawire1 memory non-correctable ECC violation */
    CY_SYSFLT_CPUSS_FM_SRAM_C_ECC                     = 0x004Au,    /**< Flash code storage sram memory correctable ECC violation */
    CY_SYSFLT_CPUSS_FM_SRAM_NC_ECC                    = 0x004Bu,    /**< Flash code storage sram memory non-correctable ECC violation */
    CY_SYSFLT_CANFD_0_CAN_C_ECC                       = 0x0050u,    /**< CAN0 message buffer correctable ECC violation */
    CY_SYSFLT_CANFD_0_CAN_NC_ECC                      = 0x0051u,    /**< CAN0 message buffer non-correctable ECC violation */
    CY_SYSFLT_CANFD_1_CAN_C_ECC                       = 0x0052u,    /**< CAN1 message buffer correctable ECC violation */
    CY_SYSFLT_CANFD_1_CAN_NC_ECC                      = 0x0053u,    /**< CAN1 message buffer non-correctable ECC violation */
    CY_SYSFLT_SRSS_FAULT_CSV                          = 0x005Au,    /**< SRSS Clock SuperVisor (CSV) violation detected. Multiple CSV can detect a violation at the same time.
                                                                           DATA0[15:0]: clk_hf* root CSV violation flags.
                                                                           DATA0[24]: clk_ref CSV violation flag (reference clock for clk_hf CSVs)
                                                                           DATA0[25]: clk_lf CSV violation flag
                                                                           DATA0[26]: clk_hvilo CSV violation flag */
    CY_SYSFLT_SRSS_FAULT_SSV                          = 0x005Bu,    /**< SRSS Clock SuperVisor (CSV) violation detected. Multiple CSV can detect a violation at the same time.
                                                                           DATA0[0]: BOD on VDDA
                                                                           DATA[1]: OVD on VDDA
                                                                           DATA[16]: LVD/HVD #1
                                                                           DATA0[17]: LVD/HVD #2 */
    CY_SYSFLT_SRSS_FAULT_MCWDT0                       = 0x005Cu,    /**< Fault output for MCWDT0 (all sub-counters) */
    CY_SYSFLT_SRSS_FAULT_MCWDT1                       = 0x005Du,    /**< Fault output for MCWDT1 (all sub-counters) */
    CY_SYSFLT_SRSS_FAULT_MCWDT2                       = 0x005Eu,    /**< Fault output for MCWDT2 (all sub-counters) */
	CY_SYSFLT_NO_FAULT
	
} cy_en_sysflt_source_t;
    
/** \} group_sysflt_enums */

#if defined(__cplusplus)
}
#endif

#endif /* CY_SYSFLT_CONFIG_H */

/* [] END OF FILE */
