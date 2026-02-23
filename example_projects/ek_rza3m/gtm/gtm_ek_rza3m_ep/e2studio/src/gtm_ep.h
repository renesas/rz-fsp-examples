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
/* MACRO Array indexing */
#define ARRAY_INDEX                           (0U)

/* MACRO to flag the status */
#define SET_FLAG                              (0x01)
#define RESET_FLAG                            (0x00)

/* MACRO for Status of timer */
#define ALREADY_RUNNING                       (0x02)

/* MACRO to check if the LED count is zero */
#define LED_COUNT_ZERO                        (0U)

/* MACROs to define time-period value limits */
#define TIME_PERIOD_MAX                       (2000U)
#define TIME_PERIOD_MIN                       (0U)

/* MACRO for null character */
#define NULL_CHAR                             ('\0')

/* Size of the buffer for terminal input to target from host */
#define BUFFER_SIZE_DOWN                      (1024)

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/
/*
 * function declarations
 */
/* This function initializes the GTM timer */
fsp_err_t gtm_init (void);

/* This function starts the GTM timer in One short mode */
fsp_err_t gtm_start_oneshot_timer (void);

/* This function checks status the GTM timer */
fsp_err_t timer_status_check (void);

/* This function de-initializes the GTM timer */
void gtm_deinit (void);

#endif /* GTM_EP_H_ */
