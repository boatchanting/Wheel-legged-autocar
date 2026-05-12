/***************************************************************************//**
* \file cy_tcpwm.h
* \version 1.0
*
* The header file of the TCPWM driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_tcpwm Timer Counter PWM (TCPWM)
* \{
*
* The TCPWM driver is a multifunction driver that implements the core 
* microcontroller functionality, including Timer Counter, PWM, and 
* Quadrature Decoder using the TCPWM block.
*
* \section group_tcpwm_configuration Configuration Considerations
*
* The component is based on a hardware structure designed to share the same 
* hardware across all the various modes of operation. This structure 
* allows the same hardware to provide a flexible set of functions with 
* a little increase in the silicon use. 
*
* The block is split into three functional modes. There is one overall tcpwm driver 
* subdivided into the following:
*
*  * The Timer/Counter has the following features:
*  \li 16 or 32 bit Timer/Counter
*  \li Programmable Period Register
*  \li Output Compare, compare value can be swapped with a buffered compare value on comparison event
*  \li Capture with buffer register
*  \li Count Up, Count Down, or Count Up and Down Counting modes
*  \li Continuous or One Shot Run modes
*  \li Interrupt and Output on Overflow, Underflow, Capture, or Compare 
*  \li Start, Reload, Stop, Capture, and Count Inputs
*
*  * The PWM has the following features:
*  \li 16 or 32 bit Counter
*  \li Two Programmable Period Registers that can be swapped on overflow
*  \li Two Output Compare registers that can be swapped on overflow and/or underflow
*  \li Left Aligned, Right Aligned, Center Aligned, and Asymmetric Aligned modes
*  \li Continuous or One Shot Run modes
*  \li Pseudo Random mode
*  \li Two PWM outputs with Dead Time insertion
*  \li Interrupt and Output on Overflow, Underflow, or Compare 
*  \li Start, Reload, Stop, Swap(Capture), and Count Inputs
*
*  * The Quadrature Decoder has the following features:
*  \li 16 or 32 bit Counter
*  \li Counter Resolution of x1, x2, and x4 the frequency of the phiA(Count) and phiB(Start) inputs.
*  \li Index(Reload) Input to determine absolute position.
*  \li A positive edge on phiA increments the counter when phiB is 0 and decrements the counter when phiB is 1.
*
* \section group_tcpwm_more_information More Information
*
* For more information on the TCPWM peripheral, refer to the technical reference manual (TRM).
*
* \section group_tcpwm_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.1</td>
*     <td>Config Struct \ref cy_stc_tcpwm_counter_config_t updated with correct members
*         for trigger1/2, changed to trigger0EventCfg/trigger1EventCfg,
*         Config Struct \ref cy_stc_tcpwm_pwm_config_t added 
*         with trigger0EventCfg/trigger1EventCfg, immediateKill, pwmOnDisable,
*         Config Struct \ref cy_stc_tcpwm_quaddec_config_t added 
*         with trigger0EventCfg/trigger1EventCfg, 
*         Config Struct \ref cy_stc_tcpwm_sr_config_t added 
*         with trigger0EventCfg/trigger1EventCfg</td>
*     <td>Corrections, and enhancements</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>1.0</td>
*     <td>Initial version</td>
*     <td></td>
*   </tr>
* </table>
* \defgroup group_tcpwm_macro 		Macro
* \defgroup group_tcpwm_functions 	Functions
* \defgroup group_tcpwm_enum 		Enumerations
*
* \defgroup group_tcpwm_counter  	Counter (TCPWM)
* \defgroup group_tcpwm_pwm    		PWM (TCPWM)
* \defgroup group_tcpwm_quaddec    	Quadrature Decoder (TCPWM)
* \defgroup group_tcpwm_sr   		Shift Register (TCPWM)
*/


