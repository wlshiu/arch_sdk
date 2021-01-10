/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_uart.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include "hal_uart.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of uart
 */
#define UART0_CSR_BASE          0x54010000ul
#define UART1_CSR_BASE          0x54080000ul
#define UART2_CSR_BASE          0x54090000ul
#define UART3_CSR_BASE          0x540A0000ul
#define UART4_CSR_BASE          0x540B0000ul
#define UART5_CSR_BASE          0x540C0000ul


/**
 * These are the definitions for the Interrupt Enable Register
 */
#define UART_IER_MSI            0x08    /* Enable Modem status interrupt */
#define UART_IER_RLSI           0x04    /* Enable receiver line status interrupt */
#define UART_IER_THRI           0x02    /* Enable Transmitter holding register int. */
#define UART_IER_RDI            0x01    /* Enable receiver data interrupt */

/**
 * These are the definitions for the Interrupt Identification Register
 */
#define UART_IIR_NO_INT         0x01    /* No interrupts pending */
#define UART_IIR_ID             0x06    /* Mask for the interrupt ID */

#define UART_IIR_MSI            0x00    /* Modem status interrupt */
#define UART_IIR_THRI           0x02    /* Transmitter holding register empty */
#define UART_IIR_RDI            0x04    /* Receiver data interrupt */
#define UART_IIR_RLSI           0x06    /* Receiver line status interrupt */

/**
 * These are the definitions for the FIFO Control Register
 */
#define UART_FCR_FIFO_EN        0x01 /* FIFO enable */
#define UART_FCR_CLEAR_RCVR     0x02 /* Clear the RCVR FIFO */
#define UART_FCR_CLEAR_XMIT     0x04 /* Clear the XMIT FIFO */
#define UART_FCR_DMA_SELECT     0x08 /* For DMA applications */
#define UART_FCR_TRIGGER_MASK   0xC0 /* Mask for the FIFO trigger range */
#define UART_FCR_TRIGGER_1      0x00 /* Mask for trigger set at 1 */
#define UART_FCR_TRIGGER_4      0x40 /* Mask for trigger set at 4 */
#define UART_FCR_TRIGGER_8      0x80 /* Mask for trigger set at 8 */
#define UART_FCR_TRIGGER_14     0xC0 /* Mask for trigger set at 14 */

#define UART_FCR_RXSR           0x02 /* Receiver soft reset */
#define UART_FCR_TXSR           0x04 /* Transmitter soft reset */

/**
 * These are the definitions for the Line Control Register
 *
 * Note: if the word length is 5 bits (UART_LCR_WLEN5), then setting
 * UART_LCR_STOP will select 1.5 stop bits, not 2 stop bits.
 */
#define UART_LCR_WLS_MSK        0x03        /* character length select mask */
#define UART_LCR_WLS_5          0x00        /* 5 bit character length */
#define UART_LCR_WLS_6          0x01        /* 6 bit character length */
#define UART_LCR_WLS_7          0x02        /* 7 bit character length */
#define UART_LCR_WLS_8          0x03        /* 8 bit character length */
#define UART_LCR_STB            0x04        /* # stop Bits, off=1, on=1.5 or 2) */
#define UART_LCR_PEN            0x08        /* Parity enable */
#define UART_LCR_EPS            0x10        /* Even Parity Select */
#define UART_LCR_STKP           0x20        /* Stick Parity */
#define UART_LCR_SBRK           0x40        /* Set Break */
#define UART_LCR_BKSE           0x80        /* Bank select enable */
#define UART_LCR_DLAB           0x80        /* Divisor latch access bit */

#define UART_LCR_PBE_POS        3
#define UART_LCR_NSB_POS        2

/**
 * These are the definitions for the Modem Control Register
 */
#define UART_MCR_DTR            0x01        /* DTR   */
#define UART_MCR_RTS            0x02        /* RTS   */
#define UART_MCR_OUT1           0x04        /* Out 1 */
#define UART_MCR_OUT2           0x08        /* Out 2 */
#define UART_MCR_LOOP           0x10        /* Enable loopback test mode */
#define UART_MCR_AFE            0x20        /* Enable auto-RTS/CTS */

