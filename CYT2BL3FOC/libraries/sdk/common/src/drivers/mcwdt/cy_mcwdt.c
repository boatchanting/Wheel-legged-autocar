/***************************************************************************//**
* \file cy_mcwdt.c
* \version 1.0
*
*  Description:
*   Provides a system API for the MCWDT driver.
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_mcwdt.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Function Name: Cy_MCWDT_Init
****************************************************************************//**
*
* Initializes the MCWDT block.
*
*  \param base
*  The base pointer to a structure that describes the registers.
*
*  \param config
*  The pointer to a structure that contains component configuration data.
*
* \return cy_en_mcwdt_status_t
*     *base checking result. If the pointer is NULL, returns error.
*
*  \note
*  This API should not be called when the counters are running. Prior to calling
*  this API the counter should be disabled.
*
*******************************************************************************/
cy_en_mcwdt_status_t Cy_MCWDT_Init(volatile stc_MCWDT_t *base, cy_stc_mcwdt_config_t const *config)
{
    cy_en_mcwdt_status_t ret = CY_MCWDT_BAD_PARAM;
    if ((base != NULL) && (config != NULL))
    {
        Cy_MCWDT_Unlock(base);
        un_MCWDT_CTR_CONFIG_t  tempConfigParams     = { 0ul };
        un_MCWDT_CTR2_CONFIG_t tempCNT2ConfigParams = { 0ul };

        base->unCPU_SELECT.u32Register                     = config->coreSelect;
        base->CTR[0].unLOWER_LIMIT.stcField.u16LOWER_LIMIT = config->c0LowerLimit;
        base->CTR[0].unUPPER_LIMIT.stcField.u16UPPER_LIMIT = config->c0UpperLimit;
        base->CTR[0].unWARN_LIMIT.stcField.u16WARN_LIMIT   = config->c0WarnLimit;
        tempConfigParams.stcField.u2LOWER_ACTION           = config->c0LowerAction;
        tempConfigParams.stcField.u2UPPER_ACTION           = config->c0UpperAction;
        tempConfigParams.stcField.u1WARN_ACTION            = config->c0WarnAction;
        tempConfigParams.stcField.u1AUTO_SERVICE           = config->c0AutoService;
        tempConfigParams.stcField.u1SLEEPDEEP_PAUSE        = config->c0SleepDeepPause;
        tempConfigParams.stcField.u1DEBUG_RUN              = config->c0DebugRun;
        base->CTR[0].unCONFIG.u32Register                  = tempConfigParams.u32Register;

        base->CTR[1].unLOWER_LIMIT.stcField.u16LOWER_LIMIT = config->c1LowerLimit;
        base->CTR[1].unUPPER_LIMIT.stcField.u16UPPER_LIMIT = config->c1UpperLimit;
        base->CTR[1].unWARN_LIMIT.stcField.u16WARN_LIMIT   = config->c1WarnLimit;
        tempConfigParams.stcField.u2LOWER_ACTION           = config->c1LowerAction;
        tempConfigParams.stcField.u2UPPER_ACTION           = config->c1UpperAction;
        tempConfigParams.stcField.u1WARN_ACTION            = config->c1WarnAction;
        tempConfigParams.stcField.u1AUTO_SERVICE           = config->c1AutoService;
        tempConfigParams.stcField.u1SLEEPDEEP_PAUSE        = config->c1SleepDeepPause;
        tempConfigParams.stcField.u1DEBUG_RUN              = config->c1DebugRun;
        base->CTR[1].unCONFIG.u32Register                  = tempConfigParams.u32Register;

        tempCNT2ConfigParams.stcField.u5BITS            = config->c2ToggleBit;
        tempCNT2ConfigParams.stcField.u1ACTION          = config->c2Action;
        tempCNT2ConfigParams.stcField.u1SLEEPDEEP_PAUSE = config->c2SleepDeepPause;
        tempCNT2ConfigParams.stcField.u1DEBUG_RUN       = config->c2DebugRun;
        base->unCTR2_CONFIG.u32Register                 = tempCNT2ConfigParams.u32Register;

        Cy_MCWDT_Lock(base);

        ret = CY_MCWDT_SUCCESS;
    }

    return (ret);
}


/*******************************************************************************
* Function Name: Cy_MCWDT_DeInit
****************************************************************************//**
*
*  De-initializes the MCWDT block, returns register values to their default state.
*
*  \param base
*  The base pointer to a structure that describes the registers.
*
*  \note
*  This API should not be called when the counters are running. Prior to calling
*  this API the counter should be disabled.
*
*  \note 
*  This API waits all counter are actually disabled after disables all counters,
*  And then start initializing other registers.
*
*******************************************************************************/
void Cy_MCWDT_DeInit(volatile stc_MCWDT_t *base)
{
    Cy_MCWDT_Unlock(base);

    // disable all counter
    for(uint32_t loop = 0ul; loop < CY_MCWDT_NUM_OF_SUBCOUNTER; loop++)
    {
        base->CTR[loop].unCTL.u32Register = 0ul;
    }
    base->unCTR2_CTL.u32Register    = 0ul;

    for(uint32_t loop = 0ul; loop < CY_MCWDT_NUM_OF_SUBCOUNTER; loop++)
    {
        while(base->CTR[loop].unCTL.u32Register != 0x0ul); // wait until enabled bit become 1
        base->CTR[loop].unLOWER_LIMIT.u32Register = 0x0ul;
        base->CTR[loop].unUPPER_LIMIT.u32Register = 0x0ul;
        base->CTR[loop].unWARN_LIMIT.u32Register  = 0x0ul;
        base->CTR[loop].unCONFIG.u32Register      = 0x0ul;
        base->CTR[loop].unCNT.u32Register         = 0x0ul;
    }

    while(base->unCTR2_CNT.u32Register != 0ul); // wait until enabled bit become 1
    base->unCPU_SELECT.u32Register  = 0ul;
    base->unCTR2_CONFIG.u32Register = 0ul;
    base->unSERVICE.u32Register     = 0x00000003ul;
    base->unINTR.u32Register        = 0xFFFFFFFFul;
    base->unINTR_MASK.u32Register   = 0ul;

    Cy_MCWDT_Lock(base);
}

/*******************************************************************************
* Function Name: Cy_MCWDT_ClearWatchdog
****************************************************************************//**
*
* Clears the MC watchdog counter, to prevent a XRES device reset or fault.
*
*  \param base
*  The base pointer to a structure that describes the registers.
*
*  \param counter
*  The number of the MCWDT counter. The valid range is [0-1].
*
*******************************************************************************/
void Cy_MCWDT_ClearWatchdog(volatile stc_MCWDT_t *base, cy_en_mcwdtctr_t counter)
{
    Cy_MCWDT_Unlock(base);
    Cy_MCWDT_ResetCounters(base, (1u << (uint8_t)counter), 0u);
    Cy_MCWDT_Lock(base);
}


#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
