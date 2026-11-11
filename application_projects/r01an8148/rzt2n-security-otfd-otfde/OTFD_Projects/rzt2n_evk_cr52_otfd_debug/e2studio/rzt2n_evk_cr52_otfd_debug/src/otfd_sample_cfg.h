/***********************************************************************************************************************
 * File Name    : otfd_sample_cfg.h
 * Description  : Contains data structure and function declarations used in otfd_param.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
#ifndef OTFD_SAMPLE_CFG_H__
#define OTFD_SAMPLE_CFG_H__

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/** Key types used in OTFD samples.
 *  @note  Set the key type to be used to  [CFG_OTFD_SAMPLE_USE_KEY].
 *
 *  @value USE_KEY128 : Use the following AES128bit length key.
 *                      00000000000000000000000000000000
 *         USE_KEY256 : Use the following AES256bit length key.
 *                      FB3577D4E8F7251869DEEC618E04C1D1767E6A17975CEB93FC976E92CCC355F0
 */
#define CFG_OTFD_SAMPLE_USE_KEY   (USE_KEY128)

/* Defining the key type specification to use. */
#define USE_KEY128                (0)
#define USE_KEY256                (1)

#endif /* OTFD_SAMPLE_CFG_H__ */
