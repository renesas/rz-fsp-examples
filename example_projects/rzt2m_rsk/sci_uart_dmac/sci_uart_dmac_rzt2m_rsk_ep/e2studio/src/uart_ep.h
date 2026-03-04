/***********************************************************************************************************************
 * File Name    : uart_ep.h
 * Description  : Contains declarations of data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef UART_EP_H_
#define UART_EP_H_

/* Macro definition */
/* Information for module */
#define RESET_VALUE                 (0x00)

/* Maximum message length */
#define TEXT_LENGTH                 (128)

#define EP_INFO                     "\nThis sample program demonstrates how to use the UART module to send and receive " \
                                    "\r\nmessages via the terminal on the RZ/T2M micro controller. The project initializes" \
                                    "\r\nthe UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate " \
                                    "\r\nvalue and transmits a 'Hello World' message to the terminal. It then enters a " \
                                    "\r\nreceive loop, waiting for user input. Any data received from the terminal is " \
                                    "\r\nimmediately echoed back.\r\n" \
                                    "\r\nAny API or event failure message is displayed via RTT Viewer.\r\n\r\n"

/* Function declaration */
fsp_err_t uart_echo (void);
fsp_err_t uart_print_user_msg (char * p_msg);
fsp_err_t uart_get_user_msg (char * p_msg);
fsp_err_t uart_initialize (void);

#endif /* UART_EP_H_ */
