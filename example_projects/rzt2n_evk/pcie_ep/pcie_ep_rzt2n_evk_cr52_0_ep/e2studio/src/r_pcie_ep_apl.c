/***********************************************************************************************************************
 * File Name    : r_pcie_ep_apl.c
 * Description  : PCIe Gen2 End Point example application functions.
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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "common_utils.h"

/******************************************************************************
 Macro definitions
 ******************************************************************************/
/* Offset definitions */
#define PCIE_EP_WINDOW_OFFSET            (0x10000U)
#define PCIE_TEST_DATA_OFFSET            (0x04U)

/* Mirror address of the PCIe area (e.g., 0xA000_0000 maps to PCIe address 0x4_0000_0000). */
#define PCIE_MIRROR_BASE_ADDR            (BSP_CFG_EL1_MPU_REGION19_BASE)
#define PCIE_WINDOW_SIZE                 (0x1000U)

/* EP -> RC notification flag
 * EP writes this value to notify RC that EP-side writeback has completed. */
#define EP_TO_RC_NOTIFY_ADDR             (PCIE_MIRROR_BASE_ADDR + PCIE_EP_WINDOW_OFFSET)  /*0xA0010000*/
#define EP_TO_RC_NOTIFY_VALUE            (0x45505243U)  /* "EPRC" */

/* RC -> EP notification flag
 * RC writes this value to notify EP that RC-side memory write has completed. */
#define RC_TO_EP_NOTIFY_ADDR             (0x10180000U)
#define RC_TO_EP_NOTIFY_VALUE            (0x52434550U)  /* "RCEP" */

/* The size of the test data area that EP writes back to RC memory,
 * corresponding to the data size written by RC to EP. */
#define EP_WRITEBACK_SIZE                (PCIE_WINDOW_SIZE - PCIE_TEST_DATA_OFFSET)

/* This address within EP's PCIe Window region is used by EP to write back test data
 * to the RC's memory space, starting immediately after the "EP -> RC notification flag". */
#define EP_WRITEBACK_DST_ADDR            (EP_TO_RC_NOTIFY_ADDR + PCIE_TEST_DATA_OFFSET)  /*0xA0010004*/

/* This is the source address within EP's AXI Window region where the test data
 * (originally written by RC) is stored, and from which EP reads the data
 * for write-back. It starts immediately after the "RC -> EP notification flag" location. */
#define EP_WRITEBACK_SRC_ADDR            (RC_TO_EP_NOTIFY_ADDR + PCIE_TEST_DATA_OFFSET)  /*0x10180004*/

/******************************************************************************
 Exported global functions
 ******************************************************************************/
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER
void handle_error (fsp_err_t err);
void user_uart_callback (uart_callback_args_t * p_args);
void PCIe_ep_function_test(void);
void pcie_memory_display_menu(void);
void pcie_memory_display(uint64_t aw_dest, uint64_t aw_mask);
void SVC_Handler(void);
void pcie_ep_writeback_to_rc(void);
bool pcie_ep_check_rc_notify(void);
void pcie_ep_notify_rc(void);
bool get_nonblocking(char * p_menu);
bool main_menu_execute(char menu);

/******************************************************************************
 Global variables
 ******************************************************************************/
uint8_t  g_out_of_band_received[TRANSFER_LENGTH];
uint32_t g_out_of_band_index = 0;
volatile uint32_t g_transfer_complete = 0;
volatile uint32_t g_receive_complete  = 0;
char g_buff[16];
char g_menu_input;
bool g_prev_rc_notify = false;
pci_configuration_register_init_t const g_pci_ep0_configuration_register_init_list
[PCIE_EP_CH0_F0_CONFIG_REG_INIT_LIST_NUM];

/******************************************************************************
 Program
 ******************************************************************************/
/*******************************************************************************************************************//**
 * @brief       PCIe example application (EP device). Initializes it as an EP device, configures the AXI Windows,
 *              and runs the interactive test menu.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void PCIe_ep_function_test(void)
{
    /* TODO: add your own code here */
    /**** Open the transfer instance with initial configuration. */
    fsp_err_t err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_error(err);

    __enable_irq();
    APP_PRINT(BANNER_INFO,EP_VERSION,4, 2, 0);
    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates PCIe Gen2 End Point communication.\r\n");

    APP_PRINT("\r\n");
    /**** pcie test start */
    APP_PRINT("---- PCIe OPERATION TEST START ~ EP Devise Side ~ ----\n");

    APP_PRINT("\r\n");
    /**** pcie ep init ch */
    APP_PRINT("   PCIe %dch Initialization End Point Mode\n", g_pci_ep0_cfg.channel);
    APP_PRINT("\r\n");
    R_PCIE_EP_Open(&g_pci_ep0_ctrl, &g_pci_ep0_cfg);

    /**** display main menu */
    pcie_memory_display_menu();

    while(1)
    {
        bool rc_notify = pcie_ep_check_rc_notify();

        if ((rc_notify && (!g_prev_rc_notify)))
        {
            pcie_ep_writeback_to_rc();
            pcie_ep_notify_rc();
        }

        g_prev_rc_notify = rc_notify;

        if (get_nonblocking(&g_menu_input))
        {
            if (main_menu_execute(g_menu_input))
            {
                pcie_memory_display_menu();
            }
            else
            {
                break;
            }
        }
    }
    while (1)
    {
        /* Wait here */
    }
}

/*******************************************************************************************************************//**
 * @brief       Checks whether the RC has signaled the EP via the RC -> EP notification flag.
 * @param[in]   None
 * @retval      true     The RC notification flag has been set
 * @retval      false    The RC notification flag has not been set
 **********************************************************************************************************************/
