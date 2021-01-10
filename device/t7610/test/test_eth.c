/**
 * Copyright (c) 2020 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file test_eth.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2020/06/03
 * @license
 * @description
 */


#include <string.h>
#include "device_hal.h"
#include "common.h"

#include "hal_eth.h"
#include "hal_scu.h"
//=============================================================================
//                  Constant Definition
//=============================================================================
#define CONFIG_ENABLE_LOOPBACK      0

#if !(CONFIG_ENABLE_LOOPBACK)
//#define CONFIG_SW_CRC_EN
//#define CONFIG_ENABLE_RX_ONLY
#endif


#define RED                 "\033[31m"
#define GREEN               "\033[32m"
#define YELLOW              "\033[33m"
#define NC                  "\033[m"

#define CONFIG_HOST_MAC_ADDR_0      0x04
#define CONFIG_HOST_MAC_ADDR_1      0x1f
#define CONFIG_HOST_MAC_ADDR_2      0x1f
#define CONFIG_HOST_MAC_ADDR_3      0xde
#define CONFIG_HOST_MAC_ADDR_4      0x76
#define CONFIG_HOST_MAC_ADDR_5      0xc7

#define CONFIG_REMOTE_MAC_ADDR_0    0x08
#define CONFIG_REMOTE_MAC_ADDR_1    0x08
#define CONFIG_REMOTE_MAC_ADDR_2    0x08
#define CONFIG_REMOTE_MAC_ADDR_3    0x08
#define CONFIG_REMOTE_MAC_ADDR_4    0x08
#define CONFIG_REMOTE_MAC_ADDR_5    0x08


#define CONFIG_SRC_PORT_NUM         8787
#define CONFIG_DEST_PORT_NUM        7878

#define PROTO_TCP                   6
#define PROTO_UDP                   17
#define PROTO_ICMP                  1

#define ETH_IPV4                    0x0800
#define ETH_ARP                     0x0806
#define ETH_PTP                     0x88F7

#define ARPOP_REQUEST               1

#define ICMP_ECHOREPLY              0	/* Echo Reply			*/
#define ICMP_ECHO                   8	/* Echo Request			*/

#define FLAG_DF                     0x4000
#define FLAG_MF                     0x2000
//=============================================================================
//                  Macro Definition
//=============================================================================
#define stringize(s)                    #s
#define tostr(a)                        stringize(a)
#define LOG_ERR_CASE(ec, errcode)       if((ec) == (errcode)) print_log("err %s\n", tostr(errcode))

static inline uint16_t __be16(uint16_t value)
{
    return (value << 8) | (value >> 8);
}

static inline uint32_t __be32(uint32_t value)
{
    return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8) | ((value & 0xFF0000) >> 8) | ((value & 0xFF000000) >> 24);
}

static inline uint32_t
__ip_convertor(uint8_t class_a, uint8_t class_b, uint8_t class_c, uint8_t class_d)
{
    uint32_t    addr32 = 0;
    addr32 = (class_d << 24) | (class_c << 16) | (class_b << 8) | (class_a);
    return addr32;
}
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

typedef struct in4addr
{
    union {
        uint8_t     addr8[4];
        uint32_t    addr32;
    };
} in4addr_t;

typedef struct inet_addr
{
    in4addr_t   v4_sa; // src address
    in4addr_t   v4_da; // dest address

    uint16_t    s_port; // src port
    uint16_t    d_port; // dest port
} inet_addr_t;

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

    in4addr_t   sa;
    in4addr_t   da;
} ipv4_hdr_t;


typedef struct icmp_hdr
{
    uint8_t     type;
    uint8_t     code;
    uint16_t    checksum;
    union {
        struct {
            uint16_t    id;
            uint16_t    sequence;
        } echo;

        uint32_t gateway;
    } un;

    uint8_t     pData[];

} icmp_hdr_t;

typedef struct udp_hdr
{
    uint16_t    s_port;
    uint16_t    d_port;
    uint16_t    length;
    uint16_t    checksum;
} udp_hdr_t;

