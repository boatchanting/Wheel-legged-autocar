/***************************************************************************//**
* \file cy_smif_s25fl.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides the source code for the S25FL HSSPI series driver APIs.
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

#include "cy_smif_s25fl.h"

/************************************************************/
/*************** Specify Read Command Sequence **************/
/************************************************************/
const cy_stc_cmd_sq_t g_s25flReadCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT},   // cmdTypeNum = 1
};

/************************************************************/
/*************** Specify Read Address Sequence **************/
/************************************************************/
const cy_stc_addr_sq_t g_s25flReadAddrTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 1
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 2
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 3
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 4
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 5
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 6
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 7
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 8
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 9
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 10
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 11
};

/************************************************************/
/**************** Specify Read Mode Sequence ****************/
/************************************************************/
const cy_stc_mode_sq_t g_s25flReadModeTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT  }, // modeTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 1
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 2
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 3
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 4
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 5
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR, .presence = CY_SMIF_PRESENT_1BYTE}, // modeTypeNum = 6
};

/************************************************************/
/**************** Specify Read Dummy Sequence ***************/
/************************************************************/
const cy_stc_dummy_sq_t g_s25flReadDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 0
    {.tableId = 1, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 1
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    {.tableId = 2, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 2
    {.tableId = 3, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 3
    {.tableId = 4, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 4
    {.tableId = 5, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 5
    {.tableId = 6, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 6
    {.tableId = 7, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 7
    {.tableId = 8, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE, .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 8
#endif
};

#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
  #ifdef S25FL_USE_LC_ENHANCED                                                 //LC: b'00 b'01 b'10 b'11
  static const uint8_t g_s25flLatancyTableOfRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {0u,  0u,  0u,  0u};
  static const uint8_t g_s25flLatancyTableOfFastRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX] = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfDOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]      = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfQOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]      = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfDIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {0u,  1u,  2u,  0u};
  static const uint8_t g_s25flLatancyTableOfQIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {4u,  4u,  5u,  1u};
  static const uint8_t g_s25flLatancyTableOfDDRFR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]    = {2u,  4u,  5u,  1u};
  static const uint8_t g_s25flLatancyTableOfDDRDIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]  = {4u,  5u,  6u,  2u};
  static const uint8_t g_s25flLatancyTableOfDDRQIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]  = {6u,  7u,  8u,  3u};
  #else                                                                        //LC: b'00 b'01 b'10 b'11
  static const uint8_t g_s25flLatancyTableOfRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {0u,  0u,  0u,  0u};
  static const uint8_t g_s25flLatancyTableOfFastRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX] = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfDOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]      = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfQOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]      = {8u,  8u,  8u,  0u};
  static const uint8_t g_s25flLatancyTableOfDIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {4u,  5u,  6u,  4u};
  static const uint8_t g_s25flLatancyTableOfQIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]     = {4u,  4u,  5u,  1u};
  static const uint8_t g_s25flLatancyTableOfDDRFR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]    = {5u,  6u,  7u,  4u};
  static const uint8_t g_s25flLatancyTableOfDDRDIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]  = {6u,  7u,  8u,  4u};
  static const uint8_t g_s25flLatancyTableOfDDRQIOR[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]  = {6u,  7u,  8u,  3u};
  #endif
#elif(CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
  static const uint8_t g_s25flLatancyTableOfRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX]          = {0u,  0u,  0u,  0u, 0u,  0u,  0u,  0u, 0u,  0u,  0u,  0u,  0u,  0u,  0u,  0u}; // CY_SMIF_READ, CY_SMIF_4READ;
  static const uint8_t g_s25flLatancyTableOfOtherThanRead[CY_SMIF_S25FL_READ_LATENCY_CODE_MAX] = {0u,  1u,  2u,  3u, 4u,  5u,  6u,  7u, 8u,  9u, 10u, 11u, 12u, 13u, 14u, 15u}; // Other than READ, 4READ;
#endif

const uint8_t* g_s25flReadLatencyCodeToDummyCycle[CY_SMIF_S25FL_READ_LC_DC_TABLE_MAX] =
{
    g_s25flLatancyTableOfRead,
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    g_s25flLatancyTableOfFastRead,
    g_s25flLatancyTableOfDOR,
    g_s25flLatancyTableOfQOR,
    g_s25flLatancyTableOfDIOR,
    g_s25flLatancyTableOfQIOR,
    g_s25flLatancyTableOfDDRFR,
    g_s25flLatancyTableOfDDRDIOR,
    g_s25flLatancyTableOfDDRQIOR,
#elif(CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
    g_s25flLatancyTableOfOtherThanRead,
#endif
};


/************************************************************/
/**************** Specify Read Data Sequence ****************/
/************************************************************/
const cy_stc_data_sq_t g_s25flReadDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 2
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_DDR}, // dataTypeNum = 3
    {.width = CY_SMIF_WIDTH_DUAL,   .dataRate = CY_SMIF_DDR}, // dataTypeNum = 4
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_DDR}, // dataTypeNum = 5
};

