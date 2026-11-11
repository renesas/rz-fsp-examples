/***********************************************************************************************************************
 * File Name    : otp.h
 * Description  : Contains data structures and functions used in otp.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef OTP_H__
#define OTP_H__

/* Definition value file for each target device */
#if defined(BSP_MCU_GROUP_RZT2H)
#include "target/rzt2h/otp_info_rzt2h.h"
#elif defined(BSP_MCU_GROUP_RZT2M)
#include "target/rzt2m/otp_info_rzt2m.h"
#elif defined(BSP_MCU_GROUP_RZT2ME)
#include "target/rzt2me/otp_info_rzt2me.h"
#elif defined(BSP_MCU_GROUP_RZT2L)
#include "target/rzt2l/otp_info_rzt2l.h"
#elif defined(BSP_MCU_GROUP_RZT2N)
#include "target/rzt2n/otp_info_rzt2n.h"
#elif defined(BSP_MCU_GROUP_RZN2H)
#include "target/rzn2h/otp_info_rzn2h.h"
#elif defined(BSP_MCU_GROUP_RZN2L)
#include "target/rzn2l/otp_info_rzn2l.h"
#endif

/* Macro definitions */
/* OTP error code */
typedef enum e_otp_err
{
    OTP_SUCCESS     = 0,
    OTP_ERROR       = 1,
} otp_err_t;

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
otp_err_t otp_power_on (void);
void otp_power_off (void);
otp_err_t write_otp_data (uint16_t otp_addr, otp_data_width_t data);
otp_err_t read_otp_data (uint16_t otp_addr, otp_data_width_t * p_data);
otp_err_t write_otp_multiple_data (uint16_t addr, uint8_t * const p_data, uint32_t data_len);
otp_err_t read_otp_multiple_data (uint16_t addr, uint8_t * p_data, uint32_t data_len);
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H)
otp_err_t enable_wordlock_for_otp_area(uint16_t addr, uint32_t area_len);
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H) */

#endif /* OTP_H__ */
