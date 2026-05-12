/***************************************************************************//**
* \file cy_smif_semp.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the Semper HSSPI series driver APIs.
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

#include "cy_smif_semp.h"

/************************************************************/
/*************** Specify Read Command Sequence **************/
/************************************************************/
const cy_stc_cmd_sq_t g_sempReadCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT},   // cmdTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 2
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 3
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 4
};

/************************************************************/
/*************** Specify Read Address Sequence **************/
/************************************************************/
const cy_stc_addr_sq_t g_sempReadAddrTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 1
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 2
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 3
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 4
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 5
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 6
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 7
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 8
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 9
};

/************************************************************/
/**************** Specify Read Mode Sequence ****************/
/************************************************************/
const cy_stc_mode_sq_t g_sempReadModeTypes[] =
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
const cy_stc_dummy_sq_t g_sempReadDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT,   .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false},// dummyTypeNum = 0
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 1
    {.tableId = 0, .width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 2
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 3
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 4
    {.tableId = 1, .width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 5
    {.tableId = 1, .width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = true}, // dummyTypeNum = 6
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = true,  .rwdsEn = true}, // dummyTypeNum = 7
    {.tableId = 0, .width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = true,  .rwdsEn = true}, // dummyTypeNum = 8
};

const uint8_t g_sempLatTblOtherThanOctalRead[CY_SMIF_SEMP_READ_LATENCY_CODE_MAX] = {0u, 1u, 2u, 3u,  4u,  5u,  6u,  7u,  8u,  9u,  10u, 11u, 12u, 13u, 14u, 15u};
const uint8_t g_sempLatTblOctalRead[CY_SMIF_SEMP_READ_LATENCY_CODE_MAX]          = {5u, 6u, 8u, 10u, 12u, 14u, 16u, 18u, 20u, 22u, 23u, 24u, 25u, 26u, 27u, 28u};

const uint8_t* g_sempReadLatencyCodeToDummyCycle[CY_SMIF_SEMP_READ_LC_DC_TABLE_MAX] =
{
    g_sempLatTblOtherThanOctalRead,
    g_sempLatTblOctalRead,
};

/************************************************************/
/**************** Specify Read Data Sequence ****************/
/************************************************************/
const cy_stc_data_sq_t g_sempReadDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 2
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR}, // dataTypeNum = 3
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR}, // dataTypeNum = 4
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR}, // dataTypeNum = 5
};

/***************************************************************/
/************* Read Command Table for Semper Flash *************/
/***************************************************************/
const cy_stc_read_info_t g_sempReadCmdInfo[] =
{
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY1_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum =  0u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_RDID_1S1S1S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY1_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum =  0u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_RDID_1S1S1S_4ADR0
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY2_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 1u, .dummyTypeNum =  1u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_RDID_1S1S1S_FAST_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY2_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 1u, .dummyTypeNum =  1u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_RDID_1S1S1S_FAST_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY3_C_0, .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 2u, .dummyTypeNum =  2u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_RDID_1S2S2S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY3_4_0, .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 2u, .dummyTypeNum =  2u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_RDID_1S2S2S_4ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 2u, .dummyTypeNum =  2u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_RDID_2S2S_3ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 2u, .dummyTypeNum =  2u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_RDID_2S2S_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY4_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S1S4S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY4_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S1S4S_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 1u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S4S4S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 1u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S4S4S_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 1u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_1S4D4D_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_4_0, .cmdTypeNum = 1u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_1S4D4D_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 2u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S4S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 2u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S4S_4ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S_3ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  3u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D4D_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D4D_4ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D_3ADR
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  4u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D_4ADR
    { .cmd = CY_SEMP_CMD_2BYTE_RDAY1_4_0, .cmdTypeNum = 3u, .addrTypeNum = 8u, .modeTypeNum = 0u, .dummyTypeNum =  5u, .dataTypeNum = 4u }, // CY_SMIF_SEMP_RDID_8S8S8S_4ADR
    { .cmd = CY_SEMP_CMD_2BYTE_RDAY2_4_0, .cmdTypeNum = 4u, .addrTypeNum = 9u, .modeTypeNum = 0u, .dummyTypeNum =  6u, .dataTypeNum = 5u }, // CY_SMIF_SEMP_RDID_8D8D8D_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY4_C_0, .cmdTypeNum = 1u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S1S4S_3ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY4_4_0, .cmdTypeNum = 1u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S1S4S_4ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 1u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S4S4S_3ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 1u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_1S4S4S_4ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 1u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_1S4D4D_3ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_4_0, .cmdTypeNum = 1u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_1S4D4D_4ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_C_0, .cmdTypeNum = 2u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S4S_3ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY5_4_0, .cmdTypeNum = 2u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S4S_4ADR_DLP
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 4u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S_3ADR_DLP
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 5u, .modeTypeNum = 3u, .dummyTypeNum =  7u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_RDID_4S4S_4ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D4D_3ADR_DLP
    { .cmd = CY_SEMP_CMD_1BYTE_RDAY7_C_0, .cmdTypeNum = 2u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D4D_4ADR_DLP
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 6u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D_3ADR_DLP
    { .cmd =      CY_SEMP_CMD_NO_COMMAND, .cmdTypeNum = 0u, .addrTypeNum = 7u, .modeTypeNum = 4u, .dummyTypeNum =  8u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_RDID_4D4D_4ADR_DLP
};

/************************************************************/
/*************** Specify Write Command Sequence *************/
/************************************************************/
const cy_stc_cmd_sq_t g_sempWriteCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 0
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 1
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 2
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_2BYTE}, // cmdTypeNum = 3
};

