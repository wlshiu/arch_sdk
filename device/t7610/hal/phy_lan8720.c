/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file phy_lan8720.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */


#include "phy.h"
#include "ftgmac030.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define LAN8720_TIMEOUT             0x4FFFFul
#define LAN8720_PHY_ADDR            0

// LAN8720 registers
#define LAN8720_PHY_REG_BMCR        0x00
#define LAN8720_PHY_REG_BMSR        0x01
#define LAN8720_PHY_REG_PHYIDR1     0x02
#define LAN8720_PHY_REG_PHYIDR2     0x03
#define LAN8720_PHY_REG_ANAR        0x04
#define LAN8720_PHY_REG_ANLPAR      0x05
#define LAN8720_PHY_REG_ANER        0x06
#define LAN8720_PHY_REG_SRR         0x10
#define LAN8720_PHY_REG_MCSR        0x11
#define LAN8720_PHY_REG_SMR         0x12
#define LAN8720_PHY_REG_SECR        0x1A
#define LAN8720_PHY_REG_SCSIR       0x1B
#define LAN8720_PHY_REG_SITCR       0x1C
#define LAN8720_PHY_REG_ISR         0x1D
#define LAN8720_PHY_REG_IMR         0x1E
#define LAN8720_PHY_REG_PSCSR       0x1F

// BMCR register (0x0, LAN8720_PHY_REG_BMCR)
#define BMCR_RESET                  (1 << 15)     // BMCR_RESET
#define BMCR_LOOPBACK               (1 << 14)     // BMCR_LOOPBACK
#define BMCR_SPEED_SEL              (1 << 13)     // BMCR_SPEED100
#define BMCR_AN_EN                  (1 << 12)     // BMCR_ANENABLE
#define BMCR_POWER_DOWN             (1 << 11)     // BMCR_PDOWN
#define BMCR_ISOLATE                (1 << 10)     // BMCR_ISOLATE
#define BMCR_RESTART_AN             (1 << 9)      // BMCR_ANRESTART
#define BMCR_DUPLEX_MODE            (1 << 8)      // BMCR_FULLDPLX
#define BMCR_COL_TEST               (1 << 7)      // BMCR_CTST

// BMSR register (0x1, LAN8720_PHY_REG_BMSR)
#define BMSR_100BT4                 (1 << 15)     // BMSR_100BASE4
#define BMSR_100BTX_FD              (1 << 14)     // BMSR_100FULL
#define BMSR_100BTX                 (1 << 13)     // BMSR_100HALF
#define BMSR_10BT_FD                (1 << 12)     // BMSR_10FULL
#define BMSR_10BT                   (1 << 11)     // BMSR_10HALF
#define BMSR_100BT2_FD              (1 << 10)     // BMSR_100FULL2
#define BMSR_100BT2                 (1 << 9)      // BMSR_100HALF2
#define BMSR_EXT_STAT_EN            (1 << 8)      // BMSR_ESTATEN
#define BMSR_AN_COMPLETE            (1 << 5)      // BMSR_ANEGCOMPLETE
#define BMSR_REMOTE_FAULT           (1 << 4)      // BMSR_RFAULT
#define BMSR_AN_ABLE                (1 << 3)      // BMSR_ANEGCAPABLE
#define BMSR_LINK_STATUS            (1 << 2)      // BMSR_LSTATUS
#define BMSR_JABBER_DETECT          (1 << 1)      // BMSR_JCD
#define BMSR_EXTENDED_CAP           (1 << 0)      // BMSR_ERCAP

// ANAR register (0x4, LAN8720_PHY_REG_ANAR)
#define ANAR_NP                     (1 << 15)     // ADVERTISE_NPAGE
#define ANAR_RF                     (1 << 13)     // ADVERTISE_RFAULT
#define ANAR_PAUSE1                 (1 << 11)     // ADVERTISE_PAUSE_ASYM
#define ANAR_PAUSE0                 (1 << 10)     // ADVERTISE_PAUSE_CAP
#define ANAR_100BT4                 (1 << 9)      // ADVERTISE_100BASE4
#define ANAR_100BTX_FD              (1 << 8)      // ADVERTISE_100FULL
#define ANAR_100BTX                 (1 << 7)      // ADVERTISE_100HALF
#define ANAR_10BT_FD                (1 << 6)      // ADVERTISE_10FULL
#define ANAR_1000BX                 (1 << 6)      // ADVERTISE_1000XHALF
#define ANAR_10BT                   (1 << 5)      // ADVERTISE_10HALF
#define ANAR_1000BX_FD              (1 << 5)      // ADVERTISE_1000XFULL
#define ANAR_SELECTOR4              (1 << 4)      //
#define ANAR_SELECTOR3              (1 << 3)      //
#define ANAR_SELECTOR2              (1 << 2)      //
#define ANAR_SELECTOR1              (1 << 1)      //
#define ANAR_SELECTOR0              (1 << 0)      // ADVERTISE_CSMA

