/***********************************************************************************************************************
 * File Name    : ssbl.c
 * Description  : Contains data structures and functions used in ssbl.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "hal_data.h"
#include "ssbl.h"
#include "otfd_param.h"

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
extern void LED_blinky(void);

static void handle_error(fsp_err_t err);
static void qspi_set_Quad_Mode_enable(xspi_qspi_instance_ctrl_t * p_ctrl);
static void qspi_set_Quad_Mode_disable(xspi_qspi_instance_ctrl_t * p_ctrl);

/******************************************************************************
 * @fn       second_application_boot_loader
 * @brief    Select of application type.
 ******************************************************************************/
void second_application_boot_loader(void)
{
    fsp_err_t err;
    
    /* ------------------------------------------------------------- */
    /* Generate a inject key for OTFD. */
    /* In this sample, the generated key is stored in RAM and used to initialize OTFD. */
    /* Note that the generation of wrap keys should be performed only during the manufacturing process of the system, and it is not */
    /* recommended to perform this within the loader program or application program, as it may compromise the security of the system. */
    err = generate_otfd_inject_key();
    handle_error(err);
    /* ------------------------------------------------------------- */
    
    /* Initialize XSPI0 and configure OTFD. */
    err = R_XSPI_QSPI_Open(&g_qspi0_ctrl, &g_qspi0_cfg);
    handle_error(err);
    
    /* Enter QPI mode. */
    qspi_set_Quad_Mode_enable(&g_qspi0_ctrl);
    
    /* Delete cache data. */
    R_BSP_CacheInvalidateAll();
    
    /* Enter XiP mode. */
    err = R_XSPI_QSPI_XipEnter(&g_qspi0_ctrl);
    handle_error(err);
    
    /* Blink 10 times. (Executed from serial Flash) */
    LED_blinky();
    
    /* Exit XiP mode. */
    err = R_XSPI_QSPI_XipExit(&g_qspi0_ctrl);
    handle_error(err);
    
    /* Exit QPI mode. */
    qspi_set_Quad_Mode_disable(&g_qspi0_ctrl);
    
    while(1);
}

/******************************************************************************
 * @fn handle_module_error
 * @brief Module error handler.
 *
 * @param[in]  fsp_err        FSP module return code.
 ******************************************************************************/
static void handle_error(fsp_err_t err)
{
    if (FSP_SUCCESS != err)
    {
        /* Error. */
        while (1);
    }
}

#if defined(BSP_MCU_GROUP_RZT2ME)
/******************************************************************************
 * @fn       qspi_set_Quad_Mode_enable
 * @brief    Enables xSPI quad mode.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 ******************************************************************************/
static void qspi_set_Quad_Mode_enable(xspi_qspi_instance_ctrl_t * p_ctrl)
{
    /* For following settings, temporary set protocol 1S-1S-1S */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);
    
    /*--------------- Read status register(RDSR) (05h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDSR = {0};
    direct_command_RDSR.command        = 0x05U;
    direct_command_RDSR.command_length = 1U;
    direct_command_RDSR.data_length    = 1U;
    
    fsp_err_t err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDSR, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    uint32_t read_status = direct_command_RDSR.data;
    FSP_PARAMETER_NOT_USED(read_status);
    /*----------------------------------------------------------*/
    
    /*---------------- Read configure register(RDCR) (15h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDCR = {0};
    direct_command_RDCR.command        = 0x15U;
    direct_command_RDCR.command_length = 1U;
    direct_command_RDCR.data_length    = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDCR, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    uint32_t read_configure = direct_command_RDCR.data;
    FSP_PARAMETER_NOT_USED(read_configure);
    /*--------------------------------------------------------------*/
    
    /*----------------- Enter 4byte address mode(EN4B) (0xB7) ----------------*/
    spi_flash_direct_transfer_t direct_command_EN4B = {0};
    direct_command_EN4B.command        = 0xB7U;
    direct_command_EN4B.command_length = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_EN4B, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);
    /*--------------------------------------------------------------*/
    
    /*--------------- Enable QPI mode(EQIO) (35h)---------------*/
    spi_flash_direct_transfer_t direct_command_EQIO = {0};
    direct_command_EQIO.command        = 0x35U;
    direct_command_EQIO.command_length = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_EQIO, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);
    /*----------------------------------------------------------*/
    
    /* 4S-4S-4S protocol settings. */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_4S_4S_4S);
    
    /*--------------- Read status register(RDSR) (05h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDSR_2 = {0};
    direct_command_RDSR_2.command        = 0x05U;
    direct_command_RDSR_2.command_length = 1U;
    direct_command_RDSR_2.data_length    = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDSR_2, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    read_status = direct_command_RDSR_2.data;
    /*----------------------------------------------------------*/
    
    /*---------------- Read configure register(RDCR) (15h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDCR_2 = {0};
    direct_command_RDCR_2.command        = 0x15U;
    direct_command_RDCR_2.command_length = 1U;
    direct_command_RDCR_2.data_length    = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDCR_2, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    read_configure = direct_command_RDCR_2.data;
    /*--------------------------------------------------------------*/
}

#endif /* defined(BSP_MCU_GROUP_RZT2ME)*/

