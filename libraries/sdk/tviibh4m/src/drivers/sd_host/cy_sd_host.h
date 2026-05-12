/***************************************************************************//**
* \file cy_sd_host.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
*  This file provides constants and parameter values for 
*  the SD Host Controller driver.
*
********************************************************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sd_host SD Host Controller (SD Host)
* \{
* This driver provides the user an easy method for accessing standard 
* Host Controller Interface (HCI) registers and provides some simple
* functionality on top of the HCI for reading and writing data to 
* an SD card, eMMc card or a SDIO device.
*
* Features:
* * Supports data transfer using CPU, SDMA, ADMA2 and ADMA3 modes
* * Supports a configurable block size (1 to 65,535 Bytes)
* * Supports interrupt enabling and masking
* * Supports SD-HCI Host version 4 mode or less
* * Compliant with the SD 6.0, SDIO 4.10 and eMMC 5.1 specifications and earlier versions
* * SD interface features:
* * - Supports the 4-bit interface
* * - Supports Ultra High Speed (UHS-I) mode
* * - Supports Default Speed (DS), High Speed (HS), SDR12, SDR25 and SDR50 speed modes
* * - Supports SDIO card interrupts in both 1-bit and 4-bit modes 
* * - Supports Standard capacity (SDSC), High capacity (SDHC) 
* and Extended capacity (SDXC) memory
* * - Supports CRC and check for command and data packets
* * - Supports packet timeouts
* * - Handles the SDIO card interrupt
* * eMMC interface features:
* * - Supports 1-bit/4-bit/8-bit interfaces
* * - Supports legacy and High Speed SDR speed modes
* * - Supports CRC and check for command and data packets
* * - Supports packet timeouts
*
* Unsupported Features:  
* * Wrap address transfers
* * eMMC boot operation
* * Suspend/Resume operation in an SDIO card
* * Operation in SDR104, UHS-II mode, High Speed DDR, HS200, and HS400
* * Serial Peripheral Interface (SPI) protocol mode
* * Interrupt input pins for the embedded SD system
* * Auto-tuning
* * Command queuing
*
* The driver has a low-level and high-level APIs. 
* The low-level functions provide an easy method to read and write registers. 
* Also, these functions allow valid interaction with an SD Card, eMMC card, 
* and SDIO card.
* The high-level functions provide an easy mechanism to enumerate a device,
* read, write, and erase data. They are RTOS-friendly. 
* When starting a command, these functions do not wait until the command completes. 
* The interrupt and flags are used to check when the transfer completes. 
* This allows to put RTOS delays in the user code.
*
* \section group_sd_host_section_Configuration_Considerations Configuration Considerations
* The SD Host driver configuration can be divided to a number of sequential
* steps listed below:
* * \ref group_sd_host_enable
* * \ref group_sd_host_pins
* * \ref group_sd_host_clock
* * \ref group_sd_host_intr
* * \ref group_sd_host_config
* * \ref group_sd_host_card_init
*
* \note
* The SD Host driver is built on top of the SDHC hardware block. The SDHC1 instance is
* used as an example for all code snippets. Modify the code to match your
* design.
*
* \subsection group_sd_host_enable Enable SD Host
* Enable the SDHC block calling \ref Cy_SD_Host_Enable.
*
* \subsection group_sd_host_pins Assign and Configure Pins
* Only dedicated SD Host pins can be used for SD Host operation. The HSIOM
* register must be configured to connect the block to the pins. Also, the SD Host 
* pins must be configured in Strong Drive, Input buffer on:
*
* \snippet sd_host\1.0\snippet\main.c SD_HOST_CFG_PINS
*
* \subsection group_sd_host_clock Assign Clock Source
* The SD Host is sourced from the CLK_HF clock. The clock must be set to 100 MHz:
*
* \snippet sd_host\1.0\snippet\main.c SD_HOST_CFG_ASSIGN_CLOCK
*
* \subsection group_sd_host_intr Configure Interrupt (Optional)
* The user can set up the interrupt for SD Host operation. 
* The user is responsible for writing its own Interrupt handler.
* The Interrupt must be called in the interrupt handler for the selected SDHC
* instance. Also this interrupt must be enabled in the NVIC otherwise
* it will not work.
* It is the user's responsibility to clear the normal and error interrupt statuses. 
* The interrupt statuses can be read using \ref Cy_SD_Host_GetNormalInterruptStatus 
* and \ref Cy_SD_Host_GetErrorInterruptStatus. 
* To clear the interrupt statuses, use \ref Cy_SD_Host_ClearNormalInterruptStatus 
* and \ref Cy_SD_Host_ClearErrorInterruptStatus.
*
* \snippet sd_host\1.0\snippet\main.c SD_HOST_INTR_A
* \snippet sd_host\1.0\snippet\main.c SD_HOST_INTR_B
*
* \subsection group_sd_host_config Configure SD Host
* To set up the SD Host driver, provide the configuration parameters in the
* \ref cy_stc_sd_host_init_config_t structure. Set the emmc parameter to true for 
* the eMMC-device, otherwise set it to false. Set dmaType if DMA mode 
* is used for read/write operations. The other parameters are optional for
* operation. To initialize the driver, call the \ref Cy_SD_Host_Init
* function providing a pointer to the filled \ref cy_stc_sd_host_init_config_t
* structure and allocated \ref cy_stc_sd_host_context_t.
*
* \snippet sd_host\1.0\snippet\main.c SD_HOST_CONTEXT
* \snippet sd_host\1.0\snippet\main.c SD_HOST_CFG
* 
* The SD, eMMC or SDIO card can be configured using the \ref Cy_SD_Host_InitCard
* function as a pointer to the filled \ref cy_stc_sd_host_sd_card_config_t
* structure and allocated \ref cy_stc_sd_host_context_t.
*
* \subsection group_sd_host_card_init Initialize the card
* Finally, enable the card operation calling 
* \ref Cy_SD_Host_InitCard.
*
* \snippet sd_host\1.0\snippet\main.c SD_HOST_ENABLE_CARD_INIT
*
* \section group_sd_host_use_cases Common Use Cases
*
* \subsection group_sd_host_sd_card_mode SD card Operation
* The master API is divided into two categories:
* \ref group_sd_host_high_level_functions and
* \ref group_sd_host_low_level_functions. Therefore, there are two
* methods for initiating SD card transactions using either <b>Low-Level</b> or
* <b>High-Level</b> API.
*
* \subsubsection group_sd_host_master_hl Use High-Level Functions
* Call \ref Cy_SD_Host_Read or \ref Cy_SD_Host_Write to
* communicate with the SD memory device. These functions do not block 
* and only start a transaction. After a transaction starts, 
* the user should check the further data-transaction complete event.
*
* \subsubsection group_sd_host_master_ll Use Low-Level Functions
* Call \ref Cy_SD_Host_InitDataTransfer to initialize the SD block  
* for a data transfer. It does not start a transfer. To start a transfer 
* call \ref Cy_SD_Host_SendCommand after calling this function. 
* If DMA is not used for Data transfer then the buffer needs to be filled 
* with data first if this is a write.
* Wait the transfer complete event.
*
* \section group_sd_host_lp Low Power Support
* The SD Host does not operate in Hibernate and Deep Sleep modes but it
* can automatically continue write/read operation after restoring from
* Deep Sleep mode.
* To reduce the power consumption in Active mode, the user can stop 
* the clock of the SD bus but the following interrupts can be allowed: 
* Card Insert, Card Removal and SDIO Interrupt. 

* \section group_sd_host_remove_insert SD Card Removal and Insertion
* SD card removal or insertion can be detected by calling 
* \ref Cy_SD_Host_GetNormalInterruptStatus which returns 
* the card removal or card insertion events 
* (CY_SD_HOST_CARD_REMOVAL or CY_SD_HOST_CARD_INSERTION bits). 
* These events should be reset using 
* \ref Cy_SD_Host_ClearNormalInterruptStatus when they occur. 
* When the card is removed, the SDHC block disables the CMD/DAT output. 
* It is recommended to set DAT pins to 
* the Digital High-Z (CY_GPIO_DM_HIGHZ) drive mode when 
* card removal is detected. This can be doing using the GPIO driver. 
* When the card is inserted, the SDHC block automatically disables 
* the card power and clock. After card insertion, 
* the user should set the DAT pins drive mode back to Strong Drive, 
* Input buffer on (CY_GPIO_DM_STRONG), and then call \ref Cy_SD_Host_InitCard.
* \note If CARD_INTERRUPT is enabled and DAT pins are not set to 
* Digital High-Z drive mode then the interrupt will continuously 
* trigger because the DAT1 line is driven low upon card re-insertion. 
* The user will have to detect the card removal in the ISR handler, 
* apply the power to the card using \ref Cy_SD_Host_EnableCardVoltage, 
* set to the DAT pins drive mode to the Digital High-Z (CY_GPIO_DM_HIGHZ) 
* and clear CY_SD_HOST_CARD_INTERRUPT bit
* using \ref Cy_SD_Host_ClearNormalInterruptStatus.
*
* \section group_sd_host_more_information More Information
*
* Refer to the appropriate device technical reference manual (TRM) for 
* a detailed description of the registers.
*
* \section group_sd_host_Changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>6.6.0</td>
*     <td>-SD output clock macros moved to cy_sd_host.h
*         -cy_en_sd_host_bus_speed_mode_t updated with DDR50
*         -Cy_SD_Host_SdCardChangeClock made as global
*         -CY_SD_HOST_DDR50_SPEED added into cy_sd_host.h
*         -Cy_SD_Host_SetBusSpeedMode updated to support various speeds</td>
*     <td>Enhancement of driver to support DDR</td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_sd_host_macros Macros
* \defgroup group_sd_host_functions Functions
* \{
* \defgroup group_sd_host_high_level_functions High-Level
* \defgroup group_sd_host_low_level_functions Low-Level
* \defgroup group_sd_host_interrupt_functions Interrupt
* \}
* \defgroup group_sd_host_data_structures Data Structures
* \defgroup group_sd_host_enums Enumerated Types
*/

