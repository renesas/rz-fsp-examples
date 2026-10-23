/***********************************************************************************************************************
 * File Name    : ping_test_multi_port.c
 * Description  : Contains the GMAC-B and Ethernet Switch multi-port ping example.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

/*
 * The gmac_b zerocopy configuration must be GMAC_ZEROCOPY_DISABLE for this example.
 */

#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define NETWORK_ADDRESS         (255UL<<24 | 255UL<<16 | 255UL<<8 | 0UL)    ///< 255.255.255.0

#define OWN_IP_ADDRESS_PORT0    (192UL<<24 | 168UL<<16 | 10UL<<8 | 100UL)   ///< 192.168.10.100
#define OWN_IP_ADDRESS_PORT1    (192UL<<24 | 168UL<<16 | 20UL<<8 | 100UL)   ///< 192.168.20.100
#define OWN_IP_ADDRESS_PORT2    (192UL<<24 | 168UL<<16 | 30UL<<8 | 100UL)   ///< 192.168.30.100

#define OWN_MAC_ADDRESS_PORT0   {0x00, 0x11, 0x11, 0x11, 0x11, 0x11}        ///< 00:11:11:11:11:11
#define OWN_MAC_ADDRESS_PORT1   {0x00, 0x22, 0x22, 0x22, 0x22, 0x22}        ///< 00:22:22:22:22:22
#define OWN_MAC_ADDRESS_PORT2   {0x00, 0x33, 0x33, 0x33, 0x33, 0x33}        ///< 00:33:33:33:33:33


#define GMAC_BUFFER_SIZE            (2048U)              ///< Transmission Buffer size

#define MAC_TYPE_ARP                (0x0806U)            ///< Ethernet type for ARP
#define MAC_TYPE_IP                 (0x0800U)            ///< Ethernet type for IP
#define IP_PROTOCALICMP             (1U)

#define ICMP_TYPE_ECHO_REQUEST      (8U)                 ///< ICMP type for ECHO Request
#define ICMP_TYPE_ECHO_REPLY        (0U)                 ///< ICMP type for ECHO Reply

#define PING_MAC_ADDRESS_SIZE       (6U)                 ///< MAC Address Size
#define PING_IP_ADDRESS_SIZE        (4U)                 ///< IP Address Size
#define PING_MANAGEMENT_TAG_SIZE    (8U)

#define P_GMAC_CTRL     g_ether0.p_ctrl                 ///< Pointer of control block for GMAC driver
#define P_GMAC_CFG      g_ether0.p_cfg                  ///< Pointer of configuration parameters for GMAC driver

#define PING_PORT_COUNT             (3U)                 ///< Count of port

/* Convert byte order of 32bits from network to host */
#define CONVERT_NTOHL(x)  \
     (uint32_t)((uint32_t)(((x)>>24)&(0xffUL))\
         |(uint32_t)(((x)>>8)&(0xff00UL))\
         |(uint32_t)(((x & 0xffffffUL)<<8)&(0xff0000UL))\
         |(uint32_t)(((x & 0xffUL)<<24)&(0xff000000UL)))

/* Convert byte order of 16bits from network to host */
#define CONVERT_NTOHS(x) \
        (uint16_t)((uint16_t)(((x)>>8)&0xffU) \
              |(uint16_t)(((x)&0xffU)<<8)) \

/* Convert byte order of 32bits from host to network */
#define CONVERT_HTONL(x)            CONVERT_NTOHL(x)

/* Convert byte order of 16bits from host to network */
#define CONVERT_HTONS(x)            CONVERT_NTOHS(x)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/* ethernet frame format struct */
typedef struct
{
    uint8_t     des_address[PING_MAC_ADDRESS_SIZE];     ///< Destination MAC Address
    uint8_t     sur_address[PING_MAC_ADDRESS_SIZE];     ///< Source MAC Address
    uint16_t    type;                                   ///< Ethernet Type
    uint8_t     data[2000];
} gmac_frame_t;

/* ethernet frame header format struct */
typedef struct st_gmac_mac
{
    uint8_t     mac_des_addr[PING_MAC_ADDRESS_SIZE];    ///< Destination MAC Address
    uint8_t     mac_sur_addr[PING_MAC_ADDRESS_SIZE];    ///< Source MAC Address
    uint16_t    mac_type;                               ///< Ethernet Type
} gmac_mac_t;

