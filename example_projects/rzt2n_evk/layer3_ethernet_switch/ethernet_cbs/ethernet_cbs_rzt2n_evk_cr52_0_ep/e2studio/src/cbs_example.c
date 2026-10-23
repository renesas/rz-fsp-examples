/***********************************************************************************************************************
 * File Name    : cbs_example.c
 * Description  : Contains data structures and functions used in cbs_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <string.h>
#include "common_utils.h"
#include "cbs_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_ethernet_cbs_ep
 * @{
 **********************************************************************************************************************/

/* Private functions declarations */
static void    cbs_example_print_info(const cbs_info_t * p_info);
static void    cbs_example_print_menu(void);
static uint8_t cbs_example_read_menu_choice(void);
static void    cbs_example_try_read_choice(uint8_t current_choice, uint8_t * p_next_choice);

/***********************************************************************************************************************
 * @brief       This function prints whether CBS is enabled, the bandwidth applied per active queue, and the MAC
 *              table entry count.
 * @param[in]   p_info        Info as filled in by cbs_configure_mac_table()/cbs_configure_bandwidth().
 * @retval      None.
 **********************************************************************************************************************/
static void cbs_example_print_info(const cbs_info_t * p_info)
{
    APP_PRINT("[CBS] CBS: %s\r\n", p_info->cbs_enabled ? "ENABLED" : "disabled");
    for (uint8_t i = 0U; i < p_info->queue_count; i++)
    {
        APP_PRINT("[CBS]   Q%u: %u%% bandwidth\r\n", p_info->queues[i], p_info->band_width_pct[i]);
    }
    APP_PRINT("[CBS] MAC table entries: %u\r\n", p_info->mac_entry_count);
    APP_PRINT("[CBS] CBS active in hardware (queue mask): 0x%02x\r\n", (unsigned) p_info->oper_enable_mask);
}

/***********************************************************************************************************************
 * @brief       This function prints the mode selection menu.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void cbs_example_print_menu(void)
{
    APP_PRINT("\r\n=============CBS menu options===============\r\n"
              "  [1] Port-to-port,  CBS disabled -- baseline, no shaping\r\n"
              "  [2] Port-to-port,  CBS enabled  -- Q0 ONLY (isolated, Q1 excluded)\r\n"
              "  [3] Port-to-port,  CBS enabled  -- Q1 ONLY (isolated, Q0 excluded)\r\n"
              "  [4] CPU-generated, CBS disabled -- baseline (batched descriptors)\r\n"
              "  [5] CPU-generated, CBS enabled  -- Q0 ONLY (isolated)\r\n"
              "  [6] CPU-generated, CBS enabled  -- Q1 ONLY (isolated)\r\n"
              "\r\nEnter 1-6: ");
}

/***********************************************************************************************************************
 * @brief       This function blocks (polling APP_CHECK_DATA) until a valid '1'-'6' is typed; reprompts on
 *              anything else instead of accepting it.
 * @param[in]   None.
 * @retval      The selected menu number (1-6).
 **********************************************************************************************************************/
