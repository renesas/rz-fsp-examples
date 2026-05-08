/***********************************************************************************************************************
 * File Name    : spi_flash_example.h
 * Description  : Contains declarations of data structures and functions used in spi_flash_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SPI_FLASH_EXAMPLE_H
#define SPI_FLASH_EXAMPLE_H

#include "bsp_api.h"
#include <stdbool.h>
#include <stdint.h>

/* Macros definitions */
#define MODULE_NAME    "SPIBSC"
#define EP_VERSION     "4.1.0"

#define UART_TX_BUFFER_ALIGNMENT       (64U)
#define UART_TX_BUFFER_ALIGNMENT_MASK  (UART_TX_BUFFER_ALIGNMENT - 1U)

#define BANNER_1    "\r\n********************************************************************************"\
                    "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                              *"\
                    "\r\n*   Example Project Version "EP_VERSION"                                              *"\
                    "\r\n*   Flex Software Pack Version "
#define BANNER_2    "                                           *"

#define BANNER_3    "\r\n********************************************************************************"\
                    "\r\nRefer to readme.txt file for more details on Example Project and" \
                    "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define EP_INFO     "The example project demonstrates the erase and write functionalities of the SPI flash"\
                    " device\r\non RZ/A3M MPU based on Renesas FSP.\r\n"\
                    "The sample code is executed by selecting the menu number on the console.\r\n"\
                    "In main menu, user selects operation for the flash device.\r\n"\
                    "    1. Erase and verify.\r\n"\
                    "    2. Write random data and verify.\r\n"\
                    "And then, user inputs the target address and size.\r\n"\
                    "After, the sample code execute the operation and show the result.\r\n"\
                    "LED blinks while running the project.\r\n"\
                    "Input guide:\r\n"\
                    "- Address : Hexadecimal flash address (e.g. 0x40000)\r\n"\
                    "- Size    : Decimal size in bytes\r\n"\
                    "- Enter 0 to return to main menu\r\n"

/* Function declarations */
extern void      example_init (void);
extern fsp_err_t example_erase (uint32_t address, uint32_t size);
extern fsp_err_t example_write (const uint8_t * data, uint32_t address, uint32_t size);

#endif /* SPI_FLASH_EXAMPLE_H */
