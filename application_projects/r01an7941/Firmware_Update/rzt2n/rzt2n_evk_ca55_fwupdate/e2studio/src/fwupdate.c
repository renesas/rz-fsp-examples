/***********************************************************************************************************************
 * Copyright [2020-2025] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
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
#include "fwupdate.h"
#include "crc32.h"
#include "flash.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/* Offset value */
#define USER_APPLICATION_AREA_OFFSET    ((uint32_t)(USER_APPLICATION_AREA_END_ADDR - USER_APPLICATION_AREA_START_ADDR + 1U))

/* Update target value */
#define OPERATING_CPU_CORE_MASK         ((uint32_t)0x0000000FUL)
#define PARAM_INFO_EXIST_MASK           ((uint32_t)0x00000010UL)

/* Other */
#define BOOT_STATE_A                    ((uint32_t)0x10011001UL)
#define BOOT_STATE_B                    ((uint32_t)0x01100110UL)
#define CRC_DATA_SIZE                   (4U)

/******************************************************************************
 * Typedef definitions
 ******************************************************************************/
/* Return check value */
typedef enum e_ret_err_t
{
    RET_SUCCESS = 0,
    RET_FAIL    = 1,
} ret_err_t;

/* Firmware Update result */
typedef enum e_update_ret_t
{
    UPDATE_RESULT_SUCCESS                     = 0,
    UPDATE_RESULT_FILE_INVALID                = 1,
    UPDATE_RESULT_USER_APPLICATION_WRITE_FAIL = 2,
    UPDATE_RESULT_PARAMETER_WRITE_FAIL        = 3,
    UPDATE_RESULT_FLASH_MNG_AREA_UPDATE_FAIL  = 4,
} update_ret_t;

/* Target application */
typedef enum e_update_target_t
{
    UPDATE_TARGET_CPU_CORE0 = 0,
    UPDATE_TARGET_CPU_CORE1 = 1,
    UPDATE_TARGET_CPU_CORE2 = 2,
    UPDATE_TARGET_CPU_CORE3 = 3
} update_target_t;

/* Update information format */
typedef struct st_update_info_t
{
    uint32_t magic_num;
    uint32_t reserved_1[2];
    uint32_t write_addr;
    uint32_t reserved_2[1];
    uint32_t size;
    uint32_t reserved_3[1];
    uint32_t target;
    uint32_t tlv_length;
    uint32_t tlv_field[9];
} update_info_t;

/******************************************************************************
 * Private global variables and functions
 *****************************************************************************/
static update_ret_t firmware_update (uint8_t * const p_data, uint32_t size);
static ret_err_t    check_updatefile (uint8_t * const p_data, uint32_t size);
static ret_err_t    write_user_application (update_info_t * const p_info, uint8_t * const p_data);
static ret_err_t    write_param_info (update_info_t * const p_info, uint8_t * const p_data);

#if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
static ret_err_t change_flash_mng_area (update_info_t * const p_info);
static void      read_flash_data (uint32_t flash_addr, void * const p_data, uint32_t size);
#endif

static ret_err_t open_flash (void);
static void      close_flash (void);
static ret_err_t write_flash_data (uint32_t flash_addr, void * const p_data, uint32_t size);

/******************************************************************************
 * @fn       packet_handler
 * @brief    Parse commands from received packets.
 ******************************************************************************/
