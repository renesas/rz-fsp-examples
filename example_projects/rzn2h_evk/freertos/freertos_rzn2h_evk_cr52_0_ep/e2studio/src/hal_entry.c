/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"

/***********************************************************************************************************************
 * @addtogroup freertos_ep
 * @{
 **********************************************************************************************************************/

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

/***********************************************************************************************************************
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

/***********************************************************************************************************************
 * @} (end addtogroup freertos_ep)
 **********************************************************************************************************************/

