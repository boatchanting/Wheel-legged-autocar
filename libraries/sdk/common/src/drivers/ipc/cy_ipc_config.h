/***************************************************************************//**
* \file cy_ipc_config.h
* \version 1.10
*
* \brief
* This header file is not intended to be part of the IPC driver since it defines
* a device specific configuration for the IPC channels and pipes.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_IPC_CONFIG_H
#define CY_IPC_CONFIG_H

/* IPC Resources */
#define CY_IPC_CHANNELS                      CPUSS_IPC_IPC_NR      /**< Max number of IPC channels in this device */
#define CY_IPC_INTERRUPTS                    CPUSS_IPC_IPC_IRQ_NR  /**< Max number of IPC interrupt structures in this device */

/* IPC channel definitions  */
#define CY_IPC_CHAN_SYSCALL_CM0              0  /**< System calls for the CM0+ processor */
#define CY_IPC_CHAN_SYSCALL_CM4_CM7_0        1  /**< System calls for the 1st non-CM0+ processor */
#define CY_IPC_CHAN_SYSCALL_CM7_1            2  /**< System calls for the 2nd non-CM0+ processor */
#define CY_IPC_CHAN_SYSCALL_CM7_2            3  /**< System calls for the 3rd non-CM0+ processor */
#define CY_IPC_CHAN_SYSCALL_CM7_3            4  /**< System calls for the 4th non-CM0+ processor */

#ifdef tviibh16m

                                             /* Channel is determined during runtime (-> allows generic library) */
    #define CY_IPC_CHAN_SYSCALL              ( (CPUSS->unIDENTITY.stcField.u4MS ==  0) ? CY_IPC_CHAN_SYSCALL_CM0       :   \
                                               (CPUSS->unIDENTITY.stcField.u4MS == 14) ? CY_IPC_CHAN_SYSCALL_CM4_CM7_0 :   \
                                               (CPUSS->unIDENTITY.stcField.u4MS == 13) ? CY_IPC_CHAN_SYSCALL_CM7_1     :   \
                                               (CPUSS->unIDENTITY.stcField.u4MS ==  7) ? CY_IPC_CHAN_SYSCALL_CM7_2     :   \
                                                                                         CY_IPC_CHAN_SYSCALL_CM7_3       )

    #define CY_IPC_CHAN_CRYPTO               8    /**< IPC data channel for the Crypto */

#else

                                             /* Channel is determined during runtime (-> allows generic library) */
    #define CY_IPC_CHAN_SYSCALL              ( (CPUSS->unIDENTITY.stcField.u4MS ==  0) ? CY_IPC_CHAN_SYSCALL_CM0       :   \
                                               (CPUSS->unIDENTITY.stcField.u4MS == 14) ? CY_IPC_CHAN_SYSCALL_CM4_CM7_0 :   \
                                                                                         CY_IPC_CHAN_SYSCALL_CM7_1       )

    #define CY_IPC_CHAN_CRYPTO               4    /**< IPC data channel for the Crypto */

#endif


#define CY_IPC_CHAN_SEMA                     5    /**< IPC data channel for the Semaphores (just a proposal, can be used as parameter when calling Cy_IPC_Sema_Init() */

#define CY_IPC_CHAN_PIPE_EP0                 6    /**< IPC data channel for PIPE EP0 */
#define CY_IPC_CHAN_PIPE_EP1                 7    /**< IPC data channel for PIPE EP1 */

/* IPC Notify interrupts definitions */
#define CY_IPC_INTR_CRYPTO_SRV               4    /**< IPC interrupt structure for the notification of the Crypto server */
#define CY_IPC_INTR_CRYPTO_CLI               5    /**< IPC interrupt structure for the release notification of the Crypto client */


/* IPC Pipe definitions */
#define CY_IPC_PIPE_MAX_ENDPOINTS            2    /**< Number of pipe endpoints (each endpoint needs its own IPC channel and IPC Intr. struct */
#define CY_IPC_PIPE_MAX_CLIENTS              8    /**< Defines max. number of client IDs (which are part of the pipe message and used to index the callback array) */

/*******************************************************************************
** CY IPC PIPE default/example configuration
*******************************************************************************/

/* The pipe endpoint configuration must be identical on all CPUs.
   Below is a default/example configuration which can be used as config struct
   initializer by the user or they can define their own configuration here 
   or somewhere else
*/

#define CY_IPC_INTR_PIPE_CPU_INT_IDX_EP0     CPUIntIdx4_IRQn    /**< CPU Interrupt Index, used in the EP0 CPU */
#define CY_IPC_INTR_PIPE_EP0                 6                  /**< IPC Interrupt Structure Index, used as notifier for EP0 */
#define CY_IPC_INTR_PIPE_CPU_INT_IDX_EP1     CPUIntIdx4_IRQn    /**< CPU Interrupt Index, used in the EP1 CPU */
#define CY_IPC_INTR_PIPE_EP1                 7                  /**< IPC Interrupt Structure Index, used as notifier for EP1 */


#define CY_IPC_PIPE_ENDPOINTS_DEFAULT_CONFIG                   \
{                                                              \
    {                                                          \
        .ipcStructNr     = CY_IPC_CHAN_PIPE_EP0,               \
        .ipcIntrStructNr = CY_IPC_INTR_PIPE_EP0,               \
        .ipcCpuIntIdx    = CY_IPC_INTR_PIPE_CPU_INT_IDX_EP0,   \
    },                                                         \
    {                                                          \
        .ipcStructNr     = CY_IPC_CHAN_PIPE_EP1,               \
        .ipcIntrStructNr = CY_IPC_INTR_PIPE_EP1,               \
        .ipcCpuIntIdx    = CY_IPC_INTR_PIPE_CPU_INT_IDX_EP1,   \
    },                                                         \
}

/******************************************************************************/


#endif /* CY_IPC_CONFIG_H  */

/* [] END OF FILE */
