/***************************************************************************//**
* \file cy_smif_device_common.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provide common definition for specific external flash drivers.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

#include "cy_smif_device_common.h"

static uint8_t CY_SMIF_GetSlaveNumber(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave);
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrByteSizeFromMemSize(uint32_t size);
static uint16_t CY_SMIF_GetReadCmdInfoNum(uint16_t rdCmdId);
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrLenFromReadCmd(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, const cy_stc_ex_dev_context_t* context);
static cy_en_smif_field_presence_t CY_SMIF_DummyCycleEachReadCmd(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, uint8_t latencyCode, uint8_t* dummyCycle, const cy_stc_ex_dev_context_t* context);
static void CY_SMIF_SetXIPReadSequence(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, uint8_t mode, uint8_t latencyCode, bool dualQuad, const cy_stc_ex_dev_context_t* context);
static uint16_t CY_SMIF_GetWriteCmdInfoNum(uint16_t wtCmdId);
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrLenFromWriteCmd(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, const cy_stc_ex_dev_context_t* context);
static cy_en_smif_field_presence_t CY_SMIF_DummyCycleEachWriteCmd(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, uint8_t latencyCode, uint8_t* dummyCycle, bool* rwdsEn, const cy_stc_ex_dev_context_t* context);
static void CY_SMIF_SetXIPWriteSequence(volatile cy_stc_smif_reg_device_t *device, uint16_t cmd, uint8_t mode, uint8_t latencyCode, bool dualQuad, const cy_stc_ex_dev_context_t* context);
static cy_en_smif_status_t Cy_SMIF_TX_CmdAddrDum(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave, cy_en_smif_trx_type_t en_trx, cy_un_smif_trx_cmd_t cmd, uint8_t constDummyNum, 
                                                 uint8_t* dummyTable, cy_en_smif_trx_addr_len_t inputAddrLen, uint32_t inputAddr, uint8_t lc, cy_stc_smif_context_t *context);


/*******************************************************************************
* Function Cy_SMIF_InitExMemContext
****************************************************************************//**
*
* This function initializes context of the device \ref cy_stc_ex_dev_context_t
* according to input external device type. The context contains read/write
* sequence for each read/command command of the device.
* The sequence itself is contained in the according c files.
*
* \param exDevice
* The external device. the command sequence of this device will be input to the 
* context. supported devices are S25FL/S and Semper. \ref cy_en_smif_ex_device_type
* \note This function is not supporting Hyper Bus memories.
*
* \param context
* This will contain external device command sequence information and will be used
* to setup XIP mode and also read/write in MMIO mode.
*
* \return (see \cy_en_smif_xip_addr_byte_t)
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_InitExMemContext(cy_en_smif_ex_device_type exDevice, cy_stc_ex_dev_context_t* context)
{

    switch(exDevice)
    {
    case CY_SMIF_S25FL:
        context->p_readCmdTypes                  = (cy_stc_cmd_sq_t*)g_s25flReadCmdTypes;
        context->p_readAddrTypes                 = (cy_stc_addr_sq_t*)g_s25flReadAddrTypes;
        context->p_readModeTypes                 = (cy_stc_mode_sq_t*)g_s25flReadModeTypes;
        context->p_readDummyTypes                = (cy_stc_dummy_sq_t*)g_s25flReadDummyTypes;
        context->pp_readLatencyCodeToDummyCycle  = (uint8_t**)g_s25flReadLatencyCodeToDummyCycle;
        context->p_readDataTypes                 = (cy_stc_data_sq_t*)g_s25flReadDataTypes;
        context->p_readCmdInfo                   = (cy_stc_read_info_t*)g_s25flReadCmdInfo;
        context->readLatencyCodeMax              = CY_SMIF_S25FL_READ_LATENCY_CODE_MAX;
        context->readCommandNum                  = CY_SMIF_S25FL_READ_ID_NUM;

        context->p_writeCmdTypes                 = (cy_stc_cmd_sq_t*)g_s25flWriteCmdTypes;
        context->p_writeAddrTypes                = (cy_stc_addr_sq_t*)g_s25flWriteAddrTypes;
        context->p_writeModeTypes                = (cy_stc_mode_sq_t*)g_s25flWriteModeTypes;
        context->p_writeDummyTypes               = (cy_stc_dummy_sq_t*)g_s25flWriteDummyTypes;
        context->pp_writeLatencyCodeToDummyCycle = (uint8_t**)g_s25flWriteLatencyCodeToDummyCycle;
        context->p_writeDataTypes                = (cy_stc_data_sq_t*)g_s25flWriteDataTypes;
        context->p_writeCmdInfo                  = (cy_stc_write_info_t*)g_s25flWriteCmdInfo;
        context->writeLatencyCodeMax             = CY_SMIF_S25FL_WRITE_LATENCY_CODE_MAX;
        context->writeCommandNum                 = CY_SMIF_S25FL_WRITE_ID_NUM;

        context->currentExDevice                 = exDevice;
        break;
    case CY_SMIF_SEMP:
        context->p_readCmdTypes                  = (cy_stc_cmd_sq_t*)g_sempReadCmdTypes;
        context->p_readAddrTypes                 = (cy_stc_addr_sq_t*)g_sempReadAddrTypes;
        context->p_readModeTypes                 = (cy_stc_mode_sq_t*)g_sempReadModeTypes;
        context->p_readDummyTypes                = (cy_stc_dummy_sq_t*)g_sempReadDummyTypes;
        context->pp_readLatencyCodeToDummyCycle  = (uint8_t**)g_sempReadLatencyCodeToDummyCycle;
        context->p_readDataTypes                 = (cy_stc_data_sq_t*)g_sempReadDataTypes;
        context->p_readCmdInfo                   = (cy_stc_read_info_t*)g_sempReadCmdInfo;
        context->readLatencyCodeMax              = CY_SMIF_SEMP_READ_LATENCY_CODE_MAX;
        context->readCommandNum                  = CY_SMIF_SEMP_READ_ID_NUM;

        context->p_writeCmdTypes                 = (cy_stc_cmd_sq_t*)g_sempWriteCmdTypes;
        context->p_writeAddrTypes                = (cy_stc_addr_sq_t*)g_sempWriteAddrTypes;
        context->p_writeModeTypes                = (cy_stc_mode_sq_t*)g_sempWriteModeTypes;
        context->p_writeDummyTypes               = (cy_stc_dummy_sq_t*)g_sempWriteDummyTypes;
        context->pp_writeLatencyCodeToDummyCycle = (uint8_t**)g_sempWriteLatencyCodeToDummyCycle;
        context->p_writeDataTypes                = (cy_stc_data_sq_t*)g_sempWriteDataTypes;
        context->p_writeCmdInfo                  = (cy_stc_write_info_t*)g_sempWriteCmdInfo;
        context->writeLatencyCodeMax             = CY_SMIF_SEMP_WRITE_LATENCY_CODE_MAX;
        context->writeCommandNum                 = CY_SMIF_SEMP_WRITE_ID_NUM;

        context->currentExDevice                 = exDevice;
        break;
    case CY_SMIF_S25FS:
        context->p_readCmdTypes                  = (cy_stc_cmd_sq_t*)g_s25fsReadCmdTypes;
        context->p_readAddrTypes                 = (cy_stc_addr_sq_t*)g_s25fsReadAddrTypes;
        context->p_readModeTypes                 = (cy_stc_mode_sq_t*)g_s25fsReadModeTypes;
        context->p_readDummyTypes                = (cy_stc_dummy_sq_t*)g_s25fsReadDummyTypes;
        context->pp_readLatencyCodeToDummyCycle  = (uint8_t**)g_s25fsReadLatencyCodeToDummyCycle;
        context->p_readDataTypes                 = (cy_stc_data_sq_t*)g_s25fsReadDataTypes;
        context->p_readCmdInfo                   = (cy_stc_read_info_t*)g_s25fsReadCmdInfo;
        context->readLatencyCodeMax              = CY_SMIF_S25FS_READ_LATENCY_CODE_MAX;
        context->readCommandNum                  = CY_SMIF_S25FS_READ_ID_NUM;

        context->p_writeCmdTypes                 = (cy_stc_cmd_sq_t*)g_s25fsWriteCmdTypes;
        context->p_writeAddrTypes                = (cy_stc_addr_sq_t*)g_s25fsWriteAddrTypes;
        context->p_writeModeTypes                = (cy_stc_mode_sq_t*)g_s25fsWriteModeTypes;
        context->p_writeDummyTypes               = (cy_stc_dummy_sq_t*)g_s25fsWriteDummyTypes;
        context->pp_writeLatencyCodeToDummyCycle = (uint8_t**)g_s25fsWriteLatencyCodeToDummyCycle;
        context->p_writeDataTypes                = (cy_stc_data_sq_t*)g_s25fsWriteDataTypes;
        context->p_writeCmdInfo                  = (cy_stc_write_info_t*)g_s25fsWriteCmdInfo;
        context->writeLatencyCodeMax             = CY_SMIF_S25FS_WRITE_LATENCY_CODE_MAX;
        context->writeCommandNum                 = CY_SMIF_S25FS_WRITE_ID_NUM;

        context->currentExDevice                 = exDevice;
        break;

    default:
        return CY_SMIF_BAD_PARAM;
    }

    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_InitXIPModeForExMem
****************************************************************************//**
*
* This function sets up SMIF registers which is used in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param bitAllocation
* Specifies which port internal SMIF signals will be connected and also which 
* data width (Octal, Quad, Dual and so on). \ref cy_en_smif_bit_alloc_t
*
* \param config
* Configuration to be applied to the SMIF device \ref cy_stc_device_config_t
*
* \param context
* Contains external device information like read/write sequence for each command
*
* \return
*     - \ref CY_SMIF_BAD_PARAM
*     - \ref CY_SMIF_SUCCESS
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_InitXIPModeForExMem(volatile cy_stc_smif_reg_device_t *device,
                                                cy_en_smif_bit_alloc_t bitAllocation,
                                                const cy_stc_device_config_t *config,
                                                const cy_stc_ex_dev_context_t* context)
{
    uint32_t startAddress  = config->addr;
    uint32_t maxDeviceSize = (~(CY_SMIF_GetXIP_Mask(device))) + 1UL;
    uint32_t size          = Cy_SMIF_MaskValueToSizeInByte(config->size);
    uint32_t endAddress    = config->addr + size - 1;
    cy_un_smif_bit_alloc_t unBitAllocation   = { .en = bitAllocation };
    cy_en_smif_xip_addr_byte_t addrSize;

    /********************/
    /* Check parameters */
    /********************/
    if(context == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if((context->p_readCmdInfo == NULL)                  ||
       (context->p_readCmdTypes == NULL)                 ||
       (context->p_readAddrTypes == NULL)                ||
       (context->p_readModeTypes == NULL)                ||
       (context->p_readDummyTypes == NULL)               ||
       (context->pp_readLatencyCodeToDummyCycle == NULL)  ||
       (context->p_readDataTypes == NULL)                ||
       (context->p_readCmdInfo == NULL)                  ||
       (context->p_writeCmdInfo == NULL)                 ||
       (context->p_writeCmdTypes == NULL)                ||
       (context->p_writeAddrTypes == NULL)               ||
       (context->p_writeModeTypes == NULL)               ||
       (context->p_writeDummyTypes == NULL)              ||
       (context->pp_writeLatencyCodeToDummyCycle == NULL) ||
       (context->p_writeDataTypes == NULL)               ||
       (context->p_writeCmdInfo == NULL))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(startAddress < CY_SMIF_GetXIP_Address(device))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(endAddress > ((uint32_t)CY_SMIF_GetXIP_Address(device) + maxDeviceSize - 1UL))
    {
        return CY_SMIF_BAD_PARAM;
    }

    {
        cy_en_smif_xip_addr_byte_t readCmdAddrByte  = CY_SMIF_GetAddrLenFromReadCmd(device, config->xipReadCmdId, context);
        cy_en_smif_xip_addr_byte_t writeCmdAddrByte = CY_SMIF_GetAddrLenFromWriteCmd(device, config->xipWriteCmdId, context);
        cy_en_smif_xip_addr_byte_t requiredAddrByte = CY_SMIF_GetAddrByteSizeFromMemSize(size);
        if(readCmdAddrByte < requiredAddrByte)
        {
            return CY_SMIF_BAD_PARAM;
        }

        if(config->writeEn == true)
        {
            if(writeCmdAddrByte < requiredAddrByte)
            {
                return CY_SMIF_BAD_PARAM;
            }

            if(readCmdAddrByte != writeCmdAddrByte)
            {
                return CY_SMIF_BAD_PARAM;
            }
        }

        addrSize = readCmdAddrByte;
    }

    /*********************/
    /*   Set registers   */
    /*********************/
    Cy_SMIF_DeviceSetDataSelect(device, (cy_en_smif_data_select_t)unBitAllocation.bits.dataSel);
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

    Cy_SMIF_SetXIPAddrCtl(device, addrSize, (bool)unBitAllocation.bits.IsDualQuad);

    CY_SMIF_SetXIPReadSequence(device, config->xipReadCmdId, config->xipReadMode, config->rdLatencyCode, (bool)unBitAllocation.bits.IsDualQuad, context);

    if(config->writeEn == true)
    {
        Cy_SMIF_DeviceWriteEnable(device);
        CY_SMIF_SetXIPWriteSequence(device, config->xipWriteCmdId, config->xipWriteMode, config->wtLatencyCode, (bool)unBitAllocation.bits.IsDualQuad, context);
    }

    Cy_SMIF_DeviceEnable(device);

    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Cy_SMIF_MMIO_Read_ExMem
****************************************************************************//**
*
* This function reads data from external memory in MMIO mode. Please call
* Cy_SMIF_InitExMemContext to initialize device context
* (\ref cy_stc_ex_dev_context_t). 
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be read. \ref cy_en_smif_slave_select_t
*
* \param readCmdId
* Read command ID. please select read command ID from dedicating enumerator.
* e.g. if you initialized device context with CY_SMIF_S25FL, please find the 
* command ID from \ref cy_en_smif_s25fl_rd_cmdid_t.
* - Device and ID enumerator
* CY_SMIF_S25FL cy_en_smif_s25fl_rd_cmdid_t
* CY_SMIF_SEMP  cy_en_smif_semp_rd_cmdid_t
*
* \param readAddress
* Specifies address of external device to be read.
*
* \param size
* Specifies memory size to be read.
*
* \param buf
* Pointer to buffer where read data to be stored
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param mode
* Mode value of the read sequence
*
* \param blockingMode
* Blocking mode or not. if this is true, process waits for the read finished in this
* function. unless, the process does not wait and exit function.
*
* \param devContext
* Contains external device information like read/write sequence for each command
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_MMIO_Read_ExMem(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t readCmdId,
                                        uint32_t readAddress,
                                        uint32_t size,
                                        uint8_t buf[],
                                        uint8_t latencyCode,
                                        uint8_t mode,
                                        cy_en_smif_block_mode_t blockingMode,
                                        const cy_stc_ex_dev_context_t* devContext, 
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t status;

    if((devContext->p_readCmdInfo == NULL)                  ||
       (devContext->p_readCmdTypes == NULL)                 ||
       (devContext->p_readAddrTypes == NULL)                ||
       (devContext->p_readModeTypes == NULL)                ||
       (devContext->p_readDummyTypes == NULL)               ||
       (devContext->pp_readLatencyCodeToDummyCycle == NULL) ||
       (devContext->p_readDataTypes == NULL)                ||
       (devContext->p_readCmdInfo == NULL))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(readCmdId >= devContext->readCommandNum)
    {
        return CY_SMIF_BAD_PARAM;
    }

    uint8_t cmdTypeNum  = devContext->p_readCmdInfo[readCmdId].cmdTypeNum;
    uint8_t addrTypeNum = devContext->p_readCmdInfo[readCmdId].addrTypeNum;

    if(devContext->p_readCmdTypes[cmdTypeNum].presence == CY_SMIF_NOT_PRESENT)
    {
        return CY_SMIF_BAD_PARAM;
    }

    cy_en_smif_xip_addr_byte_t addrLen = devContext->p_readAddrTypes[addrTypeNum].addrLen;
    uint8_t                    addrLenInByte;
    uint32_t                   addrInBigEndian;

    if(addrLen == CY_SMIF_XIP_ADDRESS_4_BYTE)
    {
        // No need to check
        addrLenInByte   = 4u;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(readAddress);
    }
    else if(addrLen == CY_SMIF_XIP_ADDRESS_3_BYTE)
    {
        if(readAddress > 0x00FFFFFFul)
        {
            return CY_SMIF_BAD_PARAM;
        }
        addrLenInByte   = 3u;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(readAddress << 8ul);
    }
    else
    {
        CY_ASSERT_L1(false);
    }

    /****** Command and Address ******/
    bool is2ByteCmd;
    if((devContext->p_readCmdInfo[readCmdId].cmd & 0xFF00) == 0)
    {
        is2ByteCmd = false;
    }
    else
    {
        is2ByteCmd = true;
    }
    status = Cy_SMIF_TransmitCommand(base,
                                     devContext->p_readCmdInfo[readCmdId].cmd,         /* command value */
                                     is2ByteCmd,
                                     devContext->p_readCmdTypes[cmdTypeNum].width,     /* command width */
                                     devContext->p_readCmdTypes[cmdTypeNum].dataRate,  /* command rate */
                                     (uint8_t*)&addrInBigEndian,                /* Data pointer */
                                     addrLenInByte,                             /* Data size */
                                     devContext->p_readAddrTypes[addrTypeNum].width,   /* data width */
                                     devContext->p_readAddrTypes[addrTypeNum].dataRate,/* data rate */
                                     slave,                /* slave select */
                                     NOT_LAST_COMMAND,
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** Mode ******/
    uint8_t modeTypeNum = devContext->p_readCmdInfo[readCmdId].modeTypeNum;
    if(devContext->p_readModeTypes[modeTypeNum].presence != CY_SMIF_NOT_PRESENT)
    {
        status = Cy_SMIF_TransmitCommand(base,
                                         mode,
                                         false,
                                         devContext->p_readModeTypes[modeTypeNum].width,   /* command width */
                                         devContext->p_readModeTypes[modeTypeNum].dataRate,/* command rate */
                                         NULL,                 /* Data pointer */
                                         0,                    /* Data size */
                                         CY_SMIF_WIDTH_SINGLE, /* data width */
                                         CY_SMIF_SDR,          /* data rate */
                                         slave,                /* slave select */
                                         NOT_LAST_COMMAND,
                                         context);
    }
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** Dummy ******/
    uint8_t dummyTypeNum = devContext->p_readCmdInfo[readCmdId].dummyTypeNum;
    if(devContext->p_readDummyTypes[dummyTypeNum].presence != CY_SMIF_NOT_PRESENT)
    {
        uint8_t tableId      = devContext->p_readDummyTypes[dummyTypeNum].tableId;
        uint8_t dummyCycle   = devContext->pp_readLatencyCodeToDummyCycle[tableId][(uint32_t)latencyCode];
        if(dummyCycle > 0u)
        {
            status = Cy_SMIF_SendDummyCycles(base, 
                                             devContext->p_readDummyTypes[dummyTypeNum].width,  /* command width */
                                             devContext->p_readDummyTypes[dummyTypeNum].dataRate,          /* command rate */
                                             devContext->p_readDummyTypes[dummyTypeNum].doubleDm, 
                                             devContext->p_readDummyTypes[dummyTypeNum].rwdsInDm,
                                             devContext->p_readDummyTypes[dummyTypeNum].dlpEn,
                                             devContext->p_readDummyTypes[dummyTypeNum].rwdsEn,
                                             dummyCycle,
                                             context);
            if(status != CY_SMIF_SUCCESS)
            {
                return status;
            }
        }
    }

    /****** Data ******/
    uint8_t dataTypeNum = devContext->p_readCmdInfo[readCmdId].dataTypeNum;
    cy_en_smif_txfr_width_t width;
    if((CY_SMIF_GetSlaveNumber(base, slave) == 2u) && (devContext->p_readDataTypes[dataTypeNum].width == CY_SMIF_WIDTH_QUAD))
    {
        // Dual Quad mode
        width = CY_SMIF_WIDTH_OCTAL;
    }
    else
    {
        width = devContext->p_readDataTypes[dataTypeNum].width;
    }

    if(blockingMode == CY_SMIF_BLOCKING)
    {
        status = Cy_SMIF_ReceiveDataBlocking(base, buf, size, width, devContext->p_readDataTypes[dataTypeNum].dataRate, slave, context);
    }
    else
    {
        status = Cy_SMIF_ReceiveData(base, buf, size, width, devContext->p_readDataTypes[dataTypeNum].dataRate, slave, NULL, context);
    }
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_MMIO_Program_ExMem
****************************************************************************//**
*
* This function writes data into external memory in MMIO mode. Please call
* Cy_SMIF_InitExMemContext to initialize device context
* (\ref cy_stc_ex_dev_context_t). 
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param writeCmdId
* Write command ID. please select write command ID from dedicating enumerator.
* e.g. if you initialized device context with CY_SMIF_S25FL, please find the 
* command ID from \ref cy_en_smif_s25fl_wt_cmdid_t.
* - Device and ID enumerator
* CY_SMIF_S25FL cy_en_smif_s25fl_wt_cmdid_t
* CY_SMIF_SEMP  cy_en_smif_semp_wt_cmdid_t
*
* \param programedAddress
* Specifies address of external device to be write.
*
* \param size
* Specifies memory size to be read.
*
* \param buf
* Pointer to buffer where read data to be stored
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param mode
* Mode value of the read sequence
*
* \param blockingMode
* Blocking mode or not. if this is true, process waits for the read finished in this
* function. unless, the process does not wait and exit function.
*
* \param devContext
* Contains external device information like read/write sequence for each command
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_MMIO_Program_ExMem(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t writeCmdId,
                                        uint32_t programedAddress,
                                        uint32_t size,
                                        const uint8_t buf[],
                                        cy_en_smif_block_mode_t blockingMode,
                                        uint8_t lcCode,
                                        uint8_t mode,
                                        const cy_stc_ex_dev_context_t* devContext,
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t status;

    if((devContext->p_writeCmdInfo == NULL)                  ||
       (devContext->p_writeCmdTypes == NULL)                 ||
       (devContext->p_writeAddrTypes == NULL)                ||
       (devContext->p_writeModeTypes == NULL)                ||
       (devContext->p_writeDummyTypes == NULL)               ||
       (devContext->pp_writeLatencyCodeToDummyCycle == NULL) ||
       (devContext->p_writeDataTypes == NULL)                ||
       (devContext->p_writeCmdInfo == NULL))
    {
        return CY_SMIF_BAD_PARAM;
    }

    uint8_t cmdTypeNum  = devContext->p_writeCmdInfo[writeCmdId].cmdTypeNum;
    uint8_t addrTypeNum = devContext->p_writeCmdInfo[writeCmdId].addrTypeNum;

    if(devContext->p_writeCmdTypes[cmdTypeNum].presence == CY_SMIF_NOT_PRESENT)
    {
        return CY_SMIF_BAD_PARAM;
    }

    cy_en_smif_xip_addr_byte_t addrLen = devContext->p_writeAddrTypes[addrTypeNum].addrLen;
    uint8_t                    addrLenInByte;
    uint32_t                   addrInBigEndian;

    if(addrLen == CY_SMIF_XIP_ADDRESS_4_BYTE)
    {
        // No need to check
        addrLenInByte = 4u;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(programedAddress);
    }
    else if(addrLen == CY_SMIF_XIP_ADDRESS_3_BYTE)
    {
        if(programedAddress > 0x00FFFFFFul)
        {
            return CY_SMIF_BAD_PARAM;
        }
        addrLenInByte = 3u;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(programedAddress << 8ul);
    }
    else
    {
        CY_ASSERT_L1(false);
    }

    /****** Command and Address ******/
    bool is2ByteCmd;
    if((devContext->p_writeCmdInfo[writeCmdId].cmd & 0xFF00) == 0)
    {
        is2ByteCmd = false;
    }
    else
    {
        is2ByteCmd = true;
    }
    status = Cy_SMIF_TransmitCommand(base,
                                     devContext->p_writeCmdInfo[writeCmdId].cmd,         /* command value */
                                     is2ByteCmd,
                                     devContext->p_writeCmdTypes[cmdTypeNum].width,     /* command width */
                                     devContext->p_writeCmdTypes[cmdTypeNum].dataRate,  /* command rate */
                                     (uint8_t*)&addrInBigEndian,                          /* Data pointer */
                                     addrLenInByte,                            /* Data size */
                                     devContext->p_writeAddrTypes[addrTypeNum].width,   /* data width */
                                     devContext->p_writeAddrTypes[addrTypeNum].dataRate,/* data rate */
                                     slave,                /* slave select */
                                     NOT_LAST_COMMAND,
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** Mode ******/
    uint8_t modeTypeNum = devContext->p_writeCmdInfo[writeCmdId].modeTypeNum;
    if(devContext->p_writeModeTypes[modeTypeNum].presence != CY_SMIF_NOT_PRESENT)
    {
        status = Cy_SMIF_TransmitCommand(base,
                                         mode,
                                         false,
                                         devContext->p_writeModeTypes[modeTypeNum].width,   /* command width */
                                         devContext->p_writeModeTypes[modeTypeNum].dataRate,/* command rate */
                                         NULL,                 /* Data pointer */
                                         0,                    /* Data size */
                                         CY_SMIF_WIDTH_SINGLE, /* data width */
                                         CY_SMIF_SDR,          /* data rate */
                                         slave,                /* slave select */
                                         NOT_LAST_COMMAND,
                                         context);
    }
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /****** Dummy ******/
    uint8_t dummyTypeNum = devContext->p_writeCmdInfo[writeCmdId].dummyTypeNum;
    if(devContext->p_readDummyTypes[dummyTypeNum].presence != CY_SMIF_NOT_PRESENT)
    {
        uint8_t tableId      = devContext->p_writeDummyTypes[dummyTypeNum].tableId;
        uint8_t dummyCycle   = devContext->pp_writeLatencyCodeToDummyCycle[tableId][(uint32_t)lcCode];
        if(dummyCycle > 0u)
        {
            status = Cy_SMIF_SendDummyCycles(base, 
                                             devContext->p_writeDummyTypes[dummyTypeNum].width,  /* command width */
                                             devContext->p_writeDummyTypes[dummyTypeNum].dataRate,          /* command rate */
                                             devContext->p_writeDummyTypes[dummyTypeNum].doubleDm, 
                                             devContext->p_writeDummyTypes[dummyTypeNum].rwdsInDm,
                                             devContext->p_writeDummyTypes[dummyTypeNum].dlpEn,
                                             devContext->p_writeDummyTypes[dummyTypeNum].rwdsEn,
                                             dummyCycle,
                                             context);
            if(status != CY_SMIF_SUCCESS)
            {
                return status;
            }
        }
    }

    /****** Data ******/
    uint8_t dataTypeNum = devContext->p_writeCmdInfo[writeCmdId].dataTypeNum;
    cy_en_smif_txfr_width_t width;
    // Check if it indicate dual quad mode.
    if((CY_SMIF_GetSlaveNumber(base, slave) == 2u) && (devContext->p_writeDataTypes[dataTypeNum].width == CY_SMIF_WIDTH_QUAD))
    {
        // Dual quad
        width = CY_SMIF_WIDTH_OCTAL;
    }
    else
    {
        // Not dual quad
        width = devContext->p_writeDataTypes[dataTypeNum].width;
    }

    if(blockingMode == CY_SMIF_BLOCKING)
    {
        Cy_SMIF_TransmitDataBlocking(base, buf, size, width, devContext->p_writeDataTypes[dataTypeNum].dataRate, slave, context);
    }
    else
    {
        Cy_SMIF_TransmitData(base, buf, size, width, devContext->p_writeDataTypes[dataTypeNum].dataRate, slave, NULL, context);
    }

    return status;
}



/*******************************************************************************
* Function Cy_SMIF_TX_CMD
****************************************************************************//**
*
* This function is only used in the driver of each device.
* This function push TX command FIFO according to input "cmd"
* (\ref cy_un_smif_trx_cmd_t) which needs only command.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param en_trx
* SPI transmission type. \ref cy_en_smif_trx_type_t
*
* \param en_cmd
* Command value. Please input form enum value specified in each device driver.
*
* \param cmdLatTbls
* Dummy cycle number table list which specified in each device driver.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TX_CMD(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context)
{
    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx  = {en_trx};
    cy_un_smif_trx_cmd_t cmd = {en_cmd};

    uint8_t                    constDummyNum = cmd.bf.constLatNum;
    uint8_t*                   dummyTable    = (uint8_t*)cmdLatTbls[cmd.bf.latTblNo];
    cy_en_smif_trx_addr_len_t  inputAddrLen  = CY_TRX_ADDR_3BYTE;
    uint32_t                   inputAddr     = 0;
    uint8_t                    latencyCode   = 0;

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CmdAddrDum(base,
                                        slave,
                                        tx.en,
                                        cmd,
                                        constDummyNum,
                                        dummyTable,
                                        inputAddrLen,
                                        inputAddr,
                                        latencyCode,
                                        context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /*** Send De-select ***/
    Cy_SMIF_Deselect(base, context);

    return status;

}

/*******************************************************************************
* Function Cy_SMIF_TX_CMD_ADDR
****************************************************************************//**
*
* This function is only used in the driver of each device.
* This function push TX command FIFO according to input "cmd"
* (\ref cy_un_smif_trx_cmd_t) which needs command and address.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param en_trx
* SPI transmission type. \ref cy_en_smif_trx_type_t
*
* \param en_cmd
* Command value. Please input form enum value specified in each device driver.
*
* \param addrLen
* Address length to be output. \ref cy_en_smif_trx_addr_len_t
*
* \param addr
* Address to be output.
*
* \param cmdLatTbls
* Dummy cycle number table list which specified in each device driver.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TX_CMD_ADDR(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context)
{
    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx  = {en_trx};
    cy_un_smif_trx_cmd_t cmd = { .cmd = en_cmd};

    uint8_t               constDummyNum      = cmd.bf.constLatNum;
    uint8_t*              dummyTable         = (uint8_t*)cmdLatTbls[cmd.bf.latTblNo];
    cy_en_smif_trx_addr_len_t inputAddrLen  = addrLen;
    uint32_t              inputAddr          = addr;
    uint8_t               latencyCode        = 0;

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CmdAddrDum(base,
                                        slave,
                                        tx.en,
                                        cmd,
                                        constDummyNum,
                                        dummyTable,
                                        inputAddrLen,
                                        inputAddr,
                                        latencyCode,
                                        context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /*** Send De-select ***/
    Cy_SMIF_Deselect(base, context);

    return status;

}

/*******************************************************************************
* Function Cy_SMIF_TX_READ
****************************************************************************//**
*
* This function is only used in the driver of each device.
* This function push TX command FIFO according to input "cmd"
* (\ref cy_un_smif_trx_cmd_t) which needs command, address, dummy, and read
* transaction.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param en_trx
* SPI transmission type. \ref cy_en_smif_trx_type_t
*
* \param en_cmd
* Command value. Please input form enum value specified in each device driver.
*
* \param addrLen
* Address length to be output. \ref cy_en_smif_trx_addr_len_t
*
* \param addr
* Address to be output.
*
* \param readData
* Address to memory where read data value will be stored.
*
* \param readSize
* Read data size
*
* \param latCode
* Latency code of this transaction.
*
* \param cmdLatTbls
* Dummy cycle number table list which specified in each device driver.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TX_READ(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        uint8_t  readData[],
                                        uint32_t readSize,
                                        uint8_t latCode,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context)
{
    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx  = {en_trx};
    cy_un_smif_trx_cmd_t cmd = { .cmd = en_cmd};

    uint8_t               constDummyNum      = cmd.bf.constLatNum;
    uint8_t*              dummyTable         = (uint8_t*)cmdLatTbls[cmd.bf.latTblNo];
    cy_en_smif_trx_addr_len_t inputAddrLen  = addrLen;
    uint32_t              inputAddr          = addr;
    uint8_t               latencyCode        = latCode;

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CmdAddrDum(base,
                                        slave,
                                        tx.en,
                                        cmd,
                                        constDummyNum,
                                        dummyTable,
                                        inputAddrLen,
                                        inputAddr,
                                        latencyCode,
                                        context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /*** Receive process ***/
    status = Cy_SMIF_ReceiveDataBlocking(base, (uint8_t*)readData, readSize, (cy_en_smif_txfr_width_t)tx.bf.width, (cy_en_smif_data_rate_t)tx.bf.dataRate, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_TX_READ
****************************************************************************//**
*
* This function is only used in the driver of each device.
* This function push TX command FIFO according to input "cmd"
* (\ref cy_un_smif_trx_cmd_t) which needs command, address, dummy, and write
* transaction.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param en_trx
* SPI transmission type. \ref cy_en_smif_trx_type_t
*
* \param en_cmd
* Command value. Please input form enum value specified in each device driver.
*
* \param addrLen
* Address length to be output. \ref cy_en_smif_trx_addr_len_t
*
* \param addr
* Address to be output.
*
* \param writeData
* Address to data to be output.
*
* \param writeSize
* write data size
*
* \param cmdLatTbls
* Dummy cycle number table list which specified in each device driver.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TX_WRITE(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        uint8_t  writeData[],
                                        uint32_t writeSize,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context)
{
    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx  = {en_trx};
    cy_un_smif_trx_cmd_t cmd = { .cmd = en_cmd};

    uint8_t               constDummyNum      = cmd.bf.constLatNum;
    uint8_t*              dummyTable         = (uint8_t*)cmdLatTbls[cmd.bf.latTblNo];
    cy_en_smif_trx_addr_len_t inputAddrLen  = addrLen;
    uint32_t              inputAddr          = addr;
    uint8_t               latencyCode        = 0;

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CmdAddrDum(base,
                                        slave,
                                        tx.en,
                                        cmd,
                                        constDummyNum,
                                        dummyTable,
                                        inputAddrLen,
                                        inputAddr,
                                        latencyCode,
                                        context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /*** Transmit process ***/
    status = Cy_SMIF_TransmitDataBlocking(base, writeData, writeSize, (cy_en_smif_txfr_width_t)tx.bf.width, (cy_en_smif_data_rate_t)tx.bf.dataRate, slave, context);

    return status;

}

/*******************************************************************************
* Function CY_SMIF_GetSlaveNumber
****************************************************************************//**
*
* This function converts \ref cy_en_smif_slave_select_t value to slave number.
* e.g. if input value is b'0101, the function returns "2"
*
* \param slave (see \cy_en_smif_slave_select_t)
*
* \return
*  Slave number
*
*******************************************************************************/
static uint8_t CY_SMIF_GetSlaveNumber(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave)
{
    uint8_t slaveTemp = (uint8_t)slave;
    uint8_t slaveNum = 0;

    for(uint8_t i = 0; i < CY_SMIF_GetDeviceNumber(base); i++)
    {
        if((slaveTemp & 0x01) == 0x01)
        {
            slaveNum++;
        }
        slaveTemp = slaveTemp >> 1u;
    }
    return slaveNum;
}

/*******************************************************************************
* Function CY_SMIF_GetAddrByteSizeFromMemSize
****************************************************************************//**
*
* This function returns CY_SMIF_XIP_ADDRESS_4_BYTE if the size is larger than
* 0x00FFFFFF and returns CY_SMIF_XIP_ADDRESS_3_BYTE unless.
* \ref cy_en_smif_xip_addr_byte_t
*
* \param size to be checked
*
* \return (see \cy_en_smif_xip_addr_byte_t)
*
*******************************************************************************/
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrByteSizeFromMemSize(uint32_t size)
{
    cy_en_smif_xip_addr_byte_t addressByte;

    if(size >= Cy_SMIF_MaskValueToSizeInByte(CY_SMIF_DEVICE_16M_BYTE))
    {
        addressByte = CY_SMIF_XIP_ADDRESS_4_BYTE;
    }
    else
    {
        addressByte = CY_SMIF_XIP_ADDRESS_3_BYTE;
    }

    return addressByte;
}

/*******************************************************************************
* Function CY_SMIF_GetReadCmdInfoNum
****************************************************************************//**
*
* This function returns read command information No. from read command ID.
*
* \param rdCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \return
*  Command number which is defined and used internally
*
*******************************************************************************/
static uint16_t CY_SMIF_GetReadCmdInfoNum(uint16_t rdCmdId)
{
    return (uint16_t)rdCmdId;
}

/*******************************************************************************
* Function CY_SMIF_GetReadCmdInfoNum
****************************************************************************//**
*
* This function returns address length of input read command
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param rdCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \param context
* Contains external device information like read/write sequence for each command
*
* \return
*  Address length of input read command \ref cy_en_smif_xip_addr_byte_t
*
*******************************************************************************/
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrLenFromReadCmd(volatile cy_stc_smif_reg_device_t *device,
                                                                uint16_t rdCmdId,
                                                                const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_readAddrTypes != NULL);
    CY_ASSERT_L3(context->p_readCmdInfo != NULL);

    uint8_t readInfoNum = CY_SMIF_GetReadCmdInfoNum(rdCmdId);
    uint8_t addrTypeNum = context->p_readCmdInfo[readInfoNum].addrTypeNum;

    return (context->p_readAddrTypes[addrTypeNum].addrLen);
}

/*******************************************************************************
* Function Name CY_SMIF_DummyCycleEachReadCmd
****************************************************************************//**
*
* This function returns dummy cycle number of input read command and latency code
* The number will be stored in memory pointed to by dummyCycle. And presence
* information will be returned.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param rdCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param dummyCycle
* Pointer to a uint8_t memory which contains the dummy cycle number
*
* \param context
* Contains external device information like read/write sequence for each command
*
* \return \ref cy_en_smif_field_presence_t
*
*******************************************************************************/
static cy_en_smif_field_presence_t CY_SMIF_DummyCycleEachReadCmd(volatile cy_stc_smif_reg_device_t *device,
                                                                 uint16_t rdCmdId,
                                                                 uint8_t latencyCode,
                                                                 uint8_t* dummyCycle,
                                                                 const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_readCmdInfo != NULL);
    CY_ASSERT_L3(context->p_readDummyTypes != NULL);
    CY_ASSERT_L3(context->pp_readLatencyCodeToDummyCycle != NULL);
    CY_ASSERT_L3(latencyCode < context->readLatencyCodeMax);

    uint8_t readCmdNum   = CY_SMIF_GetReadCmdInfoNum(rdCmdId);
    uint8_t dummyTypeNum = context->p_readCmdInfo[readCmdNum].dummyTypeNum;
    uint8_t tableId      = context->p_readDummyTypes[dummyTypeNum].tableId;

    CY_ASSERT_L3(context->pp_readLatencyCodeToDummyCycle[tableId] != NULL);
    *dummyCycle = context->pp_readLatencyCodeToDummyCycle[tableId][latencyCode];

    return (context->p_readDummyTypes[dummyTypeNum].presence);
}

/*******************************************************************************
* Function CY_SMIF_SetXIPReadSequence
****************************************************************************//**
*
* This function sets up SMIF registers which is used read sequence in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param rdCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \param mode
* mode value of the read sequence
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param dualQuad
* specifies the device is dual Quad mode or not
*
* \param context
* Contains external device information like read/write sequence for each command
*
*******************************************************************************/
static void CY_SMIF_SetXIPReadSequence(volatile cy_stc_smif_reg_device_t *device,
                                       uint16_t rdCmdId,
                                       uint8_t mode,
                                       uint8_t latencyCode,
                                       bool dualQuad,
                                       const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_readCmdInfo != NULL);
    CY_ASSERT_L3(context->p_readCmdTypes != NULL);
    CY_ASSERT_L3(context->p_readAddrTypes != NULL);
    CY_ASSERT_L3(context->p_readModeTypes != NULL);
    CY_ASSERT_L3(context->p_readDataTypes != NULL);

    CY_ASSERT_L3(rdCmdId < context->readCommandNum);

    uint8_t readCmdNum = CY_SMIF_GetReadCmdInfoNum(rdCmdId);

    /************************/
    /*** Set read command ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_cmd_mode_ctl_t rdCmdParams;
    uint8_t cmdTypeNum    = context->p_readCmdInfo[readCmdNum].cmdTypeNum;
    rdCmdParams.cmd       = (uint16_t)context->p_readCmdInfo[readCmdNum].cmd;
    rdCmdParams.width     = context->p_readCmdTypes[cmdTypeNum].width;
    rdCmdParams.dataRate  = context->p_readCmdTypes[cmdTypeNum].dataRate;
    rdCmdParams.presence  = context->p_readCmdTypes[cmdTypeNum].presence;
    /* Set the parameters */
    Cy_SMIF_SetReadCmdCtl(device, &rdCmdParams);

    /************************/
    /*** Set read address ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_addr_data_ctl_t rdAddrParams;
    uint8_t addrTypeNum = context->p_readCmdInfo[readCmdNum].addrTypeNum;
    rdAddrParams.width    = context->p_readAddrTypes[addrTypeNum].width;
    rdAddrParams.dataRate = context->p_readAddrTypes[addrTypeNum].dataRate;
    /* Set the parameters */
    Cy_SMIF_SetReadAddrCtl(device, &rdAddrParams);

    /************************/
    /***  Set read mode  ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_cmd_mode_ctl_t rdModeParams;
    uint8_t modeTypeNum = context->p_readCmdInfo[readCmdNum].modeTypeNum;
    rdModeParams.cmd      = mode;
    rdModeParams.width    = context->p_readModeTypes[modeTypeNum].width;
    rdModeParams.dataRate = context->p_readModeTypes[modeTypeNum].dataRate;
    rdModeParams.presence = context->p_readModeTypes[modeTypeNum].presence;
    /* Set the parameters */
    Cy_SMIF_SetReadModeCtl(device, &rdModeParams);

    /************************/
    /***  Set read dummy  ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_rd_dummy_ctl_t rdDummyParams;
    uint8_t dummyCycle;
    rdDummyParams.presence   = CY_SMIF_DummyCycleEachReadCmd(device, rdCmdId, latencyCode, &dummyCycle, context);
    rdDummyParams.numOfDummy = dummyCycle;
    /* Set the parameters */
    Cy_SMIF_SetReadDummyCtl(device, &rdDummyParams);

    /************************/
    /***  Set read data   ***/
    /************************/
    stc_cy_smif_addr_data_ctl_t rdDataParams;
    /* Prepare the parameters */
    uint8_t dataTypeNum = context->p_readCmdInfo[readCmdNum].dataTypeNum;
    rdDataParams.width    = (dualQuad == true) ? CY_SMIF_WIDTH_OCTAL : context->p_readDataTypes[dataTypeNum].width;
    rdDataParams.dataRate = context->p_readDataTypes[dataTypeNum].dataRate;
    /* Set the parameters */
    Cy_SMIF_SetReadDataCtl(device, &rdDataParams);

    /************************/
    /***   Set read CRC   ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_rd_crc_ctl_t rdCRCParams = { 0u };
    /* Set the parameters */
    Cy_SMIF_SetReadCrcCtl(device, &rdCRCParams);

    /************************/
    /***  Set read bound  ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_rd_bound_ctl_t rdBoundParams = { 0u };
    /* Set the parameters */
    Cy_SMIF_SetReadBoundCtl(device, &rdBoundParams);
    
    return;

}

/*******************************************************************************
* Function CY_SMIF_GetWriteCmdInfoNum
****************************************************************************//**
*
* This function returns write command information No. from read command ID.
*
* \param wtCmdId
* Write command ID which is defined in each c files dedicating to an external
* device
*
* \return
*  Command number which is defined and used internally
*
*******************************************************************************/
static uint16_t CY_SMIF_GetWriteCmdInfoNum(uint16_t wtCmdId)
{
    return (uint16_t)wtCmdId;
}

/*******************************************************************************
* Function CY_SMIF_GetAddrLenFromWriteCmd
****************************************************************************//**
*
* This function returns address length of input read command
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param wtCmdId
* Write command ID which is defined in each c files dedicating to an external
* device
*
* \param context
* Contains external device information like read/write sequence for each command
*
* \return
*  Address length of input read command \ref cy_en_smif_xip_addr_byte_t
*
*******************************************************************************/
static cy_en_smif_xip_addr_byte_t CY_SMIF_GetAddrLenFromWriteCmd(volatile cy_stc_smif_reg_device_t *device,
                                                                 uint16_t wtCmdId,
                                                                 const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_writeAddrTypes != NULL);
    CY_ASSERT_L3(context->p_writeCmdInfo != NULL);

    uint8_t writeInfoNum = CY_SMIF_GetWriteCmdInfoNum(wtCmdId);
    uint8_t addrTypeNum = context->p_writeCmdInfo[writeInfoNum].addrTypeNum;

    return (context->p_writeAddrTypes[addrTypeNum].addrLen);
}


