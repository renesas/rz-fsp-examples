/***********************************************************************************************************************
 * File Name    : bsp_multi_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef BSP_MULTI_EP_H_
#define BSP_MULTI_EP_H_

/* User LED */
#define LED3        (BSP_IO_PORT_31_PIN_6)

/* IO region */
#define REGION      (BSP_IO_REGION_NOT_SAFE_DEDICATED)

#define EP_INFO     "\r\nThis example project initializes and controls the on-board LEDs using BSP APIs"\
                    "\r\non Renesas RZ MPU multi-core CA55 Core0 to Core3, CR52 CPU0, and CR52 CPU1."\
                    "\r\nOnce started, each core will toggle each LED periodically using CMT timer."\
                    "\r\nAny API/event failure message is displayed on Terminal Emulator.\r\n\r\n"

#endif /* BSP_MULTI_EP_H_ */
