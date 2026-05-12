/*******************************************************************************
* \file cy_cpu.h
* \version 1.0
*
* Provides an API declaration of the cpu driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/**
* \defgroup group_cpu Central Processing Unit (CPU)
* \{
* This driver provides global CPU defines and API function definitions.
*
* \defgroup group_cpu_macros Macro
* \defgroup group_cpu_functions Functions
* \defgroup group_cpu_data_structures Data structures
* \defgroup group_cpu_enums Enumerated types
*/

#if !defined(__CY_CPU_H__)
#define __CY_CPU_H__

#include "cy_device_headers.h"
#include "syslib/cy_syslib.h"
#include "syspm/cy_syspm.h"
#include "systick/cy_systick.h"
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/**
* \addtogroup group_cpu_macros
* \{
*/
/** Driver major version */
#define  CY_CPU_DRV_VERSION_MAJOR   1
/** Driver minor version */
#define  CY_CPU_DRV_VERSION_MINOR   0
/** \} group_cpu_macros */

/**
* \addtogroup group_cpu_enums
* \{
*/

/* Data types for Tightly Coupled Memory type */ 
typedef enum
{
    CY_CPU_ITCM = 0,
    CY_CPU_DTCM = 1,
} cy_en_cpu_tcm_type_t;

/* Data types for CPU Core type */
typedef enum
{
    CY_CPU_CM7_0 = 0,
    CY_CPU_CM7_1 = 1,
#if defined (tviibh16m)  
    CY_CPU_CM7_2 = 2,
    CY_CPU_CM7_3 = 3,
#endif
} cy_en_cpu_core_type_t;

/* Data types for CPU Power Mode type */
typedef enum
{
    CY_CPU_PM_OFF       = 0,
    CY_CPU_PM_RESET     = 1,
    CY_CPU_PM_RETAINED  = 2,
    CY_CPU_PM_ENABLED   = 3
} cy_en_cpu_power_mode_t;

/* Data types for available CPU SRAM type */
typedef enum
{
    CY_CPU_SRAM0 = 0,
    CY_CPU_SRAM1 = 1,
    CY_CPU_SRAM2 = 2
} cy_en_cpu_sram_macro_t;

/* Data types for CPU SRAM Power Mode type */
typedef enum
{
    CY_CPU_SRAM_PM_OFF       = 0,
    CY_CPU_SRAM_PM_RESERVED  = 1,
    CY_CPU_SRAM_PM_RETAINED  = 2,
    CY_CPU_SRAM_PM_ENABLED   = 3
} cy_en_cpu_sram_power_mode_t;

/* Data types for CPU Clock Divider type */
typedef enum
{
    CY_CPU_INTDIV  = 0,
    CY_CPU_FRACDIV = 1,
} cy_en_cpu_div_type_t;

/* Data types for CPU Clock Speed Selection type */
typedef enum
{
    CY_CPU_SLOW_CLK  = 0,
    CY_CPU_FAST_CLK = 1,
} cy_en_cpu_clk_type_t;

/* Data types for CPU Protection State type */
typedef enum
{
    CY_CPU_PROTECTION_UNKNOWN   = 0,
    CY_CPU_PROTECTION_VIRGIN    = 1,
    CY_CPU_PROTECTION_NORMAL    = 2,
    CY_CPU_PROTECTION_SECURE    = 3,
    CY_CPU_PROTECTION_DEAD      = 4,
} cy_en_cpu_protection_type_t;

/** \} group_cpu_enums */

/**
* \addtogroup group_cpu_functions
* \{
*/

/* API's for the CPU Core wait function */
void Cy_Cpu_WaitClear(cy_en_cpu_core_type_t coreType);
void Cy_Cpu_WaitSet(cy_en_cpu_core_type_t coreType);

