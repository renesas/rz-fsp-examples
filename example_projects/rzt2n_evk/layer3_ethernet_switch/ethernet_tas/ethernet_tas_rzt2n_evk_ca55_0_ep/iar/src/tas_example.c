/***********************************************************************************************************************
 * File Name    : tas_example.c
 * Description  : Contains data structures and functions used in tas_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "tas_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_tas_ep
 * @{
 **********************************************************************************************************************/

/* Private functions declarations */
static void           tas_example_handle_error(fsp_err_t err, const char * p_msg);
static void           tas_example_u64_to_decimal(uint64_t value, char * p_buf, size_t buf_size);
static void           tas_example_print_schedule(const tas_info_t * p_info);
static void           tas_example_print_menu(void);
static tas_use_case_t tas_example_read_menu_choice(void);
static void           tas_example_try_read_choice(tas_use_case_t current_uc, tas_use_case_t * p_next_uc);
static void           tas_example_wait_for_next_cycle(uint64_t cycle_start_ns, uint32_t elapsed_ns);

/***********************************************************************************************************************
 * @brief       This function prints the failing call's message and error code, then traps execution.
 * @param[in]   err           The FSP error code to check.
 * @param[in]   p_msg         Message identifying the failing call, printed on error.
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_handle_error(fsp_err_t err, const char * p_msg)
{
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("[TAS] ERROR: %s (0x%x)\r\n", p_msg, (unsigned int) err);
        if (FSP_ERR_UNSUPPORTED == err)
        {
            APP_PRINT("[TAS] TAS is not enabled in the FSP Configurator "
                      "(LAYER3_SWITCH_CFG_TAS_ENABLE == 0)\r\n");
        }
#if (1 == BSP_LP64_SUPPORT)
        __asm volatile("brk 0x0\n");
#else
        __asm("BKPT #0\n");
#endif
    }
}

/***********************************************************************************************************************
 * @brief       This function converts a 64-bit value to a decimal string. The console's own %u/%lu only
 *              accept a 32-bit unsigned value, so a 64-bit value cannot be printed directly in decimal --
 *              this is printed via %s instead.
 * @param[in]   value                 The 64-bit value to convert.
 * @param[out]  p_buf                 Filled with the null-terminated decimal string.
 * @param[in]   buf_size              Size of p_buf, in bytes (21 is enough for any uint64_t value).
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_u64_to_decimal(uint64_t value, char * p_buf, size_t buf_size)
{
    char     digits[20];    /* UINT64_MAX = 18446744073709551615 -- 20 digits */
    uint32_t digit_count = 0U;

    if (0ULL == value)
    {
        digits[digit_count++] = '0';
    }
    else
    {
        while (value > 0ULL)
        {
            digits[digit_count++] = (char) ('0' + (value % 10ULL));
            value /= 10ULL;
        }
    }

    uint32_t i;
    for (i = 0U; (i < digit_count) && ((i + 1U) < buf_size); i++)
    {
        p_buf[i] = digits[digit_count - 1U - i];
    }
    p_buf[i] = '\0';
}

