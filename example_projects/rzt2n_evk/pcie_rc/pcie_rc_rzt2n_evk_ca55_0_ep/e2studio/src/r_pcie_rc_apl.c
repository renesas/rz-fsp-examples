/***********************************************************************************************************************
 * File Name    : r_pcie_rc_apl.c
 * Description  : PCIe Gen2 Root Complex example application functions.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 Includes
 ******************************************************************************/
#include "hal_data.h"
#include "sio_char.h"
#include "r_pcie_rc_dmac.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "common_utils.h"

/******************************************************************************
 Macro definitions
 ******************************************************************************/
/* PCIe DMAC channel number (0 - 7) */
#define PCIE_DMAC_CH                     (0)

/* Enable/disable each PCIe test */
#define PCI_CONFIG_READ_WRITE_TEST       (1)  /* Test PCIe Functions 1 and 2 */
#define PCI_MEMORY_READ_WRITE_TEST       (1)  /* Test PCIe Functions 3 */
#define PCI_DMAC_READ_WRITE_TEST         (1)  /* Test PCIe Functions 4 */
#define PCI_MEMORY_READ_WRITE_SPEED_TEST (1)  /* Test PCIe Functions 5 */
#define PCI_DMAC_READ_WRITE_SPEED_TEST   (1)  /* Test PCIe Functions 6 */

/* CPU's mirror base address for accessing the PCIe window configured by
 * the RC (e.g., 0xA000_0000 maps to PCIe address 0x4_0000_0000). */
#define PCIE_MIRROR_BASE_ADDR            (BSP_CFG_EL3_MMU_REGION35_VADDR)
#define PCIE_WINDOW_SIZE                 (0x1000U)

/* Offset definitions inside the PCIe window */
#define PCIE_NOTIFY_FLAG_OFFSET          (0x00U)
#define PCIE_TEST_DATA_OFFSET            (0x04U)

/* This address in the RC's PCIe mirror window is used to write test data to the EP's AXI window,
 * starting immediately after the "RC -> EP notification flag". */
#define PCIE_TEST_DATA_ADDR              (PCIE_MIRROR_BASE_ADDR + PCIE_TEST_DATA_OFFSET)   /*0xA0000004*/
#define PCIE_TEST_DATA_SIZE              (PCIE_WINDOW_SIZE - PCIE_TEST_DATA_OFFSET)

/* Local buffer address used for PCIe DMAC test */
#define PCI_DMAC_WRITE_ADDR              (0x10110000U)

/* RC -> EP notification flag
 * RC writes this value to notify EP that RC-side memory write has completed. */
#define RC_TO_EP_NOTIFY_ADDR             (PCIE_MIRROR_BASE_ADDR + PCIE_NOTIFY_FLAG_OFFSET) /*0xA0000000*/
#define RC_TO_EP_NOTIFY_VALUE            (0x52434550U)  /* "RCEP" */

/* EP -> RC notification flag
 * EP writes this value to notify RC that EP-side writeback has completed. */
#define EP_TO_RC_NOTIFY_ADDR             (0x10190000U)
#define EP_TO_RC_NOTIFY_VALUE            (0x45505243U)  /* "EPRC" */

/* Address in RC's AXI window where data written back from EP is stored,
 * starting immediately after the "EP -> RC notification flag". */
#define EP_TO_RC_WRITEBACK_ADDR          (EP_TO_RC_NOTIFY_ADDR + PCIE_TEST_DATA_OFFSET) /*0x10190004*/

/* Notification control */
#define PCIE_NOTIFY_CLEAR_VALUE          (0x00000000U)
#define PCIE_RC_EP_NOTIFY_TIMEOUT_MS     (1000U)

/******************************************************************************
 Exported global functions
 ******************************************************************************/
void handle_error (fsp_err_t err);
void user_uart_callback (uart_callback_args_t * p_args);
void PCIe_rc_function_test(void);
void pcie_config_read_test(void);
void pcie_config_write_test(void);
void pcie_memory_read_write_test(void);
void pcie_dmac_read_write_test(void);
void pcie_memory_read_write_speed_test(void);
void pcie_memory_write_speed_test(uint32_t write_buf);
void pcie_memory_read_speed_test(uint32_t read_buf);
void pcie_dmac_read_write_speed_test(void);
void pcie_dmac_write_speed_test();
void pcie_dmac_read_speed_test();

