/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_wdt.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/19
 * @license
 * @description
 */


#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_wdt.h"

#include "unity_config.h"
#include "unity.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
// #define CONFIG_WDT_ENABLE_IRQ_MODE

//=============================================================================
//                  Macro Definition
//=============================================================================
#define WDT_TIMEOUT(second)         (hal_sys_get_pclk() * (second))
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_WDT_ISR(void)
{
    hal_irq_clear(WDT_IRQn, (void*)0);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test WDT", "[WDT reset mode]"  LOG_YELLOW " (BREAK)"LOG_RESET, TEST_PRIORITY_LOW)
{
    comm_init("\n\n################################ Test WDT\n");

    print_log("%s", "  Wait WDT reset\n");

    hal_irq_register_isr(WDT_IRQn, _WDT_ISR);

#if defined(CONFIG_WDT_ENABLE_IRQ_MODE)
    hal_wdt_init(WDT_MODE_IRQ);
#else
    hal_wdt_init(WDT_MODE_RESET);
#endif

    hal_wdt_set_timeout(WDT_TIMEOUT(2));
    hal_wdt_enable();

    while(1)
        __asm("nop");

    return;
}