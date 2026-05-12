/***************************************************************************//**
* \file cy_ipc_drv.h
* \version 1.10
*
* Provides an API declaration of the IPC driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_IPC_DRV_H
#define CY_IPC_DRV_H

/**
* \defgroup group_ipc Inter Process Communication (IPC)
* \{
* Contains the driver API for the low-level IPC interface as well as Semaphores and
* Pipe upper-level interfaces.
*
* The IPC driver provides a safe and reliable method to transfer data between
* CPUs or processes in a single device. Hardware locking provides a reliable
* method to ensure that only one device can acquire and transfer data at one
* time so no data is lost or over written by asynchronous processes or CPUs.
*
* The Semaphores functions make use of a single IPC channel to allow multiple
* semaphores that can be used by system or user function calls. 
*
* The Pipe functions provide a method to transfer data between CPUs or tasks.
* The data can be defined as an array of data or a user defined structure.
* The only limitation is that the first element of the data must be a 32-bit
* unsigned word as service field in which a client ID number is passed.
* The client ID dictates the callback function that will be called by the
* receiver of the message. After the callback function is returned by the receiver,
* it invokes a release callback function defined by the sender of the
* message.
*
* \section group_ipc_configuration Configuration Considerations
*
* The usage of the IPC channels is partially fixed (system call channels for the
* various CPUs, DAP) other could be changed by the user
*
* \section group_ipc_more_information More Information
*
* See the technical reference manual(TRM) for more information on the IPC.
*
* \defgroup group_ipc_drv IPC driver layer (IPC_DRV)
* \defgroup group_ipc_sema   IPC semaphores layer (IPC_SEMA)
* \defgroup group_ipc_pipe   IPC pipes layer (IPC_PIPE)
* \} */

/**
* \addtogroup group_ipc_drv
* \{
*   The functions of this layer are used in the higher IPC levels
*   (Semaphores and Pipes).
*
*   \defgroup group_ipc_macros Macros
*       Macro definitions are used in the driver
*
*   \defgroup group_ipc_functions Functions
*       Functions are used in the driver
*
*   \defgroup group_ipc_data_structures Data Structures
*       Data structures are used in the driver
*
*   \defgroup group_ipc_enums Enumerated Types
*       Enumerations are used in the driver
*
*/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "syslib/cy_syslib.h"
#include "cy_device_headers.h"
#include "cy_ipc_config.h"

/**
* \addtogroup group_ipc_macros
* \{
*/

/** Driver major version */
#define CY_IPC_DRV_VERSION_MAJOR       1

/** Driver minor version */
#define CY_IPC_DRV_VERSION_MINOR       10

/** Defines a value to indicate that no notification events are needed */
#define CY_IPC_NO_NOTIFICATION         (uint32_t)(0x00000000ul)

/* Error Code constants */
#define CY_IPC_ID CY_PDL_DRV_ID(0x22u)  /**< Software PDL driver ID for IPC */

/** Return prefix for IPC driver function status codes */
#define CY_IPC_ID_INFO    (uint32_t)( CY_IPC_ID | CY_PDL_STATUS_INFO )
/** Return prefix for IPC driver function warning return values */
#define CY_IPC_ID_WARNING (uint32_t)( CY_IPC_ID | CY_PDL_STATUS_WARNING)
/** Return prefix for IPC driver function error return values */
#define CY_IPC_ID_ERROR   (uint32_t)( CY_IPC_ID | CY_PDL_STATUS_ERROR)

/** Converts the IPC interrupt channel number to interrupt vector */
#define CY_IPC_INTR_NUM_TO_VECT(x)         ((int32_t)cpuss_interrupts_ipc_0_IRQn + (x))

/** \} group_ipc_macros */

/* end of definition in device.h */


/**
* \addtogroup group_ipc_enums
* \{
*/

/**
* This is a list of ENUMs used for function return status.
*/
typedef enum
{
    /** Function was successfully executed */
    CY_IPC_DRV_SUCCESS      = (0x00u),
    /** Function was not executed due to an error.
        Typical conditions for the error explained
        in the function description */
    CY_IPC_DRV_ERROR        = ( CY_IPC_ID_ERROR + 1ul),
} cy_en_ipcdrv_status_t;

/** \} group_ipc_enums */


