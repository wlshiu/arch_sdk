/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_freertos.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/04
 * @license
 * @description
 */


#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "unity_config.h"
#include "unity.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define log_out(str, ...)   do{ snprintf(g_buf, sizeof(g_buf), str, ## __VA_ARGS__); \
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
    int     *pCnt = (int*)pArgv;
    int     cnt = 0;

    while(1)
    {
        log_out("hey FreeRTOS %d/%d\n ", cnt, *pCnt);
        *pCnt = *pCnt + 1;

        // delay 1000 ms
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

//    vTaskDelete(NULL);
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void vAssertCalled(const char * const pcFileName, unsigned long ulLine)
{
    log_out("assert: %s:%ld\n", pcFileName, ulLine);
    while(1) {}
    return;
}

void vApplicationIdleHook(void)
{
//    log_out("idle\n");
    return;
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
    log_out("stack overflow in task %08x %s", pxTask, (portCHAR *)pcTaskName);

    /* If the parameters have been corrupted then inspect pxCurrentTCB to
     * identify which task has overflowed its stack.
     */
    while(1) {}
}

TEST("test freertos",  "[freertos create task] (BLOCKING)", TEST_PRIORITY_LOW)
{
    static int      cnt = 10;

    {   // set std I/O
        bsp_stdio_t     bsp_io = {0};
        bsp_init(0);
        bsp_get_std_io(&bsp_io);
        g_stdout_out = bsp_io.pf_stdout_string;
    }

    //----------------
    // action start
    log_out("Start %s\n", __TIME__);

    xTaskCreate(_task_main, "main", configMINIMAL_STACK_SIZE, &cnt, configMAX_PRIORITIES, NULL);

    vTaskStartScheduler();

    while(1) {}
}
