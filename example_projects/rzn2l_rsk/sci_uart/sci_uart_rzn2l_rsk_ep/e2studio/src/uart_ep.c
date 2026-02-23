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
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t s_temp_buffer[DATA_LENGTH] = {RESET_VALUE};

/* length of data in buffer */
static volatile uint8_t s_input_length = RESET_VALUE;

/* Counter to update s_temp_buffer index */
static volatile uint8_t s_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
static volatile uint8_t s_data_received_flag = false;

/* Flag to check data input overflow */
static volatile bool s_uart_input_overflow_flag = false;

/* Flag for user callback */
static volatile uint8_t s_uart_event = RESET_VALUE;

/***********************************************************************************************************************
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
 *  @brief       Print user message to terminal
 *  @param[in]   p_msg
 *  @retval      FSP_SUCCESS                Upon success
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure
 *  @retval      Any Other Error code apart from FSP_SUCCESS,  Unsuccessful write operation
 **********************************************************************************************************************/
fsp_err_t uart_print_user_msg(uint8_t *p_msg)
{
    fsp_err_t   err = FSP_SUCCESS;

    uint8_t     msg_len         = RESET_VALUE;
    uint64_t    local_timeout   = (DATA_LENGTH * UINT32_MAX);

    char * p_temp_ptr = (char *)p_msg;  /* cast to char * type */
/* Calculate length of message received */
    msg_len = ((uint8_t)(strlen(p_temp_ptr)));

    /* Reset callback capture variable */
    s_uart_event = RESET_VALUE;

    /* Writing to terminal */
    err = R_SCI_UART_Write (&g_uart0_ctrl, p_msg, msg_len);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Check for event transfer complete */
    while ((UART_EVENT_TX_COMPLETE != s_uart_event) && (--local_timeout))
    {
        /* Check if any error event occurred */
        if (UART_ERROR_EVENTS == s_uart_event)
        {
            return FSP_ERR_TRANSFER_ABORTED;
        }
    }
    if (RESET_VALUE == local_timeout)
    {
        err = FSP_ERR_TIMEOUT;
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       Wait and echo user input from a terminal
 *  @param[in]   None
 *  @retval      FSP_SUCCESS     Upon success
 *  @retval      Any Other Error code apart from FSP_SUCCESS
 **********************************************************************************************************************/
fsp_err_t uart_echo(void)
{
    fsp_err_t err = FSP_SUCCESS;

    while (true)
    {
        if (s_data_received_flag)
        {
            s_data_received_flag  = false;

            /* Calculate s_temp_buffer length */
            s_input_length = ((uint8_t)(strlen((char *) &s_temp_buffer)));

            /* Check if input data length is in limit */
            if ((!s_uart_input_overflow_flag) && (MIN_DATA_LENGTH <= (uint8_t)s_input_length))
            {
                /* echo user input */
                err = uart_print_user_msg((uint8_t *)"\r\nUser input: ");
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                    return err;
                }

                err = uart_print_user_msg(s_temp_buffer);
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                    return err;
                }

                err = uart_print_user_msg((uint8_t *)"\r\n");   /* cast to uint8_t * type */
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                    return err;
                }

                /* Clear s_temp_buffer */
                memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);
            }
            else
            {
                /* Clear s_temp_buffer as data is not valid */
                memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);

                /* cast to uint8_t * type */
                err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 10 character \r\n");
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                    return err;
                }
            }
        } 
    }
}

/***********************************************************************************************************************
 *  @brief      UART user callback
 *  @param[in]  p_args
 *  @retval     None
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    s_uart_event = (uint8_t)p_args->event;

    if (UART_EVENT_RX_CHAR == p_args->event)
    {
        switch (p_args->data)
        {
            /* If Enter is pressed by user, set flag to process the data */
            case LINEFEED_ASCII:
            case CARRIAGE_ASCII:
            {
                s_counter_var               = RESET_VALUE;
                s_data_received_flag        = true;

                break;
            }
            default:
            {
                if (s_counter_var < DATA_LENGTH)
                {
                    /* Read all data provided by user until enter button is pressed */
                    s_temp_buffer[s_counter_var++] = (uint8_t)p_args->data;
                    s_uart_input_overflow_flag     = false;
                }
                else
                {
                    /* Input data overflow */
                    s_uart_input_overflow_flag = true;
                }
                break;
            }
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_sci_uart_ep)
 **********************************************************************************************************************/
