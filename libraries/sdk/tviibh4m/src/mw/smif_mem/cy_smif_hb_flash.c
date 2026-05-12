/* cy_smif_hb_flash.c - Source Code for Cypress's Low Level Driver */

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

#include <string.h>

#include "cy_smif_device_common.h"

#define CY_SMIF_HB_FLASH_BUFFER_SIZE (256*CY_SMIF_HB_FLASH_BUF_SIZE_MULTIPLIER)
#define CY_SMIF_HB_FLASH_SUSP_RESUME_DELAY  40

/*****************************************************************************
 ** \brief Pattern that should be used to calibrate the delay of RWDS capturing
 *****************************************************************************/
const uint8_t Cy_SMIF_HB_CalibrationDataPattern[CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH] = { 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x55, 0xAA };


/*****************************************************************************
 ** \brief HyperBus Latency Code to Frequency Threshold Mapping
 *****************************************************************************/
typedef struct
{
    uint8_t                       freqThresholdMhz;
    cy_en_smif_hb_latency_code_t  latencyCode;
} cy_smif_hb_latency_lut_t;


/* private functions */

/* Public Functions  */

/*******************************************************************************
* Function Name: Cy_SMIF_InitDeviceHyperBus
****************************************************************************//**
*
* This function sets up SMIF registers which is used in XIP mode for hyper bus
* memory.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* Configuration to be applied to the SMIF device \ref cy_stc_device_hb_config_t
*
* \return
*     - \ref CY_SMIF_BAD_PARAM
*     - \ref CY_SMIF_SUCCESS
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_InitDeviceHyperBus(volatile cy_stc_smif_reg_device_t *device, const cy_stc_device_hb_config_t *config)
{
    /********************/
    /* Check parameters */
    /********************/
    uint32_t startAddress  = config->addr;
    uint32_t maxDeviceSize = (~(CY_SMIF_GetXIP_Mask(device))) + 1UL;
    uint32_t size          = Cy_SMIF_MaskValueToSizeInByte(config->size);
    uint32_t endAddress    = config->addr + size - 1;

    if(startAddress < CY_SMIF_GetXIP_Address(device))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(endAddress > ((uint32_t)CY_SMIF_GetXIP_Address(device) + maxDeviceSize - 1UL))
    {
        return CY_SMIF_BAD_PARAM;
    }

    /*********************/
    /*   Set registers   */
    /*********************/
    Cy_SMIF_DeviceSetDataSelect(device, CY_SMIF_DATA_SEL0);
    Cy_SMIF_DeviceCryptoDisable(device);

    if(config->mergeEnable)
    {
        Cy_SMIF_DeviceTransferMergingEnable(device, config->mergeTimeout);
    }
    else
    {
        Cy_SMIF_DeviceTransferMergingDisable(device);
    }

    if(config->totalTimeoutEnable)
    {
        Cy_SMIF_DeviceTotalTimeoutEnable(device, config->totalTimeout);
    }
    else
    {
        Cy_SMIF_DeviceTotalTimeoutDisable(device);
    }

    Cy_SMIF_SetXIPBaseAddress(device, config->addr);
    Cy_SMIF_SetXIPAddressSize(device, config->size);
    Cy_SMIF_SetXIPAddrCtl(device, CY_SMIF_XIP_ADDRESS_5_BYTE, false /* not Dual Quad */);

    /* Note: All read/write sequence settings depending on the dummy cycles are moved to the following function   */
    /*       because it may be necessary to update them during runtime when the latency settings in the connected */
    /*       memories have been changed. Cy_SMIF_HB_SetDummyCycles can then be used for that purpose.             */
    Cy_SMIF_HB_SetDummyCycles(device, config->hbDevType, config->lc_hb);

    /****************************/
    /*   Set XIP Read Sequence  */
    /****************************/

    /*** Set command ***/
    {
        stc_cy_smif_cmd_mode_ctl_t rdCmdParams;
        rdCmdParams.cmd       = config->xipReadCmd;
        rdCmdParams.width     = CY_SMIF_WIDTH_OCTAL;
        rdCmdParams.dataRate  = CY_SMIF_DDR;
        rdCmdParams.presence  = CY_SMIF_PRESENT_1BYTE;
        Cy_SMIF_SetReadCmdCtl(device, &rdCmdParams);
    }

    /*** Set address ***/
    {
        stc_cy_smif_addr_data_ctl_t rdAddrParams;
        rdAddrParams.width    = CY_SMIF_WIDTH_OCTAL;
        rdAddrParams.dataRate = CY_SMIF_DDR;
        Cy_SMIF_SetReadAddrCtl(device, &rdAddrParams);
    }

    /*** Set mode ***/
    {
        stc_cy_smif_cmd_mode_ctl_t rdModeParams;
        rdModeParams.cmd      = 0ul;
        rdModeParams.width    = CY_SMIF_WIDTH_SINGLE;
        rdModeParams.dataRate = CY_SMIF_SDR;
        rdModeParams.presence = CY_SMIF_NOT_PRESENT;
        Cy_SMIF_SetReadModeCtl(device, &rdModeParams);
    }

    /*** Set data ***/
    {
        stc_cy_smif_addr_data_ctl_t rdDataParams;
        rdDataParams.width    = CY_SMIF_WIDTH_OCTAL;
        rdDataParams.dataRate = CY_SMIF_DDR;
        Cy_SMIF_SetReadDataCtl(device, &rdDataParams);
    }

    /***   Set CRC   ***/
    {
        // Not cared yet (YOTS)
        stc_cy_smif_rd_crc_ctl_t rdCRCParams = { 0u };
        Cy_SMIF_SetReadCrcCtl(device, &rdCRCParams);
    }

    /*****************************/
    /*   Set XIP Write Sequence  */
    /*****************************/
    /*** Set command ***/
    {
        stc_cy_smif_cmd_mode_ctl_t wtCmdParams;
        wtCmdParams.cmd       = config->xipWriteCmd;
        wtCmdParams.width     = CY_SMIF_WIDTH_OCTAL;
        wtCmdParams.dataRate  = CY_SMIF_DDR;
        wtCmdParams.presence  = CY_SMIF_PRESENT_1BYTE;
        Cy_SMIF_SetWriteCmdCtl(device, &wtCmdParams);
    }

    /*** Set address ***/
    {
        stc_cy_smif_addr_data_ctl_t wtAddrParams;
        wtAddrParams.width    = CY_SMIF_WIDTH_OCTAL;
        wtAddrParams.dataRate = CY_SMIF_DDR;
        Cy_SMIF_SetWriteAddrCtl(device, &wtAddrParams);
    }

    /*** Set mode ***/
    {
        stc_cy_smif_cmd_mode_ctl_t wtModeParams;
        wtModeParams.cmd      = 0ul;
        wtModeParams.width    = CY_SMIF_WIDTH_SINGLE;
        wtModeParams.dataRate = CY_SMIF_SDR;
        wtModeParams.presence = CY_SMIF_NOT_PRESENT;
        Cy_SMIF_SetWriteModeCtl(device, &wtModeParams);
    }

    /*** Set data ***/
    {
        stc_cy_smif_addr_data_ctl_t wtDataParams;
        wtDataParams.width    = CY_SMIF_WIDTH_OCTAL;
        wtDataParams.dataRate = CY_SMIF_DDR;
        Cy_SMIF_SetWriteDataCtl(device, &wtDataParams);
    }

    Cy_SMIF_DeviceWriteEnable(device);
    Cy_SMIF_DeviceEnable(device);

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_HB_SetDummyCycles
****************************************************************************//**
*
* This function updates the dummy cycles in the XIP read/write sequences
* based on the specified latency code and device type.
* If called for an already initialized device during runtime (e.g. after
* updating the latency settings in the connected memory) ensure that SMIF is 
* not busy and no access to XIP address space happens!
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param hbDevType
* Device type (HyperFlash or HyperRAM)
*
* \param lc_hb
* Latency code that shall be applied
*
* \return
*     - \ref CY_SMIF_SUCCESS
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_HB_SetDummyCycles(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_dev_type_t hbDevType, cy_en_smif_hb_latency_code_t lc_hb)
{
    // Latency code to acctual latency cycle
    uint8_t dummyCycle = Cy_SMIF_HB_LatencyCodeToCycle(lc_hb);

    /**********************************************/
    /*   Set dummy cycles for XIP Read Sequence  */
    /**********************************************/
    /*** Set dummy ***/
    {
        stc_cy_smif_rd_dummy_ctl_t rdDummyParams;
        rdDummyParams.presence   = CY_SMIF_PRESENT_2BYTE;
        rdDummyParams.numOfDummy = dummyCycle - 1; // For Hyperbus, (RD DUMMY_CTL SIZE5 + 2) = initial latency
        Cy_SMIF_SetReadDummyCtl(device, &rdDummyParams);
    }    

    /***  Set bound  ***/
    {
        // only S26K and S26H for now
        stc_cy_smif_rd_bound_ctl_t rdBoundParams = { 0u };
        if(hbDevType == CY_SMIF_HB_FLASH)
        {
            rdBoundParams.numOfCycle       = dummyCycle;
            rdBoundParams.subPageSize      = SUB_PAGE_SIZE_8BYTE;
            rdBoundParams.subPageNr        = SUB_PAGE_2_PER_PAGE;
            rdBoundParams.subsqBndEn       = false;
            rdBoundParams.bdLPresent       = true;
        }
        Cy_SMIF_SetReadBoundCtl(device, &rdBoundParams);
    }

    /**********************************************/
    /*   Set dummy cycles for XIP Write Sequence  */
    /**********************************************/
    /*** Set dummy ***/
    {
        stc_cy_smif_wr_dummy_ctl_t wtDummyParams;
        if(hbDevType == CY_SMIF_HB_FLASH)
        {
            wtDummyParams.presence   = CY_SMIF_NOT_PRESENT;
            wtDummyParams.numOfDummy = 0;
            wtDummyParams.rwdsEn     = false;
        }
        else
        {
            wtDummyParams.presence   = CY_SMIF_PRESENT_2BYTE;
            wtDummyParams.numOfDummy = (dummyCycle - 1); // For Hyperbus, (RD DUMMY_CTL SIZE5 + 2) = initial latency
            wtDummyParams.rwdsEn     = true;
        }
        Cy_SMIF_SetWriteDummyCtl(device, &wtDummyParams);
    }

    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_HB_CheckOrPrepareCalibrationDataPattern
****************************************************************************//**
*
* This function checks whether the calibration data pattern is existing
* at the specified address. If it is not existing it tries to prepare the pattern
* by either writing to HyperRAM or programming HyperFlash (with optional prior
* sector erase). The return value can be used to determine whether a subsequent
* calibration attempt can be made (e.g. by calling Cy_SMIF_HB_CalibrateDelay)
*
* \note Function must be called in XIP mode and CPU/SMIF caches need to be disabled
*       and XIP address space memory attributes should be "Device"!
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param hbDevType
* Device type (HyperFlash or HyperRAM)
*
* \param calibrationDataAddr
* (XIP) Address of/for the calibration data. Must be 16-byte aligned in case of HyperFlash!
*
* \param eraseIfNecessary
* In case of HyperFlash: Allows the function to erase the target sector if the calibration
* data target area is not blank and not equal to the calibration data. May erase unrelated data!
* Value is "don't care" in case of HyperRAM.
*
*
* \return
* "true" if 
*  - calibration data is already available at calibrationDataAddr
*  - calibration data has been successfully written/programmed to calibrationDataAddr
* "false" if
*  - calibrationDataAddr is not 16-byte aligned in case of HyperFlash
*  - eraseIfNecessary is "false" and calibration data target area is not blank and
*    data does not match the actual calibration data pattern
*  - any other error while erasing/programming/writing/verifying
*
*******************************************************************************/
bool Cy_SMIF_HB_CheckOrPrepareCalibrationDataPattern(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_dev_type_t hbDevType, uint32_t calibrationDataAddr, bool eraseIfNecessary)
{
    // Reference data pattern for a blank flash
    const uint8_t flashBlankData[CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    bool isSuccessful = true;

    if(hbDevType == CY_SMIF_HB_SRAM)
    {
        // Copy pattern to HyperRAM and verify it
        memcpy(   (void *) calibrationDataAddr, (const void *) Cy_SMIF_HB_CalibrationDataPattern, (size_t) CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH);
        if(memcmp((void *) calibrationDataAddr, (const void *) Cy_SMIF_HB_CalibrationDataPattern, (size_t) CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH) != 0)
        {
            isSuccessful = false;
        }
    }
    else // HyperFlash
    {
        // Check proper alignment
        if((calibrationDataAddr % 16) != 0)
        {
            isSuccessful = false;
        }
        else
        {
            // Check if calibration data pattern already exists at the target location in HyperFlash
            bool isPattern = (memcmp((void *) calibrationDataAddr, (const void *) Cy_SMIF_HB_CalibrationDataPattern, (size_t) CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH) == 0) ? true : false;
            // Check if the target location in HyperFlash is "blank" and hence can be programmed without prior erase
            bool isBlank   = (memcmp((void *) calibrationDataAddr, (const void *) flashBlankData,                    (size_t) CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH) == 0) ? true : false;

            if(isPattern == false)
            {
                // Driver APIs require some base address (used for the unlock commands) and offset, so we mask the calibrationDataAddr accordingly
                volatile CY_SMIF_FLASHDATA * target_base   = (volatile CY_SMIF_FLASHDATA *) (calibrationDataAddr & 0xfffff000);
                CY_SMIF_ADDRESS              target_offset =                                (calibrationDataAddr & 0x00000fff);
                
                if(isBlank == false)
                {
                    if(eraseIfNecessary == false)
                    {
                        // Erase required, bu user did not allow it --> Calibration will not be possible
                        isSuccessful = false;
                    }
                    else
                    {
                        // Erase sector which the calibration data address belongs to. (Note: 'target_offset' needs to be divided because the API expects 16-bit word addressing offsets)
                        CY_SMIF_HB_DEVSTATUS eraseStatus = CY_SMIF_HbFlash_SectorEraseOp(target_base, target_offset / 2);
                        if(eraseStatus != CY_SMIF_DEV_NOT_BUSY)
                        {
                            isSuccessful = false;
                        }
                    }

                }

                // If previous steps (erase) were not successful, we do not need to try programming
                if(isSuccessful == true)
                {
                    // Program calibration data pattern. (Note: 'target_offset' needs to be divided because the API expects 16-bit word addressing offsets)
                    CY_SMIF_HB_DEVSTATUS progStatus = CY_SMIF_HbFlash_WriteBufferProgramOp(target_base, target_offset / 2, CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH / 2, (CY_SMIF_FLASHDATA*) Cy_SMIF_HB_CalibrationDataPattern);
                    if(progStatus != CY_SMIF_DEV_NOT_BUSY)
                    {
                        isSuccessful = false;
                    }
                }
            }
        }  
    }

    return isSuccessful;
}

/*******************************************************************************
* Function Name: Cy_SMIF_HB_CalibrateDelay
****************************************************************************//**
*
* This function reads the calibration data pattern in the Hyper memory for every
* delay tap of the currently selected delay line and records whether it matches
* the reference pattern. After all taps have been scanned, it determines the
* center tap of the longest sequence of matches and applies this tap.
*
* \note Function assumes that any SMIF has the same number of delay taps
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param lc_hb
* Latency code that is currently applied
*
* \param calibrationDataOffsetFromBase
* Address offset of the calibration data from the device's XIP base address
*
* \param context
* Current SMIF driver context
*
* \param scanAllDelayLines
* If "false" only the taps of the currently selected delay line are checked,
* if "true" all delay lines are scanned for the best match (not implemented yet!)
*
*
* \return \ref cy_en_smif_status_t
*       
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_HB_CalibrateDelay(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_latency_code_t lc_hb, uint32_t calibrationDataOffsetFromBase, cy_stc_smif_context_t *context, bool scanAllDelayLines)
{
    uint16_t                  tapNum;
    cy_en_smif_status_t       retVal    = CY_SMIF_SUCCESS;
    volatile cy_stc_smif_reg_t *     smifBase  = Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(device);
    cy_en_smif_slave_select_t slave     = Cy_SMIF_GetSlaveSelectFromDeviceBasePtr(device);

    uint8_t dataRead[CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH];
    uint32_t delayTapNum = CY_SMIF_GetDelayTapsNumber(device);
    CY_ASSERT(delayTapNum < 256);  // Assume "delayTapNum" is less than 256
    static bool isMatch[256]; // static variable to avoid consuming stack.

    cy_en_smif_mode_t smifModeBackup = Cy_SMIF_GetMode(smifBase);
    
    // Reading of calibration data pattern needs to happen in SMIF MMIO mode so that it can be ensured that
    // there will be burst access happening and there are no pauses inbetween reading of individual words of the pattern
    Cy_SMIF_SetMode(smifBase, CY_SMIF_NORMAL);    

    // In MMIO mode there will be 16-bit word adressing, hence we need to divide the (byte) offset address by 2
    calibrationDataOffsetFromBase /= 2;

    for(tapNum = 0; tapNum < delayTapNum; tapNum++)
    {
        while(Cy_SMIF_IsBusy(smifBase));
        Cy_SMIF_Set_DelayTapSel(device, tapNum);
        Cy_SMIF_HB_MMIO_Read(smifBase,
                             slave,
                             CY_SMIF_HB_COUTINUOUS_BURST,
                             calibrationDataOffsetFromBase,                   // address
                             CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH / 2,  // size in half word
                             (uint16_t*)dataRead,
                             lc_hb,
                             false,                                           // single initial latency
                             true,                                            // blocking mode
                             context
                            );
        // Record whether the current tap read data matches the reference data
        isMatch[tapNum] = (memcmp(&dataRead, Cy_SMIF_HB_CalibrationDataPattern, CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH) == 0) ? true : false;
    }

    uint16_t bestTapNum             = 0xffff;
    uint16_t consecutiveMatchNum    = 0;
    uint16_t maxConsecutiveMatchNum = 0;

    // Determine the longest consecutive match
    for(tapNum = 0; tapNum < delayTapNum; tapNum++)
    {
        if(isMatch[tapNum] == true)
        {
            consecutiveMatchNum += 1;
        }
        else
        {            
            if(maxConsecutiveMatchNum < consecutiveMatchNum)
            {
                // Sequence of matches ended and it is a new longest sequence -> update the best tap number and prepare vars for next run
                maxConsecutiveMatchNum = consecutiveMatchNum;
                bestTapNum             = tapNum - ((maxConsecutiveMatchNum + 1) / 2);
                consecutiveMatchNum    = 0;
            }
        }
    }

    // Special case: All taps had a match -> use the center tap of the total taps number
    if(maxConsecutiveMatchNum < consecutiveMatchNum)
    {
        bestTapNum = delayTapNum - ((consecutiveMatchNum + 1) / 2);
    }

    // If a match has been found, apply the best tap
    if(bestTapNum != 0xffff)
    {
        Cy_SMIF_Set_DelayTapSel(device, bestTapNum);
    }
    else
    {
        // No match has been found
        retVal = CY_SMIF_GENERAL_ERROR;
    }

    Cy_SMIF_SetMode(smifBase, smifModeBackup);

    return retVal;
}



/*******************************************************************************
* Function Cy_SMIF_HB_GetOptimalLatencyCode
****************************************************************************//**
*
* This function determines the optimal latency code based on the given target
* frequency and connected memory type
*
* \param targetClockMHz
* Target frequency in MHz for which optimal latency code shall be retrieved
*
* \param memType
* Memory type (optimal latency code depends on connected memory type)
*
* \param latencyCode
* Pointer to where the optimal latency code will be stored (result is only valid
* if function return value indicates success)
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_HB_GetOptimalLatencyCode(uint8_t targetClockMHz, cy_smif_hb_memtype_t memType, cy_en_smif_hb_latency_code_t *latencyCode)
{
    cy_en_smif_hb_latency_code_t defaultLatency;

    const cy_smif_hb_latency_lut_t hyperFlashS26KLatencyLut[] =
    {
        {  52, CY_SMIF_HB_LC5  }, {  62, CY_SMIF_HB_LC6  }, {  72, CY_SMIF_HB_LC7  },
        {  83, CY_SMIF_HB_LC8  }, {  93, CY_SMIF_HB_LC9  }, { 104, CY_SMIF_HB_LC10 },
        { 114, CY_SMIF_HB_LC11 }, { 125, CY_SMIF_HB_LC12 }, { 135, CY_SMIF_HB_LC13 },
        { 145, CY_SMIF_HB_LC14 }, { 156, CY_SMIF_HB_LC15 }, { 166, CY_SMIF_HB_LC16 },
    };

    const cy_smif_hb_latency_lut_t hyperFlashS26HLatencyLut[] =
    {
        {  57, CY_SMIF_HB_LC5  }, {  71, CY_SMIF_HB_LC6  }, {  85, CY_SMIF_HB_LC7  },
        { 100, CY_SMIF_HB_LC8  }, { 107, CY_SMIF_HB_LC9  }, { 114, CY_SMIF_HB_LC10 },
        { 128, CY_SMIF_HB_LC11 }, { 142, CY_SMIF_HB_LC12 }, { 157, CY_SMIF_HB_LC13 },
        { 171, CY_SMIF_HB_LC14 }, { 185, CY_SMIF_HB_LC15 }, { 200, CY_SMIF_HB_LC16 },
    };

    const cy_smif_hb_latency_lut_t hyperRamS27KXXXX1LatencyLut[] =
    {
        {  83, CY_SMIF_HB_LC3  }, { 100, CY_SMIF_HB_LC4  }, { 133, CY_SMIF_HB_LC5  }, { 166, CY_SMIF_HB_LC6  },
    };

    const cy_smif_hb_latency_lut_t hyperRamS27KXXXX2LatencyLut[] =
    {
        {  85, CY_SMIF_HB_LC3  }, { 104, CY_SMIF_HB_LC4  }, { 133, CY_SMIF_HB_LC5  }, { 166, CY_SMIF_HB_LC6  },
        { 200, CY_SMIF_HB_LC7  },
    };

    const cy_smif_hb_latency_lut_t * lutPtr;
    uint8_t                          lutElements;

    switch(memType)
    {
        case CY_SMIF_HB_MEMTYPE_HYPERFLASH_S26K:
            lutPtr         = hyperFlashS26KLatencyLut;
            lutElements    = CY_GET_ARRAY_LEN(hyperFlashS26KLatencyLut);
            defaultLatency = CY_SMIF_HB_LC16;
            break;
        
        case CY_SMIF_HB_MEMTYPE_HYPERFLASH_S26H:
            lutPtr         = hyperFlashS26HLatencyLut;
            lutElements    = CY_GET_ARRAY_LEN(hyperFlashS26HLatencyLut);
            defaultLatency = CY_SMIF_HB_LC16;
            break;
        
        case CY_SMIF_HB_MEMTYPE_HYPERRAM_S27KXXXX1:
            lutPtr         = hyperRamS27KXXXX1LatencyLut;
            lutElements    = CY_GET_ARRAY_LEN(hyperRamS27KXXXX1LatencyLut);
            defaultLatency = CY_SMIF_HB_LC6;
            break;
        
        case CY_SMIF_HB_MEMTYPE_HYPERRAM_S27KXXXX2:
            lutPtr         = hyperRamS27KXXXX2LatencyLut;
            lutElements    = CY_GET_ARRAY_LEN(hyperRamS27KXXXX2LatencyLut);
            defaultLatency = CY_SMIF_HB_LC7;
            break;

        default:
            *latencyCode = CY_SMIF_HB_LC16;
            return CY_SMIF_BAD_PARAM;
    }
 
    // Go through all pairs in the llokup table and check whether the current frequency is less than the threshold frequency of that pair
    for(uint8_t index = 0; index < lutElements; index++)
    {
        if(targetClockMHz <= lutPtr[index].freqThresholdMhz)
        {
            // Setting found, return the optimal latency code for the current frequency
            *latencyCode = lutPtr[index].latencyCode;
            return CY_SMIF_SUCCESS;
        }
    }

    // Setting has not been found, return the default latency
    *latencyCode = defaultLatency;
    return CY_SMIF_BAD_PARAM;
}


/*******************************************************************************
* Function Name: Cy_SMIF_HB_CalculateTotalTimeoutCyclesForHyperRAM
****************************************************************************//**
*
* This function calculates the cycles for the TOTAL_TIMEOUT bitfield in the
* SMIF_DEVICE CTL register.
*
*
* \param maxChipSelectLowTimeNs
* Maximum chip select low time (in ns) as specified in the HyperRAM's datasheet
*
* \param clkMemFreqMhz
* Frequency of Traveo II internal clk_mem clock in MHz
*
* \param smifExtClockMhz
* External SMIF frequency in MHz, used to access the HyperRAM
*
*
* \return Total Timeout cycles
*       
*
*******************************************************************************/
uint32_t Cy_SMIF_HB_CalculateTotalTimeoutCyclesForHyperRAM(uint32_t maxChipSelectLowTimeNs, uint8_t clkMemFreqMhz, uint8_t smifExtClockMhz)
{
    // Apply formula described at TOTAL_TIMEOUT register bitfield description. (ensure that ns are rounded up)
    uint32_t timeoutTimeNs     = maxChipSelectLowTimeNs - ((((2 * 16) * 1000UL) + smifExtClockMhz - 1) / smifExtClockMhz);
    uint32_t timeoutTimeCycles = (timeoutTimeNs * clkMemFreqMhz) / 1000UL; // in cycles (rounded down)

    return timeoutTimeCycles;
}


/*******************************************************************************
* Function Cy_SMIF_HB_MMIO_Read
****************************************************************************//**
*
* This function reads data from hyper bus memory in MMIO mode.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be read. \ref cy_en_smif_slave_select_t
*
* \param burstType
* Specifies wrapped or continuous burst. \ref en_hb_bust_type_t
*
* \param readAddress
* Specifies address of external device to be read.
*
* \param sizeInHalfWord
* Specifies memory size to be read.
* Note hyper bus memory have 16bit data per each address.
*
* \param buf
* Pointer to buffer where read data to be stored
*
* \param lcCode
* Latency code. This value should be set also to the hyper bus device.
*
* \param dobleLat
* double initial latency or single initial latency
*
* \param isblockingMode
* Blocking mode or not. if this is true, process waits for the read finished in this
* function. unless, the process does not wait and exit function.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_HB_MMIO_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_hb_bust_type_t burstType,
                                        uint32_t readAddress,
                                        uint32_t sizeInHalfWord,
                                        uint16_t buf[],
                                        cy_en_smif_hb_latency_code_t lcCode,
                                        bool doubleLat,
                                        bool isblockingMode,
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t status;

    cy_un_Hyper_Com_Addr_t hbComAndAddr  = { .u64 = 0x0ULL };
    hbComAndAddr.fld.ColumnAddress    = readAddress & 0x00000007;
    hbComAndAddr.fld.ByteEnableForRAM = 0x3u;                     // 3 anyway
    hbComAndAddr.fld.PageAddress      = ((readAddress >> 3) & 0x00FFFFFF);
    hbComAndAddr.fld.BurstType        = burstType;
    hbComAndAddr.fld.Target           = CY_SMIF_HB_TARGET_MEMORY;
    hbComAndAddr.fld.ReadWrite        = CY_SMIF_HB_READ;

    uint32_t restOfHbAddr = Cy_SMIF_Reverse4ByteEndian(hbComAndAddr.u32[0]);

    /****** Command and Address ******/
    status = Cy_SMIF_TransmitCommand(base,
                                     hbComAndAddr.u16[2],     /* command value */
                                     true,
                                     CY_SMIF_WIDTH_OCTAL,
                                     CY_SMIF_DDR,
                                     (uint8_t*)&restOfHbAddr, /* Data pointer */
                                     4,                       /* Data size */
                                     CY_SMIF_WIDTH_OCTAL,     /* data width */
                                     CY_SMIF_DDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** No Mode ******/

    /****** Dummy ******/
    uint8_t dummyCycle = Cy_SMIF_HB_LatencyCodeToCycle(lcCode);

    dummyCycle = doubleLat      ?
                 dummyCycle * 2 :
                 dummyCycle;

    if(dummyCycle > 0u)
    {
        status = Cy_SMIF_SendDummyCycles(base, 
                                         CY_SMIF_WIDTH_OCTAL,  /* command width */
                                         CY_SMIF_DDR,          /* command rate */
                                         true, false, false, true,
                                         (dummyCycle - 1),     // For Hyperbus, (RD DUMMY_CTL SIZE5 + 2) = initial latency
                                         context);
        if(status != CY_SMIF_SUCCESS)
        {
            return status;
        }
    }

    if(isblockingMode == true)
    {
        status = Cy_SMIF_ReceiveDataBlocking(base, (uint8_t *)buf, (sizeInHalfWord*2), CY_SMIF_WIDTH_OCTAL, CY_SMIF_DDR, slave, context);
    }
    else
    {
        status = Cy_SMIF_ReceiveData(base, (uint8_t *)buf, (sizeInHalfWord*2), CY_SMIF_WIDTH_OCTAL, CY_SMIF_DDR, slave, NULL, context);
    }
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_HB_MMIO_Write
****************************************************************************//**
*
* This function writes data into hyper bus memory in MMIO mode.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be read. \ref cy_en_smif_slave_select_t
*
* \param burstType
* Specifies wrapped or continuous burst. \ref en_hb_bust_type_t
*
* \param readAddress
* Specifies address of external device to be read.
*
* \param sizeInHalfWord
* Specifies memory size to be read.
* Note hyper bus memory have 16bit data per each address.
*
* \param buf
* Pointer to buffer where read data to be stored
*
* \param hbDevType
* Specifies hyper bus type. FLASH or SRAM. \ref cy_en_smif_hb_dev_type_t
*
* \param lcCode
* Latency code. This value should be set also to the hyper bus device.
*
* \param isblockingMode
* Blocking mode or not. if this is true, process waits for the read finished in this
* function. unless, the process does not wait and exit function.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_HB_MMIO_Write(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_hb_bust_type_t burstType,
                                        uint32_t writeAddress,
                                        uint32_t sizeInHalfWord,
                                        uint16_t buf[],
                                        cy_en_smif_hb_dev_type_t hbDevType,
                                        cy_en_smif_hb_latency_code_t lcCode,
                                        bool isblockingMode,
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t status;

    cy_un_Hyper_Com_Addr_t hbComAndAddr  = { .u64 = 0x0ULL };
    hbComAndAddr.fld.ColumnAddress    = writeAddress & 0x00000007;
    hbComAndAddr.fld.ByteEnableForRAM = 0x3u;                     // 3 anyway
    hbComAndAddr.fld.PageAddress      = ((writeAddress >> 3) & 0x00FFFFFF);
    hbComAndAddr.fld.BurstType        = burstType;
    hbComAndAddr.fld.Target           = CY_SMIF_HB_TARGET_MEMORY;
    hbComAndAddr.fld.ReadWrite        = CY_SMIF_HB_WRITE;

    uint32_t restOfHbAddr = Cy_SMIF_Reverse4ByteEndian(hbComAndAddr.u32[0]);

    /****** Command and Address ******/
    status = Cy_SMIF_TransmitCommand(base,
                                     hbComAndAddr.u16[2],     /* command value */
                                     true,
                                     CY_SMIF_WIDTH_OCTAL,
                                     CY_SMIF_DDR,
                                     (uint8_t*)&restOfHbAddr, /* Data pointer */
                                     4,                       /* Data size */
                                     CY_SMIF_WIDTH_OCTAL,     /* data width */
                                     CY_SMIF_DDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** No Mode ******/

    /****** Dummy ******/
    if(hbDevType == CY_SMIF_HB_SRAM)
    {
        uint8_t dummyCycle = Cy_SMIF_HB_LatencyCodeToCycle(lcCode);
        if(dummyCycle > 0u)
        {
            status = Cy_SMIF_SendDummyCycles(base, 
                                             CY_SMIF_WIDTH_OCTAL,  /* command width */
                                             CY_SMIF_DDR,          /* command rate */
                                             true, true, false, false,
                                             (dummyCycle - 1),     // For Hyperbus, (RD DUMMY_CTL SIZE5 + 2)==initial latency
                                             context);
            if(status != CY_SMIF_SUCCESS)
            {
                return status;
            }
        }
    }


    if(isblockingMode == true)
    {
        Cy_SMIF_TransmitDataBlocking(base, (uint8_t *)buf, (sizeInHalfWord*2), CY_SMIF_WIDTH_OCTAL, CY_SMIF_DDR, slave, context);
    }
    else
    {
        Cy_SMIF_TransmitData(base, (uint8_t *)buf, (sizeInHalfWord*2), CY_SMIF_WIDTH_OCTAL, CY_SMIF_DDR, slave, NULL, context);
    }

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_HB_LatencyCodeToCycle
****************************************************************************//**
*
* This function converts latency code of hyper bus device to required
* dummy cycle.
*
* \param code
* Latency code of hyper device. \ref cy_en_smif_hb_latency_code_t
*
* \return
* Dummy cycle required
*
*******************************************************************************/
uint32_t Cy_SMIF_HB_LatencyCodeToCycle(cy_en_smif_hb_latency_code_t code)
{
    const uint32_t latencyToCycle[16] = {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 20, 0, 3, 4};
    return latencyToCycle[code];
}

/*******************************************************************************
* Function CY_SMIF_ReadHYPERRAM_REG
****************************************************************************//**
*
* This function reads registers in hyper SRAM device. Please call this function
* in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param reg_addr
* Address of register to be read from.
*
* \param reg_value
* Read value will be stored to memory pointed by this pointer.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t CY_SMIF_ReadHYPERRAM_REG(volatile cy_stc_smif_reg_device_t *device, uint32_t reg_addr, uint16_t* reg_value)
{
    uint8_t savedCode = device->unRD_CMD_CTL.stcField.u8CODE;
    uint32_t baseAddr = device->unADDR.u32Register;

    uint32_t savedIntrStatus = Cy_SysLib_EnterCriticalSection();

    device->unRD_CMD_CTL.stcField.u8CODE = 0xE0;
    device->unRD_CMD_CTL.u32Register;

    // Hyper bus devices have two byte for each address.
    // So, SMIF interface divides input address by 2 automatically.
    // To neutralize this effect, multiply the input address by 2 (left shift 1 bit).
    *reg_value = *(uint16_t*)(baseAddr + (reg_addr << 1ul));

    device->unRD_CMD_CTL.stcField.u8CODE = savedCode;
    device->unRD_CMD_CTL.u32Register;

    Cy_SysLib_ExitCriticalSection(savedIntrStatus);

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function CY_SMIF_WriteHYPERRAM_REG
****************************************************************************//**
*
* This function writes registers in hyper SRAM device. Please call this function
* in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param reg_addr
* Address of register to be written into.
*
* \param reg_value
* Value to be written with.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t CY_SMIF_WriteHYPERRAM_REG(volatile cy_stc_smif_reg_device_t *device, uint32_t reg_addr, uint16_t reg_value)
{
    uint8_t savedCode = device->unWR_CMD_CTL.stcField.u8CODE;
    uint32_t savedDummyPresence = device->unWR_DUMMY_CTL.stcField.u2PRESENT2;
    uint32_t baseAddr = device->unADDR.u32Register;

    uint32_t savedIntrStatus = Cy_SysLib_EnterCriticalSection();

    device->unWR_DUMMY_CTL.stcField.u2PRESENT2 = CY_SMIF_NOT_PRESENT;
    device->unWR_CMD_CTL.stcField.u8CODE = 0x60;
    device->unWR_CMD_CTL.u32Register;

    // Hyper bus devices have two byte for each address.
    // So, SMIF interface divides input address by 2 automatically.
    // To neutralize this effect, multiply the input address by 2 (left shift 1 bit).
    *(uint16_t*)(baseAddr + (reg_addr << 1ul)) = reg_value;

    device->unWR_DUMMY_CTL.stcField.u2PRESENT2 = savedDummyPresence;
    device->unWR_CMD_CTL.stcField.u8CODE = savedCode;
    device->unWR_CMD_CTL.u32Register;

    Cy_SysLib_ExitCriticalSection(savedIntrStatus);

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_InitCmd
****************************************************************************//**
*
* This function initialize command sequence by reading from base address. Please
* call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
*******************************************************************************/
void CY_SMIF_HbFlash_InitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    (void)*(volatile CY_SMIF_FLASHDATA*)base_addr;
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ResetCmd
****************************************************************************//**
*
* This function issues software reset command to the flash device.
* Please call this function in XIP mode
*
* \param base_addr
* device base address in system
*
*******************************************************************************/
void CY_SMIF_HbFlash_ResetCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Software RESET command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
    CY_SMIF_HbFlash_InitCmd(base_addr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_SectorEraseCmd
****************************************************************************//**
*
* This function writes a Sector Erase Command to Flash Device.
* This function only issues the Sector Erase Command sequence.
* Erase status polling is not implemented in this function.
* Please call this function in XIP mode
*
* \param base_addr
* device base address in system
*
* \param offset
* sector address offset to be erased
*
*******************************************************************************/
void CY_SMIF_HbFlash_SectorEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Issue unlock sequence command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_ERASE_SETUP_CMD);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* Write Sector Erase Command to Offset */
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_SECTOR_ERASE_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ChipEraseCmd
****************************************************************************//**
*
* This function writes a Chip Erase Command to Flash Device
* This function only issues the Chip Erase Command sequence.
* Erase status polling is not implemented in this function.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
*******************************************************************************/
void CY_SMIF_HbFlash_ChipEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Issue unlock sequence command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_ERASE_SETUP_CMD);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* Write Chip Erase Command to Base Address */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_CHIP_ERASE_CMD);
}
/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramCmd
****************************************************************************//**
*
* This function writes a Program Command to Flash Device
* This function only issues the Program Command sequence.
* Program status polling is not implemented in this function.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
* address offset to be written into
*
* \param pgm_data_ptr
* variable containing data to program
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_FLASHDATA* pgm_data_ptr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* Write Program Command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_PROGRAM_CMD);
    /* Write Data */
    CY_SMIF_FLASH_WR(base_addr, offset, *pgm_data_ptr);
}
/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_WriteToBufferCmd
****************************************************************************//**
*
* This function issues write to buffer command. Sending word count and
* program data should follow this function.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
* address offset to be written into
*
*******************************************************************************/
void CY_SMIF_HbFlash_WriteToBufferCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Issue unlock command sequence */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* Write Write Buffer Load Command */
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_WRITE_BUFFER_LOAD_CMD);
}
/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramBufferToFlashCmd
****************************************************************************//**
*
* This function writes "Pgm Buffer To Flash" Cmd sequence to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramBufferToFlashCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Transfer Buffer to Flash Command */
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_WRITE_BUFFER_PGM_CONFIRM_CMD);
}
/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_WriteBufferAbortResetCmd
****************************************************************************//**
*
* This function writes "Write To Buffer Abort" Reset to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_WriteBufferAbortResetCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Issue Write Buffer Abort Reset Command Sequence */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* Write to Buffer Abort Reset Command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_WRITE_BUFFER_ABORT_RESET_CMD);   
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CfiEntryCmd
****************************************************************************//**
*
* This function writes CFI Entry Command Sequence to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_CfiEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_CFI_UNLOCK_ADDR1, CY_SMIF_NOR_CFI_QUERY_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CfiExitCmd
****************************************************************************//**
*
* This functino writes Cfi Exit Command Sequence to Flash
* This function resets the device out of CFI Query mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_CfiExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Software RESET command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD); 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadCfiWord
****************************************************************************//**
*
* Read CFI word operation.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadCfiWord(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA data;

    CY_SMIF_HbFlash_CfiEntryCmd(base_addr, offset);
    data  = CY_SMIF_FLASH_RD(base_addr, offset);
    CY_SMIF_HbFlash_CfiExitCmd(base_addr);
    return(data);
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_StatusRegReadCmd
****************************************************************************//**
*
* Status register read command
* This function sends the status register read command before actually reading it.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_StatusRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_STATUS_REG_READ_CMD); 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_StatusRegClearCmd
****************************************************************************//**
*
* Status register clear command
* This function clear the status register. It will not clear the device operation
* bits such as program suspend and erase suspend bits.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_StatusRegClearCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_STATUS_REG_CLEAR_CMD); 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_AutoselectEntryCmd
****************************************************************************//**
*
* This function writes Autoselect Command Sequence to Flash
* This function issues the Autoselect Command Sequence to device.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_AutoselectEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Issue Autoselect Command Sequence */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_AUTOSELECT_CMD); 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_AutoselectExitCmd
****************************************************************************//**
*
* This function writes Autoselect Exit Command Sequence to Flash
* This function resets the device out of Autoselect mode.
* This is a "wrapper function" to provide "Enter/Exit" symmetry in
* higher software layers.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_AutoselectExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Software RESET command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramSuspendCmd
****************************************************************************//**
*
* This function writes Program Suspend Command to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Suspend Command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_PROGRAM_SUSPEND_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EraseSuspendCmd
****************************************************************************//**
*
* This fucnction writes Erase Suspend Command to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_EraseSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Suspend Command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ERASE_SUSPEND_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EraseResumeCmd
****************************************************************************//**
*
* This function writes Erase Resume Command to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_EraseResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Resume Command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ERASE_RESUME_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramResumeCmd
****************************************************************************//**
*
* This function writes Program Resume Command to Flash
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Write Resume Command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_PROGRAM_RESUME_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EraseSuspendOp
****************************************************************************//**
*
* This function performs Erase Suspend Operation.
* This function issues erase suspend commands and will poll for completion.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_EraseSuspendOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA status_reg;

    CY_SMIF_HbFlash_EraseSuspendCmd( base_addr );   /* issue erase suspend command */  
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0 );    /* wait for device done */

    if( (status_reg & CY_SMIF_DEV_ERASE_SUSP_MASK) == CY_SMIF_DEV_ERASE_SUSP_MASK )
    {
        return( CY_SMIF_DEV_ERASE_SUSPEND  );        /* Erase suspend  */
    }

    return( CY_SMIF_DEV_ERASE_SUSPEND_ERROR );       /* Erase suspend error */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramSuspendOp
****************************************************************************//**
*
* This function issues program suspend commands and will poll for completion.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramSuspendOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA status_reg;

    CY_SMIF_HbFlash_ProgramSuspendCmd( base_addr );   /* Issue program suspend command */  
    status_reg = CY_SMIF_HbFlash_Poll( base_addr, 0 );

    if( (status_reg & CY_SMIF_DEV_PROGRAM_SUSP_MASK) == CY_SMIF_DEV_PROGRAM_SUSP_MASK )
    {
        return( CY_SMIF_DEV_PROGRAM_SUSPEND );      /* Program suspend */
    }

    return( CY_SMIF_DEV_PROGRAM_SUSPEND_ERROR  );     /* Program suspend error */ 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_GetDeviceId
****************************************************************************//**
*
* Get device ID operation (for RPC2)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \return
* Three byte ID in a single int
*
*******************************************************************************/
uint32_t CY_SMIF_HbFlash_GetDeviceId(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    uint32_t id;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_AUTOSELECT_CMD);

    id  = (uint32_t)(CY_SMIF_FLASH_RD(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + 0x0001) & 0x000000FF) << 16;
    id |= (uint32_t)(CY_SMIF_FLASH_RD(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + 0x000E) & 0x000000FF) <<  8;
    id |= (uint32_t)(CY_SMIF_FLASH_RD(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + 0x000F) & 0x000000FF)      ;

    /* Write Software RESET command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);

    return(id);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_Poll
****************************************************************************//**
*
* This function polls the Flash device to determine when an embedded
* operation has finished - bit 7 is 1.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \return
* Value of status register \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_Poll(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    uint32_t polling_counter = 0xFFFFFFFF;
    volatile CY_SMIF_FLASHDATA status_reg;

    do
    {
        polling_counter--;
        CY_SMIF_HbFlash_StatusRegReadCmd( base_addr );    /* Issue status register read command */
        status_reg = CY_SMIF_FLASH_RD(base_addr, offset);       /* read the status register */
        if( (status_reg & CY_SMIF_DEV_RDY_MASK) == CY_SMIF_DEV_RDY_MASK  )  /* Are all devices done bit 7 is 1 */
        {
            break;
        }
    }while(polling_counter);

  return( status_reg );          /* retrun the status reg. */
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_StatusClear
****************************************************************************//**
*
* This function clears the flash status.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_StatusClear(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_HbFlash_StatusRegClearCmd(base_addr );
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_StatusGetReg
****************************************************************************//**
*
* This function gets the flash status register bits.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_StatusGetReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA status_reg = 0xFFFF;
    CY_SMIF_HbFlash_StatusRegReadCmd( base_addr );    /* Issue status register read command */
    status_reg = CY_SMIF_FLASH_RD( base_addr, offset );     /* read the status register */
    return status_reg;
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_SecSiSectorExitCmd
****************************************************************************//**
*
* This function issues the Secsi Sector Exit Command Sequence to device.
* Use this function to Exit the SecSi Sector.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_SecSiSectorExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Issue SecSi Sector Exit Command Sequence */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    /* First Secsi Sector Reset Command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    /* Second Secsi Sector Reset Command */
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}
/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_SecSiSectorEntryCmd
****************************************************************************//**
*
* This function issues the Secsi Sector Entry Command Sequence to device.
* Use this function to Enable the SecSi Sector.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_SecSiSectorEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Issue SecSi Sector Entry Command Sequence */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_SECSI_SECTOR_ENTRY_CMD); 
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_BlankCheckCmd
****************************************************************************//**
*
* This function checks the sector is blank.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_BlankCheckCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_BLANK_CHECK_CMD);  
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_BlankCheckOp
****************************************************************************//**
*
* Function check blank at <base_addr> + <offset>.
* Function issues all required commands and will poll for completion
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_BlankCheckOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA    status_reg;

    CY_SMIF_HbFlash_BlankCheckCmd( base_addr, offset );  

    status_reg = CY_SMIF_HbFlash_Poll(base_addr, offset );

    if( (status_reg & CY_SMIF_DEV_ERASE_MASK) == CY_SMIF_DEV_ERASE_MASK )
    {
        return( CY_SMIF_DEV_ERASE_ERROR );    /* sector not blank */
    }
    else
    {
        return( CY_SMIF_DEV_NOT_BUSY );      /* sector are blank */  
    }
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadOp
****************************************************************************//**
*
* This function performs read memory array operation
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA data;

    data = CY_SMIF_FLASH_RD(base_addr, offset);

    return(data);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_WriteBufferProgramOp
