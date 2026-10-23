/***********************************************************************************************************************
 * File Name    : ethernet_hpsw_example.h
 * Description  : Contains declarations of data structures and functions used in ethernet_hpsw_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ETHERNET_HPSW_EXAMPLE_H
#define ETHERNET_HPSW_EXAMPLE_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "common_utils.h"
#include "ethernet_hpsw_ep.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define DEMO_SENDER_BOARD            (1)    /* Board to send Ping request */
#define GMAC_BUFFER_SIZE             (2048U)
#define MAC_TYPE_ARP                 (0x0806U)
#define MAC_TYPE_IP                  (0x0800U)
#define IP_PROTOCOL_ICMP             (1U)
#define ICMP_TYPE_ECHO_REQUEST       (8U)
#define ICMP_TYPE_ECHO_REPLY         (0U)
#define PING_MAC_ADDRESS_SIZE        (6U)
#define PING_IP_ADDRESS_SIZE         (4U)

#define GMAC_RX_FRAME_DATA_SIZE      (GMAC_BUFFER_SIZE - PING_MAC_ADDRESS_SIZE \
                                     - PING_MAC_ADDRESS_SIZE - sizeof(uint16_t))

#define PING_PORT_COUNT              (1U)
#define PING_PORT_ALL                (5U)

#define HPSW_ENABLE                  (true)
#define HPSW_OPEN                    (('H' << 24U) | ('P' << 16U) | ('S' << 8U) | ('W' << 0U))

/* Key code for PRCMD register */
#define ETHER_SELECTOR_PRCMD_UNLOCK1    (0x000000A5U)
#define ETHER_SELECTOR_PRCMD_UNLOCK2    (0x00000001U)
#define ETHER_SELECTOR_PRCMD_UNLOCK3    (0x0000FFFEU)
#define ETHER_SELECTOR_PRCMD_UNLOCK4    (0x00000001U)
#define ETHER_SELECTOR_PRCMD_LOCK       (0x00000000U)

/* Convert byte order of 32 bits from network to host */
#define CONVERT_NTOHL(x) \
    (uint32_t) ( (uint32_t)((((x) >> 24) & (0xffUL))) | \
                 (uint32_t)((((x) >>  8) & (0xff00UL))) | \
                 (uint32_t)(((((x) & 0xffffffUL) << 8) & (0xff0000UL))) | \
                 (uint32_t)(((((x) & 0xffUL) << 24) & (0xff000000UL))) )

/* Convert byte order of 16 bits from network to host */
#define CONVERT_NTOHS(x) \
    (uint16_t) ((uint16_t)((((x) >> 8) & 0xffU)) | \
                (uint16_t)((((x) & 0xffU) << 8)) )

/* Convert byte order of 32 bits from host to network */
#define CONVERT_HTONL(x)            CONVERT_NTOHL(x)

/* Convert byte order of 16 bits from host to network */
#define CONVERT_HTONS(x)            CONVERT_NTOHS(x)

/***********************************************************************************************************************
 * Demo board selection
 *
 * Define DEMO_SENDER_BOARD in project preprocessor settings for Board A build.
 * If not defined, the code behaves as Board B build.
 **********************************************************************************************************************/
#define NETWORK_ADDRESS      (255UL<<24 | 255UL<<16 | 255UL<<8 | 0UL)   /* 255.255.255.0 */
#if defined(DEMO_SENDER_BOARD)
#define OWN_IP_ADDRESS       (192UL<<24 | 168UL<<16 | 10UL<<8 | 100UL)  /* 192.168.10.100 */
#define PEER_IP_ADDRESS      (192UL<<24 | 168UL<<16 | 10UL<<8 | 101UL)  /* 192.168.10.101 */
#else
#define OWN_IP_ADDRESS       (192UL<<24 | 168UL<<16 | 10UL<<8 | 101UL)
#define PEER_IP_ADDRESS      (192UL<<24 | 168UL<<16 | 10UL<<8 | 100UL)
#endif /* DEMO_SENDER_BOARD */

/* Macros for user input for menu selection */
#define ETHERNET_PING           (1u)
#define HPSW_GET_COUNTER        (2u)
#define HPSW_EXIT               (3u)

#define EP_INFO                "\r\nThis example project demonstrates the HSR/PRP Switch (HPSW)"\
                               "\r\nmodule for Ethernet redundancy communication using 2 Renesas RZ MPUs."\
                               "\r\nHPSW duplicates GMAC packet and transfers via ETH3 and ETH4 ports"\
                               "\r\nbetween two RZ boards. Ping communication remains available even if"\
                               "\r\none redundant network link fails."\
                               "\r\nDemonstrations:"\
                               "\r\n 1. Normal HSR/PRP communication"\
                               "\r\n 2. HSR/PRP communication during link failure"\
                               "\r\n 3. HPSW status and traffic statistics\r\n"\
                               "\r\nStatus and user prompts are displayed on the Terminal Emulator.\r\n\r\n"

