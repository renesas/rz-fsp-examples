/***********************************************************************************************************************
 * File Name    : can_fd_ep.c
 * Description  : Contains data structures and functions used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "can_fd_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_can_fd_ep
 * @{
 **********************************************************************************************************************/

/* Timeout value */
volatile uint32_t g_time_out = WAIT_TIME;

/* Flags to be set in Callback function */
volatile bool b_canfd_ch0_tx_complete = false;
volatile bool b_canfd_ch0_rx_complete = false;
volatile bool b_canfd_ch1_tx_complete = false;
volatile bool b_canfd_ch1_rx_complete = false;
volatile bool b_canfd_err_status      = false;

can_frame_t g_canfd_ch0_tx_frame;                      /* CAN 0 transmit frame */
can_frame_t g_canfd_ch0_rx_frame;                      /* CAN 0 receive frame */

can_frame_t g_canfd_ch1_tx_frame;                      /* CAN 1 transmit frame */
can_frame_t g_canfd_ch1_rx_frame;                      /* CAN 1 receive frame */

/* Variable to store RX frame status info of channel 0 */
can_info_t can_ch0_rx_info =
{
    .error_code = RESET_VALUE,
    .error_count_receive = RESET_VALUE,
    .error_count_transmit = RESET_VALUE,
    .rx_fifo_status = RESET_VALUE,
    .rx_mb_status = RESET_VALUE,
    .status = RESET_VALUE,
};

/* Variable to store RX frame status info of channel 1 */
can_info_t can_ch1_rx_info =
{
    .error_code = RESET_VALUE,
    .error_count_receive = RESET_VALUE,
    .error_count_transmit = RESET_VALUE,
    .rx_fifo_status = RESET_VALUE,
    .rx_mb_status = RESET_VALUE,
    .status = RESET_VALUE,
};

/* Data to be loaded in Classic CAN and FD frames for transmission and acknowledgment */
uint8_t tx_data[SIZE_64] = "TX_MESG";
uint8_t rx_data[SIZE_64] = "RX_MESG";
uint8_t tx_fd_data[SIZE_64];
uint8_t rx_fd_data[SIZE_64];

/* Acceptance filter array parameters */
const canfd_afl_entry_t p_canfd_ch0_afl[CANFD_CFG_AFL_CH0_RULE_NUM] =
{
    {
        /* Accept all messages with Extended ID 0x1000-0x1FFF */
        .id =
        {
            /* Specify the ID, ID type and frame type to accept */
            .id         = CANFD_FILTER_ID,
            .frame_type = CAN_FRAME_TYPE_DATA,
            .id_mode    = CAN_ID_MODE_EXTENDED
        },

        .mask =
        {
            /* These values mask which ID/mode bits to compare when filtering messages */
            .mask_id         = MASK_ID,
            .mask_frame_type = ZERO,
            .mask_id_mode    = MASK_ID_MODE
        },

        .destination =
        {
            /* If DLC checking is enabled any messages shorter than the below setting will be rejected */
            .minimum_dlc = (canfd_minimum_dlc_t)ZERO,

            /* Optionally specify a Receive Message Buffer (RX MB) to store accepted frames. RX MBs do not have an
             * interrupt or overwrite protection and must be checked with R_CANFD_InfoGet and R_CANFD_Read */
            .rx_buffer   = CANFD_RX_MB_0,

            /* Specify which FIFO(s) to send filtered messages to. Multiple FIFOs can be OR'd together */
            .fifo_select_flags = CANFD_RX_FIFO_0,
        }
    },
};

const canfd_afl_entry_t p_canfd_ch1_afl[CANFD_CFG_AFL_CH1_RULE_NUM] =
{
    {
        /* Accept all messages with Extended ID 0x1000-0x1FFF */
        .id =
        {
            /* Specify the ID, ID type and frame type to accept */
            .id         = CANFD_FILTER_ID,
            .frame_type = CAN_FRAME_TYPE_DATA,
            .id_mode    = CAN_ID_MODE_EXTENDED
        },

        .mask =
        {
        /* These values mask which ID/mode bits to compare when filtering messages */
            .mask_id         = MASK_ID,
            .mask_frame_type = ZERO,
            .mask_id_mode    = MASK_ID_MODE
        },

        .destination =
        {
            /* If DLC checking is enabled any messages shorter than the below setting will be rejected */
            .minimum_dlc = (canfd_minimum_dlc_t)ZERO,

            /* Optionally specify a Receive Message Buffer (RX MB) to store accepted frames. RX MBs do not have an
             * interrupt or overwrite protection and must be checked with R_CANFD_InfoGet and R_CANFD_Read */
            .rx_buffer   = CANFD_RX_MB_0,

            /* Specify which FIFO(s) to send filtered messages to. Multiple FIFOs can be OR'd together */
            .fifo_select_flags = CANFD_RX_FIFO_0,
        }
    },
};

