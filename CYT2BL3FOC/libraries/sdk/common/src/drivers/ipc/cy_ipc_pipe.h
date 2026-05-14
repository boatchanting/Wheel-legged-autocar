/***************************************************************************//**
* \file cy_ipc_pipe.h
* \version 1.10
*
*  Description:
*   IPC Pipe Driver - This header file contains all the function prototypes,
*   structure definitions, pipe constants, and pipe endpoint address definitions.
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#ifndef CY_IPC_PIPE_H
#define CY_IPC_PIPE_H

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "ipc/cy_ipc_drv.h"
#include "syslib/cy_syslib.h"
#include "sysint/cy_sysint.h"

/**
* \addtogroup group_ipc_pipe IPC pipes layer (IPC_PIPE)
* \{
* The Pipe functions provide a method to transfer one or more words of data
* between CPUs or tasks.  The data can be defined as a single 32-bit unsigned
* word, an array of data, or a user-defined structure.  The only limitation is
* that the first word in the array or structure must be a 32-bit unsigned word
* in which a client ID number is passed. The client ID dictates the callback
* function that will be called by the receiver of the message. After the
* callback function returns by the receiver, it will invoke a release callback
* function defined by the sender of the message.
*
* A User Pipe is provided for the user to transfer data between CPUs and
* tasks.
*
*     \defgroup group_ipc_pipe_macros Macros
*       Macro definitions are used in the driver
*
*     \defgroup group_ipc_pipe_functions Functions
*       Functions are used in the driver
*
*     \defgroup group_ipc_pipe_data_structures Data Structures
*       Data structures are used in the driver
*
*     \defgroup group_ipc_pipe_enums Enumerated Types
*       Enumerations are used in the driver
*
*/

/*
 * This section defines the system level constants required to define
 * callback arrays for the Cypress pipe and the user pipe.  These defines
 * are used for both the max callback count and maximum clients.
*/

/** Typedef for pipe callback function pointer */
typedef void (* cy_ipc_pipe_callback_ptr_t)(uint32_t * msgPtr);

/** Typedef for a pipe release callback function pointer */
typedef void (* cy_ipc_pipe_relcallback_ptr_t)(void);

/** Typedef for array of callback function pointers */
typedef void (** cy_ipc_pipe_callback_array_ptr_t)(uint32_t * msgPtr);


/**
* \addtogroup group_ipc_pipe_macros
* \{
*/


#define CY_IPC_PIPE_MSG_CLIENT_Msk     (0x000000FFul)   /**< Client mask for first word of Pipe message */
#define CY_IPC_PIPE_MSG_CLIENT_Pos     (0ul)            /**< Client shift for first word of Pipe message */
#define CY_IPC_PIPE_MSG_USR_Msk        (0x0000FF00ul)   /**< User data mask for first word of Pipe message */
#define CY_IPC_PIPE_MSG_USR_Pos        (8ul)            /**< User data shift for first word of Pipe message */
#define CY_IPC_PIPE_MSG_RELEASE_Msk    (0xFFFF0000ul)   /**< Mask for message release mask */
#define CY_IPC_PIPE_MSG_RELEASE_Pos    (16UL)           /**< Shift require to line up mask to LSb */

/** Use to set the busy flag when waiting for a release interrupt */
#define CY_IPC_PIPE_ENDPOINT_BUSY      (1UL)
/** Denotes that a release interrupt is not pending */
#define CY_IPC_PIPE_ENDPOINT_NOTBUSY   (0UL)

/** \} group_ipc_pipe_macros */

/**
* \addtogroup group_ipc_pipe_data_structures
* \{
*/



/** The Pipe endpoint configuration structure. */
typedef struct
{
    uint8_t     ipcStructNr;      /**< IPC Channel used by this endpoint */
    uint8_t     ipcIntrStructNr;  /**< IPC Interrupt Structure used by this endpoint */
    IRQn_Type   ipcCpuIntIdx;     /**< CPU interrupt index used by this endpoint */
} cy_stc_ipc_pipe_ep_config_t;

/** The Pipe configuration structure. */
typedef struct
{
    uint8_t                      epIndexForThisCpu;                          /**< The current CPU's endpoint index in the #epConfigData array */
    cy_stc_ipc_pipe_ep_config_t  epConfigData[CY_IPC_PIPE_MAX_ENDPOINTS];    /**< Configuration of all endpoints of this pipe (system-wide identical) */
} cy_stc_ipc_pipe_config_t;

/** \} goup_ipc_pipe_data_structures */

