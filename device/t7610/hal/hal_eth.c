/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_eth.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */


#include <string.h>
#include "hal_eth.h"
#include "ftgmac030.h"
#include "phy.h"

//=============================================================================
//                  Constant Definition
//=============================================================================
#define ETH_SEND_TIMEOUT                10000000

#define ETH_GMAC030_REG_BASE_ADDR       0x56000000ul
#define ETH_HIGH_PRI_TXQ_EN             0

#define ETH_TX_FIFO_SIZE                0
#define ETH_RX_FIFO_SIZE                0


/**
 *  MDC_CYCTHR(MDC clock cycle threshold)
 *      20us * 100 = 2ms > (1 / 2.5Mhz) * 0x34
 */
#define ETH_MDC_CYCTHR                  0x34
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
#pragma pack(1)
typedef struct eth_hdr
{
    uint8_t     da[6];
    uint8_t     sa[6];
    uint16_t    proto;
} eth_hdr_t;

typedef struct ipv4_hdr
{
    uint8_t     ihl: 4, version: 4;
    uint8_t     services;               /* type of service */
    uint16_t    len;                    /* total length */
    uint16_t    id;
    uint16_t    flags: 3, frag_offset: 13; /* fragment offset field */
    uint8_t     ttl;
    uint8_t     protocol;
    uint16_t    checksum;

    uint32_t    sa;
    uint32_t    da;
} ipv4_hdr_t;

typedef struct eth_pkt_hdr
{
    eth_hdr_t       eth_hdr;
    ipv4_hdr_t      ipv4_hdr;
} eth_pkt_hdr_t;
#pragma pack()

struct ftgmac030_ctrl;

typedef struct ftgmac030_queue
{
    union {
        struct {
            ftgmac030_txdesc_t      *txdes_top;
            ftgmac030_txdesc_t      *txdes_cur;
        } tx;

        struct {
            ftgmac030_rxdesc_t      *rxdes_top;
            ftgmac030_rxdesc_t      *rxdes_cur;
        } rx;
    };

    void (*kickoff)(struct ftgmac030_ctrl *ctrl);

} ftgmac030_queue_t;

typedef struct ftgmac030_ctrl
{
    uintptr_t       iobase;

    uint32_t        aptc; // default setting

    phy_dev_t       phydev;

    uint32_t        phy_addr;

    int             mdc_cycthr;

    ftgmac030_queue_t       txq_np;
    ftgmac030_queue_t       txq_hp;

    ftgmac030_queue_t       rxq;

    cb_eth_rx_handler_t     cb_rx_handler;
    cb_eth_notify_t         cb_notify;

    uint32_t        is_tx_end;  // Packets transmitted to Ethernet successfully
    uint32_t        is_rx_end;  // RXDMA has received packets to the RX buffer successfully
    uint32_t        is_rx_err;  // RXDMA has received packets to the RX buffer successfully

} ftgmac030_ctrl_t;

typedef struct eth_dev
{
    ftgmac030_ctrl_t        ctrl;
    ftgmac030_txdesc_t      *pTxdesc_np;
    ftgmac030_txdesc_t      *pTxdesc_hp;
    ftgmac030_rxdesc_t      *pRxdesc;
    uint8_t                 *pRx_packets;

    hal_eth_op_t            eth_op;
    hal_eth_macaddr_t       macaddr;

} eth_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
extern phy_driver_t     g_phy_lan8720;

static eth_dev_t                g_eth_dev = {0};
static cb_eth_rx_handler_t      g_cb_rx_handler = 0;
static cb_eth_notify_t          g_cb_notify = 0;
//=============================================================================
//                  Private Function Definition
//=============================================================================
__isr_func_h int
_hal_eth_rxpkt_verify(
    ftgmac030_ctrl_t    *pCtrl,
    ftgmac030_rxdesc_t  *pRxdes)
{
    int     rval = ETH_STATE_NOTHING;

    do {
        if( pRxdes->rxdes0 & FTGMAC030_RXDES0_FIFO_FULL)
        {
            rval = ETH_STATE_RX_FIFO_FULL;
            break;
        }

        if( pRxdes->rxdes0 & FTGMAC030_RXDES0_RX_ODD_NB )
        {
            rval = ETH_STATE_RX_ODD_NIBBLES;
            break;
        }

        if( pRxdes->rxdes0 & FTGMAC030_RXDES0_CRC_ERR )
        {
            rval = ETH_STATE_RX_CRC_FAIL;
            break;
        }

        if( pRxdes->rxdes0 & FTGMAC030_RXDES0_RX_ERR )
        {
            rval = ETH_STATE_RX_ERROR;
            break;
        }

        if( pRxdes->rxdes0 & FTGMAC030_RXDES0_FTL )
        {
            rval = ETH_STATE_RX_FRAME_TOO_LONG;
            break;
        }

        if( pRxdes->rxdes1 & FTGMAC030_RXDES1_IPCS_FAIL )
        {
            rval = ETH_STATE_RX_IP_CHECKSUM_FAIL;
            break;
        }

        if( pRxdes->rxdes1 & FTGMAC030_RXDES1_UDPCS_FAIL )
        {
            rval = ETH_STATE_RX_UDP_CHECKSUM_FAIL;
            break;
        }

        if( pRxdes->rxdes1 & FTGMAC030_RXDES1_TCPCS_FAIL )
        {
            rval = ETH_STATE_RX_TCP_CHECKSUM_FAIL;
            break;
        }

    } while(0);

    if( rval != ETH_STATE_NOTHING )
    {
        if( pCtrl->cb_notify )
            pCtrl->cb_notify((eth_state_t)rval);
    }
    return rval;
}