/* User defined functions */
static void can_write_operation (canfd_instance_ctrl_t p_api_ctrl, can_frame_t can_transmit_frame);
static void can_fd_data_update (void);
static void can_data_check_operation (void);

/***********************************************************************************************************************
 * @brief       This function is to transmit data after receiving user input and it performs subsequent operations.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
void canfd_operation(void)
{
    /* Update transmit frame parameters */
    g_canfd_ch0_tx_frame.id               = CAN_ID;
    g_canfd_ch0_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
    g_canfd_ch0_tx_frame.type             = CAN_FRAME_TYPE_DATA;
    g_canfd_ch0_tx_frame.data_length_code = CAN_CLASSIC_FRAME_DATA_BYTES;
    g_canfd_ch0_tx_frame.options          = ZERO;

    /* Update transmit frame data with message */
    memcpy((uint8_t*)&g_canfd_ch0_tx_frame.data[ZERO], (uint8_t*)&tx_data[ZERO], CAN_FD_DATA_LENGTH_CODE);

    APP_PRINT("\r\n 1. Transmission of data over classic CAN Frame\r\n");

    /* Transmission of data over classic CAN frame */
    can_write_operation(g_canfd_ch0_ctrl, g_canfd_ch0_tx_frame);

    APP_PRINT("Classic CAN transmission is successful\r\n");
}

/***********************************************************************************************************************
 * @brief       This function is to transmit data on classic CAN or CANFD frame.
 * @param[in]   can_transmit_frame        Data frame to be transmitted.
 * @return      None.
 **********************************************************************************************************************/
static void can_write_operation(canfd_instance_ctrl_t p_api_ctrl, can_frame_t can_transmit_frame)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Transmit the data from mail box #0 with tx_frame */
    err = R_CANFD_Write(&p_api_ctrl, CAN_MAILBOX_NUMBER_0, &can_transmit_frame);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** CANFD Write API failed **\r\n");
        led_update(error);
        canfd_deinit();
        APP_ERR_TRAP(err);
    }

    led_update(transmitting);

    /* Wait here for an event from callback */
    while ((true != b_canfd_ch0_tx_complete) && (true != b_canfd_ch1_tx_complete))
    {
        g_time_out--;
        if (RESET_VALUE == g_time_out)
        {
            APP_ERR_PRINT("** CAN transmission failed due to timeout **\r\n");
            led_update(error);
            APP_ERR_TRAP(true);
            break;
        }
    }

    /* Reset flag bit */
    if (true == b_canfd_ch0_tx_complete)
    {
        b_canfd_ch0_tx_complete = false;
    }
    if (true == b_canfd_ch1_tx_complete)
    {
        b_canfd_ch1_tx_complete = false;
    }
}