void pcie_rc_notify_ep(void);
bool pcie_rc_check_ep_notify(void);
void pcie_rc_clear_ep_notify(void);
static bool pcie_rc_wait_ep_notify(void);
static bool pcie_rc_verify_ep_writeback_data(void);

/******************************************************************************
 Global variables
 ******************************************************************************/
pci_configuration_register_init_t const g_pci_rc0_configuration_register_init_list
[PCIE_RC_CH0_CONFIG_REG_INIT_LIST_NUM];
uint8_t  g_out_of_band_received[TRANSFER_LENGTH];
uint32_t g_out_of_band_index = 0;
volatile uint32_t g_transfer_complete = 0;
volatile uint32_t g_receive_complete  = 0;
char g_buff[16];
volatile double g_us_counter_time = 0;
volatile double g_momory_RW_time = 0;
timer_status_t g_timer_status;

pci_configuration_register_transfer_t g_pcie_rc0_cfg_reg_read =
{
    .header_word_b.ext_register_number  = 0U,
    .header_word_b.function_number      = 0U,
    .header_word_b.bus_number           = 0U,
};

pci_configuration_register_transfer_t g_pcie_rc0_cfg_reg_write =
{
    .header_word_b.ext_register_number  = 0U,
    .header_word_b.function_number      = 0U,
    .header_word_b.bus_number           = 0U,
};

timer_status_t g_timer0_status;

/*******************************************************************************************************************//**
 * @brief       PCIe example application (RC device). Initializes it as an RC device, configures the PCIe Windows,
 *              and tests the following 6 PCIe functions after linking up the RC device and EP device:
 *              Function 1. Read EP device configuration register from RC device
 *              Function 2. Write EP device configuration register from RC device
 *              Function 3. Read/write EP device memory from RC device
 *              Function 4. Read and write EP device memory from RC device using DMAC
 *              Function 5. Measure memory read and write speed from RC device to EP device
 *              Function 6. Measure memory read and write speed from RC device to EP device using DMAC
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void PCIe_rc_function_test(void)
{
    pci_status_t g_pci_rc0_status;
    volatile uintptr_t test_data_addr = 0;
    volatile uint32_t *regsave_ptr;
    timer_info_t p_timer_info;

    /**** Open the transfer instance with initial configuration. */
    fsp_err_t err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_error(err);

    /* CMTW init */
    err = R_CMTW_Open(&g_timer0_ctrl, &g_timer0_cfg);
    handle_error(err);

    /* CMTW 1 count time[us] */
    R_CMTW_InfoGet(&g_timer0_ctrl, &p_timer_info);
    /* Calculate the duration of one CMTW count in microseconds. */
    g_us_counter_time = 1000000 / (double)p_timer_info.clock_frequency;

    __enable_irq();

    APP_PRINT(BANNER_INFO,EP_VERSION,4, 2, 0);

    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates PCIe Gen2 Root Complex operation.\r\n");

    APP_PRINT("\r\n");
    /**** pcie test start */
    APP_PRINT("---- PCIe OPERATION TEST START ~ RC Devise Side ~ ----\n");

    /**** pcie rc init ch */
    APP_PRINT("   PCIe %dch Initialization Root Complex Mode\n", g_pci_rc0_cfg.channel);
    R_PCIE_RC_Open(&g_pci_rc0_ctrl, &g_pci_rc0_cfg);

    /**** Wait for 5 ms or more */
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MILLISECONDS);

    /**** Link up check */
    APP_PRINT("   Check Link-up with EP Device ...\n");
    do
    {
        /* Poll the PCIe RC link status. */
        R_PCIE_RC_LinkStatusGet(&g_pci_rc0_ctrl, &g_pci_rc0_status);
    } while ((true) != (g_pci_rc0_status.is_link_up));
    APP_PRINT("   Link up with EP Device!\n");

