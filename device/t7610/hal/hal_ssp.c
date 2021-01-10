/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_ssp.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/02/06
 * @license
 * @description
 */


#include "hal_ssp.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  the base address of CSR of ssp
 */
#define SSP0_CSR_BASE          0x54060000ul
#define SSP1_CSR_BASE          0x54070000ul
#define SSP2_CSR_BASE          0x540D0000ul


#define SSP_CR2_RXFCLR          (0x1ul << 2)    /* receive  FIFO clear */
#define SSP_CR2_TXFCLR          (0x1ul << 3)    /* transmit FIFO clear */

#define SSP_MSK_STATUS_BUSY     (0x1ul << 2)    /* bus is busy */
#define SSP_MSK_STATUS_RFVE     (0x3Ful << 4)
#define SSP_MSK_STATUS_TFVE     (0x3Ful << 12)

#define SSP_MSK_ISR_TFTHI       (0x1ul << 3)
#define SSP_MSK_ISR_RFTHI       (0x1ul << 2)

#define SSP_SDL_MAX_BITS        ((0x1ul << 7) - 1)
#define SSP_SDL_MAX_BYTES       ((SSP_SDL_MAX_BITS + 1) >> 3)

/**
 *  CM4 support 3 ssp ports
 */
typedef enum ssp_port
{
    SSP_PORT_0     = 0,
    SSP_PORT_1,
    SSP_PORT_2,
    SSP_PORT_MAX,

} ssp_port_t;

typedef enum ssp_attr
{
    SSP_ATTR_MASTER     = (0x1ul << 0),
    SSP_ATTR_SLAVE      = (0x1ul << 1),

} ssp_attr_t;

typedef enum ssp_state
{
    SSP_STATE_RESET      = 0x00,  /*!< SPI not yet initialized or disabled                */
    SSP_STATE_READY      = 0x01,  /*!< SPI initialized and ready for use                  */
    SSP_STATE_BUSY       = 0x02,  /*!< SPI process is ongoing                             */
    SSP_STATE_BUSY_TX    = 0x12,  /*!< Data Transmission process is ongoing               */
    SSP_STATE_BUSY_RX    = 0x22,  /*!< Data Reception process is ongoing                  */
    SSP_STATE_BUSY_TX_RX = 0x32,  /*!< Data Transmission and Reception process is ongoing */
    SSP_STATE_ERROR      = 0x03   /*!< SPI error state                                    */

} ssp_state_t;

//=============================================================================
//                  Macro Definition
//=============================================================================
#define SSP_CLK_RATIO_CONVERTER(pclk, rate)         ((((pclk) / (rate)) - 1) & 0xFFFF)

#define SSP_WAIT_IDLE(pCSR)    \
    do { while( ((ssp_csr_sr_t*)&((ssp_csr_t*)(pCSR))->SR)->b.BUSY ) __asm("nop"); } while(0)

#define SSP_GET_RX_CNT(pCSR)                        ((((ssp_csr_t*)(pCSR))->SR & SSP_MSK_STATUS_RFVE) >> 4)

#define SSP_IS_MASTER_ROLE(pCSR)                    (((ssp_csr_cr0_t*)&((ssp_csr_t*)(pCSR))->CR0)->b.OPM == 0x2)
//=============================================================================
//                  Structure Definition
//=============================================================================

typedef struct ssp_csr_cr0
{
    union {
        struct {
            volatile uint32_t SCLKPH   : 1;
            volatile uint32_t SCLKPO   : 1;
            volatile uint32_t OPM      : 2;
            volatile uint32_t FSJSTFY  : 1;
            volatile uint32_t FSPO     : 1;
            volatile uint32_t LSB      : 1;
            volatile uint32_t LBM      : 1;
            volatile uint32_t FSDIST   : 2;
            volatile uint32_t Validity : 1;
            volatile uint32_t FLASH    : 1;
            volatile uint32_t FFMT     : 3;
            volatile uint32_t SPIFSPO  : 1;
            volatile uint32_t SCLKFDBK : 1;
            volatile uint32_t FSFDBK   : 1;
        } b;

        volatile uint32_t cr0;
    };
} ssp_csr_cr0_t;

typedef struct ssp_csr_cr1
{
    union {
        struct {
            volatile uint32_t SCLKDIV : 16;
            volatile uint32_t SDL     : 7;
            volatile uint32_t         : 1;
            volatile uint32_t PDL     : 8;
        } b;

        volatile uint32_t cr1;
    };
} ssp_csr_cr1_t;

