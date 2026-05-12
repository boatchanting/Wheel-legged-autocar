/***************************************************************************//**
* \file cy_i2s.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
* 
* The header file of the I2S driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_i2s Inter-IC Sound (I2S)
* \{
* The I2S driver provides a function API to manage Inter-IC Sound. I2S is used 
* to send digital audio streaming data to external I2S devices, such as audio 
* codecs or simple DACs. It can also receive digital audio streaming data.
*
* Features:
* * An industry standard NXP I2S interface.
* * Supports master/slave TX/RX operation.
* * Programmable Channel/Word Lengths.
* * Supports External Clock operation.
*
* The I2S bus is an industry standard. The hardware interface was 
* developed by Philips Semiconductors (now NXP Semiconductors). 
*
* \section group_i2s_configuration_considerations Configuration Considerations
*
* To set up an I2S, provide the configuration parameters in the
* \ref cy_stc_i2s_config_t structure. 
* 
* For example, for Tx configuration, set txEnabled to true, configure
* txDmaTrigger (depending on whether DMA is going to be used or not), set
* extClk (if an external clock is used), provide clkDiv, txMasterMode,
* txAlignment, txChannels (only 2 is supported in I2S and Left Justified modes)
* txSdoLatchingTime (for slave mode only), txChannelLength, txWordLength,
* txWsPulseWidth (for TMD modes only), txWatchdogEnable and txWatchdogValue
* (both for Slave mode only, and when the watchdog interrupt will be used),
* either txSckoInversion or txSckiInversion (based on txMasterMode setting),
* txFifoTriggerLevel (when the Trig interrupt will be used) and txOverheadValue
* (only when the word length is less than channel length).
* A similar setup is for the Rx configuration.
*
* To initialize the I2S block, call the \ref Cy_I2S_Init function, providing the
* filled \ref cy_stc_i2s_config_t structure.
* Before starting the transmission, clear the FIFO \ref Cy_I2S_ClearTxFifo, then
* fill the first Tx data frame by calling \ref Cy_I2S_WriteTxData once for each
* channel (e.g. twice for I2S mode with only two channels) with zero data. Then
* call the \ref Cy_I2S_EnableTx itself.
* For the reception the sequence is the same except for filling the first data
* frame, just RX FIFO clearing is enough.
*
* If you use a DMA, the DMA channel should be previously configured. The I2S interrupts 
* (if applicable) can be enabled by calling \ref Cy_I2S_SetInterruptMask.
*
* For example, if the trigger interrupt is used, during operation the ISR
* should call the \ref Cy_I2S_WriteTxData as many times as required for your 
* FIFO payload, but not more than the FIFO size. Then call \ref Cy_I2S_ClearInterrupt 
* with appropriate parameters.
*
* The I2S/Left Justified data formats always contains two data channels.
* They are ordered one-by-one in the FIFOs, left always goes first.
* So in case of mono audio stream transmission, each sample can be put twice
* into the TX FIFO (in this case both channels will sound the same),
* or combined with zeroes: sample1-zero-sample2-zero (in this case only the
* left channel will finally sound, for right-only case zero should go first).
* The TDM frame word order in FIFOs is similar, one-by-one.
* 
* If a DMA is used and the DMA channel is properly configured - no CPU activity
* (or any application code) is needed for I2S operation.
*
* \section group_i2s_more_information More Information
* See: the the I2S chapter of the device technical reference manual (TRM);
*      I2S_PDL Component datasheet;
*      CE218636 - PSOC 6 MCU INTER-IC SOUND (I2S) EXAMPLE.
*
* \section group_i2s_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>2.10</td>
*     <td>Flattened the driver source code organization into the single
*         source directory and the single include directory
*     </td>
*     <td>Simplified the driver library directory structure</td>
*   </tr>
*   <tr>
*     <td>2.0.1</td>
*     <td>Added Low Power Callback section</td>
*     <td>Documentation update and clarification</td>
*   </tr>
*   <tr>
*     <td>2.0</td>
*     <td>The slave operation is added, Left Justified and TDM modes are added</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_i2s_macros Macros
* \defgroup group_i2s_functions Functions
*   \{
*       \defgroup group_i2s_functions_syspm_callback  Low Power Callback
*   \}
* \defgroup group_i2s_data_structures Data Structures
* \defgroup group_i2s_enums Enumerated Types
*/


#if !defined CY_I2S_H
#define CY_I2S_H

#include <stddef.h>
#include <stdbool.h>
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"

#ifdef CY_IP_MXAUDIOSS

