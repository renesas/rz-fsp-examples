/***********************************************************************************************************************
 * File Name    : spi_ep.c
 * Description  : Contains data structures and functions used in spi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "spi_ep.h"
#include "MX25L12835F/MX25L12835F.h"

/***********************************************************************************************************************
 * @addtogroup r_rspi_ep
 * @{
 **********************************************************************************************************************/

/*
 * Private function declarations
 */

/* Master Transfer Event completion flag */
static volatile spi_event_t g_master_event_flag;

/* The number of times the master callback has been called */
volatile uint32_t g_spi_master_callback_count;

/* Drives the mx25l1235f CS pin LOW */
static void mx25l1235f_cs_assert ();

/* Drives the mx25l1235f CS pin HIGH */
static void mx25l1235f_cs_deassert ();

/* Completes a SPI transfer to the mx25l1235f */
static mx25l1235f_status_t mx25l1235f_spi_transfer_bulk (uint8_t const * tx_data, uint8_t * rx_data, uint32_t length);

/* Provides HW abstraction for mx25l1235f driver */
static mx25l1235f_drv_t g_mx25l1235f_drv;

/***********************************************************************************************************************
 * @brief       This functions initializes SPI master and slave modules.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                                 Upon successful open of SPI module.
 * @retval      Any Other Error code apart from FSP_SUCCES  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t spi_init(void)
{
    fsp_err_t err = FSP_SUCCESS;  /* Error status */

    R_IOPORT_PinWrite(&g_ioport_ctrl, MX25L12835F_SSL_PIN, BSP_IO_LEVEL_HIGH);

    /* Open/Initialize SPI Master module */
    err = R_RSPI_Open(&g_spi_master_ctrl, &g_spi_master_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* SPI Master Failure message */
        APP_ERR_PRINT("** R_RSPI_Open API failed **\r\n");
        return err;
    }

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Close SPI master */
        if ((FSP_SUCCESS != R_RSPI_Close(&g_spi_master_ctrl)))
        {
            /* SPI Master Close Failure message */
            APP_ERR_PRINT("** R_RSPI_Close API failed **\r\n");
        }

        /* SPI Slave Failure message */
        APP_ERR_PRINT("** R_RSPI_Open API failed **\r\n");
        return err;
    }

    /* Configure the MX25l1235F */
    g_mx25l1235f_drv = (mx25l1235f_drv_t){
        .transfer    = mx25l1235f_spi_transfer_bulk,
        .cs_assert   = mx25l1235f_cs_assert,
        .cs_deassert = mx25l1235f_cs_deassert
    };
    mx25l1235f_status_t status = MX25L12835F_Setup(&g_mx25l1235f_drv);
    if (MX25L12835F_SUCCESS != status)
    {
        APP_PRINT("** MX25L12835F_Setup failed **\r\n");
        spi_clean_up();
        APP_ERR_TRAP(status);
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function demos R_RSPI_Write().
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                                 Upon successful SPI Write.
 * @retval      Any Other Error code apart from FSP_SUCCES  Unsuccessful Write and Read.
 **********************************************************************************************************************/
fsp_err_t spi_write(void)
{
    /* Initialize buffers to known values */
    uint8_t tx_data[MX25L12835F_PAGE_SIZE] = { 0 };
    mx25l1235f_status_t err;
    uint32_t num_bytes = RESET_VALUE;  /* Number of bytes read by SEGGER real-time-terminal */

    /* Input to master buffer */
    APP_PRINT("\r\nEnter text input to write into PMOD SF3. Data size should not exceed 256 bytes.\r\n");
    while (BYTES_RECEIVED_ZERO == num_bytes)
    {
        if (APP_CHECK_DATA)
        {
            num_bytes = APP_READ(tx_data);

            if (BYTES_RECEIVED_ZERO == num_bytes)
            {
                APP_PRINT("\r\nNo Input\r\n");
            }
        }
    }

    APP_PRINT("\r\n%sData write to PMOD SF3: %s%s%s%s", RTT_CTRL_RESET, RTT_CTRL_BG_BRIGHT_WHITE,
              RTT_CTRL_TEXT_BRIGHT_BLUE, tx_data, RTT_CTRL_RESET);

    /* Erase a sector on the MX25l1235F */
    err = MX25L12835F_EraseSector(&g_mx25l1235f_drv, MX25L12835F_ADDRESS);
    if (MX25L12835F_SUCCESS != err)
    {
        APP_PRINT("** MX25L12835F_EraseSector failed **\r\n");
        spi_clean_up();
        APP_ERR_TRAP(err);
    }

    /* Write a page to the MX25l1235F */
    err = MX25L12835F_PageProgram(&g_mx25l1235f_drv, MX25L12835F_ADDRESS, tx_data, sizeof(tx_data));
    if (MX25L12835F_SUCCESS != err)
    {
        APP_PRINT("** MX25L12835F_PageProgram failed **\r\n");
        spi_clean_up();
        APP_ERR_TRAP(err);
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function demos R_RSPI_Read().
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful SPI Read.
 * @retval      Any Other Error code apart from FSP_SUCCES  Unsuccessful Read.
 **********************************************************************************************************************/
fsp_err_t spi_read(void)
{
    /* Initialize buffers to known values */
    uint8_t rx_data[MX25L12835F_PAGE_SIZE] = { 0 };
    mx25l1235f_status_t err;

    /* Read the page */
    err = MX25L12835F_ReadData(&g_mx25l1235f_drv, MX25L12835F_ADDRESS, rx_data, sizeof(rx_data));
    if (MX25L12835F_SUCCESS != err)
    {
        APP_PRINT("** MX25L12835F_ReadData failed **\r\n");
        spi_clean_up();
        APP_ERR_TRAP(err);
    }

    APP_PRINT("\r\n%sData read from PMOD SF3: %s%s%s%s", RTT_CTRL_RESET, RTT_CTRL_BG_BRIGHT_WHITE,
              RTT_CTRL_TEXT_BRIGHT_BLUE, rx_data, RTT_CTRL_RESET);

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function closes both SPI Master and Slave modules.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  SPI module closed successfully.
 * @retval      Any Other Error code apart from FSP_SUCCES  Unsuccessful close.
 **********************************************************************************************************************/
fsp_err_t spi_exit_demo(void)
{
    fsp_err_t err = FSP_SUCCESS; /* Error status */

    MX25L12835F_Teardown(&g_mx25l1235f_drv);

    /* Close SPI Master module */
    err = R_RSPI_Close(&g_spi_master_ctrl);

    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nMaster R_RSPI_Close() failed");
        return err;
    }

    return err;
}

/***********************************************************************************************************************
 * @brief      Master SPI callback function.
 * @param[IN]  p_args  Callback arguments.
 * @retval     None.
 **********************************************************************************************************************/
void spi_master_callback(spi_callback_args_t *p_args)
{
    g_spi_master_callback_count++;
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_master_event_flag = SPI_EVENT_TRANSFER_COMPLETE;
    }
    else
    {
        g_master_event_flag = SPI_EVENT_TRANSFER_ABORTED;
    }
}

/***********************************************************************************************************************
 * @brief      Drive the MX25L1235F CS pin low.
 * @param[IN]  None.
 * @retval     None.
 **********************************************************************************************************************/
static void mx25l1235f_cs_assert()
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MX25L12835F_SSL_PIN, BSP_IO_LEVEL_LOW);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
}

