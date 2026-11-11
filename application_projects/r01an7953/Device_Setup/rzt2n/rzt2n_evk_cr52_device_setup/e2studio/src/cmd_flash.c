/***********************************************************************************************************************
 * File Name    : cmd_flash.c
 * Description  : Contains data structures and functions used in cmd_flash.c.
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
#include "cmd_flash.h"
#include "flash.h"
#include "return_code.h"

#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZN2L)
/******************************************************************************
 * @fn cmd_write_flash
 * @brief Write to flash. (for RZ/T2M, RZ/T2ME, RZ/N2L)
 *
 * @param[in]  flash_addr     Write address
 * @param[in]  p_data         Write data
 * @param[in]  data_len       Write data length
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_write_flash(uint32_t flash_addr, uint8_t * const p_data, uint32_t data_len)
{
    flash_err_t flash_err;
    uint8_t     ret           = RET_SUCCESS;
    uint32_t    data_end_addr = 0U;
    
    /* Check whether the write data size is not 0 */
    if (0U == data_len)
    {
        return RET_DATA_FAIL;
    }
    
    /* Check if data fits into 32-bit address space */
    if ((MAX_VAL_ADDR_AREA_32BIT_SPACE - data_len + 1U) < flash_addr)
    {
        return RET_DATA_FAIL;
    }
    
    data_end_addr = (flash_addr + data_len) - 1U;
    
    /* The write destination flash is determined from the address */
    /* Check the write destination address and write data size */
    if ((XSPI0_START_ADDR <= flash_addr) && (data_end_addr <= XSPI0_END_ADDR))
    {
        /* xSPI0 flash area */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_0);
        
        if (FLASH_SUCCESS == flash_err)
        {
            flash_err = write_to_qspi_area(flash_addr, p_data, data_len, XSPI_UNIT_NUM_0);
            close_qspi_flash(XSPI_UNIT_NUM_0);
        }
    }
    else if ((NOR_CS0_START_ADDR <= flash_addr) && (data_end_addr <= NOR_CS0_END_ADDR))
    {
        /* NOR flash area */
        flash_err = open_nor_flash();
        
        if (FLASH_SUCCESS == flash_err)
        {
            flash_err = write_to_nor_area(flash_addr, p_data, data_len);
            close_nor_flash();
        }
    }
    else
    {
        /* The specified address is an unsupported area */
        /* Processing needs to be modified to write to the following areas */
        /* 0x6800_0000 - 0x6FFF_FFFF : External address space xSPI1 */
        /* 0x7400_0000 - 0x77FF_FFFF : External address space CS2 */
        /* 0x7800_0000 - 0x7BFF_FFFF : External address space CS3 */
        /* 0x7C00_0000 - 0x7FFF_FFFF : External address space CS5 */
        
        /* Two corrections are required */
        /* 1) Change the address check processing of this function and make the target area the support area */
        /* 2) After checking the address, call the write function corresponding to the target area */
        return RET_DATA_FAIL;
    }
    
    /* Set the return code */
    switch (flash_err)
    {
        case FLASH_SUCCESS:
            ret = RET_SUCCESS;
            break;
        
        case FLASH_ERR_PARAMETER:
            ret = RET_DATA_FAIL;
            break;
        
        default:
            ret = RET_WRITE_FAIL;
            break;
    }
    
    return ret;
}
#endif /* defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZN2L) */

#if defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
/******************************************************************************
 * @fn cmd_write_flash
 * @brief Write to flash. (for RZ/T2L, RZ/T2H, RZ/T2N, RZ/N2H)
 *
 * @param[in]  flash_addr     Write address
 * @param[in]  p_data         Write data
 * @param[in]  data_len       Write data length
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_write_flash(uint32_t flash_addr, uint8_t * const p_data, uint32_t data_len)
{
    flash_err_t flash_err;
    uint8_t     ret           = RET_SUCCESS;
    uint32_t    data_end_addr = 0U;
    
    /* Check whether the write data size is not 0 */
    if (0U == data_len)
    {
        return RET_DATA_FAIL;
    }
    
    /* Check if data fits into 32-bit address space */
    if ((MAX_VAL_ADDR_AREA_32BIT_SPACE - data_len + 1U) < flash_addr)
    {
        return RET_DATA_FAIL;
    }
    
    data_end_addr = (flash_addr + data_len) - 1U;
    
    /* The write destination flash is determined from the address */
    /* Check the write destination address and write data size */
    if ((XSPI0_START_ADDR <= flash_addr) && (data_end_addr <= XSPI0_END_ADDR))
    {
        /* xSPI0 flash area */
        flash_err = open_ospi_flash(XSPI_UNIT_NUM_0);
        
        if (FLASH_SUCCESS == flash_err)
        {
            flash_err = write_to_ospi_area(flash_addr, p_data, data_len, XSPI_UNIT_NUM_0);
            close_ospi_flash(XSPI_UNIT_NUM_0);
        }
    }
    else if ((XSPI1_START_ADDR <= flash_addr) && (data_end_addr <= XSPI1_END_ADDR))
    {
        /* xSPI1 flash area */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_1);
        
        if (FLASH_SUCCESS == flash_err)
        {
            flash_err = write_to_qspi_area(flash_addr, p_data, data_len, XSPI_UNIT_NUM_1);
            close_qspi_flash(XSPI_UNIT_NUM_1);
        }
    }
    else
    {
        /* The specified address is an unsupported area */
        /* Processing needs to be modified to write to the following areas */
        /* 0x7000_0000 - 0x73FF_FFFF : External address space CS0 */
        /* 0x7400_0000 - 0x77FF_FFFF : External address space CS2 */
        /* 0x7800_0000 - 0x7BFF_FFFF : External address space CS3 */
        /* 0x7C00_0000 - 0x7FFF_FFFF : External address space CS5 */
        
        /* Two corrections are required */
        /* 1) Change the address check processing of this function and make the target area the support area */
        /* 2) After checking the address, call the write function corresponding to the target area */
        return RET_DATA_FAIL;
    }
    
    /* Set the return code */
    switch (flash_err)
    {
        case FLASH_SUCCESS:
            ret = RET_SUCCESS;
            break;
        
        case FLASH_ERR_PARAMETER:
            ret = RET_DATA_FAIL;
            break;
        
        default:
            ret = RET_WRITE_FAIL;
            break;
    }
    
    return ret;
}
#endif /* defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H) */
