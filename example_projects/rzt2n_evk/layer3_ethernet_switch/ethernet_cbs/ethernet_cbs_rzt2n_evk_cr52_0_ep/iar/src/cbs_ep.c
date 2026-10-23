/***********************************************************************************************************************
 * File Name    : cbs_ep.c
 * Description  : Contains data structures and functions used in cbs_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <string.h>
#include "cbs_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_cbs_ep
 * @{
 **********************************************************************************************************************/

/* Private functions declarations */
static bool     cbs_mac_is_set(const uint8_t mac[6]);
static uint16_t cbs_build_eth_frame(uint8_t * p_buf, const uint8_t * p_dst_mac, uint8_t queue);

/* Mutable local copy of the switch's extended config, obtained once in cbs_open() from
 * g_layer3_switch0_cfg.p_extend (a void const * -- must be copied into a real, addressable
 * layer3_switch_extended_cfg_t before its p_port_cfg_list[] pointers can be dereferenced).
 * Copying the *outer* struct does not clone what its pointers point to --
 * p_port_cfg_list[CBS_DESTINATION_PORT] still refers to the Configurator-generated
 * layer3_switch_port_cfg_t/layer3_switch_cbs_cfg_t objects, which cbs_configure_bandwidth()
 * mutates directly, in place, every call. */
static layer3_switch_extended_cfg_t s_extended_cfg;

/* GWCA is a DMA engine and bypasses the CPU cache -- descriptor/frame buffers must be
 * non-cached and 32-byte aligned, or GWCA may read stale data and never pick up new frames.
 * One descriptor array and queue is shared by every row, since both queues share
 * CBS_DESTINATION_PORT's single physical link */
BSP_ALIGN_VARIABLE(32) static layer3_switch_descriptor_t s_tx_desc[CBS_TX_DESC_NUM + 1U]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
BSP_ALIGN_VARIABLE(32) static uint8_t s_tx_frame_buf[CBS_TX_DESC_NUM][CBS_FRAME_SIZE_MAX]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
static uint32_t s_tx_queue_index;
static bool     s_tx_queue_created = false;

static layer3_switch_descriptor_queue_cfg_t s_tx_queue_cfg =
{
    .descriptor_format    = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
    .type                 = LAYER3_SWITCH_QUEUE_TYPE_TX,
    .write_back_mode      = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
    .p_descriptor_array   = s_tx_desc,
    .array_length         = CBS_TX_DESC_NUM + 1U,
    .rx_timestamp_storage = LAYER3_SWITCH_RX_TIMESTAMP_STORAGE_DISABLE,
    .ports                = LAYER3_SWITCH_PORT_BITMASK_PORT2,
};

/* Backing descriptor for CPU-generated burst submission (CBS_UC_CPU_GENERATED, console menu
 * [4]/[5]/[6]) -- every CPU-generated mode uses this same batched-descriptor mechanism.
 *
 * Same descriptor/frame content submitted to every ring slot -- destination and payload never
 * change between calls for a given mode, so both are built once by cbs_tx_burst_prepare() and
 * reused as-is; multiple in-flight descriptors safely pointing at the same source buffer is
 * fine for TX (GWCA only ever reads it, never writes it). */
static layer3_switch_descriptor_t s_burst_desc;
static bool                        s_burst_prepared = false;

/* Queues committed as CBS-enabled by the last cbs_configure_bandwidth() call -- tells the next call
 * which queues it must explicitly commit as disabled. */
static uint8_t s_cbs_enabled_mask = 0U;

/***********************************************************************************************************************
 * @brief       This function checks whether a 6-byte MAC address is non-zero.
 * @param[in]   mac           The MAC address to check.
 * @retval      true          At least one byte is non-zero.
 * @retval      false         All bytes are zero (no MAC table entry for this row).
 **********************************************************************************************************************/
static bool cbs_mac_is_set(const uint8_t mac[6])
{
    for (uint8_t b = 0U; b < 6U; b++)
    {
        if (0U != mac[b]) { return true; }
    }
    return false;
}

