/***************************************************************************//**
* \file cy_smif_s25fs.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress S25FSER HSSPI series driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_SMIF_S25FS_H)
#define CY_SMIF_S25FS_H

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
typedef enum
{
    //                                 constant|  latency|   RWDS|  addr length| address| 
    //                            latency cycle| table No| enable| configurable|  length| Command Value
    /* Register Access */
    CY_S25FS_CMD_1BYTE_WRENB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0006, /**< Write Enable */
    CY_S25FS_CMD_1BYTE_WRDIS_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0004, /**< Write Disable */

    CY_S25FS_CMD_1BYTE_CLPEF_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0082, /**< Clear Program and Erase Failure Flags */
    CY_S25FS_CMD_1BYTE_EN4BA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B7, /**< Enter 4 byte Address Mode */
    CY_S25FS_CMD_1BYTE_EX4BA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B8, /**< Exit 4 byte Address Mode */

    /* Erase Flash Array */
    CY_S25FS_CMD_1BYTE_ERCHP_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0060, /**< Erase Chip (or 0x00C7) */

    /* Suspend / Resume */
    CY_S25FS_CMD_1BYTE_SPEPA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0085, /**< Suspend Erase / Program (or 0x00B0) */
    CY_S25FS_CMD_1BYTE_RSEPA_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x008A, /**< Resume Erase / Program */

    /* Advanced Sector Protection */
    CY_S25FS_CMD_1BYTE_ERPPB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E4, /**< Erase Persistent Protection Bit */
    CY_S25FS_CMD_1BYTE_WRPLB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00A6, /**< Write PPB Protection Lock Bit */

    /* Reset */
    CY_S25FS_CMD_1BYTE_SRSTE_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0066, /**< Software Reset Enable */
    CY_S25FS_CMD_1BYTE_SFRST_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0099, /**< Software Reset  */
    CY_S25FS_CMD_1BYTE_SFRSL_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00F0, /**< Legacy Software Reset */
    /* Deep Power Down */
    CY_S25FS_CMD_1BYTE_ENDPD_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00B9, /**< Enter Deep Power Down Mode */

    CY_S25FS_CMD_INVALID              = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_s25fs_cmd_t;

