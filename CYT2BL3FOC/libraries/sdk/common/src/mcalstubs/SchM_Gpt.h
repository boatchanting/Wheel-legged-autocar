/***************************************************************************//**
* \file SchM_Gpt.h
*
* \brief
* SchM_Gpt stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(SCHM_GPT_H)
#define SCHM_GPT_H

/*==================[inclusions]============================================*/
#include <Std_Types.h>     /* global configuration */
#include <SchM_Common.h>

/*==================[macros]================================================*/

/*==================[external variable definitions]=========================*/
SCHM_DECLARE_EXCLUSIVE_AREA_VAR(Gpt)

/*==================[external function definitions]=========================*/
#define SCHM_START_SEC_CODE
#include <SchM_MemMap.h>

SCHM_DECLARE_EXCLUSIVE_AREA_FUNC(Gpt, GPT, 0)

#define SCHM_STOP_SEC_CODE
#include <SchM_MemMap.h>
/*==================[internal function definitions]=========================*/

#endif /* SCHM_GPT_H */
/*==================[end of file]===========================================*/
