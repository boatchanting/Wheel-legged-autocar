/***************************************************************************//**
* \file cy_smif_semp.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress SEMPER HSSPI series driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_SMIF_SEMP_H)
#define CY_SMIF_SEMP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "cy_project.h"
#include "cy_smif_device_common.h"

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/** Specifies Read/Write Command of Semper Flash */
typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    //                                constant|  latency|   RWDS|  addr length| address| 
    //                           latency cycle| table No| enable| configurable|  length| Command Value
    /* Register Access */
    CY_SEMP_CMD_1BYTE_WRENB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0006, /**< Write Enable                                    (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_WRENV_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0050, /**< Write Enable Volatile Register                  (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRDIS_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0004, /**< Write Disable                                   (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_CLPEF_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0082, /**< Clear Program and Erase Failure Flags           (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_EN4BA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B7, /**< Enter 4 byte Address Mode                       (S25H            ) */
    CY_SEMP_CMD_1BYTE_EX4BA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B8, /**< Exit 4 byte Address Mode                        (S25H            ) */

    /* ECC */
    CY_SEMP_CMD_1BYTE_CLECC_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x001B, /**< Clear ECC Status Register                       (S25H, S26H, S28H) */

    /* Erase Flash Array */
    CY_SEMP_CMD_1BYTE_ERCHP_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0060, /**< Erase Chip (or 0x00C7)                          (S25H, S26H, S28H) */

    /* Suspend / Resume */
    CY_SEMP_CMD_1BYTE_SPEPD_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B0, /**< Suspend Erase / Program / Data Integrity Check  (       S26H, S28H) */
    CY_SEMP_CMD_1BYTE_SPEPD_0_0_S25H = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0075, /**< Suspend Erase / Program / Data Integrity Check  (S25H             ) */
    CY_SEMP_CMD_1BYTE_SPEPA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0085, /**< Suspend Erase / Program (or 0x00B0)             (S25H             ) */
    CY_SEMP_CMD_1BYTE_RSEPD_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0030, /**< Resume Erase / Program / Data Integrity Check   (       S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RSEPD_0_0_S25H = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x007A, /**< Resume Erase / Program / Data Integrity Check   (S25H             ) */
    CY_SEMP_CMD_1BYTE_RSEPA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x008A, /**< Resume Erase / Program                          (S25H             ) */

    /* Advanced Sector Protection */
    CY_SEMP_CMD_1BYTE_ERPPB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E4, /**< Erase Persistent Protection Bit                 (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_WRPLB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00A6, /**< Write PPB Protection Lock Bit                   (S25H, S26H, S28H) */

    /* Reset */
    CY_SEMP_CMD_1BYTE_SRSTE_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0066, /**< Software Reset Enable                           (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_SFRST_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0099, /**< Software Reset                                  (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_SFRSL_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00F0, /**< Legacy Software Reset                           (S25H            ) */
    /* Deep Power Down */
    CY_SEMP_CMD_1BYTE_ENDPD_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B9, /**< Enter Deep Power Down Mode                      (S25H, S26H, S28H) */
    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Register Access */
    
    CY_SEMP_CMD_2BYTE_WRENB_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x0606, /**< Write Enable                                    (            S28H) */
    CY_SEMP_CMD_2BYTE_WRDIS_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x0404, /**< Write Disable                                   (            S28H) */
    CY_SEMP_CMD_2BYTE_CLPEF_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x8282, /**< Clear Program and Erase Failure Flags           (            S28H) */
    /* ECC */
    CY_SEMP_CMD_2BYTE_CLECC_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x1B1B, /**< Clear ECC Status Register                       (            S28H) */
    /* Erase Flash Array */
    CY_SEMP_CMD_2BYTE_ERCHP_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x6060, /**< Erase Chip (or 0xC7C7)                          (            S28H) */
    /* Suspend / Resume */
    CY_SEMP_CMD_2BYTE_SPEPD_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0xB0B0, /**< Suspend Erase / Program / Data Integrity Check  (            S28H) */
    CY_SEMP_CMD_2BYTE_RSEPD_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x3030, /**< Resume Erase / Program / Data Integrity Check   (            S28H) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_2BYTE_ERPPB_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0xE4E4, /**< Erase Persistent Protection Bit                 (            S28H) */
    CY_SEMP_CMD_2BYTE_WRPLB_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x2C2C, /**< Write PPB Protection Lock Bit                   (            S28H) */
    /* Reset */
    CY_SEMP_CMD_2BYTE_SRSTE_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x6666, /**< Software Reset Enable                           (            S28H) */
    CY_SEMP_CMD_2BYTE_SFRST_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0x9999, /**< Software Reset                                  (            S28H) */
    /* Deep Power Down */
    CY_SEMP_CMD_2BYTE_ENDPD_0_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (0<<16)| 0xB9B9, /**< Enter Deep Power Down Mode                      (            S28H) */

    CY_SEMP_CMD_INVALID              = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_semp_cmd_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    //                                constant|  latency|   RWDS|  addr length| address| 
    //                           latency cycle| table No| enable| configurable|  length| Command Value
    /* Erase Flash Array */
    CY_SEMP_CMD_1BYTE_ER004_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0020, /**< Erase 4-KB Sector                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_ER004_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0021, /**< Erase 4-KB Sector                               (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_ER256_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00D8, /**< Erase 256-KB Sector                             (S25H            ) */
    CY_SEMP_CMD_1BYTE_ER256_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00DC, /**< Erase 256-KB Sector                             (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_EVERS_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00D0, /**< Evaluate Erase Status                           (    , S26H, S28H) */
    CY_SEMP_CMD_1BYTE_EVERS_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00D0, /**< Evaluate Erase Status                           (S25H            ) */
    CY_SEMP_CMD_1BYTE_SEERC_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x005D, /**< Sector Erase Count                              (    , S26H, S28H) */
    CY_SEMP_CMD_1BYTE_SEERC_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x005D, /**< Sector Erase Count                              (S25H            ) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_1BYTE_PRPPB_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FD, /**< Program Persistent Protection Bit               (S25H            ) */
    CY_SEMP_CMD_1BYTE_PRPPB_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E3, /**< Program Persistent Protection Bit               (S25H, S26H, S28H) */

    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Erase Flash Array */
    CY_SEMP_CMD_2BYTE_ER004_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x2121, /**< Erase 4-KB Sector                               (            S28H) */
    CY_SEMP_CMD_2BYTE_ER256_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xDCDC, /**< Erase 256-KB Sector                             (            S28H) */
    CY_SEMP_CMD_2BYTE_EVERS_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xD0D0, /**< Evaluate Erase Status                           (            S28H) */
    CY_SEMP_CMD_2BYTE_SEERC_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x5D5D, /**< Sector Erase Count                              (            S28H) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_2BYTE_PRPPB_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xE3E3, /**< Program Persistent Protection Bit               (            S28H) */

    CY_SEMP_CMD_ADDR_INVALID         = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_semp_cmd_addr_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    //                                constant|  latency|   RWDS|  addr length| address| 
    //                           latency cycle| table No| enable| configurable|  length| Command Value
    /* Read Device ID */
    CY_SEMP_CMD_1BYTE_RDIDN_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x009F, /**< Read manufacturer and device identification     (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDQID_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00AF, /**< Read manufacturer and device identification     (S25H            ) */
    CY_SEMP_CMD_1BYTE_RSFDP_3_0      = (0<<24)|  (4<<21)|(0<<20)|      (0<<19)| (3<<16)| 0x005A, /**< Read JEDEC Serial Flash Discoverable Parameters (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDUID_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x004C, /**< Read Unique ID                                  (S25H, S26H, S28H) */

    /* Register Access */
    CY_SEMP_CMD_1BYTE_RDSR1_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0005, /**< Read Status Register 1                          (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDSR2_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0007, /**< Read Status Register-2                          (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDSR2_0_0_QUAD = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0007, /**< Read Status Register-2                          (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDCR1_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0035, /**< Read Configuration Register 1                   (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDCR1_0_0_QUAD = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0035, /**< Read Configuration Register 1                   (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDARG_C_0_NV   = (0<<24)|  (4<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0065, /**< Read Any Register (Non-Volatile)                (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDARG_C_0_V    = (0<<24)|  (1<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0065, /**< Read Any Register (Volatile)                    (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDDLP_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0041, /**< Read Data Learning Pattern Register             (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDDLP_0_0_QUAD = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0041, /**< Read Data Learning Pattern Register             (S25H            ) */
    /* ECC */
    CY_SEMP_CMD_1BYTE_RDECC_4_0      = (0<<24)|  (4<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0019, /**< Read ECC Status                                 (      S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDECC_4_0_S25H = (0<<24)|  (4<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0018, /**< Read ECC Status                                 (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDECC_C_0      = (0<<24)|  (4<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0019, /**< Read ECC Status                                 (S25H            ) */

    /* Secure Silicon Region */
    CY_SEMP_CMD_1BYTE_RDSSR_4_0      = (0<<24)|  (4<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x004B, /**< Read Secure Silicon Region                      (      S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDSSR_C_0      = (0<<24)|  (4<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x004B, /**< Read Secure Silicon Region                      (S25H            ) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_1BYTE_RDDYB_C_0      = (0<<24)|  (1<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FA, /**< Read Dynamic Protection Bit                     (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDDYB_4_0      = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E0, /**< Read Dynamic Protection Bit                     (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDPPB_C_0      = (0<<24)|  (4<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FC, /**< Read Persistent Protection Bit                  (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDPPB_4_0      = (0<<24)|  (4<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E2, /**< Read Persistent Protection Bit                  (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDPLB_0_0      = (0<<24)|  (2<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00A7, /**< Read Password Protection Mode Lock Bit          (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDPLB_0_0_QUAD = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00A7, /**< Read Password Protection Mode Lock Bit          (S25H, S26H, S28H) */

    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Read Device ID */
    CY_SEMP_CMD_2BYTE_RDIDN_4_0      = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x9F9F, /**< Read manufacturer and device identification     (            S28H) */
    CY_SEMP_CMD_2BYTE_RSFDP_4_0      = (8<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x5A5A, /**< Read JEDEC Serial Flash Discoverable Parameters (            S28H) */
    CY_SEMP_CMD_2BYTE_RDUID_4_0      = (32<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x4C4C, /**< Read Unique ID                                  (            S28H) */
    /* Register Access */
    CY_SEMP_CMD_2BYTE_RDSR1_4_0      = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x0505, /**< Read Status Register 1                          (            S28H) */
    CY_SEMP_CMD_2BYTE_RDSR2_4_0      = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x0707, /**< Read Status Register 2                          (            S28H) */
    CY_SEMP_CMD_2BYTE_RDARG_4_0_NV   = (0<<24)|  (5<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x6565, /**< Read Any Register (Non-Volatile)                (            S28H) */
    CY_SEMP_CMD_2BYTE_RDARG_4_0_V    = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x6565, /**< Read Any Register (Volatile)                    (            S28H) */
    /* ECC */
    CY_SEMP_CMD_2BYTE_RDECC_4_0      = (0<<24)|  (5<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x1919, /**< Read ECC Status                                 (            S28H) */
    /* CRC */
    CY_SEMP_CMD_2BYTE_RDCRC_4_0      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x6464, /**< Read Interface CRC Register                     (            S28H) */
    /* Secure Silicon Region */
    CY_SEMP_CMD_2BYTE_RDSSR_4_0      = (0<<24)|  (5<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x4B4B, /**< Read Secure Silicon Region                      (            S28H) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_2BYTE_RDDYB_4_0      = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xE0E0, /**< Read Dynamic Protection Bit                     (            S28H) */
    CY_SEMP_CMD_2BYTE_RDPPB_4_0      = (0<<24)|  (5<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xE2E2, /**< Read Persistent Protection Bit                  (            S28H) */
    CY_SEMP_CMD_2BYTE_RDPLB_4_0      = (0<<24)|  (3<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x2D2D, /**< Read  Password Protection Mode Lock Bit         (            S28H) */

    CY_SEMP_RD_CMD_INVALID           = CY_SPI_INVALID_CMD, /**< Invalid Command */

} cy_en_smif_semp_rd_cmd_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    //                                constant|  latency|   RWDS|  addr length| address| 
    //                           latency cycle| table No| enable| configurable|  length| Command Value
    /* Register Access */
    CY_SEMP_CMD_1BYTE_WRREG_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0001, /**< Write Stat Reg 1 and Cfg Reg 1 - 4              (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRRSB_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0001, /**< SafeBoot Write Register.                        (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRARG_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0071, /**< Write Any Register                              (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_WRDLP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x004A, /**< Write Data Learning Pattern Volatile Register   (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRAUB_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0015, /**< AutoBoot Register Write                         (S25H            ) */
    CY_SEMP_CMD_1BYTE_PRDLP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0043, /**< Program Data Learning Pattern NV Register       (S25H            ) */
    /* CRC */
    CY_SEMP_CMD_1BYTE_DICHK_4_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x005B, /**< Data Integrity Check                            (S25H, S26H, S28H) */
    /* Secure Silicon Region */
    CY_SEMP_CMD_1BYTE_PRSSR_4_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0042, /**< Program Secure Silicon Region                   (      S26H, S28H) */
    CY_SEMP_CMD_1BYTE_PRSSR_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0042, /**< Program Secure Silicon Region                   (S25H,           ) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_1BYTE_PRASP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x002F, /**< ASP Program                                     (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRDYB_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FB, /**< Write Dynamic Protection Bit                    (S25H            ) */
    CY_SEMP_CMD_1BYTE_WRDYB_4_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E1, /**< Write Dynamic Protection Bit                    (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_PWDUL_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E9, /**< Password Unlock                                 (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_PGPWD_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E8, /**< Program Password                                (S25H            ) */

    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Register Access */
    CY_SEMP_CMD_2BYTE_WRARG_4_1      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x7171, /**< Write Any Register                              (            S28H) */
    /* CRC */
    CY_SEMP_CMD_2BYTE_DICHK_4_1      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x5B5B, /**< Data Integrity Check                            (            S28H) */
    /* Secure Silicon Region */
    CY_SEMP_CMD_2BYTE_PRSSR_4_1      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0x4242, /**< Program Secure Silicon Region                   (            S28H) */
    /* Advanced Sector Protection */
    CY_SEMP_CMD_2BYTE_WRDYB_4_1      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xE1E1, /**< Write Dynamic Protection Bit                    (            S28H) */
    CY_SEMP_CMD_2BYTE_PWDUL_4_1      = (0<<24)|  (0<<21)|(1<<20)|      (0<<19)| (4<<16)| 0xE9E9, /**< Password Unlock                                 (            S28H) */

    CY_SEMP_WT_CMD_INVALID           = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_semp_wt_cmd_t;

typedef enum
{
    CY_SEMP_CMD_NO_COMMAND           = 0x00000000, /**< In continuous mode, command is not required     (S25H            ) */

    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    /* Read Flash Array */
    CY_SEMP_CMD_1BYTE_RDAY1_C_0      = 0x00000003, /**< Read                                            (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDAY1_4_0      = 0x00000013, /**< Read                                            (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDAY2_C_0      = 0x0000000B, /**< Read Fast                                       (S25H, S26H, S28H) */
    CY_SEMP_CMD_1BYTE_RDAY2_4_0      = 0x0000000C, /**< Read Fast                                       (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY3_C_0      = 0x000000BB, /**< Read SDR Dual I/O                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY3_4_0      = 0x000000BC, /**< Read SDR Dual I/O                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY4_C_0      = 0x0000006B, /**< Read SDR Quad Output                            (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY4_4_0      = 0x0000006C, /**< Read SDR Quad Output                            (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY5_C_0      = 0x000000EB, /**< Read SDR Quad I/O                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY5_4_0      = 0x000000EC, /**< Read SDR Quad I/O                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY7_C_0      = 0x000000ED, /**< Read DDR Quad I/O                               (S25H            ) */
    CY_SEMP_CMD_1BYTE_RDAY7_4_0      = 0x000000EE, /**< Read DDR Quad I/O                               (S25H            ) */

    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Read Flash Array */
    CY_SEMP_CMD_2BYTE_RDAY1_4_0      = 0x0000ECEC, /**< Read Octal SDR                                  (            S28H) */
    CY_SEMP_CMD_2BYTE_RDAY2_4_0      = 0x0000EEEE, /**< Read Octal DDR                                  (            S28H) */
} cy_en_smif_semp_mem_rd_cmd_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    /* Program Flash Array */
    CY_SEMP_CMD_1BYTE_PRPGE_C_1      = 0x00000002, /**< Program Page                                    (S25H            ) */
    CY_SEMP_CMD_1BYTE_PRPGE_4_1      = 0x00000012, /**< Program Page                                    (S25H, S26H, S28H) */

    /******************************/
    /*      2 byte commands       */
    /******************************/
    /* Program Flash Array */
    CY_SEMP_CMD_2BYTE_PRPGE_4_1      = 0x00001212, /**< Program Page                                    (            S28H) */
} cy_en_smif_semp_mem_wt_cmd_t;
/** \} group_mw_smif_enums */

/************************************/
/** Semper Flash Register Utilities */
/************************************/

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/***** Status Register 1 Utilities *****/
/** Status Register 1 bit field definition */
typedef struct
{
    uint8_t u1RDYBSY : 1; /** Device Ready/Busy Status Flag                             (S25H, S26H, S28H) */
    uint8_t u1WRPGEN : 1; /** Write/Program Enable Status Flag                          (S25H, S26H, S28H) */
    uint8_t u3LBPROT : 3; /** Legacy Block Protection based Memory Array size selection (S25H,       S28H) */
    uint8_t u1ERSERR : 1; /** Erasing Error Status Flag                                 (S25H, S26H, S28H) */
    uint8_t u1PRGERR : 1; /** Programming Error Status Flag                             (S25H, S26H, S28H) */
    uint8_t u1STCFWR : 1; /** Status Register 1 and Configuration Register 1,2,3,4
                              Protection Selection against write (erase/program)        (S25H            ) */
} stc_cy_smif_semp_STR1_t;

/** Status Register 1 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_STR1_t field; /** < \ref stc_cy_smif_semp_STR1_t */
} un_cy_smif_semp_STR1_t;


/***** Status Register 2 Utilities *****/
/** Status Register 2 bit field definition */
typedef struct
{
    uint8_t u1PROGMS : 1; /** Program operation Suspend Status Flag                                   (S25H, S26H, S28H) */
    uint8_t u1ERASES : 1; /** Erase operation Suspend Status Flag                                     (S25H, S26H, S28H) */
    uint8_t u1SESTAT : 1; /** Sector Erase Success/Failure Status Flag                                (S25H, S26H, S28H) */
    uint8_t u1DICRCA : 1; /** Memory Array Data Integrity Cyclic Redundancy Check Abort Status Flag   (S25H, S26H, S28H) */
    uint8_t u1DICRCS : 1; /** Memory Array Data Integrity Cyclic Redundancy Check Suspend Status Flag (S25H, S26H, S28H) */
    uint8_t u3RESRVD : 3; /** Reserved for future use */
} stc_cy_smif_semp_STR2_t;

/** Status Register 2 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_STR2_t field; /** < \ref stc_cy_smif_semp_STR2_t */
} un_cy_smif_semp_STR2_t;

/***** Configuration Register 1 Utilities *****/
/** Configuration Register 1 bit field definition */
typedef struct
{
    uint8_t u1TLPROT  : 1; /** Temporary Locking selection of Legacy Block Protection and Sector Architecture      (S25H, S26H, S28H) */
    uint8_t u1QUADIT  : 1; /** Quad SPI Interface Selection - I/O width set to 4 bits (1-1-4, 1-4-4)               (S25H            ) */
    uint8_t u1TB4KBS  : 1; /** Top or Bottom Address Range selection for 4 KB Sector Block                         (S25H, S26H, S28H) */
    uint8_t u1RESRVD0 : 1; /** Reserved for future use */
    uint8_t u1PLPROT  : 1; /** Permanent Locking selection of Legacy Block Protection and 4 KB Sector Architecture (S25H, S26H, S28H) */
    uint8_t u1TBPROT  : 1; /** Top or Bottom Protection selection for Legacy Protection Mode                       (S25H, S26H, S28H) */
    uint8_t u1SP4KBS  : 1; /** Split 4 KB Sectors selection between top and bottom address space                   (S25H, S26H, S28H) */
    uint8_t u1RESRVD1 : 1; /** Reserved for future use */
} stc_cy_smif_semp_CFR1_t;

/** Configuration Register 2 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_CFR1_t field; /** < \ref stc_cy_smif_semp_CFR1_t */
} un_cy_smif_semp_CFR1_t;

/***** Configuration Register 2 Utilities *****/
/** Configuration Register 2 bit field definition */
typedef struct
{
    uint8_t u4MEMLAT : 4; /** Memory Array Read Latency selection - Dummy cycles required for initial data access (S25H, S26H, S28H) */
    uint8_t u1RESRVD : 1; /** Reserved for future use                                                             (S25H, S26H, S28H) */
    uint8_t u1DQ3RST : 1; /** DQ3 and RESET Selection for DQ3 - Multiplexed operation on I/O #3                   (S25H            ) */
    uint8_t u1QPI_IT : 1; /** QPI Interface & Protocol Selection - I/O width set to 4 bits (4-4-4)                (S25H            ) */
    uint8_t u1ADRBYT : 1; /** Address Byte Length selection between 3 or 4 bytes for Instructions                 (S25H, S26H, S28H) */
} stc_cy_smif_semp_CFR2_t;

/** Configuration Register 2 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_CFR2_t field; /** < \ref stc_cy_smif_semp_CFR2_t */
} un_cy_smif_semp_CFR2_t;


/***** Configuration Register 3 Utilities *****/
/** Configuration Register 3 bit field definition */
typedef struct
{
    uint8_t u1LSFRST : 1; /** Legacy Software Reset transaction F0h selection                                            (S25H            ) */
    uint8_t u1INTFTP : 1; /** Interface type selection between HyperBus and Legacy (x1) SPI                              (      S26H      ) */
    uint8_t u1CLSRSM : 1; /** Clear Status or Resume transaction 30h selection                                           (S25H            ) */
    uint8_t u1UNHYSA : 1; /** Uniform or Hybrid Sector Architecture selection                                            (S25H, S26H, S28H) */
    uint8_t u1PGMBUF : 1; /** Program Buffer Size selection                                                              (S25H, S26H, S28H) */
    uint8_t u1BLKCHK : 1; /** Blank Check selection during Erase operation for better endurance                          (S25H, S26H, S28H) */
    uint8_t u2VRGLAT : 2; /** Volatile Register Read Latency selection - Dummy cycles required for initial data accessed (S25H, S26H, S28H) */
} stc_cy_smif_semp_CFR3_t;

typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_CFR3_t field; /** < \ref stc_cy_smif_semp_CFR3_t */
} un_cy_smif_semp_CFR3_t;

/***** Configuration Register 4 Utilities *****/
/** Configuration Register 4 bit field definition */
typedef struct
{
    uint8_t u2RBSTWL : 2; /** Read Burst Wrap Length selection                                            (S25H, S26H, S28H) */
    uint8_t u1DPDPOR : 1; /** Deep Power Down power saving mode entry selection upon POR                  (S25H, S26H, S28H) */
    uint8_t u1ECC12S : 1; /** Error Correction Code (ECC) 1-bit or 1-bit/2-bit error correction selection (S25H, S26H, S28H) */
    uint8_t u1RBSTWP : 1; /** Read Burst Wrap Enable selection                                            (S25H, S26H, S28H) */
    uint8_t u3IOIMPD : 3; /** I/O Driver Output Impedance selection                                       (S25H, S26H, S28H) */
} stc_cy_smif_semp_CFR4_t;

/** Configuration Register 4 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_CFR4_t field; /** < \ref stc_cy_smif_semp_CFR4_t */
} un_cy_smif_semp_CFR4_t;

/***** Configuration Register 5 Utilities *****/
/** Configuration Register 5 bit field definition */
typedef struct
{
    uint8_t u1OPI_IT : 1; /** Octal Interface and Protocol Selection - I/O width set to 8 bits (8-8-8) (            S28H) */
    uint8_t u1SDRDDR : 1; /** Octal SPI SDR or DDR selection                                           (            S28H) */
    uint8_t u6RESRVD : 6; /** Reserved for Future Use                                                  (            S28H) */
} stc_cy_smif_semp_CFR5_t;

/** Configuration Register 5 union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_CFR5_t field; /** < \ref stc_cy_smif_semp_CFR5_t */
} un_cy_smif_semp_CFR5_t;


