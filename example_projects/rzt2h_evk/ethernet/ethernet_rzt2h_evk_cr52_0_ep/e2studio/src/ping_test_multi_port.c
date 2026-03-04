/***********************************************************************************************************************
 * File Name    : ping_test_multi_port.c
 * Description  : Contains data structures and functions used in ping_test_multi_port.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "hal_data.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define NETWORK_ADDRESS         (255UL<<24 | 255UL<<16 | 255UL<<8 | 0UL)    /* 255.255.255.0 */

#define OWN_IP_ADDRESS_PORT0    (192UL<<24 | 168UL<<16 | 10UL<<8 | 100UL)   /* 192.168.10.100 */
#define OWN_IP_ADDRESS_PORT1    (192UL<<24 | 168UL<<16 | 20UL<<8 | 100UL)   /* 192.168.20.100 */
#define OWN_IP_ADDRESS_PORT2    (192UL<<24 | 168UL<<16 | 30UL<<8 | 100UL)   /* 192.168.30.100 */

static const uint8_t g_mac_port0[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
static const uint8_t g_mac_port1[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
static const uint8_t g_mac_port2[6] = {0x00, 0x33, 0x33, 0x33, 0x33, 0x33};

#define GMAC_BUFFER_SIZE            (2048)              /* Transmission Buffer size */

#define MAC_TYPE_ARP                (0x0806)            /* Ethernet type for ARP */
#define MAC_TYPE_IP                 (0x0800)            /* Ethernet type for IP */
#define IP_PROTOCALICMP             (1)

#define ICMP_TYPE_ECHO_REQUEST      (8)                 /* ICMP type from ECHO Request */
#define ICMP_TYPE_ECHO_REPLY        (0)                 /* ICMP type from ECHO Reply */

#define PING_MAC_ADDRESS_SIZE       (6)                 /* MAC Address Size */
#define PING_IP_ADDRESS_SIZE        (4)                 /* IP Address Size */
#define PING_MANAGEMENT_TAG_SIZE    (8)

#define P_GMAC_CTRL                 (g_ether0.p_ctrl)   /* Pointer of control block for GMAC driver */
#define P_GMAC_CFG                  (g_ether0.p_cfg)    /* Pointer of configuration parameters for GMAC driver */

#define PING_PORT_COUNT             (3)                 /* Count of port */

/* Convert byte order of 32bits from network to host */
#define CONVERT_NTOHL(x)  \
    (uint32_t)((uint32_t)(((x)>>24)&(0xffUL))\
    |(uint32_t)(((x)>>8)&(0xff00UL))\
    |(uint32_t)(((x & 0xffffffUL)<<8)&(0xff0000UL))\
    |(uint32_t)(((x & 0xffUL)<<24)&(0xff000000UL)))

/* Convert byte order of 16bits from network to host */
#define CONVERT_NTOHS(x) \
        (uint16_t)((uint16_t)(((x)>>8)&0xffU) \
        |((uint16_t)(((x)&0xffU)<<8))) \

/* Convert byte order of 32bits from host to network */
#define CONVERT_HTONL(x)            CONVERT_NTOHL(x)

/* Convert byte order of 16bits from host to network */
#define CONVERT_HTONS(x)            CONVERT_NTOHS(x)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/* ethernet frame format struct */
typedef struct st_gmac_frame
{
    uint8_t     des_address[PING_MAC_ADDRESS_SIZE];     /* Destination MAC Address */
    uint8_t     sur_address[PING_MAC_ADDRESS_SIZE];     /* Source MAC Address */
    uint16_t    type;                                   /* Ethernet Type */
    uint8_t     data[2000];
} st_gmac_frame_t;

/* ethernet frame header format struct */
typedef struct st_gmac_mac
{
    uint8_t     mac_des_addr[PING_MAC_ADDRESS_SIZE];    /* Destination MAC Address */
    uint8_t     mac_sur_addr[PING_MAC_ADDRESS_SIZE];    /* Source MAC Address */
    uint16_t    mac_type;                               /* Ethernet Type */
} st_gmac_mac_t;

/* arp packet format struct     */
typedef struct st_gmac_arp
{
    uint16_t    arp_hardware_type;                      /* Hardware Type */
    uint16_t    arp_protocol_type;                      /* Protocol Type */
    uint8_t     arp_hardware_size;                      /* Hardware size */
    uint8_t     arp_protocol_size;                      /* Protocol Size */
    uint16_t    arp_opcode;                             /* OpCode */
    uint8_t     arp_sender_MAC[PING_MAC_ADDRESS_SIZE];  /* Sender MAC address */
    uint8_t     arp_sender_IP[PING_IP_ADDRESS_SIZE];    /* Sender IP address */
    uint8_t     arp_target_MAC[PING_MAC_ADDRESS_SIZE];  /* Target MAC address */
    uint8_t     arp_target_IP[PING_IP_ADDRESS_SIZE];    /* Target IP address */
} st_gmac_arp_t;

/* ip packet format struct      */
typedef struct st_gmac_ip
{
    uint8_t     ip_ver;                                 /* Version / Internet Header Length */
    uint8_t     ip_tos;                                 /* Type of Service */
    uint16_t    ip_total_leng;                          /* Total length */
    uint16_t    ip_id;                                  /* Identification */
    uint16_t    ip_offset;                              /* IP Flags / Fragment Offset */
    uint8_t     ip_ttl;                                 /* Time to live */
    uint8_t     ip_protocol;                            /* Protocol */
    uint16_t    ip_checksum;                            /* Header Checksum */
    uint8_t     ip_sou_addr[PING_IP_ADDRESS_SIZE];      /* Source Address */
    uint8_t     ip_des_addr[PING_IP_ADDRESS_SIZE];      /* Destination address */
} st_gmac_ip_t;

/* icmp message format struct   */
typedef struct st_gmac_icmp
{
    uint8_t     icmp_type;              /* Type (Echo Reply:0, Echo Request:8) */
    uint8_t     icmp_code;              /* Code (Echo Reply:0, Echo Request:0) */
    uint16_t    icmp_checksum;          /* Checksum */
    uint16_t    icmp_Id;                /* Identifier */
    uint16_t    icmp_Seq;               /* Sequence number */
} st_gmac_icmp_t;

/* arp format struct            */
typedef struct st_arp_packet
{
    st_gmac_mac_t     arpp_mac;            /* Ethernet Frame Header */
    st_gmac_arp_t     arpp_arp;            /* ARP Packet Header */
} st_gmac_arp_packet_t;

/* icmp format struct           */
typedef struct st_icmp_packet
{
    st_gmac_mac_t     icmpp_mac;           /* Ethernet Frame Header */
    st_gmac_ip_t         icmpp_ip;            /* IP Packet Header */
    st_gmac_icmp_t       icmpp_icmp;          /* ICMP Packet Header */
} st_gmac_icmp_packet_t;

/* own ip address struct */
typedef struct st_own_addr
{
    uint8_t         mac_addr[PING_MAC_ADDRESS_SIZE];    /* MAC Address */
    uint32_t        ip_addr;                            /* IP address */
    uint32_t        ip_mask;                            /* IP Mask */
} st_gmac_own_addr_t;

/***********************************************************************************************************************
 * Extern global variables
 **********************************************************************************************************************/
extern const    ether_instance_t g_ether0;

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
void test_main (void);
void gmac_callback (ether_callback_args_t * const p_arg);

static uint16_t compute_checksum (uint32_t pseudo_sum, uint8_t * p_buf, uint32_t size);

static void incoming_arp (uint8_t port, uint8_t * p_buff);
static void incoming_icmp (uint8_t port, uint8_t * p_buff);
static void incoming_ip (uint8_t port, uint8_t * p_buff);
static void incoming_mac (uint8_t port, uint8_t * p_buff);
static void check_link_status (void);
static void test_ping (void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
uint8_t g_broadcast_MAC_addr[PING_MAC_ADDRESS_SIZE]   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static uint8_t      g_link_change = 0;      /* Link change (bit0:port0, bit1:port1, bit2:port2) */
static uint8_t      g_link_status = 0;      /* Link status (bit0:port0, bit1:port1, bit2:port2) */

static st_gmac_frame_t   g_gmac_rx_frame = {0};

/* Own IP address table (IP address, Net mask) */
static st_gmac_own_addr_t g_own_addr[] =
{
    { {g_mac_port0[0], g_mac_port0[1], g_mac_port0[2], g_mac_port0[3], g_mac_port0[4], g_mac_port0[5]},
        OWN_IP_ADDRESS_PORT0, NETWORK_ADDRESS },
    { {g_mac_port1[0], g_mac_port1[1], g_mac_port1[2], g_mac_port1[3], g_mac_port1[4], g_mac_port1[5]},
        OWN_IP_ADDRESS_PORT1, NETWORK_ADDRESS },
    { {g_mac_port2[0], g_mac_port2[1], g_mac_port2[2], g_mac_port2[3], g_mac_port2[4], g_mac_port2[5]},
        OWN_IP_ADDRESS_PORT2, NETWORK_ADDRESS },
    {{0}, 0, 0}
};

/* Transmission Packet Buffer */
static union
{
    st_gmac_arp_packet_t   arpp;
    st_gmac_icmp_packet_t  icmpp;
    uint8_t                data[(GMAC_BUFFER_SIZE-sizeof(st_gmac_arp_packet_t))-sizeof(st_gmac_icmp_packet_t)];
} u_tx_packet_t;

ethsw_specific_tag_t g_specific_tag;        /* Management port specific frame tag */
uint32_t             g_specific_tag_id;     /* Management port specific frame tag ID */


/***********************************************************************************************************************
 * @fn          transmit_frame
 * @brief       Transmit frame
 * @param[in]   port        Transmit port number
 * @param[in]   tx_buff     Transmit buffer
 * @param[in]   tx_size     Transmit size
 * @retval      return value from R_GMAC_Write()
 **********************************************************************************************************************/
static fsp_err_t transmit_frame(uint8_t port, void * tx_buff, uint32_t tx_size)
{
    fsp_err_t       err;
    uint32_t        buf_tx_size = tx_size +PING_MANAGEMENT_TAG_SIZE;
    uint8_t         buf_tx[buf_tx_size];

    memcpy(buf_tx,tx_buff,buf_tx_size);

    st_gmac_frame_t * p_tx_frame = (st_gmac_frame_t *)buf_tx; /* cast to st_gmac_frame_t * */

    memmove(((uint8_t *)&p_tx_frame->type + PING_MANAGEMENT_TAG_SIZE), /* cast to uint8_t* */
                    (uint8_t *)&p_tx_frame->type,                      /* cast to uint8_t* */
                    (tx_size - (PING_MAC_ADDRESS_SIZE *2)));

    p_tx_frame->type    = CONVERT_HTONS(g_specific_tag_id); /* Convert byte order of 16bits from host to network */
    p_tx_frame->data[0] = 0;                       /* ControlData[b15:b8] */
    p_tx_frame->data[1] = 1;                       /* ControlData[b7:b0] */
    p_tx_frame->data[2] = 0;                       /* ControlData2[b31:b24] */
    p_tx_frame->data[3] = 0;                       /* ControlData2[b23:b16] */
    p_tx_frame->data[4] = 0;                       /* ControlData2[b15:b8] */
    p_tx_frame->data[5] = (uint8_t)(1 << port);    /* ControlData2[b7:b0] */

    tx_size += PING_MANAGEMENT_TAG_SIZE;

    err = R_GMAC_B_Write(P_GMAC_CTRL, buf_tx, tx_size);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("R_GMAC_Write()=%d port=%d tx_size=%d\n", err, port, tx_size);
        APP_ERR_TRAP(err);
    }
    return err;
}
/*****************************************************************************************************************
 End of function transmit_frame
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          compute_checksum
 * @brief       Compute checksum
 * @param[in]   pseudo_sum  Pseudo checksum
 * @param[in]   p_buf       Buffer pointer to calculate checksum
 * @param[in]   size        Buffer size to calculate checksum
 * @retval      Calculated checksum value
 **********************************************************************************************************************/
static uint16_t compute_checksum(uint32_t pseudo_sum, uint8_t *p_buf, uint32_t size)
{
    uint32_t    sum = pseudo_sum;
    uint16_t    val;
    uint16_t    val_l;

    while (size & (~1U))
    {
        val   = (uint16_t)(p_buf[0] << 8);  /* Conversion to 16-bit integer type */
        val_l = (uint16_t)p_buf[1];         /* Conversion to 16-bit integer type */
        val   = val | val_l;

        sum   += val;
        p_buf += 2;
        size  -= 2;
    }

    if (size & 1)
    {
        sum += (uint16_t)((*p_buf) << 8);   /* Conversion to 16-bit integer type */
    }

    sum = (sum & 0xffff) + (sum >> 16);     /* add overflow counts */

    return (uint16_t)(~sum);                /* Conversion to 16-bit integer type */
}
/*****************************************************************************************************************
 End of function compute_checksum
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          incoming_arp
 * @brief       Received ARP packet processing
 * @param[in]   port        Port number received
 * @param[in]   p_buff      buffer of received ethernet frame(arp packet)
 * @retval      none
 **********************************************************************************************************************/
static void incoming_arp(uint8_t port, uint8_t* p_buff)
{
    st_gmac_arp_packet_t *    p_rx_arpp = (st_gmac_arp_packet_t *)p_buff; /* cast to st_gmac_arp_packet_t * */
    st_gmac_mac_t *           p_rx_mac  = &p_rx_arpp->arpp_mac;
    st_gmac_arp_t *           p_rx_arp  = &p_rx_arpp->arpp_arp;
    uint32_t                  target_ip;

    if (CONVERT_HTONS(1) == p_rx_arp->arp_opcode) /* Convert byte order of 16bits from host to network */
    {
        /* When receive packet is ARP request */

        /* Get target IP address */
        target_ip = ((uint32_t)p_rx_arp->arp_target_IP[0] << 24)
                |   ((uint32_t)p_rx_arp->arp_target_IP[1] << 16)   /* cast to uint32_t */
                |   ((uint32_t)p_rx_arp->arp_target_IP[2] <<  8)   /* cast to uint32_t */
                |   ((uint32_t)p_rx_arp->arp_target_IP[3] <<  0);  /* cast to uint32_t */

        if (g_own_addr[port].ip_addr == target_ip)
        {
            /* Found matching target IP address in own IP address list */
            st_gmac_mac_t *   p_tx_mac = &u_tx_packet_t.arpp.arpp_mac;
            st_gmac_arp_t *   p_tx_arp = &u_tx_packet_t.arpp.arpp_arp;

            uint32_t        tx_size;

            /* make MAC header */
            memcpy (p_tx_mac->mac_des_addr, p_rx_mac->mac_sur_addr, sizeof(p_rx_mac->mac_sur_addr));
            memcpy (p_tx_mac->mac_sur_addr, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE);
            p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_ARP);

            /* make arp header */
            p_tx_arp->arp_hardware_type = CONVERT_HTONS(1);
            p_tx_arp->arp_protocol_type = CONVERT_HTONS(0x800); /* Convert byte order of 16bits from host to network */
            p_tx_arp->arp_hardware_size = PING_MAC_ADDRESS_SIZE;    /* MAC size */
            p_tx_arp->arp_protocol_size = PING_IP_ADDRESS_SIZE;     /* IP size */
            p_tx_arp->arp_opcode        = CONVERT_HTONS(2); /* replay */
            memcpy (p_tx_arp->arp_sender_MAC, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE);
            memcpy (p_tx_arp->arp_sender_IP , p_rx_arp->arp_target_IP , sizeof(p_rx_arp->arp_target_IP));
            memcpy (p_tx_arp->arp_target_MAC, p_rx_arp->arp_sender_MAC, sizeof(p_rx_arp->arp_sender_MAC));
            memcpy (p_tx_arp->arp_target_IP , p_rx_arp->arp_sender_IP , sizeof(p_rx_arp->arp_sender_IP));

            tx_size = sizeof(st_gmac_arp_packet_t);

            APP_PRINT("arp_reply  port=%d tx_size=%d IP=%d:%d:%d:%d\n",
                    port, tx_size,
                    p_tx_arp->arp_sender_IP[0], p_tx_arp->arp_sender_IP[1],
                    p_tx_arp->arp_sender_IP[2], p_tx_arp->arp_sender_IP[3]);

            /* send arp packet */
            transmit_frame(port, &u_tx_packet_t, tx_size);
        }
    }
}
/*****************************************************************************************************************
 End of function incoming_arp
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          incoming_icmp
 * @@brief      Received icmp message processing (when echo request received then send echo reply)
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame(icmp message)
 * @retval      none
 **********************************************************************************************************************/
static void incoming_icmp(uint8_t port, uint8_t* p_buff)
{
    st_gmac_icmp_packet_t * p_rx_icmpp = (st_gmac_icmp_packet_t *)p_buff; /* cast to st_gmac_arp_packet_t * */
    st_gmac_mac_t *         p_rx_mac   = &p_rx_icmpp->icmpp_mac;
    st_gmac_ip_t  *         p_rx_ip    = &p_rx_icmpp->icmpp_ip;
    st_gmac_icmp_t *        p_rx_icmp  = &p_rx_icmpp->icmpp_icmp;

    if (ICMP_TYPE_ECHO_REQUEST == p_rx_icmp->icmp_type)
    {
        /* When receive ARP packet is ECHO request */
        st_gmac_mac_t *      p_tx_mac  = &u_tx_packet_t.icmpp.icmpp_mac;
        st_gmac_ip_t  *      p_tx_ip   = &u_tx_packet_t.icmpp.icmpp_ip;
        st_gmac_icmp_t *     p_tx_icmp = &u_tx_packet_t.icmpp.icmpp_icmp;
        uint32_t             data_size;
        uint32_t             tx_size;

        /* make MAC header */
        memcpy (p_tx_mac->mac_des_addr, p_rx_mac->mac_sur_addr, sizeof(p_rx_mac->mac_sur_addr));
        memcpy (p_tx_mac->mac_sur_addr, g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE);
        p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_IP);

        /* make IP header */
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

        /* make ICMP header */
        p_tx_icmp->icmp_type     = ICMP_TYPE_ECHO_REPLY;
        p_tx_icmp->icmp_code     = 0;
        p_tx_icmp->icmp_checksum = 0;

        p_tx_icmp->icmp_Id  = p_rx_icmp->icmp_Id;
        p_tx_icmp->icmp_Seq = p_rx_icmp->icmp_Seq;

        /* cast to CONVERT_HTONS */
        data_size = (CONVERT_HTONS(p_rx_ip->ip_total_leng) - sizeof(st_gmac_ip_t)) - sizeof(st_gmac_icmp_t);

        memcpy (&u_tx_packet_t.data[sizeof(st_gmac_icmp_packet_t)], &p_buff[sizeof(st_gmac_icmp_packet_t)], data_size);

        p_tx_icmp->icmp_checksum = CONVERT_HTONS(compute_checksum( /* cast CONVERT_HTONS */
            0, (uint8_t *)p_tx_icmp, sizeof(st_gmac_icmp_t) + data_size));

        tx_size = sizeof(st_gmac_icmp_packet_t) + data_size;

        APP_PRINT("ping_reply port=%d tx_size=%d ip=%d:%d:%d:%d seq=%d\n",
                    port, tx_size,
                    p_tx_ip->ip_sou_addr[0], p_tx_ip->ip_sou_addr[1], p_tx_ip->ip_sou_addr[2], p_tx_ip->ip_sou_addr[3],
                    CONVERT_NTOHS(p_tx_icmp->icmp_Seq));

        /* send ICMP packet */
        transmit_frame(port, &u_tx_packet_t, tx_size);
    }
    else
    {
        APP_PRINT ("incoming_icmp: Unknown type\n");
    }
}
/*****************************************************************************************************************
 End of function incoming_icmp
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          incoming_ip
 * @brief       Received ip packet processing
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame(ip packet)
 * @retval      none
 **********************************************************************************************************************/
