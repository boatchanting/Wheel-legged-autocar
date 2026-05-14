/***************************************************************************//**
* \file MemIf_Types.h
*
* \brief
* MemIf_Types stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(MEMIF_TYPES_H)
#define MEMIF_TYPES_H

/*==================[inclusions]============================================*/

#include <Std_Types.h>          /* Standard AUTOSAR Types */

/*==================[macros]================================================*/

/*==================[type definitions]======================================*/

/** \brief Type definition of the MemIf status type
 **
 ** This type denotes the current status of the underlying abstraction module
 ** and device driver. It shall be used as the return value of the
 ** corresponding driver's "GetStatus" function. */
typedef enum
{
  MEMIF_UNINIT,          /** The underlying abstraction module or device
                          ** driver has not been initialized (yet)*/
  MEMIF_IDLE,            /** The underlying abstraction module or device
                          ** driver is currently idle. */
  MEMIF_BUSY,            /** The underlying abstraction module or device
                          ** driver is currently busy. */
  MEMIF_BUSY_INTERNAL    /** The underlying abstraction module is busy with
                          ** internal management operations. The underlying
                          ** device driver can be busy or idle. */
} MemIf_StatusType;

/* \brief Type definition of the MemIf job result type
**
** This type denotes the result of the last job. */
typedef enum
{
  MEMIF_JOB_OK,           /** The job has been finished successfully. */
  MEMIF_JOB_FAILED,       /** The job has not been finished successfully. */
  MEMIF_JOB_PENDING,      /** The job has not yet been finished. */
  MEMIF_JOB_CANCELED,     /** The job has been cancelled. */
  MEMIF_BLOCK_INCONSISTENT,   /** The requested block is inconsistent,
                               ** it may contain corrupted data. */
  MEMIF_BLOCK_INVALID     /** The requested block has been marked as invalid,
                           ** the requested operation can not be performed. */
} MemIf_JobResultType;

/* \brief Type definition of the MemIf mode type
**
** This type denotes the status / result of the last job. */
typedef enum
{
  MEMIF_MODE_SLOW,        /** The underlying memory abstraction modules
                           ** and drivers are working in slow mode. */
  MEMIF_MODE_FAST         /** The underlying memory abstraction modules
                           ** and drivers are working in fast mode. */
} MemIf_ModeType;


#endif /* MEMIF_TYPES_H */
/*==================[end of file]===========================================*/