/***********************************************************************************************************************
 * @brief       This function is to compare transmitted/received data and to send ACK accordingly.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
static void can_data_check_operation(void)
{
    /* Update data to be compared with data transmitted/received over FD frame */
    can_fd_data_update();

    if (RESET_VALUE == strncmp((char*)&g_canfd_ch1_rx_frame.data[ZERO], (char *)&tx_data[ZERO],
                               CAN_CLASSIC_FRAME_DATA_BYTES)) /* Cast to char * */
    {
        /* Clean receive frame */
        memset(&g_canfd_ch1_rx_frame.data[ZERO], NULL_CHAR, CAN_CLASSIC_FRAME_DATA_BYTES);

        APP_PRINT("Received 'TX__MESG' on classic frame.\r\n");
        APP_PRINT("\r\n 2. Responding with 'RX__MESG' using classic CAN frame\r\n");

        /* Update transmit frame parameters */
        g_canfd_ch1_tx_frame.id               = CAN_ID;
        g_canfd_ch1_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch1_tx_frame.type             = CAN_FRAME_TYPE_DATA;
        g_canfd_ch1_tx_frame.data_length_code = CAN_CLASSIC_FRAME_DATA_BYTES;
        g_canfd_ch1_tx_frame.options          = ZERO;

        /* Update transmit frame data with message */
        memcpy(&g_canfd_ch1_tx_frame.data, &rx_data, CAN_CLASSIC_FRAME_DATA_BYTES);

        /* Transmission of data as acknowledgment */
        can_write_operation(g_canfd_ch1_ctrl, g_canfd_ch1_tx_frame);

        APP_PRINT("CAN transmission after receive is successful. Sent back the ACK using classic CAN frame\r\n");

    }
    else if (RESET_VALUE == strncmp((char *)&g_canfd_ch0_rx_frame.data[ZERO], (char *)&rx_data[ZERO],
                                    CAN_CLASSIC_FRAME_DATA_BYTES)) /* Cast to char * */
    {
        /* Clean receive frame */
        memset(&g_canfd_ch0_rx_frame.data[ZERO], NULL_CHAR, CAN_CLASSIC_FRAME_DATA_BYTES);

        APP_PRINT("Received Acknowledgment for Classic CAN Frame transmission.\r\n"
                  "CAN operation Successful. Data length = %d\r\n", g_canfd_ch0_rx_frame.data_length_code);
        APP_PRINT("\r\n 3. Data transmission over FD frame\r\n");

        /* Update FD frame parameters for channel 0 */
        g_canfd_ch0_tx_frame.id               = CAN_ID;
        g_canfd_ch0_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch0_tx_frame.data_length_code = CAN_FD_DATA_LENGTH_CODE;
        g_canfd_ch0_tx_frame.options          = CANFD_FRAME_OPTION_FD | CANFD_FRAME_OPTION_BRS;

        /* Fill frame data that is to be sent in FD frame */
        for (uint16_t j = 0; j < SIZE_64; j++)
        {
            g_canfd_ch0_tx_frame.data[j] = (uint8_t) (j + 1); /* Cast to uint8_t */
        }

        /* Transmission of data as over FD frame */
        can_write_operation(g_canfd_ch0_ctrl, g_canfd_ch0_tx_frame);

        APP_PRINT("CAN transmission on FD Frame after receiving classic frame ACK is successful\r\n");

    }
    else if (RESET_VALUE == strncmp((char*)&g_canfd_ch1_rx_frame.data[ZERO], (char*)&tx_fd_data[ZERO],
                                    CAN_FD_DATA_LENGTH_CODE)) /* Acknowledgment for second transmission */
    {
        /* Clean receive frame */
        memset(&g_canfd_ch1_rx_frame.data[ZERO], NULL_CHAR, CAN_CLASSIC_FRAME_DATA_BYTES);

        APP_PRINT("Received data over FD Frame.\r\nCAN operation Successful. Data length = %d\r\n",
                  g_canfd_ch1_rx_frame.data_length_code);
        APP_PRINT("\r\n 4. Sending modified data over FD Frame now as acknowledgment for received FD data.\r\n");

        /* Update FD frame parameters for channel 1 */
        g_canfd_ch1_tx_frame.id               = CAN_ID;
        g_canfd_ch1_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch1_tx_frame.data_length_code = CAN_FD_DATA_LENGTH_CODE;
        g_canfd_ch1_tx_frame.options          = CANFD_FRAME_OPTION_FD | CANFD_FRAME_OPTION_BRS;

        /* Fill frame data that is to be sent in FD frame */
        for (uint16_t j = 0; j < SIZE_64; j++)
        {
            g_canfd_ch1_tx_frame.data[j] = (uint8_t) (j + 5); /* Cast to uint8_t */
        }

        /* Transmission of data as acknowledgment */
        can_write_operation(g_canfd_ch1_ctrl, g_canfd_ch1_tx_frame);

        APP_PRINT("CAN transmission on FD Frame as acknowledgment is successful\r\n");

    }
    else if (RESET_VALUE == strncmp((char*)&g_canfd_ch0_rx_frame.data[ZERO], (char*)&rx_fd_data[ZERO],
                                    CAN_FD_DATA_LENGTH_CODE)) /* Acknowledgment for second transmission */
    {
        /* Clean receive frame */
        memset(&g_canfd_ch0_rx_frame.data[ZERO], NULL_CHAR, CAN_CLASSIC_FRAME_DATA_BYTES);

        APP_PRINT("Received Acknowledgment for FD Frame.\r\nCAN operation Successful. Data length = %d\r\n\r\n",
                  g_canfd_ch0_rx_frame.data_length_code);
        APP_PRINT("Please enter any key on Terminal Emulator to initiate CAN transmission.\r\n\r\n");
    }
    else /* Wrong MSG Received */
    {
        APP_ERR_PRINT("** CAN data mismatch **\r\n** CAN operation failed **\r\n");
        led_update(error);
        APP_ERR_TRAP(true);
    }
}

