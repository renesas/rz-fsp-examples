/***********************************************************************************************************************
 * File Name    : otfde_sample.h
 * Description  : Contains data structures and functions used in otfde_sample.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#ifndef OTFDE_SAMPLE_H__
#define OTFDE_SAMPLE_H__

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define SINGLE_CORE_DATA_LEN        (0x10000U)
#define CROSS_CORE_DATA_LEN         (0x100U)

/******************************************************************************
 * External variables and functions
 ******************************************************************************/
extern uint8_t g_plaintext_single_core[SINGLE_CORE_DATA_LEN];
extern uint8_t g_plaintext_cross_core[CROSS_CORE_DATA_LEN];

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
void otfde_sample (void);

#endif /* OTFDE_SAMPLE_H__ */
