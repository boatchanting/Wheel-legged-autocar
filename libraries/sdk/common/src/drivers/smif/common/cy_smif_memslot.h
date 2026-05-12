/***************************************************************************//**
* \file cy_smif_memslot.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`.1
*
* \brief
*  This file provides the constants and parameter values for the memory-level
*  APIs of the SMIF driver.
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

#if !defined(CY_SMIF_MEMORYSLOT_H)
#define CY_SMIF_MEMORYSLOT_H

#include <stdint.h>
#include <stdbool.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"
#include "cy_project.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
* \addtogroup group_smif_macros_status
* \{
*/

/***************************************
*            Constants
****************************************/
#define CY_SMIF_DEVICE_BUSY        (1U)    /**< The external memory is busy */
#define CY_SMIF_DEVICE_READY       (0U)    /**< The external memory is ready */

/** \} group_smif_macros_status */

/**
* \addtogroup group_smif_macros_cmd
* \{
*/
#define CY_SMIF_CMD_WITHOUT_PARAM  (0U)    /**< No parameter */
#define CY_SMIF_TX_LAST_BYTE       (1U)    /**< The last byte in the command transmission
                                            * (SS is set high after the transmission)
                                            */
#define CY_SMIF_TX_NOT_LAST_BYTE   (0U)     /**< Not the last byte in the command transmission
                                            * (SS remains low after the transmission)
                                            */
#define CY_SMIF_READ_ONE_BYTE      (1U)     /**< Read 1 byte */
#define CY_SMIF_WRITE_ONE_BYTE     (1U)     /**< Write 1 byte */
#define CY_SMIF_WRITE_TWO_BYTES    (2U)     /**< Write 2 bytes */
#define CY_SMIF_ONE_WORD           (4U)     /**< 4 bytes */

#define CY_SMIF_DUAL_QUAD_DISABLED (0U)     /**< The dual quad transmission mode is disabled */
#define CY_SMIF_DUAL_QUAD_ENABLED  (1U)     /**< The dual quad transmission mode is enabled */


/** \} group_smif_macros_status */

/**
* \addtogroup group_smif_macros_flags
* \{
*/

#define CY_SMIF_FLAG_ALL_DISABLED       (0U) /**< All memory configuration flags are disabled */
/** Enables the write capability for the memory slave in the memory-mapped
 * mode. Valid when the memory-mapped mode is enabled */
#define CY_SMIF_FLAG_WR_EN              (SMIF_DEVICE_CTL_WR_EN_Msk)
/** Determines if the device is memory-mapped. If enabled, this memory slot will
 * be initialized in System init */
#define CY_SMIF_FLAG_MEMORY_MAPPED      (2U)
#define CY_SMIF_FLAG_DETECT_SFDP        (4U) /**< Enables the Autodetect using the SFDP */
/** Enables the crypto support for this memory slave. All access to the
* memory device goes through the encryption/decryption
* Valid when the memory-mapped mode is enabled */
#define CY_SMIF_FLAG_CRYPTO_EN          (SMIF_DEVICE_CTL_CRYPTO_EN_Msk)

/** \} group_smif_macros_flags */

/**
* \addtogroup group_smif_data_structures_memslot
* \{
*/

/** This structure is used to store the read dummy configuration. */
typedef struct
{
    stc_cy_smif_rd_dummy_ctl_t dummyCtl; /**< specifies existence of dummy and the number */
    bool doubleDm;                       /**< specifies whether an active RWDS refresh indicator (captured with the previous rising memory clock output) causes to double the number of dummy cycles. This is used for the variable latency in HyperRAM. */
    bool rwdsInDm;                       /**< specifies whether the RWDS output signal should be driven starting in the last dummy cycle until DESELECT. This is needed for write transactions with RWDS (e.g. Hyperbus) before a TX_COUNT command. */
    bool dlpEn;                          /**< specifies whether data learning is enabled. In XIP mode this field is set for read transactions when CTL.INT_CLOCK_DL_ENABLED is "1"). */
    bool capEn;                          /**< specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle, i.e. this needs to be set to '1' for read transactions to memory device using RWDS / DQS capturing scheme. */
} stc_smif_mem_rd_dm_ctl_t;