/***********************************************************************************************************************
 * @brief       This function prints the programmed cycle time/start, per-port/per-queue gate entry counts,
 *              and MAC table entry count.
 * @param[in]   p_info        Schedule info as filled in.
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_print_schedule(const tas_info_t * p_info)
{
    char cycle_start_str[21];
    tas_example_u64_to_decimal(p_info->cycle_start_ns, cycle_start_str, sizeof(cycle_start_str));

    APP_PRINT("[TAS] cycle_time=%u ns  cycle_start=%s ns  timer=%u\r\n",
              p_info->cycle_time_ns,
              cycle_start_str,
              p_info->timer_number);

    for (uint8_t i = 0U; i < p_info->port_count; i++)
    {
        uint8_t port = p_info->ports[i];
        APP_PRINT("[TAS] Port %u TAS enabled\r\n", port);
        for (uint8_t q = 0U; q < BSP_FEATURE_ESWM_ETHA_IPV_QUEUE_NUM; q++)
        {
            APP_PRINT("[TAS]   Q%u: %u entries\r\n", q, p_info->entries_per_gate[port][q]);
        }
    }
    APP_PRINT("[TAS] MAC table entries: %u\r\n", p_info->mac_entry_count);
}

/***********************************************************************************************************************
 * @brief       This function prints the use-case selection menu.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_print_menu(void)
{
    APP_PRINT("\r\n========================================\r\n"
              "  [1] Port-to-port    -- external traffic entering Port 1 is shaped on egress\r\n"
              "  [2] CPU-generated   -- the MPU itself injects the traffic via GWCA\r\n"
              "  [3] Combined        -- both traffic origins accepted into the same schedule\r\n"
              "\r\nEnter 1-3: ");
}

/***********************************************************************************************************************
 * @brief       This function blocks until a valid '1'/'2'/'3' is typed; reprompts on anything
 *              else instead of accepting it. Used once, for the very first selection only.
 * @param[in]   None.
 * @retval      The selected use case.
 **********************************************************************************************************************/
