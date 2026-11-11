/***********************************************************************************************************************
 * File Name    : flash.h
 * Description  : Contains data structures and functions used in flash.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef FLASH_H__
#define FLASH_H__

/* Definition value file for each target device */
#if defined(BSP_MCU_GROUP_RZT2H)
#include "target/rzt2h/flash_info_rzt2h.h"
#elif defined(BSP_MCU_GROUP_RZT2M)
#include "target/rzt2m/flash_info_rzt2m.h"
#elif defined(BSP_MCU_GROUP_RZT2ME)
#include "target/rzt2me/flash_info_rzt2me.h"
#elif defined(BSP_MCU_GROUP_RZT2L)
#include "target/rzt2l/flash_info_rzt2l.h"
#elif defined(BSP_MCU_GROUP_RZT2N)
#include "target/rzt2n/flash_info_rzt2n.h"
#elif defined(BSP_MCU_GROUP_RZN2H)
#include "target/rzn2h/flash_info_rzn2h.h"
#elif defined(BSP_MCU_GROUP_RZN2L)
#include "target/rzn2l/flash_info_rzn2l.h"
#endif

/* Macro definitions */
/* Maximum value of 32-bit address space */
#define MAX_VAL_ADDR_AREA_32BIT_SPACE    ((uint32_t)0xFFFFFFFFUL)

/* Flash error code */
typedef enum e_flash_err
{
    FLASH_SUCCESS           = 0,
    FLASH_ERR_PARAMETER     = 1,
    FLASH_ERR_MODULE        = 2,
    FLASH_ERR_VERIFY_FAILED = 3,
} flash_err_t;

/* XSPI unit number */
typedef enum e_xspi_unit_number
{
    XSPI_UNIT_NUM_0 = 0,
    XSPI_UNIT_NUM_1 = 1,
} xspi_unit_number_t;

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
#if (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1)
flash_err_t open_qspi_flash (xspi_unit_number_t unit_num);
flash_err_t close_qspi_flash (xspi_unit_number_t unit_num);
flash_err_t write_to_qspi_area (uint32_t addr, uint8_t * const p_data, uint32_t data_len, xspi_unit_number_t unit_num);
void read_to_qspi_area (uint32_t addr, uint8_t * p_data, uint32_t data_len);
#endif /* (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1) */

#if (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1)
flash_err_t open_ospi_flash (xspi_unit_number_t unit_num);
flash_err_t close_ospi_flash (xspi_unit_number_t unit_num);
flash_err_t write_to_ospi_area (uint32_t addr, uint8_t * const p_data, uint32_t data_len, xspi_unit_number_t unit_num);
void read_to_ospi_area (uint32_t addr, uint8_t * p_data, uint32_t data_len);
#endif /* (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1) */

#if (1 == USE_FLASH_NOR_CS0)
flash_err_t open_nor_flash (void);
flash_err_t close_nor_flash (void);
flash_err_t write_to_nor_area (uint32_t addr, uint8_t * const p_data, uint32_t data_len);
void read_to_nor_area (uint32_t addr, uint8_t * p_data, uint32_t data_len);
#endif /* (1 == USE_FLASH_NOR_CS0) */

#endif /* FLASH_H__ */
