/***************************************************************************//**
* \file cy_smif.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the SMIF driver APIs.
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
#include "cy_project.h"

#ifdef CY_IP_MXSMIF

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*        Function Prototypes
***************************************/
static void Cy_SMIF_PUSH_TX_Command(volatile cy_stc_smif_reg_t *base, cy_un_smif_cmd_fifo_t cmdData, cy_stc_smif_context_t *ctx);

/*******************************************************************************
* Function Name: Cy_SMIF_GetMode
****************************************************************************//**
*
* Reads the mode of operation for the SMIF. The mode of operation can be the
* XIP mode where the slave devices are mapped as memories and are directly
* accessed from the TVII register map. In the MMIO mode, the SMIF block acts as
* a simple SPI engine.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return The mode of SMIF operation (see \ref cy_en_smif_mode_t).
*
*******************************************************************************/
cy_en_smif_mode_t Cy_SMIF_GetMode(volatile cy_stc_smif_reg_t *base)
{
    return (cy_en_smif_mode_t)(base->unCTL.stcField.u1XIP_MODE);
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceSetDataSelect
****************************************************************************//**
*
* This function set the connection of the
* IP's data lines (spi_data[0], c, spi_data[7]) to the
* device's data lines (SI/IO0, SO/IO1, IO2, IO3, IO4, IO5, IO6, IO7)
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param sel (see \cy_en_smif_data_select_t)
*
*******************************************************************************/
void Cy_SMIF_DeviceSetDataSelect(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_data_select_t sel)
{
    device->unCTL.stcField.u2DATA_SEL  = sel;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceEnable
****************************************************************************//**
*
* This function enables input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceEnable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1ENABLED = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceDisable
****************************************************************************//**
*
* This function disables input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceDisable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1ENABLED = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceWriteEnable
****************************************************************************//**
*
* This function enables writing for input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceWriteEnable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1WR_EN = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceWriteDisable
****************************************************************************//**
*
* This function disables writing for input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceWriteDisable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1WR_EN = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceCryptoEnable
****************************************************************************//**
*
* This function enables on the fly cryptography for input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceCryptoEnable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1CRYPTO_EN = 1;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceCryptoDisable
****************************************************************************//**
*
* This function disables on the fly cryptography for input SMIF Device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceCryptoDisable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1CRYPTO_EN = 0;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceTransferMergingEnable
****************************************************************************//**
*
* This function enables transfer merging, and sets merge timeout.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param timeout (see \cy_en_smif_dev_merge_timeout_t)
*
*******************************************************************************/
void Cy_SMIF_DeviceTransferMergingEnable(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_dev_merge_timeout_t timeout)
{
    cy_un_smif_reg_device_ctl_t temp;
    
    temp.u32Register              = device->unCTL.u32Register;
    temp.stcField.u3MERGE_TIMEOUT = timeout;
    temp.stcField.u1MERGE_EN      = 1u;
    device->unCTL.u32Register     = temp.u32Register;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceTransferMergingDisable
****************************************************************************//**
*
* This function disables transfer merging.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceTransferMergingDisable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1MERGE_EN = 0u;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceTotalTimeoutEnable
****************************************************************************//**
*
* This function enables total timeout of input device, and sets total timeout
* value.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param timeout
* Timeout clock number
*
*******************************************************************************/
void Cy_SMIF_DeviceTotalTimeoutEnable(volatile cy_stc_smif_reg_device_t *device, uint16_t timeout)
{
    cy_un_smif_reg_device_ctl_t temp;
    
    temp.u32Register                 = device->unCTL.u32Register;
    temp.stcField.u14TOTAL_TIMEOUT   = (timeout & 0x3FFF);
    temp.stcField.u1TOTAL_TIMEOUT_EN = 1u;
    device->unCTL.u32Register        = temp.u32Register;
    
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_DeviceTotalTimeoutDisable
****************************************************************************//**
*
* This function disables total timeout of input device.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
*******************************************************************************/
void Cy_SMIF_DeviceTotalTimeoutDisable(volatile cy_stc_smif_reg_device_t *device)
{
    device->unCTL.stcField.u1TOTAL_TIMEOUT_EN = 0u;
  
    /* The void Read of the CTL register to handle buffering */
    (void)device->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetReadStatus
****************************************************************************//**
*
* This function returns functional safety status register.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \return Functional Safety Status Register of the memory 
*         received with the last read transfer.
*
*******************************************************************************/
uint8_t Cy_SMIF_GetReadStatus(volatile cy_stc_smif_reg_device_t *device)
{
    return (uint8_t)(device->unRD_STATUS.stcField.u8FS_STATUS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetXIPBaseAddress
****************************************************************************//**
*
* This function sets base address in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param addr
* Base address which will be used in XIP mode
*
*******************************************************************************/
void Cy_SMIF_SetXIPBaseAddress(volatile cy_stc_smif_reg_device_t *device, uint32_t addr)
{
    device->unADDR.u32Register = (addr & CY_SMIF_DRV_DEVICE_ADDR_MSK);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetXIPAddressSize
****************************************************************************//**
*
* This function sets size of address in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param size
* Address size which will be used in XIP mode (see \cy_en_device_size_t).
*
*******************************************************************************/
void Cy_SMIF_SetXIPAddressSize(volatile cy_stc_smif_reg_device_t *device, cy_en_device_size_t size)
{
    device->unMASK.u32Register = (uint32_t)size;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetXIPAddrCtl
****************************************************************************//**
*
* This function sets the size of the XIP device address and if the AHB-Lite
* bus transfer address is divided by 2.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param addrSize
* (see \cy_en_smif_xip_addr_byte_t).
*
* \param divForDualQuad
* true: dual quad mode setting
* faluse: not dual quad mode setting
*
*******************************************************************************/
void Cy_SMIF_SetXIPAddrCtl(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_xip_addr_byte_t addrSize, bool divForDualQuad)
{
    cy_un_smif_reg_device_addr_ctl_t tmp_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmp_ADDR_CTL.stcField.u3SIZE3  = addrSize;
    tmp_ADDR_CTL.stcField.u1DIV2   = divForDualQuad;
    device->unADDR_CTL.u32Register = tmp_ADDR_CTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadCmdCtl
****************************************************************************//**
*
* This function controls read command in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_cmd_mode_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadCmdCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
{
    cy_un_smif_reg_device_rd_cmd_ctl_t tmp_RD_CMD_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_CMD_CTL.au16Halfword[0]     = config->cmd;
    tmp_RD_CMD_CTL.stcField.u2WIDTH    = config->width;
    tmp_RD_CMD_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_RD_CMD_CTL.stcField.u2PRESENT2 = config->presence;
    device->unRD_CMD_CTL.u32Register   = tmp_RD_CMD_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteCmdCtl
****************************************************************************//**
*
* This function controls write command in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_cmd_mode_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteCmdCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
{
    cy_un_smif_reg_device_wr_cmd_t tmp_WR_CMD_CTL = { .u32Register = 0x00000000ul };
    tmp_WR_CMD_CTL.au16Halfword[0]     = config->cmd;
    tmp_WR_CMD_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_CMD_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_WR_CMD_CTL.stcField.u2PRESENT2 = config->presence;
    device->unWR_CMD_CTL.u32Register   = tmp_WR_CMD_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadAddrCtl
****************************************************************************//**
*
* This function controls read address in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_addr_data_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadAddrCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t* config)
{
    cy_un_smif_reg_device_rd_addr_ctl_t tmp_RD_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmp_RD_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unRD_ADDR_CTL.u32Register   = tmp_RD_ADDR_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteAddrCtl
****************************************************************************//**
*
* This function controls write address in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_addr_data_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteAddrCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t* config)
{
    cy_un_smif_reg_device_wr_addr_t tmp_WR_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmp_WR_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unWR_ADDR_CTL.u32Register   = tmp_WR_ADDR_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadModeCtl
****************************************************************************//**
*
* This function controls read mode in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_cmd_mode_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadModeCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
{
    cy_un_smif_reg_device_rd_mode_ctl_t tmpRD_MODE_CTL = { .u32Register = 0x00000000ul };
    tmpRD_MODE_CTL.au16Halfword[0]     = config->cmd;
    tmpRD_MODE_CTL.stcField.u2WIDTH    = config->width;
    tmpRD_MODE_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmpRD_MODE_CTL.stcField.u2PRESENT2 = config->presence;
    device->unRD_MODE_CTL.u32Register  = tmpRD_MODE_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteModeCtl
****************************************************************************//**
*
* This function controls write mode in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_cmd_mode_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteModeCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t* config)
{
    cy_un_smif_reg_device_wr_mode_ctl_t tmp_WR_MODE_CTL = { .u32Register = 0x00000000ul };
    tmp_WR_MODE_CTL.au16Halfword[0]     = config->cmd;
    tmp_WR_MODE_CTL.stcField.u2WIDTH    = config->width;
    tmp_WR_MODE_CTL.stcField.u1DDR_MODE = config->dataRate;
    tmp_WR_MODE_CTL.stcField.u2PRESENT2 = config->presence;
    device->unWR_MODE_CTL.u32Register   = tmp_WR_MODE_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadDummyCtl
****************************************************************************//**
*
* This function controls read dummy in XIP mode
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_rd_dummy_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadDummyCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_dummy_ctl_t *config)
{
    cy_un_smif_reg_device_rd_dummy_t tmpRD_DUMMY_CTL = { .u32Register = 0x00000000ul };
    if(config->numOfDummy == 0)
    {
        tmpRD_DUMMY_CTL.stcField.u5SIZE5    = 0;
        tmpRD_DUMMY_CTL.stcField.u2PRESENT2 = CY_SMIF_NOT_PRESENT;
      }
    else
    {
        tmpRD_DUMMY_CTL.stcField.u5SIZE5    = config->numOfDummy - 1u;
        tmpRD_DUMMY_CTL.stcField.u2PRESENT2 = config->presence;
    }
    
    device->unRD_DUMMY_CTL.u32Register  = tmpRD_DUMMY_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteDummyCtl
****************************************************************************//**
*
* This function controls write dummy in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_wr_dummy_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteDummyCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_wr_dummy_ctl_t* config)
{
    cy_un_smif_reg_device_wr_dummy_ctl_t tmpWR_DUMMY_CTL = { .u32Register = 0x00000000ul };
    tmpWR_DUMMY_CTL.stcField.u1RWDS_EN  = config->rwdsEn;
    if(config->numOfDummy == 0)
    {
        tmpWR_DUMMY_CTL.stcField.u5SIZE5    = 0u;
        tmpWR_DUMMY_CTL.stcField.u2PRESENT2 = CY_SMIF_NOT_PRESENT;
    }
    else
    {
        tmpWR_DUMMY_CTL.stcField.u5SIZE5    = config->numOfDummy - 1ul;
        tmpWR_DUMMY_CTL.stcField.u2PRESENT2 = config->presence;
    }
    
    device->unWR_DUMMY_CTL.u32Register   = tmpWR_DUMMY_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadDataCtl
****************************************************************************//**
*
* This function controls read data in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_addr_data_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadDataCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t* config)
{
    cy_un_smif_reg_device_rd_data_ctl_t tmpRD_DATA_CTL = { .u32Register = 0x00000000ul };
    tmpRD_DATA_CTL.stcField.u2WIDTH    = config->width;
    tmpRD_DATA_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unRD_DATA_CTL.u32Register  = tmpRD_DATA_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteDataCtl
****************************************************************************//**
*
* This function controls write data in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_addr_data_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteDataCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t* config)
{
    cy_un_smif_reg_device_wr_data_ctl_t tmpWR_ADDR_CTL = { .u32Register = 0x00000000ul };
    tmpWR_ADDR_CTL.stcField.u2WIDTH    = config->width;
    tmpWR_ADDR_CTL.stcField.u1DDR_MODE = config->dataRate;
    device->unWR_DATA_CTL.u32Register   = tmpWR_ADDR_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadCrcCtl
****************************************************************************//**
*
* This function controls read CRC in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_rd_crc_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadCrcCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_crc_ctl_t *config)
{
    cy_un_smif_reg_device_rd_crc_ctl_t tmp_RD_CRC_CTL = { .u32Register = 0x00000000ul };
    tmp_RD_CRC_CTL.stcField.u8STATUS_CHECK_MASK     = config->funcSafeChecked;
    tmp_RD_CRC_CTL.stcField.u8STATUS_ERROR_POL      = config->errPol;
    tmp_RD_CRC_CTL.stcField.u8DATA_CRC_INPUT_SIZE   = config->inputSize;
    tmp_RD_CRC_CTL.stcField.u2CMD_ADDR_CRC_WIDTH    = config->width;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_DDR_MODE = config->dataRate;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_INPUT    = config->inputScope;
    tmp_RD_CRC_CTL.stcField.u1DATA_CRC_CHECK        = config->crcChecked;
    tmp_RD_CRC_CTL.stcField.u1CMD_ADDR_CRC_PRESENT  = config->cmd_addrCrcPresent;
    tmp_RD_CRC_CTL.stcField.u1DATA_CRC_PRESENT      = config->dataCrcPresent;
    device->unRD_CRC_CTL.u32Register = tmp_RD_CRC_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetWriteCrcCtl
****************************************************************************//**
*
* This function controls write CRC in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_wr_crc_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetWriteCrcCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_wr_crc_ctl_t *config)
{
    cy_un_smif_reg_device_wr_crc_ctl_t tmp_WR_CRC_CTL      = { .u32Register = 0x00000000ul };
    tmp_WR_CRC_CTL.stcField.u8DATA_CRC_INPUT_SIZE   = config->inputSize;
    tmp_WR_CRC_CTL.stcField.u2CMD_ADDR_CRC_WIDTH    = config->width;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_DDR_MODE = config->dataRate;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_INPUT    = config->inputScope;
    tmp_WR_CRC_CTL.stcField.u1CMD_ADDR_CRC_PRESENT  = config->cmd_addrCrcPresent;
    tmp_WR_CRC_CTL.stcField.u1DATA_CRC_PRESENT      = config->dataCrcPresent;
    device->unWR_CRC_CTL.u32Register = tmp_WR_CRC_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetReadBoundCtl
****************************************************************************//**
*
* This function controls read bound in XIP mode.
*
* \param device
* Holds the base address of the SMIF Device registers.
*
* \param config
* (see \stc_cy_smif_rd_bound_ctl_t).
*
*******************************************************************************/
void Cy_SMIF_SetReadBoundCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_bound_ctl_t *config)
{
    cy_un_smif_reg_device_rd_bound_ctl_t tmp_RD_BOUND_CTL  = { .u32Register = 0x00000000ul };
    if(config->numOfCycle == 0)
    {
        tmp_RD_BOUND_CTL.stcField.u5SIZE5           = 0;
        tmp_RD_BOUND_CTL.stcField.u1PRESENT         = false;
    }
    else
    {
        tmp_RD_BOUND_CTL.stcField.u5SIZE5           = config->numOfCycle - 1;
        tmp_RD_BOUND_CTL.stcField.u1PRESENT         = config->bdLPresent;
    }
    tmp_RD_BOUND_CTL.stcField.u2SUB_PAGE_SIZE   = config->subPageSize;
    tmp_RD_BOUND_CTL.stcField.u2SUB_PAGE_NR     = config->subPageNr;
    tmp_RD_BOUND_CTL.stcField.u1SUBSEQ_BOUND_EN = config->subsqBndEn;

    device->unRD_BOUND_CTL.u32Register = tmp_RD_BOUND_CTL.u32Register;
    return;
}

/*******************************************************************************
* Function Name: Cy_SMIF_Disable
****************************************************************************//**
*
* This function disables SMIF.
*
* Disables the operation of the SMIF block. The SMIF block can be disabled only
* when it is not in the active state. Use the Cy_SMIF_IsBusy() API to check
* it before calling this API.
*
* \param base
* Holds the base address of the SMIF block registers.
*
*******************************************************************************/
void Cy_SMIF_Disable(volatile cy_stc_smif_reg_t *base)
{
    base->unCTL.stcField.u1ENABLED = 0u;

    /* The void Read of the CTL register to handle buffering */
    (void) base->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetInterruptMask
****************************************************************************//**
*
* This function is used to set an interrupt mask for the SMIF Interrupt.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param interrupt
* This is the mask for different source options that can be masked. See
* \ref group_smif_macros_isr "Interrupt Macros" for possible values.
*
*******************************************************************************/
void Cy_SMIF_SetInterruptMask(volatile cy_stc_smif_reg_t *base, uint32_t interrupt)
{
    base->unINTR_MASK.u32Register = interrupt;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetInterruptMask
****************************************************************************//**
*
* This function is used to read an interrupt mask for the SMIF Interrupt.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns the mask set for the SMIF interrupt.
*
*******************************************************************************/
uint32_t Cy_SMIF_GetInterruptMask(volatile cy_stc_smif_reg_t *base)
{
    return (base->unINTR_MASK.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetInterruptStatusMasked
****************************************************************************//**
*
* This function is used to read an active masked interrupt. This function can
* be used in the interrupt service-routine to find which source triggered the
* interrupt.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns a word with bits set at positions corresponding to the
* interrupts triggered in the system.
*
*******************************************************************************/
uint32_t Cy_SMIF_GetInterruptStatusMasked(volatile cy_stc_smif_reg_t *base)
{
    return (base->unINTR_MASKED.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetInterruptStatus
****************************************************************************//**
*
* This function is used to read an active interrupt. This status is the unmasked
* result, so will also show interrupts that will not generate active interrupts.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns a word with bits set at positions corresponding to the
* interrupts triggered in the system.
*
*******************************************************************************/
uint32_t Cy_SMIF_GetInterruptStatus(volatile cy_stc_smif_reg_t *base)
{
    return (base->unINTR.u32Register);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetInterrupt
****************************************************************************//**
*
* This function is used to set an interrupt source. This function can be used
* to activate interrupts through the software.
*
* \note Interrupt sources set using this interrupt will generate interrupts only
* if they are not masked.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param interrupt
* An encoded integer with a bit set corresponding to the interrupt to be
* triggered. See \ref group_smif_macros_isr "Interrupt Macros" for possible
* values.
*
*******************************************************************************/
void Cy_SMIF_SetInterrupt(volatile cy_stc_smif_reg_t *base, uint32_t interrupt)
{
    base->unINTR_SET.u32Register = interrupt;
}

/*******************************************************************************
* Function Name: Cy_SMIF_ClearInterrupt
****************************************************************************//**
*
* This function is used to clear an interrupt source. This function can be used
* in the user code to clear all pending interrupts.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param interrupt
* An encoded integer with a bit set corresponding to the interrupt that must
* be cleared. See \ref group_smif_macros_isr "Interrupt Macros" for possible
* values.
*
*******************************************************************************/
void  Cy_SMIF_ClearInterrupt(volatile cy_stc_smif_reg_t *base, uint32_t interrupt)
{
    base->unINTR.u32Register = interrupt;
    /* Ensure that the initial Write has been flushed out to the hardware. */
    interrupt = base->unINTR.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetRxFifoTriggerLevel()
****************************************************************************//**
*
* This function is used to set a trigger level for the RX FIFO. This value must
* be an integer between 0 and 7. For the normal mode only.
* The triggering is active when RX_DATA_FIFOSTATUS > level.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param level
* The trigger level to set(0-8).
*
*******************************************************************************/
void Cy_SMIF_SetRxFifoTriggerLevel(volatile cy_stc_smif_reg_t *base, uint32_t level)
{
    CY_ASSERT_L2(level < CY_SMIF_MAX_RX_TR_LEVEL);
    base->unRX_DATA_MMIO_FIFO_CTL.stcField.u3RX_TRIGGER_LEVEL = level;
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetCmdFifoStatus()
****************************************************************************//**
*
* This function is used to read the status of the CMD FIFO.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns the number of the entries in the CMD FIFO.
*
*******************************************************************************/
uint32_t Cy_SMIF_GetCmdFifoStatus(volatile cy_stc_smif_reg_t *base)
{
    return (base->unTX_CMD_FIFO_STATUS.stcField.u4USED4);
}

/*******************************************************************************
* Function Name: Cy_SMIF_GetRxFifoStatus()
****************************************************************************//**
*
* This function is used to read the status of the RX FIFO.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns the number of the entries in the RX FIFO.
*
*******************************************************************************/
uint32_t Cy_SMIF_GetRxFifoStatus(volatile cy_stc_smif_reg_t *base)
{
    return (base->unRX_DATA_MMIO_FIFO_STATUS.stcField.u4USED4);
}

/*******************************************************************************
* Function Name: Cy_SMIF_IsBusy
****************************************************************************//**
*
* This function provides the status of the IP block (False - not busy,
* True - busy).
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \return Returns an IP block status.
*
*******************************************************************************/
bool Cy_SMIF_IsBusy(volatile cy_stc_smif_reg_t *base)
{
    return (bool)(base->unSTATUS.stcField.u1BUSY);
}

/*******************************************************************************
* Function Name: Cy_SMIF_Interrupt
****************************************************************************//**
*
* The Interrupt Service Routine for the SMIF. The interrupt code will be
* responsible for the FIFO operations on FIFO interrupts during ongoing transfers.
* The user must place a call to this interrupt function in the interrupt
* routine corresponding to the interrupt attached to the SMIF. If the
* user does not do this, will break: the functionality of all the API functions in
* the SMIF driver that use SMIF interrupts to affect transfers.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \globalvars
*  - context->txBufferAddress - The pointer to the data to be transferred.
*
*  - context->txBufferCounter - The number of data entries left to be transferred.
*
* All the Global variables described above are used when the Software Buffer is
* used.
*
*******************************************************************************/
void Cy_SMIF_Interrupt(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context)
{
    uint32_t interruptStatus = Cy_SMIF_GetInterruptStatusMasked(base);
    
    /* Check which interrupt occurred */
    if ((interruptStatus & CY_SMIF_DRV_INTR_TR_TX_REQ_MSK) != 0ul)
    {
        /* Send data */
        Cy_SMIF_PushTxFifo(base, context);
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_TR_TX_REQ_MSK);
    }
    else if ((interruptStatus & CY_SMIF_DRV_INTR_TR_RX_REQ_MSK) != 0ul)
    {
        /* Receive data */
        Cy_SMIF_PopRxFifo(base, context);
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_TR_RX_REQ_MSK);
    }
    else if ((interruptStatus & CY_SMIF_DRV_INTR_XIP_ALIGNMENT_ERROR_MSK) != 0ul)
    {
        /* An XIP alignment error */
        context->transferStatus = CY_SMIF_XIP_ERROR;
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_XIP_ALIGNMENT_ERROR_MSK);
    }
    else if ((interruptStatus & CY_SMIF_DRV_INTR_TX_CMD_FIFO_OVERFLOW_MSK) != 0ul)
    {
        /* TX CMD FIFO overflow */
        context->transferStatus = CY_SMIF_CMD_ERROR;
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_TX_CMD_FIFO_OVERFLOW_MSK);
    }
    else if ((interruptStatus & CY_SMIF_DRV_INTR_TX_DATA_FIFO_OVERFLOW_MSK) != 0ul)
    {
        /* A TX DATA FIFO overflow */
        context->transferStatus = CY_SMIF_TX_ERROR;
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_TX_DATA_FIFO_OVERFLOW_MSK);
    }
    else if ((interruptStatus & CY_SMIF_DRV_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_MSK) != 0ul)
    {
        /* RX DATA FIFO underflow */
        context->transferStatus = CY_SMIF_RX_ERROR;
        
        Cy_SMIF_ClearInterrupt(base, CY_SMIF_DRV_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_MSK);
    }
    else
    {
        /* Processing of errors */
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_PopRxFifo
***************************************************************************//***
*
* \internal
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* This function reads data from the RX FIFO SMIF buffer by 4, 2, or 1 bytes
* based on the data availability in the RX FIFO and amount of bytes to be
* received.
*
*******************************************************************************/
void Cy_SMIF_PopRxFifo(volatile cy_stc_smif_reg_t *baseaddr, cy_stc_smif_context_t *context)
{
    /* The variable that shows which is smaller: the free FIFO size or amount of bytes to be received */
    uint32_t loadedFifoBytes = Cy_SMIF_GetRxFifoStatus(baseaddr);
    uint32_t buffCounter     = context->rxBufferCounter;
    /* readBytes = MIN(loadedFifoBytes, buffCounter) */
    uint32_t readBytes       = (loadedFifoBytes > buffCounter)? buffCounter: loadedFifoBytes;
    
    /* Check that after a FIFO Read, no new data is available */
    uint8_t *buff            = (uint8_t*) context->rxBufferAddress;
    while(readBytes != 0U)
    {
        if(readBytes == 1u)
        {
            buff[0U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
        }
        else if(readBytes == 2u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[0U], false);
        }
        else if(readBytes == 3u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[0U], false);
            buff[2U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
        }
        else if(readBytes == 4u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
        }
        else if(readBytes == 5u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
            buff[4U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
        }
        else if(readBytes == 6u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[4U], false);
        }
        else if(readBytes == 7u)
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD2.u32Register, &buff[4U], false);
            buff[6U] = (uint8_t)baseaddr->unRX_DATA_MMIO_FIFO_RD1.u32Register;
        }
        else /* readBytes => 8*/
        {
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[0U], true);
            Cy_SMIF_UnPackByteArray(baseaddr->unRX_DATA_MMIO_FIFO_RD4.u32Register, &buff[4U], true);
            readBytes = 8u;
        }
        
        buff            = &buff[readBytes];
        buffCounter    -= readBytes;
        /* Check if we already got new data in RX_FIFO*/
        loadedFifoBytes = Cy_SMIF_GetRxFifoStatus(baseaddr);
        readBytes       = (loadedFifoBytes > buffCounter)? buffCounter: loadedFifoBytes;
    }
    
    /* Save changes in the context */
    context->rxBufferAddress = buff;
    context->rxBufferCounter = buffCounter;
    
    /* Check if all bytes are received */
    if (buffCounter == 0UL)
    {
        /* Disable the TR_RX_REQ interrupt */
        baseaddr->unINTR_MASK.stcField.u1TR_RX_REQ = 0u;
        
        context->transferStatus = CY_SMIF_REC_CMPLT;
        if (NULL != context->rxCmpltCb)
        {
            context->rxCmpltCb((uint32_t) CY_SMIF_REC_CMPLT);
        }
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_CheckRxFifo
***************************************************************************//***
*
* This function reads RX FIFO without shifting data.
*
* \param baseaddr
* Holds the base address of the SMIF block registers.
*
* \return RX FIFO DATA
*
*
*******************************************************************************/
uint8_t Cy_SMIF_CheckRxFifoContents(volatile cy_stc_smif_reg_t *baseaddr)
{
    return baseaddr->unRX_DATA_MMIO_FIFO_RD1_SILENT.stcField.u8DATA0;
}

/*******************************************************************************
* Function Name: Cy_SMIF_PackBytesArray
***************************************************************************//***
*
* \internal
*
* This function packs 0-numBytes of the buff byte array into a 4-byte value.
*
* \param buff
* The byte array to pack.
*
* \param fourBytes
*  - The True pack is for a 32-bit value.
*  - The False pack is for a 16-bit value.
*
* \return
*  The 4-byte value packed from the byte array.
*
*******************************************************************************/
uint32_t Cy_SMIF_PackBytesArray(uint8_t const buff[], bool fourBytes)
{
    uint32_t result = 0UL;
    
    result = ((uint32_t)buff[1UL] << 8UL) | (uint32_t)buff[0UL];
    
    if(fourBytes)
    {
        result |= ((uint32_t)buff[3UL] << 24UL) | ((uint32_t)buff[2UL] << 16UL);
    }
    
    return result;
}

/*******************************************************************************
* Function Name: Cy_SMIF_UnPackByteArray
***************************************************************************//***
*
* \internal
*
* This function unpacks 0-numBytes from a 4-byte value into the byte array outBuff.
*
* \param smifReg
*  The 4-byte value to unpack.
*
* \param outBuff
* The byte array to fill.
*
* \param fourBytes
*  - The True unpack is for a 32-bit value.
*  - The False unpack is for a 16-bit value.
*
*******************************************************************************/
void Cy_SMIF_UnPackByteArray(uint32_t inValue, uint8_t outBuff[], bool fourBytes)
{
    outBuff[0UL] = (uint8_t)(inValue & 0xFFUL);
    outBuff[1UL] = (uint8_t)((inValue >> 8UL ) & 0xFFUL);
    
    if(fourBytes)
    {
        outBuff[2UL] = (uint8_t)((inValue >> 16UL) & 0xFFUL);
        outBuff[3UL] = (uint8_t)((inValue >> 24UL) & 0xFFUL);
    }
}

/*******************************************************************************
* Function Name: Cy_SMIF_TimeoutRun
****************************************************************************//**
*
* \internal
*
* This function checks if the timeout is expired. Use the Cy_SysLib_DelayUs() function for
* implementation.
*
* \param timeoutUnits
*  The pointer to the timeout. The timeout measured in microseconds is multiplied by
*  CY_SMIF_WAIT_1_UNIT.
*
* \return
* A timeout status:
*     - \ref CY_SMIF_SUCCESS - The timeout has not expired or input timeoutUnits is 0.
*     - \ref CY_SMIF_EXCEED_TIMEOUT - The timeout has expired.
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TimeoutRun(uint32_t *timeoutUnits)
{
    cy_en_smif_status_t status = CY_SMIF_SUCCESS;
    if (*timeoutUnits > 0u)
    {
        Cy_SysLib_DelayUs(CY_SMIF_WAIT_1_UNIT);
        --(*timeoutUnits);
        status = (0u == (*timeoutUnits))? CY_SMIF_EXCEED_TIMEOUT: CY_SMIF_SUCCESS;
    }
    return status;
}

/*******************************************************************************
* Function Name: CY_SMIF_GetDeviceNumber
****************************************************************************//**
*
* This function returns device number (quantity) which the SMIF IP has. 
* User can input both SMIF DEVICE block address and SMIF block address
*
* \param base
* Base address of the SMIF block or the SMIF DEVICE block.
*
* \ return device number of the SMIF IP.
*
*******************************************************************************/
uint32_t CY_SMIF_GetDeviceNumber(volatile void *base)
{
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF0_DEVICE_NR);
    }
#if defined(CY_SMIF_DRV_SMIF0_CORE1)
    else if((base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF1_DEVICE_NR);
    }
#endif
    else
    {
        CY_ASSERT_L2(false);
    #ifdef NDEBUG // CY_ASSERT will be removed, therefore a "return" is needed which would could otherwise cause a "statement unreachable" warning
        return 0;
    #endif
    }
}

/*******************************************************************************
* Function Name: CY_SMIF_GetDelayTapsNumber
****************************************************************************//**
*
* This function returns tap number which the SMIF IP has. 
* User can input both SMIF DEVICE block address and SMIF block address
*
* \param base
* Base address of the SMIF block or the SMIF DEVICE block.
*
* \return Delay tap Number of the SMIF IP
*
*******************************************************************************/
uint32_t CY_SMIF_GetDelayTapsNumber(volatile void *base)
{
#if defined (CY_SMIF_DRV_SMIF0_DELAY_TAPS_NR)
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return CY_SMIF_DRV_SMIF0_DELAY_TAPS_NR;
    }
#endif

#if defined (CY_SMIF_DRV_SMIF1_DELAY_TAPS_NR)
    else if((base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return CY_SMIF_DRV_SMIF1_DELAY_TAPS_NR;
    }
#endif

#if defined (CY_SMIF_DLL_TAP_MAX)
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1)
        || (base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return CY_SMIF_DLL_TAP_MAX;
    }
#endif

    // Reaching here may mean that DLP calibration is available. Please use that.
    // Or means input bad parameter
    CY_ASSERT_L2(false);
#ifdef NDEBUG // CY_ASSERT will be removed, therefore a "return" is needed which would could otherwise cause a "statement unreachable" warning
    return 0;
#endif
}

/*******************************************************************************
* Function Name: CY_SMIF_GetDelayLinesNumber
****************************************************************************//**
*
* This function returns the number of delay lines which the SMIF IP has. 
* User can input both SMIF DEVICE block address and SMIF block address
*
* \param base
* Base address of the SMIF block or the SMIF DEVICE block.
*
* \return Number of delay lines of the SMIF IP
*
*******************************************************************************/
uint32_t CY_SMIF_GetDelayLinesNumber(volatile void *base)
{
#if defined (CY_SMIF_DRV_SMIF0_DELAY_LINES_NR)
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF0_DELAY_LINES_NR);
    }
#endif
#if defined(CY_SMIF_DRV_SMIF1_DELAY_LINES_NR)
    else if((base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF1_DELAY_LINES_NR);
    }
#endif

    // Reaching here may mean that DLP calibration is available. Please use that.
    // Or means input bad parameter
    CY_ASSERT_L2(false);    
#ifdef NDEBUG // CY_ASSERT will be removed, therefore a "return" is needed which would could otherwise cause a "statement unreachable" warning
    return 0;
#endif
}

/*******************************************************************************
* Function Name: CY_SMIF_GetXIP_Mask
****************************************************************************//**
*
* This function returns XIP mask value which the SMIF IP has. 
* User can input both SMIF DEVICE block address and SMIF block address
*
* \param base
* Base address of the SMIF block or the SMIF DEVICE block.
*
* \return XIP address mask value of the SMIF IP
*
*******************************************************************************/
uint32_t CY_SMIF_GetXIP_Mask(volatile void *base)
{
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF0_SMIF_XIP_MASK);
    }
#if defined(CY_SMIF_DRV_SMIF1_SMIF_XIP_MASK)
    else if((base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF1_SMIF_XIP_MASK);
    }
#endif
    else
    {
        CY_ASSERT_L2(false);
    #ifdef NDEBUG // CY_ASSERT will be removed, therefore a "return" is needed which would could otherwise cause a "statement unreachable" warning
        return 0;
    #endif
    }
}

/*******************************************************************************
* Function Name: CY_SMIF_GetXIP_Address
****************************************************************************//**
*
* This function returns XIP address which the SMIF IP has. 
* User can input both SMIF DEVICE block address and SMIF block address
*
* \param base
* Base address of the SMIF block or the SMIF DEVICE block.
*
* \return XIP address of the SMIF IP
*
*******************************************************************************/
uint32_t CY_SMIF_GetXIP_Address(volatile void *base)
{
    if((base == CY_SMIF_DRV_SMIF0_CORE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE0_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF0_SMIF_XIP_ADDR);
    }
#if defined(CY_SMIF_DRV_SMIF1_SMIF_XIP_ADDR)
    else if((base == CY_SMIF_DRV_SMIF0_CORE1) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE0) || (base == CY_SMIF_DRV_SMIF0_CORE1_DEVICE1))
    {
        return (CY_SMIF_DRV_SMIF1_SMIF_XIP_ADDR);
    }
#endif
    else
    {
        CY_ASSERT_L2(false);
    #ifdef NDEBUG // CY_ASSERT will be removed, therefore a "return" is needed which would could otherwise cause a "statement unreachable" warning
        return 0;
    #endif
    }
}

/*******************************************************************************
* Function Name: CY_SMIF_MaskValueToSizeInByte
****************************************************************************//**
*
* This function converts \ref cy_en_device_size_t value to 32 bit value in byte.
*
* \param mask
* \ref cy_en_device_size_t
*
* \return size in byte corresponding to the input
*
*******************************************************************************/
uint32_t Cy_SMIF_MaskValueToSizeInByte(cy_en_device_size_t mask)
{
    return (~((uint32_t)mask) + 1UL);
}

/*******************************************************************************
* Function Name: Cy_SMIF_Reverse4ByteEndian
****************************************************************************//**
*
* This function reverses endianness of 32 bit input
*
* \param in
* input 32 bit data to be reversed its endianness
*
* \return 32 bit data which has been reversed its endianness
*
*******************************************************************************/
uint32_t Cy_SMIF_Reverse4ByteEndian(uint32_t in)
{
    return ((in & 0xFF000000UL) >> 24UL) |
      ((in & 0x00FF0000UL) >> 8UL)  |
        ((in & 0x0000FF00UL) << 8UL)  |
          ((in & 0x000000FFUL) << 24UL);
}

/*******************************************************************************
* Function Name: Cy_SMIF_TransmitCommand()
****************************************************************************//**
*
* This function transmits a command byte followed by a parameter which is
* typically an address field. The transfer is implemented using the TX FIFO.
* This function also asserts the slave select line.
* A command to a memory device generally starts with a command byte
* transmission. This function sets up the slave lines for the rest of the
* command structure. The \ref Cy_SMIF_TransmitCommand is called before \ref
* Cy_SMIF_TransmitData or \ref Cy_SMIF_ReceiveData is called. When enabled, the
* cmpltTxfr parameter in the function will de-assert the slave select line at 
* the end of the function execution.
*
* \note This function blocks until all the command and associated parameters
* have been transmitted over the SMIF block or timeout expire.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cmd
* The command byte to be transmitted.
*
* \param isCommand2Byte
* true: command is 2 byte. false: command is 1 byte.
*
* \param cmdTxfrWidth
* The width of command byte transfer \ref cy_en_smif_txfr_width_t.
*
* \param cmdDataRate
* The data rate of command byte transfer \ref cy_en_smif_data_rate_t.
*
* \param cmdParam
* This is the pointer to an array that has bytes to be transmitted
* after the command byte. Typically, this field has the address bytes
* associated with the memory command.
*
* \param paramSize
* The size of the cmdParam array.
*
* \param paramTxfrWidth
* The width of parameter transfer \ref cy_en_smif_txfr_width_t.
*
* \param paramDataRate
* The data rate of parameter transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param cmpltTxfr
* Specifies if the slave select line must be de-asserted after transferring
* the last byte in the parameter array. Typically, this field is set to 0 when
* this function succeed through \ref Cy_SMIF_TransmitData or \ref
* Cy_SMIF_ReceiveData.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of the command transmit.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*
*******************************************************************************/
cy_en_smif_status_t  Cy_SMIF_TransmitCommand(volatile cy_stc_smif_reg_t *base,
                                uint16_t cmd,
                                bool isCommand2Byte,
                                cy_en_smif_txfr_width_t cmdTxfrWidth,
                                cy_en_smif_data_rate_t  cmdDataRate,
                                uint8_t const cmdParam[],
                                uint32_t paramSize,
                                cy_en_smif_txfr_width_t paramTxfrWidth,
                                cy_en_smif_data_rate_t  paramDataRate,
                                cy_en_smif_slave_select_t  slaveSelect,
                                cy_en_smif_cmd_last_t cmpltTxfr,
                                cy_stc_smif_context_t *context)
{
    /* Check input values */
    // If input parameter size is not zero, but parameter pointer is NULL. then it return with error status
    if((paramSize != 0x0UL) && (cmdParam == NULL))
    {
        return CY_SMIF_BAD_PARAM;
    }


    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    cy_un_smif_cmd_fifo_t cmdData = { .u32 = 0 };

    /* Prepare a cmd fifo data */
    if(isCommand2Byte == true)
    {
        if((cmdTxfrWidth == CY_SMIF_WIDTH_OCTAL) && (cmdDataRate == CY_SMIF_DDR))
        {
            // 2byte for each one command
            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
            cmdData.fieldTx.secondByte    = (uint8_t)((cmd >> 8) & 0x00FF);
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;
        }
        else
        {
            // 1byte for each one command. need to send two command to send a command of 2byte.
            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)((cmd >> 8) & 0x00FF);
            cmdData.fieldTx.secondByte    = 0u;
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = 0;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;

            cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
            cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
            cmdData.fieldTx.secondByte    = 0u;
            cmdData.fieldTx.width         = cmdTxfrWidth;
            cmdData.fieldTx.dataRate      = cmdDataRate;
            cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
            cmdData.fieldTx.selectDev     = slaveSelect;
            /* Send the command byte */
            Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
            cmdData.u32 = 0x0ul;
        }
    }
    else
    {
        cmdData.fieldTx.cmd           = CY_SMIF_CMD_FIFO_TX_MODE;
        cmdData.fieldTx.firstByte     = (uint8_t)(cmd & 0x00FF);
        cmdData.fieldTx.secondByte    = 0u;
        cmdData.fieldTx.width         = cmdTxfrWidth;
        cmdData.fieldTx.dataRate      = cmdDataRate;
        cmdData.fieldTx.last          = (0UL == paramSize) ? cmpltTxfr : 0UL;
        cmdData.fieldTx.selectDev     = slaveSelect;
        /* Send the command byte */
        Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
        cmdData.u32 = 0x0ul;
    };


    /* Send the command parameters (usually address) in the blocking mode */
    uint8_t bufIndex           = 0U;
    uint32_t timeoutUnits      = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
    cmdData.fieldTx.cmd        = CY_SMIF_CMD_FIFO_TX_MODE;
    cmdData.fieldTx.selectDev  = slaveSelect;

    if((paramTxfrWidth == CY_SMIF_WIDTH_OCTAL) && (paramDataRate == CY_SMIF_DDR))
    {
        // 2 byte transmission for each one command.
        while ((bufIndex < paramSize) && (CY_SMIF_EXCEED_TIMEOUT != result))
        {
            /* Check if there is at least one free entry in TX_CMD_FIFO */
            if  (Cy_SMIF_GetCmdFifoStatus(base) < CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
            {
                cmdData.fieldTx.firstByte = cmdParam[bufIndex+1];
                cmdData.fieldTx.secondByte= cmdParam[bufIndex];
                cmdData.fieldTx.width     = paramTxfrWidth;
                cmdData.fieldTx.dataRate  = paramDataRate;
                cmdData.fieldTx.last      = ((((uint32_t)bufIndex + 2UL) < paramSize) ? 0UL : cmpltTxfr);
                Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
                bufIndex += 2;
            }
            result = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }
    }
    else
    {
        while ((bufIndex < paramSize) && (CY_SMIF_EXCEED_TIMEOUT != result))
        {
            /* Check if there is at least one free entry in TX_CMD_FIFO */
            if  (Cy_SMIF_GetCmdFifoStatus(base) < CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
            {
                cmdData.fieldTx.firstByte = cmdParam[bufIndex];
                cmdData.fieldTx.secondByte= 0x00u;
                cmdData.fieldTx.width     = paramTxfrWidth;
                cmdData.fieldTx.dataRate  = paramDataRate;
                cmdData.fieldTx.last      = ((((uint32_t)bufIndex + 1UL) < paramSize) ? 0UL : cmpltTxfr);
                Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
                bufIndex++;
            }
            result = Cy_SMIF_TimeoutRun(&timeoutUnits);
        }
    }

    return (result);
}

/*******************************************************************************
* Function Name: Cy_SMIF_TransmitData
****************************************************************************//**
*
* This function is used to transmit data using the SMIF interface. This
* function uses the TX Data FIFO to implement the transmit functionality. The
* function sets up an interrupt to trigger the TX Data FIFO and uses that
* interrupt to fill the TX Data FIFO until all the data is transmitted. At the
* end of the transmission, the TxCmpltCb is executed.
*
* \note  This function is to be preceded by \ref Cy_SMIF_TransmitCommand where
* the slave select is selected. The slave is de-asserted at the end of a
* transmit. The function triggers the transfer and the transfer itself utilizes
* the interrupt for FIFO operations in the background. Thus, frequent
* interrupts will be executed after this function is triggered.
* Since this API is non-blocking and sets up the interrupt to act on the data
* FIFO, ensure there will be no another instance of the function called
* before the current instance has completed execution.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param txBuffer
* The pointer to the data to be transferred. If this pointer is a NULL, then the
* function does not enable the interrupt. This use case is typically used when 
* the FIFO is handled outside the interrupt and is managed in either a 
* polling-based code or a DMA. The user would handle the FIFO management in a
* DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
* 
* \param size
* The size of txBuffer. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataDataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param TxCmpltCb
* The callback executed at the end of a transmission. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TransmitData(volatile cy_stc_smif_reg_t *base,
                            const uint8_t* txBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataDataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t TxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t trUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(txBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataDataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        trUnitNum = size / 2;
    }
    else
    {
        trUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  = 
    {
        .fieldTxCount.cmd       = CY_SMIF_CMD_FIFO_TX_COUNT_MODE,
        .fieldTxCount.width     = (uint32_t)transferWidth,
        .fieldTxCount.dataNum   = (uint32_t)(trUnitNum - 1U),
        .fieldTxCount.dataRate  = (uint32_t)dataDataRate,
        .fieldTxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldTxCount.selectDev = slaveSelect,
    };

    /* Move the parameters to the global variables */
    context->txBufferAddress       = (uint8_t*)txBuffer;
    context->txBufferCounter       = size;
    context->txCmpltCb             = TxCmpltCb;
    context->transferStatus        = CY_SMIF_SEND_BUSY;
    context->preCmdDataRate        = dataDataRate;
    context->preCmdWidth           = transferWidth;

    /* Enter the transmitting mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);
    /* Enable the TR_TX_REQ interrupt */
    base->unINTR_MASK.stcField.u1TR_TX_REQ = 1u;

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_TransmitDataBlocking
****************************************************************************//**
*
* This function implements the transmit data phase in the memory command. The
* data is transmitted using the Tx Data FIFO and the TX_COUNT command. This
* function blocks until completion. The function does not use the interrupts and
* will use CPU to monitor the FIFO status and move data accordingly. The
* function returns only on completion.
*
* \note  Since this API is blocking, ensure that other transfers finished and it
* will not be called during non-blocking transfer.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param txBuffer
* The pointer to the data to be transferred. If this pointer is a NULL, then the
* function does not fill TX_FIFO. The user would handle the FIFO management in a
* DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer
* valid.
*
* \param size
* The size of txBuffer. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataDataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a transmission.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_TransmitDataBlocking(volatile cy_stc_smif_reg_t *base,
                            const uint8_t *txBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataDataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_stc_smif_context_t *context)
{
    uint32_t trUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(txBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if(Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataDataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        trUnitNum = size / 2;
    }
    else
    {
        trUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldTxCount.cmd       = CY_SMIF_CMD_FIFO_TX_COUNT_MODE,
        .fieldTxCount.width     = (uint32_t)transferWidth,
        .fieldTxCount.dataNum   = (uint32_t)(trUnitNum - 1U),
        .fieldTxCount.dataRate  = (uint32_t)dataDataRate,
        .fieldTxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldTxCount.selectDev = slaveSelect,
    };

    /* initialize parameters for Cy_SMIF_PushTxFifo */
    cy_stc_smif_context_t contextLoc =
    {
        .txBufferAddress     = (uint8_t*)txBuffer,
        .txBufferCounter     = size,
        .txCmpltCb           = NULL,
        .transferStatus      = CY_SMIF_SEND_BUSY,
        .preCmdDataRate      = dataDataRate,
        .preCmdWidth         = transferWidth,
    };

    /* Enter the transmitting mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    uint32_t timeoutUnits = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
    while ((CY_SMIF_SEND_BUSY == contextLoc.transferStatus) &&
           (CY_SMIF_EXCEED_TIMEOUT != result))
    {
        Cy_SMIF_PushTxFifo(base, &contextLoc);
        result = Cy_SMIF_TimeoutRun(&timeoutUnits);
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveDataCmd
****************************************************************************//**
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is 
* typically used when the FIFO is handled outside the interrupt and is managed 
* in either a polling-based code or a DMA. The user would handle the FIFO 
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param RxCmpltCb
* The callback executed at the end of a reception. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_ReceiveDataCmd(volatile cy_stc_smif_reg_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t RxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveData
****************************************************************************//**
*
* This function implements the receive data phase in the memory command. The
* data is received into the RX Data FIFO using the RX_COUNT command. This
* function sets up the interrupt to trigger on the RX Data FIFO level, and the 
* data is fetched from the RX Data FIFO to the rxBuffer as it gets filled. This 
* function does not block until completion. The completion will trigger the call
* back function.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand. The
* slave select is de-asserted at the end of the receive.
* The function triggers the transfer and the transfer itself utilizes the
* interrupt for FIFO operations in the background. Thus, frequent
* interrupts will be executed after this function is triggered.
* This API is non-blocking and sets up the interrupt to act on the data
* FIFO, ensure there will be no another instance of the function called
* before the current instance has completed execution.
*
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is 
* typically used when the FIFO is handled outside the interrupt and is managed 
* in either a polling-based code or a DMA. The user would handle the FIFO 
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer 
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param RxCmpltCb
* The callback executed at the end of a reception. NULL interpreted as no
* callback.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_ReceiveData(volatile cy_stc_smif_reg_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_smif_event_cb_t RxCmpltCb,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    /* Prepare a cmd fifo data */
    cy_un_smif_cmd_fifo_t cmdData  =
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Move the parameters to the global variables */
    context->rxBufferAddress = (uint8_t*)rxBuffer;
    context->rxBufferCounter = size;
    context->rxCmpltCb       = RxCmpltCb;
    context->transferStatus  = CY_SMIF_REC_BUSY;

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    /* Enable the TR_RX_REQ interrupt */
    base->unINTR_MASK.stcField.u1TR_RX_REQ = 1u;

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_ReceiveDataBlocking
****************************************************************************//**
*
* This function implements the receive data phase in the memory command. The
* data is received into the RX Data FIFO using the RX_COUNT command. This
* function blocks until completion. The function does not use the interrupts and
* will use CPU to monitor the FIFO status and move data accordingly. The
* function returns only on completion.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand. The
* slave select is de-asserted at the end of the receive. Ensure there is
* no another transfers.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param rxBuffer
* The pointer to the variable where the receive data is stored. If this pointer
* is a NULL, then the function does not enable the interrupt. This use case is
* typically used when the FIFO is handled outside the interrupt and is managed
* in either a polling-based code or a DMA. The user would handle the FIFO
* management in a DMA or a polling-based code.
*
* \note If the user provides a NULL pointer in this function and does not handle
* the FIFO transaction, this could either stall or timeout the operation.
* The transfer statuses returned by \ref Cy_SMIF_GetTxfrStatus are no longer
* valid.
*
* \param size
* The size of data to be received. Must be > 0.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param slaveSelect
* Denotes the number of the slave device to which the transfer is made.
* (0, 1, 2 or 4 - the bit defines which slave to enable) Two-bit enable is
* possible only for the Double Quad SPI mode.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return A status of a reception.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_EXCEED_TIMEOUT
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_ReceiveDataBlocking(volatile cy_stc_smif_reg_t *base,
                            uint8_t *rxBuffer,
                            uint32_t size,
                            cy_en_smif_txfr_width_t transferWidth,
                            cy_en_smif_data_rate_t  dataRate,
                            cy_en_smif_slave_select_t  slaveSelect,
                            cy_stc_smif_context_t *context)
{
    uint32_t rxUnitNum;

    /* Check input values */
    if(size == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(rxBuffer == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* If the mode is octal SPI with DDR or Hyperbus, data unit is a 2-byte */
    if((transferWidth == CY_SMIF_WIDTH_OCTAL) && (dataRate == CY_SMIF_DDR))
    {
        if(size % 2 != 0)
        {
            return CY_SMIF_BAD_PARAM;
        }
        rxUnitNum = size / 2;
    }
    else
    {
        rxUnitNum = size;
    }

    cy_un_smif_cmd_fifo_t cmdData = 
    {
        .fieldRxCount.cmd       = CY_SMIF_CMD_FIFO_RX_COUNT_MODE,
        .fieldRxCount.width     = (uint32_t)transferWidth,
        .fieldRxCount.dataRate  = (uint32_t)dataRate,
        .fieldRxCount.dataNum   = (uint32_t)(rxUnitNum - 1U),
        .fieldRxCount.last      = (uint32_t)LAST_COMMAND_BYTE,
        .fieldRxCount.selectDev = slaveSelect,
    };

    /* Enter the receiving mode */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    /* initialize parameters for Cy_SMIF_PushTxFifo */
    cy_stc_smif_context_t contextLoc = 
    {
        .rxBufferAddress = (uint8_t*)rxBuffer,
        .rxBufferCounter = size,
        .rxCmpltCb       = NULL,
        .transferStatus  = CY_SMIF_REC_BUSY,
    };

   /* The return variable */
    uint32_t timeoutUnits      = context->timeout;
    cy_en_smif_status_t result = CY_SMIF_SUCCESS;
     while ((CY_SMIF_REC_BUSY == contextLoc.transferStatus) &&
            (CY_SMIF_EXCEED_TIMEOUT != result))
    {
        Cy_SMIF_PopRxFifo(base, &contextLoc);
        result = Cy_SMIF_TimeoutRun(&timeoutUnits);
    }

  return (result);
}


/*******************************************************************************
* Function Name: Cy_SMIF_SendDummyCycles()
****************************************************************************//**
*
* This function sends dummy-clock cycles. The data lines are tri-stated during
* the dummy cycles.
*
* \note This function is to be preceded by \ref Cy_SMIF_TransmitCommand.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param transferWidth
* The width of transfer \ref cy_en_smif_txfr_width_t.
*
* \param dataRate
* The data rate of transfer \ref cy_en_smif_data_rate_t.
*
* \param doubleDummy
* Specifies whether an active RWDS refresh indicator
* (captured with the previous rising memory clock output) causes to double
* the number of dummy cycles. This is used for the variable latency in HyperRAM.
*
* \param rwdsInDmyEn
* Specifies whether the RWDS output signal should be driven starting in the
* last dummy cycle until DESELECT. This is needed for write transactions with
* RWDS (e.g. Hyperbus) before a TX_COUNT command.
*
* \param dlpEn
* Specifies whether data learning is enabled.
* In XIP mode this field is set for read transactions when
* CTL.INT_CLOCK_DL_ENABLED is "1".
*
* \param capEn
* specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle,
* i.e. this needs to be set to '1' for read transactions to memory device using
* RWDS / DQS capturing scheme.
*
* \param cycles
* The number of dummy cycles. Must be > 0.
*
* \return A status of dummy cycles sending.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SendDummyCycles(volatile cy_stc_smif_reg_t *base,
                                             cy_en_smif_txfr_width_t transferWidth,
                                             cy_en_smif_data_rate_t  dataRate,
                                             bool doubleDm,
                                             bool rwdsInDm,
                                             bool dlpEn,
                                             bool capEn,
                                             uint32_t cycles,
                                             cy_stc_smif_context_t *context)
{
    /* Check input values */
    if(cycles == 0U)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    /* Check if there are enough free entries in TX_CMD_FIFO */
    cy_un_smif_cmd_fifo_t cmdData =
    {
        .fieldDummy.cmd      = CY_SMIF_CMD_FIFO_DUMMY_COUNT_MODE,
        .fieldDummy.cycleNum = ((uint32_t)(cycles-1U)),
        .fieldDummy.width    = (uint32_t)transferWidth,
        .fieldDummy.dataRate = (uint32_t)dataRate,
        .fieldDummy.last     = (uint32_t)NOT_LAST_COMMAND,
        .fieldDummy.doubleDm = doubleDm,
        .fieldDummy.rwdsInDm = rwdsInDm,
        .fieldDummy.dlpEn    = dlpEn,
        .fieldDummy.capEn    = capEn,
    };

    /* Send the dummy bytes */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_Deselect()
****************************************************************************//**
*
* This function  de-activates the select_out[3:0] IO output signals.
*
* \return
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_CMD_FIFO_FULL
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_Deselect(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context)
{
    /* Check if there are enough free entries in TX_CMD_FIFO */
    if  (Cy_SMIF_GetCmdFifoStatus(base) >= CY_SMIF_TX_CMD_FIFO_STATUS_RANGE)
    {
        return CY_SMIF_CMD_FIFO_FULL;
    }

    cy_un_smif_cmd_fifo_t cmdData =
    {
        .fieldDeselect.cmd      = CY_SMIF_CMD_FIFO_DESELECT_MODE,
    };

    /* Send the dummy bytes */
    Cy_SMIF_PUSH_TX_Command(base, cmdData, context);

    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_GetTxfrStatus
****************************************************************************//**
*
* This function provides the status of the transfer. This function is used to
* poll for the status of the TransmitData or receiveData function. When this
* function is called to determine the status of ongoing
* \ref Cy_SMIF_ReceiveData() or \ref Cy_SMIF_TransmitData(), the returned status
* is only valid if the functions passed a non-NULL buffer to transmit or
* receive respectively. If the pointer passed to \ref Cy_SMIF_ReceiveData()
* or \ref Cy_SMIF_TransmitData() is a NULL, then the code/DMA outside this
* driver will take care of the transfer and the Cy_GetTxfrStatus() will return 
* an erroneous result.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return Returns the transfer status. \ref cy_en_smif_txfr_status_t
*
*******************************************************************************/
cy_en_smif_txfr_status_t Cy_SMIF_GetTxfrStatus(volatile cy_stc_smif_reg_t *base, 
                                cy_stc_smif_context_t const *context)
{
    return (context->transferStatus);
}


/*******************************************************************************
* Function Name: Cy_SMIF_Enable
****************************************************************************//**
*
* Enables the operation of the SMIF block.
*
* \note This function only enables the SMIF IP. The interrupts associated with
* the SMIF will need to be separately enabled using the interrupt driver.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
void Cy_SMIF_Enable(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context)
{
    base->unCTL.stcField.u1ENABLED = 1u;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;
}

/*******************************************************************************
* Function Name: Cy_SMIF_CacheEnable
****************************************************************************//**
*
* This function is used to enable the fast cache, the slow cache or both.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheEnable(volatile cy_stc_smif_reg_t *base, 
                                        cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 1;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1ENABLED = 1;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 1;
        base->unFAST_CA_CTL.stcField.u1ENABLED = 1;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }   
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_CacheDisable
****************************************************************************//**
*
* This function is used to disable the fast cache, the slow cache or both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheDisable(volatile cy_stc_smif_reg_t *base, 
                                            cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 0;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1ENABLED = 0;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1ENABLED = 0;
        base->unFAST_CA_CTL.stcField.u1ENABLED = 0;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }   
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Name: Cy_SMIF_CachePrefetchingEnable
****************************************************************************//**
*
* This function is used to enable pre-fetching for the fast cache, the slow
* cache or both.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CachePrefetchingEnable(volatile cy_stc_smif_reg_t *base,
                                                    cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 1;
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 1;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_CachePrefetchingDisable
****************************************************************************//**
*
* This function is used to disable pre-fetching for the fast cache, the slow
* cache or both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CachePrefetchingDisable(volatile cy_stc_smif_reg_t *base,  
                                                    cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CTL.stcField.u1PREF_EN = 0;
        base->unFAST_CA_CTL.stcField.u1PREF_EN = 0;
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}


/*******************************************************************************
* Function Name: Cy_SMIF_CacheInvalidate
****************************************************************************//**
*
* This function is used to invalidate/clear the fast cache, the slow cache or
* both
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cacheType
* Holds the type of the cache to be modified. \ref cy_en_smif_cache_en_t
*
* \return A status of function completion.
*       - \ref CY_SMIF_SUCCESS
*       - \ref CY_SMIF_BAD_PARAM
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CacheInvalidate(volatile cy_stc_smif_reg_t *base, 
                                            cy_en_smif_cache_en_t cacheType)
{
    switch (cacheType)
    {
    case CY_SMIF_CACHE_SLOW:
        base->unSLOW_CA_CMD.stcField.u1INV = 1;
        (void)(base->unSLOW_CA_CMD.u32Register);
        break;
    case CY_SMIF_CACHE_FAST:
        base->unFAST_CA_CMD.stcField.u1INV = 1;
        (void)(base->unFAST_CA_CMD.u32Register);
        break;
    case CY_SMIF_CACHE_BOTH:
        base->unSLOW_CA_CMD.stcField.u1INV = 1;
        base->unFAST_CA_CMD.stcField.u1INV = 1;
        (void)(base->unFAST_CA_CMD.u32Register);
        break;
    default:
        /* A user error*/
        return CY_SMIF_BAD_PARAM;
    }
    return (CY_SMIF_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMode
****************************************************************************//**
*
* Sets the mode of operation for the SMIF. The mode of operation can be the XIP
* mode where the slave devices are mapped as memories and are directly accessed
* from the TVII register map. In the MMIO mode, the SMIF block acts as a simple
* SPI engine.
*
* \note Interrupt and triggers and not working in XIP mode, see TRM for details
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param mode
* The mode of the SMIF operation.
*
* \return If busy it will return CY_SMIF_BAD_STATUS. (see \ref cy_en_smif_status_t).
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SetMode(volatile cy_stc_smif_reg_t *base, cy_en_smif_mode_t mode)
{
    /*  Wait if busy */
    if(Cy_SMIF_IsBusy(base))
    {
        return CY_SMIF_BAD_STATUS;
    }

    Cy_SMIF_CacheInvalidate(base, CY_SMIF_CACHE_BOTH);

    /*  Set the register SMIF.CTL.XIP_MODE */
    base->unCTL.stcField.u1XIP_MODE = (uint8_t)mode;

    /* Read the register to flush the buffer */
    (void) base->unCTL.u32Register;

    return CY_SMIF_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_SMIF_SetMode
****************************************************************************//**
*
* This function calculates CRC for input data. Please call this function in XIP
* mode.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param inData
* The data that this function will calculate it's CRC.
*
* \param sizeInByte
* The size of input data.
*
* \param result
* A pointer to 8 bit CRC calculated.
*
* \note Not tested yet.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_CRC_Calibration(volatile cy_stc_smif_reg_t *base, uint8_t inData[], uint32_t sizeInByte, uint8_t* result)
{
    if(base->unCTL.stcField.u1XIP_MODE == CY_SMIF_MEMORY)
    {
        return CY_SMIF_BAD_STATUS;
    }

    if(inData == NULL)
    {
        return CY_SMIF_BAD_PARAM;
    }

    if(sizeInByte == 0ul)
    {
        return CY_SMIF_BAD_PARAM;
    }

    for(uint32_t i_byte = 0ul; i_byte < sizeInByte; i_byte += 8ul)
    {
        base->unCRC_INPUT0.u32Register = 0ul;
        base->unCRC_INPUT1.u32Register = 0ul;

        uint32_t restByte = (8ul < (sizeInByte - i_byte)) ? 8ul : (sizeInByte - i_byte); // min(8ul, (sizeInByte - i_byte));
        for(uint32_t i_subByte = 0ul; i_subByte < restByte; i_subByte++)
        {
            if(i_subByte < 4ul)
            {
                base->unCRC_INPUT0.au8Byte[i_subByte] = inData[i_byte + i_subByte];
            }
            else
            {
                base->unCRC_INPUT1.au8Byte[i_subByte-4ul] = inData[i_byte + i_subByte];
            }
        }

        if(i_byte == 0ul)
        {
            base->unCRC_CMD.stcField.u1START = 1u;
            while(base->unCRC_CMD.stcField.u1START == 1u); // Note: no timeout for now
        }
        else
        {
            base->unCRC_CMD.stcField.u1CONTINUE = 1u;
            while(base->unCRC_CMD.stcField.u1CONTINUE == 1u); // Note: no timeout for now
        }
    }

    *result = (uint8_t)(base->unCRC_OUTPUT.stcField.u8CRC_OUTPUT);
    return CY_SMIF_SUCCESS;
}


/*******************************************************************************
* Function Cy_SMIF_GetSmifBasePtrFromAnySmifPtr
****************************************************************************//**
*
* This function returns the SMIF base pointer corresponding to any pointer within
* the SMIF instance address space. 
*
* \param any_smif_address
* Can be any address within a SMIF instance address space. Typically it either is
* already a SMIF base pointer or a SMIF_DEVICE base pointer.
*
* \return SMIF base pointer
*
*******************************************************************************/
volatile cy_stc_smif_reg_t * Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(volatile void *any_smif_address)
{
    uint32_t mask_for_smif            = ~(SMIF_SECTION_SIZE - 1);
    uint32_t smif_base                = ((uint32_t) any_smif_address) & mask_for_smif;

    return (volatile cy_stc_smif_reg_t *) smif_base;
}

/*******************************************************************************
* Function Cy_SMIF_GetSlaveSelectFromDeviceBasePtr
****************************************************************************//**
*
* This function returns the cy_en_smif_slave_select_t corresponding to the
* base pointer to a SMIF_DEVICE instance which is passed as parameter
*
* \param device
* The base address of the SMIF_DEVICE registers.
*
* \return (see \cy_en_smif_slave_select_t)
*
*******************************************************************************/
cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceBasePtr(volatile cy_stc_smif_reg_device_t *device)
{
    uint32_t device_offset_from_smif    = ((uint32_t)device & (SMIF_SECTION_SIZE - 1));
    uint32_t device_offset_from_device0 = device_offset_from_smif - (((uint32_t)CY_SMIF_DRV_SMIF0_CORE0_DEVICE0 - (uint32_t) CY_SMIF_DRV_SMIF0_CORE0));
    uint8_t  device_instance            = device_offset_from_device0 / CY_SMIF_DRV_DEVICE_SECTION_SIZE;

    cy_en_smif_slave_select_t slave_select = Cy_SMIF_GetSlaveSelectFromDeviceNumber(device_instance);

    return slave_select;
}


/*******************************************************************************
* Function Cy_SMIF_GetSlaveSelectFromDeviceNumber
****************************************************************************//**
*
* This function returns the cy_en_smif_slave_select_t corresponding to the
* device number which is passed as parameter
*
* \param device_number
* Device index (0, 1, 2, ...)
*
* \return (see \cy_en_smif_slave_select_t)
*
*******************************************************************************/
cy_en_smif_slave_select_t Cy_SMIF_GetSlaveSelectFromDeviceNumber(uint8_t device_number)
{
    cy_en_smif_slave_select_t slave_select = (cy_en_smif_slave_select_t) 0xdeaddead;

    switch(device_number)
    {
        case 0:
            slave_select = CY_SMIF_SLAVE_SELECT_0;
            break;
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 2) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 2))
        case 1:
            slave_select = CY_SMIF_SLAVE_SELECT_1;
            break;
      #endif
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 3) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 3))
        case 2:
            slave_select = CY_SMIF_SLAVE_SELECT_2;
            break;
      #endif
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 4) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 4))
        case 3:
            slave_select = CY_SMIF_SLAVE_SELECT_3;
            break;
      #endif
        default:
            break;
    }

    return slave_select;
}


/*******************************************************************************
* Function Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect
****************************************************************************//**
*
* This function returns the device base pointer corresponding to the
* base pointer to a SMIF instance and slave select which are passed as parameter
*
* \param base
* The base address of the SMIF registers.
*
* \param slave
* Specifies the number of the slave device.
* \ref cy_en_smif_slave_select_t
*
* \return SMIF_DEVICE base pointer
*
*******************************************************************************/
volatile cy_stc_smif_reg_device_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave)
{
    // Offset to first device / slave select #0
    uint32_t offset = (((uint32_t)CY_SMIF_DRV_SMIF0_CORE0_DEVICE0 - (uint32_t) CY_SMIF_DRV_SMIF0_CORE0));

    // Increment offset based on actual slave select
    switch(slave)
    {
        case CY_SMIF_SLAVE_SELECT_0:
            offset += (0 * CY_SMIF_DRV_DEVICE_SECTION_SIZE);
            break;
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 2) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 2))
        case CY_SMIF_SLAVE_SELECT_1:
            offset += (1 * CY_SMIF_DRV_DEVICE_SECTION_SIZE);
            break;
      #endif
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 3) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 3))
        case CY_SMIF_SLAVE_SELECT_2:
            offset += (2 * CY_SMIF_DRV_DEVICE_SECTION_SIZE);
            break;
      #endif
      #if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 4) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 4))
        case CY_SMIF_SLAVE_SELECT_3:
            offset += (3 * CY_SMIF_DRV_DEVICE_SECTION_SIZE);
            break;
      #endif
        default:
            break;
    }

    return (volatile cy_stc_smif_reg_device_t *) ((uint32_t)base + offset);
}