#define UART_MCR_DMA_EN         0x04
#define UART_MCR_TX_DFR         0x08

/**
 * These are the definitions for the Line Status Register
 */
#define UART_LSR_DR             0x01        /* Data ready */
#define UART_LSR_OE             0x02        /* Overrun */
#define UART_LSR_PE             0x04        /* Parity error */
#define UART_LSR_FE             0x08        /* Framing error */
#define UART_LSR_BI             0x10        /* Break */
#define UART_LSR_THRE           0x20        /* Xmit holding register empty */
#define UART_LSR_TEMT           0x40        /* Xmitter empty */
#define UART_LSR_ERR            0x80        /* Error */

#define UART_MSR_DCD            0x80        /* Data Carrier Detect */
#define UART_MSR_RI             0x40        /* Ring Indicator */
#define UART_MSR_DSR            0x20        /* Data Set Ready */
#define UART_MSR_CTS            0x10        /* Clear to Send */
#define UART_MSR_DDCD           0x08        /* Delta DCD */
#define UART_MSR_TERI           0x04        /* Trailing edge ring indicator */
#define UART_MSR_DDSR           0x02        /* Delta DSR */
#define UART_MSR_DCTS           0x01        /* Delta CTS */

/**
 *  CM4 support 5 uart ports
 */
typedef enum uart_port
{
    UART_PORT_0     = 0,
    UART_PORT_1,
    UART_PORT_2,
    UART_PORT_3,
    UART_PORT_4,
    UART_PORT_5,
    UART_PORT_MAX,

} uart_port_t;
//=============================================================================
//                  Macro Definition
//=============================================================================
#define UART_ReadLineStatus(pReg)      ((pReg)->LSR)

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct uart_csr_lcr
{
    union {
        struct {
            volatile uint32_t WL             : 2;  // (rw) word length (enum FTUART_LCR_WL)
            volatile uint32_t STOP_BITS      : 1;  // (rw) stop bits (enum FTUART_LCR_STOP_BITS)
            volatile uint32_t PARITY_EN      : 1;  // (rw) parity bit/check enable
            volatile uint32_t EVEN_PARITY    : 1;  // (rw)
            volatile uint32_t STICK_PARITY   : 1;  // (rw)
            volatile uint32_t SET_BREAK      : 1;  // (rw)
            volatile uint32_t DLAB           : 1;  // (rw) divisor latch access bit (enable access to DLL DLM and PSR)
            volatile uint32_t reserved       : 24;
        } b;

        volatile uint32_t lcr;
    };
} uart_csr_lcr_t;


typedef struct uart_csr_mcr
{
    union {
        struct {
            volatile uint32_t DTR       : 1;  // (rw) data terminal ready
            volatile uint32_t RTS       : 1;  // (rw) request to send
            volatile uint32_t OUT1      : 1;  // (rw) general purpose active-low output (io_irda_nout1)
            volatile uint32_t OUT2      : 1;  // (rw) general purpose active-low output (io_irda_nout2)
            volatile uint32_t LOOP      : 1;  // (rw) loop back mode control bit
            volatile uint32_t DMA_MODE2 : 1;  // (rw) selects uart/sir DMA mode
            volatile uint32_t OUT3      : 1;  // (rw) general purpose active-low output (io_irda_nout3)
            volatile uint32_t reserved  : 25; // (rw)
        } b;

        volatile uint32_t mcr;
    };
} uart_csr_mcr_t;


