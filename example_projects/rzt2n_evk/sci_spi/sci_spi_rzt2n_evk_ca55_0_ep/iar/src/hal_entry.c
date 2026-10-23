/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point and SCI SPI example processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TRANSFER_SIZE      (64U)
#define SLAVE_SELECT_LINE  (BSP_IO_PORT_05_PIN_6)

/***********************************************************************************************************************
 * Function declarations
 **********************************************************************************************************************/
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

void hal_entry(void);
int stdio_open(void);
void handle_error(fsp_err_t error);
void spi_master_callback(spi_callback_args_t * p_args);
void spi_slave_callback(spi_callback_args_t * p_args);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static volatile bool g_spi_master_transfer_complete = false;
static volatile bool g_spi_slave_transfer_complete = false;

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
uint8_t spi_master_tx_buffer[TRANSFER_SIZE];
uint8_t spi_master_rx_buffer[TRANSFER_SIZE];
uint8_t spi_slave_tx_buffer[TRANSFER_SIZE];
uint8_t spi_slave_rx_buffer[TRANSFER_SIZE];

/*******************************************************************************************************************//**
 * @brief Main application entry point.
 *
 * This example demonstrates SCI SPI communication between a slave instance on SCI1 and a master instance on SCI2.
 * The example executes three transfer patterns and verifies the received data after each transfer. SCI0 is reserved
 * for the UART console output.
 *
 * @retval None
 **********************************************************************************************************************/