****************************************************************************//**
*
* This function performs a Write Buffer Programming Operation.
* This function programs a write-buffer overlay of addresses to data 
* passed via <data_buf>.
* This function issues all required commands and polls for completion.
* There are 4 main sections to the function:
*  - Set-up and write command sequence
*  - Determine number of locations to program and load buffer
*  - Start operation with "Program Buffer to Flash" command
*  - Poll for completion
* REQUIREMENTS:
*  Data in the buffer MUST be properly aligned with the Flash bus width.
*  No parameter checking is performed. 
*  The <word_count> variable must be properly initialized.
*  Valid <byte_cnt> values: 
*   min = 1 byte (only valid when used with a single x8 Flash)
*   max = write buffer size in bytes * number of devices in parallel
      (e.g. 32-byte buffer per device, 2 x16 devices = 64 bytes)* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
* <base_addr> + <offset> points to memory where input data will be written.
*
* \param word_count
* Number of data to be transferred.
*
* \param data_buf
* Pointer to memory from which data is transferred.
*
* \note
*
* \return
*     - \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_WriteBufferProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_WORDCOUNT word_count, CY_SMIF_FLASHDATA* data_buf)
{
    CY_SMIF_ADDRESS   last_loaded_addr;
    CY_SMIF_ADDRESS   current_offset;
    CY_SMIF_ADDRESS   end_offset;
    CY_SMIF_FLASHDATA wcount;
    CY_SMIF_FLASHDATA status_reg;

    /* Initialize variables */
    current_offset   = offset;
    end_offset       = offset + word_count - 1;
    last_loaded_addr = offset;

    /* don't try with a count of zero */
    if (!word_count) 
    {
        return(CY_SMIF_DEV_NOT_BUSY);
    }

    /* Issue Load Write Buffer Command Sequence */
    CY_SMIF_HbFlash_WriteToBufferCmd(base_addr, offset);

    /* Write # of locations to program */
    wcount = (CY_SMIF_FLASHDATA)word_count - 1;
    CY_SMIF_FLASH_WR(base_addr, offset, wcount);
  
    /* Load Data into Buffer */
    while(current_offset <= end_offset)
    {
        /* Store last loaded address & data value (for polling) */
        last_loaded_addr = current_offset;

        /* Write Data */
        CY_SMIF_FLASH_WR(base_addr, current_offset, *data_buf++);
        current_offset++;
    }

    /* Issue Program Buffer to Flash command */
    CY_SMIF_HbFlash_ProgramBufferToFlashCmd(base_addr, offset);

    status_reg = CY_SMIF_HbFlash_Poll(base_addr, last_loaded_addr);

    if( status_reg & CY_SMIF_DEV_SEC_LOCK_MASK )
    {
        return( CY_SMIF_DEV_SECTOR_LOCK );    /* sector locked */
    }

    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        return( CY_SMIF_DEV_PROGRAM_ERROR );    /* program error */
    }

    return( CY_SMIF_DEV_NOT_BUSY );         /* program complete */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramOp
