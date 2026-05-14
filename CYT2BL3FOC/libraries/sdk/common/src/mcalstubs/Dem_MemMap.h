/***************************************************************************//**
* \file Dem_MemMap.h
*
* \brief
* Dem_MemMap stub implementation
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

/*==================[inclusions]=============================================*/

/*==================[macros]=================================================*/

/*------------------[MemMap error checking]----------------------------------*/

/* PRQA S 883 1 */ /* AUTOSAR specification to be included more than once in a single compilation unit. */
#if (defined MEMMAP_ERROR) /* to prevent double definition */
#error MEMMAP_ERROR defined, wrong Dem_MemMap.h usage
#endif /* if (defined MEMMAP_ERROR) */

/** \brief Checks if this file has been correctly used
 **
 ** This definition checks if this file has been correctly included
 ** the last time.
 ** We do it here, before the big if-elif. */
#define MEMMAP_ERROR

/* PRQA S 841 EOF */ /* #undef to clear the macro definition of error detection. */

/*------------------[Start of module DEM ]--------------------------------------*/

#ifdef DEM_START_SEC_CODE
  #undef DEM_START_SEC_CODE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CODE_STARTED
#endif

#ifdef DEM_STOP_SEC_CODE
  #undef DEM_STOP_SEC_CODE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CODE_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CODE_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_BOOLEAN
  #undef DEM_START_SEC_VAR_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_BOOLEAN
  #undef DEM_STOP_SEC_VAR_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_8
  #undef DEM_START_SEC_VAR_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_8
  #undef DEM_STOP_SEC_VAR_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_16
  #undef DEM_START_SEC_VAR_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_16
  #undef DEM_STOP_SEC_VAR_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_32
  #undef DEM_START_SEC_VAR_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_32
  #undef DEM_STOP_SEC_VAR_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_UNSPECIFIED
  #undef DEM_START_SEC_VAR_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NOINIT_BOOLEAN
  #undef DEM_START_SEC_VAR_NOINIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NOINIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NOINIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_NOINIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NOINIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NOINIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NOINIT_8
  #undef DEM_START_SEC_VAR_NOINIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NOINIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NOINIT_8
  #undef DEM_STOP_SEC_VAR_NOINIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NOINIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NOINIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NOINIT_16
  #undef DEM_START_SEC_VAR_NOINIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NOINIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NOINIT_16
  #undef DEM_STOP_SEC_VAR_NOINIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NOINIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NOINIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NOINIT_32
  #undef DEM_START_SEC_VAR_NOINIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NOINIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NOINIT_32
  #undef DEM_STOP_SEC_VAR_NOINIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NOINIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NOINIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NOINIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_NOINIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NOINIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NOINIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_NOINIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NOINIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NOINIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_INIT_8
  #undef DEM_START_SEC_VAR_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_INIT_8
  #undef DEM_STOP_SEC_VAR_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_INIT_16
  #undef DEM_START_SEC_VAR_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_INIT_16
  #undef DEM_STOP_SEC_VAR_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_INIT_32
  #undef DEM_START_SEC_VAR_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_INIT_32
  #undef DEM_STOP_SEC_VAR_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_8
  #undef DEM_START_SEC_VAR_FAST_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_8
  #undef DEM_STOP_SEC_VAR_FAST_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_16
  #undef DEM_START_SEC_VAR_FAST_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_16
  #undef DEM_STOP_SEC_VAR_FAST_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_32
  #undef DEM_START_SEC_VAR_FAST_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_32
  #undef DEM_STOP_SEC_VAR_FAST_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_BOOLEAN
  #undef DEM_START_SEC_CONST_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_BOOLEAN
  #undef DEM_STOP_SEC_CONST_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_8
  #undef DEM_START_SEC_CONST_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_8_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_8
  #undef DEM_STOP_SEC_CONST_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_16
  #undef DEM_START_SEC_CONST_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_16_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_16
  #undef DEM_STOP_SEC_CONST_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_32
  #undef DEM_START_SEC_CONST_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_32_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_32
  #undef DEM_STOP_SEC_CONST_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_UNSPECIFIED
  #undef DEM_START_SEC_CONST_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_UNSPECIFIED
  #undef DEM_STOP_SEC_CONST_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONFIG_DATA_BOOLEAN
  #undef DEM_START_SEC_CONFIG_DATA_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONFIG_DATA_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_CONFIG_DATA_BOOLEAN
  #undef DEM_STOP_SEC_CONFIG_DATA_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONFIG_DATA_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONFIG_DATA_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONFIG_DATA_8
  #undef DEM_START_SEC_CONFIG_DATA_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONFIG_DATA_8_STARTED