/***** Interface CRC Enable Register Utilities *****/
/** Interface CRC Enable Register bit field definition */
typedef struct
{
    uint8_t u1ITCRCE : 1; /** Interface CRC Selection (S26H,       S28H) */
    uint8_t u7RESVRD : 7; /** Reserved for Future Use (S26H,       S28H) */
} stc_cy_smif_semp_ICEV_t;

/** Interface CRC Enable Register union  */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_ICEV_t field; /** < \ref stc_cy_smif_semp_ICEV_t */
} un_cy_smif_semp_ICEV_t;

/***** Interface CRC Check-value Register Utilities *****/
/** Interface CRC Check-value Register bit field definition */
typedef struct
{
    uint32_t u32ITCRCV : 32; /**  VInterface CRC Checksum value (S26H,       S28H) */
} stc_cy_smif_semp_ICRV_t;

/** Interface CRC Check-value Register union */
typedef union
{
    uint32_t                u32;   /** < 32 bit access */
    stc_cy_smif_semp_ICRV_t field; /** < \ref stc_cy_smif_semp_ICRV_t */
} un_cy_smif_semp_ICRV_t;

/***** Memory Array Data Integrity Check CRC Register Utilities *****/
/** Memory Array Data Integrity Check CRC Register bit field definition */
typedef struct
{
    uint32_t u32DTCRCV : 32; /** Memory Array Data CRC Checksum Value (S25H, S26H, S28H) */
} stc_cy_smif_semp_DCRV_t;

