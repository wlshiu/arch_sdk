/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_timer.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/19
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_timer.h"

#include "unity_config.h"
#include "unity.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_TIMER_ID         0
#define CONFIG_TEST_TIMES       10

#ifndef LOG_YELLOW
#define LOG_YELLOW              "\033[33m"
#endif

#ifndef LOG_RESET
#define LOG_RESET               "\033[m"
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct timer_test_entry
{
    uint32_t        timer_cnt;
    uint32_t        system_time;
} timer_test_entry_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static tmr_handle_t             *g_pHTmr = 0;
static bool                     g_is_blocking = false;

static int                      g_test_cnt = 0;
static timer_test_entry_t       g_test_entry[CONFIG_TEST_TIMES] = { {.timer_cnt = 0,}, };
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_TMR0_ISR(void)
{
    hal_irq_clear(g_pHTmr->irq_id, (void*)g_pHTmr);

    if( g_test_cnt < CONFIG_TEST_TIMES )
    {
        g_test_entry[g_test_cnt].system_time = hal_get_tick();
        hal_tmr_get_count(g_pHTmr, &g_test_entry[g_test_cnt++].timer_cnt);

        if( g_test_cnt == CONFIG_TEST_TIMES )
            g_is_blocking = false;
    }
    else
        g_test_cnt++;

    return;
}

static void
_log_result(void)
{
    for(int i = 0; i < CONFIG_TEST_TIMES; i++)
    {
        memset(g_msg, 0, sizeof(g_msg));
        print_log("%u-th: count= %u, sys= %u ms, diff= %u\n",
                  i,
                  g_test_entry[i].timer_cnt,
                  g_test_entry[i].system_time,
                  (i) ? g_test_entry[i].system_time - g_test_entry[i - 1].system_time : 0);
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test Timer", "[Timer counter test]"  LOG_YELLOW " (BLOCKING)"LOG_RESET, TEST_PRIORITY_LOW)
{
    comm_init("\n\n################################ Test Timer\n");

    print_log("%s", "  Wait Timer alarm\n");

    do {
        tmr_config_t    config = { .clk_source = TMR_CLK_PCLK, };

        g_test_cnt = 0;

        hal_tmr_init(CONFIG_TIMER_ID, &config, &g_pHTmr);

        hal_irq_register_isr(g_pHTmr->irq_id, _TMR0_ISR);
        hal_irq_enable(g_pHTmr->irq_id);

        hal_tmr_enable(g_pHTmr, TMR_MODE_REPEAT, TMR_TIMEOUT_MSEC(1));

        g_is_blocking = true;

        while( g_is_blocking )
        {
            __asm("nop");
            __asm("nop");
            __asm("nop");
        }

        hal_tmr_disable(g_pHTmr);

        // log out result
        _log_result();
    } while(0);

    while(1)
        __asm("nop");

    return;
}
