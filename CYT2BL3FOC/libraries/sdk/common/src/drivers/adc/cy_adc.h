/***************************************************************************//**
* \file cy_adc.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Adc driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_adc Analog to Digital Converter (ADC)
* \{
* The ADC driver provides an API to configure the ADC.
* The ADC has upto 4 ADC and upto 32 channel for each ADC module. About actual
* supported # or modules and channels, please refer to the technical reference
* manual (TRM) and device datasheet.
*
* \section group_adc_configuration Configuration Considerations
* Typical usage as is follows:
*  - Call Cy_Adc_Init to initialize ADC module
*  - Call Cy_Adc_Channel_Init to initialize a channel
*  - Call Cy_Adc_Channel_SetInterruptMask if you need to use interrupt.
* After initialize channel(s) call Cy_Adc_Channel_Enable function then
* trigger by software (calling Cy_Adc_Channel_SoftwareTrigger) or start
* a peripheral that is configured for HW trigger.
* Note that if you use interrupt for ADC, you have to make interrupt handler
* and register it by using sysint module. In the handler, you can check
* which interrupt is occurred by Cy_Adc_Channel_GetInterruptMaskedStatus
* and can get conversion result by Cy_Adc_Channel_GetResult.
* If you don't use interrupt, you can poll conversion status by
* Cy_Adc_Channel_GetInterruptStatus.
* For both of polling and interrupt, clear interrupt flags by
* Cy_Adc_Channel_ClearInterruptStatus after handling data.
*
* \section group_adc_more_information More Information
*
* Refer to the technical reference manual (TRM) and the device datasheet.
*
* \section group_adc_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_adc_macro Macro
* \defgroup group_adc_interrupt_sources Interrupt Sources
* \defgroup group_adc_ch_status Channel Status
* \defgroup group_adc_grp_status Group Status
* \defgroup group_adc_functions Functions
* \defgroup group_adc_data_structures Data structures
* \defgroup group_adc_enums Enumerated Types
* \defgroup group_adc_globals Global variables
*/


#if !defined(CY_ADC_H)
#define CY_ADC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"
#include "cy_device_headers.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*       Macro definitions
***************************************/

/**
* \addtogroup group_adc_macro
* \{
*/

/** Driver major version */
#define CY_ADC_DRV_VERSION_MAJOR    1

/** Driver minor version */
#define CY_ADC_DRV_VERSION_MINOR    0

/** Adc driver ID */
#define CY_ADC_ID CY_PDL_DRV_ID(0x36u)

/** \} group_adc_macro */

/***************************************
*       Enumeration
***************************************/

/**
* \addtogroup group_adc_enums
* \{
*/

/**
* Adc Driver error codes
*/
typedef enum {
    CY_ADC_SUCCESS   = 0x00u,                                   /**< Returned successful */
    CY_ADC_BAD_PARAM = CY_ADC_ID | CY_PDL_STATUS_ERROR | 0x01u, /**< Bad parameter was passed */
} cy_en_adc_status_t;

/** When set use 2 cycles for the Most Significant Bit (MSB) */
typedef enum {
    CY_ADC_MSB_STRETCH_MODE_1CYCLE = 0u, /**< Use 1 clock cycles per conversion. */
    CY_ADC_MSB_STRETCH_MODE_2CYCLE = 1u  /**< Use 2 clock cycles per conversion. */
} cy_en_adc_msb_stretch_mode_t;

