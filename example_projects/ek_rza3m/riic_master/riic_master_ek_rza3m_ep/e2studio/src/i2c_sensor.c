/***********************************************************************************************************************
 * File Name    : i2c_sensor.c
 * Description  : Contains data structures and functions used in i2c_sensor.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "i2c_sensor.h"
#include <stdbool.h>
#include <stdint.h>

/***********************************************************************************************************************
 * @addtogroup r_riic_master_ep
 * @{
 **********************************************************************************************************************/

/*
 * Global Variables
 */

uint8_t g_measure_enable_payload[MEASURE_PAYLOAD_SIZE] __attribute__((section("UNCACHED_BSS")));
uint8_t g_adxl_data[SENSOR_DATA_SIZE] __attribute__((section("UNCACHED_BSS")));
uint8_t g_internal_reg __attribute__((section("UNCACHED_BSS")));
uint8_t g_reg_address __attribute__((section("UNCACHED_BSS")));

/* Reading I2C call back event through i2c_Master callback */
static volatile i2c_master_event_t s_i2c_event = I2C_MASTER_EVENT_ABORTED;

/*
 * Private function declarations
 */
static fsp_err_t get_device_id (uint8_t * dev_id);
static fsp_err_t validate_i2c_event (void);

/***********************************************************************************************************************
 *  @brief       This function initializes the RIIC master module and sets up the PMOD ACL sensor.
 *  @param[IN]   None.
 *  @retval      FSP_SUCCESS    Upon successful open and start of timer.
 *  @retval      Any other error code apart from FSP_SUCCESS is  Unsuccessful open or start.
 **********************************************************************************************************************/
fsp_err_t init_sensor(void)
{
    fsp_err_t err = FSP_SUCCESS;

    uint8_t device_id = RESET_VALUE;

    g_measure_enable_payload[0] = POWER_CTL_REG;
    g_measure_enable_payload[1] = ENABLE_BIT;

    /* Open RIIC master module */
    err = R_RIIC_MASTER_Open(&g_i2c_master_ctrl, &g_i2c_master_cfg);
    
    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Open API failed **\r\n");
        return err;
    }

    /* Get device ID as a start of communication */
    err = get_device_id(&device_id);

    /* Verify I2C transaction is successful with PMOD ACL and correct device ID is received */
    if ((FSP_SUCCESS == err) && (DEVICE_SIGNATURE == device_id))
    {
        /* Access to POWER_CTL Register (0x2D) and enable measurement */
        err = R_RIIC_MASTER_Write(&g_i2c_master_ctrl, &g_measure_enable_payload[0], TWO_BYTE, false);
        if (FSP_SUCCESS == err)
        {
            err = validate_i2c_event();

            /* Handle error */
            if (FSP_ERR_TRANSFER_ABORTED == err)
            {
                APP_ERR_PRINT("** POWER_CTL reg I2C write failed **\r\n");
            }
        }
        /* Handle error */
        else
        {
            /* Write API returns itself is not successful */
            APP_ERR_PRINT("** R_RIIC_MASTER_Write API failed **\r\n");
        }
    }
    /* Handle error */
    else
    {
        /* Failed to get Device ID */
        APP_ERR_PRINT("** get_device_id failed **\r\n");
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief       This function de-initializes the RIIC master module.
 *  @param[IN]   None.
 *  @retval      None.
 **********************************************************************************************************************/
void deinit_sensor(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close open modules */
    err =  R_RIIC_MASTER_Close (&g_i2c_master_ctrl);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Close API failed **\r\n");
    }
}

/***********************************************************************************************************************
 *  @brief       This function reads the ACL sensor data for the X, Y, and Z axes and prints the raw data on the terminal
 *               emulator.
 *  @param[IN]   Array pointer to get the sensor data.
 *  @retval      FSP_SUCCESS    Upon successful open and start of timer.
 *  @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open or start.
 **********************************************************************************************************************/
