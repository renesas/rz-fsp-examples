/***********************************************************************************************************************
 * File Name    : ddr_example.c
 * Description  : Contains data structures and functions used in ddr_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <string.h>
#include "common_utils.h"
#include "ddr_ep.h"

/***********************************************************************************************************************
 * @addtogroup bsp_ddr_ep
 * @{
 **********************************************************************************************************************/

#define DDR_MENU_MIN                    (1U)
#define DDR_MENU_MAX                    (6U)

/* Expected values for the mode-1 bring-up pass check (this board and Configurator state) */
#define DDR_EP_EXPECT_DDRCLK_HZ         (800000000UL)
#define DDR_EP_EXPECT_ADXCTL0           (0x02000000UL)

/* Delay after each mode of the "run all" sweep so the host console drains its buffer before the next
 * mode starts; a non-blocking console transport otherwise drops whatever does not fit */
#define DDR_RUN_ALL_PACING_MS           (40U)

/* Set by ddr_example_handle_error() when an FSP call in the running mode returns non-SUCCESS. Cleared
 * at the start of every mode by ddr_run_one() and folded into that mode's PASS / FAIL verdict */
static bool s_ddr_mode_fsp_error = false;

/* Private functions declarations */
static void    ddr_example_handle_error(fsp_err_t err, const char * p_msg);
static bool    ddr_example_verdict(uint8_t mode, const char * p_name, bool pass);
static void    ddr_example_print_menu(void);
static uint8_t ddr_example_read_choice(void);
static void    ddr_example_print_result(const char * p_tag, const ddr_result_t * p_res);
static bool    ddr_run_one(uint8_t choice);
static bool    ddr_do_config_report(void);
static void    ddr_do_otfde_report(ddr_otfde_info_t * p_info);
static bool    ddr_do_memory_integrity(void);
static bool    ddr_do_window_walk(uint8_t cached);
static bool    ddr_do_pattern_suite(void);
static bool    ddr_do_addrline(void);
static bool    ddr_do_adxc_remap(void);
static void    ddr_do_coherency(bool apply_fix, ddr_coherency_result_t * p_res);
static bool    ddr_do_coherency_pair(void);
static bool    ddr_do_system_memory(void);
static bool    ddr_do_memcpy_bench(void);
static bool    ddr_do_resident_buffer(void);

/***********************************************************************************************************************
 * @brief       This function is the single entry point of the DDR example, called from hal_entry() after the banner.
 *              It prints the menu, reads a mode number, runs that mode (or the whole 1..5 sweep for mode 6), prints
 *              the result and each mode's PASS / FAIL verdict, and loops forever. Never returns.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void ddr_example_run (void)
{
    ddr_example_print_menu();

    uint8_t choice = ddr_example_read_choice();
    while (1)
    {
        if (6U == choice)
        {
            uint8_t passed = 0U;
            for (uint8_t m = 1U; m <= 5U; m++)
            {
                APP_PRINT("\r\n------------------------------------------------------------\r\n");
                if (ddr_run_one(m))
                {
                    passed++;
                }
                R_BSP_SoftwareDelay(DDR_RUN_ALL_PACING_MS, BSP_DELAY_UNITS_MILLISECONDS);
            }
            APP_PRINT("\r\n[DDR] RESULT: %u/5 modes PASS\r\n", (unsigned) passed);
            ddr_example_verdict(6U, "Run all (1..5)", (5U == passed));
        }
        else
        {
            (void) ddr_run_one(choice);
        }

        ddr_example_print_menu();
        choice = ddr_example_read_choice();
    }
}

/***********************************************************************************************************************
 * @brief       This function dispatches a menu mode to its handler and returns the mode's PASS / FAIL verdict. It
 *              resets the per-mode FSP-error flag beforehand, so that flag only ever reflects errors from the
 *              handler run by this call.
 * @param[in]   choice        Menu mode number to run.
 * @retval      true          The mode met its pass criteria.
 * @retval      false         The mode failed at least one check, or choice had no matching handler.
 **********************************************************************************************************************/
