/***********************************************************************************************************************
 * File Name    : frer_example.c
 * Description  : Contains data structures and functions used in frer_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "frer_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_frer_ep
 * @{
 **********************************************************************************************************************/

#define FRER_RX_LIFETIME_CAP  (FRER_RX_DESC_NUM - 1U)  /* Max receptions per port before frer_restart() is needed */

static void frer_example_handle_error(fsp_err_t err, const char * p_msg);
static void frer_example_print_status(const frer_info_t * p_info);
static bool frer_example_check_console(bool * p_running, frer_info_t * p_info);

/***********************************************************************************************************************
 * @brief       This function prints the failing call's message and error code, then traps execution.
 * @param[in]   err           The FSP error code to check.
 * @param[in]   p_msg         Message identifying the failing call, printed on error.
 * @retval      None.
 **********************************************************************************************************************/
static void frer_example_handle_error(fsp_err_t err, const char * p_msg)
{
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("[FRER] ERROR: %s (0x%x)\r\n", p_msg, (unsigned int) err);
#if (1 == BSP_LP64_SUPPORT)
        __asm volatile("brk 0x0\n");
#else
        __asm("BKPT #0\n");
#endif
    }
}

/***********************************************************************************************************************
 * @brief       This function prints the L3 entry count and RX queue indices currently programmed.
 * @param[in]   p_info                Snapshot of the programmed entry count and per-port RX queue indices.
 * @retval      None.
 **********************************************************************************************************************/
static void frer_example_print_status(const frer_info_t * p_info)
{
    APP_PRINT("[FRER] L3 entries programmed: %u   RX queues: Port0=%u  Port1=%u\r\n",
              (unsigned int) p_info->l3_entry_count,
              (unsigned int) p_info->rx_queue_index[FRER_PORT0],
              (unsigned int) p_info->rx_queue_index[FRER_PORT1]);
}

/***********************************************************************************************************************
 * @brief       This function checks the UART console for a completed input line (non-blocking) and, if one arrived,
 *              toggles between RUNNING and STOPPED: any line while RUNNING stops the poll loop; any line while
 *              STOPPED fully restarts the FRER receive pipeline (frer_restart()) and resumes RUNNING with every
 *              counter back at zero. The line's content is ignored -- this is a plain two-state toggle, not a menu.
 *              Only the Receiver needs this: its RX queue has a whole-lifetime reception capacity (see
 *              FRER_RX_DESC_NUM's doc comment), while the Sender's TX queue has no equivalent cap and
 *              can simply run forever, so the Sender project has no equivalent stop/restart control.
 * @param[in,out] p_running           Current run state; updated in place.
 * @param[in,out] p_info              Re-filled by frer_restart() when a restart happens.
 * @retval      true                  A restart just completed -- caller should reset its own poll/notice state.
 * @retval      false                 Nothing happened, or the pipeline was merely stopped.
 **********************************************************************************************************************/
static bool frer_example_check_console(bool * p_running, frer_info_t * p_info)
{
    unsigned char line[BUF_SIZE] = {INITIAL_VALUE};

    if (!APP_CHECK_DATA)
    {
        return false;
    }

    APP_READ(line);

    if (*p_running)
    {
        *p_running = false;
        APP_PRINT("\r\n[FRER] STOPPED. Press Enter again to RESTART the receive pipeline from zero "
                  "(each port can receive at most %u frames per run).\r\n",
                  (unsigned int) FRER_RX_LIFETIME_CAP);
        return false;
    }

    fsp_err_t err;

    APP_PRINT("[FRER] Restarting...\r\n");
    err = frer_restart(p_info);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("[FRER] ERROR: restart failed (0x%x) -- still stopped, press Enter to retry.\r\n",
                  (unsigned int) err);
        return false;
    }

    frer_example_print_status(p_info);
    APP_PRINT("[FRER] RUNNING again, all counters reset to zero.\r\n");
    *p_running = true;
    return true;
}