typedef enum
{
    //                                 constant|  latency|   RWDS|  addr length| address| 
    //                            latency cycle| table No| enable| configurable|  length| Command Value
    /* Erase Flash Array */
    CY_S25FS_CMD_1BYTE_ER004_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0020, /**< Erase 4-KB Sector */
    CY_S25FS_CMD_1BYTE_ER004_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0021, /**< Erase 4-KB Sector with 4byte address*/
    CY_S25FS_CMD_1BYTE_ER256_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00D8, /**< Erase 256-KB Sector */
    CY_S25FS_CMD_1BYTE_ER256_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00DC, /**< Erase 256-KB Sector */
    CY_S25FS_CMD_1BYTE_EVERS_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00D0, /**< Evaluate Erase Status */

    /* Advanced Sector Protection */
    CY_S25FS_CMD_1BYTE_PRPPB_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FD, /**< Program Persistent Protection Bit */
    CY_S25FS_CMD_1BYTE_PRPPB_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E3, /**< Program Persistent Protection Bit */

    CY_S25FS_CMD_ADDR_INVALID         = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_s25fs_cmd_addr_t;

typedef enum
{
    //                                 constant|  latency|   RWDS|  addr length| address| 
    //                            latency cycle| table No| enable| configurable|  length| Command Value
    /* Read Device ID */
    CY_S25FS_CMD_1BYTE_RDIDN_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x009F, /**< Read manufacturer and device identification */
    CY_S25FS_CMD_1BYTE_RDQID_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00AF, /**< Read manufacturer and device identification */
    CY_S25FS_CMD_1BYTE_RSFDP_3_0      = (8<<24)|  (0<<21)|(0<<20)|      (0<<19)| (3<<16)| 0x005A, /**< Read JEDEC Serial Flash Discoverable Parameters */

    /* Register Access */
    CY_S25FS_CMD_1BYTE_RDSR1_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0005, /**< Read Status Register 1 */
    CY_S25FS_CMD_1BYTE_RDSR2_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0007, /**< Read Status Register-2 */
    CY_S25FS_CMD_1BYTE_RDCR1_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0035, /**< Read Configuration Register 1 */
    CY_S25FS_CMD_1BYTE_RDARG_C_0      = (0<<24)|  (1<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0065, /**< Read Any Register (Non-Volatile) */
    CY_S25FS_CMD_1BYTE_RDDLP_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0041, /**< Read Data Learning Pattern Register */
    /* ECC */
    CY_S25FS_CMD_1BYTE_RDECC_4_0      = (0<<24)|  (1<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x0018, /**< Read ECC Status */
    CY_S25FS_CMD_1BYTE_RDECC_C_0      = (0<<24)|  (1<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0019, /**< Read ECC Status */
    /* Secure Silicon Region (OTP) */
    CY_S25FS_CMD_1BYTE_RDSSR_C_0      = (0<<24)|  (1<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x004B, /**< Read Secure Silicon Region */
    /* Advanced Sector Protection */
    CY_S25FS_CMD_1BYTE_RDDYB_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FA, /**< Read Dynamic Protection Bit */
    CY_S25FS_CMD_1BYTE_RDDYB_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E0, /**< Read Dynamic Protection Bit */
    CY_S25FS_CMD_1BYTE_RDPPB_C_0      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FC, /**< Read Persistent Protection Bit */
    CY_S25FS_CMD_1BYTE_RDPPB_4_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E2, /**< Read Persistent Protection Bit */
    CY_S25FS_CMD_1BYTE_RDPLB_0_0      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00A7, /**< Read Password Protection Mode Lock Bit */

    CY_S25FS_RD_CMD_INVALID           = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_s25fs_rd_cmd_t;

typedef enum
{
    //                                  constant|  latency|   RWDS|  addr length| address| 
    //                             latency cycle| table No| enable| configurable|  length| Command Value
    /* Register Access */
    CY_S25FS_CMD_1BYTE_WRREG_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0001, /**< Write Stat Reg 1 and Cfg Reg 1*/
    CY_S25FS_CMD_1BYTE_WRARG_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0071, /**< Write Any Register */
    CY_S25FS_CMD_1BYTE_WRDLP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x004A, /**< Write Data Learning Pattern Volatile Register */
    CY_S25FS_CMD_1BYTE_PRDLP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x0043, /**< Program Data Learning Pattern NV Register */
    /* Secure Silicon Region (OTP) */
    CY_S25FS_CMD_1BYTE_PRSSR_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x0042, /**< Program Secure Silicon Region */
    /* Advanced Sector Protection */
    CY_S25FS_CMD_1BYTE_PRASP_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x002F, /**< ASP Program */
    CY_S25FS_CMD_1BYTE_WRDYB_C_1      = (0<<24)|  (0<<21)|(0<<20)|      (1<<19)| (0<<16)| 0x00FB, /**< Write Dynamic Protection Bit */
    CY_S25FS_CMD_1BYTE_WRDYB_4_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (4<<16)| 0x00E1, /**< Write Dynamic Protection Bit */
    CY_S25FS_CMD_1BYTE_PWDUL_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E9, /**< Password Unlock */
    CY_S25FS_CMD_1BYTE_PGPWD_0_1      = (0<<24)|  (0<<21)|(0<<20)|      (0<<19)| (0<<16)| 0x00E8, /**< Program Password */

    CY_S25FS_WT_CMD_INVALID           = CY_SPI_INVALID_CMD, /**< Invalid Command */
} cy_en_smif_s25fs_wt_cmd_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    /* Read Flash Array */
    CY_S25FS_CMD_1BYTE_RDAY1_C_0      = 0x00000003, /**< Read */
    CY_S25FS_CMD_1BYTE_RDAY1_4_0      = 0x00000013, /**< Read */
    CY_S25FS_CMD_1BYTE_RDAY2_C_0      = 0x0000000B, /**< Read Fast */
    CY_S25FS_CMD_1BYTE_RDAY2_4_0      = 0x0000000C, /**< Read Fast */
    CY_S25FS_CMD_1BYTE_RDAY3_C_0      = 0x000000BB, /**< Read SDR Dual I/O */
    CY_S25FS_CMD_1BYTE_RDAY3_4_0      = 0x000000BC, /**< Read SDR Dual I/O */
    CY_S25FS_CMD_1BYTE_RDAY4_C_0      = 0x0000006B, /**< Read SDR Quad Output */
    CY_S25FS_CMD_1BYTE_RDAY4_4_0      = 0x0000006C, /**< Read SDR Quad Output */
    CY_S25FS_CMD_1BYTE_RDAY5_C_0      = 0x000000EB, /**< Read SDR Quad I/O */
    CY_S25FS_CMD_1BYTE_RDAY5_4_0      = 0x000000EC, /**< Read SDR Quad I/O */
    CY_S25FS_CMD_1BYTE_RDAY7_C_0      = 0x000000ED, /**< Read DDR Quad I/O */
    CY_S25FS_CMD_1BYTE_RDAY7_4_0      = 0x000000EE, /**< Read DDR Quad I/O */
} cy_en_smif_s25fs_mem_rd_cmd_t;

typedef enum
{
    /*****************************/
    /*      1 byte commands      */
    /*****************************/
    /* Program Flash Array */
    CY_S25FS_CMD_1BYTE_PRPGE_C_1      = 0x00000002, /**< Program Page */
    CY_S25FS_CMD_1BYTE_PRPGE_4_1      = 0x00000012, /**< Program Page */
} cy_en_smif_s25fs_mem_wt_cmd_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/************************************/
/** S25FS Flash Register Utilities */
/************************************/

/***** Status Register 1 Utilities *****/
/** Status Register 1 bit field definition */
typedef struct
{
    uint8_t u1WIP    : 1; /** Write-In-Progress */
    uint8_t u1WEL    : 1; /** Write/Program Enable Latch */
    uint8_t u3BP     : 3; /** Block Protection based Memory Array size selection */
    uint8_t u1E_ERR  : 1; /** Erasing Error Occurred */
    uint8_t u1P_ERR  : 1; /** Programming Error Occurred */
    uint8_t u1SRWD   : 1; /** Status Register Write Disable */
} stc_cy_smif_s25fs_STR1_t;

/** Status Register 1 union */
typedef union
{
    uint8_t                 u8;      /** < 8 bit access */
    stc_cy_smif_s25fs_STR1_t field; /** < \ref stc_cy_smif_s25fs_STR1_t */
} un_cy_smif_s25fs_STR1_t;


/***** Status Register 2 Utilities *****/
/** Status Register 2 bit field definition */
typedef struct
{
    uint8_t u1PS     : 1; /** Program Suspend  */
    uint8_t u1ES     : 1; /** Erase Suspend  */
    uint8_t u1ESTAT  : 1; /** Erase Success/Failure  */
    uint8_t u5RESRVD : 5; /** Reserved for future use */
} stc_cy_smif_s25fs_STR2_t;

/** Status Register 2 union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_STR2_t field; /** < \ref stc_cy_smif_s25fs_STR2_t */
} un_cy_smif_s25fs_STR2_t;

/***** Configuration Register 1 Utilities *****/
/** Configuration Register 1 bit field definition */
typedef struct
{
    uint8_t u1FREEZE  : 1; /** Lock-Down Block Protection until next power cycle */
    uint8_t u1QUAD    : 1; /** Quad SPI Interface Selection - I/O width set to 4 bits (1-1-4, 1-4-4) */
    uint8_t u1TBPRM   : 1; /** Top or Bottom Address Range selection for 4 KB Sector Block */
    uint8_t u1BPNV    : 1; /** Configures BP2-0 in Status Register */
    uint8_t u1RESRVD0 : 1; /** Reserved for future use */
    uint8_t u1TBPROT  : 1; /** Top or Bottom Protection selection for Protection Mode */
    uint8_t u2RESRVD1 : 2; /** Reserved for future use */
} stc_cy_smif_s25fs_CFR1_t;

/** Configuration Register 2 union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_CFR1_t field; /** < \ref stc_cy_smif_s25fs_CFR1_t */
} un_cy_smif_s25fs_CFR1_t;

/***** Configuration Register 2 Utilities *****/
/** Configuration Register 2 bit field definition */
typedef struct
{
    uint8_t u4RL     : 4; /** Read Latency selection - Dummy cycles required for initial data access */
    uint8_t u1RESRVD : 1; /** Reserved for future use */
    uint8_t u1IO3R   : 1; /** DQ3 and RESET Selection for DQ3 - Multiplexed operation on I/O #3 */
    uint8_t u1QA     : 1; /** QPI Interface & Protocol Selection - I/O width set to 4 bits (4-4-4) */
    uint8_t u1AL     : 1; /** Address Byte Length selection between 3 or 4 bytes for Instructions */
} stc_cy_smif_s25fs_CFR2_t;

/** Configuration Register 2 union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_CFR2_t field; /** < \ref stc_cy_smif_s25fs_CFR2_t */
} un_cy_smif_s25fs_CFR2_t;


/***** Configuration Register 3 Utilities *****/
/** Configuration Register 3 bit field definition */
typedef struct
{
    uint8_t u1F0h    : 1; /** Legacy Software Reset transaction F0h selection */
    uint8_t u1D8h    : 1; /** Block Erase Size */
    uint8_t u130h    : 1; /** Clear Status or Resume transaction 30h selection */
    uint8_t u120h    : 1; /** Uniform or Hybrid Sector Architecture selection */
    uint8_t u102h    : 1; /** Program Buffer Wrap Size selection */
    uint8_t u1BCh    : 1; /** Blank Check */
    uint8_t u2RESRVD : 2; /** Reserved for future use */
} stc_cy_smif_s25fs_CFR3_t;

typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_CFR3_t field; /** < \ref stc_cy_smif_s25fs_CFR3_t */
} un_cy_smif_s25fs_CFR3_t;

/***** Configuration Register 4 Utilities *****/
/** Configuration Register 4 bit field definition */
typedef struct
{
    uint8_t u2WL     : 2; /** Read Burst Wrap Length selection */
    uint8_t u2RESRVD : 2; /** Reserved for future use */
    uint8_t u1WE     : 1; /** Read Burst Wrap Enable selection */
    uint8_t u3OI     : 3; /** I/O Driver Output Impedance selection */
} stc_cy_smif_s25fs_CFR4_t;

/** Configuration Register 4 union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_CFR4_t field; /** < \ref stc_cy_smif_s25fs_CFR4_t */
} un_cy_smif_s25fs_CFR4_t;


