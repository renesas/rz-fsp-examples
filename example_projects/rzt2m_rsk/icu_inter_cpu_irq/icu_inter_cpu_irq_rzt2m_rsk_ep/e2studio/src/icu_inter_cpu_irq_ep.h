/***********************************************************************************************************************
 * File Name    : icu_inter_cpu_irq_ep.h
 * Description  : Contains declarations of data structures and functions used in hal_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ICU_INTER_CPU_IRQ_EP_H_
#define ICU_INTER_CPU_IRQ_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define EP_INFO                "\r\nThis Example Project demonstrates the basic usage of Inter-CPU IRQ on ICU driver."\
                               "\r\nThe project initializes the ICU_Inter_CPU_IRQ module."\
                               "\r\nIt generates software INTCPU0 interrupt then will start toggling of on-board LED.\r\n"

/* MACROs for Null character */
#define NULL_CHAR ('\0')

/* Function declaration */
void icu_inter_cpu_callback(icu_inter_cpu_irq_callback_args_t * p_args);

#endif /* ICU_INTER_CPU_IRQ_EP_H_ */