static bool ddr_run_one (uint8_t choice)
{
    s_ddr_mode_fsp_error = false;

    switch (choice)
    {
        case 1:  return ddr_do_config_report();
        case 2:  return ddr_do_memory_integrity();
        case 3:  return ddr_do_adxc_remap();
        case 4:  return ddr_do_coherency_pair();
        case 5:  return ddr_do_system_memory();
        default: return false;
    }
}

/***********************************************************************************************************************
 * @brief       This function reads and prints the DDR and clock bring-up snapshot (bsp_ddr_init and PLL2 state,
 *              DDRCLK, the two mirror windows, the ECC reserve, the live Address Expander mapping) and the OTFDE
 *              presence report, then prints and returns a PASS / FAIL verdict.
 * @param[in]   None.
 * @retval      true          DDR and PLL2 up, DDRCLK and ADXCTL0 at expected values, mapping decoded, OTFDE present.
 * @retval      false         Any of the above was not as expected.
 **********************************************************************************************************************/
static bool ddr_do_config_report (void)
{
    ddr_ep_info_t info;
    ddr_ep_info_get(&info);

    APP_PRINT("\r\n[DDR] Bring-up / configuration\r\n");
    APP_PRINT("      bsp_ddr_init enabled : %s\r\n", info.ddr_init_enabled ? "YES (BSP_CFG_DDR_INIT_ENABLE=1)"
                                                                          : "NO  -- DDR NOT initialised!");
    APP_PRINT("      PLL2 running         : %s\r\n", info.pll2_running ? "YES (released from standby)"
                                                                      : "NO  -- required for DDR!");
    if (0U != info.ddrclk_hz)
    {
        APP_PRINT("      DDRCLK               : %u Hz\r\n", info.ddrclk_hz);
    }
    else
    {
        APP_PRINT("      DDRCLK               : (divider not the expected 800 MHz setting)\r\n");
    }
    APP_PRINT("      mirror0 window       : 0x%08X (cached / write-back)\r\n", info.mirror0_base);
    APP_PRINT("      mirror1 window       : 0x%08X (non-cached)\r\n", info.mirror1_base);
    APP_PRINT("      ECC reserved bytes   : %u  (first %u B of DDR are off-limits)\r\n",
              info.ecc_reserved_bytes, info.ecc_reserved_bytes);
    APP_PRINT("      R_ADXC0->ADXCTL0     : 0x%08X\r\n", info.adxctl0_raw);
    APP_PRINT("        mirror0 -> phys    : 0x%08X%08X\r\n",
              (uint32_t) (info.mirror0_phys >> 32), (uint32_t) info.mirror0_phys);
    APP_PRINT("        mirror1 -> phys    : 0x%08X%08X\r\n",
              (uint32_t) (info.mirror1_phys >> 32), (uint32_t) info.mirror1_phys);

    if (!info.ddr_init_enabled || !info.pll2_running)
    {
        APP_PRINT("      >> DDR and PLL2 clock are not running, please re-check FSP configuration.\r\n");
    }

    ddr_otfde_info_t otfde;
    ddr_do_otfde_report(&otfde);

    bool pass = info.ddr_init_enabled
                && info.pll2_running
                && (DDR_EP_EXPECT_DDRCLK_HZ == info.ddrclk_hz)
                && (DDR_EP_EXPECT_ADXCTL0 == info.adxctl0_raw)
                && (DDR_EP_PHYS_LOW_HALF == info.mirror0_phys)
                && (DDR_EP_PHYS_HIGH_HALF == info.mirror1_phys)
                && otfde.peripheral_present
                && !s_ddr_mode_fsp_error;

    return ddr_example_verdict(1U, "Bring-up & config info", pass);
}

