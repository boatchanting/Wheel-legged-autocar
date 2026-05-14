/***************************************************************************//**
* \file cy_tcpwm_sr.h
* \version 1.0
*
* \brief
* The header file of the TCPWM Shift Register driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_SR_H)
#define CY_TCPWM_SR_H

#include "cy_tcpwm.h"
#include "cy_tcpwm_counter.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_tcpwm_sr
* \{
*/

/**
* \addtogroup group_tcpwm_sr_macro
* \{
*/

/** \cond INTERNAL */
#define    CY_TCPWM_QUADDEC_CTRL_SR_MODE     (0x7uL)   /**< Shift Register mode for CTRL register */
/** \endcond */

/** \} group_tcpwm_sr_macro */

/**
* \addtogroup group_tcpwm_sr_enum
* \{
*/

/**
* \defgroup group_tcpwm_sr_run_modes Counter Run Modes
* \{
* Run modes for the shift register counter.
*/
typedef enum cy_en_tcpwm_sr_run_modes
{
    CY_TCPWM_SR_CONTINUOUS = 0u,     /**< Counter runs forever */
    CY_TCPWM_SR_ONESHOT    = 1u,     /**< Counter runs once and then stops */
} cy_en_tcpwm_sr_run_modes_t;
/** \} group_tcpwm_sr_run_modes */

/** \defgroup group_tcpwm_sr_compare_capture Counter Compare Capture
* \{
* A compare or capture mode.
*/
typedef enum cy_en_tcpwm_sr_cc_mode
{
    CY_TCPWM_SR_MODE_CAPTURE    = 2u,   /**< Timer/Counter is in Capture Mode */
    CY_TCPWM_SR_MODE_COMPARE    = 0u,   /**< Timer/Counter is in Compare Mode */
} cy_en_tcpwm_sr_cc_mode_t;
/** \} group_tcpwm_sr_compare_capture */

/** \} group_tcpwm_sr_enum */

/**
* \addtogroup group_tcpwm_sr_data_structures
* \{
*/

/** Shift Register configuration structure */
typedef struct cy_stc_tcpwm_sr_config
{
     uint32_t                           period;             /**< Sets the period*/
     uint32_t                           period_buff;        /**< Sets the period_buff for the Shift Register to know taps for PWM */
     cy_en_tcpwm_clk_prescalers_t       clockPrescaler;     /**< Sets the clock prescaler inside the TCWPM block see \ref cy_en_tcpwm_clk_prescalers */
     uint32_t                           runMode;            /** < Run Mode selection. TRUE: One shot, FALSE: Continuous */
     bool                               debug_pause;        /**< If enabled, the counter pauses in debug mode else it kept on running   >*/
     uint32_t                           compare0;           /**< Sets the value for Compare0 */
     uint32_t                           compare0_buff;      /**< Sets the value for Compare0_buff */
     bool                               enableCompare0Swap; /**< If enabled, the compare values are swapped each time the comparison is true */
     uint32_t                           compare1;           /**< Sets the value for Compare1 (Compare1 can only be used for counter in Group 1) */
     uint32_t                           compare1_buff;      /**< Sets the value for Compare1_buff (Compare1 can only be used for counter in Group 1) */
     bool                               enableCompare1Swap; /**< If enabled, the compare1 values are swapped each time the comparison is true */
     cy_en_tcpwm_interrupt_sources_t    interruptSources;   /**< Enabled an interrupt on the terminal count, capture or compare, see \ref cy_en_tcpwm_interrupt_sources_t */
     cy_en_tcpwm_input_modes_t          serialInInputMode;  /**< Configures how the capture input behaves see \ref cy_en_tcpwm_input_modes_t */
     uint32_t                           serialInInput;      /**< Selects which input the capture uses, should be b/w 0x00 to 0xff */
     cy_en_tcpwm_input_modes_t          reloadInputMode;    /**< Configures how the reload input behaves see \ref cy_en_tcpwm_input_modes_t */
     uint32_t                           reloadInput;        /**< Selects which input the reload uses, should be b/w 0x00 to 0xff */
     cy_en_tcpwm_input_modes_t          startInputMode;     /**< Configures how the start input behaves see \ref cy_en_tcpwm_input_modes_t */
     uint32_t                           startInput;         /**< Selects which input the start uses, should be b/w 0x00 to 0xff */
     cy_en_tcpwm_input_modes_t          stopInputMode;      /**< Configures how the stop input behaves see \ref cy_en_tcpwm_input_modes_t */
     uint32_t                           stopInput;          /**< Selects which input the stop uses, should be b/w 0x00 to 0xff */
     cy_en_tcpwm_input_modes_t          shiftInputMode;     /**< Configures how the shift input behaves see \ref cy_en_tcpwm_input_modes_t */
     uint32_t                           shiftInput;         /**< Selects which shift the count uses, should be b/w 0x00 to 0xff */
     cy_en_tcpwm_counter_trig_out_sel_t trigger0EventCfg;   /**< Configures which internal event generates on output trigger 0 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
     cy_en_tcpwm_counter_trig_out_sel_t trigger1EventCfg;   /**< Configures which internal event generates on output trigger 1 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
}cy_stc_tcpwm_sr_config_t;

/** \} group_tcpwm_sr_data_structures */


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_tcpwm_sr_functions
* \{
*/
extern uint32_t Cy_Tcpwm_Sr_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_sr_config_t const *config);
extern void Cy_Tcpwm_Sr_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Sr_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Sr_GetCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Sr_GetCapture0Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t compare0);
extern uint32_t Cy_Tcpwm_Sr_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t compare1);
extern uint32_t Cy_Tcpwm_Sr_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, bool enable);
extern void Cy_Tcpwm_Sr_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t compare0);
extern uint32_t Cy_Tcpwm_Sr_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t compare1);
extern uint32_t Cy_Tcpwm_Sr_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_SetShiftRegister(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t sr);
extern uint32_t Cy_Tcpwm_Sr_GetShiftRegister(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Sr_SetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t period);
extern uint32_t Cy_Tcpwm_Sr_GetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);


/** \} group_tcpwm_sr_functions */

/** \} group_tcpwm_sr */

#if defined(__cplusplus)
}
#endif

#endif /* CY_TCPWM_SR_H */


/* [] END OF FILE */
