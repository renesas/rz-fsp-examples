/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point for the USB HMSC example.
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
void usb_hmsc_baremetal_example(void);
void SVC_Handler(void);

void hal_entry(void)
{
    __enable_irq();
    usb_hmsc_baremetal_example();

    while (1)
    {
        __NOP();
    }
}

void SVC_Handler(void)
{
    __asm volatile (
        "subs PC, LR, 0 \n"
        ::: "memory");
}

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

        bsp_ddr_init();

 #endif
#endif

        if (NULL != g_bsp_pin_cfg.p_extend)
        {
            R_IOPORT_Open(&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
        }
    }
}

BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
{
#if (0 == BSP_LP64_SUPPORT)
    __asm volatile ("BX lr");
#else
    __asm volatile ("BR lr");
#endif
}
