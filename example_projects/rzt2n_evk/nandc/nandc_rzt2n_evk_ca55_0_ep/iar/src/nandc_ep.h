/***********************************************************************************************************************
 * File Name    : nandc_ep.h
 * Description  : Contains declarations of data structures and functions used in nandc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef NANDC_EP_H_
#define NANDC_EP_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdint.h>
#include "hal_data.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Number of main-data bytes in one NAND sector */
#define NAND_FLASH_SECTOR_BYTES               (2048U)  /* Keep synchronized with NANDC configuration sector_bytes */

/* Number of NAND sectors accessed per page */
#define NAND_FLASH_SECTOR_NUMBER              (1U)  /* Keep synchronized with NANDC configuration sector_number */

/* Number of application OOB bytes per page */
#define NAND_FLASH_OOB_DATA_BYTES             (24U)  /* Keep synchronized with NANDC configuration oob_data_size */

/* Number of ECC bytes generated per page */
#define NAND_FLASH_ECC_BYTES_PER_PAGE         (30U)  /* Keep synchronized with NANDC configuration
                                                        ecc_correction_ability */

/* Number of CRC bytes generated per page */
#define NAND_FLASH_CRC_BYTES_PER_PAGE         (8U)  /* Keep synchronized with NANDC configuration crc_enable */

/* Number of reserved Skip Byte marker bytes per page */
#define NAND_FLASH_SKIP_BYTES_SIZE            (2U)  /* Keep synchronized with NANDC configuration skip_bytes_size */

/* Number of NAND pages used by the example project */
#define NAND_FLASH_PAGE_COUNT                 (5U)

/* Number of pages contained in one NAND erase block */
#define NAND_FLASH_PAGES_PER_BLOCK            (64U)

/* Total main-data bytes contained in one logical page */
#define NAND_FLASH_MAIN_DATA_BYTES            (NAND_FLASH_SECTOR_NUMBER * NAND_FLASH_SECTOR_BYTES)

/* Total bytes read from all pages using normal read access */
#define NAND_FLASH_READ_TOTAL_BYTES           \
            (NAND_FLASH_PAGE_COUNT * (NAND_FLASH_MAIN_DATA_BYTES + NAND_FLASH_OOB_DATA_BYTES))

/* Total programmed bytes across all pages in the example project */
#define NAND_FLASH_PROGRAMMED_TOTAL_BYTES     \
            (NAND_FLASH_PAGE_COUNT * (NAND_FLASH_MAIN_DATA_BYTES + NAND_FLASH_OOB_DATA_BYTES))

/* Physical NAND main-area size per page */
#define NAND_FLASH_PHYSICAL_DATA_BYTES        (4096U)

/* Physical NAND spare-area size per page */
#define NAND_FLASH_PHYSICAL_SPARE_BYTES       (224U)

/* Total physical NAND page size including main and spare areas */
#define NAND_FLASH_PHYSICAL_PAGE_BYTES        (NAND_FLASH_PHYSICAL_DATA_BYTES + NAND_FLASH_PHYSICAL_SPARE_BYTES)

/* Total physical bytes erased when one NAND block is erased */
#define NAND_FLASH_ERASED_BYTES_PER_BLOCK     (NAND_FLASH_PAGES_PER_BLOCK * NAND_FLASH_PHYSICAL_PAGE_BYTES)

/* Total physical bytes accessed by a raw read or raw write operation */
#define NAND_FLASH_RAW_TOTAL_BYTES            (NAND_FLASH_PAGE_COUNT * NAND_FLASH_PHYSICAL_PAGE_BYTES)

/* Convert a block number and page index to a NAND row address */
#define NAND_FLASH_ROW_ADDRESS(block, page)   (((uint32_t) (block) * NAND_FLASH_PAGES_PER_BLOCK) + (uint32_t) (page))

/* NAND chip-select index used by the example project */
#define NAND_FLASH_CHIP_SELECT                (0U)

/* Maximum polling count while waiting for a NANDC callback event */
#define NANDC_CALLBACK_WAIT_LIMIT             (0x20000000UL)

/***********************************************************************************************************************
 * Public function declaration
 **********************************************************************************************************************/
/* Open and initialize the NAND controller instance */
fsp_err_t nandc_open (void);

/* Initialize the GPT timer used for execution time measurements */
fsp_err_t nandc_timer_init (void);

/* Erase one NAND block and measure the execution time */
fsp_err_t nandc_erase_block (uint32_t block, uint32_t * const p_time);

/* Write one or more NAND pages using normal access and measure the execution time */
fsp_err_t nandc_write_pages (uint8_t const * p_main_source,
                            uint8_t const * p_oob_source,
                            uint32_t row_address,
                            uint32_t page_count,
                            uint32_t * const p_time);

/* Read one or more NAND pages using normal access and measure the execution time */
fsp_err_t nandc_read_pages (uint32_t row_address,
                            uint32_t page_count,
                            uint8_t * p_main_destination,
                            uint8_t * p_oob_destination,
                            uint32_t * const p_time);

/* Write one or more physical NAND pages using raw access and measure the execution time */
fsp_err_t nandc_raw_write_pages (uint8_t const * p_source,
                                 uint32_t row_address,
                                 uint32_t page_count,
                                 uint32_t * const p_time);

/* Read one or more physical NAND pages using raw access and measure the execution time */
fsp_err_t nandc_raw_read_pages (uint32_t row_address,
                                uint32_t page_count,
                                uint8_t * p_destination,
                                uint32_t * const p_time);

/* Configure logical-to-physical row address remapping */
fsp_err_t nandc_remap_set (uint32_t logical_address, uint32_t physical_address);

/* Clear all configured row address remapping entries */
fsp_err_t nandc_remap_clear (void);

/* Enable write protection for the specified NAND address range */
fsp_err_t nandc_write_protect_enable (uint32_t start_address, uint32_t size, uint32_t index);

/* Disable write protection for the specified protection entry */
fsp_err_t nandc_write_protect_disable (uint32_t index);

/* Attempt a write operation to validate write protection behavior */
fsp_err_t nandc_write_protected_pages (uint8_t const * p_main_source,
                                       uint8_t const * p_oob_source,
                                       uint32_t row_address,
                                       uint32_t page_count);

/* Wait until the NAND controller reports a ready state */
fsp_err_t nandc_wait_controller_ready (void);

/* Compare two buffers and verify that their contents are identical */
fsp_err_t nandc_verify_buffer (uint8_t const * p_expected, uint8_t const * p_actual, uint32_t length);

/* Fill a buffer with a specified constant byte pattern */
void nandc_fill_constant_pattern (uint8_t * p_buffer, uint32_t length, uint8_t pattern);

/* Process NAND controller callback events */
void nandc_example_callback (nand_flash_callback_args_t * p_args);

#endif /* NANDC_EP_H_ */