/***********************************************************************************************************************
 * @brief       This function opens the layer3 switch instance. Returns immediately if it is already open.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Switch open (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t cbs_open(void)
{
    fsp_err_t err;

    if (0U != g_layer3_switch0_ctrl.open)
    {
        return FSP_SUCCESS;
    }

    err = R_LAYER3_SWITCH_Open(&g_layer3_switch0_ctrl, &g_layer3_switch0_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* RMAC's receive filter (register MRAFC) defaults to non-promiscuous and is not touched
     * by R_LAYER3_SWITCH_Open() -- without this, externally-received frames addressed to any
     * MAC other than the port's own never reach MAC-table classification. Value 0x03C703C7 is
     * the promiscuous-mode setting documented in the RZT2N Hardware User's Manual (ESWM
     * chapter). CPU-injected (GWCA) traffic is unaffected, since it bypasses this filter. */
    R_RMAC0->MRAFC = 0x03C703C7UL;
    R_RMAC1->MRAFC = 0x03C703C7UL;

    memcpy(&s_extended_cfg, g_layer3_switch0_cfg.p_extend, sizeof(s_extended_cfg));

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function configures the MAC table and adds one entry per row with a non-zero MAC, steered from
 *              that row's source_port_mask into the queue that row names. Does nothing when CBS_MAC_TABLE_ENABLE == 0.
 * @param[in]   p_rows                One of g_cbs_rows_uc1/uc2 (CBS_ROW_COUNT entries).
 * @param[out]  p_info                Filled in with the programmed MAC-entry count. May be NULL.
 * @retval      FSP_SUCCESS           MAC table configured (or skipped by design).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t cbs_configure_mac_table(const cbs_row_t * p_rows, cbs_info_t * p_info)
{
#if (1 == CBS_MAC_TABLE_ENABLE)
    layer3_switch_table_cfg_t table_cfg = {0};
    fsp_err_t                 err;
    uint8_t                   mac_count = 0U;

    for (uint8_t i = 0U; i < CBS_ROW_COUNT; i++)
    {
        if (cbs_mac_is_set(p_rows[i].mac))
        {
            uint32_t source_ports_mask = p_rows[i].source_port_mask;

            /* Walk every bit set in the mask and enable MAC-table lookup on each of those
             * ingress ports. */
            for (uint8_t sp = 0U; sp <= BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS; sp++)
            {
                if (0U != (source_ports_mask & (1U << sp)))
                {
                    table_cfg.port_cfg_list[sp].mac_table_enable = true;
                }
            }
            table_cfg.port_cfg_list[CBS_DESTINATION_PORT].mac_table_enable = true;
            mac_count++;
        }
    }
    table_cfg.unsecure_entry_maximum_num = mac_count;

    err = R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &table_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    for (uint8_t i = 0U; i < CBS_ROW_COUNT; i++)
    {
        const cbs_row_t * p_row = &p_rows[i];
        if (!cbs_mac_is_set(p_row->mac))
        {
            continue;
        }

        layer3_switch_table_entry_t entry = {.entry_cfg.entry_enable = false};
        entry.target_frame.entry_type                   = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC;
        entry.target_frame.p_destination_mac_address     = (uint8_t *) p_row->mac;
        entry.entry_cfg.entry_enable                     = true;
        /* false, not true -- table_cfg only reserves unsecure_entry_maximum_num space, no
         * secure-entry space field exists at all. */
        entry.entry_cfg.security_enable                  = false;
        entry.entry_cfg.source_ports                     = (uint32_t) p_row->source_port_mask;
        entry.entry_cfg.destination_ports                = (uint32_t) (1U << CBS_DESTINATION_PORT);
        entry.entry_cfg.internal_priority_update_enable  = true;
        entry.entry_cfg.internal_priority_update_value   = p_row->queue;

        err = R_LAYER3_SWITCH_AddTableEntry(&g_layer3_switch0_ctrl,
                                            &entry.target_frame, &entry.entry_cfg);
        if (FSP_SUCCESS != err)
        {
            return err;
        }
    }

    if (NULL != p_info)
    {
        p_info->mac_entry_count = mac_count;
    }
#else
    (void) p_info;
#endif
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function resets the MAC table to empty. Call before cbs_configure_mac_table() whenever
 *              switching to a different use case at runtime. Does nothing when CBS_MAC_TABLE_ENABLE == 0.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Table cleared (or skipped by design).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t cbs_clear_mac_table(void)
{
#if (1 == CBS_MAC_TABLE_ENABLE)
    layer3_switch_table_cfg_t clear_cfg = {0};

    clear_cfg.unsecure_entry_maximum_num = CBS_ROW_COUNT;

    return R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &clear_cfg);
#else
    return FSP_SUCCESS;
#endif
}

