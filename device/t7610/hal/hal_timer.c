/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_timer.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include "hal_timer.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of timer
 */
#define TMR_CTL_CSR_BASE       0x54030000ul
#define TMR0_CSR_BASE          0x54030010ul
#define TMR1_CSR_BASE          0x54030020ul
#define TMR2_CSR_BASE          0x54030030ul
#define TMR3_CSR_BASE          0x54030040ul

/**
 *  CM4 support 4 timer ports
 */
typedef enum tmr_port
{
    TMR_PORT_0     = 0,
    TMR_PORT_1,
    TMR_PORT_2,
    TMR_PORT_3,
    TMR_PORT_MAX,

} tmr_port_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct tmr_csr_intcstat
{
    union {
        struct {
            volatile uint32_t TM1        : 1;   // (rw) timer 1 timeout interrupt (write 1 to clear)
            volatile uint32_t TM2        : 1;   // (rw) timer 2 timeout interrupt (write 1 to clear)
            volatile uint32_t TM3        : 1;   // (rw) timer 3 timeout interrupt (write 1 to clear)
            volatile uint32_t TM4        : 1;   // (rw) timer 4 timeout interrupt (write 1 to clear)
            volatile uint32_t reserved   : 28;  // (rw)
        } b;

        uint32_t intcstat;
    };
} tmr_csr_intcstat_t;

typedef struct tmr_csr_ctrl
{
    union {
        struct {
            volatile uint32_t CLK_SRC    : 1;   // (rw) clock source 0) PCLK 1) ext_clk
            volatile uint32_t START      : 1;   // (rw) 0) stop 1) start timer
            volatile uint32_t UPDATE     : 1;   // (rw) set to 1 to manually reload CNTB and CMPB (auto-clear)
            volatile uint32_t OUT_INV    : 1;   // (rw) set to 1 to invert timer output
            volatile uint32_t AUTO       : 1;   // (rw) set to 1 to enable timer counter auto-reload
            volatile uint32_t INT_EN     : 1;   // (rw) set to 1 to enable timer interrupt
            volatile uint32_t INT_MODE   : 1;   // (rw) interrupt mode 0) level 1) pulse
            volatile uint32_t DMA_EN     : 1;   // (rw) set to 1 to enable timer DMA request mode
            volatile uint32_t PWM_EN     : 1;   // (rw) set to 1 to enable timer PWM function
            volatile uint32_t reserved   : 23;  // (rw)
        } b;

        uint32_t ctrl;
    };
} tmr_csr_ctrl_t;

/**
 *  Control/Status Register (CSR) of Timer
 */
typedef struct tmr_csr
{
    volatile uint32_t CTRL;
    volatile uint32_t CNTB;
    volatile uint32_t CMPB;
    volatile uint32_t CNTO;
} tmr_csr_t;