fsp_err_t read_sensor_data(uint8_t *xyz_data)
{
    fsp_err_t err = FSP_SUCCESS;

    g_internal_reg = AXIS_DATA;
    memset(g_adxl_data, 0U, SENSOR_DATA_SIZE);
    if (NULL == xyz_data)
    {
        err = FSP_ERR_INVALID_POINTER;
        APP_ERR_PRINT("** NULL Pointer check fail **\r\n");
        return err;
    }

    /* Write Accelerometer Data register */
    err = R_RIIC_MASTER_Write(&g_i2c_master_ctrl, &g_internal_reg, ONE_BYTE, true);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Write API failed **\r\n");
        return err;
    }
    else
    {
        err = validate_i2c_event();

        /* Perform read operation only when write event is successful, failure part is handled at next stage */
        if (FSP_SUCCESS == err)
        {
            err = R_RIIC_MASTER_Read(&g_i2c_master_ctrl, &g_adxl_data[0], DATA_REGISTERS, false);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                APP_ERR_PRINT("** R_RIIC_MASTER_Read API failed **\r\n");
                return err;
            }
            else
            {
                err = validate_i2c_event();
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT("** Data register, I2C read failed **\r\n");
                    return err;
                }
            }
        }
        else
        {
            APP_ERR_PRINT("** Accel Data register, I2C write failed **\r\n");
            return err;
        }

        /* Read is successful update the x,y,z values and return success */
        for (uint8_t itr = RESET_VALUE; itr < SENSOR_DATA_SIZE; itr++)
        {
            *(xyz_data+itr) = g_adxl_data[itr];
        }
    }
    return err;
}

/***********************************************************************************************************************
 *  @brief     This function reads the device signature/ID from the sensor.
 *  @param[in] dev_id                       retrieve PMOD ACL device ID.
 *  @retval    FSP_SUCCESS                  on successful I2C transaction.
 *             FSP_ERR_INVALID_POINTER      if invalid parameters passed.
 *             err                          whichever occurs on either API or on I2C event failure.
 **********************************************************************************************************************/
static fsp_err_t get_device_id(uint8_t *dev_id)
{
    fsp_err_t err = FSP_SUCCESS;

    g_reg_address = DEVICE_ID_REG;

    /* Parameter checking */
    if (NULL == dev_id)
    {
        err = FSP_ERR_INVALID_POINTER;
        APP_ERR_PRINT("** NULL Pointer check fail **\r\n");
        return err;
    }

    /* Send register address to sensor */
    err = R_RIIC_MASTER_Write(&g_i2c_master_ctrl, &g_reg_address, ONE_BYTE, true);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** R_RIIC_MASTER_Write API failed **\r\n");
    }
    else
    {
        err = validate_i2c_event();

        /* Handle error */
        if (FSP_ERR_TRANSFER_ABORTED == err)
        {
            APP_ERR_PRINT("** Device ID reg, I2C write failed **\r\n");
        }
        else
        {
            /* Read only when I2C write and its callback event is successful */
            err  = R_RIIC_MASTER_Read(&g_i2c_master_ctrl, dev_id, ONE_BYTE, false);

            /* Handle error */
            if (FSP_SUCCESS != err)
            {
                /* Do nothing, the error is returned in the end */
                APP_ERR_PRINT("** R_RIIC_MASTER_Read API failed **\r\n");
            }
            else
            {
                err = validate_i2c_event();

                /* Handle error */
                if (FSP_ERR_TRANSFER_ABORTED == err)
                {
                    APP_ERR_PRINT("** Device ID read,  I2C read failed **\r\n");
                }
            }
        }
    }

    /* On successful I2C transaction with PMOD-ACL return from here */
    return err;
}

/***********************************************************************************************************************
 *  @brief      User callback function.
 *  @param[in]  p_args    Callback arguments.
 *  @retval     None.
 **********************************************************************************************************************/
void i2c_master_callback(i2c_master_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        /* Capture callback event for validating the I2C transfer event */
        s_i2c_event = p_args->event;
    }
}

/***********************************************************************************************************************
 *  @brief       This function validates I2C receive/transmit operations based on the required write/read operation.
 *  @param[in]   None.
 *  @retval      FSP_SUCCESS                       successful event receiving returns FSP_SUCCESS.
 *               FSP_ERR_TRANSFER_ABORTED          Either on timeout elapsed or received callback event is
 *                                                 I2C_MASTER_EVENT_ABORTED.
 **********************************************************************************************************************/
static fsp_err_t validate_i2c_event(void)
{
    uint64_t local_time_out = UINT64_MAX;

    /* Reset call back event capture variable */
    s_i2c_event = (i2c_master_event_t)RESET_VALUE;

    do
    {
        /* This is to avoid infinite loop */
        --local_time_out;

        if (RESET_VALUE == local_time_out)
        {
            return FSP_ERR_TRANSFER_ABORTED;
        }

    }
    while (RESET_VALUE == s_i2c_event);

    if (I2C_MASTER_EVENT_ABORTED != s_i2c_event)
    {
        /* Make sure this is always Reset before return */
        s_i2c_event = (i2c_master_event_t)RESET_VALUE;
        return FSP_SUCCESS;
    }

    /* Make sure this is always Reset before return */
    s_i2c_event = (i2c_master_event_t)RESET_VALUE;
    return FSP_ERR_TRANSFER_ABORTED;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_riic_master_ep)
 **********************************************************************************************************************/
