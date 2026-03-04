/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures, and function declarations commonly used in the EP.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
 
#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

/* Generic headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_data.h"

/* Functions for data input handling */
extern void app_read (unsigned char * p_data);
extern int app_check_data ();

#define SIZE_64         (64u)      /* Buffer or data size */
#define LVL_ERR         (1u)       /* Error condition */
#define RESET_VALUE     (0x00)     /* Reset value */

#define EP_VERSION      "4.0.0"
#define MODULE_NAME     "r_sci_uart"

#define BANNER_INFO     "\r\n********************************************************************************"\
                        "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                          *"\
                        "\r\n*   Example Project Version %s                                              *"\
                        "\r\n*   Flex Software Pack Version %d.%d.%d                                           *"\
                        "\r\n********************************************************************************"\
                        "\r\nRefer to readme.txt file for more details on Example Project and"\
                        "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#endif /* COMMON_UTILS_H_ */
