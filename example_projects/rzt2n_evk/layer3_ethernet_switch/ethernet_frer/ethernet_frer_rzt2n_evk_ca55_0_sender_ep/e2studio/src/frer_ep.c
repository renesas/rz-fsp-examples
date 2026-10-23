/***********************************************************************************************************************
 * File Name    : frer_ep.c
 * Description  : Contains data structures and functions used in frer_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "frer_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_frer_ep
 * @{
 **********************************************************************************************************************/

/* Minimal IPv4/UDP test frame, R-TAG already in place. Field-by-field, not a packed struct, so
 * every byte's meaning is visible at the call site building it. */
#define FRER_HDR_LEN_TO_ETHERTYPE  (20U)   /* Dst(6) + src(6) + R-TAG(6) + EtherType(2) */
#define FRER_HDR_LEN_IP            (20U)
#define FRER_HDR_LEN_UDP           (8U)
#define FRER_HDR_LEN_TOTAL         (FRER_HDR_LEN_TO_ETHERTYPE + FRER_HDR_LEN_IP + FRER_HDR_LEN_UDP)

/* Wire-format field values for the minimal IPv4/UDP header frer_build_frame() writes -- not
 * user-configurable, so these live here rather than in frer_ep.h. */
#define FRER_ETHERTYPE_IPV4_HIGH   (0x08U)
#define FRER_ETHERTYPE_IPV4_LOW    (0x00U)
#define FRER_IP_VERSION_IHL        (0x45U)   /* Version 4, IHL 5 (20-byte header, no options) */
#define FRER_IP_TOS                (0x00U)   /* Default (best-effort) */
#define FRER_IP_TTL                (0xFFU)   /* Maximum -- this frame never leaves a 2-board link */
#define FRER_IP_PROTOCOL_UDP       (17U)     /* IANA protocol number for UDP */

static uint8_t s_src_mac[6]      = FRER_STREAM_SRC_MAC;
static uint8_t s_dst_mac_base[6] = FRER_STREAM_DST_MAC_BASE;
static uint8_t s_dst_mac_port0[6] = FRER_STREAM_DST_MAC_PORT0;
static uint8_t s_dst_mac_port1[6] = FRER_STREAM_DST_MAC_PORT1;
static uint8_t s_src_ip[4]       = FRER_STREAM_SRC_IP;
static uint8_t s_dst_ip[4]       = FRER_STREAM_DST_IP;

static layer3_switch_l3_update_config_t s_update_configs[2];

/* GWCA is a DMA engine and bypasses the CPU cache -- descriptor/frame buffers must be
 * non-cached and 32-byte aligned. FRER_TX_DESC_NUM - 1 usable slots + 1 LEMPTY terminator, the
 * minimum ring shape any CPU-inject TX path on this driver needs. */
BSP_ALIGN_VARIABLE(32) static layer3_switch_descriptor_t s_tx_desc[FRER_TX_DESC_NUM]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
BSP_ALIGN_VARIABLE(32) static uint8_t s_tx_frame_buf[FRER_TX_DESC_NUM - 1U][FRER_FRAME_SIZE]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");

static uint32_t       s_tx_queue_index;
static bool           s_tx_queue_created = false;
static uint8_t        s_tx_slot;
static uint16_t       s_sequence_counter;
static frer_tx_diag_t s_tx_diag;

/* R_LAYER3_SWITCH_CreateDescriptorQueue() retains the ADDRESS of this cfg struct and dereferences
 * it on every later SetDescriptor()/StartDescriptorQueue() call, so it must be persistent, not a
 * stack temporary. */
static layer3_switch_descriptor_queue_cfg_t s_tx_queue_cfg;

static uint16_t frer_build_frame(uint8_t * p_buf);