/* arp packet format struct     */
typedef struct st_gmac_arp
{
    uint16_t    arp_hardware_type;                      ///< Hardware Type
    uint16_t    arp_protocol_type;                      ///< Protocol Type
    uint8_t     arp_hardware_size;                      ///< Hardware size
    uint8_t     arp_protocol_size;                      ///< Protocol Size
    uint16_t    arp_opcode;                             ///< OpCode
    uint8_t     arp_sender_MAC[PING_MAC_ADDRESS_SIZE];  ///< Sender MAC address
    uint8_t     arp_sender_IP[PING_IP_ADDRESS_SIZE];    ///< Sender IP address
    uint8_t     arp_target_MAC[PING_MAC_ADDRESS_SIZE];  ///< Target MAC address
    uint8_t     arp_target_IP[PING_IP_ADDRESS_SIZE];    ///< Target IP address
} gmac_arp_t;

/* ip packet format struct      */
typedef struct st_gmac_ip
{
    uint8_t     ip_ver;                                 ///< Version / Internet Header Length
    uint8_t     ip_tos;                                 ///< Type of Service
    uint16_t    ip_total_leng;                          ///< Total length
    uint16_t    ip_id;                                  ///< Identification
    uint16_t    ip_offset;                              ///< IP Flags / Fragment Offset
    uint8_t     ip_ttl;                                 ///< Time to live
    uint8_t     ip_protocol;                            ///< Protocol
    uint16_t    ip_checksum;                            ///< Header Checksum
    uint8_t     ip_sou_addr[PING_IP_ADDRESS_SIZE];      ///< Source Address
    uint8_t     ip_des_addr[PING_IP_ADDRESS_SIZE];      ///< Destination address
} gmac_ip_t;

/* icmp message format struct   */
typedef struct st_gmac_icmp
{
    uint8_t     icmp_type;              ///< Type (Echo Reply:0, Echo Request:8)
    uint8_t     icmp_code;              ///< Code (Echo Reply:0, Echo Request:0)
    uint16_t    icmp_checksum;          ///< Checksum
    uint16_t    icmp_Id;                ///< Identifier
    uint16_t    icmp_Seq;               ///< Sequence number
} gmac_icmp_t;

/* arp format struct            */
typedef struct st_arp_packet
{
    gmac_mac_t     arpp_mac;            ///< Ethernet Frame Header
    gmac_arp_t     arpp_arp;            ///< ARP Packet Header
} gmac_arp_packet_t;

/* icmp format struct           */
typedef struct st_icmp_packet
{
    gmac_mac_t     icmpp_mac;           ///< Ethernet Frame Header
    gmac_ip_t      icmpp_ip;            ///< IP Packet Header
    gmac_icmp_t    icmpp_icmp;          ///< ICMP Packet Header
} gmac_icmp_packet_t;

/* own ip address struct        */
typedef struct st_own_addr
{
    uint8_t         mac_addr[PING_MAC_ADDRESS_SIZE];    ///< MAC Address
    uint32_t        ip_addr;                            ///< IP address
    uint32_t        ip_mask;                            ///< IP Mask
} gmac_own_addr_t;

/***********************************************************************************************************************
 * Extern global variables
 **********************************************************************************************************************/
extern const    ether_instance_t g_ether0;

/***********************************************************************************************************************
 * Exported global function
 **********************************************************************************************************************/
extern volatile uint32_t g_transfer_complete;
extern volatile uint32_t g_receive_complete;

/***********************************************************************************************************************
 * Global function prototypes
 **********************************************************************************************************************/
void test_main(void);
void gmac_callback(ether_callback_args_t * const p_arg);

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void app_printf(const char * format, ...);
void btoh(char buf[], unsigned int dat, unsigned int siz);
void display_memory_dump(uint8_t * puchData, uint32_t usSize);
void disp_lookup_memory(void);

