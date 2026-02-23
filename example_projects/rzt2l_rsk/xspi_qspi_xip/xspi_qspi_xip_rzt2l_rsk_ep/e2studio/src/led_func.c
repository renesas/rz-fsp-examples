/***********************************************************************************************************************
 * File Name    : led_func.c
 * Description  : Contains data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"

void LED_blinky (void);

extern bsp_leds_t g_bsp_leds;

BSP_PLACE_IN_SECTION(".qspi_flash")

/***********************************************************************************************************************
 * @brief     Blinky LED routine executed from external Serial Flash (XiP).
 * @details   This routine is placed in the `.qspi_flash` section (via BSP_PLACE_IN_SECTION) so that instructions
 *            are fetched from Serial Flash in XiP mode. It toggles BSP LEDs on the active core at ~2 Hz for a
 *            fixed number of cycles using R_BSP_PinToggle() and a software delay.
 * @param[in] None
 * @return    None
 **********************************************************************************************************************/
void LED_blinky (void)
{
    /* Define the units to be used with the software delay function */
    const bsp_delay_units_t bsp_delay_units = BSP_DELAY_UNITS_MILLISECONDS;

    /* Set the blink frequency (must be <= bsp_delay_units */
    const uint32_t freq_in_hz = 2;

    /* Calculate the delay in terms of bsp_delay_units */
    const uint32_t delay = bsp_delay_units / freq_in_hz;

    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          /* There are no LEDs on this board */
        }
    }

    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED0][0]);  /* cast to bsp_io_port_pin_t*/
    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED1][0]);  /* cast to bsp_io_port_pin_t*/
    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED2][0]);  /* cast to bsp_io_port_pin_t*/
    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED3][0]);  /* cast to bsp_io_port_pin_t*/
    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED4][0]);  /* cast to bsp_io_port_pin_t*/
    R_BSP_PinClear(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED5][0]);  /* cast to bsp_io_port_pin_t*/

    uint8_t blink_count = 0;
    while (blink_count++ < 21) /* Blink 10 times */
    {
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED0][0]);  /* cast to bsp_io_port_pin_t*/
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED1][0]);  /* cast to bsp_io_port_pin_t*/
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED2][0]);  /* cast to bsp_io_port_pin_t*/
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED3][0]);  /* cast to bsp_io_port_pin_t*/
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED4][0]);  /* cast to bsp_io_port_pin_t*/
        R_BSP_PinToggle(BSP_IO_REGION_SAFE, (bsp_io_port_pin_t) leds.p_leds[BSP_LED_RLED5][0]);  /* cast to bsp_io_port_pin_t*/
/* Delay */
        R_BSP_SoftwareDelay(delay, bsp_delay_units);
    }
}
