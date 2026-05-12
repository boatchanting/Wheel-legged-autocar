/***************************************************************************//**
* \file cy_smif_s25fl.c
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress S25FL HSSPI series driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(CY_SMIF_S25FL_H)
#define CY_SMIF_S25FL_H

#if defined(__cplusplus)
extern "C" {
#endif

/********************* divider configuration ***********************/

#define S25FL_USE_LC_ENHANCED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "cy_project.h"
#include "cy_smif_device_common.h"

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#ifndef CY_SMIF_S25FXXXS
  #define CY_SMIF_S25FXXXS (0)
#endif

#ifndef CY_SMIF_S25FXXXL
  #define CY_SMIF_S25FXXXL (1)
#endif

#ifndef CY_SMIF0_QUAD_MEMORY
  #define CY_SMIF0_QUAD_MEMORY CY_SMIF_S25FXXXS
#endif

#define CY_SMIF_QUAD_DEVICE_TYPE CY_SMIF0_QUAD_MEMORY  // this is defined in board header file.
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
typedef enum
{
    CY_SMIF_S25FL_CMD_READ_ID    = 0x90, // Read Electronic Manufacturer Signature
    CY_SMIF_S25FL_CMD_RDID       = 0x9F, // Read ID (JEDEC Manufacturer ID and JEDEC CFI)
    CY_SMIF_S25FL_CMD_RES        = 0xAB, // Read Electronic Signature 
    CY_SMIF_S25FL_CMD_RDSR1      = 0x05, // Read Status Register-1 
    CY_SMIF_S25FL_CMD_RDSR2      = 0x07, // Read Status Register-2 
    CY_SMIF_S25FL_CMD_RDCR1      = 0x35, // Read Configuration Register-1 
    CY_SMIF_S25FL_CMD_WRR        = 0x01, // Write Register (Status-1, Configuration-1) 
    CY_SMIF_S25FL_CMD_WRDI       = 0x04, // Write Disable 
    CY_SMIF_S25FL_CMD_WREN       = 0x06, // Write Enable 
    CY_SMIF_S25FL_CMD_CLSR       = 0x30, // Clear Status Register-1 - Erase/Prog. Fail Reset 
    CY_SMIF_S25FL_CMD_ABRD       = 0x14, // AutoBoot Register Read 
    CY_SMIF_S25FL_CMD_ABWR       = 0x15, // AutoBoot Register Write 
    CY_SMIF_S25FL_CMD_BRRD       = 0x16, // Bank Register Read 
    CY_SMIF_S25FL_CMD_BRWR       = 0x17, // Bank Register Write 
    CY_SMIF_S25FL_CMD_BRAC       = 0xB9, // Bank Register Access (Legacy Command formerly used for Deep Power Down)
    CY_SMIF_S25FL_CMD_DLPRD      = 0x41, // Data Learning Pattern Read 
    CY_SMIF_S25FL_CMD_PDLRNV     = 0x43, // Program NV Data Learning Register 
    CY_SMIF_S25FL_CMD_WDLRV      = 0x4A, // Write Volatile Data Learning Register 
    CY_SMIF_S25FL_CMD_READ       = 0x03, // Read (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4READ      = 0x13, // Read (4-byte address) 
    CY_SMIF_S25FL_CMD_FAST_READ  = 0x0B, // Fast Read (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4FAST_READ = 0x0C, // Fast Read (4-byte address) 
    CY_SMIF_S25FL_CMD_DDRFR      = 0x0D, // DDR Fast Read (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4DDRFR     = 0x0E, // DDR Fast Read (4-byte address) 
    CY_SMIF_S25FL_CMD_DOR        = 0x3B, // Read Dual Out (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4DOR       = 0x3C, // Read Dual Out (4-byte address) 
    CY_SMIF_S25FL_CMD_QOR        = 0x6B, // Read Quad Out (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4QOR       = 0x6C, // Read Quad Out (4-byte address) 
    CY_SMIF_S25FL_CMD_DIOR       = 0xBB, // Dual I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4DIOR      = 0xBC, // Dual I/O Read (4-byte address) 
    CY_SMIF_S25FL_CMD_DDRDIOR    = 0xBD, // DDR Dual I/O Read (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4DDRDIOR   = 0xBE, // DDR Dual I/O Read (4-byte address) 
    CY_SMIF_S25FL_CMD_QIOR       = 0xEB, // Quad I/O Read (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4QIOR      = 0xEC, // Quad I/O Read (4-byte address) 
    CY_SMIF_S25FL_CMD_DDRQIOR    = 0xED, // DDR Quad I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4DDRQIOR   = 0xEE, // DDR Quad I/O Read (4-byte address) 
    CY_SMIF_S25FL_CMD_PP         = 0x02, // Page Program (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_4PP        = 0x12, // Page Program (4-byte address) 
    CY_SMIF_S25FL_CMD_QPP        = 0x32, // Quad Page Program (3- or 4-byte address) 
    CY_SMIF_S25FL_CMD_QPPA       = 0x38, // Quad Page Program - Alternate instruction (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4QPP       = 0x34, // Quad Page Program (4-byte address) 
    CY_SMIF_S25FL_CMD_PGSP       = 0x85, // Program Suspend 
    CY_SMIF_S25FL_CMD_PGRS       = 0x8A, // Program Resume 
    CY_SMIF_S25FL_CMD_P4E        = 0x20, // Parameter 4-kB, sector Erase (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4P4E       = 0x21, // Parameter 4-kB, sector Erase (4-byte address)
    CY_SMIF_S25FL_CMD_CE         = 0x60, // Chip Erase 
    CY_SMIF_S25FL_CMD_CEA        = 0xC7, // Chip Erase (alternate command)
    CY_SMIF_S25FL_CMD_SE         = 0xD8, // Erase 64 kB or 256 kB (3- or 4-byte address)
    CY_SMIF_S25FL_CMD_4SE        = 0xDC, // Erase 64 kB or 256 kB (4-byte address) 
    CY_SMIF_S25FL_CMD_ERSP       = 0x75, // Erase Suspend 
    CY_SMIF_S25FL_CMD_ERRS       = 0x7A, // Erase Resume 
    CY_SMIF_S25FL_CMD_OTPP       = 0x42, // OTP Program 
    CY_SMIF_S25FL_CMD_OTPR       = 0x4B, // OTP Read 
    CY_SMIF_S25FL_CMD_DYBRD      = 0xE0, // DYB Read 
    CY_SMIF_S25FL_CMD_DYBWR      = 0xE1, // DYB Write 
    CY_SMIF_S25FL_CMD_PPBRD      = 0xE2, // PPB Read 
    CY_SMIF_S25FL_CMD_PPBP       = 0xE3, // PPB Program 
    CY_SMIF_S25FL_CMD_PPBE       = 0xE4, // PPB Erase 
    CY_SMIF_S25FL_CMD_ASPRD      = 0x2B, // ASP Read 
    CY_SMIF_S25FL_CMD_ASPP       = 0x2F, // ASP Program 
    CY_SMIF_S25FL_CMD_PLBRD      = 0xA7, // PPB Lock Bit Read 
    CY_SMIF_S25FL_CMD_PLBWR      = 0xA6, // PPB Lock Bit Write 
    CY_SMIF_S25FL_CMD_PASSRD     = 0xE7, // Password Read 
    CY_SMIF_S25FL_CMD_PASSP      = 0xE8, // Password Program 
    CY_SMIF_S25FL_CMD_PASSU      = 0xE9, // Password Unlock 
    CY_SMIF_S25FL_CMD_RST        = 0xF0, // Software Reset 
    CY_SMIF_S25FL_CMD_MBR        = 0xFF, // Mode Bit Reset 
    CY_SMIF_S25FL_CMD_MPM        = 0xA3, // Reserved for Multi-I/O-High Perf Mode (MPM) 
} en_cy_smif_s25fl_cmd_t;
#elif (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
typedef enum
{
    CY_SMIF_S25FL_CMD_RDID       = 0x9F, // Read ID (JEDEC Manufacturer ID)
    CY_SMIF_S25FL_CMD_RSFDP      = 0x5A, // Read JEDEC Serial Flash Discoverable Parameters
    CY_SMIF_S25FL_CMD_RDQID      = 0xAF, // Read Quad ID
    CY_SMIF_S25FL_CMD_RUID       = 0x4B, // Read Unique ID
    CY_SMIF_S25FL_CMD_RDSR1      = 0x05, // Read Status Register 1
    CY_SMIF_S25FL_CMD_RDSR2      = 0x07, // Read Status Register 2
    CY_SMIF_S25FL_CMD_RDCR1      = 0x35, // Read Configuration Register 1
    CY_SMIF_S25FL_CMD_RDCR2      = 0x15, // Read Configuration Register 2
    CY_SMIF_S25FL_CMD_RDCR3      = 0x33, // Read Configuration Register 3
    CY_SMIF_S25FL_CMD_RDAR       = 0x65, // Read Any Register
    CY_SMIF_S25FL_CMD_WRR        = 0x01, // Write Register (Status-1 and Configuration-1,2,3)
    CY_SMIF_S25FL_CMD_WRDI       = 0x04, // Write Disable
    CY_SMIF_S25FL_CMD_WREN       = 0x06, // Write Enable for Non-volatile data change
    CY_SMIF_S25FL_CMD_WRENV      = 0x50, // Write Enable for Volatile Status and Configuration Registers
    CY_SMIF_S25FL_CMD_WRAR       = 0x71, // Write Any Register
    CY_SMIF_S25FL_CMD_CLSR       = 0x30, // Clear Status Register
    CY_SMIF_S25FL_CMD_4BEN       = 0xB7, // Enter 4 Byte Address Mode
    CY_SMIF_S25FL_CMD_4BEX       = 0xE9, // Exit 4 Byte Address Mode
    CY_SMIF_S25FL_CMD_SBL        = 0x77, // Set Burst Length
    CY_SMIF_S25FL_CMD_QPIEN      = 0x38, // Enter QPI
    CY_SMIF_S25FL_CMD_QPIEX      = 0xF5, // Exit QPI
    CY_SMIF_S25FL_CMD_DLPRD      = 0x41, // Data Learning Pattern Read
    CY_SMIF_S25FL_CMD_PDLRNV     = 0x43, // Program NV Data Learning Register
    CY_SMIF_S25FL_CMD_WDLRV      = 0x4A, // Write Volatile Data Learning Register
    CY_SMIF_S25FL_CMD_READ       = 0x03, // Read
    CY_SMIF_S25FL_CMD_4READ      = 0x13, // Read
    CY_SMIF_S25FL_CMD_FAST_READ  = 0x0B, // Fast Read
    CY_SMIF_S25FL_CMD_4FAST_READ = 0x0C, // Fast Read
    CY_SMIF_S25FL_CMD_DOR        = 0x3B, // Dual Output Read
    CY_SMIF_S25FL_CMD_4DOR       = 0x3C, // Dual Output Read
    CY_SMIF_S25FL_CMD_QOR        = 0x6B, // Quad Output Read
    CY_SMIF_S25FL_CMD_4QOR       = 0x6C, // Quad Output Read
    CY_SMIF_S25FL_CMD_DIOR       = 0xBB, // Dual I/O Read
    CY_SMIF_S25FL_CMD_4DIOR      = 0xBC, // Dual I/O Read
    CY_SMIF_S25FL_CMD_QIOR       = 0xEB, // Quad I/O Read (CR1V[1]=1 or CR2V[3]=1)
    CY_SMIF_S25FL_CMD_4QIOR      = 0xEC, // Quad I/O Read (CR1V[1]=1 or CR2V[3]=1)
    CY_SMIF_S25FL_CMD_DDRQIOR    = 0xED, // DDR Quad I/O Read (CR1V[1]=1 or CR2V[3]=1)
    CY_SMIF_S25FL_CMD_4DDRQIOR   = 0xEE, // DDR Quad I/O Read (CR1V[1]=1 or CR2V[3]=1)
    CY_SMIF_S25FL_CMD_PP         = 0x02, // Page Program
    CY_SMIF_S25FL_CMD_4PP        = 0x12, // Page Program
    CY_SMIF_S25FL_CMD_QPP        = 0x32, // Quad Page Program
    CY_SMIF_S25FL_CMD_4QPP       = 0x34, // Quad Page Program
    CY_SMIF_S25FL_CMD_SE         = 0x20, // Sector Erase
    CY_SMIF_S25FL_CMD_4SE        = 0x21, // Sector Erase
    CY_SMIF_S25FL_CMD_HBE        = 0x52, // Half Block Erase
    CY_SMIF_S25FL_CMD_4HBE       = 0x53, // Half Block Erase
    CY_SMIF_S25FL_CMD_BE         = 0xD8, // Block Erase
    CY_SMIF_S25FL_CMD_4BE        = 0xDC, // Block Erase
    CY_SMIF_S25FL_CMD_CE         = 0x60, // Chip Erase
    CY_SMIF_S25FL_CMD_CEA        = 0xC7, // Chip Erase (alternate command)
    CY_SMIF_S25FL_CMD_EPS        = 0x75, // Erase / Program Suspend
    CY_SMIF_S25FL_CMD_EPR        = 0x7A, // Erase / Program Resume
    CY_SMIF_S25FL_CMD_SECRE      = 0x44, // Security Region Erase
    CY_SMIF_S25FL_CMD_SECRP      = 0x42, // Security Region Program
    CY_SMIF_S25FL_CMD_SECRR      = 0x48, // Security Region Read
    CY_SMIF_S25FL_CMD_IBLRD      = 0x3D, // IBL Read
    CY_SMIF_S25FL_CMD_4IBLRD     = 0xE0, // IBL Read
    CY_SMIF_S25FL_CMD_IBL        = 0x36, // IBL Lock
    CY_SMIF_S25FL_CMD_4IBL       = 0xE1, // IBL Lock
    CY_SMIF_S25FL_CMD_IBUL       = 0x39, // IBL Unlock
    CY_SMIF_S25FL_CMD_4IBUL      = 0xE2, // IBL Unlock
    CY_SMIF_S25FL_CMD_GBL        = 0x7E, // Global IBL Lock0
    CY_SMIF_S25FL_CMD_GBUL       = 0x98, // Global IBL Unlock
    CY_SMIF_S25FL_CMD_SPRP       = 0xFB, // Set Pointer Region Protection
    CY_SMIF_S25FL_CMD_4SPRP      = 0xE3, // Set Pointer Region Protection
    CY_SMIF_S25FL_CMD_IRPRD      = 0x2B, // IRP Register Read
    CY_SMIF_S25FL_CMD_IRPP       = 0x2F, // IRP Register Program
    CY_SMIF_S25FL_CMD_PRRD       = 0xA7, // Protection Register Read
    CY_SMIF_S25FL_CMD_PRL        = 0xA6, // Protection Register Lock (NVLOCK Bit Write)
    CY_SMIF_S25FL_CMD_PASSRD     = 0xE7, // Password Read
    CY_SMIF_S25FL_CMD_PASSP      = 0xE8, // Password Program
    CY_SMIF_S25FL_CMD_PASSU      = 0xEA, // Password Unlock
    CY_SMIF_S25FL_CMD_RSTEN      = 0x66, // Software Reset Enable
    CY_SMIF_S25FL_CMD_RST        = 0x99, // Software Reset
    CY_SMIF_S25FL_CMD_MBR        = 0xFF, // Mode Bit Reset
    CY_SMIF_S25FL_CMD_DPD        = 0xB9, // Deep Power Down
    CY_SMIF_S25FL_CMD_RES        = 0xAB, // Release from Deep Power Down / Device Id
} en_cy_smif_s25fl_cmd_t;
#else
  #error "device not supported"
