/***********************************************************************************************************************
 * File Name    : uart_ep.h
 * Description  : Contains declarations of data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef UART_EP_H_
#define UART_EP_H_

/* Macro definition */
#define LINEFEED_ASCII              (10u)     /* Line Feed */
#define CARRIAGE_ASCII              (13u)     /* Carriage return */
#define DATA_LENGTH                 (10u)     /* Expected maximum input data length */
#define MIN_DATA_LENGTH             (1u)      /* Expected minimum input data length */
/* UART Error event bits mapped in registers */
#define UART_ERROR_EVENTS           ((UART_EVENT_BREAK_DETECT | UART_EVENT_ERR_OVERFLOW) | \
                                    (UART_EVENT_ERR_FRAMING | UART_EVENT_ERR_PARITY))    

/* Information for module */
#define RESET_VALUE                 (0x00)

/* Function declaration */
fsp_err_t uart_echo (void);
fsp_err_t uart_print_user_msg (uint8_t * p_msg);
fsp_err_t uart_initialize (void);

#endif /* UART_EP_H_ */