#if PCI_CONFIG_READ_WRITE_TEST
    /**** Function 1. config read test */
    pcie_config_read_test();

    /**** Function 2. config write test */
    pcie_config_write_test();
#endif

#if PCI_MEMORY_READ_WRITE_TEST
    /**** Function 3. memory read write test */
    pcie_memory_read_write_test();

    /* Notify EP that RC memory write test has completed. */
    pcie_rc_notify_ep();

    /* Wait for EP writeback completion notification. */
    if (!(pcie_rc_wait_ep_notify()))
    {
        printf("      Timeout: EP -> RC notification was not received.\n");
    }
    else
    {
        /* Clear RC -> EP notification flag and verify EP writeback data. */
        pcie_rc_clear_ep_notify();
        pcie_rc_verify_ep_writeback_data();
    }
#endif

#if PCI_DMAC_READ_WRITE_TEST || PCI_DMAC_READ_WRITE_SPEED_TEST
    /* set test data */
    for (test_data_addr = PCI_DMAC_WRITE_ADDR;
         (test_data_addr) < (PCI_DMAC_WRITE_ADDR + PCIE_TEST_DATA_SIZE);
         test_data_addr += 4U)
    {
        /* Cast the test data address to a pointer for direct register access. */
        regsave_ptr = (uint32_t *)test_data_addr;
        /* Write a known test pattern to the address. */
        *regsave_ptr = 0x12345678;
    }
#endif

#if PCI_DMAC_READ_WRITE_TEST
    /**** Function 4. memory read write test using DMAC */
    pcie_dmac_read_write_test();
#endif

#if PCI_MEMORY_READ_WRITE_SPEED_TEST
    /**** Function 5. memory read write speed test */
    pcie_memory_read_write_speed_test();

    /* Notify EP that RC memory write test has completed. */
    pcie_rc_notify_ep();

    /* Wait for EP writeback completion notification. */
    if (!(pcie_rc_wait_ep_notify()))
    {
        printf("      Timeout: EP -> RC notification was not received.\n");
    }
    else
    {
        /* Clear RC -> EP notification flag and verify EP writeback data. */
        pcie_rc_clear_ep_notify();
        pcie_rc_verify_ep_writeback_data();
    }
#endif

#if PCI_DMAC_READ_WRITE_SPEED_TEST
    /**** Function 6. memory read write speed test using DMAC */
    pcie_dmac_read_write_speed_test();
#endif

    printf ("-------------- PCIe OPERATION TEST END ---------------\n");
    while (1)
    {
        /* Wait here */
    }
}

/*******************************************************************************************************************//**
 * @brief       Notify EP device from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_rc_notify_ep(void)
{
    volatile uint32_t * p_notify_addr;

    /* Set the pointer to the RC-to-EP notification shared memory address. */
    p_notify_addr = (volatile uint32_t *)(uintptr_t)RC_TO_EP_NOTIFY_ADDR;

    *p_notify_addr = RC_TO_EP_NOTIFY_VALUE;
    __DSB();
}

/*******************************************************************************************************************//**
 * @brief       Check whether notification from EP device has been received.
 * @param[in]   None
 * @retval      true     EP notification was received
 * @retval      false    EP notification was not received
 **********************************************************************************************************************/
bool pcie_rc_check_ep_notify(void)
{
    volatile uint32_t * p_notify_addr;
    uint32_t notify_value;

    /* Set the pointer to the EP-to-RC notification shared memory address. */
    p_notify_addr = (volatile uint32_t *)(uintptr_t)EP_TO_RC_NOTIFY_ADDR;

    notify_value = *p_notify_addr;
    __DSB();

    return ((notify_value) == (EP_TO_RC_NOTIFY_VALUE));
}

/*******************************************************************************************************************//**
 * @brief       Wait for notification from EP device. RC polls EP_TO_RC_NOTIFY_ADDR every 1 ms.
 * @param[in]   None
 * @retval      true     EP notification was received before timeout
 * @retval      false    Timeout occurred
 **********************************************************************************************************************/
