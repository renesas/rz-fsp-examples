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
#include "hal_data.h"
#include "flash.h"
#include "ssbl.h"

#if defined(BSP_CFG_CORE_CA55)
 #if SSBL_CFG_OPERATING_CORE_MODE == MULTI_CORE
  #if ((SSBL_CFG_MULTI_CORE_CA55_CORE1 == DISABLE) && (SSBL_CFG_MULTI_CORE_CA55_CORE2 == DISABLE) && (SSBL_CFG_MULTI_CORE_CA55_CORE3 == DISABLE))
   #warning "Multi-core mode is enabled, but all settings for SSBL_CFG_MULTI_CORE_CA55_CORE1-3 are DISABLE."
  #endif
 #endif
#endif

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/* Offset when starting user application of bank B */
#define USER_APPLICATION_AREA_OFFSET    ((uint32_t)(USER_APPLICATION_AREA_END_ADDR - USER_APPLICATION_AREA_START_ADDR + 1U))

/* Other */
#define BOOT_STATE_A                    ((uint32_t)0x10011001UL)
#define BOOT_STATE_B                    ((uint32_t)0x01100110UL)
#define CALC_CHECKSUM_CNT               (6U)

/*******************************************************************************
 * Typedef definitions
 *******************************************************************************/
/* Parameter table */
typedef struct st_param_info_t
{
    uint32_t src_addr;          ///< The source address of the executable program.
    uint32_t size;              ///< Execution program size.
    uint32_t app_start_addr;    ///< The destination address of the executable program.
    uint32_t checksum;          ///< Checksum.
} param_info_t;

/* Target application */
typedef enum e_target_application_t
{
    TARGET_APP_USER_APP_FOR_CPU_CORE0,
    TARGET_APP_USER_APP_FOR_CPU_CORE1,
    TARGET_APP_USER_APP_FOR_CPU_CORE2,
    TARGET_APP_USER_APP_FOR_CPU_CORE3
} target_application_t;

/* Return check value */
typedef enum e_ret_err_t
{
    RET_SUCCESS = 0,
    RET_FAIL    = 1,
} ret_err_t;

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static ret_err_t load_user_application (uint32_t * p_addr);
static ret_err_t load_update_program (uint32_t * p_addr);
static ret_err_t check_parameter_info (param_info_t * p_param, target_application_t app_target);
static uint32_t  calc_parameter_checksum (uint16_t * const p_param);
static ret_err_t open_flash (void);
static void      close_flash (void);
static void      read_flash_data (uint32_t flash_addr, void * const p_data, uint32_t size);

#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
#if SSBL_CFG_OPERATING_CORE_MODE == MULTI_CORE
static ret_err_t load_user_application_for_multi_core (void);
#endif
#endif

#if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
static ret_err_t adjust_src_addr_for_parameter (param_info_t * p_param, uint32_t flash_mng_area_addr);
#endif

static param_info_t st_s_g_param_update =
{
    .src_addr       = UPDATE_PROG_AREA_ADDR,
    .size           = DEPLOY_UPDATE_PROG_SIZE,
    .app_start_addr = UPDATE_PROG_START_ADDR,
    .checksum       = 0U    ///< Checksum is not used because it reads a fixed value.
};

/******************************************************************************
 * @fn       second_application_boot_loader
 * @brief    Select of application type.
 ******************************************************************************/