#ifdef __cplusplus
extern "C" {
#endif

/** \addtogroup group_i2s_macros
* \{
*/

/** The driver major version */
#define CY_I2S_DRV_VERSION_MAJOR       `$CY_MAJOR_VERSION`

/** The driver minor version */
#define CY_I2S_DRV_VERSION_MINOR       `$CY_MINOR_VERSION`

/** The I2S driver identifier */
#define CY_I2S_ID                      (CY_PDL_DRV_ID(0x20U))

/**
* \defgroup group_i2s_macros_interrupt_masks Interrupt Masks
* \{
*/

/** Bit 0: Less entries in the TX FIFO than specified by Trigger Level. */
#define CY_I2S_INTR_TX_TRIGGER         (I2S_INTR_TX_TRIGGER_Msk)
/** Bit 1: TX FIFO is not full. */
#define CY_I2S_INTR_TX_NOT_FULL        (I2S_INTR_TX_NOT_FULL_Msk)
/** Bit 4: TX FIFO is empty, i.e. it has 0 entries. */
#define CY_I2S_INTR_TX_EMPTY           (I2S_INTR_TX_EMPTY_Msk)
/** Bit 5: Attempt to write to a full TX FIFO. */
#define CY_I2S_INTR_TX_OVERFLOW        (I2S_INTR_TX_OVERFLOW_Msk)
/** Bit 6: Attempt to read from an empty TX FIFO.
* This happens when the IP is ready to transfer data and TX_EMPTY is '1'. */
#define CY_I2S_INTR_TX_UNDERFLOW       (I2S_INTR_TX_UNDERFLOW_Msk)
/** Bit 8: Tx watchdog event occurs. */
#define CY_I2S_INTR_TX_WD              (I2S_INTR_TX_WD_Msk)
/** Bit 16: More entries in the RX FIFO than specified by Trigger Level. */
#define CY_I2S_INTR_RX_TRIGGER         (I2S_INTR_RX_TRIGGER_Msk)
/** Bit 18: RX FIFO is not empty. */
#define CY_I2S_INTR_RX_NOT_EMPTY       (I2S_INTR_RX_NOT_EMPTY_Msk)
/** Bit 19: RX FIFO is full. */
#define CY_I2S_INTR_RX_FULL            (I2S_INTR_RX_FULL_Msk)
/** Bit 21: Attempt to write to a full RX FIFO. */
#define CY_I2S_INTR_RX_OVERFLOW        (I2S_INTR_RX_OVERFLOW_Msk)
/** Bit 22: Attempt to read from an empty RX FIFO. */
#define CY_I2S_INTR_RX_UNDERFLOW       (I2S_INTR_RX_UNDERFLOW_Msk)
/** Bit 24: Rx watchdog event occurs. */
#define CY_I2S_INTR_RX_WD              (I2S_INTR_RX_WD_Msk)

/** \} group_i2s_macros_interrupt_masks */


/**
* \defgroup group_i2s_macros_current_state Current State
* \{
*/

/** Transmission is active */ 
#define CY_I2S_TX_START                (I2S_CMD_TX_START_Msk)
/** Transmission is paused */
#define CY_I2S_TX_PAUSE                (I2S_CMD_TX_PAUSE_Msk)
/** Reception is active */
#define CY_I2S_RX_START                (I2S_CMD_RX_START_Msk)

/** \} group_i2s_macros_current_state */

#define CY_I2S_TX_FIFO_NUM  (256ul)
#define CY_I2S_RX_FIFO_NUM  (256ul)

/** \} group_i2s_macros */

/**
* \addtogroup group_i2s_enums
* \{
*/

/**
* I2S status definitions.
*/

typedef enum 
{
    CY_I2S_SUCCESS   = 0x00ul, /**< Successful. */
    CY_I2S_BAD_PARAM = CY_I2S_ID | CY_PDL_STATUS_ERROR | 0x01ul /**< One or more invalid parameters. */
} cy_en_i2s_status_t;


/**
* I2S data alignment.
*/
typedef enum
{
    CY_I2S_LEFT_JUSTIFIED = 0u, /**< Left justified. */
    CY_I2S_I2S_MODE       = 1u, /**< I2S mode. */
    CY_I2S_TDM_MODE_A     = 2u, /**< TDM mode A. */
    CY_I2S_TDM_MODE_B     = 3u  /**< TDM mode B. */
} cy_en_i2s_alignment_t;

/**
* I2S channel/word length.
*/
typedef enum
{
    CY_I2S_LEN8  = 0u, /**< Channel/word length:  8 bit. */
    CY_I2S_LEN16 = 1u, /**< Channel/Word length: 16 bit. */
    CY_I2S_LEN18 = 2u, /**< Channel/Word length: 18 bit. */
    CY_I2S_LEN20 = 3u, /**< Channel/Word length: 20 bit. */
    CY_I2S_LEN24 = 4u, /**< Channel/Word length: 24 bit. */
    CY_I2S_LEN32 = 5u  /**< Channel/Word length: 32 bit. */
} cy_en_i2s_len_t;

/**
* I2S TX overhead value.
*/
typedef enum
{
    CY_I2S_OVHDATA_ZERO = 0u, /**< Fill overhead bits by zeroes. */
    CY_I2S_OVHDATA_ONE  = 1u, /**< Fill overhead bits by ones. */
} cy_en_i2s_overhead_t;

/**
* I2S WS pulse width.
*/
typedef enum
{
    CY_I2S_WS_ONE_SCK_CYCLE      = 0u, /**< WS pulse width is one SCK cycle. */
    CY_I2S_WS_ONE_CHANNEL_LENGTH = 1u, /**< WS pulse width is one channel length. */
} cy_en_i2s_ws_pw_t;

typedef enum
{
    CY_I2S_MCLK_DIV_1 = 0u, /**< Divide clk_audio_i2s by 1 (Bypass). */
    CY_I2S_MCLK_DIV_2 = 1u, /**< Divide clk_audio_i2s by 2. */
    CY_I2S_MCLK_DIV_4 = 2u, /**< Divide clk_audio_i2s by 4. */
    CY_I2S_MCLK_DIV_8 = 3u, /**< Divide clk_audio_i2s by 8. */
} cy_en_i2s_mclk_div_t;


/** \} group_i2s_enums */

/**
* \addtogroup group_i2s_data_structures
* \{
*/

/**
* I2S initialization configuration.
*/
typedef struct
{
    uint8_t              clkDiv;  /**< CLK_SEL divider: 1: Bypass, 2: 1/2, 3: 1/3, ..., 64: 1/64. */
    bool                 extClk;  /**< 'false': internal clock, 'true': external clock. */
    cy_en_i2s_mclk_div_t mclkDiv; /**< Selects clock divider for MCLK_OUT. */
    bool                 mclkEn;  /**< Enable MCLK - enables MCLK divider operation */
} cy_stc_i2s_config_clk_t;

typedef struct
{
    bool                  txEnabled;          /**< Enables the I2S TX component: 'false': disabled. 'true': enabled. */
    bool                  txDmaTrigger;       /**< 'false': TX DMA trigger disable, 'true': TX DMA trigger enable. */
    bool                  txMasterMode;       /**< 'false': TX in slave mode, 'true': TX in master mode. */
    cy_en_i2s_alignment_t txAlignment;        /**< TX data alignment, see: #cy_en_i2s_alignment_t. */
    cy_en_i2s_ws_pw_t     txWsPulseWidth;     /**< TX Word Select pulse width.
                                                   The value of this parameter is ignored in I2S and Left Justified modes
                                                   the WS pulse width is always "one channel length" in these modes. */
    bool                  txWatchdogEnable;   /**< 'false': TX watchdog disabled, 'true': TX watchdog enabled. */
    uint32_t              txWatchdogValue;    /**< TX watchdog counter value (32 bit). */
    bool                  txSdoLatchingTime;  /**< 'false': SDO bit starts at falling edge (accordingly to the I2S
                                                   Standard, if txSckoInversion is false),
                                                   'true': SDO bit starts at rising edge which goes before the above
                                                   mentioned falling edge, i.e. the SDO signal is advanced by 0.5 SCK
                                                   period (if txSckoInversion is false).
                                                   If txSckoInversion is true - the rising/falling edges just swaps 
                                                   in above explanations.
                                                   Effective only in slave mode, must be false in master mode.*/
    bool                  txSckoInversion;    /**< TX SCKO polarity:
                                                   'false': When transmitter is in master mode, serial data is
                                                            transmitted off the falling bit clock edge (accordingly to
                                                            the I2S Standard);
                                                   'true':  When transmitter is in master mode, serial data is
                                                            transmitted off the rising bit clock edge.
                                                   Effective only in master mode. */
    bool                  txSckiInversion;    /**< TX SCKI polarity:
                                                   'false': When transmitter is in slave mode, serial data is
                                                            transmitted off the falling bit clock edge (accordingly to
                                                            the I2S Standard);
                                                   'true':  When transmitter is in slave mode, serial data is
                                                            transmitted off the rising bit clock edge.
                                                   Effective only in slave mode. */
    uint8_t               txChannels;         /**< Number of TX channels, valid range is 1...8 for TDM modes.
                                                   In the I2S and Left Justified modes the value of this parameter is
                                                   ignored - the real number of channels is always 2 in these modes. */
    cy_en_i2s_len_t       txChannelLength;    /**< TX channel length, see #cy_en_i2s_len_t,
                                                   the value of this parameter is ignored in TDM modes, the real
                                                   channel length is 32 bit in these modes. */
    cy_en_i2s_len_t       txWordLength;       /**< TX word length, see #cy_en_i2s_len_t,
                                                   must be less or equal to txChannelLength. */
    cy_en_i2s_overhead_t  txOverheadValue;    /**< TX overhead bits value
                                                   when the word length is less than the channel length. */
    uint8_t               txFifoTriggerLevel; /**< TX FIFO interrupt trigger level (0, 1, ..., 255). */
} cy_stc_i2s_config_tx_t;

typedef struct
{
    bool                  rxEnabled;          /**< Enables the I2S RX component: 'false': disabled. 'true': enabled. */
    bool                  rxDmaTrigger;       /**< 'false': RX DMA trigger disable, 'true': RX DMA trigger enable. */
    bool                  rxMasterMode;       /**< 'false': RX in slave mode, 'true': RX in master mode. */
    cy_en_i2s_alignment_t rxAlignment;        /**< RX data alignment, see: #cy_en_i2s_alignment_t. */
    cy_en_i2s_ws_pw_t     rxWsPulseWidth;     /**< RX Word Select pulse width.
                                                   The value of this parameter is ignored in I2S and Left Justified modes
                                                   the WS pulse width is always "one channel length" in these modes. */
    bool                  rxWatchdogEnable;   /**< 'false': RX watchdog disabled, 'true': RX watchdog enabled. */
    uint32_t              rxWatchdogValue;    /**< RX watchdog counter value (32 bit). */
    bool                  rxSdiLatchingTime;  /**< 'false': SDI bit starts at falling edge (accordingly to the I2S
                                                   Standard if rxSckoInversion is false),
                                                   'true': SDI bit starts at rising edge which goes after the above
                                                   mentioned falling edge, i.e. the SDI signal is delayed by 0.5 SCK
                                                   period (if rxSckoInversion is false).
                                                   If rxSckoInversion is true - the rising/falling edges just swaps 
                                                   in above explanations.
                                                   Effective only in master mode, must be false in slave mode. */
    bool                  rxSckoInversion;    /**< RX SCKO polarity:
                                                   'false': When receiver is in master mode, serial data is
                                                            captured by the rising bit clock edge (accordingly to the
                                                            I2S Standard);
                                                   'true':  When receiver is in master mode, serial data is
                                                            captured by the falling bit clock edge.
                                                   Effective only in master mode. */
    bool                  rxSckiInversion;    /**< RX SCKI polarity:
                                                   'false': When receiver is in slave mode, serial data is
                                                            captured by the rising bit clock edge (accordingly to the
                                                            I2S Standard);
                                                   'true':  When receiver is in slave mode, serial data is
                                                            captured by the falling bit clock edge.
                                                   Effective only in slave mode. */
    uint8_t               rxChannels;         /**< Number of RX channels, valid range is 1...8 for TDM modes.
                                                   In the I2S and Left Justified modes the value of this parameter is
                                                   ignored - the real number of channels is always 2 in these modes. */
    cy_en_i2s_len_t       rxChannelLength;    /**< RX channel length, see #cy_en_i2s_len_t,
                                                   the value of this parameter is ignored in TDM modes, the real
                                                   channel length is 32 bit in these modes. */
    cy_en_i2s_len_t       rxWordLength;       /**< RX word length, see #cy_en_i2s_len_t,
                                                   must be less or equal to rxChannelLength. */
    bool                  rxSignExtension;    /**< RX value sign extension (when the word length is less than 32 bits),
                                                   'false': all MSB are filled by zeroes, 
                                                   'true': all MSB are filled by the original sign bit value. */
    uint8_t               rxFifoTriggerLevel; /**< RX FIFO interrupt trigger level (0, 1, ..., (255 - (number of channels))). */

} cy_stc_i2s_config_rx_t;

typedef struct
{
    cy_stc_i2s_config_clk_t cfg_clk;
    cy_stc_i2s_config_tx_t  cfg_tx;
    cy_stc_i2s_config_rx_t  cfg_rx;
} cy_stc_i2s_config_t;

/**
 * The I2S backup structure type to be used for the SysPm callback.
 * \ref Cy_I2S_DeepSleepCallback context definition.
 *
 * \cond Also can be used for other purposes to store the current Tx/Rx
 * operation state and interrupt settings - the factors that are usually
 * changed on the fly. \endcond
 */
typedef struct
{
    uint32_t enableState;   /**< Stores the I2S state */
    uint32_t interruptMask; /**< Stores the I2S interrupt mask */
} cy_stc_i2s_context_t;

/** \} group_i2s_data_structures */

/** \cond INTERNAL */
/******************************************************************************
 * Local definitions
*******************************************************************************/

#define CY_I2S_INTR_MASK           (CY_I2S_INTR_TX_TRIGGER   | \
                                    CY_I2S_INTR_TX_NOT_FULL  | \
                                    CY_I2S_INTR_TX_EMPTY     | \
                                    CY_I2S_INTR_TX_OVERFLOW  | \
                                    CY_I2S_INTR_TX_UNDERFLOW | \
                                    CY_I2S_INTR_TX_WD        | \
                                    CY_I2S_INTR_RX_TRIGGER   | \
                                    CY_I2S_INTR_RX_NOT_EMPTY | \
                                    CY_I2S_INTR_RX_FULL      | \
                                    CY_I2S_INTR_RX_OVERFLOW  | \
                                    CY_I2S_INTR_RX_UNDERFLOW | \
                                    CY_I2S_INTR_RX_WD)

