/***************************************************************************//**
* \file         cy_mpu.h
* \version      `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
* \platform     Microcontroller family or derivative or all platforms
* \brief        Provides an API declaration of the Cortex-M MPU driver
*
*******************************************************************************/

/*******************************************************************************
* Copyright 2017, Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/******************************************************************************/
/* Version History:                                                           */
/******************************************************************************/
/* $Log:$  2019-07-27 YOTS Setup and disable functions are available.         */

#ifndef _CY_MPU_H_
#define _CY_MPU_H_

#include <stdint.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CY_MPU_MAX_NUM     ((MPU->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos)


/** Specifies region size of the MPU region */
typedef enum
{
  #if (~CY_CPU_CORTEX_M0P)
    CY_MPU_SIZE_32B   = (0x04ul << MPU_RASR_SIZE_Pos), /**< 32 Byte (Only for M4/M7) */
    CY_MPU_SIZE_64B   = (0x05ul << MPU_RASR_SIZE_Pos), /**< 64 Byte (Only for M4/M7) */
    CY_MPU_SIZE_128B  = (0x06ul << MPU_RASR_SIZE_Pos), /**< 128 Byte (Only for M4/M7) */
  #endif
    CY_MPU_SIZE_256B  = (0x07ul << MPU_RASR_SIZE_Pos), /**< 256 Byte */
    CY_MPU_SIZE_512B  = (0x08ul << MPU_RASR_SIZE_Pos), /**< 512 Byte */
    CY_MPU_SIZE_1KB   = (0x09ul << MPU_RASR_SIZE_Pos), /**< 1K Byte */
    CY_MPU_SIZE_2KB   = (0x0Aul << MPU_RASR_SIZE_Pos), /**< 2K Byte */
    CY_MPU_SIZE_4KB   = (0x0Bul << MPU_RASR_SIZE_Pos), /**< 4K Byte */
    CY_MPU_SIZE_8KB   = (0x0Cul << MPU_RASR_SIZE_Pos), /**< 8K Byte */
    CY_MPU_SIZE_16KB  = (0x0Dul << MPU_RASR_SIZE_Pos), /**< 16K Byte */
    CY_MPU_SIZE_32KB  = (0x0Eul << MPU_RASR_SIZE_Pos), /**< 32K Byte */
    CY_MPU_SIZE_64KB  = (0x0Ful << MPU_RASR_SIZE_Pos), /**< 64K Byte */
    CY_MPU_SIZE_128KB = (0x10ul << MPU_RASR_SIZE_Pos), /**< 128K Byte */
    CY_MPU_SIZE_256KB = (0x11ul << MPU_RASR_SIZE_Pos), /**< 256K Byte */
    CY_MPU_SIZE_512KB = (0x12ul << MPU_RASR_SIZE_Pos), /**< 512K Byte */
    CY_MPU_SIZE_1MB   = (0x13ul << MPU_RASR_SIZE_Pos), /**< 1M Byte */
    CY_MPU_SIZE_2MB   = (0x14ul << MPU_RASR_SIZE_Pos), /**< 2M Byte */
    CY_MPU_SIZE_4MB   = (0x15ul << MPU_RASR_SIZE_Pos), /**< 4M Byte */
    CY_MPU_SIZE_8MB   = (0x16ul << MPU_RASR_SIZE_Pos), /**< 8M Byte */
    CY_MPU_SIZE_16MB  = (0x17ul << MPU_RASR_SIZE_Pos), /**< 16M Byte */
    CY_MPU_SIZE_32MB  = (0x18ul << MPU_RASR_SIZE_Pos), /**< 32M Byte */
    CY_MPU_SIZE_64MB  = (0x19ul << MPU_RASR_SIZE_Pos), /**< 64M Byte */
    CY_MPU_SIZE_128MB = (0x1Aul << MPU_RASR_SIZE_Pos), /**< 128M Byte */
    CY_MPU_SIZE_256MB = (0x1Bul << MPU_RASR_SIZE_Pos), /**< 256M Byte */
    CY_MPU_SIZE_512MB = (0x1Cul << MPU_RASR_SIZE_Pos), /**< 512M Byte */
    CY_MPU_SIZE_1GB   = (0x1Dul << MPU_RASR_SIZE_Pos), /**< 1G Byte */
    CY_MPU_SIZE_2GB   = (0x1Eul << MPU_RASR_SIZE_Pos), /**< 2G Byte */
    CY_MPU_SIZE_4GB   = (0x1Ful << MPU_RASR_SIZE_Pos), /**< 4G Byte */
} cy_en_mpu_region_size_t;

