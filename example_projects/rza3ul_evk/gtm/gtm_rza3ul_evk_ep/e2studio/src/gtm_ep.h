/***********************************************************************************************************************
 * File Name    : gtm_ep.h
 * Description  : Contains declarations of data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef GTM_EP_H_
#define GTM_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* MACRO array indexing */
#define ARRAY_INDEX             (0U)

/* MACRO to flag the status */
#define SET_FLAG                (0x01)
#define RESET_FLAG              (0x00)

/* MACRO status of timer */
#define ALREADY_RUNNING         (0x02)

/* MACRO to check if the LED count is zero */
#define LED_COUNT_ZERO          (0U)

/* MACROs to define time-period value limits */
#define TIME_PERIOD_MAX         (2000U)
#define TIME_PERIOD_MIN         (0U)

/* MACRO for null character */
#define NULL_CHAR               ('\0')

/* Size of the buffer for terminal input to target from host */
#define BUFFER_SIZE_DOWN        (128)

#define EP_INFO    "This Example Project demonstrates the functionality of GTM in periodic mode and one-shot mode.\r\n"\
                   "On providing any input on the Terminal Emulator, GTM channel 0 starts in one-shot mode.\r\n"\
                   "GTM channel 1 starts in periodic mode when GTM channel 0 expires.\r\n"\
                   "Timer in periodic mode expires periodically at a time period specified by user and toggles the "\
                   "PMOD0 pin 9 LED.\r\n\r\n"

/***********************************************************************************************************************
 * User-defined APIs
 **********************************************************************************************************************/

/*
 * function declarations
 */
/* Function to initialize GTM module */
fsp_err_t gtm_init (void);

/* Function to start one-shot GTM timer */
fsp_err_t gtm_start_oneshot_timer (void);

/* This function checks status the GTM timer */
fsp_err_t timer_status_check (void);

/* Function to de-initialize GTM module */
void gtm_deinit (void);

#endif /* GTM_EP_H_ */