/* Non-zero default values */
#define CY_I2S_TX_CTL_CH_NR_DEFAULT    (0x1ul)
#define CY_I2S_TX_CTL_I2S_MODE_DEFAULT (0x2ul)
#define CY_I2S_TX_CTL_WS_PULSE_DEFAULT (0x1ul)
#define CY_I2S_TX_CTL_CH_LEN_DEFAULT   (0x4ul)
#define CY_I2S_TX_CTL_WORD_LEN_DEFAULT (0x4ul)

#define CY_I2S_TX_CTL_DEFAULT (_VAL2FLD(I2S_TX_CTL_CH_NR,    CY_I2S_TX_CTL_CH_NR_DEFAULT)    | \
                               _VAL2FLD(I2S_TX_CTL_I2S_MODE, CY_I2S_TX_CTL_I2S_MODE_DEFAULT) | \
                               _VAL2FLD(I2S_TX_CTL_WS_PULSE, CY_I2S_TX_CTL_WS_PULSE_DEFAULT) | \
                               _VAL2FLD(I2S_TX_CTL_CH_LEN,   CY_I2S_TX_CTL_CH_LEN_DEFAULT)   | \
                               _VAL2FLD(I2S_TX_CTL_WORD_LEN, CY_I2S_TX_CTL_WORD_LEN_DEFAULT))