static void incoming_ip(uint8_t port, uint8_t* p_buff)
{
    st_gmac_icmp_packet_t *   p_rx_icmpp = (st_gmac_icmp_packet_t *)p_buff;  /* cast to st_gmac_icmp_packet_t * */
    st_gmac_ip_t *            p_rx_ip    = &p_rx_icmpp->icmpp_ip;
    uint32_t                  ip_des_addr;

    /* Get destination IP address */
    ip_des_addr = ((uint32_t)p_rx_ip->ip_des_addr[0] << 24)
                | ((uint32_t)p_rx_ip->ip_des_addr[1] << 16)   /* cast to uint32_t */
                | ((uint32_t)p_rx_ip->ip_des_addr[2] <<  8)   /* cast to uint32_t */
                | ((uint32_t)p_rx_ip->ip_des_addr[3] <<  0);  /* cast to uint32_t */

    if (g_own_addr[port].ip_addr == ip_des_addr)
    {
        /* Found matching destination IP address in own IP address list */
        if (IP_PROTOCALICMP == p_rx_ip->ip_protocol)
        {
            /* When receive IP packet is ICMP */
            incoming_icmp (port, p_buff);
        }
    }
}
/*****************************************************************************************************************
 End of function incoming_ip
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          incoming_mac
 * @brief       Received ethernet frame processing
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame
 * @retval      none
 **********************************************************************************************************************/
