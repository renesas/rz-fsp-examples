/***********************************************************************************************************************
 * File Name    : ddr_config.c
 * Description  : Contains data structures and functions used in ddr_config.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "ddr_config.h"

/***********************************************************************************************************************
 * @addtogroup bsp_ddr_ep
 * @{
 **********************************************************************************************************************/

/* TOP / MID / END sample blocks in each 512 MB mirror window.
 * M0_TOP starts one ECC-reserve past the window base; *_END is the last full DDR_EP_BLOCK_BYTES block
 * that fits below the window's top address (0xDFFFF000 / 0xFFFFF000 for a 0x1000-byte block) */
const ddr_region_t g_ddr_regions[DDR_REGION_COUNT] =
{
    [DDR_REGION_M0_TOP] =
    {
        .name   = "mirror0 TOP",
        .addr   = (uint32_t) DDR_EP_MIRROR0_USABLE_BASE,
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 1U,
    },
    [DDR_REGION_M0_MID] =
    {
        .name   = "mirror0 MID",
        .addr   = (uint32_t) (DDR_EP_MIRROR0_BASE + (DDR_EP_MIRROR_WINDOW_SIZE / 2U)),
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 1U,
    },
    [DDR_REGION_M0_END] =
    {
        .name   = "mirror0 END",
        .addr   = (uint32_t) (DDR_EP_MIRROR0_END - (DDR_EP_BLOCK_BYTES - 1U)),
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 1U,
    },
    [DDR_REGION_M1_TOP] =
    {
        .name   = "mirror1 TOP",
        .addr   = (uint32_t) DDR_EP_MIRROR1_BASE,
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 0U,
    },
    [DDR_REGION_M1_MID] =
    {
        .name   = "mirror1 MID",
        .addr   = (uint32_t) (DDR_EP_MIRROR1_BASE + (DDR_EP_MIRROR_WINDOW_SIZE / 2U)),
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 0U,
    },
    [DDR_REGION_M1_END] =
    {
        .name   = "mirror1 END",
        .addr   = (uint32_t) (DDR_EP_MIRROR1_END - (DDR_EP_BLOCK_BYTES - 1U)),
        .size   = DDR_EP_BLOCK_BYTES,
        .cached = 0U,
    },
};

/* Data-pattern table. The four constant fills carry their byte in .fill; the generated patterns
 * (address-as-data, walking-1/0, LFSR) are produced by ddr_ep.c from the .kind tag */
const ddr_pattern_t g_ddr_patterns[DDR_PATTERN_COUNT] =
{
    { .name = "0x00 fill",        .kind = DDR_PAT_CONST, .fill = 0x00U },
    { .name = "0xFF fill",        .kind = DDR_PAT_CONST, .fill = 0xFFU },
    { .name = "0x55 fill",        .kind = DDR_PAT_CONST, .fill = 0x55U },
    { .name = "0xAA fill",        .kind = DDR_PAT_CONST, .fill = 0xAAU },
    { .name = "address-as-data",  .kind = DDR_PAT_ADDR,  .fill = 0x00U },
    { .name = "walking-1",        .kind = DDR_PAT_WALK1, .fill = 0x00U },
    { .name = "walking-0",        .kind = DDR_PAT_WALK0, .fill = 0x00U },
    { .name = "LFSR pseudo-random", .kind = DDR_PAT_LFSR,  .fill = 0x00U },
};

/* Address Expander runtime-remap sequence.
 * step 0 (alias)  : both windows -> low half, so 0xC0.. (cached) and 0xE0.. (non-cached) hit the same cells.
 * step 1 (swap)   : mirror0 -> high half, mirror1 -> low half - the two halves exchanged versus power-on.
 * step 2 (restore): back to the Configurator default (low half at mirror 0, high half at mirror 1) */
const ddr_adxc_step_t g_ddr_adxc_steps[DDR_ADXC_STEP_COUNT] =
{
    {
        .desc         = "alias  (mirror0 & mirror1 -> physical low half 0x2_0000_0000)",
        .mirror0_phys = DDR_EP_PHYS_LOW_HALF,
        .mirror1_phys = DDR_EP_PHYS_LOW_HALF,
    },
    {
        .desc         = "swap   (mirror0 -> high half, mirror1 -> low half)",
        .mirror0_phys = DDR_EP_PHYS_HIGH_HALF,
        .mirror1_phys = DDR_EP_PHYS_LOW_HALF,
    },
    {
        .desc         = "restore(power-on default: low half @ mirror0, high half @ mirror1)",
        .mirror0_phys = DDR_ADXC_DEFAULT_MIRROR0_PHYS,
        .mirror1_phys = DDR_ADXC_DEFAULT_MIRROR1_PHYS,
    },
};

/***********************************************************************************************************************
 * @} (end addtogroup bsp_ddr_ep)
 **********************************************************************************************************************/
