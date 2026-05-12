/***************************************************************************//**
* \file cy_smif_s25fs.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the S25FS HSSPI series driver APIs.
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
#if defined(__cplusplus)
extern "C" {
#endif

#include "cy_smif_s25fs.h"

/************************************************************/
/*************** Specify Read Command Sequence **************/
/************************************************************/
const cy_stc_cmd_sq_t g_s25fsReadCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT},   // cmdTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 2
};

/************************************************************/
/*************** Specify Read Address Sequence **************/
/************************************************************/
const cy_stc_addr_sq_t g_s25fsReadAddrTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 1
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 2
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 3
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 4
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 5
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 6
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 7
};

/************************************************************/
/**************** Specify Read Mode Sequence ****************/
/************************************************************/
const cy_stc_mode_sq_t g_s25fsReadModeTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT},   // modeTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 1
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 2
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 3
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 4
};

/************************************************************/
/**************** Specify Read Dummy Sequence ***************/
/************************************************************/
const cy_stc_dummy_sq_t g_s25fsReadDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT,   .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 0
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true},  // dummyTypeNum = 1
    {.tableId = 0, .width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true},  // dummyTypeNum = 2
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true},  // dummyTypeNum = 3
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true},  // dummyTypeNum = 4
};

const uint8_t g_s25fsLatTblOtherThanOctalRead[CY_SMIF_S25FS_READ_LATENCY_CODE_MAX] = {0u,  1u,  2u,  3u,  4u,  5u,  6u,  7u,  8u,  9u, 10u, 11u, 12u, 13u, 14u, 15u};

const uint8_t* g_s25fsReadLatencyCodeToDummyCycle[CY_SMIF_S25FS_READ_LC_DC_TABLE_MAX] =
{
    g_s25fsLatTblOtherThanOctalRead,
};

/************************************************************/
/**************** Specify Read Data Sequence ****************/
/************************************************************/
const cy_stc_data_sq_t g_s25fsReadDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 2
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR}, // dataTypeNum = 3
};

/***************************************************************/
/************* Read Command Table for S25FS Flash *************/
/***************************************************************/
const cy_stc_read_info_t g_s25fsReadCmdInfo[] =
{
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY1_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u },  // CY_SMIF_S25FS_RDID_1S1S1S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY1_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u },  // CY_SMIF_S25FS_RDID_1S1S1S_4ADR0
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY2_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum = 1u, .dataTypeNum = 0u },  // CY_SMIF_S25FS_RDID_1S1S1S_FAST_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY2_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum = 1u, .dataTypeNum = 0u },  // CY_SMIF_S25FS_RDID_1S1S1S_FAST_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY3_C_0, .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 2u, .dummyTypeNum = 2u, .dataTypeNum = 1u },  // CY_SMIF_S25FS_RDID_1S2S2S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY3_4_0, .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 2u, .dummyTypeNum = 2u, .dataTypeNum = 1u },  // CY_SMIF_S25FS_RDID_1S2S2S_4ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 2u, .dummyTypeNum = 2u, .dataTypeNum = 1u },  // CY_SMIF_S25FS_RDID_2S2S_3ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 2u, .dummyTypeNum = 2u, .dataTypeNum = 1u },  // CY_SMIF_S25FS_RDID_2S2S_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY4_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_1S1S4S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY4_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_1S1S4S_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 1u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_1S4S4S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 1u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_1S4S4S_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 1u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_1S4D4D_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY7_4_0, .cmdTypeNum = 1u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_1S4D4D_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 2u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_4S4S4S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 2u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_4S4S4S_4ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 0u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_4S4S_3ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 0u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum = 3u, .dataTypeNum = 2u },  // CY_SMIF_S25FS_RDID_4S4S_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_4D4D4D_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_4D4D4D_4ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 0u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_4D4D_3ADR
    { .cmd = 0ul /* no command */,         .cmdTypeNum = 0u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum = 4u, .dataTypeNum = 3u },  // CY_SMIF_S25FS_RDID_4D4D_4ADR
};

/************************************************************/
/*************** Specify Write Command Sequence *************/
/************************************************************/
const cy_stc_cmd_sq_t g_s25fsWriteCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 0
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 1
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 2
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 3
};