static tas_use_case_t tas_example_read_menu_choice(void)
{
    uint8_t rByte[2];

    /* APP_CHECK_DATA polls once and returns immediately -- the console has no blocking read,
     * so looping here is the only way to actually block until a key arrives */
    while (1)
    {
        if (APP_CHECK_DATA)
        {
            memset(rByte, 0, sizeof(rByte));
            APP_READ(rByte);

            if ((rByte[0] >= (uint8_t) '1') && (rByte[0] <= (uint8_t) '3') &&
                ((rByte[1] == (uint8_t) '\0') || (rByte[1] == (uint8_t) '\r') || (rByte[1] == (uint8_t) '\n')))
            {
                APP_PRINT("%c\r\n\r\n", rByte[0]);
                return (tas_use_case_t) (rByte[0] - (uint8_t) '0');
            }
            APP_PRINT("\r\nInvalid input. Please enter 1-3: ");
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function checks for a new use-case selection while one is already running.
 * @param[in]      current_uc The use case currently running (1-3).
 * @param[in,out]  p_next_uc  Left unchanged unless a *different* valid '1'-'3' byte was read, in which case
 *                            it's set to the new use case so the caller's run loop ends.
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_try_read_choice(tas_use_case_t current_uc, tas_use_case_t * p_next_uc)
{
    uint8_t rByte[2];
    uint8_t requested;

    if (!APP_CHECK_DATA)
    {
        return;
    }

    memset(rByte, 0, sizeof(rByte));
    APP_READ(rByte);

    if ((rByte[0] < (uint8_t) '1') || (rByte[0] > (uint8_t) '3') ||
        ((rByte[1] != (uint8_t) '\0') && (rByte[1] != (uint8_t) '\r') && (rByte[1] != (uint8_t) '\n')))
    {
        APP_PRINT("\r\nInvalid input. Please enter 1-3: ");
        return;
    }

    requested = (uint8_t) (rByte[0] - (uint8_t) '0');
    if (requested == (uint8_t) current_uc)
    {
        APP_PRINT("\r\n[TAS] Use case %u is already active.\r\n", (unsigned int) current_uc);
        return;
    }

    APP_PRINT("%c\r\n\r\n", rByte[0]);
    *p_next_uc = (tas_use_case_t) requested;
}

/***********************************************************************************************************************
 * @brief       This function sleeps out the remainder of one CPU-generated cycle, after tas_send_frames_staggered()
 *              already spent elapsed_ns of it pacing submissions, so each cycle totals TAS_CYCLE_TIME_NS. Uses a
 *              fixed R_BSP_SoftwareDelay, not a live-timer-anchored wait.
 * @param[in]   cycle_start_ns    Unused; kept in the signature for a possible future timer-anchored version.
 * @param[in]   elapsed_ns        Time this cycle already spent in tas_send_frames_staggered() (its p_elapsed_ns).
 *                                Pass 0 if the caller used tas_send_frames() instead.
 * @retval      None.
 **********************************************************************************************************************/
static void tas_example_wait_for_next_cycle(uint64_t cycle_start_ns, uint32_t elapsed_ns)
{
    (void) cycle_start_ns;

    uint32_t cycle_us = TAS_CYCLE_TIME_NS / 1000U;
    uint32_t elapsed_us = elapsed_ns / 1000U;
    uint32_t remaining_us = (elapsed_us < cycle_us) ? (cycle_us - elapsed_us) : 0U;

    R_BSP_SoftwareDelay(remaining_us, BSP_DELAY_UNITS_MICROSECONDS);
}

/***********************************************************************************************************************
 * @brief       This function is the single entry point of the TAS example, called once from hal_entry(). Opens the
 *              switch and starts the timer once, then loops forever accepting a use-case selection. Never returns.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void tas_example_run(void)
{
    fsp_err_t err;

    err = tas_open();
    tas_example_handle_error(err, "switch open");

    /* Wait for both PHY links to settle BEFORE programming any schedule/MAC table -- Open()
     * only starts auto-negotiation, it does not block until link is actually up, so without
     * this wait, the first use case selected could run instantly after Open(), 1-3s before the
     * PHYs actually finish negotiating. Runs once -- link stays up across use-case switches. */
    {
        const uint32_t poll_ms = 100U;
        const uint32_t timeout_ms = 5000U;
        uint32_t       waited_ms = 0U;
        fsp_err_t      link0;
        fsp_err_t      link1;

        do
        {
            link0 = g_ether_phy0.p_api->linkStatusGet(g_ether_phy0.p_ctrl);
            link1 = g_ether_phy1.p_api->linkStatusGet(g_ether_phy1.p_ctrl);
            if ((FSP_SUCCESS == link0) && (FSP_SUCCESS == link1))
            {
                break;
            }
            R_BSP_SoftwareDelay(poll_ms, BSP_DELAY_UNITS_MILLISECONDS);
            waited_ms += poll_ms;
        } while (waited_ms < timeout_ms);

        APP_PRINT("[TAS] ETH0 link: %s   ETH1 link: %s  (after %u ms)\r\n",
                  (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                  (FSP_SUCCESS == link1) ? "UP" : "DOWN",
                  (unsigned) waited_ms);
    }

    err = tas_timer_start();
    tas_example_handle_error(err, "gPTP timer start");

    tas_example_print_menu();
    tas_use_case_t uc = tas_example_read_menu_choice();

    while (1)
    {
        const tas_row_t * p_rows;
        const char       * p_uc_name;
        bool               cpu_tx;

        switch (uc)
        {
            case TAS_UC_PORT_TO_PORT:
            {
                p_rows    = g_tas_rows_uc1;
                p_uc_name = "port-to-port";
                cpu_tx    = false;
                break;
            }
            case TAS_UC_CPU_GENERATED:
            {
                p_rows    = g_tas_rows_uc2;
                p_uc_name = "CPU-generated";
                cpu_tx    = true;
                break;
            }
            default:    /* TAS_UC_COMBINED */
            {
                p_rows    = g_tas_rows_uc3;
                p_uc_name = "combined";
                cpu_tx    = true;
                break;
            }
        }

        APP_PRINT("[TAS] Starting use case %u: %s\r\n", (unsigned int) uc, p_uc_name);

        /* Drop any MAC entries left over from a previous selection before adding this one's */
        err = tas_clear_mac_table();
        tas_example_handle_error(err, "clear MAC table");

        tas_info_t info = {0};

        err = tas_configure_schedule(p_rows, &info);
        tas_example_handle_error(err, "TAS schedule");

        err = tas_configure_mac_table(p_rows, &info);
        tas_example_handle_error(err, "MAC table");

        err = tas_enable();
        tas_example_handle_error(err, "TAS enable");

        tas_example_print_schedule(&info);

        if (cpu_tx)
        {
            err = tas_tx_queue_create(p_rows);
            tas_example_handle_error(err, "tx queue create");

            APP_PRINT("[TAS] Injecting one frame per open queue every cycle.\r\n");
        }
        else
        {
            APP_PRINT("[TAS] Waiting for external traffic on Port 1.\r\n");
        }
        APP_PRINT("      Type a different use case number (1-3) any time to switch -- no need to\r\n"
                  "      stop first; type the same number again just to confirm it's active.\r\n");
        tas_example_print_menu();

        /* tas_example_print_menu()'s "Enter 1-3: " is meant to sit on the same line as the typed
         * digit for the very first, blocking selection (tas_example_read_menu_choice()) -- but
         * here the run loop below may print a periodic diagnostic line before the user types
         * anything, which would otherwise run on right after "Enter 1-3: " with no line break.
         * Close this line out now so any such diagnostic always starts on its own line. */
        APP_PRINT("\r\n");

        /* Poll for a different use case while this one keeps running */
        tas_use_case_t next_uc = uc;

        /* Print the ingress ground-truth counter roughly once/sec, regardless of mode, so the
         * console always confirms frames are actually reaching L3 classification -- not just at
         * mode start. cpu_tx iterates once per ~25 ms TAS cycle (40/s); the port-to-port path
         * iterates once per 1 ms delay (1000/s). */
        uint32_t       print_counter = 0U;
        const uint32_t print_every = cpu_tx ? 40U : 1000U;

        while (next_uc == uc)
        {
            if (cpu_tx)
            {
                uint32_t elapsed_ns = 0U;
                err = tas_send_frames_staggered(p_rows, &elapsed_ns);
                tas_example_handle_error(err, "send frames");

                tas_example_wait_for_next_cycle(info.cycle_start_ns, elapsed_ns);
            }
            else
            {
                R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
            }

            if (++print_counter >= print_every)
            {
                print_counter = 0U;

                /* Link is only confirmed once at boot -- an unplugged cable or a link drop
                 * mid-run otherwise goes unnoticed, since TAS's own skip-if-busy handling
                 * (tas_send_one_row()) treats a stalled queue the same as a brief gate-timing
                 * delay. Re-poll here and only print when something is actually wrong, so this
                 * stays silent during normal operation. */
                fsp_err_t link0 = g_ether_phy0.p_api->linkStatusGet(g_ether_phy0.p_ctrl);
                fsp_err_t link1 = g_ether_phy1.p_api->linkStatusGet(g_ether_phy1.p_ctrl);
                if ((FSP_SUCCESS != link0) || (FSP_SUCCESS != link1))
                {
                    APP_PRINT("[TAS] WARNING: ETH0 link: %s   ETH1 link: %s -- frames cannot flow on a down link\r\n",
                              (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                              (FSP_SUCCESS == link1) ? "UP" : "DOWN");
                }

                if (TAS_UC_COMBINED == uc)
                {
                    APP_PRINT("[TAS] ingress: Port %u=%u  Port %u=%u frames classified since last\r\n",
                              (unsigned int) TAS_SOURCE_PORT,
                              (unsigned int) tas_get_ingress_count((uint8_t) TAS_SOURCE_PORT),
                              (unsigned int) TAS_CPU_PORT,
                              (unsigned int) tas_get_ingress_count((uint8_t) TAS_CPU_PORT));
                }
                else
                {
                    uint8_t ingress_port = (uint8_t) (cpu_tx ? TAS_CPU_PORT : TAS_SOURCE_PORT);
                    APP_PRINT("[TAS] ingress: Port %u=%u frames classified since last\r\n",
                              (unsigned int) ingress_port, (unsigned int) tas_get_ingress_count(ingress_port));
                }
            }

            tas_example_try_read_choice(uc, &next_uc);
        }

        uc = next_uc;
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_tas_ep)
 **********************************************************************************************************************/
