/***************************************************************************//**
* \file cy_sw_tmr.c
* \version 1.0
*
* \brief
* Provides an API implementation of the Software Timer (SwTmr) middleware driver
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
#include "cy_sw_tmr.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/
// Check min/max values of CY_SWTMR_TICK_INTERVAL_US
#if ((CY_SWTMR_TICK_INTERVAL_US) < 20)
    #error CY_SWTMR_TICK_INTERVAL_US out of range (must be >= 20 us)!
#endif
#if ((CY_SWTMR_TICK_INTERVAL_US) > 1000)
    #error CY_SWTMR_TICK_INTERVAL_US out of range (must be <= 1000 us)!
#endif


#define SYSTICK_RELOAD_VALUE       (((((CY_CLK_IMO_FREQ_HZ) / 1000) * (CY_SWTMR_TICK_INTERVAL_US)) / 1000) - 1)



/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/


/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/


/**
 *****************************************************************************
 ** \brief Holds configuration and status of a single SW timer
 ** 
 *****************************************************************************/
typedef struct
{
    volatile int32_t      i32Timer;
    int32_t               i32Reload;
    cy_sw_tmr_callback_t  pfnCallback;
    volatile bool         bElapsed;
    bool                  bHighPrio;
} stc_sw_tmr_cfg_status_t;


/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/

static stc_sw_tmr_cfg_status_t m_astcTimers[CY_SWTMR_MAX_TIMERS];
static volatile uint8_t m_u8CallbackFunctionTrigger;
static volatile uint64_t m_u64TickCountUs;


/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/
static void StartTimer(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback);

static void CallbackFromSysTick(void);

/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/


/**
 *****************************************************************************
 ** Initializes the HW timer used as SW Timer source (SysTick)
 **
 ** The global tick counter (returned by Cy_SwTmr_GetTickCountUs()) will be reset
 ** to 0.
 **
 ** \return none
 *****************************************************************************/
void Cy_SwTmr_Init( void )
{
    // Reset tick counter
    m_u64TickCountUs = 0;
    
    Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_IMO, SYSTICK_RELOAD_VALUE);
    Cy_SysTick_SetCallback(0, CallbackFromSysTick);
    
}


/**
 *****************************************************************************
 ** Disables the HW timer used as SW Timer source (SysTick)
 **
 ** \return none
 *****************************************************************************/
 void Cy_SwTmr_Disable( void )
{
    Cy_SysTick_Disable();   
}

/**
 *****************************************************************************
 ** Starts a SW timer, indexed by an ID.
 ** To use the callback function, it is mandatory to
 ** cyclically call the Cy_SwTmr_Main() function. The callback
 ** functions are not called directly out of the ISR!
 **
 ** \param u8TimerID   0 to (CY_SWTMR_MAX_TIMERS - 1)
 ** \param i32TimeMs   Duration in ms (must be greater than or equal 0)
 ** \param bReload     Reload the timer automatically
 ** \param pfnCallback Callback function (when timer is elapsed)
 **
 ** \retval CY_SWTMR_SUCCESS    Timer started
 ** \retval CY_SWTMR_BAD_PARAM  u8TimerID invalid or i32TimeMs invalid
 *****************************************************************************/
cy_swtmr_en_result_t Cy_SwTmr_Start(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback)
{
    if (u8TimerID >= CY_SWTMR_MAX_TIMERS)
    {
        return CY_SWTMR_BAD_PARAM;
    }
    if (i32TimeMs < 0)
    {
        return CY_SWTMR_BAD_PARAM;        
    }
    
    m_astcTimers[u8TimerID].bHighPrio = false;
    StartTimer(u8TimerID, i32TimeMs, bReload, pfnCallback);
    
    return CY_SWTMR_SUCCESS;
}

/**
 *****************************************************************************
 ** Starts a (high-prio) SW timer, indexed by an ID.
 ** The callback functions will be called directly out of the ISR!
 ** For this timer type it is NOT necessary to cyclically call Cy_SwTmr_Main()
 **
 ** \param u8TimerID   0 to (CY_SWTMR_MAX_TIMERS - 1)
 ** \param u32TimeMs   Duration in ms
 ** \param bReload     Reload the timer automatically
 ** \param pfnCallback Callback function (when timer is elapsed)
 **
 ** \retval CY_SWTMR_SUCCESS    Timer started
 ** \retval CY_SWTMR_BAD_PARAM  u8TimerID invalid or i32TimeMs invalid
 *****************************************************************************/
cy_swtmr_en_result_t Cy_SwTmr_StartHighPrio(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback)
{
    if (u8TimerID >= CY_SWTMR_MAX_TIMERS)
    {
        return CY_SWTMR_BAD_PARAM;
    }
    if (i32TimeMs < 0)
    {
        return CY_SWTMR_BAD_PARAM;        
    }    

    m_astcTimers[u8TimerID].bHighPrio = true;
    StartTimer(u8TimerID, i32TimeMs, bReload, pfnCallback);
		
	return CY_SWTMR_SUCCESS;    
}

