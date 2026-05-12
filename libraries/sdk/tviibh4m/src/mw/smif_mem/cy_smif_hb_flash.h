/* cy_smif_hb_flash.h - Source Code for Cypress's Low Level Driver */

#ifndef CY_SMIF_HB_FLASH_H
#define CY_SMIF_HB_FLASH_H

#include "cy_device_headers.h"
#include "cy_project.h"

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/**************************************************** 
* Define how to access your hardware.         *
* Describe to the LLD the flash chip configuration  *
* un-comment the one that matches your system       *
****************************************************/ 
/* LLD System Specific Typedefs */
typedef uint8_t                     CY_SMIF_BYTE;             /* 8 bits wide */
typedef uint8_t                     CY_SMIF_HB_FLASH_CHAR;    /* 8 bit wids */
typedef uint16_t                    CY_SMIF_HB_FLASH_UINT16;  /* 16 bits wide */
typedef uint32_t                    CY_SMIF_HB_FLASH_UINT32;  /* 32 bits wide */
typedef uint64_t                    CY_SMIF_HB_FLASH_UINT64;  /* 64 bits wide */
typedef CY_SMIF_HB_FLASH_UINT32     CY_SMIF_ADDRESS;          /* Used for system level addressing */
typedef uint32_t                    CY_SMIF_WORDCOUNT;        /* used for multi-byte operations */

/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SMIF_SA_OFFSET_MASK      0xFFFE0000   /* mask off the offset */

/* LLD Command Definition */
#define CY_SMIF_NOR_CFI_QUERY_CMD                         (0x98)
#define CY_SMIF_NOR_CHIP_ERASE_CMD                        (0x10)
#define CY_SMIF_NOR_ERASE_SETUP_CMD                       (0x80)
#define CY_SMIF_NOR_RESET_CMD                             (0xF0)
#define CY_SMIF_NOR_SECSI_SECTOR_ENTRY_CMD                (0x88)
#define CY_SMIF_NOR_SECTOR_ERASE_CMD                      (0x30)
#define CY_SMIF_NOR_WRITE_BUFFER_LOAD_CMD                 (0x25)
#define CY_SMIF_NOR_WRITE_BUFFER_PGM_CONFIRM_CMD          (0x29)
#define CY_SMIF_NOR_SET_CONFIG_CMD                        (0xD0)
#define CY_SMIF_NOR_BIT_FIELD_CMD                         (0xBF)

#define CY_SMIF_NOR_ERASE_SUSPEND_CMD                     (0xB0)
#define CY_SMIF_NOR_ERASE_RESUME_CMD                      (0x30)
#define CY_SMIF_NOR_PROGRAM_SUSPEND_CMD                   (0x51)
#define CY_SMIF_NOR_PROGRAM_RESUME_CMD                    (0x50)
#define CY_SMIF_NOR_STATUS_REG_READ_CMD                   (0x70)
#define CY_SMIF_NOR_STATUS_REG_CLEAR_CMD                  (0x71)
#define CY_SMIF_NOR_BLANK_CHECK_CMD                       (0x33)

#define CY_SMIF_ENTER_SPI_MODE_CMD                        (0xF5)

/* Command code definition */
#define CY_SMIF_NOR_AUTOSELECT_CMD                        (0x90)
#define CY_SMIF_NOR_PROGRAM_CMD                           (0xA0)
#define CY_SMIF_NOR_SECSI_SECTOR_EXIT_SETUP_CMD           (0x90)
#define CY_SMIF_NOR_SECSI_SECTOR_EXIT_CMD                 (0x00)
#define CY_SMIF_NOR_UNLOCK_BYPASS_ENTRY_CMD               (0x20)
#define CY_SMIF_NOR_UNLOCK_BYPASS_PROGRAM_CMD             (0xA0)
#define CY_SMIF_NOR_UNLOCK_BYPASS_RESET_CMD1              (0x90)
#define CY_SMIF_NOR_UNLOCK_BYPASS_RESET_CMD2              (0x00)
#define CY_SMIF_NOR_UNLOCK_DATA1                          (0xAA)
#define CY_SMIF_NOR_UNLOCK_DATA2                          (0x55)
#define CY_SMIF_NOR_SUSPEND_CMD                           (0xB0)
#define CY_SMIF_NOR_RESUME_CMD                            (0x30)
#define CY_SMIF_NOR_READ_CONFIG_CMD                       (0xC6)
#define CY_SMIF_NOR_WRITE_BUFFER_ABORT_RESET_CMD          (0xF0)

/* Sector protection command definition */
#define CY_SMIF_PPB_PROTECTED                             (0)
#define CY_SMIF_PPB_UNPROTECTED                           (1)

#define CY_SMIF_WSXXX_LOCK_REG_ENTRY                      (0x40)
#define CY_SMIF_WSXXX_LOCK_REG2_ENTRY                     (0x41)  // for GL-R
#define CY_SMIF_WSXXX_PSWD_PROT_CMD_ENTRY                 (0x60)
#define CY_SMIF_WSXXX_PSWD_UNLOCK_1                       (0x25)
#define CY_SMIF_WSXXX_PSWD_UNLOCK_2                       (0x03)
#define CY_SMIF_WSXXX_PSWD_UNLOCK_3                       (0x29)
#define CY_SMIF_WSXXX_PPB_ENTRY                           (0xC0)
#define CY_SMIF_WSXXX_PPB_ERASE_CONFIRM                   (0x30)
#define CY_SMIF_WSXXX_PPB_LOCK_ENTRY                      (0x50)
#define CY_SMIF_WSXXX_DYB_ENTRY                           (0xE0)
#define CY_SMIF_WSXXX_DYB_CLEAR                           (0x01)

#define CY_SMIF_NOR_LOCK_REG_ENTRY                        (0x40)
#define CY_SMIF_NOR_SECTOR_LOCK_CMD                       (0x60)
#define CY_SMIF_NOR_LOAD_SECTOR_ADR                       (0x61)
#define CY_SMIF_NOR_SECTOR_UNLOCK_ADR6                    (0x40)
#define CY_SMIF_NOR_SECTOR_LOCK_ADR6                      (~CY_SMIF_NOR_SECTOR_UNLOCK_ADR6)

