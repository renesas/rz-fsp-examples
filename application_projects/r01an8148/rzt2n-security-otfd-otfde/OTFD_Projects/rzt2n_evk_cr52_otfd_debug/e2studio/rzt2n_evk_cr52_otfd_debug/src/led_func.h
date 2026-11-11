/***********************************************************************************************************************
 * File Name    : led_function.h
 * Description  : Contains data structure and function declarations used in led_function.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include <stdint.h>

#ifndef LED_FUNC_H_
#define LED_FUNC_H_

#include "bsp_api.h"

extern const bsp_leds_t g_bsp_leds;
void LED_blinky(void);

#endif /* LED_FUNC_H_ */
