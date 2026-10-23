/***********************************************************************************************************************
 * File Name    : ethernet_hpsw_example.c
 * Description  : Contains data structures and functions used in ethernet_hpsw_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "ethernet_hpsw_example.h"

/***********************************************************************************************************************
 * @addtogroup r_hpsw_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private variables
 **********************************************************************************************************************/
uint8_t g_broadcast_mac_addr[PING_MAC_ADDRESS_SIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#if defined(DEMO_SENDER_BOARD)
static const uint8_t g_mac_port0[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
static uint8_t g_peer_mac[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
#else
static const uint8_t g_mac_port0[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
static uint8_t g_peer_mac[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
#endif /* DEMO_SENDER_BOARD */

static st_gmac_own_addr_t g_own_addr[PING_PORT_COUNT] =
{
    {{0x00, 0x22, 0x22, 0x22, 0x22, 0x22},
    OWN_IP_ADDRESS,
    NETWORK_ADDRESS}
};

/* Replace ether_frame_t with the exact RX frame type from your local GMAC project if needed */
static st_gmac_frame_t g_gmac_rx_frame      = {0};
static volatile uint32_t g_ping_seq         = 0U;
static volatile uint32_t g_rx_count         = 0U;
static volatile uint32_t g_tx_count         = 0U;
static volatile uint32_t g_arp_count        = 0U;
static volatile uint32_t g_icmp_req_count   = 0U;
static volatile uint32_t g_icmp_rep_count   = 0U;

static volatile uint32_t g_arp_req_tx_count = 0U;
static volatile uint32_t g_arp_rep_rx_count = 0U;
static volatile uint32_t g_arp_rep_tx_count = 0U;

static uint8_t g_link_change = 0;      /* Link change (bit0:port0, bit1:port1, bit2:port2) */
static uint8_t g_link_status = 0;      /* Link status (bit0:port0, bit1:port1, bit2:port2) */
volatile bool g_rx_enabled = false;

#if defined(DEMO_SENDER_BOARD)
static eth_demo_state_t g_eth_demo_state = ETH_DEMO_IDLE;
#endif /* DEMO_SENDER_BOARD */

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void subsystem_protection_disable (void);
static void subsystem_protection_enable (void);
#if defined(DEMO_SENDER_BOARD)
static void send_arp_request (uint8_t port);
static void ping_icmp (void);
#endif /* DEMO_SENDER_BOARD */
static void incoming_arp (uint8_t port, uint8_t * p_buff);
static void incoming_icmp (uint8_t port, uint8_t * p_buff);
static void incoming_ip (uint8_t port, uint8_t * p_buff);
static void incoming_mac (uint8_t port, uint8_t * p_buff);
static void check_link_status (void);
static void ether_hpsw_example_start (void);
static uint16_t compute_checksum (uint32_t pseudo_sum, uint8_t * p_buf, uint32_t size);
#if defined(DEMO_SENDER_BOARD)
static void u32_to_ip_bytes (uint32_t ip, uint8_t * p_ip);
#endif /* DEMO_SENDER_BOARD */
static uint32_t ip_bytes_to_u32 (uint8_t * p_ip);

/***********************************************************************************************************************
 * Private helper functions
 **********************************************************************************************************************/
static uint32_t ip_bytes_to_u32(uint8_t * p_ip)
{
    return ((uint32_t)p_ip[0] << 24) |
           ((uint32_t)p_ip[1] << 16) |
           ((uint32_t)p_ip[2] << 8)  |
           ((uint32_t)p_ip[3]);
}

static uint8_t mac_equal(const uint8_t * a, const uint8_t * b)
{
    return (0 == memcmp(a, b, PING_MAC_ADDRESS_SIZE)) ? 1U : 0U;
}

#if defined(DEMO_SENDER_BOARD)
static void u32_to_ip_bytes(uint32_t ip, uint8_t * p_ip)
{
    p_ip[0] = (uint8_t)((ip >> 24) & 0xFF);
    p_ip[1] = (uint8_t)((ip >> 16) & 0xFF);
    p_ip[2] = (uint8_t)((ip >>  8) & 0xFF);
    p_ip[3] = (uint8_t)( ip        & 0xFF);
}
#endif /* DEMO_SENDER_BOARD */
/***********************************************************************************************************************
 * @brief       This function computes checksum in building Ethernet frame.
 * @param[IN]   pseudo_sum  Pseudo checksum.
 * @param[IN]   p_buf       Buffer pointer to calculate checksum.
 * @param[IN]   size        Buffer size to calculate checksum.
 * @retval      Calculated checksum value.
 **********************************************************************************************************************/
static uint16_t compute_checksum(uint32_t pseudo_sum, uint8_t * p_buf, uint32_t size)
{
    uint32_t sum = pseudo_sum;
    uint32_t i;

    for (i = 0; i + 1U < size; i += 2U)
    {
        sum += ((uint32_t)p_buf[i] << 8) | p_buf[i + 1U];
    }

    if (size & 1U)
    {
        sum += ((uint32_t)p_buf[size - 1U] << 8);
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

/***********************************************************************************************************************
 * @brief       This function unlocks write access protection for Ethernet subsystem registers.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void subsystem_protection_disable(void)
{
    /* Unlock the protection of Ethernet subsystem register */
    R_ETHSS->PRCMD = ETHER_SELECTOR_PRCMD_UNLOCK1;
    R_ETHSS->PRCMD = ETHER_SELECTOR_PRCMD_UNLOCK2;
    R_ETHSS->PRCMD = ETHER_SELECTOR_PRCMD_UNLOCK3;
    R_ETHSS->PRCMD = ETHER_SELECTOR_PRCMD_UNLOCK4;

    return;
}

/***********************************************************************************************************************
 * @brief       This function locks write access protection for Ethernet subsystem registers.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void subsystem_protection_enable(void)
{
    /* Lock the protection of Ethernet access register */
    R_ETHSS->PRCMD = ETHER_SELECTOR_PRCMD_LOCK;

    return;
}

/***********************************************************************************************************************
 * @brief       This function checks link status and displays when link status changes.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void check_link_status(void)
{
        fsp_err_t                err = FSP_SUCCESS;
        gmac_b_link_status_t     port_status;
        uint8_t                  port;
        uint8_t                  port_bit;

        if (0 == P_GMAC_CFG->p_callback)
        {
            /* Not set callback */
            uint8_t previous_link_status = g_link_status;

            g_link_status = 0;

            for (port = 0; port < PING_PORT_ALL; port++)
            {
                if ((port != 2) && (port != 1))
                {
                    err = R_GMAC_B_GetLinkStatus(P_GMAC_CTRL, port, &port_status);
                    if (FSP_SUCCESS != err)
                    {
                        /* An error has occurred */
                        APP_PRINT("R_GMAC_B_GetLinkStatus API failed\n");
                    }

                    if (GMAC_LINK_STATUS_DOWN != port_status)
                    {
                        /* Set link up */
                        g_link_status |= (uint8_t)(1U << port);
                    }
                }
            }

            if (FSP_SUCCESS == err)
            {
                /* Set changed link status */
                g_link_change = previous_link_status ^ g_link_status;
            }
        }

        if (FSP_SUCCESS == err)
        {
            for (port = 0; port < PING_PORT_ALL; port++)
            {
                port_bit = (uint8_t)(1U << port);   /* cast to uint8_t */

                if (g_link_change & port_bit)
                {
                    /* Link status changed */
                    g_link_change &= (uint8_t)(~port_bit);  /* change bit clear */

                    if (g_link_status & port_bit)
                    {
                        /* Change to Link-up */
                        APP_PRINT("Port %d link is up\n", port);
                    }
                    else
                    {
                        /* Change to Link-down */
                        APP_PRINT("Port %d link is down\n", port);
                    }
                }
            }
        }
}

#if defined(DEMO_SENDER_BOARD)
/***********************************************************************************************************************
 * @brief       This function sends ARP request packet.
 * @param[IN]   port        Port number received.
 * @param[IN]   p_buff      buffer of received Ethernet frame(ARP packet).
 * @retval      None.
 **********************************************************************************************************************/
static void send_arp_request(uint8_t port)
{
    st_gmac_arp_packet_t tx_pkt;
    fsp_err_t err;
    uint32_t tx_size = sizeof(st_gmac_arp_packet_t);

    FSP_PARAMETER_NOT_USED(port);

    memset(&tx_pkt, 0, sizeof(tx_pkt));

    memset(tx_pkt.arpp_mac.mac_des_addr, 0xFF, PING_MAC_ADDRESS_SIZE);
    memcpy(tx_pkt.arpp_mac.mac_sur_addr, g_own_addr[0].mac_addr, PING_MAC_ADDRESS_SIZE);
    tx_pkt.arpp_mac.mac_type = CONVERT_HTONS(MAC_TYPE_ARP);

    tx_pkt.arpp_arp.arp_hardware_type = CONVERT_HTONS(1U);
    tx_pkt.arpp_arp.arp_protocol_type = CONVERT_HTONS(MAC_TYPE_IP);
    tx_pkt.arpp_arp.arp_hardware_size = 6U;
    tx_pkt.arpp_arp.arp_protocol_size = 4U;
    tx_pkt.arpp_arp.arp_opcode        = CONVERT_HTONS(1U);

    memcpy(tx_pkt.arpp_arp.arp_sender_mac, g_own_addr[0].mac_addr, PING_MAC_ADDRESS_SIZE);
    u32_to_ip_bytes(g_own_addr[0].ip_addr, tx_pkt.arpp_arp.arp_sender_ip);

    memset(tx_pkt.arpp_arp.arp_target_mac, 0x00, PING_MAC_ADDRESS_SIZE);
    u32_to_ip_bytes(PEER_IP_ADDRESS, tx_pkt.arpp_arp.arp_target_ip);

    APP_PRINT("Send ARP request: who has %lu.%lu.%lu.%lu ?\r\n",
            ((PEER_IP_ADDRESS >> 24) & 0xff),
            ((PEER_IP_ADDRESS >> 16) & 0xff),
            ((PEER_IP_ADDRESS >>  8) & 0xff),
            ((PEER_IP_ADDRESS >>  0) & 0xff));

    err = R_GMAC_B_Write(P_GMAC_CTRL, (uint8_t *)&tx_pkt, tx_size);
    if (FSP_SUCCESS == err)
    {
        g_tx_count++;
        g_arp_req_tx_count++;
        APP_PRINT("ARP request sent successfully\r\n");
    }
    else
    {
        APP_PRINT("ARP request sent failed\r\n");
    }
}
#endif /* DEMO_SENDER_BOARD */

#if defined(DEMO_SENDER_BOARD)
/***********************************************************************************************************************
 * @brief       This function generates and sends ICMP Echo Request frame.
 * @param       None.
 * @retval      None.
 **********************************************************************************************************************/
static void ping_icmp(void)
{
    fsp_err_t err;
    uint16_t icmp_len;
    uint16_t ip_total_len;
    uint32_t tx_size;
    uint32_t ping_seq = (uint32_t) g_ping_seq;
    st_icmp_packet_t tx_pkt;

    memset(&tx_pkt, 0, sizeof(tx_pkt));
    memcpy(tx_pkt.icmpp_mac.mac_des_addr, g_peer_mac, PING_MAC_ADDRESS_SIZE);
    memcpy(tx_pkt.icmpp_mac.mac_sur_addr, g_own_addr[0].mac_addr, PING_MAC_ADDRESS_SIZE);
    tx_pkt.icmpp_mac.mac_type = CONVERT_HTONS(MAC_TYPE_IP);

    tx_pkt.icmpp_ip.ip_ver      = 0x45U;
    tx_pkt.icmpp_ip.ip_tos      = 0U;
    tx_pkt.icmpp_ip.ip_id       = CONVERT_HTONS(0x1234U);
    tx_pkt.icmpp_ip.ip_offset   = CONVERT_HTONS(0x0000U);
    tx_pkt.icmpp_ip.ip_ttl      = 64U;
    tx_pkt.icmpp_ip.ip_protocol = IP_PROTOCOL_ICMP;

    u32_to_ip_bytes(g_own_addr[0].ip_addr, tx_pkt.icmpp_ip.ip_sou_addr);
    u32_to_ip_bytes(PEER_IP_ADDRESS, tx_pkt.icmpp_ip.ip_des_addr);

    tx_pkt.icmpp_icmp.icmp_type = ICMP_TYPE_ECHO_REQUEST;
    tx_pkt.icmpp_icmp.icmp_code = 0U;
    tx_pkt.icmpp_icmp.icmp_id   = CONVERT_HTONS(0x4321U);
    tx_pkt.icmpp_icmp.icmp_seq  = CONVERT_HTONS((uint16_t) ping_seq);
    g_ping_seq++;

    memcpy(tx_pkt.payload, "ETHER_HPSW_RZT2N-EVK", 20U);

    icmp_len = (uint16_t)(sizeof(st_gmac_icmp_t) + sizeof(tx_pkt.payload));
    tx_pkt.icmpp_icmp.icmp_checksum = 0U;
    tx_pkt.icmpp_icmp.icmp_checksum = CONVERT_HTONS(compute_checksum(0U,
                                                                    (uint8_t *)&tx_pkt.icmpp_icmp,
                                                                    icmp_len));
    ip_total_len = (uint16_t)(sizeof(st_gmac_ip_t) + icmp_len);
    tx_pkt.icmpp_ip.ip_total_leng = CONVERT_HTONS(ip_total_len);
    tx_pkt.icmpp_ip.ip_checksum = 0U;
    tx_pkt.icmpp_ip.ip_checksum = CONVERT_HTONS(compute_checksum(0U,
                                                                (uint8_t *)&tx_pkt.icmpp_ip,
                                                                sizeof(st_gmac_ip_t)));
    tx_size = sizeof(st_gmac_mac_t) + ip_total_len;
    err = transmit_frame(&tx_pkt, tx_size);
    if (FSP_SUCCESS == err)
    {
        g_tx_count++;
        APP_PRINT("ICMP echo request sent successfully, seq=%lu\r\n", (unsigned long)(g_ping_seq - 1U));
    }
    else
    {
        APP_PRINT("ICMP echo request send failed, error=%d\r\n", err);
    }
}
#endif /* DEMO_SENDER_BOARD */

/***********************************************************************************************************************
 * @brief       This function processes received ARP packet.
 * @param[in]   port    Port number received.
 * @param[in]   p_buff  buffer of received Ethernet frame(ARP packet).
 * @retval      None.
 **********************************************************************************************************************/
static void incoming_arp(uint8_t port, uint8_t * p_buff)
{
    fsp_err_t                 err;
    st_gmac_arp_packet_t *    p_rx_arpp = (st_gmac_arp_packet_t *)p_buff; /* cast to st_gmac_arp_packet_t * */
    st_gmac_mac_t *           p_rx_mac  = &p_rx_arpp->arpp_mac;
    st_gmac_arp_t *           p_rx_arp  = &p_rx_arpp->arpp_arp;
    u_tx_packet_t             tx_packet;
    uint32_t                  target_ip;
    uint32_t                  tx_size;
    uint32_t                  sender_ip;

    sender_ip = ip_bytes_to_u32(p_rx_arp->arp_sender_ip);
    target_ip = ip_bytes_to_u32(p_rx_arp->arp_target_ip);

    /* Ignore our own ARP frames if they loop back through HPSW/RMAC  */
    if ((sender_ip == g_own_addr[0].ip_addr) ||
        mac_equal(p_rx_arp->arp_sender_mac, g_own_addr[0].mac_addr))
    {
        APP_PRINT("Ignore own ARP loop back frame\r\n");
        return;
    }

    g_arp_count++;

    /* When receive packet is ARP request */
    if (CONVERT_HTONS(1) == p_rx_arp->arp_opcode) /* Convert byte order of 16bits from host to network */
    {
        /* Get target IP address */
        target_ip = ((uint32_t)p_rx_arp->arp_target_ip[0] << 24) |
                    ((uint32_t)p_rx_arp->arp_target_ip[1] << 16) |
                    ((uint32_t)p_rx_arp->arp_target_ip[2] <<  8) |
                    ((uint32_t)p_rx_arp->arp_target_ip[3] <<  0);

        if (g_own_addr[port].ip_addr == target_ip)
        {
            APP_PRINT("ARP Request received\r\n");

            /* Found matching target IP address in own IP address list */
            st_gmac_mac_t *   p_tx_mac = &tx_packet.arpp.arpp_mac;
            st_gmac_arp_t *   p_tx_arp = &tx_packet.arpp.arpp_arp;

            /* Make MAC header */
            memcpy (p_tx_mac->mac_des_addr, p_rx_mac->mac_sur_addr, sizeof(p_rx_mac->mac_sur_addr));
            memcpy (p_tx_mac->mac_sur_addr, g_own_addr[port].mac_addr,  PING_MAC_ADDRESS_SIZE);
            p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_ARP);

            /* Make ARP header */
            p_tx_arp->arp_hardware_type = CONVERT_HTONS(1);
            p_tx_arp->arp_protocol_type = CONVERT_HTONS(0x800);     /* Protocol */
            p_tx_arp->arp_hardware_size = PING_MAC_ADDRESS_SIZE;    /* MAC size */
            p_tx_arp->arp_protocol_size = PING_IP_ADDRESS_SIZE;     /* IP size */
            p_tx_arp->arp_opcode        = CONVERT_HTONS(2);         /* Reply */
            memcpy (p_tx_arp->arp_sender_mac, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE);
            memcpy (p_tx_arp->arp_sender_ip , p_rx_arp->arp_target_ip , sizeof(p_rx_arp-> arp_target_ip));
            memcpy (p_tx_arp->arp_target_mac, p_rx_arp->arp_sender_mac, sizeof(p_rx_arp-> arp_sender_mac));
            memcpy (p_tx_arp->arp_target_ip , p_rx_arp->arp_sender_ip , sizeof(p_rx_arp->arp_sender_ip));

            tx_size = sizeof(st_gmac_arp_packet_t);

            /* Send ARP Reply packet */
            err = transmit_frame(&tx_packet, tx_size);
            if (FSP_SUCCESS == err)
            {
                g_tx_count++;
                g_arp_rep_tx_count++;
                APP_PRINT("ARP Reply sent successfully\r\n");
            }
            else
            {
                APP_PRINT("ARP Reply sent failed, error=%d\r\n", err);
            }
        }
    }
    else if (CONVERT_HTONS(2) ==  p_rx_arp->arp_opcode) /* ARP Reply */
    {
        APP_PRINT("ARP reply received: sender=%lu.%lu.%lu.%lu target=%lu.%lu.%lu.%lu\r\n",
                ((sender_ip >> 24) & 0xff),
                ((sender_ip >> 16) & 0xff),
                ((sender_ip >>  8) & 0xff),
                ((sender_ip >>  0) & 0xff),
                ((target_ip >> 24) & 0xff),
                ((target_ip >> 16) & 0xff),
                ((target_ip >>  8) & 0xff),
                ((target_ip >>  0) & 0xff));

        /*
         * Board A resolves Board B only from ARP reply:
         * sender IP must be peer IP
         * target IP must be my IP
         */
        if ((PEER_IP_ADDRESS == sender_ip) &&
            (g_own_addr[0].ip_addr == target_ip))
        {
            g_arp_rep_rx_count++;
#if defined(DEMO_SENDER_BOARD)
            g_eth_demo_state = ETH_DEMO_ICMP;
            APP_PRINT("Peer MAC learned\r\n"
                        "Enter '1' again to send ICMP Echo Request\r\n");
#endif /* DEMO_SENDER_BOARD */
        }
        else
        {
            APP_PRINT("ARP reply ignored due to sender/target mismatch\r\n");
        }
    }
    else
    {
        APP_PRINT("Unknown ARP opcode\r\n");
    }
}

/***********************************************************************************************************************
 * @@brief      This function processes received ICMP Echo Request packet and sends ICMP Echo Reply packet.
 * @param[IN]   port        Port number received.
 * @param[IN]   p_buff      Buffer pointer of received Ethernet packet(ICMP message).
 * @retval      None.
 **********************************************************************************************************************/
static void incoming_icmp(uint8_t port, uint8_t * p_buff)
{
    st_icmp_packet_t * p_rx_icmpp = (st_icmp_packet_t *)p_buff; /* cast to st_gmac_arp_packet_t * */
    st_gmac_mac_t * p_rx_mac      = &p_rx_icmpp->icmpp_mac;
    st_gmac_ip_t  * p_rx_ip       = &p_rx_icmpp->icmpp_ip;
    st_gmac_icmp_t * p_rx_icmp    = &p_rx_icmpp->icmpp_icmp;
    u_tx_packet_t tx_packet;

    fsp_err_t err;

    if (ICMP_TYPE_ECHO_REQUEST == p_rx_icmp->icmp_type)
    {
        /* When receive ICMP packet is Echo Request */
        st_gmac_mac_t *      p_tx_mac  = &tx_packet.icmpp.icmpp_mac;
        st_gmac_ip_t  *      p_tx_ip   = &tx_packet.icmpp.icmpp_ip;
        st_gmac_icmp_t *     p_tx_icmp = &tx_packet.icmpp.icmpp_icmp;
        uint32_t             data_size;
        uint32_t             tx_size;

        /* Make MAC header */
        memcpy (p_tx_mac->mac_des_addr, p_rx_mac->mac_sur_addr, sizeof(p_rx_mac->mac_sur_addr));
        memcpy (p_tx_mac->mac_sur_addr, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE);
        p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_IP);

        /* Make IP header */
        p_tx_ip->ip_ver         = p_rx_ip->ip_ver;
        p_tx_ip->ip_tos         = p_rx_ip->ip_tos;
        p_tx_ip->ip_total_leng  = p_rx_ip->ip_total_leng;
        p_tx_ip->ip_id          = CONVERT_HTONS(0); /* Convert byte order of 16bits from host to network */
        p_tx_ip->ip_offset      = CONVERT_HTONS(0); /* Convert byte order of 16bits from host to network */
        p_tx_ip->ip_ttl         = p_rx_ip->ip_ttl;
        p_tx_ip->ip_protocol    = p_rx_ip->ip_protocol;
        p_tx_ip->ip_checksum    = 0;

        memcpy (p_tx_ip->ip_des_addr, p_rx_ip->ip_sou_addr, sizeof(p_tx_ip->ip_des_addr));
        memcpy (p_tx_ip->ip_sou_addr, p_rx_ip->ip_des_addr, sizeof(p_tx_ip->ip_sou_addr));

        /* Convert byte order of 16bits from host to network */
        p_tx_ip->ip_checksum = CONVERT_HTONS(compute_checksum(0, (uint8_t *)p_tx_ip, sizeof(st_gmac_ip_t)));

        /* Make ICMP header */
        p_tx_icmp->icmp_type     = ICMP_TYPE_ECHO_REPLY;
        p_tx_icmp->icmp_code     = 0;
        p_tx_icmp->icmp_checksum = 0;

        p_tx_icmp->icmp_id  = p_rx_icmp->icmp_id;
        p_tx_icmp->icmp_seq = p_rx_icmp->icmp_seq;

        uint32_t ip_total = (uint32_t)CONVERT_HTONS(p_rx_ip->ip_total_leng);

        /* Sanity-check ip_total_leng before using it */
        if (ip_total < (uint32_t)(sizeof(st_gmac_ip_t) + sizeof(st_gmac_icmp_t)))
        {
            return;  /* Malformed packet */
        }

        data_size = ip_total - (uint32_t)sizeof(st_gmac_ip_t) - (uint32_t)sizeof(st_gmac_icmp_t);

        uint32_t icmp_payload_offset = (uint32_t)sizeof(st_gmac_mac_t) +
                                        (uint32_t)sizeof(st_gmac_ip_t) +
                                        (uint32_t)sizeof(st_gmac_icmp_t);

        /* Bound-check: ensure copy fits within tx_packet */
        uint32_t max_payload = sizeof(tx_packet) - icmp_payload_offset;
        if (data_size > max_payload)
        {
            APP_PRINT("ICMP payload too large (%lu), truncating to %lu\r\n",
                    (unsigned long)data_size, (unsigned long)max_payload);
            data_size = max_payload;
        }


        memcpy(&tx_packet.data[icmp_payload_offset], &p_buff[icmp_payload_offset], data_size);

        p_tx_icmp->icmp_checksum = CONVERT_HTONS(compute_checksum( /* cast CONVERT_HTONS */
            0, (uint8_t *)p_tx_icmp, sizeof(st_gmac_icmp_t) + data_size));

        tx_size = icmp_payload_offset + data_size;
        g_icmp_req_count++;
        APP_PRINT("ICMP Echo Request received\r\n");

        /* Send ICMP Echo Reply packet */
        err = transmit_frame(&tx_packet, tx_size);
        if (FSP_SUCCESS == err)
        {
            g_tx_count++;
            APP_PRINT("ICMP Echo Reply sent successfully\r\n");
        }
        else
        {
            APP_PRINT("ICMP Echo Reply sent failed\r\n");
        }
    }
    else if (ICMP_TYPE_ECHO_REPLY == p_rx_icmp->icmp_type)
    {
        g_icmp_rep_count++;
#if defined(DEMO_SENDER_BOARD)
        g_eth_demo_state = ETH_DEMO_ICMP;
        APP_PRINT("ICMP Echo Reply received\r\n"
                    "Ping successful. Enter '1' to send next ICMP Echo Request\r\n");
#endif /* DEMO_SENDER_BOARD */
    }
    else
    {
        /* Do nothing */
    }
}

/***********************************************************************************************************************
 * @brief       This function processes Received IP packet.
 * @param[IN]   port        Port number received.
 * @param[IN]   p_buff      Buffer pointer of received Ethernet frame(IP packet).
 * @retval      None.
 **********************************************************************************************************************/
static void incoming_ip(uint8_t port, uint8_t * p_buff)
{
    st_icmp_packet_t * p_rx_icmpp = (st_icmp_packet_t *)p_buff;  /* cast to st_gmac_icmp_packet_t * */
    st_gmac_ip_t * p_rx_ip = &p_rx_icmpp->icmpp_ip;
    uint32_t ip_des_addr;

    /* Get destination IP address */
    ip_des_addr = ((uint32_t)p_rx_ip->ip_des_addr[0] << 24) |
                    ((uint32_t)p_rx_ip->ip_des_addr[1] << 16) |
                    ((uint32_t)p_rx_ip->ip_des_addr[2] <<  8) |
                    ((uint32_t)p_rx_ip->ip_des_addr[3] <<  0);

    if (g_own_addr[port].ip_addr == ip_des_addr)
    {
        /* Found matching destination IP address in own IP address list */
        if (IP_PROTOCOL_ICMP == p_rx_ip->ip_protocol)
        {
            /* When receive IP packet is ICMP */
            incoming_icmp(port, p_buff);
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function processes received Ethernet frame.
 * @param[IN]   port        Port number received.
 * @param[IN]   p_buff      Buffer pointer of received Ethernet frame.
 * @retval      None.
 **********************************************************************************************************************/
static void incoming_mac(uint8_t port, uint8_t * p_buff)
{
    st_gmac_mac_t *   p_mac = (st_gmac_mac_t *)p_buff;  /* cast to st_gmac_mac_t * */

    if ((0 == memcmp(p_mac->mac_des_addr, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE)) ||
    (0 == memcmp(p_mac->mac_des_addr, g_broadcast_mac_addr, PING_MAC_ADDRESS_SIZE)))
    {
        /* When destination MAC Address is own MAC address, broadcast address or multicast address */
        switch (p_mac->mac_type)
        {
            case (CONVERT_HTONS(MAC_TYPE_ARP)):   /* ARP packet */
            {
                incoming_arp(port, p_buff);
                break;
            }
            case (CONVERT_HTONS (MAC_TYPE_IP)):   /* IP packet */
                incoming_ip(port, p_buff);
                break;
            default:
                break;
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function starts process user input and ping between two boards.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void ether_hpsw_example_start(void)
{
    uint32_t length_byte = 0U;
    fsp_err_t err = FSP_SUCCESS;
    unsigned char rByte[BUF_SIZE] = {INITIAL_VALUE};
    uint32_t read_data = RESET_VALUE;
    hpsw_status_t hpsw_status;
    hpsw_counter_t hpsw_counter_A;
    hpsw_counter_t hpsw_counter_B;
    hpsw_counter_t hpsw_counter_C;
    ethsw_mac_table_entry_addr_t mac_entry_addr = {0};
    ethsw_mac_table_entry_info_t mac_entry_info = {0};

    memcpy(g_own_addr[0].mac_addr, g_mac_port0, sizeof(g_mac_port0));
    g_rx_enabled = true;

    mac_entry_addr.mac_addr = &g_own_addr[0].mac_addr;
    mac_entry_info.port_mask.mask = (1U << 3);   /* EHTSW management port */

    /* Set own MAC address in table entry */
    err = R_ETHSW_MacTableSet(g_ethsw0.p_ctrl, &mac_entry_addr, &mac_entry_info);
    if (FSP_SUCCESS == err)
    {
        APP_PRINT("\r\nOwn MAC registered to management port 3 successfully\r\n");
    }
    else
    {
        APP_PRINT("R_ETHSW_MacTableSet API for own MAC registration failed\r\n");
        APP_ERR_TRAP(err);
    }

    mac_entry_addr.mac_addr = (ethsw_mac_addr_t *) g_peer_mac;
    mac_entry_info.port_mask.mask = 0x4;    /* EHTSW port 2 */

    /* Set peer MAC address in table entry */
    err = R_ETHSW_MacTableSet(g_ethsw0.p_ctrl, &mac_entry_addr, &mac_entry_info);
    if (FSP_SUCCESS == err)
    {
        APP_PRINT("Peer MAC registered to external port 2 successfully\r\n");
    }
    else
    {
        APP_PRINT("R_ETHSW_MacTableSet API for peer MAC registration failed\r\n");
        APP_ERR_TRAP(err);
    }

    /* Print HPSW menu */
    APP_PRINT(HPSW_MENU);

    while (true)
    {
        err = R_GMAC_B_LinkProcess(P_GMAC_CTRL);       /* Link up/down processing */
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("R_GMAC_B_LinkProcess API failed\r\n");
            APP_ERR_TRAP(err);
        }
        check_link_status();     /* Check link status */

        /* Process input only when user has provided one */
        if (APP_CHECK_DATA)
        {
            /* Clear the input buffer before reading */
            memset(rByte, INITIAL_VALUE, sizeof(rByte));

            /* Read First byte of data provided by user */
            APP_READ(rByte);

            /* Accept only a single character command */
            if ((rByte[0] == '\0') || ((rByte[1] != '\0') &&
               (rByte[1] != '\r') && (rByte[1] != '\n')))
            {
                APP_PRINT("\r\nInvalid menu option selected\r\n");
                APP_PRINT(HPSW_MENU);
                continue;
            }

            /* Conversion from input string to integer value */
            read_data = (uint32_t) (atoi((char *)rByte));

            switch (read_data)
            {
                case ETHERNET_PING:
                {
                    APP_PRINT("\r\n");
#if defined(DEMO_SENDER_BOARD)
                    switch (g_eth_demo_state)
                    {
                        case ETH_DEMO_IDLE:
                        {
                            APP_PRINT("Sending ARP request...\r\n");
                            send_arp_request(0U);
                            break;
                        }
                        case ETH_DEMO_ICMP:
                        {
                            APP_PRINT("Sending ICMP Echo Request...\r\n");
                            ping_icmp();
                            break;
                        }
                        default:
                            break;
                    }
#else
                    APP_PRINT("Waiting for ARP/ICMP requests\r\n");
#endif /* DEMO_SENDER_BOARD */
                    break;
                }
                case HPSW_GET_COUNTER:
                    err = get_counter_status(&hpsw_status, &hpsw_counter_A, &hpsw_counter_B, &hpsw_counter_C);
                    if (FSP_SUCCESS == err)
                    {
                        APP_PRINT("HPSW Port A link status= %s\r\n", hpsw_status.link_port_A ? "UP" : "DOWN");
                        APP_PRINT("HPSW Port B link status= %s\r\n", hpsw_status.link_port_B ? "UP" : "DOWN");
                        APP_PRINT("HPSW Port C link status= %s\r\n", hpsw_status.link_port_C ? "UP" : "DOWN");
                        APP_PRINT("HPSW Port A RX= %lu TX= %lu\n", hpsw_counter_A.RX_counter, hpsw_counter_A.TX_counter);
                        APP_PRINT("HPSW Port B RX= %lu TX= %lu\n", hpsw_counter_B.RX_counter, hpsw_counter_B.TX_counter);
                        APP_PRINT("HPSW Port C RX= %lu TX= %lu\n", hpsw_counter_C.RX_counter, hpsw_counter_C.TX_counter);
                    }
                    else
                    {
                        APP_PRINT("get_counter_status failed\n");
                    }
                    break;
                case HPSW_EXIT:
                    APP_PRINT("Exit example project\r\n");
                    g_rx_enabled = false;
#if defined(DEMO_SENDER_BOARD)
                    g_eth_demo_state = ETH_DEMO_IDLE;
#endif /* DEMO_SENDER_BOARD */
                    err = deinit_hpsw_module();
                    if (FSP_SUCCESS != err)
                    {
                        APP_PRINT("deinit_hpsw_module failed\r\n");
                        APP_ERR_TRAP(err);
                    }
                    err = deinit_gmac_module();
                    if (FSP_SUCCESS != err)
                    {
                        APP_PRINT("deinit_gmac_module failed\r\n");
                        APP_ERR_TRAP(err);
                    }
                    return; /* Exit program */
                default:
                {
                    APP_PRINT("\r\nInvalid menu option selected\r\n");
                    break;
                }
            }
            read_data = RESET_VALUE;

            /* Reset buffer */
            memset(rByte, INITIAL_VALUE, BUF_SIZE);

            /* Print Menu option of HPSW */
            APP_PRINT(HPSW_MENU);
        }

        /* Read the GMAC Ethernet frame */
        if (true == g_rx_enabled)
        {
            length_byte = sizeof(g_gmac_rx_frame);
            err = R_GMAC_B_Read(P_GMAC_CTRL, &g_gmac_rx_frame, &length_byte);
            if (FSP_SUCCESS == err)
            {
                /* Receive and process Ethernet frames */
                /* Send ARP reply when receiving ARP request */
                /* Increase counter and print ARP reply received */
                /* Send ICMP reply when receiving ICMP request */
                /* Increase counter and print ICMP reply received */
                g_rx_count++;
                incoming_mac(0U, (uint8_t *)&g_gmac_rx_frame);
            }
            else
            {
                if ((FSP_ERR_ETHER_ERROR_NO_DATA != err) &&
                    (FSP_ERR_ETHER_ERROR_LINK != err) &&
                    (FSP_ERR_ETHER_ERROR_MAGIC_PACKET_MODE != err) &&
                    (FSP_ERR_ETHER_ERROR_TRANSMIT_BUFFER_FULL != err))
                {
                    APP_PRINT("R_GMAC_B_Read API failed\r\n");
                }
            }
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function demonstrates Ethernet HPSW.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void eth_hpsw_example_demo(void)
{
    fsp_err_t err;
    hpsw_cfg_t hpsw_cfg;
    bool enable = HPSW_ENABLE;
    fsp_pack_version_t version = {RESET_VALUE};

    /* Get the FSP version */
    R_FSP_VersionGet(&version);

    /* Example project information printed on the console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor,
            version.version_id_b.patch);

    /* Print the EP information on the serial console */
    APP_PRINT (EP_INFO);

    /* Initialize struct */
    memset(&hpsw_cfg, 0x0, sizeof(hpsw_cfg_t));

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SYSTEM);
    {
        R_SSC->SSTPCR7_b.HPSW_REQ = 0x0;
        while (R_SSC->SSTPCR7 & 0x20000);
    }
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SYSTEM);

    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_RESET);
    {
        /* Power on ETHSS */
        R_BSP_MODULE_START(FSP_IP_ETHSS, 0);

        /* Release ETHSS modules */
        R_BSP_ModuleResetDisable(BSP_MODULE_RESET_ESC_ETH_SUBSYSTEM);
    }
    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_RESET);

    subsystem_protection_disable();
    {
        /* Select timer signal for HPSW */
        /* When using the HPSW adjustable clock in slave mode, HPSW_SEL bits must be set to 0x4 */
        R_ETHSS->TMSEL_b.HPSW_SEL = 0x4;

        err = initialize_hpsw(&hpsw_cfg, enable);
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("\r\n** initialize_hpsw failed **\r\n");
            APP_ERR_TRAP(err);
        }
    }
    subsystem_protection_enable();

    err = R_GMAC_B_Open(P_GMAC_CTRL, P_GMAC_CFG);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\n** R_GMAC_B_Open API failed **\r\n");
        err = deinit_hpsw_module();
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("\r\n** deinit_hpsw_module failed **\r\n");
        }
        APP_ERR_TRAP(err);
    }

    ether_hpsw_example_start();
    return;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_hpsw_ep)
 **********************************************************************************************************************/