void hal_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Enable CPU interrupts before initializing the interrupt-driven UART and SCI SPI modules. */
    __enable_irq();

    /*
     * Initialize the UART console on SCI0.
     * SCI1 is reserved for the SCI SPI slave and SCI2 is reserved for the SCI SPI master.
     */
    int console_err = stdio_open();
    if ((int) FSP_SUCCESS != console_err)
    {
        while (1)
        {
            __NOP();
        }
    }

    /* Display the example project banner and overview. */
    APP_PRINT(BANNER_INFO,
              EP_VERSION,
              4, 2, 0);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates SCI SPI master/slave transfers and received-data verification.\r\n");
    APP_PRINT("UART console: SCI0. SPI slave: SCI1. SPI master: SCI2.\r\n");

    /* Initialize transmit buffers and clear receive buffers for the first transfer. */
    for (uint8_t i = 0U; i < TRANSFER_SIZE; i++)
    {
        spi_master_tx_buffer[i] = i;
        spi_master_rx_buffer[i] = 0U;
        spi_slave_tx_buffer[i] = i;
        spi_slave_rx_buffer[i] = 0U;
    }

    /* Set the Slave Select line to the inactive state before opening the SCI SPI modules. */
    R_BSP_PinSet(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Open SCI1 as the SCI SPI slave. */
    err = R_SCI_SPI_Open(&g_sci_spi0_ctrl, &g_sci_spi0_cfg);
    handle_error(err);

    /* Open SCI2 as the SCI SPI master. */
    err = R_SCI_SPI_Open(&g_sci_spi1_ctrl, &g_sci_spi1_cfg);
    handle_error(err);

    /*******************************************************************************************************************
     * Phase 1: Full-duplex transfer
     ******************************************************************************************************************/
    APP_PRINT("\r\n[SCI SPI] Phase 1: full-duplex transfer.\r\n");

    /* Start the slave write/read transfer before asserting Slave Select. */
    err = R_SCI_SPI_WriteRead(&g_sci_spi0_ctrl,
                              spi_slave_tx_buffer,
                              spi_slave_rx_buffer,
                              TRANSFER_SIZE,
                              SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Assert the Slave Select line. */
    R_BSP_PinClear(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Start the master write/read transfer. */
    err = R_SCI_SPI_WriteRead(&g_sci_spi1_ctrl,
                              spi_master_tx_buffer,
                              spi_master_rx_buffer,
                              TRANSFER_SIZE,
                              SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Wait for the master transfer-complete callback event. */
    while (false == g_spi_master_transfer_complete)
    {
        __NOP();
    }

    /* De-assert the Slave Select line after the master transfer completes. */
    R_BSP_PinSet(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Wait for the slave transfer-complete callback event. */
    while (false == g_spi_slave_transfer_complete)
    {
        __NOP();
    }

    /* Verify data received by the slave. */
    if (0 != memcmp(spi_slave_tx_buffer, spi_slave_rx_buffer, TRANSFER_SIZE))
    {
        APP_PRINT("[ERROR] Phase 1 slave-buffer verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    /* Verify data received by the master. */
    if (0 != memcmp(spi_master_tx_buffer, spi_master_rx_buffer, TRANSFER_SIZE))
    {
        APP_PRINT("[ERROR] Phase 1 master-buffer verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT("[PASS] Phase 1 completed.\r\n");

    R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MILLISECONDS);

    /* Reset transfer-complete flags before starting the next phase. */
    g_spi_master_transfer_complete = false;
    g_spi_slave_transfer_complete = false;

    /* Prepare the master transmit buffer for the master-to-slave transfer. */
    for (uint8_t i = 0U; i < TRANSFER_SIZE; i++)
    {
        spi_master_tx_buffer[i] = (uint8_t) ((TRANSFER_SIZE - i) & 0xFFU);
        spi_master_rx_buffer[i] = 0U;
    }

    /*******************************************************************************************************************
     * Phase 2: Master write / slave read
     ******************************************************************************************************************/
    APP_PRINT("[SCI SPI] Phase 2: master write / slave read.\r\n");

    /* Start the slave receive operation before asserting Slave Select. */
    err = R_SCI_SPI_Read(&g_sci_spi0_ctrl,
                         spi_slave_rx_buffer,
                         TRANSFER_SIZE,
                         SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Assert the Slave Select line. */
    R_BSP_PinClear(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Start the master transmit operation. */
    err = R_SCI_SPI_Write(&g_sci_spi1_ctrl,
                          spi_master_tx_buffer,
                          TRANSFER_SIZE,
                          SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Wait for the master transfer-complete callback event. */
    while (false == g_spi_master_transfer_complete)
    {
        __NOP();
    }

    /* De-assert the Slave Select line after the master transfer completes. */
    R_BSP_PinSet(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Wait for the slave transfer-complete callback event. */
    while (false == g_spi_slave_transfer_complete)
    {
        __NOP();
    }

    /* Verify the master-to-slave transfer data. */
    if (0 != memcmp(spi_master_tx_buffer, spi_slave_rx_buffer, TRANSFER_SIZE))
    {
        APP_PRINT("[ERROR] Phase 2 master-to-slave verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT("[PASS] Phase 2 completed.\r\n");

    R_BSP_SoftwareDelay(10U, BSP_DELAY_UNITS_MILLISECONDS);

    /* Reset transfer-complete flags before starting the next phase. */
    g_spi_master_transfer_complete = false;
    g_spi_slave_transfer_complete = false;

    /* Use the data received by the slave as the transmit data for the final transfer. */
    for (uint32_t i = 0U; i < TRANSFER_SIZE; i++)
    {
        spi_slave_tx_buffer[i] = spi_slave_rx_buffer[i];
    }

    /*******************************************************************************************************************
     * Phase 3: Slave write / master read
     ******************************************************************************************************************/
    APP_PRINT("[SCI SPI] Phase 3: slave write / master read.\r\n");

    /* Start the slave transmit operation before asserting Slave Select. */
    err = R_SCI_SPI_Write(&g_sci_spi0_ctrl,
                          spi_slave_tx_buffer,
                          TRANSFER_SIZE,
                          SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Assert the Slave Select line. */
    R_BSP_PinClear(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Start the master receive operation. */
    err = R_SCI_SPI_Read(&g_sci_spi1_ctrl,
                         spi_master_rx_buffer,
                         TRANSFER_SIZE,
                         SPI_BIT_WIDTH_8_BITS);
    handle_error(err);

    /* Wait for the master transfer-complete callback event. */
    while (false == g_spi_master_transfer_complete)
    {
        __NOP();
    }

    /* De-assert the Slave Select line after the master transfer completes. */
    R_BSP_PinSet(R_BSP_IoRegionGet(SLAVE_SELECT_LINE), SLAVE_SELECT_LINE);

    /* Wait for the slave transfer-complete callback event. */
    while (false == g_spi_slave_transfer_complete)
    {
        __NOP();
    }

    /* Verify the slave-to-master transfer data. */
    if (0 != memcmp(spi_slave_tx_buffer, spi_master_rx_buffer, TRANSFER_SIZE))
    {
        APP_PRINT("[ERROR] Phase 3 slave-to-master verification failed.\r\n");

        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }

    APP_PRINT("[PASS] Phase 3 completed.\r\n");
    APP_PRINT("\r\nSCI SPI example completed successfully.\r\n");

    /* Example completed successfully. Remain here for debugger inspection. */
    while (1)
    {
        __NOP();
    }
}

/*******************************************************************************************************************//**
 * @brief Handles errors returned by FSP APIs.
 *
 * @param[in] error FSP error code.
 *
 * @retval None
 **********************************************************************************************************************/
void handle_error(fsp_err_t error)
{
    if (FSP_SUCCESS != error)
    {
        APP_PRINT("\r\n[ERROR] FSP API returned: 0x%x\r\n", (unsigned int) error);

        /* Halt execution so the error state can be inspected with the debugger. */
        while (1)
        {
            __NOP();
            __asm volatile ("" ::: "memory");
        }
    }
}

/*******************************************************************************************************************//**
 * @brief SCI SPI master callback.
 *
 * Sets the master transfer-complete flag when the SCI SPI driver reports that the transfer has completed.
 *
 * @param[in] p_args Pointer to the SCI SPI callback arguments.
 *
 * @retval None
 **********************************************************************************************************************/
void spi_master_callback(spi_callback_args_t * p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_spi_master_transfer_complete = true;
    }
}

/*******************************************************************************************************************//**
 * @brief SCI SPI slave callback.
 *
 * Sets the slave transfer-complete flag when the SCI SPI driver reports that the transfer has completed.
 *
 * @param[in] p_args Pointer to the SCI SPI callback arguments.
 *
 * @retval None
 **********************************************************************************************************************/
void spi_slave_callback(spi_callback_args_t * p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_spi_slave_transfer_complete = true;
    }
}

/*******************************************************************************************************************//**
 * @brief Performs BSP initialization during the warm-start sequence.
 *
 * Initializes DDR when enabled and applies the generated pin configuration after C runtime initialization.
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
 * This function runs at the beginning of startup and must contain only inline assembly instructions.
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