/***********************************************************************************************************************
 * @brief       This function fills the caller's snapshot and prints OTFDE peripheral presence, the support-enabled
 *              flag, and the configured window. No decryption is performed.
 * @param[out]  p_info        Receives the OTFDE presence / window snapshot.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_do_otfde_report (ddr_otfde_info_t * p_info)
{
    ddr_ep_otfde_report(p_info);

    APP_PRINT("\r\n[DDR] OTFDE (On-The-Fly Decryption Engine) support information:\r\n");
    APP_PRINT("      peripheral present : %s\r\n", p_info->peripheral_present ? "YES" : "no");
    APP_PRINT("      support enabled    : %s\r\n", p_info->support_enabled
              ? "YES (BSP_CFG_OTFDE0/1_SUPPORT_ENABLE)" : "no (Configurator otfde0/1_support = disabled)");
    APP_PRINT("      configured window  : 0x%08X%08X .. 0x%08X%08X  (%s)\r\n",
              (uint32_t) (p_info->window_start >> 32), (uint32_t) p_info->window_start,
              (uint32_t) (p_info->window_end   >> 32), (uint32_t) p_info->window_end,
              p_info->window_configured ? "set" : "not set");
}

/***********************************************************************************************************************
 * @brief       This function write/verify-tests both mirror windows: a five-pattern walk of the TOP / MID / END
 *              blocks of each window, the eight-pattern data suite on both window tops, and an address-line /
 *              aliasing test on both window midpoints, then prints and returns a PASS / FAIL verdict.
 * @param[in]   None.
 * @retval      true          Every block, pattern and address-line check reported zero mismatching bytes.
 * @retval      false         At least one check reported a mismatch or an FSP error.
 **********************************************************************************************************************/
static bool ddr_do_memory_integrity (void)
{
    bool mirror0_walk_ok = ddr_do_window_walk(1U);     /* Mirror 0, cached     */
    bool mirror1_walk_ok = ddr_do_window_walk(0U);     /* Mirror 1, non-cached */
    bool pattern_suite_ok = ddr_do_pattern_suite();
    bool addrline_ok = ddr_do_addrline();

    bool pass = mirror0_walk_ok && mirror1_walk_ok && pattern_suite_ok && addrline_ok && !s_ddr_mode_fsp_error;
    return ddr_example_verdict(2U, "Memory integrity", pass);
}

/***********************************************************************************************************************
 * @brief       This function performs a five-pattern write/verify sweep of the TOP / MID / END blocks of one mirror
 *              window and prints a per-block and a window verdict.
 * @param[in]   cached        1 = the cached mirror-0 window; 0 = the non-cached mirror-1 window.
 * @retval      true          All three blocks verified with zero mismatching bytes.
 * @retval      false         At least one block reported a mismatch.
 **********************************************************************************************************************/
static bool ddr_do_window_walk (uint8_t cached)
{
    ddr_region_id_t ids[3];
    if (cached)
    {
        ids[0] = DDR_REGION_M0_TOP; ids[1] = DDR_REGION_M0_MID; ids[2] = DDR_REGION_M0_END;
    }
    else
    {
        ids[0] = DDR_REGION_M1_TOP; ids[1] = DDR_REGION_M1_MID; ids[2] = DDR_REGION_M1_END;
    }

    APP_PRINT("\r\n[DDR] Full-window walk -- %s window, patterns 00/FF/55/AA/addr, %u B per block\r\n",
              cached ? "mirror0 (CACHED, flushed between write & verify)" : "mirror1 (NON-CACHED)",
              (unsigned) DDR_EP_BLOCK_BYTES);

    uint32_t total_err = 0U;
    for (uint32_t i = 0U; i < 3U; i++)
    {
        const ddr_region_t * r = &g_ddr_regions[ids[i]];
        ddr_result_t res;
        fsp_err_t err = ddr_ep_region_sweep(r, &res);
        ddr_example_handle_error(err, "region sweep");

        APP_PRINT("      %-11s @ 0x%08X : ", r->name, r->addr);
        if (0U == res.error_count)
        {
            APP_PRINT("OK  (%u B x5 patterns)\r\n", (unsigned) r->size);
        }
        else
        {
            APP_PRINT("FAIL  %u mismatching B, first @ 0x%08X exp 0x%02X got 0x%02X\r\n",
                      res.error_count, res.first_err_addr, res.first_err_exp, res.first_err_got);
        }
        total_err += res.error_count;
    }
    APP_PRINT("      ---- %s ----\r\n", (0U == total_err) ? "WINDOW PASS" : "WINDOW FAIL");
    return (0U == total_err);
}

