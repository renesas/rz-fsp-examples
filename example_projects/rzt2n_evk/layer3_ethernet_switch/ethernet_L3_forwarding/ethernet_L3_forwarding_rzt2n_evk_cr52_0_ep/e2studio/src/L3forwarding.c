/***********************************************************************************************************************
 * File Name    : L3forwarding.c
 * Description  : Contains data structures and functions used in L3forwarding.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "L3forwarding.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_L3_forwarding_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * USER-CONFIGURABLE: PC NIC MAC addresses.
 * Run "ipconfig /all", copy Physical Address for NIC1 (ETH0) and NIC2 (ETH1), then rebuild.
 **********************************************************************************************************************/
static uint8_t g_l3_forwarding_pc1_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH] = { 0x00, 0x0E, 0xC6, 0xC8, 0x3C, 0x29 };
static uint8_t g_l3_forwarding_pc2_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH] = { 0x00, 0x23, 0x56, 0x3C, 0x86, 0x51 };

/***********************************************************************************************************************
 * Private macro definitions
 **********************************************************************************************************************/

/* All three ports (ETH0, ETH1, CPU_INJECT) accepted as ingress source. */
#define L3_FORWARDING_SRC_PORTS_ALL \
        (LAYER3_SWITCH_PORT_BITMASK_PORT0 | \
        LAYER3_SWITCH_PORT_BITMASK_PORT1 | \
        LAYER3_SWITCH_PORT_BITMASK_PORT2)

/* Network address initializers */
#define L3_FORWARDING_ETH0_IP       { 192U, 168U,  10U,   1U }
#define L3_FORWARDING_ETH1_IP       { 192U, 168U,  20U,   1U }
#define L3_FORWARDING_PC1_IP        { 192U, 168U,  10U, 100U }
#define L3_FORWARDING_PC2_IP        { 192U, 168U,  20U, 100U }
#define L3_FORWARDING_ETH0_MAC      { 0x00U, 0x11U, 0x22U, 0x33U, 0x44U, 0x00U }
#define L3_FORWARDING_ETH1_MAC      { 0x00U, 0x11U, 0x22U, 0x33U, 0x44U, 0x01U }

/* EtherType constants */
#define L3_FORWARDING_ETHERTYPE_ARP     (0x0806U)
#define L3_FORWARDING_ETHERTYPE_IPV4    (0x0800U)
#define L3_FORWARDING_ARP_OP_REQUEST    (0x0001U)
#define L3_FORWARDING_ARP_OP_REPLY      (0x0002U)

/* Buffer sizes */
#define L3_FORWARDING_RX_BUF_SIZE       (1536U)
#define L3_FORWARDING_ETH_HDR_LEN       (14U)

/* Forwarding table settings */
#define L3_FORWARDING_TABLE_ENTRY_MAX   (8U)

/* GWCA descriptor ring: 1 data + 1 LEMPTY terminator; array length 5 provides headroom */
#define L3_FORWARDING_DESC_QUEUE_LEN    (5U)

/* TX buffer rounded up to 32-byte DMA alignment boundary (GWCA requirement) */
#define L3_FORWARDING_DMA_ALIGN         (32U)
#define L3_FORWARDING_TX_BUF_SIZE \
        (((L3_FORWARDING_MIN_FRAME_SIZE + L3_FORWARDING_DMA_ALIGN - 1U) / \
        L3_FORWARDING_DMA_ALIGN) * L3_FORWARDING_DMA_ALIGN)

/* L3 VLAN IDs used for C-TAG stream filter key */
#define L3_FORWARDING_L3_VID_A          (L3_FORWARDING_VLAN_ID_ETH0)   /* VID=10: NIC1->NIC2 direction */
#define L3_FORWARDING_L3_VID_B          (L3_FORWARDING_VLAN_ID_ETH1)   /* VID=20: NIC2->NIC1 direction */

/* Run loop timing */
#define L3_FORWARDING_LINK_POLL_MS      (10U)
#define L3_FORWARDING_STATS_ITER        (1000U)
#define L3_FORWARDING_INJECT_GAP_MS     (100U)

/* Byte-swap helper (host <-> network order) */
#define L3_HTONS(x) ((uint16_t)(((uint16_t)(x) >> 8U) | ((uint16_t)(x) << 8U)))
#define L3_NTOHS(x) (L3_HTONS(x))

static uint8_t s_reject_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH] = { 0xDEU, 0xADU, 0xBEU, 0xEFU, 0x00U, 0x01U };
#define L3_FORWARDING_REJECT_VID    (99U)
static const uint8_t s_reject_ip[L3_FORWARDING_IP_ADDRESS_LENGTH] = { 192U, 168U, 20U, 200U };
static uint8_t s_dual_port_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH] = { 0xBBU, 0xBBU, 0xBBU, 0xBBU, 0x00U, 0x01U };

/***********************************************************************************************************************
 * Private type definitions
 **********************************************************************************************************************/

/* Packed Ethernet/ARP frame structures for ARP reply construction */
#pragma pack(push, 1)

typedef struct { uint8_t b[L3_FORWARDING_MAC_ADDRESS_LENGTH]; } l3_mac_t;

typedef struct
{
    l3_mac_t dst;
    l3_mac_t src;
    uint16_t etype;
} l3_eth_hdr_t;

typedef struct
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t  hlen;
    uint8_t  plen;
    uint16_t opcode;
    l3_mac_t sender_mac;
    uint8_t  sender_ip[L3_FORWARDING_IP_ADDRESS_LENGTH];
    l3_mac_t target_mac;
    uint8_t  target_ip[L3_FORWARDING_IP_ADDRESS_LENGTH];
} l3_arp_pkt_t;

typedef struct
{
    l3_eth_hdr_t eth;
    l3_arp_pkt_t arp;
} l3_arp_frame_t;

#pragma pack(pop)

/* Per-port context: link state, IP/MAC, ARP counters, and RMAC instance pointer */
typedef struct
{
    uint8_t                  idx;
    uint8_t                  mac[L3_FORWARDING_MAC_ADDRESS_LENGTH];
    uint8_t                  ip[L3_FORWARDING_IP_ADDRESS_LENGTH];
    volatile bool            link_up;
    volatile bool            link_changed;
    const ether_instance_t * p_ether;
    uint32_t                 cnt_arp_rx;
    uint32_t                 cnt_arp_tx;
} l3_forwarding_port_ctx_t;

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

static l3_forwarding_port_ctx_t g_port[2];
static l3_forwarding_mode_t     g_forwarding_mode;

/* Network address constants */
static const uint8_t s_eth0_ip[L3_FORWARDING_IP_ADDRESS_LENGTH]           = L3_FORWARDING_ETH0_IP;
static const uint8_t s_eth1_ip[L3_FORWARDING_IP_ADDRESS_LENGTH]           = L3_FORWARDING_ETH1_IP;
static const uint8_t s_pc1_ip[L3_FORWARDING_IP_ADDRESS_LENGTH]            = L3_FORWARDING_PC1_IP;
static const uint8_t s_pc2_ip[L3_FORWARDING_IP_ADDRESS_LENGTH]            = L3_FORWARDING_PC2_IP;
static const uint8_t s_eth0_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH]         = L3_FORWARDING_ETH0_MAC;
static const uint8_t s_eth1_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH]         = L3_FORWARDING_ETH1_MAC;

/* DMA-accessible RX buffers: must reside in non-cached SRAM (GWCA bypasses CPU cache) */
static uint8_t g_rx_buf0[L3_FORWARDING_RX_BUF_SIZE] BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
static uint8_t g_rx_buf1[L3_FORWARDING_RX_BUF_SIZE] BSP_PLACE_IN_SECTION(".noncache_buffer.eth");

/* ARP TX buffers */
static uint8_t g_tx_arp0[L3_FORWARDING_MIN_FRAME_SIZE] BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
static uint8_t g_tx_arp1[L3_FORWARDING_MIN_FRAME_SIZE] BSP_PLACE_IN_SECTION(".noncache_buffer.eth");

/* RMAC callback storage */
static ether_callback_args_t s_cb_mem0;
static ether_callback_args_t s_cb_mem1;

/* L3 table entry list (backing storage for s_l3_table) */
static layer3_switch_table_entry_t s_l3_entries[L3_FORWARDING_TABLE_ENTRY_MAX]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
static layer3_switch_table_t s_l3_table;

/* L3 route update configuration - must remain valid while entries are installed in HW */
static uint8_t s_dst_ip_pc2[L3_FORWARDING_IP_ADDRESS_LENGTH];
static uint8_t s_dst_ip_pc1[L3_FORWARDING_IP_ADDRESS_LENGTH];
static layer3_switch_l3_update_config_t s_update_to_pc2;
static layer3_switch_l3_update_config_t s_update_to_pc1;

/* Fresh-queue descriptor ring and TX frame buffer.
 * CreateDescriptorQueue is called per inject call (one-shot) to avoid FSP cursor drift. */
BSP_ALIGN_VARIABLE(32) static layer3_switch_descriptor_t s_tx_desc[L3_FORWARDING_DESC_QUEUE_LEN]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
BSP_ALIGN_VARIABLE(32) static uint8_t s_tx_frame[L3_FORWARDING_TX_BUF_SIZE]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");

/* MAC addresses */
extern uint8_t g_layer3_switch0_mac_address_port0[L3_FORWARDING_MAC_ADDRESS_LENGTH];
extern uint8_t g_layer3_switch0_mac_address_port1[L3_FORWARDING_MAC_ADDRESS_LENGTH];

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void     l3_forwarding_rmac_callback (ether_callback_args_t *p_args);
static void     l3_forwarding_check_link (l3_forwarding_port_ctx_t *p_port);
static void     l3_forwarding_rmac_init (void);
static void     l3_forwarding_verify_entry (const layer3_switch_frame_filter_t *p_filter);
static void     l3_forwarding_read_table (void);
static void     l3_forwarding_configure_table (void);
static void     l3_forwarding_install_entries (void);
static void     l3_forwarding_wait_links (void);
static void     l3_forwarding_arp_reply (l3_forwarding_port_ctx_t *p_port, const l3_arp_frame_t *p_req);
static void     l3_forwarding_rx_arp (l3_forwarding_port_ctx_t *p_port, uint8_t *p_buf);
static uint16_t l3_forwarding_ipv4_checksum (const uint8_t *p_hdr, uint16_t hdr_len);
static bool     l3_forwarding_transmit_one_frame (const uint8_t *p_frame, uint16_t len,
                                                  uint8_t tx_fmt, uint8_t tx_dv);
