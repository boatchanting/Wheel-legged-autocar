@/**************************************************************************//**
@ * @file     startup_cm4.S
@ * @brief    CMSIS Core Device Startup File for
@ *           ARMCM4 Device Series
@ ******************************************************************************/
@* \copyright
@* Copyright 2018-2021, Cypress Semiconductor Corporation.  All rights reserved.
@* You may use this file only in accordance with the license, terms, conditions,
@* disclaimers, and limitations in the end user license agreement accompanying
@* the software package with which this file was provided.
@*******************************************************************************/

        .syntax unified

        .thumb

@************************************************************************
@* Local definitions
@************************************************************************

@ Set a sufficient startup stack size for correct operation of C startup code (startup.c)
        .equ STARTUP_STACK_SIZE_DOUBLE_WORDS, 32

        .equ VTOR, 0xe000ed08


@************************************************************************
@* Import symbols
@************************************************************************

        .extern  CpuUserInt0_Handler
        .extern  CpuUserInt1_Handler
        .extern  CpuUserInt2_Handler
        .extern  CpuUserInt3_Handler
        .extern  CpuUserInt4_Handler
        .extern  CpuUserInt5_Handler
        .extern  CpuUserInt6_Handler
        .extern  CpuUserInt7_Handler
        .extern  Startup_Init
        .extern  main
        .extern  __data_lma
        .extern  __data_start
        .extern  __data_end
        .extern  __bss_start
        .extern  __bss_end
        .extern  __stack_end


@************************************************************************
@* Export symbols
@************************************************************************

        .global  _start
        .global  __vector_table
        .global  __vector_table_0x1c
        .global  __Vectors
        .global  __Vectors_End
        .global  __Vectors_Size
        .global  __ramVectors
        .global  Cy_SaveIRQ
        .global  Cy_RestoreIRQ
        .global  Cy_u32StartupStackStartAddress
        .global  Cy_u32StartupStackEndAddress

@************************************************************************
@* Vector Table and RAM Vector Table
@************************************************************************

        .section ".intvec", "a"
        @ align to 1024 byte, because CM4_VECTOR_TABLE_BASE register only supports address bits [31:10] (Note: for VTOR a 128-byte alignment would be ok)
        .balign 1024

        .type __vector_table, %object

__vector_table:
        .word     __stack_end
        .word     Reset_Handler
        .word     NMI_Handler
        .word     HardFault_Handler
        .word     MemManage_Handler
        .word     BusFault_Handler
        .word     UsageFault_Handler
__vector_table_0x1c:
        .word     0
        .word     0
        .word     0
        .word     0
        .word     SVC_Handler
        .word     DebugMon_Handler
        .word     0
        .word     PendSV_Handler
        .word     SysTick_Handler

        @ External interrupts                PowerMode    Description

        .word     CpuUserInt0_Handler      @ DeepSleep    CPU User Interrupt #0
        .word     CpuUserInt1_Handler      @ DeepSleep    CPU User Interrupt #1
        .word     CpuUserInt2_Handler      @ DeepSleep    CPU User Interrupt #2
        .word     CpuUserInt3_Handler      @ DeepSleep    CPU User Interrupt #3
        .word     CpuUserInt4_Handler      @ DeepSleep    CPU User Interrupt #4
        .word     CpuUserInt5_Handler      @ DeepSleep    CPU User Interrupt #5
        .word     CpuUserInt6_Handler      @ DeepSleep    CPU User Interrupt #6
        .word     CpuUserInt7_Handler      @ DeepSleep    CPU User Interrupt #7

        @ These IRQs can only be triggered by SW via NVIC regs
        .word     CpuUserInt8_Handler      @ Active       CPU User Interrupt #8
        .word     CpuUserInt9_Handler      @ Active       CPU User Interrupt #9
        .word     CpuUserInt10_Handler     @ Active       CPU User Interrupt #10
        .word     CpuUserInt11_Handler     @ Active       CPU User Interrupt #11
        .word     CpuUserInt12_Handler     @ Active       CPU User Interrupt #12
        .word     CpuUserInt13_Handler     @ Active       CPU User Interrupt #13
        .word     CpuUserInt14_Handler     @ Active       CPU User Interrupt #14
        .word     CpuUserInt15_Handler     @ Active       CPU User Interrupt #15

        .size __vector_table, .-__vector_table
