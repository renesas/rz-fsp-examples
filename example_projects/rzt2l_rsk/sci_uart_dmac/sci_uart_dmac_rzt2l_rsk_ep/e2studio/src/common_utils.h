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

#define BIT_SHIFT_8     (8u)
#define SIZE_64         (64u)

#define LVL_ERR         (1u)       /* Error condition */

#define RESET_VALUE     (0x00)
#define NULL_CHAR       ('\0')

#define EP_VERSION      "4.0.0"

#define MODULE_NAME     "r_sci_uart"

#define EP_INFO         "\nThis sample program demonstrates how to use the UART module to send and receive "\
                        "\r\nmessages via the terminal on the RZ/T2L micro controller. The project initializes"\
                        "\r\nthe UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate "\
                        "\r\nvalue and transmits a 'Hello World' message to the terminal. It then enters a "\
                        "\r\nreceive loop, waiting for user input. Any data received from the terminal is "\
                        "\r\nimmediately echoed back.\r\n"\
                        "\r\nAny API or event failure message is displayed via RTT Viewer.\r\n\r\n"

#define BANNER_INFO     "\r\n******************************************************************"\
                        "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module            *"\
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
