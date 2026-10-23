/***********************************************************************************************************************
 * File Name    : nandc_example.c
 * Description  : Contains data structures and functions used in nandc_example.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "nandc_example.h"

/***********************************************************************************************************************
 * @addtogroup r_nandc_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function declarations
 **********************************************************************************************************************/

/* Display the Main menu */
static void nandc_show_main_menu (void);

/* Display the Basic NANDC commands Demo submenu */
static void nandc_show_basic_command_menu (void);

/* Handle the Basic NANDC commands Demo submenu */
static void nandc_basic_command_menu (void);

/* Run the NANDC logical-to-physical remap demonstration */
static void nandc_run_remap_demo (void);

/* Run the NANDC write-protection demonstration */
static void nandc_run_write_protect_demo (void);

/* Erase the demo block and write normal NAND pages */
static fsp_err_t nandc_basic_write (uint32_t * const p_time);

/* Read and verify normal NAND pages */
static fsp_err_t nandc_basic_read (uint32_t * const p_time);

/* Erase the Basic Demo NAND block */
static fsp_err_t nandc_basic_erase (uint32_t * const p_time);

/* Write raw physical NAND pages */
static fsp_err_t nandc_basic_raw_write (uint32_t * const p_time);

/* Read raw physical NAND pages */
static fsp_err_t nandc_basic_raw_read (uint32_t * const p_time);

/* Print a summary of the raw-read buffer */
static void nandc_print_raw_page_summary (void);

/* Print operation result information */
static void nandc_print_result (char const * p_operation, fsp_err_t result);

/* Convert an FSP error code to a printable string */
static char const * nandc_result_string (fsp_err_t result);

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/