/* RPC2 command code definition */
#define CY_SMIF_NOR_ENTER_DEEP_POWER_DOWN_CMD             (0xB9)
#define CY_SMIF_NOR_MEASURE_TEMPERATURE_CMD               (0xA9)
#define CY_SMIF_NOR_READ_TEMPERATURE_REG_CMD              (0xA8)
#define CY_SMIF_NOR_PROGRAM_POR_TIMER_CMD                 (0x34)
#define CY_SMIF_NOR_READ_POR_TIMER_CMD                    (0x3C)
#define CY_SMIF_NOR_LOAD_INTERRUPT_CFG_REG_CMD            (0x36)
#define CY_SMIF_NOR_READ_INTERRUPT_CFG_REG_CMD            (0xC4)
#define CY_SMIF_NOR_LOAD_INTERRUPT_STATUS_REG_CMD         (0x37)
#define CY_SMIF_NOR_READ_INTERRUPT_STATUS_REG_CMD         (0xC5)
#define CY_SMIF_NOR_LOAD_VOLATILE_CFG_REG_CMD             (0x38)
#define CY_SMIF_NOR_LOAD_VOLATILE_CFG_REG2_CMD            (0x3A)
#define CY_SMIF_NOR_READ_VOLATILE_CFG_REG_CMD             (0xC7)
#define CY_SMIF_NOR_READ_VOLATILE_CFG_REG2_CMD            (0xC9)
#define CY_SMIF_NOR_PROGRAM_NON_VOLATILE_CFG_REG_CMD      (0x39)
#define CY_SMIF_NOR_PROGRAM_NON_VOLATILE_CFG_REG2_CMD     (0x3B)
#define CY_SMIF_NOR_ERASE_NON_VOLATILE_CFG_REG_CMD        (0xC8)
#define CY_SMIF_NOR_READ_NON_VOLATILE_CFG_REG_CMD         (0xC6)
#define CY_SMIF_NOR_READ_NON_VOLATILE_CFG_REG2_CMD        (0xCA)
#define CY_SMIF_NOR_EVALUATE_ERASE_STATUS_CMD             (0xD0)
#define CY_SMIF_NOR_ECC_STATUS_ENTER_CMD                  (0x75)
#define CY_SMIF_NOR_ECC_CLEAR_ERROR_DETECTION_COUNTER_CMD (0x50)
#define CY_SMIF_NOR_ECC_EXIT_CMD                          (0xF0)
#define CY_SMIF_NOR_ECC_READ_ERROR_CMD                    (0x60)
#define CY_SMIF_NOR_CRC_ENTRY_CMD                         (0x78)
#define CY_SMIF_NOR_LOAD_CRC_START_ADDR_CMD               (0xC3)
#define CY_SMIF_NOR_LOAD_CRC_END_ADDR_CMD                 (0x3C)
#define CY_SMIF_NOR_CRC_SUSPEND_CMD                       (0xC0)
#define CY_SMIF_NOR_CRC_RESUME_CMD                        (0xC1)
#define CY_SMIF_NOR_CRC_READ_CHECKVALUE_RESLUT_REG_CMD    (0x60)
#define CY_SMIF_NOR_CRC_EXIT_CMD                          (0xF0)
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/* polling return status */
typedef enum
{
    CY_SMIF_DEV_STATUS_UNKNOWN = 0,
    CY_SMIF_DEV_NOT_BUSY,
    CY_SMIF_DEV_BUSY,
    CY_SMIF_DEV_EXCEEDED_TIME_LIMITS,
    CY_SMIF_DEV_SUSPEND,
    CY_SMIF_DEV_WRITE_BUFFER_ABORT,
    CY_SMIF_DEV_STATUS_GET_PROBLEM,
    CY_SMIF_DEV_VERIFY_ERROR,
    CY_SMIF_DEV_BYTES_PER_OP_WRONG,
    CY_SMIF_DEV_ERASE_ERROR,       
    CY_SMIF_DEV_PROGRAM_ERROR,       
    CY_SMIF_DEV_SECTOR_LOCK,
    CY_SMIF_DEV_PROGRAM_SUSPEND,       /* Device is in program suspend mode */
    CY_SMIF_DEV_PROGRAM_SUSPEND_ERROR, /* Device program suspend error */
    CY_SMIF_DEV_ERASE_SUSPEND,         /* Device is in erase suspend mode */
    CY_SMIF_DEV_ERASE_SUSPEND_ERROR,   /* Device erase suspend error */
    CY_SMIF_DEV_BUSY_IN_OTHER_BANK     /* Busy operation in other bank */
} CY_SMIF_HB_DEVSTATUS;

/* External memory device types */
typedef enum
{
    CY_SMIF_HB_MEMTYPE_HYPERFLASH_S26K,          /**< S26K HyperFlash series */
    CY_SMIF_HB_MEMTYPE_HYPERFLASH_S26H,          /**< S26H HyperFlash series (aka SemperFlash) */
    CY_SMIF_HB_MEMTYPE_MAX_HYPERFLASH_TYPES,     /**< for internal use, to just distinguish between HyperRAM and HyperFlash */
    CY_SMIF_HB_MEMTYPE_HYPERRAM_S27KXXXX1,       /**< S27K????1 HyperRAM series */
    CY_SMIF_HB_MEMTYPE_HYPERRAM_S27KXXXX2,       /**< S27K????2 HyperRAM series */
} cy_smif_hb_memtype_t;



/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SMIF_DEV_RDY_MASK                              (0x80) /* Device Ready Bit */
#define CY_SMIF_DEV_ERASE_SUSP_MASK                       (0x40) /* Erase Suspend Bit */
#define CY_SMIF_DEV_ERASE_MASK                            (0x20) /* Erase Status Bit */
#define CY_SMIF_DEV_PROGRAM_MASK                          (0x10) /* Program Status Bit */
#define CY_SMIF_DEV_RFU_MASK                              (0x08) /* Reserved */
#define CY_SMIF_DEV_PROGRAM_SUSP_MASK                     (0x04) /* Program Suspend Bit */
#define CY_SMIF_DEV_SEC_LOCK_MASK                         (0x02) /* Sector lock Bit */
#define CY_SMIF_DEV_BANK_MASK                             (0x01) /* Operation in current bank */
#define CY_SMIF_DEV_CRCSSB_MASK                           (0x0100)  /* CRC Suspend Bit, 1: suspend, 0: no suspend*/
#define CY_SMIF_DEV_ESTAT_MASK                            (0x01)    /* Sector Erase Status Bit (for Evaluate Erase Status)*/
                                                                    /*0=previous erase did not complete successfully*/
                                                                    /*1=previous erase completed successfully*/
/*****************************************************
* Define Flash read/write macro to be used by LLD    *
*****************************************************/
#define CY_SMIF_FLASH_OFFSET(b,o)       (*(( (volatile CY_SMIF_FLASHDATA*)(b) ) + (o)))