/***** ECC Status Register Utilities *****/
/** ECC Status Register bit field definition */
typedef struct
{
    uint8_t u1ECCDI   : 1; /** ECC Disabled */
    uint8_t u1EECCD   : 1; /** Single Bit Error corrected in ECC unit data */
    uint8_t u1EECC    : 1; /** Single Bit Error found in the ECC unit error correction code */
    uint8_t u5RESRVD1 : 5; /** Reserved for Future Use */
} stc_cy_smif_s25fs_ECCSR_t;

/** ECC Status Register union */
typedef union
{
    uint8_t                   u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_ECCSR_t field; /** < \ref stc_cy_smif_s25fs_ECCSR_t */
} un_cy_smif_s25fs_ECCSR_t;


/***** Advanced Sector Protection Register Utilities *****/
/** Advanced Sector Protection Register bit field definition */
typedef struct
{
    uint8_t  u1RESRVD1 : 1; /** Reserved for Future Use */
    uint16_t u1PSTMLB  : 1; /** Persistent Protection Selection (Register Protection Selection) */
    uint16_t u1PWDMLB  : 1; /** Password Based Protection Selection */
    uint16_t u13RESRVD2: 13; /** Reserved for Future Use */
} stc_cy_smif_s25fs_ASPR_t;

/** Advanced Sector Protection Register union */
typedef union
{
    uint16_t                 u16;   /** < 16 bit access */
    stc_cy_smif_s25fs_ASPR_t field; /** < \ref stc_cy_smif_s25fs_ASPR_t */
} un_cy_smif_s25fs_ASPR_t;

