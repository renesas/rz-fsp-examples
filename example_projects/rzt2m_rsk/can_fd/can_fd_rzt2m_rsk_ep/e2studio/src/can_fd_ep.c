/***********************************************************************************************************************
 * File Name    : can_fd_ep.c
 * Description  : Contains data structures and functions used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "can_fd_ep.h"
#include "hal_data.h"

can_frame_t g_canfd_ch0_tx_frame;                      //CAN 0 transmit frame
can_frame_t g_canfd_ch0_rx_frame;                      //CAN 0 receive frame

can_frame_t g_canfd_ch1_tx_frame;                      //CAN 1 transmit frame
can_frame_t g_canfd_ch1_rx_frame;                      //CAN 1 receive frame

/* Variable to store rx frame status info of channel 0 */
can_info_t g_can_ch0_rx_info =
    {
        .error_code  = RESET_VALUE,
        .error_count_receive = RESET_VALUE,
        .error_count_transmit = RESET_VALUE,
        .rx_fifo_status = RESET_VALUE,
        .rx_mb_status = RESET_VALUE,
        .status = RESET_VALUE,
    };

/* Variable to store rx frame status info of channel 1 */
can_info_t g_can_ch1_rx_info =
    {
        .error_code  = RESET_VALUE,
        .error_count_receive = RESET_VALUE,
        .error_count_transmit = RESET_VALUE,
        .rx_fifo_status = RESET_VALUE,
        .rx_mb_status = RESET_VALUE,
        .status = RESET_VALUE,
    };

/* Data to be loaded in Classic CAN and FD frames for transmission and acknowledgement */
uint8_t g_tx_data[SIZE_64] = "TX_MESG";
uint8_t g_rx_data[SIZE_64] = "RX_MESG";
uint8_t g_tx_fd_data[SIZE_64];
uint8_t g_rx_fd_data[SIZE_64];

/* Flags to be set in Callback function */
volatile bool g_b_canfd_ch0_tx_complete = false;
volatile bool g_b_canfd_ch0_rx_complete = false;

volatile bool g_b_canfd_ch1_tx_complete = false;
volatile bool g_b_canfd_ch1_rx_complete = false;

volatile bool g_b_canfd_err_status      = false;

/* Timeout value */
volatile uint32_t g_time_out = WAIT_TIME;

/* User defined functions */
static void can_write_operation (can_ctrl_t * p_api_ctrl, can_frame_t * can_transmit_frame);
static void can_fd_data_update (void);
static void can_data_check_operation (void);

/* Acceptance filter array parameters */
const canfd_afl_entry_t p_canfd0_afl[CANFD_CFG_AFL_CH0_RULE_NUM] =
{
    {
        .id =
        {
            .id         = CAN_ID,
            .frame_type = CAN_FRAME_TYPE_DATA,
            .id_mode    = CAN_ID_MODE_STANDARD
        },
        .destination =
        {
            .minimum_dlc       = CANFD_MINIMUM_DLC_0,
            .rx_buffer         = CANFD_RX_MB_0,
            .fifo_select_flags = CANFD_RX_FIFO_0
        }
    },
};

const canfd_afl_entry_t p_canfd1_afl[CANFD_CFG_AFL_CH1_RULE_NUM] =
{
    {
        .id =
        {
            .id         = CAN_ID,
            .frame_type = CAN_FRAME_TYPE_DATA,
            .id_mode    = CAN_ID_MODE_STANDARD
        },
        .destination =
        {
            .minimum_dlc       = CANFD_MINIMUM_DLC_0,
            .rx_buffer         = CANFD_RX_MB_0,
            .fifo_select_flags = CANFD_RX_FIFO_0
        }
    },
};

/***********************************************************************************************************************
 *  Function Name: canfd_entry
 *  Description  : This function is used to start the canfd example operation.
 *  Arguments    : None
 *  Return Value : None
 **********************************************************************************************************************/
