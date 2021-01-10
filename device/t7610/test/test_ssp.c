/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_ssp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/26
 * @license
 * @description
 */

#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_ssp.h"
#include "hal_scu.h"

#include "unity_config.h"
#include "unity.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_SSP_MASTER_PORT      0
#define CONFIG_SSP_SLAVE_PORT       2
#define CONFIG_SSP_CLK_RATE         (512 * 1000)

#define CONFIG_BUFER_SIZE           (0x1ul << 10)

typedef enum spi_role
{
    SPI_ROLE_MASTER     = 0,
    SPI_ROLE_SLAVE,
    SPI_ROLE_TOTAL
} spi_role_t;

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

//=============================================================================
//                  Global Data Definition
//=============================================================================
static ssp_handle_t     *g_pHSPI_M = 0;
static ssp_handle_t     *g_pHSPI_S = 0;

static uint8_t          g_buf_tx[SPI_ROLE_TOTAL][CONFIG_BUFER_SIZE] __attribute__ ((aligned(4)));
static uint8_t          g_buf_rx[SPI_ROLE_TOTAL][CONFIG_BUFER_SIZE] __attribute__ ((aligned(4)));

static const scu_call_t     g_init_script[] =
{
    [0] = { .reg = 0x54000824ul, .value = 0x00000000ul, .mask = 0x00FFC000ul, },
};

static uint32_t         g_is_enqueue = false;
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
_ssp_filter_data_t(
    ssp_datasize_t  data_size,
    uint32_t        value,
    void            *pTunnel_info)
{
    uint32_t    valid_bytes = 0;
    do {
        if( data_size != SSP_DATASIZE_8B )
            break;

        if( (value & 0xFF) == 0xCC )
            g_is_enqueue = true;

        if( g_is_enqueue == true )
        {
            valid_bytes = 1;
        }

    } while(0);
    return valid_bytes;
}

static void
_spi_master_isr(void)
{
    hal_ssp_irq_handle(g_pHSPI_M, _ssp_filter_data_t, 0);
    return;
}

static void
_spi_slave_isr(void)
{
    hal_ssp_irq_handle(g_pHSPI_S, _ssp_filter_data_t, (void*)0x1234);
    return;
}

static ssp_err_t
_fake_usr_cs(uint32_t is_cs_active, void *pUsr_info)
{
    print_log("usr info: %x\n", pUsr_info);
    return SSP_ERR_OK;
}

