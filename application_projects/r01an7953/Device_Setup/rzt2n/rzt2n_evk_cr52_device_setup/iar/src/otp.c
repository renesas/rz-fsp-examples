/***********************************************************************************************************************
 * File Name    : otp.c
 * Description  : Contains data structures and functions used in otp.c.
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
#include "otp.h"

/******************************************************************************
 * @fn otp_power_on
 * @brief OTP power on.
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t otp_power_on(void)
{
    /* Confirm that CMD_RDY bit of the OTP access status register (OTPSTR.CMD_RDY) is 1 */
    if (0U == R_OTP->OTPSTR_b.CMD_RDY)
    {
        /* Confirm completion of the power-down process */
        if (1U == R_OTP->OTPPWR_b.PWR)
        {
            return OTP_ERROR;
        }
    }
    
    /* Set the PWR and ACCL bits of the OTP Power Control Register */
    R_OTP->OTPPWR = R_OTP_OTPPWR_PWR_Msk | R_OTP_OTPPWR_ACCL_Msk;
    
    return OTP_SUCCESS;
}

/******************************************************************************
 * @fn otp_power_off
 * @brief OTP power off.
 ******************************************************************************/
void otp_power_off(void)
{
    /* Set the PWR and ACCL bits to 0, and wait until CMD_RDY bit is set to 0 */
    R_OTP->OTPPWR = ~(uint32_t) (R_OTP_OTPPWR_PWR_Msk | R_OTP_OTPPWR_ACCL_Msk);
    
    while (1)
    {
        if (0U == R_OTP->OTPSTR_b.CMD_RDY)
        {
            break;
        }
    }
    
    return;
}

/******************************************************************************
 * @fn write_otp_data
 * @brief Writes data for OTP data width to OTP.
 *
 * @param[in]  otp_addr       Write address
 * @param[in]  data           Write data
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t write_otp_data(uint16_t otp_addr, otp_data_width_t data)
{
    otp_err_t ret = OTP_SUCCESS;
    
    /* Confirm that CMD_RDY bit of the OTP access status register (OTPSTR.CMD_RDY) is 1 */
    while (0U == R_OTP->OTPSTR_b.CMD_RDY);
    
    /* Set the PWR and ACCL bits of the OTP Power Control Register */
    R_OTP->OTPPWR = R_OTP_OTPPWR_PWR_Msk | R_OTP_OTPPWR_ACCL_Msk;
    
    /* Set the write address to the OTP Write Address Register */
    R_OTP->OTPADRWR_b.ADRWR = (uint32_t)(OTP_ADDRESS_MASK & otp_addr);
    
    /* Set the data to be written to the OTP Write Data Register */
    R_OTP->OTPDATAWR_b.DATAWR = data; 
    
    /* Set the STARWR bit of the OTP Write Command Register to 1 */
    R_OTP->OTPSTAWR_b.STAWR = 1U;
    
    /* Poll the STAWR bit until changing to 0 in order to detect the completion of the write command acceptance */
    while (1)
    {
        if (0U == R_OTP->OTPSTAWR_b.STAWR)
        {
            break;
        }
    }
    
    /* Poll the CMD_RDY bit until changing to 1 in order to detect the completion of the write command */
    while (1)
    {
        if (1U == R_OTP->OTPSTR_b.CMD_RDY)
        {
            break;
        }
    }
    
    /* Check OTP write error */
    if ((0U != R_OTP->OTPSTR_b.ERR_WR) || (0U != R_OTP->OTPSTR_b.ERR_WP))
    {
        ret = OTP_ERROR;
    }
    
    if (0U != R_OTP->OTPSTR_b.ERR_RDY_WR)
    {
        ret = OTP_ERROR;
        R_OTP->OTPSTR_b.ERR_RDY_WR = 0U;
    }
    
    return ret;
}

/******************************************************************************
 * @fn read_otp_data
 * @brief Reads data for OTP data width from OTP.
 *
 * @param[in]  otp_addr       Read address
 * @param[in]  p_data         Read data storage destination
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t read_otp_data(uint16_t otp_addr, otp_data_width_t * p_data)
{
    otp_err_t ret = OTP_SUCCESS;
    
    /* Confirm that CMD_RDY bit of the OTP access status register (OTPSTR.CMD_RDY) is 1 */
    while (0U == R_OTP->OTPSTR_b.CMD_RDY);
    
    /* Set the PWR and ACCL bits of the OTP Power Control Register */
    R_OTP->OTPPWR = R_OTP_OTPPWR_PWR_Msk | R_OTP_OTPPWR_ACCL_Msk;
    
    /* Set the read address to the OTP Read Address Register */
    R_OTP->OTPADRRD_b.ADRRD = (uint32_t)(OTP_ADDRESS_MASK & otp_addr);
    
    /* Read the OTP Read Data Register */
    *p_data = R_OTP->OTPDATARD_b.DATARD;
    
    /* Poll the CMD_RDY bit until changing to 1 in order to detect the completion of the write command */
    while (1)
    {
        if (1U == R_OTP->OTPSTR_b.CMD_RDY)
        {
            break;
        }
    }
    
    /* Check OTP read error */
    if (0U != R_OTP->OTPSTR_b.ERR_RP)
    {
        ret = OTP_ERROR;
    }
    
    if (0U != R_OTP->OTPSTR_b.ERR_RDY_RD)
    {
        ret = OTP_ERROR;
        R_OTP->OTPSTR_b.ERR_RDY_RD = 0U;
    }
    
    return ret;
}

