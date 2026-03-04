/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures, and function declarations commonly used in the EP.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

/* Generic headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "hal_data.h"

/* SEGGER RTT and error related headers */
#include "SEGGER_RTT/SEGGER_RTT.h"

#define BIT_SHIFT_8             (8u)
#define SIZE_64                 (64u)

#define LVL_ERR                 (1u)       /* Error condition */

#define RESET_VALUE             (0x00)
#define NULL_CHAR               ('\0')

#define EP_VERSION              "4.0.0"

#define MODULE_NAME             "r_gpt"

#define EP_INFO                 "\r\nThe example project initializes GPT module in Periodic, PWM or One-shot"\
                                "\r\nmode based on user input from the displayed menu options."\
                                "\r\nIn periodic mode, user can enter the time period within the "\
                                "\r\npermitted ranges to change the blink frequency."\
                                "\r\nLD2 and LD3 toggle alternately at the configured period."\
                                "\r\nIn PWM mode, user can enter the duty cycle within the "\
                                "\r\nspecified range to control the PWM output."\
                                "\r\nLD3 is driven with PWM at the configured period and duty cycle."\
                                "\r\nIn One-shot mode, the timer runs once and then expires."\
                                "\r\nOn timer expiry, LD3 turns OFF then ON immediately, while LD2 turns ON then"\
                                "\r\nOFF immediately (opposite states)."\
                                "\r\nStatus and user prompts are displayed on the J-Link RTT Viewer.\r\n"

#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                 *"\
                                "\r\n*   Example Project Version %s                                *"\
                                "\r\n*   Flex Software Pack Version %d.%d.%d                             *"\
                                "\r\n******************************************************************"\
                                "\r\nRefer to readme.txt file for more details on Example Project and"\
                                "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define SEGGER_INDEX            (0)

#define APP_PRINT(fn_, ...)     (SEGGER_RTT_printf(SEGGER_INDEX,(fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)     ({\
                                    if(LVL_ERR){\
                                    SEGGER_RTT_printf(SEGGER_INDEX, "[ERR] In Function: %s(), %s",\
                                                        __FUNCTION__,(fn_),##__VA_ARGS__);\
                                        }\
                                    })

#define APP_ERR_TRAP(err)       ({\
                                if((err)) {\
                                SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", (err));\
                                __asm("BKPT #0\n");}  /* Trap upon the error */\
                                    })

#define APP_READ(read_data)     SEGGER_RTT_Read(SEGGER_INDEX, read_data, sizeof(read_data));

#define APP_CHECK_DATA          SEGGER_RTT_HasKey()

#endif /* COMMON_UTILS_H_ */