void second_application_boot_loader(void)
{
    ret_err_t      ret            = RET_FAIL;
    uint32_t       app_start_addr = 0U;
    bsp_io_level_t pin_level;
    bsp_io_level_t pin_level_check;
    
    /* Read the PIN to determine the SSBL mode. */
    R_BSP_PinAccessEnable();
    
    do
    {
        /* Read the current pin state. */
        /* Read the pin status twice and exit the loop if the pin status matches. */
        pin_level       = SSBL_PIN_READ;
        pin_level_check = SSBL_PIN_READ;
    } while (pin_level != pin_level_check);
    
    R_BSP_PinAccessDisable();
    
    /* Open the flash. */
    ret = open_flash();
    if (RET_SUCCESS != ret)
    {
        /* If the process fails, do nothing. */
        while (1);
    }
    
    /* Check the status of switch. */
    if (BSP_IO_LEVEL_HIGH == pin_level)
    {
        ret = load_user_application(&app_start_addr);
        
#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
 #if SSBL_CFG_OPERATING_CORE_MODE == MULTI_CORE
        if (RET_SUCCESS == ret)
        {
            /* Call load_user_application for multi core. */
            ret = load_user_application_for_multi_core();
        }
 #endif
#endif
    }
    else
    {
        ret = load_update_program(&app_start_addr);
    }
    
    /* Close the flash. */
    close_flash();
    
    if (RET_SUCCESS == ret)
    {
        /* Jump to app_start_addr. Launch the UserApp or Update program. */
        void (* application_main)(void);
        
        __disable_irq();
        
        __DSB();
        application_main = (void(*)(void))app_start_addr;
        application_main();
    }
    
    /* If the process fails, do nothing. */
    while (1);
}

/******************************************************************************
 * @fn       load_user_application
 * @brief    Load application start address for user application.
 *
 * @param[in,out] p_addr    User application start address
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Data error
 ******************************************************************************/