typedef struct ssp_csr_cr2
{
    union {
        struct {
            volatile uint32_t SSPEN  : 1;   // enable ssp
            volatile uint32_t TXDOE  : 1;   // enable tx (only valid in ssp slave mode)
            volatile uint32_t RXFCLR : 1;   // rx fifo clear
            volatile uint32_t TXFCLR : 1;   // tx fifo clear
            volatile uint32_t ACWRST : 1;
            volatile uint32_t ACCRST : 1;
            volatile uint32_t SSPRST : 1;   // SSP Reset
            volatile uint32_t RXEN   : 1;   // enable rx (spi/i2s/uWire mode)
            volatile uint32_t TXEN   : 1;   // enable tx (spi/i2s/uWire mode)
            volatile uint32_t FS     : 1;   // frame sync output level (spi)
            volatile uint32_t FSOS   : 2;   // frame sync Output Select
        } b;

        volatile uint32_t cr2;
    };
} ssp_csr_cr2_t;

typedef struct ssp_csr_cr3
{
    union {
        struct {
            volatile uint32_t PCL   : 10;   // Padding Cycle Length
            volatile uint32_t       : 2;
            volatile uint32_t DPDLEN: 1;
            volatile uint32_t       : 3;
            volatile uint32_t DPDL  : 8;
            volatile uint32_t       : 8;
        } b;

        volatile uint32_t cr3;
    };
} ssp_csr_cr3_t;

typedef struct ssp_csr_sr
{
    union {
        struct {
            volatile uint32_t RFF  : 1;
            volatile uint32_t TFNF : 1;
            volatile uint32_t BUSY : 1;
            volatile uint32_t      : 1;
            volatile uint32_t RFVE : 6;
            volatile uint32_t      : 2;
            volatile uint32_t TFVE : 6;
        } b;

        volatile uint32_t sr;
    };
} ssp_csr_sr_t;

typedef struct ssp_csr_icr
{
    union{
        struct {
            volatile uint32_t RFORIEN    : 1;
            volatile uint32_t TFURIEN    : 1;
            volatile uint32_t RFTHIEN    : 1;
            volatile uint32_t TFTHIEN    : 1;
            volatile uint32_t RFDMAEN    : 1;
            volatile uint32_t TFDMAEN    : 1;
            volatile uint32_t AC97FCENL  : 1;
            volatile uint32_t RFTHOD     : 5;
            volatile uint32_t TFTHOD     : 5;
            volatile uint32_t RFTHOD_UNIT: 1;
        } b;

        volatile uint32_t icr;
    };
} ssp_csr_icr_t;

typedef struct ssp_csr_isr
{
    union {
        struct {
            volatile uint32_t RFORI  : 1;
            volatile uint32_t TFURI  : 1;
            volatile uint32_t RFTHI  : 1;
            volatile uint32_t TFTHI  : 1;
            volatile uint32_t AC97FCI: 1;

        } b;

        volatile uint32_t isr;
    };
} ssp_csr_isr_t;

/**
 *  Control/Status Register (CSR) of SSP
 */
typedef struct ssp_csr
{
    volatile uint32_t CR0;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t ICR;
    volatile uint32_t ISR;
    volatile uint32_t DR;
    volatile uint32_t CR3;
} ssp_csr_t;

struct ssp_dev;

typedef ssp_err_t (*cb_tx_isr_t)(struct ssp_dev *pDev, ssp_csr_t *pCSR, void *pTunnel_info);
typedef ssp_err_t (*cb_rx_isr_t)(struct ssp_dev *pDev, ssp_csr_t *pCSR,
                                 cb_ssp_filter_data_t cb_filter_data, void *pTunnel_info);