/** Memory Array Data Integrity Check CRC Register union */
typedef union
{
    uint32_t                u32;   /** < 32 bit access */
    stc_cy_smif_semp_DCRV_t field; /** < \ref stc_cy_smif_semp_DCRV_t */
} un_cy_smif_semp_DCRV_t;

/***** ECC Status Register Utilities *****/
/** ECC Status Register bit field definition */
typedef struct
{
    uint8_t u3RESRVD0 : 3; /** Reserved for Future Use                             (S25H, S26H, S28H) */
    uint8_t u1ECC1BT  : 1; /** ECC Error 1-bit Error Detection and Correction Flag (S25H, S26H, S28H) */
    uint8_t u1ECC2BT  : 1; /** ECC Error 2-bit Error Detection Flag                (S25H, S26H, S28H) */
    uint8_t u3RESRVD1 : 3; /** Reserved for Future Use                             (S25H, S26H, S28H) */
} stc_cy_smif_semp_ESCV_t;

/** ECC Status Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_ESCV_t field; /** < \ref stc_cy_smif_semp_ESCV_t */
} un_cy_smif_semp_ESCV_t;


/***** ECC Address Trap Register Utilities *****/
/** ECC Address Trap Register bit field definition */
typedef struct
{
    uint32_t u32ECCATP : 32; /** ECC 1-bit and 2-bit Error Address Trap Register (S25H, S26H, S28H) */
} stc_cy_smif_semp_EATV_t;