#endif

#ifdef DEM_STOP_SEC_CONFIG_DATA_8
  #undef DEM_STOP_SEC_CONFIG_DATA_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONFIG_DATA_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONFIG_DATA_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONFIG_DATA_16
  #undef DEM_START_SEC_CONFIG_DATA_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONFIG_DATA_16_STARTED
#endif

#ifdef DEM_STOP_SEC_CONFIG_DATA_16
  #undef DEM_STOP_SEC_CONFIG_DATA_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONFIG_DATA_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONFIG_DATA_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONFIG_DATA_32
  #undef DEM_START_SEC_CONFIG_DATA_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONFIG_DATA_32_STARTED
#endif

#ifdef DEM_STOP_SEC_CONFIG_DATA_32
  #undef DEM_STOP_SEC_CONFIG_DATA_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONFIG_DATA_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONFIG_DATA_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONFIG_DATA_UNSPECIFIED
  #undef DEM_START_SEC_CONFIG_DATA_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONFIG_DATA_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_CONFIG_DATA_UNSPECIFIED
  #undef DEM_STOP_SEC_CONFIG_DATA_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONFIG_DATA_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONFIG_DATA_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_CLEARED_8
  #undef DEM_START_SEC_VAR_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_CLEARED_8
  #undef DEM_STOP_SEC_VAR_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_CLEARED_16
  #undef DEM_START_SEC_VAR_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_CLEARED_16
  #undef DEM_STOP_SEC_VAR_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_CLEARED_32
  #undef DEM_START_SEC_VAR_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_CLEARED_32
  #undef DEM_STOP_SEC_VAR_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_INIT_8
  #undef DEM_START_SEC_VAR_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_INIT_8
  #undef DEM_STOP_SEC_VAR_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_INIT_16
  #undef DEM_START_SEC_VAR_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_INIT_16
  #undef DEM_STOP_SEC_VAR_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_INIT_32
  #undef DEM_START_SEC_VAR_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_INIT_32
  #undef DEM_STOP_SEC_VAR_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_CLEARED_8
  #undef DEM_START_SEC_VAR_FAST_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_CLEARED_8
  #undef DEM_STOP_SEC_VAR_FAST_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_CLEARED_16
  #undef DEM_START_SEC_VAR_FAST_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_CLEARED_16
  #undef DEM_STOP_SEC_VAR_FAST_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_CLEARED_32
  #undef DEM_START_SEC_VAR_FAST_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_CLEARED_32
  #undef DEM_STOP_SEC_VAR_FAST_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_CLEARED_8
  #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_8
  #undef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_CLEARED_16
  #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_16
  #undef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_CLEARED_32
  #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_32
  #undef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_NO_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_NO_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_NO_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_NO_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_NO_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_NO_INIT_8
  #undef DEM_START_SEC_VAR_FAST_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_NO_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_NO_INIT_8
  #undef DEM_STOP_SEC_VAR_FAST_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_NO_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_NO_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_NO_INIT_16
  #undef DEM_START_SEC_VAR_FAST_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_NO_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_NO_INIT_16
  #undef DEM_STOP_SEC_VAR_FAST_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_NO_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_NO_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_NO_INIT_32
  #undef DEM_START_SEC_VAR_FAST_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_NO_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_NO_INIT_32
  #undef DEM_STOP_SEC_VAR_FAST_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_NO_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_NO_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_NO_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_NO_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_NO_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_NO_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_NO_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_8
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_8
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_16
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_16
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_32
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_32
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_8
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_8
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_16
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_16
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_32
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_32
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_POWER_ON_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_FAST_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_FAST_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_INIT_8
  #undef DEM_START_SEC_VAR_FAST_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_INIT_8
  #undef DEM_STOP_SEC_VAR_FAST_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_INIT_16
  #undef DEM_START_SEC_VAR_FAST_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_INIT_16
  #undef DEM_STOP_SEC_VAR_FAST_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_INIT_32
  #undef DEM_START_SEC_VAR_FAST_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_INIT_32
  #undef DEM_STOP_SEC_VAR_FAST_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_FAST_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_FAST_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_FAST_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_FAST_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_FAST_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_FAST_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_FAST_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_NO_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_SLOW_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_NO_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_NO_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SLOW_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_NO_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_NO_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_NO_INIT_8
  #undef DEM_START_SEC_VAR_SLOW_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_NO_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_NO_INIT_8
  #undef DEM_STOP_SEC_VAR_SLOW_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_NO_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_NO_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_NO_INIT_16
  #undef DEM_START_SEC_VAR_SLOW_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_NO_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_NO_INIT_16
  #undef DEM_STOP_SEC_VAR_SLOW_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_NO_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_NO_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_NO_INIT_32
  #undef DEM_START_SEC_VAR_SLOW_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_NO_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_NO_INIT_32
  #undef DEM_STOP_SEC_VAR_SLOW_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_NO_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_NO_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_NO_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_8
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_8
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_16
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_16
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_32
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_32
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_8
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_8
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_16
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_16
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_32
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_32
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_POWER_ON_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_SLOW_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SLOW_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_INIT_8
  #undef DEM_START_SEC_VAR_SLOW_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_INIT_8
  #undef DEM_STOP_SEC_VAR_SLOW_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_INIT_16
  #undef DEM_START_SEC_VAR_SLOW_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_INIT_16
  #undef DEM_STOP_SEC_VAR_SLOW_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_INIT_32
  #undef DEM_START_SEC_VAR_SLOW_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_INIT_32
  #undef DEM_STOP_SEC_VAR_SLOW_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SLOW_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SLOW_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_CLEARED_BOOLEAN
  #undef DEM_START_SEC_VAR_SLOW_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SLOW_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_CLEARED_8
  #undef DEM_START_SEC_VAR_SLOW_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_CLEARED_8
  #undef DEM_STOP_SEC_VAR_SLOW_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_CLEARED_16
  #undef DEM_START_SEC_VAR_SLOW_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_CLEARED_16
  #undef DEM_STOP_SEC_VAR_SLOW_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_CLEARED_32
  #undef DEM_START_SEC_VAR_SLOW_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_CLEARED_32
  #undef DEM_STOP_SEC_VAR_SLOW_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SLOW_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SLOW_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SLOW_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SLOW_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SLOW_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SLOW_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SLOW_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_INIT_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_INIT_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_INIT_8
  #undef DEM_START_SEC_INTERNAL_VAR_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_INIT_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_INIT_16
  #undef DEM_START_SEC_INTERNAL_VAR_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_INIT_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_INIT_32
  #undef DEM_START_SEC_INTERNAL_VAR_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_INIT_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_INIT_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_CLEARED_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_CLEARED_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_CLEARED_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_CLEARED_8
  #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_CLEARED_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_CLEARED_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_CLEARED_16
  #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_CLEARED_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_CLEARED_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_CLEARED_32
  #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_CLEARED_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_CLEARED_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_CLEARED_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_ZONE_BOOLEAN
  #undef DEM_START_SEC_VAR_SAVED_ZONE_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_ZONE_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_ZONE_BOOLEAN
  #undef DEM_STOP_SEC_VAR_SAVED_ZONE_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_ZONE_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_ZONE_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_ZONE_8
  #undef DEM_START_SEC_VAR_SAVED_ZONE_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_ZONE_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_ZONE_8
  #undef DEM_STOP_SEC_VAR_SAVED_ZONE_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_ZONE_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_ZONE_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_ZONE_16
  #undef DEM_START_SEC_VAR_SAVED_ZONE_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_ZONE_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_ZONE_16
  #undef DEM_STOP_SEC_VAR_SAVED_ZONE_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_ZONE_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_ZONE_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_ZONE_32
  #undef DEM_START_SEC_VAR_SAVED_ZONE_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_ZONE_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_ZONE_32
  #undef DEM_STOP_SEC_VAR_SAVED_ZONE_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_ZONE_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_ZONE_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_ZONE_UNSPECIFIED
  #undef DEM_START_SEC_VAR_SAVED_ZONE_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_ZONE_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_ZONE_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_SAVED_ZONE_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_ZONE_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_ZONE_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALIB_BOOLEAN
  #undef DEM_START_SEC_CALIB_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALIB_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_CALIB_BOOLEAN
  #undef DEM_STOP_SEC_CALIB_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALIB_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALIB_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALIB_8
  #undef DEM_START_SEC_CALIB_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALIB_8_STARTED
