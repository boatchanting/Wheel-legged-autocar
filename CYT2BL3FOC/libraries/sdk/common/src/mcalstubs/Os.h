/***************************************************************************//**
* \file Os.h
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

#if !defined(OS_H)
#define OS_H
 
/*==================[inclusions]============================================*/

#include "Std_Types.h"

/*==================[macros]==============================================*/

#define OsCounter_0 (CounterType)0

#if !defined(ISR) /* to prevent double definition */
    #define ISR(x)     void  x(void) /* MISRA-1 */ /* PRQA S 3453 1 */ /* macros used like functions. */
#endif /* if (defined ISR) */


/*==================[type definitions]======================================*/

typedef uint32 TickType;
typedef TickType* TickRefType;
typedef uint8 CounterType;

/*==================[external function definitions]=========================*/

extern StatusType GetCounterValue(CounterType CounterId, TickRefType Value);
extern StatusType GetElapsedValue(CounterType CounterId, TickRefType Value, TickRefType ElapsedValue);


#endif /* OS_H */
/*==================[end of file]===========================================*/