#if (CY_CPU_CORTEX_M7)
  #define CY_SMIF_FLASH_WR(b,o,d)\
  {\
    __DMB();\
    CY_SMIF_FLASH_OFFSET((b),(o)) = (d);\
  }
#else
  #define CY_SMIF_FLASH_WR(b,o,d) CY_SMIF_FLASH_OFFSET((b),(o)) = (d)
#endif
#define CY_SMIF_FLASH_RD(b,o)   CY_SMIF_FLASH_OFFSET((b),(o))          


#define CY_SMIF_HB_FLASH_UNLOCK_ADDR1        0x00000555
#define CY_SMIF_HB_FLASH_UNLOCK_ADDR2        0x000002AA
#define CY_SMIF_HB_FLASH_CFI_UNLOCK_ADDR1    0x00000055
typedef CY_SMIF_HB_FLASH_UINT16              CY_SMIF_FLASHDATA;
#define CY_SMIF_HB_FLASH_BUF_SIZE_MULTIPLIER 1


/* Checks whether the given memtype (cy_smif_hb_memtype_t) is a HyperFlash (true) or HyperRAM (false) */
#define CY_SMIF_HB_IS_HYPERFLASH(memtype)    (((memtype) <= CY_SMIF_HB_MEMTYPE_MAX_HYPERFLASH_TYPES) ? true : false)


/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/** Specifies Hyper Bus latency code */
typedef enum
{
    CY_SMIF_HB_LC5  = 0u,  /**< 5 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC6  = 1u,  /**< 6 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC7  = 2u,  /**< 7 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC8  = 3u,  /**< 8 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC9  = 4u,  /**< 9 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC10 = 5u,  /**< 10 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC11 = 6u,  /**< 11 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC12 = 7u,  /**< 12 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC13 = 8u,  /**< 13 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC14 = 9u,  /**< 14 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC15 = 10u, /**< 15 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC16 = 11u, /**< 16 latency cycle for S26K and S26H */
    CY_SMIF_HB_LC20 = 12u, /**< 20 latency cycle for S26H */
    CY_SMIF_HB_LC3  = 14u, /**< 3  latency cycle for S27K */
    CY_SMIF_HB_LC4  = 15u, /**< 4  latency cycle for S27K */
} cy_en_smif_hb_latency_code_t;

/** Specifies top 8 bit of read/write sequence (bit 47 ~ bit 40). only top 3 bits have meaning */
/** bit 47: Identifies the transaction as a Read or Write. R/W#=1 indicates a Read operation and R/W#=0 indicates a Write operation. */
/** bit 46: Indicates whether the Read or Write operation accesses the memory or register spaces. */
/** bit 45: Indicates whether the burst will be continuous or wrapped. Burst Type=0 indicates Wrapped Burst, Burst Type=1 indicates Continuous Burst. */
/** bit 46 is always 0, since this drive assumes accessing memory space in XIP mode */
typedef enum
{
    CY_SMIF_HB_READ_WRAPPED_BURST    = 0x80, /**< bit 47 = 1: read, bit 45 = 0: wrapped burst */
    CY_SMIF_HB_READ_CONTINUOUS_BURST = 0xA0, /**< bit 47 = 1: read, bit 45 = 1: continuous burst */
} cy_en_smif_hb_rd_cmd_t;

/** see \ref cy_en_smif_hb_rd_cmd_t */
typedef enum
{
    CY_SMIF_HB_WRITE_WRAPPED_BURST    = 0x00, /**< bit 47 = 0: write, bit 45 = 0: wrapped burst */
    CY_SMIF_HB_WRITE_CONTINUOUS_BURST = 0x20, /**< bit 47 = 0: write, bit 45 = 1: continuous burst */
} cy_en_smif_hb_wt_cmd_t;

/** Specifies hyper bus device type. FLASH or SRAM */
typedef enum
{
    CY_SMIF_HB_FLASH = 0, /**< Hyper bus FLASH */
    CY_SMIF_HB_SRAM  = 1, /**< Hyper bus SRAM */
} cy_en_smif_hb_dev_type_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Specifies structure of configuration parameter of SMIF device for hyper bus memory */
typedef struct
{
    cy_en_smif_hb_rd_cmd_t         xipReadCmd;         /**< read command value \ref cy_en_smif_hb_rd_cmd_t */
    cy_en_smif_hb_wt_cmd_t         xipWriteCmd;        /**< write command value \ref cy_en_smif_hb_rd_cmd_t */
    bool                           mergeEnable;        /**< merge enable */
    cy_en_smif_dev_merge_timeout_t mergeTimeout;       /**< merge timeout value */
    bool                           totalTimeoutEnable; /**< total timeout enable */
    uint16_t                       totalTimeout;       /**< total timeout in clk_mem cycles */
    uint32_t                       addr;               /**< base address in XIP mode */
    cy_en_device_size_t            size;               /**< size of device memory in XIP mode \ref cy_en_device_size_t */
    cy_en_smif_hb_latency_code_t   lc_hb;              /**< latency code of read */
    cy_en_smif_hb_dev_type_t       hbDevType;          /**< hyper bus device type \ref cy_en_smif_hb_dev_type_t */
} cy_stc_device_hb_config_t;

/** command/address bit assignments of SMIF device for hyper bus memory */
typedef struct
{
    uint64_t ColumnAddress     : 3;  /**< column component of the target address */
    uint64_t Reserved0         : 10; /**< reserved */
    uint64_t ByteEnableForRAM  : 2;  /**< Byte Enables used for HyperFlash RAM devices (donft care for Flash). Byte Enable = b'01 
                                          indicates that bits 8-15 are masked, Byte Enable = b'10 indicates that bits 7-0 are masked.
                                          These bits should be set to b'11 by the host controller to insure compatibility with future functionality. */
    uint64_t Reserved1         : 1;  /**< reserved */
    uint64_t PageAddress       : 24; /**<  Page component and half page component of target address */
    uint64_t Reserved2         : 5;  /**< reserved */
    uint64_t BurstType         : 1;  /**< indicates whether the burst will be continuous or wrapped. Burst Type=0 indicates Wrapped Burst, Burst Type=1 indicates Continuous Burst */
    uint64_t Target            : 1;  /**< indicates whether the Read or Write operation accesses the memory or register spaces. The HyperFlash Flash devices will not take 
                                          advantage of this feature and this bit should be set to 0 during Read or Write transactions.*/
    uint64_t ReadWrite         : 1;  /**< identifies the transaction as a Read or Write. R/W#=1 indicates a Read operation and R/W#=0 indicates a Write operation.*/
    uint64_t Reserved3         : 16; /**< reserved. command address bit is 48 bit in hyper bus, thus this field is just a dummy. APIs ignore this field. */
} cy_stc_hb_com_addr_bitfield_t;