/***********************************************************************************************************************
 * @brief      Drive the MX25L1235F CS pin high.
 * @param[IN]  None.
 * @retval     None.
 **********************************************************************************************************************/
static void mx25l1235f_cs_deassert()
{
    R_IOPORT_PinWrite(&g_ioport_ctrl, MX25L12835F_SSL_PIN, BSP_IO_LEVEL_HIGH);
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MICROSECONDS);
}

/***********************************************************************************************************************
 * @brief       Implement the MX25L1235F transfer interface.
 * @param[IN]   tx_data              The transmit data.
 * @param[IN]   rx_data              The receive data.
 * @param[IN]   length               The length.
 * @retval      MX25L12835F_SUCCESS  Data has finished transferring.
 * @retval      MX25L12835F_ERROR    An error occurred during the operation.
 **********************************************************************************************************************/
static mx25l1235f_status_t mx25l1235f_spi_transfer_bulk(uint8_t const *tx_data, uint8_t *rx_data, uint32_t length)
{
    /* Reset the callback state before starting a transfer */
    g_spi_master_callback_count = 0;
    g_master_event_flag = SPI_EVENT_TRANSFER_ABORTED;

    if (0 == tx_data)
    {
        /* If no transmit buffer is given, only perform a read operation */
        if (FSP_SUCCESS != R_RSPI_Read(&g_spi_master_ctrl, rx_data, length, SPI_BIT_WIDTH_8_BITS))
        {
            return MX25L12835F_ERROR;
        }
    }
    else if (0 == rx_data)
    {
        /* If no receive buffer is given, only perform a write operation */
        if (FSP_SUCCESS != R_RSPI_Write(&g_spi_master_ctrl, tx_data, length, SPI_BIT_WIDTH_8_BITS))
        {
            return MX25L12835F_ERROR;
        }
    }
    else
    {
        /* Perform a write and read simultaneously */
        if (FSP_SUCCESS != R_RSPI_WriteRead(&g_spi_master_ctrl, tx_data, rx_data, length, SPI_BIT_WIDTH_8_BITS))
        {
            return MX25L12835F_ERROR;
        }
    }

    /* Wait for a callback from the SPI instance */
    volatile uint32_t timeout = 1000000;
    while (0 == (g_spi_master_callback_count && (--timeout)))
    {
        ;
    }

    /* Verify that a timeout did not occur */
    if (0 == timeout)
    {
        return MX25L12835F_ERROR;
    }

    /* Verify that the transfer was completed */
    if (SPI_EVENT_TRANSFER_COMPLETE != g_master_event_flag)
    {
        return MX25L12835F_ERROR;
    }

    return MX25L12835F_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function closes all the opened SPI modules before the project ends up in an Error Trap.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void spi_clean_up(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close SPI module */
    err = R_RSPI_Close(&g_spi_master_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* SPI Close failure message */
        APP_ERR_PRINT("** R_RSPI_Close API failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_rspi_ep)
 **********************************************************************************************************************/
