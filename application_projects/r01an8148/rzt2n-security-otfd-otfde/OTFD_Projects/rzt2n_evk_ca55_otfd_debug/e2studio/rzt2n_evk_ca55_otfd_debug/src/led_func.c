/***********************************************************************************************************************
 * File Name    : led_function.c
 * Description  : Contains data structures and functions used in led_function.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "hal_data.h"
#include "led_func.h"

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
void LED_blinky(void) BSP_PLACE_IN_SECTION(".flash_app");
void LED_blinky (void);

/******************************************************************************
 * @fn       LED_blinky
 * @brief    Blinky function executed from serial flash.
 ******************************************************************************/
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
            ;                          // There are no LEDs on this board
        }
    }
    
    /* This code uses BSP IO functions to show how it is used.*/
    /* Turn off LEDs */
    for (uint32_t i = 0; i < leds.led_count; i++)
    {
        R_BSP_PinClear((bsp_io_region_t) leds.p_leds[i][1], (bsp_io_port_pin_t) leds.p_leds[i][0]);
    }
    
    uint8_t blink_count = 0;
    while (blink_count++ < 21) // Blink 10 times
    {
        /* Toggle board LEDs */
        for (uint32_t i = 0; i < leds.led_count; i++)
        {
            R_BSP_PinToggle((bsp_io_region_t) leds.p_leds[i][1], (bsp_io_port_pin_t) leds.p_leds[i][0]);
        }
        
        /* Delay */
        R_BSP_SoftwareDelay(delay, bsp_delay_units);
    }
}