static uint8_t cbs_example_read_menu_choice(void)
{
    uint8_t rByte[2];

    /* APP_CHECK_DATA polls once and returns immediately -- the console transport has no
     * blocking read, so looping here is the only way to actually block until a key arrives */
    while (1)
    {
        if (APP_CHECK_DATA)
        {
            memset(rByte, 0, sizeof(rByte));
            APP_READ(rByte);

            if ((rByte[0] >= (uint8_t) '1') && (rByte[0] <= (uint8_t) '6') &&
                ((rByte[1] == (uint8_t) '\0') || (rByte[1] == (uint8_t) '\r') || (rByte[1] == (uint8_t) '\n')))
            {
                APP_PRINT("%c\r\n\r\n", rByte[0]);
                return (uint8_t) (rByte[0] - (uint8_t) '0');
            }
            APP_PRINT("\r\nInvalid input. Please enter 1-6: ");
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function checks for a new mode selection while one is already running. Typing the currently-
 *              active choice again just re-prints "already active" and keeps running; typing a different valid
 *              choice switches immediately -- no separate "stop" step needed.
 * @param[in]      current_choice The mode currently running (1-6).
 * @param[in,out]  p_next_choice  Left unchanged unless a *different* valid '1'-'6' byte was read, in which case
 *                                it's set to the new choice so the caller's run loop ends.
 * @retval      None.
 **********************************************************************************************************************/
static void cbs_example_try_read_choice(uint8_t current_choice, uint8_t * p_next_choice)
{
    uint8_t rByte[2];
    uint8_t requested;

    if (!APP_CHECK_DATA)
    {
        return;
    }

    memset(rByte, 0, sizeof(rByte));
    APP_READ(rByte);

    if ((rByte[0] < (uint8_t) '1') || (rByte[0] > (uint8_t) '6') ||
        ((rByte[1] != (uint8_t) '\0') && (rByte[1] != (uint8_t) '\r') && (rByte[1] != (uint8_t) '\n')))
    {
        APP_PRINT("\r\nInvalid input. Please enter 1-6: ");
        return;
    }

    requested = (uint8_t) (rByte[0] - (uint8_t) '0');
    if (requested == current_choice)
    {
        APP_PRINT("\r\n[CBS] Mode %u is already active.\r\n", (unsigned) current_choice);
        return;
    }

    APP_PRINT("%c\r\n\r\n", rByte[0]);
    *p_next_choice = requested;
}

/***********************************************************************************************************************
 * @brief       This function is the single entry point of the CBS example, called once from hal_entry(). Opens
 *              the switch once, then loops forever: run the selected traffic-source/CBS-state combination,
 *              continuously accepting a new mode number to switch. Never returns.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void cbs_example_run(void)
{
    fsp_err_t err;

    err = cbs_open();
    APP_ERR_TRAP(err);

    /* Wait for both PHY links to settle BEFORE programming any MAC table/CBS config --
     * R_LAYER3_SWITCH_Open() only *starts* auto-negotiation; it does not block until link is
     * actually up. Runs once -- link stays up across mode switches. */
    {
        const uint32_t poll_ms    = 100U;
        const uint32_t timeout_ms = 5000U;
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

        APP_PRINT("[CBS] ETH0 link: %s   ETH1 link: %s  (after %u ms)\r\n",
                  (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                  (FSP_SUCCESS == link1) ? "UP" : "DOWN",
                  (unsigned) waited_ms);
    }

    /* Backing storage for the isolated single-queue choices' local copy of g_cbs_rows_uc1
     * ([2]/[3], port-to-port) or g_cbs_rows_uc2 ([4]/[5]/[6], CPU-generated). Function-scope
     * (outside the loop) because it must stay alive for the whole run -- cbs_send_frames_burst()
     * (CPU-generated cases only) reads through isolated_rows[burst_dst_row] once at prepare
     * time, then keeps re-sending the same prepared descriptor every iteration of the run loop
     * below. Rebuilt fresh from the source table every time an isolated choice is (re-)selected,
     * so it always reflects the current g_cbs_rows_uc1/uc2 values. */
    static cbs_row_t isolated_rows[CBS_ROW_COUNT];

    cbs_example_print_menu();
    uint8_t choice = cbs_example_read_menu_choice();

    while (1)
    {
        const cbs_row_t * p_rows;
        bool               cbs_enabled;
        bool               cpu_tx;                /* also selects burst-mode submission -- every
                                                    * CPU-generated mode below uses
                                                    * cbs_send_frames_burst(), so there is no
                                                    * remaining cpu_tx-without-burst case */
        uint8_t            burst_dst_row = 1U;    /* which isolated_rows[] entry to send from
                                                    * when cpu_tx is true -- 0 = Q0, 1 = Q1 */
        const char       * p_desc;

        switch (choice)
        {
            case 1:
            {
                p_rows = g_cbs_rows_uc1; cbs_enabled = false; cpu_tx = false;
                p_desc = "port-to-port, CBS disabled";
                break;
            }
            case 2:
            {
                /* Port-to-port isolated Q0: copy g_cbs_rows_uc1, then zero out row[1]'s
                 * (Q1's) mac[] so cbs_mac_is_set() excludes it everywhere (MAC table, CBS
                 * bandwidth_list) -- only Q0's MAC table entry exists. Run the PC script
                 * with --queue 0 alongside this so the offered traffic matches what the
                 * board can actually classify. cpu_tx stays false -- traffic still comes
                 * from ETH1 externally. */
                memcpy(isolated_rows, g_cbs_rows_uc1, sizeof(isolated_rows));
                memset(isolated_rows[1].mac, 0, sizeof(isolated_rows[1].mac));
                p_rows = isolated_rows; cbs_enabled = true; cpu_tx = false;
                p_desc = "port-to-port, CBS enabled, Q0 ONLY (isolated) -- run PC script with --queue 0";
                break;
            }
            case 3:
            {
                /* Port-to-port isolated Q1: row[0] (Q0) zeroed out instead. Run the PC
                 * script with --queue 1. */
                memcpy(isolated_rows, g_cbs_rows_uc1, sizeof(isolated_rows));
                memset(isolated_rows[0].mac, 0, sizeof(isolated_rows[0].mac));
                p_rows = isolated_rows; cbs_enabled = true; cpu_tx = false;
                p_desc = "port-to-port, CBS enabled, Q1 ONLY (isolated) -- run PC script with --queue 1";
                break;
            }
            case 4:
            {
                /* CPU-generated disabled baseline -- uses the same batched-descriptor burst
                 * submission as [5]/[6], not a plain send loop, so this is a fair
                 * same-mechanism reference point for them. Targets Q1's MAC only: if the raw,
                 * unshaped path clears Q1's cap with margin, it necessarily clears Q0's
                 * smaller cap too, so one queue is enough to prove headroom for both. */
                memcpy(isolated_rows, g_cbs_rows_uc2, sizeof(isolated_rows));
                p_rows = isolated_rows; cbs_enabled = false; cpu_tx = true;
                burst_dst_row = 1U;
                p_desc = "CPU-generated, CBS disabled (batched descriptors, baseline)";
                break;
            }
            case 5:
            {
                /* Isolated Q0: copy g_cbs_rows_uc2, then zero out row[1]'s (Q1's) mac[] so
                 * cbs_mac_is_set() excludes it everywhere (MAC table, CBS bandwidth_list,
                 * and the TX loop). Burst-mode submission (see cbs_send_frames_burst()) gives
                 * more margin above Q0's cap than one-descriptor-per-Start submission would,
                 * with no downside to always using it here. */
                memcpy(isolated_rows, g_cbs_rows_uc2, sizeof(isolated_rows));
                memset(isolated_rows[1].mac, 0, sizeof(isolated_rows[1].mac));
                p_rows = isolated_rows; cbs_enabled = true; cpu_tx = true;
                burst_dst_row = 0U;
                p_desc = "CPU-generated, CBS enabled, Q0 ONLY (isolated, burst-mode offered load)";
                break;
            }
            default:    /* 6 */
            {
                /* Isolated Q1: same idea, row[0] (Q0) zeroed out instead. Burst-mode's wider
                 * offered-load margin doesn't change the cap value itself, since it's a fixed
                 * percent of the configured link speed, not of offered load. */
                memcpy(isolated_rows, g_cbs_rows_uc2, sizeof(isolated_rows));
                memset(isolated_rows[0].mac, 0, sizeof(isolated_rows[0].mac));
                p_rows = isolated_rows; cbs_enabled = true; cpu_tx = true;
                burst_dst_row = 1U;
                p_desc = "CPU-generated, CBS enabled, Q1 ONLY (isolated, burst-mode offered load)";
                break;
            }
        }

        APP_PRINT("[CBS] Starting: %s\r\n", p_desc);

        /* Drop any MAC entries left over from a previous selection -- the driver has no
         * per-entry delete API, so entries from a different mode would otherwise linger
         * alongside the new ones. */
        err = cbs_clear_mac_table();
        APP_ERR_TRAP(err);

        cbs_info_t info = {0};

        err = cbs_configure_mac_table(p_rows, &info);
        APP_ERR_TRAP(err);

        err = cbs_configure_bandwidth(p_rows, cbs_enabled, &info);
        APP_ERR_TRAP(err);

        cbs_example_print_info(&info);

        if (cpu_tx)
        {
            err = cbs_tx_queue_create();
            APP_ERR_TRAP(err);

            err = cbs_tx_burst_prepare(isolated_rows[burst_dst_row].mac, isolated_rows[burst_dst_row].queue);
            APP_ERR_TRAP(err);

            APP_PRINT("[CBS] Batching descriptors for maximum offered load (burst mode).\r\n");
        }
        else
        {
            APP_PRINT("[CBS] Waiting for external traffic.\r\n");
        }
        APP_PRINT("      Type a different mode number (1-6) any time to switch; typing the same number again "
                  "just confirms it's active.\r\n");
        cbs_example_print_menu();

        /* End the prompt line so a later link warning starts on its own line */
        APP_PRINT("\r\n");

        /* Poll for a different mode number while this one keeps running (CPU-generated: keep
         * sending frames every cycle; port-to-port: just idle-wait) */
        uint8_t  next_choice   = choice;
        uint32_t poll_divider  = 0U;

        /* Re-check link status roughly once a second (an iteration count, not a fixed
         * wall-clock time, since CPU-generated modes run with no fixed cycle time) -- an
         * unplugged cable or a link drop mid-run otherwise goes unnoticed, since link is only
         * confirmed once at boot. Prints only when something is actually wrong. */
        uint32_t       link_check_counter = 0U;
        const uint32_t link_check_every   = cpu_tx ? 65536U : 1000U;

        while (next_choice == choice)
        {
            if (cpu_tx)
            {
                err = cbs_send_frames_burst();
                APP_ERR_TRAP(err);
            }
            else
            {
                R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
            }

            if (++link_check_counter >= link_check_every)
            {
                link_check_counter = 0U;

                fsp_err_t link0 = g_ether_phy0.p_api->linkStatusGet(g_ether_phy0.p_ctrl);
                fsp_err_t link1 = g_ether_phy1.p_api->linkStatusGet(g_ether_phy1.p_ctrl);
                if ((FSP_SUCCESS != link0) || (FSP_SUCCESS != link1))
                {
                    APP_PRINT("[CBS] WARNING: ETH0 link: %s   ETH1 link: %s -- frames cannot flow on a down link\r\n",
                              (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                              (FSP_SUCCESS == link1) ? "UP" : "DOWN");
                }
            }

            /* In CPU-generated modes, poll the console only every 256th cycle so it does not
             * compete with the send loop for CPU time. Port-to-port modes poll every cycle */
            if ((!cpu_tx) || (0U == (poll_divider++ & 0xFFU)))
            {
                cbs_example_try_read_choice(choice, &next_choice);
            }
        }

        choice = next_choice;
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_cbs_ep)
 **********************************************************************************************************************/