/************************************************************/
/*************** Specify Write Address Sequence *************/
/************************************************************/
const cy_stc_addr_sq_t g_sempWriteAddrTypes[] =
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
const cy_stc_mode_sq_t g_sempWriteModeTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT  }, // modeTypeNum = 0
};

/************************************************************/
/**************** Specify Write Dummy Sequence **************/
/************************************************************/
const cy_stc_dummy_sq_t g_sempWriteDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT,   .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 0
};

const uint8_t g_latancyTableOfWrite[CY_SMIF_SEMP_WRITE_LC_DC_TABLE_MAX]     = {0u,};
const uint8_t* g_sempWriteLatencyCodeToDummyCycle[CY_SMIF_SEMP_WRITE_LATENCY_CODE_MAX] =
{
   g_latancyTableOfWrite,
};

/************************************************************/
/**************** Specify Write Data Sequence ***************/
/************************************************************/
const cy_stc_data_sq_t g_sempWriteDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_SDR}, // dataTypeNum = 2
    {.width = CY_SMIF_WIDTH_OCTAL,  .dataRate = CY_SMIF_DDR}, // dataTypeNum = 3
};

/***************************************************************/
/************* Write Command Table for Semper Flash ************/
/***************************************************************/
const cy_stc_write_info_t g_sempWriteCmdInfo[] =
{
    { .cmd = CY_SEMP_CMD_1BYTE_PRPGE_C_1,  .cmdTypeNum = 0u, .addrTypeNum = 0u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_WTID_1S1S1S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_PRPGE_4_1,  .cmdTypeNum = 0u, .addrTypeNum = 1u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 0u }, // CY_SMIF_SEMP_WTID_1S1S1S_4ADR
    { .cmd = CY_SEMP_CMD_1BYTE_PRPGE_C_1,  .cmdTypeNum = 1u, .addrTypeNum = 2u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_WTID_4S4S4S_3ADR
    { .cmd = CY_SEMP_CMD_1BYTE_PRPGE_4_1,  .cmdTypeNum = 1u, .addrTypeNum = 3u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 1u }, // CY_SMIF_SEMP_WTID_4S4S4S_4ADR
    { .cmd = CY_SEMP_CMD_2BYTE_PRPGE_4_1,  .cmdTypeNum = 2u, .addrTypeNum = 4u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 2u }, // CY_SMIF_SEMP_WTID_8S8S8S_4ADR
    { .cmd = CY_SEMP_CMD_2BYTE_PRPGE_4_1,  .cmdTypeNum = 3u, .addrTypeNum = 5u, .modeTypeNum = 0u, .dummyTypeNum = 0u, .dataTypeNum = 3u }, // CY_SMIF_SEMP_WTID_8D8D8D_4ADR
};

/**********************************************/
/************* Latency Table Array ************/
/**********************************************/
static const uint8_t g_sempLatTblRegType1[CY_SMIF_SEMP_REG_LATENCY_CODE_MAX] = {0u, 1u, 1u, 2u};
static const uint8_t g_sempLatTblRegType2[CY_SMIF_SEMP_REG_LATENCY_CODE_MAX] = {0u, 0u, 1u, 2u};
static const uint8_t g_sempLatTblRegType3[CY_SMIF_SEMP_REG_LATENCY_CODE_MAX] = {3u, 4u, 5u, 6u};

const uint8_t* cmdLatTbls[6] =
{
    NULL,                           // 0
    g_sempLatTblRegType1,           // 1
    g_sempLatTblRegType2,           // 2
    g_sempLatTblRegType3,           // 3
    g_sempLatTblOtherThanOctalRead, // 4
    g_sempLatTblOctalRead,          // 5
};

/***********************************************************************/
/******************* Functions Issuing Each Commands *******************/
/***********************************************************************/

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadId
****************************************************************************//**
*
* This function provides read access to manufacturer and device identification.
* Supported By (S25H, S26H, S28H) series.
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
* \ref cy_stc_smif_semp_id_t
*
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadId(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_semp_id_t* readID,
                                        cy_en_smif_semp_reg_latency_code_t regLc,
                                        cy_en_smif_trx_type_t trx,
                                        uint32_t readSizeInByte,
                                        cy_stc_smif_context_t *context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDIDN_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDIDN_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDIDN_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDIDN_4_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx = {trx};
    status = Cy_SMIF_TX_READ(base,
                             slave,
                             trx,
                             rdCmdArray[tx.bf.tblNo],
                             CY_TRX_ADDR_3BYTE,
                             0,
                             (uint8_t*)readID,
                             readSizeInByte,
                             regLc,
                             cmdLatTbls,
                             context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadJedecId
****************************************************************************//**
*
* This function sequentially accesses the Serial Flash  Discovery Parameters (SFDP).
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadJedecId(volatile cy_stc_smif_reg_t* base,
                                             cy_en_smif_slave_select_t slave,
                                             uint64_t* readData[], // YOTS TODO: Make sure this is correct
                                             uint32_t readAddress,
                                             uint32_t readSize,
                                             cy_en_smif_trx_type_t trx,
                                             cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RSFDP_3_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RSFDP_3_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RSFDP_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RSFDP_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadUniqueId
****************************************************************************//**
*
* This function accesses a factory programmed 64-bit number which is unique to
* each device.
* Supported By (S25H, S26H, S28H) series.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param uniqueId
* Pointer to memory in which read unique ID to be stored.
*
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadUniqueId(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint64_t* uniqueId,
                                              cy_en_smif_semp_reg_latency_code_t regLc,
                                              cy_en_smif_trx_type_t trx,
                                              cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDUID_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDUID_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDUID_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDUID_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                             (uint8_t*)uniqueId,
                             8,
                             regLc,
                             cmdLatTbls,
                             context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadStatusRegister1
****************************************************************************//**
*
* This function reads the Status Register 1 contents.
* Supported By (S25H, S26H, S28H) series.
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
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadStatusRegister1(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     uint8_t* readData,
                                                     cy_en_smif_semp_reg_latency_code_t regLc,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDSR1_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDSR1_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDSR1_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDSR1_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadStatusRegister2
****************************************************************************//**
*
* This function reads the Status Register 2 contents.
* Supported By (S25H, S26H, S28H) series.
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
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadStatusRegister2(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     uint8_t* readData,
                                                     cy_en_smif_semp_reg_latency_code_t regLc,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDSR2_0_0,      // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDSR2_0_0_QUAD, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDSR2_4_0,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDSR2_4_0,      // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadAnyNonVolatileRegister
****************************************************************************//**
*
* This function provides a way to read all addressed nonvolatile
* device registers.
* Supported By (S25H, S26H, S28H) series.
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
* \ref cy_en_smif_semp_reg_addr_t
*
* \param lc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_read_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadAnyNonVolatileRegister(volatile cy_stc_smif_reg_t* base,
                                                            cy_en_smif_slave_select_t slave,
                                                            uint8_t* readData,
                                                            cy_en_smif_semp_reg_addr_t regAddr,
                                                            cy_en_smif_semp_read_latency_code_t lc,
                                                            cy_en_smif_trx_addr_len_t addrLen,
                                                            cy_en_smif_trx_type_t trx,
                                                            cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDARG_C_0_NV, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDARG_C_0_NV, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDARG_4_0_NV, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDARG_4_0_NV, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Parameter Check ***/
    if(((uint32_t)regAddr & 0x00800000UL) != 0x0UL)
    {
        // volatile register address
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
                             addrLen,
                             regAddr,
                             (uint8_t*)tempRead,
                             readSize,
                             lc,
                             cmdLatTbls,
                             context);
    *readData = tempRead[0];

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadAnyVolatileRegister
****************************************************************************//**
*
* This function provides a way to read all addressed volatile
* device registers.
* Supported By (S25H, S26H, S28H) series.
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
* \ref cy_en_smif_semp_reg_addr_t
*
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadAnyVolatileRegister(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         uint8_t* readData,
                                                         cy_en_smif_semp_reg_addr_t regAddr,
                                                         cy_en_smif_semp_reg_latency_code_t regLc,
                                                         cy_en_smif_trx_addr_len_t addrLen,
                                                         cy_en_smif_trx_type_t trx,
                                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDARG_C_0_V, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDARG_C_0_V, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDARG_4_0_V, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDARG_4_0_V, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Parameter Check ***/
    if(((uint32_t)regAddr & 0x00800000UL) == 0x0UL)
    {
        // volatile register address
        return CY_SMIF_BAD_PARAM;
    }

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx          = { trx };
    uint8_t               tempRead[4];
    uint8_t               readSize;
    if(((cy_en_smif_txfr_width_t)tx.bf.width == CY_SMIF_WIDTH_OCTAL) && ((cy_en_smif_data_rate_t)tx.bf.dataRate == CY_SMIF_DDR))
    {
        readSize = 4;
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
                             regLc,
                             cmdLatTbls,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteEnable
****************************************************************************//**
*
* This function sets the Write Enable Latch bit of the Status Register 1 to '1'
* to enable write, program and erase transactions.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteEnable(volatile cy_stc_smif_reg_t* base,
                                             cy_en_smif_slave_select_t slave,
                                             cy_en_smif_trx_type_t trx,
                                             cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_WRENB_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteDisable
****************************************************************************//**
*
* This function sets the Write Enable Latch bit of the Status Register 1 to '0'
* to disable write, program and erase transactions.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteDisable(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              cy_en_smif_trx_type_t trx,
                                              cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_WRDIS_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteAnyRegister
****************************************************************************//**
*
* This function provides a way to write all addressed nonvolatile and volatile
* device registers.
* Supported By (S25H, S26H, S28H) series.
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
* \ref cy_en_smif_semp_reg_addr_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteAnyRegister(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint8_t writeData,
                                                  cy_en_smif_semp_reg_addr_t regAddr,
                                                  cy_en_smif_trx_addr_len_t addrLen,
                                                  cy_en_smif_trx_type_t trx,
                                                  cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRARG_C_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRARG_C_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_WRARG_4_1, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_WRARG_4_1, // CY_SPI_TRANSACTION_8D8D8D
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
                              cmdLatTbls,
                              context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_SEMP_ClearProgramAndEraseFailureFlags
****************************************************************************//**
*
* This function resets Erase failure flag and Program failure flag
* in Status Register 1.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_ClearProgramAndEraseFailureFlags(volatile cy_stc_smif_reg_t* base,
                                                                  cy_en_smif_slave_select_t slave,
                                                                  cy_en_smif_trx_type_t trx,
                                                                  cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_CLPEF_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadECC_DataUnitStatus
****************************************************************************//**
*
* This function is used to determine the ECC status of the addressed data unit.
* Supported By (S26H, S28H) series.
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
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_read_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadECC_DataUnitStatus(volatile cy_stc_smif_reg_t* base,
                                                        cy_en_smif_slave_select_t slave,
                                                        uint8_t* readStatus,
                                                        uint32_t unitAdder,
                                                        cy_en_smif_semp_read_latency_code_t lc,
                                                        cy_en_smif_trx_type_t trx,
                                                        cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDECC_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);
    *readStatus = tempRead[0];

    return status;
}



/*******************************************************************************
* Function Cy_SMIF_SEMP_ClearECC_StatusRegister
****************************************************************************//**
*
* This function resets ECC Status Register bit[4] (2-bit ECC Detection), 
* ECC Status Register bit[3] (1-bit ECC Correction), Address Trap Register
* and ECC Detection Counter.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_ClearECC_StatusRegister(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         cy_en_smif_trx_type_t trx,
                                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_CLECC_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_CLECC_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_CLECC_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_CLECC_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadInterfaceCRCRegister
****************************************************************************//**
*
* This function allows the volatile Interface CRC Register contents to be read.
* Supported By (S28H) series. Only for 8D8D8D transaction.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readICRV
* Pointer to memory in which read register value to be stored.
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadInterfaceCRCRegister(volatile cy_stc_smif_reg_t* base,
                                                          cy_en_smif_slave_select_t slave,
                                                          uint32_t* readICRV,
                                                          cy_en_smif_trx_type_t trx,
                                                          cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_RD_CMD_INVALID, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_RD_CMD_INVALID, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_RD_CMD_INVALID, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RDCRC_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                             (uint8_t*)readICRV,
                             4,
                             0,
                             cmdLatTbls,
                             context);

    *readICRV = Cy_SMIF_Reverse4ByteEndian(*readICRV);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_SEMP_DataIntegrityCheck
****************************************************************************//**
*
* This function causes the device to perform a Data Integrity Check over a 
* user defined address range. 
* Supported By (S25H, S26H, S28H) series.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param startAddress
* Value to be loaded into the CRC Start Address Register and identifying
* the beginning of the address range that will be covered by the CRC calculation.
*
* \param endAddress
* Value to be loaded into the CRC End Address Register and identifying
* the ending of the address range that will be covered by the CRC calculation.
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
cy_en_smif_status_t Cy_SMIF_SEMP_DataIntegrityCheck(volatile cy_stc_smif_reg_t* base,
                                                    cy_en_smif_slave_select_t slave,
                                                    uint32_t startAddress,
                                                    uint32_t endAddress,
                                                    cy_en_smif_trx_type_t trx,
                                                    cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_DICHK_4_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_DICHK_4_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_DICHK_4_1, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_DICHK_4_1, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx         = { trx };
    uint32_t              endAddrRev = Cy_SMIF_Reverse4ByteEndian(endAddress);

    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_4BYTE,
                              startAddress,
                              (uint8_t*)&endAddrRev,
                              4ul,
                              cmdLatTbls,
                              context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_SEMP_Erase_4KB_Sector
****************************************************************************//**
*
* This function sets all the bits of a selected 4 KB sector to 1
* (all bytes are FFh).
* Make sure the device is configured as Hybrid Sector Architecture mode.
* This transaction is ignored when the device is configured for uniform sectors
* only (CFR3V[3] = 1). If the Erase 4 KB sector transaction is issued to
* a non-4 KB sector address, the device will abort the operation and will not
* set the ERSERR status fail bit.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Erase_4KB_Sector(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint32_t erasedAddress,
                                                  cy_en_smif_trx_type_t trx,
                                                  cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_ER004_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_ER004_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_ER004_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_ER004_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                                 cmdLatTbls,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Erase_256KB_Sector
****************************************************************************//**
*
* The Erase 256 KB Sector transaction sets all bits in the addressed sector to 1
* (all bytes are FFh).
* A device configuration option (CFR3V[3]) determines if the Hybrid Sector
* Architecture is in use.
* When CFR3V[3] = 0, 4 KB sectors overlay a portion of the highest or lowest
* address 128 KB or 64 KB of the device address space. If a sector erase command
* is applied to a 256 KB sector that is overlaid by 4 KB sectors,
* the overlaid 4 KB sectors are not affected by the erase. Only the visible (non-
* overlaid) portion of the 128 KB or 192 KB sector is erased.
* When CFR3V[3] = 1, there are no 4 KB sectors in the device address space and
* the Sector Erase command always operates on fully visible 256 KB sectors.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Erase_256KB_Sector(volatile cy_stc_smif_reg_t* base,
                                                    cy_en_smif_slave_select_t slave,
                                                    uint32_t erasedAddress,
                                                    cy_en_smif_trx_type_t trx,
                                                    cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_ER256_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_ER256_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_ER256_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_ER256_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                                 cmdLatTbls,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_EraseChip
****************************************************************************//**
*
* This function sets all bits to 1 (all bytes are FFh) inside the entire flash
* memory array. 
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_EraseChip(volatile cy_stc_smif_reg_t* base,
                                           cy_en_smif_slave_select_t slave,
                                           cy_en_smif_trx_type_t trx,
                                           cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_ERCHP_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_EvaluateEraseStatus
****************************************************************************//**
*
* This function verifies that the last erase operation on the addressed sector 
* was completed successfully. 
* If the selected sector was successfully erased, then the erase status bit
* (STR2V[2]) is set to 1. If the selected sector was not completely erased
* STR2V[2] is 0. The Write/Program Enable transaction (to set the WRPGEN bit)
* is not required before this transaction. However, the RDYBSY bit is set by
* the device itself and cleared at the end of the operation, as visible in
* STR1V[0] when reading status.
* Supported By (S26H, S28H) series.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param erasedAddress
* Address to a sector to be checked its erase status.
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
cy_en_smif_status_t Cy_SMIF_SEMP_EvaluateEraseStatus(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     uint32_t erasedAddress,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_EVERS_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_EVERS_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_EVERS_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_EVERS_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                                 cmdLatTbls,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_SectorEraseCount
****************************************************************************//**
*
* This function counts the number of erase cycles for the addressed sector.
* The erase cycle count is stored in the Sector Erase Count (SECV[22:0]) Register,
* and can be read by using the Read Any Register transaction. The RDYBSY bit is
* set by the device itself and cleared at the end of the operation,
* as visible in STR1V[0] when reading status
* Supported By (S26H, S28H) series.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param erasedAddress
* Address to a sector to be checked its erase cycles.
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
cy_en_smif_status_t Cy_SMIF_SEMP_SectorEraseCount(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint32_t erasedAddress,
                                                  cy_en_smif_trx_type_t trx,
                                                  cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_addr_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_SEERC_4_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_SEERC_4_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_SEERC_4_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_SEERC_4_0, // CY_SPI_TRANSACTION_8D8D8D
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
                                 cmdLatTbls,
                                 context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_SuspendEraseProgram
****************************************************************************//**
*
* This function suspends erase, program, and data integrity check for S26H and S28H
* series, and suspends erase and program for S28H series.
* Supported By (S25H, S26H, S28H) series. 
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
cy_en_smif_status_t Cy_SMIF_SEMP_SuspendEraseProgram(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_SPEPD_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_SPEPD_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_SPEPD_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_SPEPD_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_ResumeEraseProgram
****************************************************************************//**
*
* This function resumes erase, program, and data integrity check for S26H and S28H
* series, and resumes erase and program for S28H series.
* Supported By (S25H, S26H, S28H) series. 
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
cy_en_smif_status_t Cy_SMIF_SEMP_ResumeEraseProgram(volatile cy_stc_smif_reg_t* base,
                                                    cy_en_smif_slave_select_t slave,
                                                    cy_en_smif_trx_type_t trx,
                                                    cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RSEPD_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RSEPD_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_RSEPD_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_RSEPD_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);
    return status;
}

/* Secure Silicon Region Command and Advanced Sector Protection Command are not implemented yet! */

/*******************************************************************************
* Function Cy_SMIF_SEMP_SoftwareResetEnable
****************************************************************************//**
*
* This function is required immediately before a software reset transaction
* (Cy_SMIF_SEMP_SoftwareReset).
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_SoftwareResetEnable(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     cy_en_smif_trx_type_t trx,
                                                     cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_SRSTE_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_SoftwareReset
****************************************************************************//**
*
* This function restores the device to its initial power up state,
* by reloading volatile registers from nonvolatile default values.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_SoftwareReset(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               cy_en_smif_trx_type_t trx,
                                               cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_SFRST_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_EnterDeepPowerDownMode
****************************************************************************//**
*
* This function shifts device in the lowest power consumption mode.
* Supported By (S25H, S26H, S28H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_EnterDeepPowerDownMode(volatile cy_stc_smif_reg_t* base,
                                                        cy_en_smif_slave_select_t slave,
                                                        cy_en_smif_trx_type_t trx,
                                                        cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_2BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_2BYTE_ENDPD_0_0, // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_SEMP_EnableHyperBusInterface
****************************************************************************//**
*
* This function enables the HyperBus interface by writing CFR3V.INTFTP bit in
* legacy SPI mode, if HyperBus interface is not already enabled.
* Supported By (S26H) series.
*
* \warning
* This function 
* a) will initialize/de-initialize SMIF/SMIF_DEVICE registers, so the calling code
*    should be able to cope with this
* b) assumes that
*    - the non-volatile registers of the S26H have not been changed because it relies
*      on the specified initial values of the volatile registers
*    - it is not called while SMIF (external) frequency is above 50 MHz, which is the 
*      safe frequency for accessing registers with 0 latency in SPI mode 
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_SEMP_EnableHyperBusInterface(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave)
{
    volatile cy_stc_smif_reg_device_t *device = Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect(base, slave);

    cy_stc_smif_context_t smifContext = {0ul};
    cy_stc_smif_config_t  smifSpiConfig =
    {
        .txClk         = CY_SMIF_INV_FOR_DDR,
        .rxClk         = CY_SMIF_INV_OUTPUT_CLK,
        .dlpAuto       = CY_SMIF_DLP_UPDATE_MANUAL,
        .capDelay      = CY_SMIF_CAPTURE_DELAY_0_CYCLE,
        .delaySel      = CY_SMIF_1_SEN_SEL_PER_TAP,
        .deselectDelay = CY_SMIF_MIN_DESELECT_1_CLK,
        .setupDelay    = CY_SMIF_SETUP_0_CLK_PULUS_MIN,
        .holdDelay     = CY_SMIF_HOLD_0_CLK_PULUS_MIN,
        .mode          = CY_SMIF_NORMAL,
        .blockEvent    = CY_SMIF_BUS_ERROR,
    };

    Cy_SMIF_Init(base, &smifSpiConfig, 1000, &smifContext);
    Cy_SMIF_DeviceSetDataSelect(device, CY_SMIF_DATA_SEL0);
    Cy_SMIF_DeviceWriteEnable(device);
    Cy_SMIF_Enable(base, &smifContext);

    // Try to read the Manufacturer ID in SPI mode -> If it matches the expected value, the external
    // memory (S26H) seems to be still in Legacy SPI interface mode and we can go ahead with configuration.
    // Otherwise, we just assume that the device was already brought into HyperBus interface mode
    // (e.g. MCU was reset, but external memory was not reset)
    cy_stc_smif_semp_id_t readId;
    Cy_SMIF_SEMP_ReadId(base, slave, &readId, CY_SMIF_SEMP_REG_LATENCY0, CY_SPI_TRANSACTION_1S1S1S, sizeof(cy_stc_smif_semp_id_t), &smifContext);
    if(readId.manufacturerId == CY_SMIF_CYPRESS_MANUFACTURER_ID)
    {
        // Setting Configuration Register 3
        un_cy_smif_semp_CFR3_t writeConfigReg3_0 = {0};
        writeConfigReg3_0.field.u1UNHYSA = 1; // Keep register default value for this bit (uniform sector architecture)
        writeConfigReg3_0.field.u1INTFTP = 1; // Hyper Bus Interface Mode
        Cy_SMIF_SEMP_WriteEnable(base, slave, CY_SPI_TRANSACTION_1S1S1S, &smifContext);
        Cy_SMIF_SEMP_WriteAnyRegister(base, slave, writeConfigReg3_0.u8, CY_SEMP_REG_ADDR_CFR3_V, CY_TRX_ADDR_3BYTE, CY_SPI_TRANSACTION_1S1S1S, &smifContext);
    }

    Cy_SMIF_DeInit(base);

    return CY_SMIF_SUCCESS;
}


/***************************************************************************
* Functions For Only S25H series 
****************************************************************************/

/*******************************************************************************
* Function Cy_SMIF_SEMP_ReadConfigurationRegister1
****************************************************************************//**
*
* This function allows the Configuration Register-1 contents to be read. 
* Supported By (S25H) series.
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
* \param regLc
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_ReadConfigurationRegister1(volatile cy_stc_smif_reg_t* base,
                                                            cy_en_smif_slave_select_t slave,
                                                            uint8_t* readData,
                                                            cy_en_smif_semp_reg_latency_code_t regLc,
                                                            cy_en_smif_trx_type_t trx,
                                                            cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDCR1_0_0,      // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDCR1_0_0_QUAD, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_RD_CMD_INVALID,           // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_RD_CMD_INVALID,           // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);
    *readData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteEnableVolatileRegister
****************************************************************************//**
*
* This function enables write of volatile registers.
* Supported By (S25H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteEnableVolatileRegister(volatile cy_stc_smif_reg_t* base,
                                                             cy_en_smif_slave_select_t slave,
                                                             cy_en_smif_trx_type_t trx,
                                                             cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRENV_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRENV_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteRegister
****************************************************************************//**
*
* This function provides a way to write Status Register 1
* and Configuration Registers 1 - 4.
* Supported By (S25H) series.
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
* \param cfgReg2
* Value to be written into Configuration Register 2
*
* \param cfgReg3
* Value to be written into Configuration Register 3
*
* \param cfgReg4
* Value to be written into Configuration Register 4
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteRegister(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               uint8_t  stsReg,
                                               uint8_t  cfgReg1,
                                               uint8_t  cfgReg2,
                                               uint8_t  cfgReg3,
                                               uint8_t  cfgReg4,
                                               cy_en_smif_trx_type_t trx,
                                               cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[5] = { stsReg, cfgReg1, cfgReg2, cfgReg3, cfgReg4 };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              tempWrite,
                              5,
                              cmdLatTbls,
                              context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_SafeBootWriteRegister
****************************************************************************//**
*
* This function to recover the device from configuration corruption.
* (Is this used by user? need to check. YOTS)
* Supported By (S25H) series.
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
* \param cfgReg2
* Value to be written into Configuration Register 2
*
* \param cfgReg3
* Value to be written into Configuration Register 3
*
* \param cfgReg4
* Value to be written into Configuration Register 4
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
cy_en_smif_status_t Cy_SMIF_SEMP_SafeBootWriteRegister(volatile cy_stc_smif_reg_t* base,
                                                       cy_en_smif_slave_select_t slave,
                                                       uint8_t  stsReg,
                                                       uint8_t  cfgReg1,
                                                       uint8_t  cfgReg2,
                                                       uint8_t  cfgReg3,
                                                       uint8_t  cfgReg4,
                                                       cy_en_smif_trx_type_t trx,
                                                       cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRREG_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx           = { trx };
    uint8_t               tempWrite[6] = { stsReg, cfgReg1, cfgReg2, cfgReg3, cfgReg4, 0x00 };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              tempWrite,
                              6,
                              cmdLatTbls,
                              context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Enter4ByteAddressMode
****************************************************************************//**
*
* This function sets the Address Length bit CFR2V[7] to 1.
* Supported By (S25H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Enter4ByteAddressMode(volatile cy_stc_smif_reg_t* base,
                                                       cy_en_smif_slave_select_t slave,
                                                       cy_en_smif_trx_type_t trx,
                                                       cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_EN4BA_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_EN4BA_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Exit4ByteAddressMode
****************************************************************************//**
*
* This function sets the Address Length bit CFR2V[7] to 0.
* Supported By (S25H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Exit4ByteAddressMode(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      cy_en_smif_trx_type_t trx,
                                                      cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_EX4BA_0_0, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_EX4BA_0_0, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_CMD_INVALID,         // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx     = { trx };

    /*** Transaction for Command Address Dummy ***/
    status = Cy_SMIF_TX_CMD(base,
                            slave,
                            trx,
                            rdCmdArray[tx.bf.tblNo],
                            cmdLatTbls,
                            context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Read_DLP_Register
****************************************************************************//**
*
* This function reads the DLP pattern.
* Supported By (S25H) series.
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
* Latency code specified by the semper flash.
* \ref cy_en_smif_semp_reg_latency_code_t
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
cy_en_smif_status_t Cy_SMIF_SEMP_Read_DLP_Register(volatile cy_stc_smif_reg_t* base,
                                                   cy_en_smif_slave_select_t slave,
                                                   uint8_t* dlpData,
                                                   cy_en_smif_semp_reg_latency_code_t regLc,
                                                   cy_en_smif_trx_type_t trx,
                                                   cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_rd_cmd_t rdCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_RDDLP_0_0,      // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_RDDLP_0_0_QUAD, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_RD_CMD_INVALID,           // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_RD_CMD_INVALID,           // CY_SPI_TRANSACTION_8D8D8D
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
                             cmdLatTbls,
                             context);
    *dlpData = tempRead[0];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Program_DLP_NV_Register
****************************************************************************//**
*
* This function programs DLP pattern into the Nonvolatile registers.
* Supported By (S25H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Program_DLP_NV_Register(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         uint8_t dlpData,
                                                         cy_en_smif_trx_type_t trx,
                                                         cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_PRDLP_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_PRDLP_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
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
                              cmdLatTbls,
                              context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_Write_DLP_V_Register
****************************************************************************//**
*
* This function writes DLP pattern into the Volatile register.
* Supported By (S25H) series.
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
cy_en_smif_status_t Cy_SMIF_SEMP_Write_DLP_V_Register(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      uint8_t dlpData,
                                                      cy_en_smif_trx_type_t trx,
                                                      cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRDLP_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRDLP_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
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
                              cmdLatTbls,
                              context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_SEMP_WriteAutoBootRegister
****************************************************************************//**
*
* This function writes AutoBoot pattern into the register.
* Supported By (S25H) series.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param autoBootRegVal
* Value to be written into the AutoBoot register.
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
cy_en_smif_status_t Cy_SMIF_SEMP_WriteAutoBootRegister(volatile cy_stc_smif_reg_t* base,
                                                       cy_en_smif_slave_select_t slave,
                                                       uint32_t autoBootRegVal,
                                                       cy_en_smif_trx_type_t trx,
                                                       cy_stc_smif_context_t* context)
{
    /*** Specify Transaction Parameters for Each Transaction Type ***/
    static const cy_en_smif_semp_wt_cmd_t wtCmdArray[CY_SPI_TRANSACTION_TYPE_NUM] =
    {
        CY_SEMP_CMD_1BYTE_WRAUB_0_1, // CY_SPI_TRANSACTION_1S1S1S
        CY_SEMP_CMD_1BYTE_WRAUB_0_1, // CY_SPI_TRANSACTION_4S4S4S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8S8S8S
        CY_SEMP_WT_CMD_INVALID,      // CY_SPI_TRANSACTION_8D8D8D
    };

    /*** Local Auto Variables ***/
    cy_en_smif_status_t   status;
    cy_un_smif_trx_type_t tx                = { trx };
    uint32_t              autoBootRegValRev = Cy_SMIF_Reverse4ByteEndian(autoBootRegVal);

    status = Cy_SMIF_TX_WRITE(base,
                              slave,
                              trx,
                              wtCmdArray[tx.bf.tblNo],
                              CY_TRX_ADDR_3BYTE,
                              0,
                              (uint8_t*)&autoBootRegValRev,
                              4,
                              cmdLatTbls,
                              context);
    return status;
}

#if defined(__cplusplus)
}
#endif