****************************************************************************//**
*
* This function performs a standard Programming Operation.
* This function programs a single location to the specified data.
* This function issues all required commands and polls for completion.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
* <base_addr> + <offset> points to memory where input data will be written.
*
* \param write_data
* Actual data to be written.
*
* \note
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_FLASHDATA write_data)
{
    CY_SMIF_FLASHDATA status_reg;

    CY_SMIF_HbFlash_ProgramCmd(base_addr, offset, &write_data);

    status_reg = CY_SMIF_HbFlash_Poll(base_addr, offset);

    if( status_reg & CY_SMIF_DEV_SEC_LOCK_MASK )
    {
        return( CY_SMIF_DEV_SECTOR_LOCK );    /* sector locked */
    }

    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        return( CY_SMIF_DEV_PROGRAM_ERROR );    /* program error */
    }

    return( CY_SMIF_DEV_NOT_BUSY );         /* program complete */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_SectorEraseOp
****************************************************************************//**
*
* This function performs a Sector Erase Operation
* This function erases the sector containing <base_addr> + <offset>.
* This Function issues all required commands and polls for completion.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
* <base_addr> + <offset> points to memory which located in a sector to be erased.
*
* \note
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_SectorEraseOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA         status_reg;

    CY_SMIF_HbFlash_SectorEraseCmd(base_addr, offset);
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, offset );

    if( status_reg & CY_SMIF_DEV_SEC_LOCK_MASK )
    {
        return( CY_SMIF_DEV_SECTOR_LOCK );    /* sector locked */
    }

    if( (status_reg & CY_SMIF_DEV_ERASE_MASK) == CY_SMIF_DEV_ERASE_MASK )
    {
        return( CY_SMIF_DEV_ERASE_ERROR );    /* erase error */
    }

    return( CY_SMIF_DEV_NOT_BUSY );         /* erase complete */
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ChipEraseOp
****************************************************************************//**
*
* This function Performs a Chip Erase Operation
* This function erases entire device located at <base_addr>.
* This function issues all required commands and polls for completion.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ChipEraseOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA status_reg;

    CY_SMIF_HbFlash_ChipEraseCmd(base_addr);
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0 );

    if( (status_reg & CY_SMIF_DEV_ERASE_MASK) == CY_SMIF_DEV_ERASE_MASK )
    {
        return( CY_SMIF_DEV_ERASE_ERROR );    /* erase error */
    }

    return( CY_SMIF_DEV_NOT_BUSY );         /* erase complete */ 
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_memcpy
****************************************************************************//**
*
* This function attempts to mimic the standard memcpy.
* It segments the source data into page size chunks for use
* by Write Buffer Programming.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \param word_cnt
*
* \param data_buf
*
* \note
*
* \return
* \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_memcpy(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_WORDCOUNT word_cnt, CY_SMIF_FLASHDATA* data_buf)
{
    CY_SMIF_ADDRESS      mask   = CY_SMIF_HB_FLASH_BUFFER_SIZE - 1;
    CY_SMIF_WORDCOUNT    intwc  = word_cnt;
    CY_SMIF_HB_DEVSTATUS status = CY_SMIF_DEV_NOT_BUSY;

    if(offset & mask)
    {
        /* program only as much as necessary, so pick the lower of the two numbers */
        if(word_cnt < (CY_SMIF_HB_FLASH_BUFFER_SIZE - (offset & mask)) ) 
        {
            intwc = word_cnt;
        }
        else
        {
            intwc = CY_SMIF_HB_FLASH_BUFFER_SIZE - (offset & mask);
        }

        /* program the first few to get write buffer aligned */
        status = CY_SMIF_HbFlash_WriteBufferProgramOp(base_addr, offset, intwc, data_buf);
        if(status != CY_SMIF_DEV_NOT_BUSY) 
        {
            return(status);
        }

        offset   += intwc; /* adjust pointers and counter */
        word_cnt -= intwc;
        data_buf += intwc;
        if(word_cnt == 0)
        {
            return(status);
        }
    }

    while(word_cnt >= CY_SMIF_HB_FLASH_BUFFER_SIZE) /* while big chunks to do */
    {
        status = CY_SMIF_HbFlash_WriteBufferProgramOp(base_addr, offset, CY_SMIF_HB_FLASH_BUFFER_SIZE, data_buf);
        if(status != CY_SMIF_DEV_NOT_BUSY)
        {
            return(status);
        }

        offset   += CY_SMIF_HB_FLASH_BUFFER_SIZE; /* adjust pointers and counter */
        word_cnt -= CY_SMIF_HB_FLASH_BUFFER_SIZE;
        data_buf += CY_SMIF_HB_FLASH_BUFFER_SIZE;
    }
    if(word_cnt == 0)
    {
      return(status);
    }

    status = CY_SMIF_HbFlash_WriteBufferProgramOp(base_addr, offset, word_cnt, data_buf);
    return(status);
}