/*******************************************************************************
* Function Name CY_SMIF_DummyCycleEachWriteCmd
****************************************************************************//**
*
* This function returns dummy cycle number of input write command and latency code
* The number will be stored in memory pointed to by dummyCycle. And presence
* information will be returned.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param wtCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param dummyCycle
* Pointer to a uint8_t memory which contains the dummy cycle number
*
* \param context
* Contains external device information like read/write sequence for each command
*
* \return \ref cy_en_smif_field_presence_t
*
*******************************************************************************/
static cy_en_smif_field_presence_t CY_SMIF_DummyCycleEachWriteCmd(volatile cy_stc_smif_reg_device_t *device, 
                                                                  uint16_t wtCmdId,
                                                                  uint8_t latencyCode,
                                                                  uint8_t* dummyCycle,
                                                                  bool* rwdsEn,
                                                                  const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_writeCmdInfo != NULL);
    CY_ASSERT_L3(context->p_writeDummyTypes != NULL);
    CY_ASSERT_L3(context->pp_writeLatencyCodeToDummyCycle != NULL);
    CY_ASSERT_L3(latencyCode < context->writeLatencyCodeMax);

    uint8_t writeCmdNum  = CY_SMIF_GetWriteCmdInfoNum(wtCmdId);
    uint8_t dummyTypeNum = context->p_writeCmdInfo[writeCmdNum].dummyTypeNum;
    uint8_t tableId      = context->p_writeDummyTypes[dummyTypeNum].tableId;
    *rwdsEn     = context->p_writeDummyTypes[dummyTypeNum].rwdsEn;

    CY_ASSERT_L3(context->pp_writeLatencyCodeToDummyCycle[tableId] != NULL);
    *dummyCycle = context->pp_writeLatencyCodeToDummyCycle[tableId][latencyCode];
    return (context->p_writeDummyTypes[dummyTypeNum].presence);
}

