/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros and definitions commonly used in the SCI UART DMAC example project.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

/* Generic headers. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_data.h"

#define EP_VERSION   "1.0.0"
#define MODULE_NAME  "r_sci_uart / r_dmac"

#define BANNER_INFO  "\r\n************************************************************************" \
                     "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Modules        *" \
                     "\r\n*   Example Project Version " EP_VERSION "                                      *" \
                     "\r\n*   Flex Software Pack Version 4.2.0                                   *" \
                     "\r\n************************************************************************" \
                     "\r\nRefer to readme.txt file for more details on Example Project and" \
                     "\r\nFSP User's Manual for more information about " MODULE_NAME " drivers\r\n"

#endif /* COMMON_UTILS_H_ */
