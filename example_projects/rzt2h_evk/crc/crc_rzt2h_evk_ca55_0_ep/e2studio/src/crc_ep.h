/***********************************************************************************************************************
 * File Name    : crc_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CRC_EP_H_
#define CRC_EP_H_

#define EXPECTED_CRC_VALUE (0x85) /* Expected CRC value after calculation */

#define EP_INFO "\nThis example project demonstrates the use of the CRC (Cyclic Redundancy Check) module"\
                "\r\non the evaluation board. It initializes the CRC peripheral, then performs"\
                "\r\nan 8-bit CRC calculation on a predefined sample data buffer with a seed value of 0."\
                "\r\nThe calculated result is then compared against the expected value to verify correctness."\
                "\r\nAny API error or CRC mismatch will also be displayed on the console.\r\n"

#endif /* CRC_EP_H_ */