typedef struct arp_hdr
{
  uint16_t      htype;
  uint16_t      ptype;
  uint8_t       hlen;
  uint8_t       plen;
  uint16_t      opcode;
  uint8_t       sender_mac[6];
  uint32_t      sender_ip;
  uint8_t       target_mac[6];
  uint32_t      target_ip;
} arp_hdr_t;

typedef struct udp_packet
{
    eth_hdr_t       eth_hdr;
    ipv4_hdr_t      ipv4_hdr;
    udp_hdr_t       udp_hdr;
    uint8_t         pData[];

} udp_packet_t;

typedef struct ping_packet
{
    eth_hdr_t       eth_hdr;
    ipv4_hdr_t      ipv4_hdr;
    icmp_hdr_t      icmp_hdr;

} ping_packet_t;
#pragma pack()

typedef struct packet_info
{
    inet_addr_t     net_addr;

    uint8_t         *pPacket;
    int             packet_len;

    // packet_len should be more than payload_len
    uint8_t         *pPayload;
    int             payload_len;

    uint8_t         local_macaddr[6];
    uint8_t         dest_macaddr[6];

} packet_info_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================
static uint8_t      g_tx_buf[2 << 10] __attribute__ ((aligned (16))) = {0};
static uint8_t      g_rx_buf[2 << 10] __attribute__ ((aligned (16))) = {0};
static uint32_t     g_is_rx_end = 0;
static uint8_t      *g_tx_package = 0;
static uint32_t     g_tx_pkg_len = 0;
static uint8_t      g_payload_data[1450] = {0};

static uint8_t const g_local_mac_addr[] =
{
    CONFIG_HOST_MAC_ADDR_0, CONFIG_HOST_MAC_ADDR_1, CONFIG_HOST_MAC_ADDR_2,
    CONFIG_HOST_MAC_ADDR_3, CONFIG_HOST_MAC_ADDR_4, CONFIG_HOST_MAC_ADDR_5,
};

static const scu_call_t     g_init_script[] =
{
    [0] = { .reg = 0x54000050ul, .value = (0x1ul << 2), .mask = (0x1ul << 2), },    // enable GMAC clock
    [1] = { .reg = 0x54000820ul, .value = (0x00ul << 10), .mask = (0x3ul << 10), }, // pin-mux switch to GPIO 13
};

//=============================================================================
//                  Private Function Definition
//=============================================================================
static eth_err_t
_eth_set_macaddr(
    hal_eth_macaddr_t   *pAddr)
{
    eth_err_t   rval = ETH_ERR_OK;
    pAddr->mac_addr[0] = g_local_mac_addr[0];
    pAddr->mac_addr[1] = g_local_mac_addr[1];
    pAddr->mac_addr[2] = g_local_mac_addr[2];
    pAddr->mac_addr[3] = g_local_mac_addr[3];
    pAddr->mac_addr[4] = g_local_mac_addr[4];
    pAddr->mac_addr[5] = g_local_mac_addr[5];
    return rval;
}

static eth_err_t
_eth_pht_reset(void)
{
    return ETH_ERR_OK;
}