#endif
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
typedef struct
{
    uint8_t u1WIP   : 1;
    uint8_t u1WEL   : 1;
    uint8_t u3BP    : 3;
    uint8_t u1E_ERR : 1;
    uint8_t u1P_ERR : 1;
    uint8_t u1SRWD  : 1;
} stc_cy_smif_s25fl128_SR1_t;

typedef struct
{
    uint8_t u1PS  : 1;
    uint8_t u1ES  : 1;
    uint8_t u6RFU : 6;
} stc_cy_smif_s25fl128_SR2_t;

typedef struct
{
    uint8_t u1FREEZE : 1;
    uint8_t u1QUAD   : 1;
    uint8_t u1TBPARM : 1;
    uint8_t u1BPNV   : 1;
    uint8_t u1RFU    : 1;
    uint8_t u1TBPROT : 1;
    uint8_t u2LC     : 2;
} stc_cy_smif_s25fl128_CR1_t;

typedef struct
{
    uint8_t u1BA24   : 1;
    uint8_t u6RFU    : 6;
    uint8_t u1EXTADD : 1;
} stc_cy_smif_s25fl128_BAR_t;

typedef struct
{
    uint32_t u1ABE   : 1;
    uint32_t u8ABSD  : 8;
    uint32_t u23ABSA : 23;
} stc_cy_smif_s25fl128_ABR_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_SR1_t field;
} un_cy_smif_s25fl128_SR1_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_SR2_t field;
} un_cy_smif_s25fl128_SR2_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_BAR_t field;
} un_cy_smif_s25fl128_BAR_t;


typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_CR1_t field;
} un_cy_smif_s25fl128_CR1_t;

typedef union
{
    uint32_t                     u32;
    stc_cy_smif_s25fl128_ABR_t field;
} un_cy_smif_s25fl128_ABR_t;

#elif (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
typedef struct
{
    uint8_t u1WIP   : 1;
    uint8_t u1WEL   : 1;
    uint8_t u3BP    : 3;
    uint8_t u1TBPROT: 1;
    uint8_t u1SEC   : 1;
    uint8_t u1SRP0  : 1;
} stc_cy_smif_s25fl128_SR1_t;

typedef struct
{
    uint8_t u1PS    : 1;
    uint8_t u1ES    : 1;
    uint8_t u3RFU   : 3;
    uint8_t u1P_ERR : 1;
    uint8_t u1E_ERR : 1;
    uint8_t u1RFU   : 1;
} stc_cy_smif_s25fl128_SR2_t;

typedef struct
{
    uint8_t u1SRP1 : 1;
    uint8_t u1QUAD : 1;
    uint8_t u1LB0  : 1;
    uint8_t u1LB1  : 1;
    uint8_t u1LB2  : 1;
    uint8_t u1LB3  : 1;
    uint8_t u1CMP  : 1;
    uint8_t u1SUS  : 1;
} stc_cy_smif_s25fl128_CR1_t;

