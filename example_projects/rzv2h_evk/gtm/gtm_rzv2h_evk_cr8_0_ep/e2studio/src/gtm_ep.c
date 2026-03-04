/***********************************************************************************************************************
 * File Name    : gtm_ep.c
 * Description  : Contains data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "gtm_ep.h"

/***********************************************************************************************************************
 * @addtogroup gtm_ep
 * @{
 **********************************************************************************************************************/
extern volatile uint8_t  g_periodic_timer_flag;
extern volatile uint8_t  g_one_shot_timer_flag;

extern volatile uint32_t g_error_flag;

/***********************************************************************************************************************
 * @fn          gtm_init
 * @brief       This functions initializes GTM module.
 * @param[IN]   None
 * @retval      FSP_SUCCESS                  Upon successful open of GTM module
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 **********************************************************************************************************************/
fsp_err_t gtm_init(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Open Timer2 in One Shot Mode */
    err = R_GTM_Open(&g_timer_one_shot_ctrl, &g_timer_one_shot_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("\r\nGTM2 timer open failed.\r\nRestart the Application\r\n");
        return err;
    }

    /* Open Timer1 in Periodic mode */
    err = R_GTM_Open(&g_timer_periodic_ctrl, &g_timer_periodic_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Close Timer2 in One Shot Mode */
        if ((R_GTM_Close(&g_timer_one_shot_ctrl) != FSP_SUCCESS))
        {
            APP_ERR_PRINT ("\r\nOne shot timer close failed.\r\nRestart the Application\r\n");
        }
        APP_ERR_PRINT ("\r\nGTM1 timer open failed.\r\nRestart the Application\r\n");
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: gtm_init
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          gtm_start_oneshot_timer
 * @brief       This functions starts GTM2 in one-shot mode.
 * @param[IN]   None
 * @retval      FSP_SUCCESS                  Timer started successfully
 * @retval      Any Other Error code apart from FSP_SUCCESS  
 **********************************************************************************************************************/
fsp_err_t gtm_start_oneshot_timer(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Start Timer2 in one-shot mode */
    err = R_GTM_Start(&g_timer_one_shot_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\n GTM2 timer start failed.\r\n");
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: gtm_start_oneshot_timer
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn        one_shot_timer_callback
 * @brief     This function is callback for one-shot mode timer and starts GTM1 timer in Periodic mode.
 * @param[in] (timer_callback_args_t *)
 **********************************************************************************************************************/
void one_shot_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    fsp_err_t err = FSP_SUCCESS;

    timer_status_t periodic_timer_status;

    /* Retrieve the status of timer running in periodic mode */
    err = R_GTM_StatusGet(&g_timer_periodic_ctrl, &periodic_timer_status);
    if (FSP_SUCCESS != err)
    {
        g_error_flag = SET_FLAG;
    }
    else
    {
        if (TIMER_STATE_COUNTING != periodic_timer_status.state)
        {
            /* Start the timer in periodic mode only if the timer is in stopped state */
            err = R_GTM_Start(&g_timer_periodic_ctrl);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                g_error_flag = SET_FLAG;
                APP_ERR_PRINT("\r\nGTM1 Cannot be started in Periodic mode");
            }
            else
            {
                g_periodic_timer_flag = SET_FLAG;   /* Set the flag since timer1 is started in periodic mode */
            }
        }
        else
        {
            g_periodic_timer_flag = ALREADY_RUNNING;
        }
    }
}
/***********************************************************************************************************************
 * End of function: one_shot_timer_callback
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn        periodic_timer_callback
 * @brief     This function is callback for periodic timer and blinks LED periodically.
 * @param[in] (timer_callback_args_t *)
 **********************************************************************************************************************/
void periodic_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    static volatile bsp_io_level_t s_led_level = BSP_IO_LEVEL_HIGH;

    /* Change the state of the LED write value */
    s_led_level ^= BSP_IO_LEVEL_HIGH;

    /* Change LED state connected to Pmod Type1/1a pin 7 on Expansion Board */
    fsp_err_t err = R_IOPORT_PinWrite(&g_ioport_ctrl, PIN_LED, s_led_level);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        g_error_flag = SET_FLAG;
        APP_ERR_PRINT("\r\nLED_PIN state cannot be toggled");
        return;
    }
}
/***********************************************************************************************************************
 * End of function: periodic_timer_callback
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          gtm_deinit
 * @brief       This function closes opened GTM module before the project ends up in an Error Trap.
 * @param[IN]   None
 * @retval      None
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
        APP_ERR_PRINT("** R_GTM_Close API for channel 2 failed **  \r\n");
    }

    /* Close GTM1 module */
    err = R_GTM_Close(&g_timer_periodic_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GTM1 Close failure message */
        APP_ERR_PRINT("** R_GTM_Close API for channel 1 failed **  \r\n");
    }
}
/***********************************************************************************************************************
 * End of function: gtm_deinit
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn        timer_status_check
 * @brief     This function checks the status of both timers and starts/stops the timers.
 * @param[in] None
 **********************************************************************************************************************/
fsp_err_t timer_status_check(void)
{
    fsp_err_t err = FSP_SUCCESS;

    timer_status_t periodic_timer_status =
    {
        .counter = RESET_VALUE,
        .state = (timer_state_t) RESET_VALUE,   /* cast to timer_state_t */
    };
    timer_status_t oneshot_timer_status =
    {
        .counter = RESET_VALUE,
        .state = (timer_state_t) RESET_VALUE,   /* cast to timer_state_t */
    };

    /* Retrieve the status of timer running in periodic mode */
    err = R_GTM_StatusGet(&g_timer_periodic_ctrl, &periodic_timer_status);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nStatusGet API failed");
        return err;
    }

    /* Retrieve the status of timer running in one-shot mode */
    err = R_GTM_StatusGet(&g_timer_one_shot_ctrl, &oneshot_timer_status);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nStatusGet API failed");
        return err;
    }

    if (TIMER_STATE_STOPPED != oneshot_timer_status.state)
    {
        err = R_GTM_Stop(&g_timer_one_shot_ctrl);
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("\r\nGTM Stop API failed");
            return err;
        }
        else
        {
            APP_PRINT("\r\nOne-shot timer stopped. "
                    "Enter any key to start timers.\r\n");
        }
    }
    else if (TIMER_STATE_STOPPED != periodic_timer_status.state)
    {
        err = R_GTM_Stop(&g_timer_periodic_ctrl);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\nGTM Stop API failed");
            return err;
        }
        else
        {
            APP_PRINT("\r\nPeriodic timer stopped. "
                    "Enter any key to start timers.\r\n");
        }
    }
    else
    {
        err = gtm_start_oneshot_timer();

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("\r\n GTM start failed");
            return err;
        }
        else
        {
            g_one_shot_timer_flag = SET_FLAG;        /* Set Timer Flag as timer is started */
        }
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: timer_status_check
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @} (end addtogroup gtm_ep)
 **********************************************************************************************************************/