/** union of command/address of hyper bus memory */
typedef union
{
    uint64_t       u64;             /**< 64 bit access */
    uint16_t       u16[4];          /**< 16 bit access */
    uint32_t       u32[2];          /**< 32 bit access */
    cy_stc_hb_com_addr_bitfield_t fld; /**< \ref stc_hb_com_addr_bitfield_t */
} cy_un_Hyper_Com_Addr_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/** burst type. continuous or wrapped */
typedef enum
{
    CY_SMIF_HB_WRAPPED_BURST    = 0, /**< continue to wrap within the burst length */
    CY_SMIF_HB_COUTINUOUS_BURST = 1, /**< output data in a sequential manner across page boundaries */
} cy_en_hb_bust_type_t;

/** read target. memory or register */
typedef enum
{
    CY_SMIF_HB_TARGET_MEMORY   = 0, /**< memory */
    CY_SMIF_HB_TARGET_REGISTER = 1, /**< register. it is used only for hyper SRAM */
} cy_en_hb_target_t;

/** read or write */
typedef enum
{
    CY_SMIF_HB_WRITE   = 0, /**< write */
    CY_SMIF_HB_READ    = 1, /**< read */
} cy_en_hb_read_write_t;
/** \} group_mw_smif_enums */


/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/**********************************************************/
/***     Utilities for Hyper Flash Status Register      ***/
/**********************************************************/
/** Hyper Flash Status Register bit field definition */
typedef struct
{
    uint16_t SectorEraseSts     : 1; /** < Sector Erase Success/Failure Status Flag */
    uint16_t SectorLockSts      : 1; /** < Sector Protection (Lock) Error Flag */
    uint16_t ProgramSuspendSts  : 1; /** < Program operation Suspend Status Flag */
    uint16_t WriteBufAbortSts   : 1; /** < Write Buffer Abort Status Flag */
    uint16_t ProgramSts         : 1; /** < Programming Error Status Flag */
    uint16_t EraseSts           : 1; /** < Erasing Error Status Flag */
    uint16_t EraseSuspendSts    : 1; /** < Erase operation Suspend Status Flag */
    uint16_t DeviceReady        : 1; /** < Device Ready/Busy Status Flag */
    uint16_t CRC_SuspendSts     : 1; /** < Memory Array Data Integrity Check CRC Suspend Status Flag */
    uint16_t Reserved           : 7; /** < Reserved for Future Use */
} cy_stc_h_flash_stat_fld_t;

/** Hyper Flash Status Register union */
typedef union
{
    uint16_t                  u16; /** < 16 bit access */
    cy_stc_h_flash_stat_fld_t fld; /** < \ref cy_stc_h_flash_stat_fld_t */
} cy_un_h_flash_stat_reg_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/**********************************************************/
/*** Utilities for Hyper Flash Configuration Register 1 ***/
/**********************************************************/
/** Option for deepSleepPowMode bit of \ref stc_Configuration1Field_t */
typedef enum
{
    CY_SMIF_HB_POWERDOWN_MODE_AFTER_POR = 0, /** < Power Down Power mode is entered upon the completion of Power-On-Reset */
    CY_SMIF_HB_STANBY_MODE_AFTER_POR    = 1, /** < Standby mode is entered upon the completion of Power-On-Reset */
} cy_en_hb_cfg1_deep_sleep_power_mode_t;

/** Option for driveStrength bit of \ref cy_stc_h_flash_cfg1_field_t (only for S26K series) */
typedef enum
{
    CY_SMIF_HB_S26K_DRV_27_20_OHMS  = 0, /** < 27ohms for 1.8V VCCQ,  20ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_117_71_OHMS = 1, /** < 117ohms for 1.8V VCCQ, 71ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_68_40_OHMS  = 2, /** < 68ohms for 1.V VCCQ,   40ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_45_27_OHMS  = 3, /** < 45ohms for 1.8V VCCQ,  27ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_34_20_OHMS  = 4, /** < 34ohms for 1.8V VCCQ,  20ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_27_16_OHMS  = 5, /** < 27ohms for 1.8V VCCQ,  16ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_24_14_OHMS  = 6, /** < 24ohms for 1.8V VCCQ,  14ohms for 3.0V VCCQ */
    CY_SMIF_HB_S26K_DRV_20_12_OHMS  = 7, /** < 20ohms for 1.8V VCCQ,  12ohms for 3.0V VCCQ */
} cy_en_hb_s26k_cfg1_drv_str_t;

/** Option for driveStrength bit of \ref cy_stc_h_flash_cfg1_field_t (only for S26H series) */
typedef enum
{
    CY_SMIF_HB_S26H_DRV_45_OHMS  = 0, /** < 45ohms */
    CY_SMIF_HB_S26H_DRV_120_OHMS = 1, /** < 120ohms */
    CY_SMIF_HB_S26H_DRV_90_OHMS  = 2, /** < 90ohms */
    CY_SMIF_HB_S26H_DRV_60_OHMS  = 3, /** < 60ohms */
    CY_SMIF_HB_S26H_DRV_30_OHMS  = 5, /** < 30ohms */
    CY_SMIF_HB_S26H_DRV_20_OHMS  = 6, /** < 20ohms */
    CY_SMIF_HB_S26H_DRV_15_OHMS  = 7, /** < 15ohms */
} cy_en_hb_s26h_cfg1_drv_str_t;

/** Option for VCR_freeze bit of \ref cy_stc_h_flash_cfg1_field_t */
typedef enum
{
    CY_SMIF_HB_CONFIG_REG_LOCKED   = 0, /** < No Programming or Erasing of NVCR, no changes to VCR */
    CY_SMIF_HB_CONFIG_REG_UNLOCKED = 1, /** < Configuration Registers are not protected (default) */
} cy_en_hb_cfg1_reg_lock_t;

/** Option for SSR_freeze bit of \ref cy_stc_h_flash_cfg1_field_t */
typedef enum
{
    CY_SMIF_HB_SSR_LOCKED   = 0, /** < Secure Silicon Region Locked (Programming not allowed) */
    CY_SMIF_HB_SSR_UNLOCKED = 1, /** < Secure Silicon Region Unlocked (Programming allowed) (default) */
} cy_en_bh_cfg1_ssr_lock_t;