typedef struct ssp_dev
{
    ssp_handle_t    handle;

    ssp_attr_t      attr;
    ssp_state_t     state;
    uint8_t         *pBuf_tx;
    uint8_t         *pBuf_rx;
    uint16_t        buf_len_tx;
    uint16_t        buf_len_rx;
    uint16_t        real_len_rx;

    cb_usr_cs_t     cb_usr_cs;
    void            *pUsr_info;

    cb_tx_isr_t     cb_tx_isr;
    cb_rx_isr_t     cb_rx_isr;

} ssp_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static ssp_dev_t        g_ssp_dev[SSP_PORT_MAX] =
{
    [SSP_PORT_0] = {
        .handle = { .instance = SSP0_CSR_BASE, .port_id = SSP_PORT_0, .irq_id = SSP0_IRQn, },
        .attr = SSP_ATTR_MASTER,
    },
    [SSP_PORT_1] = {
        .handle = { .instance = SSP1_CSR_BASE, .port_id = SSP_PORT_1, .irq_id = SSP1_IRQn, },
        .attr = SSP_ATTR_SLAVE,
    },
    [SSP_PORT_2] = {
        .handle = { .instance = SSP2_CSR_BASE, .port_id = SSP_PORT_2, .irq_id = SSP2_IRQn, },
        .attr = SSP_ATTR_MASTER | SSP_ATTR_SLAVE,
    },
};
//=============================================================================
//                  Private Function Definition
//=============================================================================
static ssp_err_t
_ssp_flush_fifo(
    ssp_csr_t   *pCSR)
{
    ssp_err_t   rval = SSP_ERR_OK;

    pCSR->CR2 |= (SSP_CR2_RXFCLR | SSP_CR2_TXFCLR);

    while( pCSR->CR2 & (SSP_CR2_RXFCLR | SSP_CR2_TXFCLR) )
        __asm("nop");

    return rval;
}

__isr_func_n ssp_err_t
_hal_ssp_tx_isr(
    ssp_dev_t   *pDev,
    ssp_csr_t   *pCSR,
    void        *pTunnel_info)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        uint32_t    has_send_dummy = (pDev->pBuf_tx) ? false : true;

        // disable TX IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHIEN = 0;

        if( !pDev->buf_len_tx )
        {
            // Stop transmission
            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXEN = 0;

            // set chip selection if necessary
            if( SSP_IS_MASTER_ROLE(pCSR) && pDev->cb_usr_cs )
            {
                rval = pDev->cb_usr_cs(SSP_CS_INACTIVE, pDev->pUsr_info);
                // if( rval ) break; // I don't care error
            }

            pDev->state     = SSP_STATE_READY;
            pDev->pBuf_tx   = 0;
            pDev->pUsr_info = 0;
            break;
        }

        while( ((ssp_csr_sr_t*)&pCSR->SR)->b.TFNF )
        {
            switch( pDev->handle.data_size )
            {
                case SSP_DATASIZE_8B:
                    pCSR->DR = (has_send_dummy) ? 0x0 : *pDev->pBuf_tx++;
                    pDev->buf_len_tx--;
                    break;
                case SSP_DATASIZE_16B:
                    pCSR->DR = (has_send_dummy) ? 0x0 : *((uint16_t*)pDev->pBuf_tx);
                    pDev->pBuf_tx    += sizeof(uint16_t);
                    pDev->buf_len_tx -= sizeof(uint16_t);
                    break;
                case SSP_DATASIZE_32B:
                    pCSR->DR = (has_send_dummy) ? 0x0 : *((uint32_t*)pDev->pBuf_tx);
                    pDev->pBuf_tx    += sizeof(uint32_t);
                    pDev->buf_len_tx -= sizeof(uint32_t);
                    break;
                default:
                    // never happen
                    break;
            }

            if( !pDev->buf_len_tx )
                break;
        }

        // enable TX IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHIEN = 1;

    } while(0);
    return rval;
}

