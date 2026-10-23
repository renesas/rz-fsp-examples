/***********************************************************************************************************************
 * File Name    : stdio_uart0.c
 * Description  : Provides a simple SCI UART bridge for standard input and output.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include <stdio.h>
#if defined(__ICCARM__)
#include <LowLevelIOInterface.h>
#endif
#include <string.h>

#include "bsp_api.h"
#include "hal_data.h"

int stdio_open(void);
void stdio_close(void);
int stdio_read(uint8_t * pbyBuffer, uint32_t uiCount);
int stdio_write(const uint8_t * pbyBuffer, uint32_t uiCount);
void uart0_cb_stdio(uart_callback_args_t * p_args);

void app_read(unsigned char * p_data);
int app_check_data(void);

static uint32_t          stdio_initialized = 0U;
static volatile uint32_t wait_tx           = 0U;
static volatile uint32_t scan_write        = 0U;
static uint32_t          scan_read         = 0U;
static uint8_t           scan_buffer[1024];

char g_buff[1024];

/*******************************************************************************************************************//**
 * @brief Opens the SCI UART instance used by stdio.
 *
 * @retval FSP_SUCCESS UART opened successfully or was already open.
 * @retval Others      Error returned by the UART driver.
 **********************************************************************************************************************/
int stdio_open(void)
{
    fsp_err_t err;

    if (0U != stdio_initialized)
    {
        return (int) FSP_SUCCESS;
    }

    scan_write = 0U;
    scan_read  = 0U;
    wait_tx    = 0U;
    memset(scan_buffer, 0, sizeof(scan_buffer));

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

    stdio_initialized = 1U;

    return (int) FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Closes the SCI UART instance used by stdio.
 *
 * @retval None
 **********************************************************************************************************************/
void stdio_close(void)
{
    if (0U != stdio_initialized)
    {
        (void) g_uart0.p_api->close(g_uart0.p_ctrl);
        stdio_initialized = 0U;
    }
}

/*******************************************************************************************************************//**
 * @brief Reads data received by the UART callback.
 *
 * @param[out] pbyBuffer Destination buffer.
 * @param[in]  uiCount   Maximum number of bytes to read.
 *
 * @return Number of bytes read, or -1 if UART initialization fails.
 **********************************************************************************************************************/
int stdio_read(uint8_t * pbyBuffer, uint32_t uiCount)
{
    uint32_t ret = 0U;

    if ((NULL == pbyBuffer) || (0U == uiCount))
    {
        return 0;
    }

    if (0U == stdio_initialized)
    {
        if ((int) FSP_SUCCESS != stdio_open())
        {
            return -1;
        }
    }

    while (ret < uiCount)
    {
        uint8_t current_data;

        while (scan_write == scan_read)
        {
            __NOP();
        }

        current_data = scan_buffer[scan_read];
        scan_read++;
        if (scan_read >= sizeof(scan_buffer))
        {
            scan_read = 0U;
        }

        *pbyBuffer++ = current_data;
        ret++;

        if (('\n' == current_data) || ('\r' == current_data))
        {
            break;
        }
    }

    return (int) ret;
}

/*******************************************************************************************************************//**
 * @brief Writes data through the SCI UART and waits for transmit completion.
 *
 * @param[in] pbyBuffer Source buffer.
 * @param[in] uiCount   Number of bytes to write.
 *
 * @retval 0  All bytes were written.
 * @retval -1 UART initialization or transmission failed.
 **********************************************************************************************************************/
int stdio_write(const uint8_t * pbyBuffer, uint32_t uiCount)
{
    fsp_err_t err;

    if ((NULL == pbyBuffer) || (0U == uiCount))
    {
        return 0;
    }

    if (0U == stdio_initialized)
    {
        if ((int) FSP_SUCCESS != stdio_open())
        {
            return -1;
        }
    }

    wait_tx = 0U;

    err = g_uart0.p_api->write(g_uart0.p_ctrl, pbyBuffer, uiCount);
    if (FSP_SUCCESS != err)
    {
        return -1;
    }

    while (0U == wait_tx)
    {
        __NOP();
    }

    /* syscalls.c expects the number of bytes not written. */
    return 0;
}

/*******************************************************************************************************************//**
 * @brief SCI UART callback used by stdio.
 *
 * @param[in] p_args UART callback arguments.
 *
 * @retval None
 **********************************************************************************************************************/
void uart0_cb_stdio(uart_callback_args_t * p_args)
{
    if (NULL == p_args)
    {
        return;
    }

    if (0U != (p_args->event & UART_EVENT_TX_COMPLETE))
    {
        wait_tx = 1U;
    }

    if (0U != (p_args->event & UART_EVENT_RX_CHAR))
    {
        scan_buffer[scan_write] = (uint8_t) p_args->data;

        scan_write++;
        if (scan_write >= sizeof(scan_buffer))
        {
            scan_write = 0U;
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Reads one formatted token from standard input.
 *
 * @param[out] p_data Destination buffer.
 *
 * @retval None
 **********************************************************************************************************************/
void app_read(unsigned char * p_data)
{
    if (NULL == p_data)
    {
        return;
    }

    if (0U == stdio_initialized)
    {
        if ((int) FSP_SUCCESS != stdio_open())
        {
            return;
        }
    }

    (void) scanf("%1023s", p_data);
}

/*******************************************************************************************************************//**
 * @brief Checks whether unread UART data is available.
 *
 * @retval 1 Data is available.
 * @retval 0 No data is available.
 **********************************************************************************************************************/
int app_check_data(void)
{
    if (0U == stdio_initialized)
    {
        if ((int) FSP_SUCCESS != stdio_open())
        {
            return 0;
        }
    }

    return (scan_write != scan_read) ? 1 : 0;
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
size_t __read(int handle, unsigned char * buffer, size_t size)
{
    if (0U == size)
    {
        return 0U;
    }

    if ((_LLIO_STDIN != handle) || (NULL == buffer))
    {
        return _LLIO_ERROR;
    }

    int const bytes_read = stdio_read(buffer, (uint32_t) size);

    return (bytes_read < 0) ? _LLIO_ERROR : (size_t) bytes_read;
}
#endif

