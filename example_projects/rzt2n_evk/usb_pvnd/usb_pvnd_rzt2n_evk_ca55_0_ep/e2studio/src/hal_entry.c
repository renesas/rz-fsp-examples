/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point for the USB PVND example.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

void hal_entry(void);
void main_task(void);
int stdio_open(void);

/*******************************************************************************************************************//**
 * @brief Main application entry point.
 *
 * Enables interrupts, initializes the UART console, displays the example project banner, and starts the USB Peripheral
 * Vendor application.
 *
 * @retval None
 **********************************************************************************************************************/
void hal_entry(void)
{
    /* Enable interrupts required by the UART console and USB driver. */
    __enable_irq();

    /* Initialize the UART console before the first print operation. */
    int console_err = stdio_open();
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
    APP_PRINT("This example demonstrates USB Peripheral Vendor communication using control, bulk, and "
              "interrupt transfers.\r\n");
    APP_PRINT("Received bulk and interrupt data is verified and returned to the USB host.\r\n");

    main_task();
}

/*******************************************************************************************************************//**
 * @brief Performs BSP initialization during the warm-start sequence.
 *
 * @param[in] event Current BSP warm-start event.
 *
 * @retval None
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
        /* Reserved for pre-clock initialization. */
    }

    if (BSP_WARM_START_POST_C == event)
    {
#if BSP_FEATURE_DDR_SUPPORTED
 #if (1 == BSP_CFG_DDR_INIT_ENABLE)

        /* Initialize DDR when enabled in the BSP configuration. */
        bsp_ddr_init();

 #endif
#endif

        if (NULL != g_bsp_pin_cfg.p_extend)
        {
            /* Apply the generated pin configuration. */
            R_IOPORT_Open(&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Executes the stackless portion of the BSP warm-start sequence.
 *
 * This function runs at the beginning of startup and must contain only inline assembly instructions.
 *
 * @retval None
 **********************************************************************************************************************/
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
{
    /* Do not delete. Required to return to system_init. */
#if (0 == BSP_LP64_SUPPORT)
    __asm volatile ("BX lr");
#else
    __asm volatile ("BR lr");
#endif
}