/***********************************************************************************************************************
 * @brief       This function builds a layer3_switch_cbs_cfg_t from p_rows (or all-zero when cbs_enabled is false)
 *              and applies it to CBS_DESTINATION_PORT via R_LAYER3_SWITCH_ConfigurePort(). Safe to call again with
 *              a different p_rows/cbs_enabled to switch mode at runtime.
 * @param[in]   p_rows                One of g_cbs_rows_uc1/uc2 (CBS_ROW_COUNT entries).
 * @param[in]   cbs_enabled           true = apply each row's band_width_pct/max_burst_num; false = apply an
 *                                    all-zero CBS config (shaping off).
 * @param[out]  p_info                Filled in with the applied bandwidth per queue. May be NULL.
 * @retval      FSP_SUCCESS           CBS configuration applied.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t cbs_configure_bandwidth(const cbs_row_t * p_rows, bool cbs_enabled, cbs_info_t * p_info)
{
    fsp_err_t                   err;
    layer3_switch_port_cfg_t  * p_port_cfg = s_extended_cfg.p_port_cfg_list[CBS_DESTINATION_PORT];
    layer3_switch_cbs_cfg_t   * p_cbs_cfg  = p_port_cfg->p_cbs_cfg;

    memset(p_cbs_cfg->band_width_list, 0, sizeof(p_cbs_cfg->band_width_list));
    memset(p_cbs_cfg->max_burst_num_list, 0, sizeof(p_cbs_cfg->max_burst_num_list));

    if (cbs_enabled)
    {
        for (uint8_t i = 0U; i < CBS_ROW_COUNT; i++)
        {
            const cbs_row_t * p_row = &p_rows[i];
            if (!cbs_mac_is_set(p_row->mac))
            {
                continue;
            }
            if (p_row->queue >= BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM)
            {
                return FSP_ERR_INVALID_ARGUMENT;
            }
            p_cbs_cfg->band_width_list[p_row->queue]    = p_row->band_width_pct;
            p_cbs_cfg->max_burst_num_list[p_row->queue] = p_row->max_burst_num;
        }
    }

    err = R_LAYER3_SWITCH_ConfigurePort(&g_layer3_switch0_ctrl, CBS_DESTINATION_PORT, p_port_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* The driver commits (EACC) only the queues it just enabled, so a queue switched OFF would keep
     * its old oper shaping. Commit exactly those queues (manual 34.4.2.8; their admin CE is already 0
     * from ConfigurePort()) and no others: committing all eight queues (EACC = 0xFF) makes an enabled
     * queue's shaping erratic. Nothing is written on a fresh boot. */
    R_ETHA0_Type * p_etha    = (0U == CBS_DESTINATION_PORT) ? R_ETHA0 : R_ETHA1;
    uint8_t        now_mask  = 0U;
    for (uint8_t q = 0U; q < BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM; q++)
    {
        if (0U != p_cbs_cfg->band_width_list[q])
        {
            now_mask |= (uint8_t) (1U << q);
        }
    }
    uint8_t stale_mask = (uint8_t) (s_cbs_enabled_mask & (uint8_t) ~now_mask);
    if (0U != stale_mask)
    {
        p_etha->EACC = stale_mask;
    }
    s_cbs_enabled_mask = now_mask;

    if (NULL != p_info)
    {
        p_info->cbs_enabled      = cbs_enabled;
        p_info->oper_enable_mask = (uint8_t) (p_etha->EACOEM & 0xFFU);
        p_info->queue_count = 0U;
        for (uint8_t q = 0U; q < BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM; q++)
        {
            if (0U != p_cbs_cfg->band_width_list[q])
            {
                p_info->queues[p_info->queue_count]         = q;
                p_info->band_width_pct[p_info->queue_count] = p_cbs_cfg->band_width_list[q];
                p_info->queue_count++;
            }
        }
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * CPU-generated frame TX (CBS_UC_CPU_GENERATED only)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief       This function builds a near-MTU Ethernet frame: [dst MAC 6B][src MAC 6B][EtherType 2B][payload ...],
 *              sized so the offered load is high enough to exceed every queue's CBS bandwidth cap on its own.
 * @param[out]  p_buf         Destination buffer for the built frame.
 * @param[in]   p_dst_mac     Destination MAC address (6 bytes).
 * @param[in]   queue         Queue number written as the first payload byte (diagnostic only).
 * @retval      len           Total frame length in bytes.
 **********************************************************************************************************************/
static uint16_t cbs_build_eth_frame(uint8_t * p_buf, const uint8_t * p_dst_mac, uint8_t queue)
{
    static const uint8_t src_mac[6] = CBS_TX_SOURCE_MAC;
    uint16_t             payload_len = (uint16_t) (CBS_FRAME_SIZE_MAX - CBS_TX_HEADER_LEN);

    memcpy(&p_buf[0], p_dst_mac, 6U);
    memcpy(&p_buf[6], src_mac, 6U);
    p_buf[12] = (uint8_t) (CBS_TX_ETHERTYPE >> 8);
    p_buf[13] = (uint8_t) (CBS_TX_ETHERTYPE & 0xFFU);
    p_buf[CBS_TX_HEADER_LEN] = queue;
    memset(&p_buf[CBS_TX_HEADER_LEN + 1U], 0xCDU, (size_t) (payload_len - 1U));

    return (uint16_t) (CBS_TX_HEADER_LEN + payload_len);
}

/***********************************************************************************************************************
 * @brief       This function creates the GWCA TX descriptor queue used to inject frames from CBS_CPU_PORT.
 *              Safe to call more than once -- the actual FSP call only runs the first time, so it can be
 *              called again every time CBS_UC_CPU_GENERATED is (re-)selected at runtime.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Descriptor queue created (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t cbs_tx_queue_create(void)
{
    fsp_err_t err;

    if (s_tx_queue_created)
    {
        return FSP_SUCCESS;
    }

    err = R_LAYER3_SWITCH_CreateDescriptorQueue(&g_layer3_switch0_ctrl,
                                                &s_tx_queue_index, &s_tx_queue_cfg);
    if (FSP_SUCCESS == err)
    {
        s_tx_queue_created = true;
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function builds the fixed frame/descriptor cbs_send_frames_burst() will resubmit every call.
 *              Call once right after cbs_tx_queue_create(), before entering the send loop.
 * @param[in]   dst_mac       Destination MAC to send to (any single queue's MAC from cbs_config.c).
 * @param[in]   queue         IPV queue index that MAC is classified into -- must match that row's queue field.
 *                            Passing the wrong queue silently defeats CBS shaping for the intended queue.
 * @retval      FSP_SUCCESS           Frame and descriptor prepared.
 * @retval      Any other error code  Propagated from the physical address conversion (Cortex-A55 only).
 **********************************************************************************************************************/
fsp_err_t cbs_tx_burst_prepare(const uint8_t dst_mac[6], uint8_t queue)
{
    uint16_t len = cbs_build_eth_frame(s_tx_frame_buf[0], dst_mac, queue);
    uint64_t frame_pa;

#if (1 == BSP_LP64_SUPPORT)
    /* CA55 uses the MMU: GWCA reads this frame via DMA and needs the physical address, not the CPU
     * virtual address */
    fsp_err_t err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t) s_tx_frame_buf[0], &frame_pa);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
