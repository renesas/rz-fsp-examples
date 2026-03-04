/***********************************************************************************************************************
 * File Name    : cmt_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CMT_EP_H_
#define CMT_EP_H_

/* Macros for example project info to be displayed */
#define EP_INFO           "\r\nThis Example Project demonstrates typical use of the CMT module APIs."\
                          "\r\nTimer configuration: periodic compare-match interrupt every 500 ms."\
                          "\r\nBehavior: the callback toggles the on-board LEDs; no user interaction is required.\r\n"\

/*
 * function declarations
 */
void timer_callback (timer_callback_args_t * p_args);

#endif /* CMT_EP_H_ */
