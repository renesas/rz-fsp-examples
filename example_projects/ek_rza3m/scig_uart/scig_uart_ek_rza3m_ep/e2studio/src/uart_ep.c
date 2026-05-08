/***********************************************************************************************************************
 * File Name    : uart_ep.c
 * Description  : Contains data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "uart_ep.h"
#include "timer_pwm.h"
#include "fsp_version.h"

/***********************************************************************************************************************
 * @addtogroup r_sci_uart_ep
 * @{
 **********************************************************************************************************************/

/* Private global variables */
/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_temp_buffer[DATA_LENGTH]  __attribute__((section("UNCACHED_BSS"))) = {RESET_VALUE};

/* Counter to update g_temp_buffer index */
static volatile uint8_t g_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
static volatile uint8_t g_data_received_flag = false;

/* Flag for user callback */
static volatile uint8_t g_uart_event = RESET_VALUE;

/* Banner informations */
static char g_banner_info[600] __attribute__((section("UNCACHED_BSS"))) = {RESET_VALUE};

/*****************************************************************************************************************
 *  @brief       This function demonstrates the functionality of UART example project.
 *  @param[in]   None.
 *  @retval      FSP_SUCCESS     Upon success.
 *  @retval      Any other error code apart from FSP_SUCCESS.
 ****************************************************************************************************************/
fsp_err_t uart_ep_demo(void)
{
    fsp_err_t err = FSP_SUCCESS;
    volatile bool b_valid_data = true;
    fsp_pack_version_t version;

    R_FSP_VersionGet(&version);

    snprintf(g_banner_info, sizeof(g_banner_info), BANNER_INFO, EP_VERSION, version.version_id_b.major,
            version.version_id_b.minor, version.version_id_b.patch);

    /* Print messages */
    err = uart_print_user_msg((uint8_t *)g_banner_info); /* Conversion to unsigned integer type */
    err = uart_print_user_msg((uint8_t *)EP_INFO);

    while (true)
    {
        if (g_data_received_flag)
        {
            g_data_received_flag  = false;

            uint8_t input_length    = RESET_VALUE;
            volatile uint32_t cycle = RESET_VALUE;

            /* Calculate g_temp_buffer length */
            input_length = ((uint8_t)(strlen((char *) &g_temp_buffer)));

            /* Check if input data length is in limit */
            if (DATA_LENGTH > (uint8_t)input_length)
            {
                /* This loop validates input data byte by byte to filter out decimals (floating point input).
                 * Any such data will be considered as invalid */
                for (int buf_index = RESET_VALUE; buf_index < input_length; buf_index++)
                {
                    if ((ZERO_ASCII <= g_temp_buffer[buf_index]) && (NINE_ASCII >= g_temp_buffer[buf_index]))
                    {
                        /* Set b_valid_data flag as data is valid */
                        b_valid_data = true;
                    }
                    else
                    {
                        /* Clear b_valid_data flag as data is not valid, clear the buffer and break the loop */
                        memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                        b_valid_data = false;
                        break;
                    }
                }

                /* All bytes in data are integers, convert input to integer value to set cycle */
                cycle = ((uint32_t)(atoi((char *) &g_temp_buffer)));

                /* Validation input data is in 1 - 2000 range */
                if ((MAX_CYCLE < cycle) || (RESET_VALUE == cycle))
                {
                    /* Reset the g_temp_buffer as data is out of limit */
                    memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                    b_valid_data = false;

                    /* Application is being run on Serial terminal hence transmitting error message to the same */
                    err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");
                    err = uart_print_user_msg((uint8_t *)"Please set the value\r\n");
                    if (FSP_SUCCESS != err)
                    {
                        return err;
                    }
                }
            }
            else
            {
                /* Clear b_valid_data flag as data is not valid, clear the g_temp_buffer */
                memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;

                /* Invalid input */
                /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 2000\r\n");

                /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)"Please set the value\r\n");
                if (FSP_SUCCESS != err)
                {
                    return err;
                }
            }

            /* Set intensity only for valid data */
            if (b_valid_data)
            {
                /* Display input value */
                err = uart_print_user_msg((uint8_t *)"\r\nInput value: "); /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)g_temp_buffer); /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)" (milliseconds)\r\n"); /* Conversion to unsigned integer type */
                /* Change intensity of LED */
                err = set_cycle(cycle);
                if (FSP_SUCCESS != err)
                {
                    return err;
                }

                /* Reset the temporary buffer */
                memset(g_temp_buffer, RESET_VALUE, DATA_LENGTH);
                b_valid_data = false;

                /* Display output */
                /* Conversion to unsigned integer type */
                err = uart_print_user_msg((uint8_t *)"Accepted value, the led is blinking with that value\r\n");

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
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t uart_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize UART channel with baud rate 115200 bps */
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       This function prints user message to terminal.
 *  @param[in]   p_msg     Pointer to user message.
 *  @retval      FSP_SUCCESS                Upon success.
 *  @retval      FSP_ERR_TRANSFER_ABORTED   Upon event failure.
 *  @retval      Any other error code apart from FSP_SUCCESS   Unsuccessful write operation.
 **********************************************************************************************************************/
fsp_err_t uart_print_user_msg(uint8_t *p_msg)
{
    fsp_err_t err   = FSP_SUCCESS;
    uint16_t msg_len = RESET_VALUE;
    uint64_t local_timeout = (DATA_LENGTH * UINT32_MAX);
    char *p_temp_ptr = (char *)p_msg;

    /* Calculate length of message received */
    msg_len = ((uint16_t)(strlen(p_temp_ptr)));

    /* Reset callback capture variable */
    g_uart_event = RESET_VALUE;

    /* Write to terminal */
    err = R_SCI_UART_Write(&g_uart0_ctrl, p_msg, msg_len);
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
 *  @brief       This function de-initializes SCI UART module.
 *  @param[in]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void deinit_uart(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close module */
    err =  R_SCI_UART_Close(&g_uart0_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
}

/***********************************************************************************************************************
 *  @brief      This is UART user callback function.
 *  @param[in]  p_args     Callback arguments.
 *  @retval     None.
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    g_uart_event = (uint8_t)p_args->event;

    /* Reset g_counter_var index if it exceeds than buffer size */
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
                g_counter_var         = RESET_VALUE;
                g_data_received_flag  = true;
                break;
            }
            case CARRIAGE_ASCII:
            {
                g_counter_var         = RESET_VALUE;
                g_data_received_flag  = true;
                break;
            }

            /* Read all data provided by user until enter button is pressed */
            default:
            {
                /* Read data */
                g_temp_buffer[g_counter_var++] = (uint8_t)p_args->data;
                break;
            }
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_sci_uart_ep)
 **********************************************************************************************************************/
