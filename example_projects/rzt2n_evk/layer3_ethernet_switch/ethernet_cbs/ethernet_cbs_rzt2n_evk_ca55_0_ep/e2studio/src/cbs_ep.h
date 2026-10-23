/***********************************************************************************************************************
 * File Name    : cbs_ep.h
 * Description  : Contains declarations of data structures and functions used in cbs_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef CBS_EP_H_
#define CBS_EP_H_

#include "hal_data.h"
#include "cbs_config.h"

/* Generic console banner text printed once from hal_entry() before the mode menu */
#define EP_INFO \
    "\r\nThis example project demonstrates the IEEE 802.1Qav Credit-Based Shaper (CBS) on the\r\n" \
    "ESWM (Ethernet Switch Module) of a single Renesas RZT2N EVK board.\r\n" \
    "CBS limits the bandwidth of an egress queue to a configured percentage of the link speed,\r\n" \
    "so a queue cannot exceed its share however much traffic is offered. Each mode runs a queue\r\n" \
    "with CBS disabled (baseline) or enabled.\r\n"

#define CBS_TX_HEADER_LEN  (14U)   /* Dst(6) + src(6) + EtherType(2) */

/* Snapshot of what was actually programmed, for cbs_example.c to print without recomputing
 * anything or calling any FSP API itself. */
typedef struct st_cbs_ep_info
{
    bool    cbs_enabled;
    uint8_t queue_count;
    uint8_t queues[BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM];
    uint8_t band_width_pct[BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM];
    uint8_t mac_entry_count;
    uint8_t oper_enable_mask;   /* EACOEM.CE read back after applying: bit q set = queue q is actually
                                 * shaping in hardware right now (must match the queues listed) */
} cbs_info_t;

/* Public functions declarations */

fsp_err_t cbs_open(void);
fsp_err_t cbs_configure_mac_table(const cbs_row_t * p_rows, cbs_info_t * p_info);
fsp_err_t cbs_clear_mac_table(void);
fsp_err_t cbs_configure_bandwidth(const cbs_row_t * p_rows, bool cbs_enabled, cbs_info_t * p_info);
fsp_err_t cbs_tx_queue_create(void);
fsp_err_t cbs_tx_burst_prepare(const uint8_t dst_mac[6], uint8_t queue);
fsp_err_t cbs_send_frames_burst(void);

#endif /* CBS_EP_H_ */