/** This structure is used to store the write dummy configuration. */
typedef struct
{
    stc_cy_smif_wr_dummy_ctl_t dummyCtl; /**< specifies existence of dummy and the number */
    bool doubleDm;                       /**< specifies whether an active RWDS refresh indicator (captured with the previous rising memory clock output) causes to double the number of dummy cycles. This is used for the variable latency in HyperRAM. */
    bool rwdsInDm;                       /**< specifies whether the RWDS output signal should be driven starting in the last dummy cycle until DESELECT. This is needed for write transactions with RWDS (e.g. Hyperbus) before a TX_COUNT command. */
    bool dlpEn;                          /**< specifies whether data learning is enabled. In XIP mode this field is set for read transactions when CTL.INT_CLOCK_DL_ENABLED is "1"). */
    bool capEn;                          /**< specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle, i.e. this needs to be set to '1' for read transactions to memory device using RWDS / DQS capturing scheme. */
} stc_smif_mem_wr_dm_ctl_t;

/** This structure stores sequence for one read command specified by external device. */
typedef struct
{
    stc_cy_smif_cmd_mode_ctl_t  cmdCtl;       /**< RX CMD control in XIP mode */
    stc_cy_smif_addr_data_ctl_t addrCtl;      /**< RX ADDR control in XIP mode */
    stc_cy_smif_cmd_mode_ctl_t  modeCtl;      /**< RX MODE control in XIP mode */
    stc_smif_mem_rd_dm_ctl_t    memDummyCtl;  /**< RX DUMMY control in XIP mode */
    stc_cy_smif_addr_data_ctl_t dataCtl;      /**< RX DATA control in XIP mode */
    stc_cy_smif_rd_crc_ctl_t    crcCtl;       /**< RX CRC control in XIP mode */
    stc_cy_smif_rd_bound_ctl_t  boundCtl;     /**< RX BOUND control in XIP mode */
} cy_stc_smif_mem_rd_cmd_t;

/** This structure stores sequence for one write command specified by external device. */
typedef struct
{
    stc_cy_smif_cmd_mode_ctl_t  cmdCtl;     /**< TX CMD control in XIP mode */
    stc_cy_smif_addr_data_ctl_t addrCtl;    /**< TX ADDR control in XIP mode */
    stc_cy_smif_cmd_mode_ctl_t  modeCtl;    /**< TX MODE control in XIP mode */
    stc_smif_mem_wr_dm_ctl_t    memDummyCtl;/**< TX DUMMY control in XIP mode */
    stc_cy_smif_addr_data_ctl_t dataCtl;    /**< TX DATA control in XIP mode */
    stc_cy_smif_wr_crc_ctl_t    crcCtl;     /**< TX CRC control in XIP mode */
} cy_stc_smif_mem_wr_cmd_t;


/** This structure stores sequence for one command specified by external device. */
typedef struct
{
    stc_cy_smif_cmd_mode_ctl_t cmdCtl; /**< TX CMD control */
} cy_stc_smif_mem_cmd_t;


/**
*
* This configuration structure of the SMIF memory device is used to store
* device-specific parameters.
* These parameters are used to set up the memory mode initialization and the
* memory API.
*/
typedef struct
{
    bool isCommand2Byte;
    cy_en_smif_xip_addr_byte_t numOfAddrBytes;     /**< This specifies the number of address bytes used by the 
                                                    * memory slave device, valid values 1-4 */
    cy_stc_smif_mem_rd_cmd_t* readCmd;             /**< This specifies the Read command */
    cy_stc_smif_mem_cmd_t* writeEnCmd;       /**< This specifies the Write Enable command */
    cy_stc_smif_mem_cmd_t* writeDisCmd;      /**< This specifies the Write Disable command */
    cy_stc_smif_mem_cmd_t* eraseCmd;         /**< This specifies the Erase command */
    cy_stc_smif_mem_cmd_t* chipEraseCmd;     /**< This specifies the Chip Erase command */
    cy_stc_smif_mem_wr_cmd_t* programCmd;          /**< This specifies the Program command */
    uint32_t programSize;                          /**< This specifies the page size for programming */
    cy_stc_smif_mem_cmd_t* readStsRegWipCmd; /**< This specifies the command to read the WIP-containing status register  */
    cy_stc_smif_mem_cmd_t* readStsRegQeCmd;  /**< This specifies the command to read the QE-containing status register */
    cy_stc_smif_mem_cmd_t* writeStsRegQeCmd; /**< This specifies the command to write into the QE-containing status register */
    uint32_t stsRegBusyMask;                       /**< The Busy mask for the status registers */
    uint32_t stsRegQuadEnableMask;                 /**< The QE mask for the status registers */
    uint32_t stsRegProtectionMask;
} cy_stc_smif_mem_device_cfg_t;


