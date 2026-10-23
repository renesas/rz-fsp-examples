/***********************************************************************************************************************
 * File Name    : ddr_ep.c
 * Description  : Contains data structures and functions used in ddr_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <string.h>
#include "ddr_ep.h"

/***********************************************************************************************************************
 * @addtogroup bsp_ddr_ep
 * @{
 **********************************************************************************************************************/

/* CPU core clock used to convert PMU cycle counts to microseconds when SystemCoreClock reads back as 0 */
#ifndef DDR_EP_CPU_HZ_FALLBACK
 #define DDR_EP_CPU_HZ_FALLBACK    (400000000UL)
#endif

/* Private functions declarations */
static void      ddr_ep_cyc_reset(void);
static uint32_t  ddr_ep_cyc_read(void);
static uint32_t  ddr_ep_cpu_hz(void);
static uint32_t  ddr_ep_lfsr_next(uint32_t state);
static uint8_t   ddr_ep_pattern_byte(ddr_pattern_kind_t kind, uint32_t base_addr, uint32_t off, uint32_t * p_lfsr);
static void      ddr_ep_run_pattern(const ddr_region_t * p_region, ddr_pattern_kind_t kind, uint8_t fill,
                                    ddr_result_t * p_res);
static uint64_t  ddr_ep_field_to_phys(uint32_t field);

/* Source/destination scratch in SYSTEM_RAM for the two DDR <-> SYSTEM_RAM benchmark directions */
static uint8_t s_sysram_buf_a[DDR_EP_BENCH_BYTES];
static uint8_t s_sysram_buf_b[DDR_EP_BENCH_BYTES];

/***********************************************************************************************************************
 * @brief       This function enables and resets the CPU PMU cycle counter (PMCCNTR). It is used instead of the ARM
 *              generic timer because the BSP leaves CNTFRQ at 0 on this core.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_ep_cyc_reset (void)
{
#if defined(__aarch64__)
    /* PMCR_EL0 bit 0 = E (enable), bit 2 = C (reset cycle counter), bit 3 = D (0 = count every
     * cycle, not every 64th); PMCNTENSET_EL0 bit 31 = C (enable PMCCNTR_EL0) */
    uint64_t pmcr;
    __asm volatile ("mrs %0, pmcr_el0" : "=r" (pmcr));
    pmcr |= (1ULL << 0) | (1ULL << 2);
    pmcr &= ~(1ULL << 3);
    __asm volatile ("msr pmcr_el0, %0" :: "r" (pmcr));
    __asm volatile ("msr pmcntenset_el0, %0" :: "r" ((uint64_t) 0x80000000UL));
#else
    /* CP15: PMCR (c9, c12, 0) bit 0 = enable, bit 2 = reset cycle counter, bit 3 = /64 divider;
     *       PMCNTENSET (c9, c12, 1) bit 31 = enable PMCCNTR */
    uint32_t pmcr;
    __asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r" (pmcr));
    pmcr |= (1U << 0) | (1U << 2);        /* E | C: enable all counters, reset the cycle counter */
    pmcr &= (uint32_t) ~(1U << 3);        /* D = 0: count every cycle, not every 64              */
    __asm volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r" (pmcr));
    __asm volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r" (0x80000000U));
#endif
    __ISB();
}

/***********************************************************************************************************************
 * @brief       This function reads the CPU PMU cycle counter (PMCCNTR, CP15 c9, c13, 0).
 * @param[in]   None.
 * @retval      The current 32-bit cycle count.
 **********************************************************************************************************************/
static uint32_t ddr_ep_cyc_read (void)
{
#if defined(__aarch64__)
    uint64_t c;
    __asm volatile ("mrs %0, pmccntr_el0" : "=r" (c));
    return (uint32_t) c;
#else
    uint32_t c;
    __asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r" (c));
    return c;
#endif
}

