/***************************************************************************//**
* \file cy_power.c
* \version 1.0
*
* \brief
* Provides an API implementation of the power supply middleware driver
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*****************************************************************************
* Include files
*****************************************************************************/
#include "cy_project.h"
#include "cy_device_headers.h"
#include "cy_power.h"


#if defined(__cplusplus)
extern "C" {
#endif

/*****************************************************************************
* Local pre-processor symbols/macros ('define')
*****************************************************************************/

#define PMIC_STATUS_WAIT_TIME_GRANULARITY_US                4

#define CREATE_FUNCTION_NAME_INTERNAL(func_name,infix)      Cy_ ## infix ## _ ## func_name
#define CREATE_FUNCTION_NAME(func_name,infix)               CREATE_FUNCTION_NAME_INTERNAL(func_name,infix)


#if   defined(CY_POWER_USES_REGHC_IP)

    #define DRIVER_INFIX                                    SysReghc

    #define SelectMode(mode)                                Cy_SysReghc_SelectMode(mode)                  // Only supported by REGHC IP
    #define SelectDriveOut(drvOut)                          Cy_SysReghc_SelectDriveOut(drvOut)            // Only supported by REGHC IP
    #define IsOcdWithinLimits()                             Cy_SysReghc_IsOcdWithinLimits()               // Only supported by REGHC IP
    #define IsCircuitEnabledAndOperating()                  Cy_SysReghc_IsCircuitEnabledAndOperating()    // Only supported by REGHC IP
    #define SelectPmicVref(vref)                                                                          // Not supported by REGHC IP
    #define EnableAnalogBuffer()                                                                          // Not supported by REGHC IP
    #define DisableAnalogBuffer()                                                                         // Not supported by REGHC IP

#elif defined(CY_POWER_USES_PMIC_IP)

    #define DRIVER_INFIX                                    SysPmic
    
    #define SelectMode(mode)                                                                              // Not supported by PMIC IP
    #define SelectDriveOut(drvOut)                                                                        // Not supported by PMIC IP
    #define IsOcdWithinLimits()                             (false)                                       // Not supported by PMIC IP
    #define IsCircuitEnabledAndOperating()                  (false)                                       // Not supported by PMIC IP
    #define SelectPmicVref(vref)                            Cy_SysPmic_SelectPmicVref(vref)               // Only supported by PMIC IP
    #define EnableAnalogBuffer()                            Cy_SysPmic_EnableAnalogBuffer()               // Only supported by PMIC IP
    #define DisableAnalogBuffer()                           Cy_SysPmic_DisableAnalogBuffer()              // Only supported by PMIC IP

#endif

// Generic functions supported by REGHC and PMIC
#define DisableLinearRegulator                              CREATE_FUNCTION_NAME(DisableLinearRegulator        , DRIVER_INFIX)
#define EnableLinearRegulator                               CREATE_FUNCTION_NAME(EnableLinearRegulator         , DRIVER_INFIX)
#define GetLinearRegulatorStatus                            CREATE_FUNCTION_NAME(GetLinearRegulatorStatus      , DRIVER_INFIX)
#define DisableDeepSleepRegulator                           CREATE_FUNCTION_NAME(DisableDeepSleepRegulator     , DRIVER_INFIX)
#define EnableDeepSleepRegulator                            CREATE_FUNCTION_NAME(EnableDeepSleepRegulator      , DRIVER_INFIX)
#define AdjustOutputVoltage                                 CREATE_FUNCTION_NAME(AdjustOutputVoltage           , DRIVER_INFIX)
#define DisableInternalSupplyWhileExternalSupplyActive      CREATE_FUNCTION_NAME(DisableIntSupplyWhileExtActive, DRIVER_INFIX)
#define EnableInternalSupplyWhileExternalSupplyActive       CREATE_FUNCTION_NAME(EnableIntSupplyWhileExtActive , DRIVER_INFIX)
#define DisablePmicEnableOutput                             CREATE_FUNCTION_NAME(DisablePmicEnableOutput       , DRIVER_INFIX)
#define EnablePmicEnableOutput                              CREATE_FUNCTION_NAME(EnablePmicEnableOutput        , DRIVER_INFIX)
#define EnablePmicStatusInput                               CREATE_FUNCTION_NAME(EnablePmicStatusInput         , DRIVER_INFIX)
#define DisablePmicStatusInput                              CREATE_FUNCTION_NAME(DisablePmicStatusInput        , DRIVER_INFIX)
#define SetPmicStatusWaitTime                               CREATE_FUNCTION_NAME(SetPmicStatusWaitTime         , DRIVER_INFIX)
#define IsConfigured                                        CREATE_FUNCTION_NAME(IsConfigured                  , DRIVER_INFIX)
#define SetConfigured                                       CREATE_FUNCTION_NAME(SetConfigured                 , DRIVER_INFIX)
#define Disable                                             CREATE_FUNCTION_NAME(Disable                       , DRIVER_INFIX)
#define Enable                                              CREATE_FUNCTION_NAME(Enable                        , DRIVER_INFIX)
#define DisablePmicStatusTimeout                            CREATE_FUNCTION_NAME(DisablePmicStatusTimeout      , DRIVER_INFIX)
#define EnablePmicStatusTimeout                             CREATE_FUNCTION_NAME(EnablePmicStatusTimeout       , DRIVER_INFIX)
#define IsEnabled                                           CREATE_FUNCTION_NAME(IsEnabled                     , DRIVER_INFIX)
#define IsStatusOk                                          CREATE_FUNCTION_NAME(IsStatusOk                    , DRIVER_INFIX)
#define IsSequencerBusy                                     CREATE_FUNCTION_NAME(IsSequencerBusy               , DRIVER_INFIX)
#define DisableVAdj                                         CREATE_FUNCTION_NAME(DisableVAdj                   , DRIVER_INFIX)
#define EnableVAdj                                          CREATE_FUNCTION_NAME(EnableVAdj                    , DRIVER_INFIX)
#define DisablePmicInDeepSleep                              CREATE_FUNCTION_NAME(DisablePmicInDeepSleep        , DRIVER_INFIX)
#define EnablePmicInDeepSleep                               CREATE_FUNCTION_NAME(EnablePmicInDeepSleep         , DRIVER_INFIX)




/*****************************************************************************
* Global variable definitions (declared in header file with 'extern')
*****************************************************************************/

/*****************************************************************************
* Local type definitions ('typedef')
*****************************************************************************/

/** Mode (only evaluated in case of REGHC) */
typedef enum
{
    CY_POWER_MODE_TRANSISTOR = 0,
    CY_POWER_MODE_PMIC       = 1,
} cy_en_power_mode_t;

  
/*****************************************************************************
* Local variable definitions ('static')
*****************************************************************************/

/*****************************************************************************
* Local function prototypes ('static')                                                                            
*****************************************************************************/
static void SwitchToExternal(cy_en_power_vadj_t                 voltageAdjust, 
                             cy_en_power_pmic_enable_polarity_t enablePolarity, 
                             cy_en_power_pmic_status_polarity_t statusPolarity,
                             uint16_t                           waitTimeUs,
                             cy_en_power_mode_t                 mode);
  
/*****************************************************************************
* Function implementation - global ('extern') and local ('static')
*****************************************************************************/


/**
 *****************************************************************************
 ** Switch the power source from internal regulator to an external source
 **
 ** \param 
 ** voltageAdjust  : Voltage adjustment 
 ** enablePolarity : 0 - Output Low  when PMIC shall be enabled.
 **                  1 - Output High when PMIC shall be enabled.
 ** statusPolarity : 0 - TVII detects abnormal status when receiving 'low'  from PMIC.
 **                  1 - TVII detects abnormal status when receiving 'high' from PMIC.
 **
 ** waitTimeUs     : giving additional settling time after PMIC status input is 
 **                  initially OK in [us]
 ** mode           : Switch to PMIC or Pass Transistor (only in case of REGHC IP)
 ** 
 ** \return  
 ** none
 *****************************************************************************/
static void SwitchToExternal(cy_en_power_vadj_t                 voltageAdjust, 
                             cy_en_power_pmic_enable_polarity_t enablePolarity, 
                             cy_en_power_pmic_status_polarity_t statusPolarity,
                             uint16_t                           waitTimeUs,
                             cy_en_power_mode_t                 mode)
{
    /*****************************************************************/
    /***       Case 2.Handover from internal regulator to PMIC     ***/
    /***                   OCD = 0     Dpsleep = 0                 ***/
    /*****************************************************************/
    bool Waittime_register = false;
    uint16_t waitTimeRegValue = 0u;
    uint16_t waitTimeRegValue1 = 0u;

    // Set wait time until the PMIC output settle.
    waitTimeRegValue = (waitTimeUs / PMIC_STATUS_WAIT_TIME_GRANULARITY_US);
    if(waitTimeRegValue > 0x1FFul )
    {
        Waittime_register = true;
        waitTimeRegValue1 = waitTimeRegValue;
        waitTimeRegValue = 0x1FFul;
    }
    /* 1. Check if Regulator Configuration is already Completed */
    if(IsConfigured() == false) 
    {
        /* 6. Call ConfigureRegulator */
        /* 7. Check status code and wait for API Completion */
        un_srom_api_args_t configRegArgs = {0ul};
        configRegArgs.ConfigureRegulator2.arg0.Mode                   = (cy_en_srom_regulator_mode_t)            mode;
        configRegArgs.ConfigureRegulator2.arg0.EnablePolarity         = (cy_en_srom_regulator_enable_polarity_t) enablePolarity;
        configRegArgs.ConfigureRegulator2.arg0.StatusAbnormalPolarity = (cy_en_srom_regulator_status_polarity_t) statusPolarity;
        configRegArgs.ConfigureRegulator2.arg0.DeepSleep              = 0;
        configRegArgs.ConfigureRegulator2.arg0.UseLinReg              = 0;
        configRegArgs.ConfigureRegulator2.arg0.UseRadj                = 0;/* Not used*/
        configRegArgs.ConfigureRegulator2.arg0.VadjOption             = 1;
        configRegArgs.ConfigureRegulator2.arg0.VoltageAdjust          = voltageAdjust;
        configRegArgs.ConfigureRegulator2.arg0.RAdjust                = 0; /* Not used*/
        configRegArgs.ConfigureRegulator2.arg0.Opcode                 = CY_SROM_OP_CONFIGURE_REGULATOR;
        un_srom_api_args_2_t configRegArgs2 = {0ul};
        configRegArgs2.ConfigureRegulator2.arg0.WaitCountIn1us         = waitTimeRegValue;

        /* For sort device OFFSET = 0,thus VADJ should be 26 */
        /* For normal device OFFSET not set as 0, thus VADJ should be 10 */
        uint32_t OFFSET = CY_GET_REG32(0x17000730);
        if(OFFSET == 0)
        {
            #if   defined(CY_POWER_USES_REGHC_IP)
                configRegArgs.ConfigureRegulator2.arg0.VoltageAdjust      = CY_SYSREGHC_VADJ_1V150; /* 26 */
            #elif defined(CY_POWER_USES_PMIC_IP)
                configRegArgs.ConfigureRegulator2.arg0.VoltageAdjust      = CY_SYSPMIC_VADJ_1V150; /* 26 */
            #endif
        }
        else if(OFFSET != 0)
        {
            #if   defined(CY_POWER_USES_REGHC_IP)
                configRegArgs.ConfigureRegulator2.arg0.VoltageAdjust      = CY_SYSREGHC_VADJ_1V100; /* 10 */
            #elif defined(CY_POWER_USES_PMIC_IP)
                configRegArgs.ConfigureRegulator2.arg0.VoltageAdjust      = CY_SYSPMIC_VADJ_1V150; /* 26 */
            #endif
        }

        Cy_Srom_CallApi_2(&configRegArgs, &configRegArgs2, NULL);
        /* Need to error handling, if API returns error status */

        /* 8. (if waittime > 0x1FF)Set REGHC_PMIC_STATUS_WAIT */
        /*    Set wait time until the PMIC output settle. */
        if(Waittime_register == true)
        {
            SetPmicStatusWaitTime(waitTimeRegValue1);
        }

        /* 9.Call SwitchOverRegulators */
        /* 10. Check status code and wait for API Completion */
        un_srom_api_args_t switchRegArgs = {0ul};
        switchRegArgs.SwitchRegulator.arg0.Mode         = (cy_en_srom_regulator_mode_t)mode;
        switchRegArgs.SwitchRegulator.arg0.SwitchTarget = CY_SROM_REGULATOR_SWITCH_TARGET_EXT;
        switchRegArgs.SwitchRegulator.arg0.Blocking     = 1;
        switchRegArgs.SwitchRegulator.arg0.Opcode       = CY_SROM_OP_SWITCH_REGULATOR;

        Cy_Srom_CallApi(&switchRegArgs, NULL);
        /* Need to error handling, if API returns error status */
    }
    else
    {
        /* 2. Check if Handover is in Progress */
        /*    Wait until REGHC_SEQ_BUSY = 0 and PWR_REGHC_STATUS.REGHC_ENABLED = u1PMIC_EN */
        while((IsSequencerBusy() == true) || (IsEnabled() != true));

        /* 3. Call LoadRegulatorTrims */
        /* 4. Check status code and wait for API Completion */
        un_srom_api_args_t loadRegulatorTrimArg = {0ul};
        loadRegulatorTrimArg.LoadRegulatorTrim.arg0.trim    = CY_SROM_LoadRegulatorTrim_LDO;
        loadRegulatorTrimArg.LoadRegulatorTrim.arg0.usecase = CY_SROM_LoadRegulatorTrim_Reset_Recovery;
        loadRegulatorTrimArg.LoadRegulatorTrim.arg0.opcode  = CY_SROM_OP_LoadRegulatorTrim;

        Cy_Srom_CallApi(&loadRegulatorTrimArg, NULL);
        /* Need to error handling, if API returns error status */
    
        /* 5. Check MCU operates with External or Internal Regulator */
        /*  Operated by PMIC? */
        if(IsEnabled() == 0)
        {
            /* 8. Set REGHC_PMIC_STATUS_WAIT */
            /*    Set wait time until the PMIC output settle. */
            if(Waittime_register == true)
            {
                SetPmicStatusWaitTime(waitTimeRegValue1);
            }

            /* 9.Call SwitchOverRegulators */
            /* 10. Check status code and wait for API Completion */
            un_srom_api_args_t switchRegArgs = {0ul};
            switchRegArgs.SwitchRegulator.arg0.Mode         = (cy_en_srom_regulator_mode_t)mode;
            switchRegArgs.SwitchRegulator.arg0.SwitchTarget = CY_SROM_REGULATOR_SWITCH_TARGET_EXT;
            switchRegArgs.SwitchRegulator.arg0.Blocking     = 1;
            switchRegArgs.SwitchRegulator.arg0.Opcode       = CY_SROM_OP_SWITCH_REGULATOR;

            Cy_Srom_CallApi(&switchRegArgs, NULL);
            /* Need to error handling, if API returns error status */
        }
    }
}


/**
 *****************************************************************************
 ** Switch the power source from internal regulator to PMIC.
 **
 ** \param 
 ** voltageAdjust  : Voltage adjustment 
 ** enablePolarity : 0 - Output Low  when PMIC shall be enabled.
 **                  1 - Output High when PMIC shall be enabled.
 ** statusPolarity : 0 - TVII detects abnormal status when receiving 'low'  from PMIC.
 **                  1 - TVII detects abnormal status when receiving 'high' from PMIC.
 **
 ** waitTimeUs     : giving additional settling time after PMIC status input is 
 **                  initially OK in [us]
 **               

 ** \return  
 ** none
 *****************************************************************************/
void Cy_Power_SwitchToPmic(cy_en_power_vadj_t                 voltageAdjust, 
                           cy_en_power_pmic_enable_polarity_t enablePolarity, 
                           cy_en_power_pmic_status_polarity_t statusPolarity,
                           uint16_t                           waitTimeUs)
{
    
    SwitchToExternal( voltageAdjust, 
                      enablePolarity,
                      statusPolarity,
                      waitTimeUs,                     
                      CY_POWER_MODE_PMIC );
}


#ifdef CY_POWER_USES_REGHC_IP

/**
 *****************************************************************************
 ** Switch the power source from internal regulator to Pass Transistor.
 **
 ** \param 
 ** voltageAdjust : Voltage adjustment 
 ** 
 ** \return  
 ** none
 *****************************************************************************/
void Cy_Power_SwitchToTransistor(cy_en_power_vadj_t  voltageAdjust)
{
    
    SwitchToExternal( voltageAdjust, 
                      (cy_en_power_pmic_enable_polarity_t) 0, // dummy value
                      (cy_en_power_pmic_status_polarity_t) 0, // dummy value
                      0,                                      // dummy value
                      CY_POWER_MODE_TRANSISTOR);
}

#endif


/**
 *****************************************************************************
 ** Switch the power source from external source to internal regulator.
 **
 ** \param 
 ** \return 
 ** none
 *****************************************************************************/
void Cy_Power_SwitchToInternal()
{
    // Switching back to internal regulator will only work if no debugger is currently connected
    if(SRSS->unPWR_CTL.stcField.u1DEBUG_SESSION == 0)
    {
        un_srom_api_args_t switchRegArgs = {0ul};
        switchRegArgs.SwitchRegulator.arg0.Mode         = CY_SROM_REGULATOR_MODE_PMIC;
        switchRegArgs.SwitchRegulator.arg0.SwitchTarget = CY_SROM_REGULATOR_SWITCH_TARGET_INT;
        switchRegArgs.SwitchRegulator.arg0.Blocking     = 1;
        switchRegArgs.SwitchRegulator.arg0.Opcode       = CY_SROM_OP_SWITCH_REGULATOR;

        Cy_Srom_CallApi(&switchRegArgs, NULL);
    }
}


#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