/***** ASP Password Register Utilities *****/
/** ASP Password Register bit field definition */
typedef struct
{
    uint64_t u64ASPPRM  : 64; /** ECC 1-bit and 2-bit Error Count Register */
} stc_cy_smif_s25fs_PWDO_t;

/** ASP Password Register union */
typedef union
{
    uint64_t                 u64;   /** < 64 bit access */
    stc_cy_smif_s25fs_PWDO_t field; /** < \ref stc_cy_smif_s25fs_PWDO_t */
} un_cy_smif_s25fs_PWDO_t;

/***** ASP PPB Lock Register Utilities *****/
/** ASP PPB Lock Register bit field definition */
typedef struct
{
    uint8_t u1PPBLCK  : 1; /** PPB Temporary Protection Selection */
    uint8_t u7RESVRD  : 7; /** Reserved for Future Use */
} stc_cy_smif_s25fs_PPLV_t;

/** ASP PPB Lock Register union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_PPLV_t field; /** < \ref stc_cy_smif_s25fs_PPLV_t */
} un_cy_smif_s25fs_PPLV_t;

/***** ASP PPB Access Register Utilities *****/
/** ASP PPB Access Register bit field definition */
typedef struct
{
    uint8_t u8PPBACS  : 8; /** */
} stc_cy_smif_s25fs_PPAV_t;