static ret_err_t load_user_application(uint32_t * p_addr)
{
    ret_err_t    ret          = RET_FAIL;
    ret_err_t    internal_ret = RET_FAIL;
    param_info_t param_cpu_core0;
    
    do
    {
        /* Set the parameter information for CPU0(Core0). */
        read_flash_data(PARAM_INFO_ADDR_CPU_CORE0, (uint8_t *)&param_cpu_core0, USER_PARAM_INFO_AREA_SIZE);
        
        /* Check the parameter information for CPU(Core0). */
        internal_ret = check_parameter_info(&param_cpu_core0, TARGET_APP_USER_APP_FOR_CPU_CORE0);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
#if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        /* Get value from flash manage area. */
        internal_ret = adjust_src_addr_for_parameter(&param_cpu_core0, FLASH_MNG_AREA_ADDR_CPU_CORE0);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
#endif
        
        /* Deploy the user application to app_start_addr on CPU(Core0). */
        read_flash_data(param_cpu_core0.src_addr, (uint8_t *)param_cpu_core0.app_start_addr, param_cpu_core0.size);
        
        /* Set the address to return to the SSBL's main function. */
        *p_addr = param_cpu_core0.app_start_addr;
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}

#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
#if SSBL_CFG_OPERATING_CORE_MODE == MULTI_CORE
/******************************************************************************
 * @fn       load_user_application_for_multi_core
 * @brief    Load the multi-core user application at the start address.
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Data error
 ******************************************************************************/
static ret_err_t load_user_application_for_multi_core(void)
{
    ret_err_t ret          = RET_FAIL;
    ret_err_t internal_ret = RET_FAIL;
    
    do
    {
        /* Disable register protection. */
        R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_LPC_RESET);
        R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_SYSTEM);
        
 #if defined(BSP_CFG_CORE_CR52)
        param_info_t param_cpu_core1;
        
        /* Set the parameter information for CPU1. */
        read_flash_data(PARAM_INFO_ADDR_CPU_CORE1, (uint8_t *)&param_cpu_core1, USER_PARAM_INFO_AREA_SIZE);
        
        /* Check the parameter information for CPU1. */
        internal_ret = check_parameter_info(&param_cpu_core1, TARGET_APP_USER_APP_FOR_CPU_CORE1);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
  #if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        /* Get value from flash manage area. */
        internal_ret = adjust_src_addr_for_parameter(&param_cpu_core1, FLASH_MNG_AREA_ADDR_CPU_CORE1);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
  #endif
        
        /* Deploy the user application to app_start_addr on CPU1. */
        read_flash_data(param_cpu_core1.src_addr, (uint8_t *)param_cpu_core1.app_start_addr, param_cpu_core1.size);
        
  #if (0 == BSP_FEATURE_BSP_HAS_CR52_CPU1_TCM)
        /* Release CR52_CPU1 reset state. */
        R_BSP_CpuResetRelease(SSBL_RESET_RELEASE_CPU_CORE1);
        
  #elif (1 == BSP_FEATURE_BSP_HAS_CR52_CPU1_TCM)
        R_BSP_CacheCleanAll();
        
        R_BSP_MODULE_START(FSP_IP_CR52, 1U);
        
        /* Release CR52_CPU1 reset state. */
        R_BSP_CpuResetRelease(SSBL_RESET_RELEASE_CPU_CORE1);
        
        /* Release from the slave stop state */
        R_SSC->SSTPCR7 = (uint32_t) ((uintptr_t) (R_SSC->SSTPCR7 & ~R_SSC_SSTPCR7_AXIS1_REQ_Msk));
        
        /* Polling ACK bit in SSTPCR */
        FSP_HARDWARE_REGISTER_WAIT(R_SSC->SSTPCR7_b.AXIS1_ACK, 0);
        
        __asm volatile (
            "    MOVS   r1, #0                    \n"
            "    MOVS   r2, %[dst_cpu1]           \n"
            "    MOVS   r0, %[inst]               \n"
            "    STRD   r0, r1, [r2]              \n"
            "    DSB SY                           \n"
            "    MOVS   r0, %[addr]               \n"
            "    STRD   r0, r1, [r2, #8]          \n"
            "    DSB SY                           \n"
            ::[inst] "r" (0xE51FF000),
            [addr] "r" (0x10000000),
            [dst_cpu1] "r" (0x21000000) : "memory", "r0", "r1", "r2");
        
        R_TCMAW->CPU1HALT = 0x00000000;
  #endif
 #endif
        
 #if defined(BSP_CFG_CORE_CA55)
  #if SSBL_CFG_MULTI_CORE_CA55_CORE1 == ENABLE
        param_info_t param_cpu_core1;
        
        /* Set the parameter information for Core1. */
        read_flash_data(PARAM_INFO_ADDR_CPU_CORE1, (uint8_t *)&param_cpu_core1, USER_PARAM_INFO_AREA_SIZE);
        
        /* Check the parameter information for Core1. */
        internal_ret = check_parameter_info(&param_cpu_core1, TARGET_APP_USER_APP_FOR_CPU_CORE1);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
   #if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        /* Get value from flash manage area. */
        internal_ret = adjust_src_addr_for_parameter(&param_cpu_core1, FLASH_MNG_AREA_ADDR_CPU_CORE1);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
   #endif
        
        /* Deploy the user application to app_start_addr on Core1. */
        read_flash_data(param_cpu_core1.src_addr, (uint8_t *)param_cpu_core1.app_start_addr, param_cpu_core1.size);
        
        /* Set the reset vector address in the RVBAL1 register. */
        R_CA55->RVBA[1].L = (uint32_t) (param_cpu_core1.app_start_addr);
        
        /* Cancel the module stop state. */
        R_BSP_MODULE_START(FSP_IP_CA55, 1U);
  #endif
        
  #if SSBL_CFG_MULTI_CORE_CA55_CORE2 == ENABLE
        param_info_t param_cpu_core2;
        
        /* Set the parameter information for Core2. */
        read_flash_data(PARAM_INFO_ADDR_CPU_CORE2, (uint8_t *)&param_cpu_core2, USER_PARAM_INFO_AREA_SIZE);
        
        /* Check the parameter information for Core2. */
        internal_ret = check_parameter_info(&param_cpu_core2, TARGET_APP_USER_APP_FOR_CPU_CORE2);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
   #if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        /* Get value from flash manage area. */
        internal_ret = adjust_src_addr_for_parameter(&param_cpu_core2, FLASH_MNG_AREA_ADDR_CPU_CORE2);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
   #endif
        
        /* Deploy the user application to app_start_addr on Core2. */
        read_flash_data(param_cpu_core2.src_addr, (uint8_t *)param_cpu_core2.app_start_addr, param_cpu_core2.size);
        
        /* Set the reset vector address in the RVBAL2 register. */
        R_CA55->RVBA[2].L = (uint32_t) (param_cpu_core2.app_start_addr);
        
        /* Cancel the module stop state. */
        R_BSP_MODULE_START(FSP_IP_CA55, 2U);
  #endif
        
  #if SSBL_CFG_MULTI_CORE_CA55_CORE3 == ENABLE
        param_info_t param_cpu_core3;
        
        /* Set the parameter information for Core3. */
        read_flash_data(PARAM_INFO_ADDR_CPU_CORE3, (uint8_t *)&param_cpu_core3, USER_PARAM_INFO_AREA_SIZE);
        
        /* Check the parameter information for Core3. */
        internal_ret = check_parameter_info(&param_cpu_core3, TARGET_APP_USER_APP_FOR_CPU_CORE3);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
        
   #if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
        /* Get value from flash manage area. */
        internal_ret = adjust_src_addr_for_parameter(&param_cpu_core3, FLASH_MNG_AREA_ADDR_CPU_CORE3);
        if (RET_SUCCESS != internal_ret)
        {
            break;
        }
   #endif
        
        /* Deploy the user application to app_start_addr on Core3. */
        read_flash_data(param_cpu_core3.src_addr, (uint8_t *)param_cpu_core3.app_start_addr, param_cpu_core3.size);
        
        /* Set the reset vector address in the RVBAL3 register. */
        R_CA55->RVBA[3].L = (uint32_t) (param_cpu_core3.app_start_addr);
        
        /* Cancel the module stop state. */
        R_BSP_MODULE_START(FSP_IP_CA55, 3U);
  #endif
        
        /* Clean the data cache. */
        R_BSP_CacheCleanAll();
        
  #if SSBL_CFG_MULTI_CORE_CA55_CORE1 == ENABLE
         /* Release the reset state. */
        R_BSP_CpuResetRelease(SSBL_RESET_RELEASE_CPU_CORE1);
  #endif
  #if SSBL_CFG_MULTI_CORE_CA55_CORE2 == ENABLE
        /* Release the reset state. */
        R_BSP_CpuResetRelease(SSBL_RESET_RELEASE_CPU_CORE2);
  #endif
  #if SSBL_CFG_MULTI_CORE_CA55_CORE3 == ENABLE
        /* Release the reset state. */
        R_BSP_CpuResetRelease(SSBL_RESET_RELEASE_CPU_CORE3);
  #endif
        
        /* Clean the data cache. */
        R_BSP_CacheCleanAll();
 #endif
        
        /* Enable register protection. */
        R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_SYSTEM);
        R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_LPC_RESET);
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}
#endif
#endif