/** ECC Address Trap Register union */
typedef union
{
    uint32_t                u32 ;  /** < 32 bit access */
    stc_cy_smif_semp_EATV_t field; /** < \ref stc_cy_smif_semp_EATV_t */
} un_cy_smif_semp_EATV_t;

/***** ECC Error Detection Count Register Utilities *****/
/** ECC Error Detection Count Register bit field definition */
typedef struct
{
    uint16_t u16ECCCNT : 16; /** ECC 1-bit and 2-bit Error Count Register (S25H, S26H, S28H) */
} stc_cy_smif_semp_ECTV_t;

/** ECC Error Detection Count Register union */
typedef union
{
    uint16_t                u16;   /** < 16 bit access */
    stc_cy_smif_semp_ECTV_t field; /** < \ref stc_cy_smif_semp_ECTV_t */
} un_cy_smif_semp_ECTV_t;

/***** Advanced Sector Protection Register Utilities *****/
/** Advanced Sector Protection Register bit field definition */
typedef struct
{
    uint16_t u1ASPPRM  : 1; /** Permanent Protection Selection                                            (S25H, S26H, S28H) */
    uint16_t u1ASPPER  : 1; /** Persistent Protection Selection (Register Protection Selection)           (S25H, S26H, S28H) */
    uint16_t u1ASPPWD  : 1; /** Password Based Protection Selection                                       (S25H, S26H, S28H) */
    uint16_t u1ASPPPB  : 1; /** Permanent Protection (PPB) bits for all sectors programmability Selection (S25H, S26H, S28H) */
    uint16_t u1ASPDYB  : 1; /** Dynamic Protection (DYB) for all sectors at power-up Selection            (S25H, S26H, S28H) */
    uint16_t u1ASPRDP  : 1; /** Read Password Based Protection Selection                                  (S25H, S26H, S28H) */
    uint16_t u10RESRVD : 10; /** Reserved for Future Use */
} stc_cy_smif_semp_ASPO_t;

