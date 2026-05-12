/***************************************************************************//**
* \file SchM_Common.h
*
* \brief
* Implements the macros to generate the SchM_Enter/Exit functions for the various
* SchM_<ModulePrefix> stub files
*
********************************************************************************
* \copyright
* Copyright 2016-2022, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(SCHM_COMMON_H)
#define SCHM_COMMON_H

/**
 ** \brief AUTOSAR SchM Schedule Manager STUB
 **   
 **  This file contains a common macros for declaration/definition
 **  exclusive are control functions.
 **
 **
 ** (c) 2020, Cypress Semiconductor Corporation. All rights reserved.
 **
 ** ALL RIGHTS RESERVED. No part of this publication may be copied and provided
 ** to any third party in any form or by any means, unless expressly agreed to
 ** in written form by Cypress. All trademarks used in this document are the
 ** property of their respective owners.
 **
 ** For further provisions please refer to the respective License Agreement.
 */

/*==================[inclusions]============================================*/

#include <Std_Types.h>

/*==================[macros]================================================*/

#define SCHM_DEFINE_EXCLUSIVE_AREA_VAR(ModuleName0)     \
VAR(TestEnv_IntStatusType, static) IntStatus;           \
VAR(uint32, static)                IntNestingLevel;     \
uint8 ModuleName0##_SchM_EnterCount;                    \
uint8 ModuleName0##_SchM_ExitCount;

#define SCHM_DECLARE_EXCLUSIVE_AREA_VAR(ModuleName0)    \
extern uint8 ModuleName0##_SchM_EnterCount;             \
extern uint8 ModuleName0##_SchM_ExitCount;

#define SCHM_DECLARE_EXCLUSIVE_AREA_FUNC(ModuleName0, MODULENAME1, ResNum)                                 \
extern FUNC(void,SCHM_CODE) SchM_Enter_##ModuleName0##_##MODULENAME1##_EXCLUSIVE_AREA_##ResNum (void);     \
extern FUNC(void,SCHM_CODE) SchM_Exit_##ModuleName0##_##MODULENAME1##_EXCLUSIVE_AREA_##ResNum (void);

#define SCHM_DEFINE_EXCLUSIVE_AREA_FUNC(ModuleName0, MODULENAME1, ResNum)                                           \
FUNC(void,SCHM_CODE) SchM_Enter_##ModuleName0##_##MODULENAME1##_EXCLUSIVE_AREA_##ResNum (void)                      \
{                                                                                                                   \
  TestEnv_IntStatusType PrevState = TestEnv_IntDisable();                                                           \
  if (0u == IntNestingLevel)                                                                                        \
  {                                                                                                                 \
    IntStatus = PrevState;                                                                                          \
  }                                                                                                                 \
  else                                                                                                              \
  {                                                                                                                 \
    Det_ReportError                                                                                                 \
    (                                                                                                               \
      130,  /* ModuleId: SchM */                                                                                    \
      0,    /* InstanceId */                                                                                        \
      3,    /* SercviceId: SchM_Enter */                                                                            \
      4     /* ErrorId: custom number, not defined by AUTOSAR */                                                    \
    );                                                                                                              \
  }                                                                                                                 \
  IntNestingLevel++;                                                                                                \
  ModuleName0##_SchM_EnterCount++;                                                                                  \
}                                                                                                                   \
                                                                                                                    \
FUNC(void,SCHM_CODE) SchM_Exit_##ModuleName0##_##MODULENAME1##_EXCLUSIVE_AREA_##ResNum (void)                       \
{                                                                                                                   \
  if (0 == IntNestingLevel)                                                                                         \
  {                                                                                                                 \
    Det_ReportError                                                                                                 \
    (                                                                                                               \
      130,  /* ModuleId: SchM */                                                                                    \
      0,    /* InstanceId */                                                                                        \
      4,    /* SercviceId: SchM_Exit */                                                                             \
      4     /* ErrorId: custom number, not defined by AUTOSAR */                                                    \
    );                                                                                                              \
  }                                                                                                                 \
  else                                                                                                              \
  {                                                                                                                 \
    IntNestingLevel--;                                                                                              \
  }                                                                                                                 \
  ModuleName0##_SchM_ExitCount++;                                                                                   \
                                                                                                                    \
  if (0u == IntNestingLevel)                                                                                        \
  {                                                                                                                 \
    TestEnv_IntRestore(IntStatus);                                                                                  \
  }                                                                                                                 \
}


/*==================[external function definitions]=========================*/

/*==================[internal function definitions]=========================*/

#endif /* SCHM_COMMON_H */
/*==================[end of file]===========================================*/
