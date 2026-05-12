/***************************************************************************//**
* \file cy_ipc_pipe.c
* \version 1.10
*
*  Description:
*   IPC Pipe Driver - This source file includes code for the Pipe layer on top
*   of the IPC driver.
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ipc_pipe.h"


/**
* This is the definition of the configuration and status of a pipe endpoint. 
* There is one endpoint structure for each CPU in a pipe.  It contains all
* the information to process a message send to other CPUs in the pipe.
*/
typedef struct
{
    uint32_t                         ipcChan;                   /**< IPC channel number used for this endpoint to receive messages             */
    uint32_t                         intrChan;                  /**< IPC interrupt channel number used for this endpoint to receive interrupts */
    uint32_t                         pipeIntMask;               /**< Release/Notify interrupt mask that includes all endpoints on pipe         */
    cy_en_intr_t                     pipeIntrSrc;               /**< System interrupt number that includes all endpoints on pipe               */
    volatile stc_IPC_STRUCT_t      * ipcPtr;                    /**< Pointer to receive IPC channel ( If ipcPtr == NULL, cannot receive )      */
    volatile stc_IPC_INTR_STRUCT_t * ipcIntrPtr;                /**< Pointer to IPC interrupt, needed to clear the interrupt                   */
    uint32_t                         busy;                      /**< Endpoint busy flag.  If set no messages can be sent from this endpoint    */
    cy_ipc_pipe_relcallback_ptr_t    releaseCallbackPtr;        /**< Pointer to release callback function                                      */
    cy_ipc_pipe_relcallback_ptr_t    defaultReleaseCallbackPtr; /**< Pointer to default release callback function                              */
} cy_stc_ipc_pipe_internal_cfg_status_t;


/** Create an array of endpoint structures */
static cy_stc_ipc_pipe_internal_cfg_status_t  cy_ipc_pipe_epCfgStatus[CY_IPC_PIPE_MAX_ENDPOINTS];

/** Remember this CPU's endpoint */
static uint8_t cy_ipc_pipe_epForThisCpu;

/** Create an array of callback functions */
static cy_ipc_pipe_callback_ptr_t cy_ipc_pipe_callbackArray[CY_IPC_PIPE_MAX_CLIENTS];

/** Remember "driver initialized" state, for checking in the various APIs */
static bool cy_ipc_pipe_drvInitialized = false;

static void InitEndpoint(uint32_t epAddr, cy_stc_ipc_pipe_ep_config_t const * epConfig, uint32_t intMask, cy_en_intr_t epSysInt);
static void ExecCallback(cy_stc_ipc_pipe_internal_cfg_status_t * endpoint);


/*******************************************************************************
* Function Name: Cy_IPC_Pipe_Init
****************************************************************************//**
*
* Initializes the system pipes. 
* \note The function should be called on all CPUs.
*
* After this function is called, the callback array needs to be populated
* with the callback functions for this endpoint using the
* Cy_IPC_Pipe_RegisterCallback() function.
*
* \param config
* This is the pointer to the pipe configuration structure
*
*******************************************************************************/
void Cy_IPC_Pipe_Init(cy_stc_ipc_pipe_config_t const * config)
{
    cy_stc_sysint_irq_t  irqCfg;
    uint32_t             ipcIrqMaskSuperset = 0;
    
    // Store index of this CPU's endpoint
    cy_ipc_pipe_epForThisCpu = config->epIndexForThisCpu;

    // Prepare IRQ configuration
    irqCfg.sysIntSrc = (cy_en_intr_t) CY_IPC_INTR_NUM_TO_VECT(config->epConfigData[cy_ipc_pipe_epForThisCpu].ipcIntrStructNr);
    irqCfg.intIdx    = config->epConfigData[cy_ipc_pipe_epForThisCpu].ipcCpuIntIdx;
    irqCfg.isEnabled = true;
    
    // Calculate IRQ Mask (superset of all endpoints' IPC structure number)
    for(uint8_t curEndpoint = 0; curEndpoint < CY_IPC_PIPE_MAX_ENDPOINTS; curEndpoint++)
    {
        ipcIrqMaskSuperset |= (0x1ul << config->epConfigData[curEndpoint].ipcStructNr);
    }
    
    // Initialize the internal config/status for all endpoints
    for(uint8_t curEndpoint = 0; curEndpoint < CY_IPC_PIPE_MAX_ENDPOINTS; curEndpoint++)
    {
        cy_en_intr_t sys_int = (curEndpoint == cy_ipc_pipe_epForThisCpu) ? irqCfg.sysIntSrc : disconnected_IRQn;
            
        InitEndpoint(curEndpoint, &config->epConfigData[curEndpoint], ipcIrqMaskSuperset, sys_int);
    }

    // Configure interrupt
    (void)Cy_SysInt_InitIRQ(&irqCfg);
    Cy_SysInt_SetSystemIrqVector(irqCfg.sysIntSrc, Cy_IPC_Pipe_Isr);
    
    cy_ipc_pipe_drvInitialized = true;
}

