/***********************************************************************************************************************
 * File Name    : uart_ep.c
 * Description  : Contains data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "uart_ep.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * @addtogroup r_sci_uart_ep
 * @{
 **********************************************************************************************************************/
/*
 * Private global variables
 */
/* Transmit complete Flag */
static volatile uint32_t s_transfer_complete = RESET_VALUE;

/* Receive complete Flag */
static volatile uint32_t s_receive_complete = RESET_VALUE;

/***********************************************************************************************************************
 * @fn          uart_initialize
 * @brief       Initialize UART.
 * @param[in]   None
 * @retval      FSP_SUCCESS         Upon successful open and start of UART
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 **********************************************************************************************************************/
fsp_err_t uart_initialize(void)
{ 
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 */
    err = R_SCI_UART_Open (&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_SCI_UART_Open API failed \r\n");
        return err;
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: uart_initialize
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  @fn          uart_print_user_msg
 *  @brief       Print message to terminal
 *  @param[in]   p_msg
 *  @retval      FSP_SUCCESS                Upon success
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure
 *  @retval      Any Other Error code apart from FSP_SUCCESS,  Unsuccessful write operation
 **********************************************************************************************************************/
fsp_err_t uart_print_user_msg(char* p_msg)
{
    fsp_err_t err = FSP_SUCCESS;

    uint32_t text_length = strlen(p_msg);

    /* write message */
    err = R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *)p_msg, text_length);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_SCI_UART_Write API failed \r\n");
        return err;
    }

    while (!s_transfer_complete)
    {
        ;   /* wait transfer complete */
    }
    s_transfer_complete = 0;

    return err;
}
/***********************************************************************************************************************
 * End of function: uart_print_user_msg
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  @fn          uart_get_user_msg
 *  @brief       Read message from terminal
 *  @param[in]   p_msg
 *  @retval      FSP_SUCCESS                Upon success
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure
 *  @retval      Any Other Error code apart from FSP_SUCCESS,  Unsuccessful write operation
 **********************************************************************************************************************/
fsp_err_t uart_get_user_msg(char* p_msg)
{
    fsp_err_t err = FSP_SUCCESS;

    uint8_t count = 0;

    uint8_t * p_receive_buffer = (uint8_t *)p_msg;  /* cast to uint8_t* */

    while ((TEXT_LENGTH - 2) > count)
    {
        /* read message */
        err = R_SCI_UART_Read(&g_uart0_ctrl, (uint8_t*)p_msg, 1);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT ("R_SCI_UART_Read API failed \r\n");
            return err;
        }

        while (!s_receive_complete)
        {
            ;   /* wait receive complete */
        }
        s_receive_complete = 0;

        /* Check each character input until Enter key is pressed */
        if ('\r' == (*p_msg))
        {
            /* add '\n' at the end of the string to indicate the end of the line */
            p_msg++;
            *p_msg = '\n';

            /* add '\0' at the end of the string to end the C string (Null-terminated String) */
            p_msg++;
            *p_msg = '\0';
            break;
        }
        p_msg++;
        count++;
    }

    /* Restore the p_msg pointer to its original position */
    p_msg = (char *)p_receive_buffer;   /* cast to char* */

    return err;    
}
/***********************************************************************************************************************
 * End of function: uart_get_user_msg
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  @fn         user_uart_callback
 *  @brief      UART user callback
 *  @param[in]  p_args
 *  @retval     None
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t * p_args)
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            break;
        }

        /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
        {
            s_receive_complete = 1;
            break;
        }

        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            s_transfer_complete = 1;
            break;
        }
        default:
        {
            ;   /* Do nothing */
        }
    }
}
/***********************************************************************************************************************
 * End of function: user_uart_callback
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @} (end addtogroup r_sci_uart_ep)
 **********************************************************************************************************************/
