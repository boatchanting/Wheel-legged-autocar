/***************************************************************************//**
* \file cy_tcpwm_pwm.h
* \version 1.0
*
* \brief
* The header file of the TCPWM PWM driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_PWM_H)
#define CY_TCPWM_PWM_H

#include "cy_tcpwm.h"
#include "cy_tcpwm_counter.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_tcpwm_pwm
* \{
*/

/**
* \addtogroup group_tcpwm_pwm_macro
* \{
*/

/** \defgroup group_tcpwm_pwm_status Counter Status
* \{
* The counter status.
*/
#define CY_TCPWM_PWM_STATUS_DOWN_COUNTING   (0x1uL) /**< Timer/Counter is down counting */
#define CY_TCPWM_PWM_STATUS_UP_COUNTING     (0x2uL) /**< Timer/Counter is up counting */

/**< Timer/Counter the counter is running */
#define CY_TCPWM_PWM_STATUS_COUNTER_RUNNING (TCPWM_CNT_STATUS_RUNNING_Msk)

/** \} group_tcpwm_pwm_status */

/** \} group_tcpwm_pwm_macro */

/**
* \addtogroup group_tcpwm_pwm_enum
* \{
*/

/** \defgroup group_tcpwm_pwm_run_modes PWM run modes
* \{
* Run modes for the pwm timer.
*/
typedef enum cy_en_tcpwm_pwm_run_modes
{
    CY_TCPWM_PWM_CONTINUOUS = 0u,   /**< Counter runs forever */
    CY_TCPWM_PWM_ONESHOT    = 1u,   /**< Counter runs once and then stops */
} cy_en_tcpwm_pwm_run_modes_t;
/** \} group_tcpwm_pwm_run_modes */

/** \defgroup group_tcpwm_pwm_modes PWM modes 
* \{
* Sets the PWM modes.
*/
typedef enum cy_en_tcpwm_pwm_modes
{
    CY_TCPWM_PWM_MODE_PWM           = 4u,   /**< Standard PWM Mode*/
    CY_TCPWM_PWM_MODE_DEADTIME      = 5u,   /**< PWM with deadtime mode*/
    CY_TCPWM_PWM_MODE_PSEUDORANDOM  = 6u,   /**< Pseudo Random PWM */
} cy_en_tcpwm_pwm_modes_t;
/** \} group_tcpwm_pwm_modes */

/** \defgroup group_tcpwm_pwm_alignment PWM Alignment
* Sets the alignment of the PWM.
* \{
*/
typedef enum cy_en_tcpwm_pwm_alignment
{
    CY_TCPWM_PWM_LEFT_ALIGN         = 0u,   /**< PWM is left aligned, meaning it starts high */
    CY_TCPWM_PWM_RIGHT_ALIGN        = 1u,   /**< PWM is right aligned meaning it start low */
    CY_TCPWM_PWM_CENTER_ALIGN       = 2u,   /**< PWM is centered aligned, terminal count only occurs on underflow */
    CY_TCPWM_PWM_ASYMMETRIC_ALIGN   = 3u,   /**< PWM is asymmetrically aligned, terminal count  occurs on overflow and underflow */
} cy_en_tcpwm_pwm_alignment_t;
/** \} group_tcpwm_pwm_alignment */

/** \defgroup group_tcpwm_pwm_kill_modes PWM kill modes
* Sets the kill mode for the PWM.
* \{
*/
typedef enum cy_en_tcpwm_pwm_kill_modes
{
    CY_TCPWM_PWM_NOT_STOP_ON_KILL   = 0u,   /**< PWM output won't stop */
    CY_TCPWM_PWM_STOP_ON_KILL       = 2u,   /**< PWM stops counting on kill */
    CY_TCPWM_PWM_SYNCH_STOP_ON_KILL = 3u,   /**< PWM output is killed after next TC */
} cy_en_tcpwm_pwm_kill_modes_t;
/** \} group_tcpwm_pwm_kill_modes */

/** \defgroup group_tcpwm_pwm_output_invert PWM output invert
* \{
* Output invert modes.
*/
typedef enum cy_en_tcpwm_pwm_output_invert
{
    CY_TCPWM_PWM_INVERT_DISABLE = 0u,   /**< Invert the output mode */
    CY_TCPWM_PWM_INVERT_ENABLE  = 1u,   /**< Do not invert the output mode */
} cy_en_tcpwm_pwm_output_invert_t;
/** \} group_tcpwm_pwm_output_invert */

