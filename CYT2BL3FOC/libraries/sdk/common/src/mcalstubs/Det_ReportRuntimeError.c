/***************************************************************************//**
* \file Det_ReportRuntimeError.c
*
* \brief
* Det_ReportRuntimeError stub implementation
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
#include <stdio.h>

/*==================[external function definitions]=========================*/

#define DET_START_SEC_CODE
#include "Det_MemMap.h"

FUNC(Std_ReturnType, DET_CODE) Det_ReportRuntimeError(
   uint16 ModuleId,
   uint8  InstanceId,
   uint8  ApiId,
   uint8  ErrorId)
{
  #ifdef CY_MCAL_STUBS_DET_ENABLE_PRINTF
  printf("Det_ReportRuntimeError: Module %d, Instance %d, API %d, Error %d\n", ModuleId, InstanceId, ApiId, ErrorId);
  #endif
  
  return E_OK;
}


#define DET_STOP_SEC_CODE
#include "Det_MemMap.h"

/*==================[internal function definitions]=========================*/

/*==================[end of file]===========================================*/