static bool pcie_rc_wait_ep_notify(void)
{
    uint32_t timeout_count = PCIE_RC_EP_NOTIFY_TIMEOUT_MS;

    while ((0U) < (timeout_count))
    {
        if (pcie_rc_check_ep_notify())
        {
            return true;
        }

        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
        timeout_count--;
    }

    return false;
}

/*******************************************************************************************************************//**
 * @brief       Clear notification flag for EP device from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_rc_clear_ep_notify(void)
{
    volatile uint32_t * p_notify_addr;

    /* Set the pointer to the RC-to-EP notification shared memory address. */
    p_notify_addr = (volatile uint32_t *)(uintptr_t)RC_TO_EP_NOTIFY_ADDR;

    *p_notify_addr = PCIE_NOTIFY_CLEAR_VALUE;
    __DSB();
}

/*******************************************************************************************************************//**
 * @brief       Verify writeback data from EP device.
 * @param[in]   None
 * @retval      true     Writeback data matched
 * @retval      false    Writeback data did not match
 **********************************************************************************************************************/
static bool pcie_rc_verify_ep_writeback_data(void)
{
    /* Point to the RC-written test data buffer. */
    uint32_t * p_rc_written_data_addr = (uint32_t *)(uintptr_t) PCIE_TEST_DATA_ADDR;

    /* Point to the EP write-back data buffer. */
    uint32_t * p_ep_writeback_data_addr = (uint32_t *)(uintptr_t) EP_TO_RC_WRITEBACK_ADDR;
    uint32_t   offset;

    printf("      Compare RC -> EP write data with EP -> RC writeback data\n");
    for (offset = 0U; offset < (PCIE_TEST_DATA_SIZE / 4U); offset++)
    {
        if (p_rc_written_data_addr[offset] != p_ep_writeback_data_addr[offset])
        {
            printf("    Verify error! index=%u expect=0x%08x writeback=0x%08x\n",offset,p_rc_written_data_addr[offset],
                   p_ep_writeback_data_addr[offset]);
            return false;
        }
    }

    printf("      EP writeback verify OK\n");
    return true;
}

/*******************************************************************************************************************//**
 * @brief       Read EP device configuration register from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_config_read_test()
{
    printf("\n");
    printf("   * Read EP Device Configuration Registers From RC Device\n");

    /* Read the PCI_EP_RID_CC_F0 configuration register. */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 2U;

    /* PCI_EP_RID_CC_F0_ADDR(0x92106008) */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);

    /* Display the Revision ID / Class Code register value read from the EP configuration space. */
    printf("      PCI_EP_RID_CC_F0 = 0x%08x\n",  (unsigned int)g_pcie_rc0_cfg_reg_read.data);

    /* Read the PCI_EP_SUBSID_F0 configuration register. */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 11U;

    /* PCI_EP_SUBSID_F0_ADDR(0x9210602C) */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);

    /* Display the Subsystem Vendor ID / Subsystem ID register value read from the EP configuration space. */
    printf("      PCI_EP_SUBSID_F0 = 0x%08x\n", (unsigned int)g_pcie_rc0_cfg_reg_read.data);
}

/*******************************************************************************************************************//**
 * @brief       Write EP device configuration register from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_config_write_test()
{
    printf("\n");
    printf("   * Write EP Device Configuration Registers From RC Device\n");
    printf("      Write 1 in PCI_EP_COM_STA_F0 bit10\n");

    /* Read the PCI_EP_COM_STA_F0 configuration register before writing. */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;
    /* PCI_EP_COM_STA_F0(0x92106004) */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);

    /* Display the Command/Status register value before updating the configuration register. */
    printf("      PCI_EP_COM_STA_F0 = 0x%08x(Before writing)\n", (unsigned int)g_pcie_rc0_cfg_reg_read.data);

    /* Set bit 10 and write the register back. */
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0(0x92106004) */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000400;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);

    /* Display the Command/Status register value after updating the configuration register. */
    printf("      PCI_EP_COM_STA_F0 = 0x%08x(After writing)\n", (unsigned int)g_pcie_rc0_cfg_reg_read.data);
}

