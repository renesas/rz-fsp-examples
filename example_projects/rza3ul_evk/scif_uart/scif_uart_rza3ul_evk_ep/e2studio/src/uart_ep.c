/***********************************************************************************************************************
 * File Name    : uart_ep.c
 * Description  : Contains data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "uart_ep.h"
#include "timer_pwm.h"
#include "fsp_version.h"

/***********************************************************************************************************************
 * @addtogroup r_scif_uart_ep
 * @{
 **********************************************************************************************************************/

/*
 * Private function declarations
 */

/*
 * Private global variables
 */
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_temp_buffer[DATA_LENGTH] = {RESET_VALUE};

/* Counter to update g_temp_buffer index */
static volatile uint8_t g_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
static volatile uint8_t g_data_received_flag = false;

/* Flag for user callback */
static volatile uint8_t g_uart_event = RESET_VALUE;

/* Put the runtime-built TX buffer in non-cached RAM so the DMA-based UART sees fresh data */
static char fsp_version_str[32]  __attribute__((section("UNCACHED_BSS"), aligned(64)));

/***********************************************************************************************************************
 *  @brief       UART Example project to demonstrate the functionality.
 *  @param[in]   None.
 *  @retval      FSP_SUCCESS  Upon success.
 *  @retval      Any Other Error code apart from FSP_SUCCESS.
 **********************************************************************************************************************/
fsp_err_t uart_ep_demo(void)
{
    fsp_err_t err = FSP_SUCCESS;
    volatile bool b_valid_data = true;
    snprintf(fsp_version_str, sizeof(fsp_version_str), "%u.%u.%u",
             (unsigned) FSP_VERSION_MAJOR,
             (unsigned) FSP_VERSION_MINOR,
             (unsigned) FSP_VERSION_PATCH);
    err = uart_print_user_msg((uint8_t *) BANNER_1);
    err = uart_print_user_msg((uint8_t *) BANNER_2);
    err = uart_print_user_msg((uint8_t *) BANNER_3);
    err = uart_print_user_msg((uint8_t *) BANNER_4);
    err = uart_print_user_msg((uint8_t *) fsp_version_str);
    err = uart_print_user_msg((uint8_t *) BANNER_5);
    err = uart_print_user_msg((uint8_t *) BANNER_6);
    err = uart_print_user_msg((uint8_t *) BANNER_7);
    err = uart_print_user_msg((uint8_t *)"The project initializes the UART with baud rate of 115200 bps\r\n");
    err = uart_print_user_msg((uint8_t *)"Open Serial Terminal with this baud rate value and\r\n");
    err = uart_print_user_msg((uint8_t *)"Provide input ranging from 1 - 2000 to set time cycle values in milliseconds\r\n\r\n");

    while (true)
    {
        if (g_data_received_flag)
        {
            g_data_received_flag  = false;

            uint8_t input_length = RESET_VALUE;
            volatile uint32_t cycle = RESET_VALUE;

            /* Calculate g_temp_buffer length */
            input_length = ((uint8_t)(strlen((char *) &g_temp_buffer)));

            /* Check if input data length is in limit */
            if (DATA_LENGTH > (uint8_t)input_length)
            {
                /* This loop validates input data byte by byte to filter out decimals. (floating point input)
                 * Any such data will be considered as invalid. */
                for (int buf_index = RESET_VALUE; buf_index < input_length; buf_index++)
                {
                    if (ZERO_ASCII <= g_temp_buffer[buf_index] && NINE_ASCII >= g_temp_buffer[buf_index])
                    {
                        /* Set b_valid_data Flag as data is valid */
                        b_valid_data = true;
                    }
                    else
                    {
                        /* Clear data_valid flag as data is not valid, Clear the buffer and break the loop */
                        memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                        b_valid_data = false;
                        break;
                    }
                }

                /* All bytes in data are integers, convert input to integer value to set cycle. */
                cycle = ((uint32_t)(atoi((char *) &g_temp_buffer)));

                /* Validation input data is in 1 - 2000 range. */
                if ((MAX_CYCLE < cycle) || (RESET_VALUE == cycle))
                {
                    /* Reset the g_temp_buffer as data is out of limit */
                    memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                    b_valid_data = false;

                    /* Application is being run on Serial terminal hence transmitting error message to the same */
                    err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");
                    if (FSP_SUCCESS != err)
                    {
                        return err;
                    }
                }
            }
            else
            {
                /* Clear data_valid flag as data is not valid, Clear the g_temp_buffer */
                memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;
                err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");
                if (FSP_SUCCESS != err)
                {
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
                    return err;
                }

                /* Reset the temporary buffer */
                memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;

                /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)"\r\nAccepted value, the led is blinking with that value\r\n");

                /* Conversion to unsigned integer type */
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
 * @retval      FSP_SUCCESS  Upon successful open and start of timer.
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t uart_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 */
    err = R_SCIF_UART_Open (&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       This function prints user message to terminal.
 *  @param[in]   p_msg.
 *  @retval      FSP_SUCCESS                Upon success.
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure.
 *  @retval      Any Other Error code apart from FSP_SUCCESS,  Unsuccessful write operation.
 **********************************************************************************************************************/
fsp_err_t uart_print_user_msg(uint8_t *p_msg)
{
    fsp_err_t err = FSP_SUCCESS;
    uint8_t msg_len = RESET_VALUE;
    uint64_t local_timeout = (DATA_LENGTH * UINT32_MAX);
    char *p_temp_ptr = (char *)p_msg;

    /* Calculate length of message received */
    msg_len = ((uint8_t)(strlen(p_temp_ptr)));

    /* Reset callback capture variable */
    g_uart_event = RESET_VALUE;

    /* Write to terminal */
    err = R_SCIF_UART_Write (&g_uart0_ctrl, p_msg, msg_len);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Check for event transfer complete */
    while ((UART_EVENT_TX_COMPLETE != g_uart_event) && (--local_timeout))
    {
        /* Check if any error event occurred */
        if (UART_ERROR_EVENTS == g_uart_event)
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
 *  @brief       This function de-initializes SCIF UART module.
 *  @param[in]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void deinit_uart(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close module */
    err =  R_SCIF_UART_Close (&g_uart0_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
}

/***********************************************************************************************************************
 *  @brief      This is UART user callback function.
 *  @param[in]  Callback arguments.
 *  @retval     None.
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    g_uart_event = (uint8_t)p_args->event;

    /* Reset g_temp_buffer index if it exceeds than buffer size */
    if (DATA_LENGTH == g_counter_var)
    {
        g_counter_var = RESET_VALUE;
    }

    if (UART_EVENT_RX_CHAR == p_args->event)
    {
        switch (p_args->data)
        {
            /* If Enter is pressed by user, set flag to process the data */
            case LINEFEED_ASCII:
            {
                g_counter_var = RESET_VALUE;
                g_data_received_flag  = true;
                break;
            }
            case CARRIAGE_ASCII:
            {
                g_counter_var = RESET_VALUE;
                g_data_received_flag  = true;
                break;
            }

            /* Read all data provided by user until enter button is pressed */
            default:
            {
                g_temp_buffer[g_counter_var++] = (uint8_t) p_args->data;
                break;
            }
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_scif_uart_ep)
 **********************************************************************************************************************/