typedef struct uart_csr_lsr
{
    union {
        struct {
            volatile uint32_t DATA_READY : 1;  // (rw) at least one data word is ready in the RBR or RX FIFO (cleared if RBR or RX-FIFO is empty)
            volatile uint32_t OVERRUN    : 1;  // (rw) data overrun in RBR or RX FIFO (cleared by reading LSR)
            volatile uint32_t parity_err : 1;  // (rw) parity error (cleared by reading LSR)
            volatile uint32_t frame_err  : 1;  // (rw) frame error (no vaid stop bit) (cleared by reading LSR)
            volatile uint32_t break_int  : 1;  // (ro) break interrupt (cleared by reading LSR)
            volatile uint32_t THR_EMPTY  : 1;  // (ro) THR or TX FIFO empty (cleared when sneding out a data word to the THR or RX-FIFO)
            volatile uint32_t TX_EMPTY   : 1;  // (ro) THR or TX FIFO and TSR are empty (cleared when sneding out a data word to the THR or RX-FIFO)
            volatile uint32_t FIFO_ERR   : 1;  // (ro) data in the RX FIFO has parity or frame error or get a break indication (cleared by reading LSR)
            volatile uint32_t reserved   : 24;
        } b;

        volatile uint32_t lsr;
    };
} uart_csr_lsr_t;


typedef struct uart_csr_tst
{
    union {
        struct {
            volatile uint32_t PARITY_ERR : 1;  // (wo) send an incorrect parity
            volatile uint32_t FRAME_ERR  : 1;  // (wo) sned invalid stop bits
            volatile uint32_t BAUDGEN    : 1;  // (wo) improve baud-rate generated toggle rate
            volatile uint32_t PHY_ERR    : 1;  // (wo) send incorrect 4PPM FIR encoding
            volatile uint32_t CRC_ERR    : 1;  // (wo) sned incorrect CRC data
            volatile uint32_t reserved   : 27; // (wo)
        } b;

        volatile uint32_t tst;
    };
} uart_csr_tst_t;

typedef struct uart_csr_msr
{
    union {
        struct {
            volatile uint32_t DELTA_CTS  : 1;  // (ro)
            volatile uint32_t DELTA_DSR  : 1;  // (ro)
            volatile uint32_t TRAIL_EDGE : 1;  // (ro)
            volatile uint32_t DELTA_DCD  : 1;  // (ro)
            volatile uint32_t CTS        : 1;  // (ro) clear to send
            volatile uint32_t DSR        : 1;  // (ro) data set ready
            volatile uint32_t RI         : 1;  // (ro) ring indicator
            volatile uint32_t DCD        : 1;  // (ro) data carrier detect
            volatile uint32_t reserved   : 24;
        } b;

        volatile uint32_t msr;
    };
} uart_csr_msr_t;

typedef struct uart_csr_fcr
{
    union {
        struct {
            volatile uint32_t FIFO_ENABLE    : 1;  // (wo) 1) enable TX/RX FIFO
            volatile uint32_t RX_FIFO_RESET  : 1;  // (wo) 1) clear RX FIFO (auto reset to 0)
            volatile uint32_t TX_FIFO_RESET  : 1;  // (wo) 1) clear TX FIFO (auto reset to 0)
            volatile uint32_t DMA_MODE       : 1;  // (wo) selects uart DMA mode
            volatile uint32_t TX_FIFO_TRGL   : 2;  // (wo) TX FIFO interrupt trigger level 0)one byte 1...)depending on FIFO depth
            volatile uint32_t RX_FIFO_TRGL   : 2;  // (wo) RX FIFO interrupt trigger level 0)one byte 1...)depending on FIFO depth
            volatile uint32_t reserved       : 24;
        } b;

        volatile uint32_t fcr;
    };
} uart_csr_fcr_t;

typedef struct uart_csr_ier
{
    union {
        struct {
            volatile uint32_t RX_DATA  : 1;  // (rw) rx data available int enable
            volatile uint32_t THR_EMPT : 1;  // (rw) tx holding register empty int enable
            volatile uint32_t RX_LINE  : 1;  // (rw) rx line status int enable
            volatile uint32_t MODEM    : 1;  // (rw) modem status int enable
            volatile uint32_t RTS      : 1;  // (rw) RTS flow control enable
                                    // when enabled, loopback should be disabled and FCR[0] should be 1
            volatile uint32_t CTS      : 1;  // (rw) CTS flow control enable
                                    // when enabled, loopback should be disabled and FCR[0] should be 1
            volatile uint32_t DTR      : 1;  // (rw) DTR flow control enable
                                    // when enabled, loopback should be disabled and FCR[0] should be 1
            volatile uint32_t DSR      : 1;  // (rw) DSR flow control enable
                                    // when enabled, loopback should be disabled and FCR[0] should be 1
            volatile uint32_t reserved : 24;
        } b;

        volatile uint32_t ier;
    };
} uart_csr_ier_t;