#define CY_I2S_RX_CTL_CH_NR_DEFAULT    (0x1ul)
#define CY_I2S_RX_CTL_I2S_MODE_DEFAULT (0x2ul)
#define CY_I2S_RX_CTL_WS_PULSE_DEFAULT (0x1ul)
#define CY_I2S_RX_CTL_CH_LEN_DEFAULT   (0x4ul)
#define CY_I2S_RX_CTL_WORD_LEN_DEFAULT (0x4ul)

#define CY_I2S_RX_CTL_DEFAULT (_VAL2FLD(I2S_RX_CTL_CH_NR,    CY_I2S_RX_CTL_CH_NR_DEFAULT)    | \
                               _VAL2FLD(I2S_RX_CTL_I2S_MODE, CY_I2S_RX_CTL_I2S_MODE_DEFAULT) | \
                               _VAL2FLD(I2S_RX_CTL_WS_PULSE, CY_I2S_RX_CTL_WS_PULSE_DEFAULT) | \
                               _VAL2FLD(I2S_RX_CTL_CH_LEN,   CY_I2S_RX_CTL_CH_LEN_DEFAULT)   | \
                               _VAL2FLD(I2S_RX_CTL_WORD_LEN, CY_I2S_RX_CTL_WORD_LEN_DEFAULT))

