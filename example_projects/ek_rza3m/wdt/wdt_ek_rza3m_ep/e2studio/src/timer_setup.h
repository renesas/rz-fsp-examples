/***********************************************************************************************************************
 * File Name    : timer_setup.h
 * Description  : Contains data structures and functions declaration used in timer_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef TIMER_SETUP_H_
#define TIMER_SETUP_H_

/* WDT detect reset value */
#define R_CPG_CPG_WDTOVF_RST_WDTOVF0                (1u)

#define BUFFER_SIZE                                 (16u)

/* Number of counts for printing WDT Refresh status */
#define WDT_REFRESH_COUNTER_VALUE                   (3u)

/* User command input value */
#define ENABLE_WDT                                  (1u)

/* Function declarations */
/* GTM timer initialization function */
fsp_err_t init_gtm_module (void);

/* GTM timer start function */
fsp_err_t timer_start (void);

/* GTM timer de-initialization function */
void deinit_gtm_module (void);

#endif /* TIMER_SETUP_H_ */
