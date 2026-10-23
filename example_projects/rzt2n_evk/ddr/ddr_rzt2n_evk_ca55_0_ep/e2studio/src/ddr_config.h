/***********************************************************************************************************************
 * File Name    : ddr_config.h
 * Description  : Contains declarations of data structures and functions used in ddr_config.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef DDR_CONFIG_H_
#define DDR_CONFIG_H_

#include <stdint.h>
#include "hal_data.h"

/* User-editable DDR EP configuration: memory map, sample regions, data-pattern table, Address Expander
 * remap targets and benchmark sizes. This is the only file to retarget when reusing ddr_ep.{c,h} on
 * another board */

/* DDR address map - the CPU view through the Address Expander mirror windows.
 *   Physical DDR (34-bit) : 0x2_0000_0000 .. 0x2_3FFF_FFFF   (1 GB LPDDR4)
 *   ADXC0 DDR mirror 0    : 0xC000_0000  .. 0xDFFF_FFFF      (512 MB window, MPU region 20, cached / write-back)
 *   ADXC0 DDR mirror 1    : 0xE000_0000  .. 0xFFFF_FFFF      (512 MB window, MPU region 21, non-cached)
 * The first 64 bytes (0x40) of physical DDR are reserved for inline ECC and must never be accessed
 * (BSP_DDRTOP_ECC_START_ADDR in bsp_ddr.h), so mirror 0 is usable from 0xC000_0040 */
#define DDR_EP_MIRROR0_BASE            (0xC0000000UL)
#define DDR_EP_MIRROR0_END             (0xDFFFFFFFUL)
#define DDR_EP_MIRROR1_BASE            (0xE0000000UL)
#define DDR_EP_MIRROR1_END             (0xFFFFFFFFUL)
#define DDR_EP_MIRROR_WINDOW_SIZE      (0x20000000UL)    /* 512 MB per ADXC DDR mirror window            */

#define DDR_EP_ECC_RESERVED_BYTES      (0x40UL)          /* First 64 B of DDR are prohibited (ECC seed)  */
#define DDR_EP_MIRROR0_USABLE_BASE     (DDR_EP_MIRROR0_BASE + DDR_EP_ECC_RESERVED_BYTES)

/* Physical DDR layout as seen by the Address Expander (values are physical, 34-bit) */
#define DDR_EP_PHYS_BASE               (0x200000000ULL)
#define DDR_EP_PHYS_SIZE               (0x40000000ULL)    /* 1 GB                                          */
#define DDR_EP_PHYS_LOW_HALF           (0x200000000ULL)   /* Mapped to mirror 0 by default                */
#define DDR_EP_PHYS_HIGH_HALF          (0x220000000ULL)   /* Mapped to mirror 1 by default                */

/* Test buffer size - one working block, reused for every region / pattern test. Kept small so the
 * "end of window" test addresses stay a full block below the 32-bit wrap */
#define DDR_EP_BLOCK_BYTES             (0x1000U)          /* 4 KiB                                         */

#define DDR_PATTERN_COUNT             (8U)               /* Number of entries in g_ddr_patterns[] below   */

#define DDR_EP_LFSR_SEED              (0xACE1ACE1UL)     /* Seed for the LFSR pattern generator          */

/* R_ADXC0->ADXCTL0 layout:
 *   ADXCTL0 = (DDRMIR1_field << 24) | (DDRMIR0_field << 16) | (PCIEMIR_field << 0)
 * The DDR-mirror field is the physical offset from the DDR base in 256 MB units - i.e. the same value
 * the Configurator emits as BSP_CFG_ADX0_DDR_MIRRORx_ADDRESS (0x02 for the default 0x2_2000_0000) */
#define DDR_ADXC0_ADXCTL0_DDRMIR0_MSK  (0xf0000UL)
#define DDR_ADXC0_ADXCTL0_DDRMIR1_MSK  (0xf000000UL)
#define DDR_ADXC_FIELD_UNIT            (0x10000000ULL)    /* 256 MB per DDR-mirror field step             */