/* Macros for conditions used by CY_ASSERT calls */
#define CY_I2S_IS_ALIGNMENT_VALID(alignment) ((CY_I2S_LEFT_JUSTIFIED == (alignment)) || \
                                              (CY_I2S_I2S_MODE       == (alignment)) || \
                                              (CY_I2S_TDM_MODE_A     == (alignment)) || \
                                              (CY_I2S_TDM_MODE_B     == (alignment)))

#define CY_I2S_IS_LEN_VALID(length)          ((CY_I2S_LEN8  == (length)) || \
                                              (CY_I2S_LEN16 == (length)) || \
                                              (CY_I2S_LEN18 == (length)) || \
                                              (CY_I2S_LEN20 == (length)) || \
                                              (CY_I2S_LEN24 == (length)) || \
                                              (CY_I2S_LEN32 == (length)))

#define CY_I2S_IS_OVHDATA_VALID(overhead)    ((CY_I2S_OVHDATA_ZERO == (overhead)) || \
                                              (CY_I2S_OVHDATA_ONE  == (overhead)))

#define CY_I2S_IS_WSPULSE_VALID(wsPulse)     ((CY_I2S_WS_ONE_SCK_CYCLE      == (wsPulse)) || \
                                              (CY_I2S_WS_ONE_CHANNEL_LENGTH == (wsPulse)))

#define CY_I2S_IS_CLK_DIV_VALID(clkDiv)      ((clkDiv) <= 63u)
#define CY_I2S_IS_CHANNELS_VALID(channels)   ((channels) <= 7ul)
#define CY_I2S_IS_INTR_MASK_VALID(interrupt) (0ul == ((interrupt) & ((uint32_t) ~CY_I2S_INTR_MASK)))

#define CY_I2S_IS_CHAN_WORD_VALID(channel, word)        ((CY_I2S_IS_LEN_VALID(channel)) && \
                                                         (CY_I2S_IS_LEN_VALID(word))    && \
                                                         ((channel) >= (word)))
#define CY_I2S_IS_TRIG_LEVEL_VALID(trigLevel, channels) ((trigLevel) <= (255u - (channels)))

/** \endcond */


/**
* \addtogroup group_i2s_functions
* \{
*/

extern cy_en_i2s_status_t Cy_I2S_InitClock(volatile stc_I2S_t * base, cy_stc_i2s_config_clk_t const * config);
extern cy_en_i2s_status_t Cy_I2S_InitTx(volatile stc_I2S_t * base, cy_stc_i2s_config_tx_t const * config);
extern cy_en_i2s_status_t Cy_I2S_InitRx(volatile stc_I2S_t * base, cy_stc_i2s_config_rx_t const * config);
extern void Cy_I2S_DeInit(volatile stc_I2S_t * base);
extern void Cy_I2S_DeInitTx(volatile stc_I2S_t * base);
extern void Cy_I2S_DeInitRx(volatile stc_I2S_t * base);

/** \addtogroup group_i2s_functions_syspm_callback
* The driver supports SysPm callback for Deep Sleep transition.
* \{
*/
cy_en_syspm_status_t     Cy_I2S_DeepSleepCallback(cy_stc_syspm_callback_params_t * callbackParams);
/** \} */

