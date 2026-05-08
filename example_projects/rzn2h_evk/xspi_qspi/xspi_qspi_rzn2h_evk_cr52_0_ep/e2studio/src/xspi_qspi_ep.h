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

/* Macro for start address of QSPI memory */
#define QSPI_DEVICE_START_ADDRESS   (0x50000000)

/* Macro for data size to write */
#define DATA_SIZE                   (8)

/* Macro for offset start address of QSPI memory */
#define WRITE_ADDRESS               (0x100)

/* Macro for example project info to be displayed */
#define EP_INFO     "\r\nThis example project demonstrates typical use of the xSPI_QSPI driver with an external"\
                    "\r\nSerial NOR flash. Upon successful initialization, the project erases a memory sector."\
                    "\r\nIssue Page Program (0x02) and Fast Read (0xEB) at a test address and verify the data."\
                    "\r\nRead the same location via the memory-mapped region (direct read) and verify again."\
                    "\r\nFinally, write a small data to buffer and confirm via memory-mapping read."\
                    "\r\nAny API or event is printed to the RTT Viewer.\r\n\r\n"
/*
 * Function declarations
 */
void qspi_set_Quad_Mode_enable (void);
void qspi_set_Quad_Mode_disable (void);

#endif /* XSPI_QSPI_EP_H_ */
