/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros and definitions commonly used in the MTU3 phase count example project.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_data.h"

#define EP_VERSION   "1.0.0"
#define MODULE_NAME  "r_mtu3"

#define BANNER_INFO  "\r\n************************************************************************" \
                     "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Module                      *" \
                     "\r\n*   Example Project Version %s                                      *" \
                     "\r\n*   Flex Software Pack Version %d.%d.%d                                   *" \
                     "\r\n************************************************************************" \
                     "\r\nRefer to readme.txt file for more details on Example Project and" \
                     "\r\nFSP User's Manual for more information about " MODULE_NAME " driver\r\n"

#define APP_PRINT(fn_, ...)  (printf((fn_), ##__VA_ARGS__))

#endif /* COMMON_UTILS_H_ */
