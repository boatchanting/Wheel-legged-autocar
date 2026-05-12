/***************************************************************************//**
* \file cy_gpio.h
* \version 1.0
*
* \brief
* Provides an API declaration of the GPIO driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_gpio General Purpose Input Output (GPIO)
* \{
* The GPIO driver provides an API to configure and access device Input/Output pins.
* IO pins include all general purpose types such as GPIO, SIO, HSIO, AUXIO, and
* their variants.
*
* Initialization can be performed either at the port level or by configuring the
* individual pins. For efficient use of code space, port
* configuration should be used in the field. Refer to the product device header files 
* for the list of supported ports and pins.
* 
* - Single pin configuration is performed by using \ref Cy_GPIO_Pin_FastInit 
*   (provide specific values) or \ref Cy_GPIO_Pin_Init (provide a filled
*   cy_stc_gpio_pin_config_t structure).
* - An entire port can be configured using \ref Cy_GPIO_Port_Init. Provide a filled 
*   cy_stc_gpio_prt_config_t structure. The values in the structure are 
*   bitfields representing the desired value for each pin in the port.
* - Pin configuration and management is based on the port address and pin number.
*   \ref Cy_GPIO_PortToAddr function can optionally be used to calculate the port
*   address from the port number at run-time.
*
* Once the pin/port initialization is complete, each pin can be accessed by 
* specifying the port (stc_GPIO_PRT_t) and the pin (0-7) in the provided API 
* functions.
*
* \section group_gpio_configuration Configuration Considerations
*
* 1. Pin muxing is controlled through the High-Speed IO Matrix (HSIOM) selection.
*    This allows the pin to connect to signal sources/sinks throughout the device,
*    as defined by the pin HSIOM selection options (en_hsiom_sel_t).
* 2. All pins are initialized to High-Z drive mode with HSIOM connected to CPU (SW
*    control digital pin only) at Power-On-Reset(POR).
* 3. Some API functions perform read-modify-write operations on shared port
*    registers. These functions are not thread safe and care must be taken when
*    called by the application.
*
* \section group_gpio_more_information More Information
*
* Refer to the technical reference manual (TRM) and the device datasheet.
*
*
* The gpio driver does not have any specific deviations.
*
* \section group_gpio_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_gpio_macro Macro
* \defgroup group_gpio_functions Functions
* \{
*   \defgroup group_gpio_functions_init       Initialization Functions
*   \defgroup group_gpio_functions_gpio       GPIO Functions
*   \defgroup group_gpio_functions_sio        SIO Functions
*   \defgroup group_gpio_functions_interrupt  Port Interrupt Functions
* \}
* \defgroup group_gpio_data_structures Data structures
* \defgroup group_gpio_enums Enumerated types
*/

#if !defined(CY_GPIO_H)
#define CY_GPIO_H

#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

/** \addtogroup group_gpio_macro
* \{
*/

/** Driver major version */
#define CY_GPIO_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_GPIO_DRV_VERSION_MINOR       0

/** GPIO driver ID */
#define CY_GPIO_ID                      CY_PDL_DRV_ID(0x16u)

/** \} group_gpio_macro */


/***************************************
*       Enumerations
***************************************/
/**
* \addtogroup group_gpio_enums
* \{
*/

/**
* GPIO Driver error codes
*/
typedef enum 
{
    CY_GPIO_SUCCESS   = 0x00u,                                    /**< Returned successful */
    CY_GPIO_BAD_PARAM = CY_GPIO_ID | CY_PDL_STATUS_ERROR | 0x01u, /**< Bad parameter was passed */
} cy_en_gpio_status_t;

/** \} group_gpio_enums */


/***************************************
*       Configuration Structures
***************************************/

/**
* \addtogroup group_gpio_data_structures
* \{
*/

/** This structure is used to initialize a port of GPIO pins */
typedef struct {
    uint32_t out;           /**< Initial output data for the IO pins in the port */
    uint32_t intrMask;      /**< Interrupt enable mask for the port interrupt */
    uint32_t intrCfg;       /**< Port pin interrupt edge detection configuration */
    uint32_t cfg;           /**< Port pin drive modes and input buffer enable configuration */
    uint32_t cfgIn;         /**< Port pin input buffer configuration */
    uint32_t cfgOut;        /**< Port pin output buffer configuration */
    uint32_t cfgSIO;        /**< Port SIO pins configuration */
    uint32_t sel0Active;    /**< HSIOM selection for port pins 0,1,2,3 */
    uint32_t sel1Active;    /**< HSIOM selection for port pins 4,5,6,7 */
} cy_stc_gpio_prt_config_t;

/** This structure is used to initialize a single GPIO pin */
typedef struct {
    uint32_t outVal;         /**< Pin output state */
    uint32_t driveMode;      /**< Drive mode */
    en_hsiom_sel_t hsiom;   /**< HSIOM selection */
    uint32_t intEdge;        /**< Interrupt Edge type */
    uint32_t intMask;        /**< Interrupt enable mask */
    uint32_t vtrip;          /**< Input buffer voltage trip type */
    uint32_t slewRate;       /**< Output buffer slew rate */
    uint32_t driveSel;       /**< Drive strength */
    uint32_t vregEn;         /**< SIO pair output buffer mode */
    uint32_t ibufMode;       /**< SIO pair input buffer mode */
    uint32_t vtripSel;       /**< SIO pair input buffer trip point */
    uint32_t vrefSel;        /**< SIO pair reference voltage for input buffer trip point */
    uint32_t vohSel;         /**< SIO pair regulated voltage output level */
} cy_stc_gpio_pin_config_t;

/** This structure is used to initialize an array of GPIO pin */
typedef struct
{
    volatile stc_GPIO_PRT_t* portReg; /**Port Number**/
    uint8_t                  pinNum;  /**Pin Number**/
    cy_stc_gpio_pin_config_t cfg;     /**GPIO Pin Configuration**/
} cy_stc_gpio_pin_prt_config_t;

/** \} group_gpio_data_structures */

/***************************************
*        Constants
***************************************/

/** \cond INTERNAL */

