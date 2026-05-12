/***************************************************************************//**
* \file cy_flash.c
* \version 2.0
*
* \brief
* Provides the public functions for the API for the PSoC 6 Flash Driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_flash.h"

/** Look-up Table to map cy_en_flash_programrow_datasize_t to actual programming size in bytes, used for D-Cache cleaning */
static const uint16_t programSizeBytesLut[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 }; 

/*******************************************************************************
* Function Name: Cy_Flash_CallSromApiForFlashWrite
****************************************************************************//**
*
* This function call SROM API driver for flash writing (e.g. program row,
* erase sector). Then, processes response message of the 
* driver API and response message of SROM API if required.
*
* \param apiArgs pointer to SROM API arguments.
*
* \param block driver blocking mode \ref cy_en_flash_driver_blocking_t
*
* \return Returns the status of the Flash operation.
*
*******************************************************************************/
static cy_en_flashdrv_status_t Cy_Flash_CallSromApiForFlashWrite(const un_srom_api_args_t* apiArgs, cy_en_flash_driver_blocking_t block)
{
    if(block == CY_FLASH_DRIVER_BLOCKING)
    {
        un_srom_api_resps_t apiResp = { 0ul };

        /* Call SROM API with blocking mode */
        cy_en_srom_driver_status_t sromDrvStatus;
        sromDrvStatus = Cy_Srom_CallApi(apiArgs, &apiResp);
        if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
        {
            return CY_FLASH_DRV_IPC_BUSY;
        }
        else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
        {
            return CY_FLASH_DRV_SROM_API_TIMEOUT;
        }
        else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
        {
            return CY_FLASH_DRV_ERR_UNC;
        }

        cy_en_flashdrv_status_t result;
        result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
        if(result != CY_FLASH_DRV_SUCCESS)
        {
            return result;
        }

        /* Invalidates the flash cache and buffer */
        Cy_Flashc_InvalidateFlashCacheBuffer();
#if defined (tviibh16m)
        Cy_Flashc_InvalidateFlashCacheBuffer1();
#endif
        return CY_FLASH_DRV_SUCCESS;
    }
    else
    {
        /* Call SROM API with non-blocking mode */
        if (Cy_Srom_CallApi_NonBlock(apiArgs) != CY_SROM_DR_SUCCEEDED)
        {
            /* The IPC structure is already locked by another process */
            return CY_FLASH_DRV_IPC_BUSY;
        }
        return CY_FLASH_DRV_SUCCESS;
    }
}