/* All NAND data buffers must be located in an uncached memory region */
/* Basic Demo main-data buffers */
static uint8_t g_basic_main_source[NAND_FLASH_PAGE_COUNT][NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_basic_main_destination[NAND_FLASH_PAGE_COUNT][NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/* Basic Demo OOB buffers */
static uint8_t g_basic_oob_source[NAND_FLASH_PAGE_COUNT][NAND_FLASH_OOB_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_basic_oob_destination[NAND_FLASH_PAGE_COUNT][NAND_FLASH_OOB_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/* RawRead returns complete physical NAND pages */
static uint8_t g_basic_raw_source[NAND_FLASH_PAGE_COUNT][NAND_FLASH_PHYSICAL_PAGE_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_basic_raw_destination[NAND_FLASH_PAGE_COUNT][NAND_FLASH_PHYSICAL_PAGE_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/* Remap Demo buffers */
static uint8_t g_remap_pattern_a[NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_remap_pattern_b[NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_remap_read_buffer[NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/*
 * Provide a valid OOB buffer even when Remap demo and write protection demo don't
 * actively use OOB data. This ensures consistent behavior
 * across CR52 and CA55 targets where DMA/MMU address
 * translation may be performed for OOB buffers.
 */
static uint8_t g_oob_source[NAND_FLASH_OOB_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_oob_destination[NAND_FLASH_OOB_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/* Write Protection Demo buffers */
static uint8_t g_protect_source[NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

static uint8_t g_protect_destination[NAND_FLASH_MAIN_DATA_BYTES]
BSP_PLACE_IN_SECTION(".noncache_buffer");

/* Flag indicates having programmed data */
static bool g_basic_data_programmed     = false;

/* Flag indicates having programmed raw data */
static bool g_basic_raw_data_programmed = false;

uint8_t  g_out_of_band_received[TRANSFER_LENGTH];
uint32_t volatile g_transfer_complete = 0;
uint32_t volatile g_receive_complete  = 0;
uint32_t g_out_of_band_index = 0;
/***********************************************************************************************************************
 * @brief       This function operates NANDC demo the main menu.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void nandc_example_entry(void)
{
    fsp_err_t err;
    fsp_pack_version_t version;
    uint32_t page;
    uint32_t byte;
    uint8_t rByte[BUF_SIZE] = {NULL_CHAR};

    /* Open SCI UART */
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        ERR_TRAP(err);
    }

    /* Get the FSP version */
    R_FSP_VersionGet(&version);

    /* Example project information printed on the console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor,
        version.version_id_b.patch);

    APP_PRINT(EP_INFO);

    /* Initialize g_basic_main_source, g_basic_raw_source to known data */
    for (page = 0U; page < NAND_FLASH_PAGE_COUNT; page++)
    {
        for (byte = 0U; byte < NAND_FLASH_MAIN_DATA_BYTES; byte++)
        {
            g_basic_main_source[page][byte] = (uint8_t) (NANDC_PATTERN_BASIC_BASE + (byte % 26U));
        }
    }
    
    for (page = 0U; page < NAND_FLASH_PAGE_COUNT; page++)
    {
        for (byte = 0U; byte < NAND_FLASH_PHYSICAL_PAGE_BYTES; byte++)
        {
            g_basic_raw_source[page][byte] = (uint8_t) (NANDC_PATTERN_BASIC_BASE + (byte % 26U));
        }
    }

    /*
     * Use erased OOB data unless the example explicitly demonstrates
     * application OOB content.
     */
    memset(g_basic_oob_source, 0xFF, sizeof(g_basic_oob_source));

    err = nandc_open();
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\n** R_NANDC_Open API failed **\r\n");
        APP_ERR_TRAP(err);
    }

    /* Initialize timer to measure NANDC operation */
    err = nandc_timer_init();
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\n** timer_init failed **\r\n");
        APP_ERR_TRAP(err);
    }
    
    nandc_show_main_menu();

    while (true)
    {
        /* Read first byte of data provided by user */
        APP_READ(rByte);

        /* Accept only a single character command */
        if ((rByte[0] == '\0') || ((rByte[1] != '\0') &&
           (rByte[1] != '\r') && (rByte[1] != '\n')))
        {
            APP_PRINT("\r\nInvalid selection.\r\n");
            continue;
        }

        /* Handle Main menu */
        switch ((char)rByte[0])
        {
            case NANDC_MAIN_MENU_BASIC:
            {
                nandc_basic_command_menu();
                break;
            }
            case NANDC_MAIN_MENU_REMAP:
            {
                nandc_run_remap_demo();
                break;
            }
            case NANDC_MAIN_MENU_WRITE_PROTECT:
            {
                nandc_run_write_protect_demo();
                break;
            }
            default:
            {
                APP_PRINT ("\r\nInvalid input. Provide a valid input\r\n");
                break;
            }
        }

        /* Reset buffer */
        memset(rByte, NULL_CHAR, BUF_SIZE);

        nandc_show_main_menu();
    }
}

/***********************************************************************************************************************
 * @brief       This function displays the main menu.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_show_main_menu(void)
{
    APP_PRINT("\r\n");
    APP_PRINT("Main menu\r\n");
    APP_PRINT("  1. Basic NANDC commands operation\r\n");
    APP_PRINT("  2. Logical-to-physical remap\r\n");
    APP_PRINT("  3. Write protection and recovery\r\n");
    APP_PRINT("Select option: ");
}

/***********************************************************************************************************************
 * @brief       This function displays the NAND commands menu.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_show_basic_command_menu(void)
{
    APP_PRINT("\r\n");
    APP_PRINT("Basic NANDC command Demo\r\n");
    APP_PRINT("  1. Write %u pages\r\n", (uint32_t) NAND_FLASH_PAGE_COUNT);
    APP_PRINT("  2. Read and verify %u pages\r\n", (uint32_t) NAND_FLASH_PAGE_COUNT);
    APP_PRINT("  3. Erase one block\r\n");
    APP_PRINT("  4. Raw write %u physical pages\r\n",(uint32_t) NAND_FLASH_PAGE_COUNT);
    APP_PRINT("  5. Raw read %u physical pages\r\n",(uint32_t) NAND_FLASH_PAGE_COUNT);
    APP_PRINT("  6. Return to main menu\r\n");
    APP_PRINT("Select option: ");
}

/***********************************************************************************************************************
 * @brief       This function handles the Basic NAND commands menu.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_basic_command_menu(void)
{
    bool      return_to_main = false;
    fsp_err_t result;
    uint32_t  execute_time = RESET_VALUE;
    uint8_t rByte[BUF_SIZE] = {NULL_CHAR};

    /*
     * Flush input left by the Main menu selection.
     * This prevents a CR or LF from being processed as a new submenu command.
     */
    /* Reset buffer */
    memset(rByte, NULL_CHAR, BUF_SIZE);

    nandc_show_basic_command_menu();

    while (!return_to_main)
    {
        /* Read first byte of data provided by user */
        APP_READ(rByte);

        /* Accept only a single character command */
        if ((rByte[0] == '\0') || ((rByte[1] != '\0') &&
           (rByte[1] != '\r') && (rByte[1] != '\n')))
        {
            APP_PRINT("\r\nInvalid selection.\r\n");
            continue;
        }

        switch ((char)rByte[0])
        {
            case NANDC_BASIC_MENU_WRITE:
            {
                result = nandc_basic_write(&execute_time);
                nandc_print_result("Basic write", result);
                if (FSP_SUCCESS == result)
                {
                    /* Print execution time */
                    APP_PRINT("Flash Write %d bytes completed successfully in %d nanoseconds\r\n",
                              (uint32_t)(NAND_FLASH_PROGRAMMED_TOTAL_BYTES), execute_time);
                }
                break;
            }
            case NANDC_BASIC_MENU_READ:
            {
                if (!g_basic_data_programmed)
                {
                    APP_PRINT("\r\nRun Write 5 pages before Read and Verify.\r\n");
                }
                else
                {
                    result = nandc_basic_read(&execute_time);
                    nandc_print_result("Basic read and verify", result);
                    if (FSP_SUCCESS == result)
                    {
                        /* Print execution time */
                        APP_PRINT("Flash Read %d bytes completed successfully in %d nanoseconds\r\n",
                                (uint32_t)(NAND_FLASH_READ_TOTAL_BYTES), execute_time);
                    }
                }
                break;
            }
            case NANDC_BASIC_MENU_ERASE:
            {
                result = nandc_basic_erase(&execute_time);
                nandc_print_result("Basic block erase", result);
                if (FSP_SUCCESS == result)
                {
                    /* Print execution time */
                    APP_PRINT("Flash Erase %d bytes completed successfully in %d nanoseconds\r\n",
                            (uint32_t)(NAND_FLASH_ERASED_BYTES_PER_BLOCK), execute_time);
                }
                break;
            }
            case NANDC_BASIC_MENU_RAW_WRITE:
            {
                result = nandc_basic_raw_write(&execute_time);
                nandc_print_result("Basic raw write", result);
                if (FSP_SUCCESS == result)
                {
                    /* Print execution time */
                    APP_PRINT("Flash Raw Write %d bytes completed successfully in %d nanoseconds\r\n",
                            (uint32_t)(NAND_FLASH_RAW_TOTAL_BYTES), execute_time);
                }
                break;
            }
            case NANDC_BASIC_MENU_RAW_READ:
            {
                if (!(g_basic_data_programmed || g_basic_raw_data_programmed))
                {
                    APP_PRINT( "\r\nRun Write 5 pages or Raw Write 5 pages before Raw Read.\r\n");
                }
                else
                {
                    result = nandc_basic_raw_read(&execute_time);
                    nandc_print_result("Basic raw read", result);
                    if (FSP_SUCCESS == result)
                    {
                        /* Print execution time */
                        APP_PRINT("Flash Raw Read %d bytes completed successfully in %d nanoseconds\r\n",
                                (uint32_t)(NAND_FLASH_RAW_TOTAL_BYTES), execute_time);
                        nandc_print_raw_page_summary();
                    }
                }
                break;
            }
            case NANDC_BASIC_MENU_RETURN:
            {
                APP_PRINT ("\r\nGo back to main menu\r\n");
                return_to_main = true;
                break;
            }

            default:
            {
                APP_PRINT ("\r\nInvalid input. Provide a valid input\r\n");
                break;
            }
        }

        /* Reset buffer */
        memset(rByte, NULL_CHAR, BUF_SIZE);

        if (!return_to_main)
        {
            nandc_show_basic_command_menu();
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function erases the demo block and writes normal NAND pages.
 * @param[OUT]  p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_basic_write(uint32_t * const p_time)
{
    fsp_err_t result;
    uint32_t  erase_time = RESET_VALUE;

    result = nandc_erase_block(NAND_FLASH_ACCESS_BLOCK, &erase_time);
    if (FSP_SUCCESS != result)
    {
        return result;
    }
    g_basic_data_programmed = false;
    g_basic_raw_data_programmed = false;

    result = nandc_write_pages(&g_basic_main_source[0][0],
                            &g_basic_oob_source[0][0],
                            NAND_FLASH_ACCESS_ROW_ADDRESS,
                            NAND_FLASH_PAGE_COUNT,
                            p_time);
    if (FSP_SUCCESS == result)
    {
        g_basic_data_programmed = true;
    }
    return result;
}

/***********************************************************************************************************************
 * @brief       This function reads normal NAND pages.
 * @param[OUT]  p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_basic_read(uint32_t * const p_time)
{
    fsp_err_t result;

    memset(g_basic_main_destination, 0, sizeof(g_basic_main_destination));
    memset(g_basic_oob_destination, 0, sizeof(g_basic_oob_destination));

    result = nandc_read_pages(NAND_FLASH_ACCESS_ROW_ADDRESS,
                             NAND_FLASH_PAGE_COUNT,
                             &g_basic_main_destination[0][0],
                             &g_basic_oob_destination[0][0],
                             p_time);
    if (FSP_SUCCESS != result)
    {
        return result;
    }

    result = nandc_verify_buffer(&g_basic_main_source[0][0],
                                &g_basic_main_destination[0][0],
                                sizeof(g_basic_main_source));
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nMain-data comparison failed\r\n");
        return result;
    }

    /*
     * Do not compare OOB against the source buffer here.
     * Normal write/read may include controller-managed ECC, CRC and Skip Byte
     * processing according to the selected FSP configuration.
     */
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function erases the block NAND.
 * @param[OUT]  p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_basic_erase(uint32_t * const p_time)
{
    fsp_err_t result;
    result = nandc_erase_block(NAND_FLASH_ACCESS_BLOCK, p_time);

    if (FSP_SUCCESS == result)
    {
        g_basic_data_programmed = false;
        g_basic_raw_data_programmed = false;
    }
    return result;
}

/***********************************************************************************************************************
 * @brief       This function erases the demo block, then writes one or more raw physical NAND pages,
 *              and returns the execution time.
 * @param[IN]   p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_basic_raw_write(uint32_t * const p_time)
{
    fsp_err_t result;
    uint32_t  erase_time = RESET_VALUE;

    result = nandc_erase_block(NAND_FLASH_ACCESS_BLOCK, &erase_time);
    if (FSP_SUCCESS != result)
    {
        return result;
    }
    g_basic_data_programmed = false;
    g_basic_raw_data_programmed = false;

    result = nandc_raw_write_pages(&g_basic_raw_source[0][0],
                                   NAND_FLASH_ACCESS_ROW_ADDRESS,
                                   NAND_FLASH_PAGE_COUNT,
                                   p_time);
    if (FSP_SUCCESS == result)
    {
        g_basic_raw_data_programmed = true;
    }
    return result;
}

/***********************************************************************************************************************
 * @brief       This function reads one or more raw physical NAND pages and returns the execution time.
 * @param[IN]   p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_basic_raw_read(uint32_t * const p_time)
{
    fsp_err_t err;
    memset(g_basic_raw_destination, 0, sizeof(g_basic_raw_destination));
    err = nandc_raw_read_pages(NAND_FLASH_ACCESS_ROW_ADDRESS,
                               NAND_FLASH_PAGE_COUNT,
                               &g_basic_raw_destination[0][0],
                               p_time);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    if (g_basic_raw_data_programmed)
    {
        err = nandc_verify_buffer(&g_basic_raw_source[0][0],
                                  &g_basic_raw_destination[0][0],
                                  sizeof(g_basic_raw_source));
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("\r\nRaw data verification failed\r\n");
            return err;
        }
        APP_PRINT("\r\nRaw data verification passed\r\n");
    }
    return nandc_wait_controller_ready();
}

/***********************************************************************************************************************
 * @brief       This function runs the NANDC logical-to-physical remap demonstration and verifies remap functionality.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_run_remap_demo(void)
{
    fsp_err_t   err;
    fsp_err_t   result;
    bool        remap_enabled = false;
    uint32_t    erase_time = RESET_VALUE;
    uint32_t    write_time = RESET_VALUE;
    uint32_t    read_time = RESET_VALUE;

    APP_PRINT("\r\nStarting NANDC remap demo...\r\n");

    /* Prepare two distinguishable data patterns */
    nandc_fill_constant_pattern(g_remap_pattern_a,
                               sizeof(g_remap_pattern_a),
                               NANDC_PATTERN_REMAP_A);

    nandc_fill_constant_pattern(g_remap_pattern_b,
                               sizeof(g_remap_pattern_b),
                               NANDC_PATTERN_REMAP_B);

    memset(g_oob_source, 0xFF, sizeof(g_oob_source));

    result = nandc_wait_controller_ready();
    if (FSP_SUCCESS != result)
    {
        nandc_print_result("NANDC remap demo", result);
        return;
    }

    /* Clear any existing remap table before accessing the physical blocks */
    err = nandc_remap_clear();
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nnandc_remap_clear failed: %d\r\n", (int32_t) err);
        nandc_print_result("NANDC remap demo", FSP_ERR_INTERNAL);
        return;
    }

    /* Erase the block containing physical Page A */
    result = nandc_erase_block(NANDC_REMAP_LOGICAL_BLOCK, &erase_time);
    if (FSP_SUCCESS != result)
    {
        nandc_print_result("NANDC remap demo", result);
        return;
    }

    /* Erase the block containing physical Page B */
    result = nandc_erase_block(NANDC_REMAP_PHYSICAL_BLOCK, &erase_time);
    if (FSP_SUCCESS != result)
    {
        nandc_print_result("NANDC remap demo", result);
        return;
    }

    /* Write Pattern A to Page A while remap is disabled */
    result = nandc_write_pages(g_remap_pattern_a,
                              &g_oob_source[0],
                              NANDC_REMAP_LOGICAL_ROW_ADDRESS,
                              1U,
                              &write_time);
    if (FSP_SUCCESS != result)
    {
        nandc_print_result("NANDC remap demo", result);
        return;
    }

    APP_PRINT("\r\nPattern A written to physical Page A\r\n");

    /* Wait for the controller to become ready before changing the remap configuration */
    result = nandc_wait_controller_ready();
    if (FSP_SUCCESS != result)
    {
        nandc_print_result("NANDC remap demo", result);
        return;
    }

    /* Configure address translation: logical Page A -> physical Page B */
    err = nandc_remap_set(NANDC_REMAP_LOGICAL_ROW_ADDRESS,
                         NANDC_REMAP_PHYSICAL_ROW_ADDRESS);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nnandc_remap_set failed: %d\r\n", (int32_t) err);
        nandc_print_result("NANDC remap demo", FSP_ERR_INTERNAL);
        return;
    }

    remap_enabled = true;

    APP_PRINT("\r\nRemap enabled:\r\n");
    APP_PRINT("  Logical row : 0x%08lX\r\n", (unsigned long) NANDC_REMAP_LOGICAL_ROW_ADDRESS);
    APP_PRINT("  Physical row: 0x%08lX\r\n", (unsigned long) NANDC_REMAP_PHYSICAL_ROW_ADDRESS);

    /*
     * Write Pattern B using logical Page A.
     * NANDC translates the logical row and writes Pattern B
     * to physical Page B.
     */
    result = nandc_write_pages(g_remap_pattern_b,
                              &g_oob_source[0],
                              NANDC_REMAP_LOGICAL_ROW_ADDRESS,
                              1U,
                              &write_time);
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }
    APP_PRINT("\r\nPattern B written through logical Page A\r\n");

    /*
     * Read logical Page A while remap is enabled.
     * NANDC redirects the read to physical Page B.
     */
    memset(g_remap_read_buffer, 0, sizeof(g_remap_read_buffer));
    memset(g_oob_destination, 0, sizeof(g_oob_destination));

    result = nandc_read_pages(NANDC_REMAP_LOGICAL_ROW_ADDRESS,
                             1U,
                             g_remap_read_buffer,
                             g_oob_destination,
                             &read_time);
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    result = nandc_verify_buffer(g_remap_pattern_b,
                                g_remap_read_buffer,
                                sizeof(g_remap_pattern_b));
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nVerification failed with remap enabled\r\n");
        APP_PRINT("Logical Page A did not return Pattern B\r\n");
        goto cleanup;
    }

    APP_PRINT("\r\nVerification 1 passed:\r\n");
    APP_PRINT("  Read logical Page A with remap enabled\r\n");
    APP_PRINT("  Pattern B was returned from physical Page B\r\n");

    /* Wait until the controller is idle before clearing remap */
    result = nandc_wait_controller_ready();
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    err = nandc_remap_clear();
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nnandc_remap_clear failed: %d\r\n", (int32_t) err);
        result = FSP_ERR_INTERNAL;
        goto cleanup;
    }

    remap_enabled = false;

    APP_PRINT("\r\nRemap disabled.\r\n");

    /*
     * Read Page A after clearing remap.
     * The row address now accesses physical Page A directly.
     * Pattern A must remain unchanged.
     */
    memset(g_remap_read_buffer, 0, sizeof(g_remap_read_buffer));
    memset(g_oob_destination, 0, sizeof(g_oob_destination));

    result = nandc_read_pages(NANDC_REMAP_LOGICAL_ROW_ADDRESS,
                             1U,
                             g_remap_read_buffer,
                             g_oob_destination,
                             &read_time);
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    result = nandc_verify_buffer(g_remap_pattern_a,
                                g_remap_read_buffer,
                                sizeof(g_remap_pattern_a));
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nVerification failed after clearing remap\r\n");
        APP_PRINT("Physical Page A did not contain Pattern A\r\n");
        goto cleanup;
    }
    APP_PRINT("\r\nVerification 2 passed:\r\n");
    APP_PRINT("  Physical Page A retained Pattern A\r\n");

    /*
     * Read physical Page B directly after clearing remap.
     * Pattern B must be stored at this address.
     */
    memset(g_remap_read_buffer, 0, sizeof(g_remap_read_buffer));
    memset(g_oob_destination, 0, sizeof(g_oob_destination));

    result = nandc_read_pages(NANDC_REMAP_PHYSICAL_ROW_ADDRESS,
                             1U,
                             g_remap_read_buffer,
                             g_oob_destination,
                             &read_time);
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    result = nandc_verify_buffer(g_remap_pattern_b,
                                g_remap_read_buffer,
                                sizeof(g_remap_pattern_b));
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nPhysical Page B verification failed\r\n");
        APP_PRINT("Physical Page B did not contain Pattern B\r\n");
        goto cleanup;
    }
    APP_PRINT("\r\nVerification 3 passed:\r\n");
    APP_PRINT("  Physical Page B contained Pattern B\r\n");


cleanup:

    /* Restore direct physical addressing if the function exits while remap is still enabled */
    if (remap_enabled)
    {
        fsp_err_t ready_result;

        ready_result = nandc_wait_controller_ready();
        if (FSP_SUCCESS != ready_result)
        {
            if (FSP_SUCCESS == result)
            {
                result = ready_result;
            }
        }
        else
        {
            err = nandc_remap_clear();
            if (FSP_SUCCESS != err)
            {
                APP_PRINT("\r\nnandc_remap_clear failed: %d\r\n", (int32_t) err);
                if (FSP_SUCCESS == result)
                {
                    result = FSP_ERR_INTERNAL;
                }
            }
            else
            {
                remap_enabled = false;
            }
        }
    }
    nandc_print_result("NANDC remap demo", result);
}

/***********************************************************************************************************************
 * @brief       This function runs the NANDC write-protection demonstration and verifies recovery after protection
                is disabled.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_run_write_protect_demo(void)
{
    fsp_err_t err;
    fsp_err_t result = FSP_SUCCESS;
    bool      protection_enabled = false;
    uint32_t  erase_time = RESET_VALUE;
    uint32_t  write_time = RESET_VALUE;
    uint32_t  read_time = RESET_VALUE;

    memset(g_oob_source, 0xFF, sizeof(g_oob_source));
    memset(g_protect_destination, 0, sizeof(g_protect_destination));
    APP_PRINT("\r\nStarting NANDC write-protection demo...\r\n");

    /* Prepare the source pattern and clear the read destination */
    nandc_fill_constant_pattern(g_protect_source,
                               sizeof(g_protect_source),
                               NANDC_PATTERN_PROTECT);

    /*
     * Erase the target block before enabling write protection.
     * The erase operation must be performed before protection is enabled.
     */
    result = nandc_erase_block(NANDC_PROTECT_BLOCK, &erase_time);
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nFailed to erase the write-protection test block\r\n");
        goto cleanup;
    }

    APP_PRINT("\r\nWrite-protection test block erased\r\n");

    /* Wait for the controller to get ready before write-protection */
    result = nandc_wait_controller_ready();
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    /*
     * Enable the selected write-protection range.
     * NANDC supports protection index 0 and index 1.
     * NANDC_PROTECT_INDEX must contain one of these values.
     */
    err = nandc_write_protect_enable(NANDC_PROTECT_ROW_ADDRESS,
                                    NANDC_PROTECT_REGION_SIZE,
                                    NANDC_PROTECT_INDEX);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nnandc_write_protect_enable failed: %d\r\n", (int32_t) err);
        result = FSP_ERR_INTERNAL;
        goto cleanup;
    }

    protection_enabled = true;

    APP_PRINT("\r\n");
    APP_PRINT("Write protection enabled:\r\n");
    APP_PRINT("  Start address: 0x%08lX\r\n", (unsigned long) NANDC_PROTECT_ROW_ADDRESS);
    APP_PRINT("  Region size  : 0x%08lX\r\n", (unsigned long) NANDC_PROTECT_REGION_SIZE);
    APP_PRINT("  Region index : %lu\r\n", (unsigned long) NANDC_PROTECT_INDEX);

    /* Write data into protected page */
    err = nandc_write_protected_pages(g_protect_source,
                                      &g_oob_source[0],
                                      NANDC_PROTECT_ROW_ADDRESS,
                                      1U);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nProtected write verification failed: %d\r\n", (int32_t) err);
        result = err;
        goto cleanup;
    }
    APP_PRINT("\r\n");
    APP_PRINT("Protection verification passed:\r\n");
    APP_PRINT("  NAND_FLASH_EVENT_WRITE_ERROR received\r\n");

    /* Wait until the controller is ready before disabling protection */
    result = nandc_wait_controller_ready();
    if (FSP_SUCCESS != result)
    {
        goto cleanup;
    }

    /* Disable the same protection range */
    err = nandc_write_protect_disable(NANDC_PROTECT_INDEX);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("\r\nnandc_write_protect_disable failed: %d\r\n", (int32_t) err);
        result = FSP_ERR_INTERNAL;
        goto cleanup;
    }

    protection_enabled = false;
    APP_PRINT("\r\nWrite protection disabled\r\n");

    /*
     * Write the same page again after disabling protection.
     * The previous protected write was rejected, so the target page remains
     * erased and can be programmed without another erase operation.
     */
    result = nandc_write_pages(g_protect_source,
                              &g_oob_source[0],
                              NANDC_PROTECT_ROW_ADDRESS,
                              1U,
                              &write_time);
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nWrite failed after write protection was disabled\r\n");
        goto cleanup;
    }

    APP_PRINT("\r\nWrite succeeded after write protection was disabled\r\n");

    /* Clear the destination buffer before read-back */
    memset(g_protect_destination, 0, sizeof(g_protect_destination));
    memset(g_oob_destination, 0, sizeof(g_oob_destination));

    /* Read back the programmed page */
    result = nandc_read_pages(NANDC_PROTECT_ROW_ADDRESS,
                             1U,
                             g_protect_destination,
                             g_oob_destination,
                             &read_time);
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nRead-back failed after write protection was disabled\r\n");
        goto cleanup;
    }

    /* Verify end-to-end programming after protection was disabled */
    result = nandc_verify_buffer(g_protect_source,
                                g_protect_destination,
                                sizeof(g_protect_source));
    if (FSP_SUCCESS != result)
    {
        APP_PRINT("\r\nRead-back comparison failed after protection was disabled\r\n");
        goto cleanup;
    }
    APP_PRINT("\r\nRecovery verification passed:\r\n");
    APP_PRINT("  Write completed after protection was disabled\r\n");
    APP_PRINT("  Read-back data matched the source data\r\n");

