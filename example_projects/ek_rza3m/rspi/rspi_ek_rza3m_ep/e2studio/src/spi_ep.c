/***********************************************************************************************************************
 * File Name    : spi_ep.c
 * Description  : Contains data structures and functions used in spi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2025 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "spi_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_rspi_ep
 * @{
 **********************************************************************************************************************/

/* Private function declarations */
static void error_print (void);

/* Wait counter for wait operation monitoring */
static volatile uint32_t g_wait_count = MAX_COUNT;

/* Event flags for master and slave */
static volatile spi_event_t g_master_event_flag;    /* Master transfer event completion flag */
static volatile spi_event_t g_slave_event_flag;     /* Slave transfer event completion flag */

/* SPI module buffers for master and slave */
static uint32_t g_master_tx_buff[BUFF_LEN] __attribute__((section("UNCACHED_BSS")));   /* Master transmit buffer */
static uint32_t g_master_rx_buff[BUFF_LEN] __attribute__((section("UNCACHED_BSS")));   /* Master receive buffer */
static uint32_t g_slave_tx_buff[BUFF_LEN] __attribute__((section("UNCACHED_BSS")));    /* Slave transmit buffer */
static uint32_t g_slave_rx_buff[BUFF_LEN] __attribute__((section("UNCACHED_BSS")));    /* Slave receive buffer */

/***********************************************************************************************************************
 * @brief       This function initializes SPI master and slave modules.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful open of SPI module.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open.
 **********************************************************************************************************************/