static uint16_t compute_checksum(uint32_t pseudo_sum, uint8_t * p_buf, uint32_t size);
static void incoming_arp(uint8_t port, uint8_t * p_buff);
static void incoming_icmp(uint8_t port, uint8_t * p_buff);
static void incoming_ip(uint8_t port, uint8_t * p_buff);
static void incoming_mac(uint8_t port, uint8_t * p_buff);
static void check_link_status(void);
static void test_ping(void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
uint8_t g_broadcast_MAC_addr[PING_MAC_ADDRESS_SIZE]   = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static uint8_t      g_link_change = 0;      ///< Link change (bit0:port0, bit1:port1, bit2:port2)
static uint8_t      g_link_status = 0U;      ///< Link status (bit0:port0, bit1:port1, bit2:port2)

static char g_uart_buf[1024];        ///< uart tx buffer

static gmac_frame_t   g_gmac_rx_frame = {0};

/* Own IP address table (IP address, Net mask) */
static gmac_own_addr_t g_own_addr[]  =
{
    {OWN_MAC_ADDRESS_PORT0, OWN_IP_ADDRESS_PORT0, NETWORK_ADDRESS},
    {OWN_MAC_ADDRESS_PORT1, OWN_IP_ADDRESS_PORT1, NETWORK_ADDRESS},
    {OWN_MAC_ADDRESS_PORT2, OWN_IP_ADDRESS_PORT2, NETWORK_ADDRESS},
    {{0}, 0, 0}     ///< terminate
};

/* Transmission Packet Buffer */
static union
{
    gmac_arp_packet_t  arpp;
    gmac_icmp_packet_t icmpp;
    uint8_t             data[GMAC_BUFFER_SIZE-sizeof(gmac_arp_packet_t)-sizeof(gmac_icmp_packet_t)];
} g_tx_packet;

ethsw_specific_tag_t g_specific_tag;        ///< Management port specific frame tag
uint32_t             g_specific_tag_id;     ///< Management port specific frame tag ID

/*******************************************************************************************************************//**
 * Customized printf
 *
 * @param[in] printf format
 *
 * @retval  none
 **********************************************************************************************************************/
static void app_printf(const char * format, ...)
{
    va_list   ap;
    int32_t   formatted_length;
    uint32_t  bytes;
    fsp_err_t err;

    va_start(ap, format);
    formatted_length = vsnprintf(g_uart_buf, sizeof(g_uart_buf), format, ap);
    va_end(ap);

    if (formatted_length <= 0)
    {
        return;
    }

    bytes = (uint32_t) formatted_length;
    if (bytes >= sizeof(g_uart_buf))
    {
        bytes = sizeof(g_uart_buf) - 1U;
    }

    g_transfer_complete = 0U;
    err = R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *) g_uart_buf, bytes);
    if (FSP_SUCCESS != err)
    {
        return;
    }

    while (0U == g_transfer_complete)
    {
        __NOP();
    }

    g_transfer_complete = 0U;
}

/*******************************************************************************************************************//**
 * Transmit frame
 *
 * @param[in]   port        Transmit port number
 * @param[in]   tx_buff     Transmit buffer
 * @param[in]   tx_size     Transmit size
 *
 * @retval      return value from R_GMAC_Write()
 **********************************************************************************************************************/
static fsp_err_t transmit_frame(uint8_t port, void * tx_buff, uint32_t tx_size)
{
    fsp_err_t err;
    uint8_t * p_tx_frame = (uint8_t *) tx_buff;
    uint16_t  specific_tag_id = CONVERT_HTONS(g_specific_tag_id);
    uint32_t  type_offset = PING_MAC_ADDRESS_SIZE * 2U;
    uint32_t  control_offset = type_offset + sizeof(specific_tag_id);

    /*
     * Shift the original EtherType and payload by eight bytes to insert the
     * Ethernet Switch management-port specific tag.
     *
     * Use byte addressing instead of casting g_tx_packet to gmac_frame_t.
     * g_tx_packet is intentionally smaller than gmac_frame_t, and GCC 14
     * diagnoses that cast as -Warray-bounds even though only the header area
     * is accessed here.
     */
    memmove(&p_tx_frame[type_offset + PING_MANAGEMENT_TAG_SIZE],
            &p_tx_frame[type_offset],
            tx_size - type_offset);

    /* Insert the two-byte specific tag ID followed by six bytes of control data. */
    memcpy(&p_tx_frame[type_offset], &specific_tag_id, sizeof(specific_tag_id));

    p_tx_frame[control_offset + 0U] = 0U;                       /* ControlData[b15:b8]. */
    p_tx_frame[control_offset + 1U] = 1U;                       /* ControlData[b7:b0]. */
    p_tx_frame[control_offset + 2U] = 0U;                       /* ControlData2[b31:b24]. */
    p_tx_frame[control_offset + 3U] = 0U;                       /* ControlData2[b23:b16]. */
    p_tx_frame[control_offset + 4U] = 0U;                       /* ControlData2[b15:b8]. */
    p_tx_frame[control_offset + 5U] = (uint8_t) (1U << port);  /* ControlData2[b7:b0]. */

    tx_size += PING_MANAGEMENT_TAG_SIZE;

    err = R_GMAC_B_Write(P_GMAC_CTRL, tx_buff, tx_size);
    if (FSP_SUCCESS != err)
    {
        app_printf("R_GMAC_B_Write()=%ld port=%u tx_size=%lu\n",
                   (long) err,
                   (unsigned int) port,
                   (unsigned long) tx_size);
    }

    return err;
}