/******************************************************************************
 * @fn       load_update_program
 * @brief    Load application start address for update program.
 *
 * @param[in,out] p_addr    Update program start address
 *
 * @retval    RET_SUCCESS    Success
 ******************************************************************************/
static ret_err_t load_update_program(uint32_t * p_addr)
{
    /* Deploy the update profram to app_start_addr. */
    read_flash_data(st_s_g_param_update.src_addr, (uint8_t *)st_s_g_param_update.app_start_addr, st_s_g_param_update.size);
    
    *p_addr = st_s_g_param_update.app_start_addr;
    
    return RET_SUCCESS;
}

/******************************************************************************
 * @fn       check_parameter_info
 * @brief    Check the parameter information.
 *
 * @param[in] p_param       Pointer to parameter information
 * @param[in] app_target    Application targeted by parameter information to check
 *
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Data error
 ******************************************************************************/
static ret_err_t check_parameter_info(param_info_t * p_param, target_application_t app_target)
{
    ret_err_t ret          = RET_FAIL;
    uint32_t  checksum     = 0U;
    uint32_t  end_addr     = 0U;
    uint32_t  userapp_size = 0U;    ///< User application size.
    
    do
    {
        /* Verify checksum. */
        checksum = calc_parameter_checksum((uint16_t *)p_param);
        if (p_param->checksum != checksum)
        {
            /* Process failed. */
            break;
        }
        
        /* Verify that the size of the user application is not 0. */
        if (0U == p_param->size)
        {
            /* The size of user application is invalid. */
            /* Process failed. */
            break;
        }
        
        userapp_size = p_param->size;
        
        /* Check whether the data in app_start_addr fits in 32-bit address space. */
        if ((MAX_VAL_ADDR_AREA_32BIT_SPACE - userapp_size + 1U) < p_param->app_start_addr)
        {
            /* It does not fit into the 32-bit address space. */
            /* Process failed. */
            break;
        }
        
        /* Add the size to the start address to determine the end address. */
        /* Subtract 1 because it is zero-based. */
        end_addr = (p_param->app_start_addr + userapp_size) - 1U;
        
#if defined(BSP_CFG_CORE_CR52)
        /* Check the type of CPU to be operated. */
        /* The RAM range to check changes depending on the type of CPU. */
        if (TARGET_APP_USER_APP_FOR_CPU_CORE0 == app_target)
        {
            /* Check whether the application fits within the target ATCM. */
            if ((p_param->app_start_addr < APP_EXECUTE_RAM_START_ADDR_CPU_CORE0) || (APP_EXECUTE_RAM_END_ADDR_CPU_CORE0 < end_addr))
            {
                /* Address out of range. */
                /* Process failed. */
                break;
            }
        }
 #if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
  #if SSBL_CFG_OPERATING_CORE_MODE == MULTI_CORE
        else if (TARGET_APP_USER_APP_FOR_CPU_CORE1 == app_target)
        {
            /* Check whether the application fits within the target SYSRAM. */
            if ((p_param->app_start_addr < APP_EXECUTE_RAM_START_ADDR_CPU_CORE1) || (APP_EXECUTE_RAM_END_ADDR_CPU_CORE1 < end_addr))
            {
                /* Address out of range. */
                /* Process failed. */
                break;
            }
        }
  #endif
 #endif
        else
        {
            /* Invalid app_target. */
            /* Process failed. */
            break;
        }
#endif
        
#if defined(BSP_CFG_CORE_CA55)
        if ((TARGET_APP_USER_APP_FOR_CPU_CORE0 == app_target) ||
            (TARGET_APP_USER_APP_FOR_CPU_CORE1 == app_target) ||
            (TARGET_APP_USER_APP_FOR_CPU_CORE2 == app_target) ||
            (TARGET_APP_USER_APP_FOR_CPU_CORE3 == app_target))
        {
            /* Check that the start address of the user application is within SYSRAM. */
            if ((p_param->app_start_addr < APP_EXECUTE_RAM_START_ADDR_SYSRAM) || (APP_EXECUTE_RAM_END_ADDR_SYSRAM < end_addr))
            {
                /* Check that the start address of the user application is within DDR mirror. */
                if ((p_param->app_start_addr < APP_EXECUTE_RAM_START_ADDR_DDR_MIRROR) || (APP_EXECUTE_RAM_END_ADDR_DDR_MIRROR < end_addr))
                {
                    /* Address out of range. */
                    /* Process failed. */
                    break;
                }
            }
        }
        else
        {
            /* Invalid app_target. */
            /* Process failed. */
            break;
        }
#endif
        
        /* Check whether the data in src_addr fits in 32-bit address space. */
        if ((MAX_VAL_ADDR_AREA_32BIT_SPACE - userapp_size + 1U) < p_param->src_addr)
        {
            /* It does not fit into the 32-bit address space. */
            /* Process failed. */
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}

/******************************************************************************
 * @fn       calc_parameter_checksum
 * @brief    Calculate the checksum of parameter.
 *
 * @param[in] p_param    Pointer to parameter information
 *
 * @return    Checksum value
 ******************************************************************************/
static uint32_t calc_parameter_checksum(uint16_t * const p_param)
{
    uint32_t checksum = 0U;
    uint8_t  cnt      = 0U;
    
    for (cnt = 0U; cnt < CALC_CHECKSUM_CNT; cnt++)
    {
        checksum += (*(p_param + cnt));
    }
    
    return checksum;
}

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
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
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
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
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
 #if SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
        /* Initializes the QSPI module. */
        flash_err = open_qspi_flash(XSPI_UNIT_NUM_0);
        if (FLASH_SUCCESS != flash_err)
        {
            break;
        }
        
 #elif SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
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
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_0);
    
    /* BSC close. */
    close_nor_flash();
 #endif  
    
#elif defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
    /* Close flash module for RZ/T2L, RZ/T2H, RZ/T2N or RZ/N2H. */
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
    /* OSPI close. */
    close_ospi_flash(XSPI_UNIT_NUM_0);
    
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_1);
 #endif
    
