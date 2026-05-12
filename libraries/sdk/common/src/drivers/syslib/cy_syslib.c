/***************************************************************************//**
* \file cy_syslib.c
* \version 1.0
*
*  Description:
*   Provides an API declaration of the syslib driver
*
********************************************************************************
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_syslib.h"
#include "ipc/cy_ipc_drv.h"

/* $TODO HXSH: Frequency threshold should be checked. */
/* Flash wait states (LP mode at 1.1v) */
#define CY_SYSLIB_FLASH_LP_WS_0_FREQ_MAX    (100UL)
#define CY_SYSLIB_FLASH_LP_WS_1_FREQ_MAX    (CY_HF_CLK_MAX_FREQ / 1000000UL)

/* Flash wait states (ULP mode at 0.9v) */
#define CY_SYSLIB_FLASH_ULP_WS_0_FREQ_MAX    ( 16UL)
#define CY_SYSLIB_FLASH_ULP_WS_1_FREQ_MAX    ( 33UL)
#define CY_SYSLIB_FLASH_ULP_WS_2_FREQ_MAX    ( 50UL)

/* ROM wait states for the slow clock domain (LP mode at 1.1v) */
#define CY_SYSLIB_ROM_LP_SLOW_WS_0_FREQ_MAX  (100UL)
#define CY_SYSLIB_ROM_LP_SLOW_WS_1_FREQ_MAX  (120UL)

/* ROM wait states for the slow clock domain (ULP mode at 0.9v) */
#define CY_SYSLIB_ROM_ULP_SLOW_WS_0_FREQ_MAX ( 25UL)
#define CY_SYSLIB_ROM_ULP_SLOW_WS_1_FREQ_MAX ( 50UL)

/* ROM wait states for the fast clock domain (LP mode at 1.1v) */
#define CY_SYSLIB_ROM_LP_FAST_WS_0_FREQ_MAX  (120UL)

/* ROM wait states for the slow clock domain (ULP mode at 0.9v) */
#define CY_SYSLIB_ROM_ULP_FAST_WS_0_FREQ_MAX ( 50UL)

/* SRAM wait states for the slow clock domain (LP mode at 1.1v) */
#define CY_SYSLIB_RAM_LP_SLOW_WS_0_FREQ_MAX  (100UL)
#define CY_SYSLIB_RAM_LP_SLOW_WS_1_FREQ_MAX  (120UL)

/* SRAM wait states for the slow clock domain (ULP mode at 0.9v) */
#define CY_SYSLIB_RAM_ULP_SLOW_WS_0_FREQ_MAX ( 25UL)
#define CY_SYSLIB_RAM_ULP_SLOW_WS_1_FREQ_MAX ( 50UL)

/* SRAM wait states for the fast clock domain (LP mode at 1.1v) */
#define CY_SYSLIB_RAM_LP_FAST_WS_0_FREQ_MAX  (120UL)

/* SRAM wait states for the fast clock domain (ULP mode at 0.9v) */
#define CY_SYSLIB_RAM_ULP_FAST_WS_0_FREQ_MAX ( 50UL)


#if !defined(NDEBUG)
    char_t cy_assertFileName[CY_MAX_FILE_NAME_SIZE];
    uint32_t cy_assertLine;
#endif /* NDEBUG */

#if (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED)
    cy_stc_fault_frame_t cy_faultFrame;
#endif /* (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) */


static void Cy_SysLib_SetWaitStates_LP(uint32_t clkHfMHz);