static void incoming_mac(uint8_t port, uint8_t* p_buff)
{
    st_gmac_mac_t *   p_mac = (st_gmac_mac_t *)p_buff;  /* cast to st_gmac_mac_t * */

    if ((0 == memcmp(p_mac->mac_des_addr,
        g_own_addr[port].mac_addr, PING_MAC_ADDRESS_SIZE))
    || (0 == memcmp(p_mac->mac_des_addr,
        g_broadcast_MAC_addr, PING_MAC_ADDRESS_SIZE)))
    {
        /* When destination MAC Address is own MAC address, broadcast addrss or multicast address */
        switch (p_mac->mac_type)
        {
            case (CONVERT_HTONS(MAC_TYPE_ARP)):   /* ARP packet */
            {
                incoming_arp (port, p_buff);
                break;
            }

            case (CONVERT_HTONS (MAC_TYPE_IP)):   /* IP packet */
                incoming_ip (port, p_buff);
                break;

            default:
                break;
        }
    }
}
/*****************************************************************************************************************
 End of function incoming_mac
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          check_link_status
 * @brief       Check Link status. Display when link status changes.
 * @retval      none
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

        for (port = 0; port < PING_PORT_COUNT; port++)
        {
            err = R_GMAC_B_GetLinkStatus (P_GMAC_CTRL, port, &port_status);
            if (FSP_SUCCESS != err)
            {
                /* An error has occurred */
                APP_PRINT("R_GMAC_GetLinkStatus()=%d \n", err);
                APP_ERR_TRAP(err);
                break;
            }

            if (GMAC_LINK_STATUS_DOWN != port_status)
            {
                /* Set link up */
                g_link_status |= (uint8_t)(1U << port);
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
        for (port = 0; port < PING_PORT_COUNT; port++)
        {
            port_bit = (uint8_t)(1U << port);   /* cast to uint8_t */

            if (g_link_change & port_bit)
            {
                /* Link status changed */
                g_link_change &= (uint8_t)(~port_bit);  /* change bit clear */

                if (g_link_status & port_bit)
                {
                    /* Changed to Link-up */
                    APP_PRINT("Port %d link is up\n", port);
                }
                else
                {
                    /* Changed to Link-down */
                    APP_PRINT("Port %d link is down\n", port);
                }
            }
        }
    }
}
/*****************************************************************************************************************
 End of function check_link_status
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          test_ping
 * @brief       Ping test main
 * @param       none
 * @retval      none
 **********************************************************************************************************************/
