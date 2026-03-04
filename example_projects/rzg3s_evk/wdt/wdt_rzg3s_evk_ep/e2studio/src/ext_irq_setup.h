/***********************************************************************************************************************
 * File Name    : ext_irq_setup.h
 * Description  : Contains declarations of data structures and functions used in ext_irq_setup.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#ifndef EXT_IRQ_SETUP_H_
#define EXT_IRQ_SETUP_H_

/*
 * Function declarations
 */
/* This function initialize the IRQ module */
fsp_err_t init_irq_module (void);

/* Enable the user push button and return the error to the application */
fsp_err_t enable_irq_module (void);

/* Close the IRQ HAL driver */
void deinit_irq_module (void);

#endif /* EXT_IRQ_SETUP_H_ */
