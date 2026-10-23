/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures, and function declarations commonly used in the EP.
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

#define LVL_ERR      (1u)
#define RESET_VALUE  (0x00)

#define EP_VERSION   "1.0.0"

#define MODULE_NAME  "r_sci_spi"

#define BANNER_INFO  "\r\n************************************************************************" \
                     "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Module                   *" \
                     "\r\n*   Example Project Version %s                                      *" \
                     "\r\n*   Flex Software Pack Version %d.%d.%d                                   *" \
                     "\r\n************************************************************************" \
                     "\r\nRefer to readme.txt file for more details on Example Project and" \
                     "\r\nFSP User's Manual for more information about " MODULE_NAME " driver\r\n"

#define APP_PRINT(fn_, ...)      (printf((fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)  if (LVL_ERR) \
                                 { \
                                     printf("[ERR] In Function: %s", (fn_), ##__VA_ARGS__); \
                                 }

#define APP_ERR_TRAP(err)        if (err) \
                                 { \
                                     printf("\r\nReturned Error Code: 0x%x\r\n", (unsigned int) (err)); \
                                     __BKPT(0); \
                                 }

#endif /* COMMON_UTILS_H_ */
