/***************************************************************************//**
* \file WdgIf_Types.h
*
* \brief
* WdgIf_Types stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if (!defined WDGIF_TYPES_H)
#define WDGIF_TYPES_H


/*==================[inclusions]=============================================*/

#include <Std_Types.h>          /* standard types */

/*==================[macros]=================================================*/

/*==================[type definitions]=======================================*/

/** \brief Enumeration type of WDG Interface Status Type
 **
 ** This enumeration type holds the status types that are used internally by
 ** the underlying watchdog driver(s). */
typedef enum
{
    WDGIF_UNINIT=0,
    /**< default WDG IF status after reset */
    WDGIF_IDLE,
    /**< watchdog is currently idle */
    WDGIF_BUSY
    /**< watchdog is currently being switched between modes or triggerd */
} WdgIf_StatusType;

/** \brief Enumeration type of WDG Interface Mode Type
 **
 ** This enumeration type holds the mode types that are passed as parameters
 ** to Wdg_SetMode(). */
typedef enum
{
    WDGIF_OFF_MODE=0,
    /**< watchdog switched off */
    WDGIF_SLOW_MODE,
    /**< watchdog set for long timeout period */
    WDGIF_FAST_MODE
    /**< watchdog set for short timeout period (fast triggering) */
} WdgIf_ModeType;

/*==================[external function declarations]=========================*/

/*==================[external constants]=====================================*/

/*==================[internal constants]=====================================*/

/*==================[external data]==========================================*/

/*==================[internal data]==========================================*/

/*==================[external function definitions]==========================*/

/*==================[internal function definitions]==========================*/

#endif /* if !defined( WDGIF_TYPES_H ) */
/*==================[end of file]============================================*/
