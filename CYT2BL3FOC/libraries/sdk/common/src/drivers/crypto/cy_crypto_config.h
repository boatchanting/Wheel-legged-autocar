/***************************************************************************//**
* \file cy_crypto_config.h
* \version 1.0
*
* \brief
*  This file provides user parameters for the Crypto driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CRYPTO_CONFIG_H)
#define CY_CRYPTO_CONFIG_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"
#include "crypto/cy_crypto_common.h"

/* Defines to configure Crypto driver */
extern const cy_stc_crypto_config_t cryptoConfig;

/**
* \addtogroup group_crypto_config_macros
* \{
*
* These constants defines an interrupts settings for IPC channel used for the
* client-server communication, and the Crypto Error IRQ
*
* Both the CM4/CM7_x and CM0+ uses IPC hardware depended interrupt sources
* through SYSINT structure.
*/

/** Defines the Crypto notify IPC interrupt number */
#define CY_CRYPTO_IPC_INTR_NOTIFY_NUM       CY_IPC_INTR_CRYPTO_SRV

/** Defines the Crypto release IPC interrupt number */
#define CY_CRYPTO_IPC_INTR_RELEASE_NUM      CY_IPC_INTR_CRYPTO_CLI

/** Defines the CPU Interrupt Index for the Crypto IPC server notify interrupt */
#define CY_CRYPTO_NOTIFY_CPU_INT_IDX        (CPUIntIdx3_IRQn)

/** Defines the CPU Interrupt Index for the Crypto IPC client release interrupt */
#define CY_CRYPTO_RELEASE_CPU_INT_IDX       (CPUIntIdx3_IRQn)

/** Defines the CPU Interrupt Index for the Crypto Error interrupt */
#define CY_CRYPTO_ERROR_CPU_INT_IDX         (CPUIntIdx3_IRQn)


/** \} group_crypto_config_macros */

#endif /* #if !defined(CY_CRYPTO_CONFIG_H) */


/* [] END OF FILE */
