/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures, and function declarations commonly used in the EP.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

/* Generic headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal_data.h"

/* SEGGER RTT and error related headers */
#include "SEGGER_RTT/SEGGER_RTT.h"


#define BIT_SHIFT_8             (8u)
#define SIZE_64                 (64u)

#define LVL_ERR                 (1u)       /* Error condition */

#define RESET_VALUE             (0x00)

#define EP_VERSION              ("4.0.0")
#define MODULE_NAME             "r_bsp"
#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                 *"\
                                "\r\n*   Example Project Version %s                                *"\
                                "\r\n*   Flex Software Pack Version %d.%d.%d                             *"\
                                "\r\n******************************************************************"\
                                "\r\nRefer to readme.txt file for more details on Example Project and"\
                                "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define EP_INFO                "\r\nThis example project demonstrates basic functionalities of BSP dual cores of CPU0 and CPU1."\
                               "\r\nAfter the power is turned on, the program on the CPU0 side makes initial settings, "\
                               "\r\ncopies the program code for CPU1 from the external flash memory to the RAM (the RAM execution "\
                               "\r\nwithout flash memory version does not copy). Then, the program releases the reset of CPU1."\
                               "\r\nThe program on the CPU1 side performs initial settings after software reset is released, "\
                               "\r\nand toggles on-board LED2 and LED3.\r\n"

#define SEGGER_INDEX            (0)

#define APP_PRINT(fn_, ...)      (SEGGER_RTT_printf(SEGGER_INDEX,(fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)  ({\
                                 if(LVL_ERR)\
                                 SEGGER_RTT_printf(SEGGER_INDEX, "[ERR] In Function: %s(), %s",__FUNCTION__,(fn_),##__VA_ARGS__);\
                                 })

#define APP_ERR_TRAP(err)        ({\
                                    if((err)) {\
                                    SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", (err));\
                                    __asm("BKPT #0\n");} /* Trap upon the error */\
                                    })

#define APP_READ(read_data)     (SEGGER_RTT_Read (SEGGER_INDEX, (read_data), sizeof(read_data)))

#define APP_CHECK_DATA          (SEGGER_RTT_HasKey())


#endif /* COMMON_UTILS_H_ */
