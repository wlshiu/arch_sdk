/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_gpio.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/21
 * @license
 * @description
 */


#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_gpio.h"

#include "unity_config.h"
#include "unity.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ACT_GPIO_PORT            0
#define CONFIG_ACT_PIN                  7
#define CONFIG_TRIGGER_MODE             GPIO_TRIG_BOTH //GPIO_TRIG_HIGH_LEVEL

#define CONFIG_TEST_TIMES               10
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static gpio_handle_t            *g_pHGpio = 0;
static int                      g_test_cnt = 0;

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_GPIO_ISR(void)
{
    uint32_t    irq_state = hal_gpio_get_irq(g_pHGpio);

    hal_irq_clear(g_pHGpio->irq_id, (void*)g_pHGpio);

    memset(g_msg, 0x0, sizeof(g_msg));

    if( irq_state & (0x1ul << CONFIG_ACT_PIN) )
    {
        uint32_t    in_value = 0;

        hal_gpio_disable_pin_irq(g_pHGpio, CONFIG_ACT_PIN);

        in_value = hal_gpio_get_input(g_pHGpio, (0x1ul << CONFIG_ACT_PIN));

        print_log("%u-th: Get gpio input 0x%x\n", g_test_cnt, in_value);

        if( g_test_cnt++ < CONFIG_TEST_TIMES )
        {
            // re-start IRQ
            hal_gpio_set_pin(g_pHGpio, CONFIG_ACT_PIN, GPIO_MODE_IGNORE,
                             CONFIG_TRIGGER_MODE, GPIO_DATA_IGNORE);
        }
    }
    else
    {
        print_log("--  Get wrong IRQ 0x%x\n", irq_state);

        while(1)
            __asm("nop");
    }

    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test GPIO", "[GPIO trigger test]", TEST_PRIORITY_LOW)
{
    gpio_err_t      rval = 0;

    comm_init("\n\n################################ Test GPIO\n");

    print_log("  Wait GPIO pin %u\n", CONFIG_ACT_PIN);

    g_test_cnt = 0;
    do {
        rval = hal_gpio_init(CONFIG_ACT_GPIO_PORT, 0, 0, &g_pHGpio);
        if( rval ) break;

        hal_irq_register_isr(g_pHGpio->irq_id, _GPIO_ISR);
        hal_irq_enable(g_pHGpio->irq_id);

        #if 1
        hal_gpio_set_pin(g_pHGpio, CONFIG_ACT_PIN, GPIO_MODE_INPUT,
                         CONFIG_TRIGGER_MODE, GPIO_DATA_LOW);

        while( g_test_cnt < CONFIG_TEST_TIMES )
            __asm("nop");

        hal_irq_disable(g_pHGpio->irq_id);

        #else
        while(1)
        {
            static int  cnt = 0;
            cnt ^= 1;
            hal_gpio_set_pin(g_pHGpio, CONFIG_ACT_PIN, GPIO_MODE_OUTPUT,
                             GPIO_TRIG_IGNORE, (cnt) ? GPIO_DATA_LOW : GPIO_DATA_HIGH);

            for(int j = 0; j < 1000000; j++) ;
        }
        #endif

    } while(0);

    return;
}
