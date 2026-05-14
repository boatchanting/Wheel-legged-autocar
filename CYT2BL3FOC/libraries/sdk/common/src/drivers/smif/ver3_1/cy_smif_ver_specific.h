/***************************************************************************//**
* \file cy_smif_ver_specific.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress SMIF v3.1 driver.
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
typedef stc_SMIF_t                                  cy_stc_smif_reg_t;
typedef un_SMIF_CTL_t                               cy_un_smif_reg_ctl_t;
typedef un_SMIF_STATUS_t                            cy_un_smif_reg_status_t;
typedef un_SMIF_INT_CLOCK_DELAY_TAP_SEL0_t          cy_un_smif_reg_dlp_delay_tap_sel0_t;
typedef un_SMIF_INT_CLOCK_DELAY_TAP_SEL1_t          cy_un_smif_reg_dlp_delay_tap_sel1_t;
typedef un_SMIF_DL_CTL_t                            cy_un_smif_reg_dlp_ctl_t;
typedef un_SMIF_DL_STATUS0_t                        cy_un_smif_reg_dlp_status0_t;
typedef un_SMIF_DL_STATUS1_t                        cy_un_smif_reg_dlp_status1_t;
typedef un_SMIF_TX_CMD_FIFO_STATUS_t                cy_un_smif_reg_tx_cmd_fifo_status_t;
typedef un_SMIF_TX_CMD_MMIO_FIFO_STATUS_t           cy_un_smif_reg_tx_cmd_mmio_fifo_status_t;
typedef un_SMIF_TX_CMD_MMIO_FIFO_WR_t               cy_un_smif_reg_tx_cmd_mmio_fifo_wr_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_CTL_t             cy_un_smif_reg_tx_data_mmio_fifo_ctl_t;
typedef un_SMIF_TX_DATA_FIFO_STATUS_t               cy_un_smif_reg_tx_data_fifo_status_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_STATUS_t          cy_un_smif_reg_tx_data_mmio_fifo_status_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_WR1_t             cy_un_smif_reg_tx_data_mmio_fifo_wr1_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_WR2_t             cy_un_smif_reg_tx_data_mmio_fifo_wr2_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_WR4_t             cy_un_smif_reg_tx_data_mmio_fifo_wr4_t;
typedef un_SMIF_TX_DATA_MMIO_FIFO_WR1ODD_t          cy_un_smif_reg_tx_data_mmio_fifo_wr1odd_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_CTL_t             cy_un_smif_reg_rx_data_mmio_fifo_ctl_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_STATUS_t          cy_un_smif_reg_rx_data_mmio_fifo_status_t;
typedef un_SMIF_RX_DATA_FIFO_STATUS_t               cy_un_smif_reg_rx_data_fifo_status_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_RD1_t             cy_un_smif_reg_rx_data_mmio_fifo_rd1_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_RD2_t             cy_un_smif_reg_rx_data_mmio_fifo_rd2_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_RD4_t             cy_un_smif_reg_rx_data_mmio_fifo_rd4_t;
typedef un_SMIF_RX_DATA_MMIO_FIFO_RD1_SILENT_t      cy_un_smif_reg_rx_data_mmio_fifo_rd1_silent_t;
typedef un_SMIF_SLOW_CA_CTL_t                       cy_un_smif_reg_slow_ca_ctl_t;
typedef un_SMIF_SLOW_CA_CMD_t                       cy_un_smif_reg_slow_ca_cmd_t;
typedef un_SMIF_FAST_CA_CTL_t                       cy_un_smif_reg_fast_ca_ctl_t;
typedef un_SMIF_FAST_CA_CMD_t                       cy_un_smif_reg_fast_ca_cmd_t;
typedef un_SMIF_CRC_CMD_t                           cy_un_smif_reg_crc_cmd_t;
typedef un_SMIF_CRC_INPUT0_t                        cy_un_smif_reg_crc_input0_t;
typedef un_SMIF_CRC_INPUT1_t                        cy_un_smif_reg_crc_input1_t;
typedef un_SMIF_CRC_OUTPUT_t                        cy_un_smif_reg_crc_output_t;
typedef un_SMIF_INTR_t                              cy_un_smif_reg_intr_t;
typedef un_SMIF_INTR_SET_t                          cy_un_smif_reg_intr_set_t;
typedef un_SMIF_INTR_MASK_t                         cy_un_smif_reg_intr_mask_t;
typedef un_SMIF_INTR_MASKED_t                       cy_un_smif_reg_intr_masked_t;
typedef un_SMIF_INTR_CAUSE_t                        cy_un_smif_reg_cause_t;

typedef stc_SMIF_DEVICE_t                           cy_stc_smif_reg_device_t;
typedef un_SMIF_DEVICE_CTL_t                        cy_un_smif_reg_device_ctl_t;
typedef un_SMIF_DEVICE_ADDR_t                       cy_un_smif_reg_device_addr_t;
typedef un_SMIF_DEVICE_MASK_t                       cy_un_smif_reg_device_mask_t;
typedef un_SMIF_DEVICE_ADDR_CTL_t                   cy_un_smif_reg_device_addr_ctl_t;
typedef un_SMIF_DEVICE_DELAY_TAP_SEL_t              cy_un_smif_reg_device_capture_config_t;
typedef un_SMIF_DEVICE_RD_STATUS_t                  cy_un_smif_reg_device_rd_status_t;
typedef un_SMIF_DEVICE_RD_CMD_CTL_t                 cy_un_smif_reg_device_rd_cmd_ctl_t;
typedef un_SMIF_DEVICE_RD_ADDR_CTL_t                cy_un_smif_reg_device_rd_addr_ctl_t;
typedef un_SMIF_DEVICE_RD_MODE_CTL_t                cy_un_smif_reg_device_rd_mode_ctl_t;
typedef un_SMIF_DEVICE_RD_DUMMY_CTL_t               cy_un_smif_reg_device_rd_dummy_t;
typedef un_SMIF_DEVICE_RD_DATA_CTL_t                cy_un_smif_reg_device_rd_data_ctl_t;
typedef un_SMIF_DEVICE_RD_CRC_CTL_t                 cy_un_smif_reg_device_rd_crc_ctl_t;
typedef un_SMIF_DEVICE_RD_BOUND_CTL_t               cy_un_smif_reg_device_rd_bound_ctl_t;
typedef un_SMIF_DEVICE_WR_CMD_CTL_t                 cy_un_smif_reg_device_wr_cmd_t;
typedef un_SMIF_DEVICE_WR_ADDR_CTL_t                cy_un_smif_reg_device_wr_addr_t;
typedef un_SMIF_DEVICE_WR_MODE_CTL_t                cy_un_smif_reg_device_wr_mode_ctl_t;
typedef un_SMIF_DEVICE_WR_DUMMY_CTL_t               cy_un_smif_reg_device_wr_dummy_ctl_t;
typedef un_SMIF_DEVICE_WR_DATA_CTL_t                cy_un_smif_reg_device_wr_data_ctl_t;
typedef un_SMIF_DEVICE_WR_CRC_CTL_t                 cy_un_smif_reg_device_wr_crc_ctl_t;

typedef stc_SMIF_SMIF_CRYPTO_t                      cy_stc_smif_reg_crypto_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_CMD_t            cy_un_smif_reg_crypto_cmd_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_ADDR_t           cy_un_smif_reg_crypto_addr_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_MASK_t           cy_un_smif_reg_crypto_mask_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_SUBREGION_t      cy_un_smif_reg_crypto_subregion_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_INPUT0_t         cy_un_smif_reg_crypto_input0_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_INPUT1_t         cy_un_smif_reg_crypto_input1_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_INPUT2_t         cy_un_smif_reg_crypto_input2_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_INPUT3_t         cy_un_smif_reg_crypto_input3_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_KEY0_t           cy_un_smif_reg_crypto_key0_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_KEY1_t           cy_un_smif_reg_crypto_key1_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_KEY2_t           cy_un_smif_reg_crypto_key2_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_KEY3_t           cy_un_smif_reg_crypto_key3_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_OUTPUT0_t        cy_un_smif_reg_crypto_output0_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_OUTPUT1_t        cy_un_smif_reg_crypto_output1_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_OUTPUT2_t        cy_un_smif_reg_crypto_output2_t;
typedef un_SMIF_SMIF_CRYPTO_CRYPTO_OUTPUT3_t        cy_un_smif_reg_crypto_output3_t;

/* Definitions for smif register masks, so make it common (only for required entities) */
#define CY_SMIF_DRV_DEVICE_ADDR_MSK                       SMIF_DEVICE_ADDR_ADDR_Msk
#define CY_SMIF_DRV_INTR_TR_TX_REQ_MSK                    SMIF_INTR_TR_TX_REQ_Msk
#define CY_SMIF_DRV_INTR_TR_RX_REQ_MSK                    SMIF_INTR_TR_RX_REQ_Msk
#define CY_SMIF_DRV_INTR_XIP_ALIGNMENT_ERROR_MSK          SMIF_INTR_XIP_ALIGNMENT_ERROR_Msk
#define CY_SMIF_DRV_INTR_TX_CMD_FIFO_OVERFLOW_MSK         SMIF_INTR_TX_CMD_FIFO_OVERFLOW_Msk
#define CY_SMIF_DRV_INTR_TX_DATA_FIFO_OVERFLOW_MSK        SMIF_INTR_TX_DATA_FIFO_OVERFLOW_Msk
#define CY_SMIF_DRV_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_MSK  SMIF_INTR_RX_DATA_MMIO_FIFO_UNDERFLOW_Msk