__Vectors_End:

        .equ __Vectors, __vector_table
        .equ __Vectors_Size, __Vectors_End - __Vectors

        .section ".intvec_ram", "aw"
        @ use same alignment like vector table in ROM above (even though VTOR minimum requirement would be 128 bytes if not used in combination with CM4_VECTOR_TABLE_BASE register)
        .balign 1024

        .type __ramVectors, %object

__ramVectors:
        .space     __Vectors_Size
        .size __ramVectors, .-__ramVectors


@************************************************************************
@* Helper functions
@************************************************************************

        .text

@ Saves and disables the interrupts
        .type Cy_SaveIRQ, %function
Cy_SaveIRQ:
        MRS   r0, PRIMASK
        CPSID I
        BX    LR
        .size Cy_SaveIRQ, .-Cy_SaveIRQ


@-----------------------------------------

@ Restores the interrupts
        .type Cy_RestoreIRQ, %function
Cy_RestoreIRQ:
        MSR   PRIMASK, r0
        BX    LR
        .size Cy_RestoreIRQ, .-Cy_RestoreIRQ



@************************************************************************
@* Start-up Code
@************************************************************************
        .text

        .weak Reset_Handler

        .type Reset_Handler, %function
        .type _start, %function

Reset_Handler:
_start:

@ Disable global interrupts
        CPSID   I

@ Update Vector Table Offset Register with address of user ROM table
@ (will be updated later to user RAM table address in C startup code)
        LDR  r0, =__vector_table
        LDR  r1, =VTOR
        STR  r0, [r1]
        DSB

@ Initialize ECC of startup stack (needed for local variables in C startup code) by processing 8 bytes per loop iteration,
@ because the ECC initialization feature uses this generic granularity that will cover any memory (SRAM/TCM) in any TVII device
@ Prerequisite: Stack Pointer (SP) has not been modified (from the vector table init value) by above code (otherwise code must be adapted)
        MOVS r0, #0 @ clear value
        MOVS r1, #0 @ clear value
        LDR  r2, Cy_u32StartupStackStartAddress
startup_stack_ecc_init_loop:
        STRD r0, r1, [r2], #8
        CMP  sp, r2
        BNE  startup_stack_ecc_init_loop

@ Call C startup code (no ANSI C context established yet!)
        LDR  r7, =Startup_Init
        BLX  r7

@ Initialize .bss section (zero init) - could be commented if ECC initialization in Startup_Init is enabled
        MOVS r0, #0 @ clear value
        MOVS r1, #0 @ clear value
        LDR  r2, =__bss_start
        LDR  r3, =__bss_end
        CMP  r3, r2              @ Section size != 0?
        BEQ  bss_zero_init_end
bss_zero_init_loop:
        STRD r0, r1, [r2], #8
        CMP  r3, r2
        BNE  bss_zero_init_loop
bss_zero_init_end:

@ Initialize .data section (copy init)
        LDR  r2, =__data_start
        LDR  r3, =__data_end
        LDR  r4, =__data_lma
        CMP  r4, r2              @ LMA == VMA ? (true for _LINK_sram builds where copy can be skipped because .data would have been loaded by debugger)
        BEQ  data_copy_init_end 
        CMP  r3, r2              @ Section size != 0?
        BEQ  data_copy_init_end

data_copy_init_loop:
        LDRD r0, r1, [r4], #8
        STRD r0, r1, [r2], #8
        CMP  r3, r2
        BNE  data_copy_init_loop