/**
* \addtogroup group_ipc_pipe_macros
* \{
*/
/* Status and error types */
#define CY_IPC_PIPE_RTN        (0x0200ul)                                       /**< Software PDL driver ID for IPC pipe functions */
#define CY_IPC_PIPE_ID_INFO    (uint32_t)( CY_IPC_ID_INFO    | CY_IPC_PIPE_RTN) /**< Return prefix for IPC pipe function status codes */
#define CY_IPC_PIPE_ID_WARNING (uint32_t)( CY_IPC_ID_WARNING | CY_IPC_PIPE_RTN) /**< Return prefix for IPC pipe function warning return values */
#define CY_IPC_PIPE_ID_ERROR   (uint32_t)( CY_IPC_ID_ERROR   | CY_IPC_PIPE_RTN) /**< Return prefix for IPC pipe function error return values */

/** \} group_ipc_pipe_macros */

/**
* \addtogroup group_ipc_pipe_enums
* \{
*/

/** Return constants for IPC pipe functions. */
typedef enum
{
    CY_IPC_PIPE_SUCCESS            =(uint32_t)(0x00u),                       /**<  Pipe API return for no error */
    CY_IPC_PIPE_ERROR_NO_IPC       =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 1ul),  /**<  Pipe API return for no valid IPC channel */
    CY_IPC_PIPE_ERROR_NO_INTR      =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 2ul),  /**<  Pipe API return for no valid interrupt */
    CY_IPC_PIPE_ERROR_BAD_PRIORITY =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 3ul),  /**<  Pipe API return for bad priority parameter */
    CY_IPC_PIPE_ERROR_BAD_HANDLE   =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 4ul),  /**<  Pipe API return for bad pipe handle */
    CY_IPC_PIPE_ERROR_BAD_ID       =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 5ul),  /**<  Pipe API return for bad pipe ID */
    CY_IPC_PIPE_ERROR_DIR_ERROR    =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 6ul),  /**<  Pipe API return for invalid direction (Not used at this time) */
    CY_IPC_PIPE_ERROR_SEND_BUSY    =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 7ul),  /**<  Pipe API return for pipe is currently busy */
    CY_IPC_PIPE_ERROR_NO_MESSAGE   =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 8ul),  /**<  Pipe API return for no message indicated */
    CY_IPC_PIPE_ERROR_BAD_CPU      =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 9ul),  /**<  Pipe API return for invalid CPU value */
    CY_IPC_PIPE_ERROR_BAD_CLIENT   =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 10ul), /**<  Pipe API return for client out of range */
    CY_IPC_PIPE_ERROR_UNINIT       =(uint32_t)(CY_IPC_PIPE_ID_ERROR | 11ul), /**<  Pipe API return if driver has not been initialized yet*/
    
} cy_en_ipc_pipe_status_t;

/** \} group_ipc_pipe_enums */

/**
* \addtogroup group_ipc_pipe_data_structures
* \{
*/

/** \cond
*
* Client identifier for a message.
* For a given pipe, traffic across the pipe can be multiplexed with multiple
* senders on one end and multiple receivers on the other end.
*
* The first 32-bit word of the message is used to identify the client that owns
* the message.
*
* The upper 16 bits are the client ID.
*
* The lower 16 bits are for use by the client in any way desired.
*
* The lower 16 bits are preserved (not modified) and not interpreted in any way.
* \endcond
*/

/** \} group_ipc_pipe_data_structures */

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/

/**
* \addtogroup group_ipc_pipe_functions
* \{
*/

#ifdef __cplusplus
extern "C" {
#endif

void                     Cy_IPC_Pipe_Init(cy_stc_ipc_pipe_config_t const * config);
cy_en_ipc_pipe_status_t  Cy_IPC_Pipe_SendMessage(uint32_t toEpIndex, void * msgPtr, cy_ipc_pipe_relcallback_ptr_t callBackPtr);
cy_en_ipc_pipe_status_t  Cy_IPC_Pipe_RegisterCallback(cy_ipc_pipe_callback_ptr_t callBackPtr,  uint32_t clientId);
cy_en_ipc_pipe_status_t  Cy_IPC_Pipe_RegisterCallbackRel(cy_ipc_pipe_relcallback_ptr_t callBackPtr);
cy_en_ipc_pipe_status_t  Cy_IPC_Pipe_EndpointPause(void);
cy_en_ipc_pipe_status_t  Cy_IPC_Pipe_EndpointResume(void);
void                     Cy_IPC_Pipe_Isr(void);



#ifdef __cplusplus
}
#endif

/** \} group_ipc_pipe_functions */

#endif /* CY_IPC_PIPE_H  */

/* [] END OF FILE */