typedef struct
{
    uint8_t u1ADS  : 1;
    uint8_t u1ADP  : 1;
    uint8_t u1WPS  : 1;
    uint8_t u1QPI  : 1;
    uint8_t u1RFU  : 1;
    uint8_t u2OI   : 2;
    uint8_t u1IO3R : 1;
} stc_cy_smif_s25fl128_CR2_t;

typedef struct
{
    uint8_t u4RL  : 4;
    uint8_t u1WE  : 1;
    uint8_t u2WR  : 2;
    uint8_t u1RFU : 1;
} stc_cy_smif_s25fl128_CR3_t;


typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_SR1_t field;
} un_cy_smif_s25fl128_SR1_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_SR2_t field;
} un_cy_smif_s25fl128_SR2_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_CR1_t field;
} un_cy_smif_s25fl128_CR1_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_CR2_t field;
} un_cy_smif_s25fl128_CR2_t;

typedef union
{
    uint8_t                    u8;
    stc_cy_smif_s25fl128_CR3_t field;
} un_cy_smif_s25fl128_CR3_t;
#endif

typedef struct
{
    uint8_t stsReg1;
    uint8_t confgReg1;
    uint8_t confgReg2;
    uint8_t confgReg3;
} stc_cy_smif_s25f_write_reg_t;
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
    CY_SMIF_S25FL_RW_CONTINUOUS     = 0xA5, 
    CY_SMIF_S25FL_RW_NON_CONTINUOUS = 0x00,
} cy_en_smif_rw_s25fl_mode_t;