/*******************************************************************************
* Function Name: Cy_Flash_ProgramRow
****************************************************************************//**
*
* This function writes an array of data to a single row of flash. Reports success or
* or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Write is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_programrow_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_ProgramRow(cy_un_flash_context_t* context, const cy_stc_flash_programrow_config_t* config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if(config == NULL)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address are valid */
    if((Cy_Flash_BoundsCheck((uint32_t)config->destAddr) == CY_FLASH_OUT_OF_BOUNDS) || (config->dataAddr == NULL))
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.ProgramRow.arg0.opcode         = CY_SROM_OP_FLASH_PROGRAM_ROW;
    apiArgs.ProgramRow.arg0.blocking       = config->blocking;
    apiArgs.ProgramRow.arg0.skipBlankCheck = config->skipBC;
    apiArgs.ProgramRow.arg1.dataLoc        = config->dataLoc;
    apiArgs.ProgramRow.arg1.dataSize       = config->dataSize;
    apiArgs.ProgramRow.arg1.interruptMask  = config->intrMask;
    apiArgs.ProgramRow.arg2.dstAddr        = (uint32_t)config->destAddr;
    apiArgs.ProgramRow.arg3.srcAddr        = (uint32_t)config->dataAddr;
    
    // Clean area in SRAM with data to be programmed in case current core has a D-Cache, so that CM0+ SROM handler reads correct data
    // Cleaning of actual SROM scratch area is done later by cy_srom driver
    Cy_SysLib_CleanCoreDCacheByAddr((void *)config->dataAddr, programSizeBytesLut[config->dataSize]);

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_RowChecksum
****************************************************************************//**
*
* Returns a checksum value of the specified flash row. supports only blocking
* mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param rowNum The Cheksum is calculated to the flash row.
*
* \param cheksumPtr The pointer to the address whire cheksum is to be stored
*
* \return Returns the status of the Flash operation.
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_Checksum(cy_un_flash_context_t* context, const cy_stc_flash_checksum_config_t *config, uint32_t* cheksumPtr)
{
    /* Checks whether the input parameters are valid */
    if (config->rowId >= CY_FLASH_NUMBER_ROWS)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    if(CPUSS_FLASH_SIZE >= 4096u)
    {
        apiArgs.CheckSum.arg0.opcode = CY_SROM_OP_FLASH_CHECKSUM;
        apiArgs.CheckSum.arg0.region = config->region;
        apiArgs.CheckSum.arg0.whole  = config->whole;
        apiArgs.CheckSum.arg0.row_id = config->rowId;
        apiArgs.CheckSum.arg0.bank   = config->bank;
    }
    else
    {
        apiArgs.CheckSumLessThan4M.arg0.opcode = CY_SROM_OP_FLASH_CHECKSUM;
        apiArgs.CheckSumLessThan4M.arg0.region = config->region;
        apiArgs.CheckSumLessThan4M.arg0.whole  = config->whole;
        apiArgs.CheckSumLessThan4M.arg0.row_id = config->rowId;
        apiArgs.CheckSumLessThan4M.arg0.bank   = config->bank;
    }

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];
        *cheksumPtr = Cy_IPC_Drv_ReadData1Value(syscall_ipc_struct);
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_Flash_CalculateHash
****************************************************************************//**
*
* Returns a hash value of the specified region of flash. supports only blocking
* mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param data Start the data address.
*
* \param numberOfBytes The hash value is calculated for the number of bytes after the
* start data address (0 - 1 byte, 1- 2 bytes etc).
*
* \param hashPtr The pointer to the address whire hash is to be stored
*
* \return Returns the status of the Flash operation.
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_CalculateHash(cy_un_flash_context_t* context, const cy_stc_flash_computehash_config_t *config,  uint32_t* hashPtr)
{
    /* Checks whether the input parameters are valid */
    if ((config->startAddr == NULL) || (config->numOfByte == 0ul) || (hashPtr == NULL))
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.ComputeHash.arg0.opcode      = CY_SROM_OP_FLASH_HASH;
    apiArgs.ComputeHash.arg0.type        = config->type;
    apiArgs.ComputeHash.arg1.start_addr  = (uint32_t)config->startAddr;
    apiArgs.ComputeHash.arg2.number_byte = (config->numOfByte - 1ul);

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        *hashPtr = apiResp.ComputeHash.resp0.hash_crc;
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_Flash_WorkBoundsCheck
****************************************************************************//**
*
* Checks if Flash addres is out of work region.
*
* \param address Address to be checked
*
* \return \ref en_flash_bounds_t
*
*******************************************************************************/
en_flash_bounds_t Cy_Flash_WorkBoundsCheck(uint32_t address)
{
    if(Cy_Flashc_GetWorkBankMode() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_WFLASH_LG_SBM_TOP <= address) && (address < CY_WFLASH_LG_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if((CY_WFLASH_SM_SBM_TOP <= address) && (address < CY_WFLASH_SM_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }
    else
    {
        if(( CY_WFLASH_LG_DBM0_TOP <= address) && (address < CY_WFLASH_LG_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH_SM_DBM0_TOP <= address) && (address < CY_WFLASH_SM_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH_LG_DBM1_TOP <= address) && (address < CY_WFLASH_LG_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH_SM_DBM1_TOP <= address) && (address < CY_WFLASH_SM_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }

    return(CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_IsWorkSmallSector
****************************************************************************//**
*
* Returns "true" if input address is inside of work small sector region,
* otherwise returns "false".
*
* \param address Address to be checked
*
* \return true - inside of work small sector, false - out of work small sector
*
*******************************************************************************/
bool Cy_Flash_IsWorkSmallSector(uint32_t address)
{
    if(Cy_Flashc_GetWorkBankMode() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if((CY_WFLASH_SM_SBM_TOP <= address) && (address < CY_WFLASH_SM_SBM_END))
        {
            return(true);
        }
    }
    else
    {
        if(( CY_WFLASH_SM_DBM0_TOP <= address) && (address < CY_WFLASH_SM_DBM0_END))
        {
            return(true);
        }

        if(( CY_WFLASH_SM_DBM1_TOP <= address) && (address < CY_WFLASH_SM_DBM1_END))
        {
            return(true);
        }
    }

    return(false);
}

/*******************************************************************************
* Function Name: Cy_Flash_MainBoundsCheck
****************************************************************************//**
*
* Returns 1 if Flash addres is out of main region, otherwise returns 0.
*
* \param address Address to be checked
*
* \return 1 - out of bound, 0 - in flash bounds
*
*******************************************************************************/
//rmkn:WW1735.4: modified macros
en_flash_bounds_t Cy_Flash_MainBoundsCheck(uint32_t address)
{
    if(Cy_Flashc_GetMainBankMode() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_FLASH_LG_SBM_TOP <= address) && (address < CY_FLASH_LG_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH_SM_SBM_TOP <= address) && (address < CY_FLASH_SM_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }
    else
    {
        if(( CY_FLASH_LG_DBM0_TOP <= address) && (address < CY_FLASH_LG_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH_SM_DBM0_TOP <= address) && (address < CY_FLASH_SM_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH_LG_DBM1_TOP <= address) && (address < CY_FLASH_LG_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH_SM_DBM1_TOP <= address) && (address < CY_FLASH_SM_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }

    return(CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_IsCodeSmallSector
****************************************************************************//**
*
* Returns "true" if input address is inside of code small sector region,
* otherwise returns "false".
*
* \param address Address to be checked
*
* \return true - inside of code small sector, false - out of code small sector
*
*******************************************************************************/
bool Cy_Flash_IsMainSmallSector(uint32_t address)
{
    if(Cy_Flashc_GetMainBankMode() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_FLASH_SM_SBM_TOP <= address) && (address < CY_FLASH_SM_SBM_END))
        {
            return(true);
        }
    }
    else
    {
        if(( CY_FLASH_SM_DBM0_TOP <= address) && (address < CY_FLASH_SM_DBM0_END))
        {
            return(true);
        }

        if(( CY_FLASH_SM_DBM1_TOP <= address) && (address < CY_FLASH_SM_DBM1_END))
        {
            return(true);
        }
    }

    return(false);
}

/*******************************************************************************
* Function Name: Cy_Flash_SupervisoryBoundsCheck
****************************************************************************//**
*
* Returns 1 if Flash addres is out of supervisory region, otherwise returns 0.
*
* \param address Address to be checked
*
* \return 1 - out of bound, 0 - in flash bounds
*
*******************************************************************************/
//rmkn:WW1735.4: modified macros
en_flash_bounds_t Cy_Flash_SupervisoryBoundsCheck(uint32_t address)
{
    if(( CY_SFLASH_TOP <= address) && (address < CY_SFLASH_END))
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    if(( CY_SFLASH1_TOP <= address) && (address < CY_SFLASH1_END))
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    return(CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_BoundsCheck
****************************************************************************//**
*
* Returns 1 if Flash addres is out of boundary, otherwise returns 0.
*
* \param flashAddr Address to be checked
*
* \return 1 - out of bound, 0 - in flash bounds
*
*******************************************************************************/
en_flash_bounds_t Cy_Flash_BoundsCheck(uint32_t flashAddr)
{
    if(Cy_Flash_WorkBoundsCheck(flashAddr) == CY_FLASH_IN_BOUNDS)
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    if(Cy_Flash_MainBoundsCheck(flashAddr) == CY_FLASH_IN_BOUNDS)
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    return (CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_ProcessOpcode
****************************************************************************//**
*
* Converts System Call returns to the Flash driver return defines.
*
* \param opcode The value retuned by the System Call.
*
* \return Flash driver return. \ref cy_en_flashdrv_status_t only support a 
*         part of SROM response. please use \ref Cy_Srom_GetApiResponse to
*         get complete status of SROM API.
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_ProcessOpcode(uint32_t opcode)
{
    cy_en_flashdrv_status_t retVal;
    cy_en_srom_api_status_t apiStatus;
    apiStatus = Cy_Srom_ConvertRespToStatus(opcode);

    if(apiStatus == CY_SROM_STATUS_SUCCESS)
    {
        return CY_FLASH_DRV_SUCCESS;
    }
    else if(apiStatus == CY_SROM_STATUS_INVALID)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    // Process error code
    switch (apiStatus)
    {
    case CY_SROM_STATUS_INVALID_PROTECTION:
        retVal = CY_FLASH_DRV_INV_PROT;
        break;
    case CY_SROM_STATUS_INVALID_FM_PL:
        retVal = CY_FLASH_DRV_INVALID_FM_PL;
        break;
    case CY_SROM_STATUS_INVALID_FLASH_ADDR:
        retVal = CY_FLASH_DRV_INVALID_FLASH_ADDR;
        break;
    case CY_SROM_STATUS_NVM_PROTECTED:
        retVal = CY_FLASH_DRV_ROW_PROTECTED;
        break;
    case CY_SROM_STATUS_INVALID_IPC_STRUCT:
        retVal = CY_FLASH_DRV_IPC_BUSY;
        break;
    case CY_SROM_STAUTS_CHECKSUM_NON_ZERO:
        retVal = CY_FLASH_DRV_CHECKSUM_NON_ZERO;
        break;
    case CY_SROM_STATUS_SECTOR_SUSPEND:
        retVal = CY_FLASH_DRV_SECTOR_SUSPEND;
        break;
    case CY_SROM_STATUS_NO_ERASE_SUSPEND:
        retVal = CY_FLASH_DRV_NO_ERASE_SUSPEND;
        break;
    case CY_SROM_STATUS_FLASH_NOT_ERASED:
        retVal = CY_FLASH_DRV_FLASH_NOT_ERASED;
        break;
    case CY_SROM_STATUS_NO_ERASE_ONGOING:
        retVal = CY_FLASH_DRV_NO_ERASE_ONGOING;
        break;
    case CY_SROM_STATUS_ACTIVE_ERASE:
        retVal = CY_FLASH_DRV_ACTIVE_ERASE;
        break;
    case CY_SROM_STAUTS_INVALID_DATA_WIDTH:
        retVal = CY_FLASH_DRV_INVALID_DATA_WIDTH;
        break;
    case CY_SROM_STATUS_FLASH_SAFTEY_ENABLED:
        retVal = CY_FLASH_DRV_FLASH_SAFTEY_ENABLED;
        break;
    case CY_SROM_STATUS_INVALID_SFLASH_ADDR:
        retVal = CY_FLASH_DRV_INVALID_SFLASH_ADDR;
        break;
    case CY_SROM_STATUS_SFLASH_BACKUP_ERASED:
        retVal = CY_FLASH_DRV_SFLASH_BACKUP_ERASED;
        break;
    default:
        retVal = CY_FLASH_DRV_ERR_UNC;
        break;
    }

    return (retVal);
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseAll
****************************************************************************//**
*
* This function erases the whole FLASH macro specified.
* This API will erase only the Main(Code) FLASH array. Does not erase supervisory sectors.
* Reports success or a reason for failure. Does not return until the Erase operation is complete.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Erase is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseAll(cy_un_flash_context_t* context, cy_en_flash_driver_blocking_t block)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseAll.arg0.opcode = CY_SROM_OP_FLASH_ERASE_ALL;

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseSector
****************************************************************************//**
*
* This function Starts the sector erase operation on the specified sector. 
* This function cannot be called on SFLASH. Reports success 
* or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Erase is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of  this function.
* This parameter is defined by the cy_stc_flash_erasesector_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseSector(cy_un_flash_context_t* context, const cy_stc_flash_erasesector_config_t *config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address is in work/main region */
    if(Cy_Flash_BoundsCheck((uint32_t)(config->Addr)) == CY_FLASH_OUT_OF_BOUNDS)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address is word aligned */
    if(((uint32_t)(config->Addr) % sizeof(uint32_t)) != 0ul)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseSector.arg0.opcode        = CY_SROM_OP_FLASH_ERASE_SECTOR;
    apiArgs.EraseSector.arg0.blocking      = config->blocking;
    apiArgs.EraseSector.arg0.interruptMask = config->intrMask;
    apiArgs.EraseSector.arg1.address       = (uint32_t)config->Addr;

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_CheckFMStatus
****************************************************************************//**
*
* This function returns the status of the FLASH operation. 
* Reports success or a reason for failure. 
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param status A pointer that is pointing to memory for storging status value.
* This parameter is defined by the cy_un_fm_status_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_CheckFMStatus(cy_un_flash_context_t* context, cy_un_fm_status_t *status)
{
    /* Checks if input pointers are not NULL */
    if (status == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.CheckFmStatus.arg0.opcode = CY_SROM_OP_FLASH_FM_STATUS;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        status->u16 = (uint16_t)apiResp.CheckFmStatus.resp0.status;
    }
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_ConfigureFMIntr
****************************************************************************//**
*
* This function configures the FLASH macro interrupt.
* Following functionalities are provided.
* 1. Set interrupt mask
* 2. Clear interrupt mask
* 3. Clear interrupt
* Reports success or a reason for failure. 
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param option Selector wchic funcitionality will be operated.
* This parameter is defined by the cy_en_config_fm_intr_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_ConfigureFMIntr(cy_un_flash_context_t* context, cy_en_conf_fm_intr_option_t option)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.ConfigFmIntr.arg0.opcode = CY_SROM_OP_FLASH_FM_STATUS;
    apiArgs.ConfigFmIntr.arg0.option = (uint32_t)option;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseSuspend
****************************************************************************//**
*
* This function suspends an ongoing erase operation. User should not read from a 
* sector which is suspended from an erase operation. Cy_Flash_ProgramRow function 
* will return error if invoked on suspended sector.
* This function cannot be called on SFLASH. Reports success 
* or a reason for failure. Does not return until the Erase operation is complete.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseSuspend(cy_un_flash_context_t* context)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseSuspend.arg0.opcode = CY_SROM_OP_FLASH_ERASE_SUSPEND;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        /* Invalidates the flash cache and buffer */
        Cy_Flashc_InvalidateFlashCacheBuffer();
    }
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseResume
****************************************************************************//**
*
* This function calls to resume a suspended erase operation. 
* Reports success or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_eraseresume_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseResume(cy_un_flash_context_t* context ,const cy_stc_flash_eraseresume_config_t *config)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseResume.arg0.opcode   = CY_SROM_OP_FLASH_ERASE_RESUME;
    apiArgs.EraseResume.arg0.blocking = config->intrMask;
    apiArgs.EraseResume.arg0.intrmask = config->blocking;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    return result;
}


/*******************************************************************************
* Function Name: Cy_Flash_BlankCheck
****************************************************************************//**
*
* This function performs blank check on the addressed work FLASH.
* Reports success or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_blankcheck_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_BlankCheck(cy_un_flash_context_t* context ,const cy_stc_flash_blankcheck_config_t *config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    if(config->numOfWordsToBeChecked == 0)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address are valid */
    if ((Cy_Flash_WorkBoundsCheck((uint32_t)config->addrToBeChecked) == CY_FLASH_OUT_OF_BOUNDS) ||
        (Cy_Flash_WorkBoundsCheck((uint32_t)config->addrToBeChecked + config->numOfWordsToBeChecked*4ul - 1ul) == CY_FLASH_OUT_OF_BOUNDS))
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.BlankCheck.arg0.opcode = CY_SROM_OP_FLASH_BLANK_CHECK;
    apiArgs.BlankCheck.arg1.addrToBeChecked = (uint32_t)config->addrToBeChecked;
    apiArgs.BlankCheck.arg2.numOfWordsToBeChecked = (config->numOfWordsToBeChecked - 1ul);

    /* Tries to acquire the IPC structure and pass the arguments to SROM API */
    if(block == CY_FLASH_DRIVER_BLOCKING)
    {
        /* Call SROM API with blocking mode */
        un_srom_api_resps_t apiResp = { 0ul };
        cy_en_srom_driver_status_t sromDrvStatus;
        sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
        if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
        {
            return CY_FLASH_DRV_IPC_BUSY;
        }
        else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
        {
            return CY_FLASH_DRV_SROM_API_TIMEOUT;
        }
        else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
        {
            return CY_FLASH_DRV_ERR_UNC;
        }

        /* Process response message from API */
        cy_en_flashdrv_status_t result;
        result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
        return result;
    }
    else
    {
        /* Send message by IPC */
        if (Cy_Srom_CallApi_NonBlock(&apiArgs) != CY_SROM_DR_SUCCEEDED)
        {
            /* The IPC structure is already locked by another process */
            return CY_FLASH_DRV_IPC_BUSY;
        }
        return CY_FLASH_DRV_SUCCESS;
    }
}

/*******************************************************************************
* Function Name: Cy_Flashc_InjectECC
****************************************************************************//**
*
* This function enables ECC injection and sets the address where a parity will be injected 
* and the parity value.
* Reports success or a reason for failure.
*
* \param region An indicator which region (Code/Work/Cache) ECC parity will be injected to.
* This parameter is defined by the cy_en_region_t
* in \ref group_flash_macro macro.
*
* \param address The address where ECC parity will be injected.
*
* \param parity The parity value which will be injected.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flashc_InjectECC(cy_en_region_t region, uint32_t address, uint8_t parity)
{
    cy_en_flashdrv_status_t result =CY_FLASH_DRV_SUCCESS;

    switch(region)
    {
    case CY_FLASH_MAIN_REGION:
        if (Cy_Flash_MainBoundsCheck(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
                return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_MainECCInjectionEnable();
        FLASHC->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_CODE_ECC_INJECT_POS) & CY_FLASH_CODE_ECC_INJECT_MASK;
        FLASHC->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_CODE_ECC_INJECT_PARITY_MASK);
        break;
    case CY_FLASH_WORK_REGION:
        if (Cy_Flash_WorkBoundsCheck(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
            return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_WorkECCInjectionEnable();
        FLASHC->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_WORK_ECC_INJECT_POS) & CY_FLASH_WORK_ECC_INJECT_MASK;
        FLASHC->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_WORK_ECC_INJECT_PARITY_MASK);
        break;
#if CY_FLASH_IS_MCU_BC_ENTRY
    case CY_FLASH_CA_CM4_REGION:
        if (Cy_Flash_MainBoundsCheck(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
                return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_CM4_CAECCInjectionEnable();
        FLASHC->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_CACHE_ECC_INJECT_POS) & CY_FLASH_CACHE_ECC_INJECT_MASK;
        FLASHC->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_CACHE_ECC_INJECT_PARITY_MASK);
        break;
#endif
        case CY_FLASH_CA_CM0P_REGION:
        if (Cy_Flash_MainBoundsCheck(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
                return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_CM0_CAECCInjectionEnable();
        FLASHC->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_CACHE_ECC_INJECT_POS) & CY_FLASH_CACHE_ECC_INJECT_MASK;
        FLASHC->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_CACHE_ECC_INJECT_PARITY_MASK);
        break;
    default:
        break;
    }

    // Make sure outstanding transfer done
    (void)FLASHC->unECC_CTL.u32Register;
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_GetDrvStatus
****************************************************************************//**
*
* This function returns driver status
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
*
* \return Driver status.
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_GetDrvStatus(cy_un_flash_context_t *context)
{
    un_srom_api_resps_t resp;
    Cy_Srom_GetApiResponse(&resp);
    return Cy_Flash_ProcessOpcode(resp.resp[0]);
}

#if defined (tviibh16m)
/*******************************************************************************
* Function Name: Cy_Flash_ProgramRow1
****************************************************************************//**
*
* This function writes an array of data to a single row of flash. Reports success or
* or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Write is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_programrow_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_ProgramRow1(cy_un_flash_context_t* context, const cy_stc_flash_programrow_config_t* config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if(config == NULL)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address are valid */
    if((Cy_Flash_BoundsCheck1((uint32_t)config->destAddr) == CY_FLASH_OUT_OF_BOUNDS) || (config->dataAddr == NULL))
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.ProgramRow.arg0.opcode         = CY_SROM_OP_FLASH_PROGRAM_ROW2;

    apiArgs.ProgramRow.arg0.blocking       = config->blocking;
    apiArgs.ProgramRow.arg0.skipBlankCheck = config->skipBC;
    apiArgs.ProgramRow.arg1.dataLoc        = config->dataLoc;
    apiArgs.ProgramRow.arg1.dataSize       = config->dataSize;
    apiArgs.ProgramRow.arg1.interruptMask  = config->intrMask;
    apiArgs.ProgramRow.arg2.dstAddr        = (uint32_t)config->destAddr;
    apiArgs.ProgramRow.arg3.srcAddr        = (uint32_t)config->dataAddr;

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_RowChecksum1
****************************************************************************//**
*
* Returns a checksum value of the specified flash row. supports only blocking
* mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param rowNum The Cheksum is calculated to the flash row.
*
* \param cheksumPtr The pointer to the address whire cheksum is to be stored
*
* \return Returns the status of the Flash operation.
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_Checksum1(cy_un_flash_context_t* context, const cy_stc_flash_checksum_config_t *config, uint32_t* cheksumPtr)
{
    /* Checks whether the input parameters are valid */
    if (config->rowId >= CY_FLASH_NUMBER_ROWS)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    if(CPUSS_FLASH_SIZE >= 4096u)
    {
        apiArgs.CheckSum.arg0.opcode = CY_SROM_OP_FLASH_CHECKSUM2;    
        apiArgs.CheckSum.arg0.region = config->region;
        apiArgs.CheckSum.arg0.whole  = config->whole;
        apiArgs.CheckSum.arg0.row_id = config->rowId;
        apiArgs.CheckSum.arg0.bank   = config->bank;
    }
    else
    {
        apiArgs.CheckSumLessThan4M.arg0.opcode = CY_SROM_OP_FLASH_CHECKSUM2;       
        apiArgs.CheckSumLessThan4M.arg0.region = config->region;
        apiArgs.CheckSumLessThan4M.arg0.whole  = config->whole;
        apiArgs.CheckSumLessThan4M.arg0.row_id = config->rowId;
        apiArgs.CheckSumLessThan4M.arg0.bank   = config->bank;
    }

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];
        *cheksumPtr = Cy_IPC_Drv_ReadData1Value(syscall_ipc_struct);
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_Flash_WorkBoundsCheck1
****************************************************************************//**
*
* Checks if Flash addres is out of work region.
*
* \param address Address to be checked
*
* \return \ref en_flash_bounds_t
*
*******************************************************************************/
en_flash_bounds_t Cy_Flash_WorkBoundsCheck1(uint32_t address)
{
    if(Cy_Flashc_GetWorkBankMode1() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_WFLASH1_LG_SBM_TOP <= address) && (address < CY_WFLASH1_LG_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if((CY_WFLASH1_SM_SBM_TOP <= address) && (address < CY_WFLASH1_SM_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }
    else
    {
        if(( CY_WFLASH1_LG_DBM0_TOP <= address) && (address < CY_WFLASH1_LG_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH1_SM_DBM0_TOP <= address) && (address < CY_WFLASH1_SM_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH1_LG_DBM1_TOP <= address) && (address < CY_WFLASH1_LG_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_WFLASH1_SM_DBM1_TOP <= address) && (address < CY_WFLASH1_SM_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }

    return(CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_IsWorkSmallSector1
****************************************************************************//**
*
* Returns "true" if input address is inside of work small sector region,
* otherwise returns "false".
*
* \param address Address to be checked
*
* \return true - inside of work small sector, false - out of work small sector
*
*******************************************************************************/
bool Cy_Flash_IsWorkSmallSector1(uint32_t address)
{
    if(Cy_Flashc_GetWorkBankMode1() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if((CY_WFLASH1_SM_SBM_TOP <= address) && (address < CY_WFLASH1_SM_SBM_END))
        {
            return(true);
        }
    }
    else
    {
        if(( CY_WFLASH1_SM_DBM0_TOP <= address) && (address < CY_WFLASH1_SM_DBM0_END))
        {
            return(true);
        }

        if(( CY_WFLASH1_SM_DBM1_TOP <= address) && (address < CY_WFLASH1_SM_DBM1_END))
        {
            return(true);
        }
    }

    return(false);
}

/*******************************************************************************
* Function Name: Cy_Flash_MainBoundsCheck1
****************************************************************************//**
*
* Returns 1 if Flash addres is out of main region, otherwise returns 0.
*
* \param address Address to be checked
*
* \return 1 - out of bound, 0 - in flash bounds
*
*******************************************************************************/
en_flash_bounds_t Cy_Flash_MainBoundsCheck1(uint32_t address)
{
    if(Cy_Flashc_GetMainBankMode1() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_FLASH1_LG_SBM_TOP <= address) && (address < CY_FLASH1_LG_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH1_SM_SBM_TOP <= address) && (address < CY_FLASH1_SM_SBM_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }
    else
    {
        if(( CY_FLASH1_LG_DBM0_TOP <= address) && (address < CY_FLASH1_LG_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH1_SM_DBM0_TOP <= address) && (address < CY_FLASH1_SM_DBM0_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH1_LG_DBM1_TOP <= address) && (address < CY_FLASH1_LG_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }

        if(( CY_FLASH1_SM_DBM1_TOP <= address) && (address < CY_FLASH1_SM_DBM1_END))
        {
            return(CY_FLASH_IN_BOUNDS);
        }
    }

    return(CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_IsCodeSmallSector1
****************************************************************************//**
*
* Returns "true" if input address is inside of code small sector region,
* otherwise returns "false".
*
* \param address Address to be checked
*
* \return true - inside of code small sector, false - out of code small sector
*
*******************************************************************************/
bool Cy_Flash_IsMainSmallSector1(uint32_t address)
{
    if(Cy_Flashc_GetMainBankMode1() == CY_FLASH_SINGLE_BANK_MODE)
    {
        if(( CY_FLASH1_SM_SBM_TOP <= address) && (address < CY_FLASH1_SM_SBM_END))
        {
            return(true);
        }
    }
    else
    {
        if(( CY_FLASH1_SM_DBM0_TOP <= address) && (address < CY_FLASH1_SM_DBM0_END))
        {
            return(true);
        }

        if(( CY_FLASH1_SM_DBM1_TOP <= address) && (address < CY_FLASH1_SM_DBM1_END))
        {
            return(true);
        }
    }

    return(false);
}

/*******************************************************************************
* Function Name: Cy_Flash_BoundsCheck1
****************************************************************************//**
*
* Returns 1 if Flash addres is out of boundary, otherwise returns 0.
*
* \param flashAddr Address to be checked
*
* \return 1 - out of bound, 0 - in flash bounds
*
*******************************************************************************/
en_flash_bounds_t Cy_Flash_BoundsCheck1(uint32_t flashAddr)
{
    if(Cy_Flash_WorkBoundsCheck1(flashAddr) == CY_FLASH_IN_BOUNDS)
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    if(Cy_Flash_MainBoundsCheck1(flashAddr) == CY_FLASH_IN_BOUNDS)
    {
        return(CY_FLASH_IN_BOUNDS);
    }

    return (CY_FLASH_OUT_OF_BOUNDS);
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseAll1
****************************************************************************//**
*
* This function erases the whole FLASH macro specified.
* This API will erase only the Main(Code) FLASH array. Does not erase supervisory sectors.
* Reports success or a reason for failure. Does not return until the Erase operation is complete.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Erase is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
* \note row or page is same and of the size 512 bytes
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseAll1(cy_un_flash_context_t* context, cy_en_flash_driver_blocking_t block)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };

    apiArgs.EraseAll.arg0.opcode = CY_SROM_OP_FLASH_ERASE_ALL2;

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseSector1
****************************************************************************//**
*
* This function Starts the sector erase operation on the specified sector. 
* This function cannot be called on SFLASH. Reports success 
* or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash. User firmware should not enter the hibernate
* mode until flash Erase is complete. The Flash operation is allowed in Sleep and
* Deep-sleep modes. During the Flash operation, the device should not be reset,
* including the XRES pin, a software reset, and watchdog reset sources. Also,
* low-voltage detect circuits should be configured to generate an interrupt instead
* of a reset. Otherwise, portions of flash may undergo unexpected changes.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of  this function.
* This parameter is defined by the cy_stc_flash_erasesector_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseSector1(cy_un_flash_context_t* context, const cy_stc_flash_erasesector_config_t *config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address is in work/main region */
    if(Cy_Flash_BoundsCheck1((uint32_t)(config->Addr)) == CY_FLASH_OUT_OF_BOUNDS)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address is word aligned */
    if(((uint32_t)(config->Addr) % sizeof(uint32_t)) != 0ul)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseSector.arg0.opcode        = CY_SROM_OP_FLASH_ERASE_SECTOR2; 
    apiArgs.EraseSector.arg0.blocking      = config->blocking;
    apiArgs.EraseSector.arg0.interruptMask = config->intrMask;
    apiArgs.EraseSector.arg1.address       = (uint32_t)config->Addr;

    /* Call SROM API driver and process response */
    return Cy_Flash_CallSromApiForFlashWrite(&apiArgs, block);
}

/*******************************************************************************
* Function Name: Cy_Flash_CheckFMStatus1
****************************************************************************//**
*
* This function returns the status of the FLASH operation. 
* Reports success or a reason for failure. 
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param status A pointer that is pointing to memory for storging status value.
* This parameter is defined by the cy_un_fm_status_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_CheckFMStatus1(cy_un_flash_context_t* context, cy_un_fm_status_t *status)
{
    /* Checks if input pointers are not NULL */
    if (status == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.CheckFmStatus.arg0.opcode = CY_SROM_OP_FLASH_FM_STATUS2;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        status->u16 = (uint16_t)apiResp.CheckFmStatus.resp0.status;
    }
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_ConfigureFMIntr1
****************************************************************************//**
*
* This function configures the FLASH macro interrupt.
* Following functionalities are provided.
* 1. Set interrupt mask
* 2. Clear interrupt mask
* 3. Clear interrupt
* Reports success or a reason for failure. 
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param option Selector wchic funcitionality will be operated.
* This parameter is defined by the cy_en_config_fm_intr_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_ConfigureFMIntr1(cy_un_flash_context_t* context, cy_en_conf_fm_intr_option_t option)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.ConfigFmIntr.arg0.opcode = CY_SROM_OP_FLASH_FM_STATUS2;    
    apiArgs.ConfigFmIntr.arg0.option = (uint32_t)option;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseSuspend1
****************************************************************************//**
*
* This function suspends an ongoing erase operation. User should not read from a 
* sector which is suspended from an erase operation. Cy_Flash_ProgramRow1 function 
* will return error if invoked on suspended sector.
* This function cannot be called on SFLASH. Reports success 
* or a reason for failure. Does not return until the Erase operation is complete.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
* This function supports only blocking mode for now.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseSuspend1(cy_un_flash_context_t* context)
{
    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseSuspend.arg0.opcode = CY_SROM_OP_FLASH_ERASE_SUSPEND2;   

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    if(result == CY_FLASH_DRV_SUCCESS)
    {
        /* Invalidates the flash cache and buffer */
        Cy_Flashc_InvalidateFlashCacheBuffer1();
    }
    return result;
}

/*******************************************************************************
* Function Name: Cy_Flash_EraseResume1
****************************************************************************//**
*
* This function calls to resume a suspended erase operation. 
* Reports success or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_eraseresume_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_EraseResume1(cy_un_flash_context_t* context ,const cy_stc_flash_eraseresume_config_t *config)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.EraseResume.arg0.opcode   = CY_SROM_OP_FLASH_ERASE_RESUME2; 
    apiArgs.EraseResume.arg0.blocking = config->intrMask;
    apiArgs.EraseResume.arg0.intrmask = config->blocking;

    /* Call SROM API with blocking mode */
    un_srom_api_resps_t apiResp = { 0ul };
    cy_en_srom_driver_status_t sromDrvStatus;
    sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
    if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
    {
        return CY_FLASH_DRV_IPC_BUSY;
    }
    else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
    {
        return CY_FLASH_DRV_SROM_API_TIMEOUT;
    }
    else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
    {
        return CY_FLASH_DRV_ERR_UNC;
    }

    /* Process response message from API */
    cy_en_flashdrv_status_t result;
    result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
    return result;
}


/*******************************************************************************
* Function Name: Cy_Flash_BlankCheck1
****************************************************************************//**
*
* This function performs blank check on the addressed work FLASH.
* Reports success or a reason for failure.
* Returns immediately and reports a CY_FLASH_DRV_IPC_BUSY error in the case when another
* process is operating flash.
*
* \param context The pointer to the Flash driver context defined by the user.
* The flash driver context contains configuration data for flash operation.
* (This is no longer used. I just remain this because of backward compatibility)
*
* \param config configuration of this function.
* This parameter is defined by the cy_stc_flash_blankcheck_config_t
* in \ref group_flash_srom_config_structure macro.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flash_BlankCheck1(cy_un_flash_context_t* context ,const cy_stc_flash_blankcheck_config_t *config, cy_en_flash_driver_blocking_t block)
{
    /* Checks if input pointers are not NULL */
    if (config == NULL)
    {
        return (cy_en_flashdrv_status_t)CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    if(config->numOfWordsToBeChecked == 0)
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Checks if input address are valid */
    if ((Cy_Flash_WorkBoundsCheck1((uint32_t)config->addrToBeChecked) == CY_FLASH_OUT_OF_BOUNDS) ||
        (Cy_Flash_WorkBoundsCheck1((uint32_t)config->addrToBeChecked + config->numOfWordsToBeChecked*4ul - 1ul) == CY_FLASH_OUT_OF_BOUNDS))
    {
        return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
    }

    /* Prepares arguments to be passed to SROM API */
    un_srom_api_args_t apiArgs = { 0ul };
    apiArgs.BlankCheck.arg0.opcode = CY_SROM_OP_FLASH_BLANK_CHECK;
    apiArgs.BlankCheck.arg1.addrToBeChecked = (uint32_t)config->addrToBeChecked;
    apiArgs.BlankCheck.arg2.numOfWordsToBeChecked = (config->numOfWordsToBeChecked - 1ul);

    /* Tries to acquire the IPC structure and pass the arguments to SROM API */
    if(block == CY_FLASH_DRIVER_BLOCKING)
    {
        /* Call SROM API with blocking mode */
        un_srom_api_resps_t apiResp = { 0ul };
        cy_en_srom_driver_status_t sromDrvStatus;
        sromDrvStatus = Cy_Srom_CallApi(&apiArgs, &apiResp);
        if(sromDrvStatus == CY_SROM_DR_IPC_BUSY)
        {
            return CY_FLASH_DRV_IPC_BUSY;
        }
        else if(sromDrvStatus == CY_SROM_DR_TIMEOUT)
        {
            return CY_FLASH_DRV_SROM_API_TIMEOUT;
        }
        else if(sromDrvStatus == CY_SROM_DR_API_UNKNOWN)
        {
            return CY_FLASH_DRV_ERR_UNC;
        }

        /* Process response message from API */
        cy_en_flashdrv_status_t result;
        result = Cy_Flash_ProcessOpcode(apiResp.resp[0]);
        return result;
    }
    else
    {
        /* Send message by IPC */
        if (Cy_Srom_CallApi_NonBlock(&apiArgs) != CY_SROM_DR_SUCCEEDED)
        {
            /* The IPC structure is already locked by another process */
            return CY_FLASH_DRV_IPC_BUSY;
        }
        return CY_FLASH_DRV_SUCCESS;
    }
}

/*******************************************************************************
* Function Name: Cy_Flashc_InjectECC1
****************************************************************************//**
*
* This function enables ECC injection and sets the address where a parity will be injected 
* and the parity value.
* Reports success or a reason for failure.
*
* \param region An indicator which region (Code/Work/Cache) ECC parity will be injected to.
* This parameter is defined by the cy_en_region_t
* in \ref group_flash_macro macro.
*
* \param address The address where ECC parity will be injected.
*
* \param parity The parity value which will be injected.
*
* \return Returns the status of the Flash operation (see \ref cy_en_flashdrv_status_t).
*
*******************************************************************************/
cy_en_flashdrv_status_t Cy_Flashc_InjectECC1(cy_en_region_t region, uint32_t address, uint8_t parity)
{
    cy_en_flashdrv_status_t result =CY_FLASH_DRV_SUCCESS;

    switch(region)
    {
    case CY_FLASH_MAIN_REGION:
        if (Cy_Flash_MainBoundsCheck1(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
                return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_MainECCInjectionEnable1();
        FLASHC1->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_CODE_ECC_INJECT_POS) & CY_FLASH_CODE_ECC_INJECT_MASK;
        FLASHC1->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_CODE_ECC_INJECT_PARITY_MASK);
        break;
    case CY_FLASH_WORK_REGION:
        if (Cy_Flash_WorkBoundsCheck1(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
            return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_WorkECCInjectionEnable1();
        FLASHC1->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_WORK_ECC_INJECT_POS) & CY_FLASH_WORK_ECC_INJECT_MASK;
        FLASHC1->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_WORK_ECC_INJECT_PARITY_MASK);
        break;
    case CY_FLASH_CA_CM0P_REGION:
        if (Cy_Flash_MainBoundsCheck1(address) == CY_FLASH_OUT_OF_BOUNDS)
        {
                return CY_FLASH_DRV_INVALID_INPUT_PARAMETERS;
        }
        Cy_Flashc_CM0_CAECCInjectionEnable1();
        FLASHC1->unECC_CTL.stcField.u24WORD_ADDR = (address >> CY_FLASH_CACHE_ECC_INJECT_POS) & CY_FLASH_CACHE_ECC_INJECT_MASK;
        FLASHC1->unECC_CTL.stcField.u8PARITY = (parity & CY_FLASH_CACHE_ECC_INJECT_PARITY_MASK);
        break;
    default:
        break;
    }

    // Make sure outstanding transfer done
    (void)FLASHC1->unECC_CTL.u32Register;
    return result;
}
#endif

/* [] END OF FILE */
