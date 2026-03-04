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

extern uint32_t app_read (unsigned char * p_data);
extern int      app_check_data ();

#define BIT_SHIFT_8   (8u)
#define BIT_SHIFT_12  (12u)
#define BIT_SHIFT_15  (15u)
#define SIZE_64       (64u)
#define LVL_ERR       (1u)       /* Error condition */
#define RESET_VALUE   (0x00)
#define ADC_12_BIT    (4096u)
#define V_REF         (1.8f)
#define ADC_H_LMT     (3103u)
#define ADC_L_LMT     (1241u)

#define EP_VERSION              "4.0.0"
#define MODULE_NAME             "r_adc"

#define BANNER_INFO             "\r\n******************************************************************"\
                                "\r\n*   Renesas FSP Example Project for "MODULE_NAME" Module                 *"\
                                "\r\n*   Example Project Version %s                                *"\
                                "\r\n*   Flex Software Pack Version %d.%d.%d                             *"\
                                "\r\n******************************************************************"\
                                "\r\nRefer to readme.txt file for more details on Example Project and"\
                                "\r\nFSP User's Manual for more information about "MODULE_NAME" driver\r\n"

#define APP_PRINT(fn_, ...)      (printf((fn_), ##__VA_ARGS__))

#define APP_ERR_PRINT(fn_, ...)  if(LVL_ERR)\
        printf("[ERR] In Function: %s",(fn_),##__VA_ARGS__);

#define APP_ERR_TRAP(err)        if(err) {\
        printf("\r\nReturned Error Code: 0x%x  \r\n", err);\
        __asm__ volatile("brk 0x0\n");} /* Trap upon the error  */

#define APP_READ(read_data)      app_read(read_data)

#define APP_CHECK_DATA           app_check_data()

#endif /* COMMON_UTILS_H_ */