/***********************************************************************************************************************
 * @brief       This function opens the ESWM/Layer3 Switch driver instance (returns immediately if already open).
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Switch open (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_open(void)
{
    /* Ctrl.open is 0 until R_LAYER3_SWITCH_Open() sets it -- the driver's own "is open"
     * sentinel is private, so check against zero instead. */
    if (0U != g_layer3_switch0_ctrl.open)
    {
        return FSP_SUCCESS;
    }

    /* This Sender never needs the RMAC promiscuous-mode fix that the companion Receiver's own
     * frer_open() applies: that fix only matters for a board classifying frames arriving from an
     * external port by a non-local destination MAC, and this Sender only ever ingresses via the
     * CPU port. */
    return R_LAYER3_SWITCH_Open(&g_layer3_switch0_ctrl, &g_layer3_switch0_cfg);
}

/***********************************************************************************************************************
 * @brief       This function programs the one L3 forwarding entry that replicates the CPU-injected stream: one entry,
 *              matched by IPv4 5-tuple, forwarded out both external ports with hardware sequence generation and an
 *              R-TAG added per member stream.
 * @param[out]  p_info                Filled with the programmed entry count. May be NULL.
 * @retval      FSP_SUCCESS           Table configured and the entry added.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_configure_stream(frer_info_t * p_info)
{
    fsp_err_t                        err;
    layer3_switch_table_cfg_t        table_cfg   = {0};
    layer3_switch_frame_filter_t     target_frame = {.entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_LAYER3};
    layer3_switch_table_entry_cfg_t  entry_cfg   = {0};

    table_cfg.unsecure_entry_maximum_num = FRER_L3_ENTRY_MAX_NUM;
    table_cfg.vlan_mode                  = LAYER3_SWITCH_VLAN_MODE_SC_TAG;

    for (uint8_t port = FRER_PORT0; port <= FRER_CPU_PORT; port++)
    {
        table_cfg.port_cfg_list[port].vlan_ingress_mode         = LAYER3_SWITCH_VLAN_INGRESS_MODE_TAG_BASED;
        table_cfg.port_cfg_list[port].vlan_egress_mode          = LAYER3_SWITCH_VLAN_EGRESS_MODE_SC_TAG;
        table_cfg.port_cfg_list[port].layer3_table_enable       = true;
        table_cfg.port_cfg_list[port].layer3_ipv4_filter_enable = true;
    }

    table_cfg.l3_stream_filter_cfg.filter_field_bitmask = LAYER3_SWITCH_L3_FILTER_BITMASK_MAC_DESTINATION |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_MAC_SOURCE |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_PROTOCOL |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_IP_DESTINATION_ADDRESS |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_IP_SOURCE_ADDRESS |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_IP_SOURCE_PORT |
                                                          LAYER3_SWITCH_L3_FILTER_BITMASK_IP_DESTINATION_PORT;

    err = R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &table_cfg);
    if (FSP_SUCCESS != err) { return err; }

    target_frame.p_destination_mac_address = s_dst_mac_base;
    target_frame.p_source_mac_address      = s_src_mac;
    target_frame.ip_version                = LAYER3_SWITCH_IP_VERSION_IPV4;
    target_frame.protocol                  = LAYER3_SWITCH_IP_PROTOCOL_UDP;
    target_frame.layer4_destination_port   = FRER_STREAM_UDP_DST_PORT;
    target_frame.layer4_source_port        = FRER_STREAM_UDP_SRC_PORT;
    target_frame.p_source_ip_address       = s_src_ip;
    target_frame.p_destination_ip_address  = s_dst_ip;

    /* Replication: one entry, both external ports as destinations. Each port gets its own
     * update config so the two member streams end up on distinct destination MACs. */
    memset(s_update_configs, 0, sizeof(s_update_configs));

    s_update_configs[0].enable_destination_ports          = LAYER3_SWITCH_PORT_BITMASK_PORT0;
    s_update_configs[0].sequence_number_generation_enable = true;
    s_update_configs[0].p_mac_destination_address         = s_dst_mac_port0;
    s_update_configs[0].r_tag_update_mode                 = LAYER3_SWITCH_FORWARDING_R_TAG_ALWAYS_ADD;
    s_update_configs[0].update_field_bitmask              = LAYER3_SWITCH_L3_UPDATE_BITMASK_MAC_DESTINATION;

    s_update_configs[1].enable_destination_ports          = LAYER3_SWITCH_PORT_BITMASK_PORT1;
    s_update_configs[1].sequence_number_generation_enable = true;
    s_update_configs[1].p_mac_destination_address         = s_dst_mac_port1;
    s_update_configs[1].r_tag_update_mode                 = LAYER3_SWITCH_FORWARDING_R_TAG_ALWAYS_ADD;
    s_update_configs[1].update_field_bitmask              = LAYER3_SWITCH_L3_UPDATE_BITMASK_MAC_DESTINATION;

    entry_cfg.entry_enable                    = true;
    entry_cfg.security_enable                 = true;
    entry_cfg.internal_priority_update_enable = false;
    entry_cfg.destination_ports               = LAYER3_SWITCH_PORT_BITMASK_PORT0 | LAYER3_SWITCH_PORT_BITMASK_PORT1;
    entry_cfg.source_ports                    = LAYER3_SWITCH_PORT_BITMASK_PORT2;
    entry_cfg.layer3.p_update_configs         = s_update_configs;
    entry_cfg.layer3.number_of_configs        = 2U;
    entry_cfg.p_frer_entry_cfg                = NULL;    /* Replication + sequence generation only, no recovery here */

    err = R_LAYER3_SWITCH_AddTableEntry(&g_layer3_switch0_ctrl, &target_frame, &entry_cfg);
    if (FSP_SUCCESS != err) { return err; }

    if (NULL != p_info)
    {
        p_info->l3_entry_count = 1U;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function creates the GWCA TX descriptor queue used to inject the frame from FRER_CPU_PORT
 *              (safe to call more than once -- only the first call does anything, later calls just succeed).
 * @param[out]  p_info                Filled with the created queue's index. May be NULL.
 * @retval      FSP_SUCCESS           Descriptor queue created (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_tx_queue_create(frer_info_t * p_info)
{
    fsp_err_t err;

    if (s_tx_queue_created)
    {
        if (NULL != p_info) { p_info->tx_queue_index = s_tx_queue_index; }
        return FSP_SUCCESS;
    }

    s_tx_queue_cfg = (layer3_switch_descriptor_queue_cfg_t)
    {
        .descriptor_format    = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
        .type                 = LAYER3_SWITCH_QUEUE_TYPE_TX,
        .write_back_mode      = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
        .p_descriptor_array   = s_tx_desc,
        .array_length         = FRER_TX_DESC_NUM,
        .rx_timestamp_storage = LAYER3_SWITCH_RX_TIMESTAMP_STORAGE_DISABLE,
        .ports                = LAYER3_SWITCH_PORT_BITMASK_PORT2,
    };

    err = R_LAYER3_SWITCH_CreateDescriptorQueue(&g_layer3_switch0_ctrl, &s_tx_queue_index, &s_tx_queue_cfg);
    if (FSP_SUCCESS != err) { return err; }

    /* Deliberately no priming loop here -- s_tx_desc[] is a static (zero-initialised) array, so
     * every slot already starts as raw zero, including the ring's terminator (same as the
     * Receiver's RX ring -- see frer_arm_rx_descriptors()). Pre-populating every slot up front
     * was tried and found to break the very first real send, since it advances the driver's
     * internal tail index before any real frame exists. The first frer_send_one() call populates
     * slot 0 directly instead, with no priming step needed. */
    s_tx_queue_created = true;
    if (NULL != p_info) { p_info->tx_queue_index = s_tx_queue_index; }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function builds and submits one copy of the frame, relying entirely on the matched L3 forwarding
 *              entry's destination_ports to pick the egress ports and apply each port's R-TAG/sequence-number/
 *              destination-MAC update -- this descriptor itself never touches info1_tx.fmt/dv/ipv, since the matched
 *              table entry's own destination_ports field already determines replication and egress ports for it.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Submitted, or skipped because the queue is still busy with its previous frame.
 * @retval      Any other error code  Propagated from the underlying FSP driver call, or from the physical address
 *                                    conversion (Cortex-A55 only).
 **********************************************************************************************************************/
fsp_err_t frer_send_one(void)
{
    fsp_err_t                  err;
    layer3_switch_descriptor_t desc = {0};
    uint64_t                   frame_pa;
    uint16_t                   len  = frer_build_frame(s_tx_frame_buf[s_tx_slot]);

#if (1 == BSP_LP64_SUPPORT)
    /* CA55 uses the MMU: GWCA reads this frame via DMA and needs the physical address, not the CPU
     * virtual address */
    err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t) s_tx_frame_buf[s_tx_slot], &frame_pa);
    if (FSP_SUCCESS != err) { return err; }
#else
    /* CR52 has no MMU here: the CPU-visible address is what GWCA needs */
    frame_pa = (uint64_t) (uintptr_t) s_tx_frame_buf[s_tx_slot];
#endif

    desc.basic.dt    = LAYER3_SWITCH_DESCRIPTOR_TYPE_FSINGLE;
    desc.basic.ds_l  = (uint8_t) (len & 0xFFU);
    desc.basic.ds_h  = (uint8_t) ((len >> 8U) & 0x0FU);
    desc.basic.die   = 1U;
    desc.basic.ptr_l = (uint32_t) frame_pa;
    desc.basic.ptr_h = (uint8_t) (frame_pa >> 32U);

    /* Try once; skip (don't retry-block) if the queue is still busy with its previous frame.
     * Every skip is counted in s_tx_diag so
     * "Sent N frames" (frer_example.c, counts calls that returned FSP_SUCCESS) can be told apart
     * from "N frames actually handed to hardware" -- both skip branches below also return
     * FSP_SUCCESS, so that counter alone cannot prove anything actually left the wire. */
    err = R_LAYER3_SWITCH_SetDescriptor(&g_layer3_switch0_ctrl, s_tx_queue_index, &desc);
    if (FSP_ERR_OVERFLOW == err)
    {
        (void) R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_tx_queue_index);
        s_tx_diag.skipped_overflow++;
        return FSP_SUCCESS;
    }
    if (FSP_ERR_IN_USE == err)
    {
        s_tx_diag.skipped_busy++;
        return FSP_SUCCESS;
    }
    if (FSP_SUCCESS != err) { return err; }

    err = R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_tx_queue_index);
    if (FSP_ERR_IN_USE == err)
    {
        s_tx_diag.skipped_busy++;
        return FSP_SUCCESS;
    }
    if (FSP_SUCCESS != err) { return err; }

    s_tx_diag.submitted++;

    s_tx_slot++;
    if (s_tx_slot >= (FRER_TX_DESC_NUM - 1U)) { s_tx_slot = 0U; }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function reads back the TX diagnostics accumulated by frer_send_one() -- how many frames were
 *              actually handed to hardware versus silently skipped because the queue was still busy or momentarily
 *              full. Bring-up aid: without this, "Sent N frames" (frer_example.c's own counter) cannot be told apart
 *              from "N calls made, all of them skipped".
 * @param[out]  p_diag                Filled with the current counters. Left untouched if NULL.
 * @retval      None.
 **********************************************************************************************************************/
void frer_get_tx_diag(frer_tx_diag_t * p_diag)
{
    if (NULL != p_diag)
    {
        *p_diag = s_tx_diag;
    }
}

/***********************************************************************************************************************
 * @brief       This function builds one minimal IPv4/UDP frame of the stream, with a placeholder R-TAG already in
 *              place (TPID 0xF1C1, reserved 0x0000, sequence 0x0000) -- the hardware sequence generator overwrites the
 *              sequence field as it replicates; the checksum fields are intentionally left 0, since nothing on this
 *              path (a second RZT2N board's CPU port) validates them.
 * @param[out]  p_buf         Destination buffer, at least FRER_FRAME_SIZE bytes.
 * @retval      len           Frame length in bytes.
 **********************************************************************************************************************/
static uint16_t frer_build_frame(uint8_t * p_buf)
{
    uint16_t udp_len = (uint16_t) (FRER_FRAME_SIZE - FRER_HDR_LEN_TO_ETHERTYPE - FRER_HDR_LEN_IP);
    uint16_t ip_len  = (uint16_t) (FRER_FRAME_SIZE - FRER_HDR_LEN_TO_ETHERTYPE);

    memcpy(&p_buf[0], s_dst_mac_base, 6U);
    memcpy(&p_buf[6], s_src_mac, 6U);
    p_buf[12] = 0xF1U;                                  /* R-TAG TPID high */
    p_buf[13] = 0xC1U;                                  /* R-TAG TPID low */
    p_buf[14] = 0x00U;                                  /* R-TAG reserved high */
    p_buf[15] = 0x00U;                                  /* R-TAG reserved low */
    p_buf[16] = (uint8_t) (s_sequence_counter >> 8U);   /* R-TAG sequence high (placeholder) */
    p_buf[17] = (uint8_t) (s_sequence_counter & 0xFFU); /* R-TAG sequence low  (placeholder) */
    p_buf[18] = FRER_ETHERTYPE_IPV4_HIGH;               /* EtherType high: IPv4 */
    p_buf[19] = FRER_ETHERTYPE_IPV4_LOW;                /* EtherType low */

    p_buf[20] = FRER_IP_VERSION_IHL;
    p_buf[21] = FRER_IP_TOS;
    p_buf[22] = (uint8_t) (ip_len >> 8U);
    p_buf[23] = (uint8_t) (ip_len & 0xFFU);
    p_buf[24] = 0x00U;                                  /* Identification */
    p_buf[25] = 0x00U;
    p_buf[26] = 0x00U;                                  /* Flags + fragment offset */
    p_buf[27] = 0x00U;
    p_buf[28] = FRER_IP_TTL;
    p_buf[29] = FRER_IP_PROTOCOL_UDP;
    p_buf[30] = 0x00U;                                  /* Header checksum (not validated on this path) */
    p_buf[31] = 0x00U;
    memcpy(&p_buf[32], s_src_ip, 4U);
    memcpy(&p_buf[36], s_dst_ip, 4U);

    p_buf[40] = (uint8_t) (FRER_STREAM_UDP_SRC_PORT >> 8U);
    p_buf[41] = (uint8_t) (FRER_STREAM_UDP_SRC_PORT & 0xFFU);
    p_buf[42] = (uint8_t) (FRER_STREAM_UDP_DST_PORT >> 8U);
    p_buf[43] = (uint8_t) (FRER_STREAM_UDP_DST_PORT & 0xFFU);
    p_buf[44] = (uint8_t) (udp_len >> 8U);
    p_buf[45] = (uint8_t) (udp_len & 0xFFU);
    p_buf[46] = 0x00U;                                  /* UDP checksum 0 = "not computed", valid per RFC 768 */
    p_buf[47] = 0x00U;

    /* Payload: running frame counter, low byte first, then a fixed fill pattern */
    p_buf[48] = (uint8_t) (s_sequence_counter & 0xFFU);
    p_buf[49] = (uint8_t) (s_sequence_counter >> 8U);
    memset(&p_buf[50], 0xABU, FRER_FRAME_SIZE - 50U);

    s_sequence_counter++;

    return (uint16_t) FRER_FRAME_SIZE;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_frer_ep)
 **********************************************************************************************************************/