void canfd_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize canfd module */
    err = R_CANFD_Open(&g_canfd0_ctrl, &g_canfd0_cfg);

    /* Error trap */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\nCANFD Open API failed");
        APP_ERR_TRAP(err);
    }

    /* Cast to (can_instance_t *) to avoid discarding the 'const' qualifier from the pointer. */
    err = R_CANFD_Open(&g_canfd1_ctrl, &g_canfd1_cfg);

    /* Error trap */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\nCANFD Open API failed");
        APP_ERR_TRAP(err);
    }
}
/***********************************************************************************************************************
 * End of function: canfd_entry
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn          canfd_operation
 * @brief       This function is to transmit data after receiving user input and it performs subsequent operations
 * @param[in]   None
 * @return      None
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
    memcpy((uint8_t*)&g_canfd_ch0_tx_frame.data[ZERO], (uint8_t*)&g_tx_data[ZERO], CAN_FD_DATA_LENGTH_CODE);

    APP_PRINT("\n 1. Transmission of data over classic CAN Frame\n");

    /* Transmission of data over classic CAN frame */
    can_write_operation(&g_canfd0_ctrl, &g_canfd_ch0_tx_frame);

    APP_PRINT("\nClassic CAN transmission is successful");
}
/***********************************************************************************************************************
 * End of function: canfd_operation
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn          can_write_operation
 * @brief       This function is to  transmit data  on classic CAN or CANFD frame
 * @param[in]   can_transmit_frame        Data frame to be transmitted
 * @return      None
 **********************************************************************************************************************/
static void can_write_operation(can_ctrl_t *p_api_ctrl, can_frame_t *can_transmit_frame)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Transmit the data from mail box #0 with tx_frame */
    err = R_CANFD_Write(p_api_ctrl, CANFD_TX_MB_0, can_transmit_frame);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\nCANFD Write API FAILED");
        canfd_deinit();
        APP_ERR_TRAP(err);
    }

    /* Wait here for an event from callback */
    while ((true != g_b_canfd_ch0_tx_complete) && (true != g_b_canfd_ch1_tx_complete))
    {
        g_time_out--;
        if (RESET_VALUE == g_time_out)
        {
            APP_ERR_PRINT("\nCAN transmission failed due to timeout");
            APP_ERR_TRAP(true);
            break;
        }
    }

    /* Reset flag bit */
    if (true == g_b_canfd_ch0_tx_complete)
    {
        g_b_canfd_ch0_tx_complete = false;
    }
    if (true == g_b_canfd_ch1_tx_complete)
    {
        g_b_canfd_ch1_tx_complete = false;
    }
}
/***********************************************************************************************************************
 * End of function: can_write_operation
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn            can_data_check_operation
 * @details       This function is to compare transmitted/received data and to send ACK accordingly
 * @param[in]     None
 * @retval        None
 **********************************************************************************************************************/