cleanup:

    /*
     * Restore the NANDC protection configuration if the function exits after
     * protection was enabled but before the normal disable step completed.
     */
    if (protection_enabled)
    {
        fsp_err_t ready_result;

        /* Wait for the controller to get ready before disabling write-protection */
        ready_result = nandc_wait_controller_ready();
        if (FSP_SUCCESS != ready_result)
        {
            APP_PRINT("\r\nController did not become ready during protection cleanup\r\n");
            if (FSP_SUCCESS == result)
            {
                result = ready_result;
            }
        }
        else
        {
            err = nandc_write_protect_disable(NANDC_PROTECT_INDEX);
            if (FSP_SUCCESS != err)
            {
                APP_PRINT("\r\nCleanup nandc_write_protect_disable failed: %d\r\n", (int32_t) err);
                if (FSP_SUCCESS == result)
                {
                    result = FSP_ERR_INTERNAL;
                }
            }
            else
            {
                protection_enabled = false;

                APP_PRINT("\r\nWrite protection disabled during cleanup\r\n");
            }
        }
    }

    nandc_print_result("NANDC write-protection demo", result);
}

/***********************************************************************************************************************
 * @brief       This function prints summary of the raw NAND page buffer returned by RawRead.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_print_raw_page_summary(void)
{
    APP_PRINT("\r\nRaw Read Summary\r\n");
    APP_PRINT("------------------------------\r\n");

    APP_PRINT("Row Address   : %u\r\n", (uint32_t) NAND_FLASH_ACCESS_ROW_ADDRESS);
    APP_PRINT("Page Count    : %u\r\n", (uint32_t) NAND_FLASH_PAGE_COUNT);
    APP_PRINT("Bytes/Page    : %u\r\n", (uint32_t) NAND_FLASH_PHYSICAL_PAGE_BYTES);
    APP_PRINT("Total Bytes   : %u\r\n", (uint32_t) sizeof(g_basic_raw_destination));

    APP_PRINT("\r\n");
    APP_PRINT("g_basic_raw_destination[page][bytes]\r\n");
    APP_PRINT("\r\n");
    APP_PRINT("Main Data Start  : [page][0]\r\n");
    APP_PRINT("Spare Data Start : [page][%u]\r\n", (uint32_t) NAND_FLASH_PHYSICAL_DATA_BYTES);
    APP_PRINT("\r\n");
}

/***********************************************************************************************************************
 * @brief       This function prints the operation result.
 * @param[IN]   p_operation     Pointer to operation description string.
 * @param[IN]   result          Operation result code.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_print_result(char const * p_operation, fsp_err_t result)
{
    if (FSP_SUCCESS == result)
    {
        APP_PRINT("\r\n%s completed successfully\r\n", p_operation);
    }
    else
    {
        APP_PRINT("\r\n%s failed: %s\r\n", p_operation, nandc_result_string(result));
    }
}

/***********************************************************************************************************************
 * @brief       This function converts an FSP error code to a printable string.
 * @param[IN]   result      FSP error code.
 * @retval      Pointer     to constant result string.
 **********************************************************************************************************************/
