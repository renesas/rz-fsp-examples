/***********************************************************************************************************************
 * File Name    : timer_setup.h
 * Description  : Contains declarations of data structures and functions used in timer_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef TIMER_SETUP_H_
#define TIMER_SETUP_H_

/* WDT detect reset value */
#define R_CPG_CPG_WDTOVF_RST_WDTOVF1                (1u)

#define BUFFER_SIZE                                 (16u)

/* Number of counts for printing WDT refresh status */
#define WDT_REFRESH_COUNTER_VALUE                   (3u)

/* User command input value */
#define ENABLE_WDT                                  (1u)

/*
 * Function declarations
 */
/* This function initialize the GTM timer in periodic mode */
fsp_err_t init_gtm_module (void);

/* This function start the GTM HAL timer */
fsp_err_t timer_start (void);

/* This function close the GTM HAL driver */
void deinit_gtm_module (void);

#endif /* TIMER_SETUP_H_ */