/** Option for sectorMapping bit of \ref cy_stc_h_flash_cfg1_field_t */
typedef enum
{
    CY_SMIF_HB_MAPPED_LOWEST_ADDR   = 0, /** < 4 KB Parameter-Sectors Block and Read Password Sector is mapped into the bottom(lowest) of the memory address space */
    CY_SMIF_HB_MAPPED_HIGHEST_ADDR  = 1, /** < 4 KB Parameter-Sectors Block and Read Password Sector is mapped into the top(highest) of the memory address space */
    CY_SMIF_HB_UN_SECT_LOWEST_ADDR  = 2, /** < Uniform Sector architecture and Read Password Sector is mapped into the bottom(lowest) of the memory address space */
    CY_SMIF_HB_UN_SECT_HIGHEST_ADDR = 3, /** < Uniform Sector architecture and Read Password Sector is mapped into the top(highest) of the memory address space */
} cy_en_hb_cfg1_sect_map_t;

/** Option for RDS_stallControll bit of \ref cy_stc_h_flash_cfg1_field_t */
typedef enum
{
    CY_SMIF_HB_DS_STALL_ON_ERR     = 0, /** < DS stalls on device errors */
    CY_SMIF_HB_DS_NOT_STALL_ON_ERR = 1, /** < DS does not stall on device errors */
} cy_en_hb_cfg1_ds_stall_on_err_t;

/** Option for burstLength bit of \ref cy_stc_h_flash_cfg1_field_t */
typedef enum
{
    CY_SMIF_HB_WRAP_BURST_LEN_64 = 1, /** < 64 Bytes Wrap length */
    CY_SMIF_HB_WRAP_BURST_LEN_16 = 2, /** < 16 Bytes Wrap length */
    CY_SMIF_HB_WRAP_BURST_LEN_32 = 3, /** < 32 Bytes Wrap length */
} cy_en_hb_cfg1_burst_len_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Configuration 1 Register bit field definition */
typedef struct
{
    uint16_t burstLength       : 2; /** < Read Burst Wrap Length selection */
    uint16_t RDS_stallControll : 1; /** < Data Strobe Stall during Device Error Selection (DS) */
    uint16_t Reserved0         : 1; /** < Reserved for Future Use */
    uint16_t readLatency       : 4; /** < Memory Array Read Latency selection - Dummy cycles required for initial data access */
    uint16_t sectorMapping     : 2; /** < Top or Bottom Address Range Selection for 4 KB Sector Block */
    uint16_t SSR_freeze        : 1; /** < Temporary Locking selection of Secure Silicon Region */
    uint16_t VCR_freeze        : 1; /** < Temporary Locking selection of Configuration Registers */
    uint16_t driveStrength     : 3; /** < I/O Driver Output Impedance Selection */
    uint16_t deepSleepPowMode  : 1; /** < Deep Power Down power saving mode entry selection upon Power-On-Reset (supported only by S26H) */
} cy_stc_h_flash_cfg1_field_t;

/** Configuration 1 Register union */
typedef union
{
    uint16_t                     u16; /** < 16 bit access */
    cy_stc_h_flash_cfg1_field_t  fld; /** < \ref cy_stc_h_flash_cfg1_field_t */
} cy_un_h_flash_cfg1_reg_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CFG_REG1_DEFAULT_S26K (0x8EBB)
#define CFG_REG1_DEFAULT_S26H (0xDEBF)
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/**********************************************************/
/*** Utilities for Hyper Flash Configuration Register 2 ***/
/**********************************************************/
/** Option for masterClkSel bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_MASTER_CLK_DIFFERENTIAL = 0, /** < Master Clock is differential (CK, CK#) */
    CY_SMIF_HB_MASTER_CLK_SINGLE_ENDED = 1, /** < Master Clock is single ended (CK) */
} cy_en_hb_cfg2_master_clk_select_t;

/** Option for split4KBSector bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_4KB_SECTORS_GROUPED = 0, /** < 4 KB Sectors are grouped together */
    CY_SMIF_HB_4KB_SECTORS_SPLIT   = 1, /** < 4 KB Sectors are split between High and Low Addresses */
} cy_en_hb_cfg2_4kb_sect_grouped_t;

/** Option for hybridBusrtSel bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_HYBRID_BURST_ENABLED  = 0, /** < Hybrid burst is enabled (Wrapped burst followed by linear burst) */
    CY_SMIF_HB_HYBRID_BURST_DISABLED = 1, /** < Hybrid burst is disabled */
} cy_en_hb_cfg2_hybrid_burst_en_t;

/** Option for eccErrorSel bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_1_2BIT_ECC_ERR_PROCESSED = 0, /** < 1-bit ECC Error Detection/Correction and 2-bit ECC error detection */
    CY_SMIF_HB_1BIT_ECC_ERR_PROCESSED   = 1, /** < 1-bit ECC Error Detection/Correction */
} cy_en_hb_cfg2_ecc_err_sel_t;

/** Option for addrTrapEccSel bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_1_2BIT_ERR_ADDR_TRAPED = 0, /** < Address Trap Register traps address of both 1-bit and 2-bit ECC errors */
    CY_SMIF_HB_2BIT_ERR_ADDR_TRAPED   = 1, /** < Address Trap Register traps address of 2-bit ECC errors */
} cy_en_hb_cfg2_ecc_addr_trap_sel_t;

/** Option for doubleLatencySel bit of \ref cy_stc_h_flash_cfg2_field_t */
typedef enum
{
    CY_SMIF_HB_DOUBLE_LATENCY = 0, /** < Double initial latency - zero latency at page boundaries */
    CY_SMIF_HB_SINGLE_LATENCY = 1, /** < Single initial latency - latency at page boundaries */
} cy_en_hb_cfg2_double_latency_sel_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Configuration 2 Register bit field definition */
typedef struct
{
    uint16_t masterClkSel      : 1; /** < Master Clock Selection - Single Ended or Differential */
    uint16_t reserved0         : 1; /** < Reserved for Future Use */
    uint16_t split4KBSector    : 1; /** < Split 4 KB Sectors selection between top(highest) and bottom(lowest) address space */
    uint16_t reserved1         : 1; /** < Reserved for Future Use */
    uint16_t hybridBurstSel    : 1; /** < Hybrid Burst Selection - CA[45] must select Wrapped Burst type for this option */
    uint16_t eccErrorSel       : 1; /** < Error Correction Code (ECC) 1-bit or 1-bit/2-bit error correction selection */
    uint16_t addrTrapEccSel    : 1; /** < Address Trap Register 1-bit or 1-bit/2-bit ECC Error selection */
    uint16_t doubleLatencySel  : 1; /** < Memory Read Page Boundary Latency selection */
    uint16_t reserved2         : 8; /** < Reserved for Future Use */
} cy_stc_h_flash_cfg2_field_t;