static eth_err_t
_hal_eth_setup(
    eth_dev_t   *pDev)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        hal_eth_op_t        *pOp = &pDev->eth_op;

        if( !(pDev->pTxdesc_np
                = (ftgmac030_txdesc_t*)pOp->cb_malloc(ETH_MEM_CACHE_NP_TX,
                                                      ETH_TXQ_NP_ENTRIES * sizeof(ftgmac030_txdesc_t))) )
        {
            rval = ETH_ERR_NULL_POINTER;
            break;
        }

        if( (uintptr_t)pDev->pTxdesc_np & 0xF )
        {
            rval = ETH_ERR_NOT_ALIGNMENT;
            break;
        }

#if (ETH_HIGH_PRI_TXQ_EN)
        if( !(pDev->pTxdesc_hp
                = (ftgmac030_txdesc_t*)pOp->cb_malloc(ETH_MEM_CACHE_HP_TX,
                                                      ETH_TXQ_HP_ENTRIES * sizeof(ftgmac030_txdesc_t))) )
        {
            rval = ETH_ERR_NULL_POINTER;
            break;
        }

        if( (uintptr_t)pDev->pTxdesc_hp & 0xF )
        {
            rval = ETH_ERR_NOT_ALIGNMENT;
            break;
        }
#endif

        if( !(pDev->pRxdesc
                = (ftgmac030_rxdesc_t*)pOp->cb_malloc(ETH_MEM_CACHE_RX,
                                                      ETH_RXQ_ENTRIES * sizeof(ftgmac030_rxdesc_t))) )
        {
            rval = ETH_ERR_NULL_POINTER;
            break;
        }

        if( (uintptr_t)pDev->pRxdesc & 0xF )
        {
            rval = ETH_ERR_NOT_ALIGNMENT;
            break;
        }

        if( !(pDev->pRx_packets
                = (uint8_t*)pOp->cb_malloc(ETH_MEM_PACKET_RX,
                                           ETH_RX_PACKET_SIZE * ETH_RXQ_ENTRIES)) )
        {
            rval = ETH_ERR_NULL_POINTER;
            break;
        }

        rval = pOp->cb_set_macaddr(&pDev->macaddr);
        if( rval ) break;

        if( pOp->cb_phy_reset )
        {
            rval = pOp->cb_phy_reset();
            if( rval ) break;
        }

    } while(0);
    return rval;
}

static uint16_t
_hal_eth_mdiobus_read(mii_dev_t *bus, int phy_addr, int regnum)
{
    ftgmac030_ctrl_t    *ctrl = (ftgmac030_ctrl_t*)bus->pPriv;
    ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)ctrl->iobase;

    pReg->phycr = FTGMAC030_PHYCR_PHYRD |
                  FTGMAC030_PHYCR_REGAD(regnum) |
                  FTGMAC030_PHYCR_PHYAD(phy_addr) |
                  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_RD) |
                  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF) |
                  ctrl->mdc_cycthr;

    while( pReg->phycr & FTGMAC030_PHYCR_PHYRD ) {}

    return FTGMAC030_PHYDATA_MIIRDATA(pReg->phydata);
}

