#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

/* LED assigned to the core this project is built for. */
#if defined(BSP_CFG_CORE_CR52)
 #if (0 == BSP_CFG_CORE_CR52)
  #define EP_LED_PIN    BSP_IO_PORT_13_PIN_3   /* LED8  (Green)  */
 #else
  #define EP_LED_PIN    BSP_IO_PORT_13_PIN_5   /* LED9  (Green)  */
 #endif
#else
 #if (0 == BSP_CFG_CORE_CA55)
  #define EP_LED_PIN    BSP_IO_PORT_10_PIN_2   /* LED10 (Yellow) */
 #else
  #define EP_LED_PIN    BSP_IO_PORT_10_PIN_3   /* LED11 (Red)    */
 #endif
#endif

#define EP_LED_REGION   BSP_IO_REGION_NOT_SAFE_DEDICATED

/* Blink period: 500 ms on, 500 ms off. */
#define EP_LED_DELAY_MS (500U)

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks the board LED at a rate of 1 second using the software delay function provided by the BSP.
 **********************************************************************************************************************/
void hal_entry(void)
{
    /* Turn the LED off. */
    R_BSP_PinClear(EP_LED_REGION, EP_LED_PIN);

    while (1)
    {
        R_BSP_PinToggle(EP_LED_REGION, EP_LED_PIN);
        R_BSP_SoftwareDelay(EP_LED_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);
    }
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
        /* Pre clock initialization */
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

#if BSP_FEATURE_DDR_SUPPORTED
  #if (1 == BSP_CFG_DDR_INIT_ENABLE)

       /* Initialize the DDR settings. */
       bsp_ddr_init();
 #endif
#endif

        if (NULL != g_bsp_pin_cfg.p_extend)
        {
            /* Configure pins. */
            R_IOPORT_Open (&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
        }
    }
}