#if defined(BSP_MCU_GROUP_RZT2N)
/******************************************************************************
 * @fn       qspi_set_Quad_Mode_enable
 * @brief    Enables xSPI quad mode.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 ******************************************************************************/
static void qspi_set_Quad_Mode_enable(xspi_qspi_instance_ctrl_t * p_ctrl)
{
    /* For following settings, temporary set protocol 1S-1S-1S */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);
    
    /*---------------- Read status register2(RDSR2) (35h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDSR2 = {0};
    direct_command_RDSR2.command        = 0x35U;
    direct_command_RDSR2.command_length = 1U;
    direct_command_RDSR2.data_length    = 1U;
    
    fsp_err_t err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDSR2, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    uint32_t read_status2 = direct_command_RDSR2.data;
    
    /*---------------- Write enable (06h)----------------*/
    spi_flash_direct_transfer_t qspi_test_direct_transfer_write_enable = {0};
    qspi_test_direct_transfer_write_enable.command        = 0x06;
    qspi_test_direct_transfer_write_enable.command_length = 1U;
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &qspi_test_direct_transfer_write_enable, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    
    /*---------------- Write Status Register-2 (31h) (set QE bit 1) ----------------*/
    spi_flash_direct_transfer_t direct_transfer_register_write = {0};
    direct_transfer_register_write.command              = 0x31;
    direct_transfer_register_write.data                 = 0x02 | read_status2;
    direct_transfer_register_write.command_length       = 1U;
    direct_transfer_register_write.data_length          = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_transfer_register_write, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);
    
    /* Wait for status register to update. */
    spi_flash_status_t status_write;
    do
    {
        (void) R_XSPI_QSPI_StatusGet(p_ctrl, &status_write);
    } while (true == status_write.write_in_progress);
    
    /* 1S-4S-4S protocol settings. */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_4S_4S);
}
#endif /* defined(BSP_MCU_GROUP_RZT2N) */

#if defined(BSP_MCU_GROUP_RZT2ME)
/******************************************************************************
 * @fn       qspi_set_Quad_Mode_disable
 * @brief    Disable xSPI quad mode.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 ******************************************************************************/
static void qspi_set_Quad_Mode_disable(xspi_qspi_instance_ctrl_t * p_ctrl)
{
    /*--------------- Reset QPI(RSTQIO) (F5h)---------------*/
    spi_flash_direct_transfer_t direct_command_RSTQIO = {0};
    direct_command_RSTQIO.command        = 0xF5U;
    direct_command_RSTQIO.command_length = 1U;
    
    fsp_err_t err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RSTQIO, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);
    /*----------------------------------------------------------*/
    
    /* 1S-1S-1S protocol settings. */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);
    
    /*--------------- Read status register (05h) ---------------*/
    spi_flash_direct_transfer_t direct_command = {0};
    direct_command.command        = 0x05U;
    direct_command.command_length = 1U;
    direct_command.data_length    = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    /*----------------------------------------------------------*/
    
    /*---------------- Read configure register (15h) ---------------*/
    direct_command.command        = 0x15U;
    direct_command.command_length = 1U;
    direct_command.data_length    = 1U;
    
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    /*--------------------------------------------------------------*/
}
#endif /* defined(BSP_MCU_GROUP_RZT2ME)*/

#if defined(BSP_MCU_GROUP_RZT2N)
/******************************************************************************
 * @fn       qspi_set_Quad_Mode_disable
 * @brief    Disable xSPI quad mode.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 ******************************************************************************/
static void qspi_set_Quad_Mode_disable(xspi_qspi_instance_ctrl_t * p_ctrl)
{
    /* 1S-1S-1S protocol settings. */
    R_XSPI_QSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);
    
    /*---------------- Read status register2(RDSR2) (35h) ---------------*/
    spi_flash_direct_transfer_t direct_command_RDSR2 = {0};
    direct_command_RDSR2.command        = 0x35U;
    direct_command_RDSR2.command_length = 1U;
    direct_command_RDSR2.data_length    = 1U;
    fsp_err_t err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_RDSR2, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
    handle_error(err);
    uint32_t read_status = direct_command_RDSR2.data;
    
    /*---------------- Write enable (06h) ----------------*/
    spi_flash_direct_transfer_t qspi_test_direct_transfer_write_enable = {0};
    qspi_test_direct_transfer_write_enable.command        = 0x06;
    qspi_test_direct_transfer_write_enable.command_length = 1U;
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &qspi_test_direct_transfer_write_enable, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    
    /*---------------- Write Status Register-2 (31h) (set QE bit 0) ----------------*/
    spi_flash_direct_transfer_t direct_transfer_register_write = {0};
    direct_transfer_register_write.command              = 0x31;
    direct_transfer_register_write.data                 = read_status & 0xfd;
    direct_transfer_register_write.command_length       = 1U;
    direct_transfer_register_write.data_length          = 1U;
    err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_transfer_register_write, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    handle_error(err);
    
    /* Wait for status register to update. */
    spi_flash_status_t status_write;
    do
    {
        (void) R_XSPI_QSPI_StatusGet(p_ctrl, &status_write);
    } while (true == status_write.write_in_progress);
}
#endif /* defined(BSP_MCU_GROUP_RZT2N)*/