/*******************************************************************************
* Function Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber
****************************************************************************//**
*
* This function returns the device base pointer corresponding to the
* base pointer to a SMIF instance and device number which are passed as parameter
*
* \param base
* The base address of the SMIF registers.
*
* \param device_number
* Device index (0, 1, 2, ...)
*
* \return SMIF_DEVICE base pointer
*
*******************************************************************************/
volatile cy_stc_smif_reg_device_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber(volatile cy_stc_smif_reg_t *base, uint8_t device_number)
{
    // Offset to first device / slave select #0
    uint32_t offset = (((uint32_t)CY_SMIF_DRV_SMIF0_CORE0_DEVICE0 - (uint32_t) CY_SMIF_DRV_SMIF0_CORE0));

    offset += (device_number * CY_SMIF_DRV_DEVICE_SECTION_SIZE);

    return (volatile cy_stc_smif_reg_device_t *) ((uint32_t)base + offset);
}



/*******************************************************************************
* Function Name: Cy_SMIF_DeepSleepCallback
****************************************************************************//**
*
* This function handles the transition of the SMIF into and out of Deep
* Sleep mode. It prevents the device from entering DeepSleep if SMIF is actively
* communicating, or there is any data in the TX or RX FIFOs, or SMIF is in
* memory mode.
*
* This function should be called while execution of \ref Cy_SysPm_DeepSleep
* therefore must be registered as a callback before the call. To register it
* call \ref Cy_SysPm_RegisterCallback and specify \ref CY_SYSPM_DEEPSLEEP
* as the callback type.
*
* \note
* This API is template and user should add code for external memory enter/exit
* low power mode.
*
* \param callbackParams
* The pointer to the structure with SMIF SysPm callback parameters (pointer to
* SMIF registers, context and call mode \ref cy_stc_syspm_callback_params_t).
*
* \return
* \ref cy_en_syspm_status_t
*
* Example setup of SysPM deep sleep and hibernate mode
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF SysPM Callback
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SMIF_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;
    
    CY_ASSERT_L1(NULL != callbackParams);

    cy_stc_smif_reg_t *locBase = (cy_stc_smif_reg_t *) callbackParams->base;
    cy_stc_smif_context_t *locContext = (cy_stc_smif_context_t *) callbackParams->context;

    switch(callbackParams->mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check if API is not busy executing transfer operation */
            /* If SPI bus is not busy, all data elements are transferred on
                * the bus from the TX FIFO and shifter and the RX FIFIOs is
                * empty - the SPI is ready enter Deep Sleep.
            */
            bool checkFail = (CY_SMIF_REC_BUSY == (cy_en_smif_txfr_status_t)
                                    Cy_SMIF_GetTxfrStatus(locBase, locContext));
            checkFail = (Cy_SMIF_IsBusy(locBase)) || checkFail;
            checkFail = (Cy_SMIF_GetMode(locBase) == CY_SMIF_MEMORY) || checkFail;

            if (checkFail)
            {
                retStatus = CY_SYSPM_FAIL;
            }
            else
            {
                Cy_SMIF_Disable(locBase);
                retStatus = CY_SYSPM_SUCCESS;
            }
            /* Add check memory that memory not in progress */
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* Other driver is not ready for Deep Sleep. Restore Active mode
            * configuration.
            */
            Cy_SMIF_Enable(locBase, locContext);

        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
        {
            /* This code executes inside critical section and enabling active
            * interrupt source makes interrupt pending in the NVIC. However
            * interrupt processing is delayed until code exists critical
            * section. The pending interrupt force WFI instruction does
            * nothing and device remains in the active mode.
            */

            /* Put external memory in low power mode */
        }
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            /* Put external memory in active mode */
            Cy_SMIF_Enable(locBase, locContext);
        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
            break;
    }

    return (retStatus);
}


