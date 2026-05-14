/***************************************************************************//**
* \file cmsis_ghs.h
*
* \brief
* GHS specific CMSIS definitions
*
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef __CMSIS_GHS_H
#define __CMSIS_GHS_H

#include <arm_ghs.h>   // GHS MULTI intrinsic functions (e.g. __enable_interrupt())
#include <ghs_barrier.h> 

/* CMSIS compiler specific defines */
#ifndef   __ASM
  #define __ASM                     __asm
#endif
#ifndef   __RESTRICT
  #define __RESTRICT                restrict
#endif
#ifndef   __INLINE
  #define __INLINE                  inline
#endif
#ifndef   __STATIC_INLINE
  #define __STATIC_INLINE           static inline
#endif
#ifndef   __STATIC_FORCEINLINE
  #define __STATIC_FORCEINLINE      static inline
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
  #define __COMPILER_BARRIER()      __memory_changed()
#endif
#ifndef   __get_FPSCR
  /**
    \brief   Get FPSCR
    \details Returns the current value of the Floating Point Status/Control register.
    \return               Floating Point Status/Control register value
   */
  __attribute__((always_inline)) __STATIC_INLINE uint32_t __get_FPSCR(void)
  {
  #if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
       (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
      return __VMRS(__VFP_FPSCR);;
  #else
      return(0U);
  #endif
  }
#endif

#ifndef __get_PRIMASK
  #define __get_PRIMASK()             (__MRS(__PRIMASK))
#endif
#ifndef __set_PRIMASK
  #define __set_PRIMASK(VALUE)        (__MSR(__PRIMASK, (VALUE)))
#endif


#endif /* __CMSIS_GHS_H */
