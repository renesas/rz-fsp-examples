/***********************************************************************************************************************
 * File Name    : otp_info_rzt2n.h
 * Description  : Contains data structures and functions used in OTP info for RZT2N.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef OTP_INFO_RZT2N_H__
#define OTP_INFO_RZT2N_H__

/* Macro definitions */
/* OTP address for User area */
#define USER_AREA_START_ADDR                ((uint16_t)0x01B1U)
#define USER_AREA_END_ADDR                  ((uint16_t)0x01CFU)

/* OTP address for Anti-rollback counter area */
#define COUNTER_AREA_START_ADDR             ((uint16_t)0x0159U)
#define COUNTER_AREA_END_ADDR               ((uint16_t)0x016CU)

/* OTP address for Part number */
#define PART_NUM_START_ADDR                 ((uint16_t)0x01A7U)
#define PART_NUM_END_ADDR                   ((uint16_t)0x01A8U)

/* OTP address for Product Version */
#define PRODUCT_VER_START_ADDR              ((uint16_t)0x01A9U)
#define PRODUCT_VER_END_ADDR                ((uint16_t)0x01AAU)

/* OTP address for Temperature Sensor Code High */
#define TEMPERATURE_SENSOR_H_ADDR           ((uint16_t)0x01ABU)

/* OTP address for Temperature Sensor Code Low */
#define TEMPERATURE_SENSOR_L_ADDR           ((uint16_t)0x01ACU)

/* OTP address for PCIe PHY trimming code*/
#define PCIE_PHY_TRIMMING_CODE_START_ADDR   ((uint16_t)0x00B9U)
#define PCIE_PHY_TRIMMING_CODE_END_ADDR     ((uint16_t)0x00BAU)

/* OTP address for JTAG authentication */
#define JTAG_AUTH_MODE_ADDR                 ((uint16_t)0x0091U)
#define JTAG_AUTH_LV1_PLAIN_ADDR            ((uint16_t)0x0093U)
#define JTAG_AUTH_LV2_PLAIN_ADDR            ((uint16_t)0x009BU)

/* OTP address for SCI/USB boot */
#define SCI_USB_BOOT_MODE_ADDR              ((uint16_t)0x018DU)
#define SCI_USB_BOOT_ID_PLAIN_ADDR          ((uint16_t)0x00A3U)

/* OTP address for Unique ID */
#define UID_ADDR                            ((uint16_t)0x0089U)

/* Value written to the OTP area when changing settings */
#define OTP_VAL_AUTHMODEJ_AUTHLV1           (0x0007U)
#define OTP_VAL_AUTHMODEJ_AUTHLV2           (0x0038U)
#define OTP_VAL_AUTHMODEJ_PROHIBIT          (0x01C0U)
#define OTP_VAL_AUTHMODES_AUTH              (0x0007U)
#define OTP_VAL_AUTHMODES_PROHIBIT          (0x0038U)

/* OTP area address mask value (address value: 9 bits) */
#define OTP_ADDRESS_MASK                    (0x1FFU)

/* OTP data width */
typedef uint16_t otp_data_width_t;

#endif /* OTP_INFO_RZT2N_H__ */
