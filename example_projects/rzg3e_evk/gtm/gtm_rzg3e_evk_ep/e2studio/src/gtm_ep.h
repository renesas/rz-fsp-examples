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
/* Array indexing macro */
#define ARRAY_INDEX     (0U)

/* Macro to flag the status */
#define SET_FLAG                             (0x01)
#define RESET_FLAG                           (0x00)

/* Status of timer */
#define ALREADY_RUNNING                       (0x02)

/* Macro to check if the LED count is zero */
#define LED_COUNT_ZERO                       (0U)

/* Macros to define time-period value limits */
#define TIME_PERIOD_MAX  (2000U)
#define TIME_PERIOD_MIN  (0U)

/* Macro for null character */
#define NULL_CHAR   ('\0')

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/

/* Function declarations */
fsp_err_t gtm_init(void);
fsp_err_t gtm_start_oneshot_timer(void);
fsp_err_t timer_status_check(void);

void gtm_deinit(void);

#endif /* GTM_EP_H_ */
