/***********************************************************************************************************************
 * File Name    : ext_irq_setup.c
 * Description  : Contains data structures and functions used in ext_irq_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "common_utils.h"
#include "ext_irq_setup.h"
/***********************************************************************************************************************
 * @addtogroup r_wdt_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief       This function initializes the IRQ module.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS Upon successful open.
 * @retval      Any other error code apart from FSP_SUCCESS unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t init_irq_module(void)
{
    /* Variable to track error and return values */
    fsp_err_t err = FSP_SUCCESS;

    /* Open IRQ module */
    err = R_INTC_IRQ_ExternalIrqOpen(&g_external_irq_ctrl, &g_external_irq_cfg);
    if (FSP_SUCCESS != err)
    {
        /* Print Error on J-Link RTT Viewer console */
        APP_ERR_PRINT("\r\n** R_INTC_IRQ_ExternalIrqOpen API failed **\r\n");
    }

    return err;
}

/***********************************************************************************************************************
 * @brief       This function enables the User Push button and return the error to the Application.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS Push button enabled successfully.
 * @retval      Any other error code apart from FSP_SUCCESS  unsuccessful to enable push button.
 **********************************************************************************************************************/
fsp_err_t enable_irq_module(void)
{
    /* Variable to track error and return values */
    fsp_err_t err = FSP_SUCCESS;

    /* Enable external IRQ */
    err = R_INTC_IRQ_ExternalIrqEnable(&g_external_irq_ctrl);
    if (FSP_SUCCESS != err)
    {
        /* Print Error on J-Link RTT Viewer console */
        APP_ERR_PRINT("\r\n** R_IRQ_ExternalIrqEnable API failed **\r\n");
    }

    return err;
}

/***********************************************************************************************************************
 * @brief       This function closes the IRQ HAL driver and Handle the return closing API error, to the Application.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void deinit_irq_module(void)
{
    /* Variable to track error and return values */
    fsp_err_t err = FSP_SUCCESS;

    /* Close the IRQ module */
    err = R_INTC_IRQ_ExternalIrqClose(&g_external_irq_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Print Error on J-Link RTT Viewer console */
        APP_ERR_PRINT("\r\nR_INTC_IRQ_ExternalIrqClose API failed");
        return;
    }
}

/***********************************************************************************************************************
 * @brief     This function is called when the user presses the push button to stop the GTM timer.
 * @param[IN] p_args    external IRQ callback argument.
 * @retval    None.
 **********************************************************************************************************************/
void ext_irq_callback(external_irq_callback_args_t *p_args)
{
    /* Variable to track error and return values */
    fsp_err_t err = FSP_SUCCESS;

    /* Cast to void to indicate the parameter is unused */
    FSP_PARAMETER_NOT_USED(p_args);

    /* By pressing push button, GTM timer will stops running */
    err = R_GTM_Stop(&g_timer_ctrl);
    if (FSP_SUCCESS != err)
    {
        /* Print Error on J-Link RTT Viewer console */
        APP_ERR_PRINT("\r\nR_GTM_Stop API failed");
        return;
    }
    APP_PRINT("Push button is pressed.\r\nGTM timer stopped.\r\n");
}

/***********************************************************************************************************************
 * @} (end addtogroup r_wdt_ep)
 **********************************************************************************************************************/
