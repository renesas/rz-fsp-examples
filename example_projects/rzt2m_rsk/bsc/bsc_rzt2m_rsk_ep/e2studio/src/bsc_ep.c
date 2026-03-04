/***********************************************************************************************************************
 * File Name    : bsc_ep.c
 * Description  : Contains data structures and functions used in bsc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "bsc_ep.h"

/***********************************************************************************************************************
 * @addtogroup bsc_ep
 * @{
 **********************************************************************************************************************/
/* global variables */
uint16_t g_write_data[DATA_NUM];
uint16_t g_read_data[DATA_NUM];

/*
 * private function declarations
 */
static void nor_Write (uint32_t address, uint16_t data);
static void nor_WriteBuf (uint32_t address, uint16_t * data, uint32_t cnt);
static void nor_StatusGet (uint32_t address, uint32_t * p_status);
static void nor_Erase (uint32_t address, uint32_t byte_count);


/***********************************************************************************************************************
 * @brief       This functions initializes BSC module.
 * @param[IN]   None
 * @retval      FSP_SUCCESS                  Upon successful open of BSC module
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful open
 **********************************************************************************************************************/
fsp_err_t bsc_init(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open BSC module */
    err = R_BSC_Open (&g_bsc_ctrl, &g_bsc_cfg);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* BSC Open failure message */
        APP_ERR_PRINT ("\r\n**Open BSC API FAILED**\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function is to erase a data from external NOR Flash mmemory.
 * @param[IN]   None
 * @retval      None
 **********************************************************************************************************************/
fsp_err_t bsc_erase(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint32_t status;

    /* Erase */
    nor_Erase (CS0_BASE, SECTOR_ERASE);

    do
    {
        (void)nor_StatusGet (CS0_BASE, &status);
    } while (true == status);

    /* Read data */
    volatile uint16_t get_data = *((volatile uint16_t *)CS0_BASE +0x0000);

    /* Verify data erase */
    if (0xFFFF != get_data)
    {
        /* Verify data after erasing failure message */
        APP_ERR_PRINT ("\r\n**ERASE BSC FAILED**\r\n");
        err = FSP_ERR_ERASE_FAILED;
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function writes a byte data to defined sector memory and read data after writing.
 * @param[IN]   None
 * @retval      None
 **********************************************************************************************************************/
fsp_err_t bsc_write(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint32_t  status;

    /* Write data */
    nor_Write (CS0_BASE, 0x1234);
    do
    {
        (void)nor_StatusGet (CS0_BASE, &status);
    } while (true == status);

    /* Read data */
    volatile uint16_t get_data = *((volatile uint16_t *)CS0_BASE +0x0000);

    /* Verify read data after writing */
    if (0x1234 != get_data)
    {
        /* Verify data after writing failure message  */
        APP_ERR_PRINT ("\r\n**BSC WRITE FAILED**\r\n");
        err = FSP_ERR_WRITE_FAILED;
    }

    /* Sector erase */
    nor_Erase (CS0_BASE, SECTOR_ERASE);
    do
    {
        (void)nor_StatusGet (CS0_BASE, &status);
    } while (true == status);

    /* Read data */
    get_data = *((volatile uint16_t *)CS0_BASE + 0x0000);

    /* Verify data erase */
    if (0xFFFF != get_data)
    {
        /* Verify data after erasing failure message */
        APP_ERR_PRINT ("\r\n**BSC ERASE FAILED**\r\n");
        err = FSP_ERR_ERASE_FAILED;
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function writes 256 bytes data to defined sector memory and read data after writing.
 * @param[IN]   None
 * @retval      None
 **********************************************************************************************************************/
fsp_err_t bsc_write_buff(void)
{
    fsp_err_t err = FSP_SUCCESS;
    uint32_t  status;

    for (uint16_t i = 0; i < DATA_NUM; i++)
    {
        g_write_data[i] = i;
    }

    /* Write Buf Data */
    nor_WriteBuf (CS0_BASE, g_write_data, DATA_NUM);
    do
    {
        (void)nor_StatusGet ((CS0_BASE & (~R_NOR_SECT_ADDR_MSK)), &status);
    } while (true == status);

    /* Verify datas after writing */
    for (uint32_t i = 0; i < DATA_NUM; i++)
    {
        g_read_data[i] = *((volatile uint16_t *)CS0_BASE + 0x0000 + i);
        if (g_read_data[i] != g_write_data[i])
        {
            /* Verify data after writing failure message */
            APP_ERR_PRINT ("\r\n**BSC WRITE_BUFFER FAILED**\r\n");
            err = FSP_ERR_WRITE_FAILED;
        }
    }

    /* Sector erase */
    nor_Erase (CS0_BASE, SECTOR_ERASE);
    do
    {
        (void)nor_StatusGet (CS0_BASE, &status);
    } while (true == status);

    /* Verify data erase */
    for (uint32_t i = 0; i < DATA_NUM; i++)
    {
        g_read_data[i] = *((volatile uint16_t *)CS0_BASE + 0x0000 + i);
        if (0xFFFF != g_read_data[i])
        {
            /* Verify data after erasing failure message */
            APP_ERR_PRINT ("\r\n**BSC ERASE FAILED**\r\n");
            err = FSP_ERR_ERASE_FAILED;
        }
    }
    return err;
}

/***********************************************************************************************************************
 * @brief       This function writes a byte data to external NOR flash memory.
 * @param[IN]   address                  The location in the flash device address space to write the data to.
 *              data                     Data to write to a flash memory.
 * @retval      None
 **********************************************************************************************************************/
static void nor_Write(uint32_t address, uint16_t data)
{
    /*-----Enter programming code sequence. -----*/
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = FLASH_CODE1;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD2) = FLASH_CODE2;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = WRITE_CODE;
    __asm volatile("dsb");
    *((volatile uint16_t *)address) = data;
    __asm volatile("dsb");
}

/***********************************************************************************************************************
 * @brief       This function writes number of bytes data to external NOR flash memory..
 * @param[IN]   address                  The location in the flash device address space to write the data to.
 *              data                     Data to write to a flash memory.
 *              cnt                      Number of bytes to write.
 * @retval      None
 **********************************************************************************************************************/
static void nor_WriteBuf(uint32_t address, uint16_t *data, uint32_t cnt)
{
    uint32_t sect_addr = address & (~R_NOR_SECT_ADDR_MSK);

    /*-----Enter programming code sequence. -----*/
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = FLASH_CODE1;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD2) = FLASH_CODE2;
    __asm volatile("dsb");
    *((volatile uint16_t *)sect_addr) = WRITE_BUF_CODE;
    __asm volatile("dsb");
    *((volatile uint16_t *)sect_addr) = (uint16_t)(cnt - 1U);
    __asm volatile("dsb");
    for (uint32_t i = 0U; i < cnt; i++)
    {
        *((volatile uint16_t *)address + i) = data[i];
        __asm volatile("dsb");
    }
    __asm volatile("dsb");
    *((volatile uint16_t *)sect_addr) = WRITE_BUF_CODE2;
    __asm volatile("dsb");
}

/***********************************************************************************************************************
 * @brief       This function gets current satus register.
 * @param[IN]   address                  The location in the flash device address space to get status.
 *              p_status                 pointer to current status of flash memory.
 * @retval      None
 **********************************************************************************************************************/
static void nor_StatusGet(uint32_t address, uint32_t * p_status)
{
    /* Q6bit toggle -> Read data show Status register contents. We can get current status. */
    uint32_t read1 = *(volatile uint16_t*)address;
    __asm volatile("dsb");
    uint32_t read2 = *(volatile uint16_t*)address;
    __asm volatile("dsb");

    *p_status = ((read1 ^ read2) & STATUS_REGISTER_Q6BIT) >> 6U;
}

/***********************************************************************************************************************
 * @brief       This function erases data from address in external NOR flash memory.
 * @param[IN]   address                  The location in the flash device address space to start the erase from.
 *              byte_count               Number of bytes to erase. Set to SECTOR_ERASE or CHIP_ERASE to erase.
 * @retval      None
 **********************************************************************************************************************/
static void nor_Erase(uint32_t address, uint32_t byte_count)
{
    /* For only bus width 16bit */
    /*----- Enter sector erase code sequence. -----*/
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = FLASH_CODE1;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD2) = FLASH_CODE2;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = ERASE_SECTOR_CODE1;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = FLASH_CODE1;
    __asm volatile("dsb");
    *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD2) = FLASH_CODE2;
    __asm volatile("dsb");

    if (SECTOR_ERASE == byte_count)
    {
        *((volatile uint16_t *)address)     = ERASE_SECTOR_CODE2;
        __asm volatile("dsb");
    }

    if (CHIP_ERASE == byte_count)
    {
        *((volatile uint16_t *)CS0_BASE + FLASH_A_SEQ_ADD1) = ERASE_SECTOR_CHIP;
        __asm volatile("dsb");
    }
}

/***********************************************************************************************************************
 * @brief       This function closes opened BSC module before the project ends up in an Error Trap.
 * @param[IN]   None
 * @retval      FSP_SUCCESS                  Upon successful close of BSC module
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful close
 **********************************************************************************************************************/
fsp_err_t bsc_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close BSC module */
    err = R_BSC_Close (&g_bsc_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* BSC Close failure message */
        APP_ERR_PRINT("\r\n**BSC CLOSE API FAILED**\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 * @} (end addtogroup bsc_ep)
 **********************************************************************************************************************/
