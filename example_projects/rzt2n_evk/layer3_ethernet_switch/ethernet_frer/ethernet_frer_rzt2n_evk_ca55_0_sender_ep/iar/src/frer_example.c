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

/* Console flags set by uart0_cb_stdio() and polled by IoGetchar()/IoPutchar() in siochar.c */
uint32_t volatile g_transfer_complete = 0;
uint32_t volatile g_receive_complete  = 0;

static void frer_example_handle_error(fsp_err_t err, const char * p_msg);

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
 * @brief       This function is the single entry point of the FRER Sender example, called once from hal_entry().
 *              Opens the switch, waits for both PHY links, programs the one replicated L3 entry, then loops forever
 *              injecting the stream. Never returns.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void frer_example_run(void)
{
    fsp_err_t   err;
    frer_info_t info = {0};

    err = frer_open();
    frer_example_handle_error(err, "switch open");

    /* Wait for both PHY links to settle before programming the L3 table -- Open() only starts
     * auto-negotiation, it does not block until link is actually up, and both external ports
     * must be up before the frame is replicated onto them. The timeout is generous (30 s)
     * because bringing up two boards manually means each one's Resume can be pressed at a
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

    err = frer_configure_stream(&info);
    frer_example_handle_error(err, "configure stream");

    err = frer_tx_queue_create(&info);
    frer_example_handle_error(err, "tx queue create");

    APP_PRINT("[FRER] L3 entries programmed: %u   TX queue index: %u\r\n",
              (unsigned int) info.l3_entry_count, (unsigned int) info.tx_queue_index);
    APP_PRINT("[FRER] Replicating one frame every %u ms -> Port0 (dst ...0A) and Port1 (dst ...0B).\r\n",
              (unsigned int) FRER_SEND_PERIOD_MS);

    uint32_t sent_count = 0U;

    while (1)
    {
        err = frer_send_one();
        frer_example_handle_error(err, "send frame");

        sent_count++;
        if (0U == (sent_count % FRER_RATE_REPORT_EVERY))
        {
            uint32_t       rate_fps = 1000U / FRER_SEND_PERIOD_MS;
            frer_tx_diag_t tx_diag;

            APP_PRINT("[FRER] Sent %u frames (~%u frames/s injection rate).\r\n",
                      (unsigned int) sent_count, (unsigned int) rate_fps);

            /* "Sent" above only counts calls that returned FSP_SUCCESS, which includes silent
             * skips -- this line is what actually proves whether hardware ever transmitted
             * anything. See frer_get_tx_diag(). */
            frer_get_tx_diag(&tx_diag);
            APP_PRINT("[FRER] tx diag: submitted=%u  skipped_busy=%u  skipped_overflow=%u\r\n",
                      (unsigned int) tx_diag.submitted,
                      (unsigned int) tx_diag.skipped_busy,
                      (unsigned int) tx_diag.skipped_overflow);

            /* Link is only confirmed once at boot -- an unplugged cable or a link drop mid-run
             * otherwise goes unnoticed, since a stalled TX queue looks identical on the console
             * to one that is merely busy with the previous frame (both show up as
             * skipped_busy climbing). Re-poll here and only print when something is actually
             * wrong, so this stays silent during normal operation. */
            fsp_err_t link0 = g_ether_phy0.p_api->linkStatusGet(g_ether_phy0.p_ctrl);
            fsp_err_t link1 = g_ether_phy1.p_api->linkStatusGet(g_ether_phy1.p_ctrl);
            if ((FSP_SUCCESS != link0) || (FSP_SUCCESS != link1))
            {
                APP_PRINT("[FRER] WARNING: ETH0 link: %s   ETH1 link: %s -- frames cannot flow on a down link\r\n",
                          (FSP_SUCCESS == link0) ? "UP" : "DOWN",
                          (FSP_SUCCESS == link1) ? "UP" : "DOWN");
            }
        }

        R_BSP_SoftwareDelay(FRER_SEND_PERIOD_MS, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

/***********************************************************************************************************************
 * @brief       This function is the UART callback. It flags TX/RX completion for the console.
 * @param[in]   p_args  Pointer to argument information.
 * @retval      None.
 **********************************************************************************************************************/
void uart0_cb_stdio (uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1;
            break;
        }

        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1;
            break;
        }

        default:
        {
            break;
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_ethernet_frer_ep)
 **********************************************************************************************************************/
