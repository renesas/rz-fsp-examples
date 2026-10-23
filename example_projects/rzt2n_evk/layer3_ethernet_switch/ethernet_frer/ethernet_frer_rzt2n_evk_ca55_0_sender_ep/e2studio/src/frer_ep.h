/***********************************************************************************************************************
 * File Name    : frer_ep.h
 * Description  : Contains declarations of data structures and functions used in frer_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef FRER_EP_H_
#define FRER_EP_H_

#include <stdint.h>
#include <string.h>
#include "hal_data.h"

/* Switch topology. Port 2 is the CPU / GWCA internal port -- this Sender's only ingress.
 * FRER_PORT0/FRER_PORT1 are the two disjoint egress paths a companion Receiver project (on a
 * second board, cabled ETH0<->ETH0 and ETH1<->ETH1) eliminates duplicates across. */
#define FRER_PORT0                      (0U)
#define FRER_PORT1                      (1U)
#define FRER_CPU_PORT                   (2U)

/* L3 forwarding table sizing -- must not exceed the Configurator's "Number of Layer 3 routing
 * entries" for g_layer3_switch0 (currently 10). This Sender programs exactly 1. */
#define FRER_L3_ENTRY_MAX_NUM           (10U)

/* The one stream this Sender replicates, identified by IPv4 5-tuple (same scheme as the L3
 * Forwarding EP). FRER_STREAM_DST_MAC_BASE is the pre-replication address the CPU-injected
 * frame carries; each member stream's update config rewrites it to its own address below so a
 * Receiver's two ingress entries can each filter on a distinct destination MAC. */
#define FRER_STREAM_SRC_MAC             {0x70, 0xB3, 0xD5, 0x34, 0x00, 0x10}
#define FRER_STREAM_DST_MAC_BASE        {0x44, 0x44, 0x44, 0x00, 0x00, 0x00}
#define FRER_STREAM_DST_MAC_PORT0       {0x44, 0x44, 0x44, 0x00, 0x00, 0x0A}   /* Member stream A, out Port0 */
#define FRER_STREAM_DST_MAC_PORT1       {0x44, 0x44, 0x44, 0x00, 0x00, 0x0B}   /* Member stream B, out Port1 */
#define FRER_STREAM_SRC_IP              {192, 168, 10, 3}
#define FRER_STREAM_DST_IP              {192, 168, 10, 4}
#define FRER_STREAM_UDP_SRC_PORT        (50000U)
#define FRER_STREAM_UDP_DST_PORT        (50000U)

/* CPU-generated frame TX */
#define FRER_TX_DESC_NUM                (4U)      /* Descriptor queue depth: 3 usable + 1 LEMPTY terminator */
#define FRER_FRAME_SIZE                 (1514U)   /* Full-size Ethernet frame, incl. R-TAG and IPv4/UDP headers */
#define FRER_SEND_PERIOD_MS             (100U)    /* One replicated frame injected per this interval */
#define FRER_RATE_REPORT_EVERY          (50U)     /* Print the injection rate every N frames sent */

/* Generic console banner text printed once from hal_entry() before the main loop starts */
#define EP_INFO \
    "\r\nThis example project demonstrates IEEE 802.1CB Frame Replication and Elimination for\r\n" \
    "Reliability (FRER) using the ESWM (Ethernet Switch Module) on two Renesas RZ/T2N EVK boards\r\n" \
    "configured as a Sender and a Receiver.\r\n" \
    "\r\n" \
    "The Sender replicates each frame and transmits duplicate copies through ETH0 and ETH1. The\r\n" \
    "Receiver removes duplicate frames received on both ports and forwards only a single valid\r\n" \
    "frame.\r\n" \
    "\r\n" \
    "For each successfully recovered frame, the Receiver application displays the R-TAG sequence\r\n" \
    "number and the ingress port. Status and results are displayed on the Terminal Emulator.\r\n\r\n"

/* Snapshot of what was actually programmed, for the startup print */
typedef struct st_frer_ep_info
{
    uint32_t l3_entry_count;
    uint32_t tx_queue_index;
} frer_info_t;

/* TX diagnostics -- frer_send_one() returns FSP_SUCCESS both when a frame is actually handed to
 * hardware and when it is silently skipped (queue still busy with the previous frame, or the ring
 * momentarily full), so "no error returned" alone cannot tell "N frames sent" apart from "N calls
 * made, all of them skipped". See frer_get_tx_diag(). */
typedef struct st_frer_tx_diag
{
    uint32_t submitted;         /* SetDescriptor + StartDescriptorQueue both actually succeeded */
    uint32_t skipped_overflow;  /* SetDescriptor returned FSP_ERR_OVERFLOW (ring full) */
    uint32_t skipped_busy;      /* StartDescriptorQueue returned FSP_ERR_IN_USE (previous frame
                                  * not yet finished transmitting) */
} frer_tx_diag_t;

/* Public functions declarations */

fsp_err_t frer_open(void);
fsp_err_t frer_configure_stream(frer_info_t * p_info);
fsp_err_t frer_tx_queue_create(frer_info_t * p_info);
fsp_err_t frer_send_one(void);
void      frer_get_tx_diag(frer_tx_diag_t * p_diag);

#endif /* FRER_EP_H_ */