__isr_func_n ssp_err_t
_hal_ssp_rx_isr(
    ssp_dev_t               *pDev,
    ssp_csr_t               *pCSR,
    cb_ssp_filter_data_t    cb_filter_data,
    void                    *pTunnel_info)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        uint32_t    recv_symbols = ((ssp_csr_sr_t*)&pCSR->SR)->b.RFVE;

        if( pDev->real_len_rx == pDev->buf_len_rx )
        {
            // disable RX IRQ
            ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHIEN = 0;

            // Stop receive
            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXEN = 0;

            pDev->state       = SSP_STATE_READY;
            pDev->pBuf_rx     = 0;
            pDev->pUsr_info   = 0;
            pDev->real_len_rx = 0;
            pDev->buf_len_rx  = 0;
            break;
        }

        while( recv_symbols-- )
        {
            uint32_t    value = pCSR->DR;
            uint32_t    valid_bytes = 0;

            switch( pDev->handle.data_size )
            {
                case SSP_DATASIZE_8B:
                    if( cb_filter_data )
                    {
                        valid_bytes = cb_filter_data(SSP_DATASIZE_8B, value & 0xFF,
                                                     pTunnel_info);
                        if( !valid_bytes ) break;
                    }

                    *pDev->pBuf_rx++ = (uint8_t)(value & 0xFF);
                    pDev->real_len_rx++;
                    break;

                case SSP_DATASIZE_16B:
                    if( cb_filter_data )
                    {
                        valid_bytes = cb_filter_data(SSP_DATASIZE_16B, value & 0xFFFF,
                                                     pTunnel_info);
                        if( !valid_bytes ) break;
                    }

                    *((uint16_t*)pDev->pBuf_rx) = (uint16_t)(value & 0xFFFF);
                    pDev->pBuf_rx     += sizeof(uint16_t);
                    pDev->real_len_rx += sizeof(uint16_t);
                    break;

                case SSP_DATASIZE_32B:
                    if( cb_filter_data )
                    {
                        valid_bytes = cb_filter_data(SSP_DATASIZE_32B, value, pTunnel_info);
                        if( !valid_bytes ) break;
                    }
                    *((uint32_t*)pDev->pBuf_rx) = value;
                    pDev->pBuf_rx     += sizeof(uint32_t);
                    pDev->real_len_rx += sizeof(uint32_t);
                    break;
                default:
                    // never happen
                    break;
            }

            if( pDev->real_len_rx == pDev->buf_len_rx )
                break;
        }

        // enable RX IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHIEN = 1;
    } while(0);
    return rval;
}

static ssp_err_t
_hal_ssp_full_duplex(
    ssp_dev_t   *pDev,
    ssp_csr_t   *pCSR)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        if( pDev->state == SSP_STATE_READY ||
            (SSP_IS_MASTER_ROLE(pCSR) && pDev->state == SSP_STATE_BUSY_RX) )
        {
            uint32_t    has_send_dummy = (pDev->pBuf_tx) ? false : true;

            // clear TX/RX FIFO
            _ssp_flush_fifo(pCSR);

            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXEN   = 0;
            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXEN   = 0;

            // push data to TX FIFO
            while( ((ssp_csr_sr_t*)&pCSR->SR)->b.TFNF )
            {
                switch( pDev->handle.data_size )
                {
                    case SSP_DATASIZE_8B:
                        pCSR->DR = (has_send_dummy) ? 0x0 : *pDev->pBuf_tx++;
                        pDev->buf_len_tx--;
                        break;
                    case SSP_DATASIZE_16B:
                        pCSR->DR = (has_send_dummy) ? 0x0 : *((uint16_t*)pDev->pBuf_tx);
                        pDev->pBuf_tx    += sizeof(uint16_t);
                        pDev->buf_len_tx -= sizeof(uint16_t);
                        break;
                    case SSP_DATASIZE_32B:
                        pCSR->DR = (has_send_dummy) ? 0x0 : *((uint32_t*)pDev->pBuf_tx);
                        pDev->pBuf_tx    += sizeof(uint32_t);
                        pDev->buf_len_tx -= sizeof(uint32_t);
                        break;
                    default:
                        // never happen
                        break;
                }

                if( !pDev->buf_len_tx )
                    break;
            }

            // set TX/RX IRQ threshold
            ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHOD  = 1;
            ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHOD  = (pDev->handle.data_size == SSP_DATASIZE_8B)
                                                    ? 0x4: 0x2;

            // set chip selection if necessary
            if( SSP_IS_MASTER_ROLE(pCSR) && pDev->cb_usr_cs )
            {
                rval = pDev->cb_usr_cs(SSP_CS_INACTIVE, pDev->pUsr_info);
                if( rval ) break;
            }

            // enable TX/RX IRQ
            ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHIEN = 1;
            ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHIEN = 1;

            // Start transmission and receive
            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXEN = 1;
            ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXEN = 1;
        }
    } while(0);
    return rval;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
