/**
 * Copyright (c) 2019 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file bsp_evb.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2019/10/03
 * @license
 * @description
 */


#include "bsp.h"
#include "device_hal.h"
#include "system_ARMCM4.h"
#include "hal_uart.h"
#include "hal_scu.h"
#include "hal_gpio.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define UART_ACT_DBG_PORT_ID            CONFIG_DEBUG_PORT_UART_ID
#define UART_ACT_BAUD_RATE              115200


#ifndef __unused
#define __unused                    __attribute__ ((unused))
#endif
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uart_io
{
    volatile uint32_t    wr_idx;
    volatile uint32_t    rd_idx;

    volatile uint32_t    is_flushing;

#define UART_RX_CACHE_SIZE      32
    uint8_t     uart_rx_cache[UART_RX_CACHE_SIZE];
} uart_io_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uart_io_t        g_uart_io = { .wr_idx = 0, .rd_idx = 0, };
static uart_handle_t    *g_pHUart = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
__isr_func_h void _UART_DBG_PORT_ISR(void)
{
    hal_irq_clear(g_pHUart->irq_id, (void*)g_pHUart);

    // receive data
    do {
        uint32_t    wr_pos = g_uart_io.wr_idx;
        uint32_t    rd_pos = g_uart_io.rd_idx;
        uint32_t    pos = 0;
        uint8_t     value[4] = {0};
        uint32_t    rx_len = sizeof(value);

        hal_uart_recv(g_pHUart, value, &rx_len);
        if( rx_len )
        {
            pos = (wr_pos + 1) % UART_RX_CACHE_SIZE;
            if( pos == rd_pos ) break;

            g_uart_io.uart_rx_cache[wr_pos] = value[0];
            g_uart_io.wr_idx                = pos;
        }
    } while(0);

    return;
}

static hal_errno_t
evb_init(void)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    do {
        uart_config_t   config = { .irq_priority = 0, };

        config.system_clock = hal_sys_get_core_clk();
        config.baud_rate    = UART_ACT_BAUD_RATE;
        config.parity       = UART_PARITY_NONE;
        config.stop_bits    = UART_STOP_BITS_1;
        config.word_length  = UART_WORD_LEN_8BITS;

        rval = hal_uart_init(UART_ACT_DBG_PORT_ID, &config, &g_pHUart);
        if( rval )  break;

        hal_irq_register_isr(g_pHUart->irq_id, _UART_DBG_PORT_ISR);
        hal_irq_enable(g_pHUart->irq_id);

    } while(0);

    return rval;
}

static hal_errno_t
evb_deinit(void)
{
    return 0;
}

static hal_errno_t
evb_stdout_char(int ch)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    rval = hal_uart_send(g_pHUart, (uint8_t*)&ch, 1);
    return rval;
}


static int
evb_stdout_string(char *str, int length)
{
    hal_errno_t     rval = HAL_ERRNO_OK;
    rval = hal_uart_send(g_pHUart, (uint8_t*)str, length);
    return (rval == HAL_ERRNO_OK) ? length : 0;
}

static int
evb_stdin_str(char *pBuf, int length)
{
    int         byte_cnt = 0;
    uint32_t    rd_pos = g_uart_io.rd_idx;
    uint32_t    wr_pos = g_uart_io.wr_idx;

    g_uart_io.is_flushing = false;

    do {
        while( 1 )
        {
            if( g_uart_io.is_flushing )
            {
                wr_pos = rd_pos = byte_cnt = 0;
                break;
            }

            if( rd_pos == wr_pos ) break;
            if( length == byte_cnt ) break;

            pBuf[byte_cnt++] = g_uart_io.uart_rx_cache[rd_pos];

            rd_pos = (rd_pos + 1) % UART_RX_CACHE_SIZE;
        }

        g_uart_io.rd_idx = rd_pos;
    } while(0);
    return byte_cnt;
}

static hal_errno_t
evb_stdio_flush(void)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    hal_irq_disable(g_pHUart->irq_id);

    g_uart_io.wr_idx = 0;
    g_uart_io.rd_idx = 0;
    g_uart_io.is_flushing = true;

    hal_irq_enable(g_pHUart->irq_id);
    return rval;
}

static hal_errno_t
evb_set_gpio(bsp_gpio_info_t *pInfo)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    rval = (uint32_t)hal_gpio_set_pin(pInfo->pHGpio, pInfo->pin, pInfo->mode,
                                      pInfo->trig_mode, pInfo->data);
    return rval;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
bsp_desc_t      g_bsp_evb =
{
    .pf_init           = evb_init,
    .pf_deinit         = evb_deinit,
    .pf_stdout_char    = evb_stdout_char,
    .pf_stdout_string  = evb_stdout_string,
    .pf_stdin_str      = evb_stdin_str,
    .pf_set_gpio       = evb_set_gpio,
    .pf_stdio_flush    = evb_stdio_flush,
};
