/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_uart.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include <stdio.h>
#include <string.h>
#include "device_hal.h"
#include "bsp.h"

#include "hal_uart.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ACT_UART_PORT_ID             0
#define CONFIG_SYS_CORE_IRQ_ID              SysCore_IRQn
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

#define UART_RX_CACHE_SIZE      32
    uint8_t     uart_rx_cache[UART_RX_CACHE_SIZE];
} uart_io_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint32_t     g_baud_rate[] =
{
    115200, 230400, 460800, 57600,
    38400, 19200, 14400, 9600
};

static uart_io_t        g_uart_io = { .wr_idx = 0, };
static uart_handle_t    *g_pHUart = 0;

static cb_stdout_string_t   g_logout = 0;
static cb_stdin_string_t    g_stdin = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
__isr_func_l void _UART0_ISR(void)
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

static int
_uart_read(uint8_t *pBuf, int length)
{
    int         byte_cnt = 0;
    uint32_t    rd_pos = g_uart_io.rd_idx;
    uint32_t    wr_pos = g_uart_io.wr_idx;

    do {
        while( 1 )
        {
            if( rd_pos == wr_pos ) break;
            if( length == byte_cnt ) break;

            pBuf[byte_cnt++] = g_uart_io.uart_rx_cache[rd_pos];

            rd_pos = (rd_pos + 1) % UART_RX_CACHE_SIZE;
        }

        g_uart_io.rd_idx = rd_pos;
    } while(0);

    return byte_cnt;
}


static int
_getchar(void)
{
    char    value[4] = {0};
    int     byte_cnt = 0;

    byte_cnt = g_stdin((char*)value, 1);
    if( byte_cnt )  g_logout(&value[0], 1);

    return (byte_cnt) ? value[0] : -1;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
void _HardFault(void)
{
    __pause();
}

void user_pre_sys_setting(void)
{
    hal_irq_disable(CONFIG_SYS_CORE_IRQ_ID);
    hal_init();
    return;
}


int _test_pure_uart(void)
{
    hal_errno_t     rval = HAL_ERRNO_OK;

    do {
        uint32_t        rx_cnt = 0;
        uart_config_t   config = {0};
        char            *pMsg = 0;

        config.system_clock = hal_sys_get_core_clk();
        config.baud_rate    = g_baud_rate[0];
        config.parity       = UART_PARITY_NONE;
        config.stop_bits    = UART_STOP_BITS_1;
        config.word_length  = UART_WORD_LEN_8BITS;

        rval = hal_uart_init(CONFIG_ACT_UART_PORT_ID, &config, &g_pHUart);
        if( rval ) break;

        hal_irq_register_isr(g_pHUart->irq_id, _UART0_ISR);
        hal_irq_enable(g_pHUart->irq_id);

        hal_irq_enable(CONFIG_SYS_CORE_IRQ_ID);

        pMsg = "######### ";
        rval = hal_uart_send(g_pHUart, (uint8_t*)pMsg, strlen(pMsg));
        if( rval ) break;

        pMsg = __TIME__;
        rval = hal_uart_send(g_pHUart, (uint8_t*)pMsg, strlen(pMsg));
        if( rval ) break;

        pMsg = "\nI AM UART\n";
        rval = hal_uart_send(g_pHUart, (uint8_t*)pMsg, strlen(pMsg));
        if( rval ) break;

        while( 1) // rx_cnt < 20 )
        {
            uint8_t     rx_data[32] = {};
            int         rx_bytes = 0;

            rx_bytes = _uart_read(rx_data, sizeof(rx_data));
            if( rx_bytes )
            {
                hal_uart_send(g_pHUart, rx_data, rx_bytes);
                rx_cnt += rx_bytes;
            }
        }
    } while(0);

    hal_uart_deinit(&g_pHUart);

    return 0;
}

int _test_stdio(void)
{
    int     cnt = 0;
    char    *pMsg = "######### ";
    char    *pPrepix = "Hey man \n";

    {
        bsp_stdio_t     bsp_io ={ .pf_stdout_string = 0, };
        bsp_init(0);
        bsp_get_std_io(&bsp_io);

        g_logout = bsp_io.pf_stdout_string;
        g_stdin  = bsp_io.pf_stdin_str;
    }

    hal_irq_enable(CONFIG_SYS_CORE_IRQ_ID);

    g_logout(pMsg, strlen(pMsg));

    pMsg = __TIME__;
    g_logout(pMsg, strlen(pMsg));

    g_logout(pPrepix, strlen(pPrepix));


    g_logout(pPrepix, strlen(pPrepix));

    while(1)
    {
        static int  timeout = 0;

        char    *pMsg_0 = "+";
        char    *pMsg_1 = "-";
        int     value = _getchar();

        if( value > 0 )
        {
            g_logout("\n", 1);
        }

        if( !timeout-- )
        {
            timeout = 1000000;
            if( cnt++ & 0x1 )
                g_logout(pMsg_1, strlen(pMsg_1));
            else
                g_logout(pMsg_0, strlen(pMsg_0));

            g_logout(pPrepix, strlen(pPrepix));
        }

        __asm("nop");
    }
}

int main(void)
{
    // _test_pure_uart();
    _test_stdio();
    return 0;
}
