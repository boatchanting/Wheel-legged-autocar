/***************************************************************************//**
* \file cy_flash.h
* \version 1.0
*
* Provides the API declarations of the Flash driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_FLASH_H)
#define CY_FLASH_H

/**
* \defgroup group_flash Flash System Routine (Flash)
* \{
* Internal flash memory programming
*
* Flash memory in PSoC devices provides non-volatile storage for user firmware,
* user configuration data, and bulk data storage.
*
* Flash operations are implemented as system calls. System calls are executed
* out of SROM in the privileged mode of operation. Users have no access to read
* or modify the SROM code. The driver API requests the system call by acquiring
* the Inter-processor communication (IPC) and writing the SROM function opcode
* and parameters to its input registers. As a result, an NMI interrupt is invoked
* and the requested SROM API is executed. The operation status is returned to the
* driver context and a release interrupt is triggered.
*
* Writing to flash can take up to 20 milliseconds. During this time,
* the device should not be reset (including XRES pin, software  reset, and
* watchdog) or unexpected changes may be made to portions of the flash.
* Also, the low-voltage detect circuits should be configured to generate an
* interrupt instead of a reset.
*
* The Read while Write violation occurs when the flash Read operation is initiated
* in the same flash sector where the flash Write operation is performing. The
* violation leads to the exception generation. To avoid the Read while Write
* violation, the user has to carefully split the Read and Write operation from the
* same flash sector considering both cores in the multi-processor device.
* Use different flash sectors for code and data storage. The flash is divided
* into four equal sectors.
*
* \section group_flash_configuration Configuration Considerations
*
* Note that to be able to perform FLASH writes, the VCCD should be more than 0.99 V.
* For more information see the technical reference manual (TRM).
*
* \section group_flash_more_information More Information
*
* See the technical reference manual (TRM) for more information about the Flash architecture.
*
*
* \section group_flash_changelog Changelog
*
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.1</td>
*     <td>fix check sum function</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_flash_macro Macro
* \defgroup group_flash_functions Functions
* \defgroup group_flash_data_structure Data Structures
*/

#include <cy_device_headers.h>
#include <stddef.h>
#include <ipc/cy_ipc_drv.h>
#include <syslib/cy_syslib.h>
#include "srom/cy_srom.h"

/***************************************
* Device Adapter
***************************************/
#define CY_FLASH_IS_MCU_BC_ENTRY (defined (tviibe512k) || defined (tviibe1m) || defined (tviibe2m) || defined (tviibe4m) || defined (tviice4m))

/***************************************
* Macro definitions
***************************************/
/**
* \addtogroup group_flash_macro
* \{
*/

/** Driver major version */
#define CY_FLASH_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_FLASH_DRV_VERSION_MINOR       0


/**
* \defgroup group_flash_returns  Flash return values
* \{
* Specifies return values meaning
*/
#define CY_FLASH_ID                        (CY_PDL_DRV_ID(0x14u))  /**< FLASH PDL ID */

#define CY_FLASH_ID_INFO    (uint32_t)( CY_FLASH_ID | CY_PDL_STATUS_INFO )   /**< Return prefix for FLASH driver function status codes */
#define CY_FLASH_ID_WARNING (uint32_t)( CY_FLASH_ID | CY_PDL_STATUS_WARNING) /**< Return prefix for FLASH driver function warning return values */
#define CY_FLASH_ID_ERROR   (uint32_t)( CY_FLASH_ID | CY_PDL_STATUS_ERROR)   /**< Return prefix for FLASH driver function error return values */

typedef enum
{
    CY_FLASH_IN_BOUNDS,
    CY_FLASH_OUT_OF_BOUNDS,
} en_flash_bounds_t;

#define CY_WFLASH_LG_SBM_TOP  CY_WFLASH_LG_SBM_BASE
#define CY_WFLASH_LG_SBM_END  (CY_WFLASH_LG_SBM_BASE + CY_WFLASH_LG_SBM_SIZE)

#define CY_WFLASH_SM_SBM_TOP  CY_WFLASH_SM_SBM_BASE
#define CY_WFLASH_SM_SBM_END  (CY_WFLASH_SM_SBM_BASE + CY_WFLASH_SM_SBM_SIZE)

#define CY_WFLASH_LG_DBM0_TOP CY_WFLASH_LG_DBM0_BASE
#define CY_WFLASH_LG_DBM0_END (CY_WFLASH_LG_DBM0_BASE + CY_WFLASH_LG_DBM0_SIZE)

#define CY_WFLASH_SM_DBM0_TOP CY_WFLASH_SM_DBM0_BASE
#define CY_WFLASH_SM_DBM0_END (CY_WFLASH_SM_DBM0_BASE + CY_WFLASH_SM_DBM0_SIZE)

#define CY_WFLASH_LG_DBM1_TOP CY_WFLASH_LG_DBM1_BASE
#define CY_WFLASH_LG_DBM1_END (CY_WFLASH_LG_DBM1_BASE + CY_WFLASH_LG_DBM1_SIZE)

#define CY_WFLASH_SM_DBM1_TOP CY_WFLASH_SM_DBM1_BASE
#define CY_WFLASH_SM_DBM1_END (CY_WFLASH_SM_DBM1_BASE + CY_WFLASH_SM_DBM1_SIZE)

#define CY_FLASH_LG_SBM_TOP  CY_FLASH_LG_SBM_BASE
#define CY_FLASH_LG_SBM_END  (CY_FLASH_LG_SBM_BASE + CY_FLASH_LG_SBM_SIZE)

#define CY_FLASH_SM_SBM_TOP  CY_FLASH_SM_SBM_BASE
#define CY_FLASH_SM_SBM_END  (CY_FLASH_SM_SBM_BASE + CY_FLASH_SM_SBM_SIZE)

#define CY_FLASH_LG_DBM0_TOP CY_FLASH_LG_DBM0_BASE
#define CY_FLASH_LG_DBM0_END (CY_FLASH_LG_DBM0_BASE + CY_FLASH_LG_DBM0_SIZE)

