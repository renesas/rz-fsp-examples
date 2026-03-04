/*****************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures and functions declarations used  common to the EP
 ****************************************************************************************************/
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
#define NULL_CHAR       ('\0')

#define EP_VERSION      "4.0.0"
#define MODULE_NAME     "r_elc"

#define EP_INFO         "\r\nThe example project demonstrates the typical use of the ELC HAL module APIs."\
                        "\r\nInput edge detection of single input port 0 and GPT9 events are linked using ELC."\
                        "\r\nThe start source for GPT9 is  ELC Single Port 0 Event. GPT9 runs in Periodic mode."\
                        "\r\nOn giving valid RTT input, a high level from single port triggers timer"\
                        " to start counting."\
                        "\r\nCounter value will be displayed on RTT viewer.\r\n"

#define BANNER_INFO     "\r\n******************************************************************"\
                        "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                 *"\
                        "\r\n*   Example Project Version %s                                *"\
                        "\r\n*   Flex Software Pack Version %d.%d.%d                             *"\
                        "\r\n******************************************************************"\
                        "\r\nRefer to readme.txt file for more details on Example Project and"\
                        "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define SEGGER_INDEX            (0)

#define APP_PRINT(fn_, ...)     (SEGGER_RTT_printf (SEGGER_INDEX,(fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)     ({\
                                    if(LVL_ERR){\
                                    SEGGER_RTT_printf (SEGGER_INDEX, "[ERR] In Function: %s(), %s",\
                                                        __FUNCTION__,(fn_),##__VA_ARGS__);\
                                        }\
                                    })

#define APP_ERR_TRAP(err)       ({\
                                if((err)) {\
                                SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", (err));\
                                __asm("BKPT #0\n");}  /* Trap upon the error  */\
                                    })

#define APP_READ(read_data)    (SEGGER_RTT_Read (SEGGER_INDEX, (read_data), sizeof(read_data)))

#define APP_CHECK_DATA         (SEGGER_RTT_HasKey())

#endif /* COMMON_UTILS_H_ */
