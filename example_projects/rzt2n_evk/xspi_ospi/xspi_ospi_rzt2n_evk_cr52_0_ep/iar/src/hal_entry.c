/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point and XSPI OSPI example processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

void hal_entry(void);
void ospi_set_DTR_OPI_Mode_enable(void);
void handle_error(fsp_err_t err);
int stdio_open(void);

#define DATA_SIZE                  (8U)
#define SECTOR_ERASE_SIZE          (4096U)
#define OSPI_DEVICE_START_ADDRESS  (0x40000000U)
#define WRITE_START_ADDRESS        (0x1000U)
#define P_OSPI_EXTEND              ((xspi_ospi_extended_cfg_t *) g_ospi0.p_cfg->p_extend)

const uint8_t g_src[DATA_SIZE] =
{
    0x33U, 0x33U, 0x33U, 0x33U, 0xBBU, 0xFEU, 0xEEU, 0xF1U
};

/*******************************************************************************************************************//**
 * @brief Main application entry point.
 *
 * Opens the UART console and OSPI instance, enables DTR-OPI mode, erases a sector, performs direct-transfer access,
 * verifies memory-mapped access, and verifies an eight-byte write through the OSPI write API.
 *
 * @retval None
 **********************************************************************************************************************/
void hal_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;
    int       console_err;

    __enable_irq();

    console_err = stdio_open();
    if ((int) FSP_SUCCESS != console_err)
    {
        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT(BANNER_INFO,
              EP_VERSION,
              4, 2, 0);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates XSPI OSPI flash access in DTR-OPI mode.\r\n");
    APP_PRINT("It performs sector erase, direct write/read, memory-mapped read, and write API verification.\r\n");

    /* Open the OSPI instance. */
    err = R_XSPI_OSPI_Open(&g_ospi0_ctrl, &g_ospi0_cfg);
    handle_error(err);

    /* Enable DTR-OPI mode in Configuration Register 2. */
    ospi_set_DTR_OPI_Mode_enable();

    /* Erase the configured sector. */
    err = R_XSPI_OSPI_Erase(&g_ospi0_ctrl,
                            (uint8_t *) (OSPI_DEVICE_START_ADDRESS + WRITE_START_ADDRESS),
                            SECTOR_ERASE_SIZE);
    handle_error(err);

    /* Wait for the erase operation to complete. */
    spi_flash_status_t status_erase;
    do
    {
        (void) R_XSPI_OSPI_StatusGet(&g_ospi0_ctrl, &status_erase);
    } while (true == status_erase.write_in_progress);

    APP_PRINT("[PASS] OSPI sector erase completed.\r\n");

    /* Direct transfer: Write Enable. */
    spi_flash_direct_transfer_t ospi_test_direct_transfer_write_enable = {0};
    ospi_test_direct_transfer_write_enable.command        = 0x06F9U;
    ospi_test_direct_transfer_write_enable.command_length = 2U;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &ospi_test_direct_transfer_write_enable,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Direct transfer: Write. */
    spi_flash_direct_transfer_t ospi_test_direct_transfer_write =
    {
        .command        = 0x12EDU,
        .address        = 0x00001114U,
        .data           = 0x1114U,
        .command_length = 2U,
        .address_length = 4U,
        .data_length    = 2U,
        .dummy_cycles   = 0U
    };

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &ospi_test_direct_transfer_write,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Wait for the direct-write operation to complete. */
    spi_flash_status_t status_write;
    do
    {
        (void) R_XSPI_OSPI_StatusGet(&g_ospi0_ctrl, &status_write);
    } while (true == status_write.write_in_progress);

    /* Direct transfer: Read. */
    spi_flash_direct_transfer_t ospi_test_direct_transfer_read =
    {
        .command        = 0xEE11U,
        .address        = 0x00001114U,
        .data           = 0U,
        .command_length = 2U,
        .address_length = 4U,
        .data_length    = 2U,
        .dummy_cycles   = P_OSPI_EXTEND->opi_mem_read_dummy_cycles
    };

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &ospi_test_direct_transfer_read,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);

    /* Verify the direct-transfer write/read data. */
    if (ospi_test_direct_transfer_read.data != ospi_test_direct_transfer_write.data)
    {
        APP_PRINT("[ERROR] OSPI direct-transfer verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT("[PASS] OSPI direct-transfer write/read verification passed.\r\n");

    /* Verify the same value through the memory-mapped OSPI window. */
    uint32_t read_buffer =
        *(uint16_t *) (OSPI_DEVICE_START_ADDRESS + ospi_test_direct_transfer_write.address);

    if (read_buffer != ospi_test_direct_transfer_write.data)
    {
        APP_PRINT("[ERROR] OSPI memory-mapped read verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT("[PASS] OSPI memory-mapped read verification passed.\r\n");

    /* Write the original eight-byte test pattern through the OSPI write API. */
    uint8_t * p_dest = (uint8_t *) (OSPI_DEVICE_START_ADDRESS + WRITE_START_ADDRESS);

    err = R_XSPI_OSPI_Write(&g_ospi0_ctrl, g_src, p_dest, DATA_SIZE);
    handle_error(err);

    /* Wait for the API write operation to complete. */
    spi_flash_status_t status_write2;
    do
    {
        R_BSP_SoftwareDelay(50U, BSP_DELAY_UNITS_MICROSECONDS);
        (void) R_XSPI_OSPI_StatusGet(&g_ospi0_ctrl, &status_write2);
    } while (true == status_write2.write_in_progress);

    /* Verify the original eight-byte source pattern through memory-mapped reads. */
    for (uint8_t i = 0U; i < (DATA_SIZE / 2U); i++)
    {
        uint16_t dest_data;
        uint16_t src_data;

        dest_data = *((uint16_t *) (OSPI_DEVICE_START_ADDRESS + WRITE_START_ADDRESS) + i);
        memcpy(&src_data, &g_src[(uint32_t) i * sizeof(uint16_t)], sizeof(src_data));

        if (src_data != dest_data)
        {
            APP_PRINT("[ERROR] OSPI write API verification failed at word %u.\r\n", (unsigned int) i);

            while (1)
            {
                __NOP();
                __asm volatile ("" ::: "memory");
            }
        }
    }

    APP_PRINT("[PASS] OSPI write API verification passed for %u bytes.\r\n", (unsigned int) DATA_SIZE);
    APP_PRINT("[PASS] XSPI OSPI example completed successfully.\r\n");

    while (1)
    {
        __NOP();
    }
}

/*******************************************************************************************************************//**
 * @brief Enables DTR-OPI mode and configures the external OSPI flash dummy-cycle setting.
 *
 * @retval None
 **********************************************************************************************************************/
void ospi_set_DTR_OPI_Mode_enable(void)
{
    fsp_err_t err;
    uint32_t  write_data = 0x02U;

    /* Temporarily use protocol 1S-1S-1S. */
    (void) R_XSPI_OSPI_SpiProtocolSet(&g_ospi0_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);

    /* Write Enable (06h). */
    spi_flash_direct_transfer_t ospi_test_direct_transfer_WE2 = {0};
    ospi_test_direct_transfer_WE2.command        = 0x06U;
    ospi_test_direct_transfer_WE2.command_length = 1U;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &ospi_test_direct_transfer_WE2,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Write Configuration Register 2 (72h): Enable DTR-OPI. */
    spi_flash_direct_transfer_t direct_command_WRCR2_2 = {0};
    direct_command_WRCR2_2.command        = 0x72U;
    direct_command_WRCR2_2.address        = 0x00000000U;
    direct_command_WRCR2_2.command_length = 1U;
    direct_command_WRCR2_2.address_length = 4U;
    direct_command_WRCR2_2.data_length    = 1U;
    direct_command_WRCR2_2.data           = write_data;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &direct_command_WRCR2_2,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Change protocol from 1S-1S-1S to 8D-8D-8D. */
    (void) R_XSPI_OSPI_SpiProtocolSet(&g_ospi0_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);

    /* Write Enable in DTR-OPI mode. */
    spi_flash_direct_transfer_t ospi_test_direct_transfer_WE1 = {0};
    ospi_test_direct_transfer_WE1.command        = 0x06F9U;
    ospi_test_direct_transfer_WE1.command_length = 2U;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &ospi_test_direct_transfer_WE1,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Set DC[2:0] according to the configured number of dummy cycles. */
    switch (P_OSPI_EXTEND->opi_mem_read_dummy_cycles)
    {
        case 20:
            write_data = 0x0U;
            break;

        case 18:
            write_data = 0x1U;
            break;

        case 16:
            write_data = 0x2U;
            break;

        case 14:
            write_data = 0x3U;
            break;

        case 12:
            write_data = 0x4U;
            break;

        case 10:
            write_data = 0x5U;
            break;

        case 8:
            write_data = 0x6U;
            break;

        case 6:
            write_data = 0x7U;
            break;

        default:
            write_data = 0x0U;
            break;
    }

    /* Write Configuration Register 2 dummy-cycle setting. */
    spi_flash_direct_transfer_t direct_command_WRCR2_1 = {0};
    direct_command_WRCR2_1.command        = 0x728DU;
    direct_command_WRCR2_1.address        = 0x00000300U;
    direct_command_WRCR2_1.command_length = 2U;
    direct_command_WRCR2_1.address_length = 4U;
    direct_command_WRCR2_1.data_length    = 1U;
    direct_command_WRCR2_1.data           = write_data;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &direct_command_WRCR2_1,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);

    /* Read Configuration Register 2 (71h). */
    spi_flash_direct_transfer_t direct_command_RDCR2_2 = {0};
    direct_command_RDCR2_2.command        = 0x718EU;
    direct_command_RDCR2_2.address        = 0x00000300U;
    direct_command_RDCR2_2.command_length = 2U;
    direct_command_RDCR2_2.address_length = 4U;
    direct_command_RDCR2_2.data_length    = 1U;
    direct_command_RDCR2_2.dummy_cycles   = 4U;

    err = R_XSPI_OSPI_DirectTransfer(&g_ospi0_ctrl,
                                     &direct_command_RDCR2_2,
                                     SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
}

/*******************************************************************************************************************//**
 * @brief Reports an FSP API error without changing the original application flow.
 *
 * @param[in] err FSP error code.
 *
 * @retval None
 **********************************************************************************************************************/
void handle_error(fsp_err_t err)
{
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("[ERROR] FSP API returned error code %d.\r\n", (int) err);
    }
}

/*******************************************************************************************************************//**
 * @brief Performs BSP initialization during the warm-start sequence.
 *
 * @param[in] event Current BSP warm-start event.
 *
 * @retval None
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
        /* Reserved for pre-clock initialization. */
    }

    if (BSP_WARM_START_POST_C == event)
    {
#if BSP_FEATURE_DDR_SUPPORTED
 #if (1 == BSP_CFG_DDR_INIT_ENABLE)

        /* Initialize DDR when enabled in the BSP configuration. */
        bsp_ddr_init();

 #endif
#endif

        if (NULL != g_bsp_pin_cfg.p_extend)
        {
            /* Apply the generated pin configuration. */
            R_IOPORT_Open(&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);
        }
    }
}

/*******************************************************************************************************************//**
 * @brief Executes the stackless portion of the BSP warm-start sequence.
 *
 * @retval None
 **********************************************************************************************************************/
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
{
#if (0 == BSP_LP64_SUPPORT)
    __asm volatile ("BX lr");
#else
    __asm volatile ("BR lr");
#endif
}
