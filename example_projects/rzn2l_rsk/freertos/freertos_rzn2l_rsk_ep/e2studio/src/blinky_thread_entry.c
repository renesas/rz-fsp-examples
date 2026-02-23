/***********************************************************************************************************************
 * File Name    : blinky_thread_entry.c
 * Description  : Contains data structures and functions used in blinky_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "blinky_thread.h"
#include "common_utils.h"

#define EP_INFO     "\r\nThis example demonstrates LED control using BSP IO functions under FreeRTOS.\n"\
                    "A dedicated Blinky task periodically toggles all on-board LEDs to verify IO operation.\n"\
                    "If no LEDs are available on the target board, the application halts in a safe loop.\n"\
                    "Use this example to confirm basic FreeRTOS scheduling and GPIO toggling functionality.\n"

extern bsp_leds_t g_bsp_leds;

/***********************************************************************************************************************
 * Function Name: blinky_thread_entry
 * Description  : Entry function for Blinky thread.
 *                The function retrieves FSP version, prints example information,
 *                initializes LED pins, and toggles LEDs periodically to indicate
 *                normal operation.
 * Argument     : pvParameters
 * Return Value : None
 **********************************************************************************************************************/
void blinky_thread_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED(pvParameters);
    fsp_pack_version_t version = {RESET_VALUE};

    /* Get the FSP version */
    R_FSP_VersionGet(&version);

    /* Example project information printed on the Console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
    APP_PRINT(EP_INFO);

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

    /* This code uses BSP IO functions to show how it is used.*/
    /* Turn off LEDs */
    for (uint32_t i = 0; i < leds.led_count; i++)
    {
        /* Conversion from LED array value to IO region type */
        R_BSP_PinClear((bsp_io_region_t)leds.p_leds[i][1], (bsp_io_port_pin_t)leds.p_leds[i][0]);
    }

    while (1)
    {
        /* Toggle board LEDs */
        for (uint32_t i = 0; i < leds.led_count; i++)
        {
            /* Conversion from LED array value to IO region type */
            R_BSP_PinToggle((bsp_io_region_t)leds.p_leds[i][1], (bsp_io_port_pin_t)leds.p_leds[i][0]);
        }
        APP_PRINT("LED toggled\r\n");
        vTaskDelay(configTICK_RATE_HZ);
    }
}