static uint16_t l3_forwarding_build_ctag_udp_frame(uint8_t *p_frame,
                                                   const uint8_t *p_dst_mac, const uint8_t *p_src_mac,
                                                   uint16_t vid,
                                                   const uint8_t *p_src_ip, const uint8_t *p_dst_ip,
                                                   uint16_t identification);
static void     l3_forwarding_report_tx (bool consumed, const char *p_frame, const char *p_result);
static void l3_forwarding_install_reject_entry(void);
static void l3_forwarding_install_dual_port_entry(void);
static void     l3_forwarding_inject_mac_frames (void);
static void     l3_forwarding_inject_vlan_frames (void);
static void     l3_forwarding_inject_l3_frames (void);
static void     l3_forwarding_run_l3 (void);
static void     l3_forwarding_run_mac_vlan (void);
static void     l3_forwarding_print_menu (void);
static void     l3_forwarding_clear_table (void);

/***********************************************************************************************************************
 * @brief     This is RMAC link event callback function.
 *            This function updates per-port link state on LINK_ON / LINK_OFF events.
 * @param[in] p_args  Pointer to RMAC callback arguments containing event type and context.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_rmac_callback(ether_callback_args_t *p_args)
{
    for (uint32_t i = 0U; i < 2U; i++)
    {
        if (g_port[i].p_ether->p_ctrl == p_args->p_context)
        {
            if (ETHER_EVENT_LINK_ON  == p_args->event)
            {
                g_port[i].link_up = true;
                g_port[i].link_changed = true;
            }
            else if (ETHER_EVENT_LINK_OFF == p_args->event)
            {
                g_port[i].link_up = false;
                g_port[i].link_changed = true;
            }
            else
            {
                /* Do nothing */
            }
            break;
        }
    }
}

/***********************************************************************************************************************
 * @brief     This function prints link status message when the port link state changes.
 * @param[in] p_port  Pointer to the port context to check.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_check_link(l3_forwarding_port_ctx_t *p_port)
{
    if (!p_port->link_changed) { return; }
    p_port->link_changed = false;
    if (p_port->link_up)
    {
        APP_PRINT("[ETH%u] Link UP   IP=%u.%u.%u.%u  MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                  (unsigned)p_port->idx,
                  p_port->ip[0], p_port->ip[1], p_port->ip[2], p_port->ip[3],
                  p_port->mac[0], p_port->mac[1], p_port->mac[2],
                  p_port->mac[3], p_port->mac[4], p_port->mac[5]);
    }
    else
    {
        APP_PRINT("[ETH%u] Link DOWN\r\n", (unsigned)p_port->idx);
    }
}

/***********************************************************************************************************************
 * @brief     This function opens both RMAC channels and registers the link callback.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_rmac_init(void)
{
    fsp_err_t err;

    /* Initialize port context for ETH0 */
    g_port[0].idx     = L3_FORWARDING_ETH0;
    g_port[0].p_ether = &g_rmac0;
    memcpy(g_port[0].ip,  s_eth0_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memcpy(g_port[0].mac, g_layer3_switch0_mac_address_port0, L3_FORWARDING_MAC_ADDRESS_LENGTH);

    /* Initialize port context for ETH1 */
    g_port[1].idx     = L3_FORWARDING_ETH1;
    g_port[1].p_ether = &g_rmac1;
    memcpy(g_port[1].ip,  s_eth1_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memcpy(g_port[1].mac, g_layer3_switch0_mac_address_port1, L3_FORWARDING_MAC_ADDRESS_LENGTH);

    APP_PRINT("[INIT] Opening RMAC ETH0 ...\r\n");
    err = R_RMAC_Open(g_rmac0.p_ctrl, g_rmac0.p_cfg);
    APP_ERR_TRAP(err);
    err = R_RMAC_CallbackSet(g_rmac0.p_ctrl, l3_forwarding_rmac_callback, g_rmac0.p_ctrl, &s_cb_mem0);
    APP_ERR_TRAP(err);
    APP_PRINT("       ETH0 MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
              g_port[0].mac[0], g_port[0].mac[1], g_port[0].mac[2],
              g_port[0].mac[3], g_port[0].mac[4], g_port[0].mac[5]);

    APP_PRINT("[INIT] Opening RMAC ETH1 ...\r\n");
    err = R_RMAC_Open(g_rmac1.p_ctrl, g_rmac1.p_cfg);
    APP_ERR_TRAP(err);
    err = R_RMAC_CallbackSet(g_rmac1.p_ctrl, l3_forwarding_rmac_callback, g_rmac1.p_ctrl, &s_cb_mem1);
    APP_ERR_TRAP(err);
    APP_PRINT("       ETH1 MAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n",
              g_port[1].mac[0], g_port[1].mac[1], g_port[1].mac[2],
              g_port[1].mac[3], g_port[1].mac[4], g_port[1].mac[5]);
}

/***********************************************************************************************************************
 * @brief     This function verifies an installed forwarding entry by calling SearchTableEntry and printing the result.
 * @param[in] p_filter  Pointer to the frame filter matching the installed entry.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_verify_entry(const layer3_switch_frame_filter_t *p_filter)
{
    layer3_switch_table_entry_cfg_t found = { 0 };
    fsp_err_t err = R_LAYER3_SWITCH_SearchTableEntry(g_layer3_switch0.p_ctrl, p_filter, &found);
    if (FSP_SUCCESS == err)
    {
        APP_PRINT("       [verify] dst_ports=0x%X  src_ports=0x%X\r\n",
                  (unsigned)found.destination_ports, (unsigned)found.source_ports);
    }
    else
    {
        APP_PRINT("       [verify] WARN: not found (err=0x%X)\r\n", (unsigned)err);
    }
}

/***********************************************************************************************************************
 * @brief     This function reads the installed L3 entry list back from hardware and prints the entry count.
 *            Called after L3 route installation to confirm hardware accepted all entries.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_read_table(void)
{
    memset(s_l3_entries, 0, sizeof(s_l3_entries));
    memset(&s_l3_table,  0, sizeof(s_l3_table));

    s_l3_table.p_l3_entry_list = s_l3_entries;
    s_l3_table.l3_list_length  = L3_FORWARDING_TABLE_ENTRY_MAX;

    fsp_err_t err = R_LAYER3_SWITCH_GetTable(g_layer3_switch0.p_ctrl, &s_l3_table);

    if (FSP_SUCCESS == err)
    {
        APP_PRINT("[GetTable] entries=%u/%u OK\r\n",
                  (unsigned)s_l3_table.l3_list_length,
                  (unsigned)L3_FORWARDING_TABLE_ENTRY_MAX);
    }
    else
    {
        APP_PRINT("[GetTable] WARN err=0x%X\r\n", (unsigned)err);
    }
}

/***********************************************************************************************************************
 * @brief     This function prints whether a frame injected via CPU_INJECT was consumed by the GWCA hardware.
 * @param[in] consumed  Result returned by l3_forwarding_transmit_one_frame().
 * @param[in] p_frame   Label of the injected frame, e.g. "[MAC] Frame A".
 * @param[in] p_result  Expected outcome of the frame, e.g. "forwarded to ETH1" or "dropped".
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_report_tx(bool consumed, const char *p_frame, const char *p_result)
{
    if (consumed)
    {
        APP_PRINT("%s %s successfully\r\n", p_frame, p_result);
    }
    else
    {
        APP_PRINT("%s was NOT injected: the TX descriptor was not consumed by hardware\r\n", p_frame);
    }
}

/***********************************************************************************************************************
 * @brief     This function installs a drop entry (destination_ports=0) and injects a matching frame to verify
 *            that the ESWM silently drops the frame at silicon level (no egress on any physical port).
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_install_reject_entry(void)
{
    fsp_err_t err;

    if (L3_FORWARDING_MODE_MAC == g_forwarding_mode)
    {
        APP_PRINT("[REJECT] MAC entry: dst=%02X:%02X:%02X:%02X:%02X:%02X -> dst_ports=0 (drop)\r\n",
                  s_reject_mac[0], s_reject_mac[1], s_reject_mac[2],
                  s_reject_mac[3], s_reject_mac[4], s_reject_mac[5]);
        const layer3_switch_frame_filter_t f =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC,
            .p_destination_mac_address = s_reject_mac
        };
        const layer3_switch_table_entry_cfg_t e =
        {
            .entry_enable = true,
            .destination_ports = 0U,
            .source_ports = L3_FORWARDING_SRC_PORTS_ALL
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f, &e);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("R_LAYER3_SWITCH_AddTableEntry API failed\r\n");
            return;
        }
        l3_forwarding_verify_entry(&f);

        uint8_t frame[L3_FORWARDING_MIN_FRAME_SIZE];
        memset(frame, 0, sizeof(frame));
        memcpy(frame,     s_reject_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
        memcpy(frame + 6, s_eth0_mac,   L3_FORWARDING_MAC_ADDRESS_LENGTH);
        frame[12] = 0x88U;
        frame[13] = 0xB5U;
        APP_PRINT("[REJECT] Injecting a frame to dst=%02X:%02X:%02X:%02X:%02X:%02X"
                  " (rejected by the table entry) ...\r\n",
                  s_reject_mac[0], s_reject_mac[1], s_reject_mac[2],
                  s_reject_mac[3], s_reject_mac[4], s_reject_mac[5]);
        bool consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
        l3_forwarding_report_tx(consumed, "[REJECT] Frame", "dropped");
        APP_PRINT("       Expected: the frame to eth.dst==%02x:%02x:%02x:%02x:%02x:%02x is dropped,"
                  " nothing is received on any port\r\n",
                  s_reject_mac[0], s_reject_mac[1], s_reject_mac[2],
                  s_reject_mac[3], s_reject_mac[4], s_reject_mac[5]);
    }
    else if (L3_FORWARDING_MODE_VLAN == g_forwarding_mode)
    {
        APP_PRINT("[REJECT] VLAN entry: VID=%u -> dst_ports=0 (drop)\r\n",
                  (unsigned)L3_FORWARDING_REJECT_VID);
        const layer3_switch_frame_filter_t f =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_VLAN,
            .vlan_c_tag =
            {
                .id = L3_FORWARDING_REJECT_VID
            }
        };
        const layer3_switch_table_entry_cfg_t e =
        {
            .entry_enable = true,
            .destination_ports = 0U,
            .source_ports = L3_FORWARDING_SRC_PORTS_ALL
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f, &e);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("R_LAYER3_SWITCH_AddTableEntry API failed\r\n");
            return;
        }
        l3_forwarding_verify_entry(&f);

        uint8_t vframe[L3_FORWARDING_MIN_FRAME_SIZE];
        memset(vframe, 0, sizeof(vframe));
        memset(vframe, 0xFFU, L3_FORWARDING_MAC_ADDRESS_LENGTH);
        memcpy(vframe + 6, s_eth0_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
        vframe[12] = 0x81U;
        vframe[13] = 0x00U;
        vframe[14] = 0x00U;
        vframe[15] = (uint8_t)(L3_FORWARDING_REJECT_VID & 0xFFU);
        vframe[16] = 0x88U;
        vframe[17] = 0xB5U;
        APP_PRINT("[REJECT] Injecting a frame with VID=%u (rejected by the table entry) ...\r\n",
                  (unsigned)L3_FORWARDING_REJECT_VID);
        bool consumed = l3_forwarding_transmit_one_frame(vframe, (uint16_t)sizeof(vframe), 0U, 0U);
        l3_forwarding_report_tx(consumed, "[REJECT] Frame", "dropped");
        APP_PRINT("       Expected: the frame with VID=%u is dropped, nothing is received on any port\r\n",
                  (unsigned)L3_FORWARDING_REJECT_VID);
    }
    else if (L3_FORWARDING_MODE_LAYER3 == g_forwarding_mode)
    {
        /* L3 reject uses C-TAG+IPv4+UDP to match the full-key stream filter */
        static uint8_t s_reject_src_ip[L3_FORWARDING_IP_ADDRESS_LENGTH];
        static uint8_t s_reject_dst_ip[L3_FORWARDING_IP_ADDRESS_LENGTH];
        static uint8_t s_reject_eth0_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH] = L3_FORWARDING_ETH0_MAC;
        static uint8_t s_reject_pc1_mac[L3_FORWARDING_MAC_ADDRESS_LENGTH];
        memcpy(s_reject_src_ip,  s_pc1_ip,               L3_FORWARDING_IP_ADDRESS_LENGTH);
        memcpy(s_reject_dst_ip,  s_reject_ip,             L3_FORWARDING_IP_ADDRESS_LENGTH);
        memcpy(s_reject_pc1_mac, g_l3_forwarding_pc1_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);

        APP_PRINT("[REJECT] L3 entry: dst_ip=%u.%u.%u.%u -> dst_ports=0 (drop)\r\n",
                  s_reject_ip[0], s_reject_ip[1], s_reject_ip[2], s_reject_ip[3]);
        const layer3_switch_frame_filter_t f =
        {
            .entry_type                = LAYER3_SWITCH_TABLE_ENTRY_TYPE_LAYER3,
            .ip_version                = LAYER3_SWITCH_IP_VERSION_IPV4,
            .p_destination_ip_address  = s_reject_dst_ip,
            .p_source_ip_address       = s_reject_src_ip,
            .p_destination_mac_address = s_reject_eth0_mac,
            .p_source_mac_address      = s_reject_pc1_mac,
            .vlan_c_tag                = { .id = L3_FORWARDING_L3_VID_A },
            .protocol                  = LAYER3_SWITCH_IP_PROTOCOL_UDP,
            .layer4_source_port        = L3_FORWARDING_UDP_PORT,
            .layer4_destination_port   = L3_FORWARDING_UDP_PORT,
        };
        const layer3_switch_table_entry_cfg_t e =
        {
            .entry_enable = true,
            .destination_ports = 0U,
            .source_ports = L3_FORWARDING_SRC_PORTS_ALL
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f, &e);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("R_LAYER3_SWITCH_AddTableEntry API failed\r\n");
            return;
        }
        l3_forwarding_verify_entry(&f);

        uint8_t lframe[L3_FORWARDING_TX_BUF_SIZE];
        l3_forwarding_build_ctag_udp_frame(lframe,
                                           s_eth0_mac, g_l3_forwarding_pc1_mac,
                                           L3_FORWARDING_L3_VID_A,
                                           s_pc1_ip, s_reject_ip, 0x0010U);
        APP_PRINT("[REJECT] Injecting a frame to dst_ip=%u.%u.%u.%u (rejected by the table entry) ...\r\n",
                  s_reject_ip[0], s_reject_ip[1], s_reject_ip[2], s_reject_ip[3]);
        bool consumed = l3_forwarding_transmit_one_frame(lframe, L3_FORWARDING_MIN_FRAME_SIZE, 0U, 0U);
        l3_forwarding_report_tx(consumed, "[REJECT] Frame", "dropped");
        APP_PRINT("       Expected: the frame to ip.dst==%u.%u.%u.%u is dropped, nothing is received on any port\r\n",
                  s_reject_ip[0], s_reject_ip[1], s_reject_ip[2], s_reject_ip[3]);
    }
    else
    {
        /* Do nothing */
    }
}