/**
 *****************************************************************************
 ** Starts a SW timer, indexed by an ID.
 ** (common part for Cy_SwTmr_Start and Cy_SwTmr_StartHighPrio)
 **
 ** \param u8TimerID   0 to (CY_SWTMR_MAX_TIMERS - 1)
 ** \param i32TimeMs   Duration in ms (must be greater than or equal 0)
 ** \param bReload     Reload the timer automatically
 ** \param pfnCallback Callback function (when timer is elapsed)
 **
 *****************************************************************************/
static void StartTimer(uint8_t u8TimerID, int32_t i32TimeMs, bool bReload, cy_sw_tmr_callback_t pfnCallback)
{
    // Re-calculate timer value for current tick interval
    int32_t i32ConvertedTime = (i32TimeMs * 1000) / (CY_SWTMR_TICK_INTERVAL_US);
    
    m_astcTimers[u8TimerID].i32Timer    = i32ConvertedTime;
    m_astcTimers[u8TimerID].i32Reload   = (bReload == true) ? i32ConvertedTime : -1;
    m_astcTimers[u8TimerID].bElapsed    = false;
    m_astcTimers[u8TimerID].pfnCallback = pfnCallback;
}

/**
 *****************************************************************************
 ** Stops a SW timer, indexed by an ID.
 **
 ** \param u8TimerID   0 to (CY_SWTMR_MAX_TIMERS - 1)
 **
 ** \retval CY_SWTMR_SUCCESS    Timer stopped
 ** \retval CY_SWTMR_BAD_PARAM  u8TimerID invalid
 *****************************************************************************/
cy_swtmr_en_result_t Cy_SwTmr_Stop(uint8_t u8TimerID)
{
    if (u8TimerID >= CY_SWTMR_MAX_TIMERS)
    {
        return CY_SWTMR_BAD_PARAM;
    }
    
    m_astcTimers[u8TimerID].i32Timer    = -1;
    m_astcTimers[u8TimerID].i32Reload   = -1;
    m_astcTimers[u8TimerID].bElapsed    = false;
    m_astcTimers[u8TimerID].pfnCallback = NULL;
    
    return CY_SWTMR_SUCCESS;
}

/**
 *****************************************************************************
 ** Function must be called cyclically to use callback functions of
 ** non-high-prio SW timers. If there are no callback functions in use, or only
 ** high-prio timers are used, there is no need to call Cy_SwTmr_Main().
 **
 ** \return none
 *****************************************************************************/
void Cy_SwTmr_Main( void )
{
    static uint8_t u8PrevTrigger = 0;
    uint8_t u8Trigger = m_u8CallbackFunctionTrigger;

    // If no function is to call
    if (u8PrevTrigger == u8Trigger)
    {
        return;
    }
    else
    {
        u8PrevTrigger = u8Trigger;
    }
    
    // Look for callback functions to call
    for (uint8_t u8TimerId = 0; u8TimerId < CY_SWTMR_MAX_TIMERS; u8TimerId++)
    {
        // If this timer is elapsed and it is a "standard" timer
        if ((m_astcTimers[u8TimerId].bElapsed == true) && (m_astcTimers[u8TimerId].bHighPrio == false))
        {
            // Call function
            if (m_astcTimers[u8TimerId].pfnCallback != NULL)
            {
                // Clear "Elapsed" flag before a possible long function call
                // (so it may be set again while calling)
                m_astcTimers[u8TimerId].bElapsed = false;
                // Call function
                m_astcTimers[u8TimerId].pfnCallback() ;
            }
        }
    }
}


/**
 *****************************************************************************
 ** Returns the remaining time of a timer.
 **
 ** \param u8TimerID 0 to (CY_SWTMR_MAX_TIMERS - 1)
 **
 ** \return int32_t remaining time in milliseconds or "-1" for an invalid timer ID
 *****************************************************************************/
int32_t Cy_SwTmr_Remaining(uint8_t u8TimerID)
{
    if (u8TimerID < CY_SWTMR_MAX_TIMERS)
    {
        // Re-calculate from current tick interval to milliseconds
        return m_astcTimers[u8TimerID].i32Timer / (1000 / (CY_SWTMR_TICK_INTERVAL_US));
    }

    return -1;
}

/**
 *****************************************************************************
 ** Checks, whether a timer is elapsed or not.
 **
 ** \param u8TimerID          0 to (CY_SWTMR_MAX_TIMERS - 1)
 ** \param bClearElapsedFlag  clear elapsed flag yes/no <=> true/false
 **
 ** \return bool     timer is elapsed yes/no <=> true/false (or false for an invalid timer ID)
 *****************************************************************************/
bool Cy_SwTmr_IsElapsed(uint8_t u8TimerID, bool bClearElapsedFlag)
{
    bool bElapsed = false;

    if (u8TimerID < CY_SWTMR_MAX_TIMERS)
    {
        bElapsed = m_astcTimers[u8TimerID].bElapsed;
        // Before clearing, check the mirrored bElapsed, to avoid
        // clearing the real flag just in the case it was set in the meantime
        if ((bClearElapsedFlag == true) && (bElapsed == true))
        {
            m_astcTimers[u8TimerID].bElapsed = false;
        }
    }
    return bElapsed;
}

