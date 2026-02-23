/***********************************************************************************************************************
 * File Name    : timer_initialise.c
 * Description  : Contains data structures and functions used in timer_initialise.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "timer_initialise.h"

/***********************************************************************************************************************
 * @addtogroup timer_initialise
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  @brief       MTU3 timer initialization function.
 *  @param[IN]   None.
 *  @retval      FSP_SUCCESS    Upon successful open and start of timer.
 *  @retval      Any other error code apart from FSP_SUCCESS    Unsuccessful open or start.
 **********************************************************************************************************************/
fsp_err_t mtu3_timer_init(void)
{
    /* Variable to store FSP error codes */
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Open the MTU3 timer */
    fsp_err = R_MTU3_Open(&g_mtu3_ctrl, &g_mtu3_cfg);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        APP_ERR_PRINT("\r\nR_MTU3_Open API failed. Necessary cleanup is done.\n");
        return fsp_err;
    }

    /* Start MTU3 timer */
    fsp_err = R_MTU3_Start(&g_mtu3_ctrl);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        fsp_err_t err = FSP_SUCCESS;
        err = R_MTU3_Close(&g_mtu3_ctrl);

        /* Handle error in case of failure */
        if (FSP_SUCCESS != err)
        {
            /* Failure message */
            APP_ERR_PRINT("R_MTU3_Close API failed.\n\n");
        }

        /* Failure message */
        APP_ERR_PRINT("\r\nR_MTU3_Start API failed. Necessary cleanup is done.\n");
    }
    return fsp_err;
}

/***********************************************************************************************************************
 *  @brief       GTM timer initialization function.
 *  @param[IN]   None.
 *  @retval      FSP_SUCCESS    Upon successful open and start of timer.
 *  @retval      Any other error code apart from FSP_SUCCESS    Unsuccessful open or start.
 **********************************************************************************************************************/
fsp_err_t gtm_timer_init(void)
{
    /* Variable to store FSP error codes */
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Open the GTM timer */
    fsp_err = R_GTM_Open(&g_timer_ctrl, &g_timer_cfg);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Failure message */
        APP_ERR_PRINT("R_GTM_Open API failed. Necessary cleanup is done.\n\n");
        return fsp_err;
    }

    /* Start the GTM timer */
    fsp_err = R_GTM_Start(&g_timer_ctrl);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        fsp_err_t err = FSP_SUCCESS;
        err = R_GTM_Close(&g_timer_ctrl);

        /* Handle error in case of failure */
        if (FSP_SUCCESS != err)
        {
            /* Failure message */
            APP_ERR_PRINT("R_GTM_Close API failed.\n");
        }

        /* Failure message */
        APP_ERR_PRINT("\r\nR_GTM_Start API failed. Necessary cleanup is done.\n");
    }
    return fsp_err;
}

/***********************************************************************************************************************
 *  @brief       MTU3 timer de-initialization function.
 *  @param[IN]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void mtu3_timer_deinit(void)
{
    /* Variable to store FSP error codes */
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Close the MTU3 timer */
    fsp_err = R_MTU3_Close(&g_mtu3_ctrl);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Failure message */
        APP_ERR_PRINT("R_MTU3_Close API failed.\n\n");
    }
}

/***********************************************************************************************************************
 *  @brief       GTM timer de-initialization function.
 *  @param[IN]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void gtm_timer_deinit(void)
{
    /* Variable to store FSP error codes */
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Close the GTM timer */
    fsp_err = R_GTM_Close(&g_timer_ctrl);

    /* Handle error in case of failure */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Failure message */
        APP_ERR_PRINT("R_GTM_Close API failed.\n\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup timer_initialise)
 **********************************************************************************************************************/