static int
_hal_eth_mdiobus_write(mii_dev_t *bus, int phy_addr, int regnum, uint16_t value)
{
    ftgmac030_ctrl_t    *pCtrl = (ftgmac030_ctrl_t*)bus->pPriv;
    ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)pCtrl->iobase;

    // write data to PHYWDATA (0x64h)
    pReg->phydata = FTGMAC030_PHYDATA_MIIWDATA(value);
    while( pReg->phycr & FTGMAC030_PHYCR_PHYWR ) {}

    // fill out PHYCR
    pReg->phycr = FTGMAC030_PHYCR_PHYWR |
                  FTGMAC030_PHYCR_REGAD(regnum) |
                  FTGMAC030_PHYCR_PHYAD(phy_addr) |
                  FTGMAC030_PHYCR_OP(FTGMAC030_MDIO_OP_WR) |
                  FTGMAC030_PHYCR_SOF(FTGMAC030_MDIO_SOF) |
                  pCtrl->mdc_cycthr;

    while( pReg->phycr & FTGMAC030_PHYCR_PHYWR ) {}

    return 0;
}

static int
_hal_eth_mdiobus_reset(mii_dev_t *bus)
{
    return 0;
}

static int
_hal_eth_mii_init(ftgmac030_ctrl_t *pCtrl)
{
    int     rval = 0;
    do {
        mii_dev_t           *pMii_dev = &pCtrl->phydev.mii_dev;

        /**
         *  GISR (GMAC Interface Selection Register)
         *      0: MII or GMII
         *      1: RMII
         *      2: RGMII
         *      3: reserved
         *      3: reserved
         */
        #if 0
        // faraday sample code
        ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)pCtrl->iobase;
        pCtrl->mdc_cycthr = (pReg->gisr > 0) ? 0x68 : 0x34;
        #else
        pCtrl->mdc_cycthr = ETH_MDC_CYCTHR;
        #endif

        pMii_dev->read  = _hal_eth_mdiobus_read;
        pMii_dev->write = _hal_eth_mdiobus_write;
        pMii_dev->reset = _hal_eth_mdiobus_reset;
        pMii_dev->pPriv = (void*)pCtrl;

    } while(0);

    return rval;
}

static uint32_t
_hal_eth_scan_phy_addr(ftgmac030_ctrl_t *pCtrl)
{
    // for rtl8211, no scan from zero
    uint32_t    phy_id = 0x1FFFFFFF;

    for(int i = 0; i < PHY_MAX_ADDR; i++)
    {
        uint16_t    id1 = 0;
        uint16_t    id2 = 0;

        // If the phy_id is mostly Fs, there is no device there
        id1 = _hal_eth_mdiobus_read(&pCtrl->phydev.mii_dev, i, MII_REG_ID1);
        id2 = _hal_eth_mdiobus_read(&pCtrl->phydev.mii_dev, i, MII_REG_ID2);

        phy_id = ((id1 << 16) | id2);

        if( (phy_id & 0x1FFFFFFF) == 0x1FFFFFFF )
            continue;

        pCtrl->phy_addr = i;
        pCtrl->phydev.phy_id   = phy_id;
        pCtrl->phydev.phy_addr = pCtrl->phy_addr;
        return phy_id;
    }

    // Not valid phy ID
    return 0x1FFFFFFF;
}

static void
_hal_eth_kickoff_tx(ftgmac030_ctrl_t *pCtrl)
{
    ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)pCtrl->iobase;

    if( 0 == (pReg->aptc & FTGMAC030_APTC_TX_CNT(0xf)) )
    {
        if( ETH_HIGH_PRI_TXQ_EN )
            pReg->hptxpd = 0xFFFFFFFF; // kick off high priority txdma
        else
            pReg->nptxpd = 0xFFFFFFFF; // kick off normal priority txdma
    }
    return;
}

