/***************************************************************************//**
* \file cy_button.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Button middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_button.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/


#if (CY_BUTTON_SCAN_INTERVAL_MS < 20)
    #error CY_BUTTON_SCAN_INTERVAL_MS value too low (< 20 ms)!
#endif
#if (CY_BUTTON_SCAN_INTERVAL_MS > 100)
    #error CY_BUTTON_SCAN_INTERVAL_MS value too high (> 100 ms)!
#endif

#if (CY_BUTTON_DEBOUNCE_COUNT < 1)
    #error CY_BUTTON_DEBOUNCE_COUNT value too low (< 1)!
#endif
#if (CY_BUTTON_DEBOUNCE_COUNT > 50)
    #error CY_BUTTON_DEBOUNCE_COUNT value too high (> 50)!
#endif



/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/


/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/


/**
 *****************************************************************************
 ** \brief Datatype for holding internal data needed for BUTTON
 **
 ** This struct is used to store ports and states of GPIOs configured.
 *****************************************************************************/
typedef struct
{
    cy_button_stc_definition_t  stcDef;
    uint8_t                     u8DebounceCounter;
    cy_button_en_state_t        enCurrState;
} stc_button_cfg_status_t;



/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/

static stc_button_cfg_status_t      m_astcButtonCfgStatus[CY_BUTTON_MAX_BUTTONS];
static cy_button_callback_t         m_pfnCallback = NULL;
static uint8_t                      m_u8NrOfButtons = 0;


/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/
static void ScanTimerCallback(void);


/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/


/**
 *****************************************************************************
 ** Initialize all assigned port pins and set up timer for scanning operations.
 **
 ** \param pfnCallback callback function pointer for button status update
 **
 ** \return  CY_BUTTON_SUCCESS    if buttons have been initialized successfully
 **          CY_BUTTON_BAD_PARAM  if:
 **                                 - pstcButtonDef == NULL
 **                                 - u8NrOfButtons == 0
 **                                 - u8NrOfButtons > CY_BUTTON_MAX_BUTTONS
 **          CY_BUTTON_ERROR      if Cy_SwTmr_Start() returns error
 *****************************************************************************/
cy_button_en_result_t Cy_Button_Init(const cy_button_stc_definition_t* pstcButtonDef, uint8_t u8NrOfButtons, cy_button_callback_t pfnCallback, uint8_t u8CySwTimerId)
{
    cy_button_en_result_t enButtonResult = CY_BUTTON_SUCCESS;
    cy_swtmr_en_result_t  enSwTmrResult;
    uint8_t u8ButtonIndex;
    
    if(pstcButtonDef == NULL)
    {
        return CY_BUTTON_BAD_PARAM;
    }
    if((u8NrOfButtons == 0) || (u8NrOfButtons > CY_BUTTON_MAX_BUTTONS))
    {
        return CY_BUTTON_BAD_PARAM;
    }
    
    for (u8ButtonIndex = 0; u8ButtonIndex < u8NrOfButtons; u8ButtonIndex++)
    {
        m_astcButtonCfgStatus[u8ButtonIndex].stcDef.pstcPort   = pstcButtonDef[u8ButtonIndex].pstcPort;
        m_astcButtonCfgStatus[u8ButtonIndex].stcDef.u8Pin      = pstcButtonDef[u8ButtonIndex].u8Pin;
        m_astcButtonCfgStatus[u8ButtonIndex].stcDef.u8ButtonId = pstcButtonDef[u8ButtonIndex].u8ButtonId;
        m_astcButtonCfgStatus[u8ButtonIndex].u8DebounceCounter = CY_BUTTON_DEBOUNCE_COUNT - 1;
        m_astcButtonCfgStatus[u8ButtonIndex].enCurrState       = StateInvalid;
    }
    
    m_u8NrOfButtons = u8NrOfButtons;
    m_pfnCallback   = pfnCallback;
    
    // Start timer for debouncing buttons
    enSwTmrResult = Cy_SwTmr_Start(u8CySwTimerId, CY_BUTTON_SCAN_INTERVAL_MS, true, ScanTimerCallback);
    if(enSwTmrResult != CY_SWTMR_SUCCESS)
    {
        enButtonResult = CY_BUTTON_ERROR;
    }

    return enButtonResult;
}


/**
 *****************************************************************************
 ** Returns the current state of a button, independent of its debounced state.
 **
 ** \param u8ButtonId Button ID
 **
 ** \return (StateLow, StateHigh, StateInvalid)
 *****************************************************************************/
cy_button_en_state_t Cy_Button_GetCurrentState(uint8_t u8ButtonId)
{
    for (uint8_t u8ButtonIndex = 0; u8ButtonIndex < m_u8NrOfButtons; u8ButtonIndex++)
    {
        if(m_astcButtonCfgStatus[u8ButtonIndex].stcDef.u8ButtonId == u8ButtonId)
        {
            uint32_t u32ButtonLevel = Cy_GPIO_Read(m_astcButtonCfgStatus[u8ButtonId].stcDef.pstcPort, m_astcButtonCfgStatus[u8ButtonId].stcDef.u8Pin);
            return (u32ButtonLevel == 0) ? StateLow : StateHigh;
        }
    }
    
    return StateInvalid;
}

/**
 *****************************************************************************
 ** Callback function from timer module. Scan all buttons and debounce. Call
 ** callback function (if any).
 **
 ** \param none
 **
 ** \return none
 *****************************************************************************/
static void ScanTimerCallback(void)
{
    // Go through all configured (not max) buttons
    for (uint8_t u8ButtonIndex = 0; u8ButtonIndex < m_u8NrOfButtons; u8ButtonIndex++)
    {
        // Create short pointer for current button
        stc_button_cfg_status_t* pstcButton = &m_astcButtonCfgStatus[u8ButtonIndex];

        uint32_t u32ButtonLevel = Cy_GPIO_Read(pstcButton->stcDef.pstcPort, pstcButton->stcDef.u8Pin);
        
        cy_button_en_state_t enSampledState = (u32ButtonLevel == 0) ? StateLow : StateHigh;
    
        if (enSampledState != pstcButton->enCurrState)
        {
            if (pstcButton->u8DebounceCounter == 0)
            {
                // Update to new state
                pstcButton->enCurrState = enSampledState;
                // Call callback function (if any)
                if (m_pfnCallback != NULL)
                {
                    m_pfnCallback(pstcButton->stcDef.u8ButtonId, pstcButton->enCurrState);
                }
            }
            else
            {
                pstcButton->u8DebounceCounter--;
            }
        }
        else
        {
            // state equal -> restart debounce counter
            pstcButton->u8DebounceCounter = CY_BUTTON_DEBOUNCE_COUNT - 1;
        }
    }
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
