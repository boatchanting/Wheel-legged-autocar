#include "cy_srom.h"

static un_srom_api_scrach_sram_t g_scratch __ALIGNED(32); // This must locate on SRAM and align with cache line size (in case device has cache)
static un_srom_api_args_2_t      g_scratch2 __ALIGNED(32); // This must locate on SRAM and align with cache line size (in case device has cache)
static cy_srom_handler gp_srom_resp_handler = NULL;

// Dummy var that might be needed as workaround
static uint32_t g_dummy = 0ul;


static void Cy_Srom_ISR_ResponseIPC(void);

/*******************************************************************************
* Function Name: Cy_Srom_CallApi
****************************************************************************//**
*
* This function calls SROM API. CPU will wait for API completed inside this
* function by checking IPC released. Even after checking 0x00FFFFFF times, if it 
* is still not released, this function will return timeout status.
* If IPC was busy when this function was called, it would return busy status
* immediately.
* If SROM API returned error status, this function would return error status.
* But actual message is still in first word of memory pointed by second input
* parameter "resp" (if not NULL).
* All messages will be passed via SRAM pointed to scratch address. The SRAM area
* is reserved in this file as a variable named "g_scratch".
* This function is not re-entrant.
*
* \param params
* Pointer to memory which contains SROM API arguments. The contents of arguments
* will be copied to "g_scratch". \ref un_srom_api_args_t
*
* \param resp
* Pointer to memory which SROM API response message will be copied to. Even if 
* this function return error status, response message will be copied to the
* memory. If pointer is NULL, response will not be copied \ref un_srom_api_resps_t
*
* \return
* \ref cy_en_srom_driver_status_t
*
*******************************************************************************/
cy_en_srom_driver_status_t Cy_Srom_CallApi(const un_srom_api_args_t* params, un_srom_api_resps_t* resp)
{
    if(params == NULL)
    {
        return CY_SROM_DR_INVALID_INPUT;
    }

    volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];
    uint32_t timeOut = 0x0FFFFFFFul;

    // Copy input contents to SRAM reserved
    g_scratch.args = *params;

    // Make Bit[0] to 0, it indicates all arguments are passed through scratch address
    g_scratch.u32[0] = g_scratch.u32[0] & 0xFFFFFFFEul;

    // Clean scratch RAM in case current core has a D-Cache, so that CM0+ SROM handler reads correct data
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch, sizeof(g_scratch));

    // Send a message by IPC
    // Workaround: Because some SROM APIs read memory area pointed by IPC_DATA1 even if
    // IPC_DATA0[0] == 0 (indicating the API arguments are passed through SRAM scratch),
    // it's safe to set dummy readable address to the IPC_DATA1.
    if (Cy_IPC_Drv_SendMsgWord_2(syscall_ipc_struct, CY_SROM_DR_IPC_NOTIFY_STRUCT, (uint32_t)&g_scratch, (uint32_t)&g_dummy) != CY_IPC_DRV_SUCCESS)
    {
        // The IPC structure is already locked by another process
        return CY_SROM_DR_IPC_BUSY;
    }

    while(1)
    {
        // Checks if the IPC structure is not locked
        if (Cy_IPC_Drv_IsLockAcquired(syscall_ipc_struct) == false)
        {
            // The result of SROM API calling is in the SRAM reserved.
            // Copy the contents to the memory pointed by input pointer
            if(resp != NULL)
            {
                // Invalidate scratch RAM in case current core has a D-Cache, so that updates from CM0+ SROM handler are read correctly
                Cy_SysLib_InvalidateCoreDCacheByAddr(&g_scratch, sizeof(g_scratch));
                *resp = g_scratch.resps;
            }
            break;
        }

        // Decrement time out counter value
        timeOut--;

        // If time out counter is "0" return time out status
        if(timeOut == 0)
        {
            return CY_SROM_DR_TIMEOUT;
        }
    }

    if(Cy_Srom_GetResponseType() == CY_SROM_RESPONSE_FAIL)
    {
        // If the SROM API returned an error in the scratch RAM, return error
        return CY_SROM_DR_API_ERROR;
    }
    else if(Cy_Srom_GetResponseType() == CY_SROM_RESPONSE_SUCCESS)
    {
        // If the SROM API returned "OK", return success
        return CY_SROM_DR_SUCCEEDED;
    }
    else
    {
        // Return unknown status
        return CY_SROM_DR_API_UNKNOWN;
    }
}