static uint32_t*
_eth_malloc(
    eth_mem_t   type,
    int         length)
{
    uint32_t    *pBuf = 0;
    do {
        if( type == ETH_MEM_CACHE_NP_TX )
        {
            static uint32_t   txdesc_np_cache[ETH_TXQ_NP_ENTRIES * 4]  __attribute__ ((aligned (16))) = {0};
            if( length > sizeof(txdesc_np_cache) )
                break;

            pBuf = txdesc_np_cache;
            print_log("@ tx_np: %x, len= %x\n", pBuf, length);
        }
        else if( type == ETH_MEM_CACHE_HP_TX )
        {
            static uint32_t   txdesc_hp_cache[ETH_TXQ_HP_ENTRIES * 4]  __attribute__ ((aligned (16))) = {0};
            if( length > sizeof(txdesc_hp_cache) )
                break;

            pBuf = txdesc_hp_cache;
            print_log("@ tx_hp: %x, len= %x\n", pBuf, length);
        }
        else if( type == ETH_MEM_CACHE_RX )
        {
            static uint32_t   rxdesc_cache[ETH_RXQ_ENTRIES * 4]  __attribute__ ((aligned (16))) = {0};
            if( length > sizeof(rxdesc_cache) )
                break;

            pBuf = rxdesc_cache;
            print_log("@ rx_des: %x, len= %x\n", pBuf, length);
        }
        else if( type == ETH_MEM_PACKET_RX )
        {
            static uint32_t   rx_packages[ETH_RXQ_ENTRIES * ETH_RX_PACKET_SIZE]  __attribute__ ((aligned (16))) = {0};
            if( length > sizeof(rx_packages) )
                break;

            pBuf = rx_packages;
            print_log("@ rxpkg: %x, len= %x\n", pBuf, length);
        }

    } while(0);
    return pBuf;
}

static void
_eth_free(void *p)
{
    return;
}

static void
_user_notify(eth_state_t state)
{
    switch(state)
    {
        case ETH_STATE_LOOPBACK_MODE:           print_log(YELLOW "@state: LOOPBACK_MODE   \n" NC); break;
        case ETH_STATE_PHY_STATUS_UNKNOWN:      print_log("@state: PHY_STATUS_UNKNOWN   \n"); break;
        case ETH_STATE_10M_HALF:                print_log("@state: PHY_10M_HALF   \n"); break;
        case ETH_STATE_10M_FULL:                print_log("@state: PHY_10M_FULL   \n"); break;
        case ETH_STATE_100M_HALF:               print_log("@state: PHY_100M_HALF  \n"); break;
        case ETH_STATE_100M_FULL:               print_log("@state: PHY_100M_FULL  \n"); break;
        case ETH_STATE_TX_END:
            {
                static int  cnt = 0;
                print_log("@state: TX_END (%u)              \n", ++cnt);
            }
            break;
        case ETH_STATE_TX2FIFO:                 print_log("@state: TX2FIFO             \n"); break;
        case ETH_STATE_LINK_STATE_CHANGE:       print_log("@state: LINK_STATE_CHANGE   \n"); break;
        case ETH_STATE_NOTHING:                 print_log("@state: NOTHING             \n"); break;
        case ETH_STATE_RX_FIFO_FULL:            print_log(RED "@state: RX_FIFO_FULL        \n" NC); break;
        case ETH_STATE_RX_ODD_NIBBLES:          print_log(RED "@state: RX_ODD_NIBBLES      \n" NC); break;
        case ETH_STATE_RX_CRC_FAIL:             print_log(RED "@state: RX_CRC_FAIL         \n" NC); break;
        case ETH_STATE_RX_ERROR:                print_log(RED "@state: RX_ERROR            \n" NC); break;
        case ETH_STATE_RX_FRAME_TOO_LONG:       print_log(RED "@state: RX_FRAME_TOO_LONG   \n" NC); break;
        case ETH_STATE_RX_IP_CHECKSUM_FAIL:     print_log(RED "@state: RX_IP_CHECKSUM_FAIL \n" NC); break;
        case ETH_STATE_RX_UDP_CHECKSUM_FAIL:    print_log(RED "@state: RX_UDP_CHECKSUM_FAIL\n" NC); break;
        case ETH_STATE_RX_TCP_CHECKSUM_FAIL:    print_log(RED "@state: RX_TCP_CHECKSUM_FAIL\n" NC); break;
        default:    break;
    }
    return;
}

