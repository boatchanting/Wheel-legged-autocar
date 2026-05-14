/***************************************************************************//**
* \file tcpwm_quaddec.h
* \version 1.0
*
* \brief
* The header file of the TCPWM quadrature Decoder driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_QUADDEC_H)
#define CY_TCPWM_QUADDEC_H

#include "cy_tcpwm.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_tcpwm_quaddec
* \{
*/

/**
* \addtogroup group_tcpwm_quaddec_macro
* \{
*/

/** \defgroup group_tcpwm_quaddec_status Counter Status
* \{
* The counter status.
*/
#define CY_TCPWM_QUADDEC_STATUS_DOWN_COUNTING   (0x1uL)                        /**< QuadDec is down counting */
#define CY_TCPWM_QUADDEC_STATUS_UP_COUNTING     (0x2uL)                        /**< QuadDec is up counting */
#define CY_TCPWM_QUADDEC_STATUS_COUNTER_RUNNING (TCPWM_CNT_STATUS_RUNNING_Msk) /** QuadDec the counter is running */
/** \} group_tcpwm_quaddec_status */

/** \cond INTERNAL */
#define CY_TCPWM_QUADDEC_CTRL_QUADDEC_MODE      (0x3uL) /**< Quadrature encoding mode for CTRL register */
/** \endcond */

/** \} group_tcpwm_quaddec_macro */

/**
* \addtogroup group_tcpwm_quaddec_enum
* \{
*/

/** \defgroup group_tcpwm_quaddec_mode QuadDec operating modes 
* \{
* The quadrature decoder operation.
*/
typedef enum cy_en_tcpwm_quaddec_modes
{
    CY_TCPWM_QUADDEC_X1                     = 0u,   /**< X1 mode */
    CY_TCPWM_QUADDEC_X2                     = 1u,   /**< X2 mode */
    CY_TCPWM_QUADDEC_X4                     = 2u,   /**< X4 mode */
    CY_TCPWM_QUADDEC_UP_DOWN_ROTARY_COUNT   = 3u,   /**< Up/down rotary count mode */
} cy_en_tcpwm_quaddec_modes_t;
/** \} group_tcpwm_quaddec_mode */

/** \defgroup group_tcpwm_quaddec_range_mode QuadDec range selection  
* \{
* The quadrature decoder range selection.
*/
typedef enum cy_en_tcpwm_quaddec_range_sel
{
    CY_TCPWM_QUADDEC_RANGE_0        = 0u,   /**< QUAD_RANGE0 mode */
    CY_TCPWM_QUADDEC_RANGE_0_CMP    = 1u,   /**< QUAD_RANGE0_CMP */
    CY_TCPWM_QUADDEC_RANGE_1_CMP    = 2u,   /**< QUAD_RANGE1_CMP */
    CY_TCPWM_QUADDEC_RANGE_1_CAPT   = 3u,   /**< QUAD_RANGE1_CAPT */
} cy_en_tcpwm_quaddec_range_sel_t;
/** \} group_tcpwm_quaddec_range_mode */

/** \} group_tcpwm_quaddec_enum */

/**
* \addtogroup group_tcpwm_quaddec_data_structures
* \{
*/

