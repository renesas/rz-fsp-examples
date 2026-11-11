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
#ifndef FLASH_INFO_RZT2N_CA55_H__
#define FLASH_INFO_RZT2N_CA55_H__

#include "bsp_api.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/* Flash area used by the target device. */
/* Use flash : 1, Don't use flash : 0 */
#define USE_FLASH_QSPI_XSPI0    (0)
#define USE_FLASH_QSPI_XSPI1    (1)
#define USE_FLASH_OSPI_XSPI0    (1)
#define USE_FLASH_OSPI_XSPI1    (0)
#define USE_FLASH_NOR_CS0       (0)

/* xSPI0 flash area address */
#define XSPI0_START_ADDR        ((uint32_t)0x40000000UL)
#define XSPI0_END_ADDR          ((uint32_t)0x4FFFFFFFUL)

/* xSPI1 flash area address */
#define XSPI1_START_ADDR        ((uint32_t)0x50000000UL)
#define XSPI1_END_ADDR          ((uint32_t)0x5FFFFFFFUL)

/* NOR flash area address (CS0 space) */
#define NOR_CS0_START_ADDR      ((uint32_t)0x70000000UL)
#define NOR_CS0_END_ADDR        ((uint32_t)0x73FFFFFFUL)

/* Sector size */
#define XSPI_SECTOR_SIZE        (0x1000U)
#define NOR_SECTOR_SIZE         (0x20000U)

/* Mask value for erase */
#define XSPI_ERASE_MASK         ((uint32_t)0xFFFFF000UL)
#define NOR_ERASE_MASK          ((uint32_t)0xFFFE0000UL)

/* Flash backup buffer address */
/* Set this to 0 since no writing is performed in the SSBL. */
#define BACKUP_BUFFER_ADDR      ((uint32_t)0x00000000UL)

/* Flash mirror area offset value */
/* In the CA55 project, the mirror address is calculated from the BSP MMU settings. */
/* Therefore, set this constant to 0. */
#define FLASH_MIRROR_OFFSET     ((uint32_t)0x00000000UL)

/* xSPI flash write unit */
#define XSPI_WRITE_SIZE         (0x8U)
#define XSPI_WRITE_MASK         ((uint32_t)0xFFFFFFF8UL)

/* NOR flash write uint */
#define NOR_WRITE_SIZE          (0x100U)

/******************************************************************************
 * @fn       __get_ospi_flash_mirror_addr
 * @brief    Get the mirror address of OSPI flash.
 *
 * @param[in]  addr          Flash address
 *
 * @retval Flash mirror address
 ******************************************************************************/
__STATIC_FORCEINLINE  uint32_t __get_ospi_flash_mirror_addr(uint32_t addr)
{
    uint32_t offset = addr - BSP_CFG_EL3_MMU_REGION06_PADDR;
    
    return (BSP_CFG_EL3_MMU_REGION06_VADDR + offset);
}

/******************************************************************************
 * @fn       __get_qspi_flash_mirror_addr
 * @brief    Get the mirror address of QSPI flash.
 *
 * @param[in]  addr          Flash address
 *
 * @retval Flash mirror address
 ******************************************************************************/
__STATIC_FORCEINLINE  uint32_t __get_qspi_flash_mirror_addr(uint32_t addr)
{
    uint32_t offset = addr - BSP_CFG_EL3_MMU_REGION12_PADDR;
    
    return (BSP_CFG_EL3_MMU_REGION12_VADDR + offset);
}

/******************************************************************************
 * @fn       __get_nor_flash_mirror_addr
 * @brief    Get the mirror address of NOR flash.
 *
 * @param[in]  addr          Flash address
 *
 * @retval Flash mirror address
 ******************************************************************************/
__STATIC_FORCEINLINE  uint32_t __get_nor_flash_mirror_addr(uint32_t addr)
{
    /* This flash is not supported. */
    /* Returns addr unchanged. */
    return addr;
}

#endif /* FLASH_INFO_RZT2N_CA55_H__ */
