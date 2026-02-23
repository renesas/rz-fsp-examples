/***********************************************************************************************************************
 * File Name    : dmac_transfers.h
 * Description  : Contains declarations of data structures and functions used in dmac_transfers.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef DMAC_TRANSFERS_H_
#define DMAC_TRANSFERS_H_

#define NEW_LINE            ('\n')    /* New line character */
#define NULL_CHAR           ('\0')    /* End of string character */

#ifndef BUFFER_SIZE_DOWN
/* Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16) */
#define BUFFER_SIZE_DOWN    (16)
#endif

#define EP_INFO             "\r\nThe project initializes 2 DMAC transfer instances. Upon successful\r\n"\
                            "initialization, a menu option is displayed. On selecting 1 from menu\r\n"\
                            "option, GTM generates 100ms interrupt to trigger a transfer from the\r\n"\
                            "source buffer to port control register for 60 times indicated by LED\r\n"\
                            "toggling for 60 times. On selecting 2 from menu option, DMAC (runs in\r\n"\
                            "Block mode) transfers data from the MTU counter register to destination\r\n"\
                            "which gets printed to Terminal Emulator.\r\n\r\n"

#define MENU                "\r\nPress 1 to transfer source array to LED Port in normal mode."\
                            "\r\nPress 2 to transfer MTU current timer value.\r\n\r\n"

/* Perform transfer led blink operation */
void transfer_led_blink_operation (void);

/* Perform transfer MTU3 timer operation */
void transfer_mtu_value (void);

#endif /* DMAC_TRANSFERS_H_ */