/***********************************************************************************************************************
 * @brief     This function installs a MAC entry forwarding to both ETH0 and ETH1 simultaneously,
 *            then injects one frame. Both ETH0 and ETH1 should egress the same frame.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_install_dual_port_entry(void)
{
    APP_PRINT("[DUAL-PORT] MAC entry: dst=%02X:%02X:%02X:%02X:%02X:%02X -> ETH0+ETH1\r\n",
              s_dual_port_mac[0], s_dual_port_mac[1], s_dual_port_mac[2],
              s_dual_port_mac[3], s_dual_port_mac[4], s_dual_port_mac[5]);
    const layer3_switch_frame_filter_t f =
    {
        .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC,
        .p_destination_mac_address = s_dual_port_mac
    };
    const layer3_switch_table_entry_cfg_t e =
    {
        .entry_enable      = true,
        .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT0 | LAYER3_SWITCH_PORT_BITMASK_PORT1,
        .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
        .security_enable   = false,
    };
    fsp_err_t err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f, &e);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_LAYER3_SWITCH_AddTableEntry API failed\r\n");
        return;
    }
    l3_forwarding_verify_entry(&f);

    uint8_t frame[L3_FORWARDING_MIN_FRAME_SIZE];
    memset(frame, 0, sizeof(frame));
    memcpy(frame,     s_dual_port_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(frame + 6, s_eth0_mac,      L3_FORWARDING_MAC_ADDRESS_LENGTH);
    frame[12] = 0x88U;
    frame[13] = 0xB5U;
    APP_PRINT("[DUAL-PORT] Injecting a frame to ETH0 and ETH1 ...\r\n");
    bool consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
    l3_forwarding_report_tx(consumed, "[DUAL-PORT] Frame", "forwarded to ETH0 and ETH1");
    APP_PRINT("       Verify on NIC1 and NIC2: eth.dst==%02x:%02x:%02x:%02x:%02x:%02x\r\n",
              s_dual_port_mac[0], s_dual_port_mac[1], s_dual_port_mac[2],
              s_dual_port_mac[3], s_dual_port_mac[4], s_dual_port_mac[5]);
}

/***********************************************************************************************************************
 * @brief     This function resets/configures the ESWM table context, sets VLAN mode, enables per-port lookup tables,
 *            and configures the L3 stream filter bitmask for L3 mode. It does not install MAC/VLAN/L3 entries;
 *            entries are added later by l3_forwarding_install_entries().
 *            L3 mode uses C-TAG mode; CPU_INJECT path maps to hardware internally and is not a physical Ethernet port.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_configure_table(void)
{
    fsp_err_t err;

    /* Initialize table backing storage */
    memset(s_l3_entries, 0, sizeof(s_l3_entries));
    memset(&s_l3_table,  0, sizeof(s_l3_table));

    layer3_switch_table_cfg_t tbl_cfg = { 0 };

    tbl_cfg.p_table                    = NULL;
    tbl_cfg.unsecure_entry_maximum_num = L3_FORWARDING_TABLE_ENTRY_MAX;
    tbl_cfg.mac_entry_aging_enable     = false;

    switch (g_forwarding_mode)
    {
        case L3_FORWARDING_MODE_MAC:
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].mac_table_enable                = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].mac_table_enable                = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].mac_table_enable     = true;
            tbl_cfg.vlan_mode = LAYER3_SWITCH_VLAN_MODE_NO_VLAN;
            APP_PRINT("[MAC]  Configuring MAC forwarding table ...\r\n");
            break;

        case L3_FORWARDING_MODE_VLAN:
            /* CPU_INJECT TAG_BASED: ESWM parses the 802.1Q C-TAG from CPU_INJECT frames */
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].vlan_table_enable                     = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].vlan_table_enable                     = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_table_enable          = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_ingress_mode          = LAYER3_SWITCH_VLAN_INGRESS_MODE_TAG_BASED;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_egress_mode           = LAYER3_SWITCH_VLAN_EGRESS_MODE_C_TAG;
            tbl_cfg.vlan_mode = LAYER3_SWITCH_VLAN_MODE_C_TAG;
            APP_PRINT("[VLAN] Configuring VLAN forwarding table (CPU_INJECT TAG_BASED) ...\r\n");
            break;

        case L3_FORWARDING_MODE_LAYER3:
            /* C-TAG mode required for L3 stream pipeline */
            tbl_cfg.vlan_mode = LAYER3_SWITCH_VLAN_MODE_C_TAG;

            /* Enable VLAN table on all ports (prerequisite for L3 stream lookup) */
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].vlan_table_enable                     = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].vlan_table_enable                     = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_table_enable          = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_ingress_mode          = LAYER3_SWITCH_VLAN_INGRESS_MODE_TAG_BASED;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].vlan_egress_mode                      = LAYER3_SWITCH_VLAN_EGRESS_MODE_C_TAG;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].vlan_egress_mode                      = LAYER3_SWITCH_VLAN_EGRESS_MODE_C_TAG;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].vlan_egress_mode           = LAYER3_SWITCH_VLAN_EGRESS_MODE_C_TAG;

            /* Enable L3 table and IPv4 filter on all ports */
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].layer3_table_enable                   = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH0].layer3_ipv4_filter_enable             = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].layer3_table_enable                   = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_ETH1].layer3_ipv4_filter_enable             = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].layer3_table_enable        = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].layer3_ipv4_filter_enable  = true;
            tbl_cfg.port_cfg_list[L3_FORWARDING_CPU_INJECT_PORT].layer3_l2_filter_enable    = true;

            /* Full-key stream filter: all 8 fields included for exact stream ID match */
            tbl_cfg.l3_stream_filter_cfg.filter_field_bitmask =
                LAYER3_SWITCH_L3_FILTER_BITMASK_MAC_DESTINATION    |
                LAYER3_SWITCH_L3_FILTER_BITMASK_MAC_SOURCE         |
                LAYER3_SWITCH_L3_FILTER_BITMASK_CTAG_ID            |
                LAYER3_SWITCH_L3_FILTER_BITMASK_PROTOCOL           |
                LAYER3_SWITCH_L3_FILTER_BITMASK_IP_SOURCE_ADDRESS  |
                LAYER3_SWITCH_L3_FILTER_BITMASK_IP_DESTINATION_ADDRESS |
                LAYER3_SWITCH_L3_FILTER_BITMASK_IP_SOURCE_PORT     |
                LAYER3_SWITCH_L3_FILTER_BITMASK_IP_DESTINATION_PORT;

            APP_PRINT("[L3]   Configuring L3 IP forwarding (C-TAG, CPU_INJECT TAG_BASED) ...\r\n");
            break;

        default:
            break;
    }

    err = R_LAYER3_SWITCH_ConfigureTable(g_layer3_switch0.p_ctrl, &tbl_cfg);
    APP_ERR_TRAP(err);
    APP_PRINT("       Table configured OK\r\n");
}

