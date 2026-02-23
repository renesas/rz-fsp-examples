/***********************************************************************************************************************
 * File Name    : timer_pwm.c
 * Description  : Contains data structures and functions used in timer_pwm.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "timer_pwm.h"
#include "uart_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_scif_uart_ep
 * @{
 **********************************************************************************************************************/
extern bsp_leds_t g_bsp_leds;

uint32_t raw_counts_periodic = RESET_VALUE;
uint32_t clock_freq          = RESET_VALUE;

/***********************************************************************************************************************
 * @fn          gtm_initialize
 * @brief       This function initializes the GTM in periodic mode.
 * @param[in]   None.
 * @retval      FSP_SUCCESS    Upon successful open of timer.
 * @retval      Any other error code apart from FSP_SUCCESS    Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t gtm_initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open GTM module */
    err = R_GTM_Open (&g_timer_ctrl, &g_timer_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: gtm_initialize
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          gtm_start
 * @brief       This function starts the GTM in periodic mode.
 * @param[in]   None.
 * @retval      FSP_SUCCESS    Upon successful start of timer.
 * @retval      Any other error code apart from FSP_SUCCESS    Unsuccessful start.
 **********************************************************************************************************************/
fsp_err_t gtm_start(void)
{

    fsp_err_t err = FSP_SUCCESS;

    /* Start GTM module */
    err = R_GTM_Start (&g_timer_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: gtm_start
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn        set_cycle
 * @brief     This function sets the GTM cycle.
 * @param[in] raw_count        Raw count value for cycle generation.
 * @retval    FSP_SUCCESS      Upon Period set returns Successful.
 * @retval    Any other error code apart from FSP_SUCCESS  Unsuccessful in setting Period.
 **********************************************************************************************************************/
fsp_err_t set_cycle(uint32_t raw_count)
{
    fsp_err_t err     = FSP_SUCCESS;

    /* Cast to timer_direction_t */
    timer_info_t info = {(timer_direction_t)RESET_VALUE, RESET_VALUE, RESET_VALUE};

    /* Get the current cycle setting. */
    err = R_GTM_InfoGet(&g_timer_ctrl, &info);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
    else
    {
        /* Depending on the user selected clock source, the raw counts value can be calculated
         * for the user given time-cycle values */
        clock_freq = info.clock_frequency;

        raw_counts_periodic = (uint32_t)(raw_count *(clock_freq / 1000)); /* Cast to uint32_t */

        /* Period Set API set the desired cycle on the on-board LED */
        err = R_GTM_PeriodSet(&g_timer_ctrl, raw_counts_periodic);
        if (FSP_SUCCESS != err)
        {
            APP_ERR_TRAP(err);
        }
    }
    return err;
}
/***********************************************************************************************************************
 * End of function: set_cycle
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          timer_gtm_deinit
 * @brief       This function closes the GTM HAL driver before the project enters an error trap.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
void timer_gtm_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close the GTM module */
    err = R_GTM_Close (&g_timer_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
}
/***********************************************************************************************************************
 * End of function: timer_gtm_deinit
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @fn          periodic_timer_callback
 * @brief       This function is callback for periodic timer and blinks LED periodically.
 * @param[in]   p_args    Callback arguments.
 **********************************************************************************************************************/
void periodic_timer_callback(timer_callback_args_t *p_args)
{
    (void) p_args;
    static volatile bsp_io_level_t s_led_level = BSP_IO_LEVEL_HIGH;

    /* Change the state of the LED write value */
    s_led_level ^= BSP_IO_LEVEL_HIGH;

    /* Change LED state */
    fsp_err_t err = R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t) g_bsp_leds.p_leds[1], s_led_level);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_TRAP(err);
    }
}
/***********************************************************************************************************************
 * End of function: periodic_timer_callback
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @} (end addtogroup r_scif_uart_ep)
 **********************************************************************************************************************/