/************************************************************
*************************************************************
* Following are sector protection driver routines     *
*************************************************************
*************************************************************/


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegEntryCmd
****************************************************************************//**
*
* Lock register entry command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LockRegEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_WSXXX_LOCK_REG_ENTRY);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegBitsProgramCmd
****************************************************************************//**
*
* Lock register program command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param value
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LockRegBitsProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA value)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0x0, value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegBitsReadCmd
****************************************************************************//**
*
* Lock register read command.
* User need to issue lld_LockRegEntryCmd() before use this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \return
* DQ0 Customer SecSi Sector Protection Bit  0 = set
* DQ1 Persistent Protection Mode Lock Bit   0 = set
* DQ2 Password Protection Mode Lock Bit     0 = set
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_LockRegBitsReadCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    return(CY_SMIF_FLASH_RD(base_addr, 0x0));
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegExitCmd
****************************************************************************//**
*
* Exit lock register read/write mode command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LockRegExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PasswordProtectionEntryCmd
****************************************************************************//**
*
* This function writes Password Protection Entry command sequence
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PasswordProtectionEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_WSXXX_PSWD_PROT_CMD_ENTRY);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PasswordProtectionProgramCmd
****************************************************************************//**
*
* This function writes Password Protection Program command.
* User needs to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \param pwd
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PasswordProtectionProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_FLASHDATA pwd)
{
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, offset, pwd);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PasswordProtectionReadCmd
****************************************************************************//**
*
* Issue read password command
* User needs to issue lld_PasswordProtectionEntryCmd() before issue this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param pwd0
*
* \param pwd1
*
* \param pwd2
*
* \param pwd3
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PasswordProtectionReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA* pwd0, CY_SMIF_FLASHDATA* pwd1, CY_SMIF_FLASHDATA* pwd2, CY_SMIF_FLASHDATA* pwd3)
{
    *pwd0 = CY_SMIF_FLASH_RD(base_addr, 0);
    *pwd1 = CY_SMIF_FLASH_RD(base_addr, 1);
    *pwd2 = CY_SMIF_FLASH_RD(base_addr, 2);
    *pwd3 = CY_SMIF_FLASH_RD(base_addr, 3);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PasswordProtectionUnlockCmd
****************************************************************************//**
*
* Issue unlock password command.
* User need sto issue lld_PasswordProtectionEntryCmd() before issue this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param pwd0
*
* \param pwd1
*
* \param pwd2
*
* \param pwd3
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PasswordProtectionUnlockCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA pwd0, CY_SMIF_FLASHDATA pwd1, CY_SMIF_FLASHDATA pwd2, CY_SMIF_FLASHDATA pwd3)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_WSXXX_PSWD_UNLOCK_1);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_WSXXX_PSWD_UNLOCK_2);
    CY_SMIF_FLASH_WR(base_addr, 0, pwd0);
    CY_SMIF_FLASH_WR(base_addr, 1, pwd1);
    CY_SMIF_FLASH_WR(base_addr, 2, pwd2);
    CY_SMIF_FLASH_WR(base_addr, 3, pwd3);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_WSXXX_PSWD_UNLOCK_3);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PasswordProtectionExitCmd
