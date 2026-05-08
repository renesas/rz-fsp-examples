/***********************************************************************************************************************
 * File Name    : doc_ep.h
 * Description  : Contains declarations of data structures and functions used in doc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef DOC_EP_H_
#define DOC_EP_H_

#include "common_utils.h"

#define ICU_ERROR_PERI_ERR0_DOPCI_OFFSET (18)      /* Bit offset of DOC overflow error source in PERI_ERR0 register */
#define PERI_ERR0_INTERRUPT_PRIORITY     (14)      /* Interrupt priority level for PERI_ERR0 (DOC overflow error) */
#define DOC_REF_DATA_MATCH               (0x1234U) /* Reference data used to validate comparison match operation */
#define DOC_REF_DATA_MISMATCH            (0xFFFFU) /* Reference data used to validate comparison mismatch operation */
#define DOC_CMP_INPUT_MISMATCH           (0x1234U) /* Input data used to intentionally cause comparison mismatch */
#define DOC_ADD_INPUT_VALUE              (0xF000U) /* Input data value used to trigger addition overflow */
#define DOC_ADD_INITIAL_VALUE            (0xF000U) /* Initial accumulator value for addition overflow test */
#define DOC_SUB_INPUT_VALUE              (0x0001U) /* Input data value used to trigger subtraction underflow */
#define DOC_SUB_INITIAL_VALUE            (0x0000U) /* Initial accumulator value for subtraction underflow test */

#define EP_INFO "\nThis example project demonstrates the usage of the DOC HAL module on RZT2H devices."\
                "\r\nThe project showcases DOC event-based operations configured via the FSP configurator."\
                "\r\nEach DOC event is selected and verified individually, including Comparison Match,"\
                "\r\nComparison Mismatch, Addition Overflow, and Subtraction Underflow."\
                "\r\nOn RZT2H, comparison events are verified by polling the DOC operation complete flag,"\
                "\r\nwhile arithmetic error events are verified using the PERI_ERR0 interrupt mechanism."\
                "\r\nThe operation status and verification results are displayed on the Tera Term."\
                "\r\nAny API error or unexpected DOC behavior will be reported on the console.\r\n"

/* Extern variables */
extern uint32_t g_callback_event_counter;

/* Global functions prototype */
void error_int_setting(void);
void user_error_isr (void);

#endif /* DOC_EP_H_ */
