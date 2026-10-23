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

extern uint32_t app_read (unsigned char * p_data);
extern int      app_check_data ();

#define LVL_ERR                 (1u)       /* Error condition */

#define RESET_VALUE             (0x00)
#define NULL_CHAR               ('\0')

#define EP_VERSION              "1.0.0"

#define MODULE_NAME             "r_gpt"

#define EP_INFO                 "\r\nThe example project initializes GPT module in Periodic, PWM or One-shot"\
                                "\r\nmode based on user input from the displayed menu options."\
                                "\r\nIn periodic mode, user can enter the time period within the "\
                                "\r\npermitted ranges to change the blink frequency."\
                                "\r\nLD0 and LD3 toggle alternately at the configured period."\
                                "\r\nIn PWM mode, user can enter the duty cycle within the "\
                                "\r\nspecified range to control the PWM output."\
                                "\r\nLD0 is driven with PWM at the configured period and duty cycle."\
                                "\r\nIn One-shot mode, the timer runs once and then expires."\
                                "\r\nOn timer expiry, LD3 turns OFF then ON immediately, while LD0 turns ON then"\
                                "\r\nOFF immediately (opposite states)."\
                                "\r\nStatus and user prompts are displayed on the Terminal Emulator.\r\n"

#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                 *"\
                                "\r\n*   Example Project Version %s                                *"\
                                "\r\n*   Flex Software Pack Version %d.%d.%d                             *"\
                                "\r\n******************************************************************"\
                                "\r\nRefer to readme.txt file for more details on Example Project and"\
                                "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"


#define APP_PRINT(fn_, ...)      (printf((fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)  if(LVL_ERR)\
        printf("[ERR] In Function: %s",(fn_),##__VA_ARGS__);

#define APP_ERR_TRAP(err)        if(err) {\
        printf("\r\nReturned Error Code: 0x%x  \r\n", err);\
        __asm__ volatile("brk 0x0\n");} /* Trap upon the error  */

#define APP_READ(read_data)     (app_read(read_data))

#define APP_CHECK_DATA          (app_check_data())

#endif /* COMMON_UTILS_H_ */
