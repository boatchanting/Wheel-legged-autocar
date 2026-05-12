/***************************************************************************//**
* \file cy_sysreghc.c
* \version 1.0
*
* Provides an API implementation of the system high-current regulator driver.
*
********************************************************************************
* \copyright
* Copyright 2016, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_sysreghc.h"
#include "syslib/cy_syslib.h"

#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* Function Name: Cy_SysReghc_DisableLinearRegulator
********************************************************************************
* \brief 
* Disables internal linear regulator.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisableLinearRegulator(void)
{
    SRSS->unPWR_CTL2.stcField.u1LINREG_DIS = 1u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnableLinearRegulator
********************************************************************************
* \brief 
* Enables internal linear regulator.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnableLinearRegulator(void)
{
    SRSS->unPWR_CTL2.stcField.u1LINREG_DIS = 0u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_GetLinearRegulatorStatus
********************************************************************************
* \brief 
* Check the status of the internal linear regulator.
*
* \param
* None.   
*
* \return 
* true  - status ok
* false - status fail
*******************************************************************************/
bool Cy_SysReghc_GetLinearRegulatorStatus(void)
{
    return ((bool) SRSS->unPWR_CTL2.stcField.u1LINREG_OK);
}

/*******************************************************************************
* Function Name: Cy_SysReghc_DisableDeepSleepRegulator
********************************************************************************
* \brief 
* Disables internal DeepSleep regulator.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisableDeepSleepRegulator(void)
{
    SRSS->unPWR_CTL2.stcField.u1DPSLP_REG_DIS = 1u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnableDeepSleepRegulator
********************************************************************************
* \brief 
* Enables internal DeepSleep linear regulator.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnableDeepSleepRegulator(void)
{
    SRSS->unPWR_CTL2.stcField.u1DPSLP_REG_DIS = 0u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_SelectMode
********************************************************************************
* \brief 
* Selects one of the supported REGHC modes.
*
* \param
* mode - PMIC or Pass Transistor
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_SelectMode(cy_en_sysreghc_mode_t mode)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_MODE = mode;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_SelectDriveOut
********************************************************************************
* \brief 
* Selects DRV_OUT setting.
*
* \param
* 0b00 - vccd*0.9/vadj, 0b01 - vccd*0.8/vadj, 0b10 - vccd*0.6/vadj, 0b11 - vccd.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_SelectDriveOut(cy_en_sysreghc_drive_out_t drvOut)
{
    SRSS->unPWR_REGHC_CTL.stcField.u2REGHC_PMIC_DRV_VOUT = drvOut;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_AdjustOutputVoltage
********************************************************************************
* \brief 
* Selects regulator output voltage adjustment.
*
* \param
* trim - Regulator output trim according to the formula vadj=(1.020V + REGHC_VADJ*0.005V).   
* The step is nominally 5mV/LSB.
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_AdjustOutputVoltage(cy_en_sysreghc_vadj_t trim)
{
    SRSS->unPWR_REGHC_CTL.stcField.u5REGHC_VADJ = trim;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_DisableIntSupplyWhileExtActive
********************************************************************************
* \brief 
*  Internal Active Linear Regulator disabled after PMIC enabled.  OCD is disabled.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisableIntSupplyWhileExtActive(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_USE_LINREG = 0u;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnableIntSupplyWhileExtActive
********************************************************************************
* \brief 
*   Internal Active Linear Regulator kept enabled.  See datasheet for minimum 
*   PMIC vccd input to prevent OCD.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnableIntSupplyWhileExtActive(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_USE_LINREG = 1u;
#endif
}



/*******************************************************************************
* Function Name: Cy_SysReghc_DisablePmicEnableOutput
********************************************************************************
* \brief 
* Disables "PMIC enable" output.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisablePmicEnableOutput(void)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_CTL_OUTEN = 0u;
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_CTL_POLARITY = 0u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnablePmicEnableOutput
********************************************************************************
* \brief 
* Enables "PMIC enable" output.
*
* \param
* polarity => true - output high, false - output low.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnablePmicEnableOutput(bool polarity)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_CTL_OUTEN = 1u;
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_CTL_POLARITY = (polarity == false) ? 0u : 1u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnablePmicStatusInput
********************************************************************************
* \brief 
* Enables receiving status from PMIC.
*
* \param
* polarity => true/false -> receiving high/low means abnormal status.
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnablePmicStatusInput(bool polarity)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_STATUS_INEN = 1u;
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_STATUS_POLARITY = (polarity == false) ? 0u : 1u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_DisablePmicStatusInput
********************************************************************************
* \brief 
* Disables PMIC status input 
*
* \param
* None.   
*
* \return 
* None
*******************************************************************************/
void Cy_SysReghc_DisablePmicStatusInput(void)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_PMIC_STATUS_INEN = 0u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_SetPmicStatusWaitTime
********************************************************************************
* \brief 
* Wait count in 4us steps after PMIC status ok.  This is used by the hardware 
* sequencer to allow additional settling time before disabling the internal regulator.  
* The LSB is 32 IMO periods which results in a nominal LSB step of 4us.
*
* \param
* waitTime = 0 to 1023 -> (waitTime * 4us) Delay
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_SetPmicStatusWaitTime(uint16_t waitTime)
{
    SRSS->unPWR_REGHC_CTL.stcField.u10REGHC_PMIC_STATUS_WAIT = waitTime;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_IsConfigured
********************************************************************************
* \brief 
* Indicates that the REGHC has been configured. 
* Do not change REGHC settings after this bit is high.  
*
* \param
* None.   
*
* \return 
* true - Configured, false - Not configured
*******************************************************************************/
bool Cy_SysReghc_IsConfigured(void)
{
    return ((bool) SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_CONFIGURED);
}

/*******************************************************************************
* Function Name: Cy_SysReghc_SetConfigured
********************************************************************************
* \brief 
* Set REGHC is configured. This is required to apply setting before enabling REGHC.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_SetConfigured(void)
{
    SRSS->unPWR_REGHC_CTL.stcField.u1REGHC_CONFIGURED = 1u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_Disable
********************************************************************************
* \brief 
* Disables REGHC.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_Disable(void)
{
    SRSS->unPWR_REGHC_CTL2.stcField.u1REGHC_EN = 0u;
}

/*******************************************************************************
* Function Name: Cy_SysReghc_Enable
********************************************************************************
* \brief 
* Enables REGHC.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_Enable(void)
{
    SRSS->unPWR_REGHC_CTL2.stcField.u1REGHC_EN = 1u;
}
     
/*******************************************************************************
* Function Name: Cy_SysReghc_DisablePmicStatusTimeout
********************************************************************************
* \brief 
* Disables timeout when waiting for REGHC_PMIC_STATUS_OK.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisablePmicStatusTimeout(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL2.stcField.u8REGHC_PMIC_STATUS_TIMEOUT = 0u;
#endif
}
     
/*******************************************************************************
* Function Name: Cy_SysReghc_EnablePmicStatusTimeout
********************************************************************************
* \brief 
* Enables timeout while waiting for REGHC_PMIC_STATUS_OK==1 when switching to PMIC.  
* Timeout expiration triggers reset.
*
* \param
* timeout = 1 - 255 -> (timeout * 128us) delay until reset
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnablePmicStatusTimeout(uint8_t timeout)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL2.stcField.u8REGHC_PMIC_STATUS_TIMEOUT = timeout;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysReghc_IsEnabled
********************************************************************************
* \brief 
* Indicates the state of the REGHC enable/disable sequencer.  This bit is only 
* valid when REGHC_SEQ_BUSY==0.
*
* \param
* None.   
*
* \return 
* true - Enabled, false - Disabled
*******************************************************************************/
bool Cy_SysReghc_IsEnabled(void)
{
    return ((bool) SRSS->unPWR_REGHC_STATUS.stcField.u1REGHC_ENABLED);
}

/*******************************************************************************
* Function Name: Cy_SysReghc_IsStatusOk
********************************************************************************
* \brief 
* Indicates the PMIC status is ok.  This includes polarity adjustment according 
* to REGHC_PMIC_STATUS_POLARITY.
*
* \param
* None.   
*
* \return 
* false: PMIC status is not ok or PMIC input is disabled (PMIC_STATUS_INEN == 0)
* true:  PMIC status input is enabled and indicates ok
*******************************************************************************/
bool Cy_SysReghc_IsStatusOk(void)
{
    return ((bool) SRSS->unPWR_REGHC_STATUS.stcField.u1REGHC_PMIC_STATUS_OK);
}

/*******************************************************************************
* Function Name: Cy_SysReghc_IsSequencerBusy
********************************************************************************
* \brief 
* Indicates whether the REGHC circuit is busy. Indicates the REGHC enable/disable 
* sequencer is busy transitioning to/from REGHC.
*
* \param
* None.   
*
* \return 
* true:  REGHC busy
* false: REGHC not busy
*******************************************************************************/
bool Cy_SysReghc_IsSequencerBusy(void)
{
    return ((bool) SRSS->unPWR_REGHC_STATUS.stcField.u1REGHC_SEQ_BUSY);
}

/*******************************************************************************
* Function Name: Cy_SysReghc_DisableVAdj
********************************************************************************
* \brief 
* Device does not generate VADJ, and it must not be part of the PMIC feedback loop.  
* This reduces current by turning off the internal resistor divider that generates VADJ.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisableVAdj(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL4.stcField.u1REGHC_PMIC_VADJ_DIS   = 1u;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysReghc_EnableVAdj
********************************************************************************
* \brief 
* Device generates VADJ when PMIC is enabled.  This allows the feedback loop to 
* compensate for voltage drops in the PCB and package.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnableVAdj(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL4.stcField.u1REGHC_PMIC_VADJ_DIS   = 0u;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysReghc_DisablePmicInDeepSleep
********************************************************************************
* \brief 
* Device operates from internal regulators during DEEPSLEEP.  If PMIC is enabled 
* at the beginning of the DEEPSLEEP transition, hardware changes to the internal 
* regulators and disables the PMIC.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_DisablePmicInDeepSleep(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL4.stcField.u1REGHC_PMIC_DPSLP = 0u;
#endif
}
                                  
/*******************************************************************************
* Function Name: Cy_SysReghc_EnablePmicInDeepSleep
********************************************************************************
* \brief 
* DEEPSLEEP transition does not change PMIC enable.
*
* \param
* None.   
*
* \return 
* None.
*******************************************************************************/
void Cy_SysReghc_EnablePmicInDeepSleep(void)
{
#if defined(tviibh8m) && defined(CY_MCU_rev_a)
    // No this feature. ignore.
#else
    SRSS->unPWR_REGHC_CTL4.stcField.u1REGHC_PMIC_DPSLP = 1u;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysReg_IsRegHCOcdWithinLimits
********************************************************************************
* \brief 
* Indicates the over-current detector is operating and the current drawn from 
* REGHC is within limits.  OCD is only a choice for transistor mode, and it is 
* disabled for PMIC mode.  
*
* \param
* None.   
*
* \return 
* false: Current measurement exceeds limit or detector is OFF,
* true:  Current measurement within limit
*******************************************************************************/
bool Cy_SysReghc_IsOcdWithinLimits(void)
{
    return ((bool) SRSS->unPWR_REGHC_STATUS.stcField.u1REGHC_OCD_OK);
}

/*******************************************************************************
* Function Name: Cy_SysReg_IsRegHCCktEnabledAndOperating
********************************************************************************
* \brief 
* Indicates the REGHC circuit is enabled and operating. 
*
* \param
* None.   
*
* \return 
* false: REGHC Disabled
* true:  REGHC Enabled and Operating
*******************************************************************************/
bool Cy_SysReghc_IsCircuitEnabledAndOperating(void)
{
    return ((bool) SRSS->unPWR_REGHC_STATUS.stcField.u1REGHC_CKT_OK);
}



#if defined(__cplusplus)
}
#endif /* __cplusplus */

/******************************************************************************/
/* [] END OF FILE */
/******************************************************************************/
