/***********************************************************************************************************************
 * File Name    : timer_pwm.h
 * Description  : Contains data structures and functions declaration used in timer_pwm.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef TIMER_PWM_H_
#define TIMER_PWM_H_

/* Macro definition. */
#define MAX_CYCLE       (2000u)        /* Maximum cycle. */

/* Function declaration. */
fsp_err_t gtm_initialize (void);
fsp_err_t gtm_start (void);
fsp_err_t set_cycle (uint32_t raw_count);
void timer_gtm_deinit (void);

#endif /* TIMER_PWM_H_ */