typedef struct uart_csr_psr
{
    union {
        struct {
            volatile uint32_t PSR        : 5;    // PSR
            volatile uint32_t reserved   : 27;
        } b;
        volatile uint32_t psr;
    };
} uart_csr_psr_t;

/**
 *  Control/Status Register (CSR) of uart
 */
typedef struct uart_csr
{
    union {
        volatile uint32_t RBR;      /* Receiver Buffer Register (RBR, Offset: 0x00 for Read) */
        volatile uint32_t THR;      /* Transmitter Holding Register (THR, Offset: 0x00 for Write) */
        volatile uint32_t DLL;      /* Baud Rate Divisor Latch LSB (DLL, Offset: 0x00 when DLAB = 1) */
    };

    union {
        volatile uint32_t IER;      /* Interrupt Enable Register (IER, Offset: 0x04) */
        volatile uint32_t DLM;      /* Baud Rate Divisor Latch MSB (DLM, Offset: 0x04 when DLAB = 1) */
    };

    union {
        volatile uint32_t IIR;      /* Interrupt Identification Register (IIR, Offset: 0x08) */
        volatile uint32_t FCR;      /* FIFO Control Register (FCR, Offset: 0x08 for Write) */
        volatile uint32_t PSR;      /* Prescaler Register (PSR, Offset: 0x08 when DLAB = 1) */
    };

    volatile uint32_t LCR;          /* Line Control Register (LCR, Offset: 0x0C) */
    volatile uint32_t MCR;          /* Modem Control Register (MCR, Offset: 0x10) */

    union {
        volatile uint32_t LSR;      /* Line Status Register (LSR, Offset: 0x14 for Read) */
        volatile uint32_t TST;      /* Testing Register (TST, Offset: 0x14 for Write) */
    };

    volatile uint32_t MSR;          /* Modem Status Register (MSR, Offset: 0x18) */
    volatile uint32_t SPR;          /* Scratch Pad Register (SPR, Offset: 0x1C) */
} uart_csr_t;