void packet_handler(void)
{
    uint8_t         * p_buf        = (uint8_t *)PACKET_BUFFER_ADDR;
    packet_header_t * p_rcv_packet = (packet_header_t *)p_buf;
    
    /* Firmware update packet analysis. */
    if (CMD_CODE_START_UPDATE == p_rcv_packet->command)
    {
        /* Set Ack command. */
        create_send_packet(p_rcv_packet, CMD_CODE_ACK, 0U);
    }
    else if (CMD_CODE_FIRMWARE_DATA == p_rcv_packet->command)
    {
        uint32_t rcv_payload_len = p_rcv_packet->payload_len;
        
        /* Firmware update process. */
        update_ret_t update_result = firmware_update(p_rcv_packet->payload, rcv_payload_len);
        
        /* Check fwupdate result. */
        switch (update_result)
        {
            case UPDATE_RESULT_SUCCESS:
                create_send_packet(p_rcv_packet, CMD_CODE_ACK, NACK_CODE_NONE);
                break;
                
            case UPDATE_RESULT_FILE_INVALID:
                create_send_packet(p_rcv_packet, CMD_CODE_NACK, NACK_CODE_FILE_INVALID);
                break;
                
            case UPDATE_RESULT_USER_APPLICATION_WRITE_FAIL:
                create_send_packet(p_rcv_packet, CMD_CODE_NACK, NACK_CODE_USER_APPLICATION_WRITE_FAIL);
                break;
                
            case UPDATE_RESULT_PARAMETER_WRITE_FAIL:
                create_send_packet(p_rcv_packet, CMD_CODE_NACK, NACK_CODE_PARAMETER_WRITE_FAIL);
                break;
                
            case UPDATE_RESULT_FLASH_MNG_AREA_UPDATE_FAIL:
                create_send_packet(p_rcv_packet, CMD_CODE_NACK, NACK_CODE_FLASH_MNG_AREA_UPDATE_FAIL);
                break;
                
            default:
                /* Unknown update result. */
                /* No packets are sent. */
                create_send_packet(p_rcv_packet, CMD_CODE_NONE, NACK_CODE_NONE);
                break;
        }
    }
    else
    {
        /* Unknown command code. */
        /* No packets are sent. */
        create_send_packet(p_rcv_packet, CMD_CODE_NONE, NACK_CODE_NONE);
    }
    
    return;
}

/******************************************************************************
 * @fn       firmware_update
 * @brief    Main process of firmware update.
 *
 * @param[in] p_data    Update file
 * @param[in] size      Update file size
 *
 * @retval UPDATE_RESULT_SUCCESS                        Firmware update success(0)
 * @retval UPDATE_RESULT_FILE_INVALID                   Invalid update file(1)
 * @retval UPDATE_RESULT_USER_APPLICATION_WRITE_FAIL    User application write fail(2)
 * @retval UPDATE_RESULT_PARAMETER_WRITE_FAIL           Parameter write fail(3)
 * @retval UPDATE_RESULT_FLASH_MNG_AREA_UPDATE_FAIL     Flash management area update fail(5)
 ******************************************************************************/
static update_ret_t firmware_update(uint8_t * const p_data, uint32_t size)
{
    ret_err_t    check_ret     = RET_FAIL;
    ret_err_t    ret           = RET_FAIL;
    update_ret_t update_result = UPDATE_RESULT_FILE_INVALID;
    
    /* Open the flash. */
    ret = open_flash();
    if (RET_SUCCESS != ret)
    {
        return UPDATE_RESULT_USER_APPLICATION_WRITE_FAIL;
    }
    
    do
    {
        /* Performs update file verification processing. */
        check_ret = check_updatefile(p_data, size);
        if (RET_SUCCESS != check_ret)
        {
            update_result = UPDATE_RESULT_FILE_INVALID;
            break;
        }
        
        /* Copy the Update Info of the update file. */
        update_info_t info;
        memcpy(&info, p_data, sizeof(update_info_t));
        
        /* Get the offset value to refer to the user application. */
        uint32_t fw_offset = sizeof(update_info_t);
        if (PARAM_INFO_EXIST_MASK & info.target)
        {
            fw_offset += USER_PARAM_INFO_AREA_SIZE;
        }
        
        /* Write the user application. */
        ret = write_user_application(&info, p_data + fw_offset);
        if (RET_SUCCESS != ret)
        {
            update_result = UPDATE_RESULT_USER_APPLICATION_WRITE_FAIL;
            break;
        }
        
        /* Check if the parameter for the user application is included. */
        if (PARAM_INFO_EXIST_MASK & info.target)
        {
            /* Initialize the update processing result. */
            ret = RET_FAIL;
            
            /* Write the parameter for the user application. */
            ret = write_param_info(&info, p_data + sizeof(update_info_t));
            if (RET_SUCCESS != ret)
            {
                update_result = UPDATE_RESULT_PARAMETER_WRITE_FAIL;
                break;
            }
            
            /* Switch flash management area. */
            /* Executed only when writing of parameter for the user application succeeds. */
            /* If the parameter for the user application is not given to the update file, do nothing. */
#if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
            /* Initialize the update processing result. */
            ret = RET_FAIL;
            
            /* Change the flash management area status. */
            ret = change_flash_mng_area(&info);
            if (RET_SUCCESS != ret)
            {
                update_result = UPDATE_RESULT_FLASH_MNG_AREA_UPDATE_FAIL;
                break;
            }
#endif
        }
    } while (0);
    
    /* The update is considered successful only when both verify update file and update process are RET_SUCCESS. */
    if ((RET_SUCCESS == check_ret) && (RET_SUCCESS == ret))
    {
        update_result = UPDATE_RESULT_SUCCESS;
    }
    
    /* Close the flash. */
    close_flash();
    
    return update_result;
}