/*******************************************************************************
* Function Name: Cy_SMIF_HibernateCallback
****************************************************************************//**
*
* This function handles the transition of the SMIF into Hibernate mode.
* It prevents the device from entering Hibernate if the SMIF
* is actively communicating, or there is any data in the TX or RX FIFO, or SMIF
* is in memory mode.
*
* This function should be called during execution of \ref Cy_SysPm_Hibernate
* therefore it must be registered as a callback before the call. To register it
* call \ref Cy_SysPm_RegisterCallback and specify \ref CY_SYSPM_HIBERNATE
* as the callback type.
*
* \note
* This API is template and user should add code for external memory enter/exit
* low power mode.
*
* \param callbackParams
* The pointer to the structure with SMIF SysPm callback parameters (pointer to
* SMIF registers, context and call mode \ref cy_stc_syspm_callback_params_t).
*
* \return
* \ref cy_en_syspm_status_t
*
* Example setup of SysPM deep sleep and hibernate mode
* \snippet smif/smif_sut_01.cydsn/main_cm4.c SMIF SysPM Callback
*
*******************************************************************************/
cy_en_syspm_status_t Cy_SMIF_HibernateCallback(cy_stc_syspm_callback_params_t *callbackParams)
{
    cy_en_syspm_status_t retStatus = CY_SYSPM_SUCCESS;

    CY_ASSERT_L1(NULL != callbackParams);
    
    cy_stc_smif_reg_t *locBase = (cy_stc_smif_reg_t *) callbackParams->base;
    cy_stc_smif_context_t *locContext = (cy_stc_smif_context_t *) callbackParams->context;

    switch(callbackParams->mode)
    {
        case CY_SYSPM_CHECK_READY:
        {
            /* Check if API is not busy executing transfer operation 
            * If SPI bus is not busy, all data elements are transferred on
            * the bus from the TX FIFO and shifter and the RX FIFIOs is
            * empty - the SPI is ready enter Deep Sleep.
            */
            bool checkFail = (CY_SMIF_REC_BUSY == (cy_en_smif_txfr_status_t)
                                Cy_SMIF_GetTxfrStatus(locBase, locContext));
            checkFail = (Cy_SMIF_IsBusy(locBase)) || checkFail;
            checkFail = (Cy_SMIF_GetMode(locBase) == CY_SMIF_MEMORY) || checkFail;

            if (checkFail)
            {
                retStatus = CY_SYSPM_FAIL;

            }
            else
            {
                retStatus = CY_SYSPM_SUCCESS;
                Cy_SMIF_Disable(locBase);
            }
            /* Add check memory that memory not in progress */
        }
        break;

        case CY_SYSPM_CHECK_FAIL:
        {
            /* Other driver is not ready for Deep Sleep. Restore Active mode
            * configuration.
            */
            Cy_SMIF_Enable(locBase, locContext);

        }
        break;

        case CY_SYSPM_BEFORE_ENTER:
        {
            /* Put external memory in low power mode */
        }
        break;

        case CY_SYSPM_AFTER_EXIT:
        {
            Cy_SMIF_Enable(locBase, locContext);
            /* Put external memory in active mode */

        }
        break;

        default:
            retStatus = CY_SYSPM_FAIL;
        break;
    }

    return (retStatus);
}

