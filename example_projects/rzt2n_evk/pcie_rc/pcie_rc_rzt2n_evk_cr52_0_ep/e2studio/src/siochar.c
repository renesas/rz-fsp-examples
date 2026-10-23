/***********************************************************************************************************************
 * File Name    : sio_char.c
 * Description  : SCI character input/output functions.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/*******************************************************************************
Includes   <System Includes> , "Project Includes"
*******************************************************************************/
#include <stdint.h>
#include "hal_data.h"
#include "sio_char.h"
#include "r_uart_api.h"
#include "r_sci_uart.h"
extern volatile uint32_t g_transfer_complete;
extern volatile uint32_t g_receive_complete;
extern sci_uart_instance_ctrl_t g_uart0_ctrl;
/*******************************************************************************
Typedef definitions
*******************************************************************************/
/*******************************************************************************
Macro definitions
*******************************************************************************/
/*******************************************************************************
Imported global variables and functions (from other files)
*******************************************************************************/
/*******************************************************************************
Exported global variables and functions (to be accessed by other files)
*******************************************************************************/
/*******************************************************************************
Private global variables and functions
*******************************************************************************/
/*******************************************************************************************************************//**
 * @brief       One character is received from SCI, and its data is returned. This function keeps waiting until it
 *              can obtain the receiving data.
 * @param[in]   None
 * @retval      Character received (byte), or -1 if the read request failed.
 **********************************************************************************************************************/
int32_t IoGetchar (void)
{
    int32_t    data;
    uint8_t    data_char;

    /* Read one character from SCI; return failure if the read request could not be issued. */
    if ((R_SCI_UART_Read(&g_uart0_ctrl, &data_char, TRANSFER_LENGTH)) != (FSP_SUCCESS))
    {
        return -1;
    }

    while (!g_receive_complete)
    {
        /* Wait until a character has been received. */
    }

    g_receive_complete = 0;

    /* Mask the received byte to a 32-bit character value. */
    data = (int32_t)(data_char & 0xff);

    return data;
}

/*******************************************************************************************************************//**
 * @brief       Character "buffer" is output to SCI. This function keeps waiting until it becomes the transmission
 *              enabled state.
 * @param[in]   buffer    Character to output
 * @retval      None
 **********************************************************************************************************************/
void IoPutchar (int32_t buffer)
{
    /* Transmit the character over SCI. */
    R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *) &buffer, TRANSFER_LENGTH);

    while (!g_transfer_complete)
    {
        /* Wait until the character has been transmitted. */
    }

    g_transfer_complete = 0;
}
/* End of File */
