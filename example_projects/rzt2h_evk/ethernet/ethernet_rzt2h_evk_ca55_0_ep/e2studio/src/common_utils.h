/*****************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures, and function declarations commonly used in the EP.
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

extern uint32_t app_read (unsigned char * p_data);
extern int      app_check_data ();

/* Macro for default buff size for reading through TeraTerm */
#define BUFF_SIZE       (0x0F)

#define LVL_ERR         (1u)       /* Error condition */

#define RESET_VALUE     (0x00)
#define NULL_CHAR       ('\0')

#define EP_VERSION      "4.0.0"

#define MODULE_NAME     "r_ethernet"

#define EP_INFO         "\nThis sample software performs Ethernet communication with 3 ports " \
                        "\r\nETH0, ETH1, and ETH2 in the default state." \
                        "\r\nGMAC0 is connected to ETH0, ETH1, and ETH2 via ETHSW\r\n"

#define BANNER_INFO     "\r\n******************************************************************"\
                        "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module            *"\
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

#define APP_READ(read_data)      app_read(read_data)

#endif /* COMMON_UTILS_H_ */
