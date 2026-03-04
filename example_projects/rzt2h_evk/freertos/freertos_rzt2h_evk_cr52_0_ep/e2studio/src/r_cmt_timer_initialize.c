/***********************************************************************************************************************
 * File Name    : r_cmt_timer_initialize.c
 * Description  : Contains data structures and functions declaration used in r_cmt_timer_initialize.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "r_cmt_timer_initialize.h"

/***********************************************************************************************************************
 * @brief      Initialize and start the CMT timer instance
 * @param[in]  p_timer_ctrl  Timer instance control structure
 * @param[in]  p_timer_cfg   Timer instance config structure
 * @retval     FSP_SUCCESS or any other error value.
 **********************************************************************************************************************/
fsp_err_t r_cmt_timer_init(timer_ctrl_t * p_timer_ctrl, const timer_cfg_t * p_timer_cfg)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    /* Open R_CMT timer instance */
    fsp_err = R_CMT_Open (p_timer_ctrl, p_timer_cfg);

    /* Handle error */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Print out in case of error */
        APP_ERR_PRINT ("\r\nR_CMT Timer open API failed\r\n");
        return fsp_err;
    }

    /* Start R_CMT Timer instance */
    fsp_err = R_CMT_Start (p_timer_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != fsp_err)
    {
        /* Close timer if failed to start */
        if (R_CMT_Close (p_timer_ctrl) != FSP_SUCCESS)
        {
            /* Print out in case of error */
            APP_ERR_PRINT ("\r\nR_CMT Timer Close API failed\r\n");
        }

        APP_ERR_PRINT ("\r\nR_CMT Timer Start API failed\r\n");
        return fsp_err;
    }
    else
    {
        APP_PRINT ("\r\nR_CMT Timer Started successfully\r\n");
    }
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @} (end addtogroup freertos_ep)
 **********************************************************************************************************************/
