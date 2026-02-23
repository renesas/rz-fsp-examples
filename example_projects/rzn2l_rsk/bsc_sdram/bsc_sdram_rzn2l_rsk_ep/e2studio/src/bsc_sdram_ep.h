/***********************************************************************************************************************
 * File Name    : bsc_sdram_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef BSC_SDRAM_EP_H_
#define BSC_SDRAM_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Macros for commands to be received through RTT input */
#define BSC_SDRAM_CS3_MIRROR_SPACE (0x58000000)  /* Use non-cacheable mirror space. */

#define DATA_NUM                   (512)

#define EP_INFO     "\r\nThis example project demonstrates basic functionalities of BSC SDRAM driver."\
                    "\r\nProject initializes a BSC SDRAM instance. Upon successful initialization, "\
                    "\r\nBSC SDRAM executes write data to CS3 mirror area connected to SDRAM and read data after writing."\
                    "\r\nThen, BSC SDRAM executes write 0xFFFF data to CS3 mirror area connected to SDRAM and read data after writing."\
                    "\r\nAny failure will also be displayed on J-Link RTT viewer.\r\n"

/* MACROs for Null character */
#define NULL_CHAR ('\0')

#endif /* BSC_SDRAM_EP_H_ */