/** Hyper Flash Configuration 2 Register union */
typedef union
{
    uint16_t                     u16; /** < 16 bit access */
    cy_stc_h_flash_cfg2_field_t  fld; /** < \ref cy_stc_h_flash_cfg2_field_t */
} cy_un_h_flash_cfg2_reg_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CFG_REG2_DEFAULT_S26H (0xFDFF)
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/**********************************************************/
/***  Utilities for Hyper RAM Configuration Register 0  ***/
/**********************************************************/
/** Option for deepPowerDown bit of \ref cy_stc_h_ram_cfg0_field_t */
typedef enum
{
    CY_SMIF_H_RAM_DEEP_POW_DOWN = 0, /** < Setting this causes the device to enter Deep Power Down */
    CY_SMIF_H_RAM_NORMAL_OP     = 1, /** < Normal operation */
} cy_en_h_ram_cfg0_deep_power_down_t;

/** Option for driveStrength bit of \ref cy_stc_h_ram_cfg0_field_t */
typedef enum
{
    CY_SMIF_H_RAM_DRV_34_OHMS  = 0, /** < 34ohms  */
    CY_SMIF_H_RAM_DRV_115_OHMS = 1, /** < 115ohms */
    CY_SMIF_H_RAM_DRV_67_OHMS  = 2, /** < 67ohms  */
    CY_SMIF_H_RAM_DRV_46_OHMS  = 3, /** < 46ohms  */
    CY_SMIF_H_RAM_DRV_27_OHMS  = 5, /** < 27ohms  */
    CY_SMIF_H_RAM_DRV_22_OHMS  = 6, /** < 22ohms  */
    CY_SMIF_H_RAM_DRV_19_OHMS  = 7, /** < 19ohms  */
} cy_en_h_ram_cfg0_drv_str_t;

/** Option for varLatency bit of \ref cy_stc_h_ram_cfg0_field_t */
typedef enum
{
    CY_SMIF_H_RAM_VAR_LATENCY   = 0, /** < Variable Latency - 1 or 2 times Initial Latency depending on RWDS during CA cycles.  */
    CY_SMIF_H_RAM_FIXED_LATENCY = 1, /** < Fixed 2 times Initial Latency */
} cy_en_h_ram_var_latency_t;

/** Option for hyBurstEn bit of \ref cy_stc_h_ram_cfg0_field_t */
typedef enum
{
    CY_SMIF_H_RAM_HY_BURST_ENABLE  = 0, /** < Hybrid Wrapped burst sequences to follow hybrid burst sequencing */
    CY_SMIF_H_RAM_HY_BURST_DISABLE = 1, /** < Legacy Wrapped burst sequences in legacy wrapped burst manner */
} cy_en_h_ram_hybrid_burst_t;

/** Option for burstLength bit of \ref cy_stc_h_ram_cfg0_field_t */
typedef enum
{
    CY_SMIF_H_RAM_BURST_LEN_128 = 1, /** < 128 Bytes Wrap length */
    CY_SMIF_H_RAM_BURST_LEN_64  = 1, /** < 64 Bytes Wrap length */
    CY_SMIF_H_RAM_BURST_LEN_16  = 2, /** < 16 Bytes Wrap length */
    CY_SMIF_H_RAM_BURST_LEN_32  = 3, /** < 32 Bytes Wrap length */
} cy_en_h_ram_cfg0_burst_len_t;
/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/
/** Hyper RAM Configuration 0 Register bit field definition */
typedef struct
{
    uint16_t burstLength   : 2; /** < Read Burst Wrap Length selection */
    uint16_t hyBurstEn     : 1; /** < Wrapped Burst Type
                                      - Legacy wrap (sequential access with wrap around within a selected length and aligned group)
                                      - Hybrid wrap (Legacy wrap once then linear burst at start of the next sequential group) */
    uint16_t varLatency    : 1; /** < Whether an array read or write transaction will use fixed or variable latency. If fixed latency is selected the memory will always 
                                      indicate a refresh latency and delay the read data transfer accordingly. If variable latency is selected, latency for a refresh is only 
                                      added when a refresh is required at the same time a new transaction is starting. */
    uint16_t readLatency   : 4; /** < Memory Array Read Latency selection - Dummy cycles required for initial data access */
    uint16_t reserved      : 4; /** < Reserved for Future Use */
    uint16_t driveStrength : 3; /** < I/O Driver Output Impedance Selection */
    uint16_t deepPowerDown : 1; /** < Deep Power Down Enable */
} cy_stc_h_ram_cfg0_field_t;

/** Hyper RAM Configuration 0 Register union */
typedef union
{
    uint16_t                   u16; /** < 16 bit access */
    cy_stc_h_ram_cfg0_field_t  fld; /** < \ref cy_stc_h_ram_cfg0_field_t */
} cy_un_h_ram_cfg0_reg_t;
/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SMIF_CFG_REG0_ADDR_S27K    (0x00000800)
#define CY_SMIF_CFG_REG0_DEFAULT_S27KXXXX1 (0x8F1F)
#define CY_SMIF_CFG_REG0_DEFAULT_S27KXXXX2 (0x8F2F)

/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/
/**********************************************************/
/***  Utilities for Hyper RAM Configuration Register 1  ***/
/**********************************************************/
/** Option for dstbRefreshIntvl bit of \ref cy_stc_h_ram_cfg1_s27kxxxx1_field_t */
typedef enum
{
    CY_SMIF_H_RAM_2_TIMES_DEFAULT      = 0, /** < 2 times default */
    CY_SMIF_H_RAM_4_HALF_TIMES_DEFAULT = 1, /** < 4 times default */
    CY_SMIF_H_RAM_DEFAULT_INTVL        = 2, /** < default 
                                                   4 us for Industrial temperature range devices
                                                   1 us for Industrial Plus temperature range devices */
    CY_SMIF_H_RAM_1_HALF_TIMES_DEFAULT = 3, /** < 1.5 times default */
} cy_en_h_ram_cfg1_s27kxxxx1_dstb_refresh_intvl_t;