/***********************************************************************************************************************
 * @brief       This function applies all eight data patterns to the TOP block of each mirror window and prints a
 *              per-pattern verdict.
 * @param[in]   None.
 * @retval      true          All sixteen pattern checks reported zero mismatching bytes.
 * @retval      false         At least one pattern check reported a mismatch.
 **********************************************************************************************************************/
static bool ddr_do_pattern_suite (void)
{
    const ddr_region_id_t ids[2] = { DDR_REGION_M0_TOP, DDR_REGION_M1_TOP };

    APP_PRINT("\r\n[DDR] Data-pattern suite -- %u patterns, %u B per block\r\n",
              (unsigned) DDR_PATTERN_COUNT, (unsigned) DDR_EP_BLOCK_BYTES);

    bool ok = true;
    for (uint32_t g = 0U; g < 2U; g++)
    {
        const ddr_region_t * r = &g_ddr_regions[ids[g]];
        APP_PRINT("      %s @ 0x%08X:\r\n", r->name, r->addr);
        for (uint32_t p = 0U; p < DDR_PATTERN_COUNT; p++)
        {
            ddr_result_t res;
            fsp_err_t err = ddr_ep_pattern_test(r, &g_ddr_patterns[p], &res);
            ddr_example_handle_error(err, "pattern test");
            APP_PRINT("        %-19s : ", g_ddr_patterns[p].name);
            if (0U == res.error_count)
            {
                APP_PRINT("OK\r\n");
            }
            else
            {
                ok = false;
                APP_PRINT("FAIL %u B, first @ 0x%08X exp 0x%02X got 0x%02X\r\n",
                          res.error_count, res.first_err_addr, res.first_err_exp, res.first_err_got);
            }
        }
    }
    return ok;
}

/***********************************************************************************************************************
 * @brief       This function runs an address-line / aliasing test on the MID block of each mirror window.
 * @param[in]   None.
 * @retval      true          Both midpoint tests reported zero mismatching bytes.
 * @retval      false         At least one midpoint test reported a mismatch.
 **********************************************************************************************************************/
static bool ddr_do_addrline (void)
{
    const ddr_region_id_t ids[2] = { DDR_REGION_M0_MID, DDR_REGION_M1_MID };

    APP_PRINT("\r\n[DDR] Address-line / aliasing test (unique marker per power-of-two offset)\r\n");
    bool ok = true;
    for (uint32_t i = 0U; i < 2U; i++)
    {
        const ddr_region_t * r = &g_ddr_regions[ids[i]];
        ddr_result_t res;
        fsp_err_t err = ddr_ep_addrline_test(r, &res);
        ddr_example_handle_error(err, "addrline test");
        if (0U != res.error_count)
        {
            ok = false;
        }
        ddr_example_print_result(r->name, &res);
    }
    return ok;
}

/***********************************************************************************************************************
 * @brief       This function reprograms the Address Expander DDR mirrors through the alias / swap / restore steps,
 *              reads R_ADXC0->ADXCTL0 back after each, proves the alias step overlaps the same DRAM, restores the
 *              power-on mapping, and prints and returns a PASS / FAIL verdict.
 * @param[in]   None.
 * @retval      true          Every step read back the requested fields, the alias step hit the same DRAM, and the
 *                            power-on mapping was restored.
 * @retval      false         A step mismatched on read-back, the alias check failed, or the restore did not land.
 **********************************************************************************************************************/
