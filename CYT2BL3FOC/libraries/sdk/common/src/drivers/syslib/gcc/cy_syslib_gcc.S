@/***************************************************************************//**
@* \file cy_syslib_gcc.S
@* \version 2.0
@*
@* \brief Assembly routines for GNU Toolchain.
@*
@********************************************************************************
@* \copyright
@* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
@* You may use this file only in accordance with the license, terms, conditions,
@* disclaimers, and limitations in the end user license agreement accompanying
@* the software package with which this file was provided.
@*******************************************************************************/
    
    .syntax unified

    .global Cy_SysLib_DelayCycles
    .global Cy_SysLib_EnterCriticalSection
    .global Cy_SysLib_ExitCriticalSection
    
    .text
    .thumb


@/*******************************************************************************
@* Function Name: Cy_SysLib_DelayCycles
@****************************************************************************//**
@*
@* Delays for the specified number of cycles.
@*
@* \param uint32_t cycles: The number of cycles to delay.
@*
@*******************************************************************************/
@/* void Cy_SysLib_DelayCycles(uint32_t cycles) */
    
    .type Cy_SysLib_DelayCycles, %function

Cy_SysLib_DelayCycles:
    ADDS r0, r0, #2
    LSRS r0, r0, #2
    BEQ Cy_DelayCycles_done
Cy_DelayCycles_loop:
    ADDS r0, r0, #1
    SUBS r0, r0, #2
    BNE Cy_DelayCycles_loop
    NOP
Cy_DelayCycles_done:
    BX lr
    .size Cy_SysLib_DelayCycles, .-Cy_SysLib_DelayCycles


@/*******************************************************************************
@* Function Name: Cy_SysLib_EnterCriticalSection
@****************************************************************************//**
@*
@* Cy_SysLib_EnterCriticalSection disables interrupts and returns a value
@* indicating whether interrupts were previously enabled.
@*
@* Note Implementation of Cy_SysLib_EnterCriticalSection manipulates the IRQ
@* enable bit with interrupts still enabled. The test and set of the interrupt
@* bits are not atomic. Therefore, to avoid corrupting processor state, it must
@* be the policy that all interrupt routines restore the interrupt enable bits
@* as they were found on entry.
@*
@* \return Returns 0 if interrupts were previously enabled or 1 if interrupts
@* were previously disabled.
@*
@*******************************************************************************/
@/* uint8_t Cy_SysLib_EnterCriticalSection(void) */

    .type Cy_SysLib_EnterCriticalSection, %function

Cy_SysLib_EnterCriticalSection:
    MRS r0, PRIMASK         @ Save and return an interrupt state.
    CPSID I                 @ Disable interrupts.
    BX lr
    .size Cy_SysLib_EnterCriticalSection, .-Cy_SysLib_EnterCriticalSection

@/*******************************************************************************
@* Function Name: Cy_SysLib_ExitCriticalSection
@****************************************************************************//**
@*
@* Cy_SysLib_ExitCriticalSection re-enables the interrupts if they were enabled
@* before Cy_SysLib_EnterCriticalSection was called. The argument should be the
@* value returned from Cy_SysLib_EnterCriticalSection.
@*
@*  \param uint8_t savedIntrStatus:
@*   The saved interrupt status returned by the
@*   \ref Cy_SysLib_EnterCriticalSection().
@*
@*******************************************************************************/
@/* void Cy_SysLib_ExitCriticalSection(uint8_t savedIntrStatus) */

    .type Cy_SysLib_ExitCriticalSection, %function

Cy_SysLib_ExitCriticalSection:
    MSR PRIMASK, r0         @ Restore the interrupt state.
    BX lr
    .size Cy_SysLib_ExitCriticalSection, .-Cy_SysLib_ExitCriticalSection



    .end