/* General Constants */
#define CY_GPIO_PRT_HALF                       (4ul)      /**< Half-way point of a GPIO port */
#define CY_GPIO_PRT_DEINIT                     (0ul)      /**< De-init value for port registers */

/* GPIO Masks */
#define CY_GPIO_HSIOM_MASK                     (0x1Ful)   /**< HSIOM selection mask */
#define CY_GPIO_OUT_MASK                       (0x01ul)   /**< Single pin mask for OUT register */
#define CY_GPIO_IN_MASK                        (0x01ul)   /**< Single pin mask for IN register */
#define CY_GPIO_CFG_DM_MASK                    (0x0Ful)   /**< Single pin mask for drive mode in CFG register */
#define CY_GPIO_CFG_IN_VTRIP_SEL_MASK          (0x01ul)   /**< Single pin mask for VTRIP selection in CFG IN register */
#define CY_GPIO_CFG_OUT_SLOW_MASK              (0x01ul)   /**< Single pin mask for slew rate in CFG OUT register */
#define CY_GPIO_CFG_OUT_DRIVE_SEL_MASK         (0x03ul)   /**< Single pin mask for drive strength in CFG OUT register */
#define CY_GPIO_CFG_OUT2_DRIVE_SEL_TRIM_MASK   (0x07ul)   /**< Single pin mask for drive select trim in CFG OUT2 register */
#define CY_GPIO_CFG_SLEW_EXT_SEL_MASK          (0x07ul)   /**< Single pin mask for slow slew rate of the IO pin */
#define CY_GPIO_CFG_DRIVE_EXT_SEL_MASK         (0x1Ful)   /**< Single pin mask for drive sel extention strength of IO pin */
#define CY_GPIO_INTR_STATUS_MASK               (0x01ul)   /**< Single pin mask for interrupt status in INTR register */
#define CY_GPIO_INTR_EN_MASK                   (0x01ul)   /**< Single pin mask for interrupt status in INTR register */
#define CY_GPIO_INTR_MASKED_MASK               (0x01ul)   /**< Single pin mask for masked interrupt status in INTR_MASKED register */
#define CY_GPIO_INTR_SET_MASK                  (0x01ul)   /**< Single pin mask for setting the interrupt in INTR_MASK register */
#define CY_GPIO_INTR_EDGE_MASK                 (0x03ul)   /**< Single pin mask for interrupt edge type in INTR_EDGE register */
#define CY_GPIO_INTR_FLT_EDGE_MASK             (0x07ul)   /**< Single pin mask for setting filtered interrupt */

/* Bit offsets for port pin */
#define CY_GPIO_HSIOM_OFFSET                   (3ul)      /**< Port pin bits offset for HSIOM */
#define CY_GPIO_DRIVE_MODE_OFFSET              (2ul)      /**< Port pin bits offset for Drive mode */
#define CY_GPIO_INBUF_OFFSET                   (3ul)      /**< Port pin bits offset for input buffer */
#define CY_GPIO_INTR_CFG_OFFSET                (1ul)      /**< Port pin bits offset for interrupt config */
#define CY_GPIO_CFG_SIO_OFFSET                 (2ul)      /**< Port pin bits offset for SIO config */
#define CY_GPIO_CFG_OUT_DRIVE_OFFSET           (1ul)      /**< Port pin bits offset for drive strength */
#define CY_GPIO_DRIVE_TRIM_OFFSET              (3ul)      /**< Port pin bits offset for drive strength trim value */
#define CY_GPIO_SLEW_EXT_OFFSET                (2ul)      /**< Port pin bits offset for slew rate extension values */
#define CY_GPIO_DRIVE_TRIM_EXT_OFFSET          (3ul)      /**< Port pin bits offset for gpio drive strength for pin */

/* Bit offsets for register bits */
#define CY_GPIO_CFG_OUT_DRIVE_REG_OFFSET       (16ul)     /**< Register bits offset for drive strength function */
#define CY_GPIO_INTR_FILT_REG_OFFSET           (18ul)     /**< Register bits offset for filtered interrupt config function */

#define CY_GPIO_ZERO                           (0ul)      /**< Constant zero */

/** \endcond */


/***************************************
*        Function Constants
***************************************/

/**
* \addtogroup group_gpio_macro
* \{
*/

/**
* \defgroup group_gpio_driveModes Pin drive mode
* \{
* Constants to be used for setting the drive mode of the pin.
*/
#define CY_GPIO_DM_ANALOG                      (0x00ul) /**< \brief Analog High-Z. Input buffer off */
#define CY_GPIO_DM_PULLUP_IN_OFF               (0x02ul) /**< \brief Resistive Pull-Up. Input buffer off */
#define CY_GPIO_DM_PULLDOWN_IN_OFF             (0x03ul) /**< \brief Resistive Pull-Down. Input buffer off */
#define CY_GPIO_DM_OD_DRIVESLOW_IN_OFF         (0x04ul) /**< \brief Open Drain, Drives Low. Input buffer off */
#define CY_GPIO_DM_OD_DRIVESHIGH_IN_OFF        (0x05ul) /**< \brief Open Drain, Drives High. Input buffer off */
#define CY_GPIO_DM_STRONG_IN_OFF               (0x06ul) /**< \brief Strong Drive. Input buffer off */
#define CY_GPIO_DM_PULLUP_DOWN_IN_OFF          (0x07ul) /**< \brief Resistive Pull-Up/Down. Input buffer off */
#define CY_GPIO_DM_HIGHZ                       (0x08ul) /**< \brief Digital High-Z. Input buffer on */
#define CY_GPIO_DM_PULLUP                      (0x0Aul) /**< \brief Resistive Pull-Up. Input buffer on */
#define CY_GPIO_DM_PULLDOWN                    (0x0Bul) /**< \brief Resistive Pull-Down. Input buffer on */
#define CY_GPIO_DM_OD_DRIVESLOW                (0x0Cul) /**< \brief Open Drain, Drives Low. Input buffer on */
#define CY_GPIO_DM_OD_DRIVESHIGH               (0x0Dul) /**< \brief Open Drain, Drives High. Input buffer on */
#define CY_GPIO_DM_STRONG                      (0x0Eul) /**< \brief Strong Drive. Input buffer on */
#define CY_GPIO_DM_PULLUP_DOWN                 (0x0Ful) /**< \brief Resistive Pull-Up/Down. Input buffer on */
/** \} */

