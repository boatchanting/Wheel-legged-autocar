/***************************************************************************//**
* \file EcuM_Cbk.c
*
* \brief
* EcuM_Cbk stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*==================[inclusions]============================================*/

#include <Std_Types.h>                            /* AUTOSAR standard types */
#include <EcuM_Cbk.h>
#include <stdio.h>

/*==================[external function definitions]=========================*/


FUNC(void,ECUM_CODE) EcuM_CheckWakeup (
   EcuM_WakeupSourceType wakeupSource)
{
  #ifdef CY_MCAL_STUBS_ECUM_WAKEUP_ENABLE_PRINTF
  printf("\nEcuM_CheckWakeup(WakeupSource=0x%08x)\n", wakeupSource);
  #endif
}

FUNC(void,ECUM_CODE) EcuM_SetWakeupEvent (
   EcuM_WakeupSourceType wakeupSource)
{
  #ifdef CY_MCAL_STUBS_ECUM_WAKEUP_ENABLE_PRINTF
  printf("\nEcuM_SetWakeupEvent(WakeupSource=0x%08x)\n", wakeupSource);
  #endif
}

/*==================[internal function definitions]=========================*/

/*==================[end of file]===========================================*/