#define CY_FLASH_SM_DBM0_TOP CY_FLASH_SM_DBM0_BASE
#define CY_FLASH_SM_DBM0_END (CY_FLASH_SM_DBM0_BASE + CY_FLASH_SM_DBM0_SIZE)

#define CY_FLASH_LG_DBM1_TOP CY_FLASH_LG_DBM1_BASE
#define CY_FLASH_LG_DBM1_END (CY_FLASH_LG_DBM1_BASE + CY_FLASH_LG_DBM1_SIZE)

#define CY_FLASH_SM_DBM1_TOP CY_FLASH_SM_DBM1_BASE
#define CY_FLASH_SM_DBM1_END (CY_FLASH_SM_DBM1_BASE + CY_FLASH_SM_DBM1_SIZE)

#if defined (tviibh16m)
#define CY_WFLASH1_LG_SBM_TOP  CY_WFLASH1_LG_SBM_BASE
#define CY_WFLASH1_LG_SBM_END  (CY_WFLASH1_LG_SBM_BASE + CY_WFLASH1_LG_SBM_SIZE)

#define CY_WFLASH1_SM_SBM_TOP  CY_WFLASH1_SM_SBM_BASE
#define CY_WFLASH1_SM_SBM_END  (CY_WFLASH1_SM_SBM_BASE + CY_WFLASH1_SM_SBM_SIZE)

#define CY_WFLASH1_LG_DBM0_TOP CY_WFLASH1_LG_DBM0_BASE
#define CY_WFLASH1_LG_DBM0_END (CY_WFLASH1_LG_DBM0_BASE + CY_WFLASH1_LG_DBM0_SIZE)

#define CY_WFLASH1_SM_DBM0_TOP CY_WFLASH1_SM_DBM0_BASE
#define CY_WFLASH1_SM_DBM0_END (CY_WFLASH1_SM_DBM0_BASE + CY_WFLASH1_SM_DBM0_SIZE)

#define CY_WFLASH1_LG_DBM1_TOP CY_WFLASH1_LG_DBM1_BASE
#define CY_WFLASH1_LG_DBM1_END (CY_WFLASH1_LG_DBM1_BASE + CY_WFLASH1_LG_DBM1_SIZE)

#define CY_WFLASH1_SM_DBM1_TOP CY_WFLASH1_SM_DBM1_BASE
#define CY_WFLASH1_SM_DBM1_END (CY_WFLASH1_SM_DBM1_BASE + CY_WFLASH1_SM_DBM1_SIZE)

#define CY_FLASH1_LG_SBM_TOP  CY_FLASH1_LG_SBM_BASE
#define CY_FLASH1_LG_SBM_END  (CY_FLASH1_LG_SBM_BASE + CY_FLASH1_LG_SBM_SIZE)

#define CY_FLASH1_SM_SBM_TOP  CY_FLASH1_SM_SBM_BASE
#define CY_FLASH1_SM_SBM_END  (CY_FLASH1_SM_SBM_BASE + CY_FLASH1_SM_SBM_SIZE)

#define CY_FLASH1_LG_DBM0_TOP CY_FLASH1_LG_DBM0_BASE
#define CY_FLASH1_LG_DBM0_END (CY_FLASH1_LG_DBM0_BASE + CY_FLASH1_LG_DBM0_SIZE)

#define CY_FLASH1_SM_DBM0_TOP CY_FLASH1_SM_DBM0_BASE
#define CY_FLASH1_SM_DBM0_END (CY_FLASH1_SM_DBM0_BASE + CY_FLASH1_SM_DBM0_SIZE)

#define CY_FLASH1_LG_DBM1_TOP CY_FLASH1_LG_DBM1_BASE
#define CY_FLASH1_LG_DBM1_END (CY_FLASH1_LG_DBM1_BASE + CY_FLASH1_LG_DBM1_SIZE)

#define CY_FLASH1_SM_DBM1_TOP CY_FLASH1_SM_DBM1_BASE
#define CY_FLASH1_SM_DBM1_END (CY_FLASH1_SM_DBM1_BASE + CY_FLASH1_SM_DBM1_SIZE)
#endif

#define CY_SFLASH_TOP  CY_SFLASH_BASE
#define CY_SFLASH_END  (CY_SFLASH_BASE + CY_SFLASH_SIZE)

#define CY_SFLASH1_TOP CY_SFLASH1_BASE
#define CY_SFLASH1_END (CY_SFLASH1_BASE + CY_SFLASH1_SIZE)

#define CY_WORK_LES_SIZE_IN_BYTE  (0x00000800ul)
#define CY_WORK_LES_SIZE_IN_WORD  (0x00000800ul / 4ul)
#define CY_WORK_SES_SIZE_IN_BYTE  (0x00000080ul)
#define CY_WORK_SES_SIZE_IN_WORD  (0x00000080ul / 4ul)

#define CY_CODE_LES_SIZE_IN_BYTE  (0x00008000ul)
#define CY_CODE_LES_SIZE_IN_WORD  (0x00008000ul / 4ul)
#define CY_CODE_SES_SIZE_IN_BYTE  (0x00002000ul)
#define CY_CODE_SES_SIZE_IN_WORD  (0x00002000ul / 4ul)