/******************************************************************************
 * @fn       check_updatefile
 * @brief    Verify the contents of the update file.
 *
 * @param[in] p_data    Update file
 * @param[in] size      Update file size
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Update file invalid
 ******************************************************************************/
static ret_err_t check_updatefile(uint8_t * const p_data, uint32_t size)
{
    ret_err_t ret            = RET_FAIL;
    uint32_t  start_addr     = 0U;
    uint32_t  end_addr       = 0U;
    uint32_t  calculated_crc = 0U;
    uint32_t  data_len       = 0U;
    uint32_t  updatefile_crc = 0U;
    uint32_t  userapp_size   = 0U;
    
    do
    {
        /* Check update file size. */
        if ((sizeof(update_info_t) + CRC_DATA_SIZE) >= size)
        {
            /* Invalid update file size. */
            /* Process failed. */
            break;
        }
        
        /* Subtract the CRC size from the update file size. */
        data_len = size - CRC_DATA_SIZE;
        
        /* Calculate CRC. */
        calculated_crc = calc_crc32(p_data, data_len);
        
        /* Check if the CRC matches. */
        memcpy(&updatefile_crc, ((uint8_t *)(p_data + data_len)), CRC_DATA_SIZE);
        if (calculated_crc != updatefile_crc)
        {
            /* CRC mismatch. */
            break;
        }
        
        /* Copy update info. */
        update_info_t info;
        memcpy(&info, p_data, sizeof(update_info_t));
        
        /* Check if the payload length and file size are equal. */
        if ((sizeof(update_info_t) + CRC_DATA_SIZE + info.size) != size)
        {
            /* Payload length and file size mismatch. */
            /* Process failed. */
            break;
        }
        
        /* Check if parameter is included in update file. */
        if (PARAM_INFO_EXIST_MASK & info.target)
        {
            /* Check that info.size contains a value larger than the size of param info. */
            /* This makes sure the size of userapp is not 0. */
            if (USER_PARAM_INFO_AREA_SIZE >= info.size)
            {
                /* The update file does not include userapp. */
                /* Process failed. */
                break;
            }
            
            /* Subtract the size of parameter. */
            /* The size of userapp will never become 0 because it is checked by other processing. */
            userapp_size = info.size - USER_PARAM_INFO_AREA_SIZE;
        }
        else
        {
            /* Set the size of the userapp. */
            userapp_size = info.size;
            
            /* Check that the configured userapp size is not 0. */
            if (0U == userapp_size)
            {
                /* The update file does not include userapp. */
                /* Process failed. */
                break;
            }
        }
        
        /* Set the write destination address of the User application. */
        start_addr = info.write_addr;
        
        /* Check if data fits into 32-bit address space. */
        if ((MAX_VAL_ADDR_AREA_32BIT_SPACE - userapp_size + 1U) < start_addr)
        {
            break;
        }
        
        /* Add the size to the write address to determine the end address. */
        /* Subtract 1 because it is zero-based. */
        end_addr = (start_addr + userapp_size) - 1U;
        
        /* Address check when boot mode and update target flash type match. */
        if ((USER_APPLICATION_AREA_START_ADDR <= start_addr) && (end_addr <= USER_APPLICATION_AREA_END_ADDR))
        {
            /* Address is in range. */
            /* Do nothing. */
        }
#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
 #if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_SINGLE_BANK
        /* Address check when writing to USER_APPLICATION_EXTEND_AREA. */
        /* This process is valid only when the device is RZ/T2M, RZ/T2L, RZ/T2H, RZ/T2N or RZ/N2H. */
        /* This address confirmation is only performed for SINGLE_BANK. */
        else if ((USER_APPLICATION_EXTEND_AREA_START_ADDR <= start_addr) && (end_addr <= USER_APPLICATION_EXTEND_AREA_END_ADDR))
        {
            /* Address is in range. */
            /* Do nothing. */
        }
 #endif
#endif
        else
        {
            /* Address out of range. */
            /* Process failed. */
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}

/******************************************************************************
 * @fn       write_user_application
 * @brief    Write user application to external flash.
 *
 * @param[in] p_info    Update information
 * @param[in] p_data    User application
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Write fail
 ******************************************************************************/
static ret_err_t write_user_application(update_info_t * const p_info, uint8_t * const p_data)
{
    ret_err_t ret          = RET_FAIL;
    ret_err_t internal_ret = RET_FAIL;
    uint32_t  dest_addr    = p_info->write_addr;    ///< Address to start writing the user application
    uint32_t  size         = p_info->size;          ///< Size of user application to write to flash
    
    do
    {
#if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        uint32_t boot_state;
        uint32_t read_addr;
        uint32_t update_target = OPERATING_CPU_CORE_MASK & p_info->target;
        
        /* Set the address of the flash management area. */
        /* Check the update target core. */
        if (UPDATE_TARGET_CPU_CORE0 == update_target)
        {
            read_addr = FLASH_MNG_AREA_ADDR_CPU_CORE0;
        }
        else if (UPDATE_TARGET_CPU_CORE1 == update_target)
        {
            read_addr = FLASH_MNG_AREA_ADDR_CPU_CORE1;
        }
 #if defined(BSP_CFG_CORE_CA55)
        else if (UPDATE_TARGET_CPU_CORE2 == update_target)
        {
            read_addr = FLASH_MNG_AREA_ADDR_CPU_CORE2;
        }
        else if (UPDATE_TARGET_CPU_CORE3 == update_target)
        {
            read_addr = FLASH_MNG_AREA_ADDR_CPU_CORE3;
        }
 #endif
        else
        {
            /* Invalid update target. */
            /* Process failed. */
            break;
        }
        
        /* Get information on the current flash management area. */
        read_flash_data(read_addr, (uint8_t *)&boot_state, FLASH_MNG_AREA_PER_CPU_CORE_SIZE);
        
        /* Check the value of boot_state. */
        if (BOOT_STATE_A == boot_state)
        {
            /* If the current boot_state is A, write to bankB. */
            /* Offset address. */
            dest_addr += USER_APPLICATION_AREA_OFFSET;
        }
        else if (BOOT_STATE_B == boot_state)
        {
            /* If current boot_state is B, write to bankA. */
            /* Do nothing to the address. */
        }
        else
        {
            /* If the current boot_state is unknown, write to bankA. */
            /* Do nothing to the address. */
        }
#endif
        /* Subtract the size other than the user application from img_size. */
        /* Check if the parameter for the user application is included. */
        if (PARAM_INFO_EXIST_MASK & p_info->target)
        {
            /* Subtract the size of parameter. */
            size -= USER_PARAM_INFO_AREA_SIZE;
        }
        
        /* Write the user application. */
        internal_ret = write_flash_data(dest_addr, p_data, size);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}

/******************************************************************************
 * @fn       write_param_info
 * @brief    Write parameter to external flash.
 *
 * @param[in] p_info    Update information
 * @param[in] p_data    Parameter for the user application
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Write fail
 ******************************************************************************/
static ret_err_t write_param_info(update_info_t * const p_info, uint8_t * const p_data)
{
    ret_err_t ret          = RET_FAIL;
    ret_err_t internal_ret = RET_FAIL;
    uint32_t  dest_addr;
    
    do
    {
        uint32_t update_target = OPERATING_CPU_CORE_MASK & p_info->target;
        
        /* Set the address of the parameter for the user application area. */
        /* Check the target core. */
        if (UPDATE_TARGET_CPU_CORE0 == update_target)
        {
            dest_addr = PARAM_INFO_ADDR_CPU_CORE0;
        }
        else if (UPDATE_TARGET_CPU_CORE1 == update_target)
        {
            dest_addr = PARAM_INFO_ADDR_CPU_CORE1;
        }
#if defined(BSP_CFG_CORE_CA55)
        else if (UPDATE_TARGET_CPU_CORE2 == update_target)
        {
            dest_addr = PARAM_INFO_ADDR_CPU_CORE2;
        }
        else if (UPDATE_TARGET_CPU_CORE3 == update_target)
        {
            dest_addr = PARAM_INFO_ADDR_CPU_CORE3;
        }
#endif
        else
        {
            /* Invalid update target. */
            /* Process failed. */
            break;
        }
        
        /* Write the parameter for the user application. */
        internal_ret = write_flash_data(dest_addr, p_data, USER_PARAM_INFO_AREA_SIZE);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}

#if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
/******************************************************************************
 * @fn       change_flash_mng_area
 * @brief    Switch the state of the flash management area.
 *
 * @param[in] p_info    Update information
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Write fail
 ******************************************************************************/
static ret_err_t change_flash_mng_area(update_info_t * const p_info)
{
    ret_err_t ret          = RET_FAIL;
    ret_err_t internal_ret = RET_FAIL;
    uint32_t  boot_state;
    uint32_t  addr;
    
    do
    {
        uint32_t update_target = OPERATING_CPU_CORE_MASK & p_info->target;
        
        /* Set the address of the flash management area. */
        /* Check the target core. */
        if (UPDATE_TARGET_CPU_CORE0 == update_target)
        {
            addr = FLASH_MNG_AREA_ADDR_CPU_CORE0;
        }
        else if (UPDATE_TARGET_CPU_CORE1 == update_target)
        {
            addr = FLASH_MNG_AREA_ADDR_CPU_CORE1;
        }
 #if defined(BSP_CFG_CORE_CA55)
        else if (UPDATE_TARGET_CPU_CORE2 == update_target)
        {
            addr = FLASH_MNG_AREA_ADDR_CPU_CORE2;
        }
        else if (UPDATE_TARGET_CPU_CORE3 == update_target)
        {
            addr = FLASH_MNG_AREA_ADDR_CPU_CORE3;
        }
 #endif
        else
        {
            /* Invalid update target. */
            /* Process failed. */
            break;
        }
        
        /* Get information on the current flash management area. */
        read_flash_data(addr, (uint8_t *)&boot_state, FLASH_MNG_AREA_PER_CPU_CORE_SIZE);
        
        /* Set the boot state after switching. */
        if (BOOT_STATE_A == boot_state)
        {
            /* Switch boot_state from A to B. */
            boot_state = BOOT_STATE_B;
        }
        else if (BOOT_STATE_B == boot_state)
        {
            /* Switch boot_state from B to A. */
            boot_state = BOOT_STATE_A;
        }
        else
        {
            /* If the current boot_state is unknown, switch the boot_state to A. */
            boot_state = BOOT_STATE_A;
        }
        
        /* Write the flash management area. */
        internal_ret = write_flash_data(addr, (uint8_t *)&boot_state, FLASH_MNG_AREA_PER_CPU_CORE_SIZE);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}
#endif

/******************************************************************************
 * @fn     open_flash
 * @brief  Open the flash to be updated.
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Fail
 ******************************************************************************/
static ret_err_t open_flash(void)
{
    flash_err_t flash_err = FLASH_ERR_PARAMETER;
    ret_err_t   ret       = RET_FAIL;
    
    do
    {
#if defined(BSP_MCU_GROUP_RZT2M)
        /* Open flash module for RZ/T2M. */
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
        /* Initializes the QSPI module. */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_0);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        /* Initializes the BSC module. */
        flash_err = open_nor_flash();
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
 #endif
        
#elif defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
        /* Open flash module for RZ/T2L, RZ/T2H, RZ/T2N or RZ/N2H. */
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
        /* Initializes the OSPI module. */
        flash_err = open_ospi_flash(XSPI_UNIT_NUM_0);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
        /* Initializes the QSPI module. */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_1);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
 #endif
        
#elif defined(BSP_MCU_GROUP_RZN2L)
        /* Open flash module for RZ/N2L. */
 #if FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
        /* Initializes the QSPI module. */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_0);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
 #elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
        /* Initializes the BSC module. */
        flash_err = open_nor_flash();
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
 #endif
#endif
    } while (0);

    if (FLASH_SUCCESS == flash_err)
    {
        ret = RET_SUCCESS;
    }
    
    return ret;
}

/******************************************************************************
 * @fn       close_flash
 * @brief    Close the flash to be updated.
 ******************************************************************************/
static void close_flash(void)
{
#if defined(BSP_MCU_GROUP_RZT2M)
    /* Close flash module for RZ/T2M. */
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_0);
    
    /* BSC close. */
    close_nor_flash();
 #endif  
    
#elif defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
    /* Close flash module for RZ/T2L, RZ/T2H, RZ/T2N or RZ/N2H. */
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
    /* OSPI close. */
    close_ospi_flash(XSPI_UNIT_NUM_0);
    
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_1);
 #endif
    
#elif defined(BSP_MCU_GROUP_RZN2L)
    /* Close flash module for RZ/N2L. */
 #if FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_0);
    
 #elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
    /* BSC close. */
    close_nor_flash();
 #endif
#endif
    
    return;
}