/* Raw-pointer scratch offsets into the mirror window (bench / resident) are 2-3 / 4 MiB into the
 * window, well clear of each other and of the TOP/MID/END sample regions */

/* Throughput benchmark */
#define DDR_EP_BENCH_BYTES            (64U * 1024U)       /* Per-direction copy size                      */
#define DDR_EP_BENCH_ITERATIONS       (16U)               /* Averaged                                     */
#define DDR_EP_BENCH_SCRATCH_OFF      (0x00200000UL)     /* 2 MiB into the window (src)                  */
#define DDR_EP_BENCH_SCRATCH_OFF2     (0x00300000UL)     /* 3 MiB into the window (dst)                  */

/* DDR-resident buffer. A fixed reserved span in the cached mirror-0 window - no custom linker
 * section, so the project builds against the stock generated linker script */
#define DDR_EP_RESIDENT_OFFSET        (0x00400000UL)     /* 4 MiB into the window (clear of the above)   */
#define DDR_EP_RESIDENT_WORDS         (64U * 1024U)       /* 256 KiB / 4                                  */

/* OTFDE report. Fall back to a disabled window when the BSP does not define these */
#ifndef BSP_CFG_OTFDE_START_ADDRESS
 #define BSP_CFG_OTFDE_START_ADDRESS  (0)
#endif
#ifndef BSP_CFG_OTFDE_END_ADDRESS
 #define BSP_CFG_OTFDE_END_ADDRESS    (0)
#endif

/* Region table - every entry is one contiguous DDR_EP_BLOCK_BYTES span to write and verify. TOP / MID /
 * END are sampled in each mirror window so the full-window address decode is exercised without walking
 * all 512 MB byte by byte (which would take far too long to stream over the console) */
typedef enum e_ddr_region_id
{
    DDR_REGION_M0_TOP = 0,              /* 0xC000_0040 (just past the ECC reserve)       */
    DDR_REGION_M0_MID,                  /* 0xD000_0000 (mirror 0 midpoint)              */
    DDR_REGION_M0_END,                  /* 0xDFFF_F000 (last block of mirror 0)         */
    DDR_REGION_M1_TOP,                  /* 0xE000_0000                                  */
    DDR_REGION_M1_MID,                  /* 0xF000_0000                                  */
    DDR_REGION_M1_END,                  /* 0xFFFF_F000 (last block of mirror 1)         */
    DDR_REGION_COUNT
} ddr_region_id_t;

typedef struct st_ddr_region
{
    const char * name;
    uint32_t     addr;                  /* Start address of the block (CPU view of the mirror window) */
    uint32_t     size;                  /* Always DDR_EP_BLOCK_BYTES                                  */
    uint8_t      cached;                /* 1 = cached window (mirror 0), 0 = non-cached (mirror 1)    */
} ddr_region_t;

extern const ddr_region_t g_ddr_regions[DDR_REGION_COUNT];

/* Data patterns. The generated patterns (address-as-data, walking, LFSR) are produced in code;
 * this table carries the constant fills plus a tag naming which generator ddr_ep.c should use */
typedef enum e_ddr_pattern_kind
{
    DDR_PAT_CONST = 0,                  /* Fixed byte, value in .fill                                 */
    DDR_PAT_ADDR,                       /* Each 32-bit word = its own address                         */
    DDR_PAT_WALK1,                      /* Walking-1: 0x01, 0x02, 0x04, ..., 0x80, 0x01, ...          */
    DDR_PAT_WALK0,                      /* Walking-0: bitwise complement of walking-1                 */
    DDR_PAT_LFSR                        /* 32-bit maximal-length LFSR pseudo-random stream            */
} ddr_pattern_kind_t;

typedef struct st_ddr_pattern
{
    const char *       name;
    ddr_pattern_kind_t kind;
    uint8_t            fill;            /* Used only when kind == DDR_PAT_CONST                       */
} ddr_pattern_t;

extern const ddr_pattern_t g_ddr_patterns[DDR_PATTERN_COUNT];

#endif /* DDR_CONFIG_H_ */
