/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_wdt.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include "hal_wdt.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of WDT
 */
#define WDT_CSR_BASE        0x54020000ul

#define WDT_AUTHN           0x5AB9
#define WDT_DEFAULT_CNT     0x3EF1480ul
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct wdt_csr_wdcr
{
    union {
        struct {
            volatile uint32_t ENABLE     : 1;   // (rw) watchdog timer enable (1)
            volatile uint32_t RESET      : 1;   // (rw) watchdog time out system reset enable (1)
            volatile uint32_t INTR       : 1;   // (rw) watchdog time out system interrupt enable (1)
            volatile uint32_t EXT        : 1;   // (rw) watchdog timer external signal enable (1)
            volatile uint32_t CLOCK      : 1;   // (rw) watchdog timer clock source 0) PCLK 1) EXTCLK
            volatile uint32_t reserved   : 27;  // (rw)
        } b;

        volatile uint32_t wdcr;
    };
} wdt_csr_wdcr_t;

/**
 *  Control/Status Register (CSR) of WDT
 */
typedef struct wdt_csr
{
    volatile uint32_t WdCounter;
    volatile uint32_t WdLoad;
    volatile uint32_t WdRestart;
    volatile uint32_t WdCR;
    volatile uint32_t WdStatus;
    volatile uint32_t WdClear;
    volatile uint32_t WdIntrlen;
    volatile uint32_t WdRevision;
} wdt_csr_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
wdt_err_t
hal_wdt_init(
    wdt_mode_t  mode)
{
    wdt_err_t   rval = WDT_ERR_OK;
    do {
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdCR   = 0;
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdLoad = WDT_DEFAULT_CNT;

        /**
         *  Set the duration (clock cycles base on PCLK) of the assertion
         *  of wd_rst, wd_intr, and wd_ext signals
         */
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdIntrlen = 0xFFul;

        // set WDT mode
        if( mode == WDT_MODE_IRQ )
        {
            ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.INTR  = 1;
            ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.RESET = 0;
        }
        else if( mode == WDT_MODE_RESET )
        {
            ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.INTR  = 0;
            ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.RESET = 1;
        }
        else
        {
            rval = WDT_ERR_WRONG_PARAM;
            break;
        }

    } while(0);
    return rval;
}

wdt_err_t
hal_wdt_deinit(void)
{
    wdt_err_t   rval = WDT_ERR_OK;
    do {
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdLoad = WDT_DEFAULT_CNT;
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdCR   = 0;

        hal_irq_disable(WDT_IRQn);
    } while(0);
    return rval;
}

wdt_err_t
hal_wdt_enable(void)
{
    wdt_err_t   rval = WDT_ERR_OK;
    do {
        if( ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.INTR )
        {
            hal_irq_enable(WDT_IRQn);
        }

        ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.ENABLE = 1;

    } while(0);
    return rval;
}

wdt_err_t
hal_wdt_disable(void)
{
    wdt_err_t   rval = WDT_ERR_OK;
    do {
        ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.ENABLE = 0;

        if( ((wdt_csr_wdcr_t*)(&((wdt_csr_t*)(WDT_CSR_BASE))->WdCR))->b.INTR )
            hal_irq_disable(WDT_IRQn);
    } while(0);
    return rval;
}

wdt_err_t
hal_wdt_set_timeout(
    uint32_t    timeout)
{
    wdt_err_t   rval = WDT_ERR_OK;
    do {
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdLoad    = timeout;
        ((wdt_csr_t*)(WDT_CSR_BASE))->WdRestart = WDT_AUTHN;
    } while(0);
    return rval;
}

wdt_err_t
hal_wdt_clear_irq(void)
{
    wdt_err_t   rval = WDT_ERR_OK;
    ((wdt_csr_t*)(WDT_CSR_BASE))->WdClear = 1ul;
    return rval;
}