/** This enum has the return values of the Flash driver */
typedef enum 
{
    CY_FLASH_DRV_SUCCESS                  =   0x00ul,  /**< Success */
    CY_FLASH_DRV_INV_PROT                 =   ( CY_FLASH_ID_ERROR + 0x00ul),  /**< Invalid device protection state (SROM STATUS CODE: 0xF0000001) */
    CY_FLASH_DRV_INVALID_FM_PL            =   ( CY_FLASH_ID_ERROR + 0x01ul),  /**< Invalid flash page latch address (SROM STATUS CODE: 0xF0000003) */
    CY_FLASH_DRV_INVALID_FLASH_ADDR       =   ( CY_FLASH_ID_ERROR + 0x02ul),  /**< Invalid flash address (SROM STATUS CODE: 0xF0000004) */
    CY_FLASH_DRV_ROW_PROTECTED            =   ( CY_FLASH_ID_ERROR + 0x03ul),  /**< Row is write protected (SROM STATUS CODE: 0xF0000005) */
    CY_FLASH_DRV_IPC_BUSY                 =   ( CY_FLASH_ID_ERROR + 0x04ul),  /**< IPC structure is already locked by another process  (SROM STATUS CODE: 0xF0000006) */
    CY_FLASH_DRV_CHECKSUM_NON_ZERO        =   ( CY_FLASH_ID_ERROR + 0x05ul),  /**< Checksum of FLASH resulted in non-zero (SROM STATUS CODE: 0xF000000A) */
    CY_FLASH_DRV_SECTOR_SUSPEND           =   ( CY_FLASH_ID_ERROR + 0x06ul),  /**< Returned when Program operation is called on sector which is suspended from erase (SROM STATUS CODE: 0xF0000091) */
    CY_FLASH_DRV_NO_ERASE_SUSPEND         =   ( CY_FLASH_ID_ERROR + 0x07ul),  /**< Returned when EraseResume is called when no sector is suspended from erase (SROM STATUS CODE: 0xF0000092) */
    CY_FLASH_DRV_FLASH_NOT_ERASED         =   ( CY_FLASH_ID_ERROR + 0x08ul),  /**< Returned when ProgramRow is invoked on unerased cells or blank check fails (SROM STATUS CODE: 0xF00000A4) */
    CY_FLASH_DRV_NO_ERASE_ONGOING         =   ( CY_FLASH_ID_ERROR + 0x09ul),  /**< Returned by EraseSuspend when called with no ongoing erase operation (SROM STATUS CODE: 0xF00000A5) */
    CY_FLASH_DRV_ACTIVE_ERASE             =   ( CY_FLASH_ID_ERROR + 0x0Aul),  /**< Returned by ProgramRow when active erase operation is going on (SROM STATUS CODE: 0xF00000A6) */
    CY_FLASH_DRV_INVALID_DATA_WIDTH       =   ( CY_FLASH_ID_ERROR + 0x0Bul),  /**< Returned by ProgramRow API if invalid program width option is provided (SROM STATUS CODE: 0xF00000A8) */
    CY_FLASH_DRV_FLASH_SAFTEY_ENABLED     =   ( CY_FLASH_ID_ERROR + 0x0Cul),  /**< Returned by FLASH program/erase APIs when writes are disabled in safety register (SROM STATUS CODE: 0xF00000AA) */
    CY_FLASH_DRV_INVALID_SFLASH_ADDR      =   ( CY_FLASH_ID_ERROR + 0x0Dul),  /**< Returned when WriteRow is called on invalid SFLASH rows in NORMAL state (SROM STATUS CODE: 0xF00000B2) */
    CY_FLASH_DRV_SFLASH_BACKUP_ERASED     =   ( CY_FLASH_ID_ERROR + 0x0Eul),  /**< Returned by Sflash programming APIs when backup sector is in erased state (SROM STATUS CODE: 0xF00000BB) */
    CY_FLASH_DRV_INVALID_INPUT_PARAMETERS =   ( CY_FLASH_ID_ERROR + 0x0Ful),  /**< Input parameters passed to Flash API are not valid */
    CY_FLASH_DRV_SROM_API_TIMEOUT         =   ( CY_FLASH_ID_ERROR + 0x10ul),  /**< Time out happens after calling srom API driver */
    CY_FLASH_DRV_ERR_UNC                  =   ( CY_FLASH_ID_ERROR + 0xFFul),/**< Unknown error */
    CY_FLASH_DRV_PROGRESS_NO_ERROR        =   ( CY_FLASH_ID_INFO  + 0ul),  /**< Command in progress; no error */
    CY_FLASH_DRV_OPERATION_STARTED        =   ( CY_FLASH_ID_INFO  + 1ul),  /**< Flash operation is successfully initiated */
    CY_FLASH_DRV_OPCODE_BUSY              =   ( CY_FLASH_ID_INFO  + 2ul)  /**< Flash is under operation */
} cy_en_flashdrv_status_t;

/** \} group_flash_returns */

/** \cond INTERNAL */

#define CY_FLASH_CODE_ECC_INJECT_MASK         (0x00FFFFFFul) /** CODE ECC injection mask */
#define CY_FLASH_WORK_ECC_INJECT_MASK         (0x007FFFFFul) /** WORK ECC injection mask */
#define CY_FLASH_CACHE_ECC_INJECT_MASK        (0x00FFFFFFul) /** CACHE ECC injection mask */
#define CY_FLASH_CODE_ECC_INJECT_POS          (0x03ul)       /** CODE ECC injection position shift */
#define CY_FLASH_WORK_ECC_INJECT_POS          (0x02ul)       /** WORK ECC injection position shift */
#define CY_FLASH_CACHE_ECC_INJECT_POS         (0x02ul)       /** CACHE ECC injection position shift */
#define CY_FLASH_CODE_ECC_INJECT_PARITY_MASK  (0xFFul)       /** CODE ECC injection parity mask */
#define CY_FLASH_WORK_ECC_INJECT_PARITY_MASK  (0x7Ful)       /** WORK ECC injection parity mask */
#define CY_FLASH_CACHE_ECC_INJECT_PARITY_MASK (0x7Ful)       /** CACHE ECC injection parity mask */