/** Advanced Sector Protection Register union */
typedef union
{
    uint16_t                u16;   /** < 16 bit access */
    stc_cy_smif_semp_ASPO_t field; /** < \ref stc_cy_smif_semp_ASPO_t */
} un_cy_smif_semp_ASPO_t;

/***** ASP Password Register Utilities *****/
/** ASP Password Register bit field definition */
typedef struct
{
    uint64_t u64ASPPRM  : 64; /** ECC 1-bit and 2-bit Error Count Register (S25H, S26H, S28H) */
} stc_cy_smif_semp_PWDO_t;

/** ASP Password Register union */
typedef union
{
    uint64_t                u64;   /** < 64 bit access */
    stc_cy_smif_semp_PWDO_t field; /** < \ref stc_cy_smif_semp_PWDO_t */
} un_cy_smif_semp_PWDO_t;

/***** ASP PPB Lock Register Utilities *****/
/** ASP PPB Lock Register bit field definition */
typedef struct
{
    uint8_t u1PPBLCK  : 1; /** PPB Temporary Protection Selection (S25H, S26H, S28H) */
    uint8_t u7RESVRD  : 7; /** Reserved for Future Use */
} stc_cy_smif_semp_PPLV_t;

/** ASP PPB Lock Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_PPLV_t field; /** < \ref stc_cy_smif_semp_PPLV_t */
} un_cy_smif_semp_PPLV_t;

/***** ASP PPB Access Register Utilities *****/
/** ASP PPB Access Register bit field definition */
typedef struct
{
    uint8_t u8PPBACS  : 8; /** (S25H, S26H, S28H) */
} stc_cy_smif_semp_PPAV_t;

/** ASP PPB Access Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_PPAV_t field; /** < \ref stc_cy_smif_semp_PPAV_t */
} un_cy_smif_semp_PPAV_t;

/***** ASP Dynamic Block Access Register Utilities *****/
/** ASP Dynamic Block Access Register bit field definition */
typedef struct
{
    uint8_t u8DYBACS  : 8; /** Sector Based DYB Protection Status (S25H, S26H, S28H) */
} stc_cy_smif_semp_DYAV_t;

/** ASP Dynamic Block Access Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_DYAV_t field; /** < \ref stc_cy_smif_semp_DYAV_t */
} un_cy_smif_semp_DYAV_t;

/***** Data Learning Register Utilities *****/
/** Data Learning Register bit field definition */
typedef struct
{
    uint32_t u8DTLRPT  : 8; /** Data Learning Pattern Selection (S25H            ) */
} stc_cy_smif_semp_DLPN_t;

/** Data Learning Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_DLPN_t field; /** < \ref stc_cy_smif_semp_DLPN_t */
} un_cy_smif_semp_DLPN_t;

/***** AutoBoot Register Utilities *****/
/** AutoBoot Register bit field definition */
typedef struct
{
    uint32_t u1ATBTEN : 1;  /** AutoBoot Feature Selection                                             (S25H, S26H, S28H) */
    uint32_t u8STDLY  : 8;  /** AutoBoot Read Starting Delay Selection                                 (S25H, S26H, S28H) */
    uint32_t u23STADR : 23; /** Starting Address Selection where AutoBoot will start reading data from (S25H, S26H, S28H) */
} stc_cy_smif_semp_ATBN_t;

/** AutoBoot Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_ATBN_t field; /** < \ref stc_cy_smif_semp_ATBN_t */
} un_cy_smif_semp_ATBN_t;

/***** Sector Erase Count Register Utilities *****/
/** Sector Erase Count Register bit field definition */
typedef struct
{
    uint32_t u23SECVAL : 23; /** Sector Erase Count Value                 (S25H, S26H, S28H) */
    uint32_t u1SECCPT  : 1; /** Sector Erase Count Corruption Status Flag (S25H, S26H, S28H) */
    uint32_t u8RESVRD  : 8; /** Reserved for future use                   (S25H, S26H, S28H) */
} stc_cy_smif_semp_SECV_t;

/** Sector Erase Count Register union */
typedef union
{
    uint32_t                u32;   /** < 32 bit access */
    stc_cy_smif_semp_SECV_t field; /** < \ref stc_cy_smif_semp_SECV_t */
} un_cy_smif_semp_SECV_t;

/***** INT# Pin Configuration Register Utilities *****/
/** INT# Pin Configuration Register bit field definition */
typedef struct
{
    uint8_t u1ECC1BT  : 1; /** ECC 1-bit Error Detection and Correction Selection (            S28H) */
    uint8_t u1ECC2BT  : 1; /** ECC 2-bit Error Detection Selection0               (            S28H) */
    uint8_t u2RESRVD0 : 2; /** Reserved for Future Use */
    uint8_t u1REYBSY  : 1; /** Ready/Busy Transition Selection                    (            S28H) */
    uint8_t u2RESRVD1 : 2; /** Reserved for Future Use */
    uint8_t u1INTBEN  : 1; /** INT# pin Enable Selection                          (            S28H) */
} stc_cy_smif_semp_INCV_t;

/** INT# Pin Configuration Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_INCV_t field; /** < \ref stc_cy_smif_semp_INCV_t */
} un_cy_smif_semp_INCV_t;

/***** INT# Pin Status Register Utilities *****/
/** INT# Pin Status Register bit field definition */
typedef struct
{
    uint8_t u1ECC1BT  : 1; /** ECC 1-bit Error Detection and Correction (            S28H) */
    uint8_t u1ECC2BT  : 1; /** ECC 2-bit Error Detection                (            S28H) */
    uint8_t u2RESRVD0 : 2; /** Reserved for Future Use                  (            S28H) */
    uint8_t u1REYBSY  : 1; /** Ready/Busy Transition                    (            S28H) */
    uint8_t u3RESRVD1 : 3; /** Reserved for Future Use                  (            S28H) */
} stc_cy_smif_semp_INSV_t;

/** INT# Pin Status Register union */
typedef union
{
    uint8_t                 u8;    /** < 8 bit access */
    stc_cy_smif_semp_INSV_t field; /** < \ref stc_cy_smif_semp_INSV_t */
} un_cy_smif_semp_INSV_t;

/***** EnduraFlex Architecture Selection Register 1 ~ 4 Utilities *****/
/** EnduraFlex Architecture Selection Register 1 ~ 4 bit field definition */
typedef struct
{
    uint16_t u1EPTEBx  : 1; /** EnduraFlex Pointer 4 Enable# Selection           (S25H, S26H, S28H) */
    uint16_t u1ERGNTx  : 1; /** EnduraFlex Pointer 4 based Region Type Selection (S25H, S26H, S28H) */
    uint16_t u8EPTADx  : 8; /** EnduraFlex Pointer 4 Address Selection           (S25H, S26H, S28H) */
    uint16_t u6RESRVD  : 6; /** Reserved for future use */
} stc_cy_smif_semp_EFX1TO4_t;

