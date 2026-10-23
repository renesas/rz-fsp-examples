/***********************************************************************************************************************
 * File Name    : sio_char.c
 * Description  : Contains data structures and functions used in sio_char.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include <stdint.h>
#include "hal_data.h"
#include "sio_char.h"

/* IAR does not support the GCC/e2studio-only stdio_uart0.c ring-buffer console implementation, so
 * the CA55 IAR build talks to the console UART directly through the FSP SCI UART driver instead,
 * one character at a time -- the same UART instance (g_uart0 / g_uart0_ctrl) the CA55 e2studio
 * build uses. TX/RX completion is signalled by uart0_cb_stdio() below, the function name the
 * Configurator's UART module "Callback" property already names (unchanged from the e2studio
 * build), so no configuration.xml change is needed -- only the function body differs per
 * toolchain. */
static volatile uint32_t g_transfer_complete = 0;
static volatile uint32_t g_receive_complete  = 0;

/*******************************************************************************
* Function Name: uart0_cb_stdio
* Description  : UART callback registered on g_uart0 (Configurator "Callback" property). Flags
*              : TX/RX completion for IoPutchar()/IoGetchar() below to poll on.
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
            /* Other events (e.g. RX_CHAR, ERR) are not used by this simple polling console */
        }
    }
}

/*******************************************************************************
* Function Name: IoGetchar
* Description  : One character is received from the console UART, and its data is returned.
*              : This function keeps waiting until it can obtain the receiving data.
* Arguments    : none
* Return Value : Character to receive (Byte).
*******************************************************************************/
int32_t IoGetchar (void)
{
    int32_t    data;
    uint8_t    data_char;

    if (FSP_SUCCESS != R_SCI_UART_Read(&g_uart0_ctrl, &data_char, TRANSFER_LENGTH))
    {
        return -1;
    }
    while (!g_receive_complete)
    {
    }
    g_receive_complete = 0;

    data = (int32_t) (data_char & 0xff);

    return data;
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
