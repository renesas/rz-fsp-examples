/***********************************************************************************************************************
 * File Name    : L3forwarding.h
 * Description  : Contains declarations of data structures, macros, and functions used in L3forwarding.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef L3FORWARDING_H_
#define L3FORWARDING_H_

#include "common_utils.h"
#include "r_rmac.h"
#include "r_layer3_switch.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Frame and address sizes */
#define L3_FORWARDING_MAC_ADDRESS_LENGTH    (6U)
#define L3_FORWARDING_IP_ADDRESS_LENGTH     (4U)
#define L3_FORWARDING_MIN_FRAME_SIZE        (64U)

/* Physical Ethernet port indices */
#define L3_FORWARDING_ETH0                  (0U)
#define L3_FORWARDING_ETH1                  (1U)

/* CPU_INJECT is the internal GWCA software-inject port -- no physical connector on the board.
 * Frames written via R_LAYER3_SWITCH descriptor queues enter ESWM through this port,
 * then ESWM hardware forwards them out of ETH0 or ETH1 based on the forwarding table. */
#define L3_FORWARDING_CPU_INJECT_PORT       (2U)

/* VLAN IDs: VID=10 maps to ETH0, VID=20 maps to ETH1 */
#define L3_FORWARDING_VLAN_ID_ETH0          (10U)
#define L3_FORWARDING_VLAN_ID_ETH1          (20U)

/* UDP port used for L3 stream filter key (both src and dst) */
#define L3_FORWARDING_UDP_PORT              (9999U)

#define EP_INFO \
    "\r\nThis example project demonstrates how to use the Ethernet Switch (ESWM) forwarding table of the\r\n" \
    "RZ/T2N EVK to forward frames between ETH0 and ETH1 by destination MAC address (L2), by 802.1Q\r\n" \
    "VLAN ID, and by IPv4 routing (L3, with TTL decrement and destination MAC rewrite). Frames are\r\n" \
    "injected by the CPU, so no external traffic generator is needed. Table entries that reject\r\n" \
    "(drop) frames are demonstrated as well.\r\n" \
    "\r\n" \
    "Connect ETH0 to a PC NIC in the 192.168.10.x subnet and ETH1 to a PC NIC in the\r\n" \
    "192.168.20.x subnet, then select a forwarding mode from the main menu.\r\n"

/* Forwarding mode type */
typedef enum e_l3_forwarding_mode
{
    L3_FORWARDING_MODE_MAC    = '1',    /* L2 forwarding by destination MAC */
    L3_FORWARDING_MODE_VLAN   = '2',    /* L2 forwarding by 802.1Q VLAN ID */
    L3_FORWARDING_MODE_LAYER3 = '3',    /* L3 IP routing with TTL decrement and MAC_DST rewrite */
} l3_forwarding_mode_t;

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
void l3_forwarding_main(void);

#endif /* L3FORWARDING_H_ */
