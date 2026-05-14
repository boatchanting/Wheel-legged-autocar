/***************************************************************************//**
* \file cy_smif.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Provides an API declaration of the Cypress SMIF driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_smif Serial Memory Interface (SMIF)
* \{
* The SPI-based communication interface for external memory devices.
*
* SMIF: Serial Memory Interface: This IP block implements an SPI-based
* communication interface for interfacing external memory devices to TVII. The SMIF
* supports Octal-SPI, Dual Quad-SPI, Quad-SPI, DSPI, and SPI.
*
* Features
*   - Standard SPI Master interface
*   - Supports Single/Dual/Quad/Octal SPI Memories
*   - Supports Dual-Quad SPI mode
*   - Design-time configurable support for multiple (up to 4) external serial
*   memory devices
*   - eXecute-In-Place (XIP) operation mode for both read and write accesses
*   with 4KB XIP read cache and on-the-fly encryption and decryption
*   - Supports external serial memory initialization via Serial Flash
*   Discoverable Parameters (SFDP) standard
*   - Support for SPI clock frequencies up to 80 MHz
*
* The primary usage model for the SMIF is that of an external memory interface.
* The SMIF is capable of interfacing with different types of memory, up to four
* types.
*
* \b SMIF driver is divided in next layers
*   - cy_smif.h APIs
*   - cy_smif_memslot.h APIs
*   - SMIF version specific support APIs
*
* The SMIF API is divided into the low-level functions and memory-slot functions. Use
* the low level API for the SMIF block initialization and for implementing a generic 
* SPI communication interface using the SMIF block.
*
* The memory slot API has functions to implement the basic memory operations such as 
* program, read, erase etc. These functions are implemented using the memory 
* parameters in the memory device configuration data structure. The memory-slot
* initialization API initializes all the memory slots based on the settings in the
* array.
*
* \warning The driver is not responsible for external memory persistence. You cannot edit
* a buffer during the Read/Write operations. If there is a memory error, the SMIF ip block 
* can require a reset. To determine if this has happened, check the SMIF 
* busy status using Cy_SMIF_IsBusy() and implement a timeout. Reset the SMIF 
* block by toggling CTL.ENABLED. Then reconfigure the SMIF block.
*
* For the Write operation, check that the SMIF driver has completed 
* transferring by calling Cy_SMIF_IsBusy(). Also, check that the memory is 
* available with Cy_SMIF_Memslot_IsBusy() before proceeding. 
*
* Simple example of external flash memory programming using low level SMIF API.
* All steps mentioned in example below are incorporated in
* \ref Cy_SMIF_Memslot_CmdWriteEnable(), \ref Cy_SMIF_Memslot_CmdProgram(), and
* \ref Cy_SMIF_Memslot_IsBusy() of the
* \ref group_smif_mem_slot_functions "memory slot level API".
* \warning Example is simplified, without checks of error conditions.
* \note Flash memories need erase operation before programming. Refer to
* external memory datasheet for specific memory commands.
*
* For the Read operation, before accessing the read buffer, check that it is ready
* by calling Cy_SMIF_GetTxFifoStatus().
*
* Simple example of external flash memory read using low level SMIF API. All
* steps mentioned in example below are incorporated in
* \ref Cy_SMIF_Memslot_CmdRead() of the
* \ref group_smif_mem_slot_functions "memory slot level API".
* \warning Example is simplified, without checks of error conditions.
* \note Refer to external memory datasheet for specific memory commands.
*
* The user should invalidate the cache by calling Cy_SMIF_CacheInvalidate() when 
* switching from the MMIO mode to XIP mode.
*
* \section group_smif_configuration Configuration Considerations
*
* See the documentation for Cy_SMIF_Init() and Cy_SMIF_Memslot_Init() for details
* on the required configuration structures and other initialization topics. 
*
* The normal (MMIO) mode is used for implementing a generic SPI/DSPI/QSPI/Dual
* Quad-SPI/Octal-SPI communication interface using the SMIF block. This
* interface can be used to implement special commands like Program/Erase of
* flash, memory device configuration, sleep mode entry for memory devices or
* other special commands specific to the memory device. The transfer width
* (SPI/DSP/Quad-SPI/Octal-SPI) of a transmission is a parameter set for each
* transmit/receive operation. So these can be changed at run time.
*
* In a typical memory interface with flash memory, the SMIF is used in the
* memory mode when reading from the memory and it switches to the normal mode when
* writing to flash memory.
* A typical memory device has multiple types of commands.
*
* The SMIF interface can be used to transmit different types of commands. Each
* command has different phases: command, dummy cycles, and transmit and receive
* data which require separate APIs.
*
* \subsection group_smif_init SMIF Initialization
* Create interrupt function and allocate memory for SMIF context
* structure
*
* SMIF driver initialization for low level API usage (cy_smif.h)
* Additional steps to initialize SMIF driver for memory slot level API usage
* (cy_smif_memslot.h).
*
* \note Example does not include initialization of all needed configuration
* structures (\ref cy_stc_smif_mem_device_cfg_t, \ref cy_stc_smif_mem_cmd_t).
*
* \subsection group_smif_xip_init SMIF XIP Initialization
* The eXecute In Place (XIP) is a mode of operation where read or write commands 
* to the memory device are directed through the SMIF without any use of API 
* function calls. In this mode the SMIF block maps the AHB bus-accesses to 
* external memory device addresses to make it behave similar to internal memory. 
* This allows the CPU to execute code directly from external memory. This mode 
* is not limited to code and is suitable also for data read and write accesses. 
*
* \note Example of input parameters initialization is in \ref group_smif_init 
* section.
* \warning Functions that called from external memory should be declared with 
* long call attribute.     
*
* \section group_smif_more_information More Information
*
* More information regarding the Serial Memory Interface can be found in the Technical 
* Reference Manual (TRM).
*
* \section group_smif_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
* 
* \defgroup group_smif_macros Macros
* \{
* \defgroup group_smif_macros_status Status Macros
* \defgroup group_smif_macros_cmd Command Macros
* \defgroup group_smif_macros_flags External Memory Flags
* \}
* \defgroup group_smif_functions Functions
* \{
* \defgroup group_smif_low_level_functions Low Level Functions
* \{
* Basic flow for read/write commands using \ref Cy_SMIF_TransmitCommand
* \ref Cy_SMIF_TransmitData \ref Cy_SMIF_ReceiveData
* \ref Cy_SMIF_SendDummyCycles
*
* \}
* \defgroup group_smif_mem_slot_functions Memory Slot Functions
* \defgroup group_smif_version_specific_functions SMIF version specific Functions
* \}
* \defgroup group_smif_data_structures Data Structures
* \{
* \defgroup group_smif_data_structures_memslot SMIF Memory Description Structures
* General hierarchy of memory structures are:
* Top structure is \ref cy_stc_smif_block_config_t, which could have links up to
* 4 \ref cy_stc_smif_mem_config_t which describes each connected to the SMIF
* external memory.
* \}
* \defgroup group_smif_enums Enumerated Types
*/