/***********************************************************************************************************************
 * @brief       This function is to read Channel status info and read data.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
void can_read_operation(void)
{
    fsp_err_t err = FSP_SUCCESS;

    if (true == b_canfd_ch0_rx_complete)
    {
        /* Get the status information for CAN transmission */
        err = R_CANFD_InfoGet(&g_canfd_ch0_ctrl, &can_ch0_rx_info);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_CANFD_InfoGet API failed **\r\n");
            led_update (error);
            canfd_deinit ();
            APP_ERR_TRAP(err);
        }

        /* Check if the data is received in FIFO */
        if (can_ch0_rx_info.rx_mb_status)
        {
            /* Read the input frame received */
            err = R_CANFD_Read(&g_canfd_ch0_ctrl, ZERO, &g_canfd_ch0_rx_frame);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_CANFD_Read API failed **\r\n");
                led_update (error);
                canfd_deinit ();
                APP_ERR_TRAP(err);
            }

            /* Reset flag bit */
            b_canfd_ch0_rx_complete = false;

            /* Check if the transmitted and received data are same and send ACK accordingly */
            can_data_check_operation();
        }
        else
        {
            /* Do nothing */
        }

    }
    else if (true == b_canfd_ch1_rx_complete)
    {
        /* Get the status information for CAN transmission */
        err = R_CANFD_InfoGet(&g_canfd_ch1_ctrl, &can_ch1_rx_info);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_CANFD_InfoGet API failed **\r\n");
            led_update (error);
            canfd_deinit ();
            APP_ERR_TRAP(err);
        }

        /* Check if the data is received in FIFO */
        if (can_ch1_rx_info.rx_mb_status)
        {
            /* Read the input frame received */
            err = R_CANFD_Read(&g_canfd_ch1_ctrl, ZERO, &g_canfd_ch1_rx_frame);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_CANFD_Read failed **\r\n");
                led_update (error);
                canfd_deinit ();
                APP_ERR_TRAP(err);
            }

            /* Reset flag bit */
            b_canfd_ch1_rx_complete = false;

            /* Check if the transmitted and received data are same and send ACK accordingly */
            can_data_check_operation();
        }
        else
        {
            /* Do nothing */
        }
    }
    else
    {
        /* Do nothing */
    }

    led_update(successful);
}

/***********************************************************************************************************************
 * @brief       This function is to update data buffer that is to be compared with transmitted FD frame data.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
static void can_fd_data_update(void)
{
    /* Fill frame data to be compared with data transmitted on CANFD frame */
    for (uint16_t i = 0; i < SIZE_64; i++)
    {
        tx_fd_data[i] = (uint8_t) (i + 1); /* Cast to uint8_t */
    }

    for (uint16_t j = 0; j < SIZE_64; j++)
    {
        rx_fd_data[j] = (uint8_t) (j + 5); /* Cast to uint8_t */
    }
}

/***********************************************************************************************************************
 * @brief      This function updates led state as per operation status.
 * @param[in]  led_state      Select LED states according to operations.
 * @retval     None.
 **********************************************************************************************************************/
void led_update(led_state_t led_state)
{
    switch (led_state)
    {
        case error:
        {
            /* PMOD LED will high to show error state */
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED1], BSP_IO_LEVEL_HIGH);
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED2], BSP_IO_LEVEL_HIGH);
            break;
        }
        case successful:
        {
            /* Holds level to set for pins */
            static bsp_io_level_t s_pin_level = BSP_IO_LEVEL_LOW;

            /* Toggle level for next write */
            if (BSP_IO_LEVEL_LOW == s_pin_level)
            {
                s_pin_level = BSP_IO_LEVEL_HIGH;
            }
            else
            {
                s_pin_level = BSP_IO_LEVEL_LOW;
            }

            /* PMOD LED will blink to show successful state */
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED1], s_pin_level);
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED2], s_pin_level);

            /* Delay */
            R_BSP_SoftwareDelay(WAIT_TIME, BSP_DELAY_UNITS_MILLISECONDS);

            break;
        }
        case transmitting:
        {
            /* Set the LED pin state low */
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED1], BSP_IO_LEVEL_LOW);
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[BSP_LED_LED2], BSP_IO_LEVEL_LOW);

            for (int i = 0; i < 2; i ++)
            {
                R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[i], BSP_IO_LEVEL_HIGH);

                /* Delay */
                R_BSP_SoftwareDelay(WAIT_TIME/10, BSP_DELAY_UNITS_MILLISECONDS);
            }

            for (int j = 0; j < 2; j ++)
            {
                R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t)g_bsp_leds.p_leds[j], BSP_IO_LEVEL_LOW);

                /* Delay */
                R_BSP_SoftwareDelay(WAIT_TIME/10, BSP_DELAY_UNITS_MILLISECONDS);
            }

            break;
        }
        default:
        {
            break;
        }
    }

}