/***********************************************************************************************************************
 * @brief     This function adds the actual forwarding entries after l3_forwarding_configure_table() has configured
 *            the base ESWM mode. Entries are installed by R_LAYER3_SWITCH_AddTableEntry() and verified by
 *            SearchTableEntry.
 *            CPU_INJECT frames enter ESWM through the internal GWCA port; ESWM hardware egresses them on
 *            the selected physical port (ETH0 or ETH1) based on the matching table entry.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_install_entries(void)
{
    fsp_err_t err;

    /* MAC mode */
    if (L3_FORWARDING_MODE_MAC == g_forwarding_mode)
    {
        APP_PRINT("[MAC]  Installing MAC entries ...\r\n");

        const layer3_switch_frame_filter_t f_mac_nic2 =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC,
            .p_destination_mac_address = g_l3_forwarding_pc2_mac
        };
        const layer3_switch_table_entry_cfg_t e_mac_nic2 =
        {
            .entry_enable      = true,
            .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT1,
            .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
            .security_enable   = false,
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f_mac_nic2, &e_mac_nic2);
        APP_ERR_TRAP(err);
        APP_PRINT("       CPU_INJECT->ETH1  dst=%02X:%02X:%02X:%02X:%02X:%02X (NIC2)\r\n",
                  g_l3_forwarding_pc2_mac[0], g_l3_forwarding_pc2_mac[1], g_l3_forwarding_pc2_mac[2],
                  g_l3_forwarding_pc2_mac[3], g_l3_forwarding_pc2_mac[4], g_l3_forwarding_pc2_mac[5]);
        l3_forwarding_verify_entry(&f_mac_nic2);

        const layer3_switch_frame_filter_t f_mac_nic1 =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC,
            .p_destination_mac_address = g_l3_forwarding_pc1_mac
        };
        const layer3_switch_table_entry_cfg_t e_mac_nic1 =
        {
            .entry_enable      = true,
            .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT0,
            .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
            .security_enable   = false,
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f_mac_nic1, &e_mac_nic1);
        APP_ERR_TRAP(err);
        APP_PRINT("       CPU_INJECT->ETH0  dst=%02X:%02X:%02X:%02X:%02X:%02X (NIC1)\r\n",
                  g_l3_forwarding_pc1_mac[0], g_l3_forwarding_pc1_mac[1], g_l3_forwarding_pc1_mac[2],
                  g_l3_forwarding_pc1_mac[3], g_l3_forwarding_pc1_mac[4], g_l3_forwarding_pc1_mac[5]);
        l3_forwarding_verify_entry(&f_mac_nic1);
        l3_forwarding_install_reject_entry();
        l3_forwarding_install_dual_port_entry();
        APP_PRINT("\r\n[FWD-VS-DROP] MAC summary:\r\n"
                  "  FORWARD : dst=NIC2_MAC -> ETH1/NIC2\r\n"
                  "  NO-ENTRY: dst=UNKNOWN  -> silent drop (no table match)\r\n"
                  "  REJECT  : dst=%02X:%02X:%02X:%02X:%02X:%02X (dst_ports=0) -> drop\r\n\r\n",
                  s_reject_mac[0], s_reject_mac[1], s_reject_mac[2],
                  s_reject_mac[3], s_reject_mac[4], s_reject_mac[5]);
        return;
    }

    /* VLAN mode */
    if (L3_FORWARDING_MODE_VLAN == g_forwarding_mode)
    {
        APP_PRINT("[VLAN] Installing VLAN entries (VID=%u->ETH0, VID=%u->ETH1) ...\r\n",
                  (unsigned)L3_FORWARDING_VLAN_ID_ETH0, (unsigned)L3_FORWARDING_VLAN_ID_ETH1);

        const layer3_switch_frame_filter_t f_vlan_e0 =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_VLAN,
            .vlan_c_tag =
            {
                .id = L3_FORWARDING_VLAN_ID_ETH0
            }
        };
        const layer3_switch_table_entry_cfg_t e_vlan_e0 =
        {
            .entry_enable = true,
            .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT0,
            .source_ports = L3_FORWARDING_SRC_PORTS_ALL
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f_vlan_e0, &e_vlan_e0);
        APP_ERR_TRAP(err);
        APP_PRINT("       VID=%u -> ETH0\r\n",  (unsigned)L3_FORWARDING_VLAN_ID_ETH0);
        l3_forwarding_verify_entry(&f_vlan_e0);

        const layer3_switch_frame_filter_t f_vlan_e1 =
        {
            .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_VLAN,
            .vlan_c_tag =
            {
                .id = L3_FORWARDING_VLAN_ID_ETH1
            }
        };
        const layer3_switch_table_entry_cfg_t e_vlan_e1 =
        {
            .entry_enable = true,
            .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT1,
            .source_ports = L3_FORWARDING_SRC_PORTS_ALL
        };
        err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &f_vlan_e1, &e_vlan_e1);
        APP_ERR_TRAP(err);
        APP_PRINT("       VID=%u -> ETH1\r\n", (unsigned)L3_FORWARDING_VLAN_ID_ETH1);
        l3_forwarding_verify_entry(&f_vlan_e1);

        APP_PRINT("[VLAN-UNICAST] FrameA dst=%02X:%02X:%02X:%02X:%02X:%02X (NIC1) VID=%u -> ETH0\r\n",
                  g_l3_forwarding_pc1_mac[0], g_l3_forwarding_pc1_mac[1], g_l3_forwarding_pc1_mac[2],
                  g_l3_forwarding_pc1_mac[3], g_l3_forwarding_pc1_mac[4], g_l3_forwarding_pc1_mac[5],
                  (unsigned)L3_FORWARDING_VLAN_ID_ETH0);
        APP_PRINT("[VLAN-UNICAST] FrameB dst=%02X:%02X:%02X:%02X:%02X:%02X (NIC2) VID=%u -> ETH1\r\n",
                  g_l3_forwarding_pc2_mac[0], g_l3_forwarding_pc2_mac[1], g_l3_forwarding_pc2_mac[2],
                  g_l3_forwarding_pc2_mac[3], g_l3_forwarding_pc2_mac[4], g_l3_forwarding_pc2_mac[5],
                  (unsigned)L3_FORWARDING_VLAN_ID_ETH1);
        APP_PRINT("[VLAN-UNICAST] Unicast dst MAC -- no broadcast flooding\r\n");
        l3_forwarding_install_reject_entry();
        APP_PRINT("\r\n[FWD-VS-DROP] VLAN summary:\r\n"
                  "  FORWARD : VID=10->ETH0/NIC1  VID=20->ETH1/NIC2\r\n"
                  "  NO-ENTRY: VID=50 (no entry) -> silent drop\r\n"
                  "  REJECT  : VID=%u (dst_ports=0) -> drop\r\n\r\n",
                  (unsigned)L3_FORWARDING_REJECT_VID);
        return;
    }

    /* L3 mode
     * Install VLAN table entries for C-TAG VIDs first.
     * ESWM VLAN lookup must succeed before L3 stream filter can match. */
    APP_PRINT("[L3]   Installing VLAN entries VID=%u and VID=%u for C-TAG L3 path ...\r\n",
              (unsigned)L3_FORWARDING_L3_VID_A, (unsigned)L3_FORWARDING_L3_VID_B);

    const layer3_switch_table_entry_cfg_t ev_l3 =
    {
        .entry_enable      = true,
        .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT0 |
                             LAYER3_SWITCH_PORT_BITMASK_PORT1 |
                             LAYER3_SWITCH_PORT_BITMASK_PORT2,
        .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
    };
    const layer3_switch_frame_filter_t fv_l3_a =
    {
        .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_VLAN,
        .vlan_c_tag =
        {
            .id = L3_FORWARDING_L3_VID_A
        }
    };
    err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &fv_l3_a, &ev_l3);
    APP_ERR_TRAP(err);
    l3_forwarding_verify_entry(&fv_l3_a);

    const layer3_switch_frame_filter_t fv_l3_b =
    {
        .entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_VLAN,
        .vlan_c_tag =
        {
            .id = L3_FORWARDING_L3_VID_B
        }
    };
    err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &fv_l3_b, &ev_l3);
    APP_ERR_TRAP(err);
    l3_forwarding_verify_entry(&fv_l3_b);

    APP_PRINT("[L3]   Installing IP routes ...\r\n");

    /* Route A: CPU_INJECT -> ETH1 (NIC2), dst_ip=PC2 */
    memcpy(s_dst_ip_pc2, s_pc2_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memset(&s_update_to_pc2, 0, sizeof(s_update_to_pc2));
    s_update_to_pc2.enable_destination_ports  = LAYER3_SWITCH_PORT_BITMASK_PORT1;
    s_update_to_pc2.update_field_bitmask      =
        LAYER3_SWITCH_L3_UPDATE_BITMASK_TTL | LAYER3_SWITCH_L3_UPDATE_BITMASK_MAC_DESTINATION;
    s_update_to_pc2.p_mac_destination_address = g_l3_forwarding_pc2_mac;

    const layer3_switch_frame_filter_t fa =
    {
        .entry_type                = LAYER3_SWITCH_TABLE_ENTRY_TYPE_LAYER3,
        .ip_version                = LAYER3_SWITCH_IP_VERSION_IPV4,
        .p_destination_ip_address  = s_dst_ip_pc2,
        .p_source_ip_address       = (uint8_t *)s_pc1_ip,
        .p_destination_mac_address = (uint8_t *)s_eth0_mac,
        .p_source_mac_address      = g_l3_forwarding_pc1_mac,
        .vlan_c_tag                = { .id = L3_FORWARDING_L3_VID_A },
        .protocol                  = LAYER3_SWITCH_IP_PROTOCOL_UDP,
        .layer4_source_port        = L3_FORWARDING_UDP_PORT,
        .layer4_destination_port   = L3_FORWARDING_UDP_PORT,
    };
    const layer3_switch_table_entry_cfg_t ea =
    {
        .entry_enable      = true,
        .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT1,
        .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
        .layer3 = { .p_update_configs = &s_update_to_pc2, .number_of_configs = 1U },
    };
    err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &fa, &ea);
    APP_ERR_TRAP(err);
    APP_PRINT("       Route A: CPU_INJECT->ETH1  dst=%u.%u.%u.%u  TTL-- MAC_DST=NIC2\r\n",
              s_pc2_ip[0], s_pc2_ip[1], s_pc2_ip[2], s_pc2_ip[3]);
    l3_forwarding_verify_entry(&fa);

    /* Route B: CPU_INJECT -> ETH0 (NIC1), dst_ip=PC1 */
    memcpy(s_dst_ip_pc1, s_pc1_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memset(&s_update_to_pc1, 0, sizeof(s_update_to_pc1));
    s_update_to_pc1.enable_destination_ports  = LAYER3_SWITCH_PORT_BITMASK_PORT0;
    s_update_to_pc1.update_field_bitmask      =
        LAYER3_SWITCH_L3_UPDATE_BITMASK_TTL | LAYER3_SWITCH_L3_UPDATE_BITMASK_MAC_DESTINATION;
    s_update_to_pc1.p_mac_destination_address = g_l3_forwarding_pc1_mac;

    const layer3_switch_frame_filter_t fb =
    {
        .entry_type                = LAYER3_SWITCH_TABLE_ENTRY_TYPE_LAYER3,
        .ip_version                = LAYER3_SWITCH_IP_VERSION_IPV4,
        .p_destination_ip_address  = s_dst_ip_pc1,
        .p_source_ip_address       = (uint8_t *)s_pc2_ip,
        .p_destination_mac_address = (uint8_t *)s_eth1_mac,
        .p_source_mac_address      = g_l3_forwarding_pc2_mac,
        .vlan_c_tag                = { .id = L3_FORWARDING_L3_VID_B },
        .protocol                  = LAYER3_SWITCH_IP_PROTOCOL_UDP,
        .layer4_source_port        = L3_FORWARDING_UDP_PORT,
        .layer4_destination_port   = L3_FORWARDING_UDP_PORT,
    };
    const layer3_switch_table_entry_cfg_t eb =
    {
        .entry_enable      = true,
        .destination_ports = LAYER3_SWITCH_PORT_BITMASK_PORT0,
        .source_ports      = L3_FORWARDING_SRC_PORTS_ALL,
        .layer3 = { .p_update_configs = &s_update_to_pc1, .number_of_configs = 1U },
    };
    err = R_LAYER3_SWITCH_AddTableEntry(g_layer3_switch0.p_ctrl, &fb, &eb);
    APP_ERR_TRAP(err);
    APP_PRINT("       Route B: CPU_INJECT->ETH0  dst=%u.%u.%u.%u  TTL-- MAC_DST=NIC1\r\n",
              s_pc1_ip[0], s_pc1_ip[1], s_pc1_ip[2], s_pc1_ip[3]);
    l3_forwarding_verify_entry(&fb);
    APP_PRINT("[L3]   2 routes installed OK\r\n");
    l3_forwarding_install_reject_entry();
    APP_PRINT("\r\n[FWD-VS-DROP] L3 summary:\r\n"
              "  FORWARD : dst_ip=%u.%u.%u.%u -> ETH1/NIC2  TTL-- MAC_DST rewrite\r\n"
              "  NO-ENTRY: dst_ip=1.2.3.4 (no route) -> silent drop\r\n"
              "  REJECT  : dst_ip=%u.%u.%u.%u (dst_ports=0) -> drop\r\n\r\n",
              s_pc2_ip[0], s_pc2_ip[1], s_pc2_ip[2], s_pc2_ip[3],
              s_reject_ip[0], s_reject_ip[1], s_reject_ip[2], s_reject_ip[3]);
}

