/***********************************************************************************************************************
 * File Name    : cmd_otp.h
 * Description  : Contains data structures and functions used in cmd_otp.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CMD_OTP_H__
#define CMD_OTP_H__

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "otp.h"

/* Macro definitions */
/* Parameter size */
#define UID_SIZE           (8U)

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
uint8_t cmd_write_otp (uint16_t otp_addr, otp_data_width_t data);
uint8_t cmd_read_otp (uint16_t otp_addr, otp_data_width_t * p_data);

#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L)
uint8_t cmd_set_sci_usb_boot (uint8_t mode);
uint8_t cmd_get_sci_usb_boot (uint8_t * p_mode);
#endif /* defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L) */

uint8_t cmd_get_unique_id (uint8_t * p_uid);

#endif /* CMD_OTP_H__ */