/**
* \defgroup group_gpio_vtrip Voltage trip mode
* \{
* Constants to be used for setting the voltage trip type on the pin.
*/
#define CY_GPIO_VTRIP_CMOS                     (0x00ul) /**< \brief Input buffer compatible with CMOS and I2C interfaces */
#define CY_GPIO_VTRIP_TTL                      (0x01ul) /**< \brief Input buffer compatible with TTL and MediaLB interfaces */
/** \} */

/**
* \defgroup group_gpio_vtrip_auto Voltage trip mode
* \{
* Constants to be used for setting the input buffer mode (trip points and hysteresis) for GPIO5V.
*/
#define CY_GPIO_VTRIP_DIS_AUTO                 (0x00ul) /**< \brief Input buffer is not compatible with automotive */
#define CY_GPIO_VTRIP_SEL_AUTO                 (0x01ul) /**< \brief Input buffer is compatible with automotive */
/** \} */

/**
* \defgroup group_gpio_slewRate Slew Rate Mode
* \{
* Constants to be used for setting the slew rate of the pin.
*/
#define CY_GPIO_SLEW_FAST                      (0x00ul) /**< \brief Fast slew rate */
#define CY_GPIO_SLEW_SLOW                      (0x01ul) /**< \brief Slow slew rate */
/** \} */

/**
* \defgroup group_gpio_driveStrength Pin drive strength
* \{
* Constants to be used for setting the drive strength of the pin.
*/
#define CY_GPIO_DRIVE_FULL                     (0x00ul) /**< \brief Full drive strength: Max drive current */
#define CY_GPIO_DRIVE_1_2                      (0x02ul) /**< \brief 1/2 drive strength: 1/2 drive current */
#define CY_GPIO_DRIVE_1_4                      (0x03ul) /**< \brief 1/4 drive strength: 1/4 drive current */
/** \} */

/**
* \defgroup group_gpio_driveStrength_trim Pin drive strength
* \{
* Constants to be used for setting the drive strength trim value of the pin.
*/
#define CY_GPIO_DRIVE_STRENGTH_DEFAULT         (0x00ul) /**< \brief Default drive strength: 50 ohm */
#define CY_GPIO_DRIVE_STRENGTH_120OHM          (0x01ul) /**< \brief Default drive strength: 120 ohm */
#define CY_GPIO_DRIVE_STRENGTH_90OHM           (0x02ul) /**< \brief Default drive strength: 90 ohm */
#define CY_GPIO_DRIVE_STRENGTH_60OHM           (0x03ul) /**< \brief Default drive strength: 60 ohm */
#define CY_GPIO_DRIVE_STRENGTH_50OHM           (0x04ul) /**< \brief Default drive strength: 50 ohm */
#define CY_GPIO_DRIVE_STRENGTH_30OHM           (0x05ul) /**< \brief Default drive strength: 30 ohm */
#define CY_GPIO_DRIVE_STRENGTH_20OHM           (0x06ul) /**< \brief Default drive strength: 20 ohm */
#define CY_GPIO_DRIVE_STRENGTH_15OHM           (0x07ul) /**< \brief Default drive strength: 15 ohm */
/** \} */

/**
* \defgroup group_gpio_slewRate_ext Slew Rate Mode
* \{
* Constants to be used for setting the slew rate of the pin.
*/
#define CY_GPIO_SLEW_EXT_FASTEST               (0x00ul) /**< \brief Fastest external slew rate */
#define CY_GPIO_SLEW_EXT_FAST                  (0x01ul) /**< \brief Fast external slew rate */
#define CY_GPIO_SLEW_EXT_SLOW                  (0x02ul) /**< \brief Slow external slew rate */
#define CY_GPIO_SLEW_EXT_SLOWEST               (0x03ul) /**< \brief Slowest external slew rate */
/** \} */

/**
* \defgroup group_gpio_driveStrength_ext Pin drive strength
* \{
* Constants to be used for setting the drive strength external for IO pin.
*/
#define CY_GPIO_CFG_DRIVE_EXT_1V8_4MA          (0x00ul) /**< \brief 4mA drive strength, 1.8V */
#define CY_GPIO_CFG_DRIVE_EXT_1V8_3MA          (0x01ul) /**< \brief 3mA drive strength, 1.8V */
#define CY_GPIO_CFG_DRIVE_EXT_1V8_2MA          (0x02ul) /**< \brief 2mA drive strength, 1.8V */
#define CY_GPIO_CFG_DRIVE_EXT_1V8_1MA          (0x03ul) /**< \brief 1mA drive strength, 1.8V */
#define CY_GPIO_CFG_DRIVE_EXT_1V2_4MA          (0x04ul) /**< \brief 4mA drive strength, 1.2V */
#define CY_GPIO_CFG_DRIVE_EXT_1V2_3MA          (0x05ul) /**< \brief 3mA drive strength, 1.2V */
#define CY_GPIO_CFG_DRIVE_EXT_1V2_2MA          (0x06ul) /**< \brief 2mA drive strength, 1.2V */
#define CY_GPIO_CFG_DRIVE_EXT_1V2_1MA          (0x07ul) /**< \brief 1mA drive strength, 1.2V */
/** \} */

/**
* \defgroup group_gpio_interruptTrigger Interrupt trigger type
* \{
* Constants to be used for setting the interrupt trigger type on the pin.
*/
#define CY_GPIO_INTR_DISABLE                   (0x00ul) /**< \brief Disable the pin interrupt generation */
#define CY_GPIO_INTR_RISING                    (0x01ul) /**< \brief Rising-Edge interrupt */
#define CY_GPIO_INTR_FALLING                   (0x02ul) /**< \brief Falling-Edge interrupt */
#define CY_GPIO_INTR_BOTH                      (0x03ul) /**< \brief Both-Edge interrupt */
/** \} */

/** \} group_gpio_macro */

