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
/* Information for module */
#define RESET_VALUE                 (0x00)

/* Maximum message length */
#define TEXT_LENGTH                 (128)

/* Function declaration */
fsp_err_t uart_print_user_msg (char * p_msg);
fsp_err_t uart_get_user_msg (char * p_msg);
fsp_err_t uart_initialize (void);

#endif /* UART_EP_H_ */
