/***************************************************************************//**
* \file Det.h
*
* \brief
* Det stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(DET_H)
#define DET_H


/*==================[inclusions]============================================*/

#include <Std_Types.h>                            /* AUTOSAR standard types */

/*==================[macros]=========================*/

#define DET_AR_RELEASE_MAJOR_VERSION 4
#define DET_AR_RELEASE_MINOR_VERSION 0
 
/*==================[type definitions]======================================*/


/*==================[external function declarations]=========================*/

extern Std_ReturnType Det_ReportError(
   uint16 ModuleId,
   uint8  InstanceId,
   uint8  ApiId,
   uint8  ErrorId);

extern Std_ReturnType Det_ReportRuntimeError(
   uint16 ModuleId,
   uint8  InstanceId,
   uint8  ApiId,
   uint8  ErrorId);

 
#endif /* DET_H */
/*==================[end of file]===========================================*/