/*******************************************************************************************************************//**
 * @brief       Read/write EP device memory from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_read_write_test()
{
    uint32_t i = 0;
    uint32_t rdata = 0;
    uint64_t mask_addr = PCIE_TEST_DATA_SIZE;

    printf("\n");
    printf("   * Read/Write EP Device Memory From RC Device\n");
    printf("      Write Value to AXI Window of EP Device\n");

    /* memory bus enable (EP) */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    R_PCIE0->PCI_RC_COM_STA |= 0x00000006;

    printf("      Transfer Start!\n");
    for (i = 0U; (i) < (mask_addr); i += 4U)
    {
        /* Write data to PCIe test memory */
        *(volatile uint32_t *)((uintptr_t)PCIE_TEST_DATA_ADDR + i) =
                /* Write data to PCIe test memory */
            (uint32_t)((uintptr_t)PCIE_TEST_DATA_ADDR + i);
        __DSB();

        /* Read data from PCIe test memory */
        rdata = *(volatile uint32_t *)((uintptr_t)PCIE_TEST_DATA_ADDR + i);

        /* data verify */
        if ((rdata) != (PCIE_TEST_DATA_ADDR + i))
        {
            /* Display the index and data value when a data transfer verification fails. */
            printf("      Transfer Failed : Data No.%u=%08x\n", (unsigned int)i, (unsigned int)rdata);
        }
    }
    printf("      Transfer Completed!!\n");
}

/*******************************************************************************************************************//**
 * @brief       DMAC read/write EP device memory from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_dmac_read_write_test()
{
    uint32_t * write_buf;
    uint32_t * read_buf;
    uint32_t   offset;

    /* Get a pointer to the PCIe RC extended configuration structure. */
    pcie_rc_extended_cfg_t * p_extend = (pcie_rc_extended_cfg_t *)g_pci_rc0_cfg.p_extend;
    uint64_t dst_addr = p_extend->pci_window_settings[0].destination_address;

    printf("\n");
    printf("   * DMAC Read/Write EP Device Memory From RC Device\n");
    printf("      Write Value to AXI Window of EP Device\n");
    printf("      Transfer Start!\n");

    /* PCIe DMAC Write (AXI -> PCIe) */
    R_PCIE_RC_DmacSet(&g_pci_rc0_ctrl, PCIE_DMAC_CH, AXItoPCI, PCI_DMAC_WRITE_ADDR, dst_addr, PCIE_WINDOW_SIZE);
    R_PCIe_RC_DmacStart(&g_pci_rc0_ctrl, PCIE_DMAC_CH);

    /* PCIe DMAC Read  (PCIe -> AXI) */
    R_PCIE_RC_DmacSet(&g_pci_rc0_ctrl, PCIE_DMAC_CH, PCItoAXI, dst_addr,
                       PCI_DMAC_WRITE_ADDR + PCIE_WINDOW_SIZE, PCIE_WINDOW_SIZE);
    R_PCIe_RC_DmacStart(&g_pci_rc0_ctrl, PCIE_DMAC_CH);

    /* data verify */
    write_buf = (uint32_t *)PCI_DMAC_WRITE_ADDR;
    read_buf  = (uint32_t *)(PCI_DMAC_WRITE_ADDR + PCIE_WINDOW_SIZE);

    for (offset = 0U; offset < (PCIE_WINDOW_SIZE / 4U); offset++)
    {
        if (write_buf[offset] != read_buf[offset])
        {
            printf("    Data mismatch! index=%u write=0x%08x read=0x%08x\n",offset,write_buf[offset],read_buf[offset]);
            break;
        }
    }

    if ((offset) == (PCIE_TEST_DATA_SIZE / 4))
    {
        printf("      Transfer Completed!!\n");
    }
}