/** \defgroup group_tcpwm_pwm_output_configuration PWM output signal configuration
* \{
* The configuration of PWM output signal for PWM alignment.
*/
typedef enum cy_en_tcpwm_pwm_output_configuration
{
    CY_TCPWM_PWM_TR_CTRL2_SET       = 0u,   /**< Set define for PWM output signal configuration */
    CY_TCPWM_PWM_TR_CTRL2_CLEAR     = 1u,   /**< Clear define for PWM output signal configuration */
    CY_TCPWM_PWM_TR_CTRL2_INVERT    = 2u,   /**< Invert define for PWM output signal configuration */
    CY_TCPWM_PWM_TR_CTRL2_NO_CHANGE = 3u,   /**< No change define for PWM output signal configuration */
} cy_en_tcpwm_pwm_output_configuration_t;
/** \} group_tcpwm_pwm_output_configuration */

/** \defgroup group_tcpwm_pwm_output_disable State of output signal on disabling PWM
* \{
* The configuration of PWM output signal after disable.
*/
typedef enum cy_en_tcpwm_pwm_output_disable_modes
{
    CY_TCPWM_PWM_OUT_MODE_HIGHZ     = 0u,   /**< On counter disable, the PWM outputs level set to "Z" (high impedance). */
    CY_TCPWM_PWM_OUT_MODE_RETAIN    = 1u,   /**< On counter disable, the PWM outputs are retained. */
    CY_TCPWM_PWM_OUT_MODE_LOW       = 2u,   /**< On counter disable, the PWM outputs is driven to fixed 0. */
    CY_TCPWM_PWM_OUT_MODE_HIGH      = 3u,   /**< On counter disable, the PWM outputs is driven to fixed 1. */
} cy_en_tcpwm_pwm_output_disable_modes_t;
/** \} group_tcpwm_pwm_output_disable */

/** \} group_tcpwm_pwm_enum */

/**
* \addtogroup group_tcpwm_pwm_data_structures
* \{
*/