/************************************************************/
/************* Read Command Table for S25FLxxxX *************/
/************************************************************/
const cy_stc_read_info_t g_s25flReadCmdInfo[] =
{
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    { .cmd = CY_SMIF_S25FL_CMD_READ,       .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 0, .dataTypeNum = 0u }, // cmd id = 0,
    { .cmd = CY_SMIF_S25FL_CMD_4READ,      .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 0, .dataTypeNum = 0u }, // cmd id = 1,
    { .cmd = CY_SMIF_S25FL_CMD_FAST_READ,  .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 0u }, // cmd id = 2,
    { .cmd = CY_SMIF_S25FL_CMD_4FAST_READ, .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 0u }, // cmd id = 3,
    { .cmd = CY_SMIF_S25FL_CMD_DOR,        .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 2, .dataTypeNum = 1u }, // cmd id = 4,
    { .cmd = CY_SMIF_S25FL_CMD_4DOR,       .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 2, .dataTypeNum = 1u }, // cmd id = 5,
    { .cmd = CY_SMIF_S25FL_CMD_QOR,        .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 3, .dataTypeNum = 2u }, // cmd id = 6,
    { .cmd = CY_SMIF_S25FL_CMD_4QOR,       .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 3, .dataTypeNum = 2u }, // cmd id = 7,

  #ifdef S25FL_USE_LC_ENHANCED
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 0u, .addrTypeNum = 2u,  .modeTypeNum = 2u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 8,
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 1u, .addrTypeNum = 2u,  .modeTypeNum = 2u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 9,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 0u, .addrTypeNum = 3u,  .modeTypeNum = 2u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 10,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 1u, .addrTypeNum = 3u,  .modeTypeNum = 2u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 11,
  #else
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 0u, .addrTypeNum = 2u,  .modeTypeNum = 0u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 8,
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 1u, .addrTypeNum = 2u,  .modeTypeNum = 0u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 9,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 0u, .addrTypeNum = 3u,  .modeTypeNum = 0u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 10,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 1u, .addrTypeNum = 3u,  .modeTypeNum = 0u, .dummyTypeNum = 4, .dataTypeNum = 1u }, // cmd id = 11,
  #endif

    { .cmd = CY_SMIF_S25FL_CMD_QIOR,       .cmdTypeNum = 0u, .addrTypeNum = 4u,  .modeTypeNum = 3u, .dummyTypeNum = 5, .dataTypeNum = 2u }, // cmd id = 12,
    { .cmd = CY_SMIF_S25FL_CMD_QIOR,       .cmdTypeNum = 1u, .addrTypeNum = 4u,  .modeTypeNum = 3u, .dummyTypeNum = 5, .dataTypeNum = 2u }, // cmd id = 13,
    { .cmd = CY_SMIF_S25FL_CMD_4QIOR,      .cmdTypeNum = 0u, .addrTypeNum = 5u,  .modeTypeNum = 3u, .dummyTypeNum = 5, .dataTypeNum = 2u }, // cmd id = 14,
    { .cmd = CY_SMIF_S25FL_CMD_4QIOR,      .cmdTypeNum = 1u, .addrTypeNum = 5u,  .modeTypeNum = 3u, .dummyTypeNum = 5, .dataTypeNum = 2u }, // cmd id = 15,

    { .cmd = CY_SMIF_S25FL_CMD_DDRQIOR,    .cmdTypeNum = 0u, .addrTypeNum = 10u, .modeTypeNum = 6u, .dummyTypeNum = 8, .dataTypeNum = 5u }, // cmd id = 16
    { .cmd = CY_SMIF_S25FL_CMD_DDRQIOR,    .cmdTypeNum = 1u, .addrTypeNum = 10u, .modeTypeNum = 6u, .dummyTypeNum = 8, .dataTypeNum = 5u }, // cmd id = 17
    { .cmd = CY_SMIF_S25FL_CMD_4DDRQIOR,   .cmdTypeNum = 0u, .addrTypeNum = 11u, .modeTypeNum = 6u, .dummyTypeNum = 8, .dataTypeNum = 5u }, // cmd id = 18
    { .cmd = CY_SMIF_S25FL_CMD_4DDRQIOR,   .cmdTypeNum = 1u, .addrTypeNum = 11u, .modeTypeNum = 6u, .dummyTypeNum = 8, .dataTypeNum = 5u }, // cmd id = 19

  #ifdef S25FL_USE_LC_ENHANCED
    { .cmd = CY_SMIF_S25FL_CMD_DDRFR,      .cmdTypeNum = 0u, .addrTypeNum = 6u,  .modeTypeNum = 4u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 20,
    { .cmd = CY_SMIF_S25FL_CMD_DDRFR,      .cmdTypeNum = 1u, .addrTypeNum = 6u,  .modeTypeNum = 4u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 21,
    { .cmd = CY_SMIF_S25FL_CMD_4DDRFR,     .cmdTypeNum = 0u, .addrTypeNum = 7u,  .modeTypeNum = 4u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 22,
    { .cmd = CY_SMIF_S25FL_CMD_4DDRFR,     .cmdTypeNum = 1u, .addrTypeNum = 7u,  .modeTypeNum = 4u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 23,
  #else
    { .cmd = CY_SMIF_S25FL_CMD_DDRFR,      .cmdTypeNum = 0u, .addrTypeNum = 6u,  .modeTypeNum = 0u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 20,
    { .cmd = CY_SMIF_S25FL_CMD_DDRFR,      .cmdTypeNum = 1u, .addrTypeNum = 6u,  .modeTypeNum = 0u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 21,
    { .cmd = CY_SMIF_S25FL_CMD_4DDRFR,     .cmdTypeNum = 0u, .addrTypeNum = 7u,  .modeTypeNum = 0u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 22,
    { .cmd = CY_SMIF_S25FL_CMD_4DDRFR,     .cmdTypeNum = 1u, .addrTypeNum = 7u,  .modeTypeNum = 0u, .dummyTypeNum = 6, .dataTypeNum = 3u }, // cmd id = 23,
  #endif

  #ifdef S25FL_USE_LC_ENHANCED
    { .cmd = CY_SMIF_S25FL_CMD_DDRDIOR,    .cmdTypeNum = 0u, .addrTypeNum = 8u,  .modeTypeNum = 5u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 24
    { .cmd = CY_SMIF_S25FL_CMD_DDRDIOR,    .cmdTypeNum = 1u, .addrTypeNum = 8u,  .modeTypeNum = 5u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 25
    { .cmd = CY_SMIF_S25FL_CMD_4DDRDIOR,   .cmdTypeNum = 0u, .addrTypeNum = 9u,  .modeTypeNum = 5u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 26
    { .cmd = CY_SMIF_S25FL_CMD_4DDRDIOR,   .cmdTypeNum = 1u, .addrTypeNum = 9u,  .modeTypeNum = 5u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 27
  #else
    { .cmd = CY_SMIF_S25FL_CMD_DDRDIOR,    .cmdTypeNum = 0u, .addrTypeNum = 8u,  .modeTypeNum = 0u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 24
    { .cmd = CY_SMIF_S25FL_CMD_4DDRDIOR,   .cmdTypeNum = 1u, .addrTypeNum = 9u,  .modeTypeNum = 0u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 25
    { .cmd = CY_SMIF_S25FL_CMD_DDRDIOR,    .cmdTypeNum = 0u, .addrTypeNum = 8u,  .modeTypeNum = 0u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 26
    { .cmd = CY_SMIF_S25FL_CMD_DDRDIOR,    .cmdTypeNum = 1u, .addrTypeNum = 8u,  .modeTypeNum = 0u, .dummyTypeNum = 7, .dataTypeNum = 4u }, // cmd id = 27
  #endif
#elif (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
    { .cmd = CY_SMIF_S25FL_CMD_READ,       .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 0, .dataTypeNum = 0u }, // cmd id = 0,
    { .cmd = CY_SMIF_S25FL_CMD_4READ,      .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 0, .dataTypeNum = 0u }, // cmd id = 1,
    { .cmd = CY_SMIF_S25FL_CMD_FAST_READ,  .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 0u }, // cmd id = 2,
    { .cmd = CY_SMIF_S25FL_CMD_4FAST_READ, .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 0u }, // cmd id = 3,
    { .cmd = CY_SMIF_S25FL_CMD_DOR,        .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 4,
    { .cmd = CY_SMIF_S25FL_CMD_4DOR,       .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 5,
    { .cmd = CY_SMIF_S25FL_CMD_QOR,        .cmdTypeNum = 0u, .addrTypeNum = 0u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 6,
    { .cmd = CY_SMIF_S25FL_CMD_4QOR,       .cmdTypeNum = 0u, .addrTypeNum = 1u,  .modeTypeNum = 0u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 7,
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 0u, .addrTypeNum = 2u,  .modeTypeNum = 2u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 8,
    { .cmd = CY_SMIF_S25FL_CMD_DIOR,       .cmdTypeNum = 1u, .addrTypeNum = 2u,  .modeTypeNum = 2u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 9,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 0u, .addrTypeNum = 3u,  .modeTypeNum = 2u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 10,
    { .cmd = CY_SMIF_S25FL_CMD_4DIOR,      .cmdTypeNum = 1u, .addrTypeNum = 3u,  .modeTypeNum = 2u, .dummyTypeNum = 1, .dataTypeNum = 1u }, // cmd id = 11,
    { .cmd = CY_SMIF_S25FL_CMD_QIOR,       .cmdTypeNum = 0u, .addrTypeNum = 4u,  .modeTypeNum = 3u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 12,
    { .cmd = CY_SMIF_S25FL_CMD_QIOR,       .cmdTypeNum = 1u, .addrTypeNum = 4u,  .modeTypeNum = 3u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 13,
    { .cmd = CY_SMIF_S25FL_CMD_4QIOR,      .cmdTypeNum = 0u, .addrTypeNum = 5u,  .modeTypeNum = 3u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 14,
    { .cmd = CY_SMIF_S25FL_CMD_4QIOR,      .cmdTypeNum = 1u, .addrTypeNum = 5u,  .modeTypeNum = 3u, .dummyTypeNum = 1, .dataTypeNum = 2u }, // cmd id = 15,
    { .cmd = CY_SMIF_S25FL_CMD_DDRQIOR,    .cmdTypeNum = 0u, .addrTypeNum = 10u, .modeTypeNum = 6u, .dummyTypeNum = 1, .dataTypeNum = 5u }, // cmd id = 16
    { .cmd = CY_SMIF_S25FL_CMD_DDRQIOR,    .cmdTypeNum = 1u, .addrTypeNum = 10u, .modeTypeNum = 6u, .dummyTypeNum = 1, .dataTypeNum = 5u }, // cmd id = 17
    { .cmd = CY_SMIF_S25FL_CMD_4DDRQIOR,   .cmdTypeNum = 0u, .addrTypeNum = 11u, .modeTypeNum = 6u, .dummyTypeNum = 1, .dataTypeNum = 5u }, // cmd id = 18
    { .cmd = CY_SMIF_S25FL_CMD_4DDRQIOR,   .cmdTypeNum = 1u, .addrTypeNum = 11u, .modeTypeNum = 6u, .dummyTypeNum = 1, .dataTypeNum = 5u }, // cmd id = 19
#endif
};

/************************************************************/
/*************** Specify Write Command Sequence *************/
/************************************************************/
const cy_stc_cmd_sq_t g_s25flWriteCmdTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_PRESENT_1BYTE}, // cmdTypeNum = 0
};

/************************************************************/
/*************** Specify Write Address Sequence *************/
/************************************************************/
const cy_stc_addr_sq_t g_s25flWriteAddrTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_3_BYTE}, // addrTypeNum = 0
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .addrLen = CY_SMIF_XIP_ADDRESS_4_BYTE}, // addrTypeNum = 1
};

/************************************************************/
/**************** Specify Write Mode Sequence ***************/
/************************************************************/
const cy_stc_mode_sq_t g_s25flWriteModeTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT  }, // modeTypeNum = 0
};

/************************************************************/
/**************** Specify Write Dummy Sequence **************/
/************************************************************/
const cy_stc_dummy_sq_t g_s25flWriteDummyTypes[] =
{
    {.tableId = 0, .width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR, .presence = CY_SMIF_NOT_PRESENT,   .doubleDm = false, .rwdsInDm = false, .dlpEn = false, .rwdsEn = false}, // dummyTypeNum = 0
};

static const uint8_t g_latancyTableOfWrite[CY_SMIF_S25FL_WRITE_LC_DC_TABLE_MAX]     = {0u,};
const uint8_t* g_s25flWriteLatencyCodeToDummyCycle[CY_SMIF_S25FL_WRITE_LATENCY_CODE_MAX] =
{
   g_latancyTableOfWrite,
};

/************************************************************/
/**************** Specify Write Data Sequence ***************/
/************************************************************/
const cy_stc_data_sq_t g_s25flWriteDataTypes[] =
{
    {.width = CY_SMIF_WIDTH_SINGLE, .dataRate = CY_SMIF_SDR}, // dataTypeNum = 0
    {.width = CY_SMIF_WIDTH_QUAD,   .dataRate = CY_SMIF_SDR}, // dataTypeNum = 1
};

/************************************************************/
/************* Write Command Table for S25FLxxxX ************/
/************************************************************/
const cy_stc_write_info_t g_s25flWriteCmdInfo[] =
{
    { .cmd = CY_SMIF_S25FL_CMD_PP,   .cmdTypeNum   = 0u, .addrTypeNum  = 0u, .modeTypeNum  = 0u, .dataTypeNum  = 0u },
    { .cmd = CY_SMIF_S25FL_CMD_4PP,  .cmdTypeNum   = 0u, .addrTypeNum  = 1u, .modeTypeNum  = 0u, .dataTypeNum  = 0u },
    { .cmd = CY_SMIF_S25FL_CMD_QPP,  .cmdTypeNum   = 0u, .addrTypeNum  = 0u, .modeTypeNum  = 0u, .dataTypeNum  = 1u },
    { .cmd = CY_SMIF_S25FL_CMD_4QPP, .cmdTypeNum   = 0u, .addrTypeNum  = 1u, .modeTypeNum  = 0u, .dataTypeNum  = 1u },
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    { .cmd = CY_SMIF_S25FL_CMD_QPPA, .cmdTypeNum   = 0u, .addrTypeNum  = 0u, .modeTypeNum  = 0u, .dataTypeNum  = 1u },
#endif
};


/***********************************************************************/
/******************* Functions Issuing Each Commands *******************/
/***********************************************************************/
/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadJedecId
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadJedecId(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              cy_stc_JedecId_t* readData,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDID, /* command value */
                                     false,                  /* is command 2Byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     NOT_LAST_COMMAND,       /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, (uint8_t*)readData, 0x51u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadStatusRegister1
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadStatusRegister1(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      uint8_t* readData,
                                                      cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDSR1, /* command value */
                                     false,                   /* is command 2byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadStatusRegister2
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadStatusRegister2(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      uint8_t* readData,
                                                      cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDSR2, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadConfigurationRegister
****************************************************************************//**
*
* This function reads the Configuration Register.
* Read data does not support multiple slave access at the same time.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister(volatile cy_stc_smif_reg_t* base,
                                                            cy_en_smif_slave_select_t slave,
                                                            uint8_t* readData,
                                                            cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDCR1, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteStatusAndConfigRegister
****************************************************************************//**
*
* This function provides a way to write Status Register and
* Configuration Register.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeRegs
* Pointer to memory which contain data to be written into registers.
* The data will be written in below order.
* first byte:  Status Register.
* second byte: Configuration Register 1
* third byte:  Configuration Register 2 (only for S25FL)
* forth byte:  Configuration Register 3 (only for S25FL)
*
* \param writeRegNum
* Number of register to be written into.
* i.e. If intending to write only Status Register, this value should be "1".
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteStatusAndConfigRegister(volatile cy_stc_smif_reg_t* base,
                                                               cy_en_smif_slave_select_t slave,
                                                               stc_cy_smif_s25f_write_reg_t* writeRegs,
                                                               uint8_t writeRegNum,
                                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_WRR, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate */
                                     (uint8_t*)writeRegs,   /* Data pointer */
                                     writeRegNum,           /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width */
                                     CY_SMIF_SDR,           /* data rate */
                                     slave,                 /* slave select */
                                     LAST_COMMAND_BYTE,     /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteDisable
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteDisable(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_WRDI, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteEnable
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteEnable(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_WREN, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ClearStatusRegister
****************************************************************************//**
*
* The Clear Status Register command clears the WIP (SR1V[0]), WEL (SR1V[1]),
* P_ERR (SR2V[5]), and E_ERR (SR2V[6]) bits to  g0h.
* Write Enable is required before calling this function.

* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ClearStatusRegister(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_CLSR, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_DataLeaningPatternRead
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
* \param readDLP
* Pointer to memory in which read DLP data to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_DataLeaningPatternRead(volatile cy_stc_smif_reg_t* base,
                                                         cy_en_smif_slave_select_t slave,
                                                         uint8_t* readDLP,
                                                         cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_DLPRD, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);

    status = Cy_SMIF_ReceiveDataBlocking(base, readDLP, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ProgramNVDLR
****************************************************************************//**
*
* This function programs DLP pattern into the Nonvolatile registers.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* DLP data value to be written into the Nonvolatile register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramNVDLR(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               uint8_t writeData,
                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PDLRNV, /* command value */
                                     false,                    /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,     /* command width */
                                     CY_SMIF_SDR,              /* command rate */
                                     &writeData,               /* Data pointer */
                                     1,                        /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,     /* data width */
                                     CY_SMIF_SDR,              /* data rate */
                                     slave,                    /* slave select */
                                     LAST_COMMAND_BYTE,        /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteVDLR
****************************************************************************//**
*
* This function writes DLP pattern into the Volatile register.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* DLP data value to be written to the Volatile register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteVDLR(volatile cy_stc_smif_reg_t* base,
                                            cy_en_smif_slave_select_t slave,
                                            uint8_t writeData,
                                            cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_WDLRV, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     &writeData,              /* Data pointer */
                                     1,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_SectorErase
****************************************************************************//**
*
* This function sets all bits in the addressed sector to 1 (all bytes are FFh).
* Write Enable is required before calling this function.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_SectorErase(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint32_t erasedAddress,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t    status;
    en_cy_smif_s25fl_cmd_t eraseCmd;
    uint8_t                addrSizeInByte;
    uint32_t               addrInBigEndian;

    if(erasedAddress > 0x00FFFFFFul)
    {
        eraseCmd        = CY_SMIF_S25FL_CMD_4SE;
        addrSizeInByte  = 4ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(erasedAddress);
    }
    else
    {
        // assuming that ExtAdd == 0;
        eraseCmd        = CY_SMIF_S25FL_CMD_SE;
        addrSizeInByte  = 3ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(erasedAddress << 8ul);
    }

    status = Cy_SMIF_TransmitCommand(base,
                                     (uint8_t)eraseCmd,          /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer */
                                     addrSizeInByte,             /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width */
                                     CY_SMIF_SDR,                /* data rate */
                                     slave,                      /* slave select */
                                     LAST_COMMAND_BYTE,          /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ChipErase
****************************************************************************//**
*
* This function sets all bits to 1 (all bytes are FFh) inside the entire flash
* memory array. 
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ChipErase(volatile cy_stc_smif_reg_t* base,
                                            cy_en_smif_slave_select_t slave,
                                            cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_CE, /* command value */
                                     false,                /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE, /* command width */
                                     CY_SMIF_SDR,          /* command rate */
                                     NULL,                 /* Data pointer */
                                     0,                    /* Data size */
                                     CY_SMIF_WIDTH_SINGLE, /* data width */
                                     CY_SMIF_SDR,          /* data rate */
                                     slave,                /* slave select */
                                     LAST_COMMAND_BYTE,    /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PasswordRead
****************************************************************************//**
*
* This function reads the Password Register value.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readPW
* Pointer to memory in which read password to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordRead(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               uint64_t* readPW,
                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PASSRD, /* command value */
                                     false,                    /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,     /* command width */
                                     CY_SMIF_SDR,              /* command rate  */
                                     NULL,                     /* Data pointer  */
                                     0,                        /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,     /* data width    */
                                     CY_SMIF_SDR,              /* data rate     */
                                     slave,                    /* slave select  */
                                     NOT_LAST_COMMAND,         /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, (uint8_t*)readPW, 8u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PasswordProgram
****************************************************************************//**
*
* This function program input value to the Password Register.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param programPW
* Value to be written into the password register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordProgram(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint64_t  programPW,
                                                  cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PASSP, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     (uint8_t*)&programPW,    /* Data pointer  */
                                     8u,                      /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PasswordUnlock
****************************************************************************//**
*
* If the input password does match the Password Register value,
* the PPB Lock bit is set to 1
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param inputPW
* input password value
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordUnlock(volatile cy_stc_smif_reg_t* base,
                                                 cy_en_smif_slave_select_t slave,
                                                 uint64_t  inputPW,
                                                 cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PASSU, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     (uint8_t*)&inputPW,      /* Data pointer  */
                                     8u,                      /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_SoftwareReset
****************************************************************************//**
*
* This function restores the device to its initial power up state,
* except for the volatile FREEZE bit in * the Configuration register CR1[1]
* and the volatile PPB Lock bit in the PPB Lock Register.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_SoftwareReset(volatile cy_stc_smif_reg_t* base,
                                                cy_en_smif_slave_select_t slave,
                                                cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RST, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate  */
                                     NULL,                  /* Data pointer  */
                                     0u,                    /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width    */
                                     CY_SMIF_SDR,           /* data rate     */
                                     slave,                 /* slave select  */
                                     LAST_COMMAND_BYTE,     /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ModeBitReset
****************************************************************************//**
*
* This function can be used to return the device from continuous
* high performance read mode back to normal standby awaiting any new command.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ModeBitReset(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_MBR, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate  */
                                     NULL,                  /* Data pointer  */
                                     0u,                    /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width    */
                                     CY_SMIF_SDR,           /* data rate     */
                                     slave,                 /* slave select  */
                                     LAST_COMMAND_BYTE,     /* is last command */
                                     context);
    return status;
}


#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)


/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadId
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
* \param readManufactureID
* Pointer to memory in which read manufacture identification to be stored.
*
* \param readDeviceID
* Pointer to memory in which read device identification to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadId(volatile cy_stc_smif_reg_t* base,
                                         cy_en_smif_slave_select_t slave,
                                         uint8_t* readManufactureID,
                                         uint8_t* readDeviceID,
                                         cy_stc_smif_context_t* context)
{
    uint8_t dummyAddress[3] = {0x00u};
    uint8_t recievedData[2] = {0x00u};

    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_READ_ID, /* command value */
                                     false,                     /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,      /* command width */
                                     CY_SMIF_SDR,               /* command rate */
                                     dummyAddress,              /* Data pointer */
                                     3,                         /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,      /* data width */
                                     CY_SMIF_SDR,               /* data rate */
                                     slave,                     /* slave select */
                                     NOT_LAST_COMMAND,          /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, recievedData, 2u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    *readManufactureID = recievedData[0];
    *readDeviceID      = recievedData[1];

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadElectronicSignature
****************************************************************************//**
*
* This function is used to read a single byte Electronic Signature
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readData
* Pointer to memory in which read Electronic Signature to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadElectronicSignature(volatile cy_stc_smif_reg_t* base,
                                                          cy_en_smif_slave_select_t slave,
                                                          uint8_t* readData,
                                                          cy_stc_smif_context_t* context)
{
    uint8_t dummyAddress[3] = {0x00u};
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RES, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate */
                                     dummyAddress,          /* Data pointer */
                                     3,                     /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width */
                                     CY_SMIF_SDR,           /* data rate */
                                     slave,                 /* slave select */
                                     NOT_LAST_COMMAND,      /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadBankRegister
****************************************************************************//**
*
* This function allows the Bank address Register contents to be read.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadBankRegister(volatile cy_stc_smif_reg_t* base,
                                                   cy_en_smif_slave_select_t slave,
                                                   uint8_t* readData,
                                                   cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_BRRD, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     NOT_LAST_COMMAND,       /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteBankRegister
****************************************************************************//**
*
* This function allows the Bank address Register contents to be written.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* Value to be written into the register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteBankRegister(volatile cy_stc_smif_reg_t* base,
                                                    cy_en_smif_slave_select_t slave,
                                                    uint8_t writeData,
                                                    cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_BRWR, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     &writeData,             /* Data pointer */
                                     1,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_AccessBankRegister
****************************************************************************//**
*
* This function provides full access to the Bank Address Register (BAR) but 
* they are both commands that are not present in legacy SPI memory devices.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param BarBit0
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_AccessBankRegister(volatile cy_stc_smif_reg_t* base,
                                                     cy_en_smif_slave_select_t slave,
                                                     uint8_t BarBit0,
                                                     cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint8_t writeData[2];

    /* When the BRAC command is sent, the FL-S family device will then interpret an
       immediately following Write Register (WRR) command as a write to the lower address
       bits of the BAR. */
    writeData[0] = CY_SMIF_S25FL_CMD_WRR; 
    writeData[1] = BarBit0;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_BRAC, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     writeData,              /* Data pointer */
                                     2,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteStatusRegister
****************************************************************************//**
*
* This function provides a way to write Status Register
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* Value to be written into the status register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteStatusRegister(volatile cy_stc_smif_reg_t* base,
                                                      cy_en_smif_slave_select_t slave,
                                                      uint8_t writeData,
                                                      cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_WRR, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate */
                                     &writeData,            /* Data pointer */
                                     1,                     /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width */
                                     CY_SMIF_SDR,           /* data rate */
                                     slave,                 /* slave select */
                                     LAST_COMMAND_BYTE,     /* is last command */
                                     context);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadAutoBootRegister
****************************************************************************//**
*
* This function reads AutoBoot Register.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadAutoBootRegister(volatile cy_stc_smif_reg_t* base,
                                                       cy_en_smif_slave_select_t slave,
                                                       uint32_t* readData,
                                                       cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ABRD, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     NOT_LAST_COMMAND,       /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, (uint8_t*)readData, 4u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_WriteAutoBootRegister
****************************************************************************//**
*
* This function write input value into AutoBoot Register.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeData
* Value to be written into the register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_WriteAutoBootRegister(volatile cy_stc_smif_reg_t* base,
                                                        cy_en_smif_slave_select_t slave,
                                                        uint32_t writeData,
                                                        cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ABWR, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     (uint8_t*)&writeData,   /* Data pointer */
                                     4,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ProgramSuspend
****************************************************************************//**
*
* This function allows the system to interrupt a programming operation
* and then read from any other non-erase-suspended sector or
* non-program-suspended-page. Program Suspend is valid only during a
* programming operation.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramSuspend(volatile cy_stc_smif_reg_t* base,
                                                 cy_en_smif_slave_select_t slave,
                                                 cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PGSP, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ProgramResume
****************************************************************************//**
*
* This function must be called to resume the programming operation
* after a Program Suspend. If the programming operation was completed during
* the suspend operation, a resume command is not needed and has no effect if
* issued. Program Resume commands will be ignored unless a Program operation is
* suspended.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramResume(volatile cy_stc_smif_reg_t* base,
                                                cy_en_smif_slave_select_t slave,
                                                cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PGRS, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_4KB_SectorErase
****************************************************************************//**
*
* This function sets all bits in the addressed sector to 1 (all bytes are FFh)
* Make sure the device is configured as 4-kB sectors
* Write Enable is required before calling this function.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_4KB_SectorErase(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint32_t erasedAddress,
                                                  cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t    status;
    en_cy_smif_s25fl_cmd_t eraseCmd;
    uint8_t                addrSizeInByte;
    uint32_t               addrInBigEndian;

    if(erasedAddress > 0x00FFFFFFul)
    {
        eraseCmd        = CY_SMIF_S25FL_CMD_4P4E;
        addrSizeInByte  = 4ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(erasedAddress);
    }
    else
    {
        // assuming that ExtAdd == 0;
        eraseCmd        = CY_SMIF_S25FL_CMD_P4E;
        addrSizeInByte  = 3ul;
        addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(erasedAddress << 8ul);
    }

    status = Cy_SMIF_TransmitCommand(base,
                                     (uint8_t)eraseCmd,          /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer */
                                     addrSizeInByte,             /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width */
                                     CY_SMIF_SDR,                /* data rate */
                                     slave,                      /* slave select */
                                     LAST_COMMAND_BYTE,          /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_EraseSuspend
****************************************************************************//**
*
* This function allows the system to interrupt a sector erase operation and then
* read from or program data to, any other sector. Erase Suspend is valid only
* during a sector erase operation. The Erase Suspend command is ignored if
* written during the Bulk Erase operation.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_EraseSuspend(volatile cy_stc_smif_reg_t* base,
                                               cy_en_smif_slave_select_t slave,
                                               cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ERSP, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_EraseResume
****************************************************************************//**
*
* This function must be called to resume the erase operation if an Erase is suspend.
* Erase Resume commands will be ignored unless an Erase is Suspend.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_EraseResume(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ERRS, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate */
                                     NULL,                   /* Data pointer */
                                     0,                      /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width */
                                     CY_SMIF_SDR,            /* data rate */
                                     slave,                  /* slave select */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_OPT_Program
****************************************************************************//**
*
* This function programs data in the One Time Program region,
* which is in a different address space from the main array data.
* The OTP region is 1024 bytes so, the address bits from A23 to A10 must be
* zero for this command.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param programedAddress
* Address to One Time Program region to be programed.
* Bit 31 to bit 10 of this input is asserted to be "0".
*
* \param size
* Number of byte to be written into OTP region.
*
* \param buf
* Pointer to memory containing data to be written into the OTP region. 
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_OPT_Program(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint32_t programedAddress,
                                              uint32_t size,
                                              uint8_t buf[],
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint32_t            addrInBigEndian;

    if(programedAddress > 0x000003FFul)
    {
        CY_ASSERT_L1(false);
    }
    addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(programedAddress << 8ul);

    /****** Command and Address ******/
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_OTPP,     /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer */
                                     3,                          /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width */
                                     CY_SMIF_SDR,                /* data rate */
                                     slave,                      /* slave select */
                                     NOT_LAST_COMMAND,           /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_TransmitDataBlocking(base, buf, size, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_OPT_Read
****************************************************************************//**
*
* The OTP Read command reads data from the OTP region.
* The OTP region is 1024 bytes so, the address bits from A23 to A10 must be zero
* for this function.
* 
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readAddress
* Address to One Time Program region to be read.
* Bit 31 to bit 10 of this input is asserted to be "0".
*
* \param size
* Number of byte to be read into OTP region.
*
* \param buf
* Pointer to memory in which read OTP region data to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_OPT_Read(volatile cy_stc_smif_reg_t* base,
                                           cy_en_smif_slave_select_t slave,
                                           uint32_t readAddress,
                                           uint32_t size,
                                           uint8_t buf[],
                                           cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint32_t            addrInBigEndian;

    if(readAddress > 0x000003FFul)
    {
        CY_ASSERT_L1(false);
    }
    addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(readAddress << 8ul);

    /****** Command and Address ******/
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_OTPR,     /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate  */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer  */
                                     3,                          /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width    */
                                     CY_SMIF_SDR,                /* data rate     */
                                     slave,                      /* slave select  */
                                     NOT_LAST_COMMAND,           /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, buf, size, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ASP_Read
****************************************************************************//**
*
* This function reads the 16-bit ASP register contents
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readValue
* Pointer to memory in which read register value to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ASP_Read(volatile cy_stc_smif_reg_t* base,
                                           cy_en_smif_slave_select_t slave,
                                           uint16_t* readValue,
                                           cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ASPRD, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     NULL,                    /* Data pointer  */
                                     0,                       /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, (uint8_t*)readValue, 2u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_ASP_Program
****************************************************************************//**
*
* This function programs the 16-bit ASP register contents.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param programedValue
* Value to be written into the register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ASP_Program(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint16_t* programedValue,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_ASPP,   /* command value */
                                     false,                    /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,     /* command width */
                                     CY_SMIF_SDR,              /* command rate  */
                                     (uint8_t*)programedValue, /* Data pointer  */
                                     2,                        /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,     /* data width    */
                                     CY_SMIF_SDR,              /* data rate     */
                                     slave,                    /* slave select  */
                                     LAST_COMMAND_BYTE,        /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_DYB_Read
****************************************************************************//**
*
* This function reads the 8-bit DYB access register pointed by input address.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readAddress
* The 32-bit address selecting location zero within the desired sector
*
* \param readValue
* Pointer to memory in which read value to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_DYB_Read(volatile cy_stc_smif_reg_t* base,
                                           cy_en_smif_slave_select_t slave,
                                           uint32_t readAddress,
                                           uint8_t* readValue,
                                           cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint32_t            addrInBigEndian;

    addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(readAddress);
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_DYBRD,    /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate  */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer  */
                                     4,                          /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width    */
                                     CY_SMIF_SDR,                /* data rate     */
                                     slave,                      /* slave select  */
                                     NOT_LAST_COMMAND,           /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, readValue, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_DYB_Write
****************************************************************************//**
*
* This function programs the 8-bit DYB access register pointed by input address.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param writeAddress
* The 32-bit address selecting location zero within the desired sector
*
* \param writeValue
* Value to be written into the register.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_DYB_Write(volatile cy_stc_smif_reg_t* base,
                                            cy_en_smif_slave_select_t slave,
                                            uint32_t writeAddress,
                                            uint8_t  writeValue,
                                            cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint8_t             writtenData[5];
    uint32_t            addrInBigEndian;

    addrInBigEndian         = Cy_SMIF_Reverse4ByteEndian(writeAddress);
    *(uint32_t*)writtenData = addrInBigEndian;
    writtenData[4]          = writeValue;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_DYBWR, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     writtenData,             /* Data pointer  */
                                     5,                       /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PPB_Read
****************************************************************************//**
*
* This function reads the 8-bit PPB access register pointed by input address.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readAddress
* The 32-bit address selecting location zero within the desired sector
*
* \param readValue
* Pointer to memory in which read value to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Read(volatile cy_stc_smif_reg_t* base,
                                           cy_en_smif_slave_select_t slave,
                                           uint32_t readAddress,
                                           uint8_t* readValue,
                                           cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint32_t addrInBigEndian;

    addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(readAddress);
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PPBRD,    /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate  */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer  */
                                     4,                          /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width    */
                                     CY_SMIF_SDR,                /* data rate     */
                                     slave,                      /* slave select  */
                                     NOT_LAST_COMMAND,           /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, readValue, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PPB_Program
****************************************************************************//**
*
* This function programs the 8-bit PPB access register pointed by input address.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param programedAddress
* The 32-bit address selecting location zero within the desired sector
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Program(volatile cy_stc_smif_reg_t* base,
                                              cy_en_smif_slave_select_t slave,
                                              uint32_t programedAddress,
                                              cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    uint32_t            addrInBigEndian;

    addrInBigEndian = Cy_SMIF_Reverse4ByteEndian(programedAddress);

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PPBP,     /* command value */
                                     false,                      /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,       /* command width */
                                     CY_SMIF_SDR,                /* command rate  */
                                     (uint8_t*)&addrInBigEndian, /* Data pointer  */
                                     4,                          /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,       /* data width    */
                                     CY_SMIF_SDR,                /* data rate     */
                                     slave,                      /* slave select  */
                                     LAST_COMMAND_BYTE,          /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PPB_Erase
****************************************************************************//**
*
* This function sets all PPB bits to 1.
* Write Enable is required before calling this function.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Erase(volatile cy_stc_smif_reg_t* base,
                                            cy_en_smif_slave_select_t slave,
                                            cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;

    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PPBE, /* command value */
                                     false,                  /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,   /* command width */
                                     CY_SMIF_SDR,            /* command rate  */
                                     NULL,                   /* Data pointer  */
                                     0,                      /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,   /* data width    */
                                     CY_SMIF_SDR,            /* data rate     */
                                     slave,                  /* slave select  */
                                     LAST_COMMAND_BYTE,      /* is last command */
                                     context);
    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_PPB_LockBitRead
****************************************************************************//**
*
* This function allows the PPB Lock Register contents
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param readValue
* Pointer to memory in which read register value to be stored.
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_LockBitRead(volatile cy_stc_smif_reg_t* base,
                                                  cy_en_smif_slave_select_t slave,
                                                  uint8_t* readValue,
                                                  cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PLBRD, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     NULL,                    /* Data pointer  */
                                     0,                       /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    Cy_SMIF_ReceiveDataBlocking(base, readValue, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);
    return status;
}


/*******************************************************************************
* Function Cy_SMIF_S25FL_PPB_LockBitWrite
****************************************************************************//**
*
* This function clears the PPB Lock Register to zero.
*
* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_LockBitWrite(volatile cy_stc_smif_reg_t* base,
                                                   cy_en_smif_slave_select_t slave,
                                                   cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_PLBWR, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate  */
                                     NULL,                    /* Data pointer  */
                                     0,                       /* Data size     */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width    */
                                     CY_SMIF_SDR,             /* data rate     */
                                     slave,                   /* slave select  */
                                     LAST_COMMAND_BYTE,       /* is last command */
                                     context);
    return status;
}

#elif (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadConfigurationRegister2
****************************************************************************//**
*
* This function reads the Configuration Register 2.
* Read data does not support multiple slave access at the same time.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister2(volatile cy_stc_smif_reg_t* base,
                                                             cy_en_smif_slave_select_t slave,
                                                             uint8_t* readData,
                                                             cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDCR2, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ReadConfigurationRegister3
****************************************************************************//**
*
* This function reads the Configuration Register 3.
* Read data does not support multiple slave access at the same time.
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
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister3(volatile cy_stc_smif_reg_t* base,
                                                             cy_en_smif_slave_select_t slave,
                                                             uint8_t* readData,
                                                             cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_RDCR3, /* command value */
                                     false,                   /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,    /* command width */
                                     CY_SMIF_SDR,             /* command rate */
                                     NULL,                    /* Data pointer */
                                     0,                       /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,    /* data width */
                                     CY_SMIF_SDR,             /* data rate */
                                     slave,                   /* slave select */
                                     NOT_LAST_COMMAND,        /* is last command */
                                     context);
    if(status != CY_SMIF_SUCCESS)
    {
        return status;
    }

    status = Cy_SMIF_ReceiveDataBlocking(base, readData, 1u, CY_SMIF_WIDTH_SINGLE, CY_SMIF_SDR, slave, context);

    return status;
}

/*******************************************************************************
* Function Cy_SMIF_S25FL_ProgramOrEraseSuspend
****************************************************************************//**
*
* This function allows the system to interrupt a programming or erase operation
* and then read from any other non-erase-suspended sector or
* non-program-suspended-page.
* Program or Erase Suspend is valid only during a programming or sector erase,
* half block erase or block erase operation.
* A Chip Erase operation cannot be suspended.

* \param base
* Holds the base address of the SMIF block registers.
*
* \param slave
* Specifies the number of the slave device to which the transfer is made.
* \ref cy_en_smif_slave_select_t
*
* \param context
* Passes a configuration structure that contains the transfer parameters of the
* SMIF block.
*
* \return \ref cy_en_smif_status_t
*
*******************************************************************************/
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramOrEraseSuspend(volatile cy_stc_smif_reg_t* base,
                                                        cy_en_smif_slave_select_t slave,
                                                        cy_stc_smif_context_t* context)
{
    cy_en_smif_status_t status;
    status = Cy_SMIF_TransmitCommand(base,
                                     CY_SMIF_S25FL_CMD_EPS, /* command value */
                                     false,                 /* is command 2 byte */
                                     CY_SMIF_WIDTH_SINGLE,  /* command width */
                                     CY_SMIF_SDR,           /* command rate */
                                     NULL,                  /* Data pointer */
                                     0,                     /* Data size */
                                     CY_SMIF_WIDTH_SINGLE,  /* data width */
                                     CY_SMIF_SDR,           /* data rate */
                                     slave,                 /* slave select */
                                     LAST_COMMAND_BYTE,     /* is last command */
                                     context);

    return status;
}


#endif

#if defined(__cplusplus)
}
#endif