/*******************************************************************************************************************//**
 * @brief       Measure memory read and write speed between RC device and EP device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_read_write_speed_test()
{
    printf("\n");
    printf("   Memory R/W Test buffer size ... 0x%08x\n", PCIE_TEST_DATA_SIZE);
    pcie_memory_write_speed_test(PCIE_TEST_DATA_SIZE);
    pcie_memory_read_speed_test(PCIE_TEST_DATA_SIZE);
}

/*******************************************************************************************************************//**
 * @brief       Measure the speed of writing to EP device memory from RC device.
 * @param[in]   write_buf    Number of bytes to write
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_write_speed_test(uint32_t write_buf)
{
    uint32_t i = 0;
    uint32_t pcie_mirror_addr = PCIE_TEST_DATA_ADDR;

    printf("   * Measure the speed of writing to EP Device Memory From RC Device\n");

    /* memory bus enable (EP) */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    R_PCIE0->PCI_RC_COM_STA |= 0x00000006;

    R_CMTW_Reset(&g_timer0_ctrl);
    R_CMTW_Start(&g_timer0_ctrl);

    for (i = 0U; (i) < (write_buf); i += 4U)
    {
        /* Initialize the PCIe mirror memory with its own address as the test pattern. */
        *(volatile uint32_t *)((uintptr_t)pcie_mirror_addr + i) =
                /* Initialize the PCIe mirror memory with its own address as the test pattern. */
            (uint32_t)((uintptr_t)pcie_mirror_addr + i);
        __DSB();
    }

    R_CMTW_Stop(&g_timer0_ctrl);
    R_CMTW_StatusGet(&g_timer0_ctrl, &g_timer_status);

    /* Convert the elapsed counter value to microseconds. */
    g_momory_RW_time = g_timer_status.counter * g_us_counter_time;

    printf("      write time      = %.3lf[us]\n", g_momory_RW_time);
}

/*******************************************************************************************************************//**
 * @brief       Measure the speed of reading from EP device memory to RC device.
 * @param[in]   read_buf    Number of bytes to read
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_read_speed_test(uint32_t read_buf)
{
    uint32_t i = 0;
    uint32_t rdata = 0;
    uint32_t pcie_mirror_addr = PCIE_TEST_DATA_ADDR;

    printf("   * Measure the speed of reading to EP device memory from RC device\n");

    /* memory bus enable (EP) */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    R_PCIE0->PCI_RC_COM_STA |= 0x00000006;

    R_CMTW_Reset(&g_timer0_ctrl);

    for (i = 0U; (i) < (read_buf); i += 4U)
    {
        R_CMTW_Start(&g_timer0_ctrl);
        /* Read back the data from the PCIe mirror memory for verification. */
        rdata = *(volatile uint32_t *)((uintptr_t)pcie_mirror_addr + i);
        R_CMTW_Stop(&g_timer0_ctrl);

        /* data verify */
        if ((rdata) != (pcie_mirror_addr + i))
        {
            /* Display the index and data value when the read-back verification fails. */
            printf("      Transfer Failed : Data No.%u=%08x\n", (unsigned int)i, (unsigned int)rdata);
        }
    }

    R_CMTW_StatusGet(&g_timer0_ctrl, &g_timer_status);

    /* Convert the elapsed counter value to microseconds. */
    g_momory_RW_time = g_timer_status.counter * g_us_counter_time;

    printf("      read time       = %.3lf[us]\n", g_momory_RW_time);
}

/*******************************************************************************************************************//**
 * @brief       Measure DMAC memory read and write speed between RC device and EP device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_dmac_read_write_speed_test()
{
    printf("\n");
    printf("   DMAC R/W Test buffer size   ... 0x%08x\n", PCIE_WINDOW_SIZE);
    /* PCIe DMAC Write */
    pcie_dmac_read_speed_test();
    /* PCIe DMAC Read */
    pcie_dmac_write_speed_test();
}

