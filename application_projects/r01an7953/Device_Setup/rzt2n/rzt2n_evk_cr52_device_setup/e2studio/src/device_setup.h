/***********************************************************************************************************************
 * File Name    : device_setup.h
 * Description  : Contains data structures and functions used in device_setup.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef DEVICE_SETUP_H__
#define DEVICE_SETUP_H__

/* Definition value file for each target device */
#if defined(BSP_MCU_GROUP_RZT2H)
#include "target/rzt2h/device_setup_info_rzt2h.h"
#elif defined(BSP_MCU_GROUP_RZT2M)
#include "target/rzt2m/device_setup_info_rzt2m.h"
#elif defined(BSP_MCU_GROUP_RZT2L)
#include "target/rzt2l/device_setup_info_rzt2l.h"
#elif defined(BSP_MCU_GROUP_RZT2N)
#include "target/rzt2n/device_setup_info_rzt2n.h"
#elif defined(BSP_MCU_GROUP_RZN2H)
#include "target/rzn2h/device_setup_info_rzn2h.h"
#elif defined(BSP_MCU_GROUP_RZN2L)
#include "target/rzn2l/device_setup_info_rzn2l.h"
#endif

/* Macro definitions */
/* Command type */
typedef enum e_cmd_type
{
    TYPE_REQ               = 0x50U,
    TYPE_RES               = 0x55U,
} cmd_type_t;

/* Command code */
typedef enum e_cmd_code
{
    CODE_UNDEF             = 0x00U,
    CODE_WRITE_FLASH       = 0x11U,
    CODE_WRITE_OTP         = 0x12U,
    CODE_READ_OTP          = 0x13U,
    CODE_SET_JTAG_AUTH     = 0x14U,
    CODE_GET_JTAG_AUTH     = 0x15U,
    CODE_SET_JTAG_AUTHID   = 0x16U,
    CODE_SET_SCIUSB        = 0x17U,
    CODE_GET_SCIUSB        = 0x18U,
    CODE_GET_UID           = 0x21U,
    CODE_SET_SCIUSB_AUTH   = 0x22U,
    CODE_GET_SCIUSB_AUTH   = 0x23U,
    CODE_SET_SCIUSB_AUTHID = 0x24U,
} cmd_code_t;

/******************************************************************************
 * Typedef definitions
 ******************************************************************************/
/* Packet format, WRITE_FLASH Command */
typedef struct st_cmd_write_flash
{
    uint32_t   address;
    uint8_t    data[0];
} cmd_write_flash_t;

/* Packet format, WRITE_OTP Command */
typedef struct st_cmd_write_otp
{
    uint16_t   address;
    uint8_t    data[4];
} cmd_write_otp_t;

/* Packet format, READ_OTP Command */
typedef struct st_cmd_read_otp
{
    uint16_t   address;
} cmd_read_otp_t;

/* Packet format, SET_JTAG_AUTH Command */
typedef struct st_cmd_set_jauth
{
    uint8_t    mode;
    uint8_t    type;
} cmd_set_jauth_t;

/* Packet format, SET_JTAG_AUTHID Command */
typedef struct st_cmd_set_jauthid
{
    uint8_t    mode;
    uint8_t    type;
    uint8_t    id[0];
} cmd_set_jauthid_t;

/* Packet format, SET_SCIUSB Command */
typedef struct st_cmd_set_sciusb
{
    uint8_t    mode;
} cmd_set_sciusb_t;

/* Packet format, SET_SCIUSB_AUTH Command */
typedef struct st_cmd_set_sauth
{
    uint8_t    mode;
    uint8_t    type;
} cmd_set_sauth_t;

/* Packet format, SET_SCIUSB_AUTHID Command */
typedef struct st_cmd_set_sauthid
{
    uint8_t    type;
    uint8_t    id[0];
} cmd_set_sauthid_t;

/* Packet format, Command header */
typedef struct st_head
{
    uint8_t    type;
    uint8_t    code;
    uint8_t    dummy[2];
    uint32_t   payload_size;
} head_t;

/* Packet format */
typedef struct st_packet
{
    head_t head;
    union {
        cmd_write_flash_t    wflash;
        cmd_write_otp_t      wotp;
        cmd_read_otp_t       rotp;
        cmd_set_jauth_t      jauth;
        cmd_set_jauthid_t    jauthid;
        cmd_set_sciusb_t     sciusb;
        cmd_set_sauth_t      sauth;
        cmd_set_sauthid_t    sauthid;
    } cmd;
} packet_t;

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
void device_setup (void);

#endif /* DEVICE_SETUP_H__ */