static void test_ping(void)
{
    uint32_t    length_byte;
    fsp_err_t   err = FSP_SUCCESS;
    uint8_t     port;

    while (1)
    {
        R_GMAC_B_LinkProcess(P_GMAC_CTRL);        /* Link up/down processing */
        check_link_status();                      /* Check link status */

        while (1)
        {
            err = R_GMAC_B_Read(P_GMAC_CTRL, &g_gmac_rx_frame, &length_byte); /* Read ethernet frame */

            if (FSP_SUCCESS != err)
            {
                switch (err)
                {
                    case FSP_ERR_ETHER_ERROR_NO_DATA:
                    case FSP_ERR_ETHER_ERROR_LINK:
                    case FSP_ERR_ETHER_ERROR_MAGIC_PACKET_MODE:
                    case FSP_ERR_ETHER_ERROR_TRANSMIT_BUFFER_FULL:
                        break;

                    case FSP_ERR_ETHER_ERROR_PHY_COMMUNICATION:
                        APP_PRINT("R_GMAC_Read() - FSP_ERR_ETHER_ERROR_PHY_COMMUNICATION\n");
                        break;

                    default:
                        break;
                }
                break;
            }

            port = 0;

            if ((ETHSW_SPECIFIC_TAG_ENABLE           == g_specific_tag) &&    /* Enable management port specific tag */
                (CONVERT_HTONS(g_gmac_rx_frame.type) == (uint16_t)g_specific_tag_id)) /* Matches the tag ID */
            {
                port = g_gmac_rx_frame.data[1] & 0x3;

                /* Remove management port specific tag when receive frame has management port specific tag. */
                memcpy( (uint8_t *)&g_gmac_rx_frame.type,
                        ((uint8_t *)&g_gmac_rx_frame.type + PING_MANAGEMENT_TAG_SIZE), /* cast to uint8_t * */
                        (((length_byte - PING_MAC_ADDRESS_SIZE) - PING_MAC_ADDRESS_SIZE) - PING_MANAGEMENT_TAG_SIZE));

            }

            /* Received ethernet frame processing */
            incoming_mac (port, (uint8_t *)&g_gmac_rx_frame);
        }
    }
}
/*****************************************************************************************************************
 End of function test_ping
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          test_main
 * @brief       Test main
 * @param       none
 * @retval      none
 **********************************************************************************************************************/