/*******************************************************************************************************************//**
 * @brief       Measure the speed of DMAC writing to EP device memory from RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_dmac_write_speed_test()
{
    /* Configure the DMAC transfer from local AXI memory to the EP's PCIe window. */
    R_PCIE_RC_DmacSet(&g_pci_rc0_ctrl, PCIE_DMAC_CH, AXItoPCI, PCI_DMAC_WRITE_ADDR,
                       ((pcie_rc_extended_cfg_t*)g_pci_rc0_cfg.p_extend)->pci_window_settings[0].destination_address,
                       PCIE_TEST_DATA_SIZE);

    /* memory bus enable (EP) */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    R_PCIE0->PCI_RC_COM_STA |= 0x00000006;

    R_CMTW_Reset(&g_timer0_ctrl);
    R_CMTW_Start(&g_timer0_ctrl);

    R_PCIe_RC_DmacStart(&g_pci_rc0_ctrl, PCIE_DMAC_CH);

    R_CMTW_Stop(&g_timer0_ctrl);
    R_CMTW_StatusGet(&g_timer0_ctrl, &g_timer_status);

    /* Convert the elapsed counter value to microseconds. */
    g_momory_RW_time = g_timer_status.counter * g_us_counter_time;

    printf("   * Measure the speed of DMAC writing to EP Device Memory From RC Device\n");
    printf("      DMAC write time = %.3lf[us]\n", g_momory_RW_time);
}

/*******************************************************************************************************************//**
 * @brief       Measure the speed of DMAC reading from EP device memory to RC device.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_dmac_read_speed_test()
{
    /* Configure the DMAC transfer from the EP's PCIe window to local AXI memory. */
    R_PCIE_RC_DmacSet(&g_pci_rc0_ctrl, PCIE_DMAC_CH, PCItoAXI,
                       ((pcie_rc_extended_cfg_t*)g_pci_rc0_cfg.p_extend)->pci_window_settings[0].destination_address,
                       PCI_DMAC_WRITE_ADDR + PCIE_TEST_DATA_SIZE, PCIE_TEST_DATA_SIZE);

    /* memory bus enable (EP) */
    g_pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &g_pcie_rc0_cfg_reg_read);
    g_pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    g_pcie_rc0_cfg_reg_write.data = g_pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &g_pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    R_PCIE0->PCI_RC_COM_STA |= 0x00000006;

    R_CMTW_Reset(&g_timer0_ctrl);
    R_CMTW_Start(&g_timer0_ctrl);

    R_PCIe_RC_DmacStart(&g_pci_rc0_ctrl, PCIE_DMAC_CH);

    R_CMTW_Stop(&g_timer0_ctrl);
    R_CMTW_StatusGet(&g_timer0_ctrl, &g_timer_status);

    /* Convert the elapsed counter value to microseconds. */
    g_momory_RW_time = g_timer_status.counter * g_us_counter_time;

    printf("   * Measure the speed of DMAC reading to EP Device Memory From RC Device\n");
    printf("      DMAC read time  = %.3lf[us]\n", g_momory_RW_time);
}

/*******************************************************************************************************************//**
 * @brief       Receives a callback for the sci_uart instruction and processes it according to the events in the
 *              received callback.
 * @param[in]   p_args    Pointer to argument information
 * @retval      None
 **********************************************************************************************************************/
void user_uart_callback (uart_callback_args_t * p_args)
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Received a character */
        case UART_EVENT_RX_CHAR:
        {
            /* Only put the next character in the receive buffer if there is space for it */
            if ((sizeof(g_out_of_band_received)) > (g_out_of_band_index))
            {
                /* Write either the next one or two bytes depending on the receive data size */
                if (((UART_DATA_BITS_7) == (g_uart0_cfg.data_bits)) || ((UART_DATA_BITS_8) == (g_uart0_cfg.data_bits)))
                {
                    /* Store the received byte and advance the receive buffer index. */
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
                else
                {
                    /* Store 9-bit receive data only when two bytes remain in the receive buffer. */
                    if ((sizeof(g_out_of_band_received) - g_out_of_band_index) >= sizeof(uint16_t))
                    {
                        uint16_t received_data = (uint16_t) p_args->data;

                        memcpy(&g_out_of_band_received[g_out_of_band_index],
                               &received_data,
                               sizeof(received_data));
                        g_out_of_band_index += sizeof(received_data);
                    }
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
        }
    }
}

/*******************************************************************************************************************//**
 * @brief       Handles an error returned by an FSP API call.
 * @param[in]   err    Error code returned by the FSP API
 * @retval      None
 **********************************************************************************************************************/
void handle_error (fsp_err_t err)
{
    FSP_PARAMETER_NOT_USED(err);
}

/*******************************************************************************************************************//**
 * @brief       SVC exception handler.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
