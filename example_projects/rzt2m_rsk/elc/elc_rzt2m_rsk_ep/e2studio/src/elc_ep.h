/***********************************************************************************************************************
 * File Name    : elc_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ELC_EP_H_
#define ELC_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define EP_INFO                "\r\nThis Example Project demonstrates the basic usage of ELC driver."\
                               "\r\nInput edge detection of single input port 0 and GPT7 events are linked using ELC."\
                               "\r\nThe start source for GPT7 is ELC Single Port 0 Event. GPT7 runs in Periodic mode"\
                               "\r\nOn giving valid RTT input, a high level from single port triggers timer to start counting."\
                               "\r\nCounter value will be displayed on RTT viewer.\r\n"

/* MACROs for Null character */
#define NULL_CHAR ('\0')

#endif /* ELC_EP_H_ */
