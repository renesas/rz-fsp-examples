/***********************************************************************************************************************
 * File Name    : console_uart.c
 * Description  : Provides fail-safe SCI UART output for the XSPI QSPI example.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <stdarg.h>
#include <stdio.h>

#include "common_utils.h"

#define CONSOLE_BUFFER_SIZE      (512U)
#define CONSOLE_TX_CHUNK_SIZE    (64U)
#define CONSOLE_TX_GUARD_MS      (20U)

static bool g_console_ready = false;
static char g_console_buffer[CONSOLE_BUFFER_SIZE];

/*
 * These variables are intentionally global so they can be inspected in the debugger
 * if the terminal remains blank.
 */
volatile fsp_err_t g_console_open_result       = FSP_SUCCESS;
volatile fsp_err_t g_console_last_write_result = FSP_SUCCESS;
volatile uint32_t  g_console_write_count        = 0U;

fsp_err_t console_open(void)
{
    fsp_err_t err;

    if (g_console_ready)
    {
        return FSP_SUCCESS;
    }

    err = g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);
    g_console_open_result = err;

    if (FSP_SUCCESS == err)
    {
        g_console_ready = true;
        return FSP_SUCCESS;
    }

    /*
     * If another part of the project has already opened g_uart0, reuse it instead
     * of disabling console output.
     */
    if (FSP_ERR_ALREADY_OPEN == err)
    {
        g_console_ready = true;
        return FSP_SUCCESS;
    }

    return err;
}

void console_printf(const char * p_format, ...)
{
    va_list args;
    int formatted_length;
    uint32_t total_length;
    uint32_t offset = 0U;

    if ((!g_console_ready) || (NULL == p_format))
    {
        return;
    }

    va_start(args, p_format);
    formatted_length = vsnprintf(g_console_buffer, sizeof(g_console_buffer), p_format, args);
    va_end(args);

    if (formatted_length <= 0)
    {
        return;
    }

    if ((uint32_t) formatted_length >= sizeof(g_console_buffer))
    {
        total_length = sizeof(g_console_buffer) - 1U;
    }
    else
    {
        total_length = (uint32_t) formatted_length;
    }

    while (offset < total_length)
    {
        uint32_t remaining = total_length - offset;
        uint32_t chunk_length = (remaining > CONSOLE_TX_CHUNK_SIZE) ?
                                CONSOLE_TX_CHUNK_SIZE : remaining;

        fsp_err_t err = g_uart0.p_api->write(g_uart0.p_ctrl,
                                              (uint8_t *) &g_console_buffer[offset],
                                              chunk_length);

        g_console_last_write_result = err;

        if (FSP_SUCCESS != err)
        {
            /*
             * Logging is auxiliary. Never block or alter the QSPI example if UART
             * cannot accept more data.
             */
            return;
        }

        g_console_write_count++;

        /*
         * R_SCI_UART_Write() is interrupt driven. The guard delay allows the current
         * chunk to leave the UART before the next write without relying on a user
         * TX_COMPLETE callback.
         */
        R_BSP_SoftwareDelay(CONSOLE_TX_GUARD_MS, BSP_DELAY_UNITS_MILLISECONDS);

        offset += chunk_length;
    }
}
