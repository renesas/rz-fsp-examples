/***********************************************************************************************************************
 * File Name    : cmd_otp.c
 * Description  : Contains data structures and functions used in cmd_otp.c.
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
#include "cmd_otp.h"
#include "otp.h"
#include "return_code.h"

/* Macro definitions */
/* WORDLOCK status */
#define WORDLOCK_DISABLE         (0U)
#define WORDLOCK_ENABLE          (1U)

/* SCI/USB boot mode */
#define SCIUSB_BOOT_MODE_ENABLE  (0U)
#define SCIUSB_BOOT_MODE_DISABLE (1U)

/******************************************************************************
 * @fn cmd_write_otp
 * @brief Write to OTP.
 *
 * @param[in]  otp_addr       Write address
 * @param[in]  data           Write data
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_write_otp(uint16_t otp_addr, otp_data_width_t data)
{
    uint8_t   ret      = RET_SUCCESS;
    uint8_t   wordlock = WORDLOCK_DISABLE;
    otp_err_t otp_err  = OTP_SUCCESS;
    
    /* Check address */
    if ((COUNTER_AREA_START_ADDR <= otp_addr ) && (otp_addr <= COUNTER_AREA_END_ADDR))
    {
        /* Specify the address of the anti-rollback counter area.  */
    }
    else if ((USER_AREA_START_ADDR <= otp_addr ) && (otp_addr <= USER_AREA_END_ADDR))
    {
        /* Specify the address of the user area */
#if !defined(BSP_MCU_GROUP_RZT2H) && !defined(BSP_MCU_GROUP_RZN2H)
        /* WORDLOCK is enabled by default for all models except RZ/T2H, RZ/N2H */
        wordlock = WORDLOCK_ENABLE;
#endif /* !defined(BSP_MCU_GROUP_RZT2H) && !defined(BSP_MCU_GROUP_RZN2H) */
    }
#if defined(BSP_MCU_GROUP_RZN2L) || defined(BSP_MCU_GROUP_RZT2L)
    else if ((SHOSTIF_BOOT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= SHOSTIF_BOOT_AREA_END_ADDR))
    {
        /* Specify the address of the SHOSTIF boot mode area */
        wordlock = WORDLOCK_ENABLE;
    }
#endif /* defined(BSP_MCU_GROUP_RZN2L) || defined(BSP_MCU_GROUP_RZT2L) */
#if defined(BSP_MCU_GROUP_RZN2L)
    else if ((PHOSTIF_BOOT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= PHOSTIF_BOOT_AREA_END_ADDR))
    {
        /* Specify the address of the PHOSTIF boot mode area */
        wordlock = WORDLOCK_ENABLE;
    }
#endif /* defined(BSP_MCU_GROUP_RZN2L) */
    else
    {
        /* Specify the address of the non-write area */
        return RET_DATA_FAIL;
    }
    
    /* OTP power on */
    otp_err = otp_power_on();
    
    if (OTP_SUCCESS != otp_err)
    {
        return RET_WRITE_FAIL;
    }
    
    do
    {
        /* Write data */
        otp_err = write_otp_data(otp_addr, data);
            
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
        otp_data_width_t read_data = 0U;
        
        /* Read the data from the write destination area and check the data */
        otp_err = read_otp_data(otp_addr, &read_data);
        
        if (WORDLOCK_DISABLE == wordlock)
        {
            read_data &= data;
        }
        
        if ((OTP_SUCCESS != otp_err) || (data != read_data))
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}

/******************************************************************************
 * @fn cmd_read_otp
 * @brief Read OTP. 
 *
 * @param[in]  otp_addr       Read address
 * @param[in]  p_data         Read data storage destination
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_READ_FAIL   Read error
 ******************************************************************************/
uint8_t cmd_read_otp(uint16_t otp_addr, otp_data_width_t * p_data)
{
    uint8_t   ret     = RET_SUCCESS;
    otp_err_t otp_err = OTP_SUCCESS;
    
    /* Check address */
    if ((COUNTER_AREA_START_ADDR <= otp_addr ) && (otp_addr <= COUNTER_AREA_END_ADDR))
    {
        /* Specify the address of the anti-rollback counter area */
    }
    else if ((PART_NUM_START_ADDR <= otp_addr) && (otp_addr <= PART_NUM_END_ADDR))
    {
        /* Specify the address of part number area */
    }
    else if ((PRODUCT_VER_START_ADDR <= otp_addr) && (otp_addr <= PRODUCT_VER_END_ADDR))
    {
        /* Specify the address of product version area */
    }
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
    else if ((TEMPERATURE_SENSOR_H_ADDR == otp_addr) || (TEMPERATURE_SENSOR_L_ADDR == otp_addr))
    {
        /* Specify the address of temperature sensor code */
    }
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H) */
#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZN2L)
    else if ((TX_CLOCK_CNT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= TX_CLOCK_CNT_AREA_END_ADDR))
    {
        /* Specify the address of the TX clock count area */
    }
    else if ((RX_CLOCK_CNT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= RX_CLOCK_CNT_AREA_END_ADDR))
    {
        /* Specify the address of the RX clock count area */
    }
#endif /* defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZN2L) */
    else if ((USER_AREA_START_ADDR <= otp_addr ) && (otp_addr <= USER_AREA_END_ADDR))
    {
        /* Specify the address of the user area */
    }
