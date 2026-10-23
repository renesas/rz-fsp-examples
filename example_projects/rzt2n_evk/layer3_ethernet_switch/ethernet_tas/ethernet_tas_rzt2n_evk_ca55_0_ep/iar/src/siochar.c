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
#include "hal_data.h"
#include "sio_char.h"

/* The console talks to the SCI UART (g_uart0 / g_uart0_ctrl) directly through the FSP driver, one character at a time.
 * Completion is flagged by uart0_cb_stdio(), the callback already configured on g_uart0 */
static volatile uint32_t g_transfer_complete = 0;
static volatile uint32_t g_receive_complete  = 0;

/* One-character receive that is armed but not yet consumed by IoGetchar() */
static volatile bool g_rx_pending = false;
static uint8_t       g_rx_char    = 0;

/*******************************************************************************
* Function Name: uart0_cb_stdio
* Description  : UART callback registered on g_uart0. Flags TX/RX completion for
*              : IoPutchar()/IoGetchar() to poll on.
* Arguments    : p_args -- callback event info
* Return Value : None
*******************************************************************************/
void uart0_cb_stdio (uart_callback_args_t * p_args)
{
    if (NULL != p_args)
    {
        if (UART_EVENT_RX_COMPLETE == p_args->event)
        {
            g_receive_complete = 1;
        }
        else if (UART_EVENT_TX_COMPLETE == p_args->event)
        {
            g_transfer_complete = 1;
        }
        else
        {
            /* Other events are not used by this polling console */
        }
    }
}

/*******************************************************************************
* Function Name: app_arm_receive
* Description  : Starts a one-character receive unless one is already armed.
* Arguments    : none
* Return Value : true if a receive is armed, false if the driver rejected it.
*******************************************************************************/
static bool app_arm_receive (void)
{
    if (!g_rx_pending)
    {
        if (FSP_SUCCESS != R_SCI_UART_Read(&g_uart0_ctrl, &g_rx_char, TRANSFER_LENGTH))
        {
            return false;
        }
        g_rx_pending = true;
    }

    return true;
}

/*******************************************************************************
* Function Name: app_check_data
* Description  : Reports, without blocking, whether a character has been received.
*              : The character stays buffered until IoGetchar() consumes it. A line
*              : terminator (CR or LF) is not data and is dropped here, so the Enter
*              : key, or the LF of a CR+LF pair, never reads as a stray input.
* Arguments    : none
* Return Value : 1 if a character is waiting, otherwise 0.
*******************************************************************************/
int app_check_data (void)
{
    while (app_arm_receive() && (0 != g_receive_complete))
    {
        if (('\r' != (char) g_rx_char) && ('\n' != (char) g_rx_char))
        {
            return 1;
        }

        g_receive_complete = 0;
        g_rx_pending       = false;
    }

    return 0;
}

/*******************************************************************************
* Function Name: app_read
* Description  : Reads one line from the console without echoing it. Blocks until a
*              : line terminator (CR or LF) is received. At most "size" characters are
*              : stored, the rest of the line is discarded, and no terminator is added.
* Arguments    : p_data -- destination buffer
*              : size   -- size of the destination buffer
* Return Value : Number of characters stored.
*******************************************************************************/
uint32_t app_read (uint8_t * p_data, size_t size)
{
    uint32_t count = 0;
    int32_t  data_char;

    while (true)
    {
        data_char = IoGetchar();
        if ((-1 == data_char) || ('\r' == (char) data_char) || ('\n' == (char) data_char))
        {
            break;
        }

        if ((uint32_t) size > count)
        {
            p_data[count] = (uint8_t) data_char;
            count++;
        }
    }

    return count;
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
    if (!app_arm_receive())
    {
        return -1;
    }
    while (!g_receive_complete)
    {
    }
    g_receive_complete = 0;
    g_rx_pending       = false;

    return (int32_t) (g_rx_char & 0xff);
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
    (void) R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *) &buffer, TRANSFER_LENGTH);
    while (!g_transfer_complete)
    {
    }
    g_transfer_complete = 0;
}
