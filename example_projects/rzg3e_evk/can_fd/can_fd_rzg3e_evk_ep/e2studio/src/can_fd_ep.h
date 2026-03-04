/***********************************************************************************************************************
 * File Name    : can_fd_ep.h
 * Description  : Contains declarations of data structures and functions used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
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
#undef BUFFER_SIZE_UP
#define BUFFER_SIZE_UP                  (2048)
#define ZERO                            (0U)               /* Array index value */
#define NULL_CHAR                       ('\0')             /* Macro for null character */
#define WAIT_TIME                       (500U)             /* Wait time value */

#define EP_INFO     "\nThis example project demonstrates CANFD operations on Renesas RZ MPUs using 1 RZ/G board."\
                    "\nOn pressing any key on the RTT Viewer, data is transmitted from one channel to other."\
                    "\nOn 1st transmission, channel 4 transmits data to channel 1. Channel 1 displays the received data."\
                    "\nOn 2nd transmission, channel 1 transmits updated data to channel 4 as ACK. Channel 4 displays the received data,"\
                    "\nthen channel 4 changes CAN frame to CANFD frame and updates data."\
                    "\nOn 3rd transmission, channel 4 transmits updated data to channel 1. Channel 1 displays the received data,"\
                    "\nthen channel 1 changes CAN frame to CANFD frame and updates data."\
                    "\nOn 4th transmission, channel 1 transmits updated data to channel 4 as ACK. Channel 4 displays the received data.\n\n"

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

/* Timeout value */
extern volatile uint32_t g_time_out;

/* Flags to be set in callback function */
extern bool g_canfd_ch4_tx_complete;
extern bool g_canfd_ch4_rx_complete;
extern bool g_canfd_ch1_tx_complete;
extern bool g_canfd_ch1_rx_complete;
extern bool g_canfd_err_status;

/* User defined function */
void led_update(led_state_t led_state);

/* Array of PMOD LED on bottom row of Pmod1-3A. */
static const uint16_t g_prv_leds[] =
{
    (uint16_t) BSP_IO_PORT_06_PIN_05,  /* LD0 */
    (uint16_t) BSP_IO_PORT_05_PIN_06,  /* LD1 */
    (uint16_t) BSP_IO_PORT_06_PIN_06,  /* LD2 */
    (uint16_t) BSP_IO_PORT_06_PIN_04,  /* LD3 */
};

/* Available user-controllable LEDs on this board. These enums can be can be used to index into the array of LED pins
 * found in the bsp_leds_t structure. */
typedef enum e_led
{
    LED_LD0,                      /* LD0 */
    LED_LD1,                      /* LD1 */
    LED_LD2,                      /* LD2 */
    LED_LD3,                      /* LD3 */
} led_t;

#endif /* CAN_FD_EP_H_ */
