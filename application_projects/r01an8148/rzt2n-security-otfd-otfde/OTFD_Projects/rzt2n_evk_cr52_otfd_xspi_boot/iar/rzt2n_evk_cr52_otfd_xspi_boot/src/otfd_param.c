/***********************************************************************************************************************
 * File Name    : otfd_param.c
 * Description  : Contains data structures and functions used in otfd_param.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "hal_data.h"
#include "otfd_param.h"

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
/* Key data used in OTFD. */
/* g_otfd_key_flash is the inject key area on the flash. */
/* It is written during the setup procedure before flash boot. */
/* g_qspi0_otfd_key stores the key information read from the flash. */
const uint32_t g_otfd_key_flash[RSIP_BYTE_SIZE_WRAPPED_KEY_AES_256 / 4]  BSP_PLACE_IN_SECTION(".otfd_key") = {0xFFFFFFFFU};
uint32_t g_qspi0_otfd_key[RSIP_BYTE_SIZE_WRAPPED_KEY_AES_256 / 4] = {0xFFFFFFFFU}; // Storage area for data read from flash.

/* IV (counter value) used in OTFD. */
/* g_otfd_iv_flash is the IV (counter value) area on flash. */
/* It is written during the setup procedure before flash boot. */
/* g_qspi0_otfd_iv stores the IV (counter value) read from flash. */
/* IV (counter value) = {nonce[127:28], destaddr[31:4]} */
const uint32_t g_otfd_iv_flash[4] BSP_PLACE_IN_SECTION(".otfd_iv") = {0xFFFFFFFFU};
uint32_t g_qspi0_otfd_iv[4] = {0xFFFFFFFFU}; // Storage area for data read from flash.
