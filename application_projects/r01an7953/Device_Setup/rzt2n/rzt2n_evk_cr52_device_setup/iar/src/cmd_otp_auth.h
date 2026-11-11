/***********************************************************************************************************************
 * File Name    : cmd_otp_auth.h
 * Description  : Contains data structures and functions used in cmd_otp_auth.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CMD_OTP_AUTH_H__
#define CMD_OTP_AUTH_H__

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
uint8_t cmd_set_jtag_auth (uint8_t mode, uint8_t type);
uint8_t cmd_get_jtag_auth (uint8_t * p_mode);
uint8_t cmd_set_jtag_auth_id (uint8_t mode, uint8_t type, uint8_t * const p_id);
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
uint8_t cmd_set_sci_usb_boot_auth (uint8_t mode, uint8_t type);
uint8_t cmd_get_sci_usb_boot_auth (uint8_t *p_mode);
uint8_t cmd_set_sci_usb_boot_auth_id (uint8_t type, uint8_t * const p_id);
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H) */

#endif /* CMD_OTP_AUTH_H__ */
