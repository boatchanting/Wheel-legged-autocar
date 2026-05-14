/***************************************************************************//**
* \file cmsis_diab.h
*
* \brief
* Wind River DIAB specific CMSIS definitions
*
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef __CMSIS_DIAB_H
#define __CMSIS_DIAB_H

#include <diab/asm.h>   // intrinsic functions

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                     __asm
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                restrict
#endif
#ifndef   __INLINE
  #define __INLINE                  __inline__
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE           static __inline__
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE      __attribute__((always_inline)) static __inline__
#endif 
#ifndef   __NO_RETURN
  #define __NO_RETURN               __attribute__((noreturn))
#endif
#ifndef   __USED
  #define __USED                    __attribute__((used))
#endif
#ifndef   __WEAK
  #define __WEAK                    __attribute__((weak))
#endif
#ifndef   __UNALIGNED_UINT32
  struct __attribute__((packed)) T_UINT32 { uint32_t v; };
  #define __UNALIGNED_UINT32(x)     (((struct T_UINT32 *)(x))->v)
#endif
#ifndef   __ALIGNED
  #define __ALIGNED(x)              __attribute__((aligned(x)))
#endif
#ifndef   __PACKED
  #define __PACKED                  __attribute__((packed, aligned(1)))
#endif
#ifndef   __PACKED_STRUCT
  #define __PACKED_STRUCT           struct __attribute__((packed, aligned(1)))
#endif  
#ifndef   __DMB
  #define __DMB()                   __ASM("dmb")
#endif  
#ifndef   __DSB
  #define __DSB()                   __ASM("dsb")
#endif  
#ifndef   __ISB
  #define __ISB()                   __ASM("isb")
#endif  
#ifndef   __NOP
  #define __NOP()                   __ASM("nop")
#endif
#ifndef   __WFI
  #define __WFI()                   __ASM("wfi")
#endif
#ifndef   __WFE
  #define __WFE()                   __ASM("wfe")
#endif
#ifndef   __enable_irq
  #define __enable_irq()            __ASM("cpsie i")
#endif
#ifndef   __disable_irq
  #define __disable_irq()           __ASM("cpsid i")
#endif
#ifndef   __COMPILER_BARRIER
  #define __COMPILER_BARRIER()      __asm volatile ("nop")//__memory_barrier()
#endif


#ifndef   __get_FPSCR
  #ifdef __VFP__
__asm volatile unsigned long __get_FPSCR(void)
{
! "r0"
    MRS r0, fpscr
}
  #else
      #define __get_FPSCR()    (0U)
  #endif
#endif

#ifndef __get_PRIMASK
__asm volatile unsigned long __get_PRIMASK(void)
{
! "r0"
    MRS r0, primask
}
#endif

#ifndef __set_PRIMASK
__asm volatile void __set_PRIMASK(unsigned long priMaskValue)
{
% reg priMaskValue
!
    MSR primask, priMaskValue
}
#endif


#ifndef __get_BASEPRI
__asm volatile unsigned long __get_BASEPRI(void)
{
! "r0"
    MRS r0, basepri
}
#endif

#ifndef __set_BASEPRI
__asm volatile void __set_BASEPRI(unsigned long basepriValue)
{
% reg basepriValue
!
    MSR basepri, basepriValue
}
#endif

#ifndef __get_IPSR
__asm volatile unsigned long __get_IPSR(void)
{
! "r0"
    MRS r0, ipsr
}
#endif

#endif /* __CMSIS_DIAB_H */
