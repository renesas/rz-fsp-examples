/***********************************************************************************************************************
 * File Name    : bsp_multi_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef BSP_MULTI_EP_H_
#define BSP_MULTI_EP_H_

/* User LED */
#define LED5        (BSP_IO_PORT_18_PIN_1)

/* IO region */
#define REGION      (BSP_IO_REGION_NOT_SAFE_DEDICATED)

#define APP_ERR_TRAP(err)        if(err) {\
        __asm("BKPT #0\n");} /* Trap upon the error */

#endif /* BSP_MULTI_EP_H_ */