****************************************************************************//**
*
* This function issues exit password protection mode command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PasswordProtectionExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbEntryCmd
****************************************************************************//**
*
* Non-Volatile Sector Protection Entry Command.
* Ppb entry command will disable the reads and writes for the bank selected.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1+(offset & CY_SMIF_SA_OFFSET_MASK), CY_SMIF_WSXXX_PPB_ENTRY);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbProgramCmd
****************************************************************************//**
*
* Program Non-Volatile Sector Protection Command.
* User needs to issue lld_PpbEntryCmd() before issue this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, offset, 0);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbAllEraseCmd
****************************************************************************//**
*
* Erase Non-Volatile Protection for All Sectors Command.
* User needs to issue lld_PpbEntryCmd() before issue this routine.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbAllEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ERASE_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_WSXXX_PPB_ERASE_CONFIRM);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbStatusReadCmd
****************************************************************************//**
*
* Read Non-Volatile Sector Status Command.
* User needs to issue lld_PpbEntryCmd() before issue this routine.
* Sector status 0 is locked and 1 is unlocked.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_PpbStatusReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    return(CY_SMIF_FLASH_RD(base_addr, offset));
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbExitCmd
****************************************************************************//**
*
* Exit the Non-Volatile Sector Status mode.
* After the exit command the device goes back to memory array mode.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitEntryCmd
****************************************************************************//**
*
* Issue Persistent Protection Bit Lock Bit Entry Command.
* The Ppb Lock Bit is a global bit for all sectors. 
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbLockBitEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_WSXXX_PPB_LOCK_ENTRY);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitSetCmd
****************************************************************************//**
*
* Issue set Persistent Protection Bit Lock Bit command.
* Once bit is set there is no command to clear it only hardware reset and power up 
* will clear the bit.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbLockBitSetCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, 0);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitReadCmd
****************************************************************************//**
*
* Read the Ppb Lock Bit value.
* User needs to issue lld_PpbLockBitEntryCmd() before read.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_PpbLockBitReadCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    return(CY_SMIF_FLASH_RD(base_addr, 0));
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitExitCmd
****************************************************************************//**
*
* Exit Ppb Lock Bit mode command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_PpbLockBitExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_DybEntryCmd
****************************************************************************//**
*
* Dynamic (Volatile) Sector Protection Entry Command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_DybEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_WSXXX_DYB_ENTRY);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_DybSetCmd
****************************************************************************//**
*
* Dynamic (Volatile) Sector Protection Set Command.
* User needs to issue lld_DybEntryCmd() before issue this command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_DybSetCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, offset, 0x00000000);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_DybClrCmd
****************************************************************************//**
*
* Dynamic (Volatile) Sector Protection Clear Command.
* User needs to issue lld_DybEntryCmd() before issue this command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_DybClrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD);
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_WSXXX_DYB_CLEAR);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_DybReadCmd
****************************************************************************//**
*
* Dynamic (Volatile) Sector Protection Read Command.
* User needs to issue lld_DybEntryCmd() before issue this command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_DybReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    return(CY_SMIF_FLASH_RD(base_addr, offset));
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_DybExitCmd
****************************************************************************//**
*
* Exit Dynamic (Volatile) Sector Protection Mode Command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_DybExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitReadOp
****************************************************************************//**
*
* Operation to read global lock bit.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_PpbLockBitReadOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA retval;

    (CY_SMIF_HbFlash_PpbLockBitEntryCmd)(base_addr);
    retval = (CY_SMIF_HbFlash_PpbLockBitReadCmd)(base_addr);
    CY_SMIF_HbFlash_PpbLockBitExitCmd(base_addr);

    return retval;
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbLockBitSetOp
****************************************************************************//**
*
* Operation to set the global lock bit.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* 0 Successful
* -1 Failed
*
*******************************************************************************/
int CY_SMIF_HbFlash_PpbLockBitSetOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_HB_DEVSTATUS dev_status = CY_SMIF_DEV_STATUS_UNKNOWN;
    CY_SMIF_FLASHDATA    status_reg;


    CY_SMIF_HbFlash_PpbLockBitEntryCmd(base_addr);
    CY_SMIF_HbFlash_PpbLockBitSetCmd(base_addr);
    /* poll for completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        dev_status =  CY_SMIF_DEV_PROGRAM_ERROR;    /* program error */
    }
    else
    {
        dev_status = CY_SMIF_DEV_NOT_BUSY;
    }

    /* if not done, then we have an error */
    if (dev_status != CY_SMIF_DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
        CY_SMIF_HbFlash_PpbLockBitExitCmd(base_addr);
        return (-1);  /* return error */ 
    }

    CY_SMIF_HbFlash_PpbLockBitExitCmd(base_addr);
    return 0; /* successful */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbAllEraseOp
****************************************************************************//**
*
* Operation to clear protection for all sections.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* 0 Successful
* -1 Failed
*
*******************************************************************************/
int CY_SMIF_HbFlash_PpbAllEraseOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_HB_DEVSTATUS    dev_status = CY_SMIF_DEV_STATUS_UNKNOWN;
    CY_SMIF_FLASHDATA    status_reg;

    if (CY_SMIF_HbFlash_PpbLockBitReadOp(base_addr) == CY_SMIF_PPB_PROTECTED)    /* if it is already locked */
    {
        return(-1);                            /* return an error cuz Lock Bit is locked */
    }

    CY_SMIF_HbFlash_PpbEntryCmd(base_addr, 0);
    CY_SMIF_HbFlash_PpbAllEraseCmd(base_addr);
    /* poll for completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        dev_status =  CY_SMIF_DEV_PROGRAM_ERROR;    /* program error */
    }
    else
    {
        dev_status = CY_SMIF_DEV_NOT_BUSY;
    }

    /* if not done, then we have an error */
    if (dev_status != CY_SMIF_DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
        CY_SMIF_HbFlash_PpbExitCmd(base_addr);
        return (-1); /* return error */
    }

    CY_SMIF_HbFlash_PpbExitCmd(base_addr);    /* exit Ppb protection mode command */
    return 0; /* successful */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbProgramOp
****************************************************************************//**
*
* Operation set the Persistent Protection for a sector. 
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* 0 Successful
* -1 Failed
*
*******************************************************************************/
int CY_SMIF_HbFlash_PpbProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_HB_DEVSTATUS dev_status = CY_SMIF_DEV_STATUS_UNKNOWN;
    CY_SMIF_FLASHDATA    status_reg;

    if(CY_SMIF_HbFlash_PpbLockBitReadOp(base_addr) == CY_SMIF_PPB_PROTECTED)      /* if it is already locked */
    {
        return (-1);                              /* return an error cuz Lock Bit is locked */
    }

    CY_SMIF_HbFlash_PpbEntryCmd(base_addr, offset);
    CY_SMIF_HbFlash_PpbProgramCmd(base_addr, offset);

    /* poll for completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if((status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK)
    {
        dev_status =  CY_SMIF_DEV_PROGRAM_ERROR;    /* program error */
    }
    else
    {
        dev_status = CY_SMIF_DEV_NOT_BUSY;
    }

    /* if not done, then we have an error */
    if(dev_status != CY_SMIF_DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
        CY_SMIF_HbFlash_PpbExitCmd(base_addr);
        return (-1); /* return error */
    }

    CY_SMIF_HbFlash_PpbExitCmd(base_addr);
    return 0; /* successful */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_PpbStatusReadOp
****************************************************************************//**
*
* Operation to read the Persistent Protection status register.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_PpbStatusReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA  status;

    CY_SMIF_HbFlash_PpbEntryCmd(base_addr, offset);
    status = (CY_SMIF_FLASH_RD(base_addr, offset));
    CY_SMIF_HbFlash_PpbExitCmd(base_addr);

    return status;
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegBitsReadOp
****************************************************************************//**
*
* Operation to read the lock status register.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_LockRegBitsReadOp(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_HbFlash_LockRegEntryCmd(base_addr);
    value = CY_SMIF_HbFlash_LockRegBitsReadCmd(base_addr);
    CY_SMIF_HbFlash_LockRegExitCmd(base_addr);

    return(value);
}


/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LockRegBitsProgramOp
****************************************************************************//**
*
* Operation to program the lock register.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param value
*
* \note
*
* \return
* 0 Successful
* -1 Failed
*
*******************************************************************************/
int CY_SMIF_HbFlash_LockRegBitsProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA value)
{
    CY_SMIF_HB_DEVSTATUS dev_status = CY_SMIF_DEV_STATUS_UNKNOWN;
    CY_SMIF_FLASHDATA    status_reg;

    CY_SMIF_HbFlash_LockRegEntryCmd(base_addr);
    CY_SMIF_HbFlash_LockRegBitsProgramCmd(base_addr, value);

    /* poll for completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if((status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK)
    {
        dev_status =  CY_SMIF_DEV_PROGRAM_ERROR;    /* program error */
    }
    else
    {
        dev_status = CY_SMIF_DEV_NOT_BUSY;
    }

    /* if not done, then we have an error */
    if(dev_status != CY_SMIF_DEV_NOT_BUSY)
    {
        /* Write Software RESET command */
        CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_RESET_CMD);
        CY_SMIF_HbFlash_LockRegExitCmd(base_addr);
        return (-1); /* return error */
    }

    CY_SMIF_HbFlash_LockRegExitCmd(base_addr);
    return 0; /* successful */
}