#elif defined(BSP_MCU_GROUP_RZN2L)
    /* Close flash module for RZ/N2L. */
 #if SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
    /* QSPI close. */
    close_qspi_flash(XSPI_UNIT_NUM_0);
    
 #elif SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
    /* BSC close. */
    close_nor_flash();
 #endif
#endif
    
    return;
}

/******************************************************************************
 * @fn       read_flash_data
 * @brief    Read data from the flash.
 *
 * @param[in]  flash_addr    Data acquisition address
 * @param[out] p_data        Address to store the acquired data
 * @param[in]  size          The size of the data to be acquired
 ******************************************************************************/
static void read_flash_data(uint32_t flash_addr, void * const p_data, uint32_t size)
{
    uint32_t end_addr = (flash_addr + size) - 1U;
    
#if defined(BSP_MCU_GROUP_RZT2M)
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR)
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
 #if (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0) || (SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI1)
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
 #if SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_XSPI0
    if ((XSPI0_START_ADDR <= flash_addr) && (end_addr <= XSPI0_END_ADDR))
    {
        /* Read to QSPI area. */
        read_to_qspi_area(flash_addr, p_data, size);
    }
 #elif SSBL_CFG_BOOT_MODE_SELECT == BOOT_MODE_NOR
    if ((NOR_CS0_START_ADDR <= flash_addr) && (end_addr <= NOR_CS0_END_ADDR))
    {
        /* Read to NOR area. */
        read_to_nor_area(flash_addr, p_data, size);
    }
 #endif