/*******************************************************************************************************************//**
 * Compute checksum
 *
 * @param[in]   pseudo_sum  Pseudo checksum
 * @param[in]   p_buf       Buffer pointer to calculate checksum
 * @param[in]   size        Buffer size to calculate checksum
 *
 * @retval      Calculated checksum value
 **********************************************************************************************************************/
static uint16_t compute_checksum(uint32_t pseudo_sum, uint8_t * p_buf, uint32_t size)
{
    uint32_t    sum = pseudo_sum;
    uint16_t    val;
    uint16_t    val_l;

    while (size & ~1U)
    {
        val   = (uint16_t)(p_buf[0] << 8);
        val_l = (uint16_t)p_buf[1];
        val   = val | val_l;

        sum += val;
        p_buf += 2;
        size  -= 2;
    }

    if (size & 1)
    {
        sum += (uint16_t)(*p_buf << 8);
    }

    sum = (sum & 0xffff) + (sum >> 16); /* add overflow counts */

    return (uint16_t)(~sum);
}

/*******************************************************************************************************************//**
 * Received ARP packet processing
 *
 * @param[in]   port        Port number received
 * @param[in]   p_buff      buffer of received ethernet frame(arp packet)
 *
 * @retval      none
 **********************************************************************************************************************/
static void incoming_arp(uint8_t port, uint8_t * p_buff)
{
    gmac_arp_packet_t *    p_rx_arpp = (gmac_arp_packet_t *)p_buff;
    gmac_mac_t *           p_rx_mac  = &p_rx_arpp->arpp_mac;
    gmac_arp_t *           p_rx_arp  = &p_rx_arpp->arpp_arp;
    uint32_t                target_ip;

    if (p_rx_arp->arp_opcode == CONVERT_HTONS(1))
    {
        /* When receive packet is ARP request */

        /* Get target IP address */
        target_ip = ((uint32_t)p_rx_arp->arp_target_IP[0] << 24)
                  | ((uint32_t)p_rx_arp->arp_target_IP[1] << 16)
                  | ((uint32_t)p_rx_arp->arp_target_IP[2] <<  8)
                  | ((uint32_t)p_rx_arp->arp_target_IP[3] <<  0);

        if (g_own_addr[port].ip_addr == target_ip)
        {
            /* Found matching target IP address in own IP address list */

            gmac_mac_t *   p_tx_mac = &g_tx_packet.arpp.arpp_mac;
            gmac_arp_t *   p_tx_arp = &g_tx_packet.arpp.arpp_arp;
            uint32_t        tx_size;
//          fsp_err_t       err;

            /* make MAC header */
            memcpy(p_tx_mac->mac_des_addr,
                   p_rx_mac->mac_sur_addr,
                   sizeof(p_rx_mac->mac_sur_addr));
            memcpy(p_tx_mac->mac_sur_addr,
                   g_own_addr[port].mac_addr,
                   PING_MAC_ADDRESS_SIZE);
            p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_ARP);

            /* make arp header */
            p_tx_arp->arp_hardware_type = CONVERT_HTONS(1);
            p_tx_arp->arp_protocol_type = CONVERT_HTONS(0x800);
            p_tx_arp->arp_hardware_size = PING_MAC_ADDRESS_SIZE;    /* MAC size */
            p_tx_arp->arp_protocol_size = PING_IP_ADDRESS_SIZE;     /* IP size */
            p_tx_arp->arp_opcode        = CONVERT_HTONS(2); /* replay */
            memcpy(p_tx_arp->arp_sender_MAC,
                   g_own_addr[port].mac_addr,
                   PING_MAC_ADDRESS_SIZE);
            memcpy(p_tx_arp->arp_sender_IP,
                   p_rx_arp->arp_target_IP,
                   sizeof(p_rx_arp->arp_target_IP));
            memcpy(p_tx_arp->arp_target_MAC,
                   p_rx_arp->arp_sender_MAC,
                   sizeof(p_rx_arp->arp_sender_MAC));
            memcpy(p_tx_arp->arp_target_IP,
                   p_rx_arp->arp_sender_IP,
                   sizeof(p_rx_arp->arp_sender_IP));

            tx_size = sizeof(gmac_arp_packet_t);

            app_printf("arp_reply  port=%d tx_size=%d IP=%d:%d:%d:%d\n",
                    port, tx_size,
                    p_tx_arp->arp_sender_IP[0], p_tx_arp->arp_sender_IP[1],
                    p_tx_arp->arp_sender_IP[2], p_tx_arp->arp_sender_IP[3]);

            /* send arp packet */
            transmit_frame(port, &g_tx_packet, tx_size);
        }
    }
}