/*******************************************************************************
* Function Name: Cy_Srom_CallApi_NonBlock
****************************************************************************//**
*
* This function calls SROM API. CPU "won't" wait for API completed inside this
* function by checking IPC released. 
* If IPC was busy when this function was called, it would return busy status
* immediately.
* Please call "Cy_Srom_Get_Api_Response" to retrieve API response. 
* All messages will be passed via SRAM pointed to scratch address. The SRAM area
* is reserved in this file as a variable named "g_scratch".
* This function is not re-entrant.
* Once user called this function, this function and also "Cy_Srom_CallApi"
* must not called until SROM API finished.
*
* \param params
* Pointer to memory which contains SROM API arguments. The contents of arguments
* will be copied to "g_scratch". \ref un_srom_api_args_t
*
* \return
* \ref cy_en_srom_driver_status_t
*
*******************************************************************************/
cy_en_srom_driver_status_t Cy_Srom_CallApi_NonBlock(const un_srom_api_args_t* params)
{
    if(params == NULL)
    {
        return CY_SROM_DR_INVALID_INPUT;
    }

    volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];

    // Copy input contents to SRAM reserved
    g_scratch.args = *params;

    // Make Bit[0] to 0, it indicates all arguments are passed through scratch address
    g_scratch.u32[0] = g_scratch.u32[0] & 0xFFFFFFFEul;

    // Clean scratch RAM in case current core has a D-Cache, so that CM0+ SROM handler reads correct data
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch, sizeof(g_scratch));

    // Send a message by IPC
    // Workaround: Because some SROM APIs read memory area pointed by IPC_DATA1 even if
    // IPC_DATA0[0] == 0 (indicating the API arguments are passed through SRAM scratch),
    // it's safe to set dummy readable address to the IPC_DATA1.
    if (Cy_IPC_Drv_SendMsgWord_2(syscall_ipc_struct, CY_SROM_DR_IPC_NOTIFY_STRUCT, (uint32_t)&g_scratch, (uint32_t)&g_dummy) != CY_IPC_DRV_SUCCESS)
    {
        // The IPC structure is already locked by another process
        return CY_SROM_DR_IPC_BUSY;
    }

    // Return succeeded status
    return CY_SROM_DR_SUCCEEDED;
}

/*******************************************************************************
* Function Name: Cy_Srom_CallApi_2
****************************************************************************//**
*
* This function calls SROM API which need also IPC DATA1. Other is same as 
* "Cy_SROM_CallApi"
*
* \param params
* Pointer to memory which contains SROM API arguments. The contents of arguments
* will be copied to "g_scratch". \ref un_srom_api_args_t
*
* \param params
* Pointer to memory which contains SROM API arguments 2. The contents of arguments
* will be copied to "g_scratch2". \ref un_srom_api_args_2_t
*
* \param resp
* Pointer to memory which SROM API response message will be copied to. Even if 
* this function return error status, response message will be copied to the
* memory. If pointer is NULL, response will not be copied \ref un_srom_api_resps_t
*
* \return
* \ref cy_en_srom_driver_status_t
*
*******************************************************************************/
cy_en_srom_driver_status_t Cy_Srom_CallApi_2(const un_srom_api_args_t* params, const un_srom_api_args_2_t* params2, un_srom_api_resps_t* resp)
{
    if((params == NULL) || (params2 == NULL))
    {
        return CY_SROM_DR_INVALID_INPUT;
    }

    volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];
    uint32_t timeOut = 0x00FFFFFFul;

    // Copy input contents to SRAM reserved
    g_scratch.args = *params;
    g_scratch2     = *params2;

    // Make Bit[0] to 0, it indicates all arguments are passed through scratch address
    g_scratch.u32[0]  = g_scratch.u32[0]  & 0xFFFFFFFEul;
    g_scratch2.arg[0] = g_scratch2.arg[0] & 0xFFFFFFFEul;

    // Clean scratch RAM in case current core has a D-Cache, so that CM0+ SROM handler reads correct data
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch,  sizeof(g_scratch));
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch2, sizeof(g_scratch2));

    // Send message by IPC
    if (Cy_IPC_Drv_SendMsgWord_2(syscall_ipc_struct, CY_SROM_DR_IPC_NOTIFY_STRUCT, (uint32_t)&g_scratch, (uint32_t)&g_scratch2) != CY_IPC_DRV_SUCCESS)
    {
        // The IPC structure is already locked by another process
        return CY_SROM_DR_IPC_BUSY;
    }

    while(1)
    {
        // Checks if the IPC structure is not locked
        if (Cy_IPC_Drv_IsLockAcquired(syscall_ipc_struct) == false)
        {
            // The result of SROM API calling is in the SRAM reserved.
            // Copy the contents to the memory pointed by input pointer
            if(resp != NULL)
            {                
                // Invalidate scratch RAM in case current core has a D-Cache, so that updates from CM0+ SROM handler are read correctly
                Cy_SysLib_InvalidateCoreDCacheByAddr(&g_scratch, sizeof(g_scratch));
                *resp = g_scratch.resps;
            }
            break;
        }

        // Decrement time out counter value
        timeOut--;

        // If time out counter is "0" return time out status
        if(timeOut == 0)
        {
            return CY_SROM_DR_TIMEOUT;
        }
    }

    if(Cy_Srom_GetResponseType() == CY_SROM_RESPONSE_FAIL)
    {
        // If the SROM API returned an error in the scratch RAM, return error
        return CY_SROM_DR_API_ERROR;
    }
    else if(Cy_Srom_GetResponseType() == CY_SROM_RESPONSE_SUCCESS)
    {
        // If the SROM API returned "OK", return success
        return CY_SROM_DR_SUCCEEDED;
    }
    else
    {
        // Return unknown status
        return CY_SROM_DR_API_UNKNOWN;
    }
}