static void can_data_check_operation(void)
{
    /*Update data to be compared with data transmitted/received over FD frame */
    can_fd_data_update();

    if (RESET_VALUE == strncmp((char*)&g_canfd_ch1_rx_frame.data[ZERO], // @suppress("3.1c Compiler specific functions")
        (char*)&g_tx_data[ZERO], CAN_CLASSIC_FRAME_DATA_BYTES)) /* cast uint8_t to char */
    {
        /* Cleaning receive frame */
        memset(&g_canfd_ch1_rx_frame.data[ZERO], NULL_CHAR, CAN_CLASSIC_FRAME_DATA_BYTES);

        APP_PRINT("\nReceived 'TX__MESG' on classic frame.");
        APP_PRINT("\n 2. Responding with 'RX__MESG' using classic CAN frame\n");

        /* Update transmit frame parameters */
        g_canfd_ch1_tx_frame.id               = CAN_ID;
        g_canfd_ch1_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch1_tx_frame.type             = CAN_FRAME_TYPE_DATA;
        g_canfd_ch1_tx_frame.data_length_code = CAN_CLASSIC_FRAME_DATA_BYTES;
        g_canfd_ch1_tx_frame.options          = ZERO;

        /* Update transmit frame data with message */
        memcpy(&g_canfd_ch1_tx_frame.data, &g_rx_data, CAN_CLASSIC_FRAME_DATA_BYTES);

        /* Transmission of data as acknowledgement */
        can_write_operation((can_ctrl_t *)&g_canfd1_ctrl, &g_canfd_ch1_tx_frame);

        APP_PRINT("\nCAN transmission after receive is successful. Sent back the ACK using classic CAN frame");

    }
    else if (RESET_VALUE == strncmp((char*)&g_canfd_ch0_rx_frame.data[ZERO],              /* cast uint8_t to char */
                                    (char*)&g_rx_data[ZERO], CAN_CLASSIC_FRAME_DATA_BYTES)) /* cast uint8_t to char */
    {
        /* Cleaning receive frame */
        memset(&g_canfd_ch0_rx_frame.data[ZERO], NULL_CHAR, CAN_FD_DATA_LENGTH_CODE);

        APP_PRINT("\nReceived Acknowledgement for Classic CAN Frame transmission."
                "\nCAN operation Successful. Data length = %d\n", g_canfd_ch0_rx_frame.data_length_code);
        APP_PRINT("\n 3. Data transmission over FD frame\n");

        /* Updating FD frame parameters for channel 0 */
        g_canfd_ch0_tx_frame.id               = CAN_ID;
        g_canfd_ch0_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch0_tx_frame.data_length_code = CAN_FD_DATA_LENGTH_CODE;
        g_canfd_ch0_tx_frame.options          = CANFD_FRAME_OPTION_FD | CANFD_FRAME_OPTION_BRS;

        /* Fill frame data that is to be sent in FD frame */
        for (uint16_t j = 0; j < SIZE_64; j++)
        {
            g_canfd_ch0_tx_frame.data[j] = (uint8_t)(j + 1);  /* cast uint16_t to uint8_t */
        }

        /* Transmission of data as over FD frame */
        can_write_operation((can_ctrl_t *)&g_canfd0_ctrl, &g_canfd_ch0_tx_frame);

        APP_PRINT("\nCAN transmission on FD Frame after receiving classic frame ACK is successful");
    }

    /* acknowledging for second transmission */
    else if (RESET_VALUE == strncmp((char*)&g_canfd_ch1_rx_frame.data[ZERO],
                                    (char*)&g_tx_fd_data[ZERO], CAN_FD_DATA_LENGTH_CODE))
    {
        /* Cleaning receive frame */
        memset(&g_canfd_ch1_rx_frame.data[ZERO], NULL_CHAR, CAN_FD_DATA_LENGTH_CODE);

        APP_PRINT("\nReceived data over FD Frame.\nCAN operation Successful. "
                "Data length = %d\n", g_canfd_ch1_rx_frame.data_length_code);

        APP_PRINT("\n 4. Sending modified data over FD Frame now as acknowledgement for received FD data.\n");

        /* Updating FD frame parameters for channel 1 */
        g_canfd_ch1_tx_frame.id               = CAN_ID;
        g_canfd_ch1_tx_frame.id_mode          = CAN_ID_MODE_EXTENDED;
        g_canfd_ch1_tx_frame.data_length_code = CAN_FD_DATA_LENGTH_CODE;
        g_canfd_ch1_tx_frame.options          = CANFD_FRAME_OPTION_FD | CANFD_FRAME_OPTION_BRS;

        /* Fill frame data that is to be sent in FD frame */
        for (uint16_t j = 0; j < SIZE_64; j++)
        {
            g_canfd_ch1_tx_frame.data[j] = (uint8_t)(j + 5); /* cast uint16_t to uint8_t */
        }

        /* Transmission of data as acknowledgement */
        can_write_operation((can_ctrl_t *)&g_canfd1_ctrl, &g_canfd_ch1_tx_frame);

        APP_PRINT("\nCAN transmission on FD Frame as acknowledgement is successful");
    }

    /* acknowledgement for second transmission */
    else if (RESET_VALUE == strncmp((char*)&g_canfd_ch0_rx_frame.data[ZERO],
                                    (char*)&g_rx_fd_data[ZERO], CAN_FD_DATA_LENGTH_CODE))
    {
        /* Cleaning receive frame */
        memset(&g_canfd_ch0_rx_frame.data[ZERO], NULL_CHAR, CAN_FD_DATA_LENGTH_CODE);

        APP_PRINT("\nReceived Acknowledgement for FD Frame.\nCAN operation Successful. "
                "Data length = %d\n\n", g_canfd_ch0_rx_frame.data_length_code);

        APP_PRINT("\nPlease enter any key on Terminal Emulator to initiate CAN transmission.\n");
    }
    else /* Unexpected message received */
    {
        APP_ERR_PRINT("\nCAN data mismatch\r\nCAN operation failed");
        APP_ERR_TRAP(true);
    }
}
/***********************************************************************************************************************
 * End of function: can_data_check_operation
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn          can_read_operation
 * @brief       This function is to read Channel status info and read data.
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
void can_read_operation(void)
{
    fsp_err_t err = FSP_SUCCESS;

    if (true == g_b_canfd_ch0_rx_complete)
    {
        /* Get the status information for CAN transmission */
        err = R_CANFD_InfoGet ((can_ctrl_t *)&g_canfd0_ctrl, &g_can_ch0_rx_info);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\nCAN InfoGet API FAILED");
            canfd_deinit ();
            APP_ERR_TRAP(err);
        }

        /* Check if the data is received in FIFO */
        if (g_can_ch0_rx_info.rx_mb_status)
        {
            /* Read the input frame received */
            err = R_CANFD_Read ((can_ctrl_t *)&g_canfd0_ctrl, ZERO, &g_canfd_ch0_rx_frame);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("\nCAN Read API FAILED");
                canfd_deinit ();
                APP_ERR_TRAP(err);
            }

            /* Reset flag bit */
            g_b_canfd_ch0_rx_complete = false;

            /*Check if the transmitted and received data are same and send ACK accordingly */
            can_data_check_operation();
        }
        else
        {
            /* Do nothing */;
        }

    }
    else if (true == g_b_canfd_ch1_rx_complete)
    {
        /* Get the status information for CAN transmission */
        err = R_CANFD_InfoGet (&g_canfd1_ctrl, &g_can_ch1_rx_info);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\nCAN InfoGet API FAILED");
            canfd_deinit ();
            APP_ERR_TRAP(err);
        }

        /* Check if the data is received in FIFO */
        if (g_can_ch1_rx_info.rx_mb_status)
        {
            /* Read the input frame received */
            err = R_CANFD_Read (&g_canfd1_ctrl, ZERO, &g_canfd_ch1_rx_frame);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("\nCAN Read API FAILED");
                canfd_deinit ();
                APP_ERR_TRAP(err);
            }

            /* Reset flag bit */
            g_b_canfd_ch1_rx_complete = false;

            /*Check if the transmitted and received data are same and send ACK accordingly */
            can_data_check_operation();
        }
        else
        {
            /* Do nothing */;
        }
    }
    else
    {
        /* Do nothing */;
    }
}
/***********************************************************************************************************************
 * End of function: can_read_operation
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn          can_fd_data_update
 * @brief       This function is to update data buffer that is to be compared with transmitted FD frame data
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
static void can_fd_data_update(void)
{
    /* Fill frame data to be compared with data transmitted on CANFD frame */
    for (uint16_t i = 0; i < SIZE_64; i++)
    {
        g_tx_fd_data[i]          = (uint8_t)(i + 1); /* cast uint16_t to uint8_t */
    }
    for (uint16_t j = 0; j < SIZE_64; j++)
    {
        g_rx_fd_data[j] = (uint8_t)(j + 5); /* cast uint16_t to uint8_t */
    }
}
/***********************************************************************************************************************
 * End of function: can_fd_data_update
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn         canfd0_callback
 * @brief      CAN FD callback function.
 * @param[in]  p_args
 * @retval     None
 **********************************************************************************************************************/
