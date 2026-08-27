/***********************************************************************************************************************
 * File Name    : poeg_ep.c
 * Description  : Contains data structures and functions used in poeg_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "poeg_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_poeg_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief     This function de-initializes the GPT timer module.
 * @param[in] p_timer_ctl  Timer instance control structure.
 * @retval    None.
 **********************************************************************************************************************/
void deinit_gpt_timer(timer_ctrl_t * const p_timer_ctl)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Timer closes API call */
    err = R_GPT_Close(p_timer_ctl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GPT Close failure message */
        APP_ERR_PRINT("\r\nR_GPT_Close API failed\r\n");
    }
    else
    {
        APP_PRINT("\r\nGPT closes successfully\r\n");
    }
}

/***********************************************************************************************************************
 * @brief     This function initializes the GPT timer0 module.
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful GPT Open.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t init_gpt_timer_pwm0(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_GPT_Open(&g_timer_pwm0_ctrl, &g_timer_pwm0_cfg);
    return (err);
}

/***********************************************************************************************************************
 * @brief     This function initializes the GPT timer5 module.
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful GPT Open.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t init_gpt_timer_pwm5(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_GPT_Open(&g_timer_pwm5_ctrl, &g_timer_pwm5_cfg);
    return (err);
}

/*******************************************************************************************************************//**
 * @brief     This function initializes the POEG0 module.
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful POEG0 Open.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t init_poeg0_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize the POEG */
    err = R_POEG_Open(&g_poeg0_ctrl, &g_poeg0_cfg);
    return err;
}

/***********************************************************************************************************************
 * @brief     This function de-initializes the POEG0 module.
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful POEG0 Close.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t deinit_poeg0_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* De-initialize the POEG */
    err = R_POEG_Close(&g_poeg0_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_POEG_Close API failed\r\n");
    }
    else
    {
        APP_PRINT("POEG 0 de-initializes successfully\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief     This function initializes the POEG3 module
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful POEG3 Open.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t init_poeg3_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Initialize the POEG */
    err = R_POEG_Open(&g_poeg3_ctrl, &g_poeg3_cfg);
    return err;
}

/***********************************************************************************************************************
 * @brief     This function de-initializes the POEG3 module.
 * @param[in] None.
 * @retval    FSP_SUCCESS  Upon successful POEG3 Close.
 * @retval    Any other error code apart from FSP_SUCCESS Upon unsuccessful.
 **********************************************************************************************************************/
fsp_err_t deinit_poeg3_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* De-initialize the POEG */
    err = R_POEG_Close(&g_poeg3_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_POEG_Close API failed\r\n");
    }
    else
    {
        APP_PRINT("POEG 3 de-initializes successfully\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_poeg_ep)
 **********************************************************************************************************************/