/*******************************************************************************
* Function Name: Cy_SysLib_Delay
****************************************************************************//**
*
* The function delays by the specified number of milliseconds.
* By default, the number of cycles to delay is calculated based on the
* \ref SystemCoreClock.
*
* \param milliseconds  The number of milliseconds to delay.
*
* \note The function calls \ref Cy_SysLib_DelayCycles() API to generate a delay.
*       If the function parameter (milliseconds) is bigger than 
*       CY_DELAY_MS_OVERFLOW constant, then an additional loop runs to prevent
*       an overflow in parameter passed to \ref Cy_SysLib_DelayCycles() API.
*
*******************************************************************************/
void Cy_SysLib_Delay(uint32_t milliseconds)
{
    while(milliseconds > CY_DELAY_MS_OVERFLOW)
    {
        /* This loop prevents an overflow in value passed to Cy_SysLib_DelayCycles() API.
         * At 100 MHz, (milliseconds * cy_delayFreqKhz) product overflows
         * in case if milliseconds parameter is more than 42 seconds.
         */
        Cy_SysLib_DelayCycles(cy_delay32kMs);
        milliseconds -= CY_DELAY_MS_OVERFLOW;
    }

    Cy_SysLib_DelayCycles(milliseconds * cy_delayFreqKhz);
}


/*******************************************************************************
* Function Name: Cy_SysLib_DelayUs
****************************************************************************//**
*
* The function delays by the specified number of microseconds.
* By default, the number of cycles to delay is calculated based on the
* \ref SystemCoreClock.
*
* \param microseconds  The number of microseconds to delay.
*
* \note If the CPU frequency is a small non-integer number, the actual delay
*       can be up to twice as long as the nominal value. The actual delay
*       cannot be shorter than the nominal one.
*
*******************************************************************************/
void Cy_SysLib_DelayUs(uint16_t microseconds)
{
    Cy_SysLib_DelayCycles((uint32_t) microseconds * cy_delayFreqMhz);
}


/*******************************************************************************
* Function Name: Cy_SysLib_Halt
****************************************************************************//**
*
* This function halts the CPU but only the CPU which calls the function.
* It doesn't affect other CPUs.
*
* \param reason  The value to be used during debugging.
*
* \note The function executes the BKPT instruction for halting CPU and is
*       intended to be used for the debug purpose. A regular use case requires
*       Debugger attachment before the function call.
*       The BKPT instruction causes the CPU to enter the Debug state. Debug
*       tools can use this to investigate the system state, when the
*       instruction at a particular address is reached.
*
* \note Execution of a BKPT instruction without a debugger attached produces
*       a fault. The fault results in the HardFault exception being taken
*       or causes a Lockup state if it occurs in the NMI or HardFault handler.
*       The default HardFault handler make a software reset if the build option
*       is the release mode (NDEBUG). If the build option is the debug mode,
*       the system will stay in the infinite loop of the
*       \ref Cy_SysLib_ProcessingFault() function.
*
*******************************************************************************/
__NO_RETURN void Cy_SysLib_Halt(uint32_t reason)
{
    if(0u != reason)
    {
        /* To remove an unreferenced local variable warning */
    }

    #if defined (__ARMCC_VERSION)
        __breakpoint(0x0);
    #elif defined(__GNUC__)
        __asm("    bkpt    1");
        __builtin_unreachable();
    #elif defined (__ICCARM__)
        __asm("    bkpt    1");
    #elif defined (__ghs__)
        __asm("    bkpt    1");
    #elif defined (__DCC__)
        __asm("    bkpt   #1");
    #else
        #error "Unsupported toolchain"
    #endif  /* (__ARMCC_VERSION) */

    while(1u) {}
}


/*******************************************************************************
* Function Name: Cy_SysLib_ClearFlashCacheAndBuffer
****************************************************************************//**
*
*  This function invalidates the flash cache and buffer. It ensures the valid
*  data is read from flash instead of using outdated data from the cache.
*  The caches' LRU structure is also reset to their default state.
*
*  \note The operation takes a maximum of three clock cycles on the slowest of
*        the clk_slow and clk_fast clocks.
*
*******************************************************************************/
void Cy_SysLib_ClearFlashCacheAndBuffer(void)
{
#if !defined (tviibh16m)
    FLASHC->unFLASH_CMD.u32Register = FLASHC_FLASH_CMD_INV_Msk;
#endif
}