/** ASP PPB Access Register union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_PPAV_t field; /** < \ref stc_cy_smif_s25fs_PPAV_t */
} un_cy_smif_s25fs_PPAV_t;

/***** ASP Dynamic Block Access Register Utilities *****/
/** ASP Dynamic Block Access Register bit field definition */
typedef struct
{
    uint8_t u8DYBACS  : 8; /** Sector Based DYB Protection Status */
} stc_cy_smif_s25fs_DYAV_t;

/** ASP Dynamic Block Access Register union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_DYAV_t field; /** < \ref stc_cy_smif_s25fs_DYAV_t */
} un_cy_smif_s25fs_DYAV_t;

/***** Data Learning Register Utilities *****/
/** Data Learning Register bit field definition */
typedef struct
{
    uint32_t u8DTLRPT  : 8; /** Data Learning Pattern Selection */
} stc_cy_smif_s25fs_DLPN_t;

/** Data Learning Register union */
typedef union
{
    uint8_t                  u8;    /** < 8 bit access */
    stc_cy_smif_s25fs_DLPN_t field; /** < \ref stc_cy_smif_s25fs_DLPN_t */
} un_cy_smif_s25fs_DLPN_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/** Specifies S25FS Register Address */
typedef enum
{
    /* Non-Volatile registers */
    CY_S25FS_REG_ADDR_STR1_NV       = 0x00000000UL, /** < Non-Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_CFR1_NV       = 0x00000002UL, /** < Non-Volatile Configuration Register 1 */
    CY_S25FS_REG_ADDR_CFR2_NV       = 0x00000003UL, /** < Non-Volatile Configuration Register 2 */
    CY_S25FS_REG_ADDR_CFR3_NV       = 0x00000004UL, /** < Non-Volatile Configuration Register 3 */
    CY_S25FS_REG_ADDR_CFR4_NV       = 0x00000005UL, /** < Non-Volatile Configuration Register 4 */
    CY_S25FS_REG_ADDR_DLP_NV        = 0x00000010UL, /** < Non-Volatile Data Learning Register */
    CY_S25FS_REG_ADDR_PWDO_BYTE0_NV = 0x00000020UL, /** < Non-Volatile ASP Password Register Byte 0 */
    CY_S25FS_REG_ADDR_PWDO_BYTE1_NV = 0x00000021UL, /** < Non-Volatile ASP Password Register Byte 1 */
    CY_S25FS_REG_ADDR_PWDO_BYTE2_NV = 0x00000022UL, /** < Non-Volatile ASP Password Register Byte 2 */
    CY_S25FS_REG_ADDR_PWDO_BYTE3_NV = 0x00000023UL, /** < Non-Volatile ASP Password Register Byte 3 */
    CY_S25FS_REG_ADDR_PWDO_BYTE4_NV = 0x00000024UL, /** < Non-Volatile ASP Password Register Byte 4 */
    CY_S25FS_REG_ADDR_PWDO_BYTE5_NV = 0x00000025UL, /** < Non-Volatile ASP Password Register Byte 5 */
    CY_S25FS_REG_ADDR_PWDO_BYTE6_NV = 0x00000026UL, /** < Non-Volatile ASP Password Register Byte 6 */
    CY_S25FS_REG_ADDR_PWDO_BYTE7_NV = 0x00000027UL, /** < Non-Volatile ASP Password Register Byte 7 */
    CY_S25FS_REG_ADDR_ASPO_BYTE0_NV = 0x00000030UL, /** < Non-Volatile Advanced Sector Protection Register Byte 0 */
    CY_S25FS_REG_ADDR_ASPO_BYTE1_NV = 0x00000031UL, /** < Non-Volatile Advanced Sector Protection Register Byte 1 */

    /* Volatile registers */
    CY_S25FS_REG_ADDR_STR1_V        = 0x00800000UL, /** < Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_STR2_V        = 0x00800001UL, /** < Volatile Status Register 2 */
    CY_S25FS_REG_ADDR_CFR1_V        = 0x00800002UL, /** < Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_CFR2_V        = 0x00800003UL, /** < Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_CFR3_V        = 0x00800004UL, /** < Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_CFR4_V        = 0x00800005UL, /** < Volatile Status Register 1 */
    CY_S25FS_REG_ADDR_DLP_V         = 0x00800010UL, /** < Volatile Data Learning Register */
    CY_S25FS_REG_ADDR_PPLV_V        = 0x00800040UL, /** < Volatile ASP PPB Lock Register */
} cy_en_smif_s25fs_reg_addr_t;