#else
    /* CR52 has no MMU here: the CPU-visible address is what GWCA needs */
    frame_pa = (uint64_t) (uintptr_t) s_tx_frame_buf[0];
#endif

    memset(&s_burst_desc, 0, sizeof(s_burst_desc));
    s_burst_desc.basic.dt     = LAYER3_SWITCH_DESCRIPTOR_TYPE_FSINGLE;
    s_burst_desc.basic.ds_l   = (uint8_t) (len & 0xFFU);
    s_burst_desc.basic.ds_h   = (uint8_t) ((len >> 8U) & 0x0FU);
    s_burst_desc.basic.die    = 1U;
    s_burst_desc.basic.ptr_l  = (uint32_t) frame_pa;
    s_burst_desc.basic.ptr_h  = (uint8_t) (frame_pa >> 32U);
    /* dv is a 7-bit field (bits 16..22) / ipv is a 3-bit field (bits 28..30) -- mask
     * explicitly so the narrowing is intentional instead of an implicit -Wconversion. */
    s_burst_desc.info1_tx.fmt = 1U;
    s_burst_desc.info1_tx.dv  = (uint8_t) ((1U << CBS_DESTINATION_PORT) & 0x7FU);
    /* The IPV queue must be the queue of the targeted MAC, otherwise the frames enter another queue
     * that has no bandwidth configured and pass completely unshaped */
    s_burst_desc.info1_tx.ipv = (uint8_t) (queue & 0x07U);

    s_burst_prepared = true;
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function submits the frame cbs_tx_burst_prepare() built, using batched-descriptor
 *              submission: fills the entire usable descriptor ring (CBS_TX_DESC_NUM slots) before requesting a
 *              single StartDescriptorQueue(), instead of one Start call per descriptor. GWCA reads the chained
 *              descriptors autonomously until the ring's terminator slot, which gives every CPU-generated mode
 *              ([4]-[6]) a wider offered-load margin than a naive one-descriptor-per-Start submission would.
 *              Safe to call every cycle in a tight loop with no other pacing.
 * @param[in]   None (uses the descriptor cbs_tx_burst_prepare() built).
 * @retval      FSP_SUCCESS              This cycle's submit/start attempt completed (including doing nothing
 *                                       because a slot or the queue was still busy).
 * @retval      FSP_ERR_NOT_INITIALIZED  cbs_tx_burst_prepare() was never called.
 * @retval      Any other error code     Propagated from the underlying FSP driver call (other than FSP_ERR_IN_USE).
 **********************************************************************************************************************/
fsp_err_t cbs_send_frames_burst(void)
{
    fsp_err_t err;

    if (!s_burst_prepared)
    {
        return FSP_ERR_NOT_INITIALIZED;
    }

    err = R_LAYER3_SWITCH_SetDescriptor(&g_layer3_switch0_ctrl, s_tx_queue_index, &s_burst_desc);
    if (FSP_ERR_OVERFLOW == err)
    {
        /* Ring is full for this round (all CBS_TX_DESC_NUM slots staged) -- kick off
         * transmission of the whole chain in one call. Returns FSP_ERR_IN_USE while the
         * previous chain is still transmitting; that's expected, just retry next call. */
        err = R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_tx_queue_index);
        if ((FSP_SUCCESS != err) && (FSP_ERR_IN_USE != err)) { return err; }
        return FSP_SUCCESS;
    }
    if ((FSP_SUCCESS != err) && (FSP_ERR_IN_USE != err)) { return err; }
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_cbs_ep)
 **********************************************************************************************************************/
