/***************************************************************************//**
* \file
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
*  Description:
*  This C file is not intended to be part of the Crypto driver.  It is the code
*  required to configure the crypto driver by user.
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_crypto_config.h"

#include "ipc/cy_ipc_drv.h"
#include "sysint/cy_sysint.h"


/** The Crypto configuration structure. (default example)*/
const cy_stc_crypto_config_t cryptoConfig =
{
    .ipcChannel             = CY_IPC_CHAN_CRYPTO,
    .acquireNotifierChannel = CY_CRYPTO_IPC_INTR_NOTIFY_NUM,
    .releaseNotifierChannel = CY_CRYPTO_IPC_INTR_RELEASE_NUM,

    .userCompleteCallback   = NULL,

    .releaseNotifierConfig  = {
        .sysIntSrc = (cy_en_intr_t)((uint32_t)cpuss_interrupts_ipc_0_IRQn + CY_CRYPTO_IPC_INTR_RELEASE_NUM),
        .intIdx    = CY_CRYPTO_RELEASE_CPU_INT_IDX,
        .isEnabled = true
    }

#if (CY_CPU_CORTEX_M0P)
    ,
    .userGetDataHandler     = NULL,
    .userErrorHandler       = NULL,

    .acquireNotifierConfig = {
        .sysIntSrc = (cy_en_intr_t)((uint32_t)cpuss_interrupts_ipc_0_IRQn + CY_CRYPTO_IPC_INTR_NOTIFY_NUM),
        .intIdx  = CY_CRYPTO_NOTIFY_CPU_INT_IDX,
        .isEnabled = true
    },

    .cryptoErrorIntrConfig = {
        .sysIntSrc = cpuss_interrupt_crypto_IRQn,
        .intIdx  = CY_CRYPTO_ERROR_CPU_INT_IDX,
        .isEnabled = true
    }
#endif

};


/* [] END OF FILE */

