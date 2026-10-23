/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains common macros and definitions for the XSPI QSPI example project.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "hal_data.h"

#define EP_VERSION   "1.0.0"
#define MODULE_NAME  "r_xspi_qspi"

#define BANNER_INFO  "\r\n************************************************************************" \
                     "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Module                 *" \
                     "\r\n*   Example Project Version %s                                      *" \
                     "\r\n*   Flex Software Pack Version %d.%d.%d                                   *" \
                     "\r\n************************************************************************" \
                     "\r\nRefer to readme.txt file for more details on Example Project and" \
                     "\r\nFSP User's Manual for more information about " MODULE_NAME " driver\r\n"

fsp_err_t console_open(void);
void console_printf(const char * p_format, ...);

#define APP_PRINT(...)  console_printf(__VA_ARGS__)

#endif /* COMMON_UTILS_H_ */
