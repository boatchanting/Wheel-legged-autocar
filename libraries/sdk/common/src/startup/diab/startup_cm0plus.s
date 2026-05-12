;/**************************************************************************//**
; * @file     startup_cm0plus.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM0plus Device Series
; ******************************************************************************/
;* \copyright
;* Copyright 2018-2021, Cypress Semiconductor Corporation.  All rights reserved.
;* You may use this file only in accordance with the license, terms, conditions,
;* disclaimers, and limitations in the end user license agreement accompanying
;* the software package with which this file was provided.
;*******************************************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        .file  "startup_cm0plus.s"

        
;************************************************************************
;* Local definitions
;************************************************************************

; Set a sufficient startup stack size for correct operation of C startup code (startup.c)
STARTUP_STACK_SIZE_DOUBLE_WORDS:   .EQU 32

VTOR:                              .EQU 0xe000ed08

CPUSS_RAM0_CTL0:                   .EQU 0x40201300
CPUSS_RAM1_CTL0:                   .EQU 0x40201380
CPUSS_RAM2_CTL0:                   .EQU 0x402013a0


;************************************************************************
;* Import symbols
;************************************************************************

        EXTERN  CpuUserInt0_Handler
        EXTERN  CpuUserInt1_Handler
        EXTERN  CpuUserInt2_Handler
        EXTERN  CpuUserInt3_Handler
        EXTERN  CpuUserInt4_Handler
        EXTERN  CpuUserInt5_Handler
        EXTERN  CpuUserInt6_Handler
        EXTERN  CpuUserInt7_Handler
        EXTERN  __init_main
        EXTERN  __SP_INIT
        EXTERN  __SP_END        
        EXTERN  Startup_Init
        

;************************************************************************
;* Export symbols
;************************************************************************

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

;************************************************************************
;* Vector Table and RAM Vector Table
;************************************************************************
        
        ; align to 256 byte, because CM0_VECTOR_TABLE_BASE register only supports address bits [31:8] (Note: for VTOR a 128-byte alignment would be ok)
        .section ".intvec",256,c

__vector_table:
        .word     __SP_INIT
        .word     Reset_Handler
        .word     NMI_Handler
        .word     HardFault_Handler
        .word     0
        .word     0
        .word     0
__vector_table_0x1c:
        .word     0
        .word     0
        .word     0
        .word     0
        .word     SVC_Handler
        .word     0
        .word     0
        .word     PendSV_Handler
        .word     SysTick_Handler

        ; External interrupts              PowerMode    Description

        .word     CpuUserInt0_Handler      ; DeepSleep    CPU User Interrupt #0 ; updated in RAM vector table with corresponding SROM vector table entry (after ROM-to-RAM copy)
        .word     CpuUserInt1_Handler      ; DeepSleep    CPU User Interrupt #1 ; updated in RAM vector table with corresponding SROM vector table entry (after ROM-to-RAM copy)
        .word     CpuUserInt2_Handler      ; DeepSleep    CPU User Interrupt #2
        .word     CpuUserInt3_Handler      ; DeepSleep    CPU User Interrupt #3
        .word     CpuUserInt4_Handler      ; DeepSleep    CPU User Interrupt #4
        .word     CpuUserInt5_Handler      ; DeepSleep    CPU User Interrupt #5
        .word     CpuUserInt6_Handler      ; DeepSleep    CPU User Interrupt #6
        .word     CpuUserInt7_Handler      ; DeepSleep    CPU User Interrupt #7

        ; These IRQs can only be triggered by SW via NVIC regs
        .word     CpuUserInt8_Handler      ; Active       CPU User Interrupt #8
        .word     CpuUserInt9_Handler      ; Active       CPU User Interrupt #9
        .word     CpuUserInt10_Handler     ; Active       CPU User Interrupt #10
        .word     CpuUserInt11_Handler     ; Active       CPU User Interrupt #11
        .word     CpuUserInt12_Handler     ; Active       CPU User Interrupt #12
        .word     CpuUserInt13_Handler     ; Active       CPU User Interrupt #13
        .word     CpuUserInt14_Handler     ; Active       CPU User Interrupt #14
        .word     CpuUserInt15_Handler     ; Active       CPU User Interrupt #15