/*******************************************************************************
* Function Name: InitEndpoint
****************************************************************************//**
*
* This function initializes an endpoint of a pipe for the current CPU. It needs
* to be called for all endpoints in a pipe so that their respective config/status
* structs are populated with the necessary data.
*
* \param epAddr
* This parameter is the address (or index in the array of endpoint structures)
* that designates the endpoint you want to initialize.
*
* \param epConfig
* Configuration of this endpoint
*
* \param intMask
* Superset bitmask of all IPC channels used by all endpoints
*
* \param epSysInt
* This is the system interrupt number of the IPC interrupt structure used by this endpoint.
*
*******************************************************************************/
static void InitEndpoint(uint32_t epAddr, cy_stc_ipc_pipe_ep_config_t const * epConfig, uint32_t intMask, cy_en_intr_t epSysInt)
{
    cy_stc_ipc_pipe_internal_cfg_status_t * endpoint;

    endpoint = &cy_ipc_pipe_epCfgStatus[epAddr];

    endpoint->ipcChan     = epConfig->ipcStructNr;
    endpoint->intrChan    = epConfig->ipcIntrStructNr;
    endpoint->pipeIntMask = intMask;

    /* Assign IPC channel to this endpoint */
    endpoint->ipcPtr = Cy_IPC_Drv_GetIpcBaseAddress(endpoint->ipcChan);

    /* Assign interrupt structure to endpoint and Initialize the interrupt mask for this endpoint */
    endpoint->ipcIntrPtr = Cy_IPC_Drv_GetIntrBaseAddr(endpoint->intrChan);

    /* Only allow notify and release interrupts from endpoints in this pipe. */
    Cy_IPC_Drv_SetInterruptMask(endpoint->ipcIntrPtr, endpoint->pipeIntMask, endpoint->pipeIntMask);

    /* Save the Client count and the callback array pointer */
    endpoint->busy = CY_IPC_PIPE_ENDPOINT_NOTBUSY;

    endpoint->pipeIntrSrc = epSysInt;
}


