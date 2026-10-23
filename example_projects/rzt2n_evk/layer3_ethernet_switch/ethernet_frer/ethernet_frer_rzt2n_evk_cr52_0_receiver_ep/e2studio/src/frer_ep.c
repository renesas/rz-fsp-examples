/***********************************************************************************************************************
 * File Name    : frer_ep.c
 * Description  : Contains data structures and functions used in frer_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "frer_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_frer_ep
 * @{
 **********************************************************************************************************************/

static uint8_t s_src_mac[6] = FRER_STREAM_SRC_MAC;

/* Destination MAC written onto the frame as it is delivered to the CPU port */
static uint8_t s_dst_mac_update[6] = FRER_STREAM_DST_MAC_BASE;
static uint8_t s_dst_mac_port[FRER_PORT_COUNT][6] = {FRER_STREAM_DST_MAC_PORT0, FRER_STREAM_DST_MAC_PORT1};
static uint8_t s_src_ip[4]         = FRER_STREAM_SRC_IP;
static uint8_t s_dst_ip[4]         = FRER_STREAM_DST_IP;

static layer3_switch_l3_update_config_t s_update_config[FRER_PORT_COUNT];
static layer3_switch_frer_entry_t       s_individual_recovery[FRER_PORT_COUNT];

/* Shared by both ports' FRER entries -- this is what makes the recovery compound (cross-path)
 * instead of two independent per-path ones. See frer_configure_recovery(). */
static layer3_switch_frer_entry_t     s_sequence_recovery;
static layer3_switch_frer_entry_cfg_t s_frer_entry_cfg[FRER_PORT_COUNT];

/* GWCA is a DMA engine and bypasses the CPU cache -- descriptor/frame buffers must be
 * non-cached and 32-byte aligned. One independent RX descriptor queue per ingress port, each
 * FRER_RX_DESC_NUM deep (includes the ring's terminator slot). */
BSP_ALIGN_VARIABLE(32) static layer3_switch_descriptor_t s_rx_desc[FRER_PORT_COUNT][FRER_RX_DESC_NUM]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
BSP_ALIGN_VARIABLE(32) static uint8_t s_rx_frame_buf[FRER_PORT_COUNT][FRER_RX_DESC_NUM - 1U][FRER_FRAME_SIZE]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");

static uint32_t        s_rx_queue_index[FRER_PORT_COUNT];
static uint8_t         s_rx_slot[FRER_PORT_COUNT];
static bool            s_rx_queue_created[FRER_PORT_COUNT];
static frer_rx_diag_t  s_rx_diag[FRER_PORT_COUNT];
static uint32_t        s_l3_class_total[FRER_PORT_COUNT];

/* R_LAYER3_SWITCH_CreateDescriptorQueue() retains the ADDRESS of this cfg struct and dereferences
 * it on every later GetDescriptor()/SetDescriptor()/StartDescriptorQueue() call, so it must be
 * persistent, not a stack temporary. One slot per port. */
static layer3_switch_descriptor_queue_cfg_t s_rx_queue_cfg[FRER_PORT_COUNT];

static fsp_err_t frer_arm_rx_descriptors(uint8_t port);
static uint8_t * frer_rx_frame_address(const layer3_switch_descriptor_t * p_desc);

