/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp_qemu.c.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/11/02
 * @license
 * @description
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "stm32f4xx.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_UART_BAUD_RATE       115200

#ifndef __unused
    #define __unused                    __attribute__ ((unused))
#endif

#define QEMU_STDIN       (0)
#define QEMU_STDOUT      (1)

/* Semihosting starts here */
enum SEMIHOST_SYSCALL
{
    HOSTCALL_WRITE       = 0x05,
    HOSTCALL_READ        = 0x06,
};
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
static int semihost_call(int service, void *opaque)
{
    register int    r0 asm("r0") = service;
    register void   *r1 asm("r1") = opaque;
    register int    result asm("r0");
    __asm volatile("bkpt 0xab"
                    : "=r" (result) : "r" (r0), "r" (r1));
    return result;
}

static int host_read(int fd, void *buf, int count)
{
    uint32_t    semi_param[] = { fd, (uint32_t)buf, count };
    return semihost_call(HOSTCALL_READ, semi_param);
}

static int host_write(int fd, const void *buf, int count)
{
    uint32_t    semi_param[] = { fd, (uint32_t)buf, count };
    return semihost_call(HOSTCALL_WRITE, semi_param);
}


static int
qemu_stm32f4x9_init(void)
{
    return 0;
}

static int
qemu_stm32f4x9_deinit(void)
{
    return 0;
}

static int
qemu_stm32f4x9_stdout_char(int ch)
{
    char        buf[4] = {0};
    host_write(QEMU_STDOUT, buf, 1);
    return 0;
}


static int
qemu_stm32f4x9_stdout_string(char *str, int length)
{
    int         len = 0;
    host_write(QEMU_STDOUT, str, length);

    len = length;
    return len;
}

static int
qemu_stm32f4x9_stdin_str(char *pBuf, int length)
{
    int         byte_cnt = 0;
    host_read(QEMU_STDIN, pBuf, length);
    return byte_cnt;
}

static int
qemu_stm32f4x9_set_gpio(void *pExtra)
{
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
bsp_desc_t      g_bsp_qemu_stm32f4x9 =
{
    .pf_init           = qemu_stm32f4x9_init,
    .pf_deinit         = qemu_stm32f4x9_deinit,
    .pf_stdout_char    = qemu_stm32f4x9_stdout_char,
    .pf_stdout_string  = qemu_stm32f4x9_stdout_string,
    .pf_stdin_str      = qemu_stm32f4x9_stdin_str,
    .pf_set_gpio       = qemu_stm32f4x9_set_gpio,
};
