/***********************************************************************************************************************
 * File Name    : tas_ep.c
 * Description  : Contains data structures and functions used in tas_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "tas_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_tas_ep
 * @{
 **********************************************************************************************************************/

#if (TAS_MAX_ENTRIES_PER_GATE < ((2U * TAS_MAX_WINDOWS_PER_ROW) + 1U))
 #error "TAS_MAX_ENTRIES_PER_GATE too small for TAS_MAX_WINDOWS_PER_ROW"
#endif

/* Private functions declarations */
static bool      tas_mac_is_set(const uint8_t mac[6]);
static uint8_t   tas_build_gate(const tas_row_t * p_row, layer3_switch_tas_entry_t * p_dst,
                                layer3_switch_tas_gate_cfg_t * p_gate);
static fsp_err_t tas_resolve_cycle_start(uint64_t * p_start_ns);
static uint16_t  tas_build_eth_frame(uint8_t * p_buf, const uint8_t * p_dst_mac, uint8_t ipv);

static layer3_switch_tas_entry_t s_entries[BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS]
                                           [BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM]
                                           [TAS_MAX_ENTRIES_PER_GATE];
static layer3_switch_tas_cfg_t s_tas_cfg[BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS];
static bool s_port_used[BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS];

static const layer3_switch_tas_gate_cfg_t s_gate_closed =
{
    .initial_gate_state = LAYER3_SWITCH_TAS_GATE_STATE_CLOSE,
    .tas_entry_num      = 1U,
    .p_tas_entry_list   = NULL,     /* Filled in per-port at use (needs a stable 1-entry array) */
};
static const layer3_switch_tas_entry_t s_entry_closed = {LAYER3_SWITCH_TAS_GATE_STATE_CLOSE, 0U};

BSP_ALIGN_VARIABLE(32) static layer3_switch_descriptor_t s_tx_desc[TAS_ROW_COUNT][TAS_TX_PER_QUEUE_DESC_NUM]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
BSP_ALIGN_VARIABLE(32) static uint8_t s_tx_frame_buf[TAS_ROW_COUNT][TAS_FRAME_SIZE_MAX]
    BSP_PLACE_IN_SECTION(".noncache_buffer.eth");
static uint32_t s_tx_queue_index[TAS_ROW_COUNT];
static bool     s_tx_queue_created[TAS_ROW_COUNT];

/* R_LAYER3_SWITCH_CreateDescriptorQueue() retains the ADDRESS of this cfg struct and dereferences
 * it on every later SetDescriptor()/StartDescriptorQueue() call, so it must be persistent, not a
 * stack temporary. One slot per queue, mirroring s_tx_desc/s_tx_frame_buf. */
static layer3_switch_descriptor_queue_cfg_t s_tx_queue_cfg[TAS_ROW_COUNT];

/***********************************************************************************************************************
 * @brief       This function checks whether a 6-byte MAC address is non-zero.
 * @param[in]   mac           The MAC address to check.
 * @retval      true          At least one byte is non-zero.
 * @retval      false         All bytes are zero.
 **********************************************************************************************************************/
static bool tas_mac_is_set(const uint8_t mac[6])
{
    for (uint8_t b = 0U; b < 6U; b++)
    {
        if (0U != mac[b]) { return true; }
    }
    return false;
}

/***********************************************************************************************************************
 * @brief       This function translates one schedule-table row into a gate configuration plus its backing entry list.
 * @param[in]   p_row         Schedule row to translate.
 * @param[out]  p_dst         Backing storage for the generated entry list.
 * @param[out]  p_gate        Gate config to fill in.
 * @retval      n             Number of entries written (always >= 1).
 * @retval      0             Invalid input (bad window count/ordering, or a window overruns the cycle).
 **********************************************************************************************************************/
