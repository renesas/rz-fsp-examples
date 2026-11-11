#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

/* LED assigned to the core this project is built for.
 * Source of truth: rz/board/rzt2n_evb/board_leds.c (g_bsp_prv_leds[]). */
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

/* Pin configuration word for the LED.
 *
 * IOPORT_CFG_REGION_NSAFETY is mandatory and was the reason the LED stayed
 * dark.  r_ioport_pin_set() (rz/fsp/src/r_ioport/r_ioport.c) branches on the
 * RSELP bit -- bit 16 of pin_cfg:
 *
 *     if (p_cfg_data->rsel_reg == 1U)   // non-safety
 *     { ... port > BSP_FEATURE_IOPORT_SELECTABLE_PORT_MAX -> R_PORT_NSR ... }
 *     else                              // safety
 *     { R_PORT_SRS->RSELP[port] &= ~(1U << pin_num);
 *       r_ioport_pin_set_safety(R_PORT_SR, ...); }
 *
 * With that bit clear the driver took the safety path and programmed
 * PMC/PM/DRCTL through the safety register block, while R_BSP_PinClear() /
 * R_BSP_PinToggle() with BSP_IO_REGION_NOT_SAFE_DEDICATED write
 * R_PORT_NSR->P.  P13 is a dedicated non-safety port (port 13 >
 * BSP_FEATURE_IOPORT_SELECTABLE_PORT_MAX == 7), so PMC/PM in R_PORT_NSR were
 * never touched and stayed at their reset value -- the pin never became a
 * driven output.  R_IOPORT_Open() still returned FSP_SUCCESS, which is why the
 * program ran happily in the blink loop with a dead LED.
 *
 * IOPORT_CFG_DRIVE_MID is insurance, not part of the fix: the default
 * (IOPORT_CFG_DRIVE_LOW, value 0) is the weakest drive setting.  Drop it if
 * you want to test the region bit in isolation. */
#define EP_LED_PIN_CFG  ((uint32_t) (IOPORT_CFG_REGION_NSAFETY |           \
                                     IOPORT_CFG_PORT_GPIO |               \
                                     IOPORT_CFG_PORT_DIRECTION_OUTPUT |   \
                                     IOPORT_CFG_PORT_OUTPUT_LOW |         \
                                     IOPORT_CFG_DRIVE_MID))

/* The CR52_1 project is generated without pin configuration data
 * (rz_gen/pin_data.c has number_of_pins = 0 and p_extend = NULL, so
 * R_BSP_WarmStart() never opens IOPORT).  Configure the LED explicitly so the
 * application also works when downloaded directly by the debugger, where no
 * boot application has configured the pin first. */
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

/* Debug aid -- inspect these in the Expressions view after R_IOPORT_Open().
 * For LED9 (P13_5) the expected values are:
 *   g_ep_led_pmc_readback bit 5        == 0   (pin is GPIO, not peripheral)
 *   g_ep_led_pm_readback  bits [11:10] == 0b10 (pin direction is output)
 * If PMC bit 5 is 1 or PM is 0b01/0b00, the pin configuration did not reach
 * the non-safety register block. */
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