#endif

#ifdef DEM_STOP_SEC_CALIB_8
  #undef DEM_STOP_SEC_CALIB_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALIB_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALIB_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALIB_16
  #undef DEM_START_SEC_CALIB_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALIB_16_STARTED
#endif

#ifdef DEM_STOP_SEC_CALIB_16
  #undef DEM_STOP_SEC_CALIB_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALIB_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALIB_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALIB_32
  #undef DEM_START_SEC_CALIB_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALIB_32_STARTED
#endif

#ifdef DEM_STOP_SEC_CALIB_32
  #undef DEM_STOP_SEC_CALIB_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALIB_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALIB_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALIB_UNSPECIFIED
  #undef DEM_START_SEC_CALIB_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALIB_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_CALIB_UNSPECIFIED
  #undef DEM_STOP_SEC_CALIB_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALIB_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALIB_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CARTO_BOOLEAN
  #undef DEM_START_SEC_CARTO_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CARTO_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_CARTO_BOOLEAN
  #undef DEM_STOP_SEC_CARTO_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CARTO_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CARTO_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CARTO_8
  #undef DEM_START_SEC_CARTO_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CARTO_8_STARTED
#endif

#ifdef DEM_STOP_SEC_CARTO_8
  #undef DEM_STOP_SEC_CARTO_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CARTO_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CARTO_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CARTO_16
  #undef DEM_START_SEC_CARTO_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CARTO_16_STARTED