typedef struct tmr_dev
{
    tmr_handle_t        handle;
} tmr_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static const tmr_dev_t      g_tmr_dev[TMR_PORT_MAX] =
{
    [TMR_PORT_0] = { .handle = { .instance = (uint32_t)TMR0_CSR_BASE, .port_id = TMR_PORT_0, .irq_id = TIM0_IRQn, }, },
    [TMR_PORT_1] = { .handle = { .instance = (uint32_t)TMR1_CSR_BASE, .port_id = TMR_PORT_1, .irq_id = TIM1_IRQn, }, },
    [TMR_PORT_2] = { .handle = { .instance = (uint32_t)TMR2_CSR_BASE, .port_id = TMR_PORT_2, .irq_id = TIM2_IRQn, }, },
    [TMR_PORT_3] = { .handle = { .instance = (uint32_t)TMR3_CSR_BASE, .port_id = TMR_PORT_3, .irq_id = TIM3_IRQn, }, },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
tmr_err_t
hal_tmr_init(
    uint32_t        target_id,
    tmr_config_t    *pConfig,
    tmr_handle_t    **ppHTmr)
{
    tmr_err_t       rval = TMR_ERR_OK;
    do {
        tmr_csr_intcstat_t  *pCSR_intcstat = (tmr_csr_intcstat_t*)TMR_CTL_CSR_BASE;
        tmr_csr_t           *pCSR = 0;

        if( !ppHTmr || !pConfig )
        {
            rval = TMR_ERR_WRONG_PARAM;
            break;
        }

        *ppHTmr = 0;

        if( target_id > (TMR_PORT_MAX - 1) )
        {
            rval = TMR_ERR_NO_INSTANCE;
            break;
        }

        pCSR = (tmr_csr_t*)g_tmr_dev[target_id].handle.instance;
        pCSR->CTRL = 0;

        ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.INT_EN  = 1;
        ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.CLK_SRC = (pConfig->clk_source == TMR_CLK_PCLK)
                                                    ? 0 : 1;

        // clear IRQ
        pCSR_intcstat->intcstat |= (0x1ul << target_id);

        //--------------------------
        *ppHTmr = (tmr_handle_t*)&g_tmr_dev[target_id].handle;
    } while(0);
    return rval;
}

tmr_err_t
hal_tmr_deinit(
    tmr_handle_t    **ppHTmr)
{
    tmr_err_t       rval = TMR_ERR_OK;

    do {
        tmr_csr_intcstat_t  *pCSR_intcstat = (tmr_csr_intcstat_t*)TMR_CTL_CSR_BASE;
        tmr_dev_t           *pDev = 0;
        tmr_csr_t           *pCSR = 0;

        if( !ppHTmr || !(*ppHTmr) ) break;

        pDev = DOWN_CAST(tmr_dev_t, *ppHTmr, handle);
        pCSR = (tmr_csr_t*)pDev->handle.instance;

        hal_irq_disable(pDev->handle.irq_id);

        pCSR->CTRL = 0;

        // clear IRQ
        pCSR_intcstat->intcstat |= (0x1ul << pDev->handle.port_id);

    } while(0);

    return rval;
}

tmr_err_t
hal_tmr_enable(
    tmr_handle_t    *pHTmr,
    tmr_mode_t      mode,
    uint32_t        timeout)
{
    tmr_err_t       rval = TMR_ERR_OK;

    do {
        tmr_csr_t           *pCSR = 0;

        if( !pHTmr )
        {
            rval = TMR_ERR_NULL_POINTER;
            break;
        }

        pCSR = (tmr_csr_t*)pHTmr->instance;

        pCSR->CNTB = timeout;
        ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.UPDATE = 1;

        while( ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.UPDATE ) {}

        if( mode == TMR_MODE_REPEAT )
            ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.AUTO = 1;

        ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.START = 1;
    } while(0);

    return rval;
}

tmr_err_t
hal_tmr_disable(
    tmr_handle_t    *pHTmr)
{
    tmr_err_t       rval = TMR_ERR_OK;

    do {
        tmr_csr_t           *pCSR = 0;

        if( !pHTmr )
        {
            rval = TMR_ERR_NULL_POINTER;
            break;
        }

        pCSR = (tmr_csr_t*)pHTmr->instance;

        pCSR->CNTB = 0;
        ((tmr_csr_ctrl_t*)(&pCSR->CTRL))->b.START = 0;

    } while(0);

    return rval;
}

tmr_err_t
hal_tmr_get_count(
    tmr_handle_t    *pHTmr,
    uint32_t        *pCount)
{
    tmr_err_t       rval = TMR_ERR_OK;

    do {
        if( !pHTmr || !pCount )
        {
            rval = TMR_ERR_NULL_POINTER;
            break;
        }

        *pCount = ((tmr_csr_t*)pHTmr->instance)->CNTO;

    } while(0);

    return rval;
}


tmr_err_t
hal_tmr_clear_irq(
    tmr_handle_t    *pHTmr)
{
    if( pHTmr )
    {
        // clear IRQ
        ((tmr_csr_intcstat_t*)TMR_CTL_CSR_BASE)->intcstat |= (0x1ul << pHTmr->port_id);
    }
    return TMR_ERR_OK;
}