/*******************************************************************************
* Function CY_SMIF_SetXIPWriteSequence
****************************************************************************//**
*
* This function sets up SMIF registers which is used write sequence in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param wtCmdId
* Read command ID which is defined in each c files dedicating to an external
* device
*
* \param mode
* Mode value of the write sequence
*
* \param latencyCode
* Latency code. This value should be set also to the external device.
*
* \param dualQuad
* specifies the device is dual Quad mode or not
*
* \param context
* Contains external device information like read/write sequence for each command
*
*******************************************************************************/
static void CY_SMIF_SetXIPWriteSequence(volatile cy_stc_smif_reg_device_t *device,
                                        uint16_t wtCmdId,
                                        uint8_t mode,
                                        uint8_t latencyCode,
                                        bool dualQuad,
                                        const cy_stc_ex_dev_context_t* context)
{
    CY_ASSERT_L3(context->p_writeCmdInfo != NULL);
    CY_ASSERT_L3(context->p_writeCmdTypes != NULL);
    CY_ASSERT_L3(context->p_writeAddrTypes != NULL);
    CY_ASSERT_L3(context->p_writeModeTypes != NULL);
    CY_ASSERT_L3(context->p_writeDataTypes != NULL);

    CY_ASSERT_L3(wtCmdId < context->writeCommandNum);

    uint8_t writeCmdNum = CY_SMIF_GetWriteCmdInfoNum(wtCmdId);

    /*************************/
    /*** Set write command ***/
    /*************************/
    /* Prepare the parameters */
    stc_cy_smif_cmd_mode_ctl_t wtCmdParams;
    wtCmdParams.cmd       = (uint16_t)context->p_writeCmdInfo[writeCmdNum].cmd;

    uint8_t cmdTypeNum = context->p_writeCmdInfo[writeCmdNum].cmdTypeNum;
    wtCmdParams.width     = context->p_writeCmdTypes[cmdTypeNum].width;
    wtCmdParams.dataRate  = context->p_writeCmdTypes[cmdTypeNum].dataRate;
    wtCmdParams.presence  = context->p_writeCmdTypes[cmdTypeNum].presence;
    /* Set the parameters */
    Cy_SMIF_SetWriteCmdCtl(device, &wtCmdParams);

    /*************************/
    /*** Set write address ***/
    /*************************/
    /* Prepare the parameters */
    stc_cy_smif_addr_data_ctl_t wtAddrParams;
    uint8_t addrTypeNum = context->p_writeCmdInfo[writeCmdNum].addrTypeNum;
    wtAddrParams.width    = context->p_writeAddrTypes[addrTypeNum].width;
    wtAddrParams.dataRate = context->p_writeAddrTypes[addrTypeNum].dataRate;
    /* Set the parameters */
    Cy_SMIF_SetWriteAddrCtl(device, &wtAddrParams);

    /************************/
    /***  Set write mode  ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_cmd_mode_ctl_t wtModeParams;
    uint8_t modeTypeNum = context->p_writeCmdInfo[writeCmdNum].modeTypeNum;
    wtModeParams.cmd      = mode;
    wtModeParams.width    = context->p_writeModeTypes[modeTypeNum].width;
    wtModeParams.dataRate = context->p_writeModeTypes[modeTypeNum].dataRate;
    wtModeParams.presence = context->p_writeModeTypes[modeTypeNum].presence;
    /* Set the parameters */
    Cy_SMIF_SetWriteModeCtl(device, &wtModeParams);

    /*************************/
    /***  Set write dummy  ***/
    /*************************/
    /* Prepare the parameters */
    stc_cy_smif_wr_dummy_ctl_t wtDummyParams;
    uint8_t dummyCycle;
    bool    rwdsEn;
    wtDummyParams.presence   = CY_SMIF_DummyCycleEachWriteCmd(device, wtCmdId, latencyCode, &dummyCycle, &rwdsEn, context);
    wtDummyParams.numOfDummy = dummyCycle;
    wtDummyParams.rwdsEn     = rwdsEn;
    /* Set the parameters */
    Cy_SMIF_SetWriteDummyCtl(device, &wtDummyParams);

    /************************/
    /***  Set write data   ***/
    /************************/
    /* Prepare the parameters */
    uint8_t dataTypeNum = context->p_writeCmdInfo[writeCmdNum].dataTypeNum;
    stc_cy_smif_addr_data_ctl_t wtDataParams;
    wtDataParams.width    = (dualQuad == true) ? CY_SMIF_WIDTH_OCTAL : context->p_writeDataTypes[dataTypeNum].width;
    wtDataParams.dataRate = context->p_writeDataTypes[dataTypeNum].dataRate;
    /* Set the parameters */
    Cy_SMIF_SetWriteDataCtl(device, &wtDataParams);

    /************************/
    /***   Set write CRC   ***/
    /************************/
    /* Prepare the parameters */
    stc_cy_smif_wr_crc_ctl_t wtCRCParams = { 0u };
    /* Set the parameters */
    Cy_SMIF_SetWriteCrcCtl(device, &wtCRCParams);

    return;

}

