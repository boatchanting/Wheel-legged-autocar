/***************************************************************************//**
* \file cy_button.h
* \version 1.0
*
* \brief
* Provides an API declaration of the Button middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_mw_button Middleware Buttons (MW Button)
* \{
* Offers CY_BUTTON_MAX_BUTTONS buttons
*
* Basic functions for buttons using GPIO, featuring de-bouncing (no interrupts)
* The timer module is used (TIMER_ID_BUTTON). It must be initialized before
* and the Timer_Main function must be called cyclically (for callback function
* support).
* 
* \defgroup group_mw_button_macros Macro
* \defgroup group_mw_button_functions Functions
* \defgroup group_mw_button_data_structures Data structures
* \defgroup group_mw_button_enums Enumerated types
*/


#if !defined(CY_BUTTON_H)
#define CY_BUTTON_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if defined(__cplusplus)
extern "C" {
#endif

/**
* \addtogroup group_mw_button_macros
* \{
*/

/** Sets maximum number of supported buttons of the button middleware driver
    The user cannot init more buttons than defined here, but less is possible */
#define CY_BUTTON_MAX_BUTTONS       9  // must be < 256

/** Set the scan interval for reading the pin state in milliseconds (20 ... 100) */
#define CY_BUTTON_SCAN_INTERVAL_MS     20

/** Set debounce count i.e. number of scan intervals which must have the same state (1 ... 50) */
#define CY_BUTTON_DEBOUNCE_COUNT        3

/** \} group_mw_button_macros */

/**
* \addtogroup group_mw_button_enums
* \{
*/

typedef enum
{
    CY_BUTTON_SUCCESS   = 0x00u, /**< Returned successful */
    CY_BUTTON_ERROR     = 0x01u, /**< General error */
    CY_BUTTON_BAD_PARAM = 0x02u, /**< Bad parameter was passed */
} cy_button_en_result_t;



/**
 *****************************************************************************
 ** \brief Button states used
 *****************************************************************************/

typedef enum
{
    StateInvalid,
    StateLow,
    StateHigh
} cy_button_en_state_t;

/** \} group_mw_button_enums */

/**
* \addtogroup group_mw_button_data_structures
* \{
*/

/**
 ** function pointer type to button callback function
 ** \param u8ButtonId  ID of button that changed its state (BUTTON_ID)
 ** \param enState      new state of button
 ** \return none */
typedef void (*cy_button_callback_t)(uint8_t u8ButtonId, cy_button_en_state_t enState);


/**
 *****************************************************************************
 ** \brief User definition of a button (user specific ID, port number, pin number)
 **
 ** This struct is used for initialization of the button module
 *****************************************************************************/
typedef struct
{
    volatile stc_GPIO_PRT_t*    pstcPort;
    uint8_t                     u8Pin;
    uint8_t                     u8ButtonId;
} cy_button_stc_definition_t;


/** \} group_mw_button_data_structures */

/***************************************
*       Function Prototypes
***************************************/

/**
* \addtogroup group_mw_button_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_Button_Init
****************************************************************************//**
*
* \brief Initializes the Button driver
*
* \param pstcButtonDef
* Button definition
*
* \param u8NrOfButtons
* Button number
*
* \param pfnCallback
* Callback
*
* \param u8CySwTimerId
* SW timer ID
*
* \return 
* button status based on cy_button_en_result_t
*
*******************************************************************************/
cy_button_en_result_t  Cy_Button_Init(const cy_button_stc_definition_t* pstcButtonDef, 
	uint8_t u8NrOfButtons, cy_button_callback_t pfnCallback, uint8_t u8CySwTimerId);


/*******************************************************************************
* Function Name: Cy_Button_GetCurrentButtonState
****************************************************************************//**
*
* \brief Returns the state of the desired button
*
* \param u8ButtonId 
*
* \return 
*  button state based on cy_button_en_result_t
*
*******************************************************************************/
cy_button_en_state_t   Cy_Button_GetCurrentButtonState(uint8_t u8ButtonId);


/** \} group_mw_button_functions */


#if defined(__cplusplus)
}
#endif

#endif /* CY_BUTTON_H */

/** \} group_mw_button */

/* [] END OF FILE */
