/***************************************************************************//**
* \file cy_prot.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Protection Unit driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_prot Protection Unit (PROT)
* \{
*
* The Protection Unit driver provides an API to configure the Memory Protection
* Units (MPU), Shared Memory Protection Units (SMPU), and Peripheral Protection
* Units (PPU). These are separate from the ARM Core MPUs and provide additional
* mechanisms for securing resource accesses. The Protection units address the 
* following concerns in an embedded design:
* - Security requirements. This includes the prevention of malicious attacks
*   to access secure memory or peripherals.
* - Safety requirements. This includes detection of accidental (non-malicious)
*   SW errors and random HW errors. It is important to enable failure analysis
*   to investigate the root cause of a safety violation.
* 
* \n
* <b> Memory Protection </b>
*
* Memory protection is provided by MPUs and SMPUs and allows control of 
* memory accesses by bus masters (e.g., CM0+, CM4, Crypto):
* - MPUs are used to distinguish user and privileged accesses from a single bus 
*   master such as task switching in an OS/kernel. For ARM core MPUs, the 
*   assigned Cypress MPUs for those cores provide additional functionality 
*   such as protection contexts (PC) and security attributes.
* - SMPUs are used to distinguish memory accesses between bus masters. This is
*   achieved by using protection contexts and secure-/non-secure attributes.
*
* \n
* <b> Peripheral Protection </b>
*
* Peripheral protection is provided by PPUs and allows control of peripheral
* register accesses by bus masters. Four types of PPUs are available.
* - Programmable (PROG) PPUs are used to protect any peripheral memory region
*   in a device from invalid bus master accesses. It is the most versatile
*   type of peripheral protection unit.
* - Fixed Group (GR) PPUs are used to protect an entire peripheral MMIO group
*   from invalid bus master accesses. The MMIO grouping information and which
*   resource belongs to which group is device specific and can be obtained 
*   from the device technical reference manual (TRM).
* - Fixed Region (RG) PPUs are used to protect an entire peripheral slave 
*   instance from invalid bus master accesses. For example, TCPWM0, TCPWM1, 
*   SCB0, and SCB1, etc.
* - Fixed Slave (SL) PPUs are used to protect specified regions of peripheral
*   instances. For example, individual DW channel structs, SMPU structs, and 
*   IPC structs, etc.
*
* \n
* <b> External Memory Protection </b>
*
* The External (SDRAM) Memory Protection Unit (EMPU) guards the external LPDDR4 
* SDRAM against unauthorized accesses via the AXI ports. As the external SDRAM 
* can only be accessed via the AXI ports, there is no need for protecting the 
* SDRAM against accesses via the AHB port.
* - The functionality and register model are based on the Shared Memory 
*   Protection Unit (SMPU).
* - Unauthorized accesses cause a bus error and are reported as a fault.
*
* \n
* <b> Driver Usage </b>
*
* Each protection unit is distinguished by its type (e.g. 
* stc_PROT_MPU_MPU_STRUCT_t). The list of supported protection units can be
* obtained from the device header file. Choose a protection unit of interest,
* and call its corresponding Cy_Prot_Config<X>Struct() function with its
* software protection unit configuration structure populated. Then enable the
* protection unit by calling the Cy_Prot_Enable<X>Struct() function.
*
* Each bus master can be configured to allow/disallow certain types of accesses
* by calling the Cy_Prot_ConfigBusMaster() function. For example, CM0+ core 
* is a bus master. By configuring it to allow only protection contexts (PC) 
* 2 and 3, the bus master will be able to set its protection context only to
* 2 or 3. In a thread execution, the CM0+ core can set its protection context
* to 2 by calling Cy_Prot_SetActivePC() and access all regions of memory that
* allow PC=2. A fault will be triggered if a resource is protected with different
* protection settings.
*
* \section group_prot_configuration Configuration Considerations
*
* With the exception of MPUs, Protection structures are implemented in 
* hardware as master and slave pairs. The master protection structure protects
* the slave protection structure. The slave protection structure protects the
* memory region that is to be protected.
* For example, a slave protection structure is configured to allow protection
* contexts 3 and 4 to access a set of peripheral registers. The corresponding
* master protection structure can then be used to protect the slave structure
* configuration and allow only protection context 1 to make modifications.
* Any bus master attempting to change the slave protection structure must then
* have a protection context 1. Otherwise an access fault will occur.
*
* \section group_prot_more_information More Information
*
* Refer to Technical Reference Manual (TRM) and the device datasheet.
*
* \section group_prot_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.1</td>
*     <td>Remove old commented code</td>
*     <td>Added ISB instruction after enable/disable SMPU structure</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.2</td>
*     <td>Remove pc_save accessing code</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.3</td>
*     <td>Added comments for cy_en_prot_pc_t</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.4</td>
*     <td>Added functions and code for empu</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_prot_macro Macro
* \defgroup group_prot_functions Functions
* \{
*   \defgroup group_prot_functions_busmaster   Bus Master and PC Functions
*   \defgroup group_prot_functions_mpu         MPU Functions
*   \defgroup group_prot_functions_smpu        SMPU Functions
*   \defgroup group_prot_functions_ppu_prog    PPU Programmable (PROG) Functions
*   \defgroup group_prot_functions_ppu_gr      PPU Group (GR) Functions
*   \defgroup group_prot_functions_ppu_sl      PPU Slave (SL) Functions
*   \defgroup group_prot_functions_ppu_rg      PPU Region (RG) Functions
*   \defgroup group_prot_functions_empu        EMPU Functions
* \}
* \defgroup group_prot_data_structures Data structures
* \defgroup group_prot_enums Enumerated types
*/