static char const * nandc_result_string(fsp_err_t result)
{
    switch (result)
    {
        case FSP_SUCCESS:              return "success";
        case FSP_ERR_TIMEOUT:          return "timeout";
        case FSP_ERR_INVALID_ARGUMENT: return "invalid argument";
        case FSP_ERR_INVALID_STATE:    return "invalid state or verification failed";
        default:                       return "driver/API error";
    }
}


/***********************************************************************************************************************
 * @brief       It receives a callback for the sci_uart instruction and processes it according to the events in the
 *              received callback.
 * @param[in]   *p_args      Pointer to argument information.
 * @retval      None.
 **********************************************************************************************************************/
void uart0_cb_stdio(uart_callback_args_t *p_args)
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
                if ((UART_DATA_BITS_7 == g_uart0_cfg.data_bits) || (UART_DATA_BITS_8 == g_uart0_cfg.data_bits))
                {
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
                else
                {
                    uint16_t * p_dest = (uint16_t *) &g_out_of_band_received[g_out_of_band_index];
                    *p_dest              = (uint16_t) p_args->data;
                    g_out_of_band_index += 2;
                }
            }
            break;
        }

        /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1;
            break;
        }

        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1;
            break;
        }
        default:
        {
            break;
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_nandc_ep)
 **********************************************************************************************************************/