/* Definitions for smif register top address, so make it common */
#if defined(SMIF0)
  #define CY_SMIF_DRV_SMIF0_CORE0                         SMIF0
#endif

#if defined(SMIF1)
  #define CY_SMIF_CORE1_EXISTS
  #define CY_SMIF_DRV_SMIF0_CORE1                         SMIF1
#endif

#if defined(SMIF0_DEVICE0)
  #define CY_SMIF_DRV_SMIF0_CORE0_DEVICE0                 SMIF0_DEVICE0
#endif

#if defined(SMIF0_DEVICE1)
  #define CY_SMIF_DRV_SMIF0_CORE0_DEVICE1                 SMIF0_DEVICE1
#endif

#if defined(SMIF1_DEVICE0)
  #define CY_SMIF_DRV_SMIF0_CORE1_DEVICE0                 SMIF1_DEVICE0
#endif

#if defined(SMIF1_DEVICE1)
  #define CY_SMIF_DRV_SMIF0_CORE1_DEVICE1                 SMIF1_DEVICE1
#endif

#if defined(SMIF_DEVICE_SECTION_SIZE)
  #define CY_SMIF_DRV_DEVICE_SECTION_SIZE                 SMIF_DEVICE_SECTION_SIZE
#endif

#define CY_SMIF_DRV_SMIF0_IRQN                            smif_0_interrupt_IRQn

#if defined(CY_SMIF_CORE1_EXISTS)
  #define CY_SMIF_DRV_SMIF1_IRQN                          smif_1_interrupt_IRQn
#endif

/* Specifies the clock source for "clk_if".  Must be 0 (clk_hf)  */
/* before entering DeepSleep; can be returned to 1 (clk_pll) afterwards. */
typedef enum
{
    CY_SMIF_CLKIF_SRC_CLK_HF  = 0,    /**< Clock source clk_hf */
    CY_SMIF_CLKIF_SRC_CLK_PLL = 1     /**< Clock source clk_pll */
} cy_en_cy_clkif_source_t;

#include "smif/common/cy_smif.h"

/**
* \addtogroup group_smif_version_specific_functions
*  This device uses SMIF revision 3.1
* \{
*/

extern cy_en_smif_status_t     Cy_SMIF_Init(volatile cy_stc_smif_reg_t *base, cy_stc_smif_config_t const *config, uint32_t timeout, cy_stc_smif_context_t *context);
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

/** \} group_smif_version_specific_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

#endif /* (CY_SMIF_H) */

/** \} group_smif */


/* [] END OF FILE */
