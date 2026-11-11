/***********************************************************************************************************************
 * Copyright [2020-2025] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/
#ifndef FWUPDATE_INFO_RZT2N_H__
#define FWUPDATE_INFO_RZT2N_H__

#include "fwupdate_cfg.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/* Packet buffer address */
#define PACKET_BUFFER_ADDR                         ((uint32_t)0x10000000UL)

/* Maximum size of receive buffer */
#define PACKET_RCV_MAX_SIZE                        (0x00180000U)

/* External flash address */
/* User application area address (BOOT_MODE_XSPI0) */
#if FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
#define TARGET_BOARD_FLASH_START_ADDR              ((uint32_t)XSPI0_START_ADDR)
#define USER_APPLICATION_AREA_START_ADDR           ((uint32_t)0x40094050UL)
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x4204A027UL)
 #elif FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x43FFFFFFUL)
 #endif

/* User application extend area address (BOOT_MODE_XSPI0) */
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_EXTEND_AREA_START_ADDR    ((uint32_t)0x50000000UL)
#define USER_APPLICATION_EXTEND_AREA_END_ADDR      ((uint32_t)0x50FFFFFFUL)
 #endif

/* User application area address (BOOT_MODE_XSPI1) */
#elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1
#define TARGET_BOARD_FLASH_START_ADDR              ((uint32_t)XSPI1_START_ADDR)
#define USER_APPLICATION_AREA_START_ADDR           ((uint32_t)0x50094050UL)
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x5084A027UL)
 #elif FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x50FFFFFFUL)
 #endif

/* User application extend area address (BOOT_MODE_XSPI1) */
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_EXTEND_AREA_START_ADDR    ((uint32_t)0x40000000UL)
#define USER_APPLICATION_EXTEND_AREA_END_ADDR      ((uint32_t)0x43FFFFFFUL)
 #endif

/* User application area address (BOOT_MODE_NOR) */
/* Flash areas not installed in the device are defined as 0x00000000. */
#elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
#define TARGET_BOARD_FLASH_START_ADDR              ((uint32_t)0x00000000UL)
#define USER_APPLICATION_AREA_START_ADDR           ((uint32_t)0x00000000UL)
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x00000000UL)
 #elif FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_AREA_END_ADDR             ((uint32_t)0x00000000UL)
 #endif

/* User application extend area address (BOOT_MODE_NOR) */
/* Flash areas not installed in the device are defined as 0x00000000. */
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
#define USER_APPLICATION_EXTEND_AREA_START_ADDR    ((uint32_t)0x00000000UL)
#define USER_APPLICATION_EXTEND_AREA_END_ADDR      ((uint32_t)0x00000000UL)
 #endif
#endif

/* Update program area address */
#define UPDATE_PROG_AREA_ADDR                      ((uint32_t)(TARGET_BOARD_FLASH_START_ADDR + LOADER_PARAM_INFO_AREA_SIZE + SSBL_PROG_AREA_SIZE))

/* Parameter for user application start address */
#define PARAM_INFO_ADDR_CPU_CORE0                  ((uint32_t)(UPDATE_PROG_AREA_ADDR + UPDATE_PROG_AREA_SIZE))
#define PARAM_INFO_ADDR_CPU_CORE1                  ((uint32_t)(PARAM_INFO_ADDR_CPU_CORE0 + USER_PARAM_INFO_AREA_SIZE))
#define PARAM_INFO_ADDR_CPU_CORE2                  ((uint32_t)(PARAM_INFO_ADDR_CPU_CORE1 + USER_PARAM_INFO_AREA_SIZE))
#define PARAM_INFO_ADDR_CPU_CORE3                  ((uint32_t)(PARAM_INFO_ADDR_CPU_CORE2 + USER_PARAM_INFO_AREA_SIZE))

/* Flash management area address */
#define FLASH_MNG_AREA_ADDR_CPU_CORE0              ((uint32_t)(PARAM_INFO_ADDR_CPU_CORE3 + USER_PARAM_INFO_AREA_SIZE))
#define FLASH_MNG_AREA_ADDR_CPU_CORE1              ((uint32_t)(FLASH_MNG_AREA_ADDR_CPU_CORE0 + FLASH_MNG_AREA_PER_CPU_CORE_SIZE))
#define FLASH_MNG_AREA_ADDR_CPU_CORE2              ((uint32_t)(FLASH_MNG_AREA_ADDR_CPU_CORE1 + FLASH_MNG_AREA_PER_CPU_CORE_SIZE))
#define FLASH_MNG_AREA_ADDR_CPU_CORE3              ((uint32_t)(FLASH_MNG_AREA_ADDR_CPU_CORE2 + FLASH_MNG_AREA_PER_CPU_CORE_SIZE))

/* Data size(Unit:bytes) */
#define LOADER_PARAM_INFO_AREA_SIZE                (80U)
#define SSBL_PROG_AREA_SIZE                        (0x37000U)
#define UPDATE_PROG_AREA_SIZE                      (0x5CFB0U)
#define USER_PARAM_INFO_AREA_SIZE                  (16U)
#define FLASH_MNG_AREA_PER_CPU_CORE_SIZE           (4U)

#endif /* FWUPDATE_INFO_RZT2N_H__ */