/****************************************/
/** S25FS Flash Register Utilities End */
/****************************************/

/** Specifies S25FS Flash Read command ID for this driver */
typedef enum
{
    CY_SMIF_S25FS_RDID_1S1S1S_3ADR      = 0u,  /**< Legacy SPI Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S1S1S_4ADR,            /**< Legacy SPI Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_1S1S1S_FAST_3ADR,       /**< Legacy SPI Fast Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S1S1S_FAST_4ADR,       /**< Legacy SPI Fast Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_1S2S2S_3ADR,            /**< Dual Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S2S2S_4ADR,            /**< Dual Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_2S2S_3ADR,              /**< Dual Continuous Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_2S2S_4ADR,              /**< Dual Continuous Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_1S1S4S_3ADR,            /**< Quad Output Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S1S4S_4ADR,            /**< Quad Output Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_1S4S4S_3ADR,            /**< Quad I/O Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S4S4S_4ADR,            /**< Quad I/O Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_1S4D4D_3ADR,            /**< Quad I/O Read DDR (3-byte address) */
    CY_SMIF_S25FS_RDID_1S4D4D_4ADR,            /**< Quad I/O Read DDR (4-byte address) */
    CY_SMIF_S25FS_RDID_4S4S4S_3ADR,            /**< Quad Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_4S4S4S_4ADR,            /**< Quad Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_4S4S_3ADR,              /**< Quad Continuous Read SDR (3-byte address) */
    CY_SMIF_S25FS_RDID_4S4S_4ADR,              /**< Quad Continuous Read SDR (4-byte address) */
    CY_SMIF_S25FS_RDID_4D4D4D_3ADR,            /**< Quad Read DDR (3-byte address) */
    CY_SMIF_S25FS_RDID_4D4D4D_4ADR,            /**< Quad Read DDR (4-byte address) */
    CY_SMIF_S25FS_RDID_4D4D_3ADR,              /**< Quad Continuous Read DDR (3-byte address) */
    CY_SMIF_S25FS_RDID_4D4D_4ADR,              /**< Quad Continuous Read DDR (4-byte address) */
    CY_SMIF_S25FS_READ_ID_NUM,
} cy_en_smif_s25fs_rd_cmdid_t;