typedef enum
{
    CY_SMIF_S25FL_ID_READ          = 0u,  // Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4READ         = 1u,  // Read (4-byte address) 
    CY_SMIF_S25FL_ID_FAST_READ     = 2u,  // Fast Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4FAST_READ    = 3u,  // Fast Read (4-byte address) 
    CY_SMIF_S25FL_ID_DOR           = 4u,  // Read Dual Out (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4DOR          = 5u,  // Read Dual Out (4-byte address) 
    CY_SMIF_S25FL_ID_QOR           = 6u,  // Read Quad Out (3- or 4-byte address)
    CY_SMIF_S25FL_ID_4QOR          = 7u,  // Read Quad Out (4-byte address) 
    CY_SMIF_S25FL_ID_DIOR          = 8u,  // Dual I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_DIOR_CONT     = 9u,  // Dual I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4DIOR         = 10u, // Dual I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_4DIOR_CONT    = 11u, // Dual I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_QIOR          = 12u, // Quad I/O Read (3- or 4-byte address)
    CY_SMIF_S25FL_ID_QIOR_CONT     = 13u, // Quad I/O Read (3- or 4-byte address)
    CY_SMIF_S25FL_ID_4QIOR         = 14u, // Quad I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_4QIOR_CONT    = 15u, // Quad I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_DDRQIOR       = 16u, // DDR Quad I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_DDRQIOR_CONT  = 17u, // DDR Quad I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4DDRQIOR      = 18u, // DDR Quad I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_4DDRQIOR_CONT = 19u, // DDR Quad I/O Read (4-byte address) 
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    CY_SMIF_S25FL_ID_DDRFR         = 20u, // DDR Fast Read (3- or 4-byte address)
    CY_SMIF_S25FL_ID_DDRFR_CONT    = 21u, // DDR Fast Read (3- or 4-byte address)
    CY_SMIF_S25FL_ID_4DDRFR        = 22u, // DDR Fast Read (4-byte address) 
    CY_SMIF_S25FL_ID_4DDRFR_CONT   = 23u, // DDR Fast Read (4-byte address) 
    CY_SMIF_S25FL_ID_DDRDIOR       = 24u, // DDR Dual I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_DDRDIOR_CONT  = 25u, // DDR Dual I/O Read (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4DDRDIOR      = 26u, // DDR Dual I/O Read (4-byte address) 
    CY_SMIF_S25FL_ID_4DDRDIOR_CONT = 27u, // DDR Dual I/O Read (4-byte address) 
#endif
    CY_SMIF_S25FL_READ_ID_NUM,
} cy_en_smif_s25fl_rd_cmdid_t;

