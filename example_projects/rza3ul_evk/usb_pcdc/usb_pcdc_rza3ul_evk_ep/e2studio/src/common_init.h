/***********************************************************************************************************************
 * File Name    : common_init.h
 * Description  : Contains macros, data structures, and function declarations commonly used in common_init.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "hal_data.h"

#define BLINK_FREQ_1HZ              (49999999)
#define BLINK_FREQ_5HZ              (9999999)
#define BLINK_FREQ_10HZ             (4999999)

#define KIT_INFO                    ('1')
#define NEXT_STEPS                  ('2')
#define CARRIAGE_RETURN             ('\r')

#define NUM_STRING_DESCRIPTOR       (7U)

extern bsp_leds_t g_bsp_leds;

/* Application printf */
#define APP_PRINT(fn_, ...)         (printf((fn_), ##__VA_ARGS__))

/* Application error */
#define APP_ERR_PRINT(fn_, ...)     (printf(("[ERR] In Function: %s"), (fn_), ##__VA_ARGS__))

/* Application error trap */
#define APP_ERR_TRAP(err)           ({\
                                    if (err)\
                                    {\
                                        printf("\r\nReturned Error Code: 0x%x\r\n", err);\
                                        __asm__ volatile("brk 0x0\n"); /* Trap upon the error */\
                                    }\
                                    })

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
fsp_err_t common_init (void);
void turn_led_error (void);