static bool ddr_do_adxc_remap (void)
{
    bool      ok = true;
    fsp_err_t err;

    APP_PRINT("\r\n[DDR] Address Expander DDR mirror remap (R_ADXC0->ADXCTL0)\r\n");

    /* g_ddr_adxc_steps: step 0 = alias, step 1 = swap, step 2 = restore */
    for (uint32_t s = 0U; s < DDR_ADXC_STEP_COUNT; s++)
    {
        const ddr_adxc_step_t * step = &g_ddr_adxc_steps[s];
        ddr_adxc_state_t        st;
        err = ddr_ep_adxc_set(step->mirror0_phys, step->mirror1_phys, &st);
        ddr_example_handle_error(err, "adxc set");

        APP_PRINT("      step %u: %s\r\n", (unsigned) s, step->desc);
        APP_PRINT("        ADXCTL0 = 0x%08X  DDRMIR0=%u DDRMIR1=%u  "
                  "-> m0 phys 0x%08X%08X / m1 phys 0x%08X%08X  [%s]\r\n",
                  st.adxctl0_raw, st.ddrmir0_field, st.ddrmir1_field,
                  (uint32_t) (st.mirror0_phys >> 32), (uint32_t) st.mirror0_phys,
                  (uint32_t) (st.mirror1_phys >> 32), (uint32_t) st.mirror1_phys,
                  st.matches_request ? "readback OK" : "READBACK MISMATCH");

        /* Register readback is checked for every step */
        if (!st.matches_request)
        {
            ok = false;
        }

        /* Data check only for the alias step: it is the only step where mirror0 and mirror1
         * are requested to point at the same physical half, so "same cells" is only meaningful here */
        if (0U == s)
        {
            ddr_result_t vr;
            err = ddr_ep_alias_verify(&vr);
            ddr_example_handle_error(err, "alias verify");
            APP_PRINT("        alias check (write via mirror0, read via mirror1): %s\r\n",
                      (0U == vr.error_count) ? "SAME CELLS -- OK" : "DIFFERENT -- alias not working");
            if (0U != vr.error_count)
            {
                ok = false;
            }
        }
    }

    /* Always leave the Address Expander back at its power-on mapping before returning */
    ddr_adxc_state_t rst;
    err = ddr_ep_adxc_restore(&rst);
    ddr_example_handle_error(err, "adxc restore");
    APP_PRINT("      restored to power-on mapping (ADXCTL0 = 0x%08X)\r\n", rst.adxctl0_raw);
    if (DDR_EP_EXPECT_ADXCTL0 != rst.adxctl0_raw)
    {
        ok = false;
    }

    ok = ok && !s_ddr_mode_fsp_error;
    return ddr_example_verdict(3U, "Address Expander remap", ok);
}

/***********************************************************************************************************************
 * @brief       This function runs the cache-coherency test twice back to back -- once with no cache clean (the
 *              stale-data hazard) and once with a cache clean first -- then prints and returns a PASS /
 *              FAIL verdict. Pass is the flush-first pass becoming coherent; the no-flush pass's hazard / no-hazard
 *              result is informational only (a "no hazard seen" outcome is a valid finding on this MPU).
 * @param[in]   None.
 * @retval      true          The flush-first pass read back coherent.
 * @retval      false         The flush-first pass did not become coherent, or an FSP call failed.
 **********************************************************************************************************************/
static bool ddr_do_coherency_pair (void)
{
    ddr_coherency_result_t r_noflush;
    ddr_coherency_result_t r_fixed;

    ddr_do_coherency(false, &r_noflush);
    ddr_do_coherency(true, &r_fixed);

    bool pass = r_fixed.coherent_after_fix && !s_ddr_mode_fsp_error;
    return ddr_example_verdict(4U, "Cache / DMA coherency", pass);
}

