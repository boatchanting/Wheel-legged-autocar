/***************************************************************************//**
* \file cy_smif_ver_specific.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress SMIF v4.0 driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \section group_smif_ver_specific_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
*/

#if !defined(CY_SMIF_VER_SPECIFIC_H)
#define CY_SMIF_VER_SPECIFIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"

#ifdef CY_IP_MXSMIF

#if defined(__cplusplus)
extern "C" {
#endif

/* Typedef for smif driver, so make it common */
typedef stc_SMIF_t                                  cy_stc_smif_base_reg_t;

typedef stc_SMIF_CORE_t                             cy_stc_smif_reg_t;
typedef un_SMIF_CORE_CTL_t                          cy_un_smif_reg_ctl_t;
typedef un_SMIF_CORE_STATUS_t                       cy_un_smif_reg_status_t;
typedef un_SMIF_CORE_CTL2_t                         cy_un_smif_reg_ctl2_t;
typedef un_SMIF_CORE_DLP_DELAY_TAP_SEL0_t           cy_un_smif_reg_dlp_delay_tap_sel0_t;
typedef un_SMIF_CORE_DLP_DELAY_TAP_SEL1_t           cy_un_smif_reg_dlp_delay_tap_sel1_t;
typedef un_SMIF_CORE_DLP_CTL_t                      cy_un_smif_reg_dlp_ctl_t;
typedef un_SMIF_CORE_DLP_STATUS0_t                  cy_un_smif_reg_dlp_status0_t;
typedef un_SMIF_CORE_DLP_STATUS1_t                  cy_un_smif_reg_dlp_status1_t;
typedef un_SMIF_CORE_TX_CMD_FIFO_STATUS_t           cy_un_smif_reg_tx_cmd_fifo_status_t;
typedef un_SMIF_CORE_TX_CMD_MMIO_FIFO_STATUS_t      cy_un_smif_reg_tx_cmd_mmio_fifo_status_t;
typedef un_SMIF_CORE_TX_CMD_MMIO_FIFO_WR_t          cy_un_smif_reg_tx_cmd_mmio_fifo_wr_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_CTL_t        cy_un_smif_reg_tx_data_mmio_fifo_ctl_t;
typedef un_SMIF_CORE_TX_DATA_FIFO_STATUS_t          cy_un_smif_reg_tx_data_fifo_status_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_STATUS_t     cy_un_smif_reg_tx_data_mmio_fifo_status_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_WR1_t        cy_un_smif_reg_tx_data_mmio_fifo_wr1_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_WR2_t        cy_un_smif_reg_tx_data_mmio_fifo_wr2_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_WR4_t        cy_un_smif_reg_tx_data_mmio_fifo_wr4_t;
typedef un_SMIF_CORE_TX_DATA_MMIO_FIFO_WR1ODD_t     cy_un_smif_reg_tx_data_mmio_fifo_wr1odd_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_CTL_t        cy_un_smif_reg_rx_data_mmio_fifo_ctl_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_STATUS_t     cy_un_smif_reg_rx_data_mmio_fifo_status_t;
typedef un_SMIF_CORE_RX_DATA_FIFO_STATUS_t          cy_un_smif_reg_rx_data_fifo_status_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_RD1_t        cy_un_smif_reg_rx_data_mmio_fifo_rd1_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_RD2_t        cy_un_smif_reg_rx_data_mmio_fifo_rd2_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_RD4_t        cy_un_smif_reg_rx_data_mmio_fifo_rd4_t;
typedef un_SMIF_CORE_RX_DATA_MMIO_FIFO_RD1_SILENT_t cy_un_smif_reg_rx_data_mmio_fifo_rd1_silent_t;
typedef un_SMIF_CORE_SLOW_CA_CTL_t                  cy_un_smif_reg_slow_ca_ctl_t;
typedef un_SMIF_CORE_SLOW_CA_CMD_t                  cy_un_smif_reg_slow_ca_cmd_t;
typedef un_SMIF_CORE_FAST_CA_CTL_t                  cy_un_smif_reg_fast_ca_ctl_t;
typedef un_SMIF_CORE_FAST_CA_CMD_t                  cy_un_smif_reg_fast_ca_cmd_t;
typedef un_SMIF_CORE_CRC_CMD_t                      cy_un_smif_reg_crc_cmd_t;
typedef un_SMIF_CORE_CRC_INPUT0_t                   cy_un_smif_reg_crc_input0_t;
typedef un_SMIF_CORE_CRC_INPUT1_t                   cy_un_smif_reg_crc_input1_t;
typedef un_SMIF_CORE_CRC_OUTPUT_t                   cy_un_smif_reg_crc_output_t;
typedef un_SMIF_CORE_INTR_t                         cy_un_smif_reg_intr_t;
typedef un_SMIF_CORE_INTR_SET_t                     cy_un_smif_reg_intr_set_t;
typedef un_SMIF_CORE_INTR_MASK_t                    cy_un_smif_reg_intr_mask_t;
typedef un_SMIF_CORE_INTR_MASKED_t                  cy_un_smif_reg_intr_masked_t;

typedef stc_SMIF_CORE_DEVICE_t                      cy_stc_smif_reg_device_t;
typedef un_SMIF_CORE_DEVICE_CTL_t                   cy_un_smif_reg_device_ctl_t;
typedef un_SMIF_CORE_DEVICE_ADDR_t                  cy_un_smif_reg_device_addr_t;
typedef un_SMIF_CORE_DEVICE_MASK_t                  cy_un_smif_reg_device_mask_t;
typedef un_SMIF_CORE_DEVICE_ADDR_CTL_t              cy_un_smif_reg_device_addr_ctl_t;
typedef un_SMIF_CORE_DEVICE_RX_CAPTURE_CONFIG_t     cy_un_smif_reg_device_capture_config_t;
typedef un_SMIF_CORE_DEVICE_RD_STATUS_t             cy_un_smif_reg_device_rd_status_t;
typedef un_SMIF_CORE_DEVICE_RD_CMD_CTL_t            cy_un_smif_reg_device_rd_cmd_ctl_t;
typedef un_SMIF_CORE_DEVICE_RD_ADDR_CTL_t           cy_un_smif_reg_device_rd_addr_ctl_t;
typedef un_SMIF_CORE_DEVICE_RD_MODE_CTL_t           cy_un_smif_reg_device_rd_mode_ctl_t;
typedef un_SMIF_CORE_DEVICE_RD_DUMMY_CTL_t          cy_un_smif_reg_device_rd_dummy_t;
typedef un_SMIF_CORE_DEVICE_RD_DATA_CTL_t           cy_un_smif_reg_device_rd_data_ctl_t;
typedef un_SMIF_CORE_DEVICE_RD_CRC_CTL_t            cy_un_smif_reg_device_rd_crc_ctl_t;
typedef un_SMIF_CORE_DEVICE_RD_BOUND_CTL_t          cy_un_smif_reg_device_rd_bound_ctl_t;
typedef un_SMIF_CORE_DEVICE_WR_CMD_CTL_t            cy_un_smif_reg_device_wr_cmd_t;
typedef un_SMIF_CORE_DEVICE_WR_ADDR_CTL_t           cy_un_smif_reg_device_wr_addr_t;
typedef un_SMIF_CORE_DEVICE_WR_MODE_CTL_t           cy_un_smif_reg_device_wr_mode_ctl_t;
typedef un_SMIF_CORE_DEVICE_WR_DUMMY_CTL_t          cy_un_smif_reg_device_wr_dummy_ctl_t;
typedef un_SMIF_CORE_DEVICE_WR_DATA_CTL_t           cy_un_smif_reg_device_wr_data_ctl_t;
typedef un_SMIF_CORE_DEVICE_WR_CRC_CTL_t            cy_un_smif_reg_device_wr_crc_ctl_t;

typedef stc_SMIF_CORE_SMIF_CRYPTO_t                 cy_stc_smif_reg_crypto_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_CMD_t       cy_un_smif_reg_crypto_cmd_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_ADDR_t      cy_un_smif_reg_crypto_addr_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_MASK_t      cy_un_smif_reg_crypto_mask_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_SUBREGION_t cy_un_smif_reg_crypto_subregion_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_INPUT0_t    cy_un_smif_reg_crypto_input0_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_INPUT1_t    cy_un_smif_reg_crypto_input1_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_INPUT2_t    cy_un_smif_reg_crypto_input2_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_INPUT3_t    cy_un_smif_reg_crypto_input3_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_KEY0_t      cy_un_smif_reg_crypto_key0_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_KEY1_t      cy_un_smif_reg_crypto_key1_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_KEY2_t      cy_un_smif_reg_crypto_key2_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_KEY3_t      cy_un_smif_reg_crypto_key3_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_OUTPUT0_t   cy_un_smif_reg_crypto_output0_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_OUTPUT1_t   cy_un_smif_reg_crypto_output1_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_OUTPUT2_t   cy_un_smif_reg_crypto_output2_t;
typedef un_SMIF_CORE_SMIF_CRYPTO_CRYPTO_OUTPUT3_t   cy_un_smif_reg_crypto_output3_t;

typedef stc_SMIF_SMIF_BRIDGE_t                      cy_stc_smif_reg_bridge_t;
typedef un_SMIF_SMIF_BRIDGE_CTL_t                   cy_un_smif_reg_bridge_ctl_t;

typedef stc_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_t            cy_stc_smif_reg_bridge_region_t;
typedef un_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_CTL_t         cy_stc_smif_reg_bridge_region_ctl_t;
typedef un_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_ADDR_t        cy_stc_smif_reg_bridge_region_addr_t;
typedef un_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_MASK_t        cy_stc_smif_reg_bridge_region_mask_t;
typedef un_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_SMIF0_REMAP_t cy_stc_smif_reg_bridge_region_smif0_remap_t;
typedef un_SMIF_SMIF_BRIDGE_SMIF_REMAP_REGION_SMIF1_REMAP_t cy_stc_smif_reg_bridge_region_smif1_remap_t;


/* Definitions for smif device parameters */
#define CY_SMIF_CORE_COUNT               (2ul)
#define CY_SMIF_BRIGE_REMAP_REGION_COUNT (8ul)

/* Definitions for smif register masks, so make it common (only for required entities) */
#define CY_SMIF_DRV_DEVICE_ADDR_MSK                      SMIF_CORE_DEVICE_ADDR_ADDR_Msk
#define CY_SMIF_DRV_INTR_TR_TX_REQ_MSK                   SMIF_CORE_INTR_TR_TX_REQ_Msk
#define CY_SMIF_DRV_INTR_TR_RX_REQ_MSK                   SMIF_CORE_INTR_TR_RX_REQ_Msk
#define CY_SMIF_DRV_INTR_XIP_ALIGNMENT_ERROR_MSK         SMIF_CORE_INTR_XIP_ALIGNMENT_ERROR_Msk
#define CY_SMIF_DRV_INTR_TX_CMD_FIFO_OVERFLOW_MSK        SMIF_CORE_INTR_TX_CMD_FIFO_OVERFLOW_Msk
#define CY_SMIF_DRV_INTR_TX_DATA_FIFO_OVERFLOW_MSK       SMIF_CORE_INTR_TX_DATA_FIFO_OVERFLOW_Msk
#define CY_SMIF_DRV_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_MSK SMIF_CORE_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk

/* Definitions for smif register top address, so make it common */
#if defined(SMIF0_CORE0)
  #define CY_SMIF_DRV_SMIF0_CORE0                         SMIF0_CORE0
#endif

#if defined(SMIF0_CORE1)
  #define CY_SMIF_CORE1_EXISTS
  #define CY_SMIF_DRV_SMIF0_CORE1                         SMIF0_CORE1
#endif

#if defined(SMIF0_CORE0_DEVICE0)
  #define CY_SMIF_DRV_SMIF0_CORE0_DEVICE0                 SMIF0_CORE0_DEVICE0
#endif

#if defined(SMIF0_CORE0_DEVICE1)
  #define CY_SMIF_DRV_SMIF0_CORE0_DEVICE1                 SMIF0_CORE0_DEVICE1
#endif

#if defined(SMIF0_CORE1_DEVICE0)
  #define CY_SMIF_DRV_SMIF0_CORE1_DEVICE0                 SMIF0_CORE1_DEVICE0
#endif

#if defined(SMIF0_CORE1_DEVICE1)
  #define CY_SMIF_DRV_SMIF0_CORE1_DEVICE1                 SMIF0_CORE1_DEVICE1
#endif

#if defined(SMIF_CORE_DEVICE_SECTION_SIZE)
  #define CY_SMIF_DRV_DEVICE_SECTION_SIZE                 SMIF_CORE_DEVICE_SECTION_SIZE
#endif

#define CY_SMIF_DRV_SMIF0_IRQN                            smif_0_smif0_interrupt_IRQn

#if defined(CY_SMIF_CORE1_EXISTS)
  #define CY_SMIF_DRV_SMIF1_IRQN                          smif_0_smif1_interrupt_IRQn
#endif

/* Set path of the PLL clock frequency that feeds the DLL */
typedef enum
{
    CY_SMIF_CLKIF_SRC_CLK_PLL_NORMAL = 0,    /**< clk_pll_normal is used to create clk_if */
    //CY_SMIF_CLKIF_SRC_CLK_PLL_DIRECT = 1   /**< clk_pll_direct is used to create clk_if (not for customer use!) */
} cy_en_cy_clkif_source_t;

/** The SMIF configuration structure. */
/* Set according to the PLL clock frequency that feeds the DLL */
typedef enum
{
    CY_SMIF_DLL_SRC_PLL_FREQ_160_To_180MHz = 0, /**< 160 to 180 MHz */
    CY_SMIF_DLL_SRC_PLL_FREQ_180_To_266MHz = 1, /**< >180 to 266 MHz */
    CY_SMIF_DLL_SRC_PLL_FREQ_266_To_333MHz = 2, /**< >266 to 333 MHz */
    CY_SMIF_DLL_SRC_PLL_FREQ_333_To_400MHz = 3, /**< >333 to 400 MHz */
} cy_en_cy_smif_dll_speed_mode_t;

/* The clock out to the memory is a divided version of the MDL clock out */
typedef enum
{
    CY_SMIF_MDL_CLK_OUT_DIV2  = 0, /**< Divided by 2 */
    CY_SMIF_MDL_CLK_OUT_DIV4  = 1, /**< Divided by 4 */
    CY_SMIF_MDL_CLK_OUT_DIV8  = 2, /**< Divided by 8 */
    CY_SMIF_MDL_CLK_OUT_DIV16 = 3, /**< Divided by 16 */
} cy_en_cy_smif_mdl_clk_out_div_t;

/* Determines the relative amount of delay through the MDL/SDL(including neg and pos) for the reference clock in terms of the number of tap delays. */
typedef enum
{
    CY_SMIF_DDL_1_TAP_DELAY  = 0, /**< 1 tap delay */
    CY_SMIF_DDL_2_TAP_DELAY  = 1, /**< 2 tap delay */
    CY_SMIF_DDL_3_TAP_DELAY  = 2, /**< 3 tap delay */
    CY_SMIF_DDL_4_TAP_DELAY  = 3, /**< 4 tap delay */
    CY_SMIF_DDL_5_TAP_DELAY  = 4, /**< 5 tap delay */
    CY_SMIF_DDL_6_TAP_DELAY  = 5, /**< 6 tap delay */
    CY_SMIF_DDL_7_TAP_DELAY  = 6, /**< 7 tap delay */
    CY_SMIF_DDL_8_TAP_DELAY  = 7, /**< 8 tap delay */
    CY_SMIF_DDL_9_TAP_DELAY  = 8, /**< 9 tap delay */
    CY_SMIF_DDL_10_TAP_DELAY  = 9, /**< 10 tap delay */
    CY_SMIF_DDL_11_TAP_DELAY  = 10, /**< 11 tap delay */
    CY_SMIF_DDL_12_TAP_DELAY  = 11, /**< 12 tap delay */
    CY_SMIF_DDL_13_TAP_DELAY  = 12, /**< 13 tap delay */
    CY_SMIF_DDL_14_TAP_DELAY  = 13, /**< 14 tap delay */
    CY_SMIF_DDL_15_TAP_DELAY  = 14, /**< 15 tap delay */
    CY_SMIF_DDL_16_TAP_DELAY  = 15, /**< 16 tap delay */
    CY_SMIF_DDL_TAP_NULMBER  = 16, /**< Tap Count */
} cy_en_cy_smif_ddl_tap_sel_t;

#define CY_SMIF_DLL_TAP_MAX (CY_SMIF_DDL_TAP_NULMBER)

/* Determines the receive capture mode: */
typedef enum
{
    CY_SMIF_CAP_MODE_SPI     = 0, /**< SPI without RWDS, without DLP */
    CY_SMIF_CAP_MODE_SPI_DLP = 1, /**< SPI without RWDS, with DLP */
    CY_SMIF_CAP_MODE_RWDS    = 2, /**< Hyperbus or SPI  with RWDS */
} cy_en_cy_smif_rx_cap_mode_t;

/* This field is only applicable in SDR transmit mode. */
typedef enum
{
    CY_SMIF_TX_ONE_PERIOD_AHEAD = 0, /**< transmit data is launched nominally 1 PLL clock period ahead of the rising edge of the clock out to the memory */
    CY_SMIF_TX_TWO_PERIOD_AHEAD = 1, /**< transmit data is launched nominally 2 PLL clock periods ahead of the rising edge of the clock out to the memory */
} cy_en_cy_smif_tx_sdr_extra_t;

typedef struct
{
    uint32_t                        pllFreq;    /**< Input PLL frequency */
    cy_en_cy_smif_mdl_clk_out_div_t mdlOutDiv;  /**< \ref cy_en_cy_smif_mdl_clk_out_div_t */
    cy_en_cy_smif_ddl_tap_sel_t     mdlTapSel;  /**< \ref cy_en_cy_smif_mdl_tap_sel_t */
    cy_en_cy_smif_rx_cap_mode_t     rxCapMode;  /**< \ref cy_en_cy_smif_rx_cap_mode_t */
    cy_en_cy_smif_tx_sdr_extra_t    txSdrExtra; /**< \ref cy_en_cy_smif_tx_sdr_extra_t */
} cy_stc_smif_dll_config_t;


/* SMIF Bridge Remapping Entities */
typedef enum
{
    CY_EN_BRIDGE_PRIO_SMIF0_XPI_SPACE = 0, // The master accessing through the SMIF0 XIP space has high priority.
    CY_EN_BRIDGE_PRIO_SMIF1_XPI_SPACE = 1, // The master accessing through the SMIF1 XIP space has high priority.
} cy_en_smif_bridge_xip_space_pri_t;

typedef enum
{
    CY_EN_BRIDGE_SMIF0_XPI_SPACE = 0,
    CY_EN_BRIDGE_SMIF1_XPI_SPACE = 1,
} cy_en_smif_bridge_xip_space_t;

typedef enum
{
    CY_EN_BRIGE_REMAP_TYPE_INACTIVE   = 0,
    CY_EN_BRIGE_REMAP_TYPE_TO_SMIF0   = 1,
    CY_EN_BRIGE_REMAP_TYPE_TO_SMIF1   = 2,
    CY_EN_BRIGE_REMAP_TYPE_INTERLEAVE = 3,
} cy_en_smif_bridge_remap_type_t;

typedef enum
{
    CY_EN_BRIGE_INTERLEAVE_8BYTE   = 0,
    CY_EN_BRIGE_INTERLEAVE_16BYTE  = 1,
    CY_EN_BRIGE_INTERLEAVE_32BYTE  = 2,
    CY_EN_BRIGE_INTERLEAVE_64BYTE  = 3,
    CY_EN_BRIGE_INTERLEAVE_128BYTE = 4,
} cy_en_smif_bridge_interleave_step_t;

typedef enum
{
    CY_EN_BRIGE_REMAP_SIZE_1MB   = 0x1FF00000ul,
    CY_EN_BRIGE_REMAP_SIZE_2MB   = 0x1FE00000ul,
    CY_EN_BRIGE_REMAP_SIZE_4MB   = 0x1FC00000ul,
    CY_EN_BRIGE_REMAP_SIZE_8MB   = 0x1F800000ul,
    CY_EN_BRIGE_REMAP_SIZE_16MB  = 0x1F000000ul,
    CY_EN_BRIGE_REMAP_SIZE_32MB  = 0x1E000000ul,
    CY_EN_BRIGE_REMAP_SIZE_64MB  = 0x1C000000ul,
    CY_EN_BRIGE_REMAP_SIZE_128MB = 0x18000000ul,
    CY_EN_BRIGE_REMAP_SIZE_256MB = 0x10000000ul,
    CY_EN_BRIGE_REMAP_SIZE_512MB = 0x00000000ul,
} cy_en_smif_bridge_remap_region_size_t;

typedef struct
{
    uint32_t                              regionIdx;
    cy_en_smif_bridge_remap_region_size_t regionSize;
    uint32_t                              xipAddr;
    uint32_t                              phyAddr;
} cy_stc_smif_bridge_remap_t;

typedef struct
{
    uint32_t                              regionIdx;
    cy_en_smif_bridge_remap_region_size_t regionSize;
    uint32_t                              xipAddr;
    uint32_t                              phyAddr0;
    uint32_t                              phyAddr1;
} cy_stc_smif_bridge_interleave_remap_t;

typedef enum
{
    CY_SMIF_RX_CAP_STYLE_SDR_POS       = 0,
    CY_SMIF_RX_CAP_STYLE_SDR_NEG_NORM  = 1,
    CY_SMIF_RX_CAP_STYLE_SDR_NEG_PIPED = 2,
} cy_en_sdr_cap_style_t;

typedef enum
{
    CY_SMIF_RX_CAP_STYLE_DDR_NORM_SPI   = 0, // For only normal SPI
    CY_SMIF_RX_CAP_STYLE_DDR_HYPERBUS   = 1, // For hyper bus
    CY_SMIF_RX_CAP_STYLE_DDR_OCTAL      = 2, // For Octal SPI mode. In case of using RWDS, bytes will swap: Byte captured by positive edge will go MSB
    CY_SMIF_RX_CAP_STYLE_DDR_OCTAL_SWAP = 3, // For Octal SPI mode. Byte captured by positive edge will go MSB
    CY_SMIF_RX_CAP_STYLE_DDR_xSPI       = 4, // For other SPI modes. 
} cy_en_ddr_cap_style_t;

#include "smif/common/cy_smif.h"

/**
* \addtogroup group_smif_version_specific_functions
*  This device uses SMIF revision 4.0
* \{
*/

extern cy_en_smif_status_t     Cy_SMIF_Init(volatile cy_stc_smif_reg_t *base, cy_stc_smif_config_t const *config, uint32_t timeout, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t     Cy_SMIF_DllConfig(volatile cy_stc_smif_reg_t *base, const cy_stc_smif_dll_config_t* pDllCfg);
extern void                    Cy_SMIF_SetDllOutDivider(volatile cy_stc_smif_reg_t *base, cy_en_cy_smif_mdl_clk_out_div_t div);
extern bool                    Cy_SMIF_IsDllLocked(volatile cy_stc_smif_reg_t *base);
extern void                    Cy_SMIF_DeInit(volatile cy_stc_smif_reg_t *base);
extern cy_en_smif_status_t     Cy_SMIF_Encrypt(volatile cy_stc_smif_reg_t *base, uint32_t address, uint8_t dataInOut[], uint32_t sizeInByte, cy_stc_smif_context_t const *context);
extern cy_en_smif_status_t     Cy_SMIF_SetDelayLineForFreq(volatile cy_stc_smif_reg_t *base, uint8_t freqMhz, bool isDDR, uint8_t *calculatedTap);
extern void                    Cy_SMIF_SetMasterDLP(volatile cy_stc_smif_reg_t *base, uint16_t dlp);
extern uint16_t                Cy_SMIF_GetMasterDLP(volatile cy_stc_smif_reg_t *base);
extern cy_en_smif_status_t     Cy_SMIF_SetMasterDLP_Size(volatile cy_stc_smif_reg_t *base, uint32_t size);
extern uint8_t                 Cy_SMIF_GetMasterDLP_Size(volatile cy_stc_smif_reg_t *base);
extern cy_en_smif_status_t     Cy_SMIF_SetMasterDLP_WarnLevel(volatile cy_stc_smif_reg_t *base, uint32_t warnLevel);
extern uint8_t                 Cy_SMIF_GetMasterDLP_WarnLevel(volatile cy_stc_smif_reg_t *base);
extern uint8_t                 Cy_SMIF_GetTapNumCapturedCorrectDLP(volatile cy_stc_smif_reg_t *base, uint8_t bit);
extern cy_en_smif_status_t     Cy_SMIF_Set_DelayLineSel(volatile cy_stc_smif_reg_t *base, cy_en_smif_delay_line_t lineSel);
extern cy_en_smif_delay_line_t Cy_SMIF_Get_DelayLineSel(volatile cy_stc_smif_reg_t *base);
extern cy_en_smif_status_t     Cy_SMIF_Set_DelayTapSel(volatile void *smif_or_device_base, uint8_t tapSel);
extern uint8_t                 Cy_SMIF_Get_DelayTapSel(volatile void *smif_or_device_base);
extern void                    Cy_SMIF_SetTxFifoTriggerLevel(volatile cy_stc_smif_reg_t *base, uint32_t level);
extern void                    Cy_SMIF_SetCryptoInput128(volatile cy_stc_smif_reg_t *base, const uint8_t inData[]);
extern void                    Cy_SMIF_SetCryptoKey128(volatile cy_stc_smif_reg_t *base, const uint8_t key[]);
extern uint32_t                Cy_SMIF_GetTxFifoStatus(volatile cy_stc_smif_reg_t *base);
extern void                    Cy_SMIF_PushCmdFifo(volatile cy_stc_smif_reg_t *base, uint32_t data[], uint32_t sizeInWord);
extern void                    Cy_SMIF_PushTxFifo(volatile cy_stc_smif_reg_t *baseaddr, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t     Cy_SMIF_SetCaptureConfig(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave, bool isHyperBus);
void                           Cy_SMIF_DeviceSetRxCaptureDdr(volatile cy_stc_smif_reg_device_t *device, cy_en_ddr_cap_style_t capStyle);
extern void                    Cy_SMIF_DeviceSetRxCaptureSdr(volatile cy_stc_smif_reg_device_t *device, cy_en_sdr_cap_style_t capStyle);
extern cy_en_smif_status_t     Cy_SMIF_SwitchBridge(volatile cy_stc_smif_base_reg_t *baseaddr, bool switchOn);
extern cy_en_smif_status_t     Cy_SMIF_SetBridgePriority(volatile cy_stc_smif_base_reg_t *baseaddr, cy_en_smif_bridge_xip_space_pri_t pri_ahb_smif0, cy_en_smif_bridge_xip_space_pri_t pri_ahb_smif1, cy_en_smif_bridge_xip_space_pri_t pri_axi_smif0, cy_en_smif_bridge_xip_space_pri_t pri_axi_smif1);
extern cy_en_smif_status_t     Cy_SMIF_SetSimpleRemapRegion(volatile cy_stc_smif_base_reg_t *baseaddr, const cy_stc_smif_bridge_remap_t* pCfg);
extern cy_en_smif_status_t     Cy_SMIF_SetInterleavingRemapRegion(volatile cy_stc_smif_base_reg_t *baseaddr, const cy_stc_smif_bridge_interleave_remap_t* pCfg);
extern cy_en_smif_status_t     Cy_SMIF_DeactivateRemapRegion(volatile cy_stc_smif_base_reg_t *baseaddr, uint32_t regionIdx);

/** \} group_smif_version_specific_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

#endif /* (CY_SMIF_H) */

/** \} group_smif */


/* [] END OF FILE */
