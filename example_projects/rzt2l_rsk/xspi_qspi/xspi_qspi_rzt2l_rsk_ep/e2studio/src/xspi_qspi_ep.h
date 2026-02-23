/***********************************************************************************************************************
 * File Name    : xspi_qspi_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef XSPI_QSPI_EP_H_
#define XSPI_QSPI_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define QSPI_DEVICE_START_ADDRESS (0x68000000)
#define DATA_SIZE            (64)
#define WRITE_ADDRESS        (0x100)

/* Macros for example project info to be displayed */
#define EP_INFO     "\r\nThis Example Project demonstrates typical use of the xSPI_QSPI driver with an external Serial NOR flash."\
                    "\r\nConfigure xSPI for QSPI operation, enable Quad mode and 4-byte addressing, then erase a test sector."\
                    "\r\nIssue Page Program (0x02) and Fast Read (0xEB) at a test address and verify the data."\
                    "\r\nRead the same location via the memory-mapped region (direct read) and verify again."\
                    "\r\nFinally, write a small buffer and confirm via mapped reads; status is printed to the RTT Viewer.\r\n\r\n"\
/*
 * function declarations
 */
void qspi_set_Quad_Mode_enable (void);
void qspi_set_Quad_Mode_disable (void);

#endif /* XSPI_QSPI_EP_H_ */