/** Select Diagnostic Reference function. */
typedef enum {
    CY_ADC_DIAG_REFERENCE_SELECT_VREFL       = 0u,  /**< DiagOut = VrefL */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_1DIV8 = 1u,  /**< DiagOut = VrefH * 1/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_2DIV8 = 2u,  /**< DiagOut = VrefH * 2/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_3DIV8 = 3u,  /**< DiagOut = VrefH * 3/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_4DIV8 = 4u,  /**< DiagOut = VrefH * 4/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_5DIV8 = 5u,  /**< DiagOut = VrefH * 5/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_6DIV8 = 6u,  /**< DiagOut = VrefH * 6/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH_7DIV8 = 7u,  /**< DiagOut = VrefH * 7/8 */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFH       = 8u,  /**< DiagOut = VrefH */
    CY_ADC_DIAG_REFERENCE_SELECT_VREFX       = 9u,  /**< DiagOut = VrefX = VrefH * 199/200 */
    CY_ADC_DIAG_REFERENCE_SELECT_VBG         = 10u, /**< DiagOut = Vbg from SRSS */
    CY_ADC_DIAG_REFERENCE_SELECT_VIN1        = 11u, /**< DiagOut = Vin1 */
    CY_ADC_DIAG_REFERENCE_SELECT_VIN2        = 12u, /**< DiagOut = Vin2 */
    CY_ADC_DIAG_REFERENCE_SELECT_VIN3        = 13u, /**< DiagOut = Vin3 */
    CY_ADC_DIAG_REFERENCE_SELECT_I_SOURCE    = 14u, /**< DiagOut = Isource (10uA) */
    CY_ADC_DIAG_REFERENCE_SELECT_I_SINK      = 15u  /**< DiagOut = Isink (10uA) */
} cy_en_adc_diag_reference_select_t;

/** The ADC trigger mode selection. */
typedef enum {
    CY_ADC_TRIGGER_OFF        = 0u, /**< Use for channels in group, except the first channel */
    CY_ADC_TRIGGER_TCPWM      = 1u, /**< Trigger from corresponding TCPWM channel */
    CY_ADC_TRIGGER_GENERIC0   = 2u, /**< Generic trigger input 0 */
    CY_ADC_TRIGGER_GENERIC1   = 3u, /**< Generic trigger input 1 */
    CY_ADC_TRIGGER_GENERIC2   = 4u, /**< Generic trigger input 2 */
    CY_ADC_TRIGGER_GENERIC3   = 5u, /**< Generic trigger input 3 */
    CY_ADC_TRIGGER_GENERIC4   = 6u, /**< Generic trigger input 4 */
    CY_ADC_TRIGGER_CONTINUOUS = 7u  /**< Always triggered (also called idle), can only be used for at most 1 channel */
} cy_en_adc_trigger_selection_t;

/** The ADC preemption type selection. */
typedef enum {
    CY_ADC_PREEMPTION_ABORT_CANCEL  = 0u, /**< Abort ongoing acquisition, do not return. */
    CY_ADC_PREEMPTION_ABORT_RESTART = 1u, /**< Abort ongoing acquisition, up on return Restart group from first channel. */
    CY_ADC_PREEMPTION_ABORT_RESUME  = 2u, /**< Abort ongoing acquisition, up on return Resume group from aborted channel. */
    CY_ADC_PREEMPTION_FINISH_RESUME = 3u  /**< Complete ongoing acquisition (including averaging), up on return Resume group from next channel. */
} cy_en_adc_preemption_type_t;

/** Conversion done trigger output selection. */
typedef enum {
    CY_ADC_DONE_LEVEL_PULSE = 0u,   /**< Conversion done signal for other IPs is a 2 cycle pulse (clk_sys), no need to read the result regisgter.
                                         Typically used for triggering another IPs other than DW. */
    CY_ADC_DONE_LEVEL_LEVEL = 1u    /**< Conversion done signal for other IPs is level output until result register is read.
                                         Typically used for DW and also overflow detection. */
} cy_en_adc_done_level_t;