#if !defined(CY_TCPWM_H)
#define CY_TCPWM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cy_device_headers.h"
#include "ip/cyip_tcpwm_v2.h"
#include "syslib/cy_syslib.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_tcpwm_macro  
* \{
*/

/** Driver major version */
#define CY_TCPWM_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_TCPWM_DRV_VERSION_MINOR       0

/***************************************
*        Registers Constants
***************************************/

/**
* \defgroup group_tcpwm_reg_const Default registers constants
* \{
* Default constants for CNT Registers
*/
#define CY_TCPWM_CNT_CTRL_DEFAULT               (0x0ul)         /**< Default value for CTRL register */
#define CY_TCPWM_CNT_COUNTER_DEFAULT            (0x0ul)         /**< Default value for COUNTER register */
#define CY_TCPWM_CNT_CC_DEFAULT                 (0xFFFFFFFFul)  /**< Default value for CC register */
#define CY_TCPWM_CNT_CC_BUFF_DEFAULT            (0xFFFFFFFFul)  /**< Default value for CC_BUFF register */
#define CY_TCPWM_CNT_PERIOD_DEFAULT             (0xFFFFFFFFul)  /**< Default value for PERIOD register */
#define CY_TCPWM_CNT_PERIOD_BUFF_DEFAULT        (0xFFFFFFFFul)  /**< Default value for PERIOD_BUFF register */
#define CY_TCPWM_CNT_TR_CTRL0_DEFAULT           (0x10ul)        /**< Default value for TR_CTRL0 register */
#define CY_TCPWM_CNT_TR_CTRL1_DEFAULT           (0x3FFul)       /**< Default value for TR_CTRL1 register */
#define CY_TCPWM_CNT_TR_CTRL2_DEFAULT           (0x3Ful)        /**< Default value for TR_CTRL2 register */
#define CY_TCPWM_CNT_INTR_DEFAULT               (0x3ul)         /**< Default value for INTR register */
#define CY_TCPWM_CNT_INTR_SET_DEFAULT           (0x0ul)         /**< Default value for INTR_SET register */
#define CY_TCPWM_CNT_INTR_MASK_DEFAULT          (0x0ul)         /**< Default value for INTR_MASK register */
/** \} group_tcpwm_reg_const */

#define CY_TCPWM_CNT_STATUS_UP_POS              (0x1ul)         /**< Position of Up counting counter status */
#define CY_TCPWM_CNT_UP_INIT_VAL                (0x0ul)         /**< Initial value for the counter in the Up counting mode */
#define CY_TCPWM_CNT_UP_DOWN_INIT_VAL           (0x1ul)         /**< Initial value for the counter in the Up/Down counting modes */

/* Default value of TCPWM registers*/
#define CY_TCPWM_GRP_CNT_CTRL_DEFAULT           (0x000000f0ul)
#define CY_TCPWM_GRP_CNT_COUNTER_DEFAULT        (0x00000000ul)
#define CY_TCPWM_GRP_CNT_CC0_DEFAULT            (0xfffffffful)
#define CY_TCPWM_GRP_CNT_CC0_BUFF_DEFAULT       (0xfffffffful)
#define CY_TCPWM_GRP_CNT_CC1_DEFAULT            (0xfffffffful)
#define CY_TCPWM_GRP_CNT_CC1_BUFF_DEFAULT       (0xfffffffful)
#define CY_TCPWM_GRP_CNT_PERIOD_DEFAULT         (0xfffffffful)
#define CY_TCPWM_GRP_CNT_PERIOD_BUFF_DEFAULT    (0xfffffffful)
#define CY_TCPWM_GRP_CNT_DT_DEFAULT             (0x00000000ul)
#define CY_TCPWM_GRP_CNT_TR_IN_SEL0_DEFAULT     (0x00000100ul)
#define CY_TCPWM_GRP_CNT_TR_IN_SEL1_DEFAULT     (0x00000000ul)
#define CY_TCPWM_GRP_CNT_TR_IN_EDGE_SEL_DEFAULT (0x0000fffful)
#define CY_TCPWM_GRP_CNT_TR_PWM_CTRL_DEFAULT    (0x000000fful)
#define CY_TCPWM_GRP_CNT_TR_OUT_SEL_DEFAULT     (0x00000032ul)
#define CY_TCPWM_GRP_CNT_INTR_DEFAULT           (0x00000000ul)
#define CY_TCPWM_GRP_CNT_INTR_SET_DEFAULT       (0x00000000ul)
#define CY_TCPWM_GRP_CNT_INTR_MASK_DEFAULT      (0x00000000ul)

/** \} group_tcpwm_macro */

/**
* \addtogroup group_tcpwm_enum
* \{
*/

/**
* \defgroup group_tcpwm_input_selection  Input Selection
* \{
* Configures TCPWM inputs triggers
*/

/**
 *****************************************************************************
 ** \brief Selects which input to use
 *****************************************************************************/
typedef enum cy_en_tcpwm_input_selection
{
    CY_TCPWM_INPUT0       = 0u,     /**< Input is tied to logic 0 */
    CY_TCPWM_INPUT1       = 1u,     /**< Input is tied to logic 1 */
    CY_TCPWM_INPUT_TRIG0  = 2u,     /**< Input is connected to the trigger input 0 */
    CY_TCPWM_INPUT_TRIG1  = 3u,     /**< Input is connected to the trigger input 1 */
    CY_TCPWM_INPUT_TRIG2  = 4u,     /**< Input is connected to the trigger input 2 */
    CY_TCPWM_INPUT_TRIG3  = 5u,     /**< Input is connected to the trigger input 3 */
    CY_TCPWM_INPUT_TRIG4  = 6u,     /**< Input is connected to the trigger input 4 */
    CY_TCPWM_INPUT_TRIG5  = 7u,     /**< Input is connected to the trigger input 5 */
    CY_TCPWM_INPUT_TRIG6  = 8u,     /**< Input is connected to the trigger input 6 */
    CY_TCPWM_INPUT_TRIG7  = 9u,     /**< Input is connected to the trigger input 7 */
    CY_TCPWM_INPUT_TRIG8  = 10u,    /**< Input is connected to the trigger input 8 */
    CY_TCPWM_INPUT_TRIG9  = 11u,    /**< Input is connected to the trigger input 9 */
    CY_TCPWM_INPUT_TRIG10 = 12u,    /**< Input is connected to the trigger input 10 */
    CY_TCPWM_INPUT_TRIG11 = 13u,    /**< Input is connected to the trigger input 11 */
    CY_TCPWM_INPUT_TRIG12 = 14u,    /**< Input is connected to the trigger input 12 */
    CY_TCPWM_INPUT_TRIG13 = 15u     /**< Input is connected to the trigger input 13 */
} cy_en_tcpwm_input_selection_t;

/** \} group_tcpwm_input_selection */

/**
* \defgroup group_tcpwm_input_modes  Input Modes
* \{
* Configures how TCPWM inputs behave
*/

/**
 *****************************************************************************
 ** \brief Configures how TCPWM inputs behave
 *****************************************************************************/
typedef enum cy_en_tcpwm_input_modes
{
    CY_TCPWM_INPUT_RISING_EDGE  = 0u,   /**< A rising edge triggers the event (Capture, Start, Reload, etc..) */
    CY_TCPWM_INPUT_FALLING_EDGE = 1u,   /**< A falling edge triggers the event (Capture, Start, Reload, etc..) */
    CY_TCPWM_INPUT_BOTH_EDGES   = 2u,   /**< A rising edge or falling edge triggers the event (Capture, Start, Reload, etc..) */
    CY_TCPWM_INPUT_LEVEL        = 3u    /**< The event is triggered each edge of the TCPWM clock if the input is high */
} cy_en_tcpwm_input_modes_t ;

/** \} group_tcpwm_input_modes */

/**
* \defgroup group_tcpwm_interrupt_sources  Interrupt Sources
* \{
* Interrupt Sources 
*/

/**
 *****************************************************************************
 ** \brief Configures Interrupt Sources 
 *****************************************************************************/
typedef enum cy_en_tcpwm_interrupt_sources
{
    CY_TCPWM_INT_NONE        = 0u,  /**< None of Interrupt */
    CY_TCPWM_INT_ON_TC       = 1u,  /**< Interrupt on Terminal count(TC) */
    CY_TCPWM_INT_ON_CC       = 2u,  /**< Interrupt on Compare/Capture(CC) */
    CY_TCPWM_INT_ON_TC_OR_CC = 3u   /**< Interrupt on TC or CC */
} cy_en_tcpwm_interrupt_sources_t ;

/** \} group_tcpwm_interrupt_sources */

/** \defgroup group_tcpwm_clk_prescalers Counter CLK Prescaler values
* \{
* Clock prescaler values.
*/

/**
 *****************************************************************************
 ** \brief Configures Counter Clock Prescalar 
 *****************************************************************************/
typedef enum cy_en_tcpwm_clk_prescalers
{
    CY_TCPWM_PRESCALER_DIVBY_1      = 0u,   /**< Divide by 1 */
    CY_TCPWM_PRESCALER_DIVBY_2      = 1u,   /**< Divide by 2 */
    CY_TCPWM_PRESCALER_DIVBY_4      = 2u,   /**< Divide by 4 */
    CY_TCPWM_PRESCALER_DIVBY_8      = 3u,   /**< Divide by 8 */
    CY_TCPWM_PRESCALER_DIVBY_16     = 4u,   /**< Divide by 16 */
    CY_TCPWM_PRESCALER_DIVBY_32     = 5u,   /**< Divide by 32 */
    CY_TCPWM_PRESCALER_DIVBY_64     = 6u,   /**< Divide by 64 */
    CY_TCPWM_PRESCALER_DIVBY_128    = 7u,   /**< Divide by 128 */
} cy_en_tcpwm_clk_prescalers_t;

/** \} group_tcpwm_clk_prescalers */

/** \defgroup group_tcpwm_counter_output_trigger Counter CLK Prescalers
* \{
* The trig_output selection.
*/

/**
 *****************************************************************************
 ** \brief Configures Counter Output Trigger 
 *****************************************************************************/
typedef enum cy_en_tcpwm_counter_trig_out_sel
{
    CY_TCPWM_COUNTER_OVERFLOW       = 0u,   /**< An overflow event */
    CY_TCPWM_COUNTER_UNDERFLOW      = 1u,   /**< An underflow event */
    CY_TCPWM_COUNTER_TERMINAL_COUNT = 2u,   /**< A terminal count event */
    CY_TCPWM_COUNTER_CC0_MATCH      = 3u,   /**< A cc0_match event */
    CY_TCPWM_COUNTER_CC1_MATCH      = 4u,   /**< A cc1_match event */
    CY_TCPWM_COUNTER_LINE_OUT       = 5u,   /**< PWM output signal line_out */
    CY_TCPWM_COUNTER_DISABLED       = 7u,   /**< Output trigger disabled */
} cy_en_tcpwm_counter_trig_out_sel_t;

/** \} group_tcpwm_counter_output_trigger */

/** \} group_tcpwm_enum */

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_tcpwm_functions
* \{
*/

extern void Cy_Tcpwm_TriggerStart(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_TriggerReloadOrIndex(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_TriggerStopOrKill(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_TriggerCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_TriggerCapture1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_GetInterruptStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_ClearInterrupt(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t source);
extern void Cy_Tcpwm_SetInterrupt(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t source);
extern void Cy_Tcpwm_SetInterruptMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t mask);
extern uint32_t Cy_Tcpwm_GetInterruptMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_GetInterruptStatusMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);

/** \} group_tcpwm_functions */

/** \} group_tcpwm */

#if defined(__cplusplus)
}
#endif

#endif /* CY_TCPWM_H */


/* [] END OF FILE */