/**************************************************************************
* Special API for RPC2 device (KS-S/KL-S)
**************************************************************************/

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EnterDeepPowerDown
****************************************************************************//**
*
* Enter Deep Power Down Command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_EnterDeepPowerDown(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Issue Enter Deep Power Down command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ENTER_DEEP_POWER_DOWN_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_MeasureTemperature
****************************************************************************//**
*
* Measure Temperature Command.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_MeasureTemperature(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_MEASURE_TEMPERATURE_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadTemperatureReg
****************************************************************************//**
*
* Read Temperature Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadTemperatureReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_TEMPERATURE_REG_CMD);  
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramPORTimerReg
****************************************************************************//**
*
* Program Power On Reset Timer Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param portime
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramPORTimerReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA portime)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_PROGRAM_POR_TIMER_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, portime);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadPORTimerReg
****************************************************************************//**
*
* Read Power On Reset Timer Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadPORTimerReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_POR_TIMER_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadInterruptConfigReg
****************************************************************************//**
*
* Load Interrupt Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param icr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadInterruptConfigReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA icr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_LOAD_INTERRUPT_CFG_REG_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, icr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadInterruptConfigReg
****************************************************************************//**
*
* Read Interrupt Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadInterruptConfigReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_INTERRUPT_CFG_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadInterruptStatusReg
****************************************************************************//**
*
* Load Interrupt Status Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param isr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadInterruptStatusReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA isr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_LOAD_INTERRUPT_STATUS_REG_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, isr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadInterruptStatusReg
****************************************************************************//**
*
* Read Interrupt Status Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadInterruptStatusReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_INTERRUPT_STATUS_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadVolatileConfigReg
****************************************************************************//**
*
* Load Volatile Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param vcr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_LOAD_VOLATILE_CFG_REG_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, vcr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadVolatileConfigReg2
****************************************************************************//**
*
* Load Volatile Configuration Register 2
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param vcr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_LOAD_VOLATILE_CFG_REG2_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, vcr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadVolatileConfigReg
****************************************************************************//**
*
* Read Volatile Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_VOLATILE_CFG_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadVolatileConfigReg2
****************************************************************************//**
*
* Read Volatile Configuration Register 2
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_VOLATILE_CFG_REG2_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramVolatileConfigRegOp
****************************************************************************//**
*
* Program Volatile Configuration Register Operation (with poll)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param vcr
*
* \note
*
* \return
*     - \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramVolatileConfigRegOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr)
{
    CY_SMIF_FLASHDATA status_reg;

    /* Load VCR */
    CY_SMIF_HbFlash_LoadVolatileConfigReg(base_addr, vcr);

    /* Poll for Program completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);

    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        return(CY_SMIF_DEV_PROGRAM_ERROR); /* program error */
    }

    return(CY_SMIF_DEV_NOT_BUSY);          /* program complete */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramVolatileConfigRegOp2
****************************************************************************//**
*
* Program Volatile Configuration Register 2 Operation (with poll)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param vcr
*
* \note
*
* \return
*     - \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramVolatileConfigRegOp2(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr)
{
    CY_SMIF_FLASHDATA status_reg;

    /* Load VCR */
    CY_SMIF_HbFlash_LoadVolatileConfigReg2(base_addr, vcr);

    /* Poll for Program completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);

    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        return( CY_SMIF_DEV_PROGRAM_ERROR); /* program error */
    }

    return(CY_SMIF_DEV_NOT_BUSY);           /* program complete */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramNonVolatileConfigReg
****************************************************************************//**
*
* Program Non-Volatile Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param nvcr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA nvcr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_PROGRAM_NON_VOLATILE_CFG_REG_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, nvcr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramNonVolatileConfigReg2
****************************************************************************//**
*
* Program Non-Volatile Configuration Register 2
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param nvcr
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ProgramNonVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr,  CY_SMIF_FLASHDATA nvcr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_PROGRAM_NON_VOLATILE_CFG_REG2_CMD);
    CY_SMIF_FLASH_WR(base_addr, 0, nvcr);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EraseNonVolatileConfigReg
****************************************************************************//**
*
* Erase Non-Volatile Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_EraseNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_ERASE_NON_VOLATILE_CFG_REG_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadNonVolatileConfigReg
****************************************************************************//**
*
* Read Non-Volatile Configuration Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_NON_VOLATILE_CFG_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadNonVolatileConfigReg2
****************************************************************************//**
*
* Read Non-Volatile Configuration Register 2
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
*     - \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadNonVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;

    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_READ_NON_VOLATILE_CFG_REG2_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0));

    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ProgramNonVolatileConfigRegOp
****************************************************************************//**
*
* Program Non-Volatile Configuration Register Operation (with poll)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param nvcr
*
* \note
*
* \return
*     - \ref CY_SMIF_HB_DEVSTATUS
*
*******************************************************************************/
CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramNonVolatileConfigRegOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA nvcr)
{
    CY_SMIF_FLASHDATA status_reg;

    /* Erase NVCR */
    CY_SMIF_HbFlash_EraseNonVolatileConfigReg(base_addr);

    /* Poll for erase completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if( (status_reg & CY_SMIF_DEV_ERASE_MASK) == CY_SMIF_DEV_ERASE_MASK )
    {
        return(CY_SMIF_DEV_ERASE_ERROR);   /* erase  error */
    }

    /* Program NVCR */
    CY_SMIF_HbFlash_ProgramNonVolatileConfigReg(base_addr, nvcr);

    /* Poll for program completion */
    status_reg = CY_SMIF_HbFlash_Poll(base_addr, 0);
    if( (status_reg & CY_SMIF_DEV_PROGRAM_MASK) == CY_SMIF_DEV_PROGRAM_MASK )
    {
        return(CY_SMIF_DEV_PROGRAM_ERROR);    /* program error */
    }

    return(CY_SMIF_DEV_NOT_BUSY);             /* program complete */
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EvaluateEraseStatus
****************************************************************************//**
*
* Evaluate Erase Status
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_EvaluateEraseStatus(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    /* Write Evaluate Erase Status Command to Offset */
    CY_SMIF_FLASH_WR(base_addr, (offset & CY_SMIF_SA_OFFSET_MASK) + CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_EVALUATE_ERASE_STATUS_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCStatusEnterCmd
****************************************************************************//**
*
* ECC Status Enter
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
*******************************************************************************/
void CY_SMIF_HbFlash_ECCStatusEnterCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_ECC_STATUS_ENTER_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCStatusReadCmd
****************************************************************************//**
*
* ECC Status Read
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ECCStatusReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA value;
    value = (CY_SMIF_FLASH_RD(base_addr, offset));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCErrorLowerAddrRegReadCmd
****************************************************************************//**
*
* Read Error Lower Address Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_FLASHDATA
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ECCErrorLowerAddrRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ECC_READ_ERROR_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0x01));
    return(value);  
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCErrorUpperAddrRegReadCmd
****************************************************************************//**
*
* Read Error Upper Address Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ECCErrorUpperAddrRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ECC_READ_ERROR_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0x02));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCReadErrorDetectionCounterCmd
****************************************************************************//**
*
* Read Error Detection Counter
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ECCReadErrorDetectionCounterCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ECC_READ_ERROR_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0x03));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCClearErrorDetectionCounterCmd
****************************************************************************//**
*
* Clear Error Detection Counter
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_ECCClearErrorDetectionCounterCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ECC_CLEAR_ERROR_DETECTION_COUNTER_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCExitCmd
****************************************************************************//**
*
* ECC Exit
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_ECCExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_ECC_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ECCStatusReadOp
****************************************************************************//**
*
* ECC Status Read Operation
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ECCStatusReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_HbFlash_ECCStatusEnterCmd(base_addr);
    value = (CY_SMIF_FLASH_RD(base_addr, offset));
    CY_SMIF_HbFlash_ECCExitCmd(base_addr);
    return (value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CRCEnterCmd
****************************************************************************//**
*
* CRC Enter Command
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_CRCEnterCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_CRC_ENTRY_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadCRCStartAddrCmd
****************************************************************************//**
*
* Load CRC Start Address
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
*     - \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadCRCStartAddrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_LOAD_CRC_START_ADDR_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_LoadCRCEndAddrCmd
****************************************************************************//**
*
* Load CRC End Address (start calculation)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_LoadCRCEndAddrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASH_WR(base_addr, offset, CY_SMIF_NOR_LOAD_CRC_END_ADDR_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CRCSuspendCmd
****************************************************************************//**
*
* CRC Suspend
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_CRCSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_CRC_SUSPEND_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CRCArrayReadCmd
****************************************************************************//**
*
* Array Read (during suspend)
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \param offset
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_CRCArrayReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset)
{
    CY_SMIF_FLASHDATA value;
    value = (CY_SMIF_FLASH_RD(base_addr, offset));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CRCResumeCmd
****************************************************************************//**
*
* CRC Resume
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_CRCResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_CRC_RESUME_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadCheckvalueLowResultRegCmd
****************************************************************************//**
*
* Read Checkvalue Low Result Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadCheckvalueLowResultRegCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_CRC_READ_CHECKVALUE_RESLUT_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0x00));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_ReadCheckvalueHighResultRegCmd
****************************************************************************//**
*
* Read Checkvalue High Result Register
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
CY_SMIF_FLASHDATA CY_SMIF_HbFlash_ReadCheckvalueHighResultRegCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASHDATA value;
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_CRC_READ_CHECKVALUE_RESLUT_REG_CMD);
    value = (CY_SMIF_FLASH_RD(base_addr, 0x01));
    return(value);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_CRCExitCmd
****************************************************************************//**
*
* CRC Exit Command
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note
*
* \return
* \ref CY_SMIF_HYPERBUSI_STATUS
*
*******************************************************************************/
void CY_SMIF_HbFlash_CRCExitCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    CY_SMIF_FLASH_WR(base_addr, 0, CY_SMIF_NOR_CRC_EXIT_CMD);
}

/*******************************************************************************
* Function Name: CY_SMIF_HbFlash_EnterSPIModeCmd
****************************************************************************//**
*
* For Semper flash device.
* Please call this function in XIP mode.
*
* \param base_addr
* device base address in system
*
* \note This is only for S26H series
*
*******************************************************************************/
void CY_SMIF_HbFlash_EnterSPIModeCmd(volatile CY_SMIF_FLASHDATA* base_addr)
{
    /* Issue unlock sequence command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_NOR_UNLOCK_DATA1);
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR2, CY_SMIF_NOR_UNLOCK_DATA2);
    /* Issue enter SPI mode command */
    CY_SMIF_FLASH_WR(base_addr, CY_SMIF_HB_FLASH_UNLOCK_ADDR1, CY_SMIF_ENTER_SPI_MODE_CMD);
}

#if defined(__cplusplus)
}
#endif