/** Address of the analog signal (pin) to be sampled by corresponding channel. */
typedef enum {
    CY_ADC_PIN_ADDRESS_AN0       = 0u,  /**<  Vout = AN0, select corresponding analog input. */
    CY_ADC_PIN_ADDRESS_AN1       = 1u,
    CY_ADC_PIN_ADDRESS_AN2       = 2u,
    CY_ADC_PIN_ADDRESS_AN3       = 3u,
    CY_ADC_PIN_ADDRESS_AN4       = 4u,
    CY_ADC_PIN_ADDRESS_AN5       = 5u,
    CY_ADC_PIN_ADDRESS_AN6       = 6u,
    CY_ADC_PIN_ADDRESS_AN7       = 7u,
    CY_ADC_PIN_ADDRESS_AN8       = 8u,
    CY_ADC_PIN_ADDRESS_AN9       = 9u,
    CY_ADC_PIN_ADDRESS_AN10      = 10u,
    CY_ADC_PIN_ADDRESS_AN11      = 11u,
    CY_ADC_PIN_ADDRESS_AN12      = 12u,
    CY_ADC_PIN_ADDRESS_AN13      = 13u,
    CY_ADC_PIN_ADDRESS_AN14      = 14u,
    CY_ADC_PIN_ADDRESS_AN15      = 15u,
    CY_ADC_PIN_ADDRESS_AN16      = 16u,
    CY_ADC_PIN_ADDRESS_AN17      = 17u,
    CY_ADC_PIN_ADDRESS_AN18      = 18u,
    CY_ADC_PIN_ADDRESS_AN19      = 19u,
    CY_ADC_PIN_ADDRESS_AN20      = 20u,
    CY_ADC_PIN_ADDRESS_AN21      = 21u,
    CY_ADC_PIN_ADDRESS_AN22      = 22u,
    CY_ADC_PIN_ADDRESS_AN23      = 23u,
    CY_ADC_PIN_ADDRESS_AN24      = 24u,
    CY_ADC_PIN_ADDRESS_AN25      = 25u,
    CY_ADC_PIN_ADDRESS_AN26      = 26u,
    CY_ADC_PIN_ADDRESS_AN27      = 27u,
    CY_ADC_PIN_ADDRESS_AN28      = 28u,
    CY_ADC_PIN_ADDRESS_AN29      = 29u,
    CY_ADC_PIN_ADDRESS_AN30      = 30u,
    CY_ADC_PIN_ADDRESS_AN31      = 31u,
    CY_ADC_PIN_ADDRESS_VMOTOR    = 32u, /**< Vout = Vmotor, select motor input. */
    CY_ADC_PIN_ADDRESS_VAUX      = 33u, /**< Vout = Vaux, select auxiliarly input. */
    CY_ADC_PIN_ADDRESS_AMUXBUS_A = 34u, /**< Vout = AmuxbusA. */
    CY_ADC_PIN_ADDRESS_AMUXBUS_B = 35u, /**< Vout = AmuxbusB. */
    CY_ADC_PIN_ADDRESS_VCCD      = 36u, /**< Vout = Vccd. */
    CY_ADC_PIN_ADDRESS_VDDA      = 37u, /**< Vout = Vdda. */
    CY_ADC_PIN_ADDRESS_VBG       = 38u, /**< Vout = Vbg, Bandgap voltage from SRSS. */
    CY_ADC_PIN_ADDRESS_VTEMP     = 39u, /**< Vout = Vtemp, select temperature sensor. Make sure that only 1 ADC is allowed to use this.. */
    CY_ADC_PIN_ADDRESS_VREF_L    = 62u, /**< Vout = VrefL (VrefL actually bypasses the SARMUX (XSL)). */
    CY_ADC_PIN_ADDRESS_VREF_H    = 63u  /**< Vout = VrefH  (VrefH actually bypasses the SARMUX (XSH)) */
} cy_en_adc_pin_address_t;