/*******************************************************************************
* Function Name: Cy_Srom_CallApi_NonBlock_2
****************************************************************************//**
*
* This function calls SROM API which need also IPC DATA1. Other is same as 
* "Cy_SROM_CallApi_NonBlock_2"
*
* \param params
* Pointer to memory which contains SROM API arguments. The contents of arguments
* will be copied to "g_scratch". \ref un_srom_api_args_t
*
* \param params
* Pointer to memory which contains SROM API arguments 2. The contents of arguments
* will be copied to "g_scratch2". \ref un_srom_api_args_2_t
*
* \return
* \ref cy_en_srom_driver_status_t
*
*******************************************************************************/
cy_en_srom_driver_status_t Cy_Srom_CallApi_NonBlock_2(const un_srom_api_args_t* params, const un_srom_api_args_2_t* params2)
{
    if((params == NULL) || (params2 == NULL))
    {
        return CY_SROM_DR_INVALID_INPUT;
    }

    volatile stc_IPC_STRUCT_t* syscall_ipc_struct = &IPC->STRUCT[CY_IPC_CHAN_SYSCALL];

    // Copy input contents to SRAM reserved
    g_scratch.args = *params;
    g_scratch2     = *params2;

    // Make Bit[0] to 0, it indicates all arguments are passed through scratch address
    g_scratch.u32[0]  = g_scratch.u32[0]  & 0xFFFFFFFEul;
    g_scratch2.arg[0] = g_scratch2.arg[0] & 0xFFFFFFFEul;

    // Clean scratch RAM in case current core has a D-Cache, so that CM0+ SROM handler reads correct data
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch,  sizeof(g_scratch));
    Cy_SysLib_CleanCoreDCacheByAddr(&g_scratch2, sizeof(g_scratch2));

    // Send message by IPC
    if (Cy_IPC_Drv_SendMsgWord_2(syscall_ipc_struct, CY_SROM_DR_IPC_NOTIFY_STRUCT, (uint32_t)&g_scratch, (uint32_t)&g_scratch2) != CY_IPC_DRV_SUCCESS)
    {
        // The IPC structure is already locked by another process
        return CY_SROM_DR_IPC_BUSY;
    }

    // Return succeeded status
    return CY_SROM_DR_SUCCEEDED;
}

/*******************************************************************************
* Function Name: Cy_Srom_GetResponseType
****************************************************************************//**
*
* This function returns status of SROM API response defined by \ref 
* cy_en_srom_response_type_t
* Please call this function after "Cy_Srom_CallApi_NonBlock" was called.
*
* \return
* \ref cy_en_srom_response_type_t
*
*******************************************************************************/
cy_en_srom_response_type_t Cy_Srom_GetResponseType(void)
{
    // Invalidate scratch RAM in case current core has a D-Cache, so that updates from CM0+ SROM handler are read correctly
    Cy_SysLib_InvalidateCoreDCacheByAddr(&g_scratch, sizeof(g_scratch));

    if((g_scratch.u32[0] & 0xF0000000ul) == 0xA0000000ul)
    {
        return CY_SROM_RESPONSE_SUCCESS;
    }
    else if((g_scratch.u32[0] & 0xF0000000ul) == 0xF0000000ul)
    {
        return CY_SROM_RESPONSE_FAIL;
    }
    else
    {
        return CY_SROM_NOT_RESPONSE;
    }
}