/*******************************************************************************************************************//**
 * Received icmp message processing (when echo request received then send echo reply)
 *
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame(icmp message)
 *
 * @retval      none
 **********************************************************************************************************************/
static void incoming_icmp(uint8_t port, uint8_t * p_buff)
{
    gmac_icmp_packet_t *p_rx_icmpp = (gmac_icmp_packet_t *)p_buff;
    gmac_mac_t *        p_rx_mac   = &p_rx_icmpp->icmpp_mac;
    gmac_ip_t  *        p_rx_ip    = &p_rx_icmpp->icmpp_ip;
    gmac_icmp_t*        p_rx_icmp  = &p_rx_icmpp->icmpp_icmp;

    if (ICMP_TYPE_ECHO_REQUEST == p_rx_icmp->icmp_type)
    {
        /* When the received packet is an ICMP Echo Request */

        gmac_mac_t *   p_tx_mac  = &g_tx_packet.icmpp.icmpp_mac;
        gmac_ip_t  *   p_tx_ip   = &g_tx_packet.icmpp.icmpp_ip;
        gmac_icmp_t*   p_tx_icmp = &g_tx_packet.icmpp.icmpp_icmp;
        uint32_t        data_size;
        uint32_t        tx_size;

        /* make MAC header */
        memcpy(p_tx_mac->mac_des_addr,
               p_rx_mac->mac_sur_addr,
               sizeof(p_rx_mac->mac_sur_addr));
        memcpy(p_tx_mac->mac_sur_addr,
               g_own_addr[port].mac_addr,
               PING_MAC_ADDRESS_SIZE);
        p_tx_mac->mac_type = CONVERT_HTONS(MAC_TYPE_IP);

        /* make IP header */
        p_tx_ip->ip_ver         = p_rx_ip->ip_ver;
        p_tx_ip->ip_tos         = p_rx_ip->ip_tos;
        p_tx_ip->ip_total_leng  = p_rx_ip->ip_total_leng;
        p_tx_ip->ip_id          = CONVERT_HTONS(0);
        p_tx_ip->ip_offset      = CONVERT_HTONS(0);
        p_tx_ip->ip_ttl         = p_rx_ip->ip_ttl;
        p_tx_ip->ip_protocol    = p_rx_ip->ip_protocol;
        p_tx_ip->ip_checksum    = 0;

        memcpy (p_tx_ip->ip_des_addr, p_rx_ip->ip_sou_addr, sizeof(p_tx_ip->ip_des_addr));
        memcpy (p_tx_ip->ip_sou_addr, p_rx_ip->ip_des_addr, sizeof(p_tx_ip->ip_sou_addr));

        p_tx_ip->ip_checksum = CONVERT_HTONS(compute_checksum(0, (uint8_t *)p_tx_ip, sizeof(gmac_ip_t)));

        /* make ICMP header */
        p_tx_icmp->icmp_type     = ICMP_TYPE_ECHO_REPLY;
        p_tx_icmp->icmp_code     = 0;
        p_tx_icmp->icmp_checksum = 0;

        p_tx_icmp->icmp_Id  = p_rx_icmp->icmp_Id;
        p_tx_icmp->icmp_Seq = p_rx_icmp->icmp_Seq;

        /*
         * Keep this arithmetic in uint32_t.
         *
         * sizeof() yields a 64-bit size_t on the CA55 (AArch64/LP64) build, so
         * mixing it with the 16-bit total length widens the whole expression to
         * 64 bits and then narrows it again on assignment, which -Wconversion
         * reports. Casting each term to uint32_t keeps the computation at the
         * width of data_size and produces the same value on CR52 and CA55.
         */
        data_size = (uint32_t) CONVERT_HTONS(p_rx_ip->ip_total_leng)
                    - (uint32_t) sizeof(gmac_ip_t)
                    - (uint32_t) sizeof(gmac_icmp_t);
        memcpy(&g_tx_packet.data[sizeof(gmac_icmp_packet_t)],
               &p_buff[sizeof(gmac_icmp_packet_t)],
               data_size);

        p_tx_icmp->icmp_checksum = CONVERT_HTONS(compute_checksum(0,
                                                                (uint8_t *) p_tx_icmp,
                                                                sizeof(gmac_icmp_t) + data_size));

        tx_size = sizeof(gmac_icmp_packet_t) + data_size;

        app_printf("ping_reply port=%d tx_size=%d ip=%d:%d:%d:%d seq=%d\n",
                    port, tx_size,
                    p_tx_ip->ip_sou_addr[0],
                    p_tx_ip->ip_sou_addr[1],
                    p_tx_ip->ip_sou_addr[2],
                    p_tx_ip->ip_sou_addr[3],
                    CONVERT_NTOHS(p_tx_icmp->icmp_Seq));

        /* send ICMP packet */
        transmit_frame(port, &g_tx_packet, tx_size);
    }
    else
    {
        app_printf ("incoming_icmp: Unknown type\n");
    }
}