/***********************************************************************************************************************
 * @brief       This function opens the ESWM/Layer3 Switch driver instance (returns immediately if already open).
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Switch open (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_open(void)
{
    fsp_err_t err;

    /* Ctrl.open is 0 until R_LAYER3_SWITCH_Open() sets it -- the driver's own "is open"
     * sentinel is private, so check against zero instead. */
    if (0U != g_layer3_switch0_ctrl.open)
    {
        return FSP_SUCCESS;
    }

    err = R_LAYER3_SWITCH_Open(&g_layer3_switch0_ctrl, &g_layer3_switch0_cfg);
    if (FSP_SUCCESS != err) { return err; }

    /* R_LAYER3_SWITCH_Open() only sets MPIC, never RMAC's own receive filter (MRAFC) -- without
     * this, frames addressed to the replicated stream's MACs (...0A / ...0B, neither of which
     * is this board's own port MAC) never reach L3-table classification. Any project that
     * classifies externally-received frames by a non-local MAC needs this fix; the companion
     * Sender project does not, since it never receives externally-addressed frames. Value per
     * the RZT2N Hardware User's Manual (ESWM chapter). */
    R_RMAC0->MRAFC = 0x03C703C7UL;
    R_RMAC1->MRAFC = 0x03C703C7UL;

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function creates the RX descriptor queue bound to one ingress port (safe to call more than once
 *              per port -- only the first call for that port does anything).
 * @param[in]   port                  FRER_PORT0 or FRER_PORT1.
 * @param[out]  p_info                Filled with this port's created queue index. May be NULL.
 * @retval      FSP_SUCCESS           Descriptor queue created (or already was).
 * @retval      FSP_ERR_INVALID_ARGUMENT  port is out of range.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_rx_queue_create(uint8_t port, frer_info_t * p_info)
{
    fsp_err_t err;

    if (port >= FRER_PORT_COUNT)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    if (s_rx_queue_created[port])
    {
        if (NULL != p_info) { p_info->rx_queue_index[port] = s_rx_queue_index[port]; }
        return FSP_SUCCESS;
    }

    s_rx_queue_cfg[port] = (layer3_switch_descriptor_queue_cfg_t)
    {
        .descriptor_format    = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
        .type                 = LAYER3_SWITCH_QUEUE_TYPE_RX,
        .write_back_mode      = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
        .p_descriptor_array   = s_rx_desc[port],
        .p_ts_descriptor_array = NULL,
        .array_length         = FRER_RX_DESC_NUM,
        .rx_timestamp_storage = LAYER3_SWITCH_RX_TIMESTAMP_STORAGE_DISABLE,
        .ports                = (uint32_t) (1U << port),
    };

    err = R_LAYER3_SWITCH_CreateDescriptorQueue(&g_layer3_switch0_ctrl, &s_rx_queue_index[port], &s_rx_queue_cfg[port]);
    if (FSP_SUCCESS != err) { return err; }

    /* Arm + start exactly ONE slot here -- see frer_arm_rx_descriptors()'s own doc comment for why
     * priming every usable slot up front (an earlier design) only ever captures one frame total:
     * R_LAYER3_SWITCH_StartDescriptorQueue()'s RX path requires
     * `rx_available` (set true by the immediately-preceding SetDescriptor() call, consumed back to
     * false by Start() itself), so batching all the SetDescriptor() calls before the single Start()
     * call wastes every arm after the first -- only the one Start() call ever succeeds. */
    s_rx_slot[port] = 0U;
    err = frer_arm_rx_descriptors(port);
    if (FSP_SUCCESS != err) { return err; }
    s_rx_slot[port] = 1U;

    s_rx_queue_created[port] = true;

    if (NULL != p_info) { p_info->rx_queue_index[port] = s_rx_queue_index[port]; }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function programs the two L3 forwarding entries (one per ingress port) that recover the
 *              replicated frame: each entry matches one member stream by IPv4 5-tuple + destination MAC, and both
 *              point at one shared sequence-recovery context so a duplicate arriving on either path is eliminated
 *              before the remaining frame is forwarded to the CPU port with its R-TAG removed.
 * @param[out]  p_info                Filled with the programmed entry count. May be NULL.
 * @retval      FSP_SUCCESS           Table configured and both entries added.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t frer_configure_recovery(frer_info_t * p_info)
{
    fsp_err_t                  err;
    layer3_switch_table_cfg_t  table_cfg = {0};

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

    /* Minimal FRER config -- no latent-error/timeout aging (out of scope for this project) */
    table_cfg.frer_cfg.timeout_enable = false;
    table_cfg.frer_cfg.sys_clock      = 0U;
    table_cfg.frer_cfg.check_period   = 0U;

    err = R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &table_cfg);
    if (FSP_SUCCESS != err) { return err; }

    memset(s_individual_recovery, 0, sizeof(s_individual_recovery));
    memset(&s_sequence_recovery, 0, sizeof(s_sequence_recovery));

    for (uint8_t port = 0U; port < FRER_PORT_COUNT; port++)
    {
        layer3_switch_frame_filter_t    target_frame = {.entry_type = LAYER3_SWITCH_TABLE_ENTRY_TYPE_LAYER3};
        layer3_switch_table_entry_cfg_t entry_cfg    = {0};

        target_frame.p_destination_mac_address = s_dst_mac_port[port];
        target_frame.p_source_mac_address      = s_src_mac;
        target_frame.ip_version                = LAYER3_SWITCH_IP_VERSION_IPV4;
        target_frame.protocol                  = LAYER3_SWITCH_IP_PROTOCOL_UDP;
        target_frame.layer4_destination_port   = FRER_STREAM_UDP_DST_PORT;
        target_frame.layer4_source_port        = FRER_STREAM_UDP_SRC_PORT;
        target_frame.p_source_ip_address       = s_src_ip;
        target_frame.p_destination_ip_address  = s_dst_ip;

        s_update_config[port].enable_destination_ports          = LAYER3_SWITCH_PORT_BITMASK_PORT2;
        s_update_config[port].sequence_number_generation_enable = false;
        s_update_config[port].p_mac_destination_address         = s_dst_mac_update;
        s_update_config[port].r_tag_update_mode                 = LAYER3_SWITCH_FORWARDING_R_TAG_UPDATE;
        s_update_config[port].update_field_bitmask              = LAYER3_SWITCH_L3_UPDATE_BITMASK_MAC_DESTINATION;

        s_frer_entry_cfg[port].individual_recovery  = s_individual_recovery[port];
        s_frer_entry_cfg[port].p_sequence_recovery  = &s_sequence_recovery;
        s_frer_entry_cfg[port].sequence_recovery_id = 0U;

        entry_cfg.entry_enable                    = true;
        entry_cfg.security_enable                 = true;
        entry_cfg.internal_priority_update_enable = false;
        entry_cfg.destination_ports               = LAYER3_SWITCH_PORT_BITMASK_PORT2;
        entry_cfg.source_ports                    = (uint32_t) (1U << port);
        entry_cfg.layer3.p_update_configs         = &s_update_config[port];
        entry_cfg.layer3.number_of_configs        = 1U;
        entry_cfg.destination_queue_index         = s_rx_queue_index[port];
        entry_cfg.p_frer_entry_cfg                = &s_frer_entry_cfg[port];

        err = R_LAYER3_SWITCH_AddTableEntry(&g_layer3_switch0_ctrl, &target_frame, &entry_cfg);
        if (FSP_SUCCESS != err) { return err; }
    }

    if (NULL != p_info)
    {
        p_info->l3_entry_count = FRER_PORT_COUNT;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function checks one ingress port's RX queue for a received frame. The hardware runs individual +
 *              sequence recovery before a frame ever reaches this queue, so a duplicate is never seen here -- only
 *              one copy per sequence number arrives, from whichever path delivered it first.
 * @param[in]   port                  FRER_PORT0 or FRER_PORT1.
 * @param[out]  p_sequence            Set to the frame's R-TAG sequence number, if one was received.
 * @param[out]  p_length              Set to the frame's length in bytes, if one was received.
 * @retval      true                  A frame was received; *p_sequence and *p_length are valid.
 * @retval      false                 Nothing pending, an invalid port, or a receive error on this descriptor.
 **********************************************************************************************************************/
bool frer_poll_receive(uint8_t port, uint16_t * p_sequence, uint16_t * p_length)
{
    fsp_err_t                  err;
    layer3_switch_descriptor_t desc = {0};

    if (port >= FRER_PORT_COUNT)
    {
        return false;
    }

    err = R_LAYER3_SWITCH_GetDescriptor(&g_layer3_switch0_ctrl, s_rx_queue_index[port], &desc);
    if ((FSP_ERR_IN_USE == err) || (LAYER3_SWITCH_DESCRIPTOR_TYPE_FEMPTY == desc.basic.dt))
    {
        return false;    /* Nothing received yet */
    }
    if (FSP_SUCCESS != err)
    {
        return false;
    }

    bool received = false;
    if (0U == desc.basic.err)
    {
        uint16_t   len     = (uint16_t) desc.basic.ds_l | (uint16_t) (desc.basic.ds_h << 8U);
        uint8_t  * p_frame = frer_rx_frame_address(&desc);

        if (NULL != p_frame)
        {
            *p_length   = len;
            *p_sequence = (uint16_t) ((p_frame[FRER_RTAG_SEQUENCE_OFFSET] << 8U) |
                                      p_frame[FRER_RTAG_SEQUENCE_OFFSET + 1U]);
            received    = true;
            s_rx_diag[port].frames_ok++;
        }
        else
        {
            s_rx_diag[port].frames_error++;
        }
    }
    else
    {
        s_rx_diag[port].frames_error++;
    }

    /* Re-arm one fresh slot for this reception -- see frer_arm_rx_descriptors() for why it must
     * be one-for-one, not a batch. Stops once this port's whole-lifetime capacity is used up
     * (FRER_RX_DESC_NUM - 1 total); frer_restart() is how to get a fresh one. */
    if (s_rx_slot[port] < (FRER_RX_DESC_NUM - 1U))
    {
        /* A re-arm failure here just means this port stops replenishing -- acceptable for a PoC */
        if (FSP_SUCCESS == frer_arm_rx_descriptors(port))
        {
            s_rx_slot[port]++;
        }
    }

    return received;
}

/***********************************************************************************************************************
 * @brief       This function arms one fresh slot of one port's RX descriptor ring and (re)starts the queue, so the
 *              switch can accept exactly one more frame. Must be called once per reception, never as a batch --
 *              confirmed on hardware that priming every slot up front captures exactly one frame total, while
 *              arming one slot per reception (this function's own approach) captures every frame in turn.
 * @param[in]   port                  FRER_PORT0 or FRER_PORT1.
 * @retval      FSP_SUCCESS           The next slot was armed and the queue (re)started.
 * @retval      Any other error code  Propagated from the underlying FSP driver call, or from the physical address
 *                                    conversion (Cortex-A55 only).
 **********************************************************************************************************************/
static fsp_err_t frer_arm_rx_descriptors(uint8_t port)
{
    fsp_err_t                  err;
    layer3_switch_descriptor_t desc      = {0};
    uint8_t                    buf_index = s_rx_slot[port];
    uint64_t                   frame_pa;

    memset(s_rx_frame_buf[port][buf_index], 0, FRER_FRAME_SIZE);

#if (1 == BSP_LP64_SUPPORT)
    /* CA55 uses the MMU: GWCA writes this frame via DMA and needs the physical address, not the CPU
     * virtual address */
    err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t) s_rx_frame_buf[port][buf_index], &frame_pa);
    if (FSP_SUCCESS != err) { return err; }
#else
    /* CR52 has no MMU here: the CPU-visible address is what GWCA needs */
    frame_pa = (uint64_t) (uintptr_t) s_rx_frame_buf[port][buf_index];
#endif

    desc.basic.dt    = LAYER3_SWITCH_DESCRIPTOR_TYPE_FEMPTY;
    desc.basic.ds_l  = (uint8_t) (FRER_FRAME_SIZE & 0xFFU);
    desc.basic.ds_h  = (uint8_t) ((FRER_FRAME_SIZE >> 8U) & 0x0FU);
    desc.basic.die   = 1U;
    desc.basic.ptr_l = (uint32_t) frame_pa;
    desc.basic.ptr_h = (uint8_t) (frame_pa >> 32U);

    err = R_LAYER3_SWITCH_SetDescriptor(&g_layer3_switch0_ctrl, s_rx_queue_index[port], &desc);
    if (FSP_SUCCESS != err) { return err; }

    return R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_rx_queue_index[port]);
}