fsp_err_t spi_init(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Initialize SPI master module */
    err = R_RSPI_Open (&g_spi_master_ctrl, &g_spi_master_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* SPI master failure message */
        APP_ERR_PRINT("** R_RSPI_Open API for SPI master failed **\r\n");
        return err;
    }

    /* Initialize SPI slave module */
    err = R_RSPI_Open (&g_spi_slave_ctrl, &g_spi_slave_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* Close SPI master */
        if (R_RSPI_Close(&g_spi_master_ctrl) != FSP_SUCCESS)
        {
            /* SPI master close failure message */
            APP_ERR_PRINT("** R_RSPI_Close API for SPI master failed **\r\n");
        }

        /* SPI slave failure message */
        APP_ERR_PRINT("** R_RSPI_Open API for SPI slave failed **\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function demos both R_RSPI_Write() and R_RSPI_Read() individually.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful SPI Write and SPI Read.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful Write and Read.
 **********************************************************************************************************************/
fsp_err_t spi_write_and_read(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */
    uint32_t num_bytes = RESET_VALUE;  /* Number of bytes read by real-time-terminal */

    /* Clean buffers */
    memset(&g_master_tx_buff[0], NULL_CHAR, sizeof(g_master_tx_buff));
    memset(&g_master_rx_buff[0], NULL_CHAR, sizeof(g_master_rx_buff));
    memset(&g_slave_rx_buff[0], NULL_CHAR, sizeof(g_slave_rx_buff));

    /* Input to master buffer */
    APP_PRINT("Enter text input for master buffer. Data size should not exceed 64 bytes.\r\n");
    while (BYTES_RECEIVED_ZERO == num_bytes)
    {
        if (APP_CHECK_DATA)
        {
            APP_READ((unsigned char*)g_master_tx_buff);
            APP_PRINT("Input data: %s\r\n" , (char *)g_master_tx_buff);

            num_bytes =  (uint32_t)strlen((char *)g_master_tx_buff);
            if (BYTES_RECEIVED_ZERO == num_bytes)
            {
                APP_PRINT("No input\r\n");
            }
        }
    }

    /* Terminal Emulator user input data 1 byte at a time. SPI transfers the data 4 bytes at a time.
     * With the below logic, we will calculate how many length of data has to be transferred. */
    if ((num_bytes % BITS_TO_BYTES) != RESET_VALUE)
    {
        num_bytes = (num_bytes / BITS_TO_BYTES) + 1U;
    }
    else
    {
        num_bytes = num_bytes / BITS_TO_BYTES;
        g_master_tx_buff[num_bytes] = RESET_VALUE;
    }

    /* Slave receives data from master */
    err = R_RSPI_Read(&g_spi_slave_ctrl, g_slave_rx_buff, num_bytes, SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Read API for slave failed\r\n");
        return err;
    }

    /* Master sends data to slave */
    err = R_RSPI_Write(&g_spi_master_ctrl, g_master_tx_buff, num_bytes, SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Write API for master failed\r\n");
        return err;
    }

    /* Wait until master write and slave read complete */
    while ((SPI_EVENT_TRANSFER_COMPLETE != g_master_event_flag) || (SPI_EVENT_TRANSFER_COMPLETE != g_slave_event_flag))
    {
        g_wait_count--;
        if (MIN_COUNT >= g_wait_count)
        {
            /* Return time out error if SPI operation fails to complete */
            err = FSP_ERR_TIMEOUT;
            APP_ERR_PRINT("SPI module blocked in Write/Read operation\r\n");
            error_print();
            return err;
        }
        else if ((SPI_EVENT_TRANSFER_ABORTED == g_master_event_flag) || \
                 (SPI_EVENT_TRANSFER_ABORTED == g_slave_event_flag))
        {
            err = FSP_ERR_TRANSFER_ABORTED;
            APP_ERR_PRINT("SPI module aborted in Write/Read operation\r\n");
            error_print();
            return err;
        }
        else
        {
            /* Do nothing */
        }

    }
    g_wait_count = MAX_COUNT;

    g_master_event_flag = (spi_event_t) RESET_VALUE;  /* Reset g_master_event_flag flag */
    g_slave_event_flag  = (spi_event_t) RESET_VALUE;  /* Reset g_slave_event_flag flag */

    /* Display master to slave transmission */
    APP_PRINT("Master transmitted user input data to slave\r\n");

    /* Slave sends data to master */
    err = R_RSPI_Write(&g_spi_slave_ctrl, g_slave_rx_buff, num_bytes, SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Write API for slave failed\r\n");
        return err;
    }

    /* Master receives data from slave */
    err = R_RSPI_Read(&g_spi_master_ctrl, g_master_rx_buff, num_bytes, SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Read API for master failed\r\n");
        return err;
    }

    /* Wait until slave write and master read complete */
    while ((SPI_EVENT_TRANSFER_COMPLETE != g_master_event_flag) || (SPI_EVENT_TRANSFER_COMPLETE != g_slave_event_flag))
    {
        /* Time out operation if SPI operation fails to complete */
        g_wait_count--;
        if (MIN_COUNT >= g_wait_count)
        {
            err = FSP_ERR_TIMEOUT;
            APP_ERR_PRINT("SPI module blocked in Write/Read operation\r\n");
            error_print();
            return err;
        }
        else if ((SPI_EVENT_TRANSFER_ABORTED == g_master_event_flag) || \
                 (SPI_EVENT_TRANSFER_ABORTED == g_slave_event_flag))
        {
            err = FSP_ERR_TRANSFER_ABORTED;
            APP_ERR_PRINT("SPI module aborted in Write/Read operation\r\n");
            error_print();
            return err;
        }
        else
        {
            /* Do nothing */
        }
    }
    g_wait_count = MAX_COUNT;

    g_master_event_flag = (spi_event_t) RESET_VALUE;  /* Reset g_master_event_flag flag */
    g_slave_event_flag  = (spi_event_t) RESET_VALUE;  /* Reset g_master_event_flag flag */

    /* Display slave to master transmission */
    APP_PRINT("Slave transmitted the data back to master\r\n");

    /* Check whether transmitted data is equal to received data */
    if (BUFF_EQUAL == memcmp(g_master_tx_buff, g_master_rx_buff, num_bytes))
    {
        /* Display master's received data on Terminal Emulator */
        APP_PRINT("Master received data: %s\r\n", (char *)g_master_rx_buff);
    }
    else
    {
        /* Return parity error in case of data mismatch */
        err = FSP_ERR_SPI_PARITY;
        APP_PRINT("Received data does not match with transmitted data\r\n");
        error_print();
        return err;
    }

    /* Delay of one second for user to verify the output */
    R_BSP_SoftwareDelay(DELAY_ONE_SEC, BSP_DELAY_UNITS_SECONDS);
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function demos R_RSPI_WriteRead() for both master and slave simultaneously.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful Write and Read for both master and slave.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful Write and Read.
 **********************************************************************************************************************/
fsp_err_t spi_write_read(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Number of bytes read by real-time-terminal for master and slave inputs */
    uint32_t num_bytes_master = RESET_VALUE;
    uint32_t num_bytes_slave = RESET_VALUE;

    /* Clean buffers */
    memset(&g_master_tx_buff[0], NULL_CHAR, sizeof(g_master_tx_buff));
    memset(&g_master_rx_buff[0], NULL_CHAR, sizeof(g_master_rx_buff));
    memset(&g_slave_tx_buff[0], NULL_CHAR, sizeof(g_slave_tx_buff));
    memset(&g_slave_rx_buff[0], NULL_CHAR, sizeof(g_slave_rx_buff));

    /* Input to master buffer */
    APP_PRINT("Enter text input for master buffer. Data size should not exceed 64 bytes.\r\n");
    while (BYTES_RECEIVED_ZERO == num_bytes_master)
    {
        if (APP_CHECK_DATA)
        {
            APP_READ((unsigned char*)g_master_tx_buff);
            APP_PRINT("\r\nInput data: %s\r\n", (char *)g_master_tx_buff);
            num_bytes_master = (uint32_t)strlen((char *)g_master_tx_buff);

            if (BYTES_RECEIVED_ZERO == num_bytes_master)
            {
                APP_PRINT("No input\r\n");
            }
        }
    }

    /* Terminal Emulator user input data 1 byte at a time. SPI transfers the data 4 bytes at a time.
     * With the below logic, we will calculate how many length of data has to be transferred. */
    if ((num_bytes_master % BITS_TO_BYTES) != RESET_VALUE)
    {
        num_bytes_master = (num_bytes_master / BITS_TO_BYTES) + 1U;
    }
    else
    {
        num_bytes_master = num_bytes_master / BITS_TO_BYTES;
        g_master_tx_buff[num_bytes_master] = RESET_VALUE;
    }

    /* Input to slave buffer */
    APP_PRINT("Enter text input for slave buffer. Data size should not exceed 64 bytes.\r\n");
    while (BYTES_RECEIVED_ZERO == num_bytes_slave)
    {
        if (APP_CHECK_DATA)
        {
            APP_READ((unsigned char*)g_slave_tx_buff);
            APP_PRINT("\r\nInput data: %s\r\n\r\n", (char *)g_slave_tx_buff);
            num_bytes_slave = (uint32_t)strlen((char *)g_slave_tx_buff);
            if (BYTES_RECEIVED_ZERO == num_bytes_slave)
            {
                APP_PRINT("No input\r\n");
            }
        }
    }

    /* Terminal Emulator user input data 1 byte at a time. SPI transfers the data 4 bytes at a time.
     * With the below logic, we will calculate how many length of data has to be transferred. */
    if ((num_bytes_slave % BITS_TO_BYTES) != RESET_VALUE)
    {
        num_bytes_slave = (num_bytes_slave / BITS_TO_BYTES) + 1U;
    }
    else
    {
        num_bytes_slave = num_bytes_slave / BITS_TO_BYTES;
        g_slave_tx_buff[num_bytes_slave] = RESET_VALUE;
    }

    /* Slave sends data to master and receive data from master */
    err = R_RSPI_WriteRead(&g_spi_slave_ctrl, g_slave_tx_buff, g_slave_rx_buff, num_bytes_master,
                           SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_WriteRead API for slave failed\r\n");
        return err;
    }

    /* Master sends data to slave and receive data from slave */
    err = R_RSPI_WriteRead(&g_spi_master_ctrl, g_master_tx_buff, g_master_rx_buff, num_bytes_master,
                           SPI_BIT_WIDTH_32_BITS);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_WriteRead API for master failed\r\n");
        return err;
    }

    /* Wait until master and slave WriteRead() complete */
    while ((SPI_EVENT_TRANSFER_COMPLETE != g_master_event_flag) || (SPI_EVENT_TRANSFER_COMPLETE != g_slave_event_flag))
    {
        /* Timer out operation if SPI operation fails to complete */
        g_wait_count--;
        if (MIN_COUNT >= g_wait_count)
        {
            err = FSP_ERR_TIMEOUT;
            APP_ERR_PRINT("SPI module failed in Write/Read operation.\r\n");
            error_print();
            return err;
        }
        else if ((SPI_EVENT_TRANSFER_ABORTED == g_master_event_flag) || (SPI_EVENT_TRANSFER_ABORTED == g_slave_event_flag))
        {
            err = FSP_ERR_TRANSFER_ABORTED;
            APP_ERR_PRINT("SPI module failed in Write/Read operation\r\n");
            error_print();
            return err;
        }
        else
        {
            /* Do nothing */
        }
    }
    g_wait_count = MAX_COUNT;

    g_master_event_flag = (spi_event_t) RESET_VALUE;  /* Reset g_master_event_flag flag */
    g_slave_event_flag  = (spi_event_t) RESET_VALUE;  /* Reset g_slave_event_flag flag */

    /* Display master to slave and slave to master transmission */
    APP_PRINT("Master buffer data transmitted to slave\r\n");
    APP_PRINT("Slave buffer data transmitted to master\r\n");

    /* Check whether slave transmitted data is equal to master received data */
    if (BUFF_EQUAL == memcmp(g_slave_tx_buff, g_master_rx_buff, num_bytes_master))
    {
        /* Display master's received data on Terminal Emulator */
        APP_PRINT("Master received data: %s\r\n", (char *)g_master_rx_buff);
    }
    else
    {
        /* Return parity error in case of data mismatch */
        err = FSP_ERR_SPI_PARITY;
        APP_ERR_PRINT("Master received data does not match with slave transmitted data\r\n");
        error_print();
        return err;
    }

    /* Check whether master transmitted data is equal to slave received data */
    if (BUFF_EQUAL == memcmp(g_master_tx_buff, g_slave_rx_buff, num_bytes_master))
    {
        /* Display slave's received data on Terminal Emulator */
        APP_PRINT("Slave received data: %s\r\n", (char *)g_slave_rx_buff);
    }
    else
    {
        /* Return parity error in case of data mismatch */
        err = FSP_ERR_SPI_PARITY;
        APP_ERR_PRINT("Slave received data does not match with master transmitted data.\r\n");
        error_print();
        return err;
    }

    /* Delay of one second for user to verify the output */
    R_BSP_SoftwareDelay(DELAY_ONE_SEC, BSP_DELAY_UNITS_SECONDS);
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function closes both SPI master and slave modules.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  SPI module closed successfully.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful close.
 **********************************************************************************************************************/
fsp_err_t spi_exit_demo(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Close SPI master module */
    err = R_RSPI_Close(&g_spi_master_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Close API for master failed\r\n");
        return err;
    }

    /* Close SPI slave module */
    err = R_RSPI_Close(&g_spi_slave_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("R_RSPI_Close API for slave failed\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief      This is master SPI callback function.
 * @param[in]  p_args   Callback argument.
 * @retval     None.
 **********************************************************************************************************************/
void spi_master_callback(spi_callback_args_t * p_args)
{
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
 * @brief      This is slave SPI callback function.
 * @param[in]  p_args   Callback argument.
 * @retval     None.
 **********************************************************************************************************************/
void spi_slave_callback(spi_callback_args_t * p_args)
{
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event)
    {
        g_slave_event_flag = SPI_EVENT_TRANSFER_COMPLETE;
    }
    else
    {
        g_slave_event_flag = SPI_EVENT_TRANSFER_ABORTED;
    }
}

/***********************************************************************************************************************
 * @brief       This function prints the error message.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void error_print(void)
{
    APP_PRINT("Error in configuration or connection\r\n");
    APP_PRINT("Reset the MPU...\r\n");
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
        /* SPI close failure message */
        APP_ERR_PRINT("** R_RSPI_Close API for master failed **\r\n");
    }

    err = R_RSPI_Close(&g_spi_slave_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* SPI close failure message */
        APP_ERR_PRINT("** R_RSPI_Close API for slave failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_rspi_ep)
 **********************************************************************************************************************/
