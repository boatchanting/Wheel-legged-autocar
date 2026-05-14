/***************************************************************************//**
* \file ErrorCalloutHandler.h
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

#if !defined(ERRORCALLOUTHANDLER_H)
#define ERRORCALLOUTHANDLER_H


/*==================[global variable declarations]===========================*/

extern uint32 Ech_Counter;

/*==================[external function definitions]=========================*/

extern void ErrorCalloutHandler(
   uint16 ModuleId,
   uint8  InstanceId,
   uint8  ApiId,
   uint8  ErrorId);

 
#endif /* ERRORCALLOUTHANDLER_H */
/*==================[end of file]===========================================*/

