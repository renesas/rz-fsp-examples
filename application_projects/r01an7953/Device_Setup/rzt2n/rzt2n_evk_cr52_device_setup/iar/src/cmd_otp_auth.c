/***********************************************************************************************************************
 * File Name    : cmd_otp_auth.c
 * Description  : Contains data structures and functions used in cmd_otp_auth.c.
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
#include "cmd_otp_auth.h"
#include "otp.h"
#include "return_code.h"

/* Macro definitions */
/* JTAG authentication mode */
#define JTAG_MODE_NO_AUTH          (0U)
#define JTAG_MODE_AUTHLV1          (1U)
#define JTAG_MODE_AUTHLV2          (2U)
#define JTAG_MODE_PROHIBIT         (4U)

/* SCI/USB boot authentication mode */
#define SCI_USB_BOOT_MODE_NO_AUTH  (0U)
#define SCI_USB_BOOT_MODE_AUTH     (1U)
#define SCI_USB_BOOT_MODE_PROHIBIT (2U)

/* Authentication type */
#define TYPE_PLAIN                 (0U)
#define TYPE_HASH                  (1U)

/* Authentication ID size */
#define SIZE_PLAIN_ID              (16U)
#define SIZE_HASH_ID               (32U)

/******************************************************************************
 * @fn cmd_set_jtag_auth
 * @brief Setup JTAG authentication.
 *
 * @param[in]  mode           Authentication mode
 * @param[in]  type           Authentication type
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_set_jtag_auth(uint8_t mode, uint8_t type)
{
    uint8_t          ret             = RET_SUCCESS;
    otp_err_t        otp_err         = OTP_SUCCESS;
    otp_data_width_t current_val     = 0U;
    otp_data_width_t set_val         = 0U;
    bool             mode_is_invalid = false;
    
    /* Check authentication mode and type */
    switch (mode)
    {
        case JTAG_MODE_AUTHLV1:
            set_val = OTP_VAL_AUTHMODEJ_AUTHLV1;
            break;
            
        case JTAG_MODE_AUTHLV2:
            set_val = OTP_VAL_AUTHMODEJ_AUTHLV2;
            break;
            
        case JTAG_MODE_PROHIBIT:
            set_val = OTP_VAL_AUTHMODEJ_PROHIBIT;
            break;
        
        /* Unknown mode */
        default:
            mode_is_invalid = true;
    }
    
    if ((TYPE_PLAIN != type) || (true == mode_is_invalid))
    {
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
        /* Read the current setting value. (authentication mode value) */
        /* Check if the specified value can be set */
        otp_err = read_otp_data(JTAG_AUTH_MODE_ADDR, &current_val);
        
        if ((OTP_SUCCESS != otp_err) || (current_val >= set_val))
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
        /* Write the specified setting value */
        otp_err = write_otp_data(JTAG_AUTH_MODE_ADDR, set_val);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
        /* Read the current setting value and check if the value is updated */
        otp_err = read_otp_data(JTAG_AUTH_MODE_ADDR, &current_val);
        
        if ((OTP_SUCCESS != otp_err) || (current_val < set_val))
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
 * @fn cmd_get_jtag_auth
 * @brief Get JTAG authentication information.
 *
 * @param[out]  p_mode         Authentication mode
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_READ_FAIL   Read error
 ******************************************************************************/
uint8_t cmd_get_jtag_auth(uint8_t * p_mode)
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
        /* Read authentication mode */
        otp_err = read_otp_data(JTAG_AUTH_MODE_ADDR, &current_val);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_READ_FAIL;
            break;
        }
        
        if (current_val & OTP_VAL_AUTHMODEJ_PROHIBIT)
        {
            /* Permanent prohibition */
            *p_mode = JTAG_MODE_PROHIBIT;
        }
        else if (current_val & OTP_VAL_AUTHMODEJ_AUTHLV2)
        {
            /* Authentication lv2 */
            *p_mode = JTAG_MODE_AUTHLV2;
        }
        else if (current_val & OTP_VAL_AUTHMODEJ_AUTHLV1)
        {
            /* Authentication lv1 */
            *p_mode = JTAG_MODE_AUTHLV1;
        }
        else
        {
            /* No authentication */
            *p_mode = JTAG_MODE_NO_AUTH;
        }
        
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}