#if !defined(__CY_PROT_H__)
#define __CY_PROT_H__

#include <stdbool.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** \addtogroup group_prot_macro
* \{
*/

/** Driver major version */
#define CY_PROT_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_PROT_DRV_VERSION_MINOR       0

/** Prot driver ID */
#define CY_PROT_ID CY_PDL_DRV_ID(0x30u)

/** \} group_prot_macro */

/**
* \addtogroup group_prot_enums
* \{
*/

/**
* Prot Driver error codes
*/
typedef enum 
{
    CY_PROT_SUCCESS   = 0x00u,                                    /**< Returned successful */
    CY_PROT_BAD_PARAM = CY_PROT_ID | CY_PDL_STATUS_ERROR | 0x01u, /**< Bad parameter was passed */
    CY_PROT_FAILURE   = CY_PROT_ID | CY_PDL_STATUS_ERROR | 0x03u  /**< The resource is locked */
} cy_en_prot_status_t;

/**
* User/Privileged permission
*/
typedef enum 
{
    CY_PROT_PERM_DISABLED = 0x00u, /**< Read, Write and Execute disabled */
    CY_PROT_PERM_R        = 0x01u, /**< Read enabled */
    CY_PROT_PERM_W        = 0x02u, /**< Write enabled */
    CY_PROT_PERM_RW       = 0x03u, /**< Read and Write enabled */
    CY_PROT_PERM_X        = 0x04u, /**< Execute enabled */
    CY_PROT_PERM_RX       = 0x05u, /**< Read and Execute enabled */
    CY_PROT_PERM_WX       = 0x06u, /**< Write and Execute enabled */
    CY_PROT_PERM_RWX      = 0x07u  /**< Read, Write and Execute enabled */
}cy_en_prot_perm_t;

/**
* Memory region size
*/
typedef enum 
{
    CY_PROT_SIZE_4B    = 1u,  /**< 4 bytes */
    CY_PROT_SIZE_8B    = 2u,  /**< 8 bytes */
    CY_PROT_SIZE_16B   = 3u,  /**< 16 bytes */
    CY_PROT_SIZE_32B   = 4u,  /**< 32 bytes */
    CY_PROT_SIZE_64B   = 5u,  /**< 64 bytes */  
    CY_PROT_SIZE_128B  = 6u,  /**< 128 bytes */
    CY_PROT_SIZE_256B  = 7u,  /**< 256 bytes */
    CY_PROT_SIZE_512B  = 8u,  /**< 512 bytes */
    CY_PROT_SIZE_1KB   = 9u,  /**< 1 Kilobyte */
    CY_PROT_SIZE_2KB   = 10u, /**< 2 Kilobytes */
    CY_PROT_SIZE_4KB   = 11u, /**< 4 Kilobytes */
    CY_PROT_SIZE_8KB   = 12u, /**< 8 Kilobytes */
    CY_PROT_SIZE_16KB  = 13u, /**< 16 Kilobytes */
    CY_PROT_SIZE_32KB  = 14u, /**< 32 Kilobytes */
    CY_PROT_SIZE_64KB  = 15u, /**< 64 Kilobytes */
    CY_PROT_SIZE_128KB = 16u, /**< 128 Kilobytes */
    CY_PROT_SIZE_256KB = 17u, /**< 256 Kilobytes */
    CY_PROT_SIZE_512KB = 18u, /**< 512 Kilobytes */
    CY_PROT_SIZE_1MB   = 19u, /**< 1 Megabyte */
    CY_PROT_SIZE_2MB   = 20u, /**< 2 Megabytes */
    CY_PROT_SIZE_4MB   = 21u, /**< 4 Megabytes */
    CY_PROT_SIZE_8MB   = 22u, /**< 8 Megabytes */
    CY_PROT_SIZE_16MB  = 23u, /**< 16 Megabytes */
    CY_PROT_SIZE_32MB  = 24u, /**< 32 Megabytes */
    CY_PROT_SIZE_64MB  = 25u, /**< 64 Megabytes */
    CY_PROT_SIZE_128MB = 26u, /**< 128 Megabytes */
    CY_PROT_SIZE_256MB = 27u, /**< 256 Megabytes */
    CY_PROT_SIZE_512MB = 28u, /**< 512 Megabytes */
    CY_PROT_SIZE_1GB   = 29u, /**< 1 Gigabyte */
    CY_PROT_SIZE_2GB   = 30u  /**< 2 Gigabytes */
}cy_en_prot_size_t;

/**
* Protection Context (PC)
*/
typedef enum 
{
    CY_PROT_PC0  = 0u,  /**< PC = 0, which cannot be configured */
    CY_PROT_PC1  = 1u,  /**< PC = 1 */
    CY_PROT_PC2  = 2u,  /**< PC = 2 */
    CY_PROT_PC3  = 3u,  /**< PC = 3 */
    CY_PROT_PC4  = 4u,  /**< PC = 4 */
    CY_PROT_PC5  = 5u,  /**< PC = 5 */
    CY_PROT_PC6  = 6u,  /**< PC = 6 */
    CY_PROT_PC7  = 7u,  /**< PC = 7 */
    CY_PROT_PC8  = 8u,  /**< PC = 8 */
    CY_PROT_PC9  = 9u,  /**< PC = 9 */
    CY_PROT_PC10 = 10u, /**< PC = 10 */
    CY_PROT_PC11 = 11u, /**< PC = 11 */
    CY_PROT_PC12 = 12u, /**< PC = 12 */
    CY_PROT_PC13 = 13u, /**< PC = 13 */
    CY_PROT_PC14 = 14u, /**< PC = 14 */
    CY_PROT_PC15 = 15u,  /**< PC = 15 */
    CY_PROT_PC_NUM
} cy_en_prot_pc_t;



/***************************************
*        Constants
***************************************/

/** \cond INTERNAL */

/* Helper function for finding max */
#define CY_PROT_MAX(x,y) (((x)>(y))?(x):(y))

/* General Masks and shifts */
#define	CY_PROT_MSX_CTL_SHIFT                 (0x02UL) /**< Shift for MSx_CTL register */
#define	CY_PROT_STRUCT_ENABLE                 (0x01UL) /**< Enable protection unit struct */
#define CY_PROT_ADDR_SHIFT                    (8UL)    /**< Address shift for MPU, SMPU and PROG PPU structs */
#define CY_PROT_ADDR_SHIFT_PPU                (2UL)    /**< Address shift for PROG PPU structs */

/* Permission masks and shifts */
#define	CY_PROT_ATT_PERMISSION_MASK           (0x07UL) /**< Protection Unit attribute permission mask */
#define	CY_PROT_ATT_USER_PERMISSION_SHIFT     (0x00UL) /**< Protection Unit user attribute permission shift */
#define	CY_PROT_ATT_PRIV_PERMISSION_SHIFT     (0x03UL) /**< Protection Unit priliged attribute permission shift */

/* Maximum Master Protection Context */
#define CY_PROT_MS_PC_NR_MAX                  CY_PROT_MAX(CPUSS_PROT_SMPU_MS0_PC_NR_MINUS1,     \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS1_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS2_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS3_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS4_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS5_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS6_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS7_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS8_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS9_PC_NR_MINUS1,   \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS10_PC_NR_MINUS1,  \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS11_PC_NR_MINUS1,  \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS12_PC_NR_MINUS1,  \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS13_PC_NR_MINUS1,  \
                                                CY_PROT_MAX(CPUSS_PROT_SMPU_MS14_PC_NR_MINUS1,  \
                                                            CPUSS_PROT_SMPU_MS15_PC_NR_MINUS1)))))))))))))))