/***********************************************************************************************************************
 * @brief       This function returns the effective CPU core clock in Hz, falling back to a fixed value when
 *              SystemCoreClock reads back as 0.
 * @param[in]   None.
 * @retval      The CPU core clock in Hz.
 **********************************************************************************************************************/
static uint32_t ddr_ep_cpu_hz (void)
{
    uint32_t hz = SystemCoreClock;
    if (0U == hz)
    {
        hz = DDR_EP_CPU_HZ_FALLBACK;
    }
    return hz;
}

/***********************************************************************************************************************
 * @brief       This function advances a 32-bit maximal-length Galois LFSR by one step (taps 32, 22, 2, 1).
 * @param[in]   state         Current LFSR state.
 * @retval      The next LFSR state.
 **********************************************************************************************************************/
static uint32_t ddr_ep_lfsr_next (uint32_t state)
{
    uint32_t shifted_out = state & 1U;
    state >>= 1;
    if (0U != shifted_out)
    {
        state ^= 0x80200003UL;
    }
    return state;
}

/***********************************************************************************************************************
 * @brief         This function returns the expected byte for a generated pattern at a given byte offset within a block.
 * @param[in]     kind        Which generated pattern (DDR_PAT_ADDR / DDR_PAT_WALK1 / DDR_PAT_WALK0 / DDR_PAT_LFSR).
 * @param[in]     base_addr   Start address of the block being tested.
 * @param[in]     off         Byte offset into the block.
 * @param[in,out] p_lfsr      Running LFSR state, advanced once per 32-bit word for DDR_PAT_LFSR.
 * @retval        The expected byte value at that offset.
 **********************************************************************************************************************/