/** Select the physical port. This field is only valid for ADC0. */
typedef enum {
    CY_ADC_PORT_ADDRESS_SARMUX0 = 0u, /**< ADC uses it's own SARMUX. */
    CY_ADC_PORT_ADDRESS_SARMUX1 = 1u, /**< ADC0 uses SARMUX1 (only valid for ADC0, undefined result if used for ADC1-3). */
    CY_ADC_PORT_ADDRESS_SARMUX2 = 2u, /**< ADC0 uses SARMUX2 (only valid for ADC0, undefined result if used for ADC1-3). */
    CY_ADC_PORT_ADDRESS_SARMUX3 = 3u  /**< ADC0 uses SARMUX3 (only valid for ADC0, undefined result if used for ADC1-3). */
} cy_en_adc_port_address_t;

/** Select preconditioning mode.
    Preconditioning (dis)charges the SAR sample capacitor to the selected reference voltage for precondition time (global) cycles,
    a break before make cycle will be inserted before sampling starts sample time.
    See also \ref cy_stc_adc_config_t */
typedef enum {
    CY_ADC_PRECONDITION_MODE_OFF    = 0u,   /**< No preconditioning. */
    CY_ADC_PRECONDITION_MODE_VREFL  = 1u,   /**< Discharge to VREFL. */
    CY_ADC_PRECONDITION_MODE_VREFH  = 2u,   /**< Charge to VREFH. */
    CY_ADC_PRECONDITION_MODE_DIAG   = 3u    /**< Connect the Diagnostic reference output during preconditioning.
                                                 The Diagnostic reference should be configured to output a reference voltage. */
} cy_en_adc_precondition_mode_t;

/** Select Overlap mode or SARMUX Diagnostics, in both cases the Diagnostic reference is used. */
typedef enum {
    CY_ADC_OVERLAP_DIAG_MODE_OFF        = 0u,   /**< No overlap or SARMUX Diagnostics. */
    CY_ADC_OVERLAP_DIAG_MODE_HALF       = 1u,   /**< Sample the selected analog input for 2 sample time periods.
                                                     During the first period use overlap sampling,
                                                     i.e. connect both the analog input and Diagnostic reference.
                                                     During second period only connect the analog input */
    CY_ADC_OVERLAP_DIAG_MODE_FULL       = 2u,   /**< Like normal sample the selected analog input for a single sample time period
                                                     but use overlap sampling, i.e. connect both the analog input and Diagnostic reference. */
    CY_ADC_OVERLAP_DIAG_MODE_MUX_DIAG   = 3u    /**< Select Diagnostic reference instead of analog signal at the input of the SARMUX.
                                                     This enables a functional safety check of the SARMUX analog connections. */
} cy_en_adc_overlap_diag_mode_t;

/** Select calibration values. */
typedef enum {
    CY_ADC_CALIBRATION_VALUE_REGULAR    = 0u,   /**< use regular calibration values */
    CY_ADC_CALIBRATION_VALUE_ALTERNATE  = 1u    /**< use alternate calibration values */
} cy_en_adc_calibration_value_select_t;


/** Select post processing mode. */
typedef enum {
    CY_ADC_POST_PROCESSING_MODE_NONE        = 0u,  /**< No postprocessing. */
    CY_ADC_POST_PROCESSING_MODE_AVG         = 1u,  /**< Averaging. */
    CY_ADC_POST_PROCESSING_MODE_AVG_RANGE   = 2u,  /**< Averaging followed by Range detect. */
    CY_ADC_POST_PROCESSING_MODE_RANGE       = 3u,  /**< Range detect. */
    CY_ADC_POST_PROCESSING_MODE_RANGE_PULSE = 4u,  /**< Range detect followed by pulse detect. */
} cy_en_adc_post_processing_mode_t;

/** Select result data alignment. */
typedef enum {
    CY_ADC_RESULT_ALIGNMENT_RIGHT           = 0u, /**< the data is right aligned in result[11:0], with sign extension to 16 bits if enabled. */
    CY_ADC_RESULT_ALIGNMENT_LEFT            = 1u, /**< the data is left aligned in result[15:4] with the lower nibble 0.
                                                       Caveat if the result was more than 12 bits (e.g. after averaging)
                                                       then the bits above 12 will be discarded. */
} cy_en_adc_result_alignment_t;