/* HPSW menu options */
#define HPSW_MENU              "\r\nHPSW Menu options\r\n"\
                                "1. Start Ping communication between 2 RZ boards\r\n"\
                                "2. Show HPSW statistics counters\r\n"\
                                "3. Exit demo\r\n"\
                                "Note: After successful ping, can unplug a LAN cable connected to\r\n"\
                                "ETH3 or ETH4 port to create single network failure. Then enter '1' to run next ping.\r\n\r\n"
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
#if defined(DEMO_SENDER_BOARD)
typedef enum e_eth_demo_state
{
    ETH_DEMO_IDLE = 0,
    ETH_DEMO_ICMP
} eth_demo_state_t;
#endif /* DEMO_SENDER_BOARD */

/* Ethernet frame format struct */
typedef struct st_gmac_frame
{
    uint8_t  des_address[PING_MAC_ADDRESS_SIZE];
    uint8_t  sur_address[PING_MAC_ADDRESS_SIZE];
    uint16_t type;
    uint8_t  data[GMAC_RX_FRAME_DATA_SIZE];
} st_gmac_frame_t;

/* Ethernet frame header format struct */
typedef struct st_gmac_mac
{
    uint8_t  mac_des_addr[PING_MAC_ADDRESS_SIZE];
    uint8_t  mac_sur_addr[PING_MAC_ADDRESS_SIZE];
    uint16_t mac_type;
} st_gmac_mac_t;

/* ARP packet format struct */
typedef struct st_gmac_arp
{
    uint16_t arp_hardware_type;
    uint16_t arp_protocol_type;
    uint8_t  arp_hardware_size;
    uint8_t  arp_protocol_size;
    uint16_t arp_opcode;
    uint8_t  arp_sender_mac[PING_MAC_ADDRESS_SIZE];
    uint8_t  arp_sender_ip[PING_IP_ADDRESS_SIZE];
    uint8_t  arp_target_mac[PING_MAC_ADDRESS_SIZE];
    uint8_t  arp_target_ip[PING_IP_ADDRESS_SIZE];
} st_gmac_arp_t;

/* IP packet format struct */
typedef struct st_gmac_ip
{
    uint8_t  ip_ver;
    uint8_t  ip_tos;
    uint16_t ip_total_leng;
    uint16_t ip_id;
    uint16_t ip_offset;
    uint8_t  ip_ttl;
    uint8_t  ip_protocol;
    uint16_t ip_checksum;
    uint8_t  ip_sou_addr[PING_IP_ADDRESS_SIZE];
    uint8_t  ip_des_addr[PING_IP_ADDRESS_SIZE];
} st_gmac_ip_t;

/* ICMP message format struct */
typedef struct st_gmac_icmp
{
    uint8_t  icmp_type;
    uint8_t  icmp_code;
    uint16_t icmp_checksum;
    uint16_t icmp_id;
    uint16_t icmp_seq;
} st_gmac_icmp_t;

/* ARP format struct */
typedef struct st_arp_packet
{
    st_gmac_mac_t arpp_mac;
    st_gmac_arp_t arpp_arp;
} st_gmac_arp_packet_t;

/* ICMP format struct */
typedef struct st_icmp_packet
{
    st_gmac_mac_t  icmpp_mac;
    st_gmac_ip_t   icmpp_ip;
    st_gmac_icmp_t icmpp_icmp;
    uint8_t        payload[32];
} st_icmp_packet_t;

/* Transmission Packet Buffer */
typedef union
{
    st_gmac_arp_packet_t   arpp;
    st_icmp_packet_t       icmpp;
    uint8_t                data[(GMAC_BUFFER_SIZE-sizeof(st_gmac_arp_packet_t))-sizeof(st_icmp_packet_t)];
} u_tx_packet_t;

/* Own IP address struct */
typedef struct st_own_addr
{
    uint8_t  mac_addr[PING_MAC_ADDRESS_SIZE];
    uint32_t ip_addr;
    uint32_t ip_mask;
} st_gmac_own_addr_t;

/***********************************************************************************************************************
 * Extern global variables
 **********************************************************************************************************************/
extern const ether_instance_t g_ether0;

/***********************************************************************************************************************
 * Public function declaration
 **********************************************************************************************************************/
void eth_hpsw_example_demo (void);

#endif /* ETHERNET_HPSW_EXAMPLE_H */
