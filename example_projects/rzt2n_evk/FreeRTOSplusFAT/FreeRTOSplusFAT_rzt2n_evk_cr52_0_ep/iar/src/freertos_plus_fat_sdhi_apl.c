/***********************************************************************************************************************
 * File Name    : freertos_plus_fat_sdhi_apl.c
 * Description  : Demonstrates FreeRTOS+FAT file operations using an SDHI-connected microSD card.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "common_data.h"

#define RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES  (10240U)
#define RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER   (0)
#define TRANSFER_LENGTH                                  (16)
#define SAMPLE_FILE_NAME                                 "TESTFILE.txt"
#define SAMPLE_PROGRAM_NAME                              "FreeRTOS+FAT SDHI"
#define DISK_MEMORY                                      "microSD card"

typedef enum
{
    STATE_INIT,
    STATE_FORMAT,
    STATE_MOUNT,
    STATE_DATA_WRITE,
    STATE_FILE_READ,
    STATE_REMOVE,
    STATE_FINISH,
    STATE_NONE,
} state_t;

void handle_error(fsp_err_t err);
void handle_ff_error(FF_Error_t ff_err);
void freertos_plus_fat_sdhi_example(void);
void user_uart_callback(uart_callback_args_t * p_args);
void rm_freertos_plus_fat_sdhi_callback(rm_freertos_plus_fat_callback_args_t * p_args);

uint8_t g_file_data[RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES] BSP_PLACE_IN_SECTION(".noncache_buffer");
uint8_t g_read_buffer[RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES] BSP_PLACE_IN_SECTION(".noncache_buffer");
uint8_t g_out_of_band_received[TRANSFER_LENGTH];

volatile uint32_t g_transfer_complete = 0U;
volatile uint32_t g_receive_complete = 0U;
uint32_t g_out_of_band_index = 0U;

volatile uint32_t g_rm_freertos_plus_fat_insertion_events = 0U;
volatile uint32_t g_rm_freertos_plus_fat_removal_events = 0U;
volatile uint32_t g_sdmmc_transfer_complete = 0U;

char gbuff[16];

static uint16_t g_state = STATE_NONE;

/*******************************************************************************************************************//**
 * Demonstrate FreeRTOS+FAT file operations on an SDHI-connected microSD card.
 **********************************************************************************************************************/
