/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros and definitions commonly used in the Ethernet example project.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_data.h"

#define EP_VERSION   "1.0.0"
#define MODULE_NAME  "r_gmac_b / r_ethsw"

#define BANNER_INFO  "\r\n************************************************************************" \
                     "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Module          *" \
                     "\r\n*   Example Project Version %s                                      *" \
                     "\r\n*   Flex Software Pack Version %d.%d.%d                                   *" \
                     "\r\n************************************************************************" \
                     "\r\nRefer to readme.txt file for more details on Example Project and" \
                     "\r\nFSP User's Manual for more information about " MODULE_NAME " driver\r\n"

#endif /* COMMON_UTILS_H_ */
