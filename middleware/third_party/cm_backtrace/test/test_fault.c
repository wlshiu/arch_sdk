/*
 * fault_test.c
 *
 *  Created on: 2016/12/25
 *      Author: Armink
 */


#include <stdio.h> 
#include "bsp.h"
#include "unity_config.h"
#include "unity.h"

#if 0
void fault_test_by_unalign(void)
{
    volatile int *SCB_CCR = (volatile int *) 0xE000ED14;  // SCB->CCR
    volatile int *p;
    volatile int value;

    *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

    p = (int *) 0x00;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x04;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

    p = (int *) 0x03;
    value = *p;
    printf("addr:0x%02X value:0x%08X\r\n", (int) p, value);
}
#endif

void __attribute__((used)) HardFault_Handler(void)
{
    __asm volatile
    (
        " MOV     r0, lr                \n"
        " MOV     r1, sp                \n"
        " BL      cm_backtrace_fault    \n"
    );

    __asm volatile("BKPT #01");;
}

TEST("test div0", "[fault test by div0] (BREAK)", TEST_PRIORITY_LOW)
{
    volatile int    *SCB_CCR = (volatile int *) 0xE000ED14;  // SCB->CCR
    int x, y, z;

    *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */

    x = 10;
    y = 0;
    z = x / y;
    printf("z:%d\n", z);
    return;
}