/************************************************************/
/*************** Specify Write Address Sequence *************/
/************************************************************/
const cy_stc_addr_sq_t g_s25fsWriteAddrTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 2
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 3
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 4
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 5
};

/************************************************************/
/**************** Specify Write Mode Sequence ***************/
/************************************************************/
const cy_stc_mode_sq_t g_s25fsWriteModeTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT  }, // modeTypeNum = 0
};

/************************************************************/
/**************** Specify Write Dummy Sequence **************/
/************************************************************/
const cy_stc_dummy_sq_t g_s25fsWriteDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT,   .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 0
};

const uint8_t g_latancyTableOfWrite_s25fs[CY_SMIF_S25FS_WRITE_LC_DC_TABLE_MAX]     = {0u,};
const uint8_t* g_s25fsWriteLatencyCodeToDummyCycle[CY_SMIF_S25FS_WRITE_LATENCY_CODE_MAX] =
{
   g_latancyTableOfWrite_s25fs,
};

/************************************************************/
/**************** Specify Write Data Sequence ***************/
/************************************************************/
const cy_stc_data_sq_t g_s25fsWriteDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR}, // dataTypeNum = 2
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR}, // dataTypeNum = 3
};

/***************************************************************/
/************* Write Command Table for S25FS Flash ************/
/***************************************************************/
const cy_stc_write_info_t g_s25fsWriteCmdInfo[] =
{
    { .cmd = CY_S25FS_CMD_1BYTE_PRPGE_C_1,  .cmdTypeNum = 0u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u }, // CY_SMIF_S25FS_WTID_1S1S1S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_PRPGE_4_1,  .cmdTypeNum = 0u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u }, // CY_SMIF_S25FS_WTID_1S1S1S_4ADR
    { .cmd = CY_S25FS_CMD_1BYTE_PRPGE_C_1,  .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 1u }, // CY_SMIF_S25FS_WTID_4S4S4S_3ADR
    { .cmd = CY_S25FS_CMD_1BYTE_PRPGE_4_1,  .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 1u }, // CY_SMIF_S25FS_WTID_4S4S4S_4ADR
};


/***********************************************************************/
/******************* Functions Issuing Each Commands *******************/
/***********************************************************************/
static const uint8_t* g_cmdLatTbls_s25fs[2] =
{
    NULL,                            // 0
    g_s25fsLatTblOtherThanOctalRead, // 1
};

/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadId
****************************************************************************//**
*
* This function provides read access to manufacturer and device identification.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readID
* Points to memory where ID value which is read will be stored.
* \ref cy_stc_smif_s25fs_id_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadId(volatile cy_stc_smif_reg_t* base,
                                         cy_en_smif_slave_select_t slave,
                                         cy_stc_smif_s25fs_id_t* readID,
                                         cy_en_smif_trx_type_t trx,
                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDIDN_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDIDN_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };
    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             0,
                             (uint8_t*)readID,
                             sizeof(cy_stc_smif_s25fs_id_t),
                             0,
                             g_cmdLatTbls_s25fs,
                             context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadJedecId
****************************************************************************//**
*
* This function sequentially accesses the Serial Flash  Discovery Parameters (SFDP).
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read data to be stored.
*
* \param readAddress
* Address of SFDP to be read.
*
* \param readSize
* Read data size
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadJedecId(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint64_t* readData[],  // YOTS TODO: Make sure this is correct
                                              uint32_t readAddress,
                                              uint32_t readSize,
                                              cy_en_smif_trx_type_t trx,
                                              cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RSFDP_3_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RSFDP_3_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };
    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             readAddress,
                             (uint8_t*)readData,
                             readSize,
                             0,
                             g_cmdLatTbls_s25fs,
                             context);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadStatusRegister1