/*******************************************************************************
* Function Cy_SMIF_TX_CmdAddrDum
****************************************************************************//**
*
* This function is only used in the driver of each device.
* This function push TX command FIFO according to input "cmd"
* (\ref cy_un_smif_trx_cmd_t) which need command, address, dummy.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies slave of external device to be write. \ref cy_en_smif_slave_select_t
*
* \param en_trx
* SPI transmission type. \ref cy_en_smif_trx_type_t
*
* \param constDummyNum
* Dummy cycle number in case the transaction has constant dummy cycle number.
*
* \param dummyTable
* Dummy cycle number table in case the dummy cycle number depends on latency code.
*
* \param inputAddrLen
* Address length to be output. \ref cy_en_smif_trx_addr_len_t
*
* \param inputAddr
* Address to be output.
*
* \param lc
* Latency code in case the dummy cycle number depends on latency code.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
static cy_en_smif_status_t Cy_SMIF_TX_CmdAddrDum(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        cy_un_smif_trx_cmd_t cmd,
                                        uint8_t constDummyNum,
                                        uint8_t* dummyTable,
                                        cy_en_smif_trx_addr_len_t inputAddrLen,
                                        uint32_t inputAddr,
                                        uint8_t lc,
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx  = {en_trx};

    /*** Parameter Checking ***/
    if(cmd.cmd == CY_SPI_INVALID_CMD)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /*** Decide Address field ***/
    if((bool)cmd.bf.addrCfgAble == true)
    {
        if(inputAddrLen == CY_TRX_ADDR_3BYTE)
        {
            cmd.bf.addrLen = 3;
        }
        else
        {
            cmd.bf.addrLen = 4;
        }
    }

    uint32_t addrInBE = Cy_SMIF_Reverse4ByteEndian(inputAddr << (8ul * (4u - cmd.bf.addrLen)));

    /*** Transmit Command / Address ***/
    status = Cy_SMIF_TransmitCommand(base,
                                     cmd.bf.cmdVal,      /* command value */
                                     tx.bf.isCmd2B,      /* is command 2 byte */
                                     (cy_en_smif_txfr_width_t)tx.bf.width,        /* command width */
                                     (cy_en_smif_data_rate_t)tx.bf.dataRate,     /* command rate */
                                     (uint8_t*)&addrInBE, /* Data pointer */
                                     cmd.bf.addrLen,     /* Data size */
                                     (cy_en_smif_txfr_width_t)tx.bf.width,        /* data width */
                                     (cy_en_smif_data_rate_t)tx.bf.dataRate,     /* data rate */
                                     slave,              /* slave select */
                                     NOT_LAST_COMMAND,
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    /*** Transmit dummy ***/
    uint8_t dummyCount;
    if(dummyTable == NULL)
    {
        dummyCount = constDummyNum;
    }
    else
    {
        dummyCount = dummyTable[lc];
    }
    status = Cy_SMIF_SendDummyCycles(base, 
                                     (cy_en_smif_txfr_width_t)tx.bf.width,    /* command width */
                                     (cy_en_smif_data_rate_t)tx.bf.dataRate, /* command rate */
                                     false, false, false, cmd.bf.rwdsCapEn,
                                     dummyCount,
                                     context);
    if(status == CY_SMIF_CMD_FIFO_FULL)
    {
        return status;
    }

    return CY_SMIF_SUCCESS;
}


#if defined(__cplusplus)
}
#endif