void test_main(void)
{
    fsp_err_t   err;
    uint32_t    index;

    APP_PRINT("\n");
    APP_PRINT("Start sample program for ether driver, transmit Ping replay for request.\n");

    /* Display own IP address list */
    APP_PRINT("Own Address: \n");
    for (index = 0; ; index++)
    {
        if (0 == g_own_addr[index].ip_addr)
        {
            /* list end */
            break;
        }

        APP_PRINT("  port=%d mac=%02x:%02x:%02x:%02x:%02x:%02x ip=%d.%d.%d.%d \n",
                    index,
                    g_own_addr[index].mac_addr[0], g_own_addr[index].mac_addr[1], g_own_addr[index].mac_addr[2],
                    g_own_addr[index].mac_addr[3], g_own_addr[index].mac_addr[4], g_own_addr[index].mac_addr[5],
                    ((g_own_addr[index].ip_addr >> 24) & 0xff),
                    ((g_own_addr[index].ip_addr >> 16) & 0xff),
                    ((g_own_addr[index].ip_addr >>  8) & 0xff),
                    ((g_own_addr[index].ip_addr >>  0) & 0xff));
    }

    /* cast to gmac_b_extend_cfg_t */
    if (GMAC_B_PHYLINK_ENABLE == ((gmac_b_extend_cfg_t *)P_GMAC_CFG->p_extend)->phylink)
    {
        APP_PRINT("GMAC_PHYLINK_ENABLE\n");
    }

    g_specific_tag    = ((ethsw_extend_cfg_t *) ((gmac_b_extend_cfg_t *) /* cast to ethsw_extend_cfg_t */
            P_GMAC_CFG->p_extend)->p_ethsw_instance->p_cfg->p_extend)->specific_tag;
    g_specific_tag_id = ((ethsw_extend_cfg_t *) ((gmac_b_extend_cfg_t *) /* cast to ethsw_extend_cfg_t */
            P_GMAC_CFG->p_extend)->p_ethsw_instance->p_cfg->p_extend)->specific_tag_id;

    if (ETHSW_SPECIFIC_TAG_ENABLE == g_specific_tag)
    {
        APP_PRINT("ETHSW_SPECIFIC_TAG_ENABLE: Tag ID = %04x\n", g_specific_tag_id);
    }
    else
    {
        APP_PRINT("Please enable management port specific tag in r_ethsw configuration ! \n");
    }

    while (1)
    {
        err = R_GMAC_B_Open(P_GMAC_CTRL, P_GMAC_CFG);
        APP_PRINT("R_GMAC_Open()=%d\n", err);

        {
            ethsw_mac_table_entry_addr_t    mac_entry_addr =
            {
                0
            };
            ethsw_mac_table_entry_info_t    mac_entry_info =
            {
                0
            };
            ethsw_flood_unknown_config_t    flood_config   =
            {
                0
            };

            mac_entry_addr.mac_addr       = &g_own_addr[0].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl, &mac_entry_addr, &mac_entry_info);

            mac_entry_addr.mac_addr       = &g_own_addr[1].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl, &mac_entry_addr, &mac_entry_info);

            mac_entry_addr.mac_addr       = &g_own_addr[2].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl, &mac_entry_addr, &mac_entry_info);

            flood_config.port_mask_bcast.mask = 0xf;    /* Port0, Port1, Port2, Port3(Management port) */
            flood_config.port_mask_mcast.mask = 0x7;    /* Port0, Port1, Port2 */
            flood_config.port_mask_ucast.mask = 0x7;    /* Port0, Port1, Port2 */
            R_ETHSW_FloodUnknownSet (g_ethsw0.p_ctrl, &flood_config);
        }

        test_ping ();

        err = R_GMAC_B_Close(P_GMAC_CTRL);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_GMAC_B_Close API failed ** \r\n");
        }
        else
        {
            APP_ERR_PRINT("** R_GMAC_B_Close API success ** \r\n");
        }

    }
}
/*****************************************************************************************************************
 End of function test_main
 ****************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          gmac_callback
 * @brief       Callback routine when ethernet event occur
 * @param[in]   p_arg   Pointer to the callback parameter
 * @retval      none
 **********************************************************************************************************************/
void gmac_callback(ether_callback_args_t * const p_arg)
{
    switch (p_arg->event)
    {
        case ETHER_EVENT_LINK_ON:           /* Link up detection event */
            g_link_status |= (uint8_t)p_arg->status_link;       /* status up */
            g_link_change |= (uint8_t)p_arg->status_link;       /* change bit set */
            break;

        case ETHER_EVENT_LINK_OFF:          /* Link down detection event */
            g_link_status &= (uint8_t)(~p_arg->status_link);    /* status down */
            g_link_change |= (uint8_t)p_arg->status_link;       /* change bit set */
            break;

        case ETHER_EVENT_WAKEON_LAN:        /* Magic packet detection event */
        case ETHER_EVENT_SBD_INTERRUPT:     /* SBD Interrupt event */
        case ETHER_EVENT_PMT_INTERRUPT:     /* PMT Interrupt event */
            break;

        default:
            break;
    }
}
/*****************************************************************************************************************
 End of function gmac_callback
 ****************************************************************************************************************/