static ssp_err_t __attribute__ ((unused))
_spi_loopback(
    ssp_config_t    *pConfig)
{
    ssp_err_t           rval = SSP_ERR_OK;
    uint32_t            data_len = 256;
    ssp_data_set_t      data_set = { .pData = 0, };

    do {
        /**
         *  when loop-back case (SSP0 link with SSP1),
         *  slave MUST be configured first
         */
        // slave initialize
        pConfig->role = SSP_ROLE_SLAVE;
        rval = hal_ssp_init(CONFIG_SSP_SLAVE_PORT, pConfig, &g_pHSPI_S);
        if( rval ) break;

        hal_irq_register_isr(g_pHSPI_S->irq_id, _spi_slave_isr);
        hal_irq_enable(g_pHSPI_S->irq_id);


        // master initialize
        pConfig->role = SSP_ROLE_MASTER;
        rval = hal_ssp_init(CONFIG_SSP_MASTER_PORT, pConfig, &g_pHSPI_M);
        if( rval ) break;

        hal_irq_register_isr(g_pHSPI_M->irq_id, _spi_master_isr);
        hal_irq_enable(g_pHSPI_M->irq_id);

        //-------------------
        memset(g_buf_tx, 0x0, sizeof(g_buf_tx));
        memset(g_buf_rx, 0x0, sizeof(g_buf_rx));

        data_set.pData     = (uint32_t*)g_buf_rx[SPI_ROLE_SLAVE];
        data_set.length    = data_len;
        data_set.pUsr_info = (void*)0x123456;
        data_set.cb_usr_cs = 0;
        hal_ssp_recv(g_pHSPI_S, &data_set);

        // setup transmission data
        for(int i = 0; i < data_len; i++)
            g_buf_tx[SPI_ROLE_MASTER][i] = i & 0xFF;

        data_set.pData     = (uint32_t*)g_buf_tx[SPI_ROLE_MASTER];
        data_set.length    = data_len;
        data_set.pUsr_info = (void*)0x123456;
        data_set.cb_usr_cs = _fake_usr_cs;
        hal_ssp_send(g_pHSPI_M, &data_set);

        // wait ready
        while( hal_ssp_get_state(g_pHSPI_M) == SSP_ERR_BUSY )
            __asm("nop");

        while( hal_ssp_get_state(g_pHSPI_S) == SSP_ERR_BUSY )
            __asm("nop");

        // compare data
        for(int i = 0; i < data_len; i++)
        {
            if( g_buf_tx[SPI_ROLE_MASTER][i] != g_buf_rx[SPI_ROLE_SLAVE][i] )
            {
                print_log("master send %x, slave recv %x\n",
                          g_buf_tx[SPI_ROLE_MASTER][i], g_buf_rx[SPI_ROLE_SLAVE][i]);
                print_log("Master TX: %x %x %x %x\n",
                          g_buf_tx[SPI_ROLE_MASTER][i + 0], g_buf_tx[SPI_ROLE_MASTER][i + 1],
                          g_buf_tx[SPI_ROLE_MASTER][i + 2], g_buf_tx[SPI_ROLE_MASTER][i + 3]);
                print_log("Slave RX : %x %x %x %x\n",
                          g_buf_rx[SPI_ROLE_SLAVE][i + 0], g_buf_rx[SPI_ROLE_SLAVE][i + 1],
                          g_buf_rx[SPI_ROLE_SLAVE][i + 2], g_buf_rx[SPI_ROLE_SLAVE][i + 3]);
                __pause();
            }
        }
    } while(0);

    return rval;
}

static ssp_err_t  __attribute__ ((unused))
_spi_slave_mode(
    ssp_config_t    *pConfig)
{
    ssp_err_t           rval = SSP_ERR_OK;
    uint32_t            data_len = 8;
    ssp_data_set_t      data_set = { .pData = 0, };
    do {
        pConfig->role = SSP_ROLE_SLAVE;
        rval = hal_ssp_init(CONFIG_SSP_SLAVE_PORT, pConfig, &g_pHSPI_S);
        if( rval ) break;

        hal_irq_register_isr(g_pHSPI_S->irq_id, _spi_slave_isr);
        hal_irq_enable(g_pHSPI_S->irq_id);

        memset(g_buf_tx, 0x0, sizeof(g_buf_tx));
        memset(g_buf_rx, 0x0, sizeof(g_buf_rx));

        g_is_enqueue = false;

        data_set.pData     = (uint32_t*)g_buf_rx[SPI_ROLE_SLAVE];
        data_set.length    = data_len;
        hal_ssp_recv(g_pHSPI_S, &data_set);

        while( hal_ssp_get_state(g_pHSPI_S) == SSP_ERR_BUSY )
            __asm("nop");

        print_log("%s", "  Slave RX:\n");
        for(int i = 0; i < data_len; i += 4)
        {
            print_log("  %x %x %x %x\n",
                      g_buf_rx[SPI_ROLE_SLAVE][i + 0], g_buf_rx[SPI_ROLE_SLAVE][i + 1],
                      g_buf_rx[SPI_ROLE_SLAVE][i + 2], g_buf_rx[SPI_ROLE_SLAVE][i + 3]);
        }

        //----------------------------------------------
        // send back to Master
        {
            int         cnt = 0;

            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xCC;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xCC;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xCC;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xCC;

            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xB2;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0x0;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0x2;
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0x0;

            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'V';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'A';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'N';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'G';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'p';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'h';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'n';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 'x';
            g_buf_tx[SPI_ROLE_SLAVE][cnt++] = 0xEA;
            data_len = cnt;
        }

        data_set.pData     = (uint32_t*)g_buf_tx[SPI_ROLE_SLAVE];
        data_set.length    = (data_len + 0x3) & ~0x3;
        hal_ssp_send(g_pHSPI_S, &data_set);

        while( hal_ssp_get_state(g_pHSPI_S) == SSP_ERR_BUSY )
            __asm("nop");

    } while(0);

    return rval;
}