/** Specifies S25FS Flash Write command ID for this driver */
typedef enum
{
    CY_SMIF_S25FS_WTID_1S1S1S_3ADR   = 0u, /**< Legacy SPI mode Page Program (3-byte address) */
    CY_SMIF_S25FS_WTID_1S1S1S_4ADR,        /**< Legacy SPI mode Page Program (4-byte address) */
    CY_SMIF_S25FS_WTID_4S4S4S_3ADR,        /**< SDR Quad mode Page Program (3-byte address) */
    CY_SMIF_S25FS_WTID_4S4S4S_4ADR,        /**< SDR Quad mode Page Program (4-byte address) */
    CY_SMIF_S25FS_WRITE_ID_NUM,
} cy_en_smif_s25fs_wt_cmdid_t;

/** Specifies S25FS flash memory read latency code */
typedef enum
{
    CY_SMIF_S25FS_RD_LATENCY0  = 0u,  /**< Latency code 0 */
    CY_SMIF_S25FS_RD_LATENCY1  = 1u,  /**< Latency code 1 */
    CY_SMIF_S25FS_RD_LATENCY2  = 2u,  /**< Latency code 2 */
    CY_SMIF_S25FS_RD_LATENCY3  = 3u,  /**< Latency code 3 */
    CY_SMIF_S25FS_RD_LATENCY4  = 4u,  /**< Latency code 4 */
    CY_SMIF_S25FS_RD_LATENCY5  = 5u,  /**< Latency code 5 */
    CY_SMIF_S25FS_RD_LATENCY6  = 6u,  /**< Latency code 6 */
    CY_SMIF_S25FS_RD_LATENCY7  = 7u,  /**< Latency code 7 */
    CY_SMIF_S25FS_RD_LATENCY8  = 8u,  /**< Latency code 8 */
    CY_SMIF_S25FS_RD_LATENCY9  = 9u,  /**< Latency code 9 */
    CY_SMIF_S25FS_RD_LATENCY10 = 10u, /**< Latency code 10 */
    CY_SMIF_S25FS_RD_LATENCY11 = 11u, /**< Latency code 11 */
    CY_SMIF_S25FS_RD_LATENCY12 = 12u, /**< Latency code 12 */
    CY_SMIF_S25FS_RD_LATENCY13 = 13u, /**< Latency code 13 */
    CY_SMIF_S25FS_RD_LATENCY14 = 14u, /**< Latency code 14 */
    CY_SMIF_S25FS_RD_LATENCY15 = 15u, /**< Latency code 15 */
} cy_en_smif_s25fs_read_latency_code_t;
#define CY_SMIF_S25FS_READ_LATENCY_CODE_MAX (16u)
#define CY_SMIF_S25FS_READ_LC_DC_TABLE_MAX  (1u)

/** Specifies S25FS flash memory write latency code */
typedef enum
{
    CY_SMIF_S25FS_WT_LATENCY0 = 0u,  /**< Latency code 0 */
} cy_en_smif_s25fs_write_latency_code_t;
#define CY_SMIF_S25FS_WRITE_LATENCY_CODE_MAX (1u)
#define CY_SMIF_S25FS_WRITE_LC_DC_TABLE_MAX  (1u)

