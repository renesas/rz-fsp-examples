/***********************************************************************************************************************
 * File Name    : gtm_ep.c
 * Description  : Contains data structures and functions used in gtm_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "gtm_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_gtm_ep
 * @{
 **********************************************************************************************************************/

/* Variable to store BSP LED information */
extern bsp_leds_t g_bsp_leds;

/* Flag to indicate timer event */
extern volatile uint8_t g_periodic_timer_flag;
extern volatile uint8_t g_one_shot_timer_flag;

/* Flag to indicate error condition */
extern volatile uint32_t g_error_flag;

/***********************************************************************************************************************
 * @brief       This function initializes GTM module.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful open of GTM module.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t gtm_init(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Open Timer0 in One Shot Mode */
    err = R_GTM_Open(&g_timer_one_shot_ctrl, &g_timer_one_shot_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("GTM0 timer open failed.\r\nRestart the Application\r\n\r\n");
        return err;
    }

    /* Open Timer1 in Periodic mode */
    err = R_GTM_Open(&g_timer_periodic_ctrl, &g_timer_periodic_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Close Timer0 in One Shot Mode */
        if ((FSP_SUCCESS != R_GTM_Close(&g_timer_one_shot_ctrl)))
        {
            APP_ERR_PRINT("One shot timer close failed.\r\nRestart the Application\r\n\r\n");
        }
        APP_ERR_PRINT("GTM1 timer open failed.\r\nRestart the Application\r\n\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function starts GTM0 in one-shot mode.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Timer started successfully.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful starts.
 **********************************************************************************************************************/
fsp_err_t gtm_start_oneshot_timer(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Start Timer0 in one-shot mode */
    err = R_GTM_Start(&g_timer_one_shot_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_GTM_Start API failed. **\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       User callback function for one-shot mode timer and starts GTM1 timer in Periodic mode.
 * @param[in]   p_args     Callback arguments.
 * @retval      None.
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
                APP_ERR_PRINT("** R_GTM_Start API failed in Periodic mode **\r\n");
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
 * @brief       User callback function for periodic timer and blinks LED periodically.
 * @param[in]   p_args     Callback arguments.
 * @retval      None.
 **********************************************************************************************************************/
void periodic_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    static volatile bsp_io_level_t led_level = BSP_IO_LEVEL_HIGH;

    /* If this board has no LEDs then trap here */
    if (LED_COUNT_ZERO == g_bsp_leds.led_count)
    {
        g_error_flag = SET_FLAG;
        APP_ERR_PRINT("Error: No Led's Found on the board\r\n\r\n");
        return;
    }

    /* Change the state of the LED write value */
    led_level ^= BSP_IO_LEVEL_HIGH;

    /* Change LED state */
    fsp_err_t err = R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t) g_bsp_leds.p_leds[2], led_level);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        g_error_flag = SET_FLAG;
        APP_ERR_PRINT("** R_IOPORT_PinWrite API to toggle LED failed **\r\n");
        return;
    }
}

/***********************************************************************************************************************
 * @brief       This function closes opened GTM module before the project ends up in an Error Trap.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void gtm_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close GTM0 module */
    err = R_GTM_Close(&g_timer_one_shot_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GTM0 Close failure message */
        APP_ERR_PRINT("** R_GTM_Close API for channel 0 failed **\r\n");
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
 * @brief This function checks the status of both timers and starts/stops the timers.
 *
 * @param[in] None.
 **********************************************************************************************************************/
fsp_err_t timer_status_check(void)
{
    fsp_err_t err = FSP_SUCCESS;
    timer_status_t periodic_timer_status =
    {
        .counter = RESET_VALUE,
        .state   = (timer_state_t) RESET_VALUE,
    };

    timer_status_t oneshot_timer_status =
    {
        .counter = RESET_VALUE,
        .state   = (timer_state_t) RESET_VALUE,
    };

    /* Retrieve the status of timer running in periodic mode */
    err = R_GTM_StatusGet(&g_timer_periodic_ctrl, &periodic_timer_status);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_GTM_StatusGet API failed **\r\n");
        return err;
    }

    /* Retrieve the status of timer running in one-shot mode */
    err = R_GTM_StatusGet(&g_timer_one_shot_ctrl, &oneshot_timer_status);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_GTM_StatusGet API failed **\r\n");
        return err;
    }

    if (TIMER_STATE_STOPPED != oneshot_timer_status.state)
    {
        err = R_GTM_Stop(&g_timer_one_shot_ctrl);
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("GTM Stop API failed\r\n\r\n");
            return err;
        }
        else
        {
            APP_PRINT("One-shot timer stopped.\r\n"
                      "Enter any key to start timers.\r\n\r\n");
        }
    }
    else if (TIMER_STATE_STOPPED != periodic_timer_status.state)
    {
        err = R_GTM_Stop(&g_timer_periodic_ctrl);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_GTM_Stop API failed **\r\n");
            return err;
        }
        else
        {
            APP_PRINT("Periodic timer stopped.\r\n"
                      "Enter any key to start timers.\r\n\r\n");
        }
    }
    else
    {
        err = gtm_start_oneshot_timer();

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** gtm_start_oneshot_timer failed **\r\n");
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
 * @} (end addtogroup r_gtm_ep)
 **********************************************************************************************************************/
