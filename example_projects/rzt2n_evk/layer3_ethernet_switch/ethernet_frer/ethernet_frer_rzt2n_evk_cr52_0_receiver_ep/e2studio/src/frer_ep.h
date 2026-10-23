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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "hal_data.h"

/* Switch topology and stream identity -- must stay byte-identical to the companion Sender
 * project's frer_ep.h. FRER_PORT0/FRER_PORT1 are this board's two ingress paths, cabled
 * ETH0<->ETH0 and ETH1<->ETH1 to the Sender's own FRER_PORT0/FRER_PORT1. */
#define FRER_PORT0                      (0U)
#define FRER_PORT1                      (1U)
#define FRER_CPU_PORT                   (2U)
#define FRER_PORT_COUNT                 (2U)

/* L3 forwarding table sizing -- must not exceed the Configurator's "Number of Layer 3 routing
 * entries" for g_layer3_switch0 (currently 10). This Receiver programs 2 (one per ingress port). */
#define FRER_L3_ENTRY_MAX_NUM           (10U)

/* The one frame the Sender replicates, identified by IPv4 5-tuple. FRER_STREAM_DST_MAC_BASE is
 * the address this Receiver rewrites the delivered frame's destination back to before handing
 * it to the CPU port; FRER_STREAM_DST_MAC_PORT0/1 are the two member streams' addresses, one
 * arriving on each ingress port */
#define FRER_STREAM_SRC_MAC             {0x70, 0xB3, 0xD5, 0x34, 0x00, 0x10}
#define FRER_STREAM_DST_MAC_BASE        {0x44, 0x44, 0x44, 0x00, 0x00, 0x00}
#define FRER_STREAM_DST_MAC_PORT0       {0x44, 0x44, 0x44, 0x00, 0x00, 0x0A}
#define FRER_STREAM_DST_MAC_PORT1       {0x44, 0x44, 0x44, 0x00, 0x00, 0x0B}
#define FRER_STREAM_SRC_IP              {192, 168, 10, 3}
#define FRER_STREAM_DST_IP              {192, 168, 10, 4}
#define FRER_STREAM_UDP_SRC_PORT        (50000U)
#define FRER_STREAM_UDP_DST_PORT        (50000U)

/* Descriptor queue depth per port, including the ring's terminator slot. FRER_RX_DESC_NUM - 1 is
 * the TOTAL number of frames one RX queue can EVER capture over its whole lifetime, not a
 * recycling window -- RX's StartDescriptorQueue() never resets the driver's internal tail index
 * the way TX's does, so once every slot has been armed once, no more can be. frer_restart() is
 * the only way to get a queue back to a fresh tail=0 without a board reset. Sized to fit the
 * RZT2N linker's NONCACHE region with margin -- raise only after re-checking it still fits. */
#define FRER_RX_DESC_NUM                (100U)
#define FRER_FRAME_SIZE                 (1514U)   /* Must be >= the Sender's own FRER_FRAME_SIZE */
#define FRER_POLL_PERIOD_MS             (10U)     /* How often each RX queue is polled */
#define FRER_RTAG_SEQUENCE_OFFSET       (16U)     /* Dst(6) + src(6) + R-TAG TPID/reserved(4) */

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
    uint32_t rx_queue_index[FRER_PORT_COUNT];
} frer_info_t;

/* Per-port RX diagnostics, read back by frer_get_rx_diag(): frames_ok climbing confirms real,
 * continuing reception; frames_error > 0 means frames reached this queue but were
 * hardware-flagged, which frer_poll_receive() otherwise drops silently. */
typedef struct st_frer_rx_diag
{
    uint32_t frames_ok;      /* Delivered to this queue with no hardware error flag */
    uint32_t frames_error;   /* Delivered to this queue but desc.basic.err was non-zero */
} frer_rx_diag_t;

/* Hardware's own FRER Passed/Discarded Packet Counters for the switch's three FRER table
 * entries -- see frer_get_hw_frer_counters()'s doc comment for what each field means and how
 * to read it. Deltas since the last call, not running totals (the underlying registers clear
 * on read). */
typedef struct st_frer_hw_counters
{
    uint32_t seqrec_passed;              /* Shared sequence recovery (entry #0): unique frame accepted */
    uint32_t seqrec_discarded;           /* Shared sequence recovery (entry #0): duplicate eliminated */
    uint32_t port0_individual_passed;    /* Port0's individual recovery (entry #1): integrity check ok */
    uint32_t port0_individual_flagged;   /* Port0's individual recovery (entry #1): stuck/error flagged */
    uint32_t port1_individual_passed;    /* Port1's individual recovery (entry #2): integrity check ok */
    uint32_t port1_individual_flagged;   /* Port1's individual recovery (entry #2): stuck/error flagged */
} frer_hw_counters_t;

/* Public functions declarations */

fsp_err_t frer_open(void);
fsp_err_t frer_rx_queue_create(uint8_t port, frer_info_t * p_info);
fsp_err_t frer_configure_recovery(frer_info_t * p_info);
bool      frer_poll_receive(uint8_t port, uint16_t * p_sequence, uint16_t * p_length);
void      frer_get_rx_diag(uint8_t port, frer_rx_diag_t * p_diag);
void      frer_get_hw_frer_counters(frer_hw_counters_t * p_counters);
void      frer_get_l3_classification(uint32_t * p_total_port0, uint32_t * p_total_port1);
fsp_err_t frer_restart(frer_info_t * p_info);

#endif /* FRER_EP_H_ */
