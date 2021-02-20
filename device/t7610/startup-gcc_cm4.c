/* *INDENT-OFF* */
/******************************************************************************
 * @file     startup_ARMCM4.c
 * @brief    CMSIS Core Device Startup File for
 *           ARMCM4 Device
 * @version  V1.1.0
 * @date     23. January 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

#include "device_hal.h"
#include "system_ARMCM4.h"

/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __text_start__;
extern uint32_t __text_end__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __HeapBase;
extern uint32_t __StackTop;

extern uint32_t __reloc_lma_start__;
extern uint32_t __reloc_vma_start__;
extern uint32_t __reloc_vma_end__;

extern uint32_t __test_pool_start__;
extern uint32_t __test_pool_end__;

extern uint32_t __cli_cmd_pool_start__;
extern uint32_t __cli_cmd_pool_end__;

extern uint32_t __os_heap_start__;
extern uint32_t __os_heap_end__;

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void( *pFunc )( void );


/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern void _start     (void) __attribute__((noreturn)); /* PreeMain (C library entry point) */


/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Default_Handler(void); // __attribute__ ((noreturn));
void Reset_Handler  (void) __attribute__ ((noreturn));


/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/
#undef __STACK_SIZE
//<h> Stack Configuration
//  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#define  __STACK_SIZE  CONFIG_SYSTEM_STACK_SIZE
static uint8_t stack[__STACK_SIZE] __attribute__ ((aligned(8), used, section(".stack")));

//<h> Heap Configuration
//  <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
//</h>
#undef __HEAP_SIZE
#define  __HEAP_SIZE   CONFIG_SYSTEM_HEAP_SIZE
#if __HEAP_SIZE > 0
    static uint8_t heap[__HEAP_SIZE]   __attribute__ ((aligned(8), used, section(".heap")));
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
/* void HardFault_Handler      (void) __attribute__ ((weak, alias("Default_Handler"))); */
void HardFault_Handler      (void) __attribute__ ((weak));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
/* void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler"))); */
void SVC_Handler            (void) __attribute__ ((weak));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
/* void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler"))); */
void PendSV_Handler         (void) __attribute__ ((weak));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

void SYS_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void CoreRxEv_IRQHandler    (void) __attribute__ ((weak, alias("Default_Handler")));
void ZCD_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void WAKEUP_IRQHandler      (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void CRC_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SEC_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void WDT_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void GMAC_IRQHandler        (void) __attribute__ ((weak));
// void IPC_RxCh0_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void IPC_RxCh0_IRQHandler   (void) __attribute__ ((weak));
void SSP1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void DMA_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SSP2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SSP0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
// void IPC_RxCh1_IRQHandler   (void) __attribute__ ((weak, alias("Default_Handler")));
void IPC_RxCh1_IRQHandler   (void) __attribute__ ((weak));
void TIM0_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM1_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM2_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void TIM3_IRQHandler        (void) __attribute__ ((weak, alias("Default_Handler")));
void I2C_IRQHandler         (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART5_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3_IRQHandler       (void) __attribute__ ((weak, alias("Default_Handler")));

void user_pre_sys_setting(void) __attribute__ ((weak));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const pFunc __Vectors[64];
const pFunc __Vectors[64] __attribute__((used, section(".vectors"))) =
{
    (pFunc)(&__StackTop),                     /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    MemManage_Handler,                        /* -12 MPU Fault Handler */
    BusFault_Handler,                         /* -11 Bus Fault Handler */
    UsageFault_Handler,                       /* -10 Usage Fault Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    SVC_Handler,                              /*  -5 SVCall Handler */
    DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
    0,                                        /*     Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* Interrupts */
    SYS_IRQHandler,                           /*   0 Interrupt 0 */
    CoreRxEv_IRQHandler,                      /*   1 Interrupt 1 */
    ZCD_IRQHandler,                           /*   2 Interrupt 2 */
    WAKEUP_IRQHandler,                        /*   3 Interrupt 3 */
    GPIO_IRQHandler,                          /*   4 Interrupt 4 */
    0,                                        /*   5 Interrupt 5 */
    0,                                        /*   6 Interrupt 6 */
    CRC_IRQHandler,                           /*   7 Interrupt 7 */
    SEC_IRQHandler,                           /*   8 Interrupt 8 */
    0,                                        /*   9 Interrupt 9 */
    WDT_IRQHandler,                           /*   10 Interrupt 10 */
    GMAC_IRQHandler,                          /*   11 Interrupt 11 */
    IPC_RxCh0_IRQHandler,                     /*   12 Interrupt 12 */
    SSP1_IRQHandler,                          /*   13 Interrupt 13 */
    DMA_IRQHandler,                           /*   14 Interrupt 14 */
    SSP2_IRQHandler,                          /*   15 Interrupt 15 */
    SSP0_IRQHandler,                          /*   16 Interrupt 16 */
    IPC_RxCh1_IRQHandler,                     /*   17 Interrupt 17 */
    0,                                        /*   18 Interrupt 18 */
    0,                                        /*   19 Interrupt 19 */
    TIM0_IRQHandler,                          /*   20 Interrupt 20 */
    TIM1_IRQHandler,                          /*   21 Interrupt 21 */
    TIM2_IRQHandler,                          /*   22 Interrupt 22 */
    TIM3_IRQHandler,                          /*   23 Interrupt 23 */
    I2C_IRQHandler,                           /*   24 Interrupt 24 */
    0,                                        /*   25 Interrupt 25 */
    UART4_IRQHandler,                         /*   26 Interrupt 26 */
    UART5_IRQHandler,                         /*   27 Interrupt 27 */
    UART0_IRQHandler,                         /*   28 Interrupt 28 */
    UART1_IRQHandler,                         /*   29 Interrupt 29 */
    UART2_IRQHandler,                         /*   30 Interrupt 30 */
    UART3_IRQHandler,                         /*   31 Interrupt 31 */
};


/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;

    /**
     *  relocate section
     */
    pSrc  = &__reloc_lma_start__;
    pDest = &__reloc_vma_start__;
    for ( ; pDest < &__reloc_vma_end__ ; )
    {
        *pDest++ = *pSrc++;
    }

    /* Single section scheme.
     *
     * The ranges of copy from/to are specified by following symbols
     *   __text_end__: LMA of start of the section to copy from. Usually end of text
     *   __data_start__: VMA of start of the section to copy to
     *   __data_end__: VMA of end of the section to copy to
     *
     * All addresses must be aligned to 4 bytes boundary.
     */
    pSrc  = &__text_end__;
    pDest = &__data_start__;

    for ( ; pDest < &__data_end__ ; )
    {
        *pDest++ = *pSrc++;
    }

    /* Single BSS section scheme.
     *
     * The BSS section is specified by following symbols
     *   __bss_start__: start of the BSS section.
     *   __bss_end__: end of the BSS section.
     *
     * Both addresses must be aligned to 4 bytes boundary.
     */
    pDest = &__bss_start__;

    for ( ; pDest < &__bss_end__ ; )
    {
        *pDest++ = 0UL;
    }

    pDest = &__os_heap_start__;
    for( ; pDest < &__os_heap_end__; )
    {
        *pDest++ = 0UL;
    }

    /* CMSIS System Initialization */
    SystemInit();

    if( user_pre_sys_setting )
        user_pre_sys_setting();

    _start(); /* Enter PreeMain (C library entry point) */
}


/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while(1);
}

void empty_setting(void) { }

/* *INDENT-ON* */