typedef enum
{
    CY_SMIF_S25FL_ID_PP   = 0u, // Page Program (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4PP  = 1u, // Page Program (4-byte address) 
    CY_SMIF_S25FL_ID_QPP  = 2u, // Quad Page Program (3- or 4-byte address) 
    CY_SMIF_S25FL_ID_4QPP = 3u, // Quad Page Program (4-byte address) 
#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
    CY_SMIF_S25FL_ID_QPPA = 4u, // Quad Page Program - Alternate instruction (3- or 4-byte address)
#endif
    CY_SMIF_S25FL_WRITE_ID_NUM,
} cy_en_smif_s25fl_wt_cmdid_t;


#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
typedef enum
{
    CY_SMIF_S25FL_RD_LATENCY0 = 3u,
    CY_SMIF_S25FL_RD_LATENCY1 = 0u,
    CY_SMIF_S25FL_RD_LATENCY2 = 1u,
    CY_SMIF_S25FL_RD_LATENCY3 = 2u,
} cy_en_smif_s25fl_latency_code_t;
#define CY_SMIF_S25FL_READ_LC_DC_TABLE_MAX  (9)
#define CY_SMIF_S25FL_READ_LATENCY_CODE_MAX (4)

#elif(CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
typedef enum
{
    CY_SMIF_S25FL_RD_LATENCY0  = 0u,
    CY_SMIF_S25FL_RD_LATENCY1  = 1u,
    CY_SMIF_S25FL_RD_LATENCY2  = 2u,
    CY_SMIF_S25FL_RD_LATENCY3  = 3u,
    CY_SMIF_S25FL_RD_LATENCY4  = 4u,
    CY_SMIF_S25FL_RD_LATENCY5  = 5u,
    CY_SMIF_S25FL_RD_LATENCY6  = 6u,
    CY_SMIF_S25FL_RD_LATENCY7  = 7u,
    CY_SMIF_S25FL_RD_LATENCY8  = 8u,
    CY_SMIF_S25FL_RD_LATENCY9  = 9u,
    CY_SMIF_S25FL_RD_LATENCY10 = 10u,
    CY_SMIF_S25FL_RD_LATENCY11 = 11u,
    CY_SMIF_S25FL_RD_LATENCY12 = 12u,
    CY_SMIF_S25FL_RD_LATENCY13 = 13u,
    CY_SMIF_S25FL_RD_LATENCY14 = 14u,
    CY_SMIF_S25FL_RD_LATENCY15 = 15u,
} cy_en_smif_s25fl_read_latency_code_t;
#define CY_SMIF_S25FL_READ_LC_DC_TABLE_MAX  (2)
#define CY_SMIF_S25FL_READ_LATENCY_CODE_MAX (16)
#endif