void canfd0_callback(can_callback_args_t *p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
            g_b_canfd_ch0_tx_complete = true;     // set flag bit
            break;
        }
        case CAN_EVENT_RX_COMPLETE:
        {
            g_b_canfd_ch0_rx_complete = true;     // set flag bit
            break;
        }
        case CAN_EVENT_ERR_WARNING:             // error warning event
        case CAN_EVENT_ERR_PASSIVE:             // error passive event
        case CAN_EVENT_ERR_BUS_OFF:             // error Bus Off event
        case CAN_EVENT_BUS_RECOVERY:            // Bus recovery error event
        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    // overwrite/overrun error event
        case CAN_EVENT_ERR_BUS_LOCK:            // Bus lock detected (32 consecutive dominant bits).
        case CAN_EVENT_ERR_CHANNEL:             // Channel error has occurred.
        case CAN_EVENT_TX_ABORTED:              // Transmit abort event.
        case CAN_EVENT_ERR_GLOBAL:              // Global error has occurred.
        case CAN_EVENT_TX_FIFO_EMPTY:           // Transmit FIFO is empty.
        case CAN_EVENT_FIFO_MESSAGE_LOST:       // Receive FIFO overrun.
        {
            g_b_canfd_err_status = true;          // set flag bit
            break;
        }
        default:
        {
            break;
        }
    }
}
/***********************************************************************************************************************
 * End of function: canfd0_callback
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn         canfd1_callback
 * @brief      CAN FD callback function.
 * @param[in]  p_args
 * @retval     None
 **********************************************************************************************************************/