static void
_user_rx_handler(uint8_t *in_packet, int length)
{
    do {
        int     package_len = (sizeof(g_rx_buf) < length) ? sizeof(g_rx_buf) : length;

        // filter other MAC addresses
        if( ((uint32_t*)in_packet)[0] != (uint32_t)(-1ul) &&
            memcmp(in_packet, g_local_mac_addr, 6) )
            break;

        print_log("usrx: %x, len= %u\n", in_packet, length);

        memcpy(g_rx_buf, in_packet, package_len);

        if( CONFIG_ENABLE_LOOPBACK )
        {
            if( package_len != g_tx_pkg_len )
                print_log("@ rx/tx package length not match !!!!\n");

            if( !memcmp(g_rx_buf, g_tx_package, package_len) )
            {
                print_log(GREEN "@ rx compare pass~~\n"NC);
                g_is_rx_end = 1;
                break;
            }

            print_log(RED "@ rx compare fail !!!\n" NC);
            print_mem("", g_rx_buf, package_len);

            g_is_rx_end = 1;
            break;
        }

    #if 1//defined(CONFIG_ENABLE_RX_ONLY)
        print_mem("", g_rx_buf, package_len);
    #endif
    } while(0);

    return;
}

static void
_err_check(eth_err_t err_code)
{
    LOG_ERR_CASE(err_code, ETH_ERR_OK);
    LOG_ERR_CASE(err_code, ETH_ERR_NULL_POINTER);
    LOG_ERR_CASE(err_code, ETH_ERR_NO_INSTANCE);
    LOG_ERR_CASE(err_code, ETH_ERR_NO_PHY);
    LOG_ERR_CASE(err_code, ETH_ERR_WRONG_PARAM);
    LOG_ERR_CASE(err_code, ETH_ERR_NOT_SUPPORT);
    LOG_ERR_CASE(err_code, ETH_ERR_NOT_ALIGNMENT);
    LOG_ERR_CASE(err_code, ETH_ERR_OVER_FIFO_SIZE);
    LOG_ERR_CASE(err_code, ETH_ERR_TX_BUSY);
    LOG_ERR_CASE(err_code, ETH_ERR_SEND_FAIL);
    return;
}

__attribute__((unused)) static uint16_t
_eth_checksum(uint8_t *buff, int len, uint16_t crc_value)
{
    uint32_t    value = 0x0;
    uint16_t    checksum = crc_value;

    for(int i = 0; i < len; i += 2)
    {
        value = checksum + ((buff[i] << 8) | buff[i + 1]);
        checksum = (value & 0x0000FFFF) + (value >> 16);
    }
    return checksum;
}

int
gen_arp_packet(packet_info_t *pPkt_info)
{
    int     rval = 0;
    do {
        int         payload_len = 0;

        payload_len = pPkt_info->packet_len - sizeof(eth_hdr_t) - sizeof(arp_hdr_t);
        if( payload_len < pPkt_info->payload_len )
        {
            rval = -__LINE__;
            break;
        }

        if( (uint32_t)pPkt_info->pPacket & 0xF )
        {
            rval = -__LINE__;
            break;
        }

        memset(pPkt_info->pPacket, pPkt_info->pPayload[0], pPkt_info->packet_len);

        {   // Ethernet II
            eth_hdr_t   *pHdr_eth = 0;

            pHdr_eth = (eth_hdr_t*)pPkt_info->pPacket;

            memcpy(pHdr_eth->sa, pPkt_info->local_macaddr, sizeof(pHdr_eth->sa));
            memcpy(pHdr_eth->da, pPkt_info->dest_macaddr, sizeof(pHdr_eth->da));

            pHdr_eth->proto = __be16(ETH_ARP);
        }

        {   // ARP
            arp_hdr_t   *pHdr_arp = (arp_hdr_t*)(pPkt_info->pPacket + sizeof(eth_hdr_t));

            // Hardware type (16 bits): 1 for ethernet
            pHdr_arp->htype = __be16(1);

            // Protocol type (16 bits): 2048 for IP
            pHdr_arp->ptype = __be16(ETH_IPV4);

            // Hardware address length (8 bits): 6 bytes for MAC address
            pHdr_arp->hlen = 6;

            // Protocol address length (8 bits): 4 bytes for IPv4 address
            pHdr_arp->plen = 4;

            // OpCode: 1 for ARP request
            pHdr_arp->opcode = __be16(ARPOP_REQUEST);

            // Sender hardware address (48 bits): MAC address
            memcpy(&pHdr_arp->sender_mac, &pPkt_info->local_macaddr, sizeof(pHdr_arp->sender_mac));
            pHdr_arp->sender_ip = pPkt_info->net_addr.v4_sa.addr32;

            // Target hardware address (48 bits): zero, since we don't know it yet.
            memset(&pHdr_arp->target_mac, 0x0, sizeof(pHdr_arp->target_mac));
            pHdr_arp->target_ip = pPkt_info->net_addr.v4_da.addr32;
        }

        pPkt_info->packet_len = sizeof(eth_hdr_t) + sizeof(arp_hdr_t) + pPkt_info->payload_len;
        pPkt_info->packet_len = 60;

    } while(0);
    return rval;
}