bool pcie_ep_check_rc_notify(void)
{
    volatile uint32_t * p_notify_addr;
    uint32_t notify_value;

    /* Set the pointer to the RC-to-EP notification shared memory address. */
    p_notify_addr = (volatile uint32_t *)(uintptr_t)RC_TO_EP_NOTIFY_ADDR;

    notify_value = *p_notify_addr;
    __DSB();

    return (notify_value == RC_TO_EP_NOTIFY_VALUE);
}

/*******************************************************************************************************************//**
 * @brief       Writes the test data received from the RC back to the RC's memory space through the EP PCIe window.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_ep_writeback_to_rc(void)
{
    uint32_t i = 0;
    uint64_t mask_addr = EP_WRITEBACK_SIZE;

    R_PCIE0->PCI_EP_COM_STA_F0 |= 0x00000006;

    for (i = 0U; (i < mask_addr); i += 4U)
    {
        /* Read a 32-bit value from the EP write-back source address. */
        uint32_t data = *(volatile uint32_t *)(uintptr_t)(EP_WRITEBACK_SRC_ADDR + i);
        /* Write the 32-bit value to the EP write-back destination address. */
        *(volatile uint32_t *)(uintptr_t)(EP_WRITEBACK_DST_ADDR + i) = data;
        __DSB();
    }
}

/*******************************************************************************************************************//**
 * @brief       Notifies the RC that the EP has completed the write-back operation.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_ep_notify_rc(void)
{
    volatile uint32_t * p_notify_addr;
    /* Set the pointer to the EP-to-RC notification shared memory address. */
    p_notify_addr = (volatile uint32_t *)(uintptr_t)EP_TO_RC_NOTIFY_ADDR;
    *p_notify_addr = EP_TO_RC_NOTIFY_VALUE;
    __DSB();
}

/*******************************************************************************************************************//**
 * @brief       Performs a non-blocking check for a menu character received via UART.
 * @param[in]   p_menu    Pointer to the location where the received character is stored
 * @retval      true      A character was available and has been stored in p_menu
 * @retval      false     No character was available
 **********************************************************************************************************************/
bool get_nonblocking(char * p_menu)
{
    if (g_out_of_band_index > 0U)
    {
        /* Store the first received USB CDC character into the menu variable. */
        *p_menu = (char) g_out_of_band_received[0];

        g_out_of_band_index = 0U;
        g_out_of_band_received[0] = 0U;

        return true;
    }

    return false;
}

/*******************************************************************************************************************//**
 * @brief       Executes the action associated with the selected menu option.
 * @param[in]   menu    The menu character entered by the user
 * @retval      true    The menu should continue to be displayed
 * @retval      false   The program should exit the main loop
 **********************************************************************************************************************/
bool main_menu_execute(char menu)
{
    /* Ignore CR/LF and empty Enter presses instead of reporting a false test failure. */
    if (('\r' == menu) || ('\n' == menu))
    {
        return true;
    }

    printf ("   input = %c \n", menu);

    if ('1' == menu)
    {

        pcie_memory_display(
            /* Destination address configured for AXI window 0. */
            ((pcie_ep_extended_cfg_t*)g_pci_ep0_cfg.p_extend)->axi_window_settings[0].destination_address,
            /* Address mask configured for AXI window 0. */
            ((pcie_ep_extended_cfg_t*)g_pci_ep0_cfg.p_extend)->axi_window_settings[0].mask_address);
        printf("\n");
        return true;
    }
    else if ('2' == menu)
    {
        printf("-------------- PCIe OPERATION TEST END ---------------\n");
        return false;
    }
    else
    {
        printf("   Invalid input. Please enter 1 or 2.\n");
        printf("\n");
        return true;
    }
}

/*******************************************************************************************************************//**
 * @brief       Display menu.
 * @param[in]   None
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_display_menu(void)
{
    printf("   * Display AXI Window of EP Device\n");
    printf("   [1] = Scan start.\n");
    printf("   [2] = The end of the program.\n");
    printf("   Select No.> \n");
}

/*******************************************************************************************************************//**
 * @brief       Display AXI Window of EP Device.
 * @param[in]   aw_dest    Destination address of the AXI window
 * @param[in]   aw_mask    Mask address of the AXI window
 * @retval      None
 **********************************************************************************************************************/
void pcie_memory_display(uint64_t aw_dest, uint64_t aw_mask)
{
    volatile uint32_t data;
    /* Convert the AXI window destination address to an integer address. */
    uintptr_t address = (uintptr_t) aw_dest;

    /* Calculate the end address of the AXI window using the address mask. */
    uintptr_t end_address = (uintptr_t) (aw_dest + aw_mask);

    printf("   AXI window memory\n");
    printf("   ---------------------------------------------------\n");
    printf("   | Address     0        4        8        C        |\n");

    while (address < end_address)
    {
        /* Print the current memory address. */
        printf("   | %p ", (void*)address);

        for (int i = 0; i < 4; ++i)
        {
            /* Read a 32-bit value from the current memory address. */
            data = *(volatile uint32_t*)address;
            /* Print the read data in hexadecimal format. */
            printf(" %08x", (unsigned int)data);
            address += 4U;
        }

        printf(" |\n");
    }

    printf("   ---------------------------------------------------\n");
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
                if ((UART_DATA_BITS_7 == g_uart0_cfg.data_bits) || (UART_DATA_BITS_8 == g_uart0_cfg.data_bits))
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
void SVC_Handler(void)
{
    __asm volatile (
        "subs PC, LR, 0 \n"  /* Perform exception return*/
        ::: "memory");
}
