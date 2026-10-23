/***********************************************************************************************************************
 * File Name    : hmsc_baremetal_apl.c
 * Description  : Contains USB HMSC and FreeRTOS+FAT example processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "common_data.h"

void handle_error(fsp_err_t err);
void handle_ff_error(FF_Error_t ff_err);
void usb_hmsc_baremetal_example(void);
void user_uart_callback(uart_callback_args_t * p_args);

#define RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES    (10240U)
#define RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER     (0)
#define TRANSFER_LENGTH                                   (16U)
#define SAMPLE_FILE_NAME                                  "TESTFILE.txt"
#define SAMPLE_PROGRAM_NAME                               "USB HMSC"
#define DISK_MEMORY                                       "USB memory"

typedef enum
{
    STATE_INIT, STATE_FORMAT, STATE_MOUNT, STATE_DATA_WRITE, STATE_FILE_READ, STATE_REMOVE, STATE_FINISH, STATE_NONE,
} state_t;

uint8_t g_file_data[RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES]
    BSP_PLACE_IN_SECTION(".noncache_buffer");
uint8_t g_read_buffer[RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES]
    BSP_PLACE_IN_SECTION(".noncache_buffer");
uint8_t  g_out_of_band_received[TRANSFER_LENGTH];
volatile uint32_t g_transfer_complete = 0U;
volatile uint32_t g_receive_complete  = 0U;
uint32_t g_out_of_band_index = 0U;

extern uint8_t g_format_flag;

char gbuff[16];

static uint16_t g_state = STATE_NONE;

void usb_hmsc_baremetal_example(void)
{
    uint16_t   i;
    uint16_t   k;
    uint16_t ascii_code = 33U;
    fsp_err_t  err;
    FF_FILE *  pxSourceFile;
    FF_Disk_t  disk;
    FF_Error_t ff_err;
    usb_event_info_t event_info;
    usb_status_t     event = USB_STATUS_NONE;

    /* Create file data(ASCII code 33 to 126, 10240 bytes) and init read buffer */
    for (i = 0; i < RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES; i++)
    {
        g_read_buffer[i] = 0U;

        g_file_data[i]   = (uint8_t) ascii_code;
        ascii_code++;

        if (ascii_code > 126U)
        {
            ascii_code = 33U; /* ASCII code [!]. */
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
    APP_PRINT("This example demonstrates USB HMSC mass-storage access using FreeRTOS+FAT.\r\n");
    APP_PRINT("It can format or mount USB memory, write TESTFILE.txt, read it back, and verify the data.\r\n");

    /* Open the media driver. */
    err = RM_FREERTOS_PLUS_FAT_Open(&g_rm_freertos_plus_fat0_ctrl,
                                    &g_rm_freertos_plus_fat0_cfg);
    handle_error(err);

    printf("\r\n====================================\r\n");
    printf("    ~ Start of %s Sample ~\r\n", SAMPLE_PROGRAM_NAME);
    printf("====================================\r\n");

    while (1)
    {
        switch (g_state)
        {
            case STATE_INIT :
            {
                printf("[INFO] Waiting for %s insertion...\r\n", DISK_MEMORY);

                while (USB_STATUS_CONFIGURED != event)
                {
                    R_USB_EventGet(&event_info, &event);
                }
                g_rm_block_media0_ctrl.device_address = event_info.device_address;

                printf("[INFO] %s inserted successfully!\r\n\r\n", DISK_MEMORY);

                /* Initialize the media and the disk. If the media is removable, it must be inserted before calling
                 * RM_FREERTOS_PLUS_FAT_MediaInit. */
                err = RM_FREERTOS_PLUS_FAT_MediaInit(&g_rm_freertos_plus_fat0_ctrl,
                                                       &g_rm_freertos_plus_fat0_disk_cfg.device);
                handle_error(err);

                /* Initialize one disk for each partition used in the application. */
                err = RM_FREERTOS_PLUS_FAT_DiskInit(&g_rm_freertos_plus_fat0_ctrl,
                                                    &g_rm_freertos_plus_fat0_disk_cfg,
                                                    &disk);
                handle_error(err);

                printf("[INFO] To mount the %s, it needs to be partitioned and formatted.\r\n",
                       DISK_MEMORY);
                printf("[PROMPT] Do you want to format the %s?\r\n", DISK_MEMORY);
                printf("  [1] Format the %s (FAT32)\r\n", DISK_MEMORY);
                printf("  [2] Mount without formatting\r\n");
                printf("  [3] Cancel and exit program\r\n");
                printf("Enter your choice >\r\n");
                if (NULL == fgets(gbuff, sizeof(gbuff), stdin))
                {
                    gbuff[0] = '\0';
                }
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
            case STATE_FORMAT :
            {
                printf("[INFO] Starting %s format (FAT32)...\r\n", DISK_MEMORY);
                printf("[INFO] Please wait while formatting...\r\n");

                /* The disk is likely not formatted. Partition and format the disk, then mount again. */
                FF_PartitionParameters_t partition_params;
                memset(&partition_params, 0, sizeof(partition_params));
                memset(partition_params.xSizes, 0, sizeof(partition_params.xSizes));

                partition_params.ulSectorCount   = g_rm_freertos_plus_fat0_disk_cfg.device.sector_count;
                partition_params.ulHiddenSectors = 2048;
                partition_params.ulInterSpace    = 0;
                partition_params.xSizes[RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER] = 100;
                partition_params.xPrimaryCount   = 1;
                partition_params.eSizeType       = eSizeIsPercent;

                ff_err = FF_Partition(&disk, &partition_params);
                handle_ff_error(ff_err);

                g_format_flag = 1;
                ff_err = FF_FormatDisk(&disk,
                                       RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER,
                                       pdFALSE,
                                       pdFALSE,
                                       "MY_DISK");
                g_format_flag = 0;
                handle_ff_error(ff_err);

                if (STATE_FINISH != g_state)
                {
                    printf("[SUCCESS] %s formatted successfully.\r\n\r\n", DISK_MEMORY);
                    g_state = STATE_MOUNT;
                }

                break;
            }
            case STATE_MOUNT :
            {
                printf("[INFO] Mounting %s...\r\n", DISK_MEMORY);

                ff_err = FF_Mount(&disk, RM_FREERTOS_PLUS_FAT_EXAMPLE_PARTITION_NUMBER);

                if (FF_isERR((uint32_t) ff_err))
                {
                    printf("[ERROR] Failed to mount the %s. It may not be formatted.\r\n", DISK_MEMORY);
                    printf("        FF_Mount failed. Error code = %ld (0x%08lX)\r\n",
                       (long) ff_err,
                       (unsigned long) ff_err);
                    g_state = STATE_REMOVE;
                    break;
                }

                printf("[SUCCESS] %s mounted successfully.\r\n\r\n", DISK_MEMORY);

                g_state = STATE_DATA_WRITE;

                break;
            }
            case STATE_DATA_WRITE :
            {
                printf("[INFO] Creating file [%s]...\r\n", SAMPLE_FILE_NAME);
                printf("[INFO] Writing 10240 bytes (ASCII 33 to 126)...\r\n");
                printf("[WARNING] Do NOT remove the %s during operation.\r\n", DISK_MEMORY);

                /* Add the disk to the file system. */
                FF_FS_Add("/", &disk);

                /* Open a source file for writing. */
                pxSourceFile = ff_fopen((const char *) SAMPLE_FILE_NAME, "w");
                if (NULL == pxSourceFile)
                {
                    printf("[ERROR] Failed to open file for writing!\r\n\r\n");
                    g_state = STATE_FINISH;
                    break;
                }

                /* Write file data. */
                ff_fwrite(g_file_data, sizeof(g_file_data), 1, pxSourceFile);

                /* Close the file. */
                ff_fclose(pxSourceFile);

                printf("[SUCCESS] Write operation completed.\r\n\r\n");

                g_state = STATE_FILE_READ;
                break;
            }
            case STATE_FILE_READ :
            {
                printf("[INFO] Reading and verifying data from [%s]...\r\n", SAMPLE_FILE_NAME);
                printf("[WARNING] Do NOT remove the %s during operation.\r\n", DISK_MEMORY);

                pxSourceFile = ff_fopen((const char *) SAMPLE_FILE_NAME, "r");
                if (NULL == pxSourceFile)
                {
                    printf("[ERROR] Failed to open file for reading!\r\n\r\n");
                    g_state = STATE_FINISH;
                    break;
                }

                /* Read file data. */
                ff_fread(g_read_buffer, sizeof(g_file_data), 1, pxSourceFile);

                /* Close the file. */
                ff_fclose(pxSourceFile);

                for (k = 0; k < RM_FREERTOS_PLUS_FAT_EXAMPLE_BUFFER_SIZE_BYTES; k++)
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
            case STATE_REMOVE :
            {
                FF_Unmount(&disk);

                printf("[INFO] %s can now be safely removed.\r\n", DISK_MEMORY);

                while (USB_STATUS_DETACH != event)
                {
                    R_USB_EventGet(&event_info, &event);
                }

                RM_FREERTOS_PLUS_FAT_DiskDeinit(&g_rm_freertos_plus_fat0_ctrl, &disk);

                printf("[INFO] %s removed.\r\n", DISK_MEMORY);

                g_state = STATE_FINISH;
                break;
            }
        }

        if (STATE_FINISH == g_state)
        {
            printf("\r\n====================================\r\n");
            printf("     ~ End of %s Sample ~\r\n", SAMPLE_PROGRAM_NAME);
            printf("====================================\r\n");
            break;
        }
    }
}

void user_uart_callback(uart_callback_args_t * p_args)
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            /* Only put the next character in the receive buffer if there is space for it */
            if (sizeof(g_out_of_band_received) > g_out_of_band_index)
            {
                /* Write either the next one or two bytes depending on the receive data size */
                if (UART_DATA_BITS_8 >= g_uart0_cfg.data_bits)
                {
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
                else if ((g_out_of_band_index + sizeof(uint16_t)) <= sizeof(g_out_of_band_received))
                {
                    uint16_t received_data = (uint16_t) p_args->data;

                    memcpy(&g_out_of_band_received[g_out_of_band_index],
                           &received_data,
                           sizeof(received_data));
                    g_out_of_band_index += sizeof(received_data);
                }
                else
                {
                    /* No room remains for a 9-bit receive value. */
                }
            }
            break;
        }
        /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1U;
            break;
        }
        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1U;
            break;
        }
        default:
        {
        }
    }
}

/* Error handler */
void handle_error(fsp_err_t err)
{
    /* Halt the debugger. */
    if (FSP_SUCCESS != err)
    {
        printf("[ERROR] Function execution failure.\r\n");
        g_state = STATE_FINISH;
    }
}

void handle_ff_error(FF_Error_t ff_err)
{
    /* Halt the debugger. */
    if (FF_isERR((uint32_t) ff_err))
    {
        printf("[ERROR] FF_Function execution failure.\r\n");
        printf("        Error code = %ld (0x%08lX)\r\n",
               (long) ff_err,
               (unsigned long) ff_err);
        g_state = STATE_FINISH;
    }
}