/***********************************************************************************************************************
 * @brief       This function returns the address the CPU reads a received frame through. The descriptor holds the
 *              address GWCA wrote the frame to: the same address on CR52, the physical address on CA55, which has to
 *              be converted back to the virtual address of the non-cached buffer.
 * @param[in]   p_desc                The descriptor returned by R_LAYER3_SWITCH_GetDescriptor().
 * @retval      Pointer to the received frame, or NULL if the address cannot be converted (Cortex-A55 only).
 **********************************************************************************************************************/
static uint8_t * frer_rx_frame_address(const layer3_switch_descriptor_t * p_desc)
{
#if (1 == BSP_LP64_SUPPORT)
    uint64_t frame_pa = p_desc->basic.ptr_h;
    uint64_t frame_va;

    frame_pa = (frame_pa << 32U) | p_desc->basic.ptr_l;

    if (FSP_SUCCESS != R_BSP_MmuPatoVa(frame_pa, &frame_va, BSP_MMU_CONVERSION_NON_CACHE))
    {
        return NULL;
    }

    return (uint8_t *) (uintptr_t) frame_va;
#else
    return (uint8_t *) (uintptr_t) p_desc->basic.ptr_l;
#endif
}

/***********************************************************************************************************************
 * @brief       This function reads back one port's RX diagnostics: how many frames this queue delivered with no
 *              hardware error flag, and how many were delivered but flagged as errored.
 * @param[in]   port                  FRER_PORT0 or FRER_PORT1.
 * @param[out]  p_diag                Filled with this port's counters. Left untouched if port is out of range.
 * @retval      None.
 **********************************************************************************************************************/