typedef struct uart_dev
{
    uart_handle_t       handle;
} uart_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uart_dev_t     g_uart_dev[UART_PORT_MAX] =
{
    [UART_PORT_0] = { .handle = { .instance = (uint32_t)UART0_CSR_BASE, .port_id = UART_PORT_0, .irq_id = UART0_IRQn, }, },
    [UART_PORT_1] = { .handle = { .instance = (uint32_t)UART1_CSR_BASE, .port_id = UART_PORT_1, .irq_id = UART1_IRQn, }, },
    [UART_PORT_2] = { .handle = { .instance = (uint32_t)UART2_CSR_BASE, .port_id = UART_PORT_2, .irq_id = UART2_IRQn, }, },
    [UART_PORT_3] = { .handle = { .instance = (uint32_t)UART3_CSR_BASE, .port_id = UART_PORT_3, .irq_id = UART3_IRQn, }, },
    [UART_PORT_4] = { .handle = { .instance = (uint32_t)UART4_CSR_BASE, .port_id = UART_PORT_4, .irq_id = UART4_IRQn, }, },
    [UART_PORT_5] = { .handle = { .instance = (uint32_t)UART5_CSR_BASE, .port_id = UART_PORT_5, .irq_id = UART5_IRQn, }, },
};

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
uart_err_t
hal_uart_init(
    uint32_t        target_id,
    uart_config_t   *pConfig,
    uart_handle_t   **ppHUart)
{
    uart_err_t      rval = UART_ERR_OK;

    do {
        uart_csr_t  *pCSR = 0;
        uint32_t    value = 0ul;

        if( !ppHUart || !pConfig ||
            pConfig->irq_priority > (0x1ul << __NVIC_PRIO_BITS) )
        {
            rval = UART_ERR_WRONG_PARAM;
            break;
        }

        *ppHUart = 0;

        if( target_id > (UART_PORT_MAX - 1) )
        {
            rval = UART_ERR_NO_INSTANCE;
            break;
        }

        if( hal_verify_ic_package(g_uart_dev[target_id].handle.irq_id)
                == HAL_ERRNO_NOT_SUPPORT )
        {
            rval = UART_ERR_NOT_SUPPORT;
            break;
        }

        pCSR = (uart_csr_t*)g_uart_dev[target_id].handle.instance;

        pCSR->IER = 0;          // disable interrupt
        value     = pCSR->IIR;  // clear interrupt status

        //--------------------------
        // Divisor Latch
        value = pConfig->system_clock / (pConfig->baud_rate * 16);

        ((uart_csr_lcr_t*)&pCSR->LCR)->b.DLAB = 1;

        pCSR->DLL = value & 0xFF;
        pCSR->DLM = (value >> 8) & 0xFF;

        ((uart_csr_psr_t*)&pCSR->PSR)->b.PSR  = 1;
        ((uart_csr_lcr_t*)&pCSR->LCR)->b.DLAB = 0;

        //---------------------------------
        // reset parity, data-length, stop-bits
        value = 0;
        if( pConfig->parity == UART_PARITY_NONE )
        {
            ((uart_csr_lcr_t*)&value)->b.PARITY_EN = 0;
        }
        else if( pConfig->parity == UART_PARITY_EVEN )
        {
            ((uart_csr_lcr_t*)&value)->b.PARITY_EN    = 1;
            ((uart_csr_lcr_t*)&value)->b.EVEN_PARITY  = 1;
            ((uart_csr_lcr_t*)&value)->b.STICK_PARITY = 0;
        }
        else if( pConfig->parity == UART_PARITY_ODD )
        {
            ((uart_csr_lcr_t*)&value)->b.PARITY_EN    = 1;
            ((uart_csr_lcr_t*)&value)->b.EVEN_PARITY  = 0;
            ((uart_csr_lcr_t*)&value)->b.STICK_PARITY = 0;
        }
        else if( pConfig->parity == UART_PARITY_1 )
        {
            ((uart_csr_lcr_t*)&value)->b.PARITY_EN    = 1;
            ((uart_csr_lcr_t*)&value)->b.EVEN_PARITY  = 0;
            ((uart_csr_lcr_t*)&value)->b.STICK_PARITY = 1;
        }
        else if( pConfig->parity == UART_PARITY_0 )
        {
            ((uart_csr_lcr_t*)&value)->b.PARITY_EN    = 1;
            ((uart_csr_lcr_t*)&value)->b.EVEN_PARITY  = 1;
            ((uart_csr_lcr_t*)&value)->b.STICK_PARITY = 1;
        }
        else
        {
            rval = UART_ERR_WRONG_PARAM;
            break;
        }

        ((uart_csr_lcr_t*)&value)->b.WL         = pConfig->word_length;
        ((uart_csr_lcr_t*)&value)->b.STOP_BITS  = pConfig->stop_bits;

        pCSR->LCR = value;
        pCSR->MCR = 0;

        //--------------------------
        // set FIFO
        pCSR->FCR = 0;  // FIFO reset
        value = 0;
        ((uart_csr_fcr_t*)&value)->b.FIFO_ENABLE   = 1;   // enable FIFO
        ((uart_csr_fcr_t*)&value)->b.RX_FIFO_RESET = 1;   // reset RX FIFO
        ((uart_csr_fcr_t*)&value)->b.TX_FIFO_RESET = 1;   // reset TX FIFO
        pCSR->FCR = value;

        //--------------------------
        // IRQ configure
        ((uart_csr_ier_t*)&pCSR->IER)->b.RX_DATA = 1;

        if( pConfig->irq_priority )
            NVIC_SetPriority(g_uart_dev[target_id].handle.irq_id, pConfig->irq_priority - 1);

        //--------------------------
        *ppHUart = (uart_handle_t*)&g_uart_dev[target_id].handle;
    } while(0);

    return rval;
}

