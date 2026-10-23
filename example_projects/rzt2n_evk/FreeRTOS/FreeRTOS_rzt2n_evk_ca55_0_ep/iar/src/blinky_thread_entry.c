/***********************************************************************************************************************
 * File Name    : blinky_thread_entry.c
 * Description  : Contains the FreeRTOS LED blinky task entry function.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "blinky_thread.h"
#include "common_utils.h"

extern bsp_leds_t g_bsp_leds;

int stdio_open(void);
void blinky_thread_entry(void * pvParameters);

/*******************************************************************************************************************//**
 * @brief FreeRTOS blinky task entry function.
 *
 * Initializes the UART console, clears all board LEDs, and toggles each available LED once per second.
 *
 * @param[in] pvParameters FreeRTOS task parameter.
 *
 * @retval None
 **********************************************************************************************************************/
void blinky_thread_entry(void * pvParameters)
{
    bsp_leds_t leds;
    int        console_err;

    FSP_PARAMETER_NOT_USED(pvParameters);

    /* Initialize the UART console after the FreeRTOS scheduler has started. */
    console_err = stdio_open();
    if ((int) FSP_SUCCESS != console_err)
    {
        while (1)
        {
            __NOP();
        }
    }

    APP_PRINT(BANNER_INFO,
              EP_VERSION,
              4, 2, 0);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates a FreeRTOS task that toggles all available board LEDs once per second.\r\n");

    leds = g_bsp_leds;

    if (0U == leds.led_count)
    {
        APP_PRINT("[ERROR] No board LEDs are available for the FreeRTOS blinky example.\r\n");

        while (1)
        {
            __NOP();
        }
    }

    APP_PRINT("[FreeRTOS] Blinky task started with %lu LED(s).\r\n",
              (unsigned long) leds.led_count);

    /* Turn off all LEDs before starting the blink loop. */
    for (uint32_t i = 0U; i < leds.led_count; i++)
    {
        R_BSP_PinClear((bsp_io_region_t) leds.p_leds[i][1],
                       (bsp_io_port_pin_t) leds.p_leds[i][0]);
    }

    while (1)
    {
        /* Toggle all board LEDs. */
        for (uint32_t i = 0U; i < leds.led_count; i++)
        {
            R_BSP_PinToggle((bsp_io_region_t) leds.p_leds[i][1],
                            (bsp_io_port_pin_t) leds.p_leds[i][0]);
        }

        vTaskDelay(configTICK_RATE_HZ);
    }
}