/***********************************************************************************************************************
 * @brief       This function aliases both mirror windows onto the same DRAM, runs the cache-coherency test with or
 *              without a cache clean, prints the result, and restores the power-on Address Expander mapping.
 * @param[in]   apply_fix     false = no cache clean (shows the hazard); true = clean first (shows it coherent).
 * @param[out]  p_res         Receives the coherency test result.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_do_coherency (bool apply_fix, ddr_coherency_result_t * p_res)
{
    ddr_adxc_state_t st;
    fsp_err_t         err;

    APP_PRINT("\r\n[DDR] Cache-coherency demo -- %s\r\n",
              apply_fix ? "WITH cache clean before the non-cached read"
                        : "NO flush (expect the non-cached view to be STALE)");

    /* Point both mirrors at the same physical half so the cached and non-cached windows alias */
    err = ddr_ep_adxc_set(DDR_EP_PHYS_LOW_HALF, DDR_EP_PHYS_LOW_HALF, &st);
    ddr_example_handle_error(err, "adxc alias");
    APP_PRINT("      (mirrors aliased: ADXCTL0=0x%08X)\r\n", st.adxctl0_raw);

    /* Write via the cached mirror, then read the same physical bytes via the non-cached mirror */
    err = ddr_ep_coherency_test(apply_fix, p_res);
    ddr_example_handle_error(err, "coherency test");

    APP_PRINT("      cached view    : 0x%08X   non-cached view : 0x%08X\r\n",
              p_res->cached_view_addr, p_res->noncached_view_addr);
    APP_PRINT("      non-cached read differed from the cached write in %u / %u bytes\r\n",
              p_res->mismatch_bytes, (unsigned) DDR_EP_COHERENCY_BYTES);

    if (apply_fix)
    {
        APP_PRINT("      RESULT: %s\r\n", p_res->coherent_after_fix
                  ? "COHERENT after cache clean -- the flush pushed the dirty lines to DRAM (expected)"
                  : "still not coherent -- unexpected, investigate");
    }
    else
    {
        APP_PRINT("      RESULT: %s\r\n", p_res->hazard_observed
                  ? "STALE -- non-cached view missed the cached (un-flushed) write (expected hazard)"
                  : "no hazard seen -- this system returned coherent data even without a flush");
    }

    /* Undo the alias before returning, so the next mode starts from the power-on mapping */
    err = ddr_ep_adxc_restore(&st);
    ddr_example_handle_error(err, "adxc restore");
    APP_PRINT("      restored power-on ADXC mapping.\r\n");
}

/***********************************************************************************************************************
 * @brief       This function times a repeated memcpy for four DDR / SYSTEM_RAM directions, then fills / flushes /
 *              reads back / checksums a reserved 256 KiB DDR buffer, then prints and returns a PASS / FAIL verdict.
 * @param[in]   None.
 * @retval      true          Every copy verified, the timer advanced, cached beat non-cached, and the buffer
 *                            round-tripped inside the DDR window.
 * @retval      false         A copy corrupted, the timer stalled, cached did not beat non-cached, or the buffer failed.
 **********************************************************************************************************************/
static bool ddr_do_system_memory (void)
{
    bool bench_ok = ddr_do_memcpy_bench();
    bool resident_ok = ddr_do_resident_buffer();

    bool pass = bench_ok && resident_ok && !s_ddr_mode_fsp_error;
    return ddr_example_verdict(5U, "DDR as system memory", pass);
}

/***********************************************************************************************************************
 * @brief       This function executes four timed memcpy benchmarks between DDR and SYSTEM_RAM, prints measured
 *              throughput (MiB/s) and elapsed time (us) for each.
 * @param[in]   None.
 * @retval      true          Every copy verified, every timing advanced, and the cached copy was faster than the
 *                            non-cached copy (a ratio sanity check, not an absolute MiB/s threshold).
 * @retval      false         A copy corrupted, a timing read back zero, or cached did not beat non-cached.
 **********************************************************************************************************************/