#define ADVERTISE_FULL              (ANAR_100BTX_FD | ANAR_10BT_FD | ANAR_SELECTOR0)
#define ADVERTISE_ALL               (ANAR_10BT | ANAR_10BT_FD | ANAR_100BTX | ANAR_100BTX_FD)


// ANLPAR register (0x5, LAN8720_PHY_REG_ANLPAR)
#define ANLPAR_NP                   (1 << 15)      // LPA_NPAGE
#define ANLPAR_ACK                  (1 << 14)      // LPA_LPACK
#define ANLPAR_RF                   (1 << 13)      // LPA_RFAULT
#define ANLPAR_PAUSE                (1 << 10)      // LPA_PAUSE_CAP
#define ANLPAR_100BT4               (1 << 9)       // LPA_100BASE4
#define ANLPAR_100BTX_FD            (1 << 8)       // LPA_100FULL
#define ANLPAR_100BTX               (1 << 7)       // LPA_100HALF
#define ANLPAR_10BT_FD              (1 << 6)       // LPA_10FULL
#define ANLPAR_10BT                 (1 << 5)       // LPA_10HALF
#define ANLPAR_SELECTOR4            (1 << 4)       //
#define ANLPAR_SELECTOR3            (1 << 3)       //
#define ANLPAR_SELECTOR2            (1 << 2)       //
#define ANLPAR_SELECTOR1            (1 << 1)       //
#define ANLPAR_SELECTOR0            (1 << 0)       //

// ANER register (0x6, LAN8720_PHY_REG_ANER)
#define ANER_PDF                    (1 << 4)
#define ANER_LP_NP_ABLE             (1 << 3)
#define ANER_NP_ABLE                (1 << 2)
#define ANER_PAGE_RX                (1 << 1)
#define ANER_LP_AN_ABLE             (1 << 0)

// SRR register (LAN8720_PHY_REG_SRR)
#define SRR_SILICON_REVISON3        (1 << 9)
#define SRR_SILICON_REVISON2        (1 << 8)
#define SRR_SILICON_REVISON1        (1 << 7)
#define SRR_SILICON_REVISON0        (1 << 6)

// MCSR register (0x11, LAN8720_PHY_REG_MCSR)
#define MCSR_EDPWRDOWN              (1 << 13)
#define MCSR_LOWSQEN                (1 << 11)
#define MCSR_MDPREBP                (1 << 10)
#define MCSR_FARLOOPBACK            (1 << 9)
#define MCSR_ALTINT                 (1 << 6)
#define MCSR_PHYADBP                (1 << 3)
#define MCSR_FORCE_GOOD_LINK_STATUS (1 << 2)
#define MCSR_ENERGYON               (1 << 1)

// SMR register (0x12, LAN8720_PHY_REG_SMR)
#define SMR_MODE2                   (1 << 7)
#define SMR_MODE1                   (1 << 6)
#define SMR_MODE0                   (1 << 5)
#define SMR_PHYAD4                  (1 << 4)
#define SMR_PHYAD3                  (1 << 3)
#define SMR_PHYAD2                  (1 << 2)
#define SMR_PHYAD1                  (1 << 1)
#define SMR_PHYAD0                  (1 << 0)

// SCSIR register (LAN8720_PHY_REG_SCSIR)
#define SCSIR_AMDIXCTRL             (1 << 15)
#define SCSIR_CH_SELECT             (1 << 13)
#define SCSIR_SQEOFF                (1 << 11)
#define SCSIR_XPOL                  (1 << 4)

