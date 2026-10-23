/***********************************************************************************************************************
 * File Name    : nandc_example.h
 * Description  : Contains declarations of data structures and functions used in nandc_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef NANDC_EXAMPLE_H_
#define NANDC_EXAMPLE_H_

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "nandc_ep.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Test patterns */
#define NANDC_PATTERN_BASIC_BASE                ((uint8_t) 'A')
#define NANDC_PATTERN_REMAP_A                   (0x5AU)
#define NANDC_PATTERN_REMAP_B                   (0xA5U)
#define NANDC_PATTERN_PROTECT                   (0x3CU)

/* NAND access regions used by this application */
#define NAND_FLASH_USED_PAGE                   (0U)

/* NAND block used for normal erase, write, read, and raw-access demonstrations */
#define NAND_FLASH_ACCESS_BLOCK                (0U)

/* Row address of the page used by the Basic Demo operations */
#define NAND_FLASH_ACCESS_ROW_ADDRESS          \
    NAND_FLASH_ROW_ADDRESS(NAND_FLASH_ACCESS_BLOCK, NAND_FLASH_USED_PAGE)

/* NAND block contains physical Page A used by the remap demonstration */
#define NANDC_REMAP_LOGICAL_BLOCK              (1U)

/* NAND block contains physical Page B used by the remap demonstration */
#define NANDC_REMAP_PHYSICAL_BLOCK             (2U)

/* NAND Logical row address used by the remap demonstration */
#define NANDC_REMAP_LOGICAL_ROW_ADDRESS        \
    NAND_FLASH_ROW_ADDRESS(NANDC_REMAP_LOGICAL_BLOCK, NAND_FLASH_USED_PAGE)

/* NAND Physical row address mapped to the logical address during remap testing */
#define NANDC_REMAP_PHYSICAL_ROW_ADDRESS       \
    NAND_FLASH_ROW_ADDRESS(NANDC_REMAP_PHYSICAL_BLOCK, NAND_FLASH_USED_PAGE)

/* NAND Block used by the write protection demonstration */
#define NANDC_PROTECT_BLOCK                    (3U)

/* Row address used for protected write and recovery testing */
#define NANDC_PROTECT_ROW_ADDRESS              \
    NAND_FLASH_ROW_ADDRESS(NANDC_PROTECT_BLOCK, NAND_FLASH_USED_PAGE)

/* Size of the NAND write-protected region in rows */
#define NANDC_PROTECT_REGION_SIZE              (32U)

/* Write-protection entry index used by the example project */
#define NANDC_PROTECT_INDEX                    (0U)

/* Main menu selections */
#define NANDC_MAIN_MENU_BASIC                  ('1')
#define NANDC_MAIN_MENU_REMAP                  ('2')
#define NANDC_MAIN_MENU_WRITE_PROTECT          ('3')

/* Basic Demo submenu selections */
#define NANDC_BASIC_MENU_WRITE                 ('1')
#define NANDC_BASIC_MENU_READ                  ('2')
#define NANDC_BASIC_MENU_ERASE                 ('3')
#define NANDC_BASIC_MENU_RAW_WRITE             ('4')
#define NANDC_BASIC_MENU_RAW_READ              ('5')
#define NANDC_BASIC_MENU_RETURN                ('6')

/***********************************************************************************************************************
 * Public function declaration
 **********************************************************************************************************************/
void nandc_example_entry (void);

#endif /* NANDC_EXAMPLE_H_ */
