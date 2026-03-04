/***********************************************************************************************************************
 * File Name    : gtm_timer_initialize.h
 * Description  : Contains declarations of data structures and functions used in gtm_timer_initialize.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef GTM_TIMER_INITIALIZE_H_
#define GTM_TIMER_INITIALIZE_H_

/* Function initializes the timer instance */
fsp_err_t gtm_timer_init(gtm_instance_ctrl_t * p_timer_ctrl, const timer_cfg_t * p_timer_cfg);

#endif /* GTM_TIMER_INITIALIZE_H_ */