****************************************************************************//**
*
* This function reads the Status Register 1 contents.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read status register value to be stored.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadStatusRegister1(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_en_smif_trx_type_t trx,
                                        cy_stc_smif_context_t *context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDSR1_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDSR1_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t      status;
    cy_un_smif_trx_type_t tx = {trx};
    uint8_t tempRead[2];
    uint8_t readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                                  slave,
                                  trx,
                                  rdCmdArray[tx.bf.tblNo],
                                  CY_TRX_ADDR_3BYTE,
                                  0,
                                  (uint8_t*)tempRead,
                                  readSize,
                                  0,
                                  g_cmdLatTbls_s25fs,
                                  context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadStatusRegister2
****************************************************************************//**
*
* This function reads the Status Register 2 contents.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read status register value to be stored.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadStatusRegister2(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      uint8_t* readData,
                                                      cy_en_smif_trx_type_t trx,
                                                      cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDSR2_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDSR2_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[2];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             0,
                             (uint8_t*)tempRead,
                             readSize,
                             0,
                             g_cmdLatTbls_s25fs,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_ReadAnyRegisterS25SL
****************************************************************************//**
*
* This function provides a way to read all addressed nonvolatile and volatile
* device registers.
* Read data does not support multiple slave access at the same time.
* This is only for S25SL series
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read register value to be stored.
*
* \param regAddr
* Address to the device register to be read.
* \ref cy_en_smif_s25fs_reg_addr_t
*
* \param lc
* Latency code specified by the flash device.
* \ref cy_en_smif_s25fs_read_latency_code_t
*
* \param addrLen
* Address length of the transaction which is specified by a configuration register
* of the device. 3 byte or 4 byte.
* \ref cy_en_smif_trx_addr_len_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadAnyRegister(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint8_t* readData,
                                                  cy_en_smif_s25fs_reg_addr_t regAddr,
                                                  cy_en_smif_s25fs_read_latency_code_t lc,
                                                  cy_en_smif_trx_addr_len_t addrLen,
                                                  cy_en_smif_trx_type_t trx,
                                                  cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDARG_C_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDARG_C_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[2];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             addrLen,
                             regAddr,
                             (uint8_t*)tempRead,
                             readSize,
                             lc,
                             g_cmdLatTbls_s25fs,
                             context);
    *readData = tempRead[0];

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FS_WriteEnable
****************************************************************************//**
*
* This function sets the Write Enable Latch bit of the Status Register 1 to '1'
* to enable write, program and erase transactions.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_WriteEnable(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              cy_en_smif_trx_type_t trx,
                                              cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_WriteDisable
****************************************************************************//**
*
* This function sets the Write Enable Latch bit of the Status Register 1 to '0'
* to disable write, program and erase transactions.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_WriteDisable(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               cy_en_smif_trx_type_t trx,
                                               cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_WriteAnyRegister
****************************************************************************//**
*
* This function provides a way to write all addressed nonvolatile and volatile
* device registers.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* Value to be written into target device register.
*
* \param regAddr
* Address to the device register into which value of "writeData" to be written.
* \ref cy_en_smif_s25fs_read_latency_code_t
*
* \param addrLen
* Address length of the transaction which is specified by a configuration register
* of the device. 3 byte or 4 byte.
* \ref cy_en_smif_trx_addr_len_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_WriteAnyRegister(volatile cy_stc_smif_reg_t* base,
                                                   cy_en_smif_slave_select_t slave,
                                                   uint8_t writeData,
                                                   cy_en_smif_s25fs_reg_addr_t regAddr,
                                                   cy_en_smif_trx_addr_len_t addrLen,
                                                   cy_en_smif_trx_type_t trx,
                                                   cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_WRARG_C_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_WRARG_C_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[2] = { writeData, writeData };
    uint8_t               writeSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        writeSize = 2;
    }
    else
    {
        writeSize = 1;
    }

    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              addrLen,
                              regAddr,
                              tempWrite,
                              writeSize,
                              g_cmdLatTbls_s25fs,
                              context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FS_ClearProgramAndEraseFailureFlags
****************************************************************************//**
*
* This function resets Erase failure flag and Program failure flag
* in Status Register 1.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ClearProgramAndEraseFailureFlags(volatile cy_stc_smif_reg_t* base,
                                                                   cy_en_smif_slave_select_t slave,
                                                                   cy_en_smif_trx_type_t trx,
                                                                   cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadECC_DataUnitStatus
****************************************************************************//**
*
* This function is used to determine the ECC status of the addressed data unit.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readStatus
* Pointer to memory which read ECC data unit status to be stored.
*
* \param unitAdder
* Address of ECC data unit, the status of which to be read.
* the LSB of the address must be aligned to an ECC data unit.
*
* \param lc
* Latency code specified by the flash device.
* \ref cy_en_smif_s25fs_read_latency_code_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadECC_DataUnitStatus(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         uint8_t* readStatus,
                                                         uint32_t unitAdder,
                                                         cy_en_smif_s25fs_read_latency_code_t lc,
                                                         cy_en_smif_trx_type_t trx,
                                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Parameter Check ***/
    if((unitAdder & 0x0000000Ful) != 0ul)
    {
        return CY_SMIF_BAD_PARAM;
    }

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[2];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             unitAdder,
                             (uint8_t*)tempRead,
                             readSize,
                             lc,
                             g_cmdLatTbls_s25fs,
                             context);
    *readStatus = tempRead[0];

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FS_Erase_4KB_Sector
****************************************************************************//**
*
* This function sets all the bits of a selected 4 KB sector to 1
* (all bytes are FFh).
* Make sure the device is configured as Hybrid Sector Architecture mode.
* This transaction is ignored when the device is configured for uniform sectors
* only (CR3V[3] = 1).
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param erasedAddress
* Address to a sector to be erased.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Erase_4KB_Sector(volatile cy_stc_smif_reg_t* base,
                                                   cy_en_smif_slave_select_t slave,
                                                   uint32_t erasedAddress,
                                                   cy_en_smif_trx_type_t trx,
                                                   cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_ER004_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_ER004_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_ADDR_INVALID,    // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_ADDR_INVALID,    // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD_ADDR(base,
                                 slave,
                                 trx,
                                 rdCmdArray[tx.bf.tblNo],
                                 CY_TRX_ADDR_3BYTE,
                                 erasedAddress,
                                 g_cmdLatTbls_s25fs,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_Erase_256KB_Sector
****************************************************************************//**
*
* The Erase 256 KB Sector transaction sets all bits in the addressed sector to 1
* (all bytes are FFh).
* A device configuration option (CFR3V[3]) determines if the Hybrid Sector
* Architecture is in use.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param erasedAddress
* Address to a sector to be erased.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Erase_256KB_Sector(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     uint32_t erasedAddress,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_ER256_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_ER256_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_ADDR_INVALID,    // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_ADDR_INVALID,    // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD_ADDR(base,
                                 slave,
                                 trx,
                                 rdCmdArray[tx.bf.tblNo],
                                 CY_TRX_ADDR_3BYTE,
                                 erasedAddress,
                                 g_cmdLatTbls_s25fs,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_EraseChip
****************************************************************************//**
*
* This function sets all bits to 1 (all bytes are FFh) inside the entire flash
* memory array. 
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_EraseChip(volatile cy_stc_smif_reg_t* base,
                                            cy_en_smif_slave_select_t slave,
                                            cy_en_smif_trx_type_t trx,
                                            cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_SuspendEraseProgram
****************************************************************************//**
*
* This function suspends erase, and program.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_SuspendEraseProgram(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      cy_en_smif_trx_type_t trx,
                                                      cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_SPEPA_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_SPEPA_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_ResumeEraseProgram
****************************************************************************//**
*
* This function resumes erase, and program.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ResumeEraseProgram(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RSEPA_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RSEPA_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);
    return status;
}

/* Secure Silicon Region Command and Advanced Sector Protection Command are not implemented yet! */

/*******************************************************************************
* Function Cy_SMIF_S25FS_SoftwareResetEnable
****************************************************************************//**
*
* This function is required immediately before a software reset transaction
* (Cy_SMIF_S25FS_SoftwareReset).
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_SoftwareResetEnable(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      cy_en_smif_trx_type_t trx,
                                                      cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx            = {trx};

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                                 slave,
                                 trx,
                                 rdCmdArray[tx.bf.tblNo],
                                 g_cmdLatTbls_s25fs,
                                 context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_SoftwareReset
****************************************************************************//**
*
* This function restores the device to its initial power up state,
* by reloading volatile registers from nonvolatile default values.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_SoftwareReset(volatile cy_stc_smif_reg_t* base,
                                                cy_en_smif_slave_select_t slave,
                                                cy_en_smif_trx_type_t trx,
                                                cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_EnterDeepPowerDownMode
****************************************************************************//**
*
* This function shifts device in the lowest power consumption mode.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_EnterDeepPowerDownMode(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         cy_en_smif_trx_type_t trx,
                                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FS_ReadConfigurationRegister1
****************************************************************************//**
*
* This function allows the Configuration Register-1 contents to be read. 
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read status register to be stored.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadConfigurationRegister1(volatile cy_stc_smif_reg_t* base,
                                                             cy_en_smif_slave_select_t slave,
                                                             uint8_t* readData,
                                                             cy_en_smif_trx_type_t trx,
                                                             cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDCR1_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDCR1_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[2];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             0,
                             (uint8_t*)tempRead,
                             readSize,
                             0,
                             g_cmdLatTbls_s25fs,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_WriteRegister
****************************************************************************//**
*
* This function provides a way to write Status Register and
* Configuration Registers 1.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param stsReg
* Value to be written into Status Register 1
*
* \param cfgReg1
* Value to be written into Configuration Register 1
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_WriteRegister(volatile cy_stc_smif_reg_t* base,
                                                cy_en_smif_slave_select_t slave,
                                                uint8_t  stsReg,
                                                uint8_t  cfgReg1,
                                                cy_en_smif_trx_type_t trx,
                                                cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[2] = { stsReg, cfgReg1 };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              tempWrite,
                              2,
                              g_cmdLatTbls_s25fs,
                              context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_Enter4ByteAddressMode
****************************************************************************//**
*
* This function sets the Address Legth bit CFR2V[7] to 1.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Enter4ByteAddressMode(volatile cy_stc_smif_reg_t* base,
                                                        cy_en_smif_slave_select_t slave,
                                                        cy_en_smif_trx_type_t trx,
                                                        cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_EN4BA_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_EN4BA_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            g_cmdLatTbls_s25fs,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_Read_DLP_Register
****************************************************************************//**
*
* This function reads the DLP pattern.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param dlpData
* Pointer to memory in which read DLP data to be stored.
*
* \param regLc
* Latency code specified by the flash device.
* \ref cy_en_smif_s25fs_read_latency_code_t
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Read_DLP_Register(volatile cy_stc_smif_reg_t* base,
                                                    cy_en_smif_slave_select_t slave,
                                                    uint8_t* dlpData,
                                                    cy_en_smif_s25fs_read_latency_code_t regLc,
                                                    cy_en_smif_trx_type_t trx,
                                                    cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_RDDLP_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_RDDLP_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_RD_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[2];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 2;
    }
    else
    {
        readSize = 1;
    }

    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             0,
                             (uint8_t*)tempRead,
                             readSize,
                             regLc,
                             g_cmdLatTbls_s25fs,
                             context);
    *dlpData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_Program_DLP_NV_Register
****************************************************************************//**
*
* This function programs DLP pattern into the Nonvolatile registers.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param dlpData
* DLP data value to be written into the Nonvolatile register.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Program_DLP_NV_Register(volatile cy_stc_smif_reg_t* base,
                                                          cy_en_smif_slave_select_t slave,
                                                          uint8_t dlpData,
                                                          cy_en_smif_trx_type_t trx,
                                                          cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_PRDLP_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_PRDLP_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[2] = { dlpData, dlpData };
    uint8_t               writeSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        writeSize = 2;
    }
    else
    {
        writeSize = 1;
    }

    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              tempWrite,
                              writeSize,
                              g_cmdLatTbls_s25fs,
                              context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FS_Write_DLP_V_Register
****************************************************************************//**
*
* This function writes DLP pattern into the Volatile register.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param dlpData
* DLP data value to be written to the Volatile register.
*
* \param trx
* Transaction type specifying data width, data rate, and so on.
* \ref cy_en_smif_trx_type_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FS_Write_DLP_V_Register(volatile cy_stc_smif_reg_t* base,
                                                       cy_en_smif_slave_select_t slave,
                                                       uint8_t dlpData,
                                                       cy_en_smif_trx_type_t trx,
                                                       cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_s25fs_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_S25FS_CMD_1BYTE_WRDLP_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_S25FS_CMD_1BYTE_WRDLP_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_S25FS_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[2] = { dlpData, dlpData };
    uint8_t               writeSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        writeSize = 2;
    }
    else
    {
        writeSize = 1;
    }

    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              tempWrite,
                              writeSize,
                              g_cmdLatTbls_s25fs,
                              context);
    return status;
}

#if defined(__cplusplus)
}
#endif
