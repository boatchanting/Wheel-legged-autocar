/***************************************************************************//**
* \file SchM_Port.h
*
* \brief
* SchM_Port stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(SCHM_PORT_H)
#define SCHM_PORT_H

/*==================[inclusions]============================================*/
#include <Std_Types.h>     /* global configuration */
#include <SchM_Common.h>

/*==================[macros]================================================*/

/*==================[external variable definitions]=========================*/
SCHM_DECLARE_EXCLUSIVE_AREA_VAR(Port)

/*==================[external function definitions]=========================*/
#define SCHM_START_SEC_CODE
#include <SchM_MemMap.h>

SCHM_DECLARE_EXCLUSIVE_AREA_FUNC(Port, PORT, 0)

#define SCHM_STOP_SEC_CODE
#include <SchM_MemMap.h>
/*==================[internal function definitions]=========================*/

#endif /* SCHM_PORT_H */
/*==================[end of file]===========================================*/