/***********************************************************************************************************************
 * @brief     This function polls RMAC link state on both ETH0 and ETH1 until both report link UP.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_wait_links(void)
{
    APP_PRINT("[INIT] Waiting for ETH0 and ETH1 link UP ...\r\n");
    while (!g_port[0].link_up || !g_port[1].link_up)
    {
        R_RMAC_LinkProcess(g_rmac0.p_ctrl);
        R_RMAC_LinkProcess(g_rmac1.p_ctrl);
        l3_forwarding_check_link(&g_port[0]);
        l3_forwarding_check_link(&g_port[1]);
        R_BSP_SoftwareDelay(L3_FORWARDING_LINK_POLL_MS, BSP_DELAY_UNITS_MILLISECONDS);
    }
    APP_PRINT("[INIT] Both ETH0+ETH1 UP -- ESWM forwarding ACTIVE\r\n\r\n");
}

/***********************************************************************************************************************
 * @brief     This function sends an ARP reply when the port receives an ARP request targeting its IP address.
 *            In L3 mode, CPU handles ARP via RMAC while ESWM hardware handles IP forwarding in silicon.
 * @param[in] p_port  Pointer to the port context (holds IP, MAC, and TX buffer).
 * @param[in] p_req   Pointer to the received ARP frame.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_arp_reply(l3_forwarding_port_ctx_t *p_port, const l3_arp_frame_t *p_req)
{
    if (L3_NTOHS(p_req->arp.opcode) != L3_FORWARDING_ARP_OP_REQUEST)
    {
        return;
    }
    if (memcmp(p_req->arp.target_ip, p_port->ip, L3_FORWARDING_IP_ADDRESS_LENGTH) != 0)
    {
        return;
    }

    p_port->cnt_arp_rx++;

    uint8_t *p_tx = (p_port->idx == 0U) ? g_tx_arp0 : g_tx_arp1;
    memset(p_tx, 0, L3_FORWARDING_MIN_FRAME_SIZE);

    l3_arp_frame_t *p_reply = (l3_arp_frame_t *)p_tx;
    memcpy(p_reply->eth.dst.b, p_req->eth.src.b, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(p_reply->eth.src.b, p_port->mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    p_reply->eth.etype  = L3_HTONS(L3_FORWARDING_ETHERTYPE_ARP);
    p_reply->arp.htype  = L3_HTONS(1U);
    p_reply->arp.ptype  = L3_HTONS(L3_FORWARDING_ETHERTYPE_IPV4);
    p_reply->arp.hlen   = (uint8_t)L3_FORWARDING_MAC_ADDRESS_LENGTH;
    p_reply->arp.plen   = (uint8_t)L3_FORWARDING_IP_ADDRESS_LENGTH;
    p_reply->arp.opcode = L3_HTONS(L3_FORWARDING_ARP_OP_REPLY);
    memcpy(p_reply->arp.sender_mac.b, p_port->mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(p_reply->arp.sender_ip, p_port->ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memcpy(p_reply->arp.target_mac.b, p_req->arp.sender_mac.b, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(p_reply->arp.target_ip, p_req->arp.sender_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);

    fsp_err_t err = R_RMAC_Write(p_port->p_ether->p_ctrl, p_tx, L3_FORWARDING_MIN_FRAME_SIZE);
    if (FSP_SUCCESS == err)
    {
        p_port->cnt_arp_tx++;
    }
    else
    {
        APP_ERR_PRINT("R_RMAC_Write API failed\r\n");
    }
}

/***********************************************************************************************************************
 * @brief     This function reads one frame from the RMAC receive ring and dispatches to ARP reply if applicable.
 * @param[in] p_port  Pointer to the port context.
 * @param[in] p_buf   Pointer to the receive buffer (must be L3_FORWARDING_RX_BUF_SIZE bytes).
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_rx_arp(l3_forwarding_port_ctx_t *p_port, uint8_t *p_buf)
{
    uint32_t rx_len = L3_FORWARDING_RX_BUF_SIZE;
    fsp_err_t err   = R_RMAC_Read(p_port->p_ether->p_ctrl, p_buf, &rx_len);
    if (FSP_SUCCESS != err) { return; }
    if (rx_len < L3_FORWARDING_ETH_HDR_LEN)
    {
        return;
    }

    l3_eth_hdr_t * p_eth = (l3_eth_hdr_t *)p_buf;
    if (L3_FORWARDING_ETHERTYPE_ARP != L3_NTOHS(p_eth->etype))
    {
        return;
    }
    if (rx_len < sizeof(l3_arp_frame_t))
    {
        return;
    }

    l3_forwarding_arp_reply(p_port, (l3_arp_frame_t *)p_buf);
}

/***********************************************************************************************************************
 * @brief     This function computes IPv4 header checksum.
 * @param[in] p_hdr   Pointer to the IPv4 header byte array.
 * @param[in] hdr_len IPv4 header length in bytes (normally 20 for IHL=5).
 * @retval    uint16_t  Ones-complement checksum to store in ip.checksum field.
 **********************************************************************************************************************/