/*******************************************************************************
* Function Name: Cy_SysLib_GetUniqueId
****************************************************************************//**
*
* This function returns the silicon unique ID.
* The ID includes Die lot[3]#, Die Wafer#, Die X, Die Y, DIE Sort# and
* Die Minor#
*
* \return
* A combined 64-bit unique ID.
* [63:56] - DIE_MINOR
* [55:48] - DIE_SORT
* [47:40] - DIE_Y
* [39:32] - DIE_X
* [31:24] - DIE_WAFER
* [23:16] - DIE_LOT[2]
* [15: 8] - DIE_LOT[1]
* [ 7: 0] - DIE_LOT[0]
*
*******************************************************************************/
uint64_t Cy_SysLib_GetUniqueId(void)
{
    uint64_t uniqueId = 0u;

//rmkn: to be checked
#if (0) // !defined (tviibe2m)
    uniqueId = ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_MINOR.u8Register   << CY_UNIQUE_ID_DIE_MINOR_Pos) |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_SORT.u8Register    << CY_UNIQUE_ID_DIE_SORT_Pos)  |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_Y.u8Register       << CY_UNIQUE_ID_DIE_Y_Pos)     |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_X.u8Register       << CY_UNIQUE_ID_DIE_X_Pos)     |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_WAFER.u8Register   << CY_UNIQUE_ID_DIE_WAFER_Pos) |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_LOT[2u].u8Register << CY_UNIQUE_ID_DIE_LOT_2_Pos) |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_LOT[1u].u8Register << CY_UNIQUE_ID_DIE_LOT_1_Pos) |
               ((uint64_t)((stc_SFLASH_t *)SFLASH_BASE)->unDIE_LOT[0u].u8Register << CY_UNIQUE_ID_DIE_LOT_0_Pos);
#endif
    return (uniqueId);
}


