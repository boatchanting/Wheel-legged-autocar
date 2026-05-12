/***************************************************************************//**
* \file EcuM_Cbk.h
*
* \brief
* EcuM_Cbk stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(ECUM_CBK_H)
#define ECUM_CBK_H

/*==================[inclusions]============================================*/

#include <EcuM.h>

/*==================[macros]=========================*/

/* defines that would be generated into EcuM_Cfg.h */
#define EcuMWakeupSource_0 0x20U
#define EcuMWakeupSource_1 0x40U
#define EcuMWakeupSource_2 0x80U
 
/*==================[type definitions]======================================*/

typedef uint32 EcuM_WakeupSourceType;

/*==================[external function declarations]=========================*/

void EcuM_CheckWakeup (EcuM_WakeupSourceType wakeupSource);
void EcuM_SetWakeupEvent (EcuM_WakeupSourceType wakeupSource);


#endif /* ECUM_CBK_H */
/*==================[end of file]===========================================*/