/* Protection Context limit masks */
#define CY_PROT_MPU_PC_LIMIT_MASK             (0xFFFFFFFFUL << CY_PROT_MS_PC_NR_MAX)
#define CY_PROT_SMPU_PC_LIMIT_MASK            (0xFFFFFFFFUL << CPUSS_PROT_SMPU_MS15_PC_NR_MINUS1) //RMKN CPUSS_SMPU_STRUCT_PC_NR_MINUS1)
#define CY_PROT_PPU_PROG_PC_LIMIT_MASK        (0xFFFFFFFFUL << PERI_PPU_PROG_STRUCT_PC_NR_MINUS1)
#define CY_PROT_PPU_FIXED_PC_LIMIT_MASK       (0xFFFFFFFFUL << PERI_PPU_FIXED_STRUCT_PC_NR_MINUS1)

/* Parameter validation masks to check for read-only values */
#define CY_PROT_SMPU_ATT0_MASK                ((uint32_t)~(PROT_SMPU_SMPU_STRUCT_ATT0_PC_MASK_0_Msk))
#define CY_PROT_SMPU_ATT1_MASK                ((uint32_t)~(PROT_SMPU_SMPU_STRUCT_ATT1_UR_Msk \
                                                         | PROT_SMPU_SMPU_STRUCT_ATT1_UX_Msk \
                                                         | PROT_SMPU_SMPU_STRUCT_ATT1_PR_Msk \
                                                         | PROT_SMPU_SMPU_STRUCT_ATT1_PX_Msk \
                                                         | PROT_SMPU_SMPU_STRUCT_ATT1_PC_MASK_0_Msk \
                                                         | PROT_SMPU_SMPU_STRUCT_ATT1_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_PROG_ATT0_MASK            ((uint32_t)~(PERI_PPU_GR_ATT1_UX_Msk \
                                                       | PERI_PPU_GR_ATT1_PX_Msk \
                                                       |PERI_PPU_PR_ATT0_PC_MASK_0_Msk \
                                              ))
