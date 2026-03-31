/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2022 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File      : Cortex_M_Startup.s
Purpose   : Generic startup and exception handlers for Cortex-M devices.

Additional information:
  Preprocessor Definitions
    __NO_SYSTEM_INIT
      If defined, 
        SystemInit is not called.
      If not defined,
        SystemInit is called.
        SystemInit is usually supplied by the CMSIS files.
        This file declares a weak implementation as fallback.
        
    __SUPPORT_RESET_HALT_AFTER_BTL
      If != 0 (default)
        Support J-Link's reset strategy Reset and Halt After Bootloader.
        https://wiki.segger.com/Reset_and_Halt_After_Bootloader
      If == 0,
        Disable support for Reset and Halt After Bootloader.

    __SOFTFP__
      Defined by the build system.
      If not defined, the FPU is enabled for floating point operations.
*/

        .syntax unified  

        
#ifndef   __SUPPORT_RESET_HALT_AFTER_BTL
  #define __SUPPORT_RESET_HALT_AFTER_BTL  1
#endif
        
/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
//
// Just place a vector (word) in the table
//
.macro VECTOR Name=
        .section .vectors, "a"
        .word \Name
.endm
//
// Declare an interrupt handler
//
.macro ISR_HANDLER Name=
        //
        // Insert vector in vector table
        //
        .section .vectors, "a"
        .word \Name
        //
        // Insert dummy handler in init section
        //
        .section .init.\Name, "ax"
        .thumb_func
        .weak \Name
        .balign 2
\Name:
        1: b 1b   // Endless loop
        END_FUNC \Name
.endm

//
// Place a reserved vector in vector table
//
.macro ISR_RESERVED
        .section .vectors, "a"
        .word 0
.endm

//
// Mark the end of a function and calculate its size
//
.macro END_FUNC name
        .size \name,.-\name
.endm

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
/*********************************************************************
*
*  Setup of the vector table and weak definition of interrupt handlers
*
*/
        .section .vectors, "a"
        .code 16
        .balign 4
        .global _vectors
_vectors:
        VECTOR __stack_end__
        VECTOR Reset_Handler
        ISR_HANDLER NMI_Handler
        VECTOR HardFault_Handler
        ISR_HANDLER MemManage_Handler 
        ISR_HANDLER BusFault_Handler
        ISR_HANDLER UsageFault_Handler
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_RESERVED
        ISR_HANDLER SVC_Handler
        ISR_HANDLER DebugMon_Handler
        ISR_RESERVED
        ISR_HANDLER PendSV_Handler
        ISR_HANDLER SysTick_Handler
        //
        // Add external interrupt vectors here.
        // Example:
        //   ISR_HANDLER ExternalISR0
        //   ISR_HANDLER ExternalISR1
        //   ISR_HANDLER ExternalISR2
        //   ISR_HANDLER ExternalISR3
        //

        .section .vectors, "a"
        .size _vectors, .-_vectors
_vectors_end:

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       Reset_Handler
*
*  Function description
*    Exception handler for reset.
*    Generic bringup of a Cortex-M system.
*
*  Additional information
*    The stack pointer is expected to be initialized by hardware,
*    i.e. read from vectortable[0].
*    For manual initialization add
*      ldr R0, =__stack_end__
*      mov SP, R0
*/
        .global reset_handler
        .global Reset_Handler
        .equ reset_handler, Reset_Handler
        .section .init.Reset_Handler, "ax"
        .balign 2
        .thumb_func
Reset_Handler:
#if __SUPPORT_RESET_HALT_AFTER_BTL != 0
        //
        // Perform a dummy read access from address 0x00000008 followed by two nop's
        // This is needed to support J-Links reset strategy: Reset and Halt After Bootloader.
        // https://wiki.segger.com/Reset_and_Halt_After_Bootloader
        //
        movs    R0, #8
        ldr     R0, [R0]
        nop
        nop
#endif
#ifndef __NO_SYSTEM_INIT
        //
        // Call SystemInit
        //
        bl      SystemInit
#endif
#if !defined(__SOFTFP__)
        //
        // Enable CP11 and CP10 with CPACR |= (0xf<<20)
        //
        movw    R0, 0xED88
        movt    R0, 0xE000
        ldr     R1, [R0]
        orrs    R1, R1, #(0xf << 20)
        str     R1, [R0]
#endif
        //
        // Call runtime initialization, which calls main().
        //
        bl      _start
END_FUNC Reset_Handler

        //
        // Weak only declaration of SystemInit enables Linker to replace bl SystemInit with a NOP,
        // when there is no strong definition of SystemInit.
        //
        .weak SystemInit

/*********************************************************************
*
*       HardFault_Handler
*
*  Function description
*    Simple exception handler for HardFault.
*    In case of a HardFault caused by BKPT instruction without 
*    debugger attached, return execution, otherwise stay in loop.
*
*  Additional information
*    The stack pointer is expected to be initialized by hardware,
*    i.e. read from vectortable[0].
*    For manual initialization add
*      ldr R0, =__stack_end__
*      mov SP, R0
*/

#undef L
#define L(label) .LHardFault_Handler_##label

        .weak HardFault_Handler
        .section .init.HardFault_Handler, "ax"
        .balign 2
        .thumb_func
HardFault_Handler:
        //
        // Check if HardFault is caused by BKPT instruction
        //
        ldr     R1, =0xE000ED2C         // Load NVIC_HFSR
        ldr     R2, [R1]
        cmp     R2, #0                  // Check NVIC_HFSR[31]

L(hfLoop):
        bmi     L(hfLoop)               // Not set? Stay in HardFault Handler.
        //
        // Continue execution after BKPT instruction
        //
#if defined(__thumb__) && !defined(__thumb2__)
        movs    R0, #4
        mov     R1, LR
        tst     R0, R1                  // Check EXC_RETURN in Link register bit 2.
        bne     L(Uses_PSP)
        mrs     R0, MSP                 // Stacking was using MSP.
        b       L(Pass_StackPtr)
L(Uses_PSP):
        mrs     R0, PSP                 // Stacking was using PSP.
L(Pass_StackPtr):
#else
        tst     LR, #4                  // Check EXC_RETURN[2] in link register to get the return stack
        ite     eq
        mrseq   R0, MSP                 // Frame stored on MSP
        mrsne   R0, PSP                 // Frame stored on PSP
#endif
        //
        // Reset HardFault Status
        //
#if defined(__thumb__) && !defined(__thumb2__)
        movs    R3, #1
        lsls    R3, R3, #31
        orrs    R2, R3
        str     R2, [R1]
#else
        orr R2, R2, #0x80000000
        str R2, [R1]
#endif
        //
        // Adjust return address
        //
        ldr     R1, [R0, #24]           // Get stored PC from stack
        adds    R1, #2                  // Adjust PC by 2 to skip current BKPT
        str     R1, [R0, #24]           // Write back adjusted PC to stack
        //
        bx      LR                      // Return
END_FUNC HardFault_Handler

/*************************** End of file ****************************/
