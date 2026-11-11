/***********************************************************************************************************************
 * File Name    : flash_info_rzt2n.h
 * Description  : Contains data structures and functions used in flash info for RZT2N.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef FLASH_INFO_RZT2N_H__
#define FLASH_INFO_RZT2N_H__

/* Macro definitions */
/* Flash area used by the target device */
/* Use flash : 1, Don't use flash : 0 */
#define USE_FLASH_QSPI_XSPI0      (0)
#define USE_FLASH_QSPI_XSPI1      (1)
#define USE_FLASH_OSPI_XSPI0      (1)
#define USE_FLASH_OSPI_XSPI1      (0)
#define USE_FLASH_NOR_CS0         (0)

/* xSPI0 flash area address */
#define XSPI0_START_ADDR          ((uint32_t)0x40000000UL)
#define XSPI0_END_ADDR            ((uint32_t)0x4FFFFFFFUL)

/* xSPI1 flash area address */
#define XSPI1_START_ADDR          ((uint32_t)0x50000000UL)
#define XSPI1_END_ADDR            ((uint32_t)0x5FFFFFFFUL)

/* NOR flash area address (CS0 space) */
#define NOR_CS0_START_ADDR        ((uint32_t)0x70000000UL)
#define NOR_CS0_END_ADDR          ((uint32_t)0x73FFFFFFUL)

/* Sector size */
#define XSPI_SECTOR_SIZE          (0x1000U)
#define NOR_SECTOR_SIZE           (0x20000U)

/* Mask value for erase */
#define XSPI_ERASE_MASK           ((uint32_t)0xFFFFF000UL)
#define NOR_ERASE_MASK            ((uint32_t)0xFFFE0000UL)

/* Flash backup buffer address */
/* Use a maximum size of 0x20000 when writing to flash memory */
#define BACKUP_BUFFER_ADDR        ((uint32_t)0x100E0000UL)

/* Flash mirror area offset value */
/* The address after subtracting the offset value is used as the mirror area address */
/* If there is no mirror area, set it to 0 */
#define FLASH_MIRROR_OFFSET       ((uint32_t)0x00000000UL)

/* xSPI flash write unit */
#define XSPI_WRITE_SIZE           (0x8U)
#define XSPI_WRITE_MASK           ((uint32_t)0xFFFFFFF8UL)

/* NOR flash write uint */
#define NOR_WRITE_SIZE            (0x100U)

#endif /* FLASH_INFO_RZT2N_H__ */
