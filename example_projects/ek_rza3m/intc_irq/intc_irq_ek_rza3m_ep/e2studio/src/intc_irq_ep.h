/***********************************************************************************************************************
 * File Name    : intc_irq_ep.h
 * Description  : Contains declarations of data structures and functions used in intc_irq_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_IRQ_EP_H_
#define INTC_IRQ_EP_H_

/* External IRQ channel */
#define USER_SW_IRQ_NUMBER          (0x03)        /* Channel 3 */

/* Define on-board blue LED */
#define BLUE_LED                    (BSP_IO_PORT_10_PIN_01)

#define EP_INFO     "This Example Project demonstrates the functionality of INTC_IRQ\r\n"\
                    "driver. On pressing the user push button, an external IRQ is\r\n"\
                    "triggered, which toggles LED on-board.\r\n\r\n"

/* This function initializes the INTC IRQ driver */
fsp_err_t intc_irq_init (void);

/* This function enables the INTC IRQ driver */
fsp_err_t intc_irq_enable (void);

/* This function de-initializes the INTC IRQ driver */
void intc_irq_deinit (void);

#endif /* INTC_IRQ_EP_H_ */
