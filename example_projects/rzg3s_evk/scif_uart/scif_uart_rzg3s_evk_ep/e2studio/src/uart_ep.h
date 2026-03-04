/***********************************************************************************************************************
 * File Name    : uart_ep.h
 * Description  : Contains declarations of data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef UART_EP_H_
#define UART_EP_H_

/* Macro definition */
#define LINEFEED_ASCII            (10u)     /* Line Feed */
#define CARRIAGE_ASCII            (13u)     /* Carriage return */
#define ZERO_ASCII                (48u)     /* ASCII value of zero */
#define NINE_ASCII                (57u)     /* ASCII value for nine */
#define DATA_LENGTH               (5u)      /* Expected Input Data length */
#define UART_ERROR_EVENTS         (UART_EVENT_BREAK_DETECT | UART_EVENT_ERR_OVERFLOW | UART_EVENT_ERR_FRAMING | \
                                    UART_EVENT_ERR_PARITY)    /* UART Error event bits mapped in registers */

#define EP_INFO     "\r\nThe project initializes the UART with baud rate of 115200 bps."\
                    "\r\nOpen Serial Terminal with this baud rate value and"\
                    "\r\nProvide Input ranging from 1 - 2000 to set LED Period\r\n"
/* Function declaration */
/* UART Example project to demonstrate the functionality */
fsp_err_t uart_ep_demo (void);

/* This function prints a user message to the terminal */
fsp_err_t uart_print_user_msg (uint8_t * p_msg);

/* This function initializes the UART */
fsp_err_t uart_initialize (void);

/* This function de-initializes the SCIF UART module */
void deinit_uart (void);

#endif /* UART_EP_H_ */