// ISR register (LAN8720_PHY_REG_ISR)
#define ISR_ENERGYON                (1 << 7)
#define ISR_AN_COMPLETE             (1 << 6)
#define ISR_REMOTE_FAULT            (1 << 5)
#define ISR_LINK_DOWN               (1 << 4)
#define ISR_AN_LP_ACK               (1 << 3)
#define ISR_PD_FAULT                (1 << 2)
#define ISR_AN_PAGE_RECEIVED        (1 << 1)

// IMR register (LAN8720_PHY_REG_IMR)
#define IMR_ENERGYON                (1 << 7)
#define IMR_AN_COMPLETE             (1 << 6)
#define IMR_REMOTE_FAULT            (1 << 5)
#define IMR_LINK_DOWN               (1 << 4)
#define IMR_AN_LP_ACK               (1 << 3)
#define IMR_PD_FAULT                (1 << 2)
#define IMR_AN_PAGE_RECEIVED        (1 << 1)

// PSCSR register (LAN8720_PHY_REG_PSCSR)
#define PSCSR_AUTODONE              (1 << 12)
#define PSCSR_GPO2                  (1 << 9)
#define PSCSR_GPO1                  (1 << 8)
#define PSCSR_GPO0                  (1 << 7)
#define PSCSR_ENABLE_4B5B           (1 << 6)
#define PSCSR_HCDSPEED2             (1 << 4)
#define PSCSR_HCDSPEED1             (1 << 3)
#define PSCSR_HCDSPEED0             (1 << 2)
#define PSCSR_SCRAMBLE_DISABLE      (1 << 0)

// Speed indication
#define PSCSR_HCDSPEED_MASK         (7 << 2)
#define PSCSR_HCDSPEED_10BT         (1 << 2)
#define PSCSR_HCDSPEED_100BTX       (2 << 2)
#define PSCSR_HCDSPEED_10BT_FD      (5 << 2)
#define PSCSR_HCDSPEED_100BTX_FD    (6 << 2)

// Indicates what features are supported by the interface.
#define SUPPORTED_10baseT_Half      (1 << 0)
#define SUPPORTED_10baseT_Full      (1 << 1)
#define SUPPORTED_100baseT_Half     (1 << 2)
#define SUPPORTED_100baseT_Full     (1 << 3)
#define SUPPORTED_1000baseT_Half    (1 << 4)
#define SUPPORTED_1000baseT_Full    (1 << 5)
#define SUPPORTED_Autoneg           (1 << 6)
#define SUPPORTED_TP                (1 << 7)
#define SUPPORTED_AUI               (1 << 8)
#define SUPPORTED_MII               (1 << 9)
#define SUPPORTED_FIBRE             (1 << 10)
#define SUPPORTED_BNC               (1 << 11)
#define SUPPORTED_10000baseT_Full   (1 << 12)
#define SUPPORTED_Pause             (1 << 13)
#define SUPPORTED_Asym_Pause        (1 << 14)
#define SUPPORTED_2500baseX_Full    (1 << 15)
#define SUPPORTED_Backplane         (1 << 16)
#define SUPPORTED_1000baseKX_Full   (1 << 17)
#define SUPPORTED_10000baseKX4_Full (1 << 18)
#define SUPPORTED_10000baseKR_Full  (1 << 19)
#define SUPPORTED_10000baseR_FEC    (1 << 20)
#define SUPPORTED_1000baseX_Half    (1 << 21)
#define SUPPORTED_1000baseX_Full    (1 << 22)

//Indicates what features are advertised by the interface.
#define ADVERTISED_10baseT_Half            SUPPORTED_10baseT_Half
#define ADVERTISED_10baseT_Full            SUPPORTED_10baseT_Full
#define ADVERTISED_100baseT_Half           SUPPORTED_100baseT_Half
#define ADVERTISED_100baseT_Full           SUPPORTED_100baseT_Full
#define ADVERTISED_1000baseT_Half          SUPPORTED_1000baseT_Half
#define ADVERTISED_1000baseT_Full          SUPPORTED_1000baseT_Full
#define ADVERTISED_Autoneg                 SUPPORTED_Autoneg
#define ADVERTISED_TP                      SUPPORTED_TP
#define ADVERTISED_AUI                     SUPPORTED_AUI
#define ADVERTISED_MII                     SUPPORTED_MII
#define ADVERTISED_FIBRE                   SUPPORTED_FIBRE
#define ADVERTISED_BNC                     SUPPORTED_BNC
#define ADVERTISED_10000baseT_Full         SUPPORTED_10000baseT_Full
#define ADVERTISED_Pause                   SUPPORTED_Pause
#define ADVERTISED_Asym_Pause              SUPPORTED_Asym_Pause
#define ADVERTISED_2500baseX_Full          SUPPORTED_2500baseX_Full
#define ADVERTISED_Backplane               SUPPORTED_Backplane
#define ADVERTISED_1000baseKX_Full         SUPPORTED_1000baseKX_Full
#define ADVERTISED_10000baseKX4_Full       SUPPORTED_10000baseKX4_Full
#define ADVERTISED_10000baseKR_Full        SUPPORTED_10000baseKR_Full
#define ADVERTISED_10000baseR_FEC          SUPPORTED_10000baseR_FEC
#define ADVERTISED_1000baseX_Half          SUPPORTED_1000baseX_Half
#define ADVERTISED_1000baseX_Full          SUPPORTED_1000baseX_Full


