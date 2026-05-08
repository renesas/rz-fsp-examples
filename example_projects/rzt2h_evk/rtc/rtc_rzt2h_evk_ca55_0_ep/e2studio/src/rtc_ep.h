/***********************************************************************************************************************
 * File Name    : rtc_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef RTC_EP_H_
#define RTC_EP_H_

/* Initial calendar year for RTC initialization */
#define CURRENT_YEAR        (2026)

/* Struct tm year field (years since 1900) */
#define YEARS_SINCE_1900    (CURRENT_YEAR - 1900)

#define EP_INFO     "\nThis example project demonstrates the use of the Real-Time Clock (RTC) on the"\
                    "\r\nRZ/T2H microprocessor. It initializes the RTC module with a start calendar time and enables."\
                    "\r\na 1-second periodic interrupt. On each periodic interrupt, the current calendar time is read"\
                    "\r\nand displayed via the Tera Term."\
                    "\r\nA hardware alarm is configured to trigger when the hour and minute match and"\
                    "\r\nthe current day-of-week is enabled. Any API failure is also displayed on the console.\r\n"

#endif /* RTC_EP_H_ */
