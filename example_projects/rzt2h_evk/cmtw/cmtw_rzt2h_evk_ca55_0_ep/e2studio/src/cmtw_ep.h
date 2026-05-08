/***********************************************************************************************************************
 * File Name    : cmtw_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CMTW_EP_H_
#define CMTW_EP_H_

/* Macro for example project info to be displayed */
#define EP_INFO     "\r\nThis example project demonstrates typical use of the CMTW module APIs."\
                    "\r\nThe timer is configured to generate a compare-match interrupt that toggles"\
                    " the on-board LED every two seconds."\
                    "\r\nNo user interaction is required after startup.\r\n"\

/* Function declaration */
void timer_callback (timer_callback_args_t * p_args);

#endif /* CMTW_EP_H_ */
