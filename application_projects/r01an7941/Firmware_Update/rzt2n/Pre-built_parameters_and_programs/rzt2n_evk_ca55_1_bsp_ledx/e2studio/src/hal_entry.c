#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

/***********************************************************************************************************************
 * Blinky for CA55 **core 1** on the RZ/T2N EVB (FSP 4.2.0).
 *
 * The LED table from the BSP (g_bsp_leds in rz/board/rzt2n_evb/board_leds.c)
 * is deliberately NOT used: board_leds.h does not declare it extern, so
 * referencing it gives "'g_bsp_leds' undeclared" and defining it locally gives
 * "multiple definition of g_bsp_leds".  The pin is taken straight from the
 * board table instead:
 *
 *   CR52 CPU0  -> P13_3  LED8   (Green)
 *   CR52 CPU1  -> P13_5  LED9   (Green)
 *   CA55 Core0 -> P10_2  LED10  (Yellow)
 *   CA55 Core1 -> P10_3  LED11  (Red)     <-- this project
 **********************************************************************************************************************/
#if defined(BSP_CFG_CORE_CA55)
 #if (0 == BSP_CFG_CORE_CA55)
  #define EP_LED_PIN    BSP_IO_PORT_10_PIN_2   /* LED10 (Yellow) */
 #else
  #define EP_LED_PIN    BSP_IO_PORT_10_PIN_3   /* LED11 (Red)    */
 #endif
#else
 #if (0 == BSP_CFG_CORE_CR52)
  #define EP_LED_PIN    BSP_IO_PORT_13_PIN_3   /* LED8  (Green)  */
 #else
  #define EP_LED_PIN    BSP_IO_PORT_13_PIN_5   /* LED9  (Green)  */
 #endif
#endif

#define EP_LED_REGION    BSP_IO_REGION_NOT_SAFE_DEDICATED

/* Blink period: 500 ms on, 500 ms off. */
#define EP_LED_DELAY_MS  (500U)

/* Pin configuration word for the LED.
 *
 * IOPORT_CFG_REGION_NSAFETY is mandatory.  r_ioport_pin_set()
 * (rz/fsp/src/r_ioport/r_ioport.c) branches on the RSELP bit - bit 16 of
 * pin_cfg.  With that bit clear the driver takes the SAFETY path and writes
 * PMC/PM/DRCTL through the safety register block, while
 * R_BSP_PinClear()/R_BSP_PinToggle() with BSP_IO_REGION_NOT_SAFE_DEDICATED
 * write R_PORT_NSR->P.  P10 is a dedicated non-safety port (port 10 >
 * BSP_FEATURE_IOPORT_SELECTABLE_PORT_MAX == 7), so PMC/PM in R_PORT_NSR would
 * keep their reset values, the pin would never drive, and the LED would stay
 * dark - while R_IOPORT_Open() still returns FSP_SUCCESS.
 *
 * These are the exact bits the pin configurator emits for P10_2/P10_3 in the
 * core 0 project (rz_gen/pin_data.c). */
#define EP_LED_PIN_CFG   ((uint32_t) (IOPORT_CFG_REGION_NSAFETY |             \
                                      IOPORT_CFG_PORT_GPIO |                 \
                                      IOPORT_CFG_PORT_DIRECTION_OUTPUT_INPUT |\
                                      IOPORT_CFG_PORT_OUTPUT_LOW))

/* This project is generated without pin configuration data: rz_gen/pin_data.c
 * has number_of_pins = 0 and p_extend = NULL, so the R_IOPORT_Open() call in
 * R_BSP_WarmStart() below never runs.  (The core 0 project DOES have a
 * generated pin table, which is why its hal_entry.c needs no Open call.)
 * Configure the LED explicitly here, so the application works both when the
 * SSBL starts this core and when it is downloaded straight by the debugger. */
extern const ioport_extend_cfg_t g_ioport_cfg_extend;

static const ioport_pin_cfg_t g_ep_led_pin_cfg =
{
    .pin_cfg = EP_LED_PIN_CFG,
    .pin     = EP_LED_PIN,
};

static const ioport_cfg_t g_ep_led_ioport_cfg =
{
    .number_of_pins = 1U,
    .p_pin_cfg_data = &g_ep_led_pin_cfg,
    .p_extend       = &g_ioport_cfg_extend,
};

/* Debug aid - inspect these in the Expressions view after R_IOPORT_Open().
 * For LED11 (P10_3) the expected values are:
 *   g_ep_led_pmc_readback bit 3       == 0    (pin is GPIO, not peripheral)
 *   g_ep_led_pm_readback  bits [7:6]  == 0b11 (output, input buffer enabled)
 * If PMC bit 3 is 1, or PM reads 0b01/0b00, the pin configuration did not
 * reach the non-safety register block. */
volatile uint8_t  g_ep_led_pmc_readback;
volatile uint16_t g_ep_led_pm_readback;

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks the board LED at a rate of 1 second using the software delay function provided by the BSP.
 **********************************************************************************************************************/
void hal_entry(void)
{
    fsp_err_t err = R_IOPORT_Open(&IOPORT_CFG_CTRL, &g_ep_led_ioport_cfg);
    if (FSP_SUCCESS != err)
    {
        while (1)
        {
            ;
        }
    }

    /* Read back what actually landed in the non-safety port registers. */
    g_ep_led_pmc_readback = R_PORT_NSR->PMC[EP_LED_PIN >> 8];
    g_ep_led_pm_readback  = R_PORT_NSR->PM[EP_LED_PIN >> 8];

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

/*******************************************************************************************************************//**
 * This function is called without a stack at the very beginning of the startup process.
 * All instructions must be written in inline assembly.
 **********************************************************************************************************************/
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
{
    /* The very beginning of the startup process. */

    /* Do not delete. Required to return to system_init. */
#if (0 == BSP_LP64_SUPPORT)
    __asm volatile ("BX lr");
#else
    __asm volatile ("BR lr");
#endif
}
