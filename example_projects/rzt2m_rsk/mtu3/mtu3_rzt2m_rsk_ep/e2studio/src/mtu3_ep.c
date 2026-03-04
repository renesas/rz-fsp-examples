/***********************************************************************************************************************
 * File Name    : mtu3_ep.c
 * Description  : Contains data structures and functions used in mtu3_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "mtu3_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_mtu3_ep
 * @{
 **********************************************************************************************************************/
/*
 * Private Global Variables
 */
/* Timer data */
static uint32_t s_mtu3_tgr_data[MTU3_DATA_CNT] = {
    MTU3_1_875MS_PCLKH_16,  /* Duty cycle = 25%: (Period - Duty count) / Period */
    MTU3_1_275MS_PCLKH_16,  /* Duty cycle = 50%: (Period - Duty count) / Period */
    MTU3_625US_PCLKH_16     /* Duty cycle = 75%: (Period - Duty count) / Period */
};

uint8_t g_duty_cnt;

/*
 * private functions prototype
 */
/***********************************************************************************************************************
 * @brief     Initialize MTU3 driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       MTU3 driver opened and set default timer successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCESS like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_mtu3(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open MTU3 driver */
    err = R_MTU3_Open(&g_timer_ctrl, &g_timer_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_Open API failed \r\n");
        return err;
    }

    /* Configure timer period */
    err = R_MTU3_PeriodSet(&g_timer_ctrl, MTU3_2_5MS_PCLKH_16);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_PeriodSet API failed \r\n");
        return err;
    }

    /* Configure default duty cycle (25%) */
    err = R_MTU3_DutyCycleSet(&g_timer_ctrl, s_mtu3_tgr_data[DEFAULT_TIMER_SETTING], UNUSED_PARAM);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_DutyCycleSet API failed \r\n");
        return err;
    }

    return err;
}

/***********************************************************************************************************************
 * Function Name: r_main_mtu_change_timer
 * Description  : Change the timer with duty rate (25% -> 50% -> 75%).
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void r_main_mtu_change_timer(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    g_duty_cnt++;
    
    if (MTU3_DATA_CNT <= g_duty_cnt)
    {
        /* Clear duty counter */
        g_duty_cnt = 0; 
    }
    
    /* Update duty cycle */
    err = R_MTU3_DutyCycleSet(&g_timer_ctrl, s_mtu3_tgr_data[g_duty_cnt], UNUSED_PARAM);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_DutyCycleSet API failed \r\n");
        APP_ERR_TRAP (err);
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_mtu3_ep)
 **********************************************************************************************************************/
