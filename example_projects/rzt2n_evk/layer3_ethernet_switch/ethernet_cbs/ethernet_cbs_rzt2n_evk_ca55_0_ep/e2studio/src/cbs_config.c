/***********************************************************************************************************************
 * File Name    : cbs_config.c
 * Description  : Contains data structures and functions used in cbs_config.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "cbs_config.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_cbs_ep
 * @{
 **********************************************************************************************************************/

/* UC1: port-to-port. External traffic enters Port 1 (CBS_SOURCE_PORT), CBS shapes egress
 * on Port 0 (CBS_DESTINATION_PORT). Q0 capped at 10% of link bandwidth, Q1 at 30%; Q2-Q7
 * unused (no MAC entry, 0%). Console menu choices [2]/[3] in cbs_example.c isolate Q0/Q1
 * individually, matched against the PC-side script's own --queue option for that row. */
const cbs_row_t g_cbs_rows_uc1[CBS_ROW_COUNT] =
{
/*   queue  band%  burst  mac                              src mask                  */
    { 0U,   10U,   2U,   {0x44,0x44,0x44,0x00,0x00,0x0A}, (1U << CBS_SOURCE_PORT) },
    { 1U,   30U,   2U,   {0x44,0x44,0x44,0x00,0x00,0x0B}, (1U << CBS_SOURCE_PORT) },
    { 2U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 3U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 4U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 5U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 6U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 7U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
};

/* UC2: CPU-generated. The MPU itself blasts frames via GWCA (Port 2 = CBS_CPU_PORT), CBS
 * shapes egress on Port 0 (CBS_DESTINATION_PORT). Q0 capped at 10%, Q1 at 30%; both queues
 * share CBS_DESTINATION_PORT's one physical link when run together, splitting its real
 * throughput between them -- verify each queue's cap in isolation via choices [5]/[6]. */
const cbs_row_t g_cbs_rows_uc2[CBS_ROW_COUNT] =
{
/*   queue  band%  burst  mac                              src mask               */
    { 0U,   10U,   2U,   {0x44,0x44,0x44,0x00,0x00,0x0A}, (1U << CBS_CPU_PORT) },
    { 1U,   30U,   2U,   {0x44,0x44,0x44,0x00,0x00,0x0B}, (1U << CBS_CPU_PORT) },
    { 2U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 3U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 4U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 5U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 6U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
    { 7U,    0U,   0U,   {0x00,0x00,0x00,0x00,0x00,0x00}, 0U },
};

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_cbs_ep)
 **********************************************************************************************************************/
