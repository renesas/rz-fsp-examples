/***********************************************************************************************************************
 * File Name    : doc_ep.c
 * Description  : Contains data structures and functions used in doc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "doc_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_doc_ep
 * @{
 **********************************************************************************************************************/

/* Private control for ICU */
static void * p_error_ctrl = NULL;

/* DOC event counter */
uint32_t g_callback_event_counter = 0;

/***********************************************************************************************************************
 * @brief     This function configures the PERI_ERR0 interrupt for DOC overflow error handling,
 *            clears any pending error status, un-masks the DOC overflow error source, and enables
 *            the PERI_ERR0 interrupt with the configured priority.
 * @param[IN] None.
 * @retval    None.
 **********************************************************************************************************************/
void error_int_setting(void)
{
    /* Clear error status bit of DOC interrupt */
    R_ICU_NS->PERIERR_CLR[0] = 1U << ICU_ERROR_PERI_ERR0_DOPCI_OFFSET;

    /* Configure PERI_ERR0 signal */
    R_ICU_NS->PERIERR_E0MSK[0] &= (~(1U << ICU_ERROR_PERI_ERR0_DOPCI_OFFSET));

    /* Enable error interrupt */
    R_BSP_IrqCfgEnable(VECTOR_NUMBER_PERI_ERR0, PERI_ERR0_INTERRUPT_PRIORITY, p_error_ctrl);
}

/***********************************************************************************************************************
 * @brief     This is DOC overflow event callback function.
 *            It increments the DOC event counter and clears the corresponding PERI_ERR0 error status.
 * @param[IN] p_args   Pointer to DOC callback arguments (unused in this example).
 * @retval    None.
 **********************************************************************************************************************/
void doc_ep_callback(doc_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);

    g_callback_event_counter++;

    /* Clear error status */
    R_ICU_NS->PERIERR_CLR[0] = 1U << ICU_ERROR_PERI_ERR0_DOPCI_OFFSET;
}

/***********************************************************************************************************************
 * @brief     This function handles DOC overflow error interrupts, invokes the registered DOC callback,
 *            clears the DOC overflow status flag, and restores interrupt context.
 * @param[IN] None.
 * @retval    None.
 **********************************************************************************************************************/
void user_error_isr(void)
{
    /* Save context if RTOS is used */
    FSP_CONTEXT_SAVE;

    doc_instance_ctrl_t * p_ctrl = &g_doc_ctrl;

    /* Call callback */
    doc_callback_args_t args;

    doc_callback_args_t * p_args = p_ctrl->p_callback_memory;
    if (NULL == p_args)
    {
        /* Store on stack */
        p_args = &args;
    }
    else
    {
        /* Save current arguments on the stack in case this is a nested interrupt */
        args = *p_args;
    }

    p_args->p_context = p_ctrl->p_context;

    /* User callback */
    doc_ep_callback(p_args);

    if (NULL != p_ctrl->p_callback_memory)
    {
        /* Restore callback memory in case this is a nested interrupt */
        *p_ctrl->p_callback_memory = args;
    }

    /* Clear DOPCF flag */
    R_DOC->DOCR = (uint8_t)(R_DOC_DOCR_DOPCFCL_Msk | R_DOC_DOCR_DOPCIE_Msk);

    /* Restore context if RTOS is used */
    FSP_CONTEXT_RESTORE;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_doc_ep)
 **********************************************************************************************************************/