/** Select whether result data is signed or unsigned. */
typedef enum {
    CY_ADC_SIGN_EXTENTION_UNSIGNED          = 0u, /**< Result data is unsigned (zero extended if needed). */
    CY_ADC_SIGN_EXTENTION_SIGNED            = 1u, /**< Result data is signed (sign extended if needed). */
} cy_en_adc_sign_extention_t;

/** Select range detection mode. */
typedef enum {
    CY_ADC_RANGE_DETECTION_MODE_BELOW_LO        = 0u,   /**< Below Low threshold (result < Lo) */
    CY_ADC_RANGE_DETECTION_MODE_INSIDE_RANGE    = 1u,   /**< Inside range (Lo <= result < Hi) */
    CY_ADC_RANGE_DETECTION_MODE_ABOVE_HI        = 2u,   /**< Above high threshold (Hi <= result) */
    CY_ADC_RANGE_DETECTION_MODE_OUTSIDE_RANGE   = 3u    /**< Outside range (result < Lo || Hi <= result) */
} cy_en_adc_range_detection_mode_t;

/** Select reference buffer mode. */
typedef enum {
    CY_ADC_REF_BUF_MODE_OFF     = 0u,   /**< No reference mode selected */
    CY_ADC_REF_BUF_MODE_ON      = 1u,   /**< Reference buffered Vbg from SRSS */
    CY_ADC_REF_BUF_MODE_BYPASS  = 3u,   /**< Reference unbuffered Vbg from SRSS */
} cy_en_adc_ref_buf_mode_t;

/** \} group_adc_enums */

/***************************************
*       Configuration Structure
***************************************/

/** Channel conversion status */
typedef struct
{
    bool aboveHi;       /**< If true out of range was detected and the value was above the Hi threshold */
    bool rangeIntr;     /**< If true range detected. */
    bool pulseIntr;     /**< If true pulse detected. */
    bool valid;         /**< If true result data is valid. */
} cy_stc_adc_ch_status_t;

/** Interrupt sources */
typedef struct
{
    bool grpDone;       /**< (Group) Done interrupt */
    bool grpCancelled;  /**< (Group) Cancelled Interrupt */
    bool grpOverflow;   /**< (Group) Overflow Interrupt */
    bool chRange;       /**< Range detect Interrupt */
    bool chPulse;       /**< Pulse detect Interrupt */
    bool chOverflow;    /**< Channel overflow Interrupt */
} cy_stc_adc_interrupt_source_t;

/** Group sources */
typedef struct
{
    bool grpComplete;       /**< Group acquisition complete. */
    bool grpCancelled;      /**< Group Cancelled. */
    bool grpOverflow;       /**< Group Overflow. */
    bool chRangeComplete;   /**< Channel Range complete. */
    bool chPulseComplete;   /**< Channel Pulse complete. */
    bool chOverflow;        /**< Channel Overflow.  */
    bool grpBusy;           /**< Group acquisition busy. */
} cy_stc_adc_group_status_t;

/**
* \addtogroup group_adc_data_structures
* \{
*/

/** Configuration structure of a ADC */
typedef struct {
    uint8_t                      preconditionTime;    /**< Number ADC clock cycles that Preconditioning is done before the sample window starts. */
    uint8_t                      powerupTime;         /**< Number cycles to wait to power up after IDLE_PWRDWN. */
    bool                         enableIdlePowerDown; /**< When idle automatically power down the analog if true. */
    cy_en_adc_msb_stretch_mode_t msbStretchMode;      /**< When set use 2 cycles for the Most Significant Bit (MSB), see \ref cy_en_adc_msb_stretch_mode_t */
    bool                         enableHalfLsbConv;   /**< When true take an extra cycle to convert the half LSB and add it to 12-bit result for Missing Code Recovery */
    bool                         sarMuxEnable;        /**< Enable the SARMUX (only valid if sarIpEnable = true). */
    bool                         adcEnable;           /**< Enable the SAR ADC and SAR sequencer (only valid if sarIpEnable = true). */
    bool                         sarIpEnable;         /**< Enable the SAR IP. */
} cy_stc_adc_config_t;