#endif

    return;
}

#if SSBL_CFG_FLASH_MNG_AREA == FLASH_MNG_AREA_DUAL_BANK
/******************************************************************************
 * @fn       adjust_src_addr_for_parameter
 * @brief    Adjust source address according to the status of flash manage area.
 *           If the flash manage area status is BankA, nothing will be done.
 *           If the flash manage area status is BankB, adjust src_addr with the specified value.
 *           Any other value will cause the process to terminate.
 *
 * @param[in,out] p_param                Pointer to parameter information
 * @param[in]     flash_mng_area_addr    Flash manage area address
 * 
 * @retval RET_SUCCESS    Success
 * @retval RET_FAIL       Data error
 ******************************************************************************/
static ret_err_t adjust_src_addr_for_parameter(param_info_t * p_param, uint32_t flash_mng_area_addr)
{
    uint32_t  boot_state = 0U;
    ret_err_t ret        = RET_FAIL;
    
    do
    {
        /* Get the current boot state. */
        read_flash_data(flash_mng_area_addr, (uint8_t *)&boot_state, FLASH_MNG_AREA_PER_CPU_CORE_SIZE);
        
        /* Check boot state. */
        if (BOOT_STATE_A == boot_state)
        {
            /* For boot state is A, do nothing. */
        }
        else if (BOOT_STATE_B == boot_state)
        {
            /* For boot state is B, adjust src_addr. */
            p_param->src_addr += USER_APPLICATION_AREA_OFFSET;
        }
        else
        {
            /* Invalid boot_state. */
            /* Process failed. */
            break;
        }
        
        ret = RET_SUCCESS;
        
    } while (0);
    
    return ret;
}
#endif