static bool ddr_do_memcpy_bench (void)
{
    static const char * memory_directions[DDR_BENCHMARK_COUNT] =
    {
        "DDR cached   -> DDR cached  ",
        "DDR noncache -> DDR noncache",
        "DDR cached   -> SYSTEM_RAM  ",
        "SYSTEM_RAM   -> DDR cached  ",
    };
    uint32_t           mibps[DDR_BENCHMARK_COUNT];
    ddr_bench_result_t res;
    fsp_err_t          err;
    bool               ok = true;

    APP_PRINT("\r\n[DDR] memcpy throughput -- %u KiB x %u iterations, timed via PMU cycle counter\r\n",
              (unsigned) (DDR_EP_BENCH_BYTES / 1024U), (unsigned) DDR_EP_BENCH_ITERATIONS);

    /* Each direction copies into its own scratch buffer and is verified independently, so a failed
     * or corrupted copy in one direction has no effect on the copies that follow it */
    for (uint32_t b = 0U; b < DDR_BENCHMARK_COUNT; b++)
    {
        err = ddr_ep_bench((ddr_bench_id_t) b, &res);
        ddr_example_handle_error(err, "bench");
        APP_PRINT("      %s : %4u MiB/s  (%u us, data %s)\r\n",
                  memory_directions[b], res.throughput_mibps, res.elapsed_us, res.data_ok ? "OK" : "CORRUPT");
        mibps[b] = res.throughput_mibps;
        if (!res.data_ok || (0U == res.elapsed_us))
        {
            ok = false;
        }
    }

    /* Ratio sanity, not an absolute figure: the D-cache must make the cached copy faster than the
     * non-cached copy. The absolute MiB/s varies run to run */
    if (mibps[DDR_BENCH_DDR_CACHED_COPY] <= mibps[DDR_BENCH_DDR_NONCACHED_COPY])
    {
        ok = false;
    }
    return ok;
}

/***********************************************************************************************************************
 * @brief       This function fills, flushes, reads back and checksums a reserved 256 KiB buffer in the cached DDR
 *              mirror-0 window, and prints its address and verdict.
 * @param[in]   None.
 * @retval      true          The buffer was inside the DDR window and round-tripped correctly.
 * @retval      false         The buffer fell outside the window or the read-back mismatched.
 **********************************************************************************************************************/
static bool ddr_do_resident_buffer (void)
{
    APP_PRINT("\r\n[DDR] DDR-resident buffer (reserved span in the cached mirror0 window)\r\n");

    ddr_resident_result_t res;
    fsp_err_t err = ddr_ep_resident_test(&res);
    ddr_example_handle_error(err, "resident test");

    APP_PRINT("      buffer address   : 0x%08X\r\n", res.buf_addr);
    APP_PRINT("      size             : %u words (%u KiB)\r\n", res.words, (res.words * 4U) / 1024U);
    APP_PRINT("      inside DDR window: %s\r\n", res.in_ddr_window ? "YES" : "NO -- check DDR_EP_RESIDENT_OFFSET");
    APP_PRINT("      fill+flush+read  : %s  (xor checksum 0x%08X)\r\n",
              res.data_ok ? "OK" : "MISMATCH", res.checksum);

    return res.in_ddr_window && res.data_ok;
}

/***********************************************************************************************************************
 * @brief       This function prints the failing call's message and error code and flags the running mode as failed.
 *              It does not trap, so a "run all" sweep continues past the failing mode.
 * @param[in]   err           The FSP error code to check.
 * @param[in]   p_msg         Message identifying the failing call, printed on error.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_example_handle_error (fsp_err_t err, const char * p_msg)
{
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("[DDR] ERROR: %s (fsp_err 0x%x)\r\n", p_msg, (unsigned int) err);
        s_ddr_mode_fsp_error = true;
    }
}

/***********************************************************************************************************************
 * @brief       This function prints one PASS / FAIL verdict line for a menu mode and returns the verdict unchanged.
 * @param[in]   mode          Menu mode number to print.
 * @param[in]   p_name        Short mode name.
 * @param[in]   pass          true = the mode met its pass criteria.
 * @retval      pass          The value passed in, so a caller can print and return in one expression.
 **********************************************************************************************************************/