#if !defined(CY_SMIF_H)
#define CY_SMIF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"
#include "cy_device_headers.h"

#ifdef CY_IP_MXSMIF

#if defined(__cplusplus)
extern "C" {
#endif
  
/***************************************
*            Constants
****************************************/

/**
* \addtogroup group_smif_macros
* \{
*/

/** The driver major version */
#define CY_SMIF_DRV_VERSION_MAJOR               `$CY_MAJOR_VERSION`
  
/** The driver minor version */
#define CY_SMIF_DRV_VERSION_MINOR               `$CY_MINOR_VERSION`
  
/** One microsecond timeout for Cy_SMIF_TimeoutRun() */
#define CY_SMIF_WAIT_1_UNIT                     (1U)
  
/** The SMIF driver ID, reported as part of an unsuccessful API return status
* \ref cy_en_smif_status_t */
#define CY_SMIF_ID CY_PDL_DRV_ID                (0x2CU)

/** Number of SMIF core  */
#define CY_SMIF_CORE_MAX                        (2U)


/**
* \addtogroup group_smif_macros_isr
* \{
*/
/** \} group_smif_macros_isr */

/** \cond INTERNAL */

/* Devices with a single SMIF instance use a different generated define name (w/o instance suffix), so make it common */
// For device count definitions
#if defined(SMIF0_DEVICE_NR)
  #define CY_SMIF_DRV_SMIF0_DEVICE_NR                 SMIF0_DEVICE_NR
#elif defined(SMIF_DEVICE_NR)
  #define CY_SMIF_DRV_SMIF0_DEVICE_NR                 SMIF_DEVICE_NR
#endif

#if defined(CY_SMIF_CORE1_EXISTS)
  #if defined(SMIF1_DEVICE_NR)
    #define CY_SMIF_DRV_SMIF1_DEVICE_NR               SMIF1_DEVICE_NR
  #elif defined(SMIF_DEVICE_NR)
    #define CY_SMIF_DRV_SMIF1_DEVICE_NR               SMIF_DEVICE_NR
  #endif
#endif

// For delay tap number definitions
#if defined(SMIF_DELAY_TAPS_NR)
  #define CY_SMIF_DRV_SMIF0_DELAY_TAPS_NR             SMIF_DELAY_TAPS_NR
#elif defined(SMIF0_DELAY_TAPS_NR)
  #define CY_SMIF_DRV_SMIF0_DELAY_TAPS_NR             SMIF0_DELAY_TAPS_NR
#endif

#if defined(SMIF1_DELAY_TAPS_NR)
  #define CY_SMIF_DRV_SMIF1_DELAY_TAPS_NR             SMIF1_DELAY_TAPS_NR
#endif

// For delay line number definitions
#if defined(SMIF_DELAY_LINES_NR)
  #define CY_SMIF_DRV_SMIF0_DELAY_LINES_NR            SMIF_DELAY_LINES_NR
#elif defined(SMIF0_DELAY_LINES_NR)
  #define CY_SMIF_DRV_SMIF0_DELAY_LINES_NR            SMIF0_DELAY_LINES_NR
#endif

#if defined(SMIF1_DELAY_LINES_NR)
  #define CY_SMIF_DRV_SMIF1_DELAY_LINES_NR            SMIF1_DELAY_LINES_NR
#endif

// For XIP MASK definitions
#if defined(SMIF_SMIF_XIP_MASK)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_MASK             SMIF_SMIF_XIP_MASK
#elif defined(SMIF0_SMIF_XIP_MASK)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_MASK             SMIF0_SMIF_XIP_MASK
#elif defined(SMIF_SMIF0_XIP_MASK)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_MASK             SMIF_SMIF0_XIP_MASK
#endif

#if defined(SMIF1_SMIF_XIP_MASK)
  #define CY_SMIF_DRV_SMIF1_SMIF_XIP_MASK             SMIF1_SMIF_XIP_MASK
#elif defined(SMIF_SMIF1_XIP_MASK)
  #define CY_SMIF_DRV_SMIF1_SMIF_XIP_MASK             SMIF_SMIF1_XIP_MASK
#endif

// For XIP ADDR definitions
#if defined(SMIF_SMIF_XIP_ADDR)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_ADDR             SMIF_SMIF_XIP_ADDR
#elif defined(SMIF0_SMIF_XIP_ADDR)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_ADDR             SMIF0_SMIF_XIP_ADDR
#elif defined(SMIF_SMIF0_XIP_ADDR)
  #define CY_SMIF_DRV_SMIF0_SMIF_XIP_ADDR             SMIF_SMIF0_XIP_ADDR
#endif

#if defined(SMIF1_SMIF_XIP_ADDR)
  #define CY_SMIF_DRV_SMIF1_SMIF_XIP_ADDR             SMIF1_SMIF_XIP_ADDR
#elif defined(SMIF_SMIF1_XIP_ADDR)
  #define CY_SMIF_DRV_SMIF1_SMIF_XIP_ADDR             SMIF_SMIF1_XIP_ADDR
#endif

#define CY_SMIF_CMD_FIFO_TX_MODE                (0UL)
#define CY_SMIF_CMD_FIFO_TX_COUNT_MODE          (1UL)
#define CY_SMIF_CMD_FIFO_RX_COUNT_MODE          (2UL)
#define CY_SMIF_CMD_FIFO_DUMMY_COUNT_MODE       (3UL)
#define CY_SMIF_CMD_FIFO_DESELECT_MODE          (4UL)
  
#define CY_SMIF_TX_CMD_FIFO_STATUS_RANGE        (8U)
#define CY_SMIF_TX_DATA_FIFO_STATUS_RANGE       (8U)
#define CY_SMIF_RX_DATA_FIFO_STATUS_RANGE       (8U)
#define CY_SMIF_MAX_TX_TR_LEVEL                 (8U)
#define CY_SMIF_MAX_RX_TR_LEVEL                 (8U)
  
#define CY_SMIF_CRYPTO_START                    (1UL)
#define CY_SMIF_CRYPTO_COMPLETED                (0UL)
#define CY_SMIF_CRYPTO_ADDR_MASK                (0xFFFFFFF0UL)
#define CY_SMIF_AES128_BYTES                    (16U)
  
#define CY_SMIF_CTL_REG_DEFAULT                 (0x00000300U) /* 3 - [13:12] CLOCK_IF_RX_SEL  */
#define CY_SMIF_CTL2_REG_DEFAULT                (0x00000000U)
  
  
/** \endcond*/

/** \} group_smif_macros */


/**
* \addtogroup group_smif_enums
* \{
*/

/** The Transfer width options for the command, data, the address and the mode. */
typedef enum
{
    CY_SMIF_WIDTH_SINGLE   = 0U,    /**< Normal SPI mode. */
    CY_SMIF_WIDTH_DUAL     = 1U,    /**< Dual SPI mode. */
    CY_SMIF_WIDTH_QUAD     = 2U,    /**< Quad SPI mode. */
    CY_SMIF_WIDTH_OCTAL    = 3U     /**< Octal SPI mode. */
} cy_en_smif_txfr_width_t;


/* Specifies what happens for MMIO interface read accesses to an */
/* empty RX data FIFO or to a full TX format/data FIFO.          */
typedef enum
{
    /**< Generates a bus error. */
    CY_SMIF_BUS_ERROR      = 0UL,
    /** Stalls the bus with the wait states. This option will increase the 
    * interrupt latency. 
    */
    CY_SMIF_WAIT_STATES    = 1UL
} cy_en_smif_error_event_t;

/** The data line-selection options for a slave device. */
typedef enum
{
    /**
    * smif.spi_data[0] = DATA0, smif.spi_data[1] = DATA1, ..., smif.spi_data[7] = DATA7.
    * This value is allowed for the SPI, DSPI, quad-SPI, dual quad-SPI, and octal-SPI modes.
    */
    CY_SMIF_DATA_SEL0      = 0,
    /**
    * smif.spi_data[2] = DATA0, smif.spi_data[3] = DATA1.
    * This value is only allowed for the SPI and DSPI modes.
    */
    CY_SMIF_DATA_SEL1      = 1,
    /**
    * smif.spi_data[4] = DATA0, smif.spi_data[5] = DATA1, ..., smif.spi_data[7] = DATA3.
    * This value is only allowed for the SPI, DSPI, quad-SPI and dual quad-SPI modes.
    */
    CY_SMIF_DATA_SEL2      = 2,
    /**
    * smif.spi_data[6] = DATA0, smif.spi_data[7] = DATA1.
    * This value is only allowed for the SPI and DSPI modes.
    */
    CY_SMIF_DATA_SEL3      = 3
} cy_en_smif_data_select_t;

/** The SMIF modes to work with an external memory. */
typedef enum
{
    CY_SMIF_NORMAL = 0,         /**< Command mode (MMIO mode). */
    CY_SMIF_MEMORY = 1          /**< XIP (eXecute In Place) mode. or ARB mode for smif_3*/
} cy_en_smif_mode_t;


/** Device interface transmitter clock source */
typedef enum
{
    CY_SMIF_DIV_INV_FOR_SDR = 0, /**< transmitter clock clk_if */
    CY_SMIF_INV_FOR_DDR     = 1  /**< transmitter clock clk_if_inv */
} cy_en_smif_clk_tx_t;


/** Specifies the delay line used for RX data capturing with  */
typedef enum
{
    CY_SMIF_1_NEW_SEL_PER_TAP  = 0, /**< 1 of these new delay cells per tap providing: 
    granularity  of  max.  ~0.22ns */
    CY_SMIF_1_SEN_SEL_PER_TAP  = 1, /**< 1 SEN_DEL_L4_1 cell per tap providing: 
    granularity of max. ~0.4ns */
    CY_SMIF_2_SEN_SEL_PER_TAP  = 2, /**< 2 SEN_DEL_L4_1 cells per tap providing: 
    granularity of max. ~0.8ns */ 
    CY_SMIF_4_SEN_SEL_PER_TAP  = 3, /**< 4 SEN_DEL_L4_1 cells per tap providing: 
    granularity of max. ~1.6ns */ 
    CY_SMIF_NO_DELAY_SEL       = 0xFF,/**< No delay line (disabled) */ 
} cy_en_smif_delay_line_t;

/** Specifies use HW DLP calibration or SW DLP calibration */
typedef enum
{
    CY_SMIF_DLP_UPDATE_MANUAL = 0, /**< SW will calibrates and sets DLP */
    CY_SMIF_DLP_UPDATE_AUTO   = 1, /**< Use HW DLP calibration scheme */
} cy_en_smif_dlp_auto_t;

/* Specifies the read data capture cycle for internal clocking. Only applies when receiver clock source is internal clock.*/
typedef enum
{
    CY_SMIF_CAPTURE_DELAY_0_CYCLE = 0, /**< Cycle 0 */
    CY_SMIF_CAPTURE_DELAY_1_CYCLE = 1, /**< Cycle 1 */
    CY_SMIF_CAPTURE_DELAY_2_CYCLE = 2, /**< Cycle 2 */
} cy_en_smif_ddr_capture_delay_t;

/** The SMIF transfer status return values. */
typedef enum
{
    CY_SMIF_STARTED,       /**< The SMIF started. */
    CY_SMIF_SEND_CMPLT,    /**< The data transmission is complete. */
    CY_SMIF_SEND_BUSY,     /**< The data transmission is in progress. */
    CY_SMIF_REC_CMPLT,     /**< The data reception is completed. */
    CY_SMIF_REC_BUSY,      /**< The data reception is in progress. */
    CY_SMIF_XIP_ERROR,     /**< An XIP alignment error. */
    CY_SMIF_CMD_ERROR,     /**< A TX CMD FIFO overflow. */
    CY_SMIF_TX_ERROR,      /**< A TX DATA FIFO overflow. */
    CY_SMIF_RX_ERROR       /**< An RX DATA FIFO underflow. */
} cy_en_smif_txfr_status_t;

/** The SMIF API return values. */
typedef enum
{
    CY_SMIF_SUCCESS         = 0x00U,                                    /**< Successful SMIF operation. */
    CY_SMIF_CMD_FIFO_FULL   = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x01U,  /**< The command is cancelled. The command FIFO is full. */
    CY_SMIF_EXCEED_TIMEOUT  = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x02U,  /**< The SMIF operation timeout exceeded. */
    /**
    * The device does not have a QE bit. The device detects
    * 1-1-4 and 1-4-4 Reads based on the instruction.
    */
    CY_SMIF_NO_QE_BIT       = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x03U,
    CY_SMIF_BAD_PARAM       = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x04U,  /**< The SMIF API received the wrong parameter */
    CY_SMIF_NO_SFDP_SUPPORT = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x05U,  /**< The external memory does not support SFDP (JESD216B). */
    CY_SMIF_BAD_STATUS      = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x06U,  /**< The SMIF API received the wrong parameter */
    CY_SMIF_GENERAL_ERROR   = CY_SMIF_ID |CY_PDL_STATUS_ERROR | 0x07U,  /**< Some general error */
} cy_en_smif_status_t;

/** The SMIF slave select definitions for the driver API. Each slave select is
* represented by an enumeration that has the bit corresponding to the slave 
* select number set. */
typedef enum
{
    CY_SMIF_SLAVE_SELECT_0 = 1U,  /**< The SMIF slave select 0  */
  
#if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 2) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 2))
    CY_SMIF_SLAVE_SELECT_1 = 2U,  /**< The SMIF slave select 1  */
#endif
  
#if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 3) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 3))
    CY_SMIF_SLAVE_SELECT_2 = 4U,  /**< The SMIF slave select 2  */
#endif
  
#if (CY_SMIF_DRV_SMIF0_DEVICE_NR >= 4) || (defined(CY_SMIF_DRV_SMIF1_DEVICE_NR) && (CY_SMIF_DRV_SMIF1_DEVICE_NR >= 4))
    CY_SMIF_SLAVE_SELECT_3 = 8U   /**< The SMIF slave select 3  */
#endif
} cy_en_smif_slave_select_t;


/** Specifies the clock source for the receiver clock. */
typedef enum
{
    CY_SMIF_OUTPUT_CLK       = 0U,  /**< The SMIF output clock */
    CY_SMIF_INV_OUTPUT_CLK   = 1U,  /**< The SMIF output inverted clock */
    CY_SMIF_FEEDBACK_CLK     = 2U,  /**< The SMIF feedback clock */
    CY_SMIF_INV_FEEDBACK_CLK = 3U,  /**< The SMIF feedback inverted clock */
    CY_SMIF_INTERNAL_CLK     = 4U,  /**< The SMIF internal clock */
    CY_SMIF_INV_INTERNAL_CLK = 5U,  /**< The SMIF internal inverted clock */
    CY_SMIF_INV_RWDS         = 6U,  /**< The SMIF inverted rwds */
    CY_SMIF_RWDS             = 7U,  /**< The SMIF rwds */
} cy_en_smif_clk_rx_t;

/* Specifies the minimum duration of SPI deselection in between SPI transfers: */
typedef enum
{
    CY_SMIF_MIN_DESELECT_1_CLK = 0U,   /**<  1 memory interface clock cycle */
    CY_SMIF_MIN_DESELECT_2_CLK = 1U,   /**<  2 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_3_CLK = 2U,   /**<  3 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_4_CLK = 3U,   /**<  4 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_5_CLK = 4U,   /**<  5 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_6_CLK = 5U,   /**<  6 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_7_CLK = 6U,   /**<  7 memory interface clock cycles */
    CY_SMIF_MIN_DESELECT_8_CLK = 7U    /**<  8 memory interface clock cycles */
} cy_en_smif_deselection_t;

/* Specifies the duration between spi_select becomes low to 1st spi_clk edge */
/* min = 1 memory interface clock cycles when SDR, 1/4 memory interface clock cycles when DDR  */
typedef enum
{
    CY_SMIF_SETUP_0_CLK_PULUS_MIN = 0U,   /**< 0 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_1_CLK_PULUS_MIN = 1U,   /**< 1 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_2_CLK_PULUS_MIN = 2U,   /**< 2 memory interface clock cycles + min duration */
    CY_SMIF_SETUP_3_CLK_PULUS_MIN = 3U,   /**< 3 memory interface clock cycles + min duration */
} cy_en_smif_setup_delay_t;

/* Specifies the duration between last spi_clk edge to spi_select becomes high */
/* min = 1 memory interface clock cycles when SDR, 1/4 memory interface clock cycles when DDR  */
typedef enum
{
    CY_SMIF_HOLD_0_CLK_PULUS_MIN = 0U,   /**< 0 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_1_CLK_PULUS_MIN = 1U,   /**< 1 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_2_CLK_PULUS_MIN = 2U,   /**< 2 memory interface clock cycles + min duration */
    CY_SMIF_HOLD_3_CLK_PULUS_MIN = 3U,   /**< 3 memory interface clock cycles + min duration */
} cy_en_smif_hold_delay_t;

/** Specifies enabled type of SMIF cache. */
typedef enum
{
    CY_SMIF_CACHE_SLOW      = 1U,   /**< The SMIF slow cache (in the clk_slow domain) see TRM for details */
    CY_SMIF_CACHE_FAST      = 2U,   /**< The SMIF fast cache  (in the clk_fast domain) see TRM for details */
    CY_SMIF_CACHE_BOTH      = 3U    /**< The SMIF both caches */
} cy_en_smif_cache_en_t;

/** Specifies Data Rate. single data rate (SDR) or double data rate (DDR) */
typedef enum
{
    CY_SMIF_SDR = 0, /**< Single data rate */
    CY_SMIF_DDR = 1, /**< Double data rate */
} cy_en_smif_data_rate_t;

/** The size of the device region specified by DEVICE_MASK register */
typedef enum // casting int32_t is to avoid compiler error.
{
    CY_SMIF_DEVICE_1M_BYTE   = (int32_t)(0xFFF00000UL), /**< for 1M size device */
    CY_SMIF_DEVICE_2M_BYTE   = (int32_t)(0xFFE00000UL), /**< for 2M size device */
    CY_SMIF_DEVICE_4M_BYTE   = (int32_t)(0xFFC00000UL), /**< for 4M size device */
    CY_SMIF_DEVICE_8M_BYTE   = (int32_t)(0xFF800000UL), /**< for 8M size device */
    CY_SMIF_DEVICE_16M_BYTE  = (int32_t)(0xFF000000UL), /**< for 16M size device */
    CY_SMIF_DEVICE_32M_BYTE  = (int32_t)(0xFE000000UL), /**< for 32M size device */
    CY_SMIF_DEVICE_64M_BYTE  = (int32_t)(0xFC000000UL), /**< for 64M size device */
    CY_SMIF_DEVICE_128M_BYTE = (int32_t)(0xF8000000UL), /**< for 128M size device */
    CY_SMIF_DEVICE_256M_BYTE = (int32_t)(0xF0000000UL), /**< for 256M size device */
    CY_SMIF_DEVICE_512M_BYTE = (int32_t)(0xE0000000UL), /**< for 512M size device */
    CY_SMIF_DEVICE_1G_BYTE   = (int32_t)(0xC0000000UL), /**< for 1G size device */
    CY_SMIF_DEVICE_2G_BYTE   = (int32_t)(0x80000000UL), /**< for 2G size device */
    CY_SMIF_DEVICE_4G_BYTE   = (int32_t)(0x00000000UL), /**< for 4G size device */
} cy_en_device_size_t;

/** Specifies the size of the XIP device address in Bytes */
typedef enum
{
    CY_SMIF_XIP_ADDRESS_1_BYTE = 0, /**< 1 Byte address */
    CY_SMIF_XIP_ADDRESS_2_BYTE = 1, /**< 2 Byte address */
    CY_SMIF_XIP_ADDRESS_3_BYTE = 2, /**< 3 Byte address */
    CY_SMIF_XIP_ADDRESS_4_BYTE = 3, /**< 4 Byte address */
    CY_SMIF_XIP_ADDRESS_5_BYTE = 7, /**< 4 Byte address (spread over 5 bytes) according to Hyperbus protocol */
} cy_en_smif_xip_addr_byte_t;

/** Continuous transfer merge timeout in clk_mem cycles */
typedef enum
{
    CY_SMIF_MER_TIMEOUT_1_CYCLE     = 0, /**< Timeout after 1 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_16_CYCLE    = 1, /**< Timeout after 16 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_256_CYCLE   = 2, /**< Timeout after 256 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_4096_CYCLE  = 3, /**< Timeout after 4096 clk_mem cycle */
    CY_SMIF_MER_TIMEOUT_65536_CYCLE = 4, /**< Timeout after 65536 clk_mem cycle */
} cy_en_smif_dev_merge_timeout_t;

/** Specify mode/dummy presence */
typedef enum
{
    CY_SMIF_NOT_PRESENT   = 0, /**< mode/dummy not present */
    CY_SMIF_PRESENT_1BYTE = 1, /**< mode/dummy 1 byte present */
    CY_SMIF_PRESENT_2BYTE = 2, /**< mode/dummy 2 byte present */
} cy_en_smif_field_presence_t;

/** Specify whether command to be transmitted is last one or not */
typedef enum
{
    NOT_LAST_COMMAND = 0, /**< not last command */
    LAST_COMMAND_BYTE = 1,/**< last command */
} cy_en_smif_cmd_last_t;

/** Specifies which of the Functional Safety Status field bits are checked */
typedef enum
{
    CY_SMIF_CHECK_FUNC_SAFETY      = 0, /**< The associated Functional Safety Status bit is checked */
    CY_SMIF_DONT_CHECK_FUNC_SAFETY = 1, /**< The associated Functional Safety Status bit is not checked
    If the received Functional Safety Status bit = Functional Safety Status Error Polarity (STATUS_ERROR_POLARITY),
    then a Functional Safety Status error interrupt and a XIP bus error response is generated.  */
} cy_en_smif_crc_ctl_status_check_t;

/** Specifies the polarity of the Functional Safety Status field bits */
typedef enum
{
    CY_SMIF_ERROR_POL_ACTIVE_LOW  = 0, /**< The associated Functional Safety Status bit is active-low */
    CY_SMIF_ERROR_POL_ACTIVE_HIGH = 1, /**< The associated Functional Safety Status bit is active-high */
} cy_en_smif_crc_ctl_error_pol_t;

/** Specifies which fields are included in the command / address CRC generation */
typedef enum
{
    CY_SMIF_INC_ADDR_MODE      = 0, /**< The command / address CRC field is generated over the address and (if present) mode fields only.*/
    CY_SMIF_INC_CMD_ADDR_MODE  = 1, /**< The command / address CRC field is generated over the command, address and (if present) mode fields.*/
} cy_en_smif_crc_input_scope_t;

/** Specifies the size of a memory sub page */
typedef enum
{
    SUB_PAGE_SIZE_8BYTE  = 0, /**< sub_page_size = 8 words = 16 bytes (default) */
    SUB_PAGE_SIZE_16BYTE = 1, /**< sub_page_size = 16 words = 32 bytes */
    SUB_PAGE_SIZE_32BYTE = 2, /**< sub_page_size = 32 words = 64 bytes */
    SUB_PAGE_SIZE_64BYTE = 3,/**< sub_page_size = 64 words = 128 bytes */
} en_cy_sub_page_size_t;

/** Specifies the number of sub pages per page */
typedef enum
{
    SUB_PAGE_1_PER_PAGE = 0, /**< 1 sub pages per page, i.e. page_size = sub_page_size */
    SUB_PAGE_2_PER_PAGE = 1, /**< 2 sub pages per page, i.e. page_size = 2 x sub_page_size */
    SUB_PAGE_4_PER_PAGE = 2, /**< 4 sub pages per page, i.e. page_size = 4 x sub_page_size */
    SUB_PAGE_8_PER_PAGE = 3, /**< 8 sub pages per page, i.e. page_size = 8 x sub_page_size */
} en_cy_sub_page_nr_t;

/** Specifies the DLL speed mode */
typedef enum
{
    CY_SMIF_DLL_SPEED_160_TO_180_MHZ = 0,       /**< 160 MHz to 180 MHz PLL clock frequency for DLL */
    CY_SMIF_DLL_SPEED_180_TO_266_MHZ = 1,       /**< 181 MHz to 266 MHz PLL clock frequency for DLL */
    CY_SMIF_DLL_SPEED_266_TO_333_MHZ = 2,       /**< 267 MHz to 333 MHz PLL clock frequency for DLL */
    CY_SMIF_DLL_SPEED_333_TO_400_MHZ = 3,       /**< 334 MHz to 400 MHz PLL clock frequency for DLL */
} cy_en_smif_dll_clk_mode_t;

/** Specifies the RX capture mode */
typedef enum
{
    CY_SMIF_RX_CAPTURE_MODE_SPI      = 0,       /**< Receive capture scheme - normal SPI without DLP */
    CY_SMIF_RX_CAPTURE_MODE_SPI_DLP  = 1,       /**< Receive capture scheme - normal SPI with DLP */
    CY_SMIF_RX_CAPTURE_MODE_XSPI_HB  = 2,       /**< Receive capture scheme - Hyperbus or xSPI */
} cy_en_smif_cap_mode_t;

/** \} group_smif_enums */
  
  
/**
* \addtogroup group_smif_data_structures
* \{
*/


/***************************************************************************//**
*
* The SMIF user callback function type called at the end of a transfer.
*
* \param event
* The event which caused a callback call.
*
*******************************************************************************/
typedef void (*cy_smif_event_cb_t)(uint32_t event);

/** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "TX" command */
typedef struct
{
    uint32_t firstByte  : 8; /**< Specifies the transmitted Byte */
    uint32_t secondByte : 8; /**< Specifies the second transmitted Byte. This is only used for octal data transfer with DDR mode */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
} cy_stc_smif_cmd_fifo_tx_t;

/** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "TX_COUNT" command */
typedef struct
{
    uint32_t dataNum    : 16;/**< Specifies the number of to be transmitted memory data units */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
} cy_stc_smif_cmd_fifo_tx_count_t;

/** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "RX_COUNT" command */
typedef struct
{
    uint32_t dataNum    : 16;/**< Specifies the number of to be received memory data units */
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t selectDev  : 4; /**< Specifies which of the four devices are selected */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
} cy_stc_smif_cmd_fifo_rx_count_t;

/** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "DUMMY_COUNT" command */
typedef struct
{
    uint32_t cycleNum   : 16;
    uint32_t width      : 2; /**< Specifies the width of the data transfer */
    uint32_t dataRate   : 1; /**< Specifies the mode of transfer rate */
    uint32_t last       : 1; /**< Specifies whether this is the last TX Byte */
    uint32_t doubleDm   : 1; /**< specifies whether an active RWDS refresh indicator (captured with the previous rising memory clock output) causes to double the number of dummy cycles. This is used for the variable latency in HyperRAM. */
    uint32_t rwdsInDm   : 1; /**< specifies whether the RWDS output signal should be driven starting in the last dummy cycle until DESELECT. This is needed for write transactions with RWDS (e.g. Hyperbus) before a TX_COUNT command. */
    uint32_t dlpEn      : 1; /**< specifies whether data learning is enabled. In XIP mode this field is set for read transactions when CTL.INT_CLOCK_DL_ENABLED is "1"). */
    uint32_t capEn      : 1; /**< specifies whether RWDS / DQS based capturing is enabled in the last dummy cycle, i.e. this needs to be set to '1' for read transactions to memory device using RWDS / DQS capturing scheme. */
    uint32_t cmd        : 3; /**< Specifies the specific command */
    uint32_t reserved0  : 5; /**< Reserved */
} cy_stc_smif_cmd_fifo_dm_t;

/** Specifies the bit field of TX_CMD_FIFO_WR when cmd = "DESELECT" command */
typedef struct
{
    uint32_t reserved1  : 24; /**< Reserved */
    uint32_t cmd        : 3;  /**< Specifies the specific command */
    uint32_t reserved0  : 5;  /**< Reserved */
} cy_stc_smif_cmd_fifo_deselect_t;


/** union for TX_CMD_FIFO_WR as per it's commands */
typedef union
{
    cy_stc_smif_cmd_fifo_tx_t       fieldTx;       /**< Bit field when cmd = "TX" command \ref cy_stc_smif_cmd_fifo_tx_t */
    cy_stc_smif_cmd_fifo_tx_count_t fieldTxCount;  /**< Bit field when cmd = "TX_COUNT" command \ref cy_stc_smif_cmd_fifo_tx_count_t */
    cy_stc_smif_cmd_fifo_rx_count_t fieldRxCount;  /**< Bit field when cmd = "RX_COUNT" command \ref cy_stc_smif_cmd_fifo_rx_count_t */
    cy_stc_smif_cmd_fifo_dm_t       fieldDummy;    /**< Bit field when cmd = "DUMMY_COUNT" command \ref cy_stc_smif_cmd_fifo_dm_t */
    cy_stc_smif_cmd_fifo_deselect_t fieldDeselect; /**< Bit field when cmd = "DESELECT" command \ref cy_stc_smif_cmd_fifo_deselect_t */
    uint32_t                        u32;           /**< 32-bit access */
} cy_un_smif_cmd_fifo_t;


/** The SMIF configuration structure. */
typedef struct
{
    cy_en_smif_clk_tx_t            txClk;         /**< TX clock source selection \ref cy_en_smif_clk_tx_t (ignored from SMIF_4) */
    cy_en_smif_clk_rx_t            rxClk;         /**< RX clock source selection \ref cy_en_smif_clk_rx_t (ignored from SMIF_4) */
    cy_en_smif_delay_line_t        delaySel;      /**< Delay line selection for RX clock \ref cy_en_smif_delay_line_t (ignored from SMIF_4) */
    cy_en_smif_dlp_auto_t          dlpAuto;       /**< Use SW calibration or HW calibration \ref cy_en_smif_dlp_auto_t (ignored from SMIF_4) */
    cy_en_smif_ddr_capture_delay_t capDelay;      /**< Selection for capture delay \ref cy_en_smif_ddr_capture_delay_t (ignored from SMIF_4) */
    cy_en_smif_deselection_t       deselectDelay; /**< \ref cy_en_smif_deselection_t */
    cy_en_smif_setup_delay_t       setupDelay;    /**< \ref cy_en_smif_setup_delay_t */
    cy_en_smif_hold_delay_t        holdDelay;     /**< \ref cy_en_smif_hold_delay_t */ 
    cy_en_smif_mode_t              mode;          /**<  Specifies the mode of operation \ref cy_en_smif_mode_t. */
    cy_en_smif_error_event_t       blockEvent;    /**< Specifies what happens when there is a Read  
    * from an empty RX FIFO or a Write to a full 
    * TX FIFO. \ref cy_en_smif_error_event_t. */
    cy_en_cy_clkif_source_t        clkIfSrc;      /**< \ref cy_en_cy_clkif_source_t */
    void*                          pDllCfg;       /**< Pointer to configuration parameter structure for DLL. This is ignored for smif driver versions other than SMIF_4 
                                                       Set pointer to \ref cy_stc_smif_dll_config_t for SMIF_4 */
} cy_stc_smif_config_t;


/** The SMIF internal context data. The user must not modify it. */
typedef struct
{
    uint8_t volatile * volatile txBufferAddress;    /**<  The pointer to the data to transfer */
    /**
    * The transfer counter.
    */
    uint32_t volatile txBufferCounter;
    uint8_t volatile * volatile rxBufferAddress;    /**<  The pointer to the variable where the received data is stored */
    /**
    * The transfer counter.
    */
    uint32_t volatile rxBufferCounter;
    /**
    * The status of the transfer. The transmitting / receiving is completed / in progress
    */
    cy_en_smif_txfr_status_t volatile transferStatus;
    cy_smif_event_cb_t volatile txCmpltCb;          /**< The user-defined callback executed at the completion of a transmission */
    cy_smif_event_cb_t volatile rxCmpltCb;          /**< The user-defined callback executed at the completion of a reception */
    /**
    * The timeout in microseconds for the blocking functions. This timeout value applies to all blocking APIs.
    */
    uint32_t timeout;
    cy_en_smif_data_rate_t  preCmdDataRate;
    cy_en_smif_txfr_width_t preCmdWidth;
    uint32_t volatile txCmdBufCounter;
} cy_stc_smif_context_t;

/** TX/RX CMD/MODE control in XIP mode */
typedef struct
{
    uint16_t                    cmd;      /**< Command byte code. If presence = CY_SMIF_PRESENT_1BYTE, only lower 8bit is active. */
    cy_en_smif_txfr_width_t     width;    /**< Width of data transfer \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t      dataRate; /**< Mode of transfer rate \ref cy_en_smif_data_rate_t */
    cy_en_smif_field_presence_t presence; /**< Presence of command field \ref cy_en_smif_field_presence_t */
} stc_cy_smif_cmd_mode_ctl_t;

/** TX/RX ADDR/DATA control in XIP mode */
typedef struct
{
    cy_en_smif_txfr_width_t width;    /**< Width of data transfer \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t  dataRate; /**< Mode of transfer rate \ref cy_en_smif_data_rate_t */
} stc_cy_smif_addr_data_ctl_t;

/** RX DUMMY control in XIP mode */
typedef struct
{
    uint8_t numOfDummy;                   /**< Number of dummy cycles */
    cy_en_smif_field_presence_t presence; /**< Presence of dummy cycles. If presence == CY_SMIF_PRESENT_2BYTE, Dummy cycles are doubled
    when RWDS refresh indicator is high during CA cycle. (HyperRAM variable latency mode) */
} stc_cy_smif_rd_dummy_ctl_t;

/** TX DUMMY control in XIP mode */
typedef struct
{
    uint8_t numOfDummy;                   /**< Number of dummy cycles */
    bool    rwdsEn;                       /**< Read-Write-Data-Strobe Enable. Specifies whether the RWDS output signal
    should be driven starting in the last dummy cycle until DESELECT.
    This is needed for write transactions with byte masking via RWDS (e.g. Hyperbus).
    '0': do not drive RWDS output
    '1': drive RWDS output starting in last dummy cycle until DESELECT */
    cy_en_smif_field_presence_t presence; /**< Presence of dummy cycles. If presence == CY_SMIF_PRESENT_2BYTE, Dummy cycles are doubled
    when RWDS refresh indicator is high during CA cycle. (HyperRAM variable latency mode) */
} stc_cy_smif_wr_dummy_ctl_t;

/** RX CRC control in XIP mode */
typedef struct
{
    uint8_t                        inputSize;          /**< Number of input data bytes for CRC generation */
    cy_en_smif_crc_ctl_status_check_t funcSafeChecked; /**< Specifies which of the Functional Safety Status field bits are checked.
    There is 1 mask bit for each Functional Safety Status bit. 
    \ref cy_en_smif_crc_ctl_status_check_t */
    cy_en_smif_crc_ctl_error_pol_t errPol;             /**< Specifies the polarity of the Functional Safety Status field bits.
    There is 1 polarity bit for each Functional Safety Status bit. 
    \ref cy_en_smif_crc_ctl_status_check_t */
    cy_en_smif_txfr_width_t        width;              /**< Width of command / address CRC field. \ref cy_en_smif_txfr_width_t */
    
    cy_en_smif_data_rate_t         dataRate;           /**< Mode of transfer rate of command / address CRC field \ref cy_en_smif_data_rate_t */
    
    cy_en_smif_crc_input_scope_t   inputScope;         /**< Specifies which fields are included in the command / address CRC generation \ref cy_en_smif_crc_input_scope_t */
    bool                           crcChecked;         /**< Specifies whether or not the read data CRC field is checked.
    '0': Read data CRC field is reserved in RX data stream, but not checked.
    '1': Read data CRC field is checked in RX data stream. If CRC does not match an CRC error interrupt and a XIP bus error response is generated. */
    bool                           cmd_addrCrcPresent; /**< Presence of command / address CRC field */
    bool                           dataCrcPresent;     /**< Presence of data CRC field */
} stc_cy_smif_rd_crc_ctl_t;

/** TX CRC control in XIP mode */
typedef struct
{
    uint8_t                      inputSize;            /**< Number of input data bytes for CRC generation */
    cy_en_smif_txfr_width_t      width;                /**< Width of command / address CRC field \ref cy_en_smif_txfr_width_t */
    cy_en_smif_data_rate_t       dataRate;             /**< Mode of transfer rate of command / address CRC field \ref cy_en_smif_data_rate_t */
    cy_en_smif_crc_input_scope_t inputScope;           /**< Specifies which fields are included in the command / address CRC generation \ref cy_en_smif_crc_input_scope_t*/
    bool                         cmd_addrCrcPresent;   /**< Presence of command / address CRC field */
    bool                         dataCrcPresent;       /**< Presence of data CRC field */
} stc_cy_smif_wr_crc_ctl_t;

/** RX BOUND control in XIP mode */
typedef struct
{
    uint8_t               numOfCycle;  /**< Number of base latency cycles used for calculating the number of fist page boundary crossing latency cycles */
    en_cy_sub_page_size_t subPageSize; /**< Specifies the size of a memory sub page \ref en_cy_sub_page_size_t */
    en_cy_sub_page_nr_t   subPageNr;   /**< Specifies the number of sub pages per page \ref en_cy_sub_page_nr_t */
    bool                  subsqBndEn;  /**< Enable subsequent page boundary latency cycles */
    bool                  bdLPresent;  /**< Presence of first page boundary latency cycles */
} stc_cy_smif_rd_bound_ctl_t;

/** \} group_smif_data_structures */


/**
* \addtogroup group_smif_low_level_functions
* \{
*/

extern cy_en_smif_mode_t                   Cy_SMIF_GetMode(volatile cy_stc_smif_reg_t *base);
extern void                                Cy_SMIF_DeviceSetDataSelect(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_data_select_t sel);
extern void                                Cy_SMIF_DeviceEnable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceDisable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceWriteEnable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceWriteDisable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceCryptoEnable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceCryptoDisable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceTransferMergingEnable(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_dev_merge_timeout_t timeout);
extern void                                Cy_SMIF_DeviceTransferMergingDisable(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_DeviceTotalTimeoutEnable(volatile cy_stc_smif_reg_device_t *device, uint16_t timeout);
extern void                                Cy_SMIF_DeviceTotalTimeoutDisable(volatile cy_stc_smif_reg_device_t *device);
extern uint8_t                             Cy_SMIF_GetReadStatus(volatile cy_stc_smif_reg_device_t *device);
extern void                                Cy_SMIF_SetXIPBaseAddress(volatile cy_stc_smif_reg_device_t *device, uint32_t addr);
extern void                                Cy_SMIF_SetXIPAddressSize(volatile cy_stc_smif_reg_device_t *device, cy_en_device_size_t size);
extern void                                Cy_SMIF_SetXIPAddrCtl(volatile cy_stc_smif_reg_device_t *device, cy_en_smif_xip_addr_byte_t addrSize, bool divForDualQuad);
extern void                                Cy_SMIF_SetReadCmdCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t *config);
extern void                                Cy_SMIF_SetWriteCmdCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t *config);
extern void                                Cy_SMIF_SetReadAddrCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t *config);
extern void                                Cy_SMIF_SetWriteAddrCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t *config);
extern void                                Cy_SMIF_SetReadModeCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t *config);
extern void                                Cy_SMIF_SetWriteModeCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_cmd_mode_ctl_t *config);
extern void                                Cy_SMIF_SetReadDummyCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_dummy_ctl_t *config);
extern void                                Cy_SMIF_SetWriteDummyCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_wr_dummy_ctl_t *config);
extern void                                Cy_SMIF_SetReadDataCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t *config);
extern void                                Cy_SMIF_SetWriteDataCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_addr_data_ctl_t *config);
extern void                                Cy_SMIF_SetReadCrcCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_crc_ctl_t *config);
extern void                                Cy_SMIF_SetWriteCrcCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_wr_crc_ctl_t *config);
extern void                                Cy_SMIF_SetReadBoundCtl(volatile cy_stc_smif_reg_device_t *device, const stc_cy_smif_rd_bound_ctl_t *config);
extern void                                Cy_SMIF_Disable(volatile cy_stc_smif_reg_t *base);
extern void                                Cy_SMIF_SetInterruptMask(volatile cy_stc_smif_reg_t *base, uint32_t interrupt);
extern uint32_t                            Cy_SMIF_GetInterruptMask(volatile cy_stc_smif_reg_t *base);
extern uint32_t                            Cy_SMIF_GetInterruptStatusMasked(volatile cy_stc_smif_reg_t *base);
extern uint32_t                            Cy_SMIF_GetInterruptStatus(volatile cy_stc_smif_reg_t *base);
extern void                                Cy_SMIF_SetInterrupt(volatile cy_stc_smif_reg_t *base, uint32_t interrupt);
extern void                                Cy_SMIF_ClearInterrupt(volatile cy_stc_smif_reg_t *base, uint32_t interrupt);
extern void                                Cy_SMIF_SetRxFifoTriggerLevel(volatile cy_stc_smif_reg_t *base, uint32_t level);
extern uint32_t                            Cy_SMIF_GetCmdFifoStatus(volatile cy_stc_smif_reg_t *base);
extern uint32_t                            Cy_SMIF_GetRxFifoStatus(volatile cy_stc_smif_reg_t *base);
extern bool                                Cy_SMIF_IsBusy(volatile cy_stc_smif_reg_t *base);
extern void                                Cy_SMIF_Interrupt(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context);
extern void                                Cy_SMIF_PopRxFifo(volatile cy_stc_smif_reg_t *baseaddr, cy_stc_smif_context_t *context);
extern uint8_t                             Cy_SMIF_CheckRxFifoContents(volatile cy_stc_smif_reg_t *baseaddr);
extern uint32_t                            Cy_SMIF_PackBytesArray(uint8_t const buff[], bool fourBytes);
extern void                                Cy_SMIF_UnPackByteArray(uint32_t inValue, uint8_t outBuff[], bool fourBytes);
extern cy_en_smif_status_t                 Cy_SMIF_TimeoutRun(uint32_t *timeoutUnits);
extern uint32_t                            CY_SMIF_GetDeviceNumber(volatile void *base);
extern uint32_t                            CY_SMIF_GetDelayTapsNumber(volatile void *base);
extern uint32_t                            CY_SMIF_GetDelayLinesNumber(volatile void *base);
extern uint32_t                            CY_SMIF_GetXIP_Mask(volatile void *base);
extern uint32_t                            CY_SMIF_GetXIP_Address(volatile void *base);
extern uint32_t                            Cy_SMIF_MaskValueToSizeInByte(cy_en_device_size_t mask);
extern uint32_t                            Cy_SMIF_Reverse4ByteEndian(uint32_t in);
extern cy_en_smif_status_t                 Cy_SMIF_TransmitCommand(volatile cy_stc_smif_reg_t *base, uint16_t cmd, bool isCommand2Byte, cy_en_smif_txfr_width_t cmdTxfrWidth, cy_en_smif_data_rate_t cmdDataRate, uint8_t const cmdParam[], uint32_t paramSize, cy_en_smif_txfr_width_t paramTxfrWidth, cy_en_smif_data_rate_t paramDataRate, cy_en_smif_slave_select_t slaveSelect, cy_en_smif_cmd_last_t cmpltTxfr, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_TransmitData(volatile cy_stc_smif_reg_t *base, const uint8_t *txBuffer, uint32_t size, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataDataRate, cy_en_smif_slave_select_t slaveSelect, cy_smif_event_cb_t TxCmpltCb, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_TransmitDataBlocking(volatile cy_stc_smif_reg_t *base, const uint8_t *txBuffer, uint32_t size, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataDataRate, cy_en_smif_slave_select_t slaveSelect, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_ReceiveDataCmd(volatile cy_stc_smif_reg_t *base, uint8_t *rxBuffer, uint32_t size, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataRate, cy_en_smif_slave_select_t slaveSelect, cy_smif_event_cb_t RxCmpltCb, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_ReceiveData(volatile cy_stc_smif_reg_t *base, uint8_t *rxBuffer, uint32_t size, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataRate, cy_en_smif_slave_select_t slaveSelect, cy_smif_event_cb_t RxCmpltCb, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_ReceiveDataBlocking(volatile cy_stc_smif_reg_t *base, uint8_t *rxBuffer, uint32_t size, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataRate, cy_en_smif_slave_select_t slaveSelect, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_SendDummyCycles(volatile cy_stc_smif_reg_t *base, cy_en_smif_txfr_width_t transferWidth, cy_en_smif_data_rate_t dataRate, bool doubleDm, bool rwdsInDm, bool dlpEn, bool capEn, uint32_t cycles, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_Deselect(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context);
extern cy_en_smif_txfr_status_t            Cy_SMIF_GetTxfrStatus(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t const *context);
extern void                                Cy_SMIF_Enable(volatile cy_stc_smif_reg_t *base, cy_stc_smif_context_t *context);
extern cy_en_smif_status_t                 Cy_SMIF_CacheEnable(volatile cy_stc_smif_reg_t *base, cy_en_smif_cache_en_t cacheType);
extern cy_en_smif_status_t                 Cy_SMIF_CacheDisable(volatile cy_stc_smif_reg_t *base, cy_en_smif_cache_en_t cacheType);
extern cy_en_smif_status_t                 Cy_SMIF_CachePrefetchingEnable(volatile cy_stc_smif_reg_t *base, cy_en_smif_cache_en_t cacheType);
extern cy_en_smif_status_t                 Cy_SMIF_CachePrefetchingDisable(volatile cy_stc_smif_reg_t *base, cy_en_smif_cache_en_t cacheType);
extern cy_en_smif_status_t                 Cy_SMIF_CacheInvalidate(volatile cy_stc_smif_reg_t *base, cy_en_smif_cache_en_t cacheType);
extern cy_en_smif_status_t                 Cy_SMIF_SetMode(volatile cy_stc_smif_reg_t *base, cy_en_smif_mode_t mode);
extern cy_en_smif_status_t                 Cy_SMIF_CRC_Calibration(volatile cy_stc_smif_reg_t *base, uint8_t inData[], uint32_t sizeInByte, uint8_t *result);
extern volatile cy_stc_smif_reg_t *        Cy_SMIF_GetSmifBasePtrFromAnySmifPtr(volatile void *any_smif_address);
extern cy_en_smif_slave_select_t           Cy_SMIF_GetSlaveSelectFromDeviceBasePtr(volatile cy_stc_smif_reg_device_t *device);
extern cy_en_smif_slave_select_t           Cy_SMIF_GetSlaveSelectFromDeviceNumber(uint8_t device_number);
extern volatile cy_stc_smif_reg_device_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndSlaveSelect(volatile cy_stc_smif_reg_t *base, cy_en_smif_slave_select_t slave);
extern volatile cy_stc_smif_reg_device_t * Cy_SMIF_GetDeviceBasePtrFromSmifBasePtrAndDeviceNumber(volatile cy_stc_smif_reg_t *base, uint8_t device_number);
extern cy_en_syspm_status_t                Cy_SMIF_DeepSleepCallback(cy_stc_syspm_callback_params_t *callbackParams);
extern cy_en_syspm_status_t                Cy_SMIF_HibernateCallback(cy_stc_syspm_callback_params_t *callbackParams);

/** \} group_smif_low_level_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_IP_MXSMIF */

#endif /* (CY_SMIF_H) */

/** \} group_smif */


/* [] END OF FILE */
