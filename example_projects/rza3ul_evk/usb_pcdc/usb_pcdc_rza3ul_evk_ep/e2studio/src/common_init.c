/***********************************************************************************************************************
 * File Name    : common_init.c
 * Description  : Contains data structures and functions used in common_init.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "common_init.h"
#include "board_cfg.h"

/***********************************************************************************************************************
 * @addtogroup usb_pcdc_ep
 * @{
 **********************************************************************************************************************/

#define NUM_RATES   ((sizeof(blink_rates) / sizeof(blink_rates[0])))

int g_curr_led_freq = BLINK_FREQ_1HZ;
static uint32_t cur_rate = 0;
int curr_led_freq = 0;
uint32_t blink_rates[3] = {BLINK_FREQ_1HZ, BLINK_FREQ_5HZ, BLINK_FREQ_10HZ};

/***********************************************************************************************************************
 *  @brief      External IRQ callback function.
 *  @param[in]  p_args      Callback argument.
 *  @retval     None.
 **********************************************************************************************************************/
void button_irq_callback(external_irq_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    if (++cur_rate > (NUM_RATES - 1))
    {
        cur_rate = 0;
    }

    R_GTM_PeriodSet(g_blinker.p_ctrl, blink_rates[cur_rate]);

    g_curr_led_freq = (int)blink_rates[cur_rate];
}

/***********************************************************************************************************************
 *  @brief      This function initializes External IRQ module.
 *  @param[in]  None.
 *  @retval     FSP_SUCCESS     Successful operation.
 *  @retval     Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t irq_initialize(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = R_INTC_IRQ_ExternalIrqOpen(g_external_irq.p_ctrl, g_external_irq.p_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    fsp_err = R_INTC_IRQ_ExternalIrqEnable(g_external_irq.p_ctrl);
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    return fsp_err;
}

/***********************************************************************************************************************
 *  @brief      Periodic timer callback function.
 *  @param[in]  p_args      Callback argument.
 *  @retval     None.
 **********************************************************************************************************************/
void periodic_timer_callback(timer_callback_args_t *p_args)
{
    /* Hold level to set for pins */
    static bsp_io_level_t pin_level = BSP_IO_LEVEL_LOW;

    if (TIMER_EVENT_CYCLE_END == p_args->event)
    {
        /* Update all board LEDs */
        for (uint32_t i = 0; i < g_bsp_leds.led_count; i++)
        {
            /* Get pin to toggle */
            uint32_t pin = g_bsp_leds.p_leds[i];

            /* Write to this pin */
            R_IOPORT_PinWrite(&g_ioport_ctrl, (bsp_io_port_pin_t) pin, pin_level);
        }
    }

    /* Toggle level for next write */
    if (BSP_IO_LEVEL_LOW == pin_level)
    {
        pin_level = BSP_IO_LEVEL_HIGH;
    }
    else
    {
        pin_level = BSP_IO_LEVEL_LOW;
    }
}

/***********************************************************************************************************************
 *  @brief      This function initializes GTM timer.
 *  @param[in]  None.
 *  @retval     FSP_SUCCESS     Successful initialization.
 *  @retval     Any other error code apart from FSP_SUCCESS     Unsuccessful initialization.
 **********************************************************************************************************************/
static fsp_err_t gtm_initialize(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = R_GTM_Open(g_blinker.p_ctrl, g_blinker.p_cfg);
    {
        if (FSP_SUCCESS != fsp_err)
        {
            return fsp_err;
        }
    }
    fsp_err = R_GTM_PeriodSet(g_blinker.p_ctrl, blink_rates[cur_rate]);
    {
        if (FSP_SUCCESS != fsp_err)
        {
            return fsp_err;
        }
    }

    fsp_err = R_GTM_Start(g_blinker.p_ctrl);
    if (FSP_SUCCESS != fsp_err)
    {
        /* Turn ON LED to indicate fatal error */
        turn_led_error();

        /* Close the GTM timer */
        R_GTM_Close(g_blinker.p_ctrl);

        return fsp_err;
    }

    return fsp_err;
}

/***********************************************************************************************************************
 *  @brief      This function initializes External IRQ and GTM timer modules.
 *  @param[in]  None.
 *  @retval     FSP_SUCCESS     Successful operation.
 *  @retval     Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t common_init(void)
{
    fsp_err_t fsp_err = FSP_SUCCESS;

    fsp_err = gtm_initialize();
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    fsp_err = irq_initialize();
    if (FSP_SUCCESS != fsp_err)
    {
        return fsp_err;
    }

    return fsp_err;
}

/***********************************************************************************************************************
 *  @brief      This function turns ON LED1 and LED3 to indicate the error.
 *  @param[in]  None.
 *  @retval     None.
 **********************************************************************************************************************/
void turn_led_error(void)
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, g_bsp_leds.p_leds[BSP_LED_LED1], BSP_IO_LEVEL_HIGH);
    R_IOPORT_PinWrite(&g_ioport_ctrl, g_bsp_leds.p_leds[BSP_LED_LED2], BSP_IO_LEVEL_LOW);
    R_IOPORT_PinWrite(&g_ioport_ctrl, g_bsp_leds.p_leds[BSP_LED_LED3], BSP_IO_LEVEL_HIGH);
    R_IOPORT_PinWrite(&g_ioport_ctrl, g_bsp_leds.p_leds[BSP_LED_LED4], BSP_IO_LEVEL_LOW);
}

/***********************************************************************************************************************
 * @} (end addtogroup usb_pcdc_ep)
 **********************************************************************************************************************/