/** Configuration structure of a ADC channel */
typedef struct {
    cy_en_adc_trigger_selection_t       triggerSelection;   /**< The ADC trigger mode selection, see \ref cy_en_adc_trigger_selection_t */
    uint8_t                             channelPriority;    /**< Channel priority. 0=highest, 7=lowest. Valid range = [0..7].*/
    cy_en_adc_preemption_type_t         preenptionType;     /**< The ADC preemption type selection, see \ref cy_en_adc_preemption_type_t */
    bool                                isGroupEnd;         /**< This value indicates that this channel is the last channel of a group or not */
    cy_en_adc_done_level_t              doneLevel;          /**< Conversion done trigger output selection, see \ref cy_en_adc_done_level_t */
    cy_en_adc_pin_address_t             pinAddress;         /**< Address of the analog signal (pin) to be sampled by corresponding channel,
                                                              *  see \ref cy_en_adc_pin_address_t
                                                              */
    cy_en_adc_port_address_t            portAddress;        /**< Select the physical port, see \ref cy_en_adc_port_address_t */
    uint8_t                             extMuxSelect;       /**< External analog mux select. */
    bool                                extMuxEnable;       /**< External analog mux enable. */
    cy_en_adc_precondition_mode_t       preconditionMode;   /**< Select preconditioning mode, see \ref cy_en_adc_precondition_mode_t */
    cy_en_adc_overlap_diag_mode_t       overlapDiagMode;    /**< Select Overlap mode or SARMUX Diagnostics, see \ref cy_en_adc_overlap_diag_mode_t */
    uint16_t                            sampleTime;         /**< Sample time (aperture) in ADC clock cycles. Minimum is 1 (0 gives the same result as 1),
                                                              *  minimum time needed for proper settling is at least 300ns,
                                                              *  i.e. 6 clock cycles at the max frequency of 20MHz.
                                                              */
    cy_en_adc_calibration_value_select_t calibrationValueSelect;    /**< Select calibration values, see \ref cy_en_adc_calibration_value_select_t */
    cy_en_adc_post_processing_mode_t    postProcessingMode; /**< Select post processing mode, see \ref cy_en_adc_post_processing_mode_t */
    cy_en_adc_result_alignment_t        resultAlignment;    /**< Select result data alignment, see \ref cy_en_adc_result_alignment_t */
    cy_en_adc_sign_extention_t          signExtention;      /**< Select whether result data is signed or unsigned, see \ref cy_en_adc_sign_extention_t */
    uint8_t                             averageCount;       /**< Either averaging count (minus 1) or Pulse positive reload value. */
    uint8_t                             rightShift;         /**< Either Shift Right (no pulse detection) or Pulse nagative reload value (if pulse detection is enabled).
                                                              *  When using other than pulse detection, setting value is used for right shift value of the 
                                                              *  conversion result. Typically it is used for the averaging. Averaging data may be over 12bits,
                                                              *  therefore user must ensure conversion result not to over 12bits by using this value.
                                                              *  This value is also used to fit the 12-bit result in 8bits.
                                                              *  When using other than pulse detection, valid range is [0..12].
                                                              *  In pulse detection mode this value is used as negative reload value.
                                                              */
    cy_en_adc_range_detection_mode_t    rangeDetectionMode; /**< Select range detection mode, see \ref cy_en_adc_range_detection_mode_t */
    uint16_t                            rangeDetectionLoThreshold;  /**< Range detect low threshold (Lo) */
    uint16_t                            rangeDetectionHiThreshold;  /**< Range detect high threshold (Hi) */
    cy_stc_adc_interrupt_source_t       mask;               /**< Select the interrupt sources. \ref cy_stc_adc_interrupt_source_t */
} cy_stc_adc_channel_config_t;