data_copy_init_end:

@ Call main function
        LDR  r7, =main
        BLX  r7

@ Returned from main, stay in endless loop
Cy_Main_Exited:
        B    Cy_Main_Exited

        .size Reset_Handler, .-Reset_Handler
        .size _start, .-_start

@************************************************************************
@* Literal pool
@************************************************************************

        .balign 4

        .ltorg

        .balign 4

        .type Cy_u32StartupStackStartAddress, %object
Cy_u32StartupStackStartAddress:
        .dc.l (__stack_end - (STARTUP_STACK_SIZE_DOUBLE_WORDS * 8))
        .size Cy_u32StartupStackStartAddress, .-Cy_u32StartupStackStartAddress

        .type Cy_u32StartupStackEndAddress, %object
Cy_u32StartupStackEndAddress:
        .dc.l (__stack_end - 1)
        .size Cy_u32StartupStackEndAddress, .-Cy_u32StartupStackEndAddress



@************************************************************************
@* Default and weak implementation of interrupt handlers
@************************************************************************

        .weak NMI_Handler
        .text
NMI_Handler:
        B .

@-----------------------------------------

        .weak Cy_SysLib_FaultHandler
        .text
Cy_SysLib_FaultHandler:
        B .

@-----------------------------------------

        .weak HardFault_Handler
        .text
HardFault_Handler:
        MOVS r0, #4
        MOV  r1, lr
        TST  r0, r1
        BEQ  L_MSP
        MRS  r0, PSP
        B    L_API_call
L_MSP:
        MRS  r0, MSP
L_API_call:
        @ Storing LR content for Creator call stack trace
        PUSH {lr}
        LDR  r1, =Cy_SysLib_FaultHandler
        BLX  r1
        POP  {pc}

        .balign 4

@-----------------------------------------

        .weak MemManage_Handler
        .text
MemManage_Handler:
        B .

@-----------------------------------------

        .weak BusFault_Handler
        .text
BusFault_Handler:
        B .

@-----------------------------------------

        .weak UsageFault_Handler
        .text
UsageFault_Handler:
        B .

@-----------------------------------------

        .extern Cy_SysLib_SvcHandler
        .weak SVC_Handler
        .text
SVC_Handler:
        MOVS r0, #4
        MOV  r1, lr
        TST  r0, r1
        BEQ  L_SVC_MSP
        MRS  r0, PSP
L_SVC_MSP:
        MRS  r0, MSP
        @ Storing LR content for Creator call stack trace
        PUSH {lr}
        LDR  r1, =Cy_SysLib_SvcHandler
        BLX  r1
        POP  {pc}
        .balign 4

@-----------------------------------------

        .weak DebugMon_Handler
        .text
DebugMon_Handler:
        B .

@-----------------------------------------

        .weak PendSV_Handler
        .text
PendSV_Handler:
        B .

@-----------------------------------------

        .weak SysTick_Handler
        .text
SysTick_Handler:
        B .

@-----------------------------------------


        @ External interrupts
        @ Traveo II CPU User Interrupts 0-7 handlers are defined in the project interrupt mapping file
        @ Traveo II CPU User Interrupts 8-15 can only be used as SW interrupts and need to be defined by user (weak implementation provided below)


        .weak CpuUserInt8_Handler
        .text
CpuUserInt8_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt9_Handler
        .text
CpuUserInt9_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt10_Handler
        .text
CpuUserInt10_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt11_Handler
        .text
CpuUserInt11_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt12_Handler
        .text
CpuUserInt12_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt13_Handler
        .text
CpuUserInt13_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt14_Handler
        .text
CpuUserInt14_Handler:
        B .

@-----------------------------------------

        .weak CpuUserInt15_Handler
        .text
CpuUserInt15_Handler:
        B .

@-----------------------------------------


@************************************************************************
@* File end
@************************************************************************

        .end