/** Option for masterClkSel bit of \ref cy_stc_h_ram_cfg1_s27kxxxx2_field_t */
typedef enum
{
    CY_SMIF_H_RAM_MASTER_CLK_DIFFERENTIAL = 0, /** < Master Clock is differential (CK, CK#) */
    CY_SMIF_H_RAM_MASTER_CLK_SINGLE_ENDED = 1, /** < Master Clock is single ended (CK) */
} cy_en_h_ram_cfg1_master_clk_select_t;

/** Option for partialRefresh bit of \ref cy_stc_h_ram_cfg1_s27kxxxx2_field_t */
typedef enum
{
    CY_SMIF_H_RAM_REFRESH_ARRAY_FULL           = 0,   /** < Refresh full array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_BOTTOM_HALF    = 1,   /** < Refresh bottom half of array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_BOTTOM_QUARTER = 2,   /** < Refresh bottom quarter of array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_BOTTOM_EIGHTH  = 3,   /** < Refresh bottom eighth of array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_NONE           = 4,   /** < Do not refresh array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_TOP_HALF       = 5,   /** < Refresh top half of array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_TOP_QUARTER    = 6,   /** < Refresh top quarter of array */
    CY_SMIF_H_RAM_REFRESH_ARRAY_TOP_EIGHTH     = 7,   /** < Refresh top eighth of array */
} cy_en_h_ram_cfg1_partial_refresh_t;

/** Option for hybridSleep bit of \ref cy_stc_h_ram_cfg1_s27kxxxx2_field_t */
typedef enum
{
    CY_SMIF_H_RAM_HYBRID_SLEEP_DISABLED = 0, /** < Normal operation */
    CY_SMIF_H_RAM_HYBRID_SLEEP_ENABLED = 1,  /** < Hybrid Sleep Mode enabled */
} cy_en_h_ram_cfg1_hybrid_sleep_t;

/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/** Hyper RAM Configuration 1 Register bit field definition for S27KXXXX1 */
typedef struct
{
    uint16_t dstbRefreshIntvl : 2;  /** < used to define the distributed refresh interval for this HyperRAM device */
    uint16_t reserved         : 14; /** < Reserved for Future Use */
} cy_stc_h_ram_cfg1_s27kxxxx1_field_t;

/** Hyper RAM Configuration 1 Register bit field definition for S27KXXXX2 */
typedef struct
{
    uint16_t dstbRefreshIntvl : 2;  /** < read-only! Shows the distributed refresh interval for the particular device */
    uint16_t partialRefresh   : 3;  /** < Partial Array Refresh selection */
    uint16_t hybridSleep      : 1;  /** < Hybrid Sleep or Normal Op */
    uint16_t masterClkSel     : 1;  /** < Single or differential clock */
    uint16_t reserved         : 9;  /** < Reserved for Future Use */
} cy_stc_h_ram_cfg1_s27kxxxx2_field_t;

/** Hyper RAM Configuration 1 Register bit field union for the different "generations" of S27K devices */
typedef union
{
    cy_stc_h_ram_cfg1_s27kxxxx1_field_t  s27kxxxx1; /** < \ref cy_stc_h_ram_cfg1_s27kxxxx1_field_t */
    cy_stc_h_ram_cfg1_s27kxxxx2_field_t  s27kxxxx2; /** < \ref cy_stc_h_ram_cfg1_s27kxxxx2_field_t */
} cy_un_h_ram_cfg1_field_t;

/** Hyper RAM Configuration 1 Register union */
typedef union
{
    uint16_t                   u16;      /** < 16 bit access */
    cy_un_h_ram_cfg1_field_t   fldUnion; /** < \ref cy_un_h_ram_cfg1_field_t */
} cy_un_h_ram_cfg1_reg_t;


/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SMIF_CFG_REG1_ADDR_S27K    (0x00000801)
#define CY_SMIF_CFG_REG1_DEFAULT_S27KXXXX1 (0x0002)
#define CY_SMIF_CFG_REG1_DEFAULT_S27KXXXX2 (0xffc2)
#define CY_SMIF_CFG_REG1_MASK_WRITABLE_BITS_S27KXXXX1 (0xffff)
#define CY_SMIF_CFG_REG1_MASK_WRITABLE_BITS_S27KXXXX2 (0xfffc)