#if (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) || defined(CY_DOXYGEN)
/*******************************************************************************
* Function Name: Cy_SysLib_FaultHandler
****************************************************************************//**
*
* This function stores the ARM Cortex registers into a non-zero init area for
* debugging. This function calls Cy_SysLib_ProcessingFault() after storing all
* the information.
*
* \param faultStackAddr The address of the stack pointer, indicates the lowest
*                       address in the fault stack frame to be stored.
* \note This function stores the fault stack frame only for the first occurred
*       fault.
* \note This function sets two flags into the fault structure: one checks
*       which core cause fault, another checks the type of fault for CM4 core.
*       The flags should be cleared after analysis of the stored
*       register values, to have possibility to distinguish the next fault.
* \note The PDL doesn't provide an API which can analyse the stored register
*       values. A user has to add additional functions for the analysis,
*       if necessary.
* \note The CY_ARM_FAULT_DEBUG macro defines if the Fault Handler is enabled.
*       By default it is set to CY_ARM_FAULT_DEBUG_ENABLED and enables the
*       Fault Handler.
*       If there is a necessity to save memory or have some specific custom
*       handler, etc. then CY_ARM_FAULT_DEBUG should be redefined as
*       CY_ARM_FAULT_DEBUG_DISABLED. To do this the following definition should
*       be added to the compiler Command Line (through the project Build
*       Settings): "-D CY_ARM_FAULT_DEBUG=0".
*
*******************************************************************************/
void Cy_SysLib_FaultHandler(uint32_t const *faultStackAddr)
{
    #if (CY_CPU_CORTEX_M0P)
        if(CY_CM0P_FAULT_TYPE != cy_faultFrame.fault)
        {
            cy_faultFrame.fault = CY_CM0P_FAULT_TYPE;    /* CM0P fault occurred. */
    #else
      #if (CY_CPU_CORTEX_M4)
        if(CY_CM4_FAULT_TYPE != cy_faultFrame.fault)
        {
            cy_faultFrame.fault = CY_CM4_FAULT_TYPE;     /* CM4 fault occurred. */
      #elif (CY_CPU_CORTEX_M7)
        if(CY_CM7_FAULT_TYPE != cy_faultFrame.fault)
        {
            cy_faultFrame.fault = CY_CM7_FAULT_TYPE;     /* CM7 fault occurred. */
      #endif
            /* Stores the Configurable Fault Status Register state with the fault cause */
            cy_faultFrame.cfsr = SCB->CFSR;
            cy_faultFrame.faultType = CY_NON_FPU_FAULT_TYPE;
    #endif /* CY_CPU_CORTEX_M0P */
            /* Stores general registers */
            cy_faultFrame.r0  = faultStackAddr[CY_R0_Pos];
            cy_faultFrame.r1  = faultStackAddr[CY_R1_Pos];
            cy_faultFrame.r2  = faultStackAddr[CY_R2_Pos];
            cy_faultFrame.r3  = faultStackAddr[CY_R3_Pos];
            cy_faultFrame.r12 = faultStackAddr[CY_R12_Pos];
            cy_faultFrame.lr  = faultStackAddr[CY_LR_Pos];
            cy_faultFrame.pc  = faultStackAddr[CY_PC_Pos];
            cy_faultFrame.psr = faultStackAddr[CY_PSR_Pos];

            #if ((CY_CPU_CORTEX_M7) || (CY_CPU_CORTEX_M4)) && ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
                                       (defined (__FPU_USED   ) && (__FPU_USED    == 1U)))
                /* Checks cumulative exception bits for floating-point exceptions */
                if(0U != (__get_FPSCR() & (CY_FPSCR_IXC_Msk | CY_FPSCR_IDC_Msk)))
                {
                    cy_faultFrame.faultType = CY_FPU_FAULT_TYPE;
                    cy_faultFrame.s0    = faultStackAddr[CY_S0_Pos];
                    cy_faultFrame.s1    = faultStackAddr[CY_S1_Pos];
                    cy_faultFrame.s2    = faultStackAddr[CY_S2_Pos];
                    cy_faultFrame.s3    = faultStackAddr[CY_S3_Pos];
                    cy_faultFrame.s4    = faultStackAddr[CY_S4_Pos];
                    cy_faultFrame.s5    = faultStackAddr[CY_S5_Pos];
                    cy_faultFrame.s6    = faultStackAddr[CY_S6_Pos];
                    cy_faultFrame.s7    = faultStackAddr[CY_S7_Pos];
                    cy_faultFrame.s8    = faultStackAddr[CY_S8_Pos];
                    cy_faultFrame.s9    = faultStackAddr[CY_S9_Pos];
                    cy_faultFrame.s10   = faultStackAddr[CY_S10_Pos];
                    cy_faultFrame.s11   = faultStackAddr[CY_S11_Pos];
                    cy_faultFrame.s12   = faultStackAddr[CY_S12_Pos];
                    cy_faultFrame.s13   = faultStackAddr[CY_S13_Pos];
                    cy_faultFrame.s14   = faultStackAddr[CY_S14_Pos];
                    cy_faultFrame.s15   = faultStackAddr[CY_S15_Pos];
                    cy_faultFrame.fpscr = faultStackAddr[CY_FPSCR_Pos];
                }
            #endif /* CY_CPU_CORTEX_M4 && __FPU_PRESENT */
        }

    Cy_SysLib_ProcessingFault();
}


/*******************************************************************************
* Function Name: Cy_SysLib_ProcessingFault
****************************************************************************//**
*
* This function determines how to process the current fault state. By default
* in case of exception the system will stay in the infinite loop of this
* function.
*
* \note This function has WEAK option, so the user can redefine the function
*       for a custom processing.
*       For example the function redefinition could be constructed from fault
*       stack processing and NVIC_SystemReset() function call.
*
*******************************************************************************/
CY_WEAK void Cy_SysLib_ProcessingFault(void)
{
    while(1u) {}
}
#endif /* (CY_ARM_FAULT_DEBUG == CY_ARM_FAULT_DEBUG_ENABLED) || defined(CY_DOXYGEN) */

/*******************************************************************************
* Function Name: Cy_SysLib_SvcHandler
****************************************************************************//**
*
* This function is called when SVC instruction executed with a pointer to top
* of the frame stack as a input parameter.
* By default, this function returns svc index which was input by user.
*
* \param pSvcArgs The address of the stack pointer. Note that first byte of 
                  "PC" in the stack indicates SVC index.
*
*******************************************************************************/
CY_WEAK void Cy_SysLib_SvcHandler(uint32_t* pSvcArgs)
{
    // Stack frame contains:
    // r0, r1, r2, r3, r12, r14, the return address and xPSR
    // - Stacked R0  = pSvcArgs[0]
    // - Stacked R1  = pSvcArgs[1]
    // - Stacked R2  = pSvcArgs[2]
    // - Stacked R3  = pSvcArgs[3]
    // - Stacked R12 = pSvcArgs[4]
    // - Stacked LR  = pSvcArgs[5]
    // - Stacked PC  = pSvcArgs[6]
    // - Stacked xPSR= pSvcArgs[7]
    uint8_t svcIdx = ((char*)pSvcArgs[6])[-2];

    // return "svcIdx"
    pSvcArgs[0] = svcIdx;
}


/*******************************************************************************
* Function Name: Cy_SysLib_SetWaitStates
****************************************************************************//**
*
* Sets the number of clock cycles the cache will wait for, before it samples
* data coming back from ROM, SRAM, and Flash.
*
* Call this function before increasing the HFClk0 High Frequency clock.
* Call this function optionally after lowering the HFClk0 High Frequency clock
* in order to improve the CPU performance.
*
* Also, call this function before switching core supply regulator voltage (LDO
* or SIMO Buck) from 1.1V to 0.9V.
* Call this function optionally after switching core supply regulator voltage
* from 0.9V to 1.1V in order to improve the CPU performance.
*
* \param ulpMode The device power mode.
* true, if the device should be switched to the ULP mode (nominal voltage of
* the core supply regulator should be switched to 0.9V);
* false, if the device should be switched to the LP mode (nominal voltage of
* the core supply regulator should be switched to 1.1V).
*
* \note Refer to the device TRM for low power modes description.
*
* \param clkHfMHz The HFClk0 clock frequency in MHz. Specifying frequency above
* maximum supported will set the wait states as for the maximum frequency.
*
*******************************************************************************/
void Cy_SysLib_SetWaitStates(uint32_t clkHfMHz)
{
    Cy_SysLib_SetWaitStates_LP(clkHfMHz);
}


/*******************************************************************************
* Function Name: Cy_SysLib_SetWaitStates_LP
****************************************************************************//**
*
*  Sets the ROM, SRAM, and Flash wait states for the low power mode.
*  This function is called by \ref Cy_SysLib_SetWaitStates().
*
* \param clkHfMHz The HFClk0 clock frequency in MHz. Specifying frequency above
* maximum supported will set the wait states as for the maximum frequency.
*
*******************************************************************************/
static void Cy_SysLib_SetWaitStates_LP(uint32_t clkHfMHz)
{
    uint32_t waitStates;

    /* ROM */
    if (clkHfMHz < CY_SYSLIB_ROM_LP_SLOW_WS_0_FREQ_MAX)
    {
        waitStates = 0UL;
    } else
    {
        waitStates = 1UL;
    }
    CPUSS->unROM_CTL.stcField.u2SLOW_WS = waitStates;

    waitStates = 0UL;
    CPUSS->unROM_CTL.stcField.u2FAST_WS = waitStates;

    /* SRAM */
    if (clkHfMHz < CY_SYSLIB_RAM_LP_SLOW_WS_0_FREQ_MAX)
    {
        waitStates = 0UL;
    } else
    {
        waitStates = 1UL;
    }

    CPUSS->unRAM0_CTL0.stcField.u2SLOW_WS = waitStates;
    
    #if defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1UL)
        CPUSS->unRAM1_CTL0.stcField.u2SLOW_WS = waitStates;
    #endif /* defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1UL) */
        
    #if defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1UL)
        CPUSS->unRAM2_CTL0.stcField.u2SLOW_WS = waitStates;
    #endif /* defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1UL) */

    CPUSS->unRAM0_CTL0.stcField.u2FAST_WS = waitStates;
    
    #if defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1UL)
        CPUSS->unRAM1_CTL0.stcField.u2FAST_WS = waitStates;
    #endif /* defined (CPUSS_RAMC1_PRESENT) && (CPUSS_RAMC1_PRESENT == 1UL) */
        
    #if defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1UL)
        CPUSS->unRAM2_CTL0.stcField.u2FAST_WS = waitStates;
    #endif /* defined (CPUSS_RAMC2_PRESENT) && (CPUSS_RAMC2_PRESENT == 1UL) */

    /* Flash */
    if (clkHfMHz <= CY_SYSLIB_FLASH_LP_WS_0_FREQ_MAX)
    {
        waitStates = 0UL;
    }
    else
    {
        waitStates = 1UL;
    }

#if defined (tviibe512k) || defined (tviibe1m) || defined (tviibe2m)|| defined (tviibe4m) || defined (tviice4m)
    FLASHC->unFLASH_CTL.stcField.u4MAIN_WS = waitStates;
#elif defined (tviibh4m) || defined (tviibh8m) || defined (tviic2d6m) || defined (tviic2d4m) || defined (tviic2d6mddr) || defined (tviibh16m)
    // rmkn: to be reworked FLASHC->unFLASH_CTL.stcField.u4WS = waitStates;
#endif
}


/*******************************************************************************
* Function Name: Cy_SysLib_InvalidateCoreDCacheByAddr
****************************************************************************//**
*
* Invalidates D-cache in the core for the given address.
* The D-cache memory blocks which are part of given address + given size are invalidated.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_InvalidateCoreDCacheByAddr(void *addr, int32_t dsize)
{
#if (CY_CPU_CORTEX_M7) && (CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON)
    /** Call the ARM cache maintenance function. */
    SCB_InvalidateDCache_by_Addr(addr, dsize);
#endif
}