static int
_hal_eth_set_ring_buf(eth_dev_t *pDev)
{
    ftgmac030_queue_t   *pQ = 0;
    ftgmac030_txdesc_t  *pTxdesc = 0;
    ftgmac030_rxdesc_t  *pRxdesc = 0;

    memset(pDev->pTxdesc_np, 0x0, ETH_TXQ_NP_ENTRIES * sizeof(ftgmac030_txdesc_t));

    // normal priority TXQ
    pQ               = &pDev->ctrl.txq_np;
    pQ->kickoff      = _hal_eth_kickoff_tx;
    pQ->tx.txdes_top = pQ->tx.txdes_cur = pDev->pTxdesc_np;

    pTxdesc = pQ->tx.txdes_top;
    for(int i = 0; i < ETH_TXQ_NP_ENTRIES; i++)
        pTxdesc[i].txdes2 = (unsigned int)&pTxdesc[i + 1];

    pTxdesc[ETH_TXQ_NP_ENTRIES - 1].txdes2 = (unsigned int)pQ->tx.txdes_top;
    pTxdesc[ETH_TXQ_NP_ENTRIES - 1].txdes0 |= FTGMAC030_TXDES0_EDOTR; // set last txdesc flag

#if (ETH_HIGH_PRI_TXQ_EN)
    memset(pDev->pTxdesc_hp, 0x0, ETH_TXQ_NP_ENTRIES * sizeof(ftgmac030_txdesc_t));

    // high priority TXQ
    pQ               = &pDev->ctrl.txq_hp;
    pQ->kickoff      = _hal_eth_kickoff_tx;
    pQ->tx.txdes_top = pQ->tx.txdes_cur = pDev->pTxdesc_hp;

    pTxdesc = pQ->tx.txdes_top;
    for(int i = 0; i < ETH_TXQ_HP_ENTRIES; i++)
        pTxdesc[i].txdes2 = (unsigned int)&pTxdesc[i + 1];

    pTxdesc[ETH_TXQ_HP_ENTRIES - 1].txdes2 = (unsigned int)pQ->tx.txdes_top;
    pTxdesc[ETH_TXQ_HP_ENTRIES - 1].txdes0 |= FTGMAC030_TXDES0_EDOTR; // set last txdesc flag

#endif

    // RXQ
    memset(pDev->pRxdesc, 0x0, ETH_RXQ_ENTRIES * sizeof(ftgmac030_rxdesc_t));

    pQ               = &pDev->ctrl.rxq;
    pQ->rx.rxdes_top = pQ->rx.rxdes_cur = pDev->pRxdesc;

    pRxdesc = pQ->rx.rxdes_top;
    for(int i = 0; i < ETH_RXQ_ENTRIES; i++)
    {
        pRxdesc[i].rxdes3 = (unsigned int)&pDev->pRx_packets[i * ETH_RX_PACKET_SIZE];
        pRxdesc[i].rxdes2 = (unsigned int)&pRxdesc[i + 1];

        /**
         *  For auto polling disable,
         *   we must set descriptor own bit to software
         */
        if( FTGMAC030_APTC_RX_CNT(pDev->ctrl.aptc) == 0 )
            pRxdesc[i].rxdes0 |= FTGMAC030_RXDES0_RXPKT_RDY;
    }

//    pRxdesc[ETH_RXQ_ENTRIES - 1].rxdes3 = 0;
    pRxdesc[ETH_RXQ_ENTRIES - 1].rxdes2 = (unsigned int)pQ->rx.rxdes_top;
    pRxdesc[ETH_RXQ_ENTRIES - 1].rxdes0 |= FTGMAC030_RXDES0_EDORR; // set last rxdesc flag

    return 0;
}

static void
_hal_eth_phy_notify(
    phy_event_t     event,
    phy_info_t      *pInfo)
{
    do {
        cb_eth_notify_t     cb_notify = g_eth_dev.ctrl.cb_notify;

        if( !cb_notify ) break;

        if( event == PHY_EVENT_STATUS )
        {
            eth_state_t     state = ETH_STATE_PHY_STATUS_UNKNOWN;

            if( pInfo->status.speed_duplex == PHY_10M_HALF )
                state = ETH_STATE_10M_HALF;
            else if( pInfo->status.speed_duplex == PHY_10M_FULL )
                state = ETH_STATE_10M_FULL;
            else if( pInfo->status.speed_duplex == PHY_100M_HALF )
                state = ETH_STATE_100M_HALF;
            else if( pInfo->status.speed_duplex == PHY_100M_FULL )
                state = ETH_STATE_100M_FULL;

            cb_notify(state);
        }
    } while(0);
    return;
}

__isr_func_h void GMAC_IRQHandler(void)
{
    ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)g_eth_dev.ctrl.iobase;
    phy_driver_t        *pPhy_drv = g_eth_dev.ctrl.phydev.pPhy_driver;
    cb_eth_notify_t     cb_notify = g_eth_dev.ctrl.cb_notify;
    uint32_t            irq_state = 0;

    // disable all IRQs of MAC
    pReg->ier = 0ul;

    irq_state = pReg->isr;

    // write 1 clear to reset all interrupts (faraday sample code, no idea)
    pReg->isr = irq_state;

    pPhy_drv->clear_irq(&g_eth_dev.ctrl.phydev);

#if 0
    if( irq_state & FTGMAC030_INT_TPKT2F )
    {
        if( cb_notify )
            cb_notify(ETH_STATE_TX2FIFO);
    }
