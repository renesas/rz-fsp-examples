/***********************************************************************************************************************
 * File Name    : tas_config.h
 * Description  : User-editable TAS configuration - schedule tables, timer selection, cycle geometry.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef TAS_CONFIG_H_
#define TAS_CONFIG_H_

#include <stdint.h>
#include "hal_data.h"

/* Use case selection.
 * Chosen at RUNTIME from the console menu (see tas_example.c) -- no rebuild needed to
 * switch between them; the board stays open the whole session, only the schedule/MAC
 * table get reprogrammed per selection.
 * 1 = port-to-port: external traffic enters TAS_SOURCE_PORT, TAS shapes egress on
 *     TAS_DESTINATION_PORT. Needs a PC running tas_send_test_frames.py on ETH1.
 * 2 = CPU-generated: the MPU itself injects frames via GWCA (TAS_CPU_PORT), TAS shapes egress
 *     on TAS_DESTINATION_PORT. Self-contained, no PC/script needed.
 * 3 = combined: external traffic on TAS_SOURCE_PORT AND CPU-generated traffic via TAS_CPU_PORT
 *     are BOTH accepted into the same MAC table entries / same queues at the same time --
 *     demonstrates that TAS gates per-queue, not per-ingress-source. Also self-contained (the
 *     CPU-generated half needs no PC), but accepts external traffic too if you run the script. */
typedef enum e_tas_use_case
{
    TAS_UC_PORT_TO_PORT  = 1,
    TAS_UC_CPU_GENERATED = 2,
    TAS_UC_COMBINED      = 3,
} tas_use_case_t;

/* Timer selection
 * 0 = ESWM local free-running timer (standalone TAS, no gPTP stack)
 * 1 = ESWM gPTP-synchronized timer  (gPTP GM / Bridge / Slave projects) */
#define TAS_GPTP_TIMER                  (0)

/* Start the ESWM timer from tas_ep.
 * 1 = standalone: nothing else starts it.
 * 0 = gPTP project: the gPTP stack already started both timers; starting again
 *     would reset the synchronized clock. */
#define TAS_START_TIMER                 (1)
/* Ns/tick written to PTPTIVC0.TIV -- must equal the ESWM timer's real clock period (5 on this board) */
#define TAS_GPTP_CLOCK_PERIOD           (5)

/* Cycle geometry */
#define TAS_CYCLE_TIME_NS               (25000000UL)      /* 25 ms */

/* Cycle start time source */
#define TAS_CYCLE_START_MODE_FIXED      (0)               /* Use TAS_CYCLE_START_FIXED_NS */
#define TAS_CYCLE_START_MODE_DYNAMIC    (1)               /* Read timer, add margin, round up */
#define TAS_CYCLE_START_MODE            TAS_CYCLE_START_MODE_FIXED
#define TAS_CYCLE_START_FIXED_NS        (3000000000ULL)
#define TAS_CYCLE_START_MARGIN_NS       (500000000ULL)    /* DYNAMIC mode only */

/* Reject a DYNAMIC-mode timer reading below this - means the timer is not running yet */
#define TAS_CYCLE_START_MIN_VALID_NS    (1000000000ULL)   /* 1 s */

/* Port 2 is the CPU / GWCA internal port (frames the CPU itself generates).
 * TAS_SOURCE_PORT must differ from TAS_DESTINATION_PORT -- the switch does not re-egress a
 * frame out the same physical port it ingressed on */
#define TAS_SOURCE_PORT                 (1U)
#define TAS_CPU_PORT                    (2U)
#define TAS_DESTINATION_PORT            (0U)

/* MAC table */
#define TAS_MAC_TABLE_ENABLE            (1)

/* Table sizing */
#define TAS_ROW_COUNT                   (8U)
#define TAS_MAX_WINDOWS_PER_ROW         (2U)
#define TAS_MAX_ENTRIES_PER_GATE        ((2U * TAS_MAX_WINDOWS_PER_ROW) + 1U)

/* Total entries across all 8 gates share one RAM region per port */
#define TAS_MAX_TOTAL_ENTRIES_PER_PORT  (32U)

/* CPU-generated frame TX (used whenever the running use case needs it) */
#define TAS_FRAME_SIZE_MAX              (128U)   /* Bytes, enough for a minimal Eth frame */
/* Source MAC placed in generated frames -- this board's Port 0 MAC address (TAS_DESTINATION_PORT,
 * the port these frames actually egress from). */
#define TAS_TX_SOURCE_MAC               {0x00, 0x11, 0x22, 0x33, 0x44, 0x00}
#define TAS_TX_ETHERTYPE                (0x88B5U)   /* IEEE local experimental */

/* Schedule table */

/* One open window inside a cycle: gate stays CLOSED until offset_ns, then OPEN for duration_ns */
typedef struct st_tas_window
{
    uint32_t offset_ns;
    uint32_t duration_ns;
} tas_window_t;

/* One row per (port, queue) */
typedef struct st_tas_row
{
    uint8_t      port;                                  /* Egress (gated) port */
    uint8_t      queue;                                 /* 0..7, IPV gate index */
    uint8_t      window_count;                          /* 0 = gate permanently CLOSED */
    tas_window_t windows[TAS_MAX_WINDOWS_PER_ROW];      /* Ascending, non-overlapping */
    uint8_t      mac[6];                                /* All-zero = no MAC table entry */
    uint8_t      source_port_mask;                      /* Bitmask of ingress ports to steer this MAC
                                                          * from -- bit N set = accept from port N.
                                                          * May combine multiple ports (e.g. UC3). */
} tas_row_t;

/* One schedule table per use case, picked at runtime by tas_example.c's console menu */
extern const tas_row_t g_tas_rows_uc1[TAS_ROW_COUNT];   /* Port-to-port */
extern const tas_row_t g_tas_rows_uc2[TAS_ROW_COUNT];   /* CPU-generated */
extern const tas_row_t g_tas_rows_uc3[TAS_ROW_COUNT];   /* Combined */

#endif /* TAS_CONFIG_H_ */
