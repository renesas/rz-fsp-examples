/************************************************************************************************************* * File Name    : intc_tint_ep.h
 * Description  : Contains declarations of data structures and functions used in intc_tint_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_TINT_EP_H_
#define INTC_TINT_EP_H_

/* TINT channel */
#define USER_SW_TINT_NUMBER     (0x18)        /* Channel 24 */

#define BANNER      "This Example Project demonstrates the functionality of INTC_TINT driver.\r\n"\
                    "On pressing the user push button, an external IRQ is triggered, which toggles PMOD LED.\r\n\r\n"

/* Function to initialize INTC TINT */
fsp_err_t intc_tint_init (void);

/* Function to enable INTC TINT */
fsp_err_t intc_tint_enable (void);

/* Function to de-initialize INTC TINT */
void intc_tint_deinit (void);

#endif /* INTC_TINT_EP_H_ */