#endif

    if( irq_state & FTGMAC030_INT_TPKT2E )
    {
        if( cb_notify )
            cb_notify(ETH_STATE_TX_END);

        g_eth_dev.ctrl.is_tx_end = 1ul;
    }

    // PHY link status changing
    if( irq_state & FTGMAC030_INT_PHYSTS_CHG )
    {
        if( cb_notify )
            cb_notify(ETH_STATE_LINK_STATE_CHANGE);
    }

    if( irq_state & FTGMAC030_INT_RPKT2B )
    {
        /**
         * Note.
         * If two packets or more arrived at the almost same time,
         * ftgmac030 will trigger interrupt only once.
         * So we need to handle all received packets in this interrupt.
         */

        int     rval = 0;
        do {
            ftgmac030_rxdesc_t      *pRxdes = g_eth_dev.ctrl.rxq.rx.rxdes_cur;
            cb_eth_rx_handler_t     cb_rx_handler = g_eth_dev.ctrl.cb_rx_handler;

            if( pRxdes->rxdes0 & FTGMAC030_RXDES0_FRS )
            {
                rval = _hal_eth_rxpkt_verify(&g_eth_dev.ctrl, pRxdes);
                if( rval )
                {
                    // release own of this rxdes
                    pRxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
                    pRxdes->rxdes1 = 0x0;
                    g_eth_dev.ctrl.rxq.rx.rxdes_cur = (ftgmac030_rxdesc_t*)pRxdes->rxdes2;
                    break;
                }
            }
            else
            {
                // check the valid interrupt
                if( pReg->rxr_ptr == (unsigned int)pRxdes )
                    break;  // skip this IRQ

                // release own of this rxdes
                pRxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
                pRxdes->rxdes1 = 0x0;
                g_eth_dev.ctrl.rxq.rx.rxdes_cur = (ftgmac030_rxdesc_t*)pRxdes->rxdes2;
                break;
            }

            while(1)
            {
                uint32_t    rxpkt_buf_size = pReg->rbsr;
                uint32_t    rxdes0_value = 0;
                uint32_t    rx_size = 0;

                // wait for rx_dma completes the frame reception
                while( !(pRxdes->rxdes0 & FTGMAC030_RXDES0_RXPKT_RDY) ) {}

                rx_size = FTGMAC030_RXDES0_VDBC(pRxdes->rxdes0);
                if( rx_size > rxpkt_buf_size )
                {
                    rval = -1;
                    break;
                }

                // pass the packet up to the protocol layers
                if( cb_rx_handler )
                    cb_rx_handler((uint8_t*)pRxdes->rxdes3, (int)rx_size - 4);

                rxdes0_value = pRxdes->rxdes0;

                // release own of this rxdes
                pRxdes->rxdes0 &= FTGMAC030_RXDES0_EDORR;
                pRxdes->rxdes1 = 0x0;
                g_eth_dev.ctrl.rxq.rx.rxdes_cur = (ftgmac030_rxdesc_t*)pRxdes->rxdes2;

                pRxdes = g_eth_dev.ctrl.rxq.rx.rxdes_cur;
                if( rxdes0_value & FTGMAC030_RXDES0_LRS )
                    break;

            }

            if( rval )  break;

            g_eth_dev.ctrl.is_rx_end = 1ul;
        } while(0);
    }

    // enable IRQs of MAC
    pReg->ier = (uint32_t)FTGMAC030_INT_DEFAULT;
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
eth_err_t
hal_eth_init(
    hal_eth_op_t    *pOperator)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        phy_driver_t        *pPhy_drv = 0;
        ftgmac030_reg_t     *pReg = 0;

        memset(&g_eth_dev, 0x0, sizeof(g_eth_dev));

        if( !pOperator || !pOperator->cb_set_macaddr ||
            !pOperator->cb_malloc || !pOperator->cb_free )
        {
            rval = ETH_ERR_WRONG_PARAM;
            break;
        }

        g_eth_dev.eth_op                    = *pOperator;
        g_eth_dev.ctrl.iobase               = ETH_GMAC030_REG_BASE_ADDR;
        g_eth_dev.ctrl.cb_rx_handler        = g_cb_rx_handler;
        g_eth_dev.ctrl.cb_notify            = g_cb_notify;
        g_eth_dev.ctrl.phydev.pPhy_driver   = &g_phy_lan8720;
        g_eth_dev.ctrl.phydev.cb_phy_notify = _hal_eth_phy_notify;

        rval = _hal_eth_setup(&g_eth_dev);
        if( rval ) break;

        pPhy_drv = g_eth_dev.ctrl.phydev.pPhy_driver;
        pReg     = (ftgmac030_reg_t*)g_eth_dev.ctrl.iobase;

        // disable all IRQs of MAC
        pReg->ier   = 0ul;
        pReg->maccr = 0ul;

        rval = _hal_eth_mii_init(&g_eth_dev.ctrl);
        if( rval )  break;

        // phy probe
        if( !pOperator->is_loopback )
        {
            if( _hal_eth_scan_phy_addr(&g_eth_dev.ctrl) == 0x1FFFFFFF )
            {
                rval = ETH_ERR_NO_PHY;
                break;
            }
        }

        // s/w reset
        pReg->maccr = FTGMAC030_MACCR_SW_RST;
        while( (pReg->maccr & FTGMAC030_MACCR_SW_RST) ) {}

        // set ring buffer of TX/RX
        g_eth_dev.ctrl.aptc = FTGMAC030_APTC_DEFAULT;

        _hal_eth_set_ring_buf(&g_eth_dev);

        {   // set actual TX/RX FIFO size (depend on H/w, 2KB, 4KB or 8 KB)
            uint32_t    value = 0;

            value = (ETH_TX_FIFO_SIZE << 27);
            value |= (ETH_RX_FIFO_SIZE << 24);

            pReg->tfafcr &= ~(0x3F << 24);
            pReg->tfafcr |= (value);
        }

        // initialize IRQ
        hal_irq_enable(GMAC_IRQn);

    #if (ETH_HIGH_PRI_TXQ_EN)
        pReg->hptxdesc_addr = (unsigned int)g_eth_dev.ctrl.txq_hp.tx.txdes_top;
    #endif
        // set the normal and high priority transmit ring address register
        pReg->nptxdesc_addr = (unsigned int)g_eth_dev.ctrl.txq_np.tx.txdes_top;

        // set the receive ring address register
        pReg->rxdesc_addr = (uint32_t)g_eth_dev.ctrl.rxq.rx.rxdes_top;
        pReg->rbsr        = ETH_RX_PACKET_SIZE;

        // set interrupt enable register
        #if 0
        pReg->ier = FTGMAC030_INT_DEFAULT;
        #else
        pReg->ier = FTGMAC030_INT_ALL;
        #endif

        // set mac address
        pReg->mac_ladr = (uint32_t)(*(uint32_t *)g_eth_dev.macaddr.mac_addr) & 0xFFFFFFFF;
        pReg->mac_madr = (uint32_t)(*(uint32_t *)(g_eth_dev.macaddr.mac_addr + 4)) & 0xFFFF;

        // set interrupt timer control
        pReg->txitc = FTGMAC030_TXITC_DEFAULT;
        pReg->rxitc = FTGMAC030_RXITC_DEFAULT;

        // set auto polling timer control
        pReg->aptc = g_eth_dev.ctrl.aptc;

        // set DMA burst
        pReg->dblac = FTGMAC030_DBLAC_DEFAULT |
                      FTGMAC030_DBLAC_RXDES_SIZE((sizeof(ftgmac030_rxdesc_t) >> 3)) |
                      FTGMAC030_DBLAC_TXDES_SIZE((sizeof(ftgmac030_txdesc_t) >> 3));

        // Zero out the Multicast HASH table
        pReg->maht0 = 0;
        pReg->maht1 = 0;

        #if 0
        /**
         *  Set PTP increment value:
         *      Ask HW for source clock frequency.
         *      It is 50 MHz at time FPGA verified, so period is 20 ns.
         *      It is 100 MHz at time FPGA verified, so period is 10 ns.
         */
        pReg->ptp_ns_period  = 20; // 100 MHz
        pReg->ptp_nns_period = 0;
        #endif

        {   // set MAC control
            unsigned int    maccr = FTGMAC030_MACCR_DEFAULT;

            #if 0
            maccr &= ~(FTGMAC030_MACCR_ALLADDR | \
                       FTGMAC030_MACCR_MULTIPKT | \
                       FTGMAC030_MACCR_BROADPKT | \
                       FTGMAC030_MACCR_MULTIPKT | \
                       FTGMAC030_MACCR_HT_EN | \
                       FTGMAC030_MACCR_DROP_CRC_ERR | \
                       FTGMAC030_MACCR_JUMBO_LF);

            maccr |= (FTGMAC030_MACCR_RXDMA | \
                      FTGMAC030_MACCR_RXMAC | \
                      FTGMAC030_MACCR_BROADPKT | \
                      FTGMAC030_MACCR_REMOVE_VLAN | \
                      FTGMAC030_MACCR_DROP_CRC_ERR | \
                      FTGMAC030_MACCR_DIS_IPV6_PKTREC | \
                      FTGMAC030_MACCR_ALLADDR);
            #endif

            maccr |= (FTGMAC030_MACCR_BROADPKT | FTGMAC030_MACCR_ALLADDR);

            if( pOperator->is_loopback )
            {
                cb_eth_notify_t     cb_notify = g_eth_dev.ctrl.cb_notify;

                if( cb_notify )
                    cb_notify(ETH_STATE_LOOPBACK_MODE);

                maccr |= FTGMAC030_MACCR_LOOPBACK;
                pReg->maccr = maccr;
                break;
            }

            pReg->maccr = maccr;
        }

        phy_init(&g_eth_dev.ctrl.phydev);

        #if 1
        // check phy message
        pPhy_drv->show_info(&g_eth_dev.ctrl.phydev);
        // set interrupt of phy
        pPhy_drv->set_irq(&g_eth_dev.ctrl.phydev);
        pPhy_drv->clear_irq(&g_eth_dev.ctrl.phydev);
        pPhy_drv->show_link_status(&g_eth_dev.ctrl.phydev);
        #endif

        #if 1
        {   // Broadcast and Multicast Receiving Control
            /**
             * if receive broadcast larger than BMRCR[4:0] within
             * BMRCR[23:16] * BMRCR[24] ms/us depends on current
             * link speed, h/w discards the broadcast packets for
             * this period time and enable receive again.
             *
             * Actually, I don't have any idea or experience what
             * number should be configured. Below code just a guideline
             * how to program this register. Please adjust by yourself
             * according to your real application situations.
             *
             * I am assumed to be 0.5 s and receive 1024 broadcast
             * packets and  BMRCR[24] is one.
             */
            unsigned int    bmrcr = (0x1 << 24);

            if( g_eth_dev.ctrl.phydev.speed == PHY_SPEED_10 )
                bmrcr |= (((500000000 / 409600) & 0xFF) << 16);
            else if( g_eth_dev.ctrl.phydev.speed == PHY_SPEED_100 )
                bmrcr |= (((500000000 / 40960) & 0xFF) << 16);

            // unit is 256 packets
            bmrcr |= 0x4;

            pReg->bmrcr = bmrcr;
        }
        #endif

        #if 1
        {   // writes the ITR value into the RX Interrupt Timer Control Register(RXITC)
            /**
             * itr is expected number of interrupt happens per second.
             *
             * FTGMAC030 requires to set RX cycle time.
             *
             * When RXITC.RXINT_TIME_SEL set, the RX cycle times are:
             * 1000 Mbps mode => 16.384 µs
             * 100 Mbps mode => 81.92 µs
             * 10 Mbps mode => 819.2 µs
             *
             * See FTGMAC030 datasheet register offset 0x34.
             */
            unsigned int    rxitc = FTGMAC030_RXITC_TIME_SEL;
            unsigned long   value = 0ul;

            if( g_eth_dev.ctrl.phydev.speed == PHY_SPEED_10 )
            {
                value = 1000000000ul / (20000ul * 819200ul);
            }
            else if( g_eth_dev.ctrl.phydev.speed == PHY_SPEED_100 )
            {
                value = 1000000000ul / (20000ul * 81920ul);
            }

            rxitc |= FTGMAC030_RXITC_CYL(value);

            pReg->rxitc = rxitc;
        }
        #endif

    } while(0);
    return rval;
}