__Vectors_End:

__Vectors:	  .EQU   __vector_table
__Vectors_Size:	  .EQU   __Vectors_End - __Vectors

        ; use same alignment like vector table in ROM above (even though VTOR minimum requirement would be 128 bytes if not used in combination with CM0_VECTOR_TABLE_BASE register)
        .section ".intvec_ram",256,b
__ramVectors:
        .space     __Vectors_Size


;************************************************************************
;* Helper functions
;************************************************************************

        THUMB


; Saves and disables the interrupts

        .global Cy_SaveIRQ
        .section ".text",2,c
Cy_SaveIRQ:
        MRS   r0, PRIMASK
        CPSID I
        BX    LR


;-----------------------------------------

; Restores the interrupts

        .global Cy_RestoreIRQ
        .section ".text",2,c
Cy_RestoreIRQ:
        MSR   PRIMASK, r0
        BX    LR

;-----------------------------------------

; Define strong version to return zero for
; __iar_program_start to skip data sections
; initialization.

        .global __low_level_init
        .section ".text",2,c
__low_level_init:	
        MOVS R0, #0
        BX LR

        
;************************************************************************
;* Start-up Code
;************************************************************************

        THUMB
        .weak Reset_Handler
	; NOTE:	 IAR REORDER directive is not translated here to Diab. ???
        .section ".text",2,c

Reset_Handler:
_start:

; Disable global interrupts
        CPSID   I
        
; Update Vector Table Offset Register with address of user ROM table
; (will be updated later to user RAM table address in C startup code)
        LDR  r0, =__vector_table
        LDR  r1, =VTOR
        STR  r0, [r1]
        DSB

; CM0+ bus width is 32-bit, but SRAM is built with 64-bit based ECC on Traveo II parts with CM7 core
; Set CPUSS->RAMx_CTL0.ECC_CHECK_DIS bits to avoid causing unintentional ECC faults during startup while SRAM ECC has not been initialized yet
; Generic code can be used, even if RAMx_CTL0 (x > 0) registers are not implemented in a device
; or if no ECC_CHECK_DIS bits are available in the registers in case of m4cpuss with 32-bit ECC SRAM
        MOVS r0, #1
        LSLS r0, r0, #19        
        LDR  r1, =CPUSS_RAM0_CTL0
        LDR  r2, [r1]
        ORRS r2, r0
        STR  r2, [r1]
        LDR  r1, =CPUSS_RAM1_CTL0
        LDR  r2, [r1]
        ORRS r2, r0
        STR  r2, [r1]
        LDR  r1, =CPUSS_RAM2_CTL0
        LDR  r2, [r1]
        ORRS r2, r0
        STR  r2, [r1]
        
; Initialize ECC of startup stack (needed for local variables in C startup code) by processing 8 bytes per loop iteration,
; because the ECC initialization feature uses this generic granularity that will cover any memory (SRAM/TCM) in any TVII device
; Prerequisite: Stack Pointer (SP) has not been modified (from the vector table init value) by above code (otherwise code must be adapted)
        MOVS r0, #0 ; clear value
        MOVS r1, #0 ; clear value
        LDR  r2, Cy_u32StartupStackStartAddress
startup_stack_ecc_init_loop:
        STM  r2!, {r0, r1}
        CMP  r2, sp
        BNE  startup_stack_ecc_init_loop
        
; Call C startup code (no ANSI C context established yet!)
        LDR  r7, =Startup_Init
        BLX  r7

	;; Finish initialization (copies .data
	;; ROM to RAM, clears .bss), then calls
	;; example main(), which calls exit(),
	;; which halts.

        LDR  r7, =__init_main 
        BLX  r7

; Note: Control flow does not necessarily return here.
; On some tool-chains (e.g. IAR) control flow will never return from
; the system library.
Cy_Main_Exited:
        B    Cy_Main_Exited

;************************************************************************
;* Literal pool
;************************************************************************

        .align 2
        
        .literals

        .align 2