/*******************************************************************************************************************//**
 * Received ip packet processing
 *
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame(ip packet)
 *
 * @retval      none
 **********************************************************************************************************************/
static void incoming_ip(uint8_t port, uint8_t * p_buff)
{
    gmac_icmp_packet_t *   p_rx_icmpp = (gmac_icmp_packet_t *)p_buff;
    gmac_ip_t *            p_rx_ip    = &p_rx_icmpp->icmpp_ip;
    uint32_t               ip_des_addr;

    /* Get destination IP address */
    ip_des_addr = ((uint32_t)p_rx_ip->ip_des_addr[0] << 24)
                | ((uint32_t)p_rx_ip->ip_des_addr[1] << 16)
                | ((uint32_t)p_rx_ip->ip_des_addr[2] <<  8)
                | ((uint32_t)p_rx_ip->ip_des_addr[3] <<  0);

    if (g_own_addr[port].ip_addr == ip_des_addr)
    {
        /* Found matching destination IP address in own IP address list */

        if (IP_PROTOCALICMP == p_rx_ip->ip_protocol)
        {
            /* When the received IP packet is ICMP */
            incoming_icmp (port, p_buff);
        }
    }
}

/*******************************************************************************************************************//**
 * Received ethernet frame processing
 *
 * @param[in]   port        Port number received
 * @param[in]   p_buff      Buffer pointer of received ethernet frame
 *
 * @retval      none
 **********************************************************************************************************************/
static void incoming_mac(uint8_t port, uint8_t * p_buff)
{
    gmac_mac_t *   p_mac = (gmac_mac_t *)p_buff;

    if ((0 == memcmp(p_mac->mac_des_addr,
                     g_own_addr[port].mac_addr,
                     PING_MAC_ADDRESS_SIZE))
        || (0 == memcmp(p_mac->mac_des_addr,
                        g_broadcast_MAC_addr,
                        PING_MAC_ADDRESS_SIZE)))
    {
        /* When destination MAC Address is own MAC address, broadcast address or multicast address */

        switch (p_mac->mac_type)
        {
            case CONVERT_HTONS(MAC_TYPE_ARP):   ///< ARP packet
                incoming_arp (port, p_buff);
                break;

            case CONVERT_HTONS(MAC_TYPE_IP):    ///< IP packet
                incoming_ip (port, p_buff);
                break;

            default:
                break;
        }
    }
}

/*******************************************************************************************************************//**
 * Check Link status. Display when link status changes.
 *
 * @retval      none
 **********************************************************************************************************************/