/** Quadrature Decoder configuration structure */
typedef struct cy_stc_tcpwm_quaddec_config
{
    cy_en_tcpwm_quaddec_modes_t         encoding_mode;      /**< Selects the quadrature encoding mode, see \ref cy_en_tcpwm_quaddec_modes_t */
    cy_en_tcpwm_quaddec_range_sel_t     range_mode;         /**< Selects the quadrature range mode, see \ref cy_en_tcpwm_quaddec_range_sel_t */
    uint32_t                            period;             /**< Sets the period of the counter */
    bool                                debug_pause;        /**< If enabled, the counter pauses in debug mode else it kept on running   >*/
    uint32_t                            compare0;           /**< Sets the value for Compare0 */
    uint32_t                            compare0_buff;      /**< Sets the value for Compare0_buff */
    bool                                enableCompare0Swap; /**< If enabled, the compare0 values are swapped each time the comparison is true */
    cy_en_tcpwm_input_modes_t           capture0InputMode;  /**< Configures how the capture input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                            capture0Input;      /**< Selects which input the capture uses, should be b/w 0x00 to 0xff */
    uint32_t                            compare1;           /**< Sets the value for Compare1 */
    uint32_t                            compare1_buff;      /**< Sets the value for Compare1_buff */
    bool                                enableCompare1Swap; /**< If enabled, the compare1 values are swapped each time the comparison is true */
    cy_en_tcpwm_input_modes_t           capture1InputMode;  /**< Configures how the capture input behaves see \ref cy_en_tcpwm_input_modes_t */
    uint32_t                            capture1Input;      /**< Selects which input the capture uses, should be b/w 0x00 to 0xff */
    cy_en_tcpwm_interrupt_sources_t     interruptSources;   /**< Enables an interrupt on the terminal count, capture or compare, see \ref cy_en_tcpwm_interrupt_sources_t */
    cy_en_tcpwm_input_modes_t           indexInputMode;     /**< Configures how the index input behaves, see \ref cy_en_tcpwm_input_modes_t */
    cy_en_tcpwm_input_selection_t       indexInput;         /**< Selects which input the index uses, the inputs are device-specific see \ref cy_en_tcpwm_input_selection_t */
    cy_en_tcpwm_input_modes_t           stopInputMode;      /**< Configures how the stop input behaves, see \ref cy_en_tcpwm_input_modes_t */
    cy_en_tcpwm_input_selection_t       stopInput;          /**< Selects which input the stop uses, the inputs are device-specific see \ref cy_en_tcpwm_input_selection_t */
    cy_en_tcpwm_input_selection_t       phiAInput;          /**< Selects which input the phiA uses, the inputs are device specific see \ref cy_en_tcpwm_input_selection_t */
    cy_en_tcpwm_input_selection_t       phiBInput;          /**< Selects which input the phiB uses, the inputs are device specific see \ref cy_en_tcpwm_input_selection_t */
    cy_en_tcpwm_counter_trig_out_sel_t  trigger0EventCfg;   /**< Configures which internal event generates on output trigger 0 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
    cy_en_tcpwm_counter_trig_out_sel_t  trigger1EventCfg;   /**< Configures which internal event generates on output trigger 1 see \ref cy_en_tcpwm_counter_trig_out_sel_t */
}cy_stc_tcpwm_quaddec_config_t;

/** \} group_tcpwm_quaddec_data_structures */


/*******************************************************************************
*        Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_tcpwm_quaddec_functions
* \{
*/

extern uint32_t Cy_Tcpwm_QuadDec_Init(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, cy_stc_tcpwm_quaddec_config_t const *config);
extern void Cy_Tcpwm_QuadDec_DeInit(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_Enable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_Disable(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetStatus(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetCapture0(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetCapture0Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetCapture1(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetCapture1Buf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_SetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM, uint32_t count);
extern uint32_t Cy_Tcpwm_QuadDec_GetCounter(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetPeriod(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint32_t Cy_Tcpwm_QuadDec_GetPeriodBuf(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_SetCC0_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_SetCC1_IntrMask(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_QuadDec_GetTC_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_QuadDec_GetCC0_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern uint8_t Cy_Tcpwm_QuadDec_GetCC1_IntrMasked(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_ClearTC_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_ClearCC0_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);
extern void Cy_Tcpwm_QuadDec_ClearCC1_Intr(volatile stc_TCPWM_GRP_CNT_t *ptscTCPWM);

/** \} group_tcpwm_quaddec_functions */

/** \} group_tcpwm_quaddec */

#if defined(__cplusplus)
}
#endif

#endif /* TCPWM_QUADDEC_H */


/* [] END OF FILE */