static bool ddr_example_verdict (uint8_t mode, const char * p_name, bool pass)
{
    APP_PRINT("[DDR] MODE %u %s -- %s\r\n", (unsigned) mode, pass ? "PASS" : "FAIL", p_name);
    return pass;
}

/***********************************************************************************************************************
 * @brief       This function prints a one-line OK/FAIL verdict for a write/verify result.
 * @param[in]   p_tag         Short label for the region under test.
 * @param[in]   p_res         Result to print.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_example_print_result (const char * p_tag, const ddr_result_t * p_res)
{
    if (0U == p_res->error_count)
    {
        APP_PRINT("      %-11s : OK  (%u B tested)\r\n", p_tag, p_res->bytes_tested);
    }
    else
    {
        APP_PRINT("      %-11s : FAIL  %u B, first @ 0x%08X exp 0x%08X got 0x%08X\r\n",
                  p_tag, p_res->error_count, p_res->first_err_addr, p_res->first_err_exp, p_res->first_err_got);
    }
}

/***********************************************************************************************************************
 * @brief       This function prints the mode selection menu.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void ddr_example_print_menu (void)
{
    APP_PRINT("\r\n========================= Menu options =========================\r\n"
              "  [1] Bring-up & config info  -- Read and show the DDR/clock bring-up state and ADXC mapping\r\n"
              "  [2] Memory integrity        -- Write and verify both mirror windows (TOP/MID/END), 8 data patterns, address lines\r\n"
              "  [3] Address Expander remap  -- Reprogram DDR mirrors (alias / swap / restore) and read back the register\r\n"
              "  [4] Cache / DMA coherency   -- Compare a non-cached read after a cached write, with and without a cache flush\r\n"
              "  [5] DDR as system memory    -- Measure memcpy throughput (4 directions) and exercise a 256 KiB resident buffer\r\n"
              "  [6] Run all                 -- Run modes 1-5 in sequence\r\n"
              "====================================================================\r\n"
              "Enter option: ");
}

/***********************************************************************************************************************
 * @brief       This function blocks until a valid menu number 1..6 is typed, echoes it, and returns it. Accepts
 *              exactly one digit -- a second real character right after it is rejected rather than silently
 *              dropped, and any further trailing bytes are drained so one input line equals one prompt. Anything
 *              else is rejected with an "Invalid input" reprompt.
 * @param[in]   None.
 * @retval      The selected menu number, 1..6.
 **********************************************************************************************************************/
static uint8_t ddr_example_read_choice (void)
{
    uint8_t rbyte[8];

    while (1)
    {
        if (APP_CHECK_DATA)
        {
            memset(rbyte, 0, sizeof(rbyte));
            unsigned n = APP_READ(rbyte);

            /* Drain any trailing bytes (multi-character paste, CR/LF) so one input line == one prompt */
            while (APP_CHECK_DATA)
            {
                uint8_t junk[8];
                (void) APP_READ(junk);
            }

            /* rbyte[1] must be nothing but a line terminator - if APP_READ already captured a second
             * real character (e.g. typing "11"), that character is otherwise silently discarded and
             * the input wrongly accepted as if only one digit had been typed */
            if ((n >= 1U) && (rbyte[0] >= (uint8_t) '0') && (rbyte[0] <= (uint8_t) '9')
                && ((rbyte[1] == (uint8_t) '\0') || (rbyte[1] == (uint8_t) '\r') || (rbyte[1] == (uint8_t) '\n')))
            {
                uint32_t val = (uint32_t) (rbyte[0] - (uint8_t) '0');
                if ((val >= DDR_MENU_MIN) && (val <= DDR_MENU_MAX))
                {
                    APP_PRINT("%u\r\n", val);
                    return (uint8_t) val;
                }
            }
            APP_PRINT("\r\nInvalid input. Enter 1-6: ");
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup bsp_ddr_ep)
 **********************************************************************************************************************/