void freertos_plus_fat_sdhi_example(void)
{
    uint16_t i;
    uint16_t k;
    uint16_t ascii_code = 33U;
    fsp_err_t err;
    FF_FILE * px_source_file;
    FF_Disk_t disk;
    FF_Error_t ff_err;

    /* Create 10240 bytes of test data using ASCII codes 33 through 126 and clear the read buffer. */
    for (i = 0U; i < RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES; i++)
    {
        g_read_buffer[i] = 0U;
        g_file_data[i] = (uint8_t) ascii_code;
        ascii_code++;

        if (ascii_code > 126U)
        {
            ascii_code = 33U;
        }
    }

    g_state = STATE_INIT;

    /* Open the SCI UART driver. */
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_error(err);

    APP_PRINT(BANNER_INFO,
              EP_VERSION,
              4, 2, 0);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example detects a microSD card, optionally formats it as FAT32, mounts the disk, writes and "
              "verifies a 10240-byte file, then unmounts the disk for safe removal.\r\n");

    /* Open the media driver. */
    err = RM_FREERTOS_PLUS_FAT_Open(&g_rm_freertos_plus_fat0_ctrl, &g_rm_freertos_plus_fat0_cfg);
    handle_error(err);

    printf("\r\n==============================================\r\n");
    printf("    ~ Start of %s Sample ~\r\n", SAMPLE_PROGRAM_NAME);
    printf("==============================================\r\n");

    while (1)
    {
        switch (g_state)
        {
            case STATE_INIT:
            {
                printf("[INFO] Waiting for %s insertion...\r\n", DISK_MEMORY);

                while (0U == g_rm_freertos_plus_fat_insertion_events)
                {
                    /* Wait for a card insertion. */
                }

                printf("[INFO] %s inserted successfully!\r\n\r\n", DISK_MEMORY);

                /*
                 * Initialize the media and disk. If the media is removable, it must be inserted before
                 * RM_FREERTOS_PLUS_FAT_MediaInit is called.
                 */
                err = RM_FREERTOS_PLUS_FAT_MediaInit(&g_rm_freertos_plus_fat0_ctrl,
                                                     &g_rm_freertos_plus_fat0_disk_cfg.device);
                handle_error(err);

                err = RM_FREERTOS_PLUS_FAT_DiskInit(&g_rm_freertos_plus_fat0_ctrl,
                                                    &g_rm_freertos_plus_fat0_disk_cfg,
                                                    &disk);
                handle_error(err);

                printf("[INFO] To mount the %s, it needs to be partitioned and formatted.\r\n", DISK_MEMORY);
                printf("[PROMPT] Do you want to format the %s?\r\n", DISK_MEMORY);
                printf("  [1] Format the %s (FAT32)\r\n", DISK_MEMORY);
                printf("  [2] Mount without formatting\r\n");
                printf("  [3] Cancel and exit program\r\n");
                printf("Enter your choice >\r\n");

                gets(gbuff);
                printf("You selected: %s\r\n\r\n", gbuff);

                if ('1' == gbuff[0])
                {
                    g_state = STATE_FORMAT;
                }
                else if ('2' == gbuff[0])
                {
                    g_state = STATE_MOUNT;
                }
                else
                {
                    g_state = STATE_REMOVE;
                }

                break;
            }

            case STATE_FORMAT:
            {
                FF_PartitionParameters_t partition_params;

                printf("[INFO] Starting %s format (FAT32)...\r\n", DISK_MEMORY);
                printf("[INFO] Please wait while formatting...\r\n");

                /* Partition and format the disk, then continue to mount it. */
                memset(&partition_params, 0, sizeof(partition_params));
                memset(partition_params.xSizes, 0, sizeof(partition_params.xSizes));

                partition_params.ulSectorCount = g_rm_freertos_plus_fat0_disk_cfg.device.sector_count;
                partition_params.ulHiddenSectors = 2048U;
                partition_params.ulInterSpace = 0U;
                partition_params.xSizes[RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER] = 100;
                partition_params.xPrimaryCount = 1;
                partition_params.eSizeType = eSizeIsPercent;

                ff_err = FF_Partition(&disk, &partition_params);
                handle_ff_error(ff_err);

                ff_err = FF_FormatDisk(&disk,
                                       RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER,
                                       pdFALSE,
                                       pdFALSE,
                                       "MY_DISK");
                handle_ff_error(ff_err);

                if (STATE_FINISH != g_state)
                {
                    printf("[SUCCESS] %s formatted successfully.\r\n\r\n", DISK_MEMORY);
                    g_state = STATE_MOUNT;
                }

                break;
            }

            case STATE_MOUNT:
            {
                printf("[INFO] Mounting %s...\r\n", DISK_MEMORY);

                ff_err = FF_Mount(&disk, RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER);

                if (FF_isERR((uint32_t) ff_err))
                {
                    printf("[ERROR] Failed to mount the %s. It may not be formatted.\r\n", DISK_MEMORY);
                    printf("        FF_Mount failed. Error code = %ld (0x%08lX)\r\n", ff_err, ff_err);
                    g_state = STATE_REMOVE;
                    break;
                }

                printf("[SUCCESS] %s mounted successfully.\r\n\r\n", DISK_MEMORY);

                g_state = STATE_DATA_WRITE;
                break;
            }

            case STATE_DATA_WRITE:
            {
                printf("[INFO] Creating file [%s]...\r\n", SAMPLE_FILE_NAME);
                printf("[INFO] Writing 10240 bytes (ASCII 33 to 126)...\r\n");
                printf("[WARNING] Do NOT remove the %s during operation.\r\n", DISK_MEMORY);

                /* Add the disk to the file system. */
                FF_FS_Add("/", &disk);

                /* Open a source file for writing. */
                px_source_file = ff_fopen((const char *) SAMPLE_FILE_NAME, "w");

                if (NULL == px_source_file)
                {
                    printf("[ERROR] Failed to open file for writing!\r\n\r\n");
                    g_state = STATE_FINISH;
                    break;
                }

                /* Write and close the file. */
                ff_fwrite(g_file_data, sizeof(g_file_data), 1, px_source_file);
                ff_fclose(px_source_file);

                printf("[SUCCESS] Write operation completed.\r\n\r\n");

                g_state = STATE_FILE_READ;
                break;
            }

            case STATE_FILE_READ:
            {
                printf("[INFO] Reading and verifying data from [%s]...\r\n", SAMPLE_FILE_NAME);
                printf("[WARNING] Do NOT remove the %s during operation.\r\n", DISK_MEMORY);

                px_source_file = ff_fopen((const char *) SAMPLE_FILE_NAME, "r");

                if (NULL == px_source_file)
                {
                    printf("[ERROR] Failed to open file for reading!\r\n\r\n");
                    g_state = STATE_FINISH;
                    break;
                }

                /* Read and close the file. */
                ff_fread(g_read_buffer, sizeof(g_file_data), 1, px_source_file);
                ff_fclose(px_source_file);

                for (k = 0U; k < RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES; k++)
                {
                    if (g_read_buffer[k] != g_file_data[k])
                    {
                        printf("[ERROR] Verify test failed.\r\n\r\n");
                        g_state = STATE_FINISH;
                        break;
                    }
                }

                if (STATE_FINISH != g_state)
                {
                    printf("[SUCCESS] Verify test passed!\r\n\r\n");
                    g_state = STATE_REMOVE;
                }

                break;
            }

            case STATE_REMOVE:
            {
                FF_Unmount(&disk);

                printf("[INFO] %s can now be safely removed.\r\n", DISK_MEMORY);

                while (0U == g_rm_freertos_plus_fat_removal_events)
                {
                    /* Wait for a card removal interrupt. */
                }

                printf("[INFO] %s removed.\r\n", DISK_MEMORY);

                g_state = STATE_FINISH;
                break;
            }

            case STATE_FINISH:
            case STATE_NONE:
            default:
            {
                break;
            }
        }

        if (STATE_FINISH == g_state)
        {
            printf("\r\n============================================\r\n");
            printf("     ~ End of %s Sample ~\r\n", SAMPLE_PROGRAM_NAME);
            printf("============================================\r\n");
            break;
        }
    }
}