ssp_err_t
hal_ssp_init(
    uint32_t        target_id,
    ssp_config_t    *pConfig,
    ssp_handle_t    **ppHSSP)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_csr_t   *pCSR = 0;
        uint32_t    value = 0ul;

        if( !ppHSSP || !pConfig )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        *ppHSSP = 0;

        if( target_id > (SSP_PORT_MAX - 1) )
        {
            rval = SSP_ERR_NO_INSTANCE;
            break;
        }

        if( (pConfig->role == SSP_ROLE_MASTER &&
             !(g_ssp_dev[target_id].attr & SSP_ATTR_MASTER)) ||
            (pConfig->role == SSP_ROLE_SLAVE &&
             !(g_ssp_dev[target_id].attr & SSP_ATTR_SLAVE)) )
        {
            rval = SSP_ERR_WRONG_PARAM;
            break;
        }

        if( (pConfig->role == SSP_ROLE_MASTER &&
             pConfig->pclk < pConfig->clock_rate) ||
            (pConfig->role == SSP_ROLE_SLAVE &&
             (pConfig->pclk / 3) < pConfig->clock_rate) )
        {
            rval = SSP_ERR_OVER_CLOCK;
            break;
        }

        if( hal_verify_ic_package(g_ssp_dev[target_id].handle.irq_id)
                == HAL_ERRNO_NOT_SUPPORT )
        {
            rval = SSP_ERR_NOT_SUPPORT;
            break;
        }

        pCSR = (ssp_csr_t*)g_ssp_dev[target_id].handle.instance;

        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.SSPEN  = 0;
        pCSR->ICR = 0ul;

        // reset SSP
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.SSPRST = 1;
        while( ((ssp_csr_cr2_t*)&pCSR->CR2)->b.SSPRST ) {}

        //--------------------------
        value = 0ul;
        ((ssp_csr_cr2_t*)&value)->b.TXDOE  = 1;
        ((ssp_csr_cr2_t*)&value)->b.RXFCLR = 1;
        ((ssp_csr_cr2_t*)&value)->b.TXFCLR = 1;
        ((ssp_csr_cr2_t*)&value)->b.RXEN   = 0;
        ((ssp_csr_cr2_t*)&value)->b.TXEN   = 0;
        ((ssp_csr_cr2_t*)&value)->b.FS     = 1;
        ((ssp_csr_cr2_t*)&value)->b.FSOS   = pConfig->chip_select & 0x3;

        pCSR->CR2 = value;

        //--------------------------
        // Divisor
        value = 0ul;
        ((ssp_csr_cr1_t*)&value)->b.SCLKDIV = SSP_CLK_RATIO_CONVERTER(pConfig->pclk, pConfig->clock_rate);
        ((ssp_csr_cr1_t*)&value)->b.SDL     = pConfig->data_size - 1;

        pCSR->CR1 = value;

        //--------------------------
        // configure
        value = 0ul;
        ((ssp_csr_cr0_t*)&value)->b.SCLKPH    = (pConfig->cpha == SSP_CPHA_1_EDGE) ? 0 : 1;
        ((ssp_csr_cr0_t*)&value)->b.SCLKPO    = (pConfig->cpol == SSP_CPOL_LOW) ? 0 : 1;
        ((ssp_csr_cr0_t*)&value)->b.OPM       = (pConfig->role == SSP_ROLE_MASTER) ? 0x2 : 0x0;
        ((ssp_csr_cr0_t*)&value)->b.LSB       = (pConfig->first_bit == SSP_FB_MSB) ? 0x0 : 0x1;
        ((ssp_csr_cr0_t*)&value)->b.FFMT      = 0x1; // set to SPI mode

        pCSR->CR0 = value;

        //--------------------------
        // CR3
        value = 0ul;
        ((ssp_csr_cr3_t*)&value)->b.PCL = 20;

        pCSR->CR3 = value;

        //--------------------------
        // enable IRQ
        value = 0ul;
        // ((ssp_csr_icr_t*)&value)->b.RFDMAEN = 1;
        // ((ssp_csr_icr_t*)&value)->b.TFDMAEN = 1;

        ((ssp_csr_icr_t*)&value)->b.RFTHIEN = 0;
        ((ssp_csr_icr_t*)&value)->b.RFTHOD  = 4;

        ((ssp_csr_icr_t*)&value)->b.TFTHIEN = 0;
        ((ssp_csr_icr_t*)&value)->b.TFTHOD  = 1;

        pCSR->ICR = value;

        //--------------------------
        value = pCSR->DR;

        g_ssp_dev[target_id].state            = SSP_STATE_READY;
        g_ssp_dev[target_id].handle.data_size = pConfig->data_size;

        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.SSPEN = 1;

        *ppHSSP = (ssp_handle_t*)&g_ssp_dev[target_id].handle;
    } while(0);
    return rval;
}

