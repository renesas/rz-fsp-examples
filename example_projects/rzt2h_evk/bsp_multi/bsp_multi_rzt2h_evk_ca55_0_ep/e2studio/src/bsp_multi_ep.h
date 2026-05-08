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
#define LED0        (BSP_IO_PORT_23_PIN_1)

/* IO region */
#define REGION      (BSP_IO_REGION_NOT_SAFE_DEDICATED)

#define EP_INFO     "\r\nThis example project demonstrates the on-board LEDs using BSP APIs on multi-core"\
                    "\r\nCA55 Core0, CA55 Core1, CA55 Core2, CA55 Core3, CR52 CPU0, and CR52 CPU1."\
                    "\r\nOnce started, each core will toggle each LED periodically using CMT timer."\
                    "\r\nAny API/event failure message is displayed.\r\n\r\n"

#endif /* BSP_MULTI_EP_H_ */