/** EnduraFlex Architecture Selection Register 1 ~ 4 union */
typedef union
{
    uint16_t                   u16;   /** < 16 bit access */
    stc_cy_smif_semp_EFX1TO4_t field; /** < \ref stc_cy_smif_semp_EFX1TO4_t */
} un_cy_smif_semp_EFX1TO4_t;

/***** EnduraFlex Architecture Selection Register 0 Utilities *****/
/** EnduraFlex Architecture Selection Register 0 bit field definition */
typedef struct
{
    uint16_t u1WRLVEN  : 1; /** Wear Leveling Enable Selection          (S25H, S26H, S28H) */
    uint16_t u1GBLSEL  : 1; /** All Sectors based Region type Selection (S25H, S26H, S28H) */
    uint16_t u14RESRVD : 14; /** Reserved fora future use */
} stc_cy_smif_semp_EFX0_t;

/** EnduraFlex Architecture Selection Register 0 union */
typedef union
{
    uint16_t                u16;   /** < 16 bit access */
    stc_cy_smif_semp_EFX0_t field; /** < \ref stc_cy_smif_semp_EFX0_t */
} un_cy_smif_semp_EFX0_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/** Specifies Semper Register Address */
typedef enum
{
    /* Non-Volatile registers */
    CY_SEMP_REG_ADDR_STR1_NV       = 0x00000000UL, /** < Non-Volatile Status Register 1                                   (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR1_NV       = 0x00000002UL, /** < Non-Volatile Configuration Register 1                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR2_NV       = 0x00000003UL, /** < Non-Volatile Configuration Register 2                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR3_NV       = 0x00000004UL, /** < Non-Volatile Configuration Register 3                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR4_NV       = 0x00000005UL, /** < Non-Volatile Configuration Register 4                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR5_NV       = 0x00000006UL, /** < Non-Volatile Configuration Register 5                            (            S28H) */
    CY_SEMP_REG_ADDR_DLP_NV        = 0x00000010UL, /** < Non-Volatile Data Learning Register                              (S25H,           ) */
    CY_SEMP_REG_ADDR_EFX0_NV       = 0x00000050UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 0        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX1_BYTE0_NV = 0x00000052UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 1 Byte 0 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX1_BYTE1_NV = 0x00000053UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 1 Byte 1 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX2_BYTE0_NV = 0x00000054UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 2 Byte 0 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX2_BYTE1_NV = 0x00000055UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 2 Byte 1 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX3_BYTE0_NV = 0x00000056UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 3 Byte 0 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX3_BYTE1_NV = 0x00000057UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 3 Byte 1 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX4_BYTE0_NV = 0x00000058UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 4 Byte 0 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EFX4_BYTE1_NV = 0x00000059UL, /** < Non-Volatile EnduraFlex Architecture Selection Register 4 Byte 1 (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ATBN_BYTE0_NV = 0x00000042UL, /** < Non-Volatile AutoBoot Register Byte 0                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ATBN_BYTE1_NV = 0x00000043UL, /** < Non-Volatile AutoBoot Register Byte 1                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ATBN_BYTE2_NV = 0x00000044UL, /** < Non-Volatile AutoBoot Register Byte 2                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ATBN_BYTE3_NV = 0x00000045UL, /** < Non-Volatile AutoBoot Register Byte 3                            (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ASPO_BYTE0_NV = 0x00000030UL, /** < Non-Volatile Advanced Sector Protection Register Byte 0          (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ASPO_BYTE1_NV = 0x00000031UL, /** < Non-Volatile Advanced Sector Protection Register Byte 1          (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE0_NV = 0x00000020UL, /** < Non-Volatile ASP Password Register Byte 0                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE1_NV = 0x00000021UL, /** < Non-Volatile ASP Password Register Byte 1                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE2_NV = 0x00000022UL, /** < Non-Volatile ASP Password Register Byte 2                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE3_NV = 0x00000023UL, /** < Non-Volatile ASP Password Register Byte 3                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE4_NV = 0x00000024UL, /** < Non-Volatile ASP Password Register Byte 4                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE5_NV = 0x00000025UL, /** < Non-Volatile ASP Password Register Byte 5                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE6_NV = 0x00000026UL, /** < Non-Volatile ASP Password Register Byte 6                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_PWDO_BYTE7_NV = 0x00000027UL, /** < Non-Volatile ASP Password Register Byte 7                        (S25H, S26H, S28H) */

    /* Volatile registers */
    CY_SEMP_REG_ADDR_STR1_V        = 0x00800000UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_STR2_V        = 0x00800001UL, /** < Volatile Status Register 2                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR1_V        = 0x00800002UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR2_V        = 0x00800003UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR3_V        = 0x00800004UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR4_V        = 0x00800005UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_CFR5_V        = 0x00800006UL, /** < Volatile Status Register 1                                       (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ICE_V         = 0x00800008UL, /** < Volatile Interface CRC Enable Register                           (      S26H, S28H) */
    CY_SEMP_REG_ADDR_DLP_V         = 0x00800010UL, /** < Volatile Data Learning Register                                  (S25H,           ) */
    CY_SEMP_REG_ADDR_INCV_V        = 0x00800068UL, /** < Volatile Interrupt Configuration Register                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_INSV_V        = 0x00800067UL, /** < Volatile Interrupt Status  Register                              (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ESCV_V        = 0x00800089UL, /** < Volatile ECC Status Register                                     (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ECTV_BYTE0_V  = 0x0080008AUL, /** < Volatile ECC Error Detection Count Register Byte 0               (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_ECTV_BYTE1_V  = 0x0080008BUL, /** < Volatile ECC Error Detection Count Register Byte 1               (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EATV_BYTE0_V  = 0x0080008EUL, /** < Volatile ECC Address Trap Register Byte 0                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EATV_BYTE1_V  = 0x0080008FUL, /** < Volatile ECC Address Trap Register Byte 1                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EATV_BYTE2_V  = 0x00800040UL, /** < Volatile ECC Address Trap Register Byte 2                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_EATV_BYTE3_V  = 0x00800041UL, /** < Volatile ECC Address Trap Register Byte 3                        (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_SECV_BYTE0_V  = 0x00800091UL, /** < Volatile Sector Erase Count Register Byte 0                      (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_SECV_BYTE1_V  = 0x00800092UL, /** < Volatile Sector Erase Count Register Byte 1                      (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_SECV_BYTE2_V  = 0x00800093UL, /** < Volatile Sector Erase Count Register Byte 1                      (S25H, S26H, S28H) */
    CY_SEMP_REG_ADDR_DCRV_BYTE0_V  = 0x00800095UL, /** < Volatile Data Integrity Check CRC Register Byte 0                (      S26H, S28H) */
    CY_SEMP_REG_ADDR_DCRV_BYTE1_V  = 0x00800096UL, /** < Volatile Data Integrity Check CRC Register Byte 1                (      S26H, S28H) */
    CY_SEMP_REG_ADDR_DCRV_BYTE2_V  = 0x00800097UL, /** < Volatile Data Integrity Check CRC Register Byte 2                (      S26H, S28H) */
    CY_SEMP_REG_ADDR_DCRV_BYTE3_V  = 0x00800098UL, /** < Volatile Data Integrity Check CRC Register Byte 3                (      S26H, S28H) */
    CY_SEMP_REG_ADDR_PPLV_V        = 0x0080009BUL, /** < Volatile ASP PPB Lock Register                                   (S25H, S26H, S28H) */
} cy_en_smif_semp_reg_addr_t;