#if FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
/******************************************************************************
 * @fn       read_flash_data
 * @brief    Read data from external flash.
 *
 * @param[in]  flash_addr    Data acquisition address
 * @param[out] p_data        Address to store the acquired data
 * @param[in]  size          The size of the data to be acquired
 ******************************************************************************/
static void read_flash_data(uint32_t flash_addr, void * const p_data, uint32_t size)
{
    uint32_t end_addr = (flash_addr + size) - 1U;
    
#if defined(BSP_MCU_GROUP_RZT2M)
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
    if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
    {
        /* Read to QSPI area. */
        read_to_qspi_area(flash_addr, p_data, size);
    }
    else if ((NOR_CS0_START_ADDR <= flash_addr) && (end_addr <= NOR_CS0_END_ADDR))
    {
        /* Read to NOR area. */
        read_to_nor_area(flash_addr, p_data, size);
    }
    else
    {
        /* Do nothing. */
    }
 #endif
    
#elif defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
    if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
    {
        /* Read to OSPI area. */
        read_to_ospi_area(flash_addr, p_data, size);
    }
    else if ((XSPI1_START_ADDR <= flash_addr) && (end_addr <= XSPI1_END_ADDR))
    {
        /* Read to QSPI area. */
        read_to_qspi_area(flash_addr, p_data, size);
    }
    else
    {
        /* Do nothing. */
    }
 #endif
    
#elif defined(BSP_MCU_GROUP_RZN2L)
 #if FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
    if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
    {
        /* Read to QSPI area. */
        read_to_qspi_area(flash_addr, p_data, size);
    }
 #elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
    if ((NOR_CS0_START_ADDR <= flash_addr) && (end_addr <= NOR_CS0_END_ADDR))
    {
        /* Read to NOR area. */
        read_to_nor_area(flash_addr, p_data, size);
    }
 #endif
#endif

    return;
}
#endif /* FWUPDATE_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK */

/******************************************************************************
 * @fn       write_flash_data
 * @brief    Write data to external flash.
 *
 * @param[in]  flash_addr    Data write destination address
 * @param[out] p_data        Write data
 * @param[in]  size          Write data size
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Fail
 ******************************************************************************/
static ret_err_t write_flash_data(uint32_t flash_addr, void * const p_data, uint32_t size)
{
    flash_err_t flash_err = FLASH_ERR_PARAMETER;
    ret_err_t   ret       = RET_FAIL;
    uint32_t    end_addr  = (flash_addr + size) - 1U;
    
    do
    {
#if defined(BSP_MCU_GROUP_RZT2M)
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
        if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
        {
            /* Write to QSPI area. */
            flash_err = write_to_qspi_area(flash_addr, p_data, size, XSPI_UNIT_NUM_0);
        }
        else if ((NOR_CS0_START_ADDR <= flash_addr) && (end_addr <= NOR_CS0_END_ADDR))
        {
            /* Write to NOR area. */
            flash_err = write_to_nor_area(flash_addr, p_data, size);
        }
        else
        {
            /* Do nothing. */
        }
 #endif

#elif defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
 #if (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
        if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
        {
            /* Write to OSPI area. */
            flash_err = write_to_ospi_area(flash_addr, p_data, size, XSPI_UNIT_NUM_0);
        }
        else if ((XSPI1_START_ADDR <= flash_addr) && (end_addr <= XSPI1_END_ADDR))
        {
            /* Write to QSPI area. */
            flash_err = write_to_qspi_area(flash_addr, p_data, size, XSPI_UNIT_NUM_1);
        }
        else
        {
            /* Do nothing. */
        }
 #endif
        
#elif defined(BSP_MCU_GROUP_RZN2L)
 #if FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
        if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
        {
            /* Write to QSPI area. */
            flash_err = write_to_qspi_area(flash_addr, p_data, size, XSPI_UNIT_NUM_0);
        }
 #elif FWUPDATE_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
        if ((NOR_CS0_START_ADDR <= flash_addr) && (end_addr <= NOR_CS0_END_ADDR))
        {
            /* Write to NOR area. */
            flash_err = write_to_nor_area(flash_addr, p_data, size);
        }
 #endif
#endif
    } while (0);
    
    if (FLASH_SUCCESS == flash_err)
    {
        ret = RET_SUCCESS;
    }
    
    return ret;
}

/******************************************************************************
 * @fn       create_send_packet
 * @brief    Create a send packet.
 *
 * @param[in]  p_rcv_packet     Send message buffer
 * @param[in]  cmd_code         Command code
 * @param[in]  nack_code        NACK code
 ******************************************************************************/
void create_send_packet(packet_header_t * const p_rcv_packet, uint8_t cmd_code, uint8_t nack_code)
{
    /* Initialize p_rcv_packet. */
    memset(p_rcv_packet, 0U, sizeof(packet_header_t));
    
    /* Set the packet data to be sent. */
    /* Set the command code. */
    p_rcv_packet->command = cmd_code;
    
    /* Check the type of command code. */
    if (CMD_CODE_NACK == cmd_code)
    {
        /* If the command code indicates NACK (abnormal termination), the payload is 1 bytes. */
        p_rcv_packet->payload_len = 1U;
        
        /* Set the NACK code. */
        p_rcv_packet->payload[0] = nack_code;
    }
    
    return;
}