/*******************************************************************************
* Function Name: Cy_IPC_Pipe_SendMessage
****************************************************************************//**
*
* This function is used to send a message from one endpoint to another.  It
* generates an interrupt on the endpoint that receives the message and a
* release interrupt to the sender to acknowledge the message has been processed.
*
* \param toEpIndex
* This parameter is the address (or index in the array of endpoint structures)
* of the endpoint to which you are sending the message.
*
* \param msgPtr
* Pointer to the message structure to be sent.
*
* \param callBackPtr
* Pointer to the Release callback function.
*
*  \return
*    CY_IPC_PIPE_SUCCESS:          Message was sent to the other end of the pipe
*    CY_IPC_PIPE_ERROR_BAD_HANDLE: The handle provided for the pipe was not valid
*    CY_IPC_PIPE_ERROR_SEND_BUSY:  The pipe is already busy sending a message
*    CY_IPC_PIPE_ERROR_UNINIT:     Driver init function has not yet been called
*
*******************************************************************************/
cy_en_ipc_pipe_status_t Cy_IPC_Pipe_SendMessage(uint32_t toEpIndex, void * msgPtr, cy_ipc_pipe_relcallback_ptr_t callBackPtr)
{
    cy_en_ipc_pipe_status_t  returnStatus;
    uint32_t releaseMask;
    uint32_t notifyMask;

    cy_stc_ipc_pipe_internal_cfg_status_t * fromEp;
    cy_stc_ipc_pipe_internal_cfg_status_t * toEp;
    
    if(cy_ipc_pipe_drvInitialized == false)
    {
        return CY_IPC_PIPE_ERROR_UNINIT;
    }

    toEp   = &cy_ipc_pipe_epCfgStatus[toEpIndex];
    fromEp = &cy_ipc_pipe_epCfgStatus[cy_ipc_pipe_epForThisCpu];

    /* Create the release mask for the "fromEp" channel's interrupt channel */
    releaseMask =  (uint32_t)(1ul << (fromEp->intrChan));

    /* Shift into position */
    releaseMask = _VAL2FLD(CY_IPC_PIPE_MSG_RELEASE, releaseMask);

    /* Create the notify mask for the "toEp" channel's interrupt channel */
    notifyMask  =  (uint32_t)(1ul << (toEp->intrChan));

    /* Check if IPC channel valid */
    if( toEp->ipcPtr != (void *)0u)
    {
        if(fromEp->busy == CY_IPC_PIPE_ENDPOINT_NOTBUSY)
        {
            /* Attempt to acquire the channel */
            if( CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_LockAcquire(toEp->ipcPtr) )
            {
                __DSB();
                
                /* Mask out the release mask area */
                *((uint32_t *) msgPtr) &= ~(CY_IPC_PIPE_MSG_RELEASE_Msk);

                *((uint32_t *) msgPtr) |= releaseMask;

                /* If the channel was acquired, write the message.   */
                Cy_IPC_Drv_WriteDataValue(toEp->ipcPtr, (uint32_t) msgPtr);

                /* Set the busy flag.  The ISR clears this after the release */
                fromEp->busy = CY_IPC_PIPE_ENDPOINT_BUSY;

                /* Setup release callback function */
                fromEp->releaseCallbackPtr = callBackPtr;
                
                __DSB();

                /* Cause notify event/interrupt */
                Cy_IPC_Drv_AcquireNotify(toEp->ipcPtr, notifyMask);

                returnStatus = CY_IPC_PIPE_SUCCESS;
            }
            else
            {
                /* Channel was already acquired, return Error */
                returnStatus = CY_IPC_PIPE_ERROR_SEND_BUSY;
            }
        }
        else
        {
            /* Channel may not be acquired, but the release interrupt has not executed yet */
            returnStatus = CY_IPC_PIPE_ERROR_SEND_BUSY;
        }
    }
    else
    {
        /* Null pipe handle. */
        returnStatus = CY_IPC_PIPE_ERROR_BAD_HANDLE;
    }
    return (returnStatus);
}