#define SMSC_PHY_ID      0x0007C0F1
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
static int
lan8720_init(
    phy_dev_t    *phydev)
{
    int         rval = 0;
    do {
        mii_dev_t   *bus = &phydev->mii_dev;

        phydev->is_auto_neg = 0;

        {
            uint32_t        timeout = 0;

            bus->write(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR, BMCR_RESET);
            while( (bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR) & BMCR_RESET) ) {}

            while( !(bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMSR) & BMSR_LINK_STATUS) &&
                    timeout++ < LAN8720_TIMEOUT );

            if( timeout == LAN8720_TIMEOUT )
            {
                rval = -1;
                break;
            }
        }

        {
            uint16_t    value = 0;
            value = bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR);

            // disable auto-negotiate and set to 10Mb
            value &= ~(BMCR_AN_EN | BMCR_SPEED_SEL);
            bus->write(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR, value);

            phydev->speed = (value & BMCR_SPEED_SEL) ? PHY_SPEED_100 : PHY_SPEED_10;
            phydev->duplex = (value & BMCR_DUPLEX_MODE) ? PHY_DUPLEX_FULL : PHY_DUPLEX_HALF;
        }

        {
            unsigned int        maccr = ((ftgmac030_reg_t*)phydev->pPriv)->maccr;

            maccr &= ~(0x2ul << 24);
            if( phydev->speed == PHY_SPEED_10 )
                maccr |= FTGMAC030_MACCR_SPEED_10;
            else if( phydev->speed == PHY_SPEED_100 )
                maccr |= FTGMAC030_MACCR_SPEED_100;

            if( phydev->duplex == PHY_DUPLEX_HALF )
                maccr &= ~FTGMAC030_MACCR_FULLDUP;
            else if( phydev->duplex == PHY_DUPLEX_FULL )
                maccr |= FTGMAC030_MACCR_FULLDUP;

            ((ftgmac030_reg_t*)phydev->pPriv)->maccr = maccr;
        }

    } while(0);
    return rval;
}

__attribute__((unused)) static void
lan8720_auto_neo(
    phy_dev_t    *phydev)
{
    do {
        mii_dev_t   *bus = &phydev->mii_dev;
        uint16_t    ctrl_value = 0;
        uint32_t    timeout = LAN8720_TIMEOUT;

        ctrl_value = bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR);
        ctrl_value |= (BMCR_AN_EN | BMCR_RESTART_AN);

        bus->write(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR, ctrl_value);

        while( !(bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_PSCSR) & PSCSR_AUTODONE) &&
               timeout++ < LAN8720_TIMEOUT )
        {
            if( timeout == LAN8720_TIMEOUT )
            {
                break;
            }
        }
    } while(0);
    return;
}

static void
lan8720_show_info(
    phy_dev_t    *phydev)
{
    do {
        if( phydev->phy_id != SMSC_PHY_ID )
        {
            __asm("nop");
        }
    } while(0);
    return;
}

static void
lan8720_clear_irq(
    phy_dev_t    *phydev)
{
    do {
        mii_dev_t   *bus = &phydev->mii_dev;
        bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_ISR);
    } while(0);
    return;
}

static void
lan8720_set_irq(
    phy_dev_t    *phydev)
{
    do {
        mii_dev_t   *bus = &phydev->mii_dev;
        uint16_t    value = 0;

        value = IMR_AN_COMPLETE | IMR_LINK_DOWN | IMR_ENERGYON;
        bus->write(bus, phydev->phy_addr, LAN8720_PHY_REG_IMR, value);
    } while(0);
    return;
}