/** Digital calibration values. */
typedef struct {
    uint16_t    offset; /**< Digital offset correction. Valid range is [0..4096] */
    uint8_t     gain;   /**< Digital gain correction. Valid range is [0..63] */
} cy_stc_adc_digital_calibration_conifg_t;

/** Analog calibration values. */
typedef struct {
    int8_t     offset; /**< Analog offset correction. Valid range is [0..255] */
    int8_t     gain;   /**< Analog gain correction. Valid range is [0..31] */
} cy_stc_adc_analog_calibration_conifg_t;

/** Configuration structure of diagnosis function. */
typedef struct {
    cy_en_adc_diag_reference_select_t   referenceSelect;    /**< Select Diagnostic Reference function, see \ref cy_en_adc_diag_reference_select_t */
} cy_stc_adc_diag_config_t;

/** Control freeze feature for debugging. */
typedef struct {
    bool      enableFreezeAdc0; /**< If true, freeze ADC0 in debug mode. */
    bool      enableFreezeAdc1; /**< If true, freeze ADC1 in debug mode. */
    bool      enableFreezeAdc2; /**< If true, freeze ADC2 in debug mode. */
    bool      enableFreezeAdc3; /**< If true, freeze ADC3 in debug mode. */
} cy_stc_adc_debug_freeze_config_t;

/** ADC temp sensor data values structure read form SFLASH */
typedef struct {
    double      hotValue;   /**< Pass the hot temperature reading */
    double      roomValue;  /**< Pass the room temperature reading */
    double      coldValue;  /**< Pass the cold temperature reading */
} cy_en_adc_temp_sflash_data_t;

/** ADC ref data values structure read form SFLASH */
typedef struct {
    cy_en_adc_temp_sflash_data_t adcVbgValues;    /**< Pass the Vbg read value from SFLASH */
    cy_en_adc_temp_sflash_data_t adcTempValues;  /**< Pass the sort data values from SFLASH */
    cy_en_adc_temp_sflash_data_t adcDiodeValues; /**< Pass the class data values from SFLASH */
} cy_stc_adc_temp_ref_t;

/** ADC raw data values structure of a temperature calculation */
typedef struct {
    uint16_t    adcVtempRawValue;       /**< Enable the SAR Channel VTEMP, and update the value */
    uint16_t    adcVbgRawValue;         /**< Enable the SAR Channel VBG, and update the value */
} cy_stc_adc_temp_raw_t;


/** \} group_adc_data_structures */

/***************************************
*       Global Variable
***************************************/
/* N/A */

/***************************************
*              Constants
***************************************/

/** \cond INTERNAL */

/** \endcond */


/***************************************
*       Function Prototypes
***************************************/
/**
* \addtogroup group_adc_functions
* \{
*/