/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_gpio_functions
* \{
*/

/**
* \addtogroup group_gpio_functions_init
* \{
*/

cy_en_gpio_status_t Cy_GPIO_Pin_Init(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, const cy_stc_gpio_pin_config_t *config);
cy_en_gpio_status_t Cy_GPIO_Multi_Pin_Init(const cy_stc_gpio_pin_prt_config_t *config, uint32_t pinAmount);
cy_en_gpio_status_t Cy_GPIO_Port_Init(volatile stc_GPIO_PRT_t* base, const cy_stc_gpio_prt_config_t *config);
__STATIC_INLINE void Cy_GPIO_Pin_FastInit(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t driveMode,
                                        uint32_t outVal, en_hsiom_sel_t hsiom);
__STATIC_INLINE void Cy_GPIO_Port_Deinit(volatile stc_GPIO_PRT_t* base);
__STATIC_INLINE void Cy_GPIO_SetHSIOM(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, en_hsiom_sel_t value);
__STATIC_INLINE en_hsiom_sel_t Cy_GPIO_GetHSIOM(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE stc_GPIO_PRT_t* Cy_GPIO_PortToAddr(uint32_t portNum);

/** \} group_gpio_functions_init */

/**
* \addtogroup group_gpio_functions_gpio
* \{
*/

__STATIC_INLINE uint32_t Cy_GPIO_Read(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_Write(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_ReadOut(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_Set(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_Clr(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_Inv(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetDrivemode(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetDrivemode(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetVtrip(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetVtrip(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void GPIO_SetVtripAuto(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t GPIO_GetVtripAuto(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetSlewRate(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetSlewRate(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetDriveSel(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSel(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);

#if (CY_IP_MXS40IOSS_VERSION >= 4ul)
__STATIC_INLINE void Cy_GPIO_SetDriveSelTrim(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSelTrim(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
#endif

#if (CY_IP_MXS40IOSS_VERSION >= 5ul)
__STATIC_INLINE void Cy_GPIO_SetSlewRateExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetSlewRateExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetDriveSelExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSelExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
#endif

/** \} group_gpio_functions_gpio */

/**
* \addtogroup group_gpio_functions_interrupt
* \{
*/

__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptStatus(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_ClearInterrupt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetInterruptMask(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptMask(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptStatusMasked(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetSwInterrupt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetInterruptEdge(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptEdge(volatile stc_GPIO_PRT_t* base, uint32_t pinNum);
__STATIC_INLINE void Cy_GPIO_SetFilter(volatile stc_GPIO_PRT_t* base, uint32_t value);
__STATIC_INLINE uint32_t Cy_GPIO_GetFilter(volatile stc_GPIO_PRT_t* base);

#if (IOSS_GPIO_GPIO_PORT_NR_0_31 != 0ul) || defined (CY_DOXYGEN)
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause0(void);
#endif /* (IOSS_GPIO_GPIO_PORT_NR_0_31 != 0ul) */

#if (IOSS_GPIO_GPIO_PORT_NR_32_63 != 0ul) || defined (CY_DOXYGEN)
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause1(void);
#endif /* (IOSS_GPIO_GPIO_PORT_NR_32_63 != 0ul) */

#if (IOSS_GPIO_GPIO_PORT_NR_64_95 != 0ul) || defined (CY_DOXYGEN)
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause2(void);
#endif /* (IOSS_GPIO_GPIO_PORT_NR_64_95 != 0ul) */

#if (IOSS_GPIO_GPIO_PORT_NR_96_127 != 0ul) || defined (CY_DOXYGEN)
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause3(void);
#endif /* (IOSS_GPIO_GPIO_PORT_NR_96_127 != 0ul) */

/** \} group_gpio_functions_interrupt */


/**
* \addtogroup group_gpio_functions_init
* \{
*/

/*******************************************************************************
* Function Name: Cy_GPIO_Pin_FastInit
****************************************************************************//**
*
* \brief Initialize the most common configuration settings for all pin types.
*
* These include, drive mode, initial output value, and HSIOM connection.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param driveMode
* Pin drive mode. Options are detailed in \ref group_gpio_driveModes macros
*
* \param outVal
* Logic state of the output buffer driven to the pin (1 or 0)
*
* \param hsiom
* HSIOM input selection
*
* \return
* void
*
* \note 
* This function modifies port registers in read-modify-write operations. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Pin_FastInit(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t driveMode,
                                        uint32_t outVal, en_hsiom_sel_t hsiom)
{
    Cy_GPIO_Write(base, pinNum, outVal);
    Cy_GPIO_SetHSIOM(base, pinNum, hsiom);
    Cy_GPIO_SetDrivemode(base, pinNum, driveMode);
}


/*******************************************************************************
* Function Name: Cy_GPIO_Port_Deinit
****************************************************************************//**
*
* \brief Reset a complete port of pins back to power on reset defaults.
*
* \param base
* Pointer to the pin's port register base address
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Port_Deinit(volatile stc_GPIO_PRT_t* base)
{
    uint32_t portNum;
    stc_HSIOM_PRT_t* portAddrHSIOM;

    portNum = ((uint32_t)(base) - GPIO_BASE) / GPIO_PRT_SECTION_SIZE;
    portAddrHSIOM = (stc_HSIOM_PRT_t*)(HSIOM_BASE + (HSIOM_PRT_SECTION_SIZE * portNum));

    base->unINTR_CFG.u32Register           = CY_GPIO_PRT_DEINIT;
    base->unINTR_MASK.u32Register          = CY_GPIO_PRT_DEINIT;
    base->unOUT.u32Register                = CY_GPIO_PRT_DEINIT;
    base->unCFG.u32Register                = CY_GPIO_PRT_DEINIT;
    base->unCFG_IN.u32Register             = CY_GPIO_PRT_DEINIT;
    base->unCFG_OUT.u32Register            = CY_GPIO_PRT_DEINIT;
    base->unCFG_IN_AUTOLVL.u32Register     = CY_GPIO_PRT_DEINIT;
    portAddrHSIOM->unPORT_SEL0.u32Register = CY_GPIO_PRT_DEINIT;
    portAddrHSIOM->unPORT_SEL1.u32Register = CY_GPIO_PRT_DEINIT;

}


/*******************************************************************************
* Function Name: Cy_GPIO_SetHSIOM
****************************************************************************//**
*
* \brief Configures the HSIOM connection to the pin.
*
* Connects the specified High-Speed Input Output Multiplexer (HSIOM) selection
* to the pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* HSIOM input selection
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetHSIOM(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, en_hsiom_sel_t value)
{
    uint8_t  pinNumForHsiom;
    uint32_t portNum;
    uint32_t tempReg;
    stc_HSIOM_PRT_t* portAddrHSIOM;

    portNum = ((uint32_t)(base) - GPIO_BASE) / GPIO_PRT_SECTION_SIZE;
    portAddrHSIOM = (stc_HSIOM_PRT_t*)(HSIOM_BASE + (HSIOM_PRT_SECTION_SIZE * portNum));

    if(pinNum < CY_GPIO_PRT_HALF)
    {
        pinNumForHsiom = pinNum;
        tempReg = portAddrHSIOM->unPORT_SEL0.u32Register & ~(CY_GPIO_HSIOM_MASK << (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET));
        portAddrHSIOM->unPORT_SEL0.u32Register = tempReg | ((value & CY_GPIO_HSIOM_MASK) << (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET));
    }
    else
    {
        pinNumForHsiom = pinNum - CY_GPIO_PRT_HALF;
        tempReg = portAddrHSIOM->unPORT_SEL1.u32Register & ~(CY_GPIO_HSIOM_MASK << (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET));
        portAddrHSIOM->unPORT_SEL1.u32Register = tempReg | ((value & CY_GPIO_HSIOM_MASK) << (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET));
    }

}


/*******************************************************************************
* Function Name: Cy_GPIO_GetHSIOM
****************************************************************************//**
*
* \brief Returns the current HSIOM multiplexer connection to the pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* HSIOM input selection
*
*******************************************************************************/
__STATIC_INLINE en_hsiom_sel_t Cy_GPIO_GetHSIOM(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    uint8_t  pinNumForHsiom;
    uint32_t returnValue;
    uint32_t portNum;
    stc_HSIOM_PRT_t* portAddrHSIOM;

    portNum = ((uint32_t)(base) - GPIO_BASE) / GPIO_PRT_SECTION_SIZE;
    portAddrHSIOM = (stc_HSIOM_PRT_t*)(HSIOM_BASE + (HSIOM_PRT_SECTION_SIZE * portNum));

    if(pinNum < CY_GPIO_PRT_HALF)
    {
        pinNumForHsiom = pinNum;
        returnValue = (portAddrHSIOM->unPORT_SEL0.u32Register >> (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET)) & CY_GPIO_HSIOM_MASK;
    }
    else
    {
        pinNumForHsiom = pinNum - CY_GPIO_PRT_HALF;
        returnValue = (portAddrHSIOM->unPORT_SEL1.u32Register >> (pinNumForHsiom << CY_GPIO_HSIOM_OFFSET)) & CY_GPIO_HSIOM_MASK;
    }

    return (en_hsiom_sel_t)returnValue;
}


/*******************************************************************************
* Function Name: Cy_GPIO_PortToAddr
****************************************************************************//**
*
* \brief Retrieves the port address based on the given port number.
*
* This is a helper function to calculate the port base address when given a port
* number. It is to be used when pin access needs to be calculated at runtime.
*
* \param portNum
* Port number
*
* \return
* Base address of the port register structure
*
*******************************************************************************/
__STATIC_INLINE stc_GPIO_PRT_t* Cy_GPIO_PortToAddr(uint32_t portNum)
{
    stc_GPIO_PRT_t* base;

    if(portNum < (uint32_t)IOSS_GPIO_GPIO_PORT_NR)
    {
         base = (stc_GPIO_PRT_t *)(GPIO_BASE + (GPIO_PRT_SECTION_SIZE * portNum));
    }
    else
    {
        /* Error: Return default base address */
        base = (stc_GPIO_PRT_t *)(GPIO_BASE);
    }

    return (base);
}

/** \} group_gpio_functions_init */

/**
* \addtogroup group_gpio_functions_gpio
* \{
*/

/*******************************************************************************
* Function Name: Cy_GPIO_Read
****************************************************************************//**
*
* \brief Reads the current logic level on the input buffer of the pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Logic level present on the pin
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_Read(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unIN.u32Register >> (pinNum)) & CY_GPIO_IN_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_Write
****************************************************************************//**
*
* \brief Write a logic 0 or logic 1 state to the output driver.
*
* This function should be used only for software driven pins. It does not have
* any effect on peripheral driven pins.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Logic level to drive out on the pin
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Write(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    /* Thread-safe: Directly access the pin registers instead of base->OUT */
    if(CY_GPIO_ZERO == value)
    {
        base->unOUT_CLR.u32Register = CY_GPIO_OUT_MASK << pinNum;
    }
    else
    {
        base->unOUT_SET.u32Register = CY_GPIO_OUT_MASK << pinNum;
    }
}


/*******************************************************************************
* Function Name: Cy_GPIO_ReadOut
****************************************************************************//**
*
* \brief Reads the current logic level on the pin output driver.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Logic level on the pin output driver
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_ReadOut(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unOUT.u32Register >> pinNum) & CY_GPIO_OUT_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_Set
****************************************************************************//**
*
* \brief Set a pin output to logic state high.
*
* This function should be used only for software driven pins. It does not have
* any effect on peripheral driven pins.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Set(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    base->unOUT_SET.u32Register = CY_GPIO_OUT_MASK << pinNum;
}


/*******************************************************************************
* Function Name: Cy_GPIO_Clr
****************************************************************************//**
*
* \brief Set a pin output to logic state Low.
*
* This function should be used only for software driven pins. It does not have
* any effect on peripheral driven pins.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Clr(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    base->unOUT_CLR.u32Register = CY_GPIO_OUT_MASK << pinNum;
}


/*******************************************************************************
* Function Name: Cy_GPIO_Inv
****************************************************************************//**
*
* \brief Set a pin output logic state to the inverse of the current output
* logic state.
*
* This function should be used only for software driven pins. It does not have
* any effect on peripheral driven pins.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_Inv(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    base->unOUT_INV.u32Register = CY_GPIO_OUT_MASK << pinNum;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetDrivemode
****************************************************************************//**
*
* \brief Configures the pin output buffer drive mode and input buffer enable.
*
* The output buffer drive mode and input buffer enable are combined into a single
* parameter. The drive mode controls the behavior of the pin in general.
* Enabling the input buffer allows the digital pin state to be read but also
* contributes to extra current consumption.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin drive mode. Options are detailed in \ref group_gpio_driveModes macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetDrivemode(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    pinLoc = pinNum << CY_GPIO_DRIVE_MODE_OFFSET;
    tempReg = (base->unCFG.u32Register & ~(CY_GPIO_CFG_DM_MASK << pinLoc));
    base->unCFG.u32Register = tempReg | ((value & CY_GPIO_CFG_DM_MASK) << pinLoc);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetDrivemode
****************************************************************************//**
*
* \brief Returns the pin output buffer drive mode and input buffer enable state.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin drive mode. Options are detailed in \ref group_gpio_driveModes macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetDrivemode(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG.u32Register >> (pinNum << CY_GPIO_DRIVE_MODE_OFFSET)) & CY_GPIO_CFG_DM_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetVtrip
****************************************************************************//**
*
* \brief Configures the GPIO pin input buffer voltage threshold mode.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin voltage threshold mode. Options are detailed in \ref group_gpio_vtrip macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetVtrip(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;

    tempReg = base->unCFG_IN.u32Register & ~(CY_GPIO_CFG_IN_VTRIP_SEL_MASK << pinNum);
    base->unCFG_IN.u32Register = tempReg | ((value & CY_GPIO_CFG_IN_VTRIP_SEL_MASK) << pinNum);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetVtrip
****************************************************************************//**
*
* \brief Returns the pin input buffer voltage threshold mode.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin voltage threshold mode. Options are detailed in \ref group_gpio_vtrip macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetVtrip(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_IN.u32Register >> pinNum) & CY_GPIO_CFG_IN_VTRIP_SEL_MASK;
}

/*******************************************************************************
* Function Name: GPIO_SetVtripAuto
****************************************************************************//**
*
* \brief Configures the GPIO pin input buffer for automotive compatible or not.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin voltage threshold mode. Options are detailed in \ref group_gpio_vtrip macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void GPIO_SetVtripAuto(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;

    tempReg = base->unCFG_IN_AUTOLVL.u32Register & ~(CY_GPIO_CFG_IN_VTRIP_SEL_MASK << pinNum);
    base->unCFG_IN_AUTOLVL.u32Register = tempReg | ((value & CY_GPIO_CFG_IN_VTRIP_SEL_MASK) << pinNum);
}

/*******************************************************************************
* Function Name: GPIO_GetVtripAuto
****************************************************************************//**
*
* \brief Returns the pin input buffer voltage whether it is automotive compatible or not.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin voltage for automotive or not. Options are detailed in \ref group_gpio_vtrip macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t GPIO_GetVtripAuto(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_IN_AUTOLVL.u32Register >> pinNum) & CY_GPIO_CFG_IN_VTRIP_SEL_MASK;
}

/*******************************************************************************
* Function Name: Cy_GPIO_SetSlewRate
****************************************************************************//**
*
* \brief Configures the pin output buffer slew rate.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin slew rate. Options are detailed in \ref group_gpio_slewRate macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetSlewRate(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;

    tempReg = base->unCFG_OUT.u32Register & ~(CY_GPIO_CFG_OUT_SLOW_MASK << pinNum);
    base->unCFG_OUT.u32Register = tempReg | ((value & CY_GPIO_CFG_OUT_SLOW_MASK) << pinNum);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetSlewRate
****************************************************************************//**
*
* \brief Returns the pin output buffer slew rate.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin slew rate. Options are detailed in \ref group_gpio_slewRate macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetSlewRate(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_OUT.u32Register >> pinNum) & CY_GPIO_CFG_OUT_SLOW_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetDriveSel
****************************************************************************//**
*
* \brief Configures the pin output buffer drive strength.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin drive strength. Options are detailed in \ref group_gpio_driveStrength macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetDriveSel(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    pinLoc = (uint32_t)(pinNum << CY_GPIO_CFG_OUT_DRIVE_OFFSET) + CY_GPIO_CFG_OUT_DRIVE_REG_OFFSET;
    tempReg = base->unCFG_OUT.u32Register & ~(CY_GPIO_CFG_OUT_DRIVE_SEL_MASK << pinLoc);
    base->unCFG_OUT.u32Register = tempReg | ((value & CY_GPIO_CFG_OUT_DRIVE_SEL_MASK) << pinLoc);
}

/*******************************************************************************
* Function Name: Cy_GPIO_GetDriveSel
****************************************************************************//**
*
* \brief Returns the pin output buffer drive strength.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin drive strength. Options are detailed in \ref group_gpio_driveStrength macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSel(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_OUT.u32Register >> ((pinNum << CY_GPIO_CFG_OUT_DRIVE_OFFSET) + CY_GPIO_CFG_OUT_DRIVE_REG_OFFSET)) & CY_GPIO_CFG_OUT_DRIVE_SEL_MASK;
}

#if (CY_IP_MXS40IOSS_VERSION >= 4u)

/*******************************************************************************
* Function Name: Cy_GPIO_SetDriveSelTrim
****************************************************************************//**
*
* \brief Configures the pin output buffer drive select trim.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin drive strength trim. Options are detailed in \ref group_gpio_driveStrength_trim macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetDriveSelTrim(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    pinLoc = (uint32_t)(pinNum * CY_GPIO_DRIVE_TRIM_OFFSET);
    tempReg = base->unCFG_OUT2.u32Register & ~(CY_GPIO_CFG_OUT2_DRIVE_SEL_TRIM_MASK << pinLoc);
    base->unCFG_OUT2.u32Register = tempReg | ((value & CY_GPIO_CFG_OUT2_DRIVE_SEL_TRIM_MASK) << pinLoc);
}

/*******************************************************************************
* Function Name: Cy_GPIO_GetDriveSelTrim
****************************************************************************//**
*
* \brief Returns the pin output buffer drive select trim.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin drive select trim. Options are detailed in \ref group_gpio_driveStrength_trim macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSelTrim(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_OUT2.u32Register >> (pinNum * CY_GPIO_DRIVE_TRIM_OFFSET)) & CY_GPIO_CFG_OUT2_DRIVE_SEL_TRIM_MASK;
}

#endif  /* (CY_IP_MXS40IOSS_VERSION >= 4u) */


#if (CY_IP_MXS40IOSS_VERSION >= 5u)

/*******************************************************************************
* Function Name: Cy_GPIO_SetSlewRateExt
****************************************************************************//**
*
* \brief Set the external slow slew rate for IO pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Pin number of the GPIO port
*
* \param value
* Pin slew rate. Options are detailed in \ref group_gpio_slewRate_ext macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetSlewRateExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    pinLoc = (uint32_t)(pinNum << CY_GPIO_SLEW_EXT_OFFSET);
    tempReg = base->unCFG_SLEW_EXT.u32Register & ~(CY_GPIO_CFG_SLEW_EXT_SEL_MASK << pinLoc);
    base->unCFG_SLEW_EXT.u32Register = tempReg | ((value & CY_GPIO_CFG_SLEW_EXT_SEL_MASK) << pinLoc);
}

/*******************************************************************************
* Function Name: Cy_GPIO_GetSlewRateExt
****************************************************************************//**
*
* \brief Returns the pin external buffer slew rate.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Pin number of the GPIO port
*
* \return
* Pin slew rate. Options are detailed in \ref group_gpio_slewRate_ext macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetSlewRateExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unCFG_SLEW_EXT.u32Register >> (pinNum << CY_GPIO_SLEW_EXT_OFFSET)) & CY_GPIO_CFG_SLEW_EXT_SEL_MASK;
}

/*******************************************************************************
* Function Name: Cy_GPIO_SetDriveSelExt
****************************************************************************//**
*
* \brief Configures the external pin output buffer drive strength.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Pin number of the GPIO port
*
* \param value
* Pin drive strength. Options are detailed in \ref group_gpio_driveStrength_ext macros
*
* \return
* void
*
* \note 
* When initializing IO's for bus (such as I2C), make sure the peripheral and HSIOM
* is properly configured before setting the IO drive mode to avoid producing glitches.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetDriveSelExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    if(pinNum < CY_GPIO_PRT_HALF)
    {
        pinLoc = (uint32_t)(pinNum << CY_GPIO_DRIVE_TRIM_EXT_OFFSET);
        tempReg = base->unCFG_DRIVE_EXT0.u32Register & ~(CY_GPIO_CFG_DRIVE_EXT_SEL_MASK << pinLoc);
        base->unCFG_DRIVE_EXT0.u32Register = tempReg | ((value & CY_GPIO_CFG_DRIVE_EXT_SEL_MASK) << pinLoc);
    }
    else
    {
        pinNum = pinNum - CY_GPIO_PRT_HALF;
        pinLoc = (uint32_t)(pinNum << CY_GPIO_DRIVE_TRIM_EXT_OFFSET);
        tempReg = base->unCFG_DRIVE_EXT1.u32Register & ~(CY_GPIO_CFG_DRIVE_EXT_SEL_MASK << pinLoc);
        base->unCFG_DRIVE_EXT1.u32Register = tempReg | ((value & CY_GPIO_CFG_DRIVE_EXT_SEL_MASK) << pinLoc);
    }
}

/*******************************************************************************
* Function Name: Cy_GPIO_GetDriveSelExt
****************************************************************************//**
*
* \brief Returns the external pin output buffer drive strength for GPIO pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Pin number of the GPIO port
*
* \return
* GPIO Pin drive strength. Options are detailed in \ref group_gpio_driveStrength_ext macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetDriveSelExt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    if(pinNum < CY_GPIO_PRT_HALF)
    {
        return (base->unCFG_DRIVE_EXT0.u32Register >> (pinNum << CY_GPIO_DRIVE_TRIM_EXT_OFFSET)) & CY_GPIO_CFG_DRIVE_EXT_SEL_MASK;
    }
    else
    {
        pinNum = pinNum - CY_GPIO_PRT_HALF;
        return (base->unCFG_DRIVE_EXT1.u32Register >> (pinNum << CY_GPIO_DRIVE_TRIM_EXT_OFFSET)) & CY_GPIO_CFG_DRIVE_EXT_SEL_MASK;
    }
}

#endif  /* (CY_IP_MXS40IOSS_VERSION >= 5u) */

/** \} group_gpio_functions_gpio */

/**
* \addtogroup group_gpio_functions_interrupt
* \{
*/

/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptStatus
****************************************************************************//**
*
* \brief Return the current interrupt state of the pin.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* 0 = Pin interrupt condition not detected
* 1 = Pin interrupt condition detected
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptStatus(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unINTR.u32Register >> pinNum) & CY_GPIO_INTR_STATUS_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_ClearInterrupt
****************************************************************************//**
*
* \brief Clears the triggered pin interrupt.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_ClearInterrupt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    /* Any INTR MMIO registers AHB clearing must be preceded with an AHB read access */
    (void)base->unINTR.u32Register;

    base->unINTR.u32Register = CY_GPIO_INTR_STATUS_MASK << pinNum;

    /* This read ensures that the initial write has been flushed out to the hardware */
    (void)base->unINTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetInterruptMask
****************************************************************************//**
*
* \brief Configures the pin interrupt to be forwarded to the CPU NVIC.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* 0 = Pin interrupt not forwarded to CPU interrupt controller
* 1 = Pin interrupt masked and forwarded to CPU interrupt controller
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetInterruptMask(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;

    tempReg= base->unINTR_MASK.u32Register & ~(CY_GPIO_INTR_EN_MASK << pinNum);
    base->unINTR_MASK.u32Register = tempReg | ((value & CY_GPIO_INTR_EN_MASK) << pinNum);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptMask
****************************************************************************//**
*
* \brief Returns the state of the pin interrupt mask to determine whether it is
* configured to be forwarded to the CPU interrupt controller.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* 0 = Pin interrupt not forwarded to CPU interrupt controller
* 1 = Pin interrupt masked and forwarded to CPU interrupt controller
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptMask(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unINTR_MASK.u32Register >> pinNum) & CY_GPIO_INTR_EN_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptStatusMasked
****************************************************************************//**
*
* \brief Return the pin's current interrupt state after being masked.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* 0 = Pin interrupt not detected or not forwarded to CPU interrupt controller
* 1 = Pin interrupt detected and forwarded to CPU interrupt controller
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptStatusMasked(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unINTR_MASKED.u32Register >> pinNum) & CY_GPIO_INTR_MASKED_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetSwInterrupt
****************************************************************************//**
*
* \brief Force a pin interrupt to trigger.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* void
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetSwInterrupt(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    base->unINTR_SET.u32Register = CY_GPIO_INTR_SET_MASK << pinNum;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetInterruptEdge
****************************************************************************//**
*
* \brief Configures the type of edge that will trigger a pin interrupt.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \param value
* Pin interrupt mode. Options are detailed in \ref group_gpio_interruptTrigger macros
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetInterruptEdge(volatile stc_GPIO_PRT_t* base, uint32_t pinNum, uint32_t value)
{
    uint32_t tempReg;
    uint32_t pinLoc;

    pinLoc = pinNum << CY_GPIO_INTR_CFG_OFFSET;
    tempReg = base->unINTR_CFG.u32Register & ~(CY_GPIO_INTR_EDGE_MASK << pinLoc);
    base->unINTR_CFG.u32Register = tempReg | ((value & CY_GPIO_INTR_EDGE_MASK) << pinLoc);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptEdge
****************************************************************************//**
*
* \brief Returns the current pin interrupt edge type.
*
* \param base
* Pointer to the pin's port register base address
*
* \param pinNum
* Position of the pin bit-field within the port register
*
* \return
* Pin interrupt mode. Options are detailed in \ref group_gpio_interruptTrigger macros
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptEdge(volatile stc_GPIO_PRT_t* base, uint32_t pinNum)
{
    return (base->unINTR_CFG.u32Register >> (pinNum << CY_GPIO_INTR_CFG_OFFSET)) & CY_GPIO_INTR_EDGE_MASK;
}


/*******************************************************************************
* Function Name: Cy_GPIO_SetFilter
****************************************************************************//**
*
* \brief Configures which pin on the port connects to the port filter.
*
* \param base
* Pointer to the pin's port register base address
*
* \param value
* The number of the port pin to route to the port filter (0...7)
*
* \return
* void
*
* \note 
* This function modifies a port register in a read-modify-write operation. It is
* not thread safe as the resource is shared among multiple pins on a port.
*
*******************************************************************************/
__STATIC_INLINE void Cy_GPIO_SetFilter(volatile stc_GPIO_PRT_t* base, uint32_t value)
{
    uint32_t tempReg;

    tempReg = base->unINTR_CFG.u32Register & ~(CY_GPIO_INTR_FLT_EDGE_MASK << CY_GPIO_INTR_FILT_REG_OFFSET);
    base->unINTR_CFG.u32Register = tempReg | ((value & CY_GPIO_INTR_FLT_EDGE_MASK) << CY_GPIO_INTR_FILT_REG_OFFSET);
}


/*******************************************************************************
* Function Name: Cy_GPIO_GetFilter
****************************************************************************//**
*
* \brief Returns which pin is currently configured to connect to the port filter.
*
* \param base
* Pointer to the pin's port register base address
*
* \return
* The number of the port pin routed to the port filter (0...7)
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetFilter(volatile stc_GPIO_PRT_t* base)
{
    return (base->unINTR_CFG.u32Register >> CY_GPIO_INTR_FILT_REG_OFFSET) & CY_GPIO_INTR_FLT_EDGE_MASK;
}


#if (IOSS_GPIO_GPIO_PORT_NR_0_31 != 0ul) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptCause0
****************************************************************************//**
*
* \brief Returns the interrupt status for ports 0 to 31.
*
* \return
* 0 = Interrupt not detected on port
* 1 = Interrupt detected and sent to CPU interrupt controller on port
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause0(void)
{
    return GPIO->unINTR_CAUSE0.u32Register;
}

#endif

#if (IOSS_GPIO_GPIO_PORT_NR_32_63 != 0ul) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptCause1
****************************************************************************//**
*
* \brief Returns the interrupt status for ports 32 to 63.
*
* \return
* 0 = Interrupt not detected on port
* 1 = Interrupt detected and sent to CPU interrupt controller on port
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause1(void)
{
    return GPIO->unINTR_CAUSE1.u32Register;
}

#endif

#if (IOSS_GPIO_GPIO_PORT_NR_64_95 != 0ul) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptCause2
****************************************************************************//**
*
* \brief Returns the interrupt status for ports 64 to 95.
*
* \return
* 0 = Interrupt not detected on port
* 1 = Interrupt detected and sent to CPU interrupt controller on port
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause2(void)
{       
    return GPIO->unINTR_CAUSE2.u32Register;
}

#endif

#if (IOSS_GPIO_GPIO_PORT_NR_96_127 != 0ul) || defined (CY_DOXYGEN)

/*******************************************************************************
* Function Name: Cy_GPIO_GetInterruptCause3
****************************************************************************//**
*
* \brief Returns the interrupt status for ports 96 to 127.
*
* \return
* 0 = Interrupt not detected on port
* 1 = Interrupt detected and sent to CPU interrupt controller on port
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_GPIO_GetInterruptCause3(void)
{
    return GPIO->unINTR_CAUSE3.u32Register;
}

#endif

/** \} group_gpio_functions_interrupt */

/** \} group_gpio_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_GPIO_H */

/** \} group_gpio */

/* [] END OF FILE */