static int
lan8720_change_duplex_speed(
    phy_dev_t   *phydev,
    int         duplex,
    int         speed)
{
    int     rval = 0;
    do {
        if( phydev->is_auto_neg )
            break;

        {
            mii_dev_t       *bus = &phydev->mii_dev;
            uint16_t        value = 0;
            unsigned int    maccr = ((ftgmac030_reg_t*)phydev->pPriv)->maccr;

            value = bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR);

            value &= ~BMCR_AN_EN;
            maccr &= ~(0x2ul << 24);

            if( speed == PHY_SPEED_10 )
            {
                value &= ~BMCR_SPEED_SEL;
                maccr |= FTGMAC030_MACCR_SPEED_10;
            }
            else if( speed == PHY_SPEED_100 )
            {
                value |= BMCR_SPEED_SEL;
                maccr |= FTGMAC030_MACCR_SPEED_100;
            }

            if( duplex == PHY_DUPLEX_HALF )
            {
                value &= ~BMCR_DUPLEX_MODE;
                maccr &= ~FTGMAC030_MACCR_FULLDUP;
            }
            else if( duplex == PHY_DUPLEX_FULL )
            {
                value |= BMCR_DUPLEX_MODE;
                maccr |= FTGMAC030_MACCR_FULLDUP;
            }

            bus->write(bus, phydev->phy_addr, LAN8720_PHY_REG_BMCR, value);

            ((ftgmac030_reg_t*)phydev->pPriv)->maccr = maccr;
        }
    } while(0);

    return rval;
}

static uint32_t
lan8720_is_link_up(
    phy_dev_t    *phydev)
{
    mii_dev_t   *bus = &phydev->mii_dev;
    uint16_t    value = 0;

    value = bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_BMSR);
    return (value & BMSR_LINK_STATUS);
}

static void
lan8720_show_link_status(
    phy_dev_t    *phydev)
{
    mii_dev_t   *bus = &phydev->mii_dev;
    uint16_t    value = 0;
    uint32_t    status = 0;

    value = bus->read(bus, phydev->phy_addr, LAN8720_PHY_REG_PSCSR);
    if( (value & PSCSR_HCDSPEED_MASK) == PSCSR_HCDSPEED_10BT ) // 10BASE-T half-duplex
    {
        status = PHY_10M_HALF;

        phydev->speed  = PHY_SPEED_10;
        phydev->duplex = PHY_DUPLEX_HALF;
    }
    else if( (value & PSCSR_HCDSPEED_MASK) == PSCSR_HCDSPEED_10BT_FD ) // 10BASE-T full-duplex
    {
        status = PHY_10M_FULL;

        phydev->speed  = PHY_SPEED_10;
        phydev->duplex = PHY_DUPLEX_FULL;
    }
    else if( (value & PSCSR_HCDSPEED_MASK) == PSCSR_HCDSPEED_100BTX ) // 100BASE-TX half-duplex
    {
        status = PHY_100M_HALF;

        phydev->speed  = PHY_SPEED_100;
        phydev->duplex = PHY_DUPLEX_HALF;
    }
    else if( (value & PSCSR_HCDSPEED_MASK) == PSCSR_HCDSPEED_100BTX_FD ) // 100BASE-TX full-duplex
    {
        status = PHY_100M_FULL;

        phydev->speed  = PHY_SPEED_100;
        phydev->duplex = PHY_DUPLEX_FULL;
    }
    else
    {
        phydev->speed  = PHY_SPEED_UNKNOWN;
        phydev->duplex = PHY_DUPLEX_UNKNOWN;
    }

    if( phydev->cb_phy_notify )
    {
        phy_info_t      info = {0};

        info.status.speed_duplex = status;
        phydev->cb_phy_notify(PHY_EVENT_STATUS, &info);
    }
    return;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
phy_driver_t    g_phy_lan8720 =
{
    .init                 = lan8720_init,
    .auto_neo             = 0,
    .show_info            = lan8720_show_info,
    .clear_irq            = lan8720_clear_irq,
    .set_irq              = lan8720_set_irq,
    .change_duplex_speed  = lan8720_change_duplex_speed,
    .is_link_up           = lan8720_is_link_up,
    .show_link_status     = lan8720_show_link_status,
};