typedef enum
{
    CY_SMIF_S25FL_WT_LATENCY0 = 0u,
} cy_en_smif_s25fl_write_latency_code_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SMIF_S25FL_WRITE_LC_DC_TABLE_MAX  (1)
#define CY_SMIF_S25FL_WRITE_LATENCY_CODE_MAX (1)
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_functions
* \{
*/
cy_en_smif_status_t Cy_SMIF_S25FL_ReadJedecId(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_JedecId_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadStatusRegister1(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadStatusRegister2(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteStatusAndConfigRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        stc_cy_smif_s25f_write_reg_t* writeRegs,
                                        uint8_t writeRegNum,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteDisable(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteEnable(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ClearStatusRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_DataLeaningPatternRead(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t *readDLP,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t 
Cy_SMIF_S25FL_ProgramNVDLR(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t writeData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteVDLR(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t writeData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_SectorErase(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t erasedAddress,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ChipErase(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordRead(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint64_t* readPW,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordProgram(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint64_t  programPW,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PasswordUnlock(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint64_t  inputPW,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_SoftwareReset(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ModeBitReset(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);

#if (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXS)
cy_en_smif_status_t Cy_SMIF_S25FL_ReadId(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readManufactureID,
                                        uint8_t* readDeviceID,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadElectronicSignature(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadBankRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteBankRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t writeData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_AccessBankRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t BarBit0,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteStatusRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t writeData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ReadAutoBootRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_WriteAutoBootRegister(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t writeData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramSuspend(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramResume(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_4KB_SectorErase(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t erasedAddress,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_EraseSuspend(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_EraseResume(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_OPT_Program(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t programedAddress,
                                        uint32_t size,
                                        uint8_t buf[],
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_OPT_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t readAddress,
                                        uint32_t size,
                                        uint8_t buf[],
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ASP_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t* readValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ASP_Program(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t* programedValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_DYB_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t readAddress,
                                        uint8_t* readValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_DYB_Write(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t writeAddress,
                                        uint8_t  writeValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t readAddress,
                                        uint8_t* readValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Program(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint32_t programedAddress,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_Erase(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_LockBitRead(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readValue,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_PPB_LockBitWrite(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);

#elif (CY_SMIF_QUAD_DEVICE_TYPE == CY_SMIF_S25FXXXL)
cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister2(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);

cy_en_smif_status_t Cy_SMIF_S25FL_ReadConfigurationRegister3(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint8_t* readData,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_S25FL_ProgramOrEraseSuspend(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_stc_smif_context_t *context);
/** \} group_mw_smif_functions */

#endif

extern const cy_stc_cmd_sq_t         g_s25flReadCmdTypes[];
extern const cy_stc_addr_sq_t        g_s25flReadAddrTypes[];
extern const cy_stc_mode_sq_t        g_s25flReadModeTypes[];
extern const cy_stc_dummy_sq_t       g_s25flReadDummyTypes[];
extern const uint8_t*                g_s25flReadLatencyCodeToDummyCycle[CY_SMIF_S25FL_READ_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_s25flReadDataTypes[];
extern const cy_stc_read_info_t      g_s25flReadCmdInfo[];
extern const cy_stc_cmd_sq_t         g_s25flWriteCmdTypes[];
extern const cy_stc_addr_sq_t        g_s25flWriteAddrTypes[];
extern const cy_stc_mode_sq_t        g_s25flWriteModeTypes[];
extern const cy_stc_dummy_sq_t       g_s25flWriteDummyTypes[];
extern const uint8_t*                g_s25flWriteLatencyCodeToDummyCycle[CY_SMIF_S25FL_WRITE_LC_DC_TABLE_MAX];
extern const cy_stc_data_sq_t        g_s25flWriteDataTypes[];
extern const cy_stc_write_info_t     g_s25flWriteCmdInfo[];


#if defined(__cplusplus)
}
#endif

#endif /* (CY_SMIF_S25FL_H) */

