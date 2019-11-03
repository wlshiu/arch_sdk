/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_uart.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/03
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "bsp.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define _delay_sec(a)       do{for(int _j = 0; _j < a * 10000000; _j++) __asm volatile ("nop"); }while(0)
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
static cb_stdout_string_t       g_stdout_str = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
int main(void)
{
    int     cnt = 0;

    {
        bsp_stdio_t     bsp_io ={0};
        bsp_init(0);
        bsp_get_std_io(&bsp_io);
        g_stdout_str = bsp_io.pf_stdout_string;
    }
    
    printf("dddddddddddddd\n");

    while(1)
    {
        char    buf[128] = {0};
        snprintf(buf, sizeof(buf), "hello %03d\n", cnt++);
        g_stdout_str(buf, strlen(buf));
        _delay_sec(1);
    }    
    
    return 0;
}