#ifndef CY_SD_HOST_PDL_H
#define CY_SD_HOST_PDL_H

#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"

#if defined(CY_IP_MXSDHC)

#if defined (__CC_ARM)
    #pragma anon_unions    
#endif

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
* \addtogroup group_sd_host_macros
* \{
*/

/** Driver major version */
#define CY_SD_HOST_DRV_VERSION_MAJOR       `$CY_MAJOR_VERSION`

/** Driver minor version */
#define CY_SD_HOST_DRV_VERSION_MINOR       `$CY_MINOR_VERSION`

/******************************************************************************
* API Constants
******************************************************************************/

#define CY_SD_HOST_BLOCK_SIZE               (512UL) /**< The SD memory card block size. */

/* The card states. */
#define CY_SD_HOST_CARD_IDLE                (0x0UL)  /**< The card idle state. */
#define CY_SD_HOST_CARD_READY               (0x1UL)  /**< The card ready state. */
#define CY_SD_HOST_CARD_IDENT               (0x2UL)  /**< The card identification state. */
#define CY_SD_HOST_CARD_STBY                (0x3UL)  /**< The card stand-by state. */
#define CY_SD_HOST_CARD_TRAN                (0x4UL)  /**< The card transition state. */
#define CY_SD_HOST_CARD_DATA                (0x5UL)  /**< The card sending-data state. */
#define CY_SD_HOST_CARD_RCV                 (0x6UL)  /**< The card receive-data state. */
#define CY_SD_HOST_CARD_PRG                 (0x7UL)  /**< The card programming state. */
#define CY_SD_HOST_CARD_DIS                 (0x8UL)  /**< The card disconnect state. */

/* The card status (CMD13) bits. */
#define CY_SD_HOST_CMD13_OUT_OF_RANGE       (31U) /**< The command's argument is out of range. */ 
#define CY_SD_HOST_CMD13_ADDRESS_ERROR      (30U) /**< The address does not match the block length. */ 
#define CY_SD_HOST_CMD13_BLOCK_LEN_ERROR    (29U) /**< The block length is not allowed for this card. */ 
#define CY_SD_HOST_CMD13_ERASE_SEQ_ERROR    (28U) /**< An error in the sequence of erase commands occurred. */ 
#define CY_SD_HOST_CMD13_ERASE_PARAM        (27U) /**< An invalid selection of write blocks for erase occurred. */ 
#define CY_SD_HOST_CMD13_WP_VIOLATION       (26U) /**< The host attempts to write to a protected block 
                                                    * or to the permanent write-protected card. 
                                                    */ 
#define CY_SD_HOST_CMD13_CARD_IS_LOCKED     (25U) /**< The card is locked by the host. */
#define CY_SD_HOST_CMD13_LOCK_ULOCK_FAILED  (24U) /**< A sequence or password error occurred 
                                                    * has been detected in the lock/unlock card command. 
                                                    */
#define CY_SD_HOST_CMD13_COM_CRC_ERROR      (23U) /**< The CRC of the previous command failed. */
#define CY_SD_HOST_CMD13_ILLEGAL_COMMAND    (22U) /**< The command is not legal for the card state. */
#define CY_SD_HOST_CMD13_CARD_ECC_FAILED    (21U) /**< The card internal ECC failed. */
#define CY_SD_HOST_CMD13_CC_ERROR           (20U) /**< An internal card-controller error. */
#define CY_SD_HOST_CMD13_ERROR              (19U) /**< A general or unknown error occurred. */
#define CY_SD_HOST_CMD13_CSD_OVERWRITE      (16U) /**< Can be either one of the following
                                                    * errors: 
                                                    * - The read only section of the CSD
                                                    * does not match the card content.
                                                    * - An attempt to reverse the copy (set
                                                    * as original) or permanent WP
                                                    * (unprotected) bits was made. 
                                                    */
#define CY_SD_HOST_CMD13_WP_ERASE_SKIP      (15U) /**< Set when only partial address space
                                                    * was erased due to existing write protected blocks 
                                                    * or a temporary or permanent write protected
                                                    * card was erased. 
                                                    */
#define CY_SD_HOST_CMD13_CARD_ECC_DISABLED  (14U) /**< The command has been executed
                                                    * without using the internal ECC. 
                                                    */                                                  
#define CY_SD_HOST_CMD13_CURRENT_STATE      (9U)  /**< The state of the card. */
#define CY_SD_HOST_CMD13_READY_FOR_DATA     (8U)  /**< The buffer is empty on the bus. */
#define CY_SD_HOST_CMD13_APP_CMD            (5U)  /**< The card will expect ACMD, or an
                                                    * indication that the command has
                                                    * been interpreted as ACMD. 
                                                    */
#define CY_SD_HOST_CMD13_AKE_SEQ_ERROR      (3U)  /**< Error in the sequence of the
                                                    * authentication process. 
                                                    */

#define CY_SD_HOST_CMD13_CURRENT_STATE_MSK  (0x00001E00UL) /**< The current state mask of the card. */

/** \cond INTERNAL_MACROS */

#define CY_SD_HOST_EMMC_T_RSTW_US           (1U)
#define CY_SD_HOST_EMMC_T_RSCA_US           (200U)
#define CY_SD_HOST_INIT_CLK_FREQUENCY_KHZ   (400U)  /* The CLK frequency in kHz during card initialization. */
#define CY_SD_HOST_3_PERIODS_US             ((1000U * 3U / CY_SD_HOST_INIT_CLK_FREQUENCY_KHZ) + 1U)

/** \endcond */

/** \} group_sd_host_macros */

/**
* \defgroup group_sd_host_macros_events SD Host Events
* \{
* The constants below can be used with 
* \ref Cy_SD_Host_GetNormalInterruptStatus, 
* \ref Cy_SD_Host_ClearNormalInterruptStatus, 
* \ref Cy_SD_Host_GetErrorInterruptStatus and  
* \ref Cy_SD_Host_ClearErrorInterruptStatus functions. 
* Each event is encoded in a separate bit, and therefore it is possible to
* notify about multiple events.
*/
/**
* Command complete. In SD/eMMC mode, this event is set 
* after detecting the end bit of a response except for Auto CMD12 and Auto CMD23.
* This event is not generated when the Response Interrupt is disabled.
*/
#define CY_SD_HOST_CMD_COMPLETE            (0x0001U)

/**
* Transfer complete. This event is set when a read/write 
* transfer and a command with the Busy Status are completed.
*/
#define CY_SD_HOST_XFER_COMPLETE           (0x0002U)

/**
* Block gap. This event is set when both read/write 
* transactions are stopped at the block gap due to a 
* Stop-at-Block-Gap Request.
*/
#define CY_SD_HOST_BGAP                    (0x0004U)

/**
* DMA Interrupt. This event is set if the Host Controller 
* detects a SDMA Buffer Boundary during a transfer. 
* For ADMA, the Host controller generates this
* interrupt by setting the Int field in the 
* descriptor table. 
* This interrupt is not generated after a Transfer
* Complete.
*/
#define CY_SD_HOST_DMA_INTERRUPT           (0x0008U)

/**
* The Buffer Write is ready. This event is set if 
* the Buffer Write Enable changes from 0 to 1.
*/
#define CY_SD_HOST_BUF_WR_READY            (0x0010U)

/**
* The Buffer Read is ready. This event is set if 
* the Buffer Read Enable changes from 0 to 1.
*/
#define CY_SD_HOST_BUF_RD_READY            (0x0020U)

/**
* Card insertion. This event is set if 
* the Card Inserted in the Present State
* register changes from 0 to 1.
*/
#define CY_SD_HOST_CARD_INSERTION          (0x0040U)

/**
* Card removal. This event is set if 
* the Card Inserted in the Present State
* register changes from 1 to 0.
*/
#define CY_SD_HOST_CARD_REMOVAL            (0x0080U)

/**
* The Card interrupt. This event reflects the
* synchronized value of DAT[1] Interrupt Input for SD Mode
*/
#define CY_SD_HOST_CARD_INTERRUPT          (0x0100U)

/**
* FX event.
* This status is set when R[14] of the response register is set to 1
* and Response Type R1/R5 is set to 0 in the Transfer Mode
* register. This interrupt is used with the response check function.
*/
#define CY_SD_HOST_FX_EVENT                (0x2000U)

/**
* Command Queuing event.
* This status is set if a Command Queuing/Crypto related event
* has occurred in eMMC/SD mode.
*/
#define CY_SD_HOST_CQE_EVENT               (0x4000U)

/**
* Error Interrupt. 
* If any of the bits in the Error Interrupt Status 
* register are set, then this bit is set.
*/
#define CY_SD_HOST_ERR_INTERRUPT           (0x8000U)

/**
* Command timeout error. In SD/eMMC Mode, 
* this event is set only if no response is returned 
* within 64 SD clock cycles from the end bit of the
* command. If the Host Controller detects a CMD line conflict,
* along with Command CRC Error bit, this event is set to 1,
* without waiting for 64 SD/eMMC card clock cycles. 
*/
#define CY_SD_HOST_CMD_TOUT_ERR            (0x0001U)

/**
* Command CRC error. A Command CRC Error is generated 
* in SD/eMMC mode when:
* 1. A response is returned and the Command Timeout
* Error is set to 0 (indicating no timeout),
* this bit is set to 1 when detecting a CRC error 
* in the command response.
* 2. The Host Controller detects a CMD line conflict by
* monitoring the CMD line when a command is issued. If
* the Host Controller drives the CMD line to level 1, but
* detects level 0 on the CMD line at the next SD clock
* edge, then the Host Controller aborts the command (stop
* driving CMD line) and sets this bit to 1. The Command
* Timeout Error is also set to 1 to distinguish a CMD line
* conflict.
*/
#define CY_SD_HOST_CMD_CRC_ERR             (0x0002U)

/**
* Command End Bit error.
* This bit is set after detecting that the end bit of a command
* response is 0 in SD/eMMC mode. 
*/
#define CY_SD_HOST_CMD_END_BIT_ERR         (0x0004U)

/**
* Command Index error.
* This bit is set if a Command Index error occurs in the
* command response in SD/eMMC mode. 
*/
#define CY_SD_HOST_CMD_IDX_ERR             (0x0008U)

/**
* Data Timeout error.
* This bit is set in SD/eMMC mode when detecting one of the
* following timeout conditions:
* *  Busy timeout for R1b, R5b type
* *  Busy timeout after Write CRC status
* *  Write CRC Status timeout
* *  Read Data timeout.
*/
#define CY_SD_HOST_DATA_TOUT_ERR           (0x0010U)

/**
* Data CRC error.
* This error occurs in SD/eMMC mode after detecting a CRC
* error while transferring read data that uses the DAT line,
* after detecting the Write CRC status having a value other
* than 010 or when writing a CRC status timeout.
*/
#define CY_SD_HOST_DATA_CRC_ERR            (0x0020U)

/**
* Data End Bit error.
* This error occurs in SD/eMMC mode either when detecting 0
* at the end bit position of read data that uses the DAT line or
* at the end bit position of the CRC status.
*/
#define CY_SD_HOST_DATA_END_BIT_ERR        (0x0040U)

/**
* Current Limit error.
* The SD Host driver does not support this function,
* this bit is always set to 0.
*/
#define CY_SD_HOST_CUR_LMT_ERR             (0x0080U)

/**
* Auto CMD error.
* This error status is used by Auto CMD12 and Auto CMD23 in
* SD/eMMC mode. This bit is set after detecting that any of
* the bits D00 to D05 in the Auto CMD Error Status register has
* changed from 0 to 1. D07 is effective in case for Auto CMD12.
* The Auto CMD Error Status register is valid while this bit is set to
* 1 and may be cleared by clearing this bit.
*/
#define CY_SD_HOST_AUTO_CMD_ERR            (0x0100U)

/**
* ADMA error.
* This bit is set when the Host Controller detects an error during
* an ADMA-based data transfer. The possible reasons for an error:
* * An error response is received from the System bus;
* * ADMA3, ADMA2 Descriptors are invalid;
* * CQE Task or Transfer descriptors are invalid.
* When an error occurs, the state of the ADMA is saved in the
* ADMA Error Status register.
*/
#define CY_SD_HOST_ADMA_ERR                (0x0200U)

/**
* Tuning error.
* The SD Host driver does not support this function.
*/
#define CY_SD_HOST_TUNING_ERR              (0x0400U)

/**
* Response error.
* Host Controller Version 4.00 supports the response error check
* function to avoid overhead of the response error check by Host
* Driver during DMA execution. If the Response Error Check
* Enable is set to 1 in the Transfer Mode register, the Host
* Controller checks R1 or R5 response. If an error is detected
* in a response, this bit is set to 1. This is applicable in
* SD/eMMC mode.
*/
#define CY_SD_HOST_RESP_ERR                (0x0800U)

/**
* Boot Acknowledgement error.
* This bit is set when there is a timeout for boot
* acknowledgement or after detecting the boot ACK status having
* a value other than 010. This is applicable only when boot
* acknowledgement is expected in eMMC mode.
*/
#define CY_SD_HOST_BOOT_ACK_ERR            (0x1000U)

/** \} group_sd_host_macros_events */

/**
* \defgroup group_sd_host_macros_present_status SD Host Present Status
* \{
* The constants below can be used with the 
* \ref Cy_SD_Host_GetPresentState function. 
* Each status is encoded in a separate bit, and therefore it is possible to
* notify about multiple statuses.
*/

/**
* Command Inhibit (CMD).
* This bit indicates the following:
* - SD/eMMC mode: If this bit is set to 0, it indicates that the
* CMD line is not in use and the Host controller can issue
* an SD/eMMC command using the CMD line. This bit is
* set when the command register is written. This bit is
* cleared when the command response is received. This bit
* is not cleared by the response of auto CMD12/23 but
* cleared by the response of the Read/Write command.
*/
#define CY_SD_HOST_CMD_INHIBIT             (0x00000001UL)

/**
* Command Inhibit (DAT).
* This bit is applicable for SD/eMMC mode and is generated if
* either the DAT line active or Read transfer active is set to 1. If
* this bit is set to 0, it indicates that the Host Controller can
* issue subsequent SD/eMMC commands. 
*/
#define CY_SD_HOST_CMD_CMD_INHIBIT_DAT     (0x00000002UL)

/**
* DAT Line Active (SD/eMMC Mode only).
* This bit indicates whether one of the DAT lines on the
* SD/eMMC bus is in use.
* For Read transactions, this bit indicates whether a
* read transfer is executing on the SD/eMMC bus.
* For Write transactions, this bit indicates whether a
* write transfer is executing on the SD/eMMC bus.
* For a command with the Busy status, this status indicates whether the
* command executing busy is executing on an SD or eMMC
* bus. 
*/
#define CY_SD_HOST_DAT_LINE_ACTIVE         (0x00000004UL)

/**
* DAT[7:4] Line Signal Level.
* These bits are used to check the DAT line level to recover from
* errors and for debugging. These bits reflect the value of the
* sd_dat_in (upper nibble) signal. 
*/
#define CY_SD_HOST_DAT_7_4                 (0x000000F0UL)

/**
* Write Transfer Active
* This status indicates whether the Write transfer is active for
* SD/eMMC mode.
*/
#define CY_SD_HOST_WR_XFER_ACTIVE          (0x00000100UL)

/**
* Read Transfer Active.
* This bit indicates whether the Read transfer is active for
* SD/eMMC mode.
*/
#define CY_SD_HOST_RD_XFER_ACTIVE          (0x00000200UL)

/**
* Buffer Write Enable.
* This bit is used for non-DMA transfers. This bit is set if space
* is available for writing data.
*/
#define CY_SD_HOST_BUF_WR_ENABLE           (0x00000400UL)

/**
* Buffer Read Enable.
* This bit is used for non-DMA transfers. This bit is set if valid
* data exists in the Host buffer.
*/
#define CY_SD_HOST_BUF_RD_ENABLE           (0x00000800UL)

/**
* Card Inserted.
* This bit indicates whether a card has been inserted. The
* Host Controller debounces this signal so that the Host Driver
* does not need to wait for the signal to stabilize.
*/
#define CY_SD_HOST_CARD_INSERTED           (0x00010000UL)

/**
* Card Stable.
* This bit indicates the stability of the Card Detect Pin Level. A
* card is not detected if this bit is set to 1 and the value of the
* CARD_INSERTED bit is 0.
*/
#define CY_SD_HOST_CARD_STABLE             (0x00020000UL)

/**
* Card Detect Pin Level.
* This bit reflects the inverse synchronized value of the
* card_detect_n signal.
*/
#define CY_SD_HOST_CARD_DETECT_PIN_LEVEL   (0x00040000UL)

/**
* Write Protect Switch Pin Level.
* This bit is supported only for memory and combo cards. This
* bit reflects the synchronized value of the card_write_prot
* signal.
*/
#define CY_SD_HOST_WR_PROTECT_SW_LVL       (0x00080000UL)

/**
* DAT[3:0] Line Signal Level.
* This bit is used to check the DAT line level to recover from
* errors and for debugging. These bits reflect the value of the
* sd_dat_in (lower nibble) signal.
*/
#define CY_SD_HOST_DAT_3_0                 (0x00F00000UL)

/**
* Command-Line Signal Level.
* This bit is used to check the CMD line level to recover from
* errors and for debugging. These bits reflect the value of the
* sd_cmd_in signal.
*/
#define CY_SD_HOST_CMD_LINE_LVL            (0x01000000UL)

/**
* Host Regulator Voltage Stable.
* This bit is used to check whether the host regulator voltage is
* stable for switching the voltage of UHS-I mode. This bit
* reflects the synchronized value of the host_reg_vol_stable
* signal.
*/
#define CY_SD_HOST_HOST_REG_VOL            (0x02000000UL)

/**
* Command Not Issued by Error.
* This bit is set if a command cannot be issued after setting
* the command register due to an error except an Auto
* CMD12 error.
*/
#define CY_SD_HOST_CMD_ISSU_ERR            (0x08000000UL)

/**
* Sub Command Status.
* This bit is used to distinguish between a main command and
* a sub command status.
*/
#define CY_SD_HOST_SUB_CMD_STAT            (0x10000000UL)

/**
* In Dormant Status.
* This bit indicates whether UHS-II lanes enter the Dormant state
* in the UHS-II mode. For SD/eMMC mode, this bit always
* returns 0.
*/
#define CY_SD_HOST_IN_DORMANT_ST           (0x20000000UL)

/**
* Lane Synchronization.
* This bit indicates whether a lane is synchronized in UHSII
* mode. For SD/eMMC mode, this bit always returns 0.
*/
#define CY_SD_HOST_LANE_SYNC               (0x40000000UL)

/**
* UHS-II Interface Detection.
* This bit indicates whether a card supports the UHS-II
* interface. For SD/eMMC mode, this bit always returns 0.
*/
#define CY_SD_HOST_UHS2_IF_DETECT          (0x80000000UL)

/** SD output clock. */
#define CY_SD_HOST_CLK_400K                 (400UL * 1000UL)    /* 400 kHz. */
#define CY_SD_HOST_CLK_10M                  (10UL * 1000UL * 1000UL) /* 10 MHz. */
#define CY_SD_HOST_CLK_20M                  (20UL * 1000UL * 1000UL) /* 20 MHz. */
#define CY_SD_HOST_CLK_40M                  (40UL * 1000UL * 1000UL) /* 40 MHz. */
#define CY_SD_HOST_CLK_50M                  (50UL * 1000UL * 1000UL) /* 50 MHz. */

/** \} group_sd_host_macros_present_status */


/**
* \addtogroup group_sd_host_enums
* \{
*/

/******************************************************************************
 * Enumerations
 *****************************************************************************/
 
/** SD command types. */                 
typedef enum
{
    CY_SD_HOST_CMD_NORMAL     = 0U,  /**< Other commands */
    CY_SD_HOST_CMD_SUSPEND    = 1U,  /**< CMD52 for writing "Bus Suspend" in CCCR */
    CY_SD_HOST_CMD_RESUME     = 2U,  /**< CMD52 for writing "Function Select" in CCCR */
    CY_SD_HOST_CMD_ABORT      = 3U  /**< CMD12, CMD52 for writing "I/O Abort" in CCCR */
}cy_en_sd_host_cmd_type_t;

/** SD Host auto command enable selection. */
typedef enum
{
    CY_SD_HOST_AUTO_CMD_NONE  = 0U,  /**< Auto command disable. */ 
    CY_SD_HOST_AUTO_CMD_12    = 1U,  /**< Auto command 12 enable. */
    CY_SD_HOST_AUTO_CMD_23    = 2U,  /**< Auto command 23 enable. */
    CY_SD_HOST_AUTO_CMD_AUTO  = 3U   /**< Auto command Auto enable. */
}cy_en_sd_host_auto_cmd_t;

/** SD Host reset types. */
typedef enum
{
    CY_SD_HOST_RESET_DATALINE = 0U, /**< Reset the data circuit only. */
    CY_SD_HOST_RESET_CMD_LINE = 1U, /**< Reset the command circuit only. */  
    CY_SD_HOST_RESET_ALL      = 2U  /**< Reset the whole SD Host controller. */ 
}cy_en_sd_host_reset_t;

/** SD Host error interrupt types. */
typedef enum
{
    CY_SD_HOST_ADMA_ST_STOP   = 0U,   /**< Stop DMA - The SYS_ADR register points to
                                        * a location next to the error descriptor.  
                                        */
    CY_SD_HOST_ADMA_ST_FDS    = 1U,   /**< Fetch Descriptor - The SYS_ADR register
                                        * points to the error descriptor. 
                                        */
    CY_SD_HOST_ADMA_ST_TFR    = 3U,   /**< Transfer Data - SYS_ADR register points
                                        * to a location next to the error descriptor. 
                                        */
    CY_SD_HOST_ADMA_LEN_ERR   = 4U   /**< The ADMA Length Mismatch error. */
}cy_en_sd_host_adma_error_t;

/** Auto CMD Status error codes. */
typedef enum 
{
    CY_SD_HOST_AUTO_CMD12_NOT_EXEC         = 0U,  /**< Auto CMD12 Not Executed. */
    CY_SD_HOST_AUTO_CMD_TOUT_ERR           = 1U,  /**< Auto CMD Timeout Error. */
    CY_SD_HOST_AUTO_CMD_CRC_ERR            = 2U,  /**< Auto CMD CRC Error. */
    CY_SD_HOST_AUTO_CMD_EBIT_ERR           = 3U,  /**< Auto CMD End Bit Error. */
    CY_SD_HOST_AUTO_CMD_IDX_ERR            = 4U,  /**< Auto CMD Index Error. */
    CY_SD_HOST_AUTO_CMD_RESP_ERR           = 5U,  /**< Auto CMD Response Error. */
    CY_SD_HOST_CMD_NOT_ISSUED_AUTO_CMD12   = 7U   /**< Command Not Issued By Auto CMD12 Error. */ 
} cy_en_sd_host_auto_cmd_status_t;

/** SD host error codes. */
typedef enum 
{
    CY_SD_HOST_SUCCESS                     = 0U,  /**< Successful. */
    CY_SD_HOST_ERROR                       = 1U,  /**< Non-specific error code. */
    CY_SD_HOST_ERROR_INVALID_PARAMETER     = 2U,  /**< The provided parameter is not valid. */
    CY_SD_HOST_ERROR_OPERATION_IN_PROGRESS = 3U,  /**< A conflicting or requested operation is still in progress. */
    CY_SD_HOST_ERROR_UNINITIALIZED         = 4U,  /**< The module (or part of it) was not initialized properly. */
    CY_SD_HOST_ERROR_TIMEOUT               = 5U,  /**< A Time Out error occurred */
    CY_SD_HOST_OPERATION_INPROGRESS        = 6U,  /**< The indicator for operation in progress. */
    CY_SD_HOST_ERROR_UNUSABLE_CARD         = 7U,  /**< The card is unusable. */
    CY_SD_HOST_ERROR_DISCONNECTED          = 8U   /**< The card is disconnected. */
} cy_en_sd_host_status_t;

/** The widths of the data bus. */
typedef enum 
{
    CY_SD_HOST_BUS_WIDTH_1_BIT              = 0U,  /**< The 1-bit mode data transfer width. */
    CY_SD_HOST_BUS_WIDTH_4_BIT              = 1U,  /**< The 4-bit mode data transfer width. */
    CY_SD_HOST_BUS_WIDTH_8_BIT              = 2U  /**< The 8-bit mode data transfer width. */ 
} cy_en_sd_host_bus_width_t;

/** The bus speed modes. */
typedef enum 
{
    CY_SD_HOST_BUS_SPEED_DEFAULT            = 0U,  /**< Default Speed mode: 3.3V signaling at 25 MHz SDClk. */
    CY_SD_HOST_BUS_SPEED_HIGHSPEED          = 1U,  /**< High Speed mode: 3.3V signaling at 50 MHz SDClk. */
    CY_SD_HOST_BUS_SPEED_SDR12_5            = 2U,  /**< SDR12: UHS-I (1.8V signaling) at 25 MHz SDClk (12.5 MB/sec). */
    CY_SD_HOST_BUS_SPEED_SDR25              = 3U,  /**< SD25: UHS-I (1.8V signaling) at 50 MHz SDClk (25 MB/sec). */
    CY_SD_HOST_BUS_SPEED_SDR50              = 4U,  /**< SD50: UHS-I (1.8V signaling) at 100 MHz SDClk (50 MB/sec). */
    CY_SD_HOST_BUS_SPEED_EMMC_LEGACY        = 5U,  /**< Backwards Compatibility with legacy MMC card (26MB/sec max). */
    CY_SD_HOST_BUS_SPEED_EMMC_HIGHSPEED_SDR = 6U,  /**< eMMC High speed SDR (52MB/sec max) */
    CY_SD_HOST_BUS_SPEED_DDR50              = 7U   /**< DDR50: UHS-I (1.8V signaling) at 50 MHz SDClk (50 MB/sec). */
} cy_en_sd_host_bus_speed_mode_t;

/** The SD bus voltage select. */
typedef enum 
{
    CY_SD_HOST_IO_VOLT_3_3V                 = 0U,  /**< 3.3V.*/   
    CY_SD_HOST_IO_VOLT_1_8V                 = 1U   /**< 1.8V. */
} cy_en_sd_host_io_voltage_t;

/** Erase type. */
typedef enum 
{
    CY_SD_HOST_ERASE_ERASE               = 0U,  /**< The ERASE operation.*/   
    CY_SD_HOST_ERASE_DISCARD             = 1U,  /**< The DISCARD operation. */
    CY_SD_HOST_ERASE_FULE                = 2U,  /**< The Full User Area Logical Erase (FULE) operation. */
    CY_SD_HOST_ERASE_SECURE              = 3U,  /**< The secure purge according to 
                                                * Secure Removal Type in EXT_CSD 
                                                * on the erase groups identified by the 
                                                * startAddr&endAddr parameters and 
                                                * any copies of those erase groups. 
                                                */
    CY_SD_HOST_ERASE_SECURE_TRIM_STEP_2  = 4U,  /**< The secure purge operation on 
                                                * the write blocks according to 
                                                * Secure Removal Type in EXT_CSD 
                                                * and copies of those write blocks 
                                                * that were previously identified
                                                * using \ref Cy_SD_Host_Erase with 
                                                * CY_SD_HOST_ERASE_SECURE_TRIM_STEP_1
                                                */
    CY_SD_HOST_ERASE_SECURE_TRIM_STEP_1  = 5U,  /**< Mark the write blocks, indicated 
                                                * by the startAddr&endAddr parameters, 
                                                * for secure erase.
                                                */
    CY_SD_HOST_ERASE_TRIM                = 6U   /**< Trim the write blocks identified by 
                                                * the startAddr&endAddr parameters. The controller 
                                                * can perform the actual erase at convenient time.
                                                */                             
} cy_en_sd_host_erase_type_t;

/** Card type. */
typedef enum 
{
    CY_SD_HOST_SD               = 0U,  /**< The Secure Digital card (SD). */
    CY_SD_HOST_SDIO             = 1U,  /**< The CD Input Output card (SDIO). */
    CY_SD_HOST_EMMC             = 2U,  /**< The Embedded Multimedia card (eMMC). */
    CY_SD_HOST_COMBO            = 3U,  /**< The Combo card (SD + SDIO). */
    CY_SD_HOST_UNUSABLE         = 4U   /**< The unusable or not supported. */
} cy_en_sd_host_card_type_t;

/** The card capacity type. */
typedef enum
{
    CY_SD_HOST_SDSC             = 0U,   /**< SDSC - Secure Digital Standard Capacity (up to 2 GB). */
    CY_SD_HOST_SDHC             = 1U,   /**< SDHC - Secure Digital High Capacity (up to 32 GB). */
    CY_SD_HOST_EMMC_LESS_2G     = 0U,   /**< The eMMC block addressing for less than 2GB. */
    CY_SD_HOST_EMMC_GREATER_2G  = 1U,   /**< The eMMC block addressing for greater than 2GB. */
    CY_SD_HOST_SDXC             = 2U,   /**< SDXC - Secure Digital Extended Capacity (up to 2 TB). */
    CY_SD_HOST_UNSUPPORTED      = 4U    /**< Not supported. */
}cy_en_sd_host_card_capacity_t;

/** SDHC response types. */ 
typedef enum
{
    CY_SD_HOST_RESPONSE_NONE    = 0U, /**< No Response. */
    CY_SD_HOST_RESPONSE_LEN_136 = 1U, /**< Response Length 136. */
    CY_SD_HOST_RESPONSE_LEN_48  = 2U, /**< Response Length 48. */
    CY_SD_HOST_RESPONSE_LEN_48B = 3U  /**< Response Length 48. Check Busy after response. */
}cy_en_sd_host_response_type_t;

/** The DMA type enum. */
typedef enum
{
    CY_SD_HOST_DMA_SDMA         = 0U, /**< SDMA mode. */
    CY_SD_HOST_DMA_ADMA2        = 2U, /**< ADMA2 mode. */
    CY_SD_HOST_DMA_ADMA2_ADMA3  = 3U  /**< ADMA2-ADMA3 mode. */
}cy_en_sd_host_dma_type_t;

/** Write Protect type enum. */
typedef enum 
{
    CY_SD_HOST_PERMANENT         = 0U, /**< The permanent write protect. */
    CY_SD_HOST_ENABLE_TEMPORARY  = 1U, /**< The temporary write protect. */
    CY_SD_HOST_DISABLE_TEMPORARY = 2U  /**< Clear the temporary write protect. */
} cy_en_sd_host_write_protect_t;

/** \} group_sd_host_enums */

/**
* \addtogroup group_sd_host_data_structures
* \{
*/

/******************************************************************************
 * Structures
 *****************************************************************************/

/** SD Host initialization configuration structure. */
typedef struct
{
    bool                          emmc;             /**< Set to true of eMMC otherwise false. */
    cy_en_sd_host_dma_type_t      dmaType;          /**< Selects the DMA type to be used. */
    bool                          enableLedControl; /**< If true the SD clock controls one IO 
                                                     *   used to indicate when the card 
                                                     *    is being accessed. 
                                                     */
} cy_stc_sd_host_init_config_t;

/** SD/eMMC card configuration structure. */
typedef struct
{
    bool                          lowVoltageSignaling; /**< If true, the host supports the 1.8V signaling. */ 
    cy_en_sd_host_bus_width_t     busWidth;            /**< The desired bus width. */
    cy_en_sd_host_card_type_t     *cardType;           /**< The card type. */
    uint32_t                      *rca;                /**< The pointer to where to store the cards relative card address. */
    cy_en_sd_host_card_capacity_t *cardCapacity;       /**< Stores the card capacity. */
}cy_stc_sd_host_sd_card_config_t;

/** SD Host command configuration structure. */
typedef struct
{
    uint32_t                      commandIndex;      /**< The index of the command. */
    uint32_t                      commandArgument;   /**< The argument for the command. */
    bool                          enableCrcCheck;    /**< Enables the CRC check on the response. */
    bool                          enableAutoResponseErrorCheck; /**< If true the hardware checks the response for errors. */
    cy_en_sd_host_response_type_t respType;          /**< The response type. */
    bool                          enableIdxCheck;    /**< Checks the index of the response. */
    bool                          dataPresent;       /**< true: Data is present to 
                                                      * be transferred using the DAT line, 
                                                      * false: Commands use the CMD line only. 
                                                      */
    cy_en_sd_host_cmd_type_t      cmdType;           /**< The command type. */
} cy_stc_sd_host_cmd_config_t;

/** The SD Host data transfer configuration structure. */
typedef struct
{
    uint32_t                      blockSize;           /**< The size of the data block. */ 
    uint32_t                      numberOfBlock;       /**< The number of blocks to send. */ 
    bool                          enableDma;           /**< Enables DMA for the transaction. */ 
    cy_en_sd_host_auto_cmd_t      autoCommand;         /**< Selects which auto commands are used if any. */ 
    bool                          read;                /**< true = Read from the card, false = Write to the card. */ 
    uint32_t*                     data;                /**< The pointer to data to send/receive or 
                                                        *   the pointer to the DMA descriptor. 
                                                        */ 
    uint32_t                      dataTimeout;         /**< The timeout value for the transfer. */ 
    bool                          enableIntAtBlockGap; /**< Enables the interrupt generation at the block gap. */ 
    bool                          enReliableWrite;     /**< For EMMC enables the reliable write. */ 
}cy_stc_sd_host_data_config_t;

/** SD Host Write/Read structure. */
typedef struct
{
    uint32_t*                     data;            /**< The pointer to data. */
    uint32_t                      address;         /**< The address to Write/Read data on the card or eMMC. */
    uint32_t                      numberOfBlocks;  /**< The number of blocks to Write/Read. */
    cy_en_sd_host_auto_cmd_t      autoCommand;     /**< Selects which auto commands are used if any. */
    uint32_t                      dataTimeout;     /**< The timeout value for the transfer. */
    bool                          enReliableWrite; /**< For EMMC cards, enables the reliable write. */
    bool                          enableDma;       /**< Enables DMA for the transaction. */ 
} cy_stc_sd_host_write_read_config_t;

/** Context structure. */
typedef struct
{   
    cy_en_sd_host_dma_type_t      dmaType;      /**< Defines the DMA type to be used. */
    cy_en_sd_host_card_capacity_t cardCapacity; /**< The standard card or the card with the high capacity. */
    uint32_t                      maxSectorNum; /**< The SD card maximum number of the sectors. */  
    uint32_t                      RCA;          /**< The relative card address. */
    cy_en_sd_host_card_type_t     cardType;     /**< The card type. */
    uint32_t                      csd[4];       /**< The Card-Specific Data register. */
}cy_stc_sd_host_context_t;

/** \} group_sd_host_data_structures */


/**
* \addtogroup group_sd_host_high_level_functions
* \{
*/

/******************************************************************************
* Functions
*******************************************************************************/

/* High level section */

cy_en_sd_host_status_t Cy_SD_Host_InitCard(stc_SDHC_t *base, 
                                           cy_stc_sd_host_sd_card_config_t *config, 
                                           cy_stc_sd_host_context_t *context);
cy_en_sd_host_status_t Cy_SD_Host_Read(stc_SDHC_t *base, 
                                       cy_stc_sd_host_write_read_config_t *config,
                                       cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_Write(stc_SDHC_t *base,
                                        cy_stc_sd_host_write_read_config_t *config,
                                        cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_Erase(stc_SDHC_t *base, 
                                        uint32_t startAddr, 
                                        uint32_t endAddr, 
                                        cy_en_sd_host_erase_type_t eraseType, 
                                        cy_stc_sd_host_context_t const *context);
                                        
/** \} group_sd_host_high_level_functions */ 
                                        
/**
* \addtogroup group_sd_host_low_level_functions
* \{
*/

/** \cond INTERNAL_API */
                                                   

/** \endcond */

cy_en_sd_host_status_t Cy_SD_Host_Init(stc_SDHC_t *base, 
                                       const cy_stc_sd_host_init_config_t* config, 
                                       cy_stc_sd_host_context_t *context);
cy_en_sd_host_status_t Cy_SD_Host_DeInit(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_Enable(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_Disable(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_SdCardChangeClock(stc_SDHC_t *base, uint32_t frequency);
__STATIC_INLINE void Cy_SD_Host_EnableSdClk(stc_SDHC_t *base);
__STATIC_INLINE void Cy_SD_Host_DisableSdClk(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_SetSdClkDiv(stc_SDHC_t *base, uint16_t clkDiv);
__STATIC_INLINE bool Cy_SD_Host_IsWpSet(stc_SDHC_t const *base);
cy_en_sd_host_status_t Cy_SD_Host_SetHostBusWidth(stc_SDHC_t *base, 
                                                  cy_en_sd_host_bus_width_t width);
cy_en_sd_host_status_t Cy_SD_Host_SetBusWidth(stc_SDHC_t *base, 
                                              cy_en_sd_host_bus_width_t width,
                                              cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_SetHostSpeedMode(stc_SDHC_t *base, 
                                                  cy_en_sd_host_bus_speed_mode_t speedMode);
cy_en_sd_host_status_t Cy_SD_Host_SetBusSpeedMode(stc_SDHC_t *base, 
                                                  cy_en_sd_host_bus_speed_mode_t speedMode, 
                                                  cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_SelBusVoltage(stc_SDHC_t *base, 
                                                bool enable18VSignal, 
                                                cy_stc_sd_host_context_t *context);
__STATIC_INLINE void Cy_SD_Host_EnableCardVoltage(stc_SDHC_t *base);
__STATIC_INLINE void Cy_SD_Host_DisableCardVoltage(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_GetResponse(stc_SDHC_t const *base, 
                                              uint32_t *responsePtr, 
                                              bool largeResponse);
cy_en_sd_host_status_t Cy_SD_Host_SendCommand(stc_SDHC_t *base, 
                                              cy_stc_sd_host_cmd_config_t const *config);
cy_en_sd_host_status_t Cy_SD_Host_InitDataTransfer(stc_SDHC_t *base, 
                                                   cy_stc_sd_host_data_config_t const *dataConfig);
__STATIC_INLINE uint32_t Cy_SD_Host_BufferRead(stc_SDHC_t const *base);
__STATIC_INLINE cy_en_sd_host_status_t Cy_SD_Host_BufferWrite(stc_SDHC_t *base, uint32_t data);
__STATIC_INLINE void Cy_SD_Host_ChangeIoVoltage(stc_SDHC_t *base, cy_en_sd_host_io_voltage_t ioVoltage);
__STATIC_INLINE void Cy_SD_Host_StopAtBlockGap(stc_SDHC_t *base);
__STATIC_INLINE void Cy_SD_Host_ContinueFromBlockGap(stc_SDHC_t *base);
__STATIC_INLINE uint32_t Cy_SD_Host_GetAutoCmdErrStatus(stc_SDHC_t const *base);
__STATIC_INLINE cy_en_sd_host_status_t Cy_SD_Host_EnableAutoCmd23(stc_SDHC_t *base);
__STATIC_INLINE void Cy_SD_Host_DisableAutoCmd23(stc_SDHC_t *base);
__STATIC_INLINE cy_en_sd_host_status_t Cy_SD_Host_EnableAsyncInterrupt(stc_SDHC_t *base);
__STATIC_INLINE void Cy_SD_Host_DisableAsyncInterrupt(stc_SDHC_t *base);
__STATIC_INLINE uint32_t Cy_SD_Host_GetAdmaErrorStatus(stc_SDHC_t const *base);
__STATIC_INLINE void Cy_SD_Host_EMMC_Reset(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_AbortTransfer(stc_SDHC_t *base,
                                                cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_WriteProtect(stc_SDHC_t *base, 
                                               cy_en_sd_host_write_protect_t writeProtect,
                                               cy_stc_sd_host_context_t *context);
uint32_t Cy_SD_Host_GetCardStatus(stc_SDHC_t *base, cy_stc_sd_host_context_t const *context);
cy_en_sd_host_status_t Cy_SD_Host_GetSdStatus(stc_SDHC_t *base, 
                                              uint32_t *sdStatus,
                                              cy_stc_sd_host_context_t const *context);
uint32_t Cy_SD_Host_GetOcr(stc_SDHC_t *base,
                           cy_stc_sd_host_context_t *context);
cy_en_sd_host_status_t Cy_SD_Host_GetCid(stc_SDHC_t *base, uint32_t *cid);
cy_en_sd_host_status_t Cy_SD_Host_GetCsd(stc_SDHC_t *base, 
                                         uint32_t *csd, 
                                         cy_stc_sd_host_context_t *context);
cy_en_sd_host_status_t Cy_SD_Host_GetExtCsd(stc_SDHC_t *base, 
                                            uint32_t *extCsd, 
                                            cy_stc_sd_host_context_t *context);
uint32_t Cy_SD_Host_GetRca(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_SetRca (stc_SDHC_t *base, uint32_t *rca);
cy_en_sd_host_status_t Cy_SD_Host_GetScr(stc_SDHC_t *base, 
                                         uint32_t *scr, 
                                         cy_stc_sd_host_context_t const *context);
uint32_t Cy_SD_Host_GetPresentState(stc_SDHC_t const *base);
__STATIC_INLINE bool Cy_SD_Host_IsCardConnected(stc_SDHC_t const *base);
void Cy_SD_Host_SoftwareReset(stc_SDHC_t *base, cy_en_sd_host_reset_t reset);
cy_en_sd_host_status_t Cy_SD_Host_PollTransferComplete(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_PollCmdComplete(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_CmdRxData(stc_SDHC_t *base, 
                                   cy_stc_sd_host_data_config_t *pcmd);
cy_en_sd_host_status_t Cy_SD_Host_PollBufferWriteReady(stc_SDHC_t *base);
cy_en_sd_host_status_t Cy_SD_Host_PollBufferReadReady(stc_SDHC_t *base);
/** \} group_sd_host_low_level_functions */
cy_en_sd_host_status_t Cy_SD_Host_OpsSetBlockCount(stc_SDHC_t *base, 
                                                                   bool reliableWrite,
                                                                   uint32_t blockNum);
/**
* \addtogroup group_sd_host_interrupt_functions
* \{
*/

__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptStatus(stc_SDHC_t const *base);
__STATIC_INLINE void Cy_SD_Host_ClearNormalInterruptStatus(stc_SDHC_t *base, uint32_t status);
__STATIC_INLINE void Cy_SD_Host_SetNormalInterruptEnable(stc_SDHC_t *base, uint32_t interrupt);
__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptEnable(stc_SDHC_t const *base);
__STATIC_INLINE void Cy_SD_Host_SetNormalInterruptMask(stc_SDHC_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptMask(stc_SDHC_t const *base);
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptStatus(stc_SDHC_t const *base);
__STATIC_INLINE void Cy_SD_Host_ClearErrorInterruptStatus(stc_SDHC_t *base, uint32_t status);
__STATIC_INLINE void Cy_SD_Host_SetErrorInterruptEnable(stc_SDHC_t *base, uint32_t interrupt);
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptEnable(stc_SDHC_t const *base);
__STATIC_INLINE void Cy_SD_Host_SetErrorInterruptMask(stc_SDHC_t *base, uint32_t interruptMask);
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptMask(stc_SDHC_t const *base);
cy_en_sd_host_status_t Cy_SD_Host_PollCmdLineFree(stc_SDHC_t const *base);
cy_en_sd_host_status_t Cy_SD_Host_PollDataLineNotInhibit(stc_SDHC_t const *base);
cy_en_sd_host_status_t Cy_SD_Host_PollDataLineFree(stc_SDHC_t const *base);
/** \} group_sd_host_interrupt_functions */        

/**
* \addtogroup group_sd_host_low_level_functions
* \{
*/


/*******************************************************************************
* Function Name: Cy_SD_Host_EnableSdClk
****************************************************************************//**
*
*  Enables the SD clock (SD host drives the SDCLK line).
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_EnableSdClk(stc_SDHC_t *base)
{
    un_SDHC_CORE_CLK_CTRL_R_t temp;
    
    /* Check for NULL pointer */
    if (NULL != base) 
    {       
        temp.u16Register = _CLR_SET_FLD16U(base->CORE.unCLK_CTRL_R.u16Register, 
                                        SDHC_CORE_CLK_CTRL_R_SD_CLK_EN, 1u) | 
                           _CLR_SET_FLD16U(base->CORE.unCLK_CTRL_R.u16Register, 
                                        SDHC_CORE_CLK_CTRL_R_PLL_ENABLE, 1u);
        
        base->CORE.unCLK_CTRL_R.u16Register = temp.u16Register;
    }
}


/*******************************************************************************
* Function Name: Cy_SD_Host_DisableSdClk
****************************************************************************//**
*
*  Disables the SD clock (The SD Host doesn't drive the SDCLK line).
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_DisableSdClk(stc_SDHC_t *base)
{
    /* Check for NULL pointer */
    if (NULL != base) 
    {
        base->CORE.unCLK_CTRL_R.u16Register = _CLR_SET_FLD16U(base->CORE.unCLK_CTRL_R.u16Register, 
                                        SDHC_CORE_CLK_CTRL_R_PLL_ENABLE, 0UL);
                                        
        /* Wait for at least 3 card clock periods */
        Cy_SysLib_DelayUs(CY_SD_HOST_3_PERIODS_US);
               
        base->CORE.unCLK_CTRL_R.u16Register = _CLR_SET_FLD16U(base->CORE.unCLK_CTRL_R.u16Register, 
                                        SDHC_CORE_CLK_CTRL_R_SD_CLK_EN, 0UL);
    }
}


/*******************************************************************************
* Function Name: Cy_SD_Host_IsWpSet
****************************************************************************//**
*
*  Returns the state of the write protect switch on the SD card.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return bool
*     true - the write protect is set, false - the write protect is not set.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SD_Host_IsWpSet(stc_SDHC_t const *base)
{
    return (bool)_FLD2BOOL(SDHC_CORE_PSTATE_REG_WR_PROTECT_SW_LVL, base->CORE.unPSTATE_REG.u32Register);
}



/*******************************************************************************
* Function Name: Cy_SD_Host_EnableCardVoltage
****************************************************************************//**
*
*  Sets the card_if_pwr_en pin high. 
*  This pin can be used to enable a voltage regulator used to power the card.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_EnableCardVoltage(stc_SDHC_t *base)
{  
    base->CORE.unPWR_CTRL_R.u8Register = _CLR_SET_FLD8U(base->CORE.unPWR_CTRL_R.u8Register, SDHC_CORE_PWR_CTRL_R_SD_BUS_PWR_VDD1, 1UL); 
} 


/*******************************************************************************
* Function Name: Cy_SD_Host_DisableCardVoltage
****************************************************************************//**
*
*  Sets the card_if_pwr_en pin low. 
*  This pin can be used to disable a voltage regulator used to power the card.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_DisableCardVoltage(stc_SDHC_t *base)
{
   
    base->CORE.unPWR_CTRL_R.u8Register = _CLR_SET_FLD8U(base->CORE.unPWR_CTRL_R.u8Register, SDHC_CORE_PWR_CTRL_R_SD_BUS_PWR_VDD1, 0UL); 
}


/*******************************************************************************
* Function Name: Cy_SD_Host_BufferRead
****************************************************************************//**
*
*  Reads 32-bits of data from the read buffer. 
*  Only use this function if not using SD block DMA to transfer data from buffer.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     Data that is being read.
*
*******************************************************************************/
__STATIC_INLINE uint32_t  Cy_SD_Host_BufferRead(stc_SDHC_t const *base)
{
    /* Return the Buffer Data Port Register value */ 
    return base->CORE.unBUF_DATA_R.u32Register;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_BufferWrite
****************************************************************************//**
*
*  Writes 32-bits of data into the write buffer.
*  Only use this function if not using SD block DMA to transfer data to buffer.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param data
*     Data to be written.
*
* \return \ref cy_en_sd_host_status_t
*
*******************************************************************************/
__STATIC_INLINE cy_en_sd_host_status_t  Cy_SD_Host_BufferWrite(stc_SDHC_t *base, uint32_t data)
{
    cy_en_sd_host_status_t ret = CY_SD_HOST_ERROR_INVALID_PARAMETER;

    /* Check for the NULL pointer */ 
    if (NULL != base)
    {
        base->CORE.unBUF_DATA_R.u32Register = data; 
        
        ret = CY_SD_HOST_SUCCESS;
    }

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_ChangeIoVoltage
****************************************************************************//**
*
*  Changes the logic level on the sd_io_volt_sel line. It assumes that 
*  this line is used to control a regulator connected to the VIDDO of the PSoC. 
*  This regulator allows for switching between the 3.3V and 1.8V signaling.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param ioVoltage
*     The voltage for IO.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_ChangeIoVoltage(stc_SDHC_t *base, cy_en_sd_host_io_voltage_t ioVoltage)
{
    /* Set the 1.8V signaling enable. */
    base->CORE.unHOST_CTRL2_R.u16Register = _CLR_SET_FLD16U(base->CORE.unHOST_CTRL2_R.u16Register, 
                                              SDHC_CORE_HOST_CTRL2_R_SIGNALING_EN, 
                                              (CY_SD_HOST_IO_VOLT_1_8V == ioVoltage) ? 1UL : 0UL);
}


/*******************************************************************************
* Function Name: Cy_SD_Host_StopAtBlockGap
****************************************************************************//**
*
*  Commands SD host to stop transmitting at the next block gap.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_StopAtBlockGap(stc_SDHC_t *base)
{   
    base->CORE.unBGAP_CTRL_R.u8Register = _CLR_SET_FLD8U(base->CORE.unBGAP_CTRL_R.u8Register, 
                                            SDHC_CORE_BGAP_CTRL_R_STOP_BG_REQ, 
                                            1UL);
}


/*******************************************************************************
* Function Name: Cy_SD_Host_ContinueFromBlockGap
****************************************************************************//**
*
*  Commands SD host to continue transmitting after stopping at the block gap.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_ContinueFromBlockGap(stc_SDHC_t *base)
{    
    base->CORE.unBGAP_CTRL_R.u8Register = (uint8_t)(((uint32_t)base->CORE.unBGAP_CTRL_R.u8Register &
        ~SDHC_CORE_BGAP_CTRL_R_STOP_BG_REQ_Msk) |  /* Clear the Stop At Block Gap Request bit. */
        SDHC_CORE_BGAP_CTRL_R_CONTINUE_REQ_Msk);   /* Set the Continue Request bit. */
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetAutoCmdErrStatus
****************************************************************************//**
*
*  Gets the SD host error status of the auto command.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The bit mask of the Auto Command status.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetAutoCmdErrStatus(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unAUTO_CMD_STAT_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_EnableAutoCmd23
****************************************************************************//**
*
*  If the card supports AutoCmd23 call this function to enable it in the host.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return \ref cy_en_sd_host_status_t.
*
*******************************************************************************/
__STATIC_INLINE cy_en_sd_host_status_t  Cy_SD_Host_EnableAutoCmd23(stc_SDHC_t *base)
{
    cy_en_sd_host_status_t ret = CY_SD_HOST_ERROR_INVALID_PARAMETER;

    /* Check for the NULL pointer */
    if (NULL != base) 
    {  
        base->CORE.unXFER_MODE_R.u16Register = _CLR_SET_FLD16U(base->CORE.unXFER_MODE_R.u16Register, 
                                      SDHC_CORE_XFER_MODE_R_AUTO_CMD_ENABLE, 
                                      2UL);

        ret = CY_SD_HOST_SUCCESS;
    }

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_DisableAutoCmd23
****************************************************************************//**
*
* Removes support for AutoCmd23 if it was previously set.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_DisableAutoCmd23(stc_SDHC_t *base)
{ 
    base->CORE.unXFER_MODE_R.u16Register = _CLR_SET_FLD16U(base->CORE.unXFER_MODE_R.u16Register, 
                                  SDHC_CORE_XFER_MODE_R_AUTO_CMD_ENABLE, 
                                  0UL);
}


/*******************************************************************************
* Function Name: Cy_SD_Host_EnableAsyncInterrupt
****************************************************************************//**
*
*  Enables the Asynchronous Interrupt for SDIO cards. Set this only
*  if the card supports this feature.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return \ref cy_en_sd_host_status_t
*
*******************************************************************************/
__STATIC_INLINE cy_en_sd_host_status_t  Cy_SD_Host_EnableAsyncInterrupt(stc_SDHC_t *base)
{
    cy_en_sd_host_status_t ret = CY_SD_HOST_ERROR_INVALID_PARAMETER;

    /* Check for the NULL pointer */
    if (NULL != base) 
    {
        base->CORE.unHOST_CTRL2_R.u16Register = _CLR_SET_FLD16U(base->CORE.unHOST_CTRL2_R.u16Register, 
                              SDHC_CORE_HOST_CTRL2_R_ASYNC_INT_ENABLE, 
                              1UL);
    }

    return ret;
}



/*******************************************************************************
* Function Name: Cy_SD_Host_DisableAsyncInterrupt
****************************************************************************//**
*
* Removes the support for the Asynchronous Interrupt if it was previously set.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_DisableAsyncInterrupt(stc_SDHC_t *base)
{ 
    base->CORE.unHOST_CTRL2_R.u16Register = _CLR_SET_FLD16U(base->CORE.unHOST_CTRL2_R.u16Register, 
                                              SDHC_CORE_HOST_CTRL2_R_ASYNC_INT_ENABLE, 
                                              0UL);
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetAdmaErrorStatus
****************************************************************************//**
*
*  Returns the ADMA Error Status register.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The bit mask of ADMA Error Status.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetAdmaErrorStatus(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unADMA_ERR_STAT_R.u8Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_EMMC_Reset
****************************************************************************//**
*
* Resets the eMMC card.
*
* \param *base
*     The SD host registers structure pointer.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_EMMC_Reset(stc_SDHC_t *base)
{ 
        base->CORE.unEMMC_CTRL_R.u16Register = _CLR_SET_FLD16U(base->CORE.unEMMC_CTRL_R.u16Register, 
                                                 SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N, 
                                                 0UL);
                                                 
        /* Wait RST_n pulse width of 1 us */
        Cy_SysLib_DelayUs(CY_SD_HOST_EMMC_T_RSTW_US);

        base->CORE.unEMMC_CTRL_R.u16Register = _CLR_SET_FLD16U(base->CORE.unEMMC_CTRL_R.u16Register, 
                                                 SDHC_CORE_EMMC_CTRL_R_EMMC_RST_N, 
                                                 1UL);
                                                 
        /* Wait RST_n to Command time (74 cycles of clock signal required
         * before issuing CMD1 or CMD0 with argument 0xFFFFFFFA) - 200 us 
         */
        Cy_SysLib_DelayUs(CY_SD_HOST_EMMC_T_RSCA_US);                                      
}


/*******************************************************************************
* Function Name: Cy_SD_Host_IsCardConnected
****************************************************************************//**
*
*  Checks to see if a card is currently connected.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return bool
*     true - the card is connected, false - the card is removed (not connected).
*
*******************************************************************************/
__STATIC_INLINE bool Cy_SD_Host_IsCardConnected(stc_SDHC_t const *base)
{
    /* Wait until the card is stable. */
    while(true != _FLD2BOOL(SDHC_CORE_PSTATE_REG_CARD_STABLE, base->CORE.unPSTATE_REG.u32Register));

    return (bool)_FLD2BOOL(SDHC_CORE_PSTATE_REG_CARD_INSERTED, base->CORE.unPSTATE_REG.u32Register);
}

/** \} group_sd_host_low_level_functions */

/**
* \addtogroup group_sd_host_interrupt_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_SD_Host_GetNormalInterruptStatus
****************************************************************************//**
*
*  Returns the normal Int Status register.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The Bit mask of the normal Int Status.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptStatus(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unNORMAL_INT_STAT_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_ClearNormalInterruptStatus
****************************************************************************//**
*
*  Clears the selected SD host normal status.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param status
*     The bitmask of statuses to clear.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_ClearNormalInterruptStatus(stc_SDHC_t *base, uint32_t status)
{ 
    base->CORE.unNORMAL_INT_STAT_R.u16Register = (uint16_t)status;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_SetNormalInterruptEnable
****************************************************************************//**
*
*  Sets the bit to be active in the Int status register.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param interrupt
*     The Bit field of which normal interrupt status to enable.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_SetNormalInterruptEnable(stc_SDHC_t *base, uint32_t interrupt)
{ 
    base->CORE.unNORMAL_INT_STAT_EN_R.u16Register = (uint16_t)interrupt;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetNormalInterruptEnable
****************************************************************************//**
*
*  Returns which normal interrupts are enabled.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The Bit field of which normal interrupt statuses are enabled.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptEnable(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unNORMAL_INT_STAT_EN_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_SetNormalInterruptMask
****************************************************************************//**
*
*  Setting a bit in this register allows the enabled status to cause an interrupt.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param interruptMask
*     The Bit field of which normal interrupts can cause an interrupt.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_SetNormalInterruptMask(stc_SDHC_t *base, uint32_t interruptMask)
{ 
    base->CORE.unNORMAL_INT_SIGNAL_EN_R.u16Register = (uint16_t)interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetNormalInterruptMask
****************************************************************************//**
*
*  Returns which normal interrupts are masked to cause an interrupt.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The Bit field of which normal interrupts are masked to cause an interrupt.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetNormalInterruptMask(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unNORMAL_INT_SIGNAL_EN_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetErrorInterruptStatus
****************************************************************************//**
*
*  Returns the error Int Status register.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The bit mask of the error Int status.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptStatus(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unERROR_INT_STAT_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_ClearErrorInterruptStatus
****************************************************************************//**
*
*  Clears the error interrupt status.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param status
*     The bitmask of statuses to clear.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_ClearErrorInterruptStatus(stc_SDHC_t *base, uint32_t status)
{
    base->CORE.unERROR_INT_STAT_R.u16Register = (uint16_t)status;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_SetErrorInterruptEnable
****************************************************************************//**
*
*  Setting a bit in this register allows for the bit to be active in 
*  the Int status register.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param interrupt
*     The Bit field of which error interrupt status to enable.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_SetErrorInterruptEnable(stc_SDHC_t *base, uint32_t interrupt)
{ 
    base->CORE.unERROR_INT_STAT_EN_R.u16Register = (uint16_t)interrupt;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetErrorInterruptEnable
****************************************************************************//**
*
*  Returns which error interrupts are enabled.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The Bit field of which error interrupt statuses are enabled.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptEnable(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unERROR_INT_STAT_EN_R.u16Register;

    return ret;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_SetErrorInterruptMask
****************************************************************************//**
*
* Setting a bit in this register allows the enabled status to cause an interrupt.
*
* \param *base
*     The SD host registers structure pointer.
*
* \param interruptMask
*     The Bit filed of which error interrupts can cause an interrupt.
*
*******************************************************************************/
__STATIC_INLINE void Cy_SD_Host_SetErrorInterruptMask(stc_SDHC_t *base, uint32_t interruptMask)
{ 
    base->CORE.unERROR_INT_SIGNAL_EN_R.u16Register = (uint16_t)interruptMask;
}


/*******************************************************************************
* Function Name: Cy_SD_Host_GetErrorInterruptMask
****************************************************************************//**
*
*  Returns which error interrupts are masked to cause an interrupt.
*
* \param *base
*     The SD host registers structure pointer.
*
* \return uint32_t
*     The Bit field of which error interrupts are masked to cause an interrupt.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_SD_Host_GetErrorInterruptMask(stc_SDHC_t const *base)
{
    uint32_t ret;

    ret = (uint32_t)base->CORE.unERROR_INT_SIGNAL_EN_R.u16Register;

    return ret;
}          

/** \} group_sd_host_interrupt_functions */

#ifdef __cplusplus
}
#endif

/** \} group_sd_host */

#endif /* defined(CY_IP_MXSDHC) */

#endif /* CY_SD_HOST_PDL_H */


/* [] END OF FILE */
