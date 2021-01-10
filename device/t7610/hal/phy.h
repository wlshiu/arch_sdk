/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file phy.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */

#ifndef __phy_H_wWOIdxy7_lrNG_H8rX_sXtn_u3OxSXdxhjTi__
#define __phy_H_wWOIdxy7_lrNG_H8rX_sXtn_u3OxSXdxhjTi__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  PHY register offset definition
 */
#define PHY_MAX_ADDR                32

#define MII_REG_ID1                 2
#define MII_REG_ID2                 3

#define PHY_SPEED_10                10
#define PHY_SPEED_100               100
#define PHY_SPEED_UNKNOWN           (-1)

/* Duplex, half or full. */
#define PHY_DUPLEX_HALF             0x00
#define PHY_DUPLEX_FULL             0x01
#define PHY_DUPLEX_UNKNOWN          (-1)

#define PHY_10M_HALF                0x1
#define PHY_10M_FULL                0x2
#define PHY_100M_HALF               0x3
#define PHY_100M_FULL               0x4

/**
 *  the event type from PHY
 */
typedef enum phy_event
{
    PHY_EVENT_NONE      = 0,
    PHY_EVENT_STATUS,

} phy_event_t;
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
struct phy_dev;

typedef struct mii_dev
{
	void        *pPriv;

	uint16_t    (*read)(struct mii_dev *bus, int phy_addr, int regnum);
	int         (*write)(struct mii_dev *bus, int phy_addr, int regnum, uint16_t value);
    int         (*reset)(struct mii_dev *bus);

	uint32_t    phy_mask;
} mii_dev_t;

typedef struct phy_driver
{
	int         (*init)(struct phy_dev *phydev);
	void        (*auto_neo)(struct phy_dev *phydev); // auto negotiation
	void        (*show_info)(struct phy_dev *phydev);
	void        (*clear_irq)(struct phy_dev *phydev);
	void        (*set_irq)(struct phy_dev *phydev);
	int         (*change_duplex_speed)(struct phy_dev *phydev, int duplex, int speed);
	uint32_t    (*is_link_up)(struct phy_dev *phydev);
	void        (*show_link_status)(struct phy_dev *phydev);

} phy_driver_t;

typedef struct phy_info
{
    union {
        struct {
            uint32_t    speed_duplex;
        } status;
    };
} phy_info_t;

typedef void (*cb_phy_notify_t)(phy_event_t event, phy_info_t *pInfo);

typedef struct phy_dev
{
    phy_driver_t    *pPhy_driver;
    mii_dev_t       mii_dev;

    void            *pPriv;

    uint32_t        phy_id;

    cb_phy_notify_t cb_phy_notify;

    uint32_t        phy_addr    : 6;
    uint32_t        is_auto_neg : 1;
    uint32_t        duplex      : 3;
    uint32_t        speed       : 10;

} phy_dev_t;
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
    phy_dev_t   *phydev);


void
phy_auto_neo(
    phy_dev_t   *phydev);


void
phy_info(
    phy_dev_t   *phydev);


void
phy_clear_irq(
    phy_dev_t   *phydev);


void
phy_set_irq(
    phy_dev_t   *phydev);


int
phy_change_duplex_speed(
    phy_dev_t   *phydev,
    int         duplex,
    int         speed);


uint32_t
phy_is_link_up(
    phy_dev_t   *phydev);


void
phy_show_link_status(
    phy_dev_t   *phydev);


#ifdef __cplusplus
}
#endif

#endif
