/***********************************************************************************************************************
 * File Name    : rm_block_media_ram_disk.h
 * Description  : Contains RAM-disk block-media definitions and declarations.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#ifndef RAM_DISK_H 
#define RAM_DISK_H

/***********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 ***********************************************************************************************************************/
/* Fixed width integers. */
#include <stdint.h>

/* RAM disk configuration */
#include "r_usb_basic_cfg.h"
//@@#include "r_usb_media_driver_if.h"

/***********************************************************************************************************************
 Macro definitions
 ***********************************************************************************************************************/
//#define USB_MEDIA_ADDRESS               (0x20000000ul)              /* Media Head Address */
//#define STRG_MEDIASIZE                  (96ul * 1024ul)     /* Media size */
//#define STRG_TOTALSECT                  (STRG_MEDIASIZE / STRG_SECTSIZE)
//
///* #define RAMDISK_MEDIATYPE              (0xF8u)    Fixed media
// * #define RAMDISK_MEDIATYPE              (0xF0u)    Removable media */
//#define RAMDISK_MEDIATYPE               (0xF8u)
//
//#define RAMDISK_SIGNATURE               (0xAA55u)
//
//#define RAMDISK_CLSTSIZE                (0x01u)
//#define RAMDISK_FATNUM                  (0x02u)

/*
 If the number of data areas of clusters is smaller
 than that of value 4085(4096-11), it is FAT12.
 If the number of data areas of clusters is smaller
 than that of value 65525(65536-11), it is FAT16.
 Otherwise it is FAT32.
 */

//yama
//#define USB_MEDIA_ADDRESS              (0x20008000UL)  /* Media Head Address */
#define STRG_MEDIASIZE                 (64UL * 1024UL) //(64UL * 1024UL) /* Media size */
#define STRG_TOTALSECT                 (STRG_MEDIASIZE / STRG_SECTSIZE)

#if STRG_TOTALSECT < 4096
    #define RAMDISK_FATLENGTH           (0x155ul)     /* FAT12 */
#else   /* STRG_TOTALSECT < 4096 */
    #if STRG_TOTALSECT < 65536
        #define RAMDISK_FATLENGTH       (0x100ul)     /* FAT16 */
    #else   /* STRG_TOTALSECT < 65536 */
        #define RAMDISK_FATLENGTH       (0x80ul)     /* FAT32 */
    #endif  /* STRG_TOTALSECT < 65536 */
#endif  /* STRG_TOTALSECT < 4096 */

#define RAMDISK_FATSIZE   (((STRG_TOTALSECT - 8) / RAMDISK_FATLENGTH) + 1)


/***********************************************************************************************************************
 Typedef definitions
 ***********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 ************************************************************************************************************************/

extern const uint8_t g_ram_disk_boot_sector[];
extern const uint8_t g_usb_pmsc_table1[];
extern const uint8_t g_usb_pmsc_tablefat1[];
extern const uint8_t g_usb_pmsc_tablefat2[];
extern const uint8_t g_usb_pmsc_rootdir[];

#endif /* RAM_DISK_H */

/***********************************************************************************************************************
End  Of File
***********************************************************************************************************************/