__STATIC_INLINE void     Cy_I2S_StartTxTriggerIntr(volatile stc_I2S_t * base, uint32_t fifo_lv);
__STATIC_INLINE void     Cy_I2S_EnableTx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_PauseTx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_ResumeTx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_DisableTx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_StartRxTriggerIntr(volatile stc_I2S_t * base, uint32_t fifo_lv);
__STATIC_INLINE void     Cy_I2S_EnableRx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_DisableRx(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_StopRx(volatile stc_I2S_t * base);
__STATIC_INLINE uint32_t Cy_I2S_GetCurrentState(volatile stc_I2S_t * base);

__STATIC_INLINE void     Cy_I2S_ClearTxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE uint32_t Cy_I2S_GetNumInTxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_WriteTxData(volatile stc_I2S_t * base, uint32_t data);
__STATIC_INLINE uint8_t  Cy_I2S_GetTxReadPointer(volatile stc_I2S_t * base);
__STATIC_INLINE uint8_t  Cy_I2S_GetTxWritePointer(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_FreezeTxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_UnfreezeTxFifo(volatile stc_I2S_t * base);

__STATIC_INLINE void     Cy_I2S_ClearRxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE uint32_t Cy_I2S_GetNumInRxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE uint32_t Cy_I2S_ReadRxData(volatile stc_I2S_t * base);
__STATIC_INLINE uint32_t Cy_I2S_ReadRxDataSilent(volatile stc_I2S_t * base);
__STATIC_INLINE uint8_t  Cy_I2S_GetRxReadPointer(volatile stc_I2S_t * base);
__STATIC_INLINE uint8_t  Cy_I2S_GetRxWritePointer(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_FreezeRxFifo(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_UnfreezeRxFifo(volatile stc_I2S_t * base);

__STATIC_INLINE uint32_t Cy_I2S_GetInterruptStatus(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_ClearInterrupt(volatile stc_I2S_t * base, uint32_t interrupt);
__STATIC_INLINE void     Cy_I2S_SetInterrupt(volatile stc_I2S_t * base, uint32_t interrupt);
__STATIC_INLINE uint32_t Cy_I2S_GetInterruptMask(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_SetInterruptMask(volatile stc_I2S_t * base, uint32_t interrupt);
__STATIC_INLINE uint32_t Cy_I2S_GetInterruptStatusMasked(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_SetTxTriggerInterruptMask(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_ClearTxTriggerInterruptMask(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_SetRxTriggerInterruptMask(volatile stc_I2S_t * base);
__STATIC_INLINE void     Cy_I2S_ClearRxTriggerInterruptMask(volatile stc_I2S_t * base);


/*******************************************************************************
* Function Name: Cy_I2S_StartTxTriggerIntr
****************************************************************************//**
*
* Starts an I2S transmission. Interrupts enabling (by the 
* \ref Cy_I2S_SetInterruptMask) is required after this function call, in case
* if any I2S interrupts are used in the application.
*
* \pre Cy_I2S_Init() must be called before.
*
* \param base The pointer to the I2S instance address.
*
* \param fifo_lv Fifo threshold number
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_StartTxTriggerIntr(volatile stc_I2S_t * base, uint32_t fifo_lv)
{
    base->unCTL.stcField.u1TX_ENABLED = 1ul;
    base->unTX_FIFO_CTL.stcField.u8TRIGGER_LEVEL = fifo_lv;
}

/*******************************************************************************
* Function Name: Cy_I2S_EnableTx
****************************************************************************//**
*
* Starts an I2S transmission. Interrupts enabling (by the 
* \ref Cy_I2S_SetInterruptMask) is required after this function call, in case
* if any I2S interrupts are used in the application.
*
* \pre Cy_I2S_Init() must be called before.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_EnableTx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1TX_START = 1ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_PauseTx
****************************************************************************//**
*
* Pauses an I2S transmission.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_PauseTx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1TX_PAUSE = 1ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_ResumeTx
****************************************************************************//**
*
* Resumes an I2S transmission.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ResumeTx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1TX_PAUSE = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_DisableTx
****************************************************************************//**
*
* Stops an I2S TX operation.
*
* \pre TX interrupts disabling (by the \ref Cy_I2S_SetInterruptMask) is required
* prior to this function call, in case if any TX I2S interrupts are used.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_DisableTx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1TX_START = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_EnableRx
****************************************************************************//**
*
* Starts an I2S reception. Interrupts enabling (by the 
* \ref Cy_I2S_SetInterruptMask) is required after this function call, in case
* if any I2S interrupts are used in the application.
*
* \pre \ref Cy_I2S_Init() must be called before.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_EnableRx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1RX_START = 1ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_StartRxTriggerIntr
****************************************************************************//**
*
* Starts an I2S reception.
*
* \pre \ref Cy_I2S_Init() must be called before.
*
* \param base The pointer to the I2S instance address.
*
* \param fifo_lv Fifo threshold number
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_StartRxTriggerIntr(volatile stc_I2S_t * base, uint32_t fifo_lv)
{
    base->unCTL.stcField.u1RX_ENABLED = 1ul;
    base->unRX_FIFO_CTL.stcField.u8TRIGGER_LEVEL = fifo_lv;
}


/*******************************************************************************
* Function Name: Cy_I2S_DisableRx
****************************************************************************//**
*
* Stops an I2S reception interrupt.
*
* \pre RX interrupts disabling (by the \ref Cy_I2S_SetInterruptMask) is required
* prior to this function call, in case if any RX I2S interrupts are used.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_DisableRx(volatile stc_I2S_t * base)
{
    base->unCTL.stcField.u1RX_ENABLED = 0ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_StopRx
****************************************************************************//**
*
* Stops an I2S reception.
*
* \pre RX interrupts disabling (by the \ref Cy_I2S_SetInterruptMask) is required
* prior to this function call, in case if any RX I2S interrupts are used.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_StopRx(volatile stc_I2S_t * base)
{
    base->unCMD.stcField.u1RX_START = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetCurrentState
****************************************************************************//**
*
* Returns the current I2S state (TX/RX running/paused/stopped).
*
* \param base The pointer to the I2S instance address.
*
* \return The current state \ref group_i2s_macros_current_state.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetCurrentState(volatile stc_I2S_t * base)
{
    return (base->unCMD.u32Register & (I2S_CMD_TX_START_Msk | I2S_CMD_TX_PAUSE_Msk | I2S_CMD_RX_START_Msk));
}


/*******************************************************************************
* Function Name: Cy_I2S_ClearTxFifo
****************************************************************************//**
*
* Clears the TX FIFO (resets the Read/Write FIFO pointers).
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ClearTxFifo(volatile stc_I2S_t * base)
{
    base->unTX_FIFO_CTL.stcField.u1CLEAR = 1ul;
    base->unTX_FIFO_CTL.stcField.u1CLEAR = 0ul;
    (void) base->unTX_FIFO_CTL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetNumInTxFifo
****************************************************************************//**
*
* Gets the number of used words in the TX FIFO.
*
* \param base The pointer to the I2S instance address.
*
* \return The current number of used words in the TX FIFO.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetNumInTxFifo(volatile stc_I2S_t * base)
{   
    return base->unTX_FIFO_STATUS.stcField.u9USED;
}


/*******************************************************************************
* Function Name: Cy_I2S_WriteTxData
****************************************************************************//**
*
* Writes data to the TX FIFO. Increases the TX FIFO level.
*
* \param base The pointer to the I2S instance address.
*
* \param data Data to be written to the TX FIFO.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_WriteTxData(volatile stc_I2S_t * base, uint32_t data)
{
    base->unTX_FIFO_WR.u32Register = data;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetTxReadPointer
****************************************************************************//**
*
* Gets the TX FIFO Read pointer. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
* \return The current TX Read pointer value.
*
*******************************************************************************/
__STATIC_INLINE uint8_t Cy_I2S_GetTxReadPointer(volatile stc_I2S_t * base)
{
    return (uint8_t)(base->unTX_FIFO_STATUS.stcField.u8RD_PTR);
}


/*******************************************************************************
* Function Name: Cy_I2S_GetTxWritePointer
****************************************************************************//**
*
* Gets the TX FIFO Write pointer. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
* \return The current TX Write pointer value.
*
*******************************************************************************/
__STATIC_INLINE uint8_t Cy_I2S_GetTxWritePointer(volatile stc_I2S_t * base)
{
    return (uint8_t)(base->unTX_FIFO_STATUS.stcField.u8WR_PTR);
}


/*******************************************************************************
* Function Name: Cy_I2S_FreezeTxFifo
****************************************************************************//**
*
* Freezes the TX FIFO. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_FreezeTxFifo(volatile stc_I2S_t * base)
{
    base->unTX_FIFO_CTL.stcField.u1FREEZE = 1ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_UnfreezeTxFifo
****************************************************************************//**
*
* Unfreezes the TX FIFO. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_UnfreezeTxFifo(volatile stc_I2S_t * base)
{
    base->unTX_FIFO_CTL.stcField.u1FREEZE = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_ClearRxFifo
****************************************************************************//**
*
* Clears the RX FIFO (resets the Read/Write FIFO pointers).
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ClearRxFifo(volatile stc_I2S_t * base)
{
    base->unRX_FIFO_CTL.stcField.u1CLEAR = 1ul;
    base->unRX_FIFO_CTL.stcField.u1CLEAR = 0ul;
    (void) base->unRX_FIFO_CTL.u32Register;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetNumInRxFifo
****************************************************************************//**
*
* Gets the number of used words in the RX FIFO.
*
* \param base The pointer to the I2S instance address.
*
* \return The current number of used words in rge RX FIFO.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetNumInRxFifo(volatile stc_I2S_t * base)
{
    return base->unRX_FIFO_STATUS.stcField.u9USED;
}


/*******************************************************************************
* Function Name: Cy_I2S_ReadRxData
****************************************************************************//**
*
* Reads data from the RX FIFO. Decreases the RX FIFO level.
*
* \param base The pointer to the I2S instance address.
*
* \return The read data.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_ReadRxData(volatile stc_I2S_t * base)
{
    return (base->unRX_FIFO_RD.u32Register);
}


/*******************************************************************************
* Function Name: Cy_I2S_ReadRxDataSilent
****************************************************************************//**
*
* Reads data from the RX FIFO without updating the RX FIFO read pointer.
* This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
* \return The read data.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_ReadRxDataSilent(volatile stc_I2S_t * base)
{
    return (base->unRX_FIFO_RD_SILENT.u32Register);
}


/*******************************************************************************
* Function Name: Cy_I2S_GetRxReadPointer
****************************************************************************//**
*
* Gets the RX FIFO Read pointer. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
* \return The current RX Read pointer value.
*
*******************************************************************************/
__STATIC_INLINE uint8_t Cy_I2S_GetRxReadPointer(volatile stc_I2S_t * base)
{
    return (uint8_t)(base->unRX_FIFO_STATUS.stcField.u8RD_PTR);
}


/*******************************************************************************
* Function Name: Cy_I2S_GetRxWritePointer
****************************************************************************//**
*
* Gets the RX FIFO Write pointer. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
* \return The current RX Write pointer value.
*
*******************************************************************************/
__STATIC_INLINE uint8_t Cy_I2S_GetRxWritePointer(volatile stc_I2S_t * base)
{
    return (uint8_t)(base->unRX_FIFO_STATUS.stcField.u8WR_PTR);
}


/*******************************************************************************
* Function Name: Cy_I2S_FreezeRxFifo
****************************************************************************//**
*
* Freezes the RX FIFO. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_FreezeRxFifo(volatile stc_I2S_t * base)
{
    base->unRX_FIFO_CTL.stcField.u1FREEZE = 1ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_UnfreezeRxFifo
****************************************************************************//**
*
* Unfreezes the RX FIFO. This function is rather for debug purposes.
*
* \param base The pointer to the I2S instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_UnfreezeRxFifo(volatile stc_I2S_t * base)
{
    base->unRX_FIFO_CTL.stcField.u1FREEZE = 0ul;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetInterruptStatus
****************************************************************************//**
*
* Gets an interrupt status (returns a content of the INTR register).
*
* \param base The pointer to the I2S instance address.
*
* \return The interrupt bit mask \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetInterruptStatus(volatile stc_I2S_t * base)
{
    return (base->unINTR.u32Register);
}


/*******************************************************************************
* Function Name: Cy_I2S_ClearInterrupt
****************************************************************************//**
*
* Clears one or more interrupt factors (sets the INTR register).
*
* \param base      The pointer to the I2S instance address.
*
* \param interrupt Interrupt bit mask \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ClearInterrupt(volatile stc_I2S_t * base, uint32_t interrupt)
{
    CY_ASSERT_L2(CY_I2S_IS_INTR_MASK_VALID(interrupt));
    base->unINTR.u32Register = interrupt;
    (void) base->unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_I2S_SetInterrupt
****************************************************************************//**
*
* Sets one or more interrupt factors (sets the INTR_SET register).
*
* \param base      The pointer to the I2S instance address.
*
* \param interrupt Interrupt bit mask \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_SetInterrupt(volatile stc_I2S_t * base, uint32_t interrupt)
{
    CY_ASSERT_L2(CY_I2S_IS_INTR_MASK_VALID(interrupt));
    base->unINTR_SET.u32Register = interrupt;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetInterruptMask
****************************************************************************//**
*
* Returns the interrupt mask (a content of the INTR_MASK register).
*
* \param base The pointer to the I2S instance address.
*
* \return The interrupt bit mask \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetInterruptMask(volatile stc_I2S_t * base)
{
    return (base->unINTR_MASK.u32Register);
}


/*******************************************************************************
* Function Name: Cy_I2S_SetInterruptMask
****************************************************************************//**
*
* Sets one or more interrupt factor masks (the INTR_MASK register).
*
* \param base      The pointer to the I2S instance address.
*
* \param interrupt Interrupt bit mask \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_SetInterruptMask(volatile stc_I2S_t * base, uint32_t interrupt)
{
    CY_ASSERT_L2(CY_I2S_IS_INTR_MASK_VALID(interrupt));
    base->unINTR_MASK.u32Register = interrupt;
}


/*******************************************************************************
* Function Name: Cy_I2S_GetInterruptStatusMasked
****************************************************************************//**
*
* Returns the interrupt status masked (a content of the INTR_MASKED register).
*
* \param base The pointer to the I2S instance address.
*
* \return The interrupt bit mask(s) \ref group_i2s_macros_intrerrupt_masks.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_I2S_GetInterruptStatusMasked(volatile stc_I2S_t * base)
{
    return (base->unINTR_MASKED.u32Register);
}

/*******************************************************************************
* Function Name: Cy_I2S_SetTxTriggerInterruptMask
****************************************************************************//**
*
* Sets TX Trigger Interrupt mask.
*
* \param base      The pointer to the I2S instance address.
*
* \return None.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_SetTxTriggerInterruptMask(volatile stc_I2S_t * base)
{
    base->unINTR_MASK.stcField.u1TX_TRIGGER = 1ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_ClearTxTriggerInterruptMask
****************************************************************************//**
*
* Clears TX Trigger Interrupt mask.
*
* \param base      The pointer to the I2S instance address.
*
* \return None.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ClearTxTriggerInterruptMask(volatile stc_I2S_t * base)
{
    base->unINTR_MASK.stcField.u1TX_TRIGGER = 0ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_SetRxTriggerInterruptMask
****************************************************************************//**
*
* Sets RX Trigger Interrupt mask.
*
* \param base      The pointer to the I2S instance address.
*
* \return None.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_SetRxTriggerInterruptMask(volatile stc_I2S_t * base)
{
    base->unINTR_MASK.stcField.u1RX_TRIGGER = 1ul;
}

/*******************************************************************************
* Function Name: Cy_I2S_ClearRxTriggerInterruptMask
****************************************************************************//**
*
* Clears RX Trigger Interrupt mask.
*
* \param base      The pointer to the I2S instance address.
*
* \return None.
*
*******************************************************************************/
__STATIC_INLINE void Cy_I2S_ClearRxTriggerInterruptMask(volatile stc_I2S_t * base)
{
    base->unINTR_MASK.stcField.u1RX_TRIGGER = 0ul;
}

/** \} group_i2s_functions */

#ifdef __cplusplus
}
#endif

#endif /* CY_IP_MXAUDIOSS */

#endif /* CY_I2S_H */

/** \} group_i2s */


/* [] END OF FILE */