ssp_err_t
hal_ssp_deinit(
    ssp_handle_t    **ppHSSP)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_dev_t   *pDev = 0;
        ssp_csr_t   *pCSR = 0;

        if( !ppHSSP || !(*ppHSSP) ) break;

        pDev = DOWN_CAST(ssp_dev_t, *ppHSSP, handle);
        pCSR = (ssp_csr_t*)pDev->handle.instance;

        hal_irq_disable(pDev->handle.irq_id);

        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.SSPEN = 0;

        pCSR->CR0 = 0x0000010C;
        pCSR->CR1 = 0x00078000;
        pCSR->CR2 = 0x0;

        ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFORIEN = 0;
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFURIEN = 0;
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHIEN = 0;
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHIEN = 0;

    } while(0);
    return rval;
}

ssp_err_t
hal_ssp_send(
    ssp_handle_t    *pHSSP,
    ssp_data_set_t  *pData_set)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_csr_t   *pCSR = (ssp_csr_t*)pHSSP->instance;
        ssp_dev_t   *pDev = DOWN_CAST(ssp_dev_t, pHSSP, handle);

        if( !pHSSP || !pData_set )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        if( !pData_set->pData || !pData_set->length ||
            (pData_set->length & 0x3) )
        {
            rval = SSP_ERR_WRONG_PARAM;
            break;
        }

        if( pDev->state != SSP_STATE_READY )
        {
            rval = SSP_ERR_BUSY;
            break;
        }

        pDev->state       = SSP_STATE_BUSY_TX;
        pDev->pBuf_tx     = (uint8_t*)pData_set->pData;
        pDev->buf_len_tx  = (uint16_t)pData_set->length;
        pDev->cb_usr_cs   = pData_set->cb_usr_cs;
        pDev->pUsr_info   = pData_set->pUsr_info;
        pDev->cb_tx_isr   = _hal_ssp_tx_isr;
        pDev->cb_rx_isr   = 0;
        pDev->pBuf_rx     = 0;
        pDev->real_len_rx = 0;
        pDev->buf_len_rx  = 0;

        // clear TX FIFO
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXFCLR = 1;
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXEN   = 0;

        while( ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXFCLR )
            __asm("nop");

        // push data to TX FIFO
        while( ((ssp_csr_sr_t*)&pCSR->SR)->b.TFNF )
        {
            switch( pDev->handle.data_size )
            {
                case SSP_DATASIZE_8B:
                    pCSR->DR = *pDev->pBuf_tx++;
                    pDev->buf_len_tx--;
                    break;
                case SSP_DATASIZE_16B:
                    pCSR->DR = *((uint16_t*)pDev->pBuf_tx);
                    pDev->pBuf_tx    += sizeof(uint16_t);
                    pDev->buf_len_tx -= sizeof(uint16_t);
                    break;
                case SSP_DATASIZE_32B:
                    pCSR->DR = *((uint32_t*)pDev->pBuf_tx);
                    pDev->pBuf_tx    += sizeof(uint32_t);
                    pDev->buf_len_tx -= sizeof(uint32_t);
                    break;
                default:
                    // never happen
                    break;
            }

            if( !pDev->buf_len_tx )
                break;
        }

        // set TX threshold of IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHOD  = 1;

        // set chip selection if necessary
        if( SSP_IS_MASTER_ROLE(pCSR) && pDev->cb_usr_cs )
        {
            rval = pDev->cb_usr_cs(SSP_CS_INACTIVE, pDev->pUsr_info);
            if( rval ) break;
        }

        // enable TX IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.TFTHIEN = 1;

        // Start transmission
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.TXEN    = 1;
    } while(0);
    return rval;
}

