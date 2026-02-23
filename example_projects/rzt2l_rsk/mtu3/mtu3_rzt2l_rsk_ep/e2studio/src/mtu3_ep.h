/***********************************************************************************************************************
 * File Name    : mtu3_ep.h
 * Description  : Contains declarations of data structures and functions used in mtu3_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef MTU3_H_
#define MTU3_H_

/* Macro definition */
/* TCNT value */
#define MTU3_2_5MS_PCLKH_16         (0x7A11)    /* 2.5ms (PCLKH/16) - Period */
#define MTU3_1_875MS_PCLKH_16       (0x5B8D)    /* 1.875ms (PCLKH/16) - 25% duty cycle */
#define MTU3_1_275MS_PCLKH_16       (0x3D08)    /* 1.275ms (PCLKH/16) - 50% duty cycle */
#define MTU3_625US_PCLKH_16         (0x1E84)    /* 0.625ms (PCLKH/16) - 75% duty cycle */
/* Parameter unused */
#define UNUSED_PARAM                (0u)

/* Maximum number of duty cycle steps (25%, 50%, 75%) */
#define MTU3_DATA_CNT               (3u)

/* default timer setting with duty cycle = 25% */
#define DEFAULT_TIMER_SETTING       (0u)

/***********************************************************************************************************************
 * @fn        init_mtu3
 * @brief     Initialize MTU3 driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       MTU3 driver opened and set default timer successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCESS like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_mtu3 (void);

/***********************************************************************************************************************
 * Function Name: r_main_mtu_change_timer
 * Description  : Change the timer with duty rate (25% -> 50% -> 75%).
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void r_main_mtu_change_timer (void);

#endif /* MTU3_H_ */