/***********************************************************************************************************************
 * @brief      CAN FD callback function.
 * @param[in]  p_args     Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void canfd_ch0_callback(can_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
            b_canfd_ch0_tx_complete = true;        /* Set flag bit */
            break;
        }
        case CAN_EVENT_RX_COMPLETE: /* Currently driver don't support this. This is unreachable code for now */
        {
            b_canfd_ch0_rx_complete = true;
            break;
        }
        case CAN_EVENT_ERR_WARNING:             /* Error warning event */
        case CAN_EVENT_ERR_PASSIVE:             /* Error passive event */
        case CAN_EVENT_ERR_BUS_OFF:             /* Error Bus Off event */
        case CAN_EVENT_BUS_RECOVERY:            /* Bus recovery error event */
        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    /* Overwrite/overrun error event */
        case CAN_EVENT_ERR_BUS_LOCK:            /* Bus lock detected (32 consecutive dominant bits) */
        case CAN_EVENT_ERR_CHANNEL:             /* Channel error has occurred */
        case CAN_EVENT_TX_ABORTED:              /* Transmit abort event */
        case CAN_EVENT_ERR_GLOBAL:              /* Global error has occurred */
        case CAN_EVENT_TX_FIFO_EMPTY :          /* Transmit FIFO is empty */
        case CAN_EVENT_FIFO_MESSAGE_LOST:       /* Receive FIFO overrun */
        {
            b_canfd_err_status = true;          /* Set flag bit */
            break;
        }
        default:
        {
            break;
        }
    }
}

/***********************************************************************************************************************
 * @brief      CAN FD callback function.
 * @param[in]  p_args     Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void canfd_ch1_callback(can_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
            b_canfd_ch1_tx_complete = true;        /* Set flag bit */
            break;
        }
        case CAN_EVENT_RX_COMPLETE: /* Currently driver don't support this. This is unreachable code for now */
        {
            b_canfd_ch1_rx_complete = true;
            break;
        }
        case CAN_EVENT_ERR_WARNING:             /* Error warning event */
        case CAN_EVENT_ERR_PASSIVE:             /* Error passive event */
        case CAN_EVENT_ERR_BUS_OFF:             /* Error Bus Off event */
        case CAN_EVENT_BUS_RECOVERY:            /* Bus recovery error event */
        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    /* Overwrite/overrun error event */
        case CAN_EVENT_ERR_BUS_LOCK:            /* Bus lock detected (32 consecutive dominant bits) */
        case CAN_EVENT_ERR_CHANNEL:             /* Channel error has occurred */
        case CAN_EVENT_TX_ABORTED:              /* Transmit abort event */
        case CAN_EVENT_ERR_GLOBAL:              /* Global error has occurred */
        case CAN_EVENT_TX_FIFO_EMPTY :          /* Transmit FIFO is empty */
        case CAN_EVENT_FIFO_MESSAGE_LOST:        /* Receive FIFO overrun */
        {
            b_canfd_err_status = true;          /* Set flag bit */
            break;
        }
        default:
        {
            break;
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function is to de-initializes the CANFD module.
 * @param[in]   None.
 * @return      None.
 **********************************************************************************************************************/
void canfd_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close CANFD channel */
    err = R_CANFD_Close(&g_canfd_ch0_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_CANFD_Close API for channel 0 failed **\r\n");
    }

    /* Close CANFD channel */
    err = R_CANFD_Close(&g_canfd_ch1_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_CANFD_Close API for channel 1 failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_can_fd_ep)
 **********************************************************************************************************************/
