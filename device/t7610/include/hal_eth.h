/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file hal_eth.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */

#ifndef __hal_eth_H_wI9W9muu_lPpS_HH0z_su7Y_uzLl6xaIg9PB__
#define __hal_eth_H_wI9W9muu_lPpS_HH0z_su7Y_uzLl6xaIg9PB__

#ifdef __cplusplus
extern "C" {
#endif


#include "device_hal.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define ETH_RXQ_ENTRIES                     8
#define ETH_TXQ_NP_ENTRIES                  4  // normal priority TX queue (it MUST be more than 2)
#define ETH_TXQ_HP_ENTRIES                  4  // high priority TX queue (it MUST be more than 2)
#define ETH_RX_PACKET_SIZE                  (2 << 10) // TX/RX FIFO is 2KB and ether-net packet size = 1536


typedef enum eth_err
{
    ETH_ERR_OK             = 0,
    ETH_ERR_NULL_POINTER   = (ETH_ERRNO_BASE | HAL_ERRNO_NULL_POINTER),
    ETH_ERR_NO_INSTANCE    = (ETH_ERRNO_BASE | HAL_ERRNO_NO_INSTANCE),
    ETH_ERR_WRONG_PARAM    = (ETH_ERRNO_BASE | HAL_ERRNO_WRONG_PARAM),
    ETH_ERR_NOT_SUPPORT    = (ETH_ERRNO_BASE | HAL_ERRNO_NOT_SUPPORT),
    ETH_ERR_NOT_ALIGNMENT,
    ETH_ERR_NO_PHY,
    ETH_ERR_OVER_FIFO_SIZE,
    ETH_ERR_TX_BUSY,
    ETH_ERR_SEND_FAIL,

} eth_err_t;

typedef enum eth_mem
{
    ETH_MEM_NONE    = 0,
    ETH_MEM_CACHE_NP_TX,
    ETH_MEM_CACHE_HP_TX,
    ETH_MEM_CACHE_RX,
    ETH_MEM_PACKET_RX,

} eth_mem_t;

typedef enum eth_state
{
    ETH_STATE_LOOPBACK_MODE          = 9,
    ETH_STATE_PHY_STATUS_UNKNOWN     = 8,
    ETH_STATE_10M_HALF               = 7,
    ETH_STATE_10M_FULL               = 6,
    ETH_STATE_100M_HALF              = 5,
    ETH_STATE_100M_FULL              = 4,
    ETH_STATE_TX2FIFO                = 3,
    ETH_STATE_TX_END                 = 2,
    ETH_STATE_LINK_STATE_CHANGE      = 1,
    ETH_STATE_NOTHING                = 0,
    ETH_STATE_RX_FIFO_FULL           = -3,
    ETH_STATE_RX_ODD_NIBBLES         = -4,
    ETH_STATE_RX_CRC_FAIL            = -5,
    ETH_STATE_RX_ERROR               = -6,
    ETH_STATE_RX_FRAME_TOO_LONG      = -7,
    ETH_STATE_RX_IP_CHECKSUM_FAIL    = -8,
    ETH_STATE_RX_UDP_CHECKSUM_FAIL   = -9,
    ETH_STATE_RX_TCP_CHECKSUM_FAIL   = -10,
} eth_state_t;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct hal_eth_macaddr
{
    uint8_t     mac_addr[6];
} hal_eth_macaddr_t;


/**
 *  ether-net operator
 */
typedef struct hal_eth_op
{
    uint32_t    is_loopback;

    eth_err_t   (*cb_set_macaddr)(hal_eth_macaddr_t *pAddr);    /*!< user set MAC address */

    uint32_t*   (*cb_malloc)(eth_mem_t type, int length);       /*!< request memory for procedure */
    void        (*cb_free)(void *p);                            /*!< free memory */

    eth_err_t   (*cb_phy_reset)(void);                          /*!< reset ether-net PHY module */
} hal_eth_op_t;

/**
 *  @brief  cb_eth_rx_handler_t()
 *              This callback MUST process ASAP.
 *              ps. This handler is as ISR.
 *  @param [in] in_packet       pointer of incoming packet. in_packet is 0 when receive fail
 *  @param [in] length          the length of packet
 *  @return
 *      none
 */
typedef void (*cb_eth_rx_handler_t)(uint8_t *in_packet, int length);

/**
 *  @brief  cb_eth_notify_t()
 *              notify the MAC state to protocol layer
 *
 *  @param [in] state       reference enum eth_state
 *  @return
 *      none
 */
typedef void (*cb_eth_notify_t)(eth_state_t state);

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
/**
 *  @brief  hal_eth_init (Non thread self)
 *
 *  @param [in] pOperator       the operators for procedure. reference struct hal_eth_op
 *  @return
 *      error code, reference enum eth_err
 */
eth_err_t
hal_eth_init(
    hal_eth_op_t    *pOperator);


/**
 *  @brief  hal_eth_deinit (Non thread self)
 *
 *  @return
 *      error code, reference enum eth_err
 */
eth_err_t
hal_eth_deinit(void);


/**
 *  @brief  hal_eth_send (Non thread self)
 *
 *  @param [in] pData           the pointer of raw data
 *  @param [in] data_len        the length of raw data
 *  @return
 *      error code, reference enum eth_err
 */
eth_err_t
hal_eth_send(
    uint8_t     *pData,
    uint32_t    data_len);


/**
 *  @brief  hal_eth_register_recv_handler (Non thread self)
 *
 *  @param [in] cb_rx_handler   the callback of receiving from GMAC
 *  @return
 *      error code, reference enum eth_err
 */
eth_err_t
hal_eth_register_recv_handler(
    cb_eth_rx_handler_t     cb_rx_handler);


/**
 *  @brief  hal_eth_register_notify (Non thread self)
 *
 *  @param [in] cb_notify       user notification callback
 *  @return
 *      error code, reference enum eth_err
 */
eth_err_t
hal_eth_register_notify(
    cb_eth_notify_t     cb_notify);


#ifdef __cplusplus
}
#endif

#endif
