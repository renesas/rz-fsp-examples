/***********************************************************************************************************************
 * File Name    : ext_irq_setup.h
 * Description  : Contains data structures and functions declaration used in ext_irq_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#ifndef EXT_IRQ_SETUP_H_
#define EXT_IRQ_SETUP_H_

/* Function declarations */
/* This function initializes the INTC IRQ driver */
fsp_err_t init_irq_module (void);

/* This function enables the INTC IRQ driver */
fsp_err_t enable_irq_module (void);

/* This function de-initializes the INTC IRQ driver */
void deinit_irq_module (void);

#endif /* EXT_IRQ_SETUP_H_ */
