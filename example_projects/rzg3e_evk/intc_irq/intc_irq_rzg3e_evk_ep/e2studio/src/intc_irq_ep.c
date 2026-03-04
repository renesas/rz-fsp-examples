/***********************************************************************************************************************
 * File Name    : intc_irq_ep.c
 * Description  : Contains data structures and functions used in intc_irq_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "intc_irq_ep.h"

/***********************************************************************************************************************
 * @addtogroup intc_irq_ep
 * @{
 **********************************************************************************************************************/
/* Boolean flag to determine switch is pressed or not */
volatile bool g_sw_press = false;

/***********************************************************************************************************************
 * @brief       This function initializes INTC_IRQ module.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful open of INTC_IRQ module.
 * @retval      Any other error code apart from FSP_SUCCESS  unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t intc_irq_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open INTC IRQ driver */
    err = R_INTC_IRQ_ExternalIrqOpen(&g_external_irq_ctrl, &g_external_irq_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC IRQ Open failure message */
        APP_ERR_PRINT("\r\n**R_INTC_IRQ_ExternalIrqOpen API failed**\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function enables external interrupt for specified channel.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful enable of INTC IRQ driver.
 * @retval      Any other error code apart from FSP_SUCCESS  unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t intc_irq_enable(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Enable INTC IRQ driver */
    err = R_INTC_IRQ_ExternalIrqEnable(&g_external_irq_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC IRQ Enable failure message */
        APP_ERR_PRINT("\r\n**R_INTC_IRQ_ExternalIrqEnable API failed**\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function closes opened INTC_IRQ module before the project ends up in an error trap.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void intc_irq_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close INTC IRQ module */
    err = R_INTC_IRQ_ExternalIrqClose(&g_external_irq_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* INTC_IRQ Close failure message */
        APP_ERR_PRINT("\r\n**R_INTC_IRQ_ExternalIrqClose API failed**\r\n");
    }
}

/***********************************************************************************************************************
 * @brief      This is user defined external IRQ callback.
 * @param[IN]  p_args.
 * @retval     None.
 **********************************************************************************************************************/
void irq_ep_callback(external_irq_callback_args_t *p_args)
{
    /* Make sure it's the right interrupt */
    if (USER_SW_IRQ_NUMBER == p_args->channel)
    {
        g_sw_press = true;
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup intc_irq_ep)
 **********************************************************************************************************************/