/** PWM configuration structure */
typedef struct cy_stc_tcpwm_pwm_config
{
    cy_en_tcpwm_pwm_modes_t                 pwmMode;            /**< Sets the PWM mode, see \ref cy_en_tcpwm_pwm_modes_t */
    cy_en_tcpwm_clk_prescalers_t            clockPrescaler;     /**< Sets the clock prescaler inside the TCWPM block see \ref cy_en_tcpwm_clk_prescalers */
    bool                                    debug_pause;        /**< If enabled, the counter pauses in debug mode else it kept on running   >*/
    uint16_t                                deadTime;           /**< The number of dead time-clocks in PWM in PWM_DT mode (8-bit for Grp0 and Grp2, 16-bit for Grp1)*/
    uint16_t                                deadTimeComp;       /**< The number of dead time-clocks in PWM_COMPL n PWM_DT mode (only for Grp1)*/
    bool                                    runMode;            /**< Set the PWM Run Mode, TRUE: One shot, FALSE: Continuous */
    uint32_t                                countDirection;     /**< Sets the counter direction see \ref group_tcpwm_counter_direction */
    cy_en_tcpwm_pwm_output_configuration_t  cc0MatchMode;       /**< Sets the effect of a compare match 0 event on the "line_out" output signals, see cy_en_tcpwm_pwm_output_configuration_t >*/
    cy_en_tcpwm_pwm_output_configuration_t  overflowMode;       /**< Sets the effect of a compare overflow event on the "line_out" output signals, see cy_en_tcpwm_pwm_output_configuration_t >*/
    cy_en_tcpwm_pwm_output_configuration_t  underflowMode;      /**< Sets the effect of a compare underflow event on the "line_out" output signals, see cy_en_tcpwm_pwm_output_configuration_t >*/
    cy_en_tcpwm_pwm_output_configuration_t  cc1MatchMode;       /**< Sets the effect of a compare match 1 event on the "line_out" output signals, see cy_en_tcpwm_pwm_output_configuration_t >*/
    uint32_t                                period;             /**< Sets the period of the pwm */
    uint32_t                                period_buff;        /**< Sets the period_buff of the pwm */
    bool                                    enablePeriodSwap;   /**< Enables swapping of period 0 and period 1 on terminal count */
    bool                                    enableLineSelSwap;  /**< Enables swapping of LINE_SEL and LINE_SEL_BUFF on terminal count and Switch event */
    int32_t                                 compare0;           /**< Sets the value for Compare0 */
    uint32_t                                compare0_buff;      /**< Sets the value for Compare0_buff */
    bool                                    enableCompare0Swap; /**< If enabled, the compare values are swapped each time the comparison is true */
    uint32_t                                compare1;           /**< Sets the value for Compare1 (Compare1 is available for Group 1 counter) */
    uint32_t                                compare1_buff;      /**< Sets the value for Compare1_buff (Compare1 is available for Group 1 counter) */
    bool                                    enableCompare1Swap; /**< If enabled, the compare1 values are swapped each time the comparison is true */
    cy_en_tcpwm_interrupt_sources_t         interruptSources;   /**< Enables an interrupt on the terminal count, capture or compare, see \ref cy_en_tcpwm_interrupt_sources_t */
    uint32_t                                invertPWMOut;       /**< Inverts the PWM output */
    uint32_t                                invertPWMOutN;      /**< Inverts the PWM_n output */
    cy_en_tcpwm_pwm_kill_modes_t            killMode;           /**< Configures the PWM kill modes, see \ref group_tcpwm_pwm_kill_modes */
    cy_en_tcpwm_input_modes_t               switchInputMode;    /**< Configures how the switch input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                switchInput;        /**< Selects which input the switch uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               reloadInputMode;    /**< Configures how the reload input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                reloadInput;        /**< Selects which input the reload uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               startInputMode;     /**< Configures how the start input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                startInput;         /**< Selects which input the start uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               kill0InputMode;     /**< Configures how the kill0 input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                kill0Input;         /**< Selects which input the kill0 uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               kill1InputMode;     /**< Configures how the kill1 input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                kill1Input;         /**< Selects which input the kill1 uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_input_modes_t               countInputMode;     /**< Configures how the count input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                                countInput;         /**< Selects which input the count uses, should be b/w 0x00 to 0xff */
    bool                                    immediateKill;      /**< Specifies whether the kill event immediately deactivates the dt_line_out and dt_line_compl_out or with the next module clock */
    cy_en_tcpwm_pwm_output_disable_modes_t  pwmOnDisable;       /**< Specifies the behavior of the PWM outputs line_out and line_compl_out see \ref cy_en_tcpwm_pwm_output_disable_modes_t */
    cy_en_tcpwm_counter_trig_out_sel_t      trigger0EventCfg;   /**< Configures which internal event generates on output trigger 0 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
    cy_en_tcpwm_counter_trig_out_sel_t      trigger1EventCfg;   /**< Configures which internal event generates on output trigger 1 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
}cy_stc_tcpwm_pwm_config_t;

/** \} group_tcpwm_pwm_data_structures */


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_tcpwm_pwm_functions
* \{
*/

extern uint32_t Cy_Tcpwm_Pwm_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_pwm_config_t const *config);
extern void Cy_Tcpwm_Pwm_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_Pwm_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_SetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0);
extern uint32_t Cy_Tcpwm_Pwm_GetCompare0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_SetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare0);
extern uint32_t Cy_Tcpwm_Pwm_GetCompare0_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_EnableCompare0Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable);
extern void Cy_Tcpwm_Pwm_SetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1);
extern uint32_t Cy_Tcpwm_Pwm_GetCompare1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_SetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t compare1);
extern uint32_t Cy_Tcpwm_Pwm_GetCompare1_Buff(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_EnableCompare1Swap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable);
extern void Cy_Tcpwm_Pwm_SetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t count);
extern uint32_t Cy_Tcpwm_Pwm_GetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_SetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period);
extern uint32_t Cy_Tcpwm_Pwm_GetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_SetPeriodBuf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  uint32_t period1);
extern uint32_t Cy_Tcpwm_Pwm_GetPeriodBuf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_EnablePeriodSwap(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM,  bool enable);
extern void Cy_Tcpwm_Pwm_LeftAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_RightAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_CentreAlign(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_Pwm_LSFRConfig(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t tap_value);

/** \} group_tcpwm_pwm_functions */

/** \} group_tcpwm_pwm */

#if defined(__cplusplus)
}
#endif

#endif /* CY_TCPWM_PWM_H */


/* [] END OF FILE */