ssp_err_t
hal_ssp_recv(
    ssp_handle_t    *pHSSP,
    ssp_data_set_t  *pData_set)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_csr_t   *pCSR = (ssp_csr_t*)pHSSP->instance;
        ssp_dev_t   *pDev = DOWN_CAST(ssp_dev_t, pHSSP, handle);

        if( !pHSSP || !pData_set )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        if( !pData_set->pData || !pData_set->length ||
            (pData_set->length & 0x3) )
        {
            rval = SSP_ERR_WRONG_PARAM;
            break;
        }

        if( pDev->state != SSP_STATE_READY )
        {
            rval = SSP_ERR_BUSY;
            break;
        }

        pDev->state      = SSP_STATE_BUSY_RX;
        pDev->pBuf_rx    = (uint8_t*)pData_set->pData;
        pDev->buf_len_rx = (uint16_t)pData_set->length;
        pDev->cb_usr_cs  = pData_set->cb_usr_cs;
        pDev->pUsr_info  = pData_set->pUsr_info;
        pDev->cb_rx_isr  = _hal_ssp_rx_isr;
        pDev->cb_tx_isr  = 0;
        pDev->pBuf_tx    = 0;
        pDev->buf_len_tx = 0;

        if( SSP_IS_MASTER_ROLE(pCSR) )
        {
            pDev->cb_tx_isr  = _hal_ssp_tx_isr;
            pDev->pBuf_tx    = 0;
            pDev->buf_len_tx = pDev->buf_len_rx;

            rval = _hal_ssp_full_duplex(pDev, pCSR);
            break;
        }

        // clear RX FIFO
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXFCLR = 1;
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXEN   = 0;

        while( ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXFCLR )
            __asm("nop");

        // set RX threshold of IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHOD  = (pDev->handle.data_size == SSP_DATASIZE_8B)
                                                ? 0x4: 0x2;

        // enable RX IRQ
        ((ssp_csr_icr_t*)&pCSR->ICR)->b.RFTHIEN = 1;

        // Start receive
        ((ssp_csr_cr2_t*)&pCSR->CR2)->b.RXEN = 1;
    } while(0);
    return rval;
}

/**
 *  @brief  hal_ssp_flush (Non thread-safe)
 *              Clear FIFOs (TX and RX) of SSP module
 *
 *  @param [in] pHSSP           the SSP handler
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_flush(
    ssp_handle_t    *pHSSP)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        if( !pHSSP )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        _ssp_flush_fifo((ssp_csr_t*)pHSSP->instance);

    } while(0);
    return rval;
}

/**
 *  @brief  hal_ssp_get_irq_type (Non thread-safe)
 *
 *  @param [in] pHSSP           the SSP handler
 *  @param [in] pEvent          report the event flags, reference enum ssp_event
 *  @return
 *      error code, reference enum ssp_err
 */
ssp_err_t
hal_ssp_get_irq_type(
    ssp_handle_t    *pHSSP,
    uint32_t        *pEvent)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_csr_isr_t   csr_isr = { .isr = 0ul, };

        if( !pHSSP || !pEvent )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        // clear IRQ (RC, read and clear)
        csr_isr.isr = ((ssp_csr_t*)pHSSP->instance)->ISR;

        *pEvent = 0ul;
        if( csr_isr.b.RFTHI )
            *pEvent |= SSP_EVENT_RX_IN;
        if( csr_isr.b.TFTHI )
            *pEvent |= SSP_EVENT_TX_END;
    } while(0);
    return rval;
}

ssp_err_t
hal_ssp_get_state(
    ssp_handle_t    *pHSSP)
{
    ssp_err_t   rval = SSP_ERR_OK;
    ssp_dev_t   *pDev = DOWN_CAST(ssp_dev_t, pHSSP, handle);

    if( pDev->state != SSP_STATE_READY )
        rval = SSP_ERR_BUSY;

    return rval;
}

ssp_err_t
hal_ssp_irq_handle(
    ssp_handle_t            *pHSSP,
    cb_ssp_filter_data_t    cb_filter_data,
    void                    *pTunnel_info)
{
    ssp_err_t   rval = SSP_ERR_OK;
    do {
        ssp_csr_t   *pCSR = (ssp_csr_t*)pHSSP->instance;
        ssp_dev_t   *pDev = DOWN_CAST(ssp_dev_t, pHSSP, handle);
        uint32_t    value = 0ul;

        if( !pHSSP )
        {
            rval = SSP_ERR_NULL_POINTER;
            break;
        }

        hal_irq_clear(pHSSP->irq_id, (void*)0);

        // clear IRQ (RC, read and clear)
        value = pCSR->ISR;

        // Transmit FIFO Threshold Interrupt
        if( (value & SSP_MSK_ISR_TFTHI) && pDev->cb_tx_isr )
        {
            pDev->cb_tx_isr(pDev, pCSR, pTunnel_info);
        }

        // Receive FIFO Threshold Interrupt
        if( (value & SSP_MSK_ISR_RFTHI) && pDev->cb_rx_isr )
        {
            pDev->cb_rx_isr(pDev, pCSR, cb_filter_data, pTunnel_info);
        }

    } while(0);
    return rval;
}