/*******************************************************************************
* Function Name: Cy_SysLib_CleanCoreDCacheByAddr
****************************************************************************//**
*
* Cleans D-cache in the core for the given address
* The D-cache memory blocks which are part of given address + given size are cleaned.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_CleanCoreDCacheByAddr(void *addr, int32_t dsize)
{
#if (CY_CPU_CORTEX_M7) && (CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON)
    /** Call the ARM cache maintenance function. */
    SCB_CleanDCache_by_Addr(addr, dsize);
#endif
}

/*******************************************************************************
* Function Name: Cy_SysLib_CleanInvalidateCoreDCacheByAddr
****************************************************************************//**
*
* Cleans and invalidates D-cache in the core for the given address
* The D-cache memory blocks which are part of given address + given size are cleaned
* and invalidated.
*
* \param addr address
* \param dsize size of memory block (in number of bytes)
*
* \note This function works only when called by the core with D-cache (e.g. CM7)
*       and CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON.
*
*******************************************************************************/
void Cy_SysLib_CleanInvalidateCoreDCacheByAddr(void *addr, int32_t dsize)
{
#if (CY_CPU_CORTEX_M7) && (CY_SYSLIB_CORE_D_CACHE_MAINTENANCE == CY_SYSLIB_CORE_D_CACHE_MAINTENANCE_ON)
    /** Call the ARM cache maintenance function. */
    SCB_CleanInvalidateDCache_by_Addr(addr, dsize);
#endif
}

/* [] END OF FILE */
