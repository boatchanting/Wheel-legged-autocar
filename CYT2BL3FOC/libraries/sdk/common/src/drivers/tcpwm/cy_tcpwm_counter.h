/***************************************************************************//**
* \file tcpwm_counter.h
* \version 1.0
*
* \brief
* The header file of the TCPWM Timer Counter driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_COUNTER_H)
#define CY_TCPWM_COUNTER_H

#include "cy_tcpwm.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_tcpwm_counter
* \{
*/

/**
* \addtogroup group_tcpwm_counter_macro
* \{
*/

/** \defgroup group_tcpwm_counter_status Counter Status
* \{
* The counter status.
*/
#define CY_TCPWM_COUNTER_STATUS_DOWN_COUNTING   (0x1uL) /**< Timer/Counter is down counting */
#define CY_TCPWM_COUNTER_STATUS_UP_COUNTING     (0x2uL) /**< Timer/Counter is up counting */

/**< Timer/Counter the counter is running */
#define CY_TCPWM_COUNTER_STATUS_COUNTER_RUNNING (TCPWM_CNT_STATUS_RUNNING_Msk)
/** \} group_tcpwm_counter_status */

/** \} group_tcpwm_counter_macro */

/**
* \addtogroup group_tcpwm_counter_enum
* \{
*/

/** \defgroup group_tcpwm_counter_run_modes Counter run modes
* \{
* Run modes for the counter.
*/
typedef enum cy_en_tcpwm_counter_run_modes
{
    CY_TCPWM_COUNTER_CONTINUOUS = 0u,   /**< Counter runs forever */
    CY_TCPWM_COUNTER_ONESHOT    = 1u,   /**< Counter runs once and then stops */
} cy_en_tcpwm_counter_run_modes_t;
/** \} cy_en_tcpwm_counter_run_modes */

/** \defgroup group_tcpwm_counter_modes Counter modes 
* \{
* Sets the counter modes.
*/
typedef enum group_tcpwm_counter_modes
{
    CY_TCPWM_COUNTER_COUNT_UP       = 0u,   /**< Counter counts up */
    CY_TCPWM_COUNTER_COUNT_DOWN     = 1u,   /**< Counter counts down */
    CY_TCPWM_COUNTER_COUNT_UP_DOWN1 = 2u,   /**< Counter counts up and down terminal count only occurs on underflow */
    CY_TCPWM_COUNTER_COUNT_UP_DOWN2 = 3u,   /**< Counter counts up and down terminal count occurs on both overflow and underflow */
} cy_en_tcpwm_counter_modes_t;
/** \} group_tcpwm_counter_modes */

/** \defgroup group_tcpwm_counter_compare_capture Counter Compare Capture
* \{
* A compare or capture mode.
*/
typedef enum cy_en_tcpwm_counter_cc_mode
{
    CY_TCPWM_COUNTER_MODE_CAPTURE   = 2u,   /**< Timer/Counter is in Capture Mode */
    CY_TCPWM_COUNTER_MODE_COMPARE   = 0u,   /**< Timer/Counter is in Compare Mode */
} cy_en_tcpwm_counter_cc_mode_t;
/** \} group_tcpwm_counter_compare_capture */

/** \} group_tcpwm_counter_enum */

/**
* \addtogroup group_tcpwm_counter_data_structures
* \{
*/

/** Counter Timer configuration structure */
typedef struct cy_stc_tcpwm_counter_config
{
    uint32_t                                period;             /**< Sets the period of the counter */
    cy_en_tcpwm_clk_prescalers_t            clockPrescaler;     /**< Sets the clock prescaler inside the TCWPM block see \ref cy_en_tcpwm_clk_prescalers */
    cy_en_tcpwm_counter_run_modes_t         runMode;            /**< Sets the Counter Timer Run mode see \ref cy_en_tcpwm_counter_run_modes_t */
    cy_en_tcpwm_counter_modes_t             countDirection;     /**< Sets the counter direction see \ref cy_en_tcpwm_counter_modes_t */
    bool                                    debug_pause;        /**< If enabled, the counter pauses in debug mode else it kept on running   >*/
    cy_en_tcpwm_counter_cc_mode_t           compareOrCapture;   /**< The counter can either compare or capture a value see \ref cy_en_tcpwm_counter_cc_mode_t */
    uint32_t                                compare0;           /**< Sets the value for Compare0 */
    uint32_t                                compare0_buff;      /**< Sets the value for Compare0_buff */
    bool                                    enableCompare0Swap; /**< If enabled, the compare values are swapped each time the comparison is true */
    cy_en_tcpwm_interrupt_sources_t         interruptSources;   /**< Enabled an interrupt on the terminal count, capture or compare, see \ref cy_en_tcpwm_interrupt_sources_t */
    cy_en_tcpwm_input_modes_t               capture0InputMode;  /**< Configures how the capture input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                capture0Input;      /**< Selects which input the capture uses, should be b/w 0x00 to 0xff */
    uint32_t                                compare1;           /**< Sets the value for Compare1 (Compare 1 will be active only for Group1 counters) */
    uint32_t                                compare1_buff;      /**< Sets the value for Compare1_buff (Compare 1 will be active only for Group1 counters) */
    bool                                    enableCompare1Swap; /**< If enabled, the compare1 values are swapped each time the comparison is true */
    cy_en_tcpwm_input_modes_t               capture1InputMode;  /**< Configures how the capture input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                capture1Input;      /**< Selects which input the capture uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               reloadInputMode;    /**< Configures how the reload input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                reloadInput;        /**< Selects which input the reload uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               startInputMode;     /**< Configures how the start input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                startInput;         /**< Selects which input the start uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               stopInputMode;      /**< Configures how the stop input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                stopInput;          /**< Selects which input the stop uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               countInputMode;     /**< Configures how the count input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                countInput;         /**< Selects which input the count uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_counter_trig_out_sel_t      trigger0EventCfg;   /**< Configures which internal event generates on output trigger 0 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
    cy_en_tcpwm_counter_trig_out_sel_t      trigger1EventCfg;   /**< Configures which internal event generates on output trigger 1 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
}cy_stc_tcpwm_counter_config_t;

/** \} group_tcpwm_counter_data_structures */

/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_tcpwm_counter_functions
* \{
*/

extern uint32_t Cy_Tcpwm_Counter_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_counter_config_t const *config);
extern void Cy_Tcpwm_Counter_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Counter_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Counter_GetCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Counter_GetCapture1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Counter_GetCapture0Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Counter_GetCapture1Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0);
extern uint32_t Cy_Tcpwm_Counter_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1);
extern uint32_t Cy_Tcpwm_Counter_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0);
extern uint32_t Cy_Tcpwm_Counter_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1);
extern uint32_t Cy_Tcpwm_Counter_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable);
extern void Cy_Tcpwm_Counter_EnableCompare1Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable);
extern void Cy_Tcpwm_Counter_SetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t count);
extern uint32_t Cy_Tcpwm_Counter_GetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period);
extern uint32_t Cy_Tcpwm_Counter_GetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period);
extern uint32_t Cy_Tcpwm_Counter_GetPeriod_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetTC_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCC0_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_SetCC1_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_Counter_GetTC_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_Counter_GetCC0_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_Counter_GetCC1_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_ClearTC_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_ClearCC0_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Counter_ClearCC1_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);

/** \} group_tcpwm_counter_functions */

/** \} group_tcpwm_counter */

#if defined(__cplusplus)
}
#endif

#endif /* TCPWM_COUNTER_H */


/* [] END OF FILE */
