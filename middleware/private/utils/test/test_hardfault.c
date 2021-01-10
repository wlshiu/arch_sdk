/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_hardfault.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/22
 * @license
 * @description
 */


#include "_types.h"
#include "bsp.h"
#include "utils_printf.h"
#include "utils_log.h"
#include "unity_config.h"
#include "unity.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test div0", "[hard fault test by div0]" LOG_YELLOW " (BREAK)"LOG_RESET, TEST_PRIORITY_LOW)
{
    volatile int    *SCB_CCR = (volatile int *)0xE000ED14;  // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    utils_printf("z:%d\n", z);
    return;
}

TEST("test unalign", "[hard fault test unalign]" LOG_YELLOW " (BREAK)"LOG_RESET, TEST_PRIORITY_LOW)
{
    volatile int *SCB_CCR = (volatile int *)0xE000ED14;  // SCB->CCR
    volatile int *p;
    volatile int value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p = (int *)0x00;
    value = *p;
    utils_printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);

    p = (int *)0x03;
    value = *p;
    utils_printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);

    p = (int *)0xFFFFFFFF;
    value = *p;
    utils_printf("addr:0x%02X value:0x%08X\r\n", (int)p, value);
    return;
}