int
gen_udp_packet(packet_info_t *pPkt_info)
{
    int     rval = 0;
    do {
        int             payload_len = 0;
        udp_packet_t    *pUdp_packet = 0;

        payload_len = pPkt_info->packet_len - sizeof(eth_hdr_t) - sizeof(ipv4_hdr_t) - sizeof(udp_hdr_t);
        if( payload_len < pPkt_info->payload_len )
        {
            rval = -__LINE__;
            break;
        }

        if( (uint32_t)pPkt_info->pPacket & 0xF )
        {
            rval = -__LINE__;
            break;
        }

        memset(pPkt_info->pPacket, 0x0, pPkt_info->packet_len);

        pUdp_packet = (udp_packet_t*)pPkt_info->pPacket;

        {   // Ethernet II
            eth_hdr_t   *pHdr_eth = &pUdp_packet->eth_hdr;

            memcpy(pHdr_eth->sa, pPkt_info->local_macaddr, sizeof(pHdr_eth->sa));
            memcpy(pHdr_eth->da, pPkt_info->dest_macaddr, sizeof(pHdr_eth->da));

            pHdr_eth->proto = __be16(ETH_IPV4);
        }

        {    // IPv4
            static uint16_t packet_cnt = 0;

            ipv4_hdr_t      *pHdr_ipv4 = &pUdp_packet->ipv4_hdr;
            uint16_t        pkt_len = 0;

            pkt_len = sizeof(ipv4_hdr_t) + sizeof(udp_hdr_t) + pPkt_info->payload_len;

            pHdr_ipv4->version      = 4;
            pHdr_ipv4->ihl          = 5;
            pHdr_ipv4->services     = 0;
            pHdr_ipv4->len          = __be16(pkt_len);
            pHdr_ipv4->ttl          = 128;
            pHdr_ipv4->id           = packet_cnt++;
            pHdr_ipv4->protocol     = PROTO_UDP;
            pHdr_ipv4->flags        = 0;
            pHdr_ipv4->frag_offset  = 0;// __be16(FLAG_DF);

            memcpy(pHdr_ipv4->sa.addr8, &pPkt_info->net_addr.v4_sa, sizeof(in4addr_t));
            memcpy(pHdr_ipv4->da.addr8, &pPkt_info->net_addr.v4_da, sizeof(in4addr_t));

            #if defined(CONFIG_SW_CRC_EN)
            {
                uint16_t    crc16 = 0;
                crc16 = _eth_checksum((uint8_t*)pHdr_ipv4, (pHdr_ipv4->ihl << 2), 0);
                print_log("ipv4 checksum= %x\n", ~__be16(crc16));
                pHdr_ipv4->checksum = ~__be16(crc16);
            }
            #else
            // Let HW calculate the checksum
            pHdr_ipv4->checksum = 0;
            #endif
        }

        {    // UDP
            udp_hdr_t       *pHdr_udp = &pUdp_packet->udp_hdr;

            pHdr_udp->s_port = pPkt_info->net_addr.s_port;
            pHdr_udp->d_port = pPkt_info->net_addr.d_port;
            pHdr_udp->length = __be16(pPkt_info->payload_len + sizeof(udp_hdr_t));

            #if defined(CONFIG_SW_CRC_EN)
            {
                uint16_t        crc16 = 0;
                uint16_t        udp_len16 = 0;
                int16_t         n_hdr = __be16(PROTO_UDP);
                inet_addr_t     *pNet_addr = &pPkt_info->net_addr;

                udp_len16 = pHdr_udp->length;

                crc16 = _eth_checksum(pNet_addr->v4_sa.addr8, sizeof(in4addr_t), 0);
                crc16 = _eth_checksum(pNet_addr->v4_da.addr8, sizeof(in4addr_t), crc16);
                crc16 = _eth_checksum((uint8_t*)&udp_len16, sizeof(udp_len16), crc16);
                crc16 = _eth_checksum((uint8_t*)&n_hdr, sizeof(n_hdr), crc16);
                crc16 = _eth_checksum((uint8_t*)pHdr_udp, (pPkt_info->payload_len + sizeof(udp_hdr_t)), crc16);

                print_log("udp checksum= %x\n", ~__be16(crc16));
                pHdr_udp->checksum = ~__be16(crc16);
            }
            #else
            // Let HW calculate the checksum
            pHdr_udp->checksum = 0;
            #endif

            memcpy(pUdp_packet->pData, pPkt_info->pPayload, pPkt_info->payload_len);
        }

        pPkt_info->packet_len = pPkt_info->payload_len + sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t) + sizeof(udp_hdr_t);

    } while(0);
    return rval;
}

