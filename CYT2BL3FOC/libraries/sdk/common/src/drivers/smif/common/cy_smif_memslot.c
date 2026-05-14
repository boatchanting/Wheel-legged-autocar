/***************************************************************************//**
* \file cy_smif_memslot.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the memory-level APIs of the SMIF driver.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "smif/common/cy_smif_memslot.h"

#ifdef CY_IP_MXSMIF

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*     Function Prototypes
***************************************/
static void Cy_SMIF_Memslot_XipRegInit(cy_stc_smif_reg_device_t volatile *dev,
                            cy_stc_smif_mem_config_t const * memCfg);


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_Init
****************************************************************************//**
*
* This function initializes the slots of the memory device in the SMIF
* configuration. After this initialization, the memory slave devices are
* automatically mapped into the PSoC memory map. The function needs the SMIF
* to be running in the memory mode to have the memory mapped into the PSoC
* address space. This function is typically called in the System initialization
* phase to initialize all the memory-mapped SMIF devices.
* This function only configures the memory device portion of the SMIF
* initialization and therefore assumes that the SMIF blocks initialization is
* achieved using Cy_SMIF_Init(). The cy_stc_smif_context_t context structure
* returned from Cy_SMIF_Init() is passed as a parameter to this function.
* This function calls the \ref Cy_SMIF_Memslot_SfdpDetect() function for each
* element of the \ref cy_stc_smif_mem_config_t memConfig array and fills the memory                 
* parameters if the autoDetectSfdp field is enabled in \ref cy_stc_smif_mem_config_t.
* The filled memConfig is a part of the \ref cy_stc_smif_block_config_t * blockConfig
* structure. The function expects that all the requirements of
*  \ref Cy_SMIF_Memslot_SfdpDetect() is provided.
*
* \param base
* The address of the slave-slot device register to initialize.
*
* \param context
* The SMIF internal context structure of the block.
*
* \param blockConfig
* The configuration structure array that configures the SMIF memory device to be
* mapped into the PSoC memory map. \ref cy_stc_smif_mem_config_t
*
* \return The memory slot initialization status.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_Init(cy_stc_smif_reg_t volatile *base,
                            const cy_stc_smif_block_config_t* blockConfig,
                            cy_stc_smif_context_t *context)
{
    cy_stc_smif_reg_device_t volatile * device;
    cy_stc_smif_mem_config_t const * memCfg;

    if(base == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(blockConfig == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if((blockConfig->memCount == 0u) ||
       (blockConfig->memCount  > CY_SMIF_GetDeviceNumber(base)))
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(context == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    for(uint32_t i_mem = 0; i_mem < blockConfig->memCount; i_mem++)
    {
        if(blockConfig->memConfig[i_mem] == NULL)
        {
            return CY_SMIF_BAD_PARAM;
        }

        if(blockConfig->memConfig[i_mem]->deviceCfg == NULL)
        {
            return CY_SMIF_BAD_PARAM;
        }
    }

    for(uint32_t i_memIdx = 0UL; i_memIdx < blockConfig->memCount; i_memIdx++)
    {
        memCfg = blockConfig->memConfig[i_memIdx];
        device = &base->DEVICE[i_memIdx];

        /* The slave-slot initialization of the device control register.
         * Cy_SMIF_Memslot_SfdpDetect() must work */
        Cy_SMIF_DeviceSetDataSelect(device, memCfg->dataSelect);

        if(memCfg->XIPSettingEn == true)
        {
            Cy_SMIF_Memslot_XipRegInit(device, memCfg);
            Cy_SMIF_DeviceEnable(device);
        }

        if(memCfg->writeEn == true)
        {
            Cy_SMIF_DeviceWriteEnable(device);
        }
        else
        {
            Cy_SMIF_DeviceWriteDisable(device);
        }

        if(memCfg->crypteEn == true)
        {
            Cy_SMIF_DeviceCryptoEnable(device);
        }
        else
        {
            Cy_SMIF_DeviceCryptoDisable(device);
        }
    }

    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_XipRegInit
****************************************************************************//**
*
* \internal
* This function initializes the memory device registers used for the XIP mode of
* the specified device.
*
* \param dev
* The SMIF memory device registers structure. \ref cy_stc_smif_reg_device_t
*
* \param memCfg
* The memory configuration structure that configures the SMIF memory device to
*  map into the PSoC memory map. \ref cy_stc_smif_mem_config_t
*
*******************************************************************************/
static void Cy_SMIF_Memslot_XipRegInit(cy_stc_smif_reg_device_t volatile *dev,
                            cy_stc_smif_mem_config_t const * memCfg)
{
    /* Setting addres and size and others used in XIP */
    Cy_SMIF_SetXIPBaseAddress(dev, memCfg->baseAddress);
    Cy_SMIF_SetXIPAddressSize(dev, memCfg->memMappedSize);
    Cy_SMIF_SetXIPAddrCtl(dev, memCfg->deviceCfg->numOfAddrBytes, (memCfg->dualQuadSlots != 0));

    /* Setting for reading protocol in XIP */
    Cy_SMIF_SetReadCmdCtl(dev, &memCfg->deviceCfg->readCmd->cmdCtl);
    Cy_SMIF_SetReadAddrCtl(dev, &memCfg->deviceCfg->readCmd->addrCtl);
    Cy_SMIF_SetReadModeCtl(dev, &memCfg->deviceCfg->readCmd->modeCtl);
    Cy_SMIF_SetReadDummyCtl(dev, &memCfg->deviceCfg->readCmd->memDummyCtl.dummyCtl);
    Cy_SMIF_SetReadDataCtl(dev, &memCfg->deviceCfg->readCmd->dataCtl);
    Cy_SMIF_SetReadCrcCtl(dev, &memCfg->deviceCfg->readCmd->crcCtl);
    Cy_SMIF_SetReadBoundCtl(dev, &memCfg->deviceCfg->readCmd->boundCtl);

    /* Setting for writinging protocol in XIP */
    Cy_SMIF_SetWriteCmdCtl(dev, &memCfg->deviceCfg->programCmd->cmdCtl);
    Cy_SMIF_SetWriteAddrCtl(dev, &memCfg->deviceCfg->programCmd->addrCtl);
    Cy_SMIF_SetWriteModeCtl(dev, &memCfg->deviceCfg->programCmd->modeCtl);
    Cy_SMIF_SetWriteDummyCtl(dev, &memCfg->deviceCfg->programCmd->memDummyCtl.dummyCtl);
    Cy_SMIF_SetWriteDataCtl(dev, &memCfg->deviceCfg->programCmd->dataCtl);
    Cy_SMIF_SetWriteCrcCtl(dev, &memCfg->deviceCfg->programCmd->crcCtl);
}


/*******************************************************************************
* Function Name: Cy_SMIF_MemoryDeInit
****************************************************************************//**
*
* This function de-initializes all slave slots of the memory device to their default
* values.
*
* \param base
* Holds the base address of the SMIF block registers.
*
*******************************************************************************/
void Cy_SMIF_Memslot_DeInit(cy_stc_smif_reg_t volatile *base)
{
    /* Configure the SMIF device slots to the default values. The default value is 0 */
    uint32_t deviceNum = (uint32_t)CY_SMIF_GetDeviceNumber(base);
    for(uint32_t deviceIndex = 0UL; deviceIndex < deviceNum; deviceIndex++)
    {
        base->DEVICE[deviceIndex].unCTL.u32Register           = 0U;
        base->DEVICE[deviceIndex].unADDR.u32Register          = 0U;
        base->DEVICE[deviceIndex].unMASK.u32Register          = 0U;
        base->DEVICE[deviceIndex].unADDR_CTL.u32Register      = 0U;
        base->DEVICE[deviceIndex].unRD_CMD_CTL.u32Register    = 0U;
        base->DEVICE[deviceIndex].unRD_ADDR_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unRD_MODE_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unRD_DUMMY_CTL.u32Register  = 0U;
        base->DEVICE[deviceIndex].unRD_DATA_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unRD_CRC_CTL.u32Register    = 0U;
        base->DEVICE[deviceIndex].unRD_BOUND_CTL.u32Register  = 0U;
        base->DEVICE[deviceIndex].unWR_CMD_CTL.u32Register    = 0U;
        base->DEVICE[deviceIndex].unWR_ADDR_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unWR_MODE_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unWR_DUMMY_CTL.u32Register  = 0U;
        base->DEVICE[deviceIndex].unWR_DATA_CTL.u32Register   = 0U;
        base->DEVICE[deviceIndex].unWR_CRC_CTL.u32Register    = 0U;
        /* Read the register to flush the buffer */
        (void) base->DEVICE[deviceIndex].unCTL.u32Register;
    }
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdWriteEnable
****************************************************************************//**
*
* This function sends the Write Enable command to the memory device.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteEnable(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t const *memDevice,
                                        cy_stc_smif_context_t *context)
{
    /* The memory Write Enable */
    cy_stc_smif_mem_cmd_t* writeEn = memDevice->deviceCfg->writeEnCmd;

    CY_ASSERT_L1(NULL != writeEn);  

    return Cy_SMIF_TransmitCommand( base, 
                                    writeEn->cmdCtl.cmd,
                                    memDevice->deviceCfg->isCommand2Byte,
                                    writeEn->cmdCtl.width,
                                    writeEn->cmdCtl.dataRate,
                                    CY_SMIF_CMD_WITHOUT_PARAM,
                                    CY_SMIF_CMD_WITHOUT_PARAM,
                                    (cy_en_smif_txfr_width_t)CY_SMIF_CMD_WITHOUT_PARAM,
                                    CY_SMIF_SDR,
                                    memDevice->slaveSelect,
                                    LAST_COMMAND_BYTE,
                                    context);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdWriteDisable
****************************************************************************//**
*
* This function sends a Write Disable command to the memory device.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode
* this API should be called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteDisable(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    cy_stc_smif_context_t *context)
{
    cy_stc_smif_mem_cmd_t* writeDis = memDevice->deviceCfg->writeDisCmd;

    CY_ASSERT_L1(NULL != writeDis);  
     
    /* The memory write disable */
    return Cy_SMIF_TransmitCommand( base,
                                    writeDis->cmdCtl.cmd,
                                    memDevice->deviceCfg->isCommand2Byte,
                                    writeDis->cmdCtl.width,
                                    writeDis->cmdCtl.dataRate,
                                    CY_SMIF_CMD_WITHOUT_PARAM,
                                    CY_SMIF_CMD_WITHOUT_PARAM,
                                    (cy_en_smif_txfr_width_t) CY_SMIF_CMD_WITHOUT_PARAM,
                                    CY_SMIF_SDR,
                                    memDevice->slaveSelect,
                                    LAST_COMMAND_BYTE,
                                    context);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_IsBusy
****************************************************************************//**
*
* This function checks if the status of the memory device is busy.
* This is done by reading the status register and the corresponding bit
* (stsRegBusyMask). This function is a blocking function until the status
* register from the memory is read.
*
* \note In the dual quad mode, this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
*  The device to which the command is sent.
*
* \param context
* The internal SMIF context data.
*
* \return A status of the memory device.
*       - True - The device is busy or a timeout occurs.
*       - False - The device is not busy.
*
*******************************************************************************/
bool Cy_SMIF_Memslot_IsBusy(cy_stc_smif_reg_t volatile *base, cy_stc_smif_mem_config_t *memDevice,
                            cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t readStsResult;
    cy_stc_smif_mem_device_cfg_t* device =  memDevice->deviceCfg;

    CY_ASSERT_L1(NULL != device->readStsRegWipCmd);

    uint8_t readStatus;
    readStsResult = Cy_SMIF_Memslot_CmdReadSts(base,
                                               memDevice,
                                               &readStatus,
                                               device->readStsRegWipCmd->cmdCtl.cmd,
                                               device->readStsRegWipCmd->cmdCtl.width,
                                               device->readStsRegWipCmd->cmdCtl.dataRate,
                                               context);

    CY_ASSERT_L1(readStsResult == CY_SMIF_SUCCESS);    
    (void) readStsResult; // avoid "unused" compiler warning if NDEBUG is set

    if((readStatus & device->stsRegBusyMask) == 0U)
    {
        return false; // not busy
    }
    else
    {
        return true; // work in progress
    }
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_QuadEnable
****************************************************************************//**
*
* This function enables the memory device for the quad mode of operation.
* This command must be executed before sending Quad SPI commands to the
* memory device.
*
* \note In the dual quad mode, this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param context
* The internal SMIF context data.
*
* \return A status of the command.
*   - \ref CY_SMIF_SUCCESS
*   - \ref CY_SMIF_NO_QE_BIT
*   - \ref CY_SMIF_CMD_FIFO_FULL
*   - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_QuadEnable(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t *memDevice,
                                    cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;
    cy_stc_smif_mem_device_cfg_t* device =  memDevice->deviceCfg;

    uint8_t readStatus0;
    result = Cy_SMIF_Memslot_CmdReadSts(base,
                                        memDevice,
                                        &readStatus0,
                                        device->readStsRegWipCmd->cmdCtl.cmd,
                                        device->readStsRegWipCmd->cmdCtl.width,
                                        device->readStsRegWipCmd->cmdCtl.dataRate,
                                        context);

    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    uint8_t readStatus1;
    result = Cy_SMIF_Memslot_CmdReadSts(base,
                                        memDevice,
                                        &readStatus1,
                                        device->readStsRegQeCmd->cmdCtl.cmd,
                                        device->readStsRegQeCmd->cmdCtl.width,
                                        device->readStsRegQeCmd->cmdCtl.dataRate,
                                        context);

    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    union
    {
        uint8_t  u8[2];
        uint16_t u16;
    } writeData;
    writeData.u8[0] = readStatus0;
    writeData.u8[1] = readStatus1;
    writeData.u16  |= device->stsRegQuadEnableMask;

    Cy_SMIF_Memslot_CmdWriteSts(base,
                                memDevice,
                                (void*)&writeData, 
                                device->writeStsRegQeCmd->cmdCtl.cmd,
                                device->writeStsRegQeCmd->cmdCtl.width,
                                device->writeStsRegQeCmd->cmdCtl.dataRate,
                                context);

    return(result);
}

/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_BlockProtNone
****************************************************************************//**
*
* This function get the memory block protection for none.
*
* \note In the dual quad mode, this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param context
* The internal SMIF context data.
*
* \return A status of the command.
*   - \ref CY_SMIF_SUCCESS
*   - \ref CY_SMIF_NO_QE_BIT
*   - \ref CY_SMIF_CMD_FIFO_FULL
*   - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_BlockProtNone(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t *memDevice,
                                    cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;
    cy_stc_smif_mem_device_cfg_t* device =  memDevice->deviceCfg;

    uint8_t readStatus0;
    result = Cy_SMIF_Memslot_CmdReadSts(base,
                                        memDevice,
                                        &readStatus0,
                                        device->readStsRegWipCmd->cmdCtl.cmd,
                                        device->readStsRegWipCmd->cmdCtl.width,
                                        device->readStsRegWipCmd->cmdCtl.dataRate,
                                        context);

    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    uint8_t readStatus1;
    result = Cy_SMIF_Memslot_CmdReadSts(base,
                                        memDevice,
                                        &readStatus1,
                                        device->readStsRegQeCmd->cmdCtl.cmd,
                                        device->readStsRegQeCmd->cmdCtl.width,
                                        device->readStsRegQeCmd->cmdCtl.dataRate,
                                        context);

    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    union
    {
        uint8_t  u8[2];
        uint16_t u16;
    } writeData;
    writeData.u8[0] = readStatus0;
    writeData.u8[1] = readStatus1;
    writeData.u16  &= (~device->stsRegProtectionMask);

    Cy_SMIF_Memslot_CmdWriteSts(base,
                                memDevice,
                                (void*)&writeData, 
                                device->writeStsRegQeCmd->cmdCtl.cmd,
                                device->writeStsRegQeCmd->cmdCtl.width,
                                device->writeStsRegQeCmd->cmdCtl.dataRate,
                                context);

    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdReadSts
****************************************************************************//**
*
* This function reads the status register. This function is a blocking function,
* it will block the execution flow until the status register is read.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* the Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param status
* The status register value returned by the external memory.
*
* \param command
* The command required to read the status/configuration register.
*
* \param context
* The internal SMIF context data.
*
* \return A status of the command reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdReadSts(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    uint8_t *status,
                                    uint16_t command,
                                    cy_en_smif_txfr_width_t dataWidth,
                                    cy_en_smif_data_rate_t dataRate,
                                    cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;

    /* Read the memory status register */
    result = Cy_SMIF_TransmitCommand(base,
                                     command,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     dataWidth,
                                     dataRate,
                                     CY_SMIF_CMD_WITHOUT_PARAM,
                                     CY_SMIF_CMD_WITHOUT_PARAM,
                                     (cy_en_smif_txfr_width_t) CY_SMIF_CMD_WITHOUT_PARAM,
                                     CY_SMIF_SDR,
                                     memDevice->slaveSelect,
                                     NOT_LAST_COMMAND,
                                     context);

    if (result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    result = Cy_SMIF_ReceiveDataBlocking(base, status,
                CY_SMIF_READ_ONE_BYTE, dataWidth, dataRate, memDevice->slaveSelect, context);

    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdWriteSts
****************************************************************************//**
*
* This function writes the status register. This is a blocking function, it will
* block the execution flow until the command transmission is completed.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param status
* The status to write into the status register.
*
* \param command
* The command to write into the status/configuration register.
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteSts(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    void const *status,
                                    uint8_t command,
                                    cy_en_smif_txfr_width_t dataWidth,
                                    cy_en_smif_data_rate_t dataRate,
                                    cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;

    /* The Write Enable */
    result =  Cy_SMIF_Memslot_CmdWriteEnable(base, memDevice, context);
    if (result != CY_SMIF_SUCCESS)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* The Write Status */
    result = Cy_SMIF_TransmitCommand(base,
                                     command,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     dataWidth,
                                     dataRate,
                                     (uint8_t const *)status,
                                     2,
                                     dataWidth,
                                     dataRate,
                                     memDevice->slaveSelect,
                                     LAST_COMMAND_BYTE,
                                     context);

    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdChipErase
****************************************************************************//**
*
* This function performs a chip erase of the external memory. The Write Enable
* command is called before this API.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdChipErase(cy_stc_smif_reg_t volatile *base,
                cy_stc_smif_mem_config_t const *memDevice,
                cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;

    cy_stc_smif_mem_cmd_t *cmdErase = memDevice->deviceCfg->chipEraseCmd;
    CY_ASSERT_L1(NULL != cmdErase);

    result = Cy_SMIF_TransmitCommand(base,
                                     cmdErase->cmdCtl.cmd,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     cmdErase->cmdCtl.width,
                                     cmdErase->cmdCtl.dataRate,
                                     CY_SMIF_CMD_WITHOUT_PARAM,
                                     CY_SMIF_CMD_WITHOUT_PARAM,
                                     (cy_en_smif_txfr_width_t) CY_SMIF_CMD_WITHOUT_PARAM,
                                     CY_SMIF_SDR, 
                                     memDevice->slaveSelect,
                                     LAST_COMMAND_BYTE, context);

    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdSectorErase
****************************************************************************//**
*
* This function performs a block Erase of the external memory. The Write Enable
* command is called before this API.
*
* \note This function uses the low-level Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in a blocking mode. In the dual quad mode,
* this API is called for each memory.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param sectorAddr
* The sector address to erase.
*
* \param context
* The internal SMIF context data. \ref cy_stc_smif_context_t
*
* \return A status of the command transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdSectorErase(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t *memDevice,
                                        uint32_t sectorAddr,
                                        cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;
    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;

    uint8_t addrSizeInByte;
    uint32_t addrInBigEndian;
    // For now supporting for Address bit more than 3 byte.
    if(device->numOfAddrBytes > CY_SMIF_XIP_ADDRESS_3_BYTE)
    {
        addrSizeInByte = 4ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(sectorAddr);
    }
    else
    {
        addrSizeInByte = 3ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(sectorAddr << 8ul);
    }

    cy_stc_smif_mem_cmd_t *cmdErase = memDevice->deviceCfg->eraseCmd;
    CY_ASSERT_L1(NULL != cmdErase);

    result = Cy_SMIF_TransmitCommand(base,
                                     cmdErase->cmdCtl.cmd,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     cmdErase->cmdCtl.width,
                                     cmdErase->cmdCtl.dataRate,
                                     (uint8_t*)&addrInBigEndian,
                                     addrSizeInByte,
                                     cmdErase->cmdCtl.width,
                                     cmdErase->cmdCtl.dataRate,
                                     memDevice->slaveSelect,
                                     LAST_COMMAND_BYTE,
                                     context);

    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdProgram
****************************************************************************//**
*
* This function performs the Program operation. 
*
* \note This function uses the  Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in the blocking mode. In the dual quad mode,
* this API works with both types of memory simultaneously.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param addr
* The address to program.
*
* \param writeBuff
* The pointer to the data to program. If this pointer is a NULL, then the
* function does not enable the interrupt. This use case is  typically used when
* the FIFO is handled outside the interrupt and is managed in either a 
* polling-based code or a DMA. The user would handle the FIFO management 
* in a DMA or a polling-based code. 
* If the user provides a NULL pointer in this function and does not handle 
* the FIFO transaction, this could either stall or timeout the operation 
* \ref Cy_SMIF_TransmitData().
*
* \param size
* The size of data to program. The user must ensure that the data size
* does not exceed the page size.
*
* \param cmdCmpltCb
* The callback function to call after the transfer completion. NULL interpreted
* as no callback.
*
* \param context
* The internal SMIF context data.
*
* \return A status of a transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdProgram(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    uint32_t deviceAddr,
                                    uint8_t* writeBuff,
                                    uint32_t size,
                                    cy_smif_event_cb_t cmdCmpltCb,
                                    cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result;
    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
    cy_stc_smif_mem_wr_cmd_t *cmdProg = device->programCmd;
    CY_ASSERT_L1(NULL != cmdProg);

    if(writeBuff == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(size > device->programSize)
    {
        return CY_SMIF_BAD_PARAM;
    }

    uint8_t addrSizeInByte;
    uint32_t addrInBigEndian;
    // For now supporting for Address bit more than 3 byte.
    if(device->numOfAddrBytes > CY_SMIF_XIP_ADDRESS_3_BYTE)
    {
        addrSizeInByte = 4ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(deviceAddr);
    }
    else
    {
        addrSizeInByte = 3ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(deviceAddr << 8ul);
    }

    cy_en_smif_slave_select_t slaveSelected;
    slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                                                        (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;
    /* The page program command */
    result = Cy_SMIF_TransmitCommand(base, 
                                     cmdProg->cmdCtl.cmd,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     cmdProg->cmdCtl.width,
                                     cmdProg->cmdCtl.dataRate,
                                     (uint8_t*)&addrInBigEndian,
                                     addrSizeInByte,
                                     cmdProg->addrCtl.width,
                                     cmdProg->addrCtl.dataRate,
                                     slaveSelected,
                                     NOT_LAST_COMMAND,
                                     context);
    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    if(cmdProg->memDummyCtl.dummyCtl.presence != CY_SMIF_NOT_PRESENT)
    {
        result = Cy_SMIF_SendDummyCycles(base,
                                         CY_SMIF_WIDTH_SINGLE,
                                         CY_SMIF_SDR,
                                         cmdProg->memDummyCtl.doubleDm,
                                         cmdProg->memDummyCtl.rwdsInDm,
                                         cmdProg->memDummyCtl.dlpEn,
                                         cmdProg->memDummyCtl.capEn,
                                         cmdProg->memDummyCtl.dummyCtl.numOfDummy,
					 context);
        if(CY_SMIF_SUCCESS == result)
        {
            return result;
        }
    }

    result = Cy_SMIF_TransmitData(base,
                                  writeBuff,
                                  size,
                                  cmdProg->dataCtl.width,
                                  cmdProg->dataCtl.dataRate,
                                  slaveSelected,
                                  cmdCmpltCb,
                                  context);
    return(result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Memslot_CmdRead
****************************************************************************//**
*
* This function performs the Read operation.
*
* \note This function uses the Cy_SMIF_TransmitCommand() API.
* The Cy_SMIF_TransmitCommand() API works in the blocking mode. In the dual quad mode,
* this API works with both types of memory simultaneously.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param memDevice
* The device to which the command is sent.
*
* \param addr
* The address to read.
*
* \param readBuff
* The pointer to the variable where the read data is stored. If this pointer is 
* a NULL, then the function does not enable the interrupt. This use case is 
* typically used when the FIFO is handled outside the interrupt and is managed
* in either a  polling-based code or a DMA. The user would handle the FIFO
* management in a DMA or a polling-based code. 
* If the user provides a NULL pointer in this function and does not handle 
* the FIFO transaction, this could either stall or timeout the operation 
* \ref Cy_SMIF_TransmitData().
*
* \param size
* The size of data to read.
*
* \param cmdCmpltCb
* The callback function to call after the transfer completion. NULL interpreted
* as no callback.
*
* \param context
* The internal SMIF context data.
*
* \return A status of the transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Memslot_CmdRead(cy_stc_smif_reg_t volatile *base,
                                cy_stc_smif_mem_config_t const *memDevice,
                                uint32_t deviceAddr,
                                uint8_t* readBuff,
                                uint32_t size,
                                cy_smif_event_cb_t cmdCmpltCb,
                                cy_stc_smif_context_t *context)
{
    cy_en_smif_status_t result = CY_SMIF_BAD_PARAM;
    cy_en_smif_slave_select_t slaveSelected;
    cy_stc_smif_mem_device_cfg_t *device = memDevice->deviceCfg;
    cy_stc_smif_mem_rd_cmd_t *cmdRead = device->readCmd;

    if (readBuff == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    uint8_t addrSizeInByte;
    uint32_t addrInBigEndian;
    // For now supporting for Address bit more than 3 byte.
    if(device->numOfAddrBytes > CY_SMIF_XIP_ADDRESS_3_BYTE)
    {
        addrSizeInByte = 4ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(deviceAddr);
    }
    else
    {
        addrSizeInByte = 3ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(deviceAddr << 8ul);
    }

    slaveSelected = (0U == memDevice->dualQuadSlots)?  memDevice->slaveSelect :
                               (cy_en_smif_slave_select_t)memDevice->dualQuadSlots;

    result = Cy_SMIF_TransmitCommand(base,
                                     cmdRead->cmdCtl.cmd,
                                     memDevice->deviceCfg->isCommand2Byte,
                                     cmdRead->cmdCtl.width,
                                     cmdRead->cmdCtl.dataRate,
                                     (uint8_t*)&addrInBigEndian,
                                     addrSizeInByte,
                                     cmdRead->addrCtl.width,
                                     cmdRead->addrCtl.dataRate,
                                     slaveSelected,
                                     NOT_LAST_COMMAND,
                                     context);

    if(result != CY_SMIF_SUCCESS)
    {
        return result;
    }

    if(cmdRead->modeCtl.presence != CY_SMIF_NOT_PRESENT)
    {
        result = Cy_SMIF_TransmitCommand(CY_SMIF_DRV_SMIF0_CORE0,
                                         cmdRead->modeCtl.cmd,
                                         memDevice->deviceCfg->isCommand2Byte,
                                         cmdRead->modeCtl.width,   /* command width */
                                         cmdRead->modeCtl.dataRate,/* command rate */
                                         NULL,                 /* Data pointer */
                                         0,                    /* Data size */
                                         CY_SMIF_WIDTH_SINGLE, /* data width */
                                         CY_SMIF_SDR,          /* data rate */
                                         slaveSelected,        /* slave select */
                                         NOT_LAST_COMMAND,
                                         context);
        if(result != CY_SMIF_SUCCESS)
        {
            return result;
        }
    }


    if(cmdRead->memDummyCtl.dummyCtl.presence != CY_SMIF_NOT_PRESENT)
    {
        result = Cy_SMIF_SendDummyCycles(base,
                                         CY_SMIF_WIDTH_SINGLE, 
                                         CY_SMIF_SDR, 
                                         cmdRead->memDummyCtl.doubleDm,
                                         cmdRead->memDummyCtl.rwdsInDm,
                                         cmdRead->memDummyCtl.dlpEn,
                                         cmdRead->memDummyCtl.capEn,
                                         cmdRead->memDummyCtl.dummyCtl.numOfDummy,
					 context);
        if(result != CY_SMIF_SUCCESS)
        {
            return result;
        }
    }

    result = Cy_SMIF_ReceiveData(base, 
                                 readBuff,
                                 size,
                                 cmdRead->dataCtl.width,
                                 cmdRead->dataCtl.dataRate,
                                 slaveSelected,
                                 cmdCmpltCb,
                                 context);

    return(result);
}


#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

/* [] END OF FILE */
