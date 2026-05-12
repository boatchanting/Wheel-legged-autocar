/***************************************************************************//**
* \file Os.c
*
* \brief
* Os stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*==================[inclusions]============================================*/

#include "Os.h"

/*==================[local variables]=======================================*/

#define OS_START_SEC_VAR_UNSPECIFIED
#include "Os_MemMap.h"

static TickType CurrentTick = 0;

#define OS_STOP_SEC_VAR_UNSPECIFIED
#include "Os_MemMap.h"

/*==================[external function definitions]=========================*/

#define OS_START_SEC_CODE
#include "Os_MemMap.h"

FUNC(StatusType,SCHM_CODE)  GetCounterValue
( 
  CounterType CounterID, 
  TickRefType Value 
)
{
  if (OsCounter_0 == CounterID)
  {
    *Value = CurrentTick++;
    return E_OK;
  }
  else
  {
    return E_NOT_OK;
  }
}


FUNC(StatusType, SCHM_CODE) GetElapsedValue
(
  CounterType CounterID, 
  TickRefType Value, 
  TickRefType ElapsedValue
)
{
  if (OsCounter_0 == CounterID)
  {
    if (CurrentTick >= *Value)
    {
      *ElapsedValue = CurrentTick - *Value;
    }
    else
    {
      *ElapsedValue = CurrentTick + (0xFFFFFFFFu - *Value) + 1u;
    }
    *Value = CurrentTick++;
    return E_OK;
  }
  else
  {
    return E_NOT_OK;
  }
}

#define OS_STOP_SEC_CODE
#include "Os_MemMap.h"

/*==================[internal function definitions]=========================*/

/*==================[end of file]===========================================*/
