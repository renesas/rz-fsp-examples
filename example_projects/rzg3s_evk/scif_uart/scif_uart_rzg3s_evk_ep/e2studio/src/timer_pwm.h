/***********************************************************************************************************************
 * File Name    : timer_pwm.h
 * Description  : Contains declarations of data structures and functions used in timer_pwm.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef TIMER_PWM_H_
#define TIMER_PWM_H_

/* Macros definition */
#define MAX_CYCLE       (2000u)        /* Maximum cycle */
/* Function declaration */
/* This function initializes the GTM in periodic mode */
fsp_err_t gtm_initialize (void);

/* This function initializes the GTM in periodic mode */
fsp_err_t gtm_start (void);

/* Set cycle of GTM */
fsp_err_t set_cycle (uint32_t raw_count);

/* This function closes the GTM HAL driver */
void timer_gtm_deinit (void);

#endif /* TIMER_PWM_H_ */
