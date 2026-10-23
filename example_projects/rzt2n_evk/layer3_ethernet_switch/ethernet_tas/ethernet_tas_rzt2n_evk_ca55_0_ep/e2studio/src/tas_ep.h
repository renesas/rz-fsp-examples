/***********************************************************************************************************************
 * File Name    : tas_ep.h
 * Description  : Contains declarations of data structures and functions used in tas_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef TAS_EP_H_
#define TAS_EP_H_

#include <string.h>
#include "hal_data.h"
#include "tas_config.h"

/* Generic console banner text printed once from hal_entry() after the BANNER_INFO, before the
 * use-case menu is ever shown. */
#define EP_INFO \
    "\r\nThis example project demonstrates the IEEE 802.1Qbv Time-Aware Shaper (TAS) on the\r\n" \
    "ESWM (Ethernet Switch Module) of a single Renesas RZT2N EVK board. TAS lets the switch\r\n" \
    "hardware open and close each egress queue's transmission gate on a repeating, time-based\r\n" \
    "cycle, so time-critical traffic classes get guaranteed transmission windows and cannot be\r\n" \
    "delayed by best-effort traffic.\r\n"

/* CPU-generated frame TX sizing (used by TAS_UC_CPU_GENERATED and TAS_UC_COMBINED) */
#define TAS_TX_HEADER_LEN  (14U)   /* Dst(6) + src(6) + EtherType(2) */

/* GWCA is a DMA engine and bypasses the CPU cache -- descriptor/frame buffers must be
 * non-cached and 32-byte aligned, or GWCA may read stale data and never pick up new frames.
 * One INDEPENDENT descriptor queue per TAS priority queue: a frame for a queue whose gate is
 * still closed sits held in its slot, and a shared slot would let that block every other
 * queue's submission. 2 descriptors per queue (1 frame + 1 terminator) -- only one per-cycle
 * frame per queue is ever in flight. */
#define TAS_TX_PER_QUEUE_DESC_NUM (2U)

/* Snapshot of what was actually programmed */
typedef struct st_tas_ep_info
{
    uint64_t cycle_start_ns;
    uint32_t cycle_time_ns;
    uint8_t  timer_number;
    uint8_t  port_count;
    uint8_t  ports[BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS];
    uint8_t  entries_per_gate[BSP_FEATURE_ESWM_MAX_EXTERNAL_PORTS][BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM];
    uint8_t  mac_entry_count;
} tas_info_t;

/* Public functions declarations */

fsp_err_t tas_open(void);
fsp_err_t tas_timer_start(void);
fsp_err_t tas_configure_schedule(const tas_row_t * p_rows, tas_info_t * p_info);
fsp_err_t tas_configure_mac_table(const tas_row_t * p_rows, tas_info_t * p_info);
fsp_err_t tas_clear_mac_table(void);
fsp_err_t tas_enable(void);
fsp_err_t tas_init(const tas_row_t * p_rows, tas_info_t * p_info);
fsp_err_t tas_tx_queue_create(const tas_row_t * p_rows);
fsp_err_t tas_send_frames(const tas_row_t * p_rows);
fsp_err_t tas_send_frames_staggered(const tas_row_t * p_rows, uint32_t * p_elapsed_ns);
uint32_t  tas_get_ingress_count(uint8_t port);

#endif /* TAS_EP_H_ */
