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

#define RESET_VALUE             (0x00)

#define EP_VERSION              ("4.0.0")
#define MODULE_NAME             "r_sci_uart"

#define BANNER_INFO             "\r\n********************************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                          *"\
                                "\r\n*   Example Project Version %s                                              *"\
                                "\r\n*   Flex Software Pack Version %d.%d.%d                                           *"\
                                "\r\n********************************************************************************"\
                                "\r\nRefer to readme.txt file for more details on Example Project and"\
                                "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define APP_ERR_TRAP(err)  ({\
        int _err_val = (err);\
        if (_err_val) {\
            printf("\r\nReturned Error Code: 0x%x  \r\n", _err_val);\
            __asm__ volatile("brk 0x0\n");\
        }\
    })

#endif /* COMMON_UTILS_H_ */