static uint8_t tas_build_gate(const tas_row_t * p_row, layer3_switch_tas_entry_t * p_dst,
                              layer3_switch_tas_gate_cfg_t * p_gate)
{
    uint8_t  n        = 0U;
    uint32_t prev_end = 0U;

    if (p_row->window_count > TAS_MAX_WINDOWS_PER_ROW)
    {
        return 0U;
    }

    if (0U == p_row->window_count)
    {
        p_gate->initial_gate_state = LAYER3_SWITCH_TAS_GATE_STATE_CLOSE;
        p_dst[n++] = s_entry_closed;
    }
    else
    {
        p_gate->initial_gate_state = (0U == p_row->windows[0].offset_ns) ?
            LAYER3_SWITCH_TAS_GATE_STATE_OPEN : LAYER3_SWITCH_TAS_GATE_STATE_CLOSE;

        for (uint8_t w = 0U; w < p_row->window_count; w++)
        {
            uint32_t offset_ns   = p_row->windows[w].offset_ns;
            uint32_t duration_ns = p_row->windows[w].duration_ns;

            /* Windows must be ascending, non-overlapping, and within the cycle */
            if ((offset_ns < prev_end) || ((offset_ns + duration_ns) > TAS_CYCLE_TIME_NS))
            {
                return 0U;
            }

            if (offset_ns > prev_end)
            {
                p_dst[n++] = (layer3_switch_tas_entry_t)
                    {LAYER3_SWITCH_TAS_GATE_STATE_CLOSE, offset_ns - prev_end};
            }
            p_dst[n++] = (layer3_switch_tas_entry_t)
                {LAYER3_SWITCH_TAS_GATE_STATE_OPEN, duration_ns};
            prev_end = offset_ns + duration_ns;
        }
        p_dst[n++] = s_entry_closed;    /* Terminator: close for the cycle remainder */
    }

    p_gate->tas_entry_num    = n;
    p_gate->p_tas_entry_list = p_dst;
    return n;
}

/***********************************************************************************************************************
 * @brief       This function opens the layer3 switch instance. No-op if it is already open.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Switch open (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_open(void)
{
    fsp_err_t err;

    /* Ctrl.open is 0 until R_LAYER3_SWITCH_Open() sets it -- the driver's own "is open"
     * sentinel is private, so check against zero instead. */
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

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function configures and starts the gPTP timer TAS_GPTP_TIMER points to.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Timer configured and started (or start skipped by design).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_timer_start(void)
{
#if (1 == TAS_START_TIMER)
    gptp_timer_cfg_t cfg = {.clock_period = TAS_GPTP_CLOCK_PERIOD};
    fsp_err_t         err;

    err = R_GPTP_TimerCfg(&g_gptp0_ctrl, TAS_GPTP_TIMER, &cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    return R_GPTP_Start(&g_gptp0_ctrl, TAS_GPTP_TIMER);
#else
    return FSP_SUCCESS;
#endif
}

/***********************************************************************************************************************
 * @brief       This function resolves the absolute cycle start time per TAS_CYCLE_START_MODE.
 * @param[out]  p_start_ns              Resolved cycle start time, in nanoseconds.
 * @retval      FSP_SUCCESS             Cycle start time resolved.
 * @retval      FSP_ERR_NOT_INITIALIZED (DYNAMIC mode only) timer reading is below the valid threshold, meaning
 *                                      the clock source is not running yet.
 * @retval      Any other error code    Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
static fsp_err_t tas_resolve_cycle_start(uint64_t * p_start_ns)
{
#if (TAS_CYCLE_START_MODE_FIXED == TAS_CYCLE_START_MODE)
    *p_start_ns = TAS_CYCLE_START_FIXED_NS;
    return FSP_SUCCESS;
#else
    gptp_timer_value_t now = {0};
    fsp_err_t           err;
    uint64_t            now_ns;
    uint64_t            target_ns;

    err = R_GPTP_TimerValueGet(&g_gptp0_ctrl, TAS_GPTP_TIMER, &now);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    now_ns = ((uint64_t) now.time_sec_lower * 1000000000ULL) + (uint64_t) now.time_nsec;
    if (now_ns < TAS_CYCLE_START_MIN_VALID_NS)
    {
        /* Timer not running yet -- caller likely called before the clock source
         * (gPTP sync / local timer start) was actually up. */
        return FSP_ERR_NOT_INITIALIZED;
    }

    target_ns   = now_ns + TAS_CYCLE_START_MARGIN_NS;
    *p_start_ns = ((target_ns + TAS_CYCLE_TIME_NS - 1ULL) / TAS_CYCLE_TIME_NS)
                  * (uint64_t) TAS_CYCLE_TIME_NS;
    return FSP_SUCCESS;