/**
 *****************************************************************************
 ** Starts a timer and waits synchronously until it is elapsed.
 **
 ** \param u8TimerID               0 to (CY_SWTMR_MAX_TIMERS - 1)
 ** \param i32TimeMs               Duration in ms (must be greater than or equal 0)
 ** \param pfnWatchdogCallout      Function to handle the watchdog, can be NULL
 **
 ** \retval CY_SWTMR_SUCCESS    Timer started
 ** \retval CY_SWTMR_BAD_PARAM  u8TimerID invalid or i32TimeMs invalid
 *****************************************************************************/
cy_swtmr_en_result_t Cy_SwTmr_Wait(uint8_t u8TimerID, int32_t i32TimeMs, cy_sw_tmr_wdg_handle_callout_t pfnWatchdogCallout)
{
    cy_swtmr_en_result_t enResult;
    
    enResult = Cy_SwTmr_Start(u8TimerID, i32TimeMs, false, NULL);
    
    if (enResult != CY_SWTMR_SUCCESS)
    {
        return enResult;
    }
    
    while(Cy_SwTmr_IsElapsed(u8TimerID, true) == false)
    {
        // Call watchdog handle function if configured
        if (pfnWatchdogCallout != NULL)
        {
            pfnWatchdogCallout();
        }
    }
    
    return CY_SWTMR_SUCCESS;
}

/**
 *****************************************************************************
 ** \brief Get the global tick count value.
 **
 ** The global tick count value will be reset by calling Cy_SwTmr_Init() and
 ** will go on counting infinitely.
 ** The resolution of this counter is CY_SWTMR_TICK_INTERVAL_US.
 **
 ** \return 64-bit tick count value in microseconds
 *****************************************************************************/
uint64_t Cy_SwTmr_GetTickCountUs(void)
{
    return m_u64TickCountUs;
}

/**
 *****************************************************************************
 ** \brief Check whether a certain time span is elapsed.
 **
 ** Call this function with a previously stored tick count value to determine
 ** whether a time span is elapsed or not.
 **
 ** This function can be used to realize easy timeout checks without using
 ** a dedicated timer ID.
 **
 ** The resolution of the time span is based on CY_SWTMR_TICK_INTERVAL_US.
 **
 ** \param u64TickCountUs   Tick count value previously determined by Cy_SwTmr_GetTickCountUs()
 ** \param u32TimeSpanUs    Time span in microseconds
 **
 ** \retval true    u32TimeSpanUs is elapsed since time stamp u64TickCountUs
 ** \retval false   u32TimeSpanUs is not elapsed since time stamp u64TickCountUs
 *****************************************************************************/
bool Cy_SwTmr_IsTimeSpanElapsed(uint64_t u64TickCountUs, uint32_t u32TimeSpanUs)
{
    bool bElapsed = false;
    uint64_t u64CurrentTickCount = Cy_SwTmr_GetTickCountUs();
    
    // If current time stamp is older or equal than u64TickCountUs
    if (u64CurrentTickCount >= u64TickCountUs)
    {
        // Determine elapsed time
        uint32_t u32ElapsedTimeUs = (uint32_t)((uint64_t)u64CurrentTickCount - (uint64_t)u64TickCountUs);
        // If elapsed time is greater or equal than time span, return true
        if (u32ElapsedTimeUs >= u32TimeSpanUs)
        {
            bElapsed = true;
        }
    }
    return bElapsed;
}

/**
 *****************************************************************************
 ** Callback function from SysTick driver
 **
 ** \return none
 *****************************************************************************/
static void CallbackFromSysTick(void)
{
    // Increment global tick counter
    m_u64TickCountUs += CY_SWTMR_TICK_INTERVAL_US;
    
    // Countdown all running timers
    for (uint8_t u8TimerId = 0; u8TimerId < CY_SWTMR_MAX_TIMERS; u8TimerId++)
    {
        // If timer is disabled, continue
        if (m_astcTimers[u8TimerId].i32Timer < 0)
        {
            continue;
        }
            
        if (m_astcTimers[u8TimerId].i32Timer > 0)
        {
            m_astcTimers[u8TimerId].i32Timer--;
        }
        
        if (m_astcTimers[u8TimerId].i32Timer == 0)
        {
            // Set marker for processing in Cy_SwTmr_Main()
            if(m_astcTimers[u8TimerId].bHighPrio == false)
            {
                m_astcTimers[u8TimerId].bElapsed = true;
                m_u8CallbackFunctionTrigger++;
            }
            else
            {
                // Call user callback directly from IRQ context
                if (m_astcTimers[u8TimerId].pfnCallback != NULL)
                {
                    m_astcTimers[u8TimerId].pfnCallback();
                }
                m_astcTimers[u8TimerId].bElapsed = false;
            }

            // Check, whether to reload the timer or not
            if (m_astcTimers[u8TimerId].i32Reload > 0)
            {
                m_astcTimers[u8TimerId].i32Timer = m_astcTimers[u8TimerId].i32Reload;
            }
            else
            {
                m_astcTimers[u8TimerId].i32Timer = -1;    // disable timer (single-shot mode)
            }
        }
    }

}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