void frer_get_rx_diag(uint8_t port, frer_rx_diag_t * p_diag)
{
    if ((port < FRER_PORT_COUNT) && (NULL != p_diag))
    {
        *p_diag = s_rx_diag[port];
    }
}

/***********************************************************************************************************************
 * @brief       This function reads the hardware's own FRER Passed/Discarded Packet Counters (FWFRPPCNi/FWFRDPCNi)
 *              for the three FRER table entries this Receiver programs. Not exposed by r_layer3_switch's own API,
 *              so read directly via R_MFWD. Entry allocation order follows the FSP driver's own learning order:
 *              entry 0 is always the shared sequence-recovery context (learned on the first port's
 *              AddTableEntry() call), entry 1 is Port0's individual recovery, entry 2 is Port1's -- holds as long
 *              as frer_configure_recovery() is the only code that ever learns a FRER entry, with Port0 configured
 *              before Port1 (both true here). Reading each register clears it, so the values returned are deltas
 *              since the last call, not running totals.
 * @param[out]  p_counters            Filled with this call's deltas for all three entries.
 * @retval      None.
 **********************************************************************************************************************/
void frer_get_hw_frer_counters(frer_hw_counters_t * p_counters)
{
    p_counters->seqrec_passed             = R_MFWD->FWFRPPCN0;
    p_counters->seqrec_discarded          = R_MFWD->FWFRDPCN0;
    p_counters->port0_individual_passed   = R_MFWD->FWFRPPCN1;
    p_counters->port0_individual_flagged  = R_MFWD->FWFRDPCN1;
    p_counters->port1_individual_passed   = R_MFWD->FWFRPPCN2;
    p_counters->port1_individual_flagged  = R_MFWD->FWFRDPCN2;
}