/******************************************************************************
 * @fn write_otp_multiple_data
 * @brief Write to the OTP area in units of OTP data width.
 *
 * @param[in]  addr           Write destination address 
 * @param[in]  p_data         Data
 * @param[in]  data_len       Data length
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t write_otp_multiple_data(uint16_t addr, uint8_t * const p_data, uint32_t data_len)
{
    otp_err_t ret        = OTP_SUCCESS;
    uint32_t  comp_len   = 0U;
    uint8_t   data_width = sizeof(otp_data_width_t);
    
    /* Check whether the data size is in data width units */
    if (0U != (data_len % data_width))
    {
        return OTP_ERROR;
    }
    
    /* Write the data in data width units */
    while (comp_len < data_len)
    {
        otp_data_width_t write_data = 0U;
        memcpy(&write_data, p_data + comp_len, sizeof(otp_data_width_t));
        
        /* Write data */
        ret = write_otp_data(addr, write_data);
        
        if (OTP_SUCCESS != ret)
        {
            break;
        }
        
        comp_len += sizeof(otp_data_width_t);
        addr++;
    }
    
    return ret;
}

/******************************************************************************
 * @fn read_otp_multiple_data
 * @brief Read the OTP area in units of OTP data width.
 *
 * @param[in]  addr           Read destination address 
 * @param[in]  p_data         Data
 * @param[in]  data_len       Data length
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t read_otp_multiple_data(uint16_t addr, uint8_t * p_data, uint32_t data_len)
{
    otp_err_t ret        = OTP_SUCCESS;
    uint32_t  comp_len   = 0U;
    uint8_t   data_width = sizeof(otp_data_width_t);
    
    /* Check whether the data size is in data width units */
    if (0U != (data_len % data_width))
    {
        return OTP_ERROR;
    }
    
    /* Read the data in data width units */
    while (comp_len < data_len)
    {
        otp_data_width_t read_data = 0U;
        
        /* Read data */
        ret = read_otp_data(addr, &read_data);
        
        if (OTP_SUCCESS != ret)
        {
            break;
        }
        
        memcpy(p_data + comp_len, &read_data, sizeof(otp_data_width_t));
        comp_len += sizeof(otp_data_width_t);
        addr++;
    }
    
    return ret;
}

#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H)
/******************************************************************************
 * @fn enable_wordlock_for_otp_area
 * @brief Enables WORDLOCK for the specified OTP region.
 *
 * @param[in]  addr           Start address of the target area
 * @param[in]  area_len       Length of the target area
 *
 * @retval OTP_SUCCESS   Success
 * @retval OTP_ERROR     Failure
 ******************************************************************************/
otp_err_t enable_wordlock_for_otp_area(uint16_t addr, uint32_t area_len)
{
    otp_err_t ret                = OTP_SUCCESS;
    uint32_t  enabled_len        = 0U;
    uint8_t   data_width         = sizeof(otp_data_width_t);
    uint16_t  ctr_area_addr      = 0U;
    uint16_t  next_ctr_area_addr = 0U;
    otp_data_width_t set_val     = 0U;
    
    /* Check whether the data size is in data width units */
    if (0U != (area_len % data_width))
    {
        return OTP_ERROR;
    }
    
    /* Write the data in data width units */
    while (enabled_len < area_len)
    {
        /* Calculate the address to set it to and the bit position to write to */
        ctr_area_addr = (addr / 32U);
        set_val      |= (1U << (addr % 32U));
        
        /* Check the WORDLOCK control area address of the next target area */
        enabled_len += sizeof(otp_data_width_t);
        addr++;
        
        if (enabled_len < area_len)
        {
            next_ctr_area_addr = (addr / 32U);
            
            if (ctr_area_addr == next_ctr_area_addr)
            {
                /* If writing to the same address next time, set_val is maintained and no writing is performed */
                continue;
            }
        }
        
        /* Enable WORDLOCK */
        ret = write_otp_data(ctr_area_addr, set_val);
        
        if (OTP_SUCCESS != ret)
        {
            break;
        }
        
        otp_data_width_t read_data = 0U;
        
        /* Read the data from the write destination area and check the data */
        ret = read_otp_data(ctr_area_addr, &read_data);
        
        /* Check if the write data bit is set */
        read_data &= set_val;
        
        if ((OTP_SUCCESS != ret) || (set_val != read_data))
        {
            break;
        }
        
        /* Since writing has been done, clear set_val */
        set_val = 0U;
    }
    
    return ret;
}
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H) */
