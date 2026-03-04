/***********************************************************************************************************************
 * File Name    : intc_irq_ep.h
 * Description  : Contains declarations of data structures and functions used in intc_irq_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_IRQ_EP_H_
#define INTC_IRQ_EP_H_

/* External IRQ channel */
#define USER_SW_IRQ_NUMBER        (0x0E)        /* Channel 14 */
#define LED_BLINK                 (2U)          /* Index of the LED to toggle */

#define EP_INFO     "\r\nThe example project demonstrates the typical use of the INTC IRQ module APIs.\r\n"\
                    "The project initializes the IRQ in Interrupt Controller User is requested to press\r\n"\
                    "the USER_SW1 push button on Carrier Board to trigger the external IRQ and this then\r\n"\
                    "will start toggling of user LED (LD2 on PMOD LED).\r\n"

/* Function declarations */
fsp_err_t intc_irq_init(void);
fsp_err_t intc_irq_enable(void);
void intc_irq_deinit(void);

#endif /* INTC_IRQ_EP_H_ */
