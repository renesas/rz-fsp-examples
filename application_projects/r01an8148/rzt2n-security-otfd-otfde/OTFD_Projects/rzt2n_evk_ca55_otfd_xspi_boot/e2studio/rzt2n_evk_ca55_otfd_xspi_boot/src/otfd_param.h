/***********************************************************************************************************************
 * File Name    : otfd_param.h
 * Description  : Contains data structure and function declarations used in otfd_param.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#ifndef OTFD_PARAM_H_
#define OTFD_PARAM_H_

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
/* Key data used in OTFD. */
extern const  uint32_t g_otfd_key_flash[RSIP_BYTE_SIZE_WRAPPED_KEY_AES_256 / 4];
extern uint32_t g_qspi0_otfd_key[RSIP_BYTE_SIZE_WRAPPED_KEY_AES_256 / 4];

/* IV (counter value) used in OTFD. */
extern const uint32_t g_otfd_iv_flash[4];
extern uint32_t g_qspi0_otfd_iv[4];

#endif /* OTFD_PARAM_H_ */
