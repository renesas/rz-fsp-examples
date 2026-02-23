/***********************************************************************************************************************
 * File Name    : xspi_ospi_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef XSPI_OSPI_EP_H_
#define XSPI_OSPI_EP_H_

/***********************************************************************************************************************
 Macro definitions
 **********************************************************************************************************************/

#define DATA_SIZE (64)

/* Set the same value as the sector erase size set in FSP Configuration. */
#define SECTOR_ERASE_SIZE (4096)
#define OSPI_DEVICE_START_ADDRESS (0x60000000)
#define WRITE_START_ADDRESS (0x1000)

/* Macros for example project info to be displayed */
#define EP_INFO    "\r\nThis Example Project demonstrates typical use of the xSPI_OSPI driver with an external OctaFlash in OPI-DTR mode."\
                   "\r\nThe program temporarily sets 1S-1S-1S, enables OPI-DTR via CR2, then switches the controller to 8D-8D-8D."\
                   "\r\nA test sector is erased; Page Program writes data, a high-speed read verifies it, and the same location"\
                   "\r\nis checked again via the non-cacheable memory-mapped mirror."\
                   "\r\nFinally, write a small buffer and confirm via mapped reads; status is printed to the RTT Viewer.\r\n\r\n"\

/*
 * function declarations
 */
void ospi_set_DTR_OPI_Mode_enable (void);

#endif /* XSPI_OSPI_EP_H_ */
