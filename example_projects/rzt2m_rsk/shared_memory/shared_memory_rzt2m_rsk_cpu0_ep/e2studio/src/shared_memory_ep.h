/***********************************************************************************************************************
 * File Name    : shared_memory_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef SHARED_MEMORY_EP_H_
#define SHARED_MEMORY_EP_H_

/***********************************************************************************************************************
 Macro definitions
 **********************************************************************************************************************/
#define SHARED_MEMORY_SIZE            (256U)

/* Macros for example project info to be displayed */
#define EP_INFO           "\r\nThis Example Project demonstrates inter-CPU data exchange using the shared_memory module on RZ/T."\
                          "\r\nCPU0 initializes the shared memory field, both CPUs open the driver (peer waits until READY_TO_WRITE)."\
                          "\r\nCPU0 writes the first half of shared memory and generates interrupt to CPU1."\
                          "\r\nCPU1 reads it from the shared memory, writes data to the second half, and generates interrupt to CPU0."\
                          "\r\nCPU0 reads back, verifies the expected pattern, reports MATCH/FAIL, and both close the driver."\
                          "\r\nThe result is displayed on JLink RTT Viewer at both CPU0 and CPU1 sides.\r\n\r\n"\

#endif /* SHARED_MEMORY_EP_H_ */
