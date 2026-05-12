/***************************************************************************//**
* \file startup_customize.h
* \version 1.0
*
* \brief
* C startup code before main() (ANSI C context not yet established!)
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(STARTUP_CUSTOMIZE_H)
#define STARTUP_CUSTOMIZE_H

#include <stddef.h>
#include <stdbool.h>
#include "cy_device_headers.h"


#if defined(__cplusplus)
extern "C" {
#endif

// Note: The STARTUP_CUSTOMIZE__ENABLE_* macros below can be commented/uncommented to disable/enable the respective feature


/** Enables/Disables assertions within the startup code (default implementation for failed assertions: endless loop) */
#define STARTUP_CUSTOMIZE__ENABLE_ASSERTIONS

/** Enables/Disables the ECC initialization of the memories (SRAM, and DTCM/ITCM if available) */
#define STARTUP_CUSTOMIZE__ENABLE_ECC_INITIALIZATION

/** Enables/Disables the Cortex-M7 instruction cache (setting is ignored in case of Cortex-M0+ or Cortex-M4F) */
#define STARTUP_CUSTOMIZE__ENABLE_CM7_INSTRUCTION_CACHE

/** Enables/Disables the Cortex-M7 data cache (setting is ignored in case of Cortex-M0+ or Cortex-M4F) */
#define STARTUP_CUSTOMIZE__ENABLE_CM7_DATA_CACHE


/** Declarations for symbols created in the linker file */
extern void * __ecc_init_sram_start_address;
extern void * __ecc_init_sram_end_address;
extern void * __ecc_init_dtcm_start_address;
extern void * __ecc_init_dtcm_end_address;
extern void * __ecc_init_itcm_start_address;
extern void * __ecc_init_itcm_end_address;

/** SRAM start and end address for ECC initialisation */
#define STARTUP_CUSTOMIZE__ECC_INIT_SRAM_START_ADDRESS       ((uint32_t) &__ecc_init_sram_start_address)
#define STARTUP_CUSTOMIZE__ECC_INIT_SRAM_END_ADDRESS         ((uint32_t) &__ecc_init_sram_end_address)

/** DTCM start and end address for ECC initialisation (setting is ignored in case of Cortex-M0+ or Cortex-M4F) */
#define STARTUP_CUSTOMIZE__ECC_INIT_DTCM_START_ADDRESS       ((uint32_t) &__ecc_init_dtcm_start_address)
#define STARTUP_CUSTOMIZE__ECC_INIT_DTCM_END_ADDRESS         ((uint32_t) &__ecc_init_dtcm_end_address)

/** ITCM start and end address for ECC initialisation (setting is ignored in case of Cortex-M0+ or Cortex-M4F) */
#define STARTUP_CUSTOMIZE__ECC_INIT_ITCM_START_ADDRESS       ((uint32_t) &__ecc_init_itcm_start_address)
#define STARTUP_CUSTOMIZE__ECC_INIT_ITCM_END_ADDRESS         ((uint32_t) &__ecc_init_itcm_end_address)


/** Declarations for symbols created in startup_cm*.arm/s assembly file */
extern uint32_t Cy_u32StartupStackStartAddress;
extern uint32_t Cy_u32StartupStackEndAddress;
extern void * __Vectors_Size;
extern const cy_israddress __Vectors[];


#if defined(__cplusplus)
}
#endif

#endif /* STARTUP_CUSTOMIZE_H */


/* [] END OF FILE */
