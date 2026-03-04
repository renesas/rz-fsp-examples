/***********************************************************************************************************************
 * File Name    : intc_irq_ep.h
 * Description  : Contains data structures and functions used in intc_irq_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_IRQ_EP_H_
#define INTC_IRQ_EP_H_

/* External IRQ channel. */
#define USER_SW_IRQ_NUMBER        (0x07)        /* Channel 7. */
#define EP_INFO    "\r\nThis example project demonstrates the functionality of INTC_IRQ driver.\r\n"\
                   "On pressing the user push button, an external IRQ is triggered,"\
                   "\r\nwhich toggles LED on PMOD LED.\r\n"

/* Function declaration. */
fsp_err_t intc_irq_init (void);
fsp_err_t intc_irq_enable (void);
void intc_irq_deinit (void);

#endif /* INTC_IRQ_EP_H_ */