#endif

#ifdef DEM_STOP_SEC_CARTO_16
  #undef DEM_STOP_SEC_CARTO_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CARTO_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CARTO_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CARTO_32
  #undef DEM_START_SEC_CARTO_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CARTO_32_STARTED
#endif

#ifdef DEM_STOP_SEC_CARTO_32
  #undef DEM_STOP_SEC_CARTO_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CARTO_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CARTO_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CARTO_UNSPECIFIED
  #undef DEM_START_SEC_CARTO_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CARTO_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_CARTO_UNSPECIFIED
  #undef DEM_STOP_SEC_CARTO_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CARTO_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CARTO_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NO_INIT_BOOLEAN
  #undef DEM_START_SEC_VAR_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NO_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NO_INIT_BOOLEAN
  #undef DEM_STOP_SEC_VAR_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NO_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NO_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NO_INIT_8
  #undef DEM_START_SEC_VAR_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NO_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NO_INIT_8
  #undef DEM_STOP_SEC_VAR_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NO_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NO_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NO_INIT_16
  #undef DEM_START_SEC_VAR_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NO_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NO_INIT_16
  #undef DEM_STOP_SEC_VAR_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NO_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NO_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NO_INIT_32
  #undef DEM_START_SEC_VAR_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NO_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NO_INIT_32
  #undef DEM_STOP_SEC_VAR_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NO_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NO_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_NO_INIT_UNSPECIFIED
  #undef DEM_START_SEC_VAR_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_NO_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_NO_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_VAR_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_NO_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_NO_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_8
  #undef DEM_START_SEC_INTERNAL_VAR_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_16
  #undef DEM_START_SEC_INTERNAL_VAR_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_32
  #undef DEM_START_SEC_INTERNAL_VAR_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CALLOUT_CODE
  #undef DEM_START_SEC_CALLOUT_CODE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CALLOUT_CODE_STARTED
