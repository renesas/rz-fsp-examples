/***********************************************************************************************************************
 * File Name    : r_cmt_timer_initialize.h
 * Description  : Contains declarations of data structures and functions used in r_cmt_timer_initialize.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef R_CMT_TIMER_INITIALIZE_H_
#define R_CMT_TIMER_INITIALIZE_H_

/* Function initializes the timer instance */
fsp_err_t r_cmt_timer_init (timer_ctrl_t * p_timer_ctrl, const timer_cfg_t * p_timer_cfg);

#endif /* R_CMT_TIMER_INITIALIZE_H_ */