static uint16_t l3_forwarding_ipv4_checksum(const uint8_t *p_hdr, uint16_t hdr_len)
{
    uint32_t sum = 0U;
    for (uint16_t i = 0U; i < hdr_len; i += 2U)
    {
        sum += (uint32_t)(((uint16_t)p_hdr[i] << 8U) | (uint16_t)p_hdr[i + 1U]);
    }
    while (sum >> 16U) {sum = (sum & 0xFFFFU) + (sum >> 16U);}
    return (uint16_t)(~sum);
}

/***********************************************************************************************************************
 * @brief     This function injects one Ethernet frame via a fresh GWCA TX descriptor queue (one-shot pattern).
 *            Creates a new descriptor queue per call to avoid FSP internal cursor drift.
 *            After a 50 ms wait, the first descriptor is checked: dt=0x04 (FEMPTY) means hardware consumed it.
 *            The caller is responsible for printing the injected-frame and verification lines around this call.
 * @param[in] p_frame  Pointer to the frame byte array.
 * @param[in] len      Frame length in bytes.
 * @param[in] tx_fmt   Descriptor fmt bit: 0=forwarding (ESWM table lookup).
 * @param[in] tx_dv    Destination vector for direct mode.
 * @retval    true   The frame was injected and the descriptor was consumed by hardware.
 * @retval    false  Any FSP API returned an error, or the descriptor was not consumed.
 **********************************************************************************************************************/
static bool l3_forwarding_transmit_one_frame(const uint8_t *p_frame,
                                             uint16_t       len,
                                             uint8_t        tx_fmt,
                                             uint8_t        tx_dv)
{
    fsp_err_t  err;
    uint32_t   q_idx = 0U;
    layer3_switch_descriptor_t desc = { 0 };
    uint64_t tx_desc_pa  = 0U;
    uint64_t tx_frame_pa = 0U;

    memset(s_tx_desc,  0, sizeof(s_tx_desc));
    memset(s_tx_frame, 0, sizeof(s_tx_frame));

    const layer3_switch_descriptor_queue_cfg_t q_cfg =
    {
        .descriptor_format     = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
        .type                  = LAYER3_SWITCH_QUEUE_TYPE_TX,
        .write_back_mode       = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
        .p_descriptor_array    = s_tx_desc,
        .p_ts_descriptor_array = NULL,
        .rx_timestamp_storage  = LAYER3_SWITCH_RX_TIMESTAMP_STORAGE_DISABLE,
        .array_length          = L3_FORWARDING_DESC_QUEUE_LEN,
        .ports                 = LAYER3_SWITCH_PORT_BITMASK_PORT0 |
                                 LAYER3_SWITCH_PORT_BITMASK_PORT1 |
                                 LAYER3_SWITCH_PORT_BITMASK_PORT2,
    };

    #if (1 == BSP_LP64_SUPPORT)

    /* CA55 uses MMU: GWCA requires physical addresses, not CPU virtual addresses. */
    err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t)s_tx_desc, &tx_desc_pa);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_BSP_MmuVatoPa API failed\r\n");
        return false;
    }

    err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t)s_tx_frame, &tx_frame_pa);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_BSP_MmuVatoPa API failed\r\n");
        return false;
    }

    #else

    /* CR52 uses the CPU-visible address directly for GWCA DMA. */
    tx_desc_pa  = (uint64_t) (uintptr_t) s_tx_desc;
    tx_frame_pa = (uint64_t) (uintptr_t) s_tx_frame;

    #endif

    err = R_LAYER3_SWITCH_CreateDescriptorQueue(g_layer3_switch0.p_ctrl, &q_idx, &q_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_LAYER3_SWITCH_CreateDescriptorQueue API failed\r\n");
        return false;
    }

    /* LEMPTY terminator: slot 1 wraps back to slot 0 (ring structure required by GWCA) */
    s_tx_desc[1].basic.dt    = LAYER3_SWITCH_DESCRIPTOR_TYPE_LEMPTY;
    s_tx_desc[1].basic.ptr_h = (uint8_t)(tx_desc_pa >> 32U);
    s_tx_desc[1].basic.ptr_l = (uint32_t)tx_desc_pa;

    memcpy(s_tx_frame, p_frame, len);

    desc.basic.dt     = LAYER3_SWITCH_DESCRIPTOR_TYPE_FSINGLE;
    desc.basic.die    = 1U;
    desc.basic.ds_l   = (uint8_t)(len & 0xFFU);
    desc.basic.ds_h   = (uint8_t)((len >> 8U) & 0x0FU);
    desc.basic.ptr_h = (uint8_t)(tx_frame_pa >> 32U);
    desc.basic.ptr_l = (uint32_t)tx_frame_pa;
    desc.info1_tx.fmt = (tx_fmt & 0x01U);
    desc.info1_tx.dv  = (tx_dv  & 0x7FU);

    err = R_LAYER3_SWITCH_SetDescriptor(g_layer3_switch0.p_ctrl, q_idx, &desc);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_LAYER3_SWITCH_SetDescriptor API failed\r\n");
        return false;
    }

    err = R_LAYER3_SWITCH_StartDescriptorQueue(g_layer3_switch0.p_ctrl, q_idx);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_LAYER3_SWITCH_StartDescriptorQueue API failed\r\n");
        return false;
    }

    R_BSP_SoftwareDelay(50U, BSP_DELAY_UNITS_MILLISECONDS);

    bool consumed = (LAYER3_SWITCH_DESCRIPTOR_TYPE_FEMPTY == s_tx_desc[0].basic.dt);

    /* Reclaim descriptor; err=0x21 (no more descriptors) is expected for one-shot queue */
    layer3_switch_descriptor_t d_reclaim = { 0 };
    fsp_err_t get_err = R_LAYER3_SWITCH_GetDescriptor(g_layer3_switch0.p_ctrl, q_idx, &d_reclaim);
    (void)get_err;

    return consumed;
}

/***********************************************************************************************************************
 * @brief     This function builds a C-TAG + IPv4 + UDP Ethernet frame in the provided buffer.
 *            Frame layout: [dst_mac|src_mac|TPID=0x8100|TCI|EtherType=0x0800|IPv4 hdr|UDP hdr|payload]
 *            IPv4 checksum is computed and written. UDP port = L3_FORWARDING_UDP_PORT (9999) both src and dst.
 * @param[in] p_frame         Output buffer (must be >= L3_FORWARDING_MIN_FRAME_SIZE bytes).
 * @param[in] p_dst_mac       Destination MAC (6 bytes, ETH port MAC used as gateway lookup key).
 * @param[in] p_src_mac       Source MAC (6 bytes, simulated sender NIC MAC).
 * @param[in] vid             802.1Q VLAN ID to embed in C-TAG (VID=10 or VID=20).
 * @param[in] p_src_ip        Source IPv4 address (4 bytes).
 * @param[in] p_dst_ip        Destination IPv4 address (4 bytes).
 * @param[in] identification  IPv4 identification field (used to distinguish Frame 0 from Frame 1).
 * @retval    uint16_t  Frame length in bytes (always L3_FORWARDING_MIN_FRAME_SIZE = 64).
 **********************************************************************************************************************/