/*******************************************************************************
* Function Name: Cy_IPC_Pipe_RegisterCallback
****************************************************************************//**
*
* This function registers a callback that is called when a message is received on a pipe.
* The client_ID is the same as the index of the callback function array.  The callback
* may be a real function pointer or NULL if no callback is required.
*
* \param callBackPtr
* Pointer to the callback function called when the endpoint has received a message.
*
* \param clientId
* The index in the callback array (Client ID) where the function pointer is saved.
*
*  \return
*    CY_IPC_PIPE_SUCCESS:           Callback registered successfully
*    CY_IPC_PIPE_ERROR_BAD_CLIENT:  Client ID out of range, callback not registered.
*******************************************************************************/
cy_en_ipc_pipe_status_t Cy_IPC_Pipe_RegisterCallback(cy_ipc_pipe_callback_ptr_t callBackPtr,  uint32_t clientId)
{
    cy_en_ipc_pipe_status_t returnStatus;

    /* Check if clientId is between 0 and less than client count */
    if (clientId < CY_IPC_PIPE_MAX_CLIENTS)
    {
        /* Copy callback function into callback function pointer array */
        cy_ipc_pipe_callbackArray[clientId] = callBackPtr;

        returnStatus = CY_IPC_PIPE_SUCCESS;
    }
    else
    {
        returnStatus = CY_IPC_PIPE_ERROR_BAD_CLIENT;
    }
    
    return (returnStatus);
}

/*******************************************************************************
* Function Name: Cy_IPC_Pipe_RegisterCallbackRel
****************************************************************************//**
*
* This function registers a default callback if a release interrupt
* is generated but the current release callback function is null.
*
*
* \param callBackPtr
* Pointer to the callback executed when the endpoint has received a message.
*
*  \return
*    CY_IPC_PIPE_SUCCESS:          Callback registered successfully
*    CY_IPC_PIPE_ERROR_UNINIT:     Driver init function has not yet been called
*******************************************************************************/
cy_en_ipc_pipe_status_t Cy_IPC_Pipe_RegisterCallbackRel(cy_ipc_pipe_relcallback_ptr_t callBackPtr)
{
    cy_stc_ipc_pipe_internal_cfg_status_t * endpoint;
    
    if(cy_ipc_pipe_drvInitialized == false)
    {
        return CY_IPC_PIPE_ERROR_UNINIT;
    }

    endpoint = &cy_ipc_pipe_epCfgStatus[cy_ipc_pipe_epForThisCpu];

    /* Copy callback function into callback function pointer array */
    endpoint->defaultReleaseCallbackPtr = callBackPtr;
    
    return CY_IPC_PIPE_SUCCESS;
}

/*******************************************************************************
* Function Name: Cy_IPC_Pipe_Isr
****************************************************************************//**
*
* This is the interrupt service routine for the pipe.
*
*******************************************************************************/
void Cy_IPC_Pipe_Isr(void)
{
    if(cy_ipc_pipe_drvInitialized != false)
    {
        ExecCallback(&cy_ipc_pipe_epCfgStatus[cy_ipc_pipe_epForThisCpu]);
    }
}

