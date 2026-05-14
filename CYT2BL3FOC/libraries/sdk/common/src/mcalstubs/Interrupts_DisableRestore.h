/***************************************************************************//**
* \file Interrupts_DisableRestore.h
*
* \brief
* Implementation of interrupt disabling and restoring as used within SchM stubs
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(INTERRUPTS_DISABLERESTORE_H)
#define INTERRUPTS_DISABLERESTORE_H


/*==================[inclusions]=============================================*/

#include "Std_Types.h"

/*==================[type definitions]=======================================*/

typedef uint32 TestEnv_IntStatusType;

/*==================[macros]=================================================*/

/*------------------[TestEnv_IntDisable]-----------------------------------------*/

#if (defined TestEnv_IntDisable) /* guard to prevent double definition */
#error "TestEnv_IntDisable already defined"
#endif /* if (defined TestEnv_IntDisable) */

/** \brief Disables interrupts and returns previous state.
 **
 ** This macro disables all interrupts by clearing the IE flag in the
 ** processor status word. The previous contents of the IE in PS is returned.
 **
 ** ATTENTION: This function MUST NOT be called within MCAL code, but by
 **            the Schedule Manager (i.e. in function SchM_Enter_xxx()).
 **/

#define TestEnv_IntDisable()         Cy_SysLib_EnterCriticalSection()
TestEnv_IntStatusType Cy_SysLib_EnterCriticalSection(void);

/*------------------[TestEnv_IntRestore]-----------------------------------------*/

#if (defined TestEnv_IntRestore) /* guard to prevent double definition */
#error "TestEnv_IntRestore already defined"
#endif /* if (defined TestEnv_IntRestore) */

/** \brief Restores interrupt lock state
 **
 ** This macro restores the interrupt locking status to a state
 ** as defined by the parameter. This should be the value as 
 ** returned by the previous call to TestEnv_IntDisable().
 **
 ** ATTENTION: This function MUST NOT be called within MCAL code, but by
 **            the Schedule Manager (i.e. in function SchM_Exit_xxx()).
 **/

#define TestEnv_IntRestore(s)        Cy_SysLib_ExitCriticalSection(s)
void Cy_SysLib_ExitCriticalSection(TestEnv_IntStatusType IEstatus);


#endif /* INTERRUPTS_DISABLERESTORE_H */
/*==================[end of file]============================================*/
