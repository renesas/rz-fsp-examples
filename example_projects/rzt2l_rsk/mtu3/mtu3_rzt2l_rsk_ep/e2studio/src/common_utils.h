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
#include "hal_data.h"

/* SEGGER RTT and error related headers */
#include "SEGGER_RTT/SEGGER_RTT.h"

#define BIT_SHIFT_8             (8u)
#define SIZE_64                 (64u)

#define LVL_ERR                 (1u)       /* Error condition */

#define RESET_VALUE             (0x00)
#define NULL_CHAR               ('\0')

#define EP_VERSION              "4.0.0"

#define MODULE_NAME             "r_mtu3"

#define EP_INFO     "\nThis example project show cases a typical use case of the Multi-Function Timer"\
                    "\r\nUnit 3 (MTU3) on the RZ/T2L microcontroller, demonstrating how to generate and "\
                    "\r\ndynamically adjust PWM signals. The MTU3 module is initialized to produce PWM "\
                    "\r\noutputs with a duty cycle that transitions continuously through the following "\
                    "\r\nsequence: 25 to 50 to 75 then back to 25. Each transition occurs at a fixed "\
                    "\r\ninterval of 2 seconds, allowing for clear observation of the waveform changes."\
                    "\r\n\r\nAny API/event failure message is also displayed.\r\n\r\n"

#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                *"\
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

#define APP_READ(read_data)    (SEGGER_RTT_Read (SEGGER_INDEX, (read_data), sizeof(read_data)))

#define APP_CHECK_DATA         (SEGGER_RTT_HasKey())

#endif /* COMMON_UTILS_H_ */
