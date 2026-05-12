/*******************************************************************************
* \file cy_sysrehcg.h
* \version 1.0
*
* Provides an API declaration of the system high-current regulator driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_sysreghc High Current Regulator (REGHC)
* \{
* This driver provides global REGHC defines and API function definitions.
*
* \defgroup group_sysreghc_macros Macro
* \defgroup group_sysreghc_functions Functions
* \defgroup group_sysreghc_data_structures Data structures
* \defgroup group_sysreghc_enums Enumerated types
*/

#if !defined(__CY_SYSREGHC_H__)
#define __CY_SYSREGHC_H__

#include "cy_device_headers.h"
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
* \addtogroup group_sysreghc_macros
* \{
*/
/** Driver major version */
#define  CY_SYSREGHC_DRV_VERSION_MAJOR   1
/** Driver minor version */
#define  CY_SYSREGHC_DRV_VERSION_MINOR   0
/** \} group_sysreghc_macros */

/**
* \addtogroup group_sysreghc_enums
* \{
*/

/** This enumeration is used to select the REGHC mode */
typedef enum
{
    CY_SYSREGHC_MODE_TRANSISTOR = 0u, /**< External Transistor Mode */
    CY_SYSREGHC_MODE_PMIC       = 1u, /**< External PMIC Mode */
} cy_en_sysreghc_mode_t;

/** This enumeration is used to set the voltage trims */
typedef enum
{
    CY_SYSREGHC_VADJ_1V020 =  0u,
    CY_SYSREGHC_VADJ_1V025 =  1u,
    CY_SYSREGHC_VADJ_1V030 =  2u,
    CY_SYSREGHC_VADJ_1V035 =  3u,
    CY_SYSREGHC_VADJ_1V040 =  4u,
    CY_SYSREGHC_VADJ_1V045 =  5u,
    CY_SYSREGHC_VADJ_1V050 =  6u,
    CY_SYSREGHC_VADJ_1V055 =  7u,
    CY_SYSREGHC_VADJ_1V060 =  8u,
    CY_SYSREGHC_VADJ_1V065 =  9u,
    CY_SYSREGHC_VADJ_1V070 = 10u,
    CY_SYSREGHC_VADJ_1V075 = 11u,
    CY_SYSREGHC_VADJ_1V080 = 12u,
    CY_SYSREGHC_VADJ_1V085 = 13u,
    CY_SYSREGHC_VADJ_1V090 = 14u,
    CY_SYSREGHC_VADJ_1V095 = 15u,
    CY_SYSREGHC_VADJ_1V100 = 16u, // default value
    CY_SYSREGHC_VADJ_1V105 = 17u,
    CY_SYSREGHC_VADJ_1V110 = 18u,
    CY_SYSREGHC_VADJ_1V115 = 19u,
    CY_SYSREGHC_VADJ_1V120 = 20u,
    CY_SYSREGHC_VADJ_1V125 = 21u,
    CY_SYSREGHC_VADJ_1V130 = 22u,
    CY_SYSREGHC_VADJ_1V135 = 23u,
    CY_SYSREGHC_VADJ_1V140 = 24u,
    CY_SYSREGHC_VADJ_1V145 = 25u,
    CY_SYSREGHC_VADJ_1V150 = 26u,
    CY_SYSREGHC_VADJ_1V155 = 27u,
    CY_SYSREGHC_VADJ_1V160 = 28u,
    CY_SYSREGHC_VADJ_1V165 = 29u,
    CY_SYSREGHC_VADJ_1V170 = 30u,
    CY_SYSREGHC_VADJ_1V175 = 31u,
} cy_en_sysreghc_vadj_t;

// Drive out configuration
typedef enum 
{
    CY_SYSREGHC_DRIVE_OUT_0P9_VCCD_BY_VADJ  = 0u,
    CY_SYSREGHC_DRIVE_OUT_0P8_VCCD_BY_VADJ  = 1u,
    CY_SYSREGHC_DRIVE_OUT_0P6_VCCD_BY_VADJ  = 2u,
    CY_SYSREGHC_DRIVE_OUT_VCCD              = 3u,
} cy_en_sysreghc_drive_out_t;

/** \} group_sysreghc_enums */

/**
* \addtogroup group_sysreghc_functions
* \{
*/

// APIs for internal regulators
void Cy_SysReghc_DisableLinearRegulator(void);
void Cy_SysReghc_EnableLinearRegulator(void);
bool Cy_SysReghc_GetLinearRegulatorStatus(void);
void Cy_SysReghc_DisableDeepSleepRegulator(void);
void Cy_SysReghc_EnableDeepSleepRegulator(void);

// APIs for REGHC
void Cy_SysReghc_SelectMode(cy_en_sysreghc_mode_t mode);

void Cy_SysReghc_SelectDriveOut(cy_en_sysreghc_drive_out_t drvOut);
void Cy_SysReghc_AdjustOutputVoltage(cy_en_sysreghc_vadj_t trim);

void Cy_SysReghc_DisableIntSupplyWhileExtActive(void);
void Cy_SysReghc_EnableIntSupplyWhileExtActive(void);

void Cy_SysReghc_DisablePmicEnableOutput(void);
void Cy_SysReghc_EnablePmicEnableOutput(bool polarity);

void Cy_SysReghc_EnablePmicStatusInput(bool polarity);
void Cy_SysReghc_DisablePmicStatusInput(void);
void Cy_SysReghc_SetPmicStatusWaitTime(uint16_t waitTime);

bool Cy_SysReghc_IsConfigured(void);
void Cy_SysReghc_SetConfigured(void);

void Cy_SysReghc_Disable(void);
void Cy_SysReghc_Enable(void);

void Cy_SysReghc_DisablePmicStatusTimeout(void);
void Cy_SysReghc_EnablePmicStatusTimeout(uint8_t timeout);

bool Cy_SysReghc_IsEnabled(void);
bool Cy_SysReghc_IsStatusOk(void);
bool Cy_SysReghc_IsSequencerBusy(void);

void Cy_SysReghc_DisableVAdj(void);
void Cy_SysReghc_EnableVAdj(void);

void Cy_SysReghc_DisablePmicInDeepSleep(void);
void Cy_SysReghc_EnablePmicInDeepSleep(void);

bool Cy_SysReghc_IsOcdWithinLimits(void);
bool Cy_SysReghc_IsCircuitEnabledAndOperating(void);

/** \} group_sysreghc_functions */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __CY_SYSREGHC_H__ */

/** \} group_sysreghc */


/******************************************************************************/
/* [] END OF FILE */
/******************************************************************************/
