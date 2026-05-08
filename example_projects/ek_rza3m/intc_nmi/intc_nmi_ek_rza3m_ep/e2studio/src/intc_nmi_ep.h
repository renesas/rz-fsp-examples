/***********************************************************************************************************************
 * File Name    : intc_nmi_ep.h
 * Description  : Contains declarations of data structures and functions used in intc_nmi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef INTC_NMI_EP_H_
#define INTC_NMI_EP_H_

/* NMI channel */
#define USER_SW_NMI_NUMBER        (0x00)        /* Channel 0 */

#define EP_INFO     "This Example project demonstrates the functionality of INTC_NMI driver.\r\n"\
                    "On pressing the user pushbutton, an external IRQ is triggered,\r\n"\
                    "which toggles on-board LED.\r\n\r\n"

/* Function to initialize INTC NMI */
fsp_err_t intc_nmi_init (void);

/* Function to enable INTC NMI */
fsp_err_t intc_nmi_enable (void);

/* Function to de-initialize INTC NMI */
void intc_nmi_deinit (void);

#endif /* INTC_NMI_EP_H_ */