static void check_link_status(void)
{
    fsp_err_t            err = FSP_SUCCESS;
    gmac_b_link_status_t port_status;
    uint8_t              port;
    uint8_t              port_bit;

    if (0 == P_GMAC_CFG->p_callback)
    {
        /* Not set callback */

        uint8_t previous_link_status = g_link_status;

        g_link_status = 0U;

        for (port = 0U; port < PING_PORT_COUNT; port++)
        {
            err = R_GMAC_B_GetLinkStatus (P_GMAC_CTRL, port, &port_status);
            if (FSP_SUCCESS != err)
            {
                /* An error has occurred */
                app_printf("R_GMAC_B_GetLinkStatus()=%ld\n", (long) err);
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
        for (port = 0U; port < PING_PORT_COUNT; port++)
        {
            port_bit = (uint8_t)(1U << port);

            if (g_link_change & port_bit)
            {
                /* Link status changed */
                g_link_change &= (uint8_t)(~port_bit);  /* Clear the change bit. */

                if (g_link_status & port_bit)
                {
                    /* Changed to Link-up */
                    app_printf("Port %d link is up\n", port );
                }
                else
                {
                    /* Changed to Link-down */
                    app_printf("Port %d link is down\n", port );
                }
            }
        }
    }
}

/*******************************************************************************************************************//**
 * Ping test main
 *
 * @param       none
 *
 * @retval      none
 **********************************************************************************************************************/
static void test_ping(void)
{
    uint32_t    length_byte;
    fsp_err_t   err = FSP_SUCCESS;
    uint8_t     port;

    while (1)
    {
        R_GMAC_B_LinkProcess(P_GMAC_CTRL);        ///< Link up/down processing
        check_link_status();                    ///< Check link status

        while (1)
        {
            /* Read an Ethernet frame. */
            err = R_GMAC_B_Read(P_GMAC_CTRL, &g_gmac_rx_frame, &length_byte);

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
                        app_printf("R_GMAC_Read() - FSP_ERR_ETHER_ERROR_PHY_COMMUNICATION\n");
                        break;

                    default:
                        break;
                }
                break;
            }

            port = 0;

            if ((ETHSW_SPECIFIC_TAG_ENABLE == g_specific_tag) &&
                (CONVERT_HTONS(g_gmac_rx_frame.type) == (uint16_t) g_specific_tag_id))
            {
                port = g_gmac_rx_frame.data[1] & 0x3;

                /* Remove management port specific tag when receive frame has management port specific tag. */
                memcpy( (uint8_t *)&g_gmac_rx_frame.type,
                        ((uint8_t *)&g_gmac_rx_frame.type + PING_MANAGEMENT_TAG_SIZE),
                        (length_byte - (PING_MAC_ADDRESS_SIZE * 2U) - PING_MANAGEMENT_TAG_SIZE));

            }

            /* Received ethernet frame processing */
            incoming_mac (port, (uint8_t *)&g_gmac_rx_frame);
        }
    }
}

/*******************************************************************************************************************//**
 * Test main
 *
 * @param       none
 *
 * @retval      none
 **********************************************************************************************************************/
