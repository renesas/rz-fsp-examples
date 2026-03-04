/***********************************************************************************************************************
 * File Name    : can_fd_ep.h
 * Description  : Contains declarations of data structures and functions used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CAN_FD_EP_H_
#define CAN_FD_EP_H_

#define CAN_MAILBOX_NUMBER_0            (0U)               /* Mail box number */
#define CAN_CLASSIC_FRAME_DATA_BYTES    (8U)               /* Data Length code for classic frame */
#define CAN_FD_DATA_LENGTH_CODE         (16)               /* Data Length code for FD frame */
#define CAN_ID                          (0x1100)           /* ID to be updated in transmit frame */

/* Acceptance filter array parameters */
#define CANFD_FILTER_ID                 (0x00001000)
#define MASK_ID                         (0x1FFFF000)
#define MASK_ID_MODE                    (1)

#define ZERO                            (0U)               /* Array Index value */
#define NULL_CHAR                       ('\0')             /* MACRO for null character */
#define WAIT_TIME                       (500U)             /* Wait time value */

#define EP_INFO                     " This Example Project demonstrates CAN FD operations on Renesas RZ MPUs\r\n"\
                                    "using 1 RZ/A3UL board. On pressing any key on the Terminal Emulator,\r\n"\
                                    "data is transmitted from one channel to other.\r\n"\
                                    " On 1st transmission, Channel 0 transmits data to Channel 1. Channel 1\r\n"\
                                    "displays the received data.\r\n"\
                                    " On 2nd transmission, Channel 1 transmits updated data to Channel 0 as ACK.\r\n"\
                                    "Channel 0 displays the received data,then Channel 0 changes\r\n"\
                                    "CAN frame to CANFD frame and updates data.\r\n"\
                                    " On 3rd transmission, Channel 0 transmits updated data to Channel 1.\r\n"\
                                    "Channel 1 displays the received data, then Channel 1 changes CAN frame\r\n"\
                                    "to CANFD frame and updates data to transmits back to Channel 0 as ACK.\r\n"\
                                    " On 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK.\r\n"\
                                    "Channel 0 displays the received data.\r\n"\

#ifndef BUFFER_SIZE_DOWN
/* Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16) */
#define BUFFER_SIZE_DOWN                (16)
#endif

/* Enum to select LED that is to be made high */
typedef enum
{
    error = 1,
    successful = 2,
    transmitting = 3,
} led_state_t;

/* Function to perform CAN FD operation */
void canfd_operation (void);

/* Function to perform CAN read operation */
void can_read_operation (void);

/* Function to de-initialize CAN FD module */
void canfd_deinit (void);

/* LED for board */
extern bsp_leds_t g_bsp_leds;

/* Timeout value */
extern volatile uint32_t g_time_out;

/* Flags to be set in Callback function */
extern volatile bool b_canfd_ch0_tx_complete;
extern volatile bool b_canfd_ch0_rx_complete;
extern volatile bool b_canfd_ch1_tx_complete;
extern volatile bool b_canfd_ch1_rx_complete;
extern volatile bool b_canfd_err_status;

/* User defined function */
void led_update (led_state_t led_state);

#endif /* CAN_FD_EP_H_ */
