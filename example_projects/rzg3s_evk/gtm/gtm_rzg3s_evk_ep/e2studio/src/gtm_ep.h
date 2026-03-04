/***********************************************************************************************************************
 * File Name    : gtm_ep.h
 * Description  : Contains declarations of data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef GTM_EP_H_
#define GTM_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Macro array indexing */
#define ARRAY_INDEX             (0U)

/* Macro to flag the status */
#define SET_FLAG                (0x01)
#define RESET_FLAG              (0x00)

/* Status of timer */
#define ALREADY_RUNNING         (0x02)

/* Macro to check if the LED count is zero */
#define LED_COUNT_ZERO          (0U)

/* Macros to define time-period value limits */
#define TIME_PERIOD_MAX         (2000U)
#define TIME_PERIOD_MIN         (0U)

/* Macro for null character */
#define NULL_CHAR               ('\0')

#define EP_INFO     "\r\nThis Example Project demonstrates the functionality of GTM in "\
                    "\r\nperiodic mode and one-shot mode. On providing any input on the "\
                    "\r\nRTTviewer, GTM channel 2 starts in one-shot mode. GTM channel 1 "\
                    "\r\nstarts in periodic mode when GTM channel 2 expires. Timer in "\
                    "\r\nperiodic mode expires periodically at a time period specified "\
                    "\r\nby user and toggles the LED connect on PMOD1_3A.\r\n"

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/
/*
 * Function declarations
 */
/* This functions initializes GTM module */
fsp_err_t gtm_init (void);

/* This functions starts in one-shot mode */
fsp_err_t gtm_start_oneshot_timer (void);

/* This function is callback for one-shot mode timer */
void one_shot_timer_callback (timer_callback_args_t * p_args);

/* This function is callback for periodic timer */
void periodic_timer_callback (timer_callback_args_t * p_args);

/* This function closes opened GTM module before the project ends up in an Error Trap */
void gtm_deinit (void);

/* This function checks the status of both timers and starts/stops the timers */
fsp_err_t timer_status_check (void);

#endif /* GTM_EP_H_ */