#define CY_PROT_PPU_PROG_ATT1_MASK            ((uint32_t)~(PERI_PPU_PR_ATT1_UR_Msk \
                                                       | PERI_PPU_PR_ATT1_UX_Msk \
                                                       | PERI_PPU_PR_ATT1_PR_Msk \
                                                       | PERI_PPU_PR_ATT1_PX_Msk \
                                                       | PERI_PPU_PR_ATT1_PC_MASK_0_Msk \
                                                       | PERI_PPU_PR_ATT1_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_GR_ATT0_MASK              ((uint32_t)~(PERI_PPU_GR_ATT0_UX_Msk \
                                                       | PERI_PPU_GR_ATT0_PX_Msk \
                                                       | PERI_PPU_GR_ATT0_PC_MASK_0_Msk \
                                                       | PERI_PPU_GR_ATT0_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_GR_ATT1_MASK              ((uint32_t)~(PERI_PPU_GR_ATT1_UR_Msk \
                                                       | PERI_PPU_GR_ATT1_UX_Msk \
                                                       | PERI_PPU_GR_ATT1_PR_Msk \
                                                       | PERI_PPU_GR_ATT1_PX_Msk \
                                                       | PERI_PPU_GR_ATT1_PC_MASK_0_Msk \
                                                       | PERI_PPU_GR_ATT1_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_SL_ATT0_MASK              ((uint32_t)~(PERI_GR_PPU_SL_ATT0_UX_Msk \
                                                       | PERI_GR_PPU_SL_ATT0_PX_Msk \
                                                       | PERI_GR_PPU_SL_ATT0_PC_MASK_0_Msk \
                                                       | PERI_GR_PPU_SL_ATT0_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_SL_ATT1_MASK              ((uint32_t)~(PERI_GR_PPU_SL_ATT1_UR_Msk \
                                                       | PERI_GR_PPU_SL_ATT1_UX_Msk \
                                                       | PERI_GR_PPU_SL_ATT1_PR_Msk \
                                                       | PERI_GR_PPU_SL_ATT1_PX_Msk \
                                                       | PERI_GR_PPU_SL_ATT1_PC_MASK_0_Msk \
                                                       | PERI_GR_PPU_SL_ATT1_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_RG_ATT0_MASK              ((uint32_t)~(PERI_GR_PPU_RG_ATT0_UX_Msk \
                                                       | PERI_GR_PPU_RG_ATT0_PX_Msk \
                                                       | PERI_GR_PPU_RG_ATT0_PC_MASK_0_Msk \
                                                       | PERI_GR_PPU_RG_ATT0_REGION_SIZE_Msk \
                                              ))
#define CY_PROT_PPU_RG_ATT1_MASK              ((uint32_t)~(PERI_GR_PPU_RG_ATT1_UR_Msk \
                                                       | PERI_GR_PPU_RG_ATT1_UX_Msk \
                                                       | PERI_GR_PPU_RG_ATT1_PR_Msk \
                                                       | PERI_GR_PPU_RG_ATT1_PX_Msk \
                                                       | PERI_GR_PPU_RG_ATT1_PC_MASK_0_Msk \
                                                       | PERI_GR_PPU_RG_ATT1_REGION_SIZE_Msk \
                                              ))

#if defined(LPDDR4_EMPU_EMPU_STRUCT_NR) && (LPDDR4_EMPU_EMPU_STRUCT_NR >= 0)
#define CY_PROT_EMPU_PC_LIMIT_MASK            (0xFFFFFFFFUL << LPDDR4_EMPU_STRUCT_PC_NR_MINUS1)
#define CY_PROT_EMPU_ATT0_MASK                ((uint32_t)~(LPDDR4_EMPU_EMPU_STRUCT_ATT0_PC_MASK_0_Msk))
#define CY_PROT_EMPU_ATT1_MASK                ((uint32_t)~(LPDDR4_EMPU_EMPU_STRUCT_ATT1_UR_Msk \
                                                         | LPDDR4_EMPU_EMPU_STRUCT_ATT1_UX_Msk \
                                                         | LPDDR4_EMPU_EMPU_STRUCT_ATT1_PR_Msk \
                                                         | LPDDR4_EMPU_EMPU_STRUCT_ATT1_PX_Msk \
                                                         | LPDDR4_EMPU_EMPU_STRUCT_ATT1_PC_MASK_0_Msk \
                                                         | LPDDR4_EMPU_EMPU_STRUCT_ATT1_REGION_SIZE_Msk \
                                              ))
#endif

/** \endcond */


/***************************************
*       Configuration Structures
***************************************/

/**
* \addtogroup group_prot_data_structures
* \{
*/

/** Configuration structure for MPU Struct initialization */
typedef struct 
{
    uint32_t*         address;          /**< Base address of the memory region */
    cy_en_prot_size_t regionSize;       /**< Size of the memory region */
    uint8_t           subregions;       /**< Mask of the 8 subregions to disable */
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
} cy_stc_mpu_cfg_t;

/** Configuration structure for SMPU struct initialization */
typedef struct 
{
    uint32_t*         address;          /**< Base address of the memory region (Only applicable to slave) */
    cy_en_prot_size_t regionSize;       /**< Size of the memory region (Only applicable to slave) */
    uint8_t           subregions;       /**< Mask of the 8 subregions to disable (Only applicable to slave) */
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
    bool              pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t          pcMask;           /**< Mask of allowed protection context(s) */
} cy_stc_smpu_cfg_t;

/** Configuration structure for Programmable (PROG) PPU (PPU_PR) struct initialization */
typedef struct 
{
    uint32_t*         address;          /**< Base address of the memory region (Only applicable to slave) */
    cy_en_prot_size_t regionSize;       /**< Size of the memory region (Only applicable to slave) */
} cy_stc_ppu_prog_region_cfg_t;

typedef struct 
{
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
} cy_stc_ppu_prog_attr_cfg_t;

/** Configuration structure for Fixed Group (GR) PPU (PPU_GR) struct initialization */
typedef struct 
{
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;	        /**< Non Secure = 0, Secure = 1 */
} cy_stc_ppu_gr_cfg_t;


#if defined(LPDDR4_EMPU_EMPU_STRUCT_NR) && (LPDDR4_EMPU_EMPU_STRUCT_NR >= 0)
/** Configuration structure for EMPU struct initialization */
typedef struct 
{
    uint32_t*         address;          /**< Base address of the memory region (Only applicable to slave) */
    cy_en_prot_size_t regionSize;       /**< Size of the memory region (Only applicable to slave) */
    uint8_t           subregions;       /**< Mask of the 8 subregions to disable (Only applicable to slave) */
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
    bool              pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t          pcMask;           /**< Mask of allowed protection context(s) */
} cy_stc_empu_cfg_t;

#endif

/** \} group_prot_data_structures */


/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_prot_functions
* \{
*/

/**
* \addtogroup group_prot_functions_busmaster
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigBusMaster(en_prot_master_t busMaster, bool privileged, bool secure, uint32_t pcMask);
cy_en_prot_status_t Cy_Prot_SetActivePC(en_prot_master_t busMaster, uint32_t pc);
uint32_t Cy_Prot_GetActivePC(en_prot_master_t busMaster);

/** \} group_prot_functions_busmaster */

/**
* \addtogroup group_prot_functions_mpu
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base, const cy_stc_mpu_cfg_t* config);
cy_en_prot_status_t Cy_Prot_EnableMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_DisableMpuStruct(volatile stc_PROT_MPU_MPU_STRUCT_t* base);

/** \} group_prot_functions_mpu */

/**
* \addtogroup group_prot_functions_smpu
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base, const cy_stc_smpu_cfg_t* config);
cy_en_prot_status_t Cy_Prot_ConfigSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base, const cy_stc_smpu_cfg_t* config);
cy_en_prot_status_t Cy_Prot_EnableSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_DisableSmpuMasterStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_EnableSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_DisableSmpuSlaveStruct(volatile stc_PROT_SMPU_SMPU_STRUCT_t* base);

/** \} group_prot_functions_smpu */

/**
* \addtogroup group_prot_functions_ppu_prog
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigPpuProgMasterStructAttr(volatile stc_PERI_MS_PPU_PR_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_prog_attr_cfg_t* config);
cy_en_prot_status_t Cy_Prot_ConfigPpuProgSlaveStructAttr(volatile stc_PERI_MS_PPU_PR_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_prog_attr_cfg_t* config);
cy_en_prot_status_t Cy_Prot_ConfigPpuProgSlaveStructReg(volatile stc_PERI_MS_PPU_PR_t* base, const cy_stc_ppu_prog_region_cfg_t* config);
cy_en_prot_status_t Cy_Prot_EnablePpuProgMasterStruct(volatile stc_PERI_MS_PPU_PR_t* base);
cy_en_prot_status_t Cy_Prot_DisablePpuProgMasterStruct(volatile stc_PERI_MS_PPU_PR_t* base);
cy_en_prot_status_t Cy_Prot_EnablePpuProgSlaveStruct(volatile stc_PERI_MS_PPU_PR_t* base);
cy_en_prot_status_t Cy_Prot_DisablePpuProgSlaveStruct(volatile stc_PERI_MS_PPU_PR_t* base);

/** \} group_prot_functions_ppu_prog */

/**
* \addtogroup group_prot_functions_ppu_fixed
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigPpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_gr_cfg_t* config);
cy_en_prot_status_t Cy_Prot_ConfigPpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base, cy_en_prot_pc_t setPC, const cy_stc_ppu_gr_cfg_t* config);
cy_en_prot_status_t Cy_Prot_EnablePpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base);
cy_en_prot_status_t Cy_Prot_DisablePpuFixedMasterStruct(volatile stc_PERI_MS_PPU_FX_t* base);
cy_en_prot_status_t Cy_Prot_EnablePpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base);
cy_en_prot_status_t Cy_Prot_DisablePpuFixedSlaveStruct(volatile stc_PERI_MS_PPU_FX_t* base);

/** \} group_prot_functions_ppu_fixed */


#if defined(LPDDR4_EMPU_EMPU_STRUCT_NR) && (LPDDR4_EMPU_EMPU_STRUCT_NR >= 0)

/**
* \addtogroup group_prot_functions_empu
* \{
*/

cy_en_prot_status_t Cy_Prot_ConfigEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base, const cy_stc_empu_cfg_t* config);
cy_en_prot_status_t Cy_Prot_ConfigEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base, const cy_stc_empu_cfg_t* config);
cy_en_prot_status_t Cy_Prot_EnableEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_DisableEmpuMasterStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_EnableEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base);
cy_en_prot_status_t Cy_Prot_DisableEmpuSlaveStruct(volatile stc_LPDDR4_EMPU_EMPU_STRUCT_t* base);

/** \} group_prot_functions_empu */

#endif

/** \} group_prot_functions */

/** \} group_prot */

#if defined(__cplusplus)
}
#endif

#endif /* CY_PROT_H */
