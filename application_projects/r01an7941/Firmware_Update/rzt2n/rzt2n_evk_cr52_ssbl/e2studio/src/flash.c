/***********************************************************************************************************************
 * Copyright [2020-2024] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "hal_data.h"
#include "flash.h"

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
#if (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1)
static flash_err_t qspi_erase_flash (xspi_qspi_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len);
static flash_err_t qspi_write_flash (xspi_qspi_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len);
static fsp_err_t qspi_set_4byte_address_mode (xspi_qspi_instance_ctrl_t * const p_ctrl);
#endif /* (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1) */

#if (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1)
static flash_err_t ospi_erase_flash (xspi_ospi_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len);
static flash_err_t ospi_write_flash (xspi_ospi_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len);
static fsp_err_t ospi_set_spi_mode_enable (xspi_ospi_instance_ctrl_t * const p_ctrl);
#endif /* (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1) */

#if (1 == USE_FLASH_NOR_CS0)
static flash_err_t nor_erase_flash (bsc_nor_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len);
static flash_err_t nor_write_flash (bsc_nor_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len);
#endif /* (1 == USE_FLASH_NOR_CS0) */

#if (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1)
/******************************************************************************
 * @fn open_qspi_flash
 * @brief Open QSPI flash.
 *
 * @param[in]  unit_num        xSPI unit number
 *
 * @retval FLASH_SUCCESS       Success
 * @retval FLASH_ERR_PARAMETER Parameter error
 * @retval FLASH_ERR_MODULE    Module error
 ******************************************************************************/
flash_err_t open_qspi_flash(xspi_unit_number_t unit_num)
{
    bool unit_is_invalid = false;
    fsp_err_t fsp_err;
    xspi_qspi_instance_ctrl_t * p_qspi_ctrl;
    spi_flash_cfg_t const * p_qspi_cfg;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_QSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set QSPI0 setting information. */
            p_qspi_ctrl = &g_qspi0_ctrl;
            p_qspi_cfg  = &g_qspi0_cfg;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI0) */
#if (1 == USE_FLASH_QSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set QSPI1 setting information. */
            p_qspi_ctrl = &g_qspi1_ctrl;
            p_qspi_cfg  = &g_qspi1_cfg;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Initializes the QSPI module. */
    fsp_err = R_XSPI_QSPI_Open(p_qspi_ctrl, p_qspi_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return FLASH_ERR_MODULE;
    }
    
    /* Check address bytes. */
    if (SPI_FLASH_ADDRESS_BYTES_4 == p_qspi_cfg->address_bytes)
    {
        /* Set the address to 4-byte mode. (If the flash area is 16MB or more) */
        fsp_err = qspi_set_4byte_address_mode(p_qspi_ctrl);
        if (FSP_SUCCESS != fsp_err)
        {
            /* Close. */
            R_XSPI_QSPI_Close(p_qspi_ctrl);
            return FLASH_ERR_MODULE;
        }
    }
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn close_qspi_flash
 * @brief Close QSPI flash.
 *
 * @param[in]  unit_num        xSPI unit number
 *
 * @retval FLASH_SUCCESS       Success
 * @retval FLASH_ERR_PARAMETER Parameter error
 ******************************************************************************/
flash_err_t close_qspi_flash(xspi_unit_number_t unit_num)
{
    bool unit_is_invalid = false;
    xspi_qspi_instance_ctrl_t * p_qspi_ctrl;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_QSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set QSPI0 setting information. */
            p_qspi_ctrl = &g_qspi0_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI0) */
#if (1 == USE_FLASH_QSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set QSPI1 setting information. */
            p_qspi_ctrl = &g_qspi1_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Close. */
    R_XSPI_QSPI_Close(p_qspi_ctrl);
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn write_to_qspi_area
 * @brief Write to the QSPI area.
 *
 * @param[in]  addr           Write address
 * @param[in]  p_data         Write data
 * @param[in]  data_len       Write data length
 * @param[in]  unit_num       xSPI unit number
 *
 * @retval FLASH_SUCCESS            Success
 * @retval FLASH_ERR_PARAMETER      Parameter error
 * @retval FLASH_ERR_MODULE         Module error
 * @retval FLASH_ERR_VERIFY_FAILED  Verify error
 ******************************************************************************/
