/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point for the USB PMSC example.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"

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
 * Initializes the UART console, prints the USB PMSC example overview, and starts the peripheral mass-storage task.
 *
 * @retval None
 **********************************************************************************************************************/
void hal_entry(void)
{
    int console_err;

    __enable_irq();

    console_err = stdio_open();
    if ((int) FSP_SUCCESS != console_err)
    {
        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    fsp_pack_version_t version;
 
    /* Get the FSP version */
    R_FSP_VersionGet(&version);
 
    /* Example project information printed on the console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor,
        version.version_id_b.patch);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example exposes a RAM disk to a USB host using the USB Peripheral Mass Storage Class.\r\n");
    APP_PRINT("The host can enumerate the device as removable storage and access the preformatted RAM disk.\r\n");
    APP_PRINT("[INFO] Starting USB PMSC application.\r\n");
    APP_PRINT("[INFO] Connect the USB device port to a host PC.\r\n");

    main_task();

    while (1)
    {
        __NOP();
    }
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
 * @retval None
 **********************************************************************************************************************/
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
{
#if (0 == BSP_LP64_SUPPORT)
    __asm volatile ("BX lr");
#else
    __asm volatile ("BR lr");
#endif
}
