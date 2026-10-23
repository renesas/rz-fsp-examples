/***********************************************************************************************************************
 * File Name    : blinky_thread.c
 * Description  : Blinky thread entry function.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "blinky_thread.h"

extern bsp_leds_t g_bsp_leds;

/*******************************************************************************************************************//**
 * @brief       Blinky Thread entry function. Toggles the board LEDs at a fixed interval; if the board has no LEDs,
 *              traps in an infinite loop.
 * @param[in]   pvParameters    Contains TaskHandle_t (unused)
 * @retval      None
 **********************************************************************************************************************/
void blinky_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);

    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if ((0) == (leds.led_count))
    {
        while (1)
        {
            /* There are no LEDs on this board */
        }
    }

    /* This code uses BSP IO functions to show how it is used.*/
    /* Turn off LEDs */
    for (uint32_t i = 0U; (i) < (leds.led_count); i++)
    {
        /* Clear the pin for this LED. */
        R_BSP_PinClear((bsp_io_region_t) leds.p_leds[i][1], (bsp_io_port_pin_t) leds.p_leds[i][0]);
    }

    while (1)
    {
        /* Toggle board LEDs */
        for (uint32_t i = 0U; (i) < (leds.led_count); i++)
        {
            /* Toggle the pin for this LED. */
            R_BSP_PinToggle((bsp_io_region_t) leds.p_leds[i][1], (bsp_io_port_pin_t) leds.p_leds[i][0]);
        }

        vTaskDelay(configTICK_RATE_HZ);
    }
}
