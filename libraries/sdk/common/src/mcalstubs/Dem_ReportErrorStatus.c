/***************************************************************************//**
* \file Dem_ReportErrorStatus.c
*
* \brief
* Dem_ReportErrorStatus stub implementation
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
#include "Dem.h"

/*==================[global variable definitions]===========================*/

#define DEM_START_SEC_VAR_UNSPECIFIED
#include <Dem_MemMap.h>

uint32 Dem_Counter = 0u;
Dem_EventIdType Dem_EventId = DEM_EVENT_PARAMETER_UNDEFINED;
Dem_EventStatusType Dem_EventStatus = DEM_EVENT_STATUS_UNDEFINED;

#define DEM_STOP_SEC_VAR_UNSPECIFIED
#include <Dem_MemMap.h>

/*==================[external function definitions]=========================*/

#define DEM_START_SEC_CODE
#include "Dem_MemMap.h"
	
FUNC(Std_ReturnType, DEM_CODE) Dem_ReportErrorStatus(
    Dem_EventIdType EventId,
    Dem_EventStatusType EventStatus)
{
  switch (EventStatus)
  {
    case DEM_EVENT_STATUS_FAILED:
      #ifdef CY_MCAL_STUBS_DEM_FAILED_ENABLE_PRINTF
      printf("Dem_ReportErrorStatus: EventId %d, DEM_EVENT_STATUS_FAILED\n", EventId);
      #endif
      break;
    case DEM_EVENT_STATUS_PASSED:
      #ifdef CY_MCAL_STUBS_DEM_PASSED_ENABLE_PRINTF
      printf("Dem_ReportErrorStatus: EventId %d, DEM_EVENT_STATUS_PASSED\n", EventId);
      #endif
      break;
    case DEM_EVENT_STATUS_PREFAILED:
      #ifdef CY_MCAL_STUBS_DEM_FAILED_ENABLE_PRINTF
      printf("Dem_ReportErrorStatus: EventId %d, DEM_EVENT_STATUS_PREFAILED\n", EventId);
      #endif
      break;
    case DEM_EVENT_STATUS_PREPASSED:
      #ifdef CY_MCAL_STUBS_DEM_PASSED_ENABLE_PRINTF
      printf("Dem_ReportErrorStatus: EventId %d, DEM_EVENT_STATUS_PREPASSED\n", EventId);
      #endif
      break;
    default:
      #if defined(CY_MCAL_STUBS_DEM_PASSED_ENABLE_PRINTF) || defined(CY_MCAL_STUBS_DEM_FAILED_ENABLE_PRINTF)
      printf("Dem_ReportErrorStatus: EventId %d, undefined EventStatus '%d'\n", EventId, EventStatus);
      #endif
      break;
  }
  Dem_Counter++;
  Dem_EventId = EventId;
  Dem_EventStatus = EventStatus;
  
  return E_OK;
}

#define DEM_STOP_SEC_CODE
#include "Dem_MemMap.h"

/*==================[internal function definitions]=========================*/

/*==================[end of file]===========================================*/
