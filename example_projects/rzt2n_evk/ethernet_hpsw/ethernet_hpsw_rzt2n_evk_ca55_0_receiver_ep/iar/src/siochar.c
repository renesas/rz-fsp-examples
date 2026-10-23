/***********************************************************************************************************************
 * File Name    : siochar.c
 * Description  : Contains data structures and functions used in siochar.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "hal_data.h"
#include "sio_char.h"

#define SIO_RX_BUFFER_SIZE    (1024U)

/* The console talks to the SCI UART (g_uart0 / g_uart0_ctrl) directly through the FSP driver, one character at a time.
 * Completion is flagged by uart0_cb_stdio(), the callback already configured on g_uart0 */
static volatile uint32_t g_transfer_complete = 0;

static volatile uint32_t scan_write = 0U;
static volatile uint32_t scan_read  = 0U;
static uint8_t scan_buffer[SIO_RX_BUFFER_SIZE];

/*******************************************************************************
* Function Name: uart0_cb_stdio
* Description  : UART callback registered on g_uart0. Flags TX/RX completion for
*              : IoPutchar()/IoGetchar() to poll on.
* Arguments    : p_args -- callback event info
* Return Value : None
*******************************************************************************/
void uart0_cb_stdio (uart_callback_args_t * p_args)
{
    uint32_t next_write;

    if (NULL == p_args)
    {
        return;
    }

    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
            g_transfer_complete = 1U;
            break;

        case UART_EVENT_RX_CHAR:
            next_write = scan_write + 1U;
            if (next_write >= SIO_RX_BUFFER_SIZE)
            {
                next_write = 0U;
            }

            /* Drop newest data if the ring buffer is full. */
            if (next_write != scan_read)
            {
                scan_buffer[scan_write] = (uint8_t) p_args->data;
                scan_write = next_write;
            }
            break;

        case UART_EVENT_ERR_OVERFLOW:
        default:
            break;
    }
}

/*******************************************************************************
* Function Name: stdio_read
* Description  : Reads data until buffer full.
* Arguments    : pbyBuffer      Point to buffer.
* Arguments    : uiCount        Limit counter.
* Return Value : number of counter received.
*******************************************************************************/
int stdio_read (uint8_t * pbyBuffer, uint32_t uiCount)
{
    uint32_t count = 0U;
    uint8_t current_data;

    if ((NULL == pbyBuffer) || (0U == uiCount))
    {
        return -1;
    }

    while (count < uiCount)
    {
        while (1)
        {
            uint32_t write_pos = scan_write;
            if (write_pos != scan_read)
            {
                break;
            }
        }

        current_data = scan_buffer[scan_read];
        scan_read++;
        if (scan_read >= SIO_RX_BUFFER_SIZE)
        {
            scan_read = 0U;
        }

        pbyBuffer[count] = current_data;
        count++;

        if ('\r' == current_data)
        {
            /* Consume optional LF following CR (CRLF terminals). */
            uint32_t write_pos = scan_write;

            if (scan_read != write_pos)
            {
                if ('\n' == scan_buffer[scan_read])
                {
                    scan_read++;

                    if (scan_read >= SIO_RX_BUFFER_SIZE)
                    {
                        scan_read = 0U;
                    }
                }
            }

            break;
        }

        if ('\n' == current_data)
        {
            break;
        }
    }

    return (int) count;
}

/*******************************************************************************
* Function Name: app_check_data
* Description  : Reports, without blocking, whether a character has been received.
*              : The character stays buffered until IoGetchar() consumes it.
* Arguments    : none
* Return Value : 1 if a character is waiting, otherwise 0.
*******************************************************************************/
int app_check_data (void)
{
    uint32_t pos = scan_read;
    while (pos != scan_write)
    {
        if ((scan_buffer[pos] == '\r') ||
            (scan_buffer[pos] == '\n'))
        {
            return 1;
        }

        pos++;

        if (pos >= SIO_RX_BUFFER_SIZE)
        {
            pos = 0;
        }
    }
    return 0;
}

/*******************************************************************************
* Function Name: IoGetchar
* Description  : One character is received from the console UART, and its data is returned.
*              : This function keeps waiting until it can obtain the receiving data.
* Arguments    : none
* Return Value : Character to receive (Byte), or -1 if the UART driver rejects the request.
*******************************************************************************/
int32_t IoGetchar (void)
{
    uint8_t received_char;

    if (1 != stdio_read(&received_char, 1U))
    {
        return -1;
    }

    return (int32_t) received_char;
}

/*******************************************************************************
* Function Name: IoPutchar
* Description  : Character "buffer" is output to the console UART. This function keeps waiting
*              : until it becomes the transmission enabled state.
* Arguments    : int32_t buffer : character to output
* Return Value : None
*******************************************************************************/
void IoPutchar (int32_t buffer)
{
    uint8_t tx_char = (uint8_t) buffer;

    g_transfer_complete = 0U;
    if (FSP_SUCCESS != R_SCI_UART_Write(&g_uart0_ctrl, &tx_char, 1U))
    {
        return;
    }

    while (0U == g_transfer_complete)
    {
    }
}
