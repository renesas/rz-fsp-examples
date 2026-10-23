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
#define BUF_SIZE                (16U)           /* Size of buffer for console input data */

#define EP_VERSION              "1.0.0"

#define MODULE_NAME             "r_nandc"

#define EP_INFO                 "\r\nThe example project demonstrates NAND Flash controller features"\
                                "\r\nthrough a menu-driven interface on the Terminal Emulator."\
                                "\r\nUsers can execute Basic NAND commands, Remap, and Write Protection demos."\
                                "\r\nThe Basic Demo supports block erase, page write, page read,"\
                                "\r\nraw page read, and raw page write operations."\
                                "\r\nNormal NAND access automatically applies configured ECC, CRC,"\
                                "\r\nOOB, and Skip Byte processing."\
                                "\r\nRaw access reads or writes complete physical NAND pages"\
                                "\r\nfor page layout inspection and verification."\
                                "\r\nThe Remap Demo demonstrates logical-to-physical address mapping."\
                                "\r\nThe Write Protection Demo demonstrates protected-region behavior."\
                                "\r\nStatus messages and test results are displayed on"\
                                "\r\nthe Terminal Emulator.\r\n"


#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module               *"\
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
