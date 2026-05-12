/***************************************************************************//**
* \file LinIf_Cbk.c
*
* \brief
* LinIf_Cbk stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*==================[inclusions]============================================*/

#if (!defined LinIf_Cbk_H)
#define LinIf_Cbk_H

#include <Std_Types.h>                            /* AUTOSAR standard types */
#include <EcuM_Cbk.h>
#include <stdio.h>


/*==================[external function definitions]=========================*/

FUNC(void,LINIF_CODE) LinIf_WakeupConfirmation (
   EcuM_WakeupSourceType wakeupSource)
{
  #ifdef CY_MCAL_STUBS_LINIF_CBK_ENABLE_PRINTF
  printf("\nLinIf_WakeupConfirmation(WakeupSource=0x%08x)\n", wakeupSource);
  #endif
}

/*==================[internal function definitions]=========================*/

#endif /* !defined LinIf_Cbk_H */

/*==================[end of file]===========================================*/
