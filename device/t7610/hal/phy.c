/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file phy.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */


#include "phy.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

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

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
phy_init(
    phy_dev_t   *phydev)
{
    int     rval = 0;
    do {
        phy_driver_t    *pDrv = phydev->pPhy_driver;
        if( !pDrv ) break;

        rval = pDrv->init(phydev);
    } while(0);
    return rval;
}

void
phy_auto_neo(
    phy_dev_t   *phydev)
{
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        pDrv->auto_neo(phydev);
    } while(0);
    return;
}

void
phy_info(
    phy_dev_t   *phydev)
{
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        pDrv->show_info(phydev);
    } while(0);
    return;
}

void
phy_clear_irq(
    phy_dev_t   *phydev)
{
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        pDrv->clear_irq(phydev);
    } while(0);
    return;
}

void
phy_set_irq(
    phy_dev_t   *phydev)
{
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        pDrv->set_irq(phydev);
    } while(0);
    return;
}

int
phy_change_duplex_speed(
    phy_dev_t   *phydev,
    int         duplex,
    int         speed)
{
    int             rval = -1;
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        rval = pDrv->change_duplex_speed(phydev, duplex, speed);
    } while(0);
	return rval;
}

uint32_t
phy_is_link_up(
    phy_dev_t   *phydev)
{
    uint32_t        rval = 0;
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        rval = pDrv->is_link_up(phydev);
    } while(0);
	return rval ;
}

void
phy_show_link_status(
    phy_dev_t   *phydev)
{
    phy_driver_t    *pDrv = phydev->pPhy_driver;
    do {
        if( !pDrv ) break;

        pDrv->show_link_status(phydev);
    } while(0);
    return;
}