/* HyperRAM Register CY_SMIF_WORD Offset Address */
/* Macro to be used for reg_addr argument of API */
#define CY_SMIF_ID_REG0_ADDR_S27K           (0x00000000)
#define CY_SMIF_ID_REG1_ADDR_S27K           (0x00000001)
#define CY_SMIF_DIE_MFR_INFO_REG0_ADDR_S27K (0x00010000)
#define CY_SMIF_DIE_MFR_INFO_REG1_ADDR_S27K (0x00010001)
#define CY_SMIF_DIE_MFR_INFO_REG2_ADDR_S27K (0x00010002)
/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_macros
* \{
*/

/* Length of the calibration data pattern (Cy_SMIF_HB_CalibrationDataPattern) for HyperBus in bytes */
#define CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH    12

/** \} group_mw_smif_macros */


/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/* The reference calibration data pattern, can be used for comparing the actually read calibration data pattern
   or as source for preparing the pattern */
extern const uint8_t Cy_SMIF_HB_CalibrationDataPattern[CY_SMIF_HB_CALIBRATION_DATA_PATTERN_LENGTH];

/** \} group_mw_smif_data_structures */


/**
* \addtogroup group_mw_smif_functions
* \{
*/
/* public function prototypes */
extern cy_en_smif_status_t Cy_SMIF_InitDeviceHyperBus(volatile cy_stc_smif_reg_device_t *device, const cy_stc_device_hb_config_t *config);
extern cy_en_smif_status_t Cy_SMIF_HB_SetDummyCycles(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_dev_type_t hbDevType, cy_en_smif_hb_latency_code_t lc_hb);
extern bool                Cy_SMIF_HB_CheckOrPrepareCalibrationDataPattern(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_dev_type_t hbDevType, uint32_t calibrationDataAddr, bool eraseIfNecessary);
extern cy_en_smif_status_t Cy_SMIF_HB_CalibrateDelay(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_hb_latency_code_t lc_hb, uint32_t calibrationDataOffsetFromBase, cy_stc_smif_context_t *context, bool scanAllDelayLines);
extern cy_en_smif_status_t Cy_SMIF_HB_GetOptimalLatencyCode(uint8_t targetClockMHz, cy_smif_hb_memtype_t memType, cy_en_smif_hb_latency_code_t *latencyCode);
extern uint32_t            Cy_SMIF_HB_CalculateTotalTimeoutCyclesForHyperRAM(uint32_t maxChipSelectLowTimeNs, uint8_t clkMemFreqMhz, uint8_t smifExtClockMhz);
extern cy_en_smif_status_t Cy_SMIF_HB_MMIO_Read(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_hb_bust_type_t burstType,
                                        uint32_t readAddress,
                                        uint32_t sizeInHalfWord,
                                        uint16_t buf[],
                                        cy_en_smif_hb_latency_code_t lcCode,
                                        bool doubleLat,
                                        bool isblockingMode,
                                        cy_stc_smif_context_t *context);

extern cy_en_smif_status_t Cy_SMIF_HB_MMIO_Write(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_hb_bust_type_t burstType,
                                        uint32_t writeAddress,
                                        uint32_t sizeInHalfWord,
                                        uint16_t buf[],
                                        cy_en_smif_hb_dev_type_t hbDevType,
                                        cy_en_smif_hb_latency_code_t lcCode,
                                        bool isblockingMode,
                                        cy_stc_smif_context_t *context);
extern uint32_t            Cy_SMIF_HB_LatencyCodeToCycle(cy_en_smif_hb_latency_code_t code);
extern cy_en_smif_status_t CY_SMIF_ReadHYPERRAM_REG(volatile cy_stc_smif_reg_device_t *device, uint32_t reg_addr, uint16_t* reg_value);
extern cy_en_smif_status_t CY_SMIF_WriteHYPERRAM_REG(volatile cy_stc_smif_reg_device_t *device, uint32_t reg_addr, uint16_t reg_value);

/* Operation Functions */
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_WriteBufferProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_WORDCOUNT word_cnt, CY_SMIF_FLASHDATA *data_buf);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_FLASHDATA write_data);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_SectorEraseOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ChipEraseOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_InitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ResetCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_SectorEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_ChipEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset,CY_SMIF_FLASHDATA *pgm_data_ptr);
extern void                 CY_SMIF_HbFlash_SecSiSectorExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_SecSiSectorEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_WriteToBufferCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_ProgramBufferToFlashCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_WriteBufferAbortResetCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadCfiWord(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_CfiExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_CfiEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_StatusClear(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_StatusGetReg(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_memcpy(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset,CY_SMIF_WORDCOUNT words_cnt,CY_SMIF_FLASHDATA *data_buf);
extern void                 CY_SMIF_HbFlash_StatusRegClearCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_StatusRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_AutoselectEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr,CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_AutoselectExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ProgramSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_EraseSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_EraseResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ProgramResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_EraseSuspendOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramSuspendOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_Poll(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern uint32_t             CY_SMIF_HbFlash_GetDeviceId(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_BlankCheckCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_BlankCheckOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);

/**********************************************************
* Sector protection functions prototype.
**********************************************************/
extern void                 CY_SMIF_HbFlash_LockRegEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LockRegBitsProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA value);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_LockRegBitsReadCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LockRegExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PasswordProtectionEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PasswordProtectionProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset, CY_SMIF_FLASHDATA pwd);
extern void                 CY_SMIF_HbFlash_PasswordProtectionReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA* pwd0, CY_SMIF_FLASHDATA* pwd1, CY_SMIF_FLASHDATA* pwd2, CY_SMIF_FLASHDATA* pwd3);
extern void                 CY_SMIF_HbFlash_PasswordProtectionUnlockCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA pwd0, CY_SMIF_FLASHDATA pwd1, CY_SMIF_FLASHDATA pwd2, CY_SMIF_FLASHDATA pwd3);
extern void                 CY_SMIF_HbFlash_PasswordProtectionExitCmd( volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PpbEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_PpbProgramCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_PpbAllEraseCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_PpbStatusReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_PpbExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PpbLockBitEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PpbLockBitSetCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_PpbLockBitReadCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_PpbLockBitExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_DybEntryCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_DybSetCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_DybClrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_DybReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_DybExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_PpbLockBitReadOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern int                  CY_SMIF_HbFlash_PpbAllEraseOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_PpbStatusReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern int                  CY_SMIF_HbFlash_PpbProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern int                  CY_SMIF_HbFlash_PpbLockBitSetOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_LockRegBitsReadOp(volatile CY_SMIF_FLASHDATA* base_addr);
extern int                  CY_SMIF_HbFlash_LockRegBitsProgramOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA value);
extern void                 CY_SMIF_HbFlash_EnterDeepPowerDown(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_MeasureTemperature(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadTemperatureReg (volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ProgramPORTimerReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA portime);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadPORTimerReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LoadInterruptConfigReg (volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA icr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadInterruptConfigReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LoadInterruptStatusReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA isr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadInterruptStatusReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LoadVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr);
extern void                 CY_SMIF_HbFlash_LoadVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramVolatileConfigRegOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramVolatileConfigRegOp2(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA vcr);
extern void                 CY_SMIF_HbFlash_ProgramNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA nvcr);
extern void                 CY_SMIF_HbFlash_ProgramNonVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA nvcr);
extern void                 CY_SMIF_HbFlash_EraseNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadNonVolatileConfigReg(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadNonVolatileConfigReg2(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_HB_DEVSTATUS CY_SMIF_HbFlash_ProgramNonVolatileConfigRegOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_FLASHDATA nvcr);
extern void                 CY_SMIF_HbFlash_EvaluateEraseStatus(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_ECCStatusEnterCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ECCStatusReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ECCErrorLowerAddrRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ECCErrorUpperAddrRegReadCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ECCReadErrorDetectionCounterCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ECCClearErrorDetectionCounterCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_ECCExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ECCStatusReadOp(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_CRCEnterCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_LoadCRCStartAddrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_LoadCRCEndAddrCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_CRCSuspendCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_CRCArrayReadCmd(volatile CY_SMIF_FLASHDATA* base_addr, CY_SMIF_ADDRESS offset);
extern void                 CY_SMIF_HbFlash_CRCResumeCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadCheckvalueLowResultRegCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern CY_SMIF_FLASHDATA    CY_SMIF_HbFlash_ReadCheckvalueHighResultRegCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_CRCExitCmd(volatile CY_SMIF_FLASHDATA* base_addr);
extern void                 CY_SMIF_HbFlash_EnterSPIModeCmd(volatile CY_SMIF_FLASHDATA* base_addr);

/** \} group_mw_smif_functions */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CY_SMIF_HB_FLASH_H  */