/*******************************************************************************
* Function Name: ExecCallback
****************************************************************************//**
*
* This function is called by the ISR for a given pipe endpoint to dispatch
* the appropriate callback function based on the client ID for that endpoint.
*
* \param endpoint
* Pointer to endpoint structure.
*
*  \return
*  None
*******************************************************************************/
static void ExecCallback(cy_stc_ipc_pipe_internal_cfg_status_t * endpoint)
{
    uint32_t *msgPtr = (void *)0uL;
    uint32_t clientID;
    uint32_t shadowIntr;
    uint32_t releaseMask = (uint32_t)0;

    cy_ipc_pipe_callback_ptr_t callbackPtr;

    /* Check to make sure the interrupt was a notify interrupt */
    shadowIntr = Cy_IPC_Drv_GetInterruptStatusMasked(endpoint->ipcIntrPtr);

    if( 0ul != (shadowIntr & IPC_INTR_STRUCT_INTR_MASK_NOTIFY_Msk))
    {
        /* Clear the notify interrupt.  */
        endpoint->ipcIntrPtr->unINTR.u32Register = (shadowIntr & IPC_INTR_STRUCT_INTR_MASK_NOTIFY_Msk);

        if( Cy_IPC_Drv_IsLockAcquired (endpoint->ipcPtr) )
        {
            __DSB();
            
            /* Extract Client ID  */
            if( CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_ReadMsgPtr (endpoint->ipcPtr, (void **)&msgPtr))
            {
                /* Get release mask */
                releaseMask = _FLD2VAL(CY_IPC_PIPE_MSG_RELEASE, *msgPtr);
                clientID    = _FLD2VAL(CY_IPC_PIPE_MSG_CLIENT,  *msgPtr);

                /* Make sure client ID is within valid range */
                if (clientID < CY_IPC_PIPE_MAX_CLIENTS)
                {
                    callbackPtr = cy_ipc_pipe_callbackArray[clientID];  /* Get the callback function */

                    if (callbackPtr != NULL)
                    {
                        callbackPtr(msgPtr);   /* Call the function pointer for "clientID" */
                    }
                }
            }
            
            __DSB();

            /* Must always release the IPC channel */
            (void)Cy_IPC_Drv_LockRelease (endpoint->ipcPtr, releaseMask);
        }
    }

    /* Check to make sure the interrupt was a release interrupt */
    if( 0ul != (shadowIntr & IPC_INTR_STRUCT_INTR_MASK_RELEASE_Msk))  /* Check for a Release interrupt */
    {
        /* Clear the release interrupt  */
        endpoint->ipcIntrPtr->unINTR.u32Register = (shadowIntr & IPC_INTR_STRUCT_INTR_MASK_RELEASE_Msk);

        if (endpoint->releaseCallbackPtr != NULL)
        {
            endpoint->releaseCallbackPtr();

            /* Clear the pointer after it was called */
            endpoint->releaseCallbackPtr = NULL;
        }
        else
        {
            if (endpoint->defaultReleaseCallbackPtr != NULL)
            {
                endpoint->defaultReleaseCallbackPtr();
            }
        }

        /* Clear the busy flag when release is detected */
        endpoint->busy = CY_IPC_PIPE_ENDPOINT_NOTBUSY;
    }

    (void)Cy_IPC_Drv_GetInterruptStatus(endpoint->ipcIntrPtr);
}

/*******************************************************************************
* Function Name: Cy_IPC_Pipe_EndpointPause
****************************************************************************//**
*
* This function sets the receiver endpoint to paused state.
*
*
*  \return
*    CY_IPC_PIPE_SUCCESS:          Endpoint paused successfully
*    CY_IPC_PIPE_ERROR_UNINIT:     Driver init function has not yet been called
*
*******************************************************************************/
cy_en_ipc_pipe_status_t Cy_IPC_Pipe_EndpointPause(void)
{
    cy_stc_ipc_pipe_internal_cfg_status_t * endpoint;
    
    if(cy_ipc_pipe_drvInitialized == false)
    {
        return CY_IPC_PIPE_ERROR_UNINIT;
    }

    endpoint = &cy_ipc_pipe_epCfgStatus[cy_ipc_pipe_epForThisCpu];

    /* Disable the interrupts */
    Cy_SysInt_DisableIRQ(endpoint->pipeIntrSrc);

    return (CY_IPC_PIPE_SUCCESS);
}

/*******************************************************************************
* Function Name: Cy_IPC_Pipe_EndpointResume
****************************************************************************//**
*
* This function sets the receiver endpoint to active state.
*
*
*  \return
*    CY_IPC_PIPE_SUCCESS:          Endpoint resumed successfully
*    CY_IPC_PIPE_ERROR_UNINIT:     Driver init function has not yet been called
*
*******************************************************************************/
cy_en_ipc_pipe_status_t Cy_IPC_Pipe_EndpointResume(void)
{
    cy_stc_ipc_pipe_internal_cfg_status_t * endpoint;
    
    if(cy_ipc_pipe_drvInitialized == false)
    {
        return CY_IPC_PIPE_ERROR_UNINIT;
    }

    endpoint = &cy_ipc_pipe_epCfgStatus[cy_ipc_pipe_epForThisCpu];

    /* Enable the interrupts */
    Cy_SysInt_EnableIRQ(endpoint->pipeIntrSrc);

    return (CY_IPC_PIPE_SUCCESS);
}


/* [] END OF FILE */

