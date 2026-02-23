/***********************************************************************************************************************
 * File Name    : bsc_ep.h
 * Description  : Contains declarations of data structures and functions used in bsc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef BSC_EP_H_
#define BSC_EP_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Macros for commands to be received through RTT input */
#define ERASE        (1)
#define WRITE        (2)
#define WRITE_BUFF   (3)
#define EXIT         (4)

#define CS0_BASE     (0x50000000) /* Use non-cacheable mirror address */
#define SECTOR_ERASE (0)
#define CHIP_ERASE   (1)

#define DATA_NUM     (256)

/* Parallel NOR Flash command */
#define FLASH_A_SEQ_ADD1 (0x555U)
#define FLASH_A_SEQ_ADD2 (0x2AAU)

#define FLASH_CODE1        (0x00AAU)
#define FLASH_CODE2        (0x0055U)
#define WRITE_CODE         (0x00A0U)
#define WRITE_BUF_CODE     (0x0025U)
#define WRITE_BUF_CODE2    (0x0029U)
#define ERASE_SECTOR_CODE1 (0x0080U)
#define ERASE_SECTOR_CODE2 (0x0030U)
#define ERASE_SECTOR_CHIP  (0x0010U)

#define R_NOR_SECT_ADDR_MSK (0x0001FFFFU)

#define STATUS_REGISTER_Q6BIT (1U << 6U)

/* Macros for menu options to be displayed */
#define MENUOPTION      "\r\nSelect from the below Menu options.\
                        \r\nPress 1 to erase data from sector of external NOR flash memory. Verify data after erase completion.\
                        \r\nPress 2 to write a single byte to a defined memory sector and read it back after writing.\
                        \r\nPress 3 to write 256 bytes to a defined memory sector and read them back after writing.\
                        \r\nPress 4 to exit.\r\n"

#define EP_INFO         "\r\nThis example project demonstrates basic functionalities of BSC driver."\
                        "\r\nProject initializes a BSC instance. Upon successful initialization, User has the provision to input value"\
                        "\r\nas per displayed menu through JLinkRTTViewer to select options(Erase, Write, WriteBuf)."\
                        "\r\nOn user input 1, BSC driver erases data from a sector of the external NOR flash memory and verifies the result."\
                        "\r\nOn user input 2, BSC driver writes a single byte data to a defined memory sector and reads it back."\
                        "\r\nOn user input 3, BSC driver writes 256 bytes data to a defined memory sector and reads them back."\
                        "\r\nOutput will be displayed on JLinkRTTViewer.\r\n"

/* MACROs for Null character */
#define NULL_CHAR ('\0')

/* Function declaration */
/* Initialize bsc module */
fsp_err_t bsc_init (void);

/* Erase sector of flash memory */
fsp_err_t bsc_erase (void);

/* Write data to sector of flash memory */
fsp_err_t bsc_write (void);

/* Write bytes of data to sector of flash memory */
fsp_err_t bsc_write_buff (void);

/* De-initialize the opened bsc module */
fsp_err_t bsc_deinit (void);

#endif /* BSC_EP_H_ */