/*******************************************************************************
* Function Name: Cy_Srom_Get_Api_Response
****************************************************************************//**
*
* This function returns g_scratch value as a response of SROM API. 
* Please call this function after "Cy_Srom_CallApi_NonBlock" was called.
* "resp" is raw response data from SROM API on the other hands, return value 
* indicate status of the response data like as invalid, succeeded, or failed.
* (if failed return value is same as resp.u32[0])
*
* \param resp
* Pointer to memory into which SROM API response to be copied if not NULL.
*  \ref un_srom_api_resps_t
*
* \return
* \ref cy_en_srom_api_status_t
*
*******************************************************************************/
cy_en_srom_api_status_t Cy_Srom_GetApiResponse(un_srom_api_resps_t* resp)
{
    cy_en_srom_response_type_t type = Cy_Srom_GetResponseType();
    if(type == CY_SROM_NOT_RESPONSE)
    {
        return CY_SROM_STATUS_INVALID;
    }
    else
    {
        if(resp != NULL)
        {
            *resp = g_scratch.resps;
        }

        if(type == CY_SROM_RESPONSE_SUCCESS)
        {
            return CY_SROM_STATUS_SUCCESS;
        }
        else // CY_SROM_RESPONSE_FAIL
        {
            // return error code.
            return (cy_en_srom_api_status_t)g_scratch.u32[0];
        }
    }
}

/*******************************************************************************
* Function Name: Cy_Srom_ConvertRespToStatus
****************************************************************************//**
*
* This function returns g_scratch value as a response of SROM API. 
* Please call this function after "Cy_Srom_CallApi_NonBlock" was called.
* "resp" is raw response data from SROM API on the other hands, return value 
* indicate status of the response data like as invalid, succeeded, or failed.
* (if failed return value is same as resp.u32[0])
*
* \param resp
* Pointer to memory into which SROM API response to be copied if not NULL.
*  \ref un_srom_api_resps_t
*
* \return
* \ref cy_en_srom_api_status_t
*
*******************************************************************************/
cy_en_srom_api_status_t Cy_Srom_ConvertRespToStatus(uint32_t resp0)
{
    if((resp0 & 0xF0000000ul) == 0xA0000000ul)
    {
        return CY_SROM_STATUS_SUCCESS;
    }
    else if((resp0 & 0xF0000000ul) == 0xF0000000ul)
    {
        // Error Code
        return (cy_en_srom_api_status_t)resp0;
    }
    else
    {
        return CY_SROM_STATUS_INVALID;
    }
}

/*******************************************************************************
* Function Name: Cy_Srom_SetResponseHandler
****************************************************************************//**
*
* This function sets response handler which is supposed to be called when SROM
* API finished. This function does not set NVIC. NVIC settings including enabling
* interrupt,  setting priority of interrupt, is a responsibility of the user
* code.
*
* \param handler
* This will be invoked when SROM API finished and the IPC released.
*
* \param intIdx
* Interrupt number of the ARM core. \ref IRQn_Type
*
*******************************************************************************/
void Cy_Srom_SetResponseHandler(cy_srom_handler handler, IRQn_Type intIdx)
{
    cy_stc_sysint_irq_t irq_cfg =
    {
        .sysIntSrc  = CY_SROM_DR_IPC_INTR_NO,
        .intIdx     = intIdx,
        .isEnabled  = true,
    };

    /* set response handler */
    gp_srom_resp_handler = handler;

    /*  Set CPUSS interrupt    */
    Cy_SysInt_InitIRQ(&irq_cfg);
    Cy_SysInt_SetSystemIrqVector(irq_cfg.sysIntSrc, Cy_Srom_ISR_ResponseIPC);

    /*  Set IPC interrupt mask    */
    volatile stc_IPC_INTR_STRUCT_t* sromRespIntrStr = Cy_IPC_Drv_GetIntrBaseAddr(CY_SROM_DR_IPC_INTR_STRUCT);
    Cy_IPC_Drv_SetInterruptMask(sromRespIntrStr, (1u << CY_IPC_CHAN_SYSCALL), 0);
}

/*******************************************************************************
* Function Cy_Srom_ISR_ResponseIPC
****************************************************************************//**
*
* This is interrupt handler called when SROM API was finished by CM0+ via IPC
* This function will be assigned in "Cy_Srom_SetResponseHandler".
*
*******************************************************************************/
static void Cy_Srom_ISR_ResponseIPC(void)
{
    uint32_t masked = 0;

    volatile stc_IPC_INTR_STRUCT_t* sromRespIntrStr = Cy_IPC_Drv_GetIntrBaseAddr(CY_SROM_DR_IPC_INTR_STRUCT);
    masked = Cy_IPC_Drv_GetInterruptStatusMasked(sromRespIntrStr);

    if((masked & (1ul << CY_IPC_CHAN_SYSCALL)) != 0ul)
    {
        if(gp_srom_resp_handler != NULL)
        {
            gp_srom_resp_handler();
        }
    }

    Cy_IPC_Drv_ClearInterrupt(
                                sromRespIntrStr,
                                (masked & 0x0000FFFFul),
                                (masked & 0xFFFF0000ul) >> 16ul
                              );
}

