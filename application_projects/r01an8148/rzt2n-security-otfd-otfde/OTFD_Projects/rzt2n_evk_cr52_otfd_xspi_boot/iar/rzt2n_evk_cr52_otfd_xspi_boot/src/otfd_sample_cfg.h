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
/** App verification settings on flash.
 *  @note  Set the Verify app setting to [CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP].
 *
 *  @value VERIFY_CERT_ENABLE  : Enable certificate verification for flash app.
 *                               Certificate verification will be performed for the app in flash.
 *         VERIFY_CERT_DISABLE : Disable certificate verification for flash app.
 *                               Certificate verification will not be performed for the app in flash.
 */
#define CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP (VERIFY_CERT_ENABLE)

/* Defines the app verification settings on flash to use. */
#define VERIFY_CERT_ENABLE                 (0)
#define VERIFY_CERT_DISABLE                (1)

#endif /* OTFD_SAMPLE_CFG_H__ */