static ssp_err_t  __attribute__ ((unused))
_spi_master_mode(
    ssp_config_t    *pConfig)
{
    ssp_err_t           rval = SSP_ERR_OK;
    uint32_t            data_len = 8;
    ssp_data_set_t      data_set = { .pData = 0, };
    do {
        pConfig->role = SSP_ROLE_MASTER;
        rval = hal_ssp_init(CONFIG_SSP_MASTER_PORT, pConfig, &g_pHSPI_M);
        if( rval ) break;

        hal_irq_register_isr(g_pHSPI_M->irq_id, _spi_master_isr);
        hal_irq_enable(g_pHSPI_M->irq_id);

        memset(g_buf_tx, 0x0, sizeof(g_buf_tx));
        memset(g_buf_rx, 0x0, sizeof(g_buf_rx));

        //----------------------------------------------
        // Master send
        {
            int         cnt = 0;

            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xCC;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xCC;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xCC;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xCC;

            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xB2;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0x0;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0x2;
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0x0;

            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'V';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'A';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'N';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'G';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'p';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'h';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'n';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 'x';
            g_buf_tx[SSP_ROLE_MASTER][cnt++] = 0xEA;
            data_len = cnt;
        }

        data_set.pData     = (uint32_t*)g_buf_tx[SSP_ROLE_MASTER];
        data_set.length    = (data_len + 0x3) & ~0x3;
        hal_ssp_send(g_pHSPI_M, &data_set);

        while( hal_ssp_get_state(g_pHSPI_M) == SSP_ERR_BUSY )
            __asm("nop");

        //----------------------------------------------
        // master receive
        data_len = 16;
        data_set.pData     = (uint32_t*)g_buf_rx[SSP_ROLE_MASTER];
        data_set.length    = data_len;
        hal_ssp_recv(g_pHSPI_M, &data_set);

        while( hal_ssp_get_state(g_pHSPI_M) == SSP_ERR_BUSY )
            __asm("nop");

        print_log("%s", "Master RX:\n");
        for(int i = 0; i < data_len; i += 4)
        {
            print_log("%x %x %x %x\n",
                      g_buf_rx[SSP_ROLE_MASTER][i + 0], g_buf_rx[SSP_ROLE_MASTER][i + 1],
                      g_buf_rx[SSP_ROLE_MASTER][i + 2], g_buf_rx[SSP_ROLE_MASTER][i + 3]);
        }

    } while(0);

    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
TEST("test SSP", "[SSP2 slave mode]"  LOG_YELLOW " (BREAK)"LOG_RESET, TEST_PRIORITY_LOW)
{
    comm_init("\n\n################################ Test SSP\n");
    print_log("  wait Master signal...\n");

    hal_scu_exec_script((scu_call_t*)g_init_script, sizeof(g_init_script)/sizeof(g_init_script[0]));

    do {
        ssp_config_t    config = { .clock_rate = 0, };

        config.pclk       = hal_sys_get_pclk();
        config.clock_rate = CONFIG_SSP_CLK_RATE;
        config.role       = SSP_ROLE_SLAVE;
        config.cpol       = SSP_CPOL_HIGH;
        config.cpha       = SSP_CPHA_2_EDGE;
        config.first_bit  = SSP_FB_MSB;
        config.data_size  = SSP_DATASIZE_8B;

        //------------------------
        // test
        _spi_slave_mode(&config);
//        _spi_master_mode(&config);

//        _spi_loopback(&config);

    } while(0);

    while(1)
        __asm("nop");
    return;
}