static uint8_t ddr_ep_pattern_byte (ddr_pattern_kind_t kind, uint32_t base_addr, uint32_t off, uint32_t * p_lfsr)
{
    switch (kind)
    {
        case DDR_PAT_ADDR:
        {
            uint32_t word = (base_addr + (off & ~3U));
            return (uint8_t) (word >> ((off & 3U) * 8U));
        }
        case DDR_PAT_WALK1:
        {
            return (uint8_t) (1U << (off & 7U));
        }
        case DDR_PAT_WALK0:
        {
            return (uint8_t) ~(1U << (off & 7U));
        }
        case DDR_PAT_LFSR:
        {
            if (0U == (off & 3U))
            {
                *p_lfsr = ddr_ep_lfsr_next(*p_lfsr);
            }
            return (uint8_t) (*p_lfsr >> ((off & 3U) * 8U));
        }
        default:
        {
            return 0U;
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function writes one block with a pattern, then reads it back and records any mismatches. For a
 *              cached region the block is cleaned and invalidated between the write and the read so the verify
 *              re-reads the DRAM cells rather than the L1 D-cache copy.
 * @param[in]   p_region      Region (address, size, cacheability) to test.
 * @param[in]   kind          Pattern kind.
 * @param[in]   fill          Constant fill byte (used only when kind == DDR_PAT_CONST).
 * @param[out]  p_res         Result: bytes tested, mismatch count, and the first failing address/value.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_ep_run_pattern (const ddr_region_t * p_region, ddr_pattern_kind_t kind, uint8_t fill,
                                ddr_result_t * p_res)
{
    volatile uint8_t * p    = (volatile uint8_t *) (uintptr_t) p_region->addr;
    uint32_t           size = p_region->size;
    uint32_t           lfsr = DDR_EP_LFSR_SEED;

    memset(p_res, 0, sizeof(*p_res));
    p_res->bytes_tested = size;

    for (uint32_t i = 0U; i < size; i++)
    {
        p[i] = (uint8_t) ((DDR_PAT_CONST == kind) ? fill
                                                  : ddr_ep_pattern_byte(kind, p_region->addr, i, &lfsr));
    }
    __DSB();

    if (0U != p_region->cached)
    {
        R_BSP_CacheCleanInvalidateRange((uintptr_t) p_region->addr, size);
        __DSB();
        __ISB();
    }

    lfsr = DDR_EP_LFSR_SEED;
    for (uint32_t i = 0U; i < size; i++)
    {
        uint8_t exp = (uint8_t) ((DDR_PAT_CONST == kind) ? fill
                                                        : ddr_ep_pattern_byte(kind, p_region->addr, i, &lfsr));
        uint8_t got = p[i];
        if (got != exp)
        {
            if (0U == p_res->error_count)
            {
                p_res->first_err_addr = p_region->addr + i;
                p_res->first_err_exp  = exp;
                p_res->first_err_got  = got;
            }
            p_res->error_count++;
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function converts an Address Expander DDR-mirror field value back to a physical DDR address.
 * @param[in]   field         4-bit DDR-mirror field value.
 * @retval      The corresponding physical DDR address.
 **********************************************************************************************************************/
static uint64_t ddr_ep_field_to_phys (uint32_t field)
{
    return (uint64_t) DDR_EP_PHYS_BASE + ((uint64_t) field * DDR_ADXC_FIELD_UNIT);
}

/***********************************************************************************************************************
 * @brief       This function fills a DDR / clock bring-up snapshot from the BSP configuration and the live Address
 *              Expander register.
 * @param[out]  p_info        Snapshot: DDR-init/PLL2 state, DDRCLK, the two mirror-window bases, the ECC reserve, and
 *                            the decoded Address Expander mapping.
 * @retval      None.
 **********************************************************************************************************************/
void ddr_ep_info_get (ddr_ep_info_t * p_info)
{
    memset(p_info, 0, sizeof(*p_info));

    p_info->ddr_init_enabled = (1 == BSP_CFG_DDR_INIT_ENABLE);

#if defined(BSP_CLOCKS_PLL2_NORMAL)
    p_info->pll2_running = (BSP_CFG_PLL2 == BSP_CLOCKS_PLL2_NORMAL);
#else
    p_info->pll2_running = true;
#endif

#if defined(BSP_CFG_DDRDIVSEL_DDRCLKSEL) && defined(BSP_CLOCKS_DDRDIVSEL_DDRCLKSEL_800_MHZ)
    p_info->ddrclk_hz = (BSP_CFG_DDRDIVSEL_DDRCLKSEL == BSP_CLOCKS_DDRDIVSEL_DDRCLKSEL_800_MHZ)
                        ? 800000000UL : 0UL;
#else
    p_info->ddrclk_hz = 0UL;
#endif

    p_info->mirror0_base       = (uint32_t) DDR_EP_MIRROR0_BASE;
    p_info->mirror1_base       = (uint32_t) DDR_EP_MIRROR1_BASE;
    p_info->ecc_reserved_bytes = (uint32_t) DDR_EP_ECC_RESERVED_BYTES;

    ddr_adxc_state_t st;
    ddr_ep_adxc_get(&st);
    p_info->adxctl0_raw  = st.adxctl0_raw;
    p_info->mirror0_phys = st.mirror0_phys;
    p_info->mirror1_phys = st.mirror1_phys;
}

/***********************************************************************************************************************
 * @brief       This function writes and verifies one block with a five-pattern sweep
 *              (0x00 / 0xFF / 0x55 / 0xAA / address-as-data).
 * @param[in]   p_region      Region to test.
 * @param[out]  p_res         Aggregated result across the five patterns.
 * @retval      FSP_SUCCESS   Always (mismatches are reported through p_res, not the return value).
 **********************************************************************************************************************/
fsp_err_t ddr_ep_region_sweep (const ddr_region_t * p_region, ddr_result_t * p_res)
{
    FSP_ASSERT(NULL != p_region);
    FSP_ASSERT(NULL != p_res);

    static const struct
    {
        ddr_pattern_kind_t kind;
        uint8_t            fill;
    } sweep[] =
    {
        { DDR_PAT_CONST, 0x00U },
        { DDR_PAT_CONST, 0xFFU },
        { DDR_PAT_CONST, 0x55U },
        { DDR_PAT_CONST, 0xAAU },
        { DDR_PAT_ADDR,  0x00U },
    };

    ddr_result_t acc;
    memset(&acc, 0, sizeof(acc));
    acc.bytes_tested = p_region->size * (uint32_t) (sizeof(sweep) / sizeof(sweep[0]));

    for (uint32_t s = 0U; s < (sizeof(sweep) / sizeof(sweep[0])); s++)
    {
        ddr_result_t one;
        ddr_ep_run_pattern(p_region, sweep[s].kind, sweep[s].fill, &one);
        if (one.error_count != 0U)
        {
            if (acc.error_count == 0U)
            {
                acc.first_err_addr = one.first_err_addr;
                acc.first_err_exp  = one.first_err_exp;
                acc.first_err_got  = one.first_err_got;
            }
            acc.error_count += one.error_count;
        }
    }

    *p_res = acc;
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function writes and verifies one block with a single named pattern from g_ddr_patterns[].
 * @param[in]   p_region      Region to test.
 * @param[in]   p_pat         Pattern to apply.
 * @param[out]  p_res         Result for this one pattern.
 * @retval      FSP_SUCCESS   Always (mismatches are reported through p_res).
 **********************************************************************************************************************/
fsp_err_t ddr_ep_pattern_test (const ddr_region_t * p_region, const ddr_pattern_t * p_pat, ddr_result_t * p_res)
{
    FSP_ASSERT(NULL != p_region);
    FSP_ASSERT(NULL != p_pat);
    FSP_ASSERT(NULL != p_res);

    ddr_ep_run_pattern(p_region, p_pat->kind, p_pat->fill, p_res);
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function runs an address-line / aliasing test over one block. It writes a unique 32-bit marker
 *              at offset 0 and at every power-of-two offset that fits in the block, then re-reads them all; a stuck,
 *              shorted or mis-decoded address line makes one marker overwrite another.
 * @param[in]   p_region      Region to test.
 * @param[out]  p_res         Result: markers tested and the first mismatch.
 * @retval      FSP_SUCCESS   Always (mismatches are reported through p_res).
 **********************************************************************************************************************/
fsp_err_t ddr_ep_addrline_test (const ddr_region_t * p_region, ddr_result_t * p_res)
{
    FSP_ASSERT(NULL != p_region);
    FSP_ASSERT(NULL != p_res);

    volatile uint32_t * base = (volatile uint32_t *) (uintptr_t) p_region->addr;
    uint32_t            size = p_region->size;

    memset(p_res, 0, sizeof(*p_res));

    uint32_t offs[24];
    uint32_t n = 0U;
    offs[n++] = 0U;
    for (uint32_t bit = 2U; ((1UL << bit) < size) && (n < (sizeof(offs) / sizeof(offs[0]))); bit++)
    {
        offs[n++] = (1UL << bit);
    }
    p_res->bytes_tested = n * 4U;

    for (uint32_t i = 0U; i < n; i++)
    {
        base[offs[i] / 4U] = 0xA5A50000UL | i;
    }
    __DSB();
    if (0U != p_region->cached)
    {
        R_BSP_CacheCleanInvalidateRange((uintptr_t) p_region->addr, size);
        __DSB();
        __ISB();
    }

    for (uint32_t i = 0U; i < n; i++)
    {
        uint32_t exp = 0xA5A50000UL | i;
        uint32_t got = base[offs[i] / 4U];
        if (got != exp)
        {
            if (0U == p_res->error_count)
            {
                p_res->first_err_addr = p_region->addr + offs[i];
                p_res->first_err_exp  = exp;
                p_res->first_err_got  = got;
            }
            p_res->error_count++;
        }
    }
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function snapshots the live Address Expander DDR-mirror configuration.
 * @param[out]  p_state       Register value plus the decoded DDRMIR0/DDRMIR1 fields and physical bases.
 * @retval      None.
 **********************************************************************************************************************/
void ddr_ep_adxc_get (ddr_adxc_state_t * p_state)
{
    uint32_t raw = R_ADXC0->ADXCTL0;

    p_state->adxctl0_raw     = raw;
    p_state->ddrmir0_field   = (raw & R_ADXC0_ADXCTL0_DDRMIR0_Msk) >> R_ADXC0_ADXCTL0_DDRMIR0_Pos;
    p_state->ddrmir1_field   = (raw & R_ADXC0_ADXCTL0_DDRMIR1_Msk) >> R_ADXC0_ADXCTL0_DDRMIR1_Pos;
    p_state->mirror0_phys    = ddr_ep_field_to_phys(p_state->ddrmir0_field);
    p_state->mirror1_phys    = ddr_ep_field_to_phys(p_state->ddrmir1_field);
    p_state->matches_request = true;
}

/***********************************************************************************************************************
 * @brief       This function times a repeated memcpy for one benchmark direction and reports the throughput. The
 *              source is seeded with an LFSR stream, the copy runs DDR_EP_BENCH_ITERATIONS times, and the last copy
 *              is verified. If the PMU cycle counter did not advance, elapsed and throughput are reported as 0.
 * @param[in]   which         Benchmark direction.
 * @param[out]  p_res         Result: bytes/iteration, iteration count, elapsed microseconds, MiB/s, verify flag.
 * @retval      FSP_SUCCESS   Always (a data-integrity failure is reported through p_res->data_ok).
 **********************************************************************************************************************/
fsp_err_t ddr_ep_bench (ddr_bench_id_t which, ddr_bench_result_t * p_res)
{
    FSP_ASSERT(NULL != p_res);
    FSP_ASSERT(which < DDR_BENCHMARK_COUNT);

    uint8_t * src;
    uint8_t * dst;

    switch (which)
    {
        case DDR_BENCH_DDR_CACHED_COPY:
            src = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR0_BASE + DDR_EP_BENCH_SCRATCH_OFF);
            dst = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR0_BASE + DDR_EP_BENCH_SCRATCH_OFF2);
            break;
        case DDR_BENCH_DDR_NONCACHED_COPY:
            src = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR1_BASE + DDR_EP_BENCH_SCRATCH_OFF);
            dst = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR1_BASE + DDR_EP_BENCH_SCRATCH_OFF2);
            break;
        case DDR_BENCH_DDR_TO_SYSRAM:
            src = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR0_BASE + DDR_EP_BENCH_SCRATCH_OFF);
            dst = s_sysram_buf_a;
            break;
        default:
            src = s_sysram_buf_b;
            dst = (uint8_t *) (uintptr_t) (DDR_EP_MIRROR0_BASE + DDR_EP_BENCH_SCRATCH_OFF2);
            break;
    }

    uint32_t lfsr = DDR_EP_LFSR_SEED;
    for (uint32_t i = 0U; i < DDR_EP_BENCH_BYTES; i++)
    {
        lfsr = ddr_ep_lfsr_next(lfsr);
        src[i] = (uint8_t) lfsr;
    }
    __DSB();

    ddr_ep_cyc_reset();
    uint32_t c0 = ddr_ep_cyc_read();
    for (uint32_t it = 0U; it < DDR_EP_BENCH_ITERATIONS; it++)
    {
        memcpy((void *) dst, (const void *) src, DDR_EP_BENCH_BYTES);
    }
    __DSB();
    uint32_t c1 = ddr_ep_cyc_read();

    bool ok = (0 == memcmp((const void *) dst, (const void *) src, DDR_EP_BENCH_BYTES));

    uint32_t cycles = c1 - c0;
    p_res->bytes_per_iter = DDR_EP_BENCH_BYTES;
    p_res->iterations     = DDR_EP_BENCH_ITERATIONS;
    p_res->data_ok        = ok;

    if (0U == cycles)
    {
        p_res->elapsed_us       = 0U;
        p_res->throughput_mibps = 0U;
    }
    else
    {
        uint32_t hz = ddr_ep_cpu_hz();

        /* Both figures come straight from cycles/hz in 64-bit math. Rounding cycles to whole
         * microseconds first (and flooring that to 1 us) before computing throughput from the
         * rounded value used to skew the result whenever one iteration took under 1 us */
        p_res->elapsed_us       = (uint32_t) (((uint64_t) cycles * 1000000ULL) / hz);
        p_res->throughput_mibps = (uint32_t) (((uint64_t) DDR_EP_BENCH_BYTES * DDR_EP_BENCH_ITERATIONS * hz) /
                                              ((uint64_t) cycles * 1024ULL * 1024ULL));
    }
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function fills, cache-flushes, reads back and checksums a reserved buffer in the cached DDR
 *              mirror-0 window, exercising DDR as ordinary runtime memory.
 * @param[out]  p_res         Result: buffer address/size, whether it is inside a DDR window, the XOR checksum, and
 *                            the verify flag.
 * @retval      FSP_SUCCESS   Always (a data-integrity failure is reported through p_res->data_ok).
 **********************************************************************************************************************/
fsp_err_t ddr_ep_resident_test (ddr_resident_result_t * p_res)
{
    FSP_ASSERT(NULL != p_res);
    memset(p_res, 0, sizeof(*p_res));

    uint32_t            addr  = (uint32_t) (DDR_EP_MIRROR0_BASE + DDR_EP_RESIDENT_OFFSET);
    volatile uint32_t * buf   = (volatile uint32_t *) (uintptr_t) addr;
    uint32_t            words = DDR_EP_RESIDENT_WORDS;

    p_res->buf_addr      = addr;
    p_res->words         = words;
    p_res->in_ddr_window = (addr >= (uint32_t) DDR_EP_MIRROR0_BASE) && (addr <= (uint32_t) DDR_EP_MIRROR0_END);

    for (uint32_t i = 0U; i < words; i++)
    {
        buf[i] = i * 2654435761U;              /* Knuth multiplicative hash */
    }
    __DSB();
    R_BSP_CacheCleanInvalidateRange((uintptr_t) addr, (uintptr_t) (words * 4U));
    __DSB();
    __ISB();

    uint32_t xorsum = 0U;
    bool     ok     = true;
    for (uint32_t i = 0U; i < words; i++)
    {
        uint32_t word = buf[i];
        xorsum ^= word;
        if (word != (i * 2654435761U))
        {
            ok = false;
        }
    }
    p_res->checksum = xorsum;
    p_res->data_ok  = ok;
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function reports OTFDE peripheral presence and the configured window. No decryption is performed.
 * @param[out]  p_info        Peripheral-present flag, support-enabled flag, and the configured window bounds.
 * @retval      None.
 **********************************************************************************************************************/
void ddr_ep_otfde_report (ddr_otfde_info_t * p_info)
{
    memset(p_info, 0, sizeof(*p_info));

#if defined(BSP_FEATURE_BSP_HAS_OTFDE_PERIPHERAL)
    p_info->peripheral_present = (0 != BSP_FEATURE_BSP_HAS_OTFDE_PERIPHERAL);
#else
    p_info->peripheral_present = false;
#endif
#if defined(BSP_CFG_OTFDE0_SUPPORT_ENABLE)
    p_info->support_enabled = (0 != BSP_CFG_OTFDE0_SUPPORT_ENABLE) || (0 != BSP_CFG_OTFDE1_SUPPORT_ENABLE);
#else
    p_info->support_enabled = false;
#endif
    p_info->window_start      = (uint64_t) BSP_CFG_OTFDE_START_ADDRESS;
    p_info->window_end        = (uint64_t) BSP_CFG_OTFDE_END_ADDRESS;
    p_info->window_configured = (p_info->window_end > p_info->window_start);
}

/***********************************************************************************************************************
 * @} (end addtogroup bsp_ddr_ep)
 **********************************************************************************************************************/