/****************************************/
/** Semper Flash Register Utilities End */
/****************************************/

/** Specifies Semper Flash Read command ID for this driver */
typedef enum
{
    CY_SMIF_SEMP_RDID_1S1S1S_3ADR      = 0u,  /**< Legacy SPI Read SDR (3-byte address)          (S25H, S26H, S28H) */
    CY_SMIF_SEMP_RDID_1S1S1S_4ADR,            /**< Legacy SPI Read SDR (4-byte address)          (S25H, S26H, S28H) */
    CY_SMIF_SEMP_RDID_1S1S1S_FAST_3ADR,       /**< Legacy SPI Fast Read SDR (3-byte address)     (S25H, S26H, S28H) */
    CY_SMIF_SEMP_RDID_1S1S1S_FAST_4ADR,       /**< Legacy SPI Fast Read SDR (4-byte address)     (S25H, S26H, S28H) */
    CY_SMIF_SEMP_RDID_1S2S2S_3ADR,            /**< Dual Read SDR (3-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S2S2S_4ADR,            /**< Dual Read SDR (4-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_2S2S_3ADR,              /**< Dual Continuous Read SDR (3-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_2S2S_4ADR,              /**< Dual Continuous Read SDR (4-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S1S4S_3ADR,            /**< Quad Output Read SDR (3-byte address)         (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S1S4S_4ADR,            /**< Quad Output Read SDR (4-byte address)         (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4S4S_3ADR,            /**< Quad I/O Read SDR (3-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4S4S_4ADR,            /**< Quad I/O Read SDR (4-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4D4D_3ADR,            /**< Quad I/O Read DDR (3-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4D4D_4ADR,            /**< Quad I/O Read DDR (4-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S4S_3ADR,            /**< Quad Read SDR (3-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S4S_4ADR,            /**< Quad Read SDR (4-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S_3ADR,              /**< Quad Continuous Read SDR (3-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S_4ADR,              /**< Quad Continuous Read SDR (4-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D4D_3ADR,            /**< Quad Read DDR (3-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D4D_4ADR,            /**< Quad Read DDR (4-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D_3ADR,              /**< Quad Continuous Read DDR (3-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D_4ADR,              /**< Quad Continuous Read DDR (4-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_8S8S8S_4ADR,            /**< Octal Read SDR (4-byte address)               (            S28H) */
    CY_SMIF_SEMP_RDID_8D8D8D_4ADR,            /**< Octal Read DDR (4-byte address)               (            S28H) */
    CY_SMIF_SEMP_RDID_1S1S4S_3ADR_DLP,        /**< Quad Output Read SDR (3-byte address)         (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S1S4S_4ADR_DLP,        /**< Quad Output Read SDR (4-byte address)         (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4S4S_3ADR_DLP,        /**< Quad I/O Read SDR (3-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4S4S_4ADR_DLP,        /**< Quad I/O Read SDR (4-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4D4D_3ADR_DLP,        /**< Quad I/O Read DDR (3-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_1S4D4D_4ADR_DLP,        /**< Quad I/O Read DDR (4-byte address)            (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S4S_3ADR_DLP,        /**< Quad Read SDR (3-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S4S_4ADR_DLP,        /**< Quad Read SDR (4-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S_3ADR_DLP,          /**< Quad Continuous Read SDR (3-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4S4S_4ADR_DLP,          /**< Quad Continuous Read SDR (4-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D4D_3ADR_DLP,        /**< Quad Read DDR (3-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D4D_4ADR_DLP,        /**< Quad Read DDR (4-byte address)                (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D_3ADR_DLP,          /**< Quad Continuous Read DDR (3-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_RDID_4D4D_4ADR_DLP,          /**< Quad Continuous Read DDR (4-byte address)     (S25H,           ) */
    CY_SMIF_SEMP_READ_ID_NUM,
} cy_en_smif_semp_rd_cmdid_t;

/** Specifies Semper Flash Write command ID for this driver */
typedef enum
{
    CY_SMIF_SEMP_WTID_1S1S1S_3ADR   = 0u, /**< Legacy SPI mode Page Program (3-byte address) (S25H, S26H, S28H) */
    CY_SMIF_SEMP_WTID_1S1S1S_4ADR,        /**< Legacy SPI mode Page Program (4-byte address) (S25H, S26H, S28H) */
    CY_SMIF_SEMP_WTID_4S4S4S_3ADR,        /**< SDR Quad mode Page Program (3-byte address)   (S25H,           ) */
    CY_SMIF_SEMP_WTID_4S4S4S_4ADR,        /**< SDR Quad mode Page Program (4-byte address)   (S25H,           ) */
    CY_SMIF_SEMP_WTID_8S8S8S_4ADR,        /**< SDR Octal mode Page Program (4-byte address)  (            S28H) */
    CY_SMIF_SEMP_WTID_8D8D8D_4ADR,        /**< DDR Octal mode Page Program (4-byte address)  (            S28H) */
    CY_SMIF_SEMP_WRITE_ID_NUM,
} cy_en_smif_semp_wt_cmdid_t;

/** Specifies Semper flash memory read latency code */
typedef enum
{
    CY_SMIF_SEMP_RD_LATENCY0  = 0u,  /**< Latency code 0 */
    CY_SMIF_SEMP_RD_LATENCY1  = 1u,  /**< Latency code 1 */
    CY_SMIF_SEMP_RD_LATENCY2  = 2u,  /**< Latency code 2 */
    CY_SMIF_SEMP_RD_LATENCY3  = 3u,  /**< Latency code 3 */
    CY_SMIF_SEMP_RD_LATENCY4  = 4u,  /**< Latency code 4 */
    CY_SMIF_SEMP_RD_LATENCY5  = 5u,  /**< Latency code 5 */
    CY_SMIF_SEMP_RD_LATENCY6  = 6u,  /**< Latency code 6 */
    CY_SMIF_SEMP_RD_LATENCY7  = 7u,  /**< Latency code 7 */
    CY_SMIF_SEMP_RD_LATENCY8  = 8u,  /**< Latency code 8 */
    CY_SMIF_SEMP_RD_LATENCY9  = 9u,  /**< Latency code 9 */
    CY_SMIF_SEMP_RD_LATENCY10 = 10u, /**< Latency code 10 */
    CY_SMIF_SEMP_RD_LATENCY11 = 11u, /**< Latency code 11 */
    CY_SMIF_SEMP_RD_LATENCY12 = 12u, /**< Latency code 12 */
    CY_SMIF_SEMP_RD_LATENCY13 = 13u, /**< Latency code 13 */
    CY_SMIF_SEMP_RD_LATENCY14 = 14u, /**< Latency code 14 */
    CY_SMIF_SEMP_RD_LATENCY15 = 15u, /**< Latency code 15 */
} cy_en_smif_semp_read_latency_code_t;
#define CY_SMIF_SEMP_READ_LATENCY_CODE_MAX (16u)
#define CY_SMIF_SEMP_READ_LC_DC_TABLE_MAX  (2u)