/**
*
* This SMIF memory configuration structure is used to store the memory configuration for the memory mode of operation.
* This data structure is stored in a fixed location in the flash. The data structure is required
* for the initialization of the SMIF in the SystemInit.
*/
typedef struct
{
    /** Determines the slave select where the memory device is placed */
    cy_en_smif_slave_select_t slaveSelect;
    /** Determines if the device is memory-mapped, enables the Autodetect
     * using the SFDP, enables the write capability, or enables the crypto
     * support for this memory slave */
    bool writeEn;
    bool crypteEn;
    bool XIPSettingEn;
    /** The data-line selection options for a slave device */
    cy_en_smif_data_select_t dataSelect;
    /** The base address the memory slave is mapped to in the PSoC memory map.
     * This address must be a multiple of the SMIF XIP memory size/capacity. The
     * SMIF XIP memory region should NOT overlap with other memory regions
     * (with exception to dual quad mode). Valid when the memory-mapped mode is
     * enabled.
     */
    uint32_t baseAddress;
    /** The size/capacity allocated in the PSoC memory map for the memory slave
     * device. The capacity is allocated from the base address. The capacity
     * must be a power of 2 and greater or equal than 64 KB. Valid when
     * memory-mapped mode is enabled
     */
    cy_en_device_size_t memMappedSize;
    /** Defines if this memory device is one of the devices in the dual quad SPI
     * configuration. Equals the sum of the slave-slot numbers.  */
    uint32_t dualQuadSlots;
    cy_stc_smif_mem_device_cfg_t *deviceCfg;   /**< The configuration of the device */
} cy_stc_smif_mem_config_t;


/**
*
* This SMIF memory configuration structure is used to store the memory configuration for the memory mode of operation.
* This data structure is stored in a fixed location in the flash. The data structure is required
* for the initialization of the SMIF in the SystemInit.
*/
typedef struct
{
    uint32_t memCount;                    /**< The number of SMIF memory defined  */
    cy_stc_smif_mem_config_t **memConfig; /**< The pointer to the array of the memory configuration structures of size Memory_count */
    uint32_t majorVersion;                /**< The version of the SMIF driver */
    uint32_t minorVersion;                /**< The version of the SMIF driver */
} cy_stc_smif_block_config_t;


/** \} group_smif_data_structures_memslot */

/**
* \addtogroup group_smif_mem_slot_functions
* \{
*/
cy_en_smif_status_t Cy_SMIF_Memslot_Init(cy_stc_smif_reg_t volatile *base,
                            const cy_stc_smif_block_config_t* blockConfig,
                            cy_stc_smif_context_t *context);
void        Cy_SMIF_Memslot_DeInit(cy_stc_smif_reg_t volatile *base);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteEnable(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t const *memDevice,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t    Cy_SMIF_Memslot_CmdWriteDisable(cy_stc_smif_reg_t volatile *base,
                                         cy_stc_smif_mem_config_t const *memDevice,
                                         cy_stc_smif_context_t *context);
bool Cy_SMIF_Memslot_IsBusy(cy_stc_smif_reg_t volatile *base, cy_stc_smif_mem_config_t *memDevice,
                                    cy_stc_smif_context_t *context);
cy_en_smif_status_t    Cy_SMIF_Memslot_QuadEnable(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t *memDevice,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_BlockProtNone(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t *memDevice,
                                    cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdReadSts(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    uint8_t *status,
                                    uint16_t command,
                                    cy_en_smif_txfr_width_t dataWidth,
                                    cy_en_smif_data_rate_t dataRate,
                                    cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdWriteSts(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    void const *status,
                                    uint8_t command,
                                    cy_en_smif_txfr_width_t dataWidth,
                                    cy_en_smif_data_rate_t dataRate,
                                    cy_stc_smif_context_t *context);
cy_en_smif_status_t    Cy_SMIF_Memslot_CmdChipErase(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t const *memDevice,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdSectorErase(cy_stc_smif_reg_t volatile *base,
                                        cy_stc_smif_mem_config_t *memDevice,
                                        uint32_t sectorAddr,
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdProgram(cy_stc_smif_reg_t volatile *base,
                                    cy_stc_smif_mem_config_t const *memDevice,
                                    uint32_t deviceAddr,
                                    uint8_t* writeBuff,
                                    uint32_t size,
                                    cy_smif_event_cb_t cmdCmpltCb,
                                    cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_Memslot_CmdRead(cy_stc_smif_reg_t volatile *base,
                                cy_stc_smif_mem_config_t const *memDevice,
                                uint32_t deviceAddr,
                                uint8_t* readBuff,
                                uint32_t size,
                                cy_smif_event_cb_t cmdCmpltCb,
                                cy_stc_smif_context_t *context);


/** \} group_smif_mem_slot_functions */


#if defined(__cplusplus)
}
#endif

#endif /* (CY_SMIF_MEMORYSLOT_H) */


/* [] END OF FILE */