#endif

#ifdef DEM_STOP_SEC_CALLOUT_CODE
  #undef DEM_STOP_SEC_CALLOUT_CODE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CALLOUT_CODE_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CALLOUT_CODE_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CONST_SAVED_RECOVERY_ZONE
  #undef DEM_START_SEC_CONST_SAVED_RECOVERY_ZONE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CONST_SAVED_RECOVERY_ZONE_STARTED
#endif

#ifdef DEM_STOP_SEC_CONST_SAVED_RECOVERY_ZONE
  #undef DEM_STOP_SEC_CONST_SAVED_RECOVERY_ZONE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CONST_SAVED_RECOVERY_ZONE_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CONST_SAVED_RECOVERY_ZONE_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CODE_FAST
  #undef DEM_START_SEC_CODE_FAST
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CODE_FAST_STARTED
#endif

#ifdef DEM_STOP_SEC_CODE_FAST
  #undef DEM_STOP_SEC_CODE_FAST
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CODE_FAST_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CODE_FAST_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CODE_SLOW
  #undef DEM_START_SEC_CODE_SLOW
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CODE_SLOW_STARTED
#endif

#ifdef DEM_STOP_SEC_CODE_SLOW
  #undef DEM_STOP_SEC_CODE_SLOW
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CODE_SLOW_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CODE_SLOW_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_CODE_LIB
  #undef DEM_START_SEC_CODE_LIB
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_CODE_LIB_STARTED
#endif

#ifdef DEM_STOP_SEC_CODE_LIB
  #undef DEM_STOP_SEC_CODE_LIB
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_CODE_LIB_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_CODE_LIB_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_VAR_SAVED_RECOVERY_ZONE
  #undef DEM_START_SEC_VAR_SAVED_RECOVERY_ZONE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_VAR_SAVED_RECOVERY_ZONE_STARTED
#endif

#ifdef DEM_STOP_SEC_VAR_SAVED_RECOVERY_ZONE
  #undef DEM_STOP_SEC_VAR_SAVED_RECOVERY_ZONE
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_VAR_SAVED_RECOVERY_ZONE_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_VAR_SAVED_RECOVERY_ZONE_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_NO_INIT_8
  #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_NO_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_NO_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_NO_INIT_16
  #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_NO_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_NO_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_NO_INIT_32
  #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_NO_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_NO_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_NO_INIT_UNSPECIFIED_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_BOOLEAN_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_8
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_8_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_8
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_8
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_8_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_8_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_16
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_16_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_16
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_16
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_16_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_16_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_32
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_32_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_32
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_32
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_32_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_32_STARTED
  #endif
#endif

#ifdef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifdef MEMMAP_STARTED
    #error "MemMap error: Section started without stopping of previous section."
  #else
    #define MEMMAP_STARTED
  #endif
  #define DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
#endif

#ifdef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED
  #undef DEM_STOP_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED
  #ifndef MEMMAP_ERROR
    #error "MemMap error: Multiple sections selected."
  #endif
  #undef MEMMAP_ERROR
  #ifndef MEMMAP_STARTED
    #error "MemMap error: Section must be started before stopping."
  #else
    #undef MEMMAP_STARTED
  #endif
  #ifndef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
    #error "MemMap error: Stop section does not match start section."
  #else
    #undef DEM_START_SEC_INTERNAL_VAR_POWER_ON_INIT_UNSPECIFIED_STARTED
  #endif
#endif

/*------------------[MemMap error checking]----------------------------------*/

#if (defined MEMMAP_ERROR) /* to prevent double definition */
#error MEMMAP_ERROR defined, wrong Dem_MemMap.h usage
#endif /* if (defined MEMMAP_ERROR) */

/*==================[type definitions]=======================================*/

/*==================[external function declarations]=========================*/

/*==================[internal function declarations]=========================*/

/*==================[external constants]=====================================*/

/*==================[internal constants]=====================================*/

/*==================[external data]==========================================*/

/*==================[internal data]==========================================*/

/*==================[external function definitions]==========================*/

/*==================[internal function definitions]==========================*/

/*==================[end of file]============================================*/
