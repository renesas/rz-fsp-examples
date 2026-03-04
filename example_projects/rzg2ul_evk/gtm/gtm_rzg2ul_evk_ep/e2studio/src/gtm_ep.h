/***********************************************************************************************************************
 * File Name    : gtm_ep.h
 * Description  : Contains declarations of data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef GTM_EP_H_
#define GTM_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Array indexing MACRO */
#define ARRAY_INDEX     (0U)

/* MACRO to flag the status */
#define SET_FLAG                             (0x01)
#define RESET_FLAG                           (0x00)

/* Status of timer */
#define ALREADY_RUNNING                       (0x02)

/* MACRO to check if the LED count is zero */
#define LED_COUNT_ZERO                       (0U)

/* MACROs to define time-period value limits */
#define TIME_PERIOD_MAX  (2000U)
#define TIME_PERIOD_MIN  (0U)

/* MACRO for null character */
#define NULL_CHAR   ('\0')

#define EP_INFO    "\r\nThis Example Project demonstrates the functionality of GTM in "\
                   "\r\nperiodic mode and one-shot mode. On providing any input on the "\
                   "\r\nRTTviewer, GTM channel 2 starts in one-shot mode. GTM channel 1 "\
                   "\r\nstarts in periodic mode when GTM channel 2 expires. Timer in "\
                   "\r\nperiodic mode expires periodically at a time period specified "\
                   "\r\nby user and toggles the LED connect on Pmod 0.\r\n"

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/

/*
 * function declarations
 */
fsp_err_t gtm_init (void);
fsp_err_t gtm_start_oneshot_timer (void);
void gtm_deinit (void);

#endif /* GTM_EP_H_ */
