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
#include "otfd_sample_cfg.h"
#include "otfd_param.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define KEY_CERT_SIZE                     (0xE0U)
#define CODE_CERT_SIZE                    (0x120U)
#define TOTAL_CERT_SIZE                   (KEY_CERT_SIZE + CODE_CERT_SIZE)

/* Flash app section address definition. */
#if defined(__ICCARM__)
#pragma section="FLASH_APP_RBLOCK"
#pragma section="OTFD_KEY_RBLOCK"
#pragma section="OTFD_IV_RBLOCK"
#define OTFD_FLASH_APP_START_ADDR  (uintptr_t)__section_begin("FLASH_APP_RBLOCK")
#define OTFD_FLASH_KEY_START_ADDR  (uintptr_t)__section_begin("OTFD_KEY_RBLOCK")
#define OTFD_FLASH_IV_START_ADDR   (uintptr_t)__section_begin("OTFD_IV_RBLOCK")

#elif defined(__GNUC__)
extern void * _flash_app_start;
extern void * _otfd_key_start;
extern void * _otfd_iv_start;
#define OTFD_FLASH_APP_START_ADDR  (uint32_t)&_flash_app_start
#define OTFD_FLASH_KEY_START_ADDR  (uint32_t)&_otfd_key_start
#define OTFD_FLASH_IV_START_ADDR   (uint32_t)&_otfd_iv_start
#endif

#define OTFD_FLASH_APP_SECURE_BOOT_IMAGE_START_ADDR  (OTFD_FLASH_IV_START_ADDR - (uintptr_t) TOTAL_CERT_SIZE)

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
extern void LED_blinky(void);
static uint32_t swap_uint32(const uint32_t in_val);
static void handle_error(fsp_err_t err);

#if CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE
static fsp_err_t check_secure_boot_image(uint8_t * const p_data);
#endif /* CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE */

static void qspi_set_Quad_Mode_enable(xspi_qspi_instance_ctrl_t * p_ctrl);
static void qspi_set_Quad_Mode_disable(xspi_qspi_instance_ctrl_t * p_ctrl);

/******************************************************************************
 * @fn       second_application_boot_loader
 * @brief    Select of application type.
 ******************************************************************************/
void second_application_boot_loader(void)
{
    fsp_err_t err;
    
    /* Initialize XSPI0. (OTFD setting is disabled.) */
    err = R_XSPI_QSPI_Open(&g_qspi_non_otfd_ctrl, &g_qspi_non_otfd_cfg);
    handle_error(err);
    
    /* Enter QPI mode. */
    qspi_set_Quad_Mode_enable(&g_qspi_non_otfd_ctrl);
    
#if CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE
    /* Verify the secure boot image. */
    err = check_secure_boot_image((uint8_t *)OTFD_FLASH_APP_SECURE_BOOT_IMAGE_START_ADDR);
    handle_error(err);
#endif /* CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE */
    
    /* Read the key and counter value used for the OTFD function from flash. */
    for (uint8_t i = 0; i < 4; i++)
    {
        /* The IV is endian converted for use with OTFD. */
        g_qspi0_otfd_iv[i] = swap_uint32(g_otfd_iv_flash[i]);
    }
    
    memcpy(&g_qspi0_otfd_key, (void *)g_otfd_key_flash, sizeof(g_qspi0_otfd_key));
    
    /* Exit QPI mode. */
    qspi_set_Quad_Mode_disable(&g_qspi_non_otfd_ctrl);
    
    /* Close XSPI0. (OTFD setting is disabled.) */
    R_XSPI_QSPI_Close(&g_qspi_non_otfd_ctrl);
    
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
 * @fn swap_uint32
 * @brief Converts uint32_t data into a byte order converter.
 *
 * @param[in] in_val      Value to be converted
 *
 * @retval Value after conversion
 ******************************************************************************/
static uint32_t swap_uint32(const uint32_t in_val)
{
    uint32_t  ret_val;
    uint8_t * p_in_val  = (uint8_t *) &in_val;
    uint8_t * p_out_val = (uint8_t *) &ret_val;
    
    p_out_val[0] = p_in_val[3];
    p_out_val[1] = p_in_val[2];
    p_out_val[2] = p_in_val[1];
    p_out_val[3] = p_in_val[0];
    
    return ret_val;
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

#if CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE
/******************************************************************************
 * @fn       check_secure_boot_image
 * @brief    Verify the secure boot image.
 *
 * @param[in] p_data        Secure boot image
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Data error
 ******************************************************************************/
static fsp_err_t check_secure_boot_image(uint8_t * const p_data)
{
    uint8_t * p_key_cert_addr  = p_data;                    ///< Set the start address of Key Certificate
    uint8_t * p_code_cert_addr = p_data + KEY_CERT_SIZE;    ///< Set the start address of Code Certificate
    
    fsp_err_t err;
    
    err = R_RSIP_Open(&g_rsip_ctrl, &g_rsip_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    
    /* Validate the manifest. */
    err = R_RSIP_SB_ManifestVerify(&g_rsip_ctrl, p_key_cert_addr, (uint32_t)KEY_CERT_SIZE,
                                       p_code_cert_addr, (uint32_t)CODE_CERT_SIZE);
        
    R_RSIP_Close(&g_rsip_ctrl);
    
    return err;
}
#endif /* CFG_OTFD_VERIFY_CERT_FOR_FLASH_APP == VERIFY_CERT_ENABLE */

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