/* For each ADC */
cy_en_adc_status_t Cy_Adc_Init(volatile stc_PASS_SAR_t * base, const cy_stc_adc_config_t * config);
void Cy_Adc_DeInit(volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetPendingStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetWorkValidStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetWorkRangeStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetWorkRangeHiStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetWorkPulseStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetResultValidStatus(const volatile stc_PASS_SAR_t * base);
uint32_t Cy_Adc_GetResultRangeHiStatus(const volatile stc_PASS_SAR_t * base);

/* For each channels */
cy_en_adc_status_t Cy_Adc_Channel_Init(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_channel_config_t * config);
void Cy_Adc_Channel_DeInit(volatile stc_PASS_SAR_CH_t * base);
void Cy_Adc_Channel_Enable(volatile stc_PASS_SAR_CH_t * base);
void Cy_Adc_Channel_Disable(volatile stc_PASS_SAR_CH_t * base);
void Cy_Adc_Channel_SoftwareTrigger(volatile stc_PASS_SAR_CH_t * base);
cy_en_adc_status_t Cy_Adc_Channel_GetResult(const volatile stc_PASS_SAR_CH_t * base, uint16_t * result, cy_stc_adc_ch_status_t * status);
cy_en_adc_status_t Cy_Adc_Channel_GetWorkingData(const volatile stc_PASS_SAR_CH_t * base, uint16_t * result, cy_stc_adc_ch_status_t * status);
cy_en_adc_status_t Cy_Adc_Channel_GetGroupStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_group_status_t * status);
cy_en_adc_status_t Cy_Adc_Channel_SetInterruptMask(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_interrupt_source_t * mask);
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptMask(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * mask);
cy_en_adc_status_t Cy_Adc_Channel_ClearInterruptStatus(volatile stc_PASS_SAR_CH_t * base, const cy_stc_adc_interrupt_source_t * source);
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * status);
cy_en_adc_status_t Cy_Adc_Channel_GetInterruptMaskedStatus(const volatile stc_PASS_SAR_CH_t * base, cy_stc_adc_interrupt_source_t * status);

/* For diagnosis function */
cy_en_adc_status_t Cy_Adc_Diag_Init(volatile stc_PASS_SAR_t * base, const cy_stc_adc_diag_config_t * config);
void Cy_Adc_Diag_Enable(volatile stc_PASS_SAR_t * base);
void Cy_Adc_Diag_Disable(volatile stc_PASS_SAR_t * base);

/* For calibration */
void Cy_Adc_TriggerCalibrationUpdate(volatile stc_PASS_SAR_t * base);
bool Cy_Adc_IsCalibrationUpdateDone(const volatile stc_PASS_SAR_t * base);
cy_en_adc_status_t Cy_Adc_SetDigitalCalibrationValue(volatile stc_PASS_SAR_t * base, const cy_stc_adc_digital_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_GetDigitalCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_digital_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_SetAltDigitalCalibrationValue(volatile stc_PASS_SAR_t * base, const cy_stc_adc_digital_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_GetAltDigitalCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_digital_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_SetAnalogCalibrationValue(volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_GetAnalogCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_SetAltAnalogCalibrationValue(volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config);
cy_en_adc_status_t Cy_Adc_GetAltAnalogCalibrationValue(const volatile stc_PASS_SAR_t * base, cy_stc_adc_analog_calibration_conifg_t * config);

/* For debugging */
cy_en_adc_ref_buf_mode_t Cy_Adc_GetReferenceBufferMode(volatile stc_PASS_EPASS_MMIO_t * base);
void Cy_Adc_SetReferenceBufferMode(volatile stc_PASS_EPASS_MMIO_t * base, cy_en_adc_ref_buf_mode_t mode);
cy_en_adc_status_t Cy_Adc_SetDebugFreezeMode(volatile stc_PASS_EPASS_MMIO_t * base, const cy_stc_adc_debug_freeze_config_t * config);

/* For SAR general trigger input / output trigger setting */
cy_en_adc_status_t Cy_Adc_SetGenericTriggerInput(volatile stc_PASS_EPASS_MMIO_t * base, uint8_t numOfAdc, uint8_t triggerInputNumber, uint8_t genericTriggerValue);
cy_en_adc_status_t Cy_Adc_SetGenericOutpuTrigger(volatile stc_PASS_EPASS_MMIO_t * base, uint8_t numOfAdc, uint8_t outputTriggerNumber, uint8_t genericTriggerValue);

/* For temperature measurements */
double Cy_Adc_CalculateDieTemperature(cy_stc_adc_temp_ref_t *refValue, cy_stc_adc_temp_raw_t *rawValue);


/** \} group_adc_functions */

#if defined(__cplusplus)
}
#endif

#endif /* CY_ADC_H */

/** \} group_adc */

/* [] END OF FILE */
