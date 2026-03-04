/***********************************************************************************************************************
 * File Name    : can_fd_ep.h
 * Description  : Contains data structures and functions declaration used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CAN_FD_EP_H_
#define CAN_FD_EP_H_

#define CAN_MAILBOX_NUMBER_0            (0U)               /* Mail box number. */
#define CAN_CLASSIC_FRAME_DATA_BYTES    (8U)               /* Data Length code for classic frame. */
#define CAN_FD_DATA_LENGTH_CODE         (16)               /* Data Length code for FD frame. */
#define CAN_ID                          (0x1100)           /* ID to be updated in transmit frame. */
/* Acceptance filter array parameters. */
#define CANFD_FILTER_ID                 (0x00001000)
#define MASK_ID                         (0x1FFFF000)
#define MASK_ID_MODE                    (1)
#define ZERO                            (0U)               /* Array Index value. */
#define NULL_CHAR                       ('\0')             /* Macro for null character. */
#define WAIT_TIME                       (500U)             /* Wait time value. */

#define EP_INFO     "\nThis example project demonstrates CANFD operations on Renesas RZ MPUs using 1 RZ/V board."\
                    "\nOn pressing any key on the RTT Viewer, data is transmitted from one Channel to other."\
                    "\nOn 1st transmission, Channel 0 transmits data to Channel 1. "\
                    "Channel 1 displays the received data."\
                    "\nOn 2nd transmission, Channel 1 transmits updated data to Channel 0 as ACK. "\
                    "Channel 0 displays the received data,"\
                    "\nthen Channel 0 changes CAN frame to CANFD frame and updates data."\
                    "\nOn 3rd transmission, Channel 0 transmits updated data to Channel 1. "\
                    "Channel 1 displays the received data,"\
                    "\nthen Channel 1 changes CAN frame to CANFD frame and updates data."\
                    "\nOn 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK. "\
                    "Channel 0 displays the received data.\n\n"

/* Enum to select LED that is to be made high */
typedef enum
{
   error = 1,
   successful = 2,
   transmitting = 3,
} led_state_t;

void canfd_operation(void);
void can_read_operation(void);
void canfd_deinit(void);

/* Array of Pmod LED on bottom row of Pmod0. */
static const uint16_t g_prv_leds[] =
{
    (uint16_t) BSP_IO_PORT_43_PIN_00,  /* LED0. */
    (uint16_t) BSP_IO_PORT_43_PIN_02,  /* LED1. */ 
    (uint16_t) BSP_IO_PORT_41_PIN_00,  /* LED2. */
    (uint16_t) BSP_IO_PORT_43_PIN_01,  /* LED3. */
};

/* Available user-controllable LEDs on this board. These enums can be can be used to index into the array of LED pins
 * found in the bsp_leds_t structure. */
typedef enum e_led
{
    LED_LED1,                      /* LED0. */
    LED_LED2,                      /* LED1. */
    LED_LED3,                      /* LED2. */
    LED_LED4,                      /* LED3. */
} led_t;

/* Structure with LED information for board. */
extern const bsp_leds_t g_leds;

/* Timeout value. */
extern volatile uint32_t g_time_out;

/* Flags to be set in callback function */
extern volatile bool b_canfd_ch0_tx_complete;
extern volatile bool b_canfd_ch0_rx_complete;
extern volatile bool b_canfd_ch1_tx_complete;
extern volatile bool b_canfd_ch1_rx_complete;
extern volatile bool b_canfd_err_status;

/* User defined function. */
void led_update(led_state_t led_state);

#endif /* CAN_FD_EP_H_ */