/*******************************************************************************************************************//**
 * Media callback invoked when a removable device is inserted or removed.
 **********************************************************************************************************************/
void rm_freertos_plus_fat_sdhi_callback(rm_freertos_plus_fat_callback_args_t * p_args)
{
    if (0U != (p_args->event & RM_FREERTOS_PLUS_FAT_EVENT_MEDIA_INSERTED))
    {
        g_rm_freertos_plus_fat_insertion_events = 1U;
    }

    if (0U != (p_args->event & RM_FREERTOS_PLUS_FAT_EVENT_MEDIA_REMOVED))
    {
        g_rm_freertos_plus_fat_removal_events = 1U;
    }
}

/*******************************************************************************************************************//**
 * SCI UART callback.
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            if (sizeof(g_out_of_band_received) > g_out_of_band_index)
            {
                if (UART_DATA_BITS_8 >= g_uart0_cfg.data_bits)
                {
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
#if (TRANSFER_LENGTH >= 2)
                else if ((sizeof(g_out_of_band_received) - g_out_of_band_index) >= sizeof(uint16_t))
                {
                    uint16_t received_data = (uint16_t) p_args->data;

                    memcpy(&g_out_of_band_received[g_out_of_band_index],
                           &received_data,
                           sizeof(received_data));

                    g_out_of_band_index += sizeof(received_data);
                }
#endif
                else
                {
                    /* Ignore data that does not fit in the receive buffer. */
                }
            }

            break;
        }

        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1U;
            break;
        }

        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1U;
            break;
        }

        default:
        {
            break;
        }
    }
}

/*******************************************************************************************************************//**
 * Handle an FSP driver error.
 **********************************************************************************************************************/
void handle_error(fsp_err_t err)
{
    if (FSP_SUCCESS != err)
    {
        printf("[ERROR] Function execution failure.\r\n");
        g_state = STATE_FINISH;
    }
}

/*******************************************************************************************************************//**
 * Handle a FreeRTOS+FAT error.
 **********************************************************************************************************************/
void handle_ff_error(FF_Error_t ff_err)
{
    if (FF_isERR((uint32_t) ff_err))
    {
        printf("[ERROR] FF_Function execution failure.\r\n");
        printf("        Error code = %ld (0x%08lX)\r\n", ff_err, ff_err);
        g_state = STATE_FINISH;
    }
}
