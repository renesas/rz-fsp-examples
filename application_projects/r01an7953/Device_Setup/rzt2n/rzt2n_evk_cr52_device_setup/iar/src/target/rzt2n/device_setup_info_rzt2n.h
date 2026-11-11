/***********************************************************************************************************************
 * File Name    : device_setup_info_rzt2n.h
 * Description  : Contains data structures and functions used in device setup for RZT2N.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef DEVICE_SETUP_INFO_RZT2N_H__
#define DEVICE_SETUP_INFO_RZT2N_H__

/* Macro definitions */
/* Buffer address of send / received packets  */
/* The maximum size that can be received is 0xD0000 */
/* Commands received from the tool will never exceed this value */
#define PACKET_BUFFER_ADDR      ((uint32_t)0x10000000UL)

/* Area of SYSTEM RAM used for device setup            */
/* Send/receive buffer     : 0x10000000 - 0x100CFFFF (definition : PACKET_BUFFER_ADDR)       */
/* Flash backup buffer     : 0x100E0000 - 0x100FFFFF (definition : BACKUP_BUFFER_ADDR)       */

#endif /* DEVICE_SETUP_INFO_RZT2N_H__ */
