/***********************************************************************************************************************
 * File Name    : timer_setup.h
 * Description  : Contains data structures and functions used in timer_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef TIMER_SETUP_H_
#define TIMER_SETUP_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

/* WDT detect reset value */
#define R_CPG_CPG_WDTOVF_RST_WDTOVF0                (1u)

#define BUFFER_SIZE                                 (16u)

/* Number of counts for printing WDT refresh status */
#define WDT_REFRESH_COUNTER_VALUE                   (3u)

/* User command input value */
#define ENABLE_WDT                                  (1u)

#define EP_INFO     "\r\nThis example project demonstrates the typical use of the WDT HAL module APIs."\
                    "\r\nUser input initializes the WDT and start GTM timer."\
                    "\r\nWDT counter is refreshed periodically every 1 second when the GTM timer expires."\
                    "\r\nOn pressing the Push button, WDT counter stops from refreshing."\
                    "\r\nWDT resets the MPU in 2 seconds.\r\n"

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
fsp_err_t init_gtm_module (void);
fsp_err_t timer_start (void);
void deinit_gtm_module (void);

#endif /* TIMER_SETUP_H_ */
