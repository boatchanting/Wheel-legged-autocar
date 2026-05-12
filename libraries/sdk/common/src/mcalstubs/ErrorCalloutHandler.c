/***************************************************************************//**
* \file ErrorCalloutHandler.c
*
* \brief
* ErrorCalloutHandler stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*==================[inclusions]============================================*/

#include <Std_Types.h>
#include <stdio.h>

/*==================[global variable definitions]===========================*/

uint32 Ech_Counter = 0u;

/*==================[external function definitions]=========================*/

FUNC(void, DET_CODE) ErrorCalloutHandler(
   uint16 ModuleId,
   uint8  InstanceId,
   uint8  ApiId,
   uint8  ErrorId)
{
  Ech_Counter++;
  #ifdef CY_MCAL_STUBS_ERROR_CALLOUT_ENABLE_PRINTF
  printf("ErrorCalloutHandler: Module %d, Instance %d, API %d, Error %d\n", ModuleId, InstanceId, ApiId, ErrorId);
  #endif
}


