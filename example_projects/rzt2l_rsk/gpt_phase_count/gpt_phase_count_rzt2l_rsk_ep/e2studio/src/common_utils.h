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

#define MODULE_NAME             "r_gpt"

#define EP_INFO     "\nThis sample program demonstrates the use of the General Purpose PWM Timer (GPT)"\
                    "\r\nin phase counting mode on the RZ/T2L microcontroller. It counts pulses from"\
                    "\r\na two-phase encoder (A phase, B phase) and provides RTT Viewer-based commands"\
                    "\r\nto get, set, and reset the count value."\
                    "\r\n"\
                    "\r\nOn user input via RTT Viewer:"\
                    "\r\n    \"g\": Retrieves the current count value from the GPT counter."\
                    "\r\n    \"s\": Allows the user to set a specific count value."\
                    "\r\n         After entering s, the user inputs the desired value."\
                    "\r\n    \"r\": Resets the GPT counter value to zero."\
                    "\r\nAny API/event failure message is also displayed.\r\n\r\n"

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

#define APP_READ(read_data)    (SEGGER_RTT_Read (SEGGER_INDEX, (read_data), sizeof(read_data)))

#define APP_CHECK_DATA         (SEGGER_RTT_HasKey())

#endif /* COMMON_UTILS_H_ */