/***********************************************************************************************************************
 * @brief       This function is the single entry point of the FRER Receiver example, called once from hal_entry().
 *              Opens the switch, creates both ports' RX queues, waits for both PHY links, programs the two recovery
 *              entries, then loops forever polling for received frames -- printing each one's R-TAG sequence number,
 *              a periodic per-port health summary, and reacting to console input to stop/restart the pipeline (see
 *              frer_example_check_console()). Never returns.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void frer_example_run(void)
{
    fsp_err_t   err;
    frer_info_t info = {0};

    err = frer_open();
    frer_example_handle_error(err, "switch open");

    err = frer_rx_queue_create(FRER_PORT0, &info);
    frer_example_handle_error(err, "rx queue create (Port0)");

    err = frer_rx_queue_create(FRER_PORT1, &info);
    frer_example_handle_error(err, "rx queue create (Port1)");

    /* Wait for both PHY links to settle before programming the L3 table -- Open() only starts
     * auto-negotiation, it does not block until link is actually up, and both ingress ports must
     * be up before the recovery entries can see either member stream. The timeout is generous
     * (30 s) because bringing up two boards manually means each one's Resume can be pressed at a
     * different time, and link only starts negotiating after Resume. */
    {
        const uint32_t poll_ms    = 100U;
        const uint32_t timeout_ms = 30000U;
        uint32_t       waited_ms  = 0U;
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

        APP_PRINT("[FRER] ETH0 link: %s   ETH1 link: %s  (after %u ms)\r\n",
                  (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                  (FSP_SUCCESS == link1) ? "UP" : "DOWN",
                  (unsigned int) waited_ms);
        APP_PRINT("[FRER] Port0 = ETH0 (J8A), Port1 = ETH1 (J8B).\r\n");
    }

    err = frer_configure_recovery(&info);
    frer_example_handle_error(err, "configure recovery");

    frer_example_print_status(&info);
    APP_PRINT("[FRER] Waiting for the companion Sender's replicated frame on ETH0 and ETH1.\r\n");
    APP_PRINT("[FRER] Press Enter at any time to STOP; press it again to fully RESTART.\r\n");

    uint32_t poll_count = 0U;
    bool     running    = true;
    bool     capped[FRER_PORT_COUNT] = {false, false};

    while (1)
    {
        if (frer_example_check_console(&running, &info))
        {
            /* A restart just gave both ports a fresh lifetime capacity -- let the one-time
             * per-port notice below fire again, and restart the periodic-print cadence. */
            poll_count      = 0U;
            capped[FRER_PORT0] = false;
            capped[FRER_PORT1] = false;
        }

        if (running)
        {
            uint16_t sequence;
            uint16_t length;

            if (frer_poll_receive(FRER_PORT0, &sequence, &length))
            {
                APP_PRINT("[FRER] port0 sequence number:%04x  length:%u\r\n", sequence, (unsigned int) length);
            }
            if (frer_poll_receive(FRER_PORT1, &sequence, &length))
            {
                APP_PRINT("[FRER] port1 sequence number:%04x  length:%u\r\n", sequence, (unsigned int) length);
            }

            poll_count++;
            if (0U == (poll_count % (1000U / FRER_POLL_PERIOD_MS)))
            {
                frer_rx_diag_t     diag0;
                frer_rx_diag_t     diag1;
                frer_hw_counters_t hw_counters;
                uint32_t           l3_port0;
                uint32_t           l3_port1;

                frer_get_rx_diag(FRER_PORT0, &diag0);
                frer_get_rx_diag(FRER_PORT1, &diag1);
                frer_get_hw_frer_counters(&hw_counters);
                frer_get_l3_classification(&l3_port0, &l3_port1);

                /* Printed in actual pipeline order -- ingress, then each hardware recovery
                 * stage, then what finally reached the CPU -- instead of a pile of unlabelled
                 * numbers, so a reader unfamiliar with this code can tell what each figure
                 * means and where in the pipeline it was measured. The blank lines around it
                 * separate this periodic summary from the per-frame "sequence number" lines
                 * printed above and below it. */
                APP_PRINT("\r\n");
                APP_PRINT("[FRER] ------------------------ Periodic pipeline summary (every ~1 s) ------------------------\r\n");
                APP_PRINT("[FRER] 1. Ingress, before any FRER filtering runs:\r\n");
                APP_PRINT("[FRER]      port0 = %u frame(s)      port1 = %u frame(s)\r\n",
                          (unsigned int) l3_port0, (unsigned int) l3_port1);
                APP_PRINT("[FRER] 2. Individual recovery (per-port stream integrity check):\r\n");
                APP_PRINT("[FRER]      port0: ok=%u  flagged-stuck=%u      port1: ok=%u  flagged-stuck=%u\r\n",
                          (unsigned int) hw_counters.port0_individual_passed,
                          (unsigned int) hw_counters.port0_individual_flagged,
                          (unsigned int) hw_counters.port1_individual_passed,
                          (unsigned int) hw_counters.port1_individual_flagged);
                APP_PRINT("[FRER] 3. Sequence recovery (shared, cross-port duplicate elimination):\r\n");
                APP_PRINT("[FRER]      accepted=%u      discarded-as-duplicate=%u\r\n",
                          (unsigned int) hw_counters.seqrec_passed, (unsigned int) hw_counters.seqrec_discarded);
                APP_PRINT("[FRER] 4. Delivered to the CPU (what the application actually receives):\r\n");
                APP_PRINT("[FRER]      port0: ok=%u  err=%u      port1: ok=%u  err=%u\r\n",
                          (unsigned int) diag0.frames_ok, (unsigned int) diag0.frames_error,
                          (unsigned int) diag1.frames_ok, (unsigned int) diag1.frames_error);

                if ((0U == diag1.frames_ok) && (l3_port1 > 0U))
                {
                    APP_PRINT("[FRER] VERDICT: elimination CONFIRMED -- port1 physically received "
                              "%u frame(s) at step 1, but 0 reached the CPU at step 4.\r\n",
                              (unsigned int) l3_port1);
                }
                else if (0U == l3_port1)
                {
                    APP_PRINT("[FRER] VERDICT: step 1 shows 0 frames on port1 so far -- nothing has "
                              "arrived on ETH1 to eliminate yet; \"port1 ok=0\" at step 4 is NOT "
                              "evidence of elimination by itself.\r\n");
                }

                /* One-time notice when a port has used up its whole-lifetime reception capacity
                 * (see FRER_RX_DESC_NUM's doc comment) -- expected, not an error; explains why
                 * that port's own "ok" count will stop climbing until a restart. */
                if (!capped[FRER_PORT0] && ((diag0.frames_ok + diag0.frames_error) >= FRER_RX_LIFETIME_CAP))
                {
                    capped[FRER_PORT0] = true;
                    APP_PRINT("[FRER] NOTE: port0 has reached its %u-frame lifetime capacity for this "
                              "run -- press Enter twice (STOP then RESTART) to receive on it again.\r\n",
                              (unsigned int) FRER_RX_LIFETIME_CAP);
                }
                if (!capped[FRER_PORT1] && ((diag1.frames_ok + diag1.frames_error) >= FRER_RX_LIFETIME_CAP))
                {
                    capped[FRER_PORT1] = true;
                    APP_PRINT("[FRER] NOTE: port1 has reached its %u-frame lifetime capacity for this "
                              "run -- press Enter twice (STOP then RESTART) to receive on it again.\r\n",
                              (unsigned int) FRER_RX_LIFETIME_CAP);
                }

                /* Link is only confirmed once at boot -- an unplugged cable or a link drop
                 * mid-run otherwise looks identical to "the companion Sender simply isn't
                 * sending" (both show step 1's ingress count staying flat). Re-poll here and
                 * only print when something is actually wrong, so this stays silent during
                 * normal operation. */
                fsp_err_t link0 = g_ether_phy0.p_api->linkStatusGet(g_ether_phy0.p_ctrl);
                fsp_err_t link1 = g_ether_phy1.p_api->linkStatusGet(g_ether_phy1.p_ctrl);
                if ((FSP_SUCCESS != link0) || (FSP_SUCCESS != link1))
                {
                    APP_PRINT("[FRER] WARNING: ETH0 link: %s   ETH1 link: %s -- frames cannot flow on a down link\r\n",
                              (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                              (FSP_SUCCESS == link1) ? "UP" : "DOWN");
                }
                APP_PRINT("[FRER] -----------------------------------------------------------------------------------------\r\n");
                APP_PRINT("\r\n");
            }
        }

        R_BSP_SoftwareDelay(FRER_POLL_PERIOD_MS, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_frer_ep)
 **********************************************************************************************************************/