eth_err_t
hal_eth_deinit(void)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        hal_eth_op_t    *pOp = &g_eth_dev.eth_op;

        hal_irq_disable(GMAC_IRQn);

        {   // s/w reset
            ftgmac030_reg_t     *pReg = (ftgmac030_reg_t*)g_eth_dev.ctrl.iobase;

            pReg->ier   = 0ul;
            pReg->maccr = FTGMAC030_MACCR_SW_RST;
            while( (pReg->maccr & FTGMAC030_MACCR_SW_RST) ) {}
        }

        if( pOp->cb_free )
        {
            if( g_eth_dev.pTxdesc_np )
                pOp->cb_free(g_eth_dev.pTxdesc_np);

        #if (ETH_HIGH_PRI_TXQ_EN)
            if( g_eth_dev.pTxdesc_hp )
                pOp->cb_free(g_eth_dev.pTxdesc_hp);
        #endif

            if( g_eth_dev.pRxdesc )
                pOp->cb_free(g_eth_dev.pRxdesc);

            if( g_eth_dev.pRx_packets )
                pOp->cb_free(g_eth_dev.pRx_packets);
        }

        memset(&g_eth_dev, 0x0, sizeof(g_eth_dev));
    } while(0);
    return rval;
}


eth_err_t
hal_eth_send(
    uint8_t     *pData,
    uint32_t    data_len)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        // single description
        ftgmac030_queue_t       *pTxq = (ETH_HIGH_PRI_TXQ_EN) ? &g_eth_dev.ctrl.txq_hp : &g_eth_dev.ctrl.txq_np;
        ftgmac030_txdesc_t      *pTxdes = 0;

        if( data_len > ((0x1ul << (ETH_TX_FIFO_SIZE + 1)) << 10) )
        {
            rval = ETH_ERR_OVER_FIFO_SIZE;
            break;
        }

        if( (uintptr_t)pData & 0xF )
        {
            rval = ETH_ERR_NOT_ALIGNMENT;
            break;
        }

        pTxdes = pTxq->tx.txdes_cur;

        // in this function, TX is always completed by one descriptor
        if( pTxdes->txdes0 & FTGMAC030_TXDES0_TXDMA_OWN )
        {
            rval = ETH_ERR_TX_BUSY;
            break;
        }

        // the setting order of txdes CAN NOT be changed
        pTxdes->txdes3 = (unsigned int)pData;
        pTxdes->txdes1 = FTGMAC030_TXDES1_DEFAULT;

        #if 0
        if( pData[12] == 0x08 && pData[13] == 0x00 )
        {
            pTxdes->txdes1 |= FTGMAC030_TXDES1_IPCS_EN;
            if( pData[23] == 0x11 )
                pTxdes->txdes1 |= FTGMAC030_TXDES1_UDPCS_EN;
            else if( pData[23] == 0x06 )
                pTxdes->txdes1 |= FTGMAC030_TXDES1_TCPCS_EN;
        }
        #else
        {
            /**
             *  H/w checksum enable
             *  ps. User MUST promise this packet type (ip/udp/tcp header exist or not)
             *      and enable the H/w checksum by those headers (or GMAC crash)
             */
            eth_pkt_hdr_t   *pEth_hdr = (eth_pkt_hdr_t*)pData;
            if( pEth_hdr->eth_hdr.proto == 0x0008 )
            {
                pTxdes->txdes1 |= FTGMAC030_TXDES1_IPCS_EN;
                if( pEth_hdr->ipv4_hdr.protocol == 0x11 )
                    pTxdes->txdes1 |= FTGMAC030_TXDES1_UDPCS_EN;
                else if( pEth_hdr->ipv4_hdr.protocol == 0x06 )
                    pTxdes->txdes1 |= FTGMAC030_TXDES1_TCPCS_EN;
            }
        }
        #endif

        pTxdes->txdes0 &= FTGMAC030_TXDES0_EDOTR;
        pTxdes->txdes0 |= FTGMAC030_TXDES0_BUF_SIZE(data_len) |
                          FTGMAC030_TXDES0_FTS |
                          FTGMAC030_TXDES0_LTS |
                          FTGMAC030_TXDES0_TXDMA_OWN;

        pTxq->tx.txdes_cur = (ftgmac030_txdesc_t*)pTxdes->txdes2;

        // trigger DMA
        pTxq->kickoff(&g_eth_dev.ctrl);

        g_eth_dev.ctrl.is_tx_end = 0ul;

        // wait TX end
        for(uint32_t timeout = 0; timeout < ETH_SEND_TIMEOUT; timeout++)
        {
            if( g_eth_dev.ctrl.is_tx_end )
                break;
        }

        rval = (g_eth_dev.ctrl.is_tx_end) ? ETH_ERR_OK : ETH_ERR_SEND_FAIL;
    } while(0);
    return rval;
}


eth_err_t
hal_eth_recv(
    uint8_t     *pData,
    uint32_t    *pData_len)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
    } while(0);
    return rval;
}

eth_err_t
hal_eth_register_recv_handler(
    cb_eth_rx_handler_t     cb_rx_handler)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        if( !cb_rx_handler )
        {
            rval = ETH_ERR_WRONG_PARAM;
            break;
        }

        g_cb_rx_handler              = cb_rx_handler;
        g_eth_dev.ctrl.cb_rx_handler = g_cb_rx_handler;
    } while(0);
    return rval;
}

eth_err_t
hal_eth_register_notify(
    cb_eth_notify_t     cb_notify)
{
    eth_err_t       rval = ETH_ERR_OK;
    do {
        if( !cb_notify )
        {
            rval = ETH_ERR_WRONG_PARAM;
            break;
        }

        g_cb_notify              = cb_notify;
        g_eth_dev.ctrl.cb_notify = g_cb_notify;
    } while(0);
    return rval;
}