/** Specifies access permission of the MPU region */
typedef enum
{
    CY_MPU_ACCESS_P_NO_ACCESS       = (0x0ul << MPU_RASR_AP_Pos), /**< All accesses generate a permission fault  */
    CY_MPU_ACCESS_P_PRIV_RW         = (0x1ul << MPU_RASR_AP_Pos), /**< Access from privileged software only  */
    CY_MPU_ACCESS_P_PRIV_RW_USER_RO = (0x2ul << MPU_RASR_AP_Pos), /**< Writes by unprivileged software generate a permission fault  */
    CY_MPU_ACCESS_P_FULL_ACCESS     = (0x3ul << MPU_RASR_AP_Pos), /**< Full access  */
    CY_MPU_ACCESS_P_PRIV_RO         = (0x5ul << MPU_RASR_AP_Pos), /**< Reads by privileged software only  */
    CY_MPU_ACCESS_P_RO              = (0x6ul << MPU_RASR_AP_Pos), /**< Read only, by privileged or unprivileged software  */
} cy_en_mpu_access_p_t;

/** Specifies attribute of the MPU region */
typedef enum
{
    CY_MPU_ATTR_STR_ORD_DEV               = ((0x0ul << MPU_RASR_TEX_Pos)                                                   ), /**< Strongly-ordered, shareable. */
    CY_MPU_ATTR_SHR_DEV                   = ((0x0ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk                 ), /**< Device,           shareable. */

    CY_MPU_ATTR_NORM_MEM_WT               = ((0x0ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                                  ), /**< Normal,           Not shareable, Outer and inner write-through. No write allocate. */
    CY_MPU_ATTR_NORM_SHR_MEM_WT           = ((0x0ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                  | MPU_RASR_S_Msk), /**< Normal,           shareable,     Outer and inner write-through. No write allocate. */
    CY_MPU_ATTR_NORM_MEM_WB               = ((0x0ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal,           Not shareable, Outer and inner write-back. No write allocate. */
    CY_MPU_ATTR_NORM_SHR_MEM_WB           = ((0x0ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal,           shareable,     Outer and inner write-back. No write allocate. */

    CY_MPU_ATTR_NORM_MEM_NC               = ((0x1ul << MPU_RASR_TEX_Pos)                                                   ), /**< Normal,           Not shareable, Outer and inner non-cacheable. */
    CY_MPU_ATTR_NORM_SHR_MEM_NC           = ((0x1ul << MPU_RASR_TEX_Pos)                                   | MPU_RASR_S_Msk), /**< Normal,           shareable,     Outer and inner non-cacheable. */
    CY_MPU_ATTR_NORM_MEM_WA               = ((0x1ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal,           Not shareable, Outer and inner Write-Back. Write and read allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_WA           = ((0x1ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal,           shareable,     Outer and inner Write-Back. Write and read allocate */

    CY_MPU_ATTR_DEV                       = ((0x2ul << MPU_RASR_TEX_Pos)                                                   ), /**< Device,           Not-shareable, */

    // Attribute for different policy for outer and inter cache.
    CY_MPU_ATTR_NORM_MEM_IN_NC_OUT_NC     = ((0x4ul << MPU_RASR_TEX_Pos)                                                   ), /**< Normal, Not-shareable. inner policy: Non-cachealbe,                      outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_MEM_IN_WA_OUT_NC     = ((0x4ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, write and read-Allocate outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_MEM_IN_WT_OUT_NC     = ((0x4ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                                  ), /**< Normal, Not-shareable. inner policy: Write through, no Write-Allocate,   outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_MEM_IN_WB_OUT_NC     = ((0x4ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, no Write-Allocate,      outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_NC_OUT_NC = ((0x4ul << MPU_RASR_TEX_Pos)                                   | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Non-cachealbe,                      outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WA_OUT_NC = ((0x4ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, write and read-Allocate outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WT_OUT_NC = ((0x4ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                  | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write through, no Write-Allocate,   outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WB_OUT_NC = ((0x4ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, no Write-Allocate,      outer policy: Non-cacheable */
    CY_MPU_ATTR_NORM_MEM_IN_NC_OUT_WA     = ((0x5ul << MPU_RASR_TEX_Pos)                                                   ), /**< Normal, Not-shareable. inner policy: Non-cachealbe,                      outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WA_OUT_WA     = ((0x5ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, write and read-Allocate outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WT_OUT_WA     = ((0x5ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                                  ), /**< Normal, Not-shareable. inner policy: Write through, no Write-Allocate,   outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WB_OUT_WA     = ((0x5ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, no Write-Allocate,      outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_NC_OUT_WA = ((0x5ul << MPU_RASR_TEX_Pos)                                   | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Non-cachealbe,                      outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WA_OUT_WA = ((0x5ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, write and read-Allocate outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WT_OUT_WA = ((0x5ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                  | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write through, no Write-Allocate,   outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WB_OUT_WA = ((0x5ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, no Write-Allocate,      outer policy: Write back, write and read-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_NC_OUT_WT     = ((0x6ul << MPU_RASR_TEX_Pos)                                                   ), /**< Normal, Not-shareable. inner policy: Non-cachealbe,                      outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WA_OUT_WT     = ((0x6ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, write and read-Allocate outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WT_OUT_WT     = ((0x6ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                                  ), /**< Normal, Not-shareable. inner policy: Write through, no Write-Allocate,   outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WB_OUT_WT     = ((0x6ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, no Write-Allocate,      outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_NC_OUT_WT = ((0x6ul << MPU_RASR_TEX_Pos)                                   | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Non-cachealbe,                      outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WA_OUT_WT = ((0x6ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, write and read-Allocate outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WT_OUT_WT = ((0x6ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                  | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write through, no Write-Allocate,   outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WB_OUT_WT = ((0x6ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, no Write-Allocate,      outer policy: Write through, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_NC_OUT_WB     = ((0x7ul << MPU_RASR_TEX_Pos)                                                   ), /**< Normal, Not-shareable. inner policy: Non-cachealbe,                      outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WA_OUT_WB     = ((0x7ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, write and read-Allocate outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WT_OUT_WB     = ((0x7ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                                  ), /**< Normal, Not-shareable. inner policy: Write through, no Write-Allocate,   outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_MEM_IN_WB_OUT_WB     = ((0x7ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk                 ), /**< Normal, Not-shareable. inner policy: Write back, no Write-Allocate,      outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_NC_OUT_WB = ((0x7ul << MPU_RASR_TEX_Pos)                                   | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Non-cachealbe,                      outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WA_OUT_WB = ((0x7ul << MPU_RASR_TEX_Pos)                  | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, write and read-Allocate outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WT_OUT_WB = ((0x7ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk                  | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write through, no Write-Allocate,   outer policy: Write back, no Write-Allocate */
    CY_MPU_ATTR_NORM_SHR_MEM_IN_WB_OUT_WB = ((0x7ul << MPU_RASR_TEX_Pos) | MPU_RASR_C_Msk | MPU_RASR_B_Msk | MPU_RASR_S_Msk), /**< Normal, shareable.     inner policy: Write back, no Write-Allocate,      outer policy: Write back, no Write-Allocate */
} cy_en_mpu_attr_t;

/** Specifies enable/disable of instruction access of the MPU region */
typedef enum
{
    CY_MPU_INST_ACCESS_EN  = (0ul),             /**< Instruction fetches enabled  */
    CY_MPU_INST_ACCESS_DIS = (MPU_RASR_XN_Msk), /**< Instruction fetches disabled */
} cy_en_mpu_execute_n_t;

/** Specifies enable/disable of the MPU region */
typedef enum
{
    CY_MPU_DISABLE = (0ul),                 /**< Disable  */
    CY_MPU_ENABLE  = (MPU_RASR_ENABLE_Msk), /**< Enable  */
} cy_en_mpu_region_en_t;

/** Specifies structure of configuration parameter of Cortex-M MPU */
typedef struct
{
    uint32_t                addr;       /**< The base address of the MPU region.
                                             The base address is aligned to the size of the region.
                                             For example, a 64KB region must be aligned on a multiple of 64KB,
                                             for example, at 0x00010000 or 0x00020000. */
    cy_en_mpu_region_size_t size;       /**< The size of the MPU region \ref cy_en_mpu_region_size_t */
    cy_en_mpu_access_p_t    permission; /**< The access permission of the MPU region \ref cy_en_mpu_access_p_t */
    cy_en_mpu_attr_t        attribute;  /**< The attribute of the MPU region \ref cy_en_mpu_attr_t */
    cy_en_mpu_execute_n_t   execute;    /**< Instruction access disable/enable select \ref cy_en_mpu_region_xn_t */
    uint8_t                 srd;        /**< Subregion disable bits. For each bit in this field.
                                             Regions of 256 bytes or more are divided into eight equal-sized subregions. Set the 
                                             corresponding bit in this parameter to disable a subregion.
                                               0: Corresponding sub-region is enabled.
                                               1: Corresponding sub-region is disabled.
                                             If MPU region size is less than 256 byte, this value will be ignored. */
    cy_en_mpu_region_en_t   enable;     /**< enable/disable the MPU region \ref cy_en_mpu_region_en_t */
} cy_stc_mpu_region_cfg_t;

/** Specifies enable/disable privileged software access to the default memory map */
typedef enum
{
    CY_MPU_DISABLE_USE_DEFAULT_MAP = (0ul),                     /**< If the MPU is enabled, disables use of the default memory map.
                                                                     Any memory access to a location not covered by any enabled
                                                                     region causes a fault.  */
    CY_MPU_USE_DEFAULT_MAP_AS_BG   = (MPU_CTRL_PRIVDEFENA_Msk), /**< If the MPU is enabled, enables use of the default memory map
                                                                     as a background region for privileged software accesses.  */
} cy_en_mpu_privdefena_t;

/** Specifies enable/disable the operation of MPU during hard fault, NMI, and FAULTMASK handlers. */
typedef enum
{
    CY_MPU_DISABLED_DURING_FAULT_NMI = (0ul),                    /**< MPU is disabled during hard fault, NMI, and FAULTMASK handlers,
                                                                      regardless of the value of the ENABLE bit.  */
    CY_MPU_ENABLED_DURING_FAULT_NMI  = (MPU_CTRL_HFNMIENA_Msk), /**< The MPU is enabled during hard fault, NMI, and FAULTMASK handlers.  */
} cy_en_mpu_hfnmiena_t;

/** Specifies enable/disable of the MPU */
typedef enum
{
    CY_MPU_GLOBAL_DISABLE = (0ul),                 /**< Disable  */
    CY_MPU_GLOBAL_ENABLE  = (MPU_CTRL_ENABLE_Msk), /**< Enable  */
} cy_en_mpu_global_en_t;


/** Contains the 3 global MPU control bits */
typedef struct
{
    cy_en_mpu_global_en_t   mpuGlobalEnable; /**< Global MPU enable */
    cy_en_mpu_privdefena_t  privDefMapEn;    /**< MPU default memory map for privileged accesses enabled */
    cy_en_mpu_hfnmiena_t    faultNmiEn;      /**< MPU enabled or disabled during Hard Faults and NMIs */
} cy_stc_mpu_global_ctrl_bits_t;

/** Specifies status of APIs of this driver */
typedef enum
{
    CY_MPU_SUCCESS = 0ul, /**< The API completed in success */
    CY_MPU_FAILURE,     /**< The API finished in failure */
} cy_en_mpu_status_t;



extern cy_en_mpu_status_t Cy_MPU_Setup(const cy_stc_mpu_region_cfg_t cfg[], uint8_t cfgSize, cy_en_mpu_privdefena_t privDefMapEn, cy_en_mpu_hfnmiena_t faultNmiEn);
extern void Cy_MPU_Enable(void);
extern void Cy_MPU_Disable(void);
extern cy_en_mpu_status_t Cy_MPU_SetRegion(const cy_stc_mpu_region_cfg_t * cfg, uint8_t regionNr);
extern cy_en_mpu_status_t Cy_MPU_GetRegion(cy_stc_mpu_region_cfg_t * cfg, uint8_t regionNr);
extern void Cy_MPU_GetGlobalControlBits(cy_stc_mpu_global_ctrl_bits_t * mpuGlobalCtrl);
extern void Cy_MPU_SetGlobalControlBits(const cy_stc_mpu_global_ctrl_bits_t * mpuGlobalCtrl);


#ifdef __cplusplus
}
#endif

#endif /* _CY_MPU_H_ */


/* [] END OF FILE */