uart_err_t
hal_uart_deinit(uart_handle_t **ppHUart)
{
    uart_err_t     rval = UART_ERR_OK;

    do {
        uart_dev_t  *pDev = 0;
        uart_csr_t  *pCSR = 0;

        if( !ppHUart || !(*ppHUart) ) break;

        pDev = DOWN_CAST(uart_dev_t, *ppHUart, handle);
        pCSR = (uart_csr_t*)pDev->handle.instance;

        hal_irq_disable(pDev->handle.irq_id);

        pCSR->MCR = 0;
        pCSR->IER = 0;
        pCSR->FCR = 0;
        pCSR->DLL = 0;
        pCSR->DLM = 0;
    } while(0);

    return rval;
}

uart_err_t
hal_uart_send(
    uart_handle_t   *pHUart,
    uint8_t         *pData,
    uint32_t        length)
{
    uart_err_t     rval = UART_ERR_OK;
    do {
        uart_dev_t  *pDev = 0;
        uart_csr_t  *pCSR = 0;

        if( !pHUart || !pData )
        {
            rval = UART_ERR_NULL_POINTER;
            break;
        }

        if( !length )
        {
            rval = UART_ERR_WRONG_PARAM;
            break;
        }

        pDev = DOWN_CAST(uart_dev_t, pHUart, handle);
        pCSR = (uart_csr_t*)pDev->handle.instance;

        while( length )
        {
            while( (UART_ReadLineStatus(pCSR) & UART_LSR_THRE) == 0 ) {}

            pCSR->THR = *pData++;
            length--;
        }

    } while(0);
    return rval;
}

uart_err_t
hal_uart_recv(
    uart_handle_t   *pHUart,
    uint8_t         *pData,
    uint32_t        *pLength)
{
    uart_err_t     rval = UART_ERR_OK;
    do {
        uart_dev_t  *pDev = 0;
        uart_csr_t  *pCSR = 0;

        if( !pHUart || !pData || !pLength )
        {
            rval = UART_ERR_NULL_POINTER;
            break;
        }

        if( *pLength == 0 )
        {
            rval = UART_ERR_WRONG_PARAM;
            break;
        }

        *pLength = 0;

        pDev = DOWN_CAST(uart_dev_t, pHUart, handle);
        pCSR = (uart_csr_t*)pDev->handle.instance;

        if( ((uart_csr_lsr_t*)&pCSR->LSR)->b.DATA_READY )
        {
            *pData = pCSR->RBR & 0xFF;
            *pLength = 1;
        }
    } while(0);

    return rval;
}

uart_err_t
hal_uart_clear_irq(
    uart_handle_t    *pHUart)
{
    if( pHUart )
    {
        uint32_t __attribute__ ((unused)) value = 0;

        // clear IRQ
        value = ((uart_csr_t*)(((uart_handle_t*)(pHUart))->instance))->IIR;
    }
    return UART_ERR_OK;
}

/**
 *  @brief  hal_uart_ctrl (Non thread-safe)
 *
 *  @param [in] pHUart      the uart handler
 *  @param [in] opcode      the operation code, reference enum uart_opcode
 *  @param [in] pArgv       the arguments
 *  @return
 *      error number, reference enum uart_err
 */
uart_err_t
hal_uart_ctrl(
    uart_handle_t   *pHUart,
    uart_opcode_t   opcode,
    uart_argv_t     *pArgv)
{
    uart_err_t     rval = UART_ERR_OK;
    do {
        if( !pHUart )
        {
            rval = UART_ERR_NULL_POINTER;
            break;
        }

        if( opcode == UART_OPCODE_SET_BAUDRATE )
        {
            // it can use hal_uart_init
            rval = UART_ERR_NOT_SUPPORT;
            break;
        }
    } while(0);

    return rval;
}

