/***************************************************************************//**
* \file cy_smif_device_common.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provide common definition for specific external flash drivers.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_smif Middleware SMIF Memory Access (MW SMIF)
* \{
* This driver provides global MW SMIF defines and API function definitions.
*
* \defgroup group_mw_smif_macros Macro
* \defgroup group_mw_smif_functions Functions
* \defgroup group_mw_smif_data_structures Data structures
* \defgroup group_mw_smif_enums Enumerated types
*/

#include "cy_device_headers.h"
#include "cy_project.h"

#if !defined(CY_SMIF_DEVICE_COMMON_H)
#define CY_SMIF_DEVICE_COMMON_H

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mw_smif_enums
* \{
*/

/** Specifies external devices */
typedef enum
{
    CY_SMIF_S25FL     = 0u, /**< S25FLxxxL/S memory series */
    CY_SMIF_SEMP      = 1u, /**< Semper memory series */
    CY_SMIF_S25FS     = 2u, /**< S25FS  memory series */
    CY_SMIF_DEV_MAX,
} cy_en_smif_ex_device_type;

/** Specifies bit field of \ref cy_en_smif_bit_alloc_t */
typedef struct
{
    uint8_t Reserved0 : 1; /**< unused */
    uint8_t IsDualQuad: 1; /**< indicates the device is used in dual Quad mode */
    uint8_t width     : 2; /**< indicates data width */
    uint8_t dataSel   : 2; /**< indicates data connection between output and SMIF internal signal */
    uint8_t Reserved1 : 2; /**< unused */
} cy_stc_smif_device_param_field_t;

/** Specifies the data width and bit location of the device */
typedef enum
{
    CY_SMIF_DEV_OSPI            = 0x0C, /**< Octal bit 0 ~ 7 */
    CY_SMIF_DEV_D_QUAD_BIT_0To3 = 0x0A, /**< Dual Quad bit 0 ~ 3 */
    CY_SMIF_DEV_D_QUAD_BIT_4To7 = 0x2A, /**< Dual Quad bit 4 ~ 7 */
    CY_SMIF_DEV_QUAD_BIT_0To3   = 0x08, /**< Quad bit 0 ~ 3 */
    CY_SMIF_DEV_QUAD_BIT_4To7   = 0x28, /**< Quad bit 4 ~ 7 */
    CY_SMIF_DEV_DUAL_BIT_0To1   = 0x04, /**< Dual bit 0 ~ 1 */
    CY_SMIF_DEV_DUAL_BIT_2To3   = 0x14, /**< Dual bit 2 ~ 3 */
    CY_SMIF_DEV_DUAL_BIT_4To5   = 0x24, /**< Dual bit 4 ~ 5 */
    CY_SMIF_DEV_DUAL_BIT_6To7   = 0x34, /**< Dual bit 6 ~ 7 */
    CY_SMIF_DEV_SINGLE_BIT_0To1 = 0x00, /**< Single bit 0 ~ 1 */
    CY_SMIF_DEV_SINGLE_BIT_2To3 = 0x10, /**< Single bit 2 ~ 3 */
    CY_SMIF_DEV_SINGLE_BIT_4To5 = 0x20, /**< Single bit 4 ~ 5 */
    CY_SMIF_DEV_SINGLE_BIT_6To7 = 0x30, /**< Single bit 6 ~ 7 */
} cy_en_smif_bit_alloc_t;

/** Specifies blocking mode or non-blocking mode */
typedef enum
{
    CY_SMIF_BLOCKING     = 0u, /**< blocking mode */
    CY_SMIF_NON_BLOCKING = 1u, /**< non blocking mode */
} cy_en_smif_block_mode_t;

/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/** Specifies the data width and bit location of the device */
typedef union
{
    cy_en_smif_bit_alloc_t           en;   /**< \ref cy_en_smif_bit_alloc_t */
    cy_stc_smif_device_param_field_t bits; /**< \ref cy_stc_smif_device_param_field_t */
} cy_un_smif_bit_alloc_t;

/** Specifies device configuration parameter structure */
typedef struct
{
    uint16_t                       xipReadCmdId;       /**< read command ID specified by each external device */
    uint16_t                       xipReadMode;        /**< read mode */
    bool                           writeEn;            /**< write enabled or not */
    uint16_t                       xipWriteCmdId;      /**< write command ID specified by each external device */
    uint16_t                       xipWriteMode;       /**< write mode */
    bool                           mergeEnable;        /**< merge enable */
    cy_en_smif_dev_merge_timeout_t mergeTimeout;       /**< specifies merge timeout value \ref cy_en_smif_dev_merge_timeout_t */
    bool                           totalTimeoutEnable; /**< total timeout enable */
    uint16_t                       totalTimeout;       /**< total timeout in clk_mem cycles */
    uint32_t                       addr;               /**< base address of XIP mode */
    cy_en_device_size_t            size;               /**< size of device memory in XIP mode \ref cy_en_device_size_t */
    uint8_t                        rdLatencyCode;      /**< latency code of read */
    uint8_t                        wtLatencyCode;      /**< latency code of write */
} cy_stc_device_config_t;

/** Specifies Jedec ID structure */
typedef struct
{
    uint8_t ids[81];
} cy_stc_JedecId_t;

/** Read/Write Command Sequence Structure */
typedef struct
{
    cy_en_smif_txfr_width_t     width;    /**< width of command \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t      dataRate; /**< data rate of command \ref cy_en_smif_data_rate_t */
    cy_en_smif_field_presence_t presence; /**< command presence \ref cy_en_smif_field_presence_t */
} cy_stc_cmd_sq_t;

/** Read/Write Address Sequence Structure */
typedef struct
{
    cy_en_smif_txfr_width_t    width;    /**< width of address \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t     dataRate; /**< data rate of address \ref cy_en_smif_data_rate_t */
    cy_en_smif_xip_addr_byte_t addrLen;  /**< address length \ref cy_en_smif_xip_addr_byte_t */
} cy_stc_addr_sq_t;

/** Read/Write Mode Sequence Structure */
typedef struct
{
    cy_en_smif_txfr_width_t     width;    /**< width of mode \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t      dataRate; /**< data rate of mode \ref cy_en_smif_data_rate_t */
    cy_en_smif_field_presence_t presence; /**< mode presence \ref cy_en_smif_field_presence_t */
} cy_stc_mode_sq_t;

/** Read/Write Dummy Sequence Structure */
typedef struct
{
    uint8_t                     tableId;  /**< dummy table ID specified by each external device */
    cy_en_smif_txfr_width_t     width;    /**< width dummy \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t      dataRate; /**< data rate of dummy \ref cy_en_smif_data_rate_t */
    cy_en_smif_field_presence_t presence; /**< dummy presence \ref cy_en_smif_field_presence_t */
    bool                        doubleDm; /**< specifies whether an active RWDS refresh indicator (captured with the previous rising memory clock output) causes to double the number of dummy cycles. This is used for the variable latency in HyperRAM. */
    bool                        rwdsInDm; /**< specifies whether the RWDS output signal should be driven starting in the last dummy cycle until DESELECT. This is needed for write transactions with RWDS (e.g. Hyperbus) before a TX_COUNT command. */
    bool                        dlpEn;    /**< specifies whether data learning is enabled. In XIP mode this field is set for read transactions when CTL.INT_CLOCK_DL_ENABLED is "1"). */
    bool                        rwdsEn;   /**< Read-Write-Data-Strobe Enable. Specifies whether the RWDS output signal
                                               should be driven starting in the last dummy cycle until DESELECT.
                                               This is needed for write transactions with byte masking via RWDS (e.g. Hyperbus).
                                               '0': do not drive RWDS output
                                               '1': drive RWDS output starting in last dummy cycle until DESELECT */
} cy_stc_dummy_sq_t;

/** Read/Write Data Sequence Structure */
typedef struct
{
    cy_en_smif_txfr_width_t width;    /**< width data \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t  dataRate; /**< data rate of data \ref cy_en_smif_data_rate_t */
} cy_stc_data_sq_t;

/** Read Information Structure */
typedef struct
{
    uint16_t                        cmd; /**< command value */
    uint8_t                  cmdTypeNum; /**< command type ID. the type is specified in code dedicating to each external device */
    uint8_t                 addrTypeNum; /**< address type ID. the type is specified in code dedicating to each external device */
    uint8_t                 modeTypeNum; /**< mode type ID. the type is specified in code dedicating to each external device */
    uint8_t                dummyTypeNum; /**< dummy type ID. the type is specified in code dedicating to each external device */
    uint8_t                 dataTypeNum; /**< data type ID. the type is specified in code dedicating to each external device */
} cy_stc_read_info_t;

/** Write Information Structure */
typedef struct
{
    uint16_t                        cmd; /**< command value */
    uint8_t                  cmdTypeNum; /**< command type ID. the type is specified in code dedicating to each external device */
    uint8_t                 addrTypeNum; /**< address type ID. the type is specified in code dedicating to each external device */
    uint8_t                 modeTypeNum; /**< mode type ID. the type is specified in code dedicating to each external device */
    uint8_t                dummyTypeNum; /**< dummy type ID. the type is specified in code dedicating to each external device */
    uint8_t                 dataTypeNum; /**< data type ID. the type is specified in code dedicating to each external device */
} cy_stc_write_info_t;

/** User Context Structure */
typedef struct
{
    cy_stc_cmd_sq_t*          p_readCmdTypes;   /**< pointer to read command type specified external device code */
    cy_stc_addr_sq_t*         p_readAddrTypes;  /**< pointer to read address type specified external device code */
    cy_stc_mode_sq_t*         p_readModeTypes;  /**< pointer to read mode type specified external device code */
    cy_stc_dummy_sq_t*        p_readDummyTypes; /**< pointer to read dummy type specified external device code */
    uint8_t**                 pp_readLatencyCodeToDummyCycle; /**< pointer to top address of read latency code table */
    cy_stc_data_sq_t*         p_readDataTypes;  /**< pointer to read data type specified external device code */
    cy_stc_read_info_t*       p_readCmdInfo;    /**< pointer to read command information table specified external device code */
    cy_stc_cmd_sq_t*          p_writeCmdTypes;  /**< pointer to write command type specified external device code */
    cy_stc_addr_sq_t*         p_writeAddrTypes; /**< pointer to write address type specified external device code */
    cy_stc_mode_sq_t*         p_writeModeTypes; /**< pointer to write mode type specified external device code */
    cy_stc_dummy_sq_t*        p_writeDummyTypes;/**< pointer to write dummy type specified external device code */
    uint8_t**                 pp_writeLatencyCodeToDummyCycle; /**< pointer to top address of write latency code table */
    cy_stc_data_sq_t*         p_writeDataTypes; /**< pointer to write data type specified external device code */
    cy_stc_write_info_t*      p_writeCmdInfo;   /**< pointer to write command information table specified external device code */
    uint32_t                  readLatencyCodeMax; /**< number of read latency code table */
    uint32_t                  writeLatencyCodeMax;/**< number of write latency code table */
    uint32_t                  readCommandNum;    /**< number of read command information table */
    uint32_t                  writeCommandNum;   /**< number of write command information table */
    cy_en_smif_ex_device_type currentExDevice;   /**< external device type which has been set */
} cy_stc_ex_dev_context_t;

/** bit field for defining command parameters */
typedef struct
{
    uint32_t cmdVal      :16; /**< Command value 16 bit */
    uint32_t addrLen     : 3; /**< Address length  */
    uint32_t addrCfgAble : 1; /**< Indicating Address length fixed or not */
    uint32_t rwdsCapEn   : 1; /**< Use RWDS or not */
    uint32_t latTblNo    : 3; /**< Indicating Latency Table No */
    uint32_t constLatNum : 8; /**< Indicate Latency number when no table */
} cy_stc_smif_trx_cmd_t;

/** Union for defining command parameters */
typedef union
{
    uint32_t              cmd; /**< 32 bit access */
    cy_stc_smif_trx_cmd_t bf;  /**< Bit field access \ref cy_stc_smif_trx_cmd_t */
} cy_un_smif_trx_cmd_t;

/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SPI_INVALID_CMD (0x00000000UL)

/** Manufacturer ID for Cypress which can be read out from connected memories */
#define CY_SMIF_CYPRESS_MANUFACTURER_ID     (0x34)

/** \} group_mw_smif_macros */

/**
* \addtogroup group_mw_smif_enums
* \{
*/

/** Specifies SPI transmission address length */
typedef enum
{ 
    CY_TRX_ADDR_3BYTE = 0, /**< 3 byte address length */
    CY_TRX_ADDR_4BYTE = 1, /**< 4 byte address length */
} cy_en_smif_trx_addr_len_t;

/** Specifies SPI transmission type for transmission other than flash read/write */
/** The bit field meaning is described by \ref cy_stc_smif_trx_type_t */
typedef enum
{
    //                             RWDS Enable| 2 byte Command|               transaction width|             data rate| Transaction No.
    CY_SPI_TRANSACTION_1S1S1S = (0ul << 25ul) | (0ul << 24ul) | (CY_SMIF_WIDTH_SINGLE << 16ul) | (CY_SMIF_SDR << 8ul) | 0u, /**< Legacy SPI transmission  */
    CY_SPI_TRANSACTION_4S4S4S = (0ul << 25ul) | (0ul << 24ul) |   (CY_SMIF_WIDTH_QUAD << 16ul) | (CY_SMIF_SDR << 8ul) | 1u, /**< 4S4S4S transmission */
    CY_SPI_TRANSACTION_8S8S8S = (1ul << 25ul) | (1ul << 24ul) |  (CY_SMIF_WIDTH_OCTAL << 16ul) | (CY_SMIF_SDR << 8ul) | 2u, /**< 8S8S8S transmission */
    CY_SPI_TRANSACTION_8D8D8D = (1ul << 25ul) | (1ul << 24ul) |  (CY_SMIF_WIDTH_OCTAL << 16ul) | (CY_SMIF_DDR << 8ul) | 3u, /**< 8D8D8D transmission */
} cy_en_smif_trx_type_t;

/** \} group_mw_smif_enums */

/**
* \addtogroup group_mw_smif_macros
* \{
*/
#define CY_SPI_TRANSACTION_TYPE_NUM (4)
/** \} group_mw_smif_macros */
/**
* \addtogroup group_mw_smif_data_structures
* \{
*/

/** Bit field describing \ref cy_en_smif_trx_type_t */
typedef struct
{
    uint32_t tblNo    : 8; /**< table No. which is specific to each transaction e.g. 1S1S1S = 0.*/
    uint32_t dataRate : 8; /**< SDR or DDR */
    uint32_t width    : 8; /**< data width of transition */
    uint32_t isCmd2B  : 1; /**< command size. 1: 2 byte, 0: 1 byte */
    uint32_t RwdsCapEn: 1; /**< RWDS capture enable or not. 1: enabled, 0: disabled */
} cy_stc_smif_trx_type_t;

/** Union for defining transaction type */
typedef union
{
    cy_en_smif_trx_type_t   en;
    cy_stc_smif_trx_type_t  bf;
} cy_un_smif_trx_type_t;

/** \} group_mw_smif_data_structures */

/**
* \addtogroup group_mw_smif_functions
* \{
*/


cy_en_smif_status_t Cy_SMIF_InitExMemContext(cy_en_smif_ex_device_type exDevice, cy_stc_ex_dev_context_t* context);
cy_en_smif_status_t Cy_SMIF_InitXIPModeForExMem(volatile cy_stc_smif_reg_device_t *device,
                                                cy_en_smif_bit_alloc_t bitAllocation,
                                                const cy_stc_device_config_t *config,
                                                const cy_stc_ex_dev_context_t* context);
cy_en_smif_status_t Cy_SMIF_MMIO_Read_ExMem(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t readCmdId,
                                        uint32_t readAddress,
                                        uint32_t size,
                                        uint8_t buf[],
                                        uint8_t lcCode,
                                        uint8_t mode,
                                        cy_en_smif_block_mode_t blockingMode,
                                        const cy_stc_ex_dev_context_t* devContext, 
                                        cy_stc_smif_context_t *context);
cy_en_smif_status_t Cy_SMIF_MMIO_Program_ExMem(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        uint16_t writeCmdId,
                                        uint32_t programedAddress,
                                        uint32_t size,
                                        const uint8_t buf[],
                                        cy_en_smif_block_mode_t blockingMode,
                                        uint8_t lcCode,
                                        uint8_t mode,
                                        const cy_stc_ex_dev_context_t* devContext,
                                        cy_stc_smif_context_t *context);

cy_en_smif_status_t Cy_SMIF_TX_CMD(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context);

cy_en_smif_status_t Cy_SMIF_TX_CMD_ADDR(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context);

cy_en_smif_status_t Cy_SMIF_TX_READ(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        uint8_t  readData[],
                                        uint32_t readSize,
                                        uint8_t latCode,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context);


cy_en_smif_status_t Cy_SMIF_TX_WRITE(volatile cy_stc_smif_reg_t *base,
                                        cy_en_smif_slave_select_t slave,
                                        cy_en_smif_trx_type_t en_trx,
                                        uint32_t en_cmd,
                                        cy_en_smif_trx_addr_len_t addrLen,
                                        uint32_t addr,
                                        uint8_t  writeData[],
                                        uint32_t writeSize,
                                        const uint8_t** cmdLatTbls,
                                        cy_stc_smif_context_t *context);

/** \} group_mw_smif_functions */

#include "cy_smif_s25fl.h"
#include "cy_smif_semp.h"
#include "cy_smif_s25fs.h"

#if defined(__cplusplus)
}
#endif


#endif // CY_SMIF_DEVICE_COMMON_H
