/***************************************************************************//**
* \file cy_syswdt.c
* \version 1.0
*
*  This file provides the source code to the API for the WDT driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "syswdt/cy_syswdt.h"
#include "syslib/cy_syslib.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_WDT_Init
****************************************************************************//**
*
* Initializes the Watchdog timer to its default state.
*
* The given default setting of the WDT:
*
* Before calling this function, WDT needs to be disable.
* This sets the upper limit and reset action.
*
*******************************************************************************/
void Cy_WDT_Init(void)
{
    uint32_t interruptState;
    interruptState = Cy_SysLib_EnterCriticalSection();

    /* Unlock the WDT registers by two writes */
    Cy_WDT_Unlock();

    /* Set upper limit = 32000 (1sec) */
    Cy_WDT_SetLowerLimit(0ul);
    Cy_WDT_SetUpperLimit(CY_WDT_DEFAULT_MATCH_VALUE);
    Cy_WDT_SetWarnLimit(0ul);
    Cy_WDT_SetLowerAction(CY_WDT_LOW_UPP_ACTION_NONE);
    Cy_WDT_SetUpperAction(CY_WDT_LOW_UPP_ACTION_RESET);
    Cy_WDT_SetWarnAction(CY_WDT_WARN_ACTION_NONE);

    /* Lock the WDT registers */
    Cy_WDT_Lock();

    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_WDT_Lock
****************************************************************************//**
*
* Locks out configuration changes to the Watchdog Timer register.
*
* After this function is called, the WDT configuration cannot be changed until 
* Cy_WDT_Unlock() is called.
*
*******************************************************************************/
void Cy_WDT_Lock(void)
{
    uint32_t interruptState;
    interruptState = Cy_SysLib_EnterCriticalSection();

    WDT->unLOCK.stcField.u2WDT_LOCK = 3ul;

    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_WDT_Unlock
****************************************************************************//**
*
* Unlocks the Watchdog Timer configuration register.
*
*******************************************************************************/
void Cy_WDT_Unlock(void)
{
    uint32_t interruptState;
    interruptState = Cy_SysLib_EnterCriticalSection();

    /* The WDT lock is to be removed by two writes */
    WDT->unLOCK.stcField.u2WDT_LOCK = 1ul;
    WDT->unLOCK.stcField.u2WDT_LOCK = 2ul;

    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_WDT_Enable
****************************************************************************//**
*
* Enables the Watchdog timer.
*
*******************************************************************************/
void Cy_WDT_Enable(void)
{
    Cy_WDT_Unlock();
    WDT->unCTL.stcField.u1ENABLE = 1ul;
    Cy_WDT_Lock();
}


/*******************************************************************************
* Function Name: Cy_WDT_Disable
****************************************************************************//**
*
* Disables the Watchdog timer. The Watchdog timer should be unlocked before being
* disabled. Call the Cy_WDT_Unlock() API to unlock the WDT.
*
*******************************************************************************/
void Cy_WDT_Disable(void)
{
    Cy_WDT_Unlock();
    WDT->unCTL.stcField.u1ENABLE = 0ul;
    Cy_WDT_Lock();
}


/*******************************************************************************
* Function Name: Cy_WDT_ClearInterrupt
****************************************************************************//**
*
* Clears the WDT match flag which is set every time the WDT counter reaches a 
* WDT match value. Two unserviced interrupts lead to a system reset 
* (i.e. at the third match).
*
*******************************************************************************/
void Cy_WDT_ClearInterrupt(void)
{
    WDT->unINTR.stcField.u1WDT = 1ul;

    /* Read the interrupt register to ensure that the initial clearing write has
    * been flushed out to the hardware.
    */
    (void) SRSS->unSRSS_INTR.u32Register;
}


/*******************************************************************************
* Function Name: Cy_WDT_ClearWatchdog
****************************************************************************//**
*
* Clears ("feeds") the watchdog, to prevent a XRES device reset.
* This function simply call Cy_WDT_ClearInterrupt() function.
*
*******************************************************************************/
void Cy_WDT_ClearWatchdog(void)
{
    Cy_WDT_Unlock();
    Cy_WDT_SetService();
    Cy_WDT_Lock();
}

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
