/***********************************************************************************************************************
 * File Name    : intc_nmi_ep.h
 * Description  : Contains declarations of data structures and functions used in intc_nmi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_NMI_EP_H_
#define INTC_NMI_EP_H_

/* External IRQ NMI channel */
#define USER_SW_NMI_NUMBER        (0x00)        /* Channel 0 */

/* Define user LED */
#define USER_LED                  (BSP_IO_PORT_06_PIN_06)

#define EP_INFO    "\r\nThis example project demonstrates the functionality of INTC_NMI driver.\r\n"\
                   "On pressing the user push button, an external IRQ is triggered, which toggles user LED.\r\n"

/* Function to initialize INTC NMI */
fsp_err_t intc_nmi_init (void);

/* Function to enable INTC NMI */
fsp_err_t intc_nmi_enable (void);

/* Function to de-initialize INTC NMI */
void intc_nmi_deinit (void);

#endif /* INTC_NMI_EP_H_ */
