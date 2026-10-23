/***********************************************************************************************************************
 * File Name    : tas_config.c
 * Description  : TAS schedule tables - one per use case, all three exist in the binary at once.
 *                tas_example.c's console menu picks a pointer to one of them at runtime.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "tas_config.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_tas_ep
 * @{
 **********************************************************************************************************************/

/* UC1: port-to-port. External traffic enters Port 1 (TAS_SOURCE_PORT), TAS shapes egress on
 * Port 0 (TAS_DESTINATION_PORT) -- ingress and egress must be different physical ports; the
 * switch does not re-egress a frame out the same port it ingressed on.
 * 25 ms cycle, Q0-Q3 each open one 5 ms slot back-to-back (0/5/10/15 ms), Q4-Q7 always closed. */
const tas_row_t g_tas_rows_uc1[TAS_ROW_COUNT] =
{
    /* Q0 -- gate open [0, 5) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 0U,
        .window_count     = 1U,
        .windows          = {{0UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0A},
        .source_port_mask = (1U << TAS_SOURCE_PORT),
    },
    /* Q1 -- gate open [5, 10) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 1U,
        .window_count     = 1U,
        .windows          = {{5000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0B},
        .source_port_mask = (1U << TAS_SOURCE_PORT),
    },
    /* Q2 -- gate open [10, 15) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 2U,
        .window_count     = 1U,
        .windows          = {{10000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0C},
        .source_port_mask = (1U << TAS_SOURCE_PORT),
    },
    /* Q3 -- gate open [15, 20) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 3U,
        .window_count     = 1U,
        .windows          = {{15000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0D},
        .source_port_mask = (1U << TAS_SOURCE_PORT),
    },
    /* Q4-Q7 -- permanently closed, no MAC table entry */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 4U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 5U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 6U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 7U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
};

/* UC2: CPU-generated. The MPU itself injects frames via GWCA (Port 2 = TAS_CPU_PORT),
 * TAS shapes egress on Port 0 (TAS_DESTINATION_PORT). Same gate timing as UC1 -- only the
 * MAC steering source differs. */
const tas_row_t g_tas_rows_uc2[TAS_ROW_COUNT] =
{
    /* Q0 -- gate open [0, 5) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 0U,
        .window_count     = 1U,
        .windows          = {{0UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0A},
        .source_port_mask = (1U << TAS_CPU_PORT),
    },
    /* Q1 -- gate open [5, 10) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 1U,
        .window_count     = 1U,
        .windows          = {{5000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0B},
        .source_port_mask = (1U << TAS_CPU_PORT),
    },
    /* Q2 -- gate open [10, 15) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 2U,
        .window_count     = 1U,
        .windows          = {{10000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0C},
        .source_port_mask = (1U << TAS_CPU_PORT),
    },
    /* Q3 -- gate open [15, 20) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 3U,
        .window_count     = 1U,
        .windows          = {{15000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0D},
        .source_port_mask = (1U << TAS_CPU_PORT),
    },
    /* Q4-Q7 -- permanently closed, no MAC table entry */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 4U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 5U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 6U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 7U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
};

/* UC3: combined. Each of the 4 active queues accepts a MAC hit from EITHER TAS_SOURCE_PORT
 * (external, like UC1) OR TAS_CPU_PORT (CPU-injected via GWCA, like UC2) -- source_port_mask
 * ORs both port bits together. Both traffic origins share the exact same gate schedule, so a
 * frame from either source lands in the same queue and is shaped identically -- proving TAS
 * gates purely by queue, independent of where the frame actually entered the switch. */
const tas_row_t g_tas_rows_uc3[TAS_ROW_COUNT] =
{
    /* Q0 -- gate open [0, 5) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 0U,
        .window_count     = 1U,
        .windows          = {{0UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0A},
        .source_port_mask = (1U << TAS_SOURCE_PORT) | (1U << TAS_CPU_PORT),
    },
    /* Q1 -- gate open [5, 10) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 1U,
        .window_count     = 1U,
        .windows          = {{5000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0B},
        .source_port_mask = (1U << TAS_SOURCE_PORT) | (1U << TAS_CPU_PORT),
    },
    /* Q2 -- gate open [10, 15) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 2U,
        .window_count     = 1U,
        .windows          = {{10000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0C},
        .source_port_mask = (1U << TAS_SOURCE_PORT) | (1U << TAS_CPU_PORT),
    },
    /* Q3 -- gate open [15, 20) ms */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 3U,
        .window_count     = 1U,
        .windows          = {{15000000UL, 5000000UL}, {0UL, 0UL}},
        .mac              = {0x44, 0x44, 0x44, 0x00, 0x00, 0x0D},
        .source_port_mask = (1U << TAS_SOURCE_PORT) | (1U << TAS_CPU_PORT),
    },
    /* Q4-Q7 -- permanently closed, no MAC table entry */
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 4U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 5U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 6U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
    {
        .port             = TAS_DESTINATION_PORT,
        .queue            = 7U,
        .window_count     = 0U,
        .windows          = {{0UL, 0UL}, {0UL, 0UL}},
        .mac              = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        .source_port_mask = 0U,
    },
};

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_tas_ep)
 **********************************************************************************************************************/