#ifdef __cplusplus
extern "C" {
#endif

/** \cond INTERNAL */

__STATIC_INLINE void     Cy_IPC_Drv_WriteDataValue (volatile stc_IPC_STRUCT_t* base, uint32_t dataValue);
__STATIC_INLINE uint32_t Cy_IPC_Drv_ReadDataValue (volatile stc_IPC_STRUCT_t const * base);
__STATIC_INLINE void     Cy_IPC_Drv_ReleaseNotify (volatile stc_IPC_STRUCT_t* base, uint32_t notifyEventIntr);

__STATIC_INLINE uint32_t Cy_IPC_Drv_ExtractAcquireMask (uint32_t intMask);
__STATIC_INLINE uint32_t Cy_IPC_Drv_ExtractReleaseMask (uint32_t intMask);

/** \endcond */

/**
* \addtogroup group_ipc_functions
* \{
*/

__STATIC_INLINE volatile stc_IPC_STRUCT_t*       Cy_IPC_Drv_GetIpcBaseAddress (uint32_t ipcIndex);
__STATIC_INLINE volatile stc_IPC_INTR_STRUCT_t*  Cy_IPC_Drv_GetIntrBaseAddr (uint32_t ipcIntrIndex);

__STATIC_INLINE void     Cy_IPC_Drv_AcquireNotify (volatile stc_IPC_STRUCT_t * base, uint32_t notifyEventIntr);

cy_en_ipcdrv_status_t    Cy_IPC_Drv_LockAcquire (volatile stc_IPC_STRUCT_t const * base);
cy_en_ipcdrv_status_t    Cy_IPC_Drv_LockRelease (volatile stc_IPC_STRUCT_t * base, uint32_t releaseEventIntr);
__STATIC_INLINE bool     Cy_IPC_Drv_IsLockAcquired (volatile stc_IPC_STRUCT_t const * base);
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetLockStatus (volatile stc_IPC_STRUCT_t const * base);

cy_en_ipcdrv_status_t    Cy_IPC_Drv_SendMsgWord (volatile stc_IPC_STRUCT_t * base, uint32_t notifyEventIntr, uint32_t message);
cy_en_ipcdrv_status_t    Cy_IPC_Drv_SendMsgWord_2 (volatile stc_IPC_STRUCT_t* base, uint32_t notifyEventIntr, uint32_t message, uint32_t message2);
cy_en_ipcdrv_status_t    Cy_IPC_Drv_ReadMsgWord (volatile stc_IPC_STRUCT_t const * base, uint32_t * message);
cy_en_ipcdrv_status_t    Cy_IPC_Drv_ReadMsgWord_2 (volatile stc_IPC_STRUCT_t const * base, uint32_t * message, uint32_t* message2);

__STATIC_INLINE void     Cy_IPC_Drv_SetInterruptMask (volatile stc_IPC_INTR_STRUCT_t * base,
                                                      uint32_t ipcReleaseMask, uint32_t ipcNotifyMask);
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptMask (volatile stc_IPC_INTR_STRUCT_t const * base);
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptStatusMasked (volatile stc_IPC_INTR_STRUCT_t const * base);
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptStatus (volatile stc_IPC_INTR_STRUCT_t const * base);
__STATIC_INLINE void     Cy_IPC_Drv_SetInterrupt (volatile stc_IPC_INTR_STRUCT_t * base,
                                                      uint32_t ipcReleaseMask, uint32_t ipcNotifyMask);
__STATIC_INLINE void     Cy_IPC_Drv_ClearInterrupt (volatile stc_IPC_INTR_STRUCT_t * base,
                                                      uint32_t ipcReleaseMask, uint32_t ipcNotifyMask);



/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetIpcBaseAddress
****************************************************************************//**
*
* This function takes an IPC channel index as a parameter and returns the base
* address the IPC registers corresponding to the IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \note The user is responsible for ensuring that ipcIndex does not exceed the
* limits.
*
* \param ipcIndex
* Represents the number of IPC structure. This is converted to the base address of
* the IPC channel registers.
*
* \return
* Returns a pointer to the base of the IPC registers.
*
*******************************************************************************/
__STATIC_INLINE volatile stc_IPC_STRUCT_t* Cy_IPC_Drv_GetIpcBaseAddress (uint32_t ipcIndex)
{
    CY_ASSERT((uint32_t)CPUSS_IPC_IPC_NR > ipcIndex);
    return ( (volatile stc_IPC_STRUCT_t*) ( &IPC->STRUCT[ipcIndex] ) );
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetIntrBaseAddr
****************************************************************************//**
*
* This function takes an IPC interrupt structure index and returns the base
* address of the IPC interrupt registers corresponding to the IPC Interrupt.
*
* This function is internal and should not be called directly by user
* software.
*
* \note The user is responsible for ensuring that ipcIntrIndex does not exceed the
* limits.
*
* \param ipcIntrIndex
* Represents the number of IPC interrupt structure. This is converted to the
* base address of the IPC interrupt registers.
*
* \return
* Returns a pointer to the base of the IPC interrupt registers.
*
*******************************************************************************/
__STATIC_INLINE volatile stc_IPC_INTR_STRUCT_t* Cy_IPC_Drv_GetIntrBaseAddr (uint32_t ipcIntrIndex)
{
    CY_ASSERT((uint32_t)CPUSS_IPC_IPC_IRQ_NR > ipcIntrIndex);
    return ( (volatile stc_IPC_INTR_STRUCT_t*) ( &IPC->INTR_STRUCT[ipcIntrIndex] ) );
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_SetInterruptMask
****************************************************************************//**
*
* This function is used to set the interrupt mask for an IPC Interrupt.
* The mask sets release or acquire notification events for all IPC channels.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from the
* IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \param ipcReleaseMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* release event.
*
* \param ipcNotifyMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* notify event.
*
*******************************************************************************/
__STATIC_INLINE void  Cy_IPC_Drv_SetInterruptMask (volatile stc_IPC_INTR_STRUCT_t* base,
                                              uint32_t ipcReleaseMask, uint32_t ipcNotifyMask)
{
    un_IPC_INTR_STRUCT_INTR_MASK_t reg = { 0 };
    reg.stcField.u16NOTIFY  = ipcNotifyMask;
    reg.stcField.u16RELEASE = ipcReleaseMask;
    base->unINTR_MASK.u32Register = reg.u32Register;
}


/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetInterruptMask
****************************************************************************//**
*
* This function is used to read the interrupt mask.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from
* the IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \return
*   The return value is encoded as follows
*   <table>
*   <tr><th>Interrupt sources   <th>Value
*   <tr><td>Ipc_PORTX_RELEASE   <td>Xth bit set
*   <tr><td>Ipc_PORTX_NOTIFY    <td>X+16th bit set
*   </table>
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptMask(volatile stc_IPC_INTR_STRUCT_t const * base)
{
    return (base->unINTR_MASK.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetInterruptStatusMasked
****************************************************************************//**
*
* This function is used to read the active unmasked interrupt. This function
* can be used in the interrupt service routine to find which source triggered
* the interrupt.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from the
* IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \return
*   The return value is encoded as follows
*   <table>
*   <tr><th>Interrupt sources   <th>Value
*   <tr><td>Ipc_PORTX_RELEASE   <td>Xth bit set
*   <tr><td>Ipc_PORTX_NOTIFY    <td>X+16th bit set
*   </table>
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptStatusMasked (volatile stc_IPC_INTR_STRUCT_t const * base)
{
    return (base->unINTR_MASKED.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetInterruptStatus
****************************************************************************//**
*
* This function is used to read the pending interrupts. Note that this read is
* an unmasked read of the interrupt status. Interrupt sources read as active by
* this function would generate interrupts only if they were not masked.

* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from the
* IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \return
*   The return value is encoded as follows
*   <table>
*   <tr><th>Interrupt sources   <th>Value
*   <tr><td>Ipc_PORTX_RELEASE   <td>Xth bit set
*   <tr><td>Ipc_PORTX_NOTIFY    <td>X+16th bit set
*   </table>
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetInterruptStatus(volatile stc_IPC_INTR_STRUCT_t const * base)
{
    return (base->unINTR.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_SetInterrupt
****************************************************************************//**
*
* This function is used to set the interrupt source. This function can be used
* to activate interrupts through software.
* \note That interrupt sources set using this interrupt would generate interrupts
* only if they are not masked.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from the
* IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \param ipcReleaseMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* release event.
*
* \param ipcNotifyMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* notify event.
*
*******************************************************************************/
__STATIC_INLINE void  Cy_IPC_Drv_SetInterrupt(volatile stc_IPC_INTR_STRUCT_t* base, uint32_t ipcReleaseMask, uint32_t ipcNotifyMask)
{
    un_IPC_INTR_STRUCT_INTR_SET_t reg = { 0 };
    reg.stcField.u16NOTIFY  = ipcNotifyMask;
    reg.stcField.u16RELEASE = ipcReleaseMask;
    base->unINTR_SET.u32Register = reg.u32Register;
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ClearInterrupt
****************************************************************************//**
*
* This function is used to clear the interrupt source. Use this function to clear
* a pending interrupt source in the interrupt status.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This is a handle to the IPC interrupt. This handle can be calculated from the
* IPC interrupt number using \ref Cy_IPC_Drv_GetIntrBaseAddr.
*
* \param ipcReleaseMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* release event.
*
* \param ipcNotifyMask
* An encoded list of all IPC channels that can trigger the interrupt on a
* notify event.
*
*******************************************************************************/
__STATIC_INLINE void  Cy_IPC_Drv_ClearInterrupt(volatile stc_IPC_INTR_STRUCT_t* base, uint32_t ipcReleaseMask, uint32_t ipcNotifyMask)
{
    un_IPC_INTR_STRUCT_INTR_t reg = { 0 };
    reg.stcField.u16NOTIFY  = ipcNotifyMask;
    reg.stcField.u16RELEASE = ipcReleaseMask;
    base->unINTR.u32Register = reg.u32Register;
    (void)base->unINTR.u32Register;  /* Read the register to flush the cache */
}

/** \} group_ipc_functions */

/** \} group_ipc */

/*******************************************************************************
* Function Name: Cy_IPC_Drv_AcquireNotify
****************************************************************************//**
*
* The function generates a notify event by IPC interrupt structures.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param notifyEventIntr
* Bit encoded list of IPC interrupt structures that are triggered
* by a notification. Bit number correspond to number of the IPC interrupt
* structure.
*
*******************************************************************************/
__STATIC_INLINE void  Cy_IPC_Drv_AcquireNotify (volatile stc_IPC_STRUCT_t* base, uint32_t notifyEventIntr)
{
    un_IPC_STRUCT_NOTIFY_t reg = { 0 };
    reg.stcField.u16INTR_NOTIFY = notifyEventIntr;
    base->unNOTIFY.u32Register = reg.u32Register;
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ReleaseNotify
****************************************************************************//**
*
* The function generates a notify event to an IPC interrupt structure.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param notifyEventIntr
* Bit encoded list of IPC interrupt lines that are triggered by a notification.
*
*******************************************************************************/
__STATIC_INLINE void  Cy_IPC_Drv_ReleaseNotify (volatile stc_IPC_STRUCT_t* base, uint32_t notifyEventIntr)
{
    un_IPC_STRUCT_RELEASE_t reg = { 0 };
    reg.stcField.u16INTR_RELEASE = notifyEventIntr;
    base->unRELEASE.u32Register = reg.u32Register;
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_WriteDataValue
****************************************************************************//**
*
* The function writes a value to the DATA register of the IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param dataValue
* Value to be written.
*
*******************************************************************************/
__STATIC_INLINE void     Cy_IPC_Drv_WriteDataValue (volatile stc_IPC_STRUCT_t* base, uint32_t dataValue)
{
      base->unDATA0.u32Register = dataValue;
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_WriteData1Value
****************************************************************************//**
*
* The function writes a value to the DATA1 register of the IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param dataValue
* Value to be written.
*
*******************************************************************************/
__STATIC_INLINE void     Cy_IPC_Drv_WriteData1Value (volatile stc_IPC_STRUCT_t* base, uint32_t dataValue)
{
      base->unDATA1.u32Register = dataValue;
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ReadDataValue
****************************************************************************//**
*
* The function reads a value from the DATA register of the IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \return
* Value from DATA register.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_ReadDataValue (volatile stc_IPC_STRUCT_t const * base)
{
    return (base->unDATA0.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ReadData1Value
****************************************************************************//**
*
* The function reads a value from the DATA1 register of the IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \return
* Value from DATA register.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_ReadData1Value (volatile stc_IPC_STRUCT_t const * base)
{
    return (base->unDATA1.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_IsLockAcquired
****************************************************************************//**
*
* The function is used to test the status of an IPC channel. The function
* tells the reader if the IPC channel was in the locked or released state.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \return
*   Status for the function:
*   true:  The IPC channel is in the Locked state.
*   false: The IPC channel is in the Released state.
*
*******************************************************************************/
__STATIC_INLINE bool Cy_IPC_Drv_IsLockAcquired (volatile stc_IPC_STRUCT_t const * base)
{
    return ( 0u != base->unLOCK_STATUS.stcField.u1ACQUIRED );
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_GetLockStatus
****************************************************************************//**
*
* The function is used to get the status of an IPC channel.
*
* This function is internal and should not be called directly by user
* software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \return
* Value from LOCK_STATUS register.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_GetLockStatus (volatile stc_IPC_STRUCT_t const * base)
{
    return (base->unLOCK_STATUS.u32Register);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ExtractAcquireMask
****************************************************************************//**
*
* The function extracts an Acquire mask part from full interrupt mask value.
*
* This function is internal and should not be called directly by user
* software.
*
* \param intMask
* Interrupt mask value to be processed.
*
* \return
* Acquire mask value.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_ExtractAcquireMask (uint32_t intMask)
{
    return _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_NOTIFY, intMask);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ExtractReleaseMask
****************************************************************************//**
*
* The function extracts a Release mask part from full interrupt mask value.
*
* This function is internal and should not be called directly by user
* software.
*
* \param intMask
* Interrupt mask value to be processed.
*
* \return
* Release mask value.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_IPC_Drv_ExtractReleaseMask (uint32_t intMask)
{
    return _FLD2VAL(IPC_INTR_STRUCT_INTR_MASK_RELEASE, intMask);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_SendMsgPtr
****************************************************************************//**
*
* This function is used to send a message pointer through an IPC channel.
* The message structure may hold a generic pointer that may contain the address
* of any user data type or structure. This parameter could be a pointer to a 32-bit
* integer, an array, or even a data structure defined in the user code. This
* function acts as a transfer engine for sending the pointer. Any memory
* management of the pointer allocation and deallocation is up to the application
* code.
* The function also has an associated notification field that will let the
* message notify one or multiple interrupts.
*
* This function is internal and should not be called directly by user software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param notifyEventIntr
* Bit encoded list of IPC interrupt lines that are triggered during the release
* action.
*
* \param msgPtr
* The message pointer that is being sent over the IPC channel.
*
* \return   Status of the operation:
*   \retval CY_IPC_DRV_SUCCESS: The send operation was successful.
*   \retval CY_IPC_DRV_ERROR:   The IPC channel is unavailable because
*                               it is already locked.
*
*******************************************************************************/
__STATIC_INLINE cy_en_ipcdrv_status_t  Cy_IPC_Drv_SendMsgPtr(volatile stc_IPC_STRUCT_t* base, uint32_t notifyEventIntr, void const * msgPtr)
{
    return Cy_IPC_Drv_SendMsgWord(base, notifyEventIntr, (uint32_t)msgPtr);
}

/*******************************************************************************
* Function Name: Cy_IPC_Drv_ReadMsgPtr
****************************************************************************//**
*
* This function is used to read a 32-bit pointer message through an IPC channel.
*
* This function is internal and should not be called directly by user software.
*
* \param base
* This parameter is a handle that represents the base address of the registers
* of the IPC channel.
* The parameter is generally returned from a call to the \ref
* Cy_IPC_Drv_GetIpcBaseAddress.
*
* \param msgPtr
* Pointer variable to hold the data pointer that is being read from the IPC
* channel.
*
*
* \return Status of the operation
*   \retval CY_IPC_DRV_SUCCESS: The function executed successfully and the IPC
*                       was acquired.
*   \retval CY_IPC_DRV_ERROR:   The function encountered an error because the IPC
*                       channel was already in a released state meaning the data
*                       in it is invalid.
*
*******************************************************************************/
__STATIC_INLINE  cy_en_ipcdrv_status_t  Cy_IPC_Drv_ReadMsgPtr (volatile stc_IPC_STRUCT_t const * base, void ** msgPtr)
{
    return Cy_IPC_Drv_ReadMsgWord(base, (uint32_t *)msgPtr);
}

#ifdef __cplusplus
}
#endif

#endif /* !defined(CY_IPC_DRV_H) */

/* [] END OF FILE */
