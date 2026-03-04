/***********************************************************************************************************************
 * File Name    : gtm_ep.c
 * Description  : Contains data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "gtm_ep.h"

/***********************************************************************************************************************
 * @addtogroup gtm_ep
 * @{
 **********************************************************************************************************************/

extern bsp_leds_t g_bsp_leds;
extern volatile uint8_t g_periodic_timer_flag;
extern volatile uint32_t g_error_flag;

/***********************************************************************************************************************
 * @brief       This function initializes GTM module.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS  Upon successful open of GTM module.
 * @retval      Any other error code apart from FSP_SUCCESS  unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t gtm_init(void)
{
    fsp_err_t err = FSP_SUCCESS; /* Error status */

    /* Open Timer 2 in One-shot Mode */
    err = R_GTM_Open(&g_timer_one_shot_ctrl, &g_timer_one_shot_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nR_GTM_Open API failed.\r\nRestart the Application\r\n");
        return err;
    }

    /* Open Timer 1 in Periodic mode */
    err = R_GTM_Open(&g_timer_periodic_ctrl, &g_timer_periodic_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Close Timer 2 in One-shot Mode */
        if ((FSP_SUCCESS != R_GTM_Close(&g_timer_one_shot_ctrl)))
        {
            APP_ERR_PRINT("\r\nR_GTM_Close API failed.\r\nRestart the Application\r\n");
        }
        APP_ERR_PRINT("\r\nR_GTM_Open API failed.\r\nRestart the Application\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function starts GTM2 in One-shot mode.
 * @param [IN]  None.
 * @retval      FSP_SUCCESS Timer started successfully.
 * @retval      Any other error code apart from FSP_SUCCESS.
 **********************************************************************************************************************/
fsp_err_t gtm_start_oneshot_timer(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Start Timer 2 in One-shot mode */
    err = R_GTM_Start(&g_timer_one_shot_ctrl);
    
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\n R_GTM_Start API failed.\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief      This function is callback for One-shot mode timer and starts GTM1 timer in Periodic mode.
 * @param [in] p_args.
 * @retval     None.
 **********************************************************************************************************************/
void one_shot_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    fsp_err_t err = FSP_SUCCESS;

    timer_status_t periodic_timer_status;

    /* Retrieve the status of timer running in Periodic mode */
    err = R_GTM_StatusGet(&g_timer_periodic_ctrl, &periodic_timer_status);
    if (FSP_SUCCESS != err)
    {
        g_error_flag = SET_FLAG;
    }
    else
    {
        if (TIMER_STATE_COUNTING != periodic_timer_status.state)
        {
            /* Start the timer in Periodic mode only if the timer is in stopped state */
            err = R_GTM_Start(&g_timer_periodic_ctrl);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                g_error_flag = SET_FLAG;
                APP_ERR_PRINT("\r\n R_GTM_Start API failed.\r\n");
            }
            else
            {
                g_periodic_timer_flag = SET_FLAG;   /* Set the flag since Timer 1 is started in periodic mode */
            }
        }
        else
        {
            g_periodic_timer_flag = ALREADY_RUNNING;
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function is callback for Periodic timer and blinks LED periodically.
 * @param [in]  p_args.
 * @retval      None.
 **********************************************************************************************************************/
void periodic_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    static volatile bsp_io_level_t led_level = BSP_IO_LEVEL_HIGH;

    /* Change the state of the LED write value */
    led_level ^= BSP_IO_LEVEL_HIGH;

    /* Change LED state connected to Pmod0 pin 8 */
    /* [0] For Pmod0 pin 8 */
    fsp_err_t err = R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t) g_bsp_leds.p_leds[0], led_level);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        g_error_flag = SET_FLAG;
        APP_ERR_PRINT("\r\nLED_PIN state cannot be toggled");
        return;
    }
}

/***********************************************************************************************************************
 * @brief        This function closes opened GTM module before the project ends up in an Error Trap.
 * @param [IN]   None.
 * @retval       None.
 **********************************************************************************************************************/
void gtm_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close GTM2 module */
    err = R_GTM_Close(&g_timer_one_shot_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GTM2 Close failure message */
        APP_ERR_PRINT("** R_GTM_Close API for channel 2 failed **\r\n");
    }

    /* Close GTM1 module */
    err = R_GTM_Close(&g_timer_periodic_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GTM1 Close failure message */
        APP_ERR_PRINT("** R_GTM_Close API for channel 1 failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup gtm_ep)
 **********************************************************************************************************************/
