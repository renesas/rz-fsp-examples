/***********************************************************************************************************************
 * File Name    : stdio_uart0.c
 * Description  : Provides a simple SCI UART bridge for standard output.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "hal_data.h"
#if defined(__ICCARM__)
#include <LowLevelIOInterface.h>
#endif

int stdio_open(void);
int stdio_write(const uint8_t * p_buffer, uint32_t count);
void uart0_cb_stdio(uart_callback_args_t * p_args);

static uint32_t          g_stdio_initialized = 0U;
static volatile uint32_t g_wait_tx           = 0U;

int stdio_open(void)
{
    fsp_err_t err;

    if (0U != g_stdio_initialized)
    {
        return (int) FSP_SUCCESS;
    }

    err = g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);
    if (FSP_SUCCESS != err)
    {
        return (int) err;
    }

    err = g_uart0.p_api->callbackSet(g_uart0.p_ctrl, uart0_cb_stdio, NULL, NULL);
    if (FSP_SUCCESS != err)
    {
        (void) g_uart0.p_api->close(g_uart0.p_ctrl);
        return (int) err;
    }

    g_stdio_initialized = 1U;

    return (int) FSP_SUCCESS;
}

int stdio_write(const uint8_t * p_buffer, uint32_t count)
{
    fsp_err_t err;

    if ((NULL == p_buffer) || (0U == count))
    {
        return 0;
    }

    if (0U == g_stdio_initialized)
    {
        if ((int) FSP_SUCCESS != stdio_open())
        {
            return -1;
        }
    }

    g_wait_tx = 0U;

    err = g_uart0.p_api->write(g_uart0.p_ctrl, p_buffer, count);
    if (FSP_SUCCESS != err)
    {
        return -1;
    }

    while (0U == g_wait_tx)
    {
        __NOP();
    }

    return 0;
}

void uart0_cb_stdio(uart_callback_args_t * p_args)
{
    if ((NULL != p_args) && (0U != (p_args->event & UART_EVENT_TX_COMPLETE)))
    {
        g_wait_tx = 1U;
    }
}

#if defined(__ICCARM__)
/* EWARM migration: IAR DLIB standard I/O hooks. */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    if (NULL == buffer)
    {
        return 0U;
    }

    if ((_LLIO_STDOUT != handle) && (_LLIO_STDERR != handle))
    {
        return _LLIO_ERROR;
    }

    return (0 == stdio_write(buffer, (uint32_t) size)) ? size : _LLIO_ERROR;
}
#endif