#if defined(BSP_MCU_GROUP_RZN2L) || defined(BSP_MCU_GROUP_RZT2L)
    else if ((SHOSTIF_BOOT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= SHOSTIF_BOOT_AREA_END_ADDR))
    {
        /* Specify the address of the SHOSTIF boot mode area */
    }
#endif /* defined(BSP_MCU_GROUP_RZN2L) || defined(BSP_MCU_GROUP_RZT2L) */
#if defined(BSP_MCU_GROUP_RZN2L)
    else if ((PHOSTIF_BOOT_AREA_START_ADDR <= otp_addr ) && (otp_addr <= PHOSTIF_BOOT_AREA_END_ADDR))
    {
        /* Specify the address of the PHOSTIF boot mode area */
    }
#endif /* defined(BSP_MCU_GROUP_RZN2L) */
#if defined(BSP_MCU_GROUP_RZT2N)
    else if ((PCIE_PHY_TRIMMING_CODE_START_ADDR <= otp_addr ) && (otp_addr <= PCIE_PHY_TRIMMING_CODE_END_ADDR))
    {
        /* Specify the address of the PCIe PHY trimming code area */
    }
#endif /* defined(BSP_MCU_GROUP_RZT2N) */
    else
    {
        /* Specify the address of the non-read area */
        return RET_DATA_FAIL;
    }
    
    /* OTP power on */
    otp_err = otp_power_on();
    
    if (OTP_SUCCESS != otp_err)
    {
        return RET_READ_FAIL;
    }
    
    /* Read data */
    otp_err = read_otp_data(otp_addr, p_data);
    
    if (OTP_SUCCESS != otp_err)
    {
        ret = RET_READ_FAIL;
    }
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}

#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L)
/******************************************************************************
 * @fn cmd_set_sci_usb_boot
 * @brief Setup SCI/USB boot.
 *
 * @param[in]  mode           SCI/USB boot mode
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_READ_FAIL   Read error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_set_sci_usb_boot(uint8_t mode)
{
    uint8_t          ret         = RET_SUCCESS;
    otp_err_t        otp_err     = OTP_SUCCESS;
    otp_data_width_t set_val     = 0U;
    otp_data_width_t current_val = 0U;
    
    /* Check if boot mode disabled is specified */
    if (SCIUSB_BOOT_MODE_DISABLE == mode)
    {
        set_val = OTP_VAL_SUDIS_DISABLE;
        
        /* OTP power on */
        otp_err = otp_power_on();
        
        if (OTP_SUCCESS != otp_err)
        {
            return RET_WRITE_FAIL;
        }
        
        do
        {
            /* Read SCU/USB boot setting value */
            otp_err = read_otp_data(SCI_USB_BOOT_ADDR, &current_val);
            
            if (OTP_SUCCESS != otp_err)
            {
                ret = RET_READ_FAIL;
                break;
            }
            
            if (set_val <= current_val)
            {
                break;
            }
            
            /* Write SCI/USB boot setting value */
            otp_err = write_otp_data(SCI_USB_BOOT_ADDR, set_val);
            
            if (OTP_SUCCESS != otp_err)
            {
                ret = RET_WRITE_FAIL;
                break;
            }
            
            /* Read the current setting value and check if the value is updated */
            otp_err = read_otp_data(SCI_USB_BOOT_ADDR, &current_val);
            
            if ((OTP_SUCCESS != otp_err) || (current_val < set_val))
            {
                ret = RET_WRITE_FAIL;
                break;
            }
            
        } while (0);
        
        /* OTP power off */
        otp_power_off();
    }
    
    return ret;
}

/******************************************************************************
 * @fn cmd_get_sci_usb_boot
 * @brief Get SCI/USB boot information.
 *
 * @param[out]  p_mode         SCI/USB boot mode
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_READ_FAIL   Read error
 ******************************************************************************/
uint8_t cmd_get_sci_usb_boot(uint8_t * p_mode)
{
    uint8_t          ret         = RET_SUCCESS;
    otp_err_t        otp_err     = OTP_SUCCESS;
    otp_data_width_t current_val = 0U;
    
    /* OTP power on */
    otp_err = otp_power_on();
    
    if (OTP_SUCCESS != otp_err)
    {
        return RET_READ_FAIL;
    }
    
    do
    {
        /* Read SCI/USB boot setting value */
        otp_err = read_otp_data(SCI_USB_BOOT_ADDR, &current_val);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_READ_FAIL;
            break;
        }
        
        if (current_val & OTP_VAL_SUDIS_DISABLE)
        {
            /* Disable */
            *p_mode = SCIUSB_BOOT_MODE_DISABLE;
        }
        else
        {
            /* Enable */
            *p_mode = SCIUSB_BOOT_MODE_ENABLE;
        }
        
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}
#endif /* defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2ME) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L) */

/******************************************************************************
 * @fn cmd_get_unique_id
 * @brief Read unique ID.
 *
 * @param[out]  p_uid         Unique ID
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_READ_FAIL   Read error
 ******************************************************************************/
uint8_t cmd_get_unique_id(uint8_t * p_uid)
{
    uint8_t   ret     = RET_SUCCESS;
    otp_err_t otp_err = OTP_SUCCESS;
    
    /* OTP power on */
    otp_err = otp_power_on();
    
    if (OTP_SUCCESS != otp_err)
    {
        return RET_READ_FAIL;
    }
    
    /* Read unique ID */
    otp_err = read_otp_multiple_data(UID_ADDR, p_uid, UID_SIZE);
    
    if (OTP_SUCCESS != otp_err)
    {
        ret = RET_READ_FAIL;
    }
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}
