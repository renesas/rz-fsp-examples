/***********************************************************************************************************************
 * File Name    : xspi_qspi_xip_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef XSPI_QSPI_XIP_EP_H_
#define XSPI_QSPI_XIP_EP_H_

/***********************************************************************************************************************
 Macro definitions
 **********************************************************************************************************************/
/* Macros for example project info to be displayed */
#define EP_INFO     "\r\nThis Example Project demonstrates execution from external Serial Flash memory using xSPI in XiP mode."\
                    "\r\nInitialize the xSPI QSPI interface, then enable Quad mode and enter XiP."\
                    "\r\nRun the blinky LED routine while code is fetched from the Serial Flash."\
                    "\r\nExit XiP and disable Quad mode to return to standard SPI operation."\
                    "\r\nProgress and results are printed to the RTT Viewer.\r\n\r\n"\
/*
 * function declarations
 */
void qspi_set_Quad_Mode_enable (void);
void qspi_set_Quad_Mode_disable (void);
void LED_blinky(void);

#endif /* XSPI_QSPI_XIP_EP_H_ */
