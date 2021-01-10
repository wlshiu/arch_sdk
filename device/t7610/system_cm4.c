/* *INDENT-OFF* */
/******************************************************************************
 * @file     system_ARMCM4.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM4 Device
 * @version  V5.3.1
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
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

#include "device_hal.h"
#include "hal_scu.h"
#include "system_ARMCM4.h"

#if defined(CONFIG_T7610_FPGA)
    #define  XTAL               (100000000UL)     /* Oscillator frequency */
    #define  SYSTEM_CLOCK       (XTAL / 1U)
    #define  SYSTEM_CLOCK_H     (XTAL / 1U)
    // #pragma message ("\033[31m @@@ FPGA mode \033[m")
#else
    #define  XTAL_N             (275000000UL)     /* Oscillator frequency */
    #define  XTAL_H             (300000000UL)     /* Oscillator frequency */
    #define  SYSTEM_CLOCK       (XTAL_N / 1U)
    #define  SYSTEM_CLOCK_H     (XTAL_H / 1U)
    // #pragma message ("\033[1;31m @@@ ASIC mode \033[m")
#endif

extern uint32_t             __Vectors;
static uint32_t             SystemCoreClock = SYSTEM_CLOCK;  /* System Core Clock Frequency */
static scuex_handle_t       *g_pHScu = 0;
uint32_t                    __Vectorss[64] __attribute__((used, section("vtable"))) = {0};

void SystemCoreClockUpdate(void)
{
    volatile uint32_t   *pStrap = (uint32_t*)(g_pHScu->instance + 0x0854);
    volatile uint32_t   *pStrap_u = (uint32_t*)(g_pHScu->instance + 0x0850);
    volatile uint32_t   *pSw_rst_status = (uint32_t*)(g_pHScu->instance + 0x0884);

    if( IS_BIT_SET(*pSw_rst_status, 1) )
    {
        /**
         *  warm boot
         *  the user MUST manually set pScu->EXREG_0850[bit:15] when reboot to PROM
         */
        SystemCoreClock = (IS_BIT_SET(*pStrap_u, 13))
                        ? SYSTEM_CLOCK : SYSTEM_CLOCK_H;
    }
    else
    {
        // cold boot
        SystemCoreClock = (IS_BIT_SET(*pStrap, 13))
                        ? SYSTEM_CLOCK : SYSTEM_CLOCK_H;
    }
    return;
}

void SystemInit(void)
{
    hal_scu_init(&g_pHScu);

    SystemCoreClockUpdate();

    for(int i = 0; i < 64; i++)
        __Vectorss[i] = ((uint32_t*)&__Vectors)[i];

    SCB->VTOR = (uint32_t)&__Vectorss;

    for(uint32_t irq_id = (uint32_t)SYS_IRQn; irq_id < IRQ_Total; irq_id++)
        NVIC_SetPriority((IRQn_Type)irq_id, 3);

    #if defined(CONFIG_ENABLE_DETECT_DIV0)
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
    #endif

    #if defined(CONFIG_ENABLE_DETECT_UNALIGNED)
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
    #endif

    hal_init();

    return;
}

uint32_t SystemGetCoreClock(void)
{
    return SystemCoreClock;
}

/* *INDENT-ON* */