#endif
}

/***********************************************************************************************************************
 * @brief       This function translates p_rows into TAS gate configuration and programs it on every port the table
 *              references.
 * @param[in]   p_rows                One of g_tas_rows_uc1/uc2/uc3 (TAS_ROW_COUNT entries).
 * @param[out]  p_info                Filled in with the programmed cycle/port/gate info. May be NULL.
 * @retval      FSP_SUCCESS           Schedule programmed on every referenced port.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_configure_schedule(const tas_row_t * p_rows, tas_info_t * p_info)
{
    uint64_t  start_ns;
    fsp_err_t err;

    err = tas_resolve_cycle_start(&start_ns);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    memset(s_tas_cfg, 0, sizeof(s_tas_cfg));
    memset(s_port_used, 0, sizeof(s_port_used));
    if (NULL != p_info)
    {
        memset(p_info, 0, sizeof(*p_info));
    }

    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        const tas_row_t * p_row = &p_rows[i];

        if ((p_row->port >= BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS) ||
            (p_row->queue >= BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM))
        {
            return FSP_ERR_INVALID_ARGUMENT;
        }

        layer3_switch_tas_gate_cfg_t gate = {.tas_entry_num = 0U};
        uint8_t n = tas_build_gate(p_row, s_entries[p_row->port][p_row->queue], &gate);
        if (0U == n)
        {
            return FSP_ERR_INVALID_ARGUMENT;
        }

        s_tas_cfg[p_row->port].gate_cfg_list[p_row->queue] = gate;
        s_port_used[p_row->port] = true;
    }

    for (uint8_t port = 0U; port < BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS; port++)
    {
        if (!s_port_used[port])
        {
            continue;
        }

        /* Any queue not covered by a row stays permanently closed */
        uint32_t total_entries = 0U;
        for (uint8_t q = 0U; q < BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM; q++)
        {
            if (NULL == s_tas_cfg[port].gate_cfg_list[q].p_tas_entry_list)
            {
                s_entries[port][q][0] = s_entry_closed;
                s_tas_cfg[port].gate_cfg_list[q] = s_gate_closed;
                s_tas_cfg[port].gate_cfg_list[q].p_tas_entry_list = s_entries[port][q];
            }
            total_entries += s_tas_cfg[port].gate_cfg_list[q].tas_entry_num;
        }
        if (total_entries > TAS_MAX_TOTAL_ENTRIES_PER_PORT)
        {
            return FSP_ERR_OUT_OF_MEMORY;
        }

        s_tas_cfg[port].gptp_timer_number     = TAS_GPTP_TIMER;
        s_tas_cfg[port].cycle_time            = TAS_CYCLE_TIME_NS;
        s_tas_cfg[port].cycle_time_start_high = (uint32_t) (start_ns >> 32);
        s_tas_cfg[port].cycle_time_start_low  = (uint32_t) (start_ns & 0xFFFFFFFFULL);

        fsp_err_t err2 = R_LAYER3_SWITCH_ConfigureTAS(&g_layer3_switch0_ctrl, port, &s_tas_cfg[port]);
        if (FSP_SUCCESS != err2)
        {
            return err2;
        }

        if (NULL != p_info)
        {
            p_info->ports[p_info->port_count++] = port;
            for (uint8_t q = 0U; q < BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM; q++)
            {
                p_info->entries_per_gate[port][q] = s_tas_cfg[port].gate_cfg_list[q].tas_entry_num;
            }
        }
    }

    if (NULL != p_info)
    {
        p_info->cycle_start_ns = start_ns;
        p_info->cycle_time_ns  = TAS_CYCLE_TIME_NS;
        p_info->timer_number   = TAS_GPTP_TIMER;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function configures the MAC table and adds one entry per schedule row with a non-zero MAC, steered
 *              from that row's source_port_mask to its destination port/queue.
 * @param[in]   p_rows                One of g_tas_rows_uc1/uc2/uc3 (TAS_ROW_COUNT entries).
 * @param[out]  p_info                Filled in with the programmed MAC-entry count. May be NULL.
 * @retval      FSP_SUCCESS           MAC table configured (or skipped by design).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_configure_mac_table(const tas_row_t * p_rows, tas_info_t * p_info)
{
#if (1 == TAS_MAC_TABLE_ENABLE)
    layer3_switch_table_cfg_t table_cfg = {0};
    fsp_err_t                 err;
    uint8_t                   mac_count = 0U;

    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        if (tas_mac_is_set(p_rows[i].mac))
        {
            uint8_t  port             = p_rows[i].port;
            uint32_t source_ports_mask = p_rows[i].source_port_mask;
            if (port >= BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS)
            {
                return FSP_ERR_INVALID_ARGUMENT;
            }
            /* Walk every bit set in the mask (may be more than one -- e.g. UC3 combines
             * TAS_SOURCE_PORT and TAS_CPU_PORT) and enable MAC-table lookup on each of
             * those ingress ports. */
            for (uint8_t sp = 0U; sp <= BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS; sp++)
            {
                if (0U != (source_ports_mask & (1U << sp)))
                {
                    table_cfg.port_cfg_list[sp].mac_table_enable = true;
                }
            }
            table_cfg.port_cfg_list[port].mac_table_enable = true;
            mac_count++;
        }
    }
    table_cfg.unsecure_entry_maximum_num = mac_count;

    err = R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &table_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        const tas_row_t * p_row = &p_rows[i];
        if (!tas_mac_is_set(p_row->mac))
        {
            continue;
        }

        layer3_switch_table_entry_t entry = {.entry_cfg.entry_enable = false};
        entry.target_frame.entry_type                 = LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC;
        entry.target_frame.p_destination_mac_address   = (uint8_t *) p_row->mac;
        entry.entry_cfg.entry_enable                   = true;
        entry.entry_cfg.security_enable                = false;
        entry.entry_cfg.source_ports                   = (uint32_t) p_row->source_port_mask;
        entry.entry_cfg.destination_ports               = (uint32_t) (1U << p_row->port);
        entry.entry_cfg.internal_priority_update_enable = true;
        entry.entry_cfg.internal_priority_update_value  = p_row->queue;

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
 * @brief       This function resets the MAC table to empty. Call before tas_configure_mac_table() whenever
 *              switching to a different use case at runtime.
 * @param[in]   None.
 * @retval      FSP_SUCCESS           Table cleared (or skipped by design).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_clear_mac_table(void)
{
#if (1 == TAS_MAC_TABLE_ENABLE)
    layer3_switch_table_cfg_t clear_cfg = {0};

    clear_cfg.unsecure_entry_maximum_num = TAS_ROW_COUNT;

    return R_LAYER3_SWITCH_ConfigureTable(&g_layer3_switch0_ctrl, &clear_cfg);
#else
    return FSP_SUCCESS;
#endif
}

/***********************************************************************************************************************
 * @brief       This function enables TAS on every port configured by tas_configure_schedule().
 * @param[in]   None.
 * @retval      FSP_SUCCESS           TAS enabled on every configured port.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_enable(void)
{
    fsp_err_t err;

    for (uint8_t port = 0U; port < BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS; port++)
    {
        if (!s_port_used[port])
        {
            continue;
        }
        err = R_LAYER3_SWITCH_EnableTAS(&g_layer3_switch0_ctrl, port);
        if (FSP_SUCCESS != err)
        {
            return err;
        }
    }
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function is a convenience wrapper: configure_schedule -> configure_mac_table -> enable, in order,
 *              stopping at the first failure. Does not call tas_open()/tas_timer_start() (run those once, outside
 *              the per-use-case flow) or tas_clear_mac_table() (call it first yourself if switching use case).
 * @param[in]   p_rows                One of g_tas_rows_uc1/uc2/uc3 (TAS_ROW_COUNT entries).
 * @param[out]  p_info                Passed through to tas_configure_schedule()/tas_configure_mac_table().
 * @retval      FSP_SUCCESS           Every step succeeded.
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_init(const tas_row_t * p_rows, tas_info_t * p_info)
{
    fsp_err_t err;

    err = tas_configure_schedule(p_rows, p_info);
    if (FSP_SUCCESS != err) { return err; }

    err = tas_configure_mac_table(p_rows, p_info);
    if (FSP_SUCCESS != err) { return err; }

    return tas_enable();
}

/***********************************************************************************************************************
 * CPU-generated frame TX (used by TAS_UC_CPU_GENERATED and TAS_UC_COMBINED)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief       This function builds a minimal Ethernet frame.
 * @param[out]  p_buf         Destination buffer for the built frame.
 * @param[in]   p_dst_mac     Destination MAC address (6 bytes).
 * @param[in]   ipv           IPV/queue value written as the first payload byte.
 * @retval      len           Total frame length in bytes.
 **********************************************************************************************************************/
static uint16_t tas_build_eth_frame(uint8_t * p_buf, const uint8_t * p_dst_mac, uint8_t ipv)
{
    static const uint8_t src_mac[6] = TAS_TX_SOURCE_MAC;

    memcpy(&p_buf[0], p_dst_mac, 6U);
    memcpy(&p_buf[6], src_mac, 6U);
    p_buf[12] = (uint8_t) (TAS_TX_ETHERTYPE >> 8);
    p_buf[13] = (uint8_t) (TAS_TX_ETHERTYPE & 0xFFU);
    p_buf[TAS_TX_HEADER_LEN] = ipv;
    memset(&p_buf[TAS_TX_HEADER_LEN + 1U], 0xABU, 32U);

    return (uint16_t) (TAS_TX_HEADER_LEN + 1U + 32U);
}

/***********************************************************************************************************************
 * @brief       This function creates one GWCA TX descriptor queue per row that has a non-zero mac and TAS_CPU_PORT
 *              set in source_port_mask. Idempotent per queue -- safe to call again on a use-case switch.
 * @param[in]   p_rows                One of g_tas_rows_uc2/uc3 (TAS_ROW_COUNT entries).
 * @retval      FSP_SUCCESS           Every matching row's queue created (or already was).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
fsp_err_t tas_tx_queue_create(const tas_row_t * p_rows)
{
    fsp_err_t err;

    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        const tas_row_t * p_row = &p_rows[i];
        if ((0U == (p_row->source_port_mask & (1U << TAS_CPU_PORT))) || !tas_mac_is_set(p_row->mac))
        {
            continue;
        }

        if (s_tx_queue_created[p_row->queue])
        {
            continue;
        }

        s_tx_queue_cfg[p_row->queue] = (layer3_switch_descriptor_queue_cfg_t)
        {
            .descriptor_format    = LAYER3_SWITCH_DISCRIPTOR_FORMTAT_EXTENDED,
            .type                 = LAYER3_SWITCH_QUEUE_TYPE_TX,
            .write_back_mode      = LAYER3_SWITCH_WRITE_BACK_MODE_FULL,
            .p_descriptor_array   = s_tx_desc[p_row->queue],
            .array_length         = TAS_TX_PER_QUEUE_DESC_NUM,
            .rx_timestamp_storage = LAYER3_SWITCH_RX_TIMESTAMP_STORAGE_DISABLE,
            .ports                = LAYER3_SWITCH_PORT_BITMASK_PORT2,
        };

        err = R_LAYER3_SWITCH_CreateDescriptorQueue(&g_layer3_switch0_ctrl,
                                                    &s_tx_queue_index[p_row->queue],
                                                    &s_tx_queue_cfg[p_row->queue]);
        if (FSP_SUCCESS != err)
        {
            return err;
        }
        s_tx_queue_created[p_row->queue] = true;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function builds and submits one row's Ethernet frame on that row's own descriptor queue.
 *              Shared by tas_send_frames() and tas_send_frames_staggered().
 * @param[in]   p_row         The row to submit.
 * @retval      FSP_SUCCESS           Submitted, or skipped because the row's queue is busy against its own
 *                                    still-pending previous frame (FSP_ERR_IN_USE / FSP_ERR_OVERFLOW).
 * @retval      Any other error code  Propagated from the underlying FSP driver call.
 **********************************************************************************************************************/
static fsp_err_t tas_send_one_row(const tas_row_t * p_row)
{
    fsp_err_t                  err;
    layer3_switch_descriptor_t desc = {0};
    uint64_t                   frame_pa;

    uint16_t len = tas_build_eth_frame(s_tx_frame_buf[p_row->queue], p_row->mac, p_row->queue);

#if (1 == BSP_LP64_SUPPORT)
    /* CA55 uses the MMU: GWCA reads this frame via DMA and needs the physical address, not the
     * CPU virtual address -- R_LAYER3_SWITCH_SetDescriptor() copies the descriptor in verbatim
     * (a plain memcpy() in the driver source), with no translation of its own. */
    err = R_BSP_MmuVatoPa((uint64_t) (uintptr_t) s_tx_frame_buf[p_row->queue], &frame_pa);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
#else
    /* CR52 has no MMU here: the CPU-visible address is what GWCA needs. */
    frame_pa = (uint64_t) (uintptr_t) s_tx_frame_buf[p_row->queue];
#endif

    memset(&desc, 0, sizeof(desc));
    desc.basic.dt     = LAYER3_SWITCH_DESCRIPTOR_TYPE_FSINGLE;
    desc.basic.ds_l   = (uint8_t) (len & 0xFFU);
    desc.basic.ds_h   = (uint8_t) ((len >> 8U) & 0x0FU);
    desc.basic.die    = 1U;
    desc.basic.ptr_l  = (uint32_t) frame_pa;
    desc.basic.ptr_h  = (uint8_t) (frame_pa >> 32U);
    desc.info1_tx.fmt = 1U;
    desc.info1_tx.dv  = (uint8_t) ((1U << p_row->port) & 0x7FU);   /* Destination vector (7 bits) */
    desc.info1_tx.ipv = (uint8_t) (p_row->queue & 0x07U);          /* Internal priority value (3 bits) */

    /* Try once; skip (don't retry-block) if this row's queue is still busy with its previous
     * frame. Matches the CBS EP's `cbs_send_frames()` skip-if-busy pattern. */
    err = R_LAYER3_SWITCH_SetDescriptor(&g_layer3_switch0_ctrl, s_tx_queue_index[p_row->queue], &desc);
    if (FSP_ERR_OVERFLOW == err)
    {
        /* The descriptor tail wasn't reset because a prior StartDescriptorQueue() returned
         * FSP_ERR_IN_USE (gate not yet open). Retry the start so the queue recovers once the
         * previous frame has actually gone out. */
        (void) R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_tx_queue_index[p_row->queue]);
        return FSP_SUCCESS;
    }
    if (FSP_ERR_IN_USE == err)
    {
        return FSP_SUCCESS;
    }
    if (FSP_SUCCESS != err) { return err; }

    err = R_LAYER3_SWITCH_StartDescriptorQueue(&g_layer3_switch0_ctrl, s_tx_queue_index[p_row->queue]);
    if ((FSP_SUCCESS != err) && (FSP_ERR_IN_USE != err)) { return err; }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function submits one Ethernet frame per matching schedule row (non-zero mac + TAS_CPU_PORT in
 *              source_port_mask), all back-to-back with no pacing. TAS's gate times the egress; see
 *              tas_send_frames_staggered() for a variant that also spaces out the submissions.
 * @param[in]   p_rows                One of g_tas_rows_uc2/uc3 (TAS_ROW_COUNT entries).
 * @retval      FSP_SUCCESS           Every matching row was either submitted or skipped as busy.
 * @retval      Any other error code  Propagated from the underlying FSP driver call (see tas_send_one_row()).
 **********************************************************************************************************************/
fsp_err_t tas_send_frames(const tas_row_t * p_rows)
{
    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        const tas_row_t * p_row = &p_rows[i];
        if ((0U == (p_row->source_port_mask & (1U << TAS_CPU_PORT))) || !tas_mac_is_set(p_row->mac))
        {
            continue;
        }

        fsp_err_t err = tas_send_one_row(p_row);
        if (FSP_SUCCESS != err) { return err; }
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function is the same as tas_send_frames(), except each row's submission is delayed until that
 *              row's own scheduled window offset (windows[0].offset_ns), so a frame is handed to GWCA close to
 *              when its gate opens rather than up to a full cycle early. Delays are subtractions of static row-table
 *              offsets, never a live timer read. TAS's gate still decides exact egress; this only limits how long
 *              a frame sits queued.
 * @param[in]   p_rows                One of g_tas_rows_uc2/uc3 (TAS_ROW_COUNT entries).
 * @param[out]  p_elapsed_ns          Set to the last submitted row's offset_ns, so the caller sleeps only the
 *                                    cycle remainder. May be NULL.
 * @retval      FSP_SUCCESS           Every matching row was either submitted or skipped as busy.
 * @retval      Any other error code  Propagated from the underlying FSP driver call (see tas_send_one_row()).
 **********************************************************************************************************************/
fsp_err_t tas_send_frames_staggered(const tas_row_t * p_rows, uint32_t * p_elapsed_ns)
{
    uint32_t last_offset_ns = 0U;

    for (uint8_t i = 0U; i < TAS_ROW_COUNT; i++)
    {
        const tas_row_t * p_row = &p_rows[i];
        if ((0U == (p_row->source_port_mask & (1U << TAS_CPU_PORT))) || !tas_mac_is_set(p_row->mac))
        {
            continue;
        }

        /* Rows are expected in ascending offset order; a windowless or out-of-order row just
         * submits immediately (the > check below guards against unsigned wraparound). */
        uint32_t target_offset_ns = (p_row->window_count > 0U) ? p_row->windows[0].offset_ns : last_offset_ns;
        if (target_offset_ns > last_offset_ns)
        {
            R_BSP_SoftwareDelay((target_offset_ns - last_offset_ns) / 1000U, BSP_DELAY_UNITS_MICROSECONDS);
            last_offset_ns = target_offset_ns;
        }

        fsp_err_t err = tas_send_one_row(p_row);
        if (FSP_SUCCESS != err) { return err; }
    }

    if (NULL != p_elapsed_ns)
    {
        *p_elapsed_ns = last_offset_ns;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function reads the switch's own ground-truth forwarded-descriptor counter for the given
 *              ingress port -- confirms a frame actually reached the switch, independent of TAS gate timing.
 *              Port 2 (CPU/GWCA) injects via Direct Descriptor forwarding -- the CPU writes the destination
 *              port(s) straight into the descriptor, bypassing the switch's own MAC-table lookup entirely --
 *              so it is counted separately (FWDDFDCN2, RZ/T2N User's Manual sec. 32.3.2.2), not by the same
 *              counter as externally-received traffic. Ports 0/1 (external, received on the wire) are
 *              genuinely classified through this project's MAC-type table entries
 *              (LAYER3_SWITCH_TABLE_ENTRY_TYPE_MAC), which increments the Layer 2 Forwarded Descriptor
 *              Counter (FWLTWFDCNi, sec. 32.3.2.4) -- not the Layer 3 one, which only counts IP-based
 *              LAYER3-type entries and would stay 0 here. Neither counter is exposed by the driver's own
 *              API, so both are read directly. Reading a register clears it, so the returned value is the
 *              delta since the last call, not a running total.
 * @param[in]   port                  Ingress port index (0-2).
 * @retval      The number of frames classified from that port since the last call.
 **********************************************************************************************************************/
uint32_t tas_get_ingress_count(uint8_t port)
{
    if (2U == port)
    {
        return R_MFWD->FWDDFDCN2;
    }

    if (0U == port)
    {
        return R_MFWD->FWLTWFDCN0;
    }

    return R_MFWD->FWLTWFDCN1;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_tas_ep)
 **********************************************************************************************************************/