flash_err_t write_to_qspi_area(uint32_t addr, uint8_t * const p_data, uint32_t data_len, xspi_unit_number_t unit_num)
{
    uint32_t    write_complete_len = 0U;    // Data length for which flash writing has already been completed
    uint32_t    sector_write_len   = 0U;    // Data length to be written in one flash write operation
    uint8_t   * p_buf              = (uint8_t *)BACKUP_BUFFER_ADDR;
    bool        unit_is_invalid    = false;
    flash_err_t flash_err          = FLASH_SUCCESS;
    xspi_qspi_instance_ctrl_t * p_qspi_ctrl;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_QSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set QSPI0 setting information. */
            p_qspi_ctrl = &g_qspi0_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI0) */
#if (1 == USE_FLASH_QSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set QSPI1 setting information. */
            p_qspi_ctrl = &g_qspi1_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_QSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Refer to the mirror area. */
    addr = __get_qspi_flash_mirror_addr(addr);
    
    /* Gets the maximum write size for the first sector of the write-to address. */
    sector_write_len = XSPI_SECTOR_SIZE - (addr % XSPI_SECTOR_SIZE);
    
    /* Erase processing loop. (sector unit)  */
    while (write_complete_len < data_len)
    {
        /* Check the write size for the sector. */
        if (sector_write_len > (data_len - write_complete_len))
        {
            sector_write_len = data_len - write_complete_len;
        }
        
        uint32_t src_addr  = (uint32_t)p_data + write_complete_len;
        uint32_t dest_addr = addr + write_complete_len;
        
        /* Read the data of the target sector into the buffer. */
        uint32_t read_addr = (dest_addr & XSPI_ERASE_MASK);
        memcpy(p_buf, (void*)read_addr, XSPI_SECTOR_SIZE);
        
        /* Erase sector. */
        flash_err = qspi_erase_flash(p_qspi_ctrl, (dest_addr & XSPI_ERASE_MASK), XSPI_SECTOR_SIZE);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        /* Update the data in the buffer. */
        uint32_t offset = dest_addr - (dest_addr & XSPI_ERASE_MASK);
        memcpy((p_buf + offset), (void *)src_addr, sector_write_len);
        
        /* Write to the sector where the erase was performed. */
        flash_err = qspi_write_flash(p_qspi_ctrl, (uint32_t)p_buf, (dest_addr & XSPI_ERASE_MASK), XSPI_SECTOR_SIZE);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        write_complete_len += sector_write_len;
        sector_write_len    = XSPI_SECTOR_SIZE;
    }
    
    /* Check if writing is complete. */
    if (FLASH_SUCCESS == flash_err)
    {
        /* Verify Write data. */
        int32_t write_check = memcmp((void*)addr, p_data, data_len);
        
        if (0 != write_check)
        {
            flash_err = FLASH_ERR_VERIFY_FAILED;
        }
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn read_to_qspi_area
 * @brief Read to the QSPI area.
 *
 * @param[in]  addr           Read address
 * @param[out] p_data         Read data buffer
 * @param[in]  data_len       Read data length
 ******************************************************************************/
void read_to_qspi_area(uint32_t addr, uint8_t * p_data, uint32_t data_len)
{
    /* Refer to the mirror area. */
    addr = __get_qspi_flash_mirror_addr(addr);
    
    /* Copy the data to be read to the buffer. */
    memcpy(p_data, (void*)addr, data_len);
    
    return;
}
#endif /* (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1) */

#if (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1)
/******************************************************************************
 * @fn open_ospi_flash
 * @brief Open OSPI flash.
 *
 * @param[in]  unit_num        xSPI unit number
 *
 * @retval FLASH_SUCCESS       Success
 * @retval FLASH_ERR_PARAMETER Parameter error
 * @retval FLASH_ERR_MODULE    Module error
 ******************************************************************************/
flash_err_t open_ospi_flash(xspi_unit_number_t unit_num)
{
    bool unit_is_invalid = false;
    fsp_err_t fsp_err;
    xspi_ospi_instance_ctrl_t * p_ospi_ctrl;
    spi_flash_cfg_t const * p_ospi_cfg;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_OSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set OSPI0 setting information. */
            p_ospi_ctrl = &g_ospi0_ctrl;
            p_ospi_cfg  = &g_ospi0_cfg;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI0) */
#if (1 == USE_FLASH_OSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set OSPI1 setting information. */
            p_ospi_ctrl = &g_ospi1_ctrl;
            p_ospi_cfg  = &g_ospi1_cfg;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Initializes the OSPI module. */
    fsp_err = R_XSPI_OSPI_Open(p_ospi_ctrl, p_ospi_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return FLASH_ERR_MODULE;
    }
    
    /* Set SPI mode enable. */
    fsp_err = ospi_set_spi_mode_enable(p_ospi_ctrl);
    if (FSP_SUCCESS != fsp_err)
    {
        /* OSPI close. */
        R_XSPI_OSPI_Close(p_ospi_ctrl);
        return FLASH_ERR_MODULE;
    }
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn close_ospi_flash
 * @brief Close OSPI flash.
 *
 * @param[in]  unit_num        xSPI unit number
 *
 * @retval FLASH_SUCCESS       Success
 * @retval FLASH_ERR_PARAMETER Parameter error
 ******************************************************************************/
flash_err_t close_ospi_flash(xspi_unit_number_t unit_num)
{
    bool unit_is_invalid = false;
    xspi_ospi_instance_ctrl_t * p_ospi_ctrl;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_OSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set OSPI0 setting information. */
            p_ospi_ctrl = &g_ospi0_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI0) */
#if (1 == USE_FLASH_OSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set OSPI1 setting information. */
            p_ospi_ctrl = &g_ospi1_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Close. */
    R_XSPI_OSPI_Close(p_ospi_ctrl);
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn write_to_ospi_area
 * @brief Write to the OSPI area.
 *
 * @param[in]  addr           Write address
 * @param[in]  p_data         Write data
 * @param[in]  data_len       Write data length
 * @param[in]  unit_num       XSPI unit number
 *
 * @retval FLASH_SUCCESS            Success
 * @retval FLASH_ERR_PARAMETER      Parameter error
 * @retval FLASH_ERR_MODULE         Module error
 * @retval FLASH_ERR_VERIFY_FAILED  Verify error
 ******************************************************************************/
flash_err_t write_to_ospi_area(uint32_t addr, uint8_t * const p_data, uint32_t data_len, xspi_unit_number_t unit_num)
{
    uint32_t    write_complete_len = 0U;    // Data length for which flash writing has already been completed
    uint32_t    sector_write_len   = 0U;    // Data length to be written in one flash write operation
    uint8_t   * p_buf              = (uint8_t *)BACKUP_BUFFER_ADDR;
    bool        unit_is_invalid    = false;
    flash_err_t flash_err          = FLASH_SUCCESS;
    xspi_ospi_instance_ctrl_t * p_ospi_ctrl;
    
    /* Check xspi unit number. */
    switch (unit_num)
    {
#if (1 == USE_FLASH_OSPI_XSPI0)
        case XSPI_UNIT_NUM_0:
            /* Set OSPI0 setting information. */
            p_ospi_ctrl = &g_ospi0_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI0) */
#if (1 == USE_FLASH_OSPI_XSPI1)
        case XSPI_UNIT_NUM_1:
            /* Set OSPI1 setting information. */
            p_ospi_ctrl = &g_ospi1_ctrl;
            break;
            
#endif /* (1 == USE_FLASH_OSPI_XSPI1) */
        default:
            /* Not supported. */
            unit_is_invalid = true;
            break;
    }
    
    if (true == unit_is_invalid)
    {
        return FLASH_ERR_PARAMETER;
    }
    
    /* Refer to the mirror area. */
    addr = __get_ospi_flash_mirror_addr(addr);
    
    /* Gets the maximum write size for the first sector of the write-to address. */
    sector_write_len = XSPI_SECTOR_SIZE - (addr % XSPI_SECTOR_SIZE);
    
    /* Erase processing loop. (sector unit)  */
    while (write_complete_len < data_len)
    {
        /* Check the write size for the sector. */
        if (sector_write_len > (data_len - write_complete_len))
        {
            sector_write_len = data_len - write_complete_len;
        }
        
        uint32_t src_addr = (uint32_t)p_data + write_complete_len;
        uint32_t dest_addr = addr + write_complete_len;
        
        /* Read the data of the target sector into the buffer. */
        uint32_t read_addr = (dest_addr & XSPI_ERASE_MASK);
        memcpy(p_buf, (void*)read_addr, XSPI_SECTOR_SIZE);
        
        /* Erase sector. */
        flash_err = ospi_erase_flash(p_ospi_ctrl, (dest_addr & XSPI_ERASE_MASK), XSPI_SECTOR_SIZE);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        /* Update the data in the buffer. */
        uint32_t offset = dest_addr - (dest_addr & XSPI_ERASE_MASK);
        memcpy((p_buf + offset), (void *)src_addr, sector_write_len);
        
        /* Write to the sector where the erase was performed. */
        flash_err = ospi_write_flash(p_ospi_ctrl, (uint32_t)p_buf, (dest_addr & XSPI_ERASE_MASK), XSPI_SECTOR_SIZE);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        write_complete_len += sector_write_len;
        sector_write_len    = XSPI_SECTOR_SIZE;
    }
    
    /* Check if writing is complete. */
    if (FLASH_SUCCESS == flash_err)
    {
        /* Verify Write data. */
        int32_t write_check = memcmp((void*)addr, p_data, data_len);
        
        if (0 != write_check)
        {
            flash_err = FLASH_ERR_VERIFY_FAILED;
        }
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn read_to_ospi_area
 * @brief Read to the OSPI area.
 *
 * @param[in]  addr           Read address
 * @param[out] p_data         Read data buffer
 * @param[in]  data_len       Read data length
 ******************************************************************************/
void read_to_ospi_area(uint32_t addr, uint8_t * p_data, uint32_t data_len)
{
    /* Refer to the mirror area. */
    addr = __get_ospi_flash_mirror_addr(addr);
    
    /* Copy the data to be read to the buffer. */
    memcpy(p_data, (void*)addr, data_len);
    
    return;
}
#endif /* (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1) */

#if (1 == USE_FLASH_NOR_CS0)
/******************************************************************************
 * @fn open_nor_flash
 * @brief Open nor flash.
 *
 * @retval FLASH_SUCCESS       Success
 * @retval FLASH_ERR_MODULE    Module error
 ******************************************************************************/
flash_err_t open_nor_flash(void)
{
    fsp_err_t fsp_err;
    
    /* Initializes the BSC_NOR module. */
    fsp_err = R_BSC_NOR_Open(&g_nor_flash0_ctrl, &g_nor_flash0_cfg);
    if (FSP_SUCCESS != fsp_err)
    {
        return FLASH_ERR_MODULE;
    }
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn close_nor_flash
 * @brief Close nor flash.
 *
 * @retval FLASH_SUCCESS       Success
 ******************************************************************************/
flash_err_t close_nor_flash(void)
{
    /* Close. */
    R_BSC_NOR_Close(&g_nor_flash0_ctrl);
    
    return FLASH_SUCCESS;
}

/******************************************************************************
 * @fn write_to_nor_area
 * @brief Write to the NOR area.
 *
 * @param[in]  addr           Write address
 * @param[in]  p_data         Write data
 * @param[in]  data_len       Write data length
 *
 * @retval FLASH_SUCCESS            Success
 * @retval FLASH_ERR_MODULE         Module error
 * @retval FLASH_ERR_VERIFY_FAILED  Verify error
 ******************************************************************************/
flash_err_t write_to_nor_area(uint32_t addr, uint8_t * const p_data, uint32_t data_len)
{
    uint32_t    write_complete_len = 0U;    // Data length for which flash writing has already been completed
    uint32_t    sector_write_len   = 0U;    // Data length to be written in one flash write operation
    uint8_t   * p_buf              = (uint8_t *)BACKUP_BUFFER_ADDR;
    flash_err_t flash_err          = FLASH_SUCCESS;
    
    /* Refer to the mirror area. */
    addr = __get_nor_flash_mirror_addr(addr);
    
    /* Gets the maximum write size for the first sector of the write-to address. */
    sector_write_len = NOR_SECTOR_SIZE - (addr % NOR_SECTOR_SIZE);
    
    /* Erase processing loop. (sector unit)  */
    while (write_complete_len < data_len)
    {
        /* Check the write size for the sector. */
        if (sector_write_len > (data_len - write_complete_len))
        {
            sector_write_len = data_len - write_complete_len;
        }
        
        uint32_t src_addr  = (uint32_t)p_data + write_complete_len;
        uint32_t dest_addr = addr + write_complete_len;
        
        /* Read the data of the target sector into the buffer. */
        memcpy(p_buf, (void*)(dest_addr & NOR_ERASE_MASK), NOR_SECTOR_SIZE);
        
        /* Erase sector. */
        flash_err = nor_erase_flash(&g_nor_flash0_ctrl, (dest_addr & NOR_ERASE_MASK), NOR_SECTOR_SIZE);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        /* Update the data in the buffer. */
        uint32_t offset = dest_addr - (dest_addr & NOR_ERASE_MASK);
        memcpy((p_buf + offset), (void *)src_addr, sector_write_len);
        
        uint32_t idx = 0U;
        
        /* Writes to the sector in 0x100 byte increments. */
        while (idx < NOR_SECTOR_SIZE)
        {
            /* Write to the sector where the erase was performed. */
            flash_err = nor_write_flash(&g_nor_flash0_ctrl, (uint32_t)p_buf + idx, (dest_addr & NOR_ERASE_MASK) + idx, NOR_WRITE_SIZE);
            if (FLASH_SUCCESS != flash_err)
            {
                break;
            }
            
            idx += NOR_WRITE_SIZE;
        }
        
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        write_complete_len += sector_write_len;
        sector_write_len    = NOR_SECTOR_SIZE;
    }
    
    /* Check if writing is complete. */
    if (FLASH_SUCCESS == flash_err)
    {
        /* Verify Write data. (Specify the mirror area address) */
        int32_t write_check = memcmp((void*)addr, p_data, data_len);
        
        if (0 != write_check)
        {
            flash_err = FLASH_ERR_VERIFY_FAILED;
        }
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn read_to_nor_area
 * @brief Read to the NOR area.
 *
 * @param[in]  addr           Read address
 * @param[out] p_data         Read data buffer
 * @param[in]  data_len       Read data length
 ******************************************************************************/
void read_to_nor_area(uint32_t addr, uint8_t * p_data, uint32_t data_len)
{
    /* Refer to the mirror area. */
    addr = __get_nor_flash_mirror_addr(addr);
    
    /* Copy the data to be read to the buffer. */
    memcpy(p_data, (void*)addr, data_len);
    
    return;
}
#endif /* (1 == USE_FLASH_NOR_CS0) */

#if (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1)
/******************************************************************************
 * @fn qspi_erase_flash
 * @brief QSPI Erases the specified area.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  addr           Start address to erase
 * @param[in]  len            Length to erase
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t qspi_erase_flash(xspi_qspi_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len)
{
    flash_err_t flash_err = FLASH_SUCCESS;
    fsp_err_t   fsp_err;
    
    /* Erase flash. */
    fsp_err = R_XSPI_QSPI_Erase(p_ctrl, (uint8_t *)addr, len);
    if (FSP_SUCCESS != fsp_err)
    {
        flash_err = FLASH_ERR_MODULE;
    }
    else
    {
        /* Wait for status register to update. */
        spi_flash_status_t status_erase;
        do
        {
            (void)R_XSPI_QSPI_StatusGet(p_ctrl, &status_erase);
        } while (true == status_erase.write_in_progress);
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn qspi_write_flash
 * @brief QSPI Writes data to the specified address.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  src            Source address
 * @param[in]  dest           Dest address
 * @param[in]  len            Write data length
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t qspi_write_flash(xspi_qspi_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len)
{
    flash_err_t flash_err      = FLASH_SUCCESS;
    uint32_t    written_len    = 0U;
    uint32_t    onetime_write_len;
    fsp_err_t   fsp_err;
    uint8_t     src_data[XSPI_WRITE_SIZE];
    
    onetime_write_len = XSPI_WRITE_SIZE - (dest % XSPI_WRITE_SIZE);
    
    /* Write processing loop. (64-byte unit) */
    /* Since there is a limit of writing in 64-byte units within one page, */
    /* "0xFF" is written in the front and back area depending on the specified address and data size. */
    while (written_len < len)
    {
        if (onetime_write_len > (len - written_len))
        {
            onetime_write_len = len - written_len;
        }
        
        uint8_t src_offset = 0U;
        
        /* Get the offset value for the first write. (Page crossing prevention) */
        if (0U == written_len)
        {
            src_offset = dest % XSPI_WRITE_SIZE;
        }
        
        /* Clear the data buffer of the write source. */
        memset(src_data, 0xFFU, XSPI_WRITE_SIZE);
        
        /* Stores 64 bytes of data to be written at one time in a buffer. */
        for (uint32_t i = 0U; i < onetime_write_len; i++)
        {
            src_data[src_offset + i] = *(uint8_t *)(src + written_len + i);
        }
        
        /* Write data. */
        fsp_err = R_XSPI_QSPI_Write(p_ctrl, src_data, (uint8_t *)((dest + written_len) & XSPI_WRITE_MASK), XSPI_WRITE_SIZE);
        if (FSP_SUCCESS != fsp_err)
        {
            flash_err = FLASH_ERR_MODULE;
            break;
        }
        
        /* Wait for status register to update. */
        FSP_HARDWARE_REGISTER_WAIT(p_ctrl->p_reg->COMSTT_b.WRBUFNE, 0);
        FSP_HARDWARE_REGISTER_WAIT(p_ctrl->p_reg->COMSTT_b.MEMACC, 0);
        
        spi_flash_status_t status_write;
        do
        {
            (void)R_XSPI_QSPI_StatusGet(p_ctrl, &status_write);
        } while (true == status_write.write_in_progress);
        
        written_len      += onetime_write_len;
        onetime_write_len = XSPI_WRITE_SIZE;
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn qspi_set_4byte_address_mode
 * @brief QSPI set 4byte address mode.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 *
 * @retval FSP_SUCCESS(0)  Success
 * @retval Other than 0    Failure
 ******************************************************************************/
static fsp_err_t qspi_set_4byte_address_mode(xspi_qspi_instance_ctrl_t * const p_ctrl)
{
    fsp_err_t fsp_err;
    
    /*----------------- Enter 4byte address mode(EN4B) (0xB7) ----------------*/
    spi_flash_direct_transfer_t direct_command_EN4B = {0};
    direct_command_EN4B.command        = 0xB7U;
    direct_command_EN4B.command_length = 1U;
    
    fsp_err = R_XSPI_QSPI_DirectTransfer(p_ctrl, &direct_command_EN4B, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
    /*--------------------------------------------------------------*/
    
    return fsp_err;
}
#endif /* (1 == USE_FLASH_QSPI_XSPI0) || (1 == USE_FLASH_QSPI_XSPI1) */

#if (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1)
/******************************************************************************
 * @fn ospi_erase_flash
 * @brief OSPI Erases the specified area.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  addr           Start address to erase
 * @param[in]  len            Length to erase
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t ospi_erase_flash(xspi_ospi_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len)
{
    flash_err_t flash_err = FLASH_SUCCESS;
    fsp_err_t   fsp_err;
    
    /* Erase flash. */
    fsp_err = R_XSPI_OSPI_Erase(p_ctrl, (uint8_t *)addr, len);
    if (FSP_SUCCESS != fsp_err)
    {
        flash_err = FLASH_ERR_MODULE;
    }
    else
    {
        /* Wait for status register to update. */
        spi_flash_status_t status_erase;
        do
        {
            (void)R_XSPI_OSPI_StatusGet(p_ctrl, &status_erase);
        } while (true == status_erase.write_in_progress);
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn ospi_write_flash
 * @brief OSPI Writes data to the specified address.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  src            Source address
 * @param[in]  dest           Dest address
 * @param[in]  len            Write data length
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t ospi_write_flash(xspi_ospi_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len)
{
    flash_err_t flash_err         = FLASH_SUCCESS;
    uint32_t    written_len       = 0U;
    uint32_t    onetime_write_len;
    fsp_err_t   fsp_err;
    uint8_t     src_data[XSPI_WRITE_SIZE];
    
    onetime_write_len = XSPI_WRITE_SIZE - (dest % XSPI_WRITE_SIZE);
    
    /* Write processing loop. (64-byte unit) */
    /* Since there is a limit of writing in 64-byte units within one page, */
    /* "0xFF" is written in the front and back area depending on the specified address and data size. */
    while (written_len < len)
    {
        if (onetime_write_len > (len - written_len))
        {
            onetime_write_len = len - written_len;
        }
        
        uint8_t src_offset = 0U;
        
        /* Get the offset value for the first write. (Page crossing prevention) */
        if (0U == written_len)
        {
            src_offset = dest % XSPI_WRITE_SIZE;
        }
        
        /* Clear the data buffer of the write source. */
        memset(src_data, 0xFFU, XSPI_WRITE_SIZE);
        
        /* Stores 64 bytes of data to be written at one time in a buffer. */
        for (uint32_t i = 0U; i < onetime_write_len; i++)
        {
            src_data[src_offset + i] = *(uint8_t *)(src + written_len + i);
        }
        
        /* Write data. */
        fsp_err = R_XSPI_OSPI_Write(p_ctrl, src_data, (uint8_t *)((dest + written_len) & XSPI_WRITE_MASK), XSPI_WRITE_SIZE);
        if (FSP_SUCCESS != fsp_err)
        {
            flash_err = FLASH_ERR_MODULE;
            break;
        }
        
        /* Wait for status register to update. */
        FSP_HARDWARE_REGISTER_WAIT(p_ctrl->p_reg->COMSTT_b.WRBUFNE, 0);
        FSP_HARDWARE_REGISTER_WAIT(p_ctrl->p_reg->COMSTT_b.MEMACC, 0);
        
        spi_flash_status_t status_write;
        do
        {
            (void)R_XSPI_OSPI_StatusGet(p_ctrl, &status_write);
        } while (true == status_write.write_in_progress);
        
        written_len      += onetime_write_len;
        onetime_write_len = XSPI_WRITE_SIZE;
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn ospi_set_spi_mode_enable
 * @brief OSPI set STR SPI Enable.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 *
 * @retval FSP_SUCCESS(0)  Success
 * @retval Other than 0    Failure
 ******************************************************************************/
static fsp_err_t ospi_set_spi_mode_enable(xspi_ospi_instance_ctrl_t * const p_ctrl)
{
    fsp_err_t fsp_err;
    
    do
    {
        /*----------------- Enter 4byte address mode(EN4B) (0xB7) ----------------*/
        spi_flash_direct_transfer_t direct_command_EN4B = {0};
        direct_command_EN4B.command        = 0xB7U;
        direct_command_EN4B.command_length = 1U;
        
        fsp_err = R_XSPI_OSPI_DirectTransfer(p_ctrl, &direct_command_EN4B, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
        if (FSP_SUCCESS != fsp_err)
        {
            break;
        }
        /*--------------------------------------------------------------*/
        /* Depending on the setting of the non-volatile bit, Octa flash may start from OPI mode. */
        R_XSPI_OSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_8D_8D_8D);
        
        /*--------------- Write Enable(WE) (06F9h) ---------------*/
        spi_flash_direct_transfer_t ospi_test_direct_transfer_WE = {0};
        ospi_test_direct_transfer_WE.command        = 0x06F9U;
        ospi_test_direct_transfer_WE.command_length = 2U;
        
        fsp_err = R_XSPI_OSPI_DirectTransfer(p_ctrl, &ospi_test_direct_transfer_WE, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
        if (FSP_SUCCESS != fsp_err)
        {
            break;
        }
        /*--------------- Write Configuration register(WRCR2) (728Dh) ---------------*/
        uint32_t write_data = 0x00; /* Set STR SPI Enable */
        spi_flash_direct_transfer_t direct_command_WRCR2 = {0};
        direct_command_WRCR2.command        = 0x728DU;
        direct_command_WRCR2.address        = 0x00000000U;
        direct_command_WRCR2.command_length = 2U;
        direct_command_WRCR2.address_length = 4U;
        direct_command_WRCR2.data_length    = 1U;
        direct_command_WRCR2.data = write_data;
        
        fsp_err = R_XSPI_OSPI_DirectTransfer(p_ctrl, &direct_command_WRCR2, SPI_FLASH_DIRECT_TRANSFER_DIR_WRITE);
        if (FSP_SUCCESS != fsp_err)
        {
            break;
        }
        /* Set protocol 1S-1S-1S */
        R_XSPI_OSPI_SpiProtocolSet(p_ctrl, SPI_FLASH_PROTOCOL_1S_1S_1S);
        
        /*---------------- Read Configuration register 2(RDCR2) (71h) ---------------*/
        spi_flash_direct_transfer_t direct_command_RDCR2 = {0};
        direct_command_RDCR2.command        = 0x71U;
        direct_command_RDCR2.address        = 0x00000000U;
        direct_command_RDCR2.command_length = 2U;
        direct_command_RDCR2.address_length = 4U;
        direct_command_RDCR2.data_length    = 1U;
        direct_command_RDCR2.dummy_cycles   = 0U;
        
        fsp_err = R_XSPI_OSPI_DirectTransfer(p_ctrl, &direct_command_RDCR2, SPI_FLASH_DIRECT_TRANSFER_DIR_READ);
        if (FSP_SUCCESS != fsp_err)
        {
            break;
        }
        /*--------------------------------------------------------------*/
    } while (0);
    
    return fsp_err;
}
#endif /* (1 == USE_FLASH_OSPI_XSPI0) || (1 == USE_FLASH_OSPI_XSPI1) */

#if (1 == USE_FLASH_NOR_CS0)
/******************************************************************************
 * @fn nor_erase_flash
 * @brief NOR Erases the specified area.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  addr           Start address to erase
 * @param[in]  len            Length to erase
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t nor_erase_flash(bsc_nor_instance_ctrl_t * const p_ctrl, uint32_t addr, uint32_t len)
{
    flash_err_t flash_err = FLASH_SUCCESS;
    fsp_err_t   fsp_err;
    
    /* Erase flash. */
    fsp_err = R_BSC_NOR_Erase(p_ctrl, (uint8_t *)addr, len);
    if (FSP_SUCCESS != fsp_err)
    {
        flash_err = FLASH_ERR_MODULE;
    }
    else
    {
        /* Wait for status register to update. */
        nor_flash_status_t status_erase;
        do
        {
            (void)R_BSC_NOR_StatusGet(p_ctrl, (uint8_t *)addr, &status_erase);
        } while (true == status_erase.write_in_progress);
    }
    
    return flash_err;
}

/******************************************************************************
 * @fn nor_write_flash
 * @brief NOR Writes data to the specified address.
 *
 * @param[in]  p_ctrl         Pointer to instance control block
 * @param[in]  src            Source address
 * @param[in]  dest           Dest address
 * @param[in]  len            Write data length
 *
 * @retval FLASH_SUCCESS      Success
 * @retval FLASH_ERR_MODULE   Failure
 ******************************************************************************/
static flash_err_t nor_write_flash(bsc_nor_instance_ctrl_t * const p_ctrl, uint32_t src, uint32_t dest, uint32_t len)
{
    flash_err_t flash_err = FLASH_SUCCESS;
    fsp_err_t   fsp_err;
    
    /* Write data. */
    fsp_err = R_BSC_NOR_Write(p_ctrl, (uint8_t *)src, (uint8_t *)dest, len);
    if (FSP_SUCCESS != fsp_err)
    {
        flash_err = FLASH_ERR_MODULE;
    }
    else
    {
        /* Wait for status register to update. */
        nor_flash_status_t status_write;
        do
        {
            (void)R_BSC_NOR_StatusGet(p_ctrl, (uint8_t *)(dest & NOR_ERASE_MASK), &status_write);
        } while (true == status_write.write_in_progress);
    }
    
    return flash_err;
}
#endif /* (1 == USE_FLASH_NOR_CS0) */
