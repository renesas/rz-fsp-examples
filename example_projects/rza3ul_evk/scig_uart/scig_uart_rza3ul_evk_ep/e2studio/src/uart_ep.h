/***********************************************************************************************************************
 * File Name    : uart_ep.h
 * Description  : Contains declarations of data structures and functions used in uart_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef UART_EP_H_
#define UART_EP_H_

#include <stdio.h>
#include <stdlib.h>

/* Macro definition */
#define LINEFEED_ASCII          (10u)     /* Line Feed */
#define CARRIAGE_ASCII          (13u)     /* Carriage return */
#define ZERO_ASCII              (48u)     /* ASCII value of zero */
#define NINE_ASCII              (57u)     /* ASCII value for nine */
#define DATA_LENGTH             (5u)      /* Expected Input Data length */
#define UART_ERROR_EVENTS       (UART_EVENT_BREAK_DETECT | UART_EVENT_ERR_OVERFLOW | UART_EVENT_ERR_FRAMING | UART_EVENT_ERR_PARITY)    /* UART Error event bits mapped in registers */

#define EP_INFO                 "\r\nThe project initializes the UART with baud rate of 115200 bps\r\n"\
                                "Open Serial Terminal with this baud rate value and provide input\r\n"\
                                "ranging from 1 - 2000 to set time cycle values in milliseconds\r\n"\
                                "The input value will not be printed to the monitor \r\n\r\n"\
                                "Please set the value\r\n"

/* Function declaration */
fsp_err_t uart_ep_demo (void);
fsp_err_t uart_print_user_msg (uint8_t * p_msg);
fsp_err_t uart_initialize (void);
void      deinit_uart (void);

#define APP_ERR_TRAP(err)  ({\
        int _err_val = (err);\
        if (_err_val) {\
            printf("\r\nReturned Error Code: 0x%x\r\n", _err_val);\
            __asm__ volatile("brk 0x0\n");\
        }\
    })

#endif /* UART_EP_H_ */