/***********************************************************************************************************************
 * @brief       This function reads the switch's Layer 3 Forwarded Descriptor Counters (FWLTHFDCN0/1) for Port0 and
 *              Port1 and adds the delta into a running total per port. This counts a frame as soon as it reaches L3
 *              classification, before any FRER filtering runs, so it is ground truth for whether a frame from this
 *              ingress port arrived at all -- independent of whatever individual/sequence recovery then does to it.
 *              Comparing this per-port total against frer_get_rx_diag()'s frames_ok is what tells "eliminated as a
 *              duplicate" apart from "never arrived here at all" (a link/cabling issue, not elimination).
 *              Reading FWLTHFDCN0/1 clears them, so this function must be the only reader and must accumulate -- a
 *              plain print-only version would silently under-report on every call after the first.
 * @param[out]  p_total_port0         Running total for Port0 since the last frer_restart(). May be NULL.
 * @param[out]  p_total_port1         Running total for Port1 since the last frer_restart(). May be NULL.
 * @retval      None.
 **********************************************************************************************************************/
void frer_get_l3_classification(uint32_t * p_total_port0, uint32_t * p_total_port1)
{
    s_l3_class_total[FRER_PORT0] += R_MFWD->FWLTHFDCN0;
    s_l3_class_total[FRER_PORT1] += R_MFWD->FWLTHFDCN1;

    if (NULL != p_total_port0) { *p_total_port0 = s_l3_class_total[FRER_PORT0]; }
    if (NULL != p_total_port1) { *p_total_port1 = s_l3_class_total[FRER_PORT1]; }
}

/***********************************************************************************************************************
 * @brief       This function fully restarts the FRER receive pipeline: closes and reopens the switch, then redoes
 *              the entire RX queue + recovery-entry setup from scratch. This is the only way to give an RX queue a
 *              fresh tail=0 once it has used up its whole-lifetime reception capacity -- R_LAYER3_SWITCH_Close() is
 *              the only operation that resets it, and there is no partial "free up N slots" API.
 *              Close()/Open()/ConfigureTable() do not reset the driver's own record of which FRER entry each
 *              sequence-recovery context was learned into, so this function clears it by hand -- left stale, the
 *              next frer_configure_recovery() call would skip re-writing the shared context even though the
 *              hardware FRER table was just wiped.
 * @param[out]  p_info                Filled with the freshly re-programmed entry count and queue indices. May be NULL.
 * @retval      FSP_SUCCESS           Pipeline fully restarted and ready to receive again.
 * @retval      Any other error code  Propagated from the underlying FSP driver call; the pipeline is left closed.
 **********************************************************************************************************************/
fsp_err_t frer_restart(frer_info_t * p_info)
{
    fsp_err_t err;

    err = R_LAYER3_SWITCH_Close(&g_layer3_switch0_ctrl);
    if (FSP_SUCCESS != err) { return err; }

    memset(g_layer3_switch0_ctrl.frer_sequence_recovery_status, 0,
           sizeof(g_layer3_switch0_ctrl.frer_sequence_recovery_status));

    memset(s_rx_queue_created, 0, sizeof(s_rx_queue_created));
    memset(s_rx_diag, 0, sizeof(s_rx_diag));
    memset(s_l3_class_total, 0, sizeof(s_l3_class_total));

    err = frer_open();
    if (FSP_SUCCESS != err) { return err; }

    err = frer_rx_queue_create(FRER_PORT0, p_info);
    if (FSP_SUCCESS != err) { return err; }

    err = frer_rx_queue_create(FRER_PORT1, p_info);
    if (FSP_SUCCESS != err) { return err; }

    return frer_configure_recovery(p_info);
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_frer_ep)
 **********************************************************************************************************************/