static uint16_t l3_forwarding_build_ctag_udp_frame(uint8_t       *p_frame,
                                                   const uint8_t *p_dst_mac,
                                                   const uint8_t *p_src_mac,
                                                   uint16_t       vid,
                                                   const uint8_t *p_src_ip,
                                                   const uint8_t *p_dst_ip,
                                                   uint16_t       identification)
{
    const uint16_t ip_off      = 18U;
    const uint16_t udp_off     = ip_off + 20U;
    const uint16_t payload_off = udp_off + 8U;

    memset(p_frame, 0, L3_FORWARDING_TX_BUF_SIZE);

    memcpy(p_frame,     p_dst_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(p_frame + 6, p_src_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);

    /* 802.1Q C-TAG */
    p_frame[12] = 0x81U;
    p_frame[13] = 0x00U;
    p_frame[14] = (uint8_t)((vid >> 8U) & 0x0FU);
    p_frame[15] = (uint8_t)(vid & 0xFFU);

    /* Inner EtherType: IPv4 */
    p_frame[16] = 0x08U;
    p_frame[17] = 0x00U;

    /* IPv4 header: VER=4, IHL=5, Total Length=46, TTL=64, Protocol=UDP=0x11 */
    p_frame[ip_off + 0U] = 0x45U;
    p_frame[ip_off + 1U] = 0x00U;
    p_frame[ip_off + 2U] = 0x00U;
    p_frame[ip_off + 3U] = 0x2EU;
    p_frame[ip_off + 4U] = (uint8_t)((identification >> 8U) & 0xFFU);
    p_frame[ip_off + 5U] = (uint8_t)(identification & 0xFFU);
    p_frame[ip_off + 6U] = 0x00U;
    p_frame[ip_off + 7U] = 0x00U;
    p_frame[ip_off + 8U] = 0x40U; /* TTL=64; ESWM decrements to 63 on L3 forward */
    p_frame[ip_off + 9U] = 0x11U; /* Protocol=UDP */

    memcpy(p_frame + ip_off + 12U, p_src_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);
    memcpy(p_frame + ip_off + 16U, p_dst_ip, L3_FORWARDING_IP_ADDRESS_LENGTH);

    uint16_t csum = l3_forwarding_ipv4_checksum(&p_frame[ip_off], 20U);
    p_frame[ip_off + 10U] = (uint8_t)((csum >> 8U) & 0xFFU);
    p_frame[ip_off + 11U] = (uint8_t)(csum & 0xFFU);

    /* UDP header: src=9999, dst=9999, length=26 */
    p_frame[udp_off + 0U] = 0x27U;
    p_frame[udp_off + 1U] = 0x0FU;
    p_frame[udp_off + 2U] = 0x27U;
    p_frame[udp_off + 3U] = 0x0FU;
    p_frame[udp_off + 4U] = 0x00U;
    p_frame[udp_off + 5U] = 0x1AU;

    for (uint8_t i = 0U; i < 18U; i++)
    {
        p_frame[payload_off + i] = (uint8_t)(i + 1U);
    }
    return L3_FORWARDING_MIN_FRAME_SIZE;
}

/***********************************************************************************************************************
 * @brief     This function injects two MAC mode test frames via CPU_INJECT and prints verify information.
 *            Frame A: dst=NIC2_MAC -> expected egress on ETH1.
 *            Frame B: dst=NIC1_MAC -> expected egress on ETH0.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_inject_mac_frames(void)
{
    uint8_t frame[L3_FORWARDING_MIN_FRAME_SIZE];

    /* Frame A: dst=NIC2_MAC -> CPU_INJECT -> ETH1 */
    memset(frame, 0, sizeof(frame));
    memcpy(frame,     g_l3_forwarding_pc2_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(frame + 6, s_eth0_mac,              L3_FORWARDING_MAC_ADDRESS_LENGTH);
    frame[12] = 0x88U;
    frame[13] = 0xB5U;
    bool consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
    l3_forwarding_report_tx(consumed, "[MAC] Frame A", "forwarded to ETH1");
    APP_PRINT("      Verify on NIC2: eth.type==0x88b5 && eth.dst==%02x:%02x:%02x:%02x:%02x:%02x\r\n\r\n",
              g_l3_forwarding_pc2_mac[0], g_l3_forwarding_pc2_mac[1], g_l3_forwarding_pc2_mac[2],
              g_l3_forwarding_pc2_mac[3], g_l3_forwarding_pc2_mac[4], g_l3_forwarding_pc2_mac[5]);

    R_BSP_SoftwareDelay(L3_FORWARDING_INJECT_GAP_MS, BSP_DELAY_UNITS_MILLISECONDS);

    /* Frame B: dst=NIC1_MAC -> CPU_INJECT -> ETH0 */
    memset(frame, 0, sizeof(frame));
    memcpy(frame,     g_l3_forwarding_pc1_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(frame + 6, s_eth1_mac,              L3_FORWARDING_MAC_ADDRESS_LENGTH);
    frame[12] = 0x88U;
    frame[13] = 0xB5U;
    consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
    l3_forwarding_report_tx(consumed, "[MAC] Frame B", "forwarded to ETH0");
    APP_PRINT("      Verify on NIC1: eth.type==0x88b5 && eth.dst==%02x:%02x:%02x:%02x:%02x:%02x\r\n\r\n",
              g_l3_forwarding_pc1_mac[0], g_l3_forwarding_pc1_mac[1], g_l3_forwarding_pc1_mac[2],
              g_l3_forwarding_pc1_mac[3], g_l3_forwarding_pc1_mac[4], g_l3_forwarding_pc1_mac[5]);
}

/***********************************************************************************************************************
 * @brief     This function injects two VLAN mode C-TAG frames via CPU_INJECT and prints verify information.
 *            Frame A: VID=10 -> expected egress on ETH0 (NIC1).
 *            Frame B: VID=20 -> expected egress on ETH1 (NIC2).
 *            NIC driver strips the 802.1Q tag; verify by unicast dst MAC only.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_inject_vlan_frames(void)
{
    uint8_t frame[L3_FORWARDING_MIN_FRAME_SIZE];

    /* Frame A: C-TAG VID=10 -> CPU_INJECT -> ETH0 (NIC1) */
    memset(frame, 0, sizeof(frame));
    memcpy(frame,     g_l3_forwarding_pc1_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(frame + 6, s_eth0_mac,              L3_FORWARDING_MAC_ADDRESS_LENGTH);
    frame[12] = 0x81U;
    frame[13] = 0x00U;
    frame[14] = (uint8_t)((L3_FORWARDING_VLAN_ID_ETH0 >> 8U) & 0x0FU);
    frame[15] = (uint8_t)(L3_FORWARDING_VLAN_ID_ETH0 & 0xFFU);
    frame[16] = 0x88U;
    frame[17] = 0xB5U;
    bool consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
    l3_forwarding_report_tx(consumed, "[VLAN] Frame A", "forwarded to ETH0");
    APP_PRINT("       Verify on NIC1: eth.type==0x88b5 && eth.dst==%02x:%02x:%02x:%02x:%02x:%02x\r\n\r\n",
              g_l3_forwarding_pc1_mac[0], g_l3_forwarding_pc1_mac[1], g_l3_forwarding_pc1_mac[2],
              g_l3_forwarding_pc1_mac[3], g_l3_forwarding_pc1_mac[4], g_l3_forwarding_pc1_mac[5]);

    R_BSP_SoftwareDelay(L3_FORWARDING_INJECT_GAP_MS, BSP_DELAY_UNITS_MILLISECONDS);

    /* Frame B: C-TAG VID=20 -> CPU_INJECT -> ETH1 (NIC2) */
    memset(frame, 0, sizeof(frame));
    memcpy(frame,     g_l3_forwarding_pc2_mac, L3_FORWARDING_MAC_ADDRESS_LENGTH);
    memcpy(frame + 6, s_eth1_mac,              L3_FORWARDING_MAC_ADDRESS_LENGTH);
    frame[12] = 0x81U;
    frame[13] = 0x00U;
    frame[14] = (uint8_t)((L3_FORWARDING_VLAN_ID_ETH1 >> 8U) & 0x0FU);
    frame[15] = (uint8_t)(L3_FORWARDING_VLAN_ID_ETH1 & 0xFFU);
    frame[16] = 0x88U;
    frame[17] = 0xB5U;
    consumed = l3_forwarding_transmit_one_frame(frame, (uint16_t)sizeof(frame), 0U, 0U);
    l3_forwarding_report_tx(consumed, "[VLAN] Frame B", "forwarded to ETH1");
    APP_PRINT("       Verify on NIC2: eth.type==0x88b5 && eth.dst==%02x:%02x:%02x:%02x:%02x:%02x\r\n\r\n",
              g_l3_forwarding_pc2_mac[0], g_l3_forwarding_pc2_mac[1], g_l3_forwarding_pc2_mac[2],
              g_l3_forwarding_pc2_mac[3], g_l3_forwarding_pc2_mac[4], g_l3_forwarding_pc2_mac[5]);
}

/***********************************************************************************************************************
 * @brief     This function injects two L3 mode C-TAG+IPv4+UDP frames via CPU_INJECT.
 *            Frame A: NIC1->NIC2, VID=10. ESWM rewrites eth.dst=NIC2_MAC, TTL 64->63, egress ETH1.
 *            Frame B: NIC2->NIC1, VID=20. ESWM rewrites eth.dst=NIC1_MAC, TTL 64->63, egress ETH0.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_inject_l3_frames(void)
{
    uint8_t  frame[L3_FORWARDING_TX_BUF_SIZE];
    uint16_t tx_len;

    /* Frame A: NIC1->NIC2. eth.dst=ETH0_MAC (gateway lookup key), VID=10 */
    tx_len = l3_forwarding_build_ctag_udp_frame(frame,
                                                s_eth0_mac, g_l3_forwarding_pc1_mac,
                                                L3_FORWARDING_L3_VID_A,
                                                s_pc1_ip, s_pc2_ip, 0x0001U);
    bool consumed = l3_forwarding_transmit_one_frame(frame, tx_len, 0U, 0U);
    l3_forwarding_report_tx(consumed, "[L3] Frame A (NIC1->NIC2)", "forwarded to ETH1");
    APP_PRINT("     Verify on NIC2: udp.dstport==9999 && ip.dst==192.168.20.100\r\n"
              "                     eth.dst==%02x:%02x:%02x:%02x:%02x:%02x  ip.ttl==63\r\n\r\n",
              g_l3_forwarding_pc2_mac[0], g_l3_forwarding_pc2_mac[1], g_l3_forwarding_pc2_mac[2],
              g_l3_forwarding_pc2_mac[3], g_l3_forwarding_pc2_mac[4], g_l3_forwarding_pc2_mac[5]);

    R_BSP_SoftwareDelay(L3_FORWARDING_INJECT_GAP_MS, BSP_DELAY_UNITS_MILLISECONDS);

    /* Frame B: NIC2->NIC1. eth.dst=ETH1_MAC (gateway lookup key), VID=20 */
    tx_len = l3_forwarding_build_ctag_udp_frame(frame,
                                                s_eth1_mac, g_l3_forwarding_pc2_mac,
                                                L3_FORWARDING_L3_VID_B,
                                                s_pc2_ip, s_pc1_ip, 0x0002U);
    consumed = l3_forwarding_transmit_one_frame(frame, tx_len, 0U, 0U);
    l3_forwarding_report_tx(consumed, "[L3] Frame B (NIC2->NIC1)", "forwarded to ETH0");
    APP_PRINT("     Verify on NIC1: udp.dstport==9999 && ip.dst==192.168.10.100\r\n"
              "                     eth.dst==%02x:%02x:%02x:%02x:%02x:%02x  ip.ttl==63\r\n\r\n",
              g_l3_forwarding_pc1_mac[0], g_l3_forwarding_pc1_mac[1], g_l3_forwarding_pc1_mac[2],
              g_l3_forwarding_pc1_mac[3], g_l3_forwarding_pc1_mac[4], g_l3_forwarding_pc1_mac[5]);
}

/***********************************************************************************************************************
 * @brief     This is L3 mode main run loop. Injects two test frames on entry, then polls RMAC for ARP
 *            requests from ETH0 and ETH1 (CPU handles ARP; ESWM handles IP forwarding in silicon).
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_run_l3(void)
{
    uint32_t iter  = 0U;

    APP_PRINT("[L3]   GW ETH0=192.168.10.1  GW ETH1=192.168.20.1\r\n");
    APP_PRINT("       ARP handled by CPU via RMAC; IP forwarding by ESWM HW.\r\n");
    APP_PRINT("       Press any key to return to menu.\r\n\r\n");
    APP_PRINT("[L3]   Injecting a frame to ETH1 and a frame to ETH0 ...\r\n\r\n");

    l3_forwarding_inject_l3_frames();

    while (true)
    {
        R_RMAC_LinkProcess(g_rmac0.p_ctrl);
        R_RMAC_LinkProcess(g_rmac1.p_ctrl);
        l3_forwarding_check_link(&g_port[0]);
        l3_forwarding_check_link(&g_port[1]);
        l3_forwarding_rx_arp(&g_port[0], g_rx_buf0);
        l3_forwarding_rx_arp(&g_port[1], g_rx_buf1);
        R_BSP_SoftwareDelay(L3_FORWARDING_LINK_POLL_MS, BSP_DELAY_UNITS_MILLISECONDS);

        if ((++iter % L3_FORWARDING_STATS_ITER) == 0U)
        {
            APP_PRINT("[L3]   uptime~%us  ARP ETH0 rx=%u tx=%u | ETH1 rx=%u tx=%u\r\n",
                      (unsigned)((iter * L3_FORWARDING_LINK_POLL_MS) / 1000U),
                      (unsigned)g_port[0].cnt_arp_rx, (unsigned)g_port[0].cnt_arp_tx,
                      (unsigned)g_port[1].cnt_arp_rx, (unsigned)g_port[1].cnt_arp_tx);
        }

        if (APP_CHECK_DATA)
        {
            uint8_t rByte[BUFFER_SIZE_DOWN] = {RESET_VALUE};
            APP_READ(rByte);
            APP_PRINT("[L3]   Exit.\r\n");
            break;
        }
    }
}

/***********************************************************************************************************************
 * @brief     This is MAC and VLAN mode run loop. Injects test frames on entry, then polls link state.
 *            Prints uptime every ~10 seconds. Exits on any keypress.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_run_mac_vlan(void)
{
    uint32_t iter = 0U;
    const char *p_mode_str = (L3_FORWARDING_MODE_MAC == g_forwarding_mode) ? "MAC" : "VLAN";

    if (L3_FORWARDING_MODE_MAC == g_forwarding_mode)
    {
        APP_PRINT("[MAC] Injecting a frame to ETH1 and a frame to ETH0 ...\r\n\r\n");
        l3_forwarding_inject_mac_frames();
        APP_PRINT("[MAC] Done. Press any key to return to menu.\r\n\r\n");
    }
    else
    {
        APP_PRINT("[VLAN] Injecting a frame with VID=%u to ETH0 and a frame with VID=%u to ETH1 ...\r\n\r\n",
                  (unsigned)L3_FORWARDING_VLAN_ID_ETH0, (unsigned)L3_FORWARDING_VLAN_ID_ETH1);
        l3_forwarding_inject_vlan_frames();
        APP_PRINT("[VLAN] Done. Press any key to return to menu.\r\n\r\n");
    }

    while (true)
    {
        R_RMAC_LinkProcess(g_rmac0.p_ctrl);
        R_RMAC_LinkProcess(g_rmac1.p_ctrl);
        l3_forwarding_check_link(&g_port[0]);
        l3_forwarding_check_link(&g_port[1]);
        R_BSP_SoftwareDelay(L3_FORWARDING_LINK_POLL_MS, BSP_DELAY_UNITS_MILLISECONDS);

        if ((++iter % L3_FORWARDING_STATS_ITER) == 0U)
        {
            bool eth0_up = g_port[0].link_up;
            bool eth1_up = g_port[1].link_up;

            APP_PRINT("[%s] uptime~%us  ETH0=%s ETH1=%s\r\n",
                      p_mode_str,
                      (unsigned)((iter * L3_FORWARDING_LINK_POLL_MS) / 1000U),
                      eth0_up ? "UP" : "DOWN",
                      eth1_up ? "UP" : "DOWN");
        }

        if (APP_CHECK_DATA)
        {
            uint8_t rByte[BUFFER_SIZE_DOWN] = {RESET_VALUE};
            APP_READ(rByte);
            APP_PRINT("[%s] Exit.\r\n", p_mode_str);
            break;
        }
    }
}

/***********************************************************************************************************************
 * @brief     This function prints the forwarding mode selection menu to the terminal.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_print_menu(void)
{
    APP_PRINT("\r\n");
    APP_PRINT("Main menu\r\n");
    APP_PRINT("  1. MAC forwarding   - forward frames by destination MAC address\r\n");
    APP_PRINT("  2. VLAN forwarding  - forward frames by 802.1Q VLAN ID (10 -> ETH0, 20 -> ETH1)\r\n");
    APP_PRINT("  3. L3 forwarding    - route IPv4 frames with TTL decrement and MAC_DST rewrite\r\n");
    APP_PRINT("Select option: ");
}

/***********************************************************************************************************************
 * @brief     This function clears the ESWM forwarding table and resets ARP counters between mode switches.
 *            No entries are installed during this clear operation.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
static void l3_forwarding_clear_table(void)
{
    layer3_switch_table_cfg_t clear_cfg = { 0 };
    clear_cfg.p_table                    = NULL;
    clear_cfg.unsecure_entry_maximum_num = L3_FORWARDING_TABLE_ENTRY_MAX;
    clear_cfg.mac_entry_aging_enable     = false;
    clear_cfg.vlan_mode                  = LAYER3_SWITCH_VLAN_MODE_NO_VLAN;

    fsp_err_t err = R_LAYER3_SWITCH_ConfigureTable(g_layer3_switch0.p_ctrl, &clear_cfg);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_LAYER3_SWITCH_ConfigureTable API failed\r\n");
    }
    else
    {
        APP_PRINT("[INIT] Table cleared -- ready for next mode.\r\n");
    }

    g_port[0].cnt_arp_rx = 0U;
    g_port[0].cnt_arp_tx = 0U;
    g_port[1].cnt_arp_rx = 0U;
    g_port[1].cnt_arp_tx = 0U;
}

/***********************************************************************************************************************
 * @brief     This is main entry point for the ESWM L3 forwarding example project.
 *            Opens RMAC drivers, waits for PHY link on ETH0 and ETH1, then loops presenting the mode
 *            selection menu. For each selected mode: configures table, installs entries, runs the
 *            forwarding loop, then clears the table before returning to the menu.
 * @param[in] None.
 * @retval    None.
 **********************************************************************************************************************/
void l3_forwarding_main(void)
{
    uint8_t rByte[BUFFER_SIZE_DOWN] = {RESET_VALUE};
    uint32_t sel;

    l3_forwarding_rmac_init();
    l3_forwarding_wait_links();

    APP_PRINT("========================================\r\n"
              "ONE-TIME NIC SETUP (keep for all modes):\r\n"
              "  NIC1 (ETH0): 192.168.10.100/24  GW=192.168.10.1\r\n"
              "  NIC2 (ETH1): 192.168.20.100/24  GW=192.168.20.1\r\n"
              "========================================\r\n\r\n");

    while (true)
    {
        l3_forwarding_print_menu();

        /*
         * Wait until a valid selection is received.
         * Valid input: a single '1', '2', or '3' followed by nothing, CR or LF.
         * Multi-character input such as "11", "22" or "33" is rejected.
         */
        while (true)
        {
            if (APP_CHECK_DATA)
            {
                /* Clear the input buffer before reading. */
                memset(rByte, NULL_CHAR, sizeof(rByte));
                APP_READ(rByte);

                if ((rByte[0] >= (uint8_t)'1') &&
                    (rByte[0] <= (uint8_t)'3') &&
                    ((rByte[1] == (uint8_t)NULL_CHAR) || (rByte[1] == (uint8_t)'\r') || (rByte[1] == (uint8_t)'\n')))
                {
                    sel = (uint32_t)(rByte[0] - (uint8_t)'0');

#if (1 == BSP_LP64_SUPPORT)
                    /* Re-print the prompt from column 0: with terminal local echo the cursor is already back at
                     * the line start, so printing only the digit would overwrite the first prompt character */
                    APP_PRINT("\rSelect option: %u\r\n\r\n", (unsigned int)sel);
#else
                    APP_PRINT("%u\r\n\r\n", (unsigned int)sel);
#endif
                    break;
                }
                else
                {
                    APP_PRINT("\r\nInvalid input. Please enter 1, 2, or 3: ");
                }
            }
        }

        /* Preserve the existing enum representation */
        g_forwarding_mode = (l3_forwarding_mode_t)((int)'0' + (int)sel);

        l3_forwarding_configure_table();
        l3_forwarding_install_entries();

        if (L3_FORWARDING_MODE_LAYER3 == g_forwarding_mode)
        {
            l3_forwarding_read_table();
        }
        else
        {
            APP_PRINT("[GetTable] skipped for MAC/VLAN; entries verified by SearchTableEntry\r\n");
        }

        switch (g_forwarding_mode)
        {
            case L3_FORWARDING_MODE_LAYER3:
            {
                l3_forwarding_run_l3();
                break;
            }

            case L3_FORWARDING_MODE_MAC:
            case L3_FORWARDING_MODE_VLAN:
            {
                l3_forwarding_run_mac_vlan();
                break;
            }

            default:
            {
                APP_ERR_PRINT("Invalid forwarding mode\r\n");
                break;
            }
        }

        l3_forwarding_clear_table();
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_L3_forwarding_ep)
 **********************************************************************************************************************/