/******************************************************************************
 * @fn cmd_set_jtag_auth_id
 * @brief Setup JTAG authentication ID.
 *
 * @param[in]  mode           Authentication mode
 * @param[in]  type           Authentication type
 * @param[in]  p_id           Authentication ID
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_set_jtag_auth_id(uint8_t mode, uint8_t type, uint8_t * const p_id)
{
    uint8_t   ret             = RET_SUCCESS;
    uint16_t  addr            = 0U;
    otp_err_t otp_err         = OTP_SUCCESS;
    bool      mode_is_invalid = false;
    
    /* Check where to write the authentication ID */
    switch (mode)
    {
        case JTAG_MODE_AUTHLV1:
            addr = JTAG_AUTH_LV1_PLAIN_ADDR;
            break;
            
        case JTAG_MODE_AUTHLV2:
            addr = JTAG_AUTH_LV2_PLAIN_ADDR;
            break;
        
        /* Unknown mode */
        default:
            mode_is_invalid = true;
    }
    
    /* Check authentication mode and type */
    if ((TYPE_PLAIN != type) || (true == mode_is_invalid))
    {
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
        /* Write authentication ID */
        otp_err = write_otp_multiple_data(addr, p_id, SIZE_PLAIN_ID);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H)
        /* Enable WORDLOCK */
        otp_err = enable_wordlock_for_otp_area(addr, SIZE_PLAIN_ID);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H) */
        
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}

#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
/******************************************************************************
 * @brief Setup SCI/USB boot authentication.
 *
 * @param[in]  mode           Authentication mode
 * @param[in]  type           Authentication type
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_set_sci_usb_boot_auth(uint8_t mode, uint8_t type)
{
    uint8_t          ret             = RET_SUCCESS;
    otp_err_t        otp_err         = OTP_SUCCESS;
    otp_data_width_t current_val     = 0U;
    otp_data_width_t set_val         = 0U;
    bool             mode_is_invalid = false;
    
    /* Check authentication mode and type */
    switch (mode)
    {
        case SCI_USB_BOOT_MODE_AUTH:
            set_val = OTP_VAL_AUTHMODES_AUTH;
            break;
            
        case SCI_USB_BOOT_MODE_PROHIBIT:
            set_val = OTP_VAL_AUTHMODES_PROHIBIT;
            break;
        
        /* Unknown mode */
        default:
            mode_is_invalid = true;
    }
    
    if ((TYPE_PLAIN != type) || (true == mode_is_invalid))
    {
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
        /* Read the current setting value. (authentication mode value) */
        /* Check if the specified value can be set */
        otp_err = read_otp_data(SCI_USB_BOOT_MODE_ADDR, &current_val);
        
        if ((OTP_SUCCESS != otp_err) || (current_val >= set_val))
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
        /* Write the specified setting value */
        otp_err = write_otp_data(SCI_USB_BOOT_MODE_ADDR, set_val);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
        
        /* Read the current setting value and check if the value is updated */
        otp_err = read_otp_data(SCI_USB_BOOT_MODE_ADDR, &current_val);
        
        if ((OTP_SUCCESS != otp_err) || (current_val < set_val))
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
 * @brief Get SCI/USB boot authentication information.
 *
 * @param[out]  p_mode         Authentication mode
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_READ_FAIL   Read error
 ******************************************************************************/
uint8_t cmd_get_sci_usb_boot_auth(uint8_t *p_mode)
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
        /* Read authentication mode */
        otp_err = read_otp_data(SCI_USB_BOOT_MODE_ADDR, &current_val);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_READ_FAIL;
            break;
        }
        
        if (current_val & OTP_VAL_AUTHMODES_PROHIBIT)
        {
            /* Permanent prohibition */
            *p_mode = SCI_USB_BOOT_MODE_PROHIBIT;
        }
        else if (current_val & OTP_VAL_AUTHMODES_AUTH)
        {
            /* Authentication */
            *p_mode = SCI_USB_BOOT_MODE_AUTH;
        }
        else
        {
            /* No authentication */
            *p_mode = SCI_USB_BOOT_MODE_NO_AUTH;
        }
        
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}

/******************************************************************************
 * @brief Setup SCI/USB boot authentication ID.
 *
 * @param[in]  type           Authentication type
 * @param[in]  p_id           Authentication ID
 *
 * @retval RET_SUCCESS     Success
 * @retval RET_DATA_FAIL   Data error
 * @retval RET_WRITE_FAIL  Write error
 ******************************************************************************/
uint8_t cmd_set_sci_usb_boot_auth_id(uint8_t type, uint8_t * const p_id)
{
    uint8_t  ret      = RET_SUCCESS;
    otp_err_t otp_err = OTP_SUCCESS;
    
    /* Check authentication type */
    if (TYPE_PLAIN != type)
    {
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
        /* Write authentication ID */
        otp_err = write_otp_multiple_data(SCI_USB_BOOT_ID_PLAIN_ADDR, p_id, SIZE_PLAIN_ID);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H)
        /* Enable WORDLOCK */
        otp_err = enable_wordlock_for_otp_area(SCI_USB_BOOT_ID_PLAIN_ADDR, SIZE_PLAIN_ID);
        
        if (OTP_SUCCESS != otp_err)
        {
            ret = RET_WRITE_FAIL;
            break;
        }
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZN2H) */
    
    } while (0);
    
    /* OTP power off */
    otp_power_off();
    
    return ret;
}
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H) */
