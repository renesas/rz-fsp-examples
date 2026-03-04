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
#define USER_SW_IRQ_NUMBER        (0x07)        /* Channel 7 */

#define EP_INFO     "\r\nThe example project demonstrates the typical use of the INTC IRQ module APIs.\r\n"\
                    "The project initializes the IRQ interrupt in Interrupt Controller User is requested to press\r\n"\
                    "the BTN0 on PMOD Button on Carrier Board to trigger the external IRQ and this then\r\n"\
                    "will start toggling of user LED (LD1 on PMOD LED).\r\n"

/* Function declaration */
fsp_err_t intc_irq_init(void);
fsp_err_t intc_irq_enable(void);
void intc_irq_deinit(void);

#endif /* INTC_IRQ_EP_H_ */