int
gen_ping_packet(packet_info_t *pPkt_info)
{
    int         rval = 0;
    do {
        int             payload_len = 0;
        ping_packet_t   *pPing_pkt = 0;

        payload_len = pPkt_info->packet_len - sizeof(eth_hdr_t) - sizeof(ipv4_hdr_t) - sizeof(icmp_hdr_t);
        if( payload_len < pPkt_info->payload_len )
        {
            rval = -__LINE__;
            break;
        }

        if( (uint32_t)pPkt_info->pPacket & 0xF )
        {
            rval = -__LINE__;
            break;
        }

        payload_len = 32;

        memset(pPkt_info->pPacket, pPkt_info->pPayload[0], pPkt_info->packet_len);

        pPing_pkt = (ping_packet_t*)pPkt_info->pPacket;

        {   // Ethernet II
            eth_hdr_t   *pHdr_eth = &pPing_pkt->eth_hdr;

            memcpy(pHdr_eth->sa, pPkt_info->local_macaddr, sizeof(pHdr_eth->sa));

        #if 0
            memcpy(pHdr_eth->da, pPkt_info->dest_macaddr, sizeof(pHdr_eth->da));
        #else
            #if 1
            pHdr_eth->da[0] = 0x00;
            pHdr_eth->da[1] = 0xa0;
            pHdr_eth->da[2] = 0xb0;
            pHdr_eth->da[3] = 0xae;
            pHdr_eth->da[4] = 0xec;
            pHdr_eth->da[5] = 0xca;
            #else
            pHdr_eth->da[0] = 0xb8;
            pHdr_eth->da[1] = 0x27;
            pHdr_eth->da[2] = 0xeb;
            pHdr_eth->da[3] = 0x8b;
            pHdr_eth->da[4] = 0x58;
            pHdr_eth->da[5] = 0x60;
            #endif
        #endif

            pHdr_eth->proto = __be16(ETH_IPV4);
        }

        {    // IPv4
            static uint16_t packet_cnt = 0;

            ipv4_hdr_t      *pHdr_ipv4 = &pPing_pkt->ipv4_hdr;
            uint16_t        pkt_len = 0;

            pkt_len = sizeof(ipv4_hdr_t) + sizeof(icmp_hdr_t) + payload_len;

            pHdr_ipv4->version      = 4;
            pHdr_ipv4->ihl          = 5;
            pHdr_ipv4->services     = 0;
            pHdr_ipv4->len          = __be16(pkt_len);
            pHdr_ipv4->ttl          = 128;
            pHdr_ipv4->id           = packet_cnt++;
            pHdr_ipv4->protocol     = PROTO_ICMP;
            pHdr_ipv4->flags        = 0;
            pHdr_ipv4->frag_offset  = 0;// __be16(FLAG_DF);

            memcpy(pHdr_ipv4->sa.addr8, &pPkt_info->net_addr.v4_sa, sizeof(in4addr_t));
            memcpy(pHdr_ipv4->da.addr8, &pPkt_info->net_addr.v4_da, sizeof(in4addr_t));
            pHdr_ipv4->checksum = 0;
            __asm("nop");
        }

        {   // ICMP
            static uint16_t packet_cnt = 0;
            icmp_hdr_t      *pIcmp_hdr = &pPing_pkt->icmp_hdr;
            uint16_t        crc16 = 0;

            pIcmp_hdr->type             = ICMP_ECHO;
            pIcmp_hdr->code             = 0;
            pIcmp_hdr->un.echo.id       = __be16(0x1);
            pIcmp_hdr->un.echo.sequence = __be16(++packet_cnt);
            pIcmp_hdr->checksum         = 0;

            for(int i = 0; i < payload_len; i++)
                pIcmp_hdr->pData[i] = '0' + (i & 0x7);

            crc16 = _eth_checksum((uint8_t*)pIcmp_hdr, sizeof(icmp_hdr_t) + payload_len, 0);
            pIcmp_hdr->checksum = ~__be16(crc16);
            __asm("nop");
        }

        pPkt_info->packet_len = sizeof(eth_hdr_t) + sizeof(ipv4_hdr_t) + sizeof(icmp_hdr_t) + payload_len;

    } while(0);
    return rval;
}

