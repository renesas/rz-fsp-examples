/***********************************************************************************************************************
 * File Name    : ddr_ep.h
 * Description  : Contains declarations of data structures and functions used in ddr_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef DDR_EP_H_
#define DDR_EP_H_

#include <string.h>
#include "hal_data.h"
#include "ddr_config.h"

/* Reusable DDR exercise module. Every function calls BSP / CP15 / touches DDR, returns fsp_err_t (or
 * fills a result struct) and prints nothing - ddr_example.c owns all console output. The FSP DDR
 * surface used is small: bsp_ddr_init() (run by the BSP during startup, only reported here), the
 * Address Expander DDR mirror registers, and the R_BSP_Cache* range operations. Only ddr_config.{c,h}
 * changes when this module is moved into another project */

/* Generic console banner text printed once from hal_entry() after the BANNER_INFO, before the menu */
#define EP_INFO \
    "\r\nThis example project demonstrates LPDDR4 bring-up and a runtime memory exercise via the\r\n" \
    "Address Expander on a Renesas RZT2N EVK board. DDR is brought up during the BSP WarmStart\r\n" \
    "process (BSP feature, no driver instance).\r\n" \
    "  Pick a mode from the menu below at any time; while a mode is running, type a different\r\n" \
    "  mode number directly to switch.\r\n" \
    "  Modes: Bring-up & config info, Memory integrity, Address Expander remap, Cache / DMA\r\n" \
    "  coherency, DDR as system memory, Run all.\r\n"

/* Outcome of one write/verify pass over a block */
typedef struct st_ddr_result
{
    uint32_t bytes_tested;
    uint32_t error_count;              /* Mismatching bytes                                          */
    uint32_t first_err_addr;           /* Address of the first mismatch (0 if none)                 */
    uint32_t first_err_exp;            /* Expected value at the first mismatch                      */
    uint32_t first_err_got;            /* Value actually read at the first mismatch                 */
} ddr_result_t;

/* DDR / clock bring-up snapshot */
typedef struct st_ddr_ep_info
{
    bool     ddr_init_enabled;         /* BSP_CFG_DDR_INIT_ENABLE                                    */
    bool     pll2_running;             /* BSP_CFG_PLL2 == BSP_CLOCKS_PLL2_NORMAL                     */
    uint32_t ddrclk_hz;                /* Decoded from BSP_CFG_DDRDIVSEL_DDRCLKSEL                   */
    uint32_t mirror0_base;
    uint32_t mirror1_base;
    uint32_t ecc_reserved_bytes;
    uint32_t adxctl0_raw;              /* Live R_ADXC0->ADXCTL0                                      */
    uint64_t mirror0_phys;             /* Physical DDR base currently at 0xC000_0000                 */
    uint64_t mirror1_phys;             /* Physical DDR base currently at 0xE000_0000                 */
} ddr_ep_info_t;

/* Address Expander register readback */
typedef struct st_ddr_adxc_state
{
    uint32_t adxctl0_raw;
    uint32_t ddrmir0_field;
    uint32_t ddrmir1_field;
    uint64_t mirror0_phys;
    uint64_t mirror1_phys;
    bool     matches_request;          /* Set by ddr_ep_adxc_set(): readback == intended            */
} ddr_adxc_state_t;

/* Cache-coherency demo result */
typedef struct st_ddr_coherency_result
{
    bool     hazard_observed;          /* Non-cached read saw stale DRAM after an un-flushed write   */
    bool     coherent_after_fix;       /* Non-cached read matched after R_BSP_CacheCleanRange()      */
    uint32_t cached_view_addr;
    uint32_t noncached_view_addr;
    uint32_t mismatch_bytes;           /* Bytes that differed on the non-cached read                */
} ddr_coherency_result_t;

/* Throughput benchmark direction */
typedef enum e_ddr_bench_id
{
    DDR_BENCH_DDR_CACHED_COPY = 0,      /* DDR (cached)     -> DDR (cached)                          */
    DDR_BENCH_DDR_NONCACHED_COPY,       /* DDR (non-cached) -> DDR (non-cached)                      */
    DDR_BENCH_DDR_TO_SYSRAM,            /* DDR (cached)     -> SYSTEM_RAM                            */
    DDR_BENCH_SYSRAM_TO_DDR,            /* SYSTEM_RAM       -> DDR (cached)                          */
    DDR_BENCHMARK_COUNT
} ddr_bench_id_t;

typedef struct st_ddr_bench_result
{
    uint32_t bytes_per_iter;
    uint32_t iterations;
    uint32_t elapsed_us;
    uint32_t throughput_mibps;         /* MiB/s, integer (0 if the cycle counter did not advance)   */
    bool     data_ok;                  /* Copy verified                                             */
} ddr_bench_result_t;

/* DDR-resident buffer result */
typedef struct st_ddr_resident_result
{
    bool     in_ddr_window;            /* Buffer address is inside a DDR mirror window              */
    uint32_t buf_addr;                 /* Base of the reserved DDR buffer                           */
    uint32_t words;
    uint32_t checksum;                 /* Running XOR of the buffer after fill + read-back          */
    bool     data_ok;
} ddr_resident_result_t;

/* OTFDE report */
typedef struct st_ddr_otfde_info
{
    bool     peripheral_present;       /* BSP_FEATURE_BSP_HAS_OTFDE_PERIPHERAL                       */
    bool     support_enabled;          /* BSP_CFG_OTFDE0_SUPPORT_ENABLE / BSP_CFG_OTFDE1_SUPPORT_ENABLE */
    uint64_t window_start;             /* BSP_CFG_OTFDE_START_ADDRESS                               */
    uint64_t window_end;               /* BSP_CFG_OTFDE_END_ADDRESS                                 */
    bool     window_configured;        /* window_start < window_end                                 */
} ddr_otfde_info_t;

/* Public functions declarations */

void      ddr_ep_info_get(ddr_ep_info_t * p_info);
fsp_err_t ddr_ep_region_sweep(const ddr_region_t * p_region, ddr_result_t * p_res);
fsp_err_t ddr_ep_pattern_test(const ddr_region_t * p_region, const ddr_pattern_t * p_pat, ddr_result_t * p_res);
fsp_err_t ddr_ep_addrline_test(const ddr_region_t * p_region, ddr_result_t * p_res);
void      ddr_ep_adxc_get(ddr_adxc_state_t * p_state);
fsp_err_t ddr_ep_adxc_set(uint64_t mirror0_phys, uint64_t mirror1_phys, ddr_adxc_state_t * p_state);
fsp_err_t ddr_ep_adxc_restore(ddr_adxc_state_t * p_state);
fsp_err_t ddr_ep_alias_verify(ddr_result_t * p_res);
fsp_err_t ddr_ep_coherency_test(bool apply_fix, ddr_coherency_result_t * p_res);
fsp_err_t ddr_ep_bench(ddr_bench_id_t which, ddr_bench_result_t * p_res);
fsp_err_t ddr_ep_resident_test(ddr_resident_result_t * p_res);
void      ddr_ep_otfde_report(ddr_otfde_info_t * p_info);

#endif /* DDR_EP_H_ */
