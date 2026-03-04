/***********************************************************************************************************************
 * File Name    : intc_tint_ep.c
 * Description  : Contains data structures and functions used in intc_tint_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "intc_tint_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_intc_tint_ep
 * @{
 **********************************************************************************************************************/

/* Boolean flag to determine switch is pressed or not */
volatile bool g_sw_press = false;

/***********************************************************************************************************************
 * @brief       This function initializes the TINT module.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful opening of the INTC TINT module.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful opening.
 **********************************************************************************************************************/
fsp_err_t intc_tint_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open INTC TINT driver */
    err = R_INTC_TINT_ExternalIrqOpen(&g_external_tint_ctrl, &g_external_tint_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC TINT Open failure message */
        APP_ERR_PRINT("**R_INTC_TINT_ExternalIrqOpen API failed**\r\n\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function enables external interrupt for specified channel.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful enable of INTC TINT driver.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t intc_tint_enable(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Enable INTC TINT driver */
    err = R_INTC_TINT_ExternalIrqEnable(&g_external_tint_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC TINT Enable failure message */
        APP_ERR_PRINT("**R_INTC_TINT_ExternalIrqEnable API failed**\r\n\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function closes opened TINT module before the project ends up in an Error Trap.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void intc_tint_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close INTC TINT module */
    err = R_INTC_TINT_ExternalIrqClose(&g_external_tint_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC TINT Close failure message */
        APP_ERR_PRINT("** R_INTC_TINT_ExternalIrqClose API failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @brief      tint_ep_callback callback function.
 * @param[in]  p_args     Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void tint_ep_callback(external_irq_callback_args_t *p_args)
{
    /* Make sure it's the right interrupt */
    if (USER_SW_TINT_NUMBER == p_args->channel)
    {
        g_sw_press = true;
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_intc_tint_ep)
 **********************************************************************************************************************/