static int
_test_loopback(
    packet_info_t   *pPkt_info)
{
    int             cnt = 0;
    uint32_t        timeout = 0;

    g_is_rx_end = 1;

    while( cnt < 30 )
    {
        int             rval = 0;
        uint8_t         data[36] = {0};

        if( !g_is_rx_end && timeout++ < 10000000 )
            continue;

        timeout     = 0;
        g_is_rx_end = 0;
        cnt++;

        memset(data, cnt & 0xFF, sizeof(data));
        pPkt_info->pPayload       = data;
        pPkt_info->payload_len    = sizeof(data);
        pPkt_info->pPacket        = g_tx_buf;
        pPkt_info->packet_len     = sizeof(g_tx_buf);

        gen_arp_packet(pPkt_info);

        print_log("pkg len= %u\n", pPkt_info->packet_len);
//        print_mem("tx:", pPkt_info->pPacket, pPkt_info->packet_len);

        g_tx_package = pPkt_info->pPacket;
        g_tx_pkg_len = pPkt_info->packet_len;

        rval = hal_eth_send(pPkt_info->pPacket, pPkt_info->packet_len);
        if( rval )
        {
            _err_check(rval);
        }
    }
    return 0;
}

static int
_test_send(
    packet_info_t   *pPkt_info)
{
    int         cnt = 0;
    uint32_t    unit_ms = hal_sys_get_core_clk() / 1000;

    while( cnt++ < 30 )
    {
        int             rval = 0;
        uint8_t         data[36] = {0};

        pPkt_info->pPacket        = g_tx_buf;
        pPkt_info->packet_len     = sizeof(g_tx_buf);

#if 1
        memset(data, cnt & 0xFF, sizeof(data));
        pPkt_info->pPayload       = data;
        pPkt_info->payload_len    = sizeof(data);
        gen_arp_packet(pPkt_info);
#elif (0)
        memset(g_payload_data, cnt & 0xFF, sizeof(g_payload_data));
        pPkt_info->pPayload       = g_payload_data;
        pPkt_info->payload_len    = sizeof(g_payload_data);
        gen_udp_packet(pPkt_info);
#else
        memset(g_payload_data, cnt & 0xFF, sizeof(g_payload_data));
        pPkt_info->pPayload       = g_payload_data;
        pPkt_info->payload_len    = sizeof(g_payload_data);
        gen_ping_packet(pPkt_info);
#endif

        print_log("pkg len= %u\n", pPkt_info->packet_len);
//        print_mem("tx:", pPkt_info->pPacket, pPkt_info->packet_len);

        rval = hal_eth_send(pPkt_info->pPacket, pPkt_info->packet_len);
        if( rval )
        {
            _err_check(rval);
        }

        /**
         *  delay 5 ms, the wireshark delay 200ms
         */
        for(int i = 0; i < (unit_ms * 5); i++) {}
    }
    return 0;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void HardFault_Handler(void)
{
    extern void exception_dump(void);
    exception_dump();
    __pause();
    return;
}

int main(void)
{
    uint32_t    remap = -1;
    comm_init("\n\n################################ Test Ethernet\n");

    hal_scu_get_mmp_type(&remap);
    print_log("remap= %u\n", remap);

#if 1
    hal_scu_exec_script((scu_call_t*)g_init_script, sizeof(g_init_script)/sizeof(g_init_script[0]));
#else
    *((volatile uint32_t*)0x54000050) |= (0x1 << 2);
//    *((volatile uint32_t*)0x54000864) &= (~(0x03 << 10)));
//    *((volatile uint32_t*)0x54000820) &= (~(0x03 << 10)));
#endif

    do {
        int             rval = 0;
        hal_eth_op_t    eth_op = {0};

        hal_eth_register_recv_handler(_user_rx_handler);
        hal_eth_register_notify(_user_notify);

        eth_op.is_loopback    = CONFIG_ENABLE_LOOPBACK;
        eth_op.cb_set_macaddr = _eth_set_macaddr;
        eth_op.cb_malloc      = _eth_malloc;
        eth_op.cb_free        = _eth_free;
        eth_op.cb_phy_reset   = _eth_pht_reset;
        rval = hal_eth_init(&eth_op);
        if( rval )
        {
            _err_check(rval);
        }

        #if defined(CONFIG_ENABLE_RX_ONLY)
            while(1)
                __asm("nop");
        #else
        {
            packet_info_t   pkt_info = {0};

            pkt_info.local_macaddr[0] = CONFIG_HOST_MAC_ADDR_0;
            pkt_info.local_macaddr[1] = CONFIG_HOST_MAC_ADDR_1;
            pkt_info.local_macaddr[2] = CONFIG_HOST_MAC_ADDR_2;
            pkt_info.local_macaddr[3] = CONFIG_HOST_MAC_ADDR_3;
            pkt_info.local_macaddr[4] = CONFIG_HOST_MAC_ADDR_4;
            pkt_info.local_macaddr[5] = CONFIG_HOST_MAC_ADDR_5;

            memset(&pkt_info.dest_macaddr, 0xff, sizeof(pkt_info.dest_macaddr));

            pkt_info.net_addr.v4_sa.addr32  = __ip_convertor(192, 168, 99, 5);
            pkt_info.net_addr.v4_da.addr32  = __ip_convertor(192, 168, 99, 2);
            pkt_info.net_addr.s_port        = __be16(CONFIG_SRC_PORT_NUM);
            pkt_info.net_addr.d_port        = __be16(CONFIG_DEST_PORT_NUM);

            if( CONFIG_ENABLE_LOOPBACK )
                _test_loopback(&pkt_info);
            else
                _test_send(&pkt_info);

            print_log("--- send end ---\n");
        }
        #endif
    } while(0);

    hal_eth_deinit();

    print_log("done~~\n");

    while(1)
        __asm("nop");
    return 0;
}