/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Specifies S25FS Manufacturer and Device ID structure */
typedef struct
{
    uint8_t manufacturerId;      /**< Manufacturer ID for Cypress */
    uint8_t DeviceId_MSB;        /**< Device ID MSB - Memory Interface Type */
    uint8_t DeviceId_LSB;        /**< Device ID LSB - Density */
    uint8_t idLength;            /**< ID Length - number bytes following. */
    uint8_t physicalSecotorArch; /**< Physical Sector Architecture */
    uint8_t familyId;            /**< Family ID */
} cy_stc_smif_s25fs_id_t;
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
    Cy_SMIF_S25FS_MODE_CONTINUOUS     = 0xA5, 
    Cy_SMIF_S25FS_MODE_NOT_CONTINUOUS = 0x00,
} cy_en_smif_s25fs_mode_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_functions
* \{
*/
cy_en_smif_status_t Cy_SMIF_S25FS_ReadId(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    cy_stc_smif_s25fs_id_t*              readID,
    cy_en_smif_trx_type_t                transaction,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadJedecId(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint64_t*                 readData[],
    uint32_t                  readAddress,
    uint32_t                  readSize,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadStatusRegister1(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             readData,
    cy_en_smif_trx_type_t                transaction,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadStatusRegister2(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             readData,
    cy_en_smif_trx_type_t                transaction,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadAnyRegister(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             readData,
    cy_en_smif_s25fs_reg_addr_t          regAddr,
    cy_en_smif_s25fs_read_latency_code_t lc,
    cy_en_smif_trx_addr_len_t            addrLen,
    cy_en_smif_trx_type_t                trx,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_WriteEnable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_WriteDisable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_WriteAnyRegister(
    volatile cy_stc_smif_reg_t*        base,
    cy_en_smif_slave_select_t   slave,
    uint8_t                     writeData,
    cy_en_smif_s25fs_reg_addr_t regAddr,
    cy_en_smif_trx_addr_len_t   addrLen,
    cy_en_smif_trx_type_t       transaction,
    cy_stc_smif_context_t*      context);

cy_en_smif_status_t Cy_SMIF_S25FS_ClearProgramAndEraseFailureFlags(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadECC_DataUnitStatus(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             readStatus,
    uint32_t                             unitAdder,
    cy_en_smif_s25fs_read_latency_code_t lc,
    cy_en_smif_trx_type_t                transaction,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_ClearECC_StatusRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Erase_4KB_Sector(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Erase_256KB_Sector(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint32_t                  erasedAddress,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_EraseChip(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_SuspendEraseProgram(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_ResumeEraseProgram(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_SoftwareResetEnable(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_SoftwareReset(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_EnterDeepPowerDownMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     transaction,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_ReadConfigurationRegister1(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             readData,
    cy_en_smif_trx_type_t                trx,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_WriteRegister(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   stsReg,
    uint8_t                   cfgReg1,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Enter4ByteAddressMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Exit4ByteAddressMode(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Read_DLP_Register(
    volatile cy_stc_smif_reg_t*                 base,
    cy_en_smif_slave_select_t            slave,
    uint8_t*                             dlpData,
    cy_en_smif_s25fs_read_latency_code_t regLc,
    cy_en_smif_trx_type_t                trx,
    cy_stc_smif_context_t*               context);

cy_en_smif_status_t Cy_SMIF_S25FS_Program_DLP_NV_Register(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   dlpData,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

cy_en_smif_status_t Cy_SMIF_S25FS_Write_DLP_V_Register(
    volatile cy_stc_smif_reg_t*      base,
    cy_en_smif_slave_select_t slave,
    uint8_t                   dlpData,
    cy_en_smif_trx_type_t     trx,
    cy_stc_smif_context_t*    context);

/** \} group_mw_smif_functions */

extern const cy_stc_cmd_sq_t         g_s25fsReadCmdTypes[];
extern const cy_stc_addr_sq_t        g_s25fsReadAddrTypes[];
extern const cy_stc_mode_sq_t        g_s25fsReadModeTypes[];
extern const cy_stc_dummy_sq_t       g_s25fsReadDummyTypes[];
extern const uint8_t*                g_s25fsReadLatencyCodeToDummyCycle[CY_SMIF_S25FS_READ_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_s25fsReadDataTypes[];
extern const cy_stc_read_info_t      g_s25fsReadCmdInfo[];
extern const cy_stc_cmd_sq_t         g_s25fsWriteCmdTypes[];
extern const cy_stc_addr_sq_t        g_s25fsWriteAddrTypes[];
extern const cy_stc_mode_sq_t        g_s25fsWriteModeTypes[];
extern const cy_stc_dummy_sq_t       g_s25fsWriteDummyTypes[];
extern const uint8_t*                g_s25fsWriteLatencyCodeToDummyCycle[CY_SMIF_S25FS_WRITE_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_s25fsWriteDataTypes[];
extern const cy_stc_write_info_t     g_s25fsWriteCmdInfo[];


#if defined(__cplusplus)
}
#endif

#endif /* (CY_SMIF_S25FS_H) */

