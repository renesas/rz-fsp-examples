/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains the main application entry point for the Ethernet example.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include "sio_char.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event)
BSP_PLACE_IN_SECTION(".warm_start");
BSP_ATTRIBUTE_STACKLESS void R_BSP_WarmStart_StackLess(void)
BSP_PLACE_IN_SECTION(".warm_start");
FSP_CPP_FOOTER

void hal_entry(void);
void test_main(void);
void handle_error(fsp_err_t err);
void user_uart_callback(uart_callback_args_t * p_args);

uint8_t           g_out_of_band_received[TRANSFER_LENGTH];
volatile uint32_t g_transfer_complete = 0U;
volatile uint32_t g_receive_complete  = 0U;
uint32_t          g_out_of_band_index = 0U;

char gbuff[16];

/*******************************************************************************************************************//**
 * @brief Main application entry point.
 *
 * Opens the existing SCI UART console, enables interrupts, and starts the Ethernet multi-port ping example.
 *
 * @retval None
 **********************************************************************************************************************/
void hal_entry(void)
{
    fsp_err_t err;

    /* Open the UART instance used by the existing application console. */
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_error(err);

    __enable_irq();

    test_main();
}

/*******************************************************************************************************************//**
 * @brief UART callback used by the application console.
 *
 * @param[in] p_args UART callback arguments.
 *
 * @retval None
 **********************************************************************************************************************/
void user_uart_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            if (sizeof(g_out_of_band_received) > g_out_of_band_index)
            {
                if ((UART_DATA_BITS_7 == g_uart0_cfg.data_bits) ||
                    (UART_DATA_BITS_8 == g_uart0_cfg.data_bits))
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
 * @brief Handles an FSP initialization error.
 *
 * @param[in] err FSP error code.
 *
 * @retval None
 **********************************************************************************************************************/
void handle_error(fsp_err_t err)
{
    if (FSP_SUCCESS != err)
    {
        while (1)
        {
            __NOP();
        }
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