void canfd1_callback(can_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case CAN_EVENT_TX_COMPLETE:
        {
            g_b_canfd_ch1_tx_complete = true;     // set flag bit
            break;
        }
        case CAN_EVENT_RX_COMPLETE:
        {
            g_b_canfd_ch1_rx_complete = true;     // set flag bit
            break;
        }
        case CAN_EVENT_ERR_WARNING:             // error warning event
        case CAN_EVENT_ERR_PASSIVE:             // error passive event
        case CAN_EVENT_ERR_BUS_OFF:             // error Bus Off event
        case CAN_EVENT_BUS_RECOVERY:            // Bus recovery error event
        case CAN_EVENT_MAILBOX_MESSAGE_LOST:    // overwrite/overrun error event
        case CAN_EVENT_ERR_BUS_LOCK:            // Bus lock detected (32 consecutive dominant bits).
        case CAN_EVENT_ERR_CHANNEL:             // Channel error has occurred.
        case CAN_EVENT_TX_ABORTED:              // Transmit abort event.
        case CAN_EVENT_ERR_GLOBAL:              // Global error has occurred.
        case CAN_EVENT_TX_FIFO_EMPTY:           // Transmit FIFO is empty.
        case CAN_EVENT_FIFO_MESSAGE_LOST:       // Receive FIFO overrun.
        {
            g_b_canfd_err_status = true;          // set flag bit
            break;
        }
        default :
        {
            break;
        }
    }
}
/***********************************************************************************************************************
 * End of function: canfd1_callback
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn          canfd_deinit
 * @brief       This function is to de-initializes the CANFD module
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
void canfd_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close CANFD channel */
    err = R_CANFD_Close((can_ctrl_t *)&g_canfd0_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\n**CANFD Close API failed**");
    }

    /* Close CANFD channel */
    err = R_CANFD_Close((can_ctrl_t *)&g_canfd1_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\n**CANFD Close API failed**");
    }
}
/***********************************************************************************************************************
 * End of function: canfd_deinit
 **********************************************************************************************************************/
