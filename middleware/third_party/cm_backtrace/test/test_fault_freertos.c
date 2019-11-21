/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_fault_freertos.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/21
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "unity_config.h"
#include "unity.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define log_out(str, ...)   do{ snprintf(g_buf, sizeof(g_buf), str, ##__VA_ARGS__);  \
                                g_stdout_out(g_buf, strlen(g_buf) + 1);              \
                            }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static cb_stdout_string_t       g_stdout_out = 0;
static char                     g_buf[128] = {0};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_task_main(void *pArgv)
{
    int     cnt = 0;

    log_out("main task\n ");

    while(1)
    {
        log_out("hey FreeRTOS %d\n ", cnt++);

        if( cnt == 3 )
        {
            volatile int    *SCB_CCR = (volatile int *) 0xE000ED14;  // SCB->CCR
            int x, y, z;

            *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

            x = 10;
            y = 0;
            z = x / y;
            log_out("z:%d\n", z);
        }

        // delay 1000 ms
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

//    vTaskDelete(NULL);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void __attribute__((used)) HardFault_Handler(void)
{
    __asm volatile
    (
        " MOV     r0, lr                \n"
        " MOV     r1, sp                \n"
        " BL      cm_backtrace_fault    \n"
    );

    __asm volatile("BKPT #01");
}

TEST("test div0", "[fault test by div0 (freertos)] (BREAK)", TEST_PRIORITY_LOW)
{
    {   // set std I/O
        bsp_stdio_t     bsp_io = {0};
        bsp_init(0);
        bsp_get_std_io(&bsp_io);
        g_stdout_out = bsp_io.pf_stdout_string;
    }

    xTaskCreate(_task_main, "main", configMINIMAL_STACK_SIZE, 0, configMAX_PRIORITIES, NULL);

    vTaskStartScheduler();

    while(1) {}
}
