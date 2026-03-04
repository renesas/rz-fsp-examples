/***********************************************************************************************************************
 * File Name    : uart_ep.c
 * Description  : Contains data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "uart_ep.h"
#include "timer_pwm.h"

/***********************************************************************************************************************
 * @addtogroup r_scif_uart_ep
 * @{
 **********************************************************************************************************************/

/* Private global variables */
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t s_temp_buffer[DATA_LENGTH] = {RESET_VALUE};

/* Counter to update s_temp_buffer index */
static volatile uint8_t s_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
static volatile uint8_t s_data_received_flag = false;

/* Flag for user callback */
static volatile uint8_t s_uart_event = RESET_VALUE;

/***********************************************************************************************************************
 *  @brief       UART Example project to demonstrate the functionality.
 *  @param[in]   None.
 *  @retval      FSP_SUCCESS     Upon success.
 *  @retval      Any other error code apart from FSP_SUCCESS.
 **********************************************************************************************************************/
fsp_err_t uart_ep_demo(void)
{
    fsp_err_t err = FSP_SUCCESS;

    volatile bool b_valid_data = true;

    while (true)
    {
        if (s_data_received_flag)
        {
            s_data_received_flag    = false;
            uint8_t input_length    = RESET_VALUE;
            volatile uint32_t cycle = RESET_VALUE;

            /* Calculate s_temp_buffer length */
            input_length = ((uint8_t)(strlen((char *) &s_temp_buffer)));  /* Cast to char* */

            /* Check if input data length is in limit */
            if (DATA_LENGTH > (uint8_t)input_length)
            {
                /* This loop validates input data byte by byte to filter out decimals. (Floating point input)
                 * Any such data will be considered as invalid. */
                for (uint32_t buf_index = RESET_VALUE; buf_index < input_length; buf_index++)
                {
                    if ((ZERO_ASCII <= s_temp_buffer[buf_index]) && (NINE_ASCII >= s_temp_buffer[buf_index]))
                    {
                        /* Set b_valid_data flag as data is valid */
                        b_valid_data = true;
                    }
                    else
                    {
                        /* Clear data_valid flag as data is not valid, clear the buffer and break the loop */
                        memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);
                        b_valid_data = false;
                        break;
                    }
                }

                /* All bytes in data are integers, convert input to integer value to set cycle. */
                cycle = ((uint32_t)(atoi((char *) &s_temp_buffer)));   /* Cast to char* */

                /* Validation input data is in 1 - 2000 range. */
                if ((MAX_CYCLE < cycle) || (RESET_VALUE == cycle))
                {
                    /* Resetting the s_temp_buffer as data is out of limit */
                    memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);
                    b_valid_data = false;

                    /* Application is being run on Serial terminal hence transmitting error message to the same */
                    err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");
                    if (FSP_SUCCESS != err)
                    {
                        APP_ERR_PRINT("\r\n** uart_print_user_msg failed **\r\n");
                        return err;
                    }
                }
            }
            else
            {
                /* Clear data_valid flag as data is not valid, clear the s_temp_buffer */
                memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;

                /* Cast to uint8_t */
                err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT("\r\n** uart_print_user_msg failed **\r\n");
                    return err;
                }
            }

            /* Set intensity only for valid data */
            if (b_valid_data)
            {
                /* Change intensity of LED */
                err = set_cycle(cycle);
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT("\r\n** GTM failed while changing intensity **\r\n");
                    return err;
                }

                /* Resetting the temporary buffer */
                memset(s_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;

                /* Cast to uint8_t */
                err = uart_print_user_msg((uint8_t *)"\r\nAccepted value, the led is blinking with that value\r\n");

                /* Cast to uint8_t */
                err = uart_print_user_msg((uint8_t *)"Please set the next value\r\n");
                if (FSP_SUCCESS != err)
                {
                    return err;
                }
            }
        } 
    }
}

/***********************************************************************************************************************
 * @brief       This function initializes UART.
 * @param[in]   None.
 * @retval      FSP_SUCCESS Upon successful open and start of timer.
 * @retval      Any other error code apart from FSP_SUCCESS  unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t uart_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 */
    err = R_SCIF_UART_Open(&g_uart_ctrl, &g_uart_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_SCIF_UART_Open API failed");
        APP_ERR_TRAP(err);
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       This function prints user message to terminal.
 *  @param[in]   p_msg.
 *  @retval      FSP_SUCCESS                Upon success.
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure.
 *  @retval      Any other error code apart from FSP_SUCCESS,  unsuccessful write operation.
 **********************************************************************************************************************/
fsp_err_t uart_print_user_msg(uint8_t *p_msg)
{
    fsp_err_t err   = FSP_SUCCESS;

    uint8_t msg_len = RESET_VALUE;

    uint64_t local_timeout = (DATA_LENGTH * UINT32_MAX);

    uint8_t * p_temp_ptr = (uint8_t *)p_msg;   /* Cast to uint8_t */

    /* Calculate length of message received */
    msg_len = ((uint8_t)(strlen((char *)p_temp_ptr)));  /* Cast to char* */

    /* Reset callback capture variable */
    s_uart_event = RESET_VALUE;

    /* Writing to terminal */
    err = R_SCIF_UART_Write(&g_uart_ctrl, p_msg, msg_len);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\n** R_SCIF_UART_Write API Failed **\r\n");
        return err;
    }

    /* Check for event transfer complete */
    while ((UART_EVENT_TX_COMPLETE != s_uart_event) && (--local_timeout))
    {
        /* Check if any error event occurred */
        if (UART_ERROR_EVENTS == s_uart_event)
        {
            APP_ERR_PRINT("\r\n** UART Error Event Received **\r\n");
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
 *  @brief       This function de-initializes SCIF UART module.
 *  @param[in]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void deinit_uart(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close module */
    err =  R_SCIF_UART_Close(&g_uart_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_SCIF_UART_Close API failed");
        APP_ERR_TRAP(err);
    }
}

/***********************************************************************************************************************
 *  @brief      This is UART user callback.
 *  @param[in]  p_args.
 *  @retval     None.
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    s_uart_event = (uint8_t)p_args->event;

    /* Reset s_temp_buffer index if it exceeds than buffer size */
    if (DATA_LENGTH == s_counter_var)
    {
        s_counter_var = RESET_VALUE;
    }

    if (UART_EVENT_RX_CHAR == p_args->event)
    {
        switch (p_args->data)
        {
            /* If Enter is pressed by user, set flag to process the data */
            case LINEFEED_ASCII:
            {
                s_counter_var         = RESET_VALUE;
                s_data_received_flag  = true;
                break;
            }
            case CARRIAGE_ASCII:
            {
                s_counter_var         = RESET_VALUE;
                s_data_received_flag  = true;
                break;
            }

            /* Read all data provided by user until enter button is pressed */
            default:
            {
                s_temp_buffer[s_counter_var++] = (uint8_t)p_args->data; /* Cast to uint8_t */
                break;
            }
        }
    }
}

/*******************************************************************************************************************//**
 * @} (end addtogroup r_scif_uart_ep)
 **********************************************************************************************************************/