/*******************************************************************************
* Function Name: Cy_SMIF_PUSH_TX_Command()
****************************************************************************//**
*
* 
*  If the last bit of input command value is "0" (not last),
*  This function push input tx command value to internal buffer.
*  If the last bit of input command value is "1" (last),
*  This function push all accumulated tx command value to TX_CMD_FIFO.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param cmdData
* The input command data
*
* \param ctx
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
*******************************************************************************/
static void Cy_SMIF_PUSH_TX_Command(volatile cy_stc_smif_reg_t *base, cy_un_smif_cmd_fifo_t cmdData, cy_stc_smif_context_t *ctx)
{
#if 1
    #define CY_TX_CMD_BUF_MAX (64)
    static cy_un_smif_cmd_fifo_t TxCmdBuf[CY_SMIF_CORE_MAX][CY_TX_CMD_BUF_MAX];

    uint32_t smifCoreIdx;
    if(base == CY_SMIF_DRV_SMIF0_CORE0)
    {
        smifCoreIdx = 0;
    }
#if defined(CY_SMIF_DRV_SMIF0_CORE1)
    else if(base == CY_SMIF_DRV_SMIF0_CORE1)
    {
        smifCoreIdx = 1;
    }
#endif
    else
    {
        // This driver supports up to 2 SMIF core as of now.
        CY_ASSERT(false);
    }

    TxCmdBuf[smifCoreIdx][ctx->txCmdBufCounter] = cmdData;
    ctx->txCmdBufCounter++;
    CY_ASSERT_L1(ctx->txCmdBufCounter <= CY_TX_CMD_BUF_MAX); // buffer over flow

    if((cmdData.fieldTx.last == 1)                               ||
       (cmdData.fieldTx.cmd  == CY_SMIF_CMD_FIFO_DESELECT_MODE))
    {
        uint32_t pushWord = 0;
        for(uint32_t remaindWord = ctx->txCmdBufCounter; remaindWord > 0; remaindWord -= pushWord)
        {
            uint32_t progressWord = ctx->txCmdBufCounter - remaindWord;
            uint32_t emptyWord   = CY_SMIF_TX_CMD_FIFO_STATUS_RANGE - base->unTX_CMD_FIFO_STATUS.stcField.u4USED4;

            pushWord    = (remaindWord < emptyWord) ? remaindWord
                                                    : emptyWord;

            Cy_SMIF_PushCmdFifo(base, &TxCmdBuf[smifCoreIdx][progressWord].u32, pushWord);
        }
        ctx->txCmdBufCounter = 0;
    }
#else // for debug
    base->unTX_CMD_FIFO_WR.u32Register = cmdData.u32;
#endif
}


#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

/* [] END OF FILE */
