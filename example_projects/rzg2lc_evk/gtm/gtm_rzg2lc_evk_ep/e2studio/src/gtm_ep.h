/***********************************************************************************************************************
 * File Name    : gtm_ep.h
 * Description  : Contains data structures and functions declaration used in gtm_ep.c.
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
/* Array indexing macro */
#define ARRAY_INDEX     (0U)

/* Macro to flag the status */
#define SET_FLAG                              (0x01)
#define RESET_FLAG                            (0x00)

/* Status of timer */
#define ALREADY_RUNNING                       (0x02)

/* Macro to check if the LED count is zero */
#define LED_COUNT_ZERO                        (0U)

/* Macros to define time-period value limits */
#define TIME_PERIOD_MAX  (2000U)
#define TIME_PERIOD_MIN  (0U)

/* Macro for null character */
#define NULL_CHAR   ('\0')

#define EP_INFO                 "\r\nThis Example project demonstrates the functionality of GTM in "\
                                "\r\nPeriodic mode and One-shot mode. On providing any input on the "\
                                "\r\nJ-Link RTT Viewer, GTM channel 2 starts in One-shot mode. GTM channel 1 "\
                                "\r\nstarts in Periodic mode when GTM channel 2 expires. Timer in "\
                                "\r\nPeriodic mode expires periodically at a time period specified "\
                                "\r\nby user and toggles the LD1 on PMOD LED.\r\n"
/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/

/* Function declarations */
fsp_err_t gtm_init(void);
fsp_err_t gtm_start_oneshot_timer(void);
void one_shot_timer_callback(timer_callback_args_t *p_args);
void periodic_timer_callback(timer_callback_args_t *p_args);
void gtm_deinit(void);

#endif /* GTM_EP_H_ */