/* API's for the CPU TCM block functions */
void Cy_Cpu_TcmEnable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmDisable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmReadModifyWriteEnable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmReadModifyWriteDisable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmEccEnable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmEccDisable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmEccErrorInjEnable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmEccErrorInjDisable(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_TcmReadWaitStatesSet(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType, uint8_t waitStates);
uint8_t Cy_Cpu_TcmReadWaitStatesGet(cy_en_cpu_core_type_t coreType, cy_en_cpu_tcm_type_t tcmType);
void Cy_Cpu_Cm7TcmSlavePortAccessEnable(cy_en_cpu_core_type_t coreType);
void Cy_Cpu_Cm7TcmSlavePortAccessDisable(cy_en_cpu_core_type_t coreType);

/* API's for the CPU Power Mode functions */
void Cy_Cpu_Cm7PowerModeSet(cy_en_cpu_core_type_t coreType, cy_en_cpu_power_mode_t pwrMode);
cy_en_cpu_power_mode_t Cy_Cpu_Cm7PowerModeGet(cy_en_cpu_core_type_t coreType);

/* API's for the CPU SRAM module functionality */
void Cy_Cpu_SramEccEnable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccDisable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccAutoCorrectionEnable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccAutoCorrectionDisable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccErrorInjEnable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccErrorInjDisable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccCheckEnable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramEccCheckDisable(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramPowerModeSet(cy_en_cpu_sram_macro_t sramType, cy_en_cpu_sram_power_mode_t pwrMode, uint8_t sramMacro);
cy_en_cpu_sram_power_mode_t Cy_Cpu_SramPowerModeGet(cy_en_cpu_sram_macro_t sramType, uint8_t sramMacro);
bool Cy_Cpu_SramWriteBufferStatus(cy_en_cpu_sram_macro_t sramType);
void Cy_Cpu_SramWaitStateSet(cy_en_cpu_sram_macro_t sramType, cy_en_cpu_clk_type_t clkType, uint8_t value);
uint8_t Cy_Cpu_SramWaitStateGet(cy_en_cpu_sram_macro_t sramType, cy_en_cpu_clk_type_t clkType);

/* API's for the ECC block error injection and parity function */
void Cy_Cpu_ErrorInjWordAddress(uint32_t wordAddr);
void Cy_Cpu_EccParityForErrorInj(uint8_t eccParity);

/* API's to get the CPU current Protection State */
cy_en_cpu_protection_type_t Cy_Cpu_ProtectionStateGet(void);

/* API's for CPU Fast Clock Divider functions */
void Cy_Cpu_CoreFastClkDividerSet(cy_en_cpu_core_type_t coreType, cy_en_cpu_div_type_t divType, uint8_t divValue);
uint8_t Cy_Cpu_CoreFastClkDividerGet(cy_en_cpu_core_type_t coreType, cy_en_cpu_div_type_t divType);

/* API's for CPU Memory Clock Divider functions */
void Cy_Cpu_CoreMemDividerSet(uint8_t divValue);
uint8_t Cy_Cpu_CoreMemDividerGet(void);

/* API's for CPU Peripheral Clock Divider functions */
void Cy_Cpu_CorePeriDividerSet(uint8_t divValue);
uint8_t Cy_Cpu_CorePeriDividerGet(void);

/* API's for CPU Trace/Debug Clock Divider functions */
void Cy_Cpu_CoreTraceDbgDividerSet(uint8_t divValue);
uint8_t Cy_Cpu_CoreTraceDbgDividerGet(void);

/* API's for CPU Core Slow Clock Divider functions */
void Cy_Cpu_CoreSlowClkDividerSet(uint8_t divValue);
uint8_t Cy_Cpu_CoreSlowClkDividerGet(void);

/* API's for Getting device parameter */
void Cy_Cpu_GetProductId(uint32_t* pFamilyId, uint32_t* pMajorRevisionId, uint32_t* pMinorRevisionId);

/** \} group_cpu_functions */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __CY_CPU_H__ */

/** \} group_cpu */


/******************************************************************************/
/* [] END OF FILE */
/******************************************************************************/