STARTUP_STACK_SIZE_BYTES:	    .EQU (STARTUP_STACK_SIZE_DOUBLE_WORDS * 8)  ; Multiplication does not work in below data definition directive, so an additional define is created

Cy_u32StartupStackStartAddress:
        .word (__SP_INIT - STARTUP_STACK_SIZE_BYTES)

Cy_u32StartupStackEndAddress:
        .word (__SP_INIT - 1)

        

;************************************************************************
;* Default and weak implementation of interrupt handlers
;************************************************************************


        .weak NMI_Handler
	; NOTE:	 IAR REORDER directive is not translated here to Diab. ???	
        .section ".text",2,c 
NMI_Handler:
        B NMI_Handler

;-----------------------------------------

        .weak Cy_SysLib_FaultHandler
        .section ".text",2,c
Cy_SysLib_FaultHandler:
        B Cy_SysLib_FaultHandler

;-----------------------------------------

        .weak HardFault_Handler
        .section ".text",2,c
HardFault_Handler:
        IMPORT Cy_SysLib_FaultHandler
        MOVS r0, #4
        MOV  r1, lr
        TST  r0, r1
        BEQ  L_MSP
        MRS  r0, PSP
        B    L_API_call
L_MSP:	
        MRS  r0, MSP
L_API_call:	
        ; Storing LR content for Creator call stack trace
        PUSH {lr}
        LDR  r1, =Cy_SysLib_FaultHandler
        BLX  r1
        POP  {pc}

;-----------------------------------------

        IMPORT Cy_SysLib_SvcHandler
        .weak SVC_Handler
        .section ".text",2,c
SVC_Handler:
        MOVS r0, #4
        MOV  r1, lr
        TST  r0, r1
        BEQ  L_SVC_MSP
        MRS  r0, PSP
L_SVC_MSP:
        MRS  r0, MSP
        ; Storing LR content for Creator call stack trace
        PUSH {lr}
        LDR  r1, =Cy_SysLib_SvcHandler
        BLX  r1
        POP  {pc}

;-----------------------------------------

        .weak PendSV_Handler
        .section  ".text",2,c
PendSV_Handler:
        B PendSV_Handler

;-----------------------------------------

        .weak SysTick_Handler
        .section  ".text",2,c
SysTick_Handler:
        B SysTick_Handler


;-----------------------------------------


        ; External interrupts
        ; Traveo II CPU User Interrupts 0-7 handlers are defined in the project interrupt mapping file
        ; Traveo II CPU User Interrupts 8-15 can only be used as SW interrupts and need to be defined by user (weak implementation provided below)


        .weak CpuUserInt8_Handler
        .section  ".text",2,c
CpuUserInt8_Handler:
        B CpuUserInt8_Handler

;-----------------------------------------

        .weak CpuUserInt9_Handler
        .section  ".text",2,c
CpuUserInt9_Handler:
        B CpuUserInt9_Handler

;-----------------------------------------

        .weak CpuUserInt10_Handler
        .section  ".text",2,c
CpuUserInt10_Handler:
        B CpuUserInt10_Handler

;-----------------------------------------

        .weak CpuUserInt11_Handler
        .section  ".text",2,c
CpuUserInt11_Handler:
        B CpuUserInt11_Handler

;-----------------------------------------

        .weak CpuUserInt12_Handler
        .section  ".text",2,c
CpuUserInt12_Handler:
        B CpuUserInt12_Handler

;-----------------------------------------

        .weak CpuUserInt13_Handler
        .section  ".text",2,c
CpuUserInt13_Handler:
        B CpuUserInt13_Handler

;-----------------------------------------

        .weak CpuUserInt14_Handler
        .section  ".text",2,c
CpuUserInt14_Handler:
        B CpuUserInt14_Handler

;-----------------------------------------

        .weak CpuUserInt15_Handler
        .section  ".text",2,c
CpuUserInt15_Handler:
        B CpuUserInt15_Handler

;-----------------------------------------


;************************************************************************
;* File end
;************************************************************************

        END