/** Specifies Semper flash memory write latency code */
typedef enum
{
    CY_SMIF_SEMP_WT_LATENCY0 = 0u,  /**< Latency code 0 */
} cy_en_smif_semp_write_latency_code_t;
#define CY_SMIF_SEMP_WRITE_LATENCY_CODE_MAX (1u)
#define CY_SMIF_SEMP_WRITE_LC_DC_TABLE_MAX  (1u)

/** Specifies Semper register access latency code */
typedef enum
{
    CY_SMIF_SEMP_REG_LATENCY0 = 0u,  /**< Latency code 0 */
    CY_SMIF_SEMP_REG_LATENCY1 = 1u,  /**< Latency code 1 */
    CY_SMIF_SEMP_REG_LATENCY2 = 2u,  /**< Latency code 2 */
    CY_SMIF_SEMP_REG_LATENCY3 = 3u,  /**< Latency code 3 */
} cy_en_smif_semp_reg_latency_code_t;
#define CY_SMIF_SEMP_REG_LATENCY_CODE_MAX (4u)
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Specifies Semper Manufacturer and Device ID structure */
typedef struct
{
    uint8_t manufacturerId;      /**< Manufacturer ID for Cypress */
    uint8_t DeviceId_MSB;        /**< Device ID MSB - Memory Interface Type */
    uint8_t DeviceId_LSB;        /**< Device ID LSB - Density */
    uint8_t idLength;            /**< ID Length - number bytes following. */
    uint8_t physicalSecotorArch; /**< Physical Sector Architecture */
    uint8_t familyId;            /**< Family ID */
} cy_stc_smif_semp_id_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
typedef enum
{
    /* During the Quad I/O Read Command Sequence, 
       if the Mode bits are any value other than Axh, 
       then the next time CS# is raised high the device will be 
       released from Quad I/O High Performance Read mode.
       During the DDR Quad I/O Read Command Sequence, 
       If the upper nibble (IO[7:4]) and lower nibble (IO[3:0]) 
       of the Mode bits are complementary (i.e. 5h and Ah) the device 
       transitions to Continuous Read DDR Quad I/O Mode
    */
    Cy_SMIF_SEMP_MODE_CONTINUOUS     = 0xA5, 
    Cy_SMIF_SEMP_MODE_NOT_CONTINUOUS = 0x00,
} cy_en_smif_semp_mode_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_functions
* \{
*/
extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadId(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    cy_stc_smif_semp_id_t*             readID,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              transaction,
    uint32_t                           readSizeInByte,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadJedecId(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint64_t*                 readData[],
    uint32_t                  readAddress,
    uint32_t                  readSize,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadUniqueId(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint64_t*                          uniqueId,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              transaction,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadStatusRegister1(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint8_t*                           readData,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              transaction,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadStatusRegister2(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint8_t*                           readData,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              transaction,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadAnyNonVolatileRegister(
    volatile cy_stc_smif_reg_t*                base,
    cy_en_smif_slave_select_t           slave,
    uint8_t*                            readData,
    cy_en_smif_semp_reg_addr_t          regAddr,
    cy_en_smif_semp_read_latency_code_t latencyCode,
    cy_en_smif_trx_addr_len_t           addrLen,
    cy_en_smif_trx_type_t               transaction,
    cy_stc_smif_context_t*              context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadAnyVolatileRegister(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint8_t*                           readData,
    cy_en_smif_semp_reg_addr_t         regAddr,
    cy_en_smif_semp_reg_latency_code_t latencyCode,
    cy_en_smif_trx_addr_len_t          addrLen,
    cy_en_smif_trx_type_t              transaction,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteEnable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteDisable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteAnyRegister(
    volatile cy_stc_smif_reg_t*       base,
    cy_en_smif_slave_select_t  slave,
    uint8_t                    writeData,
    cy_en_smif_semp_reg_addr_t regAddr,
    cy_en_smif_trx_addr_len_t  addrLen,
    cy_en_smif_trx_type_t      transaction,
    cy_stc_smif_context_t*     context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ClearProgramAndEraseFailureFlags(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadECC_DataUnitStatus(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t*                  readStatus,
    uint32_t                  unitAdder,
    cy_en_smif_semp_read_latency_code_t lc,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ClearECC_StatusRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_DataIntegrityCheck(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  startAddress,
    uint32_t                  endAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Erase_4KB_Sector(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Erase_256KB_Sector(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_EraseChip(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_EvaluateEraseStatus(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_SectorEraseCount(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_SuspendEraseProgram(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_ResumeEraseProgram(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_SoftwareResetEnable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_SoftwareReset(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_EnterDeepPowerDownMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_EnableHyperBusInterface(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave);


/***************************************************************************
* Functions For Only S25H series 
****************************************************************************/


extern cy_en_smif_status_t Cy_SMIF_SEMP_ReadConfigurationRegister1(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint8_t*                           readData,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              trx,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteEnableVolatileRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   stsReg,
    uint8_t                   cfgReg1,
    uint8_t                   cfgReg2,
    uint8_t                   cfgReg3,
    uint8_t                   cfgReg4,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_SafeBootWriteRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   stsReg,
    uint8_t                   cfgReg1,
    uint8_t                   cfgReg2,
    uint8_t                   cfgReg3,
    uint8_t                   cfgReg4,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Enter4ByteAddressMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Exit4ByteAddressMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Read_DLP_Register(
    volatile cy_stc_smif_reg_t*               base,
    cy_en_smif_slave_select_t          slave,
    uint8_t*                           dlpData,
    cy_en_smif_semp_reg_latency_code_t regLc,
    cy_en_smif_trx_type_t              trx,
    cy_stc_smif_context_t*             context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Program_DLP_NV_Register(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   dlpData,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_Write_DLP_V_Register(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   dlpData,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

extern cy_en_smif_status_t Cy_SMIF_SEMP_WriteAutoBootRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  autoBootRegVal,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

/** \} group_mw_smif_functions */

extern const cy_stc_cmd_sq_t         g_sempReadCmdTypes[];
extern const cy_stc_addr_sq_t        g_sempReadAddrTypes[];
extern const cy_stc_mode_sq_t        g_sempReadModeTypes[];
extern const cy_stc_dummy_sq_t       g_sempReadDummyTypes[];
extern const uint8_t*                g_sempReadLatencyCodeToDummyCycle[CY_SMIF_SEMP_READ_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_sempReadDataTypes[];
extern const cy_stc_read_info_t      g_sempReadCmdInfo[];
extern const cy_stc_cmd_sq_t         g_sempWriteCmdTypes[];
extern const cy_stc_addr_sq_t        g_sempWriteAddrTypes[];
extern const cy_stc_mode_sq_t        g_sempWriteModeTypes[];
extern const cy_stc_dummy_sq_t       g_sempWriteDummyTypes[];
extern const uint8_t*                g_sempWriteLatencyCodeToDummyCycle[CY_SMIF_SEMP_WRITE_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_sempWriteDataTypes[];
extern const cy_stc_write_info_t     g_sempWriteCmdInfo[];


#if defined(__cplusplus)
}
#endif

#endif /* (CY_SMIF_SEMP_H) */