void test_main(void)
{
    fsp_err_t err;
    uint32_t  index;

    app_printf(BANNER_INFO,
               EP_VERSION,
               4, 2, 0);

    app_printf("\r\nExample Project Overview\r\n");
    app_printf("This example demonstrates GMAC-B and Ethernet Switch operation across three Ethernet ports.\r\n");
    app_printf("The board responds to ARP and ICMP Echo Requests on 192.168.10.100, "
               "192.168.20.100, and 192.168.30.100.\r\n");

    app_printf("\n");
    app_printf("Start sample program for ether driver, transmit Ping replay for request.\n");

    /* Display own IP address list */
    app_printf("Own Address: \n");
    for (index = 0U; ; index++)
    {
        if (g_own_addr[index].ip_addr == 0)
        {
            /* list end */
            break;
        }

        app_printf("  port=%lu mac=%02x:%02x:%02x:%02x:%02x:%02x ip=%lu.%lu.%lu.%lu \n",
                   (unsigned long) index,
                   g_own_addr[index].mac_addr[0],
                   g_own_addr[index].mac_addr[1],
                   g_own_addr[index].mac_addr[2],
                   g_own_addr[index].mac_addr[3],
                   g_own_addr[index].mac_addr[4],
                   g_own_addr[index].mac_addr[5],
                   (unsigned long) ((g_own_addr[index].ip_addr >> 24) & 0xffUL),
                   (unsigned long) ((g_own_addr[index].ip_addr >> 16) & 0xffUL),
                   (unsigned long) ((g_own_addr[index].ip_addr >> 8) & 0xffUL),
                   (unsigned long) (g_own_addr[index].ip_addr & 0xffUL));
    }

    if (GMAC_B_PHYLINK_ENABLE == ((gmac_b_extend_cfg_t *) P_GMAC_CFG->p_extend)->phylink)
    {
        app_printf("GMAC_PHYLINK_ENABLE\n");
    }

    g_specific_tag =
        ((ethsw_extend_cfg_t *) ((gmac_b_extend_cfg_t *) P_GMAC_CFG->p_extend)->
         p_ethsw_instance->p_cfg->p_extend)->specific_tag;
    g_specific_tag_id =
        ((ethsw_extend_cfg_t *) ((gmac_b_extend_cfg_t *) P_GMAC_CFG->p_extend)->
         p_ethsw_instance->p_cfg->p_extend)->specific_tag_id;

    if (ETHSW_SPECIFIC_TAG_ENABLE == g_specific_tag)
    {
        app_printf("ETHSW_SPECIFIC_TAG_ENABLE: Tag ID = %04lx\n",
               (unsigned long) g_specific_tag_id);
    }
    else
    {
        app_printf("Please enable management port specific tag in r_ethsw configuration ! \n");
    }

    while (1)
    {
        err = R_GMAC_B_Open(P_GMAC_CTRL, P_GMAC_CFG);
        app_printf("R_GMAC_B_Open()=%ld\n", (long) err);

        {
            ethsw_mac_table_entry_addr_t    mac_entry_addr = {0};
            ethsw_mac_table_entry_info_t    mac_entry_info = {0};
            ethsw_flood_unknown_config_t    flood_config   = {0};

            mac_entry_addr.mac_addr       = &g_own_addr[0].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port. */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl,
                                &mac_entry_addr,
                                &mac_entry_info);

            mac_entry_addr.mac_addr       = &g_own_addr[1].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port. */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl,
                                &mac_entry_addr,
                                &mac_entry_info);

            mac_entry_addr.mac_addr       = &g_own_addr[2].mac_addr;
            mac_entry_info.port_mask.mask = (1U << 3);   /* Management port. */
            R_ETHSW_MacTableSet(g_ethsw0.p_ctrl,
                                &mac_entry_addr,
                                &mac_entry_info);

            flood_config.port_mask_bcast.mask = 0xf;    /* Port0, Port1, Port2, and Port3 (management port). */
            flood_config.port_mask_mcast.mask = 0x7;    /* Port0, Port1, and Port2. */
            flood_config.port_mask_ucast.mask = 0x7;    /* Port0, Port1, and Port2. */
            R_ETHSW_FloodUnknownSet(g_ethsw0.p_ctrl, &flood_config);
        }

        test_ping ();

        err = R_GMAC_B_Close(P_GMAC_CTRL);
        app_printf("R_GMAC_B_Close()=%ld\n", (long) err);
    }
}

/*******************************************************************************************************************//**
 * Callback routine when Ethernet event occurs
 *
 * @param[in]   p_arg   Pointer to the callback parameter
 *
 * @retval      none
 **********************************************************************************************************************/
void gmac_callback(ether_callback_args_t * const p_arg)
{
    switch (p_arg->event)
    {
        case ETHER_EVENT_LINK_ON:           ///< Link up detection event/
            g_link_status |= (uint8_t)p_arg->status_link;       ///< status up
            g_link_change |= (uint8_t)p_arg->status_link;       ///< change bit set
            break;

        case ETHER_EVENT_LINK_OFF:          ///< Link down detection event
            g_link_status &= (uint8_t)(~p_arg->status_link);    ///< status down
            g_link_change |= (uint8_t)p_arg->status_link;       ///< change bit set
            break;

        case ETHER_EVENT_WAKEON_LAN:        ///< Magic packet detection event
        case ETHER_EVENT_SBD_INTERRUPT:     ///< BSD Interrupt event
        case ETHER_EVENT_PMT_INTERRUPT:     ///< PMT Interrupt event
            break;

        default:
            break;
    }
}