typedef enum
{
    CY_FLASH_PROGRAMROW_DATA_SIZE_8BIT    = (0x00ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_16BIT   = (0x01ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_32BIT   = (0x02ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_64BIT   = (0x03ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_128BIT  = (0x04ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_256BIT  = (0x05ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_512BIT  = (0x06ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_1024BIT = (0x07ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_2048BIT = (0x08ul),
    CY_FLASH_PROGRAMROW_DATA_SIZE_4096BIT = (0x09ul)
} cy_en_flash_programrow_datasize_t;

typedef enum
{
    CY_FLASH_PROGRAMROW_NON_BLOCKING = (0x00ul),
    CY_FLASH_PROGRAMROW_BLOCKING     = (0x01ul)
} cy_en_flash_programrow_blocking_t;

typedef enum
{
    CY_FLASH_PROGRAMROW_BLANK_CHECK      =  (0x00ul),
    CY_FLASH_PROGRAMROW_SKIP_BLANK_CHECK =  (0x01ul)
} cy_en_flash_programrow_skipblankcheck_t;

typedef enum
{
    CY_FLASH_PROGRAMROW_DATA_LOCATION_SRAM       = (0x01ul)
} cy_en_flash_programrow_location_t;

typedef enum
{
    CY_FLASH_PROGRAMROW_NOT_SET_INTR_MASK = (0x00ul),
    CY_FLASH_PROGRAMROW_SET_INTR_MASK     = (0x01ul)
} cy_en_flash_programrow_intrmask_t;

typedef enum
{
    CY_FLASH_ERASESECTOR_NON_BLOCKING      = (0x00ul),
    CY_FLASH_ERASESECTOR_BLOCKING          = (0x01ul)
} cy_en_flash_erasesector_blocking_t;

typedef enum
{
    CY_FLASH_ERASESECTOR_NOT_SET_INTR_MASK = (0x00ul),
    CY_FLASH_ERASESECTOR_SET_INTR_MASK     = (0x01ul)
} cy_en_flash_erasesector_intrmask_t;

typedef enum
{
    CY_FLASH_CHECKSUM_BANK0     = (0x00ul),
    CY_FLASH_CHECKSUM_BANK1     = (0x01ul)
} cy_en_flash_checksum_bank_t;

typedef enum
{
    CY_FLASH_CHECKSUM_PAGE     = (0x00ul),
    CY_FLASH_CHECKSUM_WHOLE    = (0x01ul)
} cy_en_flash_checksum_scope_t;

typedef enum
{
    CY_FLASH_CHECKSUM_MAIN        = (0x00ul),
    CY_FLASH_CHECKSUM_WOEK        = (0x01ul),
    CY_FLASH_CHECKSUM_SUPERVISORY = (0x02ul)
} cy_en_flash_checksum_region_t;

typedef enum
{
    CY_FLASH_COMPUTEHASH_BASIC  = (0x00ul),
    CY_FLASH_COMPUTEHASH_CRC8   = (0x01ul)
} cy_en_flash_computehash_type_t;

typedef enum
{
    CY_FLASH_ERASERESUME_NOT_SET_INTR_MASK  = (0x00ul),
    CY_FLASH_ERASERESUME_SET_INTR_MASK      = (0x01ul)
} cy_en_flash_eraseresume_setintr_t;

typedef enum
{
    CY_FLASH_ERASERESUME_NON_BLOCKING  = (0x00ul),
    CY_FLASH_ERASERESUME_BLOCKING      = (0x01ul)
} cy_en_flash_eraseresume_blocking_t;

typedef enum
{
    CY_FLASH_DRIVER_NON_BLOCKING  = (0x00ul),
    CY_FLASH_DRIVER_BLOCKING      = (0x01ul)
} cy_en_flash_driver_blocking_t;


/** \endcond */


/**
* \addtogroup group_flash_general_macro Flash general parameters
* \{
* Provides general information about flash and IPC
*/
#if !defined (tviibh16m)
    #define CY_FLASH_SIZEOF_ROW  (CPUSS_FLASHC_PA_SIZE*4ul)             /** Flash row size */
    #define CY_FLASH_NUMBER_ROWS (CPUSS_FLASH_SIZE/CY_FLASH_SIZEOF_ROW) /** Number of flash rows */
#elif defined (tviibh16m)
    #define CY_FLASH_SIZEOF_ROW  (CPUSS_FLASHCX_PA_SIZE*4ul)             /** Flash row size */
    #define CY_FLASH_NUMBER_ROWS (CPUSS_FLASH_SIZE/CY_FLASH_SIZEOF_ROW) /** Number of flash rows */
#endif

/** \} group_flash_general_macro */

typedef enum
{
    CY_FLASH_MAIN_REGION = 0u,
    CY_FLASH_WORK_REGION,
#if CY_FLASH_IS_MCU_BC_ENTRY
    CY_FLASH_CA_CM4_REGION,
#endif
    CY_FLASH_CA_CM0P_REGION
} cy_en_region_t;

typedef enum
{
    CY_FLASH_SINGLE_BANK_MODE = 0u,
    CY_FLASH_DUAL_BANK_MODE = 1u
} cy_en_bankmode_t;

typedef enum
{
    CY_FLASH_MAPPING_A = 0u,
    CY_FLASH_MAPPING_B = 1u
} cy_en_maptype_t;

/** \} group_flash_macro */

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
* Data Structure definitions
***************************************/

/**
* \addtogroup group_flash_srom_config_structure
* \{
*/

typedef struct
{
    const uint32_t*                         destAddr; /**< Specifies the configuration of flash operation */
    const uint32_t*                         dataAddr; /**< Specifies the configuration of flash operation */
    cy_en_flash_programrow_blocking_t       blocking; /**< Specifies the code of flash operation */
    cy_en_flash_programrow_skipblankcheck_t skipBC;   /**< Specifies the code of flash operation */
    cy_en_flash_programrow_datasize_t       dataSize; /**< Specifies the configuration of flash operation */
    cy_en_flash_programrow_location_t       dataLoc;  /**< Specifies the configuration of flash operation */
    cy_en_flash_programrow_intrmask_t       intrMask; /**< Specifies the configuration of flash operation */
}cy_stc_flash_programrow_config_t;

typedef struct
{
    const uint32_t*                    Addr;     /**< Specifies the configuration of flash operation */
    cy_en_flash_erasesector_blocking_t blocking; /**< Specifies the code of flash operation */
    cy_en_flash_erasesector_intrmask_t intrMask; /**< Specifies the configuration of flash operation */
}cy_stc_flash_erasesector_config_t;

typedef struct
{
    uint8_t                       rowId;  /**< Specifies the configuration of flash operation */
    cy_en_flash_checksum_bank_t   bank;   /**< Specifies the code of flash operation */
    cy_en_flash_checksum_scope_t  whole;  /**< Specifies the configuration of flash operation */
    cy_en_flash_checksum_region_t region; /**< Specifies the configuration of flash operation */
} cy_stc_flash_checksum_config_t;

typedef struct
{
    const uint32_t*                startAddr; /**< Specifies the configuration of flash operation */
    uint32_t                       numOfByte; /**< Specifies the configuration of flash operation */
    cy_en_flash_computehash_type_t type;      /**< Specifies the code of flash operation */
} cy_stc_flash_computehash_config_t;

typedef struct
{
    cy_en_flash_eraseresume_setintr_t  intrMask; /**< Specifies the code of flash operation */
    cy_en_flash_eraseresume_blocking_t blocking; /**< Specifies the code of flash operation */
} cy_stc_flash_eraseresume_config_t;

typedef struct
{
    const uint32_t* addrToBeChecked;
    uint32_t        numOfWordsToBeChecked;
} cy_stc_flash_blankcheck_config_t;

/** Flash driver context (no longer required) */
typedef union
{
    uint32_t arg[4];
} cy_un_flash_context_t;

/** \} group_flash_srom_config_structure */

/*******************************************************************************
* Function Name: Cy_Flashc_SetMainWaitState
****************************************************************************//**
*
* \brief Sets main wait state
*
* \param waitState wait state to be set.
*
* \return flash status
*
*******************************************************************************/
__STATIC_INLINE cy_en_flashdrv_status_t Cy_Flashc_SetMainWaitState(uint8_t waitState)
{
    if(waitState <= 15ul)
    {
#if CY_FLASH_IS_MCU_BC_ENTRY
        FLASHC->unFLASH_CTL.stcField.u4MAIN_WS = waitState;
#else
        FLASHC->unFLASH_CTL.stcField.u4WS = waitState;
#endif
        return CY_FLASH_DRV_SUCCESS;
    }
    else
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CacheEnable
****************************************************************************//**
*
* \brief Enables CM0 cache.
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CacheEnable(void)
{
    FLASHC->unCM0_CA_CTL0.stcField.u1CA_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CacheDisable
****************************************************************************//**
*
* \brief Disables CM0 cache
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CacheDisable(void)
{
    FLASHC->unCM0_CA_CTL0.stcField.u1CA_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_PrefetchEnable
****************************************************************************//**
*
* \brief Enables CM0 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_PrefetchEnable(void)
{
    FLASHC->unCM0_CA_CTL0.stcField.u1PREF_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_PrefetchDisable
****************************************************************************//**
*
* \brief Disables CM0 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_PrefetchDisable(void)
{
    FLASHC->unCM0_CA_CTL0.stcField.u1PREF_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCEnabled
****************************************************************************//**
*
* \brief Enables ECC for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCEnabled(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCDisable
****************************************************************************//**
*
* \brief Disables ECC for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCDisable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MainECCEnable
****************************************************************************//**
*
* \brief Enables ECC for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCEnable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MainECCDisable
****************************************************************************//**
*
* \brief Disables ECC for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCDisable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCEnable
****************************************************************************//**
*
* \brief Enables ECC for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCEnable(void)
{
   FLASHC->unCM0_CA_CTL0.stcField.u1RAM_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCDisable
****************************************************************************//**
*
* \brief Disables ECC for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCDisable(void)
{
   FLASHC->unCM0_CA_CTL0.stcField.u1RAM_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCInjectionEnable
****************************************************************************//**
*
* \brief Enables ECC injection for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCInjectionEnable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCInjectionDisable
****************************************************************************//**
*
* \brief Disables ECC injection for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCInjectionDisable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_ECC_INJ_EN = 0ul;
}


/*******************************************************************************
* Function Name: Cy_Flashc_MainECCInjectionEnable
****************************************************************************//**
*
* \brief Enables ECC injection for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCInjectionEnable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MAINECCInjectionDisable
****************************************************************************//**
*
* \brief Disables ECC injection for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MAINECCInjectionDisable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_ECC_INJ_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCInjectionEnable
****************************************************************************//**
*
* \brief Enables ECC injection for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCInjectionEnable(void)
{
   FLASHC->unCM0_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCInjectionDisable
****************************************************************************//**
*
* \brief Disables ECC injection for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCInjectionDisable(void)
{
   FLASHC->unCM0_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetWorkBankMode
****************************************************************************//**
*
* \brief Sets bank mode for work flash
*
* \param mode bank mode to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetWorkBankMode(cy_en_bankmode_t mode)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_BANK_MODE = mode;
   FLASHC->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetWorkBankMode
****************************************************************************//**
*
* \brief Gets current bank mode for work flash
*
* \return Current bank mode
*
*******************************************************************************/
__STATIC_INLINE cy_en_bankmode_t Cy_Flashc_GetWorkBankMode(void)
{
   return (cy_en_bankmode_t)FLASHC->unFLASH_CTL.stcField.u1WORK_BANK_MODE;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetMainBankMode
****************************************************************************//**
*
* \brief Sets bank mode for main flash
*
* \param mode bank mode to be set
*
* \return
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetMainBankMode(cy_en_bankmode_t mode)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_BANK_MODE = mode;
   FLASHC->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetMainBankMode
****************************************************************************//**
*
* \brief Gets current bank mode for main flash
*
* \return Current bank mode
*
*******************************************************************************/
__STATIC_INLINE cy_en_bankmode_t Cy_Flashc_GetMainBankMode(void)
{
   return (cy_en_bankmode_t)FLASHC->unFLASH_CTL.stcField.u1MAIN_BANK_MODE;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetWorkMap
****************************************************************************//**
*
* \brief Sets bank map for work flash
*
* \param type Bank map type to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetWorkMap(cy_en_maptype_t type)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_MAP = type;
   FLASHC->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetWorkMap
****************************************************************************//**
*
* \brief Gets bank map for work flash
*
* \return type Bank map type
*
*******************************************************************************/
__STATIC_INLINE cy_en_maptype_t Cy_Flashc_GetWorkMap(void)
{
   return (cy_en_maptype_t)FLASHC->unFLASH_CTL.stcField.u1WORK_MAP;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetMainMap
****************************************************************************//**
*
* \brief Sets bank map for main flash
*
* \param type Bank map type to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetMainMap(cy_en_maptype_t type)
{
   FLASHC->unFLASH_CTL.stcField.u1MAIN_MAP = type;
   FLASHC->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetMainMap
****************************************************************************//**
*
* \brief Gets bank map for main flash
*
* \return type Bank map type
*
*******************************************************************************/
__STATIC_INLINE cy_en_maptype_t Cy_Flashc_GetMainMap(void)
{
   return (cy_en_maptype_t)FLASHC->unFLASH_CTL.stcField.u1MAIN_MAP;
}

/*******************************************************************************
* Function Name: Cy_Flashc_InvalidateFlashCacheBuffer
****************************************************************************//**
*
* \brief Invalidates the flash cache and buffer
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_InvalidateFlashCacheBuffer(void)
{
    FLASHC->unFLASH_CMD.stcField.u1INV = 0x1ul;

    // Wait for completion (HW will clear bit)
    while(FLASHC->unFLASH_CMD.stcField.u1INV != 0);
}

/*******************************************************************************
* Function Name: Cy_Flashc_InvalidateFlashBuffer
****************************************************************************//**
*
* \brief Invalidates the flash buffer
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_InvalidateFlashBuffer(void)
{
    FLASHC->unFLASH_CMD.stcField.u1BUFF_INV = 0x1ul;

    // Wait for completion (HW will clear bit)
    while(FLASHC->unFLASH_CMD.stcField.u1BUFF_INV != 0);
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_MainWriteEnable
****************************************************************************//**
*
* \brief Enable writing main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainWriteEnable(void)
{
    FLASHC_FM_CTL_ECT->unMAIN_FLASH_SAFETY.stcField.u1MAINFLASHWRITEENABLE = 1ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_WorkWriteEnable
****************************************************************************//**
*
* \brief Enable writing work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkWriteEnable(void)
{
    FLASHC_FM_CTL_ECT->unWORK_FLASH_SAFETY.stcField.u1WORKFLASHWRITEENABLE = 1ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_MainWriteDisable
****************************************************************************//**
*
* \brief Disable writing main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainWriteDisable(void)
{
    FLASHC_FM_CTL_ECT->unMAIN_FLASH_SAFETY.stcField.u1MAINFLASHWRITEENABLE = 0ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_WorkWriteDisable
****************************************************************************//**
*
* \brief Disable writing work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkWriteDisable(void)
{
    FLASHC_FM_CTL_ECT->unWORK_FLASH_SAFETY.stcField.u1WORKFLASHWRITEENABLE = 0ul;
}


#if CY_FLASH_IS_MCU_BC_ENTRY
/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CacheEnable
****************************************************************************//**
*
* \brief Enables CM4 cache
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CacheEnable(void)
{
    FLASHC->unCM4_CA_CTL0.stcField.u1CA_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CacheDisable
****************************************************************************//**
*
* \brief Disables CM4 cache
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CacheDisable(void)
{
    FLASHC->unCM4_CA_CTL0.stcField.u1CA_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_PrefetchEnable
****************************************************************************//**
*
* \brief Enables CM4 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_PrefetchEnable(void)
{
    FLASHC->unCM4_CA_CTL0.stcField.u1PREF_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_PrefetchDisable
****************************************************************************//**
*
* \brief Disables CM4 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_PrefetchDisable(void)
{
    FLASHC->unCM4_CA_CTL0.stcField.u1PREF_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CAECCEnable
****************************************************************************//**
*
* \brief Enables ECC for cache in CM4
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CAECCEnable(void)
{
   FLASHC->unCM4_CA_CTL0.stcField.u1RAM_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CAECCDisable
****************************************************************************//**
*
* \brief Disables ECC for cache in CM4
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CAECCDisable(void)
{
 
   FLASHC->unCM4_CA_CTL0.stcField.u1RAM_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CAECCInjectionEnable
****************************************************************************//**
*
* \brief Enables ECC injection for cache in CM4
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CAECCInjectionEnable(void)
{
   FLASHC->unCM4_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM4_CAECCInjectionDisable
****************************************************************************//**
*
* \brief Disables ECC injection for cache in CM4
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM4_CAECCInjectionDisable(void)
{
 
   FLASHC->unCM4_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 0ul;
}
#else
/*******************************************************************************
* Function Name: Cy_Flashc_WorkSeqReadEnable
****************************************************************************//**
*
* \brief Enables sequential read operation from work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkSeqReadEnable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_SEQ_RD_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkSeqReadDisable
****************************************************************************//**
*
* \brief Disables sequential read operation from work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkSeqReadDisable(void)
{
   FLASHC->unFLASH_CTL.stcField.u1WORK_SEQ_RD_EN = 0ul;
}
#endif

#if defined (tviibh16m)
/*******************************************************************************
* Function Name: Cy_Flashc_SetMainWaitState1
****************************************************************************//**
*
* \brief Sets main wait state
*
* \param waitState wait state to be set.
*
* \return flash status
*
*******************************************************************************/
__STATIC_INLINE cy_en_flashdrv_status_t Cy_Flashc_SetMainWaitState1(uint8_t waitState)
{
    if(waitState <= 15ul)
    {
        FLASHC1->unFLASH_CTL.stcField.u4WS = waitState;

        return CY_FLASH_DRV_SUCCESS;
    }
    else
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CacheEnable1
****************************************************************************//**
*
* \brief Enables CM0 cache.
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CacheEnable1(void)
{
    FLASHC1->unCM0_CA_CTL0.stcField.u1CA_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CacheDisable1
****************************************************************************//**
*
* \brief Disables CM0 cache
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CacheDisable1(void)
{
    FLASHC1->unCM0_CA_CTL0.stcField.u1CA_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_PrefetchEnable1
****************************************************************************//**
*
* \brief Enables CM0 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_PrefetchEnable1(void)
{
    FLASHC1->unCM0_CA_CTL0.stcField.u1PREF_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_PrefetchDisable1
****************************************************************************//**
*
* \brief Disables CM0 prefetch
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_PrefetchDisable1(void)
{
    FLASHC1->unCM0_CA_CTL0.stcField.u1PREF_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCEnable1
****************************************************************************//**
*
* \brief Enables ECC for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCEnable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCDisable1
****************************************************************************//**
*
* \brief Disables ECC for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCDisable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MainECCEnable1
****************************************************************************//**
*
* \brief Enables ECC for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCEnable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MainECCDisable1
****************************************************************************//**
*
* \brief Disables ECC for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCDisable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCEnable1
****************************************************************************//**
*
* \brief Enables ECC for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCEnable1(void)
{
   FLASHC1->unCM0_CA_CTL0.stcField.u1RAM_ECC_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCDisable1
****************************************************************************//**
*
* \brief Disables ECC for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCDisable1(void)
{
   FLASHC1->unCM0_CA_CTL0.stcField.u1RAM_ECC_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCInjectionEnable1
****************************************************************************//**
*
* \brief Enables ECC injection for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCInjectionEnable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkECCInjectionDisable1
****************************************************************************//**
*
* \brief Disables ECC injection for work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkECCInjectionDisable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_ECC_INJ_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MainECCInjectionEnable1
****************************************************************************//**
*
* \brief Enables ECC injection for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainECCInjectionEnable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_MAINECCInjectionDisable1
****************************************************************************//**
*
* \brief Disables ECC injection for main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MAINECCInjectionDisable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_ECC_INJ_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCInjectionEnable1
****************************************************************************//**
*
* \brief Enables ECC injection for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCInjectionEnable1(void)
{
   FLASHC1->unCM0_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_CM0_CAECCInjectionDisable1
****************************************************************************//**
*
* \brief Disables ECC injection for cache in CM0
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_CM0_CAECCInjectionDisable1(void)
{
   FLASHC1->unCM0_CA_CTL0.stcField.u1RAM_ECC_INJ_EN = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetWorkBankMode1
****************************************************************************//**
*
* \brief Sets bank mode for work flash
*
* \param mode bank mode to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetWorkBankMode1(cy_en_bankmode_t mode)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_BANK_MODE = mode;
   FLASHC1->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetWorkBankMode1
****************************************************************************//**
*
* \brief Gets current bank mode for work flash
*
* \return Current bank mode
*
*******************************************************************************/
__STATIC_INLINE cy_en_bankmode_t Cy_Flashc_GetWorkBankMode1(void)
{
   return (cy_en_bankmode_t)FLASHC1->unFLASH_CTL.stcField.u1WORK_BANK_MODE;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetMainBankMode1
****************************************************************************//**
*
* \brief Sets bank mode for main flash
*
* \param mode bank mode to be set
*
* \return
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetMainBankMode1(cy_en_bankmode_t mode)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_BANK_MODE = mode;
   FLASHC1->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetMainBankMode1
****************************************************************************//**
*
* \brief Gets current bank mode for main flash
*
* \return Current bank mode
*
*******************************************************************************/
__STATIC_INLINE cy_en_bankmode_t Cy_Flashc_GetMainBankMode1(void)
{
   return (cy_en_bankmode_t)FLASHC1->unFLASH_CTL.stcField.u1MAIN_BANK_MODE;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetWorkMap1
****************************************************************************//**
*
* \brief Sets bank map for work flash
*
* \param type Bank map type to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetWorkMap1(cy_en_maptype_t type)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_MAP = type;
   FLASHC1->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetWorkMap1
****************************************************************************//**
*
* \brief Gets bank map for work flash
*
* \return type Bank map type
*
*******************************************************************************/
__STATIC_INLINE cy_en_maptype_t Cy_Flashc_GetWorkMap1(void)
{
   return (cy_en_maptype_t)FLASHC1->unFLASH_CTL.stcField.u1WORK_MAP;
}

/*******************************************************************************
* Function Name: Cy_Flashc_SetMainMap1
****************************************************************************//**
*
* \brief Sets bank map for main flash
*
* \param type Bank map type to be set
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_SetMainMap1(cy_en_maptype_t type)
{
   FLASHC1->unFLASH_CTL.stcField.u1MAIN_MAP = type;
   FLASHC1->unFLASH_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_Flashc_GetMainMap1
****************************************************************************//**
*
* \brief Gets bank map for main flash
*
* \return type Bank map type
*
*******************************************************************************/
__STATIC_INLINE cy_en_maptype_t Cy_Flashc_GetMainMap1(void)
{
   return (cy_en_maptype_t)FLASHC1->unFLASH_CTL.stcField.u1MAIN_MAP;
}

/*******************************************************************************
* Function Name: Cy_Flashc_InvalidateFlashCacheBuffer1
****************************************************************************//**
*
* \brief Invalidates the flash cache and buffer
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_InvalidateFlashCacheBuffer1(void)
{
    FLASHC1->unFLASH_CMD.stcField.u1INV = 0x1ul;

    // Wait for completion (HW will clear bit)
    while(FLASHC1->unFLASH_CMD.stcField.u1INV != 0);
}

/*******************************************************************************
* Function Name: Cy_Flashc_InvalidateFlashBuffer1
****************************************************************************//**
*
* \brief Invalidates the flash buffer
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_InvalidateFlashBuffer1(void)
{
    FLASHC1->unFLASH_CMD.stcField.u1BUFF_INV = 0x1ul;

    // Wait for completion (HW will clear bit)
    while(FLASHC1->unFLASH_CMD.stcField.u1BUFF_INV != 0);
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_MainWriteEnable1
****************************************************************************//**
*
* \brief Enable writing main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainWriteEnable1(void)
{
    FLASHC1_FM_CTL_ECT->unMAIN_FLASH_SAFETY.stcField.u1MAINFLASHWRITEENABLE = 1ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_WorkWriteEnable1
****************************************************************************//**
*
* \brief Enable writing work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkWriteEnable1(void)
{
    FLASHC1_FM_CTL_ECT->unWORK_FLASH_SAFETY.stcField.u1WORKFLASHWRITEENABLE = 1ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_MainWriteDisable
****************************************************************************//**
*
* \brief Disable writing main flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_MainWriteDisable1(void)
{
    FLASHC1_FM_CTL_ECT->unMAIN_FLASH_SAFETY.stcField.u1MAINFLASHWRITEENABLE = 0ul;
}

// Needs to be removed once boot supports enabling by default
/*******************************************************************************
* Function Name: Cy_Flashc_WorkWriteDisable1
****************************************************************************//**
*
* \brief Disable writing work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkWriteDisable1(void)
{
    FLASHC1_FM_CTL_ECT->unWORK_FLASH_SAFETY.stcField.u1WORKFLASHWRITEENABLE = 0ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkSeqReadEnable1
****************************************************************************//**
*
* \brief Enables sequential read operation from work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkSeqReadEnable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_SEQ_RD_EN = 1ul;
}

/*******************************************************************************
* Function Name: Cy_Flashc_WorkSeqReadDisable1
****************************************************************************//**
*
* \brief Disables sequential read operation from work flash
*
* \return none
*
*******************************************************************************/
__STATIC_INLINE void Cy_Flashc_WorkSeqReadDisable1(void)
{
   FLASHC1->unFLASH_CTL.stcField.u1WORK_SEQ_RD_EN = 0ul;
}
#endif

/***************************************
* Function Prototypes
***************************************/

/**
* \addtogroup group_flash_functions
* \{
*/
cy_en_flashdrv_status_t Cy_Flash_ProgramRow(cy_un_flash_context_t* context, const cy_stc_flash_programrow_config_t* config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_Checksum (cy_un_flash_context_t* context, const cy_stc_flash_checksum_config_t *config, uint32_t* cheksumPtr);
cy_en_flashdrv_status_t Cy_Flash_CalculateHash(cy_un_flash_context_t* context, const cy_stc_flash_computehash_config_t *config,  uint32_t* hashPtr);
cy_en_flashdrv_status_t Cy_Flash_EraseAll(cy_un_flash_context_t* context, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_EraseSector(cy_un_flash_context_t* context, const cy_stc_flash_erasesector_config_t *config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_ConfigureFMIntr(cy_un_flash_context_t* context, cy_en_conf_fm_intr_option_t option);
cy_en_flashdrv_status_t Cy_Flash_EraseSuspend(cy_un_flash_context_t* context);
cy_en_flashdrv_status_t Cy_Flash_EraseResume(cy_un_flash_context_t* context ,const cy_stc_flash_eraseresume_config_t *config);
cy_en_flashdrv_status_t Cy_Flash_BlankCheck(cy_un_flash_context_t* context ,const cy_stc_flash_blankcheck_config_t *config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_GetDrvStatus(cy_un_flash_context_t *context);
cy_en_flashdrv_status_t Cy_Flashc_InjectECC(cy_en_region_t region, uint32_t address, uint8_t parity);

#if defined (tviibh16m)
/**
* \addtogroup group_flash_functions
* \{
*/
cy_en_flashdrv_status_t Cy_Flash_ProgramRow1(cy_un_flash_context_t* context, const cy_stc_flash_programrow_config_t* config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_Checksum1(cy_un_flash_context_t* context, const cy_stc_flash_checksum_config_t *config, uint32_t* cheksumPtr);
cy_en_flashdrv_status_t Cy_Flash_EraseAll1(cy_un_flash_context_t* context, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_EraseSector1(cy_un_flash_context_t* context, const cy_stc_flash_erasesector_config_t *config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flash_ConfigureFMIntr1(cy_un_flash_context_t* context, cy_en_conf_fm_intr_option_t option);
cy_en_flashdrv_status_t Cy_Flash_EraseSuspend1(cy_un_flash_context_t* context);
cy_en_flashdrv_status_t Cy_Flash_EraseResume1(cy_un_flash_context_t* context ,const cy_stc_flash_eraseresume_config_t *config);
cy_en_flashdrv_status_t Cy_Flash_BlankCheck1(cy_un_flash_context_t* context ,const cy_stc_flash_blankcheck_config_t *config, cy_en_flash_driver_blocking_t block);
cy_en_flashdrv_status_t Cy_Flashc_InjectECC1(cy_en_region_t region, uint32_t address, uint8_t parity);
#endif

/** \cond INTERNAL */
en_flash_bounds_t Cy_Flash_WorkBoundsCheck(uint32_t address);
bool Cy_Flash_IsWorkSmallSector(uint32_t address);
en_flash_bounds_t Cy_Flash_MainBoundsCheck(uint32_t address);
bool Cy_Flash_IsMainSmallSector(uint32_t address);
en_flash_bounds_t Cy_Flash_SupervisoryBoundsCheck(uint32_t address);
en_flash_bounds_t Cy_Flash_BoundsCheck(uint32_t flashAddr);
cy_en_flashdrv_status_t Cy_Flash_ProcessOpcode(uint32_t opcode);

#if defined (tviibh16m)
en_flash_bounds_t Cy_Flash_WorkBoundsCheck1(uint32_t address);
bool Cy_Flash_IsWorkSmallSector1(uint32_t address);
en_flash_bounds_t Cy_Flash_MainBoundsCheck1(uint32_t address);
bool Cy_Flash_IsMainSmallSector1(uint32_t address);
en_flash_bounds_t Cy_Flash_BoundsCheck1(uint32_t flashAddr);
#endif

/** \endcond */

/** \} group_flash_functions */

#if defined(__cplusplus)
}
#endif


#endif /* #if !defined(CY_FLASH_H) */
/** \endcond */


/** \} group_flash */


/* [] END OF FILE */
