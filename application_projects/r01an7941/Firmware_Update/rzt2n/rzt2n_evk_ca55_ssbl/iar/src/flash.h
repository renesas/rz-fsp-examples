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
#ifndef FLASH_H__
#define FLASH_H__

/* Definition value file for each target device. */
#if defined(BSP_CFG_CORE_CR52)
 #if defined(BSP_MCU_GROUP_RZT2M)
  #include "target/rzt2m/flash_info_rzt2m.h"
 #elif defined(BSP_MCU_GROUP_RZT2L)
  #include "target/rzt2l/flash_info_rzt2l.h"
 #elif defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h/flash_info_rzt2h.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n/flash_info_rzt2n.h"
 #elif defined(BSP_MCU_GROUP_RZN2L)
  #include "target/rzn2l/flash_info_rzn2l.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h/flash_info_rzn2h.h"
 #endif
#elif defined(BSP_CFG_CORE_CA55)
 #if defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h_ca55/flash_info_rzt2h_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n_ca55/flash_info_rzt2n_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h_ca55/flash_info_rzn2h_ca55.h"
 #endif
#endif

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
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
