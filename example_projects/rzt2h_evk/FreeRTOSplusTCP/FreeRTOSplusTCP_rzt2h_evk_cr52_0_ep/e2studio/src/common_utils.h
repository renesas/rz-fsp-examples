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

#define LVL_ERR         (1u)       /* Error condition */

#define RESET_VALUE     (0x00)

#define EP_VERSION      "4.1.0"

#define MODULE_NAME     "rm_FreeRTOSPlusTCP"

#define BANNER_INFO     "\r\n********************************************************************************"\
                        "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                  *"\
                        "\r\n*   Example Project Version %s                                              *"\
                        "\r\n*   Flex Software Pack Version %d.%d.%d                                           *"\
                        "\r\n********************************************************************************"\
                        "\r\nRefer to readme.txt file for more details on Example Project and"\
                        "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define EP_INFO "\r\nThis example project demonstrates Ethernet communication on 3 ports: ETH0, ETH1, and ETH2."\
                "\r\nThe board uses a static IPv4 address (192.168.0.100) and supports the following operations:"\
                "\r\n  1. Ping reply check from host PC"\
                "\r\n  2. UDP echo server test using Packet Sender (Address: 192.168.0.100, Port: 10000, ASCII text)"\
                "\r\n  3. TCP echo server test using Packet Sender (Address: 192.168.0.100, Port: 10000, ASCII text)"\
                "\r\nFor each test, verify both the host-side result and the corresponding logs on the RTT Viewer."\
                "\r\nLink up/down status for each Ethernet port is also monitored and printed on the console.\r\n"

#define SEGGER_INDEX             (0)

#define APP_PRINT(fn_, ...)      (SEGGER_RTT_printf(SEGGER_INDEX, (fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)  if(LVL_ERR)\
        SEGGER_RTT_printf(SEGGER_INDEX, "[ERR] In Function: %s(), %s",__FUNCTION__,(fn_),##__VA_ARGS__);

#define APP_ERR_TRAP(err)        if(err) {\
        SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", err);\
        __asm("BKPT #0\n");} /* Trap upon the error */

#define APP_READ(read_data)      SEGGER_RTT_Read(SEGGER_INDEX, read_data, sizeof(read_data));

#define APP_CHECK_DATA           SEGGER_RTT_HasKey()

#endif /* COMMON_UTILS_H_ */
